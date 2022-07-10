#include "vulkan_fence.h"

#include "core/logger.h"

void VulkanFenceCreate(vulkan_context* Context, b8 CreateSignaled, vulkan_fence* OutFence)
{
    OutFence->IsSignaled = CreateSignaled;

    VkFenceCreateInfo CreateInfo = {VK_STRUCTURE_TYPE_FENCE_CREATE_INFO};
    if(OutFence->IsSignaled)
    {
        CreateInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;
    }

    VK_CHECK(vkCreateFence(Context->Device.LogicalDevice, &CreateInfo, Context->Allocator, &OutFence->Handle));
}

void VulkanFenceDestroy(vulkan_context* Context, vulkan_fence* Fence)
{
    if(Fence->Handle)
    {
        vkDestroyFence(Context->Device.LogicalDevice, Fence->Handle, Context->Allocator);
        Fence->Handle = 0;
    }
    Fence->IsSignaled = false;
}

b8 VulkanFenceWait(vulkan_context* Context, vulkan_fence* Fence, u64 TimeoutNs)
{
    if(!Fence->IsSignaled)
    {
        VkResult Result = vkWaitForFences(Context->Device.LogicalDevice, 1, &Fence->Handle, true, TimeoutNs);

        switch(Result)
        {
            case VK_SUCCESS:
                Fence->IsSignaled = true;
                return true;
            case VK_TIMEOUT:
                VENG_WARN("vk_fence_wait - Time Out");
                break;
            case VK_ERROR_DEVICE_LOST:
                VENG_ERROR("vk_fence_wait - VK_ERROR_DEVICE_LOST");
                break;
            case VK_ERROR_OUT_OF_HOST_MEMORY:
                VENG_ERROR("vk_fence_wait - VK_ERROR_OUT_OF_HOST_MEMORY");
                break;
            case VK_ERROR_OUT_OF_DEVICE_MEMORY:
                VENG_ERROR("vk_fence_wait - VK_ERROR_OUT_OF_DEVICE_MEMORY");
                break;
            default:
                VENG_ERROR("vk_fence_wait - An unknown error occured");
                break;
        }
    }
    else
    {
        return true;
    }

    return false;
}

void VulkanFenceReset(vulkan_context* Context, vulkan_fence* Fence)
{
    if(Fence->IsSignaled)
    {
        VK_CHECK(vkResetFences(Context->Device.LogicalDevice, 1, &Fence->Handle));
        Fence->IsSignaled = false;
    }
}

