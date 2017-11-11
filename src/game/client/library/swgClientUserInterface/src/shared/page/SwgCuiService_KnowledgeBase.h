// ======================================================================
//
// SwgCuiService_KnowledgeBase.h
// Copyright Sony Online Entertainment
//
// ======================================================================

#ifndef INCLUDED_SwgCuiService_KnowledgeBase_H
#define INCLUDED_SwgCuiService_KnowledgeBase_H

#include "clientUserInterface/CuiMediator.h"
#include "swgClientUserInterface/SwgCuiService.h"
#include "UIEventCallback.h"

class CustomerServiceSearchResult;
class UIButton;
class UIComposite;
class UITable;
class UIText;
class UITextbox;
 
//-----------------------------------------------------------------

namespace MessageDispatch
{
	class Callback;
}

//-----------------------------------------------------------------
class SwgCuiService::KnowledgeBase : public CuiMediator
                                   , public UIEventCallback
{
public:

	KnowledgeBase(UIPage &page, SwgCuiService &communityMediator);

	virtual void performActivate();
	virtual void performDeactivate();
	virtual void update(float deltaTimeSecs);

	virtual void OnButtonPressed(UIWidget *context);
	virtual bool OnMessage(UIWidget *context, const UIMessage & msg);

	void onSearchKnowledgeBaseResponse(std::pair<bool, stdvector<CustomerServiceSearchResult>::fwd> const &result);
	void onGetArticleResponse(std::pair<bool, Unicode::String> const &result);

private:
	
	void searchArticleNumber();
	void searchArticleString();
	void disableSearch();
	void enableSearch();
	void clearTable();

	typedef stdvector<CustomerServiceSearchResult>::fwd ArticleHeaderList;

	enum Columns
	{
		C_articleId,
		C_rank,
		C_articleTitle,
		C_count
	};

	MessageDispatch::Callback *m_callBack;
	SwgCuiService &            m_serviceMediator;
	UIButton *                 m_searchArticleNumberButton;
	UIButton *                 m_searchArticleStringButton;
	UIButton *                 m_didNotHelpButton;
	UITextbox *                m_articleSearchStringTextBox;
	UITextbox *                m_articleSearchNumberTextBox;
	UITable *                  m_articleTable;
	UIText *                   m_articleSummaryText;
	UIText *                   m_articleTitleText;
	ArticleHeaderList *        m_articleHeaderList;
	time_t                     m_lastArticleRequestTime;
	time_t                     m_lastArticleSummaryRequestTime;
	Unicode::String            m_articleTitle;
	UIPage                     *m_disablePanel;
	UIComposite                *m_textBoxComposite;

	// Disabled

	~KnowledgeBase();
	KnowledgeBase(KnowledgeBase const &rhs);
	KnowledgeBase &operator =(KnowledgeBase const &rhs);
};

// ======================================================================

#endif // INCLUDED_SwgCuiService_KnowledgeBase_H
