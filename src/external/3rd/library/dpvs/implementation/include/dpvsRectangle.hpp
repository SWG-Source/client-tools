#ifndef __DPVSRECTANGLE_HPP
#define __DPVSRECTANGLE_HPP
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
 * Description: 	Rectangle classes
 *
 * $Archive: /dpvs/implementation/include/dpvsRectangle.hpp $
 * $Author: wili $ 
 * $Revision: #1 $
 * $Modtime: 20.06.01 21:31 $
 * $Date: 2003/01/22 $
 * 
 ******************************************************************************/

#if !defined (__DPVSBASEMATH_HPP)
#	include "dpvsBaseMath.hpp"
#endif

namespace DPVS
{
class FloatRectangle;

enum Space
{
	SPACE_CAMERA			= 0,		// [0,1]
	SPACE_RASTER			= 1,		// output pixels
	SPACE_OCCLUSION			= 2,		// after image space scales
	SPACE_OCCLUSION_BLOCK	= 3,		// blocks
	SPACE_OCCLUSION_BUCKET	= 4,		// buckets
	SPACE_FUCK_THAT			= 5,		// 
	SPACE_UNDEFINED						// undefined space...
};


/******************************************************************************
 *
 * Class:			DPVS::FloatRectangle
 *
 * Description:		Float rectangle class
 *
 *****************************************************************************/

class FloatRectangle
{
public:
	float			x0,x1,y0,y1;

#ifdef DPVS_DEBUG
	Space	space;
	DPVS_FORCE_INLINE Space			getSpace		(void) const					{ return space; }
	DPVS_FORCE_INLINE void			setSpace		(Space s)						{ space = s; }

#else
	DPVS_FORCE_INLINE void			setSpace		(Space)							{ /*nada*/ }
#endif

DPVS_FORCE_INLINE					FloatRectangle	(float px0=0,float px1=0,float py0=0,float py1=0) : x0(px0),x1(px1),y0(py0),y1(py1) { setSpace(SPACE_UNDEFINED); }
DPVS_FORCE_INLINE	float			width			(void) const					{ return x1-x0; }
DPVS_FORCE_INLINE	float			height			(void) const					{ return y1-y0; }
DPVS_FORCE_INLINE	float			area			(void) const					{ return (x1-x0)*(y1-y0); }
DPVS_FORCE_INLINE	bool			operator==		(const FloatRectangle &r) const	{ return x0==r.x0 && x1==r.x1 && y0==r.y0 && y1==r.y1; }
DPVS_FORCE_INLINE	bool			operator!=		(const FloatRectangle &r) const	{ return x0!=r.x0 || x1!=r.x1 || y0!=r.y0 || y1!=r.y1; }

DPVS_FORCE_INLINE	void			set				(float px0,float px1,float py0,float py1)	{ x0=px0; x1=px1; y0=py0; y1=py1; }
					void			downSample		(float x,float y);
DPVS_FORCE_INLINE	void			upSample		(float x,float y)				{ x0*=x; x1*=x; y0*=y; y1*=y; }
DPVS_FORCE_INLINE	void			grow			(float x,float y)				{ x0 = Math::min(x, x0); x1 = Math::max(x, x1); y0 = Math::min(y, y0); y1 = Math::max(y, y1); }
DPVS_FORCE_INLINE	bool			isInside		(float x,float y) const			{ return (x>=x0 && x<x1 && y>=y0 && y<y1) ? true : false; }
					bool			intersect		(const FloatRectangle &r);
};

DPVS_FORCE_INLINE void FloatRectangle::downSample (float x,float y)				
{ 
	float oox = 1.0f / x;
	float ooy = 1.0f / y;
	x0*=oox; 
	x1=(x1+x-1)*oox; 
	y0*=ooy; 
	y1=(y1+y-1)*ooy; 
}

DPVS_FORCE_INLINE bool FloatRectangle::intersect (const FloatRectangle &r)
{
	DPVS_ASSERT(getSpace() != SPACE_UNDEFINED);
	DPVS_ASSERT(getSpace() == r.getSpace());

	x0 = Math::max(x0,r.x0);
	x1 = Math::min(x1,r.x1);

	if(x1<=x0)			//horizontally degenerate
		return false;

	y0 = Math::max(y0,r.y0);
	y1 = Math::min(y1,r.y1);

	return (y1 > y0);
}

/******************************************************************************
 *
 * Class:			DPVS::IntRectangle
 *
 * Description:		Integer rectangle class
 *
 *****************************************************************************/

class IntRectangle
{
public:
	int				x0,x1,y0,y1;

#ifdef DPVS_DEBUG
	Space	space;
	DPVS_FORCE_INLINE Space		getSpace		(void) const				{ return space; }
	DPVS_FORCE_INLINE void			setSpace		(Space s)					{ space = s; }
	DPVS_FORCE_INLINE void			copySpace		(const FloatRectangle& r)	{ space = r.space; }
#else
	DPVS_FORCE_INLINE void			setSpace		(Space)						{ /*nada*/ }
	DPVS_FORCE_INLINE void			copySpace		(const FloatRectangle& )	{ /*nada*/ }
#endif

DPVS_FORCE_INLINE					IntRectangle	(int px0=0,int px1=0,int py0=0,int py1=0) : x0(px0),x1(px1),y0(py0),y1(py1) { setSpace(SPACE_UNDEFINED); }
DPVS_FORCE_INLINE	int				width			(void) const				{ return x1-x0; }
DPVS_FORCE_INLINE	int				height			(void) const				{ return y1-y0; }
DPVS_FORCE_INLINE	int				area			(void) const				{ return (x1-x0)*(y1-y0); }
DPVS_FORCE_INLINE	bool			operator==		(const IntRectangle &r) const	{ return x0==r.x0 && x1==r.x1 && y0==r.y0 && y1==r.y1; }
DPVS_FORCE_INLINE	bool			operator!=		(const IntRectangle &r) const	{ return x0!=r.x0 || x1!=r.x1 || y0!=r.y0 || y1!=r.y1; }
DPVS_FORCE_INLINE	void			set				(int px0,int px1,int py0,int py1)	{ x0=px0; x1=px1; y0=py0; y1=py1; }
DPVS_FORCE_INLINE	void			downSample		(int x,int y);
DPVS_FORCE_INLINE	void			upSample		(int x,int y)				{ x0*=x; x1*=x; y0*=y; y1*=y; }
DPVS_FORCE_INLINE	void			grow			(int x,int y)				{ if(x<x0) x0=x; if(x>x1) x1=x; if(y<y0) y0=y; if(y>y1) y1=y; }
DPVS_FORCE_INLINE	void			pad				(int x,int y)				{ if(x1%x) x1+=x-x1%x; if(y1%y) y1+=y-y1%y; }
DPVS_FORCE_INLINE	bool			isInside		(int x,int y) const			{ return (x>=x0 && x<x1 && y>=y0 && y<y1) ? true : false; }
					bool			intersect		(const IntRectangle &r);
};

DPVS_FORCE_INLINE void IntRectangle::downSample	(int ix,int iy)				
{ 
	DPVS_ASSERT (ix > 0 && iy > 0);
	unsigned int x = ix;
	unsigned int y = iy;

	x0/=x; 
	x1=(x1+x-1)/x; 
	y0/=y; 
	y1=(y1+y-1)/y; 
}

DPVS_FORCE_INLINE bool IntRectangle::intersect (const IntRectangle& r)
{
	DPVS_ASSERT(getSpace() != SPACE_UNDEFINED);
	DPVS_ASSERT(getSpace() == r.getSpace());

	x0 = Math::max(x0,r.x0);
	x1 = Math::min(x1,r.x1);

	if(x1<=x0)			//horizontally degenerate
		return false;

	y0 = Math::max(y0,r.y0);
	y1 = Math::min(y1,r.y1);

	return (y1 > y0);
}

} // DPVS

//------------------------------------------------------------------------
#endif // __DPVSRECTANGLE_HPP
