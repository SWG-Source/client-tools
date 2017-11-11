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

#ifndef __VXCEVENTS_H__
#define __VXCEVENTS_H__

#include "Vxc.h"

#ifdef __cplusplus
extern "C" {
#endif


/**
 * This event message is sent whenever the login state of the particular Account has transitioned from one value to another
 * 
 * The XML for this event can be found here: \ref AccountLoginStateChangeEvent
 *
 * \ingroup login
 */
typedef struct vx_evt_account_login_state_change {
    /**
     * The common properties for all events
     */
    vx_evt_base_t base;
    /**
    * New state of the entity, please refer to the codes at the end of this doc for specific state codes
    */
    vx_login_state_change_state state;
    /**
    * Handle returned from successful Account request .
    */
    VX_HANDLE account_handle;
    /**
    * Code used to identify why a state change has been made
    */
    int status_code;
    /**
    * Text (in English) to describe the Status Code
    */
    char* status_string;
} vx_evt_account_login_state_change_t;


/**
 * Presented when a buddy has issued presence information.
 *
 * The XML for this event can be found here: \ref BuddyPresenceEvent
 *
 * \ingroup buddy
 */
typedef struct vx_evt_buddy_presence {
    /**
     * The common properties for all events
     */
    vx_evt_base_t base;
    /**
     * DEPRECATED
     * \deprecated
     */
    vx_buddy_presence_state state;
    /**
    * Handle returned from successful 'login' request 
    */
    VX_HANDLE account_handle;
    /**
    * The URI of the sender of the message
    */
    char* buddy_uri;
    /**
    * New presense of the buddy, please refer to the codes in table \ref vx_buddy_presence_state
    */
    vx_buddy_presence_state presence;
    /**
    * Custom message string when presence is set to custom.
    */
    char* custom_message;
    /**
     * The displayname if the buddy_uri
     * had a displayname
     */
    char* displayname;
    /**
     * The application of the buddy who's presence is being reported.  May be NULL or empty.
     */
    char* application;
    /**
     * The contact address (Uri) of the buddy who's presence is being reported.  May be NULL or empty.
     */
    char* contact;
    /**
     * RESERVED FOR FUTURE USE: The priority of the buddy who's presence is being reported.  May be NULL or empty.
     */
    char* priority;
    /**
     * RESERVED FOR FUTURE USE: The unique id of the instance of the buddy who's presence is being reported.  May be NULL or empty.
     */
    char* id;
} vx_evt_buddy_presence_t;

/**
* Generated when a buddy wants to request presence visibility.  This event will not be presented if an auto-accept or auto-block  
* rule matches the requesting buddy_uri. Typically the application would use this event to prompt a user to explicitly accept or deny the request for presence.  
* Optionally the application might create and store an auto-accept or auto-block rule based upon the users' choice. The application should generate a 
* vx_req_account_send_subscription_reply_t resquest based upon application logic and/or end-user response. 
* The subscription_handle value must be extracted and returned as a parameter to  vx_req_account_send_subscription_reply_t
 *
 * The XML for this event can be found here: \ref SubscriptionEvent
 *
 * \see vx_req_account_buddy_set
 * \ingroup buddy
*/
typedef struct vx_evt_subscription {
    /**
     * The common properties for all events
     */
    vx_evt_base_t base;
    /**
     * Handle returned from successful 'login' request 
     */
    VX_HANDLE account_handle;
    /**
     * The URI of the buddy who's presence is being sent
     */
    char* buddy_uri;
    /**
     * The identifier of the subscription event. Used when forming a reply with vx_req_account_send_subscription_reply_t
     */
    char* subscription_handle;
    /**
     * "subscription_presence" is currently the only supported value
     */
    vx_subscription_type subscription_type;
    /**
     * The displayname if the buddy_uri
     * had a displayname
     */
    char* displayname;
    /**
     * The application of the buddy who's subscription is being reported.  May be NULL or empty.
     */
    char* application;
    /**
    * NOT CURRENTLY IMPLEMENTED
    * Optional message supplied by the initiating user on vx_req_account_buddy_set_t.
    */
    char* message;
} vx_evt_subscription_t;

/**
 * Received when another user has started or stopped typing, or raised or lowered their hand, within the context of a session.
 *
 * The XML for this event can be found here: \ref SessionNotificationEvent
 *
 * \ingroup session
 */
typedef struct vx_evt_session_notification {
    /**
     * The common properties for all events
     */
    vx_evt_base_t base;
    /**
     * DEPRECATED
     * \deprecated
     */
    vx_session_notification_state state;
    /**
     * the handle of the session to which this event applies.
     */
    VX_HANDLE session_handle;
    /**
    * The URI of the buddy who's presence is being sent
    */
    char* participant_uri;
    /**
    * New notification type from the buddy, please refer to the codes in table \ref vx_notification_type
    */
    vx_notification_type notification_type;
} vx_evt_session_notification_t;


/**
* Presented when an incoming message has arrived from a participant in an open session with text enabled.
*
* The XML for this event can be found here: \ref MessageEvent
*
* \ingroup session
*/
typedef struct vx_evt_message {
    /**
     * The common properties for all events
     */
    vx_evt_base_t base;
    /**
     * DEPRECATED
     * \deprecated
     */
    vx_message_state state;
    /**
     * Handle returned from successful SessionGroup 'create' request 
     * \see vx_req_sessiongroup_create
     */
    VX_HANDLE sessiongroup_handle;
    /**
     * Handle returned from successful Session 'add' request 
     * \see vx_req_sessiongroup_add_session
     */
    VX_HANDLE session_handle;
    /**
     * The URI of the sender of the message
     */
    char* participant_uri;
    /**
     * Content type of the message
     */
    char* message_header;
    /**
     * The contents of the message
     */
    char* message_body;
    /**
     * The displayname if the participant_uri
     * had a displayname
     */
    char* participant_displayname;
    /**
     * The application of the entity who is sending the message.  May be NULL or empty.
     */
    char* application;
} vx_evt_message_t;


/**
* The auxiliary 'audio properties' events are used by the SDK sound system to present 
* audio information to the application, which may be used to create a visual representation of the speaker 
* (for example, a so called 'VU' meter). These events are presented at one half the rate of the audio capture rate.
 *
 * The XML for this event can be found here: \ref AuxAudioPropertiesEvent
 *
 * \ingroup devices
 */
typedef struct vx_evt_aux_audio_properties {
    /**
     * The common properties for all events
     */
    vx_evt_base_t base;
    /**
     * DEPRECATED
     * \deprecated
     */
    vx_aux_audio_properties_state state;
    /**
    * 1 - Indicates that the capture device is detecting 'speech activity' (as determined by the built in 
    * Vivox Voice Activity Detector). 
    * 0 - Indicates no speech energy has been detected.
    */
    int mic_is_active;
    /**
    * Indicates the current value of the master 'microphone' volume, as set using the 'set mic volume' method above. 
    * Non negative value between 0 and 100 (inclusive)
    */
    int mic_volume;
    /**
    * The instantaneous energy at the capture device. This is a floating point number between 0 and 1.
    */
    double mic_energy;
    /**
    * Indicates the current value of the master 'speaker' volume, as set using the 'set speaker volume' method above. Non negative value between 0 and 100 (inclusive)
    */
    int speaker_volume;
} vx_evt_aux_audio_properties_t;


/**
 * for vx_evt_buddy_changed_t and vx_evt_buddy_group_changed_t objects, indicates whether the object is deleted 
 * or "set", which means either added or updated.
 * \ingroup buddy
 */
typedef enum {
    /**
     * buddy or group was added or updated
     */
    change_type_set = 1,
    /**
     * buddy or group was deleted
     */
    change_type_delete = 2,
} vx_change_type_t;


/**
* Presented when a buddy is either set (added or update) or removed.
*
* The XML for this event can be found here: \ref BuddyChangedEvent
*
* \ingroup buddy
*/
typedef struct vx_evt_buddy_changed {
    /**
     * The common properties for all events
     */
    vx_evt_base_t base;
    /**
    * Handle returned from successful 'login' request 
    */
    VX_HANDLE account_handle;
    /**
    * set or delete.
    */
    vx_change_type_t change_type;
    /**
    * the uri of the buddy
    */
    char *buddy_uri;
    /**
    * the display name of the buddy
    */
    char *display_name;
    /**
    * application specific buddy data
    */
    char *buddy_data;
    /**
    * the group the buddy belongs to
    */
    int group_id;
    /**
    * the account id of the buddy
    */
    int account_id;
} vx_evt_buddy_changed_t;

/**
* Presented when a buddy group is set (added or updated) or removed
*
* The XML for this event can be found here: \ref BuddyGroupChangedEvent
*
* \ingroup buddy
*/
typedef struct vx_evt_buddy_group_changed {
    /**
     * The common properties for all events
     */
    vx_evt_base_t base;
    /**
    * Handle returned from successful 'login' request 
    */
    VX_HANDLE account_handle;
    /**
    * set or delete.
    */
    vx_change_type_t change_type;
    /**
    * The id for the group
    */
    int group_id;
    /**
    * the display name for the group
    */
    char *group_name;
    /**
    * application specific group data
    */
    char *group_data;
} vx_evt_buddy_group_changed_t;

/**
 * Presented when the buddy or group list undergoes a significant change. 
 * This event is always received after login, and can be used to build the initial buddy and group UI.
 *
 * The XML for this event can be found here: \ref BuddyAndGroupListChangedEvent
 *
 * \ingroup buddy
 */
typedef struct vx_evt_buddy_and_group_list_changed {
    /**
     * The common properties for all events.
     */
    vx_evt_base_t base;
    /**
     * Handle returned from successful 'login' request 
     */
    VX_HANDLE account_handle;
    /**
     * count of buddies
     */
    int buddy_count;
    /**
     * an array of pointers to buddies
     */
    vx_buddy_t **buddies;
    /**
     * count of groups
     */
    int group_count;
    /**
     * an array of buddy group pointers
     */
    vx_group_t **groups;
} vx_evt_buddy_and_group_list_changed_t;

/**
* The vx_evt_keyboard_mouse_t event is raised to indicate to the application that a particular keyboard/mouse button combination has been pressed or cleared.
*
* The XML for this event can be found here: \ref KeyboardMouseEvent
*
* \ingroup devices
*
* \attention \attention Not supported on the PLAYSTATION(R)3 platform
*/
typedef struct vx_evt_keyboard_mouse {
    /**
     * The common properties for all events
     */
    vx_evt_base_t base;
    /**
    * the name of the binding as set in vx_req_aux_global_monitor_keyboard_mouse_t
    */
    char *name;
    /**
    * 1 if the key/mouse button combination corresponding to this name is down, and 0 when it has been cleared.
    */
    int is_down;
} vx_evt_keyboard_mouse_t;

/**
* The vx_evt_idle_state_changed_t event is raised to indicate to the application that the user has transitioned from the between idle and non-idle states (and vice-versa).
*
* The XML for this event can be found here: \ref IdleStateChangedEvent
*
* \ingroup devices
*
* \attention \attention Not supported on the PLAYSTATION(R)3 platform
*/
typedef struct vx_evt_idle_state_changed {
    /**
     * The common properties for all events
     */
    vx_evt_base_t base;
    /**
    * 1 if the user is idle, 0 if the user is not idle.
    */
    int is_idle;
} vx_evt_idle_state_changed_t;

/**
* Sent when Session Media has been altered. 
*
* The XML for this event can be found here: \ref SessionMediaEvent
*
* \ingroup session
*/
typedef struct vx_evt_media_stream_updated {
    /**
     * The common properties for all events
     */
    vx_evt_base_t base;
    /**
     * Handle created for session group
     */
    VX_HANDLE sessiongroup_handle;
    /**
     * Handle created for session
     */
    VX_HANDLE session_handle;
    /**
     * Code used to identify why a state change has been made
     * This code is only useful if state is equal to 'session_media_disconnected'.
     * In this case the following rules apply:
     * 
     * 1. Any code < 400 can be ignored.
     * 2. 401 - A password is needed to join this channel. Typically an application will present a password dialog at this point.
     * 3. 403 - If the call is to a channel, the user does not have sufficient privilege to join the channel, otherwise, the call has been declined.
     * 4. 404 - Destination (either a channel or other user) does not exist.
     * 5. 408 - The remote user did not answer the call
     * 6. 480 - The remote user is temporarily offline
     * 7. 486 - The remote user is busy (on another call).
     * 8. 603 - The remote user has declined the call.
     *
     * It is recommended that the status_string field only be display as diagnostic information for status codes > 400, and not in the list above. 
     * This status_string is often generated by the network, which can include public PSTN networks as well. This can result in status_string values that 
     * are informative to a technician, but not to an end user, and may be subject to change. Applications should not depend on the value of this field.
     *
     * Applications should present an application specific message for each of the status codes outlined above.
     */
    int status_code;
    /**
     * Text (in English) to describe the Status Code
     *
     * see the status_code field above.
     */
    char* status_string;
    /**
     * New state of the entity, please refer to the codes in table \ref vx_session_media_state
     */
    vx_session_media_state state;
    /**
     * This indicates if this is an incoming call or not.
     */
    int incoming;
} vx_evt_media_stream_updated_t;

/**
* Sent when Session Text has been altered.
*
* The XML for this event can be found here: \ref TextStreamUpdatedEvent
*
* \ingroup session
*/
typedef struct vx_evt_text_stream_updated {
    /**
     * The common properties for all events
     */
    vx_evt_base_t base;
    /**
     * Handle created for session group
     */
    VX_HANDLE sessiongroup_handle;
    /**
     * Handle created for session
     */
    VX_HANDLE session_handle;
    /**
     * Indicates if text is enabled on the session
     */
    int enabled;
    /**
     * Indicates the state of text, connected or disconnected
     */
    vx_session_text_state state;
    /**
     * Indicates if this is incoming or not
     */
    int incoming;
    /**
     * Code used to identify why a state change has been made
     *
     * These codes are only useful for when state is equal to 'session_text_disconnected'.
     * See \ref vx_evt_media_stream_updated for a description of these status codes.
     */
    int status_code;
    /**
     * Text (in English) to describe the Status Code
     *
     * See \ref vx_evt_media_stream_updated for guidelines for using this field.
     */
    char* status_string;
} vx_evt_text_stream_updated_t;

/**
* Sent when a session group is added.
*
* The XML for this event can be found here: \ref SessionGroupAddedEvent
*
* \ingroup sessiongroup
*/
typedef struct vx_evt_sessiongroup_added {
    /**
     * The common properties for all events
     */
    vx_evt_base_t base;
    /**
     * returned from successful Session Group 'create' request 
     */
    VX_HANDLE sessiongroup_handle;
    /**
     * returned from successful Session Group 'login' request 
     */
    VX_HANDLE account_handle;
    /**
    * session group type
    */
    vx_sessiongroup_type type;
} vx_evt_sessiongroup_added_t;

/**
* Sent when a session group is removed.
*
* The XML for this event can be found here: \ref SessionGroupRemovedEvent
*
* \ingroup sessiongroup
*/
typedef struct vx_evt_sessiongroup_removed {
    /**
     * The common properties for all events
     */
    vx_evt_base_t base;
    /**
     * returned from successful Session Group 'create' request 
     */
    VX_HANDLE sessiongroup_handle;
} vx_evt_sessiongroup_removed_t;

/**
* Sent when a session is added.
*
* The XML for this event can be found here: \ref SessionAddedEvent
*
* \ingroup session
*/
typedef struct vx_evt_session_added {
    /**
     * The common properties for all events
     */
    vx_evt_base_t base;
    /**
    * Handle returned from successful Session Group 'create' request 
    */
    VX_HANDLE sessiongroup_handle;
    /**
    * Handle returned from successful Session 'add' request
    */
    VX_HANDLE session_handle;
    /**
    * Full URI of the session (user/channel?)
    */
    char* uri;
    /**
    * set to 1 if this session relates to a Channel,  0 if  not
    */
    int is_channel;
    /**
     * set to 1 if this is a session that was added because it was an incoming call.
     * set to 0 for all other cases.
     */
    int incoming;
    /**
    * The name of the channel, if passed in when the channel is created.  Always empty for incoming sessions.
    */
    char* channel_name;
    /**
     * The displayname of remote uri if available
     */
    char* displayname;
    /**
     * The application of the entity requesting a session.  May be NULL or empty.
     */
    char* application;
} vx_evt_session_added_t;

/**
*Sent when a session is removed.
*
* The XML for this event can be found here: \ref SessionGroupRemovedEvent
*
* \ingroup session
*/
typedef struct vx_evt_session_removed {
    /**
     * The common properties for all events
     */
    vx_evt_base_t base;
    /**
    * Handle returned from successful Session Group 'create' request 
    */
    VX_HANDLE sessiongroup_handle;
    /**
    * Handle returned from successful Session 'add' request
    */
    VX_HANDLE session_handle;
    /**
    * Full URI of the session (user/channel?)
    */
    char* uri;
} vx_evt_session_removed_t;

/**
* Presented when a Participant is added to a session.  When joining a channel, a Participant Added Event will be raised for all active participants in the channel.
*
* The XML for this event can be found here: \ref ParticipantAddedEvent
*
* \ingroup session
*/
typedef struct vx_evt_participant_added {
    /**
     * The common properties for all events
     */
    vx_evt_base_t base;
    /**
    * Handle returned from successful Session Group 'create' request 
    */
    VX_HANDLE sessiongroup_handle;
    /**
    * Handle returned from successful Session 'create' request
    */
    VX_HANDLE session_handle;
    /**
    * The URI of the participant whose state has changed
    */
    char* participant_uri;
    /**
    * The Account name of the participant 
    */
    char* account_name;
    /**
    * The Display Name of the participant if in a channel or in a P2P initiated session.  This
    * field will not be populated for callee in a text initiated P2P session.
    */
    char* display_name;
    /**
    * DEPRECATED, WILL NOT BE IMPLEMENTED
    */
    int participant_type;
    /**
    * The name of the remote application
    */
    char* application;
} vx_evt_participant_added_t;

/**
* Presented when a participant is removed from a session.
*
* The XML for this event can be found here: \ref ParticipantRemovedEvent
*
* \ingroup session
*/
typedef struct vx_evt_participant_removed {
    /**
     * The common properties for all events
     */
    vx_evt_base_t base;
    /**
    * Handle returned from successful Session Group 'create' request 
    */
    VX_HANDLE sessiongroup_handle;
    /**
    * Handle returned from successful Session 'create' request
    */
    VX_HANDLE session_handle;
    /**
    * The URI of the participant whose state has changed
    */
    char* participant_uri;
    /**
    * The Account name of the Participant 
    */
    char* account_name;
    /**
    * The reason the participant was removed from the session.  Default is "left".  \ref vx_participant_removed_reason
    */
    vx_participant_removed_reason reason;
} vx_evt_participant_removed_t;

/**
* Indicates special state of the local voice participant that is used to indicate
* that the participant is attemping to speak while the system is in a state
* that won't transmit the participant's audio.
*/
typedef enum {
    participant_diagnostic_state_speaking_while_mic_muted = 1,
    participant_diagnostic_state_speaking_while_mic_volume_zero = 2
} vx_participant_diagnostic_state_t;
/**
* Received when the properties of the participant change (mod muted, speaking, volume, energy, typing notifications)
*
* The XML for this event can be found here: \ref ParticipantUpdatedEvent
*
* \ingroup session
*/
typedef struct vx_evt_participant_updated {
     /**
     * The common properties for all events
     */
    vx_evt_base_t base;
     /**
     * returned from successful  Session Group 'create' request 
     */
    VX_HANDLE sessiongroup_handle;
     /**
     * Handle returned from successful  Session 'create' request
     */
    VX_HANDLE session_handle;
     /**
     * The URI of the participant whose properties are being updated
     */
    char* participant_uri;
     /**
     * Used to determine if the user has been muted by the moderator (0 - not muted, 1 - muted)
     */
    int is_moderator_muted;
     /**
     * Indicates if the participant is speaking
     */
    int is_speaking;
     /**
     * This is the volume level that has been set by the user, this should not change often and is a value between 
     * 0 and 100 where 50 represents 'normal' speaking volume.
     */
    int volume;
     /**
     * This is the energy, or intensity of the participants audio.  This is used to determine how loud the user is speaking.  This is a value between 0 and 1.
     */
    double energy;
    /**
     * This indicates which media the user is participating in. See #VX_MEDIA_FLAGS_AUDIO and #VX_MEDIA_FLAGS_TEXT
     */ 
    int active_media;
    /**
    * Indicates whether or not this participant's audio is locally muted for the user
    */
    int is_muted_for_me;
    /**
    * NOT CURRENTLY IMPLEMENTED
    * Indicates whether or not this participant's text is locally muted for the user
    */
    int is_text_muted_for_me;
    /**
     * Used to determine if the user's text has been muted by the moderator (0 - not muted, 1 - muted)
     */
    int is_moderator_text_muted;
    /**
    * The type of the participant \see vx_participant_type
    */
    vx_participant_type type;
    /**
    * A list of diagnostic states to help tell the application that the participant is attempting to speak
    * but the system is not in a state to propogate that speech (mic muted etc).
    */
    vx_participant_diagnostic_state_t *diagnostic_states;
    /**
    * The total number of diagnostic states
    */
    int diagnostic_state_count;
} vx_evt_participant_updated_t;

/**
 * This event is posted after a frame has been played
 *
 * When playback has been stopped by the application, the first, current, and total frames will be equal to zero.
 *
 * The XML for this event can be found here: \ref SessionGroupPlaybackFramePlayedEvent
 *
 * \ingroup csr
 */
typedef struct vx_evt_sessiongroup_playback_frame_played {
    /*
     * The common properties for all events
     */
    vx_evt_base_t base;
    /**
     * returned from successful  Session Group 'create' request
     */
    VX_HANDLE sessiongroup_handle;
    /**
     * The sequence number of the first frame
     */
    int first_frame;
    /**
     * The current frame seqno
     */
    int current_frame;
    /**
     * The total number of frames available
     */
    int total_frames;
} vx_evt_sessiongroup_playback_frame_played_t;

/**
* Sent when a session is updated.
*
* The XML for this event can be found here: \ref SessionUpdatedEvent
*
* \ingroup session
*/
typedef struct vx_evt_session_updated {
    /**
     * The common properties for all events
     */
    vx_evt_base_t base;
    /**
    * Handle returned from successful Session Group 'create' request 
    */
    VX_HANDLE sessiongroup_handle;
    /**
    * Handle returned from successful Session 'add' request
    */
    VX_HANDLE session_handle;
    /**
    * Full URI of the session (user/channel?)
    */
    char* uri;
    /**
    * whether or not the session's audio is muted
    */
    int is_muted;
    /**
    * the volume of this session
    */
    int volume;
    /**
    * whether or not the session is transmitting
    */
    int transmit_enabled;
    /**
    * whether or not the session has focus
    */
    int is_focused;
    /**
     * The position of the virtual 'mouth'. This 3 vector is a right handed Cartesian coordinate, with the positive axis pointing
     * towards the speaker's right, the positive Y axis pointing up, and the positive Z axis pointing towards the speaker.
     */
    double speaker_position[3]; // {x, y, z}
    /**
    * The ID of the session font applied to this session.  0 = none.
    */
    int session_font_id;
    /**
    * whether or not the session's text is muted
    */
    int is_text_muted;
} vx_evt_session_updated_t;

/**
* Sent when a session group is updated.
*
* The XML for this event can be found here: \ref SessionGroupUpdatedEvent
*
* \ingroup sessiongroup
*/
typedef struct vx_evt_sessiongroup_updated {
    /**
     * The common properties for all events
     */
    vx_evt_base_t base;
    /**
     * returned from successful Session Group 'create' request 
     */
    VX_HANDLE sessiongroup_handle;
    /**
     * whether or not in delayed playback. When in delayed playback, the mic is not active
     */
    int in_delayed_playback;
    /**
     * playback speed
     */
    double current_playback_speed;
    /*
     * playback mode
     */
    vx_sessiongroup_playback_mode current_playback_mode;
    /*
     * whether or not playback is paused
     */
    int playback_paused;
    /**
     * total capacity of the loop buffer
     */
    int loop_buffer_capacity;
    /**
     * seqno of first frame in loop buffer.  This starts
     * increasing when the loop buffer fills.
     */
    int first_loop_frame;
    /**
     * total number of frames captured to loop buffer since recording
     * started.  This peaks when the loop buffer fills.
     */
    int total_loop_frames_captured;
    /**
     * sequence number of the last frame played
     */
    int last_loop_frame_played;
    /**
     * the filename currently being recorded (empty if no file being recorded)
     */
    char *current_recording_filename;
    /**
     * total frames recorded to file
     */
    int total_recorded_frames;
} vx_evt_sessiongroup_updated_t;

/**
* Received when certain media requests have completed
*
* The XML for this event can be found here: \ref MediaCompletionEvent
* \ingroup sessiongroup
*/
typedef struct vx_evt_media_completion {
     /**
     * The common properties for all events
     */
    vx_evt_base_t base;
    /**
     * Handle returned from successful SessionGroup 'create' request 
     * This field will stay empty if the completion type is 'aux_*'
     * \see vx_req_sessiongroup_create
     */
    VX_HANDLE sessiongroup_handle;
    /**
    * The type of media that has completed
    */
    vx_media_completion_type completion_type;
} vx_evt_media_completion_t;

/**
* The server may send messages to the SDK that the SDK doesn't need to consume.
* These messages will be propagated to the application via this event.  The
* application can choose to parse and consume these messages or ignore them.
*
*/
typedef struct vx_evt_server_app_data {
     /**
     * The common properties for all events
     */
    vx_evt_base_t base;
    /**
     * Handle returned from successful Account Login request
     * \see vx_req_account_login
     */
    VX_HANDLE account_handle;
    /**
    * The type of the incoming data
    */
    char* content_type;
    /**
    * The content of the message being received from the server
    */
    char* content;
} vx_evt_server_app_data_t;

/**
* This event is raised when a message from another user is received.  This is not to be
* confused with IMs... this is a peer-ro-peer communication mechanism for applications
* to communicate custom content.
* \see vx_req_account_send_user_app_data
*/
typedef struct vx_evt_user_app_data {
     /**
     * The common properties for all events
     */
    vx_evt_base_t base;
    /**
     * Handle returned from successful Account Login request
     * \see vx_req_account_login
     */
    VX_HANDLE account_handle;
    /**
    * URI of the account sending the message.
    */
    char* from_uri;
    /**
    * The type of the incoming data
    */
    char* content_type;
    /**
    * The content of the message being received from the specified account
    */
    char* content;
} vx_evt_user_app_data_t;


typedef enum {
    /**
     * message that was sent while the target user was offline.
     */
    vx_evt_network_message_type_offline_message = 1,
    vx_evt_network_message_type_admin_message = 2,
} vx_evt_network_message_type;

/**
 * This event is raised when the network sends a message to a user (as opposed to a user to user message)
 * As of this moment this includes messages that were stored and forwarded on behalf the user, and generic admin messages.
 */
typedef struct vx_evt_network_message {
     /**
     * The common properties for all events
     */
    vx_evt_base_t base;
    /**
     * Handle returned from successful Account Login request
     * \see vx_req_account_login
     */
    VX_HANDLE account_handle;
    /**
     * The type of the message
     */
    vx_evt_network_message_type network_message_type;
    /**
    * The type of the incoming data
    */
    char* content_type;
    /**
    * The content of the message being received from the server
    */
    char* content;
    /**
     * the sender of the message
     */
    char* sender_uri;
    /**
     * the sender display name
     */
    char *sender_display_name;
} vx_evt_network_message_t;

/**
 * This event is raised when the network sends a message to a user (as opposed to a user to user message)
 * As of this moment this includes messages that were stored and forwarded on behalf the user, and generic admin messages.
 */
typedef struct vx_evt_voice_service_connection_state_changed {
     /**
     * The common properties for all events
     */
    vx_evt_base_t base;
    /**
     * whether or not the voice service connection state is connected.
     */
    int connected;
    /**
     * the platform of the machine that the voice service is running on
     */
    char *platform;
    /**
     * the version of the voice service
     */
    char *version;
    /**
     * the data directory
     */
    char *data_directory;
} vx_evt_voice_service_connection_state_changed_t;

/**
 * Used to free any event of any type
 * \ingroup memorymanagement
 */
#ifndef VIVOX_TYPES_ONLY
void VIVOXSDK_DLLEXPORT destroy_evt(vx_evt_base_t *pCmd);
#endif

#ifdef __cplusplus
}
#endif

#endif /* ndef __VXCEVENTS_H__ */

