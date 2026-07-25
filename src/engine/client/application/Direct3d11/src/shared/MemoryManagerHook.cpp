// ======================================================================
//
// MemoryManagerHook.cpp
// copyright 1998 Bootprint Entertainment
// copyright 1998 Sony Online Entertainment
//
// ======================================================================

#include "FirstDirect3d11.h"

#include <intrin.h>   // _ReturnAddress

// ======================================================================

// we are using the arguments (except for file and line), but MSVC can't tell that.
#pragma warning(disable: 4100)

// ======================================================================
//
// Was: each `operator new`/`operator new[]` overload below was an x86
// naked function written in inline asm to read the caller's return
// address from [ebp+4] and forward it to localAllocate as the
// memory-owner tag. MSVC dropped support for naked + inline asm on
// x64. The `_ReturnAddress` intrinsic does the same thing portably and
// the compiler emits the prolog/epilog itself.

static void * __cdecl localAllocate(size_t size, uint32 owner, bool array, bool leakTest)
{
	return MemoryManager::allocate(size, owner, array, leakTest);
}

namespace
{
	inline uint32 callerOwner()
	{
		// Truncate the (possibly 64-bit) return address to the 32-bit
		// owner-tag type. The tag is opaque to MemoryManager; only the
		// leak tracker reads it for diagnostics.
		return static_cast<uint32>(reinterpret_cast<uintptr_t>(_ReturnAddress()));
	}
}

// ======================================================================

void *operator new(size_t size, MemoryManagerNotALeak)
{
	// localAllocate(size, [return address], array=false, leakTest=false)
	return localAllocate(size, callerOwner(), false, false);
}

// ----------------------------------------------------------------------

void *operator new(size_t size)
{
	// localAllocate(size, [return address], array=false, leakTest=true)
	return localAllocate(size, callerOwner(), false, true);
}

// ----------------------------------------------------------------------

void *operator new[](size_t size)
{
	// localAllocate(size, [return address], array=true, leakTest=true)
	return localAllocate(size, callerOwner(), true, true);
}

// ----------------------------------------------------------------------

void *operator new(size_t size, const char *file, int line)
{
	// localAllocate(size, [return address], array=false, leakTest=true)
	return localAllocate(size, callerOwner(), false, true);
}

// ----------------------------------------------------------------------

void *operator new[](size_t size, const char *file, int line)
{
	// localAllocate(size, [return address], array=true, leakTest=true)
	return localAllocate(size, callerOwner(), true, true);
}

// ----------------------------------------------------------------------

void operator delete(void *pointer)
{
	if (pointer)
		MemoryManager::free(pointer, false);
}

// ----------------------------------------------------------------------

void operator delete[](void *pointer)
{
	if (pointer)
		MemoryManager::free(pointer, true);
}

// ----------------------------------------------------------------------

void operator delete(void *pointer, const char *file, int line)
{
	if (pointer)
		MemoryManager::free(pointer, true);
}

// ----------------------------------------------------------------------

void operator delete[](void *pointer, const char *file, int line)
{
	if (pointer)
		MemoryManager::free(pointer, true);
}

// ======================================================================
