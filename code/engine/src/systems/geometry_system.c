#include "geometry_system.h"

#include "core/logger.h"
#include "core/vmemory.h"
#include "core/vstring.h"
#include "systems/material_system.h"
#include "renderer/renderer_frontend.h"

typedef struct geometry_reference
{
    u64 ReferenceCount;
    geometry Geometry;
    b8 AutoRelease;
} geometry_reference;

typedef struct geometry_system_state
{
    geometry_system_config Config;
    geometry DefaultGeometry;
    geometry DefaultGeometry2d;
    geometry_reference* RegisteredGeometries;
} geometry_system_state;

static geometry_system_state* StatePtr = 0;

b8 CreateDefaultGeometries(geometry_system_state* State);
b8 CreateGeometry(geometry_system_state* State, geometry_config Config, geometry* Geometry);
void DestroyGeometry(geometry_system_state* State, geometry* Geometry);


b8 GeometrySystemInitialize(u64* MemoryRequirement, void* State, geometry_system_config Config)
{
    if(Config.MaxGeometryCount == 0)
    {
        VENG_FATAL("GeometrySystemInitialize - Config.MaxGeometryCount should be > 0");
        return false;
    }

    u64 StructRequirement = sizeof(geometry_system_state);
    u64 ArrayRequirement = sizeof(geometry_reference) * Config.MaxGeometryCount;
    *MemoryRequirement = StructRequirement + ArrayRequirement;

    if(!State)
    {
        return true;
    }

    StatePtr = State;
    StatePtr->Config = Config;

    void* ArrayBlock = State + StructRequirement;
    StatePtr->RegisteredGeometries = ArrayBlock;

    u32 Count = StatePtr->Config.MaxGeometryCount;
    for(u32 Index = 0;
        Index < Count; 
        ++Index)
    {
        StatePtr->RegisteredGeometries[Index].Geometry.ID = INVALID_ID;
        StatePtr->RegisteredGeometries[Index].Geometry.InternalID = INVALID_ID;
        StatePtr->RegisteredGeometries[Index].Geometry.Generation = INVALID_ID;
    }

    if(!CreateDefaultGeometries(StatePtr))
    {
        VENG_FATAL("Could not create default geometries");
        return false;
    }

    return true;
}

void GeometrySystemShutdown(void* State)
{
}

geometry* GeometrySystemAcquireByID(u32 ID)
{
    if(ID != INVALID_ID && StatePtr->RegisteredGeometries[ID].Geometry.ID != INVALID_ID)
    {
        StatePtr->RegisteredGeometries[ID].ReferenceCount++;
        return &StatePtr->RegisteredGeometries[ID].Geometry;
    }

    VENG_ERROR("GeometrySystemAcquireByID cannot load invalid geometry id. Returning nullptr.");
    return 0;
}

geometry* GeometrySystemAcquireFromConfig(geometry_config Config, b8 AutoRelease)
{
    geometry* Geometry = 0;

    for(u32 GeometryIndex = 0;
        GeometryIndex < StatePtr->Config.MaxGeometryCount;
        ++GeometryIndex)
    {
        if(StatePtr->RegisteredGeometries[GeometryIndex].Geometry.ID == INVALID_ID)
        {
            StatePtr->RegisteredGeometries[GeometryIndex].AutoRelease = AutoRelease;
            StatePtr->RegisteredGeometries[GeometryIndex].ReferenceCount = 1;
            Geometry = &StatePtr->RegisteredGeometries[GeometryIndex].Geometry;
            Geometry->ID = GeometryIndex;
            break;
        }
    }

    if(!Geometry)
    {
        VENG_ERROR("Unable to ubtain free slot for geometry.");
        return 0;
    }

    if(!CreateGeometry(StatePtr, Config, Geometry))
    {
        VENG_ERROR("Failed to create geometry. Returning nullptr.");
        return 0;
    }

    return Geometry;
}

void GeometrySystemRelease(geometry* Geometry)
{
    if(Geometry && Geometry->ID != INVALID_ID)
    {
        geometry_reference* Ref = &StatePtr->RegisteredGeometries[Geometry->ID];

        u32 ID = Geometry->ID;
        if(Ref->Geometry.ID == ID)
        {
            if(Ref->ReferenceCount > 0)
            {
                Ref->ReferenceCount--;
            }

            if(Ref->ReferenceCount < 1 && Ref->AutoRelease)
            {
                DestroyGeometry(StatePtr, &Ref->Geometry);
                Ref->ReferenceCount = 0;
                Ref->AutoRelease = false;
            }
        }
        else
        {
            VENG_FATAL("Geometry id mismatch. Check registration logic as this should never be occured.");
        }

        return;
    }

    VENG_WARN("GeometrySystemRelease cannot load invalid geometry id. Nothing was done.");
}

geometry* GeometrySystemGetDefault()
{
    if(StatePtr)
    {
        return &StatePtr->DefaultGeometry;
    }

    VENG_FATAL("No default 3d geometry.");
    return 0;
}

geometry* GeometrySystemGetDefault2d()
{
    if(StatePtr)
    {
        return &StatePtr->DefaultGeometry2d;
    }

    VENG_FATAL("No default 2d geometry.");
    return 0;
}

b8 CreateGeometry(geometry_system_state* State, geometry_config Config, geometry* Geometry)
{
    if(!RendererCreateGeometry(Geometry, Config.VertexSize, Config.VertexCount, Config.Vertices, Config.VertexSize, Config.IndexCount, Config.Indices))
    {
        State->RegisteredGeometries[Geometry->ID].ReferenceCount = 0;
        State->RegisteredGeometries[Geometry->ID].AutoRelease = false;

        Geometry->ID = INVALID_ID;
        Geometry->InternalID = INVALID_ID;
        Geometry->Generation = INVALID_ID;

        return false;
    }

    if(StringLength(Config.MaterialName) > 0)
    {
        Geometry->Material = MaterialSystemAcquire(Config.MaterialName);
        if(!Geometry->Material)
        {
            Geometry->Material = MaterialSystemGetDefault();
        }
    }

    return true;
}

void DestroyGeometry(geometry_system_state* State, geometry* Geometry)
{
    RendererDestroyGeometry(Geometry);
    Geometry->ID = INVALID_ID;
    Geometry->InternalID = INVALID_ID;
    Geometry->Generation = INVALID_ID;

    StringEmpty(Geometry->Name);

    if(Geometry->Material && StringLength(Geometry->Material->Name) > 0)
    {
        MaterialSystemRelease(Geometry->Material->Name);
        Geometry->Material = 0;
    }
}

b8 CreateDefaultGeometries(geometry_system_state* State)
{
    vertex_3d Verts[4];
    ZeroMemory(Verts, 4 * sizeof(vertex_3d));

    const r32 f = 10.0f;
    Verts[0].Position.x = -0.5f * f;
    Verts[0].Position.y = -0.5f * f;
    Verts[0].TexCoord.x =  0.0f;
    Verts[0].TexCoord.y =  0.0f;

    Verts[1].Position.x =  0.5f * f;
    Verts[1].Position.y =  0.5f * f;
    Verts[1].TexCoord.x =  1.0f;
    Verts[1].TexCoord.y =  1.0f;

    Verts[2].Position.x = -0.5f * f;
    Verts[2].Position.y =  0.5f * f;
    Verts[2].TexCoord.x =  0.0f;
    Verts[2].TexCoord.y =  1.0f;

    Verts[3].Position.x =  0.5f * f;
    Verts[3].Position.y = -0.5f * f;
    Verts[3].TexCoord.x =  1.0f;
    Verts[3].TexCoord.y =  0.0f;

    u32 Indices[6] = {0, 1, 2, 0, 3, 1};

    if(!RendererCreateGeometry(&State->DefaultGeometry, sizeof(vertex_3d), 4, Verts, sizeof(u32), 6, Indices))
    {
        VENG_FATAL("Failed to create default 3d geometry. Application cannot continue.");
        return false;
    }

    State->DefaultGeometry.Material = MaterialSystemGetDefault();

    vertex_2d Verts2d[4];
    ZeroMemory(Verts, 4 * sizeof(vertex_2d));

    Verts2d[0].Position.x = -0.5f * f;
    Verts2d[0].Position.y = -0.5f * f;
    Verts2d[0].TexCoord.x =  0.0f;
    Verts2d[0].TexCoord.y =  0.0f;

    Verts2d[1].Position.x =  0.5f * f;
    Verts2d[1].Position.y =  0.5f * f;
    Verts2d[1].TexCoord.x =  1.0f;
    Verts2d[1].TexCoord.y =  1.0f;

    Verts2d[2].Position.x = -0.5f * f;
    Verts2d[2].Position.y =  0.5f * f;
    Verts2d[2].TexCoord.x =  0.0f;
    Verts2d[2].TexCoord.y =  1.0f;

    Verts2d[3].Position.x =  0.5f * f;
    Verts2d[3].Position.y = -0.5f * f;
    Verts2d[3].TexCoord.x =  1.0f;
    Verts2d[3].TexCoord.y =  0.0f;

    u32 Indices2d[6] = {2, 1, 0, 3, 0, 1};

    if(!RendererCreateGeometry(&State->DefaultGeometry2d, sizeof(vertex_2d), 4, Verts2d, sizeof(u32), 6, Indices2d))
    {
        VENG_FATAL("Failed to create default 2d geometry. Application cannot continue.");
        return false;
    }

    State->DefaultGeometry2d.Material = MaterialSystemGetDefault();

    return true;
}

geometry_config GeometrySystemGeneratePlaneConfig(r32 Width, r32 Height, u32 SegmentCountX, u32 SegmentCountY, r32 TileX, r32 TileY, const char* Name, const char* MaterialName)
{
    if(Width == 0)
    {
        Width = 1.0f;
    }

    if(Height == 0)
    {
        Height = 1.0f;
    }

    if(SegmentCountX == 0)
    {
        SegmentCountX = 1;
    }

    if(SegmentCountY == 0)
    {
        SegmentCountY = 1;
    }

    if(TileX == 0)
    {
        TileX = 1.0f;
    }

    if(TileY == 0)
    {
        TileY = 1.0f;
    }

    geometry_config Config;
    Config.VertexSize = sizeof(vertex_3d);
    Config.VertexCount = SegmentCountX * SegmentCountY * 4;
    Config.Vertices = Allocate(sizeof(vertex_3d) * Config.VertexCount, MEMORY_TAG_ARRAY);

    Config.IndexSize = sizeof(u32);
    Config.IndexCount = SegmentCountX * SegmentCountY * 6;
    Config.Indices = Allocate(sizeof(u32) * Config.IndexCount, MEMORY_TAG_ARRAY);

    r32 SegmentWidth  = Width / SegmentCountX;
    r32 SegmentHeight = Height / SegmentCountY;
    r32 HalfWidth = 0.5f * SegmentWidth;
    r32 HalfHeight = 0.5f * SegmentHeight;
    for(u32 y = 0;
        y < SegmentCountY;
        ++y)
    {
        for(u32 x = 0;
            x < SegmentCountX;
            ++x)
        {
            r32 MinX = (x * SegmentWidth) - HalfWidth;
            r32 MinY = (y * SegmentHeight) - HalfHeight;
            r32 MaxX = MinX + SegmentWidth;
            r32 MaxY = MinY + SegmentHeight;

            r32 MinCoordX = (x / (r32)SegmentCountX) * TileX;
            r32 MinCoordY = (y / (r32)SegmentCountY) * TileY;
            r32 MaxCoordX = ((x+1) / (r32)SegmentCountX) * TileX;
            r32 MaxCoordY = ((y+1) / (r32)SegmentCountY) * TileY;

            u32 OffsetV = ((y * SegmentCountY) + x) * 4;
            vertex_3d* v0 = &((vertex_3d*)Config.Vertices)[OffsetV + 0];
            vertex_3d* v1 = &((vertex_3d*)Config.Vertices)[OffsetV + 1];
            vertex_3d* v2 = &((vertex_3d*)Config.Vertices)[OffsetV + 2];
            vertex_3d* v3 = &((vertex_3d*)Config.Vertices)[OffsetV + 3];

            v0->Position.x = MinX;
            v0->Position.y = MinY;
            v0->TexCoord.x = MinCoordX;
            v0->TexCoord.y = MinCoordY;

            v1->Position.x = MaxX;
            v1->Position.y = MaxY;
            v1->TexCoord.x = MaxCoordX;
            v1->TexCoord.y = MaxCoordY;

            v2->Position.x = MinX;
            v2->Position.y = MaxY;
            v2->TexCoord.x = MinCoordX;
            v2->TexCoord.y = MaxCoordY;

            v3->Position.x = MaxX;
            v3->Position.y = MinY;
            v3->TexCoord.x = MaxCoordX;
            v3->TexCoord.y = MinCoordY;

            u32 IndexOffset = ((y * SegmentCountX) + x) * 6;
            ((u32*)Config.Indices)[IndexOffset + 0] = OffsetV + 0;
            ((u32*)Config.Indices)[IndexOffset + 1] = OffsetV + 1;
            ((u32*)Config.Indices)[IndexOffset + 2] = OffsetV + 2;
            ((u32*)Config.Indices)[IndexOffset + 3] = OffsetV + 0;
            ((u32*)Config.Indices)[IndexOffset + 4] = OffsetV + 3;
            ((u32*)Config.Indices)[IndexOffset + 5] = OffsetV + 1;
        }
    }

    if(Name && StringLength(Name) > 0)
    {
        StringCopyN(Config.Name, Name, GEOMETRY_NAME_MAX_LENGTH);
    }
    else
    {
        StringCopyN(Config.Name, DEFAULT_GEOMETRY_NAME, GEOMETRY_NAME_MAX_LENGTH);
    }

    if(MaterialName && StringLength(MaterialName) > 0)
    {
        StringCopyN(Config.MaterialName, MaterialName, GEOMETRY_NAME_MAX_LENGTH);
    }
    else
    {
        StringCopyN(Config.MaterialName, DEFAULT_MATERIAL_NAME, GEOMETRY_NAME_MAX_LENGTH);
    }

    return Config;
}

