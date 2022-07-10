#include "vulkan_swapchain.h"
#include "vulkan_image.h"

#include "core/logger.h"
#include "core/vmemory.h"
#include "vulkan_device.h"

void Create(vulkan_context* Context, u32 Width, u32 Height, vulkan_swapchain* Swapchain);
void Destroy(vulkan_context* Context, vulkan_swapchain* Swapchain);

void VulkanSwapchainCreate(vulkan_context* Context, u32 Width, u32 Height, vulkan_swapchain* OutSwapchain)
{
    Create(Context, Width, Height, OutSwapchain);
}

void VulkanSwapchainRecreate(vulkan_context* Context, u32 Width, u32 Height, vulkan_swapchain* OutSwapchain)
{
    Destroy(Context, OutSwapchain);
    Create(Context, Width, Height, OutSwapchain);
}

void VulkanSwapchainDestroy(vulkan_context* Context, vulkan_swapchain* Swapchain)
{
    Destroy(Context, Swapchain);
}

b8 VulkanSwapchainAcquireNextImageIndex(vulkan_context* Context, vulkan_swapchain* Swapchain, u64 TimeoutMs, VkSemaphore ImageAvailableSemaphore, VkFence Fence, u32* OutImageIndex)
{
    VkResult Result = vkAcquireNextImageKHR(Context->Device.LogicalDevice, Swapchain->Handle, TimeoutMs, ImageAvailableSemaphore, Fence, OutImageIndex);
    if(Result == VK_ERROR_OUT_OF_DATE_KHR)
    {
        VulkanSwapchainRecreate(Context, Context->FramebufferWidth, Context->FramebufferHeight, Swapchain);
        return false;
    }
    else if(Result != VK_SUCCESS && Result != VK_SUBOPTIMAL_KHR)
    {
        VENG_FATAL("Failed to acquire swapchain image");
        return false;
    }

    return true;
}

void VulkanSwapchainPresent(vulkan_context* Context, vulkan_swapchain* Swapchain, VkQueue GraphicsQueue, VkQueue PresentQueue, VkSemaphore RenderCompleteSemaphore, u32 PresentImageIndex)
{
    VkPresentInfoKHR PresentInfo = {VK_STRUCTURE_TYPE_PRESENT_INFO_KHR};
    PresentInfo.waitSemaphoreCount = 1;
    PresentInfo.pWaitSemaphores = &RenderCompleteSemaphore;
    PresentInfo.swapchainCount = 1;
    PresentInfo.pSwapchains = &Swapchain->Handle;
    PresentInfo.pImageIndices = &PresentImageIndex;

    VkResult Result = vkQueuePresentKHR(PresentQueue, &PresentInfo);
    if(Result == VK_SUBOPTIMAL_KHR || Result == VK_ERROR_OUT_OF_DATE_KHR)
    {
        VulkanSwapchainRecreate(Context, Context->FramebufferWidth, Context->FramebufferHeight, Swapchain);
    }
    else if(Result != VK_SUCCESS)
    {
        VENG_FATAL("Failed to present swapchain image");
    }

    Context->CurrentFrame = (Context->CurrentFrame + 1) % Swapchain->MaxFramesInFlight;
}

void Create(vulkan_context* Context, u32 Width, u32 Height, vulkan_swapchain* Swapchain)
{
    VkExtent2D SwapchainExtent = {Width, Height};

    b8 Found = false;
    for(u32 FormatIndex = 0;
        FormatIndex < Context->Device.SwapchainSupport.FormatCount;
        FormatIndex++)
    {
        VkSurfaceFormatKHR Format = Context->Device.SwapchainSupport.Formats[FormatIndex];
        if(Format.format == VK_FORMAT_B8G8R8A8_UNORM && Format.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
        {
            Swapchain->ImageFormat = Format;
            Found = true;
            break;
        }
    }

    if(!Found)
    {
        Swapchain->ImageFormat = Context->Device.SwapchainSupport.Formats[0];
    }

    VkPresentModeKHR PresentMode = VK_PRESENT_MODE_FIFO_KHR;
    for(u32 ModeIndex = 0;
        ModeIndex < Context->Device.SwapchainSupport.PresentModeCount;
        ModeIndex++)
    {
        VkPresentModeKHR Mode = Context->Device.SwapchainSupport.PresentModes[ModeIndex];
        if(Mode == VK_PRESENT_MODE_MAILBOX_KHR)
        {
            PresentMode = Mode;
            break;
        }
    }

    VulkanDeviceQuerySwapchainSupport(Context->Device.PhysicalDevice, Context->Surface, &Context->Device.SwapchainSupport);

    if(Context->Device.SwapchainSupport.Capabilities.currentExtent.width != UINT32_MAX)
    {
        SwapchainExtent = Context->Device.SwapchainSupport.Capabilities.currentExtent;
    }

    VkExtent2D Min = Context->Device.SwapchainSupport.Capabilities.minImageExtent;
    VkExtent2D Max = Context->Device.SwapchainSupport.Capabilities.maxImageExtent;
    SwapchainExtent.width  = Clamp(Min.width,  SwapchainExtent.width,  Max.width);
    SwapchainExtent.height = Clamp(Min.height, SwapchainExtent.height, Max.height);

    u32 ImageCount = Context->Device.SwapchainSupport.Capabilities.minImageCount + 1;
    if(Context->Device.SwapchainSupport.Capabilities.maxImageCount > 0 && ImageCount > Context->Device.SwapchainSupport.Capabilities.maxImageCount)
    {
        ImageCount = Context->Device.SwapchainSupport.Capabilities.maxImageCount;
    }

    Swapchain->MaxFramesInFlight = ImageCount - 1;

    VkSwapchainCreateInfoKHR SwapchainCreateInfo = {VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR};
    SwapchainCreateInfo.surface = Context->Surface;
    SwapchainCreateInfo.minImageCount = ImageCount;
    SwapchainCreateInfo.imageFormat = Swapchain->ImageFormat.format;
    SwapchainCreateInfo.imageColorSpace = Swapchain->ImageFormat.colorSpace;
    SwapchainCreateInfo.imageExtent = SwapchainExtent;
    SwapchainCreateInfo.imageArrayLayers = 1;
    SwapchainCreateInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

    if(Context->Device.GraphicsQueueIndex != Context->Device.PresentQueueIndex)
    {
        u32 QueueFamilyIndices[] = 
        {
            (u32)Context->Device.GraphicsQueueIndex,
            (u32)Context->Device.PresentQueueIndex,
        };
        SwapchainCreateInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
        SwapchainCreateInfo.queueFamilyIndexCount = 2;
        SwapchainCreateInfo.pQueueFamilyIndices = QueueFamilyIndices;
    }
    else
    {
        SwapchainCreateInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
        SwapchainCreateInfo.queueFamilyIndexCount = 0;
        SwapchainCreateInfo.pQueueFamilyIndices = 0;
    }

    SwapchainCreateInfo.preTransform = Context->Device.SwapchainSupport.Capabilities.currentTransform;
    SwapchainCreateInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    SwapchainCreateInfo.presentMode = PresentMode;
    SwapchainCreateInfo.clipped = VK_TRUE;
    SwapchainCreateInfo.oldSwapchain = 0; // There should be a swapchain for resize window

    VK_CHECK(vkCreateSwapchainKHR(Context->Device.LogicalDevice, &SwapchainCreateInfo, Context->Allocator, &Swapchain->Handle));

    Context->CurrentFrame = 0;
    Swapchain->ImageCount = 0;

    VK_CHECK(vkGetSwapchainImagesKHR(Context->Device.LogicalDevice, Swapchain->Handle, &Swapchain->ImageCount, 0));
    if(!Swapchain->Images)
    {
        Swapchain->Images = (VkImage*)Allocate(sizeof(VkImage) * Swapchain->ImageCount, MEMORY_TAG_RENDERER);
    }
    if(!Swapchain->Views)
    {
        Swapchain->Views = (VkImageView*)Allocate(sizeof(VkImageView) * Swapchain->ImageCount, MEMORY_TAG_RENDERER);
    }

    VK_CHECK(vkGetSwapchainImagesKHR(Context->Device.LogicalDevice, Swapchain->Handle, &Swapchain->ImageCount, Swapchain->Images));

    for(u32 ImageIndex = 0;
        ImageIndex < Swapchain->ImageCount;
        ImageIndex++)
    {
        VkImageViewCreateInfo ViewInfo = {VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO};
        ViewInfo.image = Swapchain->Images[ImageIndex];
        ViewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
        ViewInfo.format = Swapchain->ImageFormat.format;
        ViewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        ViewInfo.subresourceRange.levelCount = 1;
        ViewInfo.subresourceRange.layerCount = 1;

        VK_CHECK(vkCreateImageView(Context->Device.LogicalDevice, &ViewInfo, Context->Allocator, &Swapchain->Views[ImageIndex]));
    }

    if(!VulkanDeviceDetectDepthFormat(&Context->Device))
    {
        Context->Device.DepthFormat = VK_FORMAT_UNDEFINED;
        VENG_FATAL("Failed to find a supported format!");
    }

    VulkanImageCreate(Context, VK_IMAGE_TYPE_2D, SwapchainExtent.width, SwapchainExtent.height, Context->Device.DepthFormat, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, true, VK_IMAGE_ASPECT_DEPTH_BIT, &Swapchain->DepthAttachment);

    VENG_INFO("Swapchain created successfully");
}

void Destroy(vulkan_context* Context, vulkan_swapchain* Swapchain)
{
    vkDeviceWaitIdle(Context->Device.LogicalDevice);
    VulkanImageDestroy(Context, &Swapchain->DepthAttachment);

    for(u32 ImageIndex = 0;
        ImageIndex < Swapchain->ImageCount;
        ImageIndex++)
    {
        vkDestroyImageView(Context->Device.LogicalDevice, Swapchain->Views[ImageIndex], Context->Allocator);
    }

    vkDestroySwapchainKHR(Context->Device.LogicalDevice, Swapchain->Handle, Context->Allocator);
}

