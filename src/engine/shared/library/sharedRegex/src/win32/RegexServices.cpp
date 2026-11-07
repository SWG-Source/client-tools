// ======================================================================
//
// RegexServices.cpp
// Copyright 2003 Sony Online Entertainment, Inc.
// All Rights Reserved.
//
// ======================================================================

#include "sharedRegex/FirstSharedRegex.h"
#include "sharedRegex/RegexServices.h"

#if defined(_M_X64)
#include <intrin.h>
#endif

// ======================================================================

#if defined(_M_X64)
static __declspec(noinline) void * regexAllocate(size_t size)
{
	return MemoryManager::allocate(size, reinterpret_cast<uintptr_t>(_ReturnAddress()), false, true);
}
#else
static void * __cdecl localAllocate(size_t size, uint32 owner, bool array, bool leakTest)
{
	return MemoryManager::allocate(size, owner, array, leakTest);
}

static __declspec(naked) void * regexAllocate(size_t)
{
	_asm
	{
		// setup local call stack
		push    ebp
		mov     ebp, esp

		// MemoryManager::alloc(size, [return address], false, true)
		push    1
		push    0
		mov     eax, dword ptr [ebp+4]
		push    eax
		mov     eax, dword ptr [ebp+8]
		push    eax
		call    localAllocate
		add     esp, 12

		mov     esp, ebp
		pop     ebp
		ret
	}
}

#endif

// ----------------------------------------------------------------------

void *RegexServices::allocateMemory(size_t byteCount)
{
	return regexAllocate(byteCount);
}

// ----------------------------------------------------------------------

void RegexServices::freeMemory(void *pointer)
{
	MemoryManager::free(pointer, false);
}

// ======================================================================
