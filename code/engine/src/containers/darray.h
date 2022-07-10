#pragma once

#include "defines.h"

enum 
{
    DARRAY_CAPACITY,
    DARRAY_LENGTH,
    DARRAY_STRIDE,
    DARRAY_FIELD_LENGTH 
};

#define DARRAY_DEFAULT_CAPACITY 1
#define DARRAY_RESIZE_FACTOR 2

VENG_API void* _darray_create(u64 Length, u64 Stride);
VENG_API void  _darray_destroy(void* Array);

VENG_API u64  _darray_field_get(void* Array, u64 Field);
VENG_API void _darray_field_set(void* Array, u64 Field, u64 Value);

VENG_API void* _darray_resize(void* Array);

VENG_API void* _darray_push(void* Array, const void* ValuePtr);
VENG_API void  _darray_pop(void* Array, void* Dest);

VENG_API void* _darray_pop_at(void* Array, u64 Index, void* Dest);
VENG_API void* _darray_insert_at(void* Array, u64 Index, void* ValuePtr);

#define DArrayCreate(Type) _darray_create(DARRAY_DEFAULT_CAPACITY, sizeof(Type))

#define DArrayReserve(Type, Capacity) _darray_create(Capacity, sizeof(Type))

#define DArrayDestroy(Array) _darray_destroy(Array)

#define DArrayPush(Array, Value)                \
    {                                           \
        __typeof__(Value) Temp = Value;           \
        Array = _darray_push(Array, &Temp);     \
    }

#define DArrayPop(Array, ValuePtr) _darray_pop(Array, ValuePtr);

#define DArrayInsertAt(Array, Index, Value)                 \
    {                                                       \
        __typeof__(Value) Temp = Value;                       \
        Array = _darray_insert_at(Array, Index, &Temp);     \
    }

#define DArrayPopAt(Array, Index, ValuePtr) _darray_pop_at(Array, Index, ValuePtr)

#define DArrayClear(Array)                  _darray_field_set(Array, DARRAY_LENGTH, 0)

#define DArrayCapacity(Array)               _darray_field_get(Array, DARRAY_CAPACITY)

#define DArrayLength(Array)                 _darray_field_get(Array, DARRAY_LENGTH)

#define DArrayStride(Array)                 _darray_field_get(Array, DARRAY_STRIDE)

#define DArrayLengthSet(Array, Value)       _darray_field_set(Array, DARRAY_LENGTH, Value)

