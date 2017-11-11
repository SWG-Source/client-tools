#ifndef __DPVSRECYCLER_HPP
#define __DPVSRECYCLER_HPP
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
 * Desc:	Recycler class
 *
 * $Archive: /dpvs/implementation/include/dpvsRecycler.hpp $
 * $Author: wili $ 
 * $Revision: #1 $
 * $Modtime: 14.05.01 12:46 $
 * $Date: 2003/01/22 $
 * 
 *****************************************************************************/

#if !defined (__DPVSPRIVATEDEFS_HPP)
#	include "dpvsPrivateDefs.hpp"
#endif

namespace DPVS
{
/******************************************************************************
 *
 * Class:			DPVS::Recycler
 *
 * Description:		Fast memory allocation template class for situations where 
 *					temporary working buffers (which can be overwritten) are needed 
 *					often. 
 *
 ******************************************************************************/

template<class T> class Recycler
{
private:

				Recycler	(const Recycler&);		// not allowed
	Recycler&	operator=	(const Recycler&);		// not allowed

	T*			m_data;								// data pointer to element array 
	size_t		m_elements;							// number of elements in the array
public:

	Recycler (void) : m_data(null),m_elements(0) {}		// init. constructor
	
	~Recycler()									
	{ 
		DELETE_ARRAY(m_data);
	}			

	T* allocate (size_t size)
	{
		if(size > m_elements)						// if we have enough space, just return the existing buffer
		{
			DELETE_ARRAY(m_data);
			m_elements = (size*11)/8 + 16;			// ...a linearly growing set of allocations doesn't cause mem allocs every time
			m_data = NEW_ARRAY<T>(m_elements);		// new T[m_elements];
			DPVS_ASSERT(is128Aligned(m_data));		// WASSS??
		}
		return m_data;								// return data pointer
	}
	
	T* getPtr (void) const
	{
		return m_data;								// return data pointer (must've been allocated first!)
	}

	void free (void)								// free allocated memory
	{ 
		DELETE_ARRAY(m_data);
		m_data		= null; 
		m_elements	= 0; 
	}	
	
	size_t getElements() const						// return size of the buffer in terms of elements
	{ 
		return m_elements; 
	}												
	
	size_t getByteSize() const						// return current allocated size in bytes
	{ 
		return m_elements * sizeof(T); 
	}												
};



} // DPVS
//------------------------------------------------------------------------
#endif // __DPVSARRAY_HPP
