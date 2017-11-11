////////////////////////////////////////////////////////////////////////////////
//Vivox.h
//
//03/11/08 Joshua M. Kriegshauser
//
//Copyright (c) 2008 Sony Online Entertainment, LLC.  All rights reserved.
////////////////////////////////////////////////////////////////////////////////

#ifndef VIVOX_H
#define VIVOX_H

// Disable deprecated function warnings.
#define _CRT_SECURE_NO_WARNINGS

#include <map>
#include <set>
#include <vector>
#include <string>
#include <list>

// Vivox includes
#define VIVOX_TYPES_ONLY 1 // We're importing from DLL, so only need the types
#include "VxcEvents.h"
#include "VxcResponses.h"

#if !defined(VIVOX_VERSION) || VIVOX_VERSION < 3

typedef unsigned int VX_SDK_HANDLE;

#endif // VIVOX_VERSION

enum VivoxLogSeverity
{
    VLS_DEBUG = 0,
    VLS_INFO,
    VLS_WARN,
    VLS_ERROR,
    VLS_CRITICAL
};

// Response codes from sCheckMic
enum VivoxCheckMic
{
    VCM_OK              = 0,        // No issues
    VCM_MUTED           = (1 << 0), // Mute control present and checked for mic
    VCM_VOLUME_TOO_LOW  = (1 << 1), // Volume control present and set to less than 50%
    VCM_NOT_SELECTED    = (1 << 2), // MUX control present and set to something other than mic
    VCM_DISCONNECTED    = (1 << 3), // Line appears disconnected.  This can't be fixed.
    VCM_ERROR           = (1 << 4), // An error occurred during processing
};

// Function that navigates the mess that is Windows Mixer devices to determine
// whether Windows sees your microphone as properly configured.
// 
// The sDevice parameter is the string name of the device to check.  If empty,
// all devices are checked and fixed (if necessary).
// If the Mute control is present and checked, VCM_MUTED is returned
// If the Volume control is present and below 50%, VCM_VOLUME_TOO_LOW is returned
// If the MUX control is present and the Mic is not selected, VCM_NOT_SELECTED is returned.
// Since the response values are flags, multiple values may be returned.
// 
// If the bFix flag is passed, we attempt to correct these issues.  The return value
// is the same whether bFix is true or false.
//
// Due to Vista using a virtual per-application mixer (that doesn't seem to update
// properly), it is currently disabled.  Vista will always return VCM_OK.
VivoxCheckMic sCheckMic( const std::string& sDevice, bool bFix = false );

#ifdef _WIN32

typedef unsigned __int64 u64;

#elif defined( linux )

#include <sys/bitypes.h>

typedef u_int64_t u64;

#else

#error "Unsupported OS"

#endif

// The following Glue functions must be defined:
//     static void log( VivoxLogSeverity severity, const char* pattern, ... )
//     - Used for logging messages
//     - Can be ignored
//
//     static u64 getTimeMS()
//     - Returns the current time in milliseconds.
//     - This can be a cached time (i.e. updated once per frame) as long as it changes periodically.
// 
//     static void sleep( unsigned milliseconds )
//     - Sleeps for the provided number of milliseconds
// 
//     static void warning( const char* sFile, const int iLine, const char* pattern, ... )
//     - Notifies of a non-fatal runtime problem
//     - Logs filename/line number unlike log()
//     - Can be ignored
// 
//     static void fatal( const char* sFile, const int iLine, const char* pattern, ... )
//     - Notifies of a fatal runtime problem
//     - Logs filename/line number unlike log()
//     - Should not be ignored (or crash is possible)
// 
//     static const char* getGameName()
//     - Retrieves the name that is passed to Vivox in the BeginConnect function
// 
//     static int getLogLevel()
//     - Retrieves the Vivox log level passed at connection time.  Valid values are 0 - 4

struct VivoxParticipantBase {};
struct VivoxSessionBase {};
struct VivoxDiagParticipantBase {};
struct VivoxDiagSessionBase {};
struct VivoxDiagSessionGroupBase {};
struct VivoxDiagAccountBase {};
struct VivoxDiagConnectorBase {};

template <typename StrClass, typename Glue>
class Vivox
{
    Vivox();
    ~Vivox();

    static Vivox* sm_pInstance;

public:
    // Singleton management
    static void install()
    {
        if ( 0 == sm_pInstance )
            sm_pInstance = new Vivox;
    }
    static void remove()
    {
        delete sm_pInstance;
        sm_pInstance = 0;
    }
    static bool isInstalled()
    {
        return sm_pInstance != 0;
    }
    static Vivox& getInstance()
    {
        return *sm_pInstance;
    }

    enum RemoveReason
    {
        RR_UNKNOWN = -1,

        RR_LEFT = 0,
        RR_TIMEOUT,
        RR_KICKED,
        RR_BANNED,

        NUM_REMOVEREASONS
    };

    enum ModCommandType
    {
        MCT_ADDMOD = 0,
        MCT_DELETEMOD,
        MCT_BAN,
        MCT_UNBAN,
        MCT_KICK,
        MCT_MUTE,
        MCT_UNMUTE,
        MCT_MUTEALL,
        MCT_UNMUTEALL,
        MCT_CHANNELMODE,

        NUM_MODCOMMANDTYPES
    };

    enum ChannelMode
    {
        VCM_NONE = 0,
        VCM_NORMAL,
        VCM_PRESENTATION,
        VCM_LECTURE,
        VCM_OPEN,

        NUM_CHANNELMODES
    };

    enum DumpLevel
    {
        DL_ALL = 0,
        DL_SESSIONS,

        NUM_DUMPLEVELS
    };

    struct TextMessage
    {
        StrClass sHeader;
        StrClass sBody;

        TextMessage() {}
        TextMessage( const StrClass& h, const StrClass& b ) : sHeader( h ), sBody( b ) {}

        bool operator == ( const TextMessage& rhs ) const
        {
            return sHeader == rhs.sHeader &&
                   sBody   == rhs.sBody;
        }
    };

    typedef std::list< TextMessage > TextMessageList;
    typedef std::map< StrClass, TextMessageList > TextMessageMap;

    struct DiagParticipant : public VivoxDiagParticipantBase
    {
        StrClass sURI;
        StrClass sAccountName;
        StrClass sDisplayName;
        bool  bAudioEnabled;
        bool  bTextEnabled;
        bool  bAudioMutedForMe;
        bool  bTextMutedForMe;
        bool  bAudioModMuted;
        bool  bTextModMuted;
        bool  bHandRaised;
        bool  bTyping;
        bool  bSpeaking;
        float fVolume;
        float fEnergy;
    };

    struct DiagSession : public VivoxDiagSessionBase
    {
        StrClass sHandle;
        StrClass sURI;
        StrClass sName;
        bool  bAudioMutedForMe;
        bool  bTextMutedForMe;
        bool  bTransmitting;
        bool  bFocused;
        float fVolume;
        int   iFont;
        bool  bHasAudio;
        bool  bHasText;
        bool  bIsIncoming;
        bool  bIsPositional;
        bool  bIsConnected;
        std::vector< DiagParticipant > aParticipants;
    };

    struct DiagSessionGroup : public VivoxDiagSessionGroupBase
    {
        StrClass sHandle;
        std::vector< DiagSession > aSessions;
    };

    struct DiagAccount : public VivoxDiagAccountBase
    {
        StrClass sHandle;
        StrClass sAccountURI;
        StrClass sAccountName;
        StrClass sDisplayName;
        std::vector< DiagSessionGroup > aSessionGroups;
    };

    struct DiagConnector : public VivoxDiagConnectorBase
    {
        StrClass sHandle;
        std::vector< DiagAccount > aAccounts;

        float    fMicVol;
        bool     bMicMute;

        float    fSpeakerVol;
        bool     bSpeakerMute;
    };

    struct Session;
    struct Participant : public VivoxParticipantBase
    {
        friend class Vivox;
        Participant() : pSession( 0 ), bSelf( false ), bModerator( false ), bModMuted( false ), bSpeaking( false ), bPushToTalk( false ),
             fVolume( 0.0f ), fEnergy( 0.0f ), m_bLocalMute( false ), m_fLocalVolume( 0.5f ), bIsLocal( true ) {}

        const Session* pSession; // Session that this participant belongs to
        StrClass sURI;
        StrClass sAccount;
        mutable StrClass sDisplayName;
        bool  bSelf;
        mutable bool  bModerator; // Eventually this will be determined by the Vivox API, but until that catches up it's manual.
        bool  bModMuted; // muted by moderator
        bool  bSpeaking;
        bool  bPushToTalk;
        float fVolume; // Participant's set volume (0.0 - 1.0)
        float fEnergy; // How loud participant is currently speaking (0.0 - 1.0)
        mutable bool bIsLocal;

        void  CopySimple( const Participant& from );
        bool  SetLocalMute( bool bMute ) const;
        bool  GetLocalMute() const { return m_bLocalMute; }
        bool  SetLocalVolume( float f ) const; // 0.0 - 1.0
        float GetLocalVolume() const { return m_fLocalVolume; }

        // Since the moderator flag is manually controlled at this time, this function will
        // fire the VivoxOnParticipantUpdated event to simulate the change coming through the
        // Vivox API.
        void  SetModeratorFlag( bool b ) const;

    private:
        mutable bool  m_bLocalMute;
        mutable float m_fLocalVolume;

        // We can get text messages before the participant is officially added, so we store them here.
        TextMessageList m_pending;
    };

    struct Session : public VivoxSessionBase
    {
        friend class Vivox;

        // These must match vx_session_media_state
        enum State
        {
            MS_NONE = 0,
            MS_DISCONNECTED,
            MS_CONNECTED,
		    MS_RINGING,
		    MS_HOLD,
		    MS_REFER,

            NUM_STATES
        };

        Session() :
            pSelf( 0 ),
            eState( NUM_STATES ),
            bIsChannel( false ),
            bTransmitEnabled( false ),
            bFocused( false ),
            bMuted( false ),
            fVolume( 0.5f ),
            iFont( 0 ),
            m_bLocalMute( false ),
            m_fLocalVolume( 0.5f ),
            m_bNeedDisconnect( false )
        {}

        StrClass sHandle;
        StrClass sGroupHandle;
        StrClass sURI;
        StrClass sAliasURI;
        mutable StrClass sDisplayName;
        StrClass sChannelName;
        Participant* pSelf;
        State eState;
        bool bIsChannel;
        bool bTextConnected;

        // SessionUpdated elements
        bool  bTransmitEnabled;
        bool  bFocused;
        bool  bMuted;
        float fVolume;
        int   iFont;

        typedef std::map< StrClass, Participant > ParticipantMap;
        ParticipantMap mParticipants;

        bool  SetLocalMute( bool bMute ) const;
        bool  GetLocalMute() const { return m_bLocalMute; }
        bool  SetLocalVolume( float f ) const; // 0.0 - 1.0
        float GetLocalVolume() const { return m_fLocalVolume; }

        void  SetNeedDisconnect( bool b ) { m_bNeedDisconnect = b; }
        bool  NeedsDisconnect() const { return m_bNeedDisconnect; }

    private:
        mutable bool  m_bLocalMute;
        mutable float m_fLocalVolume;

        // Due to a Vivox server race condition, we can't send a remove session
        // request until we've received the media state.  Therefore, this flag
        // will be set if we try to disconnect prior to receiving the media state.
        bool m_bNeedDisconnect;
    };

    // Use this as an interface to handle callbacks
    struct VivoxEventHandler
    {
        // Auto add/remove handlers
        VivoxEventHandler( bool bAutoRegister = true )
        {
            if ( bAutoRegister && Vivox::isInstalled() )
                Vivox::getInstance().AddHandler( this );
        }
        virtual ~VivoxEventHandler()
        {
            if ( Vivox::isInstalled() )
                Vivox::getInstance().RemoveHandler( this );
        }

        // Connection events
        virtual void VivoxOnConnected( bool bConnected ) {}
        virtual void VivoxOnDisconnect( bool bAttemptingReconnect ) {}
        virtual void VivoxOnLogin( bool bLoggedIn ) {}
        virtual void VivoxOnLoginStateChange( const StrClass& sState ) {}
        virtual void VivoxVoiceServiceStateChange( bool bConnected ) {}

        // Session events
        virtual void VivoxOnSessionGroupAdded( const StrClass& sSessionGroup ) {}
        virtual void VivoxOnSessionGroupRemoved( const StrClass& sSessionGroup ) {}
        virtual void VivoxOnSessionAdded( const StrClass& sSessionGroup, const StrClass& sSession, const Session& s ) {}
        virtual void VivoxOnSessionRemoved( const StrClass& sSessionGroup, const StrClass& sSession, const Session& s ) {}
        virtual void VivoxOnSessionUpdated( const StrClass& sSessionGroup, const StrClass& sSession, const Session& s ) {}
        virtual void VivoxOnMediaStreamUpdated( const StrClass& sSessionGroup, const StrClass& sSession, int iStatusCode, const StrClass& sStatusString, typename Session::State oldState, const Session& s ) {}

        // Participant events
        virtual void VivoxOnParticipantAdded( const StrClass& sSessionGroup, const StrClass& sSession, const Participant& p ) {}
        virtual void VivoxOnParticipantRemoved( const StrClass& sSessionGroup, const StrClass& sSession, const Participant& p, RemoveReason reason ) {}
        // Note! The oldP only contains the non-string elements of the Participant structure
        virtual void VivoxOnParticipantUpdated( const StrClass& sSessionGroup, const StrClass& sSession, const Participant& oldP, const Participant& newP ) {}

        // Text events
        virtual void VivoxOnTextStreamUpdated( const StrClass& sSession, const Session& s ) {}
        virtual void VivoxOnTextMessage( const StrClass& sSession, const Participant& sender, const StrClass& sHeader, const StrClass& sBody ) {}

        // Moderator events
        virtual void VivoxOnModeratorResponse( ModCommandType type, const StrClass& sSessionURI, const StrClass& sTargetURI, int iCode /* 0 == success */, const StrClass& sMessage ) {}
        virtual void VivoxOnModeratorList( const StrClass& sSessionURI, const std::vector< StrClass >& aModerators ) {}

        // Other events
        virtual void VivoxOnIdleStateChanged( bool bIsIdle ) {}
        virtual void VivoxOnGenericError( int iError, const StrClass& sError ) {}
        virtual void VivoxOnPushToTalk( bool bTalking ) {}
        virtual void VivoxOnJoinExistingSessionBegin() {}
        virtual void VivoxOnJoinExistingSessionEnd() {}

        // Diagnostic state dumps (v3 only)
        virtual void VivoxOnBeginDiagnosticStateDump( int numConnectors ) {}
        virtual void VivoxDiagnosticStateDump( const DiagConnector& info ) {}
        virtual void VivoxOnEndDiagnosticStateDump() {}

        // Testing events
        virtual void VivoxOnVUMonitorStart() {}
        virtual void VivoxVUMonitor( float fMicVolume, float fSpeakerVolume, float fEnergy, bool bSpeaking ) {}
        virtual void VivoxOnVUMonitorEnd() {}
        virtual void VivoxOnEchoTestStart() {}
        virtual void VivoxOnEchoTestEnd( u64 uLengthMS ) {}
        virtual void VivoxOnEchoTestPlaybackStart( u64 uLengthMS ) {}
        virtual void VivoxOnEchoTestPlaybackEnd( u64 uLengthMS ) {}
        virtual void VivoxOnEchoTestError() {}

        // Device events
        virtual void VivoxOnGetOutputDevices( const StrClass& sCurrent, const std::vector< StrClass >& aDevices ) {}
        virtual void VivoxOnGetInputDevices( const StrClass& sCurrent, const std::vector< StrClass >& aDevices ) {}
        virtual void VivoxOnSetOutputDevice( const StrClass& sDevice, bool bSucceeded ) {}
        virtual void VivoxOnSetInputDevice( const StrClass& sDevice, bool bSucceeded ) {}
    };

    void AddHandler( VivoxEventHandler* pHandler );
    bool RemoveHandler( VivoxEventHandler* pHandler );

    void ProcessEvents();

    // Accessors
    bool isConnecting() const
    {
        return CS_CONNECTING == m_eConnectState ||
               CS_WAITING_FOR_STATE == m_eConnectState;
    }

    bool isConnected() const
    {
        return CS_CONNECTED == m_eConnectState;
    }

    bool isDisconnecting() const
    {
        return CS_WAITING_FOR_LOGOUT == m_eConnectState ||
               CS_DISCONNECTING      == m_eConnectState;
    }

    bool isDisconnected() const
    {
        return CS_DISCONNECTED == m_eConnectState;
    }

    bool isLoggingIn() const
    {
        return LS_LOGGING_IN == m_eLoginState;
    }

    bool isLoggedIn() const
    {
        return LS_LOGGED_IN == m_eLoginState;
    }

    bool isLoggingOut() const
    {
        return LS_LOGGING_OUT == m_eLoginState ||
               LS_WAITING_FOR_REQUESTS == m_eLoginState;
    }

    bool isLoggedOut() const
    {
        return LS_LOGGED_OUT == m_eLoginState;
    }

    const StrClass& GetAccountName() const
    {
        return m_sAccountName;
    }

    int GetLastError() const
    {
        return m_iLastError;
    }

    const StrClass& GetLastErrorString() const
    {
        return m_sLastErrorString;
    }

    // Session management
    const Session*     FindSession( const StrClass& sSession ) const;
    const Session*     FindSessionByURI( const StrClass& sSessionURI ) const;
    const Participant* FindParticipant( const StrClass& sURI, const StrClass& sSession ) const;
    bool SetPositionalSession( const StrClass& sSession );
    typedef std::map< StrClass, Session > SessionMap; // Key is session handle
    const SessionMap&  GetSessions() const
    {
        return m_mSessions;
    }

    // Connection managment
    bool BeginConnect( const StrClass& sServer, int iMinPort = 0, int iMaxPort = 0, const StrClass& sServiceExe = StrClass(), const StrClass& sServiceIP = StrClass() );
    bool Shutdown( bool bGlobal = false );
    bool DisconnectVoiceService();

    // Account management functions
    bool BeginLogin( const StrClass& sAcct, const StrClass& sPassword );
    bool Logout();
    const StrClass& GetLocalDisplayName() const;
    const StrClass& GetLocalURI() const;

    // Voice Service functions
    bool IsUsingVoiceService() const
    {
        return m_bUsingVoiceService;
    }
    bool IsVoiceServiceConnected() const
    {
        return m_bVoiceServiceConnected;
    }
    const StrClass& GetVoiceServiceExe() const
    {
        return m_sVoiceServiceExe;
    }
    const StrClass& GetVoiceServiceIP() const
    {
        return m_sVoiceServiceIP;
    }
    const StrClass& GetVoiceServicePlatform() const
    {
        return m_sVoiceServicePlatform;
    }
    const StrClass& GetVoiceServiceVersion() const
    {
        return m_sVoiceServiceVersion;
    }
    const StrClass& GetVoiceServiceDataDirectory() const
    {
        return m_sVoiceServiceDataDirectory;
    }

    // SessionGroup functions
    bool BeginSessionGroup();
    bool TerminateSessionGroup( const StrClass& sSessionGroup );

    // Session functions
    bool BeginSession( const StrClass& sURI, const StrClass& sDisplayName = StrClass(), const StrClass& sPassword = StrClass(), const StrClass& sSessionGroup = StrClass() );
    bool DisconnectAudio( const StrClass& sSession );
    bool TerminateSession( const StrClass& sSession );
    bool TerminateSessionByURI( const StrClass& sURI );
    bool SendTextMessage( const StrClass& sSession, const StrClass& sHeader, const StrClass& sBody, bool bNoSpam = false );
    bool GetModeratorList( const StrClass& sSession );

    // Moderator functions
    bool AddModerator( const StrClass& sSessionURI, const StrClass& sUserURI );
    bool DeleteModerator( const StrClass& sSessionURI, const StrClass& sUserURI );
    bool SetUserBan( const StrClass& sSessionURI, const StrClass& sUserURI, bool bBanned );
    bool KickUser( const StrClass& sSessionURI, const StrClass& sUserURI );
    bool SetUserModMute( const StrClass& sSessionURI, const StrClass& sUserURI, bool bMuted );
    bool SetUserMuteAll( const StrClass& sSessionURI, bool bMuted );
    bool SetChannelMode( const StrClass& sSessionURI, ChannelMode eNewMode, int iCapacity, int iMaxParticipants );

    // Device control
    bool SetLocalMicMute( bool bMute );
    bool SetLocalSpeakerMute( bool bMute );
    bool SetLocalMicVolume( float f ); // 0.0 - 1.0
    bool SetLocalSpeakerVolume( float f ); // 0.0 - 1.0

    bool  GetLocalMicMute() const;
    bool  GetLocalSpeakerMute() const;
    float GetLocalMicVolume() const;
    float GetLocalSpeakerVolume() const;

#if VIVOX_VERSION >= 3
    // Device definition strings for SetInputDevice() and SetOutputDevice() (Vivox v3 only)
    static const StrClass NO_DEVICE;
    static const StrClass DEFAULT_DEVICE;
#endif

    bool GetOutputDevices();
    bool SetOutputDevice( const StrClass& sDevice );
    bool GetInputDevices();
    bool SetInputDevice( const StrClass& sDevice );
    
    const StrClass& GetDefaultOutputDevice() const
    {
        return m_sDefaultOutputDevice;
    }
    const StrClass& GetDefaultInputDevice() const
    {
        return m_sDefaultInputDevice;
    }

    const StrClass& GetCurrentOutputDevice() const
    {
        return m_sCurrentOutputDevice.empty() ? m_sDefaultOutputDevice : m_sCurrentOutputDevice;
    }
    const StrClass& GetCurrentInputDevice() const
    {
        return m_sCurrentInputDevice.empty() ? m_sDefaultInputDevice : m_sCurrentInputDevice;
    }

    struct Vector
    {
        float x, y, z;
        Vector( float _x = 0, float _y = 0, float _z = 0 ) : x( _x ), y( _y ), z( _z ) {}
        Vector( const Vector& rhs ) : x( rhs.x ), y( rhs.y ), z( rhs.z ) {}
        bool operator == ( const Vector& rhs ) const
        {
            return x == rhs.x &&
                   y == rhs.y &&
                   z == rhs.z;
        }
        bool operator != ( const Vector& rhs ) const
        {
            return ! operator == ( rhs );
        }
    };

    // Location control
    void SetSpeakerLocation( const Vector& vLoc, const Vector& vForward, const Vector& vUp, const Vector& vLeft );
    void SetSpeakerVelocity( const Vector& v );
    void SetMicLocation( const Vector& vLoc, const Vector& vForward, const Vector& vUp, const Vector& vLeft );
    void SetMicVelocity( const Vector& v );

    // Push to talk
    void SetPushToTalkMode( bool b );
    bool GetPushToTalkMode() const
    {
        return m_bPushToTalk;
    }
    void ForcePushToTalk( bool b );
    bool IsPushToTalkForced() const
    {
        return m_bPTTForced;
    }
    bool IsPushToTalkTalking() const
    {
        return !m_bPreventPTT && !m_bHandsOff && ( m_bPTTForced || m_bTalking );
    }
    // iKeyCode should be a windows VK_ code
    bool SetPushToTalkKey( int iKeyCode, bool bAdd = false );
    bool RemovePushToTalkKey( int iKeyCode, bool bDisableOnEmpty = true );
    void ClearPushToTalkKeys();
    void SetPreventPushToTalk( bool b )
    {
        m_bPreventPTT = b;
        m_handlePTTEvent( IsPushToTalkTalking() );
    }
    bool GetPreventPushToTalk() const
    {
        return m_bPreventPTT;
    }

    bool SetTransmitSession( const StrClass& sSession );
    const Session* GetTransmitSession() const;

    // VU monitoring.  This will start giving VivoxVUMonitor events, but suspends all active sessions
    bool BeginVUMonitor();
    bool EndVUMonitor();
    bool IsVUMonitorActive() const
    {
        return TT_VU_MONITOR == m_testType;
    }

    // Echo Test.  This captures a short (10 sec) recording that can then be played back.
    // Active sessions must be suspended while performing the echo test.
    bool BeginEchoTest();
    bool StopEchoTest();
    bool PlayEchoTest();
    bool IsPerformingEchoTest() const
    {
        return TT_ECHO_RECORD == m_testType;
    }
    bool IsPlayingEchoTest() const
    {
        return TT_ECHO_PLAYBACK == m_testType;
    }
    bool IsEchoTestPlaybackAvailable() const
    {
        return m_uEchoTestLength != 0;
    }

    // Manual control of suspending and unsuspending sessions
    void SuspendSessions();
    bool UnsuspendSessions();
    bool AreSessionsSuspended() const
    {
        return m_iSessionsSuspended > 0;
    }

    // Miscellaneous functions
    bool RequestDiagnosticStateDump( DumpLevel level );
    bool IsHandlingUnsolicitedResponse() const
    {
        return m_bIsHandlingUnsolicitedResponse;
    }
    bool IsJoiningExistingSession() const
    {
        return m_bIsJoiningExistingSession;
    }

    // Hands-off mode.
    // This means that the Vivox Wrapper will ignore any requests made by the application
    // (usually by returning false) with the exception of global volume adjustment.
    // Events are still fired.
    void SetHandsOff( bool b )
    {
        m_bHandsOff = b;
    }
    bool IsHandsOff() const
    {
        return m_bHandsOff;
    }

private:
    // Data
    typedef std::vector< VivoxEventHandler* > EventHandlers;
    EventHandlers m_aHandlers;
    EventHandlers* m_pNewHandlers;
    int m_iFiringEvents;
    unsigned m_uID;
    StrClass m_sConnectionHandle;
    StrClass m_sAccountHandle;
    StrClass m_sAccountName;
    StrClass m_sLocalDisplayName;
    StrClass m_sLocalURI;
    SessionMap m_mSessions;
    VX_SDK_HANDLE m_sdkHandle;

    enum ConnectState
    {
        CS_DISCONNECTED = 0,
        CS_CONNECTING,
        CS_WAITING_FOR_STATE,
        CS_CONNECTED,
        CS_WAITING_FOR_LOGOUT,
        CS_DISCONNECTING,

        NUM_CONNECTSTATES
    } m_eConnectState;

    enum LoginState
    {
        LS_LOGGED_OUT = 0,
        LS_LOGGING_IN,
        LS_LOGGED_IN,
        LS_WAITING_FOR_REQUESTS,
        LS_LOGGING_OUT,

        NUM_LOGINSTATES
    } m_eLoginState;

    StrClass m_sNextServer;
    int m_iNextMinPort, m_iNextMaxPort;
    StrClass m_sNextServiceExe;
    StrClass m_sNextServiceIP;
    bool m_bNeedConnect;
    bool m_bNeedDisconnect;
    bool m_bNeedDisconnectIsGlobal;
    bool m_bHandsOff;

    std::set< StrClass > m_pendingRequests;
    StrClass m_sNextLoginAcct, m_sNextLoginPassword;
    bool m_bNeedLogin;
    bool m_bNeedLogout;

    // VivoxVoiceService management
    bool m_bUsingVoiceService;
    bool m_bVoiceServiceConnected;
    StrClass m_sVoiceServiceExe;
    StrClass m_sVoiceServiceIP;
    StrClass m_sVoiceServicePlatform;
    StrClass m_sVoiceServiceVersion;
    StrClass m_sVoiceServiceDataDirectory;

    StrClass m_sDefaultInputDevice, m_sDefaultOutputDevice;
    StrClass m_sCurrentInputDevice, m_sCurrentOutputDevice;
    bool m_bMicMutePending;
    bool m_bLocalSpeakerMute, m_bLocalMicMute;
    float m_fLocalSpeakerVolume, m_fLocalMicVolume;

    // Position information
    StrClass m_sPositionSessionGroup, m_sPositionSession;
    bool     m_bPosDirty;
    u64      m_lastPosUpdateTime;
    Vector   m_curSpeakerLoc, m_curSpeakerForward, m_curSpeakerUp, m_curSpeakerLeft;
    Vector   m_curSpeakerVelocity;
    Vector   m_curMicLoc, m_curMicForward, m_curMicUp, m_curMicLeft;
    Vector   m_curMicVelocity;

    // Error info
    int m_iLastError;
    StrClass m_sLastErrorString;

    // Push to talk
    bool m_bNotificationPending;
    bool m_bPushToTalk;
    bool m_bTalking; // push-to-talk key is up/down
    bool m_bPTTForced; // Application has activated push-to-talk
    bool m_bPreventPTT; // Application wants us to ignore push-to-talk
    std::vector< int > m_aPTTKeys; // Array of Windows VK_ codes that activate PTT
    StrClass m_sTransmitSessionGroup, m_sTransmitSession;

    // Suspended sessions while using the VU monitor or echo tests
    struct SessionInfo
    {
        StrClass sURI;
        StrClass sDisplayName;
        StrClass sPassword;
        StrClass sGroup;
        bool bSuspended;
        bool bNeedDisconnect;

        SessionInfo() : bSuspended( false ), bNeedDisconnect( false ) {}
    };

    enum TestType
    {
        TT_NONE = 0,
        TT_VU_MONITOR,
        TT_ECHO_RECORD,
        TT_ECHO_PLAYBACK,

        NUM_TESTTYPES
    };
    enum TestState
    {
        TS_NOT_RUNNING = 0,
        TS_SUSPENDING,
        TS_STARTING,
        TS_RUNNING,
        TS_STOPPING,
        TS_EVENTS,

        NUM_TESTSTATES
    };
    TestType  m_testType;
    TestState m_testState;
    bool m_bPreTestMute;
    bool m_bNeedStop;
    bool m_bSessionsSuspendedInternal;
    int  m_iSessionsSuspended;
    typedef std::map< StrClass, SessionInfo > SessionInfoMap; // Key is URI (lowercase)
    SessionInfoMap m_mSessionInfo;
    bool m_allSessionsSuspended() const;


    // Deferred sessions.
    // Sessions may only be added and removed one at a time in order to avoid
    // race conditions in the Vivox SDK or backend.  Therefore, all terminate
    // session and begin session requests go through the deferred session list.
    // The entry at the front of the deferred session list is always the currently
    // pending entry.
    struct DeferredSessionInfo
    {
        bool bTerminate;
        bool bBeginProcessing;  // ready to be processed
        bool bForeign;          // added due to other application's response being processed
        StrClass sURI;          // handle if terminate is true
        StrClass sDisplayName;
        StrClass sPassword;
        DeferredSessionInfo() : bTerminate( false ), bBeginProcessing( false ), bForeign( false ) {}
    };
    typedef std::list< DeferredSessionInfo > DeferredSessionList;

    struct GroupInfo
    {
        StrClass sHandle;               // Group handle
        std::set< StrClass > sSessions; // Session handles
        DeferredSessionList vDeferred;  // See comments above
    };
    typedef std::map< StrClass, GroupInfo > GroupInfoMap; // key is session group handle
    GroupInfoMap m_mGroupInfo;

    // Echo Test
    u64  m_uEchoTestStartTime;
    u64  m_uEchoTestLength;

    // Text messages
    TextMessageMap m_mTextMessages;
    bool m_bTextMessagePending;

    // Other
    bool m_bIsHandlingUnsolicitedResponse;
    bool m_bIsJoiningExistingSession;

    void m_generateID( vx_req_base& base, bool bUseSDKHandle = true );
    bool m_issueRequest( vx_req_base* p );
    bool m_sendLogoutRequest();
    StrClass m_parseAcctNameFromURI( const char* pURI );
    void m_tolower( StrClass& s );

    static void m_onDllLoaded();

    void m_setLastError( int iError, const StrClass& sError )
    {
        m_iLastError = iError;
        m_sLastErrorString = sError;
    }

    bool m_testHandsOff( const char* event );

    void m_fireModEvent( ModCommandType type, int iCode, const char* pStatus, const char* pSessionURI, const char* pTargetURI );
    void m_fireParticipantUpdated( const StrClass& sGroup, const StrClass& sSession, const Participant& oldP, const Participant& newP );
    bool m_sendPositionUpdate();

    void m_handlePTTEvent( const bool kbIsTalking );
    void m_sendPTTNotification( const StrClass& sSession, const bool kbIsTalking );

    bool m_sendTextMessage( const StrClass& sSession, const StrClass& sHeader, const StrClass& sBody );
    bool m_processNextDeferred( GroupInfo& group, bool bPopFront );
    bool m_removeDeferred( GroupInfo& group, const StrClass& sURI, bool bTerminate );
    void m_suspendSessions( bool bInternal );
    void m_unsuspendSessions( bool bInternal );
#if VIVOX_VERSION >= 3
    // For v3, make sure that we clean up any outstanding session groups
    void m_clearSessions( bool bFireSessionGroupEvents = true );
#else
    void m_clearSessions( bool bFireSessionGroupEvents = false );
#endif
    const char* m_getRespTypeName( vx_response_type t ) const;
    bool m_logAnyError( const vx_resp_base& );

    bool m_beginVUMonitor();
    bool m_beginEchoTest();
    bool m_playEchoTest();
    bool m_stopEchoPlayback();
    bool m_stopVUMonitor();

    Session*     m_findSession( const StrClass& sSession );
    Participant* m_findParticipant( const StrClass& sURI, const StrClass& sSession );

    // Handler functions
    // Events
    void m_processAccountLoginStateChange( const vx_evt_account_login_state_change& );
    void m_processSessionGroupAdded( const vx_evt_sessiongroup_added& );
    void m_processSessionGroupRemoved( const vx_evt_sessiongroup_removed& );
    void m_processSessionAdded( const vx_evt_session_added& );
    void m_processSessionRemoved( const vx_evt_session_removed& );
    void m_processSessionUpdated( const vx_evt_session_updated& );
    void m_processSessionNotification( const vx_evt_session_notification& );
    void m_processParticipantAdded( const vx_evt_participant_added& );
    void m_processParticipantRemoved( const vx_evt_participant_removed& );
    void m_processParticipantUpdated( const vx_evt_participant_updated& );
    void m_processMediaStreamUpdated( const vx_evt_media_stream_updated& );
    void m_processIdleStateChanged( const vx_evt_idle_state_changed& );
    void m_processKeyboardMouseEvent( const vx_evt_keyboard_mouse& );
    void m_processAudioProperties( const vx_evt_aux_audio_properties& );
    void m_processTextStreamUpdated( const vx_evt_text_stream_updated& );
    void m_processTextMessage( const vx_evt_message& );
#if VIVOX_VERSION >= 3
    void m_handleVoiceServiceConnectionStateChanged( const vx_evt_voice_service_connection_state_changed& );
#endif
    // Responses
    void m_processConnectorCreateResp( const vx_resp_connector_create& );
    void m_processAccountLoginResp( const vx_resp_account_login& );
    void m_processGetRenderDevicesResp( const vx_resp_aux_get_render_devices& );
    void m_processGetCaptureDevicesResp( const vx_resp_aux_get_capture_devices& );
    void m_processSetRenderDeviceResp( const vx_resp_aux_set_render_device& );
    void m_processSetCaptureDeviceResp( const vx_resp_aux_set_capture_device& );
    void m_processGetModeratorsResp( const vx_resp_account_channel_get_moderators& );
#if VIVOX_VERSION >= 3
    void m_processDiagnosticStateDumpResp( const vx_resp_aux_diagnostic_state_dump& );
#endif
};

template <typename StrClass, typename Glue>
Vivox<StrClass, Glue>* Vivox<StrClass, Glue>::sm_pInstance = 0;

class ScopedReset
{
    bool& m_b;
public:
    ScopedReset( bool& b ) : m_b( b ) { m_b = false; }
    ~ScopedReset()                    { m_b = false; }

private:
	ScopedReset & operator=(const ScopedReset &);
};

#include "Vivox.inl"

#endif

