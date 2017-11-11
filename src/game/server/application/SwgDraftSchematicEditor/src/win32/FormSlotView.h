// ======================================================================
//
// FormSlotView.h
// asommers
//
// copyright 2003, sony online entertainment
//
// ======================================================================

#ifndef INCLUDED_FormSlotView_H
#define INCLUDED_FormSlotView_H

// ======================================================================

#include "Resource.h"
#include "SwgDraftSchematicEditorDoc.h"

// ======================================================================

class FormSlotView : public CFormView
{
protected:

	FormSlotView();           
	DECLARE_DYNCREATE(FormSlotView)

public:

	//{{AFX_DATA(FormSlotView)
	enum { IDD = IDD_FORM_SLOTVIEW };
	CComboBox	m_nameStringTableComboBox;
	CEdit	m_nameStringIdEditCtrl;
	CComboBox	m_ingredientTypeComboBox;
	CButton	m_browseIngredientNameButton;
	BOOL	m_optional;
	CString	m_ingredientName;
	int		m_ingredientCount;
	CString	m_nameStringId;
	//}}AFX_DATA

public:

	//{{AFX_VIRTUAL(FormSlotView)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

protected:

	virtual ~FormSlotView();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

	//{{AFX_MSG(FormSlotView)
	afx_msg void OnButtonBrowseingredientname();
	afx_msg void OnCheckOptional();
	afx_msg void OnSelchangeComboIngredienttype();
	afx_msg void OnChangeEditIngredientcount();
	afx_msg void OnChangeEditNamestringid();
	afx_msg void OnEditCopy();
	afx_msg void OnEditCut();
	afx_msg void OnEditPaste();
	afx_msg void OnEditUndo();
	afx_msg void OnSelchangeComboNamestringtable();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

public:

	void setItemData (SwgDraftSchematicEditorDoc::ItemData * itemData);

private:

	SwgDraftSchematicEditorDoc::ItemData * m_itemData;
};

// ======================================================================

//{{AFX_INSERT_LOCATION}}

// ======================================================================

#endif 
