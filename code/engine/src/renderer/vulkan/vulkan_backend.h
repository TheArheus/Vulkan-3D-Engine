#pragma once

#include "renderer/renderer_backend.h"
#include "resources/resource_types.h"

b8 VulkanRendererBackendInitialize(renderer_backend* Backend, const char* ApplicationName);
void VulkanRendererBackendShutdown(renderer_backend* Backend);
void VulkanRendererBackendResized(renderer_backend* Backend, u16 Width, u16 Height);

b8 VulkanRendererBackendBeginFrame(renderer_backend* Backend, r32 DeltaTime);
void VulkanRendererUpdateGlobalState(mat4 Projection, mat4 View, v3 ViewPosition, v4 AmbientColor, s32 Mode);
void VulkanDrawGeometry(geometry_render_data RenderData);
b8 VulkanRendererBackendEndFrame(renderer_backend* Backend, r32 DeltaTime);

void VulkanCreateTexture(const u8* Pixels, texture* Texture);
void VulkanDestroyTexture(texture* Texture);

b8 VulkanRendererCreateMaterial(material* Material);
void VulkanRendererDestroyMaterial(material* Material);

b8 VulkanRendererCreateGeometry(geometry* Geometry, u32 VertexCount, const vertex_3d* Vertices, u32 IndexCount, const u32* Indices);
void VulkanRendererDestroyGeometry(geometry* Geometry);
