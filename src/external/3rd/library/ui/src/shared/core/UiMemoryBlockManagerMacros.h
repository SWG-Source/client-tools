// ======================================================================
//
// UiMemoryBlockManagerMacros.h
// Copyright 2002, Sony Online Entertainment Inc.
// All Rights Reserved.
//
// ======================================================================

#ifndef INCLUDED_UiMemoryBlockManagerMacros_H
#define INCLUDED_UiMemoryBlockManagerMacros_H

// ======================================================================

#include "UiMemoryBlockManager.h"

//----------------------------------------------------------------------

#define UI_MEMORY_BLOCK_MANAGER_INTERFACE \
	public: \
			inline void * operator new(size_t size) \
			{ \
				return ms_memoryBlockManager.allocMem(size); \
			} \
			inline void * operator new(size_t, void * placement) \
			{ \
				return placement; \
			}\
			inline void operator delete(void *pointer) \
			{ \
				ms_memoryBlockManager.freeMem(pointer); \
			} \
	private: \
		static UiMemoryBlockManager ms_memoryBlockManager; \
	public: 

// ----------------------------------------------------------------------

#define UI_MEMORY_BLOCK_MANAGER_IMPLEMENTATION(className, maxSizeMB) \
	UiMemoryBlockManager className::ms_memoryBlockManager(#className, maxSizeMB * 1024 * 1024 /*megabytes*/);

// ======================================================================

#endif
