//
// FormAffectorRoad.h
// asommers
//
// copyright 2000, sony online entertainment
//

//-------------------------------------------------------------------

#ifndef INCLUDED_FormAffectorRoad_H
#define INCLUDED_FormAffectorRoad_H

//-------------------------------------------------------------------

class AffectorRoad;

#include "FormLayerItem.h"
#include "SmartEditCtrl.h"
#include "SmartSliderCtrl.h"

//-------------------------------------------------------------------

class FormAffectorRoad : public FormLayerItem
{
private:

	AffectorRoad* affector;

private:

	//{{AFX_DATA(FormAffectorRoad)
	enum { IDD = IDD_FORM_AFFECTORROAD };
	SmartSliderCtrl	m_sliderFeatherDistanceShader;
	SmartEditCtrl	m_editFeatherDistanceShader;
	SmartSliderCtrl	m_sliderFeatherDistance;
	SmartEditCtrl	m_editFeatherDistance;
	NumberEdit	    m_width;
	CComboBox	    m_featherFunctionShader;
	CComboBox	    m_featherFunction;
	CComboBox	    m_familyCtl;
	BOOL			m_hasFixedHeights;
	CString	        m_name;
	//}}AFX_DATA

protected:

	FormAffectorRoad();           
	DECLARE_DYNCREATE(FormAffectorRoad)
	virtual ~FormAffectorRoad();

#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

	// Generated message map functions
	//{{AFX_MSG(FormAffectorRoad)
	afx_msg void OnSelchangeFamily();
	afx_msg void OnSelchangeFeatherFunction();
	afx_msg void OnDestroy();
	afx_msg void OnButtonEditControlPointList();
	afx_msg void OnSelchangeFeatherFunctionshader();
	afx_msg void OnCheckHasFixedHeights();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

public:

	virtual void Initialize (PropertyView::ViewData* vd);
	virtual bool HasChanged () const;
	virtual void ApplyChanges ();

	//{{AFX_VIRTUAL(FormAffectorRoad)
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
