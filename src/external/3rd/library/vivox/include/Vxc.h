/* Copyright (c) 2007 by Vivox Inc.
 *
 * Permission to use, copy, modify or distribute this software in binary or source form 
 * for any purpose is allowed only under explicit prior consent in writing from Vivox Inc.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND VIVOX DISCLAIMS
 * ALL WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL VIVOX
 * BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL
 * DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR
 * PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS
 * ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS
 * SOFTWARE.
 */

#ifndef __VXC_H__
#define __VXC_H__

#include <time.h>
#include "VxcExports.h"


#define VIVOX_SDK_HAS_CRASH_REPORTING 1
#define VIVOX_SDK_HAS_CLIENT_SIDE_RECORDING 1
#define VIVOX_SDK_HAS_CLIENT_SIDE_RECORDING_V2 1
#define VIVOX_SDK_HAS_VOICE_FONTS 1
#define VIVOX_SDK_HAS_GROUP_IM 1
#define VIVOX_SDK_HAS_MUTE_SCOPE 1
#define VIVOX_SDK_HAS_PARTICIPANT_TYPE 1
#define VIVOX_SDK_HAS_NETWORK_MESSAGE 1
#define VIVOX_SDK_HAS_AUX_DIAGNOSTIC_STATE 1
#define VIVOX_SDK_SESSION_RENDER_AUDIO_OBSOLETE 1
#define VIVOX_SDK_SESSION_GET_LOCAL_AUDIO_INFO_OBSOLETE 1
#define VIVOX_SDK_SESSION_MEDIA_RINGBACK_OBSOLETE 1
#define VIVOX_SDK_SESSION_CONNECT_OBSOLETE 1
#define VIVOX_SDK_SESSION_CHANNEL_GET_PARTICIPANTS_OBSOLETE 1
#define VIVOX_SDK_ACCOUNT_CHANNEL_CREATE_AND_INVITE_OBSOLETE 1
#define VIVOX_SDK_EVT_SESSION_PARTICIPANT_LIST_OBSOLETE 1
#define VIVOX_SDK_HAS_INTEGRATED_PROXY 1
#define VIVOX_SDK_HAS_NO_CHANNEL_FOLDERS 1
#define VIVOX_SDK_HAS_NO_SCORE 1
#define VIVOX_SDK_HAS_GENERIC_APP_NOTIFICATIONS_ONLY 1
#define VIVOX_SDK_HAS_FRAME_TOTALS 1
#define VIVOX_SDK_NO_LEGACY_RECORDING 1

#ifdef __cplusplus
extern "C" {
#endif

    /**
     * Request cookie type. Used when passing in requests to the SDK.
     */
    typedef char* VX_COOKIE;
    /**
     * Generic handle type for state objects (connectors, accounts, session groups, session, etc.
     */
    typedef VX_COOKIE VX_HANDLE;

    typedef unsigned int VX_SDK_HANDLE;

#ifndef VIVOX_TYPES_ONLY
    /** 
     * Used to allocate and initialize a cookie.
     * \ingroup memorymanagement
     */
    VIVOXSDK_DLLEXPORT void vx_cookie_create(const char* value, VX_COOKIE* cookie);
    /**
     * Used to free a cookie
     * \ingroup memorymanagent
     */
    VIVOXSDK_DLLEXPORT void vx_cookie_free(VX_COOKIE* cookie);
#endif

    /**
     * SDK Logging Levels
     */
    typedef enum {
        /**
         * Errors only
         */
        log_error = 0,
        /**
         * Warnings only
         */
        log_warning,
        /**
         * Generic Information
         */
        log_info,
        /**
         * Detailed debugging information. Likely to have performance implications.
         */
        log_debug,
        /**
         * The most verbose logging level. Likely to have performance implications.
         */
        log_trace
    } vx_log_level;

    /**
     * Type of STUN probe to do
     */
    typedef enum {
        /**
         * Use the default as configured on the account management server.
         */
        attempt_stun_unspecified,
        /**
         * Use STUN
         */
        attempt_stun_on,
        /**
         * Don't use STUN
         */
        attempt_stun_off,
    } vx_attempt_stun;

    /**
     * Type of API mode to use.
     * Required setting is connector_mode_normal.
     */
    typedef enum {
        /**
         * The default and only valid value
         */
        connector_mode_normal=0,
        /**
         * This value is deprecated.
         * \deprecated
         */
        connector_mode_legacy,
    } vx_connector_mode;

    /**
     * Type of API mode to use.
     * Recommended setting is connector_mode_normal.
     */
    typedef enum {
        /**
        * Each handle will be unique for the lifetime of the connector
        */
        session_handle_type_unique=0,
        /**
        * Handles will be sequential integers
        */
        session_handle_type_legacy,
    } vx_session_handle_type;

    /**
     * Type of logging for the applicaiton to use.
     * The Vivox SDK is cabable of logging to a native log file and/or sending log information
     * to the client applicaiton via a callback method registered with the SDK.
     */
    typedef enum {
        /**
         * unused 
         */
        log_to_none=0,
        /**
         * Log to the native configured logfile 
         */
        log_to_file,
        /**
         * Send log information to the client applicaiton via the registered callback method
         */
        log_to_callback,
        /**
         * Log to the native configured log file and the client applicaiton via the registered callback method
         */
        log_to_file_and_callback,
    } vx_log_type;
        
    /**
     * Used as run time type indicator for all messages passed between application and SDK.
     */
    typedef enum {
        /**
         * unused 
         */
        msg_none=0,
        /** 
         * Message is a request
         * @see vx_req_base_t
         */
        msg_request=1,
        /** 
         * Message is a response
         * @see vx_resp_base_t
         */
        msg_response,
        /** 
         * Message is an event
         * @see vx_evt_base_t
         */
        msg_event,
    } vx_message_type;

    typedef enum {
        /**
         * Stop a recording
         */
        VX_SESSIONGROUP_RECORDING_CONTROL_STOP = 0,
        /**
         * Start a recording
         */
        VX_SESSIONGROUP_RECORDING_CONTROL_START = 1,
        /**
         * Flush a continuous recording 
         */
        VX_SESSIONGROUP_RECORDING_CONTROL_FLUSH_TO_FILE = 2,
    } vx_sessiongroup_recording_control_type;

    typedef enum {
        /**
         * Stop audio injection
         */
        VX_SESSIONGROUP_AUDIO_INJECTION_CONTROL_STOP = 0,
        /**
         * Start audio injection (only if currently stopped)
         */
        VX_SESSIONGROUP_AUDIO_INJECTION_CONTROL_START = 1,
        /**
         * Restart audio injection (start if currently stopped. Stop if currently injecting, and restart)
         */
        VX_SESSIONGROUP_AUDIO_INJECTION_CONTROL_RESTART = 2,
        VX_SESSIONGROUP_AUDIO_INJECTION_CONTROL_MIN = VX_SESSIONGROUP_AUDIO_INJECTION_CONTROL_STOP,
        VX_SESSIONGROUP_AUDIO_INJECTION_CONTROL_MAX = VX_SESSIONGROUP_AUDIO_INJECTION_CONTROL_RESTART
    } vx_sessiongroup_audio_injection_control_type;


    typedef enum {
        /**
        * Stop playback
        *
        * When playback is stopped, it closes the playback file, and generates a media frame played event
        * with 0 for the first frame and 0 for the total frames.
        */
        VX_SESSIONGROUP_PLAYBACK_CONTROL_STOP = 0,
        /**
        * Start playback
        */
        VX_SESSIONGROUP_PLAYBACK_CONTROL_START = 1,
        /**
        * Pause a playback
        */
        VX_SESSIONGROUP_PLAYBACK_CONTROL_PAUSE = 3,
        /**
        * Unpause playback
        */
        VX_SESSIONGROUP_PLAYBACK_CONTROL_UNPAUSE = 4,
    } vx_sessiongroup_playback_control_type;

    typedef enum {
        /**
        * Normal mode playback
        */
        VX_SESSIONGROUP_PLAYBACK_MODE_NORMAL = 0,
        /**
        * Vox mode playback: Catch-up mode. Skip all silence periods. Playback at desired speed.
        */
        VX_SESSIONGROUP_PLAYBACK_MODE_VOX = 1,
    } vx_sessiongroup_playback_mode;

    typedef enum {
        media_type_none=0,
        media_type_text,
        media_type_audio,
        media_type_video,
        media_type_audiovideo,
    } vx_media_type;

    typedef enum {
        diagnostic_dump_level_all=0,
        diagnostic_dump_level_sessions,
    } vx_diagnostic_dump_level;

    typedef enum {
        media_ringback_none=0,
        media_ringback_ringing=1,    // 180
        //media_ringback_answer=2,     // 200
        media_ringback_busy=3,       // 486
        //media_ringback_terminated=4, // 487
    } vx_media_ringback;

    typedef enum {
        channel_type_normal=0,
        channel_type_positional = 2
    } vx_channel_type;

    typedef enum {
        channel_mode_none=0,
        channel_mode_normal=1,
        channel_mode_presentation=2,
        channel_mode_lecture=3,
        channel_mode_open=4,
        channel_mode_auditorium=5
    } vx_channel_mode;

    typedef enum {
        channel_search_type_all=0,
        channel_search_type_non_positional=1,
        channel_search_type_positional=2
    } vx_channel_search_type;

    typedef enum {
        channel_moderation_type_all=0,
        channel_moderation_type_current_user=1
    } vx_channel_moderation_type;
    
    /** The type of the sessiongroup specified at sessiongroup creation time */
    typedef enum {
        /**
        * Normal type for general use.
        */
        sessiongroup_type_normal=0,
        /**
        * Playback type.  Only use this for playing back a Vivox recording.
        * Live sessions cannot be added to this type of sessiongroup.
        */
        sessiongroup_type_playback=1
    } vx_sessiongroup_type;

    /** The reason why a participant was removed from a session. */
    typedef enum {
        participant_left=0,
        participant_timeout=1,
        participant_kicked=2,
        participant_banned=3
    } vx_participant_removed_reason;

    typedef struct vx_message_base {
        vx_message_type type;
        VX_SDK_HANDLE sdk_handle;
        unsigned long long create_time_ms;
        unsigned long long last_step_ms;
    } vx_message_base_t;

    /** The set of requests that can be issued. */
    typedef enum {
        req_none=0,
        req_connector_create=1,
        req_connector_initiate_shutdown=2,
        req_account_login=3,
        req_account_logout=4,
        req_account_set_login_properties=5,
        req_sessiongroup_create=6,
        req_sessiongroup_terminate=7,
        req_sessiongroup_add_session=8,
        req_sessiongroup_remove_session=9,
        req_sessiongroup_set_focus=10,
        req_sessiongroup_unset_focus=11,
        req_sessiongroup_reset_focus=12,
        req_sessiongroup_set_tx_session=13,
        req_sessiongroup_set_tx_all_sessions=14,
        req_sessiongroup_set_tx_no_session=15,
        req_session_create=16,                  /**< Do Not Use, use req_sessiongroup_add_session */
        req_session_media_connect=18,
        req_session_media_disconnect=19,
        req_session_terminate=21,
        req_session_mute_local_speaker=22,
        req_session_set_local_speaker_volume=23,
        req_session_channel_invite_user=25,
        req_session_set_participant_volume_for_me=26,
        req_session_set_participant_mute_for_me=27,
        req_session_set_3d_position=28,
        req_session_set_voice_font=29,
        req_account_channel_create=34,
        req_account_channel_update=35,
        req_account_channel_delete=36,
        req_account_channel_favorites_get_list=42,
        req_account_channel_favorite_set=43,
        req_account_channel_favorite_delete=44,
        req_account_channel_favorite_group_set=45,
        req_account_channel_favorite_group_delete=46,
        req_account_channel_get_info=47,
        req_account_channel_search=48,
        req_account_buddy_search=49,
        req_account_channel_add_moderator=50,
        req_account_channel_remove_moderator=51,
        req_account_channel_get_moderators=52,
        req_account_channel_add_acl=53,
        req_account_channel_remove_acl=54,
        req_account_channel_get_acl=55,
        req_channel_mute_user=56,
        req_channel_ban_user=57,
        req_channel_get_banned_users=58,
        req_channel_kick_user=59,
        req_channel_mute_all_users=60,
        req_connector_mute_local_mic=61,
        req_connector_mute_local_speaker=62,
        req_connector_set_local_mic_volume=63,
        req_connector_set_local_speaker_volume=64,
        req_connector_get_local_audio_info=65,
        req_account_buddy_set=67,
        req_account_buddy_delete=68,
        req_account_buddygroup_set=69,
        req_account_buddygroup_delete=70,
        req_account_list_buddies_and_groups=71,
        req_session_send_message=72,
        req_account_set_presence=73,
        req_account_send_subscription_reply=74,
        req_session_send_notification=75,
        req_account_create_block_rule=76,
        req_account_delete_block_rule=77,
        req_account_list_block_rules=78,
        req_account_create_auto_accept_rule=79,
        req_account_delete_auto_accept_rule=80,
        req_account_list_auto_accept_rules=81,
        req_account_update_account=82,
        req_account_get_account=83,
        req_account_send_sms=84,
        req_aux_connectivity_info=86,
        req_aux_get_render_devices=87,
        req_aux_get_capture_devices=88,
        req_aux_set_render_device=89,
        req_aux_set_capture_device=90,
        req_aux_get_mic_level=91,
        req_aux_get_speaker_level=92,
        req_aux_set_mic_level=93,
        req_aux_set_speaker_level=94,
        req_aux_render_audio_start=95,
        req_aux_render_audio_stop=96,
        req_aux_capture_audio_start=97,
        req_aux_capture_audio_stop=98,
        req_aux_global_monitor_keyboard_mouse=99,
        req_aux_set_idle_timeout=100,
        req_aux_create_account=101,
        req_aux_reactivate_account=102,
        req_aux_deactivate_account=103,
        req_account_post_crash_dump=104,
        req_aux_reset_password=105,
        req_sessiongroup_set_session_3d_position=106,
        req_account_get_session_fonts=107,
        req_account_get_template_fonts=108,
        req_aux_start_buffer_capture=109,
        req_aux_play_audio_buffer=110,
        req_sessiongroup_control_recording=111,
        req_sessiongroup_control_playback=112,
        req_sessiongroup_set_playback_options=113,
        req_session_text_connect=114,
        req_session_text_disconnect=115,
        req_channel_set_lock_mode=116,
        req_aux_render_audio_modify=117,
        req_session_send_dtmf=118,
        req_aux_set_vad_properties=120,
        req_aux_get_vad_properties=121,
        req_sessiongroup_control_audio_injection=124,
        req_account_channel_change_owner=125,           /**< Not yet implemented (3030) */
        req_account_channel_get_participants=126,       /**< Not yet implemented (3030) */
        req_account_send_user_app_data=128,             /**< Not yet implemented (3030) */
        req_aux_diagnostic_state_dump=129,
        req_account_web_call=130,
        req_account_anonymous_login=131,
        req_max=req_account_anonymous_login+1
    } vx_request_type;

    /** Response types that will be reported back to the calling app. */
    typedef enum {
        resp_none=0,
        resp_connector_create=1,
        resp_connector_initiate_shutdown=2,
        resp_account_login=3,
        resp_account_logout=4,
        resp_account_set_login_properties=5,
        resp_sessiongroup_create=6,
        resp_sessiongroup_terminate=7,
        resp_sessiongroup_add_session=8,
        resp_sessiongroup_remove_session=9,
        resp_sessiongroup_set_focus=10,
        resp_sessiongroup_unset_focus=11,
        resp_sessiongroup_reset_focus=12,
        resp_sessiongroup_set_tx_session=13,
        resp_sessiongroup_set_tx_all_sessions=14,
        resp_sessiongroup_set_tx_no_session=15,
        resp_session_create=16,                 /**< Do Not Use */
        resp_session_media_connect=18,
        resp_session_media_disconnect=19,
        resp_session_terminate=21,
        resp_session_mute_local_speaker=22,
        resp_session_set_local_speaker_volume=23,
        resp_session_channel_invite_user=25,
        resp_session_set_participant_volume_for_me=26,
        resp_session_set_participant_mute_for_me=27,
        resp_session_set_3d_position=28,
        resp_session_set_voice_font=29,
        resp_account_channel_get_list=33,
        resp_account_channel_create=34,
        resp_account_channel_update=35,
        resp_account_channel_delete=36,
        resp_account_channel_favorites_get_list=42,
        resp_account_channel_favorite_set=43,
        resp_account_channel_favorite_delete=44,
        resp_account_channel_favorite_group_set=45,
        resp_account_channel_favorite_group_delete=46,
        resp_account_channel_get_info=47,
        resp_account_channel_search=48,
        resp_account_buddy_search=49,
        resp_account_channel_add_moderator=50,
        resp_account_channel_remove_moderator=51,
        resp_account_channel_get_moderators=52,
        resp_account_channel_add_acl=53,
        resp_account_channel_remove_acl=54,
        resp_account_channel_get_acl=55,
        resp_channel_mute_user=56,
        resp_channel_ban_user=57,
        resp_channel_get_banned_users=58,
        resp_channel_kick_user=59,
        resp_channel_mute_all_users=60,
        resp_connector_mute_local_mic=61,
        resp_connector_mute_local_speaker=62,
        resp_connector_set_local_mic_volume=63,
        resp_connector_set_local_speaker_volume=64,
        resp_connector_get_local_audio_info=65,
        resp_account_buddy_set=67,
        resp_account_buddy_delete=68,
        resp_account_buddygroup_set=69,
        resp_account_buddygroup_delete=70,
        resp_account_list_buddies_and_groups=71,
        resp_session_send_message=72,
        resp_account_set_presence=73,
        resp_account_send_subscription_reply=74,
        resp_session_send_notification=75,
        resp_account_create_block_rule=76,
        resp_account_delete_block_rule=77,
        resp_account_list_block_rules=78,
        resp_account_create_auto_accept_rule=79,
        resp_account_delete_auto_accept_rule=80,
        resp_account_list_auto_accept_rules=81,
        resp_account_update_account=82,
        resp_account_get_account=83,
        resp_account_send_sms=84,
        resp_aux_connectivity_info=86,
        resp_aux_get_render_devices=87,
        resp_aux_get_capture_devices=88,
        resp_aux_set_render_device=89,
        resp_aux_set_capture_device=90,
        resp_aux_get_mic_level=91,
        resp_aux_get_speaker_level=92,
        resp_aux_set_mic_level=93,
        resp_aux_set_speaker_level=94,
        resp_aux_render_audio_start=95,
        resp_aux_render_audio_stop=96,
        resp_aux_capture_audio_start=97,
        resp_aux_capture_audio_stop=98,
        resp_aux_global_monitor_keyboard_mouse=99,
        resp_aux_set_idle_timeout=100,
        resp_aux_create_account=101,
        resp_aux_reactivate_account=102,
        resp_aux_deactivate_account=103,
        resp_account_post_crash_dump=104,
        resp_aux_reset_password=105,
        resp_sessiongroup_set_session_3d_position=106,
        resp_account_get_session_fonts=107,
        resp_account_get_template_fonts=108,
        resp_aux_start_buffer_capture=109,
        resp_aux_play_audio_buffer=110,
        resp_sessiongroup_control_recording=111,
        resp_sessiongroup_control_playback=112,
        resp_sessiongroup_set_playback_options=113,
        resp_session_text_connect=114,
        resp_session_text_disconnect=115,
        resp_channel_set_lock_mode=116,
        resp_aux_render_audio_modify=117,
        resp_session_send_dtmf=118,
        resp_aux_set_vad_properties=120,
        resp_aux_get_vad_properties=121,
        resp_sessiongroup_control_audio_injection=124,
        resp_account_channel_change_owner=125,              /**< Not yet implemented (3030) */
        resp_account_channel_get_participants=126,          /**< Not yet implemented (3030) */
        resp_account_send_user_app_data=128,                /**< Not yet implemented (3030) */
        resp_aux_diagnostic_state_dump=129,                 
        resp_account_web_call=130,
        resp_account_anonymous_login=131,
        resp_max=resp_account_anonymous_login+1
    } vx_response_type;
    
    /** Event types that will be reported back to the calling app. */
    typedef enum {
        evt_none=0,
        evt_account_login_state_change=2,
        evt_buddy_presence=7,
        evt_subscription=8,
        evt_session_notification=9,
        evt_message=10,
        evt_aux_audio_properties=11,
        evt_buddy_changed=15,
        evt_buddy_group_changed=16,
        evt_buddy_and_group_list_changed=17,
        evt_keyboard_mouse=18,
        evt_idle_state_changed=19,
        evt_media_stream_updated=20,
        evt_text_stream_updated=21,
        evt_sessiongroup_added=22,
        evt_sessiongroup_removed=23,
        evt_session_added=24,
        evt_session_removed=25,
        evt_participant_added=26,
        evt_participant_removed=27,
        evt_participant_updated=28,
        evt_sessiongroup_playback_frame_played=30,
        evt_session_updated=31,
        evt_sessiongroup_updated=32,
        evt_media_completion=33,
        evt_server_app_data=35,                 
        evt_user_app_data=36,                   
        evt_network_message=38,
        evt_voice_service_connection_state_changed=39,
        evt_max=evt_voice_service_connection_state_changed+1
    } vx_event_type;
    
    typedef struct vx_req_base {
        vx_message_base_t message;
        vx_request_type type;
        VX_COOKIE cookie;
        void *vcookie;
    } vx_req_base_t;

    typedef struct vx_resp_base {
        vx_message_base_t message;
        vx_response_type type;
        int return_code;
        int status_code;
        char* status_string;
        vx_req_base_t* request;
        char *extended_status_info;
    } vx_resp_base_t;

    typedef struct vx_evt_base {
        vx_message_base_t message;
        vx_event_type type;
        char *extended_status_info;
    } vx_evt_base_t;

    typedef enum {
        ND_E_NO_ERROR = 0,
        ND_E_TEST_NOT_RUN,
        ND_E_NO_INTERFACE,
        ND_E_NO_INTERFACE_WITH_GATEWAY,
        ND_E_NO_INTERFACE_WITH_ROUTE,
        ND_E_TIMEOUT,
        ND_E_CANT_ICMP,
        ND_E_CANT_RESOLVE_VIVOX_UDP_SERVER,
        ND_E_CANT_RESOLVE_ROOT_DNS_SERVER,
        ND_E_CANT_CONVERT_LOCAL_IP_ADDRESS,
        ND_E_CANT_CONTACT_STUN_SERVER_ON_UDP_PORT_3478,
        ND_E_CANT_CREATE_TCP_SOCKET,
        ND_E_CANT_LOAD_ICMP_LIBRARY,
        ND_E_CANT_FIND_SENDECHO2_PROCADDR,
        ND_E_CANT_CONNECT_TO_ECHO_SERVER,
        ND_E_ECHO_SERVER_LOGIN_SEND_FAILED,
        ND_E_ECHO_SERVER_LOGIN_RECV_FAILED,
        ND_E_ECHO_SERVER_LOGIN_RESPONSE_MISSING_STATUS,
        ND_E_ECHO_SERVER_LOGIN_RESPONSE_FAILED_STATUS,
        ND_E_ECHO_SERVER_LOGIN_RESPONSE_MISSING_SESSIONID,
        ND_E_ECHO_SERVER_LOGIN_RESPONSE_MISSING_SIPPORT,
        ND_E_ECHO_SERVER_LOGIN_RESPONSE_MISSING_AUDIORTP,
        ND_E_ECHO_SERVER_LOGIN_RESPONSE_MISSING_AUDIORTCP,
        ND_E_ECHO_SERVER_LOGIN_RESPONSE_MISSING_VIDEORTP,
        ND_E_ECHO_SERVER_LOGIN_RESPONSE_MISSING_VIDEORTCP,
        ND_E_ECHO_SERVER_CANT_ALLOCATE_SIP_SOCKET,
        ND_E_ECHO_SERVER_CANT_ALLOCATE_MEDIA_SOCKET,
        ND_E_ECHO_SERVER_SIP_UDP_SEND_FAILED,
        ND_E_ECHO_SERVER_SIP_UDP_RECV_FAILED,
        ND_E_ECHO_SERVER_SIP_TCP_SEND_FAILED,
        ND_E_ECHO_SERVER_SIP_TCP_RECV_FAILED,
        ND_E_ECHO_SERVER_SIP_NO_UDP_OR_TCP,
        ND_E_ECHO_SERVER_SIP_NO_UDP,
        ND_E_ECHO_SERVER_SIP_NO_TCP,
        ND_E_ECHO_SERVER_SIP_MALFORMED_TCP_PACKET,
        ND_E_ECHO_SERVER_SIP_UDP_DIFFERENT_LENGTH,
        ND_E_ECHO_SERVER_SIP_UDP_DATA_DIFFERENT,
        ND_E_ECHO_SERVER_SIP_TCP_PACKETS_DIFFERENT,
        ND_E_ECHO_SERVER_SIP_TCP_PACKETS_DIFFERENT_SIZE,
        ND_E_ECHO_SERVER_LOGIN_RECV_FAILED_TIMEOUT,
        ND_E_ECHO_SERVER_TCP_SET_ASYNC_FAILED,
        ND_E_ECHO_SERVER_UDP_SET_ASYNC_FAILED,
        ND_E_ECHO_SERVER_CANT_RESOLVE_NAME
    } ND_ERROR;

    typedef enum {
        ND_TEST_LOCATE_INTERFACE,
        ND_TEST_PING_GATEWAY,
        ND_TEST_DNS,
        ND_TEST_STUN,
        ND_TEST_ECHO,
        ND_TEST_ECHO_SIP_FIRST_PORT,
        ND_TEST_ECHO_SIP_FIRST_PORT_INVITE_REQUEST,
        ND_TEST_ECHO_SIP_FIRST_PORT_INVITE_RESPONSE,
        ND_TEST_ECHO_SIP_FIRST_PORT_REGISTER_REQUEST,
        ND_TEST_ECHO_SIP_FIRST_PORT_REGISTER_RESPONSE,
        ND_TEST_ECHO_SIP_SECOND_PORT,
        ND_TEST_ECHO_SIP_SECOND_PORT_INVITE_REQUEST,
        ND_TEST_ECHO_SIP_SECOND_PORT_INVITE_RESPONSE,
        ND_TEST_ECHO_SIP_SECOND_PORT_REGISTER_REQUEST,
        ND_TEST_ECHO_SIP_SECOND_PORT_REGISTER_RESPONSE,
        ND_TEST_ECHO_MEDIA,
        ND_TEST_ECHO_MEDIA_LARGE_PACKET
    } ND_TEST_TYPE;

    /**
    * How incoming calls are handled.  Set at login.
    */
    typedef enum {
        /**
        * Not valid for use.
        */
        mode_none=0,
        /**
        * The incoming call will be automatically connected if a call is not already established.
        */
        mode_auto_answer=1,
        /**
        * Requires the client to explicitly answer the incoming call.
        */
        mode_verify_answer,
    } vx_session_answer_mode;

    typedef enum {
        mode_auto_accept=0,
        mode_auto_add=1,
        mode_block,
        mode_hide,
        mode_application
    } vx_buddy_management_mode;

    typedef enum {
        rule_none=0,
        rule_allow,
        rule_block,
        rule_hide,
    } vx_rule_type;

    typedef enum {
        type_none=0,
        type_root=1,
        type_user=2,
    } vx_font_type;

    typedef enum {
        status_none=0,
        status_free=1,
        status_not_free=2,
    } vx_font_status;

    typedef enum {
        subscription_presence=0,
    } vx_subscription_type;

    typedef enum {
        notification_not_typing = 0,
        notification_typing = 1,
        notification_hand_lowered = 2,
        notification_hand_raised = 3,
        notification_min = notification_not_typing,
        notification_max = notification_hand_raised
    } vx_notification_type;

    /** 
     * \attention Not supported on the PLAYSTATION(R)3 platform
     */
    typedef enum {
        dtmf_0=0,
        dtmf_1=1,
        dtmf_2=2,
        dtmf_3=3,
        dtmf_4=4,
        dtmf_5=5,
        dtmf_6=6,
        dtmf_7=7,
        dtmf_8=8,
        dtmf_9=9,
        dtmf_pound=10,
        dtmf_star=11,
        dtmf_A=12,
        dtmf_B=13,
        dtmf_C=14,
        dtmf_D=15,
        dtmf_max=dtmf_D,
    } vx_dtmf_type;

    typedef enum {
        text_mode_disabled = 0,
        text_mode_enabled,
    } vx_text_mode;

    typedef enum {
        channel_unlock = 0,
        channel_lock,
    } vx_channel_lock_mode;

    typedef enum {
        mute_scope_all = 0,
        mute_scope_audio = 1,
        mute_scope_text = 2,
    } vx_mute_scope;

    /**
    * Holds a recorded audio frame
    */
    typedef enum {
        VX_RECORDING_FRAME_TYPE_DELTA = 0,
        VX_RECORDING_FRAME_TYPE_CONTROL = 1
    } vx_recording_frame_type_t;

    typedef enum {
        op_none=0,
        op_safeupdate=1,
        op_delete,
    } vx_audiosource_operation;

    typedef enum {
        aux_audio_properties_none=0
    } vx_aux_audio_properties_state;

    typedef enum {
        login_state_logged_out=0,
        login_state_logged_in = 1,
        login_state_logging_in = 2,
        login_state_logging_out = 3,
        login_state_resetting = 4,
        login_state_error=100
    } vx_login_state_change_state;

    typedef enum {
        buddy_presence_unknown=0,           /**< OBSOLETE */
        buddy_presence_pending=1,           /**< OBSOLETE */
        buddy_presence_online=2,
        buddy_presence_busy=3,
        buddy_presence_brb=4,
        buddy_presence_away=5,
        buddy_presence_onthephone=6,
        buddy_presence_outtolunch=7,
        buddy_presence_custom=8,            /**< OBSOLETE */
        buddy_presence_online_slc=9,        /**< OBSOLETE */
        buddy_presence_closed=0,            /**< OBSOLETE */
        buddy_presence_offline=0,
    } vx_buddy_presence_state;

    typedef enum {
        session_notification_none=0
    } vx_session_notification_state;

    typedef enum {
        message_none=0
    } vx_message_state;

    typedef enum {
        session_text_disconnected = 0,
        session_text_connected, 
        session_text_connecting,
        session_text_disconnecting
    } vx_session_text_state;

    typedef enum {
        session_media_none = 0,
        session_media_disconnected,
        session_media_connected,
        session_media_ringing,
        session_media_hold,
        session_media_refer, 
        session_media_connecting,
        session_media_disconnecting
    } vx_session_media_state;

    typedef enum {
        participant_user=0,
        part_user=0,        // For backward compatibility
        participant_moderator=1,
        part_moderator=1,   // For backward compatibility
        participant_owner=2,
        part_focus=2,       // For backward compatibility
    } vx_participant_type;

    enum media_codec_type {
        media_codec_type_none = 0,
        media_codec_type_siren14 = 1,
        media_codec_type_pcmu = 2,
        media_codec_type_nm = 3
    };

    typedef enum {
        orientation_default = 0,
        orientation_legacy = 1,
        orientation_vivox = 2
    } orientation_type;

    typedef enum {
        media_completion_type_none = 0,
        aux_buffer_audio_capture = 1,
        aux_buffer_audio_render = 2,
        sessiongroup_audio_injection = 3
    } vx_media_completion_type;

    /**
     * Participant media flags
     */
    #define VX_MEDIA_FLAGS_AUDIO 0x1
    #define VX_MEDIA_FLAGS_TEXT  0x2

    /**
     * Holds a recorded audio frame
     */
    typedef struct vx_recording_frame {
        /**
         * The number of bytes in the frame
         */
        int frame_size;
        /**
         * The frame data
         */
        void *frame_data;
        /**
         * The type
         */
        vx_recording_frame_type_t frame_type;
    } vx_recording_frame_t;
#ifndef VIVOX_TYPES_ONLY
    VIVOXSDK_DLLEXPORT void vx_recording_frame_create(vx_recording_frame_t** frame);
    VIVOXSDK_DLLEXPORT void vx_recording_frame_free(vx_recording_frame_t* frame);
#endif
    typedef vx_recording_frame_t* vx_recording_frame_ref_t;
    typedef vx_recording_frame_ref_t* vx_recording_frame_list_t;
#ifndef VIVOX_TYPES_ONLY
    VIVOXSDK_DLLEXPORT void vx_recording_frame_list_create(int size, vx_recording_frame_list_t* list_out);
    VIVOXSDK_DLLEXPORT void vx_recording_frame_list_free(vx_recording_frame_t** list, int size);
#endif

    /** Channel participant. */
    typedef struct vx_participant {
        char* uri;
        char* first_name;
        char* last_name;
        char* display_name;
        char* username;
        int is_moderator;
        int is_moderator_muted;
        int is_moderator_text_muted;
        int is_muted_for_me;    //NOT CURRENTLY IMPLEMENTED
        int is_owner;
        int account_id;
    } vx_participant_t;
#ifndef VIVOX_TYPES_ONLY
    VIVOXSDK_DLLEXPORT void vx_participant_create(vx_participant_t** participant);
    VIVOXSDK_DLLEXPORT void vx_participant_free(vx_participant_t* participant);
#endif
    /** Creates a participant list with the given size. */
    typedef vx_participant_t* vx_participant_ref_t;
    typedef vx_participant_ref_t* vx_participant_list_t;
#ifndef VIVOX_TYPES_ONLY
    VIVOXSDK_DLLEXPORT void vx_participant_list_create(int size, vx_participant_list_t* list_out);
    VIVOXSDK_DLLEXPORT void vx_participant_list_free(vx_participant_t** list, int size);
#endif

    /** State Participant
    * Used for state dump only
    */
    typedef struct vx_state_participant {
        char* uri;
        char* display_name;
        int is_audio_enabled;
        int is_text_enabled;
        int is_audio_muted_for_me;
        int is_text_muted_for_me;       //Not Currently Supported
        int is_audio_moderator_muted;
        int is_text_moderator_muted;
        int is_hand_raised;
        int is_typing;
        int is_speaking;
        int volume;
        double energy;
        vx_participant_type type;
    } vx_state_participant_t;
#ifndef VIVOX_TYPES_ONLY
    VIVOXSDK_DLLEXPORT void vx_state_participant_create(vx_state_participant_t** state_participant);
    VIVOXSDK_DLLEXPORT void vx_state_participant_free(vx_state_participant_t* state_participant);
#endif
    /** Creates a state_participant list with the given size. */
    typedef vx_state_participant_t* vx_state_participant_ref_t;
    typedef vx_state_participant_ref_t* vx_state_participant_list_t;
#ifndef VIVOX_TYPES_ONLY
    VIVOXSDK_DLLEXPORT void vx_state_participant_list_create(int size, vx_state_participant_list_t* list_out);
    VIVOXSDK_DLLEXPORT void vx_state_participant_list_free(vx_state_participant_t** list, int size);
#endif

    /** State Session
    * Used for state dump only
    */
    typedef struct vx_state_session {
        char* session_handle;
        char* uri;
        char* name;
        int is_audio_muted_for_me;
        int is_text_muted_for_me;       //Not Currently Supported
        int is_transmitting;
        int is_focused;
        int volume;
        int session_font_id;
        int has_audio;
        int has_text;
        int is_incoming;
        int is_positional;
        int is_connected;
        //speaker_position;     TODO
        int state_participant_count;
        vx_state_participant_t** state_participants;
    } vx_state_session_t;
#ifndef VIVOX_TYPES_ONLY
    VIVOXSDK_DLLEXPORT void vx_state_session_create(vx_state_session_t** state_session);
    VIVOXSDK_DLLEXPORT void vx_state_session_free(vx_state_session_t* state_session);
#endif
    /** Creates a state_session list with the given size. */
    typedef vx_state_session_t* vx_state_session_ref_t;
    typedef vx_state_session_ref_t* vx_state_session_list_t;
#ifndef VIVOX_TYPES_ONLY
    VIVOXSDK_DLLEXPORT void vx_state_session_list_create(int size, vx_state_session_list_t* list_out);
    VIVOXSDK_DLLEXPORT void vx_state_session_list_free(vx_state_session_t** list, int size);
#endif

    /** State SessionGroup
    * Used for state dump only
    */
    typedef struct vx_state_sessiongroup {
        char* sessiongroup_handle;
        int state_sessions_count;
        vx_state_session_t** state_sessions;
    } vx_state_sessiongroup_t;
#ifndef VIVOX_TYPES_ONLY
    VIVOXSDK_DLLEXPORT void vx_state_sessiongroup_create(vx_state_sessiongroup_t** state_sessiongroup);
    VIVOXSDK_DLLEXPORT void vx_state_sessiongroup_free(vx_state_sessiongroup_t* state_sessiongroup);
#endif
    /** Creates a state_sessiongroup list with the given size. */
    typedef vx_state_sessiongroup_t* vx_state_sessiongroup_ref_t;
    typedef vx_state_sessiongroup_ref_t* vx_state_sessiongroup_list_t;
#ifndef VIVOX_TYPES_ONLY
    VIVOXSDK_DLLEXPORT void vx_state_sessiongroup_list_create(int size, vx_state_sessiongroup_list_t* list_out);
    VIVOXSDK_DLLEXPORT void vx_state_sessiongroup_list_free(vx_state_sessiongroup_t** list, int size);
#endif

    /** State Account
    * Used for state dump only
    */
    typedef struct vx_state_account {
        char* account_handle;
        char* account_uri;
        char* display_name;
        int state_sessiongroups_count;
        vx_state_sessiongroup_t** state_sessiongroups;
    } vx_state_account_t;
#ifndef VIVOX_TYPES_ONLY
    VIVOXSDK_DLLEXPORT void vx_state_account_create(vx_state_account_t** state_account);
    VIVOXSDK_DLLEXPORT void vx_state_account_free(vx_state_account_t* state_account);
#endif
    /** Creates a state_account list with the given size. */
    typedef vx_state_account_t* vx_state_account_ref_t;
    typedef vx_state_account_ref_t* vx_state_account_list_t;
#ifndef VIVOX_TYPES_ONLY
    VIVOXSDK_DLLEXPORT void vx_state_account_list_create(int size, vx_state_account_list_t* list_out);
    VIVOXSDK_DLLEXPORT void vx_state_account_list_free(vx_state_account_t** list, int size);
#endif

    /** State Connector
    * Used for state dump only
    */
    typedef struct vx_state_connector {
        char* connector_handle;
        int state_accounts_count;
        vx_state_account_t** state_accounts;
        int mic_vol;
        int mic_mute;
        int speaker_vol;
        int speaker_mute;
    } vx_state_connector_t;
#ifndef VIVOX_TYPES_ONLY
    VIVOXSDK_DLLEXPORT void vx_state_connector_create(vx_state_connector_t** state_connector);
    VIVOXSDK_DLLEXPORT void vx_state_connector_free(vx_state_connector_t* state_connector);
#endif
    /** Creates a state_connector list with the given size. */
    typedef vx_state_connector_t* vx_state_connector_ref_t;
    typedef vx_state_connector_ref_t* vx_state_connector_list_t;
#ifndef VIVOX_TYPES_ONLY
    VIVOXSDK_DLLEXPORT void vx_state_connector_list_create(int size, vx_state_connector_list_t* list_out);
    VIVOXSDK_DLLEXPORT void vx_state_connector_list_free(vx_state_connector_t** list, int size);
#endif

    /** 
      * Channel struct. 
      */
    typedef struct vx_channel {
        /**
         * The name of the channel
         */
        char* channel_name;
        /**
         * channel_desc: The description of the channel
         */
        char* channel_desc;
        /**
         * Not currently implemented
         */
        char* host;
        /**
         * channel_id: The numeric identifier of the channel
         */
        int channel_id;
        /**
         * limit: The maximum number of participants allowed in the channel
         */
        int limit;
        /**
         * DEPRECATED.  capacity: The forecasted number of participants in the channel.
         */
        int capacity;     /**< DEPRECATED */
        /**
         * modified: The date and time the channel modified
         */
        char* modified;
        /**
         * owner: The uri of the channel owner
         */
        char* owner;
        /**
         * owner_user_name: The user name of the channel owner
         */
        char* owner_user_name;
        /**
         * is_persistent: Flag identifying this channel as persistent or not.  If it is not persistent then it will be deleted automatically after a certain period of inactivity
         */
        int is_persistent; /* 1 true, <= 0 false */
        /**
         * is_protected: A flag identifying this channel as being password protected or not
         */
        int is_protected; /* 1 true, <= 0 false */
        /**
         * size: The number of participants in the channel
         */
        int size;
        /**
         * type: This identifies this as a channel (0), positional(2)
         */
        int type;
        /**
         * mode: The mode of the channel is none (0), normal (1), presentation (2), lecture (3), open (4)
         */
        vx_channel_mode mode;
        /**
         * channel_uri: The URI of the channel, this is used to join the channel as well as perform moderator actions against the channel
         */
        char* channel_uri;
        /**
         * This is the distance beyond which a participant is considered 'out of range'. When participants cross this threshold distance from a particular 
         * listening position in a positional channel, a roster update event is fired, which results in an entry being added (or removed, as the case may be) 
         * from the user's speakers list. No audio is received for participants beyond this range. The default channel value of this parameter is 60..
         * This will use server defaults on create, and will leave existing values unchanged on update
         */
        int max_range;
        /**
         * This is the distance from the listener below which the 'gain rolloff' effects for a given audio rolloff model (see below) are not applied. 
         * In effect, it is the 'audio plateau' distance (in the sense that the gain is constant up this distance, and then falls off). 
         * The default value of this channel parameter is 3.  This will use server defaults on create, and will leave existing values unchanged on update.
         */
        int clamping_dist;
        /**
         * This value indicates how sharp the audio attenuation will 'rolloff' between the clamping and maximum distances. 
         * Larger values will result in steeper rolloffs. The extent of rolloff will depend on the distance model chosen. 
         * Default value is 1.1. This will use server defaults on create, and will leave existing values unchanged on update.
         */
        double roll_off;
        /**
         * The (render side) loudness for all speakers in this channel. Note that this is a receive side value, and should not in practice be raised above, say 2.5. 
         * The default value is 1.7. This will use server defaults on create, and will leave existing values unchanged on update.
         */
        double max_gain;
        /**
         * There are four possible values in this field:
         *    0 - None: No distance based attenuation is applied. All speakers are rendered as if they were in the same position as the listener. 
         *   The audio from speakers will drop to 0 abruptly at the maximum distance.
         *    1 - Inverse Distance Clamped: The attenuation increases in inverse proportion to the distance. The rolloff factor n is the inverse of the slope of the attenuation curve. 
         *    2 -Linear Distance Clamped: The attenuation increases in linear proportion to the distance.The rolloff factor is the negative slope of the attenuation curve.
         *    3 -Exponent Distance Clamped: The attenuation increases in inverse proportion to the distance raised to the power of the rolloff factor. 
         * The default audio model is 1- Inverse Distance Clamped. This will use server defaults on create, and will leave existing values unchanged on update.
         */
        int dist_model;
        /** 
         * encrypt_audio: Whether or not the audio is encrypted
         */
        int encrypt_audio;
        /**
         * owner_display_name: The display name of the channel owner
         */
        char* owner_display_name;
        /**
         * active_participants: The number of participants in the channel
         */
        int active_participants;
    } vx_channel_t;
#ifndef VIVOX_TYPES_ONLY
    VIVOXSDK_DLLEXPORT void vx_channel_create(vx_channel_t** channel);
    VIVOXSDK_DLLEXPORT void vx_channel_free(vx_channel_t* channel);
#endif

    typedef vx_channel_t* vx_channel_ref_t;
    typedef vx_channel_ref_t* vx_channel_list_t;
#ifndef VIVOX_TYPES_ONLY
    VIVOXSDK_DLLEXPORT void vx_channel_list_create(int size, vx_channel_list_t* list_out);
    VIVOXSDK_DLLEXPORT void vx_channel_list_free(vx_channel_t** list, int size);
#endif
    /** Channel Favorite struct. */
    typedef struct vx_channel_favorite {
        int favorite_id;
        int favorite_group_id;
        char* favorite_display_name;
        char* favorite_data;
        char* channel_uri;
        char* channel_description;
        int channel_limit;
        int channel_capacity;     /**< DEPRECATED */
        char* channel_modified;
        char* channel_owner_user_name;
        int channel_is_persistent; /* 1 true, <= 0 false */
        int channel_is_protected; /* 1 true, <= 0 false */
        int channel_size;
        char* channel_owner;
        char* channel_owner_display_name;
        int channel_active_participants;
    } vx_channel_favorite_t;
#ifndef VIVOX_TYPES_ONLY
    VIVOXSDK_DLLEXPORT void vx_channel_favorite_create(vx_channel_favorite_t** channel);
    VIVOXSDK_DLLEXPORT void vx_channel_favorite_free(vx_channel_favorite_t* channel);
#endif
    typedef vx_channel_favorite_t* vx_channel_favorite_ref_t;
    typedef vx_channel_favorite_ref_t* vx_channel_favorite_list_t;
#ifndef VIVOX_TYPES_ONLY
    VIVOXSDK_DLLEXPORT void vx_channel_favorite_list_create(int size, vx_channel_favorite_list_t* list_out);
    VIVOXSDK_DLLEXPORT void vx_channel_favorite_list_free(vx_channel_favorite_t** list, int size);
#endif
    /** Channel Favorite Group struct. */
    typedef struct vx_channel_favorite_group {
        int favorite_group_id;
        char* favorite_group_name;
        char* favorite_group_data;
        char* favorite_group_modified;
    } vx_channel_favorite_group_t;
#ifndef VIVOX_TYPES_ONLY
    VIVOXSDK_DLLEXPORT void vx_channel_favorite_group_create(vx_channel_favorite_group_t** channel);
    VIVOXSDK_DLLEXPORT void vx_channel_favorite_group_free(vx_channel_favorite_group_t* channel);
#endif
    typedef vx_channel_favorite_group_t* vx_channel_favorite_group_ref_t;
    typedef vx_channel_favorite_group_ref_t* vx_channel_favorite_group_list_t;
#ifndef VIVOX_TYPES_ONLY
    VIVOXSDK_DLLEXPORT void vx_channel_favorite_group_list_create(int size, vx_channel_favorite_group_list_t* list_out);
    VIVOXSDK_DLLEXPORT void vx_channel_favorite_group_list_free(vx_channel_favorite_group_t** list, int size);
#endif
    /** Voice Font struct. */
    typedef struct vx_voice_font {
        int id;
        int parent_id;
        vx_font_type type;
        char* name;
        char* description;
        char* expiration_date;
        int expired;        //0 is false, 1 is true
        char* font_delta;
        char* font_rules;
        vx_font_status status;
    } vx_voice_font_t;
#ifndef VIVOX_TYPES_ONLY
    VIVOXSDK_DLLEXPORT void vx_voice_font_create(vx_voice_font_t** channel);
    VIVOXSDK_DLLEXPORT void vx_voice_font_free(vx_voice_font_t* channel);
#endif
    typedef vx_voice_font_t* vx_voice_font_ref_t;
    typedef vx_voice_font_ref_t* vx_voice_font_list_t;
#ifndef VIVOX_TYPES_ONLY
    VIVOXSDK_DLLEXPORT void vx_voice_font_list_create(int size, vx_voice_font_list_t* list_out);
    VIVOXSDK_DLLEXPORT void vx_voice_font_list_free(vx_voice_font_t** list, int size);
    VIVOXSDK_DLLEXPORT void vx_string_list_create(int size, char *** list_out);
    VIVOXSDK_DLLEXPORT void vx_string_list_free(char ** list);
#endif

    typedef struct vx_block_rule {
        char* block_mask;
        int presence_only;
    } vx_block_rule_t;
#ifndef VIVOX_TYPES_ONLY
    VIVOXSDK_DLLEXPORT void vx_block_rule_create(vx_block_rule_t** block_rule);
    VIVOXSDK_DLLEXPORT void vx_block_rule_free(vx_block_rule_t* block_rule);
#endif

    typedef vx_block_rule_t* vx_block_rule_ref_t;
    typedef vx_block_rule_ref_t* vx_block_rules_t;
#ifndef VIVOX_TYPES_ONLY
    VIVOXSDK_DLLEXPORT void vx_block_rules_create(int size, vx_block_rules_t* list_out);
    VIVOXSDK_DLLEXPORT void vx_block_rules_free(vx_block_rule_t** list, int size);
#endif

    typedef struct vx_auto_accept_rule {
        char* auto_accept_mask;
        int auto_add_as_buddy;
        char* auto_accept_nickname;
    } vx_auto_accept_rule_t;
#ifndef VIVOX_TYPES_ONLY
    VIVOXSDK_DLLEXPORT void vx_auto_accept_rule_create(vx_auto_accept_rule_t** auto_accept_rule);
    VIVOXSDK_DLLEXPORT void vx_auto_accept_rule_free(vx_auto_accept_rule_t* auto_accept_rule);
#endif

    typedef vx_auto_accept_rule_t* vx_auto_accept_rule_ref_t;
    typedef vx_auto_accept_rule_ref_t* vx_auto_accept_rules_t;
#ifndef VIVOX_TYPES_ONLY
    VIVOXSDK_DLLEXPORT void vx_auto_accept_rules_create(int size, vx_auto_accept_rules_t* list_out);
    VIVOXSDK_DLLEXPORT void vx_auto_accept_rules_free(vx_auto_accept_rule_t** list, int size);
#endif

typedef struct vx_user_channel {
        char* uri;
        char* name;
    } vx_user_channel_t;
#ifndef VIVOX_TYPES_ONLY
    VIVOXSDK_DLLEXPORT void vx_user_channel_create(vx_user_channel_t** user_channel);
    VIVOXSDK_DLLEXPORT void vx_user_channel_free(vx_user_channel_t* user_channel);
#endif

    typedef vx_user_channel_t* vx_user_channel_ref_t;
    typedef vx_user_channel_ref_t* vx_user_channels_t;
#ifndef VIVOX_TYPES_ONLY
    VIVOXSDK_DLLEXPORT void vx_user_channels_create(int size, vx_user_channels_t* list_out);
    VIVOXSDK_DLLEXPORT void vx_user_channels_free(vx_user_channel_t** list, int size);
#endif

    /**
    * test_type: Enumeration that defines the test performed (see appendix ?15.8 for values).
    * error_code: Enumeration that defines the error or success of the test (see appendix ?15.9 for values).
    * test_additional_info: Any additional info for this test.  This may be IP addresses used, port numbers, error information, etc
    */
    typedef struct vx_connectivity_test_result {
        ND_TEST_TYPE test_type;
        ND_ERROR test_error_code;
        char* test_additional_info;
    } vx_connectivity_test_result_t;
#ifndef VIVOX_TYPES_ONLY
    VIVOXSDK_DLLEXPORT void vx_connectivity_test_result_create(vx_connectivity_test_result_t** connectivity_test_result, ND_TEST_TYPE tt);
    VIVOXSDK_DLLEXPORT void vx_connectivity_test_result_free(vx_connectivity_test_result_t* connectivity_test_result);
#endif
    typedef vx_connectivity_test_result_t* vx_connectivity_test_result_ref_t;
    typedef vx_connectivity_test_result_ref_t* vx_connectivity_test_results_t;
#ifndef VIVOX_TYPES_ONLY
    VIVOXSDK_DLLEXPORT void vx_connectivity_test_results_create(int size, vx_connectivity_test_results_t* list_out);
    VIVOXSDK_DLLEXPORT void vx_connectivity_test_results_free(vx_connectivity_test_result_t** list, int size);
#endif
    typedef struct vx_account {
        char* uri;
        char* firstname;
        char* lastname;
        char* username;
        char* displayname;
        char* email;
        char* phone;
        char* carrier;      //Not currently implemented
        char* created_date;
    } vx_account_t;
#ifndef VIVOX_TYPES_ONLY
    VIVOXSDK_DLLEXPORT void vx_account_create(vx_account_t** account);
    VIVOXSDK_DLLEXPORT void vx_account_free(vx_account_t* account);
#endif

    /**
     * The type of device. 
     */
    typedef enum vx_device_type {
        /**
         * This type is a specific device.
         */
        vx_device_type_specific_device = 0,
        /**
         * This type means to use what ever the system has configured as a default, at the time of the call.
         * Don't switch devices mid-call if the default system device changes.
         */
        vx_device_type_default_system = 1,
        /**
         * This is the null device, which means that either input or output from/to that device will not occur.
         */
        vx_device_type_null = 2
    } vx_device_type_t;

    typedef struct vx_device {
        /**
         * The identifier to passed to vx_req_set_render_device or vx_req_set_capture_device
         */
        char* device;
        /**
         * The display name to present to the user
         */
        char* display_name;
        /**
         * The type of device
         */
        vx_device_type_t device_type;
    } vx_device_t;

#ifndef VIVOX_TYPES_ONLY
    VIVOXSDK_DLLEXPORT void vx_device_create(vx_device_t** device);
    VIVOXSDK_DLLEXPORT void vx_device_free(vx_device_t* device);
#endif
    typedef vx_device_t* vx_device_ref_t;
    typedef vx_device_ref_t* vx_devices_t;
#ifndef VIVOX_TYPES_ONLY
    VIVOXSDK_DLLEXPORT void vx_devices_create(int size, vx_devices_t* list_out);
    VIVOXSDK_DLLEXPORT void vx_devices_free(vx_device_t** list, int size);
#endif
    typedef struct vx_buddy {
        char* buddy_uri;
        char* display_name;
        int parent_group_id;
        char* buddy_data;
        int account_id;
        char* account_name;
    } vx_buddy_t;
#ifndef VIVOX_TYPES_ONLY
    VIVOXSDK_DLLEXPORT void vx_buddy_create(vx_buddy_t** buddy);
    VIVOXSDK_DLLEXPORT void vx_buddy_free(vx_buddy_t* buddy);
#endif
    typedef vx_buddy_t* vx_buddy_ref_t;
    typedef vx_buddy_ref_t* vx_buddy_list_t;
#ifndef VIVOX_TYPES_ONLY
    VIVOXSDK_DLLEXPORT void vx_buddy_list_create(int size, vx_buddy_list_t* list_out);
    VIVOXSDK_DLLEXPORT void vx_buddy_list_free(vx_buddy_t** list, int size);
#endif
    typedef struct vx_group {
        int group_id;
        char* group_name;
        char* group_data;
    } vx_group_t;
#ifndef VIVOX_TYPES_ONLY
    VIVOXSDK_DLLEXPORT void vx_group_create(vx_group_t** group);
    VIVOXSDK_DLLEXPORT void vx_group_free(vx_group_t* group);
#endif
    typedef vx_group_t* vx_group_ref_t;
    typedef vx_group_ref_t* vx_group_list_t;
#ifndef VIVOX_TYPES_ONLY
    VIVOXSDK_DLLEXPORT void vx_group_list_create(int size, vx_group_list_t* list_out);
    VIVOXSDK_DLLEXPORT void vx_group_list_free(vx_group_t** list, int size);
#endif
    typedef struct vx_name_value_pair {
        /**
         * The name of the parameter
         */
        char* name;
        /**
         * The value of teh parameter
         */
        char* value;
    } vx_name_value_pair_t;

#ifndef VIVOX_TYPES_ONLY
    VIVOXSDK_DLLEXPORT void vx_name_value_pair_create(vx_name_value_pair_t** nvpair);
    VIVOXSDK_DLLEXPORT void vx_name_value_pair_free(vx_name_value_pair_t* nvpair);
#endif
    typedef vx_name_value_pair_t* vx_name_value_pair_ref_t;
    typedef vx_name_value_pair_ref_t* vx_name_value_pairs_t;
#ifndef VIVOX_TYPES_ONLY
    VIVOXSDK_DLLEXPORT void vx_name_value_pairs_create(int size, vx_name_value_pairs_t* list_out);
    VIVOXSDK_DLLEXPORT void vx_name_value_pairs_free(vx_name_value_pair_t** list, int size);
#endif

    /* Vivox SDK functions */
#ifndef VIVOX_TYPES_ONLY
    /**
     * Use this function to allocate string data to send to the SDK.
     * \ingroup memorymanagement
     */
    VIVOXSDK_DLLEXPORT char* vx_strdup(const char*);
    /**
     * Use this function to free string data returned to the application.
     * This funciton is rarely used in practice.
     * \ingroup memorymanagement
     */
    VIVOXSDK_DLLEXPORT void vx_free(char*);

    /**
     * The VxSDK polling function.  Should be called periodically to check for any incoming events.  
     *
     * @param message           [out] The object containing the message data.
     * @return                  Status of the poll, 0 = Success, 1 = Failure, -1 = No Mesasge Available
     * \ingroup messaging
     */
    VIVOXSDK_DLLEXPORT int vx_get_message(vx_message_base_t** message);

    /** 
     * Execute the given request. 
     * 
     * @param request           The request object to execute.  This is of one of the vx_req_* structs. 
     * @return                  Success status of the request.
     * \ingroup messaging
     */
    VIVOXSDK_DLLEXPORT int vx_issue_request(vx_req_base_t* request);
    
    /**
     * Get the XML for the given request.
     *
     * @param request           The request object.
     * @param xml               [out] The xml string.
     * @return                  XML string.
     * \ingroup xml
     */
     VIVOXSDK_DLLEXPORT void vx_request_to_xml(void* request, char** xml);
    
    /**
     * Get a request for the given XML string.
     * 
     * @param xml               XML string.
     * @param request           [out] The request struct.
     * @param error             [out] XML parse error string (if any error occurs).  NULL otherwise.
     * @return                  The request struct type.  req_none is returned if no struct could be created from the XML.
     * \ingroup xml
     */
    VIVOXSDK_DLLEXPORT vx_request_type vx_xml_to_request(const char* xml, void** request, char** error);
    
    /**
     * Get the XML for the given response.
     *
     * @param response           The response object.
     * @param xml               [out] The xml string.
     * \ingroup xml
     */
    VIVOXSDK_DLLEXPORT void vx_response_to_xml(void* response, char** xml);
    
    /**
     * Get a response for the given XML string.
     * 
     * @param xml               XML string.
     * @param response          [out] The response struct.
     * @param error             [out] XML parse error string (if any error occurs).  NULL otherwise.
     * @return                  The response struct type.  resp_none is returned if no struct could be created from the XML.
     * \ingroup xml
     */
    VIVOXSDK_DLLEXPORT vx_response_type vx_xml_to_response(const char* xml, void** response, char** error);
    
    /**
     * Get the XML for the given event.
     *
     * @param event           The event object.
     * @param xml               [out] The xml string.
     * \ingroup xml
     */
    VIVOXSDK_DLLEXPORT void vx_event_to_xml(void* event, char** xml);
    
    /**
     * Get a event for the given XML string.
     * 
     * @param xml               XML string.
     * @param event          [out] The event struct.
     * @param error             [out] XML parse error string (if any error occurs).  NULL otherwise.
     * @return                  The event struct type.  req_none is returned if no struct could be created from the XML.
     * \ingroup xml
     */
    VIVOXSDK_DLLEXPORT vx_event_type vx_xml_to_event(const char* xml, void** event, char** error);
    
    /**
     * Determine whether the XML refers to a request, response, or event.
     * \ingroup xml
     */
    VIVOXSDK_DLLEXPORT vx_message_type vx_get_message_type(const char* xml);

    /**
     * Get Millisecond Counter
     */
    VIVOXSDK_DLLEXPORT unsigned long long vx_get_time_ms();

    /**
     * Register a callback that will be called when a message is placed on the queue.
     * The application should use this to signal the main application thread that will then wakeup and call vx_get_message;
     * \ingroup messaging
     */
    VIVOXSDK_DLLEXPORT void vx_register_message_notification_handler(void (* pf_handler)(void *), void *cookie);

    /**
     * Unregister a notification handler
     * \ingroup messaging
     */
    VIVOXSDK_DLLEXPORT void vx_unregister_message_notification_handler(void (* pf_handler)(void *), void *cookie);

    /**
     * Block the caller until a message is available.
     * Returns NULL if no message was available within the allotted time.
     * \ingroup messaging
     */
    VIVOXSDK_DLLEXPORT vx_message_base_t *vx_wait_for_message(int msTimeout);

    /**
     * 
     * Register a callback that will be called to initialize logging
     * The application should use this to signal the main application thread that will then wakeup and call vx_get_message;
     * \ingroup diagnostics
     */
    VIVOXSDK_DLLEXPORT void vx_register_logging_initialization(vx_log_type log_type, 
                                                                const char* log_folder, 
                                                                const char* log_filename_prefix,
                                                                const char* log_filename_suffix,
                                                                int log_level,
                                                                void (* pf_handler)(const char* source, const char* level, const char* message));

    /**
     * Unregister the logging callback notification handler.  The parameters are reserved for future use, please
     * pass NULL for each of the paramters (ex: vx_unregister_logging_handler(0, 0);).  If a logging handler is registered then it must be unregistered
     * before shutting down the SDK.
     * \ingroup diagnostics
     */
    VIVOXSDK_DLLEXPORT void vx_unregister_logging_handler(void (* pf_handler)(void *), void *cookie);

    VIVOXSDK_DLLEXPORT int vx_create_account(const char* acct_mgmt_server, const char* admin_name, const char* admin_pw, const char* uname, const char* pw);

    /**
     * The number of crash dumps stored on disk
     * \ingroup diagnostics
     *
     * \attention Not supported on the PLAYSTATION(R)3 platform
     */
    VIVOXSDK_DLLEXPORT int vx_get_crash_dump_count();

    /**
     * Enable crash dump generation
     * \ingroup diagnostics
     *
     * \attention Not supported on the PLAYSTATION(R)3 platform
     */
    VIVOXSDK_DLLEXPORT void vx_set_crash_dump_generation_enabled(int value);

    /**
     * Determine if crash dump generation is enabled
     * \ingroup diagnostics
     *
     * \attention Not supported on the PLAYSTATION(R)3 platform
     */
    VIVOXSDK_DLLEXPORT int vx_get_crash_dump_generation();

    /**
     * Get the base64 encoded crash dump information
     * \ingroup diagnostics
     *
     * \attention Not supported on the PLAYSTATION(R)3 platform
     */
    VIVOXSDK_DLLEXPORT char *vx_read_crash_dump(int index);

    /**
     * Get the timestamp of a crash
     * \ingroup diagnostics
     *
     * \attention Not supported on the PLAYSTATION(R)3 platform
     */
    VIVOXSDK_DLLEXPORT time_t vx_get_crash_dump_timestamp(int index);

    /**
     * Delete the crash dump
     * \ingroup diagnostics
     *
     * \attention Not supported on the PLAYSTATION(R)3 platform
     */
    VIVOXSDK_DLLEXPORT int vx_delete_crash_dump(int index);

    /**
     * A Vivox Voice Recording File
     */
    typedef void *vxr_file_t;
    /**
     * Opens a vxr file, returning the file handle in *pFile.
     * return 0 on success, non-zero on error
     * mode can be "r" for reading, "w" for writing, or "a" for appending.
     * \ingroup fileio
     */
    VIVOXSDK_DLLEXPORT int vx_vxr_file_open(const char *filename, const char *mode, vxr_file_t *pFile);
    /**
     * Closes a vxr file
     * \ingroup fileio
     */
    VIVOXSDK_DLLEXPORT int vx_vxr_file_close(vxr_file_t file);
    /**
     * Gets the frame count for the file
     * \ingroup fileio
     */
    VIVOXSDK_DLLEXPORT int vx_vxr_file_get_frame_count(vxr_file_t pFile, int *pFrameCount);
    /**
     * Change the file position. 
     * framenumber is the absolute framenumber
     * \ingroup fileio
     */
    VIVOXSDK_DLLEXPORT int vx_vxr_file_move_to_frame(vxr_file_t pFile, int frameNumber);
    /**
     * reads a frame from a file, and moves to the next frame
     * if this function returns VX_E_BUFSIZE, then the buffer passed in is too small. The required size will be in *outsize, and the type will be in *outtype
     * \ingroup fileio
     */
    VIVOXSDK_DLLEXPORT int vx_vxr_file_read_frame(vxr_file_t pFile, void *buffer, int insize, int *outsize, vx_recording_frame_type_t *outtype);
    /**
     * writes a frame to a file
     * \ingroup fileio
     */
    VIVOXSDK_DLLEXPORT int vx_vxr_file_write_frame(vxr_file_t pFile, vx_recording_frame_type_t type, const void *buffer, int insize);


    /** 
     * The application should call this routine just before it exits. 
     * \ingroup memorymanagement
     */
    VIVOXSDK_DLLEXPORT void vx_on_application_exit();

    /**
     * Get the SDK Version info
     * \ingroup diagnostics
     */
    VIVOXSDK_DLLEXPORT const char *vx_get_sdk_version_info();

    /** 
     * Apply a vivox voice font to a wav file
     * 
     * @param fontDefinition - string containing the font "definition" (in XML format)
     * @param inputFile - string contaning path to the input wav file, contaning the "unmodified" voice
     * @param outputFile - string containing path to the output wav file, with font applied
     * @return                  0 if successful, non-zero if failed.
     * \ingroup voicefonts
     */
    VIVOXSDK_DLLEXPORT int vx_apply_font_to_file(const char *fontDefinition, const char *inputFile, const char *outputFile);

    /** 
     * Create a copy of the internal local audio buffer (associated with the vx_req_aux_start_buffer_capture_t request/response)
     * 
     * @param audioBufferPtr - void pointer (should be passed in uninitialized)
     * @return               - No return value is provided. However, on success the audioBufferPtr will points to a copy of the internal audio buffer, otherwise audioBufferPtr is set to NULL
     * \see vx_req_aux_start_buffer_capture
     * \ingroup adi
     */
    VIVOXSDK_DLLEXPORT void* vx_copy_audioBuffer(void *audioBufferPtr);

    /** 
     * Frees up all memory associated with an allocated vivox audioBufferPtr (generated by the vx_copy_audioBuffer() call)
     * 
     * @param audioBufferPtr - Pointer to audio data in vivox proprietary format
     * @return               - No return value is provided
     * \ingroup adi
     */
    VIVOXSDK_DLLEXPORT void vx_free_audioBuffer(void *audioBufferPtr);

    /** 
     * Export audio data in an audioBufferPtr to a wav file
     * 
     * @param audioBufferPtr - Pointer to audio data in vivox proprietary format
     * @param outputFile     - string containing path to the output wav file
     * @return               -  0 if successful, non-zero if failed.
     * \ingroup adi
     */
    VIVOXSDK_DLLEXPORT int vx_export_audioBuffer_to_wav_file(void *audioBufferPtr, const char *outputFile);

    /** 
     * Set the default out of proc server address. Once set, requests issued using vx_issue_request will be sent
     * to the server at supplied address, instead of being handle in the current processes context.
     * 
     * @param address - address of out of proc server - "127.0.0.1" is the right value for most applications
     * @param port - port - 44125 is the right value for most applications
     * @return  -  0 if successful, non-zero if failed.
     * \ingroup messaging
     */
    VIVOXSDK_DLLEXPORT int vx_set_out_of_process_server_address(const char *address, unsigned short port);

    /** 
     * Allocated an sdk handle. This allows applications to control multiple out of process servers.
     * If address is zero, then requests using this handle will run in process. Set the req.message.sdk_handle field 
     * to this value to direct a request to a specific out of process SDK instance.
     * 
     * @param address - address of out of proc server - "127.0.0.1" is the right value for most applications
     * @param port - port - 44125 is the right value for most applications
     * @return  -  0 if successful, non-zero if failed.
     * \ingroup messaging
     */
    VIVOXSDK_DLLEXPORT int vx_alloc_sdk_handle(const char *address, unsigned short port, VX_SDK_HANDLE *handle);

    /** 
     * frees the SDK handle
     * 
     * @param sdkHandle - the handle
     * @return  -  0 if successful, non-zero if failed.
     * \ingroup messaging
     */
    VIVOXSDK_DLLEXPORT int vx_free_sdk_handle(VX_SDK_HANDLE sdkHandle);

    /**
     * export a vxr recording into a single .wav file
     */
    VIVOXSDK_DLLEXPORT int vx_export_vxr_mixed(char *inputFile, char *outputFile, void (* pfOnFrameExported)(void *callbackHandle, int currentFrame, int MaxFrame), void *callbackHandle);

    /**
     * export a vxr recording into a wav file per participant in session
     */
    VIVOXSDK_DLLEXPORT int vx_export_vxr_expanded(char *inputFile, char *filePrefix, void (* pfOnFrameExported)(void *callbackHandle, int currentFrame, int MaxFrame), void *callbackHandle);

    /**
     * Application must call this API before calling any other Vivox API.
	 * @return               -  0 if successful, non-zero if failed.
     */
	VIVOXSDK_DLLEXPORT int vx_initialize();

    /**
     * Application must call this before exit
	 * @return               -  0 if successful, non-zero if failed.
     */
	VIVOXSDK_DLLEXPORT int vx_uninitialize();

#endif

#ifdef __cplusplus
}
#endif


#endif


