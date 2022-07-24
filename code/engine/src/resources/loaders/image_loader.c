#include "image_loader.h"

#include "core/logger.h"
#include "core/vmemory.h"
#include "core/vstring.h"
#include "resources/resource_types.h"
#include "systems/resource_system.h"

#define STB_IMAGE_IMPLEMENTATION
#include "vendor/stb_image.h"

b8 ImageLoaderLoad(struct resource_loader* Self, const char* Name, resource* OutResource)
{
    if(!Self || !Name || !OutResource)
    {
        return false;
    }

    const char* FormatStr = "%s/%s/%s%s";
    const s32 RequiredChannelCount = 4;
    stbi_set_flip_vertically_on_load(true);
    char FullFilePath[512];

    StringFormat(FullFilePath, FormatStr, ResourceSystemBasePath(), Self->TypePath, Name, ".png");

    s32 Width;
    s32 Height;
    s32 ChannelCount;

    u8* Data = stbi_load(FullFilePath, &Width, &Height, &ChannelCount, RequiredChannelCount);

    if(Data)
    {
        OutResource->FullPath = StringDuplicate(FullFilePath);

        image_resource_data* ResourceData = Allocate(sizeof(image_resource_data), MEMORY_TAG_TEXTURE);
        ResourceData->Width = Width;
        ResourceData->Height = Height;
        ResourceData->ChannelCount = ChannelCount;
        ResourceData->Pixels = Data;

        OutResource->Data = ResourceData;
        OutResource->DataSize = sizeof(image_resource_data);
        OutResource->Name = Name;

        return true;
    }
    else
    {
        if(stbi_failure_reason())
        {
            VENG_WARN("LoadTexture() failed to load '%s': %s", FullFilePath, stbi_failure_reason());
            stbi__err(0, 0);
        }
        return false;
    }
}

void ImageLoaderUnload(struct resource_loader* Self, resource* Resource)
{
    if(!Self || !Resource)
    {
        return;
    }

    u32 PathLength = StringLength(Resource->FullPath);
    if(PathLength)
    {
        Free(Resource->FullPath, sizeof(char) * PathLength + 1, MEMORY_TAG_STRING);
    }

    if(Resource->Data)
    {
        Free(Resource->Data, Resource->DataSize, MEMORY_TAG_TEXTURE);
        Resource->Data = 0;
        Resource->DataSize = 0;
        Resource->LoaderID = INVALID_ID;
    }
}

resource_loader ImageResourceLoaderCreate()
{
    resource_loader Loader;
    Loader.Type = RESOURCE_TYPE_IMAGE;
    Loader.CustomType = 0;
    Loader.Load = ImageLoaderLoad;
    Loader.Unload = ImageLoaderUnload;
    Loader.TypePath = "textures";

    return Loader;
}

