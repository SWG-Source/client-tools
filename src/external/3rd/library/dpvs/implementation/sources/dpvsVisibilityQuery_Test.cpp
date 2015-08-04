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
 * Description: 	VisibilityQuery's visibility test functions
 *
 * $Archive: /dpvs/implementation/sources/dpvsVisibilityQuery_Test.cpp $
 * $Author: wili $ 
 * $Revision: #1 $
 * $Modtime: 21.11.01 15:52 $
 * $Date: 2003/01/22 $
 * 
 ******************************************************************************/

#include "dpvsVisibilityQuery.hpp"
#include "dpvsRandom.hpp"
#include "dpvsImpObject.hpp"
#include "dpvsImpCamera.hpp"
#include "dpvsDebug.hpp"
#include "dpvsWriteQueue.hpp"
#include "dpvsImpVirtualPortal.hpp"
#include "dpvsImpCell.hpp"
#include "dpvsImpCommander.hpp"

using namespace DPVS;

/*****************************************************************************
 *
 * Function:		VisibilityQuery::isObjectVisible()
 *
 * Description:		Performs object visibility (occlusion) query
 *
 * Parameters:		assumeVisible	= boolean indicating whether we assume the object to be visible
 *
 * Returns:			true if object is visible
 *
 *****************************************************************************/

DPVS_FORCE_INLINE bool VisibilityQuery::isObjectVisible_INTERNAL (bool assumeVisible)
{
	ImpObject *o = VQData::get().getObject();
	ImpCamera* c = VQData::get().getCamera();

	//--------------------------------------------------------------------
	// Test if the visibility parent of an object is hidden
	//--------------------------------------------------------------------

	if(o->hasVisibilityParent())	// this is encoded as a single bit (better access coherence)
	{
		DPVS_ASSERT(o->getVisibilityParent());
		if(o->getVisibilityParent()->getHiddenTimeStamp() == c->getTimeStamp())
			return false;
	}

	//--------------------------------------------------------------------
	// Perform back-face culling (if enabled for the object or if the 
	// object is a portal)
	//--------------------------------------------------------------------

	if(o->isBackFaceCullable() || o->isPortal())
	{
		DPVS_PROFILE (Statistics::incStatistic(Library::STAT_DATABASEOBSBACKFACETESTED,1));
		if (!o->backFaceCull(c))
		{
			DPVS_PROFILE (Statistics::incStatistic(Library::STAT_DATABASEOBSBACKFACECULLED,1));
			return false;
		}
	}

	//--------------------------------------------------------------------
	// Check if we're allowed to make an occlusion test. 
	//--------------------------------------------------------------------

	if(!VQData::get().performOcclusionCulling())								// occlusion culling disabled from camera
		return true;

	//--------------------------------------------------------------------
	// Each object that is assumed to be visible has 1:16 chance to
	// change the assumption to false
	//--------------------------------------------------------------------

	if (assumeVisible && !(g_random.getI()&15))
		assumeVisible = false;

	//--------------------------------------------------------------------
	// Clear benefits of a candidate object
	//--------------------------------------------------------------------

	o->initBenefit( s_frameTimeStamp.value() );

	//--------------------------------------------------------------------
	// Perform VPT if assumed visible
	//--------------------------------------------------------------------

#if defined (DPVS_FLUSH_TABOO)
		VQData::get().setProperties(VQData::ALLOW_FLUSH,!assumeVisible);
#else
		VQData::get().setProperties(VQData::ALLOW_FLUSH,true);
#endif //DPVS_FLUSH_TABOO

#if defined (DPVS_VISIBLE_POINT_TRACKING)
	if(assumeVisible)
	{
		static Vector3 visibleCandidate;
		const Matrix4x4& cellToScreen = VQData::get().getCellToScreen();

		if(!Math::transformAndDivByW(visibleCandidate, o->getTestVertex(),cellToScreen))
			return true;						// visible point was behind the viewplane (!)

		if(isPointVisible(visibleCandidate))
		{
			DPVS_PROFILE (Statistics::incStatistic(Library::STAT_DATABASEOBSOCCLUSIONSKIPPED,1));
			if(Debug::getLineDrawFlags() & Library::LINE_VPT)
				c->debugDrawLine3D( Library::LINE_VPT, o->getTestVertex(), c->getCameraToCell().getTranslation(), Vector4(0.3f,1.0f,0.2f,0.6f));
			return true;
		}
		else
		{
			DPVS_PROFILE (Statistics::incStatistic(Library::STAT_OBJECTVPTFAILED,1));
			if(Debug::getLineDrawFlags() & Library::LINE_VPT)
				c->debugDrawLine3D( Library::LINE_VPT, o->getTestVertex(), c->getCameraToCell().getTranslation(), Vector4(1.0f,0.3f,0.2f,0.9f));
			o->testVertexFailed();				// VPT failed so we want a new VPT next frame (or maybe the occbuf query below updates the VPT)
		}
	}
#endif // DPVS_VISIBLE_POINT_TRACKING

	//--------------------------------------------------------------------
	// Perform occlusion test. If object is visible, adjust the VPT
	//--------------------------------------------------------------------

	// NOTE: Should we allow flushing for objects? Now determined by "assumeVisible"

	if(m_occlusionWriteQueue->isObjectOccluded())
		return false;

	//--------------------------------------------------------------------
	// If occlusion query gives us an exact VPT -> update our VPT...
	//--------------------------------------------------------------------

#if defined (DPVS_VISIBLE_POINT_TRACKING)
	if(VQData::get().testProperties(VQData::VPT_VALID))
	{
		Vector3 VPT = VQData::get().getVPT();
		c->rasterToScreen(VPT);											// back to screen space
		o->setTestVertex (c->getScreenToCell().transformDivByW(VPT));	// set exact VPT
	} 
#endif

	return true;
}

// NOTE: This function was splitted due to multiple return paths

bool VisibilityQuery::isObjectVisible (bool assumeVisible)
{
	DPVS_ASSERT(VQData::get().testProperties(VQData::FLOW_ACTIVE));
	ImpObject *o = VQData::get().getObject();
	ImpCamera* c = VQData::get().getCamera();

	if(isObjectVisible_INTERNAL(assumeVisible))
	{
		if(o->reportImmediately())
		{
			ImpCommander* ic = DataPasser::getImpCommander();
			ic->setInstanceObject	(o);
			ic->command				(Commander::INSTANCE_IMMEDIATE_REPORT);
			ic->setInstanceObject	(null);							//SAFETY
		}
		return true;
	}

	o->setHiddenTimeStamp(c->getTimeStamp());
	return false;
}

/*****************************************************************************
 *
 * Function:		VisibilityQuery::isSilhouetteVisible(Vector3*,int,float)
 *
 * Description:		Returns boolean value indicating whether specified silhouette is visible
 *
 * Parameters:		vloc = vertex coordinates (in 2D + Z) in screen coordinates
 *					cnt	 = number of vertices
 *					cost = rendering cost of the silhouette
 *
 * Returns:			true if silhouette is visible, false otherwise
 *
 * Notes:			The silhouette must be planar???
 *
 *****************************************************************************/

bool VisibilityQuery::isSilhouetteVisible(Vector3* vloc, int cnt,float cost)
{
	DPVS_ASSERT(getProperties() & Camera::OCCLUSION_CULLING);

#if !defined (DPVS_FLUSH_TABOO)
	VQData::get().setProperties(VQData::ALLOW_FLUSH,true);
#endif

	DPVS_ASSERT(VQData::get().testProperties(VQData::FLOW_ACTIVE));
	DPVS_ASSERT(VQData::get().getCamera());
	DPVS_ASSERT(vloc && cnt >= 3);
	Math::screenToRaster (vloc, cnt, VQData::get().getRasterViewport());	// this function is in dpvsMath.hpp

	const bool visible = (m_occlusionWriteQueue->isSilhouetteOccluded(vloc,cnt,cost)) ? false : true;

	//--------------------------------------------------------------------
	// If a new visible point was provided by the routine, convert the
	// point back to screen coordinates (-1,+1) from raster coordinates
	//--------------------------------------------------------------------

	if (VQData::get().testProperties(VQData::VPT_VALID))
		VQData::get().getCamera()->rasterToScreen(VQData::get().accessVPT());	// RASTER -> SCREEN

	return visible;
}


/*****************************************************************************
 *
 * Function:		VisibilityQuery::isRasterPointVisible_IM()
 *
 * Description:		
 *
 * Parameters:		
 *
 * Returns:			
 *
 *****************************************************************************/

bool VisibilityQuery::isRasterPointVisible_IM	(const FloatRectangle& viewport, const Vector3& rp) const
{
	if (rp.z <= 0.0f || rp.z >= 1.0f)
		return false;

	if(viewport.isInside(rp.x,rp.y) == false)
		return false;

	if(!(getProperties() & Camera::OCCLUSION_CULLING))
		return true;

	return !(m_occlusionWriteQueue->isPointOccluded_IM(rp));
}

/*****************************************************************************
 *
 * Function:		VisibilityQuery::isRectangleVisible()
 *
 * Description:		Returns boolean value indicating whether specified rectangle is visible
 *
 * Parameters:		r			= rectangle in raster-space
 *
 * Returns:			true if silhouette is visible, false otherwise
 *
 * Notes:			Immediate mode functions are not allowed to change state
 *					or use m_currentCamera (=null)
 *
 *****************************************************************************/

bool VisibilityQuery::isRectangleVisible_IM	(const FloatRectangle& viewport, FloatRectangle& r, float depth) const
{
	//---------------------------------------------------------------
	// outside the screen -> hidden/occluded
	//---------------------------------------------------------------

	if(depth<=0.f || depth>=1.f)
		return false;

	if(!r.intersect(viewport))
		return false;

	//---------------------------------------------------------------
	// occlusion culling disable -> visible
	//---------------------------------------------------------------

	if(!(getProperties() & Camera::OCCLUSION_CULLING))
		return true;

	//---------------------------------------------------------------
	// query occlusion buffer WITHOUT flushing
	//---------------------------------------------------------------

	return !m_occlusionWriteQueue->isRectangleOccluded_IM(r,depth);
}

/*****************************************************************************
 *
 * Function:		VisibilityQuery::setPortalVisible(ImpObject*)
 *
 * Description:		
 *
 * Parameters:
 *
 * Notes:			This function is called only by setVisible()
 *
 *****************************************************************************/

void VisibilityQuery::setPortalVisible (UINT32 clipMask)
{
	if(getProperties() & ImpCamera::DISABLE_PORTALTRAVERSAL)	//break if portal traversal is disabled
		return;

	//----------------------------------------------------------------
	// Resolve target cell
	//----------------------------------------------------------------

	DPVS_ASSERT(VQData::get().testProperties(VQData::FLOW_ACTIVE));
	ImpObject* o = VQData::get().getObject();
	DPVS_ASSERT(o);

	ImpPhysicalPortal* p = (ImpPhysicalPortal*)o;				//NOTE: safe operation
	Cell *targetCell = null;

	if(o->getType() == ImpObject::TYPE_VIRTUAL_PORTAL)
	{
		if(getProperties() & Camera::DISABLE_VIRTUALPORTALS)	//break if virtual portals disabled
			return;

		ImpVirtualPortal* vp = (ImpVirtualPortal*)o;			//NOTE: safe operation
		PhysicalPortal* tp = vp->getTargetPortal();
		if(!tp)
			return;												//no target portal

		targetCell = tp->getCell();
	}
	else
	{
		targetCell = p->getTargetCell();
	}

	//----------------------------------------------------------------
	// terminate traversal if the target cell is either null or disabled
	//----------------------------------------------------------------

	if(!targetCell || !targetCell->test(Cell::ENABLED))
		return;

	//----------------------------------------------------------------
	// If the portal gets backface culled, we don't add the camera
	// into the list (destroy it instead).
	//----------------------------------------------------------------

	DPVS_OUTPUT( std::cout << "TraversePortal: " );
	if(p->getType() == ImpObject::TYPE_PHYSICAL_PORTAL)		{ DPVS_OUTPUT( std::cout << "Physical " << p->getName() ); }
	else													{ DPVS_OUTPUT( std::cout << "Virtual  " << p->getName() << " -> " << ((ImpVirtualPortal*)p)->getTargetPortal()->getName() ); }

	//----------------------------------------------------------------
	// Construct a new camera WITHOUT a new occlusion buffer
	//----------------------------------------------------------------

	ImpCamera* targetCamera = new (MALLOC(sizeof(ImpCamera))) ImpCamera(*m_currentCamera);

	if(!traversePortal(targetCamera,targetCell,clipMask))
	{
		DPVS_OUTPUT( std::cout << " Fail\n" );
		DELETE(targetCamera);
	}
}

//------------------------------------------------------------------------
