#include "FirstUiBuilder.h"
#include "AddPropertyDialogBox.h"

#include "resource.h"
#include "UITypes.h"

AddPropertyDialogBox::NarrowStringList gOldPropertyNames;
AddPropertyDialogBox::StringList gOldPropertyValues;
UINarrowString gNewPropertyName;
UIString gNewPropertyValue;

bool AddPropertyDialogBox::GetNewProperty( HWND hwndParent, UINarrowString &Name, UIString &Value )
{
	if( DialogBox( GetModuleHandle(0), MAKEINTRESOURCE(IDD_ADDPROPERTY), hwndParent, DialogProc ) )
	{
		Name  = gNewPropertyName;
		Value = gNewPropertyValue;
		return true;
	}
	else
	{
		return false;
	}
}

BOOL CALLBACK AddPropertyDialogBox::DialogProc( HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam )
{
	UI_UNREF (lParam);

	switch( uMsg )
	{
		case WM_INITDIALOG:
		{
			{
				for( NarrowStringList::const_iterator i = gOldPropertyNames.begin(); i != gOldPropertyNames.end(); ++i )
					SendDlgItemMessage( hwndDlg, IDC_PROPERTYNAME, CB_ADDSTRING, 0, (LPARAM)i->c_str() );
			}
			
			{
				for( StringList::const_iterator i = gOldPropertyValues.begin(); i != gOldPropertyValues.end(); ++i )
					SendDlgItemMessage( hwndDlg, IDC_PROPERTYVALUE, CB_ADDSTRING, 0, (LPARAM)i->c_str() );
			}
			return TRUE;
		}
		case WM_CLOSE:
			// Equivalent to pressing cancel
			EndDialog( hwndDlg, 0 );
			return 0;

		case WM_COMMAND:
			if( LOWORD( wParam ) == IDOK )
			{	
				long  len        = SendDlgItemMessage( hwndDlg, IDC_PROPERTYNAME, WM_GETTEXTLENGTH, 0, 0 );
				char *TextBuffer = new char[len + 1];
				GetDlgItemText( hwndDlg, IDC_PROPERTYNAME, TextBuffer, len + 1 );
				gNewPropertyName = TextBuffer;
				delete TextBuffer;

				len        = SendDlgItemMessage( hwndDlg, IDC_PROPERTYVALUE, WM_GETTEXTLENGTH, 0, 0 );
				TextBuffer = new char[len + 1];
				GetDlgItemText( hwndDlg, IDC_PROPERTYVALUE, TextBuffer, len + 1 );
				gNewPropertyValue = UIUnicode::narrowToWide (TextBuffer);
				delete TextBuffer;

				if( gNewPropertyName.empty() )
					EndDialog( hwndDlg, 0 );
				else
				{				
					gOldPropertyNames.push_front( gNewPropertyName );
					gOldPropertyValues.push_front( gNewPropertyValue );
					EndDialog( hwndDlg, 1 );
				}
			}
			else if( LOWORD( wParam ) == IDCANCEL )
				EndDialog( hwndDlg, 0 );

			return 0;

		default:
			return 0;
	}
}