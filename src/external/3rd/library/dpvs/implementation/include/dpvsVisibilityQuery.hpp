#ifndef __DPVSVISIBILITYQUERY_HPP
#define __DPVSVISIBILITYQUERY_HPP
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
 * Description: 	VisibilityQuery class. 
 *
 * Notes:			This file is included only by "dpvsImpCamera.cpp"
 *
 * $Archive: /dpvs/implementation/include/dpvsVisibilityQuery.hpp $
 * $Author: timo $ 
 * $Revision: #2 $
 * $Modtime: 23.10.01 12:24 $
 * $Date: 2003/06/19 $
 * 
 ******************************************************************************/

#if !defined (__DPVSARRAY_HPP)
#	include "dpvsArray.hpp"
#endif
#if !defined (__DPVSVECTOR_HPP)
#	include "dpvsVector.hpp"
#endif
#if !defined (__DPVSPTR_HPP)
#	include "dpvsPtr.hpp"
#endif
#if !defined (__DPVSVQDATA_HPP)
#	include "dpvsVQData.hpp"
#endif
#if !defined (__DPVSWRITEQUEUE_HPP)
#	include "dpvsWriteQueue.hpp"
#endif

#include "dpvsXFirstTraversal.hpp"

#if !defined (__DPVSSILHOUETTE_HPP)
#	include "dpvsSilhouette.hpp"
#endif

namespace DPVS
{
class Commander;
class WriteQueue;
class OcclusionBuffer;
class ImpCamera;
class ImpObject;
class ImpPhysicalPortal;
class ImpCell;
class Cell;
class ImpModel;

extern bool g_visibilityQuery;

/******************************************************************************
 *
 * Class:			DPVS::TimeStamp
 *
 * Description:		Timestamping template
 *
 *****************************************************************************/

template<class T,T m_max> class TimeStamp
{
public:
					TimeStamp	(void) : m_time(T(1))		{}
	virtual			~TimeStamp	(void)						{}
	T				operator++	(void)						{ if(m_time==m_max) wrap(),m_time=0; return ++m_time; }
	inline T		value		(void)						{ return m_time; }
	virtual void	wrap		(void) = 0;
protected:
	T				m_time;
};


/******************************************************************************
 *
 * Class:			DPVS::VisibilityQuery
 *
 * Description:		Class used by DPVS::ImpCamera for some of its internal
 *					uses
 *
 *****************************************************************************/

class VisibilityQuery
{

public:
			VisibilityQuery				(void);
			~VisibilityQuery			(void);

	int		getRasterWidth				(void) const	{ return m_rasterWidth;		}
	int		getRasterHeight				(void) const	{ return m_rasterHeight;	}
	UINT32	getProperties				(void) const	{ return m_properties;		}
										
	// --------------------------------------------------------------
	// misc from user
	// --------------------------------------------------------------

	void	minimizeMemoryUsage			(void);
	void	resolveVisibility			(Commander* com, ImpCamera* cam, int recursionDepth,float importanceThreshold);
	bool	setParameters				(int screenWidth,int screenHeight,UINT32 properties,float imageSpaceScalingX,float imageSpaceScalingY);
	void	setStaticCoverageMask		(const unsigned char* buffer,int w,int h,int pitch);
	void	setStaticZBuffer			(const float*  buffer,int w,int h,int pitch,float farValue);

	// --------------------------------------------------------------
	// visibility queries and related
	// --------------------------------------------------------------

	bool	isRasterPointVisible_IM		(const FloatRectangle& viewport, const Vector3& p) const;
	bool	isRectangleVisible_IM		(const FloatRectangle& viewport, FloatRectangle& r, float depth) const;

	bool	isObjectVisible				(bool assumeVisible);
	bool	isPointVisible				(const Vector3& xyz);
	bool	isSilhouetteVisible			(Vector3* vloc, int cnt, float cost);

	void	setVisible					(UINT32);
	void	applyOccluder				(UINT32);

	// --------------------------------------------------------------
	// contribution culling
	// --------------------------------------------------------------

	float	getObjectMinimumCoverage	(int n) const					{ DPVS_ASSERT(n>=0 && n<=2); return m_minimumCoverage[n]; }
	void	setObjectMinimumCoverage	(float w,float h,float opacity);
	bool	allowContributionCulling	(void) const					{ return m_contributionCulling;	}
	bool	allowScreenSizeCulling		(void) const					{ return m_screenSizeCulling;	}

	// --------------------------------------------------------------
	// DEBUG
	// --------------------------------------------------------------

	float	getPreviousFrameCost		(void) const					{ return m_previousFrameCost; }

	float*	grabZBuffer					(int&w,int&h);
	void	disableCulling				(void)			{ m_properties &= ~(Camera::VIEWFRUSTUM_CULLING); m_properties &= ~(Camera::OCCLUSION_CULLING); }

private:
						VisibilityQuery	(const VisibilityQuery&);	// not allowed
	VisibilityQuery&	operator=		(const VisibilityQuery&);	// not allowed

	DPVS_FORCE_INLINE bool	isObjectVisible_INTERNAL	(bool assumeVisible);

	void	init						(void);
	void	clean						(void);

	// ----------------------------------------------------
	// resolveVisibility() helper functions
	// ----------------------------------------------------

	void	resolveVisibility_INTERNAL	(Commander* com, ImpCamera* cam, int recursionDepth,float importanceThreshold);
	bool	resolve_checkParameters		(void) const;
	void	resolve_frameStart			(int recursionDepth);
	bool	resolve_scissor				(void) const;
	void	resolve_flash				(void);
	void	resolve_frameEnd			(bool rendered);

	// ----------------------------------------------------
	// main traversal functions - called from resolveVisibility()
	// ----------------------------------------------------

	void	launchTravelers				(float importanceThreshold);

	void	launch_removeFromWriteQueue	(int levelsTraversed);
	void	launch_updateStencilMasks	(bool stencilsValid, bool currentStencil, bool currentFloating);

	void	applyOccluderSelectionPatch	(void);

	// ----------------------------------------------------
	// maintaining the state of traversal
	// ----------------------------------------------------

	void	setRecursionDepth			(int d);
	int		getRecursionDepth			(void) const			{ return m_recursionDepth;	}
	void	addCameraAndPortal			(ImpCamera* c,ImpPhysicalPortal* p, UINT32 mask);
	void	addTransition				(ImpCell* targetCell, ImpPhysicalPortal* s,ImpPhysicalPortal *d,UINT32 flags);
	bool	testTransition				(ImpPhysicalPortal* s,ImpPhysicalPortal *d,ImpCell* targetCell,UINT32 flags);	// causes only temporary changes
	void	terminateLevel				(void);

	//---------------------------------------------------------------
	// traversal helpers
	//---------------------------------------------------------------

	void	setPortalVisible			(UINT32);
	bool	traversePortal				(ImpCamera* targetCamera, Cell* userTargetCell,UINT32);

	bool	performStencilOP			(ImpCamera* sc, ImpPhysicalPortal* sp,Range<float>& zrange);

	//---------------------------------------------------------------
	// internal misc
	//---------------------------------------------------------------

	void	displayOcclusionBuffer		(void) const;
	int		getObjectCount				(void) const			{ return m_objectList.getCount();	}
	void	ROIvsObject					(const ImpObject* const * objectList,const ImpObject* const * ROIList,int objectCount,int ROICount, const Vector3&) const;
	void	updateQueriesPerSec			(void);

	//---------------------------------------------------------------
	// internal structures
	//---------------------------------------------------------------

	struct CameraPortalStencil
	{
		ImpCamera*			c;
		ImpPhysicalPortal*	p;
		ImpModel*			m;
	};

	struct CameraPortalTestValue
	{
		ImpCamera*			c;
		ImpPhysicalPortal*	p;
		int					testValue;
	};

	//---------------------------------------------------------------
	// This stuff is CONST, but due to splitting of informUser()
	// several data-sharing functions exist
	//---------------------------------------------------------------

	void	informUser					(void) const;

	float	calculateImportance			(void) const;
	void	getCurrentPortalCaps		(bool &stencilMask,bool &floating) const;
	void	reportStencilMask			(ImpPhysicalPortal*p) const;
	void	reportViewport				(ImpCamera*c) const;
	void	reportPortalExit			(ImpCommander* ic,int index) const;
	void	updateActiveStencilList		(int& activeCount) const;

	mutable DepthFirstTraversal<ImpCamera*>			m_cameraList;				// list of cameras
	mutable DepthFirstTraversal<ImpPhysicalPortal*>	m_portalList;				// list of portals
	mutable DynamicArray<CameraPortalTestValue>		m_portalChain;
	mutable DynamicArray<CameraPortalStencil>		m_stencilModelList;			// list of active stencil models
	mutable int										m_stencilModelCount;		// number of stencil models in the m_stencilModelList
	mutable int										m_activeStencilCount;

	//---------------------------------------------------------------
	// normal data
	//---------------------------------------------------------------

	MultiContainer<UINT32>			m_clipMaskList;
	MultiContainer<ImpObject*>		m_objectList;
	MultiContainer<ImpObject*>		m_ROIList;
	EdgeSilhouette					m_stencilSilhouette;		// temp place for every silhouette
	int								m_recursionDepth;			// recursion depth
	float							m_importanceThreshold;		// traversal termination point
	class SweepAndPrune*			m_sweepAndPrune;			// sweep and prune stuff
	class RecursionSolver*			m_recursionSolver;			// pointer to recursion solver

	ImpCamera*						m_currentCamera;
	Commander*						m_currentCommander;
	unsigned int					m_oldFPUMask;

	UINT32							m_properties;				// property mask	
	int								m_rasterWidth;				// rasterization width
	int								m_rasterHeight;				// rasterization height
	float							m_imageSpaceScalingX;		// image-space scaling factor X
	float							m_imageSpaceScalingY;		// image-space scaling factor Y
									
	mutable float					m_previousFrameCost;		// DEBUG DEBUG
	int								m_currentQueryObjectCount;
	int								m_previousQueryObjectCount;	// number of visible objects in last query
	float							m_queriesPerSec;			// number of queries made / sec (this updated every 0.5 seconds)
	int								m_FPSCounter;				// current FPS counter since last startup
	float							m_lastFPSTime;				// FPS timer last startup time
									
	WriteQueue*						m_occlusionWriteQueue;		// uses m_occlusionBuffer
	OcclusionBuffer*				m_occlusionBuffer;			// ptr to occlusion buffer
									
	Vector3							m_minimumCoverage;			// minimum object coverage values (horiz. and vert.)
	bool							m_contributionCulling;		// perform contribution culling?
	bool							m_screenSizeCulling;		// perform screen-size culling?
	bool							m_valid;					// is valid?
	bool							m_preparedForRESEND;		// is prepared for RESEND flag?
	bool							m_globalFlash;				// global flash on?

	//*********************************************************************
	//						Time Stamp Management
	//*********************************************************************

	class PortalTimeStamp : public TimeStamp<UINT32,UINT32(-1)>
	{
	public:
		void wrap (void);
	};

	class FrameTimeStamp : public TimeStamp<UINT32,UINT32(-1)>
	{
	public:
		void wrap (void);
	};

	static PortalTimeStamp						s_portalTimeStamp;
	static FrameTimeStamp						s_frameTimeStamp;
};




/*****************************************************************************
 *
 * Function:		VisibilityQuery::applyOccluder(ImpObject*,UINT32)
 *
 * Description:		Applies object as an occluder (places it into thr write queue)
 *
 * Parameters:		o			= pointer to object
 *					clipMask	= object clip mask
 *
 * Notes:			This function may only be called if occlusion culling
 *					is enabled.
 *					SIngle caller -> INLINE
 *
 *****************************************************************************/

DPVS_FORCE_INLINE void VisibilityQuery::applyOccluder(UINT32 clipMask)
{
	DPVS_ASSERT(VQData::get().testProperties(VQData::FLOW_ACTIVE));
	DPVS_ASSERT(VQData::get().getObject());
	DPVS_ASSERT(getProperties() & Camera::OCCLUSION_CULLING);				

	if(!VQData::get().getObject()->isOcclusionWritable())			// Can be used as an occluder?
		return;

	if (clipMask & (1<<ImpCamera::FLOATING))					// Cannot use occluders that clip the virtual plane!!!
		return;

	m_occlusionWriteQueue->appendOccluder();					// Append to write queue
}

/*****************************************************************************
 *
 * Function:		VisibilityQuery::isPointVisible(ImpObject*, const Vector3&)
 *
 * Description:		Performs a visibility query for a single point in space
 *
 * Parameters:		projXYZ		= point in screen-space coordinates (Z=[-1,+1])
 *
 * Returns:			true if point is visible, false otherwise
 *
 *****************************************************************************/

DPVS_FORCE_INLINE bool VisibilityQuery::isPointVisible (const Vector3& projXYZ)
{
	DPVS_ASSERT(getProperties() & Camera::OCCLUSION_CULLING);	// Must be

	if (projXYZ.z <= -1.0f)								// behind front plane (still using screen-space coords)
		return true;

	Vector2 p(projXYZ.x, projXYZ.y);					// SCREEN space
	m_currentCamera->screenToRaster(p);					// SCREEN -> RASTER
	
	if(!VQData::get().getRasterViewport().isInside(p.x,p.y))	// outside the viewport TODO: test in SCREEN space?
		return false;

#if !defined (DPVS_FLUSH_TABOO)
	VQData::get().setProperties(VQData::ALLOW_FLUSH,true);
#endif

	return (!(m_occlusionWriteQueue->isPointOccluded(Vector3(p.x,p.y,projXYZ.z * 0.5f + 0.5f))));
}


} // DPVS

//------------------------------------------------------------------------
#endif // __DPVSVISIBILITYQUERY_HPP
