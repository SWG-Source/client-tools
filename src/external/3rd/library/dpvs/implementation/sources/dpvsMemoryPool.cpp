/*****************************************************************************
 *
 * dPVS
 * -----------------------------------------
 *
 * (C) 1999-2001 Hybrid Holding, Ltd.
 * All Rights Reserved.
 *
 * This file consists of unpublished, proprietary source code of
 * Hybrid Holding, and is considered Confidential Information for
 * purposes of non-disclosure agreement. Disclosure outside the terms
 * outlined in signed agreement may result in irrepairable harm to
 * Hybrid Holding and legal action against the party in breach.
 *
 * Description:		Memory management code
 *
 * $Archive: /dpvs/implementation/sources/dpvsMemoryPool.cpp $
 * $Author: wili $ 
 * $Revision: #1 $
 * $Modtime: 8.10.02 12:07 $
 * $Date: 2003/01/22 $
 * 
 ******************************************************************************/

/*
TODO: add some internal counters that determine _when_ to create fixed-size pools
      and what size they should be (don't always use the 4kB pools).
*/

#include "dpvsMemoryPool.hpp"
#include "dpvsBitMath.hpp"
#include "dpvsStatistics.hpp"
#include "dpvsDebug.hpp"

//#define MEMORYPOOL_PARANOID
#if defined (MEMORYPOOL_PARANOID) && defined (DPVS_DEBUG)
#	define DEBUG_CODE(X) X
#else
#	define DEBUG_CODE(X)
#endif

#include <cstdio> // DEBUG DEBUG

using namespace DPVS;

#if defined (DPVS_DEBUG)
	bool MemoryPool::s_initialized = false;
#endif

DPVS_FORCE_INLINE void* MemoryPool::allocateMemoryBlock (size_t s)
{
	Library::Services* serv = Debug::getServices();
	DPVS_ASSERT(serv);
	return serv->allocateMemory(s);

}

DPVS_FORCE_INLINE void MemoryPool::releaseMemoryBlock  (void* p)
{
	Library::Services* serv = Debug::getServices();
	DPVS_ASSERT(serv);
	serv->releaseMemory(p);	
}

/*****************************************************************************
 *
 * Function:		MemoryPool::MemoryPool()
 *
 * Description:		Constructor
 *
 * Notes:			The data pointer must be submitted from the outside. It
 *					will _not_ be released in the destructor!
 *
 *****************************************************************************/

MemoryPool::MemoryPool (void) :
	m_firstPool(null),
	m_totalSize(0),
	m_memUsed(0),
	m_freeListMask(0)
{
#if defined (MEMORYPOOL_FIXED)
	for (int i = 0; i < FIXED_POOL_COUNT; i++)
		m_fixedPools[i] = null;
#endif

	for (int i = 0; i < 32; i++)
		m_freeList[i] = null;

	PROFILE_CODE(
	m_usedCount			= 0;
	m_allocs			= 0;
	m_releases			= 0;
	m_statIters			= 0;
	m_statAllocs		= 0;
	m_rem0 = m_rem1 = m_rem2 = m_rem3 = 0;)

#if defined (DPVS_DEBUG)
	s_initialized = true;
#endif

//	DEBUG_CODE(validateFreeList());
}

/*****************************************************************************
 *
 * Function:		MemoryPool::releaseAll()
 *
 * Description:		Internal function that releases all pages
 *
 * Notes:			
 *
 *****************************************************************************/

void MemoryPool::releaseAll (void)
{
	//--------------------------------------------------------------------
	// Force-release any existing memory allocations (we should assert here
	// that a leak has occured!)
	//--------------------------------------------------------------------

#if defined (MEMORYPOOL_FIXED)
	for (int i = 0; i < FIXED_POOL_COUNT; i++)
		DPVS_ASSERT(m_fixedPools[i] == null);					// WASS?
#endif

	DPVS_ASSERT (!m_firstPool && "DPVS::MemoryPool::releaseAll() -- there are leaks!");		// ??

	Pool* p = m_firstPool;
	while (p)
	{
		Pool* next = p->m_next;
		DPVS_PROFILE(Statistics::incStatistic(Library::STAT_MEMORYEXTERNALOPERATIONS,1));
		releaseMemoryBlock(p->m_allocation);
		p = next;
	}
#if defined (MEMORYPOOL_FIXED)
	for (int i = 0; i < FIXED_POOL_COUNT; i++)
		m_fixedPools[i] = null;
#endif
	for (int i = 0; i < 32; i++)
		m_freeList[i] = null;
	m_freeListMask = 0;

}

/*****************************************************************************
 *
 * Function:		MemoryPool::~MemoryPool()
 *
 * Description:		Destructor
 *
 * Notes:			The data pointer is _not_ released here! The function
 *					tests for leaks in debug build.
 *
 *****************************************************************************/

// should be a virtual function, but temporarily made non-virtual
MemoryPool::~MemoryPool (void)
{
#if defined (MEMORYPOOL_FIXED)
	for (int i = 0; i < FIXED_POOL_COUNT; i++)
		DPVS_ASSERT(m_fixedPools[i] == null);					// WASS?
#endif

	DPVS_ASSERT (!m_firstPool && "DPVS::MemoryPool::~MemoryPool() -- there are leaks!");	
//	releaseAll();

#if defined (DPVS_DEBUG)
	s_initialized = false;
#endif
}

#if defined (MEMORYPOOL_FIXED)

/*****************************************************************************
 *
 * Function:		MemoryPool::allocateFixed()
 *
 * Description:		Allocates using the fixed-size allocator
 *
 * Parameters:		size = allocation size (MUST'VE BEEN ALIGNED EXTERNALLY)
 * 
 * Notes:			This function is called for "small" allocs (i.e. under
 *					256 bytes). These are allocated using a different mechanism
 *					that has a smaller per-alloc overhead and no coalescing.
 * 
 *****************************************************************************/

DPVS_FORCE_INLINE void* MemoryPool::allocateFixed	(int size)
{
	DPVS_ASSERT(size >= (int)sizeof(FixedFreeHeader));				// WASS?
	DPVS_ASSERT(!(size & (ALIGNMENT-1)));							// must be aligned!
	int pool = (size>>4)-1;											// select pool
	DPVS_ASSERT(pool >= 0 && pool<FIXED_POOL_COUNT);				// HUHHHH?

	//--------------------------------------------------------------------
	// Are there any available pools?
	//--------------------------------------------------------------------

	if (!m_fixedPools[pool])									// we don't have a fixed pool.. so allocate one
	{
		int		allocSize   = 8192 + sizeof(FixedPoolHeader) + ALIGNMENT;
		int		numElements = (allocSize-sizeof(FixedPoolHeader)-ALIGNMENT) / size;				
		void*	d			= allocate(allocSize);	// allocate memory for our evil needs
		if (!d)
			return null;	// alloc failed!

//		printf ("allocating pool for elements of size %d (%d elements = %d bytes) waste = %d\n",size,numElements,numElements*size,allocSize-numElements*size);

		FixedPoolHeader* p	= reinterpret_cast<FixedPoolHeader*>(d);

		m_fixedPools[pool]	= p;
		p->m_fixedIndex		= pool;
		p->m_usedCount		= 0;
		p->m_prevPool		= null;
		p->m_nextPool		= null;

		char* c = (char*)(p+1) + ALIGNMENT-((sizeof(FixedPoolHeader)+sizeof(FixedHeader))%ALIGNMENT);
		p->m_firstFree		= reinterpret_cast<FixedFreeHeader*>(c);

		DPVS_ASSERT(((UPTR)(size)&3)==0);// ??
		DPVS_ASSERT(((UPTR)(c)&3)==0);// ??
		for (int i = 0; i < numElements; i++, c+=size)
		{
			FixedFreeHeader* h = reinterpret_cast<FixedFreeHeader*>(c);
			DPVS_ASSERT( (((UPTR)(h)+sizeof(UPTR)) & (ALIGNMENT-1)) == 0 );
			h->setPoolHeader(p);
			h->m_nextFree = (i == (numElements-1)) ? null : reinterpret_cast<FixedFreeHeader*>(c+size);
		}
	}

	//--------------------------------------------------------------------
	// Perform allocation from the pool
	//--------------------------------------------------------------------

	FixedPoolHeader* p = m_fixedPools[pool];
	DPVS_ASSERT(p);

	FixedFreeHeader* h = p->m_firstFree;
	DPVS_ASSERT( h->getPoolHeader() == p);

	p->m_usedCount++;
	p->m_firstFree = h->m_nextFree;

	//--------------------------------------------------------------------
	// If the pool "closes", remove it from the free pool list
	//--------------------------------------------------------------------

	if (!p->m_firstFree)						// remove from linked list
	{
		DPVS_ASSERT(!p->m_prevPool);			// can't be
		m_fixedPools[pool] = p->m_nextPool;
		if (p->m_nextPool)
			p->m_nextPool->m_prevPool = null;
		p->m_prevPool = null;					// unnecessary??
		p->m_nextPool = null;					// unnecessary??
	}

	//--------------------------------------------------------------------
	// Return pointer to the memory..
	//--------------------------------------------------------------------
	
	void* m = (void*)(static_cast<FixedHeader*>(h) + 1);
	DPVS_ASSERT ((reinterpret_cast<UPTR>(m) & (ALIGNMENT-1))==0);	// make sure ptr is aligned
	return m;
}

/*****************************************************************************
 *
 * Function:		MemoryPool::releaseFixed()
 *
 * Description:		Releases memory block earlier allocated from a fixed pool
 *
 * Parameters:		h = pointer to header
 *
 * Notes:			This function is called for "small" allocs (i.e. under
 *					256 bytes). These are allocated using a different mechanism
 *					that has a smaller per-alloc overhead and no coalescing.
 *
 *					The function is not called externally. It is called by
 *					the "release" function when it detects a fixed block
 * 
 *****************************************************************************/

DPVS_FORCE_INLINE void MemoryPool::releaseFixed (Header* header)
{
	DPVS_ASSERT(header);
	DPVS_ASSERT(header->getType() == Header::BLOCK_FIXED);	// WASSSS?
	// Note that the fixed pool entries have a different-size header!
	FixedFreeHeader*	fh		= reinterpret_cast<FixedFreeHeader*>((char*)(header)+sizeof(UPTR));
	FixedPoolHeader*	p		= fh->getPoolHeader();				// get pointer to pool
	
	DPVS_ASSERT(p);

	bool				wasFull = (p->m_firstFree == null);			// was the pool full?

	//--------------------------------------------------------------------
	// Add block to free list
	//--------------------------------------------------------------------

	fh->m_nextFree = p->m_firstFree;								// add to free list
	p->m_firstFree = fh;

	//--------------------------------------------------------------------
	// Check if pool became empty -> destroy it and return immediately..
	// DEBUG DEBUG TODO: ADD A RULE FOR HAVING ONLY A SINGLE PAGE SO WE
	// AVOID THE FLIP-FLOP SCENARIO
	//--------------------------------------------------------------------

	p->m_usedCount--;
	DPVS_ASSERT(p->m_usedCount>=0);									// WASS?

	if (p->m_usedCount == 0)
	{
		// pool MUST've been in the free list
		DPVS_ASSERT(p->m_prevPool || p->m_nextPool || m_fixedPools[p->m_fixedIndex]==p);

		if (p->m_prevPool) 
			p->m_prevPool->m_nextPool = p->m_nextPool;
		else
		{
			DPVS_ASSERT( p == m_fixedPools[p->m_fixedIndex]);		// WASS?
			m_fixedPools[p->m_fixedIndex] = p->m_nextPool;
		}
		if (p->m_nextPool) 
			p->m_nextPool->m_prevPool = p->m_prevPool;
		
		release (p);												// release memory associated with the pool
		return;														// out job here is done..
	}

	//--------------------------------------------------------------------
	// If pool was full before this release, add pool to head of
	// "free list" (TODO: maybe we want to add it to the tail -- see
	// about policies later)
	//--------------------------------------------------------------------

	if (wasFull)
	{
		DPVS_ASSERT (!p->m_prevPool && !p->m_nextPool && m_fixedPools[p->m_fixedIndex] != p);
		p->m_prevPool = null;
		p->m_nextPool = m_fixedPools[p->m_fixedIndex];
		if (p->m_nextPool)
			p->m_nextPool->m_prevPool = p;
		m_fixedPools[p->m_fixedIndex] = p;
	}
}

#endif // MEMORYPOOL_FIXED

/*****************************************************************************
 *
 * Function:		MemoryPool::getBlockSize()
 *
 * Description:		Returns size of block 'h' in bytes
 *
 * Parameters:		h = pointer to block header
 *
 * Returns:			size of block in bytes
 *
 * Notes:			The size includes the size of the header
 *
 *****************************************************************************/

DPVS_FORCE_INLINE int MemoryPool::getBlockSize (const Header* h)	// returns size of block (including sizeof(Header))
{
	DPVS_ASSERT(h && h->getNext());
	return (INT32)(reinterpret_cast<const char*>(h->getNext()) - reinterpret_cast<const char*>(h));
}

/*****************************************************************************
 *
 * Function:		MemoryPool::getFreeListIndex()
 *
 * Description:		Returns free list index of a FreeHeader
 *
 * Parameters:		f = pointer to free header
 *
 * Returns:			index (in range [0,31])
 *
 *****************************************************************************/

DPVS_FORCE_INLINE int MemoryPool::getFreeListIndex	(FreeHeader* f)
{
	int size  = getBlockSize(f);
	DPVS_ASSERT(size>0);
	return 31-getHighestSetBit(size);								// give these in reverse order
}

DPVS_FORCE_INLINE int MemoryPool::getFreeListIndex	(int size)
{
	DPVS_ASSERT(size>0);
	return 31-getHighestSetBit(size);								// give these in reverse order
}

DPVS_FORCE_INLINE void MemoryPool::unlinkBlock (FreeHeader* f)
{
	if (f->m_prevFree)
		f->m_prevFree->m_nextFree = f->m_nextFree;
	else
	{
		UINT32 index = getFreeListIndex(f);
		DPVS_ASSERT(m_freeList[index] == f);
		m_freeList[index] = f->m_nextFree;
		if (!m_freeList[index])
			m_freeListMask &= ~(1<<index);							// remove bit!
	}
	if (f->m_nextFree)
		f->m_nextFree->m_prevFree = f->m_prevFree;

	// NOTE: LEAVES THE PREVFREE AND NEXTFREE POINTERS INVALID!

}

DPVS_FORCE_INLINE void MemoryPool::linkBlock (FreeHeader* f)
{
	UINT32 index = getFreeListIndex(f);

	f->m_prevFree = null;
	f->m_nextFree = m_freeList[index];
	if (f->m_nextFree)
		f->m_nextFree->m_prevFree = f;
	m_freeList[index] = f;
	m_freeListMask |= (1<<index);
}

/*****************************************************************************
 *
 * Function:		MemoryPool::validateFreeList()
 *
 * Description:		Debug function for checking the validity of the current free list
 *
 *****************************************************************************/

DPVS_FORCE_INLINE void MemoryPool::validateFreeList (void) const
{
	int cnt		= 0;
	int size	= 0;
	int largest = 0;
	
	for (int i = 0; i < 32; i++)
	for (const FreeHeader* h = m_freeList[i]; h; h = h->m_nextFree)
	{
		DPVS_ASSERT (h);
		DPVS_ASSERT (h->isFree());		
		DPVS_ASSERT (h->getPrev());
		DPVS_ASSERT (h->getNext());
		DPVS_ASSERT (h->getPrev()->getNext() == static_cast<const Header*>(h));
		DPVS_ASSERT (h->getPrev()->getType() == Header::BLOCK_USED || h->getPrev()->getType() == Header::BLOCK_SENTINEL);
		DPVS_ASSERT (h->getNext()->getPrev() == static_cast<const Header*>(h));
		DPVS_ASSERT (h->getNext()->getType() == Header::BLOCK_USED || h->getNext()->getType() == Header::BLOCK_SENTINEL);
		size += getBlockSize(h);
		if (getBlockSize(h) > largest)
			largest = getBlockSize(h);
		cnt++;
	}

//	printf ("free blocks = %d, total free size = %d, largest = %d, avg = %.0f\n",cnt,size,largest,cnt ? (float)(size)/cnt : 0);
//	printf ("used = %d, reserved = %d\n",(int)getMemoryUsed(),(int)getMemoryReserved());
}

/*****************************************************************************
 *
 * Function:		MemoryPool::msize()
 *
 * Description:		Returns size of allocated memory block 'p'
 *
 * Parameters:		p = pointer to memory (may be null)
 *
 * Notes:			The function DPVS_ASSERTs in debug build that 'p' has been 
 *					allocated from the memory pool. In release build an
 *					invalid msize() call _may_ crash
 *
 *****************************************************************************/

int MemoryPool::msize (void* p) const
{
	if (!p)
		return 0;

	Header* h = reinterpret_cast<Header*>(p)-1;					// get the header
	bool ok = (h->isUsed());									// valid used block?
	DPVS_ASSERT(ok);											// assert in debug build..
	if (!ok)													// .. but survive in release build..
		return 0;
	return getBlockSize(reinterpret_cast<Header*>(p)-1);		// this takes the header into account
}

/*****************************************************************************
 *
 * Function:		MemoryPool::destroyPool()
 *
 * Description:		Unlinks a pool from the list
 *
 * Parameters:		f = pointer to the 'free area' of the pool
 *
 * Notes:			calls releaseMemoryBlock() to release the memory of the pool
 *
 *****************************************************************************/

void MemoryPool::destroyPool (FreeHeader* f)
{
	DPVS_ASSERT(f);
	DPVS_ASSERT(m_firstPool);
	DPVS_ASSERT(f->getType()			== Header::BLOCK_FREE);
	DPVS_ASSERT(f->getPrev()->getType() == Header::BLOCK_SENTINEL);
	DPVS_ASSERT(f->getNext()->getType() == Header::BLOCK_SENTINEL);

	Header* h = static_cast<Header*>(f)-1;		// sentinel
	Pool*   p = reinterpret_cast<Pool*>(h)-1;	// pointer to pool

	unlinkBlock(f);

	//------------------------------------------------------------------------
	// Unlink the pool from the pool list
	//------------------------------------------------------------------------

	if (p->m_prev)
		p->m_prev->m_next = p->m_next;
	else
	{
		DPVS_ASSERT(p == m_firstPool);
		m_firstPool = p->m_next;
	}
	if (p->m_next)
		p->m_next->m_prev = p->m_prev;

	m_totalSize -= p->m_allocationSize;			// release..
	DPVS_ASSERT(m_totalSize >= 0);				// HUH?
	DPVS_PROFILE(Statistics::incStatistic(Library::STAT_MEMORYEXTERNALOPERATIONS,1));
	releaseMemoryBlock (p->m_allocation);		// release the memory block back to the OS
}

/*****************************************************************************
 *
 * Function:		MemoryPool::createPool()
 *
 * Description:		Adds a new block of memory into the pool list
 * 
 * Parameters:		data = pointer to memory area
 *					size = size of memory area in bytes
 *
 * Notes:			This function is called only by expand()
 *
 *****************************************************************************/

DPVS_FORCE_INLINE void MemoryPool::createPool (void* data, int size)
{
	DPVS_ASSERT (data);

	//------------------------------------------------------------------------
	// Perform alignment of data and size (to guarantee properly aligned
	// allocations later on).
	//------------------------------------------------------------------------

	UPTR udata = reinterpret_cast<UPTR>(data);
	udata = udata + (UPTR)(ALIGNMENT-1) &~ (UPTR)(ALIGNMENT-1);				// beginning of next aligned block

	Pool*	p	= reinterpret_cast<Pool*>(udata);
	char*	d	= reinterpret_cast<char*>(p+1);	// skip past pool header
	int		sz  = (int)((((reinterpret_cast<char*>(data) + size)-d)) &~ (UPTR)(ALIGNMENT-1));

	DPVS_ASSERT(sz >= (int)(2*sizeof(Header)+sizeof(FreeHeader)));	// minimum size!

	// d  = pointer in memory where we can assign first block
	// sz = adjusted size of the pool 

	//------------------------------------------------------------------------
	// Create the pool data
	//------------------------------------------------------------------------

	p->m_allocation		= data;									// pointer to _original_ data
	p->m_allocationSize = size;									// original allocation size (just for book-keeping)
	p->m_prev			= null;									// link pool into global linked list
	p->m_next			= m_firstPool;
	if (m_firstPool)
		m_firstPool->m_prev = p;
	m_firstPool = p;

	m_totalSize += p->m_allocationSize;

	//------------------------------------------------------------------------
	// Create sentinels and free block
	//------------------------------------------------------------------------

	Header*		head	= reinterpret_cast<Header*>(d);
	Header*		tail	= reinterpret_cast<Header*>(d+sz-(int)sizeof(Header));
	FreeHeader* h		= reinterpret_cast<FreeHeader*>(head+1);

	head->set	(Header::BLOCK_SENTINEL, null,	h);
	tail->set	(Header::BLOCK_SENTINEL, h,		null);
	h->set		(Header::BLOCK_FREE,	 head,	tail);

	//------------------------------------------------------------------------
	// Link the free block to the global free list (head)
	//------------------------------------------------------------------------

	linkBlock(h);
}


/*****************************************************************************
 *
 * Function:		MemoryPool::expand()
 *
 * Description:		Expands the memory pool by 'n' bytes (or fails!)
 *
 * Returns:			true of success, false on failure
 * 
 *****************************************************************************/

DPVS_FORCE_INLINE bool MemoryPool::expand (size_t bytes)
{
	if (bytes < MINIMUM_POOL_SIZE)
		bytes = MINIMUM_POOL_SIZE;

	DPVS_PROFILE(Statistics::incStatistic(Library::STAT_MEMORYEXTERNALOPERATIONS,1));
	void* mem = allocateMemoryBlock(bytes);
	if (!mem)
		return false;								// failed to allocate more memory

	createPool (mem, bytes);						// create a new pool
	return true;
}

/*****************************************************************************
 *
 * Function:		MemoryPool::allocateInternal()
 *
 * Description:		Internal routine for allocating memory from the pool
 *
 * Parameters:		bytes = number of bytes of memory to allocate
 *
 * Returns:			pointer to memory block or null if allocation fails
 *
 *****************************************************************************/

// A tiny trick to shut up the compiler
DPVS_FORCE_INLINE bool MemoryPool::needAlignment (void)  { return (sizeof(FreeHeader) > ALIGNMENT); }

#if !defined (DPVS_BUILD_CW)
DPVS_FORCE_INLINE 
#endif
void* MemoryPool::allocateInternal (size_t bytes)
{
	PROFILE_CODE(m_allocs++);

	//--------------------------------------------------------------------
	// If we use the fixed allocators for small allocations, let's do it
	// here..
	//--------------------------------------------------------------------

#if defined (MEMORYPOOL_FIXED)
	if ((bytes>>4)<=FIXED_POOL_COUNT)
	{
		int size = (bytes+sizeof(FixedHeader)+ALIGNMENT-1)&~(ALIGNMENT-1);	// round memory
		int pool = (size>>4)-1;												// select pool
		if (pool < FIXED_POOL_COUNT)
			return allocateFixed(size);
		// FALLTHRU!
	}
#endif

	//--------------------------------------------------------------------
	// Align the block size (including the header) to ALIGNMENT. Make 
	// sure the allocation is at least as big as FreeHeader (otherwise
	// we may end up in trouble later).
	//--------------------------------------------------------------------
	
	int allocSize = (int)bytes + (int)sizeof(Header);

	if (needAlignment())									// if we need to clamp it
		if (allocSize < (int)sizeof(FreeHeader))			// clamp to FreeHeader size
			allocSize = (int)sizeof(FreeHeader);

	allocSize = (allocSize + ALIGNMENT-1) &~ (ALIGNMENT-1);

	//--------------------------------------------------------------------
	// Find a free block that can hold the allocation. If no such block
	// can be found, return 'null'. The free blocks are kept in separate
	// lists based on their sizes. We start the search from the smallest
	// slot that can fit allocs of this size and proceed from there..
	// The localized algorithm is a "best fit".
	//--------------------------------------------------------------------

	FreeHeader* h = null;

	{
		int			bestSize	= 0x7fffffff;
		UINT32		searchMask	= ((1<<(getFreeListIndex(allocSize)+1))-1) & m_freeListMask;	// index

		PROFILE_CODE(m_statAllocs++);

		while (searchMask)
		{
			UINT32 index = getHighestSetBit(searchMask);		// first free slot
			searchMask &= ~(1<<index);							// remove the bit from the search mask
			DPVS_ASSERT(m_freeList[index]);						// WASS??

			for (FreeHeader* f= m_freeList[index]; f; f = f->m_nextFree)
			{
				PROFILE_CODE(m_statIters++);

				DPVS_ASSERT(f->isFree());

				int blockSize = getBlockSize(f);
				if (blockSize >= allocSize && blockSize < bestSize) 
				{
					h			= f;
					bestSize	= blockSize;
					if (bestSize < (allocSize+(int)sizeof(FreeHeader)))	// can't find any better, really..
						break;
				}
			}

			if (h)												// we know that all subsequent allocs are going to be in a bigger slot
				break;
		}

		if (!h)
			return null;										// FAULIER!
	}


	DPVS_ASSERT ( h );
	DPVS_ASSERT ( h->isFree());								// WASS??

	//--------------------------------------------------------------------
	// Check if the allocation would shrink the free block smaller than
	// a FreeHeader structure -> in such a case we delete the FreeBlock 
	// entry (the allocation block itself will use slightly too much
	// memory).
	//--------------------------------------------------------------------

	int remainder = getBlockSize(h) - allocSize;			// size of the block
	if (remainder < (int)sizeof(FreeHeader))				// if we would consume the entire block...
	{
		unlinkBlock(h);
		m_memUsed+=getBlockSize(h);
		h->setType(Header::BLOCK_USED);						// set block type to "used"
//		DEBUG_CODE(validateFreeList());
		void* p = (static_cast<Header*>(h))+1;				// skip header
		DPVS_ASSERT ((reinterpret_cast<UPTR>(p) & (ALIGNMENT-1))==0);	// make sure ptr is aligned
		return p;											// we're done..
	}

	//--------------------------------------------------------------------
	// OK. There's still some memory to serve from this block.. So we 
	// keep the free block in its place but insert a new "used" block to
	// the very end of the free block's memory area.
	//--------------------------------------------------------------------

	unlinkBlock(h);	// DEBUG DEBUG TODO: WE ONLY NEED TO UNLINK/LINK IF THE BLOCK SIZE CHANGES..

	Header* used = reinterpret_cast<Header*>(reinterpret_cast<char*>(h) + remainder);
	Header* next = h->getNext();
	DPVS_ASSERT(next);
	used->set(Header::BLOCK_USED, h, next);
	next->setPrev(used);
	h->setNext (used);
	m_memUsed+=getBlockSize(used);
	DPVS_ASSERT(h->isFree());

	//--------------------------------------------------------------------
	// If our new free block is a good one (i.e. it is larger than
	// the current head), move it to the head in order to speed up 
	// searches in the future. This does not happen very often in
	// practice.
	//--------------------------------------------------------------------

	linkBlock(h);

	void* p = (void*)(used+1);												// skip header
	DPVS_ASSERT ((reinterpret_cast<UPTR>(p) & (ALIGNMENT-1))==0);	// make sure ptr is aligned
	return p;																// return pointer to memory block
}

/*****************************************************************************
 *
 * Function:		MemoryPool::allocate()
 *
 * Description:		Allocates memory from the pool
 *
 * Parameters:		bytes = number of bytes of memory to allocate
 *
 * Returns:			pointer to memory block or null if allocation fails
 *
 *****************************************************************************/

void* MemoryPool::allocate (size_t bytes)
{
#if defined (DPVS_DEBUG)
	DPVS_ASSERT(s_initialized);
#endif

#if defined (DPVS_DEBUG)
	int ctr = 0;
#endif

	for(;;)
	{
		//----------------------------------------------------------------
		// Attempt to allocate memory
		//----------------------------------------------------------------

		void* p = allocateInternal(bytes);
		if (p)
		{
			PROFILE_CODE(m_usedCount++;)			// increase allocation debug counter
			return p;
		}
#if defined (DPVS_DEBUG)
		ctr++;
		DPVS_ASSERT(ctr==1);						// we can pass this point only once!
#endif

		//----------------------------------------------------------------
		// If we failed, attempt a resize. If the resize fails, we return
		// null. Otherwise we *will* succeed on next attempt to allocate..
		//----------------------------------------------------------------

		const int ALLOC_GRANULARITY = (8192+sizeof(FixedPoolHeader) + ALIGNMENT+sizeof(Header)*2+sizeof(FreeHeader))*4;	// must be a power of two
		int size = (int)(bytes+MINIMUM_POOL_SIZE);	// + ALLOC_GRANULARITY) &~ (ALLOC_GRANULARITY-1);
		if (size < ALLOC_GRANULARITY)
			size = ALLOC_GRANULARITY;

		if (!expand(size))							// if expansion failed,
			break;									// return null
	}

	return null;									
}

/*****************************************************************************
 *
 * Function:		MemoryPool::releaseBlock()
 *
 * Description:		Releases block 'h' 
 *
 * Parameters:		h = pointer to block header
 *
 * Returns:			pointer to the free block 
 *
 * Notes:			The coalescing of free blocks is performed here. 
 *
 *****************************************************************************/

DPVS_FORCE_INLINE MemoryPool::FreeHeader* MemoryPool::releaseBlock(Header* h)
{
	DPVS_ASSERT (h);
//	DEBUG_CODE	(validateFreeList());
	
	m_memUsed-=getBlockSize(h);
	DPVS_ASSERT(m_memUsed >= 0);
	
	//--------------------------------------------------------------------
	// Now all we have to do is to perform coalescing of the blocks. There
	// are four different cases (based on status of left/right blocks). We
	// handle each case separately. Note that we _know_ there are valid
	// next and prev pointers (and nextNext *must* be valid if next is
	// a free block) because of the sentinels used.
	//--------------------------------------------------------------------

	FreeHeader* block		= reinterpret_cast<FreeHeader*>(h);					// it is really a FreeHeader
	FreeHeader* prev		= reinterpret_cast<FreeHeader*>(block->getPrev());	// these may or may not be FreeHeaders
	FreeHeader* next		= reinterpret_cast<FreeHeader*>(block->getNext());
	Header*		nextNext	= next->getNext();

	DEBUG_CODE(
	DPVS_ASSERT (prev != block && next != block);
	if (prev)
		DPVS_ASSERT (prev != next);		
	)

	if (prev->isFree())
	{
		//--------------------------------------------------------------------
		// Case #1: Prev is free, next is free (combine all three blocks
		// together). Here we need to touch prev,this,next and nextnext.
		// The probability for this case is on average 1/3
		//--------------------------------------------------------------------

		unlinkBlock(prev);

		if (next->isFree())														
		{
			unlinkBlock(next);
			DPVS_ASSERT(nextNext);									
			prev->setNext(nextNext);
			nextNext->setPrev(prev);
			PROFILE_CODE(m_rem0++;)
		} else
		{
			//--------------------------------------------------------------------
			// Case #2: Prev is free, next is used (all we need to do is to 
			// expand "prev"). We touch prev, this, next
			// The probability for this case is on average 1/6
			//--------------------------------------------------------------------

			prev->setNext(next);
			next->setPrev(prev);
			PROFILE_CODE(m_rem1++;)
		}

		linkBlock(prev);
		return prev;												// return pointer to free block
	}

	//--------------------------------------------------------------------
	// Case #3: Prev is used, next is free. This is the nasty case because 
	// we need to "enlarge" the next block to the left
	// The probability for this case is on average 1/6
	//--------------------------------------------------------------------

	if (next->isFree())						
	{
		DPVS_ASSERT (nextNext);										// must be there (due to sentinels)
		unlinkBlock (next);
		block->set (Header::BLOCK_FREE, prev, nextNext);
		prev->setNext(block);
		nextNext->setPrev(block);
		PROFILE_CODE(m_rem2++;)
	} else 
	{
		//--------------------------------------------------------------------
		// Case #4: Last case where both prev and next are "used". We need to 
		// create a new free entry and update both neighbors. We place this
		// entry to the tail of the free list so that we don't mess up the
		// ordering of the free list.
		// The probability for this case is on average 1/3
		//--------------------------------------------------------------------

		block->set (Header::BLOCK_FREE, prev, next);
		prev->setNext(block);
		next->setPrev(block);
		PROFILE_CODE(m_rem3++;)
	}

	linkBlock(block);												// link to proper free list
	return block;													// return pointer to the free block
}


/*****************************************************************************
 *
 * Function:		MemoryPool::release()
 *
 * Description:		Releases memory previously allocated from the pool
 * 
 * Parameters:		h = pointer to memory (may be null)
 *
 * Notes:			The function DPVS_ASSERTs in debug build that the block
 *					is truly in the memory pool and marked as "used".
 *
 *****************************************************************************/

void MemoryPool::release(void* p)
{
#if defined (DPVS_DEBUG)
	DPVS_ASSERT(s_initialized);
#endif

	Header* h = reinterpret_cast<Header*>(p);

	if (!h)													// handle null case
		return;
	h--;													// find the real header (skip)

	PROFILE_CODE(
		m_releases++;
		m_usedCount--;										// decrease allocation counter
		DPVS_ASSERT(m_usedCount >= 0);						// can't be true.
	)

	//--------------------------------------------------------------------
	// Some debug build code: check that pointer belongs to the data
	// block and has been marked as "USED"
	//--------------------------------------------------------------------

	DPVS_ASSERT ((reinterpret_cast<UPTR>(p) & (ALIGNMENT-1))==0);	// data is not aligned?

	//--------------------------------------------------------------------
	// If we're using the fixed block allocator, check if block is allocated
	// using it...
	//--------------------------------------------------------------------

#if defined (MEMORYPOOL_FIXED)
	if (h->getType() == Header::BLOCK_FIXED)
	{
		releaseFixed(h);									// call the local "release" routine
		return;												// we're done..
	}
#endif // MEMORYPOOL_FIXED

	//--------------------------------------------------------------------
	// Call internal function to release the block. The function returns
	// a pointer to the new free memory block.
	//--------------------------------------------------------------------

	DPVS_ASSERT (h->isUsed());								// releasing a non-allocated block?
	FreeHeader* f = releaseBlock(h);						// internal function that performs all the work
	DPVS_ASSERT(f);

	//--------------------------------------------------------------------
	// Here we should check that if prev and next blocks are sentinels,
	// the current pool has become empty and can be released. We never
	// release the "last" pool (i.e. always keep at least one pool).
	//--------------------------------------------------------------------

	if (f->getPrev()->getType() == Header::BLOCK_SENTINEL && 
		f->getNext()->getType() == Header::BLOCK_SENTINEL)
	{
		destroyPool (f);									// DIE!
		return;
	}
}


/*****************************************************************************
 *
 * Function:		MemoryPool::checkConsistency()
 *
 * Description:		Checks consistency of the memory pool
 * 
 *****************************************************************************/

void MemoryPool::checkConsistency (void)
{

//#if defined (DPVS_DEBUG)
	validateFreeList();
	PROFILE_CODE(
	if (m_allocs > 0)
	{
		printf ("allocs        = %d\n",m_allocs);
		printf ("releases      = %d\n",m_releases);
		printf ("iters/alloc   = %.3f\n",((float)(m_statIters))/m_statAllocs);
		float total = (float)(m_rem0+m_rem1+m_rem2+m_rem3);
		printf ("rem0 = %d (%.2f%%)\n",m_rem0,(100.0f*m_rem0)/total);
		printf ("rem1 = %d (%.2f%%)\n",m_rem1,(100.0f*m_rem1)/total);
		printf ("rem2 = %d (%.2f%%)\n",m_rem2,(100.0f*m_rem2)/total);
		printf ("rem3 = %d (%.2f%%)\n",m_rem3,(100.0f*m_rem3)/total);
	}
	) // PROFILE_CODE

//#endif
}

//------------------------------------------------------------------------

