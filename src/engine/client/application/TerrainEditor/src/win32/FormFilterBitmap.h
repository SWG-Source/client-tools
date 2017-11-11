//
// FormFilterBitmap.h
//
// copyright 2004, sony online entertainment
//

//-------------------------------------------------------------------

#ifndef INCLUDED_FormFilterBitmap_H
#define INCLUDED_FormFilterBitmap_H

//-------------------------------------------------------------------

class FilterBitmap;

#include "FormLayerItem.h"
#include "SmartEditCtrl.h"
#include "SmartSliderCtrl.h"

//-------------------------------------------------------------------

class FormFilterBitmap : public FormLayerItem
{
private:

	FilterBitmap* m_filter;
	CToolTipCtrl   m_tooltip;

private:

	//{{AFX_DATA(FormFilterBitmap)
	enum { IDD = IDD_FORM_FILTERBITMAP };
	CComboBox	m_familyCtrl;
	SmartEditCtrl	m_editLow;
	SmartEditCtrl	m_editHigh;
	SmartEditCtrl   m_editFeatherDistance;
	SmartSliderCtrl m_sliderLow;
	SmartSliderCtrl m_sliderHigh;
	SmartSliderCtrl m_sliderFeatherDistance;
	SmartEditCtrl   m_editGain;
	SmartSliderCtrl m_sliderGain;
	CComboBox       m_featherFunction;
	CString	m_name;
	int		m_operation;
	//}}AFX_DATA

protected:

	FormFilterBitmap();           
	DECLARE_DYNCREATE(FormFilterBitmap)
	virtual ~FormFilterBitmap();

#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

	//{{AFX_MSG(FormFilterBitmap)
	afx_msg void OnSelchangeFeatherFunction();
	afx_msg void OnDestroy();
	afx_msg void OnSelchangeFamily();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

public:

	virtual void Initialize (PropertyView::ViewData* vd);
	virtual bool HasChanged () const;
	virtual void ApplyChanges ();

	//{{AFX_VIRTUAL(FormFilterBitmap)
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
