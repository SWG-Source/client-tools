//===================================================================
//
// ShipProjectileMissHitData.cpp
//
// Copyright 2005 Sony Online Entertainment
// All Rights Reserved.
//
//===================================================================

#include "clientGame/FirstClientGame.h"
#include "clientGame/ShipProjectileMissHitData.h"

#include "clientAudio/Audio.h"
#include "clientGame/ClientEffect.h"
#include "clientGame/ClientEffectTemplate.h"
#include "clientGame/ClientEffectTemplateList.h"
#include "clientGame/ClientShipTargeting.h"
#include "clientGame/ClientWorld.h"
#include "clientGame/CreatureObject.h"
#include "clientGame/Game.h"
#include "clientGame/GameMusicManager.h"
#include "clientGame/ShipObject.h"
#include "clientGraphics/Camera.h"
#include "clientUserInterface/CuiDamageManager.h"
#include "sharedCollision/CollideParameters.h"
#include "sharedCollision/CollisionInfo.h"
#include "sharedDebug/DebugFlags.h"
#include "sharedDebug/InstallTimer.h"
#include "sharedFoundation/ExitChain.h"
#include "sharedFoundation/MemoryBlockManager.h"
#include "sharedGame/SharedTangibleObjectTemplate.h"
#include "sharedGame/ShipComponentFlags.h"
#include "sharedMath/AxialBox.h"
#include "sharedMath/Volume.h"
#include "sharedNetworkMessages/ShipDamageMessage.h"
#include "sharedObject/AlterResult.h"
#include "sharedObject/Appearance.h"
#include "sharedObject/CellProperty.h"

//----------------------------------------------------------------------

namespace ShipProjectileMissHitDataNamespace
{	
	Object const * findRootParent(Object const & childObject)
	{
		Object const * const attachedTo = childObject.getAttachedTo();
		
		if (NULL != attachedTo)
			return findRootParent(*attachedTo);
		
		return &childObject;
	}

	uint16 const ms_collisionFlags = ClientWorld::CF_tangible | ClientWorld::CF_tangibleNotTargetable | ClientWorld::CF_childObjects | ClientWorld::CF_disablePortalCrossing;
	bool ms_disableHitEffects;

}

using namespace ShipProjectileMissHitDataNamespace;

//----------------------------------------------------------------------

void ShipProjectileMissHitData::install()
{
	DebugFlags::registerFlag(ms_disableHitEffects, "ClientGame/ShipProjectile", "disableHitEffects");
	ExitChain::add(ShipProjectileMissHitData::remove, "ShipProjectileMissHitData::remove");
}

//----------------------------------------------------------------------

void ShipProjectileMissHitData::remove()
{
	DebugFlags::unregisterFlag(ms_disableHitEffects);
}	

//----------------------------------------------------------------------

ShipProjectileMissHitData::ShipProjectileMissHitData() : 
m_missHitOtherClientEffectTemplate(0),
m_missHitMetalClientEffectTemplate(0),
m_missHitStoneClientEffectTemplate(0),
m_missHitAcidClientEffectTemplate(0),
m_missHitIceClientEffectTemplate(0),
m_missHitMoltenClientEffectTemplate(0),
m_missHitObsidianClientEffectTemplate(0),
m_missHitShieldClientEffectTemplate(0),
m_missHitShieldShellClientEffectTemplate(0),
m_missHitFriendlyFireClientEffectTemplate(0)
{
}

// ----------------------------------------------------------------------

ShipProjectileMissHitData::~ShipProjectileMissHitData()
{
}

//----------------------------------------------------------------------

void ShipProjectileMissHitData::fetch()
{
	if (m_missHitOtherClientEffectTemplate)
		m_missHitOtherClientEffectTemplate->fetch();
	
	if (m_missHitMetalClientEffectTemplate)
		m_missHitMetalClientEffectTemplate->fetch();
	
	if (m_missHitStoneClientEffectTemplate)
		m_missHitStoneClientEffectTemplate->fetch();
	
	if (m_missHitAcidClientEffectTemplate)
		m_missHitAcidClientEffectTemplate->fetch();
	
	if (m_missHitIceClientEffectTemplate)
		m_missHitIceClientEffectTemplate->fetch();
	
	if (m_missHitMoltenClientEffectTemplate)
		m_missHitMoltenClientEffectTemplate->fetch();
	
	if (m_missHitObsidianClientEffectTemplate)
		m_missHitObsidianClientEffectTemplate->fetch();
	
	if (m_missHitShieldClientEffectTemplate)
		m_missHitShieldClientEffectTemplate->fetch();
	
	if (m_missHitShieldShellClientEffectTemplate)
		m_missHitShieldShellClientEffectTemplate->fetch();
	
	if (m_missHitFriendlyFireClientEffectTemplate)
		m_missHitFriendlyFireClientEffectTemplate->fetch();
	
}

//----------------------------------------------------------------------

void ShipProjectileMissHitData::release()
{
	if (m_missHitOtherClientEffectTemplate)
	{
		m_missHitOtherClientEffectTemplate->release();
		m_missHitOtherClientEffectTemplate = 0;
	}
	
	if (m_missHitMetalClientEffectTemplate)
	{
		m_missHitMetalClientEffectTemplate->release();
		m_missHitMetalClientEffectTemplate = 0;
	}
	
	if (m_missHitStoneClientEffectTemplate)
	{
		m_missHitStoneClientEffectTemplate->release();
		m_missHitStoneClientEffectTemplate = 0;
	}
	
	if (m_missHitAcidClientEffectTemplate)
	{
		m_missHitAcidClientEffectTemplate->release();
		m_missHitAcidClientEffectTemplate = 0;
	}
	
	if (m_missHitIceClientEffectTemplate)
	{
		m_missHitIceClientEffectTemplate->release();
		m_missHitIceClientEffectTemplate = 0;
	}
	
	if (m_missHitMoltenClientEffectTemplate)
	{
		m_missHitMoltenClientEffectTemplate->release();
		m_missHitMoltenClientEffectTemplate = 0;
	}
	
	if (m_missHitObsidianClientEffectTemplate)
	{
		m_missHitObsidianClientEffectTemplate->release();
		m_missHitObsidianClientEffectTemplate = 0;
	}
	
	if (m_missHitShieldClientEffectTemplate)
	{
		m_missHitShieldClientEffectTemplate->release();
		m_missHitShieldClientEffectTemplate = 0;
	}
	
	if (m_missHitShieldShellClientEffectTemplate)
	{
		m_missHitShieldShellClientEffectTemplate->release();
		m_missHitShieldShellClientEffectTemplate = 0;
	}
	
	if (m_missHitFriendlyFireClientEffectTemplate)
	{
		m_missHitFriendlyFireClientEffectTemplate->release();
		m_missHitFriendlyFireClientEffectTemplate = 0;
	}	
}

//----------------------------------------------------------------------

ClientEffectTemplate const * ShipProjectileMissHitData::getClientEffectTemplateForHitType(HitType hitType) const
{
	switch(hitType)
	{
	case HT_metal:
		return m_missHitMetalClientEffectTemplate;
		
	case HT_stone:
		return m_missHitStoneClientEffectTemplate;
		
	case HT_acid:
		return m_missHitAcidClientEffectTemplate;
		
	case HT_ice:
		return m_missHitIceClientEffectTemplate;
		
	case HT_molten:
		return m_missHitMoltenClientEffectTemplate;
		
	case HT_obsidian:
		return m_missHitObsidianClientEffectTemplate;
		
	case HT_shield:
		return m_missHitShieldClientEffectTemplate;
		
	case HT_shieldShell:
		return m_missHitShieldShellClientEffectTemplate;
		
	case HT_friendlyFire:
		return m_missHitFriendlyFireClientEffectTemplate;
		
	case HT_other:
	default:
		return m_missHitOtherClientEffectTemplate;
	}

	return NULL;
}

//----------------------------------------------------------------------

ShipProjectileMissHitData::HitType ShipProjectileMissHitData::findHitTypeForObject(Object const & object) const
{
	SharedObjectTemplate const * const sharedObjectTemplate = dynamic_cast<SharedObjectTemplate const *>(object.getObjectTemplate());
	if (sharedObjectTemplate)
	{
		switch (sharedObjectTemplate->getSurfaceType())
		{
		case SharedTangibleObjectTemplate::ST_metal:
			return HT_metal;
			
		case SharedTangibleObjectTemplate::ST_stone:
			return HT_stone;
			
		case SharedTangibleObjectTemplate::ST_acid:
			return HT_acid;
			
		case SharedTangibleObjectTemplate::ST_ice:
			return HT_ice;
			
		case SharedTangibleObjectTemplate::ST_molten:
			return HT_molten;
			
		case SharedTangibleObjectTemplate::ST_obsidian:
			return HT_obsidian;
			
		default:
			return HT_other;
		}  //lint !e788
	}

	return HT_other;
}


//----------------------------------------------------------------------

void ShipProjectileMissHitData::handleHit(Object & collider, ShipObject * ownerShip, int weaponIndex, Object const & childObject, Vector hitPoint_w, Vector hitNormal_w)
{
	Object const * const rootObject = findRootParent(childObject);
	if (NULL == rootObject)
		return;

	Object const & object = *rootObject;	

	float clientEffectScale = 1.0f;
			
	ShipObject * const playerContainingShip = Game::getPlayerContainingShip();

	ClientObject const * const clientObject = object.asClientObject();
	ShipObject const * const shipObject = (NULL != clientObject) ? clientObject->asShipObject() : NULL;
	
	HitType hitType = HT_other;

	if (NULL != shipObject)
	{
		if (ownerShip && weaponIndex != -1)
			ownerShip->onWeaponHitTarget(weaponIndex, CachedNetworkId(*shipObject));
		
		//-- If the player's ship owns the projectile, but can't attack the target, change the hit type
		if (ownerShip == playerContainingShip && !shipObject->isAttackable())
			hitType = HT_friendlyFire;
		else
		{
			Vector const & hitPoint_o = shipObject->rotateTranslate_w2o(hitPoint_w);
			
			//-- front hit
			if (hitPoint_o.z >= 0.0f && (shipObject->getComponentFlags(ShipChassisSlotType::SCST_shield_0) & ShipComponentFlags::F_shieldsFront) != 0)
			{
				hitType = HT_shield;
				//-- push back 1 meter
				hitPoint_w -= collider.getObjectFrameK_w();
			}
			//-- back hit
			if (hitPoint_o.z < 0.0f && (shipObject->getComponentFlags(ShipChassisSlotType::SCST_shield_0) & ShipComponentFlags::F_shieldsBack) != 0)
			{
				hitType = HT_shield;
				//-- push back 1 meter
				hitPoint_w -= collider.getObjectFrameK_w();
			}
			
			//-- the shield was actually hit
			if (HT_shield == hitType)
			{
				AxialBox const & tangibleExtent = shipObject->getTangibleExtent();
				const float extentRadius = tangibleExtent.getRadius();
				
				static float const s_maxShieldShellSize = 30.0f;
				
				if (extentRadius > s_maxShieldShellSize)
					clientEffectScale = extentRadius / s_maxShieldShellSize;
				else
				{
					clientEffectScale = extentRadius;
					hitType = HT_shieldShell;
					hitPoint_w = shipObject->rotateTranslate_o2w(tangibleExtent.getCenter());
				}
			}
		}
		
		//-- If the player's ship is getting hit, notify the ClientShipTargeting system
		if (shipObject == playerContainingShip && ownerShip)
		{
			ClientShipTargeting::onPlayerShipHit(ownerShip);
			
			if (ownerShip->isEnemy())
				GameMusicManager::startCombatMusic(ownerShip->getNetworkId());
		}
		
		if (ownerShip && ownerShip == playerContainingShip)
		{
			if (shipObject)
			{
				//-- Target the object if it's a ship and the player has no lookAtTarget
				if (Game::getPlayerCreature() && !Game::getPlayerCreature()->getLookAtTarget().isValid())
					Game::getPlayerCreature()->setLookAtTarget(shipObject->getNetworkId());
				
				if (shipObject->isEnemy())
				{
					GameMusicManager::startCombatMusic(shipObject->getNetworkId());
				}
			}
		}
	}
	
	if (HT_other == hitType)
	{
		hitType = findHitTypeForObject(object);
	}
	
	//-- Trigger ui feedback.
	triggerDamageFeedback(&object, ownerShip, collider.getPosition_w());
	
	Vector const & point_o = childObject.rotateTranslate_w2o(hitPoint_w);
	Vector const & normal_o = childObject.rotate_w2o(hitNormal_w);
	
	ClientEffectTemplate const * clientEffectTemplate = getClientEffectTemplateForHitType(hitType);

	if (hitType == HT_shieldShell)
	{
		//-- throttle the rate at which we can play client effects for shield shells
		if (NULL != shipObject && shipObject->isTimerShieldHitClientExpired())
		{
			const_cast<ShipObject *>(shipObject)->resetTimerShieldHitClient();
		}
	}

	if (clientEffectTemplate && !ms_disableHitEffects)
	{
		Transform clientEffectTransform;
		clientEffectTransform.setLocalFrameKJ_p(Vector::perpendicular(normal_o), normal_o);
		clientEffectTransform.setPosition_p(point_o);
		
		ClientEffect * const clientEffect = clientEffectTemplate->createClientEffect(const_cast<Object *>(&childObject), clientEffectTransform);
		clientEffect->setUniformScale(clientEffectScale);
		clientEffect->execute();
		delete clientEffect;
	}
}

//----------------------------------------------------------------------

bool ShipProjectileMissHitData::handleCollision(Object & collider, ShipObject * ownerShip, int weaponIndex, CellProperty const * lastCellProperty, Vector const & lastPosition_w, Vector const & endPosition_w, Vector & resultEndPosition_w, bool canDisplayHitEffect)
{
	CollideParameters collideParameters;
	collideParameters.setQuality(CollideParameters::Q_medium);
	
	ShipObject * const playerContainingShip = Game::getPlayerContainingShip();
	if (ownerShip != playerContainingShip)
	{
		Camera const * const camera = NON_NULL(Game::getCamera());
		if (camera)
		{
			Volume const & viewVolume = camera->getFrustumVolume();
			if (!viewVolume.intersects(camera->rotateTranslate_w2o(lastPosition_w), camera->rotateTranslate_w2o(endPosition_w)))
				collideParameters.setQuality(CollideParameters::Q_low);
		}
	}
		
	CollisionInfo result;
	if (ClientWorld::collide(lastCellProperty, lastPosition_w, endPosition_w, collideParameters, result, ms_collisionFlags, ownerShip))
	{
		Object const * const childObject = result.getObject();
		if (NULL != childObject)
		{	
			if (canDisplayHitEffect)
				handleHit(collider, ownerShip, weaponIndex, *childObject, result.getPoint(), result.getNormal());

			resultEndPosition_w = result.getPoint();
		}
		
		return true;
	}
	
	return false;
}

//----------------------------------------------------------------------

void ShipProjectileMissHitData::triggerDamageFeedback(Object const * const targetObject, ShipObject * ownerShip, Vector const & pos_w)
{
	if (targetObject)
	{
		ShipObject const * const ship = Game::getPlayerContainingShip();
		if (ship && ship == targetObject)
		{
			ShipDamageMessage shipDamageMsg(ownerShip ? ownerShip->getNetworkId() : NetworkId::cms_invalid, pos_w, 0.0f, true);
			CuiDamageManager::handleShipDamage(shipDamageMsg);
		}
	}
}

//----------------------------------------------------------------------
