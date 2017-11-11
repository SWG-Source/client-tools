#ifndef __DPVSIMPCAMERA_HPP
#define __DPVSIMPCAMERA_HPP

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
 * Description: 	Camera implementation class
 *
 * $Archive: /dpvs/implementation/include/dpvsImpCamera.hpp $
 * $Author: wili $ 
 * $Revision: #1 $
 * $Modtime: 2.10.02 13:41 $
 * $Date: 2003/01/22 $
 * 
 ******************************************************************************/

#if !defined (__DPVSPRIVATEDEFS_HPP)
#	include "dpvsPrivateDefs.hpp"
#endif
#if !defined (__DPVSCAMERA_HPP)
#	include "dpvsCamera.hpp"
#endif
#if !defined (__DPVSCOMMANDER_HPP)
#	include "dpvsCommander.hpp"
#endif
#if !defined (__DPVSMATRIX_HPP)
#	include "dpvsMatrix.hpp"
#endif
#if !defined (__DPVSRECTANGLE_HPP)
#	include "dpvsRectangle.hpp"
#endif

/******************************************************************************
 *
 * Class:			DPVS::ImpCamera
 *
 * Description:		Implementation of Camera
 *
 *****************************************************************************/

namespace DPVS
{
class ImpObject;
class VisibilityQuery;

inline class ImpCamera* Camera::getImplementation	(void) const	{ return m_imp; }

class ImpCamera
{
public:
	// DO NOT CHANGE THESE ENUMERATIONS AS THEY'RE REFLECTED
	// IN THE COMMANDER::INSTANCE::CLIP ENUMERATION!!! (NOTE THAT
	// THE ZERO PLANE IS ONLY AN INTERNAL ISSUE AND NOT SHOWN IN
	// THE PUBLIC ENUMERATION

	enum Plane						// view frustum plane enumeration
	{
		ZERO	= 0,				// zero plane (same normal as front plane but crosses the eye)
		FRONT	= 1,				// front clip plane	
		BACK	= 2,				// back clip plane
		LEFT	= 3,				// left clip plane
		RIGHT	= 4,				// right clip plane
		TOP		= 5,				// top clip plane
		BOTTOM	= 6,				// bottom clip plane
		FLOATING= 7					// portal clipping plane
	};

	enum
	{
		DISABLE_PORTALTRAVERSAL	= (1<<10),		// internal
		DISABLE_COSTBENEFIT		= (1<<11),		// internal - not effective right now
		DISABLE_USERCALLBACKS	= (1<<12)		// disables all user callbacks, used for scouts
	};

						ImpCamera					(void);
	virtual				~ImpCamera					(void);
#if defined (DPVS_DEBUG)
	void				checkConsistency			(void) const;
#else
	void				checkConsistency			(void) const {}
#endif

	static void			checkConsistencyAll			(void);

	static void			minimizeMemoryUsage			(void);

	VisibilityQuery*	getVisibilityQuery			(void) const					{ return m_vQuery; }
	
	//---------------------------------------------------------------
	// frustum		
	//---------------------------------------------------------------

	const Frustum&		getUserFrustum				(void) const					{ return m_userFrustum; }
	const Frustum&		getFrustum					(void) const					{ return m_frustum; }
	const Vector4&		getUserFrustumPlane			(int n) const					{ return m_clipPlanes[n+1]; }	//skip ZERO plane
	int					getUserFrustumPlaneCount	(void) const					{ return (m_allowVirtualPlane) ? (m_clipPlaneCount-1) : (6); }	//skip ZERO plane
	void				setUserFrustum				(const Frustum& f,bool ortho);

	bool				isOrthoProjection			(void) const					{ return m_orthoProjection; }

	void				setPixelCenter				(float x,float y)				{ m_pixelCenter.x=x; m_pixelCenter.y=y; recomputeUserCameraToScreen(); }
	void				getPixelCenter				(float& x,float& y) const		{ x=m_pixelCenter.x; y=m_pixelCenter.y; }

	//---------------------------------------------------------------
	// This can be called directly with the Plane enumerations (doesn't skip zero plane)
	// These are in camera's space
	//---------------------------------------------------------------

	const Vector4&		getClipPlane				(int n) const					{ DPVS_ASSERT(n>=0); return m_clipPlanes[n]; }
	const Vector4*		getClipPlanes				(void) const					{ return m_clipPlanes; }
	void				setClipPlane				(int n,const Vector4& p)		{ DPVS_ASSERT(n>=0); m_clipPlanes[n] = p; }
	int					getClipPlaneCount			(void) const					{ return m_clipPlaneCount; }
	void				setClipPlaneCount			(int n)							{ m_clipPlaneCount = n; }
						
	//---------------------------------------------------------------
	// scissor
	//---------------------------------------------------------------

	bool				isScissorActive				(void) const					{ return m_scissorActive; }
	void				setUserScissor				(const FloatRectangle& s)		{ m_userScissor=s; m_userScissor.setSpace(SPACE_RASTER); m_scissorDirty=true; }
const FloatRectangle&	getScissor					(void) const					{ return (m_scissorDirty) ? m_userScissor : m_rasterRectangle; }
const FloatRectangle&	getRasterViewport			(void) const					{ return m_rasterRectangle; }

	//---------------------------------------------------------------
	// matrices
	//---------------------------------------------------------------

	const Matrix4x3&	getCameraToCell				(void) const					{ return m_cameraToCell;		}
	const Matrix4x4&	getCameraToScreen			(void) const					{ return m_cameraToScreen;		}
	const Matrix4x3&	getCellToCamera				(void) const					{ return m_cellToCamera;		}
	const Matrix4x4&	getCellToScreen				(void) const					{ return m_cellToScreen;		}
	const Matrix4x4&	getScreenToCell				(void) const					{ return m_screenToCell;		}
	const Matrix4x4&	getUserCameraToScreen		(void) const					{ return m_userCameraToScreen;	}
	void				setCameraToCell				(const Matrix4x3 &mtx);


	//---------------------------------------------------------------
	// space conversions
	//---------------------------------------------------------------

	void				screenToRaster				(Vector2&) const;			// SCREEN -> RASTER
	void				rasterToScreen				(Vector2&) const;			// RASTER -> SCREEN
	void				rasterToScreen				(Vector3&) const;			// RASTER -> SCREEN

	//---------------------------------------------------------------
	// binding/instance/etc information
	//---------------------------------------------------------------

	Cell*				getCell						(void) const					{ return m_currentCell; }
	void				setCellSoftReference		(Cell* c);						
	void				setCellNoReference			(Cell* c)						{ m_currentCell = c; }
	UINT32				getID						(void) const					{ return m_ID; }
	void				setID						(UINT32 id)						{ m_ID = id; }
	Camera*				getUserCamera				(void) const					{ return m_userCamera; }
	void				setUserCamera				(Camera *c)						{ m_userCamera=c; }
	UINT32				getTimeStamp				(void) const					{ return m_timeStamp; }
	void				setTimeStamp				(UINT32 ts)						{ m_timeStamp = ts; }
	const Commander::Viewer&	getViewer			(void) const					{ return m_viewer; }

	//---------------------------------------------------------------
	// MISC
	//---------------------------------------------------------------

	void				setParameters				(int screenWidth,int screenHeight,UINT32 properties=Camera::VIEWFRUSTUM_CULLING|Camera::OCCLUSION_CULLING,float imageSpaceScaleX=1.f,float imageSpaceScaleY=1.f);

	void				allowVirtualPlane			(bool v)						{ m_allowVirtualPlane = v; }
	void				getBackProjectionShaft		(class AABB&) const;

	int					getRasterWidth				(void) const;
	int					getRasterHeight				(void) const;

	//---------------------------------------------------------------
	// DEBUG
	//---------------------------------------------------------------

	void				debugDrawBox3D				(Library::LineType type, const class AABB&, const Vector4&);
	void				debugDrawLine3D				(Library::LineType type, const Vector3&,const Vector3&, const Vector4&);
	void				debugDrawLine2D				(Library::LineType type, const Vector2&,const Vector2&, const Vector4&);

private:
	friend class VisibilityQuery;		// constructs new cameras when portals are encountered

	DPVS_FORCE_INLINE int	getRasterWidth_inline		(void) const;
	DPVS_FORCE_INLINE int	getRasterHeight_inline		(void) const;

	enum
	{
		MAX_CLIP_PLANES = 16
	};

							ImpCamera					(const ImpCamera&);
	ImpCamera&				operator=					(const ImpCamera& s);

	// Called by resolveVisibility()
	bool					validateScissor				(void);
	// Called by traversePortal()
	bool					createFrustumFromRectangle	(Frustum& f, const FloatRectangle &scissor);
	void					setFrustumPlanesAndMatrix	(const Frustum &f);

	void					recomputeUserCameraToScreen	(void);
	void					setupCellToScreen			(void);
	void					setupClipPlanes				(Vector4* c, const Matrix4x4& m);

	// ----------------------------------------------------
	// class VisibilityQuery ???
	// ----------------------------------------------------

	Frustum					m_userFrustum;					// frustum given by the user (not modified)
	FloatRectangle			m_userScissor;					// scissor given by the user (modified when resizing the viewport)

	//----------------------------------------------------
	// class ImpCamera
	//----------------------------------------------------

	ImpCamera*				m_prev;							// previous in global linked list
	ImpCamera*				m_next;							// next in global linked list	

	Frustum					m_frustum;						// frustum
	FloatRectangle			m_rasterRectangle;				// raster rectangle
	Vector4					m_clipPlanes[MAX_CLIP_PLANES];	// clip planes
	Vector2					m_pixelCenter;					// 
	int						m_clipPlaneCount;				// how many clipplanes
	
	Matrix4x3				m_cameraToCell;					// camera->cell matrix
	Matrix4x3				m_cellToCamera;					// cell->camera matrix
	Matrix4x4				m_cellToScreen;					// cell->screen matrix
	Matrix4x4				m_screenToCell;					// screen->cell matrix
	Matrix4x4				m_cameraToScreen;				// camera->screen matrix
	Matrix4x4				m_userCameraToScreen;			// camera->screen matrix with pixel center offsets
															
	UINT32					m_ID;							
	UINT32					m_timeStamp;					// portal time - incremented outside
	Cell*					m_currentCell;					// pointer to current cell
	Commander::Viewer		m_viewer;						// viewer object
	Camera*					m_userCamera;					// pointer to use camera (interface)

	VisibilityQuery*		m_vQuery;						// pointer to VisibilityQuery object

	bool					m_allowVirtualPlane : 1;		// has additional plane for virtual portals
	bool					m_ownsVisibilityQuery:1;		// KLUDGE
	bool					m_scissorDirty:1;				// is scissor rectangle dirty
	bool					m_scissorActive:1;				// set by validateScissor()
	bool					m_orthoProjection:1;			// 

	static ImpCamera*		s_head;							// first ImpCamera in global linked list
	static UINT32			s_hashCounter;					// counter used to ensure good distribution in hash functions
};

//------------------------------------------------------------------------
// Implementation of some short inline functions
//------------------------------------------------------------------------

inline void ImpCamera::screenToRaster(Vector2& v) const
{
	v.x = ( v.x * 0.5f + 0.5f) * m_rasterRectangle.width()  + m_rasterRectangle.x0;
	v.y = (-v.y * 0.5f + 0.5f) * m_rasterRectangle.height() + m_rasterRectangle.y0;
}
} //DPVS

//------------------------------------------------------------------------
#endif //__DPVSIMPCAMERA_HPP

