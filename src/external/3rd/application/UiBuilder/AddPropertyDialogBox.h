#ifndef __ADDPROPERTYDIALOGBOX_H__
#define __ADDPROPERTYDIALOGBOX_H__

#include "UIString.h"
#include <list>

class AddPropertyDialogBox
{
public:

	typedef std::list<UINarrowString> NarrowStringList;
	typedef std::list<UIString>       StringList;

	bool GetNewProperty( HWND hwndParent, UINarrowString &Name, UIString &Value );

private:

	static BOOL CALLBACK DialogProc( HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam );	
};

#endif /* __ADDPROPERTYDIALOGBOX_H__ */
