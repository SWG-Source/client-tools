// ======================================================================
//
// OsNewDel.h
//
// Copyright 2002 Sony Online Entertainment
//
// ======================================================================

#pragma once
#include <cstddef>  // for size_t

// ======================================================================
// MemoryManager custom new/delete overloads
// ======================================================================

enum MemoryManagerNotALeak
{
    MM_notALeak
};

// ----------------------------------------------------------------------
// Custom allocation operators (implemented in OsNewDel.cpp)
// ----------------------------------------------------------------------

void* __cdecl operator new(std::size_t size, MemoryManagerNotALeak);
void* __cdecl operator new(std::size_t size);
void* __cdecl operator new[](std::size_t size);
void* __cdecl operator new(std::size_t size, const char* file, int line);
void* __cdecl operator new[](std::size_t size, const char* file, int line);

// ----------------------------------------------------------------------
// Delete overloads
// ----------------------------------------------------------------------

void operator delete(void* pointer) noexcept;
void operator delete[](void* pointer) noexcept;
void operator delete(void* pointer, const char* file, int line) noexcept;
void operator delete[](void* pointer, const char* file, int line) noexcept;

// ----------------------------------------------------------------------
// C++14 sized delete overloads (optional, avoids warnings in modern compilers)
// ----------------------------------------------------------------------

#if __cplusplus >= 201402L
inline void operator delete(void* ptr, std::size_t) noexcept
{
    operator delete(ptr);
}

inline void operator delete[](void* ptr, std::size_t) noexcept
{
    operator delete[](ptr);
}
#endif

// ======================================================================

