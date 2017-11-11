// ChildView.cpp : implementation of the CChildView class
//

#include "FirstUiBuilder.h"
#include "UiBuilder.h"
#include "ChildView.h"
#include "MainFrm.h"
#include "ObjectEditor.h"
#include "UserWindowsMessages.h"

/////////////////////////////////////////////////////////////////////////////
// CChildView

CChildView::CChildView()
{
}

CChildView::~CChildView()
{
}

void CChildView::redrawView(bool synchronous)
{
	UINT redrawFlags = RDW_INVALIDATE | RDW_NOERASE | RDW_NOCHILDREN;
	if (synchronous)
	{
		redrawFlags|=RDW_UPDATENOW;
	}
	RedrawWindow(0, 0, redrawFlags);
}

CMainFrame *CChildView::_getFrame()
{
	CWnd *parent = GetParent();
	return STATIC_DOWNCAST(CMainFrame, parent);
}

ObjectEditor *CChildView::_getEditor()
{
	CMainFrame *frame = _getFrame();
	return (frame) ? frame->getEditor() : (ObjectEditor *)0;
}


BEGIN_MESSAGE_MAP(CChildView,CWnd )
	//{{AFX_MSG_MAP(CChildView)
	ON_WM_PAINT()
	ON_WM_LBUTTONDOWN()
	ON_WM_LBUTTONUP()
	ON_WM_MBUTTONDOWN()
	ON_WM_MBUTTONUP()
	ON_WM_RBUTTONDOWN()
	ON_WM_RBUTTONUP()
	ON_WM_MOUSEMOVE()
	ON_WM_MOUSEWHEEL()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////
// CChildView message handlers

BOOL CChildView::PreCreateWindow(CREATESTRUCT& cs) 
{
	if (!CWnd::PreCreateWindow(cs))
		return FALSE;

	cs.dwExStyle |= WS_EX_CLIENTEDGE;
	cs.style &= ~WS_BORDER;
	cs.lpszClass = AfxRegisterWndClass(CS_HREDRAW|CS_VREDRAW|CS_DBLCLKS, 
		::LoadCursor(NULL, IDC_ARROW), HBRUSH(COLOR_WINDOW+1), NULL);

	return TRUE;
}

void CChildView::OnPaint() 
{
	CMainFrame *frame = _getFrame();
	if (frame->isEditing())
	{
		ValidateRect(0);
		
		HWND hParent = ::GetParent(m_hWnd);
		if (hParent)
		{
			::SendMessage(hParent, WM_paintChild, 0, 0);
		}
	}
	else
	{
		CWnd::OnPaint();
	}
}

void CChildView::OnLButtonDown(UINT nFlags, CPoint point) 
{
	ObjectEditor *editor = _getEditor();
	if (editor)
	{
		editor->onLeftButtonDown(nFlags, point.x, point.y);
	}
	CWnd::OnLButtonDown(nFlags, point);
}

void CChildView::OnLButtonUp(UINT nFlags, CPoint point) 
{
	ObjectEditor *editor = _getEditor();
	if (editor)
	{
		editor->onLeftButtonUp(nFlags, point.x, point.y);
	}
	CWnd::OnLButtonUp(nFlags, point);
}

void CChildView::OnMButtonDown(UINT nFlags, CPoint point) 
{
	ObjectEditor *editor = _getEditor();
	if (editor)
	{
		editor->onMiddleButtonDown(nFlags, point.x, point.y);
	}
	CWnd::OnMButtonDown(nFlags, point);
}

void CChildView::OnMButtonUp(UINT nFlags, CPoint point) 
{
	ObjectEditor *editor = _getEditor();
	if (editor)
	{
		editor->onMiddleButtonUp(nFlags, point.x, point.y);
	}
	CWnd::OnMButtonUp(nFlags, point);
}

void CChildView::OnRButtonDown(UINT nFlags, CPoint point) 
{
	ObjectEditor *editor = _getEditor();
	if (editor)
	{
		editor->onRightButtonDown(nFlags, point.x, point.y);
	}
	CWnd::OnRButtonDown(nFlags, point);
}

void CChildView::OnRButtonUp(UINT nFlags, CPoint point) 
{
	ObjectEditor *editor = _getEditor();
	if (editor)
	{
		editor->onRightButtonUp(nFlags, point.x, point.y);
	}
	CWnd::OnRButtonUp(nFlags, point);
}

void CChildView::OnMouseMove(UINT nFlags, CPoint point) 
{
	CWnd *owner = GetOwner();
	if (owner)
	{
		for (CWnd *focus = GetFocus();focus;focus=focus->GetParent())
		{
			if (focus==this)
			{
				break;
			}
			else if (focus==owner)
			{
				SetFocus();
				break;
			}
		}
	}

	ObjectEditor *editor = _getEditor();
	if (editor)
	{
		editor->onMouseMove(nFlags, point.x, point.y);
	}
	CWnd::OnMouseMove(nFlags, point);
}

BOOL CChildView::OnMouseWheel(UINT nFlags, short zDelta, CPoint pt) 
{
	ObjectEditor *editor = _getEditor();
	if (editor)
	{
		editor->onMouseWheel(nFlags, pt.x, pt.y, zDelta);
	}
	return CWnd::OnMouseWheel(nFlags, zDelta, pt);
}
