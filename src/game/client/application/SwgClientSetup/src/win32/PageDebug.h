// ======================================================================
//
// PageDebug.h
// asommers
//
// copyright 2002, sony online entertainment
//
// ======================================================================

#ifndef INCLUDED_PageDebug_H
#define INCLUDED_PageDebug_H

// ======================================================================

#include "Resource.h"

// ======================================================================

class PageDebug : public CPropertyPage
{
	DECLARE_DYNCREATE(PageDebug)

public:

	PageDebug();
	~PageDebug();

	//{{AFX_DATA(PageDebug)
	enum { IDD = IDD_PROPPAGE_DEBUG };
	CString	m_contact;
	CString	m_crashLog;
	CString	m_hardwareInformation;
	CString	m_stationId;

	CString m_lblCrashLogs;
	CString m_lblIncludeStationId;
	CString m_lblSendHardwareInformation;
	CString m_lblContact;

	CString m_lblBtnConfigure;

	//}}AFX_DATA

	//{{AFX_VIRTUAL(PageDebug)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

	
protected:

	//{{AFX_MSG(PageDebug)
	virtual BOOL OnSetActive( );
	virtual BOOL OnInitDialog();
	afx_msg void OnButtonDeleteallminidumps();
	afx_msg void OnCheckDeleteminidumpsaftersending();
	afx_msg void OnButtonConfigure();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

private:

	void initalizeDialog( );
	void populate ();
};

// ======================================================================

//{{AFX_INSERT_LOCATION}}

// ======================================================================

#endif 
