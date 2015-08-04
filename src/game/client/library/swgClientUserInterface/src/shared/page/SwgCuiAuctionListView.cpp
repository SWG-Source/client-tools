//======================================================================
//
// SwgCuiAuctionListView.cpp
// copyright (c) 2002 Sony Online Entertainment
//
//======================================================================

#include "swgClientUserInterface/FirstSwgclientUserInterface.h"
#include "swgClientUserInterface/SwgCuiAuctionListView.h"

#include "UIDataSource.h"
#include "UIMessage.h"
#include "UITable.h"
#include "clientGame/AuctionManagerClient.h"
#include "clientGame/AuctionManagerClientData.h"
#include "clientUserInterface/CuiMessageBox.h"
#include "clientUserInterface/CuiSettings.h"
#include "clientUserInterface/CuiStringIdsAuction.h"
#include "clientUserInterface/CuiStringVariablesData.h"
#include "clientUserInterface/CuiStringVariablesManager.h"
#include "sharedGame/CommoditiesAdvancedSearchAttribute.h"
#include "sharedMessageDispatch/Transceiver.h"
#include "sharedNetworkMessages/AuctionErrorCodes.h"
#include "swgClientUserInterface/SwgCuiAuctionFilter.h"
#include "swgClientUserInterface/SwgCuiAuctionListPane.h"
#include "swgClientUserInterface/SwgCuiAuctionPaneTableModel.h"

//======================================================================

namespace SwgCuiAuctionListViewNamespace
{
	void saveSortSettings(UITableModel const & model, 
								std::string const & mediatorName,
								std::string const & columnName,
								std::string const & sortDirectionName);

	void loadSortSettings(UITableModel & model,
								std::string const & mediatorName,
								std::string const & columnName,
								std::string const & sortDirectionName);

	namespace Settings
	{
		const std::string auctionListViewSortColumn    = "auctionListViewSortColumn";
		const std::string auctionListViewSortSelection = "auctionListViewSortSelection";
	}
}

using namespace SwgCuiAuctionListViewNamespace;

//----------------------------------------------------------------------

void SwgCuiAuctionListViewNamespace::saveSortSettings(UITableModel const & model, std::string const & mediatorName,
														std::string const & columnName,
														std::string const & sortDirectionName)
{
	int column = 0;
	UITableModel::SortDirection direction;

	IGNORE_RETURN(model.getSortOnColumnState(column, direction));
	CuiSettings::saveInteger(mediatorName, columnName, column);
	CuiSettings::saveInteger(mediatorName, sortDirectionName, direction);
}

//----------------------------------------------------------------------

void SwgCuiAuctionListViewNamespace::loadSortSettings(UITableModel & model, std::string const & mediatorName,
														std::string const & columnName,
														std::string const & sortDirectionName)
{
	int column    = 0;
	int direction = 0;

	IGNORE_RETURN(CuiSettings::loadInteger(mediatorName, columnName, column));
	IGNORE_RETURN(CuiSettings::loadInteger(mediatorName, sortDirectionName, direction));

	model.sortOnColumn(column, static_cast<UITableModel::SortDirection>(direction));
}

//======================================================================

SwgCuiAuctionListView::SwgCuiAuctionListView (SwgCuiAuctionListPane & pane, UITable & table, SwgCuiAuctionFilter * filter, int type) :
UIEventCallback               (),
m_table                       (&table),
m_filter                      (filter),
m_type                        (type),
m_model                       (new TableModel (static_cast<SwgCuiAuctionPaneTypes::Type>(type))),
m_transceiverDoubleClick      (new Transceivers::DoubleClick),
m_transceiverSelectionChanged (new Transceivers::SelectionChanged),
m_transceiverContextRequest   (new Transceivers::ContextRequest),
m_active                      (false),
m_callback                    (new MessageDispatch::Callback),
m_elapsedTime                 (0.0f),
m_pane                        (&pane)
{
	//-- don't fetch the pane

	m_table->Attach (0);
	m_model->Attach (0);
	m_table->GetParent ()->AddChild (m_model);
	m_table->SetTableModel (m_model);
	if (m_filter)
		m_filter->fetch ();

	static const UILowerString s_prop_columnsInitialized = UILowerString ("ColumnsInitialized");

	if (!m_table->HasProperty (s_prop_columnsInitialized))
	{
		m_model->updateTableColumnSizes (*m_table);
		m_table->SetPropertyBoolean (s_prop_columnsInitialized, true);

		UIDataSource * const ds = new UIDataSource;
		m_table->SetColumnSizeDataSource (ds);
	}

	m_table->SetContextCapable (true, true);
}

//----------------------------------------------------------------------

SwgCuiAuctionListView::~SwgCuiAuctionListView ()
{
	setActive (false);

	m_model->persistTableColumnSizes (*m_table);
	m_model->Detach (0);
	m_model = 0;

	m_table->SetTableModel (0);
	m_table->Detach (0);
	m_table = 0;

	if (m_filter)
		m_filter->release ();
	m_filter = 0;

	delete m_transceiverDoubleClick;
	m_transceiverDoubleClick = 0;

	delete m_transceiverSelectionChanged;
	m_transceiverSelectionChanged = 0;

	delete m_transceiverContextRequest;
	m_transceiverContextRequest = 0;

	delete m_callback;
	m_callback = 0;

	m_pane = 0;
}

//----------------------------------------------------------------------

void SwgCuiAuctionListView::setData       (const DataVector & dv)
{
	static UITable::LongVector lv;
	lv.clear ();

	lv = m_table->GetSelectedRows ();

	const long lastSelectedRow = lv.empty () ? -1 : lv.back ();

	static NetworkIdVector niv;
	niv.clear ();
	niv.reserve (lv.size ());

	{
		for (UITable::LongVector::const_iterator it = lv.begin (); it != lv.end (); ++it)
		{
			const NetworkId & itemId = m_model->getAuctionDataItemId  (*it);
			if (itemId.isValid ())
				niv.push_back (itemId);
		}
	}

	m_model->setData (dv);
	m_table->SelectRow (-1);

	bool found = false;

	for (NetworkIdVector::const_iterator it = niv.begin (); it != niv.end (); ++it)
	{
		const int newRow = m_model->findVisualRowForAuction (*it);
		if (newRow >= 0)
		{
			m_table->AddRowSelection (newRow);
			found = true;
		}
	}

	if (!found)
		m_table->SelectRow (lastSelectedRow);
}

//----------------------------------------------------------------------

void SwgCuiAuctionListView::requestUpdate (int startingIndex)
{
	int              location = 3;
	int              got = 0;
	int              selectedObjectTemplateId = 0;
	bool             selectedObjectIsResourceContainer = false;
	bool             selectedObjectIsSpecialGeneralGot = false;
	Unicode::String  textFilterAll;
	Unicode::String  textFilterAny;
	int              priceFilterMin = 0;
	int              priceFilterMax = 0;
	bool             priceFilterIncludesFee = false;
	std::list<AuctionQueryHeadersMessage::SearchCondition> advancedSearch;
	AuctionQueryHeadersMessage::AdvancedSearchMatchAllAny advancedSearchMatchAllAny = AuctionQueryHeadersMessage::ASMAA_match_all;

	if (m_filter)
	{
		got                    = m_filter->getSelectedObjectType (selectedObjectTemplateId, selectedObjectIsResourceContainer, selectedObjectIsSpecialGeneralGot);
		location               = m_filter->getSelectedLocation ();
		textFilterAll          = Unicode::getTrim (m_filter->getTextFilterAll ());
		textFilterAny          = Unicode::getTrim (m_filter->getTextFilterAny ());
		priceFilterMin         = m_filter->getPriceFilterMin ();
		priceFilterMax         = m_filter->getPriceFilterMax ();
		priceFilterIncludesFee = m_filter->getPriceFilterIncludesEntranceFee ();

		if (CommoditiesAdvancedSearchAttribute::allowChildGameObjectTypeToInheritSearchAttribute(got) || selectedObjectIsSpecialGeneralGot || (selectedObjectTemplateId != 0))
		{
			advancedSearch = m_filter->getItemAttributeFilter(advancedSearchMatchAllAny);

			// server requires string used in attribute search to be lowercase
			if (!advancedSearch.empty())
			{
				for (std::list<AuctionQueryHeadersMessage::SearchCondition>::iterator iter = advancedSearch.begin(); iter != advancedSearch.end(); ++iter)
				{
					if (!iter->stringValue.empty())
						iter->stringValue = Unicode::toLower(iter->stringValue);
				}
			}
		}
	}

	// Check that all the search tokens meet the minimum length requirements
	Unicode::String textFilter[] = { textFilterAll, textFilterAny };
	for (unsigned i = 0; i < sizeof(textFilter) / sizeof(textFilter[0]); ++i)
	{
		Unicode::UnicodeStringVector textFilterTokens;
		if (Unicode::tokenize (textFilter[i], textFilterTokens))
		{
			Unicode::UnicodeStringVector::const_iterator j;
			for (j = textFilterTokens.begin (); j != textFilterTokens.end (); ++j)
			{
				if ( (*j).length() < AuctionManagerClient::getMinTextFilterTokenLength () )
				{
					CuiStringVariablesData data;
					data.digit_i = AuctionManagerClient::getMinTextFilterTokenLength ();

					Unicode::String msg;
					CuiStringVariablesManager::process (CuiStringIdsAuction::err_text_filter_token_too_small, data, msg);

					CuiMessageBox::createInfoBox (msg);

					return;
				}
			}
		}
	}

	// if searching for a resource container of a specific resource name,
	// use a special value for got to indicate to the commodities server
	// we are searching for a resource container of a specific resource name
	// (the crc of the resource name is in selectedObjectTemplateId
	if (selectedObjectIsResourceContainer)
		got = 0xffffffff;

	switch (m_type)
	{
	case SwgCuiAuctionPaneTypes::T_all:
		AuctionManagerClient::requestAuctions (location, got, selectedObjectIsSpecialGeneralGot, selectedObjectTemplateId, textFilterAll, textFilterAny, priceFilterMin, priceFilterMax, priceFilterIncludesFee, advancedSearch, advancedSearchMatchAllAny, startingIndex);
		break;
	case SwgCuiAuctionPaneTypes::T_mySales:
		AuctionManagerClient::requestSales (startingIndex);
		break;
	case SwgCuiAuctionPaneTypes::T_myBids:
		AuctionManagerClient::requestBids (startingIndex);
		break;
	case SwgCuiAuctionPaneTypes::T_available:
		AuctionManagerClient::requestAvailable (startingIndex);
		break;
	case SwgCuiAuctionPaneTypes::T_vendorSellerSelling:
		AuctionManagerClient::requestVendorSelling (true, location, got, selectedObjectIsSpecialGeneralGot, selectedObjectTemplateId, textFilterAll, textFilterAny, priceFilterMin, priceFilterMax, priceFilterIncludesFee, advancedSearch, advancedSearchMatchAllAny, startingIndex);
		break;
	case SwgCuiAuctionPaneTypes::T_vendorBuyerSelling:
		AuctionManagerClient::requestVendorSelling (false, location, got, selectedObjectIsSpecialGeneralGot, selectedObjectTemplateId, textFilterAll, textFilterAny, priceFilterMin, priceFilterMax, priceFilterIncludesFee, advancedSearch, advancedSearchMatchAllAny, startingIndex);
		break;
	case SwgCuiAuctionPaneTypes::T_vendorBuyerOffers:
		AuctionManagerClient::requestVendorOffers (true, location, got, selectedObjectIsSpecialGeneralGot, selectedObjectTemplateId, textFilterAll, textFilterAny, priceFilterMin, priceFilterMax, priceFilterIncludesFee, advancedSearch, advancedSearchMatchAllAny, startingIndex);
		break;
	case SwgCuiAuctionPaneTypes::T_vendorSellerOffers:
		AuctionManagerClient::requestVendorOffers (false, location, got, selectedObjectIsSpecialGeneralGot, selectedObjectTemplateId, textFilterAll, textFilterAny, priceFilterMin, priceFilterMax, priceFilterIncludesFee, advancedSearch, advancedSearchMatchAllAny, startingIndex);
		break;
	case SwgCuiAuctionPaneTypes::T_vendorSellerStockroom:
		AuctionManagerClient::requestVendorStockroom (location, got, selectedObjectIsSpecialGeneralGot, selectedObjectTemplateId, textFilterAll, textFilterAny, priceFilterMin, priceFilterMax, priceFilterIncludesFee, advancedSearch, advancedSearchMatchAllAny, startingIndex);
		break;
	case SwgCuiAuctionPaneTypes::T_location:
		IGNORE_RETURN(AuctionManagerClient::requestLocations (location, got, selectedObjectIsSpecialGeneralGot, selectedObjectTemplateId, textFilterAll, textFilterAny, priceFilterMin, priceFilterMax, priceFilterIncludesFee, advancedSearch, advancedSearchMatchAllAny, startingIndex));
		break;
	default:
		break;
	}

	if (m_filter)
		m_filter->checkpointTextPriceFilterValues();

	m_pane->onListViewUpdated ();
}

//----------------------------------------------------------------------

bool SwgCuiAuctionListView::OnMessage(UIWidget * context, const UIMessage & msg)
{
	if (context == m_table)
	{
		if (msg.Type == UIMessage::LeftMouseDoubleClick)
		{
			if (m_table->GetCellFromPoint  (msg.MouseCoords, 0, 0))
				m_transceiverDoubleClick->emitMessage (*this);
			return false;
		}

		else if (msg.Type == UIMessage::ContextRequest)
		{
			long visualRow = 0;
			if (m_table->GetCellFromPoint  (msg.MouseCoords, &visualRow, 0))
			{
				if (visualRow < 0)
					return true;

				const NetworkId & auctionId = m_model->getAuctionDataItemId (visualRow);

				if (auctionId.isValid ())
				{
					m_transceiverContextRequest->emitMessage (std::make_pair (this, std::make_pair (auctionId, context->GetWorldLocation () + msg.MouseCoords)));
				}
			}
		}
	}

	return true;
}

//----------------------------------------------------------------------

void SwgCuiAuctionListView::OnGenericSelectionChanged (UIWidget *widget)
{
	if (widget == m_table)
		m_transceiverSelectionChanged->emitMessage (*this);
}

//----------------------------------------------------------------------

void SwgCuiAuctionListView::setActive     (bool b)
{
	if ((m_active && !b) || (!m_active && b))
	{
		m_active = b;

		if (m_active)
		{
			switch (m_type)
			{
			case SwgCuiAuctionPaneTypes::T_all:
				m_callback->connect (*this, &SwgCuiAuctionListView::onListChanged,          static_cast<AuctionManagerClient::Messages::ListAllChanged*> (0));
				m_callback->connect (*this, &SwgCuiAuctionListView::onCreateAuction,        static_cast<AuctionManagerClient::Messages::CreateAuctionResponse*> (0));
				break;
			case SwgCuiAuctionPaneTypes::T_mySales:
				m_callback->connect (*this, &SwgCuiAuctionListView::onListChanged,          static_cast<AuctionManagerClient::Messages::ListSalesChanged*> (0));
				m_callback->connect (*this, &SwgCuiAuctionListView::onCreateAuction,        static_cast<AuctionManagerClient::Messages::CreateAuctionResponse*> (0));
				break;
			case SwgCuiAuctionPaneTypes::T_myBids:
				m_callback->connect (*this, &SwgCuiAuctionListView::onListChanged,          static_cast<AuctionManagerClient::Messages::ListBidsChanged*> (0));
				m_callback->connect (*this, &SwgCuiAuctionListView::onGenericResponse,      static_cast<AuctionManagerClient::Messages::BidResponse*> (0));
				break;
			case SwgCuiAuctionPaneTypes::T_available:
				m_callback->connect (*this, &SwgCuiAuctionListView::onListChanged,          static_cast<AuctionManagerClient::Messages::ListAvailableChanged*> (0));
				m_callback->connect (*this, &SwgCuiAuctionListView::onGenericResponse,      static_cast<AuctionManagerClient::Messages::BidResponse*> (0));
				m_callback->connect (*this, &SwgCuiAuctionListView::onGenericResponse,      static_cast<AuctionManagerClient::Messages::CancelLiveAuctionResponse*> (0));
				m_callback->connect (*this, &SwgCuiAuctionListView::onItemRetrieve,			static_cast<AuctionManagerClient::Messages::Retrieved*> (0));
				break;
			case SwgCuiAuctionPaneTypes::T_vendorSellerSelling:
			case SwgCuiAuctionPaneTypes::T_vendorBuyerSelling:
				m_callback->connect (*this, &SwgCuiAuctionListView::onListChanged,          static_cast<AuctionManagerClient::Messages::ListVendorSellingChanged*> (0));
				m_callback->connect (*this, &SwgCuiAuctionListView::onCreateAuction,        static_cast<AuctionManagerClient::Messages::CreateAuctionResponse*> (0));
				break;
			case SwgCuiAuctionPaneTypes::T_vendorSellerOffers:
			case SwgCuiAuctionPaneTypes::T_vendorBuyerOffers:
				m_callback->connect (*this, &SwgCuiAuctionListView::onListChanged,          static_cast<AuctionManagerClient::Messages::ListVendorOffersChanged*> (0));
				m_callback->connect (*this, &SwgCuiAuctionListView::onItemRetrieve,			static_cast<AuctionManagerClient::Messages::Retrieved*> (0));
				m_callback->connect (*this, &SwgCuiAuctionListView::onGenericResponse,      static_cast<AuctionManagerClient::Messages::BidResponse*> (0));
				break;
			case SwgCuiAuctionPaneTypes::T_vendorSellerStockroom:
				m_callback->connect (*this, &SwgCuiAuctionListView::onListChanged,          static_cast<AuctionManagerClient::Messages::ListVendorStockroomChanged*> (0));
				m_callback->connect (*this, &SwgCuiAuctionListView::onCreateAuction,        static_cast<AuctionManagerClient::Messages::CreateAuctionResponse*> (0));
				m_callback->connect (*this, &SwgCuiAuctionListView::onGenericResponse,      static_cast<AuctionManagerClient::Messages::CancelLiveAuctionResponse*> (0));
				m_callback->connect (*this, &SwgCuiAuctionListView::onItemRetrieve,			static_cast<AuctionManagerClient::Messages::Retrieved*> (0));
				m_callback->connect (*this, &SwgCuiAuctionListView::onGenericResponse,      static_cast<AuctionManagerClient::Messages::BidResponse*> (0));
				break;
			case SwgCuiAuctionPaneTypes::T_location:
				m_callback->connect (*this, &SwgCuiAuctionListView::onListChanged,          static_cast<AuctionManagerClient::Messages::ListVendorLocationChanged*> (0));
				break;
			default:
				DEBUG_FATAL (true, ("not supported"));
				break;
			}

			if (m_filter)
				m_callback->connect (m_filter->getTransceiverChanged (), *this, &SwgCuiAuctionListView::onFilterChanged);

			m_table->AddCallback    (this);
			refreshData   ();
//			requestUpdate ();
		}
		else
		{
			m_table->RemoveCallback (this);

			m_callback->disconnect (*this, &SwgCuiAuctionListView::onListChanged,                static_cast<AuctionManagerClient::Messages::ListAllChanged*> (0));
			m_callback->disconnect (*this, &SwgCuiAuctionListView::onListChanged,                static_cast<AuctionManagerClient::Messages::ListSalesChanged*> (0));
			m_callback->disconnect (*this, &SwgCuiAuctionListView::onListChanged,                static_cast<AuctionManagerClient::Messages::ListBidsChanged*> (0));
			m_callback->disconnect (*this, &SwgCuiAuctionListView::onListChanged,                static_cast<AuctionManagerClient::Messages::ListAvailableChanged*> (0));
			m_callback->disconnect (*this, &SwgCuiAuctionListView::onListChanged,                static_cast<AuctionManagerClient::Messages::ListVendorSellingChanged*> (0));
			m_callback->disconnect (*this, &SwgCuiAuctionListView::onListChanged,                static_cast<AuctionManagerClient::Messages::ListVendorOffersChanged*> (0));
			m_callback->disconnect (*this, &SwgCuiAuctionListView::onListChanged,                static_cast<AuctionManagerClient::Messages::ListVendorStockroomChanged*> (0));
			m_callback->disconnect (*this, &SwgCuiAuctionListView::onListChanged,                static_cast<AuctionManagerClient::Messages::ListAllChanged*> (0));
			m_callback->disconnect (*this, &SwgCuiAuctionListView::onListChanged,                static_cast<AuctionManagerClient::Messages::ListVendorLocationChanged*> (0));
			m_callback->disconnect (*this, &SwgCuiAuctionListView::onCreateAuction,         static_cast<AuctionManagerClient::Messages::CreateAuctionResponse*> (0));
			m_callback->disconnect (*this, &SwgCuiAuctionListView::onGenericResponse,       static_cast<AuctionManagerClient::Messages::BidResponse*> (0));
			m_callback->disconnect (*this, &SwgCuiAuctionListView::onGenericResponse,       static_cast<AuctionManagerClient::Messages::CancelLiveAuctionResponse*> (0));
			m_callback->disconnect (*this, &SwgCuiAuctionListView::onGenericResponse,       static_cast<AuctionManagerClient::Messages::AcceptBidResponse*> (0));
			m_callback->disconnect (*this, &SwgCuiAuctionListView::onItemRetrieve,			static_cast<AuctionManagerClient::Messages::Retrieved*> (0));

			if (m_filter)
				m_callback->disconnect (m_filter->getTransceiverChanged (), *this, &SwgCuiAuctionListView::onFilterChanged);
		}
	}
}

//----------------------------------------------------------------------

void SwgCuiAuctionListView::getDataVector (DataVector & dv)
{
	switch (m_type)
	{
	case SwgCuiAuctionPaneTypes::T_all:
		AuctionManagerClient::getAuctions (AuctionManagerClient::T_all, dv);
		break;
	case SwgCuiAuctionPaneTypes::T_mySales:
		AuctionManagerClient::getAuctions (AuctionManagerClient::T_mySales, dv);
		break;
	case SwgCuiAuctionPaneTypes::T_myBids:
		AuctionManagerClient::getAuctions (AuctionManagerClient::T_myBids, dv);
		break;
	case SwgCuiAuctionPaneTypes::T_available:
		AuctionManagerClient::getAuctions (AuctionManagerClient::T_available, dv);
		break;
	case SwgCuiAuctionPaneTypes::T_vendorSellerSelling:
	case SwgCuiAuctionPaneTypes::T_vendorBuyerSelling:
		AuctionManagerClient::getAuctions (AuctionManagerClient::T_vendorSelling, dv);
		break;
	case SwgCuiAuctionPaneTypes::T_vendorSellerOffers:
	case SwgCuiAuctionPaneTypes::T_vendorBuyerOffers:
		AuctionManagerClient::getAuctions (AuctionManagerClient::T_vendorOffers, dv);
		break;
	case SwgCuiAuctionPaneTypes::T_vendorSellerStockroom:
		AuctionManagerClient::getAuctions (AuctionManagerClient::T_vendorStockroom, dv);
		break;
	case SwgCuiAuctionPaneTypes::T_location:
		AuctionManagerClient::getAuctions (AuctionManagerClient::T_location, dv);
		break;
	default:
		DEBUG_FATAL (true, ("not supported"));
		break;
	}
} //lint !e1762 (can be const, no it can't)

//----------------------------------------------------------------------

void SwgCuiAuctionListView::refreshData   ()
{
	DataVector dv;
	getDataVector (dv);
	setData (dv);
}

//----------------------------------------------------------------------

const NetworkId & SwgCuiAuctionListView::findSelectedAuction (bool warn) const
{
	const long visualRow = m_table->GetLastSelectedRow ();
	if (visualRow < 0)
	{
		if (warn)
			CuiMessageBox::createInfoBox (CuiStringIdsAuction::err_no_item_selected.localize ());
		return NetworkId::cms_invalid;
	}

	const NetworkId & auctionId = m_model->getAuctionDataItemId (visualRow);

	if (!auctionId.isValid ())
	{
		if (warn)
			CuiMessageBox::createInfoBox (CuiStringIdsAuction::err_selected_item_invalid.localize ());
		return NetworkId::cms_invalid;
	}

	return auctionId;
}

//----------------------------------------------------------------------

const SwgCuiAuctionListView::NetworkIdVector & SwgCuiAuctionListView::findSelectedAuctions      (bool warn) const
{
	static NetworkIdVector result;
	result.clear ();

	const UITable::LongVector & selRows = m_table->GetSelectedRows ();

	if (selRows.empty ())
	{
		if (warn)
			CuiMessageBox::createInfoBox (CuiStringIdsAuction::err_no_item_selected.localize ());

		return result;
	}

	int badIdCount = 0;

	result.reserve (selRows.size ());

	for (UITable::LongVector::const_iterator it = selRows.begin (); it != selRows.end (); ++it)
	{
		const NetworkId & id = m_model->getAuctionDataItemId (*it);

		if (!id.isValid ())
			++badIdCount;
		else
			result.push_back (id);
	}

	if (badIdCount)
	{
		if (warn)
			CuiMessageBox::createInfoBox (CuiStringIdsAuction::err_selected_item_invalid.localize ());
	}

	return result;
}

//----------------------------------------------------------------------

void SwgCuiAuctionListView::traverseAuctionToView (const bool & forward)
{
	const NetworkId & itemId = AuctionManagerClient::getAuctionToView ();
	int row = m_model->findVisualRowForAuction (itemId);

	if (row < 0)
		row = 0;
	else
	{
		if (forward)
			++row;
		else
			--row;

		if (row < 0)
			row = m_model->GetRowCount () -1;
		else if (row >= m_model->GetRowCount ())
			row = 0;
	}

	m_table->SelectRow (row);
}

//----------------------------------------------------------------------

void SwgCuiAuctionListView::onListChanged            (const uint8 & sequence)
{
	UNREF (sequence);
	refreshData ();
	m_pane->onListViewUpdated ();
}


//----------------------------------------------------------------------

void SwgCuiAuctionListView::onFilterChanged (const SwgCuiAuctionFilter & filter)
{
	if (&filter == m_filter)
	{
		requestUpdate (0);
	}
}

//----------------------------------------------------------------------

void SwgCuiAuctionListView::onCreateAuction (const std::pair<NetworkId, ResultInfo> & result)
{
	if (result.second.first == 0)
	{
		// for the vendor stockroom, don't send another update request
		// if we already have a pending update request, because we could
		// be relisting multiple items, and we'll get create auction
		// response for each of the item very quickly, and we don't
		// really need to send an update request for each of the create
		// auction response
		if ((m_type == SwgCuiAuctionPaneTypes::T_vendorSellerStockroom) && (AuctionManagerClient::isListRequestOutstanding(AuctionManagerClient::T_vendorStockroom)))
			return;

		requestUpdate (-1);
	}
}

void SwgCuiAuctionListView::onGenericResponse (const std::pair<NetworkId, ResultInfo> & result)
{
	if (result.second.first == 0)
		requestUpdate (-1);
}

void SwgCuiAuctionListView::onItemRetrieve(const std::pair<NetworkId, ResultInfo> & result)
{
	if( result.second.first == ar_OK || result.second.first == ar_ITEM_NOLONGER_EXISTS || result.second.first == ar_INVALID_ITEM_REIMBURSAL )
		requestUpdate (-1);
}

//----------------------------------------------------------------------

void SwgCuiAuctionListView::update (float deltaTimeSecs)
{
	m_elapsedTime += deltaTimeSecs;

	if (m_elapsedTime >= 1.0f)
	{
		DataVector dv;
		getDataVector (dv);
		m_model->updateData (dv);
	}
}

//----------------------------------------------------------------------

void SwgCuiAuctionListView::clearAllSelection()
{
	m_table->SelectRow(-1);
}

//----------------------------------------------------------------------

/** Note, this is is NOT derived from CuiMediator, and so should not call it's saveSettings call
*/
void SwgCuiAuctionListView::saveSettings (int const paneType) const
{
	if(m_model)
	{
		char buffer[10];
		_itoa(paneType, buffer, 10);
		std::string name = m_pane->getMediatorDebugName() + "_" + buffer;
		saveSortSettings(*m_model, name, Settings::auctionListViewSortColumn, Settings::auctionListViewSortSelection);
	}
}

//----------------------------------------------------------------------

/** Note, this is is NOT derived from CuiMediator, and so should not call it's loadSettings call
*/
void SwgCuiAuctionListView::loadSettings (int const paneType)
{
	if(m_model)
	{
		char buffer[10];
		_itoa(paneType, buffer, 10);
		std::string name = m_pane->getMediatorDebugName() + "_" + buffer;
		loadSortSettings(*m_model, name, Settings::auctionListViewSortColumn, Settings::auctionListViewSortSelection);
	}
}

//======================================================================
