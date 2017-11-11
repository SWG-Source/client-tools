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
 * Description: 	Camera code
 *
 * $Archive: /dpvs/implementation/sources/dpvsImpCamera.cpp $
 * $Author: otso $ 
 * $Revision: #2 $
 * $Modtime: 2.10.02 13:43 $
 * $Date: 2004/11/09 $
 * 
 ******************************************************************************/

#include "dpvsImpCamera.hpp"
#include "dpvsVisibilityQuery.hpp"
#include "dpvsOcclusionBuffer.hpp"
#include "dpvsSurface.hpp"
#include "dpvsImpObject.hpp"
#include "dpvsWriteQueue.hpp"
#include "dpvsImpCell.hpp"
#include "dpvsRecursionSolver.hpp"
#include "dpvsRandom.hpp"
#include "dpvsImpCommander.hpp"		// for buffer grab
#include "dpvsDebug.hpp"
#include "dpvsInstanceCount.hpp"
#include "dpvsMemory.hpp"			// for consistency checking

#if defined(DPVS_OUTPUT_PORTAL_TRAVERSAL)
#pragma warning(push,1)
#	include <iostream>
#pragma warning(pop)
#endif //DPVS_OUTPUT_PORTAL_TRAVERSAL


using namespace DPVS;

DPVS_FORCE_INLINE int ImpCamera::getRasterWidth_inline		(void) const			{ return m_vQuery->getRasterWidth();  }
DPVS_FORCE_INLINE int ImpCamera::getRasterHeight_inline		(void) const			{ return m_vQuery->getRasterHeight(); }
int ImpCamera::getRasterWidth		(void) const			{ return m_vQuery->getRasterWidth();  }
int ImpCamera::getRasterHeight		(void) const			{ return m_vQuery->getRasterHeight(); }

//------------------------------------------------------------------------
// Static variables of the class
//------------------------------------------------------------------------

UINT32			ImpCamera::s_hashCounter			= 0;	
ImpCamera*		ImpCamera::s_head					= null;

/*****************************************************************************
 *
 * Function:		ImpCamera::ImpCamera()
 *
 * Description:		NOTE THAT WE CANNOT USE THE ASBESTOS PATTERN FOR THE
 *					CLASS BECAUSE WE NEED TO APPLY COPY CTORS (THE REAL CAMERA
 *					IS NOT COPIED!)
 *
 *****************************************************************************/

ImpCamera::ImpCamera	() :
	m_userFrustum(),
	m_userScissor(),
	m_prev(null),
	m_next(null),
	m_frustum(),
	m_rasterRectangle(),
	m_pixelCenter(0.5f,0.5f),
	m_clipPlaneCount(0),
	m_cameraToCell(),
	m_cellToCamera(),
	m_cellToScreen(),
	m_screenToCell(),
	m_cameraToScreen(),
	m_userCameraToScreen(),
	m_ID(++s_hashCounter),
	m_timeStamp(0),
	m_currentCell(null),
	m_viewer(),
	m_userCamera(null),
	m_vQuery(NEW<VisibilityQuery>()),
	m_allowVirtualPlane(false),
	m_ownsVisibilityQuery(true),
	m_scissorDirty(false),
	m_scissorActive(false),
	m_orthoProjection(false)
{
	//--------------------------------------------------------------------
	// Link to linked lists
	//--------------------------------------------------------------------

	InstanceCount::incInstanceCount(InstanceCount::CAMERA);
	if (s_head)
		s_head->m_prev = this;
	m_next	= s_head;
	s_head	= this;

	m_viewer.m_imp			= this;				//viewer is passed to the camera
	recomputeUserCameraToScreen();
}

/*****************************************************************************
 *
 * Function:		ImpCamera::ImpCamera(const ImpCamera&)
 *
 * Description:		copy ctor
 *
 * Parameters:
 *
 * Notes:			m_vQuery is shared among copy constructed instances
 *
 *****************************************************************************/

ImpCamera::ImpCamera	(const ImpCamera& s) :
	m_userFrustum			(s.m_userFrustum),
	m_userScissor			(s.m_userScissor),
	m_prev					(null),
	m_next					(null),
	m_frustum				(s.m_frustum),
	m_rasterRectangle		(s.m_rasterRectangle),
	m_pixelCenter			(s.m_pixelCenter),
	m_clipPlaneCount		(s.m_clipPlaneCount),
	m_cameraToCell			(s.m_cameraToCell),
	m_cellToCamera			(s.m_cellToCamera),
	m_cellToScreen			(s.m_cellToScreen),
	m_screenToCell			(s.m_screenToCell),
	m_cameraToScreen		(s.m_cameraToScreen),
	m_userCameraToScreen	(s.m_userCameraToScreen),
	m_ID					(s.m_ID + 13),
	m_timeStamp				(s.m_timeStamp),
	m_currentCell			(s.m_currentCell),
	m_viewer				(s.m_viewer),
	m_userCamera			(s.m_userCamera),
	m_vQuery				(s.m_vQuery),
	m_allowVirtualPlane		(s.m_allowVirtualPlane),
	m_ownsVisibilityQuery	(false),
	m_scissorDirty			(s.m_scissorDirty),
	m_scissorActive			(s.m_scissorActive),
	m_orthoProjection		(s.m_orthoProjection)
{
	//--------------------------------------------------------------------
	// Linked list management
	//--------------------------------------------------------------------

	InstanceCount::incInstanceCount(InstanceCount::CAMERA);

	if (s_head)
		s_head->m_prev = this;
	m_next	= s_head;
	s_head	= this;

	//--------------------------------------------------------------------
	// Copy clip planes
	//--------------------------------------------------------------------

	for (int i=0;i<MAX_CLIP_PLANES;i++) 
		m_clipPlanes[i] = s.m_clipPlanes[i];

	//--------------------------------------------------------------------
	// Viewer is passed to the camera
	//--------------------------------------------------------------------

	m_viewer.m_imp		= this;

	checkConsistency();
}

/*****************************************************************************
 *
 * Function:		ImpCamera::~ImpCamera()
 *
 * Description:		
 *
 *****************************************************************************/

ImpCamera::~ImpCamera	(void)
{
	checkConsistency();

	if(m_ownsVisibilityQuery)
	{
		setCellSoftReference(null);		// remove soft reference from cell
		DELETE(m_vQuery);
		m_vQuery = null;				// DEBUG
	}

	//--------------------------------------------------------------------
	// Linked list management
	//--------------------------------------------------------------------

	InstanceCount::decInstanceCount(InstanceCount::CAMERA);

	if (m_next)
		m_next->m_prev = m_prev;

	if (m_prev)
		m_prev->m_next = m_next;
	else
	{
		DPVS_ASSERT(s_head == this);
		s_head = m_next;
	}

	if (!s_head)
		DPVS_ASSERT(InstanceCount::getInstanceCount(InstanceCount::CAMERA)==0);
}

/*****************************************************************************
 *
 * Function:		ImpCamera::checkConsistency()
 *
 * Description:		Checks internal consistency
 *
 *****************************************************************************/

#if defined (DPVS_DEBUG)
void ImpCamera::checkConsistency (void) const
{
	DPVS_ASSERT (this);
	DPVS_ASSERT (Memory::isValidPointer(getUserCamera()));
	DPVS_ASSERT (Memory::isValidPointer(m_vQuery));
	if (getCell())
		DPVS_ASSERT (Memory::isValidPointer(getCell()));

	// TODO.. TODO..
}
#endif

/*****************************************************************************
 *
 * Function:		ImpCamera::checkConsistencyAll()
 *
 * Description:		Checks internal consistency of all ImpCameras
 *
 *****************************************************************************/

void ImpCamera::checkConsistencyAll	(void)
{
#if defined (DPVS_DEBUG)
	int cnt = 0;
	for (const ImpCamera* c = s_head; c; c = c->m_next, cnt++)
		c->checkConsistency();
	DPVS_ASSERT(InstanceCount::getInstanceCount(InstanceCount::CAMERA) == cnt);
#endif
}

/*****************************************************************************
 *
 * Function:		ImpCamera::setParameters()
 *
 * Description:		
 *
 * Parameters:
 *
 *****************************************************************************/

void ImpCamera::setParameters	(int screenWidth,int screenHeight,UINT32 properties,float imageSpaceScaleX,float imageSpaceScaleY)
{
	if(!m_vQuery->setParameters(screenWidth,screenHeight,properties,imageSpaceScaleX,imageSpaceScaleY))
	{
		m_rasterRectangle.set(0.f,(float)getRasterWidth_inline(), 0.f,(float)getRasterHeight_inline());	// full screen
		m_rasterRectangle.setSpace(SPACE_RASTER);							// defined in RASTER space
		m_userScissor = m_rasterRectangle;									// indicate as requested (user's)
		m_scissorDirty = true;												// requires validation
	}
}

/*****************************************************************************
 *
 * Function:		ImpCamera::validateScissor()
 *
 * Description:		Validates and sets scissor rectangle
 *
 * Notes:			This is called by the visibility query
 *
 *****************************************************************************/

bool ImpCamera::validateScissor()
{
//	if(!m_scissorDirty)
//		return true;

	m_frustum = m_userFrustum;			// MOVED from resolveVisiblity()

	m_scissorDirty = false;

	// set initial scissor (full screen)
	m_rasterRectangle.set (0.f,(float)getRasterWidth_inline(), 0.f,(float)getRasterHeight_inline());
	m_rasterRectangle.setSpace(SPACE_RASTER);


	FloatRectangle scissor(m_userScissor);

	if(!scissor.intersect(m_rasterRectangle))		// intersect with the raster rectangle
	{
		scissor.set(0,0,0,0);						// if out, collapse scissor
		return false;								// invalid frustum
	}


	m_scissorActive = (scissor != m_rasterRectangle);	// scissor is active only if part of the screen

	//NOTE: m_frustum and m_rasterRectangle MUST be valid
	Frustum f;
	if(!createFrustumFromRectangle(f,scissor))
		return false;


	setFrustumPlanesAndMatrix(f);
	return true;
}

// DEBUG DEBUG TODO: pre-calculate 1.0/width,1.0/height
void ImpCamera::rasterToScreen(Vector2& v) const
{
	DPVS_ASSERT(m_rasterRectangle.width());
	DPVS_ASSERT(m_rasterRectangle.height());

	v.x =  2.f * ((v.x-m_rasterRectangle.x0) / m_rasterRectangle.width()  - 0.5f);
	v.y = -2.f * ((v.y-m_rasterRectangle.y0) / m_rasterRectangle.height() - 0.5f);
}

// DEBUG DEBUG TODO: pre-calculate 1.0/width,1.0/height
void ImpCamera::rasterToScreen(Vector3& v) const
{
	DPVS_ASSERT(m_rasterRectangle.width());
	DPVS_ASSERT(m_rasterRectangle.height());
	v.x =  2.f * ((v.x-m_rasterRectangle.x0) / m_rasterRectangle.width()  - 0.5f);
	v.y = -2.f * ((v.y-m_rasterRectangle.y0) / m_rasterRectangle.height() - 0.5f);
	v.z =  2.f * v.z - 1.f;
}

/*****************************************************************************
 *
 * Function:		ImpCamera::setUserFrustum()
 *
 * Description:		
 *
 *
 *****************************************************************************/

void ImpCamera::setUserFrustum (const Frustum& f,bool ortho)	
{ 
	m_userFrustum		= f; 

	m_orthoProjection	= ortho; 
	setFrustumPlanesAndMatrix(m_userFrustum); 
}

/*****************************************************************************
 *
 * Function:		ImpCamera::createFrustumFromRectangle()
 *
 * Description:		Creates a new view frustum based on a scissor rectangle
 *
 * Notes:			m_frustum and m_rasterRectangle must be valid
 *					Called only after portal traversal, and never for the user's camera.
 *
 *					Changes m_rasterRectangle
 *
 *****************************************************************************/

bool ImpCamera::createFrustumFromRectangle(Frustum& f, const FloatRectangle &prect)
{

	//-----------------------------------------------------
	// Integer-corrected view port
	//-----------------------------------------------------

	FloatRectangle	r		= m_rasterRectangle;

	FloatRectangle portalRectangle(
		(float)Math::intFloor(prect.x0),
		(float)Math::intCeil(prect.x1),
		(float)Math::intFloor(prect.y0),
		(float)Math::intCeil(prect.y1));

	portalRectangle.setSpace(SPACE_RASTER);						//defined in raster-space

	if(!r.intersect(portalRectangle))
		return false;

	//-----------------------------------------------------
	// Create a new raster rectangle
	//-----------------------------------------------------

	Vector2 topLeft		(r.x0, r.y0);
	Vector2 bottomRight	(r.x1, r.y1);

	rasterToScreen(topLeft);									//[-1,1] USES the ORIGINAL m_rasterRectangle
	rasterToScreen(bottomRight);								//[-1,1] USES the ORIGINAL m_rasterRectangle

	m_rasterRectangle = r;										// now it's safe to override the rectangle

	//-----------------------------------------------------
	// Create new frustum
	//-----------------------------------------------------

	const Frustum&		frustum = m_frustum;

	// DEBUG DEBUG these were doubles
	float width		= (frustum.right - frustum.left) / 2;	//[0,1] @ 90' FOV
	float height	= (frustum.top - frustum.bottom) / 2;	//[0,1] @ 90' FOV
	float midx		= (frustum.left + frustum.right) / 2;	//mid
	float midy		= (frustum.bottom + frustum.top) / 2;	//mid

	f.zNear = frustum.zNear;
	f.zFar  = frustum.zFar;
	f.left	= topLeft.x * width + midx;
	f.top	= topLeft.y * height + midy;
	f.right	= bottomRight.x * width + midx;
	f.bottom= bottomRight.y * height + midy;

	return true;
}

/*****************************************************************************
 *
 * Function:		ImpCamera::setFrustumPlanesAndMatrix(const Frustum&)
 *
 * Description:		Create perspective projection matrix and clipping planes
 *					from frustum description
 *
 * Parameters:		Frustum description
 *
 * Notes:			Identical to OpenGL::glFrustum()
 *
 *****************************************************************************/

void ImpCamera::setFrustumPlanesAndMatrix (const Frustum& frustum)
{
	m_frustum		= frustum;									// take copy of the frustum structure

	//-----------------------------------------------------
	// apply pixel center correction to frustum (in DPVS's side)
	//-----------------------------------------------------

	if(getRasterWidth_inline()>0 && getRasterHeight_inline()>0)
	{
		float ooRasterWidth  = Math::reciprocal((float)getRasterWidth_inline());
		float ooRasterHeight = Math::reciprocal((float)getRasterHeight_inline());

		if(m_orthoProjection)
		{
			m_frustum.left   += m_pixelCenter.x * ooRasterWidth;
			m_frustum.right  += m_pixelCenter.x * ooRasterWidth;
			m_frustum.top    += m_pixelCenter.y * ooRasterHeight;
			m_frustum.bottom += m_pixelCenter.y * ooRasterHeight;
		}
		else
		{
			m_frustum.left   += m_pixelCenter.x * ooRasterWidth * m_frustum.zNear;
			m_frustum.right  += m_pixelCenter.x * ooRasterWidth * m_frustum.zNear;
			m_frustum.top    += m_pixelCenter.y * ooRasterHeight * m_frustum.zNear;
			m_frustum.bottom += m_pixelCenter.y * ooRasterHeight * m_frustum.zNear;
		}
	}

	//-----------------------------------------------------
	// Setup camera->screen matrix, i.e. projection
	// matrix (ortho or perspective)
	//-----------------------------------------------------

	Matrix4x4&		m = m_cameraToScreen;
	const Frustum&	f = m_frustum;								// Use our own (copied) data

	DPVS_ASSERT( f.right != f.left);
	DPVS_ASSERT( f.top   != f.bottom);
	DPVS_ASSERT( f.zFar  != f.zNear);

	// DEBUG DEBUG These were doubles
	const float ooRightLeft	= 1.f / (f.right - f.left);
	const float ooTopBottom	= 1.f / (f.top   - f.bottom);
	const float ooFarNear	= 1.f / (f.zFar  - f.zNear);
	const float near2		= 2.f * f.zNear;

	//-----------------------------------------------------
	// These are the same as in openGL glOrtho() and glFrustum()
	// except that Z-components have been negated.
	//-----------------------------------------------------

	if(m_orthoProjection)
	{
		m[0][0] = (2.f  * ooRightLeft);
		m[0][1] = 0.0f;
		m[0][2] = 0.0f;											//flipped z (0)
		m[0][3] = -((f.right+f.left) * ooRightLeft);
		m[1][0] = 0.0f;
		m[1][1] = (2.f * ooTopBottom);
		m[1][2] = 0.0f;											//flipped z (0)
		m[1][3] = -((f.top+f.bottom) * ooTopBottom);
		m[2][0] = 0.0f;
		m[2][1] = 0.0f;
		m[2][2] = (2.f * ooFarNear);						//flipped z
		m[2][3] = -((f.zFar+f.zNear) * ooFarNear);
		m[3][0] = 0.0f;
		m[3][1] = 0.0f;
		m[3][2] = 0.0f;											//flipped z (0)
		m[3][3] = 1.0f;
	}
	else
	{
		m[0][0] = (near2 * ooRightLeft);
		m[0][1] = 0.0f;
		m[0][2] = (-(f.right + f.left) * ooRightLeft);	//flipped z
		m[0][3] = 0.0f;
		m[1][0] = 0.0f;
		m[1][1] = (near2 * ooTopBottom);
		m[1][2] = (-(f.top + f.bottom) * ooTopBottom);	//flipped z
		m[1][3] = 0.0f;
		m[2][0] = 0.0f;
		m[2][1] = 0.0f;
		m[2][2] = ((f.zFar + f.zNear) * ooFarNear);		//flipped z
		m[2][3] = (-(near2* f.zFar) * ooFarNear);
		m[3][0] = 0.0f;
		m[3][1] = 0.0f;
		m[3][2] = 1.0f;											//flipped z
		m[3][3] = 0.0f;
	}

	//-----------------------------------------------------
	// Setup clipping planes (normally TOP>BOTTOM). Here
	// we need to sort the left/right, top/bottom, far/near
	// in order to get the plane directions correctly.
	//-----------------------------------------------------

	float left  = (f.left<f.right) ? f.left   : f.right;
	float right = (f.left<f.right) ? f.right  : f.left;
	float bottom= (f.bottom<f.top) ? f.bottom : f.top;
	float top	= (f.bottom<f.top) ? f.top    : f.bottom;
	float zNear = (f.zNear<f.zFar) ? f.zNear  : f.zFar;
	float zFar  = (f.zNear<f.zFar) ? f.zFar   : f.zNear;

	if(m_orthoProjection)
	{
		m_clipPlanes[ZERO].make		( 0.f, 0.f,  1.f, 0.0f);		// "zero plane"
		m_clipPlanes[FRONT].make	( 0.f, 0.f,  1.f, -zNear);
		m_clipPlanes[BACK].make		( 0.f, 0.f, -1.f,  zFar);
		m_clipPlanes[LEFT].make		( 1.f, 0.f,  0.f, -left);		// w = -normal * offset
		m_clipPlanes[RIGHT].make	(-1.f, 0.f,  0.f,  right);
		m_clipPlanes[BOTTOM].make	( 0.f, 1.f,  0.f, -bottom);
		m_clipPlanes[TOP].make		( 0.f,-1.f,  0.f,  top);
	}
	else
	{
		m_clipPlanes[ZERO].make		( 0.f, 0.f,  1.f, 0.0f);		// "zero plane"
		m_clipPlanes[FRONT].make	( 0.f, 0.f,  1.f, -zNear);
		m_clipPlanes[BACK].make		( 0.f, 0.f, -1.f,  zFar);
		m_clipPlanes[LEFT].make		( zNear, 0.f, -left, 0.f);
		m_clipPlanes[RIGHT].make	(-zNear, 0.f, right, 0.f);
		m_clipPlanes[BOTTOM].make	( 0.f, zNear, -bottom, 0.f);
		m_clipPlanes[TOP].make		( 0.f, -zNear, top, 0.f);
	}


	//--------------------------------------------------------------------
	// Setup the cell->screen matrix
	//--------------------------------------------------------------------

	setupCellToScreen();

	recomputeUserCameraToScreen();		// recompute user's camera->screen matrix

}

/*****************************************************************************
 *
 * Function:		ImpCamera::recomputeUserCameraToScreen()
 *
 * Description:		Recomputes user's camera->screen matrix
 *
 *****************************************************************************/

void ImpCamera::recomputeUserCameraToScreen (void)
{
	m_userCameraToScreen = m_cameraToScreen;

	int w = getRasterWidth_inline ();
	int h = getRasterHeight_inline();

	if(w && h)
	{
		float dx = m_userCameraToScreen[0][0]; // 2 * near / (right-left)
		float dy = m_userCameraToScreen[1][1]; // 2 * near / (top-bottom)

		float tx = m_pixelCenter.x / w * dx;		// adjust pixel center 
		float ty = m_pixelCenter.y / h * dy;

		Matrix4x4 t;
		t.setTranslation(Vector3(tx, ty, 0.f));
		m_userCameraToScreen.productFromLeft(t);

		//--------------------------------------------------------------------
		// Old code did not work correctly.
		//--------------------------------------------------------------------

/*		float tx = 2*m_pixelCenter.x / w;		// adjust pixel center
		float ty = 2*m_pixelCenter.y / h;

		m_userCameraToScreen[0][0] -= tx*m_userCameraToScreen[3][0];
		m_userCameraToScreen[1][0] -= ty*m_userCameraToScreen[3][0];
		m_userCameraToScreen[0][1] -= tx*m_userCameraToScreen[3][1];
		m_userCameraToScreen[1][1] -= ty*m_userCameraToScreen[3][1];
		m_userCameraToScreen[0][2] -= tx*m_userCameraToScreen[3][2]; 
		m_userCameraToScreen[1][2] -= ty*m_userCameraToScreen[3][2];
		m_userCameraToScreen[0][3] -= tx*m_userCameraToScreen[3][3];
		m_userCameraToScreen[1][3] -= ty*m_userCameraToScreen[3][3];
		*/
	}
}

/*****************************************************************************
 *
 * Function:		ImpCamera::setCellSoftReference(Cell*)
 *
 * Description:		
 *
 * Parameters:
 *
 * Notes:			Calls m_currentCell->removeCamera(),
 *					which calls m_cameras->remove(),
 *
 *****************************************************************************/

void ImpCamera::setCellSoftReference	(Cell* c)
{
	//--------------------------------------------------------------------
	// Check if cell is the same as the old one..
	//--------------------------------------------------------------------

	if(c==m_currentCell)
		return;

	//--------------------------------------------------------------------
	// Add to new cell and remove from old one.
	//--------------------------------------------------------------------

	Cell* oldCell = m_currentCell;
	m_currentCell = c;

	if(oldCell)
		oldCell->getImplementation()->removeCamera(m_userCamera);

	if(m_currentCell)
		m_currentCell->getImplementation()->addCamera(m_userCamera);

}

/*****************************************************************************
 *
 * Function:		ImpCamera::setCameraToCell(const Matrix4x3&)
 *
 * Description:		
 *
 * Parameters:
 *
 * Notes:			Format coversions are handles by wrapper
 *
 *****************************************************************************/

void ImpCamera::setCameraToCell(const Matrix4x3 &mtx)	//orientation,scale,translation
{

	DPVS_API_ASSERT(Math::isInvertible(mtx) && "DPVS::Camera::setCameraToCell() - matrix contains zero scale terms (is not invertible!)");

	m_cameraToCell = mtx;
	m_cameraToCell.flushToZero();						// remove underflow

	Math::invertMatrix (m_cellToCamera,m_cameraToCell);//m_cellToCamera.invert(m_cameraToCell);
	m_cellToCamera.flushToZero();						// remove underflow
	setupCellToScreen	();
}

/*****************************************************************************
 *
 * Function:		ImpCamera::setupCellToScreen()
 *
 * Description:		
 *
 *****************************************************************************/

//NOTE: camera needs to be in Cell space!
void ImpCamera::setupCellToScreen		()
{

	m_cellToScreen[0] = m_cellToCamera[0];
	m_cellToScreen[1] = m_cellToCamera[1];
	m_cellToScreen[2] = m_cellToCamera[2];
	m_cellToScreen[3].make(0.0f,0.0f,0.0f,1.0f);


	m_cellToScreen.productFromLeft(m_cameraToScreen);
	m_cellToScreen.flushToZero();						// remove underflow

	Math::invertMatrix(m_screenToCell,m_cellToScreen);	//	m_screenToCell.invert(m_cellToScreen);
	m_screenToCell.flushToZero();						// remove underflow
}

/*****************************************************************************
 *
 * Function:		ImpCamera::getBackProjectionShaft()
 *
 * Description:		Returns back-projection shaft of the camera
 *
 * Parameters:		aabb = reference to AABB where the shaft is stored
 *
 *****************************************************************************/

void ImpCamera::getBackProjectionShaft (AABB& aabb) const
{
	//--------------------------------------------------------------------
	// We need this little flipping here in order to properly support
	// view frustums that have flipped left/right values
	//--------------------------------------------------------------------

	float left   = m_frustum.left;
	float right  = m_frustum.right;
	float top    = m_frustum.top;
	float bottom = m_frustum.bottom;

	if (left > right)
		swap(left,right);

	if (bottom > top)
		swap(bottom,top);

	aabb.setMin(Vector3(left , bottom, m_frustum.zNear));
	aabb.setMax(Vector3(right, top  ,  m_frustum.zNear));
}

/*****************************************************************************
 *
 * Function:		ImpCamera::debugDrawBox3D(const AABB&)
 *
 * Description:		
 *
 * Parameters:
 *
 *****************************************************************************/

void ImpCamera::debugDrawBox3D (Library::LineType type, const AABB& cellSpaceBounds, const Vector4& col)
{
	const Matrix4x3& cellToWorld = getCell()->getImplementation()->getCellToWorld();
	
	Vector3 min = cellToWorld.transform(cellSpaceBounds.getMin());
	Vector3 max = cellToWorld.transform(cellSpaceBounds.getMax());

	Debug::drawLine(type, Vector3(min.x, min.y, min.z), Vector3(max.x, min.y, min.z), col);
	Debug::drawLine(type, Vector3(min.x, min.y, min.z), Vector3(min.x, max.y, min.z), col);
	Debug::drawLine(type, Vector3(min.x, max.y, min.z), Vector3(max.x, max.y, min.z), col);
	Debug::drawLine(type, Vector3(max.x, max.y, min.z), Vector3(max.x, min.y, min.z), col);
	Debug::drawLine(type, Vector3(min.x, min.y, max.z), Vector3(max.x, min.y, max.z), col);
	Debug::drawLine(type, Vector3(min.x, min.y, max.z), Vector3(min.x, max.y, max.z), col);
	Debug::drawLine(type, Vector3(min.x, max.y, max.z), Vector3(max.x, max.y, max.z), col);
	Debug::drawLine(type, Vector3(max.x, max.y, max.z), Vector3(max.x, min.y, max.z), col);
	Debug::drawLine(type, Vector3(min.x, min.y, min.z), Vector3(min.x, min.y, max.z), col);
	Debug::drawLine(type, Vector3(max.x, min.y, min.z), Vector3(max.x, min.y, max.z), col);
	Debug::drawLine(type, Vector3(min.x, max.y, min.z), Vector3(min.x, max.y, max.z), col);
	Debug::drawLine(type, Vector3(max.x, max.y, min.z), Vector3(max.x, max.y, max.z), col);
}

/*****************************************************************************
 *
 * Function:		ImpCamera::debugDrawLine2D()
 *
 * Description:		
 *
 * Parameters:
 *
 *****************************************************************************/

void ImpCamera::debugDrawLine2D(Library::LineType type, const Vector2& a, const Vector2& b, const Vector4& col)
{
	Vector2 c(a);
	Vector2 d(b);
	screenToRaster(c);
	screenToRaster(d);
	Debug::drawLine(type, c,d,col);
}

/*****************************************************************************
 *
 * Function:		ImpCamera::debugDrawLine3D()
 *
 * Description:		
 *
 * Parameters:
 *
 *****************************************************************************/

void ImpCamera::debugDrawLine3D(Library::LineType type, const Vector3& a, const Vector3& b, const Vector4& col)
{
	const Matrix4x3& cellToWorld = getCell()->getImplementation()->getCellToWorld();
	Vector3 aa = cellToWorld.transform(a);
	Vector3 bb = cellToWorld.transform(b);
	Debug::drawLine(type, Vector3(aa.x, aa.y, aa.z), Vector3(bb.x, bb.y, bb.z), col);
}

/*****************************************************************************
 *
 * Function:		ImpCamera::minimizeMemoryUsage()
 *
 * Description:		Minimizes memory footprint of the camera
 *
 * Parameters:
 *
 *****************************************************************************/

void ImpCamera::minimizeMemoryUsage	(void)
{
	for (ImpCamera* c = s_head; c; c = c->m_next)
		c->m_vQuery->minimizeMemoryUsage();
}

//------------------------------------------------------------------------

