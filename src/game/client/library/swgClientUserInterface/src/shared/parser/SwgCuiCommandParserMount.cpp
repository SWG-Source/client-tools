// ======================================================================
//
// SwgCuiCommandParserMount.cpp
// Copyright 2003 Sony Online Entertainment, Inc.
// All Rights Reserved.
//
// ======================================================================

#include "swgClientUserInterface/FirstSwgClientUserInterface.h"
#include "swgClientUserInterface/SwgCuiCommandParserMount.h"

#include "clientGame/ContainerInterface.h"
#include "clientGame/CreatureObject.h"
#include "clientGame/Game.h"
#include "clientGame/NetworkScene.h"
#include "clientGame/RemoteCreatureController.h"
#include "clientGraphics/RenderWorld.h"
#include "clientSkeletalAnimation/SkeletalAppearance2.h"
#include "clientUserInterface/CuiCombatManager.h"
#include "pcre/pcre.h"
#include "sharedFoundation/CrcString.h"
#include "sharedFoundation/ConstCharCrcString.h"
#include "sharedGame/MountValidScaleRangeTable.h"
#include "sharedGame/SharedCreatureObjectTemplate.h"
#include "sharedObject/AppearanceTemplateList.h"
#include "sharedObject/CachedNetworkId.h"
#include "sharedObject/NetworkIdManager.h"
#include "sharedObject/ObjectTemplateList.h"
#include "sharedObject/SlottedContainmentProperty.h"
#include "sharedObject/SlotIdManager.h"
#include "sharedRegex/RegexServices.h"
#include "swgSharedUtility/States.def"

#include "Unicode.h"
#include "UnicodeUtils.h"

#include <map>
#include <vector>

// ======================================================================

namespace SwgCuiCommandParserMountNamespace
{
	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

	typedef CommandParser::StringVector_t  StringVector;
	typedef CommandParser::String_t        String;

	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

	char const *const cs_createAt      = "createAt";
	char const *const cs_mountCreature = "mountCreature";
	char const *const cs_dismountRider = "dismountRider";

	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

	CommandParser::CmdInfo const cs_commands[] =
		{
			{cs_createAt,   2, "<object template name> <creature name regex> [<scale fraction, 0..1>]", "create a mount, default scale fraction=0.5"},
			{cs_mountCreature, 0, "[mount <mount creature id>] [rider <rider id>]", "mount rider (default=player) on creature (default=target)"},
			{cs_dismountRider, 0, "[<rider id>]",                                   "dismount spcified rider (default=player)"},
			{"",               0, "",                                               ""}
		};

	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

	bool               getFocusObject(StringVector const &argv, int optionalObjectIdIndex, Object *&object, String &result);
	bool               lookupMountNameByRegex(char const *mountNameRegex, CrcString const *&mountName);

	bool               doCreateAt(StringVector const &argv, String &result);
	bool               doMountCreature(StringVector const &argv, String &result);
	bool               doDismountRider(StringVector const &argv, String &result);

}

using namespace SwgCuiCommandParserMountNamespace;

// ======================================================================
// namespace SwgCuiCommandParserMountNamespace
// ======================================================================

bool SwgCuiCommandParserMountNamespace::getFocusObject(StringVector const &argv, int optionalObjectIdIndex, Object *&object, String &result)
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

bool SwgCuiCommandParserMountNamespace::lookupMountNameByRegex(char const *mountNameRegex, CrcString const *&mountName)
{
	NOT_NULL(mountNameRegex);

	//-- Compile the regex.
	char const *errorString = NULL;
	int         errorOffset = 0;

	pcre *const compiledRegex = pcre_compile(mountNameRegex, 0, &errorString, &errorOffset, NULL);
	if (!compiledRegex)
	{
		DEBUG_WARNING(true, ("failed to compile regex [%s]: errorString=[%s], errorOffset=[%d]", mountNameRegex, errorString, errorOffset));
		return false;
	}

	bool returnValue = false;

	int const mountCount = MountValidScaleRangeTable::getCreatureCount();
	for (int i = 0; (i < mountCount) && !returnValue; ++i)
	{
		// Get mount name.
		CrcString const &testMountName = MountValidScaleRangeTable::getCreatureName(i);

		// Test mount name against regex.
		int const regexCompareCode = pcre_exec(compiledRegex, NULL, testMountName.getString(), static_cast<int>(strlen(testMountName.getString())), 0, 0, 0, 0);
		if (regexCompareCode < -1)
		{
			DEBUG_WARNING(true, ("error: pcre_exec returned [%d] with regex [%s] trying to match against [%s].", regexCompareCode, mountNameRegex, mountName->getString()));
			break;
		}
		else if (regexCompareCode >= 0)
		{
			returnValue = true;
			mountName   = &testMountName;
		}
	}

	//-- Release resources.
	RegexServices::freeMemory(compiledRegex);

	return returnValue;
}

// ----------------------------------------------------------------------

bool SwgCuiCommandParserMountNamespace::doCreateAt(StringVector const &argv, String &result)
{
	const std::string & objectTemplateName = Unicode::wideToNarrow (argv [1]);

	// Get mount name regex.
	std::string const & mountNameRegex = Unicode::wideToNarrow(argv[2]);

	// Find mount name from regex.
	CrcString const *mountName      = NULL;
	bool const       foundMountName = lookupMountNameByRegex(mountNameRegex.c_str(), mountName);

	if (!foundMountName || (mountName == NULL))
	{
		char buffer[512];
		snprintf(buffer, sizeof(buffer) - 1, "Regex=[%s] failed to find any matches in the mounts table\n", mountNameRegex.c_str());
		buffer[sizeof(buffer) - 1] = '\0';

		result += Unicode::narrowToWide(buffer);
		return false;
	}
	else
	{
		char buffer[512];
		snprintf(buffer, sizeof(buffer) - 1, "Found mount with appearance name = [%s]\n", mountName->getString());
		buffer[sizeof(buffer) - 1] = '\0';

		result += Unicode::narrowToWide(buffer);
	}

	// Get the user scale fraction.
	float userScaleFraction = 0.5;
	if (argv.size() >= 4)
		userScaleFraction = Unicode::toFloat(argv[3]);

	// Retrieve the valid scale range for the mount.
	int const saddleCapacity = 1;
	float     minScale        = 1.0f;
	float     maxScale        = 1.0f;
	float     appearanceScale = 1.0f;

	bool const getScaleRange = MountValidScaleRangeTable::getScaleRangeForCreatureAndSaddleCapacity(*mountName, saddleCapacity, minScale, maxScale);
	if (!getScaleRange)
	{
		DEBUG_WARNING(true, ("getScaleRangeForCreatureAndSaddleCapacity() failed for mount appearance name [%s] and saddle capacity [%d].", mountName->getString(), saddleCapacity));
	}
	else
	{
		appearanceScale = minScale + userScaleFraction * (maxScale - minScale);

		char buffer[512];
		snprintf(buffer, sizeof(buffer) - 1, "Final appearance scale: [%.2f].\n", appearanceScale);
		buffer[sizeof(buffer) - 1] = '\0';

		result += Unicode::narrowToWide(buffer);
	}

	SharedCreatureObjectTemplate const *const objectTemplate = dynamic_cast<SharedCreatureObjectTemplate const*>(ObjectTemplateList::fetch(objectTemplateName));
	if (!objectTemplate)
	{
		char buffer[512];
		snprintf(buffer, sizeof(buffer) - 1, "Failed to find/load object template [%s]\n", objectTemplateName.c_str());
		buffer[sizeof(buffer) - 1] = '\0';

		result += Unicode::narrowToWide(buffer);
		return false;
	}

	CreatureObject *const mountObject = new CreatureObject(objectTemplate);
	objectTemplate->releaseReference();

	// Set the mount's appearance.
	mountObject->setAppearance(AppearanceTemplateList::createAppearance(mountName->getString()));

	// Give it a remote creature controller.
	RemoteCreatureController *const controller = new RemoteCreatureController(mountObject);
	mountObject->setController(controller);

	// Set mount to be "mountable" and trigger end baselines.
	mountObject->setCondition(TangibleObject::C_mount);
	mountObject->endBaselines();

	// Set position to player's position.
	Object *const playerObject = Game::getPlayer();
	if (playerObject)
	{
		Vector const position_w = playerObject->getPosition_w();
		mountObject->setPosition_w(position_w);

		Transform transform_w;
		transform_w.setPosition_p(position_w);
		controller->updateDeadReckoningModel(transform_w);
	}

	// Add notifications.
	RenderWorld::addObjectNotifications(*mountObject);
	mountObject->addNotification(ClientWorld::getTangibleNotification());

	// Add creature to world.
	mountObject->addToWorld();

	// Report back to user.
	{
		char buffer[512];
		snprintf(buffer, sizeof(buffer) - 1, "Using regex=[%s], user scale fraction=[%.2f]\n", mountNameRegex.c_str(), userScaleFraction);
		buffer[sizeof(buffer) - 1] = '\0';

		result += Unicode::narrowToWide(buffer);
	}
	return true;
}

// ----------------------------------------------------------------------

bool SwgCuiCommandParserMountNamespace::doMountCreature(StringVector const &argv, String &result)
{
	// For now, assume this is the player trying to mount the player's target.
	CreatureObject *const riderObject = Game::getPlayerCreature();
	if (!riderObject)
	{
		char buffer[512];
		snprintf(buffer, sizeof(buffer) - 1, "error, Game::getPlayerCreature() returned NULL.\n");
		buffer[sizeof(buffer) - 1] = '\0';

		result += Unicode::narrowToWide(buffer);
		return false;
	}

	Object *baseMountObject = NULL;
	int const mountIdIndex  = 1;
	bool const gotMountObject = getFocusObject(argv, mountIdIndex, baseMountObject, result);		
	if (!gotMountObject)
	{
		char buffer[512];
		snprintf(buffer, sizeof(buffer) - 1, "user must have a mount selected or must specify mount id on the commandline.\n");
		buffer[sizeof(buffer) - 1] = '\0';

		result += Unicode::narrowToWide(buffer);
		return false;
	}

	CreatureObject *const mountObject = dynamic_cast<CreatureObject*>(baseMountObject);
	if (!mountObject)
	{
		char buffer[512];
		snprintf(buffer, sizeof(buffer) - 1, "error: selected mount with id=[%s] is not a CreatureObject.\n", baseMountObject->getNetworkId().getValueString().c_str());
		buffer[sizeof(buffer) - 1] = '\0';

		result += Unicode::narrowToWide(buffer);
		return false;
	}

	//-- Set states.
	riderObject->setState(States::RidingMount, true);
	mountObject->setState(States::MountedCreature, true);

	//-- Transfer rider into creature's rider slot.
	SlottedContainmentProperty *const scProperty = dynamic_cast<SlottedContainmentProperty*>(riderObject->getProperty(SlottedContainmentProperty::getClassPropertyId()));
	if (!scProperty)
	{
		char buffer[512];
		snprintf(buffer, sizeof(buffer) - 1, "error: rider id=[%s] does not have a slotted containment property.\n", riderObject->getNetworkId().getValueString().c_str());
		buffer[sizeof(buffer) - 1] = '\0';

		result += Unicode::narrowToWide(buffer);
		return false;
	}

	int const arrangementIndex = scProperty->getBestArrangementForSlot(SlotIdManager::findSlotId(ConstCharCrcString("rider")));
	if ((arrangementIndex < 0) || (arrangementIndex >= scProperty->getNumberOfArrangements()))
	{
		char buffer[512];
		snprintf(buffer, sizeof(buffer) - 1, "error: failed to find arrangement with rider slot for rider id=[%s].\n", riderObject->getNetworkId().getValueString().c_str());
		buffer[sizeof(buffer) - 1] = '\0';

		result += Unicode::narrowToWide(buffer);
		return false;
	}

	bool const transferSuccess = ContainerInterface::transferItemToSlottedContainer(*mountObject, *riderObject, arrangementIndex);
	if (!transferSuccess)
	{
		char buffer[512];
		snprintf(buffer, sizeof(buffer) - 1, "error: failed to transfer player into container of mount id=[%s].\n", baseMountObject->getNetworkId().getValueString().c_str());
		buffer[sizeof(buffer) - 1] = '\0';

		result += Unicode::narrowToWide(buffer);
		return false;
	}
	else
	{
		char buffer[512];
		snprintf(buffer, sizeof(buffer) - 1, "successfully mounted id=[%s].\n", baseMountObject->getNetworkId().getValueString().c_str());
		buffer[sizeof(buffer) - 1] = '\0';

		result += Unicode::narrowToWide(buffer);
		return true;
	}
}

// ----------------------------------------------------------------------

bool SwgCuiCommandParserMountNamespace::doDismountRider(StringVector const &argv, String &result)
{
	UNREF(argv);
	UNREF(result);
	return false;
}

// ======================================================================
// class SwgCuiCommandParserMount: PUBLIC
// ======================================================================

SwgCuiCommandParserMount::SwgCuiCommandParserMount() :
	CommandParser("mount", 0, "...", "client-side mount-related commands", 0)
{
	createDelegateCommands(cs_commands);
}

// ----------------------------------------------------------------------

SwgCuiCommandParserMount::~SwgCuiCommandParserMount()
{
}

// ----------------------------------------------------------------------

bool SwgCuiCommandParserMount::performParsing(NetworkId const &userId, StringVector_t const &argv, String_t const &originalCommand, String_t &result, CommandParser const *node)
{
	UNREF(userId);
	UNREF(originalCommand);
	UNREF(result);
	UNREF(node);

	//-- Check for empty args.
	if (argv.empty())
	{
		DEBUG_WARNING(true, ("SwgCuiCommandParserMount::performParsing(): argv is empty, unexpected."));
		return false;
	}

	//-- Check which command is getting run.
	if (isCommand(argv[0], cs_createAt))
		return doCreateAt(argv, result);
	else if (isCommand(argv[0], cs_mountCreature))
		return doMountCreature(argv, result);
	else if (isCommand(argv[0], cs_dismountRider))
		return doDismountRider(argv, result);
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
