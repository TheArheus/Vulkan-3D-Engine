#include "resource_system.h"

#include "core/logger.h"
#include "core/vstring.h"

#include "resources/loaders/image_loader.h"
#include "resources/loaders/material_loader.h"
#include "resources/loaders/binary_loader.h"
#include "resources/loaders/text_loader.h"

typedef struct resource_system_state
{
    resource_system_config Config;
    resource_loader* RegisteredLoaders;
} resource_system_state;

static resource_system_state* StatePtr = 0;

b8 Load(const char* Name, resource_loader* Loader, resource* OutResource);

b8 ResourceSystemInitialize(u64* MemoryRequirement, void* State, resource_system_config Config)
{
    if(Config.MaxLoaderCount == 0)
    {
        return false;
    }

    u64 StructRequirement = sizeof(resource_system_state);
    u64 ArrayRequirement = sizeof(resource_loader) * Config.MaxLoaderCount;
    *MemoryRequirement = StructRequirement + ArrayRequirement;

    if(!State)
    {
        return true;
    }

    StatePtr = State;
    StatePtr->Config = Config;

    void* ArrayBlock = StatePtr + StructRequirement;
    StatePtr->RegisteredLoaders = ArrayBlock;

    u32 Count = StatePtr->Config.MaxLoaderCount;
    for(u32 LoaderIndex = 0;
        LoaderIndex < Count;
        ++LoaderIndex)
    {
        StatePtr->RegisteredLoaders[LoaderIndex].ID = INVALID_ID;
    }

    ResourceSystemRegisterLoader(ImageResourceLoaderCreate());
    ResourceSystemRegisterLoader(MaterialResourceLoaderCreate());
    ResourceSystemRegisterLoader(BinaryResourceLoaderCreate());
    ResourceSystemRegisterLoader(TextResourceLoaderCreate());

    return true;
}

void ResourceSystemShutdown(void* State)
{
    if(StatePtr)
    {
        StatePtr = 0;
    }
}

b8 ResourceSystemRegisterLoader(resource_loader Loader)
{
    if(StatePtr)
    {
        u32 Count = StatePtr->Config.MaxLoaderCount;
        for(u32 LoaderIndex = 0;
            LoaderIndex < Count;
            ++LoaderIndex)
        {
            resource_loader* L = StatePtr->RegisteredLoaders + LoaderIndex;
            if(L->ID != INVALID_ID)
            {
                if(Loader.Type == L->Type)
                {
                    return false;
                }
                else if(Loader.CustomType && StringLength(Loader.CustomType) > 0 && IsStringsEquali(L->CustomType, Loader.CustomType))
                {
                    return false;
                }
            }
        }

        for(u32 LoaderIndex = 0;
            LoaderIndex < Count;
            ++LoaderIndex)
        {
            if(StatePtr->RegisteredLoaders[LoaderIndex].ID == INVALID_ID)
            {
                StatePtr->RegisteredLoaders[LoaderIndex] = Loader;
                StatePtr->RegisteredLoaders[LoaderIndex].ID = LoaderIndex;
                return true;
            }
        }
    }

    return false;
}

b8 ResourceSystemLoad(const char* Name, resource_type Type, resource* OutResource)
{
    if(StatePtr && Type != RESOURCE_TYPE_CUSTOM)
    {
        u32 Count = StatePtr->Config.MaxLoaderCount;
        for(u32 LoaderIndex = 0;
            LoaderIndex < Count;
            ++LoaderIndex)
        {
            resource_loader* L = StatePtr->RegisteredLoaders + LoaderIndex;
            if(L->ID != INVALID_ID && L->Type == Type)
            {
                return Load(Name, L, OutResource);
            }
        }
    }

    OutResource->LoaderID = INVALID_ID;
    return false;
}

b8 ResourceSystemLoadCustom(const char* Name, const char* CustomType, resource* OutResource)
{
    if(StatePtr && CustomType && StringLength(CustomType) > 0)
    {
        u32 Count = StatePtr->Config.MaxLoaderCount;
        for(u32 LoaderIndex = 0;
            LoaderIndex < Count;
            ++LoaderIndex)
        {
            resource_loader* L = StatePtr->RegisteredLoaders + LoaderIndex;
            if(L->ID != INVALID_ID && L->Type == RESOURCE_TYPE_CUSTOM && IsStringsEquali(L->CustomType, CustomType))
            {
                return Load(Name, L, OutResource);
            }
        }
    }

    OutResource->LoaderID = INVALID_ID;
    return false;
}

void ResourceSystemUnload(resource* Resource)
{
    if(StatePtr && Resource)
    {
        if(Resource->LoaderID != INVALID_ID)
        {
            resource_loader* L = &StatePtr->RegisteredLoaders[Resource->LoaderID];
            if(L->ID != INVALID_ID && L->Unload)
            {
                L->Unload(L, Resource);
            }
        }
    }
}

const char* ResourceSystemBasePath()
{
    if(StatePtr)
    {
        return StatePtr->Config.AssetBasePath;
    }

    return "";
}

b8 Load(const char* Name, resource_loader* Loader, resource* OutResource)
{
    if(!Name || !Loader || !Loader->Load || !OutResource)
    {
        OutResource->LoaderID = INVALID_ID;
        return false;
    }

    OutResource->LoaderID = Loader->ID;
    return Loader->Load(Loader, Name, OutResource);
}

