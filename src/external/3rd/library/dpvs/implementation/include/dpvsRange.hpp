#ifndef __DPVSRANGE_HPP
#define __DPVSRANGE_HPP
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
 * Description: 	Range class
 *
 * $Archive: /dpvs/implementation/include/dpvsRange.hpp $
 * $Author: wili $ 
 * $Revision: #1 $
 * $Modtime: 20.06.01 20:20 $
 * $Date: 2003/01/22 $
 * 
 ******************************************************************************/

#if !defined (__DPVSPRIVATEDEFS_HPP)
#	include "dpvsPrivateDefs.hpp"
#endif

namespace DPVS
{
	template<class T> class Range
	{
	public:
		Range<T>					(T _mn=T(0),T _mx=T(0)) : mn(_mn), mx(_mx)	{}
		void			set			(T _mn,T _mx)				{ mn=_mn; mx=_mx; }

		T				width		(void) const				{ return mx-mn; }
		T				getMin		(void) const				{ return mn; }
		T				getMax		(void) const				{ return mx; }
		void			setMin		(T _mn)						{ mn = _mn; }
		void			setMax		(T _mx)						{ mx = _mx; }

		void			clampToUnit	(void)						{ mn = Math::max(mn, 0.0f); mx = Math::min(mx, 1.0f); }

		void			downSample	(T x)						{ mn/=x; mx=(mx+x-1)/x; }
		void			upSample	(T x)						{ mn*=x; mx*=x; }
		void			grow		(T x)						{ if(x<mn) mn=x; if(x>mx) mx=x; }

		void			pad			(T p)						{ if(mx%p) mx+=p-mx%p; }

		bool			intersect	(const Range<T> &r)
		{
			mn = Math::min(mn, r.mn);
			mx = Math::max(mx, r.mx);
//			if(mn<r.mn)	mn=r.mn;
//			if(mx>r.mx)	mx=r.mx;
			if(mx<=mn)			//degenerate
				return false;
			else
				return true;
		}

	private:
		T				mn,mx;
	};

} // DPVS

//------------------------------------------------------------------------
#endif // __DPVSRANGE_HPP
