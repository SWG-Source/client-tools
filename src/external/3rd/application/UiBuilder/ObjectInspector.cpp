#include "FirstUiBuilder.h"
#include "ObjectInspector.h"

#include "UIBaseObject.h"
#include "UIWidget.h"
#include "UIPage.h"
#include "UIManager.h"
#include "UIUtils.h"
#include "UIImageStyle.h"
#include "UIImage.h"

#include "resource.h"
#include "AddPropertyDialogBox.h"
#include "EditPropertyDialogBox.h"
#include "SelectRegion.h"

#include <cstdio>
#include <commctrl.h>
#include <vector>

extern void		CheckOutSelectedFile();
extern bool		gDrawHighlightRect;
extern HWND	 	gObjectTree;
extern HANDLE	gFrameRenderingMutex;
extern UIBaseObject::UISmartObjectList       gCurrentSelection;


HTREEITEM GetObjectInTreeControlFromHandle( HWND hTree, HTREEITEM hParentItem, UIBaseObject *o );
void SetSelection( UIBaseObject *NewSelection, bool pushHistory );
void LoadTopLevelObjectsToTabControl( void );
void ClearDefPushButtonLook( HWND hwndDlg, UINT nControlID );

extern const char * gVisualEditLockPropertyName;

void recordUndo(UIBaseObject * const object, bool force = false);
extern bool volatile s_UndoReady;


namespace
{
	bool s_updateAllFromTextControl = true;
}


UINT CALLBACK ColorDialogHookProc( HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam )
{
	UI_UNREF (lParam);
	UI_UNREF (wParam);

	if( uMsg == WM_INITDIALOG )
	{
		RECT rcWindow;
		RECT rcDesktop;
		long WindowWidth;
		long WindowHeight;

		GetWindowRect( GetDesktopWindow(), &rcDesktop );
		GetWindowRect( hwnd, &rcWindow );

		WindowWidth = rcWindow.right - rcWindow.left;
		WindowHeight = rcWindow.bottom - rcWindow.top;

		rcWindow.left = (rcDesktop.right - rcDesktop.left) / 2 - WindowWidth / 2;
		rcWindow.top  = (rcDesktop.bottom - rcDesktop.top) / 2 - WindowHeight / 2;

		MoveWindow( hwnd, rcWindow.left, rcWindow.top, WindowWidth, WindowHeight, TRUE );
	}
	return 0;
}

ObjectInspector::ObjectInspector( HWND hWnd )
{
	HWND hParent     = GetParent( hWnd );
	mPropertyListbox = hWnd;
	mObject          = 0;
	mTimerID         = 1007;

	SetWindowLong( mPropertyListbox, GWL_STYLE, GetWindowLong( mPropertyListbox, GWL_STYLE ) | WS_CLIPCHILDREN );

	mOldWindowProc = (WNDPROC)GetWindowLong( hParent, GWL_WNDPROC );
	SetProp( hParent, "ObjectInspector::this", this );
	SetWindowLong( hParent, GWL_WNDPROC, (long)StaticWindowProc );

	SetTimer( hParent, mTimerID, 100, 0 );

	mTextOverlay = CreateWindowEx( WS_EX_TOPMOST, "Edit", "", WS_CHILD | WS_BORDER | ES_AUTOHSCROLL | ES_LEFT,
										0, 0, 0, 0, mPropertyListbox, (HMENU)1001, GetModuleHandle(0), 0 );

//	HFONT hFont = (HFONT)SendMessage( mPropertyListbox, WM_GETFONT, 0, 0 );

	const int height = HIWORD (GetDialogBaseUnits ());
	
	HFONT fixedFont = CreateFont (height * 3 / 4, 0, 
		0, 0,
		0, 0, 0, 0,
		ANSI_CHARSET,
		OUT_DEFAULT_PRECIS,
		CLIP_DEFAULT_PRECIS,
		DEFAULT_QUALITY,
		FF_DONTCARE,
		"lucida console");
	
	SendMessage (hWnd, WM_SETFONT, reinterpret_cast<WPARAM>(fixedFont), 1);

	SendMessage( mTextOverlay, WM_SETFONT, (WPARAM)fixedFont, 0 );

	SetProp( mTextOverlay, "ObjectInspector::this", this );
	mOldTextboxWindowProc = (WNDPROC)GetWindowLong( mTextOverlay, GWL_WNDPROC );
	SetWindowLong( mTextOverlay, GWL_WNDPROC, (long)StaticTextboxWindowProc );
	
	UpdateButtonEnabledState();
}

ObjectInspector::~ObjectInspector( void )
{
	HWND hParent = GetParent( mPropertyListbox );

	assert( (WNDPROC)GetWindowLong( hParent, GWL_WNDPROC ) == StaticWindowProc );

	KillTimer( hParent, mTimerID );
	SetWindowLong( hParent, GWL_WNDPROC, (long)mOldWindowProc );
	RemoveProp( hParent, "ObjectInspector::this" );

	RemoveProp( mTextOverlay, "ObjectInspector::this" );
	DestroyWindow( mTextOverlay );
}

//-----------------------------------------------------------------

void ObjectInspector::SetObject( UIBaseObject *o )
{
	if( o != mObject )
	{
		s_updateAllFromTextControl = false;
		UpdatePropertyFromTextControl();
		mObject = o;
		LoadListboxWithObjectProperties();
	}

	UpdateTextControlFromProperty();
}

//-----------------------------------------------------------------

UIBaseObject *ObjectInspector::GetObject( void ) const
{
	return const_cast<UIBaseObject *>(mObject.pointer());
}

//-----------------------------------------------------------------

bool ObjectInspector::GetSelectedPropertyName( UINarrowString &Out )
{
	long CurrentSelection = SendMessage( mPropertyListbox, LB_GETCURSEL, 0, 0 );

	if( CurrentSelection != -1 )
	{
		int   BufferSize   = SendMessage( mPropertyListbox, LB_GETTEXTLEN, CurrentSelection, 0 );
		char *PropertyName = new char[BufferSize+1];

		SendMessage( mPropertyListbox, LB_GETTEXT, CurrentSelection, (LPARAM)PropertyName );
		Out = PropertyName;
		delete PropertyName;
		return true;
	}

	return false;
}

void ObjectInspector::UpdateButtonEnabledState( void )
{
	BOOL EnableAddButton    = FALSE;
	BOOL EnableRemoveButton = FALSE;
	BOOL EnableLockButton   = FALSE;
	BOOL PressLockButton    = FALSE;

	if( mObject )
	{
		UINarrowString PropertyName;

		EnableAddButton = TRUE;

		if( GetSelectedPropertyName( PropertyName ) )
		{
			if( mObject->IsPropertyRemovable( UILowerString (PropertyName )))
				EnableRemoveButton = TRUE;
		}

		if( mObject->IsA( TUIWidget ) )
			EnableLockButton = TRUE;

		if( mObject->HasProperty( UILowerString (gVisualEditLockPropertyName ) ))
			PressLockButton = TRUE;
	}

	HWND hParent = GetParent( mPropertyListbox );

	EnableWindow( GetDlgItem( hParent, IDC_ADDPROPERTY ), EnableAddButton );
	EnableWindow( GetDlgItem( hParent, IDC_REMOVEPROPERTY ), EnableRemoveButton );
	EnableWindow( GetDlgItem( hParent, IDC_LOCK), EnableLockButton );

	if( PressLockButton )
		CheckDlgButton( hParent, IDC_LOCK, BST_CHECKED );
	else
		CheckDlgButton( hParent, IDC_LOCK, BST_UNCHECKED );
}

void ObjectInspector::LoadListboxWithObjectProperties( void )
{
	int Selection = SendMessage(mPropertyListbox, LB_GETCURSEL, 0, 0);

	if( Selection == -1 )
		Selection = 0;

	SendMessage( mPropertyListbox, LB_RESETCONTENT, 0, 0 );

	if( !mObject )
	{
		UpdateButtonEnabledState();
		return;
	}

	UIBaseObject::UIPropertyNameVector PropertyNames;
	mObject->GetPropertyNames( PropertyNames, false );

	for( UIBaseObject::UIPropertyNameVector::const_iterator i = PropertyNames.begin(); i != PropertyNames.end(); ++i )
	{
		UILowerString const & lname = *i;
		char const * const lnameStr = lname.c_str();
		if (lname != UIBaseObject::PropertyName::SourceFile && 
			(_stricmp( lname.c_str (), gVisualEditLockPropertyName ) != 0) &&
			(lnameStr && *lnameStr != '!'))
			SendMessage( mPropertyListbox, LB_ADDSTRING, 0, (long)lname.c_str () );
	}

	SendMessage( mPropertyListbox, LB_SETCURSEL, Selection, 0 );

	HWND hParent = GetParent( mPropertyListbox );

	char ObjectType[256];
	sprintf(ObjectType, "%s", mObject->GetTypeName());
	SetWindowText( GetDlgItem( hParent, IDC_OBJECTTYPE ), ObjectType );

	UIString sourcePath;
	if (UIManager::gUIManager().GetRootPage() == mObject) 
	{
		sourcePath = Unicode::narrowToWide("ui_root");
	}
	else
	{
		mObject->GetProperty(UIBaseObject::PropertyName::SourceFile, sourcePath);
	}

	sprintf(ObjectType, "%s", Unicode::wideToNarrow(sourcePath).c_str(), mObject->GetTypeName());
	SetWindowText(GetDlgItem( hParent, IDC_OBJECTPROPERTIES_LABEL), ObjectType);

	UpdateButtonEnabledState();
}

LRESULT CALLBACK ObjectInspector::StaticWindowProc( HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam )
{
	ObjectInspector *pThis = reinterpret_cast<ObjectInspector *>( GetProp( hwnd, "ObjectInspector::this" ) );
	return pThis->WindowProc( hwnd, uMsg, wParam, lParam );
}

LRESULT CALLBACK ObjectInspector::StaticTextboxWindowProc( HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam )
{
	ObjectInspector *pThis = reinterpret_cast<ObjectInspector *>( GetProp( hwnd, "ObjectInspector::this" ) );

	if( pThis )
		return pThis->TextboxWindowProc( hwnd, uMsg, wParam, lParam );
	else
		return 0;
}

//-----------------------------------------------------------------

namespace
{
	void setPropertyOnSelection (const UILowerString & Name, const Unicode::String & Value)
	{
		for (UIBaseObject::UISmartObjectList::iterator it = gCurrentSelection.begin (); it != gCurrentSelection.end (); ++it)
		{
			recordUndo(*it, true);
			(*it)->SetProperty (Name, Value);
		}
	}

	void removePropertyOnSelection (const UILowerString & Name)
	{
		for (UIBaseObject::UISmartObjectList::iterator it = gCurrentSelection.begin (); it != gCurrentSelection.end (); ++it)
		{
			recordUndo(*it, true);
			(*it)->RemoveProperty (Name);
		}
	}
}

//-----------------------------------------------------------------

LRESULT ObjectInspector::WindowProc( HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam )
{
	if( (uMsg == WM_DRAWITEM) && GetDlgItem( hwnd, wParam ) == mPropertyListbox )
	{
		
		LPDRAWITEMSTRUCT pdis = (LPDRAWITEMSTRUCT)lParam;
		HBITMAP					 DoubleBufferBitmap, OldBitmap;
		HDC              DoubleBufferDC;
		HFONT            OldFont;
		POINT            ItemSize;
		RECT						 rcFill;
		RECT						 rcClip;
		
		ItemSize.x = pdis->rcItem.right - pdis->rcItem.left;
		ItemSize.y = pdis->rcItem.bottom - pdis->rcItem.top;
		
		SetRect( &rcFill, 0, 0, ItemSize.x, ItemSize.y );
		SetRect( &rcClip, 2, 0, ItemSize.x / 2, ItemSize.y );
		
		DoubleBufferDC		 = CreateCompatibleDC( pdis->hDC );
		DoubleBufferBitmap = CreateCompatibleBitmap( pdis->hDC, ItemSize.x, ItemSize.y );
		
		OldBitmap = (HBITMAP)SelectObject( DoubleBufferDC, DoubleBufferBitmap );
		OldFont		= (HFONT)SelectObject( DoubleBufferDC, GetCurrentObject( pdis->hDC, OBJ_FONT ) );
		
		if( pdis->itemState & ODS_SELECTED )
		{
			HWND hwndFocus = GetFocus();
			
			if( (hwndFocus == mTextOverlay) || (hwndFocus == mPropertyListbox) )
			{
				FillRect( DoubleBufferDC, &rcFill, GetSysColorBrush( COLOR_HIGHLIGHT ) );
				SetTextColor( DoubleBufferDC, GetSysColor( COLOR_HIGHLIGHTTEXT ) );
			}
			else
			{
				FillRect( DoubleBufferDC, &rcFill, GetSysColorBrush( COLOR_BTNFACE ) );
				SetTextColor( DoubleBufferDC, GetSysColor( COLOR_BTNTEXT ) );
			}
			SetTextColor( DoubleBufferDC, GetSysColor( COLOR_HIGHLIGHTTEXT ) );
		}
		else
		{
			FillRect( DoubleBufferDC, &rcFill, GetSysColorBrush( COLOR_WINDOW ) );
			SetTextColor( DoubleBufferDC, GetSysColor( COLOR_WINDOWTEXT ) );
		}
		
		SetBkMode( DoubleBufferDC, TRANSPARENT );
		
		if( pdis->itemID != -1 )
		{
			char *PropertyName = new char[ SendMessage( mPropertyListbox, LB_GETTEXTLEN, pdis->itemID, 0 ) + 1 ];
			SendMessage( mPropertyListbox, LB_GETTEXT, pdis->itemID, (long)PropertyName );
			DrawText( DoubleBufferDC, PropertyName, -1, &rcClip, DT_SINGLELINE );
			
			rcClip.left  = rcClip.right;
			rcClip.right = ItemSize.x;
			
			UIString Value;
			mObject->GetProperty( UILowerString (PropertyName), Value );
			DrawText( DoubleBufferDC, UIUnicode::wideToNarrow (Value).c_str(), -1, &rcClip, DT_SINGLELINE );
			
			delete PropertyName;
		}
		
		BitBlt( pdis->hDC, pdis->rcItem.left, pdis->rcItem.top, ItemSize.x, ItemSize.y, DoubleBufferDC, 0, 0, SRCCOPY );
		
		SelectObject( DoubleBufferDC, OldFont );
		SelectObject( DoubleBufferDC, OldBitmap );
		DeleteObject( DoubleBufferBitmap );
		DeleteDC( DoubleBufferDC );
		return TRUE;
		
	}
	else if( (uMsg == WM_COMMAND) )
	{
		if( LOWORD( wParam ) == IDC_ADDPROPERTY && mObject )
		{
			AddPropertyDialogBox AddProp;
			UINarrowString       NewName;
			UIString             NewValue;
			
			if( AddProp.GetNewProperty( hwnd, NewName, NewValue ) )
			{
				setPropertyOnSelection (UILowerString (NewName), NewValue );
				LoadListboxWithObjectProperties();
			}
			
			SetFocus(mPropertyListbox);
			ClearDefPushButtonLook( hwnd, IDC_ADDPROPERTY );
			return TRUE;
		}
		else if( LOWORD( wParam ) == IDC_REMOVEPROPERTY && mObject )
		{
			long  CurrentSelection;
			long  BufferSize;
			char *SelectionText;
			
			CurrentSelection = SendMessage( mPropertyListbox, LB_GETCURSEL, 0, 0 );
			
			if( CurrentSelection >= 0 )
			{
				BufferSize = SendMessage( mPropertyListbox, LB_GETTEXTLEN, CurrentSelection, 0 );
				
				SelectionText = new char[BufferSize+1];
				SendMessage( mPropertyListbox, LB_GETTEXT, CurrentSelection, (LPARAM)SelectionText );
				removePropertyOnSelection(UILowerString (SelectionText));
				LoadListboxWithObjectProperties();
				delete SelectionText;
			}
			
			SetFocus(mPropertyListbox);
			ClearDefPushButtonLook( hwnd, IDC_REMOVEPROPERTY );
			return TRUE;
		}
		else if( (LOWORD( wParam ) == IDC_CHECKOUT) && (HIWORD( wParam ) == BN_CLICKED) )
		{
			CheckOutSelectedFile();
			return TRUE;
		}
		else if( LOWORD( wParam ) == IDC_LOCK && mObject )
		{
			if( mObject->HasProperty( UILowerString (gVisualEditLockPropertyName) ) )
				removePropertyOnSelection(UILowerString (gVisualEditLockPropertyName));
			else
				setPropertyOnSelection (UILowerString (gVisualEditLockPropertyName), UI_UNICODE_T("true") );
			
			LoadListboxWithObjectProperties();
			SetFocus(mPropertyListbox);
			ClearDefPushButtonLook( hwnd, IDC_REMOVEPROPERTY );
		}
		else if( (HWND)lParam == mPropertyListbox )
		{
			switch( HIWORD(wParam) )
			{
			case LBN_SELCHANGE:
				s_updateAllFromTextControl = false;
				UpdatePropertyFromTextControl();
				UpdateTextControlFromProperty();
				UpdateButtonEnabledState();
				ShowTextControl();
				SetFocus( mTextOverlay );
				break;
			case LBN_SETFOCUS:
				ShowWindow( mTextOverlay, SW_HIDE );
				break;
			case LBN_DBLCLK:
				{
					long  CurrentSelection;
					long  BufferSize;
					char *SelectionText;
					
					CurrentSelection = SendMessage( mPropertyListbox, LB_GETCURSEL, 0, 0 );
					BufferSize = SendMessage( mPropertyListbox, LB_GETTEXTLEN, CurrentSelection, 0 );
					
					SelectionText = new char[BufferSize+1];
					SendMessage( mPropertyListbox, LB_GETTEXT, CurrentSelection, (LPARAM)SelectionText );
					
					if( mObject )
					{
						UIString PropertyValue;
						
						const UILowerString & lowerSelectionText = UILowerString (SelectionText);

						if( mObject->GetProperty( lowerSelectionText, PropertyValue ) )
						{
							if( UIUnicode::icmp (PropertyValue, UI_UNICODE_T ("true")) == 0)
								setPropertyOnSelection (lowerSelectionText, UI_UNICODE_T("false" ) );
							else if( UIUnicode::icmp (PropertyValue, UI_UNICODE_T ("false")) == 0)
								setPropertyOnSelection (lowerSelectionText, UI_UNICODE_T("true" ) );
							else
							{
								UIBaseObject *NewObject = 0;
								
								if( !PropertyValue.empty() )
									NewObject = mObject->GetObjectFromPath( PropertyValue.c_str() );
								
								if( NewObject )
								{
									SetSelection (NewObject, true);
									HTREEITEM	hItemToSelect = GetObjectInTreeControlFromHandle( gObjectTree, 0, NewObject );
									TreeView_SelectItem( gObjectTree, hItemToSelect );
								}
								else
								{
									UIColor theColor;
									
									if( !_stricmp( SelectionText, "SourceRect" ) )
									{
										// Show graphical source rect editor
										bool UpdateProperty = false;
										
										SelectRegionDialogBox SelectRegion;
										
										if( mObject->IsA( TUIImageStyle ) )
										{
											UpdateProperty = SelectRegion.EditProperty(hwnd, UI_ASOBJECT(UIImageStyle, mObject), PropertyValue);
										}
										else if( mObject->IsA( TUIImageFragment ) )
										{
											UIBaseObject *obj = mObject;
											
											while( obj && !obj->IsA( TUIImageStyle ) )
											{
												obj = obj->GetParent();
											}
											
											if( obj )
											{
												UpdateProperty = SelectRegion.EditProperty( hwnd, static_cast<UIImageStyle *>( obj ), PropertyValue );
											}
										}
										else if( mObject->IsA( TUIImage ) )
										{
											UpdateProperty = SelectRegion.EditProperty(hwnd, UI_ASOBJECT(UIImage, mObject), PropertyValue);
										}
										
										if( UpdateProperty )
											setPropertyOnSelection( lowerSelectionText, PropertyValue);
									}
									else if( UIUtils::ParseColor( PropertyValue, theColor ) )
									{
										static COLORREF CustomColors[16] = {0};
										CHOOSECOLOR cc = { sizeof( cc ) };
										
										cc.rgbResult		= RGB( theColor.r, theColor.g, theColor.b );
										cc.Flags				= CC_FULLOPEN | CC_RGBINIT | CC_ANYCOLOR | CC_ENABLEHOOK;
										cc.lpCustColors = CustomColors;
										cc.lpfnHook		  = ColorDialogHookProc;
										
										if( ChooseColor( &cc ) )
										{
											theColor.r = GetRValue( cc.rgbResult );
											theColor.g = GetGValue( cc.rgbResult );
											theColor.b = GetBValue( cc.rgbResult );
											
											UIUtils::FormatColor( PropertyValue, theColor );
											setPropertyOnSelection( lowerSelectionText, PropertyValue);
										}
									}
									else
									{
										EditPropertyDialogBox EditProp;
										
										if( EditProp.EditProperty( hwnd, SelectionText, PropertyValue ) )
											setPropertyOnSelection( lowerSelectionText, PropertyValue);
									}
								}
							}
						}
							delete [] SelectionText;
					}
					break;
				}
			}
			return TRUE;
		}
	}
	else if( (uMsg == WM_TIMER) && (wParam == mTimerID) )
	{
		if (GetFocus () != mTextOverlay)
			InvalidateRect( mPropertyListbox, 0, FALSE );
		return TRUE;
	}
	
	return CallWindowProc( mOldWindowProc, hwnd, uMsg, wParam, lParam );
}

//-----------------------------------------------------------------

LRESULT ObjectInspector::TextboxWindowProc( HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam )
{
	if( uMsg == WM_KILLFOCUS )
	{
		s_updateAllFromTextControl = false;
		UpdatePropertyFromTextControl();
		ShowWindow( hwnd, SW_HIDE );
	}
	else if( uMsg == WM_KEYDOWN )
	{
		if( wParam == VK_TAB )
		{
			SendMessage( GetParent( mPropertyListbox ), WM_NEXTDLGCTL, GetAsyncKeyState(VK_SHIFT) & 0x80000000, FALSE );
			return 0;
		}
		else if( wParam == VK_ESCAPE )
			UpdateTextControlFromProperty();
		else if( (wParam == VK_UP) || (wParam == VK_DOWN) )
			return SendMessage( mPropertyListbox, uMsg, wParam, lParam );
	}
	else if( uMsg == WM_KEYUP )
	{
		if( (wParam == VK_UP) || (wParam == VK_DOWN) )
			return SendMessage( mPropertyListbox, uMsg, wParam, lParam );
	}
	else if( (uMsg == WM_CHAR) && (wParam == VK_RETURN) )
	{
		UpdatePropertyFromTextControl();
		UpdateTextControlFromProperty();
		ShowTextControl();
		return TRUE;
	}
	else if( uMsg == WM_GETDLGCODE )
		return DLGC_WANTALLKEYS | CallWindowProc( mOldTextboxWindowProc, hwnd, uMsg, wParam, lParam );

	return CallWindowProc( mOldTextboxWindowProc, hwnd, uMsg, wParam, lParam );
}

void ObjectInspector::UpdateTextControlFromProperty( void )
{
	if( mObject )
	{
		int itemID = SendMessage( mPropertyListbox, LB_GETCURSEL, 0, 0 );

		if( itemID != -1 )
		{
			char *PropertyName = new char[ SendMessage( mPropertyListbox, LB_GETTEXTLEN, itemID, 0 ) + 1 ];
			SendMessage( mPropertyListbox, LB_GETTEXT, itemID, (long)PropertyName );

			mCurrentPropertyName = PropertyName;

			UIString Value;
			mObject->GetProperty( UILowerString (PropertyName), Value );

			SetWindowText( mTextOverlay, UIUnicode::wideToNarrow (Value).c_str() );

			delete PropertyName;
		}
	}
}

void ObjectInspector::ShowTextControl( void )
{
	int itemID = SendMessage( mPropertyListbox, LB_GETCURSEL, 0, 0 );

	if( itemID != -1 )
	{
		RECT  itemRect;
		POINT editSize;

		SendMessage( mPropertyListbox, LB_GETITEMRECT, itemID, (LPARAM)&itemRect );

		editSize.x = (itemRect.right - itemRect.left) / 2 + 3;
		editSize.y = (itemRect.bottom - itemRect.top);

		MoveWindow( mTextOverlay, (itemRect.right - itemRect.left) - editSize.x, itemRect.top, editSize.x, editSize.y, TRUE );
		SendMessage( mTextOverlay, EM_SETSEL, 0, -1 );
		ShowWindow( mTextOverlay, SW_SHOW );
	}
}

void ObjectInspector::UpdatePropertyFromTextControl( void )
{
	if( IsWindowVisible( mTextOverlay ) )
	{
		int itemID = SendMessage( mPropertyListbox, LB_GETCURSEL, 0, 0 );

		if( itemID != -1 && !mCurrentPropertyName.empty() )
		{
			long TextLen = SendMessage( mTextOverlay, WM_GETTEXTLENGTH, 0, 0 );

			char * const PropertyValue = new char[ TextLen + 1 ];
			SendMessage( mTextOverlay, WM_GETTEXT, TextLen + 1, (LPARAM)PropertyValue );

			WaitForSingleObject( gFrameRenderingMutex, INFINITE );

			if (s_updateAllFromTextControl)
				setPropertyOnSelection (UILowerString (mCurrentPropertyName), UIUnicode::narrowToWide (PropertyValue) );
			else if (mObject)
			{
				recordUndo(mObject, true);
				mObject->SetProperty (UILowerString (mCurrentPropertyName), UIUnicode::narrowToWide (PropertyValue) );
			}

			delete PropertyValue;

			if( mCurrentPropertyName == UIBaseObject::PropertyName::Name.get ())
			{
				InvalidateRect( gObjectTree, NULL, TRUE );

				if( mObject->IsA( TUIPage ) && (mObject->GetParent() == UIManager::gUIManager().GetRootPage() ) )
					LoadTopLevelObjectsToTabControl();
			}

			ReleaseMutex( gFrameRenderingMutex );
		}
	}

	s_updateAllFromTextControl = true;
}

void ObjectInspector::SendKeyDown( unsigned KeyCode )
{
	ShowTextControl();
	SetFocus( mTextOverlay );

	BYTE keyboardstate[256];
	WORD outchars;

	GetKeyboardState( keyboardstate );

	if( ToAscii( KeyCode, 0, keyboardstate, &outchars, 0 ) != 1 )
		return;

	char Keystroke = static_cast<char> (outchars & 0xFF);

	SendMessage( mTextOverlay, WM_CHAR, Keystroke, 0 );
}
