// ======================================================================
//
// ScriptShellView.h
// asommers 2003-09-23
//
// copyright2003, sony online entertainment
// 
// ======================================================================

#ifndef INCLUDED_ScriptShellView_H
#define INCLUDED_ScriptShellView_H

// ======================================================================

#include "ShellView.h"

class ScriptShellView : public CShellView
{
protected:

	ScriptShellView();           
	DECLARE_DYNCREATE(ScriptShellView)

public:

	//{{AFX_VIRTUAL(ScriptShellView)
	protected:
	virtual void OnUpdate(CView* pSender, LPARAM lHint, CObject* pHint);
	//}}AFX_VIRTUAL

protected:

	virtual ~ScriptShellView();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:

	//{{AFX_MSG(ScriptShellView)
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

// ======================================================================

//{{AFX_INSERT_LOCATION}}

// ======================================================================

#endif 
