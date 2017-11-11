//======================================================================
//
// SwgCuiHudWindowManagerGround.cpp
// copyright (c) 2004 Sony Online Entertainment
//
//======================================================================

#include "swgClientUserInterface/FirstSwgClientUserInterface.h"
#include "swgClientUserInterface/SwgCuiHudWindowManagerGround.h"

#include "clientGame/Game.h"
#include "clientGame/PlayerCreatureController.h"
#include "clientGame/PlayerObject.h"
#include "clientGame/TangibleObject.h"
#include "clientTerrain/ClientProceduralTerrainAppearance.h"
#include "clientUserInterface/CuiActionManager.h"
#include "clientUserInterface/CuiActions.h"
#include "clientUserInterface/CuiMediatorFactory.h"
#include "clientUserInterface/CuiMessageBox.h"
#include "clientUserInterface/CuiPreferences.h"
#include "clientUserInterface/CuiWorkspace.h"
#include "sharedFoundation/GameControllerMessage.h"
#include "sharedMessageDispatch/Message.h"
#include "sharedMessageDispatch/Transceiver.h"
#include "sharedNetworkMessages/MessageQueueGenericValueType.h"
#include "sharedNetworkMessages/NewbieTutorialEnableHudElement.h"
#include "sharedObject/NetworkIdManager.h"
#include "sharedTerrain/TerrainObject.h"
#include "swgClientUserInterface/SwgCuiAllTargetsGround.h"
#include "swgClientUserInterface/SwgCuiCybernetics.h"
#include "swgClientUserInterface/SwgCuiDroidCommand.h"
#include "swgClientUserInterface/SwgCuiGroundRadar.h"
#include "swgClientUserInterface/SwgCuiGroup.h"
#include "swgClientUserInterface/SwgCuiGroupLootLottery.h"
#include "swgClientUserInterface/SwgCuiHud.h"
#include "swgClientUserInterface/SwgCuiHudFactory.h"
#include "swgClientUserInterface/SwgCuiHudGround.h"
#include "swgClientUserInterface/SwgCuiMediatorTypes.h"
#include "swgClientUserInterface/SwgCuiShipChoose.h"
#include "swgClientUserInterface/SwgCuiShipView.h"
#include "swgClientUserInterface/SwgCuiStatusGround.h"
#include "swgClientUserInterface/SwgCuiSurvey.h"
#include "swgClientUserInterface/SwgCuiToolbar.h"
#include "swgSharedNetworkMessages/ResourceListForSurveyMessage.h"

#include "UIData.h"
#include "UIPage.h"

//======================================================================

namespace SwgCuiHudWindowManagerGroundNamespace
{
	const std::string cms_newbieTutorialEnableHudElementRadar      ("radar");
	const std::string cms_newbieTutorialEnableHudElementAll        ("all");

	typedef std::map<NetworkId /* container */, int /* number of attempts */> LotteryAttempts;
	LotteryAttempts s_lotteryAttempts;
	int const cs_numberOfFramesToAttemptToOpenLottery = 256;
}

using namespace SwgCuiHudWindowManagerGroundNamespace;

//----------------------------------------------------------------------

SwgCuiHudWindowManagerGround::SwgCuiHudWindowManagerGround (const SwgCuiHud & hud, CuiWorkspace & workspace) :
SwgCuiHudWindowManager(hud, workspace),
m_groundRadarMediator(NULL),
m_targetStatusPage(NULL),
m_secondaryTargetStatusPage(NULL),
m_playerStatusPage(NULL),
m_petStatusPage(NULL)
{
	UIPage * mediatorPage = NULL;

	{
		hud.getCodeDataObject (TUIPage,     mediatorPage,           "RadarPage");
		mediatorPage->SetEnabled (true);
		mediatorPage->SetEnabled (false);
		m_groundRadarMediator = new SwgCuiGroundRadar (*mediatorPage);
		m_groundRadarMediator->setSettingsAutoSizeLocation (true, true);
		m_groundRadarMediator->setStickyVisible (true);
		m_groundRadarMediator->fetch ();
		m_groundRadarMediator->activate ();
		getWorkspace().addMediator (*m_groundRadarMediator);
	}

	{
		hud.getCodeDataObject (TUIPage,     mediatorPage,           "AllTargets");
		mediatorPage->SetEnabled (false);
		SwgCuiAllTargets * const allTargets = new SwgCuiAllTargetsGround(*mediatorPage);
		allTargets->setStickyVisible (true);
		allTargets->activate ();
		getWorkspace().addMediator (*allTargets);
	} //lint !e429 custodial pointer not freed or returned.  The Workspace owns it.

	{
		hud.getCodeDataObject(TUIPage, mediatorPage, "group");
		mediatorPage->SetEnabled(false);
		SwgCuiGroup * const group = new SwgCuiGroup(*mediatorPage);
		group->setSettingsAutoSizeLocation(true, true);
		group->setStickyVisible(true);
		group->activate();
		getWorkspace().addMediator(*group);
	} //lint !e429 custodial pointer not freed or returned.  The Workspace owns it.

	{
		hud.getCodeDataObject (TUIPage, mediatorPage, "TargetsPage");
		m_targetStatusPage = new SwgCuiStatusGround(*mediatorPage, SwgCuiStatusGround::ST_intendedTarget);
		if (m_targetStatusPage != NULL)
		{
			mediatorPage->SetEnabled(false);
			mediatorPage->SetEnabled(true);

			CreatureObject * const player = Game::getPlayerCreature();
			NOT_NULL(player);
			// double tap for widget state
			m_targetStatusPage->setTarget(player);
			m_targetStatusPage->setTarget(NetworkId::cms_invalid);
			m_targetStatusPage->setSettingsAutoSizeLocation(true, true);
			m_targetStatusPage->setStickyVisible(true);
			m_targetStatusPage->fetch();
			m_targetStatusPage->activate();
			m_targetStatusPage->setShowFocusedGlowRect(false);
			getWorkspace().addMediator(*m_targetStatusPage);
		}
	}

	{
		hud.getCodeDataObject (TUIPage, mediatorPage, "Pet");
		m_petStatusPage = new SwgCuiStatusGround(*mediatorPage, SwgCuiStatusGround::ST_pet);
		if (m_petStatusPage != NULL)
		{
			mediatorPage->SetEnabled(false);
			mediatorPage->SetEnabled(true);

			CreatureObject * const player = Game::getPlayerCreature();
			NOT_NULL(player);
			// double tap for widget state
			m_petStatusPage->setTarget(player);
			m_petStatusPage->setTarget(NetworkId::cms_invalid);
			m_petStatusPage->setSettingsAutoSizeLocation(false, true);
			m_petStatusPage->setStickyVisible(true);
			m_petStatusPage->fetch();
			m_petStatusPage->activate();
			m_petStatusPage->setShowFocusedGlowRect(false);

			getWorkspace().addMediator(*m_petStatusPage);
			if(Game::getPlayerObject())
				onPetChanged(*Game::getPlayerObject());

		}
	}

	{
		hud.getCodeDataObject (TUIPage, mediatorPage, "SecondaryTargetsPage");
		m_secondaryTargetStatusPage = new SwgCuiStatusGround(*mediatorPage, SwgCuiStatusGround::ST_lookAtTarget);
		if (m_secondaryTargetStatusPage != NULL)
		{
			mediatorPage->SetEnabled(false);
			mediatorPage->SetEnabled(true);

			CreatureObject * const player = Game::getPlayerCreature();
			NOT_NULL(player);
			// double tap for widget state
			m_secondaryTargetStatusPage->setTarget(player);
			m_secondaryTargetStatusPage->setTarget(NetworkId::cms_invalid);
			m_secondaryTargetStatusPage->setSettingsAutoSizeLocation(true, true);
			m_secondaryTargetStatusPage->setStickyVisible(true);
			m_secondaryTargetStatusPage->fetch();
			m_secondaryTargetStatusPage->activate();
			m_secondaryTargetStatusPage->setShowFocusedGlowRect(false);
			getWorkspace().addMediator(*m_secondaryTargetStatusPage);
		}
	}

	UIPage * statusPage = 0;
	hud.getCodeDataObject(TUIPage, statusPage, "MfdStatusPage");
	statusPage->SetVisible(false);

	connectToMessage(ResourceListForSurveyMessage::MessageType);
	connectToMessage(SurveyMessage::MessageType);
	connectToMessage(ResourceListForSurveyMessage::MessageType);
	connectToMessage(Game::Messages::SCENE_CHANGED);
}

//----------------------------------------------------------------------

SwgCuiHudWindowManagerGround::~SwgCuiHudWindowManagerGround ()
{
	getWorkspace().removeMediator(*m_groundRadarMediator);
	m_groundRadarMediator->release();
	m_groundRadarMediator = 0;

	getWorkspace().removeMediator(*m_targetStatusPage);
	m_targetStatusPage->release();
	m_targetStatusPage = 0;

	getWorkspace().removeMediator(*m_secondaryTargetStatusPage);
	m_secondaryTargetStatusPage->release();
	m_secondaryTargetStatusPage = 0;

	getWorkspace().removeMediator(*m_petStatusPage);
	m_petStatusPage->release();
	m_petStatusPage = 0;

	removePlayerStatusPage();
}

//----------------------------------------------------------------------

void SwgCuiHudWindowManagerGround::spawnSurvey ()
{
	CuiMediator * const mediator = getWorkspace().findMediatorByType (typeid (SwgCuiSurvey));

	if (mediator)
	{
		toggleMediator (*mediator);
	}
	else
	{
		SwgCuiSurvey * const survey = SwgCuiSurvey::createInto (getWorkspace().getPage ());
		survey->setSettingsAutoSizeLocation (true, true);
		getWorkspace().addMediator (*survey);
		survey->activate ();
		getWorkspace().focusMediator (*survey, true);
		survey->setEnabled (true);
	}
}

//----------------------------------------------------------------------

void SwgCuiHudWindowManagerGround::spawnShipChoose(NetworkId const & terminalId) const
{
	if(Game::isSpace())
		return;

	SwgCuiShipChoose * const mediator = safe_cast<SwgCuiShipChoose * const>(CuiMediatorFactory::activateInWorkspace (CuiMediatorTypes::WS_ShipChoose));
	if(mediator)
		mediator->setTerminal(terminalId);
}

//----------------------------------------------------------------------

void SwgCuiHudWindowManagerGround::spawnShipView(NetworkId const & shipId, NetworkId const & terminalId) const
{
	if(Game::isSpace())
		return;

	Object * const o = NetworkIdManager::getObjectById(shipId);
	ClientObject * const co = o ? o->asClientObject() : NULL;
	ShipObject * const ship = co ? co->asShipObject() : NULL;
	if(ship)
	{
		SwgCuiShipView * const mediator = safe_cast<SwgCuiShipView * const>(CuiMediatorFactory::activateInWorkspace (CuiMediatorTypes::WS_ShipView));
		if(mediator)
		{
			mediator->setShip(ship);
			mediator->setTerminal(terminalId);
		}
	}
}

//----------------------------------------------------------------------

void SwgCuiHudWindowManagerGround::spawnDroidCommand(NetworkId const & droidControlDeviceId) const
{
	Object * const o = NetworkIdManager::getObjectById(droidControlDeviceId);
	ClientObject * const droidControlDeviceClientObject = o ? o->asClientObject() : NULL;
	if(droidControlDeviceClientObject)
	{
		SwgCuiDroidCommand * const mediator = safe_cast<SwgCuiDroidCommand * const>(CuiMediatorFactory::activateInWorkspace (CuiMediatorTypes::WS_DroidCommand));
		if(mediator)
		{
			mediator->setDroidControlDevice(*droidControlDeviceClientObject);
		}
	}
}

//----------------------------------------------------------------------

void SwgCuiHudWindowManagerGround::spawnGroupLootLottery(NetworkId const & container)
{
	Object * const containerObject = NetworkIdManager::getObjectById(container);
	ClientObject * const containerClientObject = (containerObject != 0) ? containerObject->asClientObject() : 0;

	if (containerClientObject != 0)
	{
		LotteryAttempts::iterator ii = s_lotteryAttempts.find(container);
		if (ii != s_lotteryAttempts.end())
		{
			IGNORE_RETURN(s_lotteryAttempts.erase(ii));
		}

		SwgCuiGroupLootLottery * const mediator = SwgCuiGroupLootLottery::createInto(getWorkspace().getPage(), *containerClientObject);

		if (mediator != 0)
		{
			getWorkspace().addMediator(*mediator);
			mediator->fetch();
			mediator->activate();
			mediator->setEnabled(true);
			getWorkspace().focusMediator(*mediator, true);
		}

		NOT_NULL(mediator);
	}
	else
	{
		int const attempts = s_lotteryAttempts[container];

		if (attempts > cs_numberOfFramesToAttemptToOpenLottery)
		{
			LotteryAttempts::iterator ii = s_lotteryAttempts.find(container);
			if (ii != s_lotteryAttempts.end())
			{
				IGNORE_RETURN(s_lotteryAttempts.erase(ii));
			}
		}
		else
		{
			s_lotteryAttempts[container] = attempts + 1;

			CreatureObject * const player = Game::getPlayerCreature();
			Controller * const controller = (player != 0) ? player->getController() : 0;

			if (controller != 0)
			{
				typedef MessageQueueGenericValueType<NetworkId> Message;
				Message * const message = new Message(container);

				controller->appendMessage(
					CM_groupOpenLotteryWindowOnClient,
					0.0f,
					message,
					GameControllerMessageFlags::SEND |
					GameControllerMessageFlags::RELIABLE |
					GameControllerMessageFlags::DEST_AUTH_CLIENT);
			}
		}
	}
}

//----------------------------------------------------------------------

void SwgCuiHudWindowManagerGround::closeGroupLootLottery(NetworkId const & container)
{
	Object * const containerObject = NetworkIdManager::getObjectById(container);
	ClientObject * const containerClientObject = (containerObject != 0) ? containerObject->asClientObject() : 0;

	if (containerClientObject != 0)
	{
		// close if it exists
		IGNORE_RETURN(SwgCuiGroupLootLottery::closeForContainer(*containerClientObject));
	}
} //lint !e1762 // logically nonconst

//----------------------------------------------------------------------

void SwgCuiHudWindowManagerGround::spawnCybernetics(NetworkId const & npc, MessageQueueCyberneticsOpen::OpenType const openType) const
{
	SwgCuiCybernetics * const mediator = safe_cast<SwgCuiCybernetics * const>(CuiMediatorFactory::activateInWorkspace (CuiMediatorTypes::WS_Cybernetics));
	if(mediator)
	{
		mediator->setNPC(npc);
		mediator->setPageOpenType(openType);
	}
}

//----------------------------------------------------------------------

void SwgCuiHudWindowManagerGround::receiveMessage(const MessageDispatch::Emitter & emitter, const MessageDispatch::MessageBase & message)
{	
	if (message.isType (ResourceListForSurveyMessage::MessageType))
	{
		spawnSurvey();
		Archive::ReadIterator ri = NON_NULL (safe_cast<const GameNetworkMessage *>(&message))->getByteStream().begin();
		const ResourceListForSurveyMessage rlfsm(ri);
		SwgCuiSurvey * const survey = safe_cast<SwgCuiSurvey *>(getWorkspace().findMediatorByType (typeid (SwgCuiSurvey)));
		if(survey)
			survey->setResourceData(rlfsm);
	}

	else if  (message.isType (SurveyMessage::MessageType))
	{
		Archive::ReadIterator ri = NON_NULL (safe_cast<const GameNetworkMessage *>(&message))->getByteStream().begin();
		const SurveyMessage sm(ri);
		SwgCuiSurvey * const survey = safe_cast<SwgCuiSurvey *>(getWorkspace().findMediatorByType (typeid (SwgCuiSurvey)));
		if(survey)
			survey->setSurveyData(sm);
	}

	else if (message.isType (NewbieTutorialEnableHudElement::cms_name))
	{
		//-- what type of request is it?
		Archive::ReadIterator ri = NON_NULL (safe_cast<const GameNetworkMessage*> (&message))->getByteStream ().begin ();
		const NewbieTutorialEnableHudElement newbieTutorialEnableHudElement (ri);

		if (newbieTutorialEnableHudElement.getName () == cms_newbieTutorialEnableHudElementRadar || newbieTutorialEnableHudElement.getName () == cms_newbieTutorialEnableHudElementAll)
		{
			setBlinkingMediator (*m_groundRadarMediator, newbieTutorialEnableHudElement.getBlinkTime ());

			if (newbieTutorialEnableHudElement.getEnable ())
			{
				if (!m_groundRadarMediator->isActive ())
					m_groundRadarMediator->activate ();
			}
			else
			{
				if (m_groundRadarMediator->isActive ())
					m_groundRadarMediator->deactivate ();
			}
		}
	}

	else if (message.isType (Game::Messages::SCENE_CHANGED))
	{
		if (m_groundRadarMediator)
			m_groundRadarMediator->onSceneChanged ();
	}

	SwgCuiHudWindowManager::receiveMessage(emitter, message);
}

//----------------------------------------------------------------------

void SwgCuiHudWindowManagerGround::update ()
{
	const TerrainObject * const terrainObject = TerrainObject::getInstance();
	const ClientProceduralTerrainAppearance * const cmtat = terrainObject ? dynamic_cast<const ClientProceduralTerrainAppearance *> (terrainObject->getAppearance ()) : 0;			

	if (cmtat && Game::getPlayer())
	{	
		SwgCuiSurvey * const survey = safe_cast<SwgCuiSurvey *>(getWorkspace().findMediatorByType (typeid (SwgCuiSurvey)));
		if(survey && survey->isActive() && survey->hasSurvey())
		{
			survey->updateMap (*cmtat);
		}
	}

	// NOTE RHanz 4/18/07:
	// We're looking up an object by network id here.  Storing this as a CachedNetworkId is a potential
	// performance improvement.  Since NetworkIds, cached or not, are always sent across the wire as
	// NetworkIds, it *may* be sufficient to only change PlayerObject on the client side to use a
	// CachedNetworkId.

	// this block makes sure that the object that the pet target page is looking at is still valid.
	// similar blocks exist in CreatureObject for the intended/lookat targets, but those set the
	// data itself to null.  Since the petId is (currently) a one-way communication, I chose to
	// just set the mfd's target to null.

	// this should be safe.  However, design needs to make sure that all pet leashes act properly.

	if(m_petStatusPage && Game::getPlayerObject() && Game::getPlayerObject()->getPetId() != NetworkId::cms_invalid)
	{
		Object * const petObject = NetworkIdManager::getObjectById(Game::getPlayerObject()->getPetId());		
		if(!petObject)
		{
			m_petStatusPage->setTarget(NetworkId::cms_invalid);
		}
	}

	updateTargetStatusPages();

	SwgCuiHudWindowManager::update();
}

//----------------------------------------------------------------------

bool SwgCuiHudWindowManagerGround::isShowingLookAtTarget() const
{
	bool ret = false;

	if (m_targetStatusPage) 
	{
		ret = m_targetStatusPage->getTarget().isValid();
	}

	return ret;
}

//----------------------------------------------------------------------

void SwgCuiHudWindowManagerGround::clearLookAtTarget()
{
	if (m_targetStatusPage) 
	{
		m_targetStatusPage->clearTarget();
	}
}


//----------------------------------------------------------------------

void SwgCuiHudWindowManagerGround::updateTargetStatusPages()
{
	if (m_targetStatusPage && m_secondaryTargetStatusPage)
	{
		CreatureObject const * const playerCreature = Game::getPlayerCreature();
		if (playerCreature) 
		{
			CachedNetworkId const intendedTarget = playerCreature->getIntendedTarget();
			m_targetStatusPage->setTarget(intendedTarget);

			switch (CuiPreferences::getSecondaryTargetMode())
			{
			case CuiPreferences::STM_lookAtTarget:
				m_secondaryTargetStatusPage->setTarget(playerCreature->getLookAtTarget());
				break;
			case CuiPreferences::STM_targetOfTarget:
				{
					CreatureObject const * const intendedTargetObject = CreatureObject::asCreatureObject(intendedTarget.getObject());

					if (intendedTargetObject)
						m_secondaryTargetStatusPage->setTarget(intendedTargetObject->isPlayer() ? intendedTargetObject->getIntendedTarget() : intendedTargetObject->getLookAtTarget());
					else
						m_secondaryTargetStatusPage->setTarget(NetworkId::cms_invalid);
				}
				break;
			case CuiPreferences::STM_none:
			default:
				m_secondaryTargetStatusPage->setTarget(NetworkId::cms_invalid);
				break;
			}
		}
		else
		{
			m_targetStatusPage->setTarget(NetworkId::cms_invalid);
			m_secondaryTargetStatusPage->setTarget(NetworkId::cms_invalid);
		}
	}
}

//----------------------------------------------------------------------

void SwgCuiHudWindowManagerGround::onTargetChanged(const CreatureObject & payload)
{
	UNREF(payload);
}

void SwgCuiHudWindowManagerGround::onPetChanged(const PlayerObject & payload)
{
	if(m_petStatusPage)
	{
		PlayerObject const * const player = Game::getPlayerObject();
		if (player && (player == &payload)) 
		{
			NetworkId const & pet = player->getPetId();
			if(pet.isValid())
			{
				m_petStatusPage->setTarget(pet);
			}
			else
			{
				m_petStatusPage->clearTarget();
			}
			setPetToolbarVisible(pet.isValid());
		}
	}
}


//----------------------------------------------------------------------

void SwgCuiHudWindowManagerGround::onPlayerSetup(const CreatureObject & payload)
{
	CreatureObject const * const player = Game::getPlayerCreature();
	if (player && (player == &payload)) 
	{
		createPlayerStatusPage();
		onTargetChanged(*player);
	}
}

//----------------------------------------------------------------------

void SwgCuiHudWindowManagerGround::onJediStateChanged(const PlayerObject & payload)
{
	PlayerObject const * const player = Game::getPlayerObject();
	if (player && (player == &payload)) 
	{
		createPlayerStatusPage();
	}
}

//----------------------------------------------------------------------

void SwgCuiHudWindowManagerGround::createPlayerStatusPage()
{
	removePlayerStatusPage();

	CuiMediator const * const hud = SwgCuiHudFactory::findMediatorForCurrentHud();

	if (!Game::isHudSceneTypeSpace() && hud) 
	{
		PlayerObject const * const playerObject = Game::getPlayerObject();
		bool const isJedi = playerObject && playerObject->isJedi();
		
		UIPage * statusPage = NULL;
		hud->getCodeDataObject(TUIPage, statusPage, "MfdStatusPage");
		statusPage->SetVisible(false);
		
		UIPage * statusPageJedi = NULL;
		hud->getCodeDataObject(TUIPage, statusPageJedi, "MFDStatusJedi");
		statusPageJedi->SetVisible(false);
		
		UIPage * const mediatorPage = isJedi ? statusPageJedi : statusPage;
		m_playerStatusPage = new SwgCuiStatusGround(*mediatorPage, SwgCuiStatusGround::ST_player);
		if (m_playerStatusPage != NULL)
		{
			CreatureObject * const player = Game::getPlayerCreature();
			NOT_NULL(player);
			m_playerStatusPage->setTarget(player);
			mediatorPage->SetVisible(true);
			m_playerStatusPage->setSettingsAutoSizeLocation(true, true);			
			m_playerStatusPage->setStickyVisible (true);
			m_playerStatusPage->fetch();
			m_playerStatusPage->activate();
			m_playerStatusPage->setShowFocusedGlowRect(false);
			
			getWorkspace().addMediator(*m_playerStatusPage);
		}
	}

	setStatusMediator(m_playerStatusPage);
}

//----------------------------------------------------------------------

void SwgCuiHudWindowManagerGround::removePlayerStatusPage()
{
	if (m_playerStatusPage) 
	{
		getWorkspace().removeMediator(*m_playerStatusPage);
		m_playerStatusPage->release();
		m_playerStatusPage = 0;
	}
}

//----------------------------------------------------------------------

void SwgCuiHudWindowManagerGround::handlePerformActivate()
{
	SwgCuiHudWindowManager::handlePerformActivate();

	m_callback->connect(*this, &SwgCuiHudWindowManagerGround::onTargetChanged, static_cast<CreatureObject::Messages::IntendedTargetChanged*>(0));
	m_callback->connect(*this, &SwgCuiHudWindowManagerGround::onTargetChanged, static_cast<CreatureObject::Messages::LookAtTargetChanged*>(0));
	m_callback->connect(*this, &SwgCuiHudWindowManagerGround::onTargetChanged, static_cast<PlayerCreatureController::Messages::AutoAimToggled*>(0));

	m_callback->connect(*this, &SwgCuiHudWindowManagerGround::onPlayerSetup, static_cast<CreatureObject::Messages::PlayerSetup *>(0));
	m_callback->connect(*this, &SwgCuiHudWindowManagerGround::onJediStateChanged, static_cast<PlayerObject::Messages::JediStateChanged*>(0));
	m_callback->connect(*this, &SwgCuiHudWindowManagerGround::onPetChanged, static_cast<PlayerObject::Messages::PetChanged*>(0));

	if (Game::getPlayerCreature())
		onPlayerSetup(*Game::getPlayerCreature());
}

//----------------------------------------------------------------------

void SwgCuiHudWindowManagerGround::handlePerformDeactivate()
{
	SwgCuiHudWindowManager::handlePerformDeactivate();

	m_callback->disconnect(*this, &SwgCuiHudWindowManagerGround::onTargetChanged, static_cast<CreatureObject::Messages::IntendedTargetChanged*>(0));
	m_callback->disconnect(*this, &SwgCuiHudWindowManagerGround::onTargetChanged, static_cast<CreatureObject::Messages::LookAtTargetChanged*>(0));
	m_callback->disconnect(*this, &SwgCuiHudWindowManagerGround::onTargetChanged, static_cast<PlayerCreatureController::Messages::AutoAimToggled*>(0));

	m_callback->disconnect(*this, &SwgCuiHudWindowManagerGround::onPlayerSetup, static_cast<CreatureObject::Messages::PlayerSetup *>(0));
	m_callback->disconnect(*this, &SwgCuiHudWindowManagerGround::onJediStateChanged, static_cast<PlayerObject::Messages::JediStateChanged*>(0));
	m_callback->disconnect(*this, &SwgCuiHudWindowManagerGround::onPetChanged, static_cast<PlayerObject::Messages::PetChanged*>(0));
}

//======================================================================
