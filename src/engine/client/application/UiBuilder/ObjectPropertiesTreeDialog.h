#if !defined(AFX_OBJECTPROPERTIESTREEDIALOG_H__071096A8_2D31_4E01_A166_F5F51693BC52__INCLUDED_)
#define AFX_OBJECTPROPERTIESTREEDIALOG_H__071096A8_2D31_4E01_A166_F5F51693BC52__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// ObjectPropertiesTreeDialog.h : header file
//

#include "ObjectPropertiesEditor.h"

#include "UIPropertyCategories.h"

/////////////////////////////////////////////////////////////////////////////
// ObjectPropertiesTreeDialog dialog

class ObjectPropertiesTreeDialog : public CDialog, public ObjectPropertiesEditor
{
// Construction
public:
	ObjectPropertiesTreeDialog(ObjectEditor &i_editor, CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(ObjectPropertiesTreeDialog)
	enum { IDD = IDD_OBJECTPROPERTIESDIALOG };
		// NOTE: the ClassWizard will add data members here
	//}}AFX_DATA


// Overrides
	void OnOK();
	void OnCancel();
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(ObjectPropertiesTreeDialog)
	public:
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual void PostNcDestroy();
	virtual LRESULT WindowProc(UINT message, WPARAM wParam, LPARAM lParam);
	//}}AFX_VIRTUAL

public:

	virtual void setAcceleratorTable(HACCEL x);
	virtual void saveUserPreferences();
	virtual void setActiveAppearance(bool i_showActive);

	virtual void destroy();

// Implementation
protected:

	// ==========================================================================

	typedef std::pair<CTreeCtrl *, HTREEITEM> TreeItem;

	// ==========================================================================

	class PropertyList : public ObjectPropertiesEditor::PropertyList
	{
	public:

		// ------------------------------------------------------

		PropertyList(const UIPropertyGroup &i_propertyGroup, PropertyCategory &owner);
		virtual ~PropertyList();

		// ------------------------------------------------------

		virtual void addObject(UIBaseObject &o);
		virtual bool removeObject(UIBaseObject &o);
		virtual void onSetValue(UIBaseObject &o, const char *i_propertyName);

		// ------------------------------------------------------

	protected:

		// ------------------------------------------------------

		// ------------------------------------------------------
	};

	// ==========================================================================

	class PropertyCategory : public ObjectPropertiesEditor::PropertyCategory
	{
	public:

		// ------------------------------------------------------

		PropertyCategory(ObjectEditor &i_editor, UIPropertyCategories::Category i_category, const TreeItem &i_treeParent);
		virtual ~PropertyCategory();

		// ------------------------------------------------------

		CTreeCtrl &tree() { return *m_treeParent.first; }
		HTREEITEM parentTreeItem() { return m_treeParent.second; }
		HTREEITEM treeItem() { return m_treeItem; }

	protected:

		// ------------------------------------------------------

		virtual ObjectPropertiesEditor::PropertyList *_newPropertyList(
			const UIPropertyGroup &propertyGroup, 
			const PropertyListMap::iterator &insertionPoint
		); 

		virtual void _freePropertyList(
			ObjectPropertiesEditor::PropertyList *pl,
			const PropertyListMap::iterator &listIter
		);

		// ------------------------------------------------------

		TreeItem m_treeParent;
		HTREEITEM  m_treeItem;

		// ------------------------------------------------------
	};
	// ==========================================================================

	// --------------------------------------------------------------------------
	virtual void onEditReset();
	virtual void onEditInsertSubtree(UIBaseObject &subTree, UIBaseObject *previousSibling);
	virtual void onEditRemoveSubtree(UIBaseObject &subTree);
	virtual void onEditMoveSubtree(UIBaseObject &subTree, UIBaseObject *previousSibling, UIBaseObject *oldParent);
	virtual void onEditSetObjectProperty(UIBaseObject &object, const char *i_propertyName);
	virtual void onSelect(UIBaseObject &object, bool isSelected);
	// --------------------------------------------------------------------------

	// --------------------------------------------------------------------------
	virtual ObjectPropertiesEditor::PropertyCategory *_createCategory(int categoryIndex);
	virtual void              _lock();
	virtual void              _unlock();
	// --------------------------------------------------------------------------

	// --------------------------------------------------------------------------
	void _retextTab(int category);
	void _retextAllTabs();
	void _retextDialog();
	// --------------------------------------------------------------------------

	// ==========================================================================

	HACCEL    accelerators;
	bool      showActive;
	CTreeCtrl m_propertyTree;

	// ==========================================================================

	// Generated message map functions
	//{{AFX_MSG(ObjectPropertiesTreeDialog)
	afx_msg void OnDestroy();
	virtual BOOL OnInitDialog();
	afx_msg void OnSize(UINT nType, int cx, int cy);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_OBJECTPROPERTIESTREEDIALOG_H__071096A8_2D31_4E01_A166_F5F51693BC52__INCLUDED_)
