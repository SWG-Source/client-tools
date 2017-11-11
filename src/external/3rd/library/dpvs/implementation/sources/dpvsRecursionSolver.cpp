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
 * Description: 	Recursion Solver code
 *
 * $Archive: /dpvs/implementation/sources/dpvsRecursionSolver.cpp $
 * $Author: wili $ 
 * $Revision: #1 $
 * $Modtime: 9.05.01 17:07 $
 * $Date: 2003/01/22 $
 * 
 ******************************************************************************/

#include "dpvsRecursionSolver.hpp"
#include "dpvsHash.hpp"

#if defined(DPVS_OUTPUT_PORTAL_TRAVERSAL)
#pragma warning(push,1)
#	include <iostream>
#pragma warning(pop)
#endif //DPVS_OUTPUT_PORTAL_TRAVERSAL


namespace DPVS
{
	template <> unsigned int HashKey<RecursionSolver::Transition>::getHashValue (const RecursionSolver::Transition& s)	
	{ 
		return (unsigned int)((signed int)(s.m_s) + (signed int)(s.m_d)*1937);	// ad-hoc hash function
	}
} // DPVS

using namespace DPVS;

/*****************************************************************************
 *
 * Function:		DPVS::RecursionSolver::clear()
 *
 * Description:		Initializes all data
 *
 *****************************************************************************/

void RecursionSolver::clear	(void)
{
	m_active->removeAll();
	m_activeCount			= 0;

	m_maximumDepth			= 1;

	m_traversal.reset();
	m_transitionListCount	= 1;		// the first is not allowed!
}

/*****************************************************************************
 *
 * Function:		DPVS::RecursionSolver::RecursionSolver()
 *
 * Description:		Constructor
 *
 *****************************************************************************/

RecursionSolver::RecursionSolver() :
	m_maximumDepth(0),
	m_activeCount(0),
	m_cellTransitions(0),
	m_active(NEW< Hash<Transition,int> >()),
	m_traversal(),
	m_transitionList(),
	m_transitionListCount(1)
{
	clear();
}

RecursionSolver::~RecursionSolver ()
{
	DELETE(m_active);
}

/*****************************************************************************
 *
 * Function:		DPVS::RecursionSolver::setMaximumDepth()
 *
 * Description:		Sets allowed recursion limit.
 *
 * Parameters:		d = allowed maximum recursion count
 *
 * Notes:			Recursion count could be set to each transition separately
 *					but it would seem there is no reason to do so.
 *
 *****************************************************************************/

void RecursionSolver::setMaximumDepth(int d)		
{ 
	m_maximumDepth = d; 
}

/*****************************************************************************
 *
 * Function:		DPVS::RecursionSolver::testRecursionDepth()
 *
 * Description:		Tests whether proposed transition is allowed
 *
 * Parameters:		s = source of transition
 *					d = target of transition
 *
 *****************************************************************************/

bool RecursionSolver::testRecursionDepth (ImpPhysicalPortal* s, ImpPhysicalPortal* d) const
{
	Transition t(null,s,d,0);	// only source & destination matter here

	//-----------------------------------------------------
	// test for maximum recursion depth
	//-----------------------------------------------------

	// portal traversal is not allowed at all
	if(m_maximumDepth==0) return false;		// Possible, single line due to coverage analysis!

	int depth=0;							// init to stop compiler from whining
	if (m_active->get(t,depth))				// this version performs only a single hashing
	{
		if (depth >= m_maximumDepth)
			return false;
	}

	return true;
}


/*****************************************************************************
 *
 * Function:		DPVS::RecursionSolver::testReTraversal()
 *
 * Description:		Tests whether proposed transition is allowed
 *
 * Parameters:		 *
 *****************************************************************************/

bool RecursionSolver::testReTraversal (ImpCell* targetCell,UINT32 flags) const
{
	//-----------------------------------------------------
	// - test for cell re-traversal condition (for nested cells with real portals)
	//-----------------------------------------------------

	if(!(flags & REAL_TRANSITION))
		return true;

	const DynamicArray<CellTransition>&	 cellTransitions = m_cellTransitions;	// most recent is the first one

	for(int i=0;i<m_activeCount;i++)
	{
		if(!(cellTransitions[i].flags & REAL_TRANSITION))			// virtual portal -> accept
			return true;

		if(cellTransitions[i].cell == targetCell)					// real portal path and retraversal -> reject
			return false;
	}

	return true;
}

/*****************************************************************************
 *
 * Function:		DPVS::RecursionSolver::append()
 *
 * Description:		Adds a new transition to recursion list
 *
 * Parameters:		s = source of transition
 *					d = target of transition
 *					f = flags associated to transition (REAL,STENCIL,etc)
 *
 * Notes:			Transition doesn't need to be added to "m_active" hash
 *					since a transition can only occur once per traversed cell.
 *					Therefore there is no need to update m_active list until
 *					the next time the function is processed.
 *
 *****************************************************************************/

void RecursionSolver::append (ImpPhysicalPortal* s, ImpPhysicalPortal* d, ImpCell *c, UINT32 f)
{
	Transition& t = m_transitionList.getElement(m_transitionListCount);
	t.set(c,s,d,f);
	m_traversal.append(m_transitionListCount);

	m_transitionListCount++;
}

/*****************************************************************************
 *
 * Function:		DPVS::RecursionSolver::appendNull()
 *
 * Description:		Adds a level terminator for traversal transitions
 *
 *****************************************************************************/

void RecursionSolver::appendNull(void)
{
	m_traversal.append(0);
}

/*****************************************************************************
 *
 * Function:		DPVS::RecursionSolver::traverse()
 *
 * Description:		Steps to the next transition in the list
 *
 * Notes:			Builds active transition list with depth values for each
 *					transition. Uses parent indices.
 *
 *****************************************************************************/

void RecursionSolver::traverse (void)
{
	m_traversal.traverse();
	int current = m_traversal.getCurrentIndex();

	//-----------------------------------------------------
	// contruct hash of transitions along with their recursion depths
	//-----------------------------------------------------

	const DynamicArray<Transition>& transitionList = m_transitionList;

	m_active->removeAll();
	m_activeCount = 0;

	int	pos = current;

	DPVS_OUTPUT( std::cout << "Recursion Tree: " << "[" << current << "] " );

	while(pos>=0)
	{
		m_traversal.setCurrentIndex(pos);

		int index = m_traversal.getCurrent();
		const Transition& t = transitionList[index];

		if(t.getSource() != getInvalidPointer())
		{
			DPVS_OUTPUT( std::cout << "{" << index << "}" << "(" << t.getSource()->getName() << "->" << t.getTarget()->getName() << ") ");

			if(m_active->exists(t))
			{
				int depth = m_active->get(t);
				m_active->remove(t);
				m_active->insert(t,depth+1);
			}
			else
			{
				m_active->insert(t,1);
			}
		}
		else
		{
			DPVS_OUTPUT( std::cout << "(Root Node)" );
		}

		CellTransition& transition = m_cellTransitions.getElement(m_activeCount);
		// build cell transition list
		transition.cell  = t.getCell ();
		transition.flags = t.getFlags();

		m_activeCount++;

		pos = m_traversal.getParentIndex();
	}

	DPVS_OUTPUT( std::cout << std::endl );
	m_traversal.setCurrentIndex(current);
}

//------------------------------------------------------------------------
