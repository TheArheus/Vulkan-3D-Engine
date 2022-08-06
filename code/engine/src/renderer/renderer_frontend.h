#pragma once

#include "renderer_types.inl"

b8 RendererInitialize(u64* MemoryRequirement, void* State, const char* ApplicationName);
void RendererShutdown(void* State);

void RendererOnResize(u16 Width, u16 Height);

b8 RendererDrawFrame(render_packet* Packet);

VENG_API void RendererSetView(mat4* View);

void RendererCreateTexture(const u8* Pixels, texture* Texture);
void RendererDestroyTexture(texture* Texture);

b8 RendererCreateMaterial(material* Material);
void RendererDestroyMaterial(material* Material);

b8 RendererCreateGeometry(geometry* Geometry, u32 VertexSize, u32 VertexCount, const void* Vertices, u32 IndexSize, u32 IndexCount, const void* Indices);
void RendererDestroyGeometry(geometry* Geometry);
