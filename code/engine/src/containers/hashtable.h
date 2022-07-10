#pragma once

#include "defines.h"

typedef struct hash_table
{
    u64 ElementSize;
    u32 ElementCount;
    b8 IsPointerType;
    void* Memory;
} hash_table;

VENG_API void HashTableCreate(u64 ElementSize, u32 ElementCount, void* Memory, b8 IsPointerType, hash_table* OutHashTable);
VENG_API void HashTableDestroy(hash_table* HashTable);

VENG_API b8 HashTableSet(hash_table* Table, const char* Name, void* Value);
VENG_API b8 HashTableSetPtr(hash_table* Table, const char* Name, void** Value);

VENG_API b8 HashTableGet(hash_table* Table, const char* Name, void* OutValue);
VENG_API b8 HashTableGetPtr(hash_table* Table, const char* Name, void** OutValue);

VENG_API b8 HashTableFill(hash_table* Table, void* Value);
