// ======================================================================
//
// SwgCuiDeleteAvatarConfirmation.h
// Copyright Sony Online Entertainment
//
// ======================================================================

#ifndef INCLUDED_SwgCuiDeleteAvatarConfirmation_H
#define INCLUDED_SwgCuiDeleteAvatarConfirmation_H

#include "clientUserInterface/CuiLoginManagerAvatarInfo.h"
#include "clientUserInterface/CuiMediator.h"
#include "UIEventCallback.h"

class UIButton;
class UIText;
class UITextbox;

//-----------------------------------------------------------------

namespace MessageDispatch
{
	class Callback;
}

//-----------------------------------------------------------------
class SwgCuiDeleteAvatarConfirmation : public CuiMediator
                                     , public UIEventCallback
{
public:

	struct Message
	{
		struct DeleteAvatarConfirmation
		{
			typedef CuiLoginManagerAvatarInfo Info;
		};
	};

	SwgCuiDeleteAvatarConfirmation(UIPage &page);
	~SwgCuiDeleteAvatarConfirmation();

	virtual void performActivate();
	virtual void performDeactivate();

	virtual void OnButtonPressed(UIWidget *context);

	void setAvatarInfo(CuiLoginManagerAvatarInfo const &avatarInfo);

private:
	
	MessageDispatch::Callback *m_callBack;
	UIButton *                 m_okButton;
	UIButton *                 m_cancelButton;
	UITextbox *                m_avatarTextBox;
	UIText *                   m_instructionsText;
	CuiLoginManagerAvatarInfo  m_avatarInfo;

	void validateText();

	// Disabled

	SwgCuiDeleteAvatarConfirmation(SwgCuiDeleteAvatarConfirmation const &rhs);
	SwgCuiDeleteAvatarConfirmation &operator =(SwgCuiDeleteAvatarConfirmation const &rhs);
};

// ======================================================================

#endif // INCLUDED_SwgCuiDeleteAvatarConfirmation_H
