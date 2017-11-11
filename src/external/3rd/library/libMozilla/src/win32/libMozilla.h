#ifndef LIBMOZILLA_H
#define LIBMOZILLA_H

#include <wchar.h>

namespace libMozilla
{
    // Forward declarations
    class IBlitter;
    class ICallback;
    class Window;
    class Manager;

    //------------------------------------------------------------------------------
    // Library API
    //
    // Description:
    // Functions to manage library and object lifespan
    //
    // Expected Usage:
    // Call libMozilla::init once you have your application's main window created.
    // Call libMozilla::release before you destroy your application's main window.
    
    bool    init( void *pNativeWindow, const char *szApplicationDirectory );
    void    update();
    void    release();

    void    enableMemoryCache( bool bEnable );
    void    enableDiskCache( bool bEnable, unsigned uMaxSizeKB );
    void    setUserAgent( const char * );

    Window *createWindow( unsigned width, unsigned height );
    void    destroyWindow( Window *pWindow );
    
    enum Command
    {
        CommandUndo = 0,
        CommandRedo,
        CommandClearUndo,
        CommandCut,
        CommandCutOrDelete,
        CommandCopy,
        CommandCopyOrDelete,
        CommandSelectAll,
        CommandPaste,
        CommandSwitchTextDirection,
        CommandDelete,
        CommandDeleteCharBackward,
        CommandDeleteCharForward,
        CommandDeleteWordBackward,
        CommandDeleteWordForward,
        CommandDeleteToBeginningOfLine,
        CommandDeleteToEndOfLine,
        CommandScrollTop,
        CommandScrollBottom,
        CommandMoveTop,
        CommandMoveBottom,
        CommandSelectTop,
        CommandSelectBottom,
        CommandLineNext,
        CommandLinePrevious,
        CommandSelectLineNext,
        CommandSelectLinePrevious,
        CommandCharPrevious,
        CommandCharNext,
        CommandSelectCharPrevious,
        CommandSelectCharNext,
        CommandBeginLine,
        CommandEndLine,
        CommandSelectBeginLine,
        CommandSelectEndLine,
        CommandWordPrevious,
        CommandWordNext,
        CommandSelectWordPrevious,
        CommandSelectWordNext,
        CommandScrollPageUp,
        CommandScrollPageDown,
        CommandScrollLineUp,
        CommandScrollLineDown,
        CommandMovePageUp,
        CommandMovePageDown,
        CommandSelectPageUp,
        CommandSelectPageDown,
        CommandInsertText,
        CommandPasteQuote,

        NUM_COMMANDS
    };

    //------------------------------------------------------------------------------
    // Window
    //
    // Description:
    // Represents a running embedded browser.
    // Each browser instance has a single URI and a navigation history.
    //
    // Expected Usage:
    // Use libMozilla::createWindow after the library has been init'd to create a window.
    // Use libMozilla::destroyWindow .

    class Window
    {   
    public:

        enum eModFlags
        {
            SHIFT   = 1 << 0,
            CONTROL = 1 << 1,
            ALT     = 1 << 2
        };

        void           setSize( unsigned width, unsigned height );
        void           setFocus( bool bFocus );
        void           setCallback( ICallback * );
        void           setRenderOnComplete( bool bVal );

        float          getProgress( bool &bIsLoading );
        const wchar_t *getStatus() const;
        const char    *getURI() const;

        bool           getCaret( int &x, int &y, int &width, int &height );
        
        // Navigation
        void           navigateTo( const wchar_t *pURI, const char *pPostData = 0, const unsigned kuPostDataLength = 0 );
        void           navigateStop();
        bool           canNavigateBack();
        void           navigateBack();
        bool           canNavigateForward();
        void           navigateForward();
        void           reload();

        // Input
        void           onLeftMouseDown( int x, int y, unsigned uFlags );
        void           onLeftMouseUp( int x, int y, unsigned uFlags );
        void           onMiddleMouseDown( int x, int y, unsigned uFlags );
        void           onMiddleMouseUp( int x, int y, unsigned uFlags );
        void           onRightMouseDown( int x, int y, unsigned uFlags );
        void           onRightMouseUp( int x, int y, unsigned uFlags );
        void           onMouseMove( int x, int y, unsigned uFlags );
        void           onMouseWheel( int ticks, unsigned flags );
        void           onKeyPress( int charcode, int keycode, unsigned uFlags );
        
        void           onCommand( Command );

        // Rendering    
        bool           render( IBlitter * );

    private:
        friend Manager;

        Window();
        ~Window();

        class WindowImpl *m_pImpl;
    };

    //------------------------------------------------------------------------------
    // IBlitter
    //
    // Description:
    // Handles transfer of graphical data from Mozilla's draw surface to a user surface.
    //
    // Expected usage:
    // Derive a class from this interface and pass an instance of the derived class to Window::render

    class IBlitter
    {
    public:

        virtual void operator()( void *pBits, unsigned uWidth, unsigned uHeight, unsigned uStride, unsigned uBytesPerRow ) = 0;
    };

    class ICallback
    {
    public:

        virtual void onURIChanged( Window * ) = 0;
        virtual void onProgressChanged( Window * ) = 0;
        virtual void onStatusChanged( Window * ) = 0;
        virtual bool doValidateURI( Window *, const char * ) = 0; // return true if it's OK to load this URI
    };
}

#endif // LIBMOZILLA_H