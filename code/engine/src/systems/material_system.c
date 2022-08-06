#include "material_system.h"

#include "core/logger.h"
#include "core/vstring.h"
#include "core/vmemory.h"
#include "containers/hashtable.h"
#include "math/vmath.h"
#include "renderer/renderer_frontend.h"
#include "platform/file_system.h"
#include "systems/texture_system.h"

#include "systems/resource_system.h"

typedef struct material_system_state
{
    material_system_config Config;
    material DefaultMaterial;
    material* RegisteredMaterials;
    hash_table RegisteredMaterialTable;
} material_system_state;

typedef struct material_reference
{
    u64 ReferenceCount;
    u32 Handle;
    b8  AutoRelease;
} material_reference;

static material_system_state* StatePtr = 0;

b8 CreateDefaultMaterial(material_system_state* State);
b8 LoadMaterial(material_config Config, material* Mat);
void DestroyMaterial(material* Mat);


b8 MaterialSystemInitialize(u64* MemoryRequirement, void* State, material_system_config Config)
{
    if(Config.MaxMaterialCount == 0)
    {
        return false;
    }

    u64 StructRequirement = sizeof(material_system_state);
    u64 ArrayRequirement = sizeof(material) * Config.MaxMaterialCount;
    u64 HashTableRequirement = sizeof(material_reference) * Config.MaxMaterialCount;

    *MemoryRequirement = StructRequirement + ArrayRequirement + HashTableRequirement;

    if(!State)
    {
        return true;
    }

    StatePtr = State;
    StatePtr->Config = Config;

    void* ArrayBlock = State + StructRequirement;
    StatePtr->RegisteredMaterials = ArrayBlock;

    void* HashTableBlock = ArrayBlock + ArrayRequirement;
    HashTableCreate(sizeof(material_reference), Config.MaxMaterialCount, HashTableBlock, false, &StatePtr->RegisteredMaterialTable);

    material_reference InvalidRef;
    InvalidRef.AutoRelease = false;
    InvalidRef.Handle = INVALID_ID;
    InvalidRef.ReferenceCount = 0;
    HashTableFill(&StatePtr->RegisteredMaterialTable, &InvalidRef);

    u32 Count = StatePtr->Config.MaxMaterialCount;
    for(u32 MaterialIndex = 0;
        MaterialIndex < Count;
        ++MaterialIndex)
    {
        StatePtr->RegisteredMaterials[MaterialIndex].ID = INVALID_ID;
        StatePtr->RegisteredMaterials[MaterialIndex].Generation = INVALID_ID;
        StatePtr->RegisteredMaterials[MaterialIndex].InternalID = INVALID_ID;
    }

    if(!CreateDefaultMaterial(StatePtr))
    {
        return false;
    }

    return true;
}

void MaterialSystemShutdown(void* State)
{
    material_system_state* SystemState = (material_system_state*)State;
    if(SystemState)
    {
        u32 Count = StatePtr->Config.MaxMaterialCount;
        for(u32 MaterialIndex = 0;
            MaterialIndex < Count;
            ++MaterialIndex)
        {
            if(SystemState->RegisteredMaterials[MaterialIndex].ID != INVALID_ID)
            {
                DestroyMaterial(&SystemState->RegisteredMaterials[MaterialIndex]);
            }
        }

        DestroyMaterial(&SystemState->DefaultMaterial);
    }

    StatePtr = 0;
}

material* MaterialSystemAcquire(const char* Name)
{
    resource MaterialResource;
    if(!ResourceSystemLoad(Name, RESOURCE_TYPE_MATERIAL, &MaterialResource))
    {
        VENG_ERROR("Failed to load material resource");
        return 0;
    }

    material* Mat = 0;
    if(MaterialResource.Data)
    {
        Mat = MaterialSystemAcquireFromConfig(*(material_config*)MaterialResource.Data);
    }

    ResourceSystemUnload(&MaterialResource);

    if(!Mat)
    {
        VENG_ERROR("Failed to load material resource");
    }

    return Mat;
}

material* MaterialSystemAcquireFromConfig(material_config Config)
{
    if(IsStringsEquali(Config.Name, DEFAULT_MATERIAL_NAME))
    {
        return &StatePtr->DefaultMaterial;
    }

    material_reference Ref;
    if(StatePtr && HashTableGet(&StatePtr->RegisteredMaterialTable, Config.Name, &Ref))
    {
        if(Ref.ReferenceCount == 0)
        {
            Ref.AutoRelease = Config.AutoRelease;
        }

        Ref.ReferenceCount++;
        if(Ref.Handle == INVALID_ID)
        {
            u32 Count = StatePtr->Config.MaxMaterialCount;
            material* Mat = 0;
            for(u32 MatIndex = 0;
                MatIndex < Count;
                ++MatIndex)
            {
                if(StatePtr->RegisteredMaterials[MatIndex].ID == INVALID_ID)
                {
                    Ref.Handle = MatIndex;
                    Mat = &StatePtr->RegisteredMaterials[MatIndex];
                    break;
                }
            }

            if(!Mat || Ref.Handle == INVALID_ID)
            {
                return 0;
            }

            if(!LoadMaterial(Config, Mat))
            {
                return 0;
            }

            if(Mat->Generation == INVALID_ID)
            {
                Mat->Generation = 0;
            }
            else
            {
                Mat->Generation++;
            }

            Mat->ID = Ref.Handle;
        }

        HashTableSet(&StatePtr->RegisteredMaterialTable, Config.Name, &Ref);
        return &StatePtr->RegisteredMaterials[Ref.Handle];
    }

    return 0;
}

void MaterialSystemRelease(const char* Name)
{
    if(IsStringsEquali(Name, DEFAULT_MATERIAL_NAME))
    {
        return;
    }

    material_reference Ref;
    if(StatePtr && HashTableGet(&StatePtr->RegisteredMaterialTable, Name, &Ref))
    {
        if(Ref.ReferenceCount == 0)
        {
            return;
        }

        Ref.ReferenceCount--;
        if(Ref.ReferenceCount == 0 && Ref.AutoRelease)
        {
            material* Mat = &StatePtr->RegisteredMaterials[Ref.Handle];

            DestroyMaterial(Mat);

            Ref.Handle = INVALID_ID;
            Ref.AutoRelease = false;
        }

        HashTableSet(&StatePtr->RegisteredMaterialTable, Name, &Ref);
    }
}


b8 CreateDefaultMaterial(material_system_state* State)
{
    ZeroMemory(&State->DefaultMaterial, sizeof(material));
    State->DefaultMaterial.ID = INVALID_ID;
    State->DefaultMaterial.Generation = INVALID_ID;
    StringCopyN(State->DefaultMaterial.Name, DEFAULT_MATERIAL_NAME, MATERIAL_NAME_MAX_LENGTH);
    State->DefaultMaterial.DiffuseColor = V4One();
    State->DefaultMaterial.DiffuseMap.Use = TEXTURE_USE_MAP_DIFFUSE;
    State->DefaultMaterial.DiffuseMap.Texture = TextureSystemGetDefaultTexture();

    if(!RendererCreateMaterial(&State->DefaultMaterial))
    {
        VENG_FATAL("Failed to acquire renderer resources for default material.");
        return false;
    }

    return true;
}

b8 LoadMaterial(material_config Config, material* Mat)
{
    ZeroMemory(Mat, sizeof(material));

    StringCopyN(Mat->Name, Config.Name, MATERIAL_NAME_MAX_LENGTH);

    Mat->Type = Config.Type;

    Mat->DiffuseColor = Config.DiffuseColor;

    if(StringLength(Config.DiffuseMapName) > 0)
    {
        Mat->DiffuseMap.Use = TEXTURE_USE_MAP_DIFFUSE;
        Mat->DiffuseMap.Texture = TextureSystemAcquire(Config.DiffuseMapName, true);
        if(!Mat->DiffuseMap.Texture)
        {
            Mat->DiffuseMap.Texture = TextureSystemGetDefaultTexture();
        }
    }
    else
    {
        Mat->DiffuseMap.Use = TEXTURE_USE_UNKNOWN;
        Mat->DiffuseMap.Texture = 0;
    }

    if(!RendererCreateMaterial(Mat))
    {
        VENG_ERROR("Failed to acquire renderer resources for material '%s'", Config.Name);
        return false;
    }

    return true;
}

void DestroyMaterial(material* Mat)
{
    if(Mat->DiffuseMap.Texture)
    {
        TextureSystemRelease(Mat->DiffuseMap.Texture->Name);
    }

    RendererDestroyMaterial(Mat);

    ZeroMemory(Mat, sizeof(material));
    Mat->ID = INVALID_ID;
    Mat->Generation = INVALID_ID;
    Mat->InternalID = INVALID_ID;
}

material* MaterialSystemGetDefault()
{
    if(StatePtr)
    {
        return &StatePtr->DefaultMaterial;
    }

    VENG_FATAL("MaterialSystemGetDefault called before system is initialized");
    return 0;
}

