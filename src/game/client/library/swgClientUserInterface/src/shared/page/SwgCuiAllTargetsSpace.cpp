//======================================================================
//
// SwgCuiAllTargetsSpace.cpp
// copyright(c) 2004 Sony Online Entertainment
//
//======================================================================

#include "swgClientUserInterface/FirstSwgClientUserInterface.h"
#include "swgClientUserInterface/SwgCuiAllTargetsSpace.h"

#include "UIData.h"
#include "UIImage.h"
#include "UIOpacityEffector.h"
#include "UIPage.h"
#include "UISizeEffector.h"
#include "UIText.h"
#include "UIUtils.h"
#include "clientGame/ClientShipTargeting.h"
#include "clientGame/ClientWaypointObject.h"
#include "clientGame/ClientWorld.h"
#include "clientGame/ConfigClientGame.h"
#include "clientGame/ContainerInterface.h"
#include "clientGame/CreatureObject.h"
#include "clientGame/Game.h"
#include "clientGame/GroundScene.h"
#include "clientGame/MissileManager.h"
#include "clientGame/PlayerShipController.h"
#include "clientGame/ShipObject.h"
#include "clientGame/ShipObjectAttachments.h"
#include "clientGame/ShipStation.h"
#include "clientGraphics/Graphics.h"
#include "clientGraphics/ShaderTemplateList.h"
#include "clientObject/GameCamera.h"
#include "clientUserInterface/CuiDamageManager.h"
#include "clientUserInterface/CuiGameColorManager.h"
#include "clientUserInterface/CuiIoWin.h"
#include "clientUserInterface/CuiLayerRenderer.h"
#include "clientUserInterface/CuiManager.h"
#include "clientUserInterface/CuiObjectTextManager.h"
#include "clientUserInterface/CuiPreferences.h"
#include "clientUserInterface/CuiUtils.h"
#include "sharedCollision/BoxExtent.h"
#include "sharedDebug/DebugFlags.h"
#include "sharedFoundation/PointerDeleter.h"
#include "sharedGame/SharedCreatureObjectTemplate.h"
#include "sharedMath/Sphere.h"
#include "sharedMessageDispatch/Transceiver.h"
#include "sharedNetworkMessages/ShipDamageMessage.h"
#include "sharedObject/Appearance.h"
#include "sharedObject/CachedNetworkId.h"
#include "sharedObject/CellProperty.h"
#include "sharedObject/ContainedByProperty.h"
#include "swgClientUserInterface/SwgCuiAllTargetsSpace_DamagerData.h"
#include "swgClientUserInterface/SwgCuiHudFactory.h"
#include "swgClientUserInterface/SwgCuiHudSpace.h"
#include <algorithm>
#include <stack>

//======================================================================

namespace SwgCuiAllTargetsSpaceNamespace
{
	float const c_damagerTimer = 3.3f;
	float const c_damageTimerMax = 15.f;

	bool s_drawMissileTargetAcquisition = false;
	bool s_debugFlagsInstalled = false;
	bool s_useTargetLeadSmoothing = true;

	std::string const s_muzzleHardpointName = "muzzle";

	int const s_virtualjoystickImageCount = 4;
	float const s_virtualjoystickDelta = 7.f;
	float const s_virtualjoystickCenterDelta = 1.0f / static_cast<float>(s_virtualjoystickImageCount + 1);

	//fixme: this needs to be data driven
	char const * const s_virtualjoystickSourceName = "ui_simjoystick_0%d";

	int const cms_minimumWaypointIndicatorDistance = 50;
	int const cms_scratchBufferSize = 64;
	char ms_scratchBuffer[cms_scratchBufferSize];

	UIPoint s_cursorLocationLast;
	float s_virtualJoystickFadeOpacity = 1.0f;
	float const s_virtualJoystickFadeRate = 0.875f;
	float const s_virtualJoystickFadeMagnitude = 5.0f;
	Vector s_lastTargetLeadLocation_o;

	float s_currentDistanceToTarget = 0.0f;
	bool  s_targetPageClosed = false;
}

using namespace SwgCuiAllTargetsSpaceNamespace;

//----------------------------------------------------------------------

SwgCuiAllTargetsSpace::SwgCuiAllTargetsSpace(UIPage & page) :
SwgCuiAllTargets("SwgCuiAllTargetsSpace", page),
m_callback(new MessageDispatch::Callback),
m_damagers(new DamagerMap),
m_arrowPages(new PageStack),
m_damageDirectionArrow(NULL),
m_virtualJoystickImage(NULL),
m_virtualJoystickImages(new ImageVector),
m_virtualJoystickReset(true),
m_timeDelta(0.0f),
m_targetLeadIndicator(NULL),
m_targetLeadIndicatorOutOfRange(NULL),
m_sampleWaypointIndicatorPage(NULL),
m_containerPage(new UIPage),
m_waypointIndicatorPages(),
m_turretTargetPages(),
m_sampleTurretTargetArrow(0),
m_targetArrowPage(NULL)
{
	m_containerPage->SetName("ContainerPage");
	getPage().AddChild(m_containerPage);
	m_containerPage->Attach (0);
	m_containerPage->SetSize(getPage().GetSize());

	if (!s_debugFlagsInstalled)
	{
		s_debugFlagsInstalled = true;
		DebugFlags::registerFlag(s_drawMissileTargetAcquisition, "SwgClientUserInterface", "drawMissileTargetAcquisition");
		DebugFlags::registerFlag(s_useTargetLeadSmoothing, "SwgClientUserInterface", "useTargetLeadSmoothing");
	}

	getCodeDataObject(TUIPage, m_damageDirectionArrow, "DamageDirectionArrow");
	m_damageDirectionArrow->SetRotation(0.0f);
	m_damageDirectionArrow->Pack();
	m_damageDirectionArrow->Center();
	m_damageDirectionArrow->SetVisible(false);

	getCodeDataObject(TUIImage, m_virtualJoystickImage, "virtualJoystick");
	m_virtualJoystickImage->SetVisible(false);

	joystickImageConstruct();

	getCodeDataObject(TUIImage, m_targetLeadIndicator, "targetLeadIndicator");
	m_targetLeadIndicator->SetVisible(false);

	getCodeDataObject(TUIImage, m_targetLeadIndicatorOutOfRange, "targetLeadIndicatorOutOfRange");
	m_targetLeadIndicatorOutOfRange->SetVisible(false);

	getCodeDataObject(TUIPage, m_sampleWaypointIndicatorPage, "waypointIndicator");
	m_sampleWaypointIndicatorPage->SetVisible(false);

	getCodeDataObject(TUIPage, m_sampleTurretTargetArrow, "sampleArrow");
	m_sampleTurretTargetArrow->SetVisible(false);
	m_sampleTurretTargetArrow->Center();
  	
	getCodeDataObject(TUIPage, m_targetArrowPage, "sampleTargetArrow");
	m_targetArrowPage->SetVisible(false);
	m_targetArrowPage->Center();
}

//----------------------------------------------------------------------

SwgCuiAllTargetsSpace::~SwgCuiAllTargetsSpace()
{
	m_containerPage->Detach(NULL);
	m_containerPage = NULL;

	std::for_each (m_waypointIndicatorPages.begin (), m_waypointIndicatorPages.end (), UIBaseObject::DetachFunctor(NULL));
	std::for_each (m_turretTargetPages.begin (), m_turretTargetPages.end (), UIBaseObject::DetachFunctor(NULL));
	std::for_each(m_damagers->begin(), m_damagers->end(), PointerDeleterPairSecond());

	m_damagers->clear();

	for (;m_arrowPages->size(); m_arrowPages->pop())
	{
		UIPage * const uiPage = m_arrowPages->top();
		getPage().RemoveChild(uiPage);
		uiPage->Detach(0);
	}

	delete m_callback;
	m_callback = NULL;

	delete m_damagers;
	m_damagers = NULL;

	delete m_arrowPages;
	m_arrowPages = NULL;

	m_damageDirectionArrow = NULL;

	joystickImageRelease();

	delete m_virtualJoystickImages;
	m_virtualJoystickImages= NULL;

	m_virtualJoystickImage = NULL;

	m_targetLeadIndicator = NULL;
	m_targetLeadIndicatorOutOfRange = NULL;

	m_targetArrowPage = NULL;
}

//----------------------------------------------------------------------
void SwgCuiAllTargetsSpace::performActivate()
{
	SwgCuiAllTargets::performActivate();
	
	m_callback->connect(*this, &SwgCuiAllTargetsSpace::onShipDamaged, static_cast<CuiDamageManager::Messages::ShipDamage *>(0));
}

//----------------------------------------------------------------------

void SwgCuiAllTargetsSpace::performDeactivate()
{
	SwgCuiAllTargets::performDeactivate();
	
	m_callback->disconnect(*this, &SwgCuiAllTargetsSpace::onShipDamaged, static_cast<CuiDamageManager::Messages::ShipDamage *>(0));
}

//----------------------------------------------------------------------

void SwgCuiAllTargetsSpace::update(float const deltaTimeSecs)
{
	m_timeDelta = deltaTimeSecs;

	if (m_damagers != NULL)
	{
		NetworkId targetId = NetworkId::cms_invalid;
		CreatureObject const * const player = Game::getPlayerCreature();
		if (player)
		{
			Object const * const lookAtTarget = player->getLookAtTarget().getObject();
			if (lookAtTarget)
			{
				targetId = lookAtTarget->getNetworkId();
			}
		}

		//-- Update damage waypoint.
		for (DamagerMap::iterator itDamager = m_damagers->begin(); itDamager != m_damagers->end(); /*increment in body*/ )
		{
			DamagerData * const damagerData = itDamager->second;

			damagerData->incrementTimer(-deltaTimeSecs);
			damagerData->getPage()->SetVisible(itDamager->first != targetId);

			if (damagerData->getTimer() < 0.0f || !CuiPreferences::getDamagerArrow())
			{
				DamagerMap::iterator itDamagerDeleted = itDamager;
				++itDamager;

				//-- Cache this page.
				damagerData->getPage()->SetVisible(false);
				m_arrowPages->push(damagerData->getPage());

				//-- Delete the memory.
				delete itDamagerDeleted->second;

				m_damagers->erase(itDamagerDeleted);
			}
			else
			{
				++itDamager;
			}
		}
	}

	Camera const * const camera = Game::getCamera();
	if (NULL != camera)
	{
		updateShipTarget(*camera);
		updateShipDamageIndicator(*camera);
		updateTargetArrow(*camera);
		
		if(Game::isSpace())
		{
			updateWaypointIndicators(*camera);
			updateTurretTargets(*camera);
		}
	}

	IGNORE_RETURN(getPage().MoveChild(m_containerPage, UIBaseObject::Bottom));

	//-- chain up last so that our screen center modifications take effect

	SwgCuiAllTargets::update(deltaTimeSecs);
}

//----------------------------------------------------------------------

void SwgCuiAllTargetsSpace::updateShipTarget(Camera const & camera)
{
	bool screenCenterSet = false;
	bool hideTargetLeadIndicator = true;
	bool hideVirtualJoystick = true;
	bool isTargetOutOfRange = true;

	CreatureObject const * const player = Game::getPlayerCreature();
	if (player)
	{
		int const gunnerWeaponIndex = ShipStation::getWeaponIndexForGunnerStation(player->getShipStation());
		if (player->getShipStation() == ShipStation::ShipStation_Pilot || gunnerWeaponIndex != -1)
		{
			ShipObject const * const ship = ShipObject::getContainingShip(*player);
			if (ship)
			{
				// Max weapon range.
				float weaponCrosshairRange = 128.0f;

				//-- Get the shot direction.
				Vector shotDirection = ship->getObjectFrameK_w();
				Transform muzzleTransform = ship->getTransform_o2w();

				if (gunnerWeaponIndex != -1)
				{
					//-- Get the shot direction if you a gunner.
					ShipChassisSlotType::Type const shipChassisSlotType = static_cast<ShipChassisSlotType::Type>(static_cast<int>(ShipChassisSlotType::SCST_weapon_0) + gunnerWeaponIndex);
					ShipObjectAttachments::TransformObjectList transformObjectList;
					if (ship->getShipObjectAttachments().findAttachmentHardpoints_o(shipChassisSlotType, s_muzzleHardpointName, ship->getTransform_o2w(), transformObjectList))
					{
						if (transformObjectList.empty())
							shotDirection = ship->getTransform_o2w().getLocalFrameK_p();
						else
						{
							muzzleTransform.multiply(muzzleTransform, transformObjectList.front().first);
							shotDirection = muzzleTransform.getLocalFrameK_p();
						}
					}

					//-- Set the weapon range.
					weaponCrosshairRange = std::max(weaponCrosshairRange, ship->getWeaponRange(gunnerWeaponIndex));
				}
				//-- choose the currently selected weapon
				else
				{
					Vector const & averageMuzzlePos = ship->getShipObjectAttachments().getAverageProjectileMuzzlePosition_p();
					muzzleTransform.setPosition_p(ship->getTransform_o2w().rotateTranslate_l2p(averageMuzzlePos));
					float const averageProjectileRange = ship->getShipObjectAttachments().getAverageProjectileRange();
					weaponCrosshairRange = std::max(weaponCrosshairRange, averageProjectileRange );
				}
				
				//-- Keep the target point outside the radius of the ship.
				float minimumRange = 32.f;
				Appearance const * appearance = ship->getAppearance();
				if(appearance)
					minimumRange += appearance->getSphere().getRadius();

				//-- Draw weapon crosshair.
				Object const * const lookAtTarget = player->getLookAtTarget().getObject();

				Vector newTargetLocation_w;
				isTargetOutOfRange = !lookAtTarget || !ship->getTargetLeadPosition_w(newTargetLocation_w, *lookAtTarget);

				if (isTargetOutOfRange) 
				{
					s_currentDistanceToTarget = linearInterpolate(s_currentDistanceToTarget, weaponCrosshairRange, std::min(m_timeDelta, 1.f));
				}
				else
				{
					float const distanceToTarget = (newTargetLocation_w - ship->getPosition_w()).approximateMagnitude();
					s_currentDistanceToTarget = linearInterpolate(s_currentDistanceToTarget, distanceToTarget, std::min(m_timeDelta, 1.f));
					isTargetOutOfRange = distanceToTarget > weaponCrosshairRange || distanceToTarget < minimumRange;
				}
				weaponCrosshairRange = std::max(minimumRange, weaponCrosshairRange);

				// Prevent the distance from getting too close or being too far away
				s_currentDistanceToTarget = clamp(minimumRange, s_currentDistanceToTarget, weaponCrosshairRange);

				//-- Display
				Vector const & targetPoint_w = muzzleTransform.getPosition_p() + (shotDirection * s_currentDistanceToTarget);
				Vector screenCoords;
				UIPoint screenCenterOffset;
				
				UIRect const screenRect(0L, 0L, Graphics::getCurrentRenderTargetWidth(), Graphics::getCurrentRenderTargetHeight());
				UIPoint centerLocation(screenRect.Size() / 2L);

				if (!Game::isViewFreeCamera() && camera.projectInWorldSpace(targetPoint_w, &screenCoords.x, &screenCoords.y, 0))
				{
					long const cursorPosX = static_cast<long>(screenCoords.x + 0.5f);
					long const cursorPosY = static_cast<long>(screenCoords.y + 0.5f);

					int const deadZoneSize = CuiIoWin::getDeadZoneSizeUsable();
					long const halfDeadZoneSize = deadZoneSize / 2;

					UIRect const resultingDeadZoneRect(UISize (cursorPosX - halfDeadZoneSize, cursorPosY - halfDeadZoneSize), UISize(deadZoneSize, deadZoneSize));
					
					//-- if the deadzone goes off the edge of the screen, the virtual joystick function will be ruined
					//-- keep the deadzone on the screen
					centerLocation.x = std::max(cursorPosX, static_cast<long>(screenRect.left + halfDeadZoneSize));
					centerLocation.x = std::min(cursorPosX, static_cast<long>(screenRect.right - halfDeadZoneSize));
					
					centerLocation.y = std::max(cursorPosY, static_cast<long>(screenRect.top + halfDeadZoneSize));
					centerLocation.y = std::min(cursorPosY, static_cast<long>(screenRect.bottom - halfDeadZoneSize));
					SwgCuiHudSpace::setReticleVisible(true);
				}
				else
					SwgCuiHudSpace::setReticleVisible(false);
				
				CuiManager::getIoWin().setScreenCenter(centerLocation, true);
				screenCenterSet = true;					
				
				CuiManager::getIoWin().getScreenCenterOffset(screenCenterOffset);
				
				if (!CuiManager::getPointerToggledOn() && !Game::isViewFirstPerson())
				{
					PlayerShipController const * const playerShipController = dynamic_cast<PlayerShipController const *>(ship->getController());
					bool const virtualJoystickActive = playerShipController && playerShipController->isVirtualJoystickActive();
					
					if (virtualJoystickActive)
					{
						UIPoint cursorLocation;
						CuiManager::getIoWin().getCursorLocation(cursorLocation);

						if ((s_cursorLocationLast - cursorLocation).Magnitude() > s_virtualJoystickFadeMagnitude)
						{
							s_virtualJoystickFadeOpacity = 1.0;
							s_cursorLocationLast = cursorLocation;
						}
						else
						{
							s_virtualJoystickFadeOpacity -= (s_virtualJoystickFadeRate * m_timeDelta);
						}
						
						s_virtualJoystickFadeOpacity = clamp(0.0f, s_virtualJoystickFadeOpacity, 1.0f);
				
						joystickImageUpdate(cursorLocation - screenCenterOffset, centerLocation - screenCenterOffset);
						hideVirtualJoystick = false;
					}					
				}
				
				//-- Draw target lead indicator.
				if (lookAtTarget && m_targetLeadIndicator && m_targetLeadIndicatorOutOfRange)
				{
					bool validLeadTarget = false;
					
					ClientObject const * const clientLookAtTarget = lookAtTarget->asClientObject();
					if (NULL != clientLookAtTarget)
					{
						int const lookAtTargetGot = clientLookAtTarget->getGameObjectType();
						
						if (lookAtTargetGot != SharedObjectTemplate::GOT_ship_station && lookAtTargetGot != SharedObjectTemplate::GOT_ship_mining_asteroid_static)
							validLeadTarget = true;
					}
					
					if (validLeadTarget)
					{						
						//-- Smooth out the lead indicator position relative to the target
						if (s_useTargetLeadSmoothing)
						{
							Vector const &newTargetLocation_o = lookAtTarget->rotateTranslate_w2o(newTargetLocation_w);
							s_lastTargetLeadLocation_o = Vector::linearInterpolate(s_lastTargetLeadLocation_o, newTargetLocation_o, std::min(m_timeDelta, 1.f));
						}
						else
							s_lastTargetLeadLocation_o = lookAtTarget->rotateTranslate_w2o(newTargetLocation_w);
						
						Vector leadScreenPos;
						
						if (camera.projectInWorldSpace(lookAtTarget->rotateTranslate_o2w(s_lastTargetLeadLocation_o), &leadScreenPos.x, &leadScreenPos.y, 0))
						{
							UIPoint leadLocation(static_cast<long>(leadScreenPos.x + 0.5f), static_cast<long>(leadScreenPos.y + 0.5f));
							leadLocation -= screenCenterOffset;
							m_targetLeadIndicator->SetLocation(leadLocation, true);
							m_targetLeadIndicatorOutOfRange->SetLocation(leadLocation, true);
							hideTargetLeadIndicator = false;
						}
					}
				}
			}
		}
	}

	// If the screen center was not modified by the previous block of code, reset.
	if (!screenCenterSet)
	{
		CuiManager::getIoWin().resetScreenCenter();
	}

	// Hide the virtual joystick if necessary.
	joystickImageHide(hideVirtualJoystick);

	// Hide the target lead if necessary.
	if (m_targetLeadIndicator && m_targetLeadIndicatorOutOfRange)
	{
		m_targetLeadIndicatorOutOfRange->SetVisible(!hideTargetLeadIndicator && isTargetOutOfRange);
		m_targetLeadIndicator->SetVisible(!hideTargetLeadIndicator && !isTargetOutOfRange);
	}

} //lint !e1762 //not made const

//----------------------------------------------------------------------

void  SwgCuiAllTargetsSpace::onShipDamaged(CuiDamageManager::Messages::ShipDamage::Payload const &shipDamage)
{
	if (m_damagers != NULL && CuiPreferences::getDamagerArrow())
	{
		// Update damage waypoints.
		DamagerMap::iterator itDamager = m_damagers->find(shipDamage.getAttackerNetworkId());
		if (itDamager != m_damagers->end())
		{
			//-- Update the timer.
			DamagerData &damagerData = *itDamager->second;
			damagerData.incrementTimer(c_damagerTimer);

			//-- Clamp the timer to max.
			if (damagerData.getTimer() > c_damageTimerMax)
			{
				damagerData.setTimer(c_damageTimerMax);
			}
		}
		else
		{
			//-- Get a new page.
			UIPage * uiPage = getShipArrowPage();
			if (uiPage)
			{
				DamagerMap::iterator damagerExisting = m_damagers->find(shipDamage.getAttackerNetworkId());
				if (damagerExisting != m_damagers->end())
				{
					delete damagerExisting->second;
				}

				(*m_damagers)[shipDamage.getAttackerNetworkId()] = new DamagerData(c_damagerTimer, uiPage, shipDamage.getAttackerPosition());
			}
		}
	}
}

//-----------------------------------------------------------------

void SwgCuiAllTargetsSpace::updateShipDamageIndicator(Camera const& camera)
{
	if (m_damagers)
	{
		for (DamagerMap::iterator itDamager = m_damagers->begin(); itDamager != m_damagers->end(); ++itDamager)
		{
			DamagerData &damagerData = *itDamager->second;
			Vector damagePos_camera = camera.rotateTranslate_w2o(damagerData.getPosition());
			
			Vector targetScreenPos;
			bool visible = camera.projectInCameraSpace(damagePos_camera, &targetScreenPos.x, &targetScreenPos.y, 0);
	
			if ( (!visible || CuiPreferences::getVisibleEnemyDamagerArrow() ) && damagePos_camera.normalize())
			{
				damagePos_camera.z = damagePos_camera.y;
				damagerData.getPage()->SetRotation(damagePos_camera.theta() / PI_TIMES_2);
				damagerData.getPage()->SetVisible(true);
			}
			else
			{
				damagerData.getPage()->SetVisible(false);
			}
		}
	}
}

//-----------------------------------------------------------------

void SwgCuiAllTargetsSpace::updateWaypointIndicators(Camera const& camera)
{
	ShipObject const * const playerShip = Game::getPlayerContainingShip();
	if(!playerShip)
		return;

	ClientWaypointObject::WaypointVector const & wv = ClientWaypointObject::getActiveWaypoints();
	size_t const numWaypoints = wv.size();
	size_t numPages = m_waypointIndicatorPages.size();
	
	if (numWaypoints > numPages)
	{
		m_waypointIndicatorPages.resize (numWaypoints);
		PageVector::iterator init_it = m_waypointIndicatorPages.begin();
		std::advance (init_it, numPages);
		for(PageVector::iterator pit = init_it; pit != m_waypointIndicatorPages.end(); ++pit)
		{
			*pit = NULL;
		}
		numPages = m_waypointIndicatorPages.size();
		DEBUG_FATAL (numPages != numWaypoints, (""));
	}
	else if (numWaypoints < numPages)
	{
		PageVector::iterator cut_it = m_waypointIndicatorPages.begin();
		std::advance (cut_it, numWaypoints);

		for (PageVector::iterator pit = cut_it; pit != m_waypointIndicatorPages.end(); ++pit)
		{
			UIPage * const page = *pit;
			NOT_NULL (page);
			m_containerPage->RemoveChild (page);
			page->Detach (0);
		}

		IGNORE_RETURN(m_waypointIndicatorPages.erase (cut_it, m_waypointIndicatorPages.end()));
	}

	for (size_t i = 0; i < numWaypoints; ++i)
	{
		ClientWaypointObject const * const waypoint = wv[i];
		NOT_NULL (waypoint);
		if (waypoint) //lint !e774 //not always true
		{
			ClientWaypointObject const * const waypoint_world = waypoint ? waypoint->getWorldObject() : NULL;
			UIPage * & page = m_waypointIndicatorPages[i];

			if (!page)
			{
				page = safe_cast<UIPage *>(m_sampleWaypointIndicatorPage->DuplicateObject());
				NOT_NULL (page);
				page->Attach (0);
				m_containerPage->AddChild (page);
				page->Link();
				page->SetName ("waypointIndicator");
				page->Center();
				page->SetVisible (false);
			}

			if (!waypoint_world)
			{
				page->SetVisible (false);
				continue;
			}

			Appearance const * const app = waypoint_world->getAppearance();
			Vector const & waypointPos = waypoint->getLocation();
			NOT_NULL (app);
						
			Vector posToDraw_o (camera.rotateTranslate_w2o (waypointPos));
			bool const isVisible = isWaypointVisible(posToDraw_o, camera, app);

			//update the UI element if necessary
			if (isVisible)
			{
				Vector waypointIndicatorScreenPos;				
				if (camera.projectInCameraSpace(posToDraw_o, &waypointIndicatorScreenPos.x, &waypointIndicatorScreenPos.y, 0))
				{
					UIPoint waypointIndicatorLocation(static_cast<long>(waypointIndicatorScreenPos.x), static_cast<long>(waypointIndicatorScreenPos.y));
					UIPoint screenCenterOffset;
					CuiManager::getIoWin().getScreenCenterOffset(screenCenterOffset);
					waypointIndicatorLocation -= screenCenterOffset;
					page->SetLocation(waypointIndicatorLocation, true);
					Vector const & distanceVector = playerShip->getPosition_p() - waypointPos;
					int const distance = static_cast<int>(distanceVector.magnitude());
					//only show indicator when more than cms_minimumWaypointIndicatorDistance meters out
					if(distance > cms_minimumWaypointIndicatorDistance)
					{
						_snprintf (ms_scratchBuffer, cms_scratchBufferSize-1, "%dm", distance);
						UIText * const text = safe_cast<UIText *>(page->GetChild("textDistance"));
						if(text)
						{
							text->SetLocalText(Unicode::narrowToWide(ms_scratchBuffer));
						}
						VectorArgb const & colorArgb = waypoint->getColorArgb();
						page->SetColor  (CuiUtils::convertColor (colorArgb));
						page->SetVisible(true);
					}
					else
						page->SetVisible(false);
				}
			}
			else
			{
				page->SetVisible(false);
			}
			page->SetEnabled (isVisible);
		}
	}
}

//----------------------------------------------------------------------

void SwgCuiAllTargetsSpace::updateTargetArrow(Camera const & camera)
{
	if (!CuiPreferences::getTargetArrow())
	{
		if (!s_targetPageClosed)
		{
			m_targetArrowPage->SetEnabled(false);
			m_targetArrowPage->SetVisible(false);
			s_targetPageClosed = true;
		}
		
		return;
	}
	else
		s_targetPageClosed = false;
	
	ShipObject const * const playerShip = Game::getPlayerContainingShip();
	CreatureObject const * const playerCreature = Game::getPlayerCreature();
		
	if (!playerShip || !playerCreature)
		return;

	int const shipStation = playerCreature->getShipStation();
	
	bool showArrow = false;

	if (shipStation != ShipStation::ShipStation_None)
	{
		CachedNetworkId const & targetId = playerCreature->getLookAtTarget();
		Object const * const targetObject = targetId.getObject();
		
		if (targetObject)
		{
			Vector const & cameraPos_w = camera.getPosition_w();
			Vector const & targetPosition_w = targetObject->getPosition_w();
			Vector deltaToTarget = targetPosition_w - cameraPos_w;
			Vector deltaToTargetInCameraSpace = camera.rotate_w2o(deltaToTarget);

			Vector targetScreenPos;
			bool visible = camera.projectInCameraSpace(deltaToTargetInCameraSpace, &targetScreenPos.x, &targetScreenPos.y, 0);

			if (!visible)
			{
				Vector normalizedDelta(deltaToTargetInCameraSpace);

				visible = !normalizedDelta.normalize();

				if (!visible)
				{
					float const theta = atan2(normalizedDelta.x, normalizedDelta.y);
					m_targetArrowPage->SetRotation(theta / PI_TIMES_2);
				}
			}
			showArrow = !visible;
		}
	}
	
	m_targetArrowPage->SetEnabled(showArrow);
	m_targetArrowPage->SetVisible(showArrow);
}

//----------------------------------------------------------------------

void SwgCuiAllTargetsSpace::updateTurretTargets(Camera const& camera)
{
	ShipObject const * const playerShip = Game::getPlayerPilotedShip();

	if (playerShip == 0)
	{
		// if the player isn't piloting then they don't need
		// to see this turret targets
		return;
	}

	typedef std::vector<Object const *> TurretTargets;
	TurretTargets turretTargets;

	{
		typedef std::vector<CreatureObject *> Passengers;
		Passengers passengers;

		playerShip->findAllPassengers(passengers);

		Passengers::const_iterator ii = passengers.begin();
		Passengers::const_iterator iiEnd = passengers.end();

		CreatureObject const * const playerCreature = Game::getPlayerCreature();
		CachedNetworkId const myLookAtTarget = (playerCreature != 0) ? playerCreature->getLookAtTarget() : CachedNetworkId();

		for (; ii != iiEnd; ++ii)
		{
			CreatureObject const * const passenger = *ii;

			int const shipStation = passenger->getShipStation();

			if (shipStation != ShipStation::ShipStation_None)
			{
				if (ShipStation::isGunnerStation(shipStation))
				{
					CachedNetworkId const & targetId = passenger->getLookAtTarget();
					Object const * const target = targetId.getObject();

					if ((target != 0) && (target != myLookAtTarget.getObject()))
					{
						turretTargets.push_back(target);
					}
				}
			}
		}
	}

	// resize the pages...
	size_t const numberOfTurretTargets = turretTargets.size();
	size_t numPages = m_turretTargetPages.size();

	if (numberOfTurretTargets > numPages)
	{
		// resize the page array and assign NULL to the new elements
		m_turretTargetPages.resize(numberOfTurretTargets, 0);
		numPages = m_turretTargetPages.size();
		DEBUG_FATAL (numPages != numberOfTurretTargets, (""));
	}
	else if (numberOfTurretTargets < numPages)
	{
		PageVector::iterator iiBegin = m_turretTargetPages.begin();
		PageVector::iterator iiEnd = m_turretTargetPages.end();

		// advance the iterator to numberOfTurretTargets past begin()
		std::advance(iiBegin, numberOfTurretTargets);

		// remove the excess pages
		for (PageVector::const_iterator ii = iiBegin; ii != iiEnd; ++ii)
		{
			UIPage * const page = *ii;
			NOT_NULL(page);
			m_containerPage->RemoveChild(page);
			page->Detach(0);
		}

		// erase from the advanced iterator to the end
		IGNORE_RETURN(m_turretTargetPages.erase(iiBegin, iiEnd));
	}

	// populate the pages...
	for (size_t i = 0; i < numberOfTurretTargets; ++i)
	{
		Object const * const target = turretTargets[i];
		NOT_NULL(target);
		if (target != 0) //lint !e774 //not always true
		{
			UIPage * & page = m_turretTargetPages[i];

			if (page == 0)
			{
				// modify and add the new page
				page = safe_cast<UIPage *>(m_sampleTurretTargetArrow->DuplicateObject());
				NOT_NULL(page);

				UIColor const & color = CuiGameColorManager::getColorForType(CuiGameColorManager::T_turretTarget);
				page->SetColor(color);
				long const size = page->GetHeight();
				page->SetSize(UIPoint(size, size));
				page->Attach(0);
				m_containerPage->AddChild(page);
				page->Link();
				page->SetName("turretTarget");
				page->Center();
				page->SetVisible(true);
			}

			Vector const & cameraPos_w = camera.getPosition_w();
			Vector const & targetPosition_w = target->getPosition_w();
			Vector deltaToTarget = targetPosition_w - cameraPos_w;
			Vector deltaToTargetInCameraSpace = camera.rotate_w2o(deltaToTarget);

			Vector targetScreenPos;
			bool visible = !camera.projectInCameraSpace(deltaToTargetInCameraSpace, &targetScreenPos.x, &targetScreenPos.y, 0);

			if (visible)
			{
				Vector normalizedDelta(deltaToTargetInCameraSpace);

				visible = normalizedDelta.normalize();

				if (visible)
				{
					float const theta = atan2(normalizedDelta.x, normalizedDelta.y);
					page->SetRotation(theta / PI_TIMES_2);
				}
			}
			page->SetEnabled(visible);
		}
	}
}

//----------------------------------------------------------------------

void SwgCuiAllTargetsSpace::updateOnRender()
{
	SwgCuiAllTargets::updateOnRender();

	Camera const * const camera = Game::getCamera();
	if (NULL == camera)
		return;
	
	SwgCuiHud * const hud = SwgCuiHudFactory::findMediatorForCurrentHud();
	if (hud && !hud->getHudEnabled())
		return;

	CreatureObject const * const player = Game::getPlayerCreature();
	if (NULL != player)
	{
		ShipObject const * const ship = ShipObject::getContainingShip(*player);
		if (NULL != ship)
		{
			//-- render missile target circle
			if (s_drawMissileTargetAcquisition)
			{
				int const missileTypeId = ship->getCurrentMissileTypeId();

				if (missileTypeId != -1 && ship->hasMissiles())
				{
					Object const * const target = player->getLookAtTarget().getObject();
					
					if (target != NULL && target != ship && target->getAppearance())
					{
						if (target->getAppearance()->getRenderedThisFrame())
						{
							float const targetConeRadiusRadians = convertDegreesToRadians(MissileManager::getTargetAcquisitionAngle(missileTypeId));

							UIPoint screenCenter;
							CuiManager::getIoWin().getScreenCenter(screenCenter);
							
							Transform shipTransform_o2w = ship->getTransform_o2w();
							Transform const & muzzleTransform = ship->getTransform_o2w();							
							Vector const & shotDirection = muzzleTransform.getLocalFrameK_p();

							float const distanceToTarget = clamp(32.0f, (target->getPosition_w() - camera->getPosition_w()).approximateMagnitude(), 128.0f);
							Vector const & targetPoint_w = muzzleTransform.getPosition_p() + (shotDirection * distanceToTarget);

							Vector const & targetPoint_o = shipTransform_o2w.rotateTranslate_p2l(targetPoint_w);
							
							shipTransform_o2w.pitch_l(targetConeRadiusRadians * 0.5f);
							Vector const & targetPointUp_w = shipTransform_o2w.rotateTranslate_l2p(targetPoint_o);
							shipTransform_o2w.pitch_l(-targetConeRadiusRadians);
							Vector const & targetPointDown_w = shipTransform_o2w.rotateTranslate_l2p(targetPoint_o);
							
							Vector screenCoordsCircleEdgeUp;
							Vector screenCoordsCircleEdgeDown;
							
							CuiLayerRenderer::flushRenderQueue();
							if (camera->projectInWorldSpace(targetPointUp_w, &screenCoordsCircleEdgeUp.x, &screenCoordsCircleEdgeUp.y, NULL) &&
								camera->projectInWorldSpace(targetPointDown_w, &screenCoordsCircleEdgeDown.x, &screenCoordsCircleEdgeDown.y, NULL))
							{
								int const circleRadius = abs(static_cast<int>((screenCoordsCircleEdgeUp.y - screenCoordsCircleEdgeDown.y) * 0.5f));
								
 								Graphics::setStaticShader(ShaderTemplateList::get2dVertexColorAStaticShader());

								if (ship->hasMissileTargetAcquisition())
								{
									Graphics::drawCircle(screenCenter.x, screenCenter.y, circleRadius, VectorArgb::solidGreen);
									Graphics::drawCircle(screenCenter.x, screenCenter.y, circleRadius - 8, VectorArgb::solidGreen);
								}
								else if (ship->isMissileTargetAcquiring())
								{
									Graphics::drawCircle(screenCenter.x, screenCenter.y, circleRadius, VectorArgb::solidYellow);
								}
								else
								{
									Graphics::drawCircle(screenCenter.x, screenCenter.y, circleRadius, VectorArgb::solidRed);
								}
							}
						}
					}
				}
			}
		}
	}

	if (CuiPreferences::getRenderVariableTargetingReticle())
	{
		CuiLayerRenderer::flushRenderQueue();

		UIPoint screenCenter;
		CuiManager::getIoWin().getScreenCenter(screenCenter);
		int const circleRadius = ClientShipTargeting::getOnScreenVariableReticleRadius();
 		Graphics::setStaticShader(ShaderTemplateList::get2dVertexColorAStaticShader());
		VectorArgb const targetColor(0.25f, 1.0f, 0.0f, 8.0f);
		Graphics::drawCircle(screenCenter.x, screenCenter.y, circleRadius, targetColor);
	}

}

//-----------------------------------------------------------------

bool SwgCuiAllTargetsSpace::isShipDamager(NetworkId const & networkId) const
{
	bool isDamager = false;

	if (m_damagers)
	{
		isDamager = m_damagers->find(networkId) != m_damagers->end();
	}

	return isDamager;
}

//-----------------------------------------------------------------

UIPage *SwgCuiAllTargetsSpace::getShipArrowPage()
{
	UIPage * uiPage = NULL;

	if (m_damageDirectionArrow)
	{
		if (m_arrowPages->empty())
		{
			uiPage = safe_cast<UIPage *>(m_damageDirectionArrow->DuplicateObject());
			NOT_NULL(uiPage);
			uiPage->Attach(0);
			getPage().AddChild(uiPage);
			uiPage->SetName("waypoint_space_damage");
			uiPage->Link();
			uiPage->Center();
		}
		else
		{
			uiPage = m_arrowPages->top();
			m_arrowPages->pop();
		}

		uiPage->SetVisible(true);
		uiPage->SetEnabled(true);
	}

	return uiPage;
}

//-----------------------------------------------------------------

void SwgCuiAllTargetsSpace::joystickImageConstruct()
{
	for(size_t imageCount = 0; imageCount< s_virtualjoystickImageCount; ++imageCount)
	{
		char imageName[128];
		sprintf(imageName, s_virtualjoystickSourceName, imageCount);

		UIImage * image = safe_cast<UIImage *>(m_virtualJoystickImage->DuplicateObject());
		NOT_NULL(image);
		IGNORE_RETURN(image->SetSourceResource(UIUnicode::narrowToWide(imageName)));
		image->SetName("virtualJoystickImage");
		image->Attach(0);
		image->SetVisible(true);
		getPage().AddChild(image);
		image->Center();
		m_virtualJoystickImages->push_back(image);
	}
}

//-----------------------------------------------------------------

void SwgCuiAllTargetsSpace::joystickImageRelease()
{
	for(ImageVector::iterator itVirtJoy = m_virtualJoystickImages->begin(); 
		itVirtJoy != m_virtualJoystickImages->end(); 
		++itVirtJoy)
	{
		UIImage * const image = *itVirtJoy;
		getPage().RemoveChild(image);
		image->Detach(0);
	}

	m_virtualJoystickImages->clear();
}


//-----------------------------------------------------------------

void SwgCuiAllTargetsSpace::joystickImageHide(bool hideEm)
{
	for(ImageVector::iterator itVirtJoy = m_virtualJoystickImages->begin(); 
	itVirtJoy != m_virtualJoystickImages->end(); 
	++itVirtJoy)
	{
		UIImage * const image = *itVirtJoy;
		if (image)
		{
			if (itVirtJoy == m_virtualJoystickImages->begin())
			{
				if((image->IsVisible() && hideEm) || (!image->IsVisible() && !hideEm))
					m_virtualJoystickReset = true;
				else
					m_virtualJoystickReset = false;
			}

			image->SetVisible(!hideEm);
		}
	}
}

//-----------------------------------------------------------------

void SwgCuiAllTargetsSpace::joystickImageUpdate(UIPoint const & uiPosition, UIPoint const & centerPosition)
{
	UIPoint previousPosition = uiPosition;
	UIPoint newPosition;

	for(ImageVector::iterator itVirtJoy = m_virtualJoystickImages->begin(); 
		itVirtJoy != m_virtualJoystickImages->end(); 
		++itVirtJoy)
	{
		UIImage * const objectImage = *itVirtJoy;
		if(objectImage)
		{
			UISize const & objectWidth = objectImage->GetSize() / 2;

			if (m_virtualJoystickReset || (itVirtJoy == m_virtualJoystickImages->begin()))
			{
				objectImage->SetLocation(previousPosition - objectWidth);
			}
			else
			{
				float const delta = m_timeDelta * s_virtualjoystickDelta;
				newPosition = UIPoint::lerp(objectImage->GetLocation() + objectWidth, centerPosition, delta * s_virtualjoystickCenterDelta);
				newPosition = UIPoint::lerp(newPosition, previousPosition, delta);
				objectImage->SetLocation(newPosition - objectWidth);
				previousPosition = newPosition;
			}

			objectImage->SetVisible(true);
			objectImage->SetOpacity(s_virtualJoystickFadeOpacity);
		}
	}

	m_virtualJoystickReset = false;
}

//-----------------------------------------------------------------

/**
 * Test a waypoint to see if it should be drawn, and move it to a position within
 * the camera's frustum if appropriate.
 */
bool SwgCuiAllTargetsSpace::isWaypointVisible (Vector & effectiveWaypointPosition_o, Camera const & camera, Appearance const * /*appearance*/) const
{
	if (effectiveWaypointPosition_o.z > camera.getFarPlane())
	{
		// Move to 90% of the far plane distance.  Not putting it right at the farplane to 
		// avoid crazy rounding problems that might push it past the plane
		float const factor = (camera.getFarPlane() * 0.9f) / effectiveWaypointPosition_o.z;
		effectiveWaypointPosition_o *= factor;
	}

	Volume const & cameraVolume = camera.getFrustumVolume();
	return cameraVolume.contains (effectiveWaypointPosition_o);
}


//======================================================================
