#include "linear_allocator.h"

#include "core/vmemory.h"
#include "core/logger.h"

void LinearAllocatorCreate(u64 TotalSize, void* Memory, linear_allocator* OutAllocator)
{
    if(OutAllocator)
    {
        OutAllocator->TotalSize = TotalSize;
        OutAllocator->Allocated = 0;
        OutAllocator->OwnsMemory = Memory == 0;
        if(Memory)
        {
            OutAllocator->Memory = Memory;
        }
        else
        {
            OutAllocator->Memory = Allocate(TotalSize, MEMORY_TAG_LINEAR_ALLOCATOR);
        }
    }
}

void LinearAllocatorDestroy(linear_allocator* Allocator)
{
    if(Allocator)
    {
        Allocator->Allocated = 0;
        if(Allocator->OwnsMemory && Allocator->Memory)
        {
            Free(Allocator->Memory, Allocator->TotalSize, MEMORY_TAG_LINEAR_ALLOCATOR);
        }

        Allocator->Memory = 0;
        Allocator->TotalSize = 0;
        Allocator->OwnsMemory = false;
    }
}

void* LinearAllocatorAllocate(linear_allocator* Allocator, u64 Size)
{
    if(Allocator && Allocator->Memory)
    {
        if(Allocator->Allocated + Size > Allocator->TotalSize)
        {
            u64 Remaining = Allocator->TotalSize - Allocator->Allocated;
            VENG_ERROR("Linear allocator Allocate - tried to allocate %lluB with only %llu remaining.", Size, Remaining);
            return 0;
        }

        void* Block = ((u8*)Allocator->Memory) + Allocator->Allocated;
        Allocator->Allocated += Size;
        return Block;
    }

    VENG_ERROR("Linear allocator Allocate - provided allocator does not initialized");
    return 0;
}

void  LinearAllocatorFreeAll(linear_allocator* Allocator)
{
    if(Allocator && Allocator->Memory)
    {
        Allocator->Allocated = 0;
        ZeroMemory(Allocator->Memory, Allocator->TotalSize);
    }
}

