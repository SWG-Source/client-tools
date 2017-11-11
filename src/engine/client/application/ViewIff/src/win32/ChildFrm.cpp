//
// ChildFrm.cpp
// asommers 6-12-99
//
// copyright 1999, bootprint entertainment
//

//-------------------------------------------------------------------

#include "FirstViewIff.h"
#include "viewiff.h"

#include "ChildFrm.h"
#include "IffTreeView.h"
#include "IffDataView.h"

#include <afxdlgs.h>

//-------------------------------------------------------------------

IMPLEMENT_DYNCREATE(CChildFrame, CMDIChildWnd)

BEGIN_MESSAGE_MAP(CChildFrame, CMDIChildWnd)
	//{{AFX_MSG_MAP(CChildFrame)
	ON_COMMAND(ID_BYTE, OnByte)
	ON_UPDATE_COMMAND_UI(ID_BYTE, OnUpdateByte)
	ON_COMMAND(ID_DWORD, OnDword)
	ON_UPDATE_COMMAND_UI(ID_DWORD, OnUpdateDword)
	ON_COMMAND(ID_TEXT, OnText)
	ON_UPDATE_COMMAND_UI(ID_TEXT, OnUpdateText)
	ON_COMMAND(ID_WORD, OnWord)
	ON_UPDATE_COMMAND_UI(ID_WORD, OnUpdateWord)
	ON_COMMAND(ID_DUMP, OnDump)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

//-------------------------------------------------------------------

CChildFrame::CChildFrame()
{
}

CChildFrame::~CChildFrame()
{
}

//-------------------------------------------------------------------

BOOL CChildFrame::OnCreateClient( LPCREATESTRUCT /*lpcs*/,
	CCreateContext* pContext)
{
	// create splitter window
	if (!m_wndSplitter.CreateStatic(this, 1, 2))
		return FALSE;

	if (!m_wndSplitter.CreateView(0, 0, RUNTIME_CLASS(IffTreeView), CSize(200, 100), pContext) ||
		!m_wndSplitter.CreateView(0, 1, RUNTIME_CLASS(IffDataView), CSize(200, 100), pContext))
	{
		m_wndSplitter.DestroyWindow();
		return FALSE;
	}

	treeView = dynamic_cast<IffTreeView*> (m_wndSplitter.GetPane (0, 0));
	dataView = dynamic_cast<IffDataView*> (m_wndSplitter.GetPane (0, 1));

	return TRUE;
}

//-------------------------------------------------------------------

BOOL CChildFrame::PreCreateWindow(CREATESTRUCT& cs)
{
	if( !CMDIChildWnd::PreCreateWindow(cs) )
		return FALSE;

	return TRUE;
}

//-------------------------------------------------------------------

#ifdef _DEBUG
void CChildFrame::AssertValid() const
{
	CMDIChildWnd::AssertValid();
}

void CChildFrame::Dump(CDumpContext& dc) const
{
	CMDIChildWnd::Dump(dc);
}

#endif //_DEBUG

//-------------------------------------------------------------------

void CChildFrame::OnByte() 
{
	// TODO: Add your command handler code here
	dataView->setDisplayMode (IffDataView::DM_byte);
	dataView->updateOutput ();
}

void CChildFrame::OnUpdateByte(CCmdUI* pCmdUI) 
{
	// TODO: Add your command update UI handler code here
	pCmdUI->SetCheck (dataView->getDisplayMode () == IffDataView::DM_byte);	
}

void CChildFrame::OnDword() 
{
	// TODO: Add your command handler code here
	dataView->setDisplayMode (IffDataView::DM_dword);
	dataView->updateOutput ();
}

void CChildFrame::OnUpdateDword(CCmdUI* pCmdUI) 
{
	// TODO: Add your command update UI handler code here
	pCmdUI->SetCheck (dataView->getDisplayMode () == IffDataView::DM_dword);	
}

void CChildFrame::OnWord() 
{
	// TODO: Add your command handler code here
	dataView->setDisplayMode (IffDataView::DM_word);
	dataView->updateOutput ();
}

void CChildFrame::OnUpdateWord(CCmdUI* pCmdUI) 
{
	// TODO: Add your command update UI handler code here
	pCmdUI->SetCheck (dataView->getDisplayMode () == IffDataView::DM_word);	
}

void CChildFrame::OnText() 
{
	// TODO: Add your command handler code here
	dataView->setDisplayMode (IffDataView::DM_text);
	dataView->updateOutput ();
}

void CChildFrame::OnUpdateText(CCmdUI* pCmdUI) 
{
	// TODO: Add your command update UI handler code here
	pCmdUI->SetCheck (dataView->getDisplayMode () == IffDataView::DM_text);	
}

//-------------------------------------------------------------------

void CChildFrame::OnDump() 
{
	CFileDialog dialog(FALSE);
	if (dialog.DoModal() == IDOK)
		treeView->Dump(dialog.GetPathName());
}
