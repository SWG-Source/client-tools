// ======================================================================
//
// AuctionQueryHeadersResponseMessage.cpp
//
// Copyright 2002 Sony Online Entertainment
//
// ======================================================================

#include "sharedFoundation/NetworkIdArchive.h"
#include "AuctionQueryArchive.h"
#include "sharedNetworkMessages/ClientCentralMessagesArchive.h"
#include "localizationArchive/StringIdArchive.h"
#include "sharedNetworkMessages/AuctionQueryHeadersResponseMessage.h"
#include "sharedNetworkMessages/FirstSharedNetworkMessages.h"

// ======================================================================

const char * const AuctionQueryHeadersResponseMessage::MessageType = "AuctionQueryHeadersResponseMessage";

//----------------------------------------------------------------------

AuctionQueryHeadersResponseMessage::AuctionQueryHeadersResponseMessage(int requestId, int typeFlag, AuctionHeaderVector const &auctionData, uint16 queryOffset, bool hasMorePages) :
	GameNetworkMessage  (MessageType),
	m_requestId         (requestId),
	m_typeFlag          (typeFlag),
	m_auctionData       (),
	m_queryOffset       (queryOffset),
	m_hasMorePages      (hasMorePages)
{
	m_auctionData = auctionData;
	AutoByteStream::addVariable(m_requestId);
	AutoByteStream::addVariable(m_typeFlag);
	AutoByteStream::addVariable(m_stringPalette);
	AutoByteStream::addVariable(m_wideStringPalette);
	palettizeAuctionData();
	AutoByteStream::addVariable(m_palettizedAuctionData);
	AutoByteStream::addVariable(m_queryOffset);
	AutoByteStream::addVariable(m_hasMorePages);
}

// ----------------------------------------------------------------------

AuctionQueryHeadersResponseMessage::AuctionQueryHeadersResponseMessage(Archive::ReadIterator & source) :
	GameNetworkMessage (MessageType),
	m_requestId        (),
	m_typeFlag         (),
	m_auctionData      (),
	m_queryOffset      (0),
	m_hasMorePages     (false)
{
	AutoByteStream::addVariable(m_requestId);
	AutoByteStream::addVariable(m_typeFlag);
	AutoByteStream::addVariable(m_stringPalette);
	AutoByteStream::addVariable(m_wideStringPalette);
	AutoByteStream::addVariable(m_palettizedAuctionData);
	AutoByteStream::addVariable(m_queryOffset);
	AutoByteStream::addVariable(m_hasMorePages);
	unpack(source);
	decodeAuctionData();
}

// ----------------------------------------------------------------------

AuctionQueryHeadersResponseMessage::~AuctionQueryHeadersResponseMessage()
{
}

//----------------------------------------------------------------------

uint16 AuctionQueryHeadersResponseMessage::getPaletteIndex(const std::string &str)
{
	std::vector<std::string> &stringPalette = m_stringPalette.get();
	for (uint16 i = 0; i < stringPalette.size(); ++i)
	{
		if (stringPalette[i] == str)
		{
			return i;	
		}
	}

	stringPalette.push_back(str);
	return static_cast<uint16>(stringPalette.size() - 1);
}

//----------------------------------------------------------------------

uint16 AuctionQueryHeadersResponseMessage::getPaletteIndex(const Unicode::String &str)
{
	std::vector<Unicode::String> & stringPalette = m_wideStringPalette.get();
	for (uint16 i = 0; i < stringPalette.size(); ++i)
	{
		if (stringPalette[i] == str)
		{
			return i;	
		}
	}

	stringPalette.push_back(str);
	return static_cast<uint16>(stringPalette.size() - 1);
}

//----------------------------------------------------------------------

void AuctionQueryHeadersResponseMessage::palettizeAuctionData()
{
	PalettizedAuctionHeaderVector palettizedData;
	palettizedData.resize (m_auctionData.size ());

	AuctionHeaderVector::const_iterator i;
	int index = 0;
	for (i = m_auctionData.begin(); i != m_auctionData.end(); ++i, ++index)
	{
		const Auction::ItemDataHeader & header = *i;
		Auction::PalettizedItemDataHeader & pal = palettizedData [index];

		pal.itemId                = header.itemId;
		pal.itemNameKey           = static_cast<uint8>(getPaletteIndex(header.itemName));
		pal.highBid               = header.highBid;
		pal.timer                 = header.timer;
		pal.buyNowPrice           = header.buyNowPrice;
		pal.locationKey           = getPaletteIndex(header.location);
		pal.ownerId               = header.ownerId;
		pal.ownerNameKey          = getPaletteIndex(header.ownerName);
		pal.highBidderId          = header.highBidderId;
		pal.highBidderNameKey     = getPaletteIndex(header.highBidderName);
		pal.maxProxyBid           = header.maxProxyBid;
		pal.myBid                 = header.myBid;
		pal.itemType              = header.itemType;
		pal.resourceContainerClassCrc = header.resourceContainerClassCrc;
		pal.flags                 = header.flags;
		pal.entranceCharge        = header.entranceCharge;
	}

	m_palettizedAuctionData.set(palettizedData);
}

//----------------------------------------------------------------------

void AuctionQueryHeadersResponseMessage::decodeAuctionData()
{
	const std::vector<std::string> & stringPalette          = m_stringPalette.get();
	const std::vector<Unicode::String> & wideStringPalette  = m_wideStringPalette.get();
	const PalettizedAuctionHeaderVector & palettizedData    = m_palettizedAuctionData.get();

	m_auctionData.resize (palettizedData.size ());

	int index = 0;
	PalettizedAuctionHeaderVector::const_iterator i;
	for (i = palettizedData.begin(); i != palettizedData.end(); ++i, ++index)
	{
		const Auction::PalettizedItemDataHeader & pal = *i;

		Auction::ItemDataHeader & header = m_auctionData [index];

		header.itemId                = pal.itemId;
		header.itemName              = wideStringPalette[pal.itemNameKey];
		header.highBid               = pal.highBid;
		header.timer                 = pal.timer;
		header.buyNowPrice           = pal.buyNowPrice;
		header.location              = stringPalette[pal.locationKey];
		header.ownerId               = pal.ownerId;
		header.ownerName             = stringPalette[pal.ownerNameKey];
		header.highBidderId          = pal.highBidderId;
		header.highBidderName        = stringPalette[pal.highBidderNameKey];
		header.maxProxyBid           = pal.maxProxyBid;
		header.myBid                 = pal.myBid;
		header.itemType              = pal.itemType;
		header.resourceContainerClassCrc = pal.resourceContainerClassCrc;
		header.flags                 = pal.flags;
		header.entranceCharge        = pal.entranceCharge;
	}
}

//----------------------------------------------------------------------

// ======================================================================

