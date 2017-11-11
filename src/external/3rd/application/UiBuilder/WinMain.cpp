#include "FirstUiBuilder.h"

#include "DDSCanvasGenerator.h"
#include "DefaultObjectPropertiesManager.h"
#include "DiffDialogBox.h"
#include "FileLocator.h"
#include "SetupUi.h"
#include "SimpleSoundCanvas.h"
#include "UIBuilderHistory.h"
#include "UIButton.h"
#include "UIButtonStyle.h"
#include "UICanvasGenerator.h"
#include "UICheckbox.h"
#include "UICheckboxStyle.h"
#include "UIClock.h"
#include "UIColorEffector.h"
#include "UIComboBox.h"
#include "UIComposite.h"
#include "UICursor.h"
#include "UICursorInterface.h"
#include "UICursorSet.h"
#include "UIData.h"
#include "UIDataSource.h"
#include "UIDataSourceContainer.h"
#include "UIDeformerHUD.h"
#include "UIDeformerRotate.h"
#include "UIDeformerWave.h"
#include "UIDirect3DPrimaryCanvas.h"
#include "UIDirect3dTextureCanvas.h"
#include "UIDropdownbox.h"
#include "UIDropdownboxStyle.h"
#include "UIEllipse.h"
#include "UIGrid.h"
#include "UIGridStyle.h"
#include "UIImage.h"
#include "UIImageStyle.h"
#include "UIList.h"
#include "UIListStyle.h"
#include "UIListbox.h"
#include "UIListboxStyle.h"
#include "UILoader.h"
#include "UILocationEffector.h"
#include "UIManager.h"
#include "UIMessage.h"
#include "UINamespace.h"
#include "UINullIMEManager.h"
#include "UIOpacityEffector.h"
#include "UIOutputStream.h"
#include "UIPage.h"
#include "UIPalette.h"
#include "UIPie.h"
#include "UIPieStyle.h"
#include "UIPopupMenu.h"
#include "UIPopupMenuStyle.h"
#include "UIProgressbar.h"
#include "UIProgressbarStyle.h"
#include "UIRadialMenu.h"
#include "UIRadialMenuStyle.h"
#include "UIRectangleStyle.h"
#include "UIRotationEffector.h"
#include "UIRunner.h"
#include "UISaver.h"
#include "UIScrollbar.h"
#include "UISizeEffector.h"
#include "UISliderbar.h"
#include "UISliderbarStyle.h"
#include "UISliderplane.h"
#include "UISliderplaneStyle.h"
#include "UITabSet.h"
#include "UITabSetStyle.h"
#include "UITabbedPane.h"
#include "UITabbedPaneStyle.h"
#include "UITable.h"
#include "UITableHeader.h"
#include "UITableModelDefault.h"
#include "UITemplate.h"
#include "UIText.h"
#include "UITextStyle.h"
#include "UITextStyleManager.h"
#include "UITextbox.h"
#include "UITextboxStyle.h"
#include "UITooltipStyle.h"
#include "UITreeView.h"
#include "UITreeViewStyle.h"
#include "UITypes.h" 
#include "UIUnknown.h"
#include "UIUtils.h"
#include "UIVersion.h"
#include "UIVolumePage.h"
#include "objectinspector.h"
#include "resource.h"

#define INITGUID
#include "ddraw.h"
#include "d3d.h"
#undef INITGUID

#include <shlwapi.h>
#include <commctrl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <windowsx.h>

#include <list>


// This warning actually wants us to make code ... less readable...
//struct S { enum E { a }; };
//int i = S::E::a;   // C4482
//int j = S::a;   // OK
#pragma warning( disable : 4482 )


//----------------------------------------------------------------------
//-- this is a SWG hack to work with our modified STLPORT

enum MemoryManagerNotALeak
{
	MM_notALeak
};

void * __cdecl operator new(size_t size, MemoryManagerNotALeak)
{
	return operator new(size);
}

//----------------------------------------------------------------------

const char               *gApplicationName     = "UIBuilder";
const char               *gNotImplemented      = "This function is not yet implemented";
const char               *gDefaultExtension    = "ui";
const char               *gFileFilter          = "User Interface Files (.ui)\0ui_*.ui\0";
const char               *gIncludeFileFilter   = "User Interface Include Files (.inc)\0ui_*.inc\0";
bool                      g_showShaders = false;

HWND                      gObjectTree          = 0;
HWND                      gTooltip             = 0;
HWND                      gMainWindow          = 0;
HHOOK                     gTooltipHook         = 0;

extern HANDLE             gFrameRenderingMutex;

UIDirect3DPrimaryCanvas  *gPrimaryDisplay      = 0;
ObjectInspector          *gObjectInspector     = 0;
DefaultObjectPropertiesManager gDefaultObjectPropertiesManager;
bool                      gVersionFilename     = false;

UISize                    gOldSize(0,0);
UISize                    gInitialSize(0,0);
UINarrowString            gFilename;
UINarrowString            gInitialDirectory;

UIBaseObject::UISmartObjectList              gClipboard;

typedef std::list<UINarrowString> MRUList;
MRUList                   gMRUList;
const int                 gMRUSize             = 8;

extern HWND               gUIWindow;
extern HWND               gTabControlWindow;
extern bool               gLimitFrameRate;
extern bool               gInVisualEditingMode;
extern long               gTriangleCount;
extern long               gFlushCount;
extern long               gFrameCount;
extern UIPage            *gCurrentlySelectedPage;
extern UIBaseObject::UISmartObjectList       gCurrentSelection;

// Selection highligt variables
extern bool               gDrawHighlightRect;
extern UIColor            gHighlightOutlineColor;
extern UIColor            gHighlightFillColor;

// Grid variables       
extern bool               gDrawGrid;
extern bool               gSnapToGrid;
extern unsigned long      gXGridStep;
extern unsigned long      gYGridStep;
extern unsigned long      gGridMajorTicks;

extern bool               gDrawCursor;

BOOL CALLBACK UIWindowDlgProc( HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam );
BOOL CALLBACK GridSettingsDlgProc( HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam );
BOOL CALLBACK HighlightSettingsDlgProc( HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam );
BOOL CALLBACK ModificationLogDlgProc( HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam );
BOOL CALLBACK ImportImageDlgProc( HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam );
void          SizeWindowToCurrentPageSelection( void );
void          SetSelection( UIBaseObject *NewSelection, bool pushHistory );
bool          RemoveFromSelection( const UIBaseObject *NewSelection);
void          AddToSelection( UIBaseObject *ObjectToAdd, bool pushHistory );

HTREEITEM GetObjectInTreeControlFromHandle( HWND hTree, HTREEITEM hParentItem, UIBaseObject *o );

bool RenameSelectedObjectInTree( HWND hTree, const char * newName );
bool CloseWorkspaceFile( bool bLock = true );


// Undo stuff.
void recordUndo(UIBaseObject * const object, bool force = false);
void recordUndo(UIBaseObject * const object, UILowerString property, UIString oldValue, UIString newValue, bool force = false);
void undo();
void clearUndoHistory();
extern bool s_UndoReady;
extern bool volatile s_UndoQueueEmpty;
HMENU gMenu = NULL;

struct TreeItem
{
	HTREEITEM mItem;
	HTREEITEM mParent;
	HTREEITEM mSibling;
};

typedef std::map<UIBaseObject * /*obj*/, TreeItem>  TreeItemMap;
TreeItemMap s_TreeItemMap;

void addItemInfo(UIBaseObject * RootObject, HTREEITEM hItem, HTREEITEM hParent, HTREEITEM hSibling)
{
	TreeItem  newItem;

	newItem.mItem = hItem;
	newItem.mParent = hParent;
	newItem.mSibling = hSibling;

	s_TreeItemMap[ RootObject ] = newItem;
}

TreeItem const * getItemInfo(UIBaseObject * RootObject)
{
	TreeItemMap::const_iterator itItem = s_TreeItemMap.find(RootObject);

	if (itItem != s_TreeItemMap.end()) 
	{
		return &(itItem->second);
	}

	return NULL;
}


namespace
{
	bool s_selectionPushOk = true;

	typedef std::map<std::string, int> FileTimeMap;
	FileTimeMap s_fileTimes;

	WNDPROC mOldLabelEditWindowProc;

}

void CheckOutSelectedFile();

//-----------------------------------------------------------------

class UIBuilderLoader : public UILoader
{
public:
							 UIBuilderLoader( void ){ ReadOnlyFilesInInput = false; };

	virtual bool LoadStringFromResource( const UINarrowString &ResourceName, UINarrowString &Out );

	bool         ReadOnlyFilesInInput;
};

//-----------------------------------------------------------------

bool UIBuilderLoader::LoadStringFromResource( const UINarrowString &ResourceName, UINarrowString &Out )
{
	FILE *fp = fopen( ResourceName.c_str(), "rb+" );

	if( !fp )
	{
		fp = fopen( ResourceName.c_str(), "rb" );

		if( fp )
			ReadOnlyFilesInInput = true;
		else
			return false;
	}
	
	struct _stat statbuf;
	const int result = _stat (ResourceName.c_str (), &statbuf);
	if (!result)
	{
		const int t = static_cast<int>(statbuf.st_mtime);
		s_fileTimes [ResourceName] = t;
	}

	fseek( fp, 0, SEEK_END );
	long len = ftell( fp );
	fseek( fp, 0, SEEK_SET );
	
	char *rawdata = new char[len];
	fread( rawdata, len, 1, fp );
	fclose( fp );

	Out.assign( rawdata, len );
	delete rawdata;

	return true;
}

//-----------------------------------------------------------------

void SetMainWindowTitle( void )
{
	if( gUIWindow )
	{
		char				buffer[_MAX_PATH+1];	
		const char *filename = 0;

		if( !gFilename.empty() )
			filename = strrchr( gFilename.c_str(), '\\' );

		if( filename )
			sprintf( buffer, "[%s]", filename + 1 );
		else
			sprintf( buffer, "[Untitled]" );
			
		SetWindowText( gUIWindow, buffer );
	}
}

//-----------------------------------------------------------------

void AddTooltipFromControlID( HWND TooltipWindow, HWND ParentWindow, UINT ControlID, char *Tooltip )
{
	TOOLINFO ti			  = {sizeof(ti)};
	HWND		 hControl = GetDlgItem( ParentWindow, ControlID );

	ti.uFlags   = TTF_IDISHWND;
	ti.hwnd     = ParentWindow;
	ti.uId      = (UINT)hControl;	
	ti.hinst    = GetModuleHandle(0);
	ti.lpszText = Tooltip;
			
	SendMessage( TooltipWindow, TTM_ADDTOOL, 0, (LPARAM)&ti );
}


//-----------------------------------------------------------------

static const char * s_drawGridKey              = "drawGrid";
static const char * s_snapToGridKey            = "snapToGrid";
static const char * s_drawHighlightRectKey     = "drawHighlightRect";
static const char * s_highlightOutlineColorKey = "highlightOutlineColor";
static const char * s_highlightFillColorKey    = "highlightFillColor";
static const char * s_xGridStepKey             = "xGridStep";
static const char * s_yGridStepKey             = "yGridStep";
static const char * s_gridMajorTicks           = "gridMajorTicks";
static const char * s_inVisualEditingMode      = "inVisualEditingMode";
static const char * s_showShadersKey           = "showShaders";
static const char * s_regKey                   = "Software\\Sony Online Entertainment\\UIBuilder\\";
static const char * s_regKeyPreferences        = "Software\\Sony Online Entertainment\\UIBuilder\\Preferences";
static const char * s_regKeyMRU                = "Software\\Sony Online Entertainment\\UIBuilder\\MRUList";

//-----------------------------------------------------------------

void LoadPreferencesFromRegistry( void )
{
	HKEY		hKey = 0;
	HRESULT hr;

	hr = RegOpenKeyEx( HKEY_CURRENT_USER, s_regKeyPreferences, 0, KEY_ALL_ACCESS, &hKey );

	if( hr == ERROR_SUCCESS )
	{
		//-- first load all numerics
		DWORD dwThisIsLame;
		DWORD dwMicrosoftSucks = sizeof (DWORD);

		if ((hr = RegQueryValueEx( hKey, s_drawGridKey, 0, 0, reinterpret_cast<BYTE *>(&dwThisIsLame), &dwMicrosoftSucks)) == ERROR_SUCCESS)
			gDrawGrid = dwThisIsLame != 0;

		if ((hr = RegQueryValueEx( hKey, s_snapToGridKey, 0, 0, reinterpret_cast<BYTE *>(&dwThisIsLame), &dwMicrosoftSucks)) == ERROR_SUCCESS)
			gSnapToGrid = dwThisIsLame != 0;
		
		if ((hr = RegQueryValueEx( hKey, s_xGridStepKey, 0, 0, reinterpret_cast<BYTE *>(&dwThisIsLame), &dwMicrosoftSucks)) == ERROR_SUCCESS)
			gXGridStep = dwThisIsLame;

		if ((hr = RegQueryValueEx( hKey, s_yGridStepKey, 0, 0, reinterpret_cast<BYTE *>(&dwThisIsLame), &dwMicrosoftSucks)) == ERROR_SUCCESS)
			gYGridStep = dwThisIsLame;

		if ((hr = RegQueryValueEx( hKey, s_gridMajorTicks, 0, 0, reinterpret_cast<BYTE *>(&dwThisIsLame), &dwMicrosoftSucks)) == ERROR_SUCCESS)
			gGridMajorTicks = dwThisIsLame;

		if ((hr = RegQueryValueEx( hKey, s_drawHighlightRectKey, 0, 0, reinterpret_cast<BYTE *>(&dwThisIsLame), &dwMicrosoftSucks)) == ERROR_SUCCESS)
			gDrawHighlightRect = dwThisIsLame != 0;

		if ((hr = RegQueryValueEx( hKey, s_inVisualEditingMode, 0, 0, reinterpret_cast<BYTE *>(&dwThisIsLame), &dwMicrosoftSucks)) == ERROR_SUCCESS)
			gInVisualEditingMode = dwThisIsLame != 0;

		if ((hr = RegQueryValueEx( hKey, s_showShadersKey , 0, 0, reinterpret_cast<BYTE *>(&dwThisIsLame), &dwMicrosoftSucks)) == ERROR_SUCCESS)
			g_showShaders = dwThisIsLame != 0;

		//-- now load all strings
		char  buf[_MAX_PATH + 1];
		dwMicrosoftSucks = sizeof( buf );
		if ((hr = RegQueryValueEx( hKey, s_highlightOutlineColorKey, 0, 0, reinterpret_cast<BYTE *>(buf), &dwMicrosoftSucks)) == ERROR_SUCCESS)
		{
			UIUtils::ParseColorARGB (buf, gHighlightOutlineColor);
		}

		dwMicrosoftSucks = sizeof( buf );
		if ((hr = RegQueryValueEx( hKey, s_highlightFillColorKey, 0, 0, reinterpret_cast<BYTE *>(buf), &dwMicrosoftSucks)) == ERROR_SUCCESS)
		{
			UIUtils::ParseColorARGB (buf, gHighlightFillColor);
		}

		RegCloseKey( hKey );
	}
}

//-----------------------------------------------------------------

void SavePreferencesToRegistry( void )
{
	HKEY		hKey = 0;
	DWORD		dwAction;
	HRESULT hr;

	hr = RegCreateKeyEx( HKEY_CURRENT_USER, s_regKeyPreferences, 0, "", REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, 0, &hKey, &dwAction );

	if( hr == ERROR_SUCCESS )
	{
		//-- first save all numerics
		DWORD dwThisIsLame;
		dwThisIsLame = gDrawGrid;
		RegSetValueEx( hKey, s_drawGridKey, 0, REG_DWORD, reinterpret_cast<const BYTE*>(&dwThisIsLame), sizeof (DWORD));
		
		dwThisIsLame = gSnapToGrid;
		RegSetValueEx( hKey, s_snapToGridKey, 0, REG_DWORD, reinterpret_cast<const BYTE*>(&dwThisIsLame), sizeof (DWORD));

		dwThisIsLame = gXGridStep;
		RegSetValueEx( hKey, s_xGridStepKey, 0, REG_DWORD, reinterpret_cast<const BYTE*>(&dwThisIsLame), sizeof (DWORD));

		dwThisIsLame = gYGridStep;
		RegSetValueEx( hKey, s_yGridStepKey, 0, REG_DWORD, reinterpret_cast<const BYTE*>(&dwThisIsLame), sizeof (DWORD));
				
		dwThisIsLame = gGridMajorTicks;
		RegSetValueEx( hKey, s_gridMajorTicks, 0, REG_DWORD, reinterpret_cast<const BYTE*>(&dwThisIsLame), sizeof (DWORD));
				
		dwThisIsLame = gDrawHighlightRect;
		RegSetValueEx( hKey, s_drawHighlightRectKey, 0, REG_DWORD, reinterpret_cast<const BYTE*>(&dwThisIsLame), sizeof (DWORD));

		dwThisIsLame = gInVisualEditingMode;
		RegSetValueEx( hKey, s_inVisualEditingMode, 0, REG_DWORD, reinterpret_cast<const BYTE*>(&dwThisIsLame), sizeof (DWORD));

		dwThisIsLame = g_showShaders;
		RegSetValueEx( hKey, s_showShadersKey, 0, REG_DWORD, reinterpret_cast<const BYTE*>(&dwThisIsLame), sizeof (DWORD));

		//-- now save all strings
		UINarrowString colorResult;

		UIUtils::FormatColorARGB (colorResult, gHighlightOutlineColor);
		RegSetValueEx( hKey, s_highlightOutlineColorKey, 0, REG_SZ, reinterpret_cast<const BYTE*>(colorResult.c_str()), colorResult.length ());

		UIUtils::FormatColorARGB (colorResult, gHighlightFillColor);
		RegSetValueEx( hKey, s_highlightFillColorKey, 0, REG_SZ, reinterpret_cast<const BYTE*>(colorResult.c_str()), colorResult.length ());

		RegCloseKey( hKey );
	}
}
//-----------------------------------------------------------------

void LoadMRUListFromRegistry( void )
{
	HKEY		hKey = 0;
	HRESULT hr;

	hr = RegOpenKeyEx( HKEY_CURRENT_USER, s_regKeyMRU, 0, KEY_ALL_ACCESS, &hKey );

	if( hr == ERROR_SUCCESS )
	{
		int CurrentFileID = 0;

		for (;;)
		{
			char  PathBuffer[_MAX_PATH + 1];
			DWORD dwPathBufferSize = sizeof( PathBuffer );
			char  Buffer[128];

			sprintf( Buffer, "MRU%d", CurrentFileID );

			hr = RegQueryValueEx( hKey, Buffer, 0, 0, (LPBYTE)PathBuffer, &dwPathBufferSize );

			if( hr != ERROR_SUCCESS )
				break;

			gMRUList.push_back( PathBuffer );

			++CurrentFileID;
		}

		RegCloseKey( hKey );
	}
}

void SaveMRUListToRegistry( void )
{
	HKEY		hKey = 0;
	DWORD		dwAction;
	HRESULT hr;

	hr = RegCreateKeyEx( HKEY_CURRENT_USER, s_regKeyMRU, 0, "", REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, 0, &hKey, &dwAction );

	if( hr == ERROR_SUCCESS )
	{
		int CurrentFileID = 0;

		for (;;)
		{
			char Buffer[128];
			sprintf( Buffer, "MRU%d", CurrentFileID );

			hr = RegDeleteValue( hKey, Buffer );

			if( hr != ERROR_SUCCESS )
				break;

			++CurrentFileID;
		}

		CurrentFileID = 0;

		for( MRUList::iterator i = gMRUList.begin(); i != gMRUList.end(); ++i )
		{
			char Buffer[128];
			sprintf( Buffer, "MRU%d", CurrentFileID );

			RegSetValueEx( hKey, Buffer, 0, REG_SZ, (LPBYTE)i->c_str(), i->size() );
			++CurrentFileID;
		}

		RegCloseKey( hKey );
	}
}

//-----------------------------------------------------------------

void AddToMRUList( const char *Filename )
{
	for( MRUList::iterator i = gMRUList.begin(); i != gMRUList.end(); ++i )
	{
		if( !_stricmp( i->c_str(), Filename ) )
		{
			gMRUList.erase( i );
			break;
		}
	}

	gMRUList.push_front( UINarrowString( Filename ) );

	while( gMRUList.size() > gMRUSize )
		gMRUList.pop_back();
}

//-----------------------------------------------------------------

void LoadMRUListToMenu( HMENU hMenu )
{
	HMENU hFileMenu = GetSubMenu( hMenu, 0 );
	HMENU hMRUList  = 0;
	int		nItems		= GetMenuItemCount( hFileMenu );	

	for( int i = 0; i < nItems; ++i )
	{
		char         Buffer[256];
		MENUITEMINFO mii;

		mii.cbSize		 = sizeof( mii );
		mii.fMask			 = MIIM_TYPE;
		mii.dwTypeData = Buffer;
		mii.cch        = sizeof( Buffer );

		GetMenuItemInfo( hFileMenu, i, TRUE, &mii );

		if( !_stricmp( Buffer, "Recent &Files" ) )
		{
			hMRUList = GetSubMenu( hFileMenu, i );
			break;
		}
	}

	if( hMRUList )
	{
		int ItemCount = 1;

		while( GetMenuItemCount( hMRUList ) > 0 )
			DeleteMenu( hMRUList, 0, MF_BYPOSITION );

		if( gMRUList.empty() )
			AppendMenu( hMRUList, MF_GRAYED | MF_STRING, 0xFFFF, "(Empty)" );
		else
		{
			for( MRUList::iterator i = gMRUList.begin(); i != gMRUList.end(); ++i )
			{
				AppendMenu( hMRUList, MF_ENABLED | MF_STRING, 0xFFFF - ItemCount, i->c_str() );
				++ItemCount;
			}
		}
	}

	DrawMenuBar( gMainWindow );
}

//-----------------------------------------------------------------

void EnableMenuItems( HMENU hMenu, UIBaseObject *SelectedObject )
{
	UINT NewState;

	if( SelectedObject )
		NewState = MF_BYCOMMAND | MF_ENABLED;
	else
		NewState = MF_BYCOMMAND | MF_GRAYED;

	EnableMenuItem( hMenu, ID_INSERT_BUTTON           , NewState );
	EnableMenuItem( hMenu, ID_INSERT_CHECKBIOX        , NewState );
	EnableMenuItem( hMenu, ID_INSERT_DATASOURCE       , NewState );
	EnableMenuItem( hMenu, ID_INSERT_DATASOURCECONTAINER   , NewState );
	EnableMenuItem( hMenu, ID_INSERT_DATAITEM	      , NewState );
	EnableMenuItem( hMenu, ID_INSERT_DROPDOWNBOX      , NewState );
	EnableMenuItem( hMenu, ID_INSERT_ELLIPSE		  , NewState );	
	EnableMenuItem( hMenu, ID_INSERT_GRID             , NewState );
	EnableMenuItem( hMenu, ID_INSERT_IMAGE            , NewState );
	EnableMenuItem( hMenu, ID_INSERT_IMAGEFRAME       , NewState );
	EnableMenuItem( hMenu, ID_INSERT_LISTBOX          , NewState );
	EnableMenuItem( hMenu, ID_INSERT_LIST             , NewState );
	EnableMenuItem( hMenu, ID_INSERT_TREEVIEW         , NewState );
	EnableMenuItem( hMenu, ID_INSERT_COMBOBOX         , NewState );
	EnableMenuItem( hMenu, ID_INSERT_NAMESPACE        , NewState );
	EnableMenuItem( hMenu, ID_INSERT_TEXTBOX          , NewState );
	EnableMenuItem( hMenu, ID_INSERT_SCROLLBAR        , NewState );
	EnableMenuItem( hMenu, ID_INSERT_SLIDERBAR        , NewState );
	EnableMenuItem( hMenu, ID_INSERT_SLIDERPLANE      , NewState );
	EnableMenuItem( hMenu, ID_INSERT_TABSET           , NewState );
	EnableMenuItem( hMenu, ID_INSERT_TABBEDPANE       , NewState );
	EnableMenuItem( hMenu, ID_INSERT_POPUPMENU        , NewState );
	EnableMenuItem( hMenu, ID_INSERT_RADIALMENU       , NewState );
	EnableMenuItem( hMenu, ID_INSERT_VOLUMEPAGE       , NewState );
	EnableMenuItem( hMenu, ID_INSERT_CURSORSET        , NewState );
	EnableMenuItem( hMenu, ID_INSERT_TABLE            , NewState );
	EnableMenuItem( hMenu, ID_INSERT_TABLEMODELDEFAULT, NewState );
	EnableMenuItem( hMenu, ID_INSERT_TABLEHEADER,       NewState );

	EnableMenuItem( hMenu, ID_INSERT_STATICTEXT       , NewState );	
	EnableMenuItem( hMenu, ID_INSERT_TEMPLATE         , NewState );	
	EnableMenuItem( hMenu, ID_INSERT_PAGE             , NewState );
	EnableMenuItem( hMenu, ID_INSERT_COMPOSITE        , NewState );
	EnableMenuItem( hMenu, ID_INSERT_PROGRESSBAR      , NewState );
	EnableMenuItem( hMenu, ID_INSERT_CURSOR			  , NewState );
	EnableMenuItem( hMenu, ID_INSERT_CLOCK			  , NewState );
	EnableMenuItem( hMenu, ID_INSERT_APPLICATIONOBJECT, NewState );	

	EnableMenuItem( hMenu, ID_INSERT_STYLE_BUTTON     , NewState );
	EnableMenuItem( hMenu, ID_INSERT_STYLE_CHECKBOX   , NewState );
	EnableMenuItem( hMenu, ID_INSERT_STYLE_DROPDOWNBOX, NewState );
	EnableMenuItem( hMenu, ID_INSERT_STYLE_GRID		  , NewState );	
	EnableMenuItem( hMenu, ID_INSERT_STYLE_IMAGE      , NewState );
	EnableMenuItem( hMenu, ID_INSERT_STYLE_LISTBOX    , NewState );
	EnableMenuItem( hMenu, ID_INSERT_STYLE_LIST       , NewState );
	EnableMenuItem( hMenu, ID_INSERT_STYLE_TREEVIEW   , NewState );
	EnableMenuItem( hMenu, ID_INSERT_STYLE_PROGRESSBAR, NewState );	
	EnableMenuItem( hMenu, ID_INSERT_STYLE_SLIDERBAR	, NewState );	
	EnableMenuItem( hMenu, ID_INSERT_STYLE_TEXT       , NewState );
	EnableMenuItem( hMenu, ID_INSERT_STYLE_TEXTBOX    , NewState );	
	EnableMenuItem( hMenu, ID_INSERT_STYLE_TOOLTIP		, NewState );
	EnableMenuItem( hMenu, ID_INSERT_STYLE_RECTANGLE	, NewState );
	EnableMenuItem( hMenu, ID_INSERT_STYLE_TABSET		, NewState );
	EnableMenuItem( hMenu, ID_INSERT_STYLE_TABBEDPANE   , NewState );
	EnableMenuItem( hMenu, ID_INSERT_STYLE_POPUPMENU    , NewState );
	EnableMenuItem( hMenu, ID_INSERT_STYLE_RADIALMENU   , NewState );

	EnableMenuItem( hMenu, ID_INSERT_EFFECTOR_COLOR    , NewState );
	EnableMenuItem( hMenu, ID_INSERT_EFFECTOR_ROTATION , NewState );
	EnableMenuItem( hMenu, ID_INSERT_EFFECTOR_LOCATION , NewState );
	EnableMenuItem( hMenu, ID_INSERT_EFFECTOR_OPACITY  , NewState );
	EnableMenuItem( hMenu, ID_INSERT_EFFECTOR_SIZE     , NewState );	
	EnableMenuItem( hMenu, ID_INSERT_DEFORMER_HUD     , NewState );	
	EnableMenuItem( hMenu, ID_INSERT_DEFORMER_WAVE     , NewState );	
	EnableMenuItem( hMenu, ID_INSERT_DEFORMER_ROTATE   , NewState );	

	EnableMenuItem( hMenu, ID_FILE_IMPORT, NewState );

	if( gClipboard.empty() && (GetFocus() != gObjectInspector->GetTextControl()) )
		EnableMenuItem( hMenu, ID_EDIT_PASTE, MF_BYCOMMAND | MF_GRAYED );
	else
		EnableMenuItem( hMenu, ID_EDIT_PASTE, MF_BYCOMMAND | MF_ENABLED );

	if(!SelectedObject || SelectedObject->GetChildCount () == 0)
	{
		EnableMenuItem( hMenu, ID_EDIT_SORT, MF_BYCOMMAND | MF_GRAYED );
	}
	else
	{
		EnableMenuItem( hMenu, ID_EDIT_SORT, MF_BYCOMMAND | MF_ENABLED );
	}


	UIPage *RootPage = UIManager::gUIManager().GetRootPage();

	if( !SelectedObject || SelectedObject == RootPage )
	{
		EnableMenuItem( hMenu, ID_EDIT_DELETE, MF_BYCOMMAND | MF_GRAYED );
		EnableMenuItem( hMenu, ID_EDIT_DUPLICATE, MF_BYCOMMAND | MF_GRAYED );
		EnableMenuItem( hMenu, ID_FILE_EXPORT, MF_BYCOMMAND | MF_GRAYED );
		EnableMenuItem( hMenu, ID_EDIT_CUT, MF_BYCOMMAND | MF_GRAYED );
		EnableMenuItem( hMenu, ID_EDIT_COPY, MF_BYCOMMAND | MF_GRAYED );
	}
	else
	{
		EnableMenuItem( hMenu, ID_EDIT_DELETE, MF_BYCOMMAND | MF_ENABLED );
		EnableMenuItem( hMenu, ID_EDIT_DUPLICATE, MF_BYCOMMAND | MF_ENABLED );
		EnableMenuItem( hMenu, ID_FILE_EXPORT, MF_BYCOMMAND | MF_ENABLED );
		EnableMenuItem( hMenu, ID_EDIT_CUT, MF_BYCOMMAND | MF_ENABLED );
		EnableMenuItem( hMenu, ID_EDIT_COPY, MF_BYCOMMAND | MF_ENABLED );
	}

	if( RootPage )
	{
		EnableMenuItem( hMenu, ID_FILE_CLOSE, MF_BYCOMMAND | MF_ENABLED );
		EnableMenuItem( hMenu, ID_FILE_SAVE, MF_BYCOMMAND | MF_ENABLED );
		EnableMenuItem( hMenu, ID_FILE_SAVEAS, MF_BYCOMMAND | MF_ENABLED );

		EnableMenuItem( hMenu, ID_VIEW_RUN, MF_BYCOMMAND | MF_ENABLED );
		EnableMenuItem( hMenu, ID_VIEW_CURSOR, MF_BYCOMMAND | MF_ENABLED );
		EnableMenuItem( hMenu, ID_VIEW_HIGHLIGHT, MF_BYCOMMAND | MF_ENABLED );
		EnableMenuItem( hMenu, ID_VIEW_GRID, MF_BYCOMMAND | MF_ENABLED );
		EnableMenuItem( hMenu, ID_VIEW_SHOWSHADERS, MF_BYCOMMAND | MF_ENABLED );
	}
	else
	{
		EnableMenuItem( hMenu, ID_FILE_CLOSE, MF_BYCOMMAND | MF_GRAYED );
		EnableMenuItem( hMenu, ID_FILE_SAVE, MF_BYCOMMAND | MF_GRAYED );
		EnableMenuItem( hMenu, ID_FILE_SAVEAS, MF_BYCOMMAND | MF_GRAYED );

		EnableMenuItem( hMenu, ID_VIEW_RUN,         MF_BYCOMMAND | MF_GRAYED );
		EnableMenuItem( hMenu, ID_VIEW_CURSOR,      MF_BYCOMMAND | MF_GRAYED );
		EnableMenuItem( hMenu, ID_VIEW_HIGHLIGHT,   MF_BYCOMMAND | MF_GRAYED );
		EnableMenuItem( hMenu, ID_VIEW_GRID,        MF_BYCOMMAND | MF_GRAYED );
		EnableMenuItem( hMenu, ID_VIEW_SHOWSHADERS, MF_BYCOMMAND | MF_GRAYED );
	}

	if( gInVisualEditingMode )
		CheckMenuItem( hMenu, ID_VIEW_RUN, MF_BYCOMMAND | MF_UNCHECKED );
	else
		CheckMenuItem( hMenu, ID_VIEW_RUN, MF_BYCOMMAND | MF_CHECKED );

	if( gDrawCursor )
		CheckMenuItem( hMenu, ID_VIEW_CURSOR, MF_BYCOMMAND | MF_CHECKED );
	else
		CheckMenuItem( hMenu, ID_VIEW_CURSOR, MF_BYCOMMAND | MF_UNCHECKED );

	if( gDrawHighlightRect )
		CheckMenuItem( hMenu, ID_VIEW_HIGHLIGHT, MF_BYCOMMAND | MF_CHECKED );
	else
		CheckMenuItem( hMenu, ID_VIEW_HIGHLIGHT, MF_BYCOMMAND | MF_UNCHECKED );

	if( gDrawGrid )
		CheckMenuItem( hMenu, ID_VIEW_GRID, MF_BYCOMMAND | MF_CHECKED );
	else
		CheckMenuItem( hMenu, ID_VIEW_GRID, MF_BYCOMMAND | MF_UNCHECKED );

	WaitForSingleObject( gFrameRenderingMutex, INFINITE );
	g_showShaders = gPrimaryDisplay->GetShowShaders();
	ReleaseMutex( gFrameRenderingMutex );
	
	if( g_showShaders )
		CheckMenuItem( hMenu, ID_VIEW_SHOWSHADERS, MF_BYCOMMAND | MF_CHECKED );
	else
		CheckMenuItem( hMenu, ID_VIEW_SHOWSHADERS, MF_BYCOMMAND | MF_UNCHECKED );
}

//-----------------------------------------------------------------

void EnableHistoryButtons ()
{
	EnableWindow( GetDlgItem( gMainWindow, IDC_BUTTON_BACK ),    UIBuilderHistory::backValid ());
	EnableWindow( GetDlgItem( gMainWindow, IDC_BUTTON_FORWARD ), UIBuilderHistory::forwardValid ());
	EnableWindow( GetDlgItem( gMainWindow, IDC_BUTTON_BACK ),    true);
}

//-----------------------------------------------------------------

void EnableButtonControls( HWND mParent, UIBaseObject *SelectedObject )
{
	BOOL bEnableUp     = FALSE;
	BOOL bEnableTop    = FALSE;
	BOOL bEnableDown   = FALSE;
	BOOL bEnableBottom = FALSE;

	if( SelectedObject )
	{
		UIBaseObject *ObjectParent = SelectedObject->GetParent();

		if( ObjectParent )
		{
			if( ObjectParent->CanChildMove( SelectedObject, UIBaseObject::ChildMovementDirection::Up ) )
				bEnableUp = TRUE;
			if( ObjectParent->CanChildMove( SelectedObject, UIBaseObject::ChildMovementDirection::Top ) )
				bEnableTop = TRUE;
			if( ObjectParent->CanChildMove( SelectedObject, UIBaseObject::ChildMovementDirection::Down ) )
				bEnableDown = TRUE;
			if( ObjectParent->CanChildMove( SelectedObject, UIBaseObject::ChildMovementDirection::Bottom ) )
				bEnableBottom = TRUE;
		}
	}

	EnableWindow( GetDlgItem( mParent, IDC_UP ),             bEnableUp );
	EnableWindow( GetDlgItem( mParent, IDC_TOP ),            bEnableTop );
	EnableWindow( GetDlgItem( mParent, IDC_DOWN ),           bEnableDown );
	EnableWindow( GetDlgItem( mParent, IDC_BOTTOM ),         bEnableBottom );

	if(gCurrentSelection.size() == 2)
	{
		EnableMenuItem(gMenu, ID_EDIT_DIFF, MF_BYCOMMAND | MF_ENABLED);
	}
	else
	{
		EnableMenuItem(gMenu, ID_EDIT_DIFF, MF_BYCOMMAND | MF_GRAYED);
	}
}

//-----------------------------------------------------------------

void ClearDefPushButtonLook( HWND hwndDlg, UINT nControlID )
{
	SetWindowLong( GetDlgItem( hwndDlg, nControlID ), GWL_STYLE, 
		GetWindowLong( GetDlgItem( hwndDlg, nControlID ), GWL_STYLE ) & ~BS_DEFPUSHBUTTON );
}

//-----------------------------------------------------------------

LRESULT CALLBACK LabelEditWindowProc( HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam )
{
	if( (uMsg == WM_CHAR) && (wParam == VK_RETURN))
	{
		TreeView_EndEditLabelNow(gObjectTree, false);
		return false;
	}	
	else if( (uMsg == WM_KEYUP) && (wParam == VK_RETURN))
	{
		TreeView_EndEditLabelNow(gObjectTree, false);
		return false;
	}
	return CallWindowProc( mOldLabelEditWindowProc, hwnd, uMsg, wParam, lParam );
}

//-----------------------------------------------------------------

typedef struct
{
	UINT	 SortCommand;
	LPARAM hItemToSort;
	LPARAM hReferenceItem;	
} TreeSortParams;

//-----------------------------------------------------------------

int CALLBACK TreeSortCompareFunc(LPARAM lParam1, LPARAM lParam2, LPARAM lParamSort)
{
	TreeSortParams *p = (TreeSortParams *)lParamSort;

	switch( p->SortCommand )
	{
		case IDC_UP:
			if( (lParam1 == p->hItemToSort) && (lParam2 == p->hReferenceItem) )
				return -1;
			else if( (lParam2 == p->hItemToSort) && (lParam1 == p->hReferenceItem) )
				return  1;
			else
				return 0;

		case IDC_TOP:
			if( lParam1 == p->hItemToSort )
				return -1;
			else if( lParam2 == p->hItemToSort )
				return  1;
			else
				return 0;

		case IDC_DOWN:
			if( (lParam1 == p->hItemToSort) && (lParam2 == p->hReferenceItem) )
				return  1;
			else if( (lParam2 == p->hItemToSort) && (lParam1 == p->hReferenceItem) )
				return -1;
			else
				return 0;

		case IDC_BOTTOM:
			if( lParam1 == p->hItemToSort )
				return  1;
			else if( lParam2 == p->hItemToSort )
				return -1;
			else
				return 0;

		default:
			return 0;
	}
}

//-----------------------------------------------------------------

void ProcessReorderButtonEvent( HWND hTree, HTREEITEM hSelection, UINT Command )
{
	if( hSelection )
	{
		UIBaseObject *SelectedObject;
		UIBaseObject *ParentObject;
		TVITEM        tvi;

		tvi.mask  = TVIF_HANDLE | TVIF_PARAM;
		tvi.hItem = hSelection;
		TreeView_GetItem( hTree, &tvi );

		if( !tvi.lParam )
			return;
		
		WaitForSingleObject( gFrameRenderingMutex, INFINITE );

		SelectedObject = reinterpret_cast<UIBaseObject *>( tvi.lParam );		
		ParentObject   = SelectedObject->GetParent();		

		if( ParentObject )
		{
			bool					 ReSortNeeded = false;
			TreeSortParams tsp;

			tsp.SortCommand = Command;
			tsp.hItemToSort = tvi.lParam;

			if( Command == IDC_UP )
			{
				if( ParentObject->MoveChild( SelectedObject, UIBaseObject::ChildMovementDirection::Up ) )
				{
					tvi.hItem = TreeView_GetPrevSibling( hTree, hSelection );
					TreeView_GetItem( hTree, &tvi );
					tsp.hReferenceItem = tvi.lParam;
					ReSortNeeded = true;
				}
			}
			else if( Command == IDC_TOP )
			{
				if( ParentObject->MoveChild( SelectedObject, UIBaseObject::ChildMovementDirection::Top ) )
					ReSortNeeded = true;
			}
			else if( Command == IDC_DOWN )
			{
				if( ParentObject->MoveChild( SelectedObject, UIBaseObject::ChildMovementDirection::Down ) )
				{
					tvi.hItem = TreeView_GetNextSibling( hTree, hSelection );
					TreeView_GetItem( hTree, &tvi );
					tsp.hReferenceItem = tvi.lParam;
					ReSortNeeded = true;
				}
			}
			else if( Command == IDC_BOTTOM )
			{
				if( ParentObject->MoveChild( SelectedObject, UIBaseObject::ChildMovementDirection::Bottom ) )
					ReSortNeeded = true;
			}

			if( ReSortNeeded )
			{
				TVSORTCB tvscb;
				
				tvscb.hParent			= TreeView_GetParent( hTree, hSelection );
				tvscb.lParam			= (LPARAM)&tsp;
				tvscb.lpfnCompare	= TreeSortCompareFunc;

				TreeView_SortChildrenCB( hTree, &tvscb, FALSE );			
				TreeView_EnsureVisible( hTree, hSelection );
			}			
			ReleaseMutex( gFrameRenderingMutex );
		}
	}
}

//-----------------------------------------------------------------

UIBaseObject *GetObjectFromNode( HTREEITEM hItem )
{
	TVITEM tvi;

	tvi.mask   = TVIF_HANDLE | TVIF_PARAM;
	tvi.hItem  = hItem;
	tvi.lParam = 0;

	TreeView_GetItem( gObjectTree, &tvi );
	return reinterpret_cast<UIBaseObject *>( tvi.lParam );
}

//-----------------------------------------------------------------

UIBaseObject *GetSelectedObjectInTreeControl( void )
{
	HTREEITEM			hSelection     = TreeView_GetSelection( gObjectTree );
	UIBaseObject *SelectedObject = 0;

	if( hSelection )
		SelectedObject = GetObjectFromNode( hSelection );

	return SelectedObject;
}

//-----------------------------------------------------------------

HTREEITEM LoadObjectStructureToTreeControl( HWND hTree, UIBaseObject * const RootObject, HTREEITEM hParent, HTREEITEM hInsertAfter)
{
	HTREEITEM hItem = 0;

	if( !hParent )
		SendMessage( hTree, WM_SETREDRAW, 0, 0 );

	if( !RootObject->IsA( TUICanvas ) )
	{
		TVINSERTSTRUCT tvis;		

		tvis.hParent                 = hParent;
		tvis.hInsertAfter            = hInsertAfter;
		tvis.item.mask               = TVIF_CHILDREN | TVIF_HANDLE | TVIF_PARAM | TVIF_TEXT | TVIF_IMAGE | TVIF_SELECTEDIMAGE;	
		tvis.item.pszText            = LPSTR_TEXTCALLBACK;
		tvis.item.iImage             = I_IMAGECALLBACK;
		tvis.item.iSelectedImage     = I_IMAGECALLBACK;
		tvis.item.cChildren          = I_CHILDRENCALLBACK;
		tvis.item.lParam             = (DWORD)RootObject;

		hItem = TreeView_InsertItem( hTree, &tvis );

		// Reverse lookup info.  RLS - move this to a ! property.
		HTREEITEM hSibling = TreeView_GetPrevSibling(hTree, hItem);
		if (!hSibling) 
		{
			hSibling = TreeView_GetNextSibling(hTree, hItem);
		}
		addItemInfo(RootObject, hItem, hParent, hSibling);

		UIBaseObject::UIObjectList ol;
		RootObject->GetChildren( ol );

		for( UIBaseObject::UIObjectList::const_iterator i = ol.begin(); i != ol.end(); ++i )
		{
			if ((*i)->IsA (TUIWidget))
				if (static_cast<UIWidget *>(*i)->IsTransient ())
					continue;

			LoadObjectStructureToTreeControl( hTree, *i, hItem, TVI_LAST );
		}

		if( !hParent)
			TreeView_Expand( hTree, hItem, TVE_EXPAND );
	}	

	if (!hParent)
	{
		SendMessage( hTree, WM_SETREDRAW, 1, 0 );
		InvalidateRect( hTree, 0, TRUE );
	}	

	return hItem;
}

//-----------------------------------------------------------------

void LoadTopLevelObjectsToTabControl( void )
{
	UIPage *RootPage = UIManager::gUIManager().GetRootPage();

	TabCtrl_DeleteAllItems( gTabControlWindow );
		
	if( RootPage )
	{
		int						TabIndex = 0;
		UIBaseObject::UIObjectList	TopLevelPages;

		RootPage->GetChildren( TopLevelPages );

		for( UIBaseObject::UIObjectList::iterator i = TopLevelPages.begin(); i != TopLevelPages.end(); ++i )
		{
			if( (*i)->IsA( TUIPage ) )
			{
				TCITEM	 tci;

				tci.mask   = TCIF_PARAM | TCIF_TEXT;
				tci.lParam = reinterpret_cast<DWORD>( *i );

				if( !tci.lParam )
					continue;

				const UINarrowString &Name = (*i)->GetName();
				// windows bug requires const_cast here
				tci.pszText = const_cast<char *>( Name.c_str() );
				TabCtrl_InsertItem( gTabControlWindow, TabIndex, &tci );
				TabIndex++;
			}
		}

		if( TabIndex > 0 )
		{
			TabCtrl_SetCurSel( gTabControlWindow, 0 );
			SizeWindowToCurrentPageSelection();
		}
	}
}

//-----------------------------------------------------------------

BOOL CALLBACK CreatePageDlgProc( HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam )
{
	UI_UNREF (lParam);

	switch( uMsg )
	{
		case WM_INITDIALOG:
			CheckRadioButton( hwndDlg, IDC_PARENT, IDC_NEWFILE, IDC_PARENT );
			EnableWindow( GetDlgItem( hwndDlg, IDC_PAGEFILE ), FALSE );
			return TRUE;

		case WM_CLOSE:
			// Equivalent to pressing cancel
			EndDialog( hwndDlg, 0 );
			return 0;

		case WM_COMMAND:
			if( LOWORD( wParam ) == IDOK )
			{
				UIPage *thePage = new UIPage;

				if( IsDlgButtonChecked( hwndDlg, IDC_NEWFILE ) )
				{
					long  BufferLength = SendDlgItemMessage( hwndDlg, IDC_PAGEFILE, WM_GETTEXTLENGTH, 0, 0 ) + 1;
					char *Buffer       = new char[BufferLength];

					GetDlgItemText( hwndDlg, IDC_PAGEFILE, Buffer, BufferLength );

					thePage->SetProperty( UIBaseObject::PropertyName::SourceFile, UIUnicode::narrowToWide( Buffer ) );
				}
				EndDialog( hwndDlg, (long)thePage );
			}
			else if( LOWORD( wParam ) == IDCANCEL )
				EndDialog( hwndDlg, 0 );
			else if( HIWORD( wParam ) == BN_CLICKED )
			{
				if( IsDlgButtonChecked( hwndDlg, IDC_NEWFILE ) )
					EnableWindow( GetDlgItem( hwndDlg, IDC_PAGEFILE ), TRUE );
				else
					EnableWindow( GetDlgItem( hwndDlg, IDC_PAGEFILE ), FALSE );
			}
			return 0;

		default:
			return 0;
	}
}

//-----------------------------------------------------------------

void InsertNewUIObject( int MenuID )
{
	UIPage        *RootPage  = UIManager::gUIManager().GetRootPage();
	UIBaseObject	*NewObject = 0;
		
	assert( RootPage );

	switch( MenuID )
	{
		case ID_INSERT_BUTTON:
			NewObject  = new UIButton;
			break;
		case ID_INSERT_CHECKBOX:
			NewObject = new UICheckbox;
			break;
		case ID_INSERT_DATASOURCE:
			NewObject = new UIDataSource;
			break;
		case ID_INSERT_DATASOURCECONTAINER:
			NewObject = new UIDataSourceContainer;
			break;
		case ID_INSERT_DATAITEM:
			NewObject = new UIData;
			break;
		case ID_INSERT_DROPDOWNBOX:
			NewObject = new UIDropdownbox;
			break;
		case ID_INSERT_ELLIPSE:
			NewObject = new UIEllipse;
			break;		
		case ID_INSERT_IMAGE:
			NewObject = new UIImage;
			break;
		case ID_INSERT_GRID:
			NewObject = new UIGrid;
			break;
		case ID_INSERT_IMAGEFRAME:
			NewObject = reinterpret_cast<UIBaseObject *>( DialogBox( GetModuleHandle(0),
				MAKEINTRESOURCE(IDD_IMPORTIMAGE),	gMainWindow, ImportImageDlgProc) );

			if( NewObject == reinterpret_cast<UIBaseObject *>( -1 ) )
				NewObject = 0;

			break;
		case ID_INSERT_LISTBOX:
			NewObject = new UIListbox;
			break;
		case ID_INSERT_LIST:
			NewObject = new UIList;
			break;
		case ID_INSERT_TREEVIEW:
			NewObject = new UITreeView;
			break;
		case ID_INSERT_PIE:
			NewObject = new UIPie;
			break;
		case ID_INSERT_RUNNER:
			NewObject = new UIRunner;
			break;
		case ID_INSERT_COMBOBOX:
			NewObject = new UIComboBox;
			break;
		case ID_INSERT_NAMESPACE:
			NewObject = new UINamespace;
			break;
		case ID_INSERT_TEXTBOX:
			NewObject = new UITextbox;
			break;
		case ID_INSERT_SCROLLBAR:
			NewObject = new UIScrollbar;
			break;
		case ID_INSERT_SLIDERBAR:
			NewObject = new UISliderbar;
			break;
		case ID_INSERT_SLIDERPLANE:
			NewObject = new UISliderplane;
			break;
		case ID_INSERT_TABSET:
			NewObject = new UITabSet;
			break;
		case ID_INSERT_TABBEDPANE:
			NewObject = new UITabbedPane;
			break;
		case ID_INSERT_POPUPMENU:
			NewObject = new UIPopupMenu;
			break;
		case ID_INSERT_RADIALMENU:
			NewObject = new UIRadialMenu;
			break;
		case ID_INSERT_CURSORSET:
			NewObject = new UICursorSet;
			break;
		case ID_INSERT_VOLUMEPAGE:
			NewObject = new UIVolumePage;
			break;
		case ID_INSERT_STATICTEXT:
			NewObject = new UIText;
			break;
		case ID_INSERT_TEMPLATE:
			NewObject = new UITemplate;
			break;
		case ID_INSERT_PAGE:
		{
			NewObject = reinterpret_cast<UIBaseObject *>( DialogBox( GetModuleHandle(0),
				MAKEINTRESOURCE(IDD_PAGEFILE), gMainWindow, CreatePageDlgProc) );

			if( NewObject == reinterpret_cast<UIBaseObject *>( -1 ) )
				NewObject = 0;

			break;
		}
		case ID_INSERT_COMPOSITE:
			NewObject = new UIComposite;
			break;
		case ID_INSERT_TABLE:
			NewObject = new UITable;
			break;
		case ID_INSERT_TABLEMODELDEFAULT:
			NewObject = new UITableModelDefault;
			break;
		case ID_INSERT_TABLEHEADER:
			NewObject = new UITableHeader;
			break;

		case ID_INSERT_PROGRESSBAR:
			NewObject = new UIProgressbar;
			break;
		case ID_INSERT_CURSOR:
			NewObject = new UICursor;
			break;
		case ID_INSERT_CLOCK:
			NewObject = new UIClock;
			break;
		case ID_INSERT_APPLICATIONOBJECT:
			NewObject = new UIUnknown;
			break;			
		case ID_INSERT_STYLE_BUTTON:
			NewObject = new UIButtonStyle;
			break;
		case ID_INSERT_STYLE_CHECKBOX:
			NewObject = new UICheckboxStyle;
			break;
		case ID_INSERT_STYLE_GRID:
			NewObject = new UIGridStyle;
			break;
		case ID_INSERT_STYLE_IMAGE:
			NewObject = new UIImageStyle;
			break;		
		case ID_INSERT_STYLE_LISTBOX:
			NewObject = new UIListboxStyle;
			break;
		case ID_INSERT_STYLE_LIST:
			NewObject = new UIListStyle;
			break;
		case ID_INSERT_STYLE_TREEVIEW:
			NewObject = new UITreeViewStyle;
			break;
		case ID_INSERT_STYLE_PIE:
			NewObject = new UIPieStyle;
			break;
		case ID_INSERT_STYLE_PALETTE:
			NewObject = new UIPalette;
			break;
		case ID_INSERT_STYLE_TEXTBOX:
			NewObject = new UITextboxStyle;
			break;
		case ID_INSERT_STYLE_TEXT:
			NewObject = new UITextStyle;
			break;
		case ID_INSERT_STYLE_PROGRESSBAR:
			NewObject = new UIProgressbarStyle;
			break;
		case ID_INSERT_STYLE_DROPDOWNBOX:
			NewObject = new UIDropdownboxStyle;
			break;
		case ID_INSERT_STYLE_SLIDERBAR:
			NewObject = new UISliderbarStyle;
			break;		
		case ID_INSERT_STYLE_TOOLTIP:
			NewObject = new UITooltipStyle;
			break;
		case ID_INSERT_STYLE_RECTANGLE:
			NewObject = new UIRectangleStyle;
			break;
		case ID_INSERT_STYLE_TABSET:
			NewObject = new UITabSetStyle;
			break;
		case ID_INSERT_STYLE_TABBEDPANE:
			NewObject = new UITabbedPaneStyle;
			break;
		case ID_INSERT_STYLE_POPUPMENU:
			NewObject = new UIPopupMenuStyle;
			break;
		case ID_INSERT_STYLE_RADIALMENU:
			NewObject = new UIRadialMenuStyle;
			break;
		
		case ID_INSERT_EFFECTOR_COLOR:
			NewObject = new UIColorEffector;
			break;

		case ID_INSERT_EFFECTOR_ROTATION:
			NewObject = new UIRotationEffector;
			break;

		case ID_INSERT_EFFECTOR_LOCATION:
			NewObject = new UILocationEffector;
			break;

		case ID_INSERT_EFFECTOR_OPACITY:
			NewObject = new UIOpacityEffector;
			break;

		case ID_INSERT_EFFECTOR_SIZE:
			NewObject = new UISizeEffector;
			break;

		case ID_INSERT_DEFORMER_HUD:
			NewObject = new UIDeformerHUD;
			break;

		case ID_INSERT_DEFORMER_WAVE:
			NewObject = new UIDeformerWave;
			break;

		case ID_INSERT_DEFORMER_ROTATE:
			NewObject = new UIDeformerRotate;
			break;
	}

	if( !NewObject )
		return;

	HTREEITEM			hSelectedItem = TreeView_GetSelection( gObjectTree );
	UIBaseObject *Container     = 0;
	
	if( NewObject->IsA( TUIImageFrame ) )
	{
		// Special case image frames so that they automatically create a style
		// when they are created unless they are being inserted into a style
	
		while( hSelectedItem )
		{
			UIBaseObject *o = GetObjectFromNode( hSelectedItem );

			if( o && o->IsA( TUIImageStyle ) )
			{
				Container = o;
				o->AddChild( NewObject );
				break;
			}			
			hSelectedItem = TreeView_GetParent( gObjectTree, hSelectedItem );
		}

		if( !Container )
		{
			UIBaseObject *NewContainer = new UIImageStyle;

			hSelectedItem = TreeView_GetSelection( gObjectTree );

			while( hSelectedItem )
			{
				UIBaseObject *o = GetObjectFromNode( hSelectedItem );

				if( o && o->AddChild( NewContainer ) )
				{
					Container = o;
					break;
				}

				hSelectedItem = TreeView_GetParent( gObjectTree, hSelectedItem );
			}

			if( !Container )
			{
				assert( RootPage->AddChild( NewContainer ) );
				Container = RootPage;
			}

			// Apply default properties to the new container
			gDefaultObjectPropertiesManager.ApplyDefaultPropertiesToObject( NewContainer );

			NewContainer->AddChild( NewObject );

			// Apply default properties after we insert the object into the tree
			// so that it can correctly link to other objects 
			gDefaultObjectPropertiesManager.ApplyDefaultPropertiesToObject( NewObject );

			// Switch this pointer to the created container so the right object gets loaded to the tree
			NewObject = NewContainer;
		}
	}
	else
	{
		while( hSelectedItem )
		{	
			UIBaseObject *o = GetObjectFromNode( hSelectedItem );

			if( o && o->AddChild( NewObject ) )
			{
				Container = o;
				break;
			}

			hSelectedItem = TreeView_GetParent( gObjectTree, hSelectedItem );
		}

		if( !Container )
		{
			assert( RootPage->AddChild( NewObject ) );
			Container = RootPage;
		}

		// Apply default properties after we insert the object into the tree
		// so that it can correctly link to other objects 
		gDefaultObjectPropertiesManager.ApplyDefaultPropertiesToObject( NewObject );
	}

	if( Container->CanChildMove( NewObject, UIBaseObject::ChildMovementDirection::Top ) )
		Container->MoveChild( NewObject, UIBaseObject::ChildMovementDirection::Top );

	if( NewObject->IsA( TUIPage ) && (NewObject->GetParent() == RootPage) )
		LoadTopLevelObjectsToTabControl();
	
	HTREEITEM hNewItem = LoadObjectStructureToTreeControl( gObjectTree, NewObject, hSelectedItem, TVI_FIRST );

	TreeView_SelectItem( gObjectTree, hNewItem );
	TreeView_EnsureVisible( gObjectTree, hNewItem );

	recordUndo(NewObject, true);
}

//-----------------------------------------------------------------

bool RenameSelectedObjectInTree( HWND hTree, const char * newName)
{
	if( !hTree )
		hTree = gObjectTree;
	
	if( !hTree )
		return false;

	const HTREEITEM hSelectedItem = TreeView_GetSelection( hTree );	

	if( !hSelectedItem )
		return false;

	const UIBaseObject * const root = UIManager::gUIManager().GetRootPage();
	UIBaseObject * const obj        = GetObjectFromNode( hSelectedItem );

	if( !obj || (obj == root) )
		return false;
	
	UIString nameVal;
	obj->GetProperty(UIBaseObject::PropertyName::Name, nameVal);
	UIString newNameVal(Unicode::narrowToWide(newName));
	obj->SetName(newName);
	recordUndo(obj, UIBaseObject::PropertyName::Name, nameVal, newNameVal, true);

	return true;
}

//----------------------------------------------------------------------

void SortSelectedObject ( HWND hTree)
{
	HTREEITEM			hSelectedItem = TreeView_GetSelection( hTree );	

	if( !hSelectedItem )
		return;

	HTREEITEM hPrevSelection = TreeView_GetPrevSibling( hTree, hSelectedItem );

	UIBaseObject * const sel = GetObjectFromNode( hSelectedItem );

	if (!sel || sel->GetChildCount () == 0)
		return;

	recordUndo(sel, true);

	typedef ui_stdmap<UILowerString, UIBaseObject *>::fwd ObjNameMap;
	ObjNameMap onm;

	UIBaseObject::UIObjectList olist;
	sel->GetChildren (olist);

	{
		for (UIBaseObject::UIObjectList::iterator it = olist.begin (); it != olist.end (); ++it)
		{
			UIBaseObject * const obj = *it;
			obj->Attach (0);
			sel->RemoveChild (obj);
			onm.insert (std::make_pair (UILowerString (obj->GetName ()), obj));
		}
	}

	for (ObjNameMap::iterator it = onm.begin (); it != onm.end (); ++it)
	{
		UIBaseObject * const obj = (*it).second;
		sel->AddChild (obj);
		obj->Detach (0);
	}

	HTREEITEM treeParent = TreeView_GetParent( hTree, hSelectedItem );

	TreeView_DeleteItem( hTree, hSelectedItem );

	WaitForSingleObject( gFrameRenderingMutex, INFINITE );

	if( sel->IsA( TUIPage ) && sel->GetParent () == 0)
		LoadTopLevelObjectsToTabControl();
	
	HTREEITEM newSelection = LoadObjectStructureToTreeControl (hTree, sel, treeParent, hPrevSelection);

	TreeView_SelectItem( hTree, newSelection );
	TreeView_Expand(hTree, newSelection, TVE_EXPAND);

	TreeView_EnsureVisible( hTree, newSelection );

	ReleaseMutex( gFrameRenderingMutex );
}

//-----------------------------------------------------------------

void DeleteSelectedObjectInTree( HWND hTree )
{
	if( !hTree )
		return;

	HTREEITEM			hSelectedItem = TreeView_GetSelection( hTree );	
	HTREEITEM			hNextSelection;
	UIPage       *RootPage = UIManager::gUIManager().GetRootPage();

	if( !hSelectedItem )
		return;

	UIBaseObject *theObjectToDelete = GetObjectFromNode( hSelectedItem );

	int allowedReferenceCount = 2; // one for the parent, one for being in the selection
	if(theObjectToDelete == gObjectInspector->GetObject())
		allowedReferenceCount++;

	if( !theObjectToDelete || (theObjectToDelete == RootPage) )
		return;

	//-- see if anything has an outstanding reference to this object
	if (theObjectToDelete->GetRefCount () > allowedReferenceCount)
	{
		char buf [512];
		_snprintf(buf, sizeof (buf) - 1, "%s has %d extra outstanding references.\nReally delete it?", theObjectToDelete->GetName().c_str(), theObjectToDelete->GetRefCount() - allowedReferenceCount);
		if (MessageBox( gMainWindow, buf, gApplicationName, MB_YESNO ) != IDYES)
			return;
	}

	recordUndo(theObjectToDelete, true);

	hNextSelection = TreeView_GetNextSibling( hTree, hSelectedItem );

	if( !hNextSelection )
	{
		hNextSelection = TreeView_GetPrevSibling( hTree, hSelectedItem );

		//-- don't select parents
	}

	RemoveFromSelection( theObjectToDelete );

	TreeView_DeleteItem( hTree, hSelectedItem );
	WaitForSingleObject( gFrameRenderingMutex, INFINITE );

	UIBaseObject *theParent = theObjectToDelete->GetParent();

	theObjectToDelete->Attach( 0 );

	if( theParent )
		theParent->RemoveChild( theObjectToDelete );

	if( theObjectToDelete->IsA( TUIPage ) && (theObjectToDelete->GetParent() == RootPage) )
		LoadTopLevelObjectsToTabControl();
	
	TreeView_SelectItem( hTree, hNextSelection );

	if( !hNextSelection )
		gObjectInspector->SetObject( 0 );
	else
	{
		TVITEM tvi = {0};

		tvi.mask  = TVIF_HANDLE;
		tvi.hItem = hNextSelection;

		TreeView_GetItem( hTree, &tvi );

		gObjectInspector->SetObject( reinterpret_cast<UIBaseObject *>( tvi.lParam ) );
		TreeView_EnsureVisible( hTree, hNextSelection );
	}

	// Do this last
	theObjectToDelete->Detach(0);
	ReleaseMutex( gFrameRenderingMutex );
}

//-----------------------------------------------------------------

void MoveSizeDlgControl( HWND hwnd, UINT itemID, UIPoint Move, UIPoint Size )
{
	RECT   rcControl;
	HWND   hwndControl;

	hwndControl = GetDlgItem( hwnd, itemID );	
	GetWindowRect( hwndControl, &rcControl );
	MapWindowPoints( 0, hwnd, (LPPOINT)&rcControl, 2 );				
	MoveWindow( hwndControl, rcControl.left + Move.x, rcControl.top + Move.y,
					rcControl.right - rcControl.left + Size.x, rcControl.bottom - rcControl.top + Size.y, TRUE );
	InvalidateRect( hwndControl, 0, TRUE );
}

//-----------------------------------------------------------------

void UnloadObjects( HWND hwndDlg )
{
	if( gObjectInspector )
		gObjectInspector->SetObject(0);

	SendMessage( gObjectTree, WM_SETREDRAW, 0, 0 );
	TreeView_DeleteAllItems( gObjectTree );
	SendMessage( gObjectTree, WM_SETREDRAW, 1, 0 );
	InvalidateRect( gObjectTree, 0, TRUE );

	SendDlgItemMessage( hwndDlg, IDC_OBJECTPROPERTIES, LB_RESETCONTENT, 0, 0 );

	UIManager::gUIManager().SetRootPage(0);
}

//-----------------------------------------------------------------

UINT CALLBACK SaveAsHookProc( HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam )
{
	UI_UNREF (lParam);

	switch( uMsg )
	{
		case WM_INITDIALOG:
			if( gFilename.empty() )
				EnableWindow( GetDlgItem( hwndDlg, ID_SAVE_NEXT_VERSION ), FALSE );
			else
				EnableWindow( GetDlgItem( hwndDlg, ID_SAVE_NEXT_VERSION ), TRUE );

			break;
		
		case WM_COMMAND:
			if( LOWORD( wParam ) == ID_SAVE_NEXT_VERSION )
			{
				HWND hParent = GetParent(hwndDlg);

				gVersionFilename = true;				
				SendMessage(hParent, WM_COMMAND, IDOK, (LPARAM)GetDlgItem( hParent, IDOK ) );
			}
			break;
	}
	return 0;
}

//-----------------------------------------------------------------

LRESULT CALLBACK TooltipMessageHookProc(int nCode, WPARAM wParam, LPARAM lParam) 
{ 
  if( nCode < 0 ) 
    return CallNextHookEx(gTooltipHook, nCode, wParam, lParam);

	switch( ((MSG *)lParam)->message) 
	{ 
    case WM_MOUSEMOVE:		
    case WM_LBUTTONDOWN: 
    case WM_LBUTTONUP:
	case WM_MBUTTONDOWN: 
    case WM_MBUTTONUP: 
    case WM_RBUTTONDOWN: 
    case WM_RBUTTONUP: 
	case WM_NCMOUSEMOVE:		
    case WM_NCLBUTTONDOWN: 
    case WM_NCLBUTTONUP:
	case WM_NCMBUTTONDOWN: 
    case WM_NCMBUTTONUP: 
    case WM_NCRBUTTONDOWN: 
    case WM_NCRBUTTONUP: 
      if (gTooltip != 0) 
        SendMessage(gTooltip, TTM_RELAYEVENT, 0, lParam);
      break;
  } 
  return CallNextHookEx(gTooltipHook, nCode, wParam, lParam);
}

//-----------------------------------------------------------------

void RecursiveSetProperty( UIBaseObject *root, const UILowerString & PropertyName, const char *OldValue, const char *NewValue )
{
	UIString Value;

	if( root->GetProperty( PropertyName, Value ) )
	{
		if( !_stricmp( UIUnicode::wideToNarrow (Value).c_str(), OldValue ) )
			root->SetProperty( PropertyName, UIUnicode::narrowToWide (NewValue) );

		UIBaseObject::UIObjectList Children;

		root->GetChildren( Children );

		for( UIBaseObject::UIObjectList::iterator i = Children.begin(); i != Children.end(); ++i )
		{
			RecursiveSetProperty( *i, PropertyName, OldValue, NewValue );
		}
	}
}

//-----------------------------------------------------------------

bool SaveWorkspaceFile( bool SaveAs )
{
	char  res[_MAX_PATH+1];
	FILE *fp = 0;

	WaitForSingleObject( gFrameRenderingMutex, INFINITE );

	UIPage * const RootPage = UIManager::gUIManager().GetRootPage();

	if (RootPage)
	{
		int version = 0;
		if (RootPage->GetPropertyInteger (UIVersion::PropertyVersion, version) && version > UIVersion::ms_version)
		{
			MessageBox (NULL, "WARNING: the data you are editing is a newer version than this UIBuilder.", "UIBuilder", MB_OK  | MB_ICONWARNING);

			if (IDYES != MessageBox( 0, "Are you sure you want to overrite the newer data?", gApplicationName, MB_YESNO | MB_DEFBUTTON2 | MB_ICONWARNING ))
			{
				ReleaseMutex( gFrameRenderingMutex );
				return false;
			}
		}
	}

	strncpy( res, gFilename.c_str(), sizeof(res) );
	res[sizeof(res)-1] = '\0';

	if( gFilename.empty() || SaveAs )
	{
		OPENFILENAME ofn = {sizeof(ofn)};

		ofn.lpstrInitialDir = ".";
		ofn.hwndOwner				= gMainWindow;
		ofn.hInstance				= GetModuleHandle(0);
		ofn.lpstrFilter     = gFileFilter;
		ofn.nFilterIndex    = 1;
		ofn.lpstrDefExt			= gDefaultExtension;
		ofn.lpstrFile				= res;
		ofn.nMaxFile				= sizeof( res );
		ofn.lpstrTitle			= "Save As...";		
		ofn.Flags						= OFN_EXPLORER | OFN_ENABLEHOOK | OFN_ENABLETEMPLATE;
		ofn.lpfnHook				= SaveAsHookProc;
		ofn.lpTemplateName  = MAKEINTRESOURCE( IDD_SAVE_AS_TEMPLATE );

		gVersionFilename = false;

		if( !GetSaveFileName( &ofn ) )
		{
			ReleaseMutex( gFrameRenderingMutex );
			return false;
		}

		if( gVersionFilename )
		{	
			strncpy( res, gFilename.c_str(), sizeof(res) );
			res[sizeof(res)-1] = '\0';

			int version     = 0;
			
			int base		    = 1;
			char *extension = strrchr( res, '.' );
			
			if( extension )
			{
				*extension = '\0';
				extension++;
			}

			int i = 0;
			for( i = strlen( res ) - 1; i >= 0 && isdigit( res[i] ); --i )
			{
				version += (res[i] - '0') * base;
				base    *= 10;
			}

			if( i >= 0 )
			{
				if( res[i] == '_' )
					res[i] = '\0';
				else
					res[i+1] = '\0';
			}

			for (;;)
			{
				++version;
				
				char buffer[_MAX_PATH+1];

				if( extension && *extension != '\0' )
					sprintf( buffer, "%s_%03d.%s", res, version, extension );
				else
					sprintf( buffer, "%s_%03d.%s", res, version, gDefaultExtension );

				fp = fopen( buffer, "rb" );

				if( !fp )
				{
					strcpy( res, buffer );
					break;
				}
				else
					fclose( fp );
			}						
		}
	}

	if (RootPage)
	{
		//-- force packing
		RootPage->ForcePackChildren();

		UISaver			Saver;
		typedef std::map<UINarrowString, UINarrowString> NarrowStringMap;
		NarrowStringMap Output;

		RecursiveSetProperty( RootPage, UIBaseObject::PropertyName::SourceFile, gFilename.c_str(), res );
		Saver.SaveToStringSet( Output, Output.end(), *RootPage );

		std::vector<std::string> messages;

		std::string TmpStr;
		UIBuilderLoader Loader;

		for (NarrowStringMap::iterator i = Output.begin(); i != Output.end();)
		{
			const std::string & FileName   = i->first.empty () ? res : i->first;
			const std::string & outputData = i->second;
			
			++i;

			{
				struct _stat statbuf;
				const int result = _stat (FileName.c_str (), &statbuf);
				if (!result)
				{
					const int t = static_cast<int>(statbuf.st_mtime);
					const int oldTime = s_fileTimes [FileName];
					if (t > oldTime)
					{
						char buf [1024];
						_snprintf (buf, sizeof (buf), "The file [%s] has changed on disk.", FileName.c_str ());
						MessageBox( gMainWindow, buf, gApplicationName, MB_OK | MB_ICONWARNING);
						_snprintf (buf, sizeof (buf), "Are you sure you want to save [%s]?\nThis will almost certainly clobber someone else's changes.", FileName.c_str ());
						if (IDYES != MessageBox( 0, buf, gApplicationName, MB_YESNO | MB_DEFBUTTON2 | MB_ICONWARNING ))
						{
							continue;
						}
					}
				}
			}
			
			{
				TmpStr.clear ();
				Loader.LoadStringFromResource (FileName, TmpStr);
				
				//-- don't even attempt to write unmodified files
				if (TmpStr == outputData)
					continue;
			}

			FILE * fp = fopen( FileName.c_str (), "wb" );
			
			if( !fp )
			{
				messages.push_back (FileName);
			}
			else
			{
				fwrite( outputData.data (), outputData.size(), 1, fp );
				fclose( fp );
				
				struct _stat statbuf;
				const int result = _stat (FileName.c_str (), &statbuf);
				if (!result)
				{
					const int t = static_cast<int>(statbuf.st_mtime);
					s_fileTimes [FileName] = t;
				}
			}
		}

		if (!messages.empty ())
		{
			const std::string header ("Could not save objects to files:\n");
			int currentNumberOfFiles = 0;				

			std::string fileNames;

			for (size_t i = 0; i < messages.size ();)
			{
				const std::string & FileName = messages [i];

				++i;

				fileNames += FileName;
				fileNames.push_back ('\n');

				if (++currentNumberOfFiles >= 25 || i >= messages.size ())
				{
					const std::string msg = header + fileNames;
					MessageBox( gMainWindow, msg.c_str (), gApplicationName, MB_OK ); 
					fileNames.clear ();
					currentNumberOfFiles = 0;
				}
			}
		}
	}	

	gFilename = res;
	SetMainWindowTitle();	
	ReleaseMutex( gFrameRenderingMutex );
	return true;
}

//-----------------------------------------------------------------

bool CloseWorkspaceFile( bool bLock )
{	
	if( bLock )
		WaitForSingleObject( gFrameRenderingMutex, INFINITE );
	
	if( UIManager::gUIManager().GetRootPage() )
	{
		switch( MessageBox( 0, "Would you like to save your workspace before closing it?", gApplicationName, MB_YESNOCANCEL | MB_ICONWARNING ) )
		{
		case IDYES:
			SaveWorkspaceFile( false );
			break;
			
		case IDNO:
			break;
			
		case IDCANCEL:
			if( bLock )
				ReleaseMutex( gFrameRenderingMutex );
			
			return false;
		}
		
		UIBuilderHistory::remove ();
	}
	
	gCurrentlySelectedPage = 0;
	
	if (gObjectInspector)
		gObjectInspector->SetObject(0);
	
	SetSelection(0, true);
	
	UnloadObjects( gMainWindow );
	ShowWindow( gUIWindow, SW_HIDE );
	
	if( bLock )
		ReleaseMutex( gFrameRenderingMutex );
	
	return true;
}

//-----------------------------------------------------------------

bool OpenWorkspaceFile( const char *Filename )
{	
	char res[_MAX_PATH+1] = "";

	WaitForSingleObject( gFrameRenderingMutex, INFINITE );

	if( !Filename )
	{				
		OPENFILENAME ofn = {sizeof(ofn)};
		
		ofn.lpstrInitialDir = ".";
		ofn.hwndOwner				= gMainWindow;
		ofn.lpstrFilter     = gFileFilter;
		ofn.nFilterIndex    = 1;
		ofn.lpstrDefExt			= gDefaultExtension;
		ofn.lpstrFile				= res;
		ofn.nMaxFile				= sizeof( res );
		ofn.lpstrTitle			= "Select a User Interface Workspace to Open";
		ofn.Flags						= OFN_FILEMUSTEXIST | OFN_HIDEREADONLY;

		if( !GetOpenFileName( &ofn ) )
		{
			ReleaseMutex( gFrameRenderingMutex );
			return false;
		}

		if( !CloseWorkspaceFile(false) )
		{
			ReleaseMutex( gFrameRenderingMutex );
			return false;
		}

		Filename = res;
	}
	else
	{
		if( !CloseWorkspaceFile(false) )
		{
			ReleaseMutex( gFrameRenderingMutex );
			return false;
		}

		strcpy( res, Filename );
	}

	UIBuilderLoader Loader;
	UIBaseObject::UIObjectList		TopLevelObjects;

	char *pLastDelimitor = strrchr( res, '\\' );

	if( pLastDelimitor )
		*pLastDelimitor = '\0';

	SetCurrentDirectory( res );

	if( pLastDelimitor )
		*pLastDelimitor = '\\';

	s_fileTimes.clear ();

	if( !Loader.LoadFromResource( res, TopLevelObjects, true ) )
	{
		GetUIOutputStream()->flush ();
		MessageBox( 0, "The file could not be opened, check ui.log for more information", gApplicationName, MB_OK );
		ReleaseMutex( gFrameRenderingMutex );
		return false;
	}

	if( TopLevelObjects.size() > 1 )
	{
		MessageBox( 0, "Error: The file contains more than one root level object", gApplicationName, MB_OK );
		ReleaseMutex( gFrameRenderingMutex );
		return false;
	}
	
	UIBaseObject * const o = TopLevelObjects.front();

	UITextStyleManager::GetInstance()->Initialize(static_cast<UIPage *>(o), Loader);
	
	o->Link ();

	Loader.Lint ();

	if( o->IsA( TUIPage ) )
		UIManager::gUIManager().SetRootPage( static_cast< UIPage * >( o ) );
	else
	{
		MessageBox( 0, "Error: The root level object in the file is not a page", gApplicationName, MB_OK );
		ReleaseMutex( gFrameRenderingMutex );
		return false;
	}

	UIBuilderHistory::install ();

//	if( Loader.ReadOnlyFilesInInput )
//		MessageBox( NULL, "Warning, one or more of the files making up this script are read only.", "UIBuilder", MB_OK );

	int version = 0;
	if (o->GetPropertyInteger (UIVersion::PropertyVersion, version) && version > UIVersion::ms_version)
	{		
		MessageBox (NULL, "WARNING: the data you are editing is a newer version than this UIBuilder.", "UIBuilder", MB_OK | MB_ICONWARNING);
	}
	else
		o->SetPropertyInteger (UIVersion::PropertyVersion, UIVersion::ms_version);

	LoadObjectStructureToTreeControl(gObjectTree, UIManager::gUIManager().GetRootPage(), 0, TVI_LAST);
	TreeView_SelectItem(gObjectTree, TreeView_GetRoot(gObjectTree) );
	gObjectInspector->SetObject(UIManager::gUIManager().GetRootPage());
	
	gFilename = Filename;
	AddToMRUList( Filename );
	SetMainWindowTitle();

	RECT rc;
	GetWindowRect( gMainWindow, &rc );
	LoadTopLevelObjectsToTabControl();
	SetWindowPos( gUIWindow, 0, rc.right, rc.top, 0, 0, SWP_NOSIZE | SWP_NOZORDER | SWP_NOACTIVATE );
	ShowWindow( gUIWindow, SW_SHOWNA );	
	ReleaseMutex( gFrameRenderingMutex );
	return true;
}

//-----------------------------------------------------------------

void CreateNewWorkspace( void )
{
	WaitForSingleObject( gFrameRenderingMutex, INFINITE );

	if (UIBuilderHistory::isInstalled())
		UIBuilderHistory::remove();

	UIBuilderHistory::install ();

	UnloadObjects( gMainWindow );

	UIPage *NewRoot = new UIPage;
	NewRoot->SetName( "root" );
	NewRoot->SetVisible( true );
	NewRoot->SetSize( UISize(800,600) );
	NewRoot->SetEnabled( true );

	UIManager::gUIManager().SetRootPage( NewRoot );

	LoadTopLevelObjectsToTabControl();
	TabCtrl_SetCurSel( gTabControlWindow, 0 );
	SizeWindowToCurrentPageSelection();

	RECT rc;
	GetWindowRect( gMainWindow, &rc );	
	SetWindowPos( gUIWindow, 0, rc.right, rc.top, 0, 0, SWP_NOSIZE | SWP_NOZORDER | SWP_NOACTIVATE | SWP_SHOWWINDOW );

	LoadObjectStructureToTreeControl(gObjectTree, NewRoot, 0, TVI_LAST);
	TreeView_SelectItem( gObjectTree, TreeView_GetRoot( gObjectTree ) );
	TreeView_EnsureVisible( gObjectTree, TreeView_GetRoot( gObjectTree ) );
	SetFocus( gObjectTree );
	gObjectInspector->SetObject( NewRoot );

	gFilename = "";
	SetMainWindowTitle();

	ReleaseMutex( gFrameRenderingMutex );
}

//-----------------------------------------------------------------

void AcceptDroppedFiles( HDROP hDrop )
{	
	if( CloseWorkspaceFile() )
	{
		char Filename[_MAX_PATH + 1];

		DragQueryFile( hDrop, 0, Filename, sizeof( Filename ) );
		OpenWorkspaceFile( Filename );
	}
	DragFinish( hDrop );
}

//-----------------------------------------------------------------

void ImportFile( void )
{
	if( gFilename.empty() )
	{
		MessageBox( 0, "You must save your workspace file before performing an import.", gApplicationName, MB_OK );
		
		if( !SaveWorkspaceFile(true) )
			return;
	}

	char				 res[_MAX_PATH+1];
	OPENFILENAME ofn = {sizeof(ofn)};
		
	res[0] = '\0';
	ofn.lpstrInitialDir = ".";
	ofn.hwndOwner				= gMainWindow;
	ofn.lpstrFilter     = gIncludeFileFilter;
	ofn.nFilterIndex    = 1;
	ofn.lpstrDefExt			= gDefaultExtension;
	ofn.lpstrFile				= res;
	ofn.nMaxFile				= sizeof( res );
	ofn.lpstrTitle			= "Select File To Import";
	ofn.Flags						= OFN_FILEMUSTEXIST | OFN_HIDEREADONLY;

	if( !GetOpenFileName( &ofn ) )
	{
		MessageBox( 0, "Could not open file", gApplicationName, MB_OK );
		return;
	}

	char RelativePath[_MAX_PATH+1];

	PathRelativePathTo( RelativePath, gFilename.c_str(), 0, res, 0);

	if( (RelativePath[0] == '.') && (RelativePath[1] == '\\') )
		memmove( RelativePath, RelativePath + 2, strlen( RelativePath + 2 ) + 1 );

  UIBuilderLoader	Loader;
	UIBaseObject::UIObjectList		TopLevelObjects;

	if( !Loader.LoadFromResource( RelativePath, TopLevelObjects, true ) )
	{
		MessageBox( 0, "The file could not be opened, check ui.log for more information", gApplicationName, MB_OK );

		while( !TopLevelObjects.empty() )
		{
			TopLevelObjects.back()->Detach (0);
			TopLevelObjects.pop_back();
		}
		return;
	}

	if( Loader.ReadOnlyFilesInInput )
		MessageBox( NULL, "Warning, one or more of the files making up the imported script are read only.", "UIBuilder", MB_OK );

	HTREEITEM     hParent = TreeView_GetSelection( gObjectTree );
	UIBaseObject *pParent = GetObjectFromNode( hParent );
	long          ObjectsNotLoaded = 0;

	for( UIBaseObject::UIObjectList::iterator i = TopLevelObjects.begin(); i != TopLevelObjects.end(); ++i )
	{
		if( !pParent || !pParent->AddChild( *i ) )
		{
			(*i)->Detach (0);
			*i = 0;
			++ObjectsNotLoaded;
		}
		else
		{
			LoadObjectStructureToTreeControl(gObjectTree, *i, hParent, TVI_LAST);
			TreeView_Expand(gObjectTree, hParent, TVE_EXPAND);
		}
	}

	if( ObjectsNotLoaded > 0 )
	{
		char Buffer[1024];
		sprintf(Buffer, "%d objects were not loaded from the file because they could not be contained in the selected object", ObjectsNotLoaded );
	}
	
	if( UIManager::gUIManager().GetRootPage() )
		UIManager::gUIManager().GetRootPage()->Link();
	
	LoadTopLevelObjectsToTabControl();	
	ReleaseMutex( gFrameRenderingMutex );
}

//-----------------------------------------------------------------

void ExportSelectionToFile( void )
{
	MessageBox(NULL, gNotImplemented, gApplicationName, MB_OK );
}

//-----------------------------------------------------------------

void PurgeSelectedProperty( UIBaseObject *o, UINarrowString PropertyName )
{
	if (o)
	{	
		recordUndo(o, true);

		UIBaseObject::UIObjectList Children;

		o->RemoveProperty( UILowerString (PropertyName) );

		o->GetChildren( Children );

		for( UIBaseObject::UIObjectList::iterator i = Children.begin(); i != Children.end(); ++i )
			PurgeSelectedProperty( *i, PropertyName );
	}
}

//-----------------------------------------------------------------

void ClearClipboard( void )
{

	gClipboard.clear();
}

//-----------------------------------------------------------------

void CopySelectedObjectInTreeToClipboard( HWND hTree )
{
	UI_UNREF (hTree);

	ClearClipboard();

	for( UIBaseObject::UISmartObjectList::iterator i = gCurrentSelection.begin(); i != gCurrentSelection.end(); ++i )
	{
		if( (*i)->GetParent() )
			gClipboard.push_back(UIBaseObject::UIBaseObjectPointer((*i)->DuplicateObject()));
	}
}

//-----------------------------------------------------------------

void PasteObjectFromClipboard( HWND hTree )
{
	UI_UNREF (hTree);

	UIBaseObject *NewParentObject = GetSelectedObjectInTreeControl();
	int           FailedCopies = 0;

	if (NewParentObject)
	{	
		for( UIBaseObject::UISmartObjectList::reverse_iterator i = gClipboard.rbegin(); i != gClipboard.rend(); ++i )
		{
			UIBaseObject::UIBaseObjectPointer NewObject((*i)->DuplicateObject());

			NewObject->PurgeProperty( UIBaseObject::PropertyName::SourceFile );
			
			if( NewParentObject->AddChild( NewObject ) )
			{
				NewObject->Link ();

				if ( NewObject->IsA(TUIWidget))
				{

					UI_ASOBJECT(UIWidget, NewObject)->ResetPackSizeInfo();
					UI_ASOBJECT(UIWidget, NewObject)->ResetPackLocationInfo();
				}
				
				if( NewParentObject->MoveChild( NewObject, UIBaseObject::ChildMovementDirection::Top ) )
					LoadObjectStructureToTreeControl( gObjectTree, NewObject, TreeView_GetSelection( gObjectTree ), TVI_FIRST );
				else
					LoadObjectStructureToTreeControl( gObjectTree, NewObject, TreeView_GetSelection( gObjectTree ), TVI_LAST );
			}
			else
			{
				++FailedCopies;
			}
		}
	}

	if( FailedCopies > 0 )
	{
		char buffer[1028];

		sprintf(buffer, "%d object%s could not be pasted because they could not be contained in the selected object.", FailedCopies, FailedCopies == 1 ? "" : "s" );
		MessageBox( 0, buffer, 0, MB_OK );
	}
}
 
//-----------------------------------------------------------------

BOOL CALLBACK MainWindowProc( HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam )
{
	switch( uMsg )
	{
	case WM_INITDIALOG:
		{	
			HIMAGELIST hImageList;
			
			// Load MRU file list
			LoadMRUListFromRegistry();
			LoadPreferencesFromRegistry ();
			
			// Initialize globals
			gMainWindow = hwndDlg;		
			gTooltip		= CreateWindow( TOOLTIPS_CLASS, 0, TTS_ALWAYSTIP, 0, 0, 0, 0, 0, 0, GetModuleHandle(0), 0 );
			gObjectTree	= GetDlgItem( hwndDlg, IDC_OBJECTSTRUCTURE );
			hImageList	= ImageList_LoadImage( GetModuleHandle(0), MAKEINTRESOURCE(IDR_IMAGELIST), 16, 1, CLR_DEFAULT, IMAGE_BITMAP, 0 );
			
			SetMainWindowTitle();
			CreateDialog( GetModuleHandle(0), MAKEINTRESOURCE(IDD_UIWINDOW), gMainWindow, UIWindowDlgProc );
			
			// Set up drag and drop 
			DragAcceptFiles( gMainWindow, TRUE );
			
			// Set up tooltips
			AddTooltipFromControlID( gTooltip, gMainWindow, IDC_TOP, "Move control to top of ZOrder" );
			AddTooltipFromControlID( gTooltip, gMainWindow, IDC_UP, "Move control up in ZOrder" );
			AddTooltipFromControlID( gTooltip, gMainWindow, IDC_DOWN, "Move control down in ZOrder" );
			AddTooltipFromControlID( gTooltip, gMainWindow, IDC_BOTTOM, "Move control to bottom of ZOrder" );
			
			AddTooltipFromControlID( gTooltip, gMainWindow, IDC_LOCK, "Lock / Unlock Control" );
			AddTooltipFromControlID( gTooltip, gMainWindow, IDC_ADDPROPERTY, "Add Property" );
			AddTooltipFromControlID( gTooltip, gMainWindow, IDC_REMOVEPROPERTY, "Remove Property" );

			AddTooltipFromControlID( gTooltip, gMainWindow, IDC_CHECKOUT, "Checkout source file using Perforce." );
		
			
			gTooltipHook = SetWindowsHookEx(WH_GETMESSAGE, TooltipMessageHookProc, (HINSTANCE) 0, GetCurrentThreadId() ); 
			
			TreeView_SetImageList( gObjectTree, 0, TVSIL_NORMAL );
			TreeView_SetImageList( gObjectTree, hImageList, TVSIL_NORMAL );
			
			gObjectInspector = new ObjectInspector( GetDlgItem( hwndDlg, IDC_OBJECTPROPERTIES ) );
			
			if( gLimitFrameRate )
				CheckDlgButton( hwndDlg, IDC_CAPFRAMERATE, BST_CHECKED );
			else
				CheckDlgButton( hwndDlg, IDC_CAPFRAMERATE, BST_UNCHECKED );
			
			SetTimer( hwndDlg, 0, 100, 0 );
			
			RECT rcDesktop;
			RECT rcWindow;
			RECT rcClient;
			
			SystemParametersInfo( SPI_GETWORKAREA, 0, &rcDesktop, 0 );
			GetWindowRect( hwndDlg, &rcWindow );
			GetClientRect( hwndDlg, &rcClient );
			
			gInitialSize.x = gOldSize.x = rcClient.right - rcClient.left;
			gInitialSize.y = gOldSize.y = rcClient.bottom - rcClient.top;
			
			SetWindowPos( gMainWindow, 0, rcDesktop.left, rcDesktop.top, rcWindow.right - rcWindow.left,
				rcDesktop.bottom - rcDesktop.top, SWP_NOZORDER | SWP_NOACTIVATE );
			
			SendDlgItemMessage( hwndDlg, IDC_TOP, BM_SETIMAGE, IMAGE_ICON,
				(LPARAM)LoadImage( GetModuleHandle(0), MAKEINTRESOURCE(IDI_TOP), IMAGE_ICON, 16, 16, 0 ) );
			SendDlgItemMessage( hwndDlg, IDC_UP, BM_SETIMAGE, IMAGE_ICON,
				(LPARAM)LoadImage( GetModuleHandle(0), MAKEINTRESOURCE(IDI_UP), IMAGE_ICON, 16, 16, 0 ) );
			SendDlgItemMessage( hwndDlg, IDC_DOWN, BM_SETIMAGE, IMAGE_ICON,
				(LPARAM)LoadImage( GetModuleHandle(0), MAKEINTRESOURCE(IDI_DOWN), IMAGE_ICON, 16, 16, 0 ) );
			SendDlgItemMessage( hwndDlg, IDC_BOTTOM, BM_SETIMAGE, IMAGE_ICON,
				(LPARAM)LoadImage( GetModuleHandle(0), MAKEINTRESOURCE(IDI_BOTTOM), IMAGE_ICON, 16, 16, 0 ) );
			
			EnableButtonControls( hwndDlg, 0 );
			
			SendDlgItemMessage( hwndDlg, IDC_ADDPROPERTY, BM_SETIMAGE, IMAGE_ICON,
				(LPARAM)LoadImage( GetModuleHandle(0), MAKEINTRESOURCE(IDI_ADDPROPERTY), IMAGE_ICON, 16, 16, 0 ) );
			
			SendDlgItemMessage( hwndDlg, IDC_REMOVEPROPERTY, BM_SETIMAGE, IMAGE_ICON,
				(LPARAM)LoadImage( GetModuleHandle(0), MAKEINTRESOURCE(IDI_REMOVEPROPERTY), IMAGE_ICON, 16, 16, 0 ) );
			
			SendDlgItemMessage( hwndDlg, IDC_LOCK, BM_SETIMAGE, IMAGE_ICON,
				(LPARAM)LoadImage( GetModuleHandle(0), MAKEINTRESOURCE(IDI_LOCK), IMAGE_ICON, 16, 16, 0 ) );

			SendDlgItemMessage( hwndDlg, IDC_CHECKOUT, BM_SETIMAGE, IMAGE_ICON,
				(LPARAM)LoadImage( GetModuleHandle(0), MAKEINTRESOURCE(IDI_CHECKOUT), IMAGE_ICON, 16, 16, 0 ) );

			
			SendMessage( hwndDlg, WM_SETICON, ICON_BIG, 
				(LPARAM)LoadImage( GetModuleHandle(0), MAKEINTRESOURCE(IDI_APP), IMAGE_ICON, 32, 32, 0 ) );
			
			SendMessage( hwndDlg, WM_SETICON, ICON_SMALL, 
				(LPARAM)LoadImage( GetModuleHandle(0), MAKEINTRESOURCE(IDI_APP), IMAGE_ICON, 16, 16, 0 ) );
			
			char DirBuff[_MAX_PATH + 1];
			GetCurrentDirectory( sizeof( DirBuff ), DirBuff );
			gInitialDirectory = DirBuff;
			
			FILE *fp = fopen("defaults.cfg", "rb");
			
			if( fp )
			{
				gDefaultObjectPropertiesManager.LoadFrom( fp );
				fclose( fp );
			}
			
			// searchpaths for FileLocator - jwatson
			
			FileLocator & loc = FileLocator::gFileLocator ();
			loc.addPath ("./");
			
			fp = fopen ("uibuilder_searchpaths.cfg", "rb");
			
			if (fp)
			{
				char buf[1024];
				
				while (fgets (buf, 1024, fp))
				{
					// strip newline
					char * ptr = strchr (buf, '\n');
					
					if (ptr)
						*ptr = 0;
					
					ptr = strchr (buf, '\r');
					
					if (ptr)
						*ptr = 0;
					
					loc.addPath (buf);
				}
				
				fclose (fp);
			}
			
			return 0;
		}
		
		case WM_INITMENU:
			gMenu = (HMENU)wParam;
			LoadMRUListToMenu(gMenu);
			EnableMenuItems(gMenu, GetSelectedObjectInTreeControl() );
			return 0;
			
		case WM_DROPFILES:
			AcceptDroppedFiles( (HDROP)wParam );
			return 0;
			
		case WM_MOUSEMOVE:
		case WM_LBUTTONDOWN:
		case WM_LBUTTONUP:
		case WM_MBUTTONDOWN:
		case WM_MBUTTONUP:
		case WM_RBUTTONDOWN:
		case WM_RBUTTONUP:
		case WM_LBUTTONDBLCLK:
		case WM_MBUTTONDBLCLK:
		case WM_RBUTTONDBLCLK:
			{
				LONG   l;
				POINTS pt;
				MSG    msg;
				
				l  = GetMessagePos();
				pt = MAKEPOINTS(l);
				msg.hwnd    = hwndDlg;
				msg.message = uMsg;
				msg.wParam  = wParam;
				msg.lParam  = lParam;
				msg.pt.x    = pt.x;
				msg.pt.y    = pt.y;
				msg.time    = GetMessageTime();
				
				SendMessage( gTooltip, TTM_RELAYEVENT, 0, (LPARAM)&msg );
				return 0;
			}
			
		case WM_MOVE:
			{
				RECT rc;
				GetWindowRect( hwndDlg, &rc );
				SetWindowPos( gUIWindow, 0, rc.right, rc.top, 0, 0, SWP_NOSIZE | SWP_NOZORDER | SWP_NOACTIVATE );
				return 0;
			}
			
		case WM_SIZE:
			{
				UISize NewSize;
				
				NewSize.x = LOWORD(lParam);
				NewSize.y = HIWORD(lParam);
				
				if( (NewSize.x == 0) || (NewSize.y == 0) )
					return 0;
				
				if( gOldSize.x == 0 )
					gInitialSize = NewSize;
				else
				{
					UISize DeltaSize = NewSize - gOldSize;
					
					MoveSizeDlgControl( hwndDlg, IDC_PERFORMANCESTATS,				UIPoint( 0, DeltaSize.y ), UIPoint(DeltaSize.x,0) );
					MoveSizeDlgControl( hwndDlg, IDC_FRAMERATE_LABEL,					UIPoint( 0, DeltaSize.y ), UIPoint(0,0) );
					MoveSizeDlgControl( hwndDlg, IDC_FRAMERATE,								UIPoint( 0, DeltaSize.y ), UIPoint(0,0) );
					MoveSizeDlgControl( hwndDlg, IDC_CAPFRAMERATE,						UIPoint( 0, DeltaSize.y ), UIPoint(0,0) );
					MoveSizeDlgControl( hwndDlg, IDC_TRIANGLECOUNT_LABEL,			UIPoint( 0, DeltaSize.y ), UIPoint(0,0) );
					MoveSizeDlgControl( hwndDlg, IDC_TRIANGLECOUNT,						UIPoint( 0, DeltaSize.y ), UIPoint(0,0) );
					MoveSizeDlgControl( hwndDlg, IDC_SHOWTRIANGLES,						UIPoint( 0, DeltaSize.y ), UIPoint(0,0) );
					MoveSizeDlgControl( hwndDlg, IDC_FLUSHCOUNT_LABEL,				UIPoint( 0, DeltaSize.y ), UIPoint(0,0) );
					MoveSizeDlgControl( hwndDlg, IDC_FLUSHCOUNT,							UIPoint( 0, DeltaSize.y ), UIPoint(0,0) );				
					
					MoveSizeDlgControl( hwndDlg, IDC_OBJECTPROPERTIES,				UIPoint( 0, DeltaSize.y ), UIPoint(DeltaSize.x,0) );
					MoveSizeDlgControl( hwndDlg, IDC_OBJECTPROPERTIES_LABEL,	UIPoint( 0, DeltaSize.y ), UIPoint(0,0) );
					
					MoveSizeDlgControl( hwndDlg, IDC_OBJECTSTRUCTURE,					UIPoint( 0, 0 ), UIPoint(DeltaSize.x,DeltaSize.y) );
					
					MoveSizeDlgControl( hwndDlg, IDC_BOTTOM,									UIPoint( DeltaSize.x, 0 ), UIPoint(0,0) );
					MoveSizeDlgControl( hwndDlg, IDC_DOWN,										UIPoint( DeltaSize.x, 0 ), UIPoint(0,0) );
					MoveSizeDlgControl( hwndDlg, IDC_UP,											UIPoint( DeltaSize.x, 0 ), UIPoint(0,0) );
					MoveSizeDlgControl( hwndDlg, IDC_TOP,											UIPoint( DeltaSize.x, 0 ), UIPoint(0,0) );
					
					MoveSizeDlgControl( hwndDlg, IDC_ADDPROPERTY,							UIPoint( DeltaSize.x, DeltaSize.y ), UIPoint(0,0) );
					MoveSizeDlgControl( hwndDlg, IDC_REMOVEPROPERTY,					UIPoint( DeltaSize.x, DeltaSize.y ), UIPoint(0,0) );
					MoveSizeDlgControl( hwndDlg, IDC_LOCK,										UIPoint( DeltaSize.x, DeltaSize.y ), UIPoint(0,0) );
					MoveSizeDlgControl( hwndDlg, IDC_CHECKOUT, UIPoint( DeltaSize.x, DeltaSize.y ), UIPoint(0,0) );				
					MoveSizeDlgControl( hwndDlg, IDC_OBJECTTYPE,							UIPoint( 0, DeltaSize.y ), UIPoint(DeltaSize.x,0) );
				}
				
				gOldSize = NewSize;
				
				RECT rc;
				GetWindowRect( hwndDlg, &rc );
				SetWindowPos( gUIWindow, 0, rc.right, rc.top, 0, 0, SWP_NOSIZE | SWP_NOZORDER | SWP_NOACTIVATE );
				return 0;
			}
			
		case WM_GETMINMAXINFO:
			{
				LPMINMAXINFO pmmi = (LPMINMAXINFO)lParam;
				RECT         rc;
				
				rc.left   = 0;
				rc.top    = 0;
				rc.right  = gInitialSize.x;
				rc.bottom = gInitialSize.y;
				
				AdjustWindowRectEx( &rc, GetWindowLong( hwndDlg, GWL_STYLE ), TRUE, GetWindowLong( hwndDlg, GWL_EXSTYLE ) );
				pmmi->ptMinTrackSize.x = rc.right - rc.left;
				pmmi->ptMinTrackSize.y = rc.bottom - rc.top;
				return 0;
			}
			
		case WM_COMMAND:
			if( LOWORD( wParam ) >= 0xFFFF - gMRUSize )
			{
				// MRU list
				int ItemID = 0xFFFF - LOWORD( wParam ) - 1;
				
				MRUList::iterator i = gMRUList.begin();
				
				while( ItemID && (i != gMRUList.end()) )
				{
					++i;
					--ItemID;
				}
				
				if( i != gMRUList.end() )
				{
					// Make a copy of this so that we have don't erase it in AddToMRU
					UINarrowString ItemCopy = *i;					
					UI_IGNORE_RETURN(OpenWorkspaceFile(ItemCopy.c_str()));
				}
				
				return 0;
			}
			
			switch( LOWORD( wParam ) )
			{
			case IDC_SHOWTRIANGLES:
				if( HIWORD( wParam ) == BN_CLICKED )
				{
					WaitForSingleObject( gFrameRenderingMutex, INFINITE );
					const bool show = SendMessage( (HWND)lParam, BM_GETSTATE, 0, 0 ) & BST_CHECKED;
					gPrimaryDisplay->ShowTriangles (show);
					UITextStyle::setEnabled (!show);

					ReleaseMutex( gFrameRenderingMutex );
				}
				break;
				
			case ID_FILE_NEW:
				CreateNewWorkspace();					
				break;
				
			case ID_FILE_OPEN:
				OpenWorkspaceFile( 0 );
				break;
				
			case ID_FILE_SAVE:					
				SaveWorkspaceFile( false );					
				break;
				
			case ID_FILE_SAVEAS:
				SaveWorkspaceFile( true );
				break;
				
			case ID_FILE_CLOSE:
				CloseWorkspaceFile();
				break;
				
			case ID_FILE_IMPORT:
				ImportFile();
				break;
				
			case ID_FILE_EXPORT:
				ExportSelectionToFile();
				break;
				
			case ID_FILE_EXIT:
				if( CloseWorkspaceFile() )
				{
					EndDialog( gUIWindow, 0 );
					DestroyWindow( hwndDlg );
				}
				break;
				
			case ID_INSERT_BUTTON:
			case ID_INSERT_CHECKBIOX:
			case ID_INSERT_DATASOURCE:
			case ID_INSERT_DATASOURCECONTAINER:
			case ID_INSERT_DATAITEM:
			case ID_INSERT_DROPDOWNBOX:
			case ID_INSERT_ELLIPSE:
			case ID_INSERT_IMAGE:				
			case ID_INSERT_GRID:
			case ID_INSERT_IMAGEFRAME:
			case ID_INSERT_LISTBOX:
			case ID_INSERT_LIST:
			case ID_INSERT_TREEVIEW:
			case ID_INSERT_COMBOBOX:
			case ID_INSERT_NAMESPACE:
			case ID_INSERT_TEXTBOX:
			case ID_INSERT_SCROLLBAR:
			case ID_INSERT_SLIDERBAR:
			case ID_INSERT_SLIDERPLANE:
			case ID_INSERT_TABSET:
			case ID_INSERT_TABBEDPANE:
			case ID_INSERT_POPUPMENU:
			case ID_INSERT_RADIALMENU:
			case ID_INSERT_CURSORSET:
			case ID_INSERT_VOLUMEPAGE:
			case ID_INSERT_STATICTEXT:
			case ID_INSERT_TEMPLATE:
			case ID_INSERT_PAGE:
			case ID_INSERT_COMPOSITE:
			case ID_INSERT_TABLE:
			case ID_INSERT_TABLEMODELDEFAULT:
			case ID_INSERT_TABLEHEADER:
			case ID_INSERT_PROGRESSBAR:
			case ID_INSERT_CURSOR:
			case ID_INSERT_CLOCK:
			case ID_INSERT_APPLICATIONOBJECT:
			case ID_INSERT_STYLE_BUTTON:
			case ID_INSERT_STYLE_CHECKBOX:
			case ID_INSERT_STYLE_DROPDOWNBOX:
			case ID_INSERT_STYLE_GRID:
			case ID_INSERT_STYLE_IMAGE:
			case ID_INSERT_STYLE_LISTBOX:
			case ID_INSERT_STYLE_LIST:
			case ID_INSERT_STYLE_TREEVIEW:
			case ID_INSERT_STYLE_PROGRESSBAR:
			case ID_INSERT_STYLE_SLIDERBAR:
			case ID_INSERT_STYLE_TEXT:
			case ID_INSERT_STYLE_TEXTBOX:
			case ID_INSERT_STYLE_TOOLTIP:
			case ID_INSERT_STYLE_RECTANGLE:
			case ID_INSERT_STYLE_TABSET:
			case ID_INSERT_STYLE_TABBEDPANE:
			case ID_INSERT_STYLE_POPUPMENU:
			case ID_INSERT_STYLE_RADIALMENU:
			case ID_INSERT_EFFECTOR_COLOR:
			case ID_INSERT_EFFECTOR_ROTATION:
			case ID_INSERT_EFFECTOR_LOCATION:
			case ID_INSERT_EFFECTOR_OPACITY:
			case ID_INSERT_EFFECTOR_SIZE:
			case ID_INSERT_DEFORMER_HUD:
			case ID_INSERT_DEFORMER_WAVE:
			case ID_INSERT_DEFORMER_ROTATE:
			case ID_INSERT_PIE:
			case ID_INSERT_RUNNER:
			case ID_INSERT_STYLE_PIE:
			case ID_INSERT_STYLE_PALETTE:
				InsertNewUIObject( LOWORD( wParam ) );
				break;

			case ID_EDIT_UNDO:
				{
					undo();
					break;
				}


			case ID_EDIT_DIFF:
				{
					if (gCurrentSelection.size() == 2)
					{
						UIBaseObject const * const lhsObject = gCurrentSelection.front();
						UIBaseObject const * const rhsObject = gCurrentSelection.back();
						
						if (lhsObject && rhsObject)
						{
							DiffDialogBox DiffObject;
							DiffObject.DiffObjects(gMainWindow, *lhsObject, *rhsObject);
						}
					}
					
					break;
				}
				
			case ID_EDIT_VISIBILITY:
				{
					UIBaseObject *SelectedObject = GetSelectedObjectInTreeControl();
					
					UIWidget *SelectedWidget = dynamic_cast<UIWidget *>(SelectedObject);
					if(SelectedWidget)
					{
						SelectedWidget->SetVisible(!SelectedWidget->IsVisible());
					}
					break;
				}

			case ID_EDIT_RENAME:
				{
					//If treeview window has focus
					HWND hwndFocus = GetFocus();
					
					if( hwndFocus == gObjectTree )
					{
						//Get selected item.  If it exists
						HTREEITEM			hSelection     = TreeView_GetSelection( gObjectTree );
						if(hSelection)
						{
							//Begin edit on it
							TreeView_EditLabel(gObjectTree, hSelection);
						}
					}					
					break;
				}

			case ID_EDIT_DUPLICATE:
				{
					UIBaseObject *SelectedObject = GetSelectedObjectInTreeControl();
					
					if( SelectedObject->GetParent() )
					{
						UIBaseObject::UIBaseObjectPointer NewObject(SelectedObject->DuplicateObject());
						
						SelectedObject->GetParent()->AddChild( NewObject );
						NewObject->Link ();

						LoadObjectStructureToTreeControl( gObjectTree, NewObject, TreeView_GetParent( gObjectTree, TreeView_GetSelection( gObjectTree )), TVI_LAST );
					}
					break;
				}
				
			case ID_EDIT_COPY:
				{
					HWND hwndFocus = GetFocus();
					
					if( hwndFocus == gObjectTree )
					{
						CopySelectedObjectInTreeToClipboard( gObjectTree );
					}
					else if( hwndFocus == gObjectInspector->GetTextControl() )
					{
						SendMessage( gObjectInspector->GetTextControl(), WM_COPY, 0, 0 );
					}
					
					break;
				}
			case ID_EDIT_CUT:
				{
					HWND hwndFocus = GetFocus();
					
					if( hwndFocus == gObjectTree )
					{
						UIBaseObject *o = GetSelectedObjectInTreeControl();
						
						if( o && (o != UIManager::gUIManager().GetRootPage()) )
						{
							CopySelectedObjectInTreeToClipboard( gObjectTree );
							DeleteSelectedObjectInTree( gObjectTree );
						}
					}
					else if( hwndFocus == gObjectInspector->GetTextControl() )
					{
						SendMessage( gObjectInspector->GetTextControl(), WM_CUT, 0, 0 );
					}
					
					break;
				}
			case ID_EDIT_PASTE:
				{
					HWND hwndFocus = GetFocus();
					
					if( hwndFocus == gObjectTree )
					{
						PasteObjectFromClipboard( gObjectTree );
					}
					else if( hwndFocus == gObjectInspector->GetTextControl() )
					{
						SendMessage( gObjectInspector->GetTextControl(), WM_PASTE, 0, 0 );
					}
				}
				break;

			case ID_EDIT_SORT:
				{
					UIBaseObject *o = GetSelectedObjectInTreeControl();
					
					if (o)
					{
						SortSelectedObject (gObjectTree);
					}
					else
					{
						MessageBeep(MB_OK);
					}
					break;
				}
				break;
			case ID_EDIT_DELETE:
				{
					UIBaseObject *o = GetSelectedObjectInTreeControl();
					
					if( o && (o != UIManager::gUIManager().GetRootPage()) )
					{
						if( MessageBox( hwndDlg, "Are you sure you want to delete this object?", gApplicationName, MB_YESNO ) == IDYES )
							DeleteSelectedObjectInTree( gObjectTree );
					}
					else
					{
						MessageBeep(MB_OK);
					}
					break;
				}
			case ID_EDIT_PURGEPROPERTY:
				{
					UINarrowString PropertyName;
					
					if( gObjectInspector->GetSelectedPropertyName( PropertyName ) )
						PurgeSelectedProperty( GetSelectedObjectInTreeControl(), PropertyName );
					
					break;
				}
			case ID_VIEW_RUN:
				gInVisualEditingMode = !gInVisualEditingMode;
				
				if( !gInVisualEditingMode )
					CheckDlgButton( gUIWindow, IDC_PLAY, BST_CHECKED );
				else
					CheckDlgButton( gUIWindow, IDC_PLAY, BST_UNCHECKED );
				
				break;
				
			case ID_VIEW_CURSOR:
				gDrawCursor = !gDrawCursor;
				
				if( gDrawCursor )
					CheckDlgButton( gUIWindow, IDC_DRAWCURSOR, BST_CHECKED );
				else
					CheckDlgButton( gUIWindow, IDC_DRAWCURSOR, BST_UNCHECKED );
				
				break;
				
			case ID_VIEW_HIGHLIGHT:
				gDrawHighlightRect = !gDrawHighlightRect;
				
				if( gDrawHighlightRect )
					CheckDlgButton( gUIWindow, IDC_DRAWHIGHLIGHT, BST_CHECKED );
				else
					CheckDlgButton( gUIWindow, IDC_DRAWHIGHLIGHT, BST_UNCHECKED );
				
				break;
				
			case ID_VIEW_GRID:
				gDrawGrid = !gDrawGrid;
				
				if( gDrawGrid )
					CheckDlgButton( gUIWindow, IDC_DRAWGRID, BST_CHECKED );
				else
					CheckDlgButton( gUIWindow, IDC_DRAWGRID, BST_UNCHECKED );
				
				break;
				
			case ID_VIEW_SHOWSHADERS:
				{
					WaitForSingleObject( gFrameRenderingMutex, INFINITE );
					g_showShaders = !gPrimaryDisplay->GetShowShaders ();
					gPrimaryDisplay->ShowShaders( g_showShaders);
					ReleaseMutex( gFrameRenderingMutex );
				}
				break;
				
			case ID_VIEW_DEFAULT_PROPERTIES:
				gDefaultObjectPropertiesManager.DisplayEditDialog( hwndDlg );
				break;
				
			case ID_VIEW_GRIDSETTINGS:
				DialogBox( GetModuleHandle(0), MAKEINTRESOURCE(IDD_GRIDSETTINGS), hwndDlg, GridSettingsDlgProc );
				
				if( gDrawGrid )
					CheckDlgButton( gUIWindow, IDC_DRAWGRID, BST_CHECKED );
				else
					CheckDlgButton( gUIWindow, IDC_DRAWGRID, BST_UNCHECKED );
				
				if( gSnapToGrid )
					CheckDlgButton( gUIWindow, IDC_SNAPTOGRID, BST_CHECKED );
				else
					CheckDlgButton( gUIWindow, IDC_SNAPTOGRID, BST_UNCHECKED );
				break;
				
			case ID_VIEW_HIGHLIGHTSETTINGS:
				DialogBox( GetModuleHandle(0), MAKEINTRESOURCE(IDD_HIGHLIGHTSETTINGS), hwndDlg, HighlightSettingsDlgProc );
				
				if( gDrawHighlightRect )
					CheckDlgButton( gUIWindow, IDC_DRAWHIGHLIGHT, BST_CHECKED );
				else
					CheckDlgButton( gUIWindow, IDC_DRAWHIGHLIGHT, BST_UNCHECKED );
				break;
				
			case IDC_CAPFRAMERATE:
				gLimitFrameRate = IsDlgButtonChecked( hwndDlg, IDC_CAPFRAMERATE ) != 0;
				break;
				
			case IDC_TOP:
			case IDC_UP:
			case IDC_DOWN:
			case IDC_BOTTOM:
				{
					HTREEITEM hSelectedItem = TreeView_GetSelection( gObjectTree );
					
					ProcessReorderButtonEvent( gObjectTree, hSelectedItem, LOWORD( wParam ) );
					SetFocus( gObjectTree );
					
					ClearDefPushButtonLook( hwndDlg, LOWORD(wParam) );
					EnableButtonControls( hwndDlg, GetSelectedObjectInTreeControl() );
					break;
				}
				
			case IDC_BUTTON_BACK:
				{
					std::string path;
					UIPage * const root = UIManager::gUIManager ().GetRootPage ();
					if (root && UIBuilderHistory::back (path))
					{
						UIBaseObject * const obj = path == "/root" ? root : root->GetObjectFromPath (path.c_str ());
						
						if (obj)
						{
							s_selectionPushOk = false;
							HTREEITEM	hItemToSelect = GetObjectInTreeControlFromHandle( gObjectTree, 0, obj );
							TreeView_SelectItem( gObjectTree, hItemToSelect );
						}
						EnableHistoryButtons ();
					}						
				}
				break;
				
			case IDC_BUTTON_FORWARD:
				{
					std::string path;
					UIPage * const root = UIManager::gUIManager ().GetRootPage ();
					if (root && UIBuilderHistory::forward (path))
					{
						UIBaseObject * const obj = path == "/root" ? root : root->GetObjectFromPath (path.c_str ());
						
						if (obj)
						{
							s_selectionPushOk = false;
							HTREEITEM	hItemToSelect = GetObjectInTreeControlFromHandle( gObjectTree, 0, obj );
							TreeView_SelectItem( gObjectTree, hItemToSelect );
						}
						EnableHistoryButtons ();
					}
				}
				break;
				
			case IDC_BUTTON_UP:
				{
					if (!gCurrentSelection.empty ())
					{
						UIBaseObject * const lastSelected = gCurrentSelection.front ();
						UIBaseObject * const parent = lastSelected->GetParent ();
						
						if (parent)
						{
							//								s_selectionPushOk = false;
							HTREEITEM	hItemToSelect = GetObjectInTreeControlFromHandle( gObjectTree, 0, parent );
							TreeView_SelectItem( gObjectTree, hItemToSelect );
						}
					}
				}
				break;
				
			case IDC_RESET:
				{
					UIPalette * palette = UIPalette::GetInstance();
					if(palette)
						palette->Reset();
				}
				break;

			case ID_HELP_MODIFICATIONLOG:
				DialogBox( GetModuleHandle(0), MAKEINTRESOURCE(IDD_MODIFICATION_LOG), hwndDlg, ModificationLogDlgProc );
				break;
			}
			return 0;
			
		case WM_NOTIFY:
			{
				if( wParam == IDC_OBJECTSTRUCTURE )
				{				
					static struct
					{
						UITypeID	Type;
						int       ImageID;
					}
					TypeToImageMap[] =
					{
						{  TUIDeformerHUD,       0 },
						{  TUIDeformerRotate,    0 },
						{  TUIDeformerWave,      0 },
						{  TUIText,              2 },
						{  TUITextbox,           3 },
						{  TUIAnimation,         4 },
						{  TUICheckbox,          5 },
						{  TUIEllipse,           4 },
						{  TUIImage,             6 },
						{  TUIButton,            9 },
						{  TUIProgressbar,      10 },
						{  TUIListbox,          11 },
						{  TUIList,             11 },
						{  TUIComboBox,         12 },
						{  TUIDropdownbox,      12 },
						{  TUINamespace,        13 },
						{  TUITooltipStyle,     17 },
						{  TUICursor,           18 },
						{  TUIDataSource,       19 },
						{  TUIData,             20 },
						{  TUITemplate,         21 },
						{  TUIImageFrame,       23 },    // Frame is a subclass of Fragment so check for it first
						{  TUIImageFragment,    22 },
						{  TUITextStyle,        24 },
						{  TUIFontCharacter,    25 },
						{  TUIClock,            26 },
						{  TUIGrid,             27 },
						{  TUIUnknown,          28 },
						{  TUISliderplane,      29 },
						{  TUITabSet,           30 },
						{  TUITabbedPane,       30 },
						{  TUIRectangleStyle,   31 },
						{  TUIPopupMenu,        32 },
						{  TUIVolumePage,       33 },
						{  TUIRadialMenu,       34 },
						{  TUICursorSet,        35 },
						{  TUITable,            36 },
						{  TUITableModelDefault,37 },
						{  TUIDataSourceContainer, 38 },
						{  TUITableHeader,       39 },
						{  TUIComposite,         40 },
						{  TUITreeView,          41 },
						{  TUIPie,               42 },
						{  TUIRunner,            43 },
						{  TUIPalette,           44 },
						{  TUIPage,              1 },    // some items are subclasses of pages, so check for it last
						{  TUIObject,            0 }     // Last because it's the fallback
					};
					
					LPNMHDR GenericHeader = (LPNMHDR)lParam;
					
					if( GenericHeader->code == TVN_GETDISPINFO )
					{
						LPNMTVDISPINFO hdr = (LPNMTVDISPINFO)lParam;
						UIBaseObject	*w = reinterpret_cast<UIBaseObject *>( hdr->item.lParam );
						
						if( hdr->item.mask & TVIF_IMAGE )
						{
							if( w->IsA( TUIScrollbar ) )
							{
								UISliderbarStyle * const pStyle = static_cast<UIScrollbar *>(w)->GetSliderbarStyle();
								
								if( pStyle )
								{
									if( pStyle->GetLayout() == UIStyle::L_horizontal )
										hdr->item.iImage = 7;
									else
										hdr->item.iImage = 8;
								}
								else
									hdr->item.iImage = 7;
							}
							else if( w->IsA( TUISliderbar ) )
							{
								UISliderbarStyle * const pStyle = static_cast<UISliderbar *>(w)->GetSliderbarStyle();
								
								if( pStyle )
								{
									if( pStyle->GetLayout() == UIStyle::L_horizontal )
										hdr->item.iImage = 15;
									else
										hdr->item.iImage = 16;
								}
								else
									hdr->item.iImage = 15;
							}
							else
							{
								for( int i = 0; i < sizeof( TypeToImageMap ) / sizeof( TypeToImageMap[0] ); ++i )
								{
									if( w->IsA( TypeToImageMap[i].Type ) )
									{
										hdr->item.iImage = TypeToImageMap[i].ImageID;
										break;
									}
								}
							}
						}
						
						if( hdr->item.mask & TVIF_SELECTEDIMAGE )
						{
							if( w->IsA( TUIScrollbar ) )
							{
								UISliderbarStyle * const pStyle = static_cast<UIScrollbar *>(w)->GetSliderbarStyle();
								
								if( pStyle )
								{
									if( pStyle->GetLayout() == UIStyle::L_horizontal )
										hdr->item.iSelectedImage = 7;
									else
										hdr->item.iSelectedImage = 8;
								}
								else
									hdr->item.iSelectedImage = 7;
							}
							else if( w->IsA( TUISliderbar ) )
							{
								UISliderbarStyle * const pStyle = static_cast<UISliderbar *>(w)->GetSliderbarStyle();
								
								if( pStyle )
								{
									if( pStyle->GetLayout() == UIStyle::L_horizontal )
										hdr->item.iSelectedImage = 15;
									else
										hdr->item.iSelectedImage = 16;
								}
								else
									hdr->item.iSelectedImage = 15;
							}
							else
							{
								for( int i = 0; i < sizeof( TypeToImageMap ) / sizeof( TypeToImageMap[0] ); ++i )
								{
									if( w->IsA( TypeToImageMap[i].Type ) )
									{
										hdr->item.iSelectedImage = TypeToImageMap[i].ImageID;
										break;
									}
								}
							}
						}
						
						if( hdr->item.mask & TVIF_CHILDREN )
						{						
							if( w->GetChildCount() > 0 )
								hdr->item.cChildren = 1;
							else
								hdr->item.cChildren = 0;
						}
						
						if( hdr->item.mask & TVIF_TEXT )
						{
							if( w->GetName().empty() )
								strncpy( hdr->item.pszText, "Unnamed Object", hdr->item.cchTextMax );
							else
								strncpy( hdr->item.pszText, w->GetName().c_str(), hdr->item.cchTextMax );
						}
				}
				else if( GenericHeader->code == TVN_SELCHANGED )
				{
					LPNMTREEVIEW  hdr							= (LPNMTREEVIEW)lParam;
					UIBaseObject *SelectedObject	= reinterpret_cast<UIBaseObject *>( hdr->itemNew.lParam );
					
					if( SelectedObject )
					{
						if( GetAsyncKeyState( VK_SHIFT ) & 0x80000000 )
							AddToSelection( SelectedObject, true );
						else if( GetAsyncKeyState( VK_CONTROL ) & 0x80000000 )
							RemoveFromSelection( SelectedObject );
						else
							SetSelection( SelectedObject, s_selectionPushOk );
					}
					else
						SetSelection( 0, true );
					
					s_selectionPushOk = true;
					
					gObjectInspector->SetObject( SelectedObject );
					EnableButtonControls( hwndDlg, SelectedObject );
				}
				else if (GenericHeader->code == TVN_BEGINLABELEDIT)
				{
					HWND hEdit = TreeView_GetEditControl(gObjectTree);
					DWORD style = GetWindowLong(hEdit, GWL_STYLE);
					style |= ES_WANTRETURN;
					SetWindowLong(hEdit, GWL_STYLE, style);
					
					mOldLabelEditWindowProc = (WNDPROC)GetWindowLong( hEdit, GWL_WNDPROC );
					SetWindowLong( hEdit, GWL_WNDPROC, (long)LabelEditWindowProc );
				}
				else if (GenericHeader->code == TVN_ENDLABELEDIT )
				{
					LPNMTVDISPINFO ptvdi = (LPNMTVDISPINFO) lParam;
					if (ptvdi->item.pszText)
					{
						RenameSelectedObjectInTree( gObjectTree, ptvdi->item.pszText);
						return true;
					}
					
					return false;
				}
				else if( GenericHeader->code == TVN_KEYDOWN )
				{
					LPNMTVKEYDOWN pKeyDown = (LPNMTVKEYDOWN)lParam;
					
					if( (pKeyDown->wVKey == VK_DELETE) || (pKeyDown->wVKey == VK_BACK) )
						SendMessage( hwndDlg, WM_COMMAND, ID_EDIT_DELETE, 0 );
					else if( (pKeyDown->wVKey != VK_UP) && (pKeyDown->wVKey != VK_DOWN) && 
						(pKeyDown->wVKey != VK_HOME) && (pKeyDown->wVKey != VK_END) &&
						(pKeyDown->wVKey != VK_NEXT) && (pKeyDown->wVKey != VK_PRIOR) &&
						(pKeyDown->wVKey != VK_CONTROL))
					{
						MSG msg;
						
						while( PeekMessage( &msg, gObjectTree, WM_KEYFIRST, WM_KEYLAST, PM_REMOVE ) )
						{}
						
						gObjectInspector->SendKeyDown( pKeyDown->wVKey );
					}
					
					// Don't search in the tree!
					return 1;
				}
			}
			return 0;
		}
		case WM_TIMER:
			{
				static DWORD LastCall = GetTickCount();
				char s[128];
				
				sprintf( s, "%d", gTriangleCount );
				SetWindowText( GetDlgItem( hwndDlg, IDC_TRIANGLECOUNT ), s );
				
				sprintf( s, "%d", gFlushCount );
				SetWindowText( GetDlgItem( hwndDlg, IDC_FLUSHCOUNT ), s );
				
				if( (GetTickCount() - LastCall) >= 500 )
				{
					sprintf( s, "%.2f", 1000.0 * (float)gFrameCount / (float)(GetTickCount() - LastCall) );
					SetWindowText( GetDlgItem( hwndDlg, IDC_FRAMERATE ), s );
					gFrameCount = 0;
					LastCall = GetTickCount();
				}
				return 0;
			}
			
		case WM_CLOSE:
			return SendMessage( hwndDlg, WM_COMMAND, ID_FILE_EXIT, 0 );
			
		case WM_DESTROY:
			{
				clearUndoHistory();

				ClearClipboard();
				
				UnhookWindowsHookEx( gTooltipHook );
				gTooltipHook = 0;
				KillTimer( hwndDlg, 0 );
				
				delete gObjectInspector;
				gObjectInspector = 0;
				
				UnloadObjects( hwndDlg );
				
				SetCurrentDirectory( gInitialDirectory.c_str() );
				FILE *fp = fopen("defaults.cfg", "wb");
				
				if (fp == 0)
				{
					MessageBox(0, "Unable to write defaults.cfg.", gApplicationName, MB_OK );
				}
				else
				{
					gDefaultObjectPropertiesManager.SaveTo( fp );
					fclose( fp );
				}
				
				SavePreferencesToRegistry ();
				SaveMRUListToRegistry();
				
				PostQuitMessage(0);
				return 0;
			}
		default:
			return 0;
	}
} 

//-----------------------------------------------------------------

int __stdcall WinMain( HINSTANCE hInst, HINSTANCE hPrevInst, LPSTR lpCmdLine, int nCmdShow )
{
	UI_UNREF (hPrevInst);
	UI_UNREF (lpCmdLine);
	UI_UNREF (nCmdShow);

	//-- install the ui system
	SetupUi::Data setupUiData;
	SetupUi::install (setupUiData);

	INITCOMMONCONTROLSEX icc = {sizeof(icc), ICC_WIN95_CLASSES};
	InitCommonControlsEx( &icc );	
	HACCEL theAccelerator = LoadAccelerators( hInst, MAKEINTRESOURCE( IDR_MAINACCELERATOR ) );

	UIStandardCanvasFactory<DDSCanvasGenerator>			theDDSCanvasFactory;
	
	UIManager &theManager = UIManager::gUIManager();
	theManager.setUIIMEManager(new UINullIMEManager);

	UICursorInterfaceDefault* cursorInterface = new UICursorInterfaceDefault;

	theManager.SetCursorInterface (cursorInterface);

	FileLocator::gFileLocator ();

	theManager.AddCanvasFactory( &theDDSCanvasFactory );

	CreateDialog( hInst, MAKEINTRESOURCE( IDD_MAINWINDOW ), 0, (DLGPROC)MainWindowProc );

	theManager.SetSoundCanvas ( new SimpleSoundCanvas ());

	MSG		 theMessage;
	while( GetMessage( &theMessage, 0, 0, 0 ) )
	{
		WaitForSingleObject( gFrameRenderingMutex, INFINITE );

		if(!TranslateAccelerator( gMainWindow, theAccelerator, &theMessage ) )
		{
			if( !IsDialogMessage( gMainWindow, &theMessage ) && !IsDialogMessage( gUIWindow, &theMessage ) )
			{
				TranslateMessage( &theMessage );
				DispatchMessage( &theMessage );
			}
		}

		ReleaseMutex( gFrameRenderingMutex );
	}

	UISoundCanvas * sc = UIManager::gUIManager ().GetSoundCanvas ();
	UIManager::gUIManager ().SetSoundCanvas (0);
	delete sc;

	theManager.SetCursorInterface (0);
	delete cursorInterface;

	UIManager::ExplicitDestroy();

	FileLocator::ExplicitDestroy ();

	if( gPrimaryDisplay )
	{
		// We better have closed all references to the display
//		if( gPrimaryDisplay->GetRefCount() != 1 )
//			MessageBox(0, "There is a handle leak: not all references to the primary display were closed.", gApplicationName, MB_OK );

		gPrimaryDisplay->Detach();
		gPrimaryDisplay = 0;
	}

	//-- remove the ui system
	SetupUi::remove ();

	return theMessage.wParam;
}

//-----------------------------------------------------------------

void DeleteObjectInTree(HTREEITEM hSelectedItem)
{
	HTREEITEM			hNextSelection;
	UIPage *RootPage = UIManager::gUIManager().GetRootPage();
	
	if( !hSelectedItem )
		return;
	
	UIBaseObject *theObjectToDelete = GetObjectFromNode( hSelectedItem );
	
	if( !theObjectToDelete || (theObjectToDelete == RootPage) )
		return;

	int allowedReferenceCount = 2; // one for the parent, one for being in the selection
	if(theObjectToDelete == gObjectInspector->GetObject())
		allowedReferenceCount++;

	//-- see if anything has an outstanding reference to this object
	if (theObjectToDelete->GetRefCount () > allowedReferenceCount)
	{
		char buf [512];
		_snprintf(buf, sizeof (buf) - 1, "%s has %d extra outstanding references.\nReally delete it?", theObjectToDelete->GetName().c_str(), theObjectToDelete->GetRefCount() - allowedReferenceCount);
		if (MessageBox( gMainWindow, buf, gApplicationName, MB_YESNO ) != IDYES)
			return;
	}
	
	hNextSelection = TreeView_GetNextSibling( gObjectTree, hSelectedItem );
	
	if( !hNextSelection )
	{
		hNextSelection = TreeView_GetPrevSibling( gObjectTree, hSelectedItem );
		
		//-- don't select parents
	}
	
	RemoveFromSelection( theObjectToDelete );
	allowedReferenceCount--;
	
	TreeView_DeleteItem( gObjectTree, hSelectedItem );
	WaitForSingleObject( gFrameRenderingMutex, INFINITE );
	
	UIBaseObject *theParent = theObjectToDelete->GetParent();
	
	theObjectToDelete->Attach( 0 );
	
	if( theParent )
		theParent->RemoveChild( theObjectToDelete );
	allowedReferenceCount--;

	//-- if anything has an outstanding reference to this object, relink the whole tree to be safe
	if (theObjectToDelete->GetRefCount () > allowedReferenceCount)
	{
		//-- set the parent so we can unlink properly
		theObjectToDelete->SetParent (theParent);
		
		//-- don't link the root page's children from here
		RootPage->UIWidget::Link();
		
		if (theObjectToDelete->GetRefCount () > 1)
		{
			UIBaseObject::UIObjectList olist;
			RootPage->GetChildren (olist);
			
			for (UIBaseObject::UIObjectList::const_iterator it = olist.begin (); it != olist.end () && theObjectToDelete->GetRefCount () > 1; ++it)
			{
				UIBaseObject * const child = *it;
				child->Link ();
			}
		}
		theObjectToDelete->SetParent (0);
	}
	
	if( theObjectToDelete->IsA( TUIPage ) && (theObjectToDelete->GetParent() == RootPage) )
		LoadTopLevelObjectsToTabControl();
	
	TreeView_SelectItem( gObjectTree, hNextSelection );
	
	if( !hNextSelection )
		gObjectInspector->SetObject( 0 );
	else
	{
		TVITEM tvi = {0};
		
		tvi.mask = TVIF_HANDLE;
		tvi.hItem = hNextSelection;
		
		TreeView_GetItem( gObjectTree, &tvi );
		
		gObjectInspector->SetObject( reinterpret_cast<UIBaseObject *>( tvi.lParam ) );
		TreeView_EnsureVisible( gObjectTree, hNextSelection );
	}
	
	// Do this last
	theObjectToDelete->Detach(0);
	ReleaseMutex( gFrameRenderingMutex );
}

//-----------------------------------------------------------------

// To make this work on delete, the parent page needs to be rebuilt.

void RebuildTreeView(UIBaseObject * selectedObject)
{
	///////////////////////////////////////
	// Brute force now.
	TreeView_DeleteAllItems(gObjectTree);
	LoadObjectStructureToTreeControl(gObjectTree, UIManager::gUIManager().GetRootPage(), 0, TVI_LAST);

	if(!selectedObject)
		selectedObject = gObjectInspector->GetObject();
	if(selectedObject)
	{	
		TreeItem const * const newItem = getItemInfo(selectedObject);
		if(newItem)
		{		
			TreeView_SelectItem(gObjectTree, newItem->mItem);
			TreeView_EnsureVisible(gObjectTree, newItem->mItem);
			TreeView_Expand(gObjectTree, newItem->mItem, TVE_EXPAND);
		}

		gObjectInspector->SetObject(selectedObject);
	}
}

// Gets the currently selected file, and checks it out
void CheckOutSelectedFile()
{
	UIBaseObject *obj = gObjectInspector->GetObject();
	if(!obj)
		return;
	UIString sourcePath;
	if (UIManager::gUIManager().GetRootPage() == obj) 
	{
		sourcePath = Unicode::narrowToWide("ui_root.ui");
	}
	else
	{
		obj->GetProperty(UIBaseObject::PropertyName::SourceFile, sourcePath);
	}
	
	char cmdLine[1024];
	sprintf(cmdLine, "p4 edit %s\n\n", Unicode::wideToNarrow(sourcePath).c_str());
	system(cmdLine);
}