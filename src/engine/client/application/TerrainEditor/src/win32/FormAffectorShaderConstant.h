//
// FormAffectorShaderConstant.h
// asommers
//
// copyright 2000, sony online entertainment
//

//-------------------------------------------------------------------

#ifndef INCLUDED_FormAffectorShaderConstant_H
#define INCLUDED_FormAffectorShaderConstant_H

//-------------------------------------------------------------------

class AffectorShaderConstant;

#include "FormLayerItem.h"
#include "SmartSliderCtrl.h"
#include "SmartEditCtrl.h"

//-------------------------------------------------------------------

class FormAffectorShaderConstant : public FormLayerItem
{
private:

	AffectorShaderConstant* affector;

private:

	//{{AFX_DATA(FormAffectorShaderConstant)
	enum { IDD = IDD_FORM_AFFECTORSHADERCONSTANT };
	SmartSliderCtrl	m_sliderFeatherClamp;
	SmartEditCtrl	m_editFeatherClamp;
	CComboBox	m_familyCtl;
	BOOL	m_featherClampOverride;
	CString	m_name;
	//}}AFX_DATA

protected:

	FormAffectorShaderConstant();           
	DECLARE_DYNCREATE(FormAffectorShaderConstant)
	virtual ~FormAffectorShaderConstant();

#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

	// Generated message map functions
	//{{AFX_MSG(FormAffectorShaderConstant)
	afx_msg void OnSelchangefamily();
	afx_msg void OnCheckFeatherClampoverride();
	afx_msg void OnDestroy();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

public:

	virtual void Initialize (PropertyView::ViewData* vd);
	virtual bool HasChanged () const;
	virtual void ApplyChanges ();

	//{{AFX_VIRTUAL(FormAffectorShaderConstant)
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
