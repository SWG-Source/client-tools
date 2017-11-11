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
 * Description: 	Occlusion buffer Z Gradient code
 *
 * $Archive: /dpvs/implementation/sources/dpvsOcclusionBuffer_ZGradient.cpp $
 * $Author: wili $ 
 * $Revision: #1 $
 * $Modtime: 12.02.02 15:03 $
 * $Date: 2003/01/22 $
 * 
 ******************************************************************************/

#include "dpvsOcclusionBuffer_ZGradient.hpp"

using namespace DPVS;

/*****************************************************************************
 *
 * Function:		DPVS::ZGradient::ZGradient()
 *
 * Description:		Constructor
 *
 *****************************************************************************/

ZGradient::ZGradient (void) :
	m_zmin(0.0f),
	m_zmax(0.0f),
	m_xscale(1.0f),
	m_yscale(1.0f),
	m_mode(TEST),
	m_planeCount(0)
{
	// debug debug todo: use memset instead to save some code space?

	for (int i = 0; i < 8; i++)
	{
		m_base[i]	= 0.0f;
		m_xstep[i]	= 0.0f;
		m_ystep[i]	= 0.0f;
		m_xref[i]	= 0.0f;
		m_yref[i]	= 0.0f;

	}

	m_zmax4[0] = m_zmax4[1] = m_zmax4[2] = m_zmax4[3] = 0.0f;
}

/*****************************************************************************
 *
 * Function:		DPVS::ZGradient::setupSubsampling()
 *
 * Description:		Sets up subsampling factors
 *
 * Notes:			This is called once by the occlusion buffer code
 *					(initially after the constructor)
 *				
 *					Implementation in header file
 *
 *****************************************************************************/
/*****************************************************************************
 *
 * Function:		DPVS::ZGradient::setConstant()
 *
 * Description:		Configures gradient for constant depth tests/writes
 *
 *					Implementation in header file
 *
 *****************************************************************************/
/*****************************************************************************
 *
 * Function:		DPVS::ZGradient::setGradient()
 *
 * Description:		Configures gradient for a single plane
 *
 *****************************************************************************/

/*****************************************************************************
 *
 * Function:		DPVS::ZGradient::setGradients()
 *
 * Description:		Configures gradient for multiple (up to 6) gradients
 *
 * Notes:			The planes are expressed as three vectors each
 *
 *****************************************************************************/

void ZGradient::setGradients (Mode mode, const Vector3* planes, int nPlanes, float minz, float maxz)
{
	DPVS_ASSERT(nPlanes >= 0 && nPlanes <= 6);
	DPVS_ASSERT(minz>=0.0f && minz <= maxz && maxz<=1.0f);

	m_mode					= mode;
	m_zmin					= minz;
	m_zmax					= maxz;
	m_planeCount			= nPlanes;

	if (nPlanes)
	{
		DPVS_ASSERT(planes);
		for (int i = 0; i < nPlanes; i++, planes += 3)
			setupPlane (i, planes[0], planes[1], planes[2]);
	}

	//--------------------------------------------------------------------
	// Setup SIMD variants..
	//--------------------------------------------------------------------

#if defined (DPVS_BUILD_MSC_PPACK)
	if (X86::getInstructionSets() & X86::S_SSE)
#endif
#if defined (DPVS_BUILD_MSC_PPACK) || defined (DPVS_PS2_VU_ASSEMBLY)
	if (m_mode == WRITE)
	{
		int	interpolationCount	= (nPlanes>4) ? 8 : 4;

		for(int i=m_planeCount;i<interpolationCount;i++)
		{
			m_base[i]	= m_zmax;
			m_xstep[i]	= 0.f;
			m_ystep[i]	= 0.f;
			m_xref[i]	= 0;
			m_yref[i]	= 0;
		}

		m_zmax4[0] = m_zmax;		// propagate zmax
		m_zmax4[1] = m_zmax;
		m_zmax4[2] = m_zmax;
		m_zmax4[3] = m_zmax;
	} 
#endif // DPVS_BUILD_MSC_PPACK || DPVS_PS2_VU_ASSEMBLY
}

/*****************************************************************************
 *
 * Function:		DPVS::ZGradient::interpolateTest()
 *
 * Description:		
 *
 * Notes:			
 *
 *****************************************************************************/

DPVS_FORCE_INLINE float ZGradient::interpolateTest (int x, int y) const
{
	DPVS_ASSERT( x >= 0 && y >= 0);
	register float z = m_zmin;
	for (int i = 0; i < m_planeCount; i++)
		z = Math::max(z,m_base[i] + m_xstep[i] * (x-m_xref[i]) + m_ystep[i] * (y-m_yref[i]));

	z = Math::min(z,m_zmax);
	DPVS_ASSERT( z >= m_zmin);
	return z;
}

/*****************************************************************************
 *
 * Function:		DPVS::ZGradient::interpolateWrite()
 *
 * Description:		
 *
 * Notes:			
 *
 *****************************************************************************/

DPVS_FORCE_INLINE float ZGradient::interpolateWrite (int x, int y) const
{
	DPVS_ASSERT( x >= 0 && y >= 0);
	register float z = m_zmax;
	for (int i = 0; i < m_planeCount; i++)
		z = Math::min(z,m_base[i] + m_xstep[i] * (x-m_xref[i]) + m_ystep[i] * (y-m_yref[i]));
	z = Math::max(z,m_zmin);
	DPVS_ASSERT ( z <= m_zmax);
	return z;
}

/*****************************************************************************
 *
 * Function:		DPVS::ZGradient::interpolate()
 *
 * Description:		
 *
 * Notes:			
 *
 *****************************************************************************/

float ZGradient::interpolate (int x, int y) const
{
	return (m_mode == TEST) ? interpolateTest(x,y) : interpolateWrite(x,y);
}

//------------------------------------------------------------------------
// X86 SSE implementations (only if MSVC processor pack has been installed)
//------------------------------------------------------------------------

#if defined (DPVS_BUILD_MSC_PPACK)

#define SHUFFLE(x, y, z, w)	(((x)&3)<< 6|((y)&3)<<4|((z)&3)<< 2|((w)&3))
#define	BROADCAST(XMM, INDEX)	DPVS_ASM	shufps	XMM,XMM,(((INDEX)&3)<< 6|((INDEX)&3)<<4|((INDEX)&3)<< 2|((INDEX)&3))

// interpolates 5-8 write planes (same as interpolateWrite(8))
void ZGradient::interpolateListSSE8	(float* dst, const ByteLoc* src, int N)
{
	// each UINT32 in src[]: x = lowest 15 bits, y = top 16 bits, ignore bit 15 (mask out)
	DPVS_ASM
	{

		mov			edx,this							// "this" pointer
		mov			ecx,N								// # of entries
		mov			eax,0x0000ffff
		mov			ebx,0x00007fff
		movd		mm0,eax
		movd		mm1,ebx
		punpckldq	mm1,mm0
		pcmpeqd		mm2,mm2
		mov			eax,dst								// destination array
		mov			ebx,src								// source array
		lea			ecx,[ecx*4-4]						// ecx = 4*(N-1)
loopie:
		movd		mm0,dword ptr[ebx+ecx]
		punpcklwd	mm0,mm0
		pand		mm0,mm1								;0x0000ffff00007fff
		pxor		mm0,mm2								; y = ~y | x = ~x
		psubd		mm0,mm2								; y = -y | x = -x
		cvtpi2ps	xmm5,mm0
		movaps		xmm6,xmm5
		BROADCAST(xmm5,0)
		BROADCAST(xmm6,1)
		movaps		xmm7,[edx+4*4]ZGradient.m_base		// interpolate planes 4-8
		movaps		xmm3,[edx+4*4]ZGradient.m_xref
		movaps		xmm4,[edx+4*4]ZGradient.m_yref
		addps		xmm3,xmm5							// (m_xref[i]-x)
		addps		xmm4,xmm6							// (m_yref[i]-y)
		mulps		xmm3,[edx+4*4]ZGradient.m_xstep		// m_xstep[i] * (m_xref[i]-x)
		mulps		xmm4,[edx+4*4]ZGradient.m_ystep		// m_ystep[i] * (m_yref[i]-y)
		subps		xmm7,xmm3							
		subps		xmm7,xmm4							
		minps		xmm7,[edx]ZGradient.m_zmax4			// if( zz < z)	z = zz;
		movaps		xmm0,[edx]ZGradient.m_base			// interpolate planes 0-3
		addps		xmm5,[edx]ZGradient.m_xref			// (m_xref[i]-x)
		addps		xmm6,[edx]ZGradient.m_yref			// (m_yref[i]-y)
		mulps		xmm5,[edx]ZGradient.m_xstep			// m_xstep[i] * (m_xref[i]-x)
		mulps		xmm6,[edx]ZGradient.m_ystep			// m_ystep[i] * (m_yref[i]-y)
		sub			ecx,4
		subps		xmm0,xmm5
		subps		xmm0,xmm6							
		minps		xmm7,xmm0							// if( zz < z)	z = zz;
		movhlps		xmm6,xmm7
		minps		xmm6,xmm7
		movaps		xmm7,xmm6
		shufps		xmm7,xmm7,SHUFFLE(1,1,1,1)
		minss		xmm7,xmm6
		maxss		xmm7,[edx]ZGradient.m_zmin			// if(z < m_zmin) z = m_zmin;
		movss		dword ptr [eax+ecx+4],xmm7			// dst[] = z
		jge			loopie								// if ecx >= 0
		emms
	}
}

// interpolates 1-4 write planes (same as interpolateWrite(4))
void ZGradient::interpolateListSSE4	(float* dst, const ByteLoc* src, int N)
{
	DPVS_ASM
	{
		mov			edx,this							// "this" pointer
		mov			ecx,N								// # of entries
		mov			eax,0x0000ffff
		mov			ebx,0x00007fff
		movd		mm0,eax
		movd		mm1,ebx
		punpckldq	mm1,mm0
		pcmpeqd		mm2,mm2
		mov			eax,dst								// destination array
		mov			ebx,src								// source array
		lea			ecx,[ecx*4-4]						// ecx = 4*(N-1)
		movaps		xmm0,[edx]ZGradient.m_base			
		movaps		xmm1,[edx]ZGradient.m_xref
		movaps		xmm2,[edx]ZGradient.m_yref
		movaps		xmm3,[edx]ZGradient.m_xstep
		movaps		xmm4,[edx]ZGradient.m_ystep
loopie:
		movd		mm0,dword ptr[ebx+ecx]
		punpcklwd	mm0,mm0
		pand		mm0,mm1								;0x0000ffff00007fff
		pxor		mm0,mm2								; y = ~y | x = ~x
		psubd		mm0,mm2								; y = -y | x = -x
		cvtpi2ps	xmm5,mm0
		movaps		xmm7,xmm0							// base
		movaps		xmm6,xmm5
		BROADCAST(xmm5,0)
		BROADCAST(xmm6,1)
		sub			ecx,4
		addps		xmm5,xmm1							// (m_xref[i]-x)
		addps		xmm6,xmm2							// (m_yref[i]-y)
		mulps		xmm5,xmm3							// m_xstep[i] * (m_xref[i]-x)
		mulps		xmm6,xmm4							// m_ystep[i] * (m_yref[i]-y)
		subps		xmm7,xmm5
		subps		xmm7,xmm6							
		minps		xmm7,[edx]ZGradient.m_zmax4			// if( zz < z)	z = zz;
		movhlps		xmm6,xmm7
		minps		xmm6,xmm7
		movaps		xmm7,xmm6
		shufps		xmm7,xmm7,SHUFFLE(1,1,1,1)
		minss		xmm7,xmm6
		maxss		xmm7,[edx]ZGradient.m_zmin			// if(z < m_zmin) z = m_zmin;
		movss		dword ptr [eax+ecx+4],xmm7			// dst[] = z
		jge			loopie								// if ecx >= 0
		emms
	}
}

#endif // DPVS_BUILD_MSC_PPACK


/*****************************************************************************
 *
 * Function:		DPVS::ZGradient::interpolateList()
 *
 * Description:		Interpolates a list of values
 *
 * Parameters:		dst = destination array of Z values
 *					src = source X/Y coordinates
 *					N   = number of elements to process
 *
 *****************************************************************************/

void ZGradient::interpolateList	(float* dst, const ByteLoc* src, int N)
{
	DPVS_ASSERT( dst && src && N > 0);

//------------------------------------------------------------------------
// If we have an SSE machine, use the SSE optimized routines...
//------------------------------------------------------------------------

#if defined (DPVS_BUILD_MSC_PPACK)

	DPVS_ASSERT(is128Aligned(&m_base[0]));

	// Note that the setupGradients() function must've been called..
	if (m_planeCount > 1 && (X86::getInstructionSets() & X86::S_SSE) && m_mode == WRITE)
	{
#if defined (DPVS_DEBUG)
		for (int i = 0; i < N; i++)
			dst[i] = FLT_MAX;						// make invalid value
#endif // DPVS_DEBUG

		if (m_planeCount <= 4)
			interpolateListSSE4 (dst,src,N);		// 4 planes at a time
		else
			interpolateListSSE8 (dst,src,N);		// 8 planes at a time

		// validate results in debug build
#if defined (DPVS_DEBUG)
		for (int i = 0; i < N; i++)
		{
			DPVS_ASSERT(dst[i] >= m_zmin && dst[i] <= m_zmax);
			float correct = interpolateWrite(src[i].getX(),src[i].getY());
			DPVS_ASSERT(Math::fabs(dst[i]-correct) <= 0.01f);
		}
#endif
		return;										// we're done
	}
	// FALLTHRU!
#endif // DPVS_BUILD_MSC_PPACK

//------------------------------------------------------------------------
// PS2 version
//------------------------------------------------------------------------

#if defined (DPVS_PS2_VU_ASSEMBLY)
//PS2-BEGIN
	if (m_planeCount > 1 && m_mode == WRITE)
	{
#if defined (DPVS_DEBUG)
		for (int i = 0; i < N; i++)
			dst[i] = FLT_MAX;						// make invalid value
#endif // DPVS_DEBUG

		if (m_planeCount <= 4)
			interpolateListPS2_4 (dst,src,N);		// 4 planes at a time
		else
			interpolateListPS2_8 (dst,src,N);		// 8 planes at a time

		// validate results in debug build
#if defined (DPVS_DEBUG)
		for (int i = 0; i < N; i++)
		{
			DPVS_ASSERT(dst[i] >= m_zmin && dst[i] <= m_zmax);
			DPVS_ASSERT(Math::fabs(dst[i]-interpolateWrite(src[i].getX(),src[i].getY())) <= 0.001f);
		}
#endif
		return;										// we're done
	}
	// FALLTHRU!!!
//PS2-END
#endif // __DPVS_PS2_VU_ASSEMBLY


//------------------------------------------------------------------------
// Standard C++ version
//------------------------------------------------------------------------

	if (m_planeCount == 0)							// flat
	{
		for (int i = 0; i < N; i++)
			dst[i] = m_zmin;
	} else
	{
		if (m_mode == TEST)							// test
		{
			for (int i = 0; i < N; i++)
			{
				int x = src[i].getX();
				int y = src[i].getY();
				dst[i] = interpolateTest(x,y);
			}
		} else										// write
		{
			for (int i = 0; i < N; i++)
			{
				int x = src[i].getX();
				int y = src[i].getY();
				dst[i] = interpolateWrite(x,y);
			}

		}
	}
}
 
/*****************************************************************************
 *
 * Function:		DPVS::ZGradient::setupPlane()
 *
 * Description:		Configures specified plane
 *
 * Notes:			assumes m_zmin,m_zmax,m_mode have been configured
 *
 *****************************************************************************/

// fix MSVC compiler bug by force-improving floating point consistency
#if defined (DPVS_BUILD_MSC)
#	pragma optimize ("p",on)			
#endif

void ZGradient::setupPlane	(int index, const Vector3& a, const Vector3& b, const Vector3& c)
{
	DPVS_ASSERT(index>=0 && index < 6);

	const float	f0		= (a.x - b.x) * m_xscale;
	const float	f1		= (a.x - c.x) * m_xscale;
	const float	f2		= (a.y - b.y) * m_yscale; 
	const float	f3		= (a.y - c.y) * m_yscale;
	const float	area	= (f0*f3 - f1*f2);

	static const float EPSILON = 1.0f;						// cannot measure gradients if area < EPSILON (so set constant Z)

	if(Math::fabs(area) <= EPSILON)								
	{
		m_base[index]	= (m_mode == WRITE) ? m_zmax : m_zmin;
		m_xstep[index]	= 0.0f;
		m_ystep[index]	= 0.0f;
		m_xref[index]	= 0;
		m_yref[index]	= 0;
	} else
	{
		const float	ooarea	= Math::reciprocal(area);
		const float	rx		= (a.x+b.x+c.x)*(1.0f/3.0f) * m_xscale;	// select point inside the triangle
		const float	ry		= (a.y+b.y+c.y)*(1.0f/3.0f) * m_yscale;
		const int	xRef	= Math::intFloor(rx);				// x-direction integer reference
		const int	yRef	= Math::intFloor(ry);				// y-direction integer reference
		const float	xfrac	= rx-xRef;							// x-direction sub-pixel
		const float	yfrac	= ry-yRef;							// y-direction sub-pixel

		m_xref[index]	= (float)xRef;							// DEBUG DEBUG Were ints
		m_yref[index]	= (float)yRef;
		
		DPVS_ASSERT( xfrac >= 0.0 && xfrac < 1.0f);
		DPVS_ASSERT( yfrac >= 0.0 && yfrac < 1.0f);

		const float	g0		= (a.z - b.z);
		const float	g1		= (a.z - c.z);
		const float	zx		= (f3 * g0 - f2 * g1) * ooarea;		// z-gradient x-adder
		const float	zy		= (f0 * g1 - f1 * g0) * ooarea;		// z-gradient y-adder
		const float	rz		= (a.z+b.z+c.z)*(1.0f/3.0f);

		float	zRef	= rz - xfrac*zx - yfrac*zy;			// subpixel-corrected z-reference value

		// DEBUG DEBUG write so that we find the center point
		// and then add/subtract 0.5 * zx and 0.5 * zy 

		if (m_mode == TEST)
		{
			if(zx<0) zRef += zx;							// conservative test estimate for a 8x8 block
			if(zy<0) zRef += zy;							
		} else
		{
			if(zx>0) zRef += zx;							// conservative write estimate for a 8x8 block
			if(zy>0) zRef += zy;							
		}

		m_base[index]	= zRef;
		m_xstep[index]	= zx;
		m_ystep[index]	= zy;
	}
}

//------------------------------------------------------------------------
