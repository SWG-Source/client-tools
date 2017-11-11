#include "FirstUiBuilder.h"
#include "SelectRegion.h"

#include "resource.h"
#include "UIDirect3DPrimaryCanvas.h"
#include "UIImage.h"
#include "UIImageFrame.h"
#include "UIImageStyle.h"
#include "UIUtils.h"
#include "UnicodeUtils.h"

#include <cstdio>

SelectRegionDialogBox		*SelectRegionDialogBox::sThis = 0;

extern UIColor gHighlightOutlineColor;
extern UIColor gHighlightFillColor;

extern HWND	gTooltip;
extern void AddTooltipFromControlID( HWND TooltipWindow, HWND ParentWindow, UINT ControlID, char *Tooltip );
extern void ClearDefPushButtonLook( HWND hwndDlg, UINT nControlID );
extern void MoveSizeDlgControl( HWND hwnd, UINT itemID, UIPoint Move, UIPoint Size );

static const float cMaxZoomLevel = 8.0f;

//-----------------------------------------------------------------

SelectRegionDialogBox::SelectRegionDialogBox( void ) : 
mHwnd(0),
mStyle(0),
mImage(0),
mDisplay(0),
mZoomLevel(1.0f),
mImageSize(0,0),
mScrollOffset(0,0),
mOldSize(0,0),
mDrawHighlight(true)
{
	mSelectionRect.SetUseBounds (true);
}

//-----------------------------------------------------------------

bool SelectRegionDialogBox::EditProperty( HWND hwndParent, UIImageStyle *theStyle, UIString &Value )
{
	bool rv = false;

	mImage  = new UIImage;

	mStyle = UI_ASOBJECT(UIImageStyle, theStyle->DuplicateObject());
	const UICanvas * const theCanvas = theStyle->GetSourceCanvas();

	if (!theCanvas)
		return false;

	theCanvas->GetSize (mImageSize);

	const UIRect bounds (mImageSize);
	mSelectionRect.SetBounds (bounds);

	mImage->SetCanvas     (const_cast<UICanvas *>(theCanvas));
	mImage->SetSourceRect (&bounds);

	mImage->SetSize( mImageSize );
	UIUtils::ParseRect( Value, mRegion );
	mSelectionRect.SetRect( mRegion );
	
	sThis = this;

	if( DialogBox( GetModuleHandle(0), MAKEINTRESOURCE(IDD_SELECTREGION), hwndParent, StaticDialogProc ) )
	{
		UIUtils::FormatRect( Value, mRegion );
		rv = true;
	}

	mImage = 0;
	mStyle = 0;

	return rv;
}

//-----------------------------------------------------------------

bool SelectRegionDialogBox::EditProperty( HWND hwndParent, UIImage *theImage, UIString &Value )
{
	bool rv = false;

	mImage = UI_ASOBJECT(UIImage, theImage->DuplicateObject());	

	const UICanvas *theCanvas = mImage->GetCanvas();

	if( !theCanvas )
		return false;

	const UIRect src( 0, 0, theCanvas->GetWidth(), theCanvas->GetHeight() );
	mImage->SetSourceRect( &src );
	mImageSize = UISize( theCanvas->GetWidth(), theCanvas->GetHeight() );
	mImage->SetColor(UIColor::white);
	mImage->SetBackgroundOpacity(0.0f);
	mImage->SetMaximumSize( mImageSize );
	mImage->SetSize( mImageSize );
	mSelectionRect.SetBounds ( UIRect ( 0, 0, mImageSize.x, mImageSize.y ) );
	UIUtils::ParseRect( Value, mRegion );
	mSelectionRect.SetRect( mRegion );

	sThis = this;

	if( DialogBox( GetModuleHandle(0), MAKEINTRESOURCE(IDD_SELECTREGION), hwndParent, StaticDialogProc ) )
	{
		UIUtils::FormatRect( Value, mRegion );
		rv = true;
	}

	mImage = 0;
	mStyle = 0;

	return rv;
}

//-----------------------------------------------------------------

void SelectRegionDialogBox::UpdateTextbox( void )
{
	UIString RectString;
	UIUtils::FormatRect( RectString, mRegion );
	SetDlgItemText( mHwnd, IDC_VALUE, Unicode::wideToNarrow (RectString).c_str() );
}

//-----------------------------------------------------------------

void SelectRegionDialogBox::UpdateSelectionRect( void )
{
	mSelectionRect.SetRect( mRegion );
}

//-----------------------------------------------------------------

void SelectRegionDialogBox::ReadRectFromTextbox( void )
{
	HWND  hControl   = GetDlgItem( mHwnd, IDC_VALUE );
	long  TextLength = SendMessage( hControl, WM_GETTEXTLENGTH, 0, 0 ) + 1;
	char *Buffer     = new char[ TextLength ];

	SendMessage( hControl, WM_GETTEXT, TextLength, (LPARAM)Buffer );

	UIUtils::ParseRect( Buffer, mRegion );
}

//-----------------------------------------------------------------

void SelectRegionDialogBox::EnableControls( void )
{
	if( mZoomLevel <= 1.0f )
	{
		EnableWindow( GetDlgItem( mHwnd, IDC_ZOOMIN ), TRUE );
		EnableWindow( GetDlgItem( mHwnd, IDC_ZOOMOUT ), FALSE );
	}
	else if( mZoomLevel >= cMaxZoomLevel )
	{
		EnableWindow( GetDlgItem( mHwnd, IDC_ZOOMIN ), FALSE );
		EnableWindow( GetDlgItem( mHwnd, IDC_ZOOMOUT ), TRUE );
	}
	else
	{
		EnableWindow( GetDlgItem( mHwnd, IDC_ZOOMIN ), TRUE );
		EnableWindow( GetDlgItem( mHwnd, IDC_ZOOMOUT ), TRUE );
	}
}

//-----------------------------------------------------------------

void SelectRegionDialogBox::UpdateScrollbars( HWND hwnd, const UIPoint * center)
{
	UISize ZoomedImageSize( UIScalar( float(mImageSize.x) * mZoomLevel ), UIScalar( float(mImageSize.y) * mZoomLevel ) );
	RECT rc;

	GetWindowRect( hwnd, &rc );

	rc.right -= rc.left;
	rc.left   = 0;

	rc.bottom -= rc.top;
	rc.top     = 0;

	if( rc.right < ZoomedImageSize.x || rc.bottom < ZoomedImageSize.y )
		GetClientRect( hwnd, &rc );
	
	UISize NewSize( rc.right, rc.bottom );

	//-----------------------------------------------------------------
	//-- center on the specified pixel

	if (center)
	{
		mScrollOffset.x = static_cast<long>(static_cast<float>(center->x) * mZoomLevel) - rc.right / 2;
		mScrollOffset.y = static_cast<long>(static_cast<float>(center->y) * mZoomLevel) - rc.bottom / 2;
	}

	// Validate scroll positions
	if( mScrollOffset.x + NewSize.x > ZoomedImageSize.x )
		mScrollOffset.x = ZoomedImageSize.x - NewSize.x;

	if( mScrollOffset.x < 0 )
		mScrollOffset.x = 0;

	if( mScrollOffset.y + NewSize.y > ZoomedImageSize.y )
		mScrollOffset.y = ZoomedImageSize.y - NewSize.y;

	if( mScrollOffset.y < 0 )
		mScrollOffset.y = 0;

	SCROLLINFO si;
	si.cbSize = sizeof( si );
	si.fMask  = SIF_PAGE | SIF_POS | SIF_RANGE;
	si.nPage  = NewSize.x;
	si.nMin   = 0;
	si.nMax   = ZoomedImageSize.x;
	si.nPos   = mScrollOffset.x;

	SetScrollInfo( hwnd, SB_HORZ, &si, TRUE );
	
	si.cbSize = sizeof( si );
	si.fMask  = SIF_PAGE | SIF_POS | SIF_RANGE;
	si.nPage  = NewSize.y;
	si.nMin   = 0;
	si.nMax   = ZoomedImageSize.y;
	si.nPos   = mScrollOffset.y;

	SetScrollInfo( hwnd, SB_VERT, &si, TRUE );
}

//-----------------------------------------------------------------

void SelectRegionDialogBox::UpdateCanvasScrollPosition( HWND hwnd, UINT uMsg, WPARAM wParam )
{
	UISize ZoomedImageSize( UIScalar( float(mImageSize.x) * mZoomLevel ), UIScalar( float(mImageSize.y) * mZoomLevel ) );
	RECT	 rc;

	GetWindowRect( hwnd, &rc );

	rc.right -= rc.left;
	rc.left   = 0;

	rc.bottom -= rc.top;
	rc.top     = 0;

	if( rc.right < ZoomedImageSize.x || rc.bottom < ZoomedImageSize.y )
		GetClientRect( hwnd, &rc );
	
	UISize NewSize( rc.right, rc.bottom );

	if( uMsg == WM_HSCROLL )
	{
		switch( LOWORD( wParam ) )
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

				GetScrollInfo( hwnd, SB_HORZ, &si );
				mScrollOffset.x = si.nTrackPos;
				break;
		}
	}
	else
	{
		switch( LOWORD( wParam ) )
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

				GetScrollInfo( hwnd, SB_VERT, &si );
				mScrollOffset.y = si.nTrackPos;
				break;
		}
	}
}

//-----------------------------------------------------------------

void SelectRegionDialogBox::SizeToContent( void )
{
	RECT rcDialog;
	RECT rcImage;
	RECT rcDesktop;

	GetWindowRect( mHwnd, &rcDialog );
	GetWindowRect( GetDlgItem( mHwnd, IDC_CANVAS ), &rcImage );

	// Note, I'm not using this rect as a rect, the right and bottom elements
	// are actually the width and height.
	rcDialog.right	-= rcDialog.left;			rcDialog.left = 0;
	rcDialog.bottom -= rcDialog.top;			rcDialog.top  = 0;

	rcImage.right	 -= rcImage.left;				rcImage.left = 0;
	rcImage.bottom -= rcImage.top;				rcImage.top  = 0;

	UISize ZoomedImageSize( UIScalar( float(mImageSize.x) * mZoomLevel ), UIScalar( float(mImageSize.y) * mZoomLevel ) );

	while( (ZoomedImageSize.x < rcImage.right) && (ZoomedImageSize.y < rcImage.bottom ) )
	{
		mZoomLevel *= 2.0f;

		ZoomedImageSize.x = UIScalar( float(mImageSize.x) * mZoomLevel );
		ZoomedImageSize.y = UIScalar( float(mImageSize.y) * mZoomLevel );

		if( mZoomLevel >= cMaxZoomLevel )
		{
			mZoomLevel = cMaxZoomLevel;
			break;
		}
	}

	rcDialog.right  = rcDialog.right  - rcImage.right  + ZoomedImageSize.x + 1;
	rcDialog.bottom = rcDialog.bottom - rcImage.bottom + ZoomedImageSize.y + 1;
	
	SystemParametersInfo( SPI_GETWORKAREA, 0, &rcDesktop, 0 );

	// Center on desktop
	rcDialog.left   = (rcDesktop.right - rcDesktop.left) / 2 + rcDesktop.left - rcDialog.right / 2;
	rcDialog.top		= (rcDesktop.bottom - rcDesktop.top) / 2 + rcDesktop.top - rcDialog.bottom / 2;

	MoveWindow( mHwnd, rcDialog.left, rcDialog.top, rcDialog.right, rcDialog.bottom, TRUE );
}

//-----------------------------------------------------------------

void SelectRegionDialogBox::ShowZoomLevel( void )
{
	char Buffer[256];

	if( mZoomLevel != 1.0f )
		sprintf( Buffer, "(%d%% Zoom)", int(mZoomLevel * 100.0f) );
	else
		sprintf( Buffer, "(Actual Pixels)" );

	SetDlgItemText( mHwnd, IDC_ZOOMLEVEL, Buffer );
}

//-----------------------------------------------------------------

LPARAM SelectRegionDialogBox::InverseTranslateLParam( LPARAM lParam )
{
	long x = LOWORD( lParam );
	long y = HIWORD( lParam );

	x += mScrollOffset.x;
	y += mScrollOffset.y;

	x = static_cast<long>( static_cast<float>(x) / mZoomLevel );
	y = static_cast<long>( static_cast<float>(y) / mZoomLevel );

	return MAKELPARAM( x, y );
}

//-----------------------------------------------------------------

BOOL CALLBACK SelectRegionDialogBox::DialogProc( HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam )
{
	switch( uMsg )
	{
		case WM_INITDIALOG:
		{
			mHwnd = hwndDlg;
			UpdateTextbox();
			UpdateSelectionRect();

			HWND theCanvasWindow = GetDlgItem( hwndDlg, IDC_CANVAS );

			if( theCanvasWindow )
			{
				sThis = this;
				SetWindowLong( theCanvasWindow, GWL_WNDPROC, (LONG)StaticCanvasProc );				
				SendMessage( theCanvasWindow, WM_APP, 0, 0 );
			}

			AddTooltipFromControlID( gTooltip, hwndDlg, IDC_ZOOMIN, "Zoom In" );
			AddTooltipFromControlID( gTooltip, hwndDlg, IDC_ZOOMOUT, "Zoom Out" );

			SendDlgItemMessage( hwndDlg, IDC_ZOOMIN, BM_SETIMAGE, IMAGE_ICON,
				(LPARAM)LoadImage( GetModuleHandle(0), MAKEINTRESOURCE(IDI_ZOOMIN), IMAGE_ICON, 16, 16, 0 ) );
			SendDlgItemMessage( hwndDlg, IDC_ZOOMOUT, BM_SETIMAGE, IMAGE_ICON,
				(LPARAM)LoadImage( GetModuleHandle(0), MAKEINTRESOURCE(IDI_ZOOMOUT), IMAGE_ICON, 16, 16, 0 ) );

			SendDlgItemMessage( hwndDlg, IDC_DRAWHIGHLIGHT, BM_SETIMAGE, IMAGE_ICON,
				(LPARAM)LoadImage( GetModuleHandle(0), MAKEINTRESOURCE(IDI_DRAWHIGHLIGHT), IMAGE_ICON, 16, 16, 0 ) );
			CheckDlgButton( hwndDlg, IDC_DRAWHIGHLIGHT, BST_CHECKED );

			mZoomLevel = 1.0f;

			RECT rcClient;
			GetClientRect( hwndDlg, &rcClient );
			SendMessage( hwndDlg, WM_SIZE, 0, MAKELPARAM( rcClient.right, rcClient.bottom ) );

			SizeToContent();
			EnableControls();
			ShowZoomLevel();
			return TRUE;
		}

		case WM_CLOSE:
			// Equivalent to pressing cancel
			EndDialog( hwndDlg, 0 );
			return 0;

		case WM_GETMINMAXINFO:
		{
			LPMINMAXINFO pmmi = (LPMINMAXINFO)lParam;
			RECT         rc;

			rc.left   = 0;
			rc.top    = 0;
			rc.right  = mInitialSize.x;
			rc.bottom = mInitialSize.y;

			AdjustWindowRectEx( &rc, GetWindowLong( hwndDlg, GWL_STYLE ), TRUE, GetWindowLong( hwndDlg, GWL_EXSTYLE ) );
			pmmi->ptMinTrackSize.x = rc.right - rc.left;
			pmmi->ptMinTrackSize.y = rc.bottom - rc.top;

			return 0;
		}

		case WM_SIZE:
		{
			UISize NewSize;
			
			NewSize.x = LOWORD(lParam);
			NewSize.y = HIWORD(lParam);

			if( (NewSize.x == 0) || (NewSize.y == 0) )
				return 0;

			if( mOldSize.x != 0 )
			{
				UISize DeltaSize = NewSize - mOldSize;

				MoveSizeDlgControl( hwndDlg, IDC_CANVAS,				UIPoint(0,0),											UIPoint( DeltaSize.x, DeltaSize.y ) );
				MoveSizeDlgControl( hwndDlg, IDOK,							UIPoint(DeltaSize.x,0),						UIPoint( 0, 0 )											);
				MoveSizeDlgControl( hwndDlg, IDCANCEL,					UIPoint(DeltaSize.x,0),						UIPoint( 0, 0 )											);
				MoveSizeDlgControl( hwndDlg, IDC_DRAWHIGHLIGHT,	UIPoint(DeltaSize.x,DeltaSize.y),	UIPoint( 0, 0 )											);
				MoveSizeDlgControl( hwndDlg, IDC_ZOOMIN,				UIPoint(DeltaSize.x,DeltaSize.y),	UIPoint( 0, 0 )											);
				MoveSizeDlgControl( hwndDlg, IDC_ZOOMOUT,				UIPoint(DeltaSize.x,DeltaSize.y),	UIPoint( 0, 0 )											);
				MoveSizeDlgControl( hwndDlg, IDC_ZOOMLEVEL,			UIPoint(DeltaSize.x,DeltaSize.y),	UIPoint( 0, 0 )											);
				MoveSizeDlgControl( hwndDlg, IDC_VALUELABEL,		UIPoint(0,DeltaSize.y),						UIPoint( 0, 0 )											);
				MoveSizeDlgControl( hwndDlg, IDC_VALUE,					UIPoint(0,DeltaSize.y),						UIPoint( DeltaSize.x, 0 )						);
			}
			else
				mInitialSize = NewSize;

			mOldSize = NewSize;
			
			return 0;
		}

		case WM_COMMAND:			
			if( LOWORD( wParam ) == IDOK )
			{	
				ReadRectFromTextbox();				
				EndDialog( hwndDlg, 1 );
			}
			else if( LOWORD( wParam ) == IDCANCEL )
				EndDialog( hwndDlg, 0 );
			else if( LOWORD( wParam ) == IDC_DRAWHIGHLIGHT )
			{
				mDrawHighlight = IsDlgButtonChecked( hwndDlg, IDC_DRAWHIGHLIGHT ) != 0;
				SetFocus( GetDlgItem( hwndDlg, IDC_VALUE ) );
				InvalidateRect( GetDlgItem( hwndDlg, IDC_CANVAS ), 0, FALSE ); 
			}
			else if( LOWORD( wParam ) == IDC_ZOOMIN )
			{
				if( HIWORD(wParam) == BN_CLICKED )
				{
					SetFocus( GetDlgItem( hwndDlg, IDC_VALUE ) );
					ClearDefPushButtonLook( hwndDlg, IDC_ZOOMIN );

					mZoomLevel *= 2.0f;
					if( mZoomLevel > cMaxZoomLevel )
						mZoomLevel = cMaxZoomLevel;
					EnableControls();
					ShowZoomLevel();

					const UIRect & rect = mSelectionRect.GetRect ();
					const UIPoint center ((rect.left + rect.right) / 2, (rect.top + rect.bottom) / 2);

					UpdateScrollbars( GetDlgItem( hwndDlg, IDC_CANVAS ), &center );
					InvalidateRect( GetDlgItem( hwndDlg, IDC_CANVAS ), 0, FALSE ); 
				}
			}
			else if( LOWORD( wParam ) == IDC_ZOOMOUT )
			{
				if( HIWORD(wParam) == BN_CLICKED )
				{
					SetFocus( GetDlgItem( hwndDlg, IDC_VALUE ) );
					ClearDefPushButtonLook( hwndDlg, IDC_ZOOMOUT );

					mZoomLevel /= 2.0f;
					if( mZoomLevel < 1.0f )
						mZoomLevel = 1.0f;
					EnableControls();
					ShowZoomLevel();

					const UIRect & rect = mSelectionRect.GetRect ();
					const UIPoint center ((rect.left + rect.right) / 2, (rect.top + rect.bottom) / 2);

					UpdateScrollbars( GetDlgItem( hwndDlg, IDC_CANVAS ), &center );
					InvalidateRect( GetDlgItem( hwndDlg, IDC_CANVAS ), 0, FALSE ); 
				}
			}
			else if( LOWORD( wParam ) == IDC_VALUE )
			{
				if( HIWORD( wParam ) == EN_CHANGE )
				{
					ReadRectFromTextbox();
					UpdateSelectionRect();
					InvalidateRect( GetDlgItem( hwndDlg, IDC_CANVAS ), 0, FALSE ); 
				}
			}

			return 0;

		default:
			return 0;
	}
}

//-----------------------------------------------------------------

BOOL CALLBACK SelectRegionDialogBox::CanvasProc( HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam )
{
	switch( uMsg )
	{
		case WM_GETDLGCODE:
			return DLGC_WANTARROWS;

		case WM_NCHITTEST:
		{
			BOOL rc = DefWindowProc( hwnd, uMsg, wParam, lParam );

			if( rc == HTNOWHERE )
				rc = HTCLIENT;

			return rc;
		}

		case WM_MOUSEACTIVATE:
			return MA_ACTIVATE;

		case WM_APP:
		{
			RECT rc;
			GetClientRect( hwnd, &rc );
			mDisplay = new UIDirect3DPrimaryCanvas( UISize( rc.right, rc.bottom ), hwnd, false );
			mDisplay->Attach( 0 );

			DWORD dwStyle = GetWindowLong( hwnd, GWL_STYLE );

			dwStyle |= WS_HSCROLL;
			dwStyle |= WS_VSCROLL;

			SetWindowLong( hwnd, GWL_STYLE, dwStyle );
			SendMessage( hwnd, WM_SIZE, 0, MAKELPARAM( rc.right, rc.bottom ) );
			return 0;
		}

		case WM_SIZE:
		{
			UISize NewSize( LOWORD(lParam), HIWORD(lParam) );
			UISize ZoomedImageSize( UIScalar( float(mImageSize.x) * mZoomLevel ), UIScalar( float(mImageSize.y) * mZoomLevel ) );
			mDisplay->SetSize( NewSize );

			UpdateScrollbars( hwnd );
			return 0;
		}

		case WM_HSCROLL:
		case WM_VSCROLL:
		{
			UpdateCanvasScrollPosition( hwnd, uMsg, wParam );
			UpdateScrollbars( hwnd );
			InvalidateRect( hwnd, 0, FALSE );
			return 0;
		}

		case WM_PAINT:
		{
			PAINTSTRUCT ps;

			BeginPaint( hwnd, &ps );
			EndPaint( hwnd, &ps );

			if( mDisplay->BeginRendering() )
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

				if( mDrawHighlight )
				{
					mSelectionRect.SetFillColor( gHighlightFillColor );
					mSelectionRect.SetOutlineColor( gHighlightOutlineColor );				
					mSelectionRect.SetCanDrawHandles( true );
					mSelectionRect.Render( mDisplay );					
				}

				mDisplay->EndRendering();
				mDisplay->Flip();

				mDisplay->PopState();
			}
			return 0;
		}

		case WM_LBUTTONDOWN:
		case WM_MBUTTONDOWN:
		case WM_RBUTTONDOWN:
			SetCapture( hwnd );
			mSelectionRect.ProcessMessage( hwnd, uMsg, wParam, InverseTranslateLParam( lParam ) );
			InvalidateRect( hwnd, 0, FALSE );
			mRegion = mSelectionRect.GetRect();
			UpdateTextbox();
			return 0;

		case WM_LBUTTONUP:
		case WM_MBUTTONUP:
		case WM_RBUTTONUP:
			ReleaseCapture();
			mSelectionRect.ProcessMessage( hwnd, uMsg, wParam, InverseTranslateLParam( lParam ) );
			InvalidateRect( hwnd, 0, FALSE );
			mRegion = mSelectionRect.GetRect();
			UpdateTextbox();
			return 0;

		case WM_MOUSEMOVE:
		case WM_KEYDOWN:		
			mSelectionRect.ProcessMessage( hwnd, uMsg, wParam, InverseTranslateLParam( lParam ) );
			InvalidateRect( hwnd, 0, FALSE );
			mRegion = mSelectionRect.GetRect();
			UpdateTextbox();
			return 0;

		case WM_ERASEBKGND:
			return 1;

		case WM_DESTROY:
			mDisplay->Detach();
			mDisplay = 0;
			return DefWindowProc( hwnd, uMsg, wParam, lParam );

		default:
			return DefWindowProc( hwnd, uMsg, wParam, lParam );
	}
}

//-----------------------------------------------------------------

BOOL CALLBACK SelectRegionDialogBox::StaticCanvasProc( HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam )
{
	SelectRegionDialogBox *thisCache = static_cast<SelectRegionDialogBox *>( GetProp( hwnd, "this" ) );

	if( !thisCache )
	{
		thisCache = sThis;
		SetProp( hwnd, "this", sThis );
	}

	return thisCache->CanvasProc( hwnd, uMsg, wParam, lParam );
}

//-----------------------------------------------------------------

BOOL CALLBACK SelectRegionDialogBox::StaticDialogProc( HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam )
{
	SelectRegionDialogBox *thisCache = static_cast<SelectRegionDialogBox *>( GetProp( hwnd, "this" ) );

	if( !thisCache )
	{
		thisCache = sThis;
		SetProp( hwnd, "this", sThis );
	}

	return thisCache->DialogProc( hwnd, uMsg, wParam, lParam );
}