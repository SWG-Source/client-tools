// ======================================================================
//
// SwgClientSetupDlg.h
// asommers
//
// copyright 2002, sony online entertainment
//
// ======================================================================

#ifndef INCLUDED_SwgClientSetupDlg_H
#define INCLUDED_SwgClientSetupDlg_H

// ======================================================================

#include "PageInformation.h"
#include "PageGraphics.h"
#include "PageSound.h"
#include "PageGame.h"
#include "PageAdvanced.h"
#include "PageDebug.h"

#include <vector>

// ======================================================================

class SwgClientSetupDlg : public CDialog
{
public:
	SwgClientSetupDlg(CWnd* pParent = NULL);	// standard constructor

	//{{AFX_DATA(SwgClientSetupDlg)
	enum { IDD = IDD_SWGCLIENTSETUP_DIALOG };
	//}}AFX_DATA

	//{{AFX_VIRTUAL(SwgClientSetupDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support
	//}}AFX_VIRTUAL

protected:
	HICON m_hIcon;

	//{{AFX_MSG(SwgClientSetupDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	afx_msg void OnButtonDelete();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

private:

	PageInformation m_pageInformation;
	PageGraphics    m_pageGraphics;
	PageSound       m_pageSound;
	PageAdvanced    m_pageAdvanced;
	PageGame        m_pageGame;
	PageDebug       m_pageDebug;

	CPropertySheet  m_propertySheet;

	CButton         m_resetInGame;
};

// ======================================================================

//{{AFX_INSERT_LOCATION}}

// ======================================================================

#endif 

