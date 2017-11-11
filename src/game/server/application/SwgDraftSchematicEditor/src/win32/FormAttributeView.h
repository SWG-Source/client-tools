// ======================================================================
//
// FormAttributeView.h
// asommers
//
// copyright 2003, sony online entertainment
//
// ======================================================================

#ifndef INCLUDED_FormAttributeView_H
#define INCLUDED_FormAttributeView_H

// ======================================================================

#include "Resource.h"
#include "SwgDraftSchematicEditorDoc.h"

// ======================================================================

class FormAttributeView : public CFormView
{
protected:

	FormAttributeView();           
	DECLARE_DYNCREATE(FormAttributeView)

public:

	//{{AFX_DATA(FormAttributeView)
	enum { IDD = IDD_FORM_ATTRIBUTEVIEW };
	CEdit	m_experimentStringTableEditCtrl;
	CEdit	m_experimentStringIdEditCtrl;
	CEdit	m_nameStringTableEditCtrl;
	CEdit	m_nameStringIdEditCtrl;
	CEdit	m_minimumEditCtrl;
	CEdit	m_maximumEditCtrl;
	CComboBox	m_minimumComboBox;
	CComboBox	m_maximumComboBox;
	int		m_valueType;
	CString	m_maximumValue;
	CString	m_minimumValue;
	CString	m_nameStringId;
	CString	m_nameStringTable;
	CString	m_experimentStringId;
	CString	m_experimentStringTable;
	//}}AFX_DATA

public:

	//{{AFX_VIRTUAL(FormAttributeView)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

protected:

	virtual ~FormAttributeView();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

	//{{AFX_MSG(FormAttributeView)
	afx_msg void OnRadioValuetype();
	afx_msg void OnRadioValuetypearmorrating();
	afx_msg void OnRadioValuetypedamagetype();
	afx_msg void OnSelchangeComboMaximum();
	afx_msg void OnSelchangeComboMinimum();
	afx_msg void OnChangeEditExperimentstringid();
	afx_msg void OnChangeEditMaximum();
	afx_msg void OnChangeEditMinimum();
	afx_msg void OnChangeEditNamestringid();
	afx_msg void OnEditCopy();
	afx_msg void OnEditCut();
	afx_msg void OnEditPaste();
	afx_msg void OnEditUndo();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

public:

	void setItemData (SwgDraftSchematicEditorDoc::ItemData * itemData);

private:

	void update ();

private:

	SwgDraftSchematicEditorDoc::ItemData * m_itemData;
};

// ======================================================================

//{{AFX_INSERT_LOCATION}}

// ======================================================================

#endif 
