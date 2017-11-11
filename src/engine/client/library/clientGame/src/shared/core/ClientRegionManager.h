//======================================================================
//
// ClientRegionManager.h
// copyright (c) 2002 Sony Online Entertainment
//
//======================================================================

#ifndef INCLUDED_ClientRegionManager_H
#define INCLUDED_ClientRegionManager_H

//======================================================================

class CreatureObject;
class SpatialSubdivisionHandle;
class Sphere;
class TangibleObject;
class VectorArgb;

#include "sharedMath/SphereTree.h"
#include "StringId.h"

//----------------------------------------------------------------------

class ClientRegionManager
{
public:
	class ClientRegionSphere : public Sphere
	{
	public:
		std::string m_name;
	};

	/** Since we actually want to track the sphere itself, make a trivial extent accessor
	*/
	class ClientRegionSphereExtentAccessor: public BaseSphereTreeAccessor<ClientRegionSphere *, ClientRegionSphereExtentAccessor>
	{
	public:

		static Sphere const & getExtent (const ClientRegionSphere* s)
		{
			return *s;
		}

	};

public:
	static void install ();
	static void remove  ();

	static void update  (float deltaTime);
	static void loadPlanet (const std::string& planetname);
	static void clearLastRegion ();

	static void drawRegions();

	static StringId getRegionOnCurrentPlanetAtPoint(const Vector& point);
	static StringId getRegionAtPoint(const std::string& planet, const Vector& point);
	static StringId getLastCurrentRegion();

private:
	static void checkCurrentRegion ();
	static void clearSphereTree    ();
	static void clearTempSphereTree();
	static void tempLoadPlanet(const std::string& planet);
	static const ClientRegionSphere* getSmallestRegionAtPoint(const SphereTree<ClientRegionSphere*, ClientRegionSphereExtentAccessor>& sphereTree, const Vector& point);


private:
	static SphereTree<ClientRegionSphere*, ClientRegionSphereExtentAccessor> m_clientRegionsSphereTree;
	static stdvector<SpatialSubdivisionHandle*>::fwd                         m_objectsInSphereTree;
	static stdmultimap<StringId, ClientRegionSphere*>::fwd                   m_objectStringMap;
	static SphereTree<ClientRegionSphere*, ClientRegionSphereExtentAccessor> m_tempClientRegionsSphereTree;
	static stdvector<SpatialSubdivisionHandle*>::fwd                         m_tempObjectsInSphereTree;
	static stdmultimap<StringId, ClientRegionSphere*>::fwd                   m_tempObjectStringMap;
};

//======================================================================

#endif
