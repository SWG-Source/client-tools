//
// ProfileFrame.h
// asommers
//
// copyright 2001, sony online entertainment
//

//-------------------------------------------------------------------

#ifndef INCLUDED_ProfileFrame_H
#define INCLUDED_ProfileFrame_H

//-------------------------------------------------------------------

class TerrainGenerator;

//-------------------------------------------------------------------

class ProfileFrame : public CMDIChildWnd
{
	DECLARE_DYNCREATE(ProfileFrame)

protected:

	CString     m_windowName;

protected:

	ProfileFrame();           

public:

	void update (const TerrainGenerator* generator);

	//{{AFX_VIRTUAL(ProfileFrame)
	protected:
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
	//}}AFX_VIRTUAL

protected:

	virtual ~ProfileFrame();

	//{{AFX_MSG(ProfileFrame)
	afx_msg void OnDestroy();
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//-------------------------------------------------------------------

//{{AFX_INSERT_LOCATION}}

//-------------------------------------------------------------------

#endif 
