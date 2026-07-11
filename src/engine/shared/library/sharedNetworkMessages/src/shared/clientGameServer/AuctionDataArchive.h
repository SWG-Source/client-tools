// ======================================================================
//
// AuctionDataArchive.h
//
// Copyright 2002 Sony Online Entertainment
//
// ======================================================================

#ifndef	_AuctionDataArchive_H
#define	_AuctionDataArchive_H

namespace Auction
{
	struct PalettizedItemDataHeader;
	struct ItemDataDetails;
}

namespace Archive
{
	class ReadIterator;
	class ByteStream;

	void get(ReadIterator& source, Auction::PalettizedItemDataHeader& target);
	void put(ByteStream& target, Auction::PalettizedItemDataHeader const& source);

	void get(ReadIterator& source, Auction::ItemDataDetails& target);
	void put(ByteStream& target, Auction::ItemDataDetails const& source);
}

#endif // _AuctionDataArchive_H
