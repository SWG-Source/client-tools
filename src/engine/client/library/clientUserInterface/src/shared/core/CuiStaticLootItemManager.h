// ======================================================================
//
// CuiStaticLootItemManager.h
//
// Copyright 2005 Sony Online Entertainment
//
// ======================================================================

#ifndef INCLUDED_CuiStaticLootItemManager_H
#define INCLUDED_CuiStaticLootItemManager_H

// ======================================================================

#include <vector>
#include <map>

// ======================================================================

class Object;

// ======================================================================

class CuiStaticLootItemManager
{
public:

	struct Messages
	{
		struct AttributesChanged
		{
			typedef std::string Payload;
		};
	};


	typedef std::map<Unicode::String, Unicode::String> ItemDictionary;

	static void install();
	static void remove();
	static void setItemData(std::vector<Unicode::String> const & keys, std::vector<Unicode::String> const & values);
	static Object const * getLootItemObject(std::string const & lootItemName);
	static ItemDictionary const * getItemData(std::string const & lootItemName);
	static stdvector<std::string>::fwd const & getAllKnownLootItems();
	static Unicode::String getTooltipAttributeString(std::string const & lootItem);
};

// ======================================================================

#endif // INCLUDED_CuiStaticLootItemManager_H

