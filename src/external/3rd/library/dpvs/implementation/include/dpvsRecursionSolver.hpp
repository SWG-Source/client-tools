#ifndef __DPVSRECURSIONSOLVER_HPP
#define __DPVSRECURSIONSOLVER_HPP
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
 * Description: 	Recursion solver class
 *
 * Notes:
 *
 * $Archive: /dpvs/implementation/include/dpvsRecursionSolver.hpp $
 * $Author: wili $ 
 * $Revision: #1 $
 * $Modtime: 7.05.01 12:06 $
 * $Date: 2003/01/22 $
 * 
 ******************************************************************************/

#if !defined (__DPVSARRAY_HPP)
#	include "dpvsArray.hpp"
#endif

#include "dpvsXFirstTraversal.hpp"

namespace DPVS
{
template <class K, class V> class Hash;
template <class T> class HashKey;
class ImpObject;
class ImpPhysicalPortal;
class ImpCell;

/******************************************************************************
 *
 * Class:			DPVS::RecursionSolver
 *
 * Description:		Tracks all possible recursion cases
 *
 *****************************************************************************/
	
class RecursionSolver
{
public:
	enum
	{
		REAL_TRANSITION		= (1<<0),
		USES_STENCIL		= (1<<1)
	};

			RecursionSolver		(void);
			~RecursionSolver	(void);
	void	setMaximumDepth		(int);
	void	clear				(void);
	bool	testRecursionDepth	(ImpPhysicalPortal*,ImpPhysicalPortal*) const;
	bool	testReTraversal		(ImpCell* targetCell,UINT32 flags) const;

	void	appendNull			(void);
	void	append				(ImpPhysicalPortal*,ImpPhysicalPortal*,ImpCell *c,UINT32 flags);	// add transition to the list
	void	traverse			(void);			// steps to the next transition //builds active transition list with depth values for each transition. //	-> uses parent indices

	class Transition
	{
	public:
				Transition	() : m_c(0), m_s(0), m_d(0), m_f(0)											{ }
				Transition  (const Transition& s) : m_c(s.m_c), m_s(s.m_s), m_d(s.m_d), m_f(s.m_f)	{ }
				Transition	(ImpCell* c,ImpPhysicalPortal *s,ImpPhysicalPortal *d, UINT32 flags) : m_c(c), m_s(s), m_d(d), m_f(flags)	{ }

		void				set			(ImpCell* c,ImpPhysicalPortal *s,ImpPhysicalPortal *d, UINT32 flags)	{ m_c=c; m_s=s; m_d=d; m_f=flags; }

		ImpCell*			getCell		(void) const				{ return m_c;		}
		ImpPhysicalPortal*	getSource	(void) const				{ return m_s;		}
		ImpPhysicalPortal*	getTarget	(void) const				{ return m_d;		}
		UINT32				getFlags	(void) const				{ return m_f;	}

		bool	    operator==	(const Transition &t) const			{ return m_s==t.m_s && m_d==t.m_d; }
		Transition& operator=	(const Transition& s)				{ m_c = s.m_c; m_s = s.m_s; m_d = s.m_d; m_f=s.m_f; return *this; }

	private:
		friend class HashKey<Transition>;
		ImpCell*			m_c;
		ImpPhysicalPortal*	m_s;
		ImpPhysicalPortal*	m_d;
		UINT32				m_f;
	};

	struct CellTransition
	{
		void*	cell;
		UINT32	flags;
	};

private:
						RecursionSolver		(const RecursionSolver&);	// not allowed
	RecursionSolver&	operator=			(const RecursionSolver&);	// not allowed

	int									m_maximumDepth;				// maximum recursion depth
	int									m_activeCount;
	DynamicArray<CellTransition>		m_cellTransitions;
	Hash<Transition,int>*				m_active;					// active path

#if defined(DPVS_DEPTH_FIRST)
	DepthFirstTraversal<int>			m_traversal;				// indices to m_transitionList
#else
	BreadthFirstTraversal<int>			m_traversal;
#endif
	DynamicArray<Transition>			m_transitionList;
	int									m_transitionListCount;
};


} // DPVS

//------------------------------------------------------------------------
#endif //__DPVSRECURSIONSOLVER_HPP
