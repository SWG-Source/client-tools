//======================================================================
//
// AuctionManagerClientListener.cpp
// copyright (c) 2002 Sony Online Entertainment
//
//======================================================================

#include "clientGame/FirstClientGame.h"
#include "clientGame/AuctionManagerClientListener.h"

#include "clientGame/AuctionManagerClient.h"
#include "localizationArchive/StringIdArchive.h"
#include "sharedDebug/InstallTimer.h"
#include "sharedMessageDispatch/Message.h"
#include "sharedMessageDispatch/Receiver.h"
#include "sharedNetworkMessages/AcceptAuctionResponseMessage.h"
#include "sharedNetworkMessages/AuctionQueryHeadersMessage.h"
#include "sharedNetworkMessages/AuctionQueryHeadersResponseMessage.h"
#include "sharedNetworkMessages/BidAuctionResponseMessage.h"
#include "sharedNetworkMessages/CancelLiveAuctionResponseMessage.h"
#include "sharedNetworkMessages/CreateAuctionResponseMessage.h"
#include "sharedNetworkMessages/GenericValueTypeMessage.h"
#include "sharedNetworkMessages/GetAuctionDetailsResponse.h"
#include "sharedNetworkMessages/IsVendorOwnerResponseMessage.h"
#include "sharedNetworkMessages/RetrieveAuctionItemResponseMessage.h"

//======================================================================

namespace
{
	//----------------------------------------------------------------------

	class Listener : public MessageDispatch::Receiver
	{
	public:

		Listener () :
		MessageDispatch::Receiver ()
		{
			connectToMessage (AcceptAuctionResponseMessage::MessageType);
			connectToMessage (AuctionQueryHeadersResponseMessage::MessageType);
			connectToMessage (BidAuctionResponseMessage::MessageType);
			connectToMessage (CancelLiveAuctionResponseMessage::MessageType);
			connectToMessage (CreateAuctionResponseMessage::MessageType);
			connectToMessage (GetAuctionDetailsResponse::MessageType);
			connectToMessage (IsVendorOwnerResponseMessage::MessageType);
			connectToMessage (RetrieveAuctionItemResponseMessage::MessageType);
			connectToMessage ("CommoditiesItemTypeListResponse");
			connectToMessage ("CommoditiesResourceTypeListResponse");
		}

		void receiveMessage(const MessageDispatch::Emitter &, const MessageDispatch::MessageBase & message)
		{
			//----------------------------------------------------------------------

			if (message.isType (GetAuctionDetailsResponse::MessageType))
			{
				Archive::ReadIterator ri = NON_NULL (safe_cast<const GameNetworkMessage *>(&message))->getByteStream().begin();
				const GetAuctionDetailsResponse msg (ri);
				AuctionManagerClient::receiveDetails (msg.getDetails ());
			}
			//----------------------------------------------------------------------

			else if (message.isType (AuctionQueryHeadersResponseMessage::MessageType))
			{
				Archive::ReadIterator ri = NON_NULL (safe_cast<const GameNetworkMessage *>(&message))->getByteStream().begin();
				const AuctionQueryHeadersResponseMessage msg (ri);

				switch (msg.getTypeFlag ())
				{
				case AuctionQueryHeadersMessage::AST_ByAll:
					AuctionManagerClient::receiveHeaders          (msg.getAuctionData (), msg.getQueryOffset (), msg.getHasMorePages ());
					break;
				case AuctionQueryHeadersMessage::AST_ByPlayerSales:
					AuctionManagerClient::receiveHeadersSales     (msg.getAuctionData (), msg.getQueryOffset (), msg.getHasMorePages ());
					break;
				case AuctionQueryHeadersMessage::AST_ByPlayerBids:
					AuctionManagerClient::receiveHeadersBids      (msg.getAuctionData (), msg.getQueryOffset (), msg.getHasMorePages ());
					break;
				case AuctionQueryHeadersMessage::AST_ByPlayerStockroom:
					AuctionManagerClient::receiveHeadersAvailable (msg.getAuctionData (), msg.getQueryOffset (), msg.getHasMorePages ());
					break;
				case AuctionQueryHeadersMessage::AST_ByVendorSelling:
					AuctionManagerClient::receiveHeadersVendorSelling   (msg.getAuctionData (), msg.getQueryOffset (), msg.getHasMorePages ());
					AuctionManagerClient::receiveHeadersLocation  (msg.getAuctionData (), msg.getQueryOffset (), msg.getHasMorePages ());
					break;
				case AuctionQueryHeadersMessage::AST_ByVendorStockroom:
					AuctionManagerClient::receiveHeadersVendorStockroom (msg.getAuctionData (), msg.getQueryOffset (), msg.getHasMorePages ());
					break;
				case AuctionQueryHeadersMessage::AST_ByVendorOffers:
				case AuctionQueryHeadersMessage::AST_ByPlayerOffersToVendor:
					AuctionManagerClient::receiveHeadersVendorOffers    (msg.getAuctionData (), msg.getQueryOffset (), msg.getHasMorePages ());
					break;
				}
			}
			//----------------------------------------------------------------------

			else if (message.isType (CreateAuctionResponseMessage::MessageType))
			{
				Archive::ReadIterator ri = NON_NULL (safe_cast<const GameNetworkMessage *>(&message))->getByteStream().begin();
				const CreateAuctionResponseMessage msg (ri);
				AuctionManagerClient::receiveCreateAuctionResponse (msg.getItemId (), msg.getResult (), msg.getItemRestrictedRejectionMessage ());
			}

			//----------------------------------------------------------------------

			else if (message.isType (CancelLiveAuctionResponseMessage::MessageType))
			{
				Archive::ReadIterator ri = NON_NULL (safe_cast<const GameNetworkMessage *>(&message))->getByteStream().begin();
				const CancelLiveAuctionResponseMessage msg (ri);
				AuctionManagerClient::receiveCancelLiveAuctionResponse (msg.getItemId (), msg.getResult ());
			}

			//----------------------------------------------------------------------

			else if (message.isType (AcceptAuctionResponseMessage::MessageType))
			{
				Archive::ReadIterator ri = NON_NULL (safe_cast<const GameNetworkMessage *>(&message))->getByteStream().begin();
				const AcceptAuctionResponseMessage msg (ri);
				AuctionManagerClient::receiveAcceptAuctionResponseMessage (msg.getItemId (), msg.getResult ());
			}

			//----------------------------------------------------------------------

			else if (message.isType (BidAuctionResponseMessage::MessageType))
			{
				Archive::ReadIterator ri = NON_NULL (safe_cast<const GameNetworkMessage *>(&message))->getByteStream().begin();
				const BidAuctionResponseMessage msg (ri);
				AuctionManagerClient::receiveBidResponse (msg.getItemId (), msg.getResult ());
			}

			//----------------------------------------------------------------------

			else if (message.isType (RetrieveAuctionItemResponseMessage::MessageType))
			{
				Archive::ReadIterator ri = NON_NULL (safe_cast<const GameNetworkMessage *>(&message))->getByteStream().begin();
				const RetrieveAuctionItemResponseMessage msg (ri);
				AuctionManagerClient::receiveRetrieveResponse (msg.getItemId (), msg.getResult ());
			}

			//----------------------------------------------------------------------

			else if (message.isType (IsVendorOwnerResponseMessage::MessageType))
			{
				Archive::ReadIterator ri = NON_NULL (safe_cast<const GameNetworkMessage *>(&message))->getByteStream().begin();
				const IsVendorOwnerResponseMessage msg (ri);
				AuctionManagerClient::receiveVendorOwnerResponseMessage (msg.getOwnerResult (), msg.getResult (), msg.getContainerId (), msg.getMarketName ());
			}

			//----------------------------------------------------------------------

			else if (message.isType ("CommoditiesItemTypeListResponse"))
			{
				Archive::ReadIterator ri = NON_NULL (safe_cast<const GameNetworkMessage *>(&message))->getByteStream().begin();
				const GenericValueTypeMessage<std::pair<std::string, AuctionManagerClient::ItemTypeListServer> > msg (ri);
				AuctionManagerClient::receiveItemTypeList (msg.getValue ().first, msg.getValue ().second);
			}

			//----------------------------------------------------------------------

			else if (message.isType ("CommoditiesResourceTypeListResponse"))
			{
				Archive::ReadIterator ri = NON_NULL (safe_cast<const GameNetworkMessage *>(&message))->getByteStream().begin();
				const GenericValueTypeMessage<std::pair<std::string, AuctionManagerClient::ResourceTypeListServer> > msg (ri);
				AuctionManagerClient::receiveResourceTypeList (msg.getValue ().first, msg.getValue ().second);
			}
		}
	};

	Listener * s_listener = 0;
}

//----------------------------------------------------------------------

void AuctionManagerClientListener::install ()
{
	InstallTimer const installTimer("AuctionManagerClientListener::install");

	s_listener = new Listener;
}

//----------------------------------------------------------------------

void AuctionManagerClientListener::remove ()
{
	NOT_NULL (s_listener);
	delete s_listener;
	s_listener = 0;
}

//======================================================================
