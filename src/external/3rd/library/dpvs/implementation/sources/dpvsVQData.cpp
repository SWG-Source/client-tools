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
 * Description: 	VQData code
 *
 * $Archive: /dpvs/implementation/sources/dpvsVQData.cpp $
 * $Author: wili $ 
 * $Revision: #1 $
 * $Modtime: 16.11.01 13:27 $
 * $Date: 2003/01/22 $
 * 
 ******************************************************************************/

#include "dpvsVQData.hpp"
#include "dpvsVisibilityQuery.hpp"
#include "dpvsIntersect.hpp"
#include "dpvsEvaluation.hpp"

using namespace DPVS;

#if defined (DPVS_EVALUATION)
	static unsigned int eval5 = 0x12944931;
#endif

/* moved here because new GCC refuses to compile it otherwise */
void VQData::setFrustumPlane				(int i,const Vector4& v) { DPVS_ASSERT(i>=0&&i<MAX_FRUSTUM_PLANES); m_viewFrustum[i] = v; }

/*****************************************************************************
 *
 * Function:		VQData::invalidate()
 *
 * Description:		Called whenever a new visibility query begins
 *
 *****************************************************************************/

void VQData::invalidate	(void)
{
	m_vQuery = null;
	m_camera = null;
	m_object = null;
	disableProperties(ALL);
}

/*****************************************************************************
 *
 * Function:		VQData::newVisibilityQuery()
 *
 * Description:		Called whenever a new visibility query begins
 *
 * Parameters:		vq = pointer to visibility query object
 *
 *****************************************************************************/

void VQData::newVisibilityQuery	(VisibilityQuery* vq)
{
	DPVS_ASSERT(!m_vQuery);
	DPVS_ASSERT(!m_camera);
	DPVS_ASSERT(!m_object);
	DPVS_ASSERT(vq);

	disableProperties(VQData::ALL);

	m_vQuery			= vq;
	m_objectCacheMask	= 0;
	m_camera			= null;
	m_cameraIDMask		= 0;
	m_orthoProjection	= false;

	m_minimumCoverage[0] = vq->getObjectMinimumCoverage(0);
	m_minimumCoverage[1] = vq->getObjectMinimumCoverage(1);

	//----------------------------------------------------------------
	// Set properties
	//----------------------------------------------------------------

#if defined (DPVS_EVALUATION)
	eval5 = DPVS::FLEXLM::getCode(DPVS_FLEXLM_CODE1);
#endif

	if(m_vQuery->getProperties() & Camera::VIEWFRUSTUM_CULLING)
		enableProperties(VQData::PERFORM_VIEWFRUSTUM_CULLING);

	if(m_vQuery->getProperties() & Camera::OCCLUSION_CULLING)
		enableProperties(VQData::PERFORM_OCCLUSION_CULLING);

	if(m_vQuery->allowContributionCulling())
		enableProperties(VQData::PERFORM_CONTRIBUTION_CULLING);

	if(m_vQuery->allowScreenSizeCulling())
		enableProperties(VQData::PERFORM_SCREENSIZE_CULLING);

	enableProperties(FLOW_ACTIVE);
}

/*****************************************************************************
 *
 * Function:		VQData::newCamera()
 *
 * Description:		Called whenever a new camera begins
 *
 * Parameters:		c = pointer to camera object
 *
 *****************************************************************************/

void VQData::newCamera			(ImpCamera* c)
{
	DPVS_ASSERT(m_vQuery);

	//----------------------------------------------------------------
	// Prepare traveler with travel infomation
	//----------------------------------------------------------------

	m_camera				= c;
	m_cellToScreen			= c->getCellToScreen();
	m_screenToCell			= c->getScreenToCell();
	m_cameraLocation		= c->getCameraToCell().getTranslation();	// loc
	m_cameraDOF             = c->getCameraToCell().getDof();			// direction of DOF vector
	m_rasterViewport		= c->getRasterViewport();
	m_orthoProjection		= c->isOrthoProjection();

	// six planes + zero plane + potentially virtualPlane
	m_viewFrustumMask		= (1<<c->getClipPlaneCount())-1;

	// NOTE: the additional "virtual plane" is already set to traveller
	Math::transformPlanes (m_viewFrustum, c->getClipPlanes(), c->getCameraToCell(), 7);

#if defined (DPVS_EVALUATION)
	if ((eval5/13 + 393911) != (DPVS_FLEXLM_ANSWER1/13 + 393911))
	{
		disableProperties(PERFORM_OCCLUSION_CULLING);
		disableProperties(PERFORM_VIEWFRUSTUM_CULLING);
	}
#endif

	//----------------------------------------------------------------
	// Set Camera pseudo-id for spatial database coherence tests. The
	// basic idea here is that scout cams get always mapped to IDs
	// 25-31 and normal cameras to IDs 0-24. 
	//----------------------------------------------------------------

	UINT32 camId = (m_vQuery->getProperties() & Camera::SCOUT) ? ((UINT32)(c->getID())%7) + 25 : ((UINT32)(c->getID())%25);
	m_cameraIDMask			= (1<<camId);			// use only 30 first bits !!

	enableProperties(VQData::CAMERA_VALID);
}

/*****************************************************************************
 *
 * Function:		VQData::newObject()
 *
 * Description:		Called whenever a new object begins
 *
 * Parameters:		o = pointer to object
 *
 * Notes:			Function implemented as inline in the header
 *
 *****************************************************************************/

/*****************************************************************************
 *
 * Function:		VQData::isPointVisible()
 *
 * Description:		
 *
 * Parameters:		cellXYZ = point in cell-space coordinates
 *
 *****************************************************************************/

bool VQData::isPointVisible (const Vector3& cellXYZ) const				
{ 
	Vector3 w(NO_CONSTRUCTOR);

	if(Math::transformAndDivByW(w,cellXYZ,getCellToScreen()))
	{
		if (intersectPointUnitBox(w) && m_vQuery->isPointVisible(w))
			return true;
	}

	return false;
}

//------------------------------------------------------------------------
