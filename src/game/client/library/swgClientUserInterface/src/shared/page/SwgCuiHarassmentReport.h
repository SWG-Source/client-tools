// ======================================================================
//
// SwgCuiHarassmentReport.h
// Copyright Sony Online Entertainment
//
// ======================================================================

#ifndef INCLUDED_SwgCuiHarassmentReport_H
#define INCLUDED_SwgCuiHarassmentReport_H

#include "clientGame/CommunityManager.h"
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
class SwgCuiHarassmentReport : public CuiMediator
                             , public UIEventCallback
{
public:

	SwgCuiHarassmentReport(UIPage &page);

	virtual void performActivate();
	virtual void performDeactivate();
	virtual void update(float deltaTimeSecs);

	virtual void OnButtonPressed(UIWidget *context);
	virtual bool OnMessage(UIWidget *context, const UIMessage & msg);

	void onVerifyPlayerNameResponse(CommunityManager::Messages::RequestVerifyPlayerNameResponse::Response const &response);

private:

	void getHarassmentCategory(int &category, int &subCategory) const;
	void verifyName();
	
	MessageDispatch::Callback *m_callBack;
	UIButton *                 m_submitButton;
	UIButton *                 m_cancelButton;
	UIButton *                 m_verifyNameButton;
	UIText *                   m_verifyNameLabel;
	UITextbox *                m_playerNameTextBox;
	UIText *                   m_commentText;
	Unicode::String            m_validPlayerName;
	bool                       m_verifyingName;
	float                      m_dotTimer;
	Unicode::String            m_dots;
	float                      m_verifyNameTimer;

	// Disabled

	~SwgCuiHarassmentReport();
	SwgCuiHarassmentReport(SwgCuiHarassmentReport const &rhs);
	SwgCuiHarassmentReport &operator =(SwgCuiHarassmentReport const &rhs);
};

// ======================================================================

#endif // INCLUDED_SwgCuiHarassmentReport_H
