//======================================================================
//
// SwgCuiOptAfk.h
// copyright (c) 2003 Sony Online Entertainment
//
//======================================================================

#ifndef INCLUDED_SwgCuiOptAfk_H
#define INCLUDED_SwgCuiOptAfk_H

#include "clientGame/AwayFromKeyBoardManager.h"
#include "swgClientUserInterface/SwgCuiOptBase.h"

class UICheckbox;
class UIComboBox;
class UIText;

//======================================================================

class SwgCuiOptAfk : 
public SwgCuiOptBase
{
public:
	explicit SwgCuiOptAfk (UIPage & page);

protected:

	void           performActivate ();
	void           performDeactivate ();
	virtual void   queryWidgetValues();
	virtual void   resetDefaults(bool confirmed);

	virtual void OnCheckboxSet(UIWidget *context);
	virtual void OnCheckboxUnset(UIWidget *context);

private:

	UIText *     m_textAwayFromKeyBoardAutomaticResponse;
	UICheckbox * m_checkBoxAutoAwayFromKeyboardEnabled;
	UIComboBox * m_comboAwayFromKeyBoard;

	~SwgCuiOptAfk ();
	SwgCuiOptAfk & operator=(const SwgCuiOptAfk & rhs);
	SwgCuiOptAfk            (const SwgCuiOptAfk & rhs);
};

//======================================================================

#endif
