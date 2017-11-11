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
 * Description:		Temporary memory management
 *
 * $Archive: /dpvs/implementation/sources/dpvsTempAllocator.cpp $
 * $Author: wili $ 
 * $Revision: #3 $
 * $Modtime: 14.06.01 17:09 $
 * $Date: 2003/02/21 $
 * 
 ******************************************************************************/

// DEBUG DEBUG TODO: ALIGNMENT ALREADY HANDLED BY MEMORY MANAGER!

#include "dpvsTempAllocator.hpp"
#include "dpvsStatistics.hpp"
//#include <cstdio> // DEBUG DEBUG
using namespace DPVS;

TempAllocator DPVS::g_tempAllocator;	// global allocator

/*****************************************************************************
 *
 * Function:		DPVS::TempAllocator::TempAllocator()
 *
 * Description:		
 *
 *****************************************************************************/

TempAllocator::TempAllocator (void) : 
	m_size(0), m_used(0), m_alloc(null),m_data(null),m_stackTop(0) 
{
	// nada
}

/*****************************************************************************
 *
 * Function:		DPVS::TempAllocator::setSize()
 *
 * Description:		
 *
 *****************************************************************************/

void TempAllocator::setSize  (int sizeInBytes)
{
	DPVS_ASSERT(sizeInBytes>=0);
	DPVS_ASSERT(isEmpty() && !m_stackTop);				// LEAK!

	m_allocs.free();									// release this as well..
	DELETE_ARRAY(m_alloc);
	m_alloc = null;
	m_data  = null;
	m_used = 0;
	m_size = sizeInBytes;

	if (sizeInBytes > 0)
	{
		m_alloc = NEW_ARRAY<char>(m_size+32);			// size in bytes..
		m_data  = (char*)(((UPTR)(m_alloc)+31)&~31);	// cache-line alignment
	} 
}

/*****************************************************************************
 *
 * Function:		DPVS::TempAllocator::~TempAllocator()
 *
 * Description:		
 *
 *****************************************************************************/

TempAllocator::~TempAllocator (void)
{
	DPVS_ASSERT(isEmpty() && !m_stackTop);				// LEAK!
	DELETE_ARRAY(m_alloc);
}

/*****************************************************************************
 *
 * Function:		DPVS::TempAllocator::allocate()
 *
 * Description:		
 *
 *****************************************************************************/

// We either serve the memory from the temporary buffer or by performing
// a real allocation. Real allocs are only used when we don't have enough
// available space in the temp buffer.

void* TempAllocator::allocate (int bytes)					
{
	bytes = (bytes+15)&~15;							// round to next 16 bytes
	
	AllocationInfo& n = m_allocs.getElement(m_stackTop++);

	if ((bytes + m_used) > m_size)					// can't serve from temporary buffer
	{
//		printf ("failed to allocate %d bytes\n",bytes);
		
		char* p = NEW_ARRAY<char>(bytes);			
		n.m_ptr  = p;
		n.m_size = bytes;
		return p;
	}

	// serve from our temporary buffer
	n.m_ptr  = 0;		// mark data pointer as null	
	n.m_size = bytes;	// mark size of allocation

	void* p = m_data + m_used;
	m_used += bytes;
	return p;
}

/*****************************************************************************
 *
 * Function:		DPVS::TempAllocator::release()
 *
 * Description:		
 *
 *****************************************************************************/

void TempAllocator::release (void)
{
	--m_stackTop;

	DPVS_ASSERT(m_stackTop >= 0);

	char* p		= m_allocs[m_stackTop].m_ptr;
	int   size	= m_allocs[m_stackTop].m_size;

	if (p)									// was allocated externally
	{
		DELETE_ARRAY(p);
		return;
	}
	
	m_used -= size;
	DPVS_ASSERT(m_used >= 0);				// WASS?
}

//------------------------------------------------------------------------
