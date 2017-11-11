// ======================================================================
//
// CuiStaticLootItemManager.cpp
//
// Copyright 2005 Sony Online Entertainment
//
// ======================================================================

#include "clientUserInterface/FirstClientUserInterface.h"
#include "clientUserInterface/CuiStaticLootItemManager.h"
#include "clientUserInterface/CuiWidget3dObjectListViewer.h"

#include "clientGame/ObjectAttributeManager.h"
#include "sharedFoundation/ConstCharCrcString.h"
#include "sharedFoundation/PointerDeleter.h"
#include "sharedMessageDispatch/Transceiver.h"
#include "sharedObject/ObjectTemplateList.h"

#include <algorithm>

// ======================================================================

namespace CuiStaticLootItemManagerNamespace
{
	namespace Transceivers
	{
		MessageDispatch::Transceiver<const CuiStaticLootItemManager::Messages::AttributesChanged::Payload &, CuiStaticLootItemManager::Messages::AttributesChanged >
			lootItemAttributesChanged;
	}

	std::map<std::string, CuiStaticLootItemManager::ItemDictionary> ms_itemData;

	//TODO could this get big and heavy, should we limit the size and delete old ones?
	std::map<std::string, Object *> ms_itemObjects;

	//this is used to return a list of known entries.  Lazily updated by getAllKnownLootItems()
	std::vector<std::string> ms_itemNames;

	Unicode::String const cms_nameString(Unicode::narrowToWide("name"));
}
using namespace CuiStaticLootItemManagerNamespace;

// ======================================================================

void CuiStaticLootItemManager::install()
{
	ms_itemData.clear();
	ms_itemObjects.clear();
}

//-----------------------------------------------------------------------

void CuiStaticLootItemManager::remove()
{
	ms_itemData.clear();

	std::for_each (ms_itemObjects.begin (), ms_itemObjects.end (), PointerDeleterPairSecond ());
	ms_itemObjects.clear();
}

//-----------------------------------------------------------------------

void CuiStaticLootItemManager::setItemData(std::vector<Unicode::String> const & keys, std::vector<Unicode::String> const & values)
{
	DEBUG_FATAL(keys.size() != values.size(), ("keys and values mismatched!"));
	ItemDictionary itemData;
	std::vector<Unicode::String>::const_iterator i = keys.begin();
	std::vector<Unicode::String>::const_iterator j = values.begin();
	Unicode::String itemName;
	for(; i != keys.end(); ++i, ++j)
	{
		itemData[*i] = *j;
		if(*i == cms_nameString)
			itemName = *j;
	}
	ms_itemData[Unicode::wideToNarrow(itemName)] = itemData;

	Transceivers::lootItemAttributesChanged.emitMessage(Unicode::wideToNarrow(itemName));
}

//-----------------------------------------------------------------------

Object const * CuiStaticLootItemManager::getLootItemObject(std::string const & lootItemName)
{
	std::map<std::string, Object *>::const_iterator i = ms_itemObjects.find(lootItemName);
	if(i != ms_itemObjects.end())
		return i->second;

	ItemDictionary const * const itemData = getItemData(lootItemName);

	if(itemData)
	{
		ItemDictionary::const_iterator i2 = itemData->find(Unicode::narrowToWide("template"));
		if(i2 != itemData->end())
		{
			std::string const & templateName = Unicode::wideToNarrow(i2->second);
			std::string::size_type const & slash = templateName.rfind('/');
			std::string const shared_templateName(templateName.substr(0, slash + 1) + "shared_" + templateName.substr(slash + 1));
			Object * const item = ObjectTemplateList::createObject (ConstCharCrcString(shared_templateName.c_str()));
			ms_itemObjects[lootItemName] = item;
			return item;
		}
	}

	return NULL;
}

//-----------------------------------------------------------------------

CuiStaticLootItemManager::ItemDictionary const * CuiStaticLootItemManager::getItemData(std::string const & lootItemName)
{
	//this will return an entry value, is this okay?  Probably.
	if(ms_itemData.find(lootItemName) != ms_itemData.end())
		return &ms_itemData[lootItemName];
	else
		return NULL;
}

//-----------------------------------------------------------------------

std::vector<std::string> const & CuiStaticLootItemManager::getAllKnownLootItems()
{
	ms_itemNames.clear();
	for(std::map<std::string, CuiStaticLootItemManager::ItemDictionary>::const_iterator i = ms_itemData.begin(); i != ms_itemData.end(); ++i)
	{
		ms_itemNames.push_back(i->first);
	}
	return ms_itemNames;
}

//-----------------------------------------------------------------------

Unicode::String CuiStaticLootItemManager::getTooltipAttributeString(std::string const & lootItem)
{
	ObjectAttributeManager::AttributeVector attribs;
	ObjectAttributeManager::getAttributes(lootItem, attribs);
	Unicode::String result = StringId("static_item_n", lootItem).localize() + Unicode::narrowToWide("\n");
	Unicode::String result2;
	ObjectAttributeManager::formatAttributes(attribs, result2, NULL, NULL, false, true);
	return result + result2;
}

// ======================================================================

