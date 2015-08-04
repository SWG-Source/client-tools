//
// FormAffectorShaderReplace.h
// asommers
//
// copyright 2000, sony online entertainment
//

//-------------------------------------------------------------------

#ifndef INCLUDED_FormAffectorShaderReplace_H
#define INCLUDED_FormAffectorShaderReplace_H

//-------------------------------------------------------------------

class AffectorShaderReplace;

#include "FormLayerItem.h"
#include "SmartSliderCtrl.h"
#include "SmartEditCtrl.h"

//-------------------------------------------------------------------

class FormAffectorShaderReplace : public FormLayerItem
{
private:

	AffectorShaderReplace* affector;

private:

	//{{AFX_DATA(FormAffectorShaderReplace)
	enum { IDD = IDD_FORM_AFFECTORSHADERREPLACE };
	SmartSliderCtrl	m_sliderFeatherClamp;
	SmartEditCtrl	m_editFeatherClamp;
	BOOL	        m_featherClampOverride;
	CComboBox	m_familySource;
	CComboBox	m_familyDestination;
	CString	m_name;
	//}}AFX_DATA

protected:

	FormAffectorShaderReplace();           
	DECLARE_DYNCREATE(FormAffectorShaderReplace)
	virtual ~FormAffectorShaderReplace();

#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

	// Generated message map functions
	//{{AFX_MSG(FormAffectorShaderReplace)
	afx_msg void OnDestroy();
	afx_msg void OnSelchangeFamilyDestination();
	afx_msg void OnSelchangeFamilySource();
	afx_msg void OnCheckFeatherClampoverride();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

public:

	virtual void Initialize (PropertyView::ViewData* vd);
	virtual bool HasChanged () const;
	virtual void ApplyChanges ();

	//{{AFX_VIRTUAL(FormAffectorShaderReplace)
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
