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

static __declspec(naked) void * dpvsAllocate(size_t)
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
