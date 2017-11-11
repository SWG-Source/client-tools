#ifndef __DPVSAABB_HPP
#define __DPVSAABB_HPP
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
 * Desc:	Axis-Aligned Bounding Box class
 *
 * $Archive: /dpvs/implementation/include/dpvsAABB.hpp $
 * $Author: wili $ 
 * $Revision: #1 $
 * $Modtime: 20.06.01 9:42 $
 * $Date: 2003/01/22 $
 * 
 *****************************************************************************/

#if !defined (__DPVSVECTOR_HPP)
#	include "dpvsVector.hpp"			
#endif

/******************************************************************************
 *
 * Class:			DPVS::AABB
 *
 * Description:		Class for storing and manipulating axis-aligned 3D bounds
 *
 * Notes:			see "dpvsBounds.hpp" for AABB construction routines
 *
 *******************************************************************************/

namespace DPVS
{

class AABB
{
private:
	Vector3	m_min;							// min XYZ coordinates
	Vector3	m_max;							// max XYZ coordinates
public:

	enum Corner								// corner enumeration
	{
		MINX_MINY_MINZ = 0,
		MAXX_MINY_MINZ = 1,
		MINX_MAXY_MINZ = 2,
		MAXX_MAXY_MINZ = 3,
		MINX_MINY_MAXZ = 4,
		MAXX_MINY_MAXZ = 5,
		MINX_MAXY_MAXZ = 6,
		MAXX_MAXY_MAXZ = 7
	};

DPVS_FORCE_INLINE 					AABB				(void) : m_min(0.0f,0.0f,0.0f),m_max(0.0f,0.0f,0.0f)					{ /* m_min and m_max are cleared by their constructors*/ }
DPVS_FORCE_INLINE 					AABB				(const AABB& s) : m_min(s.m_min),m_max(s.m_max) {}
DPVS_FORCE_INLINE	AABB&			operator=			(const AABB& s)			{ m_min = s.m_min; m_max = s.m_max; return *this; }
DPVS_FORCE_INLINE 	void			set					(const Vector3& mn, const Vector3& mx);
DPVS_FORCE_INLINE 	void			setMin				(int i, float f)		{ DPVS_ASSERT(i>=0 && i<3); m_min[i] = f; }
DPVS_FORCE_INLINE 	void			setMax				(int i, float f)		{ DPVS_ASSERT(i>=0 && i<3); m_max[i] = f; }
DPVS_FORCE_INLINE 	void			setMin				(const Vector3& mn)		{ m_min = mn; }
DPVS_FORCE_INLINE 	void			setMax				(const Vector3& mx)		{ m_max = mx; }
DPVS_FORCE_INLINE 	const Vector3&	getMin				(void) const			{ return m_min; }
DPVS_FORCE_INLINE 	const Vector3&	getMax				(void) const			{ return m_max; }
DPVS_FORCE_INLINE 	void			grow				(const AABB& s);		
DPVS_FORCE_INLINE 	void			grow				(const Vector3&);		
DPVS_FORCE_INLINE 	void			clamp				(const AABB& s);
DPVS_FORCE_INLINE 	float			getAxisLength		(int axis) const;
DPVS_FORCE_INLINE 	float			getVolume			(void) const;
DPVS_FORCE_INLINE 	Vector3			getCenter			(void) const;
DPVS_FORCE_INLINE 	Vector3			getDimensions		(void) const;
DPVS_FORCE_INLINE 	float			getDiagonalLength	(void) const;
DPVS_FORCE_INLINE 	float			getDiagonalLengthSqr(void) const;
DPVS_FORCE_INLINE 	float			getMaxAxisLength	(void) const;
DPVS_FORCE_INLINE 	bool			contains			(const Vector3&) const;
DPVS_FORCE_INLINE 	bool			containsFully		(const AABB& s) const;
DPVS_FORCE_INLINE 	bool			operator==			(const AABB& s) const	{ return m_min == s.m_min && m_max == s.m_max; }
DPVS_FORCE_INLINE 	bool			operator!=			(const AABB& s) const	{ return m_min != s.m_min || m_max != s.m_max; }
DPVS_FORCE_INLINE 	Vector3			getCorner			(Corner corner) const;
DPVS_FORCE_INLINE 	const float*	getFloatPtr			(void) const			{ return &m_min[0]; }
DPVS_FORCE_INLINE 	float			getSurfaceArea		(void) const;
DPVS_FORCE_INLINE 	bool			isOK				(void) const;
					void			validateBounds		(void);
};

//------------------------------------------------------------------------
// Implementation
//------------------------------------------------------------------------

DPVS_FORCE_INLINE bool	AABB::isOK (void) const
{
	return (m_min.x <= m_max.x && m_min.y <= m_max.y && m_min.z <= m_max.z);
}

DPVS_FORCE_INLINE Vector3 AABB::getDimensions (void) const
{
	return m_max - m_min;
}

DPVS_FORCE_INLINE float AABB::getSurfaceArea (void) const			
{ 
	Vector3 d = getDimensions(); 
	return 2.0f*(d.x*d.y + d.x*d.z + d.y*d.z); 
}

DPVS_FORCE_INLINE Vector3 AABB::getCorner (Corner corner) const
{
	DPVS_ASSERT (corner >= 0 && corner <= 7);
	return Vector3((corner&1) ? m_max.x : m_min.x, (corner&2) ? m_max.y : m_min.y,(corner&4) ? m_max.z : m_min.z);
}

DPVS_FORCE_INLINE void AABB::set (const Vector3& mn, const Vector3& mx) 
{ 
	setMin(mn);
	setMax(mx);
	DPVS_ASSERT(isOK());
}

DPVS_FORCE_INLINE void AABB::grow (const Vector3& s)		
{
	m_min = min(m_min,s);
	m_max = max(m_max,s);

	DPVS_ASSERT(isOK());
}


DPVS_FORCE_INLINE void AABB::grow (const AABB& s)		
{
	m_min = min(m_min,s.m_min);
	m_max = max(m_max,s.m_max);
}

DPVS_FORCE_INLINE void AABB::clamp (const AABB& s)		
{
	m_min = max(m_min,s.m_min);
	m_max = min(m_max,s.m_max);

	DPVS_ASSERT(isOK());
}

DPVS_FORCE_INLINE float AABB::getDiagonalLength (void) const
{
	return (m_max - m_min).length();
}

DPVS_FORCE_INLINE float AABB::getDiagonalLengthSqr (void) const
{
	return (m_max - m_min).lengthSqr();
}

DPVS_FORCE_INLINE Vector3 AABB::getCenter (void) const
{
	return Vector3((m_max.x+m_min.x)*0.5f,(m_max.y+m_min.y)*0.5f,(m_max.z+m_min.z)*0.5f);
}

DPVS_FORCE_INLINE float AABB::getVolume (void) const			
{ 
	return (m_max.x-m_min.x) * (m_max.y-m_min.y) * (m_max.z - m_min.z);
}

DPVS_FORCE_INLINE float AABB::getAxisLength (int axis) const			
{ 
	DPVS_ASSERT(axis>=0 && axis<=2);
	return (m_max[axis] - m_min[axis]);
}

DPVS_FORCE_INLINE float AABB::getMaxAxisLength (void) const			
{ 
	float x = m_max[0] - m_min[0];
	float y = m_max[1] - m_min[1];
	float z = m_max[2] - m_min[2];
	return Math::max(Math::max(x,y),z);
}

DPVS_FORCE_INLINE bool AABB::contains (const Vector3& v) const
{
	// DEBUG DEBUG TODO IMPROVE
	return (v.x >= m_min.x &&
		    v.x <= m_max.x &&
			v.y >= m_min.y &&
		    v.y <= m_max.y &&
			v.z >= m_min.z &&
		    v.z <= m_max.z);
}

DPVS_FORCE_INLINE bool AABB::containsFully (const AABB& s) const	
{ 
	// DEBUG DEBUG TODO IMPROVE
	if (s.m_min.x < m_min.x ||
	    s.m_max.x > m_max.x ||
	    s.m_min.y < m_min.y ||
	    s.m_max.y > m_max.y ||
	    s.m_min.z < m_min.z ||
	    s.m_max.z > m_max.z) 
		return false;
	
	return true;
}
} // DPVS

//------------------------------------------------------------------------
#endif // __DPVSAABB_HPP
