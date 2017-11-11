#ifndef __DPVSTEMPARRAY_HPP
#define __DPVSTEMPARRAY_HPP
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
 * Desc:	Temporary data array class
 *
 * $Archive: /dpvs/implementation/include/dpvsTempArray.hpp $
 * $Author: wili $ 
 * $Revision: #1 $
 * $Modtime: 6/16/01 12:23p $
 * $Date: 2003/01/22 $
 * 
 *****************************************************************************/

#if !defined (__DPVSTEMPALLOCATOR_HPP)
#	include "dpvsTempAllocator.hpp"
#endif

namespace DPVS
{

/******************************************************************************
 *
 * Class:			DPVS::TempArray
 *
 * Description:		An Array class that behaves like a stack-based array, except
 *					that the data is allocated by the "temporary memory allocator".
 *					The data is destroyed when the array falls out of scope.
 *
 * Notes:			This class partially replaces the Array and Recycler classes.
 *					Use this whenever you need a temporary array in a function.
 *
 *					Note that you can disable the use of ctors/dtors by
 *					settings the second template parameter to false. This can
 *					be beneficial in some cases when arrays of vectors etc.
 *					are being constructed.
 *
 ******************************************************************************/

template<class T, bool USE_CTORS = true> class TempArray
{
private:
	T*		m_data;					// array
	int		m_size;					// # of elements in array

	DPVS_FORCE_INLINE bool	useConstructor (void) const { return USE_CTORS; }
public:
	DPVS_FORCE_INLINE explicit TempArray (int size) 
	{ 
		DPVS_ASSERT(size>=0);

		m_size = size;
		m_data = reinterpret_cast<T*>(g_tempAllocator.allocate(sizeof(T)*size));
		DPVS_ASSERT(m_data);
		DPVS_ASSERT(is128Aligned(m_data));		// WASS?

		if (useConstructor())
		{
			for (int i = 0; i < size; i++)
				new(m_data+i) T;				// placement new
		}
	}

	~TempArray (void)
	{
		if (useConstructor())
		{
			for (int i = 0; i < m_size; i++) 
				(m_data+i)->~T();
		}
		g_tempAllocator.release();
	}

	DPVS_FORCE_INLINE const T& operator[]	(int i) const
	{
		DPVS_ASSERT((unsigned int)(i)<(unsigned int)(m_size));
		return m_data[i];
	}
	
	DPVS_FORCE_INLINE T& operator[]	(int i)
	{
		DPVS_ASSERT((unsigned int)(i)<(unsigned int)(m_size));
		return m_data[i];
	}

	DPVS_FORCE_INLINE T* getPtr (void) const
	{
		return m_data;
	}	

private:
					TempArray		(const TempArray&);	// no-go
	TempArray&		operator=		(const TempArray&);	// no-go
//	static void*	operator new	(size_t);			// don't allow this!
//	static void		operator delete	(void*);			// don't allow this!
};


}// DPVS

//------------------------------------------------------------------------
#endif // __DPVSTEMPARRAY_HPP


