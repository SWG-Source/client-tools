//
// FormShaderFamily.h
// asommers
//
// copyright 2001, sony online entertainment
//

//-------------------------------------------------------------------

#ifndef INCLUDED_FormShaderFamily_H
#define INCLUDED_FormShaderFamily_H

//-------------------------------------------------------------------

class ShaderGroup;

#include "PropertyView.h"
#include "SmartEditCtrl.h"
#include "SmartSliderCtrl.h"

//-------------------------------------------------------------------

class FormShaderFamily : public PropertyView
{
public:

	class FormShaderFamilyViewData : public PropertyView::ViewData
	{
	public:

		int          familyId;     //lint !e1925  //-- public data member
		ShaderGroup* shaderGroup;  //lint !e1925  //-- public data member

	public:

		FormShaderFamilyViewData (void);
		virtual ~FormShaderFamilyViewData (void);
	};

private:

	FormShaderFamilyViewData data;

private:

	//{{AFX_DATA(FormShaderFamily)
	enum { IDD = IDD_FORM_SHADERFAMILY };
	SmartEditCtrl	m_editFeatherClamp;
	SmartSliderCtrl	m_sliderFeatherClamp;
	CListCtrl	m_childList;
	CStatic	m_windowColor;
	CString	m_name;
	CString	m_surfacePropertiesName;
	//}}AFX_DATA

protected:

	FormShaderFamily();           // protected constructor used by dynamic creation
	DECLARE_DYNCREATE(FormShaderFamily)
	virtual ~FormShaderFamily();

#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

	// Generated message map functions
	//{{AFX_MSG(FormShaderFamily)
	afx_msg void OnChoosecolor();
	afx_msg void OnDestroy();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

public:

	virtual void Initialize (PropertyView::ViewData* vd);
	virtual bool HasChanged () const;
	virtual void ApplyChanges ();

	//{{AFX_VIRTUAL(FormShaderFamily)
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
