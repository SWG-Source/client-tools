// ======================================================================
//
// SwgCuiService_KnownIssues.h
// Copyright Sony Online Entertainment
//
// ======================================================================

#ifndef INCLUDED_SwgCuiService_KnownIssues_H
#define INCLUDED_SwgCuiService_KnownIssues_H

#include "clientUserInterface/CuiMediator.h"
#include "swgClientUserInterface/SwgCuiService.h"
#include "UIEventCallback.h"

class UIText;

//-----------------------------------------------------------------

namespace MessageDispatch
{
	class Callback;
}

//-----------------------------------------------------------------
class SwgCuiService::KnownIssues : public CuiMediator
                                   , public UIEventCallback
{
public:

	KnownIssues(UIPage &page, SwgCuiService &communityMediator);

	virtual void performActivate();
	virtual void performDeactivate();
	virtual void update(float deltaTimeSecs);

	void onGetFixedArticleResponse(std::pair<bool, Unicode::String> const &result);


private:

	

	MessageDispatch::Callback *m_callBack;
	SwgCuiService             &m_serviceMediator;
	UIText                    *m_listBoxText;

	void initializeListBox();

	// Disabled

	~KnownIssues();
	KnownIssues(KnownIssues const &rhs);
	KnownIssues &operator =(KnownIssues const &rhs);

	void getKnownIssuesArticle();
};

// ======================================================================

#endif // INCLUDED_SwgCuiService_KnownIssues_H
