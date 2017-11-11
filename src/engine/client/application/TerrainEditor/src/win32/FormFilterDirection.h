//
// FormFilterDirection.h
// asommers
//
// copyright 2001, sony online entertainment

//-------------------------------------------------------------------

#ifndef INCLUDED_FormFilterDirection_H
#define INCLUDED_FormFilterDirection_H

//-------------------------------------------------------------------

class FilterDirection;

#include "FormLayerItem.h"
#include "SmartEditCtrl.h"
#include "SmartSliderCtrl.h"

//-------------------------------------------------------------------

class FormFilterDirection : public FormLayerItem
{
private:

	FilterDirection* m_filter;

private:

	//{{AFX_DATA(FormFilterDirection)
	enum { IDD = IDD_FORM_FILTERDIRECTION };
	NumberEdit	m_minimumAngleDegrees;
	NumberEdit	m_maximumAngleDegrees;
	SmartSliderCtrl	m_sliderFeatherDistance;
	SmartEditCtrl	m_editFeatherDistance;
	CComboBox	m_featherFunction;
	CString	m_name;
	//}}AFX_DATA

protected:

	FormFilterDirection();           
	DECLARE_DYNCREATE(FormFilterDirection)

protected:

	virtual ~FormFilterDirection();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

	//{{AFX_MSG(FormFilterDirection)
	afx_msg void OnSelchangeFeatherFunction();
	afx_msg void OnDestroy();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

public:

	virtual void Initialize (PropertyView::ViewData* vd);
	virtual bool HasChanged () const;
	virtual void ApplyChanges ();

	//{{AFX_VIRTUAL(FormFilterDirection)
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
