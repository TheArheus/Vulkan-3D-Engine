#pragma once

#include "renderer/renderer_backend.h"
#include "resources/resource_types.h"

b8 VulkanRendererBackendInitialize(renderer_backend* Backend, const char* ApplicationName);
void VulkanRendererBackendShutdown(renderer_backend* Backend);
void VulkanRendererBackendResized(renderer_backend* Backend, u16 Width, u16 Height);

b8 VulkanRendererBackendBeginFrame(renderer_backend* Backend, r32 DeltaTime);
b8 VulkanRendererBackendEndFrame(renderer_backend* Backend, r32 DeltaTime);

void VulkanRendererUpdateGlobalWorldState(mat4 Projection, mat4 View, v3 ViewPosition, v4 AmbientColor, s32 Mode);
void VulkanRendererUpdateGlobalUiState(mat4 Projection, mat4 View, s32 Mode);

void VulkanDrawGeometry(geometry_render_data RenderData);

void VulkanCreateTexture(const u8* Pixels, texture* Texture);
void VulkanDestroyTexture(texture* Texture);

b8 VulkanRendererBeginRenderpass(renderer_backend* Backend, u8 RenderpassID);
b8 VulkanRendererEndRenderpass(renderer_backend* Backend, u8 RenderpassID);

b8 VulkanRendererCreateMaterial(material* Material);
void VulkanRendererDestroyMaterial(material* Material);

b8 VulkanRendererCreateGeometry(geometry* Geometry, u32 VertexSize, u32 VertexCount, const void* Vertices, u32 IndexSize, u32 IndexCount, const void* Indices);
void VulkanRendererDestroyGeometry(geometry* Geometry);
