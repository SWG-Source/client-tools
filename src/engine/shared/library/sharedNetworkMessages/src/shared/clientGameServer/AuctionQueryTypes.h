// ======================================================================
//
// AuctionQueryTypes.h
//
// Copyright 2002 Sony Online Entertainment
//
// ======================================================================

#ifndef	_AuctionQueryAuctionQueryTypes_H
#define	_AuctionQueryAuctionQueryTypes_H

//-----------------------------------------------------------------------

#include "sharedFoundation/Tag.h"
#include "Unicode.h"

//-----------------------------------------------------------------------
struct AuctionQueryResponse_AuctionData
{
	int64 auctionId;
	std::string location;
	int64 ownerId;
	int32 minBid;
	int32 timer;
	int64 itemId;
	int32 soldFlag;
	int64 highBidderId;
	int32 itemType;
	int32 resourceContainerClassCrc;
	int16 itemQuantity;
	int32 itemTimer;
	int32 highBid;
	int32 highBidMaxProxy;
};

enum AuctionSearchType
{
	AST_ByCategory,
	AST_ByLocation,
	AST_ByAll,
	AST_ByPlayerSales,
	AST_ByPlayerBids,
	AST_ByPlayerStockroom,
	AST_ByVendorOffers,
	AST_ByVendorSelling,
	AST_ByVendorStockroom,
	AST_ByPlayerOffersToVendor
};

enum AuctionLocationSearch
{
	ALS_Galaxy,
	ALS_Planet,
	ALS_Region,
	ALS_Market
};

enum SearchConditionComparison
{
	// DO NOT CHANGE THE ORDERING OR THE NUMBERING
	// ADD NEW ENUM VALUE BEFORE SCC_LAST
	SCC_int = 0,
	SCC_float,
	SCC_string_equal,
	SCC_string_not_equal,
	SCC_string_contain,
	SCC_string_not_contain,
	SCC_LAST
};

// order of enum value MUST match the order defined in the combo box for dsComboMatchAllAny defined in ui_auction.inc
enum AdvancedSearchMatchAllAny
{
	// DO NOT CHANGE THE ORDERING OR THE NUMBERING
	// ADD NEW ENUM VALUE BEFORE ASMAA_LAST
	ASMAA_match_all = 0,
	ASMAA_match_any,
	ASMAA_not_match_all,
	ASMAA_not_match_any,
	ASMAA_LAST
};

class SearchCondition
{
public:
	SearchCondition() :
		attributeNameCrc(), requiredAttribute(false), comparison(SCC_int), intMin(0), intMax(0), floatMin(0.0f), floatMax(0.0f), stringValue() {};

	SearchCondition(SearchCondition const & rhs) :
		attributeNameCrc(rhs.attributeNameCrc), requiredAttribute(rhs.requiredAttribute), comparison(rhs.comparison), intMin(rhs.intMin), intMax(rhs.intMax), floatMin(rhs.floatMin), floatMax(rhs.floatMax), stringValue(rhs.stringValue) {};

	explicit SearchCondition(uint32 const pAttributeNameCrc, bool const pRequiredAttribute, int const pIntMin, int const pIntMax) :
		attributeNameCrc(pAttributeNameCrc), requiredAttribute(pRequiredAttribute), comparison(SCC_int), intMin(pIntMin), intMax(pIntMax), floatMin(0.0f), floatMax(0.0f), stringValue() {};

	explicit SearchCondition(uint32 const pAttributeNameCrc, bool const pRequiredAttribute, double const pFloatMin, double const pFloatMax) :
		attributeNameCrc(pAttributeNameCrc), requiredAttribute(pRequiredAttribute), comparison(SCC_float), intMin(0), intMax(0), floatMin(pFloatMin), floatMax(pFloatMax), stringValue() {};

	explicit SearchCondition(uint32 const pAttributeNameCrc, bool const pRequiredAttribute, SearchConditionComparison const pComparison, std::string const & pStringValue) :
		attributeNameCrc(pAttributeNameCrc), requiredAttribute(pRequiredAttribute), comparison(pComparison), intMin(0), intMax(0), floatMin(0.0f), floatMax(0.0f), stringValue(pStringValue) {};

	SearchCondition &operator =(SearchCondition const & rhs)
	{
		if (this != (&rhs))
		{
			attributeNameCrc = rhs.attributeNameCrc;
			requiredAttribute = rhs.requiredAttribute;
			comparison = rhs.comparison;
			intMin = rhs.intMin;
			intMax = rhs.intMax;
			floatMin = rhs.floatMin;
			floatMax = rhs.floatMax;
			stringValue = rhs.stringValue;
		}

		return *this;
	}

	bool operator== (SearchCondition const & rhs) const
	{
		if (this == &rhs)
		{
			return true;
		}

		if ((attributeNameCrc != rhs.attributeNameCrc) ||
			(requiredAttribute != rhs.requiredAttribute) ||
			(comparison != rhs.comparison))
		{
			return false;
		}

		if (comparison == SCC_int)
		{
			return ((intMin == rhs.intMin) && (intMax == rhs.intMax));
		}
		else if (comparison == SCC_float)
		{
			return ((floatMin == rhs.floatMin) && (floatMax == rhs.floatMax));
		}

		return (stringValue == rhs.stringValue);
	}

	bool operator!= (SearchCondition const & rhs) const
	{
		return !operator==(rhs);
	}

	uint32 attributeNameCrc;
	bool requiredAttribute;
	SearchConditionComparison comparison;

	int intMin;
	int intMax;

	double floatMin;
	double floatMax;

	std::string stringValue;
};

// ======================================================================

#endif // _AuctionQueryHeadersMessage_H
