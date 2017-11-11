#include "FirstUiBuilder.h"
#include "DefaultObjectPropertiesManager.h"

#include "AddPropertyDialogBox.h"
#include "UnicodeUtils.h"
#include "UIString.h"
#include "UIWidget.h"
#include "resource.h"

#include <commctrl.h>
#include <cstdio>

#undef min

extern HWND	gTooltip;
extern HWND	gMainWindow;

extern void ClearDefPushButtonLook( HWND hwndDlg, UINT nControlID );
extern void AddTooltipFromControlID( HWND TooltipWindow, HWND ParentWindow, UINT ControlID, char *Tooltip );

DefaultObjectPropertiesManager::DefaultObjectPropertiesManager( void )
{
	struct
	{
		UITypeID		TypeID;
		const char     *TypeName;
		const char     *SizeStr;
	}
	InitialDefaults[] =
	{	
		{ TUIButton,           "Button",           "64,32" },
		{ TUICheckbox,         "Checkbox",         "64,32" },
		{ TUIComboBox,         "ComboBox",         "300,16"},
		{ TUIComposite,        "Composite",        "480,320" },
		{ TUICursorSet,        "CursorSet",        0 },
		{ TUIData,             "Data",             0 },
		{ TUIDataSource,       "DataSource",       0 },
		{ TUIDataSourceContainer,"DataSourceContainer",0 },
		{ TUIDeformerHUD,      "DeformerHud",      0 },
		{ TUIDeformerRotate,   "DeformerRotate",   0 },
		{ TUIDeformerWave,     "DeformerWave",     0 },
		{ TUIDropdownbox,      "Dropdownbox",      "128,32" },
		{ TUIEllipse,          "Ellipse",          "128,128" },
		{ TUIFontCharacter,    "FontCharacter",    "16,16" },
		{ TUIGridStyle,        "GridStyle",        0 },
		{ TUIImage,            "Image",            "128,128" },
		{ TUIImageStyle,       "ImageStyle",       0 },
		{ TUIList,             "List",             "128,160" },
		{ TUIListStyle,        "ListStyle",        0 },
		{ TUIListbox,          "Listbox",          "128,160" },
		{ TUIListboxStyle,     "ListboxStyle",     0 },
		{ TUINamespace,        "Namespace",        0 },
		{ TUIPage,             "Page",             "320,240" },
		{ TUIPalette,          "Palette",          0 },
		{ TUIPie,              "Pie",              "128,128" },
		{ TUIPieStyle,         "PieStyle",         0 },
		{ TUIPopupMenu,        "PopupMenu",       "32,128" },
		{ TUIPopupMenuStyle,   "PopupMenuStyle",   0 },
		{ TUIProgressbar,      "Progressbar",      "128,32" },
		{ TUIProgressbarStyle, "ProgressbarStyle", 0 },
		{ TUIRadialMenu,       "RadialMenu",      "32,128" },
		{ TUIRunner,           "Runner",           "128,256" },
		{ TUIScrollbar,        "Scrollbar",        "32,128" },
		{ TUIScrollbarStyle,   "ScrollbarStyle",   0 },
		{ TUISliderbar,        "Sliderbar",        "128,32" },
		{ TUISliderbarStyle,   "SliderbarStyle",   0 },
		{ TUISliderbarStyle,   "SliderbarStyle",   0 },
		{ TUISliderplane,      "Sliderplane",      "128,128" },
		{ TUITabSet,           "TabSet",           "128,64" },
		{ TUITabSetStyle,      "TabSetStyle",      0 },
		{ TUITabbedPane,       "TabbedPane",       "256,32" },
		{ TUITabbedPaneStyle,  "TabbedPaneStyle",  0 },
		{ TUITable,            "Table",            "300,400" },
		{ TUITableHeader,      "TableHeader",      "300,32" },
		{ TUITableModelDefault,"TableModelDefault",0 },
		{ TUITemplate,         "Template",         0 },
		{ TUIText,             "Text",             "128,64" },
		{ TUITextbox,          "Textbox",          "128,32" },
		{ TUITextboxStyle,     "TextboxStyle",     0 },
		{ TUITextStyleManager, "TextStyleManager", "en" },
		{ TUITreeView,         "TreeView",         "300,400"},
		{ TUIVolumePage,       "VolumePage",       "128,128" },
	};

	for( int i = 0; i < sizeof(InitialDefaults) / sizeof(InitialDefaults[0]); ++i )
	{
		DefaultObjectProperties DefaultProperty;

		DefaultProperty.TypeID   = InitialDefaults[i].TypeID;
		DefaultProperty.TypeName = InitialDefaults[i].TypeName;

		UIString NameProperty (UI_UNICODE_T("New "));

		NameProperty += UIUnicode::narrowToWide (InitialDefaults[i].TypeName);
		DefaultProperty.Properties["Name"] = NameProperty;

		
		if (DefaultProperty.TypeID == TUITextStyleManager)
			DefaultProperty.Properties["FontLocale"] = Unicode::narrowToWide (InitialDefaults[i].SizeStr);
		else if (InitialDefaults [i].SizeStr)
			DefaultProperty.Properties["Size"] = Unicode::narrowToWide (InitialDefaults [i].SizeStr);

		mDefaultObjectProperties.push_back( DefaultProperty );
	}

	mSelectedPropertyMap = 0;
	mPropertyListbox = 0;
}

void DefaultObjectPropertiesManager::ApplyDefaultPropertiesToObject( UIBaseObject *TargetObject ) const
{
	for( DefaultObjectPropertiesList::const_iterator i = mDefaultObjectProperties.begin(); i != mDefaultObjectProperties.end(); ++i )
	{
		if(_stricmp(TargetObject->GetTypeName(), i->TypeName.c_str()) == 0)
		{
			for( StringMap::const_iterator j = i->Properties.begin(); j != i->Properties.end(); ++j )
				TargetObject->SetProperty( UILowerString (j->first), j->second );

			break;
		}
	}

	if (TargetObject->IsA (TUIWidget))
	{
		UIWidget * const wid    = static_cast<UIWidget *>(TargetObject);	
		const UIWidget * const parent = wid->GetParentWidget ();

		if (parent)
		{
			UISize size = wid->GetSize ();
			const UISize & parentSize = parent->GetSize ();
			size.x = std::min (size.x, parentSize.x);
			size.y = std::min (size.y, parentSize.y);
			wid->SetSize (size);
		}
	}
}

BOOL DefaultObjectPropertiesManager::DefaultObjectPropertiesManagerWindowProc( HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam )
{
	switch( uMsg )
	{
		case WM_INITDIALOG:			
			mPropertyListbox = GetDlgItem( hwndDlg, IDC_PROPERTYLIST );
			
			AddTooltipFromControlID( gTooltip, gMainWindow, IDC_ADDPROPERTY, "Add Default Property" );
			AddTooltipFromControlID( gTooltip, gMainWindow, IDC_REMOVEPROPERTY, "Remove Default Property" );

			SendDlgItemMessage( hwndDlg, IDC_ADDPROPERTY, BM_SETIMAGE, IMAGE_ICON,
				(LPARAM)LoadImage( GetModuleHandle(0), MAKEINTRESOURCE(IDI_ADDPROPERTY), IMAGE_ICON, 16, 16, 0 ) );

			SendDlgItemMessage( hwndDlg, IDC_REMOVEPROPERTY, BM_SETIMAGE, IMAGE_ICON,
				(LPARAM)LoadImage( GetModuleHandle(0), MAKEINTRESOURCE(IDI_REMOVEPROPERTY), IMAGE_ICON, 16, 16, 0 ) );

			LoadDataToDialog( hwndDlg );
			return TRUE;

		case WM_CLOSE:
			// Equivalent to pressing cancel
			EndDialog( hwndDlg, 0 );
			return 0;

		case WM_COMMAND:
			if( LOWORD( wParam ) == IDOK )
			{
				mDefaultObjectProperties = mWorkingObjectProperties;
				EndDialog( hwndDlg, 0 );
			}
			else if( LOWORD( wParam ) == IDCANCEL )
			{
				EndDialog( hwndDlg, 0 );
			}
			else if( (LOWORD( wParam ) == IDC_OBJECTTYPE) && (HIWORD( wParam ) == LBN_SELCHANGE) )
				LoadDefaultsToListbox( hwndDlg );
			else if( (LOWORD( wParam ) == IDC_ADDPROPERTY) && (HIWORD( wParam ) == BN_CLICKED) )
				AddNewProperty( hwndDlg );
			else if( (LOWORD( wParam ) == IDC_REMOVEPROPERTY) && (HIWORD( wParam ) == BN_CLICKED) )
				RemoveSelectedProperty( hwndDlg );

			return 0;

		case WM_DRAWITEM:
			HandleOwnerDraw( hwndDlg, wParam, lParam );
			return 0;

		default:
			return 0;
	}
}

BOOL CALLBACK DefaultObjectPropertiesManager::DefaultObjectPropertiesManagerStaticWindowProc( HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam )
{
	static const char *PropertyName = "DefaultObjectPropertiesManager::this";

	if( uMsg == WM_INITDIALOG )
	{
		SetProp( hwndDlg, PropertyName, (HANDLE)lParam );
		return reinterpret_cast<DefaultObjectPropertiesManager *>( lParam )->DefaultObjectPropertiesManagerWindowProc( hwndDlg, uMsg, wParam, lParam );
	}
	else
	{
		DefaultObjectPropertiesManager *pManager = 
			reinterpret_cast<DefaultObjectPropertiesManager *>( GetProp( hwndDlg, PropertyName ) );

		if( uMsg == WM_DESTROY )
			RemoveProp( hwndDlg, PropertyName );

		return pManager->DefaultObjectPropertiesManagerWindowProc( hwndDlg, uMsg, wParam, lParam );
	}
}

void DefaultObjectPropertiesManager::DisplayEditDialog( HWND hParent )
{
	DialogBoxParam( GetModuleHandle(0), MAKEINTRESOURCE(IDD_DEFAULTPROPERTIES),
		hParent, DefaultObjectPropertiesManagerStaticWindowProc, (LPARAM)this );
}

void DefaultObjectPropertiesManager::LoadDataToDialog( HWND hwndDlg )
{
	mWorkingObjectProperties = mDefaultObjectProperties;

	for( DefaultObjectPropertiesList::const_iterator i = mWorkingObjectProperties.begin(); i != mWorkingObjectProperties.end(); ++i )
	{
		const UINarrowString & tname = (*i).TypeName;

		long InsertedAt = SendDlgItemMessage( hwndDlg, IDC_OBJECTTYPE, CB_ADDSTRING, 0, reinterpret_cast<LPARAM>(tname.c_str ()) );
		SendDlgItemMessage( hwndDlg, IDC_OBJECTTYPE, CB_SETITEMDATA, InsertedAt, (LPARAM)i->TypeID );
	}

	SendDlgItemMessage( hwndDlg, IDC_OBJECTTYPE, CB_SETCURSEL, 0, 0 );
	LoadDefaultsToListbox( hwndDlg );
}

void DefaultObjectPropertiesManager::LoadDefaultsToListbox( HWND hwndDlg )
{
	SendMessage( mPropertyListbox, LB_RESETCONTENT, 0, 0 );

	long CurrentSelection = SendDlgItemMessage( hwndDlg, IDC_OBJECTTYPE, CB_GETCURSEL, 0, 0 );

	if( CurrentSelection >= 0 )
	{
		UITypeID TypeID = (UITypeID)SendDlgItemMessage( hwndDlg, IDC_OBJECTTYPE, CB_GETITEMDATA, CurrentSelection, 0 );
		mSelectedPropertyMap = 0;

		for( DefaultObjectPropertiesList::iterator i = mWorkingObjectProperties.begin(); i != mWorkingObjectProperties.end(); ++i )
		{
			if( i->TypeID == TypeID )
			{
				mSelectedPropertyMap = &(i->Properties);

				for( StringMap::iterator j = i->Properties.begin(); j != i->Properties.end(); ++j )
				{
					const UINarrowString & name = (*j).first;
					SendMessage( mPropertyListbox, LB_ADDSTRING, 0, reinterpret_cast<LPARAM> (name.c_str ()));
				}
			}
		}
	}
}

void DefaultObjectPropertiesManager::HandleOwnerDraw( HWND hwndDlg, WPARAM wParam, LPARAM lParam )
{
	UI_UNREF (lParam);
	UI_UNREF (wParam);
	UI_UNREF (hwndDlg);

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

		if( hwndFocus == mPropertyListbox )
		{
			FillRect( DoubleBufferDC, &rcFill, GetSysColorBrush( COLOR_HIGHLIGHT ) );
			SetTextColor( DoubleBufferDC, GetSysColor( COLOR_HIGHLIGHTTEXT ) );
		}
		else
		{
			FillRect( DoubleBufferDC, &rcFill, GetSysColorBrush( COLOR_BTNFACE ) );
			SetTextColor( DoubleBufferDC, GetSysColor( COLOR_BTNTEXT ) );
		}
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
		
		StringMap::iterator i = mSelectedPropertyMap->find( PropertyName );
		
		if( i != mSelectedPropertyMap->end() )
		{
			Value = (*i).second;
		}
		
		DrawText( DoubleBufferDC, UIUnicode::wideToNarrow (Value).c_str(), -1, &rcClip, DT_SINGLELINE );
		delete PropertyName;
	}
	
	BitBlt( pdis->hDC, pdis->rcItem.left, pdis->rcItem.top, ItemSize.x, ItemSize.y, DoubleBufferDC, 0, 0, SRCCOPY );

	SelectObject( DoubleBufferDC, OldFont );
	SelectObject( DoubleBufferDC, OldBitmap );
	DeleteObject( DoubleBufferBitmap );
	DeleteDC( DoubleBufferDC );
}

void DefaultObjectPropertiesManager::AddNewProperty( HWND hwndDlg )
{
	AddPropertyDialogBox AddProp;
	UINarrowString       NewName;
	UIString             NewValue;

	if( AddProp.GetNewProperty( hwndDlg, NewName, NewValue ) )
	{
		(*mSelectedPropertyMap)[NewName] = NewValue;
		LoadDefaultsToListbox(hwndDlg);
	}
	SetFocus(mPropertyListbox);
	ClearDefPushButtonLook( hwndDlg, IDC_ADDPROPERTY );
}

void DefaultObjectPropertiesManager::RemoveSelectedProperty( HWND hwndDlg )
{
	long  CurrentSelection = SendMessage( mPropertyListbox, LB_GETCURSEL, 0, 0 );

	if( CurrentSelection >= 0 )
	{
		long  TextLength  = SendMessage( mPropertyListbox, LB_GETTEXTLEN, CurrentSelection, 0 );
		char *ItemText    = new char[TextLength + 1];

		SendMessage( mPropertyListbox, LB_GETTEXT, CurrentSelection, (LPARAM)ItemText );
		
		StringMap::iterator i = mSelectedPropertyMap->find( ItemText );
		
		if( i != mSelectedPropertyMap->end() )
			mSelectedPropertyMap->erase(i);

		LoadDefaultsToListbox(hwndDlg);
	}
	SetFocus(mPropertyListbox);
	ClearDefPushButtonLook( hwndDlg, IDC_REMOVEPROPERTY );
}

void DefaultObjectPropertiesManager::SaveTo( FILE *fp )
{
	fprintf( fp, "<DefaultObjectPropertiesManager>\n" );

	for( DefaultObjectPropertiesList::iterator i = mDefaultObjectProperties.begin();
	     i != mDefaultObjectProperties.end(); ++i )
	{
		fprintf( fp, "<%s>\n", i->TypeName );

		for( StringMap::iterator j = i->Properties.begin(); j != i->Properties.end(); ++j )
		{
			const UINarrowString & name = (*j).first;
			const UINarrowString value = Unicode::wideToNarrow ((*j).second);

			fprintf( fp, "'%s' = '%s'\n", name.c_str (), value.c_str ());
		}

		fprintf( fp, "</%s>\n", i->TypeName );
	}

	fprintf( fp, "</DefaultObjectPropertiesManager>\n" );
}

void DefaultObjectPropertiesManager::LoadFrom( FILE *fp )
{
	char LineBuffer[1024];

	while( fgets( LineBuffer, sizeof( LineBuffer ), fp ) )
	{
		char *pNewLine = strchr( LineBuffer, '\n' );

		if( pNewLine )
			*pNewLine = '\0';

		if( !_stricmp( LineBuffer, "<DefaultObjectPropertiesManager>" ) )
			continue;
		else if( !_stricmp( LineBuffer, "</DefaultObjectPropertiesManager>" ) )
			break;
		else if( *LineBuffer == '<' )
		{
			mSelectedPropertyMap = 0;

			if( *(LineBuffer + 1) == '/' )
				continue;

			if( strchr( LineBuffer + 1, '>' ) )
				*strchr( LineBuffer + 1, '>' ) = '\0';


			for( DefaultObjectPropertiesList::iterator i = mDefaultObjectProperties.begin(); i != mDefaultObjectProperties.end(); ++i )
			{
				if( !_stricmp( LineBuffer + 1, i->TypeName.c_str() ) )
				{
					mSelectedPropertyMap = &(i->Properties);
					break;
				}
			}
		}
		else
		{
			UINarrowString Name;
			UIString Value;
			char    *p = LineBuffer;
			bool     bInQuote = false;
			bool     bInValue = false;

			while( *p )
			{
				if( *p == '\'' )
				{
					if( bInQuote )
						bInQuote = false;
					else
						bInQuote = true;
				}
				else if( bInQuote )
				{
					if( bInValue )
						Value += *p;
					else
						Name += *p;
				}
				else if( *p == '=' )
					bInValue = true;

				p++;
			}

			if( mSelectedPropertyMap )
				(*mSelectedPropertyMap)[Name] = Value;
		}
	}	
}