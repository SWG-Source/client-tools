// ======================================================================
//
// SwgCuiService.h
// Copyright Sony Online Entertainment
//
// ======================================================================

#ifndef INCLUDED_SwgCuiService_H
#define INCLUDED_SwgCuiService_H

#include "clientUserInterface/CuiMediator.h"
#include "UIEventCallback.h"

//-----------------------------------------------------------------

namespace MessageDispatch
{
	class Callback;
}

//-----------------------------------------------------------------

class SwgCuiService : public CuiMediator
                    , public UIEventCallback
{
public:

	explicit SwgCuiService(UIPage & page);

	virtual void          performActivate();
	virtual void          performDeactivate();

	void                  switchToKnowledgeBaseTab();
	void                  switchToKnownIssuesTab();

	static SwgCuiService *createInto(UIPage *parent);

	virtual void          OnTabbedPaneChanged(UIWidget *context);


	class TicketList;

private:

	virtual void OnButtonPressed(UIWidget *context);

	class HowToSearch;
	class KnowledgeBase;
	class KnownIssues;

	enum Mediator
	{
		M_howToSearch,
		M_knowledgeBase,
		M_knownIssues,
		M_ticketList,
		M_count
	};

	CuiMediator                *m_mediators[M_count];
	UIButton                   *m_bugButton;
	UIButton                   *m_harassmentButton;
	UIButton                   *m_doneButton;
	HowToSearch                *m_howToSearchMediator;
	KnowledgeBase              *m_knowledgeBaseMediator;
	KnownIssues                *m_knownIssuesMediator;
	TicketList                 *m_ticketListMediator;

	UITabbedPane               *m_tabs;

	// Disabled

	~SwgCuiService();
	SwgCuiService(SwgCuiService const &rhs);
	SwgCuiService &operator =(SwgCuiService const &rhs);
};

// ======================================================================

#endif // INCLUDED_SwgCuiService_H
