#include "vulkan_command_buffer.h"
#include "core/vmemory.h"


void VulkanCommandBufferAllocate(vulkan_context* Context, VkCommandPool Pool, b8 IsPrimary, vulkan_command_buffer* OutCommandBuffer)
{
    ZeroMemory(OutCommandBuffer, sizeof(vulkan_command_buffer));

    VkCommandBufferAllocateInfo AllocateInfo = {VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO};
    AllocateInfo.commandPool = Pool;
    AllocateInfo.level = IsPrimary ? VK_COMMAND_BUFFER_LEVEL_PRIMARY : VK_COMMAND_BUFFER_LEVEL_SECONDARY;
    AllocateInfo.commandBufferCount = 1;

    VK_CHECK(vkAllocateCommandBuffers(Context->Device.LogicalDevice, &AllocateInfo, &OutCommandBuffer->Handle));
    OutCommandBuffer->State = COMMAND_BUFFER_STATE_READY;
}

void VulkanCommandBufferFree(vulkan_context* Context, VkCommandPool Pool, vulkan_command_buffer* CommandBuffer)
{
    vkFreeCommandBuffers(Context->Device.LogicalDevice, Pool, 1, &CommandBuffer->Handle);
    CommandBuffer->Handle = 0;
    CommandBuffer->State = COMMAND_BUFFER_STATE_NOT_ALLOCATED;
}


void VulkanCommandBufferBegin(vulkan_command_buffer* CommandBuffer, b8 IsSingleUse, b8 IsRenderPassContinue, b8 IsSimultaneousUse)
{
    VkCommandBufferBeginInfo BeginInfo = {VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO};
    BeginInfo.flags = 0;
    if(IsSingleUse)
    {
        BeginInfo.flags |= VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
    }
    if(IsRenderPassContinue)
    {
        BeginInfo.flags |= VK_COMMAND_BUFFER_USAGE_RENDER_PASS_CONTINUE_BIT;
    }
    if(IsSimultaneousUse)
    {
        BeginInfo.flags |= VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT;
    }

    VK_CHECK(vkBeginCommandBuffer(CommandBuffer->Handle, &BeginInfo));
    CommandBuffer->State = COMMAND_BUFFER_STATE_RECORDING;
}

void VulkanCommandBufferEnd(vulkan_command_buffer* CommandBuffer)
{
    VK_CHECK(vkEndCommandBuffer(CommandBuffer->Handle));
    CommandBuffer->State = COMMAND_BUFFER_STATE_RECORDING_ENDED;
}


void VulkanCommandBufferUpdateSubmitted(vulkan_command_buffer* CommandBuffer)
{
    CommandBuffer->State = COMMAND_BUFFER_STATE_SUBMITTED;
}

void VulkanCommandBufferReset(vulkan_command_buffer* CommandBuffer)
{
    CommandBuffer->State = COMMAND_BUFFER_STATE_READY;
}


void VulkanCommandBufferAllocateAndBeginSingleUse(vulkan_context* Context, VkCommandPool Pool, vulkan_command_buffer* OutCommandBuffer)
{
    VulkanCommandBufferAllocate(Context, Pool, true, OutCommandBuffer);
    VulkanCommandBufferBegin(OutCommandBuffer, true, false, false);
}

void VulkanCommandBufferEndSingleUse(vulkan_context* Context, VkCommandPool Pool, vulkan_command_buffer* CommandBuffer, VkQueue Queue)
{
    VulkanCommandBufferEnd(CommandBuffer);

    VkSubmitInfo SubmitInfo = {VK_STRUCTURE_TYPE_SUBMIT_INFO};
    SubmitInfo.commandBufferCount = 1;
    SubmitInfo.pCommandBuffers = &CommandBuffer->Handle;
    VK_CHECK(vkQueueSubmit(Queue, 1, &SubmitInfo, 0));

    VK_CHECK(vkQueueWaitIdle(Queue));

    VulkanCommandBufferFree(Context, Pool, CommandBuffer);
}

