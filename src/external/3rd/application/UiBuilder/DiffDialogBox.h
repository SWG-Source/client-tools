#pragma once

//////////////////////////////////////////////////////////////////////////

class UIBaseObject;

//////////////////////////////////////////////////////////////////////////

class DiffDialogBox
{
public:

	bool DiffObjects(HWND hwndParent, UIBaseObject const & lhs, UIBaseObject const & rhs);

private:

	static BOOL CALLBACK DialogProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);
};
