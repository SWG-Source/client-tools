//======================================================================
//
// SwgCuiBuffBuilderBuffee.cpp
// copyright (c) 2006 Sony Online Entertainment
//
//======================================================================

#include "swgClientUserInterface/FirstSwgClientUserInterface.h"
#include "swgClientUserInterface/SwgCuiBuffBuilderBuffee.h"

#include "clientGame/CreatureObject.h"
#include "clientGame/Game.h"
#include "clientGame/PlayerMoneyManagerClient.h"
#include "clientGame/ProsePackageManagerClient.h"
#include "clientUserInterface/CuiManager.h"
#include "clientUserInterface/CuiSystemMessageManager.h"
#include "sharedGame/SharedStringIds.h"
#include "sharedFoundation/GameControllerMessage.h"
#include "sharedFoundation/Os.h"
#include "sharedGame/ProsePackage.h"
#include "sharedGame/SharedBuffBuilderManager.h"
#include "sharedMessageDispatch/Transceiver.h"
#include "sharedObject/NetworkIdManager.h"


#include "UIButton.h"
#include "UIList.h"
#include "UIText.h"

SwgCuiBuffBuilderBuffee::SwgCuiBuffBuilderBuffee (UIPage & page) :
CuiMediator       ("SwgCuiBuffBuilderBuffee", page),
UIEventCallback   (),
m_callback        (new MessageDispatch::Callback),
m_bufferId(),
m_cancelButton(NULL),
m_acceptButton(NULL),
m_effectList(NULL),
m_coverCharge(NULL),
m_entertainerName(NULL),
m_committed(false)
{
	IGNORE_RETURN(setState(MS_closeable));

	getCodeDataObject(TUIButton, m_cancelButton, "buttonCancel");
	getCodeDataObject(TUIButton, m_acceptButton, "buttonAccept");
	getCodeDataObject(TUIList, m_effectList, "listView");
	getCodeDataObject(TUIText, m_coverCharge, "coverCharge");
	getCodeDataObject(TUIText, m_entertainerName, "entertainerName");
	
	m_callback->connect (*this, &SwgCuiBuffBuilderBuffee::onBuffBuilderChangeReceived, static_cast<PlayerCreatureController::Messages::BuffBuilderChangeReceived *>(0));
	m_callback->connect (*this, &SwgCuiBuffBuilderBuffee::onBuffBuilderCancelReceived, static_cast<PlayerCreatureController::Messages::BuffBuilderCancelReceived *>(0));

	registerMediatorObject (*m_cancelButton, true);
	registerMediatorObject (*m_acceptButton, true);
	registerMediatorObject (*m_effectList, true);


	m_coverCharge->SetText(Unicode::narrowToWide("0"));
	m_coverCharge->SetTextColor(UIColor::green);
	m_effectList->Clear();
	m_acceptButton->SetEnabled(false);
}

//----------------------------------------------------------------------

SwgCuiBuffBuilderBuffee::~SwgCuiBuffBuilderBuffee ()
{
	m_callback->disconnect (*this, &SwgCuiBuffBuilderBuffee::onBuffBuilderCancelReceived, static_cast<PlayerCreatureController::Messages::BuffBuilderCancelReceived *>(0));
	m_callback->disconnect (*this, &SwgCuiBuffBuilderBuffee::onBuffBuilderChangeReceived, static_cast<PlayerCreatureController::Messages::BuffBuilderChangeReceived *>(0));

	delete m_callback;
	m_callback      = 0;

	m_cancelButton = NULL;
	m_acceptButton = NULL;
	m_effectList = 0;
}

//----------------------------------------------------------------------

void SwgCuiBuffBuilderBuffee::performActivate   ()
{
	CuiManager::requestPointer (true);
}

//----------------------------------------------------------------------

void SwgCuiBuffBuilderBuffee::performDeactivate ()
{
	CuiManager::requestPointer (false);
}

//----------------------------------------------------------------------

void SwgCuiBuffBuilderBuffee::OnButtonPressed( UIWidget *context )
{
	//tell the server to cancel the session if necessary
	if(context == m_cancelButton)
	{
		closeThroughWorkspace ();
	}
	//send the update packet
	else if(context == m_acceptButton)
	{
		buildAndSendUpdateToServer(true);
		m_committed = true;
		closeThroughWorkspace ();
	}
} 

//----------------------------------------------------------------------

void SwgCuiBuffBuilderBuffee::OnGenericSelectionChanged (UIWidget * context)
{
	if(context == m_effectList)
	{
		long selectedRow = m_effectList->GetLastSelectedRow();
		if(selectedRow != -1)
		{
			m_effectList->SelectRow(-1);
		}
	}
} 

//----------------------------------------------------------------------

void SwgCuiBuffBuilderBuffee::setBufferId(NetworkId const & bufferId)
{
	CreatureObject * const player = Game::getPlayerCreature();
	if(!player)
		return;

	m_bufferId = bufferId;

	//add the buffer's name to the window caption
	CreatureObject * const buffer = getBufferCreature();
	if(buffer)
	{
		Unicode::String entertainerName = buffer->getLocalizedFirstName();
		m_entertainerName->SetPreLocalized(true);
		m_entertainerName->SetLocalText(entertainerName);
		
	}

	//start a session.  This data will get overridden by designer-sent data once they start making changes
	SharedBuffBuilderManager::Session newSession;
	newSession.bufferId = getBufferId();
	newSession.recipientId = player->getNetworkId();
	newSession.startingTime = Os::getRealSystemTime();
	SharedBuffBuilderManager::startSession(newSession);
	setAssociatedObjectId(bufferId);
	setMaxRangeFromObject(16.0f);
}

//----------------------------------------------------------------------
NetworkId const & SwgCuiBuffBuilderBuffee::getBufferId() const
{
	return m_bufferId;
}

//----------------------------------------------------------------------

bool SwgCuiBuffBuilderBuffee::close()
{
	CreatureObject * const player = Game::getPlayerCreature();

	if(!m_committed && player)
	{
		SharedBuffBuilderManager::Session session;
		bool const result = SharedBuffBuilderManager::getSession(getBufferId(), session);
		if(result)
		{
			SharedBuffBuilderManager::endSession(session.bufferId);
			Object * const recipientObj = NetworkIdManager::getObjectById(session.recipientId);
			if(recipientObj && recipientObj == player)
			{
				BuffBuilderChangeMessage * const msg = new BuffBuilderChangeMessage;
				SharedBuffBuilderManager::populateChangeMessage(session, *msg);
				msg->setOrigin(BuffBuilderChangeMessage::O_RECIPIENT);
				msg->setAccepted(false);
				recipientObj->getController()->appendMessage (CM_buffBuilderCancel, 0.0f, msg, GameControllerMessageFlags::SEND | GameControllerMessageFlags::RELIABLE | GameControllerMessageFlags::DEST_AUTH_SERVER);
			}
		}
	}
	return CuiMediator::close();
}

//----------------------------------------------------------------------

void SwgCuiBuffBuilderBuffee::update (float const deltaTimeSecs)
{
	CreatureObject * const player = Game::getPlayerCreature();
	if(!player)
		return;

	SharedBuffBuilderManager::Session session;
	bool const result = SharedBuffBuilderManager::getSession(getBufferId(), session);
	if(result)
	{
		if(player->getNetworkId() != session.recipientId)
			return;

		if(hasEnoughMoney())
		{
			m_coverCharge->SetTextColor(UIColor::green);
			m_acceptButton->SetEnabled(session.accepted);
		}
		else
		{
			m_coverCharge->SetTextColor(UIColor::red);
			m_acceptButton->SetEnabled(false);
		}
	}
	CuiMediator::update(deltaTimeSecs);
}


/** When receiving a change packet, update the UI
*/
void SwgCuiBuffBuilderBuffee::onBuffBuilderChangeReceived(PlayerCreatureController::Messages::BuffBuilderChangeReceived::Payload const & payload)
{
	CreatureObject * const player = Game::getPlayerCreature();
	if(!player)
		return;

	if(player->getNetworkId() != payload.recipientId)
		return;

	SharedBuffBuilderManager::Session session = payload;

	//get our current session, to ensure that we don't use the server-sent starting time
	SharedBuffBuilderManager::Session clientSession;
	bool const result = SharedBuffBuilderManager::getSession(session.bufferId, clientSession);
	if(result)
		session.startingTime = clientSession.startingTime;

	SharedBuffBuilderManager::updateSession(session);
	
	// effect list
	m_effectList->Clear();
	char tmp[512];
	for(std::map<std::string, std::pair<int,int> >::iterator buffIter = session.buffComponents.begin(); buffIter != session.buffComponents.end(); ++buffIter)
	{
		snprintf(tmp, 512, "%s_prose", buffIter->first.c_str());
		std::string tmpS(tmp);
		ProsePackage pp;
		pp.stringId = StringId("buff_builder",tmpS);
		
		const int affectAmount = SharedBuffBuilderManager::getAffectAmountForRecordName(buffIter->first);
		const int buffCount = buffIter->second.first;
		const int expertiseModifier = buffIter->second.second;
		const int adjustedAmount = SharedBuffBuilderManager::computeAdjustedAffectAmount(buffIter->first,affectAmount,expertiseModifier);
		pp.digitInteger = buffCount * adjustedAmount;
		
		Unicode::String resultStr;
		IGNORE_RETURN(ProsePackageManagerClient::appendTranslation(pp, resultStr));
		m_effectList->AddRow(resultStr,buffIter->first);
	}

	// cover charge
	int const creditsRequiredByBuffer = session.bufferRequiredCredits;
	char buf[256];
	_itoa(creditsRequiredByBuffer, buf, 10);
	m_coverCharge->SetText(Unicode::narrowToWide(buf));

	// accept button	
	m_acceptButton->SetEnabled(session.accepted);
}

//----------------------------------------------------------------------

/** When receiving a cancel packet, close the UI
*/
void SwgCuiBuffBuilderBuffee::onBuffBuilderCancelReceived(PlayerCreatureController::Messages::BuffBuilderChangeReceived::Payload const & payload)
{
	CreatureObject * const player = Game::getPlayerCreature();
	if(!player)
		return;

	if(player->getNetworkId() != payload.recipientId)
		return;

	SharedBuffBuilderManager::endSession(payload.bufferId);
	
	//no need to tell server
	m_committed = true;
	CuiSystemMessageManager::sendFakeSystemMessage (SharedStringIds::buffbuilder_canceled_by_buffer.localize());
	closeThroughWorkspace();
}

//----------------------------------------------------------------------

CreatureObject * SwgCuiBuffBuilderBuffee::getBufferCreature() const
{
	Object * const o = NetworkIdManager::getObjectById(getBufferId());
	ClientObject * const co = o ? o->asClientObject() : NULL;
	return co ? co->asCreatureObject() : NULL;
}

//----------------------------------------------------------------------

void SwgCuiBuffBuilderBuffee::buildAndSendUpdateToServer(bool const accepted) const
{
	CreatureObject * const buffer = getBufferCreature();
	if(buffer)
	{
		SharedBuffBuilderManager::Session session;
		bool const result = SharedBuffBuilderManager::getSession(buffer->getNetworkId(), session);
		if(result)
		{
			BuffBuilderChangeMessage * const msg = new BuffBuilderChangeMessage;
			SharedBuffBuilderManager::populateChangeMessage(session, *msg);
			msg->setOrigin(BuffBuilderChangeMessage::O_RECIPIENT);
			msg->setAccepted(accepted);
			Game::getPlayer()->getController()->appendMessage (CM_buffBuilderChange, 0.0f, msg, GameControllerMessageFlags::SEND | GameControllerMessageFlags::RELIABLE | GameControllerMessageFlags::DEST_AUTH_SERVER);

			//if accepted, also send message to buffer, so they forward on the buffer's session to the recipient
			if(accepted)
			{
				BuffBuilderChangeMessage * const msg2 = new BuffBuilderChangeMessage;
				SharedBuffBuilderManager::populateChangeMessage(session, *msg2);
				msg->setOrigin(BuffBuilderChangeMessage::O_RECIPIENT);
				msg->setAccepted(true);
				Controller * const bufferController = buffer->getController();
				if(bufferController)
					bufferController->appendMessage (CM_buffBuilderChange, 0.0f, msg2, GameControllerMessageFlags::SEND | GameControllerMessageFlags::RELIABLE | GameControllerMessageFlags::DEST_AUTH_SERVER);
			}
		}
	}
}

//----------------------------------------------------------------------

bool SwgCuiBuffBuilderBuffee::hasEnoughMoney() const
{
	UIString coverCharge;
	m_coverCharge->GetText(coverCharge);
	const int amount = atoi(Unicode::wideToNarrow(coverCharge).c_str());
	
	int cash = 0;
	int bank = 0;

	IGNORE_RETURN(PlayerMoneyManagerClient::getPlayerMoney (cash, bank));

	if((bank + cash) < amount)
	{
		return false;
	}

	return true;
}

//==================================================================
