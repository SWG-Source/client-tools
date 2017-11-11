//
// WarningFrame.h
// asommers
//
// copyright 2001, sony online entertainment
//

//-------------------------------------------------------------------

#ifndef INCLUDED_WarningFrame_H
#define INCLUDED_WarningFrame_H

//-------------------------------------------------------------------

#include "TerrainGeneratorHelper.h"

//-------------------------------------------------------------------

class WarningFrame : public CMDIChildWnd
{
	DECLARE_DYNCREATE(WarningFrame)

protected:

	CString     m_windowName;

protected:

	WarningFrame();           

public:

	void update (const TerrainGeneratorHelper::OutputData& outputData);
	void clear (void);

	//{{AFX_VIRTUAL(WarningFrame)
	protected:
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
	//}}AFX_VIRTUAL

protected:

	virtual ~WarningFrame();

	//{{AFX_MSG(WarningFrame)
	afx_msg void OnDestroy();
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//-------------------------------------------------------------------

//{{AFX_INSERT_LOCATION}}

//-------------------------------------------------------------------

#endif 
