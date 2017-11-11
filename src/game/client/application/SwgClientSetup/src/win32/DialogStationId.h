// ======================================================================
//
// DialogStationId.h
// asommers
//
// copyright 2003, sony online entertainment
//
// ======================================================================

#ifndef INCLUDED_DialogStationId_H
#define INCLUDED_DialogStationId_H

// ======================================================================

#include "Resource.h"

// ======================================================================

class DialogStationId : public CDialog
{
public:

	DialogStationId(CWnd* pParent = NULL);   

	//{{AFX_DATA(DialogStationId)
	enum { IDD = IDD_DIALOG_STATIONID };
	int		m_radio;

	CString m_lblSendStationIdInfo;
	CString m_lblSendStationIdQuery;
	CString m_lblPleaseInclude;
	CString m_lblDoNotInclude;

	CString m_lblBtnCancel;
	CString m_lblBtnNext;

	//}}AFX_DATA

	//{{AFX_VIRTUAL(DialogStationId)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    
	//}}AFX_VIRTUAL

protected:

	//{{AFX_MSG(DialogStationId)
	afx_msg void OnButtonProceed();
	virtual BOOL OnInitDialog();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

// ======================================================================

//{{AFX_INSERT_LOCATION}}

// ======================================================================

#endif 
