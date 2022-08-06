#pragma once

#include "math/math_types.h"

#define TEXTURE_NAME_MAX_LENGHT  512
#define MATERIAL_NAME_MAX_LENGTH 256
#define GEOMETRY_NAME_MAX_LENGTH 256

typedef enum resource_type
{
    RESOURCE_TYPE_TEXT,
    RESOURCE_TYPE_BINARY,
    RESOURCE_TYPE_IMAGE,
    RESOURCE_TYPE_MATERIAL,
    RESOURCE_TYPE_STATIC_MESH,
    RESOURCE_TYPE_CUSTOM,
} resource_type;

typedef struct resource
{
    u32 LoaderID;
    const char* Name;
    char* FullPath;
    u64 DataSize;
    void* Data;
} resource;

typedef struct image_resource_data
{
    u8 ChannelCount;
    u32 Width;
    u32 Height;
    u8* Pixels;
} image_resource_data;

typedef struct texture
{
    u32 ID;
    u32 Width;
    u32 Height;
    u8  ChannelCount;
    u8  HasTransparency;
    u32 Generation;
    char Name[TEXTURE_NAME_MAX_LENGHT];

    void* Data;
} texture;

typedef enum texture_use
{
    TEXTURE_USE_UNKNOWN = 0x00,
    TEXTURE_USE_MAP_DIFFUSE = 0x01,
} texture_use;

typedef struct texture_map
{
    texture* Texture;
    texture_use Use;
} texture_map;

typedef enum material_type
{
    MATERIAL_TYPE_WORLD,
    MATERIAL_TYPE_UI,
} material_type;

typedef struct material
{
    u32 ID;
    u32 Generation;
    u32 InternalID;
    material_type Type;
    char Name[MATERIAL_NAME_MAX_LENGTH];
    v4 DiffuseColor;
    texture_map DiffuseMap;
} material;

typedef struct material_config
{
    char Name[MATERIAL_NAME_MAX_LENGTH];
    material_type Type;
    b8 AutoRelease;
    v4 DiffuseColor;
    char DiffuseMapName[TEXTURE_NAME_MAX_LENGHT];
} material_config;

typedef struct geometry
{
    u32 ID;
    u32 InternalID;
    u32 Generation;
    char Name[GEOMETRY_NAME_MAX_LENGTH];
    material* Material;
} geometry;

