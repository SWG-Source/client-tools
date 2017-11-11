//
// FormBoundaryRect.h
// asommers
//
// copyright 2001, sony online entertainment
//

//-------------------------------------------------------------------

#ifndef INCLUDED_FormBoundaryRect_H
#define INCLUDED_FormBoundaryRect_H

//-------------------------------------------------------------------

class BoundaryRectangle;

#include "FormLayerItem.h"
#include "SmartEditCtrl.h"
#include "SmartSliderCtrl.h"

//-------------------------------------------------------------------

class FormBoundaryRect : public FormLayerItem
{
private:

	BoundaryRectangle* boundary;

private:

	//{{AFX_DATA(FormBoundaryRect)
	enum { IDD = IDD_FORM_BOUNDARYRECT };
	CButton	m_localGlobalWaterTable;
	CButton	m_browseButton;
	CEdit	m_waterShader;
	NumberEdit	m_z1;
	NumberEdit	m_z0;
	NumberEdit	m_x1;
	NumberEdit	m_x0;
	SmartSliderCtrl	m_sliderFeatherDistance;
	SmartEditCtrl	m_editFeatherDistance;
	CComboBox	m_featherFunction;
	CString	m_name;
	BOOL	m_isWaterTable;
	NumberEdit	m_waterHeight;
	NumberEdit	m_waterShaderSize;
	BOOL	m_isLocalGlobalWaterTable;
	CComboBox	    m_waterType;
	//}}AFX_DATA

protected:

	FormBoundaryRect();           
	DECLARE_DYNCREATE(FormBoundaryRect)

protected:

	virtual ~FormBoundaryRect();

#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

	//{{AFX_MSG(FormBoundaryRect)
	afx_msg void OnSelchangeFeatherFunction();
	afx_msg void OnDestroy();
	afx_msg void OnButtonShaderBrowse();
	afx_msg void OnCheckIswatertable();
	afx_msg void OnCheckLocalglobalwatertable();
	afx_msg void OnSelchangeWaterType();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

public:

	virtual void Initialize (PropertyView::ViewData* vd);
	virtual bool HasChanged () const;
	virtual void ApplyChanges ();

	void update (void);

	//{{AFX_VIRTUAL(FormBoundaryRect)
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
