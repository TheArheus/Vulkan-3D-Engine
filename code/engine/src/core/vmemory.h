#pragma once

#include "defines.h"

typedef enum memory_tag
{
    MEMORY_TAG_UNKNOWN,
    MEMORY_TAG_LINEAR_ALLOCATOR,
    MEMORY_TAG_ARRAY,
    MEMORY_TAG_DARRAY,
    MEMORY_TAG_DICT,
    MEMORY_TAG_RING_QUEUE,
    MEMORY_TAG_BST,
    MEMORY_TAG_STRING,
    MEMORY_TAG_APPLICATION,
    MEMORY_TAG_JOB,
    MEMORY_TAG_TEXTURE,
    MEMORY_TAG_MATERIAL_INSTANCE,
    MEMORY_TAG_RENDERER,
    MEMORY_TAG_GAME,
    MEMORY_TAG_TRANSFORM,
    MEMORY_TAG_ENTITY,
    MEMORY_TAG_ENTITY_NODE,
    MEMORY_TAG_SCENE,

    MEMORY_TAG_MAX_TAGS
} memory_tag;

VENG_API void InitializeMemory(u64* MemoryRequirement, void* State);
VENG_API void ShutdownMemory(void* State);

VENG_API void* Allocate(u64 Size, memory_tag Tag);
VENG_API void  Free(void* Block, u64 Size, memory_tag Tag);
VENG_API void* ZeroMemory(void* Block, u64 Size);
VENG_API void* CopyMemory(void* Dest, const void* Source, u64 Size);
VENG_API void* SetMemory(void* Dest, s32 Value, u64 Size);

VENG_API char* GetMemoryUsageStr();

VENG_API u64 GetMemoryAllocCount();

