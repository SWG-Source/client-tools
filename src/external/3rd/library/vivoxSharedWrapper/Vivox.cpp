////////////////////////////////////////////////////////////////////////////////
//Vivox.cpp
//
//03/11/08 Joshua M. Kriegshauser
//
//Copyright (c) 2008 Sony Online Entertainment, LLC.  All rights reserved.
////////////////////////////////////////////////////////////////////////////////

#include "Vivox.h"

#include <algorithm>
#include <functional>

// Disable the nameless struct warning.
#pragma warning(disable:4201)

#define WIN32_EXTRA_LEAN
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <mmsystem.h>
#include <stdio.h>

// Vivox includes
#define VIVOX_TYPES_ONLY 1 // We're importing from DLL, so only need the types
#include "Vxc.h"
#include "VxcRequests.h"
#include "VxcResponses.h"
#include "VxcEvents.h"

// Functions from DLL
HMODULE shVivoxDll = 0;
#define DECLARE_FN( ret, name, parms ) name##_type name = 0
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

static bool sLoadAttempted = false;

void
sUnloadVivoxDLL()
{
    // Unload appears to crash randomly in versions prior to 3, even if calling vx_on_application_exit()
#if VIVOX_VERSION >= 3
	if ( shVivoxDll )
	{
        // Call onExit function if retrieved
        if ( vx_on_application_exit )
        {
            vx_on_application_exit();
        }

        sLoadAttempted = false;
		FreeLibrary( shVivoxDll );
		shVivoxDll = 0;
        #define CLEAR_FN(name) name = 0
	    CLEAR_FN(destroy_resp);
	    CLEAR_FN(destroy_evt);
	    CLEAR_FN(vx_get_message);
	    CLEAR_FN(vx_issue_request);
	    CLEAR_FN(vx_strdup);
	    CLEAR_FN(vx_req_connector_initiate_shutdown_create);
	    CLEAR_FN(vx_req_account_logout_create);
	    CLEAR_FN(vx_req_connector_create_create);
	    CLEAR_FN(vx_req_account_login_create);
        CLEAR_FN(vx_req_account_channel_add_moderator_create);
        CLEAR_FN(vx_req_account_channel_remove_moderator_create);
        CLEAR_FN(vx_req_channel_ban_user_create);
        CLEAR_FN(vx_req_channel_kick_user_create);
        CLEAR_FN(vx_req_channel_mute_user_create);
        CLEAR_FN(vx_req_channel_mute_all_users_create);
        CLEAR_FN(vx_req_account_channel_update_create);
        CLEAR_FN(vx_req_account_channel_get_moderators_create);
        CLEAR_FN(vx_req_sessiongroup_create_create);
        CLEAR_FN(vx_req_sessiongroup_add_session_create);
        CLEAR_FN(vx_req_sessiongroup_remove_session_create);
        CLEAR_FN(vx_req_sessiongroup_terminate_create);
	    CLEAR_FN(vx_req_session_create_create);
	    CLEAR_FN(vx_req_session_terminate_create);
        CLEAR_FN(vx_req_session_media_disconnect_create);
        CLEAR_FN(vx_req_session_send_message_create);
        CLEAR_FN(vx_req_connector_mute_local_mic_create);
        CLEAR_FN(vx_req_connector_mute_local_speaker_create);
        CLEAR_FN(vx_req_connector_set_local_mic_volume_create);
        CLEAR_FN(vx_req_connector_set_local_speaker_volume_create);
        CLEAR_FN(vx_req_session_set_participant_mute_for_me_create);
        CLEAR_FN(vx_req_session_set_participant_volume_for_me_create);
        CLEAR_FN(vx_req_aux_get_render_devices_create);
        CLEAR_FN(vx_req_aux_set_render_device_create);
        CLEAR_FN(vx_req_aux_get_capture_devices_create);
        CLEAR_FN(vx_req_aux_set_capture_device_create);
        CLEAR_FN(vx_req_aux_start_buffer_capture_create);
        CLEAR_FN(vx_req_aux_play_audio_buffer_create);
        CLEAR_FN(vx_req_aux_render_audio_stop_create);
        CLEAR_FN(vx_req_session_set_3d_position_create);
        CLEAR_FN(vx_req_sessiongroup_set_tx_session_create);
        CLEAR_FN(vx_req_aux_global_monitor_keyboard_mouse_create);
        CLEAR_FN(vx_req_session_mute_local_speaker_create);
        CLEAR_FN(vx_req_session_set_local_speaker_volume_create);
        CLEAR_FN(vx_req_session_send_notification_create);
        CLEAR_FN(vx_on_application_exit);
        CLEAR_FN(vx_req_aux_capture_audio_start_create);
        CLEAR_FN(vx_req_aux_capture_audio_stop_create);
        CLEAR_FN(vx_req_aux_set_mic_level_create);
        CLEAR_FN(vx_req_aux_set_speaker_level_create);
#if VIVOX_VERSION >= 3
        CLEAR_FN(vx_req_aux_diagnostic_state_dump_create);
        CLEAR_FN(vx_alloc_sdk_handle);
        CLEAR_FN(vx_free_sdk_handle);
#endif
        #undef CLEAR_FN
	}
#endif
}

bool
sStartService( const char* sExe, const char* /*sIP*/, int log_level )
{
    // If the management process is available, start it and connect to it.
    HANDLE hFile = ::CreateFile( sExe, GENERIC_READ, FILE_SHARE_READ|FILE_SHARE_WRITE|FILE_SHARE_DELETE, 0, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0 );
    if ( hFile != INVALID_HANDLE_VALUE )
    {
        // File exists, start the process and set the ENV variable
        ::CloseHandle( hFile );

        STARTUPINFO startupInfo;
        memset( &startupInfo, 0, sizeof(startupInfo) );
        startupInfo.cb = sizeof(STARTUPINFO);

        PROCESS_INFORMATION processInfo;

        // Command line options:
        // -ll <n> log level
        // -lf <folder> log folder
        // -lp <prefix> prefix for log files
        // -ls <suffix> suffix for log files

        char commandLine[ 1024 ];
        int pos = _snprintf( commandLine, sizeof(commandLine), "%s -lf logs -lp vvs -ls .log", sExe );

        if ( log_level > 0 )
        {
            pos += _snprintf( commandLine + pos, sizeof(commandLine) - pos, " -ll %d", log_level );
        }

        commandLine[ pos ] = '\0';

        if ( TRUE == ::CreateProcess( 0, commandLine, 0, 0, FALSE, 0, 0, 0, &startupInfo, &processInfo ) )
        {
            return true;
        }
    }

    return false;
}

bool
sLoadVivoxDLL( void (*onLoadFunc)( void ), void (*onWarning)( const char* file, int line, const char* pattern, ... ) )
{
	if ( shVivoxDll )
		return true;

	if ( sLoadAttempted )
	{
		// Something is broken, don't try loading again
		return false;
	}
	sLoadAttempted = true;

	shVivoxDll = LoadLibraryA( "vivoxsdk.dll" );
	if ( 0 == shVivoxDll )
    {
        // alert( "Failed to load vivoxsdk.dll" );
        return false;
    }

	// Import functions
    #define IMPORT_FN(name) name = (name##_type)GetProcAddress( shVivoxDll, #name ); if ( 0 == name ) { onWarning( __FILE__, __LINE__, "Failed to import function: %s", #name ); sUnloadVivoxDLL(); return false; }
    IMPORT_FN(vx_on_application_exit); // Import application exit function first
	IMPORT_FN(destroy_resp);
	IMPORT_FN(destroy_evt);
	IMPORT_FN(vx_get_message);
	IMPORT_FN(vx_issue_request);
	IMPORT_FN(vx_strdup);
	IMPORT_FN(vx_req_connector_initiate_shutdown_create);
	IMPORT_FN(vx_req_account_logout_create);
	IMPORT_FN(vx_req_connector_create_create);
	IMPORT_FN(vx_req_account_login_create);
    IMPORT_FN(vx_req_account_channel_add_moderator_create);
    IMPORT_FN(vx_req_account_channel_remove_moderator_create);
    IMPORT_FN(vx_req_channel_ban_user_create);
    IMPORT_FN(vx_req_channel_kick_user_create);
    IMPORT_FN(vx_req_channel_mute_user_create);
    IMPORT_FN(vx_req_channel_mute_all_users_create);
    IMPORT_FN(vx_req_account_channel_update_create);
    IMPORT_FN(vx_req_account_channel_get_moderators_create);
    IMPORT_FN(vx_req_sessiongroup_create_create);
    IMPORT_FN(vx_req_sessiongroup_add_session_create);
    IMPORT_FN(vx_req_sessiongroup_remove_session_create);
    IMPORT_FN(vx_req_sessiongroup_terminate_create);
	IMPORT_FN(vx_req_session_create_create);
	IMPORT_FN(vx_req_session_terminate_create);
    IMPORT_FN(vx_req_session_media_disconnect_create);
    IMPORT_FN(vx_req_session_send_message_create);
    IMPORT_FN(vx_req_connector_mute_local_mic_create);
    IMPORT_FN(vx_req_connector_mute_local_speaker_create);
    IMPORT_FN(vx_req_connector_set_local_mic_volume_create);
    IMPORT_FN(vx_req_connector_set_local_speaker_volume_create);
    IMPORT_FN(vx_req_session_set_participant_mute_for_me_create);
    IMPORT_FN(vx_req_session_set_participant_volume_for_me_create);
    IMPORT_FN(vx_req_aux_get_render_devices_create);
    IMPORT_FN(vx_req_aux_set_render_device_create);
    IMPORT_FN(vx_req_aux_get_capture_devices_create);
    IMPORT_FN(vx_req_aux_set_capture_device_create);
    IMPORT_FN(vx_req_aux_start_buffer_capture_create);
    IMPORT_FN(vx_req_aux_play_audio_buffer_create);
    IMPORT_FN(vx_req_aux_render_audio_stop_create);
    IMPORT_FN(vx_req_session_set_3d_position_create);
    IMPORT_FN(vx_req_sessiongroup_set_tx_session_create);
    IMPORT_FN(vx_req_aux_global_monitor_keyboard_mouse_create);
    IMPORT_FN(vx_req_session_mute_local_speaker_create);
    IMPORT_FN(vx_req_session_set_local_speaker_volume_create);
    IMPORT_FN(vx_req_session_send_notification_create);
    IMPORT_FN(vx_req_aux_capture_audio_start_create);
    IMPORT_FN(vx_req_aux_capture_audio_stop_create);
    IMPORT_FN(vx_req_aux_set_mic_level_create);
    IMPORT_FN(vx_req_aux_set_speaker_level_create);
#if VIVOX_VERSION >= 3
    IMPORT_FN(vx_req_aux_diagnostic_state_dump_create);
    IMPORT_FN(vx_alloc_sdk_handle);
    IMPORT_FN(vx_free_sdk_handle);
#endif
	#undef IMPORT_FN

    // Alright, we just loaded the DLL, now do all of the initial state setup
    onLoadFunc();

	return true;
}


bool
sGetKeyState( int iKeyCode )
{
    // Swap mouse buttons if necessary (see docs for GetAsyncKeyState)
    if ( ( iKeyCode == VK_LBUTTON || iKeyCode == VK_RBUTTON ) && GetSystemMetrics( SM_SWAPBUTTON ) == TRUE )
    {
        if ( iKeyCode == VK_LBUTTON )
            iKeyCode = VK_RBUTTON;
        else
            iKeyCode = VK_LBUTTON;
    }

    // For numeric keypad keys, verify that numlock is on
    const bool kbNumeric = ( iKeyCode >= VK_NUMPAD0 && iKeyCode <= VK_NUMPAD9 );
    if ( kbNumeric )
    {
        SHORT s = GetKeyState( VK_NUMLOCK );
        if ( ( s & 0x0001 ) == 0 )
        {
            // If the numeric key is still 'on', force it to be off
            s = GetKeyState( iKeyCode );
            if ( ( s & 0x8000 ) != 0 )
            {
                BYTE keys[ 256 ];
                if ( GetKeyboardState( keys ) == TRUE )
                {
                    keys[ iKeyCode ] = 0;
                    SetKeyboardState( keys );
                }
            }
            return false;
        }
    }

    SHORT s = kbNumeric ? GetKeyState( iKeyCode ) : GetAsyncKeyState( iKeyCode );
    bool b = ( s & 0x8000 ) != 0;

    if ( !b )
    {
        // Handle generic control/alt/shift
        if ( iKeyCode == VK_CONTROL )
            b = ( ( GetAsyncKeyState( VK_LCONTROL ) | GetAsyncKeyState( VK_RCONTROL ) ) & 0x8000 ) != 0;
        else if ( iKeyCode == VK_MENU )
            b = ( ( GetAsyncKeyState( VK_LMENU    ) | GetAsyncKeyState( VK_RMENU    ) ) & 0x8000 ) != 0;
        else if ( iKeyCode == VK_SHIFT )
            b = ( ( GetAsyncKeyState( VK_LSHIFT   ) | GetAsyncKeyState( VK_RSHIFT   ) ) & 0x8000 ) != 0;
    }

    return b;
}


VivoxCheckMic
sCheckMic( const std::string& sDevice, bool bFix /*=false*/ )
{
    // Disable for Vista
    static enum
    {
        UNKNOWN = -1,
        IS_NOT_VISTA = 0,
        IS_VISTA
    } sIsVista = UNKNOWN;

    if ( UNKNOWN == sIsVista )
    {
        // Determine once
        OSVERSIONINFOEX info;
        info.dwOSVersionInfoSize = sizeof(OSVERSIONINFOEX);
        ::GetVersionEx( (OSVERSIONINFO*)&info );
        const bool kbIsVista = ( 6 == info.dwMajorVersion &&
                                 0 == info.dwMinorVersion &&
                                 VER_NT_WORKSTATION == info.wProductType );

        sIsVista = kbIsVista ? IS_VISTA : IS_NOT_VISTA;
    }

    if ( IS_VISTA == sIsVista )
    {
        return VCM_OK;
    }

    int retval = VCM_OK;

    const UINT num = mixerGetNumDevs();

    bool bFound = false;
    for ( UINT uDev = 0; uDev != num; ++uDev )
    {
        MIXERCAPS caps;
        if ( mixerGetDevCaps( uDev, &caps, sizeof(caps) ) != MMSYSERR_NOERROR )
        {
            retval |= VCM_ERROR;
            continue;
        }

        // Skip this device if it's not the one we're looking for
        if ( !sDevice.empty() && sDevice != caps.szPname )
            continue;

        bFound = true;

        HMIXER h;
        if ( mixerOpen( &h, uDev, 0, 0, MIXER_OBJECTF_MIXER ) != MMSYSERR_NOERROR )
        {
            retval |= VCM_ERROR;
            continue;
        }

        // Temp object to close the mixer device when we go out of scope (or continue;)
        class MixerCloser
        {
            HMIXER& m_h;
        public:
            MixerCloser( HMIXER& h ) : m_h( h ) {}
            ~MixerCloser() { mixerClose( m_h ); m_h = 0; }

		private:
			MixerCloser & operator= (const MixerCloser &) { return *this; }
        } closer( h );

        MIXERLINE lineWaveIn;
        lineWaveIn.cbStruct = sizeof(lineWaveIn);
        lineWaveIn.dwComponentType = MIXERLINE_COMPONENTTYPE_DST_WAVEIN;

        if ( mixerGetLineInfo( (HMIXEROBJ)h, &lineWaveIn, MIXER_GETLINEINFOF_COMPONENTTYPE ) != MMSYSERR_NOERROR )
        {
            retval |= VCM_ERROR;
            continue;
        }

        DWORD dwDesiredLineID = ~0U;

        // Check the Wave In mixer device for Mute, Volume and MUX controls
        if ( lineWaveIn.cControls != 0 )
        {
            // Find the Mute All control (if it exists)
            MIXERCONTROL mxc;
            MIXERLINECONTROLS mxlc;
            memset( &mxlc, 0, sizeof(mxlc) );
            mxlc.cbStruct = sizeof(mxlc);
            mxlc.cbmxctrl = sizeof(mxc);
            mxlc.cControls = 1;
            mxlc.pamxctrl = &mxc;
            mxlc.dwLineID = lineWaveIn.dwLineID;
            mxlc.dwControlType = MIXERCONTROL_CONTROLTYPE_MUTE;
            if ( mixerGetLineControls( (HMIXEROBJ)h, &mxlc, MIXER_GETLINECONTROLSF_ONEBYTYPE ) == MMSYSERR_NOERROR )
            {
                // Found a Mute control.  Query it and set it if necessary.
                MIXERCONTROLDETAILS_BOOLEAN mxcdBool;
                MIXERCONTROLDETAILS mxcd;
                mxcd.cbStruct = sizeof(mxcd);
                mxcd.dwControlID = mxc.dwControlID;
                mxcd.cChannels = 1;
                mxcd.cMultipleItems = 0;
                mxcd.cbDetails = sizeof(MIXERCONTROLDETAILS_BOOLEAN);
                mxcd.paDetails = &mxcdBool;
                if ( mixerGetControlDetails( (HMIXEROBJ)h, &mxcd, MIXER_GETCONTROLDETAILSF_VALUE ) == MMSYSERR_NOERROR )
                {
                    if ( mxcdBool.fValue == TRUE )
                    {
                        retval |= VCM_MUTED;
                        if ( bFix )
                        {
                            mxcdBool.fValue = FALSE;
                            if ( mixerSetControlDetails( (HMIXEROBJ)h, &mxcd, MIXER_SETCONTROLDETAILSF_VALUE ) != MMSYSERR_NOERROR )
                            {
                                retval |= VCM_ERROR;
                            }
                        }
                    }
                }
                else
                {
                    retval |= VCM_ERROR;
                }
            }

            // Find the volume control (if it exists)
            mxlc.dwControlType = MIXERCONTROL_CONTROLTYPE_VOLUME;
            if ( mixerGetLineControls( (HMIXEROBJ)h, &mxlc, MIXER_GETLINECONTROLSF_ONEBYTYPE ) == MMSYSERR_NOERROR )
            {
                // Found a Volume control.  Query it and set it if necessary.
                MIXERCONTROLDETAILS_UNSIGNED mxcdUns;
                MIXERCONTROLDETAILS mxcd;
                mxcd.cbStruct = sizeof(mxcd);
                mxcd.dwControlID = mxc.dwControlID;
                mxcd.cChannels = 1;
                mxcd.cMultipleItems = 0;
                mxcd.cbDetails = sizeof(MIXERCONTROLDETAILS_UNSIGNED);
                mxcd.paDetails = &mxcdUns;
                if ( mixerGetControlDetails( (HMIXEROBJ)h, &mxcd, MIXER_GETCONTROLDETAILSF_VALUE ) == MMSYSERR_NOERROR )
                {
                    const float kfRange = static_cast< float >( mxc.Bounds.dwMaximum - mxc.Bounds.dwMinimum );
                    DWORD desired = static_cast< DWORD >( 0.5f * kfRange ) + mxc.Bounds.dwMinimum;

                    if ( mxcdUns.dwValue < desired )
                    {
                        retval |= VCM_VOLUME_TOO_LOW;
                        if ( bFix )
                        {
                            mxcdUns.dwValue = desired;
                            if ( mixerSetControlDetails( (HMIXEROBJ)h, &mxcd, MIXER_SETCONTROLDETAILSF_VALUE ) != MMSYSERR_NOERROR )
                            {
                                retval |= VCM_ERROR;
                            }
                        }
                    }
                }
                else
                {
                    retval |= VCM_ERROR;
                }
            }

            // Find the MUX control and determine if we have more than one device
            mxlc.dwControlType = MIXERCONTROL_CONTROLTYPE_MUX;
            if ( mixerGetLineControls( (HMIXEROBJ)h, &mxlc, MIXER_GETLINECONTROLSF_ONEBYTYPE ) == MMSYSERR_NOERROR &&
                 mxc.cMultipleItems > 1 )
            {
                // Got a MUX control, now see what it's set to.  We need to get the list because the MUX items
                // might be sorted differently than the connections

                std::vector< MIXERCONTROLDETAILS_LISTTEXT > aList( mxc.cMultipleItems );
                MIXERCONTROLDETAILS mxcdList;
                mxcdList.cbStruct = sizeof(mxcdList);
                mxcdList.dwControlID = mxc.dwControlID;
                mxcdList.cChannels = 1;
                mxcdList.cMultipleItems = mxc.cMultipleItems;
                mxcdList.cbDetails = sizeof(MIXERCONTROLDETAILS_LISTTEXT);
                mxcdList.paDetails = &aList[ 0 ];

                std::vector< MIXERCONTROLDETAILS_BOOLEAN > aBool( mxc.cMultipleItems );
                MIXERCONTROLDETAILS mxcdBool;
                mxcdBool.cbStruct = sizeof(mxcdBool);
                mxcdBool.dwControlID = mxc.dwControlID;
                mxcdBool.cChannels = 1;
                mxcdBool.cMultipleItems = mxc.cMultipleItems;
                mxcdBool.cbDetails = sizeof(MIXERCONTROLDETAILS_BOOLEAN);
                mxcdBool.paDetails = &aBool[ 0 ];

                if ( mixerGetControlDetails( (HMIXEROBJ)h, &mxcdList, MIXER_GETCONTROLDETAILSF_LISTTEXT ) == MMSYSERR_NOERROR &&
                     mixerGetControlDetails( (HMIXEROBJ)h, &mxcdBool, MIXER_GETCONTROLDETAILSF_VALUE ) == MMSYSERR_NOERROR )
                {
                    // Whew, got all the control details we need for the MUX.  See if it's set to the right value.
                    // We'll also change the values here, but we only send them back to the API if we're told to fix.
                    // 
                    // We also must handle multiple microphones, as some devices have separate front/rear mics

                    // Loop through once to see if a mic is already selected
                    for ( unsigned int iVal = 0; iVal != mxc.cMultipleItems; ++iVal )
                    {
                        if ( MIXERLINE_COMPONENTTYPE_SRC_MICROPHONE == aList[ iVal ].dwParam2 &&
                             aBool[ iVal ].fValue == TRUE )
                        {
                            // Found a mic that was selected
                            dwDesiredLineID = aList[ iVal ].dwParam1;
                            break;
                        }
                    }

                    // If we don't have a desired mic, take the first one that is connected
                    if ( ~0U == dwDesiredLineID )
                    {
                        for ( unsigned int iVal = 0; iVal != mxc.cMultipleItems; ++iVal )
                        {
                            if ( MIXERLINE_COMPONENTTYPE_SRC_MICROPHONE == aList[ iVal ].dwParam2 )
                            {
                                MIXERLINE line;
                                line.cbStruct = sizeof(line);
                                line.dwLineID = aList[ iVal ].dwParam1;
                                if ( mixerGetLineInfo( (HMIXEROBJ)h, &line, MIXER_GETLINEINFOF_LINEID ) != MMSYSERR_NOERROR )
                                {
                                    retval |= VCM_ERROR;
                                    continue;
                                }

                                if ( ( line.fdwLine & MIXERLINE_LINEF_DISCONNECTED ) == 0 )
                                {
                                    // This line appears to be connected (if the line even supports notification about that)
                                    // Take this one.
                                    dwDesiredLineID = aList[ iVal ].dwParam1;
                                    break;
                                }
                            }
                        }
                    }

                    for ( unsigned int iVal = 0; iVal != mxc.cMultipleItems; ++iVal )
                    {
                        if ( MIXERLINE_COMPONENTTYPE_SRC_MICROPHONE == aList[ iVal ].dwParam2 )
                        {
                            // Select this mic as the desired mic if we don't already have one
                            if ( dwDesiredLineID == ~0U )
                            {
                                dwDesiredLineID = aList[ iVal ].dwParam1;
                            }

                            // If this is the desired mic, make sure it's turned on
                            if ( dwDesiredLineID == aList[ iVal ].dwParam1 )
                            {
                                if ( aBool[ iVal ].fValue != TRUE )
                                    retval |= VCM_NOT_SELECTED;
                                aBool[ iVal ].fValue = TRUE;
                            }
                            else
                            {
                                // Not desired mic, make sure it's turned off
                                aBool[ iVal ].fValue = FALSE;
                            }
                        }
                        else
                        {
                            // Set everything other than the mic to false
                            aBool[ iVal ].fValue = FALSE;
                        }
                    }

                    if ( bFix )
                    {
                        // If told to fix but no desired mic was found, show as an error.
                        if ( dwDesiredLineID == ~0U || mixerSetControlDetails( (HMIXEROBJ)h, &mxcdBool, MIXER_SETCONTROLDETAILSF_VALUE ) != MMSYSERR_NOERROR )
                            retval |= VCM_ERROR;
                    }
                }
                else
                {
                    retval |= VCM_ERROR;
                }
            }
        }

        // Now find the Microphone connection and check it for Mic-specific Volume and Mute controls
        for ( unsigned int iCon = 0; iCon != lineWaveIn.cConnections; ++iCon )
        {
            MIXERLINE line = lineWaveIn;
            line.dwSource = iCon;

            if ( mixerGetLineInfo( (HMIXEROBJ)h, &line, MIXER_GETLINEINFOF_SOURCE ) != MMSYSERR_NOERROR )
            {
                retval |= VCM_ERROR;
                continue;
            }

            if ( MIXERLINE_COMPONENTTYPE_SRC_MICROPHONE != line.dwComponentType )
                continue;

            // Only care about our desired line ID (if we have a desired line ID)
            if ( dwDesiredLineID != ~0U && line.dwLineID != dwDesiredLineID )
                continue;

            if ( line.fdwLine & MIXERLINE_LINEF_DISCONNECTED )
            {
                retval |= VCM_DISCONNECTED;
                // This situation can't be fixed in code (at least... I don't think so)
            }

            if ( 0 == line.cControls )
                continue;

            // Find the Mute control (if it exists)
            MIXERCONTROL mxc;
            MIXERLINECONTROLS mxlc;
            memset( &mxlc, 0, sizeof(mxlc) );
            mxlc.cbStruct = sizeof(mxlc);
            mxlc.cbmxctrl = sizeof(mxc);
            mxlc.cControls = 1;
            mxlc.pamxctrl = &mxc;
            mxlc.dwLineID = line.dwLineID;
            mxlc.dwControlType = MIXERCONTROL_CONTROLTYPE_MUTE;
            if ( mixerGetLineControls( (HMIXEROBJ)h, &mxlc, MIXER_GETLINECONTROLSF_ONEBYTYPE ) == MMSYSERR_NOERROR )
            {
                // Found a Mute control.  Query it and set it if necessary.
                MIXERCONTROLDETAILS_BOOLEAN mxcdBool;
                MIXERCONTROLDETAILS mxcd;
                mxcd.cbStruct = sizeof(mxcd);
                mxcd.dwControlID = mxc.dwControlID;
                mxcd.cChannels = 1;
                mxcd.cMultipleItems = 0;
                mxcd.cbDetails = sizeof(MIXERCONTROLDETAILS_BOOLEAN);
                mxcd.paDetails = &mxcdBool;
                if ( mixerGetControlDetails( (HMIXEROBJ)h, &mxcd, MIXER_GETCONTROLDETAILSF_VALUE ) == MMSYSERR_NOERROR )
                {
                    if ( mxcdBool.fValue == TRUE )
                    {
                        retval |= VCM_MUTED;
                        if ( bFix )
                        {
                            mxcdBool.fValue = FALSE;
                            if ( mixerSetControlDetails( (HMIXEROBJ)h, &mxcd, MIXER_SETCONTROLDETAILSF_VALUE ) != MMSYSERR_NOERROR )
                            {
                                retval |= VCM_ERROR;
                            }
                        }
                    }
                }
                else
                {
                    retval |= VCM_ERROR;
                }
            }

            // Find the volume control (if it exists)
            mxlc.dwControlType = MIXERCONTROL_CONTROLTYPE_VOLUME;
            if ( mixerGetLineControls( (HMIXEROBJ)h, &mxlc, MIXER_GETLINECONTROLSF_ONEBYTYPE ) == MMSYSERR_NOERROR )
            {
                // Found a Volume control.  Query it and set it if necessary.
                MIXERCONTROLDETAILS_UNSIGNED mxcdUns;
                MIXERCONTROLDETAILS mxcd;
                mxcd.cbStruct = sizeof(mxcd);
                mxcd.dwControlID = mxc.dwControlID;
                mxcd.cChannels = 1;
                mxcd.cMultipleItems = 0;
                mxcd.cbDetails = sizeof(MIXERCONTROLDETAILS_UNSIGNED);
                mxcd.paDetails = &mxcdUns;
                if ( mixerGetControlDetails( (HMIXEROBJ)h, &mxcd, MIXER_GETCONTROLDETAILSF_VALUE ) == MMSYSERR_NOERROR )
                {
                    const float kfRange = static_cast< float >( mxc.Bounds.dwMaximum - mxc.Bounds.dwMinimum );
                    DWORD desired = static_cast< DWORD >( 0.5f * kfRange ) + mxc.Bounds.dwMinimum;

                    if ( mxcdUns.dwValue < desired )
                    {
                        retval |= VCM_VOLUME_TOO_LOW;
                        if ( bFix )
                        {
                            mxcdUns.dwValue = desired;
                            if ( mixerSetControlDetails( (HMIXEROBJ)h, &mxcd, MIXER_SETCONTROLDETAILSF_VALUE ) != MMSYSERR_NOERROR )
                            {
                                retval |= VCM_ERROR;
                            }
                        }
                    }
                }
                else
                {
                    retval |= VCM_ERROR;
                }
            }
        }
    }

    if ( !bFound )
        retval |= VCM_ERROR;

    return (VivoxCheckMic)retval;
}

static HANDLE shVivoxMutex = 0;

bool
sGrabVivoxSystemMutex()
{
#if VIVOX_VERSION < 3
    if ( shVivoxMutex )
    {
        // Already have it
        return true;
    }

    // Create/Open the named mutex
    shVivoxMutex = ::CreateMutex( 0, TRUE, "{4355ce47-39f2-4f52-bd0e-f6e609b20f82}" );
    if ( 0 == shVivoxMutex )
        return false;

    if ( ERROR_ALREADY_EXISTS == ::GetLastError() )
    {
        // Ok, it exists, but we weren't granted ownership
        if ( WAIT_OBJECT_0 != ::WaitForSingleObject( shVivoxMutex, 0 ) )
        {
            // Failed to grab it, just release for now
            ::CloseHandle( shVivoxMutex );
            shVivoxMutex = 0;
            return false;
        }
    }
#endif
    // Grabbed it
    return true;
}

bool
sReleaseVivoxSystemMutex()
{
#if VIVOX_VERSION < 3
    if ( shVivoxMutex )
    {
        ::ReleaseMutex( shVivoxMutex );
        ::CloseHandle( shVivoxMutex );
        shVivoxMutex = 0;
        return true;
    }

    return false;
#else
    return true;
#endif
}

