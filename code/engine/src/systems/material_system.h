#pragma once

#include "defines.h"

#include "resources/resource_types.h"

#define DEFAULT_MATERIAL_NAME "default"

typedef struct material_system_config
{
    u32 MaxMaterialCount;
} material_system_config;

b8 MaterialSystemInitialize(u64* MemoryRequirement, void* State, material_system_config config);
void MaterialSystemShutdown(void* State);

material* MaterialSystemAcquire(const char* Name);
material* MaterialSystemAcquireFromConfig(material_config Config);
void MaterialSystemRelease(const char* Name);

material* MaterialSystemGetDefault();

