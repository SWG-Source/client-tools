#ifndef __DPVSPTR_HPP
#define __DPVSPTR_HPP
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
 * Description: 	Ptr class interface
 *
 * $Archive: /dpvs/implementation/include/dpvsPtr.hpp $
 * $Author: wili $ 
 * $Revision: #1 $
 * $Modtime: 7.05.01 12:06 $
 * $Date: 2003/01/22 $
 * 
 ******************************************************************************/

#if !defined (__DPVSPRIVATEDEFS_HPP)
#	include "dpvsPrivateDefs.hpp"
#endif

#if !defined (__DPVSREFERENCECOUNT_HPP)
#	include "dpvsReferenceCount.hpp"
#endif

/******************************************************************************
 *
 * Class:			DPVS::Ptr
 *
 * Description:		A smart pointer class to be used with ReferenceCount class
 *
 * Notes:			The Ptr can only be used with classes derived from
 *					DPVS::ReferenceCount
 *
 *****************************************************************************/

namespace DPVS
{
template<class T>
class Ptr
{
private:
	T*			_ptr;									// pointer to object derived from ReferenceCount
public:
				Ptr			(const Ptr& that)			{ if (that._ptr) that._ptr->addReference(); _ptr = that._ptr; }
				Ptr			(void) : _ptr(0)			{ }
				Ptr			(T* r) : _ptr(r)			{ if (r) r->addReference(); }
				~Ptr		(void)						{ if (_ptr) _ptr->release (); }
	const Ptr&  operator=	(T* p)						{ if (p != _ptr){if (p) p->addReference(); if (_ptr) _ptr->release(); _ptr = p; } return *this; }
	const Ptr&	operator=	(const Ptr& src)			{ if (&src != this) { if (&src != 0) { if (src._ptr) (src._ptr)->addReference(); if (_ptr) _ptr->release (); _ptr = src._ptr; } else { if (_ptr) _ptr->release (); _ptr = 0; } } return *this;}
	bool		operator==	(const Ptr& src) const		{ return _ptr == src._ptr; }
	bool		operator!=	(const Ptr& src) const		{ return _ptr != src._ptr; }
	bool		operator==	(const T* src) const		{ return _ptr == src; }
	bool		operator!=	(const T* src) const		{ return _ptr != src; }
	T*			operator->	(void)						{ return _ptr; }
				operator T* (void) const				{ return _ptr; }
	T*			getPtr		(void) const				{ return _ptr; }
};
} // DPVS

//------------------------------------------------------------------------
#endif // __DPVSPTR_HPP
