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
 * Description:		Virtual Portal Code
 *
 * $Archive: /dpvs/implementation/sources/dpvsImpVirtualPortal.cpp $
 * $Author: wili $ 
 * $Revision: #1 $
 * $Modtime: 31.05.01 14:03 $
 * $Date: 2003/01/22 $
 * 
 ******************************************************************************/

#include "dpvsImpVirtualPortal.hpp"
#include "dpvsInstanceCount.hpp"
#include "dpvsVQData.hpp"

using namespace DPVS;

/*****************************************************************************
 *
 * Function:		DPVS::ImpVirtualPortal::
 *
 * Description:		
 *
 * Parameters:		
 *
 *****************************************************************************/

ImpVirtualPortal::ImpVirtualPortal	(Model* testModel, Cell* targetCell,PhysicalPortal *p) : 
	ImpPhysicalPortal(testModel, targetCell),
	m_targetPortal(null),
	m_warpMatrix(NEW<Matrix4x3>())
{
	setType(TYPE_VIRTUAL_PORTAL);
	setTargetPortal(p);

	InstanceCount::incInstanceCount(InstanceCount::VIRTUALPORTAL);
}


/*****************************************************************************
 *
 * Function:		DPVS::ImpVirtualPortal::
 *
 * Description:		
 *
 * Parameters:		
 *
 *****************************************************************************/

ImpVirtualPortal::~ImpVirtualPortal		(void)
{
	//---------------------------------------------------------------
	// Remove from "interested" list
	//---------------------------------------------------------------

	setTargetPortal(null);
	InstanceCount::decInstanceCount(InstanceCount::VIRTUALPORTAL);
	DELETE(m_warpMatrix);
}

/*****************************************************************************
 *
 * Function:		DPVS::ImpVirtualPortal::
 *
 * Description:		
 *
 * Parameters:		
 *
 *****************************************************************************/

void ImpVirtualPortal::setWarpMatrix (const Matrix4x3& mtx)			
{ 
	DPVS_ASSERT(m_warpMatrix);
	*m_warpMatrix = mtx;	
}

/*****************************************************************************
 *
 * Function:		DPVS::ImpVirtualPortal::
 *
 * Description:		
 *
 * Parameters:		
 *
 *****************************************************************************/

void ImpVirtualPortal::setTargetPortal	(PhysicalPortal *s)
{
	// not interested about the state of the OLD target portal
	//---------------------------------------------------------------
	if(m_targetPortal)
	{
		ImpPhysicalPortal* imp = (ImpPhysicalPortal*)m_targetPortal->getImplementation();	//SAFE
		imp->removeInterestedPortal(this);
	}

	// assign a NEW target portal
	//---------------------------------------------------------------
	m_targetPortal = s;

	// very interested about the state of the NEW target portal
	//---------------------------------------------------------------
	if(m_targetPortal)
	{
		ImpPhysicalPortal* imp = (ImpPhysicalPortal*)m_targetPortal->getImplementation();	//SAFE
		imp->addInterestedPortal(this);
	}
}

/*****************************************************************************
 *
 * Function:		ImpVirtualPortal::calculcateTransition()
 *
 * Description:		
 *
 *****************************************************************************/

bool ImpVirtualPortal::calculateTransition	(ImpPhysicalPortal*& s,ImpPhysicalPortal*& d, Matrix4x3& m, ImpCell*)
{
	DPVS_ASSERT(VQData::get().testProperties(VQData::FLOW_ACTIVE));
	ImpCamera* c = VQData::get().getCamera();
	DPVS_ASSERT(c);
	DPVS_ASSERT((void*)VQData::get().getObject() == (void*)this);

	if(!getTargetPortal()) return false;			// Possible, single line due to coverage analysis!

	ImpPhysicalPortal *tp = static_cast<ImpPhysicalPortal*>( getTargetPortal()->getImplementation() );	//SAFE
	DPVS_ASSERT(tp);

	s = this;
	d = tp;

	//-------------------------------------------------------------------------------------------------------
	// CameraToCell2 = CameraToCell1 * Cell1ToPortal1 * Portal1ToWarp1 * Warp2ToPortal2 * Portal2ToCell2
	//-------------------------------------------------------------------------------------------------------

	Matrix4x3 cellToPortal1(NO_CONSTRUCTOR);
	Matrix4x3 portalToWarp1(NO_CONSTRUCTOR);

	//-----------------------------------------------------------
	//		Warp matrices are defined in object space
	//
	// If virtual portal is connected to a physical portal the
	// warp matrix is identity. This assumption is a must
	// because physical portals do not define warp matices.
	//-----------------------------------------------------------
	
	Math::invertMatrix (cellToPortal1, getObjectToCell());					// CellToPortal1
	Math::invertMatrix (portalToWarp1, getWarpMatrix());					// PortalToWarp1

	Math::productFromLeft(m, c->getCameraToCell(), cellToPortal1);			// CameraToCell1 * Cell1ToPortal1
	m.productFromLeft(portalToWarp1);										// CameraToCell1 * Cell1ToPortal1 * Portal1ToWarp1

	if(tp->getType() == ImpObject::TYPE_VIRTUAL_PORTAL)
	{
		ImpVirtualPortal* vp2 = static_cast<ImpVirtualPortal*>(tp);			// SAFE
		m.productFromLeft(vp2->getWarpMatrix());							// CameraToCell1 * Cell1ToPortal1 * Portal1ToWarp1 * Warp2ToPortal2
	}

	m.productFromLeft(tp->getObjectToCell());								// CameraToCell1 * Cell1ToPortal1 * Portal1ToWarp1 * Warp2ToPortal2 * Portal2ToCell2

	return true;
}

//------------------------------------------------------------------------

