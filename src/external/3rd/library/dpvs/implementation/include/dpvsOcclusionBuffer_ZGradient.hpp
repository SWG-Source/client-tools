#ifndef __DPVSOCCLUSIONBUFFERZGRADIENT_HPP
#define __DPVSOCCLUSIONBUFFERZGRADIENT_HPP
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
 * Desc:	Z-Gradient computation class for occlusion buffer
 *
 * $Archive: /dpvs/implementation/include/dpvsOcclusionBuffer_ZGradient.hpp $
 * $Author: wili $ 
 * $Revision: #1 $
 * $Modtime: 12.02.02 15:02 $
 * $Date: 2003/01/22 $
 * 
 *****************************************************************************/

#if !defined (__DPVSVECTOR_HPP)
#	include "dpvsVector.hpp"
#endif
#include "dpvsX86.hpp"

namespace DPVS
{

class ByteLoc
{
public:
	ByteLoc() : m_data(0)				{ }

	void	set		(int x,int y,bool full=false)		
	{
		DPVS_ASSERT(x>=0 && x<=16383);
		DPVS_ASSERT(y>=0 && y<=16383);
		m_data = (UINT32)(x) | ((UINT32)(full)<<15) | ((UINT32)(y)<<16);
	}
	int		getX	(void) const						{ return (int)(m_data & 32767);	}
	int		getY	(void) const						{ return (int)(m_data>>16);		}
	bool	full	(void) const						{ return (m_data & 32768)!=0;	}

private:
	UINT32	m_data;
};

/******************************************************************************
 *
 * Class:			DPVS::ZGradient
 *
 * Description:		Class for computing depth gradients for occlusion buffering
 *
 ******************************************************************************/

class ZGradient
{
public:

	enum Mode
	{
		TEST		= 0,						// gradients should be conservative for testing
		WRITE		= 1							// gradients should be conservative for writing
	};

					ZGradient		(void);
	float			getMax			(void) const						{ return m_zmax; }
	float			getMin			(void) const						{ return m_zmin; }
	Mode			getMode			(void) const						{ return m_mode; }

	float			interpolate		(int x, int y) const;
	void			interpolateList	(float* dst, const ByteLoc* src, int N);

	void			setConstant		(Mode mode, float z);
	void			setGradient		(Mode mode, const Vector3& a, const Vector3& b, const Vector3& c, float minz, float maxz);
	void			setGradients	(Mode mode, const Vector3* planes, int nPlanes, float minz, float maxz);
	void			setSubSampling	(float sx, float sy);				


private:
				ZGradient				(const ZGradient&);
	ZGradient&	operator=				(const ZGradient&);

	void		setupPlane				(int index, const Vector3& a, const Vector3& b, const Vector3& c);
	
//PS2-BEGIN
	void		interpolateListPS2_4	(float*, const ByteLoc*, int);
	void		interpolateListPS2_8	(float*, const ByteLoc*, int);
//PS2-END
	void		interpolateListSSE4		(float*, const ByteLoc*, int);
	void		interpolateListSSE8		(float*, const ByteLoc*, int);
	float		interpolateTest			(int,int) const;
	float		interpolateWrite		(int,int) const;

	//Plane struct opened to arrays (for SIMD implementation)
	float		DPVS_ALIGN16(m_base[8]);		// Z-reference value (default: zmax if WRITE, zmin if TEST)
	float		m_xstep[8];		// Z x-stepper (default: 0)
	float		m_ystep[8];		// Z y-stepper (default: 0)
	float		m_xref[8];		// reference point x-coord (default: 0)
	float		m_yref[8];		// reference point y-coord (default: 0)	
	float		m_zmax4[4];		// 4 copies of m_zmax

	float		m_zmin;						// absolute minimum z
	float		m_zmax;						// absolute maxiumum z
	float		m_xscale;					// x-subsampling scale (initialized in occbuf ctor)
	float		m_yscale;					// y-subsampling scale (initialized in occbuf ctor)
	Mode		m_mode;						// current interpolation mode
	int			m_planeCount;				// number of planes
};											


DPVS_FORCE_INLINE void ZGradient::setSubSampling (float sx, float sy)
{
	m_xscale = sx;
	m_yscale = sy;
}

DPVS_FORCE_INLINE void ZGradient::setConstant (Mode m, float z)
{
	DPVS_ASSERT( z >= 0.0f && z <= 1.0f);

	m_planeCount	= 0;								// we have zero planes (only use zmin/zmax values)
	m_mode			= m;
	m_zmin			= z; 
	m_zmax			= z; 
}	

DPVS_FORCE_INLINE void ZGradient::setGradient (Mode mode, const Vector3& a, const Vector3& b, const Vector3& c, float minz, float maxz)
{
	DPVS_ASSERT(minz>=0.0f && minz <= maxz && maxz<=1.0f);
	m_mode					= mode;
	m_zmin					= minz;
	m_zmax					= maxz;
	m_planeCount			= 1;
	setupPlane		(0, a,b,c);
}


} // DPVS

//------------------------------------------------------------------------
#endif // __DPVSOCCLUSIONBUFFERZGRADIENT_HPP
