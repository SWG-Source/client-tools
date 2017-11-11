#ifndef __DPVSMEMORYPOOL_HPP
#define __DPVSMEMORYPOOL_HPP
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
 *
 * Desc:	Memory manager
 *
 * $Archive: /dpvs/implementation/include/dpvsMemoryPool.hpp $
 * $Author: wili $ 
 * $Revision: #2 $
 * $Modtime: 19.06.01 15:07 $
 * $Date: 2004/09/23 $
 * 
 *****************************************************************************/

#if !defined (__DPVSPRIVATEDEFS_HPP)
#	include "dpvsPrivateDefs.hpp"
#endif

namespace DPVS
{

/******************************************************************************
 *
 * Class:			DPVS::MemoryPool
 *
 * Description:		A memory manager class that serves memory from a dynamic-size
 *					data pool
 *
 * Notes:			The memory allocation routine always returns 16-byte aligned 
 *					memory blocks.
 *
 *					When the pool needs more memory, it makes a call to the
 *					function allocateMemoryBlock(). The user
 *					of the class should implement this function. Such allocations
 *					are later released by calls to releaseMemoryBlock(). These
 *					allocations are usually quite large (>=128 kB).
 *
 * Algorithm:		There are two kinds of blocks in the pool: FREE and USED.
 *					All blocks have an 8-byte header ("Header") that contains
 *					a doubly-linked list of all blocks in the pool. The linked
 *					list corresponds with the physical locations of the blocks,
 *					so the "prev" field is a pointer to the (physically) previous
 *					block in the heap.
 *
 *					Additionally, the FREE blocks are organized into separate
 *					linked lists (embedded into their data area) based on their
 *					sizes. Up to 32 lists (nextPowerOfTwo(size)) are maintained.
 *
 *					The allocation policy is best-fit. Free block coalescing
 *					is always performed in the release() routine; this means
 *					that there are at most (used+1) FREE blocks in existance and
 *					there are never two or more consecutive FREE blocks. 
 *
 *					The physical location list uses sentinels
 *					for avoiding comparisons in linked list management.
 *
 *					Small allocations (<256 bytes) are handled by separate
 *					fixed-size block allocators (16 individual allocators).
 *
 *					See documentation of the individual functions for further
 *					information about the algorithms used.
 *
 ******************************************************************************/

#define MEMORYPOOL_FIXED				// use fixed pools as well
//#define MEMORYPOOL_PROFILE
#if defined (MEMORYPOOL_PROFILE)
#	define PROFILE_CODE(X) X
#else
#	define PROFILE_CODE(X)
#endif

class MemoryPool
{
public:
					MemoryPool			(void);
					~MemoryPool			(void);
	void*			allocate			(size_t bytes);
	void			checkConsistency	(void);
	int				getMemoryReserved	(void) const		{ return m_totalSize;	}
	int				getMemoryUsed		(void) const		{ return m_memUsed;		}
	int				msize				(void* p) const;
	void			release				(void* p);
	void			releaseAll			(void);					// used to shutdown the system..
protected:
	void			releaseMemoryBlock  (void*); 
	void*			allocateMemoryBlock (size_t);
private:
					MemoryPool			(const MemoryPool&);	// not permitted
	MemoryPool&		operator=			(const MemoryPool&);	// not permitted

	enum
	{
		ALIGNMENT = 16								// data alloc alignment (must be a power of two!)
	};

	//--------------------------------------------------------------------
	// Block Header
	//--------------------------------------------------------------------

	struct Header									// header for both free and used blocks
	{
	private:
		// the "type" field is encoded into the bottom two bits of "m_prev"
		UPTR	m_next;								// pointer to next entry
		UPTR	m_prev;								// pointer to previous entry (with type field encoded into lower 2 bits)
	public:

		
		enum Type									// NOTE: if these enums are changed, also change the hard-coded stuff in FixedHeader!!!
		{
			BLOCK_FREE		= 0,					// block is a free one
			BLOCK_USED		= 1,					// block is a used one
			BLOCK_SENTINEL	= 2,					// block is a sentinel
			BLOCK_FIXED		= 3						// block is a part of the fixed-size heap
		};

		DPVS_FORCE_INLINE bool		isFree  (void) const	{ return (m_prev&3) == BLOCK_FREE; }
		DPVS_FORCE_INLINE bool		isUsed  (void) const	{ return (m_prev&3) == BLOCK_USED; }
		DPVS_FORCE_INLINE Type		getType (void) const	{ return (Type)(m_prev&3); }
		DPVS_FORCE_INLINE Header*	getPrev (void) const	{ return reinterpret_cast<Header*>(m_prev&~3);	}
		DPVS_FORCE_INLINE Header*	getNext (void) const	{ return reinterpret_cast<Header*>(m_next);		}
		DPVS_FORCE_INLINE void		setType (Type t)		{ m_prev = (m_prev&~3) + (UPTR)(t); }
		DPVS_FORCE_INLINE void		setPrev (Header* h)		{ DPVS_ASSERT (!((UPTR)(h)&3)); m_prev = (m_prev&3) + (UPTR)(h); }
		DPVS_FORCE_INLINE void		setNext (Header* h)		{ m_next = (UPTR)(h);	}

		DPVS_FORCE_INLINE void		set		(Type t, Header* prev, Header* next)
		{
			DPVS_ASSERT((UPTR)(t)<=3);
			DPVS_ASSERT(!((UPTR)(prev)&3));
			m_next = reinterpret_cast<UPTR>(next);
			m_prev = reinterpret_cast<UPTR>(prev) + static_cast<UPTR>(t);

		}
		DPVS_FORCE_INLINE			Header	(void) : m_next(0), m_prev(0) { setType (BLOCK_SENTINEL); }

	};

	//--------------------------------------------------------------------
	// Free Block Header
	//--------------------------------------------------------------------

	struct FreeHeader : public Header			
	{
		FreeHeader*		m_prevFree;					// pointer to previous free block in free list
		FreeHeader*		m_nextFree;					// pointer to next free block in free list
		FreeHeader (void) : m_prevFree(null), m_nextFree(null) { setType(BLOCK_FREE); }
	};


	//--------------------------------------------------------------------
	// Fixed-size block allocators
	//--------------------------------------------------------------------

#if defined (MEMORYPOOL_FIXED)

	struct FixedHeader;
	struct FixedFreeHeader;

	struct FixedPoolHeader 
	{
		FixedPoolHeader*	m_prevPool;				// previous free FixedHeader of the same size
		FixedPoolHeader*	m_nextPool;				// next free FixedHeader of the same size
		FixedFreeHeader*	m_firstFree;			// first free element (or null if pool is closed)
		INT32				m_fixedIndex;			// index to MemoryPool Fixed List
		INT32				m_usedCount;			// # of FixedElements that are used (when zero we can free the header!)
	};

	struct FixedHeader								// Note that this is a copy of "Header"'s first DWORD!
	{
	private:
		UPTR				m_poolHeader;			// this is the same as the "m_prev" field in the standard headers..
	public:
		DPVS_FORCE_INLINE FixedPoolHeader*	getPoolHeader (void) const				{ return reinterpret_cast<FixedPoolHeader*>(m_poolHeader&~3);						}
		DPVS_FORCE_INLINE void				setPoolHeader (FixedPoolHeader* p)		
		{
			UPTR l = reinterpret_cast<UPTR>(p);
			DPVS_ASSERT(!(l&3));
			m_poolHeader = l + 3 /* Header::BLOCK_FIXED */;	
		}
	};

	struct FixedFreeHeader : public FixedHeader		// Note that this is a copy of "Header"'s first DWORD!
	{
		FixedFreeHeader*	m_nextFree;				// next free element in the same block
	};

	void*					allocateFixed		(int size);
	void					releaseFixed		(Header*);

	enum
	{
		FIXED_POOL_COUNT = 17
	};

	FixedPoolHeader*		m_fixedPools[FIXED_POOL_COUNT];
#endif

#if defined (DPVS_DEBUG)
	static bool s_initialized;
#endif

	//--------------------------------------------------------------------
	// Pool class
	//--------------------------------------------------------------------

	struct Pool
	{
		Pool*		m_prev;							// pointer to previous pool
		Pool*		m_next;							// pointer to next pool	
		void*		m_allocation;					// original memory allocation
		UPTR		m_allocationSize;				// size of original memory allocation
	};

	enum
	{
		MINIMUM_POOL_SIZE = (sizeof(Pool)+2*sizeof(Header)+sizeof(FreeHeader)+2*ALIGNMENT)
	};

	/*DPVS_FORCE_INLINE*/void*			allocateInternal	(size_t bytes);
	DPVS_FORCE_INLINE void			createPool			(void* data, int size);
	void							destroyPool			(FreeHeader* f);
	bool							expand				(size_t bytes);
	static DPVS_FORCE_INLINE int	getBlockSize		(const Header*);					
	static DPVS_FORCE_INLINE int	getFreeListIndex	(FreeHeader* f);
	static DPVS_FORCE_INLINE int	getFreeListIndex	(int size);
	DPVS_FORCE_INLINE void			linkBlock			(FreeHeader* f);
	static DPVS_FORCE_INLINE bool	needAlignment		(void);
	DPVS_FORCE_INLINE void			unlinkBlock			(FreeHeader* f);
	DPVS_FORCE_INLINE FreeHeader*	releaseBlock		(Header*);
	DPVS_FORCE_INLINE void			validateFreeList	(void) const;


	//--------------------------------------------------------------------
	// Member variables
	//--------------------------------------------------------------------

	Pool*			m_firstPool;					// pointer to first memory pool
	int				m_totalSize;					// total allocation size so far
	int				m_memUsed;						// # of bytes of memory reserved
	UINT32			m_freeListMask;					// free list bit mask indicating which freelist entries are free
	FreeHeader*		m_freeList[32];					// 32 free lists

	//--------------------------------------------------------------------
	// Extra data members used only when profiling or debugging
	//--------------------------------------------------------------------

	PROFILE_CODE(
	int			m_usedCount;						// # of used entries (used by debug code)
	int			m_allocs;							// # of times allocate() has been called
	int			m_releases;							// # of times release() has been called
	int			m_statIters;						// # of iterations (in first-fit search)
	int			m_statAllocs;						// # of allocate calls
	int			m_rem0;								// different release cases...
	int			m_rem1;							
	int			m_rem2;
	int			m_rem3;)
};

} // DPVS

//------------------------------------------------------------------------
#endif // __DPVSMEMORYPOOL_HPP

