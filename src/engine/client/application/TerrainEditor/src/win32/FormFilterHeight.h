//
// FormFilterHeight.h
// asommers
//
// copyright 2000, sony online entertainment
//

//-------------------------------------------------------------------

#ifndef INCLUDED_FormFilterHeight_H
#define INCLUDED_FormFilterHeight_H

//-------------------------------------------------------------------

class FilterHeight;

#include "FormLayerItem.h"
#include "SmartEditCtrl.h"
#include "SmartSliderCtrl.h"

//-------------------------------------------------------------------

class FormFilterHeight : public FormLayerItem
{
private:

	FilterHeight* m_filter;

private:

	//{{AFX_DATA(FormFilterHeight)
	enum { IDD = IDD_FORM_FILTERHEIGHT };
	NumberEdit	m_lowHeight;
	NumberEdit	m_highHeight;
	SmartSliderCtrl	m_sliderFeatherDistance;
	SmartEditCtrl	m_editFeatherDistance;
	CComboBox	m_featherFunction;
	CString	m_name;
	//}}AFX_DATA

protected:

	FormFilterHeight();           
	DECLARE_DYNCREATE(FormFilterHeight)
	virtual ~FormFilterHeight();

#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

	//{{AFX_MSG(FormFilterHeight)
	afx_msg void OnSelchangeFeatherFunction();
	afx_msg void OnDestroy();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

public:

	virtual void Initialize (PropertyView::ViewData* vd);
	virtual bool HasChanged () const;
	virtual void ApplyChanges ();

	//{{AFX_VIRTUAL(FormFilterHeight)
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
