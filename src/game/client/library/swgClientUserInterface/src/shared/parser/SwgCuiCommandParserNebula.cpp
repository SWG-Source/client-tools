// ======================================================================
//
// SwgCuiCommandParserNebula.cpp
// Copyright Sony Online Entertainment
//
// ======================================================================

#include "swgClientUserInterface/FirstSwgClientUserInterface.h"
#include "swgClientUserInterface/SwgCuiCommandParserNebula.h"

#include "clientGame/ClientNebula.h"
#include "clientGame/CreatureObject.h"
#include "clientGame/Game.h"
#include "clientGame/NebulaManagerClient.h"
#include "clientUserInterface/CuiPreferences.h"
#include "sharedFoundation/Production.h"
#include "sharedGame/Nebula.h"
#include "sharedObject/NetworkIdManager.h"

// ======================================================================

namespace SwgCuiCommandParserNebulaNamespace
{
	namespace CommandNames
	{
		const char * const ms_info = "info";

		const char * const ms_orientedPercent = "orientedPercent";
		const char * const ms_density = "density";
		const char * const ms_range = "range";
		const char * const ms_numShells = "numShells";
		const char * const ms_regenerate = "regenerate";
		const char * const ms_randomSeedOffset = "randomSeedOffset";
		const char * const ms_colorVariance = "colorVariance";
		const char * const ms_quadGenerationRadius = "quadGenerationRadius";
		const char * const ms_renderNear = "renderNear";
		const char * const ms_renderFar = "renderFar";
		const char * const ms_reload = "reload";
	}

	const CommandParser::CmdInfo cmds[] =
	{
		{CommandNames::ms_info,                  0,  "",  "Show Nebula info"},
#if PRODUCTION==0
		{CommandNames::ms_orientedPercent, 1, "<percent>", "oriented percent of nebula quads"},
		{CommandNames::ms_density, 1, "<density>", "set nebula quad density"},
		{CommandNames::ms_range, 1, "<range>", "set nebula visual range"},
		{CommandNames::ms_numShells, 1, "<numShells>", "set number of nebula shells"},
		{CommandNames::ms_regenerate, 0, "", "regenerate nebulas next frame"},
		{CommandNames::ms_randomSeedOffset, 1, "<offset>", "set the random seed offset"},
		{CommandNames::ms_colorVariance, 3, "<r> <g> <b>", "set color variance"},
		{CommandNames::ms_quadGenerationRadius, 1, "<radius>", "quad generation radius"},
		{CommandNames::ms_renderNear, 1, "<on/off>", "render near quads (inside far plane)"},
		{CommandNames::ms_renderFar, 1, "<on/off>", "render far quads (outside far plane)"},
		{CommandNames::ms_reload, 1, "<scene name>", "load nebulas for scene"},
#endif
		{"", 0, "", ""} // this must be last
	};
}

using namespace SwgCuiCommandParserNebulaNamespace;

//-----------------------------------------------------------------

SwgCuiCommandParserNebula::SwgCuiCommandParserNebula()
 : CommandParser ("nebula", 0, "...", "Nebula commands", NULL)
{
	createDelegateCommands(SwgCuiCommandParserNebulaNamespace::cmds);
} 

//-----------------------------------------------------------------

bool SwgCuiCommandParserNebula::performParsing(const NetworkId &, const StringVector_t & argv, const String_t & originalCommand, String_t &, const CommandParser * node)
{
	UNREF(node);
	UNREF(originalCommand);

	CreatureObject * const player = Game::getPlayerCreature();
	if (player == NULL)
		return true;

	//-----------------------------------------------------------------
	
	if (isCommand(argv[0], CommandNames::ms_info))
	{
		return true;
	}
	
	//----------------------------------------------------------------------

	else if (isCommand(argv[0], CommandNames::ms_orientedPercent))
	{
		float const f = static_cast<float>(atof(Unicode::wideToNarrow(argv[1]).c_str()));
		NebulaManagerClient::Config::setOrientedPercent(f);
		NebulaManagerClient::Config::setRegenerate(true);		
		return true;
	}

	//----------------------------------------------------------------------

	else if (isCommand(argv[0], CommandNames::ms_density))
	{
		float const f = static_cast<float>(atof(Unicode::wideToNarrow(argv[1]).c_str()));
		CuiPreferences::setGlobalNebulaDensity(f);
		NebulaManagerClient::Config::setRegenerate(true);
		return true;
	}
	
	//----------------------------------------------------------------------

	else if (isCommand(argv[0], CommandNames::ms_range))
	{
		float const f = static_cast<float>(atof(Unicode::wideToNarrow(argv[1]).c_str()));
		CuiPreferences::setGlobalNebulaRange(f);
		NebulaManagerClient::Config::setRegenerate(true);
		return true;
	}
	
	//----------------------------------------------------------------------

	else if (isCommand(argv[0], CommandNames::ms_numShells))
	{
		int const i = atoi(Unicode::wideToNarrow(argv[1]).c_str());
		NebulaManagerClient::Config::setNumShells(i);
		NebulaManagerClient::Config::setRegenerate(true);
		return true;
	}
	
	//----------------------------------------------------------------------

	else if (isCommand(argv[0], CommandNames::ms_regenerate))
	{		
		NebulaManagerClient::Config::setRegenerate(true);
		return true;
	}

	//----------------------------------------------------------------------

	else if (isCommand(argv[0], CommandNames::ms_randomSeedOffset))
	{
		int const i = atoi(Unicode::wideToNarrow(argv[1]).c_str());
		NebulaManagerClient::Config::setRandomSeedOffset(i);
		NebulaManagerClient::Config::setRegenerate(true);
		return true;
	}
	
	//----------------------------------------------------------------------

	else if (isCommand(argv[0], CommandNames::ms_colorVariance))
	{
		float const r = static_cast<float>(atof(Unicode::wideToNarrow(argv[1]).c_str()));
		float const g = static_cast<float>(atof(Unicode::wideToNarrow(argv[2]).c_str()));
		float const b = static_cast<float>(atof(Unicode::wideToNarrow(argv[3]).c_str()));
		
		NebulaManagerClient::Config::setColorVariance(VectorArgb(1.0f, r, g, b));
		NebulaManagerClient::Config::setRegenerate(true);
		return true;
	}

	//----------------------------------------------------------------------

	else if (isCommand(argv[0], CommandNames::ms_quadGenerationRadius))
	{
		float const f = static_cast<float>(atof(Unicode::wideToNarrow(argv[1]).c_str()));
		NebulaManagerClient::Config::setQuadGenerationRadius(f);
		NebulaManagerClient::Config::setRegenerate(true);
		return true;
	}

	//----------------------------------------------------------------------

	else if (isCommand(argv[0], CommandNames::ms_renderNear))
	{
		NebulaManagerClient::Config::setRenderNear(argv[1][0] != '0');
		return true;
	}

	//----------------------------------------------------------------------

	else if (isCommand(argv[0], CommandNames::ms_renderFar))
	{
		NebulaManagerClient::Config::setRenderFar(argv[1][0] != '0');
		return true;
	}
	
	//----------------------------------------------------------------------

	else if (isCommand(argv[0], CommandNames::ms_reload))
	{
		NebulaManagerClient::loadScene(Unicode::wideToNarrow(argv[1]));
		return true;
	}

	//----------------------------------------------------------------------

	return false;
}

// ======================================================================
