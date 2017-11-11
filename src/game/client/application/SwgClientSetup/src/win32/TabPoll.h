// ======================================================================
//
// TabPoll.h
// rsitton
//
// copyright 2005, sony online entertainment
//
// ======================================================================

#if !defined(AFX_TABPOLL_H__9A31AD5A_3581_407F_B7C7_A9A9A39D591C__INCLUDED_)
#define AFX_TABPOLL_H__9A31AD5A_3581_407F_B7C7_A9A9A39D591C__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// TabPoll.h : header file
//

#ifndef INCLUDED_Parser_H
#include "Parser.h"
#endif

class DialogPoll;
class Parser;

/////////////////////////////////////////////////////////////////////////////
// TabPoll window

class TabPoll : public CTabCtrl
{
// Construction
public:
	TabPoll(CStdioFile & file);

	DECLARE_DYNAMIC(TabPoll);

// Attributes
public:

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(TabPoll)
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~TabPoll();

	void Init();
	void SetRectangle();

	bool isHeaderParsed() const;

	void setTitle(CString const & title);
	CString const & getTitle() const;

	void setDescription(CString const & description);
	CString const & getDescription() const;

	void setTag(CString const & tag);
	CString const & getTag() const;

	void setCancelButton(CString const & tag);
	CString const & getCancelButton() const;

	void setFrequency(float const frequency);
	float getFrequency() const;

	void setEmailAddress(CString const & tag);
	CString const & getEmailAddress() const;

	void setSubmitButtonText(CString const & tag);
	CString const & getSubmitButtonText() const;


	void setPollActive(bool const active);
	bool isPollActive() const;


	// Set the tab you want to display.
	void setCurrentTab(int tabIndex);
	void setTabFromFocus();
	int getCurrentTab() const;
	void setNextTab();
	void setPreviousTab();
	bool isLastTab() const;

	// Did the user cancel?
	void setCanceled(bool cancel);

	// Get the information off the page.
	void getMessageInformation(CString & msgInfo);

	// Generated message map functions
protected:
	//{{AFX_MSG(TabPoll)
	afx_msg void OnDestroy();
	//}}AFX_MSG

	DECLARE_MESSAGE_MAP()

private:
	TabPoll();

	// Internal f(x)s.
	DialogPoll * createTab(CString const & tabName);
	void deleteTabs();

	void refreshItems();

	bool parseHeader();
	void setInfoFromHeader(Parser const & headerParser);

	// Question pages.
	typedef std::vector<CDialog *> TabPages;
	TabPages m_pages;
	int m_currentTab;

	// Header info.
	CString m_title;
	CString m_description;
	CString m_tag;
	float m_frequency;
	CString m_cancel;
	CString m_email;
	CString m_submit;


	// Canceled the dlg.
	bool	m_wasCanceled : 1,
			m_headerParsed : 1,
			m_pollActive : 1;

	// Keep a ref to the file.
	CStdioFile & m_file;
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_TABPOLL_H__9A31AD5A_3581_407F_B7C7_A9A9A39D591C__INCLUDED_)
