#include "vulkan_backend.h"
#include "vulkan_types.inl"
#include "vulkan_platform.h"
#include "vulkan_device.h"
#include "vulkan_swapchain.h"
#include "vulkan_command_buffer.h"
#include "vulkan_renderpass.h"
#include "vulkan_framebuffer.h"
#include "vulkan_fence.h"
#include "vulkan_utils.h"
#include "vulkan_buffer.h"
#include "vulkan_image.h"

#include "shaders/vulkan_material_shader.h"

#include "core/logger.h"
#include "core/vmemory.h"
#include "core/vstring.h"
#include "core/application.h"

#include "math/math_types.h"

#include "containers/darray.h"

static vulkan_context Context;
static u32 CachedFramebufferWidth;
static u32 CachedFramebufferHeight;

VKAPI_ATTR VkBool32 DebugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT MessageSeverity,
                                  VkDebugUtilsMessageTypeFlagsEXT MessageTypes,
                                  const VkDebugUtilsMessengerCallbackDataEXT* CallbackData,
                                  void* UserData);

s32 FindMemoryIndex(u32 TypeFilter, u32 PropertyFlags);
b8 CreateBuffers(vulkan_context* Context);

void CreateCommandBuffers(renderer_backend* Backend);
void RegenerateFramebuffers(renderer_backend* Backend, vulkan_swapchain* Swapchain, vulkan_renderpass* Renderpass);
b8 RecreateSwapchain(renderer_backend* Backend);

void UploadDataRange(vulkan_context* Context, VkCommandPool Pool, VkFence Fence, VkQueue Queue, vulkan_buffer* Buffer, u64 Offset, u64 Size, void* Data)
{
    VkBufferUsageFlags Flags = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
    vulkan_buffer Staging;
    VulkanCreateBuffer(Context, Size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, Flags, true, &Staging);

    VulkanBufferLoadData(Context, &Staging, 0, Size, 0, Data);

    VulkanBufferCopyTo(Context, Pool, Fence, Queue, Staging.Handle, 0, Buffer->Handle, Offset, Size);

    VulkanDestroyBuffer(Context, &Staging);
}

b8 VulkanRendererBackendInitialize(renderer_backend* Backend, const char* ApplicationName)
{
    Context.Allocator         = 0;
    Context.FindMemoryIndex   = FindMemoryIndex;

    ApplicationGetFramebufferSize(&CachedFramebufferWidth, &CachedFramebufferHeight);
    Context.FramebufferWidth  = (CachedFramebufferWidth  != 0) ? CachedFramebufferWidth  : 800;
    Context.FramebufferHeight = (CachedFramebufferHeight != 0) ? CachedFramebufferHeight : 800;
    CachedFramebufferWidth  = 0;
    CachedFramebufferHeight = 0;

    VkApplicationInfo AppInfo = {VK_STRUCTURE_TYPE_APPLICATION_INFO};
    AppInfo.apiVersion = VK_API_VERSION_1_2;
    AppInfo.pApplicationName = ApplicationName;
    AppInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
    AppInfo.pEngineName = "VENG";
    AppInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);

    VkInstanceCreateInfo InstanceCreateInfo = {VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO};
    InstanceCreateInfo.pApplicationInfo = &AppInfo;

    const char** RequiredExtensions = DArrayCreate(const char*);
    DArrayPush(RequiredExtensions, &VK_KHR_SURFACE_EXTENSION_NAME);
    PlatformGetRequiredExtensionNames(&RequiredExtensions);
#if defined(_DEBUG)
    DArrayPush(RequiredExtensions, &VK_EXT_DEBUG_UTILS_EXTENSION_NAME);

    VENG_DEBUG("Required Extensions:");
    for(u32 ReqIndex = 0;
        ReqIndex < DArrayLength(RequiredExtensions);
        ++ReqIndex)
    {
        VENG_DEBUG(RequiredExtensions[ReqIndex]);
    }
#endif

    InstanceCreateInfo.ppEnabledExtensionNames = RequiredExtensions;
    InstanceCreateInfo.enabledExtensionCount   = DArrayLength(RequiredExtensions);

    const char** RequiredValidationLayerNames = 0;
    u32 RequiredValidationLayerCount = 0;

#if defined(_DEBUG)
    RequiredValidationLayerNames = DArrayCreate(const char*);
    DArrayPush(RequiredValidationLayerNames, &"VK_LAYER_KHRONOS_validation");
    RequiredValidationLayerCount = DArrayLength(RequiredValidationLayerNames);

    u32 AvailableLayerCount = 0;
    VK_CHECK(vkEnumerateInstanceLayerProperties(&AvailableLayerCount, 0));
    VkLayerProperties* AvailableLayers = DArrayReserve(VkLayerProperties, AvailableLayerCount);
    VK_CHECK(vkEnumerateInstanceLayerProperties(&AvailableLayerCount, AvailableLayers));

    for(u32 RequiredIndex = 0;
        RequiredIndex < RequiredValidationLayerCount;
        ++RequiredIndex)
    {
        b8 IsFound = false;
        for(u32 LayerIndex = 0;
            LayerIndex < AvailableLayerCount;
            ++LayerIndex)
        {
            if(IsStringsEqual(RequiredValidationLayerNames[RequiredIndex], AvailableLayers[LayerIndex].layerName))
            {
                IsFound = true;
                break;
            }
        }

        if(!IsFound)
        {
            VENG_FATAL("Required validation layer is missing: %s", RequiredValidationLayerNames[RequiredIndex]);
            return false;
        }
    }
#endif

    InstanceCreateInfo.enabledLayerCount = RequiredValidationLayerCount;
    InstanceCreateInfo.ppEnabledLayerNames = RequiredValidationLayerNames;

    VK_CHECK(vkCreateInstance(&InstanceCreateInfo, Context.Allocator, &Context.Instance));

#if defined(_DEBUG)
    u32 LogSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT | 
                      VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT;

    VkDebugUtilsMessengerCreateInfoEXT DebugCreateInfo = {VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT};
    DebugCreateInfo.messageSeverity = LogSeverity;
    DebugCreateInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT;
    DebugCreateInfo.pfnUserCallback = DebugCallback;

    PFN_vkCreateDebugUtilsMessengerEXT vkCreateDebugUtilsMessengerEXT = 
        (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(Context.Instance, "vkCreateDebugUtilsMessengerEXT");
    VK_CHECK(vkCreateDebugUtilsMessengerEXT(Context.Instance, &DebugCreateInfo, Context.Allocator, &Context.DebugMessenger));
#endif

    if(!PlatformCreateVulkanSurface(&Context))
    {
        VENG_ERROR("Failed to create SurfaceKHR");
        return false;
    }

    if(!VulkanDeviceCreate(&Context))
    {
        VENG_ERROR("Failed to create device");
        return false;
    }

    VulkanSwapchainCreate(&Context, Context.FramebufferWidth, Context.FramebufferHeight, &Context.Swapchain);

    VulkanRenderpassCreate(&Context, &Context.MainRenderpass, 
                           0, 0, Context.FramebufferWidth, Context.FramebufferHeight, 
                           0.0f, 0.0f, 0.2f, 1.0f, 
                           1.0f, 0);

    Context.Swapchain.Framebuffers = DArrayReserve(vulkan_framebuffer, Context.Swapchain.ImageCount);
    RegenerateFramebuffers(Backend, &Context.Swapchain, &Context.MainRenderpass);

    CreateCommandBuffers(Backend);

    Context.ImageAvailableSemaphores = DArrayReserve(VkSemaphore,  Context.Swapchain.MaxFramesInFlight);
    Context.QueueCompleteSemaphores  = DArrayReserve(VkSemaphore,  Context.Swapchain.MaxFramesInFlight);
    Context.InFlightFences           = DArrayReserve(vulkan_fence, Context.Swapchain.MaxFramesInFlight);

    for(u8 FrameIndex = 0;
        FrameIndex < Context.Swapchain.MaxFramesInFlight;
        ++FrameIndex)
    {
        VkSemaphoreCreateInfo SemaphoreCreateInfo = {VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO};
        vkCreateSemaphore(Context.Device.LogicalDevice, &SemaphoreCreateInfo, Context.Allocator, &Context.ImageAvailableSemaphores[FrameIndex]);
        vkCreateSemaphore(Context.Device.LogicalDevice, &SemaphoreCreateInfo, Context.Allocator, &Context.QueueCompleteSemaphores[FrameIndex]);

        VulkanFenceCreate(&Context, true, &Context.InFlightFences[FrameIndex]);
    }

    Context.ImagesInFlight = DArrayReserve(vulkan_fence, Context.Swapchain.ImageCount);
    for(u32 ImageIndex = 0;
        ImageIndex < Context.Swapchain.ImageCount;
        ++ImageIndex)
    {
        Context.ImagesInFlight[ImageIndex] = 0;
    }

    if(!VulkanMaterialShaderCreate(&Context, &Context.MaterialShader))
    {
        VENG_ERROR("Error loading built-in basic shader");
        return false;
    }

    CreateBuffers(&Context);

    // NOTE: Test code
    const u32 VertCount = 4;
    vertex_3d Verts[VertCount];
    ZeroMemory(Verts, VertCount * sizeof(vertex_3d));

    const r32 f = 10.0f;
    Verts[0].Position.x = -0.5f * f;
    Verts[0].Position.y = -0.5f * f;
    Verts[0].TexCoord.x =  0.0f;
    Verts[0].TexCoord.y =  0.0f;

    Verts[1].Position.x =  0.5f * f;
    Verts[1].Position.y =  0.5f * f;
    Verts[1].TexCoord.x =  1.0f;
    Verts[1].TexCoord.y =  1.0f;

    Verts[2].Position.x = -0.5f * f;
    Verts[2].Position.y =  0.5f * f;
    Verts[2].TexCoord.x =  0.0f;
    Verts[2].TexCoord.y =  1.0f;

    Verts[3].Position.x =  0.5f * f;
    Verts[3].Position.y = -0.5f * f;
    Verts[3].TexCoord.x =  1.0f;
    Verts[3].TexCoord.y =  0.0f;

    const u32 IndexCount = 6;
    u32 Indices[IndexCount] = {0, 1, 2, 0, 3, 1};

    UploadDataRange(&Context, Context.Device.GraphicsCommandPool, 0, Context.Device.GraphicsQueue, &Context.ObjectVertexBuffer, 0, sizeof(vertex_3d) * VertCount, Verts);
    UploadDataRange(&Context, Context.Device.GraphicsCommandPool, 0, Context.Device.GraphicsQueue, &Context.ObjectIndexBuffer , 0, sizeof(u32) * IndexCount, Indices);

    u32 ObjectID = 0;
    if(!VulkanMaterialShaderAcquireResources(&Context, &Context.MaterialShader, &ObjectID))
    {
        VENG_ERROR("Failed to acquire resources!");
        return false;
    }

    VENG_INFO("Vulkan renderer initialized successfully.");

    return true;
}

void VulkanRendererBackendShutdown(renderer_backend* Backend)
{
    vkDeviceWaitIdle(Context.Device.LogicalDevice);

    VulkanDestroyBuffer(&Context, &Context.ObjectIndexBuffer);
    VulkanDestroyBuffer(&Context, &Context.ObjectVertexBuffer);

    VulkanMaterialShaderDestroy(&Context, &Context.MaterialShader);

    for(u8 FrameIndex = 0;
        FrameIndex < Context.Swapchain.MaxFramesInFlight;
        ++FrameIndex)
    {
        if(Context.ImageAvailableSemaphores[FrameIndex])
        {
            vkDestroySemaphore(Context.Device.LogicalDevice, Context.ImageAvailableSemaphores[FrameIndex], Context.Allocator);
        }
        if(Context.QueueCompleteSemaphores[FrameIndex])
        {
            vkDestroySemaphore(Context.Device.LogicalDevice, Context.QueueCompleteSemaphores[FrameIndex], Context.Allocator);
        }
        VulkanFenceDestroy(&Context, &Context.InFlightFences[FrameIndex]);
    }
    DArrayDestroy(Context.ImageAvailableSemaphores);
    Context.ImageAvailableSemaphores = 0;

    DArrayDestroy(Context.QueueCompleteSemaphores);
    Context.QueueCompleteSemaphores = 0;

    DArrayDestroy(Context.InFlightFences);
    Context.InFlightFences = 0;

    DArrayDestroy(Context.ImagesInFlight);
    Context.ImagesInFlight = 0;

    for(u32 ImageIndex = 0;
        ImageIndex < Context.Swapchain.ImageCount;
        ++ImageIndex)
    {
        if(Context.GraphicsCommandBuffers[ImageIndex].Handle)
        {
            VulkanCommandBufferFree(&Context, Context.Device.GraphicsCommandPool, &Context.GraphicsCommandBuffers[ImageIndex]);
            Context.GraphicsCommandBuffers[ImageIndex].Handle = 0;
        }
    }

    DArrayDestroy(Context.GraphicsCommandBuffers);
    Context.GraphicsCommandBuffers = 0;

    for(u32 ImageIndex = 0;
        ImageIndex < Context.Swapchain.ImageCount;
        ++ImageIndex)
    {
        VulkanFramebufferDestroy(&Context, &Context.Swapchain.Framebuffers[ImageIndex]);
    }

    VulkanRenderpassDestroy(&Context, &Context.MainRenderpass);

    VulkanSwapchainDestroy(&Context, &Context.Swapchain);

    VulkanDeviceDestroy(&Context);

    if(Context.Surface)
    {
        vkDestroySurfaceKHR(Context.Instance, Context.Surface, Context.Allocator);
        Context.Surface = 0;
    }

#if defined(_DEBUG)
    if(Context.DebugMessenger)
    {
        PFN_vkDestroyDebugUtilsMessengerEXT vkDestroyDebugUtilsMessengerEXT = 
            (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(Context.Instance, "vkDestroyDebugUtilsMessengerEXT");
        vkDestroyDebugUtilsMessengerEXT(Context.Instance, Context.DebugMessenger, Context.Allocator);
    }
#endif

    vkDestroyInstance(Context.Instance, Context.Allocator);
}

void VulkanRendererBackendResized(renderer_backend* Backend, u16 Width, u16 Height)
{
    CachedFramebufferWidth  = Width;
    CachedFramebufferHeight = Height;
    Context.FramebufferSizeGeneration++;

    VENG_INFO("Vulkan renderer backend->resized: Width=%i; Height=%i; Gen=%llu", Width, Height, Context.FramebufferSizeGeneration);
}

b8 VulkanRendererBackendBeginFrame(renderer_backend* Backend, r32 DeltaTime)
{
    Context.DeltaTime = DeltaTime;

    vulkan_device* Device = &Context.Device;

    if(Context.RecreatingSwapchain)
    {
        VkResult Result = vkDeviceWaitIdle(Device->LogicalDevice);
        if(!VulkanResultIsSuccess(Result))
        {
            VENG_ERROR("vulkan_renderer_backend_begin_frame vkDeviceWaitIdle (1) failed: '%s'", VulkanResultString(Result, true));
            return false;
        }
        VENG_INFO("Recreating swapchain, booting");
        return false;
    }

    if(Context.FramebufferSizeGeneration != Context.FramebufferSizeLastGeneration)
    {
        VkResult Result = vkDeviceWaitIdle(Device->LogicalDevice);
        if(!VulkanResultIsSuccess(Result))
        {
            VENG_ERROR("vulkan_renderer_backend_begin_frame vkDeviceWaitIdle (2) failed: '%s'", VulkanResultString(Result, true));
            return false;
        }

        if(!RecreateSwapchain(Backend))
        {
            return false;
        }

        VENG_INFO("Resized, booting");
        return false;
    }

    if(!VulkanFenceWait(&Context, &Context.InFlightFences[Context.CurrentFrame], UINT64_MAX))
    {
        VENG_WARN("In-Flight fence wait failure");
        return false;
    }

    if(!VulkanSwapchainAcquireNextImageIndex(&Context, &Context.Swapchain, UINT64_MAX, Context.ImageAvailableSemaphores[Context.CurrentFrame], 0, &Context.ImageIndex))
    {
        return false;
    }

    vulkan_command_buffer* CommandBuffer = &Context.GraphicsCommandBuffers[Context.ImageIndex];
    VulkanCommandBufferReset(CommandBuffer);
    VulkanCommandBufferBegin(CommandBuffer, false, false, false);

    VkViewport Viewport = {};
    Viewport.x = 0.0f;
    Viewport.y = (r32)Context.FramebufferHeight;
    Viewport.width  =  (r32)Context.FramebufferWidth;
    Viewport.height = -(r32)Context.FramebufferHeight;
    Viewport.minDepth = 0.0f;
    Viewport.maxDepth = 1.0f;

    VkRect2D Scissor = {};
    Scissor.offset.x = 0;
    Scissor.offset.y = 0;
    Scissor.extent.width  = Context.FramebufferWidth;
    Scissor.extent.height = Context.FramebufferHeight;

    vkCmdSetViewport(CommandBuffer->Handle, 0, 1, &Viewport);
    vkCmdSetScissor(CommandBuffer->Handle, 0, 1, &Scissor);

    Context.MainRenderpass.Width  = Context.FramebufferWidth;
    Context.MainRenderpass.Height = Context.FramebufferHeight;

    VulkanRenderpassBegin(CommandBuffer, &Context.MainRenderpass, Context.Swapchain.Framebuffers[Context.ImageIndex].Handle);

    return true;
}

void VulkanRendererUpdateGlobalState(mat4 Projection, mat4 View, v3 ViewPosition, v4 AmbientColor, s32 Mode)
{
    vulkan_command_buffer* CommandBuffer = &Context.GraphicsCommandBuffers[Context.ImageIndex];

    VulkanMaterialShaderUse(&Context, &Context.MaterialShader);

    Context.MaterialShader.GlobalUBO.Projection = Projection;
    Context.MaterialShader.GlobalUBO.View       = View;

    VulkanMaterialShaderUpdateGlobalState(&Context, &Context.MaterialShader, Context.DeltaTime);
}

void VulkanUpdateObject(geometry_render_data RenderData)
{
    vulkan_command_buffer* CommandBuffer = &Context.GraphicsCommandBuffers[Context.ImageIndex];

    VulkanMaterialShaderUpdateObject(&Context, &Context.MaterialShader, RenderData);

    VulkanMaterialShaderUse(&Context, &Context.MaterialShader);

    VkDeviceSize Offsets[1] = {0};
    vkCmdBindVertexBuffers(CommandBuffer->Handle, 0, 1, &Context.ObjectVertexBuffer.Handle, (VkDeviceSize*)Offsets);
    vkCmdBindIndexBuffer(CommandBuffer->Handle, Context.ObjectIndexBuffer.Handle, 0, VK_INDEX_TYPE_UINT32);
    vkCmdDrawIndexed(CommandBuffer->Handle, 6, 1, 0, 0, 0);
}

b8 VulkanRendererBackendEndFrame(renderer_backend* Backend, r32 DeltaTime)
{
    vulkan_command_buffer* CommandBuffer = &Context.GraphicsCommandBuffers[Context.ImageIndex];

    VulkanRenderpassEnd(CommandBuffer, &Context.MainRenderpass);
    VulkanCommandBufferEnd(CommandBuffer);

    if(Context.ImagesInFlight[Context.ImageIndex] != VK_NULL_HANDLE)
    {
        VulkanFenceWait(&Context, Context.ImagesInFlight[Context.ImageIndex], UINT64_MAX);
    }

    Context.ImagesInFlight[Context.ImageIndex] = &Context.InFlightFences[Context.CurrentFrame];

    VulkanFenceReset(&Context, &Context.InFlightFences[Context.CurrentFrame]);

    VkPipelineStageFlags Flags[1] = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};

    VkSubmitInfo SubmitInfo = {VK_STRUCTURE_TYPE_SUBMIT_INFO};
    SubmitInfo.commandBufferCount = 1;
    SubmitInfo.pCommandBuffers = &CommandBuffer->Handle;

    SubmitInfo.signalSemaphoreCount = 1;
    SubmitInfo.pSignalSemaphores = &Context.QueueCompleteSemaphores[Context.CurrentFrame];

    SubmitInfo.waitSemaphoreCount = 1;
    SubmitInfo.pWaitSemaphores = &Context.ImageAvailableSemaphores[Context.CurrentFrame];

    SubmitInfo.pWaitDstStageMask = Flags;

    VkResult Result = vkQueueSubmit(Context.Device.GraphicsQueue, 1, &SubmitInfo, Context.InFlightFences[Context.CurrentFrame].Handle);

    if(Result != VK_SUCCESS)
    {
        VENG_ERROR("vkQueueSubmit failed with result: '%s'", VulkanResultString(Result, true));
        return false;
    }

    VulkanCommandBufferUpdateSubmitted(CommandBuffer);

    VulkanSwapchainPresent(&Context, &Context.Swapchain, Context.Device.GraphicsQueue, Context.Device.PresentQueue, Context.QueueCompleteSemaphores[Context.CurrentFrame], Context.ImageIndex);

    return true;
}

s32 FindMemoryIndex(u32 TypeFilter, u32 PropertyFlags)
{
    VkPhysicalDeviceMemoryProperties MemoryProperties;
    vkGetPhysicalDeviceMemoryProperties(Context.Device.PhysicalDevice, &MemoryProperties);

    for(u32 PropertyIndex = 0;
        PropertyIndex < MemoryProperties.memoryTypeCount;
        ++PropertyIndex)
    {
        if(TypeFilter & (1 << PropertyIndex) && (MemoryProperties.memoryTypes[PropertyIndex].propertyFlags & PropertyFlags) == PropertyFlags)
        {
            return PropertyIndex;
        }
    }

    VENG_WARN("Unable to find suitable memory type!");
    return -1;
}

b8 CreateBuffers(vulkan_context* Context)
{
    VkMemoryPropertyFlagBits MemoryPropertyFlags = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;

    const u64 VertexBufferSize = sizeof(vertex_3d) * 1024 * 1024;

    if(!VulkanCreateBuffer(Context, VertexBufferSize, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                          MemoryPropertyFlags,  true, &Context->ObjectVertexBuffer))
    {
        VENG_ERROR("Error creating buffer");
        return false;
    }

    Context->GeometryVertexOffset = 0;

    const u64 IndexBufferSize = sizeof(u32) * 1024 * 1024;

    if(!VulkanCreateBuffer(Context, IndexBufferSize, VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                          MemoryPropertyFlags,  true, &Context->ObjectIndexBuffer))
    {
        VENG_ERROR("Error creating buffer");
        return false;
    }

    Context->GeometryIndexOffset = 0;

    return true;
}

VKAPI_ATTR VkBool32 
DebugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT MessageSeverity,
              VkDebugUtilsMessageTypeFlagsEXT MessageTypes,
              const VkDebugUtilsMessengerCallbackDataEXT* CallbackData,
              void* UserData)
{
    switch(MessageSeverity)
    {
        default:
        case VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT:
            VENG_ERROR(CallbackData->pMessage);
            break;
        case VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT:
            VENG_WARN(CallbackData->pMessage);
            break;
        case VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT:
            VENG_INFO(CallbackData->pMessage);
            break;
        case VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT:
            VENG_TRACE(CallbackData->pMessage);
            break;
    }

    return VK_FALSE;
}

void CreateCommandBuffers(renderer_backend* Backend)
{
    if(!Context.GraphicsCommandBuffers)
    {
        Context.GraphicsCommandBuffers = DArrayReserve(vulkan_command_buffer, Context.Swapchain.ImageCount);
        for(u32 ImageIndex = 0;
            ImageIndex < Context.Swapchain.ImageCount;
            ++ImageIndex)
        {
            ZeroMemory(&Context.GraphicsCommandBuffers[ImageIndex], sizeof(vulkan_command_buffer));
        }
    }

    for(u32 ImageIndex = 0;
        ImageIndex < Context.Swapchain.ImageCount;
        ++ImageIndex)
    {
        if(Context.GraphicsCommandBuffers[ImageIndex].Handle)
        {
            VulkanCommandBufferFree(&Context, Context.Device.GraphicsCommandPool, &Context.GraphicsCommandBuffers[ImageIndex]);
        }

        ZeroMemory(&Context.GraphicsCommandBuffers[ImageIndex], sizeof(vulkan_command_buffer));
        VulkanCommandBufferAllocate(&Context, Context.Device.GraphicsCommandPool, true, &Context.GraphicsCommandBuffers[ImageIndex]);
    }

    VENG_DEBUG("Vulkan command buffers created");
}

void RegenerateFramebuffers(renderer_backend* Backend, vulkan_swapchain* Swapchain, vulkan_renderpass* Renderpass)
{
    for(u32 ImageIndex = 0;
        ImageIndex < Swapchain->ImageCount;
        ++ImageIndex)
    {
        u32 AttachmentCount = 2;
        VkImageView Attachments[] = 
        {
            Swapchain->Views[ImageIndex],
            Swapchain->DepthAttachment.View
        };

        VulkanFramebufferCreate(&Context, Renderpass, Context.FramebufferWidth, Context.FramebufferHeight, AttachmentCount, Attachments, &Context.Swapchain.Framebuffers[ImageIndex]);
    }
}

b8 RecreateSwapchain(renderer_backend* Backend)
{
    if(Context.RecreatingSwapchain)
    {
        VENG_DEBUG("RecreateSwapchain called when already recreating. Booting.");
        return false;
    }

    if(Context.FramebufferWidth == 0 || Context.FramebufferHeight == 0)
    {
        VENG_DEBUG("RecreateSwapchain called when window is < 1 in any dimension. Booting.");
        return false;
    }

    Context.RecreatingSwapchain = true;
    vkDeviceWaitIdle(Context.Device.LogicalDevice);

    for(u32 ImageIndex = 0;
        ImageIndex < Context.Swapchain.ImageCount;
        ++ImageIndex)
    {
        Context.ImagesInFlight[ImageIndex] = 0;
    }

    VulkanDeviceQuerySwapchainSupport(Context.Device.PhysicalDevice, Context.Surface, &Context.Device.SwapchainSupport);
    VulkanDeviceDetectDepthFormat(&Context.Device);

    VulkanSwapchainRecreate(&Context, CachedFramebufferWidth, CachedFramebufferHeight, &Context.Swapchain);

    Context.FramebufferWidth  = CachedFramebufferWidth;
    Context.FramebufferHeight = CachedFramebufferHeight;
    Context.MainRenderpass.Width  = Context.FramebufferWidth;
    Context.MainRenderpass.Height = Context.FramebufferHeight;
    CachedFramebufferWidth  = 0;
    CachedFramebufferHeight = 0;

    Context.FramebufferSizeLastGeneration = Context.FramebufferSizeGeneration;

    for(u32 ImageIndex = 0;
        ImageIndex < Context.Swapchain.ImageCount;
        ++ImageIndex)
    {
        VulkanCommandBufferFree(&Context, Context.Device.GraphicsCommandPool, &Context.GraphicsCommandBuffers[ImageIndex]);
    }

    for(u32 ImageIndex = 0;
        ImageIndex < Context.Swapchain.ImageCount;
        ++ImageIndex)
    {
        VulkanFramebufferDestroy(&Context, &Context.Swapchain.Framebuffers[ImageIndex]);
    }

    Context.MainRenderpass.X = 0;
    Context.MainRenderpass.Y = 0;
    Context.MainRenderpass.Width  = Context.FramebufferWidth;
    Context.MainRenderpass.Height = Context.FramebufferHeight;

    RegenerateFramebuffers(Backend, &Context.Swapchain, &Context.MainRenderpass);
    CreateCommandBuffers(Backend);

    Context.RecreatingSwapchain = false;

    return true;
}

void VulkanCreateTexture(const char* Name, u32 Width, u32 Height, u32 ChannelCount, const u8* Pixels, b8 HasTransparency, texture* OutTexture)
{
    OutTexture->Width  = Width;
    OutTexture->Height = Height;
    OutTexture->ChannelCount = ChannelCount;
    OutTexture->Generation = INVALID_ID;

    OutTexture->Data = (vulkan_texture*)Allocate(sizeof(vulkan_texture), MEMORY_TAG_TEXTURE);
    vulkan_texture* TextureData = (vulkan_texture*)OutTexture->Data;
    VkDeviceSize ImageSize = Width * Height * ChannelCount;

    VkFormat ImageFormat = VK_FORMAT_R8G8B8A8_UNORM;

    VkBufferUsageFlags Usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
    VkMemoryPropertyFlags MemoryProperties = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
    vulkan_buffer Staging;
    VulkanCreateBuffer(&Context, ImageSize, Usage, MemoryProperties, true, &Staging);

    VulkanBufferLoadData(&Context, &Staging, 0, ImageSize, 0, Pixels);

    VulkanImageCreate(&Context, VK_IMAGE_TYPE_2D, Width, Height, ImageFormat, 
                      VK_IMAGE_TILING_OPTIMAL, 
                      VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT, 
                      VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
                      true, VK_IMAGE_ASPECT_COLOR_BIT, &TextureData->Image);

    vulkan_command_buffer TempBuffer;
    VkCommandPool Pool = Context.Device.GraphicsCommandPool;
    VkQueue Queue = Context.Device.GraphicsQueue;
    VulkanCommandBufferAllocateAndBeginSingleUse(&Context, Pool, &TempBuffer);

    VulkanImageTransitionLayout(&Context, &TempBuffer, &TextureData->Image, ImageFormat, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
    VulkanImageCopyFromBuffer(&Context, &TextureData->Image, Staging.Handle, &TempBuffer);
    VulkanImageTransitionLayout(&Context, &TempBuffer, &TextureData->Image, ImageFormat, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
    VulkanCommandBufferEndSingleUse(&Context, Pool, &TempBuffer, Queue);
    VulkanDestroyBuffer(&Context, &Staging);

    VkSamplerCreateInfo SamplerCreateInfo = {VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO};
    SamplerCreateInfo.magFilter = VK_FILTER_LINEAR;
    SamplerCreateInfo.minFilter = VK_FILTER_LINEAR;
    SamplerCreateInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    SamplerCreateInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    SamplerCreateInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    SamplerCreateInfo.anisotropyEnable = VK_TRUE;
    SamplerCreateInfo.maxAnisotropy = 16;
    SamplerCreateInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
    SamplerCreateInfo.unnormalizedCoordinates = VK_FALSE;
    SamplerCreateInfo.compareEnable = VK_FALSE;
    SamplerCreateInfo.compareOp = VK_COMPARE_OP_ALWAYS;
    SamplerCreateInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;

    VkResult SamplerCreateResult = vkCreateSampler(Context.Device.LogicalDevice, &SamplerCreateInfo, Context.Allocator, &TextureData->Sampler);
    if(!VulkanResultIsSuccess(SamplerCreateResult))
    {
        VENG_ERROR("Error creating texture sampler: %s", VulkanResultString(SamplerCreateResult, true));
        return;
    }

    OutTexture->HasTransparency = HasTransparency;
    OutTexture->Generation++;
}

void VulkanDestroyTexture(texture* Texture)
{
    vkDeviceWaitIdle(Context.Device.LogicalDevice);

    vulkan_texture* TextureData = (vulkan_texture*)Texture->Data;

    if(TextureData)
    {
        VulkanImageDestroy(&Context, &TextureData->Image);
        ZeroMemory(&TextureData->Image, sizeof(vulkan_image));
        vkDestroySampler(Context.Device.LogicalDevice, TextureData->Sampler, Context.Allocator);
        TextureData->Sampler = 0;

        Free(Texture->Data, sizeof(vulkan_texture), MEMORY_TAG_TEXTURE);
        ZeroMemory(Texture, sizeof(texture));
    }
}

