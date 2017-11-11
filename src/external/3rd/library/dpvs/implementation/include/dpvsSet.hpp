#ifndef __DPVSSET_HPP
#define __DPVSSET_HPP
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
 * Desc:	Set class
 *
 * $Archive: /dpvs/implementation/include/dpvsSet.hpp $
 * $Author: wili $ 
 * $Revision: #1 $
 * $Modtime: 7.05.01 16:06 $
 * $Date: 2003/01/22 $
 * 
 *****************************************************************************/

#if !defined (__DPVSHASHKEY_HPP)
#	include "dpvsHashKey.hpp"
#endif

#if !defined (__DPVSBITMATH_HPP)
#	include "dpvsBitMath.hpp"
#endif

namespace DPVS
{
/******************************************************************************
 *
 * Class:			DPVS::Set
 *
 * Description:		Set class template
 *
 * Notes:			The Set class represents a 'set' of data elements.
 *					Standard set operation include inserting and removing
 *					elements from a set, determining whether an element
 *					is part of a set and a variety of operations between
 *					sets (comparison, copying, subtraction, union).
 *
 *					Sets never contain duplicate components. For example
 *					if the sets A = {1,3,5} and B = {1,4,5} were unioned,
 *					the resulting set would be {1,3,4,5}.
 *
 *					When a set is converted into a linearly accessible
 *					array (either through the Set::Array class constructor
 *					or Set::getArray()), the order of elements is not
 *					constant. Thus the set A = {1,3,5} could return an
 *					array {1,3,5} or {5,1,3} or any other combination. If
 *					ordering of arrays is required, the easiest way is to
 *					apply any of SurRender's sorting functions to the
 *					resulting set. However, for purposes of comparing
 *					sets two sets are considered equal if they contain
 *					the same elements (i.e. {1,3,5} and {5,3,1} are equal).
 *
 *					The Set class is implemented with a powerful hash
 *					table mechanism for optimal query times. On average
 *					all single element operations are O(1) (i.e. insertion,
 *					removal and query) and all set-to-set operations
 *					are O(N). The hash function can be overloaded.
 *
 *					Sets can be used for all data types which have an
 *					equality operator (i.e. operator==) defined.
 *
 ******************************************************************************/
/******************************************************************************
 *
 * Class:			DPVS::Set::Array
 *
 * Description:		Auxiliary class used to query a linear array of set elements
 *
 ******************************************************************************/

template <class S> class Set
{
public:
	class Array
	{
	private:
		S*				arr;				// array
		int				size;				// length of array in terms of elements
	public:
		explicit		Array		(int sz) : arr(0),size(sz)				{ arr = NEW_ARRAY<S>(size); }
						Array		(const Array& src) : arr(0),size(0)		{ *this = src; }
		explicit		Array		(const Set<S>& set) : arr(0),size(0)	{ size = set.getSize(); if (size > 0) arr  = NEW_ARRAY<S>(size); int cnt = 0; for (unsigned int i = 0; i < set.size; i++) for (Handle h = set.hash[i]; h != NIL; h = set.table[h].next) arr[cnt++] = set.table[h].s; DPVS_ASSERT(cnt == size); }
						~Array		(void)									{ DELETE_ARRAY(arr); arr = null; }
		int				getSize		(void) const							{ return size; }
		const S&		operator[]	(int i) const							{ DPVS_ASSERT (arr && i>=0 && i < size); return (arr[i]);	}
		S&				operator[]	(int i)									{ DPVS_ASSERT (arr && i>=0 && i < size); return (arr[i]);	}
		Array&			operator=	(const Array& src)						
		{
			if (&src != this)
			{
				DELETE_ARRAY(arr);
				arr		= null;
				size	= src.size; 
				if (size > 0) 
				{
					arr = NEW_ARRAY<S>(size);
					for (int i = 0; i < size; i++) 
						arr[i] = src.arr[i];
				}
			}
			return *this;
		}
	};
		
	inline				Set					(void);
	inline				Set					(const Set<S>& src);
	inline				Set					(const S* src, int cnt);
	inline				~Set				(void);
	inline bool			operator==			(const Set<S>& src) const;
	inline bool			operator!=			(const Set<S>& src) const		{ return (!(*this == src));}
	inline Set<S>&		operator+=			(const S& s)					{ insert(s); return *this; }
	inline Set<S>&		operator-=			(const S& s)					{ remove(s); return *this; }
	inline Set<S>&		operator=			(const Set<S>& src);
	inline Set<S>&		operator&=			(const Set<S>& src);
	inline Set<S>&		operator|=			(const Set<S>& src);
	inline Set<S>&		operator-=			(const Set<S>& src);
	inline void			checkConsistency	(void);
	inline bool			contains			(const S& s) const;
	inline bool			contains			(const Set<S>& src) const; 
	inline void			getArray			(S* arr);
	inline int			getMemoryUsage		(void) const					{ return sizeof(Set<S>) + sizeof(Handle)*size + sizeof(Entry)*size; }
	inline int			getSize				(void) const					{ return elemCount; }
	inline void			insert				(const S& s);
	inline bool			isEmpty				(void) const					{ return (elemCount == 0); }
	inline void			remove				(const S& s);
	inline void			removeAll			(bool minimizeMemoryUsage = true);
private:
	friend class	Array;
	
	enum
	{
		NIL		    = -1,					// value used for terminating linked lists
		MIN_SIZE	= 8						// smallest size the hash can shrink
	};

	typedef int Handle;						// signed int

#if defined (DPVS_BUILD_MSC) || defined(DPVS_BUILD_CW)
	template <class S> struct EEntry
	{
		Handle		next;					// next pointer in linked list
		S			s;						// src val
	};

	typedef EEntry<S> Entry;
#else
	struct Entry;
	friend struct Entry;

	struct Entry
	{
		Handle		next;					// next pointer in linked list
		S			s;						// src val
	};
#endif

	unsigned int	size;						// size of hash table
	Handle*			hash;						// hash pointers
	Entry*			table;						// allocation table
	Handle			first;						// handle of first free entry
	int				elemCount;

	// extension to provide fast stack-based serving for
	// small sets


	Handle			sHash[MIN_SIZE];
	Entry			sTable[MIN_SIZE];

	DPVS_FORCE_INLINE unsigned int	getHashVal		(const S& s, const unsigned int hashArraySize) const;
	void			rehash		    (size_t newSize);
	void			cleanup			(void);
};

//------------------------------------------------------------------------
// Implementation
//------------------------------------------------------------------------

template <class S> DPVS_FORCE_INLINE unsigned int Set<S>::getHashVal	(const S& s, const unsigned int hashArraySize) const
{
	return HashKey<S>::getHashValue (s) & (hashArraySize-1);
}

template <class S> inline void	Set<S>::getArray (S* arr)
{
	int cnt = 0;
	for (unsigned int i = 0; i < size; i++)
	for (Handle h = hash[i]; h != NIL; h = table[h].next)
		arr[cnt++] = table[h].s;
	DPVS_ASSERT(cnt == elemCount);
}

template <class S> inline void Set<S>::checkConsistency (void)
{
#if defined (DPVS_DEBUG)
	unsigned int cnt = 0;
	if (hash)
	{
		if (hash == sHash)
		{
			DPVS_ASSERT (size == MIN_SIZE);
			DPVS_ASSERT (table == sTable);
		}

		for (unsigned int i = 0; i < size; i++)
		{
			DPVS_ASSERT (hash[i] == NIL || (hash[i] >= 0 && hash[i] < (int)size));
			for (Handle h = hash[i]; h != NIL; h = table[h].next)
			{
				DPVS_ASSERT (table[h].next == NIL || (table[h].next >= 0 && table[h].next < (int)size));
				cnt++;
			}
		}
	}
	
	DPVS_ASSERT ((int)cnt == elemCount);

	if (size)
		DPVS_ASSERT (size == getNextPowerOfTwo(size));
#endif
}

template <class S> inline void Set<S>::rehash	(size_t newSize)
{
#ifdef DPVS_DEBUG
	checkConsistency();
#endif
	if (newSize < MIN_SIZE)
		newSize = MIN_SIZE;

	DPVS_ASSERT ((int)newSize >= elemCount);

	Entry*	newTable;
	Handle*		newHash;

	// If performing minimum size alloc then use
	// the arrays allocated originally along with
	// the set itself.

	if (newSize == MIN_SIZE)
	{
		DPVS_ASSERT (hash != sHash);
		DPVS_ASSERT (table != sTable);
		newTable = sTable;
		newHash  = sHash;
	} else
	{
		newTable = NEW_ARRAY<Entry>(newSize);
		newHash  = NEW_ARRAY<Handle>(newSize);
	}

	unsigned int i, cnt = 0;

	for (i = 0; i < newSize; i++)
		newHash[i] = NIL;

	for (i = 0; i < size; i++)						// step through each old hash set
	for (Handle h = hash[i]; h != NIL; h = table[h].next)
	{
		unsigned int hVal		= getHashVal(table[h].s, newSize);
		newTable[cnt].s		= table[h].s;
		newTable[cnt].next	= newHash[hVal];
		newHash[hVal]		= (int)cnt;
		cnt++;
	}
	DPVS_ASSERT ((int)cnt == elemCount);

	for (i = cnt; i < newSize; i++)
		newTable[i].next = (int)(i+1);
	newTable[newSize-1].next = NIL;

	if (hash != sHash)
		DELETE_ARRAY(hash);
	if (table != sTable)
		DELETE_ARRAY(table);

	first	= (int)cnt;
	hash	= newHash;
	table	= newTable;
	size	= newSize;
}

template <class S> inline void Set<S>::remove (const S& s)
{
	Handle			prev = NIL;
	unsigned int	hval = getHashVal(s,size);
	Handle			h	 = hash[hval];

	while (h != NIL)
	{
		if (table[h].s == s)
		{
			if (prev!=NIL)
				table[prev].next = table[h].next;
			else
				hash[hval]  = table[h].next;
			table[h].next	= first;
			first			= h;
			elemCount--;

			if (size > MIN_SIZE && (elemCount*3)<(int)size)
				rehash(size/2);
			return;
		}
		prev = h;
		h    = table[h].next;
	}
}

template <class S> inline Set<S>::Set (void) : size(MIN_SIZE),hash(sHash),table(sTable),first(0),elemCount(0)
{
	for (unsigned int i = 0; i < size; i++)
	{
		hash[i] = NIL;
		table[i].next = (int)(i+1);
	}
	table[size-1].next = NIL;
}

template <class S> inline Set<S>::Set (const S* src, int cnt) : size(0),hash(null),table(null),first(NIL),elemCount(0)
{
	rehash ((size_t)getNextPowerOfTwo((unsigned int)cnt));

	if (src)
	for (int i = 0; i < cnt; i++)
		insert(src[i]);
}

template <class S> inline Set<S>::Set (const Set<S>& src) : size(0),hash(null),table(null),first(NIL),elemCount(0)
{
	*this = src;
}

template <class S> inline Set<S>::~Set (void)
{
	// check consistency (in debug build only)
	#ifdef DPVS_DEBUG
		checkConsistency();
	#endif

	// Delete hash/table only if they're not pointed to sHash and sTable.
	// Because the sHash/sTable selection is synched we only need to test
	// for one case.
	
	if (hash != sHash && hash != null)
	{
		DPVS_ASSERT (table != sTable && table != null);
		DELETE_ARRAY(hash);
		DELETE_ARRAY(table);
	}
}

template <class S> inline Set<S>& Set<S>::operator= (const Set<S>& src)
{
	DPVS_ASSERT (&src);

	if (&src != this)
	{
		// Performs a direct copy since we use indices
		// rather than pointers in the linked lists...
		// Quite a bit faster than inserting stuff
		// to the hash table.
		
		cleanup();

		size		= src.size;
		elemCount	= src.elemCount;
		first		= src.first;

		if (size > MIN_SIZE)
		{
			hash	= NEW_ARRAY<Handle>(size);
			table	= NEW_ARRAY<Entry>(size);
		} else
		{
			DPVS_ASSERT (size == MIN_SIZE);
			hash	= sHash;
			table	= sTable;
		}
		
		memcpy (hash,	src.hash,	size*sizeof(Handle));
		memcpy (table,src.table,	size*sizeof(Entry));
	}
	
	return *this;
}

template <class S> inline bool Set<S>::operator== (const Set<S>& src) const
{
	if (elemCount != src.elemCount)
		return false;

	for (unsigned int i = 0; i < src.size; i++)
	for (Handle  h = src.hash[i]; h != NIL; h = src.table[h].next)
		if (!contains(src.table[h].s))
			return false;
	return true;
}

template <class S> inline Set<S>& Set<S>::operator&= (const Set<S>& src)
{
	// this could probably be optimized

	typename Set<S>::Array a(*this);

	int i,cnt = a.getSize();
	for (i = 0; i < cnt; i++)
		if (!src.contains(a[i]))
			remove(a[i]);
	return *this;
}

template <class S> inline Set<S>& Set<S>::operator|= (const Set<S>& src)
{
	for (unsigned int i = 0; i < src.size; i++)
	for (Handle h = src.hash[i]; h != NIL; h = src.table[h].next)
		insert(src.table[h].s);
	return *this;
}

template <class S> inline Set<S>& Set<S>::operator-= (const Set<S>& src)
{
	if (isEmpty() || src.isEmpty())
		return *this;

	for (unsigned int i = 0; i < src.size; i++)
	for (Handle h = src.hash[i]; h != NIL; h = src.table[h].next)
		remove(src.table[h].s);
	return *this;
}

template <class S> inline bool Set<S>::contains(const Set<S>& src) const
{
	if (src.elemCount > elemCount)			// source is larger than this
		return false;

	for (unsigned int i = 0; i < src.size; i++)
	for (Handle h = src.hash[i]; h != NIL; h = src.table[h].next)
		if (!contains(src.table[h].s))
			return false;
	return true;
}

template <class S> inline bool Set<S>::contains (const S& s) const
{
	Handle  h = hash[getHashVal(s,size)];
	while (h!=NIL)
	{
		if (table[h].s == s)
			return true;
		h = table[h].next;
	};
	return false;
}

template <class S> inline void Set<S>::insert (const S& s)
{
	if (first == NIL)					// need to re-alloc and re-hash tables
		rehash((size_t)(size*2));						

	Handle	h;
	unsigned int	hval    = getHashVal(s,size);

	DPVS_ASSERT (hash);
	for (h = hash[hval]; h!=NIL; h = table[h].next)
	if (table[h].s == s)							// already there
		return;
	h				= first;
	first			= table[first].next;
	table[h].s		= s;
	table[h].next	= hash[hval];
	hash[hval]		= h;
	elemCount++;
}

template <class S> inline void Set<S>::removeAll (bool minimizeMemoryUsage)
{
	if (minimizeMemoryUsage)				// free all allocs
	{
		cleanup	();
		rehash	(MIN_SIZE);
	} else
	{
		unsigned int i;
		
		for (i = 0; i < size; i++)			// clear hash table
		{
			hash[i]			= NIL;
			table[i].next	= i+1;
		}

		if (size)
			table[size-1].next = NIL;
		
		first		= 0;
		elemCount	= 0;
	}

#ifdef DPVS_DEBUG
	checkConsistency();
#endif
}

template <class S> inline void Set<S>::cleanup (void)
{
	// check consistency (in debug build only)
#ifdef DPVS_DEBUG
	checkConsistency();
#endif

	// Delete hash/table only if they're not pointed to sHash and sTable.
	// Deleting null pointers is OK in C++.

	if (hash != sHash)
		DELETE_ARRAY(hash);
	if (table != sTable)
		DELETE_ARRAY(table);
	hash		= null;
	table		= null;
	size		= 0;
	elemCount	= 0;
	first		= NIL;
}

} // DPVS
//------------------------------------------------------------------------
#endif // __DPVSSET_HPP
