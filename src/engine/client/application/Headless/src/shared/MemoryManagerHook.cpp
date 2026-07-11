// ======================================================================
//
// MemoryManagerHook.cpp
// copyright 1998 Bootprint Entertainment
// copyright 1998 Sony Online Entertainment
//
// x64 port: original x86 __declspec(naked) / __asm trampolines replaced
// with normal C++ that calls localAllocate. The original captured the
// caller return address from the stack and passed it as the "owner" arg
// (debug tracking); on x64 we use _ReturnAddress() instead.
//
// ======================================================================

#include "FirstHeadless.h"

#include <intrin.h>

#pragma intrinsic(_ReturnAddress)

// ======================================================================

#pragma warning(disable: 4100)

// ----------------------------------------------------------------------

namespace
{
	inline uint32 OwnerFromReturnAddress(void *ret)
	{
		// MemoryManager's owner field is uint32 — truncate on x64.
		return static_cast<uint32>(reinterpret_cast<uintptr_t>(ret));
	}
}

// ----------------------------------------------------------------------

static void *localAllocate(size_t size, uint32 owner, bool array, bool leakTest)
{
	return MemoryManager::allocate(size, owner, array, leakTest);
}

// ======================================================================

void *operator new(size_t size, MemoryManagerNotALeak)
{
	return localAllocate(size, OwnerFromReturnAddress(_ReturnAddress()), false, false);
}

// ----------------------------------------------------------------------

void *operator new(size_t size)
{
	return localAllocate(size, OwnerFromReturnAddress(_ReturnAddress()), false, true);
}

// ----------------------------------------------------------------------

void *operator new[](size_t size)
{
	return localAllocate(size, OwnerFromReturnAddress(_ReturnAddress()), true, true);
}

// ----------------------------------------------------------------------

void *operator new(size_t size, const char *file, int line)
{
	return localAllocate(size, OwnerFromReturnAddress(_ReturnAddress()), false, true);
}

// ----------------------------------------------------------------------

void *operator new[](size_t size, const char *file, int line)
{
	return localAllocate(size, OwnerFromReturnAddress(_ReturnAddress()), true, true);
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
