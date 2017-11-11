// ======================================================================
//
// RenderWorld.h
// Copyright 2001 Sony Online Entertainment
// All Rights Reserved.
//
// ======================================================================

#ifndef INCLUDED_RenderWorld_H
#define INCLUDED_RenderWorld_H

// ======================================================================

class Appearance;
class AxialBox;
class CellProperty;
class IndexedTriangleList;
class Sphere;
class RenderWorldCamera;
class RenderWorldCommander;
class Light;
class NetworkId;
class Object;
class ObjectList;
class ProfilerBlock;
class Transform;
class Vector;

namespace DPVS
{
	class Camera;
	class Cell;
	class Matrix4x4;
	class MeshModel;
	class Model;
	class OBBModel;
	class Object;
	class ProfilerBlock;
	class RegionOfInfluence;
	class SphereModel;
};

// ======================================================================

class RenderWorld
{
public:

	class CellNotification;
	class OcclusionNotification;

	friend class CellNotification;
	friend class OcclusionNotification;
	friend class RenderWorldCommander;

	typedef stdvector<const CellProperty *>::fwd CellPropertyList;

public:

	static void                     install();

	static void                     addObjectNotifications(Object & object);
	static void                     addCellNotifications(Object & object);

	static DPVS::Model             *fetchDefaultModel();
	static DPVS::SphereModel       *fetchSphereModel(float radius);
	static DPVS::SphereModel       *fetchSphereModel(const Sphere &sphere);
	static DPVS::OBBModel          *fetchBoxModel(const AxialBox &box);
	static DPVS::MeshModel         *fetchMeshModel(const IndexedTriangleList &indexedTriangleList, bool clockwiseWindingOrder = true);

	static DPVS::Object            *createObject(Appearance *owner, DPVS::Model *testModel);
	static DPVS::Object            *createObject(Appearance *owner, float testModelSphereRadius);
	static DPVS::Object            *createObject(Appearance *owner, const AxialBox &box);
	static DPVS::Object            *createObject(Appearance *owner, DPVS::Model *testModel, DPVS::Model *writeModel);
	static DPVS::Object            *createUnboundedObject(Appearance *owner);

	static DPVS::RegionOfInfluence *createRegionOfInfluence(Object *object, DPVS::Model *testModel);
	static DPVS::RegionOfInfluence *createRegionOfInfluence(Object *object, float testModelSphereRadius);
	static DPVS::RegionOfInfluence *createUnboundedRegionOfInfluence(Object *object);

	static void                     addObjectToWorld(DPVS::Object *dpvsObject, Object *owner);
	static void                     removeObjectFromWorld(DPVS::Object *dpvsObject);
	static bool                     isObjectInWorld(DPVS::Object *dpvsObject);

	static void                     convertToTransform(const DPVS::Matrix4x4 &source, Transform &destination);
	static void                     convertToDpvsTransform(const Transform &source, const Vector &scale, DPVS::Matrix4x4 &destination, Object const & debugObject);

	static void                     addWorldEnvironmentObject(Object * object);
	static void                     addMoveWithCameraWorldEnvironmentObject(Object * object);
	static void                     removeWorldEnvironmentObject(Object * object);
	static void                     addWorldEnvironmentLight(Light * light);
	static void                     removeWorldEnvironmentLight(Light * light);
	static void                     setClearDepthAndStencilBufferAfterRenderingEnvironment(bool clearDepthAndStencilBufferAfterRenderingEnvironment);

	static void                     recursivelyDisableDpvsObjectsForThisRender(const Object *object);
	static void                     disableDpvsObjectForThisRender(DPVS::Object *dpvsObject);
	static void                     drawScene(const RenderWorldCamera &camera);
	static const Vector             getCameraPosition();

	static const CellPropertyList & getVisibleCells();

	static ProfilerBlock           &getDpvsQueryProfilerBlock();

	static void setDisableOcclusionCulling(bool disableOcclusionCulling);

	static bool wasObjectRenderedThisFrame(NetworkId const & id);

private:

	static void                     remove();
	static void                     leakedObject(DPVS::Object *leak);
	static void                     cellVisible(const CellProperty * cellProperty);

	static int                      getViewportX0();
	static int                      getViewportY0();

private:

	// disable
	RenderWorld();
	RenderWorld(const RenderWorld &);
	RenderWorld &operator =(const RenderWorld &);
};

// ======================================================================

#endif
