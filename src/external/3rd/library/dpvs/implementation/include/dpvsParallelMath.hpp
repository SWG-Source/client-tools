#ifndef __DPVSPARALLELMATH_HPP
#define __DPVSPARALLELMATH_HPP
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
 * Desc:	Filler code for "dpvsOcclusionBuffer_render.cpp"
 *			Included only in a single place. DO NOT MOVE the functions
 *			to .cpp file.
 *
 * $Archive: /dpvs/implementation/include/dpvsParallelMath.hpp $
 * $Author: wili $ 
 * $Revision: #1 $
 * $Modtime: 20.06.01 16:31 $
 * $Date: 2003/01/22 $
 * 
 *****************************************************************************/

#if !defined (__DPVSOCCLUSIONBUFFER_HPP)
#	include "dpvsOcclusionBuffer.hpp"
#endif

/******************************************************************************
 *
 * Class:			DPVS::OcclusionBuffer::ParallelMath
 *
 * Description:		64-bit filler routines used by the occlusion buffer
 *
 ******************************************************************************/

class DPVS::OcclusionBuffer::ParallelMath
{
public:
	enum
	{
		PLANESIZE		= 2
	};

/*	static DPVS_FORCE_INLINE void	fill	(QWORD *s,UINT32 pattern)
	{
		s[0].setBoth(pattern);
		s[1].setBoth(pattern);
		s[2].setBoth(pattern);
	}
*/
	static DPVS_FORCE_INLINE void	fillZero	(QWORD* s)
	{
		s[0].setZero();
		s[1].setZero();
		s[2].setZero();
	}

	static DPVS_FORCE_INLINE void	any		(QWORD& dst, const QWORD* src)
	{
		dst = (src[0] | src[1] | src[2]);
	}

	static DPVS_FORCE_INLINE void eor1	(QWORD* d, UINT32 bitnum)
	{
		DPVS_ASSERT(UINT32(bitnum)<64);

		UINT32* dst = d[0].getDWORDPtr(bitnum>>5);		//returns ptr to either one of the DWORDS
		*dst ^= 1<<(bitnum&31);
	}

	//increment 1 line x, position bitnum
	static DPVS_FORCE_INLINE void inc	(QWORD* d, UINT32 bitnum)
	{
		DPVS_ASSERT(UINT32(bitnum)<64);

		UINT32* dst = d[0].getDWORDPtr(bitnum>>5);	//returns ptr to either one of the DWORDS
		UINT32	src = 1<<(bitnum&31);

		UINT32 carry0	 = dst[2*0] & src;
		dst[2*0]		^= src;					// Bit 0

		if(!carry0)
			return;								// NOTE: This is a rather common case

		UINT32 carry1	 = dst[2*1] & carry0;
		dst[2*1]		^= carry0;				// Bit 1
		dst[2*2]		^= carry1;				// Bit 2
	}

	//decrement 1 line x, position bitnum
	static DPVS_FORCE_INLINE void dec	(QWORD* d, UINT32 bitnum)
	{
		DPVS_ASSERT(UINT32(bitnum)<64);

		UINT32* dst = d[0].getDWORDPtr(bitnum>>5);	//returns ptr to either one of the DWORDS
		UINT32	src = 1<<(bitnum&31);

		UINT32 carry0	 = ~(dst[2*0]) & src;
		UINT32 carry1	 = ~(dst[2*1]) & carry0;

		dst[2*0]		^= src;					// Bit 0
		dst[2*1]		^= carry0;				// Bit 1
		dst[2*2]		^= carry1;				// Bit 2
	}


	static DPVS_FORCE_INLINE void add3	(QWORD* dst, const QWORD* src)
	{
		QWORD carry0(dst[0] & src[0]);
		QWORD carry1((dst[1]&(src[1]|carry0)) | (src[1]&carry0));
		carry0 ^= src[1];
		carry1 ^= src[2];
		dst[0] ^= src[0];
		dst[1] ^= carry0;
		dst[2] ^= carry1;
	}

/*
	static DPVS_FORCE_INLINE void sub3(QWORD* dst, const QWORD* src)
	{
		QWORD carry0 = (src[0] & ~dst[0]);		//src has, but dst doesn't
		QWORD carry1 = ((src[1] | carry0) & ~dst[1]) | (src[1] & carry0 & dst[1]);

		dst[0] ^= src[0];
		dst[1] ^= src[1]^carry0;
		dst[2] ^= src[2]^carry1;
	}

*/
	//self test
	static void	test()
	{
		QWORD	a[3],b[3];

		//test inc
		for(int i=0;i<8;i++)
		{
			ParallelMath::fillZero(a);
			ParallelMath::set(a,0,i);
			ParallelMath::inc(a,0);
			DPVS_ASSERT(ParallelMath::get(a,0) == ((i+1)&7));
		}

		//test dec
		for(int i=0;i<8;i++)
		{
			ParallelMath::fillZero(a);
			ParallelMath::set(a,0,i);
			ParallelMath::dec(a,0);
			DPVS_ASSERT(ParallelMath::get(a,0) == ((i-1)&7));
		}

/*		//test sub3
		for(int i=0;i<8;i++)
		for(int j=0;j<8;j++)
		{
			ParallelMath::fillZero(a);
			ParallelMath::fillZero(b);
			ParallelMath::set(a,0,i);
			ParallelMath::set(b,0,j);
			ParallelMath::sub3(a,b);
			DPVS_ASSERT(ParallelMath::get(a,0) == ((i-j)&7));
		}
*/
		//test add3
		for(int i=0;i<8;i++)
		for(int j=0;j<8;j++)
		{
			ParallelMath::fillZero(a);
			ParallelMath::fillZero(b);
			ParallelMath::set(a,0,i);
			ParallelMath::set(b,0,j);
			ParallelMath::add3(a,b);
			DPVS_ASSERT(ParallelMath::get(a,0) == ((i+j)&7));
		}
	}

private:
	enum
	{
		PLANE1_ACTIVE	= (1<<0),
		PLANE2_ACTIVE	= (1<<1),
		PLANE3_ACTIVE	= (1<<2)
	};

	static DPVS_FORCE_INLINE void	set		(QWORD *d,int bitnum,int value)
	{
		if(!value)
			return;

		DPVS_ASSERT(UINT32(value)<8);
		DPVS_ASSERT(UINT32(bitnum)<64);

		UINT32 *dst = d[0].getDWORDPtr(bitnum>>5);			//returns ptr to either one of the DWORDS
		UINT32	src  = 1<<(bitnum&31);

		if(value & PLANE1_ACTIVE)	*dst |= src;
		dst += 2*1;
		if(value & PLANE2_ACTIVE)	*dst |= src;
		dst += 2*1;
		if(value & PLANE3_ACTIVE)	*dst |= src;
	}

	static DPVS_FORCE_INLINE int	get		(const QWORD *s,int bitnum)
	{
		DPVS_ASSERT(UINT32(bitnum)<64);

		const UINT32 *dst = s[0].getDWORDPtr(bitnum>>5);	//returns ptr to either one of the DWORDS
		UINT32	src  = 1<<(bitnum&31);

		int	value = 0;
		if(*dst & src)	value |= PLANE1_ACTIVE;
		dst += 2*1;
		if(*dst & src)	value |= PLANE2_ACTIVE;
		dst += 2*1;
		if(*dst & src)	value |= PLANE3_ACTIVE;
		return value;
	}
};

#endif //__DPVSPARALLELMATH_HPP
//------------------------------------------------------------------------
