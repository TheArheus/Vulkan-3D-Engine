#include "image_loader.h"

#include "core/logger.h"
#include "core/vmemory.h"
#include "core/vstring.h"
#include "resources/resource_types.h"
#include "systems/resource_system.h"
#include "platform/file_system.h"

#include "resources/loaders/loader_utils.h"

b8 BinaryLoaderLoad(struct resource_loader* Self, const char* Name, resource* OutResource)
{
    if(!Self || !Name || !OutResource)
    {
        return false;
    }

    const char* FormatStr = "%s/%s/%s%s";
    char FullFilePath[512];

    StringFormat(FullFilePath, FormatStr, ResourceSystemBasePath(), Self->TypePath, Name, "");

    file_handle File;
    FileOpen(FullFilePath, FILE_MODE_READ, true, &File);

    OutResource->FullPath = StringDuplicate(FullFilePath);

    u64 FileTotalSize = 0;
    FileSize(&File, &FileTotalSize);

    u8* ResourceData = Allocate(sizeof(u8) * FileTotalSize, MEMORY_TAG_ARRAY);
    u64 ReadSize = 0;
    FileReadAllBytes(&File, ResourceData, &ReadSize);

    FileClose(&File);

    OutResource->Data = ResourceData;
    OutResource->DataSize = ReadSize;
    OutResource->Name = Name;

    return true;
}

void BinaryLoaderUnload(struct resource_loader* Self, resource* Resource)
{
    ResourceUnload(Self, Resource, MEMORY_TAG_ARRAY);
}

resource_loader BinaryResourceLoaderCreate()
{
    resource_loader Loader;
    Loader.Type = RESOURCE_TYPE_BINARY;
    Loader.CustomType = 0;
    Loader.Load = BinaryLoaderLoad;
    Loader.Unload = BinaryLoaderUnload;
    Loader.TypePath = "";

    return Loader;
}

