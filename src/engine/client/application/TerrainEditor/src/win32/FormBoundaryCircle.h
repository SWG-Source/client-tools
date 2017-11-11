//
// FormBoundaryCircle.h
// asommers
//
// copyright 2001, sony online entertainment
//

//-------------------------------------------------------------------

#ifndef INCLUDED_FormBoundaryCircle_H
#define INCLUDED_FormBoundaryCircle_H

//-------------------------------------------------------------------

class BoundaryCircle;

#include "FormLayerItem.h"
#include "SmartEditCtrl.h"
#include "SmartSliderCtrl.h"

//-------------------------------------------------------------------

class FormBoundaryCircle : public FormLayerItem
{
private:

	BoundaryCircle* boundary;

private:

	//{{AFX_DATA(FormBoundaryCircle)
	enum { IDD = IDD_FORM_BOUNDARYCIRCLE };
	NumberEdit	m_radius;
	NumberEdit	m_centerZ;
	NumberEdit	m_centerX;
	SmartSliderCtrl	m_sliderFeatherDistance;
	SmartEditCtrl	m_editFeatherDistance;
	CComboBox	m_featherFunction;
	CString	m_name;
	//}}AFX_DATA

protected:

	FormBoundaryCircle();           
	DECLARE_DYNCREATE(FormBoundaryCircle)

protected:

	virtual ~FormBoundaryCircle();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

	//{{AFX_MSG(FormBoundaryCircle)
	afx_msg void OnSelchangeFeatherFunction();
	afx_msg void OnDestroy();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

public:

	virtual void Initialize (PropertyView::ViewData* vd);
	virtual bool HasChanged () const;
	virtual void ApplyChanges ();

	void update (void);

	//{{AFX_VIRTUAL(FormBoundaryCircle)
	public:
	virtual void OnInitialUpdate();
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual void OnUpdate(CView* pSender, LPARAM lHint, CObject* pHint);
	//}}AFX_VIRTUAL
};

//-------------------------------------------------------------------

//{{AFX_INSERT_LOCATION}}

//-------------------------------------------------------------------

#endif 
