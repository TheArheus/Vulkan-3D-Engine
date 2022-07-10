#include "vulkan_buffer.h"

#include "vulkan_device.h"
#include "vulkan_command_buffer.h"
#include "vulkan_utils.h"

#include "core/logger.h"
#include "core/vmemory.h"


b8 VulkanCreateBuffer(vulkan_context* Context, u64 Size, VkBufferUsageFlagBits Usage, u32 MemoryPropertyFlags, b8 BindOnCreate, vulkan_buffer* OutBuffer)
{
    ZeroMemory(OutBuffer, sizeof(vulkan_buffer));
    OutBuffer->TotalSize = Size;
    OutBuffer->Usage = Usage;
    OutBuffer->MemoryPropertyFlags = MemoryPropertyFlags;

    VkBufferCreateInfo BufferCreateInfo = {VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO};
    BufferCreateInfo.size = Size;
    BufferCreateInfo.usage = Usage;
    BufferCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    VK_CHECK(vkCreateBuffer(Context->Device.LogicalDevice, &BufferCreateInfo, Context->Allocator, &OutBuffer->Handle));

    VkMemoryRequirements Requirements;
    vkGetBufferMemoryRequirements(Context->Device.LogicalDevice, OutBuffer->Handle, &Requirements);
    OutBuffer->MemoryIndex = Context->FindMemoryIndex(Requirements.memoryTypeBits, OutBuffer->MemoryPropertyFlags);
    if(OutBuffer->MemoryIndex == -1)
    {
        VENG_ERROR("Unable to create vulkan buffer because required type index was not found");
        return false;
    }

    VkMemoryAllocateInfo AllocateInfo = {VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO};
    AllocateInfo.allocationSize = Requirements.size;
    AllocateInfo.memoryTypeIndex = (u32)OutBuffer->MemoryIndex;

    VkResult Result = vkAllocateMemory(Context->Device.LogicalDevice, &AllocateInfo, Context->Allocator, &OutBuffer->Memory);
    if(Result != VK_SUCCESS)
    {
        VENG_ERROR("Unable to allocate vulkan buffer memory");
        return false;
    }

    if(BindOnCreate)
    {
        VulkanBufferBind(Context, OutBuffer, 0);
    }

    return true;
}

void VulkanDestroyBuffer(vulkan_context* Context, vulkan_buffer* Buffer)
{
    if(Buffer->Memory)
    {
        vkFreeMemory(Context->Device.LogicalDevice, Buffer->Memory, Context->Allocator);
        Buffer->Memory = 0;
    }
    if(Buffer->Handle)
    {
        vkDestroyBuffer(Context->Device.LogicalDevice, Buffer->Handle, Context->Allocator);
    }
    Buffer->TotalSize = 0;
    Buffer->Usage = 0;
    Buffer->IsLocked = 0;
}

b8 VulkanBufferResize(vulkan_context* Context, u64 NewSize, vulkan_buffer* Buffer, VkQueue Queue, VkCommandPool Pool)
{
    VkBufferCreateInfo BufferCreateInfo = {VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO};
    BufferCreateInfo.size = NewSize;
    BufferCreateInfo.usage = Buffer->Usage;
    BufferCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    VkBuffer NewBuffer;
    VK_CHECK(vkCreateBuffer(Context->Device.LogicalDevice, &BufferCreateInfo, Context->Allocator, &NewBuffer));

    VkMemoryRequirements Requirements;
    vkGetBufferMemoryRequirements(Context->Device.LogicalDevice, NewBuffer, &Requirements);

    VkMemoryAllocateInfo AllocateInfo = {VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO};
    AllocateInfo.allocationSize = Requirements.size;
    AllocateInfo.memoryTypeIndex = (u32)Buffer->MemoryIndex;

    VkDeviceMemory NewMemory;
    VkResult Result = vkAllocateMemory(Context->Device.LogicalDevice, &AllocateInfo, Context->Allocator, &NewMemory);
    if(Result != VK_SUCCESS)
    {
        VENG_ERROR("Unable to allocate vulkan buffer memory");
        return false;
    }

    VK_CHECK(vkBindBufferMemory(Context->Device.LogicalDevice, NewBuffer, NewMemory, 0));

    VulkanBufferCopyTo(Context, Pool, 0, Queue, Buffer->Handle, 0, NewBuffer, 0, Buffer->TotalSize);

    vkDeviceWaitIdle(Context->Device.LogicalDevice);

    if(Buffer->Memory)
    {
        vkFreeMemory(Context->Device.LogicalDevice, Buffer->Memory, Context->Allocator);
        Buffer->Memory = 0;
    }
    if(Buffer->Handle)
    {
        vkDestroyBuffer(Context->Device.LogicalDevice, Buffer->Handle, Context->Allocator);
        Buffer->Handle = 0;
    }

    Buffer->TotalSize = NewSize;
    Buffer->Memory = NewMemory;
    Buffer->Handle = NewBuffer;

    return true;
}

void VulkanBufferBind(vulkan_context* Context, vulkan_buffer* Buffer, u64 Offset)
{
    VK_CHECK(vkBindBufferMemory(Context->Device.LogicalDevice, Buffer->Handle, Buffer->Memory, Offset));
}

void* VulkanBufferLockMemory(vulkan_context* Context, vulkan_buffer* Buffer, u64 Offset, u64 Size, u64 Flags)
{
    void* Data;
    VK_CHECK(vkMapMemory(Context->Device.LogicalDevice, Buffer->Memory, Offset, Size, Flags, &Data));
    return Data;
}

void VulkanBufferUnlockMemory(vulkan_context* Context, vulkan_buffer* Buffer)
{
    vkUnmapMemory(Context->Device.LogicalDevice, Buffer->Memory);
}

void VulkanBufferLoadData(vulkan_context* Context, vulkan_buffer* Buffer, u64 Offset, u64 Size, u32 Flags, const void* Data)
{
    void* DataPtr;
    VK_CHECK(vkMapMemory(Context->Device.LogicalDevice, Buffer->Memory, Offset, Size, Flags, &DataPtr));
    CopyMemory(DataPtr, Data, Size);
    vkUnmapMemory(Context->Device.LogicalDevice, Buffer->Memory);
}

void VulkanBufferCopyTo(vulkan_context* Context, VkCommandPool Pool, 
                        VkFence Fence, VkQueue Queue, 
                        VkBuffer Source, u64 SourceOffset, 
                        VkBuffer Dest, u64 DestOffset, 
                        u64 Size)
{
    vkQueueWaitIdle(Queue);

    vulkan_command_buffer TempCommandBuffer;
    VulkanCommandBufferAllocateAndBeginSingleUse(Context, Pool, &TempCommandBuffer);

    VkBufferCopy CopyRegion;
    CopyRegion.srcOffset = SourceOffset;
    CopyRegion.dstOffset = DestOffset;
    CopyRegion.size = Size;

    vkCmdCopyBuffer(TempCommandBuffer.Handle, Source, Dest, 1, &CopyRegion);

    VulkanCommandBufferEndSingleUse(Context, Pool, &TempCommandBuffer, Queue);
}

