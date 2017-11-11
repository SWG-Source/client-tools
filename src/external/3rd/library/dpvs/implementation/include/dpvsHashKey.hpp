#ifndef __DPVSHASHKEY_HPP
#define __DPVSHASHKEY_HPP
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
 * Desc:	Hashing functions used by the different hash classes
 *
 * $Archive: /dpvs/implementation/include/dpvsHashKey.hpp $
 * $Author: wili $ 
 * $Revision: #1 $
 * $Modtime: 14.06.01 12:54 $
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
 * Class:			DPVS::HashKey
 *
 * Description:		Class for providing hash values for different data types
 *
 * Notes:			Specialize the template for your data type in *this*
 *					file (don't do it inside a .cpp file)...
 *
 ******************************************************************************/

template <class Key> class HashKey
{
public:
	static DPVS_FORCE_INLINE unsigned int getHashValue (const Key& k);
};

// Default hash function for data types that can be cast into an unsigned int
template <class Key> DPVS_FORCE_INLINE unsigned int HashKey<Key>::getHashValue (const Key& k)
{ 
	UINT32 hval = *(reinterpret_cast<const UINT32*>(&k));
	hval = hval + (hval>>5) + (hval>>10) + (hval >> 20);
	return hval;
}

// Specialization for floating point hash values (as the default dword-
// casting yields a very bad hash function)
template <> unsigned int HashKey<float>::getHashValue (const float& s)
{
	UINT32 z = *(reinterpret_cast<const UINT32*>(&s));
	return ((z>>22)+(z>>12)+(z));
}

template <> unsigned int HashKey<Vector2i>::getHashValue (const Vector2i& s)
{
	UINT32 a = (reinterpret_cast<const UINT32*>(&s))[0];
	UINT32 b = (reinterpret_cast<const UINT32*>(&s))[1];
	return a + (a>>12) + b*173;
}

template <> unsigned int HashKey<Vector3>::getHashValue (const Vector3& s)
{
	const UINT32* h = (reinterpret_cast<const UINT32*>(&s));
	UINT32 foo = h[0]+h[1]*11-(h[2]*17);
	return (foo>>22)^(foo>>12)^(foo);
}

template <> unsigned int HashKey<Vector4>::getHashValue (const Vector4& s)
{
	const UINT32* h = (reinterpret_cast<const UINT32*>(&s));
	UINT32 foo = h[0]+h[1]-(h[2]*17)+h[3]*7;
	return (foo>>12)^(foo>>22)^(foo);
}

} // DPVS

//------------------------------------------------------------------------
#endif //__DPVSHASHKEY_HPP

