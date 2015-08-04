#ifndef __DPVSSPHERE_HPP
#define __DPVSSPHERE_HPP
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
 * Desc:	Sphere class
 *
 * $Archive: /dpvs/implementation/include/dpvsSphere.hpp $
 * $Author: wili $ 
 * $Revision: #1 $
 * $Modtime: 7.05.01 12:10 $
 * $Date: 2003/01/22 $
 * 
 *****************************************************************************/

#if !defined (__DPVSMATH_HPP)
#	include "dpvsMath.hpp"				
#endif

/******************************************************************************
 *
 * Class:			DPVS::Sphere
 *
 * Description:		Class for representing a sphere 
 *
 * Notes:			see "dpvsBounds.hpp" for sphere construction routines
 *
 ******************************************************************************/

namespace DPVS
{
class Sphere
{
private:
	Vector4				m_vector;			// Vector3 (center), radius in W component
public:
						Sphere				(void)	: m_vector() {}
						Sphere				(const Sphere& s) : m_vector(s.m_vector) {}
						Sphere				(const Vector3& center, float rad) : m_vector(center,rad) {}
	Sphere&				operator=			(const Sphere& s)						{ m_vector = s.m_vector; return *this; }
	void				set					(const Vector3& center, float rad)		{ m_vector.make(center,rad); }
	void				setCenter			(const Vector3& center)					{ m_vector.x = center.x; m_vector.y = center.y; m_vector.z = center.z; }
	void				setRadius			(float rad)								{ DPVS_ASSERT(rad>=0.0f); m_vector.w = rad;	}
	const Vector3&		getCenter			(void) const							{ return (const Vector3&)m_vector;	}
	float				getRadius			(void) const							{ return m_vector.w;	}
	float				getVolume			(void) const							{ return (4.0f*Math::PI/3.0f)*m_vector.w*m_vector.w*m_vector.w;	}
	bool				operator==			(const Sphere& s) const					{ return m_vector == s.m_vector;	}
	bool				operator!=			(const Sphere& s) const					{ return m_vector != s.m_vector;	}
};
} // DPVS

//------------------------------------------------------------------------
#endif // __DPVSSPHERE_HPP
