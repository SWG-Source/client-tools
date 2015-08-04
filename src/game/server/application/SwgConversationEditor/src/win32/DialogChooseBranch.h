// ======================================================================
//
// DialogChooseBranch.h
// asommers
//
// copyright 2003, sony online entertainment
// 
// ======================================================================

#ifndef INCLUDED_DialogChooseBranch_H
#define INCLUDED_DialogChooseBranch_H

// ======================================================================

#include "Resource.h"

class Conversation;
class ConversationResponse;

// ======================================================================

class DialogChooseBranch : public CDialog
{
public:

	DialogChooseBranch (Conversation const * conversation, ConversationResponse const * response, CWnd * const wnd); 

	//{{AFX_DATA(DialogChooseBranch)
	enum { IDD = IDD_DIALOG_CHOOSE_BRANCH };
	CButton	m_button4;
	CButton	m_button3;
	CButton	m_button2;
	CButton	m_button1;
	CButton	m_button0;
	//}}AFX_DATA

	//{{AFX_VIRTUAL(DialogChooseBranch)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

protected:

	//{{AFX_MSG(DialogChooseBranch)
	virtual BOOL OnInitDialog();
	afx_msg void OnButton0();
	afx_msg void OnButton1();
	afx_msg void OnButton2();
	afx_msg void OnButton3();
	afx_msg void OnButton4();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

public:

	int m_result;

private:

	Conversation const * const m_conversation;
	ConversationResponse const * const m_response;
};

// ======================================================================

//{{AFX_INSERT_LOCATION}}

// ======================================================================

#endif 
