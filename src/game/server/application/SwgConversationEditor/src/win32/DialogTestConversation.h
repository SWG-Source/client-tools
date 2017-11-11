// ======================================================================
//
// DialogTestConversation.h
// asommers 2003-09-23
//
// copyright2003, sony online entertainment
// 
// ======================================================================

#ifndef INCLUDED_DialogTestConversation_H
#define INCLUDED_DialogTestConversation_H

// ======================================================================

#include "Resource.h"

class Conversation;
class ConversationItem;

// ======================================================================

class DialogTestConversation : public CDialog
{
public:

	DialogTestConversation(Conversation const * conversation, ConversationItem const * conversationItem, CWnd * const wnd);   

	//{{AFX_DATA(DialogTestConversation)
	enum { IDD = IDD_DIALOG_TEST };
	CButton	m_button4;
	CButton	m_button3;
	CButton	m_button2;
	CButton	m_button1;
	CButton	m_button0;
	CString	m_text;
	//}}AFX_DATA

	//{{AFX_VIRTUAL(DialogTestConversation)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

protected:

	//{{AFX_MSG(DialogTestConversation)
	virtual BOOL OnInitDialog();
	afx_msg void OnButton0();
	afx_msg void OnButton1();
	afx_msg void OnButton2();
	afx_msg void OnButton3();
	afx_msg void OnButton4();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

private:

	void update ();
	void processButton (int buttonIndex);

private:

	Conversation const * m_conversation;
	ConversationItem const * m_conversationItem;
};

// ======================================================================

//{{AFX_INSERT_LOCATION}}

// ======================================================================

#endif 

