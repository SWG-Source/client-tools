// ======================================================================
//
// UiMemoryBlockManager.cpp
//
// copyright 2005 Sony Online Entertainment
// ======================================================================

#include "_precompile.h"


#include "UiMemoryBlockManager.h"

#include <assert.h>

#include <map>
#include <vector>
#include <set>

#ifndef PASS_THROUGH_TO_GLOBAL_MEMORY_MANAGER
#define PASS_THROUGH_TO_GLOBAL_MEMORY_MANAGER 0
#endif

// ======================================================================

namespace UiMemoryBlockManagerNamespace
{
	inline size_t getSizeFromUserPointer(void * ptr)
	{
		size_t * iptr = (size_t *)ptr;
		iptr--;
		return *iptr;
	}

	inline void setSizeUsingRawPointer(void * ptr, size_t size)
	{
		size_t * iptr = (size_t *)ptr;
		*iptr = size;
	}

	inline void * getUserPointerFromBasePointer(void * ptr)
	{
		size_t * iptr = (size_t *)ptr;
		iptr++;
		return (void *)iptr;
	}

	inline void freeMemoryUsingUserPointer(void * ptr)
	{
		size_t * iptr = (size_t *)ptr;
		free(--iptr);
	}

	size_t const s_alignment = 4;
}

using namespace UiMemoryBlockManagerNamespace;

// ----------------------------------------------------------------------

UiMemoryBlockManager::MemoryPool::MemoryPool(size_t const size) :
m_size(size),
m_memoryBase(malloc(size)),
m_memoryCurrent(m_memoryBase)
{

}

// ----------------------------------------------------------------------

UiMemoryBlockManager::MemoryPool::~MemoryPool()
{
	free(m_memoryBase);
	m_memoryBase = 0;
	m_memoryCurrent = 0;
}

// ----------------------------------------------------------------------

size_t UiMemoryBlockManager::MemoryPool::remainingBytes() const
{
	size_t const offset = (char *)m_memoryCurrent - (char *)m_memoryBase;
	return m_size - offset;
}

// ----------------------------------------------------------------------

void * UiMemoryBlockManager::MemoryPool::allocate(size_t const size)
{
	UI_ASSERT(remainingBytes() >= size);
	void * newMem = m_memoryCurrent;
	m_memoryCurrent = (void *)((char *)m_memoryCurrent + size);
	return newMem;
}


// ======================================================================
// Create a UiMemoryBlockManager
//

UiMemoryBlockManager::UiMemoryBlockManager(const char *name, size_t granularity)
	: m_name("No Name"),
#ifdef _DEBUG
	m_bytesAllocated(0),
	m_allAllocations(new MemorySet()),
#endif
	m_memoryMap(new MemoryMap(251)),
	m_memoryPool(new MemoryPool::Array),
	m_poolGranularity(granularity)
{
#ifndef _DEBUG
	addPool();
#endif

	if (name)
	{
		m_name = new char [strlen (name) + 1];
		strcpy (m_name, name);
	}
}

// ----------------------------------------------------------------------
/**
 * Destroy a UiMemoryBlockManager.
 * 
 * It is an error if there are any elements still allocated from this UiMemoryBlockManager
 * when its destructor is called.  In this case, debug compiles will call Fatal.  Release
 * compiles will not return the blocks to the real memory manager, because a memory leak is
 * better than an exception.
 */

UiMemoryBlockManager::~UiMemoryBlockManager()
{
#ifdef _DEBUG
	debugDump();

	if (!m_allAllocations->empty()) 
	{
		UI_DEBUG_REPORT_LOG(true, ("UiMemoryBlockManager(%s)::~ %d elements leaked.\n", m_name, m_allAllocations->size()));

		size_t totalMemLeaked = 0;
		for (MemorySet::const_iterator itLeak = m_allAllocations->begin(); itLeak != m_allAllocations->end(); ++itLeak) 
		{
			size_t objectSize = getSizeFromUserPointer(*itLeak);
			totalMemLeaked += objectSize;
			UI_DEBUG_REPORT_LOG(true, ("     %x - %d bytes.\n", *itLeak, objectSize));
		}

		UI_DEBUG_REPORT_LOG(true, ("    %d bytes leaked.\n", totalMemLeaked));
	}

	delete m_allAllocations;
	m_allAllocations = NULL;
#endif

#if PASS_THROUGH_TO_GLOBAL_MEMORY_MANAGER
	// Free the mem.
	for (MemoryMap::const_iterator itMem = m_memoryMap->begin(); itMem != m_memoryMap->end(); ++itMem)
	{
		freeMemoryUsingUserPointer(itMem->second);
	}
#endif

	delete m_memoryMap;
	m_memoryMap = NULL;
	
	for (MemoryPool::Array::const_iterator itPool = m_memoryPool->begin(); itPool != m_memoryPool->end(); ++itPool) 
	{
		delete *itPool;
	}
	
	delete m_memoryPool;
	m_memoryPool = 0;

	delete [] m_name;
	m_name = 0;
}

// ----------------------------------------------------------------------

void *UiMemoryBlockManager::allocMem(size_t const size, bool)
{
	void * mem = NULL;

	if (size) 
	{
		MemoryMap::iterator const itMem = m_memoryMap->find(size);
		if (itMem == m_memoryMap->end()) 
		{
			size_t const requestedSize = s_alignment + size + sizeof(size_t);
			size_t const alignedSize = requestedSize / s_alignment;
			size_t const allocSize = alignedSize * s_alignment;
			
			assert(size < allocSize);
			
			mem = poolAllocate(allocSize);

			setSizeUsingRawPointer(mem, size);

			mem = getUserPointerFromBasePointer(mem);

#ifdef _DEBUG
			m_allAllocations->insert(mem);
			m_bytesAllocated += size;
#endif
		}
		else
		{
			mem = itMem->second;
			m_memoryMap->erase(itMem);
		}
	}
	return mem;
}

// ----------------------------------------------------------------------
/**
 * Free an element.
 * 
 * The pointer passed in to this routine may be NULL, in which case nothing happens.
 */

void UiMemoryBlockManager::freeMem(void *pointer)
{
	if (pointer) 
	{
#ifdef _DEBUG
		m_allAllocations->erase(pointer);
#endif
		size_t size = getSizeFromUserPointer(pointer);
		m_memoryMap->insert(std::make_pair(size, pointer));
	}
}

// ----------------------------------------------------------------------

#ifdef _DEBUG
void UiMemoryBlockManager::debugDump() const
{
	UI_DEBUG_REPORT_LOG(true, ("UiMemoryBlockManager(%s) %d bytes allocated.\n", m_name, m_bytesAllocated));
}
#endif

// ----------------------------------------------------------------------

void * UiMemoryBlockManager::poolAllocate(size_t const size)
{
#if PASS_THROUGH_TO_GLOBAL_MEMORY_MANAGER
	void * mem = malloc(size);
#else
	UI_ASSERT(size < m_poolGranularity);

	void * mem = NULL;
	for (MemoryPool::Array::reverse_iterator itPool = m_memoryPool->rbegin(); itPool != m_memoryPool->rend(); ++itPool) 
	{
		if (*itPool) 
		{
			MemoryPool & pool = **itPool;
			
			if (pool.remainingBytes() > (size + sizeof(size_t))) 
			{
				mem = pool.allocate(size);
				break;
			}
		}
	}
	
	if (!mem) 
	{
		addPool();
		mem = poolAllocate(size);
	}
#endif

	return mem;
}

// ----------------------------------------------------------------------

void UiMemoryBlockManager::addPool()
{
#if !PASS_THROUGH_TO_GLOBAL_MEMORY_MANAGER
	MemoryPool * newPool = new MemoryPool(m_poolGranularity);
	m_memoryPool->push_back(newPool);
#endif
}


// ======================================================================
