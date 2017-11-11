// ======================================================================
//
// PageGraphics.h
// asommers
//
// copyright 2002, sony online entertainment
//
// ======================================================================

#ifndef INCLUDED_PageGraphics_H
#define INCLUDED_PageGraphics_H

// ======================================================================

#include "Resource.h"

#include <vector>

// ======================================================================

class PageGraphics : public CPropertyPage
{
	DECLARE_DYNCREATE(PageGraphics)

public:

	PageGraphics();
	~PageGraphics();

	//{{AFX_DATA(PageGraphics)
	enum { IDD = IDD_PROPPAGE_GRAPHICS };

	CString	m_lblGameResolution;
	CString	m_lblPixelShaderVersion;
	
	CString m_lblWindowedMode;
	CString m_lblBorderlessWindow;
	CString m_lblDisableBumpMapping;
	CString m_lblDisableHardwareMouseControl;
	CString m_lblUseLowDetailTextures;
	CString m_lblUseLowDetailNormalMaps;
	CString m_lblDisableMultipassRendering;
	CString m_lblDisableVsync;
	CString m_lblDisableFastMouseCursor;
	CString m_lblUseSafeRenderer;

	CString m_lblVspsOptimal;
	CString m_lblVsps20;
	CString m_lblVsps14;
	CString m_lblVsps11;
	CString m_lblVspsDisabled;

	CComboBox	m_vertexPixelShaderVersion;
	CComboBox	m_resolution;
	BOOL	m_windowed;
	BOOL	m_borderlessWindow;
	BOOL	m_disableBumpMapping;
	CButton	m_disableBumpMappingButton;
	BOOL	m_disableHardwareMouseCursor;
	CButton	m_disableHardwareMouseCursorButton;
	BOOL    m_useLowDetailTextures;
	BOOL    m_useLowDetailNormalMaps;
	BOOL	m_disableMultipassRendering;
	BOOL	m_disableVsync;
	BOOL	m_disableFastMouseCursor;
	BOOL	m_useSafeRenderer;
	//}}AFX_DATA

	//{{AFX_VIRTUAL(PageGraphics)
	public:
	virtual BOOL OnKillActive();
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

	
protected:

	//{{AFX_MSG(PageGraphics)
	virtual BOOL OnSetActive( );
	virtual BOOL OnInitDialog();
	afx_msg void OnSelchangeResolution();
	afx_msg void OnSelchangeVertexpixelshaderversion();
	afx_msg void OnCheckWindowed();
	afx_msg void OnCheckBorderlesswindow();
	afx_msg void OnCheckDisablebump();
	afx_msg void OnCheckDisablehardwaremousecursor();
	afx_msg void OnCheckUselowdetailtextures();
	afx_msg void OnCheckUselowdetailnormalmaps();
	afx_msg void OnCheckDisablemultipassrendering();
	afx_msg void OnCheckDisablevsync();
	afx_msg void OnCheckDisablefastmousecursor();
	afx_msg void OnCheckUsesaferenderer();
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
