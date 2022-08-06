#include "image_loader.h"

#include "core/logger.h"
#include "core/vmemory.h"
#include "core/vstring.h"
#include "resources/resource_types.h"
#include "systems/resource_system.h"
#include "math/vmath.h"

#include "resources/loaders/loader_utils.h"

#include "platform/file_system.h"

b8 MaterialLoaderLoad(struct resource_loader* Self, const char* Name, resource* OutResource)
{
    if(!Self || !Name || !OutResource)
    {
        return false;
    }
    
    const char* FormatStr = "%s/%s/%s%s";
    char FullFilePath[512];

    StringFormat(FullFilePath, FormatStr, ResourceSystemBasePath(), Self->TypePath, Name, ".vmt");

    file_handle File;
    if(!FileOpen(FullFilePath, FILE_MODE_READ, false, &File))
    {
        VENG_ERROR("LoadConfigurationFile - unable to open file '%'", FullFilePath);
        return false;
    }

    OutResource->FullPath = StringDuplicate(FullFilePath);

    material_config* ResourceData = Allocate(sizeof(material_config), MEMORY_TAG_MATERIAL_INSTANCE);
    ResourceData->AutoRelease = true;
    ResourceData->Type = MATERIAL_TYPE_WORLD;
    ResourceData->DiffuseColor = V4Zero();
    ResourceData->DiffuseMapName[0] = 0;
    StringCopyN(ResourceData->Name, Name, MATERIAL_NAME_MAX_LENGTH);

    char LineBuf[512] = "";
    char* p = &LineBuf[0];
    u64 LineLength = 0;
    u64 LineNumber = 1;
    while(FileReadLine(&File, 511, &p, &LineLength))
    {
        char* Trimmed = StringTrim(LineBuf);
        LineLength = StringLength(Trimmed);

        if(LineLength < 1 || Trimmed[0] == '#')
        {
            LineNumber++;
            continue;
        }

        s32 EqualIndex = StringIndexOf(Trimmed, '=');
        if(EqualIndex == -1)
        {
            VENG_WARN("Potential formatting issue found in file '%s': '=' token not found. Skipping line %ui", FullFilePath, LineNumber);
            continue;
        }

        char RawVarName[64];
        ZeroMemory(RawVarName, sizeof(char) * 64);
        StringMid(RawVarName, Trimmed, 0, EqualIndex);
        char* TrimmedVarName = StringTrim(RawVarName);

        char RawValue[512];
        ZeroMemory(RawValue, sizeof(char) * 512);
        StringMid(RawValue, Trimmed, EqualIndex + 1, -1);
        char* TrimmedValue = StringTrim(RawValue);

        if(IsStringsEquali(TrimmedVarName, "name"))
        {
            StringCopyN(ResourceData->Name, TrimmedValue, MATERIAL_NAME_MAX_LENGTH);
        }
        else if(IsStringsEquali(TrimmedVarName, "diffuse_map_name"))
        {
            StringCopyN(ResourceData->DiffuseMapName, TrimmedValue, MATERIAL_NAME_MAX_LENGTH);
        }
        else if(IsStringsEquali(TrimmedVarName, "diffuse_color"))
        {
            if(!StringToVec4(TrimmedValue, &ResourceData->DiffuseColor))
            {
                VENG_WARN("Error parsing diffuse_color in file '%s'. Using default of white instead.", FullFilePath);
            }
        }
        else if(IsStringsEquali(TrimmedVarName, "type"))
        {
            if(IsStringsEquali(TrimmedValue, "ui"))
            {
                ResourceData->Type = MATERIAL_TYPE_UI;
            }
        }

        ZeroMemory(LineBuf, sizeof(char) * 512);
        LineNumber++;
    }

    FileClose(&File);

    OutResource->Data = ResourceData;
    OutResource->DataSize = sizeof(material_config);
    OutResource->Name = Name;
    return true;
}

void MaterialLoaderUnload(struct resource_loader* Self, resource* Resource)
{
    ResourceUnload(Self, Resource, MEMORY_TAG_MATERIAL_INSTANCE);
}

resource_loader MaterialResourceLoaderCreate()
{
    resource_loader Loader;
    Loader.Type = RESOURCE_TYPE_MATERIAL;
    Loader.CustomType = 0;
    Loader.Load = MaterialLoaderLoad;
    Loader.Unload = MaterialLoaderUnload;
    Loader.TypePath = "materials";

    return Loader;
}

