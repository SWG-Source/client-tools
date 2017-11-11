#ifndef __DPVSIMPVIRTUALPORTAL_HPP
#define __DPVSIMPVIRTUALPORTAL_HPP
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
 * Description: 	Physical portal implementation class
 *
 * $Archive: /dpvs/implementation/include/dpvsImpVirtualPortal.hpp $
 * $Author: wili $ 
 * $Revision: #1 $
 * $Modtime: 31.05.01 14:03 $
 * $Date: 2003/01/22 $
 * 
 ******************************************************************************/

#if !defined (__DPVSIMPPHYSICALPORTAL_HPP)
#	include "dpvsImpPhysicalPortal.hpp"
#endif

namespace DPVS
{
/******************************************************************************
 *
 * Class:			DPVS::ImpVirtualPortal
 *
 * Description:		Implementation of VirtualPortal
 *
 *****************************************************************************/

class ImpVirtualPortal : public ImpPhysicalPortal
{
public:
						ImpVirtualPortal	(Model* testModel, Cell* targetCell,PhysicalPortal *p);
	virtual				~ImpVirtualPortal	(void);

	void				setTargetPortal		(PhysicalPortal *s);
	void				setWarpMatrix		(const Matrix4x3& mtx);

	PhysicalPortal*		getTargetPortal		(void) const					{ return m_targetPortal;}
	const Matrix4x3&	getWarpMatrix		(void) const					{ DPVS_ASSERT(m_warpMatrix); return *m_warpMatrix;	}

	virtual bool		calculateTransition		(ImpPhysicalPortal*& s,ImpPhysicalPortal*& d, Matrix4x3& m, ImpCell* targetCell);

private:
						ImpVirtualPortal	(const ImpVirtualPortal&);
	ImpVirtualPortal&	operator=			(const ImpVirtualPortal&);

	PhysicalPortal*		m_targetPortal;		// soft reference
	Matrix4x3*			m_warpMatrix;		// warp matrix
};

} // DPVS

//------------------------------------------------------------------------
#endif // __DPVSIMPVIRTUALPORTAL_HPP
