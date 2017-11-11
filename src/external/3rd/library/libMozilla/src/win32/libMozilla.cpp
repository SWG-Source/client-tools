#include "libMozilla.h"

// Define this if you want to link with the debug version of Mozilla in debug builds.  By default we link with release libs, even in
// debug builds because of DLL issues.  If you want to link with the debug Mozilla you will also need to use the libMozilla_to_x_copy_debug.bat
// script included with libMozilla to get the debug libraries into the right place on your X drive.  You will also need to modify the
// 'Additional Library Directories' in Librarian Settings to be lib\debug rather than lib\release
#define DEBUG_MOZILLA 0

#if !DEBUG_MOZILLA
#undef _DEBUG
#endif

#ifdef _DEBUG
// We need to define the DEBUG symbol before we include NS headers because their
// structures have members that are conditionally excluded based on #if DEBUG
// If we don't do this then any inline functions that get instantiated can be wrong
#define DEBUG 1
#endif

#include <mozilla-config.h>
#undef WINVER
#define WINVER 0x500

#define MOZILLA_INTERNAL_API 1

#include <nsCOMPtr.h>
#include <nsEmbedCID.h>
#include <nsGUIEvent.h>

// Interfaces
#include <nsIBadCertListener.h>
#include <nsIBaseWindow.h>
#include <nsIBrowserDOMWindow.h>
#include <nsICaret.h>
#include <nsIComboboxControlFrame.h>
#include <nsIContent.h>
#include <nsIController.h>
#include <nsIDocShell.h>
#include <nsIDocShellTreeItem.h>
#include <nsIDocument.h>
#include <nsIDOMDocument.h>
#include <nsIDOMElement.h>
#include <nsIDOMWindow.h>
#include <nsIEventQueue.h>
#include <nsIEventQueueService.h>
#include <nsIFocusController.h>
#include <nsIFrame.h>
#include <nsIHttpChannel.h>
#include <nsILocalFile.h>
#include <nsIPref.h>
#include <nsIInterfaceRequestor.h>
#include <nsIInterfaceRequestorUtils.h>
#include <nsIMIMEInputStream.h>
#include <nsIScriptGlobalObject.h>
#include <nsISelectionController.h>
#include <nsIScrollableView.h>
#include <nsIStringStream.h>
#include <nsIURIContentListener.h>
#include <nsIView.h>
#include <nsIViewManager.h>
#include <nsIWebBrowser.h>
#include <nsIWebBrowserChrome.h>
#include <nsIWebBrowserFocus.h>
#include <nsIWebProgress.h>
#include <nsIWebProgressListener.h>
#include <nsIWebNavigation.h>
#include <nsIWindowCreator2.h>
#include <nsIWindowWatcher.h>
#include <nsPIDOMWindow.h>

#include <nsPresContext.h>
#include <nsProfileDirServiceProvider.h>
#include <nsString.h>
#include <nsXPCOM.h>
#include <nsXULAppAPI.h>

#include <windows.h>

static char *s_nullString = "";
static bool  s_bEnableMemoryCache = true;
static bool  s_bEnableDiskCache = true;
static unsigned s_uMaxDiskCacheSizeKB = 1024 * 10;
static char  s_userAgent[32];

// Internal Declarations
namespace libMozilla
{
    //------------------------------------------------------------------------------
    // Manager
    //------------------------------------------------------------------------------
    class Manager :
        public nsIWindowCreator2
    {
    public:

                Manager();
               ~Manager();

        NS_DECL_ISUPPORTS
        NS_DECL_NSIWINDOWCREATOR
        NS_DECL_NSIWINDOWCREATOR2

        bool    init( void *pNativeWindow, const char *szApplicationDirectory );
        void    update();
        void    shutdown();

        Window *createWindow( unsigned width, unsigned height );
        void    destroyWindow( Window *pWindow );

        void   *getNativeWindow() { return m_pNativeWindow; }

        void    enableMemoryCache( bool );
        void    enableDiskCache( bool, unsigned uMaxSizeKB );
        void    setUserAgent( const char * );

    private:

        void                     *m_pNativeWindow;
        nsCOMPtr< nsIEventQueue > m_pEventQ;
    };

    //------------------------------------------------------------------------------
    // WindowImpl
    //------------------------------------------------------------------------------
    class WindowImpl :
        public nsIInterfaceRequestor,
        public nsIWebBrowserChrome,
        public nsSupportsWeakReference,
        public nsIWebProgressListener,
        public nsIURIContentListener,
        public nsIBadCertListener
    {
    public:

                 WindowImpl( Window *pWindow );
        virtual ~WindowImpl();

        NS_DECL_ISUPPORTS
        NS_DECL_NSIINTERFACEREQUESTOR
        NS_DECL_NSIWEBBROWSERCHROME
        NS_DECL_NSIWEBPROGRESSLISTENER
        NS_DECL_NSIURICONTENTLISTENER
        NS_DECL_NSIBADCERTLISTENER
        
        bool           init( void *pParentNativeWindow, int width, int height );

        void           setSize( unsigned width, unsigned height );
        void           setFocus( bool );
        void           setCallback( ICallback *pCallback );
        void           setRenderOnComplete( bool bVal );

        float          getProgress( bool &bIsLoading );
        const wchar_t *getStatus() const;
        const char    *getURI() const;

        bool           getCaret( int &x, int &y, int &width, int &height );
        
        // Navigation
        void           navigateTo( const PRUnichar *pURI, const char *pPostData, const unsigned kuPostDataLength );
        void           navigateStop();
        bool           canNavigateBack();
        void           navigateBack();
        bool           canNavigateForward();
        void           navigateForward();
        void           reload();

        // Input
        void           onMouseEvent( const PRUint32 event, const int x, const int y, const unsigned uFlags );
        void           onMouseWheel( const int iTicks, const unsigned uFlags );
        void           onKeyEvent( const PRUint32 iEvent, const PRUint32 iCharCode, const PRUint32 iKeyCode, const unsigned uFlags );

        void           onCommand( Command );

        // Rendering    
        bool           render( IBlitter *pBlitter );

    private:

        nsresult       m_getRootView( nsIViewManager **ppViewManager );
        nsresult       m_getPresentContext( nsPresContext **ppPresentContext );
        nsresult       m_doCommand( const char * );

    private:

        nsCOMPtr< nsIBaseWindow >    m_pBaseWindow;
        nsCOMPtr< nsIWebBrowser >    m_pWebBrowser;    
        nsCOMPtr< nsIWebNavigation > m_pWebNavigation;

        unsigned                     m_uTotalRequests;
        unsigned                     m_uFinishedRequests;
        bool                         m_bUseRealProgress;
        bool                         m_bDocumentLoaded;
        bool                         m_bRenderOnComplete;

        ICallback                   *m_pCallback;
        Window                      *m_pWindow;

        unsigned                     m_uWidth;
        unsigned                     m_uHeight;

        int                          m_iOffsetX;
        int                          m_iOffsetY;

        float                        m_fProgress;
        wchar_t                      m_aStatus[200];
        char                        *m_aURI;
    };
};

static struct
{
    void *pNativeWindow;
    char  szApplicationDirectory[ _MAX_PATH + 1 ];
} libPrivateData;

// Implementation
namespace libMozilla
{
    Manager *g_pManager = 0;
    char *aCommandTable[ NUM_COMMANDS ] = {};

    void initCommandTable()
    {
        aCommandTable[ CommandUndo ]                       = "cmd_undo";
        aCommandTable[ CommandRedo ]                       = "cmd_redo";
        aCommandTable[ CommandClearUndo ]                  = "cmd_clearUndo";
        aCommandTable[ CommandCut ]                        = "cmd_cut";
        aCommandTable[ CommandCutOrDelete ]                = "cmd_cutOrDelete";
        aCommandTable[ CommandCopy ]                       = "cmd_copy";
        aCommandTable[ CommandCopyOrDelete ]               = "cmd_copyOrDelete";
        aCommandTable[ CommandSelectAll ]                  = "cmd_selectAll";                  
        aCommandTable[ CommandPaste ]                      = "cmd_paste";
        aCommandTable[ CommandSwitchTextDirection ]        = "cmd_switchTextDirection";                  
        aCommandTable[ CommandDelete ]                     = "cmd_delete";
        aCommandTable[ CommandDeleteCharBackward ]         = "cmd_deleteCharBackward";
        aCommandTable[ CommandDeleteCharForward ]          = "cmd_deleteCharForward";
        aCommandTable[ CommandDeleteWordBackward ]         = "cmd_deleteWordBackward";
        aCommandTable[ CommandDeleteWordForward ]          = "cmd_deleteWordForward";
        aCommandTable[ CommandDeleteToBeginningOfLine ]    = "cmd_deleteToBeginningOfLine";
        aCommandTable[ CommandDeleteToEndOfLine ]          = "cmd_deleteToEndOfLine";
        aCommandTable[ CommandScrollTop ]                  = "cmd_scrollTop";
        aCommandTable[ CommandScrollBottom ]               = "cmd_scrollBottom";
        aCommandTable[ CommandMoveTop ]                    = "cmd_moveTop";
        aCommandTable[ CommandMoveBottom ]                 = "cmd_moveBottom";
        aCommandTable[ CommandSelectTop ]                  = "cmd_selectTop";
        aCommandTable[ CommandSelectBottom ]               = "cmd_selectBottom";
        aCommandTable[ CommandLineNext ]                   = "cmd_lineNext";
        aCommandTable[ CommandLinePrevious ]               = "cmd_linePrevious";
        aCommandTable[ CommandSelectLineNext ]             = "cmd_selectLineNext";
        aCommandTable[ CommandSelectLinePrevious ]         = "cmd_selectLinePrevious";
        aCommandTable[ CommandCharPrevious ]               = "cmd_charPrevious";
        aCommandTable[ CommandCharNext ]                   = "cmd_charNext";
        aCommandTable[ CommandSelectCharPrevious ]         = "cmd_selectCharPrevious";
        aCommandTable[ CommandSelectCharNext ]             = "cmd_selectCharNext";
        aCommandTable[ CommandBeginLine ]                  = "cmd_beginLine";
        aCommandTable[ CommandEndLine ]                    = "cmd_endLine";
        aCommandTable[ CommandSelectBeginLine ]            = "cmd_selectBeginLine";
        aCommandTable[ CommandSelectEndLine ]              = "cmd_selectEndLine";
        aCommandTable[ CommandWordPrevious ]               = "cmd_wordPrevious";
        aCommandTable[ CommandWordNext ]                   = "cmd_wordNext";
        aCommandTable[ CommandSelectWordPrevious ]         = "cmd_selectWordPrevious";
        aCommandTable[ CommandSelectWordNext ]             = "cmd_selectWordNext";
        aCommandTable[ CommandScrollPageUp ]               = "cmd_scrollPageUp";
        aCommandTable[ CommandScrollPageDown ]             = "cmd_scrollPageDown";
        aCommandTable[ CommandScrollLineUp ]               = "cmd_scrollLineUp";
        aCommandTable[ CommandScrollLineDown ]             = "cmd_scrollLineDown";
        aCommandTable[ CommandMovePageUp ]                 = "cmd_movePageUp";
        aCommandTable[ CommandMovePageDown ]               = "cmd_movePageDown";
        aCommandTable[ CommandSelectPageUp ]               = "cmd_selectPageUp";
        aCommandTable[ CommandSelectPageDown ]             = "cmd_selectPageDown";
        aCommandTable[ CommandInsertText ]                 = "cmd_insertText";
        aCommandTable[ CommandPasteQuote ]                 = "cmd_pasteQuote";
    }
    
    //------------------------------------------------------------------------------
    // Library API
    //------------------------------------------------------------------------------
    bool init( void *pNativeWindow, const char *szApplicationDirectory )
    {
        if( g_pManager )
            return false;

        libPrivateData.pNativeWindow = pNativeWindow;
        strncpy( libPrivateData.szApplicationDirectory, szApplicationDirectory, sizeof( libPrivateData.szApplicationDirectory ) );
        libPrivateData.szApplicationDirectory[ sizeof( libPrivateData.szApplicationDirectory ) - 1 ] = 0;
        return true;
    }

    void ensure()
    {
        if( !g_pManager )
        {
            g_pManager = new Manager;
            g_pManager->init( libPrivateData.pNativeWindow, libPrivateData.szApplicationDirectory );
            g_pManager->AddRef();

            g_pManager->enableMemoryCache( s_bEnableMemoryCache );
            g_pManager->enableDiskCache( s_bEnableDiskCache, s_uMaxDiskCacheSizeKB );
            g_pManager->setUserAgent( s_userAgent );
        }
    }

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
            g_pManager->Release();
            g_pManager = 0;
        }
    }

    //------------------------------------------------------------------------------
    void enableMemoryCache( bool bEnable )
    {
        if( g_pManager )
            g_pManager->enableMemoryCache( bEnable );
        else
        {
            s_bEnableMemoryCache = bEnable;
        }
    }
    
    //------------------------------------------------------------------------------
    void enableDiskCache( bool bEnable, unsigned uMaxSizeKB )
    {
        if( g_pManager )
            g_pManager->enableDiskCache( bEnable, uMaxSizeKB );
        else
        {
            s_bEnableDiskCache = bEnable;
            s_uMaxDiskCacheSizeKB = uMaxSizeKB;
        }
    }

    //------------------------------------------------------------------------------
    void setUserAgent( const char *pUserAgent )
    {
        if( g_pManager )
            g_pManager->setUserAgent( pUserAgent );
        else
        {
            strncpy( s_userAgent, pUserAgent, sizeof( s_userAgent ) );
            s_userAgent[ sizeof( s_userAgent ) / sizeof( s_userAgent[0] ) - 1 ] = 0;
        }
    }

    //------------------------------------------------------------------------------
    Window *createWindow( unsigned width, unsigned height )
    {
        ensure();
        return g_pManager->createWindow( width, height );
    }

    //------------------------------------------------------------------------------
    void destroyWindow( Window *pWindow )
    {
        ensure();
        return g_pManager->destroyWindow( pWindow );
    }

    //------------------------------------------------------------------------------
    // Manager
    //------------------------------------------------------------------------------
    Manager::Manager()
    {   
    }

    //------------------------------------------------------------------------------
    Manager::~Manager()
    {   
    }

    NS_IMPL_ADDREF( Manager )
    NS_IMPL_RELEASE( Manager )
    
    NS_INTERFACE_MAP_BEGIN( Manager )
        NS_INTERFACE_MAP_ENTRY_AMBIGUOUS( nsISupports, nsIWindowCreator2 )
        NS_INTERFACE_MAP_ENTRY( nsIWindowCreator )
        NS_INTERFACE_MAP_ENTRY( nsIWindowCreator2 )
    NS_INTERFACE_MAP_END

    //------------------------------------------------------------------------------
    // nsIWindowCreator2 interface
    //------------------------------------------------------------------------------
        NS_IMETHODIMP Manager::CreateChromeWindow(nsIWebBrowserChrome *parent, PRUint32 chromeFlags, nsIWebBrowserChrome **_retval)
        {
            // No soup for you!
            *_retval = nsnull;
            return NS_ERROR_NOT_IMPLEMENTED;
        }

        NS_IMETHODIMP Manager::CreateChromeWindow2(nsIWebBrowserChrome *parent, PRUint32 chromeFlags, PRUint32 contextFlags, nsIURI *uri, PRBool *cancel, nsIWebBrowserChrome **_retval)
        {
            // No soup for you!
            *_retval = nsnull;
            return NS_ERROR_NOT_IMPLEMENTED;
        }

    //------------------------------------------------------------------------------
    // End nsIWindowCreator interface
    //------------------------------------------------------------------------------
    
    //------------------------------------------------------------------------------
    bool Manager::init( void *pNativeWindow, const char *szApplicationDirectory )
    {
        initCommandTable();

        m_pNativeWindow = pNativeWindow;

        // Initialize XPCOM
        nsCOMPtr<nsILocalFile> pApplicationDirectory;
        nsresult rv;
        
        rv = NS_NewNativeLocalFile( nsCString( szApplicationDirectory ), PR_FALSE, getter_AddRefs( pApplicationDirectory ) );

        if( NS_FAILED( rv ) )
        {
            return false;
        }

        rv = XRE_InitEmbedding( pApplicationDirectory, pApplicationDirectory, nsnull, nsnull, 0 );

        if( NS_FAILED( rv ) )
        {
            return false;
        }

        nsCOMPtr<nsProfileDirServiceProvider> pProvider;
        NS_NewProfileDirServiceProvider( PR_TRUE, getter_AddRefs( pProvider ) );

        if( !pProvider )
        {
            XRE_TermEmbedding();
            return false;
        }

        rv = pProvider->Register();

        if( NS_FAILED( rv ) )
        {
            XRE_TermEmbedding();
            return false;
        }

        rv = pProvider->SetProfileDir( pApplicationDirectory );

        if( NS_FAILED( rv ) )
        {
            pProvider->Shutdown();
            XRE_TermEmbedding();
            return false;
        }

        nsCOMPtr< nsIPref > pPref = do_CreateInstance( NS_PREF_CONTRACTID );

        if( !pPref )
        {
            pProvider->Shutdown();
            XRE_TermEmbedding();
            return false;
        }

        // Set options for how we want mozilla to run
        pPref->SetBoolPref( "security.warn_entering_secure", PR_FALSE );
        pPref->SetBoolPref( "security.warn_entering_weak", PR_FALSE );        
        pPref->SetBoolPref( "security.warn_leaving_secure", PR_FALSE );
        pPref->SetBoolPref( "security.warn_submit_insecure", PR_FALSE );
        pPref->SetBoolPref( "security.warn_viewing_mixed", PR_FALSE );
        pPref->SetBoolPref( "security.enable_java", PR_TRUE );
        pPref->SetBoolPref( "javascript.enabled", PR_TRUE );

        pPref->SetCharPref( "signon.SignonFileName", "signon.txt" );
        pPref->SetBoolPref( "browser.cache.disk.enable", PR_FALSE );
        pPref->SetBoolPref( "browser.cache.memory.enable", PR_FALSE );

        pPref->SetBoolPref( "network.protocol-handler.external.snews", PR_FALSE );
        pPref->SetBoolPref( "network.protocol-handler.external.news", PR_FALSE );
        pPref->SetBoolPref( "network.protocol-handler.external.irc", PR_FALSE );
        pPref->SetBoolPref( "network.protocol-handler.external.mail", PR_FALSE );
        pPref->SetBoolPref( "network.protocol-handler.external.mailto", PR_FALSE );

        pPref->SetBoolPref( "plugin.default_plugin_disabled", PR_TRUE );
  
        pPref->SetIntPref( "browser.link.open_newwindow", nsIBrowserDOMWindow::OPEN_CURRENTWINDOW );

        // Get the thread Q so we can pump it
        {
            nsCOMPtr< nsIEventQueueService > pEventQService = do_GetService( NS_EVENTQUEUESERVICE_CONTRACTID, &rv );

            if( pEventQService )
                pEventQService->GetThreadEventQueue( NS_CURRENT_THREAD, getter_AddRefs( m_pEventQ ) );
        }

        // Override the window creator so we can control it
        {
            nsCOMPtr<nsIWindowWatcher> pWindowWatcher( do_GetService( NS_WINDOWWATCHER_CONTRACTID ) );
            if( pWindowWatcher )
                pWindowWatcher->SetWindowCreator( NS_STATIC_CAST( nsIWindowCreator *, this ) );
        }

        return true;
    }

    //------------------------------------------------------------------------------
    void Manager::update()
    {
        if( m_pEventQ )
            m_pEventQ->ProcessPendingEvents();
    }

    //------------------------------------------------------------------------------
    void Manager::shutdown()
    {
        {
            // Window Watcher keeps a reference to us
            nsCOMPtr<nsIWindowWatcher> pWindowWatcher( do_GetService( NS_WINDOWWATCHER_CONTRACTID ) );
            if( pWindowWatcher )
                pWindowWatcher->SetWindowCreator( NS_STATIC_CAST( nsIWindowCreator *, 0 ) );
        }

        XRE_TermEmbedding();
    }

    //------------------------------------------------------------------------------
    Window *Manager::createWindow( unsigned width, unsigned height )
    {
        Window *pWindow = new Window;

        if( pWindow )
        {
            if( !pWindow->m_pImpl->init( m_pNativeWindow, width, height ) )
            {
                delete pWindow;
                pWindow = 0;
            }
        }

        return pWindow;
    }

    //------------------------------------------------------------------------------
    void Manager::destroyWindow( Window *pWindow )
    {
        delete pWindow;
    }

    //------------------------------------------------------------------------------
    void Manager::enableMemoryCache( bool bEnable )
    {
        nsCOMPtr< nsIPref > pPref = do_CreateInstance( NS_PREF_CONTRACTID );

        if( pPref )
        {
            pPref->SetBoolPref( "browser.cache.memory.enable", bEnable ? PR_TRUE : PR_FALSE );
        }
    }
    
    //------------------------------------------------------------------------------
    void Manager::enableDiskCache( bool bEnable, unsigned uMaxSizeKB )
    {
        nsCOMPtr< nsIPref > pPref = do_CreateInstance( NS_PREF_CONTRACTID );

        if( pPref )
        {
            pPref->SetBoolPref( "browser.cache.disk.enable", bEnable ? PR_TRUE : PR_FALSE );
            pPref->SetIntPref( "browser.cache.disk.capactiy", uMaxSizeKB );
        }
    }

    //------------------------------------------------------------------------------
    void Manager::setUserAgent( const char *pUserAgent )
    {
        nsCOMPtr< nsIPref > pPref = do_CreateInstance( NS_PREF_CONTRACTID );

        if( pPref )
        {
            pPref->SetCharPref( "general.useragent.product", pUserAgent );
        }
    }
    
    //------------------------------------------------------------------------------
    // WindowImpl
    //------------------------------------------------------------------------------
    WindowImpl::WindowImpl( Window *pWindow ) :
        m_uTotalRequests( 0 ),
        m_uFinishedRequests( 0 ),
        m_bUseRealProgress( true ),
        m_bDocumentLoaded( false ),
        m_bRenderOnComplete( false ),
        m_pCallback( 0 ),
        m_pWindow( pWindow ),
        m_uWidth( 0 ),
        m_uHeight( 0 ),
        m_iOffsetX( 4000 ),  // Hack the position so that dropdown windows get created offscreen.  
        m_iOffsetY( -4000 ), // Y must be negative here so that dropdowns continue to drop down, large positive makes them drop up.
        m_fProgress( 1.0f ),
        m_aURI( s_nullString )
    {
        m_aStatus[0] = 0;
    }

    //------------------------------------------------------------------------------
    WindowImpl::~WindowImpl()
    {
        // Controlled order of reference removal - DON
        nsCOMPtr< nsIWeakReference > pListener( do_GetWeakReference( NS_STATIC_CAST( nsIWebProgressListener *, this ) ) );
        m_pWebBrowser->RemoveWebBrowserListener( pListener, NS_GET_IID( nsIWebProgressListener ) );

        if( m_pWebNavigation )
        {
            m_pWebNavigation->Stop( nsIWebNavigation::STOP_ALL );
            m_pWebNavigation = nsnull;
        }

        if( m_pBaseWindow )
        {
            m_pBaseWindow->Destroy();
            m_pBaseWindow = nsnull;
        }

        if( m_aURI != s_nullString )
            delete [] m_aURI;
    }

    //------------------------------------------------------------------------------
    NS_IMPL_THREADSAFE_ADDREF( WindowImpl )
    NS_IMPL_THREADSAFE_RELEASE( WindowImpl )
    
    NS_IMPL_THREADSAFE_QUERY_INTERFACE6( WindowImpl, 
        nsIWebBrowserChrome,
        nsIInterfaceRequestor,
        nsISupportsWeakReference,
        nsIWebProgressListener,
        nsIURIContentListener,
        nsIBadCertListener )

    //------------------------------------------------------------------------------
    // Implementation of methods declared in NS_DECL_NSIINTERFACEREQUESTOR.  
    //------------------------------------------------------------------------------
        NS_IMETHODIMP WindowImpl::GetInterface(const nsIID& aIID, void** aSink)
        {   
            if( aIID.Equals( NS_GET_IID( nsIDOMWindow ) ) )
            {
                return m_pWebBrowser ? m_pWebBrowser->GetContentDOMWindow( reinterpret_cast< nsIDOMWindow** >( aSink ) ) : NS_ERROR_UNEXPECTED;
            }

            return QueryInterface(aIID, aSink);
        }

    //------------------------------------------------------------------------------
    // nsIWebBrowserChrome interface
    //------------------------------------------------------------------------------
        NS_IMETHODIMP WindowImpl::SetStatus(PRUint32 statusType, const PRUnichar *status)
        {
            return NS_OK;
        }

        //------------------------------------------------------------------------------
        NS_IMETHODIMP WindowImpl::GetWebBrowser(nsIWebBrowser * *aWebBrowser)
        {
            *aWebBrowser = m_pWebBrowser;
            NS_IF_ADDREF( *aWebBrowser );
            return NS_OK;
        }

        //------------------------------------------------------------------------------
        NS_IMETHODIMP WindowImpl::SetWebBrowser(nsIWebBrowser * aWebBrowser)
        {
            m_pWebBrowser = aWebBrowser;
            return NS_OK;
        }

        //------------------------------------------------------------------------------
        NS_IMETHODIMP WindowImpl::GetChromeFlags(PRUint32 *aChromeFlags)
        {
            return NS_ERROR_NOT_IMPLEMENTED;
        }
        
        //------------------------------------------------------------------------------
        NS_IMETHODIMP WindowImpl::SetChromeFlags(PRUint32 aChromeFlags)
        {
            return NS_ERROR_NOT_IMPLEMENTED;
        }

        //------------------------------------------------------------------------------
        NS_IMETHODIMP WindowImpl::DestroyBrowserWindow()
        {
            return NS_OK;
        }

        //------------------------------------------------------------------------------
        NS_IMETHODIMP WindowImpl::SizeBrowserTo(PRInt32 aCX, PRInt32 aCY)
        {
            return NS_OK;
        }

        //------------------------------------------------------------------------------
        NS_IMETHODIMP WindowImpl::ShowAsModal()
        {
            return NS_ERROR_NOT_IMPLEMENTED;
        }

        //------------------------------------------------------------------------------
        NS_IMETHODIMP WindowImpl::IsWindowModal(PRBool *_retval)
        {
            *_retval = PR_FALSE;
            return NS_OK;
        }

        //------------------------------------------------------------------------------
        NS_IMETHODIMP WindowImpl::ExitModalEventLoop(nsresult aStatus)
        {
            return NS_OK;
        }
    //------------------------------------------------------------------------------
    // End nsIWebBrowserChrome interface
    //------------------------------------------------------------------------------    

    //------------------------------------------------------------------------------
    // nsWebProgressListener interface
    //------------------------------------------------------------------------------
        NS_IMETHODIMP WindowImpl::OnStateChange(nsIWebProgress *aWebProgress, nsIRequest *aRequest, PRUint32 aStateFlags, nsresult aStatus)
        {
            nsCOMPtr< nsILoadGroup > pLoadGroup;
            aRequest->GetLoadGroup( getter_AddRefs( pLoadGroup ) );

            const float kfOldProgress( m_fProgress );
            
            if( aStateFlags & STATE_START )
            {
                if( aStateFlags & STATE_IS_NETWORK )
                {
                    m_uTotalRequests = 0;
                    m_uFinishedRequests = 0;
                    m_bUseRealProgress = false;
                    m_bDocumentLoaded = false;
                }

                if( aStateFlags & STATE_IS_REQUEST )
                {
                    ++m_uTotalRequests;
                    m_bUseRealProgress = 1 == m_uTotalRequests;
                    m_fProgress = static_cast< float >( m_uFinishedRequests ) / static_cast< float >( m_uTotalRequests );
                }
            }

            if( aStateFlags & STATE_STOP )
            {
                if( aStateFlags & STATE_IS_REQUEST )
                {
                    ++m_uFinishedRequests;
                    m_fProgress = static_cast< float >( m_uFinishedRequests ) / static_cast< float >( m_uTotalRequests );
                }
                if( aStateFlags & STATE_IS_DOCUMENT )
                {
                    m_bDocumentLoaded = true;
                }
            }

            if( aStateFlags & STATE_IS_WINDOW )
            {
                if( aStateFlags & STATE_START )
                {
                    if( pLoadGroup )
                    {
                        pLoadGroup->SetNotificationCallbacks( this );
                    }
                }
                else if( aStateFlags & STATE_STOP )
                {
                    m_bDocumentLoaded = true;

                    if( pLoadGroup )
                    {
                        // The load group's callback pointer to us is ref counted if we
                        // don't clear it we will leak this WindowImpl object.
                        pLoadGroup->SetNotificationCallbacks( 0 );
                    }
            
                    nsCOMPtr< nsIWebBrowserFocus > pFocus( do_GetInterface( m_pWebBrowser ) );

                    if( pFocus )
                    {
                        pFocus->SetFocusAtFirstElement();
                    }
                }
            }

            if( m_pCallback )
            {
                if( kfOldProgress != m_fProgress )
                {
                    m_pCallback->onProgressChanged( m_pWindow );
                }
            }

            return NS_OK;
        }

        //------------------------------------------------------------------------------
        NS_IMETHODIMP WindowImpl::OnProgressChange(nsIWebProgress *aWebProgress, nsIRequest *aRequest, PRInt32 aCurSelfProgress, PRInt32 aMaxSelfProgress, PRInt32 aCurTotalProgress, PRInt32 aMaxTotalProgress)
        {
            if( m_bUseRealProgress )
            {
                if( aMaxTotalProgress == -1 )
                    m_fProgress = 0.0f;
                else
                    m_fProgress = static_cast< float >( aCurTotalProgress ) / static_cast< float >( aMaxTotalProgress );

                if( m_pCallback )
                    m_pCallback->onProgressChanged( m_pWindow );
            }

            return NS_OK;
        }

        //------------------------------------------------------------------------------
        NS_IMETHODIMP WindowImpl::OnLocationChange(nsIWebProgress *aWebProgress, nsIRequest *aRequest, nsIURI *aLocation)
        {
            nsCString aStr;
            aLocation->GetSpec( aStr );

            const unsigned kuLen = aStr.Length();

            if( m_aURI != s_nullString )
                delete [] m_aURI;

            if( kuLen == 0 )
                m_aURI = s_nullString;
            else
            {
                m_aURI = new char[ kuLen + 1 ];
                memcpy( m_aURI, aStr.BeginReading(), kuLen );
                m_aURI[ kuLen ] = 0;
                aStr.EndReading();
            }

            if( m_pCallback )
                m_pCallback->onURIChanged( m_pWindow );

            return NS_OK;
        }

        //------------------------------------------------------------------------------
        NS_IMETHODIMP WindowImpl::OnStatusChange(nsIWebProgress *aWebProgress, nsIRequest *aRequest, nsresult aStatus, const PRUnichar *aMessage)
        {
            unsigned uStatusLen = sizeof( m_aStatus ) / sizeof( m_aStatus[0] );
            wcsncpy( m_aStatus, aMessage, uStatusLen );
            m_aStatus[ uStatusLen - 1 ] = 0;

            if( m_pCallback )
                m_pCallback->onStatusChanged( m_pWindow );

            return NS_OK;
        }

        //------------------------------------------------------------------------------
        NS_IMETHODIMP WindowImpl::OnSecurityChange(nsIWebProgress *aWebProgress, nsIRequest *aRequest, PRUint32 aState)
        {
            return NS_ERROR_NOT_IMPLEMENTED;
        }
    //------------------------------------------------------------------------------
    // End nsWebProgressListener interface
    //------------------------------------------------------------------------------    

    //------------------------------------------------------------------------------
    // nsIURIContentListener interface
    //------------------------------------------------------------------------------
        NS_IMETHODIMP WindowImpl::OnStartURIOpen(nsIURI *aURI, PRBool *pAbortLoad)
        {
            *pAbortLoad = PR_FALSE;

            if( m_pCallback )
            {
                nsCString aStr;
                aURI->GetSpec( aStr );

                bool bURIOK = m_pCallback->doValidateURI( m_pWindow, aStr.BeginReading() );
                aStr.EndReading();

                if( !bURIOK )
                {
                    *pAbortLoad = PR_TRUE;
                }
            }

            return NS_OK;
        }

        //------------------------------------------------------------------------------
        NS_IMETHODIMP WindowImpl::DoContent(const char *aContentType, PRBool aIsContentPreferred, nsIRequest *aRequest, nsIStreamListener **aContentHandler, PRBool *_retval)
        {
            return NS_ERROR_NOT_IMPLEMENTED;
        }

        //------------------------------------------------------------------------------
        NS_IMETHODIMP WindowImpl::IsPreferred(const char *aContentType, char **aDesiredContentType, PRBool *_retval)
        {
            *_retval = PR_TRUE;
            return NS_OK;
        }

        //------------------------------------------------------------------------------
        NS_IMETHODIMP WindowImpl::CanHandleContent(const char *aContentType, PRBool aIsContentPreferred, char **aDesiredContentType, PRBool *_retval)
        {
            return NS_ERROR_NOT_IMPLEMENTED;
        }

        //------------------------------------------------------------------------------
        NS_IMETHODIMP WindowImpl::GetLoadCookie(nsISupports * *aLoadCookie)
        {
            return NS_ERROR_NOT_IMPLEMENTED;
        }
        
        //------------------------------------------------------------------------------
        NS_IMETHODIMP WindowImpl::SetLoadCookie(nsISupports * aLoadCookie)
        {
            return NS_ERROR_NOT_IMPLEMENTED;
        }

        //------------------------------------------------------------------------------
        NS_IMETHODIMP WindowImpl::GetParentContentListener(nsIURIContentListener * *aParentContentListener)
        {
            return NS_ERROR_NOT_IMPLEMENTED;
        }
        
        //------------------------------------------------------------------------------
        NS_IMETHODIMP WindowImpl::SetParentContentListener(nsIURIContentListener * aParentContentListener)
        {
            return NS_ERROR_NOT_IMPLEMENTED;
        }
    //------------------------------------------------------------------------------
    // End nsIURIContentListener interface
    //------------------------------------------------------------------------------    

    //------------------------------------------------------------------------------
    // nsIBadCertListener interface
    //------------------------------------------------------------------------------    
        NS_IMETHODIMP WindowImpl::ConfirmUnknownIssuer(nsIInterfaceRequestor *socketInfo, nsIX509Cert *cert, PRInt16 *certAddType, PRBool *_retval)
        {
            // We completely ignore any SSL errors, don't do your banking with an embedded browser kk?
            *certAddType = ADD_TRUSTED_FOR_SESSION;
            *_retval = PR_TRUE;
            return NS_OK;
        }

        //------------------------------------------------------------------------------
        NS_IMETHODIMP WindowImpl::ConfirmMismatchDomain(nsIInterfaceRequestor *socketInfo, const nsACString & targetURL, nsIX509Cert *cert, PRBool *_retval)
        {
            // We completely ignore any SSL errors, don't do your banking with an embedded browser kk?
            *_retval = PR_TRUE;
            return NS_OK;
        }

        //------------------------------------------------------------------------------
        NS_IMETHODIMP WindowImpl::ConfirmCertExpired(nsIInterfaceRequestor *socketInfo, nsIX509Cert *cert, PRBool *_retval)
        {
            // We completely ignore any SSL errors, don't do your banking with an embedded browser kk?
            *_retval = PR_TRUE;
            return NS_OK;
        }

        //------------------------------------------------------------------------------
        NS_IMETHODIMP WindowImpl::NotifyCrlNextupdate(nsIInterfaceRequestor *socketInfo, const nsACString & targetURL, nsIX509Cert *cert)
        {
            return NS_ERROR_NOT_IMPLEMENTED;
        }
    //------------------------------------------------------------------------------
    // End nsIBadCertListener interface
    //------------------------------------------------------------------------------    
    
    //------------------------------------------------------------------------------
    bool WindowImpl::init( void *pParentNativeWindow, int width, int height )
    {
        m_pWebBrowser = do_CreateInstance( NS_WEBBROWSER_CONTRACTID );

        if( !m_pWebBrowser )
            return false;

        // Technically these can fail, as NS_WEBBROWSER_CONTRACTID only guarantees nsIWebBrowser
        m_pWebNavigation = do_QueryInterface( m_pWebBrowser );
        m_pBaseWindow = do_QueryInterface( m_pWebBrowser );

        if( !m_pWebNavigation || !m_pBaseWindow )
            return false;

        m_pWebBrowser->SetContainerWindow( this );

        nsCOMPtr< nsIDocShellTreeItem > pDocShellTreeItem( do_QueryInterface( m_pWebBrowser ) );
        if( !pDocShellTreeItem )
            return false;

        pDocShellTreeItem->SetItemType( nsIDocShellTreeItem::typeContentWrapper );

        m_pBaseWindow->InitWindow( pParentNativeWindow, nsnull, 0, 0, width, height );
        m_pBaseWindow->Create();

        {
            nsCOMPtr< nsIWeakReference > pListener( do_GetWeakReference( NS_STATIC_CAST( nsIWebProgressListener *, this ) ) );
            m_pWebBrowser->AddWebBrowserListener( pListener, NS_GET_IID( nsIWebProgressListener ) );
        }

        m_pWebBrowser->SetParentURIContentListener( NS_STATIC_CAST( nsIURIContentListener *, this ) );
        m_pBaseWindow->SetVisibility( PR_FALSE );
        m_pBaseWindow->SetPosition( m_iOffsetX, m_iOffsetY );

        m_uWidth = width;
        m_uHeight = height;

        return true;
    }

    //------------------------------------------------------------------------------
    void WindowImpl::setSize( unsigned width, unsigned height )
    {
        const PRBool kfRepaint( PR_FALSE );
        
        m_pBaseWindow->SetSize( width, height, kfRepaint );
        m_pBaseWindow->SetVisibility( PR_FALSE );
        m_pBaseWindow->SetPosition( m_iOffsetX, m_iOffsetY );

        m_uWidth = width;
        m_uHeight = height;
    }

    //------------------------------------------------------------------------------
    void WindowImpl::setFocus( bool bFocus )
    {
        nsCOMPtr< nsIWebBrowserFocus > pFocus( do_GetInterface( m_pWebBrowser ) );

        if( pFocus )
        {
            (void)(bFocus ? pFocus->Activate() : pFocus->Deactivate());
        }
    }

    //------------------------------------------------------------------------------
    void WindowImpl::setCallback( ICallback *pCallback )
    {
        m_pCallback = pCallback;
    }

    //------------------------------------------------------------------------------
    float WindowImpl::getProgress( bool &bIsLoading )
    {
        bIsLoading = (m_uFinishedRequests != m_uTotalRequests);
        return m_fProgress;
    }

    //------------------------------------------------------------------------------
    void WindowImpl::setRenderOnComplete( bool bVal )
    {
        m_bRenderOnComplete = bVal;
    }

    //------------------------------------------------------------------------------
    const wchar_t *WindowImpl::getStatus() const
    {
        return m_aStatus;
    }

    //------------------------------------------------------------------------------
    const char *WindowImpl::getURI() const
    {
        return m_aURI;
    }

    //------------------------------------------------------------------------------
    bool WindowImpl::getCaret( int &x, int &y, int &width, int &height )
    {
        nsCOMPtr< nsIWebBrowserFocus > pFocus( do_QueryInterface( m_pWebBrowser ) );

        if( !pFocus )
            return false;

        nsCOMPtr< nsIDOMElement > pElement;
        pFocus->GetFocusedElement( getter_AddRefs( pElement ) );

        if( !pElement )
            return false;

        nsCOMPtr< nsIContent > pContent = do_QueryInterface( pElement );

        if( !pContent )
            return false;

        nsCOMPtr< nsIDOMWindow > pDOMWindow;
        m_pWebBrowser->GetContentDOMWindow( getter_AddRefs( pDOMWindow ) );

        if( !pDOMWindow )
            return false;

        nsCOMPtr< nsIDOMDocument > pDOMDocument;
        pDOMWindow->GetDocument( getter_AddRefs( pDOMDocument ) );

        if( !pDOMDocument )
            return false;

        nsCOMPtr< nsIDocument > pDocument = do_QueryInterface( pDOMDocument );

        if( !pDocument )
            return false;

        nsIPresShell *pPresShell = pDocument->GetShellAt( 0 );

        if( !pPresShell )
            return false;

        nsCOMPtr< nsICaret > pCaret;
        pPresShell->GetCaret( getter_AddRefs( pCaret ) );

        if( !pCaret )
            return false;

        nsIFrame *pFrame;
        pPresShell->GetPrimaryFrameFor( pContent, &pFrame );

        if( !pFrame )
            return false;

        nsCOMPtr< nsISelectionController > pSelectionController;
        pFrame->GetSelectionController( pPresShell->GetPresContext(), getter_AddRefs( pSelectionController ) );

        if( !pSelectionController )
            return false;

        nsCOMPtr< nsISelection > pSelection;
        pSelectionController->GetSelection( nsISelectionController::SELECTION_NORMAL, getter_AddRefs( pSelection ) );

        nsRect rcCaret;
        PRBool bCollapsed;
        nsIView *pCaretView;
        pCaret->GetCaretCoordinates( nsICaret::eTopLevelWindowCoordinates, pSelection, &rcCaret, &bCollapsed, &pCaretView );

        const float kfTwipsToPixels = pPresShell->GetPresContext()->TwipsToPixels();

        x      = NSTwipsToIntPixels( rcCaret.x,      kfTwipsToPixels );
        y      = NSTwipsToIntPixels( rcCaret.y,      kfTwipsToPixels );
        width  = NSTwipsToIntPixels( rcCaret.width,  kfTwipsToPixels );
        height = NSTwipsToIntPixels( rcCaret.height, kfTwipsToPixels );

        return true;
    }

    //------------------------------------------------------------------------------
    void WindowImpl::navigateTo( const PRUnichar *szURI, const char *szPostData, const unsigned kuPostDataLength )
    {
        if( szPostData )
        {
            nsresult rv = NS_OK;
            nsCOMPtr< nsIStringInputStream > pStringDataStream( do_CreateInstance("@mozilla.org/io/string-input-stream;1", &rv) );

            if( NS_FAILED( rv ) )
                return;

            pStringDataStream->SetData( szPostData, kuPostDataLength );

            nsCOMPtr< nsIMIMEInputStream > pPostDataStream( do_CreateInstance("@mozilla.org/network/mime-input-stream;1", &rv) );

            if( NS_FAILED( rv ) )
                return;

            pPostDataStream->AddHeader( "Content-Type", "application/x-www-form-urlencoded" );
            pPostDataStream->SetAddContentLength( true );
            pPostDataStream->SetData( pStringDataStream );

            m_pWebNavigation->LoadURI( szURI, nsIWebNavigation::LOAD_FLAGS_NONE, 0 /* DEFAULT REFERER */, pPostDataStream, 0 /* NO HEADER */ );
        }
        else
        {
            m_pWebNavigation->LoadURI( szURI, nsIWebNavigation::LOAD_FLAGS_NONE, 0 /* DEFAULT REFERER */, 0 /* NO POST DATA */, 0 /* NO HEADER */ );
        }
    }

    //------------------------------------------------------------------------------
    void WindowImpl::navigateStop()
    {
        m_pWebNavigation->Stop( nsIWebNavigation::STOP_ALL );
    }

    //------------------------------------------------------------------------------
    bool WindowImpl::canNavigateBack()
    {
        PRBool b;
        m_pWebNavigation->GetCanGoBack( &b );
        return b == PR_TRUE;
    }

    //------------------------------------------------------------------------------
    void WindowImpl::navigateBack()
    {
        m_pWebNavigation->GoBack();
    }

    //------------------------------------------------------------------------------
    bool WindowImpl::canNavigateForward()
    {
        PRBool b;
        m_pWebNavigation->GetCanGoForward( &b );
        return b == PR_TRUE;
    }

    //------------------------------------------------------------------------------
    void WindowImpl::navigateForward()
    {
        m_pWebNavigation->GoForward();
    }

    //------------------------------------------------------------------------------
    void WindowImpl::reload()
    {
        m_pWebNavigation->Reload( nsIWebNavigation::LOAD_FLAGS_NONE );
    }

    //------------------------------------------------------------------------------
    static bool GetOpenComboboxControlFrame( nsIFrame *pRoot, nsIComboboxControlFrame **pOut )
    {
        nsCOMPtr< nsIComboboxControlFrame > pComboControlFrame = do_QueryInterface( pRoot );

        if( pComboControlFrame )
        {
            PRBool bDroppedDown( false );
            pComboControlFrame->IsDroppedDown( &bDroppedDown );

            if( bDroppedDown )
            {
                *pOut = pComboControlFrame;
                NS_ADDREF( *pOut );
                return true;
            }
        }

        PRInt32 iChildListIndex = 0;
        while( 1 )
        {
            nsIAtom *pAtom = pRoot->GetAdditionalChildListName( iChildListIndex++ ); // We want to get a NULL back here and use it once - DON
            nsIFrame *pChild = pRoot->GetFirstChild(pAtom);

            while( pChild )
            {
                if( GetOpenComboboxControlFrame( pChild, pOut ) )
                    return true;

                pChild = pChild->GetNextSibling();
            }

            if( !pAtom )
                break;
        }

        return false;
    }

    //------------------------------------------------------------------------------
    void WindowImpl::onMouseEvent( const PRUint32 iEvent, const int x, const int y, const unsigned uFlags )
    {
        nsCOMPtr< nsIViewManager > pViewManager;
        m_getRootView( getter_AddRefs( pViewManager ) );
        
        if( !pViewManager )
            return;

        nsIView *pView;        
        pViewManager->GetRootView( pView );

        if( !pView )
            return;

        nsCOMPtr< nsIWidget > pWidget = pView->GetWidget();

        if( !pWidget )
            return;

        nsCOMPtr< nsIWidget > pEventWidget( pWidget );
        int eventX( x ), eventY( y );

        nsCOMPtr< nsIDocShell > pDocShell = do_GetInterface( m_pWebBrowser );

        // Dear Santa, I've been a very, very bad man this year...
        // This code handles the way mozilla does popups.  They normally create a top level window on top of the
        // application window, but we don't let them do that since we're full screen.  Instead, that top level window
        // is off at -4000 or so (see m_iOffsetX and m_iOffsetY).  So when we get a mouse message we need to go through
        // these contortions to find out if there's an open dropdown box, and if so does this mouse message need to go
        // to it.  We do that by recursively traversing the entire frame set inside GetOpenComboboxControlFrame
        // to see if there are any open nsIComboboxControlFrame's.  If so we get the dropdown and check the coordinates
        // to see if this message need to be fixed up to be relative to the widget for the dropdown.  If this isn't done
        // then what happens is controls that are under the dropdown end up getting the messages that should go to the
        // dropdown - DON
        if( pDocShell )
        {
            nsCOMPtr< nsIPresShell > pPresentShell;
            pDocShell->GetPresShell( getter_AddRefs( pPresentShell ) );

            if( pPresentShell )
            {
                nsIFrame *pFrame = pPresentShell->GetRootFrame();

                if( pFrame )
                {
                    nsCOMPtr< nsIComboboxControlFrame > pComboboxControlFrame;
                    GetOpenComboboxControlFrame( pFrame, getter_AddRefs( pComboboxControlFrame ) );

                    if( pComboboxControlFrame )
                    {
                        nsIFrame *pDropDownFrame( nsnull );
                        pComboboxControlFrame->GetDropDown( &pDropDownFrame ); // WARNING: THIS METHOD IS BUSTED AND DOESN'T ADD REF

                        if( pDropDownFrame )
                        {
                            nsIWidget *pDropDownWidget = pDropDownFrame->GetWindow();

                            if( pDropDownWidget )
                            {
                                nsRect rcBounds;
                                pDropDownWidget->GetBounds( rcBounds );
                                
                                WINDOWINFO windowInfo = { sizeof( WINDOWINFO ) };
                                GetWindowInfo( (HWND)g_pManager->getNativeWindow(), &windowInfo );

                                rcBounds.x = rcBounds.x - m_iOffsetX - windowInfo.rcClient.left;
                                rcBounds.y = rcBounds.y - m_iOffsetY - windowInfo.rcClient.top;

                                if( rcBounds.Contains( x, y ) )
                                {
                                    eventX = x - rcBounds.x;
                                    eventY = y - rcBounds.y;
                                    pEventWidget = pDropDownWidget;
                                }
                            }
                        }
                    }
                }
            }
        }
        
        nsMouseEvent mouseEvent( PR_TRUE, iEvent, pEventWidget, nsMouseEvent::eReal );
        mouseEvent.clickCount = 1;
        mouseEvent.point.x    = eventX;
        mouseEvent.point.y    = eventY;
        mouseEvent.refPoint.x = eventX;
        mouseEvent.refPoint.y = eventY;
        mouseEvent.isAlt      = Window::ALT     & uFlags;
        mouseEvent.isControl  = Window::CONTROL & uFlags;
        mouseEvent.isShift    = Window::SHIFT   & uFlags;

        // Preserve the cursor so mozilla and our app don't fight over it.
        HCURSOR hCursor( ::GetCursor() );
        
        nsEventStatus eventStatus;
        pViewManager->DispatchEvent( &mouseEvent, &eventStatus );

        ::SetCursor( hCursor );
    }

    //------------------------------------------------------------------------------
    void WindowImpl::onMouseWheel( const int iTicks, const unsigned uFlags )
    {
        nsCOMPtr< nsIDOMWindow > pDOMWindow;
        m_pWebBrowser->GetContentDOMWindow( getter_AddRefs( pDOMWindow ) );

        if( pDOMWindow )
        {   
            int iLines = abs( iTicks ) / 120;
            if( iLines == 0 )
                iLines = 1;

            if( iTicks < 0 )
                iLines = -iLines;

            pDOMWindow->ScrollByLines( -iLines );
        }
    }

    //------------------------------------------------------------------------------
    void WindowImpl::onKeyEvent( const PRUint32 iEvent, const PRUint32 iCharCode, const PRUint32 iKeyCode, const unsigned uFlags )
    {
        nsCOMPtr< nsIViewManager > pViewManager;
        m_getRootView( getter_AddRefs( pViewManager ) );
        
        if( !pViewManager )
            return;

        nsIView *pView;        
        pViewManager->GetRootView( pView );

        if( !pView )
            return;

        nsCOMPtr< nsIWidget > pWidget = pView->GetWidget();

        if( !pWidget )
            return;

        nsKeyEvent keyEvent( PR_TRUE, iEvent, pWidget );
        keyEvent.keyCode   = iKeyCode;
        keyEvent.charCode  = iCharCode;
        keyEvent.isChar    = PR_TRUE;
        keyEvent.isAlt     = Window::ALT     & uFlags;
        keyEvent.isControl = Window::CONTROL & uFlags;
        keyEvent.isShift   = Window::SHIFT   & uFlags;

        nsEventStatus eventStatus;
        pViewManager->DispatchEvent( &keyEvent, &eventStatus );
    }

    //------------------------------------------------------------------------------
    void WindowImpl::onCommand( Command command )
    {
        if( command >= 0 && command < NUM_COMMANDS )
            m_doCommand( aCommandTable[ command ] );
    }
    
    //------------------------------------------------------------------------------
    bool WindowImpl::render( IBlitter *pBlitter )
    {
        nsCOMPtr< nsPresContext > pPresentContext;
        m_getPresentContext( getter_AddRefs( pPresentContext ) );

        nsCOMPtr< nsIViewManager > pViewManager;
        m_getRootView( getter_AddRefs( pViewManager ) );

        if( !pPresentContext || !pViewManager )
            return false;

        if( m_bRenderOnComplete && !m_bDocumentLoaded )
            return true;

        nsIScrollableView *pScrollableView; // Views are not XPCOM objects, so this is not an nsCOMPtr
        pViewManager->GetRootScrollableView( &pScrollableView );

        nsIView *pView(0); // Views are not XPCOM objects, so this is not an nsCOMPtr
        if( pScrollableView )
            pScrollableView->GetScrolledView( pView );
        else
            pViewManager->GetRootView( pView );

        // Check for no view or nothing to render
        if( !pView || pView->GetBounds().IsEmpty() )
            return false;
        
        // Calculate the rect to render in the coordinate system of the view.  NS renders in Twips, but we use pixels
        float pixelsToTwips = pPresentContext->PixelsToTwips();
        // Correct for scroll offset.  Have to subtract twice because there's no * operator for their position thing *boggle*
        nsRect viewExtent( pView->GetBounds() - pView->GetPosition() - pView->GetPosition() );
        viewExtent.width  = NSIntPixelsToTwips( m_uWidth, pixelsToTwips );
        viewExtent.height = NSIntPixelsToTwips( m_uHeight, pixelsToTwips );

        // Image will be created and returned as an out param
        nsCOMPtr< nsIRenderingContext > pRenderContext;
        const PRBool kbIsUntrustedRenderContext( PR_FALSE );
        const PRBool kbIgnoreScrolling( PR_FALSE );
        const nscolor colBackground( NS_RGB( 0xFF, 0xFF, 0xFF ) );

        // After we RenderOffscreen we are required to cleanup per nsIViewManager.h:467        
        nsresult rv = pViewManager->RenderOffscreen( pView, viewExtent, kbIsUntrustedRenderContext, kbIgnoreScrolling, colBackground, getter_AddRefs( pRenderContext ) );

        if( NS_FAILED( rv ) || !pRenderContext )
        {
            return false;
        }

        nsIDrawingSurface *pDrawingSurface; // Note: Intentially not an XPCOM pointer - DON
        (void)pRenderContext->GetDrawingSurface( &pDrawingSurface );
        
        // Get the pixels
        void *pBits;
        PRInt32 iStride, iWidthBytes;
        const PRUint32 kuLockFlags(NS_LOCK_SURFACE_READ_ONLY);

        // Locking is done in pixels, not twips.
        rv = pDrawingSurface->Lock(0, 0, m_uWidth, m_uHeight, &pBits, &iStride, &iWidthBytes, kuLockFlags );

        if( NS_FAILED( rv ) )
        {
            (void)pRenderContext->DestroyDrawingSurface( pDrawingSurface );
            return false;
        }

        // On Mozilla 1.8.0 for Win32 offscreen rendering always uses at least 24 bits so
        // we don't have to care about wacko 16 bit pixel formats.  Read
        // nsDrawingSurfaceWin::Init for confirmation at gfx\src\windows\nsDrawingSurfaceWin.cpp(384)

        // Invoke the blitter object so the application can copy data out
        (*pBlitter)( pBits, m_uWidth, m_uHeight, static_cast< unsigned >( iStride ), static_cast< unsigned >( iWidthBytes ) );

        (void)pDrawingSurface->Unlock();
        (void)pRenderContext->DestroyDrawingSurface( pDrawingSurface );

        return true;
    }

    //------------------------------------------------------------------------------
    nsresult WindowImpl::m_getRootView( nsIViewManager **ppViewManager )
    {
        *ppViewManager = nsnull;

        nsCOMPtr< nsIDocShell > pDocShell = do_GetInterface( m_pWebBrowser );

        if( !pDocShell )
            return NS_ERROR_NO_INTERFACE;

        nsCOMPtr< nsIPresShell > pPresentShell;
        pDocShell->GetPresShell( getter_AddRefs( pPresentShell ) );

        if( !pPresentShell )
            return NS_ERROR_NULL_POINTER;

        *ppViewManager = pPresentShell->GetViewManager();

        if( !*ppViewManager )
            return NS_ERROR_NULL_POINTER;

        (*ppViewManager)->AddRef();

        return NS_OK;
    }

    //------------------------------------------------------------------------------
    nsresult WindowImpl::m_getPresentContext( nsPresContext **ppPresentContext )
    {
        *ppPresentContext = nsnull;

        nsCOMPtr< nsIDocShell > pDocShell = do_GetInterface( m_pWebBrowser );

        if( !pDocShell )
            return NS_ERROR_NO_INTERFACE;

        // GetPresContext addRefs
        return pDocShell->GetPresContext( ppPresentContext );
    }

    //------------------------------------------------------------------------------
    nsresult WindowImpl::m_doCommand( const char *cmd )
    {
        nsCOMPtr< nsIDocShell > pDocShell = do_GetInterface( m_pWebBrowser );

        if( !pDocShell )
            return NS_ERROR_NO_INTERFACE;

        nsCOMPtr< nsIScriptGlobalObject > pScriptGlobalObject( do_GetInterface( pDocShell ) );

        if( !pScriptGlobalObject )
            return NS_ERROR_NO_INTERFACE;

        nsCOMPtr< nsPIDOMWindow > pPDOMWindow( do_QueryInterface( pScriptGlobalObject ) );

        if( !pPDOMWindow )
            return NS_ERROR_NO_INTERFACE;

        nsIFocusController *pFocusController = pPDOMWindow->GetRootFocusController();

        if( !pFocusController )
            return NS_ERROR_NULL_POINTER;

        nsCOMPtr< nsIController > pController;
        pFocusController->GetControllerForCommand( cmd, getter_AddRefs( pController ) );

        if( pController )
            pController->DoCommand( cmd );

        return NS_OK;
    }
    
    //------------------------------------------------------------------------------
    // Window
    //------------------------------------------------------------------------------
    void Window::setSize( unsigned width, unsigned height )
    {
        m_pImpl->setSize( width, height );
    }

    //------------------------------------------------------------------------------
    void Window::setFocus( bool bFocus )
    {
        m_pImpl->setFocus( bFocus );
    }

    //------------------------------------------------------------------------------
    void Window::setCallback( ICallback *pCallback )
    {
        m_pImpl->setCallback( pCallback );
    }

    //------------------------------------------------------------------------------
    float Window::getProgress( bool &bIsLoading )
    {
        return m_pImpl->getProgress( bIsLoading );
    }

    //------------------------------------------------------------------------------
    void Window::setRenderOnComplete( bool bVal )
    {
        m_pImpl->setRenderOnComplete( bVal );
    }

    //------------------------------------------------------------------------------
    const wchar_t *Window::getStatus() const
    {
        return m_pImpl->getStatus();
    }

    //------------------------------------------------------------------------------
    const char *Window::getURI() const
    {
        return m_pImpl->getURI();
    }

    //------------------------------------------------------------------------------
    bool Window::getCaret( int &x, int &y, int &width, int &height )
    {
        return m_pImpl->getCaret( x, y, width, height );
    }

    //------------------------------------------------------------------------------
    void Window::navigateTo( const wchar_t *pURI, const char *pPostData, const unsigned kuPostDataLength )
    {
        m_pImpl->navigateTo( pURI, pPostData, kuPostDataLength );
    }

    //------------------------------------------------------------------------------
    void Window::navigateStop()
    {
        m_pImpl->navigateStop();
    }

    //------------------------------------------------------------------------------
    bool Window::canNavigateBack()
    {
        return m_pImpl->canNavigateBack();
    }

    //------------------------------------------------------------------------------
    void Window::navigateBack()
    {
        m_pImpl->navigateBack();
    }

    //------------------------------------------------------------------------------
    bool Window::canNavigateForward()
    {
        return m_pImpl->canNavigateForward();
    }

    //------------------------------------------------------------------------------
    void Window::navigateForward()
    {
        m_pImpl->navigateForward();
    }

    //------------------------------------------------------------------------------
    void Window::reload()
    {
        m_pImpl->reload();
    }

    //------------------------------------------------------------------------------
    void Window::onLeftMouseDown( int x, int y, unsigned uFlags )
    {
        m_pImpl->onMouseEvent( NS_MOUSE_LEFT_BUTTON_DOWN, x, y, uFlags );
    }

    //------------------------------------------------------------------------------
    void Window::onLeftMouseUp( int x, int y, unsigned uFlags )
    {
        m_pImpl->onMouseEvent( NS_MOUSE_LEFT_BUTTON_UP, x, y, uFlags );
    }

    //------------------------------------------------------------------------------
    void Window::onMiddleMouseDown( int x, int y, unsigned uFlags )
    {
        m_pImpl->onMouseEvent( NS_MOUSE_MIDDLE_BUTTON_DOWN, x, y, uFlags );
    }

    //------------------------------------------------------------------------------
    void Window::onMiddleMouseUp( int x, int y, unsigned uFlags )
    {
        m_pImpl->onMouseEvent( NS_MOUSE_MIDDLE_BUTTON_UP, x, y, uFlags );
    }

    //------------------------------------------------------------------------------
    void Window::onRightMouseDown( int x, int y, unsigned uFlags )
    {
        m_pImpl->onMouseEvent( NS_MOUSE_RIGHT_BUTTON_DOWN, x, y, uFlags );
    }

    //------------------------------------------------------------------------------
    void Window::onRightMouseUp( int x, int y, unsigned uFlags )
    {
        m_pImpl->onMouseEvent( NS_MOUSE_RIGHT_BUTTON_UP, x, y, uFlags );
    }

    //------------------------------------------------------------------------------
    void Window::onMouseMove( int x, int y, unsigned uFlags )
    {
        m_pImpl->onMouseEvent( NS_MOUSE_MOVE, x, y, uFlags );
    }

    //------------------------------------------------------------------------------
    void Window::onMouseWheel( int iTicks, unsigned uFlags )
    {
        m_pImpl->onMouseWheel(  iTicks, uFlags );
    }

    //------------------------------------------------------------------------------
    void Window::onKeyPress( int charcode, int keycode, unsigned uFlags )
    {
        m_pImpl->onKeyEvent( NS_KEY_PRESS, charcode, keycode, uFlags );
    }

    //------------------------------------------------------------------------------
    void Window::onCommand( Command command )
    {
        m_pImpl->onCommand( command );
    }
    
    //------------------------------------------------------------------------------
    bool Window::render( IBlitter *pBlitter )
    {
        return m_pImpl->render( pBlitter );
    }

    //------------------------------------------------------------------------------
    Window::Window() : m_pImpl( new WindowImpl( this ) )
    {
        m_pImpl->AddRef();
    }
    
    //------------------------------------------------------------------------------
    Window::~Window()
    {
        m_pImpl->Release();
    }
}
