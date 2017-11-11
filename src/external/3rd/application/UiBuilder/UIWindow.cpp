#include "FirstUiBuilder.h"

#include "UIBuilderHistory.h"
#include "UIDirect3DPrimaryCanvas.h"
#include "UIManager.h"
#include "UIPage.h"
#include "UIScriptEngine.h"
#include "uicanvas.h"
#include "uimessage.h"
#include "uipalette.h"
#include "UIUndo.h"
#include "UIWidget.h"
#include "UIUtils.h"

#include <cmath>
#include <commctrl.h>
#include <process.h>
#include <windowsx.h>

#include "objectinspector.h"
#include "resource.h"

static const int         UITimerID             = 100;

extern bool              g_showShaders;

HWND                     gUIWindow             = 0;
HWND                     gCanvasWindow         = 0;
HWND                     gTabControlWindow     = 0;

HANDLE                   gEngineThread         = 0;
HANDLE                   gStopEvent            = 0;
HANDLE                   gFrameRenderingMutex  = 0;

long                     gTriangleCount        = 0;
long                     gFlushCount           = 0;
long                     gFrameCount           = 0;

bool                     gButtonDown;
UIPoint                  gMouseDownPoint;

UIBaseObject::UISmartObjectList gCurrentSelection;

bool                     gMoving;
UIPoint                  gOriginalLocation;
bool                     gSizing;
UISize                   gOriginalSize;

UIRect                   gTabControlPadding;
UIPage                  *gCurrentlySelectedPage = 0;

bool                     gDrawHighlightRect   = true;
bool                     gLimitFrameRate      = true;
bool                     gInVisualEditingMode = false;
UIColor                  gHighlightOutlineColor(0xff,0,0,0xaa);
UIColor                  gHighlightFillColor(0xff,0,0,0);

bool                     gSizeUp               = false;
bool                     gSizeDown             = false;
bool                     gSizeLeft             = false;
bool                     gSizeRight            = false;

bool                     gNoChangeSelection    = false;

bool                     gDrawGrid             = false;
bool                     gSnapToGrid           = false;
UIColor                  gGridColor(255,255,255,32);
unsigned long            gXGridStep            = 10;
unsigned long            gYGridStep            = 10;
unsigned long            gGridMajorTicks       = 10;

bool                     gDrawCursor           = true;

const long               LineWidth             = 1;
const long               HandleSize            = 4;
const double             MoveThreshold         = 5.0;
const double             SizeThreshold         = 3.0;

extern HWND                      gObjectTree;
extern HWND                      gMainWindow;
extern HWND                      gTooltip;
extern ObjectInspector          *gObjectInspector;
extern UIDirect3DPrimaryCanvas  *gPrimaryDisplay;

const char *gVisualEditLockPropertyName = "VisualEditLock";

void AddTooltipFromControlID( HWND TooltipWindow, HWND ParentWindow, UINT ControlID, char *Tooltip );
void ClearDefPushButtonLook( HWND hwndDlg, UINT nControlID );
void CopySelectedObjectInTreeToClipboard( HWND hTree );
void PasteObjectFromClipboard( HWND hTree );
void DeleteSelectedObjectInTree( HWND hTree );
void EnableHistoryButtons();


// Undo Stuff
void RebuildTreeView(UIBaseObject * newObject);

void recordUndo(UIBaseObject * const object, bool force = false);
void recordUndo(UIBaseObject * const object, UILowerString property, UIString oldValue, UIString newValue, bool force = false);
void undo();

typedef std::vector<UIUndo> UndoQueue;
UndoQueue s_UndoQueue;
bool volatile s_UndoReady = false;
bool volatile s_UndoQueueEmpty = true;
extern HMENU gMenu;
int const MaxUndoSteps = 128;
// End Undo Stuff

enum Align
{
	LeftAlignment,
	HCenterAlignment,
	RightAlignment,
	TopAlignment,
	VCenterAlignment,
	BottomAlignment,
};

void UIThread( void * );

void AlignSelection( Align NewAlignment )
{
	if( gCurrentSelection.size() < 2 )
		return;
	
	UIRect ReferenceRect;	
	bool   bFirst = true;
	bool const forceUndo = s_UndoReady;
	
	for( UIBaseObject::UISmartObjectList::iterator i = gCurrentSelection.begin(); i != gCurrentSelection.end(); ++i )
	{
		if( (*i)->IsA( TUIWidget ) )
		{
			UIWidget * w = UI_ASOBJECT(UIWidget, *i );
			
			if( bFirst )
			{
				w->GetWorldRect( ReferenceRect );
				bFirst = false;
			}
			else
			{
				UIRect  ControlRect;
				UIPoint MovementRequired(0,0);
				
				w->GetWorldRect( ControlRect );
				
				switch( NewAlignment )
				{
				case LeftAlignment:
					MovementRequired.x = ReferenceRect.left - ControlRect.left;
					break;
				case HCenterAlignment:
					MovementRequired.x = (ReferenceRect.right + ReferenceRect.left ) / 2 - 
						(ControlRect.right + ControlRect.left ) / 2;
					break;
				case RightAlignment:
					MovementRequired.x = ReferenceRect.right - ControlRect.right;
					break;
				case TopAlignment:
					MovementRequired.y = ReferenceRect.top - ControlRect.top;
					break;
				case VCenterAlignment:
					MovementRequired.y = (ReferenceRect.bottom + ReferenceRect.top ) / 2 - 
						(ControlRect.bottom + ControlRect.top ) / 2;
					break;
				case BottomAlignment:
					MovementRequired.y = ReferenceRect.bottom - ControlRect.bottom;
					break;				
				}
				
				recordUndo(w, forceUndo);
				w->SetLocation( w->GetLocation() + MovementRequired );
			}
		}
	}
}

void SizeSelection( bool MatchWidth, bool MatchHeight )
{
	if( gCurrentSelection.size() < 2 )
		return;
	
	UISize ReferenceSize;	
	bool   bFirst = true;
	bool const forceUndo = s_UndoReady;
	
	for( UIBaseObject::UISmartObjectList::iterator i = gCurrentSelection.begin(); i != gCurrentSelection.end(); ++i )
	{
		if( (*i)->IsA( TUIWidget ) )
		{
			UIWidget *w = UI_ASOBJECT(UIWidget, *i );
			
			if( bFirst )
			{
				ReferenceSize = w->GetSize();
				bFirst = false;
			}
			else
			{
				UISize ControlSize (w->GetSize());
				
				if( MatchWidth )
					ControlSize.x = ReferenceSize.x;
				
				if( MatchHeight )
					ControlSize.y = ReferenceSize.y;
				
				recordUndo(w, forceUndo);
				w->SetSize( ControlSize );
			}
		}
	}
}

void UpdateAlignmentControlEnabledState( void )
{
	BOOL  NewState;
	
	if( gCurrentSelection.size() >=2 )
		NewState = TRUE;
	else
		NewState = FALSE;
	
	EnableWindow( GetDlgItem(gUIWindow,IDC_ALIGNLEFT), NewState );
	EnableWindow( GetDlgItem(gUIWindow,IDC_ALIGNHCENTER), NewState );
	EnableWindow( GetDlgItem(gUIWindow,IDC_ALIGNRIGHT), NewState );
	EnableWindow( GetDlgItem(gUIWindow,IDC_ALIGNTOP), NewState );
	EnableWindow( GetDlgItem(gUIWindow,IDC_ALIGNVCENTER), NewState );
	EnableWindow( GetDlgItem(gUIWindow,IDC_ALIGNBOTTOM), NewState );
	
	EnableWindow( GetDlgItem(gUIWindow,IDC_SIZEWIDTH), NewState );
	EnableWindow( GetDlgItem(gUIWindow,IDC_SIZEHEIGHT), NewState );	
}

//-----------------------------------------------------------------

UIWidget *GetObjectInSelection( const UIPoint &PointToTest )
{
	for( UIBaseObject::UISmartObjectList::iterator i = gCurrentSelection.begin(); i != gCurrentSelection.end(); ++i )
	{
		if( (*i)->IsA( TUIWidget ) )
		{
			UIWidget *w = UI_ASOBJECT(UIWidget,  *i);
			
			UIRect ObjectRect;
			
			w->GetWorldRect( ObjectRect );
			if( ObjectRect.ContainsPoint( PointToTest ) )
				return w;
		}
	}
	return 0;
}

//-----------------------------------------------------------------


bool SelectionContains( const UIBaseObject *ObjectToQuery )
{
	return std::find (gCurrentSelection.begin(),gCurrentSelection.end(), ObjectToQuery) != gCurrentSelection.end ();
}

//-----------------------------------------------------------------

void SetSelection( UIBaseObject *NewSelection, bool pushHistory )
{
	if( gNoChangeSelection )
		return;
	
	WaitForSingleObject( gFrameRenderingMutex, INFINITE );
	
	const bool skip = !gCurrentSelection.empty () && gCurrentSelection.front () == static_cast<const UIBaseObject *>(NewSelection);
	
	if (!skip)
	{
		gCurrentSelection.clear();
		
		if( NewSelection)
		{
			gCurrentSelection.push_front(UIBaseObject::UIBaseObjectPointer(NewSelection));
			
			if( NewSelection->IsA( TUIWidget ) )
			{
				UIWidget *w = static_cast<UIWidget *>( NewSelection );
				
				gOriginalLocation = w->GetLocation();
				gOriginalSize     = w->GetSize();
			}
			
			if (pushHistory)
				UIBuilderHistory::pushNode (NewSelection->GetFullPath ());
			
			EnableHistoryButtons ();
		}
	}

	UpdateAlignmentControlEnabledState();
	
	ReleaseMutex( gFrameRenderingMutex );
}

//-----------------------------------------------------------------

bool RemoveFromSelection( const UIBaseObject *ObjectToRemove )
{
	bool selectionChanged = false;

	if( gNoChangeSelection )
		return selectionChanged;
	
	if( ObjectToRemove )
	{
		WaitForSingleObject( gFrameRenderingMutex, INFINITE );
		
		for( UIBaseObject::UISmartObjectList::iterator i = gCurrentSelection.begin(); i != gCurrentSelection.end(); ++i )
		{
			if( *i == ObjectToRemove )
			{
				gCurrentSelection.erase( i );
				selectionChanged = true;
				break;
			}
		}
		UpdateAlignmentControlEnabledState();
		
		ReleaseMutex( gFrameRenderingMutex );
	}

	return selectionChanged;
}

//-----------------------------------------------------------------

void AddToSelection( UIBaseObject *ObjectToAdd, bool pushHistory )
{
	if( gNoChangeSelection )
		return;
	
	if( ObjectToAdd )
	{
		WaitForSingleObject( gFrameRenderingMutex, INFINITE );
		
		RemoveFromSelection( ObjectToAdd );
		gCurrentSelection.push_front(UIBaseObject::UIBaseObjectPointer(ObjectToAdd));
		UpdateAlignmentControlEnabledState();
		
		if (pushHistory)
			UIBuilderHistory::pushNode (ObjectToAdd->GetFullPath ());

		EnableHistoryButtons ();

		ReleaseMutex( gFrameRenderingMutex );
	}
}

//-----------------------------------------------------------------

HTREEITEM GetObjectInTreeControlFromHandle( HWND hTree, HTREEITEM hParentItem, UIBaseObject *o )
{
	HTREEITEM hItem;
	
	if( hParentItem )
		hItem = TreeView_GetChild(hTree, hParentItem);
	else
		hItem = TreeView_GetRoot(hTree);
	
	while( hItem )
	{
		TVITEM tvi;
		
		tvi.mask  = TVIF_HANDLE | TVIF_PARAM;
		tvi.hItem = hItem;
		
		TreeView_GetItem( hTree, &tvi );
		
		if( reinterpret_cast<UIBaseObject *>( tvi.lParam ) == o )
			return hItem;
		
		HTREEITEM hChildFound = GetObjectInTreeControlFromHandle( gObjectTree, hItem, o );
		
		if( hChildFound )
			return hChildFound;
		
		hItem = TreeView_GetNextSibling( gObjectTree, hItem );
	}
	return 0;
}

void SizeWindowToCurrentPageSelection( void )
{
	int iCurrentTab = TabCtrl_GetCurSel( gTabControlWindow );
	
	if( iCurrentTab >= 0 )
	{
		TCITEM tci;
		
		tci.mask = TCIF_PARAM;
		TabCtrl_GetItem( gTabControlWindow, iCurrentTab, &tci );		
		assert(tci.lParam);
		
		if( gCurrentlySelectedPage )
			gCurrentlySelectedPage->ForceVisible( false );
		
		gCurrentlySelectedPage = reinterpret_cast<UIPage *>( tci.lParam );
		const UISize & s = gCurrentlySelectedPage->GetSize();
		gCurrentlySelectedPage->ForceVisible( true );
		
		SetWindowPos( gCanvasWindow, 0, 0, 0, s.x, s.y, SWP_NOMOVE | SWP_NOZORDER | SWP_NOACTIVATE );
		
		RECT  rc;
		rc.left = 0;
		rc.top  = 0;
		rc.right = s.x;
		rc.bottom = s.y;
		TabCtrl_AdjustRect( gTabControlWindow, TRUE, &rc );
		
		SetWindowPos( gTabControlWindow, 0, 0, 0, rc.right - rc.left, rc.bottom - rc.top, SWP_NOMOVE | SWP_NOZORDER | SWP_NOACTIVATE );
		
		rc.left   -= gTabControlPadding.left;
		rc.top    -= gTabControlPadding.top;
		rc.right  += gTabControlPadding.right;
		rc.bottom += gTabControlPadding.bottom;
		
		AdjustWindowRectEx( &rc, GetWindowLong( gUIWindow, GWL_STYLE ), FALSE, GetWindowLong( gUIWindow, GWL_EXSTYLE ) );
		SetWindowPos( gUIWindow, 0, 0, 0, rc.right - rc.left, rc.bottom - rc.top, SWP_NOMOVE | SWP_NOZORDER | SWP_NOACTIVATE );
	}
}

void RefreshContents( void )
{
	WaitForSingleObject( gFrameRenderingMutex, INFINITE );
	UIManager::gUIManager().RefreshGraphics();
	ReleaseMutex( gFrameRenderingMutex );
}

LRESULT CALLBACK CanvasWindowProc( HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam )
{	
	UIMessage UIMsg;	
	
	// Convert lParam to local coordinate space if needed
	if( gCurrentlySelectedPage )
	{
		switch( uMsg )
		{
		case WM_MOUSEMOVE:
		case WM_LBUTTONDOWN:
		case WM_LBUTTONUP:
		case WM_MBUTTONDOWN:
		case WM_MBUTTONUP:
		case WM_RBUTTONDOWN:
		case WM_RBUTTONUP:
			{					
				UIPoint CanvasTranslation = gCurrentlySelectedPage->GetLocation();
				POINTS  MouseCoord        = MAKEPOINTS( lParam );
				
				const long x = MouseCoord.x + CanvasTranslation.x;
				const long y = MouseCoord.y + CanvasTranslation.y;
				
				lParam = MAKELPARAM( x, y );
				break;
			}
		}
	}
	
	// First stage: Handle ONLY things that the rendering thread is not
	// in contention for.  Return is prefered to break unless you want the
	// second stage to process the message as well
	switch( uMsg )
	{
	case WM_CREATE:
		{
			gFrameRenderingMutex = CreateMutex( 0, 0, 0 );
			gStopEvent           = CreateEvent( 0, 1, 0, 0 );
			
			if( !InitializeCanvasSystem( hwnd ) )
			{
				MessageBox( NULL, "Could not initialize canvas system", NULL, MB_OK );
				return -1;
			}
			
			RECT rc;
			
			// Create the primary display before we invoke the loader so that the global gPrimaryDisplay
			// is set up - so that we can make the textures with the correct pixelformat.
			GetClientRect( hwnd, &rc );
			gPrimaryDisplay = new UIDirect3DPrimaryCanvas( UISize( rc.right, rc.bottom ), hwnd, false );
			gPrimaryDisplay->ShowShaders (g_showShaders);
			gPrimaryDisplay->Attach( 0 );
			
			UIManager::gUIManager().SetScriptEngine( new UIScriptEngine );
			
			gEngineThread = (HANDLE)_beginthread( UIThread, 0, 0 );
			return DefWindowProc( hwnd, uMsg, wParam, lParam );
		}
		
	case WM_SIZE:
		WaitForSingleObject( gFrameRenderingMutex, INFINITE );
		gPrimaryDisplay->SetSize( UISize( LOWORD(lParam), HIWORD(lParam) ) );
		ReleaseMutex( gFrameRenderingMutex );
		
		InvalidateRect( hwnd, NULL, FALSE );
		return DefWindowProc( hwnd, uMsg, wParam, lParam );
		
	case WM_PAINT:
		{
			PAINTSTRUCT ps;
			
			BeginPaint( hwnd, &ps );
			EndPaint( hwnd, &ps );
			return 0;
		}
		
	case WM_DESTROY:
		{
			SetEvent( gStopEvent );
			WaitForSingleObject( gEngineThread, INFINITE );
			ShutdownCanvasSystem( hwnd );
			
			CloseHandle( gFrameRenderingMutex );
			CloseHandle( gStopEvent );
			
			gFrameRenderingMutex = 0;
			gStopEvent = 0;
			
			delete UIManager::gUIManager().GetScriptEngine();
			UIManager::gUIManager().SetScriptEngine(0);
			
			return DefWindowProc( hwnd, uMsg, wParam, lParam );
		}
		
	case WM_LBUTTONDOWN:
	case WM_MBUTTONDOWN:
	case WM_RBUTTONDOWN:
		SetCapture( hwnd );
		break;	// Pass through to second stage
	case WM_LBUTTONDBLCLK:
	case WM_MBUTTONDBLCLK:
	case WM_RBUTTONDBLCLK:
		{			
			if (gInVisualEditingMode)
			{
				UIPage * const RootPage = UIManager::gUIManager ().GetRootPage ();
				
				if (RootPage)
				{
					UIWidget * ClickedWidget = RootPage->GetWidgetFromPoint( gMouseDownPoint, true );

					if (!ClickedWidget)
						ClickedWidget = RootPage->GetWidgetFromPoint( gMouseDownPoint, false );
					else
					{
						const UIPoint & pt = gMouseDownPoint - ClickedWidget->GetWorldLocation ();

						UIWidget * const ClickedWidgetNoInput = ClickedWidget->GetWidgetFromPoint( pt, false );

						UIBaseObject * p = ClickedWidgetNoInput;

						while (p)
						{
							if (p == ClickedWidget)
							{
								ClickedWidget = ClickedWidgetNoInput;
								break;
							}

							p = p->GetParent ();
						}
					}
					
					//-- don't allow selection of transients
					while (ClickedWidget && ClickedWidget->IsTransient ())
						ClickedWidget = ClickedWidget->GetParentWidget ();
					
					UIWidget * const ObjectInSelection = GetObjectInSelection( gMouseDownPoint );
					
					// Drill down
					while( ClickedWidget && (ClickedWidget->GetParent() != ObjectInSelection) )
					{
						UIWidget * const SearchObject = ClickedWidget->GetParentWidget ();
						
						if( SearchObject )
						{
							if( ClickedWidget != SearchObject->GetParent() )
								ClickedWidget = SearchObject;
						}
						else
							ClickedWidget = 0;
					}
					
					if (ClickedWidget)
					{
						SetSelection( ClickedWidget, true);
						
						HTREEITEM	hItemToSelect = GetObjectInTreeControlFromHandle( gObjectTree, 0, ClickedWidget );
						
						gNoChangeSelection = true;
						TreeView_SelectItem( gObjectTree, hItemToSelect );
						gNoChangeSelection = false;
					}
				}
			}
		}

		break;	// Pass through to second stage
								
	case WM_LBUTTONUP:
	case WM_MBUTTONUP:
	case WM_RBUTTONUP:
		if( GetCapture() == hwnd )
			ReleaseCapture();
		
		break;	// Pass through to second stage
		
	case WM_KEYDOWN:
	case WM_KEYUP:
	case WM_CHAR:	
		if( gInVisualEditingMode )
		{
			RECT  rcClient;
			POINT CursorPosition;
			
			GetCursorPos( &CursorPosition );
			GetWindowRect( hwnd, &rcClient );
			
			if( PtInRect( &rcClient, CursorPosition ) )
			{
				HCURSOR hCursor;
				
				if( GetAsyncKeyState( VK_SHIFT ) & 0x80000000 )
					hCursor = LoadCursor( GetModuleHandle(0), MAKEINTRESOURCE(IDC_ADDTOSEL) );
				else if( GetAsyncKeyState( VK_CONTROL ) & 0x80000000 )
					hCursor = LoadCursor( GetModuleHandle(0), MAKEINTRESOURCE(IDC_REMOVEFROMSEL) );
				else
					hCursor = LoadCursor( 0, IDC_ARROW );
				
				SetCursor( hCursor );
			}
		}
		break;	// Pass through to second stage
		
	case WM_MOUSEMOVE:
		{
			POINTS  MousePoint = MAKEPOINTS( lParam );
			HCURSOR hCursor    = 0;
			
			if( gInVisualEditingMode )
			{
				UIWidget * const SelectedObject = GetObjectInSelection( UIPoint( MousePoint.x, MousePoint.y ) );
				
				if( !gButtonDown )
				{
					gSizeUp    = false;
					gSizeDown  = false;
					gSizeLeft  = false;
					gSizeRight = false;

					if( SelectedObject )
					{
						UIRect HighlightRect;
						long	 HHandleLoc;
						long   VHandleLoc;
						
						SelectedObject->GetWorldRect( HighlightRect );
						
						HHandleLoc = (HighlightRect.bottom + HighlightRect.top - HandleSize) / 2;
						VHandleLoc = (HighlightRect.right + HighlightRect.left - HandleSize) / 2;
												
						// Ordered so when very small the control will prefer to size down and to the right
						if( MousePoint.x > HighlightRect.right - HandleSize )
						{
							if( (MousePoint.y <= HighlightRect.top + HandleSize ) ||
								(MousePoint.y >= HighlightRect.bottom - HandleSize ) ||
								(MousePoint.y >= HHandleLoc && MousePoint.y <= HHandleLoc + HandleSize) )
							{
								gSizeRight = true;
							}
						}
						else if( MousePoint.x < HighlightRect.left + HandleSize )
						{
							if( (MousePoint.y <= HighlightRect.top + HandleSize ) ||
								(MousePoint.y >= HighlightRect.bottom - HandleSize ) ||
								(MousePoint.y >= HHandleLoc && MousePoint.y <= HHandleLoc + HandleSize) )
							{
								gSizeLeft = true;
							}
						}
						
						if( MousePoint.y > HighlightRect.bottom - HandleSize )
						{
							if( (MousePoint.x <= HighlightRect.left + HandleSize ) ||
								(MousePoint.x >= HighlightRect.right - HandleSize ) ||
								(MousePoint.x >= VHandleLoc && MousePoint.x <= VHandleLoc + HandleSize) )
							{
								gSizeDown = true;
							}
						}
						else if( MousePoint.y < HighlightRect.top + HandleSize )
						{
							if( (MousePoint.x <= HighlightRect.left + HandleSize ) ||
								(MousePoint.x >= HighlightRect.right - HandleSize ) ||
								(MousePoint.x >= VHandleLoc && MousePoint.x <= VHandleLoc + HandleSize) )
							{
								gSizeUp = true;
							}
						}
					}
				}
				
				if( gSizeUp )
				{
					if( gSizeLeft )
						hCursor = LoadCursor( 0, IDC_SIZENWSE );
					else if( gSizeRight )
						hCursor = LoadCursor( 0, IDC_SIZENESW );
					else
						hCursor = LoadCursor( 0, IDC_SIZENS );
				}
				else if( gSizeDown )
				{
					if( gSizeLeft )
						hCursor = LoadCursor( 0, IDC_SIZENESW );
					else if( gSizeRight )
						hCursor = LoadCursor( 0, IDC_SIZENWSE );
					else
						hCursor = LoadCursor( 0, IDC_SIZENS );
				}
				else if( gSizeLeft || gSizeRight )
					hCursor = LoadCursor( 0, IDC_SIZEWE );
			}
			
			if( !hCursor )
			{
				hCursor = LoadCursor( 0, IDC_ARROW );
				
				if( gInVisualEditingMode )
				{
					if( wParam & MK_SHIFT )
						hCursor = LoadCursor( GetModuleHandle(0), MAKEINTRESOURCE(IDC_ADDTOSEL) );
					else if( wParam & MK_CONTROL )
						hCursor = LoadCursor( GetModuleHandle(0), MAKEINTRESOURCE(IDC_REMOVEFROMSEL) );
				}	
			}
			
			if( gInVisualEditingMode || !gDrawCursor )
				SetCursor( hCursor );
			else
				SetCursor( 0 );
			
			break;	// Pass through to second stage
		}
		
		case WM_GETDLGCODE:
			return DLGC_WANTALLKEYS;
			
		case WM_MOUSEACTIVATE:
			SetFocus( hwnd );
			return MA_ACTIVATE;
			
		default:
			return DefWindowProc( hwnd, uMsg, wParam, lParam );
	}
	
	// Second stage, handle things that the rendering thread
	// is in contention for.
	WaitForSingleObject( gFrameRenderingMutex, INFINITE );
				
	if( !gInVisualEditingMode )
	{
		if( UIMsg.CreateFromWindowsMessage( uMsg, wParam, lParam ) )
			UIManager::gUIManager().ProcessMessage( UIMsg );
	}
	else if( UIManager::gUIManager().GetRootPage() )
	{
		UIPage *RootPage = UIManager::gUIManager().GetRootPage();
		
		switch( uMsg )
		{
		case WM_KEYUP:
			{
				s_UndoReady = true;

				switch (wParam)
				{
				case VK_DELETE:
					
					SendMessage (gMainWindow, WM_COMMAND, ID_EDIT_DELETE, 0);
					break;
				case 'C':
				case 'c':
					if ( GetAsyncKeyState( VK_CONTROL ) & 0x80000000 )
						CopySelectedObjectInTreeToClipboard( gObjectTree );
					break;
				case 'v':
				case 'V':
					if ( GetAsyncKeyState( VK_CONTROL ) & 0x80000000 )
						PasteObjectFromClipboard( gObjectTree );
					break;
				case 'x':
				case 'X':
					if ( GetAsyncKeyState( VK_CONTROL ) & 0x80000000 )
					{
						CopySelectedObjectInTreeToClipboard( gObjectTree );
						DeleteSelectedObjectInTree( gObjectTree );
					}
					break;
				default:
					s_UndoReady = false;
				}
			}
			
			break;
		case WM_KEYDOWN:
			{
				s_UndoReady = true;

				UIPoint Nudge(0,0);
				
				switch( wParam )
				{
				case VK_UP:
					Nudge.y = -1;
					break;
				case VK_DOWN:
					Nudge.y = 1;
					break;
				case VK_LEFT:
					Nudge.x = -1;
					break;
				case VK_RIGHT:
					Nudge.x = 1;
					break;
				default:
					s_UndoReady = false;
				}
				
				bool const forceUndo = s_UndoReady;
				for( UIBaseObject::UISmartObjectList::iterator i = gCurrentSelection.begin(); i != gCurrentSelection.end(); ++i )
				{
					if( (*i)->IsA( TUIWidget ) )
					{
						UIWidget * w = UI_ASOBJECT(UIWidget, *i );
						
						if( w != RootPage )
						{
							recordUndo(w, forceUndo);
							w->SetLocation( w->GetLocation() + Nudge );
						}
					}
				}
				
				break;
			}
			
		case WM_MOUSEMOVE:
			{
				if( gButtonDown )
				{
					POINTS CurrentMousePoint = MAKEPOINTS( lParam );
					bool const forceUndo = s_UndoReady;

					if( gMoving )
					{
						UIPoint GroupMovement(0,0);
						bool    bFirst = true;
						
						for( UIBaseObject::UISmartObjectList::iterator i = gCurrentSelection.begin(); i != gCurrentSelection.end(); ++i )
						{
							if( !(*i)->IsA( TUIWidget ) )
								continue;
							
							UIWidget * w = UI_ASOBJECT(UIWidget, *i );
								
							if( (w == RootPage) || w->HasProperty( UILowerString (gVisualEditLockPropertyName )) )
								continue;
							
							if( !bFirst )
							{
								recordUndo(w, forceUndo);
								w->SetLocation( w->GetLocation() + GroupMovement );
								continue;
							}
							
							
							UIPoint NewLocation;
							
							UIPoint LocationDiff (CurrentMousePoint.x - gMouseDownPoint.x, 
												  CurrentMousePoint.y - gMouseDownPoint.y);

							NewLocation.x = gOriginalLocation.x + LocationDiff.x;
							NewLocation.y = gOriginalLocation.y + LocationDiff.y;
							
							const UIPoint OldLocation (w->GetLocation());
							
							if( gSnapToGrid )
							{
								UIRect SnapRect;
								
								w->GetWorldRect( SnapRect );
								
								SnapRect.left   += LocationDiff.x;
								SnapRect.top    += LocationDiff.y;
								SnapRect.right  += LocationDiff.x;
								SnapRect.bottom += LocationDiff.y;
								
								SnapRect.left %= gXGridStep;
								SnapRect.top  %= gYGridStep;
								
								//-- jww hack to prevent jumping

								NewLocation.x -= SnapRect.left;
								NewLocation.y -= SnapRect.top;

								/*
								if( SnapRect.left < halfStepX )
									NewLocation.x -= SnapRect.left;
								else 
									NewLocation.x += gXGridStep - SnapRect.left;
								
								if( SnapRect.top < halfStepY )
									NewLocation.y -= SnapRect.top;
								else 
									NewLocation.y += gYGridStep - SnapRect.top;
*/
							}

							recordUndo(w, forceUndo);
							w->SetLocation( NewLocation );
							
							GroupMovement = NewLocation - OldLocation;
							bFirst        = false;							
						}
						
					}
					else if( gSizing )
					{
						UIPoint GroupMoveAmount(0,0);
						UISize  GroupSizeAmount(0,0);
						bool    bFirst = true;
						
						for( UIBaseObject::UISmartObjectList::iterator i = gCurrentSelection.begin(); i != gCurrentSelection.end(); ++i )
						{
							if( !(*i)->IsA( TUIWidget ) )
								continue;
							
							UIWidget * w = UI_ASOBJECT(UIWidget, *i );
							
							if( w->HasProperty( UILowerString (gVisualEditLockPropertyName ) ))
								continue;
							
							if( !bFirst )
							{
								recordUndo(w, forceUndo);
								w->SetLocation( w->GetLocation() + GroupMoveAmount );
								w->SetSize( w->GetSize() + GroupSizeAmount );
								continue;
							}
														
							UISize NewSize;
							UIPoint NewLocation;
							
							const UIPoint OldLocation (w->GetLocation());
							const UISize OldSize (w->GetSize());
							
							if( gSizeLeft )
							{
								NewSize.x     = gOriginalSize.x + (gMouseDownPoint.x - CurrentMousePoint.x);
								NewLocation.x = gOriginalLocation.x - (gMouseDownPoint.x - CurrentMousePoint.x);
							}
							else if( gSizeRight )
							{
								NewSize.x     = gOriginalSize.x - (gMouseDownPoint.x - CurrentMousePoint.x);
								NewLocation.x = gOriginalLocation.x;
							}
							else
							{
								NewSize.x     = gOriginalSize.x;
								NewLocation.x = gOriginalLocation.x;
							}
							
							if( gSizeUp )
							{
								NewSize.y     = gOriginalSize.y + (gMouseDownPoint.y - CurrentMousePoint.y);
								NewLocation.y = gOriginalLocation.y - (gMouseDownPoint.y - CurrentMousePoint.y);
							}
							else if( gSizeDown )
							{
								NewSize.y     = gOriginalSize.y - (gMouseDownPoint.y - CurrentMousePoint.y);
								NewLocation.y = gOriginalLocation.y;
							}
							else
							{
								NewSize.y     = gOriginalSize.y;
								NewLocation.y = gOriginalLocation.y;
							}
							
							if( gSnapToGrid )
							{
								UIRect SnapRect;
								
								if (w->GetParent () && w->GetParent ()->IsA (TUIWidget))
								{
									UIWidget * parentalWidget = static_cast<UIWidget *> (w->GetParent ());
									
									UIPoint parentalWorldLoc;
									parentalWidget->GetWorldLocation (parentalWorldLoc);
									
									SnapRect.left = parentalWorldLoc.x;
									SnapRect.top  = parentalWorldLoc.y;
								}
								
								SnapRect.left += NewLocation.x;
								SnapRect.top += NewLocation.y;
								
								SnapRect.right  = SnapRect.left + NewSize.x;
								SnapRect.bottom = SnapRect.top + NewSize.y;
								
								SnapRect.left   %= gXGridStep;
								SnapRect.top    %= gYGridStep;
								SnapRect.right  %= gXGridStep;
								SnapRect.bottom %= gYGridStep;
								
								if( NewLocation.x != gOriginalLocation.x )
								{
									if( (unsigned long)SnapRect.left < gXGridStep / 2 )
									{
										NewLocation.x -= SnapRect.left;
										NewSize.x     += SnapRect.left;
									}
									else 
									{
										NewLocation.x += gXGridStep - SnapRect.left;
										NewSize.x     -= gXGridStep - SnapRect.left;
									}
								}
								else if( NewSize.x != gOriginalSize.x )
								{
									if( (unsigned long)SnapRect.right < gXGridStep / 2 )
										NewSize.x -= SnapRect.right;
									else
										NewSize.x += gXGridStep - SnapRect.right;
								}
								
								if( NewLocation.y != gOriginalLocation.y )
								{
									if( (unsigned long)SnapRect.top < gYGridStep / 2 )
									{
										NewLocation.y -= SnapRect.top;
										NewSize.y     += SnapRect.top;
									}
									else 
									{
										NewLocation.y += gYGridStep - SnapRect.top;
										NewSize.y     -= gYGridStep - SnapRect.top;
									}
								}
								else if( NewSize.y != gOriginalSize.y )
								{
									if( (unsigned long)SnapRect.bottom < gYGridStep / 2 )
										NewSize.y -= SnapRect.bottom;
									else
										NewSize.y += gYGridStep - SnapRect.bottom;
								}
							}
							
							if( NewSize.x < 1 )
								NewSize.x = 1;
							if( NewSize.y < 1 )
								NewSize.y = 1;
							
							recordUndo(w, forceUndo);
							w->SetSize( NewSize );
							w->SetLocation( NewLocation );
							
							GroupMoveAmount = NewLocation - OldLocation;
							GroupSizeAmount = NewSize - OldSize;
							bFirst = false;
							
						}

					}
					else
					{
						int    dx = CurrentMousePoint.x - gMouseDownPoint.x;
						int    dy = CurrentMousePoint.y - gMouseDownPoint.y;
						double d  = sqrt( (double)(dx * dx + dy * dy) );
						
						if( gSizeUp || gSizeDown || gSizeLeft || gSizeRight )
						{
							if( d > SizeThreshold )
								gSizing = true;
						}
						else if( d > MoveThreshold )
							gMoving = true;
					}	
				}
				break;
			}
			case WM_LBUTTONDOWN:
			case WM_MBUTTONDOWN:
			case WM_RBUTTONDOWN:
				{				
					gMouseDownPoint.x   = GET_X_LPARAM( lParam );
					gMouseDownPoint.y   = GET_Y_LPARAM( lParam );
					
					UIWidget * ClickedWidget = 0;

					ClickedWidget = RootPage->GetWidgetFromPoint( gMouseDownPoint, true);
		
					if (!ClickedWidget)
						ClickedWidget = RootPage->GetWidgetFromPoint( gMouseDownPoint, false );

					//-- don't allow selection of transients
					while (ClickedWidget && (ClickedWidget->IsTransient ()))
					{
						if (ClickedWidget->GetParent ()->IsA (TUIWidget))
							ClickedWidget = static_cast<UIWidget *>(ClickedWidget->GetParent ());
					}

					gButtonDown         = true;
					gMoving             = false;
					gSizing             = false;
					
					if( ClickedWidget )
					{
						bool ResetMoveSize = true;
						
						if( wParam & MK_SHIFT )
							AddToSelection( ClickedWidget, true );
						else if( wParam & MK_CONTROL )
						{
							RemoveFromSelection( ClickedWidget );
							ResetMoveSize = false;
						}
						else
						{
							UIWidget *ObjectInSelection = GetObjectInSelection( gMouseDownPoint );							
							const bool theSelectionContains = SelectionContains( ClickedWidget );

							if( ObjectInSelection && !theSelectionContains)
							{
								/*
								UIBaseObject * p = ObjectInSelection;

								while (p)
								{
									if (p == ClickedWidget)
										break;

									p = p->GetParent ();
								}

								if (!p)
									SetSelection( ClickedWidget, true );
								else
									*/
								ResetMoveSize = false;
							}
							else
							{
								if(theSelectionContains)
								{
									// Move clicked item to the start of the selection
									RemoveFromSelection( ClickedWidget );
									AddToSelection( ClickedWidget, true );
								}
								else
									SetSelection( ClickedWidget, true );
							}
						}
						
						if( ResetMoveSize && ClickedWidget )
						{
							gOriginalSize    				= ClickedWidget->GetSize();
							gOriginalLocation				= ClickedWidget->GetLocation();
							HTREEITEM	hItemToSelect = GetObjectInTreeControlFromHandle( gObjectTree, 0, ClickedWidget );
							
							gNoChangeSelection = true;
							TreeView_SelectItem( gObjectTree, hItemToSelect );
							gNoChangeSelection = false;
						}
						
						else
						{
							//-- reset the original location and size - jww

							for( UIBaseObject::UISmartObjectList::iterator i = gCurrentSelection.begin(); i != gCurrentSelection.end(); ++i )
							{
								if( !(*i)->IsA( TUIWidget ) )
									continue;
								
								UIWidget * w = UI_ASOBJECT(UIWidget, *i );
								gOriginalSize = w->GetSize ();
								gOriginalLocation = w->GetLocation ();
								break;
							}
						}

					}
					
					break;
				}
				
			case WM_LBUTTONUP:
			case WM_MBUTTONUP:
			case WM_RBUTTONUP:
				{
					s_UndoReady = true;
					gButtonDown	= false;
					break;
				}
		}
	}
	
	ReleaseMutex( gFrameRenderingMutex );
	return DefWindowProc( hwnd, uMsg, wParam, lParam );
}

void UIThread( void * )
{
	UIManager &GUIManager = UIManager::gUIManager();
	
	while( WaitForSingleObject( gStopEvent, 0 ) == WAIT_TIMEOUT )
	{
		if( WaitForSingleObject( gFrameRenderingMutex, 100 ) == WAIT_OBJECT_0 )
		{
			UIPage *RootPage = GUIManager.GetRootPage();
			
			if( !gInVisualEditingMode )
				GUIManager.SendHeartbeats();
			
			if( !RootPage || !IsWindowVisible( gPrimaryDisplay->GetWindow() ) )
			{
				ReleaseMutex( gFrameRenderingMutex );
				Sleep( 100 );
				continue;
			}
			
			if( gPrimaryDisplay->BeginRendering() )
			{
				UIPoint CanvasTranslation(0,0);
				
				gPrimaryDisplay->ClearTo( UIColor( 0, 0, 0 ), UIRect (0, 0, gPrimaryDisplay->GetWidth (), gPrimaryDisplay->GetHeight () ));
				gPrimaryDisplay->PushState();
				
				if( gCurrentlySelectedPage )
				{
					CanvasTranslation = -gCurrentlySelectedPage->GetLocation();
					gPrimaryDisplay->Translate( CanvasTranslation );
				}
				
				GUIManager.DrawCursor( gDrawCursor );
				GUIManager.Render( *gPrimaryDisplay );
				
				if( gDrawGrid )
				{
					int x;
					int y;
					int linecount;
					int width  = RootPage->GetWidth();
					int height = RootPage->GetHeight();
					
					float LineOpacity = gGridColor.a / 255.0f;
					float ThickLineOpacity = 2.0f * LineOpacity;
					
					gPrimaryDisplay->SetOpacity( LineOpacity );
					
					for( x = 0, linecount = 0; x <= width; x += gXGridStep, ++linecount )
					{
						if( (linecount % gGridMajorTicks) == 0 )
						{
							gPrimaryDisplay->SetOpacity( ThickLineOpacity );
							gPrimaryDisplay->ClearTo( gGridColor, UIRect( x, CanvasTranslation.y, x+1, height ) );
							gPrimaryDisplay->SetOpacity( LineOpacity );
						}
						else
							gPrimaryDisplay->ClearTo( gGridColor, UIRect( x, CanvasTranslation.y, x+1, height ) );
					}
					
					for( y = 0, linecount = 0; y <= height; y += gYGridStep, ++linecount )
					{
						if( (linecount % gGridMajorTicks) == 0 )
						{
							gPrimaryDisplay->SetOpacity( ThickLineOpacity );
							gPrimaryDisplay->ClearTo( gGridColor, UIRect( CanvasTranslation.x, y, width, y+1 ) );
							gPrimaryDisplay->SetOpacity( LineOpacity );
						}
						else
							gPrimaryDisplay->ClearTo( gGridColor, UIRect( CanvasTranslation.x, y, width, y+1 ) );
					}
				}
				
				if( gDrawHighlightRect )
				{
					for( UIBaseObject::UISmartObjectList::iterator i = gCurrentSelection.begin(); i != gCurrentSelection.end(); ++i )
					{
						if( !(*i)->IsA( TUIWidget ) )
							continue;
						
						UIWidget * w = UI_ASOBJECT(UIWidget, *i );
						UIRect    HighlightRect;						
						
						w->GetWorldRect( HighlightRect );
						
						gPrimaryDisplay->SetOpacity( gHighlightFillColor.a / 255.0f );
						
						gPrimaryDisplay->ClearTo( gHighlightFillColor, UIRect( HighlightRect.left, HighlightRect.top,
							HighlightRect.right, HighlightRect.bottom ) );
						
						gPrimaryDisplay->SetOpacity( gHighlightOutlineColor.a / 255.0f );
						
						gPrimaryDisplay->ClearTo( gHighlightOutlineColor, UIRect( HighlightRect.left, HighlightRect.top,
							HighlightRect.left + LineWidth, HighlightRect.bottom ) );
						
						gPrimaryDisplay->ClearTo( gHighlightOutlineColor, UIRect( HighlightRect.right - LineWidth, HighlightRect.top,
							HighlightRect.right, HighlightRect.bottom ) );
						
						gPrimaryDisplay->ClearTo( gHighlightOutlineColor, UIRect( HighlightRect.left + LineWidth, HighlightRect.top,
							HighlightRect.right - LineWidth, HighlightRect.top + LineWidth ) );
						
						gPrimaryDisplay->ClearTo( gHighlightOutlineColor, UIRect( HighlightRect.left + LineWidth, HighlightRect.bottom - LineWidth,
							HighlightRect.right - LineWidth, HighlightRect.bottom ) );
						
						if( gInVisualEditingMode && (HighlightRect.Height() >= 8) && (HighlightRect.Width() >= 8) && !w->HasProperty(UILowerString (gVisualEditLockPropertyName) ))
						{
							gPrimaryDisplay->ClearTo( gHighlightOutlineColor, UIRect( HighlightRect.left + LineWidth, HighlightRect.top + LineWidth, 
								HighlightRect.left + HandleSize, HighlightRect.top + HandleSize ) );
							
							gPrimaryDisplay->ClearTo( gHighlightOutlineColor, UIRect( HighlightRect.right - HandleSize, HighlightRect.top + LineWidth,
								HighlightRect.right - LineWidth, HighlightRect.top + HandleSize ) );
							
							gPrimaryDisplay->ClearTo( gHighlightOutlineColor, UIRect( HighlightRect.left + LineWidth, HighlightRect.bottom - HandleSize,
								HighlightRect.left + HandleSize, HighlightRect.bottom - LineWidth ) );
							
							gPrimaryDisplay->ClearTo( gHighlightOutlineColor, UIRect( HighlightRect.right - HandleSize, HighlightRect.bottom - HandleSize,
								HighlightRect.right - LineWidth, HighlightRect.bottom - LineWidth ) );
							
							if( HighlightRect.Height() >= 16 && HighlightRect.Width() >= 16 )
							{
								long HHandleLoc, VHandleLoc;
								
								HHandleLoc = (HighlightRect.bottom + HighlightRect.top - HandleSize) / 2;
								VHandleLoc = (HighlightRect.right + HighlightRect.left - HandleSize) / 2;
								
								gPrimaryDisplay->ClearTo( gHighlightOutlineColor, UIRect( HighlightRect.left + LineWidth, HHandleLoc,
									HighlightRect.left + HandleSize, HHandleLoc + HandleSize ) );
								
								gPrimaryDisplay->ClearTo( gHighlightOutlineColor, UIRect( HighlightRect.right - HandleSize, HHandleLoc,
									HighlightRect.right - LineWidth, HHandleLoc + HandleSize ) );
								
								gPrimaryDisplay->ClearTo( gHighlightOutlineColor, UIRect( VHandleLoc, HighlightRect.top + LineWidth,
									VHandleLoc + HandleSize, HighlightRect.top + HandleSize ) );
								
								gPrimaryDisplay->ClearTo( gHighlightOutlineColor, UIRect( VHandleLoc, HighlightRect.bottom - HandleSize,
									VHandleLoc + HandleSize, HighlightRect.bottom - LineWidth ) );
							}
						}
					}
				}
				
				gPrimaryDisplay->PopState();
				gPrimaryDisplay->EndRendering();
				gPrimaryDisplay->Flip();
				
				gTriangleCount = gPrimaryDisplay->GetTriangleCount();
				gFlushCount = gPrimaryDisplay->GetFlushCount();
				
				++gFrameCount;
			}
			ReleaseMutex( gFrameRenderingMutex );			
			
			if( gLimitFrameRate && !(gFrameCount % 2) )
				Sleep(18);
		}
	}
}

void InitializeButtonGraphic( HWND hwndDlg, UINT ControlID, UINT ResourceID )
{
	SendDlgItemMessage( hwndDlg, ControlID, BM_SETIMAGE, IMAGE_ICON,
		(LPARAM)LoadImage( GetModuleHandle(0), MAKEINTRESOURCE(ResourceID), IMAGE_ICON, 16, 16, 0 ) );
}

void InitializeButtonGraphicAndState( HWND hwndDlg, UINT ControlID, UINT ResourceID, bool bPressed )
{
	InitializeButtonGraphic( hwndDlg, ControlID, ResourceID );
	
	if( bPressed )
		CheckDlgButton( hwndDlg, ControlID, BST_CHECKED );
	else
		CheckDlgButton( hwndDlg, ControlID, BST_UNCHECKED );
}

BOOL CALLBACK UIWindowDlgProc( HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam )
{
	switch( uMsg )
	{
	case WM_INITDIALOG:
		{
			WNDCLASSEX wc = { sizeof( wc ) };
			RECT			 rcClient;
			RECT			 rcCanvas;
			RECT			 rcCanvasClient;
			
			gUIWindow				 = hwndDlg;
			
			wc.lpfnWndProc	 = CanvasWindowProc;
			wc.hInstance		 = GetModuleHandle(0);
			wc.lpszMenuName  = MAKEINTRESOURCE( IDR_MAIN_MENU );
			wc.hCursor       = NULL;
			wc.lpszClassName = "UICanvas";
			wc.style         |= CS_DBLCLKS;
			
			if( !RegisterClassEx( &wc ) )
				return 1;
			
			gTabControlWindow = GetDlgItem( hwndDlg, IDC_ROOTPAGE );
			
			GetClientRect( gTabControlWindow, &rcCanvas );
			MapWindowPoints( gTabControlWindow, hwndDlg, (LPPOINT)&rcCanvas, 2 );
			CopyRect( &rcCanvasClient, &rcCanvas );
			
			TCITEM tci;
			tci.mask    = TCIF_TEXT;
			tci.pszText = "Size Test";
			TabCtrl_InsertItem( gTabControlWindow, 0, &tci );
			TabCtrl_AdjustRect( gTabControlWindow, FALSE, &rcCanvasClient );
			TabCtrl_DeleteAllItems( gTabControlWindow );
			
			GetClientRect( hwndDlg, &rcClient );
			
			gTabControlPadding.left   = rcCanvas.left;
			gTabControlPadding.top    = rcCanvas.top;
			gTabControlPadding.right  = rcClient.right - rcCanvas.right;
			gTabControlPadding.bottom = rcClient.bottom - rcCanvas.bottom;
			
			gCanvasWindow = CreateWindow( "UICanvas", "UICanvas", WS_CHILD | WS_VISIBLE | WS_TABSTOP,
				rcCanvasClient.left, rcCanvasClient.top, rcCanvas.right, rcCanvas.bottom, hwndDlg,
				(HMENU)100, GetModuleHandle(0), NULL );
			
			InitializeButtonGraphicAndState( hwndDlg, IDC_PLAY, IDI_PLAY, !gInVisualEditingMode );
			AddTooltipFromControlID( gTooltip, hwndDlg, IDC_PLAY, "Run / Stop User Interface" );
			
			InitializeButtonGraphic( hwndDlg, IDC_REFRESH, IDI_REFRESH );
			AddTooltipFromControlID( gTooltip, hwndDlg, IDC_REFRESH, "Refresh" );
			
			InitializeButtonGraphicAndState( hwndDlg, IDC_DRAWCURSOR, IDI_CURSOR, gDrawCursor );
			AddTooltipFromControlID( gTooltip, hwndDlg, IDC_DRAWCURSOR, "Draw Cursor" );
			
			InitializeButtonGraphicAndState( hwndDlg, IDC_DRAWHIGHLIGHT, IDI_DRAWHIGHLIGHT, gDrawHighlightRect );
			AddTooltipFromControlID( gTooltip, hwndDlg, IDC_DRAWHIGHLIGHT, "Draw Selection Highlight" );
			
			InitializeButtonGraphicAndState( hwndDlg, IDC_DRAWGRID, IDI_DRAWGRID, gDrawGrid );
			AddTooltipFromControlID( gTooltip, hwndDlg, IDC_DRAWGRID, "Draw Grid" );
			
			InitializeButtonGraphicAndState( hwndDlg, IDC_SNAPTOGRID, IDI_SNAPTOGRID, gSnapToGrid );			
			AddTooltipFromControlID( gTooltip, hwndDlg, IDC_SNAPTOGRID, "Snap to Grid" );
			
			InitializeButtonGraphic( hwndDlg, IDC_ALIGNLEFT, IDI_ALIGNLEFT );
			AddTooltipFromControlID( gTooltip, hwndDlg, IDC_ALIGNLEFT, "Align Left" );
			
			InitializeButtonGraphic( hwndDlg, IDC_ALIGNHCENTER, IDI_ALIGNHCENTER );
			AddTooltipFromControlID( gTooltip, hwndDlg, IDC_ALIGNHCENTER, "Align on Horzontal Center" );
			
			InitializeButtonGraphic( hwndDlg, IDC_ALIGNRIGHT, IDI_ALIGNRIGHT );
			AddTooltipFromControlID( gTooltip, hwndDlg, IDC_ALIGNRIGHT, "Align Right" );
			
			InitializeButtonGraphic( hwndDlg, IDC_ALIGNTOP, IDI_ALIGNTOP );
			AddTooltipFromControlID( gTooltip, hwndDlg, IDC_ALIGNTOP, "Align Top" );
			
			InitializeButtonGraphic( hwndDlg, IDC_ALIGNVCENTER, IDI_ALIGNVCENTER );
			AddTooltipFromControlID( gTooltip, hwndDlg, IDC_ALIGNVCENTER, "Align on Vertical Center" );
			
			InitializeButtonGraphic( hwndDlg, IDC_ALIGNBOTTOM, IDI_ALIGNBOTTOM );
			AddTooltipFromControlID( gTooltip, hwndDlg, IDC_ALIGNBOTTOM, "Align Bottom" );
			
			InitializeButtonGraphic( hwndDlg, IDC_SIZEWIDTH, IDI_SIZEWIDTH );
			AddTooltipFromControlID( gTooltip, hwndDlg, IDC_SIZEWIDTH, "Make Controls Same Width" );
			
			InitializeButtonGraphic( hwndDlg, IDC_SIZEHEIGHT, IDI_SIZEHEIGHT );
			AddTooltipFromControlID( gTooltip, hwndDlg, IDC_SIZEHEIGHT, "Make Controls Same Height" );
			
			InitializeButtonGraphic( hwndDlg, IDC_RESET, IDI_RESET );
			AddTooltipFromControlID( gTooltip, hwndDlg, IDC_RESET, "Reset the Palette Data" );

			UpdateAlignmentControlEnabledState();
			
			SetTimer( hwndDlg, UITimerID, 100, 0 );
			return 0;
		}
		
	case WM_MOVE:
		{
			RECT rc;
			RECT rcMainWindow;
			GetWindowRect( hwndDlg, &rc );
			GetWindowRect( gMainWindow, &rcMainWindow );
			
			SetWindowPos( gMainWindow, 0, rc.left - (rcMainWindow.right - rcMainWindow.left), rc.top, 0, 0, SWP_NOSIZE | SWP_NOZORDER | SWP_NOACTIVATE );
			return 0;
		}
		
	case WM_NOTIFY:
		if( wParam == IDC_ROOTPAGE )
		{
			LPNMHDR GenericHeader = (LPNMHDR)lParam;
			
			if( GenericHeader->code == TCN_SELCHANGE )
			{
				HTREEITEM hRoot;
				
				SizeWindowToCurrentPageSelection();
				hRoot = GetObjectInTreeControlFromHandle( gObjectTree, 0, gCurrentlySelectedPage );
				TreeView_SelectItem( gObjectTree, hRoot );
				TreeView_EnsureVisible( gObjectTree, hRoot );
			}
		}
		return 0;
		
	case WM_COMMAND:
		if( HIWORD(wParam) == BN_CLICKED )
		{
			if( LOWORD(wParam) == IDC_PLAY )
				gInVisualEditingMode = !gInVisualEditingMode;
			if( LOWORD(wParam) == IDC_REFRESH )
			{
				RefreshContents();
				ClearDefPushButtonLook( hwndDlg, IDC_REFRESH );
			}
			else if( LOWORD( wParam ) == IDC_DRAWCURSOR )
				gDrawCursor = !gDrawCursor;
			else if( LOWORD( wParam ) == IDC_DRAWHIGHLIGHT )
				gDrawHighlightRect = !gDrawHighlightRect;
			else if( LOWORD( wParam ) == IDC_DRAWGRID )
				gDrawGrid = !gDrawGrid;
			else if( LOWORD( wParam ) == IDC_SNAPTOGRID )
				gSnapToGrid = !gSnapToGrid;
			else if( LOWORD( wParam ) == IDC_ALIGNLEFT )
			{					
				AlignSelection( LeftAlignment );
				ClearDefPushButtonLook( hwndDlg, IDC_ALIGNLEFT );					
			}
			else if( LOWORD( wParam ) == IDC_ALIGNHCENTER )
			{
				AlignSelection( HCenterAlignment );
				ClearDefPushButtonLook( hwndDlg, IDC_ALIGNHCENTER );
			}
			else if( LOWORD( wParam ) == IDC_ALIGNRIGHT )
			{
				AlignSelection( RightAlignment );
				ClearDefPushButtonLook( hwndDlg, IDC_ALIGNRIGHT );
			}
			else if( LOWORD( wParam ) == IDC_ALIGNTOP )
			{
				AlignSelection( TopAlignment );
				ClearDefPushButtonLook( hwndDlg, IDC_ALIGNTOP );
			}
			else if( LOWORD( wParam ) == IDC_ALIGNVCENTER )
			{
				AlignSelection( VCenterAlignment );
				ClearDefPushButtonLook( hwndDlg, IDC_ALIGNVCENTER );
			}
			else if( LOWORD( wParam ) == IDC_ALIGNBOTTOM )
			{
				AlignSelection( BottomAlignment );
				ClearDefPushButtonLook( hwndDlg, IDC_ALIGNBOTTOM );
			}
			else if( LOWORD( wParam ) == IDC_SIZEWIDTH )
			{
				SizeSelection( true, false );
				ClearDefPushButtonLook( hwndDlg, IDC_SIZEWIDTH );
			}
			else if( LOWORD( wParam ) == IDC_SIZEHEIGHT )
			{
				SizeSelection( false, true );
				ClearDefPushButtonLook( hwndDlg, IDC_SIZEHEIGHT );
			}
			else if( LOWORD( wParam ) == IDC_RESET )
			{
				UIPalette * palette = UIPalette::GetInstance();
				if(palette)
					palette->Reset();
			}
		}
		SetFocus( gCanvasWindow );
		return 0;
		
	case WM_TIMER:
		if( UIManager::gUIManager().GetRootPage() )
		{
			RECT   rc;
			
			const UISize & RootSize = UIManager::gUIManager().GetRootPage()->GetSize();
			GetClientRect( gCanvasWindow, &rc );
			
			if( ((rc.right - rc.left) != RootSize.x) || ((rc.bottom - rc.top) != RootSize.y) )
				SizeWindowToCurrentPageSelection();
		}
		return 0;
		
	case WM_DESTROY:
		KillTimer( hwndDlg, UITimerID );
		return 0;
		
	default:
		return 0;
	}
}

//----------------------------------------------------------------------

void recordUndo(UIBaseObject * const object, bool force)
{
	WaitForSingleObject( gFrameRenderingMutex, INFINITE );

	if (s_UndoReady || force) 
	{
		if (object && object->GetParent()) 
		{
			UIUndo undo(object);
			s_UndoQueue.push_back(undo);
			
			// Don't let the list grow too big.
			if (s_UndoQueue.size() > MaxUndoSteps) 
			{
				s_UndoQueue.erase(s_UndoQueue.begin());
			}
			
			s_UndoReady = false;
		}
	}

	s_UndoQueueEmpty = s_UndoQueue.empty();
	EnableMenuItem(gMenu, ID_EDIT_UNDO, s_UndoQueueEmpty ? (MF_BYCOMMAND | MF_GRAYED) : (MF_BYCOMMAND | MF_ENABLED));

	ReleaseMutex( gFrameRenderingMutex );
}


//----------------------------------------------------------------------

void recordUndo(UIBaseObject * const object, UILowerString property, UIString oldValue, UIString newValue, bool force)
{
	//Can't undo operations to the root object
	if(object == UIManager::gUIManager().GetRootPage())
		return;

	WaitForSingleObject( gFrameRenderingMutex, INFINITE );

	if (s_UndoReady || force) 
	{
		UIUndo undo(object, property, oldValue, newValue);
		s_UndoQueue.push_back(undo);
		
		// Don't let the list grow too big.
		if (s_UndoQueue.size() > MaxUndoSteps) 
		{
			s_UndoQueue.erase(s_UndoQueue.begin());
		}

		s_UndoReady = false;
	}

	s_UndoQueueEmpty = s_UndoQueue.empty();
	EnableMenuItem(gMenu, ID_EDIT_UNDO, s_UndoQueueEmpty ? (MF_BYCOMMAND | MF_GRAYED) : (MF_BYCOMMAND | MF_ENABLED));

	ReleaseMutex( gFrameRenderingMutex );
}

//----------------------------------------------------------------------

void undo()
{
	WaitForSingleObject( gFrameRenderingMutex, INFINITE );

	UIBaseObject * selectedObject = NULL;

	if(!s_UndoQueue.empty()) 
	{
		UIUndo & undoObject = s_UndoQueue.back();

		selectedObject = undoObject.getObject();

		bool rebuildTree = true;
		undoObject.undo(rebuildTree, &selectedObject);

		if (rebuildTree && selectedObject) 
		{
			// delete old tree, and add a new one.
			RebuildTreeView(NULL);

		}
		
		s_UndoQueue.pop_back();

		s_UndoQueueEmpty = s_UndoQueue.empty();
		EnableMenuItem(gMenu, ID_EDIT_UNDO, s_UndoQueueEmpty ? (MF_BYCOMMAND | MF_GRAYED) : (MF_BYCOMMAND | MF_ENABLED));
	}

	ReleaseMutex( gFrameRenderingMutex );
}

//----------------------------------------------------------------------

void clearUndoHistory()
{
	s_UndoQueue.clear();
}
