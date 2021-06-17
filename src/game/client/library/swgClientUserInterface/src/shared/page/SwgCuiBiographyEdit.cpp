// ======================================================================
//
// SwgCuiBiographyEdit.cpp
// Copyright(c) 2004 Sony Online Entertainment
//
// ======================================================================

#include "swgClientUserInterface/FirstSwgClientUserInterface.h"
#include "swgClientUserInterface/SwgCuiBiographyEdit.h"

#include "clientGame/ClientCommandQueue.h"
#include "clientGame/ConfigClientGame.h"
#include "clientGame/ContainerInterface.h"
#include "clientGame/CreatureObject.h"
#include "clientGame/Game.h"
#include "clientGame/PlayerCreatureController.h"
#include "clientGame/PlayerObject.h"
#include "clientUserInterface/CuiManager.h"
#include "sharedFoundation/Crc.h"
#include "sharedMessageDispatch/Transceiver.h"

#include "UIButton.h"
#include "UIData.h"
#include "UIImage.h"
#include "UIPage.h"
#include "UITabbedPane.h"
#include "UIText.h"

//-----------------------------------------------------------------------

namespace SwgCuiBiographyEditNamespace
{
	float const s_displayTimeout = 5.0f;

	uint32 const hashRequestBiography = Crc::normalizeAndCalculate("requestBiography");
	uint32 const hashSetBiography = Crc::normalizeAndCalculate("setBiographyAdmin");

	PlayerObject * getPlayerObject(CachedNetworkId const & networkId);
}

using namespace SwgCuiBiographyEditNamespace;

//-----------------------------------------------------------------------

PlayerObject * SwgCuiBiographyEditNamespace::getPlayerObject(CachedNetworkId const & playerId)
{
	Object  * const object = playerId.getObject();
	ClientObject * const clientObject = object ? object->asClientObject() : NULL;
	CreatureObject  * const creatureObject = clientObject ? clientObject->asCreatureObject() : NULL;
	PlayerObject * const playerObject = creatureObject ? creatureObject->getPlayerObject() : NULL;

	return playerObject;
}

//-----------------------------------------------------------------------

SwgCuiBiographyEdit::SwgCuiBiographyEdit(UIPage & page): 
CuiMediator("SwgCuiBiographyEdit", page),
UIEventCallback(),
m_player(),
m_biographyText(NULL),
m_characterName(NULL),
m_pageStatus(NULL),
m_saveButton(NULL),
m_cancelButton(NULL),
m_editColor(),
m_biographyStatus(BES_none),
m_biographyTimeout(s_displayTimeout),
m_callBack(new MessageDispatch::Callback)
{
	getCodeDataObject(TUIText, m_biographyText, "textBiography");
	getCodeDataObject(TUIText, m_characterName, "textCharacterName");
	getCodeDataObject(TUIText, m_pageStatus, "textStatus");
	
	m_editColor = m_biographyText->GetColor();

	getCodeDataObject(TUIButton, m_saveButton, "buttonSave");
	getCodeDataObject(TUIButton, m_cancelButton, "buttonCancel");

	m_characterName->Clear();
	m_characterName->SetPreLocalized(true);
	
	m_biographyText->Clear();
	m_biographyText->SetPreLocalized(true);

	m_pageStatus->Clear();
	m_pageStatus->SetPreLocalized(true);

	setState(MS_closeable);
	setState(MS_closeDeactivates);
	removeState(MS_iconifiable);

	registerMediatorObject(*m_saveButton, true);
	registerMediatorObject(*m_cancelButton, true);

	m_callBack->connect(*this, &SwgCuiBiographyEdit::onBiographyRetrieved, static_cast<PlayerCreatureController::Messages::BiographyRetrieved *>(0));
}

//-----------------------------------------------------------------------

SwgCuiBiographyEdit::~SwgCuiBiographyEdit()
{
	m_callBack->disconnect(*this, &SwgCuiBiographyEdit::onBiographyRetrieved, static_cast<PlayerCreatureController::Messages::BiographyRetrieved *>(0));

	m_biographyText = NULL;
	m_characterName = NULL;
	m_pageStatus = NULL;
	m_saveButton = NULL;

	delete m_callBack;
	m_callBack = NULL;
}

//-----------------------------------------------------------------------

void SwgCuiBiographyEdit::performActivate()
{
	if (Game::getPlayerObject()->isAdmin())
	{
		CuiManager::requestPointer(true);
		setIsUpdating(true);
		m_biographyStatus = BES_waiting;
	}
}

//-----------------------------------------------------------------------

void SwgCuiBiographyEdit::performDeactivate()
{
	if (Game::getPlayerObject()->isAdmin())
	{
		setIsUpdating(false);
		CuiManager::requestPointer(false);
		m_biographyStatus = BES_none;
	}
}

//-----------------------------------------------------------------------

void SwgCuiBiographyEdit::onBiographyRetrieved(PlayerCreatureController::Messages::BiographyRetrieved::BiographyOwner const & msg)
{
	if (Game::getPlayerObject()->isAdmin())
	{
		if (m_biographyStatus == BES_waiting)
		{
			m_biographyStatus = BES_failed;
			m_biographyText->Clear();
		
			PlayerObject const * const playerObject = dynamic_cast<PlayerObject const *>(msg.second);
			if (msg.first != NetworkId::cms_invalid && playerObject && playerObject->haveBiography() && (getPlayerObject(m_player) == playerObject)) 
			{
				m_biographyStatus = BES_success;
				m_biographyText->SetLocalText(playerObject->getBiography());
			}
		}
	}
}

//-----------------------------------------------------------------------

void SwgCuiBiographyEdit::OnButtonPressed(UIWidget *context)
{
	if (Game::getPlayerObject()->isAdmin())
	{
		if (context == m_saveButton)
		{
			if (m_biographyStatus == BES_success) 
			{
				PlayerObject * const playerObject = getPlayerObject(m_player);

				if (playerObject && (m_biographyText->GetLocalText() != playerObject->getBiography())) 
				{
					CreatureObject const * const creatureObject = safe_cast<CreatureObject const *>(ContainerInterface::getContainedByObject(*playerObject));
					if (creatureObject)
					{
						m_biographyStatus = BES_none;
						m_biographyTimeout.reset();
						
						playerObject->setHaveBiography(false);

						IGNORE_RETURN(ClientCommandQueue::enqueueCommand(hashSetBiography, creatureObject->getNetworkId(), m_biographyText->GetLocalText()));

						closeThroughWorkspace();
					}
				}
			}
		}
		else if (context == m_cancelButton)
		{
			closeThroughWorkspace();
		}
	}
}

//----------------------------------------------------------------------

void SwgCuiBiographyEdit::update(float deltaTimeSecs)
{
	CuiMediator::update(deltaTimeSecs);

	if (Game::getPlayerObject()->isAdmin())
	{
		// Out of range check...
		bool const isInRange = m_player != CachedNetworkId::cms_cachedInvalid && m_player.isValid();

		// Set editable based on success.
		bool const isEditable = isInRange && (m_biographyStatus == BES_success);
		m_biographyText->SetEditable(isEditable);
		m_biographyText->SetColor(isEditable ? m_editColor : UIColor::black);

		// Check for timeouts.
		if (m_biographyStatus == BES_waiting)
		{
			m_biographyText->SetLocalText(Unicode::narrowToWide("Fetching biography..."));

			if (m_biographyTimeout.updateNoReset(deltaTimeSecs)) 
			{
				m_biographyStatus = BES_timedout;
			}
		}

		if (isInRange) 
		{
			char const * statusText = "Undefined";
			
			switch(m_biographyStatus)
			{
			case BES_none:
				statusText = "None";
				break;
				
			case BES_failed:
				statusText = "Failed!";
				break;
				
			case BES_waiting:
				statusText = "Waiting...";
				break;
				
			case BES_success:
				statusText = "Success!";
				break;
				
			case BES_timedout:
				statusText = "Timed Out?";
				break;
			}
			
			m_pageStatus->SetLocalText(Unicode::narrowToWide(statusText));
		}
		else
		{
			m_pageStatus->SetLocalText(Unicode::narrowToWide("Target out of range."));
		}
	}
}

//----------------------------------------------------------------------

bool SwgCuiBiographyEdit::requestBiography(CachedNetworkId const & playerId)
{
	m_biographyStatus = BES_failed;
	m_player = playerId;
	m_biographyTimeout.reset();

	bool success = false;
	
	PlayerObject * const playerObject = getPlayerObject(playerId);

	if (playerObject) 
	{
		CreatureObject const * const creatureObject = safe_cast<CreatureObject const *>(ContainerInterface::getContainedByObject(*playerObject));

		if (creatureObject) 
		{
			m_characterName->SetLocalText(creatureObject->getLocalizedName());

			playerObject->setHaveBiography(false);
			
			IGNORE_RETURN(ClientCommandQueue::enqueueCommand(hashRequestBiography, creatureObject->getNetworkId(), Unicode::emptyString));
			m_biographyStatus = BES_waiting;
			success = true;
		}
	}

	return success;
}


// ======================================================================
