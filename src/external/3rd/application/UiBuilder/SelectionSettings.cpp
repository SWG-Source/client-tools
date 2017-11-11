#include "FirstUiBuilder.h"

#include "resource.h"
#include "uitypes.h"

#include <commctrl.h>

static HBRUSH  gFillColorBrush = 0;
static HBRUSH  gEdgeColorBrush = 0;

static UIColor gCurrentEdgeColor;
static UIColor gCurrentFillColor;

extern bool    gDrawHighlightRect;
extern UIColor gHighlightOutlineColor;
extern UIColor gHighlightFillColor;

static void LoadData( HWND hwndDlg )
{
	if( gDrawHighlightRect )
		CheckDlgButton( hwndDlg, IDC_DRAWHIGHLIGHT, BST_CHECKED );
	else
		CheckDlgButton( hwndDlg, IDC_DRAWHIGHLIGHT, BST_UNCHECKED );

	SendDlgItemMessage( hwndDlg, IDC_EDGEOPACITY_SLIDER, TBM_SETRANGE, FALSE, MAKELONG( 0, 255 ) );
	SendDlgItemMessage( hwndDlg, IDC_EDGEOPACITY_SLIDER, TBM_SETPOS, TRUE, gHighlightOutlineColor.a );

	SendDlgItemMessage( hwndDlg, IDC_FILLOPACITY_SLIDER, TBM_SETRANGE, FALSE, MAKELONG( 0, 255 ) );
	SendDlgItemMessage( hwndDlg, IDC_FILLOPACITY_SLIDER, TBM_SETPOS, TRUE, gHighlightFillColor.a );	

	gCurrentEdgeColor = gHighlightOutlineColor;
	gCurrentFillColor = gHighlightFillColor;
}

static void SaveData( HWND hwndDlg )
{
	gHighlightOutlineColor	 = gCurrentEdgeColor;
	gHighlightOutlineColor.a = (unsigned char)SendDlgItemMessage( hwndDlg, IDC_EDGEOPACITY_SLIDER, TBM_GETPOS, 0, 0 );
	gHighlightFillColor			 = gCurrentFillColor;	
	gHighlightFillColor.a		 = (unsigned char)SendDlgItemMessage( hwndDlg, IDC_FILLOPACITY_SLIDER, TBM_GETPOS, 0, 0 );
	gDrawHighlightRect			 = IsDlgButtonChecked( hwndDlg, IDC_DRAWHIGHLIGHT ) != 0;
}

static void ChangeEdgeColor( HWND hwndDlg )
{
	CHOOSECOLOR cc = {sizeof(cc)};
	COLORREF    sc[16];

	ZeroMemory( sc, sizeof( sc ) );

	cc.hwndOwner    = hwndDlg;
	cc.rgbResult    = RGB( gCurrentEdgeColor.r, gCurrentEdgeColor.g, gCurrentEdgeColor.b );
	cc.Flags        = CC_RGBINIT;	
	cc.lpCustColors = sc;

	if( ChooseColor( &cc ) )
	{
		gCurrentEdgeColor.r = GetRValue( cc.rgbResult );
		gCurrentEdgeColor.g = GetGValue( cc.rgbResult );
		gCurrentEdgeColor.b = GetBValue( cc.rgbResult );
	}
}

static void ChangeFillColor( HWND hwndDlg )
{
	CHOOSECOLOR cc = {sizeof(cc)};
	COLORREF    sc[16];

	ZeroMemory( sc, sizeof( sc ) );

	cc.hwndOwner    = hwndDlg;
	cc.rgbResult    = RGB( gCurrentFillColor.r, gCurrentFillColor.g, gCurrentFillColor.b );
	cc.Flags        = CC_RGBINIT;	
	cc.lpCustColors = sc;

	if( ChooseColor( &cc ) )
	{
		gCurrentFillColor.r = GetRValue( cc.rgbResult );
		gCurrentFillColor.g = GetGValue( cc.rgbResult );
		gCurrentFillColor.b = GetBValue( cc.rgbResult );
	}
}

static void Cleanup( void )
{
	if( gFillColorBrush )
	{
		DeleteObject( gFillColorBrush );
		gFillColorBrush = 0;
	}

	if( gEdgeColorBrush )
	{
		DeleteObject( gEdgeColorBrush );
		gEdgeColorBrush = 0;
	}
}

BOOL CALLBACK HighlightSettingsDlgProc( HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam )
{
	switch( uMsg )
	{
		case WM_INITDIALOG:			
			LoadData( hwndDlg );
			return TRUE;

		case WM_CLOSE:
			// Equivalent to pressing cancel
			EndDialog( hwndDlg, 0 );
			Cleanup();
			return 0;

		case WM_COMMAND:
			if( LOWORD( wParam ) == IDOK )
			{
				SaveData( hwndDlg );
				EndDialog( hwndDlg, 0 );
				Cleanup();
			}
			else if( LOWORD( wParam ) == IDCANCEL )
			{
				EndDialog( hwndDlg, 0 );
				Cleanup();
			}
			else if( LOWORD( wParam ) == IDC_CHANGE_EDGECOLOR )
			{
				ChangeEdgeColor( hwndDlg );
				InvalidateRect( GetDlgItem( hwndDlg, IDC_EDGECOLOR ), 0, TRUE );
			}
			else if( LOWORD( wParam ) == IDC_CHANGE_FILLCOLOR )
			{
				ChangeFillColor( hwndDlg );
				InvalidateRect( GetDlgItem( hwndDlg, IDC_FILLCOLOR ), 0, TRUE );
			}
			return 0;

		case WM_CTLCOLORSTATIC:
			if( (HWND)lParam == GetDlgItem( hwndDlg, IDC_EDGECOLOR ) )
			{
				COLORREF col = RGB( gCurrentEdgeColor.r, gCurrentEdgeColor.g, gCurrentEdgeColor.b );

				if( gEdgeColorBrush )
					DeleteObject( gEdgeColorBrush );

				gEdgeColorBrush = CreateSolidBrush( col );
				SetTextColor( (HDC)wParam, col );
				SetBkColor( (HDC)wParam, col );

				return (BOOL)gEdgeColorBrush;
			}
			else if( (HWND)lParam == GetDlgItem( hwndDlg, IDC_FILLCOLOR ) )
			{
				COLORREF col = RGB( gCurrentFillColor.r, gCurrentFillColor.g, gCurrentFillColor.b );

				if( gFillColorBrush )
					DeleteObject( gFillColorBrush );

				gFillColorBrush = CreateSolidBrush( col );
				SetTextColor( (HDC)wParam, col );
				SetBkColor( (HDC)wParam, col );

				return (BOOL)gFillColorBrush;
			}
			return 0;

		default:
			return 0;
	}
}