// ============================================================================
//
// ClientAutomation.h
// copyright (c) 2005 Sony Online Entertainment
//
// ============================================================================

#ifndef INCLUDED_ClientAutomation_H
#define INCLUDED_ClientAutomation_H

#include "Automation/IEvent.h"
#include "Automation/EventValue.h"

class ClientAutomation
{
public:
	static void                      start( const std::string &scriptName );
	static Automation::IEventAutoPtr createEvent( Automation::EventType type );
	static void                      sendEvent( const Automation::IEventAutoPtr &event );

private:
	static void                      install();
	static void                      remove();

	friend class SetupSwgClientAutomation;
};

#define AUTOMATION_EVENT_PARAM( type, args ) \
{ \
	Automation::IEventAutoPtr event = ClientAutomation::createEvent( ET_##type ); \
	*event << args; \
	ClientAutomation::sendEvent( event ); \
}

#define AUTOMATION_EVENT( type ) \
{ \
	Automation::IEventAutoPtr event = ClientAutomation::createEvent( ET_##type ); \
	ClientAutomation::sendEvent( event ); \
}

#endif // INCLUDED_ClientAutomation_H