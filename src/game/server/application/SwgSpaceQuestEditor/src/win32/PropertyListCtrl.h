// ======================================================================
//
// PropertyListCtrl.h
// asommers
//
// copyright 2004, sony online entertainment
//
// ======================================================================

#ifndef INCLUDED_PropertyListCtrl_H
#define INCLUDED_PropertyListCtrl_H

// ======================================================================

#include "Configuration.h"

class NumberEdit;

// ======================================================================

#define ID_PROPERTY_TEXT                  1
#define ID_PROPERTY_BOOL                  2
#define ID_PROPERTY_STATIC                3
#define ID_PROPERTY_COMBO_BTN             4
#define ID_PROPERTY_COMBO_LIST            5
#define ID_PROPERTY_TEXT_SPACEMOBILE      6
#define ID_PROPERTY_TEXT_SPACEMOBILELIST  7
#define ID_PROPERTY_TEXT_NAVPOINT         8
#define ID_PROPERTY_TEXT_NAVPOINTLIST     9
#define ID_PROPERTY_TEXT_SPAWNER          10
#define ID_PROPERTY_TEXT_SPAWNERLIST      11
#define ID_PROPERTY_TEXT_CARGO            12
#define ID_PROPERTY_INTEGER               13
#define ID_PROPERTY_TEXT_QUEST            14
#define ID_PROPERTY_TEXT_OBJECTTEMPLATE   15
#define ID_PROPERTY_LAST                  16

#define ID_PROPERTY_CHANGED               11111

// ======================================================================

class PropertyListCtrl : public CListBox
{
public:

	class ParentInterface
	{
	public:

		virtual void onPropertyChanged(UINT hiControlId_loItemChanged, LONG lPropertyType) const=0;
	};

public:

	PropertyListCtrl(int controlId, bool boldSelection);
	virtual ~PropertyListCtrl();

	void setParentInterface(ParentInterface * parentInterface);
	bool addString(Configuration::MissionTemplate::PropertyTemplate const * m_propertyTemplate, CString const & csText, int nType, CString const & csData, CString const & tooltip, int nPropertySelected = 0, int nAlignment = DT_LEFT, bool bComboEditable = false, bool bComboSorted = false);
	void removeString(CString const & string);
	bool getPropertyName(int index, CString & propertyName) const;
	bool getProperty(int index, CString & text) const;

public:

	//{{AFX_VIRTUAL(PropertyListCtrl)
	virtual void DrawItem(LPDRAWITEMSTRUCT lpDrawItemStruct);
	virtual void MeasureItem(LPMEASUREITEMSTRUCT lpMeasureItemStruct);
	//}}AFX_VIRTUAL

protected:

	//{{AFX_MSG(PropertyListCtrl)
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnSelchange();
	afx_msg void OnDblclk();
	afx_msg void OnEditLostFocus();
	afx_msg void OnEditChange();
	afx_msg void OnNumberEditLostFocus();
	afx_msg void OnNumberEditChange();
	afx_msg void OnTextPropertyClick();
	afx_msg void OnQuestPropertyClick();
	afx_msg void OnObjectTemplatePropertyClick();
	afx_msg void OnSpaceMobilePropertyClick();
	afx_msg void OnSpaceMobileListPropertyClick();
	afx_msg void OnNavPointPropertyClick();
	afx_msg void OnNavPointListPropertyClick();
	afx_msg void OnSpawnerPropertyClick();
	afx_msg void OnSpawnerListPropertyClick();
	afx_msg void OnCargoPropertyClick();
	afx_msg void OnComboBoxClick();
	afx_msg void OnSelChange();
	afx_msg void OnListboxLostFocus();
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnVScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
	afx_msg void OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);
	//}}AFX_MSG

	DECLARE_MESSAGE_MAP()

private:

	class PropertyItem;
	typedef std::vector<PropertyItem *> PropertyItemList;

private:

	void drawItem(CDC * pDC, CRect itemRect, bool selected);
	void drawPropertyText(CDC * pDC, CRect itemRect);
	void createControl(int nType);
	bool setProperty(PropertyItem * propertyItem, CString const & data);
	void parseProperties(PropertyItem * propertyItem, CString const & data);
	void hideControls();

private:

	ParentInterface * m_parentInterface;
	int const m_controlId;
	bool const m_boldSelection;
	CFont * const m_font;
	CFont * const m_selectedFont;
	COLORREF const m_borderColor;
	COLORREF const m_backgroundColor;
	COLORREF const m_textColor;
	COLORREF const m_backgroundHighlightColor;
	COLORREF const m_textHighlightColor;

	int m_widestItem;
	bool m_changed;
	CPen * m_borderPen;
	CRect m_currentRect;
	CBrush * m_backgroundBrush;
	CBrush * m_backgroundHighlightBrush;
	CButton * m_questButton;
	CButton * m_objectTemplateButton;
	CButton * m_spaceMobileButton;
	CButton * m_spaceMobileListButton;
	CButton * m_navPointButton;
	CButton * m_navPointListButton;
	CButton * m_spawnerButton;
	CButton * m_spawnerListButton;
	CButton * m_cargoButton;

	class CComboButton;
	CComboButton * m_comboButton;
	CListBox * m_listBox;

	// Controls
	CEdit * m_editCtrl;
	NumberEdit * m_numberEditCtrl;

	// The item list
	CString m_text;
	PropertyItemList m_propertyItemList;
	PropertyItem * m_currentItem;
	PropertyItem * m_currentDrawItem;
};

#endif
