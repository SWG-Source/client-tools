
//======================================================================
//
// SwgCuiAuctionPaneTableModel.cpp
// copyright (c) 2002 Sony Online Entertainment
//
//======================================================================

#include "swgClientUserInterface/FirstSwgclientUserInterface.h"
#include "swgClientUserInterface/SwgCuiAuctionPaneTableModel.h"

#include "clientGame/AuctionManagerClient.h"
#include "clientGame/AuctionManagerClientData.h"
#include "clientGame/Game.h"
#include "clientUserInterface/CuiAuctionManager.h"
#include "clientUserInterface/CuiPreferences.h"
#include "clientUserInterface/CuiStringIdsAuction.h"
#include "clientUserInterface/CuiUtils.h"
#include "sharedFoundation/Clock.h"
#include "sharedFoundation/Timer.h"
#include "sharedGame/AuctionManager.h"
#include "sharedGame/GameObjectTypes.h"
#include "sharedGame/ResourceClassObject.h"
#include "sharedGame/SharedObjectTemplate.h"
#include "sharedGame/Universe.h"
#include "sharedObject/Object.h"

#include "UIImageStyle.h"
#include "UIManager.h"
#include "UITable.h"
#include "UIUtils.h"

#include <vector>
#include <set>

//======================================================================

namespace
{
	namespace ColumnInfo
	{
		enum Id
		{
			I_none,
			I_type,
			I_name,
			I_bid,
			I_price,
			I_myBid,
			I_highBidder,
			I_myProxy,
			I_time,
			I_saleType,
			I_location,
			I_owner,
			I_premium,
			I_distanceTo,
			I_entranceFee,
			I_count
		};

		struct ColumnData
		{
			StringId               name;
			UITableTypes::CellType cellType;

			ColumnData () :
				name (),
				cellType (UITableTypes::CT_none)
			{
			}

			ColumnData (const StringId & _name, UITableTypes::CellType _cellType) :
				name (_name),
				cellType (_cellType)
			{
			}
		};

		ColumnData data [I_count];

		//-- install the data
		void install ()
		{
			static bool installed = false;

			if (installed)
				return;

			installed = true;

			data [I_none]         = ColumnData (StringId ("NONE"),                        UITableTypes::CT_none);
			data [I_type]         = ColumnData (CuiStringIdsAuction::table_type,          UITableTypes::CT_text);
			data [I_name]         = ColumnData (CuiStringIdsAuction::table_name,          UITableTypes::CT_text);
			data [I_owner]        = ColumnData (CuiStringIdsAuction::table_owner,         UITableTypes::CT_text);
			data [I_premium]      = ColumnData (CuiStringIdsAuction::table_premium,       UITableTypes::CT_image);
			data [I_bid]          = ColumnData (CuiStringIdsAuction::table_bid,           UITableTypes::CT_delimitedInteger);
			data [I_price]        = ColumnData (CuiStringIdsAuction::table_price,         UITableTypes::CT_delimitedInteger);
			data [I_myBid]        = ColumnData (CuiStringIdsAuction::table_my_bid,        UITableTypes::CT_delimitedInteger);
			data [I_highBidder]   = ColumnData (CuiStringIdsAuction::table_high_bidder,   UITableTypes::CT_text);
			data [I_myProxy]      = ColumnData (CuiStringIdsAuction::table_my_proxy,      UITableTypes::CT_delimitedInteger);
			data [I_time]         = ColumnData (CuiStringIdsAuction::table_time,          UITableTypes::CT_text);
			data [I_saleType]     = ColumnData (CuiStringIdsAuction::table_sale_type,     UITableTypes::CT_text);
			data [I_location]     = ColumnData (CuiStringIdsAuction::table_location,      UITableTypes::CT_text);
			data [I_distanceTo]   = ColumnData (CuiStringIdsAuction::table_distance_to,   UITableTypes::CT_text);
			data [I_entranceFee]  = ColumnData (CuiStringIdsAuction::table_entrance_fee,  UITableTypes::CT_text);
		}

		//----------------------------------------------------------------------

		// SwgCuiAuctionPaneTableModel::ColumnInfo::All and CuiPreferences::ms_cscsAll *MUST* be kept in sync
		namespace All
		{
			const int numColumns = 7;
			const Id ids [numColumns] =
			{
				I_premium,
				I_name,
				I_type,
				I_bid,
				I_time,
				I_saleType,
				I_location
			};
		}

		// SwgCuiAuctionPaneTableModel::ColumnInfo::MySales and CuiPreferences::ms_cscsMySales *MUST* be kept in sync
		namespace MySales
		{
			const int numColumns = 7;
			const Id ids [numColumns] =
			{
				I_premium,
				I_name,
				I_type,
				I_bid,
				I_time,
				I_saleType,
				I_location
			};
		}

		// SwgCuiAuctionPaneTableModel::ColumnInfo::MyBids and CuiPreferences::ms_cscsMyBids *MUST* be kept in sync
		namespace MyBids
		{
			const int numColumns = 7;
			const Id ids [numColumns] =
			{
				I_name,
				I_type,
				I_bid,
				I_time,
				I_myBid,
				I_myProxy,
				I_location
			};
		}

		// SwgCuiAuctionPaneTableModel::ColumnInfo::Available and CuiPreferences::ms_cscsAvailable *MUST* be kept in sync
		namespace Available
		{
			const int numColumns = 4;
			const Id ids [numColumns] =
			{
				I_name,
				I_type,
				I_time,
				I_location
			};
		}

		// SwgCuiAuctionPaneTableModel::ColumnInfo::VendorSellerSelling and CuiPreferences::ms_cscsVendorSellerSelling *MUST* be kept in sync
		namespace VendorSellerSelling
		{
			const int numColumns = 5;
			const Id ids [numColumns] =
			{
				I_name,
				I_type,
				I_price,
				I_time,
				I_location
			};
		}

		// SwgCuiAuctionPaneTableModel::ColumnInfo::VendorBuyerSelling and CuiPreferences::ms_cscsVendorBuyerSelling *MUST* be kept in sync
		namespace VendorBuyerSelling
		{
			const int numColumns = 3;
			const Id ids [numColumns] =
			{
				I_name,
				I_type,
				I_price
			};
		}

		// SwgCuiAuctionPaneTableModel::ColumnInfo::VendorSellerOffers and CuiPreferences::ms_cscsVendorSellerOffers *MUST* be kept in sync
		namespace VendorSellerOffers
		{
			const int numColumns = 5;
			const Id ids [numColumns] =
			{
				I_name,
				I_type,
				I_price,
				I_owner,
				I_location
			};
		}

		// SwgCuiAuctionPaneTableModel::ColumnInfo::VendorBuyerOffers and CuiPreferences::ms_cscsVendorBuyerOffers *MUST* be kept in sync
		namespace VendorBuyerOffers
		{
			const int numColumns = 5;
			const Id ids [numColumns] =
			{
				I_name,
				I_type,
				I_price,
				I_time,
				I_owner //@todo must be status

			};
		}

		// SwgCuiAuctionPaneTableModel::ColumnInfo::VendorSellerStockroom and CuiPreferences::ms_cscsVendorSellerStockroom *MUST* be kept in sync
		namespace VendorSellerStockroom
		{
			const int numColumns = 5;
			const Id ids [numColumns] =
			{
				I_name,
				I_type,
				I_price,
				I_time,
				I_location
			};
		}

		// SwgCuiAuctionPaneTableModel::ColumnInfo::Location and CuiPreferences::ms_cscsLocation *MUST* be kept in sync
		namespace Location
		{
			const int numColumns = 6;
			const Id ids [numColumns] =
			{
				I_name,
				I_type,
				I_location,
				I_distanceTo,
				I_entranceFee,
				I_bid
			};
		}

		//----------------------------------------------------------------------

		Id findColumnId (SwgCuiAuctionPaneTableModel::Type type, int col)
		{
			DEBUG_FATAL (col < 0, ("bad col"));

			switch (type)
			{
			case SwgCuiAuctionPaneTypes::T_all:
				DEBUG_FATAL (col >= ColumnInfo::All::numColumns, ("bad col"));
				return ColumnInfo::All::ids [col];
			case SwgCuiAuctionPaneTypes::T_mySales:
				DEBUG_FATAL (col >= ColumnInfo::MySales::numColumns, ("bad col"));
				return ColumnInfo::MySales::ids [col];
			case SwgCuiAuctionPaneTypes::T_myBids:
				DEBUG_FATAL (col >= ColumnInfo::MyBids::numColumns, ("bad col"));
				return ColumnInfo::MyBids::ids [col];
			case SwgCuiAuctionPaneTypes::T_available:
				DEBUG_FATAL (col >= ColumnInfo::Available::numColumns, ("bad col"));
				return ColumnInfo::Available::ids [col];
			case SwgCuiAuctionPaneTypes::T_vendorSellerSelling:
				DEBUG_FATAL (col >= ColumnInfo::VendorSellerSelling::numColumns, ("bad col"));
				return ColumnInfo::VendorSellerSelling::ids [col];
			case SwgCuiAuctionPaneTypes::T_vendorBuyerSelling:
				DEBUG_FATAL (col >= ColumnInfo::VendorBuyerSelling::numColumns, ("bad col"));
				return ColumnInfo::VendorBuyerSelling::ids [col];
			case SwgCuiAuctionPaneTypes::T_vendorSellerOffers:
				DEBUG_FATAL (col >= ColumnInfo::VendorSellerOffers::numColumns, ("bad col"));
				return ColumnInfo::VendorSellerOffers::ids [col];
			case SwgCuiAuctionPaneTypes::T_vendorBuyerOffers:
				DEBUG_FATAL (col >= ColumnInfo::VendorBuyerOffers::numColumns, ("bad col"));
				return ColumnInfo::VendorBuyerOffers::ids [col];
			case SwgCuiAuctionPaneTypes::T_vendorSellerStockroom:
				DEBUG_FATAL (col >= ColumnInfo::VendorSellerStockroom::numColumns, ("bad col"));
				return ColumnInfo::VendorSellerStockroom::ids [col];
			case SwgCuiAuctionPaneTypes::T_location:
				DEBUG_FATAL (col >= ColumnInfo::Location::numColumns, ("bad col"));
				return ColumnInfo::Location::ids [col];
			case SwgCuiAuctionPaneTypes::T_numTypes:
			default:
				return I_none;
			}
		}
	}

	int            s_selfRefcount = 0;
	UIImageStyle * s_premiumImageStyle = 0;

	const Unicode::String s_retrievingStrings [] =
	{
		Unicode::narrowToWide ("\\#pcontrast1 Retrieving"),
		Unicode::narrowToWide ("\\#pcontrast1 Retrieving ."),
		Unicode::narrowToWide ("\\#pcontrast1 Retrieving . ."),
		Unicode::narrowToWide ("\\#pcontrast1 Retrieving . . ."),
	};

	const Unicode::String s_withdrawingStrings [] =
	{
		Unicode::narrowToWide ("\\#pcontrast1 Withdrawing"),
		Unicode::narrowToWide ("\\#pcontrast1 Withdrawing ."),
		Unicode::narrowToWide ("\\#pcontrast1 Withdrawing . ."),
		Unicode::narrowToWide ("\\#pcontrast1 Withdrawing . . ."),
	};

	bool findPositionOnSamePlanet(std::string const & location, int & x, int & z)
	{
		// this isn't the most efficient way to get this information...
		// it is the simplest that requires the least amount of
		// code being changed -- TMF
		std::string planet;
		std::string region;
		std::string name;
		NetworkId id;
		IGNORE_RETURN(AuctionManager::separateLocation(location, planet, region, name, id, x, z));

		bool isOnSamePlanet = Game::getNonInstanceSceneId() == planet;
		return isOnSamePlanet;
	}

	std::set<NetworkId> s_requestedObjIds;
	Timer s_updateTimer(2.0f);

	void requestDetails(NetworkId const & objId)
	{
		if (!AuctionManagerClient::hasDetails(objId)) 
		{
			if (s_requestedObjIds.find(objId) == s_requestedObjIds.end()) 
			{
				IGNORE_RETURN(AuctionManagerClient::requestDetails(objId));
				s_requestedObjIds.insert(objId);
			}
		}
	}
}

//----------------------------------------------------------------------

SwgCuiAuctionPaneTableModel::SwgCuiAuctionPaneTableModel (Type type) :
UITableModel   (),
m_type         (type),
m_data         (new DataVector),
m_displayCache (new DisplayCacheVector),
m_toolTips()
{
	ColumnInfo::install ();
	SetName ("SwgCuiAuctionPaneTableModel");

	if (!s_selfRefcount)
	{
		s_premiumImageStyle = safe_cast<UIImageStyle *>(UIManager::gUIManager ().GetObjectFromPath ("/styles.icon.misc.star", TUIImageStyle));
		s_premiumImageStyle->Attach (0);
	}

	++s_selfRefcount;
}

//----------------------------------------------------------------------

SwgCuiAuctionPaneTableModel::~SwgCuiAuctionPaneTableModel ()
{
	delete m_data;
	m_data = 0;

	delete m_displayCache;
	m_displayCache = 0;

	if (--s_selfRefcount <= 0)
	{
		if (s_premiumImageStyle)
		{
			s_premiumImageStyle->Detach (0);
			s_premiumImageStyle = 0;
		}
	}
}

//----------------------------------------------------------------------

bool SwgCuiAuctionPaneTableModel::GetValueAtText          (int row, int col, UIString     & value)  const
{
	const DisplayCache * cache = 0;
	const Data * const elem = findDataElement (row, cache);
	NOT_NULL (elem);
	NOT_NULL (cache);

	const ColumnInfo::Id id = ColumnInfo::findColumnId (m_type, col);

	// @todo we must do some sort of caching and localizing here!!!
	switch (id)
	{
	case ColumnInfo::I_type:
		{
			value = cache->typeName;
		}
		break;
	case ColumnInfo::I_name:
		{
			value = cache->itemName;
		}
		break;
	case ColumnInfo::I_owner:
		{
			value = cache->ownerName;
		}
		break;
	case ColumnInfo::I_highBidder:
		{
			value = cache->highBidder;
		}
		break;
	case ColumnInfo::I_time:
		{
			if (CuiAuctionManager::isWaitingForRetrieve (cache->id))
			{
				value = s_retrievingStrings [static_cast<int>(Game::getElapsedTime ()) % 4];
			}
			else if (CuiAuctionManager::isWaitingForWithdraw (cache->id))
			{
				value = s_withdrawingStrings [static_cast<int>(Game::getElapsedTime ()) % 4];
			}
			else
				value = cache->timeRemaining;
		}
		break;
	case ColumnInfo::I_location:
		{
			if (CuiAuctionManager::isWaitingForRetrieve (cache->id))
			{
				value = s_retrievingStrings [static_cast<int>(Game::getElapsedTime ()) % 4];
			}
			else if (CuiAuctionManager::isWaitingForWithdraw (cache->id))
			{
				value = s_withdrawingStrings [static_cast<int>(Game::getElapsedTime ()) % 4];
			}
			else
				value = cache->location;
		}
		break;
	case ColumnInfo::I_saleType:
		{
			if (CuiAuctionManager::isWaitingForRetrieve (cache->id))
			{
				value = s_retrievingStrings [static_cast<int>(Game::getElapsedTime ()) % 4];
			}
			else if (CuiAuctionManager::isWaitingForWithdraw (cache->id))
			{
				value = s_withdrawingStrings [static_cast<int>(Game::getElapsedTime ()) % 4];
			}
			else
			{
				if (elem->header.buyNowPrice > 0)
					value = CuiStringIdsAuction::sale_type_instant.localize ();
				else
					value = CuiStringIdsAuction::sale_type_auction.localize ();
			}
		}
		break;
	case ColumnInfo::I_distanceTo:
		{
			if (CuiAuctionManager::isWaitingForRetrieve (cache->id))
			{
				value = s_retrievingStrings [static_cast<int>(Game::getElapsedTime ()) % 4];
			}
			else if (CuiAuctionManager::isWaitingForWithdraw (cache->id))
			{
				value = s_withdrawingStrings [static_cast<int>(Game::getElapsedTime ()) % 4];
			}
			else if (cache->isOnSamePlanet)
			{
				Vector const & vendorPosition_w = cache->position_w;
				float distance = 0.0f;

				Object const * const player = Game::getPlayer();

				if (player != 0)
				{
					Vector const & playerPosition_w = player->getPosition_w();
					distance = playerPosition_w.magnitudeBetween(vendorPosition_w);
				}

				char buffer[32];
				snprintf(buffer, 32, "%8.0f", distance);
				UINarrowString position(buffer);
				value = UIUnicode::narrowToWide(position);
			}
			else
			{
				value = Unicode::emptyString;
			}
		}
		break;
	case ColumnInfo::I_entranceFee:
		{
			if (CuiAuctionManager::isWaitingForRetrieve (cache->id))
			{
				value = s_retrievingStrings [static_cast<int>(Game::getElapsedTime ()) % 4];
			}
			else if (CuiAuctionManager::isWaitingForWithdraw (cache->id))
			{
				value = s_withdrawingStrings [static_cast<int>(Game::getElapsedTime ()) % 4];
			}
			else
			{
				int entranceFee = cache->entranceFee;

				char buffer[32];
				snprintf(buffer, 32, "%8d", entranceFee);
				UINarrowString position(buffer);
				value = UIUtils::FormatDelimitedInteger( UIUnicode::narrowToWide(position));
			}
		}
		break;
	case ColumnInfo::I_none:
	case ColumnInfo::I_bid:
	case ColumnInfo::I_price:
	case ColumnInfo::I_myBid:
	case ColumnInfo::I_myProxy:
	case ColumnInfo::I_count:
	default:
		return false;
	}

	return true;
}

//----------------------------------------------------------------------

bool SwgCuiAuctionPaneTableModel::GetValueAtImage          (int row, int col, UIImageStyle *& value)  const
{
	const DisplayCache * cache = 0;
	const Data * const elem = findDataElement (row, cache);
	NOT_NULL (elem);
	NOT_NULL (cache);

	const ColumnInfo::Id id = ColumnInfo::findColumnId (m_type, col);

	// @todo we must do some sort of caching and localizing here!!!
	switch (id)
	{
	case ColumnInfo::I_premium:
		{
			if (elem->header.isPremium ())
				value = s_premiumImageStyle;
			else
				value = 0;
		}
		break;
	default:
		return false;
	}

	return true;
}
//----------------------------------------------------------------------

bool SwgCuiAuctionPaneTableModel::GetSortKeyAtString     (int row, int col, UITableTypes::CellType type, UIString & strValue)  const
{
	const DisplayCache * cache = 0;
	const Data * const elem = findDataElement (row, cache);
	NOT_NULL (elem);
	NOT_NULL (cache);

	const ColumnInfo::Id id = ColumnInfo::findColumnId (m_type, col);

	// @todo we must do some sort of caching and localizing here!!!
	switch (id)
	{

	case ColumnInfo::I_time:
		{
			static char buf [128];
			static const size_t buf_size = sizeof (buf);
			snprintf (buf, buf_size, "%012d", cache->last_timer);
			strValue = Unicode::narrowToWide (buf);
			return true;
		}
		break;
	}

	return UITableModel::GetSortKeyAtString (row, col, type, strValue);
}


//----------------------------------------------------------------------

bool SwgCuiAuctionPaneTableModel::GetValueAtInteger       (int row, int col, int & value)           const
{
	const DisplayCache * cache = 0;
	const Data * const elem = findDataElement (row, cache);
	NOT_NULL (elem);
	NOT_NULL (cache);

	const ColumnInfo::Id id = ColumnInfo::findColumnId (m_type, col);

	// @todo we must do some sort of caching and localizing here!!!
	switch (id)
	{
	case ColumnInfo::I_bid:
	case ColumnInfo::I_price:
		{
			value = elem->header.highBid;
		}
		break;
	case ColumnInfo::I_myBid:
		{
			value = elem->header.myBid;
		}
		break;
	case ColumnInfo::I_myProxy:
		{
			value = elem->header.maxProxyBid;
		}
		break;
	default:
		return false;
	}  //lint !e788

	return true;
}

//----------------------------------------------------------------------

bool SwgCuiAuctionPaneTableModel::GetLocalizedColumnName  (int col, UIString & str) const
{
	if (col < 0 || col >= GetColumnCount ())
		return false;

	using namespace ColumnInfo;

	switch (m_type)
	{
	case SwgCuiAuctionPaneTypes::T_all:
		str = data [All::ids [col]].name.localize ();
		break;
	case SwgCuiAuctionPaneTypes::T_myBids:
		str = data [MyBids::ids [col]].name.localize ();
		break;
	case SwgCuiAuctionPaneTypes::T_mySales:
		str = data [MySales::ids [col]].name.localize ();
		break;
	case SwgCuiAuctionPaneTypes::T_available:
		str = data [Available::ids [col]].name.localize ();
		break;
	case SwgCuiAuctionPaneTypes::T_vendorBuyerSelling:
		str = data [VendorBuyerSelling::ids [col]].name.localize ();
		break;
	case SwgCuiAuctionPaneTypes::T_vendorSellerSelling:
		str = data [VendorSellerSelling::ids [col]].name.localize ();
		break;
	case SwgCuiAuctionPaneTypes::T_vendorSellerOffers:
		str = data [VendorSellerOffers::ids [col]].name.localize ();
		break;
	case SwgCuiAuctionPaneTypes::T_vendorBuyerOffers:
		str = data [VendorBuyerOffers::ids [col]].name.localize ();
		break;
	case SwgCuiAuctionPaneTypes::T_vendorSellerStockroom:
		str = data [VendorSellerStockroom::ids [col]].name.localize ();
		break;
	case SwgCuiAuctionPaneTypes::T_location:
		str = data [Location::ids [col]].name.localize ();
		break;
	default:
		return false;
	} //lint !e788

	return true;
}

//----------------------------------------------------------------------

UITableTypes::CellType SwgCuiAuctionPaneTableModel::FindColumnCellType      (int col) const
{
	if (col < 0 || col >= FindColumnCount ())
		return UITableTypes::CT_none;

	using namespace ColumnInfo;

	switch (m_type)
	{
	case SwgCuiAuctionPaneTypes::T_all:
		return data [All::ids [col]].cellType;
	case SwgCuiAuctionPaneTypes::T_myBids:
		return data [MyBids::ids [col]].cellType;
	case SwgCuiAuctionPaneTypes::T_mySales:
		return data [MySales::ids [col]].cellType;
	case SwgCuiAuctionPaneTypes::T_available:
		return data [Available::ids [col]].cellType;

	case SwgCuiAuctionPaneTypes::T_vendorBuyerSelling:
		return data [VendorBuyerSelling::ids [col]].cellType;
	case SwgCuiAuctionPaneTypes::T_vendorSellerSelling:
		return data [VendorSellerSelling::ids [col]].cellType;
	case SwgCuiAuctionPaneTypes::T_vendorSellerOffers:
		return data [VendorSellerOffers::ids [col]].cellType;
	case SwgCuiAuctionPaneTypes::T_vendorBuyerOffers:
		return data [VendorBuyerOffers::ids [col]].cellType;
	case SwgCuiAuctionPaneTypes::T_vendorSellerStockroom:
		return data [VendorSellerStockroom::ids [col]].cellType;

	case SwgCuiAuctionPaneTypes::T_location:
		return data [Location::ids [col]].cellType;

	default:
		return UITableTypes::CT_none;
	}  //lint !e788
}

//----------------------------------------------------------------------

int SwgCuiAuctionPaneTableModel::FindColumnCount         () const
{
	switch (m_type)
	{
	case SwgCuiAuctionPaneTypes::T_all:
		return ColumnInfo::All::numColumns;
	case SwgCuiAuctionPaneTypes::T_myBids:
		return ColumnInfo::MyBids::numColumns;
	case SwgCuiAuctionPaneTypes::T_mySales:
		return ColumnInfo::MySales::numColumns;
	case SwgCuiAuctionPaneTypes::T_available:
		return ColumnInfo::Available::numColumns;
	case SwgCuiAuctionPaneTypes::T_vendorBuyerSelling:
		return ColumnInfo::VendorBuyerSelling::numColumns;
	case SwgCuiAuctionPaneTypes::T_vendorSellerSelling:
		return ColumnInfo::VendorSellerSelling::numColumns;
	case SwgCuiAuctionPaneTypes::T_vendorSellerOffers:
		return ColumnInfo::VendorSellerOffers::numColumns;
	case SwgCuiAuctionPaneTypes::T_vendorBuyerOffers:
		return ColumnInfo::VendorBuyerOffers::numColumns;
	case SwgCuiAuctionPaneTypes::T_vendorSellerStockroom:
		return ColumnInfo::VendorSellerStockroom::numColumns;
	case SwgCuiAuctionPaneTypes::T_location:
		return ColumnInfo::Location::numColumns;
	default:
		return 0;
	} //lint !e788
}

//----------------------------------------------------------------------

int  SwgCuiAuctionPaneTableModel::FindRowCount            () const
{
	return static_cast<int>(m_data->size ());
}

//----------------------------------------------------------------------

void SwgCuiAuctionPaneTableModel::refreshDisplayCache ()
{
	const DataVector & dv          = *m_data;
	DisplayCacheVector & dcv       = *m_displayCache;

	dcv.clear ();
	dcv.reserve (dv.size ());

	for (DataVector::const_iterator it = dv.begin (); it != dv.end (); ++it)
	{
		const Data & data = *it;
		DisplayCache dc;

		dc.id = data.header.itemId;

		ResourceClassObject const * rco = NULL;
		if ((GameObjectTypes::isTypeOf(data.header.itemType, SharedObjectTemplate::GOT_resource_container)) && (data.header.resourceContainerClassCrc != 0))
			rco = Universe::getInstance().getResourceClassByNameCrc(static_cast<uint32>(data.header.resourceContainerClassCrc));

		if (rco)
			dc.typeName = rco->getFriendlyName().localize();
		else
			dc.typeName = GameObjectTypes::getLocalizedName (static_cast<int>(data.header.itemType));
		
		data.constructLocalizedName (dc.itemName);

		dc.ownerName = Unicode::narrowToWide (data.header.ownerName);

		dc.highBidder    = Unicode::narrowToWide (data.header.highBidderName);
		const int secsRemaining = data.header.timer;
		dc.last_timer = secsRemaining;
		CuiUtils::FormatTimeDuration (dc.timeRemaining, static_cast<size_t>(secsRemaining), true, true, true, false, true);

		dc.location.clear ();
		if (!AuctionManagerClient::localizeLocation (data.header.location, dc.location, true, false))
			dc.location = Unicode::narrowToWide (data.header.location);

		int x = 0;
		int z = 0;
		bool const isOnSamePlanet = findPositionOnSamePlanet(data.header.location, x, z);

		Vector const position_w(static_cast<float>(x), 0.0f, static_cast<float>(z));

		dc.position_w = position_w;
		dc.entranceFee = data.header.entranceCharge;
		dc.isOnSamePlanet = isOnSamePlanet;


		dcv.push_back(dc);
	}
}

//----------------------------------------------------------------------

SwgCuiAuctionPaneTableModel::Data * SwgCuiAuctionPaneTableModel::findDataElement(const NetworkId & auctionId) const
{
	for (DataVector::iterator it = m_data->begin (); it != m_data->end (); ++it)
	{
		Data & data = *it;
		if (data.header.itemId == auctionId)
			return & data;
	}

	return 0;
}

//----------------------------------------------------------------------

SwgCuiAuctionPaneTableModel::DisplayCache * SwgCuiAuctionPaneTableModel::findDisplayCache(const NetworkId & auctionId) const 
{
	int const row = findVisualRowForAuction (auctionId);

	const DisplayCache * dc = 0;

	if (findDataElement (row, dc))
		return const_cast<DisplayCache *>(dc);

	return 0;
}

//----------------------------------------------------------------------

void SwgCuiAuctionPaneTableModel::updateData              (const DataVector & dv)
{
	bool displayCacheCorrupt = false;

	for (DataVector::const_iterator it = dv.begin (); it != dv.end (); ++it)
	{
		const Data & data    = *it;
		Data * const my_data = findDataElement (data.header.itemId);
		if (!my_data)
			WARNING (true, ("SwgCuiAuctionPaneTableModel no such data element for [%s]", data.header.itemId.getValueString ().c_str ()));
		else
		{
			my_data->header.timer = data.header.timer;

			DisplayCache * const displayCache = findDisplayCache (data.header.itemId);

			if (!displayCache)
			{
				WARNING (true, ("SwgCuiAuctionPaneTableModel display cache no element for [%s]", data.header.itemId.getValueString ().c_str ()));
				displayCacheCorrupt = true;
				break;
			}
			else
			{
				const int secsRemaining = data.header.timer;
				if ((displayCache->last_timer / 60) != (secsRemaining / 60))
				{
					displayCache->timeRemaining.clear ();
					CuiUtils::FormatTimeDuration (displayCache->timeRemaining, static_cast<size_t>(secsRemaining), true, true, true, false, true);
				}
			}
		}
	}


	if (displayCacheCorrupt)
		refreshDisplayCache ();

	if (s_updateTimer.updateZero(Clock::frameTime())) 
	{
		for(std::set<NetworkId>::iterator itId = s_requestedObjIds.begin(); itId != s_requestedObjIds.end(); /**/)
		{
			if (AuctionManagerClient::hasDetails(*itId)) 
			{
				s_requestedObjIds.erase(itId++);
			}
			else
			{
				++itId;
			}
		}
	}
}

//----------------------------------------------------------------------

void SwgCuiAuctionPaneTableModel::setData                 (const DataVector & dv)
{
	*m_data = dv;

	refreshDisplayCache ();
	fireColumnsChanged ();
	fireDataChanged ();
}

//----------------------------------------------------------------------

const NetworkId & SwgCuiAuctionPaneTableModel::getAuctionDataItemId    (int row) const
{
	if (row < 0 || row >= GetRowCount ())
		return NetworkId::cms_invalid;
	else
	{
		const DisplayCache * cache = 0;
		const Data * const elem = findDataElement (row, cache);

		NOT_NULL (elem);
		NOT_NULL (cache);

		UNREF (cache);

		return elem->header.itemId;
	}
}

//----------------------------------------------------------------------

const SwgCuiAuctionPaneTableModel::Data * SwgCuiAuctionPaneTableModel::findDataElement         (int visualRow, const DisplayCache *& cache) const
{
	if (visualRow < 0)
		return 0;

	const long row = GetLogicalDataRowIndex (visualRow);
	DEBUG_FATAL (row < 0 || static_cast<int>(m_data->size ()) <= row, ("bad row"));
	DEBUG_FATAL (static_cast<int>(m_displayCache->size ()) <= row, ("bad display cache row"));

	cache = &(*m_displayCache) [static_cast<size_t>(row)];
	return &(*m_data) [static_cast<size_t>(row)];
}

//----------------------------------------------------------------------

int SwgCuiAuctionPaneTableModel::findVisualRowForAuction (const NetworkId & itemId) const
{
	const int rowCount = GetRowCount ();

	for (int i = 0; i < rowCount; ++i)
	{
		const Data & data = (*m_data)[static_cast<size_t>(i)];

		if (data.header.itemId == itemId)
			return GetVisualDataRowIndex (i);
	}

	return -1;
}

//----------------------------------------------------------------------

void SwgCuiAuctionPaneTableModel::clearAll ()
{
	m_data->clear                 ();
	fireColumnsChanged ();
	fireDataChanged    ();
}

//----------------------------------------------------------------------

void SwgCuiAuctionPaneTableModel::updateTableColumnSizes (UITable & table)
{
	//-- copy vector
	UITable::ColumnSizeInfoVector csiv = table.GetColumnSizeInfo ();
	const int columnCount = FindColumnCount ();
	csiv.resize (columnCount);

	// check to see if we there are saved column widths and use those as the initial column width
	std::vector<int> savedColumnWidth;	
	for (int j = 0; j < columnCount; ++j)
		savedColumnWidth.push_back(CuiPreferences::getCommoditiesWindowColumnSize(static_cast<int>(m_type), j));

	int numToDivide = columnCount;
	int nameColumn = -1;
	int premiumColumn = -1;

	for (int i = 0; i < columnCount; ++i)
	{
		const ColumnInfo::Id id = ColumnInfo::findColumnId (m_type, i);
		UITable::ColumnSizeInfo & csi = csiv [i];

		switch (id)
		{
		case ColumnInfo::I_premium:
		case ColumnInfo::I_saleType:
		case ColumnInfo::I_bid:
		case ColumnInfo::I_price:
		case ColumnInfo::I_myBid:
		case ColumnInfo::I_myProxy:
		case ColumnInfo::I_distanceTo:
		case ColumnInfo::I_entranceFee:
		case ColumnInfo::I_count:
			if (id == ColumnInfo::I_premium)
				premiumColumn = i;
			csi.constant = true;
			csi.width    = ((savedColumnWidth[i] > 0) ? savedColumnWidth[i] : 64L);
			--numToDivide;
			break;
		case ColumnInfo::I_type:
		case ColumnInfo::I_highBidder:
		case ColumnInfo::I_owner:
		case ColumnInfo::I_time:
			csi.constant = true;
			csi.width    = ((savedColumnWidth[i] > 0) ? savedColumnWidth[i] : 96L);
			--numToDivide;
			break;
		case ColumnInfo::I_name:
			nameColumn = i;
			break;
		case ColumnInfo::I_location:
			// if there's a saved column width for location,
			// use it, and set location as fixed width
			if (savedColumnWidth[i] > 0)
			{
				csi.constant = true;
				csi.width    = savedColumnWidth[i];
				--numToDivide;
			}
			break;
		}

		if (numToDivide)
		{
			const float prop = 1.0f / static_cast<float>(numToDivide);

			for (UITable::ColumnSizeInfoVector::iterator it = csiv.begin (); it != csiv.end (); ++it)
			{
				UITable::ColumnSizeInfo & csi = *it;
				csi.proportion = prop;
			}
		}
	}

	fireDataChanged ();

	table.SetColumnSizeInfo (csiv);

	if (nameColumn >= 0)
		sortOnColumn (nameColumn , UITableModel::SD_up);

	if (premiumColumn >= 0)
		sortOnColumn (premiumColumn, UITableModel::SD_down);
}

//----------------------------------------------------------------------

void SwgCuiAuctionPaneTableModel::persistTableColumnSizes(UITable const & table) const
{
	static const UILowerString s_prop_columnsInitialized = UILowerString ("ColumnsInitialized");
	if (!table.HasProperty(s_prop_columnsInitialized))
		return;

	if ((m_type >= 0) && (m_type < SwgCuiAuctionPaneTypes::T_numTypes))
	{
		UITable::ColumnSizeInfoVector const & csiv = table.GetColumnSizeInfo();
		const int columnCount = FindColumnCount();
		if (columnCount == static_cast<int>(csiv.size()))
		{
			for (int i = 0; i < columnCount; ++i)
				CuiPreferences::setCommoditiesWindowColumnSize(static_cast<int>(m_type), i, csiv[i].width);
		}
	}
}

//----------------------------------------------------------------------

bool SwgCuiAuctionPaneTableModel::GetTooltipAt(int row, int /*column*/ , UIString & tooltip) const
{
	bool foundTooltip = false;

	NetworkId const & objID = getAuctionDataItemId(row);

	if (objID != NetworkId::cms_invalid) 
	{
		std::map<NetworkId, UIString>::const_iterator itToolTip = m_toolTips.find(objID);
		if (itToolTip != m_toolTips.end()) 
		{
			tooltip = itToolTip->second;
			foundTooltip = true;
		} 
		else
		{
			if (AuctionManagerClient::hasDetails(objID)) 
			{
				tooltip.clear();
				AuctionManagerClient::getTooltipsFromDetails(objID, tooltip);
				m_toolTips[objID] = tooltip;
				foundTooltip = true;
			}
			else
			{
				requestDetails(objID);


				// Request adjacent objects to prevent lag when browsing items.
				if (row > 0 && row < (GetRowCount() - 1))
				{
					NetworkId const & objIDA = getAuctionDataItemId(row - 1);
					requestDetails(objIDA);

					NetworkId const & objIDB = getAuctionDataItemId(row + 1);
					requestDetails(objIDB);
				}
			}
		}
	}


	if (!foundTooltip)
	{
		tooltip = CuiStringIdsAuction::waiting_details.localize();
	}

	return true;
}

//======================================================================
