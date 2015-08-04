//
// FormAffectorHeightTerrace.h
// asommers
//
// copyright 2001, sony online entertainment
//

//-------------------------------------------------------------------

#ifndef INCLUDED_FormAffectorHeightTerrace_H
#define INCLUDED_FormAffectorHeightTerrace_H

//-------------------------------------------------------------------

class AffectorHeightTerrace;

#include "FormLayerItem.h"
#include "SmartEditCtrl.h"
#include "SmartSliderCtrl.h"

//-------------------------------------------------------------------

class FormAffectorHeightTerrace : public FormLayerItem
{
private:

	AffectorHeightTerrace* m_affector;

private:

	//{{AFX_DATA(FormAffectorHeightTerrace)
	enum { IDD = IDD_FORM_AFFECTORHEIGHTTERRACE };
	NumberEdit	m_height;
	SmartEditCtrl   m_editFraction;
	CString	m_name;
	SmartSliderCtrl	m_sliderFraction;
	//}}AFX_DATA

protected:

	FormAffectorHeightTerrace();           
	DECLARE_DYNCREATE(FormAffectorHeightTerrace)
	virtual ~FormAffectorHeightTerrace();

#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

	//{{AFX_MSG(FormAffectorHeightTerrace)
	afx_msg void OnDestroy();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

public:

	virtual void Initialize (PropertyView::ViewData* vd);
	virtual bool HasChanged () const;
	virtual void ApplyChanges ();

	//{{AFX_VIRTUAL(FormAffectorHeightTerrace)
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
