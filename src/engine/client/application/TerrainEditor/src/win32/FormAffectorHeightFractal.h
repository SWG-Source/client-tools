//
// FormAffectorHeightFractal.h
// asommers
//
// copyright 2001, sony online entertainment
//

//-------------------------------------------------------------------

#ifndef INCLUDED_FormAffectorHeightFractal_H
#define INCLUDED_FormAffectorHeightFractal_H

//-------------------------------------------------------------------

class AffectorHeightFractal;

#include "FormLayerItem.h"
#include "SmartEditCtrl.h"
#include "SmartSliderCtrl.h"

//-------------------------------------------------------------------

class FormAffectorHeightFractal : public FormLayerItem
{
private:

	AffectorHeightFractal* m_affector;

private:

	//{{AFX_DATA(FormAffectorHeightFractal)
	enum { IDD = IDD_FORM_AFFECTORHEIGHTFRACTAL };
	CComboBox	m_familyCtrl;
	NumberEdit	m_scaleY;
	CComboBox	m_operation;
	CString	m_name;
	//}}AFX_DATA

private:

	void updateBitmap () const;

protected:

	FormAffectorHeightFractal();           
	DECLARE_DYNCREATE(FormAffectorHeightFractal)
	virtual ~FormAffectorHeightFractal();

#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

	//{{AFX_MSG(FormAffectorHeightFractal)
	afx_msg void OnDestroy();
	afx_msg void OnSelchangeOperation();
	afx_msg void OnSelchangeFamily();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

public:

	virtual void Initialize (PropertyView::ViewData* vd);
	virtual bool HasChanged () const;
	virtual void ApplyChanges ();

	//{{AFX_VIRTUAL(FormAffectorHeightFractal)
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
