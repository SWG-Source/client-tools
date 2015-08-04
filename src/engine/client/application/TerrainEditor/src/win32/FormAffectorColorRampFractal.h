//
// FormAffectorColorRampFractal.h
// asommers
//
// copyright 2001, sony online entertainment
//

//-------------------------------------------------------------------

#ifndef INCLUDED_FormAffectorColorRampFractal_H
#define INCLUDED_FormAffectorColorRampFractal_H

//-------------------------------------------------------------------

class AffectorColorRampFractal;
class CDib;
class Image;

#include "FormLayerItem.h"
#include "SmartEditCtrl.h"
#include "SmartSliderCtrl.h"

//-------------------------------------------------------------------

class FormAffectorColorRampFractal : public FormLayerItem
{
private:

	AffectorColorRampFractal* m_affector;

	CDib* m_dib;
	int   m_dibSize;

private:

	Image* verifyImage (const CString& name, bool verbose=true) const;
	void   createDib (const Image* image);
	void   updateBitmap () const;

private:

	//{{AFX_DATA(FormAffectorColorRampFractal)
	enum { IDD = IDD_FORM_AFFECTORCOLORRAMPFRACTAL };
	CComboBox	m_familyCtrl;
	CComboBox	m_operation;
	CString	m_name;
	CStatic	m_rampWindow;
	CString	m_imageName;
	//}}AFX_DATA

protected:

	FormAffectorColorRampFractal();           
	DECLARE_DYNCREATE(FormAffectorColorRampFractal)
	virtual ~FormAffectorColorRampFractal();

#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

	//{{AFX_MSG(FormAffectorColorRampFractal)
	afx_msg void OnButtonBrowse();
	afx_msg void OnButtonRefresh();
	afx_msg void OnDestroy();
	afx_msg void OnSelchangeOperation();
	afx_msg void OnSelchangeFamily();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

public:

	virtual void Initialize (PropertyView::ViewData* vd);
	virtual bool HasChanged () const;
	virtual void ApplyChanges ();

	//{{AFX_VIRTUAL(FormAffectorColorRampFractal)
	public:
	virtual void OnInitialUpdate();
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual void OnDraw(CDC* pDC);
	//}}AFX_VIRTUAL
};

//-------------------------------------------------------------------

//{{AFX_INSERT_LOCATION}}

//-------------------------------------------------------------------

#endif 
