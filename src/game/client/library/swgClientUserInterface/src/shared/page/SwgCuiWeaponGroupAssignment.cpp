//===================================================================
//
// SwgCuiWeaponGroupAssignment.cpp
// copyright 2004, sony online entertainment
//
//===================================================================

#include "swgClientUserInterface/FirstSwgClientUserInterface.h"
#include "swgClientUserInterface/SwgCuiWeaponGroupAssignment.h"

#include "UIButton.h"
#include "UIData.h"
#include "UIList.h"
#include "UIMessage.h"
#include "UIText.h"
#include "clientGame/Game.h"
#include "clientGame/ObjectAttributeManager.h"
#include "clientGame/ShipObject.h"
#include "clientGame/ShipWeaponGroup.h"
#include "clientGame/ShipWeaponGroupManager.h"
#include "clientGame/ShipWeaponGroupsForShip.h"
#include "clientUserInterface/CuiManager.h"
#include "clientUserInterface/CuiStringIdsSpaceHud.h"
#include "clientUserInterface/CuiStringIdsWeaponGroupAssignment.h"
#include "sharedFoundation/FormattedString.h"
#include "sharedGame/ShipComponentData.h"
#include "sharedGame/ShipComponentDescriptor.h"
#include "sharedMessageDispatch/Transceiver.h"
#include "sharedObject/NetworkIdManager.h"

//===================================================================

namespace SwgCuiWeaponGroupAssignmentNamespace
{
	UILowerString const cms_weaponIndex("weaponindex");
}

using namespace SwgCuiWeaponGroupAssignmentNamespace;

//===================================================================

SwgCuiWeaponGroupAssignment::SwgCuiWeaponGroupAssignment (UIPage& page) :
UIEventCallback(),
CuiMediator("SwgCuiWeaponGroupAssignment",page),
m_weaponGroupList(NULL),
m_allWeaponsList(NULL),
m_weaponsInSelectedGroupList(NULL),
m_addButton(NULL),
m_removeButton(NULL),
m_shipId(NetworkId::cms_invalid),
m_selectedGroup(0),
m_callback(new MessageDispatch::Callback),
m_textAttribsTop(NULL),
m_textAttribsBottom(NULL)
{
	getCodeDataObject (TUIList,   m_weaponGroupList, "leftlist");
	m_weaponGroupList->Clear();
	registerMediatorObject (*m_weaponGroupList, true);

	getCodeDataObject (TUIList,   m_allWeaponsList, "toplist");
	m_allWeaponsList->Clear();
	registerMediatorObject (*m_allWeaponsList, true);

	getCodeDataObject (TUIList,   m_weaponsInSelectedGroupList, "bottomlist");
	m_weaponsInSelectedGroupList->Clear();
	registerMediatorObject (*m_weaponsInSelectedGroupList, true);

	getCodeDataObject (TUIButton,   m_addButton, "addbutton");
	registerMediatorObject (*m_addButton, true);

	getCodeDataObject (TUIButton,   m_removeButton, "removebutton");
	registerMediatorObject (*m_removeButton, true);

	getCodeDataObject (TUIText,   m_textAttribsTop, "topTextAttribs");
	getCodeDataObject (TUIText,   m_textAttribsBottom, "bottomTextAttribs");

	setState (MS_closeable);
	setState (MS_closeDeactivates);

	m_textAttribsTop->SetPreLocalized(true);
	m_textAttribsBottom->SetPreLocalized(true);
}

//----------------------------------------------------------------------

SwgCuiWeaponGroupAssignment::~SwgCuiWeaponGroupAssignment ()
{
	delete m_callback;
	m_callback = NULL;
}

//-------------------------------------------------------------------

void SwgCuiWeaponGroupAssignment::performActivate ()
{
	CuiMediator::performActivate();
	CuiManager::requestPointer (true);

	ShipObject const * const ship = Game::getPlayerPilotedShip();
	if(ship)
		m_shipId = ship->getNetworkId();

	initializeUi();

	m_callback->connect (*this, &SwgCuiWeaponGroupAssignment::onWeaponGroupsResetForShip,                static_cast<ShipWeaponGroupManager::Messages::WeaponGroupsResetForShip*>     (0));
}

//-------------------------------------------------------------------

void SwgCuiWeaponGroupAssignment::performDeactivate ()
{
	CuiMediator::performDeactivate();
	CuiManager::requestPointer (false);
	m_callback->disconnect (*this, &SwgCuiWeaponGroupAssignment::onWeaponGroupsResetForShip,                static_cast<ShipWeaponGroupManager::Messages::WeaponGroupsResetForShip*>     (0));
}

//-------------------------------------------------------------------

void SwgCuiWeaponGroupAssignment::initializeUi()
{
	m_allWeaponsList->Clear();
	m_weaponsInSelectedGroupList->Clear();
	m_textAttribsTop->Clear();
	m_textAttribsBottom->Clear();

	if(!m_shipId.isValid())
	{
		m_weaponGroupList->Clear();
		return;
	}

	initializeGroupListUi();
	updateWeaponDescriptions();
}

//----------------------------------------------------------------------

void SwgCuiWeaponGroupAssignment::initializeGroupListUi()
{
	long const selectedRow = std::max(0L, m_weaponGroupList->GetLastSelectedRow());

	m_weaponGroupList->Clear();

	if(!m_shipId.isValid())
		return;

	ShipWeaponGroupsForShip const & weaponGroups = ShipWeaponGroupManager::getShipWeaponGroupsForShip(m_shipId);

	char buffer[256];
	for(int i = 0; i < weaponGroups.getGroupCount(); ++i)
	{
		Unicode::String const & pre = CuiStringIdsSpaceHud::weapon_group.localize();
		snprintf(buffer, sizeof(buffer)-1, "%d", i + 1); //UI references weapon groups from 1 base countb

		Unicode::String suffix;

		ShipWeaponGroup const & group = weaponGroups.getGroup(i);

		if (group.isEmpty())
		{
			suffix = CuiStringIdsWeaponGroupAssignment::suffix_empty.localize();
		}
		else
		{
			if (group.hasMissiles())
				suffix = CuiStringIdsWeaponGroupAssignment::suffix_missile.localize();
			else if (group.hasCountermeasures())
				suffix = CuiStringIdsWeaponGroupAssignment::suffix_countermeasure.localize();
			else
				suffix = CuiStringIdsWeaponGroupAssignment::suffix_projectile.localize();
		}

		Unicode::String const & str = pre + Unicode::narrowToWide(buffer) + suffix;
		m_weaponGroupList->AddRow(str, Unicode::wideToNarrow(str));
	}

	m_weaponGroupList->SelectRow(selectedRow);
	setSelectedWeaponGroup(m_weaponGroupList->GetLastSelectedRow());
}

//----------------------------------------------------------------------

void SwgCuiWeaponGroupAssignment::OnGenericSelectionChanged (UIWidget * const context)
{
	if (context == m_weaponGroupList)
	{
		int const selectedRow = m_weaponGroupList->GetLastSelectedRow();
		setSelectedWeaponGroup(selectedRow);
	}
	else if (context == m_allWeaponsList)
	{
		m_addButton->SetEnabled(true);
		m_removeButton->SetEnabled(false);

		updateWeaponDescriptions();
	}
	else if (context == m_weaponsInSelectedGroupList)
	{
		m_addButton->SetEnabled(false);
		m_removeButton->SetEnabled(true);

		updateWeaponDescriptions();
	}
}

//----------------------------------------------------------------------

bool SwgCuiWeaponGroupAssignment::OnMessage (UIWidget * const context, UIMessage const & msg)
{
	if (msg.Type == UIMessage::LeftMouseDoubleClick)
	{
		if (context == m_allWeaponsList)
		{
			m_addButton->Press();
			return false;
		}
		else if (context == m_weaponsInSelectedGroupList)
		{
			m_removeButton->Press();
			return false;
		}
	}

	return true;
}

//----------------------------------------------------------------------

void SwgCuiWeaponGroupAssignment::OnButtonPressed   (UIWidget * const context)
{
	if (!m_shipId.isValid())
		return;

	if (context == m_addButton)
	{
		UIData const * const data = m_allWeaponsList->GetDataAtRow(m_allWeaponsList->GetLastSelectedRow());
		if(data)
		{
			int weaponIndex = 0;
			bool const success = data->GetPropertyInteger(cms_weaponIndex, weaponIndex);
			if(success)
			{
				ShipObject * const ship = getShipObject();

				if (NULL != ship)
				{
					bool const isMissile = ship->isMissile(weaponIndex);
					bool const isCountermeasure = ship->isCountermeasure(weaponIndex);

					ShipWeaponGroupsForShip & weaponGroups = ShipWeaponGroupManager::getShipWeaponGroupsForShip(m_shipId);
					ShipWeaponGroup & group = weaponGroups.getGroup(m_selectedGroup);

					if (!group.canWeaponAdd(isMissile, isCountermeasure))
						return;

					group.addWeapon(weaponIndex, isMissile, isCountermeasure);
					initializeGroupListUi();
				}
				else
					WARNING(true, ("Invalid ship [%s] in SwgCuiWeaponGroupAssignment::OnButtonPressed", m_shipId.getValueString().c_str()));
			}
		}
	}
	else if (context == m_removeButton)
	{
		UIData const * const data = m_weaponsInSelectedGroupList->GetDataAtRow(m_weaponsInSelectedGroupList->GetLastSelectedRow());
		if(data)
		{
			int weaponIndex = 0;
			bool const success = data->GetPropertyInteger(cms_weaponIndex, weaponIndex);
			if(success)
			{
				ShipWeaponGroupsForShip & weaponGroups = ShipWeaponGroupManager::getShipWeaponGroupsForShip(m_shipId);
				ShipWeaponGroup & group = weaponGroups.getGroup(m_selectedGroup);
				group.removeWeapon(weaponIndex);
				initializeGroupListUi();
			}
		}
	}
}

//----------------------------------------------------------------------

void SwgCuiWeaponGroupAssignment::setSelectedWeaponGroup(int const groupNumber)
{
	m_allWeaponsList->Clear();
	m_weaponsInSelectedGroupList->Clear();

	if (!m_shipId.isValid())
		return;

	if (groupNumber < 0)
		return;

	ShipWeaponGroupsForShip const & weaponGroups = ShipWeaponGroupManager::getShipWeaponGroupsForShip(m_shipId);
	ShipObject * const ship = getShipObject();

	char buffer[16];
	FormattedString<1024> fs;

	Unicode::String const highlight = Unicode::narrowToWide("\\#pcontrast1 ");
	if(ship)
	{
		ShipWeaponGroup const & group = weaponGroups.getGroup(groupNumber);
		int weaponIndex = 0;
		for(int slot = ShipChassisSlotType::SCST_weapon_first; slot <= ShipChassisSlotType::SCST_weapon_last; ++slot, ++weaponIndex)
		{
			if(ship->isSlotInstalled(slot))
			{
				std::string const & name = ShipChassisSlotType::getNameFromType (static_cast<ShipChassisSlotType::Type>(slot));
				StringId sid("ship_slot_n", name);
				_itoa(weaponIndex, buffer, 10);

				char const * const prefix = fs.sprintf("(%d)\\>032 ", weaponIndex + 1);
				Unicode::String const & weaponName = ship->getComponentName(slot);
				bool const isMissile = ship->isMissile(slot - ShipChassisSlotType::SCST_weapon_first);
				bool const isCountermeasure = ship->isCountermeasure(slot - ShipChassisSlotType::SCST_weapon_first);
				
				Unicode::String const & colorPrefix = group.canWeaponAdd(isMissile, isCountermeasure) ? highlight : Unicode::emptyString;
				Unicode::String const & weaponNameSlotDisplay = colorPrefix + Unicode::narrowToWide(prefix) + weaponName;

				if(group.hasWeapon(weaponIndex))
				{
					m_weaponsInSelectedGroupList->AddRow(weaponNameSlotDisplay, Unicode::wideToNarrow(weaponName));
					UIData * const data = m_weaponsInSelectedGroupList->GetDataAtRow(m_weaponsInSelectedGroupList->GetRowCount() - 1);
					if(data)
						IGNORE_RETURN(data->SetProperty(cms_weaponIndex, Unicode::narrowToWide(buffer)));
				}
				else
				{
					m_allWeaponsList->AddRow(weaponNameSlotDisplay, Unicode::wideToNarrow(weaponName));
					UIData * const data = m_allWeaponsList->GetDataAtRow(m_allWeaponsList->GetRowCount() - 1);
					if(data)
						IGNORE_RETURN(data->SetProperty(cms_weaponIndex, Unicode::narrowToWide(buffer)));
				}
			}
		}
		m_selectedGroup = groupNumber;
	}
}

//----------------------------------------------------------------------

void SwgCuiWeaponGroupAssignment::onWeaponGroupsResetForShip(ShipWeaponGroupManager::Messages::WeaponGroupsResetForShip::Payload const & shipId)
{
	if (m_shipId == shipId)
		initializeUi();
}

//----------------------------------------------------------------------
		
void SwgCuiWeaponGroupAssignment::updateWeaponDescriptions()
{
	m_textAttribsTop->Clear();
	m_textAttribsBottom->Clear();
	
	ShipObject * const ship = getShipObject();
	
	if (NULL == ship)
		return;
	
	{
		UIData const * const dataTop = m_allWeaponsList->GetDataAtRow(m_allWeaponsList->GetLastSelectedRow());
		
		if (NULL != dataTop)
		{
			int weaponIndex = 0;
			if (dataTop->GetPropertyInteger(cms_weaponIndex, weaponIndex))
			{
				ShipComponentData * const shipComponentData = ship->createShipComponentData(static_cast<int>(ShipChassisSlotType::SCST_weapon_first) + weaponIndex);
				if (NULL != shipComponentData)
				{
					ShipComponentData::AttributeVector attribs;
					shipComponentData->getAttributes(attribs);
					delete shipComponentData;
					
					Unicode::String result;
					ObjectAttributeManager::formatAttributes(attribs, result, NULL, NULL, false);
					m_textAttribsTop->SetLocalText(result);
				}
			}
		}
	}
	
	{
		UIData const * const dataBottom = m_allWeaponsList->GetDataAtRow(m_weaponsInSelectedGroupList->GetLastSelectedRow());
		
		if (NULL != dataBottom)
		{
			int weaponIndex = 0;
			if (dataBottom->GetPropertyInteger(cms_weaponIndex, weaponIndex))
			{
				ShipComponentData * const shipComponentData = ship->createShipComponentData(static_cast<int>(ShipChassisSlotType::SCST_weapon_first) + weaponIndex);
				if (NULL != shipComponentData)
				{
					ShipComponentData::AttributeVector attribs;
					shipComponentData->getAttributes(attribs);
					delete shipComponentData;
					
					Unicode::String result;
					ObjectAttributeManager::formatAttributes(attribs, result, NULL, NULL, false);
					m_textAttribsBottom->SetLocalText(result);
				}
			}
		}
	}
}

//----------------------------------------------------------------------

ShipObject * SwgCuiWeaponGroupAssignment::getShipObject() const
{
	Object * const o = NetworkIdManager::getObjectById(m_shipId);
	ClientObject * const co = o ? o->asClientObject() : NULL;
	return co ? co->asShipObject() : NULL;
}

//===================================================================

