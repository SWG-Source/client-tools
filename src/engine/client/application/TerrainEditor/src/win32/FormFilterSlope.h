//
// FormFilterSlope.h
// asommers
//
// copyright 2001, sony online entertainment

//-------------------------------------------------------------------

#ifndef INCLUDED_FormFilterSlope_H
#define INCLUDED_FormFilterSlope_H

//-------------------------------------------------------------------

class FilterSlope;

#include "FormLayerItem.h"
#include "SmartEditCtrl.h"
#include "SmartSliderCtrl.h"

//-------------------------------------------------------------------

class FormFilterSlope : public FormLayerItem
{
private:

	FilterSlope* m_filter;

private:

	//{{AFX_DATA(FormFilterSlope)
	enum { IDD = IDD_FORM_FILTERSLOPE };
	NumberEdit	m_minimumAngleDegrees;
	NumberEdit	m_maximumAngleDegrees;
	SmartSliderCtrl	m_sliderFeatherDistance;
	SmartEditCtrl	m_editFeatherDistance;
	CComboBox	m_featherFunction;
	CString	m_name;
	//}}AFX_DATA

protected:

	FormFilterSlope();           
	DECLARE_DYNCREATE(FormFilterSlope)
	virtual ~FormFilterSlope();

#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

	//{{AFX_MSG(FormFilterSlope)
	afx_msg void OnSelchangeFeatherFunction();
	afx_msg void OnDestroy();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

public:

	virtual void Initialize (PropertyView::ViewData* vd);
	virtual bool HasChanged () const;
	virtual void ApplyChanges ();

	//{{AFX_VIRTUAL(FormFilterSlope)
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
