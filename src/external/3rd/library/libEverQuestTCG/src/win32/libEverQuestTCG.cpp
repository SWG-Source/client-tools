//#if ENABLE_TCG
#include "libEverQuestTCG.h"

#include <windows.h>
#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <malloc.h>

#if 0 // _DEBUG
#define SELECT_RELEASE_OR_DEBUG( r, d ) d
#else
#define SELECT_RELEASE_OR_DEBUG( r, d ) r
#endif

// Internal Declarations
namespace libEverQuestTCG
{
    // Order needs to match coreclient\include\coreclient.h
    struct CallbackTable
    {
        NavigateProc                navigateProc;
        NavigateWithPostDataProc    navigateWithPostDataProc;
        PlaySoundProc               playSoundProc;
        PlayMusicProc               playMusicProc;
        SetSoundVolumeProc          setSoundVolumeProc;
        SetMusicVolumeProc          setMusicVolumeProc;
        StopAllSoundsProc           stopAllSoundsProc;
        SetWindowStateProc          setWindowStateProc;
    };

    // Module functions
    // Signatures need to match coreclient\dllmain.cpp
    static void (*pInitializeProc)       (int argc, const char *argv[], HWND hwndDesktop, CallbackTable * );
    static void (*pRunFrameProc)         ();
    static void (*pShutdownProc)         ();
    static unsigned (*pGetWindowsProc)   ( unsigned *pWindowsIds, unsigned uNumWindowIds );
    static unsigned (*pGetCaptureWindowProc)();
    static HCURSOR (*pGetCursorProc)();
    static void (*pOnWindowStateChangedProc)( int );
    static void (*pOnMusicCompletionProc)();

    // Window functions
    // Signatures need to match coreclient\dllmain.cpp
    static unsigned (*pGetWindowRepaintRects)( unsigned uWindowId, RECT *pRects, unsigned uNumRects );
    static unsigned (*pGetWindowSurfaceData)( unsigned uWindowId, void **ppBits, unsigned *pWidth, unsigned *pHeight, unsigned *pStride );
    static void (*pOnMouseEventProc)     ( unsigned uWindowId, int eventType, int x, int y, int gx, int gy, int mouseButton, int mouseState, int keyboardState );
    static void (*pOnMouseWheelEventProc)( unsigned uWindowId, int x, int y, int gx, int gy, int delta, int mouseState, int keyboardState );
    static unsigned (*pOnKeyEventProc)       ( int eventType, int key, int keyboardState, int code, int vkey, int nativeMods );
    static void (*pOnFocusProc)          ( unsigned uWindowId, int focus );

    // These must match the event IDs in QEvent::Type - DON
    enum QEvent
    {
        MouseButtonPress = 2,                   // mouse button pressed
        MouseButtonRelease = 3,                 // mouse button released
        MouseButtonDblClick = 4,                // mouse button double click
        MouseMove = 5,                          // mouse move
        KeyPress = 6,                           // key pressed
        KeyRelease = 7,                         // key released
    };

    //------------------------------------------------------------------------------
    // Manager
    //------------------------------------------------------------------------------
    class Manager
    {
    public:

                 Manager();
                ~Manager();

        bool     init();
        void     update();
        void     shutdown();

        unsigned getWindows( Window **pWindows, unsigned uNumWindows );

        Window * getWindow( unsigned uWindowId );

        Window * createWindow( unsigned uWindowId, int iX, int iY, unsigned uWidth, unsigned uHeight );
        void     destroyWindow( unsigned uWindowId );
        
    private:
        
        HMODULE  m_hDLL;
        Window **m_ppWindows;
        unsigned m_uNumWindows;
    };

    class WindowImpl
    {
    public:

                WindowImpl( Window *pWindow );
               ~WindowImpl();

        bool    init( unsigned uWindowId );

        bool    canGetFocus() const;
        void    getTitle( char *pTitle, unsigned uBufferLen ) const;

        void    setFocus( bool bFocus );
        void    setLocation( int x, int y );
        void    setSize( unsigned width, unsigned height );

        void    getMinMaxInfo( unsigned &minWidth, unsigned &minHeight, unsigned &maxWidth, unsigned &maxHeight );

        unsigned getWindowRepaintRects( RECT *pRects, unsigned uNumRects );
        bool     getWindowSurfaceData( void **ppBits, unsigned *pWidth, unsigned *pHeight, unsigned *pStride );
        
        void    close();

        void    onLeftMouseDown( int x, int y, int gx, int gy, unsigned uFlags );
        void    onLeftMouseUp( int x, int y, int gx, int gy, unsigned uFlags );
        void    onLeftMouseDoubleClick( int x, int y, int gx, int gy, unsigned uFlags );
        void    onMiddleMouseDown( int x, int y, int gx, int gy, unsigned uFlags );
        void    onMiddleMouseUp( int x, int y, int gx, int gy, unsigned uFlags );
        void    onMiddleMouseDoubleClick( int x, int y, int gx, int gy, unsigned uFlags );
        void    onRightMouseDown( int x, int y, int gx, int gy, unsigned uFlags );
        void    onRightMouseUp( int x, int y, int gx, int gy, unsigned uFlags );
        void    onRightMouseDoubleClick( int x, int y, int gx, int gy, unsigned uFlags );
        void    onMouseMove( int x, int y, int gx, int gy, unsigned uFlags );
        void    onMouseWheel( int x, int y, int gx, int gy, int ticks, unsigned uFlags );

        bool    onKeyDown( int key, unsigned uFlags, int code, int vkey, int nativeMods );
        bool    onKeyUp( int key, unsigned uFlags, int code, int vkey, int nativeMods );

    private:

        friend Manager;

        Window  *m_pWindow;
        unsigned m_uWindowId;
        HWND     m_hWnd;
    };

    // Implementation
    static const char userNameArgHeader[] = "--username=";
    static const char sessionIdArgHeader[] = "--sessionID=";
    static const char challengeArgHeader[] = "--challenge=";
    static const char charnameArgHeader[] = "--character=";

    namespace PrivateData
    {
        char             szApplicationDirectory[ _MAX_PATH + 1 ];
        char             szUserName[128];
        char             szSessionID[64];
        char             szChallenge[128];
        char             szCharacterName[128];
        HWND             hwndDesktop;
        Language         eLanguage = LANG_English;
        CallbackTable    callbackTable;
        HostProcessType  eHostProcessType;
        RealmType        eRealmType;
        bool             bStartTutorial;
        bool             bIsChallengeFounder;
        
        HANDLE           hShutdownLoaderThread;
        HANDLE           hLoaderThread;

        const char      *aDLLsToLoad[] = 
        {
			SELECT_RELEASE_OR_DEBUG( "msvcp80.dll",                        "msvcp80d.dll" ),
			SELECT_RELEASE_OR_DEBUG( "msvcr80.dll",                        "msvcr80d.dll" ),

			SELECT_RELEASE_OR_DEBUG( "QtCore4.dll",                        "QtCored4.dll" ),
			SELECT_RELEASE_OR_DEBUG( "QtGui4.dll",                         "QtGuid4.dll" ),
			SELECT_RELEASE_OR_DEBUG( "QtNetwork4.dll",                     "QtNetworkd4.dll" ),
			SELECT_RELEASE_OR_DEBUG( "QtScript4.dll",                      "QtScriptd4.dll" ),
			SELECT_RELEASE_OR_DEBUG( "QtSvg4.dll",                         "QtSvgd4.dll" ),
			SELECT_RELEASE_OR_DEBUG( "QtXml4.dll",                         "QtXmld4.dll" ),

			SELECT_RELEASE_OR_DEBUG( "SWGTCG.dll",                         "SWGTCG.dll" ),

			SELECT_RELEASE_OR_DEBUG( "plugins\\imageformats\\qgif4.dll",   "plugins\\imageformats\\qgifd4.dll" ),
			SELECT_RELEASE_OR_DEBUG( "plugins\\imageformats\\qjpeg4.dll",  "plugins\\imageformats\\qjpegd4.dll" ),
			SELECT_RELEASE_OR_DEBUG( "plugins\\imageformats\\qmng4.dll",   "plugins\\imageformats\\qmngd4.dll" ),
			SELECT_RELEASE_OR_DEBUG( "plugins\\imageformats\\qtiff4.dll",  "plugins\\imageformats\\qtiffd4.dll" ),
        };

        enum { NUM_DLLS_TO_LOAD = sizeof(aDLLsToLoad) / sizeof(aDLLsToLoad[0]) };

        HMODULE          aDLLHandles[ NUM_DLLS_TO_LOAD ];

        const char *aFilesToMap[] = 
        { 
            "resources.rcc",
            "sounds.rcc",
            "effects.rcc",
            "cards.rcc"
        };

        enum { NUM_FILES_TO_MAP = sizeof(aFilesToMap) / sizeof(aFilesToMap[0]) };
    }
    
    Manager *g_pManager = 0;
    
    bool init( const char *szApplicationDirectory, HostProcessType eHostProcessType, RealmType eRealmType )
    {
        if( g_pManager )
            return false;

        assert( strlen( szApplicationDirectory ) < sizeof( PrivateData::szApplicationDirectory ) );
        strcpy( PrivateData::szApplicationDirectory, szApplicationDirectory );
        PrivateData::eHostProcessType = eHostProcessType;
        PrivateData::eRealmType = eRealmType;
        return true;
    }

    void setDesktopWindow( HWND hwndDesktop )
    {
        PrivateData::hwndDesktop = hwndDesktop;
    }

    void setLanguage( Language eLanguage )
    {
        PrivateData::eLanguage = eLanguage;
    }

    void setSessionID( const char *szSessionID )
    {   
        assert( strlen( szSessionID ) + sizeof(sessionIdArgHeader) < sizeof( PrivateData::szSessionID ) );
        strcpy( PrivateData::szSessionID, sessionIdArgHeader );
        strcat( PrivateData::szSessionID, szSessionID );
    }

    void setUserName( const char *szUserName )
    {
        assert( strlen( szUserName ) + sizeof(userNameArgHeader) < sizeof( PrivateData::szUserName ) );
        strcpy( PrivateData::szUserName, userNameArgHeader );
        strcat( PrivateData::szUserName, szUserName );
    }

    void setCharacterName( const char *szCharacterName )
    {
        assert( strlen( szCharacterName ) + sizeof(charnameArgHeader) < sizeof( PrivateData::szCharacterName ) );
        strcpy( PrivateData::szCharacterName, charnameArgHeader );
        strcat( PrivateData::szCharacterName, szCharacterName );
    }

    void setChallenge( const char *szChallengeKey, const bool bIsFounder )
    {
        if( !szChallengeKey || *szChallengeKey == 0 )
            PrivateData::szChallenge[0] = 0;
        else
        {
            assert( strlen( szChallengeKey ) + sizeof(challengeArgHeader) < sizeof( PrivateData::szChallenge ) );
            strcpy( PrivateData::szChallenge, challengeArgHeader );
            strcat( PrivateData::szChallenge, szChallengeKey );
            PrivateData::bIsChallengeFounder = bIsFounder;
        }
    }

    void setStartTutorial( const bool bStartTutorial )
    {
        PrivateData::bStartTutorial = bStartTutorial;
    }

    void setWindowState( WindowState eWindowState )
    {
        pOnWindowStateChangedProc( eWindowState );
    }

    void onMusicCompletion()
    {
        pOnMusicCompletionProc();
    }

    void setNavigateCallback( NavigateProc navigateProc )
    {
        PrivateData::callbackTable.navigateProc = navigateProc;
    }

    void setNavigateWithPostDataCallback( NavigateWithPostDataProc navigateWithPostDataProc )
    {
        PrivateData::callbackTable.navigateWithPostDataProc = navigateWithPostDataProc;
    }

    void setPlaySoundCallback( PlaySoundProc playSoundProc )
    {
        PrivateData::callbackTable.playSoundProc = playSoundProc;
    }

    void setPlayMusicCallback( PlayMusicProc playMusicProc )
    {
        PrivateData::callbackTable.playMusicProc = playMusicProc;
    }

    void setSetSoundVolumeCallback( SetSoundVolumeProc setSoundVolumeProc )
    {
        PrivateData::callbackTable.setSoundVolumeProc = setSoundVolumeProc;
    }
    
    void setSetMusicVolumeCallback( SetMusicVolumeProc setMusicVolumeProc )
    {
        PrivateData::callbackTable.setMusicVolumeProc = setMusicVolumeProc;
    }

    void setStopAllSoundsCallback( StopAllSoundsProc stopAllSoundsProc )
    {
        PrivateData::callbackTable.stopAllSoundsProc = stopAllSoundsProc;
    }

    void setSetWindowStateCallback( SetWindowStateProc setWindowStateProc )
    {
        PrivateData::callbackTable.setWindowStateProc = setWindowStateProc;
    }

    DWORD WINAPI tcgHintThreadProc( LPVOID );

    void hintPrepareToLaunch()
    {
        if( 0 == PrivateData::hLoaderThread )
        {
            DWORD dwThreadId;
            PrivateData::hShutdownLoaderThread = CreateEvent( 0, TRUE, FALSE, 0 );
            PrivateData::hLoaderThread         = CreateThread( 0, 4096, tcgHintThreadProc, 0, 0, &dwThreadId );
        }
    }

    void waitForLoaderThread()
    {
        if( PrivateData::hLoaderThread )
        {
            SetEvent( PrivateData::hShutdownLoaderThread );
            WaitForSingleObject( PrivateData::hLoaderThread, INFINITE );
            CloseHandle( PrivateData::hLoaderThread );
            PrivateData::hLoaderThread = 0;

            CloseHandle( PrivateData::hShutdownLoaderThread );
            PrivateData::hShutdownLoaderThread = 0;
        }
    }

    void unloadPreloadedDLLs()
    {
        // Release any extra handles we got from the preload
        for( int i = 0; i != PrivateData::NUM_DLLS_TO_LOAD; ++i )
        {
            if( HMODULE &rHandle = PrivateData::aDLLHandles[ i ] )
            {
                FreeLibrary( rHandle );
                rHandle = 0;
            }
        }
    }

    void hintAbortLaunch()
    {
        waitForLoaderThread();
        unloadPreloadedDLLs();
    }
    
    bool launch()
    {
        waitForLoaderThread();

        if( !g_pManager )
        {
            g_pManager = new Manager;

            if( !g_pManager->init() )
            {
                delete g_pManager;
                g_pManager = 0;
            }

            return 0 != g_pManager;
        }

        return true;
    }

    bool isLaunched()
    {
        return 0 != g_pManager;
    }

    //------------------------------------------------------------------------------
    void update()
    {
        if( g_pManager )
            g_pManager->update();
    }

    //------------------------------------------------------------------------------
    void release()
    {
        if( g_pManager )
        {
            g_pManager->shutdown();
            delete g_pManager;
            g_pManager = 0;
        }
    }

    //------------------------------------------------------------------------------
    unsigned getWindows( Window **pWindows, unsigned uNumWindows )
    {
        return g_pManager ? g_pManager->getWindows( pWindows, uNumWindows ) : 0;
    }

    //------------------------------------------------------------------------------
    Window *getCaptureWindow()
    {
        return g_pManager ? g_pManager->getWindow( pGetCaptureWindowProc() ) : 0;
    }

    //------------------------------------------------------------------------------
    HCURSOR getCursor()
    {
        return pGetCursorProc();
    }
    
    //------------------------------------------------------------------------------
    // Manager
    //------------------------------------------------------------------------------
    Manager::Manager()
        : m_hDLL( 0 ),
          m_ppWindows( 0 ),
          m_uNumWindows( 0 )
    {   
    }

    //------------------------------------------------------------------------------
    Manager::~Manager()
    {   
        delete [] m_ppWindows;
    }
    
    //------------------------------------------------------------------------------
    bool Manager::init()
    {
        char fullpath[_MAX_PATH];
        strncpy( fullpath, PrivateData::szApplicationDirectory, sizeof( fullpath ) );
        fullpath[ sizeof( fullpath ) - 1 ] = 0;

        strncat( fullpath, SELECT_RELEASE_OR_DEBUG( "\\SWGTCG.dll", "\\SWGTCG.dll" ), sizeof( fullpath ) - strlen( fullpath ) );
        fullpath[ sizeof( fullpath ) - 1 ] = 0;

        m_hDLL = LoadLibraryEx( fullpath, 0, LOAD_WITH_ALTERED_SEARCH_PATH );

        if( !m_hDLL )
		{
			char buffer[256];
			memset(buffer, 0, 256);
			sprintf(buffer, "Failed to load SWGTCG.dll from %s. Please check your install.\n", fullpath);
			MessageBox(NULL, buffer, "Failed to load DLL", MB_OK);
            return false;
		}

        bool bSuccess = true;

#define MAP_PROC( var, name ) (*(void **)&var) = GetProcAddress( m_hDLL, name ); bSuccess = bSuccess && var != 0;

        MAP_PROC( pInitializeProc,        "Initialize" );
        MAP_PROC( pRunFrameProc,          "RunFrame" );
        MAP_PROC( pShutdownProc,          "Shutdown" );
        MAP_PROC( pGetWindowsProc,        "GetWindows" );
        MAP_PROC( pGetCaptureWindowProc,  "GetCaptureWindow" );
        MAP_PROC( pGetCursorProc,         "GetCurrentCursor" );
        MAP_PROC( pGetWindowRepaintRects, "GetWindowRepaintRects" );
        MAP_PROC( pGetWindowSurfaceData,  "GetWindowSurfaceData" );
        MAP_PROC( pOnMouseEventProc,      "OnMouseEvent" );
        MAP_PROC( pOnMouseWheelEventProc, "OnMouseWheelEvent" );
        MAP_PROC( pOnKeyEventProc,        "OnKeyEvent" );
        MAP_PROC( pOnFocusProc,           "OnFocus" );
        MAP_PROC( pOnWindowStateChangedProc,"OnWindowStateChanged" );
        MAP_PROC( pOnMusicCompletionProc, "OnMusicCompletion" );

#undef MAP_PROC

        if( !bSuccess )
        {
            FreeLibrary( m_hDLL );
            m_hDLL = 0;
			char buffer[256];
			memset(buffer, 0, 256);
			sprintf(buffer, "Could not map to SWGTCG.dll processes. Please check your install and make sure you have the latest version.\n", fullpath);
			MessageBox(NULL, buffer, "Failed to load DLL", MB_OK);
            return false;
        }

        // Game keeps a pointer to this table, so we better make sure it's valid - DON
        static const char *argv[10];
        int argc = 0;

        argv[argc++] = PrivateData::szApplicationDirectory; // argc = 0

        if( PrivateData::szUserName[0] )
            argv[ argc++ ] = PrivateData::szUserName;       // argc = 1

        if( PrivateData::szSessionID[0] )
            argv[ argc++ ] = PrivateData::szSessionID;      // argc = 2

        if( PrivateData::szChallenge[0] )
        {
            argv[ argc++ ] = PrivateData::szChallenge;      // argc = 3
            argv[ argc++ ] = PrivateData::bIsChallengeFounder ? "--is-founder=true" : "--is-founder=false";  // argc = 4
        }

        if( PrivateData::szCharacterName[0] )
            argv[ argc++ ] = PrivateData::szCharacterName;  // argc = 5

        if( PrivateData::bStartTutorial )
            argv[ argc++ ] = "--post-login=tutorial";         // argc = 6

        // TEMP: connect to stage environment
        switch( PrivateData::eRealmType )
        {
            case REALM_Live: break;
            case REALM_Stage: argv[ argc++ ] = "--realm=stage"; break; // argc = 7
        }

        switch( PrivateData::eHostProcessType )
        {
            case HPT_EverQuest:        argv[ argc++ ] = "--host=eq" ; break;  // argc = 8
            case HPT_EverQuestII:      argv[ argc++ ] = "--host=eq2"; break; // argc = 8
			case HPT_StarWarsGalaxies: argv[ argc++ ] = "--host=swg"; break; // argc = 8
		}

        switch( PrivateData::eLanguage )
        {
            case LANG_French: argv[ argc++ ] = "--lang=fr_FR"; break; // argc = 9
            case LANG_German: argv[ argc++ ] = "--lang=de_DE"; break; // argc = 9
        }

        assert( argc < sizeof( argv ) / sizeof( *argv ) );
        argv[argc] = 0; // argc = 10

        pInitializeProc( argc, argv, PrivateData::hwndDesktop, &PrivateData::callbackTable );
        return true;
    }

    //------------------------------------------------------------------------------
    void Manager::update()
    {
        pRunFrameProc();

        unsigned uNumWindows = pGetWindowsProc(0,0);
        unsigned *pWindowIds = 0;

        if( uNumWindows )
        {
            pWindowIds = (unsigned *)alloca( uNumWindows * sizeof( unsigned ) );
            pGetWindowsProc( pWindowIds, uNumWindows );
        }

        // Look for destroyed windows
        for( unsigned i = 0; i != m_uNumWindows; /* increment in body */ )
        {
            unsigned uOldWindowId = m_ppWindows[i]->m_pImpl->m_uWindowId;

            for( unsigned j = 0; j != uNumWindows; ++j )
            {
                if( uOldWindowId == pWindowIds[j] )
                {
                    uOldWindowId = 0;
                    break;
                }
            }

            if( uOldWindowId )
                destroyWindow( uOldWindowId );
            else
                ++i;
        }

        // Look for newly created windows
        WINDOWINFO winfo = { sizeof( WINDOWINFO ) };

        for( unsigned i = 0; i != uNumWindows; ++i )
        {
            unsigned uWindowId = pWindowIds[ i ];
            Window *pWindow = getWindow( uWindowId );

            GetWindowInfo( *(HWND*)&uWindowId, &winfo );
            
            int      iX      = winfo.rcClient.left;
            int      iY      = winfo.rcClient.top;
            unsigned uWidth  = winfo.rcClient.right  - winfo.rcClient.left;
            unsigned uHeight = winfo.rcClient.bottom - winfo.rcClient.top;

            if( !pWindow )
            {
                createWindow( uWindowId, iX, iY, uWidth, uHeight );
            }
            else
            {
                pWindow->m_iX      = iX;
                pWindow->m_iY      = iY;
                pWindow->m_uWidth  = uWidth;
                pWindow->m_uHeight = uHeight;
            }
        }
    }

    //------------------------------------------------------------------------------
    void Manager::shutdown()
    {
        if( m_hDLL )
        {
            pShutdownProc();

            FreeLibrary( m_hDLL );
            m_hDLL = 0;
        }

        unloadPreloadedDLLs();

        // Qt has a problem where it keeps its image plugins loaded, which bloats our memory after we shut down.  We have to tear them down manually.        
        if( HMODULE hModule = GetModuleHandle( SELECT_RELEASE_OR_DEBUG( "qgif4.dll", "qgifd4.dll" ) ) )
            FreeLibrary( hModule );

        if( HMODULE hModule = GetModuleHandle( SELECT_RELEASE_OR_DEBUG( "qjpeg4.dll", "qjpegd4.dll" ) ) )
            FreeLibrary( hModule );

        if( HMODULE hModule = GetModuleHandle( SELECT_RELEASE_OR_DEBUG( "qmng4.dll", "qmngd4.dll" ) ) )
            FreeLibrary( hModule );

        if( HMODULE hModule = GetModuleHandle( SELECT_RELEASE_OR_DEBUG( "qtiff4.dll", "qtiffd4.dll" ) ) )
            FreeLibrary( hModule );
    }

    //------------------------------------------------------------------------------
    unsigned Manager::getWindows( Window **pWindows, unsigned uNumWindows )
    {
        if( unsigned uCopyNum = m_uNumWindows > uNumWindows ? uNumWindows : m_uNumWindows )
            memcpy( pWindows, m_ppWindows, uCopyNum * sizeof( Window * ) );

        return m_uNumWindows;
    }

    //------------------------------------------------------------------------------
    Window *Manager::getWindow( unsigned uWindowId )
    {
        for( Window **ppWindow = m_ppWindows, **ppEnd = m_ppWindows + m_uNumWindows; ppWindow != ppEnd; ++ppWindow )
        {
            if( uWindowId == (*ppWindow)->m_pImpl->m_uWindowId )
                return (*ppWindow );
        }

        return 0;
    }

    //------------------------------------------------------------------------------
    Window *Manager::createWindow( unsigned uWindowId, int iX, int iY, unsigned uWidth, unsigned uHeight )
    {
        Window *pWindow = new Window;

        if( !pWindow )
            return 0;

        if( !pWindow->m_pImpl->init( uWindowId ) )
        {
            delete pWindow;
            return 0;
        }

        pWindow->m_iX      = iX;
        pWindow->m_iY      = iY;
        pWindow->m_uWidth  = uWidth;
        pWindow->m_uHeight = uHeight;

        Window **ppWindows = new Window *[ m_uNumWindows + 1 ];
        memcpy( ppWindows, m_ppWindows, m_uNumWindows * sizeof( Window * ) );
        ppWindows[ m_uNumWindows ] = pWindow;
        ++m_uNumWindows;

        delete m_ppWindows;
        m_ppWindows = ppWindows;

        return pWindow;
    }

    //------------------------------------------------------------------------------
    void Manager::destroyWindow( unsigned uWindowId )
    {
        for( Window **ppWindow = m_ppWindows, **ppEnd = m_ppWindows + m_uNumWindows; ppWindow != ppEnd; ++ppWindow )
        {
            Window *pWindow = *ppWindow;

            if( uWindowId == pWindow->m_pImpl->m_uWindowId )
            {
                if( ppWindow + 1 != ppEnd )
                {
                    memmove( ppWindow, ppWindow + 1, (ppEnd - ppWindow + 1) * sizeof( Window * ) );
                }

                --m_uNumWindows;
                return;
            }
        }
    }

    
    //------------------------------------------------------------------------------
    // Window
    //------------------------------------------------------------------------------
    Window::Window()
        : m_pImpl( 0 ),
          m_pUserData( 0 )
    {
        m_pImpl = new WindowImpl( this );
    }

    //------------------------------------------------------------------------------
    Window::~Window()
    {
        delete m_pImpl;
    }

    //------------------------------------------------------------------------------
    bool Window::canGetFocus() const
    {
        return m_pImpl->canGetFocus();
    }

    //------------------------------------------------------------------------------
    void Window::getTitle( char *pTitle, unsigned uBufferLen ) const
    {
        m_pImpl->getTitle( pTitle, uBufferLen );
    }

    //------------------------------------------------------------------------------
    void Window::setFocus( bool bFocus )
    {
        m_pImpl->setFocus( bFocus );
    }

    //------------------------------------------------------------------------------
    void Window::setLocation( int x, int y )
    {
        m_pImpl->setLocation( x, y );
    }

    //------------------------------------------------------------------------------
    void Window::setSize( unsigned width, unsigned height )
    {
        m_pImpl->setSize( width, height );
    }

    //------------------------------------------------------------------------------
    void Window::getMinMaxInfo( unsigned &minWidth, unsigned &minHeight, unsigned &maxWidth, unsigned &maxHeight )
    {
        m_pImpl->getMinMaxInfo( minWidth, minHeight, maxWidth, maxHeight );
    }

    //------------------------------------------------------------------------------
    unsigned Window::getWindowRepaintRects( RECT *pRects, unsigned uNumRects )
    {
        return m_pImpl->getWindowRepaintRects( pRects, uNumRects );
    }

    //------------------------------------------------------------------------------
    bool Window::getWindowSurfaceData( void **ppBits, unsigned *pWidth, unsigned *pHeight, unsigned *pStride )
    {
        return m_pImpl->getWindowSurfaceData( ppBits, pWidth, pHeight, pStride );
    }

    //------------------------------------------------------------------------------
    void Window::close()
    {
        m_pImpl->close();
    }

    //------------------------------------------------------------------------------
    void Window::onLeftMouseDown( int x, int y, int gx, int gy, unsigned uFlags )
    {
        m_pImpl->onLeftMouseDown( x, y, gx, gy, uFlags );
    }
    
    //------------------------------------------------------------------------------
    void Window::onLeftMouseUp( int x, int y, int gx, int gy, unsigned uFlags )
    {
        m_pImpl->onLeftMouseUp( x, y, gx, gy, uFlags );
    }

    //------------------------------------------------------------------------------
    void Window::onLeftMouseDoubleClick( int x, int y, int gx, int gy, unsigned uFlags )
    {
        m_pImpl->onLeftMouseDoubleClick( x, y, gx, gy, uFlags );
    }
    
    //------------------------------------------------------------------------------
    void Window::onMiddleMouseDown( int x, int y, int gx, int gy, unsigned uFlags )
    {
        m_pImpl->onMiddleMouseDown( x, y, gx, gy, uFlags );
    }
    
    //------------------------------------------------------------------------------
    void Window::onMiddleMouseUp( int x, int y, int gx, int gy, unsigned uFlags )
    {
        m_pImpl->onMiddleMouseUp( x, y, gx, gy, uFlags );
    }

    //------------------------------------------------------------------------------
    void Window::onMiddleMouseDoubleClick( int x, int y, int gx, int gy, unsigned uFlags )
    {
        m_pImpl->onMiddleMouseDoubleClick( x, y, gx, gy, uFlags );
    }
    
    //------------------------------------------------------------------------------
    void Window::onRightMouseDown( int x, int y, int gx, int gy, unsigned uFlags )
    {
        m_pImpl->onRightMouseDown( x, y, gx, gy, uFlags );
    }
    
    //------------------------------------------------------------------------------
    void Window::onRightMouseUp( int x, int y, int gx, int gy, unsigned uFlags )
    {
        m_pImpl->onRightMouseUp( x, y, gx, gy, uFlags );
    }

    //------------------------------------------------------------------------------
    void Window::onRightMouseDoubleClick( int x, int y, int gx, int gy, unsigned uFlags )
    {
        m_pImpl->onRightMouseDoubleClick( x, y, gx, gy, uFlags );
    }
    
    //------------------------------------------------------------------------------
    void Window::onMouseMove( int x, int y, int gx, int gy, unsigned uFlags )
    {
        m_pImpl->onMouseMove( x, y, gx, gy, uFlags );
    }

    //------------------------------------------------------------------------------
    void Window::onMouseWheel( int x, int y, int gx, int gy, int ticks, unsigned uFlags )
    {
        m_pImpl->onMouseWheel( x, y, gx, gy, ticks, uFlags );
    }

    //------------------------------------------------------------------------------
    bool Window::onKeyDown( int key, unsigned uFlags, int code, int vkey, int nativeMods )
    {
        return m_pImpl->onKeyDown( key, uFlags, code, vkey, nativeMods );
    }

    //------------------------------------------------------------------------------
    bool Window::onKeyUp( int key, unsigned uFlags, int code, int vkey, int nativeMods )
    {
        return m_pImpl->onKeyUp( key, uFlags, code, vkey, nativeMods );
    }

    //------------------------------------------------------------------------------
    // WindowImpl
    //------------------------------------------------------------------------------
    WindowImpl::WindowImpl( Window *pWindow )
        : m_pWindow( pWindow ),
          m_uWindowId( 0 ),
          m_hWnd( 0 )
    {
    }

    //------------------------------------------------------------------------------
    WindowImpl::~WindowImpl()
    {
    }

    //------------------------------------------------------------------------------
    bool WindowImpl::init( unsigned uWindowId )
    {
        m_uWindowId = uWindowId;
        m_hWnd      = *(HWND*)&m_uWindowId;
        return true;
    }

    //------------------------------------------------------------------------------
    bool WindowImpl::canGetFocus() const
    {
        char szClassName[257];
        GetClassName( m_hWnd, szClassName, sizeof( szClassName ) / sizeof( szClassName[0] ) );

        if( 0 == strcmp( "QToolTip", szClassName ) )
            return false;

        return true;
    }

    //------------------------------------------------------------------------------
    void WindowImpl::getTitle( char *pTitle, unsigned uBufferLen ) const
    {
        GetWindowText( m_hWnd, pTitle, uBufferLen );
    }

    //------------------------------------------------------------------------------
    void WindowImpl::setFocus( bool bFocus )
    {
        pOnFocusProc( m_uWindowId, bFocus ? 1 : 0 );
    }

    //------------------------------------------------------------------------------
    void WindowImpl::setLocation( int x, int y )
    {
        WINDOWINFO winfo = { sizeof( WINDOWINFO ) };
        GetWindowInfo( m_hWnd, &winfo );

        if( x != winfo.rcWindow.left ||
            y != winfo.rcWindow.top )
        {
            SetWindowPos( m_hWnd, 0, x, y, 0, 0, SWP_NOACTIVATE | SWP_NOSIZE | SWP_NOZORDER );
        }
    }

    //------------------------------------------------------------------------------
    void WindowImpl::setSize( unsigned width, unsigned height )
    {
        WINDOWINFO winfo = { sizeof( WINDOWINFO ) };
        GetWindowInfo( m_hWnd, &winfo );

        if( width  != static_cast<unsigned>(winfo.rcWindow.right  - winfo.rcWindow.left) ||
            height != static_cast<unsigned>(winfo.rcWindow.bottom - winfo.rcWindow.top) )
        {
            SetWindowPos( m_hWnd, 0, 0, 0, width, height, SWP_NOACTIVATE | SWP_NOMOVE | SWP_NOZORDER );
        }
    }

    //------------------------------------------------------------------------------
    void WindowImpl::getMinMaxInfo( unsigned &minWidth, unsigned &minHeight, unsigned &maxWidth, unsigned &maxHeight )
    {
        MINMAXINFO minMaxInfo;
        minMaxInfo.ptMinTrackSize.x = GetSystemMetrics( SM_CXMINTRACK );
        minMaxInfo.ptMinTrackSize.y = GetSystemMetrics( SM_CYMINTRACK );
        minMaxInfo.ptMaxTrackSize.x = 64000; // No limit on these
        minMaxInfo.ptMaxTrackSize.y = 64000; // No limit on these
        SendMessage( m_hWnd, WM_GETMINMAXINFO, 0, (LPARAM)&minMaxInfo );

        WINDOWINFO winfo = { sizeof( WINDOWINFO ) };
        GetWindowInfo( m_hWnd, &winfo );

        {
            int framex = (winfo.rcWindow.right - winfo.rcWindow.left) - (winfo.rcClient.right - winfo.rcClient.left);
            minWidth = minMaxInfo.ptMinTrackSize.x - framex;
            maxWidth = minMaxInfo.ptMaxTrackSize.x - framex;
        }
        {
            int framey = (winfo.rcWindow.bottom - winfo.rcWindow.top) - (winfo.rcClient.bottom - winfo.rcClient.top);
            minHeight = minMaxInfo.ptMinTrackSize.y - framey;
            maxHeight = minMaxInfo.ptMaxTrackSize.y - framey;
        }
    }

    //------------------------------------------------------------------------------
    unsigned WindowImpl::getWindowRepaintRects( RECT *pRects, unsigned uNumRects )
    {
        return pGetWindowRepaintRects( m_uWindowId, pRects, uNumRects );
    }

    //------------------------------------------------------------------------------
    bool WindowImpl::getWindowSurfaceData( void **ppBits, unsigned *pWidth, unsigned *pHeight, unsigned *pStride )
    {
        return pGetWindowSurfaceData( m_uWindowId, ppBits, pWidth, pHeight, pStride ) != 0;
    }

    //------------------------------------------------------------------------------
    void WindowImpl::close()
    {
        SendMessage( m_hWnd, WM_CLOSE, 0, 0 );
    }

    //------------------------------------------------------------------------------
    void WindowImpl::onLeftMouseDown( int x, int y, int gx, int gy, unsigned uFlags )
    {
        pOnMouseEventProc( m_uWindowId, MouseButtonPress, x, y, gx, gy, Window::LeftButton, uFlags & Window::MouseMask, uFlags & Window::KeyboardMask );
    }
    
    //------------------------------------------------------------------------------
    void WindowImpl::onLeftMouseUp( int x, int y, int gx, int gy, unsigned uFlags )
    {
        pOnMouseEventProc( m_uWindowId, MouseButtonRelease, x, y, gx, gy, Window::LeftButton, uFlags & Window::MouseMask, uFlags & Window::KeyboardMask );
    }

    //------------------------------------------------------------------------------
    void WindowImpl::onLeftMouseDoubleClick( int x, int y, int gx, int gy, unsigned uFlags )
    {
        pOnMouseEventProc( m_uWindowId, MouseButtonDblClick, x, y, gx, gy, Window::LeftButton, uFlags & Window::MouseMask, uFlags & Window::KeyboardMask );
    }
    
    //------------------------------------------------------------------------------
    void WindowImpl::onMiddleMouseDown( int x, int y, int gx, int gy, unsigned uFlags )
    {
        pOnMouseEventProc( m_uWindowId, MouseButtonPress, x, y, gx, gy, Window::MiddleButton, uFlags & Window::MouseMask, uFlags & Window::KeyboardMask );
    }
    
    //------------------------------------------------------------------------------
    void WindowImpl::onMiddleMouseUp( int x, int y, int gx, int gy, unsigned uFlags )
    {
        pOnMouseEventProc( m_uWindowId, MouseButtonRelease, x, y, gx, gy, Window::MiddleButton, uFlags & Window::MouseMask, uFlags & Window::KeyboardMask );
    }

    //------------------------------------------------------------------------------
    void WindowImpl::onMiddleMouseDoubleClick( int x, int y, int gx, int gy, unsigned uFlags )
    {
        pOnMouseEventProc( m_uWindowId, MouseButtonDblClick, x, y, gx, gy, Window::MiddleButton, uFlags & Window::MouseMask, uFlags & Window::KeyboardMask );
    }
    
    //------------------------------------------------------------------------------
    void WindowImpl::onRightMouseDown( int x, int y, int gx, int gy, unsigned uFlags )
    {
        pOnMouseEventProc( m_uWindowId, MouseButtonPress, x, y, gx, gy, Window::RightButton, uFlags & Window::MouseMask, uFlags & Window::KeyboardMask );
    }
    
    //------------------------------------------------------------------------------
    void WindowImpl::onRightMouseUp( int x, int y, int gx, int gy, unsigned uFlags )
    {
        pOnMouseEventProc( m_uWindowId, MouseButtonRelease, x, y, gx, gy, Window::RightButton, uFlags & Window::MouseMask, uFlags & Window::KeyboardMask );
    }

    //------------------------------------------------------------------------------
    void WindowImpl::onRightMouseDoubleClick( int x, int y, int gx, int gy, unsigned uFlags )
    {
        pOnMouseEventProc( m_uWindowId, MouseButtonDblClick, x, y, gx, gy, Window::RightButton, uFlags & Window::MouseMask, uFlags & Window::KeyboardMask );
    }
    
    //------------------------------------------------------------------------------
    void WindowImpl::onMouseMove( int x, int y, int gx, int gy, unsigned uFlags )
    {
        pOnMouseEventProc( m_uWindowId, MouseMove, x, y, gx, gy, 0, uFlags & Window::MouseMask, uFlags & Window::KeyboardMask );
    }

    //------------------------------------------------------------------------------
    void WindowImpl::onMouseWheel( int x, int y, int gx, int gy, int ticks, unsigned uFlags )
    {
        pOnMouseWheelEventProc( m_uWindowId, x, y, gx, gy, ticks, uFlags & Window::MouseMask, uFlags & Window::KeyboardMask );
    }

    //------------------------------------------------------------------------------
    bool WindowImpl::onKeyDown( int key, unsigned uFlags, int code, int vkey, int nativeMods )
    {
        return 0 != pOnKeyEventProc( KeyPress, key, uFlags & Window::KeyboardMask, code, vkey, nativeMods );
    }

    //------------------------------------------------------------------------------
    bool WindowImpl::onKeyUp( int key, unsigned uFlags, int code, int vkey, int nativeMods )
    {
        return 0 != pOnKeyEventProc( KeyRelease, key, uFlags & Window::KeyboardMask, code, vkey, nativeMods );
    }

    //------------------------------------------------------------------------------
    int preloadFile( const char *fileName )
    {
        unsigned int sum = 0;

        HANDLE hFile = CreateFile( fileName, GENERIC_READ, FILE_SHARE_READ, 0, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL | FILE_FLAG_SEQUENTIAL_SCAN, 0 );

        if( INVALID_HANDLE_VALUE != hFile )
        {
            DWORD dwFileSize = GetFileSize( hFile,  NULL );
            HANDLE hMapping = CreateFileMapping( hFile, 0, PAGE_READONLY | SEC_COMMIT, 0, dwFileSize, 0 );

            if( INVALID_HANDLE_VALUE != hMapping )
            {
                unsigned char* pData = reinterpret_cast<unsigned char*>( MapViewOfFile( hMapping, FILE_MAP_READ, 0, 0, dwFileSize ) );

                if( pData )
                {
                    for( unsigned char *p = pData, *end = pData + dwFileSize; p < end; p += 4096 )
                        sum += *(int *)p;

                    UnmapViewOfFile( pData );
                }

                CloseHandle( hMapping );
            }

            CloseHandle( hFile );
        }

        return sum;
    }

    //------------------------------------------------------------------------------
    DWORD WINAPI tcgHintThreadProc( LPVOID )
    {
        // WARNING, THIS THREAD HAS A VERY SMALL STACK
        SetThreadPriority( GetCurrentThread(), THREAD_PRIORITY_HIGHEST );

        size_t iMaxFileNameLen = 0;
        for( int i = 0; i != PrivateData::NUM_FILES_TO_MAP; ++i )
        {
            size_t iFileNameLen = strlen( PrivateData::aFilesToMap[i] );

            if( iMaxFileNameLen < iFileNameLen )
                iMaxFileNameLen = iFileNameLen;
        }

        for( int i = 0; i != PrivateData::NUM_DLLS_TO_LOAD; ++i )
        {
            size_t iFileNameLen = strlen( PrivateData::aDLLsToLoad[ i ] );

            if( iMaxFileNameLen < iFileNameLen )
                iMaxFileNameLen = iFileNameLen;
        }

        size_t iDirectoryNameLen = strlen( PrivateData::szApplicationDirectory );
        char *buffer = reinterpret_cast< char * >( alloca( iDirectoryNameLen + iMaxFileNameLen + 2 ) );
        memcpy( buffer, PrivateData::szApplicationDirectory, iDirectoryNameLen );
        buffer[ iDirectoryNameLen ] = '\\';

        for( int i = 0; WaitForSingleObject( PrivateData::hShutdownLoaderThread, 0 ) == WAIT_TIMEOUT && i != PrivateData::NUM_DLLS_TO_LOAD; ++i )
        {
            strcpy( buffer + iDirectoryNameLen + 1, PrivateData::aDLLsToLoad[ i ] );
            preloadFile( buffer );
            PrivateData::aDLLHandles[ i ] = LoadLibraryEx( buffer, 0, LOAD_WITH_ALTERED_SEARCH_PATH );
        }

        for( int i = 0; WaitForSingleObject( PrivateData::hShutdownLoaderThread, 0 ) == WAIT_TIMEOUT && i != PrivateData::NUM_FILES_TO_MAP; ++i )
        {
            strcpy( buffer + iDirectoryNameLen + 1, PrivateData::aFilesToMap[ i ] );
            preloadFile( buffer );
        }

        return 0;
    }

}; // namespace

//#endif // ENABLE_TCG
