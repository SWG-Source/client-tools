//
// FormBoundaryPolyline.h
// asommers
//
// copyright 2001, sony online entertainment
//

//-------------------------------------------------------------------

#ifndef INCLUDED_FormBoundaryPolyline_H
#define INCLUDED_FormBoundaryPolyline_H

//-------------------------------------------------------------------

class BoundaryPolyline;

#include "FormLayerItem.h"
#include "SmartEditCtrl.h"
#include "SmartSliderCtrl.h"

//-------------------------------------------------------------------

class FormBoundaryPolyline : public FormLayerItem
{
private:

	BoundaryPolyline* boundary;

private:

	//{{AFX_DATA(FormBoundaryPolyline)
	enum { IDD = IDD_FORM_BOUNDARYPOLYLINE };
	NumberEdit	m_width;
	SmartSliderCtrl	m_sliderFeatherDistance;
	SmartEditCtrl	m_editFeatherDistance;
	CComboBox	m_featherFunction;
	CString	m_name;
	//}}AFX_DATA

protected:
	
	FormBoundaryPolyline();           
	DECLARE_DYNCREATE(FormBoundaryPolyline)
	virtual ~FormBoundaryPolyline();

#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

	//{{AFX_MSG(FormBoundaryPolyline)
	afx_msg void OnSelchangeFeatherFunction();
	afx_msg void OnDestroy();
	afx_msg void OnButtonEditControlPointList();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

public:

	virtual void Initialize (PropertyView::ViewData* vd);
	virtual bool HasChanged () const;
	virtual void ApplyChanges ();

	//{{AFX_VIRTUAL(FormBoundaryPolyline)
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
