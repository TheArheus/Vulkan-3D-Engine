#pragma once

#include "renderer/renderer_types.inl"

typedef struct texture_system_config
{
    u32 MaxTextureCount;
} texture_system_config;

#define DEFAULT_TEXTURE_NAME "default"

b8 TextureSystemInitialize(u64* MemoryRequirement, void* State, texture_system_config config);
void TextureSystemShutdown(void* State);

texture* TextureSystemAcquire(const char* Name, b8 AutoRelease);
void TextureSystemRelease(const char* Name);

texture* TextureSystemGetDefaultTexture();
