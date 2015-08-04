// =====================================================================
//
// OverheadMap.cpp
// asommers
//
// copyright 2003, sony online entertainment
//
// =====================================================================

#include "clientGame/FirstClientGame.h"
#include "clientGame/OverheadMap.h"

#include "clientAudio/Audio.h"
#include "clientGame/CellObject.h"
#include "clientGame/ClientWaypointObject.h"
#include "clientGame/ClientWorld.h"
#include "clientGame/CreatureObject.h"
#include "clientGame/Game.h"
#include "clientGame/GroundScene.h"
#include "clientGraphics/DynamicIndexBuffer.h"
#include "clientGraphics/DynamicVertexBuffer.h"
#include "clientGraphics/Graphics.h"
#include "clientGraphics/ShaderTemplateList.h"
#include "clientObject/GameCamera.h"
#include "clientObject/ObjectListCamera.h"
#include "clientUserInterface/CuiGameColorManager.h"
#include "clientUserInterface/CuiPreferences.h"
#include "clientUserInterface/CuiSoundManager.h"
#include "clientUserInterface/CuiSounds.h"
#include "clientUserInterface/CuiTextManager.h"
#include "sharedCollision/BaseExtent.h"
#include "sharedCollision/Floor.h"
#include "sharedDebug/DebugFlags.h"
#include "sharedFoundation/Crc.h"
#include "sharedFoundation/PointerDeleter.h"
#include "sharedGame/PlanetMapManager.h"
#include "sharedMath/AxialBox.h"
#include "sharedMath/IndexedTriangleList.h"
#include "sharedMath/Range.h"
#include "sharedMath/Sphere.h"
#include "sharedMath/VectorArgb.h"
#include "sharedMessageDispatch/Receiver.h"
#include "sharedMessageDispatch/Transceiver.h"
#include "sharedNetworkMessages/MapLocation.h"
#include "sharedObject/Appearance.h"
#include "sharedObject/CellProperty.h"
#include "sharedObject/Object.h"
#include "sharedObject/PortalProperty.h"
#include "sharedObject/PortalPropertyTemplate.h"
#include "swgSharedUtility/Postures.def"

#include <vector>

// =====================================================================
// OverheadMapNamespace
// =====================================================================

namespace OverheadMapNamespace
{
	const int        cms_maximumRadarZoomSetting = 10;
	const float      cms_ooMaximumRadarZoomSetting = RECIP (10.f);
	const float      cms_minimumRadarZoom = 25.f;
	const float      cms_maximumRadarZoom = 750.f;
	const VectorArgb cms_radarColorExteriorTangible (0.33f, 0.f, 1.f, 0.f);
	const VectorArgb cms_radarColorExteriorIntangible (0.33f, 0.5f, 1.f, 0.f);
	const VectorArgb cms_radarColorInterior (0.25f, 1.f, 1.f, 1.f);
	const VectorArgb cms_radarColorCurrentCellInterior (0.50f, 0.8f, 0.8f, 1.f);
	const VectorArgb cms_radarColorEdges (0.5f, 1.f, 1.f, 0.f);
	const VectorArgb cms_radarColorPortals (1.f, 0.f, 0.f, 1.f);
	const VectorArgb cms_radarColorCreature (0.7f, 1.f, 0.f, 0.f);
	const VectorArgb cms_radarColorPlayer  (1.0f, 0.5f, 0.5f, 1.0f);
	const VectorArgb cms_radarColorPlayer2 (1.0f, 1.0f, 1.0f, 0.0f);
	const VectorArgb cms_radarColorClientPath (0.7f, 1.f, 1.f, 0.f);

	PlanetMapManagerClient::CategoryVector ms_categoryList;

	void renderIndexedTriangleList (const Transform& transform, const Vector& scale, const IndexedTriangleList* const indexedTriangleList, const VectorArgb& color);
	void renderLineList (const Transform& transform, const std::vector<Vector>& vertices, const VectorArgb& color);
	void renderLineStrip (const Transform& transform, const std::vector<Vector>& vertices, const VectorArgb& color);
	void renderObjectList (const Camera* const camera, const int objectListIndex, const VectorArgb& color);

	const int MAX_DEPTH_CHECK = 16;
	const float PLAYER_Y_POSITION_TOLERANCE = 15.0f;

	const float MAXIMUM_PLAYER_ZOOM = 1.7f;
}

using namespace OverheadMapNamespace;

// =====================================================================
// OverheadMap::Location
// =====================================================================

class OverheadMap::Location
{
public:

	Location (const Vector& position_w, const Unicode::String& name) :
		m_position_w (position_w),
		m_name (name)
	{
	}

	const Vector& getPosition_w () const
	{
		return m_position_w;
	}

	const Unicode::String& getName () const
	{
		return m_name;
	}

private:

	Vector          m_position_w;
	Unicode::String m_name;
};

// =====================================================================
// STATIC PUBLIC OverheadMap
// =====================================================================

OverheadMap::OverheadMap () :
	UIEventCallback(),
	m_renderMap (false),
	m_radarCamera (0),
	m_radarZoomSetting (cms_maximumRadarZoomSetting / 2),
	m_normalizedRadarZoom (0.75f),
	m_radarZoom ((cms_minimumRadarZoom + cms_maximumRadarZoom) * 0.75f),
	m_creatureIndexedTriangleList (0),
	m_playerIndexedTriangleList (0),
	m_waypointIndexedTriangleList (0),
	m_waypointEntranceIndexedTriangleList (0),
	m_radarCellList (new CellPropertyList),
	m_cachedStartCell(0),
	m_clientPath (new PointList),
	m_locationList (new LocationList),
	m_lastPlayerPosition_w (),
	m_timer (15.f),
	m_screenOverlapLocations(new PointList),
	m_callback(new MessageDispatch::Callback)
{
	m_radarCamera = new ObjectListCamera (1);

	//-- create object used to represent a creature
	{
		const Vector vertices [3] = 
		{
			Vector (0.f, 0.f, 1.f),
			Vector (0.5f, 0.f, -1.f),
			Vector (-0.5f, 0.f, -1.f)
		};

		const int indices [3] =
		{
			0, 1, 2
		};

		m_creatureIndexedTriangleList = new IndexedTriangleList;
		m_creatureIndexedTriangleList->addIndexedTriangleList (vertices, 3, indices, 3);
	}

	//-- create object used to represent a waypoint
	{
		const Vector vertices [3] = 
		{
			Vector ( 0.0f, 0.0f,  2.0f),
			Vector ( 2.0f, 0.0f, -2.0f),
			Vector (-2.0f, 0.0f, -2.0f)
		};

		const int indices [3] =
		{
			0, 1, 2
		};

		m_waypointIndexedTriangleList = new IndexedTriangleList;
		m_waypointIndexedTriangleList->addIndexedTriangleList (vertices, 3, indices, 3);
	}

	//-- create object used to represent a waypoint entrance marker
	{
		const Vector vertices [8] =
		{
			Vector (-1.7f, 0.0f,  1.7f),
			Vector ( 1.7f, 0.0f,  1.7f),
			Vector ( 1.7f, 0.0f, -1.7f),
			Vector (-1.7f, 0.0f, -1.7f),
			Vector (-3.0f, 0.0f,  0.5f),
			Vector ( 3.0f, 0.0f,  0.5f),
			Vector ( 3.0f, 0.0f, -0.5f),
			Vector (-3.0f, 0.0f, -0.5f)
		};

		const int indices [12] = 
		{
			0, 1, 2, 2, 3, 0, 4, 5, 6, 6, 7, 4
		};
	
		m_waypointEntranceIndexedTriangleList = new IndexedTriangleList;
		m_waypointEntranceIndexedTriangleList->addIndexedTriangleList (vertices, 8, indices, 12);
	}

	//-- create object used to represent the player
	{
		const Vector vertices [4] = 
		{
			Vector ( 0.0f, 0.0f,  1.0f),
			Vector ( 0.8f, 0.0f, -1.0f),
			Vector ( 0.0f, 0.0f, -0.5f),
			Vector (-0.8f, 0.0f, -1.0f)
		};

		const int indices [6] =
		{
			0, 1, 2, 2, 3, 0
		};

		m_playerIndexedTriangleList = new IndexedTriangleList;
		m_playerIndexedTriangleList->addIndexedTriangleList (vertices, 4, indices, 6);
	}

}

// ---------------------------------------------------------------------

OverheadMap::~OverheadMap ()
{
	delete m_radarCamera;
	m_radarCamera = 0;

	delete m_creatureIndexedTriangleList;
	m_creatureIndexedTriangleList = 0;

	delete m_playerIndexedTriangleList;
	m_playerIndexedTriangleList = 0;

	delete m_waypointIndexedTriangleList;
	m_waypointIndexedTriangleList = 0;

	delete m_waypointEntranceIndexedTriangleList;
	m_waypointEntranceIndexedTriangleList = 0;

	delete m_radarCellList;

	std::for_each (m_locationList->begin (), m_locationList->end (), PointerDeleter ());
	m_locationList->clear ();
	delete m_locationList;

	delete m_clientPath;

	delete m_screenOverlapLocations;

	delete m_callback;
}

// ---------------------------------------------------------------------

bool OverheadMap::getRenderMap () const
{
	return m_renderMap;
}

// ---------------------------------------------------------------------

void OverheadMap::toggle ()
{
	m_renderMap = !m_renderMap;

	if (m_renderMap)
	{
		m_callback->connect (*this, &OverheadMap::onLocationsReceived, static_cast<PlanetMapManagerClient::Messages::LocationsReceived *> (0));
		PlanetMapManagerClient::requestEntries ();
	}
	else
	{
		m_callback->disconnect (*this, &OverheadMap::onLocationsReceived, static_cast<PlanetMapManagerClient::Messages::LocationsReceived *> (0));
	}
}

// ---------------------------------------------------------------------

void OverheadMap::zoom (const bool in)
{
	//-- don't zoom when invisible
	if (!m_renderMap)
		return;

	const int lastRadarZoomSetting = m_radarZoomSetting;

	if (in)
		m_radarZoomSetting = std::max (m_radarZoomSetting - 1, 0);
	else
		m_radarZoomSetting = std::min (m_radarZoomSetting + 1, cms_maximumRadarZoomSetting);

	if (m_radarZoomSetting != lastRadarZoomSetting)
	{
		if (m_radarZoomSetting < lastRadarZoomSetting)
			CuiSoundManager::play (CuiSounds::item_zoom_in_st);
		else
			CuiSoundManager::play (CuiSounds::item_zoom_out_st);
	}

	m_normalizedRadarZoom = clamp (0.f, static_cast<float> (m_radarZoomSetting) * cms_ooMaximumRadarZoomSetting, 1.f);
	m_radarZoom = linearInterpolate (cms_minimumRadarZoom, cms_maximumRadarZoom, m_normalizedRadarZoom);
}

// ---------------------------------------------------------------------

void OverheadMap::update (float const elapsedTime)
{
	if (m_timer.updateZero (elapsedTime))
	{
		refreshLocationList ();

		if (m_renderMap)
			PlanetMapManagerClient::requestEntries ();
	}
}

// ---------------------------------------------------------------------

void OverheadMap::setClientPath (PointList const & clientPath)
{
	*m_clientPath = clientPath;
}

// ---------------------------------------------------------------------

void OverheadMap::clearClientPath ()
{
	m_clientPath->clear ();
}

// ---------------------------------------------------------------------

void OverheadMap::avoidScreenOverlap(Vector &location) const
{
	NOT_NULL(m_screenOverlapLocations);
	for (int i = 0; i < static_cast<int>(m_screenOverlapLocations->size()); i++)
	{
		if ((abs(location.x - (*m_screenOverlapLocations)[i].x) < 25.0f) && (abs(location.y - (*m_screenOverlapLocations)[i].y + 3.0f) < 5.0f))
		{
			location.y += 10.0f;
			i = -1;
		}
	}

	m_screenOverlapLocations->push_back(location);
}

// ---------------------------------------------------------------------

void OverheadMap::render () const
{
	if (!m_renderMap)
		return;

	const Camera* const camera = safe_cast<const GroundScene*> (Game::getConstScene ())->getCamera (GroundScene::CI_freeChase);
	const Object* const player = Game::getPlayer ();
	const Vector playerPosition_w = player->getPosition_w ();
	const Vector playerPosition2d_w (playerPosition_w.x, 0.f, playerPosition_w.z);

	if (playerPosition2d_w.magnitudeBetweenSquared (m_lastPlayerPosition_w) > sqr (16.f))
	{
		m_lastPlayerPosition_w = playerPosition2d_w;
		refreshLocationList ();
	}
	
	m_radarCamera->resetRotate_o2p ();

	if (Game::isSpace())
	{
		if (!player->isInWorldCell())
		{
			CellProperty const * const cellProperty = player->getParentCell();
			if (cellProperty)
			{
				PortalProperty const * const portalProperty = cellProperty->getPortalProperty();
				if (portalProperty)
				{
					Object const & owner = portalProperty->getOwner();
					m_radarCamera->setTransform_o2w(owner.getTransform_o2w());

					if (CuiPreferences::getRotateMap())
						m_radarCamera->yaw_o(owner.rotate_w2o(camera->getObjectFrameK_w()).theta());

					m_radarCamera->setPosition_w(playerPosition_w);
				}
			}
		}
	}
	else
	{
		if (CuiPreferences::getRotateMap())
			m_radarCamera->yaw_o (camera->getObjectFrameK_w ().theta ());

		m_radarCamera->setPosition_p (playerPosition2d_w);
	}

	m_radarCamera->pitch_o (PI_OVER_2);
	m_radarCamera->move_o (Vector::negativeUnitZ * m_radarZoom);
	m_radarCamera->applyState ();
	
	if(CuiPreferences::getOverheadMapShowBuildings())
	{
		if (player->isInWorldCell ())
		{
			//-- render exterior objects
			renderObjectList (m_radarCamera, WOL_Tangible, VectorArgb(cms_radarColorExteriorTangible.a * CuiPreferences::getOverheadMapOpacity(), cms_radarColorExteriorTangible.r, cms_radarColorExteriorTangible.g, cms_radarColorExteriorTangible.b));
			renderObjectList (m_radarCamera, WOL_TangibleNotTargetable, VectorArgb(cms_radarColorExteriorIntangible.a * CuiPreferences::getOverheadMapOpacity(), cms_radarColorExteriorIntangible.r, cms_radarColorExteriorIntangible.g, cms_radarColorExteriorIntangible.b));

			//-- render client path
			if (!m_clientPath->empty ())
				renderLineStrip (Transform::identity, *m_clientPath, VectorArgb(cms_radarColorClientPath.a * CuiPreferences::getOverheadMapOpacity(), cms_radarColorClientPath.r, cms_radarColorClientPath.g, cms_radarColorClientPath.b));

			//-- render the names
			{
				uint i;
				for (i = 0; i < m_locationList->size (); ++i)
				{
					const Location& location = *(*m_locationList) [i];
					Vector position_w = location.getPosition_w ();
					position_w.y = 0.0f;

					CuiTextManager::TextEnqueueInfo info;
					if (CuiPreferences::getOverheadMapShowLabels() && m_radarCamera->projectInWorldSpace (position_w, &info.screenVect.x, &info.screenVect.y, &info.screenVect.z))
					{
						avoidScreenOverlap(info.screenVect);
						info.backgroundOpacity = 0.f;
						info.textSize          = 0.5f;
						info.textColor         = UIColor::white;
						CuiTextManager::enqueueText (location.getName (), info);
					}
				}

				m_screenOverlapLocations->clear();
			}
		}
		else
		{

			const CellProperty* const startCell = player->getParentCell ();
			if(!startCell)
			{
				m_cachedStartCell = NULL;
				m_radarCellList->clear();
			}
			else if(m_cachedStartCell != startCell)
			{
				
				m_radarCellList->clear ();
				m_radarCellList->push_back (startCell);

				const PortalProperty* const portalProperty = startCell->getPortalProperty ();
				if (portalProperty)
				{
					int i;				
					CellPropertyList lastSetAdded;
					for (i = 1; i < portalProperty->getNumberOfCells (); ++i)
					{
						const CellProperty* const cellProperty = portalProperty->getCell (i);
						if (cellProperty && CellProperty::areAdjacent(startCell, cellProperty))
						{
							m_radarCellList->push_back (cellProperty);
							lastSetAdded.push_back(cellProperty);
						}
					}
					uint oldListSize = 0;
					bool done = false;
					for(uint depth = 0; !done && (depth < MAX_DEPTH_CHECK); depth++)
					{				
						if(oldListSize == m_radarCellList->size())
						{
							done = true;
							continue;
						}
						oldListSize = m_radarCellList->size();
						uint j;					
						CellPropertyList listToAdd;
						for(i = 1; i < portalProperty->getNumberOfCells(); ++i)
						{
							const CellProperty* const cellProperty = portalProperty->getCell (i);
							if (cellProperty)
							{
								for(j = 0; j < lastSetAdded.size(); ++j)
								{
									if((lastSetAdded[j] != cellProperty) && CellProperty::areAdjacent(lastSetAdded[j], cellProperty))
									{
										listToAdd.push_back(cellProperty);
									}
								}
							}					
						}		
						lastSetAdded.clear();
						for(j = 0; j < listToAdd.size(); ++j)
						{
							if(std::find(m_radarCellList->begin(), m_radarCellList->end(), listToAdd[j]) == m_radarCellList->end())
							{
								m_radarCellList->push_back(listToAdd[j]);
								lastSetAdded.push_back(listToAdd[j]);
							}
						}
					}
				}

				//-- check all cells against the player's y position.  if the player's y position +- 5m doesn't intersect with the cell's boundary then take it off the list
				{
					const Vector playerPosition_p = player->getPosition_p();
					Range p(playerPosition_p.y - PLAYER_Y_POSITION_TOLERANCE, playerPosition_p.y + PLAYER_Y_POSITION_TOLERANCE);
					uint i;
					uint cellListSize = m_radarCellList->size();
					for (i = 0; i < cellListSize;)
					{
						const CellProperty* const cellProperty = (*m_radarCellList) [i];
						if (cellProperty && cellProperty->getFloor())
						{
							const BaseExtent *be = cellProperty->getFloor()->getExtent_p();
							if(!be)
								DEBUG_WARNING(true, ("cell has no floor extent"));
							else
							{						
								AxialBox ab = be->getBoundingBox();
								Range r = ab.getRangeY();
								
								if(r.isBelow(p) || r.isAbove(p))
								{
									m_radarCellList->erase(m_radarCellList->begin() + i);
									cellListSize--;
									continue;
								}							
							}
						}
						++i;
					}
				}			
			} // end if(m_cachedStartCell != startCell)


			//-- render all cells
			{
				uint i;
				for (i = 0; i < m_radarCellList->size (); ++i)
				{
					const CellProperty* const cellProperty = (*m_radarCellList) [i];
					if (cellProperty)
					{
						const CellObject* const cellObject = safe_cast<const CellObject*> (&cellProperty->getOwner ());
						if (cellObject)
						{
							if (cellObject->getRadarShape ())
							{
								if(cellProperty == startCell)
									renderIndexedTriangleList (cellObject->getTransform_o2w (), cellObject->getScale (), cellObject->getRadarShape (), VectorArgb(cms_radarColorCurrentCellInterior.a * CuiPreferences::getOverheadMapOpacity(), cms_radarColorCurrentCellInterior.r, cms_radarColorCurrentCellInterior.g, cms_radarColorCurrentCellInterior.b));	
								else
									renderIndexedTriangleList (cellObject->getTransform_o2w (), cellObject->getScale (), cellObject->getRadarShape (), VectorArgb(cms_radarColorInterior.a * CuiPreferences::getOverheadMapOpacity(), cms_radarColorInterior.r, cms_radarColorInterior.g, cms_radarColorInterior.b));
							}
							if (cellObject->getRadarEdges ())
								renderLineList (cellObject->getTransform_o2w (), *cellObject->getRadarEdges (), VectorArgb(cms_radarColorEdges.a * CuiPreferences::getOverheadMapOpacity(), cms_radarColorEdges.r, cms_radarColorEdges.g, cms_radarColorEdges.b));
							
							//-- render the names
							{
								Unicode::String const cellDisplayName = cellObject->getCellLabel();
								if(!cellDisplayName.empty())
								{
									Transform cellTransform = cellObject->getTransform_o2w();

									if (cellProperty->getFloor())
									{										
										const BaseExtent *be = cellProperty->getFloor()->getExtent_p();
										if(be)
										{
											Vector const cellFloorCenter = be->getCenter();
											cellTransform.move_l(cellFloorCenter);
										}
									}

									Vector const labelOffset = cellObject->getLabelOffset();
									cellTransform.move_l(labelOffset);

									Vector cellCenter = cellTransform.getPosition_p();
									cellCenter.y = 0.0f;

									CuiTextManager::TextEnqueueInfo info;
									if (CuiPreferences::getOverheadMapShowLabels() && m_radarCamera->projectInWorldSpace (cellCenter, &info.screenVect.x, &info.screenVect.y, &info.screenVect.z))
									{
										info.backgroundOpacity = 0.f;
										info.textSize          = 0.5f;
										info.textColor         = UIColor::white;
										CuiTextManager::enqueueText (cellDisplayName, info);
									}
								}					
							}

						}
					}
				}
			}

			//-- render the portal edges
			/*if (portalProperty)
			{
				const PortalPropertyTemplate& portalPropertyTemplate = portalProperty->getPortalPropertyTemplate ();
				renderLineList (portalProperty->getOwner ().getTransform_o2w (), *portalPropertyTemplate.getRadarPortalGeometry (), cms_radarColorPortals);
			}*/
		}
	}
	float const zoom_t         = 1.f + clamp (0.f, ((m_radarZoom - cms_minimumRadarZoom) / (cms_maximumRadarZoom - cms_minimumRadarZoom)), 1.f);
	const Vector & zoom_vector = Vector::xyz111 * zoom_t;

	//-- render the active waypoints
	if(CuiPreferences::getOverheadMapShowWaypoints())
	{
		const float elapsedTime = Game::getElapsedTime ();
		const float fraction    = static_cast<float>(fmod (elapsedTime, 1.0f));
		const float opacity     = 0.25f + 1.5f * ((fraction < 0.5f) ? fraction : (1.0f - fraction)); // flash the opacity over time, between 0.25 and 1.0

		Transform waypoint_transform;

		PlanetMapManagerClient::WaypointVector wv;
		PlanetMapManagerClient::getPlayerWaypoints (wv);

		for (PlanetMapManagerClient::WaypointVector::const_iterator it = wv.begin (); it != wv.end (); ++it)
		{
			const ClientWaypointObject * const waypoint = it->getPointer();
			if (waypoint && waypoint->isWaypointActive ())
			{
				const ClientWaypointObject * const waypointWorldObject = waypoint->getWorldObject ();
				if (waypointWorldObject)
				{
					//-- if the player is not in the world cell, only render the creatures in the same portal property
					bool renderObject = false;

					if (player->isInWorldCell ())
						renderObject = true;
					else
					{
						const NetworkId & objectCellId = waypoint->getCell();
						const uint32 cellNameCrc = static_cast<uint32>(objectCellId.getValue());
						if(cellNameCrc == 0)
							renderObject = true;   //Render all waypoints that are in the world cell (erroneously or not)
						else
						{						
							for(CellPropertyList::iterator rcli = m_radarCellList->begin(); rcli != m_radarCellList->end(); ++rcli)
							{							
								const CellProperty * const curProp = *rcli;	
								const uint32 curPropCrc = curProp->getCellNameCrc();
								if(curPropCrc == cellNameCrc)
									renderObject = true;
							}
						}
					}

					if (renderObject)
					{
						VectorArgb         color = waypoint->getColorArgb ();
						color.a                  = opacity * CuiPreferences::getOverheadMapOpacity();
						waypoint_transform       = waypointWorldObject->getTransform_o2w ();

						if (waypoint->isEntrance())
							renderIndexedTriangleList (waypoint_transform, zoom_vector, m_waypointEntranceIndexedTriangleList, color);
						else
							renderIndexedTriangleList (waypoint_transform, zoom_vector, m_waypointIndexedTriangleList, color);
						
						Vector waypointPosition = waypointWorldObject->getPosition_w ();
						waypointPosition.y = 0.0f;

						CuiTextManager::TextEnqueueInfo info;
						if (CuiPreferences::getOverheadMapShowLabels() && m_radarCamera->projectInWorldSpace (waypointPosition, &info.screenVect.x, &info.screenVect.y, &info.screenVect.z))
						{
							info.backgroundOpacity = 0.f;
							info.textSize          = 0.5f;
							info.textColor         = UIColor::white;
							CuiTextManager::enqueueText (waypoint->getLocalizedName (), info);
						}
					}
				}
			}
		}
	}

	//-- render the characters
	if(CuiPreferences::getOverheadMapShowCreatures() || CuiPreferences::getOverheadMapShowPlayer())
	{
		int i;
		for (i = 0; i < ClientWorld::getNumberOfObjects (WOL_Tangible); ++i)
		{
			const Object* const object = ClientWorld::getConstObject (WOL_Tangible, i);
			const ClientObject* const clientObject = object ? object->asClientObject () : 0;
			const CreatureObject* const creatureObject = clientObject ? clientObject->asCreatureObject () : 0;

			if(!creatureObject)
				continue;

			if (creatureObject->getVisualPosture() == Postures::Dead)
				continue;

			// should creature be shown on the overhead map?
			// always show the main character on the overhead map
			if (creatureObject != player)
			{
				if (!creatureObject->getVisibleOnMapAndRadar()
					|| (!creatureObject->getCoverVisibility() && !creatureObject->isPassiveRevealPlayerCharacter(Game::getPlayerNetworkId()))
				)
				{	
					continue;
				}
			}

			//-- if the player is not in the world cell, only render the creatures in the same portal property
			bool renderObject = false;

			if (player->isInWorldCell ())
				renderObject = (object->getParentCell() == CellProperty::getWorldCellProperty());
			else
			{
				const CellProperty* const objectCell = object->getParentCell ();
				if(std::find(m_radarCellList->begin(), m_radarCellList->end(), objectCell) != m_radarCellList->end())
					renderObject = true;					
			}

			if (renderObject)
			{
				renderCreatureOnMap (*creatureObject, zoom_vector);
			}
		}
	}
}

//----------------------------------------------------------------------

void OverheadMap::onLocationsReceived (const PlanetMapManagerClient::Messages::LocationsReceived::Payload & msg)
{
	// did we get anything new?
	if (msg.first || msg.second.first || msg.second.second)
		refreshLocationList ();
}

//----------------------------------------------------------------------

void OverheadMap::renderCreatureOnMap (const CreatureObject & creatureObject, const Vector & zoom_vector) const
{
	const Transform & transform_o2w = creatureObject.getTransform_o2w ();	
	const CreatureObject * const rider = creatureObject.getRiderDriverCreature ();

	if (&creatureObject != static_cast<const Object *>(Game::getPlayer ()) && CuiPreferences::getOverheadMapShowCreatures())
	{
		UIColor const uiColor = CuiGameColorManager::findColorForObject (creatureObject, true, true);
		PackedArgb const color0 (192, uiColor.r, uiColor.g, uiColor.b);
		VectorArgb const color1 (color0);
		VectorArgb const color2 (color1.a * CuiPreferences::getOverheadMapOpacity(), color1.r, color1.g, color1.b);

		if (rider)
			renderIndexedTriangleList (transform_o2w, zoom_vector * 2.0f, m_creatureIndexedTriangleList, color2);
		else
			renderIndexedTriangleList (transform_o2w, zoom_vector, m_creatureIndexedTriangleList, color2);
	}
	else if( &creatureObject == static_cast<const Object *>(Game::getPlayer ()) && CuiPreferences::getOverheadMapShowPlayer())
	{
		//-- double-render the player with a white border to make it more obvious
		float zoomFactor = 1.0f;
		if(zoom_vector.x < MAXIMUM_PLAYER_ZOOM)
			zoomFactor = zoom_vector.x / MAXIMUM_PLAYER_ZOOM;

		renderIndexedTriangleList (transform_o2w, zoom_vector * zoomFactor * 3.0f, m_playerIndexedTriangleList, VectorArgb(cms_radarColorPlayer2.a * CuiPreferences::getOverheadMapOpacity(), cms_radarColorPlayer2.r, cms_radarColorPlayer2.g, cms_radarColorPlayer2.b));
		renderIndexedTriangleList (transform_o2w, zoom_vector * zoomFactor * 1.65f, m_playerIndexedTriangleList, VectorArgb(cms_radarColorPlayer.a * CuiPreferences::getOverheadMapOpacity(), cms_radarColorPlayer.r, cms_radarColorPlayer.g, cms_radarColorPlayer.b));				
	}

	if (rider)
		renderCreatureOnMap (*rider, zoom_vector);
}

// =====================================================================
// PRIVATE OverheadMap
// =====================================================================

void OverheadMap::refreshLocationList () const
{
	std::for_each (m_locationList->begin (), m_locationList->end (), PointerDeleter ());
	m_locationList->clear ();

	ms_categoryList.clear ();
	PlanetMapManagerClient::getCategories (ms_categoryList);

	static const Unicode::String nameSeparator = Unicode::narrowToWide (" - ");

	uint i;
	for (i = 0; i < ms_categoryList.size (); ++i)
	{
		const PlanetMapManagerClient::MapLocationVector& mapLocations = PlanetMapManagerClient::getMapLocations (ms_categoryList [i]);
		if (mapLocations.size ())
		{
			uint j;
			for (j = 0; j < mapLocations.size (); ++j)
			{
				const Vector v (mapLocations [j].m_location.x, 0.f, mapLocations [j].m_location.y);
				const Unicode::String & nameStr = PlanetMapManagerClient::findCategoryString (mapLocations [j].m_category) + nameSeparator + StringId::decodeString (mapLocations [j].m_locationName);
				Location * const loc = new Location (v, nameStr);
				m_locationList->push_back (loc);
			}
		}
	}
}

// =====================================================================
// OverheadMapNamespace
// =====================================================================

void OverheadMapNamespace::renderIndexedTriangleList (const Transform& transform, const Vector& scale, const IndexedTriangleList* const indexedTriangleList, const VectorArgb& color)
{
	NOT_NULL (indexedTriangleList);

	Graphics::setStaticShader (ShaderTemplateList::get3dVertexColorAStaticShader ());

	Transform t (transform);

	if (!Game::isSpace())
	{
		Vector const position = transform.getPosition_p ();
		t.setPosition_p (Vector (position.x, 0.f, position.z));
	}

	Graphics::setObjectToWorldTransformAndScale (t, scale);

	{
		const std::vector<Vector> vertices = indexedTriangleList->getVertices ();
		
		VertexBufferFormat format;
		format.setPosition ();
		format.setColor0 ();
		DynamicVertexBuffer vertexBuffer (format);

		vertexBuffer.lock (vertices.size ());

			VertexBufferWriteIterator v = vertexBuffer.begin ();
			const uint n = vertices.size ();
			uint i;
			for (i = 0; i < n; ++i, ++v)
			{
				Vector vertex = vertices [i];
				vertex.y = 0.f;
				v.setPosition (vertex);
				v.setColor0 (color);
			}

		vertexBuffer.unlock ();

		Graphics::setVertexBuffer (vertexBuffer);
	}

	{
		const std::vector<int> indices = indexedTriangleList->getIndices ();

		DynamicIndexBuffer indexBuffer;

		indexBuffer.lock (indices.size ());

			Index *ii = indexBuffer.begin ();
			const uint n = indices.size ();
			uint i;
			for (i = 0; i < n; ++i, ++ii)
				*ii = static_cast<Index> (indices [i]);

		indexBuffer.unlock ();

		Graphics::setIndexBuffer (indexBuffer);
	}

	Graphics::drawIndexedTriangleList ();
}

// ---------------------------------------------------------------------

void OverheadMapNamespace::renderLineList (const Transform& transform, const std::vector<Vector>& vertices, const VectorArgb& color)
{
	Graphics::setStaticShader (ShaderTemplateList::get3dVertexColorAStaticShader ());

	Transform t (transform);

	if (!Game::isSpace())
	{
		Vector const position = transform.getPosition_p ();
		t.setPosition_p (Vector (position.x, 0.f, position.z));
	}

	Graphics::setObjectToWorldTransformAndScale (t, Vector::xyz111);

	{
		VertexBufferFormat format;
		format.setPosition ();
		format.setColor0 ();
		DynamicVertexBuffer vertexBuffer (format);

		vertexBuffer.lock (vertices.size ());

			VertexBufferWriteIterator v = vertexBuffer.begin ();
			const uint n = vertices.size ();
			uint i;
			for (i = 0; i < n; ++i, ++v)
			{
				Vector vertex = vertices [i];
				vertex.y = 0.f;
				v.setPosition (vertex);
				v.setColor0 (color);
			}

		vertexBuffer.unlock ();

		Graphics::setVertexBuffer (vertexBuffer);
	}

	Graphics::drawLineList ();
}

// ---------------------------------------------------------------------

void OverheadMapNamespace::renderLineStrip (const Transform& transform, const std::vector<Vector>& vertices, const VectorArgb& color)
{
	Graphics::setStaticShader (ShaderTemplateList::get3dVertexColorAStaticShader ());

	Transform t (transform);

	if (!Game::isSpace())
	{
		Vector const position = transform.getPosition_p ();
		t.setPosition_p (Vector (position.x, 0.f, position.z));
	}

	Graphics::setObjectToWorldTransformAndScale (t, Vector::xyz111);

	{
		VertexBufferFormat format;
		format.setPosition ();
		format.setColor0 ();
		DynamicVertexBuffer vertexBuffer (format);

		vertexBuffer.lock (vertices.size ());

			VertexBufferWriteIterator v = vertexBuffer.begin ();
			const uint n = vertices.size ();
			uint i;
			for (i = 0; i < n; ++i, ++v)
			{
				Vector vertex = vertices [i];
				vertex.y = 0.f;
				v.setPosition (vertex);
				v.setColor0 (color);
			}

		vertexBuffer.unlock ();

		Graphics::setVertexBuffer (vertexBuffer);
	}

	Graphics::drawLineStrip ();
}

// ---------------------------------------------------------------------

void OverheadMapNamespace::renderObjectList (const Camera* const camera, const int objectListIndex, const VectorArgb& color)
{
	int i;
	for (i = 0; i < ClientWorld::getNumberOfObjects (objectListIndex); ++i)
	{
		const Object* const object = ClientWorld::getConstObject (objectListIndex, i);
		if (object)
		{
			const Appearance* const appearance = object->getAppearance ();
			if (appearance)
			{
				const IndexedTriangleList* const indexedTriangleList = appearance->getRadarShape ();
				if (indexedTriangleList)
				{
					const Sphere& sphere = appearance->getSphere ();
					if (camera->testVisibility_w (object->rotateTranslate_o2w (sphere)))
						renderIndexedTriangleList (object->getTransform_o2w (), object->getScale (), indexedTriangleList, color);

					//-- does the object have a portal property?
					const PortalProperty* const portalProperty = object->getPortalProperty ();
					if (portalProperty)
					{
						const PortalPropertyTemplate& portalPropertyTemplate = portalProperty->getPortalPropertyTemplate ();
						renderLineList (portalProperty->getOwner ().getTransform_o2w (), *portalPropertyTemplate.getRadarPortalGeometry (), VectorArgb(cms_radarColorPortals.a * CuiPreferences::getOverheadMapOpacity(), cms_radarColorPortals.r, cms_radarColorPortals.g, cms_radarColorPortals.b));
					}
				}
			}
		}
	}
}

// =====================================================================

