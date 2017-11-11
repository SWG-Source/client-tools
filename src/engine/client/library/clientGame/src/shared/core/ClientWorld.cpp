// ======================================================================
//
// ClientWorld.cpp
// copyright 2000, verant interactive
// copyright 2003, sony online entertainment
//
// ======================================================================

#include "clientGame/FirstClientGame.h"
#include "clientGame/ClientWorld.h"

#include "clientGame/ClientCommandQueue.h"
#include "clientGame/ClientObject.h"
#include "clientGame/ClientRegionEffectManager.h"
#include "clientGame/ConfigClientGame.h"
#include "clientGame/CreatureObject.h"
#include "clientGame/Game.h"
#include "clientGame/GameNetwork.h"
#include "clientGraphics/DebugPrimitive.h"
#include "clientGraphics/Light.h"
#include "clientGraphics/ShaderPrimitiveSorter.h"
#include "clientObject/GameCamera.h"
#include "clientSkeletalAnimation/SkeletalAppearance2.h"
#include "clientTerrain/ClientTerrainSorter.h"
#include "clientTerrain/GroundEnvironment.h"
#include "clientTerrain/SpaceEnvironment.h"
#include "sharedCollision/CollideParameters.h"
#include "sharedCollision/CollisionProperty.h"
#include "sharedCollision/CollisionInfo.h"
#include "sharedCollision/Extent.h"
#include "sharedDebug/DebugFlags.h"
#include "sharedDebug/InstallTimer.h"
#include "sharedDebug/Profiler.h"
#include "sharedFoundation/NetworkId.h"
#include "sharedFoundation/PointerDeleter.h"
#include "sharedFoundation/Production.h"
#include "sharedGame/SharedCreatureObjectTemplate.h"
#include "sharedGame/SharedObjectTemplate.h"
#include "sharedMath/SphereTree.h"
#include "sharedObject/AlterScheduler.h"
#include "sharedObject/Appearance.h"
#include "sharedObject/AppearanceTemplate.h"
#include "sharedObject/CellProperty.h"
#include "sharedObject/Object.h"
#include "sharedObject/ObjectList.h"
#include "sharedObject/PortalProperty.h"
#include "sharedTerrain/TerrainObject.h"

#include "UnicodeUtils.h"

#include <algorithm>
#include <string>

#ifdef _DEBUG
#define CLIENTWORLD_VERBOSE_ERROR_CHECKING 1
#endif

// ======================================================================

namespace ClientWorldNamespace
{
	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

	BaseExtent const * getCorrectExtent(Object const * const object)
	{
		if (object != 0)
		{
			Appearance const * const appearance = object->getAppearance();
			BaseExtent const * const appearanceExtent = (appearance != 0) ? appearance->getExtent() : 0;
			CollisionProperty const * const collisionProperty = object->getCollisionProperty();
			BaseExtent const * const collisionExtent = (collisionProperty != 0) ? collisionProperty->getExtent_l() : 0;

			float const appearanceRadius = (appearanceExtent != 0) ? appearanceExtent->getRadius() : 0.0f;
			float const collisionRadius = (collisionExtent != 0) ? collisionExtent->getRadius() : 0.0f;

			return (appearanceRadius > collisionRadius) ? appearanceExtent : collisionExtent;
		}
		return 0;
	}

	class ObjectSphereExtentAccessor: public BaseSphereTreeAccessor<Object *, ObjectSphereExtentAccessor>
	{
	public:

		static Sphere const getExtent(Object * const object)
		{
			BaseExtent const * const extent = getCorrectExtent(object);

			if (extent != 0)
			{
				Sphere sphere(extent->getBoundingSphere());
				sphere.setCenter(object->rotateTranslate_o2w(sphere.getCenter()));
				return sphere;
			}

			if (object != 0)
			{
				return Sphere(object->getPosition_w(), 1.0f);
			}

			return Sphere::unit;
		}

		static char const *getDebugName (Object* const object)
		{
			return object->getDebugName ();
		}

	};

	//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

	class NoSkeletalAppearanceFilter : public SpatialSubdivisionFilter<Object *>
	{
	public:

		virtual bool operator()(Object * const & object) const;
	};

	//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

	class CollisionObjectSorter
	{
	public:

		explicit CollisionObjectSorter(Vector const & position_w);
		~CollisionObjectSorter();
		CollisionObjectSorter(CollisionObjectSorter const &);
		CollisionObjectSorter & operator=(CollisionObjectSorter const &);

		bool operator()(Object const * object1, Object const * object2) const;

	private:

		CollisionObjectSorter();

	private:

		Vector m_position_w;
	};

	//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

	class CollisionInfoSorter
	{
	public:

		CollisionInfoSorter();
		~CollisionInfoSorter();
		bool operator()(CollisionInfo const & left, CollisionInfo const & right) const;
	};

	//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

	bool          ms_installed;
	bool          ms_debugReport;

	//-- collision detection function
	ClientWorld::CollisionDetectionFunction ms_collisionDetectionFunction;
	Object const * ms_overrideExcludeObject;

	//-- camera lists
	ObjectList*   ms_cameraList;

	//-- light
	ObjectList*   ms_lightList;
	bool          ms_lightListEnabled;

	const Camera* ms_useCamera;

#if PRODUCTION == 0
	bool          ms_disableCollision;
	bool          ms_disableCollisionSorting;
	int ms_collideCalls;
	int ms_collideObjectCalls;
	int ms_appearanceCollideCalls;
	int ms_highQualityCollides;
	int ms_mediumQualityCollides;
	int ms_lowQualityCollides;
#endif
	bool          ms_concludePlayerOnly;
	bool          ms_renderTangibleSphereTree;
	bool          ms_renderTangibleNotTargetableSphereTree;
	bool          ms_renderTangibleFloraSphereTree;
	bool          ms_logWorld;

	SphereTree<Object*, ObjectSphereExtentAccessor> ms_tangibleSphereTree;
	SphereTree<Object*, ObjectSphereExtentAccessor> ms_tangibleNotTargetableSphereTree;
	SphereTree<Object*, ObjectSphereExtentAccessor> ms_tangibleFloraSphereTree;

	typedef std::vector<std::pair<Object *, Sphere> > ObjectSphereList;
	ObjectSphereList ms_objectSphereList;

	//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

	typedef stdmap<Unicode::String, ClientObject *>::fwd NameObjectMap;

	void testObjectByAbbrev (const Vector & pos_w, const Unicode::String & abbrev, NameObjectMap & nameObjectMap, Object &obj)
	{
		ClientObject * const co = obj.asClientObject();
		if (co)
		{
			const Unicode::String & localName = co->getLocalizedName ();
			if (Unicode::caseInsensitiveCompare (localName, abbrev, 0, abbrev.size ()))
			{
				const std::pair<NameObjectMap::iterator, bool> retval = nameObjectMap.insert (std::make_pair (localName, co));

				//-- something by this name is already in the map, choose the one that is closer
				if (!retval.second)
				{
					const NameObjectMap::iterator & it = retval.first;
					ClientObject * const other = (*it).second;

					if (other->getPosition_w ().magnitudeBetweenSquared (pos_w) > obj.getPosition_w ().magnitudeBetweenSquared (pos_w))
						(*it).second = co;
				}
				return;
			}
		}

		for (int i = 0; i < obj.getNumberOfChildObjects(); ++i)
		{
			Object * const childObject = obj.getChildObject(i);
			if (childObject && childObject->isChildObject())
				testObjectByAbbrev(pos_w, abbrev, nameObjectMap, *childObject);
		}
	}

	//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

	void getObjectsInCell (const int objectListIndex, const CellProperty* const cellProperty, ClientWorld::ObjectVector& objectVector);
	void debugRender (Camera const & camera);

	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

	void clearSphereTree (SphereTree<Object*, ObjectSphereExtentAccessor>& sphereTree)
	{
		typedef std::vector<Object *> ObjectVector;
		ObjectVector objectVector;
		sphereTree.dumpSphereTreeObjects(objectVector);

		ObjectVector::const_iterator end = objectVector.end ();
		for (ObjectVector::const_iterator iter = objectVector.begin (); iter != end; ++iter)
		{
			Object * const object = *iter;
			if (object)
				sphereTree.removeObject(object->getSpatialSubdivisionHandle());
		}
	}

	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

	typedef std::vector<ClientWorld::ObjectVector *> ObjectVectorList;
	ObjectVectorList ms_objectVectorList;

	ClientWorld::ObjectVector & allocateObjectVector()
	{
		if (ms_objectVectorList.empty())
			ms_objectVectorList.push_back(new ClientWorld::ObjectVector);

		ClientWorld::ObjectVector * const result = ms_objectVectorList.back();
		ms_objectVectorList.pop_back();

		return *result;
	}

	void freeObjectVector(ClientWorld::ObjectVector & objectVector)
	{
		objectVector.clear();
		ms_objectVectorList.push_back(&objectVector);
	}

	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

	bool internalCollideObjectTest(uint16 const flags, Object const * const object, Vector const & startPosition_w, Vector const & endPosition_w, CollideParameters const & collideParameters, CollisionInfo & result);

}

using namespace ClientWorldNamespace;

// ======================================================================
// ClientWorldNamespace::NoSkeletalAppearanceFilter
// ======================================================================

bool NoSkeletalAppearanceFilter::operator()(Object * const & object) const
{
	if (object->getAppearance() && object->getAppearance()->asSkeletalAppearance2())
		return false;

	return true;
}

// ======================================================================
// ClientWorldNamespace::CollisionObjectSorter
// ======================================================================

ClientWorldNamespace::CollisionObjectSorter::CollisionObjectSorter(Vector const & position_w) :
	m_position_w(position_w)
{
}

// ----------------------------------------------------------------------

ClientWorldNamespace::CollisionObjectSorter::~CollisionObjectSorter()
{
}

// ----------------------------------------------------------------------

ClientWorldNamespace::CollisionObjectSorter::CollisionObjectSorter(CollisionObjectSorter const & rhs) :
	m_position_w(rhs.m_position_w)
{
}

// ----------------------------------------------------------------------

ClientWorldNamespace::CollisionObjectSorter & CollisionObjectSorter::operator=(CollisionObjectSorter const & rhs)
{
	if (this != &rhs)
		m_position_w = rhs.m_position_w;

	return *this;
}

// ----------------------------------------------------------------------

bool ClientWorldNamespace::CollisionObjectSorter::operator()(Object const * const object1, Object const * const object2) const
{
	float const distanceSquaredToObject1 = object1 ? m_position_w.magnitudeBetweenSquared(object1->getPosition_w()) : 0.f;
	float const distanceSquaredToObject2 = object2 ? m_position_w.magnitudeBetweenSquared(object2->getPosition_w()) : 0.f;

	return distanceSquaredToObject1 < distanceSquaredToObject2;
}

// ======================================================================
// ClientWorldNamespace::CollisionInfoSorter
// ======================================================================

ClientWorldNamespace::CollisionInfoSorter::CollisionInfoSorter()
{
}

// ----------------------------------------------------------------------

ClientWorldNamespace::CollisionInfoSorter::~CollisionInfoSorter()
{
}

// ----------------------------------------------------------------------

bool ClientWorldNamespace::CollisionInfoSorter::operator()(CollisionInfo const & left, CollisionInfo const & right) const
{
	return left.getTime() < right.getTime();
}

// ======================================================================
// ClientWorld::TangibleNotification
// ======================================================================

ClientWorld::TangibleNotification::TangibleNotification()
: ObjectNotification()
{
}

// ----------------------------------------------------------------------

ClientWorld::TangibleNotification::~TangibleNotification()
{
}

// ----------------------------------------------------------------------

void ClientWorld::TangibleNotification::addToWorld(Object &object) const
{
#if CLIENTWORLD_VERBOSE_ERROR_CHECKING
	int const existingListIndex = ClientWorld::findObjectList(&object);
	FATAL(existingListIndex != WOL_MarkerAllObjectListsEnd, ("ClientWorld::TangibleNotification::addToWorld: object %s is already in ClientWorld (list %i)", object.getDebugInformation(true).c_str(), existingListIndex));
#endif

	if ((!object.isChildObject()) && (object.getSpatialSubdivisionHandle() == 0))
	{
		DEBUG_REPORT_LOG (ms_logWorld, ("ClientWorld::addObject WOL_Tangible pointer=%p networkId=%s objectTemplateName=%s appearanceTemplateName=%s\n", &object, object.getNetworkId ().getValueString ().c_str (), object.getObjectTemplateName () ? object.getObjectTemplateName () : "null", object.getAppearance () && object.getAppearance ()->getAppearanceTemplate () && object.getAppearance ()->getAppearanceTemplate ()->getName () ? object.getAppearance ()->getAppearanceTemplate ()->getName () : "null"));
		World::addObject(&object, WOL_Tangible);

		object.setSpatialSubdivisionHandle (ms_tangibleSphereTree.addObject (&object));
	}
}

// ----------------------------------------------------------------------

void ClientWorld::TangibleNotification::removeFromWorld(Object &object) const
{
	if (!object.isChildObject())
	{
		DEBUG_REPORT_LOG (ms_logWorld, ("ClientWorld::removeObject WOL_Tangible pointer=%p\n", &object));
		World::removeObject(&object, WOL_Tangible);

		ms_tangibleSphereTree.removeObject (object.getSpatialSubdivisionHandle ());
		object.setSpatialSubdivisionHandle (0);
	}
}

// ----------------------------------------------------------------------

bool ClientWorld::TangibleNotification::positionChanged (Object& object, const bool /*dueToParentChange*/, const Vector& /*oldPosition*/) const
{
	if ((!object.isChildObject()) && (object.getSpatialSubdivisionHandle() != 0))
	{
		ms_tangibleSphereTree.removeObject (object.getSpatialSubdivisionHandle ());
		object.setSpatialSubdivisionHandle (ms_tangibleSphereTree.addObject (&object));
	}

	return true;
}

// ----------------------------------------------------------------------

bool ClientWorld::TangibleNotification::positionAndRotationChanged (Object& object, const bool dueToParentChange, const Vector& oldPosition) const
{
	return positionChanged (object, dueToParentChange, oldPosition);
}

// ----------------------------------------------------------------------

void ClientWorld::TangibleNotification::extentChanged (Object& object) const
{
	if ((!object.isChildObject()) && (object.getSpatialSubdivisionHandle() != 0))
	{
		ms_tangibleSphereTree.removeObject (object.getSpatialSubdivisionHandle ());
		object.setSpatialSubdivisionHandle (ms_tangibleSphereTree.addObject (&object));
	}
}

// ======================================================================
// ClientWorld::TangibleNotTargetableNotification
// ======================================================================

ClientWorld::TangibleNotTargetableNotification::TangibleNotTargetableNotification()
: ObjectNotification()
{
}

// ----------------------------------------------------------------------

ClientWorld::TangibleNotTargetableNotification::~TangibleNotTargetableNotification()
{
}

// ----------------------------------------------------------------------

void ClientWorld::TangibleNotTargetableNotification::addToWorld(Object &object) const
{
#if CLIENTWORLD_VERBOSE_ERROR_CHECKING
	int const existingListIndex = ClientWorld::findObjectList(&object);
	FATAL(existingListIndex != WOL_MarkerAllObjectListsEnd, ("ClientWorld::TangibleNotTargetableNotification::addToWorld: object %s is already in ClientWorld (list %i)", object.getDebugInformation(true).c_str(), existingListIndex));
#endif

	if ((!object.isChildObject()) && (object.getSpatialSubdivisionHandle() == 0))
	{
		DEBUG_REPORT_LOG (ms_logWorld, ("ClientWorld::addObject WOL_TangibleNotTargetable pointer=%p networkId=%s objectTemplateName=%s appearanceTemplateName=%s\n", &object, object.getNetworkId ().getValueString ().c_str (), object.getObjectTemplateName () ? object.getObjectTemplateName () : "null", object.getAppearance () && object.getAppearance ()->getAppearanceTemplate () && object.getAppearance ()->getAppearanceTemplate ()->getName () ? object.getAppearance ()->getAppearanceTemplate ()->getName () : "null"));
		World::addObject(&object, WOL_TangibleNotTargetable);

		object.setSpatialSubdivisionHandle (ms_tangibleNotTargetableSphereTree.addObject (&object));
	}
}

// ----------------------------------------------------------------------

void ClientWorld::TangibleNotTargetableNotification::removeFromWorld(Object &object) const
{
	if (!object.isChildObject())
	{
		DEBUG_REPORT_LOG (ms_logWorld, ("ClientWorld::removeObject WOL_TangibleNotTargetable pointer=%p\n", &object));
		World::removeObject(&object, WOL_TangibleNotTargetable);

		ms_tangibleNotTargetableSphereTree.removeObject (object.getSpatialSubdivisionHandle ());
		object.setSpatialSubdivisionHandle (0);
	}
}

// ----------------------------------------------------------------------

bool ClientWorld::TangibleNotTargetableNotification::positionChanged (Object& object, const bool /*dueToParentChange*/, const Vector& /*oldPosition*/) const
{
	if ((!object.isChildObject()) && (object.getSpatialSubdivisionHandle() != 0))
	{
		ms_tangibleNotTargetableSphereTree.removeObject (object.getSpatialSubdivisionHandle ());
		object.setSpatialSubdivisionHandle (ms_tangibleNotTargetableSphereTree.addObject (&object));
	}

	return true;
}

// ----------------------------------------------------------------------

bool ClientWorld::TangibleNotTargetableNotification::positionAndRotationChanged (Object& object, const bool dueToParentChange, const Vector& oldPosition) const
{
	return positionChanged (object, dueToParentChange, oldPosition);
}

// ----------------------------------------------------------------------

void ClientWorld::TangibleNotTargetableNotification::extentChanged (Object& object) const
{
	if ((!object.isChildObject()) && (object.getSpatialSubdivisionHandle() != 0))
	{
		ms_tangibleNotTargetableSphereTree.removeObject (object.getSpatialSubdivisionHandle ());
		object.setSpatialSubdivisionHandle (ms_tangibleNotTargetableSphereTree.addObject (&object));
	}
}

// ======================================================================
// ClientWorld::TangibleFloraNotification
// ======================================================================

ClientWorld::TangibleFloraNotification::TangibleFloraNotification()
: ObjectNotification()
{
}

// ----------------------------------------------------------------------

ClientWorld::TangibleFloraNotification::~TangibleFloraNotification()
{
}

// ----------------------------------------------------------------------

void ClientWorld::TangibleFloraNotification::addToWorld(Object &object) const
{
#if CLIENTWORLD_VERBOSE_ERROR_CHECKING
	int const existingListIndex = ClientWorld::findObjectList(&object);
	FATAL(existingListIndex != WOL_MarkerAllObjectListsEnd, ("ClientWorld::TangibleFloraNotification::addToWorld: object %s is already in ClientWorld (list %i)", object.getDebugInformation(true).c_str(), existingListIndex));
#endif

	if (!object.isChildObject())
	{
		DEBUG_REPORT_LOG (ms_logWorld, ("ClientWorld::addObject WOL_TangibleFlora pointer=%p networkId=%s objectTemplateName=%s appearanceTemplateName=%s\n", &object, object.getNetworkId ().getValueString ().c_str (), object.getObjectTemplateName () ? object.getObjectTemplateName () : "null", object.getAppearance () && object.getAppearance ()->getAppearanceTemplate () && object.getAppearance ()->getAppearanceTemplate ()->getName () ? object.getAppearance ()->getAppearanceTemplate ()->getName () : "null"));
		World::addObject(&object, WOL_TangibleFlora);

		object.setSpatialSubdivisionHandle (ms_tangibleFloraSphereTree.addObject (&object));
	}
}

// ----------------------------------------------------------------------

void ClientWorld::TangibleFloraNotification::removeFromWorld(Object &object) const
{
	if (!object.isChildObject())
	{
		DEBUG_REPORT_LOG (ms_logWorld, ("ClientWorld::removeObject WOL_TangibleFlora pointer=%p\n", &object));
		World::removeObject(&object, WOL_TangibleFlora);

		ms_tangibleFloraSphereTree.removeObject (object.getSpatialSubdivisionHandle ());
		object.setSpatialSubdivisionHandle (0);
	}
}

// ----------------------------------------------------------------------

bool ClientWorld::TangibleFloraNotification::positionChanged (Object& /*object*/, const bool /*dueToParentChange*/, const Vector& /*oldPosition*/) const
{
	DEBUG_FATAL (true, ("ClientWorld::TangibleFloraNotification::positionChanged: not allowed"));

	return true;
}

// ----------------------------------------------------------------------

bool ClientWorld::TangibleFloraNotification::positionAndRotationChanged (Object& /*object*/, const bool /*dueToParentChange*/, const Vector& /*oldPosition*/) const
{
	DEBUG_FATAL (true, ("ClientWorld::TangibleFloraNotification::positionAndRotationChanged: not allowed"));

	return true;
}

// ----------------------------------------------------------------------

void ClientWorld::TangibleFloraNotification::extentChanged (Object& object) const
{
	ms_tangibleFloraSphereTree.removeObject (object.getSpatialSubdivisionHandle ());
	object.setSpatialSubdivisionHandle (ms_tangibleFloraSphereTree.addObject (&object));
}

// ======================================================================
// ClientWorld::IntangibleNotification
// ======================================================================

ClientWorld::IntangibleNotification::IntangibleNotification()
: ObjectNotification()
{
}

// ----------------------------------------------------------------------

ClientWorld::IntangibleNotification::~IntangibleNotification()
{
}

// ----------------------------------------------------------------------

void ClientWorld::IntangibleNotification::addToWorld(Object &object) const
{
#if CLIENTWORLD_VERBOSE_ERROR_CHECKING
	int const existingListIndex = ClientWorld::findObjectList(&object);
	FATAL(existingListIndex != WOL_MarkerAllObjectListsEnd, ("ClientWorld::IntangibleNotification::addToWorld: object %s is already in ClientWorld (list %i)", object.getDebugInformation(true).c_str(), existingListIndex));
#endif

	if (!object.isChildObject())
		World::addObject(&object, WOL_Intangible);
}

// ----------------------------------------------------------------------

void ClientWorld::IntangibleNotification::removeFromWorld(Object &object) const
{
	if (!object.isChildObject())
		World::removeObject(&object, WOL_Intangible);
}

// ======================================================================
// PUBLIC ClientWorld
// ======================================================================

void ClientWorld::addRenderHookFunctions(const Camera *camera)
{
	ms_useCamera = camera;
	TerrainObject* const terrainObject = TerrainObject::getInstance ();

	// @todo these should be added when the terrain object is added to the world and left there until the remove
	if (terrainObject && terrainObject->isInWorld() && dynamic_cast<const ClientProceduralTerrainAppearance*> (terrainObject->getAppearance ()))
	{
		CellProperty::getWorldCellProperty()->addPreVisibilityTraversalRenderHookFunction(&ClientWorld::prepareToDrawTerrain);
		CellProperty::getWorldCellProperty()->addPreDrawRenderHookFunction(&ClientTerrainSorter::draw);
		CellProperty::getWorldCellProperty()->addExitRenderHookFunction(&ClientTerrainSorter::clear);
		CellProperty::getWorldCellProperty()->addExitRenderHookFunction(&ClientWorld::postDrawTerrain);
	}
}

// ----------------------------------------------------------------------

void ClientWorld::removeRenderHookFunctions()
{
	ms_useCamera = NULL;
	TerrainObject* const terrainObject = TerrainObject::getInstance ();

	if (terrainObject && terrainObject->isInWorld() && dynamic_cast<const ClientProceduralTerrainAppearance*> (terrainObject->getAppearance ()))
	{
		CellProperty::getWorldCellProperty()->removePreVisibilityTraversalRenderHookFunction(&ClientWorld::prepareToDrawTerrain);
		CellProperty::getWorldCellProperty()->removePreDrawRenderHookFunction(&ClientTerrainSorter::draw);
		CellProperty::getWorldCellProperty()->removeExitRenderHookFunction(&ClientTerrainSorter::clear);
		CellProperty::getWorldCellProperty()->removeExitRenderHookFunction(&ClientWorld::postDrawTerrain);
	}
}

// ----------------------------------------------------------------------

void ClientWorld::draw ()
{
	DEBUG_FATAL (!ms_installed, ("not installed"));

	NP_PROFILER_AUTO_BLOCK_DEFINE("ClientWorld::draw");

	uint camerasRendered = 0;

	int i;
	for (i = 0; i < ms_cameraList->getNumberOfObjects (); i++)
	{
		Camera *camera = static_cast<Camera*> (ms_cameraList->getObject (i));

		if (camera->isActive ())
		{
			debugRender (*camera);

			addRenderHookFunctions(camera);
			camera->renderScene ();
			removeRenderHookFunctions();

			camerasRendered++;
		}
	}

	DEBUG_FATAL (camerasRendered != 1, ("ClientWorld::draw - incorrect number of cameras rendered"));
}

// ----------------------------------------------------------------------

void ClientWorld::addCamera (GameCamera* camera)
{
	DEBUG_FATAL (!ms_installed, ("not installed"));
	NOT_NULL (camera);
	camera->addToWorld();
	ms_cameraList->addObject (camera);

	//-- Since we're using the camera list to alter the camera, ensure that it isn't added to the AlterScheduler
	camera->setUseAlterScheduler(false);
}

// ----------------------------------------------------------------------

void ClientWorld::removeCamera (GameCamera* camera)
{
	DEBUG_FATAL (!ms_installed, ("not installed"));
	NOT_NULL (camera);
	camera->removeFromWorld();
	ms_cameraList->removeObject (camera);
}

// ----------------------------------------------------------------------

void ClientWorld::addLight (Light* light, bool isChild)
{
	DEBUG_FATAL (!ms_installed, ("not installed"));
	NOT_NULL (light);
	DEBUG_WARNING (ms_lightList->getNumberOfObjects () >= 100, ("ClientWorld::addLight - there's an excessive number of lights in the world, count=[%d].", ms_lightList->getNumberOfObjects ()));

	if (!isChild)
	{
		ms_lightList->addObject (light);
		light->addToWorld();
	}

	AlterScheduler::submitForAlter (*light);
}

// ----------------------------------------------------------------------

void ClientWorld::removeLight (Light* light, bool isChild)
{
	DEBUG_FATAL (!ms_installed, ("not installed"));
	NOT_NULL (light);

	if (!isChild)
	{
		ms_lightList->removeObject (light);
		light->removeFromWorld();
	}

	AlterScheduler::removeObject (*light);
}

// ----------------------------------------------------------------------

void ClientWorld::removeAllLights ()
{
	// Remove each light object from the alter scheduler.
	int const lightCount = ms_lightList->getNumberOfObjects ();
	for (int i = 0; i < lightCount; ++i)
	{
		Object *const light = (*ms_lightList)[i];
		if (light)
			AlterScheduler::removeObject (*light);
	}

	if (ms_installed)
		ms_lightList->removeAll (true);
}

// ----------------------------------------------------------------------

int ClientWorld::getNumberOfLights ()
{
	DEBUG_FATAL (!ms_installed, ("not installed"));
	return ms_lightList->getNumberOfObjects ();
}

// ----------------------------------------------------------------------

Light* ClientWorld::getLight (int index)
{
	DEBUG_FATAL (!ms_installed, ("not installed"));
	DEBUG_FATAL (index < 0 || index >= ms_lightList->getNumberOfObjects (), ("ClientWorld::getLight - index %i is out of range", index));
	return static_cast<Light*> (ms_lightList->getObject (index));
}

// ----------------------------------------------------------------------

const Light* ClientWorld::getConstLight (int index)
{
	DEBUG_FATAL (!ms_installed, ("not installed"));
	DEBUG_FATAL (index < 0 || index >= ms_lightList->getNumberOfObjects (), ("ClientWorld::getConstLight - index %i is out of range", index));
	return static_cast<const Light*> (ms_lightList->getObject (index));
}

// ----------------------------------------------------------------------

void ClientWorld::updateIfYouKnowWhatYoureDoing (float time)
{
	update (time);
}

// ----------------------------------------------------------------------

bool ClientWorld::collide(CellProperty const * const cellProperty, Vector const & startPosition_w, Vector const & endPosition_w, CollideParameters const & collideParameters, CollisionInfo & result, uint16 const flags, Object const * const excludeObject, CollisionDetectionExcludeObjectFunction cdeof)
{
#if PRODUCTION == 0
	if (ms_disableCollision)
		return false;

	++ms_collideCalls;

	if (collideParameters.getQuality() == CollideParameters::Q_high)
		++ms_highQualityCollides;
	else
		if (collideParameters.getQuality() == CollideParameters::Q_medium)
			++ms_mediumQualityCollides;
		else
			++ms_lowQualityCollides;
#endif

	NP_PROFILER_AUTO_BLOCK_DEFINE("ClientWorld::collide");
	bool const collided = internalCollide(flags, cellProperty, startPosition_w, endPosition_w, excludeObject ? excludeObject : ms_overrideExcludeObject, cdeof, collideParameters, result);

	return collided;
}

// ----------------------------------------------------------------------

bool ClientWorld::collideObject(Object const * const object, Vector const & startPosition_w, Vector const & endPosition_w, CollideParameters const & collideParameters, CollisionInfo & result, uint16 const flags, Object const * const excludeObject, CollisionDetectionExcludeObjectFunction cdeof)
{
	NP_PROFILER_AUTO_BLOCK_DEFINE("ClientWorld::collideObject");
	bool const collided = internalCollideObject(flags, object, startPosition_w, endPosition_w, excludeObject ? excludeObject : ms_overrideExcludeObject, cdeof, collideParameters, result);

	return collided;
}

// ----------------------------------------------------------------------

bool ClientWorld::collide(CellProperty const * startCell, Vector const & startPosition_w, Vector const & endPosition_w, CollideParameters const & collideParameters, CollisionInfoVector & results, uint16 flags, Object const * excludeObject, CollisionDetectionExcludeObjectFunction cdeof)
{
	NP_PROFILER_AUTO_BLOCK_DEFINE("ClientWorld::collide");
	bool const collided = internalCollide(flags, startCell, startPosition_w, endPosition_w, excludeObject ? excludeObject : ms_overrideExcludeObject, cdeof, collideParameters, results);

	std::stable_sort(results.begin(), results.end(), CollisionInfoSorter());
	
	return collided;
}

// ----------------------------------------------------------------------

void ClientWorld::setCollideOverrideExcludeObject(Object const * const overrideExcludeObject)
{
	ms_overrideExcludeObject = overrideExcludeObject;
}

// ======================================================================
// PUBLIC ClientWorld
// ======================================================================

void ClientWorld::install (ClientWorld::CollisionDetectionFunction newCollisionDetectionFunction)
{
	InstallTimer const installTimer("ClientWorld::install");

	DEBUG_FATAL (ms_installed, ("ClientWorld::install - already installed"));
	ms_installed = true;

	//-- install World
	World::install ();

	//-- set the collision function
	ms_collisionDetectionFunction = newCollisionDetectionFunction;

	//-- create the cameraList
	ms_cameraList = new ObjectList (10);

	//-- create the lightList
	ms_lightList = new ObjectList (100);

	//--
	GroundEnvironment::install();
	ClientRegionEffectManager::install();

#if PRODUCTION == 0
	DebugFlags::registerFlag(ms_disableCollision, "ClientGame/ClientWorld", "disableCollision");
	DebugFlags::registerFlag(ms_disableCollisionSorting, "ClientGame/ClientWorld", "disableCollisionSorting");
#endif
	DebugFlags::registerFlag(ms_concludePlayerOnly, "ClientGame/ClientWorld", "concludePlayerOnly");
	DebugFlags::registerFlag(ms_debugReport, "ClientGame/ClientWorld", "reportWorld", ClientWorld::debugReport);
	DebugFlags::registerFlag(ms_renderTangibleSphereTree, "ClientGame/ClientWorld", "renderTangibleSphereTree");
	DebugFlags::registerFlag(ms_renderTangibleNotTargetableSphereTree, "ClientGame/ClientWorld", "renderTangibleNotTargetableSphereTree");
	DebugFlags::registerFlag(ms_renderTangibleFloraSphereTree, "ClientGame/ClientWorld", "renderTangibleFloraSphereTree");
	DebugFlags::registerFlag(ms_logWorld, "ClientGame/ClientWorld", "logWorld");
}

// ----------------------------------------------------------------------

void ClientWorld::remove ()
{
	DEBUG_FATAL (!ms_installed, ("ClientWorld::remove - not installed"));
	ms_installed = false;

	//-- remove World
	World::remove ();

	//-- verify the sphere trees are empty (they should have been cleared via World::remove)
	if (ms_tangibleSphereTree.getObjectCount () > 0)
	{
		DEBUG_WARNING (true, ("ClientWorld::remove: Tangible sphere tree not empty"));
		clearSphereTree (ms_tangibleSphereTree);
	}

	if (ms_tangibleNotTargetableSphereTree.getObjectCount () != 0)
	{
		DEBUG_WARNING (true, ("ClientWorld::remove: TangibleNotTargetable sphere tree not empty"));
		clearSphereTree (ms_tangibleNotTargetableSphereTree);
	}

	if (ms_tangibleFloraSphereTree.getObjectCount () != 0)
	{
		DEBUG_WARNING (true, ("ClientWorld::remove: TangibleFlora sphere tree not empty"));
		clearSphereTree (ms_tangibleFloraSphereTree);
	}

	//-- delete the camera list
	delete ms_cameraList;
	ms_cameraList = 0;

	//-- delete the light list
	ms_lightList->removeAll (true);
	delete ms_lightList;
	ms_lightList = 0;

	//--
	GroundEnvironment::remove();
	ClientRegionEffectManager::remove();

#if PRODUCTION == 0
	DebugFlags::unregisterFlag(ms_disableCollision);
	DebugFlags::unregisterFlag(ms_disableCollisionSorting);
#endif
	DebugFlags::unregisterFlag(ms_concludePlayerOnly);
	DebugFlags::unregisterFlag(ms_debugReport);
	DebugFlags::unregisterFlag(ms_renderTangibleSphereTree);
	DebugFlags::unregisterFlag(ms_renderTangibleNotTargetableSphereTree);
	DebugFlags::unregisterFlag(ms_renderTangibleFloraSphereTree);
	DebugFlags::unregisterFlag(ms_logWorld);

	std::for_each(ms_objectVectorList.begin(), ms_objectVectorList.end(), PointerDeleter());
	ms_objectVectorList.clear();
}

// ----------------------------------------------------------------------

void ClientWorld::defaultCollisionDetectionFunction (float time)
{
	UNREF (time);
}

// ----------------------------------------------------------------------

void ClientWorld::update (float time)
{
	DEBUG_FATAL (!ms_installed, ("not installed"));

	NP_PROFILER_AUTO_BLOCK_DEFINE("ClientWorld::update");

	//-- begin
	World::beginFrame ();
	{
		//-- alter and then conclude all cameras, lights, and world objects that requested an alter.
		ClientCommandQueue::executeClientCommands();
		if (ms_concludePlayerOnly)
			AlterScheduler::alterAllAndConcludeOne (time, Game::getPlayer());
		else
			AlterScheduler::alterAndConclude (time);

		//-- now run collision so that the final position of the
		//    objects are known before the cameras are altered
		{
			NP_PROFILER_AUTO_BLOCK_DEFINE("ClientWorld collisionDetectionFunction");
			if (ms_collisionDetectionFunction)
				ms_collisionDetectionFunction (time);
		}

		//-- alter and conclude cameras
		{
			NP_PROFILER_AUTO_BLOCK_DEFINE("ClientWorld::update cameras");
			ms_cameraList->prepareToAlter ();
			ms_cameraList->alter (time);
			ms_cameraList->conclude ();
		}

		//-- alter and conclude Environments
		{
			NP_PROFILER_AUTO_BLOCK_DEFINE("ClientWorld::update Environments");
			GroundEnvironment::getInstance().alter(time);
			SpaceEnvironment::setCamera(Game::getCamera());
			SpaceEnvironment::alter(time);
		}
	}

	ShaderPrimitiveSorter::alter(time);
	//-- end
	World::endFrame ();
}

// ----------------------------------------------------------------------

void ClientWorld::prepareToDrawTerrain ()
{
	NOT_NULL (TerrainObject::getInstance ());
	TerrainObject::getInstance ()->preRender (ms_useCamera);
}

// ----------------------------------------------------------------------

void ClientWorld::postDrawTerrain ()
{
	NOT_NULL (TerrainObject::getInstance ());
	TerrainObject::getInstance ()->postRender ();
}

// ======================================================================
// PRIVATE ClientWorld
// ======================================================================

ClientWorld::TangibleNotification              ClientWorld::ms_tangibleNotification;
ClientWorld::TangibleNotTargetableNotification ClientWorld::ms_tangibleNotTargetableNotification;
ClientWorld::TangibleFloraNotification         ClientWorld::ms_tangibleFloraNotification;
ClientWorld::IntangibleNotification            ClientWorld::ms_intangibleNotification;

// ----------------------------------------------------------------------

Object* ClientWorld::findObjectByName (const char * name, bool searchQueued)
{
	NOT_NULL (name);

	if (!ms_installed)
		return 0;

	Object * object = NULL;

	for(int i = 0; i < getNumberOfObjects(WOL_Tangible); i++)
	{
		Object * const o = getObject(WOL_Tangible, i);
		if (o && o->getDebugName () && !_stricmp (name, o->getDebugName ()))
			return o;
	}

	if (!object && searchQueued)
	{
		//If we are supposed to search the queued list also, do it.
		for(int i = 0; i < getNumberOfQueuedObjects(); i++)
		{
			Object * const o = getQueuedObject(i);
			if (o && o->getDebugName () && !_stricmp (name, o->getDebugName ()))
				return o;
		}
	}

	return 0;
}

// ----------------------------------------------------------------------

void ClientWorld::findObjectsInRange (const Vector& position_w, const float range, ObjectVector& objectVector)
{
	ms_tangibleSphereTree.findInRange (position_w, range, objectVector);
}

// ----------------------------------------------------------------------

bool ClientWorld::isObjectInCone(const Object & testObject, const Object & coneCenterObject, const Object & coneDirectionObject, float angle)
{
	// NOTE: all calculations are made within the object space of coneCenterObject.

	//-- Compute cone axis vector.
	Vector coneAxisVector(coneCenterObject.rotateTranslate_w2o(coneDirectionObject.getPosition_w()));
	IGNORE_RETURN(coneAxisVector.normalize());

	//-- Get test object orientation in cone's center object's space.
	Vector testOrientation(coneCenterObject.rotateTranslate_w2o(testObject.getPosition_w()));
	IGNORE_RETURN(testOrientation.normalize());

	//-- return true if angle between forward and position_o is smaller than the cone angle
	const float dotProduct   = coneAxisVector.dot(testOrientation);
	const bool  withinCone   = (dotProduct >= cos(angle));
	
	return withinCone;
}

// ----------------------------------------------------------------------

bool ClientWorld::isObjectInCone(const Object & testObject, const Vector & coneCenterPosition, const Vector & coneDirectionPosition, float angle)
{
	// NOTE: all calculations are made within the object space of coneCenterObject.

	//-- Compute cone axis vector.
	Vector coneAxisVector = coneDirectionPosition - coneCenterPosition;
	IGNORE_RETURN(coneAxisVector.normalize());

	//-- Get test object orientation in cone's center object's space.
	Vector testOrientation = testObject.getPosition_w() - coneCenterPosition;
	IGNORE_RETURN(testOrientation.normalize());

	//-- return true if angle between forward and position_o is smaller than the cone angle
	const float dotProduct   = coneAxisVector.dot(testOrientation);
	const bool  withinCone   = (dotProduct >= cos(angle));
	
	return withinCone;
}

// ----------------------------------------------------------------------

Object* ClientWorld::findObjectByLocalizedAbbrev (const Vector & pos_w, const Unicode::String & abbrev, float range, bool searchQueued)
{
	if (!ms_installed)
		return 0;

	typedef stdmap<Unicode::String, ClientObject *>::fwd NameObjectMap;
	NameObjectMap nameObjectMap;

	const float rangeSquared = sqr (range > 0.0f ? range : 1000000.0f);

	for (int i = 0; i < getNumberOfObjects(WOL_Tangible); i++)
	{
		ClientObject * const o = getObject(WOL_Tangible, i)->asClientObject();

		if (o)
		{
			const float distanceSquared = o->getPosition_w ().magnitudeBetweenSquared (pos_w);

			if (distanceSquared < rangeSquared)
				testObjectByAbbrev (pos_w, abbrev, nameObjectMap, *o);
		}
	}

	if (searchQueued)
	{
		//If we are supposed to search the queued list also, do it.
		for(int i = 0; i < getNumberOfQueuedObjects(); i++)
		{
			ClientObject * const o = getQueuedObject(i)->asClientObject();

			if (o)
			{
				const float distanceSquared = o->getPosition_w ().magnitudeBetweenSquared (pos_w);

				if (distanceSquared < rangeSquared)
					testObjectByAbbrev (pos_w, abbrev, nameObjectMap, *o);
			}
		}
	}

	if (!nameObjectMap.empty ())
		return (*(nameObjectMap.begin ())).second;

	return 0;
}

// ----------------------------------------------------------------------

void ClientWorldNamespace::getObjectsInCell (const int objectListIndex, const CellProperty* const cellProperty, ClientWorld::ObjectVector& clientObjectVector)
{
	int i;
	for (i = 0; i < ClientWorld::getNumberOfObjects (objectListIndex); ++i)
	{
		ClientObject* const object = ClientWorld::getObject (objectListIndex, i)->asClientObject();
		if (object && object->getParentCell () == cellProperty)
			clientObjectVector.push_back (object);
	}
}

// ----------------------------------------------------------------------

void ClientWorld::getObjectsInCell (const CellProperty* const cellProperty, ObjectVector& objectVector)
{
	ClientWorldNamespace::getObjectsInCell (WOL_Tangible, cellProperty, objectVector);
	ClientWorldNamespace::getObjectsInCell (WOL_TangibleNotTargetable, cellProperty, objectVector);
}

// ======================================================================
// PRIVATE ClientWorld
// ======================================================================

bool ClientWorldNamespace::internalCollideObjectTest(uint16 const flags, Object const * const object, Vector const & startPosition_w, Vector const & endPosition_w, CollideParameters const & collideParameters, CollisionInfo & result)
{
	bool collided = false;

	if (object->getAppearance() && object->getAppearance()->implementsCollide())
	{
		const bool isSkeletalAppearance = object->getAppearance () && object->getAppearance ()->asSkeletalAppearance2 ();
		if (isSkeletalAppearance && !(flags & ClientWorld::CF_skeletal))
			return false;

		if (!isSkeletalAppearance && (flags & ClientWorld::CF_skeletalOnly))
			return false;

		const Vector& scale = object->getScale ();

		Transform const transform_o2w (object->getTransform_o2w ());

		Vector start_o = transform_o2w.rotateTranslate_p2l (startPosition_w);
		Vector end_o = transform_o2w.rotateTranslate_p2l (endPosition_w);

		if (!isSkeletalAppearance && scale != Vector::xyz111)
		{
			start_o.x /= scale.x;
			start_o.y /= scale.y;
			start_o.z /= scale.z;
			end_o.x /= scale.x;
			end_o.y /= scale.y;
			end_o.z /= scale.z;
		}

		bool testExtentsOnly = (flags & ClientWorld::CF_useScaleThreshold) == 0;
		if (!testExtentsOnly && object->getObjectTemplate ())
		{
			const SharedObjectTemplate* const sharedObjectTemplate = object->getObjectTemplate () ? object->getObjectTemplate ()->asSharedObjectTemplate () : 0;
			if (sharedObjectTemplate)
			{
				const float scaleFactor = std::max (std::max (scale.x, scale.y), scale.z);
				testExtentsOnly = scaleFactor < sharedObjectTemplate->getScaleThresholdBeforeExtentTest ();
			}
		}

		bool testExtentFromLongDistance = false;
		if (flags & ClientWorld::CF_longDistanceUseExtent)
		{
			float distanceToObject = (startPosition_w - object->getAppearanceSphereCenter_w()).approximateMagnitude();

			if (distanceToObject > 0 && (object->getAppearanceSphereRadius() / distanceToObject) < ConfigClientGame::getCollisionExtentRatio())
				testExtentFromLongDistance = true;
		}

		if (testExtentFromLongDistance || ((flags & ClientWorld::CF_extentsOnly) && testExtentsOnly))
		{
			BaseExtent const * const extent = getCorrectExtent(object);

			if (extent != 0)
			{
				Vector surfaceNormal;
				float t = 0.0f;
				if (extent->intersect(start_o, end_o, &surfaceNormal, &t))
				{
					collided = true;

					Vector point_o(Vector::linearInterpolate(start_o, end_o, t));

					if (!isSkeletalAppearance)
					{
						// this isn't really correct.  The scale should be applied
						// before the intersection occurs and not after.  This should
						// change sometime in the future -- TMF
						point_o.x *= scale.x;
						point_o.y *= scale.y;
						point_o.z *= scale.z;
					}

					result.setObject(object);
					result.setPoint(transform_o2w.rotateTranslate_l2p(point_o));
					result.setNormal(transform_o2w.rotate_l2p(surfaceNormal));
					result.setTime((point_o.magnitudeBetween(start_o)));
				}
			}
		}
		else
		{
#if PRODUCTION == 0
			++ms_appearanceCollideCalls;
#endif

			//-- check collisions with full appearance
			CollisionInfo info;
			info.setPoint(end_o);
			if (object->getAppearance()->collide(start_o, end_o, collideParameters, info))
			{
				collided = true;

				Vector point_o = info.getPoint();
				if (!isSkeletalAppearance)
				{
					point_o.x *= scale.x;
					point_o.y *= scale.y;
					point_o.z *= scale.z;
				}

				result.setObject(object);
				result.setPoint(transform_o2w.rotateTranslate_l2p(point_o));
				result.setNormal(transform_o2w.rotate_l2p(info.getNormal()));
				result.setTime((point_o.magnitudeBetween(start_o)));
			}
		}
	}

	return collided;
}

// ----------------------------------------------------------------------

bool ClientWorld::internalCollideObject(uint16 const flags, Object const * const object, Vector const & startPosition_w, Vector const & endPosition_w, Object const * const excludeObject, CollisionDetectionExcludeObjectFunction cdeof, CollideParameters const & collideParameters, CollisionInfo & result)
{
#if PRODUCTION == 0
	++ms_collideObjectCalls;
#endif

	if (!object || !object->isInWorld () || object == excludeObject || (cdeof && (*cdeof)(object)))
		return false;

	bool const isSkeletalAppearance = object->getAppearance () && object->getAppearance ()->asSkeletalAppearance2 ();
	if (isSkeletalAppearance && !(flags & CF_skeletal))
	{
		return false;
	}

	if (!isSkeletalAppearance && (flags & CF_skeletalOnly))
	{
		return false;
	}

	result.setPoint(endPosition_w);

	bool collided = false;

	CollisionInfo info;
	if (internalCollideObjectTest(flags, object, startPosition_w, result.getPoint(), collideParameters, info))
	{
		collided = true;
		result   = info;
	}

	//-- check child objects
	{
		int j;
		for (j = 0; j < object->getNumberOfChildObjects (); ++j)
		{
			const Object* const childObject = object->getChildObject (j);
			if (childObject)
			{
				if (!childObject->isChildObject() || flags & CF_childObjects)
				{
					CollisionInfo info;
					if (internalCollideObject (flags, childObject, startPosition_w, result.getPoint (), excludeObject, cdeof, collideParameters, info))
					{
						collided = true;
						result   = info;
					}
				}
			}
		}
	}

	return collided;
}

// ----------------------------------------------------------------------

bool ClientWorld::internalCollideObject(uint16 const flags, Object const * object, Vector const & startPosition_w, Vector const & endPosition_w, Object const * excludeObject, CollisionDetectionExcludeObjectFunction cdeof, CollideParameters const & collideParameters, CollisionInfoVector & results)
{
#if PRODUCTION == 0
	++ms_collideObjectCalls;
#endif

	if (!object || !object->isInWorld () || object == excludeObject || (cdeof && (*cdeof)(object)))
		return false;

	bool const isSkeletalAppearance = object->getAppearance () && object->getAppearance ()->asSkeletalAppearance2 ();
	if (isSkeletalAppearance && !(flags & CF_skeletal))
	{
		return false;
	}

	if (!isSkeletalAppearance && (flags & CF_skeletalOnly))
	{
		return false;
	}

	bool collided = false;

	CollisionInfo info;
	if (internalCollideObjectTest(flags, object, startPosition_w, endPosition_w, collideParameters, info))
	{
		results.push_back(info);
		collided = true;
	}

	//-- check child objects
	{
		int j;
		for (j = 0; j < object->getNumberOfChildObjects (); ++j)
		{
			const Object* const childObject = object->getChildObject (j);
			if (childObject)
			{
				if (!childObject->isChildObject() || flags & CF_childObjects)
				{
					CollisionInfo info;
					if (internalCollideObject (flags, childObject, startPosition_w, endPosition_w, excludeObject, cdeof, collideParameters, info))
					{
						results.push_back(info);
						collided = true;
					}
				}
			}
		}
	}

	return collided;
}

// ----------------------------------------------------------------------

bool ClientWorld::internalCollideClosestObject(uint16 const flags, int const objectListIndex, Vector const & startPosition_w, Vector const & endPosition_w, Object const * const excludeObject, CollisionDetectionExcludeObjectFunction cdeof, CollideParameters const & collideParameters, CollisionInfo & result)
{
	result.setPoint (endPosition_w);

	ObjectVector & objectVector = allocateObjectVector();

	//-- Query desired sphere tree
	switch (objectListIndex)
	{
	case WOL_Tangible:
		{
			if (flags & CF_skeletal)
				ms_tangibleSphereTree.findOnSegment(startPosition_w, endPosition_w, objectVector);
			else
				ms_tangibleSphereTree.findOnSegment(startPosition_w, endPosition_w, NoSkeletalAppearanceFilter(), objectVector);
		}
		break;

	case WOL_TangibleNotTargetable:
		{
			if (flags & CF_skeletal)
				ms_tangibleNotTargetableSphereTree.findOnSegment(startPosition_w, endPosition_w, objectVector);
			else
				ms_tangibleNotTargetableSphereTree.findOnSegment(startPosition_w, endPosition_w, NoSkeletalAppearanceFilter(), objectVector);
		}
		break;

	case WOL_TangibleFlora:
		{
			if (flags & CF_skeletal)
				ms_tangibleFloraSphereTree.findOnSegment(startPosition_w, endPosition_w, objectVector);
			else
				ms_tangibleFloraSphereTree.findOnSegment(startPosition_w, endPosition_w, NoSkeletalAppearanceFilter(), objectVector);
		}
		break;
	}

	//-- Sort objects front to back based on proximity to line
#if PRODUCTION == 0
	if (!ms_disableCollisionSorting)
#endif
	{
		std::stable_sort(objectVector.begin(), objectVector.end(), CollisionObjectSorter(startPosition_w));
	}

	//-- Iterate through returned objects for collision
	bool collided = false;

	for (size_t i = 0; i < objectVector.size (); ++i)
	{
		const Object* const object = objectVector [i];
		if (object)
		{
			CollisionInfo info;
			if (internalCollideObject (flags, object, startPosition_w, result.getPoint (), excludeObject, cdeof, collideParameters, info))
			{
				collided = true;
				result   = info;
			}
		}
	}

	freeObjectVector(objectVector);

	return collided;
}

// ----------------------------------------------------------------------

bool ClientWorld::internalCollideFindAllObjects(uint16 const flags, int const objectListIndex, Vector const & startPosition_w, Vector const & endPosition_w, Object const * const excludeObject, CollisionDetectionExcludeObjectFunction cdeof, CollideParameters const & collideParameters, CollisionInfoVector & results)
{
	ObjectVector & objectVector = allocateObjectVector();

	//-- Query desired sphere tree
	switch (objectListIndex)
	{
	case WOL_Tangible:
		{
			if (flags & CF_skeletal)
				ms_tangibleSphereTree.findOnSegment(startPosition_w, endPosition_w, objectVector);
			else
				ms_tangibleSphereTree.findOnSegment(startPosition_w, endPosition_w, NoSkeletalAppearanceFilter(), objectVector);
		}
		break;

	case WOL_TangibleNotTargetable:
		{
			if (flags & CF_skeletal)
				ms_tangibleNotTargetableSphereTree.findOnSegment(startPosition_w, endPosition_w, objectVector);
			else
				ms_tangibleNotTargetableSphereTree.findOnSegment(startPosition_w, endPosition_w, NoSkeletalAppearanceFilter(), objectVector);
		}
		break;

	case WOL_TangibleFlora:
		{
			if (flags & CF_skeletal)
				ms_tangibleFloraSphereTree.findOnSegment(startPosition_w, endPosition_w, objectVector);
			else
				ms_tangibleFloraSphereTree.findOnSegment(startPosition_w, endPosition_w, NoSkeletalAppearanceFilter(), objectVector);
		}
		break;
	}

	//-- Sort objects front to back based on proximity to line
	std::stable_sort(objectVector.begin(), objectVector.end(), CollisionObjectSorter(startPosition_w));

	//-- Iterate through returned objects for collision
	bool collided = false;

	size_t numberOfObjectsToTest = objectVector.size();

	for (size_t i = 0; i < numberOfObjectsToTest; ++i)
	{
		Object const * const object = objectVector[i];
		if (object != 0)
		{
			CollisionInfo info;
			if (internalCollideObject(flags, object, startPosition_w, endPosition_w, excludeObject, cdeof, collideParameters, info))
			{
				results.push_back(info);
				collided = true;
			}
		}
	}

	freeObjectVector(objectVector);

	return collided;
}

// ----------------------------------------------------------------------

bool ClientWorld::internalCollide(uint16 const flags, CellProperty const * const cellProperty, Vector const & startPosition_w, Vector const & endPosition_w, Object const * const excludeObject, CollisionDetectionExcludeObjectFunction cdeof, CollideParameters const & collideParameters, CollisionInfo & result)
{
	if (startPosition_w == endPosition_w)
		return false;

	NOT_NULL (cellProperty);
	result.setPoint (endPosition_w);

	bool collided = false;

	//-- am i in the world cell?
	if (cellProperty == CellProperty::getWorldCellProperty ())
	{
		//-- collide with the terrain
		{
			const TerrainObject* const terrainObject = TerrainObject::getConstInstance ();
			if (terrainObject)
			{
				if (flags & CF_terrain)
				{
					CollisionInfo info;
					if (terrainObject->collide (startPosition_w, result.getPoint (), info))
					{
						collided = true;
						result   = info;
					}
				}

				if (flags & CF_terrainFlora)
				{
					CollisionInfo info;
					if (terrainObject->collideObjects (startPosition_w, result.getPoint (), info))
					{
						collided = true;
						result   = info;
					}
				}
			}
		}

		//-- take the shorter ray and collide with any tangibles in the world cell
		if (flags & CF_tangible)
		{
			CollisionInfo info;
			if (internalCollideClosestObject(flags, WOL_Tangible, startPosition_w, result.getPoint (), excludeObject, cdeof, collideParameters, info))
			{
				collided = true;
				result   = info;
			}
		}

		//-- take the shorter ray and collide with any tangible not targetables in the world cell
		if (flags & CF_tangibleNotTargetable)
		{
			CollisionInfo info;
			if (internalCollideClosestObject(flags, WOL_TangibleNotTargetable, startPosition_w, result.getPoint (), excludeObject, cdeof, collideParameters, info))
			{
				collided = true;
				result   = info;
			}
		}

		//-- collide with terrain flora
		if (flags & CF_tangibleFlora)
		{
			CollisionInfo info;
			if (internalCollideClosestObject(flags, WOL_TangibleFlora, startPosition_w, result.getPoint (), excludeObject, cdeof, collideParameters, info))
			{
				collided = true;
				result   = info;
			}
		}
	}
	else
	{
		Object const & object = cellProperty->getOwner ();

		CollisionInfo info;
		if (internalCollideObject (flags, &object, startPosition_w, result.getPoint (), excludeObject, cdeof, collideParameters, info))
		{
			collided = true;
			result   = info;
		}
	}

	//-- lastly, check for portal collisions and recurse
	if (!(flags & CF_disablePortalCrossing))
	{
		Transform const transform_o2w = cellProperty->getOwner ().getTransform_o2w ();
		const Vector start_o = transform_o2w.rotateTranslate_p2l (startPosition_w);
		const Vector end_o   = transform_o2w.rotateTranslate_p2l (result.getPoint ());

		float t;
		const CellProperty* const destinationCellProperty = cellProperty->getDestinationCell (start_o, end_o, t);
		if (destinationCellProperty)
		{
			const Vector start_w = Vector::linearInterpolate (startPosition_w, result.getPoint (), t);
			const Vector end_w   = result.getPoint ();

			CollisionInfo info;
			if (internalCollide (flags, destinationCellProperty, start_w, end_w, excludeObject, cdeof, collideParameters, info))
			{
				collided = true;
				result   = info;
			}
		}
	}

	return collided;
}

//-------------------------------------------------------------------

Object const * ClientWorld::findClosestCellObjectFromWorldPosition(Vector const & position_w)
{
	Object const * cellObject = &(CellProperty::getWorldCellProperty()->getOwner());
	
	ObjectVector objects;
	ms_tangibleSphereTree.findInRange(position_w, 1.0f, objects);

	for (ObjectVector::const_iterator itObj = objects.begin(); itObj != objects.end(); ++itObj) 
	{
		Object const * const object = *itObj;
		if (object)
		{
			PortalProperty const * const portal = object->getPortalProperty();
			if (portal) 
			{
				Vector const position_l = object->rotateTranslate_w2o(position_w);
				
				CellProperty const * const cell = const_cast<PortalProperty *>(portal)->findContainingCell(position_l);
				NetworkId const & containingCellId = cell->getOwner().getNetworkId();

				if (containingCellId.isValid())
				{
					cellObject = &(cell->getOwner());
					break;
				}
			}
		}
	}
	
	return cellObject;
}


// ----------------------------------------------------------------------

bool ClientWorld::internalCollide(uint16 const flags, CellProperty const * cellProperty, Vector const & startPosition_w, Vector const & endPosition_w, Object const * excludeObject, CollisionDetectionExcludeObjectFunction cdeof, CollideParameters const & collideParameters, CollisionInfoVector & results)
{
	if (startPosition_w == endPosition_w)
	{
		return false;
	}

	NOT_NULL (cellProperty);

	bool collided = false;

	//-- am i in the world cell?
	if (cellProperty == CellProperty::getWorldCellProperty())
	{
		//-- collide with the terrain
		{
			const TerrainObject* const terrainObject = TerrainObject::getConstInstance();
			if (terrainObject)
			{
				if (flags & CF_terrain)
				{
					CollisionInfo info;
					if (terrainObject->collide (startPosition_w, endPosition_w, info))
					{
						results.push_back(info);
						collided = true;
					}
				}

				if (flags & CF_terrainFlora)
				{
					CollisionInfo info;
					if (terrainObject->collideObjects (startPosition_w, endPosition_w, info))
					{
						results.push_back(info);
						collided = true;
					}
				}
			}
		}

		//-- take the shorter ray and collide with any tangibles in the world cell
		if (flags & CF_tangible)
		{
			if (internalCollideFindAllObjects(flags, WOL_Tangible, startPosition_w, endPosition_w, excludeObject, cdeof, collideParameters, results))
			{
				collided = true;
			}
		}

		//-- take the shorter ray and collide with any tangible not targetables in the world cell
		if (flags & CF_tangibleNotTargetable)
		{
			if (internalCollideFindAllObjects(flags, WOL_TangibleNotTargetable, startPosition_w, endPosition_w, excludeObject, cdeof, collideParameters, results))
			{
				collided = true;
			}
		}

		//-- collide with terrain flora
		if (flags & CF_tangibleFlora)
		{
			if (internalCollideFindAllObjects(flags, WOL_TangibleFlora, startPosition_w, endPosition_w, excludeObject, cdeof, collideParameters, results))
			{
				collided = true;
			}
		}
	}
	else
	{
		Object const & object = cellProperty->getOwner();

		if (internalCollideObject(flags, &object, startPosition_w, endPosition_w, excludeObject, cdeof, collideParameters, results))
		{
			collided = true;
		}
	}

	//-- lastly, check for portal collisions and recurse
	if (!(flags & CF_disablePortalCrossing))
	{
		Transform const transform_o2w = cellProperty->getOwner ().getTransform_o2w ();
		const Vector start_o = transform_o2w.rotateTranslate_p2l (startPosition_w);
		const Vector end_o   = transform_o2w.rotateTranslate_p2l (endPosition_w);

		float t = 0.0f;
		const CellProperty* const destinationCellProperty = cellProperty->getDestinationCell (start_o, end_o, t);
		if (destinationCellProperty)
		{
			const Vector start_w = Vector::linearInterpolate (startPosition_w, endPosition_w, t);
			const Vector end_w = endPosition_w;

			if (internalCollide(flags, destinationCellProperty, start_w, end_w, excludeObject, cdeof, collideParameters, results))
			{
				collided = true;
			}
		}
	}

	return collided;
}

void ClientWorld::debugReport ()
{
	DEBUG_REPORT_PRINT (true, ("-- ClientWorld\n"));
	DEBUG_REPORT_PRINT (true, ("                     cameraList %4i cameras\n", ms_cameraList->getNumberOfObjects ()));
	DEBUG_REPORT_PRINT (true, ("                      lightList %4i objects\n", ms_lightList->getNumberOfObjects ()));
	DEBUG_REPORT_PRINT (true, ("             tangibleSphereTree %4i objects\n", ms_tangibleSphereTree.getObjectCount ()));
	DEBUG_REPORT_PRINT (true, ("tangibleNotTargetableSphereTree %4i objects\n", ms_tangibleNotTargetableSphereTree.getObjectCount ()));
	DEBUG_REPORT_PRINT (true, ("        tangibleFloraSphereTree %4i objects\n", ms_tangibleFloraSphereTree.getObjectCount ()));

#if PRODUCTION == 0
	DEBUG_REPORT_PRINT (true, ("                   collideCalls %4i\n", ms_collideCalls));
	DEBUG_REPORT_PRINT (true, ("             collideObjectCalls %4i\n", ms_collideObjectCalls));
	DEBUG_REPORT_PRINT (true, ("         appearanceCollideCalls %4i\n", ms_appearanceCollideCalls));
	DEBUG_REPORT_PRINT (true, ("            highQualityCollides %4i\n", ms_highQualityCollides));
	DEBUG_REPORT_PRINT (true, ("          mediumQualityCollides %4i\n", ms_mediumQualityCollides));
	DEBUG_REPORT_PRINT (true, ("             lowQualityCollides %4i\n", ms_lowQualityCollides));

	ms_collideCalls = 0;
	ms_collideObjectCalls = 0;
	ms_appearanceCollideCalls = 0;
	ms_highQualityCollides = 0;
	ms_mediumQualityCollides = 0;
	ms_lowQualityCollides = 0;
#endif
}

// ----------------------------------------------------------------------

void ClientWorldNamespace::debugRender (Camera const & camera)
{
#ifdef _DEBUG
	if (ms_renderTangibleSphereTree || ms_renderTangibleNotTargetableSphereTree || ms_renderTangibleFloraSphereTree)
	{
		ms_objectSphereList.clear ();

		if (ms_renderTangibleSphereTree)
			ms_tangibleSphereTree.dumpSphereTreeObjs (ms_objectSphereList);

		if (ms_renderTangibleNotTargetableSphereTree)
			ms_tangibleNotTargetableSphereTree.dumpSphereTreeObjs (ms_objectSphereList);

		if (ms_renderTangibleFloraSphereTree)
			ms_tangibleFloraSphereTree.dumpSphereTreeObjs (ms_objectSphereList);

		uint i;
		for (i = 0; i < ms_objectSphereList.size (); ++i)
		{
			const Sphere& sphere = ms_objectSphereList [i].second;
			camera.addDebugPrimitive (new SphereDebugPrimitive (UtilityDebugPrimitive::S_z, Transform::identity, sphere.getCenter (), sphere.getRadius (), 10, 10));
		}
	}
#endif
	UNREF(camera);
}

// ======================================================================

