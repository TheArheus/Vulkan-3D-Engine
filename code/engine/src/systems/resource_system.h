#pragma once

#include "resources/resource_types.h"

typedef struct resource_system_config
{
    u32 MaxLoaderCount;
    char* AssetBasePath;
} resource_system_config;

typedef struct resource_loader
{
    u32 ID;
    resource_type Type;
    const char* CustomType;
    const char* TypePath;
    b8 (*Load)(struct resource_loader* Self, const char* Name, resource* OutResource);
    void (*Unload)(struct resource_loader* Self, resource* Resource);
} resource_loader;

b8 ResourceSystemInitialize(u64* MemoryRequirement, void* State, resource_system_config Config);
void ResourceSystemShutdown(void* State);

VENG_API b8 ResourceSystemRegisterLoader(resource_loader Loader);

VENG_API b8 ResourceSystemLoad(const char* Name, resource_type Type, resource* OutResource);
VENG_API b8 ResourceSystemLoadCustom(const char* Name, const char* CustomType, resource* OutResource);

VENG_API void ResourceSystemUnload(resource* Resource);

VENG_API const char* ResourceSystemBasePath();

