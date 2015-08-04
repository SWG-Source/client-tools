//
// FormLayer.h
// asommers
//
// copyright 2001, sony online entertainment
//

//-------------------------------------------------------------------

#ifndef INCLUDED_FormLayer_H
#define INCLUDED_FormLayer_H

//-------------------------------------------------------------------

#include "FormLayerItem.h"

//-------------------------------------------------------------------

class FormLayer : public FormLayerItem
{
private:

	TerrainGenerator::Layer* layer;

private:

	//{{AFX_DATA(FormLayer)
	enum { IDD = IDD_FORM_LAYER };
	CString	m_name;
	BOOL	m_invertBoundaries;
	BOOL	m_invertFilters;
	CString	m_notes;
	//}}AFX_DATA

protected:

	FormLayer (void);           
	DECLARE_DYNCREATE(FormLayer)
	virtual ~FormLayer();

#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

	//{{AFX_MSG(FormLayer)
	afx_msg void OnChangeEditNotes();
	afx_msg void OnDestroy();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

public:

	virtual void Initialize (PropertyView::ViewData* vd);
	virtual bool HasChanged () const;
	virtual void ApplyChanges ();

	//{{AFX_VIRTUAL(FormLayer)
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
