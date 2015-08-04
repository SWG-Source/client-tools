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
 * Description: 	Wrapper code and documentation for the entire public API
 *
 * $Archive: /dpvs/implementation/sources/dpvsWrapper.cpp $
 * $Author: otso $ 
 * $Revision: #8 $
 * $Modtime: 8.10.02 14:59 $
 * $Date: 2004/11/09 $
 * 
 ******************************************************************************/

#include "dpvsWrapper.hpp"
#include "dpvsImpCamera.hpp"
#include "dpvsVisibilityQuery.hpp"
#include "dpvsImpCell.hpp"
#include "dpvsImpObject.hpp"
#include "dpvsImpCommander.hpp"
#include "dpvsImpMeshModel.hpp"
#include "dpvsImpMiscModel.hpp"
#include "dpvsSystem.hpp"
#include "dpvsVersion.hpp"
#include "dpvsDebug.hpp"
#include "dpvsMatrixConverter.hpp"
#include "dpvsRange.hpp"
#include "dpvsSweepAndPrune.hpp"
#include "dpvsImpRegionOfInfluence.hpp"
#include "dpvsImpPhysicalPortal.hpp"
#include "dpvsImpVirtualPortal.hpp"
#include "dpvsMemory.hpp"
#include "dpvsBounds.hpp"
#include "dpvsScratchpad.hpp"
#include "dpvsTempAllocator.hpp"
#include "dpvsSurface.hpp"
#include "dpvsDatabase.hpp" // DEBUG DEBUG DEBUG JUST FOR MEM INFO
#include "dpvsEvaluation.hpp"
#include "dpvsRemoteDebugger.hpp"

#if defined (DPVS_MT)
#	include "dpvsMT.hpp"
#	define DPVS_MUTEX_LOCK Lock lock;
#else
#	define DPVS_MUTEX_LOCK
#endif


#include <cstdio>		// for sprintf()
#include <string.h>		// DEBUG DEBUG
#include <stdlib.h>		// DEBUG DEBUG
#include <stdio.h>		// DEBUG DEBUG

namespace DPVS
{
    void*           kake;
	extern "C"
	{
		void*      g_invalidPointer  = (void*)(&kake); // whatever...
	}

	bool			g_libraryInitialized	= false;

	//--------------------------------------------------------------------
	// Make sure that sizes of certain variables are correct (as we make
	// internal assumptions on these sizes)
	//--------------------------------------------------------------------

	DPVS_CT_ASSERT (sizeof(UINT32) == 4);
	DPVS_CT_ASSERT (sizeof(INT32)  == 4);
	DPVS_CT_ASSERT (sizeof(float)  == 4);
	DPVS_CT_ASSERT (sizeof(double) == 8);
	DPVS_CT_ASSERT (sizeof(Matrix4x4) == (16*sizeof(float)));	// some routines depend on this!
}

using namespace DPVS;

/******************************************************************************
 *
 * Enum:			DPVS::Camera::Property
 *
 * Description:		Enumeration of culling methods used
 *
 * Notes:			These enumerations are used when constructing the
 *					property mask for the API calls DPVS::Camera::setParameters()
 *					and DPVS::Camera::getProperties()
 *
 *****************************************************************************/

/*****************************************************************************
 *
 * Function:		DPVS::Camera::setPixelCenter()
 *
 * Description:		Defines pixel center used by the rasterizer
 *
 * Parameters:		x = pixel center x offset in pixels [-1,+1]
 *					y = pixel center y offset in pixels [-1,+1]
 *
 * Notes:			In order to synchronize the internal rasterization
 *					of DPVS with the external rasterizer used to draw
 *					the image, the pixel center convention must
 *					match. If the pixel center is not the same for the
 *					two rasterizers, minor artifacts will occur, and small
 *					objects may be incorrectly culled.
 *
 *					DPVS uses (0.5,0.5) as its default pixel center. 
 *					This corresponds with the convention used by most 
 *					rendering APIs, including OpenGL. If your rasterizer
 *					uses a different convention, this function can be used
 *					to define an alternative pixel center.
 *
 * See Also:		Camera::getPixelCenter()
 *
 *****************************************************************************/

void Camera::setPixelCenter	(float x,float y)
{
	DPVS_MUTEX_LOCK
	DPVS_ASSERT_INIT();
	DPVS_ASSERT_NO_VISIBILITY_QUERY();
	DPVS_ASSERT_SELF_OK();


	DPVS_API_ASSERT ( x>=-1.0f && x <= +1.0f && "Camera::setPixelCenter() - parameter 'x' out of range");
	DPVS_API_ASSERT ( y>=-1.0f && y <= +1.0f && "Camera::setPixelCenter() - parameter 'y' out of range");

	Math::clamp (x, -1.0f, +1.0f);
	Math::clamp (y, -1.0f, +1.0f);

	m_imp->setPixelCenter(x,y);
}

/*****************************************************************************
 *
 * Function:		DPVS::Camera::getPixelCenter()
 *
 * Description:		Returns pixel center used by the rasterizer
 *
 * Parameters:		x = reference to double where pixel x-offset is stored
 *					y = reference to double where pixel y-offset is stored
 *
 * Notes:			The default value for the pixel center is [0.5,0.5].
 *
 * See Also:		Camera::setPixelCenter()
 *
 *****************************************************************************/

void Camera::getPixelCenter	(float& x,float& y) const
{
	DPVS_MUTEX_LOCK
	DPVS_ASSERT_INIT();
	DPVS_ASSERT_SELF_OK();
	
	float fx,fy;
	m_imp->getPixelCenter(fx,fy);
	x = fx;
	y = fy;
}

/*****************************************************************************
 *
 * Function:		DPVS::Camera::setParameters()
 *
 * Description:		Defines resolution of the screen, culling methods used and some associated parameters
 *
 * Parameters:		screenWidth		= width of screen in pixels [0,16384]
 *					screenHeight	= height of screen in pixels [0,16384]
 *					properties		= bitmask of property enumerations (see DPVS::Camera::Property)
 *					imageSpaceScaleX= image space subsampling horizontal factor (defaults to 1.0)
 *					imageSpaceScaleY= image space subsampling vertical factor (defaults to 1.0)
 *
 * Notes:			The property enumeration mask is used during subsequent
 *					DPVS::Camera::resolveVisibility() calls to determine what
 *					culling methods are used.
 *
 *					This function performs expensive memory reallocations. Do not
 *					call the function if parameters aren't really changing (i.e. don't
 *					put the call into the main per-frame rendering loop).
 *
 *					If OCCLUSION_CULLING flag is set in the properties, the VIEWFRUSTUM_CULLING
 *					flag will be automatically set as well. 
 *
 *					Note that if a static coverage mask or a static z-buffer is being
 *					used, it must be redefined to the camera after this call.
 *
 * See Also:		DPVS::Camera::getWidth(),DPVS::Camera::getHeight(),DPVS::Camera::setObjectMinimumCoverage()
 *
 *****************************************************************************/

void Camera::setParameters	(int screenWidth,int screenHeight,unsigned int properties,float imageSpaceScaleX,float imageSpaceScaleY)	
{ 
	DPVS_MUTEX_LOCK
	DPVS_ASSERT_INIT();
	DPVS_ASSERT_NO_VISIBILITY_QUERY();
	DPVS_ASSERT_SELF_OK();

	DPVS_API_ASSERT ("Camera::setParameters()" && screenWidth >= 0);
	DPVS_API_ASSERT ("Camera::setParameters()" && screenWidth <= 16384);
	DPVS_API_ASSERT ("Camera::setParameters()" && screenHeight >= 0);
	DPVS_API_ASSERT ("Camera::setParameters()" && screenHeight <= 16384);
	DPVS_API_ASSERT ("Camera::setParameters()" && imageSpaceScaleX > 0);
	DPVS_API_ASSERT ("Camera::setParameters()" && imageSpaceScaleY > 0);

	Math::clamp (screenWidth, 0,16384);
	Math::clamp (screenHeight,0,16384);
	Math::clamp (imageSpaceScaleX,0.001f,1000.0f);
	Math::clamp (imageSpaceScaleY,0.001f,1000.0f);

	m_imp->setParameters(screenWidth,screenHeight,properties,imageSpaceScaleX,imageSpaceScaleY); 
}

/*****************************************************************************
 *
 * Function:		DPVS::Camera::setScissor ()
 *
 * Description:		Sets a new scissor rectangle
 *
 * Parameters:		left   = smallest x-coordinate of an axis-aligned rectangle (inclusive)
 *					top    = smallest y-coordinate of an axis-aligned rectangle (inclusive)
 *					right  = largest x-coordinate of an axis-aligned rectangle (exclusive)
 *					bottom = largest y-coordinate of an axis-aligned rectangle (exclusive)
 *
 * Notes:			The coordinates returned are in output resolution scale.
 *
 * See Also:		DPVS::Camera::setParameters()
 *
 *****************************************************************************/

void Camera::setScissor		(int left,int top,int right,int bottom)
{
	DPVS_MUTEX_LOCK
	DPVS_ASSERT_INIT();
	DPVS_ASSERT_NO_VISIBILITY_QUERY();
	DPVS_ASSERT_SELF_OK();

	//--------------------------------------------------------------------
	// Assert in debug build, validate in release build
	//--------------------------------------------------------------------
	
	DPVS_API_ASSERT("Camera::setScissor()" && left<=right);
	DPVS_API_ASSERT("Camera::setScissor()" && top<=bottom);
	if (left > right)  left = right;
	if (top  > bottom) top  = bottom;

	FloatRectangle r;
	r.x0 = (float)left;
	r.x1 = (float)right;
	r.y0 = (float)top;
	r.y1 = (float)bottom;

	m_imp->setUserScissor(r);	// cannot validate the scissor here (frustum may change etc)
}

/*****************************************************************************
 *
 * Function:		DPVS::Camera::getScissor ()
 *
 * Description:		Returns current scissor rectangle
 *
 * Parameters:		left   = smallest x-coordinate of an axis-aligned rectangle (inclusive)
 *					top    = smallest y-coordinate of an axis-aligned rectangle (inclusive)
 *					right  = largest x-coordinate of an axis-aligned rectangle (exclusive)
 *					bottom = largest y-coordinate of an axis-aligned rectangle (exclusive)
 *
 * Notes:			Requested scissor is clamped to screen resolution in resolveVisibility().
 *					If resolveVisibility() has not been called, the scissor returned is
 *					the one requested.
 *
 * See Also:		DPVS::Camera::setParameters()
 *
 *****************************************************************************/

void Camera::getScissor		(int& left,int& top,int& right,int& bottom) const
{
	DPVS_MUTEX_LOCK
	DPVS_ASSERT_INIT();
	DPVS_ASSERT_SELF_OK();
	const FloatRectangle &r = m_imp->getScissor();

	// DEBUG DEBUG OPTIMIZE: COULDN'T WE USE INTCHOP INSTEAD (AS THESE VALUES SHOULD ALWAYS BE >=0)
	left	= Math::intFloor(r.x0);
	top		= Math::intFloor(r.y0);
	right	= Math::intFloor(r.x1);
	bottom	= Math::intFloor(r.y1);
}

/*****************************************************************************
 *
 * Function:		DPVS::Camera::getHeight()
 *
 * Description:		Returns height of the screen
 *
 * Returns:			height of the screen in pixels
 *
 * Notes:			The screen width and height have been determined
 *					by the previous call to DPVS::Camera::setParameters()
 *
 * See Also:		DPVS::Camera::setParameters(), DPVS::Camera::getWidth()
 *
 *****************************************************************************/

int	Camera::getHeight (void) const			
{ 
	DPVS_MUTEX_LOCK
	DPVS_ASSERT_INIT();
	DPVS_ASSERT_SELF_OK();
	return m_imp->getRasterHeight();
}

/*****************************************************************************
 *
 * Function:		DPVS::Camera::getWidth()
 *
 * Description:		Returns width of the screen
 *
 * Returns:			width of the screen in pixels
 *
 * Notes:			The screen width and height have been determined
 *					by the previous call to DPVS::Camera::setParameters()
 *
 * See Also:		DPVS::Camera::setParameters(), DPVS::Camera::getHeight()
 *
 *****************************************************************************/

int	Camera::getWidth (void) const			
{ 
	DPVS_MUTEX_LOCK
	DPVS_ASSERT_INIT();
	DPVS_ASSERT_SELF_OK();
	return m_imp->getRasterWidth();
}

/*****************************************************************************
 *
 * Function:		DPVS::Camera::getProperties()
 *
 * Description:		Returns property mask of the camera
 *
 * Returns:			bitmask containing culling properties of the camera
 *
 * Notes:			The property mask has been defined by the previous
 *					call to DPVS::Camera::setParameters()
 *
 * See Also:		DPVS::Camera::Property, DPVS::Camera::setParameters()
 *
 *****************************************************************************/

unsigned int Camera::getProperties	(void) const
{
	DPVS_MUTEX_LOCK
	DPVS_ASSERT_INIT();
	DPVS_ASSERT_SELF_OK();
	VisibilityQuery* vq = m_imp->getVisibilityQuery();
	return vq->getProperties();	
}

/*****************************************************************************
 *
 * Function:		DPVS::Camera::setFrustum ()
 *
 * Description:		Defines view frustum used by subsequent visibility queries
 *
 * Parameters:		f = reference to frustum structure
 *
 * Notes:			The near and far frustum values must be greater
 *					than zero. This is asserted in the debug build.
 *
 *****************************************************************************/

void Camera::setFrustum (const Frustum& f)		
{ 
	DPVS_MUTEX_LOCK
	DPVS_ASSERT_INIT();
	DPVS_ASSERT_NO_VISIBILITY_QUERY();
	DPVS_ASSERT_SELF_OK();

	Frustum frustum(f);	// take a copy so that we can clamp some values
	
	DPVS_API_ASSERT		("Camera::setFrustum()" && frustum.zNear >= 1e-10f && frustum.zNear <= 1e20f);
	DPVS_API_ASSERT		("Camera::setFrustum()" && frustum.zFar  >= 1e-10f && frustum.zFar  <= 1e20f);
	DPVS_API_ASSERT		("Camera::setFrustum()" && frustum.zNear < frustum.zFar);
	DPVS_API_ASSERT		("Camera::setFrustum()" && frustum.left != frustum.right);
	DPVS_API_ASSERT		("Camera::setFrustum()" && frustum.top  != frustum.bottom);

	Math::clamp (frustum.zNear, 1e-10f,1e20f);
	Math::clamp (frustum.zFar,  1e-10f,1e20f);

	DPVS_API_ASSERT ("Camera::setFrustum()" && frustum.zNear < frustum.zFar);

	m_imp->setUserFrustum	(frustum,frustum.type == Frustum::PERSPECTIVE ? false : true);
}

/*****************************************************************************
 *
 * Function:		DPVS::Camera::setObjectMinimumCoverage	()
 *
 * Description:		Defines object minimum coverage values
 *
 * Parameters:		pixelWidth	= minimum width in pixels (zero or greater)
 *					pixelHeight = minimum height in pixels (zero or greater)
 *					opacity		=
 *
 * Notes:			DPVS reserves the right to cull these object, but doesn't
 *					promise to do so. In practise this feature is effective when
 *					appropriate intermediate results are available. This policy
 *					was chosen not to decrease performance in any case.
 *
 *					The initial minimum coverage values are both zero, i.e.
 *					no contribution culling is performed by default.
 *
 *					The width and height values must be zero or greater. Negative
 *					values are asserted in the debug build.
 *
 * See Also:		DPVS::Camera::getObjectMinimumCoverage	()
 *
 *****************************************************************************/

void Camera::setObjectMinimumCoverage	(float pixelWidth,float pixelHeight,float opacity)
{
	DPVS_MUTEX_LOCK
	DPVS_ASSERT_INIT();
	DPVS_ASSERT_NO_VISIBILITY_QUERY();
	DPVS_ASSERT_SELF_OK();
	DPVS_API_ASSERT("Camera::setObjectMinimumCoverage()" && pixelWidth>=0.f);
	DPVS_API_ASSERT("Camera::setObjectMinimumCoverage()" && pixelHeight>=0.f);

	Math::clamp (opacity, 0.0f, 1.0f);

	m_imp->getVisibilityQuery()->setObjectMinimumCoverage(pixelWidth,pixelHeight,opacity);
}


/*****************************************************************************
 *
 * Function:		DPVS::Camera::getObjectMinimumCoverage	()
 *
 * Description:		Returns minimum object coverage values
 *
 * Parameters:		w = reference to float where to store horizontal min. coverage value
 *					h = reference to float where to store vertical min. coverage value
 *
 * Notes:			The minimum coverage values are initialize to zero in the
 *					camera constructor, i.e. no contribution culling is perfomed
 *					by default.
 *
 * See Also:		DPVS::Camera::setObjectMinimumCoverage()
 *
 *****************************************************************************/

void Camera::getObjectMinimumCoverage	(float &w,float &h,float &opacity) const
{
	DPVS_MUTEX_LOCK
	DPVS_ASSERT_INIT();
	DPVS_ASSERT_SELF_OK();

	VisibilityQuery* vq = m_imp->getVisibilityQuery();

	w		= vq->getObjectMinimumCoverage(0);
	h		= vq->getObjectMinimumCoverage(1);
	opacity	= vq->getObjectMinimumCoverage(2);
}

/*****************************************************************************
 *
 * Function:		DPVS::Camera::getFrustum ()
 *
 * Description:		Returns current view frustum
 *
 * Parameters:		f = reference to view frustum structure
 *
 * Notes:			The view frustum should be defined with a call
 *					to DPVS::Camera::setFrustum().
 *
 * See Also:		DPVS::Camera::setFrustum()
 *
 *****************************************************************************/

void Camera::getFrustum (Frustum &f) const			
{ 
	DPVS_MUTEX_LOCK
	DPVS_ASSERT_INIT();
	DPVS_ASSERT(&f);
	DPVS_ASSERT_SELF_OK();
	f = m_imp->getUserFrustum();
}

/*****************************************************************************
 *
 * Function:		DPVS::Camera::resolveVisibility()
 *
 * Description:		Performs a visibility query using the camera's viewing/culling parameters
 *
 * Parameters:		c					= pointer to DPVS::Commander object (non-null)
 *					recursionDepth		= maximum portal recursion depth (0 or greater)
 *					importanceThreshold = allow culling of portals with smaller cumulative importance than the threshold (set to 0.0 to disable such culling)
 *
 * Notes:			Visible objects, regions of influence etc. are informed
 *					using the DPVS::Commander functions.
 *
 *					If no recursive portals (i.e. mirrors) are used, use value
 *					1 for the recursion depth. 
 *
 *					In debug build assertions are made that the commander
 *					object is non-null and recursion depth is >= 1.
 *
 *					The camera must belong to some cell in order for this function
 *					to do anything (traversal is started from that cell).
 *
 *					The camera's properties determine which culling methods
 *					are used during the traversal.
 *
 * See Also:		DPVS::Commander, DPVS::Camera::setParameters()
 *
 *****************************************************************************/

void Camera::resolveVisibility (class Commander* com,int recursionDepth,float importanceThreshold) const
{ 
	DPVS_MUTEX_LOCK
	DPVS_ASSERT_INIT();
	DPVS_ASSERT_NO_VISIBILITY_QUERY();
	DPVS_ASSERT_SELF_OK();
	DPVS_API_ASSERT(this && this->m_imp && "Camera::resolveVisibility() - null camera!!!");
	DPVS_API_ASSERT(com && "Camera::resolveVisibility() - Commander pointer must not be null!");
	DPVS_API_ASSERT(getCell() && "Camera::resolveVisibility() - Camera is not placed into any cell!");
	DPVS_API_ASSERT("Camera::resolveVisibility()" && recursionDepth>=0);
	DPVS_API_ASSERT("Camera::resolveVisibility()" && importanceThreshold>=0.0 && importanceThreshold<=1.0);

	#if defined (DPVS_DEBUG)
		UINT32 oldFPUMode = getFPUMode();
	#endif

	if (com && getCell() && recursionDepth>=0)
	{
		Scratchpad::push();
		VQData::get().invalidate();
		Math::clamp (importanceThreshold,0.0f,1.0f);
		m_imp->getVisibilityQuery()->resolveVisibility(com,m_imp,recursionDepth,importanceThreshold);
		Scratchpad::pop();
	}

#if defined (DPVS_USE_REMOTE_DEBUGGER)
	{	
		RemoteDebuggerWrapper::update();
	}
#endif 

	DPVS_ASSERT (getFPUMode() == oldFPUMode && "Somewhere, somehow the FPU rounding mode has changed during the query!!");
}

/*****************************************************************************
 *
 * Function:		DPVS::Camera::isPointVisible(const Vector3& xyz)
 *
 * Description:		Tests whether specified 3D point is visible 
 *
 * Parameters:		xyz = point in raster space
 *
 * Notes:			The query is performed against the internal state of
 *					the previous DPVS::Camera::resolveVisibility() call. 
 *					If database has been modified in any way after the call 
 *					(by moving any objects or the camera), the results of 
 *					the query can be wrong.
 *
 *					The point's coordinates are in raster space, i.e.
 *					the X and Y coordinates should be in range [0,screen width] and
 *					and [0, screen height]. The Z coordinate should be in range
 *					[0,1] (where 0 corresponds to the front clip plane and
 *					1 to the far clip plane).
 *
 * See Also:		DPVS::Camera::resolveVisibility()
 *
 *****************************************************************************/

bool Camera::isPointVisible	(const Vector3& xyz) const
{
	DPVS_MUTEX_LOCK
	DPVS_ASSERT_INIT();
	DPVS_ASSERT_SELF_OK();
	VisibilityQuery* vq = m_imp->getVisibilityQuery();
	return vq->isRasterPointVisible_IM(m_imp->getRasterViewport(),xyz);
}

/*****************************************************************************
 *
 * Function:		DPVS::Camera::isRectangleVisible(float xmin,float ymin,float xmax,float ymax, float zmax)
 *
 * Description:		Tests whether specified rectangle is visible
 *
 * Parameters:		xmin = rectangle left   corner in screen pixels
 *					ymin = rectangle top    corner in screen pixels
 *					xmax = rectangle right  corner in screen pixels
 *					ymax = rectangle bottom corner in screen pixels
 *					zmax = rectangle z value in screen space [0,1]
 *
 * Notes:			The query is performed against the internal state of
 *					the previous DPVS::Camera::resolveVisibility() call. 
 *					If database has been modified in any way after the call 
 *					(by moving any objects or the camera), the results of 
 *					the query can be wrong.
 *
 *					The rectangle's coordinates are in raster space, i.e.
 *					the X and Y coordinates should be in range [0,screen width] and
 *					and [0, screen height]. The Z coordinate should be in range
 *					[0,1] (where 0 corresponds to the front clip plane and
 *					1 to the far clip plane).
 *
 * See Also:		DPVS::Camera::isPointVisible()
 *
 *****************************************************************************/

bool Camera::isRectangleVisible	(float xmin,float ymin,float xmax,float ymax, float zmax) const
{
	DPVS_MUTEX_LOCK
	DPVS_ASSERT_INIT();
	DPVS_ASSERT_SELF_OK();

	FloatRectangle r(xmin,xmax, ymin,ymax);
	r.setSpace (SPACE_RASTER);

	return m_imp->getVisibilityQuery()->isRectangleVisible_IM(m_imp->getRasterViewport(),r,zmax);
}

/*****************************************************************************
 *
 * Function:		DPVS::Camera::setCell()
 *
 * Description:		Sets camera into specified cell
 *	
 * Parameters:		c = pointer to cell (may be null)
 *
 * Notes:			The camera must be set into some (non-null) cell before it can be
 *					used for visibility queries
 *
 * See Also:		DPVS::Camera::getCell()
 *
 *****************************************************************************/

void Camera::setCell (Cell* c)				
{ 
	DPVS_MUTEX_LOCK
	DPVS_ASSERT_INIT();
	DPVS_ASSERT_NO_VISIBILITY_QUERY();
	DPVS_ASSERT_SELF_OK();
	DPVS_ASSERT(c == 0 || Memory::isValidPointer(c));
	m_imp->setCellSoftReference(c); 
}

/*****************************************************************************
 *
 * Function:		DPVS::Camera::getCell()
 *
 * Description:		Returns pointer to cell where camera resides
 *
 * Returns:			pointer to camera's cell (may be null)
 *
 * See Also:		DPVS::Camera::setCell()
 *
 *****************************************************************************/

Cell* Camera::getCell (void) const			
{ 
	DPVS_MUTEX_LOCK
	DPVS_ASSERT_INIT();
	DPVS_ASSERT_SELF_OK();
	return m_imp->getCell(); 
}

/*****************************************************************************
 *
 * Function:		DPVS::Camera::setStaticCoverageMask()
 *
 * Description:		Defines a static coverage mask used	to mask out portions of the screen.
 *
 * Parameters:		buffer  = pointer to byte array, where value!=0 indicates covered (submitting null indicates no static coverage buffer)
 *					w		= width of the mask buffer in pixels
 *					h		= height of the mask buffer in pixels
 *					pitch   = pitch of the buffer in bytes
 *
 * Notes:			If parts of the screen are constantly being covered by HUD etc.,
 *					it's a good idea to hint the occlusion subsystem to take this
 *					into account. Convention is similar to using stencil write masks.
 *					Bytes with values other than zero are considered to be covered.
 *					
 *					Internally a copy of input buffer is taken and resampled
 *					to match current internal resolution. This function must be
 *					called whenever the viewport is resized.
 *
 *					The function asserts in debug build if width or height are negative.
 *
 *****************************************************************************/

void Camera::setStaticCoverageMask	(const unsigned char* buffer,int w,int h, int pitch)
{ 
	DPVS_MUTEX_LOCK
	DPVS_ASSERT_INIT();
	DPVS_ASSERT_NO_VISIBILITY_QUERY();
	DPVS_ASSERT_SELF_OK();
	DPVS_API_ASSERT ("Camera::setStaticCoverageMask()" && w >= 0 && h >= 0);
	m_imp->getVisibilityQuery()->setStaticCoverageMask(buffer,w,h,pitch); 
}

/*****************************************************************************
 *
 * Function:		DPVS::Camera::setStaticZBuffer()
 *
 * Description:		Defines a static depth buffer used to mask out portions of the screen.
 *
 * Parameters:		buffer  = pointer to depth buffer (submitting null indicates no static coverage buffer).
 *					w		= width of the buffer in pixels
 *					h		= height of the buffer in pixels
 *					pitch   = pitch of the buffer in bytes
 *					farValue= value indicating nothing has been written into the depth buffer
 *
 * Notes:			Applications with static backgroud can mix dynamic occlusions by
 *					submitting the pre-rendered depth buffer to the system.
 *					Consult User's Manual for description of depth data format.
 *
 *					Internally a copy of input buffer is taken and conservatively resampled to
 *					match current internal resolution. This function must be
 *					re-called whenever the viewport is resized.
 *
 *					The function asserts in debug build if width or height are negative.
 *
 *****************************************************************************/

void Camera::setStaticZBuffer (const float* buffer,int w, int h, int pitch,float farValue)
{ 
	DPVS_MUTEX_LOCK
	DPVS_ASSERT_INIT();
	DPVS_ASSERT_NO_VISIBILITY_QUERY();
	DPVS_ASSERT_SELF_OK();
	DPVS_API_ASSERT ("Camera::setStaticZBuffer()" && w >= 0 && h >= 0);
	m_imp->getVisibilityQuery()->setStaticZBuffer(buffer,w,h,pitch,farValue); 
}	

/*****************************************************************************
 *
 * Enum:			DPVS::Commander::Viewer::Handedness
 *
 * Description:		Enumeration specifying handedness of matrix returned
 *
 * Notes:			This enumeration is used as a parameter to some DPVS::Viewer 
 *					member functions to specify the handedness of the matrix
 *					returned.
 *
 *****************************************************************************/

/*****************************************************************************
 *
 * Function:		DPVS::Commander::Viewer::getProjectionMatrix (Matrix4x4& mtx, Handedness h) const
 *
 * Description:		Returns current projection matrix
 *
 * Parameters:		mtx = reference to matrix structure (single-precision) where result is stored
 *					h   = handedness of the matrix
 *
 * Notes:			The matrix format used has been defined in the call to
 *					DPVS::Library::init().
 *
 *****************************************************************************/

void Commander::Viewer::getProjectionMatrix	(Matrix4x4& mtx, Handedness h) const
{
	DPVS_ASSERT_INIT	();
	DPVS_API_ASSERT	("Commander::Viewer::getProjectionMatrix()" && this);
	DPVS_ASSERT		(m_imp);

	mtx = m_imp->getUserCameraToScreen();		// pixel center has been corrected

	/* \todo [otso] use setDof(), getDof() ? How about the fourth component in the column.. */
	if(h!=LEFT_HANDED)
		mtx.setColumn(2, -mtx.getColumn(2));	// invert dof

	MatrixConverter::convertMatrix(mtx);		// column/row major
}

/*****************************************************************************
 *
 * Function:		DPVS::Commander::Viewer::getProjectionMatrix(Matrix4x4d& mtx, Handedness h) const
 *
 * Description:		Returns projection matrix
 *
 * Parameters:		mtx = reference to matrix structure (double-precision) where result is stored
 *					h   = handedness of the matrix
 *
 * Notes:			The matrix format used has been defined in the call to
 *					DPVS::Library::init().
 *
 *****************************************************************************/

void Commander::Viewer::getProjectionMatrix	(Matrix4x4d& mtx, Handedness h) const
{
	DPVS_ASSERT_INIT	();
	DPVS_API_ASSERT	("Commander::Viewer::getProjectionMatrix()" && this);
	DPVS_ASSERT		(m_imp);

	Matrix4x4 tmp(NO_CONSTRUCTOR);
	tmp = m_imp->getUserCameraToScreen();			// pixel center has been corrected

	if(h!=LEFT_HANDED)
		tmp.setColumn(2, -tmp.getColumn(2));		// invert dof

	MatrixConverter::convertMatrix(mtx,tmp);		// float->double, column/row major
}

/*****************************************************************************
 *
 * Function:		DPVS::Commander::Viewer::getCameraToWorldMatrix(Matrix4x4d &mtx) const
 *
 * Description:		Returns camera->world transformation matrix
 *
 * Parameters:		mtx = reference to matrix structure (single-precision) where result is stored
 *
 * Notes:			This matrix is not needed for the actual rendering. It is
 *					supplied for debug visualization purposes.
 *
 *					The matrix format used has been defined in the call to
 *					DPVS::Library::init().
 *
 *****************************************************************************/
							
void Commander::Viewer::getCameraToWorldMatrix		(Matrix4x4& mtx) const		//for debug visualization. CameraToCell * CellToWorld
{
	DPVS_ASSERT_INIT();
	DPVS_API_ASSERT ("Commander::Viewer::getCameraToWorldMatrix()" && this);
	DPVS_ASSERT (m_imp);

	Matrix4x3 tmp(m_imp->getCameraToCell());
	if(m_imp->getCell())					// if no cell is defined, consider it identity
		tmp.productFromLeft(m_imp->getCell()->getImplementation()->getCellToWorld());
	MatrixConverter::convertMatrix(mtx, tmp);
}

/*****************************************************************************
 *
 * Function:		DPVS::Commander::Viewer::getCameraToWorldMatrix(Matrix4x4d &mtx) const
 *
 * Description:		Returns camera->world transformation matrix
 *
 * Parameters:		mtx = reference to matrix structure (double-precision) where result is stored
 *
 * Notes:			This matrix is not needed for the actual rendering. It is
 *					supplied for debug visualization purposes.
 *
 *					The matrix format used has been defined in the call to
 *					DPVS::Library::init().
 *
 *****************************************************************************/

void Commander::Viewer::getCameraToWorldMatrix		(Matrix4x4d& mtx) const		//for debug visualization. CameraToCell * CellToWorld
{
	DPVS_ASSERT_INIT();
	DPVS_API_ASSERT ("Commander::Viewer::getCameraToWorldMatrix()" && this);
	DPVS_ASSERT (m_imp);

	Matrix4x3 tmp(m_imp->getCameraToCell());
	tmp.productFromLeft(m_imp->getCell()->getImplementation()->getCellToWorld());
	MatrixConverter::convertMatrix(mtx, tmp);
}

/*****************************************************************************
 *
 * Function:		DPVS::Commander::Viewer::getFrustum ()
 *
 * Description:		Returns current view frustum
 *
 * Parameters:		f = reference to view frustum structure
 *
 * Notes:			The frustum can be different from the one specified
 *					by the user if portals are used.
 *
 * See Also:		DPVS::Camera::getFrustum(), DPVS::Camera::setFrustum()
 *
 *****************************************************************************/

void Commander::Viewer::getFrustum		(Frustum& f) const
{
	DPVS_ASSERT_INIT();
	DPVS_API_ASSERT ("Commander::Viewer::getFrustum()" && this);
	DPVS_ASSERT (m_imp);

	f = m_imp->getFrustum();

	// Validate frustum type

	f.type = m_imp->isOrthoProjection() ? Frustum::ORTHOGRAPHIC : Frustum::PERSPECTIVE;

	// Remove pixel center correction from the frustum

	int rasterWidth = m_imp->getRasterWidth();
	int rasterHeight = m_imp->getRasterHeight();

	if(rasterWidth > 0 && rasterHeight > 0)
	{
		float ooRasterWidth  = Math::reciprocal((float)rasterWidth);
		float ooRasterHeight = Math::reciprocal((float)rasterHeight);

		float pixelCenterX, pixelCenterY;
		m_imp->getPixelCenter(pixelCenterX, pixelCenterY);

		if(f.type == Frustum::ORTHOGRAPHIC)
		{
			f.left   -= pixelCenterX * ooRasterWidth;
			f.right  -= pixelCenterX * ooRasterWidth;
			f.top    -= pixelCenterY * ooRasterHeight;
			f.bottom -= pixelCenterY * ooRasterHeight;
		}
		else
		{
			f.left   -= pixelCenterX * ooRasterWidth * f.zNear;
			f.right  -= pixelCenterX * ooRasterWidth * f.zNear;
			f.top    -= pixelCenterY * ooRasterHeight * f.zNear;
			f.bottom -= pixelCenterY * ooRasterHeight * f.zNear;
		}
	}
}

/*****************************************************************************
 *
 * Function:		DPVS::Commander::Viewer::getFrustumPlaneCount()
 *
 * Description:		Returns number of planes creating the view frustum
 *
 * Notes:			This function is only needed if the performing
 *					per plane rejection.
 *
 * See Also:		DPVS::Camera::getFrustumPlane()
 *
 *****************************************************************************/

int	Commander::Viewer::getFrustumPlaneCount	(void) const
{
	DPVS_ASSERT_INIT();
	DPVS_API_ASSERT ("Commander::Viewer::getFrustumPlaneCount()" && this);

	return m_imp->getUserFrustumPlaneCount();
}

/*****************************************************************************
 *
 * Function:		DPVS::Commander::Viewer::getFrustumPlane()
 *
 * Description:		Returns a plane N of the view frustum
 *
 * Parameters:		n	 = number of the plane [0,getFrustumPlaneCount()[
 *					pleq = okane equation of the plane
 *
 * Notes:			This function is only needed if the performing
 *					per plane rejection.
 *
 * See Also:		DPVS::Camera::getFrustumPlaneCount()
 *
 *****************************************************************************/

void Commander::Viewer::getFrustumPlane			(int n,Vector4& pleq) const
{
	DPVS_ASSERT_INIT();
	DPVS_API_ASSERT ("Commander::Viewer::getFrustumPlane()" && this);
	DPVS_API_ASSERT(n>=0 && n<getFrustumPlaneCount());

	pleq = m_imp->getUserFrustumPlane(n);
}

/*****************************************************************************
 *
 * Function:		DPVS::Commander::Viewer::isMirrored()
 *
 * Description:		
 *
 * Parameters:		
 *					
 * Returns:
 *
 *****************************************************************************/

bool Commander::Viewer::isMirrored (void) const
{
	DPVS_ASSERT_INIT();

	//-----------------------------------------------------
	// is camera -> cell flipped (VirtualPortals)
	//-----------------------------------------------------

	const Matrix4x3&	m		= m_imp->getCameraToCell();
	Vector3				normal	= cross(m.getRight(),m.getUp());
	bool				flipped = dot(normal,m.getDof()) < 0.f;

	//-----------------------------------------------------
	// is projection flipped (flipped frustum)
	//-----------------------------------------------------

	const Matrix4x4&	m2		= m_imp->getCameraToScreen();
	normal	= cross(m2.getRight(),m2.getUp());
	if(dot(normal,m2.getDof()) < 0.f)
		flipped = !flipped;

	return flipped;
}

/*****************************************************************************
 *
 * Function:		DPVS::Commander::Viewer::getScissor ()
 *
 * Description:		Returns current scissor rectangle
 *
 * Parameters:		left   = smallest x-coordinate of an axis-aligned rectangle (inclusive)
 *					top    = smallest y-coordinate of an axis-aligned rectangle (inclusive)
 *					right  = largest x-coordinate of an axis-aligned rectangle (exclusive)
 *					bottom = largest y-coordinate of an axis-aligned rectangle (exclusive)
 *
 * Notes:			The coordinates returned are in output resolution scale.
 *
 * See Also:		DPVS::Camera::setParameters()
 *
 *****************************************************************************/

void Commander::Viewer::getScissor (int &left,int &top,int &right,int &bottom) const
{
	DPVS_ASSERT_INIT();
	DPVS_API_ASSERT("Commander::Viewer::getScissor()" && this);
	DPVS_ASSERT	(m_imp);

	//--------------------------------------------------------------------
	// Input rectangle already is integer ccoordinates, stored as float 
	// for performance purposes.
	//--------------------------------------------------------------------

	const FloatRectangle& r = m_imp->getRasterViewport();	// non-negative
	
	left	= Math::intChop(r.x0);
	top		= Math::intChop(r.y0);
	right	= Math::intChop(r.x1);
	bottom	= Math::intChop(r.y1);
}

/*****************************************************************************
 *
 * Function:		DPVS::Cell::set()
 *
 * Description:		Enables or disables specified property
 *
 * Parameters:		p	= property enumeration
 *					b	= boolean value indicating new state of the property (true = enabled)
 *
 * See Also:		DPVS::Cell::test()
 *
 *****************************************************************************/

void Cell::set	(Property p, bool b)
{
	DPVS_MUTEX_LOCK
	DPVS_ASSERT_INIT();
	DPVS_ASSERT_SELF_OK();
	DPVS_ASSERT_NO_VISIBILITY_QUERY();
	getImplementation()->set(p,b);
}

/*****************************************************************************
 *
 * Function:		DPVS::Cell::test()
 *
 * Description:		Returns state of specified property
 *
 * Parameters:		p	= property enumeration
 *
 * Returns:			true if property is set, false otherwise
 *
 * See Also:		DPVS::Cell::set()
 *
 *****************************************************************************/

bool Cell::test	(Property p) const
{
	DPVS_MUTEX_LOCK
	DPVS_ASSERT_INIT();
	DPVS_ASSERT_SELF_OK();
	return getImplementation()->test(p);
}

/******************************************************************************
 *
 * Category:		Core
 *
 * Class:			DPVS::LibraryDefs
 *
 * Description:		Dummy class containing some enumerations and structures
 *					used by DPVS::Library
 *
 *****************************************************************************/
/******************************************************************************
 *
 * Enum:			DPVS::LibraryDefs::InfoString
 *
 * Description:		Enumerations of input values for DPVS::Library::getInfoString()
 *
 * See Also:		DPVS::Library::getInfoString()
 *
 *****************************************************************************/
/******************************************************************************
 *
 * Enum:			DPVS::LibraryDefs::MatrixFormat
 *
 * Description:		Enumeration of matrix formats
 *
 * See Also:		DPVS::Library::Info
 *
 *****************************************************************************/
/******************************************************************************
 *
 * Enum:			DPVS::LibraryDefs::Statistic
 *
 * Description:		Enumeration of different statistics
 *
 * See Also:		DPVS::Library::getStatistic()
 *
 *****************************************************************************/
/*****************************************************************************
 *
 * Function:		DPVS::Library::init()
 *
 * Description:		Initializes DPVS
 *
 * Parameters:		mf = matrix format enumeration
 *
 * Notes:			Call this function once at the beginning of your
 *					application, before constructing any DPVS-related objects.
 *
 *					The Commander object must be defined. DPVS uses it
 *					to perform callbacks during visibility queries.
 *
 * See Also:		DPVS::Library::exit()
 * 
 *****************************************************************************/

void Library::init (MatrixFormat mf, LibraryDefs::Services* services)		
{ 
	DPVS_ASSERT_NO_VISIBILITY_QUERY();
	DPVS_API_ASSERT (!g_libraryInitialized && "Library::init() - function called multiple times!");

	if (!g_libraryInitialized)
	{
		#if defined (DPVS_EVALUATION)	// evaluation copy code
			static unsigned int foo = DPVS::FLEXLM::getCode(DPVS_FLEXLM_CODE4);
		#endif
		g_libraryInitialized = true;		

		//--------------------------------------------------------------------
		// Initialize some basic sub-systems
		//--------------------------------------------------------------------

		MatrixConverter::setColumnMajor(mf == COLUMN_MAJOR ? true : false);

		Debug::init(services);									// init debug system and assign services

		DPVS_MUTEX_LOCK											// mutex lock!

		// NOTE: WE ARE ALLOWED TO MAKE MEMORY ALLOCATIONS ONLY AFTER THIS POINT!!!!
		
		InstanceCount::init();									// init instance counting system

		//--------------------------------------------------------------------
		// Test little/big endian
		//--------------------------------------------------------------------

#if defined (DPVS_DEBUG)

		bool compiledLittleEndian = false;		// settings used to compile this library
		#if defined (DPVS_LITTLE_ENDIAN)
			compiledLittleEndian = true;
		#endif

		bool	littleEndian	= false;
		UINT32	dword			= 0x01000002;
		unsigned char *ptr = (unsigned char*)&dword;
		if(ptr[0]==0x02)	
			littleEndian = true;
		else				
			littleEndian = false;

		DPVS_ASSERT (compiledLittleEndian == littleEndian);

#endif // DPVS_DEBUG

		//--------------------------------------------------------------------
		// Initialize silhouette cache and set maximum memory usage for it..
		//--------------------------------------------------------------------

		SilhouetteCache::init();
		SilhouetteCache::get()->setMaxCacheSize(256*1024);

		//--------------------------------------------------------------------
		// Initialize temporary allocation system
		//--------------------------------------------------------------------

		g_tempAllocator.setSize(65536);							

		//--------------------------------------------------------------------
		// Check FlexLM status in evaluation version
		//-------------------------------------------------------------------- 

#if defined (DPVS_EVALUATION)	// evaluation copy code
	if ((foo/131 + 439049) != (DPVS_FLEXLM_ANSWER4/131 + 439049))
		DPVS::FLEXLM::failMessage();
#endif // DPVS_EVALUATION

		//--------------------------------------------------------------------
		// Initialize RemoteDebugger
		//-------------------------------------------------------------------- 

		#if defined (DPVS_USE_REMOTE_DEBUGGER)
		Debug::setLineDrawFlags(0xFFFFFFFFu);
		Debug::setBufferFlags(0xFFFFFFFFu);
		DPVS::RemoteDebuggerWrapper::init();
		#endif
	}

	checkConsistency();						// check that everything is a-ok
}

/*****************************************************************************
 *
 * Function:		DPVS::Library::exit()
 *
 * Description:		Shuts down DPVS
 *
 * Notes:			Call this function once at the end of your application,
 *					after destructing _all_ DPVS objects.
 *
 * See Also:		DPVS::Library::init()
 * 
 *****************************************************************************/

void Library::exit (void)					
{ 
	DPVS_ASSERT_NO_VISIBILITY_QUERY();

	// don't assert here g_libraryInitialized because the DLL shutdown calls (again)
	// Library::exit() just to make sure we shut down properly

	if (g_libraryInitialized)				// perform shutdown only once..
	{
		{
			DPVS_MUTEX_LOCK											// mutex lock!

			//--------------------------------------------------------------------
			// Shut down remote debugger
			//-------------------------------------------------------------------- 

			checkConsistency();					// check that everything is a-ok

			#if defined (DPVS_USE_REMOTE_DEBUGGER)
			RemoteDebuggerWrapper::shutdown();
			#endif

			SilhouetteCache::exit();
			InstanceCount::exit();				// make sure there are no instances left

			g_tempAllocator.setSize(0);			// release all temporary alloc memory

			ImpCommander::releaseAll();			// kill the ImpCommanders (but may leave Commanders alive)...
			
			Debug::exit();						// shutdown debug system
			Memory::shutdown();					// shutdown memory manager (no allocs/releases after this point!!)
		}

		Debug::setServices(null);			// unlink the Services of the user....
		g_libraryInitialized = false;
	}
}

/*****************************************************************************
 *
 * Function:		DPVS::Library::checkConsistency()
 *
 * Description:		Executes an internal consistency check for the
 *					entire library (debug build only). Raises assertions
 *					if any problems are found.
 *
 * Notes:			The check may take some time (it touches *all* internal 
 *					data structures) - use this only as an aid
 *					for debugging. The consistency check attempts to trap
 *					invalid pointers etc.
 *
 *****************************************************************************/

void Library::checkConsistency (void)
{
#if defined (DPVS_DEBUG)
	DPVS_MUTEX_LOCK
	if (g_libraryInitialized)
	{
		ImpCamera::checkConsistencyAll();		// check all ImpCameras
		ImpObject::checkConsistencyAll();		// check all ImpObjects (these check Models in turn)
		ImpCell::checkConsistencyAll();			// check all Cells and associated Databases
		Memory::checkConsistency();				// check memory manager consistency (all problems should've been trapped by now)
	}
#endif
}

/*****************************************************************************
 *
 * Function:		DPVS::Library::minimizeMemoryUsage()
 *
 * Description:		Minimizes memory usage of all DPVS sub-systems
 *
 * Notes:			When this function is called, DPVS will inform
 *					all of its sub-systems to minimize their memory usage.
 *					This will cause freeing of all temporary allocations,
 *					invalidation of all caches etc. This function shouldn't
 *					be called often as it has a negative impact on the speed
 *					of the following visibility queries (as caches are rebuilt
 *					and new temporary memory allocations are performed).
 *
 * See Also:		DPVS::Library::suggestGarbageCollect()
 * 
 *****************************************************************************/

void Library::minimizeMemoryUsage (void)
{
	DPVS_MUTEX_LOCK
	DPVS_ASSERT_INIT();
	DPVS_ASSERT_NO_VISIBILITY_QUERY();

	checkConsistency();							// run consistency checks before..
	SilhouetteCache::get()->invalidateCache();	// invalidate silhouette cache
	ImpCell::minimizeMemoryUsage();				// iterate through all cells
	ImpCamera::minimizeMemoryUsage();			// iterate through all cameras
	SweepAndPrune::minimizeMemoryUsage();		// minimize sweep&prune memory usage
	checkConsistency();							// run consistency checks after this operation

//	g_heap.minimizeMemoryUsage();				// minimize heap memory usage
}

/*****************************************************************************
 *
 * Function:		DPVS::Library::getInfoString()
 *
 * Description:		Returns information about the library
 *
 * Parameters:		t = enumerated value indicating which string is returned
 *
 * Returns:			corresponding information string
 *
 *****************************************************************************/

const char* Library::getInfoString (InfoString t)
{
	DPVS_MUTEX_LOCK
	switch (t)
	{
		case VERSION:
		{
			static char s_tmp[64];
			sprintf (s_tmp,"dPVS %d.%d.%d",DPVS_VERSION_MAJOR, DPVS_VERSION_MINOR, DPVS_VERSION_REVIS);
			return s_tmp;
		}

		case COPYRIGHT:		return DPVS_COPYRIGHT;
		case BUILD_TIME:	return DPVS_BUILD_TIME;
#if defined (DPVS_EVALUATION)
		case FUNCTIONALITY:	return "evaluation version";
		case CUSTOMER:		return "evaluation user";	
#else
		case FUNCTIONALITY:	return "registered version";
		case CUSTOMER:		return "dpvs@hybrid.fi";
#endif // DPVS_EVALUATION
		
		case CPU_OPTIMIZATIONS:

			#if defined (DPVS_CPU_X86)
			{
				unsigned int set = X86::getInstructionSets();

				if (set & X86::S_ATHLON)	return "X86+MMX+Athlon";
				if (set & X86::S_SSE)		return "X86+MMX+SSE";
				if (set & X86::S_3DNOW)		return "X86+MMX+3DNow";
				if (set & X86::S_MMX)		return "X86+MMX";
			}
			#endif // DPVS_CPU_X86

			#if defined (DPVS_UINT64_DEFINED)
				return DPVS_CPU_NAME" (64-bit)";
			#else
				return DPVS_CPU_NAME" (32-bit)";					// default
			#endif

		case COMPILER:
			#if defined (DPVS_DEBUG)
				return DPVS_COMPILER" (debug build)";
			#else
				return DPVS_COMPILER;
			#endif
	}

	DPVS_API_ASSERT (!"Invalid InfoString enumeration");

	return "";		// don't die in release build
}

/*****************************************************************************
 *
 * Function:		DPVS::Library::resetStatistics()
 *
 * Description:		Resets all statistics counters
 *
 * See Also:		DPVS::Library::getStatistic()
 *
 *****************************************************************************/

void Library::resetStatistics (void)					
{ 
	DPVS_ASSERT_INIT();
	DPVS_ASSERT_NO_VISIBILITY_QUERY();
	Statistics::resetStatistics();
}

/*****************************************************************************
 *
 * Function:		DPVS::Library::getStatisticName()
 *
 * Description:		Returns name of a statistic
 *
 * Parameters:		s = statistic enumeration
 *
 * Returns:			corresponding name (const string - DO NOT MODIFY!!!!)
 *
 * See Also:		DPVS::Library::getStatistic()
 *
 *****************************************************************************/

const char* Library::getStatisticName (Statistic s)
{
	DPVS_MUTEX_LOCK
	DPVS_ASSERT_INIT();
	return Statistics::getStatisticName(s);
}

/*****************************************************************************
 *
 * Function:		DPVS::Library::getStatistic()
 *
 * Description:		Returns statistic value
 *
 * Parameters:		s = statistic enumeration
 *
 * Returns:			corresponding statistic value
 *
 * See Also:		DPVS::Library::resetStatistics(),
 *                  DPVS::Library::getStatisticName()
 *					
 *
 *****************************************************************************/

float Library::getStatistic (Statistic s)
{
	DPVS_MUTEX_LOCK
	DPVS_ASSERT_INIT();
	return Statistics::getStatistic(s);
}

/*****************************************************************************
 *
 * Function:		DPVS::Library::getFlags()
 *
 * Description:		Returns current debug linedraw flags
 *
 * Parameters:		t	 = type of the flag
 *					flag = debug linedraw flags to use
 *
 *****************************************************************************/

unsigned int Library::getFlags		(Library::FlagType t)	
{
	DPVS_MUTEX_LOCK
	DPVS_ASSERT_INIT();
	switch(t)
	{
		case LINEDRAW:	return Debug::getLineDrawFlags();
		case BUFFER:	return Debug::getBufferFlags();
	}
	
	DPVS_ASSERT(!"Library::getFlags() - Unknown flag type"); 
	return 0;
}

/*****************************************************************************
 *
 * Function:		DPVS::Library::setFlags()
 *
 * Description:		Returns current debug linedraw flags
 *
 * Parameters:		t	 = type of the flag
 *					flag = debug linedraw flags to use
 *
 *****************************************************************************/

void Library::setFlags		(Library::FlagType t,unsigned int flag)
{
	DPVS_MUTEX_LOCK
	DPVS_ASSERT_INIT();
//	DPVS_ASSERT_NO_VISIBILITY_QUERY();

	switch(t)
	{
		case LINEDRAW:	Debug::setLineDrawFlags(flag); break;
		case BUFFER:	Debug::setBufferFlags(flag);   break;
		default:
			DPVS_ASSERT(!"Library::setFlags() - Unknown flag type"); 
			break;
	}
}

/*****************************************************************************
 *
 * Function:		DPVS::Library::clearFlags()
 *
 * Description:		Returns current debug linedraw flags
 *
 * Parameters:		t	 = type of the flag
 *					flag = debug linedraw flags to use
 *
 *****************************************************************************/

void Library::clearFlags	(Library::FlagType t,unsigned int flag)
{
	DPVS_MUTEX_LOCK
	DPVS_ASSERT_INIT();
//	DPVS_ASSERT_NO_VISIBILITY_QUERY();

	switch(t)
	{
		case LINEDRAW:	Debug::clearLineDrawFlags(flag); break;
		case BUFFER:	Debug::clearBufferFlags(flag);   break;
		default:
			DPVS_ASSERT(!"Library::clearFlags() - Unknown flag type"); 
			break;
	}
	
}

/*****************************************************************************
 *
 * Function:		DPVS::Library::suggestGarbageCollect()
 *
 * Description:		Suggest garbage collection of objects that haven't been
 *					visible in specified time period
 *
 * Parameters:		c = pointer to Commander to catch the suggestions (non-null)
 *					t = time in seconds (must be zero or positive)
 *
 * Notes:			The function will report through the REMOVAL_SUGGESTED
 *					command all objects that haven't been visible (in any
 *					camera) during the last 't' seconds. The object pointer
 *					can be queried in the callback by using 
 *					DPVS::Commander::getInstance().
 *
 *					If the commander pointer is null or the time value is negative, 
 *					the function won't do anything in the release build. In debug
 *					build such cases are asserted.
 *
 * See Also:		DPVS::Library::minimizeMemoryUsage()
 *
 *****************************************************************************/

void Library::suggestGarbageCollect(Commander* c, float t)
{
	DPVS_MUTEX_LOCK
	DPVS_ASSERT_INIT();
	DPVS_ASSERT_NO_VISIBILITY_QUERY();
	DPVS_API_ASSERT (c);
	if (t < 0.0f || !c)
		return;

	ImpObject::suggestGarbageCollect (c, t);
}

/*****************************************************************************
 *
 * Function:		DPVS::Library::textCommand()
 *
 * Description:		Submits a command to library in text format.
 *
 * Parameters:		cmd = string descibing the command
 *
 * Returns:			command-specific information
 *
 * Notes:			This function is used only for debugging/profiling DPVS
 *					and not needed in everyday development.
 *
 *****************************************************************************/

int Library::textCommand(Commander* commander, const char* cmd)
{
	DPVS_MUTEX_LOCK
	DPVS_ASSERT_INIT();
	DPVS_API_ASSERT(commander && "Library::textCommand() - commander not specified");
	DPVS_API_ASSERT(cmd       && "Library::textCommand() - null command string");

	if (!commander || !cmd)
		return 0;

	//--------------------------------------------------------------------
	// Some text commands
	//--------------------------------------------------------------------

	DPVS_ASSERT(commander);
	DataPasser::setCommander (commander);

	if (!strcmp(cmd,"assert"))
	{
		assertFail("reason: testing textCommand 'assert'\n",__FILE__,__LINE__,"this is a message");
		Debug::print ("I hear you loud and clear!\n");
	} else
	if (!strcmp(cmd,"test"))
	{
		Debug::print ("I hear you loud and clear!\n");
	} else
	if (!strcmp(cmd,"wili"))
	{
		Debug::print ("respect\n");
//		Memory::dump();

		int total = Memory::getMemoryReserved();
		int silh  = SilhouetteCache::get()->getMemoryUsed();
		int der   = Math::intFloor(Library::getStatistic(Library::STAT_MODELDERIVEDMEMORYUSED));
		int obs   = Math::intFloor(Library::getStatistic(Library::STAT_LIVEOBJECTS)) * (sizeof(Object)+sizeof(ImpObject));
		int topo  = Mesh::getMemoryUsed();
		int node  = IDatabase::getNodeMemoryUsed();
		int inst  = IDatabase::getInstanceMemoryUsed();
		int model = Math::intFloor(Library::getStatistic(Library::STAT_LIVEMODELS)) * (sizeof(MeshModel)+sizeof(ImpMeshModel));
		int tmp   = g_tempAllocator.getSize();
		int buf   = total - Memory::getMemoryUsed();
		int other = total-(silh+der+topo+obs+node+inst+model+tmp+buf);

		Debug::print ("silhouette cache    = %-7d (%.2f%%)\n",silh,(100.0f*silh) / total);
		Debug::print ("models              = %-7d (%.2f%%)\n",model,(100.0f*model) / total);
		Debug::print ("objects             = %-7d (%.2f%%)\n",obs,(100.0f*obs) / total);
		Debug::print ("ob instances        = %-7d (%.2f%%)\n",inst,(100.0f*inst) / total);
		Debug::print ("nodes               = %-7d (%.2f%%)\n",node,(100.0f*node) / total);
		Debug::print ("topology            = %-7d (%.2f%%)\n",topo,(100.0f*topo) / total);
		Debug::print ("derived topology    = %-7d (%.2f%%)\n",der,(100.0f*der) / total);
		Debug::print ("temporary allocator = %-7d (%.2f%%)\n",tmp,(100.0f*tmp) / total);
		Debug::print ("allocation reserve  = %-7d (%.2f%%)\n",buf,(100.0f*buf) / total);
		Debug::print ("other               = %-7d (%.2f%%)\n",other,(100.0f*other) / total);
		Debug::print ("total memory used   = %-7d\n",total);

		Debug::print ("sizeof(Camera)            == %d\n",sizeof(Camera)+sizeof(ImpCamera));
		Debug::print ("sizeof(Object)            == %d\n",sizeof(Object)+sizeof(ImpObject));
		Debug::print ("sizeof(MeshModel)         == %d\n",sizeof(MeshModel)+sizeof(ImpMeshModel));
		Debug::print ("scratchpad used           == %d (left = %d)\n",Scratchpad::MAX_ADDRESS,16384-Scratchpad::MAX_ADDRESS);

//		Math::janiTest();
//		Debug::print ("sizeof(OcclusionBuffer)   == %d\n",sizeof(OcclusionBuffer));
//		Debug::print ("sizeof(WriteQueue)        == %d\n",sizeof(WriteQueue));

//		Math::ppcTest();

	} else
	{
		DPVS_ASSERT("textCommand: invalid command");
	}
	
	DataPasser::setCommander (null);
	return 0;
}

/******************************************************************************
 *
 * Enum:			DPVS::Object::Property
 *
 * Description:		Object property enumeration
 *
 * Notes:			These enumerations are used by DPVS::Object::set() and
 *					DPVS::Object::test() functions to define some object-specific
 *					properties.
 *
 *					The object properties are initialized as follows:
 *
 *					ENABLED            = true
 *					INFORM_VISIBLE     = true
 *
 *					When an object is not ENABLED, it will be interpreted
 *					as the object didn't exist at all during visibility queries. Non-enabled
 *					objects have zero processing overhead during visibility traversals. The
 *					user can thus disable large portions of a scene and be sure that the disabled
 *					objects won't slow down visibility processing.
 *
 *					If INFORM_VISIBLE is disabled, the visibility of the object
 *					will be determined and the object can be used as an occluder. However,
 *					a visibility callback is not made (i.e. no message is sent
 *					to DPVS::Commander). This flag applies only to
 *					Objects and Portals -- Regions Of Influence ignore this flag (as 
 *					they never create a visibility callback).
 *
 * See Also:		DPVS::Object::Object(Model*)
 *
 *****************************************************************************/

/*****************************************************************************
 *
 * Function:		DPVS::Object::setCell()
 *
 * Description:		Assigns object to specified cell
 *
 * Parameters:		cell = pointer to cell (can be null)
 *
 * Notes:			The object is removed from its previous cell.
 *
 *					By default, objects don't belong to any cell. In order
 *					to see an object, it MUST be added to some cell.
 *
 *					The cell will take a reference to the object (i.e.
 *					increase the object's reference count by one). When the cell
 *					is destroyed, the reference will be released.
 *
 *					The object can be temporarily disabled by assigning
 *					it into a null cell. However, the ENABLED flag method
 *					for disabling objects should be preferred instead.
 *
 * See Also:		DPVS::Object::getCell()
 *
 *****************************************************************************/

void Object::setCell (Cell* cell)	
{ 
	DPVS_MUTEX_LOCK
	DPVS_ASSERT_INIT();
	DPVS_ASSERT_SELF_OK();
	DPVS_ASSERT (!cell || Memory::isValidPointer(cell));

//	DPVS_ASSERT_NO_VISIBILITY_QUERY();
	getImplementation()->setCell(cell); 
}

/*****************************************************************************
 *
 * Function:		DPVS::Object::getCell()
 *
 * Description:		Returns pointer to cell where object belongs to
 *
 * Returns:			pointer to cell where object belongs (or null if none)
 *
 * See Also:		DPVS::Object::setCell()
 *
 *****************************************************************************/

Cell*	Object::getCell (void) const		
{ 
	DPVS_MUTEX_LOCK
	DPVS_ASSERT_INIT();
	DPVS_ASSERT_SELF_OK();
	return getImplementation()->getCell(); 
}


/*****************************************************************************
 *
 * Function:		DPVS::Object::setTestModel()
 *
 * Description:		Defines a new test model for the object
 *
 * Parameters:		m = pointer to test model (cannot be null)
 *
 * Notes:			If the object is a portal the following rules apply:
 *
 *					- Backface culling MUST be enabled for the model!
 *
 *					- If portal models cannot be backface culled, infinite
 *					  recursion cases are evident. Even though DPVS employs the
 *					  concept of recursion depth, the results would be wrong.
 *
 *					The object takes a reference to the model. The reference
 *					is released when a new test model is assigned or when
 *					the object is destroyed.
 *
 *					Objects must always have test models. Thus a null pointer
 *					cannot be passed to the function (asserted in debug build
 *					and ignored in release build).
 *
 *****************************************************************************/
				
void Object::setTestModel (Model *m)			
{ 
	DPVS_MUTEX_LOCK
	DPVS_ASSERT_INIT();
	DPVS_ASSERT_SELF_OK();
//	DPVS_ASSERT_NO_VISIBILITY_QUERY();
	DPVS_API_ASSERT (m && "Object::setTestModel() - Test models cannot be null");
	DPVS_API_ASSERT (Memory::isValidPointer(m));

	if (m)
		getImplementation()->setTestModel(m); 
}

/*****************************************************************************
 *
 * Function:		DPVS::PhysicalPortal::setImportanceDecay()
 *
 * Description:		Sets importance decay factor to portal.
 *
 * Parameters:		d = decay factor [0,1]
 *
 * Notes:			Defaults to 1.0 and physical portals that are solely used
 *					for visibility determination should always use the
 *					factor 1.0. Virtual portals used to create special effects
 *					such as mirrors should set this to
 *					(1.0 - average transparency value).
 *					
 *					This information is visible for the user when
 *					LOD level is being estimated.
 *
 *					The input parameter is silently clamped into [0,1] range.
 *
 *****************************************************************************/

float PhysicalPortal::getImportanceDecay (void) const
{
	DPVS_MUTEX_LOCK
	DPVS_ASSERT_INIT();
	DPVS_ASSERT_SELF_OK();
	return ((ImpPhysicalPortal*)getImplementation())->getImportanceDecay();
}

void PhysicalPortal::setImportanceDecay	(float d)
{
	DPVS_MUTEX_LOCK
	DPVS_ASSERT_INIT();
	DPVS_ASSERT_SELF_OK();
//	DPVS_ASSERT_NO_VISIBILITY_QUERY();

	Math::clamp(d,0.0f,1.0f);

	((ImpPhysicalPortal*)getImplementation())->setImportanceDecay(d);
}

/*****************************************************************************
 *
 * Function:		DPVS::PhysicalPortal::setStencilModel()
 *
 * Description:		Defines stencil mask model of a portal.
 *
 * Parameters:		m = pointer to stencil model (can be null)
 *
 *****************************************************************************/

void PhysicalPortal::setStencilModel		(Model* m)
{
	DPVS_MUTEX_LOCK
	DPVS_ASSERT_INIT();
	DPVS_ASSERT_SELF_OK();
	DPVS_ASSERT(m == 0 || Memory::isValidPointer(m));
//	DPVS_ASSERT_NO_VISIBILITY_QUERY();
	DPVS_API_ASSERT(m->getImplementation()->isOcclusionWritable() && "PhysicalPortal::setStencilModel() - model must be occlusion writable");

	((ImpPhysicalPortal*)getImplementation())->setStencilModel(m);
}

/*****************************************************************************
 *
 * Function:		DPVS::PhysicalPortal::setTargetCell()
 *
 * Description:		Defines target cell the portal leads to.
 *
 * Parameters:		c = target cell
 *
 *****************************************************************************/

void PhysicalPortal::setTargetCell			(Cell* c)
{
	DPVS_MUTEX_LOCK
	DPVS_ASSERT_INIT();
	DPVS_ASSERT_SELF_OK();
	DPVS_ASSERT(c == 0 || Memory::isValidPointer(c));
//	DPVS_ASSERT_NO_VISIBILITY_QUERY();
	ImpPhysicalPortal* imp = (ImpPhysicalPortal*)getImplementation();

	if(imp->getType() == ImpObject::TYPE_PHYSICAL_PORTAL)
		imp->setTargetCell(c);
	else
	{
		DPVS_API_ASSERT(!"VirtualPortal::setTargetCell() - target cell is automatically obtained from target portal and should not be set!");
	}
}

/*****************************************************************************
 *
 * Function:		DPVS::PhysicalPortal::getTargetCell()
 *
 * Description:		Returns target cell the portal leads to.
 *
 *****************************************************************************/

Cell* PhysicalPortal::getTargetCell			(void) const
{
	DPVS_MUTEX_LOCK
	DPVS_ASSERT_INIT();
	DPVS_ASSERT_SELF_OK();
	return ((ImpPhysicalPortal*)getImplementation())->getTargetCell();
}

/*****************************************************************************
 *
 * Function:		DPVS::PhysicalPortal::getStencilModel()
 *
 * Description:		Returns stencil mask model of a portal.
 *
 * Returns:			pointer to stencil mask model of the portal (or null if none defined)
 *
 *****************************************************************************/

Model* PhysicalPortal::getStencilModel		(void) const
{
	DPVS_MUTEX_LOCK
	DPVS_ASSERT_INIT();
	DPVS_ASSERT_SELF_OK();
	return ((ImpPhysicalPortal*)getImplementation())->getStencilModel();
}


/*****************************************************************************
 *
 * Function:		DPVS::Object::getTestModel()
 *
 * Description:		Returns pointer to test model of the object
 *
 * Returns:			pointer to test model (cannot be null)
 *
 * See Also:		DPVS::Object::setTestModel()
 *
 *****************************************************************************/

Model* Object::getTestModel (void) const		
{ 
	DPVS_MUTEX_LOCK
	DPVS_ASSERT_INIT();
	DPVS_ASSERT_SELF_OK();
	return getImplementation()->getUserTestModel(); 
}

/*****************************************************************************
 *
 * Function:		DPVS::Object::getWriteModel()
 *
 * Description:		Returns pointer to write model of the object
 *
 * Returns:			pointer to write model or null if none defined
 *
 * See Also:		DPVS::Object::setWriteModel()
 *
 *****************************************************************************/

Model* Object::getWriteModel (void) const		
{ 
	DPVS_MUTEX_LOCK
	DPVS_ASSERT_INIT();
	DPVS_ASSERT_SELF_OK();
	return getImplementation()->getUserWriteModel(); 
}

/*****************************************************************************
 *
 * Function:		DPVS::Object::setWriteModel()
 *
 * Description:		Defines a write model for the object
 *
 * Parameters:		model = pointer to model (may be null)
 *
 * Notes:			The object takes a reference to the model. The reference
 *					is released when a new write model is assigned or when
 *					the object is destroyed.
 *
 *					The write model can be disabled by assigning a null
 *					write model (this is the default). If no write model
 *					is assigned, the object is never used as an occluder.
 *
 * See Also:		DPVS::Object::getWriteModel()
 *
 *****************************************************************************/

void Object::setWriteModel (Model* m)
{ 
	DPVS_MUTEX_LOCK
	DPVS_ASSERT_INIT();
	DPVS_ASSERT_SELF_OK();
	DPVS_ASSERT(m == 0 || Memory::isValidPointer(m));

//	DPVS_ASSERT_NO_VISIBILITY_QUERY();		// NOTE: Is allowed in the middle of resolveVisibility()
	getImplementation()->setWriteModel(m); 
}

/*****************************************************************************
 *
 * Function:		DPVS::Object::setVisibilityParent()
 *
 * Description:		Defines a visibility parent for the object
 *
 * Parameters:		parent = pointer to visibility parent (can be null)
 *
 * Notes:			The visibility parent must exist in the same cell
 *					as the object.
 *
 *					Visibility parents can be used in some cases to
 *					create visibility relationships between objects. An
 *					object will be found 'hidden' in a visibility query if its 
 *					visibility parent is hidden. An object should be fully
 *					contained inside its visibility parent's test bounds -
 *					otherwise artifacts will occur. 
 *
 *					By default, objects don't have visibility parents, i.e.
 *					same as calling DPVS::Object::setVisibilityParent(null);
 *
 * See Also:		DPVS::Object::getVisibilityParent()
 *
 *****************************************************************************/
				
void Object::setVisibilityParent (Object *parent)	
{
	DPVS_MUTEX_LOCK
	DPVS_ASSERT_INIT();
	DPVS_ASSERT_SELF_OK();
	DPVS_API_ASSERT(parent == 0 || Memory::isValidPointer(parent));

//	DPVS_ASSERT_NO_VISIBILITY_QUERY();
	getImplementation()->setVisibilityParent(parent ? parent->getImplementation() : null); 

	//--------------------------------------------------------------------
	// Check for circular references in debug build
	//--------------------------------------------------------------------

#if defined (DPVS_DEBUG)
	Object* p = parent;
	while (p)
	{
		if (p == this)
			DPVS_API_ASSERT(!"Object::setVisibilityParent(): Circular reference in visibility parent list");
		p = p->getVisibilityParent();
	}
#endif
}

/*****************************************************************************
 *
 * Function:		DPVS::Object::getVisibilityParent()
 *
 * Description:		Returns pointer to visibility parent of the object
 *
 * Returns:			pointer to visibility parent of the object (or null if none defined)
 *
 * See Also:		DPVS::Object::setVisibilityParent()
 *
 *****************************************************************************/

Object* Object::getVisibilityParent (void) const		
{ 
	DPVS_MUTEX_LOCK
	DPVS_ASSERT_INIT();
	DPVS_ASSERT_SELF_OK();
	ImpObject* obj = getImplementation()->getVisibilityParent();
	return obj ? obj->getUserObject() : null;
}

/*****************************************************************************
 *
 * Function:		DPVS::Object::set()
 *
 * Description:		Enables or disables specified property
 *
 * Parameters:		p	= property enumeration
 *					b	= boolean value indicating new state of the property (true = enabled)
 *
 * Notes:			The default values of the properties are described in
 *					the documentation for DPVS::Object::Object().
 *
 * See Also:		DPVS::Object::test()
 *
 *****************************************************************************/

void Object::set (Property p, bool b)
{
	DPVS_MUTEX_LOCK
	DPVS_ASSERT_INIT();
	DPVS_ASSERT_SELF_OK();
//	DPVS_ASSERT_NO_VISIBILITY_QUERY();

	switch(p)
	{
		case ENABLED:				getImplementation()->selfEnable(b);					break;
		case INFORM_VISIBLE:		getImplementation()->setInformVisible(b);			break;
		case CONTRIBUTION_CULLING:	getImplementation()->setContributionCulling(b);		break;
		case INFORM_PORTAL_ENTER:	getImplementation()->setInformPortalEnter(b);		break;
		case INFORM_PORTAL_EXIT:	getImplementation()->setInformPortalExit(b);		break;
		case INFORM_PORTAL_PRE_EXIT:getImplementation()->setInformPortalPreExit(b);		break;
		case FLOATING_PORTAL:		getImplementation()->setFloatingPortal(b);			break;
		case REPORT_IMMEDIATELY:	getImplementation()->setReportImmediately(b);		break;
		case UNBOUNDED:				getImplementation()->setUnbounded(b);				break;
		case OCCLUDER:				getImplementation()->setUserOccluder(b);			break;
//		case ABORT:					getImplementation()->setAbortable(b);			    break;
		default: 
			DPVS_API_ASSERT(false && "Object::set() - Invalid property enumeration"); 
			break;
	}
}

/*****************************************************************************
 *
 * Function:		DPVS::Object::test()
 *
 * Description:		Returns state of specified property
 *
 * Parameters:		p	= property enumeration
 *
 * Returns:			true if property is set, false otherwise
 *
 * See Also:		DPVS::Object::set()
 *
 *****************************************************************************/

bool Object::test	(Property p) const
{
	DPVS_MUTEX_LOCK
	DPVS_ASSERT_INIT();
	DPVS_ASSERT_SELF_OK();
	switch(p)
	{
		case ENABLED:				return getImplementation()->isSelfEnabled();
		case INFORM_VISIBLE:		return getImplementation()->informVisible();
		case CONTRIBUTION_CULLING:	return getImplementation()->isContributionCullingEnabled();
		case INFORM_PORTAL_ENTER:	return getImplementation()->informPortalEnter();
		case INFORM_PORTAL_EXIT:	return getImplementation()->informPortalExit();
		case INFORM_PORTAL_PRE_EXIT:return getImplementation()->informPortalPreExit();
		case FLOATING_PORTAL:		return getImplementation()->isFloatingPortal();
		case REPORT_IMMEDIATELY:	return getImplementation()->reportImmediately();
//		case ABORT:					return getImplementation()->isAbortable(); 
		case UNBOUNDED:				return getImplementation()->isUnbounded();
		case OCCLUDER:				return getImplementation()->isUserOccluder();
	}

	DPVS_API_ASSERT(false && "Object::test() - Invalid property enumeration"); 
	return false;

}
				
/*****************************************************************************
 *
 * Function:		DPVS::Object::setCost()
 *
 * Description:		Defines rendering cost of the object
 *
 * Parameters:		vnum		= number of vertices (0 or more)
 *					tnum		= number of triangles (0 or more)
 *					complexity	= relative complexity value (must be >= 0.0, default = 1.0)
 *
 * Notes:			The rendering cost of an object can be defined
 *					to allow DPVS to make better decisions in its
 *					occluder selection policy. The complexity term
 *					can be used to indicate the relative rendering complexity
 *					of the triangles (number of passes during multi-passing 
 *					or relative cost of shader).
 *	
 *					The rendering cost is assigned on a per-object basis
 *					because many objects can share simplified bounds (test models) -
 *					such models wouldn't give any idea about the actual rendering
 *					cost. 
 *
 *					The rendering cost is initialized using the test model
 *					provided in the Object constructor.
 *
 *****************************************************************************/
				
void Object::setCost	(int vnum,int tnum,float complexity)	
{ 
	DPVS_MUTEX_LOCK
	DPVS_ASSERT_INIT();
	DPVS_ASSERT_SELF_OK();
	DPVS_API_ASSERT ("Object::setCost()" && vnum>=0);
	DPVS_API_ASSERT ("Object::setCost()" && tnum>=0);
	DPVS_API_ASSERT ("Object::setCost()" && complexity>=0.0f);

	if (vnum <= 0 || tnum <= 0 || complexity <= 0.0f)	// don't set it
		return;
	
	getImplementation()->setRenderCost(vnum,tnum,complexity); 
}

/*****************************************************************************
 *
 * Function:		DPVS::VirtualPortal::setTargetPortal()
 *
 * Description:		Defines target portal for a virtual portal
 *
 * Parameters:		s = pointer to target (physical) portal
 *
 * See Also:		DPVS::VirtualPortal::getTargetPortal()
 *
 *****************************************************************************/

void VirtualPortal::setTargetPortal (PhysicalPortal *s)	
{
	DPVS_MUTEX_LOCK
	DPVS_ASSERT_INIT();
	DPVS_ASSERT_SELF_OK();
	DPVS_API_ASSERT(s == 0 || Memory::isValidPointer(s));

//	DPVS_ASSERT_NO_VISIBILITY_QUERY();

	ImpVirtualPortal* imp = (ImpVirtualPortal*)getImplementation();

	imp->setTargetPortal(s);
	if(s)
		imp->setTargetCell(s->getCell());
}

/*****************************************************************************
 *
 * Function:		DPVS::VirtualPortal::getTargetPortal()
 *
 * Description:		Returns pointer to target portal
 *
 * Returns:			pointer to target portal
 *
 * See Also:		DPVS::VirtualPortal::setTargetPortal()
 *
 *****************************************************************************/

PhysicalPortal*	VirtualPortal::getTargetPortal (void) const		
{
	DPVS_MUTEX_LOCK
	DPVS_ASSERT_INIT();
	DPVS_ASSERT_SELF_OK();
	return ((ImpVirtualPortal*)getImplementation())->getTargetPortal(); 
}

/*****************************************************************************
 *
 * Function:		DPVS::Model::getAABB() const
 *	
 * Description:		Returns axis-aligned bounding box
 *
 * Parameters:		
 *
 *****************************************************************************/

void Model::getAABB		(Vector3& mn,Vector3& mx) const	// DEBUG DEBUG - has to be tested
{
	DPVS_MUTEX_LOCK
	DPVS_ASSERT_INIT();
	DPVS_ASSERT_SELF_OK();

	AABB aabb2;
	calculateAABB(aabb2,getImplementation()->getExactOBB());	// From OBB

	mn = aabb2.getMin();
	mx = aabb2.getMax();
}

/*****************************************************************************
 *
 * Function:		DPVS::Model::getOBB(Matrix4x4& m) const
 *	
 * Description:		Returns oriented bounding box
 *
 * Parameters:		m = matrix describing OBB
 *
 * Notes:			Last row is set to (0,0,0,1)
 *
 *****************************************************************************/

void Model::getOBB		(Matrix4x4& m) const
{
	DPVS_MUTEX_LOCK
	DPVS_ASSERT_INIT();
	DPVS_ASSERT_SELF_OK();
	MatrixConverter::convertMatrix(m,getImplementation()->getExactOBB().getMatrix());
}

/*****************************************************************************
 *
 * Function:		DPVS::Model::getSphere(Vector3& center, float& radius) const
 *	
 * Description:		Returns bounding sphere
 *
 * Parameters:		center = center of sphere to be returned
 *					radius = radius of sphere to be returned
 *
 *****************************************************************************/

void Model::getSphere	(Vector3& center, float& radius) const
{
	DPVS_MUTEX_LOCK
	DPVS_ASSERT_INIT();
	DPVS_ASSERT_SELF_OK();
	Sphere sphere = getImplementation()->getSphere();
	center = sphere.getCenter();
	radius = sphere.getRadius();
}

/*****************************************************************************
 *
 * Function:		DPVS::Object::getAABB() const
 *	
 * Description:		Returns axis-aligned bounding box
 *
 * Parameters:		mn = minimum of axis-aligned bounding box
 *					mx = maximum of axis-aligned bounding box
 *
 *****************************************************************************/

void Object::getAABB (Vector3& mn,Vector3& mx) const		
{
	DPVS_MUTEX_LOCK
	DPVS_ASSERT_INIT();
	DPVS_ASSERT_SELF_OK();

	if (getImplementation()->isDirty())				// if dirty, set up quick bounds
	{
		AABB aabb;
		getImplementation()->setupQuickBounds(aabb);
		mn = aabb.getMin();
		mx = aabb.getMax();
	} else								// non-dirty
	{
		mn = getImplementation()->getCellSpaceAABB().getMin();
		mx = getImplementation()->getCellSpaceAABB().getMax();
	}
}

/*****************************************************************************
 *
 * Function:		DPVS::Object::getOBB(Matrix4x4& m) const
 *	
 * Description:		Returns oriented bounding box
 *
 * Parameters:		m = matrix describing OBB
 *
 * Notes:			Last row is set to (0,0,0,1)
 *
 *****************************************************************************/

void Object::getOBB		(Matrix4x4& m) const					// DEBUG DEBUG - has to be tested
{
	DPVS_MUTEX_LOCK
	DPVS_ASSERT_INIT();
	DPVS_ASSERT_SELF_OK();

	Matrix4x3 obb(getImplementation()->getTestModel()->getExactOBB().getMatrix());	// Unit -> Model
	obb.productFromLeft(getImplementation()->getObjectToCell());					// Unit -> Model (-> Object) -> Cell
	MatrixConverter::convertMatrix(m,obb);
}

/*****************************************************************************
 *
 * Function:		DPVS::Object::getSphere(Vector3& center, float& radius) const
 *	
 * Description:		Returns bounding sphere
 *
 * Parameters:		center = center of sphere to be returned
 *					radius = radius of sphere to be returned
 *
 *****************************************************************************/

void Object::getSphere	(Vector3& center, float& radius) const
{
	DPVS_MUTEX_LOCK
	DPVS_ASSERT_INIT();
	DPVS_ASSERT_SELF_OK();

	// note: in theory we could attempt to compute a better bounding sphere if the
	// object has a sphere model!

	Sphere sphere = getImplementation()->getCellSpaceSphere();
	center = sphere.getCenter();
	radius = sphere.getRadius();
}

/*****************************************************************************
 *
 * Enum:			DPVS::Model::Property()
 *	
 * Description:		Model property enumeration
 * 
 * Notes:			These enumerations are used by the DPVS::Model::set()
 *					and DPVS::Model::test() functions.
 *
 *****************************************************************************/
/*****************************************************************************
 *
 * Function:		DPVS::Model::set(Property,bool)
 *	
 * Description:		Enables or disables specified property
 *
 * Parameters:		p = property enumeration
 *					b = boolean value indicating whether property is enabled (true) or disabled (false)
 *
 * See Also:		DPVS::Model::test()
 *
 *****************************************************************************/

void Model::set (Property p, bool b)
{
	DPVS_MUTEX_LOCK
	DPVS_ASSERT_INIT();
	DPVS_ASSERT_SELF_OK();
//	DPVS_ASSERT_NO_VISIBILITY_QUERY();

	switch(p)
	{
		case BACKFACE_CULLABLE:	getImplementation()->setBackFaceCullable(b);	break;
		case SOLID:				getImplementation()->setSolid(b);				break;

		default:
			DPVS_API_ASSERT(false && "Model::set() - Invalid property enumeration"); 
			break;
	}
}

/*****************************************************************************
 *
 * Function:		DPVS::Model::test(Property)
 *
 * Description:		Returns state of specified property
 *
 * Parameters:		p = property enumeration
 *
 * Returns:			boolean value indicating if property is set (true=enabled,false=disabled)
 *
 * See Also:		DPVS::Model::set()
 *
 *****************************************************************************/

bool Model::test (Property p) const
{
	DPVS_MUTEX_LOCK
	DPVS_ASSERT_INIT();
	DPVS_ASSERT_SELF_OK();
	switch(p)
	{
		case BACKFACE_CULLABLE:	return getImplementation()->isBackFaceCullable();
		case SOLID:				return getImplementation()->isSolid();
	}

	DPVS_API_ASSERT(false && "Model::test() - Invalid property enumeration"); 
	return false;
}



/******************************************************************************
 *
 * Enum:			DPVS::Commander::Command
 *
 * Description:		Enumeration of different commands
 *
 * Notes:			These enumerations are used in the DPVS::Commander::command()
 *					function
 *
 *****************************************************************************/

/*****************************************************************************
 *
 * Function:		DPVS::Commander::Commander()
 *
 * Description:		Constructs a new Commander
 *
 *****************************************************************************/

Commander::Commander(void) : m_imp(null)
{
	DPVS_MUTEX_LOCK
	DPVS_ASSERT_INIT();
	m_imp = new(MALLOC(sizeof(ImpCommander))) ImpCommander(this);
}

/*****************************************************************************
 *
 * Function:		DPVS::Commander::~Commander()
 *
 * Description:		Destructor
 *
 *****************************************************************************/

Commander::~Commander(void)
{
	DPVS_MUTEX_LOCK
	DPVS_ASSERT_INIT();
	DPVS_ASSERT(Memory::isValidPointer(m_imp));
	DELETE(m_imp);
}

/*****************************************************************************
 *
 * Function:		DPVS::Commander::command(Command) = 0
 *
 * Description:		Request commander to perform specified action
 *
 * Parameters:		c = command enumeration
 *
 * Notes:			This function is called by DPVS whenever the application
 *					user should be informed about something (such as an object
 *					being visible or a ROI becoming active/inactive). It is also
 *					used to request actions such as clearing a stencil mask.
 *
 *					No parameters other than the command enumeration are passed.
 *					Instead, depending on the command, the user should query
 *					further information using the DPVS::Commander::getViewer()
 *					and DPVS::Commander::getInstance() calls.
 *
 *					Notes for the different commands:
 *
 *					VIEW_PARAMETERS_CHANGED:
 *
 *					Viewer* v = getViewer();
 *
 *					Set frustum, projection matrix and scissor to your rendering engine.
 *
 * 					INSTANCE_VISIBLE:
 *
 *					Instance* ins = getInstance();
 *				
 *					Render specified object instantly with specified matrix and current viewer parameters.
 *				
 *					NOTE: same object can be reported several times with different matrices
 *						  if there are mirrors or other virtual portals present in the scene.
 * 
 *					REGION_OF_INFLUENCE_ACTIVE:
 *					REGION_OF_INFLUENCE_INACTIVE:
 *
 *					Instance* ins = getInstance();
 *
 *					Toggle on/off ROI from active light list, reflect changes to renderer.
 *
 *					STENCIL_VALUES_CHANGED
 *
 *					Stencil test and write values have changed.
 *					NOTE1: Write value of -1 is invalid and indicates the stencil buffer
 *					       should be disabled.
 *					NOTE2: Test value of -1 is invalid and indicates the subsequent stencil
 *						   writes are unconditional (no test).
 *					NOTE3: The values can currently exceed 255 (hardware limit) in *extreme*
 *						   cases. Such an extreme case is seeing through 256+ consecutive
 *						   portals each one using a stencil mask. That is a very unlikely
 *						   scenario.
 *
 *					STENCIL_MASK:
 *				
 *					Draw object using previously specified stencil test & write mask values.
 *
 *					If your rendering subsystem doens't support stencils (or destination alpha test),
 *					ignore this event.
 *						-> Accurate portal rendering is then impossible and incorrect results
 *						   are evident with floating portals such as mirror cude or portable mirrors.
 *						-> The only known way to solve this issue without stencils is to create
 *						   a complex clipper that clips objects to *any* shape. This approach
 *						   only has theoretical value and is not really an option.
 * 			
 *					NOTE1: Initial z-buffer clear (full screen) is not reported. This is due
 *						   to potential efficiency resons. Determining this condition on
 *						   application side is trivial and DPVS might not be aware
 *						   of all Z-buffer modifications.
 *
 *				    NOTE2: This is never called unless stencil models are used (in portals)
 *		
 *					CLEAR_STENCIL:
 *
 *					Stencil buffer must be cleared (inside view port)
 *
 *					NOTE1: Initial stencil buffer clear (full screen) is not necessary but
 *						   normally this is decoupled with Z-Buffer clear and therefore made
 *						   automatically.
 *
 *					NOTE2: This is never called unless stencil models are used (in portals)
 *
 *					REMOVAL_SUGGESTED:
 * 
 *					Instance* ins = getInstance();
 *
 *					Swap out specified object, if considered appropriate.
 *					This is only a hint and a consequence of calling DPVS::Library::suggestGarbageCollect().
 *					User doesn't need to take action.
 *
 * See Also:		DPVS::Commander::getViewer(), DPVS::Commander::getInstance()
 *
 *****************************************************************************/

/*****************************************************************************
 *
 * Function:		DPVS::Commander::getViewer()
 *
 * Description:		Returns pointer to current viewer object
 *
 * Returns:			pointer to current viewer object or null on failure
 *
 * Notes:			Access to the viewer object is granted only during
 *					certain DPVS::Commander::command() callbacks. See
 *					documentation of the function for further details.
 *
 *					Access to the viewer is available only for the
 *					eduration of the current DPVS::Commander::command()
 *					function!
 *
 *					The function returns null if a viewer isn't
 *					available for the current Commander callback 
 *
 * See Also:		DPVS::Commander::command()
 *
 *****************************************************************************/

const Commander::Viewer* Commander::getViewer(void) const
{
	DPVS_ASSERT_INIT();
	DPVS_API_ASSERT(Memory::isValidPointer(m_imp));
	return m_imp->getViewer();
}

/*****************************************************************************
 *
 * Function:		DPVS::Commander::getInstance()
 *
 * Description:		Returns pointer to instance object
 *
 * Returns:			pointer to current instance object or null on failure
 *
 * Notes:			Access to the instance object is granted only during
 *					certain DPVS::Commander::command() callbacks. See
 *					documentation of the function for further details.
 *
 *					Access to the instance is available only for the
 *					duration of the current DPVS::Commander::command()
 *					function!
 *
 *					The function returns null if an instance isn't
 *					available for the current Commander callback 
 *
 * See Also:		DPVS::Commander::command()
 *
 *****************************************************************************/

const Commander::Instance* Commander::getInstance(void) const
{
	DPVS_ASSERT_INIT();
	DPVS_API_ASSERT(Memory::isValidPointer(m_imp));
	if (!m_imp->isInstanceSpecified())			// no instance specified
		return null;
	return &(m_imp->getInstance());				// return pointer
}

/*****************************************************************************
 *
 * Function:		DPVS::Commander::getTextMessage()
 *
 * Description:		Returns text message
 *
 * Notes:			The string returned must be either processed, copied or
 *					ignored immediately.
 *
 *****************************************************************************/

const char* Commander::getTextMessage	(void) const
{
	DPVS_ASSERT_INIT();
	DPVS_API_ASSERT(Memory::isValidPointer(m_imp));
	return m_imp->getTextMessage();
}

/*****************************************************************************
 *
 * Function:		DPVS::Commander::getLine2D()
 *
 * Description:		Fetches the line data
 *
 * Returns:			Type of the line
 *
 *****************************************************************************/
/*****************************************************************************
 *
 * Function:		DPVS::Commander::getLine3D()
 *
 * Description:		Fetches the line data
 *
 * Returns:			Type of the line
 *
 *****************************************************************************/

Library::LineType	Commander::getLine2D	(Vector2& a, Vector2& b, Vector4& color) const	
{ 	
	DPVS_ASSERT_INIT(); 
	DPVS_API_ASSERT(Memory::isValidPointer(m_imp));
	return m_imp->getLine2D(a,b,color); 
}

Library::LineType	Commander::getLine3D	(Vector3& a, Vector3& b, Vector4& color) const	
{ 
	DPVS_ASSERT_INIT(); 
	DPVS_API_ASSERT(Memory::isValidPointer(m_imp));
	return m_imp->getLine3D(a,b,color); 
}

Library::BufferType	Commander::getBuffer	(const unsigned char*& buf, int &w,int &h) const
{
	DPVS_ASSERT_INIT(); 
	DPVS_API_ASSERT(Memory::isValidPointer(m_imp));
	
	Surface&			s		= Debug::getDefaultSurface();
	Library::BufferType	type	= m_imp->getBuffer(s);

	buf = s.getData();
	w	= s.getWidth();
	h	= s.getHeight();
	
	return type;
}

/*****************************************************************************
 *
 * Function:		DPVS::Commander::getCell()
 *
 * Description:		Returns pointer to current Commander cell
 *
 *****************************************************************************/

Cell* Commander::getCell (void) const
{
	DPVS_ASSERT_INIT(); 
	DPVS_API_ASSERT(Memory::isValidPointer(m_imp));
	return m_imp->getCell();
}

/*****************************************************************************
 *
 * Function:		DPVS::Commander::getStencilValues()
 *
 * Description:		Returns current test and write stencil values
 *
 * Parameters:		test	= reference to int where to store the test stencil value 
 *					write	= reference to int where to store the write stencil value 
 *
 *****************************************************************************/

void Commander::getStencilValues	(int& test,int& write) const
{
	DPVS_ASSERT_INIT(); 
	DPVS_API_ASSERT(Memory::isValidPointer(m_imp));
	m_imp->getStencilValues(test,write);
}

/*****************************************************************************
 *
 * Function:		DPVS::Commander::Instance::getObject()
 *
 * Description:		Returns pointer to corresponding DPVS::Object
 *
 * Returns:			pointer to DPVS::Object
 *
 *****************************************************************************/

class Object* Commander::Instance::getObject(void) const
{
	DPVS_ASSERT_INIT(); 
	DPVS_API_ASSERT	("Commander::Instance::getObject()" && this);
	DPVS_ASSERT		(m_imp);
	return m_imp ? m_imp->getUserObject() : null;
}

/******************************************************************************
 *
 * Category:		Math
 *
 * Class:			DPVS::Frustum
 *
 * Description:		Structure for passing view frustum information
 *
 * Notes:			When used to pass information to perspective projection
 *					routines, the near and far clipping values must be
 *					greater than zero.
 *
 *****************************************************************************/
/******************************************************************************
 *
 * Category:		Math
 *
 * Class:			DPVS::Frustum::Frustum()
 *
 * Description:		Frustum constructor
 *
 * Notes:			Initializes all components to zero.
 *
 *****************************************************************************/

Frustum::Frustum (void) : 
	left(0.0f), 
	right(0.0f), 
	top(0.0f), 
	bottom(0.0f), 
	zNear(0.0f), 
	zFar(0.0f),
	type(Frustum::PERSPECTIVE)
{
	// nada here	
}

/******************************************************************************
 *
 * Category:		Math
 *
 * Class:			DPVS::Vector3i
 *
 * Description:		Structure for holding three integers
 *
 * Notes:			This structure is used mainly to describe triangle
 *					vertex indices
 *
 *****************************************************************************/
/******************************************************************************
 *
 * Category:		Math
 *
 * Class:			DPVS::Vector2
 *
 * Description:		Template class representing two-component vectors
 *
 *****************************************************************************/
/******************************************************************************
 *
 * Category:		Math
 *
 * Class:			DPVS::Vector3T
 *
 * Description:		Template class representing three-component vectors
 *
 *****************************************************************************/
/******************************************************************************
 *
 * Category:		Math
 *
 * Class:			DPVS::Vector4T
 *
 * Description:		Template class representing four-component vectors
 *
 *****************************************************************************/
/******************************************************************************
 *
 * Category:		Math
 *
 * Class:			DPVS::Matrix4x4T
 *
 * Description:		Template class representing a 4x4 transformation matrix
 *
 * Notes:			The matrix ordering (i.e. whether it's column or row
 *					major) is determined during library initialization.
 *
 * See Also:		DPVS::Library::init()
 *
 *****************************************************************************/
/*****************************************************************************
 *
 * Function:		DPVS::Camera::setCameraToCellMatrix (const Matrix4x4 &mtx)
 *
 * Description:		Defines camera->cell transformation matrix
 *
 * Parameters:		mtx = camera->cell transformation matrix (single-precision)
 *
 * Notes:			The matrix may not contain any projection (i.e.
 *					the fourth column/row must be 0,0,0,1)
 *
 *					The matrix format used has been defined in the call to
 *					DPVS::Library::init().
 *
 *					The matrix has been initialized to identity in the
 *					constructor.
 *
 *****************************************************************************/
						
void Camera::setCameraToCellMatrix (const Matrix4x4 &mtx)
{
	DPVS_MUTEX_LOCK
	DPVS_ASSERT_INIT(); 
	DPVS_ASSERT_NO_VISIBILITY_QUERY();
	DPVS_ASSERT_SELF_OK();

	Matrix4x3 cm(NO_CONSTRUCTOR);
	MatrixConverter::convertMatrix(cm,mtx);
	Math::validateMatrix (cm);
	m_imp->setCameraToCell(cm); 
}

/*****************************************************************************
 *
 * Function:		DPVS::Camera::setCameraToCellMatrix (const Matrix4x4d &mtx)
 *
 * Description:		Defines camera->cell transformation matrix
 *
 * Parameters:		mtx = camera->cell transformation matrix (double-precision)
 *
 * Notes:			The matrix may not contain any projection (i.e.
 *					the fourth column/row must be 0,0,0,1)
 *
 *					The matrix format used has been defined in the call to
 *					DPVS::Library::init().
 *
 *					The matrix has been initialized to identity in the
 *					constructor.
 *
 *****************************************************************************/

void Camera::setCameraToCellMatrix (const Matrix4x4d &mtx)
{
	DPVS_MUTEX_LOCK
	DPVS_ASSERT_INIT(); 
	DPVS_ASSERT_NO_VISIBILITY_QUERY();
	DPVS_ASSERT_SELF_OK();

	Matrix4x3 cm(NO_CONSTRUCTOR);
	MatrixConverter::convertMatrix(cm,mtx);
	Math::validateMatrix (cm);
	m_imp->setCameraToCell(cm); 
}

/*****************************************************************************
 *
 * Function:		DPVS::Camera::getCameraToCellMatrix (Matrix4x4& mtx) const
 *
 * Description:		Returns camera->cell transformation matrix
 *
 * Parameters:		mtx = reference to matrix structure (single-precision) where result is stored
 *
 * Notes:			The matrix format used has been defined in the call to
 *					DPVS::Library::init().
 *
 *					The matrix has been initialized to identity in the
 *					constructor.
 *
 *****************************************************************************/

void Camera::getCameraToCellMatrix (Matrix4x4& mtx) const
{
	DPVS_MUTEX_LOCK
	DPVS_ASSERT_INIT(); 
	DPVS_ASSERT_SELF_OK();
	MatrixConverter::convertMatrix(mtx,m_imp->getCameraToCell());
}

/*****************************************************************************
 *
 * Function:		DPVS::Camera::getCameraToCellMatrix(Matrix4x4d& mtx) const
 *
 * Description:		Returns camera->cell transformation matrix
 *
 * Parameters:		mtx = reference to matrix structure (double-precision) where result is stored
 *
 * Notes:			The matrix format used has been defined in the call to
 *					DPVS::Library::init().
 *
 *					The matrix has been initialized to identity in the
 *					constructor.
 *
 *****************************************************************************/

void Camera::getCameraToCellMatrix (Matrix4x4d& mtx) const
{
	DPVS_MUTEX_LOCK
	DPVS_ASSERT_INIT(); 
	DPVS_ASSERT_SELF_OK();
	MatrixConverter::convertMatrix(mtx,m_imp->getCameraToCell());
}

/*****************************************************************************
 *
 * Function:		DPVS::Camera::getCameraToWorldMatrix(Matrix4x4& mtx) const
 *
 * Description:		Returns camera->world transformation matrix
 *
 * Parameters:		mtx = reference to matrix structure (single-precision) where result is stored
 *
 * Notes:			The matrix format used has been defined in the call to
 *					DPVS::Library::init().
 *
 *****************************************************************************/

void Camera::getCameraToWorldMatrix	(Matrix4x4& mtx) const
{
	DPVS_MUTEX_LOCK
	DPVS_ASSERT_INIT(); 
	DPVS_ASSERT_SELF_OK();
	Matrix4x3 tmp(m_imp->getCameraToCell());
	tmp.productFromLeft(m_imp->getCell()->getImplementation()->getCellToWorld());
	MatrixConverter::convertMatrix(mtx, tmp);
}

/*****************************************************************************
 *
 * Function:		DPVS::Camera::getCameraToWorldMatrix(Matrix4x4d &mtx) const
 *
 * Description:		Returns camera->world transformation matrix
 *
 * Parameters:		mtx = reference to matrix structure (double-precision) where result is stored
 *
 * Notes:			The matrix format used has been defined in the call to
 *					DPVS::Library::init().
 *
 *****************************************************************************/

void Camera::getCameraToWorldMatrix	(Matrix4x4d& mtx) const
{
	DPVS_MUTEX_LOCK
	DPVS_ASSERT_INIT(); 
	DPVS_ASSERT_SELF_OK();
	Matrix4x3 tmp(m_imp->getCameraToCell());
	tmp.productFromLeft(m_imp->getCell()->getImplementation()->getCellToWorld());
	MatrixConverter::convertMatrix(mtx, tmp);
}

/*****************************************************************************
 *
 * Function:		DPVS::Cell::setCellToWorldMatrix(const Matrix4x4& )
 *
 * Description:		Defines cell->world transformation matrix
 *
 * Parameters:		mtx = reference to cell->world matrix (single-precision) where result is stored
 *
 * Notes:			The matrix format used has been defined in the call to
 *					DPVS::Library::init().
 *
 *					The matrix has been initialized to identity in the
 *					constructor.
 *
 *****************************************************************************/

void Cell::setCellToWorldMatrix (const Matrix4x4& mtx)	
{
	DPVS_MUTEX_LOCK
	DPVS_ASSERT_INIT(); 
	DPVS_ASSERT_NO_VISIBILITY_QUERY();
	DPVS_ASSERT_SELF_OK();

	Matrix4x3 cm(NO_CONSTRUCTOR);
	MatrixConverter::convertMatrix(cm,mtx);
	Math::validateMatrix (cm);
	getImplementation()->setCellToWorld(cm); 
}

/*****************************************************************************
 *
 * Function:		DPVS::Cell::setCellToWorldMatrix(const Matrix4x4d& )
 *
 * Description:		Defines cell->world transformation matrix
 *
 * Parameters:		mtx = reference to cell->world matrix (double-precision)
 *
 * Notes:			The matrix format used has been defined in the call to
 *					DPVS::Library::init().
 *
 *					The matrix has been initialized to identity in the
 *					constructor.
 *
 *****************************************************************************/

void Cell::setCellToWorldMatrix (const Matrix4x4d& mtx)	
{
	DPVS_MUTEX_LOCK
	DPVS_ASSERT_INIT(); 
	DPVS_ASSERT_NO_VISIBILITY_QUERY();
	DPVS_ASSERT_SELF_OK();

	Matrix4x3 cm(NO_CONSTRUCTOR);
	MatrixConverter::convertMatrix(cm,mtx);
	Math::validateMatrix (cm);
	getImplementation()->setCellToWorld(cm); 
}

/*****************************************************************************
 *
 * Function:		DPVS::Cell::getCellToWorldMatrix(const Matrix4x4& )
 *
 * Description:		Returns cell->world transformation matrix
 *
 * Parameters:		mtx = reference to matrix structure (single-precision) where result is stored
 *
 * Notes:			The matrix format used has been defined in the call to
 *					DPVS::Library::init().
 *
 *					The matrix has been initialized to identity in the
 *					constructor.
 *
 *****************************************************************************/

void Cell::getCellToWorldMatrix (Matrix4x4& mtx) const	
{ 
	DPVS_MUTEX_LOCK
	DPVS_ASSERT_INIT(); 
	DPVS_ASSERT_SELF_OK();

	MatrixConverter::convertMatrix(mtx,getImplementation()->getCellToWorld()); 
}

/*****************************************************************************
 *
 * Function:		DPVS::Cell::getCellToWorldMatrix(const Matrix4x4d& )
 *
 * Description:		Returns cell->world transformation matrix
 *
 * Parameters:		mtx = reference to matrix structure (double-precision) where result is stored
 *
 * Notes:			The matrix format used has been defined in the call to
 *					DPVS::Library::init().
 *
 *					The matrix has been initialized to identity in the
 *					constructor.
 *
 *****************************************************************************/

void Cell::getCellToWorldMatrix (Matrix4x4d& mtx) const	
{ 
	DPVS_MUTEX_LOCK
	DPVS_ASSERT_INIT(); 
	DPVS_ASSERT_SELF_OK();
	MatrixConverter::convertMatrix(mtx,getImplementation()->getCellToWorld()); 
}

/*****************************************************************************
 *
 * Function:		DPVS::Cell::getWorldToCellMatrix(const Matrix4x4& )
 *
 * Description:		Returns world->cell transformation matrix
 *
 * Parameters:		mtx = reference to matrix structure (single-precision) where result is stored
 *
 * Notes:			The matrix format used has been defined in the call to
 *					DPVS::Library::init().
 *
 *****************************************************************************/

void Cell::getWorldToCellMatrix (Matrix4x4&  mtx) const	
{ 
	DPVS_MUTEX_LOCK
	DPVS_ASSERT_INIT(); 
	DPVS_ASSERT_SELF_OK();
	MatrixConverter::convertMatrix(mtx,getImplementation()->getWorldToCell()); 
}

/*****************************************************************************
 *
 * Function:		DPVS::Cell::getWorldToCellMatrix(const Matrix4x4d& )
 *
 * Description:		Returns world->cell transformation matrix
 *
 * Parameters:		mtx = reference to matrix structure (double-precision) where result is stored
 * 
 * Notes:			The matrix format used has been defined in the call to
 *					DPVS::Library::init().
 *
 *****************************************************************************/

void Cell::getWorldToCellMatrix (Matrix4x4d& mtx) const	
{ 
	DPVS_MUTEX_LOCK
	DPVS_ASSERT_INIT(); 
	DPVS_ASSERT_SELF_OK();

	MatrixConverter::convertMatrix(mtx,getImplementation()->getWorldToCell()); 
}

/*****************************************************************************
 *
 * Function:		DPVS::Object::setObjectToCellMatrix(const Matrix4x4&)
 *
 * Description:		Defines object->cell transformation matrix
 *
 * Parameters:		mtx = reference to object->cell matrix (single-precision)
 *
 * Notes:			The matrix may not contain any projection (i.e.
 *					the fourth column/row must be 0,0,0,1)
 *
 *					The matrix format used has been defined in the call to
 *					DPVS::Library::init(). The matrix is initialized to
 *					identity in the object constructor.
 *
 *****************************************************************************/

void Object::setObjectToCellMatrix (const Matrix4x4& mtx)		
{ 
	DPVS_MUTEX_LOCK
	DPVS_ASSERT_INIT(); 
	DPVS_ASSERT_SELF_OK();
//	DPVS_ASSERT_NO_VISIBILITY_QUERY();
	Matrix4x3 cm(NO_CONSTRUCTOR);
	MatrixConverter::convertMatrix(cm,mtx);		
	Math::validateMatrix (cm);
	getImplementation()->setObjectToCell(cm); 
}

/*****************************************************************************
 *
 * Function:		DPVS::Object::setObjectToCellMatrix(const Matrix4x4d&)
 *
 * Description:		Defines object->cell transformation matrix
 *
 * Parameters:		mtx = reference to object->cell matrix (double-precision)
 *
 * Notes:			The matrix may not contain any projection (i.e.
 *					the fourth column/row must be 0,0,0,1)
 *
 *					The matrix format used has been defined in the call to
 *					DPVS::Library::init(). The matrix is initialized to
 *					identity in the object constructor.
 *
 *****************************************************************************/

void Object::setObjectToCellMatrix(const Matrix4x4d &mtx)		
{ 
	DPVS_MUTEX_LOCK
	DPVS_ASSERT_INIT(); 
	DPVS_ASSERT_SELF_OK();
//	DPVS_ASSERT_NO_VISIBILITY_QUERY();
	Matrix4x3 cm(NO_CONSTRUCTOR);
	MatrixConverter::convertMatrix(cm,mtx);
	Math::validateMatrix (cm);
	getImplementation()->setObjectToCell(cm); 
}

/*****************************************************************************
 *
 * Function:		DPVS::Object::getObjectToCellMatrix(Matrix4x4& mtx) const
 *
 * Description:		Returns object->cell transformation matrix
 *
 * Parameters:		mtx = reference to matrix structure (single-precision)
 *
 * Notes:			The matrix format used has been defined in the call to
 *					DPVS::Library::init().
 *					
 *					The matrix is initialized to identity in the object constructor.
 *
 ******************************************************************************/

void Object::getObjectToCellMatrix	(Matrix4x4& mtx) const
{
	DPVS_MUTEX_LOCK
	DPVS_ASSERT_INIT(); 
	DPVS_ASSERT_SELF_OK();
	MatrixConverter::convertMatrix(mtx,getImplementation()->getObjectToCell());
}

/*****************************************************************************
 *
 * Function:		DPVS::Object::getObjectToCellMatrix(Matrix4x4d &mtx) const
 *
 * Description:		Returns object->cell transformation matrix
 *
 * Parameters:		mtx = reference to matrix structure (double-precision)
 *
 * Notes:			The matrix format used has been defined in the call to
 *					DPVS::Library::init().
 *
 *					The matrix is initialized to identity in the object constructor.
 *
 *****************************************************************************/

void Object::getObjectToCellMatrix (Matrix4x4d& mtx) const
{
	DPVS_MUTEX_LOCK
	DPVS_ASSERT_INIT(); 
	DPVS_ASSERT_SELF_OK();
	MatrixConverter::convertMatrix(mtx,getImplementation()->getObjectToCell());
}

/*****************************************************************************
 *
 * Function:		DPVS::VirtualPortal::setWarpMatrix(const Matrix4x4&)
 *
 * Description:		Assigns transformation matrix used for portal-to-portal
 *					traversal.
 *
 * Parameters:		mtx = reference to warp matrix (single-precision)
 *
 * Notes:			The matrix may not contain any projection (i.e.
 *					the fourth column/row must be 0,0,0,1)
 *
 *					The matrix format used has been defined in the call to
 *					DPVS::Library::init().
 *
 *					The matrix is initialized to identity in the object constructor.
 *
 *****************************************************************************/

void VirtualPortal::setWarpMatrix	(const Matrix4x4& mtx)
{
	DPVS_MUTEX_LOCK
	DPVS_ASSERT_INIT(); 
	DPVS_ASSERT_SELF_OK();
//	DPVS_ASSERT_NO_VISIBILITY_QUERY();
	Matrix4x3 cm(NO_CONSTRUCTOR);
	MatrixConverter::convertMatrix(cm,mtx);
	Math::validateMatrix (cm);
	((ImpVirtualPortal*)getImplementation())->setWarpMatrix(cm);
}

/*****************************************************************************
 *
 * Function:		DPVS::VirtualPortal::setWarpMatrix(const Matrix4x4d&)
 *
 * Description:		Assigns transformation matrix used for portal-to-portal
 *					traversal.
 *
 * Parameters:		mtx = reference to warp matrix (double-precision)
 *
 * Notes:			The matrix may not contain any projection (i.e.
 *					the fourth column/row must be 0,0,0,1)
 *
 *					The matrix format used has been defined in the call to
 *					DPVS::Library::init().
 *
 *					The matrix is initialized to identity in the object constructor.
 *
 *****************************************************************************/

void VirtualPortal::setWarpMatrix	(const Matrix4x4d& mtx)
{
	DPVS_MUTEX_LOCK
	DPVS_ASSERT_INIT(); 
	DPVS_ASSERT_SELF_OK();
//	DPVS_ASSERT_NO_VISIBILITY_QUERY();
	Matrix4x3 cm(NO_CONSTRUCTOR);
	MatrixConverter::convertMatrix(cm,mtx);
	Math::validateMatrix (cm);
	((ImpVirtualPortal*)getImplementation())->setWarpMatrix(cm);
}

/*****************************************************************************
 *
 * Function:		DPVS::VirtualPortal::getWarpMatrix(const Matrix4x4&)
 *
 * Description:		Returns transformation matrix used for portal-to-portal
 *					traversal.
 *
 * Parameters:		mtx = reference to warp matrix (single-precision)
 *
 * Notes:			The matrix format used has been defined in the call to
 *					DPVS::Library::init().
 *
 *					The matrix is initialized to identity in the object constructor.
 *
 *****************************************************************************/

void VirtualPortal::getWarpMatrix	(Matrix4x4& mtx) const
{
	DPVS_MUTEX_LOCK
	DPVS_ASSERT_INIT(); 
	DPVS_ASSERT_SELF_OK();
	MatrixConverter::convertMatrix(mtx, ((ImpVirtualPortal*)getImplementation())->getWarpMatrix());
}

/*****************************************************************************
 *
 * Function:		DPVS::VirtualPortal::getWarpMatrix(const Matrix4x4d&)
 *
 * Description:		Returns transformation matrix used for portal-to-portal
 *					traversal.
 *
 * Parameters:		mtx = reference to warp matrix (double-precision)
 *
 * Notes:			The matrix format used has been defined in the call to
 *					DPVS::Library::init().
 *
 *					The matrix is initialized to identity in the object constructor.
 *
 *****************************************************************************/

void VirtualPortal::getWarpMatrix	(Matrix4x4d& mtx) const
{
	DPVS_MUTEX_LOCK
	DPVS_ASSERT_INIT(); 
	DPVS_ASSERT_SELF_OK();
	ImpVirtualPortal* imp = (ImpVirtualPortal*)getImplementation();
	MatrixConverter::convertMatrix(mtx, imp->getWarpMatrix());
}

/*****************************************************************************
 *
 * Function:		DPVS::Commander::Instance::getObjectToCameraMatrix(Matrix4x4&)
 *
 * Description:		Returns object->camera transformation matrix of the instance
 *
 * Parameters:		mtx = reference to matrix structure (single-precision)
 *
 * Notes:			The matrix format used has been defined in the call to
 *					DPVS::Library::init().
 *
 *					This matrix corresponds with the OpenGL concept of a
 *					"modelview" matrix.
 *
 *****************************************************************************/
				
void Commander::Instance::getObjectToCameraMatrix	(Matrix4x4& dd) const		//without projection
{
	DPVS_MUTEX_LOCK
	DPVS_ASSERT_INIT(); 
	DPVS_API_ASSERT	("Commander::Instance::getObjectToCameraMatrix()" && this);
	DPVS_ASSERT		(m_imp);

	const Matrix4x3& o2c = m_imp->getObjectToCell();
	const Matrix4x3& c2c = DataPasser::getViewerCamera()->getCellToCamera();

	Math::productFromLeft(dd,o2c,c2c);	

	MatrixConverter::convertMatrix(dd);				// In-place format conversion (maybe?)
}

/*****************************************************************************
 *
 * Function:		DPVS::Commander::Instance::getObjectToCameraMatrix(Matrix4x4d&)
 *
 * Description:		Returns object->camera transformation matrix of the instance
 *
 * Parameters:		mtx = reference to matrix structure (double-precision)
 *
 * Notes:			The matrix format used has been defined in the call to
 *					DPVS::Library::init().
 *
 *					This matrix corresponds with the OpenGL concept of a
 *					"modelview" matrix.
 *
 *****************************************************************************/

void Commander::Instance::getObjectToCameraMatrix	(Matrix4x4d& mtx) const		//without projection
{
	DPVS_MUTEX_LOCK
	DPVS_ASSERT_INIT(); 
	DPVS_API_ASSERT	("Commander::Instance::getObjectToCameraMatrix()" && this);
	DPVS_ASSERT		(m_imp);

	const Matrix4x3& o2c = m_imp->getObjectToCameraMatrix( DataPasser::getViewerCamera() );
	MatrixConverter::convertMatrix(mtx, o2c);
}

/*****************************************************************************
 *
 * Function:		DPVS::Commander::Instance::getProjectionSize()
 *
 * Description:		Estimates projection size of the object
 *
 * Parameters:		p = reference to projection structure
 *
 * Returns:			true if 'p' is valid (object is at least partially inside view frustum), false otherwise 
 *
 * Notes:			left/right/top/bottom are clamped to viewport rectangle
 *
 *****************************************************************************/

bool Commander::Instance::getProjectionSize (Projection& p) const
{
	DPVS_ASSERT_INIT(); 
	DPVS_API_ASSERT	("Commander::Instance::getProjectionSize()" && this);
	DPVS_ASSERT		(m_imp);

	ImpCamera*		c	= DataPasser::getViewerCamera();

	if (m_imp->isUnbounded())							// handle unbounded objects first...
	{
		p.left   = Math::intFloor(c->getRasterViewport().x0);
		p.right  = Math::intFloor(c->getRasterViewport().x1);
		p.top    = Math::intFloor(c->getRasterViewport().y0);
		p.bottom = Math::intFloor(c->getRasterViewport().y1);
		p.zNear  = 0.0f;
		p.zFar   = 1.0f;
		return true;
	}

	ImpModel*	m	= m_imp->getTestModel();
	
	DPVS_ASSERT(m && c);

	FloatRectangle	rect;
	Range<float>	depthRange;

	if(!m || !m->getRectangle_IM(rect,depthRange,m_imp->getObjectToCameraMatrix(c),c) )
		return false;

	p.left		= Math::intFloor(rect.x0);	
	p.right		= Math::intFloor(rect.x1);	
	p.top		= Math::intFloor(rect.y0);	// is this inclusive??
	p.bottom	= Math::intFloor(rect.y1);
	p.zNear		= depthRange.getMin();
	p.zFar		= depthRange.getMax();

	return true;
}

/*****************************************************************************
 *
 * Function:		DPVS::Commander::Instance::getImportance()
 *
 * Description:		Estimates importance of an object 
 *
 * Returns:			value in range [0,1] where 1.0 indicates full importance.
 *					
 * Notes:			The importance is estimated by calculating the 
 *					cumulative importance decay terms of the portal
 *					sequence through which the object is visible. If the
 *					object is in the same cell as the camera (and not seen through
 *					a mirror), the importance is thus 1.0.
 *
 * See Also:		DPVS::PhysicalPortal::setImportanceDecay()
 *
 *****************************************************************************/

float Commander::Instance::getImportance (void) const
{
	DPVS_ASSERT_INIT(); 
	return DataPasser::getObjectImportance();
}

/*****************************************************************************
 *
 * Function:		DPVS::Commander::Instance::getClipMask()
 *
 * Description:		Returns conservatively correct clip mask of an object
 *
 * Returns:			a 32-bit mask indicating which clip planes the object penetrates
 *
 *****************************************************************************/

DPVS::UINT32 Commander::Instance::getClipMask (void) const
{
	DPVS_ASSERT_INIT(); 
	DPVS_ASSERT(m_imp);
	return m_imp->getUserClipMask();	// ZERO plane has been removed
}

//------------------------------------------------------------------------
