#include "_precompile.h"

#include "UIMessage.h"

#if WIN32
#include <windows.h>
//#include <ZMouse.h>

unsigned short UIMessage::BackSpace  = 0x1000 | VK_BACK;
unsigned short UIMessage::Insert     = 0x1000 | VK_INSERT;
unsigned short UIMessage::Delete     = 0x1000 | VK_DELETE;
unsigned short UIMessage::LeftArrow  = 0x1000 | VK_LEFT;
unsigned short UIMessage::RightArrow = 0x1000 | VK_RIGHT;
unsigned short UIMessage::UpArrow    = 0x1000 | VK_UP;
unsigned short UIMessage::DownArrow  = 0x1000 | VK_DOWN;
unsigned short UIMessage::Home       = 0x1000 | VK_HOME;
unsigned short UIMessage::End        = 0x1000 | VK_END;
unsigned short UIMessage::PageUp     = 0x1000 | VK_PRIOR;
unsigned short UIMessage::PageDown   = 0x1000 | VK_NEXT;
unsigned short UIMessage::Tab        = 0x1000 | VK_TAB;
unsigned short UIMessage::Space      = ' ';
unsigned short UIMessage::Enter      = '\r';
unsigned short UIMessage::Escape     = 27;
unsigned short UIMessage::Copy       = 0x1000 | 'c';
unsigned short UIMessage::Paste      = 0x1000 | 'v';
unsigned short UIMessage::Cut        = 0x1000 | 'x';
unsigned short UIMessage::Undo       = 0x1000 | 'z';
unsigned short UIMessage::SelectAll  = 0x1000 | 'a';

#endif

//----------------------------------------------------------------------

namespace
{
	namespace KeyCodes
	{
		const unsigned short cut         = 'x';
		const unsigned short copy        = 'c';
		const unsigned short paste       = 'v';
		const unsigned short undo        = 'z';
		const unsigned short selectAll   = 'a';
	}
	
	bool isKeyCommand (const UIMessage & msg, unsigned short lowerCode)
	{
		return msg.Modifiers.isControlDown () && tolower (msg.Keystroke) == lowerCode;		
	}
}

//----------------------------------------------------------------------

UIMessage::UIMessage () :
Type (KeyFirst),
Modifiers (),
Keystroke (0),
Data (0),
MouseCoords (),
DragSource (0),
DragObject (0),
DragTarget (0)
{

}

//----------------------------------------------------------------------

bool UIMessage::IsCutCommand () const
{
	return Keystroke == Cut || isKeyCommand (*this, KeyCodes::cut);
}

//----------------------------------------------------------------------

bool UIMessage::IsCopyCommand () const
{
	return Keystroke == Copy || isKeyCommand (*this, KeyCodes::copy);
}

//----------------------------------------------------------------------

bool UIMessage::IsPasteCommand () const
{
	return Keystroke == Paste || isKeyCommand (*this, KeyCodes::paste);
}

//----------------------------------------------------------------------

bool UIMessage::IsUndoCommand () const
{
	return Keystroke == Undo || isKeyCommand (*this, KeyCodes::undo);
}

//----------------------------------------------------------------------

bool UIMessage::IsSelectAllCommand () const
{
	return Keystroke == SelectAll || isKeyCommand (*this, KeyCodes::selectAll);
}

bool UIMessage::IsIMEMessage () const
{
	return (Type > UIMessage::IMEFirst && Type < UIMessage::IMELast);
}

//----------------------------------------------------------------------

#if _WIN32

bool UIMessage::CreateFromWindowsMessage( unsigned int uMsg, long wParam, long lParam )
{
  Modifiers.LeftAlt          = 0 != (0x8000 & GetAsyncKeyState( VK_LMENU ));
  Modifiers.LeftControl      = 0 != (0x8000 & GetAsyncKeyState( VK_LCONTROL ));
  Modifiers.LeftMouseDown    = 0 != (0x8000 & GetAsyncKeyState( VK_LBUTTON ));
  Modifiers.LeftShift        = 0 != (0x8000 & GetAsyncKeyState( VK_LSHIFT ));
  Modifiers.MiddleMouseDown  = 0 != (0x8000 & GetAsyncKeyState( VK_MBUTTON ));
  Modifiers.RightAlt         = 0 != (0x8000 & GetAsyncKeyState( VK_RMENU ));
  Modifiers.RightControl     = 0 != (0x8000 & GetAsyncKeyState( VK_RCONTROL ));
  Modifiers.RightMouseDown   = 0 != (0x8000 & GetAsyncKeyState( VK_RBUTTON ));
  Modifiers.RightShift       = 0 != (0x8000 & GetAsyncKeyState( VK_RSHIFT ));

	if( GetSystemMetrics( SM_SWAPBUTTON ) )
	{
		bool Temp = Modifiers.LeftMouseDown;
		Modifiers.LeftMouseDown = Modifiers.RightMouseDown;
		Modifiers.RightMouseDown = Temp;
	}

	MouseCoords.x = LOWORD( lParam );
	MouseCoords.y = HIWORD( lParam ); //lint !e1924 //win32 bug

	switch( uMsg )
	{
		case WM_LBUTTONDBLCLK:
			Type          = LeftMouseDoubleClick;
			return true;

		case WM_MBUTTONDBLCLK:
			Type          = MiddleMouseDoubleClick;
			return true;

		case WM_RBUTTONDBLCLK:
			Type          = RightMouseDoubleClick;
			return true;

		case WM_LBUTTONDOWN:
			Type          = LeftMouseDown;
			return true;

		case WM_LBUTTONUP:
			Type          = LeftMouseUp;
			return true;

		case WM_MBUTTONDOWN:
			Type          = MiddleMouseDown;
			return true;

		case WM_MBUTTONUP:
			Type          = MiddleMouseUp;
			return true;

		case WM_RBUTTONDOWN:
			Type          = RightMouseDown;
			return true;

		case WM_RBUTTONUP:
			Type          = RightMouseUp;
			return true;

		case WM_MOUSEMOVE:
			Type          = MouseMove;
			return true;

			/*
		case WM_MOUSEWHEEL:
			Type            = MouseWheel;
			Data = HIWORD (wParam);
			MouseCoords.x = LOWORD( lParam );
			MouseCoords.y = HIWORD( lParam );
			return true;
*/
		case WM_CHAR:
			Type          = Character;
			Keystroke     = static_cast<unsigned short>(wParam);
			return true;
			
		case WM_IME_CHAR:
			Type          = Character;
			{
				const char ansiChars [2] =
				{
					static_cast<char>(HIBYTE( wParam )), //lint !e1924 // c-style case msvc bug
					static_cast<char>(LOBYTE( wParam ))  //lint !e1924 // c-style case msvc bug
				};	
				
				wchar_t u;
				if (MultiByteToWideChar (CP_ACP, MB_PRECOMPOSED, reinterpret_cast<const char *>(ansiChars), 2, &u, 1))
				{
					Keystroke     = static_cast<unsigned short>(u);
					return true;
				}
			}
			return false;
			
		case WM_KEYUP:
		case WM_KEYDOWN:
		{
			switch( wParam )
			{
				case VK_BACK:
					Keystroke = BackSpace;
					break;

				case VK_INSERT:
					Keystroke = Insert;
					break;

				case VK_DELETE:
					Keystroke = Delete;
					break;

				case VK_LEFT:
					Keystroke = LeftArrow;
					break;

				case VK_RIGHT:
					Keystroke = RightArrow;
					break;

				case VK_UP:
					Keystroke = UpArrow;
					break;

				case VK_DOWN:
					Keystroke = DownArrow;
					break;

				case VK_HOME:
					Keystroke = Home;
					break;

				case VK_END:
					Keystroke = End;
					break;

				case VK_PRIOR:
					Keystroke = PageUp;
					break;

				case VK_NEXT:
					Keystroke = PageDown;
					break;

				case VK_TAB:
					Keystroke = Tab;
					break;

				default:
				{
					BYTE keyboardstate[256];
					WORD outchars;

					GetKeyboardState( keyboardstate );

					if( ToAscii( static_cast<size_t>(wParam), (static_cast<size_t>(lParam) >> 15) & 0xFF, keyboardstate, &outchars, 0 ) != 1 )
						return false;

					Keystroke = static_cast<unsigned short> (outchars & 0xFF);
					break;
				}
			}

			if( uMsg == WM_KEYDOWN )
			{
				if( (lParam & 0xFFFF) != 0 )
					Type = KeyRepeat;
				else
					Type = KeyDown;
			}
			else
			{
				Type = KeyUp;
			}			
			return true;
		}
		default:
			return false;
	}
}

//----------------------------------------------------------------------

bool UIMessage::MovesFocus         () const
{
	return (Type >= MouseFirst) && (Type <= MouseLastFocusChanger) && Type != LeftMouseUp && Type != MiddleMouseUp && Type != RightMouseUp;

}

//----------------------------------------------------------------------

#endif // _WIN32
