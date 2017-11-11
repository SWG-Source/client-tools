//
// FormAffectorColorConstant.h
// asommers
//
// copyright 2000, sony online entertainment
//

//-------------------------------------------------------------------

#ifndef INCLUDED_FormAffectorColorConstant_H
#define INCLUDED_FormAffectorColorConstant_H

//-------------------------------------------------------------------

class AffectorColorConstant;

#include "FormLayerItem.h"

//-------------------------------------------------------------------

class FormAffectorColorConstant : public FormLayerItem
{
private:

	AffectorColorConstant* m_affector;
	PackedRgb              m_color;

private:

	//{{AFX_DATA(FormAffectorColorConstant)
	enum { IDD = IDD_FORM_AFFECTORCOLORCONSTANT };
	CStatic	m_windowColor;
	CString	m_name;
	int		m_operation;
	//}}AFX_DATA

protected:

	FormAffectorColorConstant();
	DECLARE_DYNCREATE(FormAffectorColorConstant)

protected:

	virtual ~FormAffectorColorConstant();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

	// Generated message map functions
	//{{AFX_MSG(FormAffectorColorConstant)
	afx_msg void OnChoosecolor();
	afx_msg void OnEditchangeOperation();
	afx_msg void OnDestroy();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

public:

	virtual void Initialize (PropertyView::ViewData* vd);
	virtual bool HasChanged (void) const;
	virtual void ApplyChanges (void);

	//{{AFX_VIRTUAL(FormAffectorColorConstant)
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
