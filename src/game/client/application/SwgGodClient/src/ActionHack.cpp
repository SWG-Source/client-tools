// ======================================================================
//
// ActionHack.cpp
// copyright (c) 2001 Sony Online Entertainment
//
// ======================================================================

#include "SwgGodClient/FirstSwgGodClient.h"
#include "ActionHack.h"
#include "ActionHack.moc"

// ======================================================================
ActionHack::ActionHack(QObject* theParent, const char* theName, bool toggle)
: QAction(theParent, theName, toggle)
{
}

//-----------------------------------------------------------------

ActionHack::ActionHack(const QString& theText, const QIconSet& icon, const QString& theMenuText, int theAccel, QObject* theParent, const char* theName, bool toggle)
: QAction(theText, icon, theMenuText, theAccel, theParent, theName, toggle)
{ 
}

//-----------------------------------------------------------------

ActionHack::ActionHack(const QString& theText, const QString& theMenuText, int theAccel, QObject* theParent, const char* theName, bool toggle) 
: QAction(theText, theMenuText, theAccel, theParent, theName, toggle)
{
}

//-----------------------------------------------------------------

/**
* Emits the activated () signal.  Calls doToggle(bool) if the action is a toggle action.
*/ 
void ActionHack::doActivate()
{
	emit activated();

	if( isToggleAction())
		doToggle(!isOn());
}

//-----------------------------------------------------------------

/**
* Only useful for toggle actions.  Emits the toggled(bool) signal.
*/
void ActionHack::doToggle(bool b)
{
	if(!isToggleAction())
		return;
	setOn(b);
	emit toggled(b);
}

// ======================================================================
