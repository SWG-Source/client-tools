//
// FormAffectorHeightConstant.h
// asommers
//
// copyright 2000, sony online entertainment
//

//-------------------------------------------------------------------

#ifndef INCLUDED_FormAffectorHeightConstant_H
#define INCLUDED_FormAffectorHeightConstant_H

//-------------------------------------------------------------------

class AffectorHeightConstant;

#include "FormLayerItem.h"

//-------------------------------------------------------------------

class FormAffectorHeightConstant : public FormLayerItem
{
private:

	AffectorHeightConstant* affector;

private:

	//{{AFX_DATA(FormAffectorHeightConstant)
	enum { IDD = IDD_FORM_AFFECTORHEIGHTCONSTANT };
	NumberEdit	m_value;
	CString	m_name;
	int		m_operation;
	//}}AFX_DATA

protected:

	FormAffectorHeightConstant();           
	DECLARE_DYNCREATE(FormAffectorHeightConstant)
	virtual ~FormAffectorHeightConstant();

#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

	//{{AFX_MSG(FormAffectorHeightConstant)
	afx_msg void OnSelchangeOperation();
	afx_msg void OnDestroy();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

public:

	virtual void Initialize (PropertyView::ViewData* vd);
	virtual bool HasChanged () const;
	virtual void ApplyChanges ();

	//{{AFX_VIRTUAL(FormAffectorHeightConstant)
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
