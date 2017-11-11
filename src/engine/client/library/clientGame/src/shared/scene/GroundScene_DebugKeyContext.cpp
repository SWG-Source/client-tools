// =====================================================================
//
// GroundScene_DebugKeyContext.cpp
// asommers
//
// copyright 2003, sony online entertainment
//
// =====================================================================

#include "clientGame/FirstClientGame.h"
#include "clientGame/GroundScene.h"

#include "clientGame/BuildingObject.h"
#include "clientGame/ClientPathObject.h"
#include "clientGame/ClientWeaponObjectTemplate.h"
#include "clientGame/ClientWorld.h"
#include "clientGame/CreatureController.h"
#include "clientGame/CreatureObject.h"
#include "clientGame/EnvironmentMapGenerator.h"
#include "clientGame/FreeChaseCamera.h"
#include "clientGame/Game.h"
#include "clientGame/OverheadMap.h"
#include "clientGame/Projectile.h"
#include "clientGame/TangibleObject.h"
#include "clientGame/WanderController.h"
#include "clientGraphics/Graphics.h"
#include "clientGraphics/Light.h"
#include "clientGraphics/RenderWorld.h"
#include "clientObject/GameCamera.h"
#include "clientObject/HardpointObject.h"
#include "clientObject/RibbonAppearance.h"
#include "clientObject/RibbonTrailAppearance.h"
#include "clientObject/TrailAppearance.h"
#include "clientParticle/ParticleEffectAppearance.h"
#include "clientSkeletalAnimation/SkeletalAppearance2.h"
#include "clientSkeletalAnimation/Skeleton.h"
#include "clientTerrain/GroundEnvironment.h"
#include "sharedCollision/CollisionInfo.h"
#include "sharedCollision/Floor.h"
#include "sharedCollision/FloorLocator.h"
#include "sharedDebug/PerformanceTimer.h"
#include "sharedFoundation/ConstCharCrcLowerString.h"
#include "sharedMath/Ray3d.h"
#include "sharedMath/VectorArgb.h"
#include "sharedMathArchive/VectorArchive.h"
#include "sharedObject/AppearanceTemplate.h"
#include "sharedObject/AppearanceTemplateList.h"
#include "sharedObject/CellProperty.h"
#include "sharedObject/LotManager.h"
#include "sharedObject/ObjectTemplateList.h"
#include "sharedObject/PortalProperty.h"
#include "sharedRandom/Random.h"
#include "sharedTerrain/TerrainObject.h"

#include <vector>

// =====================================================================

namespace GroundSceneNamespace
{
	int  ms_debugWeatherIndex = 0;
}

using namespace GroundSceneNamespace;

// =====================================================================

void GroundScene::setDebugKeyContext (int debugKeyContext, const std::string& debugKeySubContext)
{
	m_debugKeyContext    = debugKeyContext;
	m_debugKeySubContext = debugKeySubContext;

	if (m_debugKeyContextWeaponObjectTemplate)
		m_debugKeyContextWeaponObjectTemplate->releaseReference ();

	switch (m_debugKeyContext)
	{
	case 1:
	case 2:
		m_debugKeyContextWeaponObjectTemplate = static_cast<const ClientWeaponObjectTemplate*> (ObjectTemplateList::fetch (m_debugKeySubContext));
		break;
	}
}

// ---------------------------------------------------------------------

void GroundScene::getDebugKeyContextHelp (std::string& result) const
{
	UNREF (result);

#ifdef _DEBUG
	result = 
		"1 <shared weapon template>  CTRL-V  fire tracking projectile from camera\n" \
		"2 <shared weapon template>  CTRL-V  fire non-tracking projectile from camera\n" \
		"3 0  CTRL-V    create 50 boxes that roam the terrain\n" \
		"4 0  CTRL-V/B  V-decrease [B-increase] weather index\n" \
		"5 0  CTRL-V    test the fading screen flash\n" \
		"6 0  CTRL-V/B  V-decrease [B-increase] time of day time\n" \
		"7 0  CTRL-V/B  V-attach [B-remove one] random RibbonTrailAppearance to player\n" \
		"8 0  CTRL-V    test RibbonAppearance\n" \
		"9 <appearance template>  CTRL-V  test attached appearance template\n" \
		"11 0  CTRL-V  spawn 50 random run-time rules\n" \
		"12 0  CTRL-V  toggle on/off for an object\n" \
		"13 0  CTRL-V  add a run-time rule to the player\n" \
		"14 0  CTRL-V  test getHeightForceChunkCreation\n" \
		"15 <appearance template>  CTRL-V/B  V-attach [B-remove one] random appearance to random hardpoint\n" \
		"16 0  CTRL-V/B  V-add [B-remove] test client path\n" \
		"17 0  CTRL-V  test getGoodLocation\n" \
		"18 0  CTRL-V  generate 6 textures for environment map\n" \
		"19 0  CTRL-V  test isValidInteriorLocation\n";
#endif
}

// =====================================================================

void GroundScene::handleDebugKeyContextKey1 () 
{
#ifdef _DEBUG
	switch (m_debugKeyContext)
	{
	case 1:
		{
			//-- test non-tracking projectiles
			if (m_debugKeyContextWeaponObjectTemplate)
			{
				Projectile* const projectile = m_debugKeyContextWeaponObjectTemplate->createProjectile (false, false);
				if (projectile)
				{
					const Camera* const camera = getCurrentCamera();
					const Vector start_w = camera->getPosition_w() - camera->getObjectFrameI_w() * 0.25f;
					const Vector end_w   = getPlayer()->getAppearanceSphereCenter_w();
					projectile->setExpirationTime(10.f);
					const CellProperty* const cellProperty = camera->getParentCell();
					projectile->setParentCell(const_cast<CellProperty*>(cellProperty));
					CellProperty::setPortalTransitionsEnabled(false);
						projectile->setPosition_p(cellProperty->getOwner().rotateTranslate_w2o(start_w));
					CellProperty::setPortalTransitionsEnabled(true);
					projectile->lookAt_o(projectile->rotateTranslate_w2o(end_w));
					projectile->setSpeed(20.f);
					projectile->setTarget(getPlayer());
					projectile->addToWorld();

					Object* const trail = new Object ();
					trail->addNotification (ClientWorld::getIntangibleNotification ());
					RenderWorld::addObjectNotifications (*trail);
					trail->setParentCell (const_cast<CellProperty*> (projectile->getParentCell ()));
					trail->setTransform_o2p (projectile->getTransform_o2p ());
					trail->setAppearance (new TrailAppearance (projectile, 0.05f, VectorArgb::solidRed));
					trail->addToWorld ();
				}
			}
		}
		break;

	case 2:
		{
			//-- test tracking projectiles
			if (m_debugKeyContextWeaponObjectTemplate)
			{
				Projectile* const projectile = m_debugKeyContextWeaponObjectTemplate->createProjectile (true, false);
				if (projectile)
				{
					const Camera* const camera = getCurrentCamera ();
					const Vector start_w = camera->getPosition_w () - camera->getObjectFrameI_w () * 0.25f;
					const Vector end_w   = start_w + camera->getObjectFrameK_w () * 512.f;
					projectile->setExpirationTime(10.f);
					const CellProperty* const cellProperty = camera->getParentCell ();
					projectile->setParentCell (const_cast<CellProperty*> (cellProperty));
					CellProperty::setPortalTransitionsEnabled (false);
						projectile->setPosition_p (cellProperty->getOwner ().rotateTranslate_w2o (start_w));
						projectile->lookAt_o (projectile->rotateTranslate_w2o (end_w));
					CellProperty::setPortalTransitionsEnabled (true);
					projectile->setSpeed (20.f);

					Object* target = 0;
					bool isCombatTarget = false;
					safe_cast<const CreatureController*>(getPlayer()->getController())->getAnimationTarget(target, isCombatTarget);
					projectile->setTarget (target);

					projectile->addToWorld ();

					Object* const trail = new Object ();
					trail->addNotification (ClientWorld::getIntangibleNotification ());
					RenderWorld::addObjectNotifications (*trail);
					trail->setParentCell (const_cast<CellProperty*> (projectile->getParentCell ()));
					trail->setTransform_o2p (projectile->getTransform_o2p ());
					trail->setAppearance (new TrailAppearance (projectile, 0.05f, VectorArgb::solidRed));
					trail->addToWorld ();
				}
			}
		}
		break;

	case 3:
		{
			//-- test terrain generation
			int i;
			for (i = 0; i < 100; ++i)
			{
				Object* const object = new Object ();
				object->setAppearance (AppearanceTemplateList::createAppearance ("appearance/defaultappearance.apt"));
				object->addNotification (ClientWorld::getIntangibleNotification ());
				object->setParentCell (getPlayer ()->getParentCell ());
				CellProperty::setPortalTransitionsEnabled (false);
				object->setPosition_p (getPlayer ()->getPosition_p ());
				object->yaw_o (Random::randomReal (PI_TIMES_2));
				object->move_o (Vector::unitZ * Random::randomReal (1024.f));
				CellProperty::setPortalTransitionsEnabled (true);
				object->setController (new WanderController (object));
				RenderWorld::addObjectNotifications (*object);
				CellProperty::addPortalCrossingNotification(*object);
				object->addToWorld ();
				TerrainObject::getInstance ()->addReferenceObject (object);

				if (m_debugKeySubContext == "1")
				{
					const VectorArgb color (1.f, Random::randomReal (0.5f, 1.f), Random::randomReal (0.5f, 1.f), Random::randomReal (0.5f, 1.f));
					Light* const light = new Light (Light::T_point, color);
					light->setRange (8.f);
					light->setConstantAttenuation (0.f);
					light->setLinearAttenuation (1.f);
					light->setQuadraticAttenuation (0.f);
					light->setPosition_p (Vector::unitY * 1.25f);
					object->addChildObject_o (light);
				}
			}
		}
		break;

	case 4:
		{
			//-- test weather
			if (--ms_debugWeatherIndex < 0)
				ms_debugWeatherIndex = 0;

			GroundEnvironment::getInstance().setWeatherIndex(ms_debugWeatherIndex);
		}
		break;

	case 5:
		{
			//-- test screen flash
			static int color = 0;
			++color;
			if (color > 2)
				color = 0;

			switch (color)
			{
			case 0:  getCurrentCamera ()->flash (VectorArgb::solidRed, 0.5f);    break;
			case 1:  getCurrentCamera ()->flash (VectorArgb::solidGreen, 0.5f);  break;
			case 2:  getCurrentCamera ()->flash (VectorArgb::solidBlue, 0.5f);   break;
			default: break;
			}
		}
		break;

	case 6:
		{
			//-- decrease time of day
			if (TerrainObject::getInstance ())
			{
				float currentTime = TerrainObject::getInstance ()->getTime ();
				currentTime -= 0.015f;

				if (currentTime < 0.f)
					currentTime += 1.f;
				
				TerrainObject::getInstance ()->setTime (currentTime, true);
			}
		}
		break;

	case 7:
		{
			//-- attach ribbon trail appearance to player
			Object* const object = getPlayer ();
			if (object)
			{
				const SkeletalAppearance2* const appearance = dynamic_cast<const SkeletalAppearance2*> (object->getAppearance ());
				if (appearance)
				{
					const Skeleton& skeleton = appearance->getSkeleton (appearance->getSkeletonLodCount () - 1);
					const CrcString& transformName = skeleton.getTransformName (Random::random (0, skeleton.getTransformCount () - 1));

					Object* const childObject = new HardpointObject (transformName);
					RenderWorld::addObjectNotifications (*childObject);
					childObject->setAppearance (new RibbonTrailAppearance (childObject, 0.025f, VectorArgb::solidWhite));
					object->addChildObject_o (childObject);
				}
			}
		}
		break;

	case 8:
		{
			const Object* const player = getPlayer ();

			std::vector<Vector> pointList;

			Transform t;
			int i;
			for (i = 0; i < 10; ++i)
			{
				t.yaw_l (PI_TIMES_2 / 10.f);
				pointList.push_back (t.rotateTranslate_l2p (Vector::unitY * Random::randomReal (0.5f, 0.75f) + Vector::unitZ * 5.f));
			}

			Object* const object = new Object ();
			object->addNotification (ClientWorld::getIntangibleNotification ());
			object->setAppearance (new RibbonAppearance (pointList, 0.1f, VectorArgb::solidRed));
			RenderWorld::addObjectNotifications (*object);
			object->setTransform_o2p (player->getTransform_o2w ());
			object->addToWorld ();
		}
		break;

	case 9:
		{
			Object* const childObject = new HardpointObject (ConstCharCrcLowerString ("head"));
			RenderWorld::addObjectNotifications (*childObject);
			childObject->setAppearance (AppearanceTemplateList::createAppearance (m_debugKeySubContext.c_str ()));
			getPlayer ()->addChildObject_o (childObject);
		}
		break;

	case 11:
		{
			const char* const objectTemplateNames [3] =
			{
				"object/building/poi/base/shared_base_poi_large.iff", 
				"object/building/poi/base/shared_base_poi_medium.iff",
				"object/building/poi/base/shared_base_poi_small.iff"
			};

			//-- test run-time rules
			int i;
			for (i = 0; i < 50; ++i)
			{
				ClientObject * const object = safe_cast<ClientObject *> (ObjectTemplateList::createObject (TemporaryCrcString (objectTemplateNames [Random::random (0, 2)], false)));
				object->setPosition_p (getPlayer ()->getPosition_p ());
				object->yaw_o (Random::randomReal (PI_TIMES_2));
				object->move_o (Vector::unitZ * Random::randomReal (1024.f));
				TerrainObject::getConstInstance ()->placeObject (*object, false, true);
				object->endBaselines ();
				object->addToWorld ();
			}
		}
		break;

	case 12:
		{
			//-- Test turning objects on/off
			if (Game::getSinglePlayer ())
			{
				TangibleObject* const tangibleObject = dynamic_cast<TangibleObject*> (safe_cast<CreatureObject*> (getPlayer ())->getLookAtTarget ().getObject ());
				if (tangibleObject)
				{
					const bool on = tangibleObject->hasCondition (TangibleObject::C_onOff);
					if (on)
						tangibleObject->clearCondition (TangibleObject::C_onOff);
					else
						tangibleObject->setCondition (TangibleObject::C_onOff);
				}
			}
		}
		break;

	case 13:
		{
			//-- Test terrain rebuilding by adding a building with a run-time rule to the player
			if (Game::getSinglePlayer ())
			{
				Object* const object = ObjectTemplateList::createObject (TemporaryCrcString (m_debugKeySubContext.c_str (), true));
				if (object)
				{
					RenderWorld::addObjectNotifications (*object);

					PortalProperty* const property = object->getPortalProperty ();
					if (property)
						property->clientSinglePlayerInitializeFirstTimeObject ();

					safe_cast<ClientObject*> (object)->endBaselines ();
					getPlayer ()->addChildObject_o (object);
				}
			}
		}
		break;

	case 14:
		{
			if (Game::getSinglePlayer ())
			{
				const TerrainObject* const terrainObject = TerrainObject::getConstInstance ();
				if (terrainObject)
				{
					m_debugPointList->clear ();

					const float deltaAngle = PI_OVER_4 / 16.f;
					const float startAngle = Random::randomReal (PI_TIMES_2);
					float angle = startAngle;
					int i = 0;
					while (angle < PI_TIMES_2 + startAngle)
					{
						Transform t;
						t.yaw_l (angle);
						const Vector position = t.rotateTranslate_l2p (Vector::unitZ * 50.f);

						float height = 0.f;
						if (!terrainObject->getHeightForceChunkCreation (position, height))
						{
							WARNING (true, ("yup %i %1.2f\n", i, angle));
							m_debugPointList->push_back (position);
						}

						angle += deltaAngle;
						++i;
					}
				}
			}
		}
		break;

	case 15:
		{
			//-- attach ribbon trail appearance to player
			Object* const object = getPlayer ();
			if (object)
			{
				const SkeletalAppearance2* const appearance = dynamic_cast<const SkeletalAppearance2*> (object->getAppearance ());
				if (appearance)
				{
					const Skeleton& skeleton = appearance->getSkeleton (appearance->getSkeletonLodCount () - 1);
					const CrcString& transformName = skeleton.getTransformName (Random::random (0, skeleton.getTransformCount () - 1));

					Object* const childObject = new HardpointObject (transformName);
					RenderWorld::addObjectNotifications (*childObject);
					childObject->setAppearance (AppearanceTemplateList::createAppearance (m_debugKeySubContext.c_str ()));
					object->addChildObject_o (childObject);
				}
			}
		}
		break;

	case 16:
		{
			std::vector<Vector> pointList;

			const TerrainObject* const terrainObject = TerrainObject::getConstInstance ();
			if (terrainObject)
			{
				const float deltaAngle = PI_OVER_4 / 16.f;
				const float startAngle = Random::randomReal (PI_TIMES_2);
				float angle = startAngle;
				int i = 0;
				while (angle < PI_TIMES_2)
				{
					Transform t;
					t.yaw_l (angle);
					Vector position = t.rotateTranslate_l2p (Vector::unitZ * Random::randomReal (40.f, 50.f));

					terrainObject->getHeight (position, position.y);
					pointList.push_back (position);

					angle += deltaAngle;
					++i;
				}
			}
			
			if (m_clientPathObject)
			{
				delete m_clientPathObject;
				m_clientPathObject = 0;
			}

			if (pointList.size ())
			{
				m_clientPathObject = new ClientPathObject (pointList);
				m_clientPathObject->addNotification (ClientWorld::getIntangibleNotification ());
				m_clientPathObject->addToWorld ();

				m_overheadMap->setClientPath (pointList);
			}
		}
		break;

	case 17:
		{
			TerrainObject const * const terrainObject = TerrainObject::getInstance ();
			if (!terrainObject)
			{
				DEBUG_WARNING (true, ("getGoodLocation (): PB no terrain"));
				return;
			}

			LotManager const * const lotManager = ClientWorld::getConstLotManager ();
			if (!lotManager)
			{
				DEBUG_WARNING (true, ("getGoodLocation (): PB no lot system"));
				return;
			}

			Object* const object = getPlayer ();
			if (!object)
			{
				DEBUG_WARNING (true, ("getGoodLocation (): PB no player"));
				return;
			}
		
			//build searching rectangles from the given locations
			Vector const position_w = object->getPosition_w ();
			Rectangle2d searchRect;
			searchRect.x0 = position_w.x - 200.f;
			searchRect.y0 = position_w.z - 200.f;
			searchRect.x1 = position_w.x + 200.f;
			searchRect.y1 = position_w.z + 200.f;

			//validate scripter-input rectangle areas > 0
			if (searchRect.x0 > searchRect.x1)
			{
				DEBUG_WARNING (true, ("getGoodLocation (): DB searchRect.x0 (%1.2f) > searchRect.x1 (%1.2f)", searchRect.x0, searchRect.x1));
				return;
			}

			if (searchRect.y0 > searchRect.y1)
			{
				DEBUG_WARNING (true, ("getGoodLocation (): DB searchRect.z0 (%1.2f) > searchRect.z1 (%1.2f)", searchRect.y0, searchRect.y1));
				return;
			}

			float const areaSizeX = 15.f;
			float const areaSizeY = 15.f;

			//validate that the area we want to find is smaller than our search area
			if (areaSizeX > searchRect.getWidth())
			{
				DEBUG_WARNING (true, ("getGoodLocation (): DB goal rectangle x (%1.2f) is larger than our search rectangle width (%1.2f)", areaSizeX, searchRect.getWidth()));
				return;
			}

			if (areaSizeY > searchRect.getHeight())
			{
				DEBUG_WARNING (true, ("getGoodLocation (): DB goal rectangle y (%1.2f) is larger than our search rectangle height (%1.2f)", areaSizeY, searchRect.getHeight()));
				return;
			}

			Rectangle2d startingRect;
			startingRect.x0 = searchRect.x0;
			startingRect.y0 = searchRect.y0;
			startingRect.x1 = startingRect.x0 + areaSizeX;
			startingRect.y1 = startingRect.y0 + areaSizeY;

			//generate vectors with a random raster range
			int numRows = static_cast<int>(floor(searchRect.getWidth() / areaSizeX));
			int numCols = static_cast<int>(floor(searchRect.getHeight() / areaSizeY));
			static std::vector<int> xRange;
			static std::vector<int> yRange;
			xRange.clear();
			yRange.clear();
			for(int i = 0; i < numRows; ++i)
				xRange.push_back(i);
			for(int j = 0; j < numCols; ++j)
				yRange.push_back(j);

			std::random_shuffle(xRange.begin(), xRange.end());
			std::random_shuffle(yRange.begin(), yRange.end());

			bool dontCheckWater = false;
			bool dontCheckSlope = m_debugKeySubContext == "1";

			Rectangle2d successRect;
			bool success = false;
			Rectangle2d currentRect;
			for(std::vector<int>::iterator it_x = xRange.begin(); it_x != xRange.end()  && !success; ++it_x)
			{
				for(std::vector<int>::iterator it_y = yRange.begin(); it_y != yRange.end() && !success; ++it_y)
				{
					currentRect = startingRect;

					//jump to the random raster area
					currentRect.translate(*it_x * areaSizeX, *it_y * areaSizeY);

					//the rectangle is a "good location" if it has neither water nor a steep slope
					if (!dontCheckWater && terrainObject->getWater (currentRect))
						continue;

					if (!dontCheckSlope && terrainObject->getSlope (currentRect))
						continue;

					if (!lotManager->canPlace (currentRect))
						continue;

					success = true;
					successRect = currentRect;
				}
			}

			if (!success)
			{
				DEBUG_WARNING (true, ("getGoodLocation (): failed"));
				return;
			}

			//validate that our success point lies within the search area (debugging tests)
			if (successRect.x0 < searchRect.x0 || successRect.x1 > searchRect.x1 || successRect.y0 < searchRect.y0 || successRect.y1 > searchRect.y1)
			{
				DEBUG_WARNING (true, ("getGoodLocation (): PB result does not fit within the search location"));
				return;
			}

			//get the center as a 2d point
			Vector2d successLoc2d = successRect.getCenter();
			//conver that point into a location-friendly 3d point
			Vector successLoc3d(successLoc2d.x, 0.f, successLoc2d.y);
			m_debugPointList->push_back (successLoc3d);
		}
		break;

	case 18:
		{
			IGNORE_RETURN (new EnvironmentMapGenerator (getCurrentCamera (), getPlayer ()));
		}
		break;

	case 19:
		{
			Vector const offsets [4] =
			{
				Vector::unitX,
				Vector::negativeUnitX,
				Vector::unitZ,
				Vector::negativeUnitZ
			};

			CellProperty const * const cellProperty = getPlayer ()->getParentCell ();
			if (cellProperty)
			{
				Floor const * const floor = cellProperty->getFloor ();
				if (floor)
				{
					for (int j = 0; j < 4; ++j)
					{
						Vector const position_p = getPlayer ()->getPosition_p () + offsets [j];

						FloorLocator result;
						Vector const direction_p = Vector::negativeUnitY;
						Ray3d ray_p (position_p + Vector::unitY, direction_p);
						if (floor->intersectClosest (ray_p, result))
							m_debugPointList->push_back (cellProperty->getOwner ().rotateTranslate_o2w (result.getPosition_p ()));
					}
				}
			}
		}
		break;

	case 20:
		{
			Object const * const player = getPlayer ();
			TerrainObject const * const terrainObject = TerrainObject::getConstInstance ();

			if (player && terrainObject)
			{
				PerformanceTimer timer;
				timer.start ();

				for (uint i = 0; i < 100; ++i)
				{
					Vector start_w = Vector::randomUnit () * 100.f + player->getPosition_w ();
					if (start_w.y < 0.f)
						start_w.y = -start_w.y;

					Vector end_w = Vector::randomUnit () * 100.f + player->getPosition_w ();
					if (end_w.y > 0.f)
						end_w.y = -end_w.y;

					CollisionInfo info;
					if (terrainObject->collide (start_w, end_w, info))
					{
						Line line;
						line.m_start_w = start_w;
						line.m_end_w = end_w;
						line.m_color = PackedArgb::solidRed;
						m_debugLineList->push_back (line);

						line.m_start_w = info.getPoint ();
						line.m_end_w = line.m_start_w + info.getNormal ();
						line.m_color = PackedArgb::solidGreen;
						m_debugLineList->push_back (line);
					}
					else
					{
						Line line;
						line.m_start_w = start_w;
						line.m_end_w = end_w;
						line.m_color = PackedArgb::solidGreen;
						m_debugLineList->push_back (line);
					}
				}

				timer.stop ();
				timer.logElapsedTime ("Terrain collision");
			}
		}
		break;

	case 21:
		{
			Object const * const player = getPlayer();
			TerrainObject * const terrainObject = TerrainObject::getInstance();

			if (player && terrainObject)
			{
				PerformanceTimer timer;
				timer.start();

				Vector start_w = player->getPosition_w();
				start_w.y += 0.5f;

				Vector end_w = Vector::randomUnit() * 64.f + start_w;
				terrainObject->getHeightForceChunkCreation(end_w, end_w.y);
				end_w.y += 0.5f;

				CollisionInfo info;
				if (terrainObject->collideForceChunkCreation(start_w, end_w, info))
				{
					Line line;
					line.m_start_w = start_w;
					line.m_end_w = end_w;
					line.m_color = PackedArgb::solidRed;
					m_debugLineList->push_back(line);

					line.m_start_w = info.getPoint();
					line.m_end_w = line.m_start_w + info.getNormal();
					line.m_color = PackedArgb::solidGreen;
					m_debugLineList->push_back(line);
				}
				else
				{
					Line line;
					line.m_start_w = start_w;
					line.m_end_w = end_w;
					line.m_color = PackedArgb::solidGreen;
					m_debugLineList->push_back(line);
				}

				timer.stop();
				timer.logElapsedTime("Terrain collideForceChunkCreation");
			}
		}
		break;

	default:
		break;
	}

#endif
}

// ---------------------------------------------------------------------

void GroundScene::handleDebugKeyContextKey2 () 
{
#ifdef _DEBUG
	switch (m_debugKeyContext)
	{
	case 4:
		{
			//-- test weather
			++ms_debugWeatherIndex;
			GroundEnvironment::getInstance().setWeatherIndex(ms_debugWeatherIndex);
		}
		break;

	case 6:
		{
			//-- increase time of day
			if (TerrainObject::getInstance ())
			{
				float currentTime = TerrainObject::getInstance ()->getTime ();
				currentTime += 0.015f;

				if (currentTime >= 1.f)
					currentTime -= 1.f;
				
				TerrainObject::getInstance ()->setTime (currentTime, true);
			}
		}
		break;

	case 7:
		{
			//-- remove all ribbon appearances trail appearance from player
			Object* const object = getPlayer ();
			int i;
			for (i = 0; i < object->getNumberOfChildObjects (); ++i)
			{
				Object* const childObject = object->getChildObject (i);
				if (dynamic_cast<RibbonTrailAppearance*> (childObject->getAppearance ()))
				{
					static_cast<RibbonTrailAppearance*> (childObject->getAppearance ())->kill ();
					break;
				}
			}
		}
		break;

	case 13:
		{
			//-- Remove all building objects from player
			Object* const object = getPlayer ();
			int i;
			for (i = 0; i < object->getNumberOfChildObjects (); ++i)
			{
				Object* const childObject = object->getChildObject (i);
				if (dynamic_cast<const BuildingObject*> (childObject))
					childObject->kill ();
			}
		}
		break;

	case 14:
		m_debugPointList->clear ();
		break;

	case 15:
		{
			//-- remove all particle appearances trail appearance from player
			Object* const object = getPlayer ();
			int i;
			for (i = 0; i < object->getNumberOfChildObjects (); ++i)
			{
				Object* const childObject = object->getChildObject (i);
				if (ParticleEffectAppearance::asParticleEffectAppearance(childObject->getAppearance ()))
				{
					childObject->kill ();
					break;
				}
			}
		}
		break;

	case 16:
		{
			if (m_clientPathObject)
			{
				delete m_clientPathObject;
				m_clientPathObject = 0;
			}

			m_overheadMap->clearClientPath ();
		}
		break;

	case 17:
	case 19:
		m_debugPointList->clear ();
		break;

	case 20:
	case 21:
		m_debugLineList->clear ();
		break;

	default:
		break;
	}
#endif
}

// =====================================================================
