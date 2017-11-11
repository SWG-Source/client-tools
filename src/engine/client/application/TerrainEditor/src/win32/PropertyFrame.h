//
// PropertyFrame.h
// asommers
//
// copyright 2001, sony online entertainment
//

//-------------------------------------------------------------------

#ifndef INCLUDED_PropertyFrame_H
#define INCLUDED_PropertyFrame_H

//-------------------------------------------------------------------

#include "TerrainEditorDoc.h"
#include "PropertyView.h"

//-------------------------------------------------------------------

class PropertyFrame : public CMDIChildWnd
{
private:

	CString windowName;

protected:

	PropertyFrame();           
	DECLARE_DYNCREATE(PropertyFrame)
	virtual ~PropertyFrame();

	// Generated message map functions
	//{{AFX_MSG(PropertyFrame)
	afx_msg void OnDestroy();
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

public:

	void ApplyChanges ();
	void ChangeView (CRuntimeClass* cls, PropertyView::ViewData* vd);
	void RefreshView ();

	//{{AFX_VIRTUAL(PropertyFrame)
	protected:
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
	//}}AFX_VIRTUAL
};

//-------------------------------------------------------------------

//{{AFX_INSERT_LOCATION}}

//-------------------------------------------------------------------

#endif 
