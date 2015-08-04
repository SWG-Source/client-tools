#ifndef __DPVSIMPCELL_HPP
#define __DPVSIMPCELL_HPP
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
 * Description: 	Cell implementation class
 *
 * $Archive: /dpvs/implementation/include/dpvsImpCell.hpp $
 * $Author: wili $ 
 * $Revision: #1 $
 * $Modtime: 2.10.02 13:28 $
 * $Date: 2003/01/22 $
 * 
 ******************************************************************************/

#if !defined (__DPVSPRIVATEDEFS_HPP)
#	include "dpvsPrivateDefs.hpp"
#endif
#if !defined (__DPVSCELL_HPP)
#	include "dpvsCell.hpp"
#endif
#if !defined (__DPVSMATRIX_HPP)
#	include "dpvsMatrix.hpp"
#endif
#if !defined (__DPVSSET_HPP)
#	include "dpvsSet.hpp"
#endif

/******************************************************************************
 *
 * Class:			DPVS::ImpCell
 *
 * Description:		Implementation of Cell
 *
 *****************************************************************************/

namespace DPVS
{
class IDatabase;
class Object;
class ImpObject;
class Camera;
class ImpPhysicalPortal;

/******************************************************************************
 *
 * Class:			DPVS::ImpCellDirtyObjectList
 *
 * Description:		A tiny class used for keeping the "dirty objects" of
 *                  a cell. It has been made a separate class so that we
 *                  can have access to it only from ImpCell and ImpObject
 *					classes.
 *
 *****************************************************************************/

class ImpCellDirtyObjectList
{
protected:

	friend class ImpObject;

							ImpCellDirtyObjectList  (void) : m_dirtyHead(null), m_dirtyTail(null) {}
	virtual					~ImpCellDirtyObjectList (void) { DPVS_ASSERT(!m_dirtyHead); DPVS_ASSERT(!m_dirtyTail); }

	ImpObject*				m_dirtyHead;			// head of dirty object linked list
	ImpObject*				m_dirtyTail;			// tail of dirty object linked list
private:
							ImpCellDirtyObjectList	(const ImpCellDirtyObjectList&);	// not allowed
	ImpCellDirtyObjectList&	operator=				(const ImpCellDirtyObjectList&);	// not allowed

};


/******************************************************************************
 *
 * Class:			DPVS::ImpCell
 *
 * Description:		Implementation of the Cell class
 *
 *****************************************************************************/

class ImpCell : public ImpCellDirtyObjectList
{
private:
							ImpCell				(const ImpCell&);	// not allowed
	ImpCell&				operator=			(const ImpCell&);	// not allowed

	ImpCell*				m_prev;					// previous cell in world
	ImpCell*				m_next;					// next cell in world
	ImpObject*				m_firstObject;			// first object in cell

	IDatabase*				m_database;				// pointer to database object
	Set<Camera*>			m_cameras;				// cameras in the cell
	Set<ImpPhysicalPortal*>	m_interestedPortals;	// interested portals
	Matrix4x3				m_cellToWorld;			// cell->world matrix
	Matrix4x3				m_worldToCell;			// world->cell matrix

	bool					m_enabled:1;			// is the cell enabled?
	bool					m_reportImmediately:1;	// is the cell reporting immediately?

	static ImpCell*			s_head;					// linked list of ImpCells
public:
	static void				minimizeMemoryUsage		(void);		// minimizes memory usage of all cells
	static void				checkConsistencyAll		(void);

							ImpCell					(void);
	virtual					~ImpCell				(void);		// DEBUG DEBUG TODO: why is this virtual?
	void					addCamera				(Camera*);
	void					addInterestedPortal		(ImpPhysicalPortal* p);
	void					addObject				(Object*);
	void					enableObject			(Object*, bool enable);
	const Matrix4x3&		getCellToWorld			(void) const			{ return m_cellToWorld; }
	class IDatabase*		getDatabase				(void) const			{ return m_database; }
	Cell*					getUserCell				(void) const			{ return (reinterpret_cast<Cell*>(const_cast<ImpCell*>(this)))-1; }
	const Matrix4x3&		getWorldToCell			(void) const			{ return m_worldToCell; }
	void					removeCamera			(Camera*);
	void					removeInterestedPortal	(ImpPhysicalPortal* p);
	void					removeObject			(Object*);
	bool					reportImmediately		(void) const			{ return m_reportImmediately; }
	void					set						(Cell::Property,bool);
	void					setCellToWorld			(const Matrix4x3&);
	bool					test					(Cell::Property);
};

DPVS_FORCE_INLINE ImpCell* Cell::getImplementation (void) const						
{ 
	return reinterpret_cast<ImpCell*>(const_cast<Cell*>(this)+1); // yeah baby
}

} //namespace DPVS

//------------------------------------------------------------------------
#endif //__DPVSIMPCELL_HPP
