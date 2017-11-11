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
 * Description: 	VisibilityQuery::resolveVisibility() code
 *
 * $Archive: /dpvs/implementation/sources/dpvsVisibilityQuery_Resolve.cpp $
 * $Author: wili $ 
 * $Revision: #2 $
 * $Modtime: 8.01.02 12:39 $
 * $Date: 2003/08/24 $
 * 
 ******************************************************************************/

#include "dpvsVisibilityQuery.hpp"
#include "dpvsImpCommander.hpp"
#include "dpvsImpCamera.hpp"
#include "dpvsImpObject.hpp"
#include "dpvsImpCell.hpp"
#include "dpvsSystem.hpp"
#include "dpvsDebug.hpp"
#include "dpvsWriteQueue.hpp"
#include "dpvsOcclusionBuffer.hpp"
#include "dpvsSilhouetteCache.hpp"
#include "dpvsScratchpad.hpp"

#if defined (DPVS_BUILD_MSC) && defined(DPVS_DEBUG)
#	include <cfloat>									// for FPU exception masking
#endif

using namespace DPVS;

namespace DPVS
{
	bool g_visibilityQuery = false;						// used for debugging purposes
}

/*****************************************************************************
 *
 * Function:		VisibilityQuery::resolveVisibility()
 *
 * Parameters:		forceFlash = boolean value indicating whether "flashing" should be forced
 *
 *****************************************************************************/

void VisibilityQuery::resolveVisibility(Commander* com, ImpCamera* cam, int recursionDepth,float importanceThreshold)
{
	DPVS_PROFILE		(Statistics::incStatistic(Library::STAT_CAMERARESOLVEVISIBILITYCALLS,1));
	DPVS_API_ASSERT	("resolveVisibility() - RESEND flag can only be used after PREPARE_RESEND flag" && !((m_properties & Camera::RESEND) && !m_preparedForRESEND));

	if((m_properties & Camera::RESEND) && !m_preparedForRESEND)
		return;

	//-----------------------------------------------------
	//					QUERY_BEGIN
	//-----------------------------------------------------

	g_visibilityQuery	= true;
	m_currentCamera		= cam;
	m_currentCommander	= com;
	DataPasser::setCommander(com);

	{
		#if defined (DPVS_DEBUG)
			UINT32 oldFPUMode = getFPUMode();
		#endif
		m_currentCommander->command(Commander::QUERY_BEGIN);
		DPVS_ASSERT(getFPUMode() == oldFPUMode && "FPU mode changed in user callback!");
	}

	if(!resolve_checkParameters())
	{
		//-----------------------------------------------------
		//					QUERY_END
		//-----------------------------------------------------

	#if defined (DPVS_DEBUG)
		UINT32 oldFPUMode = getFPUMode();
	#endif
		m_currentCommander->command(Commander::QUERY_END);
		DPVS_ASSERT(getFPUMode() == oldFPUMode && "FPU mode changed in user callback!");
		DataPasser::setCommander(null);
		g_visibilityQuery = false;
		return;
	}

	//-----------------------------------------------------
	// Handle RESEND
	//-----------------------------------------------------

	if(m_properties & Camera::RESEND)
	{
		Statistics::pushStatistics();

		if(!(m_properties & ImpCamera::DISABLE_USERCALLBACKS) && DataPasser::getCommander())
			informUser();

		Statistics::popStatistics();
	}
	else
	{
		VisibilityQuery::resolveVisibility_INTERNAL(com,cam,recursionDepth,importanceThreshold);

		//-----------------------------------------------------
		// Handle PREPARE_RESEND
		//-----------------------------------------------------

		if((m_properties & Camera::PREPARE_RESEND))
		{
			m_preparedForRESEND = true;
		}
		else
		{
			m_preparedForRESEND = false;
			clean();							// destroy internal camera allocations
			init();								// clear object lists
		}
	}

	//-----------------------------------------------------
	//					QUERY_END
	//-----------------------------------------------------

	{
		#if defined (DPVS_DEBUG)
			UINT32 oldFPUMode = getFPUMode();
		#endif

		m_currentCommander->command(Commander::QUERY_END);
		DPVS_ASSERT(getFPUMode() == oldFPUMode && "FPU mode changed in user callback!");
		DataPasser::setCommander(null);
		g_visibilityQuery = false;
	}
}


/*****************************************************************************
 *
 * Function:		VisibilityQuery::resolveVisibility_INTERNAL()
 *
 * Parameters:		
 *
 *****************************************************************************/

void VisibilityQuery::resolveVisibility_INTERNAL(Commander* com, ImpCamera* cam, int recursionDepth,float importanceThreshold)
{
/*
	if(m_previousQueryObjectCount<20)
		m_previousQueryObjectCount	= m_currentQueryObjectCount;
	else
		m_previousQueryObjectCount	= Math::intChop(0.9f*m_previousQueryObjectCount + 0.1f*m_currentQueryObjectCount);
*/
	m_currentQueryObjectCount	= 0;

	m_currentCamera		= cam;
	m_currentCommander	= com;
	DataPasser::setCommander(m_currentCommander);				// commander active

	resolve_frameStart(recursionDepth);							// init everything
	resolve_flash();											// determine flash condition

	if(resolve_scissor())										// validate scissor
	{
		launchTravelers(importanceThreshold);					// perform traversal
		m_currentCamera = null;									// not in valid state!
		resolve_frameEnd(true);
	}
	else
	{
		resolve_frameEnd(false);
	}

	if(m_globalFlash || m_previousQueryObjectCount<20)
		m_previousQueryObjectCount	= m_currentQueryObjectCount;
	else
		m_previousQueryObjectCount	= Math::intChop(0.9f*m_previousQueryObjectCount + 0.1f*m_currentQueryObjectCount);
}

/*****************************************************************************
 *
 * Function:		VisibilityQuery::resolve_checkParameters()
 *
 * Description:		Checks parameters of resolveVisibility()
 *
 *****************************************************************************/

bool VisibilityQuery::resolve_checkParameters() const
{
	DPVS_API_ASSERT(m_valid && "Camera::resolveVisibility(): Probably forgot to call setParameters()");
	DPVS_API_ASSERT(m_rasterWidth>=0 && m_rasterHeight>=0 && "Camera::resolveVisibility(): Invalid screen resolution");	// "valid" screen

	//--------------------------------------------------------------------
	// Make sure view frustum is OK. If not, assert in debug build and
	// return immediately in release build (other stuff could crash
	// internally if we proceeded).
	//--------------------------------------------------------------------

	const Frustum& frustum = m_currentCamera->m_frustum;
	
	if (frustum.left  == frustum.right  ||
		frustum.top   == frustum.bottom ||
		frustum.zNear >= frustum.zFar   ||
		frustum.zNear <= 0.0f)
	{
		DPVS_API_ASSERT(frustum.zNear >= 0.f && "Camera::resolveVisibility(): Invalid view frustum Z near value");				// "valid" frustum
		DPVS_API_ASSERT(frustum.zFar  > frustum.zNear && "Camera::resolveVisibility(): Invalid view frustum Z values");		// "valid" frustum
		DPVS_API_ASSERT(frustum.left != frustum.right && "Camera::resolveVisibility(): Invalid view frustum");
		DPVS_API_ASSERT(frustum.top  != frustum.bottom && "Camera::resolveVisibility(): Invalid view frustum");
		return false;
	}

	if(getRasterWidth()==0 || getRasterHeight()==0)
	{
#if defined(DPVS_DEBUG)
		Debug::print("DPVS did nothing because either vertical or horizontal resolution was zero in resolveVisibility()\n");
#endif
		return false;
	}

	if(m_currentCamera->getCell()==null)
	{
#if defined(DPVS_DEBUG)
		Debug::print("DPVS did nothing because camera was not assigned to a cell\n");
#endif
		return false;
	}

	if(!m_currentCamera->getCell()->test(Cell::ENABLED))
	{
#if defined(DPVS_DEBUG)
		Debug::print("DPVS did nothing because camera is in a disabled cell\n");
#endif
		return false;
	}

	return true;
}


/*****************************************************************************
 *
 * Function:		VisibilityQuery::resolve_frameStart()
 *
 * Description:		
 *
 * Notes:			
 *
 *****************************************************************************/

void VisibilityQuery::resolve_frameStart(int recursionDepth)
{
	//--------------------------------------------------------------------
	// Activate FPU exceptions
	//--------------------------------------------------------------------

#if defined (DPVS_BUILD_MSC) && defined(DPVS_DEBUG)
	_clearfp  ();
	m_oldFPUMask = _controlfp (0,0);
    _controlfp(_EM_INEXACT | _EM_UNDERFLOW, _MCW_EM); 
#endif

	//--------------------------------------------------------------------
	// inform occlusion subsystem
	//--------------------------------------------------------------------

	if(m_properties & Camera::OCCLUSION_CULLING)
		m_occlusionBuffer->frameStart();

	//--------------------------------------------------------------------
	// init internal lists
	//--------------------------------------------------------------------

	clean();							// destroy internal camera allocations
	init();								// clear object lists
	setRecursionDepth(recursionDepth);

	//--------------------------------------------------------------------
	// update time related stats
	//--------------------------------------------------------------------

	DataPasser::setQueryTime(Debug::getServices()->getTime());
	updateQueriesPerSec();

	//--------------------------------------------------------------------
	// update time stamps
	//--------------------------------------------------------------------

	DataPasser::setFrameCounter(++s_frameTimeStamp);
	++s_portalTimeStamp;

	m_currentCamera->setTimeStamp(s_portalTimeStamp.value());	// set time stamp to USERs camera
	SilhouetteCache::get()->updateTimeStamp();					// update silhouette cache time stamp
}

/*****************************************************************************
 *
 * Function:		VisibilityQuery::resolve_scissor()
 *
 * Description:		
 *
 * Returns:			false is must be aborted
 *
 *****************************************************************************/

bool VisibilityQuery::resolve_scissor(void) const
{
	if(!m_currentCamera->validateScissor())
	{
	#if defined(DPVS_DEBUG)
		Debug::print("DPVS did nothing because the requested scissor rectangle is outside the screen\n");
	#endif
	#if defined (DPVS_DEBUG)
		UINT32 oldFPUMode = getFPUMode();
	#endif

		m_currentCommander->command(Commander::QUERY_END);

		DPVS_ASSERT(getFPUMode() == oldFPUMode && "FPU mode changed in user callback!");

		if(m_properties & Camera::OCCLUSION_CULLING)
			m_occlusionBuffer->frameEnd();
		return false;
		}

	if(m_properties & Camera::OCCLUSION_CULLING)
		m_occlusionWriteQueue->clear(m_currentCamera);		// CLEARS, PADS and SCISSORS occlusion buffer

	return true;
}

/*****************************************************************************
 *
 * Function:		VisibilityQuery::resolve_flash()
 *
 * Description:		Determine if this is a "flash" frame. And do the flash.
 *
 *****************************************************************************/

void VisibilityQuery::resolve_flash(void)
{
	//------------------------------------------------------------------------
	// Decide whether we're going to "flash". This is always done during the
	// first 20 frames of the lifetime of the camera or if the SCOUT flag
	// has been turned on.
	//------------------------------------------------------------------------

	if(	(m_properties & (/*Camera::OPTIMIZE|*/Camera::SCOUT)) ||
		(m_currentCamera->getTimeStamp()<20) /*|| ((m_currentCamera->getTimeStamp() & 31)==0)*/)
		m_globalFlash = true;
	else
		m_globalFlash = false;

	ImpObject::forceSelectAll (m_globalFlash);
}


/*****************************************************************************
 *
 * Function:		VisibilityQuery::resolve_frameEnd()
 *
 * Description:		
 *
 *****************************************************************************/

void VisibilityQuery::resolve_frameEnd(bool rendered)
{
#if defined (DPVS_EVALUATION)
	static unsigned int foo = DPVS::FLEXLM::getCode(DPVS_FLEXLM_CODE3);
#endif
	//------------------------------------------------------------------------
	// Assign benefits to objects and teel the frame has ended
	//------------------------------------------------------------------------

	if(m_properties & Camera::OCCLUSION_CULLING)
	{
		if(rendered)
			m_occlusionWriteQueue->evaluateBenefits();
		m_occlusionBuffer->frameEnd();
	}

	//------------------------------------------------------------------------
	// Restore old floating point status (MSVC debug build only) before making
	// any use callbacks
	//------------------------------------------------------------------------

#if defined (DPVS_BUILD_MSC) && defined(DPVS_DEBUG)
    _controlfp(m_oldFPUMask, _MCW_EM); 
	_clearfp ();
#endif

	//------------------------------------------------------------------------
	// Display requested buffers
	//------------------------------------------------------------------------

	if (rendered)
		displayOcclusionBuffer();

	Scratchpad::pop();				// let's restore scratchpad

	//------------------------------------------------------------------------
	// informUser...
	//------------------------------------------------------------------------

	if(rendered)
	{
		if(!(m_properties & ImpCamera::DISABLE_USERCALLBACKS))
			informUser();
	}

	// evaluation copy code
#if defined (DPVS_EVALUATION)
	if (!(g_random.getI()&63) && ((foo+119224)*29339 != (DPVS_FLEXLM_CODE3+119224)*29339))
		disableCulling();
#endif // 

}

//------------------------------------------------------------------------
