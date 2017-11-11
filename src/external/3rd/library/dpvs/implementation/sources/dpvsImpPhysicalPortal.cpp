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
 * Description:		Physical Portal code
 *
 * $Archive: /dpvs/implementation/sources/dpvsImpPhysicalPortal.cpp $
 * $Author: wili $ 
 * $Revision: #1 $
 * $Modtime: 23.10.01 15:37 $
 * $Date: 2003/01/22 $
 * 
 ******************************************************************************/

#include "dpvsImpPhysicalPortal.hpp"
#include "dpvsImpVirtualPortal.hpp"
#include "dpvsImpCell.hpp"
#include "dpvsImpCommander.hpp"
#include "dpvsSet.hpp"
#include "dpvsVQData.hpp"

using namespace DPVS;

UINT32 ImpPhysicalPortal::s_hashCounter = 0;

/*****************************************************************************
 *
 * Function:		ImpPhysicalPortal::addInterestedPortal()
 *
 * Description:		
 *
 * Parameters:		p = pointer to virtual portal
 *
 *****************************************************************************/

void ImpPhysicalPortal::addInterestedPortal		(ImpVirtualPortal* p)			
{
	DPVS_ASSERT(m_interestedPortals);
	*m_interestedPortals += p;		// add to set
}

/*****************************************************************************
 *
 * Function:		ImpPhysicalPortal::removeInterestedPortal()
 *
 * Description:		
 *
 * Parameters:		p = pointer to virtual portal
 *
 *****************************************************************************/

void ImpPhysicalPortal::removeInterestedPortal	(ImpVirtualPortal* p)			
{
	DPVS_ASSERT(m_interestedPortals && m_interestedPortals->contains(p)); 
	*m_interestedPortals -= p; // remove from
}

/*****************************************************************************
 *
 * Function:		ImpPhysicalPortal::ImpPhysicalPortal()
 *
 * Description:		Constructor
 *
 *****************************************************************************/

ImpPhysicalPortal::ImpPhysicalPortal	(Model* testModel, Cell *targetCell) : 
	ImpObject(testModel),
	m_stencilModel(null),
	m_targetCell(null),
	m_hashValue(s_hashCounter++),
	m_importanceDecay(1.f),
	m_interestedPortals(NEW< Set<ImpVirtualPortal*> >())
{
	setType(TYPE_PHYSICAL_PORTAL);
	setTargetCell(targetCell);
	setRenderCost(PORTAL_BASE_COST);
	InstanceCount::incInstanceCount(InstanceCount::PHYSICALPORTAL);
}

/*****************************************************************************
 *
 * Function:		ImpPhysicalPortal::~ImpPhysicalPortal()
 *
 * Description:		Destructor
 *
 *****************************************************************************/

ImpPhysicalPortal::~ImpPhysicalPortal	(void)
{
	InstanceCount::decInstanceCount(InstanceCount::PHYSICALPORTAL);

	//--------------------------------------------------------------------
	// Remove from "interested" list (CELL)
	//---------------------------------------------------------------
	
	setTargetCell(null);

	Set<ImpVirtualPortal*>::Array tgt(*m_interestedPortals);

	for(int i=0;i<tgt.getSize();i++)
		tgt[i]->setTargetPortal(null);	// causes call to this->removeInterestedPortal() (SAFE)

	DELETE(m_interestedPortals);
}

/*****************************************************************************
 *
 * Function:		ImpPhysicalPortal::setTargetCell()
 *
 * Description:		
 *
 * Parasmeters:		cell = pointer to cell
 *
 *****************************************************************************/

void ImpPhysicalPortal::setTargetCell		(Cell* cell)
{
	//--------------------------------------------------------------------
	// Not interested about the state of the OLD target cell
	//--------------------------------------------------------------------

	if(m_targetCell)
		m_targetCell->getImplementation()->removeInterestedPortal(this);

	//--------------------------------------------------------------------
	// Assign a NEW target cell
	//--------------------------------------------------------------------

	m_targetCell = cell;

	//--------------------------------------------------------------------
	// Very interested about the state of the NEW target cell
	//--------------------------------------------------------------------

	if(m_targetCell)
		m_targetCell->getImplementation()->addInterestedPortal(this);
}

/*****************************************************************************
 *
 * Function:		ImpPhysicalPortal::calculcateTransition()
 *
 * Description:		
 *
 *****************************************************************************/

bool ImpPhysicalPortal::calculateTransition	(ImpPhysicalPortal*& s,ImpPhysicalPortal*& d, Matrix4x3& m, ImpCell* targetCell)
{
	DPVS_ASSERT(VQData::get().testProperties(VQData::FLOW_ACTIVE));
	ImpCamera* c = VQData::get().getCamera();
	DPVS_ASSERT(c);
	DPVS_ASSERT((void*)VQData::get().getObject() == (void*)this);

	// set transition
	s = this;
	d = this;

	// tcameraTotCell = cameraToCell1 * cell1ToWorld * worldTotargetcell
	Math::productFromLeft(m, c->getCameraToCell(), c->getCell()->getImplementation()->getCellToWorld());
	m.productFromLeft(targetCell->getWorldToCell());
	return true;
}

/*****************************************************************************
 *
 * Function:		DPVS::ImpPhysicalPortal::getStencilSilhouette()
 *
 * Description:		Get exact silhouette of stencil model
 *
 * Parameters:
 *
 *****************************************************************************/

bool ImpPhysicalPortal::getStencilSilhouette (EdgeSilhouette& s, Range<float> &depthRange, ImpCamera *c)
{
	Model* sm = getStencilModel();

	if(!sm)
		return false;

	ImpModel* m = sm->getImplementation();
	DPVS_ASSERT(m->isOcclusionWritable());

	if(!m->isOcclusionWritable())
		return false;

	bool retValue = m->getWriteSilhouette(s,depthRange,getObjectToCameraMatrix(c),c);

	// DEBUG DEBUG ARE WE MISSING SOMETHING HERE?
	if(retValue)
	{
	}

	return retValue;
}

//------------------------------------------------------------------------

