//===================================================================
//
// PreferencesDialog.h
// copyright 1999, Bootprint Entertainment
// copyright 2002, sony online entertainment
//
//===================================================================

#ifndef INCLUDED_PreferencesDialog_H
#define INCLUDED_PreferencesDialog_H

//===================================================================

#include "Viewer.h"

//===================================================================

class PreferencesDialog : public CDialog
{
public:

	explicit PreferencesDialog(CWnd* pParent);

	//{{AFX_DATA(PreferencesDialog)
	enum { IDD = IDD_PREFERENCES };
	CButton	m_bloom;
	CStatic	m_fogColor;
	CStatic	m_windowColor;
	BOOL	m_alternateRGB;
	int		m_colorB;
	int		m_colorG;
	int		m_colorR;
	int		m_gridSize;
	int		m_numSubdivisions;
	int		m_cameraMode;
	BOOL	m_lowPriority;
	BOOL	m_enableFog;
	BOOL	m_displayRenderStats;
	int		m_fogB;
	float	m_fogDensity;
	int		m_fogG;
	int		m_fogR;
	CString	m_environmentMapName;
	//}}AFX_DATA

	//{{AFX_VIRTUAL(PreferencesDialog)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);   
	//}}AFX_VIRTUAL

protected:

	void readPreferences ();
	void writePreferences () const;

	//{{AFX_MSG(PreferencesDialog)
	virtual BOOL OnInitDialog();
	virtual void OnOK();
	afx_msg void OnButtonLightColor();
	afx_msg void OnPaint();
	afx_msg void OnButtonFogColor();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//===================================================================

//{{AFX_INSERT_LOCATION}}

//===================================================================

#endif 
