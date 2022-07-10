#pragma once

#include "defines.h"

typedef struct linear_allocator
{
    u64 TotalSize;
    u64 Allocated;
    void* Memory;
    b8 OwnsMemory;
} linear_allocator;


VENG_API void LinearAllocatorCreate(u64 TotalSize, void* Memory, linear_allocator* OutAllocator);
VENG_API void LinearAllocatorDestroy(linear_allocator* OutAllocator);

VENG_API void* LinearAllocatorAllocate(linear_allocator* Allocator, u64 Size);
VENG_API void  LinearAllocatorFreeAll(linear_allocator* Allocator);
