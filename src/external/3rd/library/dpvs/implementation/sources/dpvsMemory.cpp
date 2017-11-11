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
 * $Archive: /dpvs/implementation/sources/dpvsMemory.cpp $
 * $Author: wili $ 
 * $Revision: #1 $
 * $Modtime: 2.10.02 15:34 $
 * $Date: 2003/01/22 $
 * 
 ******************************************************************************/

#include "dpvsMemory.hpp"
#include "dpvsStatistics.hpp"
#include "dpvsDebug.hpp"
#include "dpvsSort.hpp"
#include "dpvsMemoryPool.hpp"
#include "dpvsWrapper.hpp"
#include "dpvsRemoteDebugger.hpp"		// added to support new/delete in RemoteDebugger

#include <cstdio>
//------------------------------------------------------------------------
// Local memorypool class that directs all true allocs/releases to the
// library user
//------------------------------------------------------------------------

namespace DPVS
{
	MemoryPool g_memoryPool;					// memory pool
}

using namespace DPVS;

//------------------------------------------------------------------------
// Static variables
//------------------------------------------------------------------------

int		Memory::s_allocates = 0;				// number of allocations performed
int		Memory::s_releases  = 0;				// number of releases performed
//bool	Memory::s_isShutDown = false;

//------------------------------------------------------------------------
// Some stuff done only in the debug build
//------------------------------------------------------------------------

#if defined (DPVS_DEBUG)
#	include "dpvsSet.hpp"

namespace DPVS
{
	struct SetGuard
	{
		Set<void*>		m_allocSet;				// all allocs are inserted into this set
		bool			m_addToSet;				// temporary mutex

		SetGuard ()		{ m_addToSet = true; }
		~SetGuard ()	{ m_addToSet = false; }
	};

	SetGuard s_setGuard;						// static object that is created/destructed
} // DPVS

int g_freqTable[64];							// last slot reserved for "large allocs"

#endif // DPVS_DEBUG

/*****************************************************************************
 *
 * Function:		DPVS::Memory::allocateInternal()
 *
 * Description:		Internal memory allocation routine
 *
 *****************************************************************************/

DPVS_FORCE_INLINE void* Memory::allocateInternal (int sz)
{
	DPVS_PROFILE(Statistics::incStatistic(Library::STAT_MEMORYOPERATIONS,1)); 
	s_allocates++;

	void* p = g_memoryPool.allocate(sz);
	DPVS_ASSERT( is128Aligned(p));		// memory must be 128-bit aligned!
	return p;
}

/*****************************************************************************
 *
 * Function:		DPVS::Memory::releaseInternal()
 *
 * Description:		Internal memory release routine
 *
 *****************************************************************************/

DPVS_FORCE_INLINE void Memory::releaseInternal (void* p)
{
	DPVS_ASSERT(p);
	DPVS_ASSERT(is128Aligned(p));
	DPVS_PROFILE(Statistics::incStatistic(Library::STAT_MEMORYOPERATIONS,1)); 
	s_releases++;
	g_memoryPool.release(p);
}

/*****************************************************************************
 *
 * Function:		DPVS::Memory::allocate()
 *
 * Description:		Allocates 'sz' bytes of memory. Adds memory guards in
 *					debug build
 *
 *****************************************************************************/

void* Memory::allocate (int sz)
{
//	DPVS_ASSERT(!s_isShutDown);
	DPVS_API_ASSERT(sz>=0 && "DPVS::Memory::allocate() -- allocating memory block of negative size");

#if defined (DPVS_DEBUG)
	sz += sizeof(UPTR)-1;													// round to proper size
	sz &= ~(sizeof(UPTR)-1);	

	// frequency table updates (just for debugging)
	int freqOffset = sz>>2;
	if (freqOffset > 63)
		freqOffset = 63;
	g_freqTable[freqOffset]++;

	// we allocate 5 dwords of extra memory here. four are needed to keep
	// the data 128-bit aligned and the last one is needed for the rear guard.

	void*		d	= allocateInternal(sz+5*sizeof(UINT32));	
	UINT32*		id  = reinterpret_cast<UINT32*>(d);										
	UINT32*		id2 = reinterpret_cast<UINT32*>(((char*)d)+sz+4*sizeof(UINT32));	// REAR GUARD

	id[0] = sz;																		// store size of allocation
	id[1] = (UINT32)(0xaf342910);	// MAGIC NUMBER 1
	id[2] = (UINT32)(0x34984398);	// MAGIC NUMBER 2
	id[3] = (UINT32)(id);
	*id2  = (UINT32)(id2);			// store own address

	if (s_setGuard.m_addToSet)
	{
		s_setGuard.m_addToSet = false;												// we need this - otherwise endless recursion can occur when set enlargens itself
		DPVS_ASSERT(!s_setGuard.m_allocSet.contains((id+4)));
		s_setGuard.m_allocSet.insert((id+4));										// insert into set
		s_setGuard.m_addToSet = true;
	}

	::memset ((id+4),0xba, sz);														// fill with '0xba'

	return (id+4);																	// ok
#else
	return allocateInternal(sz);
#endif
}

/*****************************************************************************
 *
 * Function:		DPVS::Memory::isValidPointer()
 *
 * Description:		Determines if a pointer is valid (pointing to some
 *					allocated memory location). Check only done
 *					in debug build. NULL is not considered a valid pointer
 *					(this is checked even in release build).
 * Parameters:		p	 = pointer
 *
 * Returns:			true if pointer points to a memory location allocated
 *					from the Memory manager (or when this is called in
 *					release build), false otherwise
 *
 * Notes:			TODO: also perform overwrite checks here??
 *
 *****************************************************************************/

bool Memory::isValidPointer	(const void* p)
{
	if (!p)
		return false;												// NULL is not valid
#if defined (DPVS_DEBUG)
	if (s_setGuard.m_allocSet.contains(const_cast<void*>(p)))
	{
		checkInternal(p);											// execute memory overwrite checks
		return true;
	} else
		return false;												// faulier! Not found in allocation set!
#else
	return true;													// in release build consider all other pointers valid
#endif
}

/*****************************************************************************
 *
 * Function:		DPVS::Memory::checkConsistency()
 *
 * Description:		Executes internal consistency checks in debug build
 *
 *****************************************************************************/

void Memory::checkConsistency (void)
{
#if defined (DPVS_DEBUG)
	static volatile bool reEntrancyCheck = false;
	if (reEntrancyCheck)						// not re-entrant
		return;

	//--------------------------------------------------------------------
	// Execute consistency checks
	//--------------------------------------------------------------------

	reEntrancyCheck = true;

	g_memoryPool.checkConsistency();
	s_setGuard.m_allocSet.checkConsistency();

	//--------------------------------------------------------------------
	// Go through all memory entries and make sure there have been no
	// memory overwrites.
	//--------------------------------------------------------------------

	s_setGuard.m_addToSet = false;
	
	{
		Set<void*>::Array arr(s_setGuard.m_allocSet);		// query pointers to all memory allocs
		int				  size = arr.getSize();
		for (int i = 0; i < size; i++)
			checkInternal(arr[i]);
	}

	s_setGuard.m_addToSet = true;

	//--------------------------------------------------------------------
	// Done
	//--------------------------------------------------------------------

	reEntrancyCheck = false;

#endif
}

/*****************************************************************************
 *
 * Function:		DPVS::Memory::checkInternal()
 *
 * Description:		Performs internal validity check for memory block
 *					pointed by 'p'
 *
 * Parameters:		p = pointer to be checked
 *
 *****************************************************************************/

#if defined (DPVS_DEBUG)
void Memory::checkInternal (const void* p)
{
	DPVS_ASSERT (p != NULL);											// huh??
	const UINT32*	id		= reinterpret_cast<const UINT32*>(p)-4;		// go backwards to dwords
	int				size	= id[0];									// size was stored here
	const UINT32*	id2		= reinterpret_cast<const UINT32*>(reinterpret_cast<const char*>(p)+size);

	// TODO: other checks for size?
	DPVS_API_ASSERT(size >= 0 && "DPVS::Memory::checkInternal() - memory overwrite detected (front guard size failure)");

	DPVS_API_ASSERT((id[1] == ((UINT32)(0xaf342910))) && "DPVS::Memory::checkInternal() - memory overwrite detected (front guard magic failure)");
	DPVS_API_ASSERT((id[2] == ((UINT32)(0x34984398))) && "DPVS::Memory::checkInternal() - memory overwrite detected (front guard magic failure)");
	DPVS_API_ASSERT((id[3] == ((UINT32)id))			  && "DPVS::Memory::checkInternal() - memory overwrite detected (front guard failure)");
	DPVS_API_ASSERT((*id2  == ((UINT32)id2))		  && "DPVS::Memory::checkInternal() - memory overwrite detected (rear guard failure)");
}
#else
void Memory::checkInternal (const void*) {}
#endif

/*****************************************************************************
 *
 * Function:		DPVS::Memory::release()
 *
 * Description:		Releases data pointed by 'p'. P can be null. In debug
 *					build checks memory guards.
 *
 *****************************************************************************/

void Memory::release (void* p)
{
	if (!p)
		return;

//	DPVS_ASSERT(!s_isShutDown);

#if defined (DPVS_DEBUG)

	//--------------------------------------------------------------------
	// Check that set contains the pointer
	//--------------------------------------------------------------------

	if (s_setGuard.m_addToSet)	// the addToSet boolean is to make sure we don't add the Set allocations to the set (would cause infinite recursion)
	{
		s_setGuard.m_addToSet = false;

		DPVS_API_ASSERT(s_setGuard.m_allocSet.contains(p) && "DPVS::Memory::release() - trying to delete a pointer that has not been allocated!");
		s_setGuard.m_allocSet.remove(p);

		//--------------------------------------------------------------------
		// Last one leaving the house turns off the lights...
		//--------------------------------------------------------------------

		if (s_setGuard.m_allocSet.isEmpty())
			s_setGuard.m_allocSet.removeAll(true);
		s_setGuard.m_addToSet = true;
	}

	//--------------------------------------------------------------------
	// Memory guard checks
	//--------------------------------------------------------------------

	checkInternal(p);													// execute internal checks
	const UINT32*	id		= reinterpret_cast<const UINT32*>(p)-4;		// go backwards to dwords
	int				size	= id[0];									// size was stored here
	::memset (p,0xbe, size);											// fill with '0xbe'
	releaseInternal(const_cast<UINT32*>(id));	

	// frequency table updates (just for debugging)
	int freqOffset = size>>2;
	if (freqOffset > 63)
		freqOffset = 63;
	g_freqTable[freqOffset]--;

#else
	releaseInternal(p);
#endif
}

/*****************************************************************************
 *
 * Function:		DPVS::Memory::getMemoryUsed()
 *
 * Description:		Returns amount of memory used by DPVS
 *
 * Returns:			amount of memory used by DPVS in bytes
 *
 *****************************************************************************/

int Memory::getMemoryUsed (void)
{
	return g_memoryPool.getMemoryUsed();
}

/*****************************************************************************
 *
 * Function:		DPVS::Memory::getMemoryReserved()
 *
 * Description:		Returns amount of memory reserved by DPVS
 *
 * Returns:			amount of memory reserved by DPVS in bytes
 *
 *****************************************************************************/

int Memory::getMemoryReserved (void)
{
	return g_memoryPool.getMemoryReserved();
}

/*****************************************************************************
 *
 * Function:		DPVS::Memory::shutdown()
 *
 * Description:		Shutdown of the memory manager
 *
 *****************************************************************************/

void Memory::shutdown (void)
{
	//----------------------------------------------------------------
	// In debug build make sure that we're not leaking anything
	//----------------------------------------------------------------

#if defined (DPVS_DEBUG)
	int numMemoryAllocs = getAllocationCount();
	int memoryUsed      = g_memoryPool.getMemoryUsed();
	
	if (numMemoryAllocs || memoryUsed)		// we're leaking!
	{
		char tmp[256];
		sprintf (tmp,"Library::exit() - there are memory leaks! Number of leaks: %d. Number of bytes leaked: %d (actual allocation may be smaller).\n",numMemoryAllocs,memoryUsed);
		assertFail(tmp,__FILE__,__LINE__,"");		
	} 

	DPVS_ASSERT("There are memory leaks!" && s_setGuard.m_allocSet.isEmpty());

	s_setGuard.m_addToSet = false;
	s_setGuard.m_allocSet.removeAll(true);		// make sure that all set data is released...
	s_setGuard.m_addToSet = true;

#endif // DPVS_DEBUG

//	s_isShutDown = true;						// memory system is now officially shut down!
	g_memoryPool.releaseAll();					// kill the pool...

}

/*****************************************************************************
 *
 * Function:		DPVS::Memory::dump()
 *
 * Description:		Debugging info..
 *
 *****************************************************************************/

void Memory::dump (void)
{
#if defined (DPVS_DEBUG)

/*	s_setGuard.m_addToSet = false;	// don't add the alloc we're making here to the set itself

	{
		int		size	= s_setGuard.m_allocSet.getSize();
		void**	tmp		= NEW_ARRAY<void*>(size);

		s_setGuard.m_allocSet.getArray(tmp);


		quickSort(tmp,size);
		
		for (int i = 0; i < size; i++)
		{
			void* p = tmp[i];

			const unsigned long*	id		= (const unsigned long*)(p)-2;		// go backwards to dwords
			long					size	= id[0];

			Debug::print ("0x%08x: %d\n",p,size);
		}

		DELETE_ARRAY(tmp);
	}

	s_setGuard.m_addToSet = true;	// allow additions to the set again...
*/
	int tot = 0;
	for (int i = 0; i < 64; i++)
		tot += g_freqTable[i];

	int sum = 0;
	for (int i = 0; i < 63; i++)
	if (g_freqTable[i])
	{
		sum += ((i*4+4)*g_freqTable[i]+4095)&~4095;
		Debug::print ("%04d: %-8d %-6d kB (%.2f%%)\n",i*4,g_freqTable[i],((i*4+4)*g_freqTable[i]+4095)/1024,(100.0f*g_freqTable[i])/tot);
	}
	Debug::print ("<= 256 byte allocs = %d bytes\n",sum);
	Debug::print (">= 256 byte allocs = %d\n",g_freqTable[63]);

#endif

	g_memoryPool.checkConsistency();

}

//------------------------------------------------------------------------
// MALLOC and FREE functions..
//------------------------------------------------------------------------

namespace DPVS
{
	void* MALLOC (size_t s)
	{
		DPVS_ASSERT_INIT();
		void* p = Memory::allocate((int)s);
		DPVS_ASSERT(is128Aligned(p));
		DPVS_ASSERT(p);			// remove this once we can support MALLOCs that fail!!!
		return p;
	}

	void FREE (void* p)
	{
		if (p)
		{
			DPVS_ASSERT_INIT();
			DPVS_ASSERT(is128Aligned(p));
			Memory::release(p);			
		}
	}
}


//------------------------------------------------------------------------
// If we have a DLL build, override global operator new/delete just to make
// sure we don't make any calls to them (functions should always use 
// NEW and DELETE macros..)
//------------------------------------------------------------------------

#if !defined (DPVS_USE_REMOTE_DEBUGGER)

#if defined (DPVS_DLL) && defined (DPVS_DEBUG)
	void*	operator new		(size_t)	{	DPVS_ASSERT(false); return null;}
	void*	operator new[]		(size_t)	{	DPVS_ASSERT(false); return null;}
	void	operator delete		(void*)		{	DPVS_ASSERT(false);}
	void	operator delete[]	(void*)		{	DPVS_ASSERT(false);}
#endif // DPVS_DLL && DPVS_DEBUG

#endif // DPVS_USE_REMOTE_DEBUGGER

//------------------------------------------------------------------------
