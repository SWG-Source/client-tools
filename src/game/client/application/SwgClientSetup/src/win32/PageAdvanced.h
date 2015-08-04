// ======================================================================
//
// PageAdvanced.h
// asommers
//
// copyright 2002, sony online entertainment
//
// ======================================================================

#ifndef INCLUDED_PageAdvanced_H
#define INCLUDED_PageAdvanced_H

// ======================================================================

#include "Resource.h"

// ======================================================================

class PageAdvanced : public CPropertyPage
{
	DECLARE_DYNCREATE(PageAdvanced)

public:

	PageAdvanced();
	~PageAdvanced();

	//{{AFX_DATA(PageAdvanced)
	enum { IDD = IDD_PROPPAGE_ADVANCED };
	BOOL	m_disableWorldPreloading;
	BOOL	m_skipL0Characters;
	BOOL	m_skipL0Meshes;
	CString	m_gameMemorySize;
	BOOL	m_disableTextureBaking;
	BOOL	m_disableFileCaching;
	BOOL	m_disableAsynchronousLoader;
	CButton	m_skipL0MeshesButton;

	CString m_lblHeader;
	CString m_lblDisableWordPreloading;
	CString m_lblUseLowDetailCharacters;
	CString m_lblUseLowDetailMeshes;
	CString m_lblDisableTextureBaking;
	CString m_lblDisableFileCaching;
	CString m_lblDisableAsynchronousLoader;
	CString m_lblGameMemorySize;
	CString m_lblGameMemoryInfo;
	//}}AFX_DATA

	//{{AFX_VIRTUAL(PageAdvanced)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

protected:

	//{{AFX_MSG(PageAdvanced)
	virtual BOOL OnInitDialog();
	virtual BOOL OnSetActive();
	afx_msg void OnCheckDisableshadows();
	afx_msg void OnCheckDisableworldpreloading();
	afx_msg void OnCheckSkipl0characters();
	afx_msg void OnCheckSkipl0meshes();
	afx_msg void OnCheckDisabletexturebaking();
	afx_msg void OnCheckDisablefilecaching();
	afx_msg void OnCheckDisableasynchronousloader();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

private:

	void initializeDialog();
	void applyOptions ();
};

// ======================================================================

//{{AFX_INSERT_LOCATION}}

// ======================================================================

#endif
