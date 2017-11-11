//
// FormFractalFamily.h
// asommers
//
// copyright 2001, sony online entertainment
//

//-------------------------------------------------------------------

#ifndef INCLUDED_FormFractalFamily_H
#define INCLUDED_FormFractalFamily_H

//-------------------------------------------------------------------

class FractalGroup;
class MultiFractal;

#include "PropertyView.h"
#include "SmartEditCtrl.h"
#include "SmartSliderCtrl.h"

//-------------------------------------------------------------------

class FormFractalFamily : public PropertyView
{
public:

	class FormFractalFamilyViewData : public PropertyView::ViewData
	{
	public:

		int           familyId;     //lint !e1925  //-- public data member
		FractalGroup* fractalGroup;  //lint !e1925  //-- public data member

	public:

		FormFractalFamilyViewData ();
		virtual ~FormFractalFamilyViewData ();
	};

private:

	FormFractalFamilyViewData data;

	MultiFractal*  m_multiFractal;
	CToolTipCtrl   m_tooltip;

private:

	//{{AFX_DATA(FormFractalFamily)
	enum { IDD = IDD_FORM_FRACTALFAMILY };
	NumberEdit	    m_editSeed;
	CSpinButtonCtrl	m_spinSeed;
	NumberEdit	    m_offsetZ;
	NumberEdit	    m_offsetX;
	SmartEditCtrl   m_editOctaves;
	SmartEditCtrl	m_editScaleZ;
	SmartEditCtrl	m_editScaleX;
	SmartEditCtrl   m_editGain;
	SmartEditCtrl   m_editBias;
	SmartEditCtrl   m_editAmplitude;
	SmartEditCtrl   m_editFrequency;
	SmartSliderCtrl	m_sliderOctaves;
	SmartSliderCtrl	m_sliderScaleZ;
	SmartSliderCtrl	m_sliderScaleX;
	SmartSliderCtrl	m_sliderGain;
	SmartSliderCtrl	m_sliderFrequency;
	SmartSliderCtrl	m_sliderBias;
	SmartSliderCtrl	m_sliderAmplitude;
	CComboBox       m_combinationRule;
	CString	m_name;
	BOOL	m_useBias;
	BOOL	m_useGain;
	//}}AFX_DATA

private:

	void updateBitmap () const;

protected:

	FormFractalFamily();           
	DECLARE_DYNCREATE(FormFractalFamily)
	virtual ~FormFractalFamily();

#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

	//{{AFX_MSG(FormFractalFamily)
	afx_msg void OnCheckBias();
	afx_msg void OnChangeSeed();
	afx_msg void OnSelchangeCombinationrule();
	afx_msg void OnCheckGain();
	afx_msg void OnDestroy();
	afx_msg void OnChangeOffsetx();
	afx_msg void OnChangeOffsetz();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

public:

	virtual void Initialize (PropertyView::ViewData* vd);
	virtual bool HasChanged () const;
	virtual void ApplyChanges ();

	//{{AFX_VIRTUAL(FormFractalFamily)
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
