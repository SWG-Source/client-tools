//======================================================================
//
// SwgCuiSpaceRadarSpherical2_BlipPane.cpp
// copyright(c) 2001 Sony Online Entertainment
//
//======================================================================
#include "swgClientUserInterface/FirstSwgClientUserInterface.h"
#include "swgClientUserInterface/SwgCuiSpaceRadarSpherical2_BlipPane.h"

#include "UIButton.h"
#include "UICanvas.h"
#include "UIData.h"
#include "UIImage.h"
#include "UIManager.h"
#include "UIText.h"
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
#include "clientUserInterface/CuiCombatManager.h"
#include "clientUserInterface/CuiDamageManager.h"
#include "clientUserInterface/CuiGameColorManager.h"
#include "clientUserInterface/CuiIoWin.h"
#include "clientUserInterface/CuiUtils.h"
#include "clientUserInterface/CuiWorkspaceIcon.h"
#include "sharedCollision/BoxExtent.h"
#include "sharedCollision/CollisionProperty.h"
#include "sharedGame/GameObjectTypes.h"
#include "sharedGame/PvpData.h"
#include "sharedGame/SharedBuildingObjectTemplate.h"
#include "sharedGame/SharedCreatureObjectTemplate.h"
#include "sharedGame/SharedWaypointObjectTemplate.h"
#include "sharedMath/Range.h"
#include "sharedMessageDispatch/Transceiver.h"
#include "sharedNetworkMessages/ShipDamageMessage.h"


//======================================================================

namespace SwgCuiSpaceRadarSpherical2_BlipPaneNamespace
{
	const Unicode::String s_canvasName = Unicode::narrowToWide("uicanvas_filtered_clamped:ui_radar_mask");
	UICanvas * s_maskCanvas = 0;
	const float s_blinkTimer = 3.0f;
	const float s_maxBlinkDuration = 15.0f;
	const float s_blinkTimerDuration = 0.2f;
	Range const s_blipSizes(7.0f, 3.0f);
	float const s_ooPI = 1.0f / PI;
}

using namespace SwgCuiSpaceRadarSpherical2_BlipPaneNamespace;
															
//----------------------------------------------------------------------

SwgCuiSpaceRadarSpherical2::BlipPane::BlipPane(UIImage & blipImage, UIImage & blipImageCorpse, UIImage & blipImageWaypoint, UIImage & blipImageStructure, UIImage & bracket, UIImage & blipImageGroup) :
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

	m_callback->connect(*this, &SwgCuiSpaceRadarSpherical2::BlipPane::onShipDamaged, static_cast<CuiDamageManager::Messages::ShipDamage *>(0));
}

//----------------------------------------------------------------------

SwgCuiSpaceRadarSpherical2::BlipPane::~BlipPane()
{
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
bool SwgCuiSpaceRadarSpherical2::BlipPane::convertToWidgetSpace(ShipObject const * const ship, Vector &widgetPosition, bool abortOutOfRange, float & targetDistance) const
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

void SwgCuiSpaceRadarSpherical2::BlipPane::renderBlip(ShipObject const * const ship, UICanvas & canvas, Vector const & worldPos, float const worldRadius, UIImage & blip, const UIColor & color, bool const allowWaypointArrow, bool const isWaypoint, bool const isBracket, bool const isTargetGroup, bool const isBlinking, bool const isLargeShip) const
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

void SwgCuiSpaceRadarSpherical2::BlipPane::renderWorldObject(ShipObject const * const ship, UICanvas & canvas, const ClientObject& clientObject, const NetworkId & lookAtTargetGroupId) const
{
	NOT_NULL(ship);

	const int got = clientObject.getGameObjectType();
	if(got == SharedObjectTemplate::GOT_misc_furniture || GameObjectTypes::isTypeOf(got, SharedObjectTemplate::GOT_building))
		return;

	const Tag objectType = clientObject.getObjectType();
	if(objectType == SharedBuildingObjectTemplate::SharedBuildingObjectTemplate_tag)
		return;

	const TangibleObject * const tangible = clientObject.asTangibleObject();
	const ClientWaypointObject * waypoint = 0;
	
	if(!tangible && objectType == SharedWaypointObjectTemplate::SharedWaypointObjectTemplate_tag)
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

	const CreatureObject * rider = 0;

	if (tangible)
	{
		type = CuiGameColorManager::findTypeForObject(*tangible, false);
		nonTargetType = type;
		if(CuiGameColorManager::isTargetType(nonTargetType))
			nonTargetType = CuiGameColorManager::findTypeForObject(*tangible, true);

		if(tangible->getContainedBy() != NULL)
		{
			return;
		}

		if(type != CuiGameColorManager::T_target && 
			type != CuiGameColorManager::T_corpse &&
			type != CuiGameColorManager::T_yourCorpse &&
			type != CuiGameColorManager::T_group &&
			type != CuiGameColorManager::T_groupLeader &&
			type != CuiGameColorManager::T_combatTarget &&
			got  != SharedObjectTemplate::GOT_creature_droid_probe)

		{
			if(tangible->isInvulnerable() || !tangible->isTargettable())
				return;
		}

		if(nonTargetType == CuiGameColorManager::T_corpse || nonTargetType == CuiGameColorManager::T_yourCorpse)
		{
			blip = &m_blipImageCorpse;
		}

		if(CuiGameColorManager::objectHasOverrideColor(*tangible))
		{
			color = CuiGameColorManager::findColorForObject(*tangible, true, true);
		}
		else
		{
			color = CuiGameColorManager::getColorForType(nonTargetType);
		}
		float rangeMod = std::max(CuiGameColorManager::getRangeModForType(type), CuiGameColorManager::getRangeModForType(nonTargetType));
		if(got == SharedObjectTemplate::GOT_creature_droid_probe)
		{
			//-- test to see if the probe droid is owned by you
			rangeMod = 1000.0f;
		}
		
		const CreatureObject * const targetCreature = tangible->asCreatureObject();
		if(targetCreature)
		{
			rider = targetCreature->getRiderDriverCreature();
			if(rider)
			{
				CuiGameColorManager::Type riderType             = CuiGameColorManager::findTypeForObject(*rider, false);
				rangeMod = std::max(CuiGameColorManager::getRangeModForType(riderType), rangeMod);
			}
		}
		
		maxRangeSquared = sqr(rangeMod * Game::getRadarRange() * 1.5f);
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
		if (got == SharedObjectTemplate::GOT_ship_capital || 
			got == SharedObjectTemplate::GOT_ship_station || 
			got == SharedObjectTemplate::GOT_ship_mining_asteroid_static)
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
	else if (nonTargetType == SharedObjectTemplate::GOT_ship_mining_asteroid_static)
		sphereRadius *= 3.0f;

	if(CuiGameColorManager::isTargetType(type))
	{
		const UIColor & targetColor = CuiGameColorManager::getColorForType(type);
		renderBlip(ship, canvas, object_pos, sphereRadius, m_blipImageTargetBracket, targetColor, true, waypoint != 0, true, false, false, isLargeShip);
	}
	
	bool const isDamagerObject = isDamager(clientObject.getNetworkId());
	renderBlip(ship, canvas, object_pos, sphereRadius, *blip, color, true, waypoint != 0, false, blip == &m_blipImageGroup, isDamagerObject, isLargeShip);

	//-- render the rider
	if(rider)
		renderWorldObject(ship, canvas, *rider, lookAtTargetGroupId);
}

//----------------------------------------------------------------------

void SwgCuiSpaceRadarSpherical2::BlipPane::Render(UICanvas & canvas) const
{
	//TODO:  this function seems completely wrong
	ShipObject * const ship = Game::getPlayerContainingShip();
	if(!ship)
		return;

	if(!Game::getScene())
		return;

	NetworkId lookAtTargetGroupId;
	const Object * const lookAtObject = ship->getPilotLookAtTarget().getObject();
	const ClientObject * const lookAtClientObject =(lookAtObject != NULL) ? lookAtObject->asClientObject() : NULL;
	const CreatureObject * const lookAtTarget =(lookAtClientObject != NULL) ? lookAtClientObject->asCreatureObject() : NULL;


	CachedNetworkId playerGroupId;
	NetworkId playerGroupLeaderId;
	CreatureObject const * const player = Game::getPlayerCreature();

	if(player)
	{
		playerGroupId = player->getGroup();

		const GroupObject * const group = safe_cast<const GroupObject *>(playerGroupId.getObject());
		if(group)
			playerGroupLeaderId = group->getLeader();
	}

	if(lookAtTarget)
	{
		lookAtTargetGroupId = lookAtTarget->getGroup();
		if(lookAtTargetGroupId == playerGroupId)
			lookAtTargetGroupId = NetworkId::cms_invalid;
	}

	
	const UIPoint & point = GetWorldLocation();
	const UISize & widgetSize = GetSize();

	const UIPoint screenCenter(point.x + widgetSize.x / 2, point.y + widgetSize.y / 2);
	const UIRect widgetExtent(point.x, point.y, point.x +  widgetSize.x, point.y + widgetSize.y);

	Graphics::setStaticShader(ShaderTemplateList::get2dVertexColorStaticShader());
	Graphics::setObjectToWorldTransformAndScale(Transform::identity, Vector::xyz111);

	canvas.SetOpacity(0.8f);
	
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
		const Vector & pos_w = ship->getPosition_w();
		
		typedef stdmap<float, const ClientObject *>::fwd DistanceObjectMap;
		DistanceObjectMap dom;
		
		{
			const float maxRange = std::min(8192.f, Game::getRadarRange());		
			static ClientWorld::ObjectVector cov;
			cov.clear();
			ClientWorld::findObjectsInRange(pos_w, maxRange, cov);
			
			ClientWorld::ObjectVector::const_iterator end = cov.end();
			for(ClientWorld::ObjectVector::const_iterator it = cov.begin(); it != end; ++it)
			{
				const ClientObject * const obj = dynamic_cast<ClientObject*>(*it);
				if(obj && obj->isActive() && obj->getAppearance())
				{
					//don't render waypoints here
					if(obj->getObjectType() == SharedWaypointObjectTemplate::SharedWaypointObjectTemplate_tag)
						continue;
					
					float distance = -pos_w.magnitudeBetweenSquared(obj->findPosition_w());
					
					const CreatureObject * const creature = obj->asCreatureObject();

					if(obj == ship)
						distance = 10000.0f;
					else if(obj == lookAtTarget)
						distance = 9000.0f;
					else if(playerGroupLeaderId == obj->getNetworkId())
						distance = 8500.0f;
					else if(creature && playerGroupId.isValid() && creature->getGroup() == playerGroupId)
						distance = 8000.0f;
					else if(creature &&(creature->getPvpFlags() & PvpStatusFlags::CanAttackYou) != 0)
						distance = 7000.0f;
					else if(creature && creature->isPlayer())
						distance = 6000.0f;
					//-- bias non creatures fully below the creatures
					else if(!creature)
						distance -= 16384.0f;

					IGNORE_RETURN(dom.insert(std::make_pair(distance, obj)));
					
					renderWorldObject(ship, canvas, *obj, lookAtTargetGroupId);
				}
			}
		}
		
		{
			for(DistanceObjectMap::const_iterator it = dom.begin(); it != dom.end(); ++it)
			{
				const ClientObject * const obj =(*it).second;
				renderWorldObject(ship, canvas, *obj, lookAtTargetGroupId);
			}
		}
	}
}

//-----------------------------------------------------------------

void  SwgCuiSpaceRadarSpherical2::BlipPane::onShipDamaged(ShipDamageMessage const &shipDamage)
{
	// Add to damager list.
	damagerAdd(shipDamage.getAttackerNetworkId());
}

//-----------------------------------------------------------------

void SwgCuiSpaceRadarSpherical2::BlipPane::damagerAdd(const NetworkId & attackerNetworkId)
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

void SwgCuiSpaceRadarSpherical2::BlipPane::update(float deltaTimeSeconds)
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
}

//-----------------------------------------------------------------

bool SwgCuiSpaceRadarSpherical2::BlipPane::isDamager(const NetworkId & blipNetworkId) const
{
	return m_damagers->find(blipNetworkId) != m_damagers->end();
}


//======================================================================
