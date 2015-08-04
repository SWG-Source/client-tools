#include "FirstUiBuilder.h"

#include "resource.h"
#include "uitypes.h"

#include <commctrl.h>

static UIColor CurrentGridColor;
static HBRUSH  gBrush = 0;

extern bool          gDrawGrid;
extern bool          gSnapToGrid;
extern UIColor       gGridColor;
extern unsigned long gXGridStep;
extern unsigned long gYGridStep;
extern unsigned long gGridMajorTicks;

static void LoadData( HWND hwndDlg )
{
	SetDlgItemInt( hwndDlg, IDC_HORIZONTAL_GRID_RESOLUTION, gXGridStep, FALSE );
	SetDlgItemInt( hwndDlg, IDC_VERTICAL_GRID_RESOLUTION,   gYGridStep, FALSE );
	SetDlgItemInt( hwndDlg, IDC_GRID_MAJORTICKS,            gGridMajorTicks, FALSE );

	if( gDrawGrid )
		CheckDlgButton( hwndDlg, IDC_DRAWGRID, BST_CHECKED );
	else
		CheckDlgButton( hwndDlg, IDC_DRAWGRID, BST_UNCHECKED );

	if( gSnapToGrid )
		CheckDlgButton( hwndDlg, IDC_SNAPTOGRID2, BST_CHECKED );
	else
		CheckDlgButton( hwndDlg, IDC_SNAPTOGRID2, BST_UNCHECKED );

	SendDlgItemMessage( hwndDlg, IDC_OPACITY_SLIDER, TBM_SETRANGE, FALSE, MAKELONG( 0, 255 ) );
	SendDlgItemMessage( hwndDlg, IDC_OPACITY_SLIDER, TBM_SETPOS, TRUE, gGridColor.a );

	CurrentGridColor = gGridColor;
}

static void SaveData( HWND hwndDlg )
{
	BOOL Worked;
	long NewValue;

	NewValue = GetDlgItemInt( hwndDlg, IDC_HORIZONTAL_GRID_RESOLUTION, &Worked, FALSE );

	if( Worked )
		gXGridStep = NewValue;

	NewValue = GetDlgItemInt( hwndDlg, IDC_VERTICAL_GRID_RESOLUTION, &Worked, FALSE );

	if( Worked )
		gYGridStep = NewValue;

	NewValue = GetDlgItemInt( hwndDlg, IDC_GRID_MAJORTICKS, &Worked, FALSE );

	if( Worked )
		gGridMajorTicks = NewValue;

	CurrentGridColor.a = (unsigned char)SendDlgItemMessage( hwndDlg, IDC_OPACITY_SLIDER, TBM_GETPOS, 0, 0 );

	gDrawGrid   = IsDlgButtonChecked( hwndDlg, IDC_DRAWGRID ) != 0;
	gSnapToGrid = IsDlgButtonChecked( hwndDlg, IDC_SNAPTOGRID2 ) != 0;
	gGridColor  = CurrentGridColor;
}

static void ChangeGridColor( HWND hwndDlg )
{
	CHOOSECOLOR cc = {sizeof(cc)};
	COLORREF    sc[16];

	ZeroMemory( sc, sizeof( sc ) );

	cc.hwndOwner    = hwndDlg;
	cc.rgbResult    = RGB( CurrentGridColor.r, CurrentGridColor.g, CurrentGridColor.b );
	cc.Flags        = CC_RGBINIT;	
	cc.lpCustColors = sc;

	if( ChooseColor( &cc ) )
	{
		CurrentGridColor.r = GetRValue( cc.rgbResult );
		CurrentGridColor.g = GetGValue( cc.rgbResult );
		CurrentGridColor.b = GetBValue( cc.rgbResult );
	}
}

static void Cleanup( void )
{
	if( gBrush )
	{
		DeleteObject( gBrush );
		gBrush = 0;
	}
}

BOOL CALLBACK GridSettingsDlgProc( HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam )
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
			else if( LOWORD( wParam ) == IDC_CHANGE_GRIDCOLOR )
			{
				ChangeGridColor( hwndDlg );
				InvalidateRect( GetDlgItem( hwndDlg, IDC_GRIDCOLOR ), 0, TRUE );
			}
			return 0;

		case WM_CTLCOLORSTATIC:
			if( (HWND)lParam == GetDlgItem( hwndDlg, IDC_GRIDCOLOR ) )
			{
				COLORREF col = RGB( CurrentGridColor.r, CurrentGridColor.g, CurrentGridColor.b );

				if( gBrush )
					DeleteObject( gBrush );

				gBrush = CreateSolidBrush( col );
				SetTextColor( (HDC)wParam, col );
				SetBkColor( (HDC)wParam, col );

				return (BOOL)gBrush;
			}
			return 0;

		default:
			return 0;
	}
}