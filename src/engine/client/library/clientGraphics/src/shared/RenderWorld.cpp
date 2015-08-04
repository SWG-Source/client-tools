// ======================================================================
//
// RenderWorld.cpp
// Copyright 2001 Sony Online Entertainment
// All Rights Reserved.
//
// ======================================================================

#include "clientGraphics/FirstClientGraphics.h"
#include "clientGraphics/RenderWorld.h"

#include "clientGraphics/ConfigClientGraphics.h"
#include "clientGraphics/DebugPrimitive.h"
#include "clientGraphics/Graphics.h"
#include "clientGraphics/RenderWorldCamera.h"
#include "clientGraphics/RenderWorldCommander.h"
#include "clientGraphics/RenderWorldServices.h"
#include "clientGraphics/RenderWorld_CellNotification.h"
#include "clientGraphics/RenderWorld_OcclusionNotification.h"
#include "clientGraphics/ShaderPrimitiveSorter.h"
#include "clientGraphics/Light.h"

#include "sharedMath/AxialBox.h"
#include "sharedMath/Sphere.h"
#include "sharedMath/IndexedTriangleList.h"
#include "sharedDebug/DebugFlags.h"
#include "sharedDebug/Profiler.h"
#include "sharedFoundation/ExitChain.h"
#include "sharedFoundation/Production.h"
#include "sharedObject/Appearance.h"
#include "sharedObject/CellProperty.h"
#include "sharedObject/ObjectList.h"
#include "sharedObject/Portal.h"

#include "dpvsCamera.hpp"
#include "dpvsCell.hpp"
#include "dpvsObject.hpp"
#include "dpvsModel.hpp"

#include <vector>

#define DO_OBJECT_TRACKING 0

#if DO_OBJECT_TRACKING
#include <map>
#endif

// ======================================================================

#define VECTOR_TO_V3(a)  (reinterpret_cast<const DPVS::Vector3 *>(&a)[0])

// ======================================================================

namespace RenderWorldNamespace
{
	class PortalAppearance : public Appearance
	{
	public:

		PortalAppearance(Object *objectToDisable);
		virtual void preRender() const;

	private:

		PortalAppearance();
		PortalAppearance(const PortalAppearance &);
		PortalAppearance &operator =(const PortalAppearance &);

	private:
		Watcher<Object> m_objectToDisable;
	};

	typedef std::vector<DPVS::Object *>          DpvsObjects;

	bool                                         ms_installed;
	bool                                         ms_disableObjectMinimumCoverage = true;
	bool                                         ms_clearDepthAndStencilBufferAfterRenderingEnvironment;
	bool                                         ms_disableOcclusionCulling;
#ifdef _DEBUG
	bool                                         ms_disableEnvironment;
	bool                                         ms_disableResolveVisibility;
	bool                                         ms_forceDisableOcclusionCulling;
	bool                                         ms_disablePortalTraversal;
	bool                                         ms_disableViewFrustumCulling;
	bool                                         ms_lockViewFrustum;
	bool                                         ms_renderAxisAlignedBoundingBoxes;
	bool                                         ms_renderFrustum;
	bool                                         ms_renderObjectAlignedBoundingBoxes;
	bool                                         ms_renderPortals;
	bool                                         ms_renderSilhouettes;
	bool                                         ms_renderTestSilhouettes;
	bool                                         ms_renderVoxels;
	bool                                         ms_reportMetrics;
	bool                                         ms_checkConsistencyBeforeRender;
#endif

// #if PRODUCTION == 0
	ProfilerBlock                                ms_dpvsQueryProfilerBlock("dpvs query and immediate callbacks");
// #endif


	RenderWorld::CellNotification                ms_cellNotification;
	RenderWorld::OcclusionNotification           ms_occlusionNotification;
	RenderWorldCommander *                       ms_commander;
	RenderWorldServices *                        ms_services;
	const Camera *                               ms_camera;
	const CellProperty *                         ms_cameraCell;
	Vector                                       ms_cameraFrustum[8];
	Transform                                    ms_cameraToWorld;
	int                                          ms_cameraViewportWidth;
	int                                          ms_cameraViewportHeight;
	int                                          ms_lastCameraViewportWidth;
	int                                          ms_lastCameraViewportHeight;
	uint                                         ms_lastCullingParameters;
	DPVS::Cell *                                 ms_dpvsCameraCell;
	DPVS::Camera *                               ms_dpvsCamera;
	ObjectList                                   ms_worldEnvironmentObjects;
	ObjectList                                   ms_moveWithCameraWorldEnvironmentObjects;
	ObjectList                                   ms_worldEnvironmentLights;
	std::vector<DPVS::Object *>                  ms_excludedDpvsObjects;

	DPVS::Model                                 *ms_defaultModel;

	RenderWorld::CellPropertyList                ms_visibleCellList;

	void                  remove();
	void                  clearVisibleCells();
	void                  inWorldAddDpvsObject(Object *object, DPVS::Object *dpvsObject);
	void                  inWorldRemoveDpvsObject(DPVS::Object *dpvsObject);
	DPVS::Cell           *createDpvsCell(CellProperty *owner);
	void                  destroyDpvsCell(DPVS::Cell *dpvsCell);

	DPVS::Object         *createDpvsPortal(Portal *owner);
	void                  destroyDpvsPortal(DPVS::Object *dpvsPortal);
	void                  closedStateChangedHookFunction(Portal &portal);

	void                  deleteVisibleCellProperty(CellProperty const *cellProperty);

#ifdef _DEBUG
	void                  reportMetrics();
#endif

#if DO_OBJECT_TRACKING
	const int CALL_STACK_SIZE = 16;
	struct CallStack
	{
		uint32 callers[CALL_STACK_SIZE];
	};
	typedef std::map<DPVS::Object*, CallStack> CallStacks;
	CallStacks            ms_callStacks;
#endif

	void worldEnvironmentRender(Object * const object)
	{
		if (object->isActive())
		{
			Appearance * const appearance = object->getAppearance();
			if (appearance)
			{
				appearance->setTransform_w(object->getTransform_o2w());
				appearance->render();
				appearance->setRenderedThisFrame();
			}

			for (int i = 0; i < object->getNumberOfChildObjects(); ++i)
				worldEnvironmentRender(object->getChildObject(i));
		}
	}
}
using namespace RenderWorldNamespace;

// ======================================================================

PortalAppearance::PortalAppearance(Object *objectToDisable)
: Appearance(NULL),
	m_objectToDisable(objectToDisable)
{
}

// ----------------------------------------------------------------------

void PortalAppearance::preRender() const
{
	if (m_objectToDisable)
	{
		RenderWorld::recursivelyDisableDpvsObjectsForThisRender(m_objectToDisable);
	}
}

// ======================================================================

void RenderWorld::install()
{
	DEBUG_FATAL(ms_installed, ("already installed"));
	ms_installed = true;

	// add the hook function so we know when visible cell properties are deleted
	CellProperty::setDeleteVisibleCellProperty(&deleteVisibleCellProperty);

#if DO_OBJECT_TRACKING
	DEBUG_WARNING(true, ("RenderWorld object tracking is enabled"));
#endif

	// install dpvs
	ms_services = new RenderWorldServices;
	DPVS::Library::init(DPVS::Library::ROW_MAJOR, ms_services);
	ms_commander = new RenderWorldCommander;
	const char *ulv = DPVS::Library::getInfoString(DPVS::Library::VERSION);
	const char *ulc = DPVS::Library::getInfoString(DPVS::Library::COPYRIGHT);
	const char *ulb = DPVS::Library::getInfoString(DPVS::Library::BUILD_TIME);
	const char *ulf = DPVS::Library::getInfoString(DPVS::Library::FUNCTIONALITY);
	const char *ulr = DPVS::Library::getInfoString(DPVS::Library::CUSTOMER);
	const char *ulo = DPVS::Library::getInfoString(DPVS::Library::CPU_OPTIMIZATIONS);
	REPORT_LOG (true, ("+=======================================================================\n"));
	REPORT_LOG(true, ("%s built %s\n%s\n%s for %s\n%s\n", ulv, ulb, ulc, ulf, ulr, ulo));
	REPORT_LOG (true, ("+=======================================================================\n"));

	// create the camera
	ms_dpvsCamera = DPVS::Camera::create();
	ms_dpvsCamera->setPixelCenter(0.0f, 0.0f);
	ms_dpvsCamera->setObjectMinimumCoverage(ConfigClientGraphics::getDpvsMinimumObjectWidth(), ConfigClientGraphics::getDpvsMinimumObjectHeight(), ConfigClientGraphics::getDpvsMinimumObjectOpacity());

	// add debugging stuff
#ifdef _DEBUG
	DebugFlags::registerFlag(ms_disableEnvironment,               "ClientGraphics/Dpvs", "disableEnvironment");
	DebugFlags::registerFlag(ms_disableResolveVisibility,         "ClientGraphics/Dpvs", "disableResolveVisibility");
	DebugFlags::registerFlag(ms_disableObjectMinimumCoverage,     "ClientGraphics/Dpvs", "disableObjectMinimumCoverage");
	DebugFlags::registerFlag(ms_forceDisableOcclusionCulling,     "ClientGraphics/Dpvs", "disableOcclusionCulling");
	DebugFlags::registerFlag(ms_disablePortalTraversal,           "ClientGraphics/Dpvs", "disablePortalTraversal");
	DebugFlags::registerFlag(ms_disableViewFrustumCulling,        "ClientGraphics/Dpvs", "disableViewFrustumCulling");
	DebugFlags::registerFlag(ms_lockViewFrustum,                  "ClientGraphics/Dpvs", "lockViewFrustum");
	DebugFlags::registerFlag(ms_renderAxisAlignedBoundingBoxes,   "ClientGraphics/Dpvs", "renderAxisAlignedBoundingBoxes");
	DebugFlags::registerFlag(ms_renderFrustum,                    "ClientGraphics/Dpvs", "renderFrustum");
	DebugFlags::registerFlag(ms_renderObjectAlignedBoundingBoxes, "ClientGraphics/Dpvs", "renderObjectAlignedBoundingBoxes");
	DebugFlags::registerFlag(ms_renderPortals,                    "ClientGraphics/Dpvs", "renderPortals");
	DebugFlags::registerFlag(ms_renderSilhouettes,                "ClientGraphics/Dpvs", "renderSilhouettes");
	DebugFlags::registerFlag(ms_renderVoxels,                     "ClientGraphics/Dpvs", "renderVoxels");
	DebugFlags::registerFlag(ms_reportMetrics,                    "ClientGraphics/Dpvs", "reportMetrics",  &reportMetrics);
	DebugFlags::registerFlag(ms_checkConsistencyBeforeRender,     "ClientGraphics/Dpvs", "checkConsistencyBeforeRender");
#endif

	DPVS::Vector3 const center = { 0.0f, 0.0f, 0.0f };
	ms_defaultModel = DPVS::SphereModel::create(center, 1.0f);
	NOT_NULL(ms_defaultModel);

	Object::setDpvsObjectHookFunctions(&inWorldAddDpvsObject, &inWorldRemoveDpvsObject, &leakedObject);
	CellProperty::setCreateAndDestroyDpvsCellHookFunctions(createDpvsCell, destroyDpvsCell);
	Portal::setHookFunctions(createDpvsPortal, destroyDpvsPortal, closedStateChangedHookFunction, true);

	// the world cell property has already been created by the time this function is called, so we have to fix up its dpvs cell
	CellProperty::ms_worldCellProperty->m_dpvsCell = createDpvsCell(CellProperty::ms_worldCellProperty);

	ms_visibleCellList.reserve(128);

	ExitChain::add(&remove, "RenderWorld::Remove");
}

// ----------------------------------------------------------------------

void RenderWorld::remove()
{
	clearVisibleCells();

	ms_defaultModel->release();
	ms_defaultModel = NULL;

	destroyDpvsCell(CellProperty::ms_worldCellProperty->m_dpvsCell);
	CellProperty::ms_worldCellProperty->m_dpvsCell = NULL;

	IGNORE_RETURN(ms_dpvsCamera->release());
	ms_dpvsCamera = 0;

#ifdef _DEBUG
	const int liveObjects         = static_cast<int>(DPVS::Library::getStatistic(DPVS::Library::STAT_LIVEOBJECTS));
	const int liveCells           = static_cast<int>(DPVS::Library::getStatistic(DPVS::Library::STAT_LIVECELLS));
	const int livePhysicalPortals = static_cast<int>(DPVS::Library::getStatistic(DPVS::Library::STAT_LIVEPHYSICALPORTALS));
	const int liveModels          = static_cast<int>(DPVS::Library::getStatistic(DPVS::Library::STAT_LIVEMODELS));
	const int liveCameras         = static_cast<int>(DPVS::Library::getStatistic(DPVS::Library::STAT_LIVECAMERAS));
	const int liveRois            = static_cast<int>(DPVS::Library::getStatistic(DPVS::Library::STAT_LIVEREGIONSOFINFLUENCE));

	if (liveObjects || liveCells || livePhysicalPortals || liveModels)
	{
		DPVS::Library::suggestGarbageCollect(ms_commander, 0.0f);
		DEBUG_WARNING(true, ("DPVS items still allocated: %d=objects %d=cells %d=portals %d=models %d=cameras %d=rois", liveObjects, liveCells, livePhysicalPortals, liveModels, liveCameras, liveRois));
	}
#endif

	delete ms_commander;
	ms_commander = 0;

	DPVS::Library::exit();
	delete ms_services;
	ms_services = 0;

	ms_installed = false;
}

// ----------------------------------------------------------------------

void RenderWorldNamespace::clearVisibleCells()
{
	RenderWorld::CellPropertyList::iterator const cEnd = ms_visibleCellList.end();
	for (RenderWorld::CellPropertyList::iterator c = ms_visibleCellList.begin(); c != cEnd; ++c)
		(*c)->setVisible(false);

	ms_visibleCellList.clear();
}

// ----------------------------------------------------------------------

static std::string const getObjectDebugInformation(Object const & object)
{
	char buffer[512];
	snprintf(buffer, sizeof(buffer) - 1, "[ptr=%p, id=%s, ot=%s, at=%s]", 
		&object,
		object.getNetworkId().getValueString().c_str(),
		object.getObjectTemplateName() ? object.getObjectTemplateName() : "(null)", 
		(object.getAppearance() && object.getAppearance()->getAppearanceTemplateName()) ? object.getAppearance()->getAppearanceTemplateName() : "(null)");
	buffer[sizeof(buffer) - 1] = '\0';

	return buffer;
}

void RenderWorld::convertToDpvsTransform(const Transform &source, const Vector & scale, DPVS::Matrix4x4 &destination, Object const & debugObject)
{
#ifdef _DEBUG
	DEBUG_FATAL(_isnan(source.matrix[0][0]), ("matrix element 00 is invalid for %s", getObjectDebugInformation(debugObject).c_str()));
	DEBUG_FATAL(_isnan(source.matrix[0][1]), ("matrix element 01 is invalid for %s", getObjectDebugInformation(debugObject).c_str()));
	DEBUG_FATAL(_isnan(source.matrix[0][2]), ("matrix element 02 is invalid for %s", getObjectDebugInformation(debugObject).c_str()));
	DEBUG_FATAL(_isnan(source.matrix[0][3]), ("matrix element 03 is invalid for %s", getObjectDebugInformation(debugObject).c_str()));
	DEBUG_FATAL(_isnan(source.matrix[1][0]), ("matrix element 10 is invalid for %s", getObjectDebugInformation(debugObject).c_str()));
	DEBUG_FATAL(_isnan(source.matrix[1][1]), ("matrix element 11 is invalid for %s", getObjectDebugInformation(debugObject).c_str()));
	DEBUG_FATAL(_isnan(source.matrix[1][2]), ("matrix element 12 is invalid for %s", getObjectDebugInformation(debugObject).c_str()));
	DEBUG_FATAL(_isnan(source.matrix[1][3]), ("matrix element 13 is invalid for %s", getObjectDebugInformation(debugObject).c_str()));
	DEBUG_FATAL(_isnan(source.matrix[2][0]), ("matrix element 20 is invalid for %s", getObjectDebugInformation(debugObject).c_str()));
	DEBUG_FATAL(_isnan(source.matrix[2][1]), ("matrix element 21 is invalid for %s", getObjectDebugInformation(debugObject).c_str()));
	DEBUG_FATAL(_isnan(source.matrix[2][2]), ("matrix element 22 is invalid for %s", getObjectDebugInformation(debugObject).c_str()));
	DEBUG_FATAL(_isnan(source.matrix[2][3]), ("matrix element 23 is invalid for %s", getObjectDebugInformation(debugObject).c_str()));
	DEBUG_FATAL(_isnan(scale.x), ("scale.x is invalid for %s", getObjectDebugInformation(debugObject).c_str()));
	DEBUG_FATAL(_isnan(scale.y), ("scale.y is invalid for %s", getObjectDebugInformation(debugObject).c_str()));
	DEBUG_FATAL(_isnan(scale.z), ("scale.z is invalid for %s", getObjectDebugInformation(debugObject).c_str()));
#else
	FATAL(_isnan(source.matrix[0][0]), ("matrix element 00 is invalid for %s", getObjectDebugInformation(debugObject).c_str()));
	FATAL(_isnan(source.matrix[0][1]), ("matrix element 01 is invalid for %s", getObjectDebugInformation(debugObject).c_str()));
	FATAL(_isnan(source.matrix[0][2]), ("matrix element 02 is invalid for %s", getObjectDebugInformation(debugObject).c_str()));
	FATAL(_isnan(source.matrix[0][3]), ("matrix element 03 is invalid for %s", getObjectDebugInformation(debugObject).c_str()));
	FATAL(_isnan(source.matrix[1][0]), ("matrix element 10 is invalid for %s", getObjectDebugInformation(debugObject).c_str()));
	FATAL(_isnan(source.matrix[1][1]), ("matrix element 11 is invalid for %s", getObjectDebugInformation(debugObject).c_str()));
	FATAL(_isnan(source.matrix[1][2]), ("matrix element 12 is invalid for %s", getObjectDebugInformation(debugObject).c_str()));
	FATAL(_isnan(source.matrix[1][3]), ("matrix element 13 is invalid for %s", getObjectDebugInformation(debugObject).c_str()));
	FATAL(_isnan(source.matrix[2][0]), ("matrix element 20 is invalid for %s", getObjectDebugInformation(debugObject).c_str()));
	FATAL(_isnan(source.matrix[2][1]), ("matrix element 21 is invalid for %s", getObjectDebugInformation(debugObject).c_str()));
	FATAL(_isnan(source.matrix[2][2]), ("matrix element 22 is invalid for %s", getObjectDebugInformation(debugObject).c_str()));
	FATAL(_isnan(source.matrix[2][3]), ("matrix element 23 is invalid for %s", getObjectDebugInformation(debugObject).c_str()));
	FATAL(_isnan(scale.x), ("scale.x is invalid for %s", getObjectDebugInformation(debugObject).c_str()));
	FATAL(_isnan(scale.y), ("scale.y is invalid for %s", getObjectDebugInformation(debugObject).c_str()));
	FATAL(_isnan(scale.z), ("scale.z is invalid for %s", getObjectDebugInformation(debugObject).c_str()));
#endif

	destination.m[0][0] = source.matrix[0][0] * scale.x;
	destination.m[0][1] = source.matrix[0][1] * scale.y;
	destination.m[0][2] = source.matrix[0][2] * scale.z;
	destination.m[0][3] = source.matrix[0][3];
	destination.m[1][0] = source.matrix[1][0] * scale.x;
	destination.m[1][1] = source.matrix[1][1] * scale.y;
	destination.m[1][2] = source.matrix[1][2] * scale.z;
	destination.m[1][3] = source.matrix[1][3];
	destination.m[2][0] = source.matrix[2][0] * scale.x;
	destination.m[2][1] = source.matrix[2][1] * scale.y;
	destination.m[2][2] = source.matrix[2][2] * scale.z;
	destination.m[2][3] = source.matrix[2][3];
	destination.m[3][0] = 0;
	destination.m[3][1] = 0;
	destination.m[3][2] = 0;
	destination.m[3][3] = 1;
}

// ----------------------------------------------------------------------

void RenderWorld::convertToTransform(const DPVS::Matrix4x4 &source, Transform &destination)
{
	destination.matrix[0][0] = source.m[0][0];
	destination.matrix[0][1] = source.m[0][1];
	destination.matrix[0][2] = source.m[0][2];
	destination.matrix[0][3] = source.m[0][3];
	destination.matrix[1][0] = source.m[1][0];
	destination.matrix[1][1] = source.m[1][1];
	destination.matrix[1][2] = source.m[1][2];
	destination.matrix[1][3] = source.m[1][3];
	destination.matrix[2][0] = source.m[2][0];
	destination.matrix[2][1] = source.m[2][1];
	destination.matrix[2][2] = source.m[2][2];
	destination.matrix[2][3] = source.m[2][3];
}

// ----------------------------------------------------------------------

void RenderWorld::addObjectNotifications(Object &object)
{
	if (!object.hasNotification(ms_cellNotification))
		object.addNotification(ms_occlusionNotification);
}

// ----------------------------------------------------------------------

void RenderWorld::addCellNotifications(Object &object)
{
	object.addNotification(ms_cellNotification);
}

// ----------------------------------------------------------------------

DPVS::Model *RenderWorld::fetchDefaultModel()
{
	NOT_NULL(ms_defaultModel);
	ms_defaultModel->addReference();
	return ms_defaultModel;
}

// ----------------------------------------------------------------------

DPVS::SphereModel *RenderWorld::fetchSphereModel(float radius)
{
	if (radius <= 0.0f)
	{
		DEBUG_WARNING(radius < 0.0f, ("fetching dpvs sphere with radius < 0"));
		radius = 0.1f;
	}
	
	const DPVS::Vector3 dpvsCenter = { 0.0f, 0.0f, 0.0f };
	return NON_NULL(DPVS::SphereModel::create(dpvsCenter, radius));
}

// ----------------------------------------------------------------------

DPVS::SphereModel *RenderWorld::fetchSphereModel(const Sphere &sphere)
{
	const Vector &center = sphere.getCenter();
	const DPVS::Vector3 dpvsCenter = { center.x, center.y, center.z };
	float radius = sphere.getRadius();
	if (radius <= 0.0f)
	{
		DEBUG_WARNING(radius < 0.0f, ("fetching dpvs sphere with radius < 0"));
		radius = 0.1f;
	}

	return NON_NULL(DPVS::SphereModel::create(dpvsCenter, radius));
}

// ----------------------------------------------------------------------

DPVS::OBBModel *RenderWorld::fetchBoxModel(const AxialBox &box)
{
	Vector min = box.getMin();
	Vector max = box.getMax();
	
	if (max.x < min.x || max.y < min.y || max.z < min.z)
	{
		DEBUG_WARNING(true, ("fetching dpvs inverted box"));
		min.set(-0.05f,-0.05f,-0.05f);
		max.set(0.05f, 0.05f, 0.05f);
	}
	else
	{
		if (min.x == max.x)
			min.x -= 0.05f;

		if (min.y == max.y)
			min.y -= 0.05f;

		if (min.z == max.z)
			min.z -= 0.05f;
	}

	return NON_NULL(DPVS::OBBModel::create(VECTOR_TO_V3(min), VECTOR_TO_V3(max))); //lint !e826 // suspicious pointer converstion
}

// ----------------------------------------------------------------------

DPVS::MeshModel *RenderWorld::fetchMeshModel(const IndexedTriangleList &indexedTriangleList, bool const clockwiseWindingOrder)
{
	DEBUG_FATAL(indexedTriangleList.getIndices().size() % 3 != 0, ("not an indexed triangle list"));
	DPVS::MeshModel *returnValue = DPVS::MeshModel::create(reinterpret_cast<const DPVS::Vector3*>(&indexedTriangleList.getVertices()[0]), reinterpret_cast<const DPVS::Vector3i*>(&indexedTriangleList.getIndices()[0]), static_cast<int>(indexedTriangleList.getVertices().size()), static_cast<int>(indexedTriangleList.getIndices().size()) / 3, clockwiseWindingOrder);
	return NON_NULL(returnValue);
}

// ----------------------------------------------------------------------

DPVS::Object *RenderWorld::createObject(Appearance *appearance, DPVS::Model *testModel)
{
	NOT_NULL(appearance);
	NOT_NULL(testModel);
	DPVS::Object *dpvsObject = DPVS::Object::create(testModel);
	NOT_NULL(dpvsObject);

#if DO_OBJECT_TRACKING
	CallStack callStack;
	DebugHelp::getCallStack(callStack.callers, CALL_STACK_SIZE);
	ms_callStacks[dpvsObject] = callStack;
#endif

	dpvsObject->setUserPointer(appearance);
	return dpvsObject;
}

// ----------------------------------------------------------------------

DPVS::Object *RenderWorld::createObject(Appearance *appearance, float testModelSphereRadius)
{
	DPVS::SphereModel *testModel = fetchSphereModel(testModelSphereRadius);
	DPVS::Object *result = createObject(appearance, testModel);
	testModel->release();
	return result;
}

// ----------------------------------------------------------------------

DPVS::Object *RenderWorld::createObject(Appearance *appearance, const AxialBox &box)
{
	DPVS::OBBModel *testModel = fetchBoxModel(box);
	DPVS::Object *result = createObject(appearance, testModel);
	testModel->release();
	return result;
}

// ----------------------------------------------------------------------

DPVS::Object *RenderWorld::createObject(Appearance *appearance, DPVS::Model *testModel, DPVS::Model *writeModel)
{
	NOT_NULL(appearance);
	NOT_NULL(testModel);
	DPVS::Object *dpvsObject = createObject(appearance, testModel);
	if (writeModel)
		dpvsObject->setWriteModel(writeModel);
	return dpvsObject;
}

// ----------------------------------------------------------------------

DPVS::Object *RenderWorld::createUnboundedObject(Appearance *appearance)
{
	NOT_NULL(appearance);
	DPVS::Model *testModel = fetchDefaultModel();
	DPVS::Object *dpvsObject = createObject(appearance, testModel);
	dpvsObject->set(DPVS::Object::UNBOUNDED, true);
	testModel->release();
	return dpvsObject;
}

// ----------------------------------------------------------------------

DPVS::RegionOfInfluence *RenderWorld::createRegionOfInfluence(Object *object, DPVS::Model *testModel)
{
	NOT_NULL(object);
	NOT_NULL(testModel);
	DPVS::RegionOfInfluence *dpvsRoi = DPVS::RegionOfInfluence::create(testModel);

#if DO_OBJECT_TRACKING
	CallStack callStack;
	DebugHelp::getCallStack(callStack.callers, CALL_STACK_SIZE);
	ms_callStacks[dpvsRoi] = callStack;
#endif

	NOT_NULL(dpvsRoi);
	dpvsRoi->setUserPointer(object);
	return dpvsRoi;
}

// ----------------------------------------------------------------------

DPVS::RegionOfInfluence *RenderWorld::createRegionOfInfluence(Object *object, float testModelSphereRadius)
{
	DPVS::SphereModel *testModel = fetchSphereModel(testModelSphereRadius);
	DPVS::RegionOfInfluence *result = createRegionOfInfluence(object, testModel);
	testModel->release();
	return result;
}

// ----------------------------------------------------------------------

DPVS::RegionOfInfluence *RenderWorld::createUnboundedRegionOfInfluence(Object *object)
{
	NOT_NULL(object);
	DPVS::Model *testModel = fetchDefaultModel();
	DPVS::RegionOfInfluence *dpvsRoi = createRegionOfInfluence(object, testModel);
	dpvsRoi->set(DPVS::Object::UNBOUNDED, true);
	testModel->release();
	return dpvsRoi;
}

// ----------------------------------------------------------------------

void RenderWorld::addObjectToWorld(DPVS::Object *dpvsObject, Object *owner)
{
	inWorldAddDpvsObject(owner, dpvsObject);
}

// ----------------------------------------------------------------------

void RenderWorld::removeObjectFromWorld(DPVS::Object *dpvsObject)
{
	if (dpvsObject)
	{
		dpvsObject->setCell(NULL);
	}
}

// ----------------------------------------------------------------------

bool RenderWorld::isObjectInWorld(DPVS::Object *dpvsObject)
{
	if (!dpvsObject)
	{
		return false;
	}
	else
	{
		return dpvsObject->getCell()!=NULL;
	}
}

// ----------------------------------------------------------------------

void RenderWorldNamespace::inWorldAddDpvsObject(Object *object, DPVS::Object *dpvsObject)
{
	if (!object || !dpvsObject)
	{
		return;
	}

	const CellProperty *cell = object->getParentCell();
	NOT_NULL(cell);
	if (!cell)
	{
		return;
	}

	DPVS::Cell *dpvsCell = cell->getDpvsCell();
	NOT_NULL(dpvsCell);
	if (!dpvsCell)
	{
		return;
	}

	const Transform o2c = object->getTransform_o2c();
	DPVS::Matrix4x4 dpvsTransform;
	RenderWorld::convertToDpvsTransform(o2c, object->getScale(), dpvsTransform, *object);

	dpvsObject->setCell(dpvsCell);
	dpvsObject->setObjectToCellMatrix(dpvsTransform);
}

// ----------------------------------------------------------------------

void RenderWorldNamespace::inWorldRemoveDpvsObject(DPVS::Object *dpvsObject)
{
	if (!dpvsObject)
	{
		return;
	}
	dpvsObject->setCell(NULL);
}

// ----------------------------------------------------------------------

DPVS::Cell *RenderWorldNamespace::createDpvsCell(CellProperty *owner)
{
	DPVS::Cell *dpvsCell = DPVS::Cell::create();
	dpvsCell->set(DPVS::Cell::REPORT_IMMEDIATELY, true);
	dpvsCell->setUserPointer(owner);
	return dpvsCell;
}

// ----------------------------------------------------------------------

void RenderWorldNamespace::destroyDpvsCell(DPVS::Cell *dpvsCell)
{
	dpvsCell->release();
}

// ----------------------------------------------------------------------

DPVS::Object *RenderWorldNamespace::createDpvsPortal(Portal *portal)
{
	IndexedTriangleList const & portalGeometry = portal->getGeometry();

	// @todo these need to be shared as well
	// create the target model
	DPVS::MeshModel *dpvsPortalModel = RenderWorld::fetchMeshModel(portalGeometry, portal->isGeometryWindingOrderClockwise());
	dpvsPortalModel->set(DPVS::Model::BACKFACE_CULLABLE, true);

	// get the target cell in dpvs
	DPVS::PhysicalPortal * dpvsPortal = 0;
	if (portal->isDisabled())
	{
		// create the portal
		dpvsPortal = DPVS::PhysicalPortal::create(dpvsPortalModel, NULL);
	}
	else
	{
		const CellProperty *targetCell = portal->getNeighbor()->getParentCell();
		NOT_NULL(targetCell);
		DPVS::Cell *dpvsTargetCell = targetCell->getDpvsCell();
		NOT_NULL(dpvsTargetCell);

		// create the portal
		dpvsPortal = DPVS::PhysicalPortal::create(dpvsPortalModel, dpvsTargetCell);
	}

	NOT_NULL(dpvsPortal);

#if DO_OBJECT_TRACKING
	CallStack callStack;
	DebugHelp::getCallStack(callStack.callers, CALL_STACK_SIZE);
	ms_callStacks[dpvsPortal] = callStack;
#endif

	IGNORE_RETURN(dpvsPortalModel->autoRelease());

	// we want to be told when we enter and leave portals
	dpvsPortal->set(DPVS::Object::INFORM_PORTAL_ENTER, true);
	dpvsPortal->set(DPVS::Object::INFORM_PORTAL_EXIT, true);
	dpvsPortal->set(DPVS::Object::INFORM_PORTAL_PRE_EXIT, true);

	// create an appearance for the portal here.  This appearance doesn't render anything, but it
	// present so that we can get a callback when the portal is determined to be visible so we
	// can disable the rendering of the neighboring portal's door (to avoid z-fighting among
	// other issues).  This needs to be an appearance because the RenderWorldCommander expects
	// all DPVS object user pointers to be appearance pointers.
	Object *neighboringDoor = portal->getNeighbor()->getDoorObject();
	if (neighboringDoor)
	{
		PortalAppearance *portalAppearance = new PortalAppearance(neighboringDoor);
		dpvsPortal->set(DPVS::Object::REPORT_IMMEDIATELY, true);
		portal->setAppearance(portalAppearance);
		dpvsPortal->setUserPointer(portalAppearance);
	}

	return dpvsPortal;
}

// ----------------------------------------------------------------------

void RenderWorldNamespace::destroyDpvsPortal(DPVS::Object *dpvsPortal)
{
	dpvsPortal->release();
}

// ----------------------------------------------------------------------

void RenderWorldNamespace::closedStateChangedHookFunction(Portal &portal)
{
	DPVS::Object *dpvsObject = portal.getDpvsObject();

	if (dpvsObject != 0)
	{
		dpvsObject->set(DPVS::Object::ENABLED, !portal.isClosed());
	}
}

// ----------------------------------------------------------------------

void RenderWorld::addWorldEnvironmentObject(Object *object)
{
	ms_worldEnvironmentObjects.addObject(object);
}

// ----------------------------------------------------------------------

void RenderWorld::addMoveWithCameraWorldEnvironmentObject(Object * const object)
{
	ms_moveWithCameraWorldEnvironmentObjects.addObject(object);
}

// ----------------------------------------------------------------------

void RenderWorld::removeWorldEnvironmentObject(Object *object)
{
	ms_worldEnvironmentObjects.removeObject(object);
	ms_moveWithCameraWorldEnvironmentObjects.removeObject(object);
}

// ----------------------------------------------------------------------

void RenderWorld::addWorldEnvironmentLight(Light *light)
{
	ms_worldEnvironmentLights.addObject(light);
}

// ----------------------------------------------------------------------

void RenderWorld::removeWorldEnvironmentLight(Light *light)
{
	ms_worldEnvironmentLights.removeObject(light);
}

// ----------------------------------------------------------------------

void RenderWorld::setClearDepthAndStencilBufferAfterRenderingEnvironment(bool const clearDepthAndStencilBufferAfterRenderingEnvironment)
{
	ms_clearDepthAndStencilBufferAfterRenderingEnvironment = clearDepthAndStencilBufferAfterRenderingEnvironment;
}

// ----------------------------------------------------------------------

const Vector RenderWorld::getCameraPosition()
{
	return ms_cameraToWorld.getPosition_p();
}

// ----------------------------------------------------------------------

void RenderWorld::recursivelyDisableDpvsObjectsForThisRender(const Object *object)
{
	NOT_NULL(object);

	{
		Object::DpvsObjects const *dpvsObjects = object->getDpvsObjects();
		if (dpvsObjects)
		{
			Object::DpvsObjects::const_iterator iEnd = dpvsObjects->end();
			for (Object::DpvsObjects::const_iterator i = dpvsObjects->begin(); i != iEnd; ++i)
				RenderWorld::disableDpvsObjectForThisRender(*i);
		}
	}

	{
		const int numberOfChildObjects = object->getNumberOfChildObjects();
		for (int i = 0; i < numberOfChildObjects; ++i)
			recursivelyDisableDpvsObjectsForThisRender(object->getChildObject(i));
	}
}

// ----------------------------------------------------------------------

void RenderWorld::disableDpvsObjectForThisRender(DPVS::Object *dpvsObject)
{
	if (dpvsObject->test(DPVS::Object::ENABLED))
	{
		dpvsObject->set(DPVS::Object::ENABLED, false);
		ms_excludedDpvsObjects.push_back(dpvsObject);
	}
}

// ----------------------------------------------------------------------
#ifdef _DEBUG
#define ENABLE_DPVS_LINE(myFlag, dpvsFlag) \
if (myFlag) \
	DPVS::Library::setFlags(DPVS::Library::LINEDRAW, DPVS::Library::dpvsFlag); \
else \
	DPVS::Library::clearFlags(DPVS::Library::LINEDRAW, DPVS::Library::dpvsFlag);
#endif

void RenderWorld::drawScene(const RenderWorldCamera &camera)
{
	DEBUG_FATAL(!ms_installed, ("RenderWorld not installed"));

	NP_PROFILER_AUTO_BLOCK_DEFINE("RenderWorld::drawScene setup");

	int portalRecusionDepth = 0;
	
	ms_camera = &camera;
	{
		///////////////////////////////////////////////////////////////////////////////////////
		// set camera
		NP_PROFILER_AUTO_BLOCK_DEFINE("setup");

#ifdef _DEBUG
		if (!ms_lockViewFrustum)
#endif
		{
			ms_cameraCell = camera.getParentCell();
			ms_dpvsCameraCell = ms_cameraCell->getDpvsCell();

			ms_cameraToWorld = camera.getTransform_o2w();
			ms_cameraViewportWidth = camera.getViewportWidth();
			ms_cameraViewportHeight = camera.getViewportHeight();
			for (int i = 0; i < 8; ++i)
				ms_cameraFrustum[i] = camera.getFrustumVertices()[i];
		}
		RenderWorldCommander::setCamera(&camera, ms_cameraCell, ms_cameraToWorld);

		///////////////////////////////////////////////////////////////////////////////////////
		// set the camera cell and transform
		ms_dpvsCamera->setCell(ms_dpvsCameraCell);
		const Transform cameraToCell = camera.getTransform_o2c();
		DPVS::Matrix4x4 dpvsTransform;
		convertToDpvsTransform(cameraToCell, Vector::xyz111, dpvsTransform, camera);
		ms_dpvsCamera->setCameraToCellMatrix(dpvsTransform);

		///////////////////////////////////////////////////////////////////////////////////////
		// setup the rest of the camera
		DPVS::Frustum dpvsFrustum;
		dpvsFrustum.type   = DPVS::Frustum::PERSPECTIVE;
		dpvsFrustum.left   = ms_cameraFrustum[Camera::FV_NearUpperLeft].x;
		dpvsFrustum.top    = ms_cameraFrustum[Camera::FV_NearUpperLeft].y;
		dpvsFrustum.right  = ms_cameraFrustum[Camera::FV_NearLowerRight].x;
		dpvsFrustum.bottom = ms_cameraFrustum[Camera::FV_NearLowerRight].y;
		dpvsFrustum.zNear  = ms_cameraFrustum[Camera::FV_NearUpperLeft].z;
		dpvsFrustum.zFar   = ms_cameraFrustum[Camera::FV_FarUpperLeft].z;
		ms_dpvsCamera->setFrustum(dpvsFrustum);
	#ifdef _DEBUG
		uint const cullingParameters = ((ms_forceDisableOcclusionCulling || ms_disableOcclusionCulling) ? 0 : DPVS::Camera::OCCLUSION_CULLING) | (ms_disableViewFrustumCulling ? 0 : DPVS::Camera::VIEWFRUSTUM_CULLING);
		portalRecusionDepth = ms_disablePortalTraversal ? 0 : 8;
	#else
		uint const cullingParameters = (ms_disableOcclusionCulling ? 0 : DPVS::Camera::OCCLUSION_CULLING) | DPVS::Camera::VIEWFRUSTUM_CULLING;
		portalRecusionDepth = 8;
	#endif

		///////////////////////////////////////////////////////////////////////////////////////
		// Update DPVS camera parameters if needed
		if (  ms_cameraViewportWidth != ms_lastCameraViewportWidth 
			|| ms_cameraViewportHeight!= ms_lastCameraViewportHeight 
			|| cullingParameters != ms_lastCullingParameters
			)
		{	
			ms_lastCameraViewportWidth  = ms_cameraViewportWidth;
			ms_lastCameraViewportHeight = ms_cameraViewportHeight;
			ms_lastCullingParameters    = cullingParameters;
			float const imageScale = ConfigClientGraphics::getDpvsImageScale();
			ms_dpvsCamera->setParameters(ms_cameraViewportWidth, ms_cameraViewportHeight, cullingParameters, imageScale, imageScale);
		}

		///////////////////////////////////////////////////////////////////////////////////////
	#ifdef _DEBUG
		ENABLE_DPVS_LINE(ms_renderAxisAlignedBoundingBoxes,   LINE_RECTANGLES);
		ENABLE_DPVS_LINE(ms_renderObjectAlignedBoundingBoxes, LINE_OBJECT_BOUNDS);
		ENABLE_DPVS_LINE(ms_renderPortals,                    LINE_PORTAL_RECTANGLES);
		ENABLE_DPVS_LINE(ms_renderSilhouettes,                LINE_SILHOUETTES);
		ENABLE_DPVS_LINE(ms_renderTestSilhouettes,            LINE_TEST_SILHOUETTES);
		ENABLE_DPVS_LINE(ms_renderVoxels,                     LINE_VOXELS);
	#undef ENABLE_DPVS_LINE
	#endif
		///////////////////////////////////////////////////////////////////////////////////////

		///////////////////////////////////////////////////////////////////////////////////////
		// determine the visible objects
		if (ms_disableObjectMinimumCoverage)
		{
			ms_dpvsCamera->setObjectMinimumCoverage(0.f, 0.f, 1.f);
		}
		else
		{
			ms_dpvsCamera->setObjectMinimumCoverage(ConfigClientGraphics::getDpvsMinimumObjectWidth(), ConfigClientGraphics::getDpvsMinimumObjectHeight(), ConfigClientGraphics::getDpvsMinimumObjectOpacity());
		}
		///////////////////////////////////////////////////////////////////////////////////////

		///////////////////////////////////////////////////////////////////////////////////////
		// render environment
	#ifdef _DEBUG
		if (!ms_disableEnvironment)
	#endif
		{
			Vector const & cameraPosition_w = ShaderPrimitiveSorter::getCurrentCameraPosition();

			//-- Move all moveWithCamera objects to their render locations
			{
				int const numberOfObjects = ms_moveWithCameraWorldEnvironmentObjects.getNumberOfObjects();
				for (int i = 0; i < numberOfObjects; ++i)
				{
					Object * object = ms_moveWithCameraWorldEnvironmentObjects[i];
					object->setPosition_p(object->getPosition_p() + cameraPosition_w);
				}
			}

			ShaderPrimitiveSorter::pushCell(*CellProperty::getWorldCellProperty());

			{
				const int numberOfLights = ms_worldEnvironmentLights.getNumberOfObjects();
				for (int i = 0; i < numberOfLights; ++i)
				{
					ms_worldEnvironmentLights[i]->setRegionOfInfluenceEnabled(true);
				}
			}

			bool const useClip = ShaderPrimitiveSorter::getUseClipRectangle();
			ShaderPrimitiveSorter::setUseClipRectangle(false);
			ShaderPrimitiveSorter::setUseWaterTests(false);

			{
				const int numberOfObjects = ms_worldEnvironmentObjects.getNumberOfObjects();
				for (int i = 0; i < numberOfObjects; ++i)
				{
					Object *object = ms_worldEnvironmentObjects[i];
					worldEnvironmentRender(object);
				}
			}

			{
				int const numberOfObjects = ms_moveWithCameraWorldEnvironmentObjects.getNumberOfObjects();
				for (int i = 0; i < numberOfObjects; ++i)
				{
					Object *object = ms_moveWithCameraWorldEnvironmentObjects[i];
					worldEnvironmentRender(object);
				}
			}

			ShaderPrimitiveSorter::setUseClipRectangle(useClip);
			ShaderPrimitiveSorter::setUseWaterTests(true);

			{
				const int numberOfLights = ms_worldEnvironmentLights.getNumberOfObjects();
				for (int i = 0; i < numberOfLights; ++i)
				{
					ms_worldEnvironmentLights[i]->setRegionOfInfluenceEnabled(false);
				}
			}

			ShaderPrimitiveSorter::popCell();

			//-- Move all moveWithCamera objects back to their original locations
			{
				int const numberOfObjects = ms_moveWithCameraWorldEnvironmentObjects.getNumberOfObjects();
				for (int i = 0; i < numberOfObjects; ++i)
				{
					Object * object = ms_moveWithCameraWorldEnvironmentObjects[i];
					object->setPosition_p(object->getPosition_p() - cameraPosition_w);
				}
			}
		}
		///////////////////////////////////////////////////////////////////////////////////////

		if (ms_clearDepthAndStencilBufferAfterRenderingEnvironment)
		{
			Graphics::clearViewport(false, 0, true, 1.f, true, 0);
		}

	#ifdef _DEBUG
		if (ms_checkConsistencyBeforeRender)
		{
			DPVS::Library::checkConsistency();
		}
	#endif
	}

#ifdef _DEBUG
	if (!ms_disableResolveVisibility)
#endif
	{
		clearVisibleCells();
		NP_PROFILER_AUTO_BLOCK_DEFINE("resolveVisibility");

		// the end of this profiler block is in RenderWorldCommander::command() case QUERY_BEGIN
		NP_PROFILER_BLOCK_ENTER(ms_dpvsQueryProfilerBlock);

		ms_dpvsCamera->resolveVisibility(ms_commander, portalRecusionDepth, 0.0f);
	}

	{
		NP_PROFILER_AUTO_BLOCK_DEFINE("clean up");

	#ifdef _DEBUG
		if (ms_renderFrustum)
		{
			ms_camera->addDebugPrimitive(new FrustumDebugPrimitive(UtilityDebugPrimitive::S_none, ms_cameraToWorld, ms_cameraFrustum, PackedArgb::solidYellow));
		}
	#endif

		///////////////////////////////////////////////////////////////////////////////////////
		DpvsObjects::const_iterator const iEnd = ms_excludedDpvsObjects.end();
		for (DpvsObjects::const_iterator i = ms_excludedDpvsObjects.begin(); i != iEnd; ++i)
		{
			DPVS::Object *dpvsObject = *i;
			dpvsObject->set(DPVS::Object::ENABLED, true);
		}
		ms_excludedDpvsObjects.clear();
		///////////////////////////////////////////////////////////////////////////////////////

		ms_camera = 0;
	}
}

// ----------------------------------------------------------------------

void RenderWorld::leakedObject(DPVS::Object *object)
{
	object->set(DPVS::Object::ENABLED, false);
	DPVS::Vector3 center;
	float radius;
	object->getSphere(center, radius);
	DEBUG_REPORT_LOG_PRINT(true, ("leaked DPVS object %08x [%f %f %f] %f\n", reinterpret_cast<int>(object),
		center.v[0], center.v[1], center.v[2], radius));

#ifdef _DEBUG
	Appearance *const appearance = static_cast<Appearance*>(object->getUserPointer());
	DEBUG_REPORT_LOG((appearance != NULL) && dynamic_cast<Appearance*>(appearance), ("|^= looks like this came from appearance template [%s], object template [%s].\n", appearance->getAppearanceTemplateName(), appearance->getOwner() ? appearance->getOwner()->getObjectTemplateName() : "<null owner object>"));
#endif

#if DO_OBJECT_TRACKING

	char   libName[256];
	char   fileName[256];
	int    line;

	CallStacks::iterator callStackIterator = ms_callStacks.find(object);
	if (callStackIterator != ms_callStacks.end())
	{
		CallStack & callStack = callStackIterator->second;

		for (int i = 0; i < CALL_STACK_SIZE; ++i)
			if (callStack.callers[i])
			{
				if (DebugHelp::lookupAddress(callStack.callers[i], libName, fileName, sizeof(fileName), line))
					DEBUG_REPORT_LOG_PRINT(true, ("  %s(%d) : caller %d\n", fileName, line, i));
				else
					DEBUG_REPORT_LOG_PRINT(true, ("  0x%08X : caller %d\n", static_cast<int>(callStack.callers[i]), i));
			}
	}
	else
		DEBUG_REPORT_LOG_PRINT(true, ("  could not find any tracking information\n"));
#endif
}

// ----------------------------------------------------------------------

void RenderWorld::cellVisible(const CellProperty * cellProperty)
{
	if (!cellProperty->wasVisible())
	{
		ms_visibleCellList.push_back(cellProperty);
		cellProperty->setVisible(true);
	}
}

// ----------------------------------------------------------------------

void RenderWorldNamespace::deleteVisibleCellProperty(CellProperty const *cellProperty)
{
	RenderWorld::CellPropertyList::iterator i = std::find(ms_visibleCellList.begin(), ms_visibleCellList.end(), cellProperty);
	DEBUG_FATAL(i == ms_visibleCellList.end(), ("visible cell not found in list"));
	cellProperty->setVisible(false);
	ms_visibleCellList.erase(i);
}

// ----------------------------------------------------------------------

const RenderWorld::CellPropertyList & RenderWorld::getVisibleCells()
{
	return ms_visibleCellList;
}

// ----------------------------------------------------------------------

ProfilerBlock &RenderWorld::getDpvsQueryProfilerBlock()
{
	return ms_dpvsQueryProfilerBlock;
}

// ----------------------------------------------------------------------

void RenderWorld::setDisableOcclusionCulling(bool const disableOcclusionCulling)
{
	ms_disableOcclusionCulling = disableOcclusionCulling;
}

// ----------------------------------------------------------------------

bool RenderWorld::wasObjectRenderedThisFrame(NetworkId const & id)
{
	return RenderWorldCommander::wasObjectRenderedThisFrame(id);
}

// ----------------------------------------------------------------------

#ifdef _DEBUG

void RenderWorldNamespace::reportMetrics()
{
	DEBUG_REPORT_PRINT(true, ("%4d=o %4d=v %4d=p %4d=lc %4d=lp\n", 
		static_cast<int>(DPVS::Library::getStatistic(DPVS::Library::STAT_LIVEOBJECTS)),
		RenderWorldCommander::getNumberOfVisibleObjects(),
		RenderWorldCommander::getNumberOfPortalsCrossed(),
		static_cast<int>(DPVS::Library::getStatistic(DPVS::Library::STAT_LIVECELLS)),
		static_cast<int>(DPVS::Library::getStatistic(DPVS::Library::STAT_LIVEPHYSICALPORTALS))));

	DEBUG_REPORT_PRINT(true, ("%4f=m\n", 
		DPVS::Library::getStatistic(DPVS::Library::STAT_WRITEQUEUEWRITESPERFORMED)));

	DPVS::Library::resetStatistics();
}

#endif

// ----------------------------------------------------------------------

int RenderWorld::getViewportX0()
{
	return ms_camera->getViewportX0();
}

// ----------------------------------------------------------------------

int RenderWorld::getViewportY0()
{
	return ms_camera->getViewportY0();
}

// ======================================================================
