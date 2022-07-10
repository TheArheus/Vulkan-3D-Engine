
#include "vmemory.h"

#include "core/logger.h"
#include "platform/platform.h"

#include <string.h>
#include <stdio.h>

typedef struct memory_stats
{
    u64 TotalAllocated;
    u64 TaggedAllocation[MEMORY_TAG_MAX_TAGS];
} memory_stats;

static const char* MemoryTagStrings[MEMORY_TAG_MAX_TAGS] = 
{
    "UNKNOWN",
    "MEMORY_TAG_LINEAR_ALLOCATOR",
    "ARRAY",
    "DARRAY",
    "DICT",
    "RING_QUEUE",
    "BST",
    "STRING",
    "APPLICATION",
    "JOB",
    "TEXTURE",
    "MATERIAL_INSTANCE",
    "RENDERER",
    "GAME",
    "TRANSFORM",
    "ENTITY",
    "ENTITY_NODE",
    "SCENE"
};

typedef struct memory_system_state
{
    memory_stats Stats;
    u64 AllocCount;
} memory_system_state;

static memory_system_state* MemoryState;

void InitializeMemory(u64* MemoryRequirement, void* State)
{
    *MemoryRequirement = sizeof(memory_system_state);
    if(State == 0)
    {
        return;
    }

    MemoryState = State;
    MemoryState->AllocCount = 0;
    PlatformZeroMemory(&MemoryState->Stats, sizeof(memory_stats));
}

void ShutdownMemory(void* State)
{
    MemoryState = 0;
}

void* Allocate(u64 Size, memory_tag Tag)
{
    if(Tag == MEMORY_TAG_UNKNOWN)
    {
        VENG_WARN("Allocate called using MEMORY_TAG_UNKNOWN. Re-class this allocation.");
    }

    if(MemoryState)
    {
        MemoryState->Stats.TotalAllocated += Size;
        MemoryState->Stats.TaggedAllocation[Tag] += Size;
        MemoryState->AllocCount++;
    }

    void* Block = PlatformAllocate(Size, false);
    PlatformZeroMemory(Block, Size);
    return Block;
}

void Free(void* Block, u64 Size, memory_tag Tag)
{
    if(Tag == MEMORY_TAG_UNKNOWN)
    {
        VENG_WARN("Free called using MEMORY_TAG_UNKNOWN. Re-class this allocation.");
    }

    if(MemoryState)
    {
        MemoryState->Stats.TotalAllocated -= Size;
        MemoryState->Stats.TaggedAllocation[Tag] -= Size;
    }

    PlatformFree(Block, false);
}

void* ZeroMemory(void* Block, u64 Size)
{
    return PlatformZeroMemory(Block, Size);
}

void* CopyMemory(void* Dest, const void* Source, u64 Size)
{
    return PlatformCopyMemory(Dest, Source, Size);
}

void* SetMemory(void* Dest, s32 Value, u64 Size)
{
    return PlatformSetMemory(Dest, Value, Size);
}

char* GetMemoryUsageStr()
{
    const u64 Gb = 1024 * 1024 * 1024;
    const u64 Mb = 1024 * 1024;
    const u64 Kb = 1024;

    char Buffer[8000] = "System memory use (tagged):\n";
    u64 Offset = strlen(Buffer);

    for(u32 MemoryIndex = 0;
        MemoryIndex < MEMORY_TAG_MAX_TAGS;
        ++MemoryIndex)
    {
        char Unit[4] = "XiB";
        r32 Ammount = 1.0f;
        if(MemoryState->Stats.TaggedAllocation[MemoryIndex] >= Gb)
        {
            Unit[0] = 'G';
            Ammount = MemoryState->Stats.TaggedAllocation[MemoryIndex] / (float)Gb;
        }
        else if(MemoryState->Stats.TaggedAllocation[MemoryIndex] >= Mb)
        {
            Unit[0] = 'M';
            Ammount = MemoryState->Stats.TaggedAllocation[MemoryIndex] / (float)Mb;
        }
        else if(MemoryState->Stats.TaggedAllocation[MemoryIndex] >= Kb)
        {
            Unit[0] = 'K';
            Ammount = MemoryState->Stats.TaggedAllocation[MemoryIndex] / (float)Kb;
        }
        else
        {
            Unit[0] = 'B';
            Unit[1] = 0;
            Ammount = (float)MemoryState->Stats.TaggedAllocation[MemoryIndex];
        }

        s32 Length = snprintf(Buffer + Offset, 8000, "  %s: %.2f%s\n", MemoryTagStrings[MemoryIndex], Ammount, Unit);
        Offset += Length;
    }

    char* OutString = _strdup(Buffer);
    return OutString;
}

VENG_API u64 GetMemoryAllocCount()
{
    if(MemoryState)
    {
        return MemoryState->AllocCount;
    }

    return 0;
}

