#ifndef __DPVSHASH_HPP
#define __DPVSHASH_HPP
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
 * Desc:	Basic hash template
 *
 * $Archive: /dpvs/implementation/include/dpvsHash.hpp $
 * $Author: wili $ 
 * $Revision: #1 $
 * $Modtime: 20.06.01 16:31 $
 * $Date: 2003/01/22 $
 * 
 *****************************************************************************/

#if !defined (__DPVSHASHKEY_HPP)
#	include "dpvsHashKey.hpp"
#endif

namespace DPVS
{
/******************************************************************************
 *
 * Class:			DPVS::Hash
 *
 * Description:		Minimal hash table template
 *
 * Notes:			This is old legacy code taken from SurRender - we may
 *					want to replace it at some point...
 *
 ******************************************************************************/

template <class Key, class Value> 
class Hash
{
public:
	enum 
	{
		NIL = -1								// internal enumeration for representing a null link
	};
					Hash				(void);
					~Hash				(void);
	void			insert				(const Key& s, const Value& d);
	void			remove				(const Key& s);
	void			remove				(const Key& s, const Value& d);
	Value			get					(const Key& s) const;
	bool			get					(const Key& s, Value& d) const;
	bool			exists				(const Key& s) const;
	bool			exists				(const Key& s, const Value& d) const;
	void			removeAll			(void);
	unsigned int	getSize				(void) const;
private:
						Hash			(const Hash&);	// not allowed
	Hash&				operator=		(const Hash&);	// not allowed
	static DPVS_FORCE_INLINE unsigned int	getHashVal		(const Key& s, const unsigned int hashArraySize);

	void		rehash				(void);

	struct Entry
	{
		INT32		next;					// next pointer in linked list
		Key			key;					// key
		Value		value;					// value
	};

	INT32*			hash;						// hash pointers
	Entry*			table;						// allocation table
	INT32			first;						// handle of first free entry
	unsigned int	size;						// size of hash table
};

//------------------------------------------------------------------------
// Implementation
//------------------------------------------------------------------------

template <class Key, class Value> DPVS_FORCE_INLINE unsigned int Hash<Key,Value>::getHashVal	(const Key& s, const unsigned int hashArraySize)
{
	return HashKey<Key>::getHashValue (s) & (hashArraySize-1);
}

template <class Key, class Value> inline void Hash<Key,Value>::insert	(const Key& s, const Value& d)
{
	if (first == NIL)					// need to re-alloc and re-hash tables
		rehash();						
	INT32 h	= first;
	first	= table[first].next;

	unsigned int hval	= getHashVal(s,size);
	table[h].key	= s;
	table[h].value	= d;
	table[h].next	= hash[hval];
	hash[hval]		= h;
}

template <class Key, class Value> inline unsigned int Hash<Key,Value>::getSize (void) const
{
	return size;
}

template <class Key, class Value> inline void Hash<Key,Value>::removeAll (void)
{
	for (unsigned int i = 0; i < size; i++)
	{
		INT32 f = hash[i];
		if (f!=NIL)
		{
			INT32 h = f;
			while (table[h].next != NIL)
				h = table[h].next;
			table[h].next = first;		// link to first free
			first = f;					// link to beginning
			hash[i] = NIL;				// kill entry in hash
		}
	}
}

template <class Key, class Value> inline void Hash<Key,Value>::remove	(const Key& s)
{
	unsigned int hval = getHashVal(s,size);
	INT32  prev = NIL;
	INT32	h	 = hash[hval];

	while (h != NIL)
	{
		if (table[h].key == s)
		{
			if (prev!=NIL)
				table[prev].next = table[h].next;
			else
				hash[hval] = table[h].next;
			table[h].next = first;
			first = h;
			return;
		}
		prev = h;
		h    = table[h].next;
	}
}

template <class Key, class Value> inline void Hash<Key,Value>::remove (const Key& s, const Value& d)
{
	unsigned int hval = getHashVal(s,size);
	INT32  prev = NIL;
	INT32	h	 = hash[hval];

	while (h != NIL)
	{
		if (table[h].key == s && table[h].value == d)
		{
			if (prev!=NIL)
				table[prev].next = table[h].next;
			else
				hash[hval] = table[h].next;
			table[h].next = first;
			first = h;
			return;
		}
		prev = h;
		h    = table[h].next;
	}
}

template <class Key, class Value> inline Value Hash<Key,Value>::get (const Key& s) const
{
	INT32  h = hash[getHashVal(s,size)];
	while (h!=NIL)
	{
		if (table[h].key == s)
			return table[h].value;
		h = table[h].next;
	}
	return (Value)(0);
}

template <class Key, class Value> inline bool Hash<Key,Value>::get (const Key& s, Value& d) const
{
	INT32  h = hash[getHashVal(s,size)];
	while (h!=NIL)
	{
		if (table[h].key == s)
		{
			d = table[h].value;
			return true;
		}
		h = table[h].next;
	}
	return false;
}

template <class Key, class Value> inline bool Hash<Key,Value>::exists (const Key& s) const
{
	INT32  h = hash[getHashVal(s,size)];
	while (h!=NIL)
	{
		if (table[h].key == s)
			return true;
		h = table[h].next;
	}
	return false;
}

template <class Key, class Value> inline bool Hash<Key,Value>::exists (const Key& s, const Value& d) const
{
	INT32  h = hash[getHashVal(s,size)];
	while (h!=NIL)
	{
		if (table[h].key == s && table[h].value == d)
			return true;
		h = table[h].next;
	}
	return false;
}

template <class Key, class Value> inline void Hash<Key,Value>::rehash	(void)
{
	size_t newSize = size*2;
	if (newSize < 4)
		newSize = 4;

	Entry  *newTable = NEW_ARRAY<Entry>(newSize);
	INT32 *newHash   = NEW_ARRAY<INT32>(newSize);

	int cnt = 0;
	int	i;

	for (i = 0; i < (int)newSize; i++)
	{
		newTable[i].next	= NIL;
		newHash[i]			= NIL;
	}

	if (size)											// if we have existing data, it needs to be rehashed
	{
		for (i = 0; i < (int)size; i++)						// step through each old hash set
		{
			INT32	h = hash[i];
			while (h != NIL)
			{
				unsigned int hVal		= getHashVal(table[h].key, newSize);
				newTable[cnt].key	= table[h].key;
				newTable[cnt].value = table[h].value;
				newTable[cnt].next	= newHash[hVal];
				newHash[hVal]		= cnt;
				cnt++;
				h = table[h].next;
			}
		}
		DELETE_ARRAY(hash);
		DELETE_ARRAY(table);
	}

	for (i = cnt; i < (int)newSize; i++)
		newTable[i].next = i+1;
	newTable[newSize-1].next = NIL;

	first	= cnt;
	hash	= newHash;
	table	= newTable;
	size	= newSize;
}

template <class Key, class Value> inline Hash<Key,Value>::Hash () : hash(0),table(0),first(NIL),size(0)
{
	rehash ();
}

template <class Key, class Value> inline Hash<Key,Value>::~Hash ()
{
	DELETE_ARRAY(hash);
	DELETE_ARRAY(table);
}

} // DPVS

//------------------------------------------------------------------------
#endif //__DPVSHASH_HPP
