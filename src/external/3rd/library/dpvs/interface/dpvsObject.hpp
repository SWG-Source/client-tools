#ifndef __DPVSOBJECT_HPP
#define __DPVSOBJECT_HPP
/*****************************************************************************
 *
 * dPVS
 * -----------------------------------------
 *
 * (C) 1999-2004 Hybrid Graphics, Ltd.
 * All Rights Reserved.
 *
 * Dynamic PVS and dPVS are trademarks of Criterion Software Ltd.
 *
 * This file consists of unpublished, proprietary source code of
 * Hybrid Graphics, and is considered Confidential Information for
 * purposes of non-disclosure agreement. Disclosure outside the terms
 * outlined in signed agreement may result in irrepairable harm to
 * Hybrid Graphics and legal action against the party in breach.
 *
 * Description:     Objects, Portals and Regions of Influence
 *
 * $Id: //depot/products/dpvs/interface/dpvsObject.hpp#7 $
 * $Date: 2004/11/12 $
 * $Author: wili $
 * 
 ******************************************************************************/

#if !defined (__DPVSREFERENCECOUNT_HPP)
#   include "dpvsReferenceCount.hpp"
#endif

namespace DPVS
{
class Cell;
class Model;

/******************************************************************************
 *
 * Class:           DPVS::Object
 *
 * Description:     Physical instance of a model
 *
 * Notes:           Objects are used to place models into the world. For
 *                  each object separate test/write models can be defined.
 *
 *****************************************************************************/

class Object : public ReferenceCount
{
public:
    enum Property
    {
        ENABLED                 = 0,        // is the object active (all objects)
        INFORM_VISIBLE          = 1,        // should the user be informed when object becomes visible (all objects)
        CONTRIBUTION_CULLING    = 2,        // is contribution culling allowed (all objects)
        INFORM_PORTAL_ENTER     = 3,        // inform when traversing through the portal (portal objects only)
        INFORM_PORTAL_EXIT      = 4,        // inform when traversing backwards through the portal (portal objects only)
        FLOATING_PORTAL         = 5,        // object is a floating portal (portal objects only)
		REPORT_IMMEDIATELY		= 6,		// report visibility immediately
		UNBOUNDED				= 7,		// object test model ignored, object regarded to be of infinite size and reported as always visible
		OCCLUDER				= 8,		// can object ever be used as an occluder? (enabled by default)
        INFORM_PORTAL_PRE_EXIT  = 9         // inform when about to traverse backwards through the portal (portal objects only)
    };

static DPVSDEC Object*	create					(Model* testModel);
DPVSDEC void			getAABB					(Vector3& mn,Vector3& mx) const;
DPVSDEC Cell*			getCell                 (void) const;
DPVSDEC void			getOBB					(Matrix4x4&) const;
DPVSDEC void			getObjectToCellMatrix   (Matrix4x4&) const;
DPVSDEC void			getObjectToCellMatrix   (Matrix4x4d&) const;
DPVSDEC void			getSphere				(Vector3& center, float& radius) const;
DPVSDEC Model*			getTestModel            (void) const;
DPVSDEC Object*			getVisibilityParent     (void) const;
DPVSDEC Model*			getWriteModel           (void) const;
DPVSDEC void			set                     (Property, bool);
DPVSDEC void			setCell                 (Cell*);
DPVSDEC void			setCost                 (int nVertices, int nTriangles, float complexity);
DPVSDEC void			setObjectToCellMatrix   (const Matrix4x4&);
DPVSDEC void			setObjectToCellMatrix   (const Matrix4x4d&);
DPVSDEC void			setTestModel            (Model*);
DPVSDEC void			setVisibilityParent     (Object*);
DPVSDEC void			setWriteModel           (Model*);
DPVSDEC bool			test                    (Property) const;

class ImpObject*        getImplementation       (void) const;               // internal
protected:
						Object					(class ImpReferenceCount*);
	virtual void		destruct				(void) const;
private:
                        Object                  (const Object&);            // not allowed
    Object&             operator=               (const Object&);            // not allowed
};

/******************************************************************************
 *
 * Class:           DPVS::RegionOfInfluence
 *
 * Description:     Region Of Influence
 *
 * Notes:           Regions Of Influence (or ROIs for short) are used
 *                  to place light sources and similar objects into the
 *                  scene. The camera's visibility resolving process will
 *                  determine overlaps between ROIs and other objects.
 *
 *                  The actual region is defined by assigning a test model
 *                  to the ROI. In order to model a spot light, a mesh model
 *                  of the corresponding shape can be used.
 *
 *                  ROIs are tested during the visibility query in a similar
 *                  fashion to other objects. This means that if a ROI is
 *                  fully outside the view frustum or fully occluded, it will
 *                  be rejected prior to the ROI vs. object overlap tests.
 *
 *****************************************************************************/

class RegionOfInfluence : public Object
{
public:
static DPVSDEC RegionOfInfluence*	create					(Model* testModel);
protected:
									RegionOfInfluence		(class ImpReferenceCount*);
private:
									RegionOfInfluence       (const RegionOfInfluence&); // not allowed
    RegionOfInfluence&				operator=               (const RegionOfInfluence&); // not allowed
};

/******************************************************************************
 *
 * Class:           DPVS::PhysicalPortal
 * 
 * Description:     Class describing a physical link between two cells
 *
 * Notes:           Physical portals are physical links between cells. Such links
 *                  cannot cause discontinuities in space and are only used for
 *                  visibility determination.
 *
 *****************************************************************************/
    
class PhysicalPortal : public Object
{
public:
static DPVSDEC PhysicalPortal*		create					(Model* testModel, Cell* targetCell);
DPVSDEC float						getImportanceDecay      (void) const;
DPVSDEC Model*						getStencilModel         (void) const;
DPVSDEC Cell*						getTargetCell           (void) const;
DPVSDEC void						setImportanceDecay      (float);
DPVSDEC void						setStencilModel         (Model*);
DPVSDEC void						setTargetCell           (Cell*);
protected:
									PhysicalPortal			(class ImpReferenceCount*);
private:
									PhysicalPortal          (const PhysicalPortal&);    // not allowed
    PhysicalPortal&					operator=               (const PhysicalPortal&);    // not allowed
};

/******************************************************************************
 *
 * Class:           DPVS::VirtualPortal
 *
 * Description:     Class describing an arbitrary link between two cells
 *
 * Notes:           Virtual portals involve free transformation between source
 *                  and target cell. Special effects such as mirrors,
 *                  surveillance cameras, teleports etc. can be created with
 *                  VirtualPortals.
 *
 *                  There are some limitations in using virtual portals.
 *                  Consult User's Manual for detailed description.
 *
 *****************************************************************************/

class VirtualPortal : public PhysicalPortal
{
public:
static DPVSDEC VirtualPortal*	create					(Model* testModel, PhysicalPortal* targetPortal);
DPVSDEC PhysicalPortal*			getTargetPortal         (void) const;
DPVSDEC void					getWarpMatrix           (Matrix4x4&) const;
DPVSDEC void					getWarpMatrix           (Matrix4x4d&) const;
DPVSDEC void					setTargetPortal         (PhysicalPortal*);
DPVSDEC void					setWarpMatrix           (const Matrix4x4&);
DPVSDEC void					setWarpMatrix           (const Matrix4x4d&);
protected:
								VirtualPortal			(class ImpReferenceCount*);
private:
								VirtualPortal           (const VirtualPortal&);     // not allowed
    VirtualPortal&				operator=               (const VirtualPortal&);     // not allowed
};

} // DPVS

//------------------------------------------------------------------------
#endif // __DPVSOBJECT_HPP
