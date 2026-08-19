// ======================================================================
//
// RegexServices.cpp
// Copyright 2003 Sony Online Entertainment, Inc.
// All Rights Reserved.
//
// ======================================================================

#include "sharedRegex/FirstSharedRegex.h"
#include "sharedRegex/RegexServices.h"

#include <intrin.h> // _ReturnAddress

// ======================================================================

static void * __cdecl localAllocate(size_t size, uint32 owner, bool array, bool leakTest)
{
	return MemoryManager::allocate(size, owner, array, leakTest);
}

// ----------------------------------------------------------------------
//
// Was: an x86 naked function that read the caller's return address from
// [ebp+4] and passed it to localAllocate as the memory-owner tag. The
// `_ReturnAddress` intrinsic does the same thing portably and the
// compiler skips emitting any prolog/epilog itself, so we no longer
// need the naked declaration. Truncating the 64-bit return address to
// the 32-bit `owner` is fine - it's an opaque identifier used only by
// the leak tracker.

static void *regexAllocate(size_t size)
{
	const uintptr_t returnAddress = reinterpret_cast<uintptr_t>(_ReturnAddress());
	return localAllocate(size, static_cast<uint32>(returnAddress), false, true);
}

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
