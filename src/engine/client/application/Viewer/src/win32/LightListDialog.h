#if !defined(AFX_LIGHTLISTDIALOG_H__350FD6C7_9AA8_48DB_9E5B_D7AC08575EC7__INCLUDED_)
#define AFX_LIGHTLISTDIALOG_H__350FD6C7_9AA8_48DB_9E5B_D7AC08575EC7__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// LightListDialog.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// LightListDialog dialog

#include "../../../../../../engine/shared/library/sharedFoundation/include/public/sharedFoundation/ArrayList.h"
#include "clientObject/LightManager.h"

class LightListDialog : public CDialog
{
public:

	LightManager lightManager;

// Construction
public:

	LightListDialog(CWnd* pParent, const LightManager& newLightManager);

	const ArrayList<LightManager::LightData>& getLightDataList (void) const;

// Dialog Data
	//{{AFX_DATA(LightListDialog)
	enum { IDD = IDD_LIGHTLIST };
	CListBox	m_lightList;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(LightListDialog)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(LightListDialog)
	afx_msg void OnAddLight();
	afx_msg void OnEditLight();
	afx_msg void OnDblclkLightList();
	afx_msg void OnLoadLights();
	afx_msg void OnRemoveLight();
	afx_msg void OnSaveLights();
	virtual BOOL OnInitDialog();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

inline const ArrayList<LightManager::LightData>& LightListDialog::getLightDataList (void) const
{
	return lightManager.getLights ();
}

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_LIGHTLISTDIALOG_H__350FD6C7_9AA8_48DB_9E5B_D7AC08575EC7__INCLUDED_)
