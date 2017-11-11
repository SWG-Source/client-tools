// ======================================================================
//
// Camera.cpp
// Portions copyright 1998 Bootprint Entertainment
// Portions copyright 2001 Sony Online Entertainment Inc.
// All Rights Reserved.
//
// ======================================================================

#include "clientGraphics/FirstClientGraphics.h"
#include "clientGraphics/Camera.h"

#include "clientGraphics/DebugPrimitive.h"
#include "clientGraphics/Graphics.h"
#include "clientGraphics/GraphicsDebugFlags.h"
#include "clientGraphics/Light.h"
#include "clientGraphics/ShaderPrimitiveSorter.h"
#include "sharedDebug/DebugFlags.h"
#include "sharedMath/Plane.h"
#include "sharedMath/Sphere.h"
#include "sharedObject/Appearance.h"

#include <limits>

#ifdef _DEBUG
#include <vector>
#endif

// ======================================================================

#ifdef _DEBUG
bool Camera::reportCamera;
#endif

// ======================================================================

void Camera::install()
{
#ifdef _DEBUG
	DebugFlags::registerFlag(reportCamera,       "ClientGraphics", "reportCamera");
#endif
}

// ======================================================================
/**
 * Construct a camera.
 */

Camera::Camera()
: Object(),
	viewportX0(0),
	viewportY0(0),
	viewportWidth(Graphics::getCurrentRenderTargetWidth()),
	viewportHeight(Graphics::getCurrentRenderTargetHeight()),
	oneOverViewportWidth(1.0f),
	oneOverViewportHeight(1.0f),
	nearPlane(1),
	farPlane(5000),
	horizontalFieldOfView(PI / CONST_REAL(3)),
	verticalFieldOfView(0),
	tanOfHalfHorizontalFov(1.0f),
	tanOfHalfVerticalFov(1.0f),
	frustumVolume(FP_Max),
	frustumVolume_w(FP_Max),
	projectionMode(PM_Perspective),
	left(CONST_REAL(0)),
	top(CONST_REAL(0)),
	right(CONST_REAL(1)),
	bottom(CONST_REAL(1)),
	m_underWater(false)
#ifdef _DEBUG
	,
	numberOfSphereTests(0),
	numberOfObjectsAdded(0),
	debugPrimitives(NULL)
#endif
//lint -save -esym(1926, Camera::frustumVertex, Camera::frustumPlane) // default constructur implicitly called
{
//lint -restore
	setup(); //lint !e1506 // call to virtual function within constructor
}

// ----------------------------------------------------------------------
/**
 * Destroy a camera.
 */

Camera::~Camera(void)
{
#ifdef _DEBUG
	delete debugPrimitives;
#endif
}

// ----------------------------------------------------------------------
/**
 * Setup the camera.
 * 
 * This will rebuild all the necessary data for rendering using the camera.
 * This will be called when the camera data changes, such as field of view or the
 * viewport.
 */

void Camera::setup(void)
{
	if (projectionMode == PM_Perspective)
	{
		//-- setup the perspective projection

		if (viewportWidth)
			oneOverViewportWidth = 1.0f / static_cast<float>(viewportWidth);
		else
			oneOverViewportWidth = std::numeric_limits<float>::max();

		if (viewportHeight)
			oneOverViewportHeight = 1.0f / static_cast<float>(viewportHeight);
		else
			oneOverViewportHeight = std::numeric_limits<float>::max();

		// calculate the vertical FOV based on the horizontal fov
		verticalFieldOfView    = horizontalFieldOfView * real(viewportHeight) / real(viewportWidth);

		tanOfHalfHorizontalFov = tan(horizontalFieldOfView * CONST_REAL(0.5));
		tanOfHalfVerticalFov   = tan(verticalFieldOfView * CONST_REAL(0.5));

		const real xTan  = tanOfHalfHorizontalFov;
		const real yTan  = tanOfHalfVerticalFov;
		const real xNear = nearPlane * xTan;
		const real yNear = nearPlane * yTan;
		const real xFar  = farPlane  * xTan;
		const real yFar  = farPlane  * yTan;

		frustumVertex[FV_NearUpperLeft ].set(-xNear,  yNear, nearPlane);
		frustumVertex[FV_NearUpperRight].set( xNear,  yNear, nearPlane);
		frustumVertex[FV_NearLowerRight].set( xNear, -yNear, nearPlane);
		frustumVertex[FV_NearLowerLeft ].set(-xNear, -yNear, nearPlane);
		frustumVertex[FV_FarUpperLeft  ].set(-xFar,   yFar,  farPlane);
		frustumVertex[FV_FarUpperRight ].set( xFar,   yFar,  farPlane);
		frustumVertex[FV_FarLowerRight ].set( xFar,  -yFar,  farPlane);
		frustumVertex[FV_FarLowerLeft  ].set(-xFar,  -yFar,  farPlane);

		frustumVolume.setPlane(FP_Near,   Plane(Vector::negativeUnitZ,  nearPlane));
		frustumVolume.setPlane(FP_Far,    Plane(Vector::unitZ,         -farPlane));
		frustumVolume.setPlane(FP_Top,    Plane(Vector::zero, frustumVertex[4], frustumVertex[5]));
		frustumVolume.setPlane(FP_Right,  Plane(Vector::zero, frustumVertex[5], frustumVertex[6]));
		frustumVolume.setPlane(FP_Bottom, Plane(Vector::zero, frustumVertex[6], frustumVertex[7]));
		frustumVolume.setPlane(FP_Left,   Plane(Vector::zero, frustumVertex[7], frustumVertex[4]));

		// the following code was ripped out of the DX 6.0 help
		// DirectX Foundation/Direct3D Immediate Mode/D3D Immediate Mode Essentials/The Geometry Pipeline/The Projection Transformation/Setting Up a Projection Matrix
		const real   w          = cot(horizontalFieldOfView * 0.5f);
		const real   h          = cot(verticalFieldOfView * 0.5f);
		const real   Q          = farPlane/(farPlane - nearPlane);

		projectionMatrix.matrix[0][0] = w;
		projectionMatrix.matrix[0][1] = 0;
		projectionMatrix.matrix[0][2] = 0;
		projectionMatrix.matrix[0][3] = 0;

		projectionMatrix.matrix[1][0] = 0;
		projectionMatrix.matrix[1][1] = h;
		projectionMatrix.matrix[1][2] = 0;
		projectionMatrix.matrix[1][3] = 0;

		projectionMatrix.matrix[2][0] = 0;
		projectionMatrix.matrix[2][1] = 0;
		projectionMatrix.matrix[2][2] = Q;
		projectionMatrix.matrix[2][3] = -nearPlane*Q;

		projectionMatrix.matrix[3][0] = 0;
		projectionMatrix.matrix[3][1] = 0;
		projectionMatrix.matrix[3][2] = 1;
		projectionMatrix.matrix[3][3] = 0;
	}
	else if (projectionMode == PM_Parallel)
	{
		//-- setup parallel projection matrix

		// map camera-space upper left (left, top) to view frustum upper left
		// map camera-space lower right (right, bottom) to view frustum lower right
		// projected view frustum is (-1,-1,0) .. (1,1,1)
		const real dx   = right - left;
		const real oodx = CONST_REAL(1)/dx;

		const real dy   = bottom - top;
		const real oody = CONST_REAL(1)/dy;

		DEBUG_FATAL(WithinEpsilonInclusive(CONST_REAL(0), farPlane - nearPlane, CONST_REAL(0)), ("farPlane and nearPlane must not be equal [%f]", farPlane));
		const real oodz = CONST_REAL(1)/(farPlane - nearPlane);

		// xp = projected x, xc = camera-space x
		// xp = (2 * (xc - left) / dx) - 1
		//    = [2/dx] xc + [-2*left/dx - 1]
		projectionMatrix.matrix[0][0] = CONST_REAL(2) * oodx;
		projectionMatrix.matrix[0][1] = CONST_REAL(0);
		projectionMatrix.matrix[0][2] = CONST_REAL(0);
		projectionMatrix.matrix[0][3] = CONST_REAL(-2) * left * oodx - CONST_REAL(1);

		// yp = projected y, yc = camera-space y
		// yp = -( (2 * (yc - top) / dy) - 1)  // negative needed because we have +y going down, projected space needs +y going up
		//    = 2 * (top - yc) / dy  + 1
		//    = [-2/dy] yc + [2*top/dy + 1]
		projectionMatrix.matrix[1][0] = CONST_REAL(0);
		projectionMatrix.matrix[1][1] = CONST_REAL(-2) * oody;
		projectionMatrix.matrix[1][2] = CONST_REAL(0);
		projectionMatrix.matrix[1][3] = CONST_REAL(2) * top * oody + CONST_REAL(1);

		// zp = projected z, zc = camera-space z
		// zp = (zc - znear) / dz
		//    = [1/dz] zc + [-znear/dz]
		projectionMatrix.matrix[2][0] = CONST_REAL(0);
		projectionMatrix.matrix[2][1] = CONST_REAL(0);
		projectionMatrix.matrix[2][2] = oodz;
		projectionMatrix.matrix[2][3] = -nearPlane * oodz;

		projectionMatrix.matrix[3][0] = CONST_REAL(0);
		projectionMatrix.matrix[3][1] = CONST_REAL(0);
		projectionMatrix.matrix[3][2] = CONST_REAL(0);
		projectionMatrix.matrix[3][3] = CONST_REAL(1);

		//-- setup camera-space frustum vertices
		frustumVertex[FV_NearUpperLeft ].set( left,    top, nearPlane);
		frustumVertex[FV_NearUpperRight].set(right,    top, nearPlane);
		frustumVertex[FV_NearLowerRight].set(right, bottom, nearPlane);
		frustumVertex[FV_NearLowerLeft ].set( left, bottom, nearPlane);
		frustumVertex[FV_FarUpperLeft  ].set( left,    top,  farPlane);
		frustumVertex[FV_FarUpperRight ].set(right,    top,  farPlane);
		frustumVertex[FV_FarLowerRight ].set(right, bottom,  farPlane);
		frustumVertex[FV_FarLowerLeft  ].set( left, bottom,  farPlane);

		//-- setup camera-space frustum volume
		frustumVolume.setPlane(FP_Near,   Plane(Vector::negativeUnitZ,  nearPlane));
		frustumVolume.setPlane(FP_Far,    Plane(Vector::unitZ,         -farPlane));

		frustumVolume.setPlane(FP_Top,    Plane(Vector::unitY,         -top));
		frustumVolume.setPlane(FP_Bottom, Plane(Vector::negativeUnitY,  bottom));

		frustumVolume.setPlane(FP_Right,  Plane(Vector::unitX,          right));
		frustumVolume.setPlane(FP_Left,   Plane(Vector::negativeUnitX, -left));
	}
	else
	{
		FATAL(true, ("unknown projection mode [%u]", projectionMode));
	}

	_updateWorldFrustumVolume();
}

// ----------------------------------------------------------------------

/**
 * Get the camera viewport .
 * 
 * @param vpx0 The left edge of the viewport
 * @param vpx1 The right edge of the viewport
 * @param vpy0 The top edge of the viewport
 * @param vpy1 The bottom edge of the viewport
 */

void Camera::getViewport(int &vpx0, int &vpy0, int &vpx1, int &vpy1) const
{
	vpx0 = getViewportX0();
	vpy0 = getViewportY0();
	vpx1 = vpx0 + getViewportWidth() - 1;
	vpy1 = vpy0 + getViewportHeight() - 1;
}

// ----------------------------------------------------------------------

/**
 * Get the camera viewport .
 * 
 * @param vpx0 The left edge of the viewport
 * @param vpx1 The right edge of the viewport
 * @param vpy0 The top edge of the viewport
 * @param vpy1 The bottom edge of the viewport
 */

void Camera::getViewport(float &vpx0, float &vpy0, float &vpx1, float &vpy1) const
{
	vpx0 = static_cast<float>(getViewportX0());
	vpy0 = static_cast<float>(getViewportY0());
	vpx1 = vpx0 + static_cast<float>(getViewportWidth() - 1);
	vpy1 = vpy0 + static_cast<float>(getViewportHeight() - 1);
}

// ----------------------------------------------------------------------
/**
 * Set a new viewport for the camera.
 * 
 * @param newX0  The left edge of the viewport
 * @param newY0  The top edge of the viewport
 * @param width  The viewport width
 * @param height  The viewport height
 */

void Camera::setViewport(int newX0, int newY0, int width, int height)
{
	DEBUG_FATAL(newX0 < 0, ("bad x0 %d < 0", newX0));
	DEBUG_FATAL(newY0 < 0, ("bad y0 %d < 0", newY0));
	DEBUG_FATAL(width <= 0, ("bad width %d <= 0", width));
	DEBUG_FATAL(height <= 0, ("bad height %d <= 0", height));

	viewportX0 = newX0;
	viewportY0 = newY0;
	viewportWidth = width;
	viewportHeight = height;

	setup();
}

// ----------------------------------------------------------------------
/**
 * Set the near clipping plane.
 * 
 * @param newNearPlane  Distance to the near clipping plane
 */

void Camera::setNearPlane(real newNearPlane)
{
	nearPlane = newNearPlane;
	setup();
}

// ----------------------------------------------------------------------
/**
 * Set the far clipping plane.
 * 
 * @param newFarPlane  Distance to the far clipping plane
 */

void Camera::setFarPlane(real newFarPlane)
{
	farPlane = newFarPlane;
	setup();
}

// ----------------------------------------------------------------------
/**
 * Set a new horizontal field of view for the camera.
 * 
 * Small field-of-views can have the affect of a zoom.  Large field-of-views
 * may cause a fish-eye type effect.
 * 
 * The vertical field-of-view will be calculated based on the horizontal
 * field of view and the viewport aspect ratio.
 * 
 * @param newFieldOfView  New hoizontal field-of-view
 */

void Camera::setHorizontalFieldOfView(real newFieldOfView)
{
	horizontalFieldOfView = newFieldOfView;
	setup();
}

// ----------------------------------------------------------------------
/**
 * Test a camera-space sphere's inclusion within the view volume.
 * 
 * A point can be tested to be within the view volume by specifying a radius of zero.
 * 
 * @param sphere  Sphere in camera space
 * @return True if any part of the sphere is contained within the view volume, otherwise false
 */

bool Camera::testVisibility_c(const Sphere &sphere) const
{
#ifdef _DEBUG
	++numberOfSphereTests;
#endif

	return frustumVolume.intersects(sphere);
}

// ----------------------------------------------------------------------
/**
 * Test a world-space sphere's inclusion within the view volume.
 * 
 * A point can be tested to be within the view volume by specifying a radius of zero.
 * 
 * @param sphere  Sphere in world space
 * @return True if any part of the sphere is contained within the view volume, otherwise false
 */

bool Camera::testVisibility_w(const Sphere &sphere) const
{
#ifdef _DEBUG
	++numberOfSphereTests;
#endif
	return getWorldFrustumVolume().intersects(sphere);
}

// ----------------------------------------------------------------------
/**
 * Project a 3d vector onto the screen.
 * 
 * If the return value is true, the x and y values will be updated
 * to indicate the position of the vector in the viewport.  If the return
 * value is false, the contents of x and y are undefined.
 *
 * If z is non-null, the returned value is the z value that should be
 * presented as the z buffer value for pre-transformed vertices.
 * 
 * @arg z  if non-null, will be set to the normalized z buffer value (typically between 0.0 and 1.0)
 * 
 * @return True if the point is visible, otherwise false.
 */

bool Camera::projectInCameraSpace(const Vector &vector, real *x, real *y, real *z, bool testSides) const
{
	DEBUG_FATAL(!x, ("x may not be null"));
	DEBUG_FATAL(!y, ("y may not be null"));

	// trivial near and far plane clipping
	if (vector.z < nearPlane || vector.z > farPlane)
		return false;

	//-- find x and y in projected space
	const real projectedW = projectionMatrix.matrix[3][0] * vector.x + projectionMatrix.matrix[3][1] * vector.y + projectionMatrix.matrix[3][2] * vector.z + projectionMatrix.matrix[3][3];

	DEBUG_FATAL(WithinEpsilonInclusive(CONST_REAL(0), projectedW, CONST_REAL(0)), ("projected w was zero"));
	const real ooProjectedW = CONST_REAL(1) / projectedW;

	const real projectedX   = (projectionMatrix.matrix[0][0] * vector.x + projectionMatrix.matrix[0][1] * vector.y + projectionMatrix.matrix[0][2] * vector.z + projectionMatrix.matrix[0][3]) * ooProjectedW;
	const real projectedY   = (projectionMatrix.matrix[1][0] * vector.x + projectionMatrix.matrix[1][1] * vector.y + projectionMatrix.matrix[1][2] * vector.z + projectionMatrix.matrix[1][3]) * ooProjectedW;

	//-- reject against clip-space view frustum cube
	if (testSides)
	{
		if (!WithinRangeInclusiveInclusive(CONST_REAL(-1), projectedX, CONST_REAL(1)) ||
		    !WithinRangeInclusiveInclusive(CONST_REAL(-1), projectedY, CONST_REAL(1)))
		{
			// point is outside the view frustum
			return false;
		}
	}

	//-- transform projected point into viewport
	const real halfViewportWidth  = static_cast<real>(viewportWidth)  * CONST_REAL(0.5);
	const real halfViewportHeight = static_cast<real>(viewportHeight) * CONST_REAL(0.5);

	*x = (projectedX * halfViewportWidth)   + static_cast<real>(viewportX0) + halfViewportWidth;
	*y = (projectedY * -halfViewportHeight) + static_cast<real>(viewportY0) + halfViewportHeight;

	if (z)
		*z = (projectionMatrix.matrix[2][0] * vector.x + projectionMatrix.matrix[2][1] * vector.y + projectionMatrix.matrix[2][2] * vector.z + projectionMatrix.matrix[2][3]) * ooProjectedW;

	return true;
}

//----------------------------------------------------------------------

/**
* This projection function allows the caller to find out the raw projected x and y values in case
* the projection fails.  If the projection fails, one of the x or y values may still be valid.
*
* @param projectedX  [-1,1]
* @param projectedY  [-1,1]
* @param projected  true if the projection was performed on projectedX and projectedY
* @return true if the projection is within the viewport
*/

bool Camera::projectInCameraSpace (const Vector &vector, float & x, float & y, float * z, float & projectedX, float & projectedY, bool & projectedOk) const
{
	projectedOk = false;

	// trivial near and far plane clipping
	if (vector.z < nearPlane || vector.z > farPlane)
		return false;

	//-- find x and y in projected space
	const real projectedW = projectionMatrix.matrix[3][0] * vector.x + projectionMatrix.matrix[3][1] * vector.y + projectionMatrix.matrix[3][2] * vector.z + projectionMatrix.matrix[3][3];

	DEBUG_FATAL(WithinEpsilonInclusive(0.0f, projectedW, 0.0f), ("projected w was zero"));
	const real ooProjectedW = CONST_REAL(1) / projectedW;

	projectedX   = (projectionMatrix.matrix[0][0] * vector.x + projectionMatrix.matrix[0][1] * vector.y + projectionMatrix.matrix[0][2] * vector.z + projectionMatrix.matrix[0][3]) * ooProjectedW;
	projectedY   = (projectionMatrix.matrix[1][0] * vector.x + projectionMatrix.matrix[1][1] * vector.y + projectionMatrix.matrix[1][2] * vector.z + projectionMatrix.matrix[1][3]) * ooProjectedW;

	projectedOk = true;
	const bool xOk = WithinRangeInclusiveInclusive(-1.0f, projectedX, 1.0f);
	const bool yOk = WithinRangeInclusiveInclusive(-1.0f, projectedY, 1.0f);

	if (!xOk && !yOk)
		return false;

	//-- transform projected point into viewport
	const float halfViewportWidth  = static_cast<float>(viewportWidth)  * 0.5f;
	const float halfViewportHeight = static_cast<float>(viewportHeight) * 0.5f;

	x = (projectedX * halfViewportWidth)   + static_cast<real>(viewportX0) + halfViewportWidth;
	y = (projectedY * -halfViewportHeight) + static_cast<real>(viewportY0) + halfViewportHeight;

	if (z)
		*z = (projectionMatrix.matrix[2][0] * vector.x + projectionMatrix.matrix[2][1] * vector.y + projectionMatrix.matrix[2][2] * vector.z + projectionMatrix.matrix[2][3]) * ooProjectedW;

	return xOk && yOk;
}

// ----------------------------------------------------------------------
/**
 * Compute a point in camera space of pixel in the viewport.
 * 
 * This routine returns a point on the near plane through which a ray from
 * the camera passes that will penetrate the specified pixel in the viewport.
 * 
 * @return Point in camera space of pixel in the viewport
 */

const Vector Camera::reverseProjectInViewportSpace(int x, int y) const
{
//	DEBUG_FATAL(x < 0 || y < 0 || x >= viewportWidth || y >= viewportHeight, ("invalid viewport position %d/%d %d/%d", x, viewportWidth, y, viewportHeight));

	// pierce the center of the pixel
	const real deltaX = (static_cast<real>(x) + CONST_REAL(0.5)) / static_cast<real>(viewportWidth);
	const real deltaY = (static_cast<real>(y) + CONST_REAL(0.5)) / static_cast<real>(viewportHeight);

	if (projectionMode == PM_Perspective)
	{
		return Vector(
			(getFrustumVertex(FV_NearUpperRight).x - getFrustumVertex(FV_NearUpperLeft).x) * deltaX + getFrustumVertex(FV_NearUpperLeft).x,
			(getFrustumVertex(FV_NearLowerLeft).y  - getFrustumVertex(FV_NearUpperLeft).y) * deltaY + getFrustumVertex(FV_NearUpperLeft).y,
			nearPlane);
	}
	else if (projectionMode == PM_Parallel)
	{
		const real cameraX = deltaX * (right - left) + left;
		const real cameraY = deltaY * (bottom - top) + top;

		return Vector(cameraX, cameraY, nearPlane);
	}
	else
	{
		DEBUG_FATAL(true, ("unknown projection mode [%u]", projectionMode));
		return Vector::zero; //lint !e527 // unreachable // yes, for MSVC
	}
}

// ----------------------------------------------------------------------
/**
 * Compute a point in camera space of a pixel in the viewport, where
 * the z in camera space is given.
 * 
 * This routine returns a point at the given camera z distance through which a ray from
 * the camera passes that will penetrate the specified pixel in the viewport.
 * 
 * @return Point in camera space of pixel in the viewport at camera z distance
 */

const Vector Camera::reverseProjectInViewportSpace(float xViewport, float yViewport, float zCamera) const
{
	//-- figure out fraction of x,y for the viewport position
	const float xFraction = ( 2.0f * (xViewport + 0.5f) - static_cast<float>(viewportWidth)) * oneOverViewportWidth;
	const float yFraction = (-2.0f * (yViewport + 0.5f) + static_cast<float>(viewportHeight)) * oneOverViewportHeight;

	if (projectionMode == PM_Perspective)
	{
		const real xCamera = xFraction * (zCamera * tanOfHalfHorizontalFov);
		const real yCamera = yFraction * (zCamera * tanOfHalfVerticalFov);
		return Vector(xCamera, yCamera, zCamera);
	}
	else if (projectionMode == PM_Parallel)
	{
		DEBUG_FATAL(true, ("to do")); // @todo -TRF-
		return Vector::zero;  //lint !e527 // unreachable // yes, for MSVC
	}
	else
	{
		DEBUG_FATAL(true, ("unknown projection mode [%u]", projectionMode));
		return Vector::zero; //lint !e527 // unreachable // yes, for MSVC
	}
}

// ----------------------------------------------------------------------
/**
 * This routine updates the frustum volume in world space whenever the camera is moved
 */
void Camera::positionChanged(bool dueToParentChange, const Vector &oldPosition)
{
	_updateWorldFrustumVolume();
	Object::positionChanged(dueToParentChange, oldPosition);
}

// ----------------------------------------------------------------------
/**
 * This routine updates the frustum volume in world space whenever the camera is moved
 */
void Camera::rotationChanged(bool dueToParentChange)
{
	_updateWorldFrustumVolume();
	Object::rotationChanged(dueToParentChange);
}

// ----------------------------------------------------------------------
/**
 * This routine updates the frustum volume in world space whenever the camera is moved
 */
void Camera::positionAndRotationChanged(bool dueToParentChange, const Vector &oldPosition)
{
	_updateWorldFrustumVolume();
	Object::positionChanged(dueToParentChange, oldPosition);
}

// ----------------------------------------------------------------------
/**
 * This routine updates the frustum volume in world space
 *
 */
void Camera::_updateWorldFrustumVolume()
{
	const Transform &objectToWorld = getTransform_o2w();

	//-- frustum volume in world space (used for culling)
	frustumVolume_w.transform(getFrustumVolume(), objectToWorld);

	//-- frustum volume vertices in world space (used for culling).
	for (int i=0;i<FV_Max;i++)
	{
		frustumVertex_w[i]=objectToWorld.rotateTranslate_l2p(frustumVertex[i]);
	}
}

// ----------------------------------------------------------------------
/**
 * This routine sets the camera's data into the Graphics class.
 *
 */
void Camera::applyState() const
{
	// invert the objectToWorld matrix
	const Transform &objectToWorld = getTransform_o2w();
	Transform worldToCamera(Transform::IF_none);
	worldToCamera.invert(objectToWorld);

	// make sure the Gl is rendering with the proper camera
	Graphics::setWorldToCameraTransform(worldToCamera, objectToWorld.getPosition_p());
	Graphics::setProjectionMatrix(projectionMatrix);
	Graphics::setViewport(viewportX0, viewportY0, viewportWidth, viewportHeight);
}

// ----------------------------------------------------------------------
/**
 * Start a 3d scene.
 * 
 * This routine starts a 3d scene.  Objects may now be added to the scene.  endScene()
 * must be called at the end of the 3d scene.
 * 
 * @see Camera::endScene()
 */

void Camera::beginScene(void) const
{
	applyState();

	ShaderPrimitiveSorter::setCurrentCamera(*this);

#ifdef _DEBUG
	numberOfSphereTests = 0;
	numberOfObjectsAdded = 0;
#endif
}

// ----------------------------------------------------------------------
/**
 * Add all objects to the 3d scene.
 * 
 * This function call is only valid between a beginScene() and an endScene() pair.
 * 
 * @see Camera::beginScene(), Camera::endScene()
 */

void Camera::drawScene(void) const
{
	DEBUG_FATAL(true, ("no need to call this function"));
}

// ----------------------------------------------------------------------
/**
 * Add a light to the rendered scene.
 * 
 * The light is only in effect until the end of the scene.
 * 
 * @param light  Light to add to the scene
 */

#ifdef _DEBUG

void Camera::addDebugPrimitive(DebugPrimitive *debugPrimitive) const
{
	if (!debugPrimitives)
		debugPrimitives = new DebugPrimitives;

	debugPrimitives->push_back(debugPrimitive);
}

#endif

// ----------------------------------------------------------------------
/**
 * End a 3d scene.
 * 
 * This call ends a 3d scene that was started with beginScene().
 * 
 * This will also remove all lights from the Gl that were added during
 * the scene's rendering.
 * 
 * @see Camera::beginScene()
 */

void Camera::endScene(void) const
{
	ShaderPrimitiveSorter::clearCurrentCamera();

#ifdef _DEBUG
	if (debugPrimitives)
	{
		DebugPrimitives::iterator end = debugPrimitives->end();
		for (DebugPrimitives::iterator i = debugPrimitives->begin(); i != end; ++i)
		{
			(*i)->render();
			delete *i;
		}
		debugPrimitives->clear();
	}
#endif

#ifdef _DEBUG
	DEBUG_REPORT_PRINT(reportCamera, ("cam: %d=tested %d=added\n", numberOfSphereTests, numberOfObjectsAdded));
#endif
}

// ----------------------------------------------------------------------
/**
 * Render a 3d scene.
 * 
 * This routine will simply call beginScene(), addObjectsToScene() and endScene()
 * 
 * @see Camera::beginScene(), Camera::addObjectsToScene(), Camera::endScene(),
 */

void Camera::renderScene(void) const
{
	if (isActive())
	{
		beginScene();
		drawScene();
		endScene();
	}
}

// ----------------------------------------------------------------------
/**
 * place the camera in parallel projection mode and set the camera
 * coordinates that map into the view frustum
 *
 * the near and far z values come from setting the near and far
 * clipping plane.  there is no order dependency between calling
 * this function and the setNearPlane() and setFarPlane() functions.
 *
 * once this function is called, the camera is in parallel-projection mode.
 * 
 * @param left    left-most value for camera-space x mapping to view frustum
 * @param top     upper-most value for camera-space y mapping to view frustum
 * @param right   rigth-most value for camera-space x mapping to view frustum
 * @param bottom  lower-most value for camera-space y mapping to view frustum
 *
 * @see Camera::getProjectionMode()
 */

void Camera::setParallelProjection(real newLeft, real newTop, real newRight, real newBottom)
{
	left           = newLeft;
	top            = newTop;
	right          = newRight;
	bottom         = newBottom;
	projectionMode = PM_Parallel;

	DEBUG_FATAL(WithinEpsilonInclusive(CONST_REAL(0), right - left, CONST_REAL(0)), ("right and left cannot be equal [%f]", right));
	DEBUG_FATAL(WithinEpsilonInclusive(CONST_REAL(0), top - bottom, CONST_REAL(0)), ("top and bottom cannot be equal [%f]", top));

	setup();
}

// ----------------------------------------------------------------------
/**
 * compute camera space radius in screen space at the specified position
 */

bool Camera::computeRadiusInScreenSpace(const Vector &vector, const float radius, float &screenRadius) const
{
	Vector position = vector;

	//-- Adjust z if center point happens to be too near the camera but edge of object is not.
	if (position.z < nearPlane)
		position.z += radius;
	else if (position.z > farPlane)
		position.z -= radius;

	float screenX0;
	float screenY;
	if (projectInCameraSpace(position, &screenX0, &screenY, 0, false))
	{
		Vector position2  = vector;
		position2.x      += radius;

		float screenX1;
		IGNORE_RETURN(projectInCameraSpace(position2, &screenX1, &screenY, 0, false));
		screenRadius = abs (screenX1 - screenX0);

		return true;
	}

	return false;
}

//-------------------------------------------------------------------

void Camera::setUnderWaterThisFrame (const bool underWater)
{
	m_underWater = underWater;
}

// ======================================================================

