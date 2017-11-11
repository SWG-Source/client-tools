// SelectRegionCanvas.cpp : implementation file
//

#include "FirstUiBuilder.h"
#include "SelectRegionCanvas.h"

#include "UIImage.h"
#include "UIUtils.h"

#include "UIDirect3DPrimaryCanvas.h"

#include <assert.h>

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

static UIColor gHighlightOutlineColor(255,   0,   0, 128);
static UIColor gHighlightFillColor(255,   0,   0,   32);

/////////////////////////////////////////////////////////////////////////////
// SelectRegionCanvas

SelectRegionCanvas::SelectRegionCanvas()
:	mImage(0),
	mDisplay(0),
	mImageSize(0,0),
	mZoomLevel(1.0f),
	mScrollOffset(0,0),
	m_mouseWheelCounter(0),
	mDrawHighlight(true)
{
	mSelectionRect.SetUseBounds(true);
}

void SelectRegionCanvas::construct(UIImage &image, const UISize &imageSize, const UINarrowString &value)
{
	mImage = &image;
	mImageSize = imageSize;

	const UIRect bounds(imageSize);
	mSelectionRect.SetBounds(bounds);

	image.SetSourceRect(&bounds);
	image.SetSize(imageSize);

	mSelectionRect.SetBounds(bounds);
	UIUtils::ParseRect(value, mRegion);
	mSelectionRect.SetRect(mRegion);
}

SelectRegionCanvas::~SelectRegionCanvas()
{
}

/////////////////////////////////////////////////////////////////////////////

void SelectRegionCanvas::updateValue(const CString &i_newValue)
{
	UIUtils::ParseRect(static_cast<const char *>(i_newValue), mRegion);
}

// =========================================================================

void SelectRegionCanvas::updateScrollbars(const UIPoint * center)
{
	CRect rc;
	GetWindowRect(&rc);
	_updateScrollbars(rc, center);
}

void SelectRegionCanvas::_updateScrollbars(const CRect &windowRect, const UIPoint * center)
{
	const int width = windowRect.right-windowRect.left;
	const int height = windowRect.bottom-windowRect.top;


	// -------------------------------------------------------------------
	// calculate scroll bar mods.
	const int imageWidth = int(float(mImageSize.x) * mZoomLevel);
	const int imageHeight = int(float(mImageSize.y) * mZoomLevel);

	DWORD style = GetStyle();
	CRect clientAdjust(0,0,0,0);
	AdjustWindowRectEx(&clientAdjust, style&~(WS_HSCROLL|WS_VSCROLL), FALSE, GetExStyle());
	int clientWidth = width - clientAdjust.Width();
	int clientHeight = height - clientAdjust.Height();

	bool hscroll = imageWidth > clientWidth;
	bool vscroll = imageHeight > clientHeight;

	if (hscroll)
	{
		clientHeight -= GetSystemMetrics(SM_CYHSCROLL);
		vscroll = imageHeight > clientHeight;
	}
	if (vscroll)
	{
		clientWidth -= GetSystemMetrics(SM_CXVSCROLL);
		if (!hscroll)
		{
			hscroll = imageWidth > clientWidth;
			if (hscroll)
			{
				clientHeight -= GetSystemMetrics(SM_CYHSCROLL);
			}
		}
	}

	// -------------------------------------------------------------------

	if (center)
	{
		mScrollOffset.x = static_cast<long>(static_cast<float>(center->x) * mZoomLevel) - clientWidth/2;
		mScrollOffset.y = static_cast<long>(static_cast<float>(center->y) * mZoomLevel) - clientHeight/2;
	}

	// -------------------------------------------------------------------
	// Validate scroll positions
	const int hscrollMax = hscroll ? imageWidth - clientWidth : 0;

	if (mScrollOffset.x > hscrollMax)
	{
		mScrollOffset.x = hscrollMax;
	}
	if (mScrollOffset.x < 0)
	{
		mScrollOffset.x = 0;
	}

	const int vscrollMax = vscroll ? imageHeight - clientHeight : 0;
	if (mScrollOffset.y > vscrollMax)
	{
		mScrollOffset.y = vscrollMax;
	}
	if (mScrollOffset.y < 0)
	{
		mScrollOffset.y = 0;
	}

	// -------------------------------------------------------------------
	SCROLLINFO si;

	if (!hscroll)
	{
		ModifyStyle(WS_HSCROLL, 0);
	}
	else
	{
		si.cbSize = sizeof( si );
		si.fMask  = SIF_PAGE | SIF_POS | SIF_RANGE;
		si.nPage  = clientWidth;
		si.nMin   = 0;
		si.nMax   = imageWidth;
		si.nPos   = mScrollOffset.x;
		SetScrollInfo(SB_HORZ, &si, TRUE);

		ModifyStyle(0, WS_HSCROLL);
	}
	if (!vscroll)
	{
		ModifyStyle(WS_VSCROLL, 0);
	}
	else
	{
		si.cbSize = sizeof( si );
		si.fMask  = SIF_PAGE | SIF_POS | SIF_RANGE;
		si.nPage  = clientHeight;
		si.nMin   = 0;
		si.nMax   = imageHeight;
		si.nPos   = mScrollOffset.y;
		SetScrollInfo(SB_VERT, &si, TRUE);

		ModifyStyle(0, WS_VSCROLL);
	}
}

void SelectRegionCanvas::_updateCanvasScrollPosition(UINT uMsg, UINT sbCode)
{
	UISize ZoomedImageSize( UIScalar( float(mImageSize.x) * mZoomLevel ), UIScalar( float(mImageSize.y) * mZoomLevel ) );
	CRect rc;

	GetWindowRect(&rc);

	rc.right -= rc.left;
	rc.left   = 0;

	rc.bottom -= rc.top;
	rc.top     = 0;

	if (rc.right < ZoomedImageSize.x || rc.bottom < ZoomedImageSize.y)
	{
		GetClientRect(&rc);
	}
	
	UISize NewSize( rc.right, rc.bottom );

	if( uMsg == WM_HSCROLL )
	{
		switch (sbCode)
		{
			case SB_LEFT:
				mScrollOffset.x = 0;
				break;

			case SB_RIGHT:
				mScrollOffset.x = ZoomedImageSize.x - NewSize.x;
				break;

			case SB_LINELEFT:
				mScrollOffset.x -= int( mZoomLevel );

				if( mScrollOffset.x < 0 )
					mScrollOffset.x = 0;
				break;

			case SB_LINERIGHT:
				mScrollOffset.x += int( mZoomLevel );

				if( mScrollOffset.x > ZoomedImageSize.x - NewSize.x )
					mScrollOffset.x = ZoomedImageSize.x - NewSize.x;

				break;

			case SB_PAGELEFT:
				mScrollOffset.x -= int( 10.0f * mZoomLevel );

				if( mScrollOffset.x < 0 )
					mScrollOffset.x = 0;

				break;

			case SB_PAGERIGHT:
				mScrollOffset.x += int( 10.0f * mZoomLevel );

				if( mScrollOffset.x > ZoomedImageSize.x - NewSize.x )
					mScrollOffset.x = ZoomedImageSize.x - NewSize.x;

				break;

			case SB_THUMBTRACK:
				SCROLLINFO si;
				si.cbSize = sizeof( si );
				si.fMask  = SIF_TRACKPOS;

				GetScrollInfo(SB_HORZ, &si);
				mScrollOffset.x = si.nTrackPos;
				break;
		}
	}
	else
	{
		switch (sbCode)
		{
			case SB_LEFT:
				mScrollOffset.y = 0;
				break;

			case SB_RIGHT:
				mScrollOffset.y = ZoomedImageSize.y - NewSize.y;
				break;

			case SB_LINELEFT:
				mScrollOffset.y -= int( mZoomLevel );

				if( mScrollOffset.y < 0 )
					mScrollOffset.y = 0;
				break;

			case SB_LINERIGHT:
				mScrollOffset.y += int( mZoomLevel );

				if( mScrollOffset.y > ZoomedImageSize.y - NewSize.y )
					mScrollOffset.y = ZoomedImageSize.y - NewSize.y;

				break;

			case SB_PAGELEFT:
				mScrollOffset.y -= int( 10.0f * mZoomLevel );

				if( mScrollOffset.y < 0 )
					mScrollOffset.y = 0;

				break;

			case SB_PAGERIGHT:
				mScrollOffset.y += int( 10.0f * mZoomLevel );

				if( mScrollOffset.y > ZoomedImageSize.y - NewSize.y )
					mScrollOffset.y = ZoomedImageSize.y - NewSize.y;

				break;

			case SB_THUMBTRACK:
				SCROLLINFO si;
				si.cbSize = sizeof( si );
				si.fMask  = SIF_TRACKPOS;

				GetScrollInfo(SB_VERT, &si );
				mScrollOffset.y = si.nTrackPos;
				break;
		}
	}
}

//-----------------------------------------------------------------

CPoint SelectRegionCanvas::_inverseTranslatePosition(const CPoint &p)
{
	long x = p.x;
	long y = p.y;

	x += mScrollOffset.x;
	y += mScrollOffset.y;

	x = static_cast<long>( static_cast<float>(x) / mZoomLevel );
	y = static_cast<long>( static_cast<float>(y) / mZoomLevel );

	return CPoint(x, y);
}

void SelectRegionCanvas::_notifyRectChanged()
{
	CWnd *parent = GetParent();
	if (parent)
	{
		parent->SendMessage(WM_COMMAND, MAKEWPARAM(GetDlgCtrlID(), NC_rectChanged), LPARAM(m_hWnd));
	}
}

void SelectRegionCanvas::_notifyZoomUp()
{
	CWnd *parent = GetParent();
	if (parent)
	{
		parent->SendMessage(WM_COMMAND, MAKEWPARAM(GetDlgCtrlID(), NC_zoomUp), LPARAM(m_hWnd));
	}
}

void SelectRegionCanvas::_notifyZoomDown()
{
	CWnd *parent = GetParent();
	if (parent)
	{
		parent->SendMessage(WM_COMMAND, MAKEWPARAM(GetDlgCtrlID(), NC_zoomDown), LPARAM(m_hWnd));
	}
}

/////////////////////////////////////////////////////////////////////////////

BEGIN_MESSAGE_MAP(SelectRegionCanvas, CWnd)
	ON_MESSAGE(WM_APP, OnApp)
	//{{AFX_MSG_MAP(SelectRegionCanvas)
	ON_WM_GETDLGCODE()
	ON_WM_NCHITTEST()
	ON_WM_MOUSEACTIVATE()
	ON_WM_SIZE()
	ON_WM_HSCROLL()
	ON_WM_VSCROLL()
	ON_WM_PAINT()
	ON_WM_LBUTTONDOWN()
	ON_WM_MBUTTONDOWN()
	ON_WM_RBUTTONDOWN()
	ON_WM_LBUTTONUP()
	ON_WM_MBUTTONUP()
	ON_WM_RBUTTONUP()
	ON_WM_MOUSEMOVE()
	ON_WM_KEYDOWN()
	ON_WM_ERASEBKGND()
	ON_WM_DESTROY()
	ON_WM_NCCALCSIZE()
	ON_WM_MOUSEWHEEL()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// SelectRegionCanvas message handlers
static const char *className = "UiBuilder::SelectRegionCanvas";

bool SelectRegionCanvas::registerClass()
{
	HINSTANCE hInst = AfxGetInstanceHandle();

	WNDCLASS wndClass;

	if (!::GetClassInfo(hInst, className, &wndClass))
	{
		memset(&wndClass, 0, sizeof(wndClass));
		wndClass.lpfnWndProc = ::DefWindowProc;
		wndClass.lpszClassName = className;
		if (!AfxRegisterClass(&wndClass))
		{
			return FALSE;
		}
	}

	return TRUE;
}

BOOL SelectRegionCanvas::PreCreateWindow(CREATESTRUCT& cs) 
{
	if (!CWnd::PreCreateWindow(cs))
	{
		return FALSE;
	}

	if (!registerClass())
	{
		return false;
	}

	cs.hMenu = 0;
	cs.lpszClass = className;

	return TRUE;
}

UINT SelectRegionCanvas::OnGetDlgCode() 
{
	return DLGC_WANTARROWS;
}

UINT SelectRegionCanvas::OnNcHitTest(CPoint point) 
{
	CRect windowRect;
	GetWindowRect(&windowRect);

	UINT rc=HTNOWHERE;

	// -------------------------------------------------------

	if (  point.x>=windowRect.left
		&& point.x< windowRect.right
		&& point.y>=windowRect.top
		&& point.y< windowRect.bottom
		)
	{
		DWORD style = GetStyle();
		
		const bool hscroll = (style&WS_HSCROLL)!=0;
		const int hscrollY = windowRect.bottom - ((hscroll) ? GetSystemMetrics(SM_CYHSCROLL) : 0);

		const bool vscroll = (style&WS_VSCROLL)!=0;
		const int vscrollX = windowRect.right - ((vscroll) ? GetSystemMetrics(SM_CXVSCROLL) : 0);

		if (point.x<vscrollX) // left of vertical scroll bar
		{
			if (point.y<hscrollY) // above horizontal scroll bar
			{
				rc = HTCLIENT;
			}
			else // over horizontal scroll bar (Y-only)
			{
				rc = HTHSCROLL;
			}
		}
		else // over vertical scroll bar (X-only)
		{
			if (point.y<hscrollY) // above horizontal scroll bar
			{
				rc = HTVSCROLL;
			}
			else // over horizontal scroll bar (Y-only)
			{
			}
		}
	}

	// -------------------------------------------------------

	if (rc == HTNOWHERE)
	{
		rc = HTCLIENT;
	}

	return rc;
}

int SelectRegionCanvas::OnMouseActivate(CWnd* pDesktopWnd, UINT nHitTest, UINT message) 
{
	return MA_ACTIVATE;
}

void SelectRegionCanvas::OnSize(UINT nType, int cx, int cy) 
{
	UISize NewSize(cx, cy);
	mDisplay->SetSize(NewSize);
	updateScrollbars();
}

void SelectRegionCanvas::OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar)
{ 
	_updateCanvasScrollPosition(WM_HSCROLL, nSBCode);
	updateScrollbars();
	InvalidateRect(0, FALSE );
}

void SelectRegionCanvas::OnVScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar)
{ 
	_updateCanvasScrollPosition(WM_VSCROLL, nSBCode);
	updateScrollbars();
	InvalidateRect(0, FALSE );
}

void SelectRegionCanvas::OnPaint() 
{
	PAINTSTRUCT ps;

	BeginPaint(&ps);
	EndPaint(&ps);

	if (mDisplay->BeginRendering())
	{
		UIColor FillColor;
		DWORD   dwFillColor = GetSysColor( COLOR_3DFACE );
		dwFillColor = 0xff666666;

		mDisplay->PushState();

		FillColor.r = GetRValue( dwFillColor );
		FillColor.g = GetGValue( dwFillColor );
		FillColor.b = GetBValue( dwFillColor );
		FillColor.a = 255;
		
		mDisplay->SetOpacity( 1.0f );
		mDisplay->ClearTo( FillColor, UIRect (0, 0, mDisplay->GetWidth (), mDisplay->GetHeight () ));
		mDisplay->Translate( -mScrollOffset );
		
		mDisplay->Scale( mZoomLevel, mZoomLevel );
		mImage->Render( *mDisplay );

		mDisplay->Flush();

		if (mDrawHighlight)
		{
			mSelectionRect.SetFillColor(gHighlightFillColor);
			mSelectionRect.SetOutlineColor(gHighlightOutlineColor);
			mSelectionRect.SetCanDrawHandles(true);
			mSelectionRect.Render(mDisplay);
		}

		mDisplay->EndRendering();
		mDisplay->Flip();

		mDisplay->PopState();
	}
}

// ===============================================================================================

void SelectRegionCanvas::OnLButtonDown(UINT nFlags, CPoint point) 
{
	SetCapture();
	CPoint iPos = _inverseTranslatePosition(point);
	mSelectionRect.ProcessMessage(m_hWnd, WM_LBUTTONDOWN, nFlags, MAKELPARAM(iPos.x, iPos.y));
	InvalidateRect(0, FALSE);
	mRegion = mSelectionRect.GetRect();
	_notifyRectChanged();
}

void SelectRegionCanvas::OnMButtonDown(UINT nFlags, CPoint point)
{
}

void SelectRegionCanvas::OnRButtonDown(UINT nFlags, CPoint point)
{ 
}

// ===============================================================================================

void SelectRegionCanvas::OnLButtonUp(UINT nFlags, CPoint point) 
{
	ReleaseCapture();
	CPoint iPos = _inverseTranslatePosition(point);
	mSelectionRect.ProcessMessage(m_hWnd, WM_LBUTTONUP, nFlags, MAKELPARAM(iPos.x, iPos.y));
	InvalidateRect(0, FALSE);
	mRegion = mSelectionRect.GetRect();
	_notifyRectChanged();
}

void SelectRegionCanvas::OnMButtonUp(UINT nFlags, CPoint point) { OnLButtonUp(nFlags, point); }
void SelectRegionCanvas::OnRButtonUp(UINT nFlags, CPoint point) { OnLButtonUp(nFlags, point); }

// ===============================================================================================

void SelectRegionCanvas::OnMouseMove(UINT nFlags, CPoint point) 
{
	CPoint iPos = _inverseTranslatePosition(point);
	mSelectionRect.ProcessMessage(m_hWnd, WM_MOUSEMOVE, nFlags, MAKELPARAM(iPos.x, iPos.y));
	InvalidateRect(0, FALSE );
	mRegion = mSelectionRect.GetRect();
	_notifyRectChanged();
}

void SelectRegionCanvas::OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags) 
{
	mSelectionRect.ProcessMessage(m_hWnd, WM_KEYDOWN, nFlags, 0);
	InvalidateRect(0, FALSE);
	mRegion = mSelectionRect.GetRect();
	_notifyRectChanged();
}

BOOL SelectRegionCanvas::OnEraseBkgnd(CDC* pDC) 
{
	return true;
}

void SelectRegionCanvas::OnDestroy() 
{
	mDisplay->Detach();
	mDisplay = 0;
	CWnd::OnDestroy();
}

LRESULT SelectRegionCanvas::OnApp(WPARAM, LPARAM)
{
	CRect rc;

	GetClientRect(&rc);

	const UISize size(rc.right, rc.bottom);

	mDisplay = new UIDirect3DPrimaryCanvas(size, m_hWnd, false);
	mDisplay->Attach( 0 );

	SetWindowPos(0,0,0,0,0,SWP_FRAMECHANGED|SWP_NOACTIVATE|SWP_NOCOPYBITS|SWP_NOMOVE|SWP_NOOWNERZORDER|SWP_NOSIZE|SWP_NOZORDER|SWP_SHOWWINDOW);
	return 0;
}

void SelectRegionCanvas::OnNcCalcSize(BOOL bCalcValidRects, NCCALCSIZE_PARAMS FAR* lpncsp) 
{
	const RECT &rect = lpncsp->rgrc[0];
	_updateScrollbars(rect, 0);
	CWnd::OnNcCalcSize(bCalcValidRects, lpncsp);
}

BOOL SelectRegionCanvas::OnMouseWheel(UINT nFlags, short zDelta, CPoint pt) 
{
	m_mouseWheelCounter+=int(zDelta);
	if (m_mouseWheelCounter>0)
	{
		while (m_mouseWheelCounter>=WHEEL_DELTA)
		{
			m_mouseWheelCounter-=WHEEL_DELTA;
		}
	}
	else if (m_mouseWheelCounter<0)
	{
		while (m_mouseWheelCounter<=-WHEEL_DELTA)
		{
			m_mouseWheelCounter+=WHEEL_DELTA;
		}
	}
	return CWnd::OnMouseWheel(nFlags, zDelta, pt);
}
