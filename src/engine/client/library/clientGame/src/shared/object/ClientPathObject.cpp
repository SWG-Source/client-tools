// ======================================================================
//
// ClientPathObject.cpp
// asommers
//
// copyright 2003, sony online entertainment
//
// ======================================================================

// see ScriptMethodsTerrain::createClientPath && ScriptMethodsTerrain::createClientPathAdvanced 
// before modifying this code.

#include "clientGame/FirstClientGame.h"
#include "clientGame/ClientPathObject.h"

#include "clientGame/ClientPathObjectNotification.h"
#include "clientGame/ContainerInterface.h"
#include "clientGame/CreatureObject.h"
#include "clientGame/Game.h"
#include "clientGraphics/RenderWorld.h"
#include "clientObject/RibbonAppearance.h"
#include "sharedCollision/CollisionNotification.h"
#include "sharedCollision/CollisionWorld.h"
#include "sharedCollision/FloorContactShape.h"
#include "sharedCollision/MultiList.h"
#include "sharedDebug/InstallTimer.h"
#include "sharedFile/Iff.h"
#include "sharedFoundation/TemporaryCrcString.h"
#include "sharedFoundation/Timer.h"
#include "sharedGame/GroundZoneManager.h"
#include "sharedMath/Vector.h"
#include "sharedMath/VectorArgb.h"
#include "sharedObject/AlterResult.h"
#include "sharedObject/Appearance.h"
#include "sharedObject/AppearanceTemplateList.h"
#include "sharedObject/CellProperty.h"
#include "sharedTerrain/TerrainObject.h"

#include <limits>
#include <map>
#include <vector>

// ======================================================================

namespace ClientPathObjectNamespace
{
	typedef std::vector<Vector> PointList;
	typedef std::map<float /*distance*/, int /*index*/> PathObjectDataMap;
	typedef std::map<Object *, float /*distance*/> PathObjectDistanceMap;

	// ----------------------------------------------------------------------
	
	std::string ms_pathAppearanceName("appearance/pt_find_path_glow.prt");
	std::string ms_pathEndAppearanceName("appearance/pt_arrow_disc.prt");
	
	// ----------------------------------------------------------------------

	const float ms_minRunSpeed = 5.8f;
	float ms_runSpeed = ms_minRunSpeed;
	float ms_playerHeight = 0.75f;
	float ms_expireTime = 3.5f;
	float ms_playerRunSpeedMultiplier = 1.15f;
	int ms_maxObjects = 256;
	
	// ----------------------------------------------------------------------
	
	PathObjectDataMap::iterator getRangeIterator(PathObjectDataMap & dataMap, float distance)
	{
		PathObjectDataMap::iterator it = dataMap.lower_bound(distance);

		if (it != dataMap.end() && it != dataMap.begin()) 
			it--;
		
		return it;
	}
}

using namespace ClientPathObjectNamespace;

// ----------------------------------------------------------------------

void ClientPathObject::install ()
{
	InstallTimer const installTimer("ClientPathObject::install");
}

// ----------------------------------------------------------------------

class ClientPathObject::Implementation
{
public:
	Implementation() :
	m_pointList(),
	m_dataMap(),
	m_distanceMap(),
	m_endPoint(new Object),
	m_pathNodeTimer(1.0f),
	m_pathLength(0.0f),
	m_distanceDelta(0.0f)	
	{
	}

	~Implementation()
	{
		delete m_endPoint;
		m_endPoint = NULL;
	}

	PointList m_pointList;
	PathObjectDataMap m_dataMap;
	PathObjectDistanceMap m_distanceMap;
	Object * m_endPoint;
	
	Timer m_pathNodeTimer;
	
	float m_pathLength;
	float m_distanceDelta;
};

// ----------------------------------------------------------------------

ClientPathObject::ClientPathObject(PointList const & pointList) :
Object(),
m_pimpl(new Implementation)
{
	addNotification(ClientPathObjectNotification::getInstance());
	RenderWorld::addObjectNotifications(*this);

	Appearance * const appearance = AppearanceTemplateList::createAppearance(ms_pathEndAppearanceName.c_str());
	m_pimpl->m_endPoint->setAppearance(appearance);
	
	RenderWorld::addObjectNotifications(*(m_pimpl->m_endPoint));
	addChildObject_p(m_pimpl->m_endPoint);

	if (!pointList.empty())
		create(pointList);
}

// ----------------------------------------------------------------------

ClientPathObject::~ClientPathObject ()
{
	delete m_pimpl;
	m_pimpl = NULL;
}

// ----------------------------------------------------------------------

void ClientPathObject::resetBoundary()
{
	updatePath();
}

// ----------------------------------------------------------------------

void ClientPathObject::create(PointList const & pointList)
{
	int const size = static_cast<int>(pointList.size());
	if (size > 1)
	{
		m_pimpl->m_pointList = pointList;

		optimizePath();
		updatePath();
		updateSpeedAndHeight();

		float const distanceDelta = ms_runSpeed * ms_expireTime;
		for (float distance = 0.0f; distance < m_pimpl->m_pathLength; distance += distanceDelta) 
		{
			addPathNode(distance);
		}
	}
}

// ----------------------------------------------------------------------

float ClientPathObject::alter(float const time)
{
	updateNodes();

	updateSpeedAndHeight();

	m_pimpl->m_distanceDelta = (ms_runSpeed * time);

	int const objectCount = getNumberOfChildObjects();
	if (!objectCount || (m_pimpl->m_pathNodeTimer.updateNoReset(time) && objectCount < ms_maxObjects)) 
	{
		m_pimpl->m_pathNodeTimer.reset();
		addPathNode();
	}

	IGNORE_RETURN(Object::alter(time));
	return AlterResult::cms_alterNextFrame;
}

// ----------------------------------------------------------------------

void ClientPathObject::updatePath()
{	
	float const oldPathLength = m_pimpl->m_pathLength;
	m_pimpl->m_pathLength = 0.0f;
	m_pimpl->m_dataMap.clear();

	PointList & pointList = m_pimpl->m_pointList;
	int const size = static_cast<int>(pointList.size());
	TerrainObject const * const terrainObject = TerrainObject::getConstInstance();

	for (int i = 0; i < size; ++i)
	{
		// Snap point to the terrain if necessary.
		getTerrainHeight(terrainObject, pointList[static_cast<size_t>(i)]);
		
		if (i) 
		{
			m_pimpl->m_pathLength += (pointList[static_cast<size_t>(i)] - pointList[static_cast<size_t>(i-1)]).magnitude();
		}
		
		IGNORE_RETURN(m_pimpl->m_dataMap.insert(std::make_pair(m_pimpl->m_pathLength, i)));
	}
	
	// If we update the length of the path, we must update the object distances.
	if (oldPathLength > std::numeric_limits<float>::min())
	{
		float const distanceRatio = m_pimpl->m_pathLength / oldPathLength;
		for (PathObjectDistanceMap::iterator itDist = m_pimpl->m_distanceMap.begin(); itDist != m_pimpl->m_distanceMap.end(); ++itDist)
		{
			itDist->second *= distanceRatio;
		}
	}
	
	m_pimpl->m_pathNodeTimer.setExpireTime(ms_expireTime);


	Vector endPoint_p = m_pimpl->m_endPoint->getPosition_p();
	getTerrainHeight(terrainObject, endPoint_p);
	m_pimpl->m_endPoint->setPosition_p(endPoint_p);
}

// ----------------------------------------------------------------------

void ClientPathObject::optimizePath()
{
	// see ScriptMethodsTerrain::createClientPath && ScriptMethodsTerrain::createClientPathAdvanced 
	// before modifying this code.

	// rls todo: clientside path optimization has been removed because 
	// in some cases the client may not have all the objects loaded and
	// would generate a bogus path. This should be addressed in the future.


	// Add the player height to the points that are not terrain height adjusted.
	PointList & pointList = m_pimpl->m_pointList;
	int const size = static_cast<int>(pointList.size());
	
	for (int i = 0; i < size; ++i)
	{
		Vector & point = pointList[static_cast<size_t>(i)];
		if (point.y != 0.0f) 
		{
			point.y += ms_playerHeight;
		}
	}
}

// ----------------------------------------------------------------------

void ClientPathObject::getTerrainHeight(TerrainObject const * const terrain, Vector & point)
{
	// see ScriptMethodsTerrain::createClientPath && ScriptMethodsTerrain::createClientPathAdvanced 
	// before modifying this code.
	if (terrain && (point.y == 0.0f)) 
	{
		point = rotateTranslate_o2w(point);
		IGNORE_RETURN(terrain->getHeight(point, point.y));
		point = rotateTranslate_w2o(point);
		point += (Vector::unitY * ms_playerHeight);
	}
}

// ----------------------------------------------------------------------

void ClientPathObject::addPathNode(float const distance)
{
	Object * const object = new Object();
	if (!m_pimpl->m_pointList.empty())
	{
		object->setPosition_p(m_pimpl->m_pointList[0]);
	}

	Appearance * const appearance = AppearanceTemplateList::createAppearance(ms_pathAppearanceName.c_str());
	object->setAppearance(appearance);

	RenderWorld::addObjectNotifications(*object);
	addChildObject_p(object);

	IGNORE_RETURN(m_pimpl->m_distanceMap.insert(std::make_pair(object, distance)));
}

// ----------------------------------------------------------------------

void ClientPathObject::updateNodes()
{
	PointList & pointList = m_pimpl->m_pointList;
	int const pointListSize = static_cast<int>(pointList.size());
	int const pointListMaxIndex = pointListSize - 1;

	// set end point.
	if (!pointList.empty()) 
	{
		m_pimpl->m_endPoint->setPosition_p(pointList.back());
	}

	CreatureObject * const playerCreature = Game::getPlayerCreature();
	if ( playerCreature) 
	{
		CellProperty * const playerParentCell = playerCreature->getParentCell() ? playerCreature->getParentCell() : CellProperty::getWorldCellProperty();

		if (playerParentCell != getParentCell())
		{
			setParentCell(playerParentCell);
			updatePath();
		}

		Vector const & playerPosition_w = playerCreature->getPosition_w();
		std::string const & sceneName = Game::getSceneId();
	
		int const numObjects = getNumberOfChildObjects();
		for (int count = 0; count < numObjects; ++count) 
		{
			Object * const object = getChildObject(count);
			if (object) 
			{
				PathObjectDistanceMap::iterator itDist = m_pimpl->m_distanceMap.find(object);
				if (itDist != m_pimpl->m_distanceMap.end()) 
				{
					float const distance = itDist->second;
					if (distance > m_pimpl->m_pathLength) 
					{
						m_pimpl->m_distanceMap.erase(itDist);
						delete object;
						break;
					}
					else
					{
						if (pointListMaxIndex > 0)
						{
							Appearance const * const appearance = object ? object->getAppearance() : NULL;
							if (appearance)
							{
								PathObjectDataMap::iterator const itRangeLower = getRangeIterator(m_pimpl->m_dataMap, distance);
								if (itRangeLower != m_pimpl->m_dataMap.end())
								{
									itDist->second += m_pimpl->m_distanceDelta;
									
									int const startIndex = itRangeLower->second;
									if (startIndex < pointListMaxIndex) 
									{
										Vector const & start = pointList[static_cast<size_t>(startIndex)];
										
										int const endIndex = startIndex + 1;
										Vector const & end = pointList[static_cast<size_t>(endIndex)];
										
										float const range = (end - start).magnitude();
										float const lowerValue = itRangeLower->first;
										float const t = range > 0.0f ? ((distance - lowerValue) / range) : 0.0f;
										
										Vector pointl = linearInterpolate(start, end, t);

										// rls - this could be performed once in the initialization of the path.
										Vector const & point = GroundZoneManager::getRelativePositionFromPlayer(sceneName.c_str(), playerPosition_w, pointl);
										
										Transform transform;
										transform.setPosition_p(point);
										
										Vector const & dir = end - start;
										transform.ypr_l(dir.theta(), dir.phi(), 0.0f);
										
										object->setTransform_o2p(transform);
									}
									else
									{
										// rls - this could be performed once in the initialization of the path.
										Vector const & point = GroundZoneManager::getRelativePositionFromPlayer(sceneName.c_str(), playerPosition_w, pointList[startIndex]);
										object->setPosition_p(point);
									}
								}
							}
						}
					}
				}
			}
		}
	}
}

// ----------------------------------------------------------------------

void ClientPathObject::setAppearance(std::string const & appearanceName)
{
	ms_pathAppearanceName = "appearance/" + appearanceName;	
}

// ----------------------------------------------------------------------

void ClientPathObject::updateSpeedAndHeight()
{
	CreatureObject * player = Game::getPlayerCreature();
	if (player) 
	{
		ms_runSpeed = std::max(ms_minRunSpeed, player->getMaximumSpeed() * ms_playerRunSpeedMultiplier);
		ms_playerHeight = player->getAppearanceSphereRadius() * 0.75f;
	}
}

// ======================================================================
