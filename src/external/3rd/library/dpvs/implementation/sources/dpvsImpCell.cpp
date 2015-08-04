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
 * Description: 	Cell code
 *
 * $Archive: /dpvs/implementation/sources/dpvsImpCell.cpp $
 * $Author: wili $ 
 * $Revision: #1 $
 * $Modtime: 2.10.02 14:10 $
 * $Date: 2003/01/22 $
 * 
 ******************************************************************************/

#include "dpvsImpCell.hpp"			
#include "dpvsImpObject.hpp"
#include "dpvsCamera.hpp"
#include "dpvsDatabase.hpp"
#include "dpvsInstanceCount.hpp"
#include "dpvsImpPhysicalPortal.hpp"
#include "dpvsMemory.hpp"

//#include <cstdio>

using namespace DPVS;


//------------------------------------------------------------------------
// Static data
//------------------------------------------------------------------------

ImpCell* ImpCell::s_head = null;			// linked list

/*****************************************************************************
 *
 * Function:		ImpCell::ImpCell()
 *
 * Description:		Constructor
 *
 *****************************************************************************/

ImpCell::ImpCell(void) :
	m_prev(null),
	m_next(null),
	m_firstObject(null),
	m_database(NEW<IDatabase>()),
	m_cellToWorld(),
	m_worldToCell(),
	m_enabled(true),
	m_reportImmediately(false)
{
	InstanceCount::incInstanceCount(InstanceCount::CELL);		// increase instance count
	if (s_head)													// add to linked list of cells
		s_head->m_prev = this;
	m_next = s_head;
	s_head = this;
}

/*****************************************************************************
 *
 * Function:		ImpCell::minimizeMemoryUsage()
 *
 * Description:		Iterates through all cells and minimizes their memory
 *					usage
 *
 *****************************************************************************/

void ImpCell::minimizeMemoryUsage (void)
{
	for (ImpCell* c = s_head; c; c = c->m_next)
	{
		IDatabase*	db = c->getDatabase();
		DPVS_ASSERT(db);
		db->optimize (IDatabase::OPTIMIZE_MEMORY_USAGE);
	}
}
	
/*****************************************************************************
 *
 * Function:		ImpCell::~ImpCell()
 *
 * Description:		Destructor
 *
 *****************************************************************************/

ImpCell::~ImpCell (void)
{
	//--------------------------------------------------------------------
	// Inform interested portals (clear target cell, which is now being deleted)
	//--------------------------------------------------------------------


	{
		Set<ImpPhysicalPortal*>::Array tgt(m_interestedPortals);
		for(int i=0;i<tgt.getSize();i++)
			tgt[i]->setTargetCell(null);			// causes call to this->removeInterestedPortal() (SAFE)
	}
	
	//--------------------------------------------------------------------
	// Remove cameras from the cell
	//--------------------------------------------------------------------

	{
		Set<Camera*>::Array camArray(m_cameras);	// construct array from camera set
		for (int i = 0; i < camArray.getSize(); i++)
		{
			camArray[i]->setCell(null);				// set cell to null and remove from the cell
			camArray[i]->addReference();			// take and give up a reference -> deletes autoreleased objects	DEBUG DEBUG REF
			camArray[i]->release();
		}

		DPVS_ASSERT(m_cameras.isEmpty());
	}

	//--------------------------------------------------------------------
	// Remove objects
	//--------------------------------------------------------------------

	{
		while (m_firstObject)
		{
			Object* o = m_firstObject->getUserObject();
			m_firstObject->setCell(null);			// remove it from this cell (this updates m_firstObject)
			o->addReference();						// take and give up a reference -> deletes autoreleased objects
			o->release();
		}
	}

	//--------------------------------------------------------------------
	// Kill the database
	//--------------------------------------------------------------------

	DELETE(m_database);
	m_database = null;

	//--------------------------------------------------------------------
	// Remove cell from linked list of cells
	//--------------------------------------------------------------------

	if (m_prev) 
		m_prev->m_next = m_next;
	else
	{
		DPVS_ASSERT(this == s_head);
		s_head = m_next;
	}
	if (m_next) 
		m_next->m_prev = m_prev;

	InstanceCount::decInstanceCount(InstanceCount::CELL);

#if defined (DPVS_DEBUG)

	if (InstanceCount::getInstanceCount(InstanceCount::CELL)<=0)			// last one removed..
		DPVS_ASSERT(!s_head);
	if (!s_head)
		DPVS_ASSERT(InstanceCount::getInstanceCount(InstanceCount::CELL)==0);
#endif
}

/*****************************************************************************
 *
 * Function:		ImpCell::getCellToWorld()
 *
 * Description:		
 *
 * Parameters:
 *
 *****************************************************************************/
/*****************************************************************************
 *
 * Function:		ImpCell::getWorldToCell()
 *
 * Description:		
 *
 * Parameters:
 *
 *****************************************************************************/

/*****************************************************************************
 *
 * Function:		ImpCell::setCellToWorld(const Matrix4x3&)
 *
 * Description:		
 *
 * Parameters:
 *
 *****************************************************************************/

void ImpCell::setCellToWorld(const Matrix4x3& mtx)
{
	DPVS_API_ASSERT(Math::isInvertible(mtx) && "Cell::setCellToWorld() - matrix contains zero scale terms");
	m_cellToWorld = mtx;
	Math::invertMatrix(m_worldToCell,m_cellToWorld);
}

/*****************************************************************************
 *
 * Function:		ImpCell::addObject(Object*,bool)
 *
 * Description:		Adds object to cell
 *
 * Parameters:		obj		= pointer to object
 *
 *****************************************************************************/

void ImpCell::addObject(Object* obj)
{
	DPVS_ASSERT(obj);
	ImpObject* imp = obj->getImplementation();

	DPVS_ASSERT(imp->m_cellPrev == null);
	DPVS_ASSERT(imp->m_cellNext == null);

	imp->m_cellPrev = null;
	imp->m_cellNext = m_firstObject;
	if (imp->m_cellNext)
		imp->m_cellNext->m_cellPrev = imp;
	m_firstObject = imp;
}

/*****************************************************************************
 *
 * Function:		ImpCell::enableObject(Object*,bool)
 *
 * Description:		Enables/disables object in database (object *MUST* be already added into the cell)
 *
 * Parameters:		obj		= pointer to object
 *					enable	= boolean indicating whether object should be enabled
 *
 *****************************************************************************/

void ImpCell::enableObject (Object* obj, bool enable)
{
	ImpObject* imp = obj->getImplementation();		
	m_database->removeObject (imp);								// remove from database
	if (enable)
		m_database->addObject (imp);							// add to database?
}

/*****************************************************************************
 *
 * Function:		ImpCell::removeObject(Object*)
 *
 * Description:		Removes object from cell
 *
 * Parameters:		obj = pointer to object
 *
 *****************************************************************************/

void ImpCell::removeObject(Object* obj)
{
	DPVS_ASSERT(obj && m_database);

	ImpObject* imp = obj->getImplementation();		

	if (imp->m_cellPrev)
		imp->m_cellPrev->m_cellNext = imp->m_cellNext;
	else
	{
		DPVS_ASSERT(m_firstObject == imp);
		m_firstObject = imp->m_cellNext;
	}
	
	if (imp->m_cellNext)
		imp->m_cellNext->m_cellPrev = imp->m_cellPrev;

	imp->m_cellPrev = null;
	imp->m_cellNext = null;

	m_database->removeObject (imp);						
}

/*****************************************************************************
 *
 * Function:		ImpCell::addCamera(Camera*)
 *
 * Description:		Adds camera to cell (if it's not already there)
 *
 * Parameters:		cam		= pointer to camera
 *
 *****************************************************************************/

void ImpCell::addCamera(Camera* cam)
{
	DPVS_ASSERT(cam);

	if (!m_cameras.contains(cam))
		m_cameras.insert(cam);
}

/*****************************************************************************
 *
 * Function:		ImpCell::removeCamera(Camera*)
 *
 * Description:		
 *
 * Parameters:		cam = pointer to camera
 *
 *****************************************************************************/

void ImpCell::removeCamera(Camera* cam)
{
	DPVS_ASSERT(cam);

	m_cameras.remove(cam);								// remove from set
}

/*****************************************************************************
 *
 * Function:		ImpCell::set()
 *
 * Description:		
 *
 * Parameters:		p = property
 *					v = boolean value (true = set property, false = remove property)
 *
 *****************************************************************************/

void ImpCell::set (Cell::Property p,bool v)
{
	switch(p)
	{
		case Cell::ENABLED:				m_enabled			= v; break;
		case Cell::REPORT_IMMEDIATELY:	m_reportImmediately = v; break;

		default:
			DPVS_ASSERT(!"Unknown property in ImpCell::set(Cell::Property p,bool v)");
			break;
	}
}

/*****************************************************************************
 *
 * Function:		ImpCell::test()
 *
 * Description:		
 *
 * Parameters:
 *
 *****************************************************************************/

bool ImpCell::test	(Cell::Property p)
{
	switch(p)
	{
		case Cell::ENABLED:				return m_enabled;
		case Cell::REPORT_IMMEDIATELY:	return m_reportImmediately;
	}

	DPVS_ASSERT(!"Unknown property in ImpCell::set(Cell::Property p,bool v)");
	return false;
}

/*****************************************************************************
 *
 * Function:		ImpCell::addInterestedPortal()
 *
 * Description:		
 *
 * Parameters:		p = pointer to portal
 *
 *****************************************************************************/

void ImpCell::addInterestedPortal (ImpPhysicalPortal* p)			
{ 
	m_interestedPortals += p;							// add to set
}

/*****************************************************************************
 *
 * Function:		ImpCell::removeInterestedPortal()
 *
 * Description:		
 *
 * Parameters:		p = pointer to portal
 *
 *****************************************************************************/

void ImpCell::removeInterestedPortal (ImpPhysicalPortal* p)			
{ 
	DPVS_ASSERT(m_interestedPortals.contains(p)); 
	m_interestedPortals -= p;							// remove from set
}

/*****************************************************************************
 *
 * Function:		ImpCell::checkConsistencyAll();
 *
 * Description:		Checks consistency of all ImpCells in the world (debug
 *					build only)
 *
 *****************************************************************************/

void ImpCell::checkConsistencyAll (void)
{
#if defined (DPVS_DEBUG)
	int cnt = 0;
	for (ImpCell* c = s_head; c; c = c->m_next, cnt++)
	{
		DPVS_ASSERT(Memory::isValidPointer(c->getUserCell()));
		DPVS_ASSERT(Memory::isValidPointer(c->m_database));

		// make sure Database is a-ok
		c->m_database->checkConsistency();

		// make sure all objects in the cell are ok
		for (ImpObject* o = c->m_firstObject; o; o = o->m_cellNext)
		{
			o->checkConsistency();
			DPVS_ASSERT (o->getCell() == c->getUserCell());
		}
	}

	DPVS_ASSERT(InstanceCount::getInstanceCount(InstanceCount::CELL) == cnt);

	// TODO: check camera consistency etc.??
#endif
}
//------------------------------------------------------------------------
