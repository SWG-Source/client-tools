#include "FirstUiBuilder.h"

#include "resource.h"
#include "UITypes.h"

const char *ModificationLog =	"August 2, 2000\r\n" \
															"-------------------------------------\r\n\r\n" \
															"Much optimization of text\r\n" \
															"\r\n" \
															"August 1, 2000\r\n" \
															"-------------------------------------\r\n\r\n" \
															"Added ability to lock down objects to prevent inadvertantly\r\n" \
															"moving or sizing them with the mouse.\r\n\r\n" \
															"Added ability to import other objects.\r\n\r\n" \
															"Added ability to delete objects using the delete key.\r\n" \
															"Implemented default extension (.ui).\r\n" \
															"Added verification dialog whenever a workspace is closed.\r\n" \
															"\r\n" \
															"July 31, 2000\r\n" \
															"-------------------------------------\r\n\r\n" \
															"Added support for assigning default properties to new objects.\r\n" \
															"Defaults are saved in the file defaults.cfg.\r\n" \
															"Implemented better handling of <include> in the editor.\r\n" \
															"\r\n" \
															"July 28, 2000\r\n" \
															"-------------------------------------\r\n\r\n" \
															"Added alignment buttons to the user interface window.\r\n" \
															"Controls are aligned relative to the last control you clicked on.\r\n\r\n" \
															"Added add & remove property buttons to the object inspector window.\r\n\r\n" \
															"Added support for opening files using drag and drop.\r\n\r\n" \
															"Made the cursor change to reflect state of the SHIFT and CTRL keys when selecting.\r\n" \
															"Fixed some crash bugs relating to switching the size of the active page.\r\n" \
															"Made changes to the object structure update the page tab control as well.\r\n" \
															"Fixed various focus issues causes by bad calls to SetWindowPos.\r\n" \
															"\r\n" \
															"July 27, 2000\r\n" \
															"-------------------------------------\r\n\r\n" \
															"Added support for having multiple object in the selection.\r\n" \
															"Use Shift + Click to add items to the selection.\r\n" \
															"Use Ctrl + Click to remove items from the selection.\r\n\r\n" \
															"Added color key and vertex alpha to the editor.\r\n" \
															"Color keys work only on DDS textures.\r\n\r\n" \
															"Changed the way images are searched for.\r\n" \
															"When refering to an image on disk, do not use an extension\r\n" \
															"as the code will seach first for .DDS files and then\r\n" \
															"for .bmp files\r\n" \
															"\r\n\r\n\r\n- End -";

BOOL CALLBACK ModificationLogDlgProc( HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam )
{
	UI_UNREF (lParam);

	switch( uMsg )
	{
		case WM_INITDIALOG:			
			SetDlgItemText( hwndDlg, IDC_MODIFICATION_LOG, ModificationLog );
			return TRUE;

		case WM_CLOSE:
			// Equivalent to pressing cancel
			EndDialog( hwndDlg, 0 );
			return 0;

		case WM_COMMAND:
			if( LOWORD( wParam ) == IDOK )
				EndDialog( hwndDlg, 0 );
			else if( LOWORD( wParam ) == IDCANCEL )
				EndDialog( hwndDlg, 0 );

			return 0;

		default:
			return 0;
	}
}