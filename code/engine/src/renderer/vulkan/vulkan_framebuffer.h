#pragma once

#include "vulkan_types.inl"

void VulkanFramebufferCreate(vulkan_context* Context, vulkan_renderpass* Renderpass, u32 Width, u32 Height, u32 AttachmentCount, VkImageView* Attachments, vulkan_framebuffer* OutFramebuffer);
void VulkanFramebufferDestroy(vulkan_context* Context, vulkan_framebuffer* Framebuffer);
