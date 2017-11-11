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
 * Description:		Name Hash code
 *
 * $Archive: /dpvs/implementation/sources/dpvsNameHash.cpp $
 * $Author: wili $ 
 * $Revision: #1 $
 * $Modtime: 7.05.01 12:06 $
 * $Date: 2003/01/22 $
 * 
 ******************************************************************************/

#include "dpvsNameHash.hpp"
#include "dpvsHash.hpp"

#include <cstring>

using namespace DPVS;

NameHash::NameHash (void) 
{ 
	m_entryCount	= 0; 
	m_hash			= 0;
}

NameHash::~NameHash (void) 
{ 
	DPVS_ASSERT(m_entryCount==0);
	// if there's entries, they're going to leak!!!
	DELETE(m_hash);
}

void NameHash::remove (const void* e)
{
	if (m_hash && m_hash->exists(e))
	{
		char* foo = m_hash->get(e);
		FREE(foo);
		m_hash->remove(e);
		m_entryCount--;
		DPVS_ASSERT(m_entryCount >= 0);
		if (m_entryCount == 0)						// remove hash table as it's not needed anymore
		{
			DELETE(m_hash);
			m_hash = 0;
		}
	}
}

static DPVS_FORCE_INLINE int getStringLength (const char* s)
{
	int len = 0;
	while (s[len])
		len++;
	return len;
}

static DPVS_FORCE_INLINE void copyString (char* d, const char* s)
{
	while (*s)
	{
		*d++ = *s++;
	} 

	*d = (char)(0);
}

void NameHash::insert (const void* e, const char* s)
{
	remove(e);										

	if (s && *s)									// if non-null and non-empty string
	{
		if (!m_hash)
			m_hash = NEW< Hash<const void*,char*> >();	// create hash table

		char* foo = (char*)MALLOC(getStringLength(s)+1);
		copyString(foo,s);
		m_hash->insert(e,foo);
		m_entryCount++;
	}
}


const char* NameHash::get (const void* e) const
{
	return m_hash ? m_hash->get(e) : 0;
}

//------------------------------------------------------------------------
