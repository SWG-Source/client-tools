//
// FormRadial.h
// asommers
// 
// copyright 2001, sony online entertainment
//

//-------------------------------------------------------------------

#ifndef INCLUDED_FormRadial_H
#define INCLUDED_FormRadial_H

//-------------------------------------------------------------------

class RadialGroup;

#include "PropertyView.h"

//-------------------------------------------------------------------

class FormRadial : public PropertyView
{
public:

	class FormRadialViewData : public PropertyView::ViewData
	{
	public:

		CString      childName;   //lint !e1925  //-- public data member
		int          familyId;    //lint !e1925  //-- public data member
		RadialGroup* radialGroup; //lint !e1925  //-- public data member

	public:

		FormRadialViewData (void);
		virtual ~FormRadialViewData (void);
	};

private:

	FormRadialViewData data;

private:

	//{{AFX_DATA(FormRadial)
	enum { IDD = IDD_FORM_RADIALCHILD };
	NumberEdit	m_period;
	NumberEdit	m_displacement;
	NumberEdit	m_distance;
	NumberEdit	m_minHeight;
	NumberEdit  m_maxHeight;
	NumberEdit	m_minWidth;
	NumberEdit  m_maxWidth;
	NumberEdit	m_weight;
	CStatic	m_familyColorWindow;
	CString	m_name;
	CString	m_familyName;
	BOOL	m_shouldSway;
	BOOL	m_createPlus;
	BOOL    m_maintainAspectRatio;
	BOOL	m_alignToTerrain;
	//}}AFX_DATA

protected:

	FormRadial();           
	DECLARE_DYNCREATE(FormRadial)
	virtual ~FormRadial();

#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

	//{{AFX_MSG(FormRadial)
	afx_msg void OnCheckSway();
	afx_msg void OnAlignToTerrain();
	afx_msg void OnCreatePlus();
	afx_msg void OnMaintainAspectRatio();
	afx_msg void OnChangeEditDisplacement();
	afx_msg void OnChangeEditDistance();
	afx_msg void OnChangeEditFrequency();
	afx_msg void OnChangeEditMinHeight();
	afx_msg void OnChangeEditMaxHeight();
	afx_msg void OnChangeEditMinWidth();
	afx_msg void OnChangeEditMaxWidth();
	afx_msg void OnChangeWeight();
	afx_msg void OnDestroy();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

public:

	virtual void Initialize (PropertyView::ViewData* vd);
	virtual bool HasChanged () const;
	virtual void ApplyChanges ();

	//{{AFX_VIRTUAL(FormRadial)
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
