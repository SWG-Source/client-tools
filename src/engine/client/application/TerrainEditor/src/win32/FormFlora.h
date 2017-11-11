//
// FormFlora.h
// asommers
//
// copyright 2001, sony online entertainment
//

//-------------------------------------------------------------------

#ifndef INCLUDED_FormFlora_H
#define INCLUDED_FormFlora_H

//-------------------------------------------------------------------

class FloraGroup;

#include "PropertyView.h"

//-------------------------------------------------------------------

class FormFlora : public PropertyView
{
public:

	class FormFloraViewData : public PropertyView::ViewData
	{
	public:

		CString     childName;   //lint !e1925  //-- public data member
		int         familyId;    //lint !e1925  //-- public data member
		FloraGroup* floraGroup;  //lint !e1925  //-- public data member

	public:

		FormFloraViewData ();
		virtual ~FormFloraViewData();
	};

private:

	FormFloraViewData data;

private:

	//{{AFX_DATA(FormFlora)
	enum { IDD = IDD_FORM_FLORACHILD };
	NumberEdit	m_weight;
	NumberEdit	m_period;
	NumberEdit	m_displacement;
	CStatic		m_familyColorWindow;
	CString     m_name;
	BOOL		m_shouldSway;
	CString		m_familyName;
	BOOL		m_alignToTerrain;
	BOOL	    m_shouldScale;
	NumberEdit	m_minimumScale;
	NumberEdit	m_maximumScale;
	//}}AFX_DATA

protected:

	FormFlora();           
	DECLARE_DYNCREATE(FormFlora)
	virtual ~FormFlora();

#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

	//{{AFX_MSG(FormFlora)
	afx_msg void OnCheckSway();
	afx_msg void OnAlignToTerrain();
	afx_msg void OnChangeEditDisplacement();
	afx_msg void OnChangeEditFrequency();
	afx_msg void OnChangeWeight();
	afx_msg void OnDestroy();
	afx_msg void OnCheckScale();
	afx_msg void OnChangeEditMinimumScale();
	afx_msg void OnChangeEditMaximumScale();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

public:

	virtual void Initialize (PropertyView::ViewData* vd);
	virtual bool HasChanged () const;
	virtual void ApplyChanges ();

	//{{AFX_VIRTUAL(FormFlora)
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
