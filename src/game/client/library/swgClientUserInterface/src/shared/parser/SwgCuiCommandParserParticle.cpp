// ======================================================================
//
// SwgCuiCommandParserParticle.cpp
// Copyright Sony Online Entertainment
//
// ======================================================================

#include "swgClientUserInterface/FirstSwgClientUserInterface.h"
#include "swgClientUserInterface/SwgCuiCommandParserParticle.h"

#include "clientParticle/ConfigClientParticle.h"
#include "clientParticle/ParticleManager.h"
#include "sharedFoundation/FormattedString.h"
#include "sharedFoundation/NetworkId.h"
#include <vector>

// ======================================================================

namespace SwgCuiCommandParserParticleNamespace
{

	namespace CommandNames
	{
		const char * const s_textures = "textures";
		const char * const s_extents = "extents";
		const char * const s_axis = "axis";
		const char * const s_text = "text";
	}

	const CommandParser::CmdInfo cmds[] =
	{
		{CommandNames::s_textures, 0, "", "Toggle particle textures"},
		{CommandNames::s_extents, 0, "", "Toggle particle extents"},
		{CommandNames::s_axis, 0, "", "Toggle particle axis"},
		{CommandNames::s_text, 0, "", "Toggle particle debug text"},
		{"", 0, "", ""} // this must be last
	};
}

//-----------------------------------------------------------------

SwgCuiCommandParserParticle::SwgCuiCommandParserParticle  ():
CommandParser ("particle", 0, "...", "particle commands", 0)
{
	createDelegateCommands (SwgCuiCommandParserParticleNamespace::cmds);
}
//-----------------------------------------------------------------

bool SwgCuiCommandParserParticle::performParsing(const NetworkId & userId, const StringVector_t & argv, const String_t & originalCommand, String_t & result, const CommandParser * node)
{
	UNREF(node);
	UNREF(originalCommand);
	UNREF(userId);
	UNREF(result);

	FormattedString<256> fs;

	//-----------------------------------------------------------------
	
	if (isCommand(argv[0], SwgCuiCommandParserParticleNamespace::CommandNames::s_textures))
	{
		ParticleManager::setTexturingEnabled(!ParticleManager::isTexturingEnabled());
	
		result += Unicode::narrowToWide(fs.sprintf("%s Show particle textures", ParticleManager::isTexturingEnabled() ? "[ENABLED]" : "[DISABLED]"));

		return true;
	}

	//-----------------------------------------------------------------
	
	else if (isCommand(argv[0], SwgCuiCommandParserParticleNamespace::CommandNames::s_extents))
	{
		ParticleManager::setDebugExtentsEnabled(!ParticleManager::isDebugExtentsEnabled());
	
		result += Unicode::narrowToWide(fs.sprintf("%s Show particle extents", ParticleManager::isDebugExtentsEnabled() ? "[ENABLED]" : "[DISABLED]"));

		return true;
	}

	//-----------------------------------------------------------------
	
	else if (isCommand(argv[0], SwgCuiCommandParserParticleNamespace::CommandNames::s_axis))
	{
		ParticleManager::setDebugAxisEnabled(!ParticleManager::isDebugAxisEnabled());
	
		result += Unicode::narrowToWide(fs.sprintf("%s Show particle axis", ParticleManager::isDebugAxisEnabled() ? "[ENABLED]" : "[DISABLED]"));

		return true;
	}

	//-----------------------------------------------------------------

	else if (isCommand(argv[0], SwgCuiCommandParserParticleNamespace::CommandNames::s_text))
	{
		ParticleManager::setDebugWorldTextEnabled(!ParticleManager::isDebugWorldTextEnabled());
	
		result += Unicode::narrowToWide(fs.sprintf("%s Show particle debug text", ParticleManager::isDebugWorldTextEnabled() ? "[ENABLED]" : "[DISABLED]"));

		return true;
	}

	return false;
}

// ======================================================================
