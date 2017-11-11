// ============================================================================
//
// ClientAutomation.cpp
// copyright (c) 2005 Sony Online Entertainment
//
// ============================================================================

#include "swgClientAutomation/FirstSwgClientAutomation.h"
#include "ClientAutomation.h"
#include "GameClient.h"

using namespace Automation;
using namespace std;

#include "Automation/IAutomation.h"
#include "Automation/IScript.h"
#include "Automation/IEvent.h"


namespace
{
	HMODULE       s_dllModule  = 0;
	IAutomation * s_automation = 0;
	IScript *     s_currentScript = 0;
}

void ClientAutomation::install()
{
	s_dllModule = LoadLibrary( "automation.dll" );

	if ( !s_dllModule )
	{
		return;
	}

	// get the interface

	typedef bool(*InitFunc)(IAutomation**);

	InitFunc init = (InitFunc)GetProcAddress( s_dllModule, "init" );

	if ( init )
	{
		init( &s_automation );
	}

	s_automation->setClient( new GameClient() );

	start( "TestScript" );
}

void ClientAutomation::remove()
{
	if ( s_dllModule )
	{
		FreeLibrary( s_dllModule );
	}
}

void ClientAutomation::start( const string &scriptName )
{
	FATAL( !s_automation, ( "Automated testing engine has not been initialized." ) );
	s_automation->start( scriptName.c_str(), &s_currentScript );
	AUTOMATION_EVENT_PARAM( ScriptLoaded, __FILE__ << __LINE__ );

}

IEventAutoPtr ClientAutomation::createEvent( EventType type )
{
	FATAL( !s_automation, ( "Automated testing engine has not been initialized." ) );
	return s_automation->createEvent( type );
}

void ClientAutomation::sendEvent( const IEventAutoPtr &event )
{
	FATAL( !s_automation, ( "Automated testing engine has not been initialized." ) );
	
	if ( s_currentScript )
	{
		s_currentScript->sendEvent( event );
	}
}