//
// ConsoleFrame.h
// asommers 10-9-2000
//
// copyright 2000, verant interactive
//

//-------------------------------------------------------------------

#ifndef CONSOLEFRAME_H
#define CONSOLEFRAME_H

//-------------------------------------------------------------------

class ConsoleFrame : public CMDIChildWnd
{
protected:

	ConsoleFrame (void);
	DECLARE_DYNCREATE(ConsoleFrame)
	virtual ~ConsoleFrame();

protected:

	//{{AFX_MSG(ConsoleFrame)
	afx_msg void OnDestroy();
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

public:

	void setConsoleMessage (const CString& newMessage);

public:

	//{{AFX_VIRTUAL(ConsoleFrame)
	protected:
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
	//}}AFX_VIRTUAL
};

//-------------------------------------------------------------------

//{{AFX_INSERT_LOCATION}}

//-------------------------------------------------------------------

#endif 
