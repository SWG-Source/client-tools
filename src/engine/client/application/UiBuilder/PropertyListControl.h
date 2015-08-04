#if !defined(AFX_PROPERTYLISTCONTROL_H__3C33A076_C287_465D_B769_B4A53AE81765__INCLUDED_)
#define AFX_PROPERTYLISTCONTROL_H__3C33A076_C287_465D_B769_B4A53AE81765__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// PropertyListControl.h : header file

#include "UIString.h"

#include <set>

class UIBaseObject;

/////////////////////////////////////////////////////////////////////////////
class UIPropertyDescriptor;
class PropertyListControl;

/////////////////////////////////////////////////////////////////////////////

class PropertyListControlProperty
{
public:
	PropertyListControlProperty(
		PropertyListControl &owner,
		const UIPropertyDescriptor &i_descriptor
		)
	:	m_ownerControl(owner),
		m_descriptor(i_descriptor), 
		m_listIndex(0)
	{}

	virtual bool setValue(const CString &i_newValue);
	virtual void onRelease();
	virtual void onFollowLink();
	virtual UIBaseObject *getAnchor()=0;

	void refreshItemValue() const;

	UIString wideValue() const;
	const UINarrowString &narrowValue() const { return m_narrowValue; }

	PropertyListControl         &m_ownerControl;
	const UIPropertyDescriptor  &m_descriptor;
	mutable int                  m_listIndex;
	mutable UINarrowString       m_narrowValue;
};

/////////////////////////////////////////////////////////////////////////////
// PropertyListControl window

class PropertyListControl : public CListCtrl
{
public:

	typedef PropertyListControlProperty Property;
	friend class PropertyListControlProperty;

// Construction
public:

	// --------------------------------------------------------

	PropertyListControl();

	// --------------------------------------------------------

// Attributes
public:

	enum EHighlight { HIGHLIGHT_NORMAL, HIGHLIGHT_ALLCOLUMNS, HIGHLIGHT_ROW };

// Operations
public:

	int SetHighlightType(EHighlight hilite);

	// --------------------------------------------------------

	void clear();
	void addProperty(const Property &i_property);
	void removeProperty(const Property &i_property);

	bool empty() const;
	int  size() const;

	// --------------------------------------------------------

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(PropertyListControl)
	public:
	virtual void DrawItem(LPDRAWITEMSTRUCT lpDrawItemStruct);
	protected:
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~PropertyListControl();

	// --------------------------------------------------------

	BOOL DeleteAllItems()                          { return CListCtrl::DeleteAllItems(); }
	BOOL SubclassDlgItem(UINT nID, CWnd* pParent)  { return CListCtrl::SubclassDlgItem(nID, pParent); }

	// --------------------------------------------------------
protected:

	// --------------------------------------------------------

	void _edit(int item, CPoint loc, bool forceText);

	bool _contextMenu(int item, CPoint loc);

	void _repaintSelectedItems();
	void _onDeleteItem(int item);
	void _refreshItemValue(int item);
	void _reindexProperties(int item);
	void _initializeColumns();
	int  _itemHitTest(LVHITTESTINFO &results, CPoint *mouseScreen=0); // returns index of item hit or -1 if not item was hit.

	bool m_inited;
	int m_contextItem;
	CPoint m_contextLoc;
	int m_nHighlight;

	// --------------------------------------------------------
	// Generated message map functions
protected:
	//{{AFX_MSG(PropertyListControl)
	afx_msg void OnDblclk(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnPaint();
	afx_msg void OnSetFocus(CWnd* pOldWnd);
	afx_msg void OnKillFocus(CWnd* pNewWnd);
	afx_msg void OnDeleteallitems(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnDeleteitem(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnRclick(NMHDR* pNMHDR, LRESULT* pResult);
	//}}AFX_MSG
	afx_msg void OnFollowLink();
	afx_msg void OnEditProperty();

	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_PROPERTYLISTCONTROL_H__3C33A076_C287_465D_B769_B4A53AE81765__INCLUDED_)
