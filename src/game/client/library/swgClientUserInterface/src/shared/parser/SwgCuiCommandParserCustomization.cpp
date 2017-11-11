// ======================================================================
//
// SwgCuiCommandParserCustomization.cpp
// Copyright 2003 Sony Online Entertainment, Inc.
// All Rights Reserved.
//
// ======================================================================

#include "swgClientUserInterface/FirstSwgClientUserInterface.h"
#include "swgClientUserInterface/SwgCuiCommandParserCustomization.h"

#include "clientGame/Game.h"
#include "clientGame/NetworkScene.h"
#include "clientSkeletalAnimation/SkeletalAppearance2.h"
#include "clientUserInterface/CuiCombatManager.h"
#include "sharedObject/CachedNetworkId.h"
#include "sharedObject/CustomizationData.h"
#include "sharedObject/CustomizationDataProperty.h"
#include "sharedObject/CustomizationVariable.h"
#include "sharedObject/NetworkIdManager.h"
#include "sharedObject/RangedIntCustomizationVariable.h"

#include "Unicode.h"

#include <map>
#include <vector>

// ======================================================================

namespace SwgCuiCommandParserCustomizationNamespace
{
	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

	typedef CommandParser::StringVector_t  StringVector;
	typedef CommandParser::String_t        String;

	typedef std::map<std::string, CustomizationVariable*> VariableMap;	

	class WearableParser: public CommandParser
	{
	public:

		WearableParser();
		virtual ~WearableParser();

		virtual bool performParsing(NetworkId const &userId, StringVector_t const &argv, String_t const &originalCommand, String_t &result, CommandParser const *node);

	private:

		// Disabled.
		WearableParser(WearableParser const&); //lint -esym(754, WearableParser::WearableParser) // unreferenced // required.
		WearableParser &operator =(WearableParser const&);
	};

	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

	char const *const cs_listVariablesCommand   = "listVariables";
	char const *const cs_listWearablesCommand   = "listWearables";
	char const *const cs_listAttachmentsCommand = "listAttachments";
	char const *const cs_setIntCommand          = "setInt";

	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

	CommandParser::CmdInfo const s_commands[] =
		{
			{cs_listVariablesCommand,   0, "[<object id>]",                    "list all variables on an object"},
			{cs_listWearablesCommand,   0, "[<object id>]",                    "list all wearables on an object"},
			{cs_listAttachmentsCommand, 0, "[<object id>]",                    "list all attachments on an object"},
			{cs_setIntCommand,          2, "<variable> <value> [<object id>]", "set a variable's integer value"},
			{"",                        0, "",                                 ""}
		};

	CommandParser::CmdInfo const s_wearableCommands[] =
		{
			{cs_listVariablesCommand,   1, "<wearable index> [<object id>]",                    "list all variables on a wearable"},
			{cs_setIntCommand,          3, "<wearable index> <variable> <value> [<object id>]", "set a wearable variable's integer value"},
			{"",                        0, "",                                 ""}
		};

	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

	CustomizationData *fetchObjectCustomizationData(Object &object, String &result);
	bool               getFocusObject(StringVector const &argv, int optionalObjectIdIndex, Object *&object, String &result);
	Object            *getObjectWearableByIndex(Object &object, int index, String &result);
	void               buildVariableMapCallback(std::string const &fullVariablePathName, CustomizationVariable *customizationVariable, void *context);
	int                getLongestVariableName(VariableMap const &variableMap);
	void               addObjectVariablesToString(Object &object, String &result);
	void               addObjectWearablesToString(Object &object, String &result);
	void               setObjectInt(Object &object, std::string const &variableName, int value, String &result);

	bool               doListVariables(StringVector const &argv, String &result);
	bool               doListWearables(StringVector const &argv, String &result);
	bool               doSetInt(StringVector const &argv, String &result);

	bool               doWearableListVariables(StringVector const &argv, String &result);
	bool               doWearableSetInt(StringVector const &argv, String &result);

}

using namespace SwgCuiCommandParserCustomizationNamespace;

// ======================================================================
// namespace SwgCuiCommandParserCustomizationNamespace
// ======================================================================

CustomizationData *SwgCuiCommandParserCustomizationNamespace::fetchObjectCustomizationData(Object &object, String &result)
{
	//-- Get customization data for the object.
	CustomizationDataProperty *cdProperty = safe_cast<CustomizationDataProperty*>(object.getProperty(CustomizationDataProperty::getClassPropertyId()));
	if (!cdProperty)
	{
		//-- No customizations on this object.
		char buffer[1024];
		snprintf(buffer, sizeof(buffer) - 1, "Object id [%s], template [%s] has no customization data.\n", object.getNetworkId().getValueString().c_str(), object.getObjectTemplateName());
		buffer[sizeof(buffer) - 1] = '\0';

		result = Unicode::narrowToWide(buffer);
		return 0;
	}

	return cdProperty->fetchCustomizationData();
}

// ----------------------------------------------------------------------

bool SwgCuiCommandParserCustomizationNamespace::getFocusObject(StringVector const &argv, int optionalObjectIdIndex, Object *&object, String &result)
{
	//-- Check if the object id is given.
	if ((optionalObjectIdIndex >= 0) && (static_cast<int>(argv.size()) > optionalObjectIdIndex))
	{
		//-- Make sure id value is parsable as an int.
		std::string const narrowIdString = Unicode::wideToNarrow(argv[static_cast<size_t>(optionalObjectIdIndex)]);

		unsigned long idValue;
		int const count = sscanf(narrowIdString.c_str(), "%ul", &idValue);

		if (count != 1)
		{
			//-- Failed to parse number.
			char buffer[256];
			snprintf(buffer, sizeof(buffer) - 1, "Could not make an object id from argument [%s], aborting command.\n", narrowIdString.c_str());
			buffer[sizeof(buffer) - 1] = '\0';

			result = Unicode::narrowToWide(buffer);
			return false;
		}

		//-- Make network id.
		NetworkId  id(narrowIdString);
		
		object = NetworkIdManager::getObjectById(id);
		if (!object)
		{
			//-- Failed to parse number.
			char buffer[256];
			snprintf(buffer, sizeof(buffer) - 1, "Failed to find object for id [%s], aborting command.\n", narrowIdString.c_str());
			buffer[sizeof(buffer) - 1] = '\0';

			result = Unicode::narrowToWide(buffer);
			return false;
		}

		//-- Success.
		return true;
	}

	//-- Get the Object instance for the target on which we'll play the animation.
	//   This will be either the active selection, or if none, the player.
	object = CuiCombatManager::getLookAtTarget().getObject();
	if (!object)
	{
		// Get the scene.
		NetworkScene *networkScene = dynamic_cast<NetworkScene*>(Game::getScene());
		if (!networkScene)
		{
			DEBUG_WARNING(true, ("No network scene.\n"));
			result = Unicode::narrowToWide("No look at target and no network scene, aborting command.\n");

			return false;
		}

		// Get the player.
		object = networkScene->getPlayer();
		if (!object)
		{
			result = Unicode::narrowToWide("Player object is NULL, aborting command.\n");
			return false;
		}
	}

	return true;
}

// ----------------------------------------------------------------------

Object *SwgCuiCommandParserCustomizationNamespace::getObjectWearableByIndex(Object &object, int index, String &result)
{
	//-- Get the skeletal appearance 2 for the object.
	SkeletalAppearance2 *const appearance = (object.getAppearance() ? object.getAppearance()->asSkeletalAppearance2() : 0);
	if (!appearance)
	{
		//-- No customizations on this object.
		char buffer[1024];
		snprintf(buffer, sizeof(buffer) - 1, "Object id [%s], template [%s] does not have a skeletal appearance.\n", object.getNetworkId().getValueString().c_str(), object.getObjectTemplateName());
		buffer[sizeof(buffer) - 1] = '\0';

		result = Unicode::narrowToWide(buffer);
		return 0;
	}

	//-- Check if index is in range.
	int const wearableCount = appearance->getWearableCount();
	if ((index < 0) || (index >= wearableCount))
	{
		//-- Print out header info.
		char buffer[1024];
		snprintf(buffer, sizeof(buffer) - 1, "Wearable index [%d] out of valid range [0..%d] for object id [%s] template [%s].\n", index, wearableCount - 1, object.getNetworkId().getValueString().c_str(), object.getObjectTemplateName());
		buffer[sizeof(buffer) - 1] = '\0';

		result = Unicode::narrowToWide(buffer);
		return 0;
	}

	SkeletalAppearance2 *const wearable = appearance->getWearableAppearance(index);
	return wearable ? wearable->getOwner() : 0;
}

// ----------------------------------------------------------------------

void SwgCuiCommandParserCustomizationNamespace::buildVariableMapCallback(std::string const &fullVariablePathName, CustomizationVariable *customizationVariable, void *context)
{
	NOT_NULL(customizationVariable);
	NOT_NULL(context);

	VariableMap *const variableMap = static_cast<VariableMap*>(context);
	std::pair<VariableMap::const_iterator, bool> result = variableMap->insert(VariableMap::value_type(fullVariablePathName, customizationVariable));

	DEBUG_WARNING(!result.second, ("Failed to insert variable name [%s] into map, does it exist multiple times?", fullVariablePathName.c_str()));
	UNREF(result);
}

// ----------------------------------------------------------------------

int SwgCuiCommandParserCustomizationNamespace::getLongestVariableName(VariableMap const &variableMap)
{
	int maxLength = 0;
	
	VariableMap::const_iterator const endIt = variableMap.end();
	for (VariableMap::const_iterator it = variableMap.begin(); it != endIt; ++it)
	{
		int const nameLength = static_cast<int>(it->first.length());
		maxLength = std::max(nameLength, maxLength);
	}

	return maxLength;
}

// ----------------------------------------------------------------------

void SwgCuiCommandParserCustomizationNamespace::addObjectVariablesToString(Object &object, String &result)
{
	//-- Get customization data for the object.
	CustomizationData *const customizationData = fetchObjectCustomizationData(object, result);
	if (!customizationData)
		return;

	//-- Collect variable entries.
	VariableMap  variableMap;
	customizationData->iterateOverVariables(buildVariableMapCallback, &variableMap, false);

	{
		//-- Print out header info.
		char buffer[1024];
		snprintf(buffer, sizeof(buffer) - 1, "There are [%d] customization variables for object id [%s], template [%s]:\n", static_cast<int>(variableMap.size()), object.getNetworkId().getValueString().c_str(), object.getObjectTemplateName());
		buffer[sizeof(buffer) - 1] = '\0';

		result = Unicode::narrowToWide(buffer);
	}

	//-- Get length of longest variable name, used for name column justification.
	int const longestNameLength = getLongestVariableName(variableMap);
	int const nameColumnLength  = longestNameLength + 2;
	String    leftBorder        = Unicode::narrowToWide("  ");

	//-- List all the names.
	VariableMap::iterator const endIt = variableMap.end();
	for (VariableMap::iterator it = variableMap.begin(); it != endIt; ++it)
	{
		// Add left border.
		IGNORE_RETURN(result.append(leftBorder));

		// Add variable name.
		IGNORE_RETURN(result.append(Unicode::narrowToWide(it->first)));

		// Justify name column.
		int const spaceCount = nameColumnLength - static_cast<int>(it->first.length());
		for (int i = 0; i < spaceCount; ++i)
			IGNORE_RETURN(result.append(Unicode::narrowToWide(" ")));

		// Add value string.
		CustomizationVariable const *const customizationVariable = it->second;
		NOT_NULL(customizationVariable);
#ifdef _DEBUG
		IGNORE_RETURN(result.append(Unicode::narrowToWide(customizationVariable->debugToString())));
#endif

		// Add end of line.
		IGNORE_RETURN(result.append(Unicode::narrowToWide("\n")));
	}

	//-- Release local references.
	customizationData->release();
}

// ----------------------------------------------------------------------

void SwgCuiCommandParserCustomizationNamespace::addObjectWearablesToString(Object &object, String &result)
{
	//-- Get the skeletal appearance 2 for the object.
	SkeletalAppearance2 *const appearance = (object.getAppearance() ? object.getAppearance()->asSkeletalAppearance2() : 0);
	if (!appearance)
	{
		//-- No customizations on this object.
		char buffer[1024];
		snprintf(buffer, sizeof(buffer) - 1, "Object id [%s], template [%s] does not have a skeletal appearance.\n", object.getNetworkId().getValueString().c_str(), object.getObjectTemplateName());
		buffer[sizeof(buffer) - 1] = '\0';

		result = Unicode::narrowToWide(buffer);
		return;
	}

	//-- List all the wearables associated with the skeletal appearance.
	int const wearableCount = appearance->getWearableCount();

	// Print header.
	{
		//-- Print out header info.
		char buffer[1024];
		snprintf(buffer, sizeof(buffer) - 1, "There are [%d] wearables for object id [%s], template [%s]:\n", wearableCount, object.getNetworkId().getValueString().c_str(), object.getObjectTemplateName());
		buffer[sizeof(buffer) - 1] = '\0';

		result = Unicode::narrowToWide(buffer);
	}

	// List each wearable.
	for (int i = 0; i < wearableCount; ++i)
	{
		SkeletalAppearance2 *const wearable = appearance->getWearableAppearance(i);
		Object *const wearableOwner = wearable ? wearable->getOwner() : 0;

		if (wearableOwner && wearable)
		{
			char buffer[1024];
			snprintf(buffer, sizeof(buffer) - 1, "  wearable %d:\n    oid [%s]\n    otemplate [%s]\n    atemplate [%s]\n", i, wearableOwner->getNetworkId().getValueString().c_str(), wearableOwner->getObjectTemplateName(), wearable->getAppearanceTemplateName());
			buffer[sizeof(buffer) - 1] = '\0';

			IGNORE_RETURN(result.append(Unicode::narrowToWide(buffer)));
		}		
	}
}

// ----------------------------------------------------------------------

void SwgCuiCommandParserCustomizationNamespace::setObjectInt(Object &object, std::string const &variableName, int value, String &result)
{
	//-- Get customization data for the object.
	CustomizationData *const customizationData = fetchObjectCustomizationData(object, result);
	if (!customizationData)
		return;

	//-- Get variable.
	RangedIntCustomizationVariable *const variable = dynamic_cast<RangedIntCustomizationVariable*>(customizationData->findVariable(variableName));
	if (!variable)
	{
		//-- Print out header info.
		char buffer[1024];
		snprintf(buffer, sizeof(buffer) - 1, "Object id [%s], template [%s] does not have variable [%s]\n", object.getNetworkId().getValueString().c_str(), object.getObjectTemplateName(), variableName.c_str());
		buffer[sizeof(buffer) - 1] = '\0';

		result = Unicode::narrowToWide(buffer);
		return;
	}

	//-- Set value.
	variable->setValue(value);

	//-- Release local references.
	customizationData->release();

	//-- Print feedback.
	{
		//-- Print out header info.
		char buffer[1024];
		snprintf(buffer, sizeof(buffer) - 1, "Object id [%s], set variable [%s] to [%d]\n", object.getNetworkId().getValueString().c_str(), variableName.c_str(), value);
		buffer[sizeof(buffer) - 1] = '\0';

		result = Unicode::narrowToWide(buffer);
	}
}

// ----------------------------------------------------------------------

bool SwgCuiCommandParserCustomizationNamespace::doListVariables(StringVector const &argv, String &result)
{
	//-- Get the object for the command.
	Object *object = 0;
	if (!getFocusObject(argv, 1, object, result))
		return false;

	addObjectVariablesToString(*object, result);
	return true;
}

// ----------------------------------------------------------------------

bool SwgCuiCommandParserCustomizationNamespace::doListWearables(StringVector const &argv, String &result)
{
	//-- Get the object for the command.
	Object *object = 0;
	if (!getFocusObject(argv, 1, object, result))
		return false;

	addObjectWearablesToString(*object, result);
	return true;
}

// ----------------------------------------------------------------------

bool SwgCuiCommandParserCustomizationNamespace::doSetInt(StringVector const &argv, String &result)
{
	//-- Validate parameter count.
	if (argv.size() < 3)
	{
		result = Unicode::narrowToWide("setInt requires at a minimum the <variable name> and <int value> parameter.\n");
		return true;
	}

	//-- Get parameters.
	std::string const variableName = Unicode::wideToNarrow(argv[1]);

	Object *object = 0;
	if (!getFocusObject(argv, 3, object, result) || !object)
		return false;

	setObjectInt(*object, variableName, Unicode::toInt(argv[2]), result);
	return true;
}

// ----------------------------------------------------------------------

bool SwgCuiCommandParserCustomizationNamespace::doWearableListVariables(StringVector const &argv, String &result)
{
	//-- Get the object for the command.
	Object *object = 0;
	if (!getFocusObject(argv, 2, object, result))
		return false;

	//-- Get the nth wearable from it.
	Object *const wearableObject = getObjectWearableByIndex(*object, Unicode::toInt(argv[1]), result);
	if (!wearableObject)
		return true;

	addObjectVariablesToString(*wearableObject, result);
	return true;
}

// ----------------------------------------------------------------------

bool SwgCuiCommandParserCustomizationNamespace::doWearableSetInt(StringVector const &argv, String &result)
{
	//-- Get parameters.
	std::string const variableName = Unicode::wideToNarrow(argv[2]);

	Object *object = 0;
	if (!getFocusObject(argv, 4, object, result) || !object)
		return false;

	//-- Get the nth wearable from it.
	Object *const wearableObject = getObjectWearableByIndex(*object, Unicode::toInt(argv[1]), result);
	if (!wearableObject)
		return true;

	setObjectInt(*wearableObject, variableName, Unicode::toInt(argv[3]), result);
	return true;
}

// ======================================================================
// class SwgCuiCommandParserCustomizationNamespace::WearableParser
// ======================================================================

SwgCuiCommandParserCustomizationNamespace::WearableParser::WearableParser() :
	CommandParser("wearable", 0, "...", "commands applying to wearables", 0)
{
	createDelegateCommands(s_wearableCommands);
}

// ----------------------------------------------------------------------

SwgCuiCommandParserCustomizationNamespace::WearableParser::~WearableParser()
{
}

// ----------------------------------------------------------------------

bool SwgCuiCommandParserCustomizationNamespace::WearableParser::performParsing(NetworkId const &userId, StringVector_t const &argv, String_t const &originalCommand, String_t &result, CommandParser const *node)
{
	UNREF(userId);
	UNREF(originalCommand);
	UNREF(result);
	UNREF(node);

	//-- Check for empty args.
	if (argv.empty())
	{
		DEBUG_WARNING(true, ("WearableParser::performParsing(): argv is empty, unexpected."));
		return false;
	}

	//-- Check which command is getting run.
	if (isCommand(argv[0], cs_listVariablesCommand))
		return doWearableListVariables(argv, result);
	else if (isCommand(argv[0], cs_setIntCommand))
		return doWearableSetInt(argv, result);
	else
	{
		char buffer[256];

		snprintf(buffer, sizeof(buffer) - 1, "Unrecognized command [%s], ignoring.\n", Unicode::wideToNarrow(argv[0]).c_str());
		buffer[sizeof(buffer) - 1] = 0;

		result += Unicode::narrowToWide(buffer);
		return false;
	}
}

// ======================================================================
// class SwgCuiCommandParserCustomization: PUBLIC
// ======================================================================

SwgCuiCommandParserCustomization::SwgCuiCommandParserCustomization() :
	CommandParser("customization", 0, "...", "appearance customization commands", 0)
{
	createDelegateCommands(s_commands);
	IGNORE_RETURN(addSubCommand(new WearableParser())); //lint !e1524 // new in ctor with no explicit dtor // It's okay, it is cleaned up internally.
}

// ----------------------------------------------------------------------

SwgCuiCommandParserCustomization::~SwgCuiCommandParserCustomization()
{
}

// ----------------------------------------------------------------------

bool SwgCuiCommandParserCustomization::performParsing(NetworkId const &userId, StringVector_t const &argv, String_t const &originalCommand, String_t &result, CommandParser const *node)
{
	UNREF(userId);
	UNREF(originalCommand);
	UNREF(result);
	UNREF(node);

	//-- Check for empty args.
	if (argv.empty())
	{
		DEBUG_WARNING(true, ("SwgCuiCommandParserCustomization::performParsing(): argv is empty, unexpected."));
		return false;
	}

	//-- Check which command is getting run.
	if (isCommand(argv[0], cs_listVariablesCommand))
		return doListVariables(argv, result);
	else if (isCommand(argv[0], cs_listWearablesCommand))
		return doListWearables(argv, result);
	else if (isCommand(argv[0], cs_setIntCommand))
		return doSetInt(argv, result);
	else
	{
		char buffer[256];

		snprintf(buffer, sizeof(buffer) - 1, "Unrecognized command [%s], ignoring.\n", Unicode::wideToNarrow(argv[0]).c_str());
		buffer[sizeof(buffer) - 1] = 0;

		result += Unicode::narrowToWide(buffer);
		return false;
	}
}

// ======================================================================
