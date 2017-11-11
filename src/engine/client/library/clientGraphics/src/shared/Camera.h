// ======================================================================
//
// Camera.h
// Portions copyright 1998 Bootprint Entertainment
// Portions copyright 2001 Sony Online Entertainment Inc.
// All Rights Reserved.
//
// ======================================================================

#ifndef INCLUDED_Camera_H
#define INCLUDED_Camera_H

// ======================================================================

class DebugPrimitive;
class Light;

#include "sharedObject/Object.h"
#include "sharedObject/ObjectList.h"
#include "sharedMath/Volume.h"
#include "sharedMath/PackedArgb.h"
#include "clientGraphics/Graphics.def"

// ======================================================================

class Camera : public Object
{
	// @todo remove this friendship
	friend class BlueprintTextureRendererTemplate;
	friend class DrawTextureCommandElement;

public:

	enum FrustumVertex
	{
		FV_NearUpperLeft  = 0,
		FV_NearUpperRight = 1,
		FV_NearLowerRight = 2,
		FV_NearLowerLeft  = 3,
		FV_FarUpperLeft   = 4,
		FV_FarUpperRight  = 5,
		FV_FarLowerRight  = 6,
		FV_FarLowerLeft   = 7,

		FV_Max            = 8
	};

	enum FrustumPlane
	{
		FP_Near   = 0,
		FP_Far    = 1,
		FP_Top    = 2,
		FP_Right  = 3,
		FP_Bottom = 4,
		FP_Left   = 5,

		FP_Max    = 6
	};

	enum ProjectionMode
	{
		PM_Perspective,
		PM_Parallel
	};

protected:

#ifdef _DEBUG
	static bool reportCamera;
#endif

private:

	GlMatrix4x4               projectionMatrix;

	int                       viewportX0;
	int                       viewportY0;
	int                       viewportWidth;
	int                       viewportHeight;
	float                     oneOverViewportWidth;
	float                     oneOverViewportHeight;

	real                      nearPlane;
	real                      farPlane;

	real                      horizontalFieldOfView;
	real                      verticalFieldOfView;
	float                     tanOfHalfHorizontalFov;
	float                     tanOfHalfVerticalFov;

	Vector                    frustumVertex[FV_Max];
	Volume                    frustumVolume;
	mutable Vector            frustumVertex_w[FV_Max];
	mutable Volume            frustumVolume_w;

	ProjectionMode            projectionMode;
	real                      left;
	real                      top;
	real                      right;
	real                      bottom;

	bool                      m_underWater;

#ifdef _DEBUG
	mutable int               numberOfSphereTests;
	mutable int               numberOfObjectsAdded;

	typedef stdvector<DebugPrimitive *>::fwd  DebugPrimitives;
	mutable DebugPrimitives  *debugPrimitives;
#endif

protected:

	virtual void beginScene(void) const;
	virtual void drawScene(void) const = 0;
	virtual void endScene(void) const;

	virtual void positionChanged(bool dueToParentChange, const Vector &oldPosition);
	virtual void rotationChanged(bool dueToParentChange);
	virtual void positionAndRotationChanged(bool dueToParentChange, const Vector &oldPosition);

	void         _updateWorldFrustumVolume();

protected:

	virtual void  setup(void);

private:

	// disabled
	Camera(const Camera &);
	Camera &operator =(const Camera &);

public:

	static void install();

public:

	Camera();
	virtual ~Camera(void);

#ifdef _DEBUG
	void              addDebugPrimitive(DebugPrimitive *debugPrimitive) const;
#endif

	void              applyState() const;

	void              renderScene(void) const;

	bool              testVisibility_c(const Sphere &sphere) const;
	bool              testVisibility_w(const Sphere &sphere) const;

	bool              projectInCameraSpace(const Vector &vector, float & x, float & y, float * z, float & projectedX, float & projectedY, bool & projectedOk) const;
	bool              projectInCameraSpace(const Vector &vector, real *x, real *y, real *z, bool testSides = true) const;
	bool              projectInWorldSpace(const Vector &vector, float & x, float & y, float * z, float & projectedX, float & projectedY, bool & projectedOk) const;
	bool              projectInWorldSpace(const Vector &vector, real *x, real *y, real *z, bool testSides = true) const;
	bool              computeRadiusInScreenSpace(const Vector &vector, float radius, float& screenRadius) const;

	const Vector      reverseProjectInViewportSpace(int x, int y) const;
	const Vector      reverseProjectInScreenSpace(int x, int y) const;

	const Vector      reverseProjectInViewportSpace(float xViewport, float yViewport, float zCamera) const;
	const Vector      reverseProjectInScreenSpace(float xViewport, float yViewport, float zCamera) const;

	int               getViewportX0(void) const;
	int               getViewportY0(void) const;
	int               getViewportWidth(void) const;
	int               getViewportHeight(void) const;
	void              getViewport(int &x0, int &y0, int &x1, int &y1) const;
	void              getViewport(float &x0, float &y0, float &x1, float &y1) const;
	void              setViewport(int x0, int y0, int width, int height);

	void              setNearPlane(real newNearPlane);
	void              setFarPlane(real newFarPlane);
	real              getNearPlane(void) const;
	real              getFarPlane(void) const;
	void              setHorizontalFieldOfView(real newFieldOfView);
	real              getHorizontalFieldOfView(void) const;
	real              getVerticalFieldOfView(void) const;

	const Vector     &getFrustumVertex(FrustumVertex whichVertex) const;
	const Vector     *getFrustumVertices(void) const;
	int               getNumberOfFrustumVertices(void) const;
	const Vector     &getFrustumVertex_w(FrustumVertex whichVertex) const;
	const Vector     *getFrustumVertices_w(void) const;

	const Volume     &getFrustumVolume() const;
	const Volume     &getWorldFrustumVolume() const;

	void               setParallelProjection(real newLeft, real newTop, real newRight, real newBottom);
	ProjectionMode     getProjectionMode() const;
	const GlMatrix4x4 &getProjectionMatrix() const;

	void setUnderWaterThisFrame (bool newUnderWater);
	bool isUnderWater() const;
};

// ======================================================================
/**
 * Return the left edge of the camera's viewport.
 * 
 * Together with the getViewportY0(), getViewportWidth(), and getViewportHeight(),
 * these functions describe the camera's on-screen rendering area.
 * 
 * @return The screen-space coordinate of the left edge of the camera's viewport.
 * @see Camera::getViewportY0(), Camera::getViewportWidth(), Camera::getViewportHeight()
 */

inline int Camera::getViewportX0(void) const
{
	return viewportX0;
}

// ----------------------------------------------------------------------
/**
 * Return the top edge of the camera's viewport.
 * 
 * Together with the getViewportX0(), getViewportWidth(), and getViewportHeight(),
 * these functions describe the camera's on-screen rendering area.
 * 
 * @return The screen-space coordinate of the top edge of the camera's viewport.
 * @see Camera::getViewportX0(), Camera::getViewportWidth(), Camera::getViewportHeight()
 */

inline int Camera::getViewportY0(void) const
{
	return viewportY0;
}

// ----------------------------------------------------------------------
/**
 * Return the width of the camera's viewport.
 * 
 * Together with the getViewportX0(), getViewportY0(), and getViewportHeight(),
 * these functions describe the camera's on-screen rendering area.
 * 
 * @return The pixel width of the camera's viewport.
 * @see Camera::getViewportX0(), Camera::getViewportY0(), Camera::getViewportHeight()
 */

inline int Camera::getViewportWidth(void) const
{
	return viewportWidth;
}

// ----------------------------------------------------------------------
/**
 * Return the height of the camera's viewport.
 * 
 * Together with the getViewportX0(), getViewportY0(), and getViewportWidth(),
 * these functions describe the camera's on-screen rendering area.
 * 
 * @return The pixel width of the camera's viewport.
 * @see Camera::getViewportX0(), Camera::getViewportY0(), Camera::getViewportWidth()
 */

inline int Camera::getViewportHeight(void) const
{
	return viewportHeight;
}

// ----------------------------------------------------------------------
/**
 * Get the distance to the near plane.
 * 
 * @return The distance to the near plane
 */

inline real Camera::getNearPlane(void) const
{
	return nearPlane;
}

// ----------------------------------------------------------------------
/**
 * Get the distance to the far plane.
 * 
 * @return The distance to the far plane
 */

inline real Camera::getFarPlane(void) const
{
	return farPlane;
}

// ----------------------------------------------------------------------
/**
 * Get the horizontal field of view.
 * 
 * @return The horizontal field of view of the camera
 */

inline real Camera::getHorizontalFieldOfView(void) const
{
	return horizontalFieldOfView;
}

// ----------------------------------------------------------------------
/**
 * Get the vertical field of view.
 * 
 * @return The vertical field of view of the camera
 */

inline real Camera::getVerticalFieldOfView(void) const
{
	return verticalFieldOfView;
}

// ----------------------------------------------------------------------
/**
 * Get a specific frustum vertex.  The vertex is in camera space.
 * 
 * @param whichVertex  The frustum vertex to retrieve
 * @return The specified frustum vertex
 */

inline const Vector &Camera::getFrustumVertex(FrustumVertex whichVertex) const
{
	DEBUG_FATAL(static_cast<int>(whichVertex) > static_cast<int>(FV_Max), ("Camera::getFrustumVertex out of range %u", static_cast<int>(whichVertex)));
	return frustumVertex[static_cast<int>(whichVertex)];
}

// ----------------------------------------------------------------------
/**
 * Get a specific frustum vertex.  The vertex is in world space.
 * 
 * @param whichVertex  The frustum vertex to retrieve
 * @return The specified frustum vertex
 */

inline const Vector &Camera::getFrustumVertex_w(FrustumVertex whichVertex) const
{
	DEBUG_FATAL(static_cast<int>(whichVertex) > static_cast<int>(FV_Max), ("Camera::getFrustumVertex_w out of range %u", static_cast<int>(whichVertex)));
	return frustumVertex_w[static_cast<int>(whichVertex)];
}

// ----------------------------------------------------------------------
/**
 * Get a pointer to the array of all frustum vertices.  The vertices are in camera space.
 * 
 * @return Pointer to the array of all frustum vertices in camera space
 */

inline const Vector *Camera::getFrustumVertices(void) const
{
	return frustumVertex;
}

// ----------------------------------------------------------------------
/**
 * Get a pointer to the array of all frustum vertices.  The vertices are in world space.
 * 
 * @return Pointer to the array of all frustum vertices in world space
 */

inline const Vector *Camera::getFrustumVertices_w(void) const
{
	return frustumVertex_w;
}

// ----------------------------------------------------------------------
/**
 * Get the number of vertices in the frustum vertex array.
 * 
 * @return The number of vertices in the frustum vertex array
 */

inline int Camera::getNumberOfFrustumVertices(void) const
{
	return static_cast<int>(FV_Max);
}

// ----------------------------------------------------------------------
/**
 * Get the volume describing the camera's frustum.  The volume is in camera space.
 *
 * @return The Volume describing the camera's frustum.
 */

inline const Volume &Camera::getFrustumVolume() const
{
	return frustumVolume;
}

// ----------------------------------------------------------------------
/**
 * Get the volume describing the camera's frustum.  The volume is in world space.
 *
 * @return The Volume describing the camera's frustum in world space.
 */

inline const Volume &Camera::getWorldFrustumVolume() const
{
	return frustumVolume_w;
}

// ----------------------------------------------------------------------
/**
 * Project a 3d vector onto the screen.
 * 
 * @return True if the point is visible, otherwise false.
 * @see projectInCameraSpace()
 */

inline bool Camera::projectInWorldSpace(const Vector &vector, real *x, real *y, real *z, bool testSides) const
{
	return projectInCameraSpace(rotateTranslate_w2o(vector), x, y, z, testSides);
}

// ----------------------------------------------------------------------
/**
 * Project a 3d vector onto the screen.
 * 
 * @return True if the point is visible, otherwise false.
 * @see projectInCameraSpace()
 */

//----------------------------------------------------------------------

inline bool Camera::projectInWorldSpace(const Vector &vector, float & x, float & y, float * z, float & projectedX, float & projectedY, bool & projectedOk) const
{
	return projectInCameraSpace(rotateTranslate_w2o(vector), x, y, z, projectedX, projectedY, projectedOk);
}

// ----------------------------------------------------------------------
/**
 * Compute a point in camera space of pixel in the screen.
 * 
 * This routine returns a point on the near plane through which a ray from
 * the camera passes that will penetrate the specified pixel in the viewport.
 * 
 * @return Point in camera space of pixel in the screen
 */

inline const Vector Camera::reverseProjectInScreenSpace(int x, int y) const
{
	return reverseProjectInViewportSpace(x - viewportX0, y - viewportY0);
}

// ----------------------------------------------------------------------
/**
 * Compute a point in camera space of a pixel on the screen, where
 * the z in camera space is given.
 * 
 * This routine returns a point at the given camera z distance through which a ray from
 * the camera passes that will penetrate the specified pixel on the screen.
 * 
 * @return Point in camera space of pixel on the screen at camera z distance
 */

inline const Vector Camera::reverseProjectInScreenSpace(float xViewport, float yViewport, float zCamera) const
{
	return reverseProjectInViewportSpace(xViewport - static_cast<float>(viewportX0), yViewport - static_cast<float>(viewportY0), zCamera);
}

// ----------------------------------------------------------------------
/**
 * retrieve the current projection mode for the camera.
 *
 * @see Camera::setParallelProjection()
 */

inline Camera::ProjectionMode Camera::getProjectionMode() const
{
	return projectionMode;
}

// ----------------------------------------------------------------------
/**
 * retrieve the projection matrix for the camera.
 */

inline const GlMatrix4x4 &Camera::getProjectionMatrix() const
{
	return projectionMatrix;
}

//----------------------------------------------------------------------

inline bool Camera::isUnderWater() const
{
	return m_underWater;
}

// ======================================================================

#endif
