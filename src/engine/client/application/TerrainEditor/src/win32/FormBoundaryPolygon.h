//
// FormBoundaryPolygon.h
// asommers
//
// copyright 2001, sony online entertainment
//

//-------------------------------------------------------------------

#ifndef INCLUDED_FormBoundaryPolygon_H
#define INCLUDED_FormBoundaryPolygon_H

//-------------------------------------------------------------------

class BoundaryPolygon;

#include "FormLayerItem.h"

//-------------------------------------------------------------------

class FormBoundaryPolygon : public FormLayerItem
{
private:

	BoundaryPolygon* boundary;

private:

	//{{AFX_DATA(FormBoundaryPolygon)
	enum { IDD = IDD_FORM_BOUNDARYPOLYGON };
	CEdit	m_waterShader;
	CButton	m_browseButton;
	NumberEdit	m_featherDistance;
	NumberEdit	m_waterShaderSize;
	NumberEdit	m_waterHeight;
	CComboBox	m_featherFunction;
	CString	m_name;
	BOOL	m_isWaterTable;
	CComboBox	    m_waterType;
	//}}AFX_DATA

protected:

	FormBoundaryPolygon();           
	DECLARE_DYNCREATE(FormBoundaryPolygon)

protected:

	virtual ~FormBoundaryPolygon();

#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

	//{{AFX_MSG(FormBoundaryPolygon)
	afx_msg void OnSelchangeFeatherFunction();
	afx_msg void OnButtonShaderBrowse();
	afx_msg void OnCheckIswatertable();
	afx_msg void OnDestroy();
	afx_msg void OnButtonEditControlPointList();
	afx_msg void OnSelchangeWaterType();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

public:

	virtual void Initialize (PropertyView::ViewData* vd);
	virtual bool HasChanged () const;
	virtual void ApplyChanges ();

	//{{AFX_VIRTUAL(FormBoundaryPolygon)
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
