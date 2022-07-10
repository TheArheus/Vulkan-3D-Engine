#pragma once

#include "renderer_types.inl"

b8 RendererInitialize(u64* MemoryRequirement, void* State, const char* ApplicationName);
void RendererShutdown(void* State);

void RendererOnResize(u16 Width, u16 Height);

b8 RendererDrawFrame(render_packet* Packet);

VENG_API void RendererSetView(mat4* View);

void RendererCreateTexture(const char* Name, u32 Width, u32 Height, u32 ChannelCount, const u8* Pixels, b8 HasTransparency, texture* OutTexture);
void RendererDestroyTexture(texture* Texture);
