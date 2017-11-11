//
// FindFrame.h
// asommers
//
// copyright 2001, sony online entertainment
//

//-------------------------------------------------------------------

#ifndef INCLUDED_FindFrame_H
#define INCLUDED_FindFrame_H

//-------------------------------------------------------------------

#include "TerrainGeneratorHelper.h"

//-------------------------------------------------------------------

class FindFrame : public CMDIChildWnd
{
protected:

	CString m_windowName;

protected:

	FindFrame();           
	DECLARE_DYNCREATE(FindFrame)
	virtual ~FindFrame();

	//{{AFX_MSG(FindFrame)
	afx_msg void OnDestroy();
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

public:

	void update (const TerrainGeneratorHelper::OutputData& outputData);
	void clear (void);

	//{{AFX_VIRTUAL(FindFrame)
	protected:
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
	//}}AFX_VIRTUAL
};

//-------------------------------------------------------------------

//{{AFX_INSERT_LOCATION}}

//-------------------------------------------------------------------

#endif 
