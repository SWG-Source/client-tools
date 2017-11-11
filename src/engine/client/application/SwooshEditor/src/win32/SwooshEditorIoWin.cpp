// ============================================================================
//
// SwooshEditorIoWin.cpp
// Copyright Sony Online Entertainment
//
// ============================================================================

#include "FirstSwooshEditor.h"
#include "SwooshEditorIoWin.h"

#include "clientAnimation/PlaybackScript.h"
#include "clientAnimation/PlaybackScriptManager.h"
#include "clientGame/ClientCombatActionInfo.h"
#include "clientGame/ClientCombatPlaybackManager.h"
#include "clientGame/ClientWorld.h"
#include "clientGame/CreatureController.h"
#include "clientGame/CreatureObject.h"
#include "clientGame/FreeCamera.h"
#include "clientGame/Game.h"
#include "clientGame/GroundScene.h"
#include "clientGraphics/Graphics.h"
#include "clientGraphics/RenderWorld.h"
#include "clientGraphics/ShaderTemplateList.h"
#include "clientObject/GameCamera.h"
#include "clientParticle/SwooshAppearance.h"
#include "clientParticle/SwooshAppearanceTemplate.h"
#include "clientUserInterface/CuiInventoryManager.h"
#include "MainWindow.h"
#include "sharedDebug/PerformanceTimer.h"
#include "sharedFoundation/Crc.h"
#include "sharedFoundation/Os.h"
#include "sharedObject/AppearanceTemplate.h"
#include "sharedObject/AppearanceTemplateList.h"
#include "sharedObject/Object.h"
#include "sharedObject/ObjectTemplateList.h"
#include "sharedObject/World.h"
#include "sharedTerrain/TerrainObject.h"

//=============================================================================
//
// SwooshEditorIoWin
//
//=============================================================================

//-----------------------------------------------------------------------------
SwooshEditorIoWin::SwooshEditorIoWin(MainWindow *particleEditor)
 : IoWin ("SwooshEditorIoWin")
 , m_object1(NULL)
 , m_object2(NULL)
 , m_objectTransform(Transform::identity)
 , m_mainWindow(particleEditor)
 , m_timeOfDayCycle(false)
 , m_objectRotationSpeed(3.0f)
 , m_defenderObject(NULL)
 , m_weaponObject(NULL)
 , m_gameUpdate(0)
 , m_animationName()
 , m_trailFlags(0)
 , m_defenderDistance(0.0f)
 , m_weaponName()
 , m_pauseAfterEachAnimation(true)
 , m_pauseAfterEachAnimationTimer(0.0f)
 , m_nextAppearanceTemplate(NULL)
 , m_appearanceChanged(false)
 , m_referenceSwoosh(RS_none)
 , m_spiralRadius(1.0f)
{
}

//-----------------------------------------------------------------------------
SwooshEditorIoWin::~SwooshEditorIoWin()
{
	delete m_object1;
	m_object1 = NULL;

	delete m_object2;
	m_object2 = NULL;

	delete m_defenderObject;
	m_defenderObject = NULL;

	unequipWeapon();

	AppearanceTemplateList::release(m_nextAppearanceTemplate);
}

//-----------------------------------------------------------------------------
void SwooshEditorIoWin::setAppearanceTemplate(AppearanceTemplate const * const appearanceTemplate)
{
	AppearanceTemplateList::release(m_nextAppearanceTemplate);

	m_nextAppearanceTemplate = appearanceTemplate;
	
	AppearanceTemplateList::fetch(m_nextAppearanceTemplate);

	m_appearanceChanged = true;
}

//-----------------------------------------------------------------------------
void SwooshEditorIoWin::draw() const
{
	IoWin::draw();

	// Hold the lighting of the terrain constant

	TerrainObject * const terrainObject = TerrainObject::getInstance();
	
	static float timeOfDay = 0.25f;

	if (terrainObject != NULL)
	{
		terrainObject->setTime(timeOfDay, true);
	}
}

//-----------------------------------------------------------------------------
void SwooshEditorIoWin::setObjectTransform(Transform const &transform)
{
	m_objectTransform = transform;
}

//-----------------------------------------------------------------------------
void SwooshEditorIoWin::setTimeOfDayCycle(bool const timeOfDayCycle)
{
	m_timeOfDayCycle = timeOfDayCycle;
}

//-----------------------------------------------------------------------------
void SwooshEditorIoWin::alter(float const deltaTime)
{
	Object *attackerObject = Game::getPlayer();

	if (attackerObject != NULL)
	{
		PlaybackScriptManager::ConstPlaybackScriptVector playbackScripts;
		PlaybackScriptManager::getPlaybackScriptsForActor(attackerObject, playbackScripts);

		// Check to see if we can restart the playback script every few game frames

		if (   (Os::getNumberOfUpdates() > (m_gameUpdate + 60))
		    && playbackScripts.empty())
		{
			bool startAnimation = false;

			if (m_pauseAfterEachAnimation)
			{
				m_pauseAfterEachAnimationTimer += deltaTime;

				if (m_pauseAfterEachAnimationTimer > 1.0f)
				{
					startAnimation = true;
				}
			}
			else
			{
				startAnimation = true;
			}

			if (startAnimation)
			{
				m_pauseAfterEachAnimationTimer = 0.0f;
				m_gameUpdate = Os::getNumberOfUpdates();

				// For animations check "swg\current\dsrc\sku.0\sys.client\compiled\game\combat\combat_manager.mif"
				// For weapons check "swg\current\data\sku.0\sys.shared\compiled\game\object\weapon\"

				m_defenderObject->setPosition_w(Vector(0.0f, 0.0f, m_defenderDistance));

				if (m_weaponName.empty())
				{
					unequipWeapon();
				}
				else
				{
					equipWeapon(m_weaponName);
				}

				//-- Setup an attack.

				Object *      attacker = attackerObject;
				const uint32  actionId = Crc::normalizeAndCalculate (m_animationName.c_str());
				const int     attackerPostureEndIndex = 0; // Standing
				const int     attackerTrailBits = m_trailFlags;
				const int     attackerClientEffectId = 0;
				const int     actionNameCrc = 0;
				const bool    attackerUseTarget = 0;
				const Vector  attackerTargetLocation;
				const NetworkId attackerTargetCell = NetworkId::cms_invalid;
				Object *      defender = m_defenderObject;
				const int     defenderEndPostureIndex = 0;
				const ClientCombatActionInfo::DefenderDefense defense = static_cast<ClientCombatActionInfo::DefenderDefense> (ClientCombatActionInfo::DD_hit);
				const int     defenderClientEffectId = 0;
				const int     defenderHitLocation     = 0;
				const int     damageAmount = 0;

				const ClientCombatActionInfo actionInfo(attacker, m_weaponObject, attackerPostureEndIndex, attackerTrailBits, attackerClientEffectId, actionNameCrc, attackerUseTarget, attackerTargetLocation, attackerTargetCell, actionId, defender, defenderEndPostureIndex, defense, defenderClientEffectId, defenderHitLocation, damageAmount);

				ClientCombatPlaybackManager::handleCombatAction(actionInfo);

				if (m_appearanceChanged)
				{
					m_appearanceChanged = false;

					// Recreate the reference objects each time a appearance is assigned and the animation is restarting

					if (m_nextAppearanceTemplate != NULL)
					{
						delete m_object1;
						m_object1 = new Object();

						delete m_object2;
						m_object2 = new Object();

						SwooshAppearanceTemplate const * const swooshAppearanceTemplate = dynamic_cast<SwooshAppearanceTemplate const * const>(m_nextAppearanceTemplate);
						
						if (swooshAppearanceTemplate != NULL)
						{
							SwooshAppearanceTemplate::setDefaultTemplate(*swooshAppearanceTemplate);
						}

						{
							SwooshAppearance * const swooshAppearance = SwooshAppearance::asSwooshAppearance(m_nextAppearanceTemplate->createAppearance());
							
							if (swooshAppearance != NULL)
							{
								swooshAppearance->setUnBounded(true);
								swooshAppearance->setAllowAutoDelete(false);
							}
							
							RenderWorld::addObjectNotifications(*m_object1);
							m_object1->addNotification(ClientWorld::getIntangibleNotification());
							m_object1->setAppearance(swooshAppearance);
							m_object1->addToWorld();
						}
						
						{
							SwooshAppearance * const swooshAppearance = SwooshAppearance::asSwooshAppearance(m_nextAppearanceTemplate->createAppearance());
							
							if (swooshAppearance != NULL)
							{
								swooshAppearance->setUnBounded(true);
								swooshAppearance->setAllowAutoDelete(false);
								swooshAppearance->setReferenceObject(m_object2);
							}
							
							RenderWorld::addObjectNotifications(*m_object2);
							m_object2->addNotification(ClientWorld::getIntangibleNotification());
							m_object2->setAppearance(swooshAppearance);
							m_object2->addToWorld();
						}
					}
				}
			}
		}
	}

	if (m_defenderObject == NULL)
	{
		m_defenderObject = ObjectTemplateList::createObject(TemporaryCrcString("object/creature/player/shared_wookiee_male.iff", true));
		m_defenderObject->setController(new CreatureController((dynamic_cast<CreatureObject *>(m_defenderObject.getPointer()))));
		(dynamic_cast<CreatureObject *>(m_defenderObject.getPointer()))->endBaselines();
		RenderWorld::addObjectNotifications(*m_defenderObject);
		m_defenderObject->addToWorld();
		m_defenderObject->yaw_o(3.14159f);
		/* TPERRY - commented out to fix compile error - requestSetCombatTarget() was removed from TangibleObject
		@TODO: hook this up when requestSetCombatTarget() or its replacement is working again
		(dynamic_cast<TangibleObject *>(attackerObject))->requestSetCombatTarget((dynamic_cast<TangibleObject *>(m_defenderObject.getPointer())));
		*/
	}

	if (   (m_object1 != NULL)
	    && (m_object2 != NULL))
	{
		
		{
			// Hide the reference swooshes

			SwooshAppearance * const swooshAppearance1 = SwooshAppearance::asSwooshAppearance(m_object1->getAppearance());
			SwooshAppearance * const swooshAppearance2 = SwooshAppearance::asSwooshAppearance(m_object2->getAppearance());

			VectorArgb color(VectorArgb::solidWhite);

			if (m_referenceSwoosh == RS_none)
			{
				color.a = 0.0f;
			}

			swooshAppearance1->setColorModifier(color);
			swooshAppearance2->setColorModifier(color);
		}

		switch (m_referenceSwoosh)
		{
			case RS_none: {} break;
			case RS_circling:
			case RS_spiraling:
				{
					SwooshAppearance * const swooshAppearance = SwooshAppearance::asSwooshAppearance(m_object1->getAppearance());

					if (swooshAppearance != NULL)
					{
						static float radian = 0.0f;
						float radius = 1.0f;

						if (m_referenceSwoosh == RS_spiraling)
						{
							m_spiralRadius += 0.3f * deltaTime;
							if (m_spiralRadius > 6.0f)
							{
								m_spiralRadius = 1.0f;
							}
							radius = m_spiralRadius;
						}
						else
						{
							radius = 4.0f;
						}

						float const x = m_objectTransform.getPosition_p().x + sinf(radian) * radius;
						float const y = m_objectTransform.getPosition_p().y;
						float const z = m_objectTransform.getPosition_p().z + cosf(radian) * radius;

						radian += m_objectRotationSpeed * deltaTime;

						Vector singlePosition_w(x, y + 4.0f, z);
						Vector topPosition_w(x, y + 2.0f, z);
						Vector bottomPosition_w(x, y + 1.0f, z);

						swooshAppearance->setPosition_w(topPosition_w, bottomPosition_w);
						m_object2->setPosition_w(singlePosition_w);
					}
				}
				break;
		}
	}
}

//-----------------------------------------------------------------------------
void SwooshEditorIoWin::setObjectRotationSpeed(float const objectRotationSpeed)
{
	m_objectRotationSpeed = objectRotationSpeed;
}

//-----------------------------------------------------------------------------
void SwooshEditorIoWin::setAnimation(std::string const &animationName, std::string const &weaponName, int const trailFlags, float const defenderDistance)
{
	m_animationName = animationName;
	m_weaponName = weaponName;
	m_trailFlags = trailFlags;
	m_defenderDistance = defenderDistance;
}

//-----------------------------------------------------------------------------
void SwooshEditorIoWin::unequipWeapon()
{
	if (m_weaponObject != NULL)
	{
		TangibleObject *obj = dynamic_cast<TangibleObject *>(m_weaponObject.getPointer());
		TangibleObject *target = dynamic_cast<TangibleObject *>(Game::getPlayer());

		if (   (obj != NULL)
			&& (target != NULL))
		{
			if (!CuiInventoryManager::unequipItem(*obj, *target))
			{
				DEBUG_REPORT_LOG(true, ("Unable to unequip item: %s\n", obj->getObjectTemplateName()));
			}
		}

		delete m_weaponObject;
		m_weaponObject = NULL;
	}
}

//-----------------------------------------------------------------------------
void SwooshEditorIoWin::equipWeapon(std::string const &weaponName)
{
	// See if we are changing weapons and need to unequip the current one

	if (   (m_weaponObject != NULL)
	    && (strstr(weaponName.c_str(), m_weaponObject->getObjectTemplateName()) == NULL))
	{
		unequipWeapon();
	}

	// See if we need to equip the weapon, it may already be equipped

	if (m_weaponObject == NULL)
	{
		m_weaponObject = ObjectTemplateList::createObject(TemporaryCrcString(weaponName.c_str(), true));
		ClientObject *clientObject = (dynamic_cast<ClientObject *>(m_weaponObject.getPointer()));

		if (clientObject != NULL)
		{
			clientObject->endBaselines();

			TangibleObject *obj = dynamic_cast<TangibleObject *>(m_weaponObject.getPointer());
			TangibleObject *target = dynamic_cast<TangibleObject *>(Game::getPlayer());

			if (   (obj != NULL)
				&& (target != NULL))
			{
				if (!CuiInventoryManager::equipItem(*obj, *target))
				{
					DEBUG_REPORT_LOG(true, ("Unable to equip item: %s\n", obj->getObjectTemplateName()));
				}
			}
		}
	}
}

//-----------------------------------------------------------------------------
void SwooshEditorIoWin::setPauseAfterEachAnimation(bool const enabled)
{
	m_pauseAfterEachAnimation = enabled;
}

//-----------------------------------------------------------------------------
bool SwooshEditorIoWin::isPauseAfterEachAnimation() const
{
	return m_pauseAfterEachAnimation;
}

//-----------------------------------------------------------------------------
void SwooshEditorIoWin::setReferenceSwoosh(ReferenceSwoosh const referenceSwoosh)
{
	m_referenceSwoosh = referenceSwoosh;
}

//=============================================================================
