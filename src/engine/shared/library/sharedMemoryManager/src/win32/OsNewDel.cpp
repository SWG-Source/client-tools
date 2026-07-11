// ======================================================================
//
// OsNewDel.cpp
//
// Copyright 2002 Sony Online Entertainment
//
// ======================================================================

#include "sharedMemoryManager/FirstSharedMemoryManager.h"
#include "sharedMemoryManager/OsNewDel.h"

// Disable "unreferenced parameter" warning for debug overloads
#pragma warning(disable: 4100)

// ----------------------------------------------------------------------
// Local wrapper for MemoryManager::allocate()
// (historically used by the old asm versions)
static inline void* localAllocate(size_t size, uint32 owner, bool array, bool leakTest)
{
    return MemoryManager::allocate(size, owner, array, leakTest);
}

// ----------------------------------------------------------------------
// operator new overloads
// ----------------------------------------------------------------------

void* __cdecl operator new(size_t size, MemoryManagerNotALeak)
{
    return MemoryManager::allocate(size, 0, false, false);
}

void* __cdecl operator new(size_t size)
{
    return MemoryManager::allocate(size, 0, false, true);
}

void* __cdecl operator new[](size_t size)
{
    return MemoryManager::allocate(size, 0, true, true);
}

void* __cdecl operator new(size_t size, const char* file, int line)
{
    UNREF(file);
    UNREF(line);
    return MemoryManager::allocate(size, 0, false, true);
}

void* __cdecl operator new[](size_t size, const char* file, int line)
{
    UNREF(file);
    UNREF(line);
    return MemoryManager::allocate(size, 0, true, true);
}

// ----------------------------------------------------------------------
// operator delete overloads (C++14-compliant, noexcept)
// ----------------------------------------------------------------------

void __cdecl operator delete(void* pointer) noexcept
{
    if (pointer)
        MemoryManager::free(pointer, false);
}

void __cdecl operator delete[](void* pointer) noexcept
{
    if (pointer)
        MemoryManager::free(pointer, true);
}

// "delete-size" overloads (added in C++14+ for matching new/delete pairs)
void __cdecl operator delete(void* pointer, size_t) noexcept
{
    if (pointer)
        MemoryManager::free(pointer, false);
}

void __cdecl operator delete[](void* pointer, size_t) noexcept
{
    if (pointer)
        MemoryManager::free(pointer, true);
}

// Debug variants (file/line versions)
void __cdecl operator delete(void* pointer, const char* file, int line) noexcept
{
    UNREF(file);
    UNREF(line);

    if (pointer)
        MemoryManager::free(pointer, false);
}

void __cdecl operator delete[](void* pointer, const char* file, int line) noexcept
{
    UNREF(file);
    UNREF(line);

    if (pointer)
        MemoryManager::free(pointer, true);
}

// ----------------------------------------------------------------------
// Re-enable warnings
// ----------------------------------------------------------------------
#pragma warning(default: 4100)