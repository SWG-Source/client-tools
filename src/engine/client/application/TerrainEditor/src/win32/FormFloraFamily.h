//
// FormFloraFamily.h
// asommers
//
// copyright 2001, sony online entertainment
//

//-------------------------------------------------------------------

#ifndef INCLUDED_FormFloraFamily_H
#define INCLUDED_FormFloraFamily_H

//-------------------------------------------------------------------

class FloraGroup;

#include "PropertyView.h"
#include "SmartEditCtrl.h"
#include "SmartSliderCtrl.h"

//-------------------------------------------------------------------

class FormFloraFamily : public PropertyView
{
public:

	class FormFloraFamilyViewData : public PropertyView::ViewData
	{
	public:

		int         familyId;    //lint !e1925  //-- public data member
		FloraGroup* floraGroup;  //lint !e1925  //-- public data member

	public:

		FormFloraFamilyViewData (void);
		virtual ~FormFloraFamilyViewData (void);
	};

protected:

	FormFloraFamilyViewData data;

protected:

	//{{AFX_DATA(FormFloraFamily)
	enum { IDD = IDD_FORM_FLORAFAMILY };
	SmartEditCtrl	m_editDensity;
	SmartSliderCtrl	m_sliderDensity;
	CListCtrl	m_childList;
	CStatic	m_windowColor;
	CString	m_name;
	BOOL	m_floats;
	//}}AFX_DATA

protected:

	FormFloraFamily();           
	DECLARE_DYNCREATE(FormFloraFamily)

public:

	virtual void Initialize (PropertyView::ViewData* vd);
	virtual bool HasChanged () const;
	virtual void ApplyChanges ();

	//{{AFX_VIRTUAL(FormFloraFamily)
	public:
	virtual void OnInitialUpdate();
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual void OnDraw(CDC* pDC);
	//}}AFX_VIRTUAL

protected:

	virtual ~FormFloraFamily();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

	//{{AFX_MSG(FormFloraFamily)
	afx_msg void OnChoosecolor();
	afx_msg void OnCheckFloats();
	afx_msg void OnDestroy();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//-------------------------------------------------------------------

//{{AFX_INSERT_LOCATION}}

//-------------------------------------------------------------------

#endif 
