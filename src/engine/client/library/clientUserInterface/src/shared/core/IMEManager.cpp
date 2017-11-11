// ======================================================================
//
// IMEManager.cpp
// Portions copyright 2004 Sony Online Entertainment
// All Rights Reserved.
//
// ======================================================================

#include "clientUserInterface/FirstClientUserInterface.h"
#include "clientUserInterface/CUIPreferences.h"
#include "clientUserInterface/IMEManager.h"

#include "sharedFoundation/ExitChain.h"
#include "sharedFoundation/Os.h"
#include "sharedIoWin/IoWinManager.h"

#include "UIManager.h"

#include <dinput.h>
#include <vector>

#define GETPROCADDRESS( Module, APIName, functionPointer) \
    functionPointer = GetProcAddress( Module, #APIName ); \
    if (functionPointer) \
        *(FARPROC*)&_##APIName = functionPointer

#define IMM32_DLLNAME "\\imm32.dll"
#define VER_DLLNAME "\\version.dll"

#if 0
#define GETPROCADDRESS( Module, APIName, Temp ) \
    Temp = GetProcAddress( Module, #APIName ); \
    if( Temp ) \
        *(FARPROC*)&_##APIName = Temp
#endif

#define PLACEHOLDERPROC( APIName ) \
    _##APIName = 0

// IME constants
#define MAX_COMPSTRING_SIZE 256

#define CHT_IMEFILENAME1    "TINTLGNT.IME" // New Phonetic
#define CHT_IMEFILENAME2    "CINTLGNT.IME" // New Chang Jie
#define CHT_IMEFILENAME3    "MSTCIPHA.IME" // Phonetic 5.1
#define CHS_IMEFILENAME1    "PINTLGNT.IME" // MSPY1.5/2/3
#define CHS_IMEFILENAME2    "MSSCIPYA.IME" // MSPY3 for OfficeXP

#define LANG_CHT            MAKELANGID(LANG_CHINESE, SUBLANG_CHINESE_TRADITIONAL)
#define LANG_CHS            MAKELANGID(LANG_CHINESE, SUBLANG_CHINESE_SIMPLIFIED)
#define _CHT_HKL            ( (HKL)(INT_PTR)0xE0080404 ) // New Phonetic
#define _CHT_HKL2           ( (HKL)(INT_PTR)0xE0090404 ) // New Chang Jie
#define _CHS_HKL            ( (HKL)(INT_PTR)0xE00E0804 ) // MSPY
#define MAKEIMEVERSION( major, minor ) \
    ( (DWORD)( ( (BYTE)( major ) << 24 ) | ( (BYTE)( minor ) << 16 ) ) )

#define IMEID_CHT_VER42 ( LANG_CHT | MAKEIMEVERSION( 4, 2 ) )	// New(Phonetic/ChanJie)IME98  : 4.2.x.x // Win98
#define IMEID_CHT_VER43 ( LANG_CHT | MAKEIMEVERSION( 4, 3 ) )	// New(Phonetic/ChanJie)IME98a : 4.3.x.x // Win2k
#define IMEID_CHT_VER44 ( LANG_CHT | MAKEIMEVERSION( 4, 4 ) )	// New ChanJie IME98b          : 4.4.x.x // WinXP
#define IMEID_CHT_VER50 ( LANG_CHT | MAKEIMEVERSION( 5, 0 ) )	// New(Phonetic/ChanJie)IME5.0 : 5.0.x.x // WinME
#define IMEID_CHT_VER51 ( LANG_CHT | MAKEIMEVERSION( 5, 1 ) )	// New(Phonetic/ChanJie)IME5.1 : 5.1.x.x // IME2002(w/OfficeXP)
#define IMEID_CHT_VER52 ( LANG_CHT | MAKEIMEVERSION( 5, 2 ) )	// New(Phonetic/ChanJie)IME5.2 : 5.2.x.x // IME2002a(w/Whistler)
#define IMEID_CHT_VER60 ( LANG_CHT | MAKEIMEVERSION( 6, 0 ) )	// New(Phonetic/ChanJie)IME6.0 : 6.0.x.x // IME XP(w/WinXP SP1)
#define IMEID_CHS_VER41	( LANG_CHS | MAKEIMEVERSION( 4, 1 ) )	// MSPY1.5	// SCIME97 or MSPY1.5 (w/Win98, Office97)
#define IMEID_CHS_VER42	( LANG_CHS | MAKEIMEVERSION( 4, 2 ) )	// MSPY2	// Win2k/WinME
#define IMEID_CHS_VER53	( LANG_CHS | MAKEIMEVERSION( 5, 3 ) )	// MSPY3	// WinXP

	static const int MAX_CANDLIST;

	enum IMESTATE
	{
		IMEUI_STATE_OFF,
		IMEUI_STATE_ON,
		IMEUI_STATE_ENGLISH
	};

  enum
	{
		INDICATOR_NON_IME,
		INDICATOR_CHS,
		INDICATOR_CHT,
		INDICATOR_KOREAN,
		INDICATOR_JAPANESE
	};

bool				installed = false;
HINSTANCE		s_hDllIme;

HINSTANCE							s_hDllImm32;      // IMM32 DLL handle
HINSTANCE							s_hDllVer;        // Version DLL handle

IMESTATE  s_ImeState = IMEUI_STATE_OFF;
bool									s_bVerticalCand;  // Indicates that the candidates are listed vertically

bool									s_enabled = false;

CandidateList_t				s_candidateList;
unsigned int					s_selectedCand;
int										s_selectedCandIndex;
unsigned int					s_curCandPage;
unsigned int					s_numCands;
unsigned int					s_candsPerPage;
unsigned int					s_maxCandsPerPage = 10;

BYTE									s_abCompStringAttr[MAX_COMPSTRING_SIZE];
Unicode::String				s_CompString;
int					s_compCaretPos = 0;

HKL										s_hklEnglish;
HKL										s_hklIme;
HKL										s_hklCurrent;

bool s_consumeEnter;
bool s_candidateListOpen = false;
bool s_ignoreNextKey = false;
bool s_ignoreNextBackspace = false;
bool s_ignoreNextEscape = false;
bool s_compositionWindowOpen = false;

UIWidget *s_contextWidget = NULL;
UIWidget *s_lastNonNullContextWidget = NULL;

IMEStatusFunctionPointer s_openStatusNotifyFunction = NULL;

  // Function pointers: IMM32
//  static INPUTCONTEXT * (WINAPI * _ImmLockIMC)( HIMC );
  static BOOL (WINAPI * _ImmUnlockIMC)( HIMC );
  static LPVOID (WINAPI * _ImmLockIMCC)( HIMCC );
  static BOOL (WINAPI * _ImmUnlockIMCC)( HIMCC );
//  static BOOL (WINAPI * _ImmDisableTextFrameService)( DWORD );
  static LONG (WINAPI * _ImmGetCompositionStringW)( HIMC, DWORD, LPVOID, DWORD );
	static BOOL (WINAPI * _ImmSetCompositionStringW)( HIMC, DWORD, LPVOID, DWORD, LPVOID, DWORD);
  static DWORD (WINAPI * _ImmGetCandidateListW)( HIMC, DWORD, LPCANDIDATELIST, DWORD );
  static HIMC (WINAPI * _ImmGetContext)( HWND );
  static BOOL (WINAPI * _ImmReleaseContext)( HWND, HIMC );
  static HIMC (WINAPI * _ImmAssociateContext)( HWND, HIMC );
  static BOOL (WINAPI * _ImmGetOpenStatus)( HIMC );
  static BOOL (WINAPI * _ImmSetOpenStatus)( HIMC, BOOL );
  static BOOL (WINAPI * _ImmGetConversionStatus)( HIMC, LPDWORD, LPDWORD );
  static HWND (WINAPI * _ImmGetDefaultIMEWnd)( HWND );
  static UINT (WINAPI * _ImmGetIMEFileNameA)( HKL, LPSTR, UINT );
  static UINT (WINAPI * _ImmGetVirtualKey)( HWND );
  static BOOL (WINAPI * _ImmNotifyIME)( HIMC, DWORD, DWORD, DWORD );
  static BOOL (WINAPI * _ImmSetConversionStatus)( HIMC, DWORD, DWORD );
  static BOOL (WINAPI * _ImmSimulateHotKey)( HWND, DWORD );
  static BOOL (WINAPI * _ImmIsIME)( HKL );

  // Function pointers: Version library imports
  static BOOL (APIENTRY * _VerQueryValueA)( const LPVOID, LPSTR, LPVOID *, PUINT );
  static BOOL (APIENTRY * _GetFileVersionInfoA)( LPSTR, DWORD, DWORD, LPVOID );
  static DWORD (APIENTRY * _GetFileVersionInfoSizeA)( LPSTR, LPDWORD );

  // Function pointers: Traditional Chinese IME
  static UINT (WINAPI * _GetReadingString)( HIMC, UINT, LPWSTR, PINT, BOOL*, PUINT );
  static BOOL (WINAPI * _ShowReadingWindow)( HIMC, BOOL );

  static WORD GetLanguage() { return LOWORD( s_hklCurrent ); }
  static WORD GetPrimaryLanguage() { return (WORD) PRIMARYLANGID( LOWORD( s_hklCurrent ) ); }
  static WORD GetSubLanguage() { return (WORD) SUBLANGID( LOWORD( s_hklCurrent ) ); }
  static DWORD GetImeId( UINT uIndex = 0 );

	static int handleProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

	static void SendCompString();

	static void HandleIMEComposition(LPARAM lParam);
	static void HandleChangeCandidateList();
	static void CheckToggleState();
	static void CheckInputLocale();

	static void InitializeImm();
	static void UninitializeImm();
	static void SetupImeApi();

	static void ResetCompositionString();

	static bool IoWinIMEFunction(int key);

//////////////////////////////////////////////////////////////////////////////////////////////////////
// Function pointers
//////////////////////////////////////////////////////////////////////////////////////////////////////

void IMEManager::install(bool useIME)
{	
	//@TODO This install is not matched correctly with the remove because there is no SetupClientUserInterface	

	if(installed)
		return;

	InitializeImm();
	ExitChain::add(IMEManager::remove, "IMEManager::remove");
	installed = true;

	// Save the system default for when we switch to an IME enabled text element
	s_hklIme = GetKeyboardLayout(0);

	// Force the current IME to english
	s_hklCurrent = s_hklIme;

	SetupImeApi();
	if(useIME)
	{
		SetEnabled(true);	
		Os::setIMEHookFunction(handleProc);
		IoWinManager::registerIMEFunction(IoWinIMEFunction);
	}
}


void IMEManager::remove()
{
	Os::setIMEHookFunction(NULL);

	UninitializeImm();

	DEBUG_FATAL(!installed, ("not installed"));
	installed = false;
}

void IMEManager::SetContextWidget(UIWidget *contextWidget)
{
	if((contextWidget == NULL) && (s_contextWidget != NULL))
		s_lastNonNullContextWidget = s_contextWidget;

	// With Non-Modal chat we can't be killing the composition string since we get the character event prior
	// to setting focus (due to the character event causing focus).  But, with the modal chat we may be
	// having this after using WASD to move and so have lots of characters in the IME composition which need
	// to be cleared on any context change.
	if (CuiPreferences::getModalChat())
	{
		// Clear composition string in IME
		IMEManager::ClearCompositionString();
	}

	if (s_contextWidget != NULL)
	{
		IoWinManager::queueIMECloseCandidate();
	}

	s_contextWidget = contextWidget;
}

//Warning: this function isn't checking anything about the pointer it holds; for example, it's not
//a watcher.  The UIBaseObject pointed to might have gotten deleted out from underneath and this
//function wouldn't know that.  You can be reasonably sure that this hasn't happened if the Hud hasn't
//changed.  The /ui reset command deletes this widget but also cleans up after itself.
//You also don't know that the widget is neccessarily active (so you should check that).
UIWidget *IMEManager::GetContextWidget(void)
{
	return s_contextWidget;
}

void IMEManager::SetEnabled(bool enabled)
{
	if (s_enabled)
	{
		// Turning IME off
		if (!enabled)
		{			
			s_enabled = enabled;
			IoWinManager::queueIMECloseCandidate();
			ResetCompositionString();

			HIMC hImc;
			if( NULL != ( hImc = _ImmGetContext( Os::getWindow() ) ) )
			{
				_ImmSetOpenStatus( hImc, false);			
				_ImmReleaseContext( Os::getWindow(), hImc );
			}
		}
	}
	else
	{
		// Turning IME on
		if (enabled)
		{
			s_enabled = enabled;
			if (s_hklCurrent != s_hklIme)
			{
				s_hklCurrent = s_hklIme;
				s_consumeEnter = false;
			}
			
			HIMC hImc;
			if( NULL != ( hImc = _ImmGetContext( Os::getWindow() ) ) )
			{				
				_ImmSetOpenStatus( hImc, true);
				_ImmReleaseContext( Os::getWindow(), hImc );
			}
		}
	}
}

bool IMEManager::IsIMEMessage(UINT uMsg)
{
	return 	(uMsg == WM_IME_COMPOSITION ||
			 uMsg == WM_IME_ENDCOMPOSITION ||
			 uMsg == WM_IME_NOTIFY ||
			 uMsg == WM_IME_SETCONTEXT ||
			 uMsg == WM_IME_STARTCOMPOSITION);
}

int handleProc(HWND /*hwnd*/, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	UNREF(lParam);
	//DEBUG_REPORT_LOG(true, ("IMEManager::handleProc %x %x %x\n", uMsg, wParam, lParam));	

	if (!s_enabled)
	{
		if (uMsg == WM_INPUTLANGCHANGE
		||  uMsg == WM_IME_COMPOSITION
		||  uMsg == WM_IME_ENDCOMPOSITION
		||  uMsg == WM_IME_SETCONTEXT
		||  uMsg == WM_IME_STARTCOMPOSITION)
		{
			return 0;
		}

		if (uMsg == WM_IME_NOTIFY)
		{
			if(wParam == IMN_SETOPENSTATUS)
			{
				HIMC hImc;
				if( NULL != ( hImc = _ImmGetContext( Os::getWindow() ) ) )
				{
					if(_ImmGetOpenStatus( hImc ))
					{
						_ImmSetOpenStatus(hImc, false);										
						if(s_openStatusNotifyFunction)
							s_openStatusNotifyFunction(false);
					}
					_ImmReleaseContext( Os::getWindow(), hImc );
				}
			}
			return 0;
		}
		return 1;
	}

	// Loose/fast instantiation check.  This is getting called on all Window events, so this must be fast.
	// Unfortunately, it also needs to know that the UI has all the messaging it needs to get started so
	// we have to check readiness.  This will make sure the UI has all the messaging to get to the point
	// where UIManager::gUIManager() will be able to return a value and not lock the Windows messaging.
	if (!UIManager::isUIReady())
	{
		return 1;
	}

	if (CuiPreferences::getModalChat() && !s_compositionWindowOpen && IMEManager::IsIMEMessage(uMsg))
	{
		const UIWidget * const focused = UIManager::gUIManager ().GetFocusedLeafWidget ();
		if (!focused ||
			!(focused->IsA (TUITextbox) || focused->IsA (TUIText) || focused->IsA (TUIComboBox) || focused->IsA (TUIList) || focused->IsA (TUITable) || focused->IsA(TUIWebBrowser)))
		{
			return 0;
		}
	}

	switch (uMsg)
	{
		case WM_KEYDOWN:
//			s_consumeEnter = false;
			return 1;
		case WM_INPUTLANGCHANGE:
			SetupImeApi();
			return 1;
		case WM_IME_COMPOSITION:
			HandleIMEComposition(lParam);
			IoWinManager::queueIMEComposition();
			s_ignoreNextEscape = true;
			return 0;
		case WM_IME_ENDCOMPOSITION:
			s_compositionWindowOpen = false;
			IoWinManager::queueIMEEndComposition();
			return 0;
		case WM_IME_NOTIFY:
			switch (wParam)
			{
				case IMN_SETCONVERSIONMODE:
				case IMN_SETOPENSTATUS:
					CheckToggleState();
					return 0;
				case IMN_OPENCANDIDATE:
				case IMN_CHANGECANDIDATE:
					HandleChangeCandidateList();
					IoWinManager::queueIMEChangeCandidate();
					s_candidateListOpen = true;
					return 0;
				case IMN_CLOSECANDIDATE:
					IoWinManager::queueIMECloseCandidate();
					s_candidateListOpen = false;
					s_ignoreNextKey = true;
					s_consumeEnter = true;
					return 0;
				case IMN_PRIVATE:
					return 0;
				default:
					return 0;
			}
			return 1;
		case WM_IME_SETCONTEXT:
			return 0;
		case WM_IME_STARTCOMPOSITION:
			s_consumeEnter = false;
			ResetCompositionString();
			HandleIMEComposition(GCS_COMPSTR | GCS_CURSORPOS);
			return 0;
		case 0x288: //WM_IME_REQUEST, which is not found in winuser but must return 0 or windows ME crashes -- ARH 12/07/04
			return 0;

		default:
			return 1;
	}

	return 0;
}

//--------------------------------------------------------------------------------------
void InitializeImm()
{
	FARPROC Temp;

    char wszPath[MAX_PATH+1];
    if( !::GetSystemDirectoryA( wszPath, MAX_PATH+1 ) )
	{
		int err = GetLastError();
		UNREF(err);
		DEBUG_WARNING(true, ("Could not GetSystemDirectory: error %d\n", err));
        return;
	}
    strcat( wszPath, IMM32_DLLNAME );
	
	//const char * const fakeFile = "C:\\WINDOWS\\SYSTEM32\\IMM32.DLL";
    s_hDllImm32 = LoadLibraryA( wszPath );

    if( s_hDllImm32 )
    {
//        GETPROCADDRESS( s_hDllImm32, ImmLockIMC, Temp );
        GETPROCADDRESS( s_hDllImm32, ImmUnlockIMC, Temp );
        GETPROCADDRESS( s_hDllImm32, ImmLockIMCC, Temp );
        GETPROCADDRESS( s_hDllImm32, ImmUnlockIMCC, Temp );
//        GETPROCADDRESS( s_hDllImm32, ImmDisableTextFrameService, Temp );
        GETPROCADDRESS( s_hDllImm32, ImmGetCompositionStringW, Temp );
		GETPROCADDRESS( s_hDllImm32, ImmSetCompositionStringW, Temp );
        GETPROCADDRESS( s_hDllImm32, ImmGetCandidateListW, Temp );
        GETPROCADDRESS( s_hDllImm32, ImmGetContext, Temp );
        GETPROCADDRESS( s_hDllImm32, ImmReleaseContext, Temp );
        GETPROCADDRESS( s_hDllImm32, ImmAssociateContext, Temp );
        GETPROCADDRESS( s_hDllImm32, ImmGetOpenStatus, Temp );
        GETPROCADDRESS( s_hDllImm32, ImmSetOpenStatus, Temp );
        GETPROCADDRESS( s_hDllImm32, ImmGetConversionStatus, Temp );
        GETPROCADDRESS( s_hDllImm32, ImmGetDefaultIMEWnd, Temp );
        GETPROCADDRESS( s_hDllImm32, ImmGetIMEFileNameA, Temp );
        GETPROCADDRESS( s_hDllImm32, ImmGetVirtualKey, Temp );
        GETPROCADDRESS( s_hDllImm32, ImmNotifyIME, Temp );
        GETPROCADDRESS( s_hDllImm32, ImmSetConversionStatus, Temp );
        GETPROCADDRESS( s_hDllImm32, ImmSimulateHotKey, Temp );
        GETPROCADDRESS( s_hDllImm32, ImmIsIME, Temp );
    }

    if( !::GetSystemDirectoryA( wszPath, MAX_PATH+1 ) )
        return;
    strcat( wszPath, VER_DLLNAME );
    s_hDllVer = LoadLibraryA( wszPath );
    if( s_hDllVer )
    {
		GETPROCADDRESS( s_hDllVer, VerQueryValueA, Temp );
        GETPROCADDRESS( s_hDllVer, GetFileVersionInfoA, Temp );
        GETPROCADDRESS( s_hDllVer, GetFileVersionInfoSizeA, Temp );
    }
}

//--------------------------------------------------------------------------------------
void UninitializeImm()
{
    if( s_hDllImm32 )
    {
//        PLACEHOLDERPROC( ImmLockIMC );
        PLACEHOLDERPROC( ImmUnlockIMC );
        PLACEHOLDERPROC( ImmLockIMCC );
        PLACEHOLDERPROC( ImmUnlockIMCC );
//        PLACEHOLDERPROC( ImmDisableTextFrameService );
        PLACEHOLDERPROC( ImmGetCompositionStringW );
		PLACEHOLDERPROC( ImmSetCompositionStringW );
        PLACEHOLDERPROC( ImmGetCandidateListW );
        PLACEHOLDERPROC( ImmGetContext );
        PLACEHOLDERPROC( ImmReleaseContext );
        PLACEHOLDERPROC( ImmAssociateContext );
        PLACEHOLDERPROC( ImmGetOpenStatus );
        PLACEHOLDERPROC( ImmSetOpenStatus );
        PLACEHOLDERPROC( ImmGetConversionStatus );
        PLACEHOLDERPROC( ImmGetDefaultIMEWnd );
        PLACEHOLDERPROC( ImmGetIMEFileNameA );
        PLACEHOLDERPROC( ImmGetVirtualKey );
        PLACEHOLDERPROC( ImmNotifyIME );
        PLACEHOLDERPROC( ImmSetConversionStatus );
        PLACEHOLDERPROC( ImmSimulateHotKey );
        PLACEHOLDERPROC( ImmIsIME );

        FreeLibrary( s_hDllImm32 );
        s_hDllImm32 = NULL;
    }
    if( s_hDllIme )
    {
        PLACEHOLDERPROC( GetReadingString );
        PLACEHOLDERPROC( ShowReadingWindow );

        FreeLibrary( s_hDllIme );
        s_hDllIme = NULL;
    }
    if( s_hDllVer )
    {
        PLACEHOLDERPROC( VerQueryValueA );
        PLACEHOLDERPROC( GetFileVersionInfoA );
        PLACEHOLDERPROC( GetFileVersionInfoSizeA );

        FreeLibrary( s_hDllVer );
        s_hDllVer = NULL;
    }
}

void SetupImeApi()
{
	FARPROC Temp;
	
	char szImeFile[MAX_PATH + 1];

	if( s_hDllIme )
	{
		FreeLibrary( s_hDllIme );
		s_hDllIme = NULL;
	}

	s_hklCurrent = GetKeyboardLayout(0);

	if ( _ImmGetIMEFileNameA( s_hklCurrent, szImeFile, sizeof(szImeFile)/sizeof(szImeFile[0]) - 1) == 0)
	{
		return;
	}
	char wszPath[MAX_PATH+1];
    if( !::GetSystemDirectoryA( wszPath, MAX_PATH+1 ) )
	{
		int err = GetLastError();
		UNREF(err);
		DEBUG_WARNING(true, ("Could not GetSystemDirectory: error %d\n", err));
        return;
	}
	strcat( wszPath, "\\" );
	strcat( wszPath, szImeFile );

	//const char * const fakeFile = "C:\\WINDOWS\\SYSTEM\\IMEJP98.IME";
	s_hDllIme = LoadLibraryA( wszPath );

	if ( !s_hDllIme )
		return;
	
    GETPROCADDRESS( s_hDllIme, GetReadingString, Temp );
	GETPROCADDRESS( s_hDllIme, ShowReadingWindow, Temp );

}

//--------------------------------------------------------------------------------------
void CheckToggleState()
{
  CheckInputLocale();
  bool bIme = _ImmIsIME( s_hklCurrent ) != 0;

  HIMC hImc;
  if( NULL != ( hImc = _ImmGetContext( Os::getWindow() ) ) )
  {
    s_ImeState = ( bIme && _ImmGetOpenStatus( hImc ) != 0 ) ? IMEUI_STATE_ON : IMEUI_STATE_OFF;	
	_ImmReleaseContext( Os::getWindow(), hImc );

	if(s_openStatusNotifyFunction)
		s_openStatusNotifyFunction(s_ImeState == IMEUI_STATE_ON);    
  }
	else
	{
    s_ImeState = IMEUI_STATE_OFF;
	}
}

//--------------------------------------------------------------------------------------
void CheckInputLocale()
{
  static HKL hklPrev = 0;
  s_hklCurrent = GetKeyboardLayout( 0 );
  if ( hklPrev == s_hklCurrent )
	{
    return;
	}

  hklPrev = s_hklCurrent;
  switch ( GetPrimaryLanguage() )
  {
    // Simplified Chinese
    case LANG_CHINESE:
      s_bVerticalCand = true;
      switch ( GetSubLanguage() )
      {
				case SUBLANG_CHINESE_SIMPLIFIED:
					s_bVerticalCand = GetImeId() == 0;
          break;
        case SUBLANG_CHINESE_TRADITIONAL:
          break;
        default:    // unsupported sub-language
          break;
      }
      break;
    case LANG_KOREAN:
			break;
    case LANG_JAPANESE:
			s_bVerticalCand = true;
      break;
    default:
			break;
	}
}

//--------------------------------------------------------------------------------------
//	GetImeId( UINT uIndex )
//		returns
//	returned value:
//	0: In the following cases
//		- Non Chinese IME input locale
//		- Older Chinese IME
//		- Other error cases
//
//	Othewise:
//      When uIndex is 0 (default)
//			bit 31-24:	Major version
//			bit 23-16:	Minor version
//			bit 15-0:	Language ID
//		When uIndex is 1
//			pVerFixedInfo->dwFileVersionLS
//
//	Use IMEID_VER and IMEID_LANG macro to extract version and language information.
//

// We define the locale-invariant ID ourselves since it doesn't exist prior to WinXP
// For more information, see the CompareString() reference.
#define LCID_INVARIANT MAKELCID(MAKELANGID(LANG_ENGLISH, SUBLANG_ENGLISH_US), SORT_DEFAULT)

DWORD GetImeId( UINT uIndex )
{
    static HKL hklPrev = 0;
    static DWORD dwID[2] = { 0, 0 };  // Cache the result

    DWORD   dwVerSize;
    DWORD   dwVerHandle;
    LPVOID  lpVerBuffer;
    LPVOID  lpVerData;
    UINT    cbVerData;
    char    szTmp[1024];

    if( uIndex >= sizeof( dwID ) / sizeof( dwID[0] ) )
        return 0;

    if( hklPrev == s_hklCurrent )
        return dwID[uIndex];

    hklPrev = s_hklCurrent;  // Save for the next invocation

    // Check if we are using an older Chinese IME
    if( !( ( s_hklCurrent == _CHT_HKL ) || ( s_hklCurrent == _CHT_HKL2 ) || ( s_hklCurrent == _CHS_HKL ) ) )
    {
        dwID[0] = dwID[1] = 0;
        return dwID[uIndex];
    }

    // Obtain the IME file name
    if ( !_ImmGetIMEFileNameA( s_hklCurrent, szTmp, ( sizeof(szTmp) / sizeof(szTmp[0]) ) - 1 ) )
    {
        dwID[0] = dwID[1] = 0;
        return dwID[uIndex];
    }

    // Check for IME that doesn't implement reading string API
    if ( !_GetReadingString )
    {
        if( ( CompareStringA( LCID_INVARIANT, NORM_IGNORECASE, szTmp, -1, CHT_IMEFILENAME1, -1 ) != CSTR_EQUAL ) &&
            ( CompareStringA( LCID_INVARIANT, NORM_IGNORECASE, szTmp, -1, CHT_IMEFILENAME2, -1 ) != CSTR_EQUAL ) &&
            ( CompareStringA( LCID_INVARIANT, NORM_IGNORECASE, szTmp, -1, CHT_IMEFILENAME3, -1 ) != CSTR_EQUAL ) &&
            ( CompareStringA( LCID_INVARIANT, NORM_IGNORECASE, szTmp, -1, CHS_IMEFILENAME1, -1 ) != CSTR_EQUAL ) &&
            ( CompareStringA( LCID_INVARIANT, NORM_IGNORECASE, szTmp, -1, CHS_IMEFILENAME2, -1 ) != CSTR_EQUAL ) )
        {
            dwID[0] = dwID[1] = 0;
            return dwID[uIndex];
        }
    }

    dwVerSize = _GetFileVersionInfoSizeA( szTmp, &dwVerHandle );
    if( dwVerSize )
    {
        lpVerBuffer = HeapAlloc( GetProcessHeap(), 0, dwVerSize );
        if( lpVerBuffer )
        {
            if( _GetFileVersionInfoA( szTmp, dwVerHandle, dwVerSize, lpVerBuffer ) )
            {
                if( _VerQueryValueA( lpVerBuffer, "\\", &lpVerData, &cbVerData ) )
                {
                    DWORD dwVer = ( (VS_FIXEDFILEINFO*)lpVerData )->dwFileVersionMS;
                    dwVer = ( dwVer & 0x00ff0000 ) << 8 | ( dwVer & 0x000000ff ) << 16;
                    if( _GetReadingString
                        ||
                        ( GetLanguage() == LANG_CHT &&
                          ( dwVer == MAKEIMEVERSION(4, 2) ||
                            dwVer == MAKEIMEVERSION(4, 3) ||
                            dwVer == MAKEIMEVERSION(4, 4) ||
                            dwVer == MAKEIMEVERSION(5, 0) ||
                            dwVer == MAKEIMEVERSION(5, 1) ||
                            dwVer == MAKEIMEVERSION(5, 2) ||
                            dwVer == MAKEIMEVERSION(6, 0) ) )
                        ||
                        ( GetLanguage() == LANG_CHS &&
                          ( dwVer == MAKEIMEVERSION(4, 1) ||
                            dwVer == MAKEIMEVERSION(4, 2) ||
                            dwVer == MAKEIMEVERSION(5, 3) ) )
                      )
                    {
                        dwID[0] = dwVer | GetLanguage();
                        dwID[1] = ( (VS_FIXEDFILEINFO*)lpVerData )->dwFileVersionLS;
                    }
                }
            }
            HeapFree( GetProcessHeap(), 0, lpVerBuffer );
        }
    }

    return dwID[uIndex];
}

//--------------------------------------------------------------------------------------
// Resets the composition string.
void ResetCompositionString()
{
    s_CompString = L"";
	s_compCaretPos = 0;
    ZeroMemory( s_abCompStringAttr, sizeof(s_abCompStringAttr) );
}

void IMEManager::ClearCompositionString()
{
	ResetCompositionString();
	
	HIMC hImc;
	if( NULL != ( hImc = _ImmGetContext( Os::getWindow() ) ) )
	{
		_ImmNotifyIME( hImc, NI_COMPOSITIONSTR, CPS_CANCEL, 0 );
		_ImmReleaseContext( Os::getWindow(), hImc );
	}
}

void HandleIMEComposition(LPARAM lParam)
{
	LONG lRet; // returned count in Characters
	WCHAR wszCompStr[MAX_COMPSTRING_SIZE];
	HIMC hImc;

	hImc = _ImmGetContext(Os::getWindow());

	if (hImc == NULL)
	{
		return;
	}

	if (lParam == 0)
	{
		s_ignoreNextBackspace = true;
		ResetCompositionString();
	}

	if (lParam & GCS_RESULTSTR)
	{
		lRet = _ImmGetCompositionStringW( hImc, GCS_RESULTSTR, wszCompStr, sizeof(wszCompStr));

		if (lRet >= 0)
		{
			lRet /= sizeof(WCHAR); // convert size in byte to size in char
			wszCompStr[lRet] = 0;
			s_CompString = wszCompStr;
			s_consumeEnter = true;
			SendCompString();
			ResetCompositionString();
		}
	}

	if (lParam & GCS_COMPSTR)
	{
		s_compositionWindowOpen = true;
		lRet = _ImmGetCompositionStringW(hImc, GCS_COMPSTR, wszCompStr, sizeof(wszCompStr));
		if (lRet >= 0)
		{
			lRet /= sizeof(WCHAR); // convert size in byte to size in char
			wszCompStr[lRet] = 0;

			s_CompString = wszCompStr;

			lRet = _ImmGetCompositionStringW(hImc, GCS_COMPATTR, s_abCompStringAttr, sizeof(s_abCompStringAttr));
			if (lRet >= 0)
			{
				s_abCompStringAttr[lRet] = 0;
			}
			// Clear the composition string if a num_pad key, other than enter, is hit
			if ( !CuiPreferences::getModalChat() && Os::isNumPadChar((unsigned char)wszCompStr[lRet-1]) )
			{
				IMEManager::ClearCompositionString();
			}
		}
	}

	s_compCaretPos = _ImmGetCompositionStringW( hImc, GCS_CURSORPOS, NULL, 0);
	if (s_compCaretPos < 0)
	{
		s_compCaretPos = 0;
	}

	_ImmReleaseContext(Os::getWindow(), hImc);
}

void HandleChangeCandidateList()
{
	HIMC hImc;

	s_candidateList.clear();

	hImc = _ImmGetContext(Os::getWindow());

	if (hImc == NULL)
	{
		return;
	}

	DWORD lenRequired = 0;
	lenRequired = _ImmGetCandidateListW(hImc, 0, NULL, 0);

	LPCANDIDATELIST lpCandList = NULL;

	if (lenRequired)
	{
		lpCandList = (LPCANDIDATELIST) new char[lenRequired];

		if (lpCandList)
		{
			lenRequired = _ImmGetCandidateListW(hImc, 0, lpCandList, lenRequired);
		}
	}

	if (!lpCandList)
	{
		_ImmReleaseContext(Os::getWindow(), hImc);
		return;
	}

	s_selectedCand = lpCandList->dwSelection;
	s_numCands = lpCandList->dwCount;
	s_candsPerPage = std::min(static_cast<unsigned int>(lpCandList->dwPageSize), s_maxCandsPerPage);

	// Calculate the index number of the first displayed element
	int nPageTopIndex = 0;
	if (GetPrimaryLanguage() == LANG_JAPANESE)
	{
		nPageTopIndex = (s_selectedCand / s_candsPerPage) * s_candsPerPage;
	}
	else
	{
		nPageTopIndex = lpCandList->dwPageStart;
	}

	// For certain older Chinese IMEs, you cannot select a candidate by number
	// Otherwise, we turn selected candidate into the current offset from 0 to MAX_CANDLIST - 1
	if (GetLanguage() == LANG_CHS && !GetImeId())
	{
		s_selectedCandIndex = -1;
	}
	else
	{
		s_selectedCandIndex = s_selectedCand - nPageTopIndex;
	}

	unsigned int curCand = nPageTopIndex;

	for (unsigned int i = 0; i < s_candsPerPage; i++)
	{
		if (curCand >= s_numCands)
		{
			break;
		}

		Unicode::String str = (LPWSTR) ((char *) lpCandList + lpCandList->dwOffset[curCand]);

		s_candidateList.push_back(str);

		curCand++;
	}

	if (lpCandList)
	{
		delete [] lpCandList;
	}

	_ImmReleaseContext(Os::getWindow(), hImc);
}


void SendCompString()
{
	for (unsigned int i = 0; i < s_CompString.length(); ++i)
	{
		IoWinManager::queueCharacter(0, s_CompString[i]);
	}
}

void IMEManager::ResendCharacter(unsigned short ch)
{
	UNREF(ch);
	HIMC hImc;

	hImc = _ImmGetContext(Os::getWindow());

	if (hImc == NULL)
		return;
	
	if (s_enabled && s_ImeState == IMEUI_STATE_ON)
	{
		_ImmSetCompositionStringW(hImc, SCS_SETSTR, &ch, sizeof(unsigned short), NULL, 0);
		PostMessage(Os::getWindow(), WM_IME_COMPOSITION, 0, 0);
	}
	else
	{
		PostMessage(Os::getWindow(), WM_CHAR, ch, 0);
	}

	_ImmReleaseContext(Os::getWindow(), hImc);
}

//If this function returns true, then IoWin ignores this keystroke
bool IoWinIMEFunction(int key)
{
	HIMC hImc = _ImmGetContext(Os::getWindow());

	if (hImc == NULL)
		return false;	

	//DEBUG_WARNING(true, ("IOWinIMEFunction: GCLO = %d IGOS = %d CS = %d\n",
	//	GetCandidateListOpen(), _ImmGetOpenStatus(_ImmGetContext(Os::getWindow())), lRet));
	if ((key == DIK_RETURN || key == DIK_NUMPADENTER) && IMEManager::GetConsumeEnter())
	{
		IMEManager::SetConsumeEnter(false);		
		_ImmReleaseContext(Os::getWindow(), hImc);
		return true;
	}

	//If candidate list is open, consume function keys, up or down arrow
	if ((s_ignoreNextKey || IMEManager::GetCandidateListOpen()) &&
		((key == DIK_UP) ||
		(key == DIK_DOWN) ||
		((key >= DIK_F1) && (key <= DIK_F10)) ||
		(key == DIK_F11) ||
		(key == DIK_F12) ||
		(key == DIK_ESCAPE)) )
	{
		s_ignoreNextBackspace = false;
		s_ignoreNextKey = false;
		_ImmReleaseContext(Os::getWindow(), hImc);
		return true;
	}	
	else if(s_ignoreNextEscape && (key == DIK_ESCAPE))
	{
		s_ignoreNextBackspace = false;
		s_ignoreNextKey = false;
		s_ignoreNextEscape = false;
		_ImmReleaseContext(Os::getWindow(), hImc);
		return true;
	}
	else if(!CuiPreferences::getModalChat() && _ImmGetOpenStatus(hImc))
	{
		static WCHAR wszCompStr[MAX_COMPSTRING_SIZE];
		LONG lRet = _ImmGetCompositionStringW( hImc, GCS_COMPSTR, wszCompStr, sizeof(wszCompStr));
		if (lRet > 0)
		{
			//IME is active, but candidate list is not open - consume function keys
			if ( (key == DIK_LEFT) || (key == DIK_RIGHT) ||
				 ((key >= DIK_F1) && (key <= DIK_F10)) ||
				 (key == DIK_F11) ||
				 (key == DIK_F12) ||
				 (key == DIK_ESCAPE) )
			{
				s_ignoreNextBackspace = false;
				s_ignoreNextKey = false;
				_ImmReleaseContext(Os::getWindow(), hImc);
				return true;
			}
		}
	}
	if(!IMEManager::GetCandidateListOpen() && (!s_CompString.empty() || s_ignoreNextBackspace))
	{
		//Backspace is 0x0E
		if((key == 0x0E))
		{
			s_ignoreNextBackspace = false;
			s_ignoreNextKey = false;
			_ImmReleaseContext(Os::getWindow(), hImc);
			return true;
		}
	}
	s_ignoreNextBackspace = false;
	s_ignoreNextKey = false;
	_ImmReleaseContext(Os::getWindow(), hImc);
	return false;
}

#if 0
void IMEManager::CloseCandidateList()
{
	// Now generate keypress events to move the comp string cursor
    // to the click point.  First, if the candidate window is displayed,
    // send Esc to close it.
    HIMC hImc = _ImmGetContext( Os::getWindow() );
    if( !hImc )
        return;

    _ImmNotifyIME( hImc, NI_CLOSECANDIDATE, 0, 0 );
    _ImmReleaseContext( Os::getWindow(), hImc );
}
#endif

void IMEManager::SetIMEOpenStatusNotifyFunction(IMEStatusFunctionPointer function)
{
	s_openStatusNotifyFunction = function;
}

Unicode::String& IMEManager::GetCompString() { return s_CompString; }
unsigned char * IMEManager::GetCompStringAttrs() { return s_abCompStringAttr; }
unsigned int IMEManager::GetCompCaratIndex() { return (unsigned int) s_compCaretPos; }
CandidateList_t& IMEManager::GetCandidateList() { return s_candidateList; }
bool IMEManager::GetEnabled() { return s_enabled; }
int IMEManager::GetSelectedCandidateIndex() { return s_selectedCandIndex; }
int IMEManager::GetSelectedCandidate()	{ return	s_selectedCand; }
int IMEManager::GetNumCandidates() { return s_numCands; }
void IMEManager::SetMaxCandidatesPerPage(unsigned int maxCandsPerPage) { s_maxCandsPerPage = maxCandsPerPage; }
void IMEManager::SetConsumeEnter(bool consume) { s_consumeEnter = consume; }
bool IMEManager::GetConsumeEnter() { return s_consumeEnter; }
bool IMEManager::GetCandidateListOpen() { return s_candidateListOpen; }




