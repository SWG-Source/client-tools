#ifndef __DPVSFILLER_HPP
#define __DPVSFILLER_HPP
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
 *			into a .cpp file.
 *
 * $Archive: /dpvs/implementation/include/dpvsFiller.hpp $
 * $Author: wili $ 
 * $Revision: #1 $
 * $Modtime: 4/22/02 6:33p $
 * $Date: 2003/01/22 $
 * 
 *****************************************************************************/

#if !defined (__DPVSOCCLUSIONBUFFER_HPP)
#	include "dpvsOcclusionBuffer.hpp"
#endif
#if !defined (__DPVSSCRATCHPAD_HPP)
#	include "dpvsScratchpad.hpp"
#endif
#if !defined (__DPVSPARALLELMATH_HPP)
#	include "dpvsParallelMath.hpp"
#endif

/******************************************************************************
 *
 * Class:			DPVS::OcclusionBuffer::Cache
 *
 * Description:		
 *
 * Notes:			
 *
 ******************************************************************************/

#if defined (DPVS_X86_ASSEMBLY)
#	pragma warning (disable:4035) // don't whine about no return value (it's returned in eax)
#endif

namespace DPVS
{
class DPVS::OcclusionBuffer::Cache
{
private:
				Cache		(const Cache&);	// not allowed
	Cache&		operator=	(const Cache&);	// not allowed
	
	DPVS_FORCE_INLINE QWORD*	getData			(void) const		{ return (QWORD*)Scratchpad::getPtr(Scratchpad::OCCLUSIONBUFFER_CACHE);}

	enum
	{
		WIDTH			= DPVS_TILE_WIDTH,				// must agree with occlusion buffer tile (BW 32)
		PLANECOUNT		= 3,
		PLANESIZE		= (WIDTH*2),
		PLANE1_ACTIVE	= (1<<0),
		PLANE2_ACTIVE	= (1<<1),
		PLANE3_ACTIVE	= (1<<2)
	};

	enum COVERED
	{
		NOT_COVERED   = 0,
		MAYBE_COVERED = 1,
		IS_COVERED    = 2
	};

	bool			m_full;				// is it full?
	COVERED			m_fullCoverage;		// fully covered?
	int				m_mn;				// inclusive min
	int				m_mx;				// inclusive max
public:

			 Cache			(void);
			~Cache			(void);

	DPVS_FORCE_INLINE QWORD*			operator[]		(int i)			{ DPVS_ASSERT(UINT32(i)<WIDTH); return getData()+PLANECOUNT*i; }
	DPVS_FORCE_INLINE const QWORD*		operator[]		(int i)	const	{ DPVS_ASSERT(UINT32(i)<WIDTH); return getData()+PLANECOUNT*i; }
	static DPVS_FORCE_INLINE int		getPlaneCount	(void)			{ return PLANECOUNT; }

	DPVS_FORCE_INLINE void clear	(int lo,int hi)
	{
		DPVS_ASSERT((UINT32)(lo)<WIDTH);
		DPVS_ASSERT((UINT32)(hi)<=WIDTH);

		fillDWord((UINT32*)getData()+PLANECOUNT*lo, 0, PLANECOUNT*(hi-lo)*2);	//QWORD is 2 DWORDs
	}

	DPVS_FORCE_INLINE void	allocate		(void)
	{
		DPVS_CT_ASSERT(sizeof(QWORD)*PLANECOUNT*WIDTH <= 768);
		clear(0,WIDTH);
	}

	DPVS_FORCE_INLINE void	free			(void)
	{
		//nada
	}

	DPVS_FORCE_INLINE void	setRange		(int mn,int mx)	//inclusive,inclusive
	{
		m_mn = mn;
		m_mx = mx;
	}

	static DPVS_FORCE_INLINE UINT32 byteReorder		(const unsigned char* s)
	{
		#if defined (DPVS_X86_ASSEMBLY)
		#if defined (DPVS_X86_RETURN_EAX)
			__asm
			{
				mov edx,s
				movzx eax,byte ptr [edx+1*24]
				shl eax,8
				movzx ebx,byte ptr [edx+2*24]
				shl ebx,16
				movzx ecx,byte ptr [edx+3*24]
				shl ecx,24
				or eax,ebx
				movzx ebx,byte ptr [edx+0*24]
				or eax,ecx
				or eax,ebx
			}
		#else
			int retVal;
			__asm
			{
				mov edx,s
				movzx eax,byte ptr [edx+1*24]
				shl eax,8
				movzx ebx,byte ptr [edx+2*24]
				shl ebx,16
				movzx ecx,byte ptr [edx+3*24]
				shl ecx,24
				or eax,ebx
				movzx ebx,byte ptr [edx+0*24]
				or eax,ecx
				or eax,ebx
				mov retVal,eax
			}
			return retVal;
		#endif
		#else
			return (UINT32)(s[0*24]) | ((UINT32)(s[1*24])<<8) | ((UINT32)(s[2*24])<<16) | ((UINT32)(s[3*24])<<24);
		#endif 
	}


	DPVS_FORCE_INLINE QWORD getReordered	(int x,int y) const
	{
		DPVS_ASSERT((UINT32)(x)<WIDTH/8);
		DPVS_ASSERT((UINT32)(y)<8);

		if(m_full)
			return QWORD(0xFFFFFFFF,0xFFFFFFFF);

		const unsigned char* ptr = getData()[x*PLANECOUNT*8].getBytePtr(y);
		return QWORD(byteReorder(ptr+4*24), byteReorder(ptr));
	}

	DPVS_FORCE_INLINE void					setFull			()					{ m_full=true; m_fullCoverage=IS_COVERED; }
	DPVS_FORCE_INLINE bool					full			(void) const		{ return m_full; }
	DPVS_FORCE_INLINE bool					fullCoverage	(void) const
	{
		switch(m_fullCoverage)
		{
		case NOT_COVERED:	return false;
		case IS_COVERED:	return true;
		case MAYBE_COVERED:
			{
				QWORD*	ptr = getData() + PLANECOUNT*m_mn;
				for(int i=m_mn;i<m_mx;i++)
				{
					if(!ptr->full())
						return false;
					ptr += PLANECOUNT;
				}
			}
			return true;
		}

		DPVS_ASSERT(0);
		return false;
	}

	//DEBUG function
	void assertEmpty(void) const;

	DPVS_FORCE_INLINE void init	(void)
	{
		m_full = false;
		m_fullCoverage = NOT_COVERED;
	}

	void fillPlane0 (int lo,int hi,const QWORD pattern);

	DPVS_FORCE_INLINE void cleanup	(int bmin,int bmax)
	{
		int right = (bmax < (DPVS_TILE_WIDTH/8)) ? (bmax*8+1) : (bmax*8);
		fillPlane0(bmin*8,right,QWORD());

#ifdef DPVS_DEBUG
		assertEmpty();
#endif
	}


	DPVS_FORCE_INLINE void inc (int x,int y)
	{
		DPVS_ASSERT(x>=0 && x<WIDTH);
		DPVS_ASSERT(y>=0 && y<64);
		ParallelMath::inc(getData()+PLANECOUNT*x, y);
	}

	DPVS_FORCE_INLINE void dec (int x,int y)
	{
		DPVS_ASSERT(x>=0 && x<WIDTH);
		DPVS_ASSERT(y>=0 && y<64);
		ParallelMath::dec(getData()+PLANECOUNT*x, y);
	}

	DPVS_FORCE_INLINE void eor (int x,int y)
	{
		DPVS_ASSERT(x>=0 && x<WIDTH);
		DPVS_ASSERT(y>=0 && y<64);
		ParallelMath::eor1(getData()+PLANECOUNT*x, y);
	}

	void fillerXOR  (QWORD& dirty, QWORD &fvalue,int &bmin,int &bmax,int limit);
	void fillerNBIT (QWORD& dirty, QWORD *fill,QWORD &fvalue,int &bmin,int &bmax,int limit);
};

} // DPVS
#if defined (DPVS_X86_ASSEMBLY)
#	pragma warning (default:4035)
#endif
	
//------------------------------------------------------------------------
#endif //__DPVSFILLER_HPP
