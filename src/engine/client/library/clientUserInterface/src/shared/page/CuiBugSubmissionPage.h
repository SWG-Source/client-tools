//======================================================================
//
// CuiBugSubmissionPage.h
// copyright (c) 2002 Sony Online Entertainment
//
//======================================================================

#ifndef INCLUDED_CuiBugSubmissionPage_H
#define INCLUDED_CuiBugSubmissionPage_H

//======================================================================

#include "UIEventCallback.h"
#include "clientGame/CustomerServiceManager.h"
#include "clientUserInterface/CuiMediator.h"
#include "sharedNetworkMessages/CustomerServiceCategory.h"

class UIComboBox;
class UIDataSource;
class UIPage;
class UIText;
class UIWidget;

namespace MessageDispatch
{
	class Callback;
};

// ======================================================================

/**
* CuiBugSubmissionPage
*/
class CuiBugSubmissionPage :
public CuiMediator,
public UIEventCallback
{
public:
	explicit                      CuiBugSubmissionPage (UIPage & page);
	                              ~CuiBugSubmissionPage();

	virtual void                  performActivate   ();
	virtual void                  performDeactivate ();

	virtual void                  OnButtonPressed              (UIWidget * context);
	virtual void                  OnGenericSelectionChanged    (UIWidget *context);

	void                          setSendMiniDump(bool sendMiniDump);
	void                          addLineToBody(const std::string& line);

	void                          update(float deltaTimeSecs);

	void                          onRequestTicketCategoriesResponse(CustomerServiceManager::Messages::RequestTicketCategoriesResponse::Response const &response);

private:
	                              //unsigned ints are used to mesh up with the CSServer
	bool                          getCSCategories(Unicode::String const & catName, Unicode::String const & subCatName, /*OUT*/unsigned int& category, /*OUT*/unsigned int& subCategory);

	void                          setComboBoxItems(UIComboBox &comboBox, stdvector<CustomerServiceCategory>::fwd const &items) const;
	void                          populateCategories();

private:
//disabled
	                              CuiBugSubmissionPage ();
	                              CuiBugSubmissionPage (const CuiBugSubmissionPage & rhs);
	CuiBugSubmissionPage &        operator=            (const CuiBugSubmissionPage & rhs);

private:

	MessageDispatch::Callback *m_callBack;
	UIButton *                 m_cancelButton;
	UIButton *                 m_okButton;
	UIComboBox *               m_repeatableCombo;
	UIComboBox *               m_bugTypeCombo;
	UIComboBox *               m_bugSubTypeCombo;
	UIComboBox *               m_systemCombo;
	UIComboBox *               m_severityCombo;
	UIText *                   m_descriptionText;

	bool                       m_sendMiniDump;
	std::string                m_addedBody;
	float                      m_timeoutTimer;
};

//----------------------------------------------------------------------

inline void CuiBugSubmissionPage::setSendMiniDump(bool s)
{
	m_sendMiniDump = s;
}

//======================================================================

#endif

