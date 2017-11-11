// ======================================================================
//
// FormPropertyView.h
// asommers
//
// copyright 2003, sony online entertainment
//
// ======================================================================

#ifndef INCLUDED_FormPropertyView_H
#define INCLUDED_FormPropertyView_H

// ======================================================================

#include "Resource.h"

// ======================================================================

class FormPropertyView : public CFormView
{
protected:

	FormPropertyView();           
	DECLARE_DYNCREATE(FormPropertyView)

public:

	//{{AFX_DATA(FormPropertyView)
	enum { IDD = IDD_FORM_PROPERTYVIEW };
	CButton	m_overrideDestroyIngredientsButton;
	CComboBox	m_overrideCraftingTypeComboBox;
	CComboBox	m_overrideXpTypeComboBox;
	CComboBox	m_serverBaseDraftSchematicCtrl;
	CButton	m_browseSharedCraftedObjectTemplate;
	CButton	m_browseServerCraftedObjectTemplate;
	CButton	m_browseManufactureScriptButton;
	CString	m_manufactureScript;
	CString	m_serverCraftedObjectTemplate;
	CString	m_sharedCraftedObjectTemplate;
	BOOL	m_overrideCraftingType;
	BOOL	m_overrideXpType;
	BOOL	m_overrideDestroyIngredients;
	BOOL	m_overrideDestroyIngredientsValue;
	int		m_complexity;
	int		m_itemsPerContainer;
	//}}AFX_DATA

public:

	//{{AFX_VIRTUAL(FormPropertyView)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

protected:

	virtual ~FormPropertyView();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

	//{{AFX_MSG(FormPropertyView)
	afx_msg void OnCheckOverridecraftingtype();
	afx_msg void OnCheckOverridedestroyingredient();
	afx_msg void OnCheckOverridexptype();
	afx_msg void OnButtonBrowsemanufacturescript();
	afx_msg void OnButtonBrowseservercraftedobjecttemplate();
	afx_msg void OnButtonBrowsesharedcraftedobjecttemplate();
	afx_msg void OnCheckOverridedestroyingredientvalue();
	afx_msg void OnSelchangeComboOverridecraftingtypevalue();
	afx_msg void OnSelchangeComboOverridexptypevalue();
	afx_msg void OnSelchangeComboServerbasedraftschematic();
	afx_msg void OnButtonGeneratesharedcraftedobjecttemplatename();
	afx_msg void OnEditCopy();
	afx_msg void OnEditCut();
	afx_msg void OnEditPaste();
	afx_msg void OnEditUndo();
	afx_msg void OnChangeEditComplexity();
	afx_msg void OnChangeEditItemspercontainer();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

public:

	void update ();
};

// ======================================================================

//{{AFX_INSERT_LOCATION}}

// ======================================================================

#endif 
