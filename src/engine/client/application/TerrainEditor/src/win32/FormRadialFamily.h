//
// FormRadialFamily.h
// asommers
//
// copyright 2001, sony online entertainment
//

//-------------------------------------------------------------------

#ifndef INCLUDED_FormRadialFamily_H
#define INCLUDED_FormRadialFamily_H

//-------------------------------------------------------------------

class RadialGroup;

#include "PropertyView.h"
#include "SmartEditCtrl.h"
#include "SmartSliderCtrl.h"

//-------------------------------------------------------------------

class FormRadialFamily : public PropertyView
{
public:

	class FormRadialFamilyViewData : public PropertyView::ViewData
	{
	public:

		int          familyId;     //lint !e1925  //-- public data member
		RadialGroup* radialGroup;  //lint !e1925  //-- public data member

	public:

		FormRadialFamilyViewData (void);
		virtual ~FormRadialFamilyViewData (void);
	};

private:

	FormRadialFamilyViewData data;

private:

	//{{AFX_DATA(FormRadialFamily)
	enum { IDD = IDD_FORM_RADIALFAMILY };
	SmartEditCtrl	m_editDensity;
	SmartSliderCtrl	m_sliderDensity;
	CListCtrl	m_childList;
	CStatic	m_windowColor;
	CString	m_name;
	//}}AFX_DATA

protected:

	FormRadialFamily();           
	DECLARE_DYNCREATE(FormRadialFamily)
	virtual ~FormRadialFamily();

#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

	//{{AFX_MSG(FormRadialFamily)
	afx_msg void OnChoosecolor();
	afx_msg void OnDestroy();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

public:

	virtual void Initialize (PropertyView::ViewData* vd);
	virtual bool HasChanged () const;
	virtual void ApplyChanges ();

	//{{AFX_VIRTUAL(FormRadialFamily)
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
