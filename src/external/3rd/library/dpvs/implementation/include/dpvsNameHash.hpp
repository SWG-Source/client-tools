#ifndef __DPVSNAMEHASH_HPP
#define __DPVSNAMEHASH_HPP
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
 * Desc:	Name hash (for objects)
 *
 * $Archive: /dpvs/implementation/include/dpvsNameHash.hpp $
 * $Author: wili $ 
 * $Revision: #1 $
 * $Modtime: 7.05.01 12:06 $
 * $Date: 2003/01/22 $
 * 
 *****************************************************************************/

#if !defined (__DPVSPRIVATEDEFS_HPP)
#	include "dpvsPrivateDefs.hpp"
#endif

namespace DPVS
{

template <class Key, class Value> class Hash;	// forward declaration

/******************************************************************************
 *
 * Class:			DPVS::NameHash
 *
 * Description:		Class providing storage and hashing of names for objects.
 *					The basic idea is that names are usually a debugging facility
 *					and storage for the name pointers are not allocated if not
 *					necessary.
 *
 ******************************************************************************/

class NameHash
{
public:
				NameHash	(void);
				~NameHash	(void);

	void		insert		(const void* e, const char* s);	// multiple insertions allowed (replaces value) . null insertion == remove
	void		remove		(const void* e);				
	const char* get			(const void* e) const;

private:
				NameHash	(const NameHash&);
	NameHash&	operator=	(const NameHash&);

	Hash<const void*,char*>*	m_hash;				// hash table
	int							m_entryCount;		// number of entries
};

} 

//------------------------------------------------------------------------
#endif // __DPVSNAMEHASH_HPP
