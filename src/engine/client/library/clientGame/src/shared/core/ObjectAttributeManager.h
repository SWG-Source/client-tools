//======================================================================
//
// ObjectAttributeManager.h
// copyright (c) 2002 Sony Online Entertainment
//
//======================================================================

#ifndef INCLUDED_ObjectAttributeManager_H
#define INCLUDED_ObjectAttributeManager_H

//======================================================================

class ClientObject;
class CuiMediator;
class NetworkId;
class Object;
class StringId;
struct UIColor;

//----------------------------------------------------------------------

class ObjectAttributeManager
{
public:

	typedef std::pair<std::string, Unicode::String> AttributePair;
	typedef stdvector<AttributePair>::fwd           AttributeVector;

	struct Messages
	{
		struct AttributesChanged
		{
			typedef NetworkId Payload;
		};

		struct StaticItemAttributesChanged
		{
			typedef std::string Payload;
		};
	};

	static void   install                     ();
	static void   remove                      ();
	static void   reset                       ();
	static void   update                      (float elapsedTime);

	static void   updateAttributes            (const NetworkId & id, const AttributeVector & av, int const revision);
	static void   updateAttributes            (std::string const & staticItemName, const AttributeVector & av, int const revision);
	static void   populateMergedAttributes    (const ClientObject & obj, AttributeVector & mergedAttributeVector, const AttributeVector & av);
	static void   requestUpdate               (const NetworkId & id, bool updateIfAttributesAreEmpty = false);
	static void   requestUpdate               (std::string const & staticItemName, bool updateIfAttributesAreEmpty = false);
	static bool   hasAttributesForId          (const NetworkId & id);
	static bool   hasAttributesForStaticItemName (std::string const & staticItemName);

	static bool   getAttributes               (const NetworkId & id, AttributeVector & av, bool forceUpdate = true, bool updateIfAttributesAreEmpty = false);
	static bool   getAttributesIfNewer        (const NetworkId & id, AttributeVector & av, int & frame, bool forceUpdate, bool updateIfAttributesAreEmpty);
	static bool   getAttributes               (std::string const & staticItemName, AttributeVector & av, bool forceUpdate = true, bool updateIfAttributesAreEmpty = false);
	static bool   getAttributesIfNewer        (std::string const & staticItemName, AttributeVector & av, int & frame, bool forceUpdate, bool updateIfAttributesAreEmpty);

	static void   formatAttributes            (const AttributeVector & av, Unicode::String & str, Unicode::String * categoryDisplayValue, Unicode::String * keyDisplayValue, bool minimal, bool const tooltips = false);
	static bool   formatAttributes            (const NetworkId & id, Unicode::String & str, bool minimal, bool const tooltips = false);
	static bool   formatAttributesIfNewer     (const NetworkId & id, Unicode::String & str, bool minimal, int & frame, bool const tooltips);
	static bool   formatDescription           (const NetworkId & id, Unicode::String & header, Unicode::String & desc, Unicode::String & attribs, bool minimalAttribs, bool const tooltips = false);
	static bool   formatDescriptionIfNewer    (const NetworkId & id, Unicode::String & header, Unicode::String & desc, Unicode::String & attribs, bool minimalAttribs, int & frame, bool const tooltips = false);

	static void   formatHeaderAndDesc         (const ClientObject & obj, Unicode::String & header, Unicode::String & desc, bool tooltip = false);

	static void   formatDebugInfo             (const Object & obj, Unicode::String & str);

	static AttributePair * const findAttribute  (AttributeVector & av, const std::string & name);

	static void    setObjectAttributesDirty     (NetworkId const & id);

	static bool hasExtendedTooltipInformation(NetworkId const & id);
	static void getExtendedTooltipInformation(NetworkId const & id, Unicode::String & tooltip);

	// support real-time updates.
	static void startWatching(CuiMediator const * const widget, NetworkId const & id);
	static void stopWatching(CuiMediator const * const widget);

	// 
	static void appendUsabilityInformation(Unicode::String & str, NetworkId const & id, bool verbose);

	// no trade stuff.
	static bool isNoTrade(NetworkId const & id);
	static bool isNoTrade(std::string const & staticItemName);
	static bool isNoTradeShared(NetworkId const & id);
	static bool isNoTradeShared(std::string const & staticItemName);
	// unique stuff
	static bool isUnique(NetworkId const & id);
	static bool isUnique(std::string const & staticItemName);

	// tier
	static int getTier(NetworkId const & id);
	static int getTier(std::string const & staticItemName);
	static StringId const & getTierStringId(int const tier);
	static StringId const & getTierDescStringId(int const tier);
	static UIColor const & getTierColor(int const tier);

public:
	static const std::string    ms_nameStringTable;
	static const std::string    ms_descStringTable;
};

//======================================================================

#endif
