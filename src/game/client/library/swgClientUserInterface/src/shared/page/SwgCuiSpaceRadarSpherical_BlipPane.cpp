//======================================================================
//
// SwgCuiSpaceRadarSpherical_BlipPane.cpp
// copyright(c) 2001 Sony Online Entertainment
//
//======================================================================
#include "swgClientUserInterface/FirstSwgClientUserInterface.h"
#include "swgClientUserInterface/SwgCuiSpaceRadarSpherical_BlipPane.h"

#include "clientGame/ClientWaypointObject.h"
#include "clientGame/ClientWorld.h"
#include "clientGame/CreatureObject.h"
#include "clientGame/Game.h"
#include "clientGame/GroupObject.h"
#include "clientGame/ShipObject.h"
#include "clientGraphics/Graphics.h"
#include "clientGraphics/ShaderTemplateList.h"
#include "clientObject/GameCamera.h"
#include "clientTerrain/ClientProceduralTerrainAppearance.h"
#include "clientUserInterface/ConfigClientUserInterface.h"
#include "clientUserInterface/CuiCombatManager.h"
#include "clientUserInterface/CuiDamageManager.h"
#include "clientUserInterface/CuiGameColorManager.h"
#include "clientUserInterface/CuiIoWin.h"
#include "clientUserInterface/CuiUtils.h"
#include "clientUserInterface/CuiWorkspaceIcon.h"
#include "sharedCollision/BoxExtent.h"
#include "sharedCollision/CollisionProperty.h"
#include "sharedFoundation/Timer.h"
#include "sharedGame/SharedBuildingObjectTemplate.h"
#include "sharedGame/SharedWaypointObjectTemplate.h"
#include "sharedMath/Range.h"
#include "sharedMessageDispatch/Transceiver.h"
#include "sharedNetworkMessages/ShipDamageMessage.h"
#include "sharedObject/CachedNetworkId.h"

#include "UIButton.h"
#include "UICanvas.h"
#include "UIData.h"
#include "UIImage.h"
#include "UIManager.h"
#include "UIText.h"

//======================================================================

namespace SwgCuiSpaceRadarSpherical_BlipPaneNamespace
{
	const Unicode::String s_canvasName = Unicode::narrowToWide("uicanvas_filtered_clamped:ui_radar_mask");
	UICanvas * s_maskCanvas = 0;
	const float s_blinkTimer = 3.0f;
	const float s_maxBlinkDuration = 15.0f;
	const float s_blinkTimerDuration = 0.2f;
	Range const s_blipSizes(7.0f, 3.0f);
	float const s_ooPI = 1.0f / PI;

	CachedNetworkId s_playerGroupId = CachedNetworkId::cms_cachedInvalid;
	CachedNetworkId s_playerGroupLeaderId = CachedNetworkId::cms_cachedInvalid;

	typedef std::map<NetworkId /*member*/, bool /*isleader*/> GroupShipMap;
	GroupShipMap s_groupMembers;

	Timer s_updateRadarObjects(0.5f);
	typedef std::vector<CachedNetworkId> CachedRadarObjectVector;
	CachedRadarObjectVector s_cachedRadarObjectVector;

	typedef std::map<float, CachedNetworkId> DistanceObjectMap;
	DistanceObjectMap s_distanceObjectMap;
	typedef std::vector<CachedNetworkId> ObjectVector;
	ObjectVector s_alwaysInBackObjectVector;
	ObjectVector s_alwaysInFrontObjectVector;
}

using namespace SwgCuiSpaceRadarSpherical_BlipPaneNamespace;
															
//----------------------------------------------------------------------

SwgCuiSpaceRadarSpherical::BlipPane::BlipPane(UIImage & blipImage, UIImage & blipImageCorpse, UIImage & blipImageWaypoint, UIImage & blipImageStructure, UIImage & bracket, UIImage & blipImageGroup) :
UIWidget               (),
m_blipImage            (blipImage),
m_blipImageCorpse      (blipImageCorpse),
m_blipImageWaypoint    (blipImageWaypoint),
m_blipImageStructure   (blipImageStructure),
m_blipImageGroup       (blipImageGroup),
m_blipImageTargetBracket(bracket),
m_callback(new MessageDispatch::Callback),
m_damagers(new DamagerMap),
m_blinkTimer(s_blinkTimerDuration),
m_blinkEnabled(false)
{
	SetName("BlipPane");

	m_blipImage.Attach            (0);
	m_blipImageCorpse.Attach      (0);
	m_blipImageWaypoint.Attach    (0);
	m_blipImageStructure.Attach   (0);
	m_blipImageGroup.Attach       (0);
	m_blipImageTargetBracket.Attach(0);

	m_blipImage.SetVisible            (false);
	m_blipImageCorpse.SetVisible      (false);
	m_blipImageWaypoint.SetVisible    (false);
	m_blipImageStructure.SetVisible   (false);
	m_blipImageGroup.SetVisible       (false);
	m_blipImageTargetBracket.SetVisible(false);

	s_maskCanvas = UIManager::gUIManager().GetCanvas(s_canvasName);
	NOT_NULL(s_maskCanvas);

	m_callback->connect(*this, &SwgCuiSpaceRadarSpherical::BlipPane::onShipDamaged, static_cast<CuiDamageManager::Messages::ShipDamage *>(0));
}

//----------------------------------------------------------------------

SwgCuiSpaceRadarSpherical::BlipPane::~BlipPane()
{
	m_callback->disconnect(*this, &SwgCuiSpaceRadarSpherical::BlipPane::onShipDamaged, static_cast<CuiDamageManager::Messages::ShipDamage *>(0));

	m_blipImage.Detach            (0);
	m_blipImageCorpse.Detach      (0);
	m_blipImageWaypoint.Detach    (0);
	m_blipImageStructure.Detach   (0);
	m_blipImageGroup.Detach       (0);
	m_blipImageTargetBracket.Detach(0);

	delete m_callback;
	m_callback = NULL;
	
	delete m_damagers;
	m_damagers = NULL;
}

//----------------------------------------------------------------------
// RLS TODO: Optimize this convert to widget code.  Cache up the ship, etc.
bool SwgCuiSpaceRadarSpherical::BlipPane::convertToWidgetSpace(ShipObject const * const ship, Vector &widgetPosition, bool abortOutOfRange, float & targetDistance) const
{	
	NOT_NULL(ship);

	//-- Get the ship position.
	Vector const & shipPosition_w = ship->getPosition_w();
	
	//-- Get the target position.
	Vector const & targetPosition_w = widgetPosition;
	
	//-- Check distances.
	float const targetDistanceSquared = (targetPosition_w - shipPosition_w).magnitudeSquared();

	//-- Check for out of range condition.
	float const radarRange = Game::getRadarRange();
	if (abortOutOfRange && targetDistanceSquared > sqr(radarRange))
	{
		return false;
	}

	//-- Get the real target distance.
	targetDistance = sqrt(targetDistanceSquared);

	//-- Get the local position of the object.
	Vector targetPosition_l;
	if (ship == Game::getPlayerPilotedShip())
	{
		ship->getTransform_o2w().rotateTranslate_p2l(&targetPosition_w, &targetPosition_l, 1);
	}
	else
	{
		Camera const * const camera = Game::getCamera();
		if (camera)
		{
			camera->getTransform_o2w().rotateTranslate_p2l(&targetPosition_w, &targetPosition_l, 1);
		}
		else
		{
			return false;
		}
	}
	
	float renderScale = 1.0f;
	if (targetDistance > targetPosition_l.z)
	{
		renderScale = static_cast<float>(acos(targetPosition_l.z / targetDistance) * s_ooPI);
	}
	
	//-- Make a new method for this.  magnitudexy();
	float const zDistance = static_cast<float>(_hypot(static_cast<double>(targetPosition_l.x), 
													 static_cast<double>(targetPosition_l.y)));

	Vector radarRadius(static_cast<float>(GetWidth()) * 0.45f, static_cast<float>(GetHeight()) * 0.45f, 0.0f);

	Vector targetPosition_widget(radarRadius);
	
	if (zDistance > FLT_MIN) 
	{
		float const zDistanceInverse = 1.0f / zDistance;

		targetPosition_widget.x =(targetPosition_l.x * zDistanceInverse) * renderScale * radarRadius.x;
		targetPosition_widget.y =(targetPosition_l.y * zDistanceInverse) * renderScale * radarRadius.y;
		
		float const outerRingSize             = 15.0f;
		float const localDisplacementOnZAxis  = static_cast<float>(_hypot(static_cast<double>(targetPosition_widget.x), 
																		 static_cast<double>(targetPosition_widget.y)));
		float const radarConeRadius           = radarRadius.approximateMagnitude() - outerRingSize;
		
		if(localDisplacementOnZAxis >= radarConeRadius && localDisplacementOnZAxis > FLT_MIN) 
		{
			float const diagInverse = 1.0f / localDisplacementOnZAxis;
				
			targetPosition_widget.x = radarConeRadius *(targetPosition_widget.x * diagInverse);
			targetPosition_widget.y = radarConeRadius *(targetPosition_widget.y * diagInverse);
		}
	}

	widgetPosition.x = targetPosition_widget.x;
	widgetPosition.y = -targetPosition_widget.y;

	return true;
}

//----------------------------------------------------------------------

void SwgCuiSpaceRadarSpherical::BlipPane::renderBlip(ShipObject const * const ship, UICanvas & canvas, Vector const & worldPos, float const worldRadius, UIImage & blip, const UIColor & color, bool const allowWaypointArrow, bool const isWaypoint, bool const isBracket, bool const isTargetGroup, bool const isBlinking, bool const isLargeShip) const
{	
	NOT_NULL(ship);

	UNREF(isTargetGroup);
	UNREF(allowWaypointArrow);
	UNREF(worldRadius);

	Vector objectPosition_w(worldPos);
	float distanceToTarget = Game::getRadarRange();

	if (!convertToWidgetSpace(ship, objectPosition_w, true, distanceToTarget))
	{
		return;
	}
	
	UIColor const oldColor(canvas.GetColor());
	canvas.SetColor(color);
	canvas.ModifyColor(blip.GetBackgroundTint());
	
	float const blipScale = clamp(0.0f, distanceToTarget / Game::getRadarRange(), 1.0f);
	long blipWidthAbs = static_cast<long>(s_blipSizes.linearInterpolate(blipScale) * (isWaypoint ? 2.0f : 1.0f) * (isBracket ? 2.0f : 1.0f));

	if (isBlinking)
	{
		blipWidthAbs *= (m_blinkEnabled ? 2 : 1);
	}
	
	long blipWidth = clamp(3L, blipWidthAbs, 19L);

	// Scale capital ships and space station blips.
	if (isLargeShip)
	{
		blipWidth *= 2L;
	}
	
	const UIPoint blipPos(static_cast<long>(objectPosition_w.x) +(GetWidth() - blipWidth) / 2,
							static_cast<long>(objectPosition_w.y) +(GetHeight() - blipWidth) / 2);
	
	blip.SetSize(UISize(blipWidth, blipWidth));
	canvas.Translate(blipPos.x,  blipPos.y);
	blip.Render(canvas);
	canvas.Translate(-blipPos.x, -blipPos.y);
	canvas.SetColor(oldColor);
}

//----------------------------------------------------------------------

void SwgCuiSpaceRadarSpherical::BlipPane::renderWorldObject(ShipObject const * const ship, UICanvas & canvas, const ClientObject& clientObject, const NetworkId & lookAtTargetGroupId) const
{
	NOT_NULL(ship);

	const int got = clientObject.getGameObjectType();
	const TangibleObject * const tangible = clientObject.asTangibleObject();
	const ClientWaypointObject * waypoint = 0;
	
	if(!tangible && clientObject.getObjectType() == SharedWaypointObjectTemplate::SharedWaypointObjectTemplate_tag)
	{
		waypoint = safe_cast<const ClientWaypointObject *>(&clientObject);
	}

	if(!tangible && !waypoint)
		return;

	if (clientObject.getNetworkId() == ship->getNetworkId())
	{
		return;
	}

	UIColor color;

	float maxRangeSquared = 0.0f;

	UIImage * blip = 0;

	CuiGameColorManager::Type type             = CuiGameColorManager::T_default;
	CuiGameColorManager::Type nonTargetType    = CuiGameColorManager::T_default;

	bool isTargetType = false;

	if (tangible)
	{
		type = CuiGameColorManager::findTypeForObject(*tangible, false);
		nonTargetType = type;
		isTargetType = CuiGameColorManager::isTargetType(nonTargetType);
		if(isTargetType)
			nonTargetType = CuiGameColorManager::findTypeForObject(*tangible, true);

		if(tangible->getContainedBy() != NULL)
		{
			return;
		}


		if(nonTargetType == CuiGameColorManager::T_corpse || nonTargetType == CuiGameColorManager::T_yourCorpse)
		{
			blip = &m_blipImageCorpse;
		}

		GroupShipMap::const_iterator itGroupObj = s_groupMembers.find(clientObject.getNetworkId());
		if (itGroupObj != s_groupMembers.end()) 
		{
			nonTargetType = (itGroupObj->second ? CuiGameColorManager::T_groupLeader : CuiGameColorManager::T_group);
			type =  nonTargetType;
		}

		if(CuiGameColorManager::objectHasOverrideColor(*tangible))
		{
			color = CuiGameColorManager::findColorForObject(*tangible, true, true);
		}
		else
		{
			color = CuiGameColorManager::getColorForType(nonTargetType);
		}
		
		maxRangeSquared = sqr(Game::getRadarRange() * 1.5f);
	}
	else if(waypoint)
	{
		maxRangeSquared = 16384.0f;
		const VectorArgb & colorArgb = waypoint->getColorArgb();
		color = CuiUtils::convertColor(colorArgb);
	}

	bool isLargeShip = false;
	if(!blip)
	{
		if (got == SharedObjectTemplate::GOT_ship_capital || got == SharedObjectTemplate::GOT_ship_station)
		{
			blip = &m_blipImageStructure;
			isLargeShip = true;
		}
		else
		{
			if(lookAtTargetGroupId.isValid())
			{
				const CreatureObject * const creature = clientObject.asCreatureObject();
				if(creature && creature->getGroup() == lookAtTargetGroupId)
					blip = &m_blipImageGroup;
			}
			
			if(!blip)
				blip = &m_blipImage;
		}
	}

	NOT_NULL(blip);

	const Vector& object_pos = clientObject.getAppearanceSphereCenter_w();

	const float distanceSquared = object_pos.magnitudeBetweenSquared(ship->getPosition_w());
	if(distanceSquared > maxRangeSquared)
		return;

	float sphereRadius = clientObject.getAppearanceSphereRadius();
	if(nonTargetType == CuiGameColorManager::T_corpse)
		sphereRadius *= 2.0f;
	else if(nonTargetType == CuiGameColorManager::T_yourCorpse)
		sphereRadius *= 3.0f;

	// Bracket mission critical object.
	bool const isMissionCritical = Game::getPlayerCreature() ? Game::getPlayerCreature()->isGroupMissionCriticalObject(clientObject.getNetworkId()) : false;
	
	if(isTargetType || isMissionCritical)
	{
		const UIColor & targetColor = CuiGameColorManager::getColorForType((isMissionCritical && !isTargetType) ? CuiGameColorManager::T_combatTarget : type);
		renderBlip(ship, canvas, object_pos, sphereRadius, m_blipImageTargetBracket, targetColor, true, waypoint != 0, true, false, false, isLargeShip);
	}
	
	bool const isDamagerObject = isDamager(clientObject.getNetworkId());
	renderBlip(ship, canvas, object_pos, sphereRadius, *blip, color, true, waypoint != 0, false, blip == &m_blipImageGroup, isDamagerObject, isLargeShip);

}

//----------------------------------------------------------------------

void SwgCuiSpaceRadarSpherical::BlipPane::Render(UICanvas & canvas) const
{
	//TODO: this function seems completely wrong
	ShipObject * const ship = Game::getPlayerContainingShip();
	if(!ship)
		return;

	if(!Game::getScene())
		return;

	NetworkId lookAtTargetGroupId;
	const Object * const lookAtObject = ship->getPilotLookAtTarget().getObject();
	const ClientObject * const lookAtClientObject =(lookAtObject != NULL) ? lookAtObject->asClientObject() : NULL;
	const CreatureObject * const lookAtTarget =(lookAtClientObject != NULL) ? lookAtClientObject->asCreatureObject() : NULL;


	// Keep track of the player group and group leader.
	s_playerGroupId = CachedNetworkId::cms_cachedInvalid;
	s_playerGroupLeaderId = CachedNetworkId::cms_cachedInvalid;
	s_groupMembers.clear();

	CreatureObject const * const player = Game::getPlayerCreature();
	if (player)
	{
		s_playerGroupId = player->getGroup();
		GroupObject const * const group = safe_cast<const GroupObject *>(s_playerGroupId.getObject());
		
		if(group)
		{
			// Get the leader.
			s_playerGroupLeaderId = group->getLeader();

			// Always have a leader.
			s_groupMembers[s_playerGroupLeaderId] = true;
			
			// Get leader ship.
			CachedNetworkId leaderShip(group->getShipFromMember(s_playerGroupLeaderId));
			
			// Add the leader ship to the group member list.
			s_groupMembers[leaderShip] = true;

			// Look for the rest of your buddies.
			GroupObject::GroupMemberVector const & groupMembers = group->getGroupMembers();
			for (GroupObject::GroupMemberVector::const_iterator itMember = groupMembers.begin(); itMember != groupMembers.end(); ++itMember)
			{
				// Get the member and add.
				GroupObject::GroupMember const & member = *itMember;
				s_groupMembers[member.first] = (s_playerGroupLeaderId == member.first);

				// Get the ship of the member in the group.
				CachedNetworkId memberShip(group->getShipFromMember(member.first));
				if (s_groupMembers.find(memberShip) == s_groupMembers.end()) 
				{
					s_groupMembers[memberShip] = false;
				}
			}
		}
	}

	if(lookAtTarget)
	{
		lookAtTargetGroupId = lookAtTarget->getGroup();
		if(lookAtTargetGroupId == s_playerGroupId)
			lookAtTargetGroupId = NetworkId::cms_invalid;
	}

	
	const UIPoint & point = GetWorldLocation();
	const UISize & widgetSize = GetSize();

	const UIPoint screenCenter(point.x + widgetSize.x / 2, point.y + widgetSize.y / 2);
	const UIRect widgetExtent(point.x, point.y, point.x +  widgetSize.x, point.y + widgetSize.y);

	Graphics::setStaticShader(ShaderTemplateList::get2dVertexColorStaticShader());
	Graphics::setObjectToWorldTransformAndScale(Transform::identity, Vector::xyz111);

	canvas.SetOpacity(ConfigClientUserInterface::getHudOpacity());
	
	{
		const ClientWaypointObject::WaypointVector & wv = ClientWaypointObject::getActiveWaypoints();
		
		for(ClientWaypointObject::WaypointVector::const_iterator it = wv.begin(); it != wv.end(); ++it)
		{
			const ClientWaypointObject * const waypoint = *it;
			if(waypoint)
			{
				const VectorArgb & colorArgb = waypoint->getColorArgb();
				const UIColor & color = CuiUtils::convertColor(colorArgb);
				
				renderBlip(ship, canvas, waypoint->getLocation(), 3.0f, m_blipImageWaypoint, color, true, true, false, false, false, false);
			}
		}
	}
	
	{
		// Get the ship position.
		const Vector & pos_w = ship->getPosition_w();

		// Reset the distance object map.
		s_distanceObjectMap.clear();
		s_alwaysInBackObjectVector.clear();
		s_alwaysInFrontObjectVector.clear();

		// --------------------------------------------------------------

		// Don't poll for objects every frame.
		if (s_updateRadarObjects.isExpired() || s_cachedRadarObjectVector.empty()) 
		{
			// Reset the timer.
			s_updateRadarObjects.reset();

			// Keep a static vector to prevent memory thrashing.
			static ClientWorld::ObjectVector radarObjectVector;
			radarObjectVector.clear();

			// Get Objects in range.
			const float maxRange = std::min(8192.f, Game::getRadarRange());		
			ClientWorld::findObjectsInRange(pos_w, maxRange, radarObjectVector);

			// Reset the cached vector.
			s_cachedRadarObjectVector.clear();

			// Copy to the cached list.
			ClientWorld::ObjectVector::const_iterator end = radarObjectVector.end();
			for(ClientWorld::ObjectVector::const_iterator it = radarObjectVector.begin(); it != end; ++it)
			{
				const ClientObject * const obj = dynamic_cast<ClientObject*>(*it);
				if (obj) 
				{
					s_cachedRadarObjectVector.push_back(CachedNetworkId(obj->getNetworkId()));
				}
			}
		}
		
		// --------------------------------------------------------------

		{
			if (player) 
			{
				// Force mission critical ships visible at all ranges.
				CreatureObject::GroupMissionCriticalObjectSet const & missionCriticalSet = player->getGroupMissionCriticalObjects();
				
				for (CreatureObject::GroupMissionCriticalObjectSet::const_iterator itMissionCritical = missionCriticalSet.begin(); itMissionCritical != missionCriticalSet.end(); ++itMissionCritical)
				{
					IGNORE_RETURN(s_alwaysInFrontObjectVector.push_back(CachedNetworkId(itMissionCritical->second)));
				}
			}
		}

		// --------------------------------------------------------------

		{
			CachedRadarObjectVector::const_iterator end = s_cachedRadarObjectVector.end();
			for(CachedRadarObjectVector::const_iterator it = s_cachedRadarObjectVector.begin(); it != end; ++it)
			{
				CachedNetworkId const & cachedNetId = *it;
				Object const * const object = cachedNetId.getObject();
				ClientObject const * const obj = object ? object->asClientObject() : NULL;				
				if(obj && obj->isActive() && obj->getAppearance())
				{
					//don't render waypoints here
					if(obj->getObjectType() == SharedWaypointObjectTemplate::SharedWaypointObjectTemplate_tag)
						continue;
					
					float distance = -pos_w.magnitudeBetweenSquared(obj->getPosition_w());
					
					// Draw cap ships behind all others because of their size.
					int const got = obj->getGameObjectType();
					if (got == SharedObjectTemplate::GOT_ship_capital || got == SharedObjectTemplate::GOT_ship_station)
					{
						IGNORE_RETURN(s_alwaysInBackObjectVector.push_back(cachedNetId));						
					}
					else
					{					
						IGNORE_RETURN(s_distanceObjectMap.insert(std::make_pair(distance, cachedNetId)));
					}
				}
			}
		}

		// --------------------------------------------------------------

		{
			for (ObjectVector::const_iterator itb = s_alwaysInBackObjectVector.begin(); itb != s_alwaysInBackObjectVector.end(); ++itb)
			{
				Object const * const object = (*itb).getObject();
				ClientObject const * const obj = object ? object->asClientObject() : NULL;
				
				if (obj && obj->isActive() && obj->getAppearance()) 
				{
					renderWorldObject(ship, canvas, *obj, lookAtTargetGroupId);
				}
			}

			for (DistanceObjectMap::const_iterator it = s_distanceObjectMap.begin(); it != s_distanceObjectMap.end(); ++it)
			{
				Object const * const object = (*it).second.getObject();
				ClientObject const * const obj = object ? object->asClientObject() : NULL;
				
				if (obj && obj->isActive() && obj->getAppearance()) 
				{
					renderWorldObject(ship, canvas, *obj, lookAtTargetGroupId);
				}
			}
				
			for (ObjectVector::const_iterator itf = s_alwaysInFrontObjectVector.begin(); itf != s_alwaysInFrontObjectVector.end(); ++itf)
			{
				Object const * const object = (*itf).getObject();
				ClientObject const * const obj = object ? object->asClientObject() : NULL;
				
				if (obj && obj->isActive() && obj->getAppearance()) 
				{
					renderWorldObject(ship, canvas, *obj, lookAtTargetGroupId);
				}
			}
		}
	}
}

//-----------------------------------------------------------------

void  SwgCuiSpaceRadarSpherical::BlipPane::onShipDamaged(const ShipDamageMessage & shipDamage)
{
	// Add to damager list.
	damagerAdd(shipDamage.getAttackerNetworkId());
}

//-----------------------------------------------------------------

void SwgCuiSpaceRadarSpherical::BlipPane::damagerAdd(const NetworkId & attackerNetworkId)
{
	DamagerMap::iterator itDamager = m_damagers->find(attackerNetworkId);
	if (itDamager == m_damagers->end())
	{
		// Add the damager.
		(*m_damagers)[attackerNetworkId] = s_blinkTimer;
	}
	else
	{
		itDamager->second = clamp(0.0f, itDamager->second + s_blinkTimer, s_maxBlinkDuration);
	}
}

//-----------------------------------------------------------------

void SwgCuiSpaceRadarSpherical::BlipPane::update(float deltaTimeSeconds)
{
	for (DamagerMap::iterator itDamagers = m_damagers->begin();itDamagers != m_damagers->end(); /**/)
	{
		itDamagers->second -= deltaTimeSeconds;
				
		// increment iterator.
		if (itDamagers->second < 0.0f)
		{
			DamagerMap::iterator itDamagerDeleted = itDamagers;
			itDamagers++;
			m_damagers->erase(itDamagerDeleted);
		}
		else
		{
			++itDamagers;
		}
	}

	if (m_blinkTimer.updateZero(deltaTimeSeconds))
	{
		m_blinkEnabled = !m_blinkEnabled;
	}

	// Do not scan for all object every frame.
	IGNORE_RETURN(s_updateRadarObjects.updateNoReset(deltaTimeSeconds));
}

//-----------------------------------------------------------------

bool SwgCuiSpaceRadarSpherical::BlipPane::isDamager(const NetworkId & blipNetworkId) const
{
	return m_damagers->find(blipNetworkId) != m_damagers->end();
}


//======================================================================
