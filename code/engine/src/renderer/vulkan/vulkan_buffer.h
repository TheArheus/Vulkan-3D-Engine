#pragma once

#include "vulkan_types.inl"

b8 VulkanCreateBuffer(vulkan_context* Context, u64 Size, VkBufferUsageFlagBits Usage, u32 MemoryPropertyFlags, b8 BindOnCreate, vulkan_buffer* OutBuffer);
void VulkanDestroyBuffer(vulkan_context* Context, vulkan_buffer* Buffer);

b8 VulkanBufferResize(vulkan_context* Context, u64 NewSize, vulkan_buffer* Buffer, VkQueue Queue, VkCommandPool Pool);

void VulkanBufferBind(vulkan_context* Context, vulkan_buffer* Buffer, u64 Offset);

void* VulkanBufferLockMemory(vulkan_context* Context, vulkan_buffer* Buffer, u64 Offset, u64 Size, u64 Flags);
void VulkanBufferUnlockMemory(vulkan_context* Context, vulkan_buffer* Buffer);

void VulkanBufferLoadData(vulkan_context* Context, vulkan_buffer* Buffer, u64 Offset, u64 Size, u32 Flagss, const void* Data);

void VulkanBufferCopyTo(vulkan_context* Context, VkCommandPool Pool, 
                        VkFence Fence, VkQueue Queue, 
                        VkBuffer Source, u64 SourceOffset, 
                        VkBuffer Dest, u64 DestOffset, 
                        u64 Size);

