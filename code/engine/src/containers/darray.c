
#include "darray.h"

#include "core/vmemory.h"
#include "core/logger.h"


void* _darray_create(u64 Length, u64 Stride)
{
    u64 HeaderSize = DARRAY_FIELD_LENGTH * sizeof(u64);
    u64 ArraySize  = Length * Stride;

    u64* NewArray = (u64*)Allocate(HeaderSize + ArraySize, MEMORY_TAG_DARRAY);
    SetMemory(NewArray, 0, HeaderSize + ArraySize);

    NewArray[DARRAY_CAPACITY] = Length;
    NewArray[DARRAY_LENGTH]   = 0;
    NewArray[DARRAY_STRIDE]   = Stride;

    return (void*)(NewArray + DARRAY_FIELD_LENGTH);
}
void  _darray_destroy(void* Array)
{
    u64* Header = (u64*)Array - DARRAY_FIELD_LENGTH;
    u64 HeaderSize = DARRAY_FIELD_LENGTH * sizeof(u64);
    u64 TotalSize = HeaderSize + Header[DARRAY_CAPACITY] * Header[DARRAY_STRIDE];
    Free(Header, TotalSize, MEMORY_TAG_DARRAY);
}

u64  _darray_field_get(void* Array, u64 Field)
{
    u64* Header = (u64*)Array - DARRAY_FIELD_LENGTH;
    return Header[Field];
}
void _darray_field_set(void* Array, u64 Field, u64 Value)
{
    u64* Header = (u64*)Array - DARRAY_FIELD_LENGTH;
    Header[Field] = Value;
}

void* _darray_resize(void* Array)
{
    u64 Length = DArrayLength(Array);
    u64 Stride = DArrayStride(Array);
    void* Temp = _darray_create((DARRAY_RESIZE_FACTOR * DArrayCapacity(Array)), Stride);

    CopyMemory(Temp, Array, Length * Stride);

    _darray_field_set(Temp, DARRAY_LENGTH, Length);
    _darray_destroy(Array);
    return Temp;
}

void* _darray_push(void* Array, const void* ValuePtr)
{
    u64 Length = DArrayLength(Array);
    u64 Stride = DArrayStride(Array);
    if(Length >= DArrayCapacity(Array))
    {
        Array = _darray_resize(Array);
    }

    u64 Addr = (u64)Array;
    Addr += (Length * Stride);
    CopyMemory((void*)Addr, ValuePtr, Stride);
    _darray_field_set(Array, DARRAY_LENGTH, Length + 1);
    return Array;
}
void  _darray_pop(void* Array, void* Dest)
{
    u64 Length = DArrayLength(Array);
    u64 Stride = DArrayStride(Array);
    u64 Addr = (u64)Array;
    Addr += ((Length - 1) * Stride);
    CopyMemory(Dest, (void*)Addr, Stride);
    _darray_field_set(Array, DARRAY_LENGTH, Length - 1);
}

void* _darray_pop_at(void* Array, u64 Index, void* Dest)
{
    u64 Length = DArrayLength(Array);
    u64 Stride = DArrayStride(Array);
    if(Index >= Length)
    {
        VENG_ERROR("Index is outside of the array! Length: %i, index: %i", Length, Index);
        return Array;
    }

    u64 Addr = (u64)Array;
    CopyMemory(Dest, (void*)(Addr + (Index * Stride)), Stride);

    if(Index != Length - 1)
    {
        CopyMemory((void*)(Addr +  (Index * Stride)), 
                   (void*)(Addr + ((Index - 1) * Stride)), 
                   Stride);
    }

    _darray_field_set(Array, DARRAY_LENGTH, Length - 1);
    return Array;
}
void* _darray_insert_at(void* Array, u64 Index, void* ValuePtr)
{
    u64 Length = DArrayLength(Array);
    u64 Stride = DArrayStride(Array);
    if(Index >= Length)
    {
        VENG_ERROR("Index is outside of the array! Length: %i, index: %i", Length, Index);
        return Array;
    }
    
    if(Length >= DArrayCapacity(Array))
    {
        Array = _darray_resize(Array);
    }

    u64 Addr = (u64)Array;

    if(Index != Length - 1)
    {
        CopyMemory((void*)(Addr + ((Index - 1) * Stride)), 
                   (void*)(Addr +  (Index * Stride)), 
                   Stride * (Length - Index));
    }

    CopyMemory((void*)(Addr + (Index * Stride)), ValuePtr, Stride);
    _darray_field_set(Array, DARRAY_LENGTH, Length + 1);
    return Array;
}

