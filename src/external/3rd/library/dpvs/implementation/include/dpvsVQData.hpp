#ifndef __DPVSVQDATA_HPP
#define __DPVSVQDATA_HPP
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
 *
 * Desc:	Shared data during visibility queries
 *
 * $Archive: /dpvs/implementation/include/dpvsVQData.hpp $
 * $Author: wili $ 
 * $Revision: #1 $
 * $Modtime: 16.11.01 13:27 $
 * $Date: 2003/01/22 $
 * 
 *****************************************************************************/

#if !defined (__DPVSMATRIX_HPP)
#	include "dpvsMatrix.hpp"		
#endif
#if !defined (__DPVSIMPCAMERA_HPP)
#	include "dpvsImpCamera.hpp"
#endif
#if !defined (__DPVSIMPOBJECT_HPP)
#	include "dpvsImpObject.hpp"
#endif
#if !defined (__DPVSRANGE_HPP)
#	include "dpvsRange.hpp"
#endif
#if !defined (__DPVSSCRATCHPAD_HPP)
#	include "dpvsScratchpad.hpp"
#endif

namespace DPVS
{ 
class ImpCamera;
class AABB;
class ImpObject;
class VisibilityQuery;


/******************************************************************************
 *
 * Class:			DPVS::VQData
 *
 * Description:		Class for passing traversal information for database
 *					traversal. The database also accesses the camera/occlusion
 *					subsystem by using the traveler's member functions.
 *
 ******************************************************************************/

//extern class VQData g_myVQData;

class VQData
{
public:

	// the VQData is allocated into the scratchpad
	static DPVS_FORCE_INLINE VQData& get (void)	 { return *reinterpret_cast<VQData*>(Scratchpad::getPtr(Scratchpad::VQDATA)); }

	enum
	{
		MAX_FRUSTUM_PLANES			= 8			// max # of planes supported by the view frustum
	};

	enum
	{
		FLOW_ACTIVE					= (1<<0),
		CAMERA_VALID				= (1<<1),
		OBJECT_VALID				= (1<<2),
		VPT_VALID					= (1<<3),
		DEPTH_RANGE_VALID			= (1<<4),
		PERFORM_OCCLUSION_CULLING	= (1<<10),
		PERFORM_VIEWFRUSTUM_CULLING	= (1<<11),
		PERFORM_CONTRIBUTION_CULLING= (1<<12),
		PERFORM_SCREENSIZE_CULLING	= (1<<13),
		ALLOW_FLUSH					= (1<<14),
		RECTANGLE_CLIPPED			= (1<<15),
		ALL							= (0x7FFFFFFF)
	};

	DPVS_FORCE_INLINE void						enableProperties			(UINT32 m)				{ m_properties |= m; }
	DPVS_FORCE_INLINE void						disableProperties			(UINT32 m)				{ m_properties &= ~m; }
	DPVS_FORCE_INLINE void						setProperties				(UINT32 m,bool v)		{ if(v) enableProperties(m); else disableProperties(m); }
	DPVS_FORCE_INLINE UINT32					getProperties				(void) const			{ return m_properties; }
	DPVS_FORCE_INLINE bool						testProperties				(UINT32 m) const		{ return (m_properties&m) == m; }
							
	void										invalidate					(void);
	void										newVisibilityQuery			(VisibilityQuery* vq);
	void										newCamera					(ImpCamera* c);
	DPVS_FORCE_INLINE void						newObject					(ImpObject* o);

	//---------------------------------------------------------------
	// object
	//---------------------------------------------------------------

	DPVS_FORCE_INLINE ImpObject*				getObject					(void) const			{ return m_object; }
	DPVS_FORCE_INLINE Vector3&					accessVPT					(void)					{ return m_visiblePoint; }
	DPVS_FORCE_INLINE const Vector3&			getVPT						(void) const			{ return m_visiblePoint; }
	DPVS_FORCE_INLINE void						setVPT						(const Vector3& v)		{ m_visiblePoint = v; }
	DPVS_FORCE_INLINE Range<float>&				accessDepthRange			(void)					{ return m_depthRange; }
	DPVS_FORCE_INLINE const Range<float>&		getDepthRange				(void) const			{ return m_depthRange; }

	//---------------------------------------------------------------
	// camera
	//---------------------------------------------------------------

	DPVS_FORCE_INLINE ImpCamera*				getCamera					(void) const			{ return m_camera; }
	DPVS_FORCE_INLINE bool						isOrthoProjection			(void) const			{ return m_orthoProjection; }
	DPVS_FORCE_INLINE bool						isPerspectiveProjection		(void) const			{ return !m_orthoProjection; }
							
	DPVS_FORCE_INLINE const Matrix4x4&			getCellToScreen				(void) const			{ return m_cellToScreen; }
	DPVS_FORCE_INLINE const Matrix4x4&			getScreenToCell				(void) const			{ return m_screenToCell; }
							
	DPVS_FORCE_INLINE const Vector3&			getCameraDOF				(void) const			{ return m_cameraDOF;					}
	DPVS_FORCE_INLINE UINT32					getCameraIDMask				(void) const			{ return m_cameraIDMask; }
	DPVS_FORCE_INLINE const Vector3&			getCameraLocation			(void) const			{ return m_cameraLocation;				}
	DPVS_FORCE_INLINE UINT32					getInitialFrustumMask		(void) const			{ return m_viewFrustumMask;				}
	DPVS_FORCE_INLINE float						getObjectMinimumCoverage	(int n) const			{ return m_minimumCoverage[n]; }
	DPVS_FORCE_INLINE const Vector4*			getViewFrustumPlanes		(void) const			{ return m_viewFrustum;					}

	DPVS_FORCE_INLINE FloatRectangle&			accessRasterViewport		(void)					{ return m_rasterViewport; }
	DPVS_FORCE_INLINE const FloatRectangle&		getRasterViewport			(void) const			{ return m_rasterViewport; }

	DPVS_FORCE_INLINE bool						performOcclusionCulling		(void) const			{ return testProperties(PERFORM_OCCLUSION_CULLING); }
	DPVS_FORCE_INLINE bool						performViewFrustumCulling	(void) const			{ return testProperties(PERFORM_VIEWFRUSTUM_CULLING); }

	//---------------------------------------------------------------
	// TO VisiblityQuery
	//---------------------------------------------------------------

	DPVS_FORCE_INLINE VisibilityQuery*			getVisibilityQuery			(void) const					{ return m_vQuery; }
	bool										isPointVisible				(const Vector3& cellXYZ) const;

	//---------------------------------------------------------------
	// TO ImpCamera
	//---------------------------------------------------------------

	DPVS_FORCE_INLINE void						drawLine2D					(Library::LineType type, const Vector2& a, const Vector2& b, const Vector4& col) const		{ m_camera->debugDrawLine2D(type, a, b, col);  }
	DPVS_FORCE_INLINE void						drawLine3D					(Library::LineType type, const Vector3& a, const Vector3& b, const Vector4& col) const		{ m_camera->debugDrawLine3D(type, a, b, col); }
	DPVS_FORCE_INLINE void						drawBox3D					(Library::LineType type, const AABB& cellSpaceBounds, const Vector4& color) const			{ m_camera->debugDrawBox3D (type, cellSpaceBounds, color); }

	//---------------------------------------------------------------
	// FROM VisiblityQuery (get rid of this?)
	//---------------------------------------------------------------
	void										setFrustumPlane				(int i,const Vector4& v);

private:
						VQData					(void);				// can't construct these
						VQData					(const VQData&);	// not allowed
	VQData&				operator=				(const VQData&);	// not allowed

	//---------------------------------------------------------------
	// per visibility query
	//---------------------------------------------------------------

	VisibilityQuery*	m_vQuery;							// visiblity query object
	UINT32				m_properties;
	float				m_minimumCoverage[2];

	//---------------------------------------------------------------
	// per object
	//---------------------------------------------------------------

	ImpObject*			m_object;							// pointer to current object
	UINT32				m_objectCacheMask;					// object cache mask

	Vector3				m_visiblePoint;
	Range<float>		m_depthRange;

	//---------------------------------------------------------------
	// per camera (set up by ImpCamera::setupVQData())
	//---------------------------------------------------------------
	
	FloatRectangle		m_rasterViewport;
	UINT32				m_viewFrustumMask;					// view frustum active mask
	Vector3				m_cameraLocation;					// camera location in cell space
	Vector3				m_cameraDOF;						// camera DOF
	ImpCamera*			m_camera;							// call m_camera->isVisible() etc
	UINT32				m_cameraIDMask;						// camera ID mask (only one bit can be set!!)
	bool				m_orthoProjection;					// is the projection orthographic?
	Matrix4x4			m_cellToScreen;						// cell -> screen (modelview*projection) matrix
	Matrix4x4			m_screenToCell;						// screen -> cell matrix
	Vector4				m_viewFrustum[MAX_FRUSTUM_PLANES];	// view frustum plane equations in cell space
};

DPVS_CT_ASSERT(sizeof(VQData) <= 384);						// we said it's going to be smaller than 768 bytes!

DPVS_FORCE_INLINE void VQData::newObject (ImpObject* o)
{
	m_object = o;
	m_objectCacheMask = 0;
	disableProperties(VPT_VALID);
}

} //namespace DPVS

//------------------------------------------------------------------------
#endif //__DPVSVQDATA_HPP
