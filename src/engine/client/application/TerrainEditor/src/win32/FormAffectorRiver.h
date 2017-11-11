//
// FormAffectorRiver.h
// asommers
//
// copyright 2000, sony online entertainment
//

//-------------------------------------------------------------------

#ifndef INCLUDED_FormAffectorRiver_H
#define INCLUDED_FormAffectorRiver_H

//-------------------------------------------------------------------

class AffectorRiver;

#include "FormLayerItem.h"
#include "SmartEditCtrl.h"
#include "SmartSliderCtrl.h"

//-------------------------------------------------------------------

class FormAffectorRiver : public FormLayerItem
{
private:

	AffectorRiver* affector;

private:

	//{{AFX_DATA(FormAffectorRiver)
	enum { IDD = IDD_FORM_AFFECTORRIVER };
	CComboBox	m_familyBottom;
	CComboBox	m_familyBank;
	CEdit	m_waterShader;
	NumberEdit	m_waterDepth;
	NumberEdit	m_waterWidth;
	NumberEdit	m_trenchDepth;
	CButton	m_browse;
	NumberEdit	m_width;
	NumberEdit	m_velocity;
	NumberEdit	m_waterShaderSize;
	SmartSliderCtrl	m_sliderFeatherDistance;
	SmartEditCtrl	m_editFeatherDistance;
	CComboBox	m_featherFunction;
	CString	m_name;
	BOOL	m_hasWater;
	CComboBox m_waterType;
	//}}AFX_DATA

protected:

	FormAffectorRiver();           
	DECLARE_DYNCREATE(FormAffectorRiver)
	virtual ~FormAffectorRiver();

#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

	// Generated message map functions
	//{{AFX_MSG(FormAffectorRiver)
	afx_msg void OnSelchangeFeatherFunction();
	afx_msg void OnButtonShaderBrowse();
	afx_msg void OnDestroy();
	afx_msg void OnCheckHaswater();
	afx_msg void OnSelchangeFamilyBank();
	afx_msg void OnSelchangeFamilyBottom();
	afx_msg void OnButtonEditControlPointList();
	afx_msg void OnSelchangeWaterType();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

public:

	virtual void Initialize (PropertyView::ViewData* vd);
	virtual bool HasChanged () const;
	virtual void ApplyChanges ();

	//{{AFX_VIRTUAL(FormAffectorRiver)
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
