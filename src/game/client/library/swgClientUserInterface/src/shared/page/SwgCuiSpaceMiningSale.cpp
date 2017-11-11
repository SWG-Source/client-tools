//=============================================================================
//
// SwgCuiSpaceMiningSale.cpp
// copyright (c) 2004 Sony Online Entertainment
//
//=============================================================================

#include "swgClientUserInterface/FirstSwgClientUserInterface.h"
#include "swgClientUserInterface/SwgCuiSpaceMiningSale.h"

#include "UIButton.h"
#include "UIComposite.h"
#include "UIData.h"
#include "UISliderbar.h"
#include "UITable.h"
#include "UITableModelDefault.h"
#include "UIText.h"
#include "UIUtils.h"
#include "clientGame/Game.h"
#include "clientGame/ResourceIconManager.h"
#include "clientGame/ResourceIconManager.h"
#include "clientGame/ResourceTypeManager.h"
#include "clientGame/ShipObject.h"
#include "clientUserInterface/CuiIconManager.h"
#include "clientUserInterface/CuiManager.h"
#include "clientUserInterface/CuiWidget3dObjectListViewer.h"
#include "sharedFoundation/GameControllerMessage.h"
#include "sharedGame/ResourceClassObject.h"
#include "sharedGame/Universe.h"
#include "sharedUtility/DataTable.h"
#include "sharedUtility/DataTableManager.h"
#include "sharedObject/NetworkIdManager.h"
#include "sharedMessageDispatch/Transceiver.h"
#include <map>
#include "sharedNetworkMessages/MessageQueueSpaceMiningSellResource.h"
#include "sharedObject/Controller.h"

//----------------------------------------------------------------------

namespace SwgCuiSpaceMiningSaleNamespace
{
	std::string const s_tablePrefix = "datatables/space_mining/station_price_list/";
	std::string const s_tableSuffix = "_price_list.iff";
	
	bool getPrices(std::string const & spaceStationName, SwgCuiSpaceMiningSale::StringIntMap & result)
	{
		result.clear();
		std::string const & filename = s_tablePrefix + spaceStationName + s_tableSuffix;

		DataTable const * const dt = DataTableManager::getTable(filename, true);

		if (NULL == dt)
		{
			WARNING(true, ("SwgCuiSpaceMiningSale unable to load price list for station from [%s]", filename.c_str()));
			return false;
		}

		int const numRows = dt->getNumRows();

		{
			for (int row = 0; row < numRows; ++row)
			{
				std::string const & resourceClassName = dt->getStringValue(0, row);
				int const pricePerUnit = dt->getIntValue(1, row);

				ResourceClassObject const * const resourceClassObject = Universe::getInstance ().getResourceClassByName (resourceClassName);;

				if (NULL == resourceClassObject)
				{
					WARNING(true, ("SwgCuiSpaceMiningSale [%s] specifies invalid resource class [%s]", filename.c_str(), resourceClassName.c_str()));
					continue;
				}

				result.insert(std::make_pair(resourceClassName, pricePerUnit));
			}
		}

		return true;
	}

	//----------------------------------------------------------------------

	void generateSimulation()
	{
		ShipObject * const ship = Game::getPlayerContainingShip();
		if (NULL == ship)
			return;

		ShipObject::NetworkIdIntMap cargoHoldContents;

		cargoHoldContents[NetworkId("1444")] = 50;
		cargoHoldContents[NetworkId("1445")] = 150;
		cargoHoldContents[NetworkId("1446")] = 1500;
		cargoHoldContents[NetworkId("1447")] = 800;
		cargoHoldContents[NetworkId("1448")] = 400;

		ship->clientSetCargoHoldContents(cargoHoldContents);

		ResourceTypeManager::setTypeInfo(NetworkId("1444"), Unicode::narrowToWide("Test Resource 1444"), "space_metal_ice");
		ResourceTypeManager::setTypeInfo(NetworkId("1445"), Unicode::narrowToWide("Test Resource 1445"), "space_gem_diamond");
		ResourceTypeManager::setTypeInfo(NetworkId("1446"), Unicode::narrowToWide("Test Resource 1446"), "space_chemical_acid");
		ResourceTypeManager::setTypeInfo(NetworkId("1447"), Unicode::narrowToWide("Test Resource 1447"), "space_chemical_organometallic");
		ResourceTypeManager::setTypeInfo(NetworkId("1448"), Unicode::narrowToWide("Test Resource 1448"), "gas_inert_malium");
	}
	
	//----------------------------------------------------------------------

	/*
	
	  UI Table Columns for buying table:
		Icon (widget), class name (string), ppu (int), amount in hold (int)

	  UI Table Columns for non buying table:
		Icon (widget), class name (string), amount in hold (int)

	*/
	//----------------------------------------------------------------------


};

using namespace SwgCuiSpaceMiningSaleNamespace;

//----------------------------------------------------------------------

SwgCuiSpaceMiningSale::SwgCuiSpaceMiningSale(UIPage & page) :
CuiMediator("SwgCuiSpaceMiningSale", page),
UIEventCallback(),
m_buttonSell(NULL),
m_textCaption(NULL),
m_sliderSelling(NULL),
m_tableWillPurchase(NULL),
m_tableWillNotPurchase(NULL),
m_textInfoSelling(NULL),
m_textPriceSelling(NULL),
m_textPriceUnits(NULL),
m_pageWidgets(NULL),
m_spaceStationId(),
m_spaceStationName(),
m_resourceClassPrices(new StringIntMap),
m_sellingResourceId(),
m_sellingPricePerUnit(0),
m_callback(new MessageDispatch::Callback),
m_dirty(false)
{
	setState (MS_closeable);

	getCodeDataObject(TUIButton, m_buttonSell, "buttonSell");
	getCodeDataObject(TUIText, m_textCaption, "textCaption");
	getCodeDataObject(TUISliderbar, m_sliderSelling, "sliderSelling");
	getCodeDataObject(TUITable, m_tableWillPurchase, "tableWillPurchase");
	getCodeDataObject(TUITable, m_tableWillNotPurchase, "tableWillNotPurchase");
	getCodeDataObject(TUIText, m_textInfoSelling, "textInfoSelling");
	getCodeDataObject(TUIText, m_textPriceSelling, "textPriceSelling");
	getCodeDataObject(TUIText, m_textPriceUnits, "textPriceUnits");
	getCodeDataObject(TUIPage, m_pageSelling, "pageSelling");

	m_pageWidgets = new UIPage;
	getPage().AddChild(m_pageWidgets);
	m_pageWidgets->SetVisible(false);

	m_sliderSelling->SetLowerLimit(1);

	m_textInfoSelling->SetPreLocalized(true);
	m_textPriceSelling->SetPreLocalized(true);
	m_textPriceUnits->SetPreLocalized(true);
	m_textCaption->SetPreLocalized(true);

	if (Game::getSinglePlayer())
	{
		generateSimulation();
		setStation(Game::getPlayer()->getNetworkId(), "tatooine");
	}
} 

//----------------------------------------------------------------------

SwgCuiSpaceMiningSale::~SwgCuiSpaceMiningSale()
{
	delete m_resourceClassPrices;
	delete m_callback;
}

//----------------------------------------------------------------------

void SwgCuiSpaceMiningSale::performActivate()
{
	CuiMediator::performActivate();
	CuiManager::requestPointer (true);

	reset();

	m_sliderSelling->AddCallback(this);
	m_tableWillPurchase->AddCallback(this);
	m_buttonSell->AddCallback(this);

	m_callback->connect(*this, &SwgCuiSpaceMiningSale::onShipCargoChanged, static_cast<ShipObject::Messages::CargoChanged*> (0));

	setIsUpdating(true);
}

//----------------------------------------------------------------------

void SwgCuiSpaceMiningSale::performDeactivate()
{
	CuiMediator::performDeactivate();
	CuiManager::requestPointer (false);

	m_callback->disconnect(*this, &SwgCuiSpaceMiningSale::onShipCargoChanged, static_cast<ShipObject::Messages::CargoChanged*> (0));
	setIsUpdating(false);
}

//----------------------------------------------------------------------

void SwgCuiSpaceMiningSale::setStation(NetworkId const & id, std::string const & spaceStationName)
{
	m_spaceStationId = id;
	m_spaceStationName = spaceStationName;

	getPrices(spaceStationName, *m_resourceClassPrices);

	if (isActive())
		reset();
}

//----------------------------------------------------------------------

void SwgCuiSpaceMiningSale::update(float /*updateDeltaSeconds*/)
{
	if (m_dirty)
		reset();
}

//----------------------------------------------------------------------

void SwgCuiSpaceMiningSale::reset()
{ 
	m_dirty = false;

	ShipObject const * const ship = Game::getPlayerContainingShip();
	if (NULL == ship)
		return;

	Object const * const station = NetworkIdManager::getObjectById(m_spaceStationId);
	ClientObject const * const clientStation = station ? station->asClientObject() : NULL;

	if (NULL == clientStation)
		WARNING(true, ("SwgCuiSpaceMiningSale invalid station id [%s]", m_spaceStationId));
	else
		m_textCaption->SetLocalText(Unicode::narrowToWide("Space Mining Sales - ") + clientStation->getLocalizedName());

	int const oldSelectedRow = std::max(long(0), m_tableWillPurchase->GetLastSelectedRow());

	UITableModelDefault * const tableModelWillPurchase = NON_NULL(safe_cast<UITableModelDefault *>(m_tableWillPurchase->GetTableModel()));
	UITableModelDefault * const tableModelWillNotPurchase = NON_NULL(safe_cast<UITableModelDefault *>(m_tableWillNotPurchase->GetTableModel()));

	tableModelWillPurchase->Attach(NULL);
	tableModelWillNotPurchase->Attach(NULL);

	m_tableWillPurchase->SetTableModel(NULL);
	m_tableWillNotPurchase->SetTableModel(NULL);

	tableModelWillPurchase->ClearTable();
	tableModelWillNotPurchase->ClearTable();

	m_pageWidgets->Clear();

	typedef std::pair<NetworkId /*resource type*/, int /*amount*/> ResourceTypeAmount;
	typedef stdmap<std::string, ResourceTypeAmount>::fwd CargoHoldClassContents;
	CargoHoldClassContents cargoHoldClassContents;

	//-- count amounts of resources
	//-- space resources are only allowed to have 1 type per class
	//-- more than 1 type per class will confuse this UI, which refers only to the classes

	{
		ShipObject::NetworkIdIntMap const & cargoHoldContents = ship->getCargoHoldContents();
		ShipObject::NetworkIdIntMap::const_iterator const end = cargoHoldContents.end();
		for (ShipObject::NetworkIdIntMap::const_iterator it = cargoHoldContents.begin(); it != end; ++it)
		{
			NetworkId const & resourceTypeId = it->first;
			int const amount = it->second;

			std::string resourceClassName;
			if (!ResourceTypeManager::findTypeParent(resourceTypeId, resourceClassName))
			{
				WARNING(true, ("SwgCuiSpaceMiningSale invalid resource type [%s]", resourceTypeId.getValueString().c_str()));
				continue;
			}

			ResourceClassObject const * const resourceClassObject = Universe::getInstance ().getResourceClassByName (resourceClassName);;

			if (NULL == resourceClassObject)
			{
				//-- already warned during loading
				continue;
			}

			cargoHoldClassContents[resourceClassName] = ResourceTypeAmount(resourceTypeId, amount);
		}
	}

	Unicode::String const strContrast = Unicode::narrowToWide("\\#pcontrast1 ");
	Unicode::String const strReset = Unicode::narrowToWide("\\#.");

	//-- update the will buy table

	{
		int row = 0;
		StringIntMap::const_iterator const end = m_resourceClassPrices->end();
		for (StringIntMap::const_iterator it = m_resourceClassPrices->begin(); it != end; ++it, ++row)
		{
			std::string const & resourceClassName = it->first;
			int const pricePerUnit = it->second;

			//-- the setup code for this map already checked the resource class validity
			ResourceClassObject const * const resourceClassObject = NON_NULL(Universe::getInstance ().getResourceClassByName (resourceClassName));

			CargoHoldClassContents::iterator ship_it = cargoHoldClassContents.find(resourceClassName);

			int amountInShip = 0;
			NetworkId resourceTypeId;

			if (ship_it != cargoHoldClassContents.end())
			{
				resourceTypeId = ship_it->second.first;
				amountInShip = ship_it->second.second;
				cargoHoldClassContents.erase(ship_it);
			}

			UIBaseObject * iconWidget = m_pageWidgets->GetChild(resourceClassName.c_str());
			if (NULL == iconWidget)
			{
				Object * const resourceIcon = ResourceIconManager::getObjectForClass(resourceClassName);
				iconWidget = CuiIconManager::createObjectIcon(*resourceIcon, NULL);
				iconWidget->SetName(resourceClassName);
				m_pageWidgets->AddChild(iconWidget);
			}

			Unicode::String tmpStrPricePerUnit;
			Unicode::String tmpStrAmountInShip;

			Unicode::String tmpStrPath;
			m_tableWillPurchase->GetPathTo(tmpStrPath, iconWidget);
			UIUtils::FormatInteger(tmpStrPricePerUnit, pricePerUnit);
			UIUtils::FormatInteger(tmpStrAmountInShip, amountInShip);

			tableModelWillPurchase->AppendCell(0, resourceTypeId.getValueString().c_str(), tmpStrPath);

			Unicode::String const & resourceClassDisplayName = resourceClassObject->getFriendlyName().localize();
			if (amountInShip > 0)
			{
				tableModelWillPurchase->AppendCell(1, NULL, strContrast + resourceClassDisplayName + strReset);
				tableModelWillPurchase->SetSortKeyAtString(1, row, Unicode::narrowToWide("0_") + resourceClassDisplayName);
			}
			else
			{
				tableModelWillPurchase->AppendCell(1, NULL, resourceClassDisplayName);
			}

			tableModelWillPurchase->AppendCell(2, NULL, tmpStrPricePerUnit);
			tableModelWillPurchase->AppendCell(3, NULL, tmpStrAmountInShip);

		}
	}

	//-- update the will NOT buy table

	if (cargoHoldClassContents.empty())
		m_tableWillNotPurchase->SetEnabled(false);
	else	
	{
		m_tableWillNotPurchase->SetEnabled(true);
		CargoHoldClassContents::const_iterator const end = cargoHoldClassContents.end();
		for (CargoHoldClassContents::const_iterator it = cargoHoldClassContents.begin(); it != end; ++it)
		{
			std::string const & resourceClassName = it->first;
			int const amountInShip = it->second.second;

			//-- the setup code for this map already checked the resource class validity
			ResourceClassObject const * const resourceClassObject = NON_NULL(Universe::getInstance ().getResourceClassByName (resourceClassName));

			Object * const resourceIcon = ResourceIconManager::getObjectForClass(resourceClassName);
			CuiWidget3dObjectListViewer * const iconWidget = CuiIconManager::createObjectIcon(*resourceIcon, NULL);

			m_pageWidgets->AddChild(iconWidget);

			Unicode::String tmpStr;
			tableModelWillNotPurchase->GetPathTo(tmpStr, iconWidget);

			tableModelWillNotPurchase->AppendCell(0, NULL, tmpStr);
			tableModelWillNotPurchase->AppendCell(1, NULL, resourceClassObject->getFriendlyName().localize());
			UIUtils::FormatInteger(tmpStr, amountInShip);
			tableModelWillNotPurchase->AppendCell(2, NULL, tmpStr);

		}
	}

	m_tableWillPurchase->SetTableModel(tableModelWillPurchase);
	m_tableWillNotPurchase->SetTableModel(tableModelWillNotPurchase);

	tableModelWillPurchase->sortOnColumn(1, UITableModel::SD_up);
	tableModelWillNotPurchase->sortOnColumn(1, UITableModel::SD_up);
	
	m_tableWillPurchase->SelectRow(oldSelectedRow);

	resetSellingWindow();
}

//----------------------------------------------------------------------

void SwgCuiSpaceMiningSale::OnGenericSelectionChanged(UIWidget * context)
{
	if (context == m_tableWillPurchase)
	{
		resetSellingWindow();
	}
}

//----------------------------------------------------------------------

void SwgCuiSpaceMiningSale::OnSliderbarChanged(UIWidget * context)
{
	if (context == m_sliderSelling)
		updateSellingWindow();
}

//----------------------------------------------------------------------

void SwgCuiSpaceMiningSale::OnButtonPressed(UIWidget * context)
{
	if (context == m_buttonSell)
		sellResources();
}

//----------------------------------------------------------------------

void SwgCuiSpaceMiningSale::resetSellingWindow()
{
	UITableModelDefault * const tableModelWillPurchase = NON_NULL(safe_cast<UITableModelDefault *>(m_tableWillPurchase->GetTableModel()));

	int const row = m_tableWillPurchase->GetLastSelectedRow();

	if (row >= 0)
	{
		UIData const * const data = tableModelWillPurchase->GetCellDataVisual(row, 0);

		if (!data)
		{
			WARNING(true, ("SwgCuiSpaceMiningSale invalid data"));
		}
		else
		{
			m_sellingResourceId = NetworkId(data->GetName());
			Unicode::String resourceClassName;
			tableModelWillPurchase->GetValueAtText(row, 1, resourceClassName);
			tableModelWillPurchase->GetValueAtInteger(row, 2, m_sellingPricePerUnit);
			int amountInShip = 0;
			tableModelWillPurchase->GetValueAtInteger(row, 3, amountInShip);

			if (amountInShip > 0)
			{
				m_pageSelling->SetEnabled(true);
				m_sliderSelling->SetUpperLimit(amountInShip);
				m_sliderSelling->SetValue(amountInShip, false);

				Unicode::String caption = Unicode::narrowToWide("Sell units of ") + resourceClassName;
				m_textInfoSelling->SetLocalText(caption);

				updateSellingWindow();
				return;
			}
		}
	}

	m_pageSelling->SetEnabled(false);
	m_sliderSelling->SetValue(1, false);
	m_textInfoSelling->Clear();
	m_sellingPricePerUnit = 0;
	m_sellingResourceId = NetworkId::cms_invalid;

	updateSellingWindow();
}

//----------------------------------------------------------------------

void SwgCuiSpaceMiningSale::updateSellingWindow()
{
	if (!m_sellingResourceId.isValid() || m_sellingPricePerUnit <= 0)
	{
		m_textPriceSelling->Clear();
		m_textPriceUnits->Clear();
		return;
	}

	int const unitsSelling = m_sliderSelling->GetValue();

	Unicode::String tmpStr;
	UIUtils::FormatInteger(tmpStr, unitsSelling);
	m_textPriceUnits->SetLocalText(tmpStr + Unicode::narrowToWide(" units"));

	UIUtils::FormatInteger(tmpStr, unitsSelling * m_sellingPricePerUnit);
	m_textPriceSelling->SetLocalText(tmpStr + Unicode::narrowToWide(" credits"));
}

//----------------------------------------------------------------------

void SwgCuiSpaceMiningSale::sellResources()
{
	if (!m_sellingResourceId.isValid() || m_sellingPricePerUnit <= 0)
	{
		WARNING(true, ("SwgCuiSpaceMiningSale invalid selling resource"));
		return;
	}

	ShipObject * const ship = Game::getPlayerContainingShip();
	if (NULL == ship)
	{
		WARNING(true, ("SwgCuiSpaceMiningSalno ship"));
		return;
	}

	int const unitsSelling = m_sliderSelling->GetValue();
	int const amountInHold = ship->getCargoHoldContent(m_sellingResourceId);

	if (amountInHold < unitsSelling)
	{
		WARNING(true, ("SwgCuiSpaceMiningSale can't sell [%s] because it is not in the cargo hold", m_sellingResourceId.getValueString().c_str()));
		return;		
	}

	if (Game::getSinglePlayer())
	{
		ship->clientSetCargoHoldContent(m_sellingResourceId, amountInHold - unitsSelling);
	}
	else
	{
		Object * const player = Game::getPlayer();

		if (NULL != player)
		{
			MessageQueueSpaceMiningSellResource * const msg = new MessageQueueSpaceMiningSellResource(ship->getNetworkId(), m_spaceStationId, m_sellingResourceId, unitsSelling);
			player->getController()->appendMessage(CM_spaceMiningSaleSellResource, 0.0f, msg, GameControllerMessageFlags::SEND | GameControllerMessageFlags::DEST_SERVER | GameControllerMessageFlags::RELIABLE);
		}
	}
}

//----------------------------------------------------------------------

void SwgCuiSpaceMiningSale::onShipCargoChanged(ShipObject::Messages::CargoChanged::Payload & payload)
{
	if (&payload == Game::getPlayerContainingShip())
	{
		m_dirty = true;
	}
}

//----------------------------------------------------------------------

