#include "vulkan_device.h"

#include "core/logger.h"
#include "core/vstring.h"
#include "core/vmemory.h"
#include "containers/darray.h"

typedef struct vulkan_physical_device_requirements
{
    b8 Graphics;
    b8 Present;
    b8 Compute;
    b8 Transfer;

    const char** DeviceExtensionNames;
    b8 SamplerAnisotropy;
    b8 DiscreteGPU;
} vulkan_physical_device_requirements;

typedef struct vulkan_physical_device_queue_family_index
{
    u32 GraphicsFamilyIndex;
    u32 PresentFamilyIndex;
    u32 ComputeFamilyIndex;
    u32 TransferFamilyIndex;
} vulkan_physical_device_queue_family_index;


b8 SelectPhysicalDevice(vulkan_context* Context);
b8 PhysicalDeviceMeetsRequirement(VkPhysicalDevice Device, 
                                  VkSurfaceKHR Surface, 
                                  const VkPhysicalDeviceProperties* Properties, 
                                  const VkPhysicalDeviceFeatures* Features, 
                                  const vulkan_physical_device_requirements* Requirements, 
                                  vulkan_physical_device_queue_family_index* QueueInfo, 
                                  vulkan_swapchain_support_info* OutSwapchainSupport);

b8 VulkanDeviceCreate(vulkan_context* Context)
{
    if(!SelectPhysicalDevice(Context))
    {
        return false;
    }

    b8 PresentSharedGraphicsQueue  = Context->Device.GraphicsQueueIndex == Context->Device.PresentQueueIndex;
    b8 TransferSharedGraphicsQueue = Context->Device.GraphicsQueueIndex == Context->Device.TransferQueueIndex;
    u32 IndexCount = 1;
    if(!PresentSharedGraphicsQueue)
    {
        IndexCount++;
    }
    if(!TransferSharedGraphicsQueue)
    {
        IndexCount++;
    }

    u32 Indices[32];
    u8 Index = 0;
    Indices[Index++] = Context->Device.GraphicsQueueIndex;

    if(!PresentSharedGraphicsQueue)
    {
        Indices[Index++] = Context->Device.PresentQueueIndex;
    }
    if(!TransferSharedGraphicsQueue)
    {
        Indices[Index++] = Context->Device.TransferQueueIndex;
    }

    VkDeviceQueueCreateInfo QueueCreateInfos[32];
    for(u32 DeviceIndex = 0;
        DeviceIndex < IndexCount;
        ++DeviceIndex)
    {
        QueueCreateInfos[DeviceIndex].sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        QueueCreateInfos[DeviceIndex].queueFamilyIndex = Indices[DeviceIndex];
        QueueCreateInfos[DeviceIndex].queueCount = 1;
#if 1
        if(Indices[DeviceIndex] == Context->Device.GraphicsQueueIndex)
        {
            QueueCreateInfos[DeviceIndex].queueCount = 2;
        }
#endif
        QueueCreateInfos[DeviceIndex].flags = 0;
        QueueCreateInfos[DeviceIndex].pNext = 0;
        r32 QueuePriority = 1.0f;
        QueueCreateInfos[DeviceIndex].pQueuePriorities = &QueuePriority;
    }

    VkPhysicalDeviceFeatures DeviceFeatures = {};
    DeviceFeatures.samplerAnisotropy = VK_TRUE;

    VkDeviceCreateInfo DeviceCreateInfo = {VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO};
    DeviceCreateInfo.queueCreateInfoCount = IndexCount;
    DeviceCreateInfo.pQueueCreateInfos = QueueCreateInfos;
    DeviceCreateInfo.pEnabledFeatures = &DeviceFeatures;
    DeviceCreateInfo.enabledExtensionCount = 1;
    const char* ExtensionName = VK_KHR_SWAPCHAIN_EXTENSION_NAME;
    DeviceCreateInfo.ppEnabledExtensionNames = &ExtensionName;

    VK_CHECK(vkCreateDevice(Context->Device.PhysicalDevice, &DeviceCreateInfo, Context->Allocator, &Context->Device.LogicalDevice));

    vkGetDeviceQueue(Context->Device.LogicalDevice, Context->Device.GraphicsQueueIndex, 0, &Context->Device.GraphicsQueue);
    vkGetDeviceQueue(Context->Device.LogicalDevice, Context->Device.PresentQueueIndex, 0, &Context->Device.PresentQueue);
    vkGetDeviceQueue(Context->Device.LogicalDevice, Context->Device.TransferQueueIndex, 0, &Context->Device.TransferQueue);

    VkCommandPoolCreateInfo PoolCreateInfo = {VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO};
    PoolCreateInfo.queueFamilyIndex = Context->Device.GraphicsQueueIndex;
    PoolCreateInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
    VK_CHECK(vkCreateCommandPool(Context->Device.LogicalDevice, &PoolCreateInfo, Context->Allocator, &Context->Device.GraphicsCommandPool));

    return true;
}

void VulkanDeviceDestroy(vulkan_context* Context)
{
    Context->Device.GraphicsQueue = 0;
    Context->Device.PresentQueue  = 0;
    Context->Device.TransferQueue = 0;

    vkDestroyCommandPool(Context->Device.LogicalDevice, Context->Device.GraphicsCommandPool, Context->Allocator);

    if(Context->Device.LogicalDevice)
    {
        vkDestroyDevice(Context->Device.LogicalDevice, Context->Allocator);
        Context->Device.LogicalDevice = 0;
    }

    Context->Device.PhysicalDevice = 0;

    if(Context->Device.SwapchainSupport.Formats)
    {
        Free(Context->Device.SwapchainSupport.PresentModes, sizeof(VkPresentModeKHR) * Context->Device.SwapchainSupport.PresentModeCount, MEMORY_TAG_RENDERER);
        Context->Device.SwapchainSupport.PresentModeCount = 0;
        Context->Device.SwapchainSupport.PresentModes = 0;
    }

    ZeroMemory(&Context->Device.SwapchainSupport.Capabilities, sizeof(Context->Device.SwapchainSupport.Capabilities));

    Context->Device.GraphicsQueueIndex = -1;
    Context->Device.PresentQueueIndex  = -1;
    Context->Device.TransferQueueIndex = -1;
}

void VulkanDeviceQuerySwapchainSupport(VkPhysicalDevice PhysicalDevice, VkSurfaceKHR Surface, vulkan_swapchain_support_info* OutSupportInfo)
{
    VK_CHECK(vkGetPhysicalDeviceSurfaceCapabilitiesKHR(PhysicalDevice, Surface, &OutSupportInfo->Capabilities));

    VK_CHECK(vkGetPhysicalDeviceSurfaceFormatsKHR(PhysicalDevice, Surface, &OutSupportInfo->FormatCount, 0));
    if(OutSupportInfo->FormatCount != 0)
    {
        if(!OutSupportInfo->Formats)
        {
            OutSupportInfo->Formats = Allocate(sizeof(VkSurfaceFormatKHR) * OutSupportInfo->FormatCount, MEMORY_TAG_RENDERER);
        }

        VK_CHECK(vkGetPhysicalDeviceSurfaceFormatsKHR(PhysicalDevice, Surface, &OutSupportInfo->FormatCount, OutSupportInfo->Formats));
    }

    VK_CHECK(vkGetPhysicalDeviceSurfacePresentModesKHR(PhysicalDevice, Surface, &OutSupportInfo->PresentModeCount, 0));
    if(OutSupportInfo->PresentModeCount != 0)
    {
        if(!OutSupportInfo->PresentModes)
        {
            OutSupportInfo->PresentModes = Allocate(sizeof(VkPresentModeKHR) * OutSupportInfo->PresentModeCount, MEMORY_TAG_RENDERER);
        }

        VK_CHECK(vkGetPhysicalDeviceSurfacePresentModesKHR(PhysicalDevice, Surface, &OutSupportInfo->PresentModeCount, OutSupportInfo->PresentModes));
    }
}

b8 VulkanDeviceDetectDepthFormat(vulkan_device* Device)
{
    const u64 CandidateCount = 3;
    VkFormat Candidates[3] = 
    {
        VK_FORMAT_D32_SFLOAT,
        VK_FORMAT_D32_SFLOAT_S8_UINT,
        VK_FORMAT_D24_UNORM_S8_UINT
    };

    u32 Flags = VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT;
    for(u64 CandidateIndex = 0;
        CandidateIndex < CandidateCount;
        CandidateIndex++)
    {
        VkFormatProperties Properties;
        vkGetPhysicalDeviceFormatProperties(Device->PhysicalDevice, Candidates[CandidateIndex], &Properties);
        if((Properties.linearTilingFeatures & Flags) == Flags)
        {
            Device->DepthFormat = Candidates[CandidateIndex];
            return true;
        }
        else if((Properties.optimalTilingFeatures & Flags) == Flags)
        {
            Device->DepthFormat = Candidates[CandidateIndex];
            return true;
        }
    }
    return false;
}

b8 SelectPhysicalDevice(vulkan_context* Context)
{
    u32 PhysicalDeviceCount = 0;
    VK_CHECK(vkEnumeratePhysicalDevices(Context->Instance, &PhysicalDeviceCount, 0));

    if(PhysicalDeviceCount == 0)
    {
        VENG_FATAL("No devices which support vulkan found");
        return false;
    }

    VkPhysicalDevice AvailablePhysicalDevices[32];
    VK_CHECK(vkEnumeratePhysicalDevices(Context->Instance, &PhysicalDeviceCount, AvailablePhysicalDevices));

    for(u32 PDeviceIndex = 0;
        PDeviceIndex < PhysicalDeviceCount;
        ++PDeviceIndex)
    {
        VkPhysicalDeviceProperties Properties;
        vkGetPhysicalDeviceProperties(AvailablePhysicalDevices[PDeviceIndex], &Properties);

        VkPhysicalDeviceFeatures Features;
        vkGetPhysicalDeviceFeatures(AvailablePhysicalDevices[PDeviceIndex], &Features);

        VkPhysicalDeviceMemoryProperties MemoryProperties;
        vkGetPhysicalDeviceMemoryProperties(AvailablePhysicalDevices[PDeviceIndex], &MemoryProperties);

        b8 SupportsDeviceLocalHostVisible = false;
        for(u32 MemoryTypeIndex = 0;
            MemoryTypeIndex < MemoryProperties.memoryTypeCount;
            ++MemoryTypeIndex)
        {
            if(((MemoryProperties.memoryTypes[MemoryTypeIndex].propertyFlags & VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT) != 0) && 
               ((MemoryProperties.memoryTypes[MemoryTypeIndex].propertyFlags & VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT) != 0))
            {
                SupportsDeviceLocalHostVisible = true;
                break;
            }
        }

        vulkan_physical_device_requirements Requirements = {};
        Requirements.Graphics = true;
        Requirements.Present = true;
        Requirements.Transfer = true;
        //Requirements.Compute = true;
        Requirements.SamplerAnisotropy = true;
        Requirements.DiscreteGPU = true;
        Requirements.DeviceExtensionNames = DArrayCreate(const char*);
        DArrayPush(Requirements.DeviceExtensionNames, &VK_KHR_SWAPCHAIN_EXTENSION_NAME);

        vulkan_physical_device_queue_family_index QueueInfo = {};

        b8 Result = PhysicalDeviceMeetsRequirement(AvailablePhysicalDevices[PDeviceIndex], Context->Surface, &Properties, &Features, &Requirements, &QueueInfo, &Context->Device.SwapchainSupport);

        if(Result)
        {
            VENG_INFO("Selected device: '%s'.", Properties.deviceName);
            // GPU type, etc.
            switch (Properties.deviceType) 
            {
                default:
                case VK_PHYSICAL_DEVICE_TYPE_OTHER:
                    VENG_INFO("GPU type is Unknown.");
                    break;
                case VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU:
                    VENG_INFO("GPU type is Integrated.");
                    break;
                case VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU:
                    VENG_INFO("GPU type is Descrete.");
                    break;
                case VK_PHYSICAL_DEVICE_TYPE_VIRTUAL_GPU:
                    VENG_INFO("GPU type is Virtual.");
                    break;
                case VK_PHYSICAL_DEVICE_TYPE_CPU:
                    VENG_INFO("GPU type is CPU.");
                    break;
            }

            VENG_INFO(
                "GPU Driver version: %d.%d.%d",
                VK_VERSION_MAJOR(Properties.driverVersion),
                VK_VERSION_MINOR(Properties.driverVersion),
                VK_VERSION_PATCH(Properties.driverVersion));

            // Vulkan API version.
            VENG_INFO(
                "Vulkan API version: %d.%d.%d",
                VK_VERSION_MAJOR(Properties.apiVersion),
                VK_VERSION_MINOR(Properties.apiVersion),
                VK_VERSION_PATCH(Properties.apiVersion));
                
            
            Context->Device.PhysicalDevice = AvailablePhysicalDevices[PDeviceIndex];
            Context->Device.GraphicsQueueIndex = QueueInfo.GraphicsFamilyIndex;
            Context->Device.TransferQueueIndex = QueueInfo.TransferFamilyIndex;
            Context->Device.PresentQueueIndex  = QueueInfo.PresentFamilyIndex;

            Context->Device.Properties = Properties;
            Context->Device.Features   = Features;
            Context->Device.MemoryProperties = MemoryProperties;
            Context->Device.SupportsDeviceLocalHostVisible = SupportsDeviceLocalHostVisible;

            break;
        }
    }

    if(!Context->Device.PhysicalDevice)
    {
        VENG_ERROR("No physical devices were found which meet requrements");
        return false;
    }

    return true;
}

b8 PhysicalDeviceMeetsRequirement(VkPhysicalDevice Device, 
                                  VkSurfaceKHR Surface, 
                                  const VkPhysicalDeviceProperties* Properties, 
                                  const VkPhysicalDeviceFeatures* Features, 
                                  const vulkan_physical_device_requirements* Requirements, 
                                  vulkan_physical_device_queue_family_index* QueueInfo, 
                                  vulkan_swapchain_support_info* OutSwapchainSupport)
{
    QueueInfo->GraphicsFamilyIndex = -1;
    QueueInfo->PresentFamilyIndex = -1;
    QueueInfo->ComputeFamilyIndex = -1;
    QueueInfo->TransferFamilyIndex = -1;

    if(Requirements->DiscreteGPU)
    {
        if(Properties->deviceType != VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU)
        {
            VENG_INFO("Device is not a discrete GPU. Skipping");
            return false;
        }
    }

    u32 QueueFamilyCount = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(Device, &QueueFamilyCount, 0);
    VkQueueFamilyProperties QueueFamilies[32];
    vkGetPhysicalDeviceQueueFamilyProperties(Device, &QueueFamilyCount, QueueFamilies);

    VENG_INFO("Graphics | Present | Compute | Transfer | Name");
    u8 MinTransferScore = 255;
    for(u32 QueueIndex = 0;
        QueueIndex < QueueFamilyCount;
        ++QueueIndex)
    {
        u8 CurrentTransferScore = 0;

        if(QueueFamilies[QueueIndex].queueFlags & VK_QUEUE_GRAPHICS_BIT)
        {
            QueueInfo->GraphicsFamilyIndex = QueueIndex;
            ++CurrentTransferScore;
        }

        if(QueueFamilies[QueueIndex].queueFlags & VK_QUEUE_COMPUTE_BIT)
        {
            QueueInfo->ComputeFamilyIndex = QueueIndex;
            ++CurrentTransferScore;
        }

        if(QueueFamilies[QueueIndex].queueFlags & VK_QUEUE_TRANSFER_BIT)
        {
            if(CurrentTransferScore <= MinTransferScore)
            {
                MinTransferScore = CurrentTransferScore;
                QueueInfo->TransferFamilyIndex = QueueIndex;
            }
        }

        VkBool32 SupportsPresent = VK_FALSE;
        VK_CHECK(vkGetPhysicalDeviceSurfaceSupportKHR(Device, QueueIndex, Surface, &SupportsPresent));
        if(SupportsPresent)
        {
            QueueInfo->PresentFamilyIndex = QueueIndex;
        }
    }

    VENG_INFO("     %d |     %d |     %d |      %d | %s", 
              QueueInfo->GraphicsFamilyIndex != -1,
              QueueInfo->ComputeFamilyIndex  != -1,
              QueueInfo->TransferFamilyIndex != -1,
              QueueInfo->PresentFamilyIndex  != -1,
              Properties->deviceName);

    if((!Requirements->Graphics || (Requirements->Graphics && QueueInfo->GraphicsFamilyIndex != -1)) && 
       (!Requirements->Present  || (Requirements->Present  && QueueInfo->PresentFamilyIndex  != -1)) &&
       (!Requirements->Compute  || (Requirements->Compute  && QueueInfo->ComputeFamilyIndex  != -1)) &&
       (!Requirements->Transfer || (Requirements->Transfer && QueueInfo->TransferFamilyIndex != -1)))
    {
        VulkanDeviceQuerySwapchainSupport(Device, Surface, OutSwapchainSupport);

        if(OutSwapchainSupport->FormatCount < 1 || OutSwapchainSupport->PresentModeCount < 1)
        {
            if(OutSwapchainSupport->Formats)
            {
                Free(OutSwapchainSupport->Formats, sizeof(VkSurfaceFormatKHR) * OutSwapchainSupport->FormatCount, MEMORY_TAG_RENDERER);
            }
            if(OutSwapchainSupport->PresentModes)
            {
                Free(OutSwapchainSupport->PresentModes, sizeof(VkPresentModeKHR) * OutSwapchainSupport->FormatCount, MEMORY_TAG_RENDERER);
            }

            VENG_INFO("Required swapchain support not presented, skipping device");
            return false;
        }

        if(Requirements->DeviceExtensionNames)
        {
            u32 AvailableExtensionCount = 0;
            VkExtensionProperties* AvailableExtensions = 0;
            VK_CHECK(vkEnumerateDeviceExtensionProperties(Device, 0, &AvailableExtensionCount, 0));
            if (AvailableExtensionCount != 0)
            {
                AvailableExtensions = Allocate(sizeof(VkExtensionProperties) * AvailableExtensionCount, MEMORY_TAG_RENDERER);
                VK_CHECK(vkEnumerateDeviceExtensionProperties(Device, 0, &AvailableExtensionCount, AvailableExtensions));

                u32 RequiredExtensionCount = DArrayLength(Requirements->DeviceExtensionNames);
                for (u32 ReqIndex = 0;
                    ReqIndex < RequiredExtensionCount;
                    ++ReqIndex)
                {
                    b8 IsFound = false;
                    for (u32 AvIndex = 0;
                        AvIndex < AvailableExtensionCount;
                        ++AvIndex)
                    {
                        if (IsStringsEqual(Requirements->DeviceExtensionNames[ReqIndex], AvailableExtensions[AvIndex].extensionName))
                        {
                            IsFound = true;
                            break;
                        }
                    }

                    if (!IsFound)
                    {
                        VENG_INFO("Required extension not found: '%s', skipping device", Requirements->DeviceExtensionNames[ReqIndex]);
                        Free(AvailableExtensions, sizeof(VkExtensionProperties) * AvailableExtensionCount, MEMORY_TAG_RENDERER);
                        return false;
                    }
                }
            }

            Free(AvailableExtensions, sizeof(VkExtensionProperties) * AvailableExtensionCount, MEMORY_TAG_RENDERER);
        }

        if (Requirements->SamplerAnisotropy && !Features->samplerAnisotropy)
        {
            VENG_INFO("Device does not support Sampler Anisotropy");
            return false;
        }

        return true;
    }

    return false;
}

