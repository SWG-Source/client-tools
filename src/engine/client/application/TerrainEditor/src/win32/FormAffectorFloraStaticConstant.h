//
// FormAffectorFloraStaticConstant.h
// asommers
//
// copyright 2000, sony online entertainment
//

//-------------------------------------------------------------------

#ifndef INCLUDED_FormAffectorFloraStaticConstant_H
#define INCLUDED_FormAffectorFloraStaticConstant_H

//-------------------------------------------------------------------

class AffectorFloraStatic;

#include "FormLayerItem.h"
#include "SmartSliderCtrl.h"
#include "SmartEditCtrl.h"

//-------------------------------------------------------------------

class FormAffectorFloraStaticConstant : public FormLayerItem
{
private:

	AffectorFloraStatic*  affector;

private:

	//{{AFX_DATA(FormAffectorFloraStaticConstant)
	enum { IDD = IDD_FORM_AFFECTORFLORACONSTANT };
	SmartSliderCtrl	m_sliderDensity;
	SmartEditCtrl	m_editDensity;
	CButton	m_addButton;
	CButton	m_removeButton;
	CComboBox	m_familyCtl;
	BOOL	m_removeAllFlora;
	int		m_operation;
	BOOL	m_densityOverride;
	CString	m_name;
	CString	m_type;
	//}}AFX_DATA

protected:

	FormAffectorFloraStaticConstant();           
	DECLARE_DYNCREATE(FormAffectorFloraStaticConstant)
	virtual ~FormAffectorFloraStaticConstant();

#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

	//{{AFX_MSG(FormAffectorFloraStaticConstant)
	afx_msg void OnSelchangefamily();
	afx_msg void Onadd();
	afx_msg void OnRemove();
	afx_msg void OnCheckRemoveAllFlora();
	afx_msg void OnCheckDensityoverride();
	afx_msg void OnDestroy();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

public:

	virtual void Initialize (PropertyView::ViewData* vd);
	virtual bool HasChanged () const;
	virtual void ApplyChanges ();

	//{{AFX_VIRTUAL(FormAffectorFloraStaticConstant)
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
