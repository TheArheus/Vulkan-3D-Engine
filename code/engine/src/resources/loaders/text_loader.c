#include "text_loader.h"

#include "core/logger.h"
#include "core/vmemory.h"
#include "core/vstring.h"
#include "resources/resource_types.h"
#include "systems/resource_system.h"
#include "platform/file_system.h"

b8 TextLoaderLoad(struct resource_loader* Self, const char* Name, resource* OutResource)
{
    if(!Self || !Name || !OutResource)
    {
        return false;
    }

    const char* FormatStr = "%s/%s/%s%s";
    const s32 RequiredChannelCount = 4;
    char FullFilePath[512];

    StringFormat(FullFilePath, FormatStr, ResourceSystemBasePath(), Self->TypePath, Name, "");

    OutResource->FullPath = StringDuplicate(FullFilePath);

    file_handle File;
    FileOpen(FullFilePath, FILE_MODE_READ, false, &File);

    u64 FileTotalSize = 0;
    FileSize(&File, &FileTotalSize);

    char* ResourceData = Allocate(sizeof(char) * FileTotalSize, MEMORY_TAG_ARRAY);
    u64 ReadSize = 0;
    FileReadAllText(&File, ResourceData, &ReadSize);

    FileClose(&File);

    OutResource->Data = ResourceData;
    OutResource->DataSize = ReadSize;
    OutResource->Name = Name;

    return true;
}

void TextLoaderUnload(struct resource_loader* Self, resource* Resource)
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
        Free(Resource->Data, Resource->DataSize, MEMORY_TAG_ARRAY);
        Resource->Data = 0;
        Resource->DataSize = 0;
        Resource->LoaderID = INVALID_ID;
    }
}


resource_loader TextResourceLoaderCreate()
{
    resource_loader Loader;
    Loader.Type = RESOURCE_TYPE_TEXT;
    Loader.CustomType = 0;
    Loader.Load = TextLoaderLoad;
    Loader.Unload = TextLoaderUnload;
    Loader.TypePath = "";

    return Loader;
}

