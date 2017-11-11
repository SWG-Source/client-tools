// ======================================================================
//
// ClientWorld.h
// copyright 2000, verant interactive
// copyright 2003, sony online entertainment
//
// ======================================================================

#ifndef	INCLUDED_ClientWorld_H
#define	INCLUDED_ClientWorld_H

// ======================================================================

#include "sharedObject/ObjectNotification.h"
#include "sharedObject/World.h"

class Camera;
class CellProperty;
class CollideParameters;
class CollisionInfo;
class ClientObject;
class GameCamera;
class Light;
class Object;
class Vector;

// ======================================================================

class ClientWorld : public World
{
public:

	typedef void (*CollisionDetectionFunction) (float time);
	typedef bool (*CollisionDetectionExcludeObjectFunction) (Object const *);
	typedef stdvector<Object*>::fwd ObjectVector;

public:

	enum CollisionFlags
	{
		CF_terrain               = BINARY4(0000,0000,0000,0001),
		CF_terrainFlora          = BINARY4(0000,0000,0000,0010),
		CF_tangible              = BINARY4(0000,0000,0000,0100),
		CF_tangibleNotTargetable = BINARY4(0000,0000,0000,1000),
		CF_tangibleFlora         = BINARY4(0000,0000,0001,0000),
		CF_interiorObjects       = BINARY4(0000,0000,0010,0000),
		CF_interiorGeometry      = BINARY4(0000,0000,0100,0000),
		CF_childObjects          = BINARY4(0000,0000,1000,0000),
		CF_skeletal              = BINARY4(0000,0001,0000,0000),
		CF_skeletalOnly          = BINARY4(0000,0010,0000,0000),
		CF_extentsOnly           = BINARY4(0000,0100,0000,0000),
		CF_useScaleThreshold     = BINARY4(0000,1000,0000,0000),
		CF_disablePortalCrossing = BINARY4(0001,0000,0000,0000),
		CF_longDistanceUseExtent = BINARY4(0010,0000,0000,0000),

		//-- useful flag combinations
		CF_all                   = CF_terrain | CF_terrainFlora | CF_tangible | CF_tangibleNotTargetable | CF_tangibleFlora | CF_interiorObjects | CF_interiorGeometry | CF_skeletal | CF_childObjects,
		CF_allExtentsOnly        = CF_terrain | CF_terrainFlora | CF_tangible | CF_tangibleNotTargetable | CF_tangibleFlora | CF_interiorObjects | CF_interiorGeometry | CF_skeletal | CF_childObjects | CF_extentsOnly,
		CF_allCamera             = CF_terrain | CF_tangible | CF_tangibleNotTargetable | CF_interiorGeometry | CF_childObjects,
		CF_allSkeletalOnly       = CF_tangible | CF_skeletalOnly
	};

public:

	//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

	class TangibleNotification;
	friend class TangibleNotification;

	class TangibleNotification : public ObjectNotification
	{
	public:
		TangibleNotification();
		virtual ~TangibleNotification();

		virtual void addToWorld(Object &object) const;
		virtual void removeFromWorld(Object &object) const;

		virtual bool positionChanged (Object& object, bool dueToParentChange, const Vector& oldPosition) const;
		virtual bool positionAndRotationChanged (Object& object, bool dueToParentChange, const Vector& oldPosition) const;

		virtual void extentChanged (Object& object) const;

	private:

		TangibleNotification(const TangibleNotification &);
		TangibleNotification &operator =(const TangibleNotification &);
	};

	//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

	class TangibleNotTargetableNotification;
	friend class TangibleNotTargetableNotification;

	class TangibleNotTargetableNotification : public ObjectNotification
	{
	public:
		TangibleNotTargetableNotification();
		virtual ~TangibleNotTargetableNotification();

		virtual void addToWorld(Object &object) const;
		virtual void removeFromWorld(Object &object) const;

		virtual bool positionChanged (Object& object, bool dueToParentChange, const Vector& oldPosition) const;
		virtual bool positionAndRotationChanged (Object& object, bool dueToParentChange, const Vector& oldPosition) const;

		virtual void extentChanged (Object& object) const;

	private:

		TangibleNotTargetableNotification(const TangibleNotTargetableNotification &);
		TangibleNotTargetableNotification &operator =(const TangibleNotTargetableNotification &);
	};

	//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

	class TangibleFloraNotification;
	friend class TangibleFloraNotification;

	class TangibleFloraNotification : public ObjectNotification
	{
	public:
		TangibleFloraNotification();
		virtual ~TangibleFloraNotification();

		virtual void addToWorld(Object &object) const;
		virtual void removeFromWorld(Object &object) const;

		virtual bool positionChanged (Object& object, bool dueToParentChange, const Vector& oldPosition) const;
		virtual bool positionAndRotationChanged (Object& object, bool dueToParentChange, const Vector& oldPosition) const;

		virtual void extentChanged (Object& object) const;

	private:

		TangibleFloraNotification(const TangibleFloraNotification &);
		TangibleFloraNotification &operator =(const TangibleFloraNotification &);
	};

	//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

	class IntangibleNotification;
	friend class IntangibleNotification;

	class IntangibleNotification : public ObjectNotification
	{
	public:
		IntangibleNotification();
		virtual ~IntangibleNotification();

		virtual void addToWorld(Object &object) const;
		virtual void removeFromWorld(Object &object) const;

	private:

		IntangibleNotification(const IntangibleNotification &);
		IntangibleNotification &operator =(const IntangibleNotification &);
	};

	//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

public:

	using World::findNextObject;
	using World::getConstLotManager;
	using World::getNumberOfObjects;
	using World::getObject;
	using World::getConstObject;
	using World::findClosestObjectTo;
	using World::findClosestConstObjectTo;
	using World::isInstalled;
	using World::getLotManager;
	using World::queueObject;

	//-- make sure you chain up from derived worlds
	static void          install (CollisionDetectionFunction collisionDetectionFunction=defaultCollisionDetectionFunction);
	static void          remove ();

	static void          defaultCollisionDetectionFunction (float time);

	static void          update (float time);

	static void          prepareToDrawTerrain ();
	static void          postDrawTerrain ();

	static void          addRenderHookFunctions(const Camera *camera);
	static void          removeRenderHookFunctions();
	static void          draw ();

	static void          addCamera (GameCamera* camera);
	static void          removeCamera (GameCamera* camera);

	static void          addLight (Light* light, bool isChild=false);
	static void          removeLight (Light* light, bool isChild=false);
	static void          removeAllLights ();

	static int           getNumberOfLights ();
	static Light*        getLight (int index);
	static const Light*  getConstLight (int index);

	static void          updateIfYouKnowWhatYoureDoing (float time);

	//-- use these functions for polygon accurate collision with everything
	static bool collide(CellProperty const * startCell, Vector const & startPosition_w, Vector const & endPosition_w, CollideParameters const & collideParameters, CollisionInfo & result, uint16 flags=CF_all, Object const * excludeObject=0, CollisionDetectionExcludeObjectFunction cdeof=NULL);
	static bool collideObject(Object const * object, Vector const & startPosition_w, Vector const & endPosition_w, CollideParameters const & collideParameters, CollisionInfo & result, uint16 flags=CF_all, Object const * excludeObject=0, CollisionDetectionExcludeObjectFunction cdeof=NULL);
	typedef stdvector<CollisionInfo>::fwd CollisionInfoVector;
	// results are ordered front to back
	static bool collide(CellProperty const * startCell, Vector const & startPosition_w, Vector const & endPosition_w, CollideParameters const & collideParameters, CollisionInfoVector & results, uint16 flags=CF_all, Object const * excludeObject=0, CollisionDetectionExcludeObjectFunction cdeof=NULL);

	static void setCollideOverrideExcludeObject(Object const * overrideExcludeObject);

	static Object* findObjectByName      (const char * name, bool searchQueued = false); 

	static Object* findObjectByLocalizedAbbrev    (const Vector & pos_w, const Unicode::String & abbrev, float range = -1.0f, bool searchQueued = false); 

	static void                 findObjectsInRange (const Vector& position_w, float range, ObjectVector& objectVector);
	static bool                 isObjectInCone(const Object & testObject, const Object & coneCenterObject, const Object & coneDirectionObject, float angle);
	static bool                 isObjectInCone(const Object & testObject, const Vector & coneCenterPosition, const Vector & coneDirectionPosition, float angle);

	static void  getObjectsInCell (const CellProperty* cellProperty, ObjectVector& objectVector);

	static const ObjectNotification  &getTangibleNotification();
	static const ObjectNotification  &getTangibleNotTargetableNotification();
	static const ObjectNotification  &getTangibleFloraNotification();
	static const ObjectNotification  &getIntangibleNotification();

	static Object const * findClosestCellObjectFromWorldPosition(Vector const & position_w);

private:

	static bool          internalCollide(uint16 const flags, CellProperty const * startCell, Vector const & startPosition_w, Vector const & endPosition_w, Object const * excludeObject, CollisionDetectionExcludeObjectFunction cdeof, CollideParameters const & collideParameters, CollisionInfo & result);
	static bool          internalCollide(uint16 const flags, CellProperty const * startCell, Vector const & startPosition_w, Vector const & endPosition_w, Object const * excludeObject, CollisionDetectionExcludeObjectFunction cdeof, CollideParameters const & collideParameters, CollisionInfoVector & results);
	static bool          internalCollideObject(uint16 const flags, Object const * object, Vector const & startPosition_w, Vector const & endPosition_w, Object const * excludeObject, CollisionDetectionExcludeObjectFunction cdeof, CollideParameters const & collideParameters, CollisionInfo & result);
	static bool          internalCollideObject(uint16 const flags, Object const * object, Vector const & startPosition_w, Vector const & endPosition_w, Object const * excludeObject, CollisionDetectionExcludeObjectFunction cdeof, CollideParameters const & collideParameters, CollisionInfoVector & results);
	static bool          internalCollideClosestObject(uint16 const flags, const int objectListIndex, const Vector& startPosition_w, const Vector& endPosition_w, const Object* excludeObject, CollisionDetectionExcludeObjectFunction cdeof, CollideParameters const & collideParameters, CollisionInfo & result);
	static bool          internalCollideFindAllObjects(uint16 const flags, int const objectListIndex, Vector const & startPosition_w, Vector const & endPosition_w, Object const * const excludeObject, CollisionDetectionExcludeObjectFunction cdeof, CollideParameters const & collideParameters, CollisionInfoVector & results);

	static void          debugReport ();

private:

	static TangibleNotification               ms_tangibleNotification;
	static TangibleNotTargetableNotification  ms_tangibleNotTargetableNotification;
	static TangibleFloraNotification          ms_tangibleFloraNotification;
	static IntangibleNotification             ms_intangibleNotification;

private:

	ClientWorld();
	~ClientWorld();
	ClientWorld(const ClientWorld & other);
	ClientWorld& operator=(const ClientWorld & other);
};

//-----------------------------------------------------------------------

inline const ObjectNotification  &ClientWorld::getTangibleNotification()
{
	return ms_tangibleNotification;
}

// ----------------------------------------------------------------------

inline const ObjectNotification  &ClientWorld::getTangibleNotTargetableNotification()
{
	return ms_tangibleNotTargetableNotification;
}

// ----------------------------------------------------------------------

inline const ObjectNotification  &ClientWorld::getTangibleFloraNotification()
{
	return ms_tangibleFloraNotification;
}

// ----------------------------------------------------------------------

inline const ObjectNotification  &ClientWorld::getIntangibleNotification()
{
	return ms_intangibleNotification;
}

// ======================================================================

#endif
