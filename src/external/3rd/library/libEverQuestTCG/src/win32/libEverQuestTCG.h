//#if ENABLE_TCG

#ifndef LIBEVERQUESTTCG_H
#define LIBEVERQUESTTCG_H

#include <windows.h>

namespace libEverQuestTCG
{
    class Manager;
    class Window;
    class WindowImpl;

    enum AudioFormatType // Matches enum at coreclient\include\wasoundservice.h
    {
        WAVAudioFormat,
        MP3AudioFormat,
        OGGAudioFormat
    };

    enum WindowState // Matches enum at coreclient\include\coreclient.h
    {
        Normal,
        Minimized,
        Maximized
    };

    enum Language
    {
        LANG_English,
        LANG_French,
        LANG_German
    };

    // Types need to match coreclient\include\coreclient.h
    typedef void (__stdcall * NavigateProc)(const char *);
    typedef void (__stdcall * NavigateWithPostDataProc)(const char *, const char *);
    typedef void (__stdcall * PlaySoundProc)(char *buffer, unsigned uBufferLenInBytes, AudioFormatType eType);
    typedef void (__stdcall * PlayMusicProc)(char *buffer, unsigned uBufferLenInBytes, AudioFormatType eType);
    typedef void (__stdcall * SetSoundVolumeProc)(float fZeroToOne);
    typedef void (__stdcall * SetMusicVolumeProc)(float fZeroToOne);
    typedef void (__stdcall * StopAllSoundsProc)();
    typedef void (__stdcall * SetWindowStateProc)(int);

    enum HostProcessType
    {
        HPT_EverQuest,
        HPT_EverQuestII,
		HPT_StarWarsGalaxies
    };

    enum RealmType
    {
        REALM_Live,
        REALM_Stage,
    };

    bool init( const char *, HostProcessType, RealmType = REALM_Live );
    void setDesktopWindow( HWND ); // Set this to your top level window
    void setLanguage( Language ); // Default is English if you don't call this
    void setUserName( const char * );
    void setSessionID( const char * );
    void setCharacterName( const char * );
    void setChallenge( const char *key, const bool bIsFounder );
    void setStartTutorial( const bool );
    void setWindowState( WindowState );
    void onMusicCompletion();

    void setNavigateCallback( NavigateProc );
    void setNavigateWithPostDataCallback( NavigateWithPostDataProc );
    void setPlaySoundCallback( PlaySoundProc );
    void setPlayMusicCallback( PlayMusicProc );
    void setSetSoundVolumeCallback( SetSoundVolumeProc );
    void setSetMusicVolumeCallback( SetMusicVolumeProc );
    void setStopAllSoundsCallback( StopAllSoundsProc );
    void setSetWindowStateCallback( SetWindowStateProc );

    void hintPrepareToLaunch();
    void hintAbortLaunch();
    bool launch();
    bool isLaunched();
    void update();
    void release();

    unsigned getWindows( Window **pWindows, unsigned uNumWindows );
    Window *getCaptureWindow();
    HCURSOR getCursor();

    enum eSpecialKeys
    {
        // These values match those in Qt, don't mess with them - DON
        // If you need another key you can find it in qnamespace.h

        Key_Escape = 0x01000000,                // misc keys
        Key_Tab = 0x01000001,
        Key_Backtab = 0x01000002,
        Key_Backspace = 0x01000003,
        Key_Return = 0x01000004,
        Key_Enter = 0x01000005,
        Key_Insert = 0x01000006,
        Key_Delete = 0x01000007,
        Key_Pause = 0x01000008,
        Key_Print = 0x01000009,
        Key_SysReq = 0x0100000a,
        Key_Clear = 0x0100000b,
        Key_Home = 0x01000010,                // cursor movement
        Key_End = 0x01000011,
        Key_Left = 0x01000012,
        Key_Up = 0x01000013,
        Key_Right = 0x01000014,
        Key_Down = 0x01000015,
        Key_PageUp = 0x01000016,
        Key_PageDown = 0x01000017,
        Key_Shift = 0x01000020,                // modifiers
        Key_Control = 0x01000021,
        Key_Meta = 0x01000022,
        Key_Alt = 0x01000023,
        Key_CapsLock = 0x01000024,
        Key_NumLock = 0x01000025,
        Key_ScrollLock = 0x01000026,
    };

    class Window
    {
    public:

        enum eModFlags
        {
            // These values match those in Qt, don't mess with them - DON
            SHIFT        = 0x02000000,
            CONTROL      = 0x04000000,
            ALT          = 0x08000000,
            LeftButton   = 0x00000001, // Set if mouse button is down when event happens
            RightButton  = 0x00000002, // Set if mouse button is down when event happens
            MiddleButton = 0x00000004, // Set if mouse button is down when event happens

            KeyboardMask = 0xFF000000,
            MouseMask    = 0x000000FF,
        };
        
        bool           canGetFocus() const;
        void           getTitle( char *pTitle, unsigned uBufferLen ) const;

        void           setFocus( bool bFocus );
        void           setLocation( int x, int y );
        void           setSize( unsigned width, unsigned height );
        void           getRect( int &iX, int &iY, unsigned &uWidth, unsigned &uHeight ) const { iX = m_iX; iY = m_iY; uWidth = m_uWidth; uHeight = m_uHeight; }

        void *         getUserData() const { return m_pUserData; }
        void           setUserData( void *pUserData ) { m_pUserData = pUserData; }

        void           getMinMaxInfo( unsigned &minWidth, unsigned &minHeight, unsigned &maxWidth, unsigned &maxHeight );

        unsigned       getWindowRepaintRects( RECT *pRects, unsigned uNumRects );
        bool           getWindowSurfaceData( void **ppBits, unsigned *pWidth, unsigned *pHeight, unsigned *pStride );
        
        void           close();

        // Input
        void           onLeftMouseDown( int x, int y, int gx, int gy, unsigned uFlags );
        void           onLeftMouseUp( int x, int y, int gx, int gy, unsigned uFlags );
        void           onLeftMouseDoubleClick( int x, int y, int gx, int gy, unsigned uFlags );
        void           onMiddleMouseDown( int x, int y, int gx, int gy, unsigned uFlags );
        void           onMiddleMouseUp( int x, int y, int gx, int gy, unsigned uFlags );
        void           onMiddleMouseDoubleClick( int x, int y, int gx, int gy, unsigned uFlags );
        void           onRightMouseDown( int x, int y, int gx, int gy, unsigned uFlags );
        void           onRightMouseUp( int x, int y, int gx, int gy, unsigned uFlags );
        void           onRightMouseDoubleClick( int x, int y, int gx, int gy, unsigned uFlags );
        void           onMouseMove( int x, int y, int gx, int gy, unsigned uFlags );
        void           onMouseWheel( int x, int y, int gx, int gy, int ticks, unsigned uFlags );

        bool           onKeyDown( int key, unsigned uFlags, int code, int vkey, int nativeMods );
        bool           onKeyUp( int key, unsigned uFlags, int code, int vkey, int nativeMods );

    private:

        friend Manager;
        friend WindowImpl;

        Window();
        ~Window();

        WindowImpl *m_pImpl;
        void       *m_pUserData;
        int         m_iX;
        int         m_iY;
        unsigned    m_uHeight;
        unsigned    m_uWidth;
    };
};

#endif // LIBEVERQUESTTCG_H

//#endif // ENABLE_TCG