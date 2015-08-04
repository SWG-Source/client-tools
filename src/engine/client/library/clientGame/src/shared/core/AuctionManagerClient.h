//======================================================================
//
// AuctionManagerClient.h
// copyright (c) 2002 Sony Online Entertainment
//
//======================================================================

#ifndef INCLUDED_AuctionManagerClient_H
#define INCLUDED_AuctionManagerClient_H

#include "sharedNetworkMessages/AuctionQueryHeadersMessage.h"

//======================================================================

class AuctionManagerClientData;
class NetworkId;
class CachedNetworkId;
class ClientObject;
class StringId;

namespace Auction
{
	struct ItemDataHeader;
	struct ItemDataDetails;
}

//----------------------------------------------------------------------

class AuctionManagerClient
{
public:

	typedef AuctionManagerClientData Data;
	typedef stdvector<Data>::fwd     DataVector;
	typedef Auction::ItemDataHeader  Header;
	typedef Auction::ItemDataDetails Details;

	typedef stdvector<Auction::ItemDataHeader>::fwd AuctionHeaderVector;

	// if the display string is the same, then compare based on the item's crc;
	// this way, same named items will always appear in the same order in the
	// object type filter tree, so at least after some experimentation, you will
	// know that "the 3rd Misc->Furniture->Portrait is always the large female
	// twi'lek painting #1
	struct ItemTypeNameListLessCompare
	{
		bool operator()(std::pair<Unicode::String, std::pair<int, std::string> > s1, std::pair<Unicode::String, std::pair<int, std::string> > s2) const
		{
			const Unicode::String lowerCase1 = Unicode::toLower(s1.first);
			const Unicode::String lowerCase2 = Unicode::toLower(s2.first);

			if (lowerCase1 < lowerCase2)
				return true;
			else if (lowerCase1 == lowerCase2)
				return (s1.second < s2.second);

			return false;
		}
	};

	typedef stdset<std::pair<Unicode::String, std::pair<int, std::string> >, ItemTypeNameListLessCompare>::fwd ItemTypeNameList;
	typedef stdmap<int, ItemTypeNameList>::fwd ItemTypeList;

	typedef stdmap<int, std::pair<int, StringId> >::fwd ItemTypeNameListServer;
	typedef stdmap<int, ItemTypeNameListServer>::fwd ItemTypeListServer;

	typedef stdmap<std::string, int>::fwd ResourceTypeNameList;
	typedef stdmap<int, ResourceTypeNameList>::fwd ResourceTypeList;

	typedef stdset<std::string>::fwd ResourceTypeNameListServer;
	typedef stdmap<int, ResourceTypeNameListServer>::fwd ResourceTypeListServer;

	struct Messages
	{
		typedef std::pair<int, Unicode::String> ResultInfo;

		struct ListAllChanged
		{
			typedef uint8 Payload;
		};

		struct ListSalesChanged
		{
			typedef uint8 Payload;
		};

		struct ListBidsChanged
		{
			typedef uint8 Payload;
		};

		struct ListAvailableChanged
		{
			typedef uint8 Payload;
		};

		struct ListVendorSellingChanged
		{
			typedef uint8 Payload;
		};

		struct ListVendorOffersChanged
		{
			typedef uint8 Payload;
		};

		struct ListVendorStockroomChanged
		{
			typedef uint8 Payload;
		};

		struct ListVendorLocationChanged
		{
			typedef uint8 Payload;
		};

		struct DetailsReceived
		{
			typedef Data Payload;
		};

		struct CreateAuctionResponse
		{
			typedef std::pair<NetworkId, ResultInfo> Payload;
		};

		struct BidResponse
		{
			typedef std::pair<NetworkId, ResultInfo> Payload;
		};

		struct CancelLiveAuctionResponse
		{
			typedef std::pair<NetworkId, ResultInfo> Payload;
		};

		struct AuctionToViewChanged
		{
			typedef NetworkId Payload;
		};

		struct AuctionToBidOnChanged
		{
			typedef NetworkId Payload;
		};

		struct AuctionToSetPriceOnChanged
		{
			typedef NetworkId Payload;
		};

		struct TraverseAuctionToView
		{
			typedef bool Payload;
		};

		struct Retrieved
		{
			typedef std::pair<NetworkId, ResultInfo> Payload;
		};

		struct Withdrawn
		{
			typedef std::pair<NetworkId, ResultInfo> Payload;
		};

		struct RetrieveRequested
		{
			typedef NetworkId Payload;
		};

		struct WithdrawRequested
		{
			typedef NetworkId Payload;
		};

		struct AcceptBidResponse
		{
			typedef std::pair<NetworkId, ResultInfo> Payload;
		};

		struct VendorOwnerResponse
		{
			typedef std::pair<int, int> Codes;
			typedef std::pair<Codes, std::string> CodesName;
			typedef std::pair<NetworkId, CodesName> Payload;
		};

		struct ItemTypeListChanged
		{
			typedef std::string Payload;
		};

		struct ResourceTypeListChanged
		{
			typedef std::string Payload;
		};
	};

	static void                      install                      ();
	static void                      remove                       ();
	static void                      reset                        ();
	static void                      pruneObjects                 ();

	static void                      receiveHeaders                      (const AuctionHeaderVector & ahv, int queryOffset, bool hasMorePages);
	static void                      receiveHeadersSales                 (const AuctionHeaderVector & ahv, int queryOffset, bool hasMorePages);
	static void                      receiveHeadersBids                  (const AuctionHeaderVector & ahv, int queryOffset, bool hasMorePages);
	static void                      receiveHeadersAvailable             (const AuctionHeaderVector & ahv, int queryOffset, bool hasMorePages);

	static void                      receiveHeadersVendorSelling         (const AuctionHeaderVector & ahv, int queryOffset, bool hasMorePages);
	static void                      receiveHeadersVendorOffers          (const AuctionHeaderVector & ahv, int queryOffset, bool hasMorePages);
	static void                      receiveHeadersVendorStockroom       (const AuctionHeaderVector & ahv, int queryOffset, bool hasMorePages);
	static void                      receiveHeadersLocation(const AuctionHeaderVector & ahv, int queryOffset, bool hasMorePages);

	static void                      handleReceiveHeaders                (int type, const AuctionHeaderVector & ahv, int queryOffset, bool hasMorePages);

	static void                      receiveCreateAuctionResponse        (const NetworkId & id, int resultCode, const std::string & itemRestrictedRejectionMessage);
	static void                      receiveBidResponse                  (const NetworkId & id, int resultCode);
	static void                      receiveCancelLiveAuctionResponse    (const NetworkId & id, int resultCode);
	static void                      receiveAcceptAuctionResponseMessage (const NetworkId & id, int resultCode);
	static void                      receiveDetails                      (const Auction::ItemDataDetails & details);
	static void                      receiveRetrieveResponse             (const NetworkId & id, int resultCode);

	static void                      receiveVendorOwnerResponseMessage   (int ownerResult, int resultCode, const NetworkId & containerId, const std::string & marketName);

	static void                      receiveItemTypeList                 (const std::string & itemTypeListVersion, const ItemTypeListServer & itemTypeList);

	static void                      receiveResourceTypeList             (const std::string & resourceTypeListVersion, const ResourceTypeListServer & resourceTypeList);

	enum AuctionListType
	{
		T_all,
		T_mySales,
		T_myBids,
		T_available,
		T_vendorSelling,
		T_vendorOffers,
		T_vendorStockroom,
		T_location,
		T_numTypes		
	};

	static void                      getAuctions              (AuctionListType alt, DataVector & dv);
	static void                      getAuctions              (AuctionListType alt, DataVector & dv, int & currentIndex, bool & hasMoreAuctions, int & pageSize);

	static uint8                     requestDetails           (const NetworkId & itemId);
	static uint8                     getMinTextFilterTokenLength();
	static uint8                     requestAuctions          (int locationSearchType, int objectType, bool objectTypeExactMatch, int objectTemplateId, const Unicode::String & textFilterAll, const Unicode::String & textFilterAny, int priceFilterMin, int priceFilterMax, bool priceFilterIncludesFee, const std::list<AuctionQueryHeadersMessage::SearchCondition> & advancedSearch, AuctionQueryHeadersMessage::AdvancedSearchMatchAllAny advancedSearchMatchAllAny, int startingIndex);
	static uint8                     requestSales             (int startingIndex);
	static uint8                     requestBids              (int startingIndex);
	static uint8                     requestAvailable         (int startingIndex);
	static uint8                     requestVendorSelling     (bool sellerView, int locationSearchType, int objectType, bool objectTypeExactMatch, int objectTemplateId, const Unicode::String & textFilterAll, const Unicode::String & textFilterAny, int priceFilterMin, int priceFilterMax, bool priceFilterIncludesFee, const std::list<AuctionQueryHeadersMessage::SearchCondition> & advancedSearch, AuctionQueryHeadersMessage::AdvancedSearchMatchAllAny advancedSearchMatchAllAny, int startingIndex);
	static uint8                     requestVendorOffers      (bool playerOffers, int locationSearchType, int objectType, bool objectTypeExactMatch, int objectTemplateId, const Unicode::String & textFilterAll, const Unicode::String & textFilterAny, int priceFilterMin, int priceFilterMax, bool priceFilterIncludesFee, const std::list<AuctionQueryHeadersMessage::SearchCondition> & advancedSearch, AuctionQueryHeadersMessage::AdvancedSearchMatchAllAny advancedSearchMatchAllAny, int startingIndex);
	static uint8                     requestVendorStockroom   (int locationSearchType, int objectType, bool objectTypeExactMatch, int objectTemplateId, const Unicode::String & textFilterAll, const Unicode::String & textFilterAny, int priceFilterMin, int priceFilterMax, bool priceFilterIncludesFee, const std::list<AuctionQueryHeadersMessage::SearchCondition> & advancedSearch, AuctionQueryHeadersMessage::AdvancedSearchMatchAllAny advancedSearchMatchAllAny, int startingIndex);
	static uint8                     requestLocations         (int locationSearchType, int objectType, bool objectTypeExactMatch, int objectTemplateId, const Unicode::String & textFilterAll, const Unicode::String & textFilterAny, int priceFilterMin, int priceFilterMax, bool priceFilterIncludesFee, const std::list<AuctionQueryHeadersMessage::SearchCondition> & advancedSearch, AuctionQueryHeadersMessage::AdvancedSearchMatchAllAny advancedSearchMatchAllAny, int startingIndex);

	static void                      requestItemTypeList      ();
	static const ItemTypeNameList &  getItemTypeNameList      (int got);
	static const ItemTypeNameListServer & getItemTypeNameListServer(int got);
	static const std::string &       getItemTypeListVersion   ();

	static void                      requestResourceTypeList  ();
	static const ResourceTypeNameList & getResourceTypeNameList  (int resourceClassCrc);
	static const std::string &       getResourceTypeListVersion   ();

	static bool                      isListRequestOutstanding (AuctionListType alt);
	static bool                      isListValid              (AuctionListType alt);

	static bool                      getAuctionData           (const NetworkId & itemId, Data & data);

	static void                      makeBid                  (const NetworkId & itemId, int bid, int proxyBid, bool isBuying);
	static void                      sellItem                 (const NetworkId & container, const NetworkId & item, const Unicode::String & itemLocalizedName, int bid, bool instantSale, int timeLimitSecs, const Unicode::String & desc, bool isPremium, bool isVendorStockroomTransfer);
	static void                      relistItemsFromStock     (const stdvector<NetworkId>::fwd & itemIds, int timeLimitSecs);
	static void                      withdrawItem             (const NetworkId & itemId);
	static void                      retrieve                 (const NetworkId & itemId, const NetworkId & marketId);
	static void                      acceptBid                (const NetworkId & itemId);

	static void                      setMarketObjectId        (const NetworkId & id);
	static void                      setMarketRegionName      (const std::string & regionName);

	static const CachedNetworkId &   getMarketObjectId        ();
	static bool                      getMarketLocationStrings (std::string & planet, std::string & city, std::string & market);

	static void                      setAuctionToBidOn        (const NetworkId & auctionToBidOn);
	static const NetworkId &         getAuctionToBidOn        ();

	static void                      setAuctionToSetPriceOn   (const NetworkId & auctionToSetPriceOn);
	static const NetworkId &         getAuctionToSetPriceOn   ();

	static void                      setAuctionToView         (const NetworkId & auctionToView);
	static const NetworkId &         getAuctionToView         ();

	static ClientObject *            getClientObjectForAuction (const NetworkId & auctionId);

	static Data *                    findAuction               (DataVector & dv, const NetworkId & auctionId);
	static Data *                    findAuction               (const NetworkId & auctionId);

	static void                      traverseAuctionToView     (bool forward);

	static bool                      isAtCommodityMarket       ();
	static bool                      isAtVendor                (bool & playerOwns);

	static void                      setIsAtCommodityMarket    (bool b);
	static void                      setIsAtVendor             (bool b, bool playerOwns);

	static void                      setItemToSellFromStock    (const NetworkId & itemId);
	static void                      setItemsToSellFromStock   (const stdvector<NetworkId>::fwd & itemIds);
	static const stdvector<NetworkId>::fwd & getItemsToSellFromStock();

	static bool                      isInStockroom             (const NetworkId & itemId);

	static bool                      isItemRetrievable         (const NetworkId & itemId);
	static bool                      isItemWithdrawable        (const NetworkId & itemId);
	static bool                      isItemAcceptable          (const NetworkId & itemId);

	static bool                      localizeLocation          (const std::string & location, Unicode::String & result, bool compactFormat, bool showObjectName);
	static bool                      localizeLocation          (const std::string & location, Unicode::String & planet, Unicode::String & region, Unicode::String & objectName);
	static bool                      separateLocation          (const std::string & location, std::string & planet, std::string & region, std::string & name, NetworkId & id);

	static void                      update                    (float elapsedTime);

	static bool                      hasDetails               (const NetworkId & itemId);
	static void                      getTooltipsFromDetails   (const NetworkId & itemId, Unicode::String & tooltip);
	
private:

	static bool                      removeAuction             (int type, const NetworkId & auctionId);
	static bool                      removeAuction             (const NetworkId & auctionId);
	static void                      addAuction                (int type, const Data & data);
	static void                      fireListChanged           (int type);

	static CachedNetworkId           ms_marketObjectId;
	static NetworkId                 ms_auctionToBidOn;
	static NetworkId                 ms_auctionToView;
	static NetworkId                 ms_auctionToSetPriceOn;
	static stdvector<NetworkId>::fwd ms_itemsToSellFromStock;

	static bool                      ms_atCommodityMarket;
	static bool                      ms_atVendor;
	static bool                      ms_playerOwnsVendor;

	static bool                      s_listRequestOutstanding [AuctionManagerClient::T_numTypes];
	static bool                      s_listValid              [AuctionManagerClient::T_numTypes];
};

//----------------------------------------------------------------------

inline const CachedNetworkId & AuctionManagerClient::getMarketObjectId        ()
{
	return ms_marketObjectId;
}

//----------------------------------------------------------------------

inline const NetworkId & AuctionManagerClient::getAuctionToBidOn ()
{
	return ms_auctionToBidOn;
}

//----------------------------------------------------------------------

inline const NetworkId & AuctionManagerClient::getAuctionToView ()
{
	return ms_auctionToView;
}

//----------------------------------------------------------------------

inline const NetworkId & AuctionManagerClient::getAuctionToSetPriceOn   ()
{
	return ms_auctionToSetPriceOn;
}

//----------------------------------------------------------------------

inline const stdvector<NetworkId>::fwd & AuctionManagerClient::getItemsToSellFromStock    ()
{
	return ms_itemsToSellFromStock;
}

//----------------------------------------------------------------------

inline bool AuctionManagerClient::isListRequestOutstanding (AuctionListType alt)
{
	if (alt >= T_all && alt < T_numTypes)
		return s_listRequestOutstanding [alt];
	return false;
}

//----------------------------------------------------------------------

inline bool AuctionManagerClient::isListValid (AuctionListType alt)
{
	if (alt >= T_all && alt < T_numTypes)
		return s_listValid [alt];
	return false;

}

//======================================================================

#endif
