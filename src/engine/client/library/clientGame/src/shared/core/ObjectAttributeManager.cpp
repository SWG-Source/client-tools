//======================================================================
//
// ObjectAttributeManager.cpp
// copyright (c) 2002 Sony Online Entertainment
//
//======================================================================

#include "clientGame/FirstClientGame.h"
#include "clientGame/ObjectAttributeManager.h"

#if WIN32
#pragma warning (disable:4503) // truncated debug symbol
#endif

#include "clientGame/ClientCommandQueue.h"
#include "clientGame/ClientMissionObject.h"
#include "clientGame/ClientObjectUsabilityManager.h"
#include "clientGame/ClientTextManager.h"
#include "clientGame/ClientWaypointObject.h"
#include "clientGame/CreatureObject.h"
#include "clientGame/Game.h"
#include "clientGame/PlayerObject.h"
#include "clientGame/ResourceContainerObject.h"
#include "clientGame/ResourceTypeManager.h"
#include "clientGame/WeaponObject.h"
#include "clientUserInterface/CuiCombatManager.h"
#include "clientUserInterface/CuiCraftManager.h"
#include "clientUserInterface/CuiManager.h"
#include "clientUserInterface/CuiPreferences.h"
#include "clientUserInterface/CuiUtils.h"
#include "sharedDebug/InstallTimer.h"
#include "sharedFoundation/Crc.h"
#include "sharedFoundation/NetworkId.h"
#include "sharedFoundation/Production.h"
#include "sharedFoundation/Timer.h"
#include "sharedGame/GameObjectTypes.h"
#include "sharedGame/GroundZoneManager.h"
#include "sharedGame/SharedBuildoutAreaManager.h"
#include "sharedGame/SharedObjectAttributes.h"
#include "sharedGame/SharedStringIds.h"
#include "sharedGame/SharedWeaponObjectTemplate.h"
#include "sharedGame/Universe.h"
#include "sharedMath/PackedRgb.h"
#include "sharedMessageDispatch/Message.h"
#include "sharedMessageDispatch/Receiver.h"
#include "sharedMessageDispatch/Transceiver.h"
#include "sharedNetworkMessages/AttributeListMessage.h"
#include "sharedObject/Appearance.h"
#include "sharedObject/AppearanceTemplate.h"
#include "sharedObject/NetworkIdManager.h"

#include "StringId.h"
#include "UITypes.h"
#include "UIUtils.h"
#include "Unicode.h"
#include "UnicodeUtils.h"

#include <algorithm>
#include <cstdio>
#include <map>
#include <set>
#include <sstream>

//======================================================================

namespace ObjectAttributeManagerNamespace
{
	namespace Transceivers
	{
		MessageDispatch::Transceiver<const ObjectAttributeManager::Messages::AttributesChanged::Payload &, ObjectAttributeManager::Messages::AttributesChanged >
			attributesChanged;

		MessageDispatch::Transceiver<const ObjectAttributeManager::Messages::StaticItemAttributesChanged::Payload &, ObjectAttributeManager::Messages::StaticItemAttributesChanged >
			staticItemAttributesChanged;
	}

	typedef ObjectAttributeManager::AttributePair   AttributePair;
	typedef ObjectAttributeManager::AttributeVector AttributeVector;

	//----------------------------------------------------------------------

	Unicode::unicode_char_t const bad_chars[] = { '\n', '\r', '\t', 0x3000, 0 };

	// set the client revision to some arbitrary negative value that is different from the default server revision (see TaskGetAttributes.cpp).
	int const s_initialClientRevision = -255;

	struct AttributeInfo
	{
		AttributeVector attribs;
		int lastFrameUpdated;
		bool hasTooltips;
		bool noTrade;
		bool noTradeShared;
		int revision;
		int tier;
		bool unique;
		bool hasCrafter;

		AttributeInfo (const AttributeVector & _attribs, int _lastFrameUpdated, int _revision) :
		attribs(_attribs),
		lastFrameUpdated (_lastFrameUpdated),
		hasTooltips(false),
		noTrade(false),
		noTradeShared(false),
		revision(_revision),
		tier(-1),
		unique(false),
		hasCrafter(false)
		{
			for (AttributeVector::iterator itAttrib = attribs.begin(); itAttrib != attribs.end(); ++itAttrib)
			{
				AttributePair & attribPair = *itAttrib;

				if (attribPair.first.find(SharedObjectAttributes::tooltip) != std::string::npos)
					hasTooltips = true;

				if (attribPair.first.find(SharedObjectAttributes::no_trade) != std::string::npos)
					noTrade = true;

				if (attribPair.first.find(SharedObjectAttributes::no_trade_shared) != std::string::npos)
					noTradeShared = true;

				if (attribPair.first.find(SharedObjectAttributes::unique) != std::string::npos)
					unique = true;

				if (attribPair.first.find(SharedObjectAttributes::tier) != std::string::npos)
					tier = atoi(Unicode::wideToNarrow(attribPair.second).c_str());

				if(attribPair.first.find(SharedObjectAttributes::crafter) != std::string::npos)
					hasCrafter = true;
			}
		}

		AttributeInfo() :
		attribs(),
		lastFrameUpdated(),
		hasTooltips(false),
		noTrade(false),
		noTradeShared(false),
		revision(s_initialClientRevision),
		tier(-1),
		unique(false),
		hasCrafter(false)
		{
		}
	};

	//----------------------------------------------------------------------

	typedef stdmap<NetworkId, AttributeInfo>::fwd AttributeMap;
	AttributeMap s_attribs;
	typedef stdmap<std::string, AttributeInfo>::fwd StaticItemAttributeMap;
	StaticItemAttributeMap s_staticItemsAttribs;

	std::set<NetworkId> ms_attributeRequestSet;

	// after the initial batch of attribute requests, cache up requests and wait before sending.
	Timer ms_attributeThrottle(1.5f);

	// do not request new attributes until the server acknowledges your previous request or it times out.
	std::set<NetworkId> ms_outstandingAttributeRequestSet;

	// After a few seconds, the client can resend requests.  This will prevent a flood.
	Timer ms_attributeTimeout(5.0f);


	//----------------------------------------------------------------------

	class Listener :
	public MessageDispatch::Receiver
	{
	public:

		//----------------------------------------------------------------------

		Listener () :
		MessageDispatch::Receiver ()
		{
			connectToMessage(AttributeListMessage::MessageType);
		}

		//----------------------------------------------------------------------

		void receiveMessage(const MessageDispatch::Emitter &, const MessageDispatch::MessageBase & message)
		{
			//----------------------------------------------------------------------

			if (message.isType(AttributeListMessage::MessageType))
			{
				Archive::ReadIterator ri = NON_NULL(safe_cast<const GameNetworkMessage *>(&message))->getByteStream().begin();
				AttributeListMessage const msg(ri);
				if(msg.getNetworkId() != NetworkId::cms_invalid)
					ObjectAttributeManager::updateAttributes(msg.getNetworkId(), msg.getData(), msg.getRevision());
				else
					ObjectAttributeManager::updateAttributes(msg.getStaticItemName(), msg.getData(), msg.getRevision());
				ms_outstandingAttributeRequestSet.erase(msg.getNetworkId());
			}
		}
	};

	Listener * s_listener = 0;

	typedef stdvector<std::string>::fwd StringVector;
	StringVector s_minimalAttributeNames;
	StringVector s_minimalCategoryNames;
	StringVector s_statModNames;

	//----------------------------------------------------------------------
	// Reset the attributes when skills change, player is setup, or the admin flag changes.
	class MyCallback : public MessageDispatch::Callback
	{
	public:
		MyCallback() :
		MessageDispatch::Callback()
		{
			connect(*this, &MyCallback::onSkillsChanged, static_cast<CreatureObject::Messages::SkillsChanged *>(0));
			connect(*this, &MyCallback::onPlayerSetup, static_cast<CreatureObject::Messages::PlayerSetup *>(0));
			connect(*this, &MyCallback::onPlayerAdminChanged, static_cast<CreatureObject::Messages::AdminChanged *>(0));
		}

		void onSkillsChanged(const CreatureObject::Messages::SkillsChanged::Payload & payload)
		{
			if (&payload == static_cast<const Object *>(Game::getPlayer()))
				ObjectAttributeManager::reset();
		}

		void onPlayerSetup(const CreatureObject::Messages::PlayerSetup::Payload & payload)
		{
			if (&payload == static_cast<const Object *>(Game::getPlayer ()))
				ObjectAttributeManager::reset();
		}

		void onPlayerAdminChanged(const CreatureObject::Messages::AdminChanged::Payload &)
		{
			ObjectAttributeManager::reset();
		}
	};

	//----------------------------------------------------------------------

	MyCallback * s_callback = 0;

	//----------------------------------------------------------------------

	typedef std::map<CuiMediator const *, NetworkId> WidgetWatcherMap;
	WidgetWatcherMap ms_watchedObjects;

	//----------------------------------------------------------------------

	static const Unicode::String keyColor             = Unicode::narrowToWide ("\\#pcontrast1 ");
	static const Unicode::String categoryColor        = Unicode::narrowToWide ("\\#pcontrast2 ");
	static const Unicode::String indent               = Unicode::narrowToWide (":   \\>072\\#.");
	static const Unicode::String unindent             = Unicode::narrowToWide ("\\>000\n");
	static const Unicode::String category_indent      = Unicode::narrowToWide (":   \\>104\\#.");
	static const Unicode::String category_preindent   = Unicode::narrowToWide ("\\>024");
	static const Unicode::String redColor             = Unicode::narrowToWide ("\\#ff0000");
	static const Unicode::String greenColor           = Unicode::narrowToWide ("\\#00ff00");
	static const Unicode::String purpleColor          = Unicode::narrowToWide ("\\#8000ff");

	StringId const canWearId("object_usability", "can_wear");
	StringId const canWearDescId("object_usability", "can_wear_desc");
	StringId const canNotWearId("object_usability", "can_not_wear");
	StringId const canNotWearDescId("object_usability", "can_not_wear_desc");
	StringId const canUseId("object_usability", "can_use");
	StringId const canUseDescId("object_usability", "can_use_desc");
	StringId const canNotUseId("object_usability", "can_not_use");
	StringId const canNotUseDescId("object_usability", "can_not_use_desc");
	StringId const appearanceOnly("object_usability", "appearance_only");
	StringId const appearanceOnlyInvis("object_usability", "appearance_only_invisible");
	StringId const restrictedAccess("object_usability", "restricted_access");

	// tier...
	static StringId s_tierIdArray [] = 
	{
		StringId("object_usability", "tier_0"),
		StringId("object_usability", "tier_1"),
		StringId("object_usability", "tier_2"),
		StringId("object_usability", "tier_3"),
		StringId("object_usability", "tier_4"),
		StringId("object_usability", "tier_5"),
		StringId("object_usability", "tier_6")
	};

	int const s_tierArraySize = sizeof(s_tierIdArray) / sizeof(StringId);

	static StringId s_tierIdDescArray[s_tierArraySize] = 
	{
		StringId("object_usability", "tier_desc_0"),
		StringId("object_usability", "tier_desc_1"),
		StringId("object_usability", "tier_desc_2"),
		StringId("object_usability", "tier_desc_3"),
		StringId("object_usability", "tier_desc_4"),
		StringId("object_usability", "tier_desc_5"),
		StringId("object_usability", "tier_desc_6")
	};

	UIColor s_tierColorArray[s_tierArraySize] =
	{
		UIColor(0xff,0xff,0xff),	// 0:Unknown
		UIColor(0x99,0x99,0x99),	// 1:Parts   (grey color text)
		UIColor(0xff,0xff,0xff),	// 2:Standard  (white color text)
		UIColor(0xff,0xff,0x00),	// 3:Manufactured  (yellow color text)
		UIColor(0xff,0x00,0x00),	// 4:Prototype  (green color text)
		UIColor(0x00,0x00,0xff),	// 5:Old Republic Technology  (blue color text)
		UIColor(0x00,0xff,0xff)		// 6:Lost Technology  (purple color text)
	};
	std::string const luck("luck_modified");
	std::string const stamina ("stamina_modified");
	std::string const strength ("strength_modified");
	std::string const agility ("agility_modified");
	std::string const constitution ("constitution_modified");
	std::string const precision ("precision_modified");

}

using namespace ObjectAttributeManagerNamespace;

//----------------------------------------------------------------------

const std::string ObjectAttributeManager::ms_nameStringTable = "obj_attr_n";
const std::string ObjectAttributeManager::ms_descStringTable = "obj_attr_d";

//----------------------------------------------------------------------

void ObjectAttributeManager::install ()
{
	InstallTimer const installTimer("ObjectAttributeManager::install");

	DEBUG_FATAL (s_listener, ("already installed"));
	s_listener = new Listener;
	s_callback  = new MyCallback;

	s_minimalAttributeNames.clear();
	s_minimalCategoryNames.clear();
	ms_attributeRequestSet.clear();

	s_minimalAttributeNames.push_back (SharedObjectAttributes::condition);
	s_minimalAttributeNames.push_back (SharedObjectAttributes::contents);
	s_minimalAttributeNames.push_back (SharedObjectAttributes::counter_charges_remaining);
	s_minimalAttributeNames.push_back (SharedObjectAttributes::counter_uses_remaining);
	s_minimalAttributeNames.push_back (SharedObjectAttributes::craft_tool_status);
	s_minimalAttributeNames.push_back (SharedObjectAttributes::craft_tool_time);
	s_minimalAttributeNames.push_back (SharedObjectAttributes::data_volume);
	s_minimalAttributeNames.push_back (SharedObjectAttributes::resource_class);
	s_minimalAttributeNames.push_back (SharedObjectAttributes::resource_contents);
	s_minimalAttributeNames.push_back (SharedObjectAttributes::resource_name);
	s_minimalAttributeNames.push_back (SharedObjectAttributes::travel_arrival_planet);
	s_minimalAttributeNames.push_back (SharedObjectAttributes::travel_arrival_point);
	s_minimalAttributeNames.push_back (SharedObjectAttributes::travel_departure_planet);
	s_minimalAttributeNames.push_back (SharedObjectAttributes::travel_departure_point);
	s_minimalAttributeNames.push_back (SharedObjectAttributes::volume);
	s_minimalAttributeNames.push_back (SharedObjectAttributes::waypoint_planet);
	s_minimalAttributeNames.push_back (SharedObjectAttributes::weapon_cert_status);
	s_minimalAttributeNames.push_back (SharedObjectAttributes::manf_limit);
	s_minimalAttributeNames.push_back (SharedObjectAttributes::factory_count);

	s_statModNames.push_back(luck);
	s_statModNames.push_back(stamina);
	s_statModNames.push_back(strength);
	s_statModNames.push_back(agility);
	s_statModNames.push_back(constitution);
	s_statModNames.push_back(precision);
}

//----------------------------------------------------------------------

void ObjectAttributeManager::remove ()
{
	ms_attributeRequestSet.clear();
	ms_outstandingAttributeRequestSet.clear();

	NOT_NULL (s_listener);
	delete s_listener;
	s_listener = 0;

	delete s_callback;
	s_callback = 0;
}

//----------------------------------------------------------------------

void ObjectAttributeManager::requestUpdate (NetworkId const & id, bool const updateIfAttributesAreEmpty)
{
	if(updateIfAttributesAreEmpty && hasAttributesForId(id))
		return;

	if (Game::getSinglePlayer())
	{
		updateAttributes(id, AttributeVector(), rand());
	}
	else
	{
		// prevent the client from sending requests on an object until it receives an update from the server.
		bool const hasOutstandingRequest = ms_outstandingAttributeRequestSet.find(id) != ms_outstandingAttributeRequestSet.end();

		// allow the currently crafted object to send one update message every second for feedback.
		bool const isCrafting = CuiCraftManager::isCrafting(id);

		if (!hasOutstandingRequest || isCrafting)
		{
			ms_attributeRequestSet.insert(id);
			ms_outstandingAttributeRequestSet.insert(id);
		}
	}
}

//----------------------------------------------------------------------

void ObjectAttributeManager::requestUpdate (std::string const & staticItemName, bool const updateIfAttributesAreEmpty)
{
	UNREF(staticItemName);
	UNREF(updateIfAttributesAreEmpty);
}

//----------------------------------------------------------------------

bool ObjectAttributeManager::hasAttributesForId (NetworkId const & id)
{
	return (s_attribs.find (id) != s_attribs.end());
}

//----------------------------------------------------------------------

bool ObjectAttributeManager::hasAttributesForStaticItemName (std::string const & staticItemName)
{
	return (s_staticItemsAttribs.find (staticItemName) != s_staticItemsAttribs.end());
}

//----------------------------------------------------------------------

void ObjectAttributeManager::updateAttributes(NetworkId const & id, AttributeVector const & av, int const serverRevision)
{
	if (id != NetworkId::cms_invalid)
	{
		static AttributeVector mergedAttributeVector;
		mergedAttributeVector.clear();

		ClientObject const * const obj = dynamic_cast<ClientObject *>(NetworkIdManager::getObjectById(id));
		if (obj)
		{
			populateMergedAttributes (*obj, mergedAttributeVector, av);
		}
		else
			mergedAttributeVector = av;

		s_attribs[id] = AttributeInfo(mergedAttributeVector, Game::getLoopCount(), serverRevision);

		Transceivers::attributesChanged.emitMessage(id);
	}
}

//----------------------------------------------------------------------

void ObjectAttributeManager::updateAttributes(std::string const & staticItemName, const AttributeVector & av, int const /*revision*/)
{
	if (!staticItemName.empty())
	{
		s_staticItemsAttribs[staticItemName] = AttributeInfo(av, Game::getLoopCount(), 1);
		Transceivers::staticItemAttributesChanged.emitMessage(staticItemName);
	}
}

//----------------------------------------------------------------------

bool   ObjectAttributeManager::getAttributes    (NetworkId const & id, AttributeVector & av, bool const forceUpdate, bool const updateIfAttributesAreEmpty)
{
	int frame = 0;
	return getAttributesIfNewer (id, av, frame, forceUpdate, updateIfAttributesAreEmpty);
}

//----------------------------------------------------------------------

bool ObjectAttributeManager::getAttributesIfNewer(NetworkId const & id, AttributeVector & av, int & frame, bool const forceUpdate, bool const updateIfAttributesAreEmpty)
{
	if (forceUpdate || CuiCraftManager::isCrafting())
		requestUpdate(id);

	const AttributeMap::const_iterator it = s_attribs.find (id);
	if (it == s_attribs.end ())
	{
		if(updateIfAttributesAreEmpty)
			requestUpdate(id);
	}
	else
	{
		const AttributeInfo & info = (*it).second;

		if (frame == 0 || frame < info.lastFrameUpdated)
		{
			frame = info.lastFrameUpdated;
			av = ((*it).second).attribs;
			return true;
		}

		return false;
	}

	return true;
}

//----------------------------------------------------------------------

bool ObjectAttributeManager::getAttributes(std::string const & staticItemName, AttributeVector & av, bool const forceUpdate, bool const updateIfAttributesAreEmpty)
{
	int frame = 0;
	return getAttributesIfNewer (staticItemName, av, frame, forceUpdate, updateIfAttributesAreEmpty);
}

//----------------------------------------------------------------------

bool ObjectAttributeManager::getAttributesIfNewer(std::string const & staticItemName, AttributeVector & av, int & frame, bool const forceUpdate, bool const updateIfAttributesAreEmpty)
{
	if (forceUpdate || CuiCraftManager::isCrafting())
		requestUpdate(staticItemName);

	const StaticItemAttributeMap::const_iterator it = s_staticItemsAttribs.find (staticItemName);
	if (it == s_staticItemsAttribs.end ())
	{
		if(updateIfAttributesAreEmpty)
			requestUpdate(staticItemName);
	}
	else
	{
		const AttributeInfo & info = (*it).second;

		if (frame == 0 || frame < info.lastFrameUpdated)
		{
			frame = info.lastFrameUpdated;
			av = ((*it).second).attribs;
			return true;
		}

		return false;
	}

	return true;
}

//----------------------------------------------------------------------

void ObjectAttributeManager::reset ()
{
	s_attribs.clear();
	s_staticItemsAttribs.clear();
	ms_attributeThrottle.reset();
	ms_attributeRequestSet.clear();
	ms_outstandingAttributeRequestSet.clear();
	ms_watchedObjects.clear();
	ClientObjectUsabilityManager::reset();
}

//----------------------------------------------------------------------

bool   ObjectAttributeManager::formatAttributes   (const NetworkId & id, Unicode::String & str, bool minimal, bool const tooltips)
{
	int frame = 0;
	return formatAttributesIfNewer (id, str, minimal, frame, tooltips);
}

//----------------------------------------------------------------------

bool   ObjectAttributeManager::formatAttributesIfNewer   (const NetworkId & id, Unicode::String & str, bool minimal, int & frame, bool const tooltips)
{
	AttributeVector av;
	if (getAttributesIfNewer(id, av, frame, false, true))
	{
		formatAttributes (av, str, NULL, NULL, minimal, tooltips);
		if (!tooltips) 
		{
			appendUsabilityInformation(str, id, true);
		}
		return true;
	}

	return false;
}
//----------------------------------------------------------------------

namespace
{
	inline void createDisplayKey(const std::string & key, Unicode::String & keyDisplayValue, Unicode::String & result)
	{
		if (key [0] == '@')
			keyDisplayValue = StringId::decodeString (Unicode::toLower (Unicode::narrowToWide (key)));
		else if (key [0] == '"')
			keyDisplayValue = StringId::decodeString (Unicode::narrowToWide (key.c_str () + 1));
		else
		{
			const StringId sid (ObjectAttributeManager::ms_nameStringTable, Unicode::toLower (key));
			keyDisplayValue = sid.localize ();
		}

		result.append(keyDisplayValue);
	}
}

//----------------------------------------------------------------------

void ObjectAttributeManager::formatAttributes   (const AttributeVector & av, Unicode::String & str, Unicode::String * categoryDisplayValue, Unicode::String * keyDisplayValue, bool minimal, bool const tooltips)
{
	UNREF (minimal);

	static std::string category;
	static Unicode::String tempCategoryDisplayValue;
	static std::string key;
	static Unicode::String tempKeyDisplayValue;
	static std::string inCategory;
	Unicode::String expertiseMods;
	Unicode::String statMods;

	if (!categoryDisplayValue)
		categoryDisplayValue = &tempCategoryDisplayValue;

	if (!keyDisplayValue)
		keyDisplayValue = &tempKeyDisplayValue;

	inCategory.clear();
	expertiseMods.clear();
	statMods.clear();

	size_t insertLocation = 0;
	bool abortStatNewLine = false;
	for (AttributeVector::const_iterator it = av.begin (); it != av.end (); ++it)
	{
		std::string const & fullKey = (*it).first;

		if (fullKey.empty())
			continue;

		bool const isTooltip = (fullKey.find(SharedObjectAttributes::tooltip) != std::string::npos);
		if (isTooltip != tooltips)
			continue;

		if (fullKey.find(SharedObjectAttributes::no_trade) != std::string::npos)
			continue;

		if (fullKey.find(SharedObjectAttributes::no_trade_shared) != std::string::npos)
			continue;

		if (fullKey.find(SharedObjectAttributes::unique) != std::string::npos)
			continue;

		if (fullKey.find(SharedObjectAttributes::tier) != std::string::npos)
			continue;

		size_t const dotpos = fullKey.find ('.');

		category.clear();
		key.clear();

		if (dotpos != std::string::npos)
		{
			if (!tooltips)
				category = fullKey.substr(0, dotpos);

			key = fullKey.substr(dotpos + 1);
		}
		else
			key = fullKey;

		if (key.empty() && !tooltips)
			continue;

		//-- only print minimal attributes
		if (minimal)
		{
			if (category.empty () || !std::binary_search(s_minimalCategoryNames.begin(), s_minimalCategoryNames.end(), category))
			{
				if (!std::binary_search (s_minimalAttributeNames.begin (), s_minimalAttributeNames.end (), key))
					continue;
			}
		}
		
		bool statModProcessing = false;
		for(size_t i = 0; i < s_statModNames.size(); ++i)
		{
			if(key.find(s_statModNames[i]) != std::string::npos)
			{
				statModProcessing = true;
				break;
			}
		}

		if(statModProcessing && statMods.empty())
		{
			statMods.append (categoryColor);
			statMods.append(1, '\n');
			createDisplayKey("cat_stat_mod_bonus", *categoryDisplayValue, statMods);
			statMods.append (unindent);
		}

		if (category != inCategory && !statModProcessing)
		{
			if(!expertiseMods.empty())
			{
				if(!statMods.empty())
					abortStatNewLine = true;

				str.append(expertiseMods);
				expertiseMods.clear();
			}

			if (!category.empty ())
			{
				str.append (categoryColor);
				str.append(1, '\n');
				createDisplayKey(category, *categoryDisplayValue, str);
				str.append (unindent);
			}
			else
			{
				str.append(1, '\n');
			}

			inCategory = category;
		}

		Unicode::String value = it->second;
		Unicode::String* writeString = &str;

		if(key.find(":expertise") != std::string::npos)
			writeString = &expertiseMods;

		if(statModProcessing)
			writeString = &statMods;
		
		writeString->append (keyColor);

		if (!inCategory.empty () || statModProcessing)
			writeString->append (category_preindent);

		createDisplayKey(key, *keyDisplayValue, *writeString);

		if (!value.empty())
		{
			if (!inCategory.empty()|| statModProcessing)
				writeString->append(category_indent);
			else
				writeString->append(indent);

			size_t pos = 0;
			size_t dollarPos = 0;

			while ((dollarPos = value.find('$', pos)) != std::string::npos)
			{
				size_t const nextDollarPos = value.find('$', dollarPos + 1);
				if (nextDollarPos != std::string::npos)
				{
					Unicode::String const & first = dollarPos > pos ? value.substr(pos, dollarPos - pos) :Unicode::emptyString;
					Unicode::String const & token = value.substr(dollarPos + 1, nextDollarPos - dollarPos - 1);

					writeString->append(StringId::decodeString(first));
					writeString->append(StringId::decodeString(token));

					pos = nextDollarPos + 1;
				}
				else
					break;
			}

			if (pos != std::string::npos)
			{
				if (pos == 0)
					writeString->append(StringId::decodeString(value));
				else
					writeString->append(StringId::decodeString(value.substr(pos)));
			}

			writeString->append(unindent);
		}
		else
			writeString->append (1, '\n');

		if(key.find("volume") != std::string::npos && insertLocation == 0)
			insertLocation = str.length();
	}

	if(!expertiseMods.empty())
		str.append(expertiseMods);

	if(!statMods.empty())
	{
		if(insertLocation != 0)
		{
			if(str[insertLocation + 1] != '\n' && !abortStatNewLine)
				statMods.append(1, '\n');
			str.insert(insertLocation, statMods);
		}
		else
			str.append(statMods);
	}
}

//----------------------------------------------------------------------

bool ObjectAttributeManager::formatDescription (const NetworkId & id, Unicode::String & header, Unicode::String & desc, Unicode::String & attribs, bool mininalAttribs, bool const tooltips)
{
	int frame = 0;
	return formatDescriptionIfNewer (id, header, desc, attribs, mininalAttribs, frame, tooltips);
}

//----------------------------------------------------------------------

bool ObjectAttributeManager::formatDescriptionIfNewer (const NetworkId & id, Unicode::String & header, Unicode::String & desc, Unicode::String & attribs, bool minimalAttribs, int & frame, bool const tooltips)
{
	ClientObject const * const obj = safe_cast<const ClientObject *>(NetworkIdManager::getObjectById (id));

	header.clear ();
	attribs.clear ();

	if (!formatAttributesIfNewer(id, attribs, minimalAttribs, frame, tooltips))
		return false;

#if PRODUCTION == 0
	if (obj && CuiPreferences::getDebugExamine ())
	{
		static Unicode::String debugInfo;
		debugInfo.clear ();
		ObjectAttributeManager::formatDebugInfo (*obj, debugInfo);
		debugInfo.append (1, '\n');
		attribs = debugInfo + attribs;
	}
#endif

	if (obj)
	{
		formatHeaderAndDesc (*obj, header, desc, tooltips);
		return true;
	}
	else
	{
		if (ResourceTypeManager::createTypeDisplayLabel(id, header))
		{
			desc.clear ();
			return true;
		}
	}
	return false;
}

//----------------------------------------------------------------------

void ObjectAttributeManager::formatHeaderAndDesc (const ClientObject & obj, Unicode::String & header, Unicode::String & desc, bool const tooltip)
{
	header.append (obj.getLocalizedName ());
	header.append (1, '\n');
	header.append (GameObjectTypes::getLocalizedName (obj.getGameObjectType ()));

	if (tooltip) 
	{
		appendUsabilityInformation(header, obj.getNetworkId(), false);
	}
	if (!obj.getLocalizedDescription (desc))
	{
#if PRODUCTION==1
		desc.clear ();
#endif
	}
}

//----------------------------------------------------------------------

void ObjectAttributeManager::populateMergedAttributes (const ClientObject & obj, AttributeVector & mergedAttributeVector, const AttributeVector & av)
{
	mergedAttributeVector = av;

	const TangibleObject * const tangible = obj.asTangibleObject ();

	char buf [256];

	static AttributeVector tmpVector;
	tmpVector.clear ();

	if (!tangible)
	{
		const ClientWaypointObject * const waypoint = dynamic_cast<const ClientWaypointObject *>(&obj);

		if (waypoint)
		{
			StringId const planetStringId("planet_n", waypoint->getPlanetName());

			Vector displayPosition = waypoint->getLocation();

			tmpVector.push_back (AttributePair (SharedObjectAttributes::waypoint_planet, planetStringId.localize()));

			Vector const pos_w(displayPosition.x, 0.0f, displayPosition.z);
			std::string zoneName;
			BuildoutArea const * const ba = GroundZoneManager::getZoneName(waypoint->getPlanetName().c_str(), pos_w, zoneName);

			if (NULL != ba)
			{
				zoneName = waypoint->getPlanetName() + "__" + zoneName;
				StringId const zoneStringId("zone_n", zoneName);

				tmpVector.push_back (AttributePair (SharedObjectAttributes::waypoint_region, zoneStringId.localize()));
				displayPosition = ba->getRelativePosition(displayPosition, true);
			}
			else
			{
				tmpVector.push_back (AttributePair (SharedObjectAttributes::waypoint_region,   waypoint->getRegionName ()));
			}

			{
				_snprintf (buf, sizeof (buf), "%d, %d, %d", static_cast<int>(displayPosition.x), static_cast<int>(displayPosition.y), static_cast<int>(displayPosition.z));
				tmpVector.push_back (AttributePair (SharedObjectAttributes::waypoint_position, Unicode::narrowToWide (buf)));
			}

			{
				Unicode::String strActive;

				if (waypoint->isWaypointActive ())
					strActive = SharedStringIds::yes.localize ();
				else
					strActive = SharedStringIds::no.localize ();

				tmpVector.push_back (AttributePair (SharedObjectAttributes::waypoint_active, strActive));
			}
		}

		tmpVector.insert (tmpVector.end (), mergedAttributeVector.begin (), mergedAttributeVector.end ());
		mergedAttributeVector = tmpVector;
	}
	else
	{
		//-- print the original name

		if (!tangible->isPlayer())
		{
			const StringId & sid = obj.getObjectNameStringId ();
			if (sid.isValid ())
			{
				Unicode::String originalName;

				if (sid.localize (originalName))
				{
					if (originalName != obj.getLocalizedName ())
					{
						if (obj.asCreatureObject ())
							tmpVector.push_back (AttributePair (SharedObjectAttributes::original_name_creature, originalName));
						else
							tmpVector.push_back (AttributePair (SharedObjectAttributes::original_name, originalName));
					}
				}
			}
		}

		CreatureObject const * const creatureObject = tangible->asCreatureObject();
		if (creatureObject != NULL && 
			!creatureObject->isPlayer() && 
			!GameObjectTypes::isTypeOf(creatureObject->getGameObjectType(), SharedObjectTemplate::GOT_vehicle)) // don't show con of vehicles.
		{
			Unicode::String message;
			CuiCombatManager::getConMessage(*creatureObject, message);
			mergedAttributeVector.push_back(AttributePair (SharedObjectAttributes::consider, message));
		}

		// Certifications
		// TODO:  when the weapon system is unified with generic certifications, get rid of the section of code above this
		CreatureObject const * const player = Game::getPlayerCreature ();
		if (player)
		{
			std::vector<std::string> certs;
			tangible->getRequiredCertifications(certs);
			for (std::vector<std::string>::const_iterator i=certs.begin(); i!=certs.end(); ++i)
			{
				Unicode::String strActive;

				if (player->hasCommand (*i))
					strActive = SharedStringIds::yes.localize ();
				else
					strActive = Unicode::narrowToWide("\\#ff0000") + SharedStringIds::no.localize ();

				//TODO:  the string id "weapon_cert_status" should probably be renamed "cert_status"
				tmpVector.push_back (AttributePair (SharedObjectAttributes::weapon_cert_status + ".@cmd_n:" + (*i), strActive));
			}
		}

		if (tmpVector.size ())
		{
			tmpVector.insert (tmpVector.end (), mergedAttributeVector.begin (), mergedAttributeVector.end ());
			mergedAttributeVector = tmpVector;
		}
	}

	/*
	mergedAttributeVector.push_back (AttributePair (std::string ("key_from_table"), Unicode::narrowToWide ("A literal value")));
	mergedAttributeVector.push_back (AttributePair (std::string ("key_from_table"), Unicode::narrowToWide ("@table:a_value_id")));

	mergedAttributeVector.push_back (AttributePair (std::string ("@table:key_id"), Unicode::narrowToWide ("A literal value")));
	mergedAttributeVector.push_back (AttributePair (std::string ("@table:key_id"), Unicode::narrowToWide ("@table:a_value_id")));

	mergedAttributeVector.push_back (AttributePair (std::string ("\"A literal key"), Unicode::narrowToWide ("A literal value")));
	mergedAttributeVector.push_back (AttributePair (std::string ("\"A literal key"), Unicode::narrowToWide ("@table:a_value_id")));


	mergedAttributeVector.push_back (AttributePair (std::string ("category_from_table.key_from_table"), Unicode::narrowToWide ("A literal value")));
	mergedAttributeVector.push_back (AttributePair (std::string ("category_from_table.@table:key_id"), Unicode::narrowToWide ("A literal value")));
	mergedAttributeVector.push_back (AttributePair (std::string ("category_from_table.\"A literal key"), Unicode::narrowToWide ("@table:a_value_id")));

	mergedAttributeVector.push_back (AttributePair (std::string ("@table:category_id.key_from_table"), Unicode::narrowToWide ("A literal value")));
	mergedAttributeVector.push_back (AttributePair (std::string ("@table:category_id.@table:key_id"), Unicode::narrowToWide ("A literal value")));
	mergedAttributeVector.push_back (AttributePair (std::string ("@table:category_id.\"A literal key"), Unicode::narrowToWide ("@table:a_value_id")));

	mergedAttributeVector.push_back (AttributePair (std::string ("\"A literal category.key_from_table"), Unicode::narrowToWide ("A literal value")));
	mergedAttributeVector.push_back (AttributePair (std::string ("\"A literal category.@table:key_id"), Unicode::narrowToWide ("A literal value")));
	mergedAttributeVector.push_back (AttributePair (std::string ("\"A literal category.\"A literal key"), Unicode::narrowToWide ("@table:a_value_id")));
	*/
}

//----------------------------------------------------------------------

void   ObjectAttributeManager::formatDebugInfo             (const Object & object, Unicode::String & str)
{
	str.append (1, '\n');

	char tmp[256];

	const ClientObject * const clientObject = object.asClientObject();

	const Unicode::String yellowColorCode(ClientTextManager::getColorCode(PackedRgb::solidYellow));
	const Unicode::String resetColorCode(ClientTextManager::getResetTagCode());

	if (clientObject)
	{
		snprintf (tmp, sizeof (tmp), "%s%s%s (%s): %s",
			Unicode::wideToNarrow(yellowColorCode).c_str(),
			Unicode::wideToNarrow (clientObject->getLocalizedName ()).c_str (),
			Unicode::wideToNarrow(resetColorCode).c_str(),
			GameObjectTypes::getCanonicalName (clientObject->getGameObjectType ()).c_str (),
			object.getNetworkId ().getValueString ().c_str ());
		str += Unicode::narrowToWide (tmp);
	}
	else
	{
		snprintf (tmp, sizeof (tmp), "%s%s%s: %s", Unicode::wideToNarrow(yellowColorCode).c_str(), object.getDebugName (), Unicode::wideToNarrow(resetColorCode).c_str(), object.getNetworkId ().getValueString().c_str ());
		str += Unicode::narrowToWide (tmp);
	}

	const char * const objectTemplateName = object.getObjectTemplateName ();
	str.append (1, '\n');

	static const Unicode::String template_str_no_template = Unicode::narrowToWide ("NO OBJECT TEMPLATE");
	static const Unicode::String template_str_prefix      = yellowColorCode + Unicode::narrowToWide ("Template: ") + resetColorCode;
	static const Unicode::String app_str_prefix           = yellowColorCode + Unicode::narrowToWide ("Appearance: ") + resetColorCode;
	static const Unicode::String app_str_no_app           = Unicode::narrowToWide ("NO APPEARANCE!");

	str += template_str_prefix + (objectTemplateName ? Unicode::narrowToWide (objectTemplateName) : template_str_no_template);
	str.append (1, '\n');

	const Appearance * const         app = object.getAppearance();
	const AppearanceTemplate * const at  = app ? app->getAppearanceTemplate() : 0;

	if (at)
		str += app_str_prefix + Unicode::narrowToWide (at->getName ());
	else
		str += app_str_no_app;

	str.append (1, '\n');

	const Vector pos = object.getPosition_w ();

	snprintf (tmp, sizeof (tmp), "%sPosition:%s %5.2f,%5.2f,%5.2f\n", Unicode::wideToNarrow(yellowColorCode).c_str(), Unicode::wideToNarrow(resetColorCode).c_str(), pos.x, pos.y, pos.z);

	str += Unicode::narrowToWide (tmp);



	const TangibleObject * const tangible   = clientObject ? clientObject->asTangibleObject() : NULL;
	const CreatureObject * const creature   = clientObject ? clientObject->asCreatureObject() : NULL;

	const bool isVehicle = creature && GameObjectTypes::isTypeOf (creature->getGameObjectType (), SharedObjectTemplate::GOT_vehicle);


	if (creature && !isVehicle)
	{
		snprintf (tmp, sizeof (tmp), "%sHP:%s %3d/%3d/%3d\n", Unicode::wideToNarrow(yellowColorCode).c_str(), Unicode::wideToNarrow(resetColorCode).c_str(), creature->getAttribute (Attributes::Health), creature->getCurrentMaxAttribute (Attributes::Health), creature->getMaxAttribute (Attributes::Health));
		str += Unicode::narrowToWide (tmp);
	}
	else if (tangible)
	{
		snprintf (tmp, sizeof (tmp), "%sHP:%s %3d/%d\n", Unicode::wideToNarrow(yellowColorCode).c_str(), Unicode::wideToNarrow(resetColorCode).c_str(), tangible->getMaxHitPoints () - tangible->getDamageTaken (), tangible->getMaxHitPoints ());
		str += Unicode::narrowToWide (tmp);
	}

	if (tangible)
	{
		snprintf (tmp, sizeof(tmp), "%sConditionBits:%s 0x%08x\n", Unicode::wideToNarrow(yellowColorCode).c_str(), Unicode::wideToNarrow(resetColorCode).c_str(), tangible->getCondition());
		str += Unicode::narrowToWide (tmp);
	}

	ClientMissionObject const * const mission = dynamic_cast<ClientMissionObject const *>(&object);
	if (mission)
	{
		str += Unicode::narrowToWide(Unicode::wideToNarrow(yellowColorCode) + "Internal name:  "+ Unicode::wideToNarrow(resetColorCode) + mission->getTitle().getCanonicalRepresentation() + "\n");
	}
}

//----------------------------------------------------------------------

ObjectAttributeManager::AttributePair * const ObjectAttributeManager::findAttribute       (AttributeVector & av, const std::string & name)
{
	for (AttributeVector::iterator it = av.begin (); it != av.end (); ++it)
	{
		AttributePair & ap = *it;
		if (!_stricmp (ap.first.c_str (), name.c_str ()))
			return &ap;
	}
	return 0;
}

//----------------------------------------------------------------------

void ObjectAttributeManager::update(float deltaTime)
{
	if (ms_attributeThrottle.updateZero(deltaTime))
	{
		static const uint32 hash_getAttributesBatch = Crc::normalizeAndCalculate("getAttributesBatch");

		if (!ms_attributeRequestSet.empty() || !ms_watchedObjects.empty())
		{
			std::stringstream attributes;

			// -- requested
			if (!ms_attributeRequestSet.empty())
			{
				for(std::set<NetworkId>::const_iterator i = ms_attributeRequestSet.begin(); i != ms_attributeRequestSet.end(); ++i)
				{
					NetworkId const & id = *i;

					int revision = s_initialClientRevision;
					AttributeMap::const_iterator itInfo = s_attribs.find(id);
					if (itInfo != s_attribs.end())
					{
						AttributeInfo const & info = itInfo->second;
						revision = info.revision;
					}

					attributes << id.getValueString() << " " << revision << " ";
				}

				ms_attributeRequestSet.clear();
				ms_attributeTimeout.reset();
			}

			// -- watched
			if (!ms_watchedObjects.empty())
			{
				for(WidgetWatcherMap::const_iterator i = ms_watchedObjects.begin(); i != ms_watchedObjects.end(); ++i)
				{
					NetworkId const & id = i->second;
					attributes << id.getValueString() << " " << s_initialClientRevision << " ";
				}
			}

			if (!Game::getSinglePlayer())
			{
				IGNORE_RETURN(ClientCommandQueue::enqueueCommand(hash_getAttributesBatch, NetworkId::cms_invalid, Unicode::narrowToWide(attributes.str())));
			}
		}
	}
	else if (ms_attributeTimeout.updateZero(deltaTime))
	{
		ms_outstandingAttributeRequestSet.clear();
	}
}

//----------------------------------------------------------------------

void ObjectAttributeManager::setObjectAttributesDirty(NetworkId const & id)
{
	AttributeMap::iterator it = s_attribs.find(id);
	if (it != s_attribs.end())
	{
		AttributeInfo & info = (*it).second;
		info.lastFrameUpdated = 0;
		info.revision = s_initialClientRevision;
		requestUpdate(id);
	}
}

//----------------------------------------------------------------------

bool ObjectAttributeManager::hasExtendedTooltipInformation(NetworkId const & id)
{
	bool hasTooltips = false;

	AttributeMap::const_iterator it = s_attribs.find(id);
	if (it != s_attribs.end())
	{
		AttributeInfo const & info = it->second;
		hasTooltips = info.hasTooltips;
	}

	return hasTooltips;
}

//----------------------------------------------------------------------

void ObjectAttributeManager::getExtendedTooltipInformation(NetworkId const & objId, Unicode::String & tooltip)
{
	if (objId != NetworkId::cms_invalid)
	{
		static Unicode::String header;
		static Unicode::String desc;
		static Unicode::String attribs;

		header.clear();
		desc.clear();
		attribs.clear();

		IGNORE_RETURN(ObjectAttributeManager::formatDescription(objId, header, desc, attribs, false, true));

		IGNORE_RETURN(tooltip.append(header));
		IGNORE_RETURN(tooltip.append(1, '\n'));
		IGNORE_RETURN(tooltip.append(attribs));
	}
}

//----------------------------------------------------------------------

void ObjectAttributeManager::startWatching(CuiMediator const * const widget, NetworkId const & id)
{
	ms_watchedObjects[ widget ] = id;
}

//----------------------------------------------------------------------

void ObjectAttributeManager::stopWatching(CuiMediator const * const widget)
{
	WidgetWatcherMap::iterator it = ms_watchedObjects.find(widget);
	if (it != ms_watchedObjects.end())
	{
		ms_attributeRequestSet.insert(it->second);
		ms_watchedObjects.erase(it);
	}
}

//----------------------------------------------------------------------

void ObjectAttributeManager::appendUsabilityInformation(Unicode::String & str, NetworkId const & id, bool const verbose)
{
	Object const * const object = NetworkIdManager::getObjectById(id);
	ClientObject const * const client = object ? object->asClientObject() : NULL;
	TangibleObject const * const tangible = client ? client->asTangibleObject() : NULL;

	CreatureObject const * const player = Game::getPlayerCreature();
	bool const isAUsable         = ClientObjectUsabilityManager::isUsable(tangible);
	bool const isUsableByPlayer  = ClientObjectUsabilityManager::canUse(player, tangible);
	bool const isAWearable       = ClientObjectUsabilityManager::isWearable(tangible);
	bool const canBeWornByPlayer = ClientObjectUsabilityManager::canWear(player, tangible);

	if(tangible)
	{
		if(tangible->hasCondition(TangibleObject::C_locked))
		{
			str.append(1, '\n');
			str.append(redColor);
			str.append(restrictedAccess.localize());
		}
	}

	if (isAWearable && canBeWornByPlayer)
	{
		str.append(1, '\n');
		str.append(greenColor);
		str.append(verbose ? canWearDescId.localize() : canWearId.localize());
	}
	else if(isAUsable && isUsableByPlayer)
	{
		str.append(1, '\n');
		str.append(greenColor);
		str.append(verbose ? canUseDescId.localize() : canUseId.localize());
	}
	else if(isAWearable && !canBeWornByPlayer)
	{
		str.append(1, '\n');
		str.append(redColor);
		str.append(verbose ? canNotWearDescId.localize() : canNotWearId.localize());
	}
	else if(isAUsable && !isUsableByPlayer)
	{
		str.append(1, '\n');
		str.append(redColor);
		str.append(verbose ? canNotUseDescId.localize() : canNotUseId.localize());
	}

	if(tangible)
	{
		if(tangible->getGameObjectType() == SharedObjectTemplate::GOT_misc_appearance_only)
		{
			str.append(1, '\n');
			str.append(1, '\n');
			str.append(purpleColor);
			str.append(appearanceOnly.localize());
		}
		else if (tangible->getGameObjectType() == SharedObjectTemplate::GOT_misc_appearance_only_invisible)
		{
			str.append(1, '\n');
			str.append(1, '\n');
			str.append(purpleColor);
			str.append(appearanceOnlyInvis.localize());
		}


	}
}

//----------------------------------------------------------------------

bool ObjectAttributeManager::isNoTrade(NetworkId const & id)
{
	bool noTrade = false;
	
	AttributeMap::const_iterator it = s_attribs.find(id);
	if (it != s_attribs.end())
	{
		AttributeInfo const & info = it->second;
		noTrade = info.noTrade;
	}
	
	return noTrade;
}

//----------------------------------------------------------------------

bool ObjectAttributeManager::isNoTrade(std::string const & staticItemName)
{
	bool noTrade = false;
	
	StaticItemAttributeMap::const_iterator it = s_staticItemsAttribs.find(staticItemName);
	if (it != s_staticItemsAttribs.end())
	{
		AttributeInfo const & info = it->second;
		noTrade = info.noTrade;
	}
	
	return noTrade;
}

//----------------------------------------------------------------------

bool ObjectAttributeManager::isNoTradeShared(NetworkId const & id)
{
	bool noTradeShared = false;

	AttributeMap::const_iterator it = s_attribs.find(id);
	if (it != s_attribs.end())
	{
		AttributeInfo const & info = it->second;
		noTradeShared = info.noTradeShared;
	}

	return noTradeShared;
}

//----------------------------------------------------------------------

bool ObjectAttributeManager::isNoTradeShared(std::string const & staticItemName)
{
	bool noTradeShared = false;

	StaticItemAttributeMap::const_iterator it = s_staticItemsAttribs.find(staticItemName);
	if (it != s_staticItemsAttribs.end())
	{
		AttributeInfo const & info = it->second;
		noTradeShared = info.noTradeShared;
	}

	return noTradeShared;
}

//----------------------------------------------------------------------

bool ObjectAttributeManager::isUnique(NetworkId const & id)
{
	bool unique = false;

	AttributeMap::const_iterator it = s_attribs.find(id);
	if (it != s_attribs.end())
	{
		AttributeInfo const & info = it->second;
		unique = info.unique;
	}

	return unique;
}

//----------------------------------------------------------------------

bool ObjectAttributeManager::isUnique(std::string const & staticItemName)
{
	bool unique = false;

	StaticItemAttributeMap::const_iterator it = s_staticItemsAttribs.find(staticItemName);
	if (it != s_staticItemsAttribs.end())
	{
		AttributeInfo const & info = it->second;
		unique = info.unique;
	}

	return unique;
}


//----------------------------------------------------------------------

int ObjectAttributeManager::getTier(NetworkId const & id)
{
	int tier = -1;
	
	AttributeMap::const_iterator it = s_attribs.find(id);
	if (it != s_attribs.end())
	{
		AttributeInfo const & info = it->second;
		tier = info.tier;
		if (tier <= 0 || tier >= s_tierArraySize)
		{
			if(info.hasCrafter)
				tier = 3;
			else
				tier = 2;
		}
	}
	
	return tier;
}

//----------------------------------------------------------------------

int ObjectAttributeManager::getTier(std::string const & staticItemName)
{
	int tier = -1;
	
	StaticItemAttributeMap::const_iterator it = s_staticItemsAttribs.find(staticItemName);
	if (it != s_staticItemsAttribs.end())
	{
		AttributeInfo const & info = it->second;
		tier = info.tier;
		if (tier <= 0 || tier >= s_tierArraySize)
		{
			if(info.hasCrafter)
				tier = 3;
			else
				tier = 2;
		}
	}
	
	return tier;
}

//----------------------------------------------------------------------

StringId const & ObjectAttributeManager::getTierStringId(int tier)
{
	tier = clamp(0, tier, s_tierArraySize - 1);
	return s_tierIdArray[tier];
}
//----------------------------------------------------------------------

StringId const & ObjectAttributeManager::getTierDescStringId(int tier)
{
	tier = clamp(0, tier, s_tierArraySize - 1);
	return s_tierIdDescArray[tier];
}

//----------------------------------------------------------------------

UIColor const & ObjectAttributeManager::getTierColor(int tier)
{
	tier = clamp(0, tier, s_tierArraySize - 1);
	return s_tierColorArray[tier];;
}


//======================================================================
