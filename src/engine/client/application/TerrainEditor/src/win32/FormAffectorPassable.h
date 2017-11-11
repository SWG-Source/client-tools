//
// FormAffectorPassable.h
// asommers
//
// copyright 2000, sony online entertainment
//

//-------------------------------------------------------------------

#ifndef INCLUDED_FormAffectorPassable_H
#define INCLUDED_FormAffectorPassable_H

//-------------------------------------------------------------------

class AffectorPassable;

#include "FormLayerItem.h"
#include "SmartSliderCtrl.h"
#include "SmartEditCtrl.h"

//-------------------------------------------------------------------

class FormAffectorPassable : public FormLayerItem
{
private:

	AffectorPassable* affector;

private:

	//{{AFX_DATA(FormAffectorPassable)
	enum { IDD = IDD_FORM_AFFECTORPASSABLE };
	SmartSliderCtrl	m_sliderFeatherClamp;
	SmartEditCtrl	m_editFeatherClamp;
	BOOL	m_passable;
	//}}AFX_DATA

protected:

	FormAffectorPassable();           
	DECLARE_DYNCREATE(FormAffectorPassable)
	virtual ~FormAffectorPassable();

#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

	// Generated message map functions
	//{{AFX_MSG(FormAffectorPassable)
	afx_msg void OnSelchangefamily();
	afx_msg void OnCheckPassable();
	afx_msg void OnDestroy();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

public:

	virtual void Initialize (PropertyView::ViewData* vd);
	virtual bool HasChanged () const;
	virtual void ApplyChanges ();

	//{{AFX_VIRTUAL(FormAffectorPassable)
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
