//======================================================================
//
// SwgCuiOptUi.h
// copyright (c) 2003 Sony Online Entertainment
//
//======================================================================

#ifndef INCLUDED_SwgCuiOptUi_H
#define INCLUDED_SwgCuiOptUi_H

#include "swgClientUserInterface/SwgCuiOptBase.h"

//======================================================================

class UIComboBox;

class SwgCuiOptUi : 
public SwgCuiOptBase
{
public:
	explicit SwgCuiOptUi (UIPage & page);

	static int  onComboPaletteGet    (const SwgCuiOptBase & , const UIComboBox &);
	static void onComboPaletteSet    (const SwgCuiOptBase & , const UIComboBox & , int value);

	virtual void             resetDefaults     (bool confirmed);

protected:
	void           performActivate ();
	void           performDeactivate ();

private:

	~SwgCuiOptUi ();
	SwgCuiOptUi & operator=(const SwgCuiOptUi & rhs);
	SwgCuiOptUi            (const SwgCuiOptUi & rhs);

	UIComboBox *   m_combo;
	
	class CallbackReceiverWaypointMonitor;
	CallbackReceiverWaypointMonitor * m_callbackReceiverWaypointMonitor;
	
	class CallbackReceiverExpMonitor;
	CallbackReceiverExpMonitor * m_callbackReceiverExpMonitor;

	class CallbackReceiverLocationDisplay;
	CallbackReceiverLocationDisplay * m_callbackReceiverLocationDisplay;
};

//======================================================================

#endif
