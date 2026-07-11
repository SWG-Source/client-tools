// ======================================================================
//
// AuctionQueryArchive.h
//
// Copyright 2002 Sony Online Entertainment
//
// ======================================================================

#ifndef	_AuctionQueryArchive_H
#define	_AuctionQueryArchive_H

//-----------------------------------------------------------------------

class SearchCondition;
struct AuctionQueryResponse_AuctionData;

namespace Archive
{
	class ReadIterator;
	class ByteStream;

	// forward declare before AutoList
	void get(ReadIterator& source, SearchCondition& target);
	void put(ByteStream& target, const SearchCondition& source);

	void get(ReadIterator& source, AuctionQueryResponse_AuctionData& target);
	void put(ByteStream& target, AuctionQueryResponse_AuctionData const& source);
}

// ======================================================================

#endif // _AuctionQueryArchive_H
