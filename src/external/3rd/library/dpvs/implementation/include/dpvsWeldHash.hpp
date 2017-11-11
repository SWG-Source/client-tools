#ifndef __DPVSWELDHASH_HPP
#define __DPVSWELDHASH_HPP
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
 * Desc:	Weld hash template
 *
 * $Archive: /dpvs/implementation/include/dpvsWeldHash.hpp $
 * $Author: wili $ 
 * $Revision: #1 $
 * $Modtime: 14.06.01 17:56 $
 * $Date: 2003/01/22 $
 * 
 *****************************************************************************/

#if !defined (__DPVSHASHKEY_HPP)
#	include "dpvsHashKey.hpp"
#endif

#if !defined (__DPVSBITMATH_HPP)
#	include "dpvsBitMath.hpp"
#endif

#if !defined (__DPVSARRAY_HPP)
#	include "dpvsArray.hpp"
#endif

namespace DPVS
{
/******************************************************************************
 *
 * Class:			DPVS::WeldHash
 *
 * Description:		Hash class for welding operations
 *
 * Notes:			This is a simplified variant of the Hash template that
 *					is much faster when entries are only inserted once and
 *					never deleted - something that is very typical in
 *					"welding" operations.
 *
 ******************************************************************************/

template <class T> 
class WeldHash
{
public:

					WeldHash		(void);
					~WeldHash		(void);
	int				getIndex		(const T& src);
	int				getOffset		(void) const					{ return m_offset; }
	void			reset			(T* output, int maxSize);
private:
					WeldHash		(const WeldHash&);	// not allowed
	WeldHash&		operator=		(const WeldHash&);	// not allowed

	struct Entry
	{
		Entry*		m_next;			// next entry in same hash bucket
	};

	T*				m_output;		// output array (given by last call to reset)
	int				m_maxSize;		// max output size
	int				m_offset;		// current output offset
	Entry*			m_hash;			// pointer to m_entries
	UINT32			m_hashSize;		// size of the hash table (power of two)
	Array<Entry>	m_entries;		// # of entries
};

//------------------------------------------------------------------------
// Implementation
//------------------------------------------------------------------------

template <class T> inline WeldHash<T>::WeldHash (void) : 
	m_output(null),
	m_maxSize(0),
	m_offset(0),
	m_hash(null),
	m_hashSize(0)
{
	// nada
}

template <class T> inline void WeldHash<T>::reset (T* output, int maxSize)
{
	DPVS_ASSERT(maxSize >= 0);

	if (maxSize > 0)
	{
		m_maxSize  = maxSize;
		m_hashSize = getNextPowerOfTwo((maxSize*3)>>2);		
		if (m_hashSize < 1)
			m_hashSize = 1;
		
		int totalSize = m_maxSize + m_hashSize;
		if (m_entries.getSize() < totalSize)				// make sure we have enough space..
			m_entries.reset((totalSize*5)>>2);				// geometry series increase

		m_hash = &m_entries[m_maxSize];						// pointer to hash table
		
		fillByte (m_hash,0,sizeof(Entry) * m_hashSize);		// clear the hash table to 'null'
	} else
	{
		m_maxSize  = 0;
		m_hashSize = 0;
		m_entries.reset(0);									// release the memory
	}

	m_output   = output;
	m_offset   = 0;
}


template <class T> inline int WeldHash<T>::getIndex (const T& src)
{
	DPVS_ASSERT(m_output && m_maxSize > 0 && m_hash);
	// compute hash value
	UINT32 hashValue = HashKey<T>::getHashValue (src) & (m_hashSize-1);

	// first try to locate the entry from the hash table
	Entry*			entry = m_hash[hashValue].m_next;
	const Entry*	base  = &m_entries[0];
	while (entry)
	{
		int index = entry - base;				// calculate offset
		if (m_output[index] == src)
			return index;						// found it
		entry = entry->m_next;					// next in hash table
	}
	DPVS_ASSERT(m_offset < m_maxSize);			// WASS? OVERWRITE?
	entry = &m_entries[m_offset];				// allocate entry
	entry->m_next = m_hash[hashValue].m_next;	// link to hash table
	m_hash[hashValue].m_next = entry;			// link to hash table
	m_output[m_offset] = src;					// copy 'src' to output
	return m_offset++;							// return index and advance offset
}

template <class T> inline WeldHash<T>::~WeldHash (void)
{
	// nada
}

} // DPVS

//------------------------------------------------------------------------
#endif //__DPVSHASH_HPP
