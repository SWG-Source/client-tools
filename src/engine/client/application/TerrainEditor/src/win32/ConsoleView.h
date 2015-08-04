//
// ConsoleView.h
// asommers 10-9-2000
//
// copyright 2000, verant interactive
//

//-------------------------------------------------------------------

#ifndef CONSOLEVIEW_H
#define CONSOLEVIEW_H

//-------------------------------------------------------------------

class ConsoleView : public CEditView
{
protected:

	ConsoleView (void);           
	DECLARE_DYNCREATE(ConsoleView)
	virtual ~ConsoleView();

#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

	//{{AFX_MSG(ConsoleView)
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

public:

	void setConsoleMessage (const CString& newMessage);

	//{{AFX_VIRTUAL(ConsoleView)
	public:
	virtual void OnInitialUpdate();
	protected:
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
	//}}AFX_VIRTUAL

private:

	CFont* m_font;
};

//-------------------------------------------------------------------

//{{AFX_INSERT_LOCATION}}

#endif 

