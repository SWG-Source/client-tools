#if !defined(AFX_DEFAULTOBJECTPROPERTIESDIALOG_H__1D27069F_454F_4592_9C36_AAB0F59590A6__INCLUDED_)
#define AFX_DEFAULTOBJECTPROPERTIESDIALOG_H__1D27069F_454F_4592_9C36_AAB0F59590A6__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// DefaultObjectPropertiesDialog.h : header file
//

#include "DefaultObjectPropertiesManager.h"
//class DefaultObjectPropertiesManager;

/////////////////////////////////////////////////////////////////////////////
// DefaultObjectPropertiesDialog dialog

class DefaultObjectPropertiesDialog : public CDialog
{
// Construction
public:
	DefaultObjectPropertiesDialog(DefaultObjectPropertiesManager &defaults, CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(DefaultObjectPropertiesDialog)
	enum { IDD = IDD_DEFAULTPROPERTIES };
	CButton	m_removeProperty;
	CButton	m_addProperty;
	CListBox	m_propertyList;
	CComboBox	m_objectType;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(DefaultObjectPropertiesDialog)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	void _loadDataToDialog();
	void _loadDefaultsToListbox();
	void _addNewProperty();
	void _removeSelectedProperty();

	typedef DefaultObjectPropertiesManager::DefaultObjectPropertiesList DefaultObjectPropertiesList;
	typedef DefaultObjectPropertiesManager::StringMap StringMap;

	DefaultObjectPropertiesManager &m_defaults;
	DefaultObjectPropertiesList     m_workingObjectProperties;

	// Generated message map functions
	//{{AFX_MSG(DefaultObjectPropertiesDialog)
	virtual BOOL OnInitDialog();
	afx_msg void OnClose();
	afx_msg void OnDrawItem(int nIDCtl, LPDRAWITEMSTRUCT lpDrawItemStruct);
	virtual void OnOK();
	virtual void OnCancel();
	afx_msg void OnAddproperty();
	afx_msg void OnRemoveproperty();
	afx_msg void OnSelchangeObjecttype();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_DEFAULTOBJECTPROPERTIESDIALOG_H__1D27069F_454F_4592_9C36_AAB0F59590A6__INCLUDED_)
