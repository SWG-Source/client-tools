//===================================================================
//
// ClientRadialFloraManager.cpp
// asommers 9-11-2000
//
// copyright 2000, verant interactive
//
//===================================================================

#include "clientTerrain/FirstClientTerrain.h"
#include "clientTerrain/ClientRadialFloraManager.h"

#include "sharedCollision/BoxExtent.h"
#include "sharedDebug/DebugFlags.h"
#include "sharedFoundation/ExitChain.h"
#include "sharedFoundation/VoidBindSecond.h"
#include "sharedFoundation/VoidMemberFunction.h"
#include "sharedFoundation/MemoryBlockManager.h"
#include "sharedFoundation/MemoryBlockManagerMacros.h"
#include "sharedMath/Transform.h"
#include "sharedMath/VectorArgb.h"
#include "sharedObject/Object.h"
#include "sharedRandom/RandomGenerator.h"
#include "sharedTerrain/ConfigSharedTerrain.h"
#include "sharedTerrain/TerrainObject.h"
#include "clientGraphics/Camera.h"
#include "clientGraphics/DebugPrimitive.h"
#include "clientGraphics/ShaderPrimitiveSorter.h"
#include "clientTerrain/ClientProceduralTerrainAppearance.h"

#include <algorithm>
#include <map>
#include <string>

//===================================================================
// ClientRadialFloraManager::ClearFloraEntry
//===================================================================

class ClientRadialFloraManager::ClearFloraEntry
{
	MEMORY_BLOCK_MANAGER_INTERFACE_WITH_INSTALL;
	
public:

	const ClientRadialFloraManager::ClearFloraEntryList* m_clearFloraEntryList;
	ClientRadialFloraManager::ClearFloraEntryList        m_clearFloraEntryList_w;

	//-- bounding information
	Vector m_center_w;
	float  m_radius;

public:

	ClearFloraEntry ();
	~ClearFloraEntry ();
};

//===================================================================
// PUBLIC ClientRadialFloraManager::ClearFloraEntry
//===================================================================

MEMORY_BLOCK_MANAGER_IMPLEMENTATION_WITH_INSTALL (ClientRadialFloraManager::ClearFloraEntry, true, 0, 0, 0);

//-------------------------------------------------------------------

ClientRadialFloraManager::ClearFloraEntry::ClearFloraEntry () :
	m_clearFloraEntryList (0),
	m_clearFloraEntryList_w (),
	m_center_w (),
	m_radius (0.f)
{
}

//-------------------------------------------------------------------

ClientRadialFloraManager::ClearFloraEntry::~ClearFloraEntry ()
{
	m_clearFloraEntryList = 0;
}

//===================================================================
// PUBLIC ClientRadialFloraManager::RadialNode
//===================================================================

ClientRadialFloraManager::RadialNode::RadialNode () :
	m_manager (0),
	m_hasFlora (false),
	m_alpha (0),
	m_floraAllowed (false),
	m_normal()
{
}

//-------------------------------------------------------------------
	
ClientRadialFloraManager::RadialNode::~RadialNode ()
{
}

//-------------------------------------------------------------------

void ClientRadialFloraManager::RadialNode::setManager (ClientRadialFloraManager* const manager)
{
	m_manager = manager;
}

//-------------------------------------------------------------------

void ClientRadialFloraManager::RadialNode::enabledChanged ()
{
}

//-------------------------------------------------------------------

void ClientRadialFloraManager::RadialNode::setHasFlora (bool hasFlora)
{
	m_hasFlora = hasFlora;
}

//-------------------------------------------------------------------

void ClientRadialFloraManager::RadialNode::setFloraAllowed (bool floraAllowed)
{
	m_floraAllowed = floraAllowed;
}

//-------------------------------------------------------------------

void ClientRadialFloraManager::RadialNode::setNormal (const Vector& normal)
{
	m_normal = normal;
}

//-------------------------------------------------------------------

void ClientRadialFloraManager::RadialNode::setAlpha (float alpha)
{
	DEBUG_FATAL(alpha<0 || alpha>1, ("RadialNode alpha out of range (<0 || >1)"));
	m_alpha = static_cast<uint8>(alpha*255.0f);
}

//===================================================================
// STATIC PUBLIC ClientRadialFloraManager
//===================================================================

namespace ClientRadialFloraManagerNamespace
{
	typedef std::map<const Object*, ClientRadialFloraManager::ClearFloraEntry*> ClearFloraEntryMap;
	ClearFloraEntryMap ms_clearFloraEntryMap;

	// ----------------------------------------------------------------------------------------------

	struct TestClearFloraEntry
	{
		ClientRadialFloraManager::ClearFloraEntry *entry;
		Vector m_center_w;
		float  m_radius;
	};

	typedef std::vector<TestClearFloraEntry> TestClearFloraEntryVector;

	void _updateClearFloraEntryVector(Vector origin, float maximumDistance);

	static TestClearFloraEntryVector clearFloraEntries_w;

	// ----------------------------------------------------------------------------------------------

	bool               ms_renderClearFloraEntryMap;
	bool               ms_debugReport;

	void debugReport ();
}

using namespace ClientRadialFloraManagerNamespace;

//-------------------------------------------------------------------

void ClientRadialFloraManager::install ()
{
	ClearFloraEntry::install ();
	DebugFlags::registerFlag (ms_renderClearFloraEntryMap, "ClientTerrain", "renderClearFloraEntryMap");
	DebugFlags::registerFlag (ms_debugReport, "ClientTerrain", "reportClientRadialFloraManager", debugReport);
}

//-------------------------------------------------------------------

void ClientRadialFloraManager::addClearFloraObject (const Object* const object, const ClearFloraEntryList& clearFloraEntryList)
{
	ClearFloraEntry* const clearFloraEntry = new ClearFloraEntry;

	//-- save off original list
	clearFloraEntry->m_clearFloraEntryList = &clearFloraEntryList;

	//-- transform object space points into world space and create a bounding circle
	clearFloraEntry->m_clearFloraEntryList_w.reserve (clearFloraEntryList.size ());

	float largestRadius = 0.f;

	uint i;
	for (i = 0; i < clearFloraEntryList.size (); ++i)
	{
		const Vector center (object->rotateTranslate_o2w (clearFloraEntryList [i].first));
		const Vector center_w (center.x, 0.f, center.z);

		const float  radius = clearFloraEntryList [i].second;

		clearFloraEntry->m_clearFloraEntryList_w.push_back (std::make_pair (center_w, radius));
		clearFloraEntry->m_center_w += center_w;
		largestRadius = std::max (largestRadius, radius);
	}

	clearFloraEntry->m_center_w /= static_cast<float> (i);

	float largestDistanceToCenter = 0.f;
	
	for (i = 0; i < clearFloraEntry->m_clearFloraEntryList_w.size (); ++i)
	{
		const float largestDistanceToCenterSquared = clearFloraEntry->m_center_w.magnitudeBetweenSquared (clearFloraEntry->m_clearFloraEntryList_w [i].first);
		largestDistanceToCenter = std::max (largestDistanceToCenter, largestDistanceToCenterSquared);
	}

	largestDistanceToCenter = sqrt (largestDistanceToCenter);

	clearFloraEntry->m_radius = largestRadius + largestDistanceToCenter;

	//-- insert into the flora map
	ms_clearFloraEntryMap.insert (std::make_pair (object, clearFloraEntry));

	clearFloraEntries_w.clear(); // safety fallback to avoid crashes. This *shouldn't* be needed.
}

//-------------------------------------------------------------------

void ClientRadialFloraManager::removeClearFloraObject (const Object* const object)
{
	ClearFloraEntryMap::iterator iter = ms_clearFloraEntryMap.find (object);
	if (iter != ms_clearFloraEntryMap.end ())
	{
		delete iter->second;
		ms_clearFloraEntryMap.erase (iter);
	}

	clearFloraEntries_w.clear(); // safety fallback to avoid crashes. This *shouldn't* be needed.
}

//===================================================================
// PUBLIC ClientRadialFloraManager
//===================================================================

ClientRadialFloraManager::ClientRadialFloraManager (const ClientProceduralTerrainAppearance& terrainAppearance, const bool& enabled, float minimumDistance, float const & maximumDistance) :
	FloraManager (),
	m_debugName (new std::string),
	m_enabled (enabled),
	m_lastEnabled(enabled),
	m_terrainAppearance (terrainAppearance),
	m_mapHalfWidthInMeters(terrainAppearance.getMapWidthInMeters()*.5f),
	m_minimumDistance (minimumDistance),
	m_maximumDistance (maximumDistance),
	m_lastMaximumDistance (maximumDistance),
	m_floraTileSize (8.f),
	m_floraTileBorderIgnoreDistance (2.f),
	m_seed (0),
	m_oldOrigin (Vector (maximumDistance * 2.f, 0.f, maximumDistance * 2.f))
{
	DEBUG_FATAL (minimumDistance >= maximumDistance, ("minimumDistance (%1.2f) >= maximumDistance (%1.2f)", minimumDistance, maximumDistance));
}

//-------------------------------------------------------------------
	
ClientRadialFloraManager::~ClientRadialFloraManager ()
{
	delete m_debugName;
	m_debugName = 0;

	RadialNodeList::iterator rni;
	for (rni=m_radialNodeList.begin();rni!=m_radialNodeList.end();++rni)
	{
		delete rni->nodePointer;
	}
}

//-------------------------------------------------------------------

void ClientRadialFloraManager::setDebugName (const std::string& debugName)
{
	*m_debugName = debugName;
}

//-------------------------------------------------------------------

bool ClientRadialFloraManager::isEnabled () const
{
	return m_enabled;
}

//-------------------------------------------------------------------

void ClientRadialFloraManager::_constructRadialNodeReference(RadialNodeReference &o_ref, RadialNode *node)
{
	Vector position = node->getPosition();

	o_ref.nodePointer=node;
	o_ref.position_x = position.x;
	o_ref.position_z = position.z;
	o_ref.floraAllowed = node->getFloraAllowed();
	o_ref.hasFlora = node->getHasFlora();
}

//-------------------------------------------------------------------

void ClientRadialFloraManager::_addRadialNode(RadialNode *newRadialNode)
{
	RadialNodeReference ref;
	_constructRadialNodeReference(ref, newRadialNode);
	m_radialNodeList.push_back(ref);
}

//-------------------------------------------------------------------

#ifdef _DEBUG
void ClientRadialFloraManager::_verifyRadialNodeReference(const RadialNodeReference &reference) const
{
	const RadialNode *node = reference.nodePointer;
	Vector position = node->getPosition();
	bool floraAllowed = node->getFloraAllowed();
	bool hasFlora = node->getHasFlora();

	DEBUG_FATAL(reference.position_x!=position.x || reference.position_z!=position.z, ("Reference position does not match."));
	DEBUG_FATAL(floraAllowed!=reference.floraAllowed, ("Reference floraAllowed flag does not match."));
	DEBUG_FATAL(hasFlora!=reference.hasFlora, ("Reference hasFlora flag does not match."));
}
#endif

//-------------------------------------------------------------------

void ClientRadialFloraManager::initialize (float floraTileSize, float floraTileBorderIgnoreDistance, uint32 seed)
{
	DEBUG_FATAL (floraTileSize < 0.5f, ("floraTileSize (%1.2f) < 0.5f", floraTileSize));
	if (floraTileSize - 2.f * floraTileBorderIgnoreDistance <= 0.f)
	{
		DEBUG_WARNING (true, ("floraTileBorderIgnoreDistance (%1.2f) must be less than 1/2 the floraTileSize (%1.2f)", floraTileBorderIgnoreDistance, floraTileSize));
		floraTileBorderIgnoreDistance = 0.f;
	}

	m_floraTileSize = floraTileSize;
	m_floraTileBorderIgnoreDistance = floraTileBorderIgnoreDistance;
	m_seed = seed;

#ifdef _DEBUG
	int numberOfFlora = 0;
#endif

	//-- place the radial nodes on a fixed grid spaced within the tile. make sure to ignore whatever ignore distance is specified
	RandomGenerator random (seed);

	const float side = m_maximumDistance / floraTileSize;

	float i;
	for (i = -side; i < side; i += 1.f)
	{
		float j;
		for (j = -side; j < side; j += 1.f)
		{
			const float dx = random.randomReal (floraTileSize - 2.f * floraTileBorderIgnoreDistance);
			const float dz = random.randomReal (floraTileSize - 2.f * floraTileBorderIgnoreDistance);
			const Vector position (i * floraTileSize + floraTileBorderIgnoreDistance + dx, 0.f, j * floraTileSize + floraTileBorderIgnoreDistance + dz);

			//-- only add the point if it is within the circle
			if (position.magnitudeSquared () < sqr (m_maximumDistance))
			{
				RadialNode *newRadialNode = createRadialNode(position + m_oldOrigin);
				_addRadialNode(newRadialNode);
				newRadialNode->setManager (this);

#ifdef _DEBUG
				++numberOfFlora;
#endif
			}
		}
	}

	//-- mark this new region as dirty
	Rectangle2d rectangle;
	rectangle.x0 = -8192.f;
	rectangle.y0 = -8192.f;
	rectangle.x1 =  8192.f;
	rectangle.y1 =  8192.f;
	m_regionList.push_back (rectangle);
	
	DEBUG_REPORT_LOG_PRINT(ConfigSharedTerrain::getDebugReportLogPrint (), ("[%s] - %i pieces of created; radius=%1.2f; tileSize=%1.2f\n", m_debugName->c_str () ? m_debugName->c_str () : "unnamed", numberOfFlora, m_maximumDistance, floraTileSize));
}

//-------------------------------------------------------------------

void ClientRadialFloraManager::markRegionDirty (const Rectangle2d& rectangle)
{
	m_regionList.push_back (rectangle);
}

//-------------------------------------------------------------------

void ClientRadialFloraManager::markExtentDirty (const BoxExtent* const extent)
{
	NOT_NULL (extent);

	Rectangle2d rectangle;
	rectangle.x0 = extent->getMin ().x;
	rectangle.y0 = extent->getMin ().z;
	rectangle.x1 = extent->getMax ().x;
	rectangle.y1 = extent->getMax ().z;
	markRegionDirty (rectangle);
}

//-------------------------------------------------------------------

void ClientRadialFloraManager::maximumDistanceChanged ()
{
	m_regionList.clear ();

	RadialNodeList::iterator rni;
	for (rni=m_radialNodeList.begin();rni!=m_radialNodeList.end();++rni)
	{
		delete rni->nodePointer;
	}
	m_radialNodeList.clear ();

	initialize (m_floraTileSize, m_floraTileBorderIgnoreDistance, m_seed);
}

//-------------------------------------------------------------------

void ClientRadialFloraManager::alter (float time)
{
	UNREF(time);

	if (m_lastEnabled != isEnabled ())
	{
		m_lastEnabled = isEnabled ();

		RadialNodeList::iterator rni;
		for (rni=m_radialNodeList.begin();rni!=m_radialNodeList.end();++rni)
		{
			rni->nodePointer->enabledChanged();
		}
	}

	if (!isEnabled ())
	{
		return;
	}

	if (m_lastMaximumDistance != m_maximumDistance)
	{
		m_lastMaximumDistance = m_maximumDistance;

		maximumDistanceChanged ();
	}
}

//-------------------------------------------------------------------

void ClientRadialFloraManager::preRender (const Camera* const /*camera*/)
{
}

//-------------------------------------------------------------------

void ClientRadialFloraManager::draw () const
{
#ifdef _DEBUG
	const Camera& camera = ShaderPrimitiveSorter::getCurrentCamera ();

	if (ms_renderClearFloraEntryMap)
	{
		ClearFloraEntryMap::iterator iter = ms_clearFloraEntryMap.begin ();
		ClearFloraEntryMap::iterator end = ms_clearFloraEntryMap.end ();
		for (; iter != end; ++iter)
		{
			const ClearFloraEntry* const clearFloraEntry = iter->second;

			{
				Vector center = clearFloraEntry->m_center_w;
				TerrainObject::getConstInstance ()->getHeight (center, center.y);

				CircleDebugPrimitive* const debugPrimitive = new CircleDebugPrimitive(CircleDebugPrimitive::S_none, Transform::identity, center, clearFloraEntry->m_radius, 20);
				debugPrimitive->setColor (VectorArgb::solidWhite);
				camera.addDebugPrimitive (debugPrimitive);
			}

			uint i;
			for (i = 0; i < clearFloraEntry->m_clearFloraEntryList_w.size (); ++i)
			{
				Vector center = clearFloraEntry->m_clearFloraEntryList_w [i].first;
				TerrainObject::getConstInstance ()->getHeight (center, center.y);

				CircleDebugPrimitive* const debugPrimitive = new CircleDebugPrimitive(CircleDebugPrimitive::S_none, Transform::identity, center, clearFloraEntry->m_clearFloraEntryList_w [i].second, 20);
				debugPrimitive->setColor (VectorArgb::solidYellow);
				camera.addDebugPrimitive (debugPrimitive);
			}
		}
	}
#endif
}

//-------------------------------------------------------------------

bool ClientRadialFloraManager::collide (const Vector& /*start_w*/, const Vector& /*end_w*/, CollisionInfo& /*result*/) const
{
	return false;
}

//-------------------------------------------------------------------

bool ClientRadialFloraManager::approximateCollide (const Vector& /*start_w*/, const Vector& /*end_w*/, CollisionInfo& /*result*/) const
{
	return false;
}

//===================================================================
// STATIC PROTECTED ClientRadialFloraManager
//===================================================================

bool ClientRadialFloraManager::shouldClearFlora(float positionX, float positionZ, float mapHalfWidthInMeters)
{
	//-- check x and z
	if (  positionX> mapHalfWidthInMeters 
		|| positionZ> mapHalfWidthInMeters
		|| positionX<-mapHalfWidthInMeters
		|| positionZ<-mapHalfWidthInMeters
		)
	{
		return false;
	}

	if (clearFloraEntries_w.empty())
	{
		return false;
	}

	const Vector position2d (positionX, 0.f, positionZ);

	TestClearFloraEntry * iter = &clearFloraEntries_w[0];
	TestClearFloraEntry * stop = iter + clearFloraEntries_w.size();

	for (;iter!=stop; ++iter)
	{
		const TestClearFloraEntry &testEntry = *iter;

		//-- check to see if within the global sphere
		if (testEntry.m_center_w.magnitudeBetweenSquared (position2d) < sqr (testEntry.m_radius))
		{
			const ClearFloraEntry* const clearFloraEntry = testEntry.entry;

			//-- check to see if within any of the smaller spheres
			uint i;
			for (i = 0; i < clearFloraEntry->m_clearFloraEntryList_w.size (); ++i)
				if (clearFloraEntry->m_clearFloraEntryList_w [i].first.magnitudeBetweenSquared (position2d) < sqr (clearFloraEntry->m_clearFloraEntryList_w [i].second))
					return true;
		}
	}

	return false;
}

//===================================================================
// PROTECTED ClientRadialFloraManager
//===================================================================

bool ClientRadialFloraManager::createFlora (float positionX, float positionZ, RadialNode* radialNode, bool& floraAllowed) const
{
	UNREF (positionX);
	UNREF (positionZ);
	UNREF (radialNode);
	UNREF (floraAllowed);
	DEBUG_FATAL (true, ("this should not be called"));

	return false;  //lint !e527  //-- unreachable
}  //lint !e1764  //-- floraAllowed could be made a const reference

//-------------------------------------------------------------------

ClientRadialFloraManager::RadialNode* ClientRadialFloraManager::createRadialNode (const Vector& position) const
{
	UNREF (position);
	DEBUG_FATAL (true, ("this should not be called"));

	return 0;  //lint !e527  //-- unreachable
}

//===================================================================
// STATIC PRIVATE ClientRadialFloraManager
//===================================================================

void ClientRadialFloraManager::clearRegionList () const
{
	m_regionList.clear ();
}

//-------------------------------------------------------------------

bool ClientRadialFloraManager::isWithinDirtyRegion (float x, float y) const
{
	for (RegionList::iterator iterator = m_regionList.begin (); iterator != m_regionList.end (); ++iterator)
		if ((*iterator).isWithin (x, y))
			return true;

	return false;
}

//-------------------------------------------------------------------

void ClientRadialFloraManagerNamespace::_updateClearFloraEntryVector(Vector origin, float maximumDistance)
{
	clearFloraEntries_w.clear();

	const Vector position2d (origin.x, 0.f, origin.z);

	ClearFloraEntryMap::iterator iter = ms_clearFloraEntryMap.begin ();
	for (; iter != ms_clearFloraEntryMap.end (); ++iter)
	{
		ClientRadialFloraManager::ClearFloraEntry* const clearFloraEntry = iter->second;

		float dist = clearFloraEntry->m_center_w.magnitudeBetween(position2d);

		//-- check to see if within the global sphere
		if (dist - maximumDistance < clearFloraEntry->m_radius)
		{
			TestClearFloraEntry testEntry;
			testEntry.entry=clearFloraEntry;
			testEntry.m_center_w=clearFloraEntry->m_center_w;
			testEntry.m_radius=clearFloraEntry->m_radius;
			clearFloraEntries_w.push_back(testEntry);
		}
	}
}

//-------------------------------------------------------------------

void ClientRadialFloraManager::update (const Vector& origin) const
{
	const Vector newOrigin (origin.x, 0.f, origin.z);
	if (m_radialNodeList.empty())
	{
		m_oldOrigin = newOrigin;
		return;
	}

	if (newOrigin == m_oldOrigin && m_regionList.empty ())
	{
		return;
	}

	float mapHalfWidthInMeters = m_mapHalfWidthInMeters;
	float maxDistSqr = sqr(m_maximumDistance);
	RadialNodeReference *ri = &m_radialNodeList[0];
	RadialNodeReference *const ristop = ri + m_radialNodeList.size();

	_updateClearFloraEntryVector(origin, m_maximumDistance);

	for (;ri!=ristop;++ri)
	{
	#ifdef _DEBUG
		_verifyRadialNodeReference(*ri);
	#endif

		RadialNode* const radialNode = ri->nodePointer;

		bool         needsHeightSet = false;

		float        magSqr = sqr(ri->position_x-newOrigin.x) + sqr(ri->position_z-newOrigin.z);
		bool         outsideCircle  = magSqr > maxDistSqr;

		Vector position;

		DEBUG_FATAL(!ri->floraAllowed && ri->hasFlora, ("Node has flora when its not allowed."));

		//-- see if we need to replace a RadialNode
		if (outsideCircle || !ri->floraAllowed)
		{
			if (outsideCircle)
			{
				const Vector oldPosition (ri->position_x, 0.f, ri->position_z);

				//-- flora fell out of circle, so mirror its position in its old circle into the new circle
				position = oldPosition - m_oldOrigin;
				position = -position;
				position += newOrigin;

				radialNode->setPosition (position);
				ri->position_x=position.x;
				ri->position_z=position.z;
			}

			//-- figure out if a piece is supposed to go there
			if (ri->hasFlora)
			{
				needsHeightSet = outsideCircle;

				radialNode->setHasFlora (false);
				ri->hasFlora=false;
			}

			bool shouldCreate = !shouldClearFlora(ri->position_x, ri->position_z, mapHalfWidthInMeters);

			bool floraAllowed = false;
			if (  shouldCreate
				&& createFlora(ri->position_x, ri->position_z, radialNode, floraAllowed)
				)
			{
				DEBUG_FATAL (!floraAllowed, ("can't create flora when it's not allowed"));

				needsHeightSet = true;
				radialNode->setHasFlora (true);
				ri->hasFlora=true;
			}

			if (ri->floraAllowed!=floraAllowed)
			{
				radialNode->setFloraAllowed (floraAllowed);
				ri->floraAllowed=floraAllowed;
			}
		}

	#ifdef _DEBUG
		_verifyRadialNodeReference(*ri);
	#endif

		//-- make sure the ground hasn't changed from under the object
		if (ri->hasFlora)
		{
			if (needsHeightSet || isWithinDirtyRegion(ri->position_x, ri->position_z))
			{
				bool heightSet = false;

				position = radialNode->getPosition ();

				Vector normal = Vector::unitY;
				if (radialNode->shouldFloat ())
				{
					heightSet = m_terrainAppearance.getWaterHeight (position, position.y);
					position.y += 0.05f;
				}
				else
					heightSet = m_terrainAppearance.getHeight (position, position.y, normal);

				if (heightSet)
				{
					radialNode->setPosition (position);
					radialNode->setNormal (normal);

					Transform t = radialNode->getTransform ();
					t.resetRotate_l2p ();
					t.yaw_l (position.x + position.z);
					
					if (radialNode->shouldAlignToTerrain ())
					{
						Vector vk = t.getLocalFrameK_p ();

						Vector vi = normal.cross (vk);
						if (!vi.normalize ())
							DEBUG_FATAL (true, ("couldn't normalize vector"));

						vk = vi.cross (normal);
						if (!vk.normalize ())
							DEBUG_FATAL (true, ("couldn't normalize vector"));

						t.setLocalFrameIJK_p (vi, normal, vk);
						t.reorthonormalize ();	
					}

					radialNode->setTransform (t);

					ri->position_x=position.x;
					ri->position_z=position.z;
				}
			}
		}
	#ifdef _DEBUG
		_verifyRadialNodeReference(*ri);
	#endif
	}

	m_oldOrigin = newOrigin;
}

//===================================================================
// ClientRadialFloraManagerNamespace
//===================================================================

void ClientRadialFloraManagerNamespace::debugReport ()
{
	DEBUG_REPORT_PRINT (true, ("-- ClientRadialFloraManager\n"));
	DEBUG_REPORT_PRINT (true, (" root entries = %i\n", ms_clearFloraEntryMap.size ()));

	int total = 0;
	ClearFloraEntryMap::iterator end = ms_clearFloraEntryMap.end ();
	for (ClearFloraEntryMap::iterator iter = ms_clearFloraEntryMap.begin (); iter != end; ++iter)
		total += iter->second->m_clearFloraEntryList->size ();

	DEBUG_REPORT_PRINT (true, ("total entries = %i\n", total));
}

//===================================================================
