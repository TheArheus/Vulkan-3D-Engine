#pragma once

#include "vulkan_types.inl"

void VulkanImageCreate(vulkan_context* Context, 
                       VkImageType ImageType, 
                       u32 Width, u32 Height, 
                       VkFormat Format, VkImageTiling Tiling, 
                       VkImageUsageFlags Usage, VkMemoryPropertyFlags MemoryFlags,
                       b32 CreateView,
                       VkImageAspectFlags ViewAspectFlags, 
                       vulkan_image* OutImage);

void VulkanImageViewCreate(vulkan_context* Context, VkFormat Format, vulkan_image* Image, VkImageAspectFlags AspectFlags);
void VulkanImageDestroy(vulkan_context* Context, vulkan_image* Image);

void VulkanImageTransitionLayout(vulkan_context* Context, vulkan_command_buffer* CommandBuffer, vulkan_image* Image, VkFormat Format, VkImageLayout OldLayout, VkImageLayout NewLayout);
void VulkanImageCopyFromBuffer(vulkan_context* Context, vulkan_image* Image, VkBuffer Buffer, vulkan_command_buffer* CommandBuffer);
