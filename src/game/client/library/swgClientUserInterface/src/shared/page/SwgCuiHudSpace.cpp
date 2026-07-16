//======================================================================
//
// SwgCuiHudSpace.cpp
// copyright (c) 2004 Sony Online Entertainment
//
//======================================================================

#include "swgClientUserInterface/FirstSwgClientUserInterface.h"
#include "swgClientUserInterface/SwgCuiHudSpace.h"

#include "clientGame/ConfigClientGame.h"
#include "clientGame/CreatureObject.h"
#include "clientGame/Game.h"
#include "clientGame/NebulaManagerClient.h"
#include "clientGame/ShipObject.h"
#include "clientGame/ShipObjectEffects.h"
#include "clientGame/ShipStation.h"
#include "clientGame/SpaceTargetBracketOverlay.h"
#include "clientUserInterface/CuiDamageManager.h"
#include "clientUserInterface/CuiGameColorManager.h"
#include "clientUserInterface/CuiUtils.h"
#include "sharedDebug/DebugFlags.h"
#include "sharedFile/Iff.h"
#include "sharedMath/VectorArgb.h"
#include "sharedMessageDispatch/Transceiver.h"
#include "sharedNetworkMessages/CreateMissileMessage.h"
#include "sharedNetworkMessages/ShipDamageMessage.h"
#include "sharedUtility/DataTable.h"
#include "swgClientUserInterface/SwgCuiHudActionSpace.h"
#include "swgClientUserInterface/SwgCuiHudWindowManagerSpace.h"

#include "UIButton.h"
#include "UIDeformer.h"
#include "UIEffector.h"

//======================================================================

namespace SwgCuiHudSpaceNamespace
{
	std::string const name_hudPage         = "HudSpace";
	std::string const name_hudPageTemplate = "templateHudSpace";

	float const c_damagePageOpacityMaximum = 1.0f;
	float const c_damagePageOpacityDecay = 0.75f;
	float const c_blinkDecaySeconds = 3.0f;

	typedef std::multimap<SwgCuiHudSpace::ChassisAndStationPair, SwgCuiHudSpace::ShipPageAndStatusPair> SpaceHudPageStates;

	bool s_drawHudSpaceStateDrivenWidgets = false;

	bool s_reticleVisible = true;
}


using namespace SwgCuiHudSpaceNamespace;

//----------------------------------------------------------------------

SwgCuiHudSpace::SwgCuiHudSpace(UIPage & page) :
	SwgCuiHud(page),
	m_damagePage(NULL),
	m_nebulaHuePage(NULL),
	m_callback(new MessageDispatch::Callback),
	m_spaceHudPageStates(new SpaceHudPageStates()),
	m_ejectButton(NULL),
	m_exitStationButton(NULL),
	m_tutorialButton(NULL),
	m_buttonParentPage(NULL),
	m_missileLockOnYouPage(NULL),
	m_missileLockTimer(c_blinkDecaySeconds),
	m_missileLockWidget(NULL),
	m_missileLockEffector(NULL),
	m_enterSpaceButton(NULL)
{ 
	m_targetingEnabled = false;

	DebugFlags::registerFlag(s_drawHudSpaceStateDrivenWidgets, "ClientUserInterface", "drawHudSpaceStateDrivenWidgets");

	setWindowManager(new SwgCuiHudWindowManagerSpace(*this, getWorkspace()));
	setAction(new SwgCuiHudActionSpace(*this));

	setHudEnabled(!ConfigClientGame::getHudDisabled());

	// Space HUD widgets are looked up optionally and guarded so a UI data set
	// missing one of them degrades gracefully instead of crashing the ctor.
	getCodeDataObject(TUIPage, m_damagePage, "DamagePage", true);
	if (m_damagePage)
	{
		m_damagePage->SetVisible(false);
		UIBaseObject * const damageParent = m_damagePage->GetParent();
		if (damageParent)
		{
			m_nebulaHuePage = safe_cast<UIPage *>(m_damagePage->DuplicateObject());
			safe_cast<UIPage *>(damageParent)->InsertChildBefore(m_nebulaHuePage, m_damagePage);
			m_nebulaHuePage->SetPropertyNarrow(UIWidget::PropertyName::PackSize, "1,1");
			m_nebulaHuePage->SetBackgroundTint(UIColor::white);
			m_nebulaHuePage->SetBackgroundColor(UIColor::white);
			m_nebulaHuePage->SetBackgroundOpacity(1.0f);
			m_nebulaHuePage->SetVisible(false);
		}
	}
	else
		WARNING(true, ("SwgCuiHudSpace: space HUD page missing widget 'DamagePage'; HUD running degraded"));

	getCodeDataObject(TUIButton, m_ejectButton, "buttonEject", true);
	if (m_ejectButton)
		m_ejectButton->SetVisible(false);

	getCodeDataObject(TUIButton, m_exitStationButton, "buttonExitStation", true);
	if (m_exitStationButton)
		m_exitStationButton->SetVisible(false);

	getCodeDataObject(TUIButton, m_tutorialButton, "trainerButton", true);
	if (m_tutorialButton)
		m_tutorialButton->SetVisible(false);

	getCodeDataObject(TUIButton, m_enterSpaceButton, "buttonEnterSpace", true);
	if (m_enterSpaceButton)
		m_enterSpaceButton->SetVisible(false);

	if (m_exitStationButton)
	{
		m_buttonParentPage = static_cast<UIPage*>(m_exitStationButton->GetParent(TUIPage));
		if (m_buttonParentPage)
		{
			m_buttonParentPage->SetVisible(true);
			m_buttonParentPage->Link();
		}
	}

	getCodeDataObject(TUIPage, m_missileLockWidget, "missileLockOnYou", true);
	if (m_missileLockWidget)
		m_missileLockWidget->SetVisible(false);

	getCodeDataObject(TUIEffector, m_missileLockEffector, "missileLockOnYouDeformer", true);

	m_callback->connect(*this, &SwgCuiHudSpace::onCreateMissileMessage);
}

//----------------------------------------------------------------------

SwgCuiHudSpace::~SwgCuiHudSpace()
{
	m_damagePage = NULL;
	m_nebulaHuePage = NULL;

	delete m_spaceHudPageStates;
	m_spaceHudPageStates = NULL;

	m_ejectButton = NULL;
	m_exitStationButton = NULL;
	m_buttonParentPage = NULL;
	m_missileLockOnYouPage = NULL;

	m_callback->disconnect(*this, &SwgCuiHudSpace::onCreateMissileMessage);

	delete m_callback;
	m_callback = NULL;
}

//----------------------------------------------------------------------

void SwgCuiHudSpace::createHudTemplate()
{
	internalCreateHudTemplate(name_hudPage, name_hudPageTemplate);
}

//----------------------------------------------------------------------

void SwgCuiHudSpace::createFreshHud()
{
	internalCreateFreshHud(name_hudPage, name_hudPageTemplate);
}

//----------------------------------------------------------------------

bool SwgCuiHudSpace::shouldRenderReticle() const
{
	return false;
}

//----------------------------------------------------------------------

void SwgCuiHudSpace::performActivate()
{
	SwgCuiHud::performActivate();
	
	m_callback->connect(*this, &SwgCuiHudSpace::onShipDamaged, static_cast<CuiDamageManager::Messages::ShipDamage *>(0));

	// Set element availability based on ship chassis and active station
	ShipObject const * const ship = Game::getPlayerContainingShip();
	CreatureObject const * const creature = Game::getPlayerCreature();
	if (ship && creature)
	{
		int station = creature->getShipStation();
		if (station != ShipStation::ShipStation_None)
		{
			if (station >= ShipStation::ShipStation_Gunner_First && station <= ShipStation::ShipStation_Gunner_Last)
				station = ShipStation::ShipStation_Gunner_First;
			ChassisAndStationPair const chassisAndStation(ship->getChassisType(), station);
			std::pair<SpaceHudPageStates::iterator, SpaceHudPageStates::iterator> range = m_spaceHudPageStates->equal_range(chassisAndStation);
			for (SpaceHudPageStates::iterator i = range.first; i != range.second; ++i)
			{
				std::string const &pageName = (*i).second.first;
				bool const enable = (*i).second.second;
				UIPage *mediatorPage = 0;
				getCodeDataObject(TUIPage, mediatorPage, pageName.c_str(), true);
				if (mediatorPage)
					mediatorPage->SetVisible(enable);
			}
		}
	}

	SwgCuiHudWindowManagerSpace * const windowManager = safe_cast<SwgCuiHudWindowManagerSpace *>(getWindowManager());
	if (windowManager)
	{
		const bool chatWindowActive = windowManager->isSpaceChatVisible();
		windowManager->activateChatWindow(chatWindowActive);
	}
}

//----------------------------------------------------------------------

void SwgCuiHudSpace::performDeactivate()
{
	SwgCuiHud::performDeactivate();
	
	m_callback->disconnect(*this, &SwgCuiHudSpace::onShipDamaged, static_cast<CuiDamageManager::Messages::ShipDamage *>(0));
}

//----------------------------------------------------------------------

void  SwgCuiHudSpace::onShipDamaged(CuiDamageManager::Messages::ShipDamage::Payload const &shipDamage)
{
	if (m_damagePage != NULL)
	{
		if (!shipDamage.getIsLocal() || Game::getSinglePlayer())
		{
			// Update damage opacity.
			m_damagePage->SetOpacity(c_damagePageOpacityMaximum);
			m_damagePage->SetVisible(true);
		}
	}
}

//----------------------------------------------------------------------

void SwgCuiHudSpace::update(float const updateDeltaSeconds)
{
	SwgCuiHud::update(updateDeltaSeconds);

	//-- Update damage page opacity.

	SwgCuiHudWindowManagerSpace * const windowManager = safe_cast<SwgCuiHudWindowManagerSpace *>(getWindowManager());
	if (NULL != windowManager)
		windowManager->setReticleVisible(s_reticleVisible);

	if (m_damagePage != NULL)
	{
		float const damagePageCurrentOpacity = m_damagePage->GetOpacity() -(c_damagePageOpacityDecay * updateDeltaSeconds);
		if (damagePageCurrentOpacity < 0.0f)
		{
			m_damagePage->SetVisible(false);
		}
		else
		{
			m_damagePage->SetOpacity(damagePageCurrentOpacity);
		}
	}
	
	//-- update nebula hueing
	
	VectorArgb const & nebulaCameraHue = NebulaManagerClient::getCurrentNebulaCameraHue();
	UIColor const & color = CuiUtils::convertColor(nebulaCameraHue);
	float const opacity = nebulaCameraHue.a * 0.25f;
	
		if (m_nebulaHuePage)
	{
if (opacity < 0.01f)
		m_nebulaHuePage->SetVisible(false);
	else
	{
		m_nebulaHuePage->SetVisible(true);		
		m_nebulaHuePage->SetColor(color);
		m_nebulaHuePage->SetOpacity(opacity);
	}

	}


	// Update the buttons.
		if (m_ejectButton && m_exitStationButton && m_tutorialButton && m_buttonParentPage)
	{
CreatureObject const * const player = Game::getPlayerCreature();
	bool canEject = false;
	bool canExitStation = false;
	bool canTutor = false;

	if (player)
	{
		// Tutorial on player.
		canTutor = canTutor || player->hasCondition(TangibleObject::C_inflightTutorial);

		ShipObject const * const shipObject = Game::getPlayerContainingShip();
		if(shipObject)
		{
			// Check for eject!!!!
			canEject = shipObject->hasCondition(TangibleObject::C_eject);
			bool const wasEjectVisible = m_ejectButton->IsVisible();
			
			if (canEject != wasEjectVisible) //lint !e731
			{
				m_buttonParentPage->SetPackDirty(true);
			}
			
			// Update the exit station icon.
			bool const isPobShip = shipObject->isPobShip();
			ShipStation::Type const shipStation = static_cast<ShipStation::Type>(player->getShipStation());
			canExitStation =  isPobShip && shipStation != ShipStation::ShipStation_None;
			bool const wasExitStationVisible = m_exitStationButton->IsVisible();
			
			if (canExitStation != wasExitStationVisible) //lint !e731
			{
				m_buttonParentPage->SetPackDirty(true);
			}

			// Tutor on ship.
			canTutor = canTutor || shipObject->hasCondition(TangibleObject::C_inflightTutorial);
		}
	}

	// Check for the trainer thingy.
	bool const wasTutorVisible = m_tutorialButton->IsVisible();
	if (canTutor != wasTutorVisible) //lint !e731
	{
		m_buttonParentPage->SetPackDirty(true);
	}

	if (s_drawHudSpaceStateDrivenWidgets)
	{
		m_exitStationButton->SetVisible(true);
		m_ejectButton->SetVisible(true);
		m_tutorialButton->SetVisible(true);
		m_buttonParentPage->SetVisible(true);
	}
	else
	{
		m_exitStationButton->SetVisible(canExitStation);
		m_ejectButton->SetVisible(canEject);
		m_tutorialButton->SetVisible(canTutor);
		m_buttonParentPage->SetVisible(getHudEnabled());
	}

	}

	//-- Do the missile lock blinky thing.
	{
		bool const missileTimerFired = m_missileLockTimer.updateNoReset(updateDeltaSeconds);
		if (missileTimerFired && m_missileLockWidget && m_missileLockWidget->IsVisible())
		{
			m_missileLockWidget->SetVisible(false);
			if (m_missileLockEffector)
				m_missileLockWidget->CancelEffector(*m_missileLockEffector);
		}
	}

	if (s_drawHudSpaceStateDrivenWidgets)
	{
		ShipObject const * const playerShip = Game::getPlayerPilotedShip();
		if (playerShip)
		{
			CreateMissileMessage createMissileMessage(0, playerShip->getNetworkId(), playerShip->getNetworkId(), Vector::zero, playerShip->getPosition_w(), 0, 0, 0, 0);
			onCreateMissileMessage(createMissileMessage);
		}
	}

	SpaceTargetBracketOverlay::setDisabled(!getHudEnabled());
	ShipObjectEffects::setHideAllTargetEffects(!getHudEnabled());
}

//----------------------------------------------------------------------

void SwgCuiHudSpace::onCreateMissileMessage(const CreateMissileMessage & createMissileMessage)
{
	ShipObject const * const playerShip = Game::getPlayerContainingShip();
	
	if (playerShip && playerShip->getNetworkId() == createMissileMessage.getTarget() && m_missileLockTimer.isExpired())
	{
		m_missileLockTimer.reset();
		if (m_missileLockWidget && m_missileLockEffector)
		{
			m_missileLockWidget->SetVisible(true);
			m_missileLockWidget->CancelEffector(*m_missileLockEffector);
			m_missileLockWidget->ExecuteEffector(*m_missileLockEffector);
		}
	}
}

//----------------------------------------------------------------------

void SwgCuiHudSpace::setReticleVisible(bool b)
{
	s_reticleVisible = b;
}

//======================================================================
