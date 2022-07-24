#pragma once

#include "renderer/renderer_types.inl"
#include "resources/resource_types.h"

#define DEFAULT_GEOMETRY_NAME "default"

typedef struct geometry_system_config
{
    u32 MaxGeometryCount;
} geometry_system_config;

typedef struct geometry_config
{
    u32 VertexCount;
    vertex_3d* Vertices;
    u32 IndexCount;
    u32* Indices;
    char Name[GEOMETRY_NAME_MAX_LENGTH];
    char MaterialName[MATERIAL_NAME_MAX_LENGTH];
} geometry_config;

b8 GeometrySystemInitialize(u64* MemoryRequirement, void* State, geometry_system_config Config);
void GeometrySystemShutdown(void* State);
geometry* GeometrySystemAcquireByID(u32 ID);
geometry* GeometrySystemAcquireFromConfig(geometry_config Config, b8 AutoRelease);
void GeometrySystemRelease(geometry* Geometry);
geometry_config GeometrySystemGeneratePlaneConfig(r32 Width, r32 Height, u32 SegmentCountX, u32 SegmentCountY, r32 TileX, r32 TileY, const char* Name, const char* MaterialName);
geometry* GeometrySystemGetDefault();
