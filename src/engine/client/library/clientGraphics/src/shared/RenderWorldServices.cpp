// ======================================================================
//
// RenderWorld_Services.cpp
// Copyright 2000-01, Sony Online Entertainment Inc.
// All Rights Reserved.
//
// ======================================================================

#include "clientGraphics/FirstClientGraphics.h"
#include "clientGraphics/RenderWorldServices.h"

#include "sharedFoundation/ExitChain.h"
#include "sharedSynchronization/Mutex.h"

#include <intrin.h>   // _ReturnAddress

// ======================================================================

RenderWorldServices::RenderWorldServices()
: DPVS::LibraryDefs::Services(),
	m_mutex(new Mutex())
{
}

// ----------------------------------------------------------------------

RenderWorldServices::~RenderWorldServices()
{
	delete m_mutex;
}

// ----------------------------------------------------------------------

void RenderWorldServices::error(const char * message)
{
	UNREF(message);
//	if (!ExitChain::isFataling())
//		FATAL(true, ("DPVS error: %s", message));
}

// ----------------------------------------------------------------------

static void * __cdecl localAllocate(size_t size, uint32 owner, bool array, bool leakTest)
{
	return MemoryManager::allocate(size, owner, array, leakTest);
}

// Was an x86 naked function that read the caller's return address from
// [ebp+4] and forwarded it as the memory-owner tag. _ReturnAddress() is
// the portable intrinsic equivalent and works on both x86 and x64.
static void * dpvsAllocate(size_t size)
{
	const uintptr_t returnAddress = reinterpret_cast<uintptr_t>(_ReturnAddress());
	return localAllocate(size, static_cast<uint32>(returnAddress), false, true);
}

// ----------------------------------------------------------------------

void *RenderWorldServices::allocateMemory(size_t bytes)
{
	return dpvsAllocate(bytes);
}

// ----------------------------------------------------------------------

void RenderWorldServices::releaseMemory(void * ptr)
{
	MemoryManager::free(ptr, false);
}

// ----------------------------------------------------------------------

void RenderWorldServices::enterMutex()
{
	m_mutex->enter();
}

// ----------------------------------------------------------------------

void RenderWorldServices::leaveMutex()
{
	m_mutex->leave();
}

// ======================================================================
