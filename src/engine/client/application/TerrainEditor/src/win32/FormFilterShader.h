//
// FormFilterShader.h
// asommers
//
// copyright 2000, sony online entertainment
//

//-------------------------------------------------------------------

#ifndef INCLUDED_FormFilterShader_H
#define INCLUDED_FormFilterShader_H

//-------------------------------------------------------------------

class FilterShader;

#include "FormLayerItem.h"

//-------------------------------------------------------------------

class FormFilterShader : public FormLayerItem
{
private:

	FilterShader* m_filter;

private:

	//{{AFX_DATA(FormFilterShader)
	enum { IDD = IDD_FORM_FILTERSHADER };
	CComboBox	m_familyCtl;
	CString	m_name;
	//}}AFX_DATA

protected:

	FormFilterShader();           
	DECLARE_DYNCREATE(FormFilterShader)

protected:

	virtual ~FormFilterShader();

#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

	//{{AFX_MSG(FormFilterShader)
	afx_msg void OnSelchangefamily();
	afx_msg void OnDestroy();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

public:

	virtual void Initialize (PropertyView::ViewData* vd);
	virtual bool HasChanged () const;
	virtual void ApplyChanges ();

	//{{AFX_VIRTUAL(FormFilterShader)
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
