//======================================================================
//
// CuiAuctionManager.h
// copyright (c) 2002 Sony Online Entertainment
//
//======================================================================

#ifndef INCLUDED_CuiAuctionManager_H
#define INCLUDED_CuiAuctionManager_H

//======================================================================

class NetworkId;
class StringId;

//----------------------------------------------------------------------

class CuiAuctionManager
{
public:
	
	struct Messages
	{
		typedef std::pair<int, Unicode::String> ResultInfo;

		struct Retrieved
		{
			typedef std::pair<NetworkId, ResultInfo> Payload;
		};

		struct Withdrawn
		{
			typedef std::pair<NetworkId, ResultInfo> Payload;
		};

	};
	
	static void              install ();
	static void              remove  ();

	typedef std::pair<int, Unicode::String> ResultInfo;

	static void              handleItemRetrieve    (const NetworkId & itemId);
	static void              handleAcceptBid       (const NetworkId & itemId, bool confirmed);
	static void              handleWithdraw        (const NetworkId & itemId, bool confirmed);
	static void              handleAuctionSale     (const NetworkId & itemId, int price, int seconds, bool instant, const Unicode::String & desc, bool confirmed, bool isPremium);
	static void              handleVendorSale      (const NetworkId & itemId, int price, const Unicode::String & desc, bool confirmed, bool isPremium, bool isVendorStockroomTransfer);
	static void              handleVendorRelist    (const stdvector<NetworkId>::fwd & itemIds);
	static void              handleReject          (const NetworkId & itemId, bool confirmed);

	static void              handleStartVendor     (const NetworkId & vendorId, const std::string & mediatorName);
	static void              handleStartAuction    (const NetworkId & marketId, const std::string & mediatorName);

	static void              handleOnItemRetrieved (const std::pair<NetworkId, ResultInfo> & payload);
	static void              handleOnAcceptBid     (const std::pair<NetworkId, ResultInfo> & payload);
	static void              handleOnWithdraw      (const std::pair<NetworkId, ResultInfo> & payload);
	static void              handleOnReject        (const std::pair<NetworkId, ResultInfo> & payload);
	static void              handleOnSell          (const std::pair<NetworkId, ResultInfo> & payload);

	static void              handleOnStartVendor   (const NetworkId & vendorId, int ownerCode, int resultCode, const std::string & marketFullName);
	static void              handleOnStartAuction  (const NetworkId & marketId, int resultCode, const std::string & marketFullName);

	static bool              isWaitingForRetrieve  (const NetworkId & id);
	static int               getOutstandingRetrieveRequestCount ();

	static bool              isWaitingForWithdraw  (const NetworkId & id);
	static int               getOutstandingWithdrawRequestCount ();

	static void              setLastContainerItemId(NetworkId const &itemId);
	static NetworkId const & getLastContainerItemId();

private:

	static void              handleGenericSell     (const NetworkId & itemId, int price, int seconds, bool instant, const Unicode::String & desc, const StringId & sid, bool isPremium, bool isVendorStockroomTransfer);

};

//======================================================================

#endif
