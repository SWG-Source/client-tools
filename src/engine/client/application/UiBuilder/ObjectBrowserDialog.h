#ifndef __OBJECTBROWSERDIALOG_H__
#define __OBJECTBROWSERDIALOG_H__

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// ObjectBrowserDialog.h : header file
//

#include "CTreeCtrlEx.h"
#include "resource.h"
#include "EditorMonitor.h"

#include <map>

class UIBaseObject;
class ObjectEditor;

/////////////////////////////////////////////////////////////////////////////
// ObjectBrowserDialog dialog
class ObjectBrowserDialog : public CDialog, public EditorMonitor
{
public:

	ObjectBrowserDialog(ObjectEditor &i_model, CWnd* pParent, CWnd* pOwner);
   ~ObjectBrowserDialog();

// Dialog Data
	//{{AFX_DATA(ObjectBrowserDialog)
	enum { IDD = IDD_OBJECTBROWSERDIALOG };
		// NOTE: the ClassWizard will add data members here
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides                     
	//{{AFX_VIRTUAL(ObjectBrowserDialog)
	public:
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual void PostNcDestroy();
	virtual LRESULT WindowProc(UINT message, WPARAM wParam, LPARAM lParam);
	//}}AFX_VIRTUAL

// Implementation

public:

	// --------------------------------------------------------------------------
   // lock the dialog to temporarily prevent redraws during periods where numerous events will be generated.
   virtual void lock(); 

	// --------------------------------------------------------------------------
   // unlock the dialog to allow redraws. lock and unlock are reference-count based to each call to lock
   // must have a corresponding call to unlock.
   virtual void unlock();

   // Editor object-related change events are passed here.
	virtual void onEditReset();
	virtual void onEditInsertSubtree(UIBaseObject &subTree, UIBaseObject *previousSibling);
	virtual void onEditRemoveSubtree(UIBaseObject &subTree);
	virtual void onEditMoveSubtree(UIBaseObject &subTree, UIBaseObject *previousSibling, UIBaseObject *oldParent);
	virtual void onEditSetObjectProperty(UIBaseObject &object, const char *i_propertyName);
	virtual void onSelect(UIBaseObject &object, bool isSelected);

	// --------------------------------------------------------------------------

	void setAcceleratorTable(HACCEL x) { accelerators=x; }

	void saveUserPreferences();

	void setActiveAppearance(bool i_showActive);

protected:

   void _setHandle(UIBaseObject &object, HTREEITEM h);
   HTREEITEM _getHandle(UIBaseObject &object);
   UIBaseObject *_getObject(HTREEITEM h) { return (UIBaseObject *)(tree.GetItemData(h)); }
   void _associate(UIBaseObject &object, HTREEITEM h);
   void _dissociate(UIBaseObject &object);

	void _makeDisplayString(char *o_string, UIBaseObject &i_object);
	int  _getImage(UIBaseObject &i_object);

   void _insertObject(UIBaseObject &object, UIBaseObject *previousSibling, bool recurse);
   void _removeObject(UIBaseObject &object, bool recurse);
   void _retextObject(UIBaseObject &object);

   void _removeTree();
   void _setRoot(UIBaseObject *object);
   bool _isAttached() { return root!=0; }

   bool _beginDrag(HTREEITEM h, CPoint p);
   void _endDrag();
   bool _isDragging() { return dragitem!=0; }
   void _performDragDrop();
   void _contextMenu(HTREEITEM h, CPoint pt);

	bool _r_collapseUnselected(HTREEITEM item);

   void _updateSelect(HTREEITEM item, int state);

	void OnOK();
	void OnCancel();

	// Generated message map functions
	//{{AFX_MSG(ObjectBrowserDialog)
	virtual BOOL OnInitDialog();
	afx_msg void OnDestroy();
	afx_msg void OnBegindragObjecttree(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnSelchangedObjecttree(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnContextMenu(CWnd* pWnd, CPoint point);
	afx_msg void OnSelchangingObjecttree(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnEndlabeledit(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnClose();
	//}}AFX_MSG

   // handlers for the standard context-sensitive menu operations.
   void OnDefaultParent();
   void OnRenameobject();
   void OnShowtoplevelpage();
   void OnShowobject();
   void OnHideobject();
	void OnCollapseAll();
	void OnClearCollapseAll();
	void OnCollapse();
	void OnFullyExpand();

	DECLARE_MESSAGE_MAP()

	typedef std::map<UIBaseObject *, HTREEITEM> TreeItemLookup;

   ObjectEditor &model;
	TreeItemLookup uiTreeItems;
   CTreeCtrlEx tree;
   CImageList iconlist;
	HACCEL accelerators;
   HTREEITEM dragitem, drop_target, context_target;
   UIBaseObject *root;
   HCURSOR prev_cursor;
   int lock_ref;
   bool supress_sel_changes;
	bool showActive;
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif
