// ======================================================================
//
// AuctionQueryArchive.cpp
//
// Copyright 2002 Sony Online Entertainment
//
// ======================================================================

#include "unicodeArchive/UnicodeArchive.h"
#include "sharedFoundation/NetworkIdArchive.h"
#include "sharedNetworkMessages/FirstSharedNetworkMessages.h"
#include "AuctionQueryTypes.h"
#include "sharedNetworkMessages/AuctionQueryArchive.h"

// ======================================================================

namespace Archive
{
	void get(ReadIterator & source, SearchCondition & target)
	{
		get(source, target.attributeNameCrc);
		get(source, target.requiredAttribute);

		int8 temp;
		get(source, temp);
		target.comparison = static_cast<SearchConditionComparison>(temp);

		if (target.comparison == SCC_int)
		{
			get(source, target.intMin);
			get(source, target.intMax);
		}
		else if (target.comparison == SCC_float)
		{
			get(source, target.floatMin);
			get(source, target.floatMax);
		}
		else if ((target.comparison == SCC_string_equal) ||
		         (target.comparison == SCC_string_not_equal) ||
		         (target.comparison == SCC_string_contain) ||
		         (target.comparison == SCC_string_not_contain))
		{
			get(source, target.stringValue);
		}
		else // error
		{
			target.comparison = SCC_int;
		}
	}

	void put(ByteStream & target, const SearchCondition & source)
	{
		put(target, source.attributeNameCrc);
		put(target, source.requiredAttribute);
		put(target, static_cast<int8>(source.comparison));

		if (source.comparison == SCC_int)
		{
			put(target, source.intMin);
			put(target, source.intMax);
		}
		else if (source.comparison == SCC_float)
		{
			put(target, source.floatMin);
			put(target, source.floatMax);
		}
		else if ((source.comparison == SCC_string_equal) ||
		         (source.comparison == SCC_string_not_equal) ||
		         (source.comparison == SCC_string_contain) ||
		         (source.comparison == SCC_string_not_contain))
		{
			put(target, source.stringValue);
		}
	}

	void get(ReadIterator& source, AuctionQueryResponse_AuctionData& target)
	{
		get(source, target.auctionId);
		get(source, target.location);
		get(source, target.ownerId);
		get(source, target.minBid);
		get(source, target.timer);
		get(source, target.itemId);
		get(source, target.soldFlag);
		get(source, target.highBidderId);
		get(source, target.itemType);
		get(source, target.resourceContainerClassCrc);
		get(source, target.itemQuantity);
		get(source, target.itemTimer);
		get(source, target.highBid);
		get(source, target.highBidMaxProxy);
	}

	void put(ByteStream& target, AuctionQueryResponse_AuctionData const& source)
	{
		put(target, source.auctionId);
		put(target, source.location);
		put(target, source.ownerId);
		put(target, source.minBid);
		put(target, source.timer);
		put(target, source.itemId);
		put(target, source.soldFlag);
		put(target, source.highBidderId);
		put(target, source.itemType);
		put(target, source.resourceContainerClassCrc);
		put(target, source.itemQuantity);
		put(target, source.itemTimer);
		put(target, source.highBid);
		put(target, source.highBidMaxProxy);
	}
}
