////////////////////////////////////////////////////////////////////////////////
//Vivox.inl
//
//04/07/08 Joshua M. Kriegshauser
// 
//Copyright (c) 2008 Sony Online Entertainment, LLC.  All rights reserved.
////////////////////////////////////////////////////////////////////////////////

#ifndef VIVOX_INL
#define VIVOX_INL

#include <algorithm>
#include <stdio.h>

// Vivox options
const int vivox_answer_mode = mode_verify_answer;
const bool vivox_buddies = false;
const bool vivox_text = true;
const int vivox_update_freq = 10; // See documentation for more details.  5=10 times per sec, 10=5 times per second, 50=1 time per sec, 100=state change only
const float vivox_positional_update_rate = 10.0f; // max number of updates per second
const u64 vivox_max_record_time = 10000; // Milliseconds
const unsigned short vivox_vvs_port = 44125;

#if VIVOX_VERSION >= 3

#define VIVOX_V3_RACE_FIX

#endif // VIVOX_VERSION

struct HINSTANCE__;
extern HINSTANCE__* shVivoxDll;

#define DECLARE_FN( ret, name, parms ) typedef ret (*name##_type) parms; extern name##_type name
DECLARE_FN(void,destroy_resp,(vx_resp_base_t *pCmd));
DECLARE_FN(void,destroy_evt,(vx_evt_base_t *pCmd));
DECLARE_FN(int,vx_get_message,(vx_message_base_t** message));
DECLARE_FN(int,vx_issue_request,(vx_req_base_t* request));
DECLARE_FN(char*,vx_strdup,(const char*));
DECLARE_FN(void,vx_req_connector_initiate_shutdown_create,(vx_req_connector_initiate_shutdown_t ** req));
DECLARE_FN(void,vx_req_account_logout_create,(vx_req_account_logout_t ** req));
DECLARE_FN(void,vx_req_connector_create_create,(vx_req_connector_create_t ** req));
DECLARE_FN(void,vx_req_account_login_create,(vx_req_account_login_t ** req));
DECLARE_FN(void,vx_req_account_channel_add_moderator_create,(vx_req_account_channel_add_moderator ** req));
DECLARE_FN(void,vx_req_account_channel_remove_moderator_create,(vx_req_account_channel_remove_moderator ** req));
DECLARE_FN(void,vx_req_channel_ban_user_create,(vx_req_channel_ban_user ** req));
DECLARE_FN(void,vx_req_channel_kick_user_create,(vx_req_channel_kick_user ** req));
DECLARE_FN(void,vx_req_channel_mute_user_create,(vx_req_channel_mute_user ** req));
DECLARE_FN(void,vx_req_channel_mute_all_users_create,(vx_req_channel_mute_all_users ** req));
DECLARE_FN(void,vx_req_account_channel_update_create,(vx_req_account_channel_update ** req));
DECLARE_FN(void,vx_req_account_channel_get_moderators_create,(vx_req_account_channel_get_moderators ** req));
DECLARE_FN(void,vx_req_sessiongroup_create_create,(vx_req_sessiongroup_create_t ** req));
DECLARE_FN(void,vx_req_sessiongroup_add_session_create,(vx_req_sessiongroup_add_session_t ** req));
DECLARE_FN(void,vx_req_sessiongroup_remove_session_create,(vx_req_sessiongroup_remove_session_t ** req));
DECLARE_FN(void,vx_req_sessiongroup_terminate_create,(vx_req_sessiongroup_terminate_t ** req));
DECLARE_FN(void,vx_req_session_create_create,(vx_req_session_create_t ** req));
DECLARE_FN(void,vx_req_session_terminate_create,(vx_req_session_terminate_t ** req));
DECLARE_FN(void,vx_req_session_media_disconnect_create,(vx_req_session_media_disconnect ** req));
DECLARE_FN(void,vx_req_session_send_message_create,(vx_req_session_send_message ** req));
DECLARE_FN(void,vx_req_connector_mute_local_mic_create,(vx_req_connector_mute_local_mic ** req));
DECLARE_FN(void,vx_req_connector_mute_local_speaker_create,(vx_req_connector_mute_local_speaker ** req));
DECLARE_FN(void,vx_req_connector_set_local_mic_volume_create,(vx_req_connector_set_local_mic_volume ** req));
DECLARE_FN(void,vx_req_connector_set_local_speaker_volume_create,(vx_req_connector_set_local_speaker_volume ** req));
DECLARE_FN(void,vx_req_session_set_participant_mute_for_me_create,(vx_req_session_set_participant_mute_for_me ** req));
DECLARE_FN(void,vx_req_session_set_participant_volume_for_me_create,(vx_req_session_set_participant_volume_for_me ** req));
DECLARE_FN(void,vx_req_aux_get_render_devices_create,(vx_req_aux_get_render_devices ** req));
DECLARE_FN(void,vx_req_aux_set_render_device_create,(vx_req_aux_set_render_device ** req));
DECLARE_FN(void,vx_req_aux_get_capture_devices_create,(vx_req_aux_get_capture_devices ** req));
DECLARE_FN(void,vx_req_aux_set_capture_device_create,(vx_req_aux_set_capture_device ** req));
DECLARE_FN(void,vx_req_aux_start_buffer_capture_create,(vx_req_aux_start_buffer_capture ** req));
DECLARE_FN(void,vx_req_aux_play_audio_buffer_create,(vx_req_aux_play_audio_buffer ** req));
DECLARE_FN(void,vx_req_aux_render_audio_stop_create,(vx_req_aux_render_audio_stop ** req));
DECLARE_FN(void,vx_req_session_set_3d_position_create,(vx_req_session_set_3d_position ** req));
DECLARE_FN(void,vx_req_sessiongroup_set_tx_session_create,(vx_req_sessiongroup_set_tx_session ** req));
DECLARE_FN(void,vx_req_aux_global_monitor_keyboard_mouse_create,(vx_req_aux_global_monitor_keyboard_mouse_t ** req));
DECLARE_FN(void,vx_req_session_mute_local_speaker_create,(vx_req_session_mute_local_speaker ** req));
DECLARE_FN(void,vx_req_session_set_local_speaker_volume_create,(vx_req_session_set_local_speaker_volume ** req));
DECLARE_FN(void,vx_req_session_send_notification_create,(vx_req_session_send_notification ** req));
DECLARE_FN(void,vx_req_aux_capture_audio_start_create,(vx_req_aux_capture_audio_start ** req));
DECLARE_FN(void,vx_req_aux_capture_audio_stop_create,(vx_req_aux_capture_audio_stop ** req));
DECLARE_FN(void,vx_req_aux_set_mic_level_create,(vx_req_aux_set_mic_level ** req));
DECLARE_FN(void,vx_req_aux_set_speaker_level_create,(vx_req_aux_set_speaker_level ** req));
DECLARE_FN(void,vx_on_application_exit,());
#if VIVOX_VERSION >= 3
DECLARE_FN(void,vx_req_aux_diagnostic_state_dump_create,(vx_req_aux_diagnostic_state_dump ** req));
DECLARE_FN(int, vx_alloc_sdk_handle,(const char *address, unsigned short port, VX_SDK_HANDLE *handle));
DECLARE_FN(int, vx_free_sdk_handle,(VX_SDK_HANDLE sdkHandle));
#endif
#undef DECLARE_FN

#define LOAD_DLL() sLoadVivoxDLL( &m_onDllLoaded, &Glue::warning )
bool sStartService( const char* sExe, const char* sIP, int log_level );
bool sLoadVivoxDLL( void (*)(), void (*)( const char*, int, const char*, ... ) );
void sUnloadVivoxDLL();
bool sGetKeyState( int iKeyCode );

// We have to ensure that we're the only instance of Vivox running on this machine,
// so use a named mutex to accomplish this.
bool sGrabVivoxSystemMutex();
bool sReleaseVivoxSystemMutex();

template <typename T>
inline
T VivoxClamp( T Min, T Cur, T Max )
{
    if ( Cur < Min )
        return Min;
    else if ( Cur > Max )
        return Max;
    return Cur;
}

#ifdef _DEBUG
#define vivox_db_fatal( x ) if ( !(x) ) Glue::fatal( __FILE__, __LINE__, #x )
#else
// Don't do anything for non-debug builds
#define vivox_db_fatal( x ) static_cast< void >( x )
#endif

#define vivox_fatal( x ) if ( !(x) ) Glue::fatal( __FILE__, __LINE__, #x )

// Compile-time fatal
#define vivox_ct_fatal( x ) { int __unused[ x ? 1 : -1 ]; static_cast< void >( __unused[0] ); }

// Helper macro to fire events
// This also correctly handles the event where handlers might try to remove themselves
// during iteration
#define FIRE_EVENT( __fn, __args ) \
    do { \
        ++m_iFiringEvents; \
        for ( EventHandlers::const_iterator __iter = m_aHandlers.begin(); __iter != m_aHandlers.end(); ++__iter ) \
            (*__iter)-> __fn __args; \
        if ( --m_iFiringEvents == 0 && m_pNewHandlers ) \
        { \
            m_aHandlers.swap( *m_pNewHandlers ); \
            delete m_pNewHandlers; \
            m_pNewHandlers = 0; \
        } \
    } while( 0 )


////////////////////////////////////////////////////////////////////////////////
// Static variables
////////////////////////////////////////////////////////////////////////////////
#if VIVOX_VERSION >= 3
template <typename StrClass, typename Glue>
const StrClass Vivox<StrClass, Glue>::NO_DEVICE( "No Device" );

template <typename StrClass, typename Glue>
const StrClass Vivox<StrClass, Glue>::DEFAULT_DEVICE( "Default System Device" );
#endif

////////////////////////////////////////////////////////////////////////////////
// Constructor
////////////////////////////////////////////////////////////////////////////////
template <typename StrClass, typename Glue>
Vivox<StrClass, Glue>::Vivox() :
    m_pNewHandlers( 0 ),
    m_iFiringEvents( 0 ),
    m_uID( 1000 ),       // Arbitrary starting ID value from Vivox sample app
    m_sdkHandle( 0 ),
    m_eConnectState( CS_DISCONNECTED ),
    m_eLoginState( LS_LOGGED_OUT ),
    m_iNextMinPort( 0 ),
    m_iNextMaxPort( 0 ),
    m_bNeedConnect( false ),
    m_bNeedDisconnect( false ),
    m_bNeedDisconnectIsGlobal( false ),
    m_bHandsOff( false ),
    m_bNeedLogin( false ),
    m_bNeedLogout( false ),
    m_bUsingVoiceService( false ),
    m_bVoiceServiceConnected( false ),
    m_bMicMutePending( false ),
    m_bLocalSpeakerMute( false ),
    m_bLocalMicMute( false ),
    m_fLocalSpeakerVolume( 0.5f ),
    m_fLocalMicVolume( 0.5f ),
    m_bPosDirty( false ),
    m_lastPosUpdateTime( 0 ),
    m_iLastError( 0 ),
    m_bNotificationPending( false ),
    m_bPushToTalk( false ),
    m_bTalking( false ),
    m_bPTTForced( false ),
    m_bPreventPTT( false ),
    m_testType( TT_NONE ),
    m_testState( TS_NOT_RUNNING ),
    m_bPreTestMute( false ),
    m_bNeedStop( false ),
    m_bSessionsSuspendedInternal( false ),
    m_iSessionsSuspended( 0 ),
    m_uEchoTestStartTime( 0 ),
    m_uEchoTestLength( 0 ),
    m_bTextMessagePending( false ),
    m_bIsHandlingUnsolicitedResponse( false ),
    m_bIsJoiningExistingSession( false )
{}

////////////////////////////////////////////////////////////////////////////////
// Destructor
////////////////////////////////////////////////////////////////////////////////
template <typename StrClass, typename Glue>
Vivox<StrClass, Glue>::~Vivox()
{
    // Doing this while firing events wouldn't be good
    vivox_fatal( 0 == m_iFiringEvents );
    Shutdown();

    // Wait for shutdown to complete
    int iMaxIter = 300; // 300 * 100 = 30000 ms or 30 sec
    while ( !isDisconnected() && --iMaxIter >= 0 )
    {
        ProcessEvents();
        Glue::sleep( 100 );
    }

    delete m_pNewHandlers;
    m_pNewHandlers = 0;

    // Unload DLL since we're done with it
	sUnloadVivoxDLL();
}


////////////////////////////////////////////////////////////////////////////////
// AddHandler
// 
// Adds a callback mechanism
// Safely performed during an event, but your handler will not be called
// if added during an event.
////////////////////////////////////////////////////////////////////////////////
template <typename StrClass, typename Glue>
void
Vivox<StrClass, Glue>::AddHandler( VivoxEventHandler* pHandler )
{
    if ( pHandler )
    {
        EventHandlers* pHandlers = &m_aHandlers;
        if ( m_iFiringEvents )
        {
            // Firing events and can't monkey with the original list, so create temporary
            if ( 0 == m_pNewHandlers ) m_pNewHandlers = new EventHandlers( m_aHandlers );
            pHandlers = m_pNewHandlers;
        }
        // Make sure we're unique
        RemoveHandler( pHandler );
        pHandlers->push_back( pHandler );
    }
}


////////////////////////////////////////////////////////////////////////////////
// RemoveHandler
// 
// Removes a callback mechanism
// Safely performed during an event, but your handler will not be removed
// until the entire event chain has completed.
////////////////////////////////////////////////////////////////////////////////
template <typename StrClass, typename Glue>
bool
Vivox<StrClass, Glue>::RemoveHandler( VivoxEventHandler* pHandler )
{
    EventHandlers* pHandlers = &m_aHandlers;
    if ( m_iFiringEvents )
    {
        // Firing events and can't monkey with the original list, so create temporary
        if ( 0 == m_pNewHandlers ) m_pNewHandlers = new EventHandlers( m_aHandlers );
        pHandlers = m_pNewHandlers;
    }

    // Maintain order instead of doing swap and chop
    for ( unsigned i = 0; i != pHandlers->size(); ++i )
    {
        if ( (*pHandlers)[ i ] == pHandler )
        {
            pHandlers->erase( pHandlers->begin() + i );
            return true;
        }
    }
    return false;
}


////////////////////////////////////////////////////////////////////////////////
// ProcessEvents
// 
// Handles responses from vivox and fires off event notification
////////////////////////////////////////////////////////////////////////////////
template <typename StrClass, typename Glue>
void
Vivox<StrClass, Glue>::ProcessEvents()
{
    // Only process events if the DLL is loaded, but don't load the DLL
    // as a result of trying to process events.
	if ( 0 == shVivoxDll ) return;

    const u64 uTimeNow = Glue::getTimeMS();

    // Do position updates if necessary
    // These are done at the rate of 'vivox_positional_update_rate' times per second
    if ( m_bPosDirty &&
         vivox_positional_update_rate > 0 &&
         static_cast< float >( uTimeNow - m_lastPosUpdateTime ) > ( 1000.0f / vivox_positional_update_rate ) )
    {
        m_lastPosUpdateTime = uTimeNow;
        // Don't mark clean unless the send succeeded
        m_bPosDirty = !m_sendPositionUpdate();
    }

    vx_message_base* pBaseMsg = 0;

    bool bUnload = false;

    // Handle push-to-talk key events
    if ( !m_aPTTKeys.empty() )
    {
        bool bTalking = false;
        for ( unsigned i = 0; i != m_aPTTKeys.size(); ++i )
        {
            if ( sGetKeyState( m_aPTTKeys[ i ] ) )
            {
                bTalking = true;
                break;
            }
        }
        if ( bTalking != m_bTalking )
        {
            m_bTalking = bTalking;
            m_handlePTTEvent( IsPushToTalkTalking() );
        }
    }

    // Echo test events
    if ( TT_ECHO_RECORD == m_testType && m_uEchoTestStartTime != 0 )
    {
        if ( uTimeNow - m_uEchoTestStartTime >= vivox_max_record_time )
        {
            // Need to stop playback/echo test after 10 sec so that we don't trip the Vivox SDK crash
            StopEchoTest();
        }
    }

    // Echo test playback checking
    if ( TT_ECHO_PLAYBACK == m_testType && m_uEchoTestStartTime != 0 )
    {
        if ( uTimeNow - m_uEchoTestStartTime >= m_uEchoTestLength )
        {
            StopEchoTest();
        }
    }

    // Process all available messages
    while ( 0 == vx_get_message( &pBaseMsg ) )
    {
        // Always reset m_bIsHandlingUnsolicitedResponse when done processing
        ScopedReset resetter( m_bIsHandlingUnsolicitedResponse );

        // Process the Vivox message
        vivox_db_fatal( pBaseMsg );
        switch ( pBaseMsg->type )
        {
        case msg_event:
            {
                vx_evt_base* pEvent = (vx_evt_base*)pBaseMsg;

                // If we are connecting through the VVS service, we might get events before we get the response
                // to our state dump request.  We can safely ignore these (with the notable exception of 
                if ( !isConnecting() )
                {    
                    switch ( pEvent->type )
                    {
                    case evt_account_login_state_change: m_processAccountLoginStateChange( *(vx_evt_account_login_state_change*)pEvent ); break;
                    case evt_sessiongroup_added:         m_processSessionGroupAdded( *(vx_evt_sessiongroup_added*)pEvent ); break;
                    case evt_sessiongroup_removed:       m_processSessionGroupRemoved( *(vx_evt_sessiongroup_removed*)pEvent ); break;
                    case evt_session_added:              m_processSessionAdded( *(vx_evt_session_added*)pEvent ); break;
                    case evt_session_removed:            m_processSessionRemoved( *(vx_evt_session_removed*)pEvent ); break;
                    case evt_session_updated:            m_processSessionUpdated( *(vx_evt_session_updated*)pEvent ); break;
                    case evt_session_notification:       m_processSessionNotification( *(vx_evt_session_notification*)pEvent ); break;
                    case evt_participant_added:          m_processParticipantAdded( *(vx_evt_participant_added*)pEvent ); break;
                    case evt_participant_removed:        m_processParticipantRemoved( *(vx_evt_participant_removed*)pEvent ); break;
                    case evt_participant_updated:        m_processParticipantUpdated( *(vx_evt_participant_updated*)pEvent ); break;
                    case evt_media_stream_updated:       m_processMediaStreamUpdated( *(vx_evt_media_stream_updated*)pEvent ); break;
                    case evt_idle_state_changed:         m_processIdleStateChanged( *(vx_evt_idle_state_changed*)pEvent ); break;
                    case evt_keyboard_mouse:             m_processKeyboardMouseEvent( *(vx_evt_keyboard_mouse*)pEvent ); break;
                    case evt_aux_audio_properties:       m_processAudioProperties( *(vx_evt_aux_audio_properties*)pEvent ); break;
                    case evt_text_stream_updated:        m_processTextStreamUpdated( *(vx_evt_text_stream_updated*)pEvent ); break;
                    case evt_message:                    m_processTextMessage( *(vx_evt_message*)pEvent ); break;
    
#if VIVOX_VERSION >= 3
                    case evt_buddy_and_group_list_changed: break; // don't care
                    case evt_server_app_data:
                        {
                            vx_evt_server_app_data* p = (vx_evt_server_app_data*)pEvent;
                            Glue::log( VLS_DEBUG, "Server app data: %s/%s", p->content_type, p->content );
                        }
                        break;
                    case evt_user_app_data:
                        {
                            vx_evt_user_app_data* p = (vx_evt_user_app_data*)pEvent;
                            Glue::log( VLS_DEBUG, "User app data: (uri:%s) %s/%s", p->from_uri, p->content_type, p->content );
                        }
                        break;
                    case evt_voice_service_connection_state_changed:
                        m_handleVoiceServiceConnectionStateChanged( *(vx_evt_voice_service_connection_state_changed*)pEvent );
                        break;
#endif
    
                    default:
                        Glue::log( VLS_DEBUG, "Unhandled Vivox event: %u", pEvent->type );
                    }
                }
#if VIVOX_VERSION >= 3
                else if ( pEvent->type == evt_voice_service_connection_state_changed )
                {
                    m_handleVoiceServiceConnectionStateChanged( *(vx_evt_voice_service_connection_state_changed*)pEvent );
                }
#endif
                else
                {
                    Glue::log( VLS_DEBUG, "Ignoring event received during connect: %u", pEvent->type );
                }

                destroy_evt( pEvent );
            }
            break;

        case msg_response:
            {
                vx_resp_base* pResponse = (vx_resp_base*)pBaseMsg;

                if ( 0 == m_pendingRequests.erase( pResponse->request->cookie ) )
                {
                    Glue::log( VLS_DEBUG,
                               "Unsolicited response: %d/%s  %d %d %s",
                               pResponse->type,
                               m_getRespTypeName( pResponse->type ),
                               pResponse->return_code,
                               pResponse->status_code,
                               pResponse->status_string );

#if VIVOX_VERSION >= 3
                    // Wasn't our request, but this can happen if we're talking to the VVS in v3
                    // since other applications may also be controlling the show.  We only want to
                    // hear certain types of responses

                    bool bTreatAsLocal = false;
                    m_bIsHandlingUnsolicitedResponse = true;

                    switch ( pResponse->type )
                    {
                    case resp_account_login:
                    case resp_sessiongroup_add_session:
                    case resp_sessiongroup_remove_session:
                    case resp_aux_set_render_device:
                    case resp_aux_set_capture_device:
                    case resp_account_channel_get_moderators:
                        // Handle these like they were our request
                        bTreatAsLocal = true;
                        break;

                    case resp_aux_set_mic_level:
                        if ( !m_logAnyError( *pResponse ) )
                        {
                            // Something else is changing our mic volume
                            vx_req_aux_set_mic_level* req = (vx_req_aux_set_mic_level*)pResponse->request;
                            m_fLocalMicVolume = static_cast< float >( req->level ) / 100.0f;
                        }
                        break;

                    case resp_connector_set_local_mic_volume:
                        if ( !m_logAnyError( *pResponse ) )
                        {
                            // Something else is changing our mic volume
                            vx_req_connector_set_local_mic_volume* req = (vx_req_connector_set_local_mic_volume*)pResponse->request;
                            m_fLocalMicVolume = static_cast< float >( req->volume ) / 100.0f;
                        }
                        break;

                    case resp_aux_set_speaker_level:
                        if ( !m_logAnyError( *pResponse ) )
                        {
                            // Something else is changing our speaker volume
                            vx_req_aux_set_speaker_level* req = (vx_req_aux_set_speaker_level*)pResponse->request;
                            m_fLocalSpeakerVolume = static_cast< float >( req->level ) / 100.0f;
                        }
                        break;

                    case resp_connector_set_local_speaker_volume:
                        if ( !m_logAnyError( *pResponse ) )
                        {
                            // Something else is changing our speaker volume
                            vx_req_connector_set_local_speaker_volume* req = (vx_req_connector_set_local_speaker_volume*)pResponse->request;
                            m_fLocalSpeakerVolume = static_cast< float >( req->volume ) / 100.0f;
                        }
                        break;
                    }

                    if ( !bTreatAsLocal )
                    {
                        destroy_resp( pResponse );
                        continue;
                    }
#endif
                }

                if ( pResponse->return_code != 0 )
                    m_setLastError( pResponse->status_code, pResponse->status_string );
                else
                    m_setLastError( 0, StrClass() );

                switch ( pResponse->type )
                {
                case resp_connector_create:        m_processConnectorCreateResp( *(vx_resp_connector_create*)pResponse ); break;
                case resp_account_login:           m_processAccountLoginResp( *(vx_resp_account_login*)pResponse ); break;
                case resp_aux_get_render_devices:  m_processGetRenderDevicesResp( *(vx_resp_aux_get_render_devices*)pResponse ); break;
                case resp_aux_get_capture_devices: m_processGetCaptureDevicesResp( *(vx_resp_aux_get_capture_devices*)pResponse ); break;
                case resp_aux_set_render_device:   m_processSetRenderDeviceResp( *(vx_resp_aux_set_render_device*)pResponse ); break;
                case resp_aux_set_capture_device:  m_processSetCaptureDeviceResp( *(vx_resp_aux_set_capture_device*)pResponse ); break;
                case resp_account_channel_get_moderators: m_processGetModeratorsResp( *(vx_resp_account_channel_get_moderators*)pResponse ); break;
#if VIVOX_VERSION >= 3
                case resp_aux_diagnostic_state_dump: m_processDiagnosticStateDumpResp( *(vx_resp_aux_diagnostic_state_dump*)pResponse ); break;
#endif

                case resp_connector_initiate_shutdown:
                    m_logAnyError( *pResponse );
                    vivox_db_fatal( CS_DISCONNECTING == m_eConnectState );
                    m_eConnectState = CS_DISCONNECTED;
                    m_bNeedDisconnect = false;
                    m_sConnectionHandle.clear();
                    DisconnectVoiceService(); // If necessary...

                    sReleaseVivoxSystemMutex();

                    if ( m_bNeedConnect )
                    {
                        FIRE_EVENT(VivoxOnDisconnect,(true));

                        m_bNeedConnect = false;
                        BeginConnect( m_sNextServer, m_iNextMinPort, m_iNextMaxPort, m_sNextServiceExe, m_sNextServiceIP );
                        m_sNextServer.clear();
                        m_sNextServiceExe.clear();
                        m_sNextServiceIP.clear();
                    }
                    else
                        bUnload = true;
                    break;

                case resp_account_logout:
                    m_logAnyError( *pResponse );
                    vivox_db_fatal( LS_LOGGING_OUT == m_eLoginState || LS_LOGGED_OUT == m_eLoginState );
                    // Wait for evt_account_login_state_change before actually changing anything
                    break;

                case resp_connector_mute_local_speaker:
                case resp_connector_set_local_mic_volume:
                case resp_connector_set_local_speaker_volume:
                case resp_sessiongroup_create:
                case resp_sessiongroup_terminate:
                case resp_sessiongroup_set_tx_session:
                case resp_session_terminate:
                case resp_session_set_participant_mute_for_me:
                case resp_session_set_participant_volume_for_me:
                case resp_session_set_3d_position:
                case resp_session_mute_local_speaker:
                case resp_session_set_local_speaker_volume:
                case resp_session_media_disconnect:
                case resp_aux_global_monitor_keyboard_mouse:
                case resp_aux_set_mic_level:
                case resp_aux_set_speaker_level:
                    // Just log error for these responses; we don't care about them otherwise
                    m_logAnyError( *pResponse );
                    break;

                case resp_session_send_message:
                    vivox_db_fatal( m_bTextMessagePending );
                    m_logAnyError( *pResponse );
                    m_bTextMessagePending = false;

                    // Try to send the next remaining message
                    if ( !m_mTextMessages.empty() )
                    {
                        bool bSent = false;
                        for ( TextMessageMap::iterator iter( m_mTextMessages.begin() );
                              iter != m_mTextMessages.end() && !bSent;
                              /*in loop*/ )
                        {
                            const StrClass& sSession = (*iter).first;
                            TextMessageList& v = (*iter).second;
                            if ( !v.empty() )
                            {
                                TextMessage& t = v.front();
                                m_sendTextMessage( sSession, t.sHeader, t.sBody );
                                v.pop_front();
                                bSent = true;
                            }
                            if ( v.empty() )
                                m_mTextMessages.erase( iter++ );
                            else
                                ++iter;
                        }
                    }
                    break;

                case resp_aux_capture_audio_stop:
                    vivox_fatal( TT_ECHO_RECORD == m_testType || TT_VU_MONITOR == m_testType );
                    m_logAnyError( *pResponse );
                    // The response for the aux_capture_audio_stop causes events to fire
                    {
                        m_testState = TS_EVENTS;
                    
                        if ( TT_ECHO_RECORD == m_testType )
                        {
                            FIRE_EVENT(VivoxOnEchoTestEnd,(m_uEchoTestLength));
                        }
                        else if ( TT_VU_MONITOR == m_testType )
                        {
                            FIRE_EVENT(VivoxOnVUMonitorEnd,());
                        }

                        m_testType = TT_NONE;
                        m_testState = TS_NOT_RUNNING;

                        // Need to reinstate suspended sessions.
                        m_unsuspendSessions( true );
                    }
                    break;

                case resp_aux_render_audio_stop:
                    vivox_fatal( TT_ECHO_PLAYBACK == m_testType );
                    m_logAnyError( *pResponse );
                    // The response for the aux_render_audio_stop causes events to fire
                    {
                        m_testState = TS_EVENTS;
                    
                        FIRE_EVENT(VivoxOnEchoTestPlaybackEnd,(m_uEchoTestLength));

                        m_testType = TT_NONE;
                        m_testState = TS_NOT_RUNNING;

                        // Need to reinstate suspended sessions.
                        m_unsuspendSessions( true );
                    }
                    break;

                case resp_session_send_notification:
                    m_bNotificationPending = false;
                    m_logAnyError( *pResponse );
                    {
                        // If the current push-to-talk talking value doesn't match the notification we sent, send the
                        // correct event
                        vx_req_session_send_notification* pReq = (vx_req_session_send_notification*)pResponse->request;
                        const vx_notification_type notification_type = IsPushToTalkTalking() ? notification_hand_raised : notification_hand_lowered;
                        if ( notification_type != pReq->notification_type )
                        {
                            // Need to update the notification type
                            m_sendPTTNotification( pReq->session_handle, IsPushToTalkTalking() );
                        }
                    }
                    break;

                case resp_connector_mute_local_mic:
                    m_bMicMutePending = false;
                    m_logAnyError( *pResponse );
                    {
                        // If our local mic mute value doesn't match what we just got a response for, correct the
                        // mic mute.
                        vx_req_connector_mute_local_mic* pReq = (vx_req_connector_mute_local_mic*)pResponse->request;
                        if ( ( pReq->mute_level != 0 ) != m_bLocalMicMute )
                        {
                            SetLocalMicMute( m_bLocalMicMute );
                        }
                    }
                    break;

                case resp_aux_start_buffer_capture:
                    vivox_fatal( m_testType == TT_ECHO_RECORD );
                    m_testState = TS_RUNNING;
                    m_bPreTestMute = m_bLocalMicMute;
                    if ( m_logAnyError( *pResponse ) )
                    {
                        m_uEchoTestLength = 0;
                        m_stopVUMonitor();
                        FIRE_EVENT(VivoxOnEchoTestError,());
                    }
                    else
                    {
                        m_uEchoTestStartTime = Glue::getTimeMS();
                        FIRE_EVENT(VivoxOnEchoTestStart,());

                        if ( m_bNeedStop )
                            m_stopVUMonitor();
                        else
                        {
                            // Un-mute for test
                            SetLocalMicMute( false );
                        }
                    }
                    break;

                case resp_aux_play_audio_buffer:
                    vivox_fatal( m_testType == TT_ECHO_PLAYBACK );
                    m_testState = TS_RUNNING;
                    if ( m_logAnyError( *pResponse ) )
                    {
                        m_uEchoTestLength = 0;
                        m_stopEchoPlayback();
                        FIRE_EVENT(VivoxOnEchoTestError,());
                    }
                    else
                    {
                        m_uEchoTestStartTime = Glue::getTimeMS();
                        FIRE_EVENT(VivoxOnEchoTestPlaybackStart,(m_uEchoTestLength));

                        if ( m_bNeedStop )
                            m_stopEchoPlayback();
                    }
                    break;

                case resp_aux_capture_audio_start:
                    vivox_fatal( m_testType == TT_VU_MONITOR );
                    m_testState = TS_RUNNING;
                    m_bPreTestMute = m_bLocalMicMute;
                    if ( m_logAnyError( *pResponse ) )
                    {
                        m_stopVUMonitor();
                    }
                    else
                    {
                        FIRE_EVENT(VivoxOnVUMonitorStart,());

                        if ( m_bNeedStop )
                            m_stopVUMonitor();
                        else
                        {
                            // Un-mute for test
                            SetLocalMicMute( false );
                        }
                    }
                    break;

                case resp_sessiongroup_add_session:
                    if ( m_logAnyError( *pResponse ) )
                    {
                        // Couldn't add the session for some reason, but it's no longer our current deferred session
                        vx_req_sessiongroup_add_session* req = (vx_req_sessiongroup_add_session*)pResponse->request;
                        StrClass sURI( req->uri );
                        m_mSessionInfo.erase( sURI );

                        // Start connecting the next deferred session for this session group
                        StrClass sGroup( req->sessiongroup_handle );
                        vivox_db_fatal( m_mGroupInfo.count( sGroup ) != 0 );
                        GroupInfo& info = m_mGroupInfo[ sGroup ];
                        m_removeDeferred( info, sURI, false );
                        m_processNextDeferred( info, false );
                    }
                    else if ( m_bIsHandlingUnsolicitedResponse )
                    {
                        vx_req_sessiongroup_add_session* pReq = (vx_req_sessiongroup_add_session*)pResponse->request;

                        // Some other application's add session request; need to add it to our deferred list
                        StrClass sSessionGroup( pReq->sessiongroup_handle );
                        vivox_db_fatal( m_mGroupInfo.count( sSessionGroup ) != 0 );
                        GroupInfo& group = m_mGroupInfo[ sSessionGroup ];

                        DeferredSessionInfo& info = *(group.vDeferred.insert(group.vDeferred.end(), DeferredSessionInfo()));
                        info.bTerminate = false;
                        info.bForeign = true;
                        info.sURI = pReq->uri;
                        info.sDisplayName = pReq->name;
                        info.sPassword = pReq->password;

                        // Add SessionInfo
                        SessionInfo& session = m_mSessionInfo[ info.sURI ];
                        session.sURI = info.sURI;
                        session.sDisplayName = info.sDisplayName;
                        session.sPassword = info.sPassword;
                        session.sGroup = sSessionGroup;
                    }
                    break;

                case resp_sessiongroup_remove_session:
                    if ( m_logAnyError( *pResponse ) )
                    {
                        // Couldn't remove the session for some reason, but it's no longer our current deferred session
                        vx_req_sessiongroup_remove_session* req = (vx_req_sessiongroup_remove_session*)pResponse->request;

                        StrClass sGroup( req->sessiongroup_handle );
                        vivox_db_fatal( m_mGroupInfo.count( sGroup ) != 0 );
                        GroupInfo& info = m_mGroupInfo[ sGroup ];
                        m_removeDeferred( info, req->session_handle, true );
                        m_processNextDeferred( info, false );
                    }
                    else if ( m_bIsHandlingUnsolicitedResponse )
                    {
                        vx_req_sessiongroup_remove_session* pReq = (vx_req_sessiongroup_remove_session*)pResponse->request;

                        // Some other application's remove session request; need to add it to our deferred list
                        StrClass sSessionGroup( pReq->sessiongroup_handle );
                        vivox_db_fatal( m_mGroupInfo.count( sSessionGroup ) != 0 );
                        GroupInfo& group = m_mGroupInfo[ sSessionGroup ];

                        DeferredSessionInfo& info = *(group.vDeferred.insert(group.vDeferred.end(), DeferredSessionInfo()));
                        info.bTerminate = true;
                        info.bForeign = true;
                        info.sURI = pReq->session_handle;
                    }
                    break;

                case resp_session_create:
                    if ( m_logAnyError( *pResponse ) )
                    {
                        // Need to remove the session from our session info map
                        vx_req_session_create* p = (vx_req_session_create*)pResponse->request;
                        m_mSessionInfo.erase( p->uri );
                    }
                    break;

                // Moderator responses
                case resp_account_channel_add_moderator:
                    {
                        vx_req_account_channel_add_moderator* p = (vx_req_account_channel_add_moderator*)pResponse->request;
                        m_fireModEvent( MCT_ADDMOD, pResponse->return_code == 0 ? 0 : pResponse->status_code, pResponse->status_string, p->channel_uri, p->moderator_uri );
                    }
                    break;

                case resp_account_channel_remove_moderator:
                    {
                        vx_req_account_channel_remove_moderator* p = (vx_req_account_channel_remove_moderator*)pResponse->request;
                        m_fireModEvent( MCT_DELETEMOD, pResponse->return_code == 0 ? 0 : pResponse->status_code, pResponse->status_string, p->channel_uri, p->moderator_uri );
                    }
                    break;

                case resp_channel_ban_user:
                    {
                        vx_req_channel_ban_user* p = (vx_req_channel_ban_user*)pResponse->request;
                        m_fireModEvent( p->set_banned != 0 ? MCT_BAN : MCT_UNBAN, pResponse->return_code == 0 ? 0 : pResponse->status_code, pResponse->status_string, p->channel_uri, p->participant_uri );
                    }
                    break;

                case resp_channel_kick_user:
                    {
                        vx_req_channel_kick_user* p = (vx_req_channel_kick_user*)pResponse->request;
                        m_fireModEvent( MCT_KICK, pResponse->return_code == 0 ? 0 : pResponse->status_code, pResponse->status_string, p->channel_uri, p->participant_uri );
                    }
                    break;

                case resp_channel_mute_user:
                    {
                        vx_req_channel_mute_user* p = (vx_req_channel_mute_user*)pResponse->request;
                        m_fireModEvent( p->set_muted != 0 ? MCT_MUTE : MCT_UNMUTE, pResponse->return_code == 0 ? 0 : pResponse->status_code, pResponse->status_string, p->channel_uri, p->participant_uri );
                    }
                    break;

                case resp_channel_mute_all_users:
                    {
                        vx_req_channel_mute_all_users* p = (vx_req_channel_mute_all_users*)pResponse->request;
                        m_fireModEvent( p->set_muted != 0 ? MCT_MUTEALL : MCT_UNMUTEALL, pResponse->return_code == 0 ? 0 : pResponse->status_code, pResponse->status_string, p->channel_uri, 0 );
                    }
                    break;

                case resp_account_channel_update:
                    {
                        vx_req_account_channel_update* p = (vx_req_account_channel_update*)pResponse->request;
                        m_fireModEvent( MCT_CHANNELMODE, pResponse->return_code == 0 ? 0 : pResponse->status_code, pResponse->status_string, p->channel_uri, 0 );
                    }
                    break;

                default:
                    Glue::log( VLS_DEBUG, "Unhandled Vivox response: %u (ret: %d; status %d/\"%s\")", pResponse->type, pResponse->return_code, pResponse->status_code, pResponse->status_string );
                }

                destroy_resp( pResponse );
            }
            break;
        }
    }

    // If code is added here, be aware that some events and responses may return from the function and this
    // code may not be reached until ProcessEvents() is called again

    if ( bUnload )
    {
        sUnloadVivoxDLL();

        FIRE_EVENT(VivoxOnDisconnect,(false));
    }

    // If all requests are finally satisfied, we can log out
    if ( m_eLoginState == LS_WAITING_FOR_REQUESTS && m_pendingRequests.empty() )
    {
        m_sendLogoutRequest();
    }

    if ( m_bNeedLogout )
    {
        m_bNeedLogout = false;
        Logout();
    }

    if ( m_bNeedDisconnect )
    {
        m_bNeedDisconnect = false;
        Shutdown( m_bNeedDisconnectIsGlobal );
    }
}


////////////////////////////////////////////////////////////////////////////////
// FindSession
// 
// Attempts to find a Session object given the session handle
/////////////////////////////////////////////////////////////////////////////
template <typename StrClass, typename Glue>
typename const Vivox<StrClass, Glue>::Session*
Vivox<StrClass, Glue>::FindSession( const StrClass& sSession ) const
{
    SessionMap::const_iterator iter( m_mSessions.find( sSession ) );
    if ( iter != m_mSessions.end() )
    {
        return &(*iter).second;
    }

    return 0;
}


////////////////////////////////////////////////////////////////////////////////
// FindSessionByURI
// 
// Attempts to find a Session object given the session URI.
// Performs a linear search of all sessions
/////////////////////////////////////////////////////////////////////////////
template <typename StrClass, typename Glue>
typename const Vivox<StrClass, Glue>::Session*
Vivox<StrClass, Glue>::FindSessionByURI( const StrClass& sSessionURI ) const
{
    for ( SessionMap::const_iterator iter( m_mSessions.begin() );
          iter != m_mSessions.end();
          ++iter )
    {
        const Session& session = (*iter).second;
        if ( session.sURI == sSessionURI ||
             ( !session.sAliasURI.empty() && session.sAliasURI == sSessionURI ) )
        {
            return &session;
        }
    }

    return 0;
}


////////////////////////////////////////////////////////////////////////////////
// FindParticipant
// 
// Attempts to find a Participant object given the URI and session handle and optionally
// the session group handle
////////////////////////////////////////////////////////////////////////////////
template <typename StrClass, typename Glue>
typename const Vivox<StrClass, Glue>::Participant*
Vivox<StrClass, Glue>::FindParticipant( const StrClass& sURI, const StrClass& sSession ) const
{
    const Session* pSession = FindSession( sSession );
    if ( 0 == pSession )
        return 0;

    Session::ParticipantMap::const_iterator iter( pSession->mParticipants.find( sURI ) );
    if ( iter == pSession->mParticipants.end() )
        return 0;

    return &(*iter).second;
}


////////////////////////////////////////////////////////////////////////////////
// SetPositionalSession
// 
// Sets the session for which positional updates are sent
////////////////////////////////////////////////////////////////////////////////
template <typename StrClass, typename Glue>
bool
Vivox<StrClass, Glue>::SetPositionalSession( const StrClass& sSession )
{
    // Support clearing
    if ( sSession.empty() )
    {
        m_sPositionSession = m_sPositionSessionGroup = StrClass();
        return true;
    }

    const Session* pSession = FindSession( sSession );
    if ( 0 == pSession )
        return false;

    m_sPositionSessionGroup = pSession->sGroupHandle;
    m_sPositionSession = pSession->sHandle;

    return true;
}


////////////////////////////////////////////////////////////////////////////////
// Shutdown
// 
// Fires off a vx_req_connector_initiate_shutdown request
// This function works a little differently based on whether the VVS is
// connected or not.  If it's connected and bGlobal is true, voice chat will be
// disconnected for any other clients of the VVS.  If the VVS is connected but
// bGlobal is false, we merely disconnect from the VVS.  If we were the only
// connection, the VVS will shutdown effectively terminating the voice connection.
// If other clients were present, the VVS will remain connected to Vivox.
// bGlobal is ignored if the VVS is not connected.
// Hands-off mode trumps the global flag (i.e. in hands-off mode, the global
// flag will always be false)
////////////////////////////////////////////////////////////////////////////////
template <typename StrClass, typename Glue>
bool
Vivox<StrClass, Glue>::Shutdown( bool bGlobal /*= false*/ )
{
    if ( m_bVoiceServiceConnected && ( !bGlobal || m_bHandsOff ) )
    {
        return DisconnectVoiceService();
    }
   
    if ( CS_DISCONNECTED == m_eConnectState ||
         CS_DISCONNECTING == m_eConnectState )
    {
        vivox_db_fatal( isLoggedOut() );
        return false;
    }

    if ( isConnecting() )
    {
        // Need to disconnect after connect has completed
        m_bNeedDisconnect = true;
        m_bNeedDisconnectIsGlobal = bGlobal;
        return true;
    }

    if ( !LOAD_DLL() ) return false;

    vivox_db_fatal( CS_CONNECTED == m_eConnectState ||
                    CS_WAITING_FOR_LOGOUT == m_eConnectState );

    m_bNeedDisconnect = false;

    // Send logout
    if ( !isLoggedOut() )
    {
        m_eConnectState = CS_WAITING_FOR_LOGOUT;
        Logout();
        return true;
    }

    vx_req_connector_initiate_shutdown* p = 0;
    vx_req_connector_initiate_shutdown_create( &p );

    p->client_name = vx_strdup( "" ); // From sample app
    p->connector_handle = vx_strdup( m_sConnectionHandle.c_str() );
    m_generateID( p->base );

    Glue::log( VLS_DEBUG, "Sending Shutdown request" );
    if ( m_issueRequest( (vx_req_base*)p ) )
    {
        m_eConnectState = CS_DISCONNECTING;

        return true;
    }

    // Shouldn't get here.
    vivox_db_fatal( 0 );

    return false;
}


////////////////////////////////////////////////////////////////////////////////
// DisconnectVoiceService
// 
// Disconnects from the VVS without changing state.  If another application is
// currently connected to the VVS, this should cause the other application to
// keep the voice channel open and be in control of it.  If no other applications
// are connected, this will cause the VVS to terminate and the voice channel
// will be terminated.
// 
// Version 3 only
////////////////////////////////////////////////////////////////////////////////
template <typename StrClass, typename Glue>
bool
Vivox<StrClass, Glue>::DisconnectVoiceService()
{
#if VIVOX_VERSION >= 3
    if ( !LOAD_DLL() ) return false;

    bool retval = true;

    if ( m_sdkHandle != 0 )
    {
        retval = ( 0 == vx_free_sdk_handle( m_sdkHandle ) );
        m_sdkHandle = 0;
    }

    return retval;
#else
    return false;
#endif
}


////////////////////////////////////////////////////////////////////////////////
// Logout
// 
// Fires off a vx_req_account_logout request
////////////////////////////////////////////////////////////////////////////////
template <typename StrClass, typename Glue>
bool
Vivox<StrClass, Glue>::Logout()
{
    if ( isLoggedOut() || isLoggingOut() )
    {
        return false;
    }

    if ( isLoggingIn() )
    {
        m_bNeedLogout = true;
        return true;
    }

    m_bNeedLogout = false;

    vivox_db_fatal( LS_LOGGED_IN == m_eLoginState );

    if ( !m_pendingRequests.empty() )
    {
        m_eLoginState = LS_WAITING_FOR_REQUESTS;
        return true;
    }

    return m_sendLogoutRequest();
}


////////////////////////////////////////////////////////////////////////////////
// BeginConnect
// 
// Fires off a vx_req_connector_create request.  This function should not be
// called from any VivoxEventHandler functions with the exception of 
// VivoxVoiceServiceStateChange().
////////////////////////////////////////////////////////////////////////////////
template <typename StrClass, typename Glue>
bool
Vivox<StrClass, Glue>::BeginConnect( const StrClass& sServer, int iMinPort, int iMaxPort, const StrClass& sServiceExe, const StrClass& sServiceIP )
{
    if ( m_testHandsOff( "BeginConnect" ) )
        return false;

    if ( isConnecting() )
        return false;

    // Shutdown any previous connection
    //  - If we're connected but talking to the VVS, we'll be disconnected "instantly" when DisconnectVoiceService is called below
    if ( ( isConnected() && !IsVoiceServiceConnected() ) || isDisconnecting() )
    {
        if ( isConnected() )
            Shutdown();

        m_bNeedConnect = true;
        m_sNextServer = sServer;
        m_iNextMinPort = iMinPort;
        m_iNextMaxPort = iMaxPort;
        m_sNextServiceExe = sServiceExe;
        m_sNextServiceIP  = sServiceIP;

        return true;
    }

    m_bNeedConnect = false;

    vivox_db_fatal( CS_DISCONNECTED == m_eConnectState );

    if ( !sGrabVivoxSystemMutex() )
    {
        // Don't spam log when retrying constantly
        static bool bLogMessage = false;
        if ( !bLogMessage )
        {
            Glue::log( VLS_INFO, "Cannot connect to Vivox: cannot run on multiple instances" );
            bLogMessage = true;
        }
        return false;
    }

#if VIVOX_VERSION >= 3
    // If connected...
    const bool kbWantService = !sServiceExe.empty() && !sServiceIP.empty();
    DisconnectVoiceService();

    if ( kbWantService )
    {
        if ( !sStartService( sServiceExe.c_str(), sServiceIP.c_str(), Glue::getLogLevel() ) )
        {
            static bool bLogMessage = false;
            if ( !bLogMessage )
            {
                Glue::log( VLS_ERROR, "Cannot connect to Vivox: Failed to start voice service %s", sServiceExe.c_str() );
                bLogMessage = true;
            }
            return false;
        }
        m_bUsingVoiceService = true;
        m_sVoiceServiceIP = sServiceIP;
    }
    else
    {
        m_bUsingVoiceService = false;
        m_sVoiceServiceIP.clear();
    }
#endif

    if ( !LOAD_DLL() ) return false;

#if VIVOX_VERSION >= 3
    if ( m_bUsingVoiceService )
    {
        // We expect that we don't already have an SDK Handle going into this
        vivox_db_fatal( m_sdkHandle == 0 );

        if ( vx_alloc_sdk_handle( m_sVoiceServiceIP.c_str(), vivox_vvs_port, &m_sdkHandle ) != 0 )
        {
            m_sdkHandle = 0;
            static bool bLogMessage = true;
            if ( bLogMessage )
            {
                Glue::log( VLS_ERROR, "Cannot connect to Vivox Voice Service" );
                bLogMessage = false;
            }
            return false;
        }

        // We expect that this function will give us something other than zero
        vivox_db_fatal( m_sdkHandle != 0 );
    }
#endif

    vx_req_connector_create* p = 0;
    vx_req_connector_create_create( &p );

    p->mode = connector_mode_normal;
    p->acct_mgmt_server = vx_strdup( sServer.c_str() );
    p->minimum_port = iMinPort;
    p->maximum_port = iMaxPort;
    p->client_name = vx_strdup( Glue::getGameName() );
    p->log_filename_prefix = vx_strdup("vx");
    p->log_filename_suffix = vx_strdup(".txt");
    p->log_folder = vx_strdup("logs");
    p->log_level = Glue::getLogLevel();
    m_generateID( p->base );

    m_eConnectState = CS_CONNECTING;

    Glue::log( VLS_DEBUG, "Sending Connect request %s", sServer.c_str() );
    return m_issueRequest((vx_req_base*)p);
}


////////////////////////////////////////////////////////////////////////////////
// BeginLogin
// 
// Fires off a vx_req_account_login request
////////////////////////////////////////////////////////////////////////////////
template <typename StrClass, typename Glue>
bool
Vivox<StrClass, Glue>::BeginLogin( const StrClass& sAcct, const StrClass& sPassword )
{
    if ( m_testHandsOff( "BeginLogin" ) )
        return false;

    if ( !isConnected() )
        return false;

    if ( isLoggingIn() )
        return false;

    // Log out of any previous session
    if ( isLoggedIn() || isLoggingOut() )
    {
        if ( isLoggedIn() )
            Logout();

        m_bNeedLogin = true;
        m_sNextLoginAcct = sAcct;
        m_sNextLoginPassword = sPassword;

        return true;
    }

    m_bNeedLogin = false;

    vivox_db_fatal( LS_LOGGED_OUT == m_eLoginState );

	if ( !LOAD_DLL() ) return false;

    vx_req_account_login* p = 0;
    vx_req_account_login_create( &p );

    p->acct_name = vx_strdup( sAcct.c_str() );
    p->acct_password = vx_strdup( sPassword.c_str() );
    p->answer_mode = (vx_session_answer_mode)vivox_answer_mode;
    p->participant_property_frequency = vivox_update_freq;
    m_generateID( p->base );
    p->connector_handle = vx_strdup( m_sConnectionHandle.c_str() );
    p->enable_buddies_and_presence = vivox_buddies ? 1 : 0;
    p->enable_text = vivox_text ? text_mode_enabled : text_mode_disabled;
    p->buddy_management_mode = mode_application;  // Notify us about buddies

    m_eLoginState = LS_LOGGING_IN;

    Glue::log( VLS_DEBUG, "Sending Login request %s", p->acct_name );
    return m_issueRequest( (vx_req_base*)p );
}


//////////////////////////////////////////////////////////////////////////////////
// GetLocalDisplayName
// 
// Retrieves the local display name as given by the account login response.
// Requires version 3 or later and must be logged in.
////////////////////////////////////////////////////////////////////////////////
template <typename StrClass, typename Glue>
const StrClass&
Vivox<StrClass, Glue>::GetLocalDisplayName() const
{
    return m_sLocalDisplayName;
}


//////////////////////////////////////////////////////////////////////////////////
// GetLocalURI
// 
// Retrieves the local URI as given by the account login response.  Requires
// version 3 or later and must be logged in.
////////////////////////////////////////////////////////////////////////////////
template <typename StrClass, typename Glue>
const StrClass&
Vivox<StrClass, Glue>::GetLocalURI() const
{
    return m_sLocalURI;
}


//////////////////////////////////////////////////////////////////////////////////
// BeginSessionGroup
// 
// Fires off a vx_req_sessiongroup_create request
////////////////////////////////////////////////////////////////////////////////
template <typename StrClass, typename Glue>
bool
Vivox<StrClass, Glue>::BeginSessionGroup()
{
    if ( m_testHandsOff( "BeginSessionGroup" ) )
        return false;

    if ( !isLoggedIn() )
        return false;

    if ( !LOAD_DLL() ) return false;

    vx_req_sessiongroup_create* p = 0;
    vx_req_sessiongroup_create_create( &p );

    m_generateID( p->base );
    p->account_handle = vx_strdup( m_sAccountHandle.c_str() );

    Glue::log( VLS_DEBUG, "Sending SessionGroup Create request" );
    return m_issueRequest( (vx_req_base*)p );
}


//////////////////////////////////////////////////////////////////////////////////
// TerminateSessionGroup
// 
// Fires off a vx_req_sessiongroup_terminate request
////////////////////////////////////////////////////////////////////////////////
template <typename StrClass, typename Glue>
bool
Vivox<StrClass, Glue>::TerminateSessionGroup( const StrClass& sSessionGroup )
{
    if ( m_testHandsOff( "TerminateSessionGroup" ) )
        return false;

    if ( !isLoggedIn() )
        return false;

    if ( !LOAD_DLL() ) return false;

    vx_req_sessiongroup_terminate* p = 0;
    vx_req_sessiongroup_terminate_create( &p );

    m_generateID( p->base );
    p->sessiongroup_handle = vx_strdup( sSessionGroup.c_str() );

    Glue::log( VLS_DEBUG, "Sending SessionGroup Terminate request" );
    if ( m_issueRequest( (vx_req_base*)p ) )
    {
        if ( AreSessionsSuspended() )
        {
            // Need to remove all session info that matches this group
            for ( SessionInfoMap::iterator iter( m_mSessionInfo.begin() );
                  iter != m_mSessionInfo.end();
                  /*in loop*/ )
            {
                SessionInfo& info = (*iter).second;
                if ( info.sGroup == sSessionGroup )
                {
                    m_mSessionInfo.erase( iter++ );
                    continue;
                }
                ++iter;
            }
        }
        return true;
    }

    return false;
}


//////////////////////////////////////////////////////////////////////////////////
// BeginSession
// 
// Fires off a vx_req_session_create request
////////////////////////////////////////////////////////////////////////////////
template <typename StrClass, typename Glue>
bool
Vivox<StrClass, Glue>::BeginSession( const StrClass& sURI, const StrClass& sDisplayName, const StrClass& sPassword, const StrClass& sSessionGroup )
{
    if ( m_testHandsOff( "BeginSession" ) )
        return false;

    if ( !isLoggedIn() )
        return false;

	if ( !LOAD_DLL() ) return false;

    bool bSuccess = true;

    if ( !AreSessionsSuspended() )
    {
        if ( sSessionGroup.empty() )
        {
            // Create a new session and sessiongroup
            vx_req_session_create* p = 0;
            vx_req_session_create_create( &p );
        
            p->connect_audio = 1;
            p->connect_text = vivox_text ? 1 : 0;
            p->name = vx_strdup( sDisplayName.c_str() );
            p->uri = vx_strdup( sURI.c_str() );
            m_generateID( p->base );
            p->account_handle = vx_strdup( m_sAccountHandle.c_str() );
            if ( !sPassword.empty() )
            {
                p->password = vx_strdup( sPassword.c_str() );
            }
            p->password_hash_algorithm = password_hash_algorithm_cleartext;

            Glue::log( VLS_DEBUG, "Sending Session Create request" );
            bSuccess = m_issueRequest( (vx_req_base*)p );
        }
        else
        {
            vivox_db_fatal( m_mGroupInfo.count( sSessionGroup ) != 0 );
            GroupInfo& info = m_mGroupInfo[ sSessionGroup ];

            // Due to a race condition in Vivox's server code, we must always wait for the
            // media stream event when going from zero -> one session.  However, we must
            // actually apply this rule to EVERY session since we can't guarantee at
            // any time that we have whatever number of sessions we think we have
            // (including zero and one) due to race conditions between our code and the
            // Vivox client SDK (i.e. we may have pending messages that we haven't yet
            // processed).
            // 
            // Therefore, anytime we're connecting a session just defer everything else.
            // After we get a media stream event, we call m_processNextDeferred() to 
            // process the next item in the deferred list.
            bool bProcessImmediately = true;
            bool bAdd = true;
            if ( !info.vDeferred.empty() )
            {
                bProcessImmediately = false;
                for ( DeferredSessionList::iterator iter( info.vDeferred.begin() );
                      iter != info.vDeferred.end();
                      ++iter )
                {
                    DeferredSessionInfo& deferred = (*iter);
                    if ( !deferred.bTerminate && deferred.sURI == sURI )
                    {
                        // Already in list
                        if ( deferred.bBeginProcessing )
                        {
                            bProcessImmediately = true;
                            bAdd = false;
                            deferred.bBeginProcessing = false;
                            break;
                        }
                        else
                        {
                            return true;
                        }
                    }
                }
            }

            if ( bAdd )
            {
                // Must add to the deferred list regardless
                DeferredSessionInfo& deferred = *(info.vDeferred.insert(info.vDeferred.end(), DeferredSessionInfo()));
                deferred.sURI = sURI;
                deferred.sDisplayName = sDisplayName;
                deferred.sPassword = sPassword;
            }

            if ( bProcessImmediately )
            {
                // Just add to an existing session group
                vx_req_sessiongroup_add_session* p = 0;
                vx_req_sessiongroup_add_session_create( &p );
        
                p->connect_audio = 1;
                p->connect_text = vivox_text ? 1 : 0;
                p->sessiongroup_handle = vx_strdup( sSessionGroup.c_str() );
                p->uri = vx_strdup( sURI.c_str() );
                p->name = vx_strdup( sDisplayName.c_str() );
                m_generateID( p->base );
                if ( !sPassword.empty() )
                {
                    p->password = vx_strdup( sPassword.c_str() );
                }
                p->password_hash_algorithm = password_hash_algorithm_cleartext;

                Glue::log( VLS_DEBUG, "Sending SessionGroup Add Session request" );
                bSuccess = m_issueRequest( (vx_req_base*)p );
            }
        }
    }

    if ( bSuccess )
    {
        SessionInfo& info = m_mSessionInfo[ sURI ];
        info.sURI = sURI;
        info.sDisplayName = sDisplayName;
        info.sPassword = sPassword;
        info.sGroup = sSessionGroup;
    }

    return bSuccess;
}


//////////////////////////////////////////////////////////////////////////////////
// DisconnectAudio
// 
// Fires off a vx_req_session_media_disconnect request
// Note!  This will disconnect a session if the session is audio-only.
////////////////////////////////////////////////////////////////////////////////
template <typename StrClass, typename Glue>
bool
Vivox<StrClass, Glue>::DisconnectAudio( const StrClass& sSession )
{
    if ( m_testHandsOff( "DisconnectAudio" ) )
        return false;

    // Try to find the session
    const Session* pSession = FindSession( sSession );
    if ( 0 == pSession )
        return false;

	if ( !LOAD_DLL() ) return false;

    vx_req_session_media_disconnect* p = 0;
    vx_req_session_media_disconnect_create( &p );

    m_generateID( p->base );
    p->sessiongroup_handle = vx_strdup( pSession->sGroupHandle.c_str() );
    p->session_handle = vx_strdup( pSession->sHandle.c_str() );

    Glue::log( VLS_DEBUG, "Sending Session Media Disconnect request" );
    return m_issueRequest( (vx_req_base*)p );
}


//////////////////////////////////////////////////////////////////////////////////
// TerminateSession
// 
// Fires off a vx_req_sessiongroup_remove_session request
////////////////////////////////////////////////////////////////////////////////
template <typename StrClass, typename Glue>
bool
Vivox<StrClass, Glue>::TerminateSession( const StrClass& sSession )
{
    if ( m_testHandsOff( "TerminateSession" ) )
        return false;

    // Try to find the session
    Session* pSession = m_findSession( sSession );
    if ( 0 == pSession )
        return false;

    if ( Session::NUM_STATES == pSession->eState )
    {
        // Haven't received media state yet, must defer disconnect
        pSession->SetNeedDisconnect( true );
        return true;
    }

    vivox_db_fatal( m_mGroupInfo.count( pSession->sGroupHandle ) != 0 );
    GroupInfo& info = m_mGroupInfo[ pSession->sGroupHandle ];

    bool bProcessImmediately = true;
    bool bAdd = true;
    if ( !info.vDeferred.empty() )
    {
        // Something else is processing
        bProcessImmediately = false;

        for ( DeferredSessionList::iterator iter( info.vDeferred.begin() );
              iter != info.vDeferred.end();
              ++iter )
        {
            DeferredSessionInfo& deferred = (*iter);
            if ( deferred.bTerminate && deferred.sURI == pSession->sHandle )
            {
                // Already in list
                if ( deferred.bBeginProcessing )
                {
                    bProcessImmediately = true;
                    bAdd = false;
                    deferred.bBeginProcessing = false;
                    break;
                }
                else
                {
                    return true;
                }
            }
        }
    }

    if ( bAdd )
    {
        // Must add to the deferred list regardless since the first item is always the currently processing one
        DeferredSessionInfo& deferred = *(info.vDeferred.insert(info.vDeferred.end(), DeferredSessionInfo()));
        deferred.bTerminate = true;
        deferred.sURI = pSession->sHandle;
    }

    bool bSuccess = true;

    if ( bProcessImmediately )
    {    
        if ( !LOAD_DLL() ) return false;
    
        vx_req_sessiongroup_remove_session* p = 0;
        vx_req_sessiongroup_remove_session_create( &p );
    
        m_generateID( p->base );
        p->sessiongroup_handle = vx_strdup( pSession->sGroupHandle.c_str() );
        p->session_handle = vx_strdup( pSession->sHandle.c_str() );

        Glue::log( VLS_DEBUG, "Sending SessionGroup Remove Session request" );
        bSuccess = m_issueRequest( (vx_req_base*)p );
    }

    return bSuccess;
}


//////////////////////////////////////////////////////////////////////////////////
// TerminateSessionByURI
// 
// Fires off a vx_req_sessiongroup_remove_session request.
// This function is preferred to TerminateSession() because this function can
// also remove sessions from the suspended session list (i.e. during VU
// monitoring, or echo test recording or playback).
////////////////////////////////////////////////////////////////////////////////
template <typename StrClass, typename Glue>
bool
Vivox<StrClass, Glue>::TerminateSessionByURI( const StrClass& sURI )
{
    // Try to find the session
    const Session* pSession = FindSessionByURI( sURI );
    if ( 0 == pSession )
    {
        // We don't have the session, so if we have info about it it's
        // either still connecting or suspended.  If it's suspended, we'll
        // just erase it.  If it's still connecting, we'll mark that it
        // needs a disconnect.
        SessionInfoMap::iterator iter( m_mSessionInfo.find( sURI ) );
        if ( iter != m_mSessionInfo.end() )
        {
            SessionInfo& info = (*iter).second;
            if ( !info.bSuspended )
                info.bNeedDisconnect = true;
            else
                m_mSessionInfo.erase( iter );
            return true;
        }
        return false;
    }

    return TerminateSession( pSession->sHandle );
}


//////////////////////////////////////////////////////////////////////////////////
// SendTextMessage
// 
// Sends a text message to a session if text is connected
// sHeader must be a HTTP content-type (i.e. text/plain, text/html, etc)
// If bNoSpam is true and messages are already queued matching sHeader and
// sBody (case sensitive) this additional message will not be queued.
////////////////////////////////////////////////////////////////////////////////
template <typename StrClass, typename Glue>
bool
Vivox<StrClass, Glue>::SendTextMessage( const StrClass& sSession, const StrClass& sHeader, const StrClass& sBody, bool bNoSpam /*=false*/ )
{
    // No can do if text is disabled
    if ( !vivox_text )
        return false;

    Session* pSession = m_findSession( sSession );
    if ( 0 == pSession )
        return false;

    TextMessageMap::iterator iter( m_mTextMessages.find( sSession ) );
    TextMessageList* v = iter != m_mTextMessages.end() ? &(*iter).second : 0;
    if ( !pSession->bTextConnected || m_bTextMessagePending || ( v && !v->empty() ) )
    {
        // Text not connected yet or message in flight (or prior messages pending), defer until later
        if ( 0 == v )
            v = &( m_mTextMessages[ sSession ] );

        TextMessage msg( sHeader, sBody );
        if ( !bNoSpam || v->end() == std::find( v->begin(), v->end(), msg ) )
        {
            v->push_back( msg );
        }

        return true;
    }

    return m_sendTextMessage( sSession, sHeader, sBody );
}

template <typename StrClass, typename Glue>
bool
Vivox<StrClass, Glue>::m_sendTextMessage( const StrClass& sSession, const StrClass& sHeader, const StrClass& sBody )
{
    vivox_db_fatal( !m_bTextMessagePending );

    if ( !LOAD_DLL() ) return false;

    vx_req_session_send_message* p = 0;
    vx_req_session_send_message_create( &p );

    m_generateID( p->base );
    p->session_handle = vx_strdup( sSession.c_str() );
    p->message_header = vx_strdup( sHeader.c_str() );
    p->message_body   = vx_strdup( sBody.c_str() );

    Glue::log( VLS_DEBUG, "Sending Send Message request" );
    if ( m_issueRequest( (vx_req_base*)p ) )
    {
        m_bTextMessagePending = true;
        return true;
    }

    // Shouldn't get here
    vivox_db_fatal( 0 );

    return false;
}


//////////////////////////////////////////////////////////////////////////////////
// GetModeratorList
// 
// Requests a moderator list for a session.  This MUST be done after the
// VivoxOnMediaStreamUpdated has fired reporting that the session is connected.
// Fires a VivoxOnModeratorList when finished successfully.
////////////////////////////////////////////////////////////////////////////////
template <typename StrClass, typename Glue>
bool
Vivox<StrClass, Glue>::GetModeratorList( const StrClass& sSession )
{
    if ( !isLoggedIn() )
        return false;

    const Session* pSession = FindSession( sSession );
    if ( 0 == pSession )
        return false;

    if ( pSession->eState != Session::MS_CONNECTED )
        return false;

    if ( !LOAD_DLL() ) return false;

    vx_req_account_channel_get_moderators* p = 0;
    vx_req_account_channel_get_moderators_create( &p );

    m_generateID( p->base );
    p->account_handle = vx_strdup( m_sAccountHandle.c_str() );
    p->channel_uri = vx_strdup( pSession->sURI.c_str() );

    Glue::log( VLS_DEBUG, "Sending Get Moderators request" );
    return m_issueRequest( (vx_req_base*)p );
}


//////////////////////////////////////////////////////////////////////////////////
// AddModerator
// 
// Allows a moderator to add another moderator
////////////////////////////////////////////////////////////////////////////////
template <typename StrClass, typename Glue>
bool
Vivox<StrClass, Glue>::AddModerator( const StrClass& sSessionURI, const StrClass& sUserURI )
{
    if ( m_testHandsOff( "AddModerator" ) )
        return false;

    if ( !isLoggedIn() )
        return false;

    if ( !LOAD_DLL() ) return false;

    vx_req_account_channel_add_moderator* p = 0;
    vx_req_account_channel_add_moderator_create( &p );

    m_generateID( p->base );
    p->account_handle = vx_strdup( m_sAccountHandle.c_str() );
    p->channel_uri = vx_strdup( sSessionURI.c_str() );
    p->channel_name = vx_strdup( "" );
    p->moderator_uri = vx_strdup( sUserURI.c_str() );

    Glue::log( VLS_DEBUG, "Sending Add Moderator request" );
    return m_issueRequest( (vx_req_base*)p );
}


//////////////////////////////////////////////////////////////////////////////////
// DeleteModerator
// 
// Allows a moderator to delete another moderator
////////////////////////////////////////////////////////////////////////////////
template <typename StrClass, typename Glue>
bool
Vivox<StrClass, Glue>::DeleteModerator( const StrClass& sSessionURI, const StrClass& sUserURI )
{
    if ( m_testHandsOff( "DeleteModerator" ) )
        return false;

    if ( !isLoggedIn() )
        return false;

    if ( !LOAD_DLL() ) return false;

    vx_req_account_channel_remove_moderator* p = 0;
    vx_req_account_channel_remove_moderator_create( &p );

    m_generateID( p->base );
    p->account_handle = vx_strdup( m_sAccountHandle.c_str() );
    p->channel_uri = vx_strdup( sSessionURI.c_str() );
    p->channel_name = vx_strdup( "" );
    p->moderator_uri = vx_strdup( sUserURI.c_str() );

    Glue::log( VLS_DEBUG, "Sending Remove Moderator request" );
    return m_issueRequest( (vx_req_base*)p );
}


//////////////////////////////////////////////////////////////////////////////////
// SetUserBan
// 
// Allows a moderator to ban or unban another user
////////////////////////////////////////////////////////////////////////////////
template <typename StrClass, typename Glue>
bool
Vivox<StrClass, Glue>::SetUserBan( const StrClass& sSessionURI, const StrClass& sUserURI, bool bBanned )
{
    if ( m_testHandsOff( "SetUserBan" ) )
        return false;

    if ( !isLoggedIn() )
        return false;

    if ( !LOAD_DLL() ) return false;

    vx_req_channel_ban_user* p = 0;
    vx_req_channel_ban_user_create( &p );

    m_generateID( p->base );
    p->account_handle = vx_strdup( m_sAccountHandle.c_str() );
    p->channel_uri = vx_strdup( sSessionURI.c_str() );
    p->channel_name = vx_strdup( "" );
    p->participant_uri = vx_strdup( sUserURI.c_str() );
    p->set_banned = bBanned ? 1 : 0;

    Glue::log( VLS_DEBUG, "Sending Ban User request" );
    return m_issueRequest( (vx_req_base*)p );
}


//////////////////////////////////////////////////////////////////////////////////
// KickUser
// 
// Allows a moderator to kick another user from the channel
////////////////////////////////////////////////////////////////////////////////
template <typename StrClass, typename Glue>
bool
Vivox<StrClass, Glue>::KickUser( const StrClass& sSessionURI, const StrClass& sUserURI )
{
    if ( m_testHandsOff( "KickUser" ) )
        return false;

    if ( !isLoggedIn() )
        return false;

    if ( !LOAD_DLL() ) return false;

    vx_req_channel_kick_user* p = 0;
    vx_req_channel_kick_user_create( &p );

    m_generateID( p->base );
    p->account_handle = vx_strdup( m_sAccountHandle.c_str() );
    p->channel_uri = vx_strdup( sSessionURI.c_str() );
    p->channel_name = vx_strdup( "" );
    p->participant_uri = vx_strdup( sUserURI.c_str() );

    Glue::log( VLS_DEBUG, "Sending Kick User request" );
    return m_issueRequest( (vx_req_base*)p );
}


//////////////////////////////////////////////////////////////////////////////////
// SetUserModMute
// 
// Allows a moderator to mute or unmute another user for everyone in the channel
////////////////////////////////////////////////////////////////////////////////
template <typename StrClass, typename Glue>
bool
Vivox<StrClass, Glue>::SetUserModMute( const StrClass& sSessionURI, const StrClass& sUserURI, bool bMuted )
{
    if ( m_testHandsOff( "SetUserModMute" ) )
        return false;

    if ( !isLoggedIn() )
        return false;

    if ( !LOAD_DLL() ) return false;

    vx_req_channel_mute_user* p = 0;
    vx_req_channel_mute_user_create( &p );

    m_generateID( p->base );
    p->account_handle = vx_strdup( m_sAccountHandle.c_str() );
    p->channel_uri = vx_strdup( sSessionURI.c_str() );
    p->channel_name = vx_strdup( "" );
    p->participant_uri = vx_strdup( sUserURI.c_str() );
    p->set_muted = bMuted ? 1 : 0;

    Glue::log( VLS_DEBUG, "Sending Mute User request" );
    return m_issueRequest( (vx_req_base*)p );
}


//////////////////////////////////////////////////////////////////////////////////
// SetUserMuteAll
// 
// Allows a moderator to mute or unmute all users in the channel
////////////////////////////////////////////////////////////////////////////////
template <typename StrClass, typename Glue>
bool
Vivox<StrClass, Glue>::SetUserMuteAll( const StrClass& sSessionURI, bool bMuted )
{
    if ( m_testHandsOff( "SetUserMuteAll" ) )
        return false;

    if ( !isLoggedIn() )
        return false;

    if ( !LOAD_DLL() ) return false;

    vx_req_channel_mute_all_users* p = 0;
    vx_req_channel_mute_all_users_create( &p );

    m_generateID( p->base );
    p->account_handle = vx_strdup( m_sAccountHandle.c_str() );
    p->channel_uri = vx_strdup( sSessionURI.c_str() );
    p->channel_name = vx_strdup( "" );
    p->set_muted = bMuted ? 1 : 0;

    Glue::log( VLS_DEBUG, "Sending Mute All Users request" );
    return m_issueRequest( (vx_req_base*)p );
}


//////////////////////////////////////////////////////////////////////////////////
// SetChannelMode
// 
// Allows a moderator to change the channel type (normal, presentation, lecture, etc)
// Due to a bug in the Vivox SDK, capacity and max_participants must be provided and
// they overwrite the current settings for the channel
////////////////////////////////////////////////////////////////////////////////
template <typename StrClass, typename Glue>
bool
Vivox<StrClass, Glue>::SetChannelMode( const StrClass& sSessionURI, ChannelMode eNewMode, int iCapacity, int iMaxParticipants )
{
    if ( m_testHandsOff( "SetChannelMode" ) )
        return false;

    if ( !isLoggedIn() )
        return false;

    if ( !LOAD_DLL() ) return false;

    // Verify that our values are the same as Vivox
    vivox_ct_fatal( channel_mode_none == VCM_NONE );
    vivox_ct_fatal( channel_mode_normal == VCM_NORMAL );
    vivox_ct_fatal( channel_mode_presentation == VCM_PRESENTATION );
    vivox_ct_fatal( channel_mode_lecture == VCM_LECTURE );
    vivox_ct_fatal( channel_mode_open == VCM_OPEN );

    vx_req_account_channel_update* p = 0;
    vx_req_account_channel_update_create( &p );

    m_generateID( p->base );
    p->account_handle = vx_strdup( m_sAccountHandle.c_str() );
    p->channel_uri = vx_strdup( sSessionURI.c_str() );
    p->channel_mode = (vx_channel_mode)eNewMode;
    p->capacity = iCapacity;
    p->max_participants = iMaxParticipants;
    p->set_persistent = -1;
    p->set_protected = -1;

    Glue::log( VLS_DEBUG, "Sending Channel Update request" );
    return m_issueRequest( (vx_req_base*)p );
}


//////////////////////////////////////////////////////////////////////////////////
// SetLocalMicMute
// 
// Fires off a vx_req_connector_mute_local_mic request
////////////////////////////////////////////////////////////////////////////////
template <typename StrClass, typename Glue>
bool
Vivox<StrClass, Glue>::SetLocalMicMute( bool bMute )
{
    if ( m_testHandsOff( "SetLocalMicMute" ) )
        return false;

    // NOTE: Send this request even if m_bLocalMicMute matches bMute,
    // as expected by m_processEvents()
    m_bLocalMicMute = bMute;

    if ( m_bMicMutePending )
    {
        // If we already have a mic mute request outstanding, defer until we
        // get a response
        return true;
    }

    if ( !isConnected() )
        return true;

    if ( !LOAD_DLL() ) return false;

    vx_req_connector_mute_local_mic* p = 0;
    vx_req_connector_mute_local_mic_create( &p );

    m_generateID( p->base );
    p->mute_level = bMute ? 1 : 0;
    p->connector_handle = vx_strdup( m_sConnectionHandle.c_str() );

    // Assignment intended
    Glue::log( VLS_DEBUG, "Sending Mute Local Mic request (%s)", bMute ? "mute" : "un-mute" );
    return ( m_bMicMutePending = ( m_issueRequest( (vx_req_base*)p ) ) );
}


//////////////////////////////////////////////////////////////////////////////////
// SetLocalSpeakerMute
// 
// Fires off a vx_req_connector_mute_local_speaker request
////////////////////////////////////////////////////////////////////////////////
template <typename StrClass, typename Glue>
bool
Vivox<StrClass, Glue>::SetLocalSpeakerMute( bool bMute )
{
    if ( m_testHandsOff( "SetLocalSpeakerMute" ) )
        return false;

    m_bLocalSpeakerMute = bMute;

    if ( !isConnected() )
        return true;

    if ( !LOAD_DLL() ) return false;

    vx_req_connector_mute_local_speaker* p = 0;
    vx_req_connector_mute_local_speaker_create( &p );

    m_generateID( p->base );
    p->mute_level = bMute ? 1 : 0;
    p->connector_handle = vx_strdup( m_sConnectionHandle.c_str() );

    Glue::log( VLS_DEBUG, "Sending Mute Local Speaker request" );
    return m_issueRequest( (vx_req_base*)p );
}


//////////////////////////////////////////////////////////////////////////////////
// SetLocalMicVolume
// 
// Fires off a vx_req_connector_set_local_mic_volume request
// f should be in the range 0.0 - 1.0
// This command is allowed in Hands-Off mode
////////////////////////////////////////////////////////////////////////////////
template <typename StrClass, typename Glue>
bool
Vivox<StrClass, Glue>::SetLocalMicVolume( float f )
{
    m_fLocalMicVolume = VivoxClamp( 0.0f, f, 1.0f );

    if ( !LOAD_DLL() ) return false;

    // Set up the aux levels which don't require a connector handle
    vx_req_aux_set_mic_level* pAux = 0;
    vx_req_aux_set_mic_level_create( &pAux );

    m_generateID( pAux->base );
    pAux->level = static_cast< int >( m_fLocalMicVolume * 100.0f );

    Glue::log( VLS_DEBUG, "Sending Set Mic Level request" );
    m_issueRequest( (vx_req_base*)pAux );

    if ( !isConnected() )
        return true;

    vx_req_connector_set_local_mic_volume* p = 0;
    vx_req_connector_set_local_mic_volume_create( &p );

    m_generateID( p->base );
    p->volume = static_cast< int >( m_fLocalMicVolume * 100.0f );
    p->connector_handle = vx_strdup( m_sConnectionHandle.c_str() );

    Glue::log( VLS_DEBUG, "Sending Set Local Mic Volume request" );
    return m_issueRequest( (vx_req_base*)p );
}


//////////////////////////////////////////////////////////////////////////////////
// SetLocalSpeakerVolume
// 
// Fires off a vx_req_connector_set_local_speaker_volume request
// f should be in the range 0.0 - 1.0
// This command is allowed in Hands-Off mode
//////////////////////////////////////////////////////////////////////////////////
template <typename StrClass, typename Glue>
bool
Vivox<StrClass, Glue>::SetLocalSpeakerVolume( float f )
{
    m_fLocalSpeakerVolume = VivoxClamp( 0.0f, f, 1.0f );

    if ( !LOAD_DLL() ) return false;

    // Set up the aux levels which don't require a connector handle
    vx_req_aux_set_speaker_level* pAux = 0;
    vx_req_aux_set_speaker_level_create( &pAux );

    m_generateID( pAux->base );
    pAux->level = static_cast< int >( m_fLocalSpeakerVolume * 100.0f );

    Glue::log( VLS_DEBUG, "Sending Set Speaker Level request" );
    m_issueRequest( (vx_req_base*)pAux );

    if ( !isConnected() )
        return true;

    vx_req_connector_set_local_speaker_volume* p = 0;
    vx_req_connector_set_local_speaker_volume_create( &p );

    m_generateID( p->base );
    p->volume = static_cast< int >( m_fLocalSpeakerVolume * 100.0f );
    p->connector_handle = vx_strdup( m_sConnectionHandle.c_str() );

    Glue::log( VLS_DEBUG, "Sending Set Local Speaker Volume request" );
    return m_issueRequest( (vx_req_base*)p );
}


//////////////////////////////////////////////////////////////////////////////////
// GetLocalMicMute
// 
// Returns true if the microphone is muted.
//////////////////////////////////////////////////////////////////////////////////
template <typename StrClass, typename Glue>
bool
Vivox<StrClass, Glue>::GetLocalMicMute() const
{
    return m_bLocalMicMute;
}


//////////////////////////////////////////////////////////////////////////////////
// GetLocalSpeakerMute
// 
// Returns true if the speaker is muted.
//////////////////////////////////////////////////////////////////////////////////
template <typename StrClass, typename Glue>
bool
Vivox<StrClass, Glue>::GetLocalSpeakerMute() const
{
    return m_bLocalSpeakerMute;
}


//////////////////////////////////////////////////////////////////////////////////
// GetLocalMicVolume
// 
// Returns the local mic sensitivity in the range 0.0 - 1.0
//////////////////////////////////////////////////////////////////////////////////
template <typename StrClass, typename Glue>
float
Vivox<StrClass, Glue>::GetLocalMicVolume() const
{
    return m_fLocalMicVolume;
}


//////////////////////////////////////////////////////////////////////////////////
// GetLocalSpeakerVolume
// 
// Returns the local speaker volume in the range 0.0 - 1.0
//////////////////////////////////////////////////////////////////////////////////
template <typename StrClass, typename Glue>
float
Vivox<StrClass, Glue>::GetLocalSpeakerVolume() const
{
    return m_fLocalSpeakerVolume;
}


//////////////////////////////////////////////////////////////////////////////////
// GetOutputDevices
// 
// Fires off a vx_req_aux_get_render_devices request
// The output devices will show up as a OnGetOutputDevices event
//////////////////////////////////////////////////////////////////////////////////
template <typename StrClass, typename Glue>
bool
Vivox<StrClass, Glue>::GetOutputDevices()
{
    if ( !LOAD_DLL() ) return false;

    vx_req_aux_get_render_devices* p = 0;
    vx_req_aux_get_render_devices_create( &p );

    m_generateID( p->base );

    Glue::log( VLS_DEBUG, "Sending Get Render Devices request" );
    return m_issueRequest( (vx_req_base*)p );
}


//////////////////////////////////////////////////////////////////////////////////
// SetOutputDevice
// 
// Fires off a vx_req_aux_set_render_device request
// The given name must match one of the results from OnGetOutputDevices
// This command is allowed in Hands-Off mode
//////////////////////////////////////////////////////////////////////////////////
template <typename StrClass, typename Glue>
bool
Vivox<StrClass, Glue>::SetOutputDevice( const StrClass& sDevice )
{
    if ( !LOAD_DLL() ) return false;

    vx_req_aux_set_render_device* p = 0;
    vx_req_aux_set_render_device_create( &p );

    m_generateID( p->base );
    p->render_device_specifier = vx_strdup( sDevice.c_str() );

    Glue::log( VLS_DEBUG, "Sending Set Render Device request" );
    return m_issueRequest( (vx_req_base*)p );
}


//////////////////////////////////////////////////////////////////////////////////
// GetInputDevices
// 
// Fires off a vx_req_aux_get_capture_devices request
// The input devices will show up as a OnGetInputDevices event
//////////////////////////////////////////////////////////////////////////////////
template <typename StrClass, typename Glue>
bool
Vivox<StrClass, Glue>::GetInputDevices()
{
    if ( !LOAD_DLL() ) return false;

    vx_req_aux_get_capture_devices* p = 0;
    vx_req_aux_get_capture_devices_create( &p );

    m_generateID( p->base );

    Glue::log( VLS_DEBUG, "Sending Get Capture Devices request" );
    return m_issueRequest( (vx_req_base*)p );
}


//////////////////////////////////////////////////////////////////////////////////
// SetInputDevice
// 
// Fires off a vx_req_aux_set_capture_device request
// The given name must match one of the results from OnGetInputDevices
// This command is allowed in Hands-Off mode
//////////////////////////////////////////////////////////////////////////////////
template <typename StrClass, typename Glue>
bool
Vivox<StrClass, Glue>::SetInputDevice( const StrClass& sDevice )
{
    if ( !LOAD_DLL() ) return false;

    vx_req_aux_set_capture_device* p = 0;
    vx_req_aux_set_capture_device_create( &p );

    m_generateID( p->base );
    p->capture_device_specifier = vx_strdup( sDevice.c_str() );

    Glue::log( VLS_DEBUG, "Sending Set Capture Device request" );
    return m_issueRequest( (vx_req_base*)p );
}


//////////////////////////////////////////////////////////////////////////////////
// SetSpeakerLocation
// 
// Sets the speaker (i.e. listening) location
//////////////////////////////////////////////////////////////////////////////////
template <typename StrClass, typename Glue>
void
Vivox<StrClass, Glue>::SetSpeakerLocation( const Vector& vLoc, const Vector& vForward, const Vector& vUp, const Vector& vLeft )
{
    if ( vLoc != m_curSpeakerLoc ||
         vForward != m_curSpeakerForward ||
         vUp != m_curSpeakerUp ||
         vLeft != m_curSpeakerLeft )
    {
        m_curSpeakerLoc = vLoc;
        m_curSpeakerForward = vForward;
        m_curSpeakerUp = vUp;
        m_curSpeakerLeft = vLeft;
        m_bPosDirty = true;
    }
}


//////////////////////////////////////////////////////////////////////////////////
// SetSpeakerVelocity
// 
// Sets the speaker (i.e. listening) velocity
//////////////////////////////////////////////////////////////////////////////////
template <typename StrClass, typename Glue>
void
Vivox<StrClass, Glue>::SetSpeakerVelocity( const Vector& v )
{
    if ( v != m_curSpeakerVelocity )
    {
        m_curSpeakerVelocity = v;
        m_bPosDirty = true;
    }
}


//////////////////////////////////////////////////////////////////////////////////
// SetMicLocation
// 
// Sets the microphone (i.e. speaking) location
//////////////////////////////////////////////////////////////////////////////////
template <typename StrClass, typename Glue>
void
Vivox<StrClass, Glue>::SetMicLocation( const Vector& vLoc, const Vector& vForward, const Vector& vUp, const Vector& vLeft )
{
    if ( vLoc != m_curMicLoc ||
         vForward != m_curMicForward ||
         vUp != m_curMicUp ||
         vLeft != m_curMicLeft )
    {
        m_curMicLoc = vLoc;
        m_curMicForward = vForward;
        m_curMicUp = vUp;
        m_curMicLeft = vLeft;
        m_bPosDirty = true;
    }
}


//////////////////////////////////////////////////////////////////////////////////
// SetMicVelocity
// 
// Sets the microphone (i.e. speaking) velocity
//////////////////////////////////////////////////////////////////////////////////
template <typename StrClass, typename Glue>
void
Vivox<StrClass, Glue>::SetMicVelocity( const Vector& v )
{
    if ( v != m_curMicVelocity )
    {
        m_curMicVelocity = v;
        m_bPosDirty = true;
    }
}


//////////////////////////////////////////////////////////////////////////////////
// SetPushToTalkMode
// 
// Turns on/off push-to-talk mode
// If changed while connected, the mic will be muted/unmuted immediately.
// If not connected yet, the mic will be muted/unmuted when connection succeeds
//////////////////////////////////////////////////////////////////////////////////
template <typename StrClass, typename Glue>
void
Vivox<StrClass, Glue>::SetPushToTalkMode( bool b )
{
    m_bPushToTalk = b;
    if ( isConnected() )
    {
        // We're connected, mute the mic if necessary
        SetLocalMicMute( m_bPushToTalk );
    }
}


////////////////////////////////////////////////////////////////////////////////
// ForcePushToTalk
// 
// This function provides a way for the application to simulate a push-to-talk
// key/mouse event
////////////////////////////////////////////////////////////////////////////////
template <typename StrClass, typename Glue>
void
Vivox<StrClass, Glue>::ForcePushToTalk( bool b )
{
    if ( m_bPTTForced == b )
        return;

    m_bPTTForced = b;
    m_handlePTTEvent( IsPushToTalkTalking() );
}


////////////////////////////////////////////////////////////////////////////////
// SetPushToTalkKey
// 
// Sets the key used for push-to-talk.  The key code should be
// a Windows VK_ code.  Set to zero to clear all keys.
// If bAdd is false (default), the new key becomes the only valid push-to-talk
// key to maintain API compatibility with a previous version.  If bAdd is true,
// the key code is added to a unique list of push-to-talk keys
////////////////////////////////////////////////////////////////////////////////
template <typename StrClass, typename Glue>
bool
Vivox<StrClass, Glue>::SetPushToTalkKey( int iKeyCode, bool bAdd )
{
    /*
    if ( !LOAD_DLL() ) return false;

    vx_req_aux_global_monitor_keyboard_mouse_t* p = 0;
    vx_req_aux_global_monitor_keyboard_mouse_create( &p );

    m_generateID( p->base );
    p->name = vx_strdup( "push2talk" );

    p->code_count = 0;
    if ( iKeyCode != 0 )
    {
        p->codes[ p->code_count++ ] = iKeyCode;

        // Reset the mic if we change the keycode
        SetPushToTalkMode( m_bPushToTalk );
    }
    else
    {
        // Turn off push to talk if the key is unset
        SetPushToTalkMode( false );
    }

    return 0 == vx_issue_request( (vx_req_base*)p );
    */

    if ( iKeyCode != 0 )
    {
        if ( bAdd )
        {
            // Adding to list, keep existing Ensure unique
            RemovePushToTalkKey( iKeyCode, false );
            m_aPTTKeys.push_back( iKeyCode );
        }
        else
        {
            m_aPTTKeys.clear();
            m_aPTTKeys.push_back( iKeyCode );
        }
        SetPushToTalkMode( m_bPushToTalk );
    }
    else
    {
        // Turn off push-to-talk if the key is unset
        // ClearPushToTalkKeys(); (or not...)
    }

    return true;
}


////////////////////////////////////////////////////////////////////////////////
// RemovePushToTalkKey
// 
// Removes a key previously set with SetPushToTalkKey from the list of unique
// push-to-talk keys.
////////////////////////////////////////////////////////////////////////////////
template <typename StrClass, typename Glue>
bool
Vivox<StrClass, Glue>::RemovePushToTalkKey( int iKeyCode, bool bDisableOnEmpty /*=true*/ )
{
    std::vector< int >::iterator iter = std::find( m_aPTTKeys.begin(), m_aPTTKeys.end(), iKeyCode );
    if ( iter != m_aPTTKeys.end() )
    {
        m_aPTTKeys.erase( iter );

        if ( bDisableOnEmpty && m_aPTTKeys.empty() )
            ClearPushToTalkKeys();

        return true;
    }
    return false;
}


////////////////////////////////////////////////////////////////////////////////
// ClearPushToTalkKeys
// 
// Removes all keys previously set with SetPushToTalkKey, but leaves push-to-
// talk mode intact (effectively disabling the mic)
////////////////////////////////////////////////////////////////////////////////
template <typename StrClass, typename Glue>
void
Vivox<StrClass, Glue>::ClearPushToTalkKeys()
{
    std::vector< int >().swap( m_aPTTKeys );
    // SetPushToTalkMode( false );
}


////////////////////////////////////////////////////////////////////////////////
// SetTransmitSession
// 
// Sets the session that the user will speak in
////////////////////////////////////////////////////////////////////////////////
template <typename StrClass, typename Glue>
bool
Vivox<StrClass, Glue>::SetTransmitSession( const StrClass& sSession )
{
    if ( m_testHandsOff( "SetTransmitSession" ) )
        return false;

    const Session* pSession = FindSession( sSession );
    if ( 0 == pSession )
        return false;

    if ( !LOAD_DLL() ) return false;

    m_sTransmitSessionGroup = pSession->sGroupHandle;
    m_sTransmitSession = pSession->sHandle;

    vx_req_sessiongroup_set_tx_session* p = 0;
    vx_req_sessiongroup_set_tx_session_create( &p );

    m_generateID( p->base );
    p->session_handle = vx_strdup( m_sTransmitSession.c_str() );
    p->sessiongroup_handle = vx_strdup( m_sTransmitSessionGroup.c_str() );

    Glue::log( VLS_DEBUG, "Sending Set TX Session request" );
    return m_issueRequest( (vx_req_base*)p );
}


////////////////////////////////////////////////////////////////////////////////
// SetTransmitSession
// 
// Sets the session that the user will speak in
////////////////////////////////////////////////////////////////////////////////
template <typename StrClass, typename Glue>
typename const Vivox<StrClass, Glue>::Session*
Vivox<StrClass, Glue>::GetTransmitSession() const
{
    return FindSession( m_sTransmitSession );
}


////////////////////////////////////////////////////////////////////////////////
// BeginVUMonitor
// 
// Starts monitoring volume units from the mic.  Sends a
// vx_req_aux_capture_audio_start request.  This will cause VivoxVUMonitor
// events to fire
////////////////////////////////////////////////////////////////////////////////
template <typename StrClass, typename Glue>
bool
Vivox<StrClass, Glue>::BeginVUMonitor()
{
    if ( m_testHandsOff( "BeginVUMonitor" ) )
        return false;

    // If another test is already active, return
    if ( m_testType != TT_NONE )
        return false;

    vivox_db_fatal( m_testState == TS_NOT_RUNNING );

    m_testType = TT_VU_MONITOR;
    m_testState = TS_SUSPENDING;

    // Close all sessions
    m_suspendSessions( true );

    m_bNeedStop = false;
    m_uEchoTestLength = 0;

    // All sessions must be fully terminated to active the VU Monitor
    if ( m_mSessions.empty() && m_allSessionsSuspended() )
        return m_beginVUMonitor();

    return true;
}

template <typename StrClass, typename Glue>
bool
Vivox<StrClass, Glue>::m_beginVUMonitor()
{
    vivox_db_fatal( TT_VU_MONITOR == m_testType );
    vivox_db_fatal( m_testState == TS_SUSPENDING );

    if ( m_bNeedStop )
    {
        // Simulate event firing
        m_testState = TS_EVENTS;
        FIRE_EVENT(VivoxOnVUMonitorStart,());
        FIRE_EVENT(VivoxOnVUMonitorEnd,());

        m_testType = TT_NONE;
        m_testState = TS_NOT_RUNNING;
        m_unsuspendSessions( true );
        return false;
    }

    if ( !LOAD_DLL() ) return false;

    vx_req_aux_capture_audio_start* p = 0;
    vx_req_aux_capture_audio_start_create( &p );

    m_generateID( p->base /*, false*/ );

    Glue::log( VLS_DEBUG, "Sending Capture Audio Start request" );
    if ( !m_issueRequest( (vx_req_base*)p ) )
    {
        // failed; need to re-instate sessions
        vivox_db_fatal( 0 );
        m_testType = TT_NONE;
        m_testState = TS_NOT_RUNNING;
        m_unsuspendSessions( true );
        return false;
    }

    m_testState = TS_STARTING;

    return true;
}


////////////////////////////////////////////////////////////////////////////////
// EndVUMonitor
// 
// Stops monitoring volume units from the mic.  Sends a
// vx_req_aux_capture_audio_stop request
////////////////////////////////////////////////////////////////////////////////
template <typename StrClass, typename Glue>
bool
Vivox<StrClass, Glue>::EndVUMonitor()
{
    if ( m_testType != TT_VU_MONITOR && m_testType != TT_ECHO_RECORD )
        return false;

    if ( m_testState != TS_RUNNING )
    {
        // Hasn't been fully started yet.
        m_bNeedStop = true;
        return true;
    }

    if ( m_stopVUMonitor() )
    {
        // Record echo test time if necessary
        if ( TT_ECHO_RECORD == m_testType  )
            m_uEchoTestLength = m_uEchoTestStartTime != 0 ? Glue::getTimeMS() - m_uEchoTestStartTime : 0;

        return true;
    }

    return false;
}


template <typename StrClass, typename Glue>
bool
Vivox<StrClass, Glue>::m_stopVUMonitor()
{
    vivox_db_fatal( TT_VU_MONITOR == m_testType || TT_ECHO_RECORD == m_testType );
    vivox_db_fatal( TS_RUNNING == m_testState );

    if ( !LOAD_DLL() ) return false;

    vx_req_aux_capture_audio_stop* p = 0;
    vx_req_aux_capture_audio_stop_create( &p );

    m_generateID( p->base /*, false*/ );

    Glue::log( VLS_DEBUG, "Sending Capture Audio Stop request" );
    if ( m_issueRequest( (vx_req_base*)p ) )
    {
        // Return mute to previous state
        SetLocalMicMute( m_bPreTestMute );

        m_testState = TS_STOPPING;
        return true;
    }

    // Shouldn't get here
    vivox_db_fatal( 0 );

    return false;
}



////////////////////////////////////////////////////////////////////////////////
// BeginEchoTest
// 
// Begins recording a short (10sec) sequence for playback
////////////////////////////////////////////////////////////////////////////////
template <typename StrClass, typename Glue>
bool
Vivox<StrClass, Glue>::BeginEchoTest()
{
    if ( m_testHandsOff( "BeginEchoTest" ) )
        return false;

    if ( m_testType != TT_NONE )
        return false;

    vivox_db_fatal( m_testState == TS_NOT_RUNNING );

    m_testType = TT_ECHO_RECORD;
    m_testState = TS_SUSPENDING;

    // Close all sessions
    m_suspendSessions( true );

    m_bNeedStop = false;
    m_uEchoTestStartTime = 0;

    if ( m_mSessions.empty() && m_allSessionsSuspended() )
        return m_beginEchoTest();

    return true;
}

template <typename StrClass, typename Glue>
bool
Vivox<StrClass, Glue>::m_beginEchoTest()
{
    vivox_db_fatal( TT_ECHO_RECORD == m_testType );
    vivox_db_fatal( TS_SUSPENDING == m_testState );

    if ( m_bNeedStop )
    {
        // Simulate event firing
        m_testState = TS_EVENTS;
        m_uEchoTestLength = 0;
        FIRE_EVENT(VivoxOnEchoTestStart,());
        FIRE_EVENT(VivoxOnEchoTestEnd,(m_uEchoTestLength));

        m_testType = TT_NONE;
        m_testState = TS_NOT_RUNNING;
        m_unsuspendSessions( true );
        return false;
    }

    if ( !LOAD_DLL() ) return false;

    vx_req_aux_start_buffer_capture* p;
    vx_req_aux_start_buffer_capture_create( &p );

    m_generateID( p->base /*, false*/ );

    Glue::log( VLS_DEBUG, "Sending Start Buffer Capture request" );
    if ( !m_issueRequest( (vx_req_base*)p ) )
    {
        // failed; need to re-instate sessions
        vivox_db_fatal( 0 );
        m_testType = TT_NONE;
        m_testState = TS_NOT_RUNNING;
        m_unsuspendSessions( true );
        return false;
    }

    m_testState = TS_STARTING;

    return true;
}


////////////////////////////////////////////////////////////////////////////////
// StopEchoTest
// 
// Stops echo test recording
////////////////////////////////////////////////////////////////////////////////
template <typename StrClass, typename Glue>
bool
Vivox<StrClass, Glue>::StopEchoTest()
{
    // EndVUMonitor sends the same message...
    if ( TT_ECHO_PLAYBACK == m_testType )
    {
        if ( m_testState != TS_RUNNING )
        {
            // Hasn't been fully started yet
            m_bNeedStop = true;
            return true;
        }

        return m_stopEchoPlayback();
    }

    return EndVUMonitor();
}


template <typename StrClass, typename Glue>
bool
Vivox<StrClass, Glue>::m_stopEchoPlayback()
{
    vivox_db_fatal( TT_ECHO_PLAYBACK == m_testType );
    vivox_db_fatal( TS_RUNNING == m_testState );

    if ( !LOAD_DLL() ) return false;

    vx_req_aux_render_audio_stop* p = 0;
    vx_req_aux_render_audio_stop_create( &p );

    m_generateID( p->base /*, false*/ );

    Glue::log( VLS_DEBUG, "Sending Render Audio Stop request" );
    if ( m_issueRequest( (vx_req_base*)p ) )
    {
        m_testState = TS_STOPPING;
        return true;
    }

    // Shouldn't get here
    vivox_db_fatal( 0 );

    return false;
}


////////////////////////////////////////////////////////////////////////////////
// PlayEchoTest
// 
// Plays back a previously recorded echo test (See BeginEchoTest)
////////////////////////////////////////////////////////////////////////////////
template <typename StrClass, typename Glue>
bool
Vivox<StrClass, Glue>::PlayEchoTest()
{
    if ( m_testHandsOff( "PlayEchoTest" ) )
        return false;

    if ( m_testType != TT_NONE || 0 == m_uEchoTestLength )
        return false;

    vivox_db_fatal( m_testState == TS_NOT_RUNNING );

    m_testType = TT_ECHO_PLAYBACK;
    m_testState = TS_SUSPENDING;

    // Close all sessions
    m_suspendSessions( true );

    m_bNeedStop = false;
    m_uEchoTestStartTime = 0;

    if ( m_mSessions.empty() && m_allSessionsSuspended() )
        return m_playEchoTest();

    return true;
}

template <typename StrClass, typename Glue>
bool
Vivox<StrClass, Glue>::m_playEchoTest()
{
    vivox_db_fatal( TT_ECHO_PLAYBACK == m_testType );
    vivox_db_fatal( TS_SUSPENDING == m_testState );

    if ( m_bNeedStop )
    {
        // Simulate event firing
        m_testState = TS_EVENTS;
        FIRE_EVENT(VivoxOnEchoTestPlaybackStart,(m_uEchoTestLength));
        FIRE_EVENT(VivoxOnEchoTestPlaybackEnd,(m_uEchoTestLength));

        m_testType = TT_NONE;
        m_testState = TS_NOT_RUNNING;
        m_unsuspendSessions( true );

        return false;
    }

    if ( !LOAD_DLL() ) return false;

    vx_req_aux_play_audio_buffer* p;
    vx_req_aux_play_audio_buffer_create( &p );

    m_generateID( p->base /*, false*/ );

    Glue::log( VLS_DEBUG, "Sending Play Audio Buffer request" );
    if ( !m_issueRequest( (vx_req_base*)p ) )
    {
        // failed; need to re-instate sessions
        vivox_db_fatal( 0 );
        m_testType = TT_NONE;
        m_testState = TS_NOT_RUNNING;
        m_unsuspendSessions( true );

        return false;
    }

    m_testState = TS_STARTING;

    return true;
}


////////////////////////////////////////////////////////////////////////////////
// SuspendSessions
// 
// Suspends all current and new sessions until UnsuspendSessions() is called.
// This is counter-based, so UnsuspendSessions() must be called as many times
// as SuspendSessions() is called.  The Vivox testing functions (BeginVUMonitor(),
// BeginEchoTest(), etc) also suspend sessions.
////////////////////////////////////////////////////////////////////////////////
template <typename StrClass, typename Glue>
void
Vivox<StrClass, Glue>::SuspendSessions()
{
    m_suspendSessions( false );
}


////////////////////////////////////////////////////////////////////////////////
// UnsuspendSessions
// 
// Unsuspends sessions if currently suspended.  This is counter-based, so
// UnsuspendSessions() must be called as many times as SuspendSessions() is called.
// If a Vivox test is currently in progress (BeginVUMonitor(), BeginEchoTest(),
// PlayEchoTest()), this function will not unsuspend sessions.  Combined with
// SuspendSessions(), this function allows an application to extend the time
// that sessions are suspended.
// 
// Returns false if calling the function had no effect.
////////////////////////////////////////////////////////////////////////////////
template <typename StrClass, typename Glue>
bool
Vivox<StrClass, Glue>::UnsuspendSessions()
{
    // Logic for return value:
    // Always return true if counter is two or more.  If counter is one, only
    // return true if the internal suspended flag is false.  This will allow us
    // to loop calling this function to remove all external suspensions.
    // Do it before calling the function so that we still return 'true' if
    // we actually changed something.
    bool bReturn = m_iSessionsSuspended > 1 || ( 1 == m_iSessionsSuspended && !m_bSessionsSuspendedInternal );

    m_unsuspendSessions( false );

    return bReturn;
}


////////////////////////////////////////////////////////////////////////////////
// RequestDiagnosticStateDump
// 
// Requests a diagnostic state dump from the Vivox SDK.  Available only with v3
// or later.  Fires a VivoxOnBeginDiagnosticStateDump immediately, followed by
// a VivoxDiagnosticStateDump event for each connector, followed by a
// VivoxOnEndDiagnosticStateDump event when completed.  Returns false if the
// request could not be issued to the SDK.
////////////////////////////////////////////////////////////////////////////////
template <typename StrClass, typename Glue>
bool
Vivox<StrClass, Glue>::RequestDiagnosticStateDump( DumpLevel level )
{
#if VIVOX_VERSION >= 3
    // Verify expected enum values
    vivox_ct_fatal( DL_ALL == diagnostic_dump_level_all );
    vivox_ct_fatal( DL_SESSIONS == diagnostic_dump_level_sessions );

    if ( !LOAD_DLL() ) return false;

    vx_req_aux_diagnostic_state_dump* p = 0;
    vx_req_aux_diagnostic_state_dump_create( &p );

    m_generateID( p->base );

    p->level = (vx_diagnostic_dump_level)level;

    Glue::log( VLS_DEBUG, "Sending Diagnostic State Dump request" );
    return m_issueRequest( (vx_req_base_t*)p );
#else
    return false;
#endif
}


////////////////////////////////////////////////////////////////////////////////
// m_allSessionsSuspended
// 
// Returns true if all SessionInfos currently managed are marked as suspended
////////////////////////////////////////////////////////////////////////////////
template <typename StrClass, typename Glue>
bool
Vivox<StrClass, Glue>::m_allSessionsSuspended() const
{
    for ( SessionInfoMap::const_iterator iter( m_mSessionInfo.begin() );
          iter != m_mSessionInfo.end();
          ++iter )
    {
        const SessionInfo& info = (*iter).second;
        if ( !info.bSuspended )
            return false;
    }

    return true;
}


////////////////////////////////////////////////////////////////////////////////
// m_generateID
// 
// Generates the next ID
////////////////////////////////////////////////////////////////////////////////
template <typename StrClass, typename Glue>
void
Vivox<StrClass, Glue>::m_generateID( vx_req_base& base, bool bUseSDKHandle /*=true*/ )
{
    char buf[256];
    ::_snprintf( buf, sizeof(buf), "%u%s", m_uID++, Glue::getGameName() );
    buf[ sizeof(buf) - 1 ] = '\0';
    base.cookie = vx_strdup( buf );
#if VIVOX_VERSION >= 3
    if ( m_sdkHandle != 0 && bUseSDKHandle )
    {
        base.message.sdk_handle = m_sdkHandle;
    }
#endif
}


////////////////////////////////////////////////////////////////////////////////
// m_issueRequest
// 
// Issues a request to the Vivox SDK
////////////////////////////////////////////////////////////////////////////////
template <typename StrClass, typename Glue>
bool
Vivox<StrClass, Glue>::m_issueRequest( vx_req_base* pv )
{
    vx_req_base* p = reinterpret_cast< vx_req_base* >( pv );

    if ( 0 == vx_issue_request( p ) )
    {
        m_pendingRequests.insert( p->cookie );
        return true;
    }

    return false;
}


////////////////////////////////////////////////////////////////////////////////
// m_sendLogoutRequest
// 
// Sends the logout request to the Vivox SDK
////////////////////////////////////////////////////////////////////////////////
template <typename StrClass, typename Glue>
bool
Vivox<StrClass, Glue>::m_sendLogoutRequest()
{
    vivox_db_fatal( m_eLoginState == LS_LOGGED_IN || m_eLoginState == LS_WAITING_FOR_REQUESTS );
    vivox_db_fatal( m_pendingRequests.empty() );

    if ( !LOAD_DLL() ) return false;

    vx_req_account_logout* p = 0;
    vx_req_account_logout_create( &p );

    p->account_handle = vx_strdup( m_sAccountHandle.c_str() );
    m_generateID( p->base );

    Glue::log( VLS_DEBUG, "Sending Logout request %s", m_sAccountHandle.c_str() );
    if ( m_issueRequest( (vx_req_base*)p ) )
    {
        m_eLoginState = LS_LOGGING_OUT;
        return true;
    }

    // Shouldn't get here
    vivox_db_fatal( 0 );

    return false;
}

////////////////////////////////////////////////////////////////////////////////
// m_parseAcctNameFromURI
// 
// Returns the <name> portion of [sip:]<name>@<game>.vivox.com
// Returns an empty string if account name was not found.
////////////////////////////////////////////////////////////////////////////////
template <typename StrClass, typename Glue>
StrClass
Vivox<StrClass, Glue>::m_parseAcctNameFromURI( const char* pURI )
{
    const char* begin = strchr( pURI, ':' );
    if ( begin )
        ++begin; // skip ':'
    else
        begin = pURI;

    const char* end = strchr( begin, '@' );
    if ( end )
        return StrClass( begin, end - begin );
    else
        return StrClass();
}


////////////////////////////////////////////////////////////////////////////////
// m_tolower
// 
// Converts a string to lowercase
////////////////////////////////////////////////////////////////////////////////
template <typename StrClass, typename Glue>
void
Vivox<StrClass, Glue>::m_tolower( StrClass& s )
{
    for ( char* p = &s[ 0 ]; *p; ++p )
        *p = tolower( *p );
}

#if !defined(VIVOX_VERSION) || VIVOX_VERSION < 3

////////////////////////////////////////////////////////////////////////////////
// m_onDllLoaded
// 
// Fired to notify the Vivox wrapper to set up initial state as soon as the DLL
// is loaded the first time
////////////////////////////////////////////////////////////////////////////////
template <typename StrClass, typename Glue>
struct DllLoadedEventHandler : public Vivox<StrClass, Glue>::VivoxEventHandler
{
    int& iRequests;
    DllLoadedEventHandler( int& reqs ) : iRequests( reqs ) {}

    void VivoxOnGetInputDevices( const StrClass& sCurrent, const std::vector< StrClass >& aDevices )
    {
        --iRequests;
    }
    void VivoxOnGetOutputDevices( const StrClass& sCurrent, const std::vector< StrClass >& aDevices )
    {
        --iRequests;
    }

private:
	DllLoadedEventHandler & operator=(DllLoadedEventHandler const &);
};

#endif

/*static*/
template <typename StrClass, typename Glue>
void
Vivox<StrClass, Glue>::m_onDllLoaded()
{
    vivox_db_fatal( isInstalled() );

    Vivox& v = getInstance();

#if !defined(VIVOX_VERSION) || VIVOX_VERSION < 3
    int iRequests = 0;

    // Save off the handlers so that we don't fire events during setup
    EventHandlers old;
    old.swap( v.m_aHandlers );

    // Simple event handler that just decrements the number of outstanding events
    DllLoadedEventHandler<StrClass, Glue> myEventHandler( iRequests );

    // Set up the default devices
    if ( v.m_sDefaultInputDevice.empty() )
    {
        v.GetInputDevices();
        ++iRequests;
    }
    if ( v.m_sDefaultOutputDevice.empty() )
    {
        v.GetOutputDevices();
        ++iRequests;
    }

    // Process events until they're all done (or a 5 second timeout is hit)
    int iCount = 0;
    while ( myEventHandler.iRequests > 0 && iCount++ < 500 )
    {
        v.ProcessEvents();
        if ( myEventHandler.iRequests > 0 )
        {
            Glue::sleep( 10 );
        }
    }

    // Restore the handlers
    old.swap( v.m_aHandlers );
#else
    // Version 3 uses a special string name
    v.m_sDefaultInputDevice = v.m_sDefaultOutputDevice = DEFAULT_DEVICE;
#endif
}

//////////////////////////////////////////////////////////////////////////////
// m_testHandsOff()
// 
// Returns the same value as IsHandsOff() but will generate a debug log if
// an event is provided
////////////////////////////////////////////////////////////////////////////////
template <typename StrClass, typename Glue>
bool
Vivox<StrClass, Glue>::m_testHandsOff( const char* event )
{
    if ( IsHandsOff() )
    {
        Glue::log( VLS_DEBUG, "%s while hands-off mode active", event );
        return true;
    }

    return false;
}

//////////////////////////////////////////////////////////////////////////////
// m_fireModEvent()
// 
// Fires the moderator event given the provided parameters
////////////////////////////////////////////////////////////////////////////////
template <typename StrClass, typename Glue>
void
Vivox<StrClass, Glue>::m_fireModEvent( ModCommandType type, int iCode, const char* pStatus, const char* pSessionURI, const char* pTargetURI )
{
    StrClass sStatus( pStatus ? pStatus : "" );
    StrClass sSessionURI( pSessionURI ? pSessionURI : "" );
    StrClass sTargetURI( pTargetURI ? pTargetURI : "" );

    // Lowercase the target URI
    m_tolower( sTargetURI );

    FIRE_EVENT(VivoxOnModeratorResponse,(type, sSessionURI, sTargetURI, iCode, sStatus));
}

//////////////////////////////////////////////////////////////////////////////
// m_fireParticipantUpdated()
// 
// Fires the moderator event given the provided parameters
////////////////////////////////////////////////////////////////////////////////
template <typename StrClass, typename Glue>
void
Vivox<StrClass, Glue>::m_fireParticipantUpdated( const StrClass& sGroup, const StrClass& sSession, const Participant& oldP, const Participant& newP )
{
    FIRE_EVENT(VivoxOnParticipantUpdated,(sGroup, sSession, oldP, newP));
}

//////////////////////////////////////////////////////////////////////////////
// m_sendPositionUpdate()
// 
// Sends a mic/speaker position update to the Vivox system
////////////////////////////////////////////////////////////////////////////////
template <typename StrClass, typename Glue>
bool
Vivox<StrClass, Glue>::m_sendPositionUpdate()
{
    if ( !isLoggedIn() || m_sPositionSession.empty() || m_sPositionSessionGroup.empty() )
    {
        return false;
    }

    if ( !LOAD_DLL() ) return false;

    vx_req_session_set_3d_position* p = 0;
    vx_req_session_set_3d_position_create( &p );

    p->session_handle = vx_strdup( m_sPositionSession.c_str() );
    // These are in the docs but apparently not in the struct
    //p->sessiongroup_handle = vx_strdup( m_sPositionSessionGroup.c_str() );
    //p->cookie = vx_strdup( m_generateID().c_str() );

    #define SET_VECTOR( __req, __vec ) \
    (__req)[0] = __vec.x; \
    (__req)[1] = __vec.y; \
    (__req)[2] = __vec.z

    SET_VECTOR( p->speaker_position, m_curMicLoc );
    SET_VECTOR( p->speaker_at_orientation, m_curMicForward );
    SET_VECTOR( p->speaker_up_orientation, m_curMicUp );
    SET_VECTOR( p->speaker_left_orientation, m_curMicLeft );
    SET_VECTOR( p->speaker_velocity, m_curMicVelocity );

    SET_VECTOR( p->listener_position, m_curSpeakerLoc );
    SET_VECTOR( p->listener_at_orientation, m_curSpeakerForward );
    SET_VECTOR( p->listener_up_orientation, m_curSpeakerUp );
    SET_VECTOR( p->listener_left_orientation, m_curSpeakerLeft );
    SET_VECTOR( p->listener_velocity, m_curSpeakerVelocity );

    #undef SET_VECTOR

    Glue::log( VLS_DEBUG, "Sending Set 3D Position request" );
    return m_issueRequest( (vx_req_base*)p );
}


//////////////////////////////////////////////////////////////////////////////
// m_handlePTTEvent()
// 
// Turns on/off the mic depending on push-to-talk states
////////////////////////////////////////////////////////////////////////////////
template <typename StrClass, typename Glue>
void
Vivox<StrClass, Glue>::m_handlePTTEvent( const bool kbIsTalking )
{
    // If we're not doing push-to-talk, ignore it
    if ( !m_bPushToTalk || m_bPreventPTT || m_testHandsOff( "m_handlePTTEvent" ) )
        return;

    // If we don't have any connected sessions, bail
    // (unless trying to mute)
    if ( m_mSessions.empty() && kbIsTalking )
        return;

    // Turn on/off the mic and fire events as necessary
    if ( kbIsTalking == m_bLocalMicMute )
    {
        if ( !m_mSessions.empty() )
        {
            const StrClass& sSession = m_sTransmitSession.empty() ? (*m_mSessions.begin()).first : m_sTransmitSession;
            m_sendPTTNotification( sSession, kbIsTalking );
        }

        SetLocalMicMute( !kbIsTalking );
        FIRE_EVENT( VivoxOnPushToTalk, (kbIsTalking) );
    }
}


//////////////////////////////////////////////////////////////////////////////
// m_sendPTTNotification()
// 
// Sends push-to-talk notification to other participants and/or self as directed
////////////////////////////////////////////////////////////////////////////////
template <typename StrClass, typename Glue>
void
Vivox<StrClass, Glue>::m_sendPTTNotification( const StrClass& sSession, const bool kbIsTalking )
{
    vx_notification_type notification_type = kbIsTalking ? notification_hand_raised : notification_hand_lowered;

    // Defer if we already have an outstanding notification pending
    if ( !m_bNotificationPending )
    {    
        // Send a hand raised/lowered event based on whether we're talking or not
        vx_req_session_send_notification* req = 0;
        vx_req_session_send_notification_create( &req );
    
        m_generateID( req->base );
        req->session_handle = vx_strdup( sSession.c_str() );
        req->notification_type = notification_type;

        Glue::log( VLS_DEBUG, "Sending Send Notification request" );
        m_bNotificationPending = m_issueRequest( (vx_req_base*)req );
    }

    // This doesn't happen to be received locally, so fake an event to the self participant in the session
    if ( Session* pSession = m_findSession( sSession ) )
    {
        if ( pSession->pSelf )
        {
            StrClass sURI( pSession->pSelf->sURI );

            vx_evt_session_notification evt;
            evt.notification_type = notification_type;
            evt.session_handle = (char*)sSession.c_str();
            evt.participant_uri = (char*)sURI.c_str();
            m_processSessionNotification( evt );
        }
    }
}


//////////////////////////////////////////////////////////////////////////////
// m_processNextDeferred()
// 
// Handles processing the next deferred item.  This will pop the now-finished
// currently-being-processed deferred item (i.e. the one at the front)
//////////////////////////////////////////////////////////////////////////////
template <typename StrClass, typename Glue>
bool
Vivox<StrClass, Glue>::m_processNextDeferred( GroupInfo& group, bool bPopFront )
{
    vivox_db_fatal( !group.sHandle.empty() );
    // vivox_db_fatal( !group.vDeferred.empty() );

    if ( group.vDeferred.empty() )
        return false;

    if ( bPopFront )
    {
        group.vDeferred.pop_front();
    
        // Done with all deferred items
        if ( group.vDeferred.empty() )
            return false;
    }

    DeferredSessionInfo& info = group.vDeferred.front();
    if ( info.bForeign )
    {
        // If the next item is a foreign item (added as a result of some other application
        // doing something) then don't do anything.  Wait for the response to finish.
        return false;
    }

    info.bBeginProcessing = true;
    bool b = info.bTerminate ?
             TerminateSession( info.sURI ) :
             BeginSession( info.sURI, info.sDisplayName, info.sPassword, group.sHandle );

    // If it failed, recurse until we get a valid one
    return b ? b : m_processNextDeferred( group, true );
}


//////////////////////////////////////////////////////////////////////////////
// m_removeDeferred()
// 
// Searches for and removes the specified item.  Returns true if found
//////////////////////////////////////////////////////////////////////////////
template <typename StrClass, typename Glue>
bool
Vivox<StrClass, Glue>::m_removeDeferred( GroupInfo& group, const StrClass& sURI, bool bTerminate )
{
    vivox_db_fatal( !group.sHandle.empty() );

    for ( DeferredSessionList::iterator iter( group.vDeferred.begin() );
          iter != group.vDeferred.end();
          ++iter )
    {
        DeferredSessionInfo& info = *iter;

        if ( info.bTerminate == bTerminate &&
             info.sURI       == sURI )
        {
            group.vDeferred.erase( iter );
            return true;
        }
    }

    return false;
}


//////////////////////////////////////////////////////////////////////////////
// m_suspendSessions()
// 
// Suspends sessions by setting the 'sessions suspended' flag and terminating
// existing sessions.  Existing sessions may then be re-established by using
// m_unsuspendSessions.
//////////////////////////////////////////////////////////////////////////////
template <typename StrClass, typename Glue>
void
Vivox<StrClass, Glue>::m_suspendSessions( bool bInternal )
{
    if ( bInternal )
    {
        vivox_db_fatal( !m_bSessionsSuspendedInternal );
        m_bSessionsSuspendedInternal = true;
    }

    // Return if we've already suspended the sessions
    if ( m_iSessionsSuspended++ > 0 )
        return;

    for ( SessionMap::const_iterator iter( m_mSessions.begin() );
          iter != m_mSessions.end();
          ++iter )
    {
        const Session& ssn = (*iter).second;

        // Find the session in the session info map and mark as suspended
        SessionInfoMap::iterator infoIter( m_mSessionInfo.find( ssn.sURI ) );
        if ( infoIter != m_mSessionInfo.end() )
        {
            SessionInfo& info = (*infoIter).second;
            info.bSuspended = true;
        }

        TerminateSession( ssn.sHandle );
    }

    // Clear out all of the deferred sessions so that they don't try to connect
    for ( GroupInfoMap::iterator iter( m_mGroupInfo.begin() );
          iter != m_mGroupInfo.end();
          ++iter )
    {
        GroupInfo& info = (*iter).second;

        for ( DeferredSessionList::iterator defIter( info.vDeferred.begin() );
              defIter != info.vDeferred.end();
              ++defIter )
        {
            DeferredSessionInfo& deferred = (*defIter);

            // Find the session in the session info map and mark as suspended
            SessionInfoMap::iterator infoIter( m_mSessionInfo.find( deferred.sURI ) );
            if ( infoIter != m_mSessionInfo.end() )
            {
                SessionInfo& sessionInfo = (*infoIter).second;

                // Only set the suspended flag for connecting deferred sessions.  This
                // will cause pending terminates to just stay terminated.
                if ( !deferred.bTerminate )
                    sessionInfo.bSuspended = true;
            }
        }

        info.vDeferred.clear();
    }
}


//////////////////////////////////////////////////////////////////////////////
// m_unsuspendSessions()
// 
// Unsuspends sessions (if suspended) and sets the suspended flag to false
////////////////////////////////////////////////////////////////////////////////
template <typename StrClass, typename Glue>
void
Vivox<StrClass, Glue>::m_unsuspendSessions( bool bInternal )
{
    if ( bInternal )
    {
        vivox_db_fatal( m_bSessionsSuspendedInternal && m_iSessionsSuspended > 0 );
        m_bSessionsSuspendedInternal = false;
    }
    else
    {
        // External unsuspend request-- only allow if we have suspend count
        // and the internal suspension isn't holding a lock
        if ( m_iSessionsSuspended <= 0 ||
             ( 1 == m_iSessionsSuspended && m_bSessionsSuspendedInternal ) )
            return;
    }

    if ( 0 == --m_iSessionsSuspended )
    {
        // Swap the map out since BeginSession() adds things to the map
        // This also resets the bSuspended flag for each session
        SessionInfoMap siMap; siMap.swap( m_mSessionInfo );
        for ( SessionInfoMap::const_iterator iter( siMap.begin() );
              iter != siMap.end();
              ++iter )
        {
            const SessionInfo& info = (*iter).second;
            BeginSession( info.sURI, info.sDisplayName, info.sPassword, info.sGroup );
        }
    }
}


//////////////////////////////////////////////////////////////////////////////
// m_clearSessions()
// 
// Clears out all sessions firing events as necessary
////////////////////////////////////////////////////////////////////////////////
template <typename StrClass, typename Glue>
void
Vivox<StrClass, Glue>::m_clearSessions( bool bFireSessionGroupEvents )
{
    // Swap out the session maps and fire events notifying the app that they're gone
    SessionMap temp; temp.swap( m_mSessions );
    for ( SessionMap::const_iterator iter( temp.begin() );
          iter != temp.end();
          ++iter )
    {
        const Session& s = (*iter).second;

        // Fire participant events
        for ( Session::ParticipantMap::const_iterator pIter( s.mParticipants.begin() );
              pIter != s.mParticipants.end();
              ++pIter )
        {
            const Participant& p = (*pIter).second;

            FIRE_EVENT(VivoxOnParticipantRemoved,(s.sGroupHandle, s.sHandle, p, RR_UNKNOWN));
        }

        m_mSessionInfo.erase( s.sURI );
        FIRE_EVENT(VivoxOnSessionRemoved,(s.sGroupHandle, s.sHandle, s));
    }

    m_mTextMessages.clear();

    vivox_db_fatal(m_mSessions.empty());

    // Now fire the session group events.  The SDK normally sends the proper events to clean
    // these up, but if the voice service disconnects unexpectedly the events never arrive.
    if ( bFireSessionGroupEvents )
    {
        // Make sure all groups are represented
        for ( GroupInfoMap::const_iterator iter( m_mGroupInfo.begin() );
              iter != m_mGroupInfo.end();
              ++iter )
        {
            FIRE_EVENT(VivoxOnSessionGroupRemoved,((*iter).first));
        }
    }
    else
    {
        vivox_db_fatal(m_mGroupInfo.empty());
    }

    m_mGroupInfo.clear();

    // No more sessions, so clear the positional session
    SetPositionalSession( StrClass() );
}


//////////////////////////////////////////////////////////////////////////////
// m_getRespTypeName()
// 
// Returns the name of a response type (Debug only)
////////////////////////////////////////////////////////////////////////////////
template <typename StrClass, typename Glue>
const char*
Vivox<StrClass, Glue>::m_getRespTypeName( vx_response_type t ) const
{
    const char* str = "[unknown]";

#ifdef _DEBUG
    #define RESPTYPE(__type) case __type: str = #__type; break
    switch ( t )
    {
    RESPTYPE(resp_none);
    RESPTYPE(resp_connector_create);
    RESPTYPE(resp_connector_initiate_shutdown);
    RESPTYPE(resp_account_login);
    RESPTYPE(resp_account_logout);
    RESPTYPE(resp_account_set_login_properties);
    RESPTYPE(resp_sessiongroup_create);
    RESPTYPE(resp_sessiongroup_terminate);
    RESPTYPE(resp_sessiongroup_add_session);
    RESPTYPE(resp_sessiongroup_remove_session);
    RESPTYPE(resp_sessiongroup_set_focus);
    RESPTYPE(resp_sessiongroup_unset_focus);
    RESPTYPE(resp_sessiongroup_reset_focus);
    RESPTYPE(resp_sessiongroup_set_tx_session);
    RESPTYPE(resp_sessiongroup_set_tx_all_sessions);
    RESPTYPE(resp_sessiongroup_set_tx_no_session);
    RESPTYPE(resp_session_create);
#if !defined(VIVOX_VERSION) || VIVOX_VERSION < 3
    RESPTYPE(resp_session_connect);
    RESPTYPE(resp_session_media_connect);
    RESPTYPE(resp_session_get_local_audio_info);
    RESPTYPE(resp_session_render_audio_start);
    RESPTYPE(resp_session_render_audio_stop);
    RESPTYPE(resp_session_channel_get_participants);
    RESPTYPE(resp_account_channel_create_and_invite);
    RESPTYPE(resp_account_channel_folder_create);
    RESPTYPE(resp_account_channel_folder_update);
    RESPTYPE(resp_account_channel_folder_delete);
    RESPTYPE(resp_account_channel_folder_get_info);
    RESPTYPE(resp_account_get_voice_fonts);       //deprecated
    RESPTYPE(resp_account_get_call_logs);
    RESPTYPE(resp_session_media_ringback);
#endif
    RESPTYPE(resp_session_media_disconnect);
    RESPTYPE(resp_session_terminate);
    RESPTYPE(resp_session_mute_local_speaker);
    RESPTYPE(resp_session_set_local_speaker_volume);
    RESPTYPE(resp_session_channel_invite_user);
    RESPTYPE(resp_session_set_participant_volume_for_me);
    RESPTYPE(resp_session_set_participant_mute_for_me);
    RESPTYPE(resp_session_set_3d_position);
    RESPTYPE(resp_session_set_voice_font);
    RESPTYPE(resp_account_channel_get_list);
    RESPTYPE(resp_account_channel_create);
    RESPTYPE(resp_account_channel_update);
    RESPTYPE(resp_account_channel_delete);
    RESPTYPE(resp_account_channel_favorites_get_list);
    RESPTYPE(resp_account_channel_favorite_set);
    RESPTYPE(resp_account_channel_favorite_delete);
    RESPTYPE(resp_account_channel_favorite_group_set);
    RESPTYPE(resp_account_channel_favorite_group_delete);
    RESPTYPE(resp_account_channel_get_info);
    RESPTYPE(resp_account_channel_search);
    RESPTYPE(resp_account_buddy_search);
    RESPTYPE(resp_account_channel_add_moderator);
    RESPTYPE(resp_account_channel_remove_moderator);
    RESPTYPE(resp_account_channel_get_moderators);
    RESPTYPE(resp_account_channel_add_acl);
    RESPTYPE(resp_account_channel_remove_acl);
    RESPTYPE(resp_account_channel_get_acl);
    RESPTYPE(resp_channel_mute_user);
    RESPTYPE(resp_channel_ban_user);
    RESPTYPE(resp_channel_get_banned_users);
    RESPTYPE(resp_channel_kick_user);
    RESPTYPE(resp_channel_mute_all_users);
    RESPTYPE(resp_connector_mute_local_mic);
    RESPTYPE(resp_connector_mute_local_speaker);
    RESPTYPE(resp_connector_set_local_mic_volume);
    RESPTYPE(resp_connector_set_local_speaker_volume);
    RESPTYPE(resp_connector_get_local_audio_info);
    RESPTYPE(resp_account_buddy_set);
    RESPTYPE(resp_account_buddy_delete);
    RESPTYPE(resp_account_buddygroup_set);
    RESPTYPE(resp_account_buddygroup_delete);
    RESPTYPE(resp_account_list_buddies_and_groups);
    RESPTYPE(resp_session_send_message);
    RESPTYPE(resp_account_set_presence);
    RESPTYPE(resp_account_send_subscription_reply);
    RESPTYPE(resp_session_send_notification);
    RESPTYPE(resp_account_create_block_rule);
    RESPTYPE(resp_account_delete_block_rule);
    RESPTYPE(resp_account_list_block_rules);
    RESPTYPE(resp_account_create_auto_accept_rule);
    RESPTYPE(resp_account_delete_auto_accept_rule);
    RESPTYPE(resp_account_list_auto_accept_rules);
    RESPTYPE(resp_account_update_account);
    RESPTYPE(resp_account_get_account);
    RESPTYPE(resp_account_send_sms);
    RESPTYPE(resp_aux_connectivity_info);
    RESPTYPE(resp_aux_get_render_devices);
    RESPTYPE(resp_aux_get_capture_devices);
    RESPTYPE(resp_aux_set_render_device);
    RESPTYPE(resp_aux_set_capture_device);
    RESPTYPE(resp_aux_get_mic_level);
    RESPTYPE(resp_aux_get_speaker_level);
    RESPTYPE(resp_aux_set_mic_level);
    RESPTYPE(resp_aux_set_speaker_level);
    RESPTYPE(resp_aux_render_audio_start);
    RESPTYPE(resp_aux_render_audio_stop);
    RESPTYPE(resp_aux_capture_audio_start);
    RESPTYPE(resp_aux_capture_audio_stop);
    RESPTYPE(resp_aux_global_monitor_keyboard_mouse);
    RESPTYPE(resp_aux_set_idle_timeout);
    RESPTYPE(resp_aux_create_account);
    RESPTYPE(resp_aux_reactivate_account);
    RESPTYPE(resp_aux_deactivate_account);
    RESPTYPE(resp_account_post_crash_dump);
    RESPTYPE(resp_aux_reset_password);
    RESPTYPE(resp_sessiongroup_set_session_3d_position);
    RESPTYPE(resp_account_get_session_fonts);
    RESPTYPE(resp_account_get_template_fonts);
    RESPTYPE(resp_aux_start_buffer_capture);
    RESPTYPE(resp_aux_play_audio_buffer);
    RESPTYPE(resp_sessiongroup_control_recording);
    RESPTYPE(resp_sessiongroup_control_playback);
    RESPTYPE(resp_sessiongroup_set_playback_options);
    RESPTYPE(resp_session_text_connect);
    RESPTYPE(resp_session_text_disconnect);
#if VIVOX_VERSION >= 3
    RESPTYPE(resp_channel_set_lock_mode);
    RESPTYPE(resp_aux_render_audio_modify);
    RESPTYPE(resp_session_send_dtmf);
    RESPTYPE(resp_aux_set_vad_properties);
    RESPTYPE(resp_aux_get_vad_properties);
    RESPTYPE(resp_sessiongroup_control_audio_injection);
    RESPTYPE(resp_account_channel_change_owner);
    RESPTYPE(resp_account_channel_get_participants);
    RESPTYPE(resp_account_send_user_app_data);
    RESPTYPE(resp_aux_diagnostic_state_dump);
    RESPTYPE(resp_account_web_call);
#endif
    }
#if VIVOX_VERSION >= 3
    vivox_ct_fatal( 132 == resp_max ); // If this fails, we're missing some response type
#else
    vivox_ct_fatal( 116 == resp_max ); // If this fails, we're missing some response type
#endif

#endif // _DEBUG

    return str;
}


//////////////////////////////////////////////////////////////////////////////
// m_logAnyError()
// 
// Determines if an error is present in a response message and logs it.
// Returns true if there was an error
////////////////////////////////////////////////////////////////////////////////
template <typename StrClass, typename Glue>
bool
Vivox<StrClass, Glue>::m_logAnyError( const vx_resp_base& resp )
{
    if ( resp.return_code != 0 )
    {
        const char* unsolicited = m_bIsHandlingUnsolicitedResponse ? " (unsolicited)" : "";
#ifdef _DEBUG
        const char* str = m_getRespTypeName( resp.type );
        Glue::warning( __FILE__, __LINE__, "Error while processing Response type %s(%d): %d/\"%s\"%s", str, resp.type, resp.status_code, resp.status_string, unsolicited );
#else
        Glue::warning( __FILE__, __LINE__, "Error while processing Response type %d: %d/\"%s\"%s", resp.type, resp.status_code, resp.status_string, unsolicited );
#endif
        if ( !m_bIsHandlingUnsolicitedResponse )
        {
            FIRE_EVENT( VivoxOnGenericError, (m_iLastError, m_sLastErrorString) );
        }
        return true;
    }

    return false;
}


//////////////////////////////////////////////////////////////////////////////
// m_findSession
// 
// Attempts to find a session.  The session returned is mutable
////////////////////////////////////////////////////////////////////////////////
template <typename StrClass, typename Glue>
typename Vivox<StrClass, Glue>::Session*
Vivox<StrClass, Glue>::m_findSession( const StrClass& sSession )
{
    SessionMap::iterator iter( m_mSessions.find( sSession ) );
    if ( iter != m_mSessions.end() )
    {
        return &(*iter).second;
    }

    return 0;
}


//////////////////////////////////////////////////////////////////////////////
// m_findParticipant
// 
// Attempts to find a participant.  The participant returned is mutable
////////////////////////////////////////////////////////////////////////////////
template <typename StrClass, typename Glue>
typename Vivox<StrClass, Glue>::Participant*
Vivox<StrClass, Glue>::m_findParticipant( const StrClass& sURI, const StrClass& sSession )
{
    Session* pSession = m_findSession( sSession );
    if ( 0 == pSession )
        return 0;

    Session::ParticipantMap::iterator iter( pSession->mParticipants.find( sURI ) );
    if ( iter == pSession->mParticipants.end() )
        return 0;

    return &(*iter).second;
}



////////////////////////////////////////////////////////////////////////////////
// HANDLER FUNCTIONS
////////////////////////////////////////////////////////////////////////////////

template <typename StrClass, typename Glue>
void
Vivox<StrClass, Glue>::m_processAccountLoginStateChange( const vx_evt_account_login_state_change& p )
{
    StrClass s;
    switch ( p.state )
    {
    case login_state_logged_in:
        s = "logged_in";
        break;

    case login_state_logged_out:
        s = "logged_out";
        break;
    
    case login_state_logging_in:
        s = "logging_in";
        break;

    case login_state_logging_out:
        s = "logging_out";
        if ( LS_LOGGED_IN == m_eLoginState )
            m_eLoginState = LS_LOGGING_OUT;
        break;
    
    case login_state_resetting:
        s = "resetting";
        break;
    
    case login_state_error:
    default:
        s = "error";
        break;
    }

    Glue::log( VLS_DEBUG, "AccountLoginStateChange: %d(%s) %d %s", p.state, s.c_str(), p.status_code, p.status_string );

    FIRE_EVENT( VivoxOnLoginStateChange, (s) );

    if ( login_state_logged_out == p.state )
    {
        // vivox_db_fatal( LS_LOGGING_OUT == m_eLoginState );
        m_eLoginState = LS_LOGGED_OUT;
        m_bNeedLogout = false;
        m_sAccountHandle.clear();
        m_sAccountName.clear();
        m_sLocalDisplayName.clear();
        m_sLocalURI.clear();
        m_clearSessions();

        if ( CS_WAITING_FOR_LOGOUT == m_eConnectState )
        {
            // Logout complete, continue the shutdown
            m_bNeedLogin = false;
            Shutdown( true );
        }
        else if ( m_bNeedLogin )
        {
            m_bNeedLogin = false;
            BeginLogin( m_sNextLoginAcct, m_sNextLoginPassword );
            m_sNextLoginAcct.clear();
            m_sNextLoginPassword.clear();
        }
    }
}

template <typename StrClass, typename Glue>
void
Vivox<StrClass, Glue>::m_processSessionGroupAdded( const vx_evt_sessiongroup_added& p )
{
    Glue::log( VLS_DEBUG, "SessionGroupAdded: %s", p.sessiongroup_handle );

    StrClass s( p.sessiongroup_handle );

    // Prepare to defer sessions for this session group
    vivox_db_fatal( m_mGroupInfo.count( s ) == 0 );
    GroupInfo& info = m_mGroupInfo[ s ];
    info.sHandle = s;

    FIRE_EVENT( VivoxOnSessionGroupAdded, (s) );
}

template <typename StrClass, typename Glue>
void
Vivox<StrClass, Glue>::m_processSessionGroupRemoved( const vx_evt_sessiongroup_removed& p )
{
    Glue::log( VLS_DEBUG, "SessionGroupRemoved: %s", p.sessiongroup_handle );

    StrClass s( p.sessiongroup_handle );

    for ( SessionMap::iterator iter( m_mSessions.begin() );
          iter != m_mSessions.end();
          /*in loop*/ )
    {
        const StrClass& sSession = (*iter).first;
        Session& ssn = (*iter).second;
        if ( ssn.sGroupHandle == s )
        {
            // Fire participant events
            for ( Session::ParticipantMap::const_iterator pIter( ssn.mParticipants.begin() );
                  pIter != ssn.mParticipants.end();
                  ++pIter )
            {
                const Participant& p = (*pIter).second;

                FIRE_EVENT(VivoxOnParticipantRemoved,(ssn.sGroupHandle, ssn.sHandle, p, RR_UNKNOWN));
            }

            m_mSessionInfo.erase( ssn.sURI );
            FIRE_EVENT( VivoxOnSessionRemoved, (s, sSession, ssn) );
            m_mTextMessages.erase( sSession );
            m_mSessions.erase( iter++ );
            continue;
        }

        ++iter;
    }

    FIRE_EVENT( VivoxOnSessionGroupRemoved, (s) );

    m_mGroupInfo.erase( s );

    // If this was our positional session group, clear it
    if ( s == m_sPositionSessionGroup )
    {
        SetPositionalSession( StrClass() );
    }
}

template <typename StrClass, typename Glue>
void
Vivox<StrClass, Glue>::m_processSessionAdded( const vx_evt_session_added& p )
{
    Glue::log( VLS_DEBUG, "SessionAdded: %s %s %s %s", p.sessiongroup_handle, p.session_handle, p.displayname, p.uri );

    StrClass sGroup( p.sessiongroup_handle );
    StrClass sSession( p.session_handle );

    vivox_db_fatal( m_mGroupInfo.count( sGroup ) != 0 );
    GroupInfo& group = m_mGroupInfo[ sGroup ];
    group.sSessions.insert( sSession );

    vivox_db_fatal( m_mSessions.count( sSession ) == 0 );

    Session& session = m_mSessions[ sSession ];
    session.sHandle = sSession;
    session.sGroupHandle = sGroup;
    session.bIsChannel = ( p.is_channel != 0 );
    session.sDisplayName = p.displayname;
    session.sURI = p.uri;
#if !defined(VIVOX_VERSION) || VIVOX_VERSION < 3
    session.sAliasURI = p.alias_uri;
#endif
    session.sChannelName = p.channel_name;

    bool bNeedDisconnect = false;

    // Update the group handle in our sessioninfo
    SessionInfoMap::iterator iter( m_mSessionInfo.find( session.sURI ) );
    if ( iter != m_mSessionInfo.end() )
    {
        SessionInfo& info = (*iter).second;
        info.sGroup = session.sGroupHandle;
        bNeedDisconnect = info.bNeedDisconnect;
    }
    else
    {
        Glue::warning( __FILE__, __LINE__, "SessionAdded: Failed to find session info for URI %s", session.sURI.c_str() );
    }

    FIRE_EVENT( VivoxOnSessionAdded, (session.sGroupHandle, sSession, session) );

    if ( AreSessionsSuspended() && !bNeedDisconnect )
    {
        // This session must have been requested before sessions were suspended, but
        // completed connecting after sessions were suspended.  All events still fire
        // for this session.
        // We don't need to mark the session as suspended if we're already planning on
        // disconnecting the session.
        if ( iter != m_mSessionInfo.end() )
            (*iter).second.bSuspended = true;

        bNeedDisconnect = true;
    }

    if ( bNeedDisconnect )
    {
        TerminateSession( session.sHandle );
        return;
    }
}

template <typename StrClass, typename Glue>
void
Vivox<StrClass, Glue>::m_processSessionRemoved( const vx_evt_session_removed& p )
{
    Glue::log( VLS_DEBUG, "SessionRemoved: %s %s", p.sessiongroup_handle, p.session_handle );

    StrClass sSessionGroup( p.sessiongroup_handle );
    StrClass sSession( p.session_handle );

    const Session* pSession = FindSession( sSession );
    if ( pSession )
    {
        // Fire participant events
        for ( Session::ParticipantMap::const_iterator pIter( pSession->mParticipants.begin() );
              pIter != pSession->mParticipants.end();
              ++pIter )
        {
            const Participant& p = (*pIter).second;

            FIRE_EVENT(VivoxOnParticipantRemoved,(sSessionGroup, sSession, p, RR_UNKNOWN));
        }

        // If the session isn't suspended, remove it
        SessionInfoMap::iterator iter( m_mSessionInfo.find( pSession->sURI ) );
        if ( iter != m_mSessionInfo.end() )
        {
            SessionInfo& info = (*iter).second;
            if ( !info.bSuspended )
                m_mSessionInfo.erase( iter );
        }
        FIRE_EVENT( VivoxOnSessionRemoved, (sSessionGroup, sSession, *pSession) );
    }
    else
    {
        Glue::warning( __FILE__, __LINE__, "SessionRemoved: Session %s/%s not found!", sSessionGroup.c_str(), sSession.c_str() );
    }

    // If this was our positional session, clear it
    if ( sSession == m_sPositionSession )
    {
        SetPositionalSession( StrClass() );
    }

    m_mSessions.erase( sSession );
    m_mTextMessages.erase( sSession );

    vivox_db_fatal( m_mGroupInfo.count( sSessionGroup ) != 0 );
    GroupInfo& group = m_mGroupInfo[ sSessionGroup ];
    group.sSessions.erase( sSession );

    // If this is our current deferred session, go on to the next.  We can't guarantee
    // this is the front because we may get this event unsolicited.
    m_removeDeferred( group, sSession, true );
    m_processNextDeferred( group, false );

    // If a test is pending, start the test once all sessions have been suspended
    if ( m_testType != TT_NONE && m_mSessions.empty() && m_allSessionsSuspended() )
    {
        switch ( m_testType )
        {
        case TT_VU_MONITOR:     m_beginVUMonitor(); break;
        case TT_ECHO_RECORD:    m_beginEchoTest();  break;
        case TT_ECHO_PLAYBACK:  m_playEchoTest();   break;
        }
    }
}

template <typename StrClass, typename Glue>
void
Vivox<StrClass, Glue>::m_processSessionUpdated( const vx_evt_session_updated& p )
{
    Glue::log( VLS_DEBUG, "SessionUpdated: %s %s %s transmit_enabled:%s", p.sessiongroup_handle, p.session_handle, p.uri, p.transmit_enabled ? "true" : "false" );

    StrClass sSessionGroup( p.sessiongroup_handle );
    StrClass sSession( p.session_handle );

    vivox_db_fatal( m_mSessions.count( sSession ) != 0 );

    Session* pSession = m_findSession( sSession );
    if ( pSession )
    {
        vivox_db_fatal( pSession->sURI == p.uri );
        pSession->bTransmitEnabled = p.transmit_enabled != 0;
        pSession->bFocused = p.is_focused != 0;
        pSession->bMuted = p.is_muted != 0;
        pSession->iFont = p.session_font_id;
        pSession->fVolume = VivoxClamp( 0.0f, static_cast< float >( p.volume ) / 100.0f, 1.0f );
        FIRE_EVENT( VivoxOnSessionUpdated, (sSessionGroup, sSession, *pSession) );
    }
    else
    {
        Glue::warning( __FILE__, __LINE__, "SessionUpdated: Session %s/%s not found!", sSessionGroup.c_str(), sSession.c_str() );
    }
}

template <typename StrClass, typename Glue>
void
Vivox<StrClass, Glue>::m_processSessionNotification( const vx_evt_session_notification& p )
{
    Glue::log( VLS_DEBUG, "SessionNotification: %s %s %d", p.session_handle, p.participant_uri, p.notification_type );

    StrClass sSession( p.session_handle );

    vivox_db_fatal( m_mSessions.count( sSession ) != 0 );

    Session* pSession = m_findSession( sSession );
    if ( pSession )
    {
        StrClass sURI( p.participant_uri );

        // Convert to lowercase
        m_tolower( sURI );

        Session::ParticipantMap::iterator iter( pSession->mParticipants.find( sURI ) );
        if ( iter != pSession->mParticipants.end() )
        {
            Participant& ppt = (*iter).second;

            Participant old;
            old.CopySimple( ppt ); // Don't copy strings, just data that can change

            switch ( p.notification_type )
            {
            case notification_hand_lowered: ppt.bPushToTalk = false; break;
            case notification_hand_raised:  ppt.bPushToTalk = true;  break;
            }

            if ( ppt.bPushToTalk != old.bPushToTalk )
            {
                // Fire a participant updated event
                Glue::log( VLS_DEBUG, "ParticipantUpdated: %s %s push-to-talk:%s", p.session_handle, p.participant_uri, ppt.bPushToTalk ? "enabled" : "disabled" );

                m_fireParticipantUpdated( pSession->sGroupHandle, sSession, old, ppt );
            }
        }
        else
        {
            Glue::warning( __FILE__, __LINE__, "SessionNotification: Participant %s not found in session %s!", p.participant_uri, sSession.c_str() );
        }
    }
    else
    {
        Glue::warning( __FILE__, __LINE__, "SessionNotification: Session %s not found!", sSession.c_str() );
    }
}

template <typename StrClass, typename Glue>
void
Vivox<StrClass, Glue>::m_processParticipantAdded( const vx_evt_participant_added& p )
{
    Glue::log( VLS_DEBUG, "ParticipantAdded: %s %s %s %s %s", p.sessiongroup_handle, p.session_handle, p.participant_uri, p.display_name, p.account_name );

    StrClass sSessionGroup( p.sessiongroup_handle );
    StrClass sSession( p.session_handle );

    Session* pSession = m_findSession( sSession );
    if ( 0 == pSession )
    {
        Glue::warning( __FILE__, __LINE__, "ParticipantAdded: Failed to find session %s/%s", p.sessiongroup_handle, p.session_handle );
        return;
    }

    StrClass sURI( p.participant_uri );

    // Convert to lowercase
    m_tolower( sURI );

    Participant& ppt = pSession->mParticipants[ sURI ];
    ppt.pSession = pSession;
    ppt.sURI = sURI;
	if(p.account_name)
	{
		ppt.sAccount = p.account_name;
		m_tolower( ppt.sAccount );
	}
	if(p.display_name)
		ppt.sDisplayName = p.display_name;
    // Don't care about participant type for now

    // If this is me, set the "self" pointer in the session
    if ( m_sAccountName == ppt.sAccount )
    {
        m_sLocalURI = p.participant_uri;
        m_sLocalDisplayName = p.display_name;
        if ( 0 == pSession->pSelf )
        {
            pSession->pSelf = &ppt;
            ppt.bSelf = true;
        }
        else
        {
            // Self appears to have already been set
            Glue::warning( __FILE__, __LINE__, "ParticipantAdded: %s appears to be self, but self already set to %s",
                           sURI.c_str(),
                           pSession->pSelf->sURI.c_str() );
        }
#ifdef VIVOX_V3_RACE_FIX
        // If this is the local participant we can finally release the session and process the next deferred item
        vivox_db_fatal( m_mGroupInfo.count( pSession->sGroupHandle ) != 0 ); // should know about the group
        GroupInfo& info = m_mGroupInfo[ pSession->sGroupHandle ];
        m_removeDeferred( info, pSession->sURI, false );
        m_processNextDeferred( info, false );
#endif
    }

    FIRE_EVENT( VivoxOnParticipantAdded, (sSessionGroup, sSession, ppt) );

    // Handle any pending text messages
    if ( !ppt.m_pending.empty() )
    {
        TextMessageList v; v.swap( ppt.m_pending );

        while ( !v.empty() )
        {
            TextMessage& t = v.front();
            FIRE_EVENT(VivoxOnTextMessage,(pSession->sHandle, ppt, t.sHeader, t.sBody));
            v.pop_front();
        }

        vivox_db_fatal( ppt.m_pending.empty() );
    }
}

template <typename StrClass, typename Glue>
void
Vivox<StrClass, Glue>::m_processParticipantRemoved( const vx_evt_participant_removed& p )
{
    const char* sReason = "";
#ifdef _DEBUG
    switch ( p.reason )
    {
    #define REASON(s) case participant_##s: sReason = #s; break
    REASON(left);
    REASON(timeout);
    REASON(kicked);
    REASON(banned);
    #undef REASON
    }
#endif
    Glue::log( VLS_DEBUG, "ParticipantRemoved: %s %s %s %s %s", p.sessiongroup_handle, p.session_handle, p.participant_uri, p.account_name, sReason );

    StrClass sSession( p.session_handle );

    Session* pSession = m_findSession( sSession );
    if ( pSession )
    {
        StrClass sURI( p.participant_uri );

        // Convert to lowercase
        m_tolower( sURI );

        Session::ParticipantMap::iterator iter( pSession->mParticipants.find( sURI ) );
        if ( iter != pSession->mParticipants.end() )
        {
            StrClass sSessionGroup( p.sessiongroup_handle );

            // Verify at compile-time that these match
            vivox_ct_fatal( participant_left == RR_LEFT );
            vivox_ct_fatal( participant_timeout == RR_TIMEOUT );
            vivox_ct_fatal( participant_kicked == RR_KICKED );
            vivox_ct_fatal( participant_banned == RR_BANNED );

            Participant& ppt = (*iter).second;
#ifdef _DEBUG
            StrClass sLowerAccount( p.account_name );
            m_tolower( sLowerAccount );
            vivox_db_fatal( ppt.sAccount == sLowerAccount );
#endif
            FIRE_EVENT( VivoxOnParticipantRemoved, (sSessionGroup, sSession, ppt, (RemoveReason)p.reason) );

            // If this is the 'self' participant, remove the pointer in the session
            if ( &ppt == pSession->pSelf )
            {
                pSession->pSelf = 0;
            }

            pSession->mParticipants.erase( iter );
        }
        else
        {
            Glue::warning( __FILE__, __LINE__, "ParticipantRemoved: Failed to find participant URI %s", p.participant_uri );
        }
    }
    else
    {
        Glue::warning( __FILE__, __LINE__, "ParticipantRemoved: Failed to find session %s/%s", p.sessiongroup_handle, p.session_handle );
    }
}

template <typename StrClass, typename Glue>
void
Vivox<StrClass, Glue>::m_processParticipantUpdated( const vx_evt_participant_updated& p )
{
    StrClass sSessionGroup( p.sessiongroup_handle );
    StrClass sSession( p.session_handle );

    Session* pSession = m_findSession( sSession );
    if ( pSession )
    {
        // Apparently, this can be fired before we've been notified of a new participant,
        // so go ahead and allow participant creation here.
        StrClass sURI( p.participant_uri );

        // Convert to lowercase
        m_tolower( sURI );

        Participant& ppt = pSession->mParticipants[ sURI ];

        Participant old;
        old.CopySimple( ppt ); // Don't copy strings, just data that can change

        ppt.pSession   = pSession;
        ppt.bModMuted  = ( p.is_moderator_muted != 0 ? true : false );
        ppt.bSpeaking  = ( p.is_speaking != 0 ? true : false );
        ppt.fVolume    = static_cast< float >( p.volume ) / 100.0f;
        ppt.fEnergy    = p.energy;
#if VIVOX_VERSION >= 3
        ppt.bModerator = (p.type == participant_moderator);
#endif

        // Only fire events if something changed
        if ( old.bModMuted  != ppt.bModMuted  ||
             old.bSpeaking  != ppt.bSpeaking  ||
             old.bModerator != ppt.bModerator ||
             old.fVolume    != ppt.fVolume    ||
             old.fEnergy    != ppt.fEnergy    )
        {
            Glue::log( VLS_DEBUG, "ParticipantUpdated: %s %s %s %f%s", p.sessiongroup_handle, p.session_handle, p.participant_uri, p.energy, p.is_speaking ? " [speaking]" : "" );

            m_fireParticipantUpdated( sSessionGroup, sSession, old, ppt );
        }
    }
    else
    {
        Glue::warning( __FILE__, __LINE__, "ParticipantUpdated: Failed to find session %s/%s", p.sessiongroup_handle, p.session_handle );
    }
}

template <typename StrClass, typename Glue>
void
Vivox<StrClass, Glue>::m_processMediaStreamUpdated( const vx_evt_media_stream_updated& p )
{
    Glue::log( VLS_DEBUG, "MediaStreamUpdated: %s %s %d/\"%s\" %d", p.sessiongroup_handle, p.session_handle, p.status_code, p.status_string, p.state );

#if VIVOX_VERSION >= 3
    // Ignore new connecting and disconnecting states
    if ( p.state == session_media_connecting ||
         p.state == session_media_disconnecting )
    {
        return;
    }
#endif

    StrClass sSessionGroup( p.sessiongroup_handle );
    StrClass sSession( p.session_handle );

    Session* pSession = m_findSession( sSession );
    if ( pSession )
    {
        const bool bWasConnecting = ( Session::NUM_STATES == pSession->eState );

#ifdef VIVOX_V3_RACE_FIX // V3 has a race condition where we need to wait for our participant record to be added before connecting the next thing
        if ( bWasConnecting && session_media_disconnected == p.state )
#else
        if ( bWasConnecting && ( session_media_connected == p.state || session_media_disconnected == p.state ) )
#endif
        {
            // Try connecting the next session now
            vivox_db_fatal( m_mGroupInfo.count( sSessionGroup ) != 0 );
            GroupInfo& group = m_mGroupInfo[ sSessionGroup ];
            m_removeDeferred( group, pSession->sURI, false );
            m_processNextDeferred( group, false );
        }

        // Verify at compile-time that these match
        vivox_ct_fatal( Session::MS_NONE == session_media_none );
        vivox_ct_fatal( Session::MS_DISCONNECTED == session_media_disconnected );
        vivox_ct_fatal( Session::MS_CONNECTED == session_media_connected );
        vivox_ct_fatal( Session::MS_RINGING == session_media_ringing );
        vivox_ct_fatal( Session::MS_HOLD == session_media_hold );
        vivox_ct_fatal( Session::MS_REFER == session_media_refer );

        Session::State oldState = pSession->eState;
        pSession->eState = (Session::State)p.state;

        // If we were asked to disconnect this session before we had the media state,
        // we can disconnect it now.  Don't fire the media stream event.
        if ( pSession->eState != Session::MS_DISCONNECTED &&
             pSession->NeedsDisconnect() )
        {
            TerminateSession( pSession->sHandle );
            return;
        }

        StrClass sStatusString( p.status_string );

        FIRE_EVENT( VivoxOnMediaStreamUpdated, (sSessionGroup, sSession, p.status_code, sStatusString, oldState, *pSession) );
    }
    else
    {
        Glue::warning( __FILE__, __LINE__, "MediaStreamUpdated: Failed to find session %s/%s", p.sessiongroup_handle, p.session_handle );
    }
}

template <typename StrClass, typename Glue>
void
Vivox<StrClass, Glue>::m_processIdleStateChanged( const vx_evt_idle_state_changed& p )
{
    Glue::log( VLS_DEBUG, "IdleStateChanged: idle:%s", p.is_idle ? "true" : "false" );

    FIRE_EVENT( VivoxOnIdleStateChanged, (p.is_idle != 0) );
}

template <typename StrClass, typename Glue>
void
Vivox<StrClass, Glue>::m_processKeyboardMouseEvent( const vx_evt_keyboard_mouse& p )
{
    /*
    Glue::log( VLS_DEBUG, "KeyboardMouseEvent: name:%s  is_down:%s", p.name, p.is_down ? "true" : "false" );

    // Ignore it if it's not our push2talk event
    if ( strcmp( p.name, "push2talk" ) != 0 )
        return;

    m_handlePTTEvent( p.is_down != 0 );
    */
}

template <typename StrClass, typename Glue>
void
Vivox<StrClass, Glue>::m_processAudioProperties( const vx_evt_aux_audio_properties& p )
{
    Glue::log( VLS_DEBUG, "AudioPropertiesEvent: %d %f %d%s", p.mic_volume, p.mic_energy, p.speaker_volume, p.mic_is_active ? " [speaking]" : "" );

    const float fMicVolume     = VivoxClamp( 0.0f, static_cast< float >( p.mic_volume ) / 100.0f, 1.0f );
    const float fSpeakerVolume = VivoxClamp( 0.0f, static_cast< float >( p.speaker_volume ) / 100.0f, 1.0f );
    const bool  bSpeaking      = ( p.mic_is_active != 0 );
    const float fEnergy        = static_cast< float >( p.mic_energy );

    FIRE_EVENT( VivoxVUMonitor, ( fMicVolume, fSpeakerVolume, fEnergy, bSpeaking ) );
}

template <typename StrClass, typename Glue>
void
Vivox<StrClass, Glue>::m_processTextStreamUpdated( const vx_evt_text_stream_updated& p )
{
    Glue::log( VLS_DEBUG, "TextStreamUpdated: %d %d %d (%d/%s)", p.enabled, p.state, p.incoming, p.status_code, p.status_string );

    Session* pSession = m_findSession( p.session_handle );
    if ( pSession )
    {
        pSession->bTextConnected = ( p.enabled != 0 && p.state == session_text_connected );

        FIRE_EVENT(VivoxOnTextStreamUpdated,(pSession->sHandle, *pSession));

        if ( pSession->bTextConnected && !m_bTextMessagePending )
        {
            // Send first deferred text message
            TextMessageMap::iterator iter = m_mTextMessages.find( pSession->sHandle );
            if ( iter != m_mTextMessages.end() )
            {
                TextMessageList& v = (*iter).second;
                if ( !v.empty() )
                {
                    TextMessage& t = v.front();
                    m_sendTextMessage( pSession->sHandle, t.sHeader, t.sBody );
                    v.pop_front();
                }
                if ( v.empty() )
                    m_mTextMessages.erase( iter );
            }
        }
    }
    else
    {
        Glue::warning( __FILE__, __LINE__, "m_processTextStreamUpdated: session %s not found", p.session_handle );
    }
}

template <typename StrClass, typename Glue>
void
Vivox<StrClass, Glue>::m_processTextMessage( const vx_evt_message& p )
{
    Glue::log( VLS_DEBUG, "TextMessage: %s(%s) \"%s\" \"%s\"", p.participant_uri, p.participant_displayname, p.message_header, p.message_body );

    StrClass sSession( p.session_handle );
    StrClass sURI( p.participant_uri );
    m_tolower( sURI );

    Session* pSession = m_findSession( sSession );
    if ( 0 == pSession )
    {
        Glue::warning( __FILE__, __LINE__, "m_processTextMessage: session %s not found", sSession.c_str() );
        return;
    }

    // We can get text messages before participants are officially added, so defer until
    // we have the actual participant
    std::pair< Session::ParticipantMap::iterator, bool > result =
        pSession->mParticipants.insert( std::make_pair( sURI, Participant() ) );

    Participant& participant = (*result.first).second;
    if ( result.second || !participant.m_pending.empty() )
    {
        // Haven't gotten the participant added notification
        participant.pSession = pSession;
        participant.m_pending.push_back( TextMessage( p.message_header, p.message_body ) );
        return;
    }

    StrClass sHeader( p.message_header );
    StrClass sBody( p.message_body );

    FIRE_EVENT(VivoxOnTextMessage,(sSession, participant, sHeader, sBody));
}

#if VIVOX_VERSION >= 3
template <typename StrClass, typename Glue>
void
Vivox<StrClass, Glue>::m_handleVoiceServiceConnectionStateChanged( const vx_evt_voice_service_connection_state_changed& p )
{
    const bool kbConnected       = p.connected != 0;
    m_sVoiceServicePlatform      = p.platform       ? p.platform       : "";
    m_sVoiceServiceVersion       = p.version        ? p.version        : "";
    m_sVoiceServiceDataDirectory = p.data_directory ? p.data_directory : "";

    Glue::log( VLS_DEBUG, "VoiceServiceConnectionStateChanged: (connected:%s) (platform:%s) (version:%s) (data_directory:%s)",
               kbConnected ? "true" : "false", m_sVoiceServicePlatform.c_str(), m_sVoiceServiceVersion.c_str(), m_sVoiceServiceDataDirectory.c_str() );

    if ( kbConnected != m_bVoiceServiceConnected )
    {
        m_bVoiceServiceConnected = kbConnected;

        if ( m_bVoiceServiceConnected )
        {
            // Need to re-set the current devices now that we're connected to the VVS.
            SetInputDevice( m_sCurrentInputDevice );
            SetOutputDevice( m_sCurrentOutputDevice );
        }
        else
        {
            // Make sure the SDK handle gets freed
            DisconnectVoiceService();

            // Tear everything down
            m_clearSessions( true );
            if ( m_eLoginState != LS_LOGGED_OUT )
            {
                // Fire the logged out event
                FIRE_EVENT(VivoxOnLoginStateChange,("logged_out"));
                m_eLoginState = LS_LOGGED_OUT;
            }
            m_bNeedLogin = false;
            m_bNeedLogout = false;
            m_sAccountHandle.clear();
            m_sAccountName.clear();
            m_sLocalDisplayName.clear();
            m_sLocalURI.clear();
            m_eConnectState = CS_DISCONNECTED;
            m_bNeedConnect = false;
            m_bNeedDisconnect = false;
            m_sConnectionHandle.clear();
            m_sNextServer.clear();
            m_sNextServiceExe.clear();
            m_sNextServiceIP.clear();

            sReleaseVivoxSystemMutex();

            FIRE_EVENT(VivoxOnDisconnect,(false));
        }
    }

    FIRE_EVENT( VivoxVoiceServiceStateChange, (kbConnected) );
}
#endif

template <typename StrClass, typename Glue>
void
Vivox<StrClass, Glue>::m_processConnectorCreateResp( const vx_resp_connector_create& p )
{
    Glue::log( VLS_DEBUG, "ConnectorCreateResp: %d %s %s %s", p.base.status_code, p.base.status_string, p.connector_handle, p.version_id );

    vivox_db_fatal( CS_CONNECTING == m_eConnectState );

    // Call the handlers
    const bool bConnected = ( 0 == p.base.return_code );

    if ( !bConnected )
    {
        if ( m_bUsingVoiceService && m_bVoiceServiceConnected )
        {
            // Failed to create connector, need to join existing state?
            if ( RequestDiagnosticStateDump( DL_ALL ) )
            {
                m_eConnectState = CS_WAITING_FOR_STATE;
                // Don't fire the OnConnected state yet
                return;
            }
            else
            {
                Glue::warning( __FILE__, __LINE__, "Failed to request diagnostic state dump while connected to voice service" );
                m_eConnectState = CS_DISCONNECTED;
            }
        }
        else
        {
            m_eConnectState = CS_DISCONNECTED;
            const vx_req_connector_create* pRequest = (const vx_req_connector_create*)p.base.request;
            Glue::warning( __FILE__, __LINE__, "Vivox connection failed: %d %s (server: %s)", p.base.status_code, p.base.status_string, pRequest->acct_mgmt_server );
        }
    }
    else
    {
        m_eConnectState = CS_CONNECTED;
        m_sConnectionHandle = p.connector_handle;

        // Now that we're connected, set up initial state
        SetPushToTalkMode( m_bPushToTalk );
        SetLocalSpeakerMute( m_bLocalSpeakerMute );
        SetLocalMicMute( m_bLocalMicMute );
        SetLocalSpeakerVolume( m_fLocalSpeakerVolume );
        SetLocalMicVolume( m_fLocalMicVolume );
    }

    FIRE_EVENT( VivoxOnConnected, (bConnected) );
}

template <typename StrClass, typename Glue>
void
Vivox<StrClass, Glue>::m_processAccountLoginResp( const vx_resp_account_login& p )
{
    Glue::log( VLS_DEBUG, "AccountLoginResp: %d %s %s", p.base.status_code, p.base.status_string, p.account_handle );

    // vivox_db_fatal( LS_LOGGING_IN == m_eLoginState );
    
    const bool bLoginSuccess = ( 0 == p.base.return_code );

    const vx_req_account_login* pRequest = (const vx_req_account_login*)p.base.request;
    if ( !bLoginSuccess )
    {
        m_eLoginState = LS_LOGGED_OUT;
        Glue::warning( __FILE__, __LINE__, "Vivox login failed: %d %s (uname: %s)", p.base.status_code, p.base.status_string, pRequest->acct_name );
    }
    else
    {
        m_eLoginState = LS_LOGGED_IN;
        m_sAccountHandle = p.account_handle;
        m_sAccountName = pRequest->acct_name;
        m_tolower( m_sAccountName );
#if VIVOX_VERSION >= 3
        m_sLocalDisplayName = p.display_name;
        m_sLocalURI = p.uri;
        m_tolower( m_sLocalURI );
#endif
    }

    FIRE_EVENT( VivoxOnLogin, (bLoginSuccess) );
}

template <typename StrClass, typename Glue>
void
Vivox<StrClass, Glue>::m_processGetRenderDevicesResp( const vx_resp_aux_get_render_devices& p )
{
    Glue::log( VLS_DEBUG, "GetRenderDevicesResp: %d %s %s %d", p.base.status_code, p.base.status_string, p.current_render_device ? p.current_render_device->device : "(null)", p.count );

    if ( p.base.return_code == 0 )
    {
        if ( p.current_render_device )
            m_sCurrentOutputDevice = p.current_render_device->device;

        if ( m_sDefaultOutputDevice.empty() )
            m_sDefaultOutputDevice = m_sCurrentOutputDevice;

        bool bFound = false;

        std::vector< StrClass > aDevices;
        aDevices.reserve( p.count );
        for ( int i = 0; i != p.count; ++i )
        {
            if ( p.render_devices[ i ] )
            {
                aDevices.push_back( p.render_devices[ i ]->device );
                if ( aDevices.back() == m_sDefaultOutputDevice )
                    bFound = true;
            }
        }

        // If we couldn't find the default input device, reset to current
        if ( !bFound )
        {
            m_sDefaultInputDevice = m_sCurrentOutputDevice;
        }

        FIRE_EVENT( VivoxOnGetOutputDevices, (m_sCurrentOutputDevice, aDevices) );
    }
    else
    {
        if ( p.base.status_code != 7001 ) // No render devices found
        {
            Glue::warning( __FILE__, __LINE__, "GetRenderDevicesResp failed: %d/\"%s\"", p.base.status_code, p.base.status_string );
        }

        // Fire event without any real data
        std::vector< StrClass > aDevices;
        FIRE_EVENT( VivoxOnGetOutputDevices, (m_sCurrentOutputDevice, aDevices) );
    }
}

template <typename StrClass, typename Glue>
void
Vivox<StrClass, Glue>::m_processGetCaptureDevicesResp( const vx_resp_aux_get_capture_devices& p )
{
    Glue::log( VLS_DEBUG, "GetCaptureDevicesResp: %d %s %s %d", p.base.status_code, p.base.status_string, p.current_capture_device ? p.current_capture_device->device : "(null)", p.count );

    if ( p.base.return_code == 0 )
    {
        if ( p.current_capture_device )
            m_sCurrentInputDevice = p.current_capture_device->device;

        if ( m_sDefaultInputDevice.empty() )
            m_sDefaultInputDevice = m_sCurrentInputDevice;

        bool bFound = false;

        std::vector< StrClass > aDevices;
        aDevices.reserve( p.count );
        for ( int i = 0; i != p.count; ++i )
        {
            if ( p.capture_devices[ i ] )
            {
                aDevices.push_back( p.capture_devices[ i ]->device );
                if ( aDevices.back() == m_sDefaultInputDevice )
                    bFound = true;
            }
        }

        // If we couldn't find the default input device, reset to current
        if ( !bFound )
        {
            m_sDefaultInputDevice = m_sCurrentInputDevice;
        }

        FIRE_EVENT( VivoxOnGetInputDevices, (m_sCurrentInputDevice, aDevices) );
    }
    else
    {
        if ( p.base.status_code != 7002 ) // No capture devices found
        {
            Glue::warning( __FILE__, __LINE__, "GetCaptureDevicesResp failed: %d/\"%s\"", p.base.status_code, p.base.status_string );
        }

        // Fire event without any real data
        std::vector< StrClass > aDevices;
        FIRE_EVENT( VivoxOnGetInputDevices, (m_sCurrentInputDevice, aDevices) );
    }
}

template <typename StrClass, typename Glue>
void
Vivox<StrClass, Glue>::m_processSetRenderDeviceResp( const vx_resp_aux_set_render_device& p )
{
    Glue::log( VLS_DEBUG, "SetRenderDeviceResp: %d %s", p.base.status_code, p.base.status_string );

    // Make sure we got the default output device before this happens
    vivox_db_fatal( !m_sDefaultOutputDevice.empty() );

    bool bSucceeded = ( p.base.return_code == 0 );
    const vx_req_aux_set_render_device* pReq = (const vx_req_aux_set_render_device*)p.base.request;

    StrClass sDevice;
    if( pReq->render_device_specifier )
        sDevice = pReq->render_device_specifier;

    if ( bSucceeded )
        m_sCurrentOutputDevice = sDevice;

    FIRE_EVENT( VivoxOnSetOutputDevice, (sDevice, bSucceeded) );
}

template <typename StrClass, typename Glue>
void
Vivox<StrClass, Glue>::m_processSetCaptureDeviceResp( const vx_resp_aux_set_capture_device& p )
{
    Glue::log( VLS_DEBUG, "SetCaptureDeviceResp: %d %s", p.base.status_code, p.base.status_string );

    // Make sure we got the default input device before this happens
    vivox_db_fatal( !m_sDefaultInputDevice.empty() );

    bool bSucceeded = ( p.base.return_code == 0 );
    const vx_req_aux_set_capture_device* pReq = (const vx_req_aux_set_capture_device*)p.base.request;

    StrClass sDevice;
    if( pReq->capture_device_specifier != NULL )
        sDevice = pReq->capture_device_specifier;

    if ( bSucceeded )
        m_sCurrentInputDevice = sDevice;

    FIRE_EVENT( VivoxOnSetInputDevice, (sDevice, bSucceeded) );
}

template <typename StrClass, typename Glue>
void
Vivox<StrClass, Glue>::m_processGetModeratorsResp( const vx_resp_account_channel_get_moderators& p )
{
    Glue::log( VLS_DEBUG, "GetModeratorsResp: %d", p.participants_size );

    if ( 0 == p.base.return_code && p.participants_size >= 0 )
    {
        const vx_req_account_channel_get_moderators* pReq = (const vx_req_account_channel_get_moderators*)p.base.request;

        std::vector< StrClass > mods; mods.reserve( p.participants_size );

        for ( int i = 0; i != p.participants_size; ++i )
        {
            vx_participant* ppt = p.participants[ i ];
            if ( ppt && ppt->is_moderator != 0 )
            {
                mods.push_back( ppt->uri );

                // Lowercase the uri
                m_tolower( mods.back() );
            }
        }

        StrClass sURI( pReq->channel_uri );

        FIRE_EVENT(VivoxOnModeratorList,(sURI, mods));
    }
}

#if VIVOX_VERSION >= 3
template <typename StrClass, typename Glue>
void
Vivox<StrClass, Glue>::m_processDiagnosticStateDumpResp( const vx_resp_aux_diagnostic_state_dump& dump )
{
    Glue::log( VLS_DEBUG, "DiagnosticStateDump: %d connectors", dump.state_connector_count );

    DiagConnector con;

    // Fire events if we're not waiting on state from login
    const bool bConnecting = ( m_eConnectState == CS_WAITING_FOR_STATE );

    if ( !bConnecting )
    {
        FIRE_EVENT(VivoxOnBeginDiagnosticStateDump,(dump.state_connector_count));
    }
    else
    {
        // If we're fetching state to join it, we expect that there's only one
        // connector (or zero if something bad happened).
        vivox_db_fatal( dump.state_connector_count <= 1 );
    }

    if ( !m_logAnyError( dump.base ) )
    {    
        // If you thought this was ugly, you thought right.
        for ( int iCon = 0; iCon != dump.state_connector_count; ++iCon )
        {
            const vx_state_connector_t& cc = (*dump.state_connectors[ iCon ]);
    
            con.sHandle = cc.connector_handle;
            con.fMicVol = static_cast< float >( cc.mic_vol ) / 100.0f;
            con.bMicMute = cc.mic_mute != 0;
            con.fSpeakerVol = static_cast< float >( cc.speaker_vol ) / 100.0f;
            con.bSpeakerMute = cc.speaker_mute != 0;
            con.aAccounts.resize( cc.state_accounts_count );
    
            for ( int iAccount = 0; iAccount != cc.state_accounts_count; ++iAccount )
            {
                const vx_state_account_t& ca = (*cc.state_accounts[ iAccount ]);
                DiagAccount& acct = con.aAccounts[ iAccount ];
    
                acct.sHandle = ca.account_handle;
                acct.sAccountURI = ca.account_uri;
                // Lowercase the URI
                m_tolower( acct.sAccountURI );
                // Parse the account name
                acct.sAccountName = m_parseAcctNameFromURI( ca.account_uri );
                acct.sDisplayName = ca.display_name;
                acct.aSessionGroups.resize( ca.state_sessiongroups_count );
    
                for ( int iSG = 0; iSG != ca.state_sessiongroups_count; ++iSG )
                {
                    const vx_state_sessiongroup_t& csg = (*ca.state_sessiongroups[ iSG ]);
                    DiagSessionGroup& sg = acct.aSessionGroups[ iSG ];
    
                    sg.sHandle = csg.sessiongroup_handle;
                    sg.aSessions.resize( csg.state_sessions_count );
    
                    for ( int iS = 0; iS != csg.state_sessions_count; ++iS )
                    {
                        const vx_state_session_t& cs = (*csg.state_sessions[ iS ]);
                        DiagSession& s = sg.aSessions[ iS ];
    
                        s.sHandle = cs.session_handle;
                        s.sURI = cs.uri;
                        // Lowercase the URI
                        m_tolower( s.sURI );
                        s.sName = cs.name;
                        s.bAudioMutedForMe = cs.is_audio_muted_for_me != 0;
                        s.bTextMutedForMe = cs.is_text_muted_for_me != 0;
                        s.bTransmitting = cs.is_transmitting != 0;
                        s.bFocused = cs.is_focused != 0;
                        s.fVolume = static_cast< float >( cs.volume ) / 100.0f;
                        s.iFont = cs.session_font_id;
                        s.bHasAudio = cs.has_audio != 0;
                        s.bHasText = cs.has_text != 0;
                        s.bIsIncoming = cs.is_incoming != 0;
                        s.bIsPositional = cs.is_positional != 0;
                        s.bIsConnected = cs.is_connected != 0;
                        s.aParticipants.resize( cs.state_participant_count );
    
                        for ( int iP = 0; iP != cs.state_participant_count; ++iP )
                        {
                            const vx_state_participant_t& cp = (*cs.state_participants[ iP ]);
                            DiagParticipant& p = s.aParticipants[ iP ];

                            p.sAccountName = m_parseAcctNameFromURI( cp.uri );
                            m_tolower( p.sAccountName );
                            p.sURI = cp.uri;
                            m_tolower( p.sURI );
                            p.sDisplayName = cp.display_name;
                            p.bAudioEnabled = cp.is_audio_enabled != 0;
                            p.bTextEnabled = cp.is_text_enabled != 0;
                            p.bAudioMutedForMe = cp.is_audio_muted_for_me != 0;
                            p.bTextMutedForMe = cp.is_text_muted_for_me != 0;
                            p.bAudioModMuted = cp.is_audio_moderator_muted != 0;
                            p.bTextModMuted = cp.is_text_moderator_muted != 0;
                            p.bHandRaised = cp.is_hand_raised != 0;
                            p.bTyping = cp.is_typing != 0;
                            p.bSpeaking = cp.is_speaking != 0;
                            p.fVolume = static_cast< float >( cp.volume ) / 100.0f;
                            p.fEnergy = static_cast< float >( cp.energy );
                        }
                    }
                }
            }
    
            if ( !bConnecting )
            {
                // Fire the event once for each connector
                FIRE_EVENT(VivoxDiagnosticStateDump,(con));
            }
            else
            {
                // Make sure the m_bIsJoiningExistingSession var gets reset when we leave scope
                ScopedReset resetter( m_bIsJoiningExistingSession );
                m_bIsJoiningExistingSession = true;

                FIRE_EVENT(VivoxOnJoinExistingSessionBegin,());

                // Set up this as our state.
                m_eConnectState = CS_CONNECTED;
                m_sConnectionHandle = con.sHandle;
                m_fLocalMicVolume = con.fMicVol;
                m_fLocalSpeakerVolume = con.fSpeakerVol;

                FIRE_EVENT(VivoxOnConnected,(true));

                if ( !con.aAccounts.empty() )
                {
                    // Just take the first account since we don't support multiple logins
                    const DiagAccount& acct = con.aAccounts.front();

                    FIRE_EVENT(VivoxOnLoginStateChange,("logging_in"));

                    m_eLoginState = LS_LOGGED_IN;
                    m_sAccountHandle = acct.sHandle;
                    m_sAccountName = acct.sAccountName;
                    m_sLocalURI = acct.sAccountURI;
                    m_sLocalDisplayName = acct.sDisplayName;

                    FIRE_EVENT(VivoxOnLogin,(true));

                    FIRE_EVENT(VivoxOnLoginStateChange,("logged_in"));

                    for ( std::vector< DiagSessionGroup >::const_iterator sgiter( acct.aSessionGroups.begin() );
                          sgiter != acct.aSessionGroups.end();
                          ++sgiter )
                    {
                        const DiagSessionGroup& sg = (*sgiter);

                        GroupInfo& group = m_mGroupInfo[ sg.sHandle ];
                        group.sHandle = sg.sHandle;

                        FIRE_EVENT(VivoxOnSessionGroupAdded,(group.sHandle));

                        Session* pFocusedSession = 0;

                        for ( std::vector< DiagSession >::const_iterator siter( sg.aSessions.begin() );
                              siter != sg.aSessions.end();
                              ++siter )
                        {
                            const DiagSession& s = (*siter);
                            group.sSessions.insert( s.sHandle );

                            SessionInfo& sessionInfo = m_mSessionInfo[ s.sURI ];
                            Session& session = m_mSessions[ s.sHandle ];
                            session.sHandle = s.sHandle;
                            session.sURI = sessionInfo.sURI = s.sURI;
                            session.sDisplayName = sessionInfo.sDisplayName = s.sName;
                            session.sGroupHandle = sessionInfo.sGroup = group.sHandle;
                            // Password for sessionInfo is unknown
                            session.sDisplayName = s.sName;
                            session.eState = s.bIsConnected && s.bHasAudio ? Session::MS_CONNECTED : Session::MS_DISCONNECTED;
                            session.bTextConnected = s.bHasText;
                            session.bTransmitEnabled = s.bTransmitting;
                            session.bFocused = s.bFocused;
                            session.bMuted = session.m_bLocalMute = s.bAudioMutedForMe;
                            session.fVolume = s.fVolume;
                            session.iFont = s.iFont;

                            // Fire the focused session last
                            if ( session.bFocused )
                                pFocusedSession = &session;
                            else
                            {
                                FIRE_EVENT(VivoxOnSessionAdded,(session.sGroupHandle, session.sHandle, session));
                            }

                            for ( std::vector< DiagParticipant >::const_iterator piter( s.aParticipants.begin() );
                                  piter != s.aParticipants.end();
                                  ++piter )
                            {
                                const DiagParticipant& dp = (*piter);
                                Participant& p = session.mParticipants[ dp.sURI ];
                                p.pSession = &session;
                                p.sURI = dp.sURI;
                                if ( p.sURI == m_sLocalURI )
                                {
                                    p.bSelf = true;
                                    session.pSelf = &p;
                                }
                                // Parse account from URI
                                p.sAccount = dp.sAccountName;
                                p.sDisplayName = dp.sDisplayName;
                                p.bModMuted = dp.bAudioModMuted;
                                p.bSpeaking = dp.bSpeaking;
                                p.bPushToTalk = dp.bHandRaised;
                                p.fVolume = p.m_fLocalVolume = dp.fVolume;
                                p.fEnergy = dp.fEnergy;
                                p.m_bLocalMute = dp.bAudioMutedForMe;

                                if ( pFocusedSession != &session )
                                {
                                    FIRE_EVENT(VivoxOnParticipantAdded,(session.sGroupHandle, session.sHandle, p));
                                }
                            }
                        }

                        if ( pFocusedSession )
                        {
                            FIRE_EVENT(VivoxOnSessionAdded, (pFocusedSession->sGroupHandle, pFocusedSession->sHandle, *pFocusedSession));

                            for ( Session::ParticipantMap::const_iterator piter( pFocusedSession->mParticipants.begin() );
                                  piter != pFocusedSession->mParticipants.end();
                                  ++piter )
                            {
                                FIRE_EVENT(VivoxOnParticipantAdded, (pFocusedSession->sGroupHandle, pFocusedSession->sHandle, (*piter).second));
                            }
                        }
                    }
                }

                FIRE_EVENT(VivoxOnJoinExistingSessionEnd,());

                return;
            }
        }
    }

    if ( !bConnecting )
    {
        FIRE_EVENT(VivoxOnEndDiagnosticStateDump,());
    }
    else
    {
        // If we've made it this far then there was no state and connection failed
        m_eConnectState = CS_DISCONNECTED;
        FIRE_EVENT(VivoxOnConnected,(false));
    }
}
#endif

////////////////////////////////////////////////////////////////////////////////
// PARTICIPANT FUNCTIONS
////////////////////////////////////////////////////////////////////////////////

template <typename StrClass, typename Glue>
void
Vivox<StrClass, Glue>::Participant::CopySimple( typename const Vivox<StrClass, Glue>::Participant& from )
{
    // Copy everything but the strings
    pSession       = from.pSession;
    bSelf          = from.bSelf;
    bModerator     = from.bModerator;
    bModMuted      = from.bModMuted;
    bSpeaking      = from.bSpeaking;
    bPushToTalk    = from.bPushToTalk;
    fVolume        = from.fVolume;
    fEnergy        = from.fEnergy;
    m_bLocalMute   = from.m_bLocalMute;
    m_fLocalVolume = from.m_fLocalVolume;
    bIsLocal       = from.bIsLocal;
}

//////////////////////////////////////////////////////////////////////////////////
// Participant::SetLocalMute
// 
// Fires off a vx_req_session_set_participant_mute_for_me request
// Only affects local mute
// This is allowed in Hands-Off mode
//////////////////////////////////////////////////////////////////////////////////
template <typename StrClass, typename Glue>
bool
Vivox<StrClass, Glue>::Participant::SetLocalMute( bool bMute ) const
{
    if ( !LOAD_DLL() || 0 == pSession ) return false;

    // Vivox doesn't like it when you try to set mute for yourself
    if ( pSession->pSelf == this )
        return false;

    vx_req_session_set_participant_mute_for_me* p = 0;
    vx_req_session_set_participant_mute_for_me_create( &p );

    m_bLocalMute = bMute;

    p->mute = m_bLocalMute ? 1 : 0;
    Vivox<StrClass, Glue>::getInstance().m_generateID( p->base );
    p->participant_uri = vx_strdup( sURI.c_str() );
    p->session_handle = vx_strdup( pSession->sHandle.c_str() );

    Glue::log( VLS_DEBUG, "Sending Set Participant Mute For Me request" );
    return Vivox<StrClass, Glue>::getInstance().m_issueRequest( (vx_req_base*)p );
}

//////////////////////////////////////////////////////////////////////////////////
// Participant::SetVolume
// 
// Fires off a vx_req_session_set_participant_volume_for_me request
// f should be in the range 0.0 - 1.0
// Only affects local volume
// This is allowed in Hands-Off mode
//////////////////////////////////////////////////////////////////////////////////
template <typename StrClass, typename Glue>
bool
Vivox<StrClass, Glue>::Participant::SetLocalVolume( float f ) const
{
    if ( !LOAD_DLL() || 0 == pSession ) return false;

    // Vivox doesn't like it when you try to set volume for yourself
    if ( pSession->pSelf == this )
        return false;

    vx_req_session_set_participant_volume_for_me* p = 0;
    vx_req_session_set_participant_volume_for_me_create( &p );

    m_fLocalVolume = VivoxClamp( 0.0f, f, 1.0f );

    p->volume = static_cast< int >( m_fLocalVolume * 100.0f );
    Vivox<StrClass, Glue>::getInstance().m_generateID( p->base );
    p->participant_uri = vx_strdup( sURI.c_str() );
    p->session_handle = vx_strdup( pSession->sHandle.c_str() );

    Glue::log( VLS_DEBUG, "Sending Set Participant Volume For Me request" );
    return Vivox<StrClass, Glue>::getInstance().m_issueRequest( (vx_req_base*)p );
}

//////////////////////////////////////////////////////////////////////////////////
// Participant::SetModeratorFlag
// 
// Sets the moderator flag and, if it changed, fires off a
// VivoxOnParticipantUpdated event
//////////////////////////////////////////////////////////////////////////////////
template <typename StrClass, typename Glue>
void
Vivox<StrClass, Glue>::Participant::SetModeratorFlag( bool b ) const
{
    if ( bModerator != b )
    {
        Participant old;
        old.CopySimple( *this );

        bModerator = b;

        Vivox<StrClass, Glue>::getInstance().m_fireParticipantUpdated( pSession->sGroupHandle, pSession->sHandle, old, *this );
    }
}


////////////////////////////////////////////////////////////////////////////////
// SESSION FUNCTIONS
////////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////////
// Session::SetLocalMute
// 
// Fires off a vx_req_session_mute_local_speaker request
// Only affects local mute
// This is allowed in Hands-Off mode
//////////////////////////////////////////////////////////////////////////////////
template <typename StrClass, typename Glue>
bool
Vivox<StrClass, Glue>::Session::SetLocalMute( bool bMute ) const
{
    if ( !LOAD_DLL() || 0 == pSession ) return false;

    vx_req_session_mute_local_speaker* p = 0;
    vx_req_session_mute_local_speaker_create( &p );

    m_bLocalMute = bMute;

    p->mute_level = m_bLocalMute ? 1 : 0;
    Vivox<StrClass, Glue>::getInstance().m_generateID( p->base );
    p->session_handle = vx_strdup( sHandle.c_str() );

    Glue::log( VLS_DEBUG, "Sending Mute Local Speaker request" );
    return Vivox<StrClass, Glue>::getInstance().m_issueRequest( (vx_req_base*)p );
}

//////////////////////////////////////////////////////////////////////////////////
// Session::SetVolume
// 
// Fires off a vx_req_session_set_local_speaker_volume request
// f should be in the range 0.0 - 1.0
// Only affects local volume
// This is allowed in Hands-Off mode
//////////////////////////////////////////////////////////////////////////////////
template <typename StrClass, typename Glue>
bool
Vivox<StrClass, Glue>::Session::SetLocalVolume( float f ) const
{
    if ( !LOAD_DLL() || 0 == pSession ) return false;

    vx_req_session_set_local_speaker_volume* p = 0;
    vx_req_session_set_local_speaker_volume_create( &p );

    m_fLocalVolume = VivoxClamp( 0.0f, f, 1.0f );

    p->volume = static_cast< int >( m_fLocalVolume * 100.0f );
    Vivox<StrClass, Glue>::getInstance().m_generateID( p->base );
    p->session_handle = vx_strdup( pSession->sHandle.c_str() );

    Glue::log( VLS_DEBUG, "Sending Set Local Speaker Volume request" );
    return Vivox<StrClass, Glue>::getInstance().m_issueRequest( (vx_req_base*)p );
}

#endif

