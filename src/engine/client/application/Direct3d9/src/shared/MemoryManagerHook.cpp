// ======================================================================
//
// MemoryManagerHook.cpp
// copyright 1998 Bootprint Entertainment
// copyright 1998 Sony Online Entertainment
//
// ======================================================================

#include "FirstDirect3d9.h"

#include <intrin.h>

// ======================================================================

// we are using the arguments (except for file and line), but MSVC can't tell that.
#pragma warning(disable: 4100)

// ======================================================================
// Phase 31 (BITS-01, B-GAP-1): these per-DLL operator new overloads were
// __declspec(naked) x86 inline-asm trampolines (illegal on x64, C4235) that
// forwarded to MemoryManager::allocate, passing the CALLER's return address as
// the leak-tracking `owner` (the original read `[ebp+4]`). Each is replaced by a
// normal operator new that captures the same owner via the _ReturnAddress()
// intrinsic at the operator-new entry point -- so the leak owner stays the real
// allocation call site -- preserving the per-routine (array, leakTest) flags the
// asm encoded. Compiles on both x86 and x64 (no #ifdef fork).

static void * __cdecl localAllocate(size_t size, uint32 owner, bool array, bool leakTest)
{
	return MemoryManager::allocate(size, owner, array, leakTest);
}

// ======================================================================

void *operator new(size_t size, MemoryManagerNotALeak)
{
	uint32 const owner = static_cast<uint32>(reinterpret_cast<uintptr_t>(_ReturnAddress()));
	return localAllocate(size, owner, false, false);
}

// ----------------------------------------------------------------------

void *operator new(size_t size)
{
	uint32 const owner = static_cast<uint32>(reinterpret_cast<uintptr_t>(_ReturnAddress()));
	return localAllocate(size, owner, false, true);
}

// ----------------------------------------------------------------------

void *operator new[](size_t size)
{
	uint32 const owner = static_cast<uint32>(reinterpret_cast<uintptr_t>(_ReturnAddress()));
	return localAllocate(size, owner, true, true);
}

// ----------------------------------------------------------------------

void *operator new(size_t size, const char *file, int line)
{
	uint32 const owner = static_cast<uint32>(reinterpret_cast<uintptr_t>(_ReturnAddress()));
	return localAllocate(size, owner, false, true);
}

// ----------------------------------------------------------------------

void *operator new[](size_t size, const char *file, int line)
{
	uint32 const owner = static_cast<uint32>(reinterpret_cast<uintptr_t>(_ReturnAddress()));
	return localAllocate(size, owner, true, true);
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
