// ======================================================================
//
// DialogPoll.h
// rsitton
//
// copyright 2005, sony online entertainment
//
// ======================================================================

#if !defined(AFX_DIALOGPOLL_H__1F567D4B_1C3A_42F3_9319_9AD2A530ACBD__INCLUDED_)
#define AFX_DIALOGPOLL_H__1F567D4B_1C3A_42F3_9319_9AD2A530ACBD__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// DialogPoll.h : header file
//

class CButton;

/////////////////////////////////////////////////////////////////////////////
// DialogPoll dialog

class DialogPoll : public CDialog
{
// Construction
public:
	DialogPoll(CWnd* pParent = NULL);   // standard constructor

	DECLARE_DYNAMIC(DialogPoll);

	void addButton(CString const & name, bool const isDefault);
	void setQuestionText(CString const & questionText);

	
	CString getSelectedButtonText() const;

	void setInputText(CString const & description, bool enable);
	CString getInputText() const;

	void getMessageInformation(CString & msgInfo);

// Dialog Data
	//{{AFX_DATA(DialogPoll)
	enum { IDD = IDD_DIALOG_POLL };
	CButton	m_buttonTemplate6;
	CButton	m_buttonTemplate5;
	CButton	m_buttonTemplate4;
	CStatic	m_inputDesc;
	CButton	m_buttonTemplate0;
	CButton	m_buttonTemplate1;
	CButton	m_buttonTemplate2;
	CButton	m_buttonTemplate3;
	CStatic	m_questionText;
	CEdit	m_templateText;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(DialogPoll)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation

protected:

	// Generated message map functions
	//{{AFX_MSG(DialogPoll)
	virtual BOOL OnInitDialog();
	afx_msg void OnButtonTemplate0();
	afx_msg void OnButtonTemplate1();
	afx_msg void OnButtonTemplate2();
	afx_msg void OnButtonTemplate3();
	afx_msg void OnButtonTemplate4();
	afx_msg void OnButtonTemplate5();
	afx_msg void OnButtonTemplate6();
	afx_msg void OnShowWindow(BOOL bShow, UINT nStatus);
	afx_msg void OnPaint();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

private:
	typedef std::vector<CButton *> Buttons;
				
	Buttons m_buttons;
	int m_buttonsActive;
	CString m_buttonSelectedText;

	void initializeButtons();
	void deleteButtons();
	void selectButton(CButton *);
	void setButtonFocus();
	void updateButtonPositions();
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_DIALOGPOLL_H__1F567D4B_1C3A_42F3_9319_9AD2A530ACBD__INCLUDED_)
