// ======================================================================
//
// MemoryManagerHook.cpp
// copyright 1998 Bootprint Entertainment
// copyright 1998 Sony Online Entertainment
//
// ======================================================================

#include "FirstHeadless.h"

#if defined(_M_X64)
#include <intrin.h>
#endif

// ======================================================================

// we are using the arguments (except for file and line), but MSVC can't tell that.
#pragma warning(disable: 4100)

// ======================================================================
#if defined(_M_X64)
__declspec(noinline) void *operator new(size_t size, MemoryManagerNotALeak)
{
	return MemoryManager::allocate(size, reinterpret_cast<uintptr_t>(_ReturnAddress()), false, false);
}

__declspec(noinline) void *operator new(size_t size)
{
	return MemoryManager::allocate(size, reinterpret_cast<uintptr_t>(_ReturnAddress()), false, true);
}

__declspec(noinline) void *operator new[](size_t size)
{
	return MemoryManager::allocate(size, reinterpret_cast<uintptr_t>(_ReturnAddress()), true, true);
}

__declspec(noinline) void *operator new(size_t size, const char *file, int line)
{
	UNREF(file);
	UNREF(line);
	return MemoryManager::allocate(size, reinterpret_cast<uintptr_t>(_ReturnAddress()), false, true);
}

__declspec(noinline) void *operator new[](size_t size, const char *file, int line)
{
	UNREF(file);
	UNREF(line);
	return MemoryManager::allocate(size, reinterpret_cast<uintptr_t>(_ReturnAddress()), true, true);
}

#else
// this is here because MSVC won't let me call MemoryManager::allocate() directly from inline assembly

static void * __cdecl localAllocate(size_t size, uint32 owner, bool array, bool leakTest)
{
	return MemoryManager::allocate(size, owner, array, leakTest);
}

// ======================================================================

__declspec(naked) void *operator new(size_t size, MemoryManagerNotALeak)
{
	_asm
	{
		// setup local call stack
		push    ebp
		mov     ebp, esp

		// localAllocate(size, [return address], false, false)
		push    0
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

__declspec(naked) void *operator new(size_t size)
{
	_asm
	{
		// setup local call stack
		push    ebp
		mov     ebp, esp

		// localAllocate(size, [return address], false, true)
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

__declspec(naked) void *operator new[](size_t size)
{
	_asm
	{
		// setup local call stack
		push    ebp
		mov     ebp, esp

		// localAllocate(size, [return address], true, true)
		push    1
		push    1
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

__declspec(naked) void *operator new(size_t size, const char *file, int line)
{
	_asm
	{
		// setup local call stack
		push    ebp
		mov     ebp, esp

		// localAllocate(size, [return address], false, true)
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

__declspec(naked) void *operator new[](size_t size, const char *file, int line)
{
	_asm
	{
		// setup local call stack
		push    ebp
		mov     ebp, esp

		// localAllocate(size, [return address], true, true)
		push    1
		push    1
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

#endif

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
