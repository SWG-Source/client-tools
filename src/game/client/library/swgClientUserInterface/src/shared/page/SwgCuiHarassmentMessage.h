// ======================================================================
//
// SwgCuiHarassmentMessage.h
// Copyright Sony Online Entertainment
//
// ======================================================================

#ifndef INCLUDED_SwgCuiHarassmentMessage_H
#define INCLUDED_SwgCuiHarassmentMessage_H
#include "clientUserInterface/CuiMediator.h"
#include "UIEventCallback.h"


//-----------------------------------------------------------------

namespace MessageDispatch
{
	class Callback;
}

//-----------------------------------------------------------------
class SwgCuiHarassmentMessage : public CuiMediator
                             , public UIEventCallback
{
public:

	SwgCuiHarassmentMessage(UIPage &page);

	virtual void performActivate();
	virtual void performDeactivate();

	virtual void OnButtonPressed(UIWidget *context);

	void setFromKBSearch(bool val);

	static SwgCuiHarassmentMessage *createInto(UIPage *parent);
private:


	UIButton *                 m_cancelButton;
	UIButton *                 m_continueButton;


	bool                       m_fromKBSearch;

	// Disabled

	~SwgCuiHarassmentMessage();
	SwgCuiHarassmentMessage(SwgCuiHarassmentMessage const &rhs);
	SwgCuiHarassmentMessage &operator =(SwgCuiHarassmentMessage const &rhs);
};

// ======================================================================

#endif // INCLUDED_SwgCuiHarassmentMessage_H
