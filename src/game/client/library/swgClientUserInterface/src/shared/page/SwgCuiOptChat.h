//======================================================================
//
// SwgCuiOptChat.h
// copyright (c) 2003 Sony Online Entertainment
//
//======================================================================

#ifndef INCLUDED_SwgCuiOptChat_H
#define INCLUDED_SwgCuiOptChat_H

#include "swgClientUserInterface/SwgCuiOptBase.h"

//======================================================================

class SwgCuiOptChat : 
public SwgCuiOptBase
{
public:
	explicit SwgCuiOptChat (UIPage & page);
	~SwgCuiOptChat () {}

	static int  onCombatSpamFilterGet(const SwgCuiOptBase & base, const UIComboBox & box);
	static void onCombatSpamFilterSet(const SwgCuiOptBase & base, const UIComboBox & box, int index);

	virtual void             update           (float deltaTimeSecs);

protected:
	virtual void             performActivate  ();
	virtual void             performDeactivate();

private:

	SwgCuiOptChat & operator=(const SwgCuiOptChat & rhs);
	SwgCuiOptChat            (const SwgCuiOptChat & rhs);
};

//======================================================================

#endif
