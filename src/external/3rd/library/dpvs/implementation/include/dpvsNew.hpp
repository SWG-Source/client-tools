#ifndef __DPVSNEW_HPP
#define __DPVSNEW_HPP
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
 * Description: 	NEW and DELETE macros
 *
 * Notes:			This file is always included directly by "dpvsPrivateDefs.hpp"
 *
 * $Archive: /dpvs/implementation/include/dpvsNew.hpp $
 * $Author: wili $ 
 * $Revision: #1 $
 * $Modtime: 8.10.02 13:19 $
 * $Date: 2003/01/22 $
 * 
 ******************************************************************************/

#if !defined (__DPVSPRIVATEDEFS_HPP)
#	error This file should be included directly by dpvsPrivateDefs.hpp!
#endif

//#include <new>

DPVS_FORCE_INLINE void* /*__cdecl*/ operator new(size_t, void *_P) { return (_P); }

namespace DPVS
{
//------------------------------------------------------------------------
// A tiny template meta-programming trick that allows querying whether
// a data type is built-in or not. The code is taken from Housemarque's
// UMP library - thanks to Vesa Karvonen...
//------------------------------------------------------------------------

#if defined (DPVS_BUILD_MSC)		// MSVC does not have partial evaluation...

	template<size_t n>
	struct Sizeof_Ret
	{
		char ret[n];

		DPVS_FORCE_INLINE operator size_t() const
		{
			return n;
		}
	};

	const size_t
	  sizeof_ret_true	= 1,
	  sizeof_ret_false	= 2;	//	WARNING! sizeof_ret_false != 0

	template<class T>
	struct IsPointer
	{
	private:
		static T t;
	
		template<class U>
		static Sizeof_Ret<sizeof_ret_true> //UMP_STD_CPP
			Helper(
				U*&);
		static Sizeof_Ret<sizeof_ret_false> //UMP_STD_CPP
			Helper(
				...);
	
	public:
		enum
		{
			ret = sizeof(Helper(t).ret) == sizeof_ret_true
		};
	};
	
	template<>
	struct IsPointer<void>
	{
		enum
		{
			ret = false
		};
	};

#else	// this is the classic Stroustrup way of doing it..

	template<class T>
	struct IsPointer
	{
		enum
		{
			ret = false
		};
	};
	
	template<class T>
	struct IsPointer<T*>
	{
		enum
		{
			ret = true
		};
	};

#endif

template <class T> class IsBuiltin
{
public:
	static DPVS_FORCE_INLINE bool eval (void)  { return IsPointer<T>::ret; }
};

#define DPVS_BUILTIN(X) template <> bool IsBuiltin<X>::eval (void) { return true; }
DPVS_BUILTIN(char)
DPVS_BUILTIN(signed char)
DPVS_BUILTIN(unsigned char)
DPVS_BUILTIN(INT16)
DPVS_BUILTIN(UINT16)
DPVS_BUILTIN(INT32)
DPVS_BUILTIN(UINT32)
DPVS_BUILTIN(float)
DPVS_BUILTIN(double)
DPVS_BUILTIN(FloatInt)
#if defined (DPVS_UINT64_DEFINED)							// 64-bit integer support
DPVS_BUILTIN(INT64)
DPVS_BUILTIN(UINT64)
#endif
#undef DPVS_BUILTIN

#undef MALLOC
#undef FREE
#undef DELETE
#undef DELETE_ARRAY
#undef NEW
#undef NEW_ARRAY

//------------------------------------------------------------------------
// Global memory management functions --> always use these instead of
// standard new/delete... The functions recognized built-in types and
// allocate them differently
//------------------------------------------------------------------------


void*										MALLOC				(size_t size);
void										FREE				(void* p);
template <class T> DPVS_FORCE_INLINE void	DELETE				(T* p) 
{ 
	if (p) 
	{ 
		if (!IsBuiltin<T>::eval())
			p->~T(); 
		FREE(p); 
	}
}
template <class T> DPVS_FORCE_INLINE void	DELETE_ARRAY		(T* p) 
{ 
	if (IsBuiltin<T>::eval())			// handle built-in types separately
		FREE(p);						// FREE handles NULL pointer checks internally (less code bloat this way)
	else if (p) 
	{ 
		int* q = reinterpret_cast<int*>(((reinterpret_cast<char*>(p))-16)); 
		int elems	= *q; 
		DPVS_ASSERT(elems >= 0);	
		for (int i = 0; i < elems; i++)	
			(p+i)->~T(); 
		FREE(q);
	}
}

#if defined (DPVS_BUILD_INTELC)
#	pragma warning (disable:488)		// don't whine about T not being used as a function parameter type						
#endif

template <class T> DPVS_FORCE_INLINE T*	NEW					(void) 			{ void* p = MALLOC(sizeof(T)); return new(p) T; }
template <class T> DPVS_FORCE_INLINE T*	NEW_ARRAY			(int elems)	
{ 
	if (IsBuiltin<T>::eval())				// handle built-in types separately
		return (T*)MALLOC(elems*sizeof(T));
	else
	{
		DPVS_ASSERT(elems >= 0); 
		char* p = reinterpret_cast<char*>(MALLOC(sizeof(T) * elems + 16)); 
		*(reinterpret_cast<int*>(p)) = elems; 
		p+=16; 
		for (int i = 0; i < elems; i++) 
			new(p+i*sizeof(T)) T; 
		return reinterpret_cast<T*>(p); 
	}
}

#if defined (DPVS_BUILD_INTELC)
#	pragma warning (default:488)							
#endif

} // namespace DPVS

//------------------------------------------------------------------------
#endif // __DPVSNEW_HPP
