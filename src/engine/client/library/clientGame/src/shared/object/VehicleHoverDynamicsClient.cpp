//======================================================================
//
// VehicleHoverDynamicsClient.cpp
// copyright (c) 2003 Sony Online Entertainment
//
//======================================================================

#include "clientGame/FirstClientGame.h"
#include "clientGame/VehicleHoverDynamicsClient.h"

#include "clientAudio/Audio.h"
#include "clientAudio/Sound3d.h"
#include "clientAudio/SoundTemplateList.h"
#include "clientGame/ClientDataFile.h"
#include "clientGame/ClientDataFile_VehicleGroundEffectData.h"
#include "clientGame/ClientDataFile_VehicleLightningEffectData.h"
#include "clientGame/ClientDataFile_VehicleThrusterSoundData.h"
#include "clientGame/ClientDataTemplateList.h"
#include "clientGame/CreatureController.h"
#include "clientGame/CreatureObject.h"
#include "clientGame/FreeChaseCamera.h"
#include "clientGame/Game.h"
#include "clientGame/PlayerCreatureController.h"
#include "clientGraphics/DebugPrimitive.h"
#include "clientGraphics/RenderWorld.h"
#include "clientObject/GameCamera.h"
#include "clientObject/HardpointObject.h"
#include "clientParticle/ParticleEffectAppearance.h"
#include "clientParticle/LightningAppearance.h"
#include "clientSkeletalAnimation/SkeletalAppearance2.h"
#include "sharedCollision/BoxExtent.h"
#include "sharedFoundation/PersistentCrcString.h"
#include "sharedFoundation/TemporaryCrcString.h"
#include "sharedObject/Appearance.h"
#include "sharedObject/AppearanceTemplate.h"
#include "sharedObject/AppearanceTemplateList.h"
#include "sharedTerrain/TerrainObject.h"
#include <list>

//======================================================================

namespace
{
	inline CreatureObject * getMotorCreature (Object & vehicle)
	{
		return safe_cast<CreatureObject *>(vehicle.getParent ());
	}
	
	typedef stdlist<Transform>::fwd TransformList;
	TransformList s_crumbs;
	typedef stdlist<std::pair <Transform, float> >::fwd TransformPopupList;
	TransformPopupList s_crumbsPopup;
	
	const int s_numCrumbs      = 120;
	const int s_numCrumbsPopup = 60;
	
	typedef VehicleHoverDynamicsClient::ParticleEffectVector ParticleEffectVector;
	typedef VehicleHoverDynamicsClient::SoundIdVector SoundIdVector;
	
	namespace HardpointNames
	{
		static const TemporaryCrcString s_hp_engine_0        ("engine_0", true);
		static const TemporaryCrcString s_hp_engine_1        ("engine_1", true);
		static const TemporaryCrcString s_hp_engine_2        ("engine_2", true);
		static const TemporaryCrcString s_hp_engine_3        ("engine_3", true);
		static const TemporaryCrcString s_hp_engine_sound_0  ("engine_sound_0", true);
		static const TemporaryCrcString s_hp_ground_effect_0 ("ground_effect_0", true);
	}
}

//----------------------------------------------------------------------

VehicleHoverDynamicsClient::GroundEffectClientData::GroundEffectClientData (Object & parent, const VehicleGroundEffectData & _vged) :
obj       (0),
soundId   (),
transform (),
pea       (0),
old_obj   (0),
old_pea   (0),
baseEffectScale (1.0f),
vged      (&_vged)
{	
	if (!vged->m_hardpointName.empty ())
		parent.getAppearance ()->findHardpoint(TemporaryCrcString (vged->m_hardpointName.c_str (), true), transform);
	else
		transform = Transform::identity;
}

//----------------------------------------------------------------------

void VehicleHoverDynamicsClient::GroundEffectClientData::manageEffectOnOff (Object & parent, bool valid)
{
	if (old_obj && old_pea)
	{
		if (old_pea->isDeletable ())
		{
			old_obj->kill ();
			old_obj = 0;
			old_pea = 0;
		}
	}

	if (!valid)
	{
		if (obj)
		{
			Audio::stopSound (soundId, 3.0f);

			//-- don't destroy the old ground effect, just disable it and wait for it to become deletable
			if (pea)
				pea->setEnabled (false);

			old_obj = obj;
			old_pea = pea;

			obj = 0;
			pea = 0;
		}
	}
	else
	{
		if (!obj)
		{
			if (old_obj)
			{
				obj = old_obj;
				pea = old_pea;
				if (pea)
					pea->setEnabled (true);
			}
			else
			{
				obj = new Object;
				if (vged->m_appearanceTemplate)
				{
					Appearance * const app = vged->m_appearanceTemplate->createAppearance ();
					pea = ParticleEffectAppearance::asParticleEffectAppearance(app);
					
					if (pea)
					{
						pea->setAutoDelete (false);

						const BoxExtent * const box = dynamic_cast<const BoxExtent *>(parent.getAppearance ()->getExtent ());
						if (box)
						{
							//-- scale effect to be as big across as the vehicle
							//-- the ground effect Particle effects must be normalized at 1 meter radius
							const float boxWidth_x = box->getWidth ();
							baseEffectScale = std::max (0.5f, std::min (boxWidth_x * 0.5f, 1.5f));
							pea->setUniformScale (baseEffectScale);
						}
					}
					
					if (app)
						obj->setAppearance (app);
				}
				
				RenderWorld::addObjectNotifications (*obj);
				
				parent.addChildObject_o (obj);			
			}

			if (!vged->m_soundTemplateName.empty ())
				soundId = Audio::attachSound (vged->m_soundTemplateName.c_str (), obj, 0);
		}
	}
}

//----------------------------------------------------------------------

void VehicleHoverDynamicsClient::GroundEffectClientData::manageEffect (Object & parent, float speedPercent)
{
	
	TerrainObject * const terrain = TerrainObject::getInstance ();
	
	if (!terrain)
	{
		manageEffect (parent, false);
		return;
	}
	
	Transform t_o2w = parent.getTransform_o2w ().rotateTranslate_l2p (transform);

	Vector effectPos_w = t_o2w.getPosition_p ();

	bool isWater = false;

	float height = 0.0f;
	if (terrain->getHeight (effectPos_w, height))
	{
		float waterHeight = 0.0f;
		TerrainGeneratorWaterType waterType;
		if (terrain->getWaterHeight (effectPos_w, waterHeight, waterType))
		{
			if (waterHeight > height)
			{
				isWater = (waterType == TGWT_water || waterType == TGWT_invalid); 
				height = waterHeight;
			}
		}
	}
	else
		return;

	float heightDiff = effectPos_w.y - height;

	const float parentRadius = std::max (1.0f, parent.getAppearanceSphereRadius ());
	const float heightRatio  = std::min (1.0f, std::max (0.0f, (3.0f - heightDiff / parentRadius) * 0.33f));

	const float isValid = (vged->m_isWaterEffect && isWater) || (!vged->m_isWaterEffect && !isWater);
	manageEffectOnOff (parent, isValid);

	if (obj)
	{
		const static float GROUND_EFFECT_Y_OFFSET = 0.05f;
		t_o2w.move_p (Vector::unitY * (-heightDiff + GROUND_EFFECT_Y_OFFSET));
		const Transform & t_o2p = parent.getTransform_o2w ().rotateTranslate_p2l (t_o2w);

		obj->setTransform_o2p (t_o2p);

		Audio::setSoundVolume (soundId, heightRatio * speedPercent);
		if (pea)
		{
			pea->setLodBias (heightRatio);
			pea->setUniformScale (baseEffectScale * (0.5f + (speedPercent * 0.5f)));
		}
	}
}

//----------------------------------------------------------------------

VehicleHoverDynamicsClient::VehicleHoverDynamicsClient (Object* newOwner, float yaw, float hoverHeight, const char * clientDataFilename) :
VehicleHoverDynamics (newOwner, yaw, hoverHeight),
m_engineParticleEffects (new ParticleEffectVector),
m_engineSounds          (),
m_engineSoundsDamaged   (),
m_lastDamageLevel       (0.0f),
m_clientDataFile        (0),
m_hasAltered            (0),
m_groundEffects         (new GroundEffectClientDataVector),
m_lastSpeedPercent      (0.0f),
m_lastHeadTurnPercent   (0.0f),
m_lightningEffects      (new LightningEffectDataVector)
{
	m_engineSounds.m_dynamics = this;
	m_engineSoundsDamaged.m_dynamics = this;

//	const std::string & appearanceName = newOwner->getAppearance ()->getAppearanceTemplateName ();
	
	if (clientDataFilename && *clientDataFilename)
	{
		m_clientDataFile = ClientDataTemplateList::fetch (clientDataFilename);
		if (m_clientDataFile)
		{
			Object * const vehicleObject = getOwner ();
			CreatureObject * const motorCreature = getMotorCreature (*vehicleObject);
			VehicleThrusterSoundData vtsd;
			ClientDataFile::ObjectVector ov;
			if(motorCreature && motorCreature->getGameObjectType() == SharedObjectTemplate::GOT_vehicle)
			{
				m_clientDataFile->apply (motorCreature);
				m_clientDataFile->initVehicleThrusters (*motorCreature, ov, vtsd);
			}
			else
			{
				m_clientDataFile->apply (newOwner);
				m_clientDataFile->initVehicleThrusters (*newOwner, ov, vtsd);
			}

			updateSoundData (vtsd);

			m_engineParticleEffects->reserve (ov.size ());
			for (ClientDataFile::ObjectVector::const_iterator it = ov.begin (); it != ov.end (); ++it)
			{
				ParticleEffectAppearance * const pea = ParticleEffectAppearance::asParticleEffectAppearance((*it)->getAppearance ());
				if (pea)
					m_engineParticleEffects->push_back (pea);
			}


			const ClientDataFile::VehicleGroundEffectDataList * const vgedl = m_clientDataFile->getVehicleGroundEffectsData ();

			if (vgedl)
			{
				for (ClientDataFile::VehicleGroundEffectDataList::const_iterator it = vgedl->begin (); it != vgedl->end (); ++it)
				{
					const VehicleGroundEffectData * const vged = *it;
					Object * const vehicleObject = getOwner ();
					CreatureObject * const motorCreature = getMotorCreature (*vehicleObject);
					if(motorCreature && motorCreature->getGameObjectType() == SharedObjectTemplate::GOT_vehicle)
					{
						const GroundEffectClientData  gecd(*motorCreature, *vged);
						m_groundEffects->push_back (gecd);
					}
					else
					{
						const GroundEffectClientData  gecd(*getOwner (), *vged);
						m_groundEffects->push_back (gecd);
					}
				}
			}

			const ClientDataFile::VehicleLightningEffectDataList * const lightningDataList = m_clientDataFile->getVehicleLightningEffectsData();
			if (lightningDataList)
			{
				for (ClientDataFile::VehicleLightningEffectDataList::const_iterator data = lightningDataList->begin(); data != lightningDataList->end(); ++data)
				{
					VehicleLightningEffectData const * const lightningData = *data;

					m_lightningEffects->push_back(new LightningEffectData(*newOwner, lightningData->m_startHardpointName.c_str(), 
						lightningData->m_endHardpointName.c_str(), lightningData->m_appearanceTemplateName.c_str()));

				}
			}
		}
	}
}

//----------------------------------------------------------------------

VehicleHoverDynamicsClient::~VehicleHoverDynamicsClient ()
{
	delete m_engineParticleEffects;
	m_engineParticleEffects = 0;
	delete m_groundEffects;
	m_groundEffects = 0;


	if(m_lightningEffects)
	{
		for(LightningEffectDataVector::iterator i = m_lightningEffects->begin(); i != m_lightningEffects->end(); ++i)
		{
			FATAL((*i == NULL),("trying to delete a null pointer"));
			delete *i;
			*i = NULL;
		}

		delete m_lightningEffects;
		m_lightningEffects = NULL;
	}

	if (m_clientDataFile)
	{
		m_clientDataFile->releaseReference ();
		m_clientDataFile = 0;
	}
}

//----------------------------------------------------------------------

float VehicleHoverDynamicsClient::alter(float elapsedTime)
{
	Object * const vehicleObject = getOwner ();
	const CreatureController * motorCreatureController = 0;
	CreatureObject * const motorCreature = getMotorCreature (*vehicleObject);
	
	if (m_hasAltered == 2)
	{		
		if (motorCreature)
		{
			//-- get custom vars
			m_speedMin      = motorCreature->getMinimumSpeed (true);
			m_speedMax      = motorCreature->getMaximumSpeed (true);
			m_accelMin      = motorCreature->getMaximumAcceleration (m_speedMin, true);
			m_accelMax      = motorCreature->getMaximumAcceleration (m_speedMax, true);
			m_turnRateMin   = convertDegreesToRadians (motorCreature->getMaximumTurnRate (m_speedMin, true));
			m_turnRateMax   = convertDegreesToRadians (motorCreature->getMaximumTurnRate (m_speedMax, true));
		}
	}

	++m_hasAltered;

	{
		motorCreatureController = motorCreature ? safe_cast<const CreatureController *>(motorCreature->getController ()) : 0;
		
		const CreatureObject * const player = Game::getPlayerCreature ();
		
		if (player)
		{
			//-- player is riding in vehcile
			//-- @todo: only if player is driver, use the player's controller to get the speed
			
			if (player->getParent () == vehicleObject || 
				player->getContainedBy () == vehicleObject || 
				player->getContainedBy () == static_cast<const Object *>(motorCreature))
				motorCreatureController = safe_cast<const CreatureController *>(player->getController ());
		}
				
		if (motorCreatureController)
		{
			m_currentSpeed = motorCreatureController->getCurrentSpeed ();
		}
	}
	
	if (motorCreature)
	{
		const int healthmax = motorCreature->getMaxHitPoints ();
		const int health    = healthmax - motorCreature->getDamageTaken ();
		
		if (healthmax > 0)
		{
			//-- check damageLevel
			const float currentDamageLevel = 1.0f - (static_cast<float>(health) / healthmax);
			
			if (currentDamageLevel != m_lastDamageLevel)
			{
				Object* particleOwner = vehicleObject;
				CreatureObject * const motorCreature = getMotorCreature (*vehicleObject);
				
				if(motorCreature && motorCreature->getGameObjectType() == SharedObjectTemplate::GOT_vehicle)
				{
					particleOwner = motorCreature;
				}

				if (m_clientDataFile)
					m_clientDataFile->applyDamage (particleOwner, motorCreature->hasCondition (TangibleObject::C_onOff), m_lastDamageLevel, currentDamageLevel);

				static ClientDataFile::ObjectVector ov;
				ov.clear ();
				
				{
					ov.reserve (m_engineParticleEffects->size ());
					for (ParticleEffectVector::const_iterator it = m_engineParticleEffects->begin (); it != m_engineParticleEffects->end (); ++it)
					{
						ParticleEffectAppearance * const pea = *it;
						ov.push_back (pea->getOwner ());
					}
				}

				VehicleThrusterSoundData vtsd;

				if (m_clientDataFile && m_clientDataFile->updateVehicleThrusters (*particleOwner, m_lastDamageLevel, currentDamageLevel, ov, vtsd))
				{
					updateSoundData (vtsd);
					m_engineParticleEffects->clear ();
					m_engineParticleEffects->reserve (ov.size ());
					for (ClientDataFile::ObjectVector::const_iterator it = ov.begin (); it != ov.end (); ++it)
					{
						ParticleEffectAppearance * const pea = ParticleEffectAppearance::asParticleEffectAppearance((*it)->getAppearance ());
						if (pea)
							m_engineParticleEffects->push_back (pea);
					}
				}
				
				m_lastDamageLevel = currentDamageLevel;
			}
		}
	}

	const float retval = VehicleHoverDynamics::alter (elapsedTime);
	
	FreeChaseCamera::ms_vehicleCameraOffsetY = 0.0f;
	
	float speedPercent = 0.0f;
	
	if (m_speedMax > 0.0f && m_speedMax > m_speedMin)
		speedPercent = std::max (0.0f, std::min (1.0f, (m_currentSpeed - m_speedMin) / (m_speedMax - m_speedMin)));
	
	{
		for (ParticleEffectVector::const_iterator it = m_engineParticleEffects->begin (); it != m_engineParticleEffects->end (); ++it)
		{
			ParticleEffectAppearance * const pea = *it;
			const float scale = linearInterpolate (0.1f, 1.0f, speedPercent);
			pea->setUniformScale (scale);
		}
	}
	
	if (motorCreatureController)
		updateSounds (motorCreatureController->getDesiredSpeed (), elapsedTime, speedPercent);
		
	m_lastSpeedPercent = speedPercent;
	
	updateGroundEffects (elapsedTime, speedPercent);

	updateLightningEffects (elapsedTime);

	//-- update player head turning

	if (motorCreature)
	{
		CreatureObject * const driver = motorCreature->getRiderDriverCreature ();
		if (driver)
		{
			SkeletalAppearance2 * const skelApp = driver->getAppearance ()->asSkeletalAppearance2 ();
			if (skelApp)
			{
				const float turnRatePercentLastFrame  = getTurnRatePercentLastFrame ();
				const float currentHeadTurningPercent = linearInterpolate (m_lastHeadTurnPercent, turnRatePercentLastFrame, std::min (1.0f, elapsedTime));
				Vector targetPos = Vector::unitZ;
				targetPos.x = -currentHeadTurningPercent;
				skelApp->setTargetPosition_w (driver->getPosition_w () + driver->rotate_o2w (targetPos));

				m_lastHeadTurnPercent = currentHeadTurningPercent;
			}

		}
	}

	return retval;
}

//----------------------------------------------------------------------

void VehicleHoverDynamicsClient::updateGroundEffects (float elapsedTime, float speedPercent)
{

	UNREF (elapsedTime);
	UNREF (speedPercent);

	for (GroundEffectClientDataVector::iterator it = m_groundEffects->begin (); it != m_groundEffects->end (); ++it)
	{
		GroundEffectClientData & gecd = *it;
		Object * const vehicleObject = getOwner ();
		CreatureObject * const motorCreature = getMotorCreature (*vehicleObject);
		if(motorCreature && motorCreature->getGameObjectType() == SharedObjectTemplate::GOT_vehicle)
		{
			gecd.manageEffect (*motorCreature, speedPercent);
		}
		else
		{
			gecd.manageEffect (*getOwner (), speedPercent);
		}
	}
}

//----------------------------------------------------------------------

void VehicleHoverDynamicsClient::updateSounds (float desiredSpeed, float elapsedTime, float speedPercent)
{
	m_engineSounds.update (elapsedTime, desiredSpeed, speedPercent);
	m_engineSoundsDamaged.update (elapsedTime, desiredSpeed, speedPercent);
}

//----------------------------------------------------------------------

void VehicleHoverDynamicsClient::showDebugBoxHoverPlane (const Vector & lookAhead) const
{
	UNREF (lookAhead);
#if _DEBUG
	const Camera * const camera = Game::getCamera ();
	
	if (camera)
	{
		const Object * const vehicleObject = getOwner ();
		const BoxExtent * const box = dynamic_cast<const BoxExtent *>(vehicleObject->getAppearance ()->getExtent ());
		AxialBox abox = box->getBoundingBox ();
		Transform t = vehicleObject->getTransform_o2w ();
		BoxDebugPrimitive * const bdp0 = new BoxDebugPrimitive (BoxDebugPrimitive::S_z, t, abox);
		bdp0->setColor (PackedArgb::solidMagenta);
		camera->addDebugPrimitive (bdp0);
		
		t.move_p (lookAhead);
		
		BoxDebugPrimitive * const bdp1 = new BoxDebugPrimitive (BoxDebugPrimitive::S_z, t, abox);
		bdp1->setColor (PackedArgb::solidMagenta);
		camera->addDebugPrimitive (bdp1);
	}
#endif
}

//----------------------------------------------------------------------

void VehicleHoverDynamicsClient::showDebugBoxHeight     (const Vector & lookAhead) const
{
	UNREF (lookAhead);

#if _DEBUG
	const Camera * const camera = Game::getCamera ();
	
	if (camera)
	{
		const Object * const vehicleObject = getOwner ();
		const BoxExtent * const box = dynamic_cast<const BoxExtent *>(vehicleObject->getAppearance ()->getExtent ());
		AxialBox abox = box->getBoundingBox ();
		Transform t = vehicleObject->getTransform_o2w ();
		BoxDebugPrimitive * const bdp0 = new BoxDebugPrimitive (BoxDebugPrimitive::S_z, t, abox);
		bdp0->setColor (PackedArgb::solidWhite);
		camera->addDebugPrimitive (bdp0);
		
		t.move_p (lookAhead);					
		BoxDebugPrimitive * const bdp1 = new BoxDebugPrimitive (BoxDebugPrimitive::S_z, t, abox);
		bdp1->setColor (PackedArgb::solidYellow);
		camera->addDebugPrimitive (bdp1);
	}
#endif
}

//----------------------------------------------------------------------

void VehicleHoverDynamicsClient::updateCrumbTrail () const
{
#if _DEBUG
	const Object * const vehicleObject = getOwner ();
	const Transform & vt = vehicleObject->getTransform_o2w ();
	if (s_crumbs.empty () || !vt.approximates (s_crumbs.back (), 0.01f, 0.01f))
	{
		s_crumbs.push_back (vt);
		if (s_crumbs.size () > s_numCrumbs)
			s_crumbs.pop_front ();
	}
	
	if (m_distancePopupThisFrame > 0.0f)
	{
		s_crumbsPopup.push_back (std::make_pair (vt, m_distancePopupThisFrame));
		if (s_crumbsPopup.size () > s_numCrumbsPopup)
			s_crumbsPopup.pop_front ();
	}
	
	Transform t = Transform::identity;
	
	const Camera * const camera = Game::getCamera ();
	
	if (camera)
	{
		{
			for (TransformList::const_iterator it = s_crumbs.begin (); it != s_crumbs.end (); ++it)
			{
				const Transform & t = *it;
				FrameDebugPrimitive * const fdp = new FrameDebugPrimitive (BoxDebugPrimitive::S_z, t, 1.0f);
				camera->addDebugPrimitive (fdp);
			}
		}
		
		for (TransformPopupList::const_iterator it = s_crumbsPopup.begin (); it != s_crumbsPopup.end (); ++it)
		{
			const Transform & t = (*it).first;
			const float & distance  = (*it).second;
			
			SphereDebugPrimitive * const sdp = new SphereDebugPrimitive (BoxDebugPrimitive::S_z, t, Vector::unitY * (-distance), distance, 4, 4);
			sdp->setColor (PackedArgb::solidYellow);
			camera->addDebugPrimitive (sdp);
		}
		
	}
#endif
}

//----------------------------------------------------------------------

void VehicleHoverDynamicsClient::SoundIdData::transitionToIntermediateState ()
{
	Audio::stopSound (getSoundIdByState (getOppositeState (m_stateCurrent)),   0.0f);
	Audio::stopSound (getSoundIdByState (m_stateCurrent),  3.0f);

	const SoundState transitionState = getIntermediateTransitionState (m_stateCurrent);
	fadeSoundIn (getSoundIdByState (transitionState), 1.0f, 0);
	m_stateCurrent = transitionState;
}

//----------------------------------------------------------------------

void VehicleHoverDynamicsClient::SoundIdData::transitionToFinalState ()
{
	bool transitionReady = false;
	
	int currentSoundTime = 0;
	int totalSoundTime   = 0;
	int soundTimeRemaining = 0;

//	if (m_stateCurrent != getIntermediateTransitionState (s))
//		return;
	
	SoundId & transitionSoundId = getSoundIdByState (m_stateCurrent);

	Sound2 * const s2 = Audio::getSoundById (transitionSoundId);

	if (!Audio::isSoundPlaying (transitionSoundId) || (s2 && s2->isInfiniteLooping ()))
		transitionReady = true;
	else if (Audio::isSampleForSoundIdPlaying (transitionSoundId) && Audio::getCurrentSoundTime (transitionSoundId, totalSoundTime, currentSoundTime) && totalSoundTime > 0)
	{
		getTotalTimeHolder (m_stateCurrent) = totalSoundTime;
		soundTimeRemaining = totalSoundTime - currentSoundTime;
		if (soundTimeRemaining < 2000)
		{
			transitionReady = true;
		}
	}
	
	if (transitionReady)
	{
		SoundId & targetSoundId = getSoundIdByState (m_stateTarget);

		const float fadeInTime = soundTimeRemaining * 0.001f;
		Audio::stopSound (getSoundIdByState (getOppositeState (m_stateTarget)),  0.0f);
		Audio::stopSound (transitionSoundId, fadeInTime);
		fadeSoundIn (targetSoundId, fadeInTime * 0.7f, 0);
		m_stateCurrent = m_stateTarget;
	}
}

//----------------------------------------------------------------------

void VehicleHoverDynamicsClient::SoundIdData::transitionToReverseState ()
{
	SoundId & firstSoundId       = getSoundIdByState (m_stateCurrent);
	const SoundState secondState = getOppositeState (m_stateCurrent);
	SoundId & secondSoundId      = getSoundIdByState (secondState);

	int firstCurrentSoundTime = 0;
	int firstTotalSoundTime   = 0;
	if (Audio::isSampleForSoundIdPlaying (firstSoundId) && Audio::getCurrentSoundTime (firstSoundId, firstTotalSoundTime, firstCurrentSoundTime) && firstTotalSoundTime > 0)
	{
		getTotalTimeHolder (m_stateCurrent) = firstTotalSoundTime;
		
		float firstSoundTimePercent   = (firstTotalSoundTime > 0) ? static_cast<float>(firstCurrentSoundTime) / static_cast<float>(firstTotalSoundTime) : 0.0f;
		
		int & secondTotalTime = getTotalTimeHolder (secondState);
		if (secondTotalTime == 0)
			Audio::getTotalSoundTime (secondSoundId, secondTotalTime);
		
		int secondCurrentSoundTime = secondTotalTime - static_cast<int>(firstSoundTimePercent * secondTotalTime);
		secondCurrentSoundTime = secondCurrentSoundTime / 2;
		secondCurrentSoundTime = std::min (static_cast<int>(secondTotalTime * 0.75f), secondCurrentSoundTime);

		/*
		int speedBasedsecondCurrentSoundTime = static_cast<int>(speedPercent * m_secondTotalSoundTime);
		if (speedBasedsecondCurrentSoundTime < secondCurrentSoundTime)
			secondCurrentSoundTime = (speedBasedsecondCurrentSoundTime + secondCurrentSoundTime) / 2;
		*/

		const float fadeInTime = std::min (1.0f, firstSoundTimePercent * 1.5f);
		
		Audio::stopSound (m_idle,       std::max (1.0f, fadeInTime * 2.0f));
		Audio::stopSound (m_run,        std::max (1.0f, fadeInTime * 2.0f));
		Audio::stopSound (firstSoundId, std::max (1.0f, fadeInTime * 2.0f));
		fadeSoundIn (secondSoundId, fadeInTime, secondCurrentSoundTime);
		m_stateCurrent = secondState;
	}
}

//----------------------------------------------------------------------

void VehicleHoverDynamicsClient::SoundIdData::update (float elapsedTime, float desiredSpeed, float speedPercent)
{
	UNREF (speedPercent);
	if (m_fadeInSoundId.isValid ())
	{
		if (!m_fadeInLoaded)
		{
			//				s_soundFadeInOffset += elapsedTime;
			
			int total = 0;
			int current = 0;
			
			if (Audio::isSampleForSoundIdPlaying (m_fadeInSoundId) && 
				Audio::getCurrentSoundTime (m_fadeInSoundId, total, current) && total > 0)
			{
				int offset = std::min (total, static_cast<int>(m_fadeInOffset * 1000.0f));
				Audio::setCurrentSoundTime (m_fadeInSoundId, offset);
				m_fadeInLoaded = true;
			}
		}
		
		m_fadeInVolume = std::min (1.0f, m_fadeInVolume + (m_fadeInRate * elapsedTime));
		Audio::setSoundVolume (m_fadeInSoundId, m_fadeInVolume);
		
		if (m_fadeInVolume >= 1.0f)
		{
			m_fadeInSoundId.invalidate ();
		}
	}
	
	if (desiredSpeed > m_dynamics->getSpeedMax () * 0.1f)
		m_stateTarget = SS_run;
	else
		m_stateTarget = SS_idle;
	
	if (m_stateTarget == SS_run)
	{
		if (m_stateCurrent == SS_idle)
		{
			transitionToIntermediateState ();
		}
		else if (m_stateCurrent == SS_accel)
		{
			transitionToFinalState ();
		}
		else if (m_stateCurrent == SS_decel)
		{
			transitionToReverseState ();
		}
		else if (!m_run.getPath ().isEmpty () && !Audio::isSoundPlaying (m_run))
		{
			fadeSoundIn (m_run, 1.0f, 0);
		}
	}
	//-- transition to idling
	else if (m_stateTarget == SS_idle)
	{
		if (m_stateCurrent == SS_run)
		{
			transitionToIntermediateState ();
		}
		else if (m_stateCurrent == SS_decel)
		{
			transitionToFinalState ();
		}
		
		else if (m_stateCurrent == SS_accel)
		{	
			transitionToReverseState ();
		}
		else if (!m_idle.getPath ().isEmpty () && !Audio::isSoundPlaying (m_idle))
		{
			fadeSoundIn (m_idle, 1.0f, 0);
		}
	}
}

//----------------------------------------------------------------------

void VehicleHoverDynamicsClient::SoundIdData::fadeSoundIn (SoundId & id, float fadeInTime, int offsetStartMs)
{
	if (id.getPath ().isEmpty ())
		return;

	m_fadeInOffset = offsetStartMs * 0.001f;
	m_fadeInLoaded = false;

	m_fadeInVolume = 0.0f;

	//-- if the sound is already fading out, start the volume at that fade point rather than resetting to zero and fading in from there

	Sound2d * const s2 = dynamic_cast<Sound2d *>(Audio::getSoundById (id));
	if (s2)
	{
		m_fadeInVolume = s2->getFadeoutVolume ();
		fadeInTime *= (1.0f - m_fadeInVolume);
	}

	id  = Audio::attachSound (id.getPath ().getString (), m_dynamics->getOwner (), "engine_sound_0");
	
	m_fadeInSoundId = id;
	if (fadeInTime > 0.0f)
	{
		m_fadeInRate = RECIP (fadeInTime);
		Audio::setSoundVolume (m_fadeInSoundId, 0.0f);
	}
	else
	{
		m_fadeInRate = 0.0f;
		m_fadeInVolume = 1.0f;
		Audio::setSoundVolume (m_fadeInSoundId, 1.0f);
	}
}

//----------------------------------------------------------------------

void VehicleHoverDynamicsClient::updateSoundData (const VehicleThrusterSoundData & vtsd)
{
	updateSoundId (m_engineSounds, m_engineSounds.m_idle,         vtsd.m_idle);
	updateSoundId (m_engineSounds, m_engineSounds.m_accel,        vtsd.m_accel);
	updateSoundId (m_engineSounds, m_engineSounds.m_decel,        vtsd.m_decel);
	updateSoundId (m_engineSounds, m_engineSounds.m_run,          vtsd.m_run);
	updateSoundId (m_engineSoundsDamaged, m_engineSoundsDamaged.m_idle,  vtsd.m_damageIdle);
	updateSoundId (m_engineSoundsDamaged, m_engineSoundsDamaged.m_accel, vtsd.m_damageAccel);
	updateSoundId (m_engineSoundsDamaged, m_engineSoundsDamaged.m_decel, vtsd.m_damageDecel);
	updateSoundId (m_engineSoundsDamaged, m_engineSoundsDamaged.m_run,   vtsd.m_damageRun);
}

//----------------------------------------------------------------------

void VehicleHoverDynamicsClient::updateSoundId (SoundIdData & data, SoundId & soundId, const std::string & soundTemplateName)
{
	if (soundId.isValid () && data.m_fadeInSoundId.getPath () == soundId.getPath ())
	{
		if (!data.m_fadeInLoaded || !Audio::isSampleForSoundIdPlaying (data.m_fadeInSoundId))
		{
			const int current = static_cast<int>(data.m_fadeInOffset * 1000.0f);
			Audio::stopSound (soundId, 3.0f);
			soundId = SoundId (0, soundTemplateName.c_str ());
			data.fadeSoundIn (soundId, 1.0f, current);
			return;
		}

		int total = 0;
		int current = 0;
		
		if (!Audio::getCurrentSoundTime (data.m_fadeInSoundId, total, current) || total <= 0)
			current =0;
		
		Audio::stopSound (soundId, 3.0f);
		soundId = SoundId (0, soundTemplateName.c_str ());
		data.fadeSoundIn (soundId, 1.0f, current);
		return;
	}

	if (Audio::isSampleForSoundIdPlaying (soundId))
	{
		Audio::stopSound (soundId, 3.0f);
		soundId = SoundId (0, soundTemplateName.c_str ());
		data.fadeSoundIn (soundId, 1.0f, 0);
		return;
	}

	Audio::stopSound (soundId, 2.0f);
	soundId = SoundId (0, soundTemplateName.c_str ());
}

//----------------------------------------------------------------------

void VehicleHoverDynamicsClient::updateLightningEffects (float elapsedTime)
{
	if(m_lightningEffects)
	{
		for(LightningEffectDataVector::iterator i = m_lightningEffects->begin(); i != m_lightningEffects->end(); ++i)
		{
			(*i)->update(elapsedTime);
		}
	}
}

//----------------------------------------------------------------------

VehicleHoverDynamicsClient::LightningEffectData::LightningEffectData (Object & parent, char const * const startHardpoint, char const * const endHardpoint, char const * const appearancePath)
:	m_parentObject (&parent),
	m_lightningObject (NULL),
	m_startHardpoint (startHardpoint, true),
	m_endHardpoint (endHardpoint, true)
{
	AppearanceTemplate const * appearanceTemplate = AppearanceTemplateList::fetch(appearancePath);
	
	DEBUG_WARNING((appearanceTemplate == NULL), ("VehicleHoverDynamicsClient::LightningEffectData could not find lighning appearance %s", appearancePath));
	
	if (appearanceTemplate != NULL)
	{
		//set the appearance and get it running
		LightningAppearance * const newBoltAppearance = appearanceTemplate->createAppearance()->asLightningAppearance();

		DEBUG_WARNING((newBoltAppearance == NULL), ("VehicleHoverDynamicsClient::LightningEffectData appearance %s could not be made as lightning appearance", appearancePath));

		if (newBoltAppearance != NULL)
		{
			m_lightningObject = new Object();
			m_lightningObject->setPosition_p(m_parentObject->getPosition_p()); //UseLightningAction did this, seems like it makes debugging easier//TODO
			m_lightningObject->setAppearance(newBoltAppearance);
			
			newBoltAppearance->setEnabled(true);
			newBoltAppearance->setPaused(false);

			//add notifications and stuff. Using the same ones they use in UseLightningAction
			m_lightningObject->addNotification(ClientWorld::getIntangibleNotification());
			RenderWorld::addObjectNotifications(*m_lightningObject);
			m_lightningObject->addToWorld();
		}

		AppearanceTemplateList::release(appearanceTemplate);
	}
}

//----------------------------------------------------------------------

VehicleHoverDynamicsClient::LightningEffectData::~LightningEffectData()
{
	if (m_lightningObject != NULL)
	{
		m_lightningObject->kill();
		m_lightningObject = NULL;
	}
}

//----------------------------------------------------------------------

void VehicleHoverDynamicsClient::LightningEffectData::update(float deltaTimeSeconds)
{
	UNREF(deltaTimeSeconds);
	Object const *const vehicleObject = m_parentObject.getPointer();
	if(vehicleObject == NULL)
	{
		DEBUG_WARNING(true,("LightningEffectData has lost its parent object"));
		return;
	}

	// grab the appearance
	NOT_NULL(m_lightningObject);
	Appearance * const appearance = m_lightningObject->getAppearance();
	NOT_NULL(appearance);
	LightningAppearance * const lightningAppearance = appearance->asLightningAppearance();
	NOT_NULL(lightningAppearance);

	//get the positions

	Vector boltStart_w = vehicleObject->getPosition_p();
	Vector boltEnd_w = vehicleObject->getPosition_p();

	Transform startTransform;
	Transform endTransform;

	NOT_NULL(vehicleObject);
	Appearance const * const vehicleAppearance = vehicleObject->getAppearance();
	NOT_NULL(vehicleAppearance);

	if(vehicleAppearance->findHardpoint(m_startHardpoint, startTransform)
		&& vehicleAppearance->findHardpoint(m_endHardpoint, endTransform))
	{
		boltStart_w = vehicleObject->rotateTranslate_o2w(startTransform.getPosition_p());
		boltEnd_w = vehicleObject->rotateTranslate_o2w(endTransform.getPosition_p());
	}


	//make sure the bolt is in same cell as the vehicle
	m_lightningObject->setParentCell(vehicleObject->getParentCell());
	m_lightningObject->setPosition_w(vehicleObject->getPosition_w());

	//finally set the position of the the bolt
	static int boltIndex = 0;
	lightningAppearance->setPosition_w(boltIndex, boltStart_w, boltEnd_w);
}

//======================================================================
