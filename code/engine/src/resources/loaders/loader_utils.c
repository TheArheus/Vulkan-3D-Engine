#include "loader_utils.h"

#include "core/vmemory.h"
#include "core/logger.h"
#include "core/vstring.h"

b8 ResourceUnload(struct resource_loader* Self, resource* Resource, memory_tag Tag)
{
    if(!Self || !Resource)
    {
        return false;
    }

    u32 PathLength = StringLength(Resource->FullPath);
    if(PathLength)
    {
        Free(Resource->FullPath, sizeof(char) * PathLength + 1, MEMORY_TAG_STRING);
    }

    if(Resource->Data)
    {
        Free(Resource->Data, Resource->DataSize, Tag);
        Resource->Data = 0;
        Resource->DataSize = 0;
        Resource->LoaderID = INVALID_ID;
    }

    return true;
}

