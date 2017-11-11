// ======================================================================
//
// DialogStringEdit.h
// asommers
//
// copyright 2004, sony online entertainment
//
// ======================================================================

#ifndef INCLUDED_DialogStringEdit_H
#define INCLUDED_DialogStringEdit_H

// ======================================================================

#include "Resource.h"

// ======================================================================

class DialogStringEdit : public CDialog
{
public:

	DialogStringEdit(CString const & stringId, CString const & string);

	CString const & getString() const;

	//{{AFX_DATA(DialogStringEdit)
	enum { IDD = IDD_DIALOG_STRINGEDIT };
	CButton	m_button3;
	CButton	m_button2;
	CButton	m_button1;
	CButton	m_button0;
	CEdit	m_stringCtrl;
	//}}AFX_DATA

	//{{AFX_VIRTUAL(DialogStringEdit)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);
	//}}AFX_VIRTUAL

protected:

	//{{AFX_MSG(DialogStringEdit)
	afx_msg void OnButton0();
	afx_msg void OnButton1();
	afx_msg void OnButton2();
	afx_msg void OnButton3();
	virtual BOOL OnInitDialog();
	virtual void OnOK();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

private:

	void OnButton(CButton const & button);

private:

	CString const m_stringId;
	CString m_string;
	CButton * m_buttonList[4];
	CString m_buttonData[4];
	int m_numberOfButtons;
};

// ======================================================================

//{{AFX_INSERT_LOCATION}}

// ======================================================================

#endif 
