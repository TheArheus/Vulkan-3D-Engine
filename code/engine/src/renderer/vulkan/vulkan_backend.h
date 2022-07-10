#pragma once

#include "renderer/renderer_backend.h"
#include "resources/resource_types.h"

b8 VulkanRendererBackendInitialize(renderer_backend* Backend, const char* ApplicationName);
void VulkanRendererBackendShutdown(renderer_backend* Backend);
void VulkanRendererBackendResized(renderer_backend* Backend, u16 Width, u16 Height);

b8 VulkanRendererBackendBeginFrame(renderer_backend* Backend, r32 DeltaTime);
void VulkanRendererUpdateGlobalState(mat4 Projection, mat4 View, v3 ViewPosition, v4 AmbientColor, s32 Mode);
void VulkanUpdateObject(geometry_render_data RenderData);
b8 VulkanRendererBackendEndFrame(renderer_backend* Backend, r32 DeltaTime);

void VulkanCreateTexture(const char* Name, u32 Width, u32 Height, u32 ChannelCount, const u8* Pixels, b8 HasTransparency, texture* OutTexture);
void VulkanDestroyTexture(texture* Texture);

