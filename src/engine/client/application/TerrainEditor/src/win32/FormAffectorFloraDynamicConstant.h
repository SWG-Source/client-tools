//
// FormAffectorFloraDynamicConstant.h
// asommers
//
// copyright 2001, sony online entertainment
//

//-------------------------------------------------------------------

#ifndef INCLUDED_FormAffectorFloraDynamicConstant_H
#define INCLUDED_FormAffectorFloraDynamicConstant_H

//-------------------------------------------------------------------

class AffectorFloraDynamic;

#include "FormLayerItem.h"
#include "SmartSliderCtrl.h"
#include "SmartEditCtrl.h"

//-------------------------------------------------------------------

class FormAffectorFloraDynamicConstant : public FormLayerItem
{
private:

	AffectorFloraDynamic* affector;

private:

	//{{AFX_DATA(FormAffectorFloraDynamicConstant)
	enum { IDD = IDD_FORM_AFFECTORRADIALCONSTANT };
	SmartSliderCtrl	m_sliderDensity;
	SmartEditCtrl	m_editDensity;
	CButton	m_addButton;
	CButton	m_removeButton;
	CComboBox	m_familyCtl;
	BOOL	m_removeAllRadial;
	int		m_operation;
	BOOL	m_densityOverride;
	CString	m_name;
	CString	m_type;
	//}}AFX_DATA

protected:

	FormAffectorFloraDynamicConstant();           
	DECLARE_DYNCREATE(FormAffectorFloraDynamicConstant)
	virtual ~FormAffectorFloraDynamicConstant();

#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

	//{{AFX_MSG(FormAffectorFloraDynamicConstant)
	afx_msg void OnSelchangefamily();
	afx_msg void Onadd();
	afx_msg void OnRemove();
	afx_msg void OnCheckRemoveAllRadial();
	afx_msg void OnCheckDensityoverride();
	afx_msg void OnDestroy();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

public:

	virtual void Initialize (PropertyView::ViewData* vd);
	virtual bool HasChanged () const;
	virtual void ApplyChanges ();

	//{{AFX_VIRTUAL(FormAffectorFloraDynamicConstant)
	public:
	virtual void OnInitialUpdate();
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL
};

//-------------------------------------------------------------------

//{{AFX_INSERT_LOCATION}}

//-------------------------------------------------------------------

#endif 


