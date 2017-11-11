#ifndef __DPVSIMPPHYSICALPORTAL_HPP
#define __DPVSIMPPHYSICALPORTAL_HPP
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
 * $Archive: /dpvs/implementation/include/dpvsImpPhysicalPortal.hpp $
 * $Author: wili $ 
 * $Revision: #1 $
 * $Modtime: 7.05.01 12:06 $
 * $Date: 2003/01/22 $
 * 
 ******************************************************************************/

#if !defined (__DPVSIMPOBJECT_HPP)
#	include "dpvsImpObject.hpp"
#endif
#if !defined (__DPVSPTR_HPP)
#	include "dpvsPtr.hpp"
#endif

namespace DPVS
{
/******************************************************************************
 *
 * Class:			DPVS::ImpPhysicalPortal
 *
 * Description:		Implementation of PhysicalPortal
 *
 *****************************************************************************/

class ImpVirtualPortal;
class ImpPhysicalPortal : public ImpObject
{
public:
						ImpPhysicalPortal		(Model* testModel, Cell *targetCell);
	virtual				~ImpPhysicalPortal		(void);
												
	UINT32				getHashValue			(void) const					{ return m_hashValue;	}
	float				getImportanceDecay		(void) const					{ return m_importanceDecay; }
	Model*				getStencilModel			(void) const					{ return m_stencilModel; }
	Cell*				getTargetCell			(void) const					{ return m_targetCell; }

	void				setImportanceDecay		(float d)						{ m_importanceDecay = d; }
	void				setStencilModel			(Model* model)					{ m_stencilModel = model; }
	void				setTargetCell			(Cell* cell);

	bool				getStencilSilhouette	(EdgeSilhouette &s, Range<float> &depthRange, ImpCamera *c);
	virtual bool		calculateTransition		(ImpPhysicalPortal*& s,ImpPhysicalPortal*& d, Matrix4x3& m, ImpCell* targetCell);

//protected:	// COMPILER?
	void				addInterestedPortal		(ImpVirtualPortal* p);
	void				removeInterestedPortal	(ImpVirtualPortal* p);

private:
						ImpPhysicalPortal	(const ImpPhysicalPortal&);
	ImpPhysicalPortal&	operator=			(const ImpPhysicalPortal&);

	Ptr<Model>				m_stencilModel;			// refcounting pointer to stencil model
	Cell*					m_targetCell;			// pointer to target cell
	UINT32					m_hashValue;			// hash value
	float					m_importanceDecay;
	Set<ImpVirtualPortal*>*	m_interestedPortals;

	static UINT32			s_hashCounter;			// ensure good distribution in hash functions
};

} // DPVS

//------------------------------------------------------------------------
#endif // __DPVSIMPPHYSICALPORTAL_HPP
