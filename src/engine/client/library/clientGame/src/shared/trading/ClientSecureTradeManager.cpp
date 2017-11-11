// ======================================================================
//
// ClientSecureTradeManager.cpp
// Copyright 2001, 2002 Sony Online Entertainment Inc.
// All Rights Reserved.
//
// ======================================================================

#include "clientGame/FirstClientGame.h"
#include "clientGame/ClientSecureTradeManager.h"

#include "clientGame/CreatureController.h"
#include "clientGame/CreatureObject.h"
#include "clientGame/Game.h"
#include "clientGame/GameNetwork.h"
#include "clientUserInterface/CuiStringIdsTrade.h"
#include "clientUserInterface/CuiStringVariablesManager.h"
#include "clientUserInterface/CuiSystemMessageManager.h"
#include "sharedDebug/InstallTimer.h"
#include "sharedFoundation/ExitChain.h"
#include "sharedFoundation/GameControllerMessage.h"
#include "sharedMessageDispatch/Transceiver.h"
#include "sharedNetworkMessages/MessageQueueSecureTrade.h"
#include "sharedNetworkMessages/SecureTradeMessages.h"
#include "sharedObject/NetworkIdManager.h"

#include <algorithm>

//----------------------------------------------------------------------

namespace
{
	namespace Transceivers
	{
		MessageDispatch::Transceiver<const ClientSecureTradeManager::Messages::AbortTrade::Payload &,               ClientSecureTradeManager::Messages::AbortTrade >
			abort;
		MessageDispatch::Transceiver<const ClientSecureTradeManager::Messages::BeginTrade::Payload &,               ClientSecureTradeManager::Messages::BeginTrade >
			beginTrade;
		MessageDispatch::Transceiver<const ClientSecureTradeManager::Messages::AddItem::Payload &,                  ClientSecureTradeManager::Messages::AddItem >
			addItem;
		MessageDispatch::Transceiver<const ClientSecureTradeManager::Messages::RemoveItem::Payload &,               ClientSecureTradeManager::Messages::RemoveItem >
			removeItem;
		MessageDispatch::Transceiver<const ClientSecureTradeManager::Messages::GiveMoney::Payload &,                ClientSecureTradeManager::Messages::GiveMoney >
			giveMoney;
		MessageDispatch::Transceiver<const ClientSecureTradeManager::Messages::Accept::Payload &,                   ClientSecureTradeManager::Messages::Accept >
			accept;
		MessageDispatch::Transceiver<const ClientSecureTradeManager::Messages::RequestVerify::Payload &,            ClientSecureTradeManager::Messages::RequestVerify >
			requestVerify;
		MessageDispatch::Transceiver<const ClientSecureTradeManager::Messages::Complete::Payload &,                 ClientSecureTradeManager::Messages::Complete >
			complete;
		MessageDispatch::Transceiver<const ClientSecureTradeManager::Messages::Requested::Payload &,                ClientSecureTradeManager::Messages::Requested >
			requested;
		MessageDispatch::Transceiver<const ClientSecureTradeManager::Messages::RequestDenied::Payload &,            ClientSecureTradeManager::Messages::RequestDenied >
			requestDenied;
		MessageDispatch::Transceiver<const ClientSecureTradeManager::Messages::RequestDeniedBusy::Payload &,        ClientSecureTradeManager::Messages::RequestDeniedBusy >
			requestDeniedBusy;
		MessageDispatch::Transceiver<const ClientSecureTradeManager::Messages::RequestDeniedUnreachable::Payload &, ClientSecureTradeManager::Messages::RequestDeniedUnreachable >
			requestDeniedUnreachable;
	}

	CachedNetworkId       s_otherId;
	CachedNetworkId       s_initiateItemId;

	typedef ClientSecureTradeManager::CachedNetworkIdVector CachedNetworkIdVector;

	CachedNetworkId       s_tradeRequestId;

	CachedNetworkIdVector s_itemsSelf;
	CachedNetworkIdVector s_itemsOther;

	int                   s_moneySelf      = 0;
	int                   s_moneyOther     = 0;

	bool                  s_okOther    = false;
	bool                  s_okSelf     = false;
	CachedNetworkIdVector s_waitingForObjects;
	bool                  s_tradeInProgress = false;

	void reset ()
	{
		s_initiateItemId = NetworkId::cms_invalid;
		s_moneySelf      = 0;
		s_moneyOther     = 0;
		s_okOther        = false;
		s_okSelf         = false;
		s_itemsSelf.clear  ();
		s_itemsOther.clear ();
		s_tradeRequestId = NetworkId::cms_invalid;
		s_waitingForObjects.clear();
	}

	bool                 s_playerIsInitiator = false;

}

// -------------------------------------------------------------

ClientSecureTradeManager::TestReceiver *ClientSecureTradeManager::m_receiver;

// -------------------------------------------------------------

void ClientSecureTradeManager::install()
{
	InstallTimer const installTimer("ClientSecureTradeManager::install");

	m_receiver = new TestReceiver;
	m_receiver->connectToMessage ("BeginTradeMessage");
	m_receiver->connectToMessage ("AbortTradeMessage");
	m_receiver->connectToMessage ("AddItemMessage");
	m_receiver->connectToMessage ("RemoveItemMessage");
	m_receiver->connectToMessage ("GiveMoneyMessage");
	m_receiver->connectToMessage ("TradeCompleteMessage");
	m_receiver->connectToMessage ("AcceptTransactionMessage");
	m_receiver->connectToMessage ("UnAcceptTransactionMessage");
	m_receiver->connectToMessage ("VerifyTradeMessage");
	m_receiver->connectToMessage ("DenyTradeMessage");
	m_receiver->connectToMessage ("BeginVerificationMessage");
	m_receiver->connectToMessage (AddItemFailedMessage::MessageType);

	reset ();

	ExitChain::add (remove, "ClientSecureTradeManager::remove");
}

// -------------------------------------------------------------

void ClientSecureTradeManager::remove()
{
	reset ();

	m_receiver->disconnectAll();
	delete m_receiver;
}


// -------------------------------------------------------------

bool ClientSecureTradeManager::initiateTrade(const NetworkId & player, const NetworkId & item, bool )
{
	if (s_tradeRequestId != CachedNetworkId::cms_invalid )
	{
		acceptTradeRequest ();
	}
	else
	{
		if (s_tradeInProgress)
		{
			if (player == s_otherId)
			{
				addItem (item);
				return true;
			}
			else
			{
				const ClientObject * const targetObject = safe_cast<ClientObject *>(NetworkIdManager::getObjectById (player));
				const ClientObject * const otherObject  = safe_cast<ClientObject *>(NetworkIdManager::getObjectById (s_otherId));
				if (targetObject && otherObject)
				{
					Unicode::String result;
					CuiStringVariablesManager::process (CuiStringIdsTrade::start_fail_target_other_prose, Unicode::emptyString, targetObject->getLocalizedName (), otherObject->getLocalizedName (), result);
					CuiSystemMessageManager::sendFakeSystemMessage (result);
					return false;
				}
				else
				{
					//-- try to get out of whatever bogus trade we are stuck in
					abortTrade ();
					reset      ();
				}
			}
		}

		CreatureObject const * const playerCreatureObject = Game::getPlayerCreature();
		ClientObject const * const targetObject = safe_cast<ClientObject *>(NetworkIdManager::getObjectById (player));
		CreatureObject const * const targetCreatureObject = targetObject ? targetObject->asCreatureObject() : NULL;

		if (!targetObject || !targetCreatureObject)
		{
			StringId error;
			if (targetObject && targetObject->asShipObject())
				error=CuiStringIdsTrade::start_fail_target_ship; // player may be trying to trade with the ship's pilot, so give a special error message
			else
				error=CuiStringIdsTrade::start_fail_target_not_player;

			CuiSystemMessageManager::sendFakeSystemMessage(error.localize());
			return false;
		}
		else if (targetCreatureObject && targetCreatureObject->isDead())
		{
			CuiSystemMessageManager::sendFakeSystemMessage(CuiStringIdsTrade::target_dead.localize());
			return false;
		}
		else if (targetCreatureObject && targetCreatureObject->isIncapacitated())
		{
			CuiSystemMessageManager::sendFakeSystemMessage(CuiStringIdsTrade::target_incapacitated.localize());
			return false;
		}
		else if (playerCreatureObject && playerCreatureObject->isDead())
		{
			CuiSystemMessageManager::sendFakeSystemMessage(CuiStringIdsTrade::player_dead.localize());
			return false;
		}
		else if (playerCreatureObject && playerCreatureObject->isIncapacitated())
		{
			CuiSystemMessageManager::sendFakeSystemMessage(CuiStringIdsTrade::player_incapacitated.localize());
			return false;
		}
		else
		{
			CreatureController* const playerController = safe_cast<CreatureController*>(Game::getPlayer()->getController());
			MessageQueueSecureTrade * m = new MessageQueueSecureTrade(MessageQueueSecureTrade::TMI_RequestTrade, NetworkId::cms_invalid, player);
			playerController->appendMessage(static_cast<int>(CM_secureTrade), 0.0f, m,
				GameControllerMessageFlags::SEND | GameControllerMessageFlags::RELIABLE | GameControllerMessageFlags::DEST_AUTH_SERVER);
			s_playerIsInitiator = true;
		}
	}

	reset ();
	s_otherId        = player;
	s_initiateItemId = item;
	return true;
}

// -------------------------------------------------------------

void ClientSecureTradeManager::denyTradeRequest()
{
	CreatureController* const playerController = safe_cast<CreatureController*>(Game::getPlayer()->getController());
	MessageQueueSecureTrade * m = new MessageQueueSecureTrade(MessageQueueSecureTrade::TMI_DeniedTrade, NetworkId::cms_invalid, NetworkId::cms_invalid);
	playerController->appendMessage(static_cast<int>(CM_secureTrade), 0.0f, m,
					GameControllerMessageFlags::SEND | GameControllerMessageFlags::RELIABLE | GameControllerMessageFlags::DEST_AUTH_SERVER);

}

// -------------------------------------------------------------

void ClientSecureTradeManager::acceptTradeRequest()
{
	s_playerIsInitiator = false;
	CreatureController* const playerController = safe_cast<CreatureController*>(Game::getPlayer()->getController());
	MessageQueueSecureTrade * m = new MessageQueueSecureTrade(MessageQueueSecureTrade::TMI_AcceptTrade, NetworkId::cms_invalid, NetworkId::cms_invalid);
	playerController->appendMessage(static_cast<int>(CM_secureTrade), 0.0f, m,
					GameControllerMessageFlags::SEND | GameControllerMessageFlags::RELIABLE | GameControllerMessageFlags::DEST_AUTH_SERVER);

	reset ();
}

// -------------------------------------------------------------

void ClientSecureTradeManager::addItem(const NetworkId & item)
{
	const CachedNetworkId cid (item);

	//-- don't add the item twice
	if (std::find (s_itemsSelf.begin (), s_itemsSelf.end (), cid) != s_itemsSelf.end ())
		return;

	const AddItemMessage m(item);
	GameNetwork::send(m, true);

	invalidateAcceptance ();
	s_itemsSelf.push_back (cid);

	Transceivers::addItem.emitMessage (Messages::AddItem::Payload (true, item));
}

// -------------------------------------------------------------

void ClientSecureTradeManager::removeItem(const NetworkId & item)
{
	//-- CS wants us to abort trades when this happens
	UNREF (item);
	abortTrade ();

#if 0
	const RemoveItemMessage m(item);
	GameNetwork::send(m, true);

	invalidateAcceptance ();
	s_itemsSelf.erase (std::remove (s_itemsSelf.begin (), s_itemsSelf.end (), CachedNetworkId (item)), s_itemsSelf.end ());
	Transceivers::removeItem.emitMessage (Messages::RemoveItem::Payload (true, item));
#endif
}

// -------------------------------------------------------------

void ClientSecureTradeManager::giveMoney(int amount)
{
	//- positive money total indicates money is going to the recipient
	s_moneySelf = amount;

	const GiveMoneyMessage m(s_moneySelf);
	GameNetwork::send(m, true);

	invalidateAcceptance ();
	Transceivers::giveMoney.emitMessage (amount);
}

// -------------------------------------------------------------

void ClientSecureTradeManager::acceptTrade()
{
	s_okSelf = true;
	const AcceptTransactionMessage m;
	GameNetwork::send(m, true);
	Transceivers::accept.emitMessage (s_okOther);
}

// -------------------------------------------------------------

void ClientSecureTradeManager::unacceptTrade()
{
	if (s_okSelf && s_okOther)
	{
		denyTrade ();
	}

	s_okSelf = false;
	const UnAcceptTransactionMessage m;
	GameNetwork::send(m, true);
	Transceivers::accept.emitMessage (s_okOther);
}

// -------------------------------------------------------------

void ClientSecureTradeManager::verifyTrade()
{
	const VerifyTradeMessage m;
	GameNetwork::send(m, true);
}

// -------------------------------------------------------------

void ClientSecureTradeManager::denyTrade()
{
	s_okSelf  = false;
	s_okOther = false;

	const DenyTradeMessage m;
	GameNetwork::send(m, true);

	Transceivers::accept.emitMessage (s_okOther);
}

// -------------------------------------------------------------

void ClientSecureTradeManager::abortTrade()
{
	const AbortTradeMessage m;
	GameNetwork::send(m, true);
	Transceivers::abort.emitMessage (true);
}

// -------------------------------------------------------------

void ClientSecureTradeManager::TestReceiver::receiveMessage(const MessageDispatch::Emitter &, const MessageDispatch::MessageBase & message)
{
//	DEBUG_REPORT_LOG(true, ("Secure Trade, received message %d\n", message.getType()));
	if(message.isType("BeginTradeMessage"))
    {
		//Upon receipt of this message, the client should uncheck all ok boxes if a trade is in progress
		//or launch a new trade window if there is none in progress
		Archive::ReadIterator ri = NON_NULL (safe_cast<const GameNetworkMessage *>(&message))->getByteStream().begin();
		const BeginTradeMessage msg (ri);

		onBeginTrade (msg.getPlayerId ());
	}
	else if (message.isType("AbortTradeMessage"))
	{
		//Kill any trade in progress
		onAbortTrade ();
	}
	else if (message.isType("AddItemMessage"))
	{
		//Add object to the other player's window
		Archive::ReadIterator ri = NON_NULL (safe_cast<const GameNetworkMessage *>(&message))->getByteStream().begin();
		const AddItemMessage msg (ri);

		onAddItem (msg.getNetworkId ());

	}
	else if (message.isType("RemoveItemMessage"))
	{
		//Remove object from other player's window
		Archive::ReadIterator ri = NON_NULL (safe_cast<const GameNetworkMessage *>(&message))->getByteStream().begin();
		const RemoveItemMessage msg (ri);

		onRemoveItem (msg.getNetworkId ());
	}
	else if (message.isType("GiveMoneyMessage"))
	{
		//Change money control to reflect other player giving you money
		Archive::ReadIterator ri = NON_NULL (safe_cast<const GameNetworkMessage *>(&message))->getByteStream().begin();
		const GiveMoneyMessage msg (ri);

		onGiveMoney (msg.getAmount ());
	}
	else if (message.isType("TradeCompleteMessage"))
	{
		//Trade is done, close the window
		onTradeComplete ();
	}
	else if (message.isType("AcceptTransactionMessage"))
	{
		//Check the other person's accepted box
		onAcceptTrade (true);
	}
	else if (message.isType("UnAcceptTransactionMessage"))
	{
		//Uncheck the other person's accepted box
		onAcceptTrade (false);
	}
	else if (message.isType("VerifyTradeMessage"))
	{
		//Other person has ok'd the trade.  Do nothing?
		onVerifyTrade ();
	}
	else if (message.isType("DenyTradeMessage"))
	{
		//Other person has denied the trade.  Do nothing? (You will get begin trade message)
		onDenyTrade ();
	}
	else if (message.isType("BeginVerificationMessage"))
	{
		//Let's you know both parties have checked the accept boxes.
		onRequestVerification ();
	}
	else if (message.isType (AddItemFailedMessage::MessageType))
	{
		Archive::ReadIterator ri = NON_NULL (safe_cast<const GameNetworkMessage *>(&message))->getByteStream().begin();
		const AddItemFailedMessage msg (ri);
		onAddItemFailed (msg.getNetworkId ());
	}
	else
	{
	}
}

//----------------------------------------------------------------------

void ClientSecureTradeManager::onTradeRequested                      (const NetworkId & initiator)
{
	const CachedNetworkId id (initiator);

	s_tradeRequestId = id;

	Transceivers::requested.emitMessage (initiator);
}

//----------------------------------------------------------------------

void ClientSecureTradeManager::onTradeRequestDenied                  (const NetworkId & recipient)
{
	Transceivers::requestDenied.emitMessage (recipient);
}

//----------------------------------------------------------------------

void ClientSecureTradeManager::onTradeRequestDeniedPlayerBusy        (const NetworkId & recipient)
{
	Transceivers::requestDeniedBusy.emitMessage (recipient);
}

//----------------------------------------------------------------------

void ClientSecureTradeManager::onTradeRequestDeniedPlayerUnreachable (const NetworkId & recipient)
{
	Transceivers::requestDeniedUnreachable.emitMessage (recipient);
}

//----------------------------------------------------------------------

void ClientSecureTradeManager::onBeginTrade          (const NetworkId & other)
{
	//-- someone is starting trade with us
	if (s_otherId == NetworkId::cms_invalid)
	{
		s_otherId = other;
		reset ();
	}
	else if (s_otherId != other)
	{
		WARNING (true, ("Begin trade with a bogus id"));
		reset ();
		s_otherId = NetworkId::cms_invalid;
		return;
	}
	//-- we started trade with someone else by dragging an item onto them
	else if (s_initiateItemId != NetworkId::cms_invalid)
	{
		addItem (s_initiateItemId);
	}

	s_tradeInProgress = true;
	s_initiateItemId == NetworkId::cms_invalid;
	Transceivers::beginTrade.emitMessage (true);
}

//----------------------------------------------------------------------

void ClientSecureTradeManager::onAddItem             (const NetworkId & item)
{
	CachedNetworkId id (item);

	//If we cannot resolve the id, then it hasn't been created yet.  Wait for it.
	if (!id.getObject())
	{
		s_waitingForObjects.push_back(id);
	}
	else
	{
		s_itemsOther.push_back (CachedNetworkId (item));
		Transceivers::addItem.emitMessage (Messages::AddItem::Payload (false, item));
		invalidateAcceptance ();
	}
}

//----------------------------------------------------------------------

void ClientSecureTradeManager::onRemoveItem          (const NetworkId & item)
{
	//First check the waiting list to see if we are still waiting for the create message
	CachedNetworkIdVector::iterator i = std::find(s_waitingForObjects.begin(), s_waitingForObjects.end(), item);
	if (i != s_waitingForObjects.end())
	{
		s_waitingForObjects.erase(i);
	}
	else
	{
		s_itemsOther.erase (std::remove (s_itemsOther.begin (), s_itemsOther.end (), CachedNetworkId (item)), s_itemsOther.end ());
		Transceivers::removeItem.emitMessage (Messages::RemoveItem::Payload (false, item));
		invalidateAcceptance ();
	}
}

//----------------------------------------------------------------------

void ClientSecureTradeManager::onGiveMoney          (int amount)
{
	s_moneyOther = amount;

	Transceivers::giveMoney.emitMessage (amount);
	invalidateAcceptance ();
}

//----------------------------------------------------------------------

void ClientSecureTradeManager::onAcceptTrade         (bool accept)
{
	s_okOther = accept;
	Transceivers::accept.emitMessage (accept);
}

//----------------------------------------------------------------------

void ClientSecureTradeManager::onObjectCreated(const CachedNetworkId & id)
{
	CachedNetworkIdVector::iterator i = std::find(s_waitingForObjects.begin(), s_waitingForObjects.end(), id);
	if (i != s_waitingForObjects.end())
	{
		s_waitingForObjects.erase(i);
		onAddItem(id);
	}

}

//----------------------------------------------------------------------

void ClientSecureTradeManager::onRequestVerification ()
{
	Transceivers::requestVerify.emitMessage (true);
}

//----------------------------------------------------------------------

void ClientSecureTradeManager::onVerifyTrade         ()
{
}

//----------------------------------------------------------------------

void ClientSecureTradeManager::onDenyTrade           ()
{
	s_okOther = false;
	s_okSelf  = false;
	Transceivers::accept.emitMessage (false);
}

//----------------------------------------------------------------------

void ClientSecureTradeManager::onTradeComplete       ()
{
	s_tradeInProgress = false;
	Transceivers::complete.emitMessage (true);
	reset ();
}

//----------------------------------------------------------------------

void ClientSecureTradeManager::onAbortTrade          ()
{
	s_tradeInProgress = false;
	Transceivers::abort.emitMessage (true);
	reset ();
}

//----------------------------------------------------------------------

void ClientSecureTradeManager::onAddItemFailed (const NetworkId & id)
{
	const CachedNetworkId cid (id);
	ClientObject * const clientObject = safe_cast<ClientObject *>(cid.getObject ());

	if (clientObject)
	{
		Unicode::String str;
		CuiStringVariablesManager::process (CuiStringIdsTrade::add_item_failed_prose, Unicode::emptyString,  clientObject->getLocalizedName (),Unicode::emptyString, str);
		CuiSystemMessageManager::sendFakeSystemMessage (str);
	}

	s_itemsSelf.erase (std::remove (s_itemsSelf.begin (), s_itemsSelf.end (), CachedNetworkId (id)), s_itemsSelf.end ());
	Transceivers::addItem.emitMessage (Messages::RemoveItem::Payload (true, id));
}

//----------------------------------------------------------------------

void  ClientSecureTradeManager::getItems    (CachedNetworkIdVector & selfItems, CachedNetworkIdVector & otherItems)
{
	selfItems  = s_itemsSelf;
	otherItems = s_itemsOther;
}

//----------------------------------------------------------------------

void ClientSecureTradeManager::getMoney    (int & self, int & other)
{
	self  = s_moneySelf;
	other = s_moneyOther;
}

//----------------------------------------------------------------------

void ClientSecureTradeManager::getAccepted (bool & self, bool & other)
{
	self  = s_okSelf;
	other = s_okOther;
}

//----------------------------------------------------------------------

const CachedNetworkId & ClientSecureTradeManager::getOther    ()
{
	return s_otherId;
}

//----------------------------------------------------------------------

const CachedNetworkId & ClientSecureTradeManager::getRequestInitiator ()
{
	return s_tradeRequestId;
}

//----------------------------------------------------------------------

void ClientSecureTradeManager::invalidateAcceptance ()
{
	s_okSelf  = false;
	s_okOther = false;
	Transceivers::accept.emitMessage (false);
}

// -------------------------------------------------------------
