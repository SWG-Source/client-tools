//
// FormAffectorRibbon.h
//
// copyright 2005 Sony Online Entertainment
//

//-------------------------------------------------------------------

#ifndef INCLUDED_FormAffectorRibbon_H
#define INCLUDED_FormAffectorRibbon_H

//-------------------------------------------------------------------

class AffectorRibbon;

#include "FormLayerItem.h"
#include "SmartEditCtrl.h"
#include "SmartSliderCtrl.h"

//-------------------------------------------------------------------

class FormAffectorRibbon : public FormLayerItem
{
private:

	AffectorRibbon* affector;

private:

	//{{AFX_DATA(FormAffectorRibbon)
	enum { IDD = IDD_FORM_AFFECTORRIBBON };
	NumberEdit	    m_width;
	NumberEdit      m_capWidth;
	NumberEdit      m_waterShaderSize;
	NumberEdit      m_velocity;
	CString	        m_name;
	CButton	m_browse;
	CEdit	m_waterShader;
	CComboBox	    m_waterType;
	SmartSliderCtrl	m_sliderFeatherDistanceTerrainShader;
	CComboBox	    m_featherFunctionTerrainShader;
	SmartEditCtrl	m_editFeatherDistanceTerrainShader;
	CComboBox	    m_terrainShaderFamilyCtl;
	//}}AFX_DATA

protected:

	FormAffectorRibbon();
	DECLARE_DYNCREATE(FormAffectorRibbon)
	virtual ~FormAffectorRibbon();

#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

	// Generated message map functions
	//{{AFX_MSG(FormAffectorRibbon)
	afx_msg void OnDestroy();
	afx_msg void OnButtonEditControlPointList();
	afx_msg void OnButtonWaterShaderBrowse();
	afx_msg void OnSelchangeWaterType();
	afx_msg void OnSelchangeFeatherFunctionTerrainShader();
	afx_msg void OnSelchangeTerrainShaderFamily();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

public:

	virtual void Initialize (PropertyView::ViewData* vd);
	virtual bool HasChanged () const;
	virtual void ApplyChanges ();

	//{{AFX_VIRTUAL(FormAffectorRibbon)
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
