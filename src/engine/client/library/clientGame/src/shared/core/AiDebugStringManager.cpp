// ======================================================================
//
// AiDebugStringManager.cpp
// Copyright 2004 Sony Online Entertainment, Inc.
//
// ======================================================================

#include "clientGame/FirstClientGame.h"
#include "clientGame/AiDebugStringManager.h"

#ifdef _DEBUG

#include "clientGame/ClientWorld.h"
#include "clientGame/FadingTextAppearance.h"
#include "clientGame/Game.h"
#include "clientGame/GroundScene.h"
#include "clientGame/ShipObject.h"
#include "clientGraphics/Camera.h"
#include "clientGraphics/DebugPrimitive.h"
#include "clientGraphics/RenderWorld.h"
#include "clientUserInterface/CuiTextManager.h"
#include "sharedCollision/CollisionProperty.h"
#include "sharedCollision/CollisionWorld.h"
#include "sharedCollision/Footprint.h"
#include "sharedDebug/InstallTimer.h"
#include "sharedFoundation/ExitChain.h"
#include "sharedFoundation/FormattedString.h"
#include "sharedFoundation/NetworkId.h"
#include "sharedFoundation/Os.h"
#include "sharedGame/AiDebugString.h"
#include "sharedMath/Circle.h"
#include "sharedObject/CachedNetworkId.h"
#include "sharedObject/CellProperty.h"
#include "sharedObject/NetworkIdManager.h"
#include "sharedObject/Object.h"
#include "sharedTerrain/TerrainObject.h"

#include <map>
#include <vector>

// ======================================================================
//
// AiDebugStringManagerNamespace
//
// ======================================================================

namespace AiDebugStringManagerNamespace
{
	class AiObjectData
	{
	public:

		AiObjectData(Object & object, AiDebugString const & aiDebugString)
		 : m_object(&object)
		 , m_aiDebugString(aiDebugString)
		 , m_finalString()
		{
			AiDebugString::TextList const & textList = m_aiDebugString.getTextList();
			AiDebugString::TextList::const_iterator iterTextList = textList.begin();

			for (; iterTextList != textList.end(); ++iterTextList)
			{
				std::string const & text = *iterTextList;
				m_finalString += text;
			}
		}

		Watcher<Object> m_object;
		AiDebugString const m_aiDebugString;
		std::string m_finalString;

	private:

		// Disabled

		AiObjectData();
		AiObjectData const & operator =(AiObjectData const &);
	};

	typedef std::map<NetworkId, std::string> PendingAiObjectList;
	typedef std::map<NetworkId, AiObjectData *> AiObjectList;

	PendingAiObjectList s_pendingAiObjectList;
	AiObjectList s_aiObjectList;
	int s_numberProcessedLastFrame = 0;
	int s_lastNumberOfUpdates = 0;

	void remove();
	float getHeight(Vector const & position_w, bool const snapToTerrain);
	void drawCircle(Vector const & position_w, float const radius, PackedRgb const & color, bool const snapToTerrain);
}

using namespace AiDebugStringManagerNamespace;

// ----------------------------------------------------------------------
void AiDebugStringManagerNamespace::remove()
{
	while (!s_aiObjectList.empty())
	{
		AiObjectData * aiObjectData = s_aiObjectList.begin()->second;
		delete aiObjectData;
		s_aiObjectList.erase(s_aiObjectList.begin());
	}
}

// ----------------------------------------------------------------------
float AiDebugStringManagerNamespace::getHeight(Vector const & position_w, bool const snapToTerrain)
{
	float result = 0.0f;
	TerrainObject * const terrainObject = TerrainObject::getInstance();

	if (   snapToTerrain
	    &&(terrainObject != NULL)
	    && terrainObject->getHeight(position_w, result))
	{
		result += 0.1f;
	}
	else
	{
		result = position_w.y + 0.03125f;
	}

	return result;
}

// ----------------------------------------------------------------------
void AiDebugStringManagerNamespace::drawCircle(Vector const & center_w, float const radius, PackedRgb const & color, bool const snapToTerrain)
{
	Camera const * const camera = Game::getCamera();
	int const edges = clamp(8, 8 + static_cast<int>(sqrt(radius)) * 6, 256);

	if (   (camera != NULL)
	    && (edges > 0))
	{
		TerrainObject * const terrainObject = TerrainObject::getInstance();

		if (terrainObject != NULL)
		{
			Vector a;
			Vector b;

			for (int edge = 0; edge <= edges; ++edge)
			{
				float const percent = (static_cast<float>(edge) / static_cast<float>(edges)) * PI_TIMES_2;
				float const x = center_w.x + sin(percent) * radius;
				float const z = center_w.z + cos(percent) * radius;
				float const y = getHeight(Vector(x, center_w.y, z), snapToTerrain);

				b = a;
				a = Vector(x, y, z);

				if (edge == 0)
				{
					continue;
				}

				camera->addDebugPrimitive(new Line3dDebugPrimitive(UtilityDebugPrimitive::S_z, Transform::identity, a, b, PackedArgb(255, color.r, color.g, color.b)));
			}
		}
		else
		{
			CircleDebugPrimitive * const circleDebugPrimitive = new CircleDebugPrimitive(CircleDebugPrimitive::S_z, Transform::identity, center_w, radius, edges);
			float const red = static_cast<float>(color.r) / 255.0f;
			float const green = static_cast<float>(color.g) / 255.0f;
			float const blue = static_cast<float>(color.b) / 255.0f;
			circleDebugPrimitive->setColor(VectorArgb(1.0f, red, green, blue));
			camera->addDebugPrimitive(circleDebugPrimitive);
		}
	}
}

// ======================================================================
//
// AiDebugStringManager
//
// ======================================================================

// ----------------------------------------------------------------------
void AiDebugStringManager::install()
{
	InstallTimer const installTimer("AiDebugStringManager::install");

	ExitChain::add(&remove, "ParticleManager::remove");
}

// ----------------------------------------------------------------------
void AiDebugStringManager::alter()
{
	if (s_lastNumberOfUpdates != Os::getNumberOfUpdates())
	{
		s_lastNumberOfUpdates = Os::getNumberOfUpdates();
		s_numberProcessedLastFrame = 0;
	}

	PendingAiObjectList::iterator iterPendingAiObjectList = s_pendingAiObjectList.begin();

	for (; iterPendingAiObjectList != s_pendingAiObjectList.end();)
	{
		NetworkId const & networkId = iterPendingAiObjectList->first;

		// If this object is already being tracked, delete the current debug info and lets add a new debug info

		AiObjectList::iterator iterAiObjectList = s_aiObjectList.find(networkId);

		if (iterAiObjectList != s_aiObjectList.end())
		{
			Object * aiObject = iterAiObjectList->second->m_object;
			delete aiObject;
			AiObjectData * aiObjectData = iterAiObjectList->second;
			delete aiObjectData;
			s_aiObjectList.erase(iterAiObjectList);
		}

		// Add the new debug info

		Object * object = NetworkIdManager::getObjectById(networkId);

		if (object != NULL)
		{
			//DEBUG_REPORT_LOG(true, ("%s\n", iterPendingAiObjectList->second.c_str()));
			AiDebugString aiDebugString(iterPendingAiObjectList->second);

			// See if we need to reset the client debugging

			if (aiDebugString.isClearClientFlagEnabled())
			{
				while (!s_aiObjectList.empty())
				{
					Object * aiObject = s_aiObjectList.begin()->second->m_object;
					s_aiObjectList.erase(s_aiObjectList.begin());
					delete aiObject;
				}

				s_pendingAiObjectList.clear();
				break;
			}

			Object * const childObject = new Object;
			AiObjectData * aiObjectData = new AiObjectData(*childObject, aiDebugString);
			IGNORE_RETURN(s_aiObjectList.insert(std::make_pair(networkId, aiObjectData)));
			childObject->addNotification(ClientWorld::getIntangibleNotification());
			RenderWorld::addObjectNotifications (*childObject);

			VectorArgb const color(VectorArgb::solidGreen);
			float const startingOpacity = 1.0f;
			float const fadeOutTime = 0.0f;
			float sizeModifier = 0.1f;
			FadingTextAppearance * const fadingTextAppearance = new FadingTextAppearance(Unicode::narrowToWide(aiObjectData->m_finalString), color, startingOpacity, fadeOutTime, sizeModifier, CuiTextManagerTextEnqueueInfo::TW_normal);

			fadingTextAppearance->setFadeEnabled(false);
			childObject->setAppearance(fadingTextAppearance);
			object->addChildObject_o(childObject);
			childObject->move_o(Vector::unitY * 0.15f);

			s_pendingAiObjectList.erase(iterPendingAiObjectList++);
		}
		else
		{
			++iterPendingAiObjectList;
		}
	}

	// See if we need to remove any dead objects

	{
		AiObjectList::iterator iterAiObjectList = s_aiObjectList.begin();

		for (; iterAiObjectList != s_aiObjectList.end();)
		{
			Object const * const object = iterAiObjectList->second->m_object;

			if (   (object == NULL)
			    || (object->getParent() == NULL))
			{
				s_aiObjectList.erase(iterAiObjectList++);
			}
			else
			{
				++iterAiObjectList;
			}
		}
	}

	// Draw the debug stuff every frame

	Camera const * const camera = Game::getCamera();

	if (camera != NULL)
	{
		AiObjectList::const_iterator iterAiObjectList = s_aiObjectList.begin();

		for (; iterAiObjectList != s_aiObjectList.end(); ++iterAiObjectList)
		{
			AiDebugString const & aiDebugString = iterAiObjectList->second->m_aiDebugString;
			Object const * const ownerObject = iterAiObjectList->second->m_object;

			if (ownerObject == NULL)
			{
				continue;
			}

			Object const * const parentObject = ownerObject->getRootParent();

			if (parentObject == NULL)
			{
				continue;
			}

			//CollisionProperty const * const ownerCollisionProperty = parentObject->getCollisionProperty();
			//Footprint const * const ownerFootprint = (ownerCollisionProperty != NULL) ? ownerCollisionProperty->getFootprint() : NULL;
			//Floor const * const ownerFloor = (ownerFootprint != NULL) ? CollisionWorld::getFloorStandingOn(*parentObject, ownerFootprint->getFloorList()) : NULL;
			//bool const snapToTerrain = (ownerFloor == NULL);
			bool const snapToTerrain = false;

			// Lines
			{
				AiDebugString::LineList::const_iterator iterLineList = aiDebugString.getLineList().begin();

				for (; iterLineList != aiDebugString.getLineList().end(); ++iterLineList)
				{
					Object const * const targetObject = iterLineList->first.getObject();

					Vector const & position = iterLineList->second.first;
					Vector a(ownerObject->getPosition_w());
					a.y = getHeight(a, snapToTerrain);
					Vector b((targetObject != NULL) ? targetObject->rotateTranslate_o2w(position) : position);
					b.y = getHeight(b, snapToTerrain);
					PackedRgb const & color = iterLineList->second.second;

					camera->addDebugPrimitive(new Line3dDebugPrimitive(UtilityDebugPrimitive::S_z, Transform::identity, a, b, PackedArgb(255, color.r, color.g, color.b)));

					if (targetObject == NULL)
					{
						// Draw a small circle at each position

						drawCircle(b, 0.25f, PackedRgb(color.r, color.g, color.b), snapToTerrain);
					}
				}
			}

			// Paths
			{
				AiDebugString::PathList::const_iterator iterPathList = aiDebugString.getPathList().begin();

				for (; iterPathList != aiDebugString.getPathList().end(); ++iterPathList)
				{
					Vector a;
					Vector b;
					AiDebugString::TransformList const & transformList = iterPathList->first;
					bool const cyclic = iterPathList->second;

					if (!transformList.empty())
					{
						AiDebugString::TransformList::const_iterator iterTransformList = transformList.begin();
						int index = 0;
						for (; iterTransformList != transformList.end(); ++iterTransformList)
						{
							b = a;
							a = iterTransformList->getPosition_p();

							{
								// Draw a small circle at each path position

								PackedRgb color;
								
								if (index == 0)
								{
									color = PackedRgb::solidGreen;
								}
								else if (index == static_cast<int>(transformList.size() - 1))
								{
									color = PackedRgb::solidMagenta;
								}

								drawCircle(a, 0.25f, color, snapToTerrain);
							}

							if (index > 0)
							{
								camera->addDebugPrimitive(new Line3dDebugPrimitive(UtilityDebugPrimitive::S_z, Transform::identity, a, b, PackedArgb::solidRed));
							}

							++index;
						}

						if (cyclic)
						{
							b = transformList.begin()->getPosition_p();
							camera->addDebugPrimitive(new Line3dDebugPrimitive(UtilityDebugPrimitive::S_z, Transform::identity, a, b, PackedArgb::solidRed));
						}
					}
				}
			}

			// Circles
			{
				AiDebugString::CircleList::const_iterator iterCircleList = aiDebugString.getCircleList().begin();

				for (; iterCircleList != aiDebugString.getCircleList().end(); ++iterCircleList)
				{
					Object const * const targetObject = iterCircleList->first.getObject();
					Circle const & circle = iterCircleList->second.first;
					PackedRgb const & color = iterCircleList->second.second;
					Vector const center_w((targetObject != NULL) ? targetObject->rotateTranslate_o2w(circle.getCenter()) : circle.getCenter());

					drawCircle(center_w, circle.getRadius(), color, snapToTerrain);
				}
			}

			// Axes
			{
				AiDebugString::AxisList::const_iterator iterAxisList = aiDebugString.getAxisList().begin();

				for (; iterAxisList != aiDebugString.getAxisList().end(); ++iterAxisList)
				{
					Transform const & transform = iterAxisList->first;
					float const axisLength = iterAxisList->second;
					FrameDebugPrimitive * const frameDebugPrimitive = new FrameDebugPrimitive(FrameDebugPrimitive::S_z, transform, axisLength);
					camera->addDebugPrimitive(frameDebugPrimitive);
				}
			}

			// Cones
			{
				AiDebugString::ConeList::const_iterator iterConeList = aiDebugString.getConeList().begin();

				for (; iterConeList != aiDebugString.getConeList().end(); ++iterConeList)
				{
					AiObjectData * const aiObjectData = iterAiObjectList->second;
					Object * const aiObject = aiObjectData->m_object;
					float const adjacentLength = iterConeList->first;
					float const coneAngle = iterConeList->second.first;
					PackedRgb const & color = iterConeList->second.second;
					float const red = static_cast<float>(color.r) / 255.0f;
					float const green = static_cast<float>(color.g) / 255.0f;
					float const blue = static_cast<float>(color.b) / 255.0f;

					Vector const directionToCamera(camera->getPosition_w() - aiObject->getPosition_w());
					Vector const aiObjectZAxis_w(aiObject->rotateTranslate_o2w(Vector(0.0f, 0.0f, 1.0f)));
					Vector sideDirection_w(directionToCamera.cross(aiObjectZAxis_w));
					
					if (!sideDirection_w.normalize())
					{
						sideDirection_w = camera->rotateTranslate_o2w(Vector(1.0f, 0.0f, 0.0f));
						sideDirection_w.normalize();
					}

					Vector const & a = aiObject->getPosition_w();
					float const oppositeLength = tan(coneAngle) * adjacentLength;

					{
						Vector b(aiObject->rotateTranslate_o2w(Vector(0.0f, 0.0f, adjacentLength)));
						b += (sideDirection_w * oppositeLength);

						camera->addDebugPrimitive(new Line3dDebugPrimitive(UtilityDebugPrimitive::S_z, Transform::identity, a, b, VectorArgb(1.0f, red, green, blue)));
					}

					{
						Vector b(aiObject->rotateTranslate_o2w(Vector(0.0f, 0.0f, adjacentLength)));
						b += (sideDirection_w * -oppositeLength);

						camera->addDebugPrimitive(new Line3dDebugPrimitive(UtilityDebugPrimitive::S_z, Transform::identity, a, b, VectorArgb(1.0f, red, green, blue)));
					}
				}
			}
		}

		// Update the per frame text

		iterAiObjectList = s_aiObjectList.begin();

		for (; iterAiObjectList != s_aiObjectList.end(); ++iterAiObjectList)
		{
			AiObjectData * const aiObjectData = iterAiObjectList->second;
			Object * const aiObject = aiObjectData->m_object;
			Appearance * const appearance = (aiObject != NULL) ? aiObject->getAppearance() : NULL;
			TextAppearance * const textAppearance = dynamic_cast<TextAppearance *>(appearance);

			if (   (aiObject != NULL)
			    && (textAppearance != NULL))
			{
				float distance = 0.0f;
				GroundScene * const groundScene = safe_cast<GroundScene *>(Game::getScene());

				if (   (groundScene != NULL)
				    && (groundScene->getCurrentView() == static_cast<int>(GroundScene::CI_free))
				    && (groundScene->getCurrentView() == static_cast<int>(GroundScene::CI_debugPortal)))
				{
					// We are in a free camera mode, so display the distance to the camera

					distance = aiObject->getPosition_w().magnitudeBetween(camera->getPosition_w());
					distance -= aiObject->getAppearanceSphereRadius();
				}
				else
				{
					// We are in a camera mode that is attached to the player, so calculate a distance from the player to the ai object

					if (Game::getPlayerContainingShip() != NULL)
					{
						distance = aiObject->getPosition_w().magnitudeBetween(Game::getPlayerContainingShip()->getPosition_w());
						distance -= aiObject->getAppearanceSphereRadius();
						distance -= Game::getPlayerContainingShip()->getAppearanceSphereRadius();
					}
					else
					{
						distance = aiObject->getPosition_w().magnitudeBetween(Game::getPlayer()->getPosition_w());
						distance -= aiObject->getAppearanceSphereRadius();
						distance -= Game::getPlayer()->getAppearanceSphereRadius();
					}
				}

				// Get the ShipObject's overall health

				Object * const parentObject = aiObject->getParent();
				ClientObject * const parentClientObject = (parentObject != NULL) ? parentObject->asClientObject() : NULL;
				ShipObject * const parentShipObject = (parentClientObject != NULL) ? parentClientObject->asShipObject() : NULL;

				if (parentShipObject != NULL)
				{
					std::string shipStatsString;

					{
						float const overallHealth = parentShipObject->getOverallHealthWithShieldsAndArmor() * 100.0f;
						float const currentSpeed = parentShipObject->getCurrentSpeed();
						bool const boostActive = parentShipObject->isBoosterActive();
						float const boosterAcceleration = parentShipObject->getBoosterAcceleration();
						char const * const text = FormattedString<4096>().sprintf("%.0fm hlth%.0f%% spd%.0f%s\n", distance, overallHealth, currentSpeed, (boosterAcceleration > 0.0f) ? (boostActive ? "+BOOST" : "+") : "-");
						shipStatsString = text;
					}

					std::string weaponString;

					{
						int const turretCount = parentShipObject->getTurretCount();
						int const countermeasureCount = parentShipObject->getCountermeasureCount();
						float const maxProjectileRange = parentShipObject->getMaxProjectileWeaponRange();
						bool addSpace = false;

						if (parentShipObject->hasMissiles())
						{
							addSpace = true;
							weaponString += "Missiles";
						}

						if (turretCount > 0)
						{
							weaponString += addSpace ? " " : "";
							weaponString += FormattedString<256>().sprintf("Turrets(%d)", turretCount);
							addSpace = true;
						}

						if (countermeasureCount > 0)
						{
							weaponString += addSpace ? " " : "";
							weaponString += FormattedString<256>().sprintf("CountrMsr(%d)", countermeasureCount);
							addSpace = true;
						}

						if (maxProjectileRange > 0.0f)
						{
							weaponString += addSpace ? " " : "";
							weaponString += FormattedString<256>().sprintf("GunRng(%.0f)", maxProjectileRange);
							addSpace = true;
						}
						else
						{
							weaponString += addSpace ? " " : "";
							weaponString += FormattedString<256>().sprintf("NO WEAPONS");
							addSpace = true;
						}

						if (addSpace)
						{
							weaponString += "\n";
						}
					}

					std::string shipPositionString;

					{
						Vector const & position_w = parentShipObject->getPosition_w();
						shipPositionString = FormattedString<256>().sprintf("pos(%.1f,%.1f,%.1f)", position_w.x, position_w.y, position_w.z);
					}

					char const * const text = FormattedString<4096>().sprintf("%s %s\n%s%s%s", parentShipObject->getNetworkId().getValueString().c_str(), shipPositionString.c_str(), shipStatsString.c_str(), weaponString.c_str(), aiObjectData->m_finalString.c_str());
					textAppearance->setText(text);
				}
				else
				{
					char const * const text = FormattedString<4096>().sprintf("%s (%.1fm)\n%s", parentObject->getNetworkId().getValueString().c_str(), distance, aiObjectData->m_finalString.c_str());
					textAppearance->setText(text);
				}
			}
		}
	}
}

// ----------------------------------------------------------------------
void AiDebugStringManager::add(NetworkId const & object, std::string const & text)
{
	++s_numberProcessedLastFrame;
	IGNORE_RETURN(s_pendingAiObjectList.insert(std::make_pair(object, text)));
}

// ----------------------------------------------------------------------
int AiDebugStringManager::getNumberProcessedLastFrame()
{
	return s_numberProcessedLastFrame;
}

// ======================================================================

#endif // _DEBUG
