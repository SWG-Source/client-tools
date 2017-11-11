// ======================================================================
//
// PageGame.h
// asommers
//
// copyright 2002, sony online entertainment
//
// ======================================================================

#ifndef INCLUDED_PageGame_H
#define INCLUDED_PageGame_H

// ======================================================================

#include "Resource.h"

// ======================================================================

class PageGame : public CPropertyPage
{
	DECLARE_DYNCREATE(PageGame)

public:

	PageGame();
	~PageGame();

	//{{AFX_DATA(PageGame)
	enum { IDD = IDD_PROPPAGE_GAME };
	CButton	m_enableForceFeedback;
	CButton	m_enableJoysticks;
	CComboBox	m_joystick;
	CComboBox   m_language;
	BOOL	m_disableCharacterLodManager;
	BOOL	m_skipIntro;

	CString m_lblSkipIntro;
	CString m_lblDisableCharacterLodManager;
	CString m_lblLanguage;
	CString m_lblEnableJoysticks;
	CString m_lblJoystick;
	CString m_lblEnabledForceFeedback;

	CString m_localeCode;
	CString m_fontLocaleCode;
	//}}AFX_DATA

	//{{AFX_VIRTUAL(PageGame)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

	
protected:

	void applyOptions ();

	//{{AFX_MSG(PageGame)
	virtual BOOL OnSetActive( );
	afx_msg void OnCheckSkipintro();
	afx_msg void OnCheckDisablecharacterlodmanager();
	virtual BOOL OnInitDialog();
	afx_msg void OnCheckEnablejoysticks();
	afx_msg void OnSelchangeGamecontroller();
	afx_msg void OnSelchangeLanguage();
	afx_msg void OnCheckEnableforcefeedback();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

private:
	
	void initializeDialog( );
	
	bool m_userChangedLanguage;
};

// ======================================================================

//{{AFX_INSERT_LOCATION}}

// ======================================================================

#endif 
