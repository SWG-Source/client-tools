#ifndef __DPVSOBB_HPP
#define __DPVSOBB_HPP
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
 * Desc:	Oriented Bounding Box (OBB) class
 *
 * $Archive: /dpvs/implementation/include/dpvsOBB.hpp $
 * $Author: wili $ 
 * $Revision: #1 $
 * $Modtime: 7.05.01 12:10 $
 * $Date: 2003/01/22 $
 * 
 *****************************************************************************/

#if !defined (__DPVSMATRIX_HPP)
#	include "dpvsMatrix.hpp"
#endif

/******************************************************************************
 *
 * Class:			DPVS::OBB
 *
 * Description:		Class for storing and manipulating oriented bounding
 *					boxes
 *
 * Notes:			see "dpvsBounds.hpp" for OBB construction routines
 *
 ******************************************************************************/

namespace DPVS
{

class OBB
{
private:
	Matrix4x3			m_matrix;			// rotation and translation matrix with scaling
public:
						OBB					(void)										: m_matrix()					{ /* initialized by vector&matrix ctors*/	}
						OBB					(const OBB& s)								: m_matrix(s.m_matrix)	{ }
	OBB&				operator=			(const OBB& s)								{ m_matrix = s.m_matrix; return *this; }
	void				setMatrix			(const Matrix4x3& m)						{ m_matrix = m;			}
	const Matrix4x3&	getMatrix			(void) const								{ return m_matrix;		}
	float				getVolume			(void) const								{ return getHalfExtentLength(0)*getHalfExtentLength(1)*getHalfExtentLength(2)*8.0f; }
	float				getHalfExtentLength	(int i) const								{ DPVS_ASSERT(i>=0&&i<3); return m_matrix.getColumn(i).length(); }
	void				getAABBExtentLength	(Vector3& l) const;
	float				getSurfaceArea		(void) const
	{
		Vector3 d(2.0f*getHalfExtentLength(0),2.0f*getHalfExtentLength(1),2.0f*getHalfExtentLength(2));
		return 2.0f*(d.x*d.y + d.x*d.z + d.y*d.z); 
	}
};

} // DPVS




//------------------------------------------------------------------------
#endif // __DPVSOBB_HPP
