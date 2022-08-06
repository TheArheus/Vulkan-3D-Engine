#pragma once

#include "vulkan_types.inl"

typedef enum renderpass_clear_flag
{
    RENDERPASS_CLEAR_NONE_FLAG = 0x0,
    RENDERPASS_CLEAR_COLOR_BUFFER_FLAG = 0x1,
    RENDERPASS_CLEAR_DEPTH_BUFFER_FLAG = 0x2,
    RENDERPASS_CLEAR_STENCIL_BUFFER_FLAG = 0x4,
} renderpass_clear_flag;

void VulkanRenderpassCreate(vulkan_context* Context, vulkan_renderpass* OutRenderpass, 
                            v4 RenderArea, v4 Color, 
                            r32 Depth, u32 Stencil, u8 Flags, b8 HasPrevPass, b8 HasNextPass);

void VulkanRenderpassDestroy(vulkan_context* Context, vulkan_renderpass* Renderpass);

void VulkanRenderpassBegin(vulkan_command_buffer* CommandBuffer, vulkan_renderpass* Renderpass, VkFramebuffer Framebuffer);
void VulkanRenderpassEnd(vulkan_command_buffer* CommandBuffer, vulkan_renderpass* Renderpass);
