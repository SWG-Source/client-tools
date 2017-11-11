//
// FormAffectorEnvironment.h
// asommers
//
// copyright 2000, sony online entertainment
//

//-------------------------------------------------------------------

#ifndef INCLUDED_FormAffectorEnvironment_H
#define INCLUDED_FormAffectorEnvironment_H

//-------------------------------------------------------------------

class AffectorEnvironment;

#include "FormLayerItem.h"
#include "SmartSliderCtrl.h"
#include "SmartEditCtrl.h"

//-------------------------------------------------------------------

class FormAffectorEnvironment : public FormLayerItem
{
private:

	AffectorEnvironment* affector;

private:

	//{{AFX_DATA(FormAffectorEnvironment)
	enum { IDD = IDD_FORM_AFFECTORENVIRONMENT };
	SmartSliderCtrl	m_sliderFeatherClamp;
	SmartEditCtrl	m_editFeatherClamp;
	CComboBox	m_familyCtl;
	BOOL	m_featherClampOverride;
	CString	m_name;
	//}}AFX_DATA

protected:

	FormAffectorEnvironment();           
	DECLARE_DYNCREATE(FormAffectorEnvironment)
	virtual ~FormAffectorEnvironment();

#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

	// Generated message map functions
	//{{AFX_MSG(FormAffectorEnvironment)
	afx_msg void OnSelchangefamily();
	afx_msg void OnCheckFeatherClampoverride();
	afx_msg void OnDestroy();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

public:

	virtual void Initialize (PropertyView::ViewData* vd);
	virtual bool HasChanged () const;
	virtual void ApplyChanges ();

	//{{AFX_VIRTUAL(FormAffectorEnvironment)
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
