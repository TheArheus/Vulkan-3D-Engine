#include "hashtable.h"

#include "core/vmemory.h"
#include "core/logger.h"


u64 HashName(const char* Name, u32 ElementCount)
{
    static const u64 Multiplier = 97;

    unsigned const char* us;
    u64 Hash = 0;
    for(us = (unsigned const char*)Name; *us; us++)
    {
        Hash = Hash * Multiplier + *us;
    }

    Hash %= ElementCount;
}

VENG_API void HashTableCreate(u64 ElementSize, u32 ElementCount, void* Memory, b8 IsPointerType, hash_table* OutHashTable)
{
    if(!Memory || !OutHashTable)
    {
        VENG_ERROR("Hash table creation failed! Pointer to memory and output hash table required.");
        return;
    }
    if(!ElementCount || !ElementSize)
    {
        VENG_ERROR("Hash table creation failed! No element size or element count specified.");
        return;
    }

    OutHashTable->Memory = Memory;
    OutHashTable->ElementCount = ElementCount;
    OutHashTable->ElementSize = ElementSize;
    OutHashTable->IsPointerType = IsPointerType;
    ZeroMemory(OutHashTable->Memory, ElementSize * ElementCount);
}

VENG_API void HashTableDestroy(hash_table* HashTable)
{
    if(HashTable)
    {
        ZeroMemory(HashTable, sizeof(hash_table));
    }
}

VENG_API b8 HashTableSet(hash_table* Table, const char* Name, void* Value)
{
    if(!Table || !Name || !Value)
    {
        return false;
    }
    if(Table->IsPointerType)
    {
        return false;
    }

    u64 Hash = HashName(Name, Table->ElementCount);
    CopyMemory(Table->Memory + (Table->ElementSize * Hash), Value, Table->ElementSize);
    return true;
}

VENG_API b8 HashTableSetPtr(hash_table* Table, const char* Name, void** Value)
{
    if(!Table || !Name)
    {
        return false;
    }
    if(!Table->IsPointerType)
    {
        return false;
    }

    u64 Hash = HashName(Name, Table->ElementCount);
    ((void**)Table->Memory)[Hash] = Value ? *Value : 0;
    return true;
}

VENG_API b8 HashTableGet(hash_table* Table, const char* Name, void* OutValue)
{
    if(!Table || !Name)
    {
        return false;
    }
    if(Table->IsPointerType)
    {
        return false;
    }

    u64 Hash = HashName(Name, Table->ElementCount);
    CopyMemory(OutValue, Table->Memory + (Table->ElementSize * Hash), Table->ElementSize);
    return true;
}

VENG_API b8 HashTableGetPtr(hash_table* Table, const char* Name, void** OutValue)
{
    if(!Table || !Name)
    {
        return false;
    }
    if(!Table->IsPointerType)
    {
        return false;
    }

    u64 Hash = HashName(Name, Table->ElementCount);
    *OutValue = ((void**)Table->Memory)[Hash];
    return *OutValue != 0;
}

VENG_API b8 HashTableFill(hash_table* Table, void* Value)
{
    if(!Table || !Value)
    {
        return false;
    }
    if(Table->IsPointerType)
    {
        return false;
    }
    for(u32 i = 0; i < Table->ElementCount; i++)
    {
        CopyMemory(Table->Memory + (Table->ElementSize * i), Value, Table->ElementSize);
    }

    return true;
}

