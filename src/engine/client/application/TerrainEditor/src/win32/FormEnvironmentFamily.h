//
// FormEnvironmentFamily.h
// asommers
//
// copyright 2001, sony online entertainment
//

//-------------------------------------------------------------------

#ifndef INCLUDED_FormEnvironmentFamily_H
#define INCLUDED_FormEnvironmentFamily_H

//-------------------------------------------------------------------

class EnvironmentGroup;

#include "PropertyView.h"
#include "SmartEditCtrl.h"
#include "SmartSliderCtrl.h"

//-------------------------------------------------------------------

class FormEnvironmentFamily : public PropertyView
{
public:

	class FormEnvironmentFamilyViewData : public PropertyView::ViewData
	{
	public:

		int               familyId;     //lint !e1925  //-- public data member
		EnvironmentGroup* environmentGroup;  //lint !e1925  //-- public data member

	public:

		FormEnvironmentFamilyViewData (void);
		virtual ~FormEnvironmentFamilyViewData (void);
	};

private:

	const CString getWindowName (const char* name);

private:

	FormEnvironmentFamilyViewData data;

private:

	//{{AFX_DATA(FormEnvironmentFamily)
	enum { IDD = IDD_FORM_ENVIRONMENTFAMILY };
	SmartEditCtrl	m_editFeatherClamp;
	SmartSliderCtrl	m_sliderFeatherClamp;
	CStatic	m_windowColor;
	CString	m_name;
	//}}AFX_DATA

protected:

	FormEnvironmentFamily();           // protected constructor used by dynamic creation
	DECLARE_DYNCREATE(FormEnvironmentFamily)
	virtual ~FormEnvironmentFamily();

#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

	// Generated message map functions
	//{{AFX_MSG(FormEnvironmentFamily)
	afx_msg void OnChoosecolor();
	afx_msg void OnDestroy();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

public:

	virtual void Initialize (PropertyView::ViewData* vd);
	virtual bool HasChanged () const;
	virtual void ApplyChanges ();

	//{{AFX_VIRTUAL(FormEnvironmentFamily)
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
