// ======================================================================
//
// AuctionQueryHeadersMessage.h
//
// Copyright 2002 Sony Online Entertainment
//
// ======================================================================

#ifndef	_AuctionQueryHeadersMessage_H
#define	_AuctionQueryHeadersMessage_H

//-----------------------------------------------------------------------

#include "sharedFoundation/NetworkIdArchive.h"
#include "sharedFoundation/NetworkId.h"
#include "sharedFoundation/Tag.h"
#include "unicodeArchive/UnicodeArchive.h"
#include "Unicode.h"
#include "AuctionQueryArchive.h"
#include "Archive/AutoDeltaByteStream.h"
#include "AuctionQueryTypes.h"
#include "sharedNetworkMessages/GameNetworkMessage.h"

//-----------------------------------------------------------------------

class AuctionQueryHeadersMessage : public GameNetworkMessage
{
public:

	AuctionQueryHeadersMessage(
		AuctionLocationSearch locationSearchType,
		const NetworkId & container,
		int requestId,
		AuctionSearchType searchType,
		int itemType,
		bool itemTypeExactMatch,
		int itemTemplateId,
		const Unicode::String & textFilterAll,
		const Unicode::String & textFilterAny,
		int priceFilterMin,
		int priceFilterMax,
		bool priceFilterIncludesFee,
		const std::list<SearchCondition> & advancedSearch,
		AdvancedSearchMatchAllAny advancedSearchMatchAllAny,
		bool myVendorsOnly = false,
		uint16 queryOffset = 0);
	explicit AuctionQueryHeadersMessage(Archive::ReadIterator &source);

	~AuctionQueryHeadersMessage();

public: // methods

	int                              getLocationSearchType() const;
	int                              getRequestId() const;
	int                              getSearchType() const;
	int                              getItemType() const;
	bool                             getItemTypeExactMatch() const;
	int                              getItemTemplateId() const;
	const Unicode::String &          getTextFilterAll() const;
	const Unicode::String &          getTextFilterAny() const;
	int                              getPriceFilterMin() const;
	int                              getPriceFilterMax() const;
	bool                             getPriceFilterIncludesFee() const;
	const std::list<SearchCondition> & getAdvancedSearch() const;
	AdvancedSearchMatchAllAny        getAdvancedSearchMatchAllAny() const;
	const NetworkId &                getContainer() const;
	bool                             getMyVendorsOnly() const;
	uint16                           getQueryOffset() const;

private: 
	Archive::AutoVariable<int>              m_locationSearchType;   //string to search by
	Archive::AutoVariable<int>              m_requestId;      //sequence number
	Archive::AutoVariable<int>              m_searchType;     //item category
	Archive::AutoVariable<int>              m_itemType;       //item category
	Archive::AutoVariable<bool>             m_itemTypeExactMatch;
	Archive::AutoVariable<int>              m_itemTemplateId; //if non-zero, specifies the template id of a particular item
	Archive::AutoVariable<Unicode::String>  m_textFilterAll;  //text filter for name
	Archive::AutoVariable<Unicode::String>  m_textFilterAny;
	Archive::AutoVariable<int>              m_priceFilterMin;
	Archive::AutoVariable<int>              m_priceFilterMax;
	Archive::AutoVariable<bool>             m_priceFilterIncludesFee;
	Archive::AutoList<SearchCondition>      m_advancedSearch;
	Archive::AutoVariable<int8>             m_advancedSearchMatchAllAny;
	Archive::AutoVariable<NetworkId>        m_container;      
	Archive::AutoVariable<bool>             m_myVendorsOnly;
	Archive::AutoVariable<uint16>           m_queryOffset;
};

// ----------------------------------------------------------------------

inline int AuctionQueryHeadersMessage::getLocationSearchType() const
{
	return m_locationSearchType.get();
}

// ----------------------------------------------------------------------

inline int AuctionQueryHeadersMessage::getRequestId() const
{
	return m_requestId.get();
}

// ----------------------------------------------------------------------

inline int AuctionQueryHeadersMessage::getSearchType() const
{
	return m_searchType.get();
}

// ----------------------------------------------------------------------

inline int AuctionQueryHeadersMessage::getItemType() const
{
	return m_itemType.get();
}

// ----------------------------------------------------------------------

inline bool AuctionQueryHeadersMessage::getItemTypeExactMatch() const
{
	return m_itemTypeExactMatch.get();
}

// ----------------------------------------------------------------------

inline int AuctionQueryHeadersMessage::getItemTemplateId() const
{
	return m_itemTemplateId.get();
}

// ----------------------------------------------------------------------

inline const Unicode::String & AuctionQueryHeadersMessage::getTextFilterAll() const
{
	return m_textFilterAll.get();
}

// ----------------------------------------------------------------------

inline const Unicode::String & AuctionQueryHeadersMessage::getTextFilterAny() const
{
	return m_textFilterAny.get();
}

// ----------------------------------------------------------------------

inline int AuctionQueryHeadersMessage::getPriceFilterMin() const
{
	return m_priceFilterMin.get();
}

// ----------------------------------------------------------------------

inline int AuctionQueryHeadersMessage::getPriceFilterMax() const
{
	return m_priceFilterMax.get();
}

// ----------------------------------------------------------------------

inline bool AuctionQueryHeadersMessage::getPriceFilterIncludesFee() const
{
	return m_priceFilterIncludesFee.get();
}

// ----------------------------------------------------------------------

inline const std::list<SearchCondition> & AuctionQueryHeadersMessage::getAdvancedSearch() const
{
	return m_advancedSearch.get();
}

// ----------------------------------------------------------------------

inline AdvancedSearchMatchAllAny AuctionQueryHeadersMessage::getAdvancedSearchMatchAllAny() const
{
	return static_cast<AdvancedSearchMatchAllAny>(m_advancedSearchMatchAllAny.get());
}

// ----------------------------------------------------------------------

inline const NetworkId & AuctionQueryHeadersMessage::getContainer() const
{
	return m_container.get();
}

// ----------------------------------------------------------------------

inline bool AuctionQueryHeadersMessage::getMyVendorsOnly() const
{
	return m_myVendorsOnly.get();
}

// ----------------------------------------------------------------------

inline uint16 AuctionQueryHeadersMessage::getQueryOffset() const
{
	return m_queryOffset.get();
}

// ======================================================================

#endif // _AuctionQueryHeadersMessage_H
