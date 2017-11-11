#ifndef __DPVSARRAY_HPP
#define __DPVSARRAY_HPP
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
 * Desc:	Array classes
 *
 * $Archive: /dpvs/implementation/include/dpvsArray.hpp $
 * $Author: wili $ 
 * $Revision: #1 $
 * $Modtime: 7/12/01 4:17p $
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
 * Class:			DPVS::Array
 *
 * Description:		Basic Java-style array class that supports resizing of
 *					the array while preserving m_data.
 *
 * Notes:			The array doesn't automatically expand itself from
 *					out-of-range reads/writes (these are DPVS_ASSERTed in
 *					debug build). If such behavior is needed, use
 *					the DPVS::DynamicArray class instead.
 *
 ******************************************************************************/

template<class T> class Array
{
private:
	void			free		(void)								{ DELETE_ARRAY(m_data); m_data = 0; m_elements = 0; }

	T*				m_data;											// m_data m_elements (or 0 if zero-length array)
	int				m_elements;										// number of m_elements alloced (length of array)
	static void		copyElements (T* dst, const T* src, int cnt) { DPVS_ASSERT (cnt>=0); if (cnt>0) { DPVS_ASSERT (dst && src); for (int i = 0; i < cnt; i++) dst[i] = src[i]; } }
public:
					Array		(void)								: m_data(0),m_elements(0) { /* nothing to do */ }
	explicit		Array		(int size)							: m_data(0),m_elements(0) { DPVS_ASSERT (size>=0); reset (size); }
					Array		(const Array<T>& src)				: m_data(0),m_elements(0) { *this = src; }
					Array		(const T* src, int size)			: m_data(0),m_elements(0) { reset (size); copyElements (m_data,src,size); }
					~Array		(void)								{ DELETE_ARRAY(m_data);}
	Array<T>&		operator=	(const Array<T> &src)				{ if (&src != this) { free(); if (src.getSize()) { reset (src.getSize()); copyElements (m_data, src.getPtr(), m_elements); } } return *this;	}
	const T&		operator[]	(int i) const;
	T&				operator[]	(int i);
	T*				getPtr		(void) const						{ return m_data; }
	int				getSize		(void) const						{ return m_elements; }
	int				getByteSize	(void) const						{ return (m_elements * sizeof(T)); }
	void			resize		(int newSize)		// copies existing elements
	{
		DPVS_ASSERT (newSize>=0);
		if(m_elements != newSize)
		{
			T* tmp = null;

			if(newSize > 0)
			{
				tmp = NEW_ARRAY<T>(newSize);
				if(m_data && (m_elements > 0))
				{
					int sz = (m_elements <= newSize) ? m_elements : newSize;
					copyElements (tmp,m_data,sz);
				}
			}
			this->free();
			m_data		= tmp;
			m_elements	= newSize;
		}
	}

	void			reset		(int newSize)		// doesn't copy existing elements
	{
		DPVS_ASSERT (newSize>=0);
		if(m_elements == newSize)
			return;
		this->free();
		if (newSize > 0)
		{
			m_data		= NEW_ARRAY<T>(newSize);
			m_elements	= newSize;
		}
	}
};

template <class T> DPVS_FORCE_INLINE const T&	Array<T>::operator[]	(int i) const	{ DPVS_ASSERT(i >= 0 && i < (int)m_elements); return m_data[i]; }
template <class T> DPVS_FORCE_INLINE T&			Array<T>::operator[]	(int i)			{ DPVS_ASSERT(i >= 0 && i < (int)m_elements); return m_data[i]; }


/******************************************************************************
 *
 * Class:			DPVS::DynamicArray
 *
 * Description:		Dynamic-length array class
 *
 * Notes:			Dynamic-length arrays can be used when linear storage
 *					format is needed (the overhead of linked list management
 *					per element would be too big) and maximum length of the
 *					array is not known in advance.
 *
 *					Because the array can move in memory (as it grows), DO NOT
 *					use pointers to m_elements in the array (use indices instead).
 *
 ******************************************************************************/

template<class T> class DynamicArray
{
private:
	T*				m_data;				// array pointer
	size_t			m_elements;			// number of m_elements alloced
	T*				getPtr			(size_t);
public:
	DPVS_FORCE_INLINE				DynamicArray	(void);
	DPVS_FORCE_INLINE explicit		DynamicArray	(size_t sz);
	DPVS_FORCE_INLINE				DynamicArray	(const DynamicArray& src);

	DPVS_FORCE_INLINE				~DynamicArray	(void);
	DPVS_FORCE_INLINE DynamicArray&	operator=		(const DynamicArray&);

	DPVS_FORCE_INLINE void			copy			(size_t dstOffset, const T* src, size_t N);
	DPVS_FORCE_INLINE void			reset			(size_t);
	DPVS_FORCE_INLINE void			resize			(size_t);
	DPVS_FORCE_INLINE void			free			(void);	
	DPVS_FORCE_INLINE size_t		getElements		(void) const;
	DPVS_FORCE_INLINE size_t		getByteSize		(void) const; 
	DPVS_FORCE_INLINE T&			operator[]		(size_t) const;
	DPVS_FORCE_INLINE T&			getElement		(size_t);

};

//------------------------------------------------------------------------
// Implementation
//------------------------------------------------------------------------

// copies data into the array (this is the fastest way to perform copies as
// range checkes are made only once)

template <class T> DPVS_FORCE_INLINE void DynamicArray<T>::copy (size_t dstOffset, const T* src, size_t N)
{
	DPVS_ASSERT(dstOffset>=0);
	getPtr (dstOffset+N);	// make sure we have enough space
	T* dst = m_data + dstOffset;
	for (size_t i = 0; i < N; i++)
		dst[i] = src[i];
}

template <class T> DPVS_FORCE_INLINE void DynamicArray<T>::free (void)	
{ 
	DELETE_ARRAY(m_data);
	m_data		= 0; 
	m_elements	= 0; 
}

template <class T> DPVS_FORCE_INLINE void DynamicArray<T>::reset(size_t newSize)
{
	this->free();
	if(newSize > 0)
	{
		m_elements = newSize;
		m_data = NEW_ARRAY<T>(m_elements);
		DPVS_ASSERT (m_data);
	}
}

template <class T> DPVS_FORCE_INLINE DynamicArray<T>& DynamicArray<T>::operator= (const DynamicArray<T>& src)
{
	if (&src != this)
	{
		reset (src.m_elements);		// allocate memory
		for (size_t i = 0; i < src.m_elements; i++)
			m_data[i] = src.m_data[i];

	}
	return *this;
}

template <class T> DPVS_FORCE_INLINE DynamicArray<T>::~DynamicArray (void)
{
	DELETE_ARRAY(m_data); 
} 


template <class T> DPVS_FORCE_INLINE T& DynamicArray<T>::operator[] (size_t i) const	
{ 
	DPVS_ASSERT (m_data && i < m_elements); 
	return m_data[i];	
}


template <class T> DPVS_FORCE_INLINE DynamicArray<T>::DynamicArray (void) : m_data(0),m_elements(0) 
{ 
	// nada
}

template <class T> DPVS_FORCE_INLINE DynamicArray<T>::DynamicArray (size_t sz) : m_data(0),m_elements(0) 
{ 
	if (sz) 
		reset (sz); 
}

template <class T> DPVS_FORCE_INLINE DynamicArray<T>::DynamicArray	(const DynamicArray& src) : m_data(0),m_elements(0) 
{ 
	DPVS_ASSERT(&src);
	*this = src; 
}

template <class T> DPVS_FORCE_INLINE size_t DynamicArray<T>::getElements(void) const	
{ 
	return (m_elements); 
}

template <class T> DPVS_FORCE_INLINE size_t DynamicArray<T>::getByteSize(void) const 
{ 
	return (m_elements * sizeof(T)); 
}

template <class T> DPVS_FORCE_INLINE void DynamicArray<T>::resize (size_t newSize)
{
	if(m_elements != newSize)
	{
		T* tmp = 0;
		if(newSize > 0)
		{
			tmp = NEW_ARRAY<T>(newSize);
			if(m_data)
			{
				size_t smaller = (m_elements < newSize) ? m_elements : newSize;
				for (size_t i = 0; i < smaller; i++)
					tmp[i] = m_data[i];
			}
		}
		DELETE_ARRAY(m_data);
		m_data		= tmp;
		m_elements	= newSize;
	}
}

template <class T> DPVS_FORCE_INLINE T* DynamicArray<T>::getPtr (size_t ind)
{
	DPVS_ASSERT (ind>=0);
	if(ind >= m_elements)
		resize (ind+m_elements+8);				// enlarge container
	return (m_data + ind);
}

template <class T> DPVS_FORCE_INLINE T& DynamicArray<T>::getElement (size_t ind)			
{ 
	if(ind >= m_elements)
		resize (ind+m_elements+8);				// enlarge container
	return m_data[ind];
}

/******************************************************************************
 *
 * Class:			DPVS::Guard
 *
 * Description:		Class for subjecting a pointer to out-of-range asserts
 *					(debug build only).
 *
 * Notes:			min is inclusive, max is exclusive
 *
 ******************************************************************************/

#if defined (DPVS_DEBUG)
	template<class T> class Guard
	{
	private:
		T*		m_data;
		int		m_max;
		int		m_min;
		bool	m_readOnly;
	public:
						Guard		(T* data=0, int mn=0, int mx=0)		: m_data(data),m_max(mx), m_min(mn), m_readOnly(false)								{}
		explicit		Guard		(const Array<T>& src)				: m_data(src.getPtr()),m_max(src.getSize()),m_min(0),m_readOnly(true)				{}
		explicit		Guard		(const DynamicArray<T>& src)		: m_data(&src[0]),m_max(src.getElements()),m_min(0),m_readOnly(true)				{}
		explicit		Guard		(DynamicArray<T>& src)				: m_data(&src[0]),m_max(src.getElements()),m_min(0),m_readOnly(false)				{}
		explicit		Guard		(Array<T>& src)						: m_data(src.getPtr()),m_max(src.getSize()),m_min(0), m_readOnly(false)				{}
						Guard		(const Guard<T>& src)				: m_data(src.m_data),m_max(src.m_max),m_min(src.m_min),m_readOnly(src.m_readOnly)	{}
		T*				getPtr		(void) const						{ return m_data; }
		Guard<T>&		operator=	(const Guard<T> &src)				{ m_data=src.m_data; m_max=src.m_max; m_min=src.m_min; return *this;	}
		const T&		operator[]	(int i) const;
		T&				operator[]	(int i);
	};

	template <class T> DPVS_FORCE_INLINE const T&	Guard<T>::operator[]	(int i) const	{ DPVS_ASSERT(m_data); DPVS_ASSERT(i>=m_min && i<m_max); return m_data[i]; }
	template <class T> DPVS_FORCE_INLINE T&			Guard<T>::operator[]	(int i)			{ DPVS_ASSERT(m_data); DPVS_ASSERT(i>=m_min && i<m_max); DPVS_ASSERT(!m_readOnly); return m_data[i]; }
#else
	template<class T> class Guard
	{
	private:
		T*	m_data;
	public:
						Guard		(T* data=0, int=0, int=0)			: m_data(data)			{}
		explicit		Guard		(Array<T>& src)						: m_data(src.getPtr())	{}
		explicit		Guard		(const Array<T>& src)				: m_data(src.getPtr())	{}
		explicit		Guard		(const DynamicArray<T>& src)		: m_data(&src[0])		{}
						Guard		(const Guard<T>& src)				: m_data(src.m_data)	{}
		T*				getPtr		(void) const						{ return m_data; }
		Guard<T>&		operator=	(const Guard<T> &src)				{ m_data=src.m_data; return *this;	}
		const T&		operator[]	(int i) const						{ return m_data[i]; }
		T&				operator[]	(int i)								{ return m_data[i]; }
	};

#endif // DPVS_DEBUG

}// DPVS
//------------------------------------------------------------------------
#endif // __DPVSARRAY_HPP
