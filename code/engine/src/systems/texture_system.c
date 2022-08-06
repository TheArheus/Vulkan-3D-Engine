#include "texture_system.h"

#include "core/logger.h"
#include "core/vstring.h"
#include "core/vmemory.h"
#include "containers/hashtable.h"

#include "renderer/renderer_frontend.h"

#include "resources/loaders/image_loader.h"

typedef struct texture_system_state
{
    texture_system_config Config;
    texture DefaultTexture;

    texture* RegisteredTextures;

    hash_table RegisteredTextureTable;
} texture_system_state;

typedef struct texture_reference
{
    u64 ReferenceCount;
    u32 Handle;
    b8 AutoRelease;
} texture_reference;

static texture_system_state* StatePtr = 0;

b8 LoadTexture(const char* TextureName, texture* Texture)
{
    resource ImageResource;
    if(!ResourceSystemLoad(TextureName, RESOURCE_TYPE_IMAGE, &ImageResource))
    {
        VENG_ERROR("Failed to load image resource for texture");
        return false;
    }

    image_resource_data* ResourceData = ImageResource.Data;

    texture TempTexture;
    TempTexture.Width = ResourceData->Width;
    TempTexture.Height = ResourceData->Height;
    TempTexture.ChannelCount = ResourceData->ChannelCount;

    u32 CurrentGeneration = Texture->Generation;
    Texture->Generation = INVALID_ID;

    u64 TotalSize = TempTexture.Width * TempTexture.Height * TempTexture.ChannelCount;
    b32 HasTransparency = false;
    for(u32 i = 0; i < TotalSize; i += TempTexture.ChannelCount)
    {
        u8 a = ResourceData->Pixels[i + 3];
        if(a < 255)
        {
            HasTransparency = true;
            break;
        }
    }

    StringCopyN(TempTexture.Name, TextureName, TEXTURE_NAME_MAX_LENGHT);
    TempTexture.Generation = INVALID_ID;
    TempTexture.HasTransparency = HasTransparency;

    RendererCreateTexture(ResourceData->Pixels, &TempTexture);

    texture Old = *Texture;
    *Texture = TempTexture;

    RendererDestroyTexture(&Old);

    if(CurrentGeneration == INVALID_ID)
    {
        Texture->Generation = 0;
    }
    else
    {
        Texture->Generation = CurrentGeneration + 1;
    }

    ResourceSystemUnload(&ImageResource);
    return true;
}

void DestroyTexture(texture* Texture)
{
    RendererDestroyTexture(Texture);
    ZeroMemory(Texture->Name, sizeof(char) * TEXTURE_NAME_MAX_LENGHT);
    ZeroMemory(Texture, sizeof(texture));

    Texture->ID = INVALID_ID;
    Texture->Generation = INVALID_ID;
}

b8 CreateDefaultTextures(texture_system_state* State);
void DestroyDefaultTexture(texture_system_state* State);

b8 TextureSystemInitialize(u64* MemoryRequirement, void* State, texture_system_config Config)
{
    if(Config.MaxTextureCount == 0)
    {
        VENG_FATAL("TextureSystemInitialize - max texture count must be > 0");
        return false;
    }

    u64 StructRequirement = sizeof(texture_system_state);
    u64 ArrayRequirement = sizeof(texture) * Config.MaxTextureCount;
    u64 HashTableRequirement = sizeof(texture_reference) * Config.MaxTextureCount;
    *MemoryRequirement = StructRequirement + ArrayRequirement + HashTableRequirement;

    if(!State)
    {
        return true;
    }

    StatePtr = State;
    StatePtr->Config = Config;

    void* ArrayBlock = State + StructRequirement;
    StatePtr->RegisteredTextures = ArrayBlock;

    void* HashTableBlock = ArrayBlock + ArrayRequirement;
    HashTableCreate(sizeof(texture_reference), Config.MaxTextureCount, HashTableBlock, false, &StatePtr->RegisteredTextureTable);

    texture_reference InvalidRef;
    InvalidRef.AutoRelease = false;
    InvalidRef.Handle = INVALID_ID;
    InvalidRef.ReferenceCount = 0;
    HashTableFill(&StatePtr->RegisteredTextureTable, &InvalidRef);

    u32 Count = StatePtr->Config.MaxTextureCount;
    for(u32 TextureIndex = 0;
        TextureIndex < Count;
        ++TextureIndex)
    {
        StatePtr->RegisteredTextures[TextureIndex].ID = INVALID_ID;
        StatePtr->RegisteredTextures[TextureIndex].Generation = INVALID_ID;
    }

    CreateDefaultTextures(StatePtr);
    return true;
}

void TextureSystemShutdown(void* State)
{
    if(State)
    {
        for(u32 TextureIndex = 0;
            TextureIndex < StatePtr->Config.MaxTextureCount;
            ++TextureIndex)
        {
            texture* Texture = StatePtr->RegisteredTextures + TextureIndex;
            if(Texture->Generation != INVALID_ID)
            {
                RendererDestroyTexture(Texture);
            }
        }

        DestroyDefaultTexture(State);

        StatePtr = 0;
    }
}

texture* TextureSystemAcquire(const char* Name, b8 AutoRelease)
{
    if(IsStringsEquali(Name, DEFAULT_TEXTURE_NAME))
    {
        VENG_WARN("TextureSystemAcquire called for default name. Use GetDefaultTexture for texture 'default'");
        return &StatePtr->DefaultTexture;
    }

    texture_reference Ref;
    if(StatePtr && HashTableGet(&StatePtr->RegisteredTextureTable, Name, &Ref))
    {
        if(Ref.ReferenceCount == 0)
        {
            Ref.AutoRelease = AutoRelease;
        }

        Ref.ReferenceCount++;
        if(Ref.Handle == INVALID_ID)
        {
            u32 Count = StatePtr->Config.MaxTextureCount;
            texture* Texture = 0;
            for(u32 TextureIndex = 0;
                TextureIndex < Count;
                ++TextureIndex)
            {
                if(StatePtr->RegisteredTextures[TextureIndex].ID == INVALID_ID)
                {
                    Ref.Handle = TextureIndex;
                    Texture = StatePtr->RegisteredTextures + TextureIndex;
                    break;
                }
            }

            if(!Texture || Ref.Handle == INVALID_ID)
            {
                VENG_FATAL("TextureSystemAcquire - Texture system cannot hold anymore textures. Adjust configuration to allow more");
                return 0;
            }

            if(!LoadTexture(Name, Texture))
            {
                VENG_ERROR("Failed to load texture '%s'", Name);
                return 0;
            }

            Texture->ID = Ref.Handle;
        }
        HashTableSet(&StatePtr->RegisteredTextureTable, Name, &Ref);
        return &StatePtr->RegisteredTextures[Ref.Handle];
    }

    VENG_ERROR("TextureSystemAcquire failed to acquire texture '%s'.", Name);
    return 0;
}

void TextureSystemRelease(const char* Name)
{
    if(IsStringsEquali(Name, DEFAULT_TEXTURE_NAME))
    {
        return;
    }

    texture_reference Ref;
    if(StatePtr && HashTableGet(&StatePtr->RegisteredTextureTable, Name, &Ref))
    {
        if(Ref.ReferenceCount == 0)
        {
            VENG_WARN("Trued to release non-existent texture: '%s'", Name);
            return;
        }
        char NameCopy[TEXTURE_NAME_MAX_LENGHT];
        StringCopyN(NameCopy, Name, TEXTURE_NAME_MAX_LENGHT);

        Ref.ReferenceCount--;

        if(Ref.ReferenceCount == 0 && Ref.AutoRelease)
        {
            texture* Texture = &StatePtr->RegisteredTextures[Ref.Handle];

            DestroyTexture(Texture);

            Ref.Handle = INVALID_ID;
            Ref.AutoRelease = false;
        }

        HashTableSet(&StatePtr->RegisteredTextureTable, NameCopy, &Ref);
    }
    else
    {
        VENG_ERROR("TextureSystemRelease failed to release texture: '%s'", Name);
    }
}

texture* TextureSystemGetDefaultTexture()
{
    if(StatePtr)
    {
        return &StatePtr->DefaultTexture;
    }

    VENG_ERROR("TextureSystemGetDefaultTexture called before texture system initialization!");
    return 0;
}

b8 CreateDefaultTextures(texture_system_state* State)
{
    const u32 TexDim = 256;
    const u32 Channels = 4;
    const u32 PixelCount = TexDim * TexDim;
    u8 Pixels[PixelCount * Channels];
    SetMemory(Pixels, 255, sizeof(u8) * PixelCount * Channels);

    for(int row = 0; row < TexDim; ++row)
    {
        for(int col = 0; col < TexDim; ++col)
        {
            if(row % 2)
            {
                if(col % 2)
                {
                    Pixels[(row * TexDim + col)*Channels + 0] = 0;
                    Pixels[(row * TexDim + col)*Channels + 1] = 0;
                }
            }
            else
            {
                if(!(col % 2))
                {
                    Pixels[(row * TexDim + col)*Channels + 0] = 0;
                    Pixels[(row * TexDim + col)*Channels + 1] = 0;
                }
            }
        }
    }

    StringCopyN(StatePtr->DefaultTexture.Name, DEFAULT_TEXTURE_NAME, TEXTURE_NAME_MAX_LENGHT);
    StatePtr->DefaultTexture.Width  = TexDim;
    StatePtr->DefaultTexture.Height = TexDim;
    StatePtr->DefaultTexture.ChannelCount = 4;
    StatePtr->DefaultTexture.Generation = INVALID_ID;
    StatePtr->DefaultTexture.HasTransparency = false;
    RendererCreateTexture(Pixels, &State->DefaultTexture);
    State->DefaultTexture.Generation = INVALID_ID;
    return true;
}

void DestroyDefaultTexture(texture_system_state* State)
{
    if(State)
    {
        DestroyTexture(&State->DefaultTexture);
    }
}

