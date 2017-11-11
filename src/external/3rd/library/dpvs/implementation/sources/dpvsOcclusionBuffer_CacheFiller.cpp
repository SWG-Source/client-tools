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
 * Description: 	Occlusion Cache filler code
 *
 * $Archive: /dpvs/implementation/sources/dpvsOcclusionBuffer_CacheFiller.cpp $
 * $Author: wili $ 
 * $Revision: #1 $
 * $Modtime: 4/22/02 6:26p $
 * $Date: 2003/01/22 $
 * 
 ******************************************************************************/

#include "dpvsFiller.hpp"
#include "dpvsStatistics.hpp"

using namespace DPVS;

/*****************************************************************************
 *
 * Function:		DPVS::OcclusionBuffer::Cache::fillPlane0()
 *
 * Description:		XOR filler
 *
 *****************************************************************************/

// Temporarily placed into this file so that we can see the actual time spent here..
void OcclusionBuffer::Cache::fillPlane0 (int lo,int hi,const QWORD pattern)
{
	DPVS_ASSERT(UINT32(lo)<=WIDTH);
	DPVS_ASSERT(UINT32(hi)<=WIDTH);
	DPVS_ASSERT(lo <= hi);

#if defined (DPVS_X86_ASSEMBLY)
	if (X86::getInstructionSets() & X86::S_MMX)			// X86 MMX implementation..
	{
		DPVS_ASSERT(PLANECOUNT==3 && sizeof(QWORD)==8);	// we make these assumptions

		QWORD*			d	= getData() + PLANECOUNT*lo;
		const QWORD*	src = &pattern;

		__asm
		{
			mov		eax,src
			movq	mm0,QWORD PTR [eax]
			mov		eax,d
			mov		edx,hi
			sub		edx,lo
			cmp		edx,3
			jle		padding
			mov		ecx,edx
			shr		ecx,2
			and		edx,3
loopie:												// unrolled by 4
			add		eax,3*4*8
			dec		ecx
			movq	[eax-3*4*8+0],mm0
			movq	[eax-3*4*8+3*1*8],mm0
			movq	[eax-3*4*8+3*2*8],mm0
			movq	[eax-3*4*8+3*3*8],mm0
			jnz		loopie

padding:	cmp		edx,0							// 0-3 entry padding
			je		outta
loopie2:
			movq	[eax],mm0
			add		eax,3*8
			dec		edx
			jnz		loopie2
outta:
			emms
		}

	} else // FALLTHRU IF NO MMX!
#endif // DPVS_X86_ASSEMBLY							// standard C++ implementation with QWORD math
	{
		// DEBUG DEBUG

		QWORD*	ptr = getData() + PLANECOUNT*lo;
		QWORD	p(pattern);
		
		for(int i=lo;i<hi;i++)
		{
			*ptr = p;
			ptr += PLANECOUNT;
		}

	}
}

/*
// old implementation...
void OcclusionBuffer::Cache::fill (int bitplane,int lo,int hi,const QWORD pattern)
{

	DPVS_ASSERT(UINT32(bitplane)<PLANECOUNT);
	DPVS_ASSERT(UINT32(lo)<=WIDTH);
	DPVS_ASSERT(UINT32(hi)<=WIDTH);

	QWORD* ptr = getData() + PLANECOUNT*lo + bitplane;

	for(int i=lo;i<hi;i++)
	{
		*ptr = pattern;
		ptr += PLANECOUNT;
	}
}
*/
/*****************************************************************************
 *
 * Function:		DPVS::OcclusionBuffer::Cache::fillerXOR()
 *
 * Description:		XOR filler
 *
 *****************************************************************************/

void OcclusionBuffer::Cache::fillerXOR(QWORD& dirty, QWORD& fvalue,int& bmin,int& bmax, int limit) 
{

	//-------------------------------------------------------
	// Copy border(inclusive) -> min(exclusive)
	// if fvalue==0, skip work
	//-------------------------------------------------------

	if(fvalue.empty())
		bmin = m_mn >> BLOCK_BITS;
	else
	{
		bmin = 0;
		fillPlane0(0,m_mn,fvalue);
	}

	//-------------------------------------------------------
	// Execute filler for min(inclusive) -> max(exclusive)
	//-------------------------------------------------------

	const int mx	= m_mx + 1;						// exclusive

#if defined (DPVS_X86_ASSEMBLY)
	if (X86::getInstructionSets() & X86::S_MMX)		// X86 MMX implementation..
	{
		QWORD*	l	= (*this)[m_mn];				// ptr to first line participating for filling
		int		len = mx - m_mn;

		__asm
		{
			push	eax
			push	esi
			push	ecx
			mov		eax,fvalue						// eax = address of fvalue
			mov		esi,l							// esi = l
			mov		ecx,len							// ecx = loop counter
			movq	mm0,QWORD PTR [eax]				// mm0 = fvalue
			movq	mm1,mm0							// mm1 = dirty scanlines (init to fvalue)
			cmp		ecx,0
			jle		done
			jmp		loopie							
			align 16
loopie:												// DEBUG DEBUG TODO: UNROLL THIS LOOP?
			movq	mm2,QWORD PTR [esi+0]			// mm2 = l[0]
			add		esi,24							// l += PLANECOUNT
			dec		ecx								
			pxor	mm0,mm2							// fvalue ^= l[0]
			movq	QWORD PTR [esi-24],mm0			// l[0] = fv
			por		mm1,mm2							// dirty |= fv
			jnz		loopie							// next...
done:
			movq	QWORD PTR [eax], mm0			// update value
			mov		eax,dirty						// get address of 'dirty'
			movq	QWORD PTR [eax],mm1				// update dirty
			emms									// end of MMX
			pop		ecx
			pop		esi
			pop		eax
		}
	} else // FALLTHRU IF NO MMX!
#endif // DPVS_X86_ASSEMBLY							// standard C++ implementation with QWORD math
	{
		QWORD*	l = (*this)[m_mn];					// ptr to first line participating for filling
		QWORD	dirtyScanlines(fvalue);
		QWORD	fv(fvalue);							// local copy
		for (int mn = m_mn; mn < mx; mn++)
		{
			fv ^= l[0];								// XOR filler
			l[0] = fv;								// write to surface
			dirtyScanlines |= fv;
			l += PLANECOUNT;
		}

		dirty  = dirtyScanlines;
		fvalue = fv;								// copy fvalue
	}
	
	//-------------------------------------------------------
	// Fill max(inclusive) -> BUCKET_W(exclusive)
	//-------------------------------------------------------

	if(fvalue.empty())
	{
		bmax = (limit < BUCKET_W) ? ((limit+7) >> BLOCK_BITS) : (m_mx >> BLOCK_BITS) + 1;		
	} else // !fv.empty()
	{
		if (limit >= BUCKET_W)
			limit = BUCKET_W;
		bmax = (limit + 7) >> BLOCK_BITS;
		fillPlane0 (mx,limit,fvalue);
	}
}

/*****************************************************************************
 *
 * Function:		DPVS::OcclusionBuffer::Cache::fillerNBIT()
 *
 * Description:		NBIT filler
 *
 *****************************************************************************/


void OcclusionBuffer::Cache::fillerNBIT(QWORD& dirty, QWORD *f,QWORD &fvalue,int &bmin,int &bmax,int limit)
{
	DPVS_ASSERT(f && !(reinterpret_cast<size_t>(f)&(size_t)7));	// make sure it is aligned!!!

	m_fullCoverage = MAYBE_COVERED;

	//-------------------------------------------------------
	// Copy border(inclusive) -> min(exclusive)
	// if fvalue==0, skip work
	//-------------------------------------------------------

	if(fvalue.empty())
		bmin = m_mn >> BLOCK_BITS;
	else
	{
		bmin = 0;
		fillPlane0 (0,m_mn,fvalue);
	}

	//-------------------------------------------------------
	// Execute filler for min(inclusive) -> max(exclusive)
	//-------------------------------------------------------

	const int	mx	= m_mx + 1;							// exclusive

#if defined (DPVS_X86_ASSEMBLY)
	if (X86::getInstructionSets() & X86::S_MMX)			// X86 MMX implementation..
	{
		QWORD*	l	= (*this)[m_mn];					// ptr to first line participating for filling
		int		len = mx - m_mn;
		__asm
		{
			push    eax
			push    esi
			push    ecx
			push    edx
			mov		eax,f								// eax = f
			mov		esi,l								// esi = l
			mov		ecx,len								// ecx = loop counter
			mov		edx,fvalue							// address of fvalue
			movq	mm0,QWORD PTR [eax+0]				// mm0 = f[0]
			movq	mm1,QWORD PTR [eax+8]				// mm1 = f[1]
			movq	mm2,QWORD PTR [eax+16]				// mm2 = f[2]
			movq	mm4,QWORD PTR [edx]					// mm4 = fvalue
			movq	mm3,mm4								// mm3 = m_dirtyScanlines(fvalue)
			cmp		ecx,0
			jle		done
			jmp		loopie
			align 16
	loopie:
			movq	mm6,QWORD PTR [esi+0]				// mm6 = l[0]
			movq	mm7,QWORD PTR [esi+8]				// mm7 = l[1]
			movq	mm4,mm0								// carry0 = f[0]
			pand	mm4,mm6								// carry0 = f[0] & l[0]
			movq	mm5,mm7								// carry1 = l[1]
			por		mm5,mm4								// carry1 = l[1] | carry0
			pand	mm5,mm1								// carry1 = f[1] & (l[1] | carry0)
			pxor	mm0,mm6								// f[0] ^= l[0]
			movq	mm6,mm7								// l[1]
			pand	mm6,mm4								// l[1] & carry0
			por		mm5,mm6								// carry1 = (l[1]&carry0) | (f[1]&(l[1]|carry0))
			movq	mm6,QWORD PTR [esi+16]				// l[2]
			add		esi,24								// l += PLANECOUNT (3 * sizeof(QWord))
			dec		ecx									// decrease loop counter
			pxor	mm7,mm4								// l[1] ^ carry0
			pxor	mm1,mm7								// f[1] ^= (l[1] ^ carry0)
			pxor	mm6,mm5								// l[2] ^ carry1
			pxor	mm5,mm5								// temporary value holding zero
			pxor	mm2,mm6								// f[2] ^= (l[2] ^ carry1)
			movq	mm4,mm0								// f[0]
			por		mm4,mm1								// f[0] | f[1]
			por		mm4,mm2								// f[0] | f[1] | f[2]
			por		mm3,mm4								// dirtyScanlines |= f[0] | f[1] | f[2]
			movq	QWORD PTR [esi+0-24],mm4			// l[0] = f[0] | f[1] | f[2]
			movq	QWORD PTR [esi+8-24],mm5			// l[1] = 0
			movq	QWORD PTR [esi+16-24],mm5			// l[2] = 0
			jnz		loopie								// more work to do
	done:
			movq	QWORD PTR [eax+0],mm0				// update f[0]
			movq	QWORD PTR [eax+8],mm1				// update f[1]
			movq	QWORD PTR [eax+16],mm2				// update f[2]
			movq	QWORD PTR [edx],mm4					// fvalue = (f[0] | f[1] | f[2])
			mov		eax,dirty
			movq	QWORD PTR [eax],mm3					// update dirty
			emms										// end of MMX
			pop		edx
			pop		ecx
			pop		esi
			pop		eax
		}
	} else // FALLTHRU IF NO MMX!
#endif // DPVS_X86_ASSEMBLY								// standard C++ implementation with QWORD math
	{	
		QWORD*	l	= (*this)[m_mn];					// ptr to first line participating for filling
		QWORD	dirtyScanlines( fvalue);

		for (int mn = m_mn;mn < mx;mn++)
		{
			ParallelMath::add3(f,l);
			ParallelMath::any(l[0],f);
			dirtyScanlines |= l[0];
			l[1].setZero();
			l[2].setZero();
			l += PLANECOUNT;
		}
		ParallelMath::any(fvalue,f);
		dirty = dirtyScanlines | fvalue;
	}
	
	//-------------------------------------------------------
	// Fill max(inclusive) -> BUCKET_W(exclusive)
	//-------------------------------------------------------

	if(fvalue.empty())
	{
		bmax = (limit < BUCKET_W) ? ((limit+7) >> BLOCK_BITS) : (m_mx >> BLOCK_BITS) + 1;		
	} else // !fv.empty()
	{
		if (limit >= BUCKET_W)
			limit = BUCKET_W;
		bmax = (limit + 7) >> BLOCK_BITS;
		fillPlane0 (mx,limit,fvalue);
	}
}


//------------------------------------------------------------------------

