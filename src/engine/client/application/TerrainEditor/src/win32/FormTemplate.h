//
// FormTemplate.h
// asommers
//
// copyright 2000, sony online entertainment
//

//-------------------------------------------------------------------

#ifndef INCLUDED_FormTemplate_H
#define INCLUDED_FormTemplate_H

//-------------------------------------------------------------------

class Affector;

#include "FormLayerItem.h"

//-------------------------------------------------------------------

class FormTemplate : public FormLayerItem
{
private:

	Affector* affector;

private:

	//{{AFX_DATA(FormTemplate)
	enum { IDD = IDD_DEFAULT_PROPVIEW };
	//}}AFX_DATA

protected:

	FormTemplate();           
	DECLARE_DYNCREATE(FormTemplate)
	virtual ~FormTemplate();

#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

	// Generated message map functions
	//{{AFX_MSG(FormTemplate)
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

public:

	virtual void Initialize (PropertyView::ViewData* vd);
	virtual bool HasChanged () const;
	virtual void ApplyChanges ();

	//{{AFX_VIRTUAL(FormTemplate)
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
