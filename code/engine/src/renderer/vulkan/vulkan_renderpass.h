#pragma once

#include "vulkan_types.inl"

void VulkanRenderpassCreate(vulkan_context* Context, vulkan_renderpass* OutRenderpass, 
                            r32 X, r32 Y, 
                            r32 W, r32 H, 
                            r32 R, r32 G, r32 B, r32 A, 
                            r32 Depth, u32 Stencil);

void VulkanRenderpassDestroy(vulkan_context* Context, vulkan_renderpass* Renderpass);

void VulkanRenderpassBegin(vulkan_command_buffer* CommandBuffer, vulkan_renderpass* Renderpass, VkFramebuffer Framebuffer);
void VulkanRenderpassEnd(vulkan_command_buffer* CommandBuffer, vulkan_renderpass* Renderpass);
