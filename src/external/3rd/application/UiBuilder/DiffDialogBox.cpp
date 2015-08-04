//////////////////////////////////////////////////////////////////////////

#include "FirstUiBuilder.h"
#include "DiffDialogBox.h"
#include "UIBaseObject.h"
#include "UILowerString.h"

//////////////////////////////////////////////////////////////////////////

#include "resource.h"
#include <vector>
#include <algorithm>
#include <memory>

namespace DiffDialogBoxNamespace
{
	std::string s_diffStringBuffer;
}

using namespace DiffDialogBoxNamespace;

//////////////////////////////////////////////////////////////////////////

bool DiffDialogBox::DiffObjects(HWND hwndParent, UIBaseObject const & lhs, UIBaseObject const & rhs)
{
	// Do the Diff inline.
	s_diffStringBuffer = lhs.GetFullPath();
	s_diffStringBuffer += " and ";
	s_diffStringBuffer += rhs.GetFullPath();

	// Get properties on the left.
	UIBaseObject::UIPropertyNameVector allProperties;
	lhs.GetPropertyNames(allProperties, false);

	// Get properties on the right.
	rhs.GetPropertyNames(allProperties, false);

	// Merge.
	std::sort(allProperties.begin(), allProperties.end());
	allProperties.erase(std::unique(allProperties.begin(), allProperties.end()), allProperties.end());
	

	// Diff if both objects have it and add to diff text.
	bool hasDifferences = false;
	for (UIBaseObject::UIPropertyNameVector::const_iterator itPropertyName = allProperties.begin(); itPropertyName != allProperties.end(); ++itPropertyName)
	{
		UIString lhsValue;
		UIString rhsValue;
		UILowerString const & property = *itPropertyName;

		if (property == UIBaseObject::PropertyName::Name)
		{
			continue;
		}

		lhs.GetProperty(property, lhsValue);
		rhs.GetProperty(property, rhsValue);

		UILowerString lhsLowerValue(UIUnicode::wideToNarrow(lhsValue));
		UILowerString rhsLowerValue(UIUnicode::wideToNarrow(rhsValue));
		
		// Add Diff!
		if (lhsLowerValue != rhsLowerValue)
		{
			if (!hasDifferences)
			{
				s_diffStringBuffer += allProperties.size() == 1 ? " has the following difference:\r\n\r\n" : 
																  " have the following differences:\r\n\r\n";
				hasDifferences = true;
			}

			s_diffStringBuffer += "[";
			s_diffStringBuffer += property.c_str();
			s_diffStringBuffer += "]\t";
			s_diffStringBuffer += lhsLowerValue.empty() ? "[EMPTY]" : lhsLowerValue.c_str();
			s_diffStringBuffer += " <=> ";
			s_diffStringBuffer += rhsLowerValue.empty() ? "[EMPTY]" : rhsLowerValue.c_str();
			s_diffStringBuffer += "\r\n";
		}
	}

	if (!hasDifferences)
	{
		s_diffStringBuffer += " are not different.";
	}


	if (DialogBox(GetModuleHandle(0), MAKEINTRESOURCE(IDD_DIFF_WINDOW), hwndParent, DialogProc))
	{		
		return true;
	}
	else
	{
		return false;
	}
}

//////////////////////////////////////////////////////////////////////////

BOOL CALLBACK DiffDialogBox::DialogProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	UI_UNREF(lParam);
	
	switch(uMsg)
	{
	case WM_INITDIALOG:
		{
			const int height = HIWORD(GetDialogBaseUnits());
			HWND hControl = GetDlgItem(hwndDlg, IDC_VALUE);

			HFONT fixedFont = CreateFont(height * 3 / 4, 0, 
				0, 0,
				0, 0, 0, 0,
				ANSI_CHARSET,
				OUT_DEFAULT_PRECIS,
				CLIP_DEFAULT_PRECIS,
				DEFAULT_QUALITY,
				FF_DONTCARE,
				"lucida console");
			
			SendMessage(hControl, WM_SETFONT, reinterpret_cast<WPARAM>(fixedFont), 1);

			SetDlgItemText(hwndDlg, IDC_DIFF_EDIT, s_diffStringBuffer.c_str());
			return TRUE;
		}

		case WM_CLOSE:
			// Equivalent to pressing cancel
			EndDialog(hwndDlg, 0);
			return 0;
			
		case WM_COMMAND:
			if(LOWORD(wParam) == IDOK)
			{
				EndDialog(hwndDlg, 1);
			}
			else if(LOWORD(wParam) == IDCANCEL)
			{	
				EndDialog(hwndDlg, 0);
			}
			return 0;

		default:
			return 0;
	}
}