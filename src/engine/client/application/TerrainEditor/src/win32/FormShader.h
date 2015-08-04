//
// FormShader.h
// asommers
//
// copyright 2000, sony online entertainment
//

//-------------------------------------------------------------------

#ifndef INCLUDED_FormShader_H
#define INCLUDED_FormShader_H

//-------------------------------------------------------------------

class ShaderGroup;

#include "PropertyView.h"

//-------------------------------------------------------------------

class FormShader : public PropertyView
{
public:

	class FormShaderViewData : public PropertyView::ViewData
	{
	public:

		CString      childName;    //lint !e1925  //-- public data member
		int          familyId;     //lint !e1925  //-- public data member
		ShaderGroup* shaderGroup;  //lint !e1925  //-- public data member

	public:

		FormShaderViewData (void);
		virtual ~FormShaderViewData (void);
	};

private:

	FormShaderViewData data;

private:

	//{{AFX_DATA(FormShader)
	enum { IDD = IDD_FORM_SHADERCHILD };
	NumberEdit	m_weight;
	CStatic	m_familyColorWindow;
	CString	m_name;
	CString	m_familyName;
	//}}AFX_DATA

protected:

	FormShader();           
	DECLARE_DYNCREATE(FormShader)
	virtual ~FormShader();

#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

	//{{AFX_MSG(FormShader)
	afx_msg void OnDestroy();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

public:

	virtual void Initialize (PropertyView::ViewData* vd);
	virtual bool HasChanged () const;
	virtual void ApplyChanges ();

	//{{AFX_VIRTUAL(FormShader)
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
