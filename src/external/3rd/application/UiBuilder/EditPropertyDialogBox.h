#ifndef __EDITPROPERTYDIALOGBOX_H__
#define __EDITPROPERTYDIALOGBOX_H__

#include "UIString.h"

class EditPropertyDialogBox
{
public:

	bool EditProperty( HWND hwndParent, const char *Name, UIString &Value );

private:

	static BOOL CALLBACK DialogProc( HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam );	
};

#endif /* __EDITPROPERTYDIALOGBOX_H__ */
