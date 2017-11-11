#include "_precompile.h"

#include "UIClipboard.h"
#include "UnicodeUtils.h"

#if WIN32
#include <windows.h>

#endif

UIClipboard *UIClipboard::gSingleton = 0;

UIClipboard::UIClipboard() :
mText ()
{
}

UIClipboard::~UIClipboard()
{
}

void UIClipboard::SetText( const UIString &NewText )
{
#if WIN32
	
	// Attempt to gain access to the clipboard
	
	if (OpenClipboard(0))
	{
		// Allocate a global memory object from Windows.
		
		HGLOBAL hMem = GlobalAlloc(GMEM_MOVEABLE | GMEM_DDESHARE, (NewText.size () + 1) * sizeof(unsigned short));
		if (hMem != NULL)
		{
			
			void * const pMem = GlobalLock(hMem);
			if (pMem != NULL)
			{							
				// @todo: use unicode clipboard
				wcscpy((unsigned short *)pMem, NewText.c_str ());
				GlobalUnlock(hMem);								
				
				// Clear the clipboard

				if (EmptyClipboard())
				{
//					OutputDebugString("Empty clipboard failure\n");
				}

				// Set the clipboard data

				HANDLE handle = SetClipboardData(CF_UNICODETEXT, hMem);

				if (handle == NULL)
				{
					char text[256];
					sprintf(text, "Clipboard fail: error: %d\n", static_cast<int>(GetLastError()));
					OutputDebugString(text);
				}
			}

			else 				
				GlobalFree(hMem);
		}			
		
		CloseClipboard();
	}
#else
	
	mText = NewText;	

#endif

}

void UIClipboard::GetText( UIString &Out ) const
{	
#if WIN32
	
	// Attempt to gain access to the clipboard
	
	if (OpenClipboard(0))
	{
		// Try to fetch clipboard data in text format.
		// @todo : use unicode on winnt
		
		const HANDLE h = GetClipboardData(CF_UNICODETEXT);
		
		if (h)
		{
			Out = Unicode::String (static_cast<const unsigned short *>(h));

			// @todo the string from the clipboard is corrupted by windows with \r characters.
			// we may need to remove them at this point
		}
		else
		{
			int error = ::GetLastError();
			UI_REPORT_LOG_PRINT(true, ("UIClipboard ReturnedNull GetLastError %d\n", error));
			Out = Unicode::emptyString;
		}

		// Clean up
		
		CloseClipboard();

	}
	else
	{
		int error = ::GetLastError();
		UI_REPORT_LOG_PRINT(true, ("UIClipboard OpenFailed GetLastError %d\n", error));			
		Out = Unicode::emptyString;
	}

#else
	Out = mText

#endif
}

void UIClipboard::EraseText ()
{
	mText.erase ();
}