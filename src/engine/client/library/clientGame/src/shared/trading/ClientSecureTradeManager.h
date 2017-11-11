// ======================================================================
//
// ClientSecureTradeManager.h
// Copyright 2001, 2002 Sony Online Entertainment Inc.
// All Rights Reserved.
// 
// ======================================================================


#ifndef INCLUDED_ClientSecureTradeManager_H
#define INCLUDED_ClientSecureTradeManager_H

//----------------------------------------------------------------------

class ClientObject;
class CreatureObject;
class CachedNetworkId;

#include "../../../../../../engine/shared/library/sharedFoundation/include/public/sharedFoundation/NetworkId.h"
#include "sharedMessageDispatch/Receiver.h"

//----------------------------------------------------------------------

class ClientSecureTradeManager
{

public:

	struct Messages
	{
		struct AbortTrade
		{
			typedef bool Payload;
		};
		
		struct BeginTrade
		{
			typedef bool Payload;
		};

		struct AddItem
		{
			//-- bool true = self
			typedef std::pair<bool, NetworkId> Payload;
		};

		struct RemoveItem
		{
			//-- bool true = self
			typedef std::pair<bool, NetworkId> Payload;
		};

		struct GiveMoney
		{
			typedef int Payload;
		};

		struct Accept
		{
			typedef bool Payload;
		};

		struct RequestVerify
		{
			typedef bool Payload;
		};

		struct Complete
		{
			typedef bool Payload;
		};

		struct Requested
		{
			typedef NetworkId Payload;
		};

		struct RequestDenied
		{
			typedef NetworkId Payload;
		};

		struct RequestDeniedBusy
		{
			typedef NetworkId Payload;
		};

		struct RequestDeniedUnreachable
		{
			typedef NetworkId Payload;
		};
	};

	static void install();

	//Outbound messages
	static bool initiateTrade                         (const NetworkId & player, const NetworkId & item, bool findme);
	static void denyTradeRequest                      ();
	static void acceptTradeRequest                    ();
	static void addItem                               (const NetworkId &);
	static void removeItem                            (const NetworkId &);
	static void giveMoney                             (int amount);
	static void acceptTrade                           ();
	static void unacceptTrade                         ();
	static void verifyTrade                           ();
	static void denyTrade                             ();
	static void abortTrade                            ();

	//Incoming messages

	static void onTradeRequested                      (const NetworkId & initiator);
	static void onTradeRequestDenied                  (const NetworkId & recipient);
	static void onTradeRequestDeniedPlayerBusy        (const NetworkId & recipient);
	static void onTradeRequestDeniedPlayerUnreachable (const NetworkId & recipient);

	typedef stdvector<CachedNetworkId>::fwd CachedNetworkIdVector;

	static void                    getItems            (CachedNetworkIdVector & selfItems, CachedNetworkIdVector & otherItems);
	static void                    getMoney            (int & self, int & other);
	static void                    getAccepted         (bool & self, bool & other);
	static const CachedNetworkId & getOther            ();
	static const CachedNetworkId & getRequestInitiator ();

	static void onBeginTrade          (const NetworkId & other);
	static void onAddItem             (const NetworkId & item);
	static void onAddItemFailed       (const NetworkId & item);
	static void onRemoveItem          (const NetworkId & item);
	static void onGiveMoney           (int amount);
	static void onAcceptTrade         (bool accept);
	static void onRequestVerification ();
	static void onVerifyTrade         ();
	static void onDenyTrade           ();
	static void onTradeComplete       ();
	static void onAbortTrade          ();
	
	static void onObjectCreated (const CachedNetworkId &);

private:

	static void remove();
	static void invalidateAcceptance ();

	ClientSecureTradeManager();
	~ClientSecureTradeManager();

private:
	class TestReceiver : public MessageDispatch::Receiver
	{
	public:
		void receiveMessage(const MessageDispatch::Emitter & source, const MessageDispatch::MessageBase & message);
	};

	static TestReceiver *m_receiver;
};

/*

  General flow:

  1. Initiate the trade with another player.  This involves sending a request to them and them accepting your request.
  2. Begin trading.  Players add items to their respective sides.  When you are satisfied with your side, you accept.  When both parties accept we move to next stage
  3. Verify trade.  Players must now verify the trade to complete it.   Any adds/remove/alters to the trade causes it to fall back to the previous stage.
  4. Complete trade.  Complete the trade, transferring the items.
  */


/*
	//Outbound messages
	static void initiateTrade(NetworkId player);  request to begin trading with another player
	static void denyTradeRequest(); deny someone's request to trade with you
	static void acceptTradeRequest(); accept someone's request to trade with you
	static void addItem(Object &); add an item to your side of the trade
	static void removeItem(Object &); remove an item from your side of the trade
	static void alterMoney(int amount); alter the amount of money you wish to give the other person (can be negative)
	static void acceptTrade(); when both sides are acceptable to you, send this
	static void unacceptTrade(); if you decide the trade is unacceptable before the verification stage, send this
	static void verifyTrade(); in the verification stage, accept the trade
	static void denyTrade(); in the verification stage, deny the trade...go back to trade phase.
	static void abortTrade();  cause the trade to exit entirely transferring no items.

	//Incoming messages
	static void onRequestDenied();  If you requested a trade, this indicates that the trade has been rejected
	static void onBeginTrade();  Receipt of this message initiates the trade phase
	static void onAddItem(Object &); An item has been added to the other side
	static void onRemoveItem(Object &); An item has been removed from the other side
	static void onAlterMoney(int amount); Money trasfer has been altered
	static void onAcceptTrade(); The other party has accepted their side
	static void onUnAcceptTrade (); The other party has cancelled their acceptance
	static void onRequestVerification (); Both parties have accepted, we need verification. Begins the verification phase
	static void onVerifyTrade (); Other party has verified the trade
	static void onDenyTrade(); Other party has denied the trade.
	static void onTradeComplete(); Trade is successful.
*/


#endif