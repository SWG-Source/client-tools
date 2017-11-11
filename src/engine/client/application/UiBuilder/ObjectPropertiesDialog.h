#ifndef INCLUDED_ObjectPropertiesDialog_H
#define INCLUDED_ObjectPropertiesDialog_H

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// ObjectPropertiesDialog.h : header file
//

class PropertiesPage;

#include "ObjectPropertiesEditor.h"

#include "UIPropertyCategories.h"

/////////////////////////////////////////////////////////////////////////////
// ObjectPropertiesDialog

class ObjectPropertiesDialog : public CPropertySheet, public ObjectPropertiesEditor
{
	DECLARE_DYNAMIC(ObjectPropertiesDialog)

// Construction
public:
	ObjectPropertiesDialog(ObjectEditor &i_editor, UINT nIDCaption, CWnd* pParentWnd = NULL, UINT iSelectPage = 0);
	ObjectPropertiesDialog(ObjectEditor &i_editor, LPCTSTR pszCaption, CWnd* pParentWnd = NULL, UINT iSelectPage = 0);

// Attributes
public:

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(ObjectPropertiesDialog)
	public:
	virtual BOOL OnInitDialog();
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	protected:
	virtual void PostNcDestroy();
	virtual BOOL OnNotify(WPARAM wParam, LPARAM lParam, LRESULT* pResult);
	virtual LRESULT WindowProc(UINT message, WPARAM wParam, LPARAM lParam);
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual void destroy();

	void setAcceleratorTable(HACCEL x) { accelerators=x; }

	void saveUserPreferences();

	void setActiveAppearance(bool i_showActive);

	// Generated message map functions
protected:

	virtual ~ObjectPropertiesDialog();

	// --------------------------------------------------------------------------
	virtual void onEditReset();
	virtual void onEditInsertSubtree(UIBaseObject &subTree, UIBaseObject *previousSibling);
	virtual void onEditRemoveSubtree(UIBaseObject &subTree);
	virtual void onEditMoveSubtree(UIBaseObject &subTree, UIBaseObject *previousSibling, UIBaseObject *oldParent);
	virtual void onEditSetObjectProperty(UIBaseObject &object, const char *i_propertyName);
	virtual void onSelect(UIBaseObject &object, bool isSelected);
	// --------------------------------------------------------------------------

	// --------------------------------------------------------------------------
	virtual void _lock();
	virtual void _unlock();
	virtual PropertyCategory *_createCategory(int i);
	// --------------------------------------------------------------------------

	// --------------------------------------------------------------------------
	void _retextTab(CTabCtrl *tabControl, int tabIndex);
	void _retextAllTabs();
	void _retextSelectedTab();
	void _retextDialog();
	// --------------------------------------------------------------------------

	HACCEL accelerators;
	int tabControlID;
	bool showActive;

	//{{AFX_MSG(ObjectPropertiesDialog)
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnClose();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif
