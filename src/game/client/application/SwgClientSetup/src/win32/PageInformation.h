// ======================================================================
//
// PageInformation.h
// asommers
//
// copyright 2002, sony online entertainment
//
// ======================================================================

#ifndef INCLUDED_PageInformation_H
#define INCLUDED_PageInformation_H

// ======================================================================

#include "Resource.h"

// ======================================================================

class PageInformation : public CPropertyPage
{
	DECLARE_DYNCREATE(PageInformation)

public:

	PageInformation();
	~PageInformation();

	//{{AFX_DATA(PageInformation)
	enum { IDD = IDD_PROPPAGE_INFORMATION };
	CString	m_physicalMemorySize;
	CString	m_numberOfProcessors;
	CString	m_os;
	CString	m_videoMemorySize;
	CString	m_pixelShaderVersion;
	CString	m_vertexShaderVersion;
	CString	m_videoIdentifier;
	CString	m_videoDriverVersion;
	CString	m_videoDeviceId;
	CString	m_cpuIdentifier;
	CString	m_cpuSpeed;
	CString	m_cpuVendor;
	CString	m_directXVersion;
	CString	m_trackIRVersion;

	CString	m_lblNumProcessors;
	CString	m_lblCpuIdentifier;
	CString m_lblCpuSpeed;
	CString	m_lblCpuVendor;
	CString	m_lblPhysicalMemorySize;
	CString	m_lblVideoDescription;
	CString	m_lblVideoIdentifier;
	CString	m_lblVideoDriverVersion;
	CString	m_lblVideoMemorySize;
	CString	m_lblVertexShaderVersion;
	CString	m_lblPixelShaderVersion;
	CString	m_lblOperatingSystem;
	CString m_lblBtnCopyToClipboard;
	CString	m_lblDirectXVersion;
	CString	m_lblTrackIRVersion;
	//}}AFX_DATA

	//{{AFX_VIRTUAL(PageInformation)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

protected:

	//{{AFX_MSG(PageInformation)
	virtual BOOL OnSetActive( );
	virtual BOOL OnInitDialog();
	afx_msg void OnButtonCopytoclipboard();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

private:
	void initializeDialog();
};

// ======================================================================

//{{AFX_INSERT_LOCATION}}

// ======================================================================

#endif 
