#include "FirstUiBuilder.h"
#include "EditPropertyDialogBox.h"

#include "resource.h"

UINarrowString gName;
UIString gValue;

bool EditPropertyDialogBox::EditProperty( HWND hwndParent, const char *Name, UIString &Value )
{
	gName  = Name;
	gValue = Value;

	if( DialogBox( GetModuleHandle(0), MAKEINTRESOURCE(IDD_EDITPROPERTY), hwndParent, DialogProc ) )
	{		
		Value = gValue;
		return true;
	}
	else
	{
		return false;
	}
}

BOOL CALLBACK EditPropertyDialogBox::DialogProc( HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam )
{
	UI_UNREF (lParam);
	
	switch( uMsg )
	{
	case WM_INITDIALOG:
		{
			const int height = HIWORD (GetDialogBaseUnits ());
			HWND  hControl   = GetDlgItem( hwndDlg, IDC_VALUE );

			HFONT fixedFont = CreateFont (height * 3 / 4, 0, 
				0, 0,
				0, 0, 0, 0,
				ANSI_CHARSET,
				OUT_DEFAULT_PRECIS,
				CLIP_DEFAULT_PRECIS,
				DEFAULT_QUALITY,
				FF_DONTCARE,
				"lucida console");
			
			SendMessage (hControl, WM_SETFONT, reinterpret_cast<WPARAM>(fixedFont), 1);

			SetDlgItemText( hwndDlg, IDC_VALUE, UIUnicode::wideToNarrow (gValue).c_str() );
			return TRUE;
		}

		case WM_CLOSE:
			// Equivalent to pressing cancel
			EndDialog( hwndDlg, 0 );
			return 0;

		case WM_COMMAND:
			if( LOWORD( wParam ) == IDOK )
			{	
				HWND  hControl   = GetDlgItem( hwndDlg, IDC_VALUE );
				long  TextLength = SendMessage( hControl, WM_GETTEXTLENGTH, 0, 0 ) + 1;
				char *Buffer     = new char[ TextLength ];

				SendMessage( hControl, WM_GETTEXT, TextLength, (LPARAM)Buffer );

				gValue = UIUnicode::narrowToWide (Buffer);
				EndDialog( hwndDlg, 1 );
			}
			else if( LOWORD( wParam ) == IDCANCEL )
				EndDialog( hwndDlg, 0 );

			return 0;

		default:
			return 0;
	}
}