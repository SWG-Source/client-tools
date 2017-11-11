// ======================================================================
//
// PageSound.h
// asommers
//
// copyright 2002, sony online entertainment
//
// ======================================================================

#ifndef INCLUDED_PageSound_H
#define INCLUDED_PageSound_H

// ======================================================================

#include "Resource.h"

// ======================================================================

class PageSound : public CPropertyPage
{
	DECLARE_DYNCREATE(PageSound)

public:

	PageSound();
	~PageSound();

	//{{AFX_DATA(PageSound)
	enum { IDD = IDD_PROPPAGE_SOUND };
	CButton	m_disableAudioButton;
	CComboBox	m_soundProvider;
	BOOL	m_disableAudio;
	CString	m_soundVersion;

	CString m_lblDisableAudio;
	CString m_lblMilesVersion;
	CString m_lblMilesProvider;
	//}}AFX_DATA

	//{{AFX_VIRTUAL(PageSound)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL
	
protected:

	//{{AFX_MSG(PageSound)
	
	virtual BOOL OnSetActive( );virtual BOOL OnInitDialog();
	afx_msg void OnCheckDisableaudio();
	afx_msg void OnSelchangeSoundprovider();
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
