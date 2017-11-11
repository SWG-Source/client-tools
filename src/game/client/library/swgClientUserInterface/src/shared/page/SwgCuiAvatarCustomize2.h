// ======================================================================
//
// SwgCuiAvatarCustomize2.h
// copyright (c) 2001 Sony Online Entertainment
//
// ======================================================================

#ifndef INCLUDED_SwgCuiAvatarCustomize2_H
#define INCLUDED_SwgCuiAvatarCustomize2_H

// ======================================================================

#include "swgClientUserInterface/SwgCuiAvatarCustomizationBase.h"

//-----------------------------------------------------------------------

class UIButton;

//-----------------------------------------------------------------------

class SwgCuiAvatarCustomize2 : public SwgCuiAvatarCustomizationBase
{
public:
	explicit SwgCuiAvatarCustomize2(UIPage & page);
	~SwgCuiAvatarCustomize2();
	virtual void OnButtonPressed(UIWidget *context);

protected:
	virtual void performActivate();
	virtual void performDeactivate();
	virtual void updateSelectedHairColorState();
	virtual void setGroup(std::string const & groupName);

private:
	//disabled
	SwgCuiAvatarCustomize2();
	SwgCuiAvatarCustomize2(const SwgCuiAvatarCustomize2 & rhs);
	SwgCuiAvatarCustomize2 & operator= (const SwgCuiAvatarCustomize2 & rhs);

private:
	void setColorPickerColumnAndMaxIndexes() const;

private:
	UIButton * m_okButton;
	UIButton * m_cancelButton;
	UIButton * m_buttonRandom;
	UIButton * m_buttonRandomAll;
};

// ======================================================================

#endif

