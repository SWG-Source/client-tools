#ifndef __DPVSTEMPALLOCATOR_HPP
#define __DPVSTEMPALLOCATOR_HPP
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
 * Desc:	Temporary data allocator
 *
 * $Archive: /dpvs/implementation/include/dpvsTempAllocator.hpp $
 * $Author: wili $ 
 * $Revision: #1 $
 * $Modtime: 7.05.01 12:06 $
 * $Date: 2003/01/22 $
 * 
 *****************************************************************************/

#if !defined (__DPVSARRAY_HPP)
#	include "dpvsArray.hpp"
#endif

namespace DPVS
{
/******************************************************************************
 *
 * Class:			DPVS::TempAllocator
 *
 * Description:		Global allocator for temporary memory allocations
 *
 * Notes:			The temporary allocator always serves 16-byte aligned memory
 *
 ******************************************************************************/

class TempAllocator
{
private:

	struct AllocationInfo
	{
		char*	m_ptr;									// data ptr (in case we performed an external allocation)
		int		m_size;									// size of alloc (if we used our own allocation)
	};

	int									m_size;			// size of the temporary memory buffer
	int									m_used;			// number of bytes used (in temp buffer)
	char*								m_alloc;		// real data allocation
	char*								m_data;			// 16-byte aligned data pointer to temp buffer
	int									m_stackTop;		// stack top index
	DPVS::DynamicArray<AllocationInfo>	m_allocs;		// stack of allocation sizes

					TempAllocator	(const TempAllocator&);
	TempAllocator&	operator=		(const TempAllocator&);

public:
					TempAllocator	(void);
					~TempAllocator	(void);
	void*			allocate		(int bytes);
	int				getSize			(void) const			{ return m_size; }
	bool			isEmpty			(void) const			{ return m_used == 0; }
	void			release			(void);
	void			setSize			(int sizeInBytes);
};

extern TempAllocator g_tempAllocator;					// global !

} 

//------------------------------------------------------------------------
#endif // __DPVSTEMPALLOCATOR_HPP

