//
// FormFilterFractal.h
// asommers
//
// copyright 2001, sony online entertainment
//

//-------------------------------------------------------------------

#ifndef INCLUDED_FormFilterFractal_H
#define INCLUDED_FormFilterFractal_H

//-------------------------------------------------------------------

class FilterFractal;

#include "FormLayerItem.h"
#include "SmartEditCtrl.h"
#include "SmartSliderCtrl.h"

//-------------------------------------------------------------------

class FormFilterFractal : public FormLayerItem
{
private:

	FilterFractal* m_filter;
	CToolTipCtrl   m_tooltip;

private:

	//{{AFX_DATA(FormFilterFractal)
	enum { IDD = IDD_FORM_FILTERFRACTAL };
	CComboBox	m_familyCtrl;
	SmartEditCtrl	m_editLow;
	SmartEditCtrl	m_editHigh;
	SmartEditCtrl   m_editFeatherDistance;
	SmartSliderCtrl m_sliderLow;
	SmartSliderCtrl m_sliderHigh;
	SmartSliderCtrl m_sliderFeatherDistance;
	CComboBox       m_featherFunction;
	CString	m_name;
	int		m_operation;
	//}}AFX_DATA

protected:

	FormFilterFractal();           
	DECLARE_DYNCREATE(FormFilterFractal)
	virtual ~FormFilterFractal();

#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

	//{{AFX_MSG(FormFilterFractal)
	afx_msg void OnSelchangeFeatherFunction();
	afx_msg void OnDestroy();
	afx_msg void OnSelchangeFamily();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

public:

	virtual void Initialize (PropertyView::ViewData* vd);
	virtual bool HasChanged () const;
	virtual void ApplyChanges ();

	//{{AFX_VIRTUAL(FormFilterFractal)
	public:
	virtual void OnInitialUpdate();
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL
};

//-------------------------------------------------------------------

//{{AFX_INSERT_LOCATION}}

//-------------------------------------------------------------------

#endif 
