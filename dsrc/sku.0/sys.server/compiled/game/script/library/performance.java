package script.library;

import script.*;
import script.base_class.*;
import script.combat_engine.*;
import java.util.Arrays;
import java.util.Hashtable;
import java.util.Vector;
import script.base_script;


import script.library.colors;
import script.library.datatable;
import script.library.utils;
import script.library.sui;
import script.library.combat;
import script.library.camping;
import script.library.city;
import script.library.factions;
import script.library.groundquests;
import script.library.regions;
import script.library.group;
import script.library.pet_lib;
import script.library.xp;
import script.library.buff;
import script.library.prose;
import script.library.respec;
import script.library.session;
import script.library.beast_lib;


public class performance extends script.base_script
{
	public performance()
	{
	}
	public static final String DATATABLE_PERFORMANCE = "datatables/performance/performance.iff";
	public static final String DATATABLE_COL_HEAL_WOUND = "healMindWound";
	public static final String DATATABLE_COL_HEAL_SHOCK = "healShockWound";
	public static final String DATATABLE_COL_LOOP_DURATION = "loopDuration";
	public static final String DATATABLE_COL_ACTION_COST = "actionPointsPerLoop";
	public static final String DATATABLE_COL_REQ_SKILL = "requiredSkillMod";
	public static final String DATATABLE_COL_REQ_SKILL_VALUE = "requiredSkillModValue";
	public static final String DATATABLE_COL_REQ_INSTRUMENT = "requiredInstrument";
	public static final String DATATABLE_COL_REQ_SONG = "requiredSong";
	public static final String DATATABLE_COL_REQ_DANCE = "requiredDance";
	public static final String DATATABLE_COL_REQ_JUGGLE = "requiredJuggle";
	public static final String DATATABLE_COL_BASE_XP = "baseXp";
	public static final String DATATABLE_COL_FLOURISH_XP_MOD = "flourishXpMod";
	
	public static final String DATATABLE_PERFORMANCE_EFFECT = "datatables/performance/perform_effect.iff";
	public static final String DATATABLE_COL_EFFECT_PERFORM_TYPE = "performanceType";
	public static final String DATATABLE_COL_EFFECT_REQ_SKILL = "requiredSkillMod";
	public static final String DATATABLE_COL_EFFECT_REQ_SKILL_VAL = "requiredSkillModValue";
	public static final String DATATABLE_COL_EFFECT_REQ_PERFORM = "requiredPerforming";
	public static final String DATATABLE_COL_EFFECT_TARGET_TYPE = "targetType";
	public static final String DATATABLE_COL_EFFECT_DURATION = "effectDuration";
	public static final String DATATABLE_COL_EFFECT_ACTION_COST = "effectActionCost";
	
	public static final String DATATABLE_INSTRUMENT = "datatables/tangible/instrument_datatable.iff";
	public static final String DATATABLE_COL_INSTRUMENT_NAME = "serverTemplateName";
	public static final String DATATABLE_COL_INSTRUMENT_ID = "instrumentAudioId";
	public static final String DATATABLE_COL_VISIBLE_NAME = "visibleName";
	
	public static final float PERFORMANCE_HEARTBEAT_TIME = 10.0f;
	public static final float PERFORMANCE_FLOURISH_ROUNDTIME = 0.0f;
	public static final float PERFORMANCE_EFFECT_ROUNDTIME = 2.0f;
	public static final float PERFORMANCE_HEAL_RANGE = 60.0f;
	public static final float PERFORMANCE_INSPIRE_RANGE = 20.0f;
	public static final float PERFORMANCE_XP_RANGE = 30.0f;
	public static final float PERFORMANCE_OUTRO_ROUNDTIME = 15.0f;
	public static final float PERFORMANCE_BAND_MEMBER_RANGE = 50.0f;
	
	public static final int MIN_BUFF_DELAY = 60;
	public static final float INSPIRATION_BUFF_SEGMENT = 600.0f;
	public static final float MAX_INSPIRATION_TIME = 10800.0f;
	
	public static final String BUFF_PERFORM_INSPIRATION = "perform_inspire_1";
	
	public static final String MUSIC_HEARTBEAT_SCRIPT = "systems.skills.performance.active_music";
	public static final String DANCE_HEARTBEAT_SCRIPT = "systems.skills.performance.active_dance";
	public static final String JUGGLE_HEARTBEAT_SCRIPT = "systems.skills.performance.active_juggle";
	
	public static final String PERFORMANCE_SELECT = "systems.skills.performance.select_performance";
	public static final String POST_PERFORMANCE = "systems.skills.performance.post_performance";
	public static final String PERFORMANCE_ENTERTAINED_SCRIPT = "systems.skills.performance.entertained";
	
	public static final String PERFORMANCE_TYPE_DANCE = "perf_type_dance";
	public static final String PERFORMANCE_TYPE_MUSIC = "perf_type_music";
	public static final String PERFORMANCE_TYPE_JUGGLE = "perf_type_juggle";
	
	public static final String NPC_ENTERTAINMENT_MESSAGE_START = "handleEntertainmentStart";
	public static final String NPC_ENTERTAINMENT_MESSAGE_STOP = "handleEntertainmentStop";
	public static final String NPC_ENTERTAINMENT_NO_ENTERTAIN = "ai.noClap";
	
	public static final String VAR_PERFORM = "performance";
	public static final String VAR_PERFORM_FLOURISH = "performance.flourish";
	public static final String VAR_PERFORM_COVERCHARGE = "performance.covercharge";
	public static final String VAR_PERFORM_NO_PAY = "performance.no_pay";
	public static final String VAR_PERFORM_PAY_WAIT = "performance.pay_wait";
	public static final String VAR_PERFORM_PAY_AGREE = "performance.pay_agree";
	public static final String VAR_PERFORM_PAY_DISAGREE = "performance.pay_disagree";
	public static final String VAR_PERFROM_ALREADY_PAID = "performance.already_paid";
	public static final String VAR_PERFORM_INSPIRATION = "performance.inspiration";
	public static final String VAR_PERFORM_EFFECT = "performance.effect";
	public static final String VAR_PERFORM_OUTRO = "post_performance.outro";
	public static final String VAR_PERFORM_SEQUENCE = "performance.sequence";
	public static final String VAR_PERFORM_APPLAUSE_COUNT = "performance.applause_count";
	public static final String VAR_PERFORM_FLOURISH_COUNT = "performance.flourish_count";
	public static final String VAR_PERFORM_FLOURISH_TOTAL = "performance.flourish_total";
	public static final String VAR_PERFORM_FLOURISH_XP = "performance.flourish_xp";
	public static final String VAR_PERFORM_NO_BAND_FLOURISH = "setting.performance.no_band_flourish";
	public static final String VAR_AVAILABLE_PERFORMANCES = "performance.available_performances";
	public static final String VAR_SELECT_JUGGLE_CHANGE = "performance.change_juggle";
	public static final String VAR_SELECT_DANCE_CHANGE = "performance.change_dance";
	public static final String VAR_SELECT_MUSIC_CHANGE = "performance.change_music";
	public static final String VAR_SELECT_MUSIC_BAND = "performance.band_command";
	public static final String VAR_PERFORM_MOOD = "performance.old_mood";
	public static final String VAR_PERFORM_BUFF_TARGET = "performance.buff_target";
	public static final String VAR_HEALING_XP_WOUND = "performance.healing_xp.wound";
	public static final String VAR_HEALING_XP_DAMAGE = "performance.healing_xp.damage";
	public static final String VAR_HEALING_XP_BUFF = "performance.healing_xp.buff";
	public static final String VAR_TROUPE = "performance.troupe";
	public static final String VAR_CURRENT_DANCE = "performance.dance.current";
	
	public static final String VAR_BUFF_LIST = "performance.buff.list";
	public static final String VAR_BUFF_TARGET = "performance.buff.target";
	public static final String VAR_BUFF_SUI = "performance.buff.pid";
	public static final String VAR_BUFF_TYPE = "performance.buff.type";
	
	public static final int PERFORM_XP_FLOURISH_MAX = 2;
	public static final int PERFORM_XP_GROUP_MAX = 8;
	public static final int PERFORM_XP_AUDIENCE_MAX_1 = 15;
	public static final int PERFORM_XP_AUDIENCE_MAX_2 = 35;
	public static final int PERFORM_XP_APPLAUSE_MAX = 5;
	public static final int PERFORM_XP_INSPIRATION = 10;
	public static final int CURE_CLONING_SICKNESS_XP = 50;
	
	public static final float PERFORM_XP_GROUP_MOD = 0.33f;
	public static final float INSPIRATION_BONUS = .10f;
	
	public static final String[] APPLAUSE_POSTIVE =
	{
		"applaud",
		"bang",
		"cheer",
		"clap",
		"congratulate",
		"encourage",
		"softclap",
		"tiphat"
	};
	
	public static final String[] APPLAUSE_NEGATIVE =
	{
		"bored",
		"fidget",
		"frown",
		"gag",
		"groan",
		"hiss",
		"nausea",
		"puke",
		"raspberry",
		"shake",
		"wince",
		"zone"
	};
	
	public static final String[] INSPIRATION_BUFF_ENTERTAINER =
	{
		"general_inspiration",
		"artisan_inspiration",
		"entertainer_inspiration"
	};
	
	public static final String[] INSPIRATION_BUFF_DANCER =
	{
		"architect_inspiration",
		"chef_inspiration",
		"tailor_inspiration",
		"merchant_inspiration"
	};
	
	public static final String[] INSPIRATION_BUFF_MUSICIAN =
	{
		"droidengineer_inspiration",
		"weaponsmith_inspiration",
		"shipwright_inspiration",
		"armorsmith_inspiration"
	};
	
	public static final int PERFORMANCE_EFFECT_DAZZLE = 1;
	public static final int PERFORMANCE_EFFECT_DISTRACT = 2;
	public static final int PERFORMANCE_EFFECT_SPOT_LIGHT = 3;
	public static final int PERFORMANCE_EFFECT_COLOR_LIGHTS = 4;
	public static final int PERFORMANCE_EFFECT_SMOKE_BOMB = 5;
	public static final int PERFORMANCE_EFFECT_FIRE_JETS = 6;
	public static final int PERFORMANCE_EFFECT_VENTRILOQUISM = 7;
	public static final int PERFORMANCE_EFFECT_CENTER_STAGE = 8;
	public static final int PERFORMANCE_EFFECT_COLOR_SWIRL = 9;
	public static final int PERFORMANCE_EFFECT_DANCE_FLOOR = 10;
	public static final int PERFORMANCE_EFFECT_FEATURED_SOLO = 11;
	public static final int PERFORMANCE_EFFECT_FIRE_JETS2 = 12;
	public static final int PERFORMANCE_EFFECT_LASER_SHOW = 13;
	
	public static final int SPECIAL_FLOURISH_OUTRO = -1;
	public static final int SPECIAL_FLOURISH_PAUSE_MUSIC = 0;
	
	public static final int HOLOGRAM_COMMAND_START_SONG = 1;
	public static final int HOLOGRAM_COMMAND_CHANGE_SONG = 2;
	public static final int HOLOGRAM_COMMAND_STOP_SONG = 3;
	public static final int HOLOGRAM_COMMAND_START_DANCE = 4;
	public static final int HOLOGRAM_COMMAND_CHANGE_DANCE = 5;
	public static final int HOLOGRAM_COMMAND_STOP_DANCE = 6;
	
	public static final String EFFECT_SPOT_LIGHT_1 = "clienteffect/entertainer_spot_light_level_1.cef";
	public static final String EFFECT_SPOT_LIGHT_2 = "clienteffect/entertainer_spot_light_level_2.cef";
	public static final String EFFECT_SPOT_LIGHT_3 = "clienteffect/entertainer_spot_light_level_3.cef";
	public static final String EFFECT_COLOR_LIGHTS_1 = "clienteffect/entertainer_color_lights_level_1.cef";
	public static final String EFFECT_COLOR_LIGHTS_2 = "clienteffect/entertainer_color_lights_level_2.cef";
	public static final String EFFECT_COLOR_LIGHTS_3 = "clienteffect/entertainer_color_lights_level_3.cef";
	public static final String EFFECT_DAZZLE_1 = "clienteffect/entertainer_dazzle_level_1.cef";
	public static final String EFFECT_DAZZLE_2 = "clienteffect/entertainer_dazzle_level_2.cef";
	public static final String EFFECT_DAZZLE_3 = "clienteffect/entertainer_dazzle_level_3.cef";
	public static final String EFFECT_DISTRACT_1 = "clienteffect/entertainer_distract_level_1.cef";
	public static final String EFFECT_DISTRACT_2 = "clienteffect/entertainer_distract_level_2.cef";
	public static final String EFFECT_DISTRACT_3 = "clienteffect/entertainer_distract_level_3.cef";
	public static final String EFFECT_SMOKE_BOMB_1 = "clienteffect/entertainer_smoke_bomb_level_1.cef";
	public static final String EFFECT_SMOKE_BOMB_2 = "clienteffect/entertainer_smoke_bomb_level_2.cef";
	public static final String EFFECT_SMOKE_BOMB_3 = "clienteffect/entertainer_smoke_bomb_level_3.cef";
	public static final String EFFECT_FIRE_JETS_1 = "clienteffect/entertainer_fire_jets_level_1.cef";
	public static final String EFFECT_FIRE_JETS_2 = "clienteffect/entertainer_fire_jets_level_2.cef";
	public static final String EFFECT_FIRE_JETS_3 = "clienteffect/entertainer_fire_jets_level_3.cef";
	public static final String EFFECT_VENTRILOQUISM_1 = "clienteffect/entertainer_ventriloquism_level_1.cef";
	public static final String EFFECT_VENTRILOQUISM_2 = "clienteffect/entertainer_ventriloquism_level_2.cef";
	public static final String EFFECT_VENTRILOQUISM_3 = "clienteffect/entertainer_ventriloquism_level_3.cef";
	
	public static final String EFFECT_CENTER_STAGE = "clienteffect/entertainer_center_stage.cef";
	public static final String EFFECT_COLOR_SWIRL = "clienteffect/entertainer_color_swirl.cef";
	public static final String EFFECT_DANCE_FLOOR = "clienteffect/entertainer_dance_floor.cef";
	public static final String EFFECT_FEATURED_SOLO = "clienteffect/entertainer_featured_solo.cef";
	public static final String EFFECT_FIRE_JETS2 = "clienteffect/entertainer_fire_jets2.cef";
	public static final String EFFECT_LASER_SHOW = "clienteffect/entertainer_laser_show.cef";
	
	public static final string_id SID_RADIAL_PERFORMANCE_LISTEN = new string_id("radial_performance", "listen");
	public static final string_id SID_RADIAL_PERFORMANCE_WATCH = new string_id("radial_performance", "watch");
	public static final string_id SID_RADIAL_PERFORMANCE_LISTEN_STOP = new string_id("radial_performance", "listen_stop");
	public static final string_id SID_RADIAL_PERFORMANCE_WATCH_STOP = new string_id("radial_performance", "watch_stop");
	
	public static final string_id SID_ALREADY_PERFORMING_SELF = new string_id("performance", "already_performing_self");
	
	public static final string_id SID_MUSIC_NO_INSTRUMENT = new string_id("performance", "music_no_instrument");
	public static final string_id SID_MUSIC_HOLOGRAM_NO_INSTRUMENT = new string_id("performance", "music_hologram_no_instrument");
	
	public static final string_id SID_MUSIC_INVALID_SONG = new string_id("performance", "music_invalid_song");
	public static final string_id SID_MUSIC_START_SELF = new string_id("performance", "music_start_self");
	public static final string_id SID_MUSIC_START_OTHER = new string_id("performance", "music_start_other");
	public static final string_id SID_MUSIC_START_BAND_SELF = new string_id("performance", "music_start_band_self");
	public static final string_id SID_MUSIC_START_BAND_MEMBERS = new string_id("performance", "music_start_band_members");
	public static final string_id SID_MUSIC_START_BAND_OTHER = new string_id("performance", "music_start_band_other");
	public static final string_id SID_MUSIC_JOIN_BAND_SELF = new string_id("performance", "music_join_band_self");
	public static final string_id SID_MUSIC_JOIN_BAND_STOP = new string_id("performance", "music_join_band_stop");
	public static final string_id SID_MUSIC_BAND_JOIN_SONG_SELF = new string_id("performance", "music_band_join_song_self");
	public static final string_id SID_MUSIC_BAND_JOIN_SONG_MEMBERS = new string_id("performance", "music_band_join_song_members");
	public static final string_id SID_MUSIC_STOP_SELF = new string_id("performance", "music_stop_self");
	public static final string_id SID_MUSIC_STOP_OTHER = new string_id("performance", "music_stop_other");
	public static final string_id SID_MUSIC_STOP_BAND_SELF = new string_id("performance", "music_stop_band_self");
	public static final string_id SID_MUSIC_STOP_BAND_MEMBERS = new string_id("performance", "music_stop_band_members");
	public static final string_id SID_MUSIC_STOP_BAND_OTHER = new string_id("performance", "music_stop_band_other");
	public static final string_id SID_MUSIC_PREPARE_STOP_SELF = new string_id("performance", "music_prepare_stop_self");
	public static final string_id SID_MUSIC_PREPARE_STOP_BAND_SELF = new string_id("performance", "music_prepare_stop_band_self");
	public static final string_id SID_MUSIC_PREPARE_STOP_BAND_MEMBERS = new string_id("performance", "music_prepare_stop_band_members");
	public static final string_id SID_MUSIC_LISTEN_SELF = new string_id("performance", "music_listen_self");
	public static final string_id SID_MUSIC_LISTEN_STOP_SELF = new string_id("performance", "music_listen_stop_self");
	public static final string_id SID_MUSIC_LISTEN_PLAYING_SELF = new string_id("performance", "music_listen_playing_self");
	public static final string_id SID_MUSIC_LISTEN_NOT_PLAYING = new string_id("performance", "music_listen_not_playing");
	public static final string_id SID_MUSIC_LISTEN_OUT_OF_RANGE = new string_id("performance", "music_listen_out_of_range");
	public static final string_id SID_MUSIC_LISTEN_NPC = new string_id("performance", "music_listen_npc");
	public static final string_id SID_MUSIC_STILL_PLAYING = new string_id("performance", "music_still_playing");
	public static final string_id SID_MUSIC_NOT_MUSICIAN = new string_id("performance", "music_not_musician");
	public static final string_id SID_MUSIC_NOT_PERFORMING = new string_id("performance", "music_not_performing");
	public static final string_id SID_MUSIC_MUST_BE_PERFORMING_SELF = new string_id("performance", "music_must_be_performing_self");
	public static final string_id SID_MUSIC_LACK_SKILL_INSTRUMENT = new string_id("performance", "music_lack_skill_instrument");
	public static final string_id SID_MUSIC_LACK_SKILL_SONG_SELF = new string_id("performance", "music_lack_skill_song_self");
	public static final string_id SID_MUSIC_LACK_SKILL_SONG_BAND = new string_id("performance", "music_lack_skill_song_band");
	public static final string_id SID_MUSIC_LACK_SKILL_BAND_MEMBER = new string_id("performance", "music_lack_skill_band_member");
	public static final string_id SID_MUSIC_MUST_MATCH_BAND = new string_id("performance", "music_must_match_band");
	public static final string_id SID_MUSIC_FAIL = new string_id("performance", "music_fail");
	public static final string_id SID_MUSIC_OUTRO_WAIT = new string_id("performance", "music_outro_wait");
	public static final string_id SID_MUSIC_TOO_TIRED = new string_id("performance", "music_too_tired");
	public static final string_id SID_MUSIC_NO_MUSIC_PARAM = new string_id("performance", "music_no_music_param");
	public static final string_id SID_MUSIC_TRACK_NOT_AVAILABLE = new string_id("performance", "music_track_not_available");
	public static final string_id SID_MUSIC_OUT_OF_POWER = new string_id("performance", "music_out_of_power");
	
	public static final string_id SID_DANCE_START_SELF = new string_id("performance", "dance_start_self");
	public static final string_id SID_DANCE_START_OTHER = new string_id("performance", "dance_start_other");
	public static final string_id SID_DANCE_STOP_SELF = new string_id("performance", "dance_stop_self");
	public static final string_id SID_DANCE_STOP_OTHER = new string_id("performance", "dance_stop_other");
	public static final string_id SID_DANCE_WATCH_SELF = new string_id("performance", "dance_watch_self");
	public static final string_id SID_DANCE_WATCH_STOP_SELF = new string_id("performance", "dance_watch_stop_self");
	public static final string_id SID_DANCE_WATCH_NOT_DANCING = new string_id("performance", "dance_watch_not_dancing");
	public static final string_id SID_DANCE_WATCH_OUT_OF_RANGE = new string_id("performance", "dance_watch_out_of_range");
	public static final string_id SID_DANCE_WATCH_NPC = new string_id("performance", "dance_watch_npc");
	public static final string_id SID_DANCE_MUST_BE_PERFORMING_SELF = new string_id("performance", "dance_must_be_performing_self");
	public static final string_id SID_DANCE_NOT_PERFORMING = new string_id("performance", "dance_not_performing");
	public static final string_id SID_DANCE_UNKNOWN_SELF = new string_id("performance", "dance_unknown_self");
	public static final string_id SID_DANCE_LACK_SKILL_SELF = new string_id("performance", "dance_lack_skill_self");
	public static final string_id SID_DANCE_NOT_DANCER = new string_id("performance", "dance_not_dancer");
	public static final string_id SID_DANCE_TOO_TIRED = new string_id("performance", "dance_too_tired");
	public static final string_id SID_DANCE_NO_DANCE_PARAM = new string_id("performance", "dance_no_dance_param");
	public static final string_id SID_DANCE_FAIL = new string_id("performance", "dance_fail");
	public static final string_id SID_DANCE_LACK_SKILL_BAND = new string_id("performance", "dance_lack_skill_band");
	public static final string_id SID_DANCE_LACK_SKILL_BAND_MEMBER = new string_id("performance", "dance_lack_skill_band_member");
	
	public static final string_id SID_JUGGLE_START_SELF = new string_id("performance", "juggle_start_self");
	public static final string_id SID_JUGGLE_START_OTHER = new string_id("performance", "juggle_start_other");
	public static final string_id SID_JUGGLE_STOP_SELF = new string_id("performance", "juggle_stop_self");
	public static final string_id SID_JUGGLE_STOP_OTHER = new string_id("performance", "juggle_stop_other");
	public static final string_id SID_JUGGLE_WATCH_SELF = new string_id("performance", "juggle_watch_self");
	public static final string_id SID_JUGGLE_WATCH_STOP_SELF = new string_id("performance", "juggle_watch_stop_self");
	public static final string_id SID_JUGGLE_WATCH_NOT_DANCING = new string_id("performance", "juggle_watch_not_juggling");
	public static final string_id SID_JUGGLE_WATCH_OUT_OF_RANGE = new string_id("performance", "juggle_watch_out_of_range");
	public static final string_id SID_JUGGLE_WATCH_NPC = new string_id("performance", "juggle_watch_npc");
	public static final string_id SID_JUGGLE_MUST_BE_PERFORMING_SELF = new string_id("performance", "juggle_must_be_performing_self");
	public static final string_id SID_JUGGLE_NOT_PERFORMING = new string_id("performance", "juggle_not_performing");
	public static final string_id SID_JUGGLE_UNKNOWN_SELF = new string_id("performance", "juggle_unknown_self");
	public static final string_id SID_JUGGLE_LACK_SKILL_SELF = new string_id("performance", "juggle_lack_skill_self");
	public static final string_id SID_JUGGLE_NOT_JUGGLER = new string_id("performance", "juggle_not_juggler");
	public static final string_id SID_JUGGLE_TOO_TIRED = new string_id("performance", "juggle_too_tired");
	public static final string_id SID_JUGGLE_NO_JUGGLE_PARAM = new string_id("performance", "juggle_no_juggle_param");
	public static final string_id SID_JUGGLE_FAIL = new string_id("performance", "juggle_fail");
	
	public static final string_id SID_FLOURISH_WAIT_SELF = new string_id("performance", "flourish_wait_self");
	public static final string_id SID_FLOURISH_PERFORM = new string_id("performance", "flourish_perform");
	public static final string_id SID_FLOURISH_PERFORM_BAND_SELF = new string_id("performance", "flourish_perform_band_self");
	public static final string_id SID_FLOURISH_PERFORM_BAND_MEMBER = new string_id("performance", "flourish_perform_band_member");
	public static final string_id SID_FLOURISH_FORMAT = new string_id("performance", "flourish_format");
	public static final string_id SID_BAND_FLOURISH_FORMAT = new string_id("performance", "band_flourish_format");
	public static final string_id SID_BAND_SOLO_FORMAT = new string_id("performance", "band_solo_format");
	public static final string_id SID_BAND_PAUSE_FORMAT = new string_id("performance", "band_pause_format");
	public static final string_id SID_BAND_FLOURISH_ON = new string_id("performance", "band_flourish_on");
	public static final string_id SID_BAND_FLOURISH_OFF = new string_id("performance", "band_flourish_off");
	public static final string_id SID_BAND_FLOURISH_STATUS_ON = new string_id("performance", "band_flourish_status_on");
	public static final string_id SID_BAND_FLOURISH_STATUS_OFF = new string_id("performance", "band_flourish_status_off");
	public static final string_id SID_FLOURISH_NOT_PERFORMING = new string_id("performance", "flourish_not_performing");
	public static final string_id SID_FLOURISH_NOT_VALID = new string_id("performance", "flourish_not_valid");
	public static final string_id SID_FLOURISH_TOO_TIRED = new string_id("performance", "flourish_too_tired");
	public static final string_id SID_FLOURISH_INSTRUMENT_UNKNOWN = new string_id("performance", "flourish_instrument_unknown");
	
	public static final string_id SID_EFFECT_WAIT_SELF = new string_id("performance", "effect_wait_self");
	public static final string_id SID_EFFECT_NOT_PERFORMING = new string_id("performance", "effect_not_performing");
	public static final string_id SID_EFFECT_NOT_PERFORMING_CORRECT = new string_id("performance", "effect_not_performing_correct");
	public static final string_id SID_EFFECT_LACK_SKILL_SELF = new string_id("performance", "effect_lack_skill_self");
	public static final string_id SID_EFFECT_TOO_TIRED = new string_id("performance", "effect_too_tired");
	public static final string_id SID_EFFECT_NEED_TARGET = new string_id("performance", "effect_need_target");
	public static final string_id SID_EFFECT_LEVEL_TOO_HIGH = new string_id("performance", "effect_level_too_high");
	public static final string_id SID_EFFECT_PERFORM_SPOT_LIGHT = new string_id("performance", "effect_perform_spot_light");
	public static final string_id SID_EFFECT_PERFORM_COLOR_LIGHTS = new string_id("performance", "effect_perform_color_lights");
	public static final string_id SID_EFFECT_PERFORM_DAZZLE = new string_id("performance", "effect_perform_dazzle");
	public static final string_id SID_EFFECT_PERFORM_DISTRACT = new string_id("performance", "effect_perform_distract");
	public static final string_id SID_EFFECT_PERFORM_SMOKE_BOMB = new string_id("performance", "effect_perform_smoke_bomb");
	public static final string_id SID_EFFECT_PERFORM_FIRE_JETS = new string_id("performance", "effect_perform_fire_jets");
	public static final string_id SID_EFFECT_PERFORM_VENTRILOQUISM = new string_id("performance", "effect_perform_ventriloquism");
	public static final string_id SID_EFFECT_PERFORM_CENTER_STAGE = new string_id("performance", "effect_perform_center_stage");
	public static final string_id SID_EFFECT_PERFORM_COLOR_SWIRL = new string_id("performance", "effect_perform_color_swirl");
	public static final string_id SID_EFFECT_PERFORM_DANCE_FLOOR = new string_id("performance", "effect_perform_dance_floor");
	public static final string_id SID_EFFECT_PERFORM_FEATURED_SOLO = new string_id("performance", "effect_perform_featured_solo");
	public static final string_id SID_EFFECT_PERFORM_FIRE_JETS_2 = new string_id("performance", "effect_perform_fire_jets_2");
	public static final string_id SID_EFFECT_PERFORM_LASER_SHOW = new string_id("performance", "effect_perform_laser_show");
	
	public static final string_id SID_BUFF_SET_TARGET_SELF = new string_id("performance", "buff_set_target_self");
	public static final string_id SID_BUFF_SET_TARGET_OTHER = new string_id("performance", "buff_set_target_other");
	public static final string_id SID_BUFF_INVALID_TARGET_SELF = new string_id("performance", "buff_invalid_target_self");
	
	public static final string_id SID_DENY_SERVICE_ADD_SELF = new string_id("performance", "deny_service_add_self");
	public static final string_id SID_DENY_SERVICE_ADD_OTHER = new string_id("performance", "deny_service_add_other");
	public static final string_id SID_DENY_SERVICE_REMOVE_SELF = new string_id("performance", "deny_service_remove_self");
	public static final string_id SID_DENY_SERVICE_REMOVE_OTHER = new string_id("performance", "deny_service_remove_other");
	public static final string_id SID_DENY_SERVICE_COVER_CHARGE = new string_id("performance", "deny_service_cover_charge");
	
	public static final string_id SID_INSPIRE_MAX = new string_id("performance", "perform_inspire_max");
	public static final string_id SID_INSPIRE_BUILDING = new string_id("performance", "perform_inspire_building");
	
	public static final string_id SID_CC_START_CHARGE = new string_id("performance", "cc_start_charge");
	public static final string_id SID_CC_STOP_CHARGE = new string_id("performance", "cc_stop_charge");
	public static final string_id SID_CC_NO_FUNDS = new string_id("performance", "cc_no_funds");
	
	public static final string_id SID_CC_NO_SKILL = new string_id("performance", "cc_no_skill");
	public static final string_id SID_PERFORM_CC_QUERY1 = new string_id("performance", "cc_query1");
	public static final string_id SID_PERFORM_CC_QUERY2 = new string_id("performance", "cc_query2");
	public static final string_id SID_PERFORM_CC_QUERY_TITLE = new string_id("performance", "cc_query_t");
	
	public static final string_id BUFF_TIME_FAILED = new string_id("performance", "buff_time_failed");
	
	public static final string_id SHAPECHANGE = new string_id("spam","not_while_shapechanged");
	
	
	public static void stopInspiration(obj_id actor) throws InterruptedException
	{
		int inspireBuffCrc = buff.getBuffOnTargetFromGroup(actor, "inspiration");
		
		float buff_time = utils.getFloatScriptVar(actor, VAR_PERFORM_INSPIRATION);
		
		utils.removeScriptVar(actor, VAR_BUFF_TYPE);
		
		if (utils.hasScriptVar(actor, "performance.inspireMaxReached"))
		{
			utils.removeScriptVar(actor, "performance.inspireMaxReached");
		}
	}
	
	
	public static void stopListen(obj_id actor) throws InterruptedException
	{
		obj_id watchTarget = getPerformanceWatchTarget(actor);
		obj_id listenTarget = getPerformanceListenTarget(actor);
		
		stopListeningToMessage(listenTarget, "handlePerformerStartPerforming");
		stopListeningToMessage(listenTarget, "handlePerformerStopPerforming");
		removeTriggerVolume("performance_listen_volume");
		
		if (!isIdValid(watchTarget))
		{
			detachScript(actor, PERFORMANCE_ENTERTAINED_SCRIPT);
		}
		
		setPerformanceListenTarget(actor, obj_id.NULL_ID);
		checkPlayerEntertained(actor, "dancer");
		
		if (utils.hasScriptVar(actor, VAR_PERFORM_INSPIRATION))
		{
			stopInspiration(actor);
		}
		
		if (!utils.hasScriptVar(actor, VAR_PERFROM_ALREADY_PAID))
		{
			int charge = utils.getIntScriptVar(actor, performance.VAR_PERFORM_PAY_AGREE);
			money.systemPayout(money.ACCT_PERFORM_ESCROW, listenTarget, charge, "handlePayment", null);
		}
		
		utils.removeScriptVar(actor, VAR_PERFORM_PAY_AGREE);
		utils.removeScriptVar(actor, VAR_PERFORM_PAY_WAIT);
		utils.removeScriptVar(actor, VAR_PERFORM_PAY_DISAGREE);
		utils.removeScriptVar(actor, VAR_PERFROM_ALREADY_PAID);
		utils.removeScriptVar(actor, VAR_PERFORM_INSPIRATION);
		
	}
	
	
	public static void listen(obj_id actor, obj_id target) throws InterruptedException
	{
		session.logActivity(actor, session.ACTIVITY_BEEN_ENTERTAINED);
		obj_id musician = getPerformanceListenTarget(actor);
		
		LOG ("performance_listen", "Current listen target is "+ musician);
		
		if (isIdValid(musician))
		{
			stopListen(musician);
		}
		
		if (!isIdValid(target))
		{
			performanceMessageToSelf(actor, null, SID_MUSIC_LISTEN_STOP_SELF);
			stopListen(actor);
			return;
		}
		
		if (!isPlayer(target))
		{
			performanceMessageToSelf(actor, null, SID_MUSIC_LISTEN_NPC);
			return;
		}
		
		if (!hasScript(target, MUSIC_HEARTBEAT_SCRIPT))
		{
			performanceMessageToSelf(actor, target, SID_MUSIC_LISTEN_NOT_PLAYING);
			return;
		}
		
		if (utils.hasScriptVar(target, performance.VAR_PERFORM_COVERCHARGE))
		{
			if (checkDenyService(target, actor))
			{
				sendSystemMessage(actor, new string_id("performance", "deny_cant_pay_cover"));
				utils.setScriptVar(actor, performance.VAR_PERFORM_PAY_WAIT, 0);
			}
			else
			{
				int charge = utils.getIntScriptVar(target, performance.VAR_PERFORM_COVERCHARGE);
				utils.setScriptVar(actor, performance.VAR_PERFORM_PAY_WAIT, charge);
				String title = "@performance:cc_query_t";
				String prompt = "@performance:cc_query1";
				prompt += " "+ charge + " ";
				prompt += "@performance:cc_query2";
				sui.msgbox(target, actor, prompt, sui.YES_NO, title, "HandleCoverCharge");
				
				utils.setScriptVar(actor, "dancerID", target);
			}
		}
		
		setPerformanceListenTarget(actor, target);
		
		if (actor != target)
		{
			listenToMessage(target, "handlePerformerStopPerforming");
			
			createTriggerVolume("performance_listen_volume", PERFORMANCE_HEAL_RANGE, true);
			
			addTriggerVolumeEventSource("performance_listen_volume", target);
			
			startEntertainingPlayer(actor);
			utils.setScriptVar(actor, "performance.musicBuffTimer", getGameTime());
			
			if (!hasScript(actor, PERFORMANCE_ENTERTAINED_SCRIPT))
			{
				attachScript(actor, PERFORMANCE_ENTERTAINED_SCRIPT);
			}
			
			if (hasScript(target, "quest.crowd_pleaser.player_popularity"))
			{
				dictionary popularity = new dictionary();
				popularity.put("player", actor);
				messageTo(target, "handleEntertainedPlayer", popularity, 1, false);
			}
			
			if (hasScript(target, "theme_park.new_player.new_player"))
			{
				dictionary webster = new dictionary();
				webster.put("listener", actor);
				messageTo(target, "handleNewPlayerEntertainerAction", webster, 1, false);
				
				obj_id[] bandMembers = getBandMembers(target);
				if (bandMembers != null && bandMembers.length > 0)
				{
					for (int i = 0; i < bandMembers.length; i++)
					{
						testAbortScript();
						obj_id bandMate = bandMembers[i];
						if (isIdValid(bandMate))
						{
							messageTo(bandMate, "handleNewPlayerEntertainerAction", webster, 1, false);
						}
					}
				}
			}
		}
		
		if (!utils.hasScriptVar(target, performance.VAR_PERFORM_COVERCHARGE))
		{
			performanceMessageToSelf(actor, target, SID_MUSIC_LISTEN_SELF);
		}
	}
	
	
	public static void stopWatch(obj_id actor) throws InterruptedException
	{
		obj_id watchTarget = getPerformanceWatchTarget(actor);
		obj_id listenTarget = getPerformanceListenTarget(actor);
		
		stopListeningToMessage(watchTarget, "handlePerformerStartPerforming");
		stopListeningToMessage(watchTarget, "handlePerformerStopPerforming");
		removeTriggerVolume("performance_watch_volume");
		
		if (!isIdValid(listenTarget))
		{
			detachScript(actor, PERFORMANCE_ENTERTAINED_SCRIPT);
		}
		
		setPerformanceWatchTarget(actor, obj_id.NULL_ID);
		checkPlayerEntertained(actor, "musician");
		
		if (utils.hasScriptVar(actor, VAR_PERFORM_INSPIRATION))
		{
			stopInspiration(actor);
		}
		
		if (!utils.hasScriptVar(actor, VAR_PERFROM_ALREADY_PAID))
		{
			int charge = utils.getIntScriptVar(actor, performance.VAR_PERFORM_PAY_AGREE);
			money.systemPayout(money.ACCT_PERFORM_ESCROW, watchTarget, charge, "handlePayment", null);
		}
		
		utils.removeScriptVar(actor, VAR_PERFORM_PAY_AGREE);
		utils.removeScriptVar(actor, VAR_PERFORM_PAY_WAIT);
		utils.removeScriptVar(actor, VAR_PERFORM_PAY_DISAGREE);
		utils.removeScriptVar(actor, VAR_PERFROM_ALREADY_PAID);
		utils.removeScriptVar(actor, VAR_PERFORM_INSPIRATION);
		
	}
	
	
	public static void watch(obj_id actor, obj_id target) throws InterruptedException
	{
		session.logActivity(actor, session.ACTIVITY_BEEN_ENTERTAINED);
		obj_id dancer = getPerformanceWatchTarget(actor);
		
		if (isIdValid(dancer))
		{
			stopWatch(dancer);
		}
		
		if (!isIdValid(target))
		{
			performanceMessageToSelf(actor, null, SID_DANCE_WATCH_STOP_SELF);
			stopWatch(actor);
			return;
		}
		
		if (!isPlayer(target))
		{
			performanceMessageToSelf(actor, null, SID_DANCE_WATCH_NPC);
			return;
		}
		
		if (!hasScript(target, DANCE_HEARTBEAT_SCRIPT))
		{
			performanceMessageToSelf(actor, target, SID_DANCE_WATCH_NOT_DANCING);
			return;
		}
		
		if (utils.hasScriptVar(target, performance.VAR_PERFORM_COVERCHARGE))
		{
			if (checkDenyService(target, actor))
			{
				sendSystemMessage(actor, new string_id("performance", "deny_cant_pay_cover"));
				utils.setScriptVar(actor, performance.VAR_PERFORM_PAY_WAIT, 0);
			}
			else
			{
				int charge = utils.getIntScriptVar(target, performance.VAR_PERFORM_COVERCHARGE);
				utils.setScriptVar(actor, performance.VAR_PERFORM_PAY_WAIT, charge);
				String title = "@performance:cc_query_t";
				String prompt = "@performance:cc_query1";
				prompt += " "+ charge + " ";
				prompt += "@performance:cc_query2";
				sui.msgbox(target, actor, prompt, sui.YES_NO, title, "HandleCoverCharge");
				
				utils.setScriptVar(actor, "dancerID", target);
			}
		}
		
		setPerformanceWatchTarget(actor, target);
		
		listenToMessage(target, "handlePerformerStopPerforming");
		
		createTriggerVolume("performance_watch_volume", PERFORMANCE_HEAL_RANGE, true);
		
		addTriggerVolumeEventSource("performance_watch_volume", target);
		
		startEntertainingPlayer(actor);
		utils.setScriptVar(actor, "performance.dancerBuffTimer", getGameTime());
		
		if (!hasScript(actor, PERFORMANCE_ENTERTAINED_SCRIPT))
		{
			attachScript(actor, PERFORMANCE_ENTERTAINED_SCRIPT);
		}
		
		if (hasScript(target, "theme_park.new_player.new_player"))
		{
			dictionary webster = new dictionary();
			webster.put("watcher", actor);
			messageTo(target, "handleNewPlayerEntertainerAction", webster, 1, false);
		}
		
		if (hasScript(target, "quest.crowd_pleaser.player_popularity"))
		{
			dictionary popularity = new dictionary();
			popularity.put("player", actor);
			messageTo(target, "handleEntertainedPlayer", popularity, 1, false);
		}
		
		if (!utils.hasScriptVar(target, performance.VAR_PERFORM_COVERCHARGE))
		{
			performanceMessageToSelf(actor, target, SID_DANCE_WATCH_SELF);
		}
	}
	
	
	public static void covercharge(obj_id actor, int amount) throws InterruptedException
	{
		if (!hasCommand(actor, "covercharge"))
		{
			performanceMessageToSelf(actor, null, SID_CC_NO_SKILL);
			return;
		}
		
		utils.setScriptVar(actor, VAR_PERFORM_COVERCHARGE, amount);
		
		prose_package pp = new prose_package();
		pp = prose.setStringId(pp, SID_CC_START_CHARGE);
		pp = prose.setDI(pp, amount);
		sendSystemMessageProse(actor, pp);
		
		return;
	}
	
	
	public static void startEntertainingPlayer(obj_id actor) throws InterruptedException
	{
		
		setAnimationMood(actor, "entertained");
		
		if (beast_lib.getBeastOnPlayer(actor) != null)
		{
			obj_id beast = beast_lib.getBeastOnPlayer(actor);
			obj_id beastBCD = beast_lib.getBeastBCD(beast);
			messageTo (beastBCD, "ownerEntertained", null, 1, false);
		}
	}
	
	
	public static void stopEntertainingPlayer(obj_id actor) throws InterruptedException
	{
		
		setAnimationMood(actor, "default");
		
		detachScript(actor, PERFORMANCE_ENTERTAINED_SCRIPT);
	}
	
	
	public static boolean checkPlayerEntertained(obj_id actor, String performanceCheck) throws InterruptedException
	{
		LOG ("performance_entertained", "Checking to see if still being entertained");
		
		boolean entertained = false;
		
		if (performanceCheck.equals("dancer") || performanceCheck.equals("both"))
		{
			obj_id dancer = getPerformanceWatchTarget(actor);
			
			if (isIdValid(dancer))
			{
				if (getDistance(actor, dancer) < PERFORMANCE_HEAL_RANGE)
				{
					if (getPerformanceType(dancer) != 0)
					{
						LOG ("performance_entertained", "Still watching a dancer, still entertained");
						entertained = true;
					}
				}
				else
				{
					performance.performanceMessageToSelf(actor, dancer, performance.SID_DANCE_WATCH_OUT_OF_RANGE);
					performance.stopWatch(actor);
				}
			}
		}
		
		if (performanceCheck.equals("musician") || performanceCheck.equals("both"))
		{
			obj_id musician = getPerformanceListenTarget(actor);
			
			if (isIdValid(musician))
			{
				if (getDistance(actor, musician) < PERFORMANCE_HEAL_RANGE)
				{
					if (getPerformanceType(musician) != 0)
					{
						LOG ("performance_entertained", "Still listening to a musician, still entertained");
						entertained = true;
					}
				}
				else
				{
					performance.performanceMessageToSelf(actor, musician, performance.SID_MUSIC_LISTEN_OUT_OF_RANGE);
					performance.stopListen(actor);
				}
			}
		}
		
		if (!entertained)
		{
			LOG ("performance_entertained", "No performances, leaving entertained mood");
			stopEntertainingPlayer(actor);
		}
		
		return entertained;
	}
	
	
	public static int lookupPerformanceIndex(int typeHash, String performanceName, int instrumentNumber) throws InterruptedException
	{
		String lowerPerformanceName = toLower(performanceName);
		int numRows = dataTableGetNumRows(DATATABLE_PERFORMANCE);
		for (int row = 0; row < numRows; ++row)
		{
			testAbortScript();
			int rowTypeHash = dataTableGetInt(DATATABLE_PERFORMANCE, row, "type");
			if (rowTypeHash == typeHash)
			{
				int rowInstrumentNumber = dataTableGetInt(DATATABLE_PERFORMANCE, row, "instrumentAudioId");
				if (rowInstrumentNumber == instrumentNumber)
				{
					String rowPerformanceName = dataTableGetString(DATATABLE_PERFORMANCE, row, "performanceName");
					if (rowPerformanceName.equals(lowerPerformanceName))
					{
						return row+1;
					}
				}
			}
		}
		return 0;
	}
	
	
	public static boolean checkPerformanceType(int typeHash, int performanceIndex) throws InterruptedException
	{
		int rowTypeHash = dataTableGetInt(DATATABLE_PERFORMANCE, performanceIndex-1, "type");
		return (rowTypeHash == typeHash);
	}
	
	
	public static int findMatchingPerformanceIndex(int leaderPerformanceType, int instrumentNumber) throws InterruptedException
	{
		if (leaderPerformanceType == 0)
		{
			return 0;
		}
		
		String performanceName = dataTableGetString(DATATABLE_PERFORMANCE, leaderPerformanceType-1, "performanceName");
		
		int numRows = dataTableGetNumRows(DATATABLE_PERFORMANCE);
		for (int row = 0; row < numRows; ++row)
		{
			testAbortScript();
			int rowTypeHash = dataTableGetInt(DATATABLE_PERFORMANCE, row, "type");
			if (rowTypeHash == (866729052))
			{
				int rowInstrumentNumber = dataTableGetInt(DATATABLE_PERFORMANCE, row, "instrumentAudioId");
				if (rowInstrumentNumber == instrumentNumber)
				{
					String rowPerformanceName = dataTableGetString(DATATABLE_PERFORMANCE, row, "performanceName");
					if (rowPerformanceName.equals(performanceName))
					{
						return row+1;
					}
				}
			}
		}
		return 0;
	}
	
	
	public static int findMatchingDroidPerformanceIndex(int leaderType, obj_id droid) throws InterruptedException
	{
		
		if (!pet_lib.isDroidPet(droid))
		{
			return 0;
		}
		
		if (!hasObjVar(droid, "module_data.playback.tracks"))
		{
			return 0;
		}
		
		int[] tracks = getIntArrayObjVar(droid, "module_data.playback.tracks");
		
		for (int i = 0; i < tracks.length; i++)
		{
			testAbortScript();
			
			int index = tracks[i];
			index--;
			
			int instrument = dataTableGetInt(DATATABLE_PERFORMANCE, index, "instrumentAudioId");
			int match = findMatchingPerformanceIndex(leaderType, instrument);
			
			if ((match != 0) && (match == tracks[i]))
			{
				
				return match;
			}
		}
		
		return 0;
	}
	
	
	public static int getInstrumentFromPerformanceType(int type) throws InterruptedException
	{
		
		type--;
		
		int instrument = dataTableGetInt(DATATABLE_PERFORMANCE, type, "instrumentAudioId");
		
		return instrument;
	}
	
	
	public static boolean isHologramPerformer(obj_id who) throws InterruptedException
	{
		if (isPlayer(who))
		{
			return false;
		}
		
		if (utils.hasScriptVar(who, "hologram_performer"))
		{
			return true;
		}
		
		return false;
	}
	
	
	public static obj_id[] getBandMembers(obj_id who) throws InterruptedException
	{
		
		obj_id group = getGroupObject(who);
		if (isIdValid(group))
		{
			obj_id[] members = getGroupMemberIds(group);
			if (members != null)
			{
				int count = 0;
				for (int i = 0; i < members.length; ++i)
				{
					testAbortScript();
					if (isPlayer(members[i]) || isHologramPerformer(members[i]) || (pet_lib.isDroidPet(members[i]) && hasObjVar(members[i], "module_data.playback")))
					{
						++count;
					}
				}
				if (count > 0)
				{
					obj_id[] ret = new obj_id[count];
					count = 0;
					for (int i = 0; i < members.length; ++i)
					{
						testAbortScript();
						if (isPlayer(members[i]) || isHologramPerformer(members[i]) || (pet_lib.isDroidPet(members[i]) && hasObjVar(members[i], "module_data.playback")))
						{
							ret[count] = members[i];
							++count;
						}
					}
					return ret;
				}
			}
			
			return null;
		}
		
		if (isPlayer(who) || isHologramPerformer(who) || (pet_lib.isDroidPet(who) && hasObjVar(who, "module_data.playback")))
		{
			obj_id[] ret = new obj_id[1];
			ret[0] = who;
			return ret;
		}
		
		return null;
	}
	
	
	public static void startPlaying(obj_id actor, int performanceIndex, int performanceStartTime, int instrumentNumber) throws InterruptedException
	{
		dictionary params;
		
		setPerformanceType(actor, performanceIndex);
		setPerformanceStartTime(actor, performanceStartTime);
		
		if (!isIdValid(getPerformanceListenTarget(actor)))
		{
			setPerformanceListenTarget(actor, actor);
		}
		
		attachScript(actor, MUSIC_HEARTBEAT_SCRIPT);
		
		if (!pet_lib.isDroidPet(actor) && !isHologramPerformer(actor))
		{
			sendNpcsEntertainmentMessage(actor, NPC_ENTERTAINMENT_MESSAGE_START, PERFORMANCE_TYPE_MUSIC);
			
			params = new dictionary();
			params.put("performer", actor);
			broadcastMessage("handlePerformerStartPerforming", params);
		}
		
		params = new dictionary();
		params.put("performanceIndex", performanceIndex);
		params.put("instrumentNumber", instrumentNumber);
		params.put("sequence", 0);
		messageTo(actor, "OnPulse", params, PERFORMANCE_HEARTBEAT_TIME, false);
	}
	
	
	public static void stopPlaying(obj_id actor) throws InterruptedException
	{
		makeOthersStopListening(actor);
		
		int type = getPerformanceType(actor);
		
		if (type != 0)
		{
			utils.setScriptVar(actor, "performance.performance_delay", getGameTime());
		}
		
		dictionary params;
		
		if (!pet_lib.isDroidPet(actor) && !isHologramPerformer(actor))
		{
			params = new dictionary();
			params.put("performer", actor);
			params.put("check", "dancer");
			broadcastMessage("handlePerformerStopPerforming", params);
			
			dictionary d = new dictionary();
			d.put("commandType", HOLOGRAM_COMMAND_STOP_SONG);
			
			broadcastMessage("handleHologramPerformanceCommand", d);
		}
		
		messageTo(actor, "OnPerformanceEnd", null, 0.0f, false);
		detachScript(actor, performance.POST_PERFORMANCE);
		setPerformanceType(actor, 0);
		setPerformanceStartTime(actor, 0);
		
		if (utils.hasScriptVar(actor, VAR_PERFORM_INSPIRATION))
		{
			stopInspiration(actor);
		}
	}
	
	
	public static void startMusic(obj_id actor, String songName, int instrumentNumber) throws InterruptedException
	{
		session.logActivity(actor, session.ACTIVITY_ENTERTAINED);
		
		if (!pet_lib.isDroidPet(actor))
		{
			if (getPerformanceType(actor) != 0)
			{
				if (!hasScript(actor, MUSIC_HEARTBEAT_SCRIPT) && !hasScript(actor, DANCE_HEARTBEAT_SCRIPT))
				{
					stopPlaying(actor);
					
					if (hasObjVar(actor, performance.VAR_PERFORM))
					{
						removeObjVar(actor, performance.VAR_PERFORM);
					}
				}
				else
				{
					performanceMessageToSelf(actor, null, SID_ALREADY_PERFORMING_SELF);
					return;
				}
			}
			
			instrumentNumber = getInstrumentAudioId(actor);
			if (instrumentNumber == 0)
			{
				if (isHologramPerformer(actor))
				{
					obj_id master = getMaster(actor);
					performanceMessageToSelf(master, null, SID_MUSIC_HOLOGRAM_NO_INSTRUMENT);
				}
				else
				{
					performanceMessageToSelf(actor, null, SID_MUSIC_NO_INSTRUMENT);
				}
				
				return;
			}
		}
		
		if (getIntObjVar(actor, VAR_PERFORM_OUTRO) == 1)
		{
			performanceMessageToSelf(actor, null, SID_MUSIC_OUTRO_WAIT);
			return;
		}
		
		boolean activeBandSong = false;
		int performanceIndex = 0;
		int performanceStartTime = 0;
		obj_id[] bandMembers = getBandMembers(actor);
		
		for (int i = 0; i < bandMembers.length; ++i)
		{
			testAbortScript();
			obj_id player = bandMembers[i];
			if (player == actor)
			{
				continue;
			}
			
			if (!hasScript(player, MUSIC_HEARTBEAT_SCRIPT))
			{
				continue;
			}
			
			int memberPerformanceIndex = getPerformanceType(player);
			if (memberPerformanceIndex != 0)
			{
				if (!pet_lib.isDroidPet(actor))
				{
					
					performanceIndex = findMatchingPerformanceIndex(memberPerformanceIndex, instrumentNumber);
					
				}
				else
				{
					
					performanceIndex = findMatchingDroidPerformanceIndex(memberPerformanceIndex, actor);
				}
				performanceStartTime = getPerformanceStartTime(player);
				activeBandSong = true;
				break;
			}
		}
		
		if (!activeBandSong)
		{
			if (songName.length() < 1)
			{
				if (!displayAvailableMusic(actor, false, false))
				{
					performanceMessageToSelf(actor, null, SID_MUSIC_LACK_SKILL_SONG_SELF);
				}
				return;
			}
			performanceIndex = lookupPerformanceIndex((866729052), songName, instrumentNumber);
			performanceStartTime = getGameTime();
		}
		
		if (performanceIndex == 0)
		{
			if (!pet_lib.isDroidPet(actor))
			{
				performanceMessageToSelf(actor, null, SID_MUSIC_INVALID_SONG);
			}
			else
			{
				performanceMessageToMaster(actor, null, SID_MUSIC_TRACK_NOT_AVAILABLE);
			}
			
			return;
		}
		
		if (!notShapechanged(actor))
		{
			return;
		}
		
		if (!pet_lib.isDroidPet(actor) && !canUseInstrument(actor, performanceIndex))
		{
			performanceMessageToSelf(actor, null, SID_MUSIC_LACK_SKILL_INSTRUMENT);
			return;
		}
		
		if (!pet_lib.isDroidPet(actor) && !canPerformSong(actor, performanceIndex))
		{
			if (activeBandSong)
			{
				performanceMessageToSelf(actor, null, SID_MUSIC_LACK_SKILL_SONG_BAND);
			}
			else
			{
				performanceMessageToSelf(actor, null, SID_MUSIC_LACK_SKILL_SONG_SELF);
			}
			return;
		}
		
		startPlaying(actor, performanceIndex, performanceStartTime, instrumentNumber);
		
		dictionary d = new dictionary();
		d.put("commandType", HOLOGRAM_COMMAND_START_SONG);
		d.put("songName", songName);
		
		broadcastMessage("handleHologramPerformanceCommand", d);
		
		if (hasScript(actor, "theme_park.new_player.new_player"))
		{
			dictionary webster = new dictionary();
			webster.put("playingMusic", 1);
			messageTo(actor, "handleNewPlayerEntertainerAction", webster, 1, false);
		}
		
		if (!!pet_lib.isDroidPet(actor) && !isHologramPerformer(actor))
		{
			
			if (activeBandSong)
			{
				performanceMessageToSelf(actor, null, SID_MUSIC_JOIN_BAND_SELF);
			}
			else
			{
				performanceMessageToSelf(actor, null, SID_MUSIC_START_SELF);
			}
			performanceMessageToBand(actor, null, SID_MUSIC_START_OTHER);
			performanceMessageToBandListeners(actor, null, SID_MUSIC_START_OTHER);
		}
	}
	
	
	public static void startMusic(obj_id actor, String songName) throws InterruptedException
	{
		startMusic(actor, songName, 0);
	}
	
	
	public static void startBand(obj_id actor, String songName, int instrumentNumber) throws InterruptedException
	{
		
		if (!pet_lib.isDroidPet(actor))
		{
			
			instrumentNumber = getInstrumentAudioId(actor);
			if (instrumentNumber == 0)
			{
				if (isHologramPerformer(actor))
				{
					obj_id master = getMaster(actor);
					performanceMessageToSelf(master, null, SID_MUSIC_HOLOGRAM_NO_INSTRUMENT);
				}
				else
				{
					performanceMessageToSelf(actor, null, SID_MUSIC_NO_INSTRUMENT);
				}
				
				return;
			}
		}
		
		if (getIntObjVar(actor, VAR_PERFORM_OUTRO) == 1)
		{
			performanceMessageToSelf(actor, null, SID_MUSIC_OUTRO_WAIT);
			return;
		}
		
		boolean activeSong = false;
		int performanceIndex = 0;
		int performanceStartTime = 0;
		obj_id[] bandMembers = getBandMembers(actor);
		
		for (int i = 0; i < bandMembers.length; ++i)
		{
			testAbortScript();
			obj_id player = bandMembers[i];
			
			if (!hasScript(player, MUSIC_HEARTBEAT_SCRIPT))
			{
				continue;
			}
			
			int memberPerformanceIndex = getPerformanceType(player);
			if (memberPerformanceIndex != 0)
			{
				performanceIndex = findMatchingPerformanceIndex(memberPerformanceIndex, instrumentNumber);
				performanceStartTime = getPerformanceStartTime(player);
				activeSong = true;
				break;
			}
		}
		
		if (!activeSong)
		{
			if (songName.length() < 1)
			{
				if (!displayAvailableMusic(actor, true, false))
				{
					performanceMessageToSelf(actor, null, SID_MUSIC_LACK_SKILL_SONG_SELF);
				}
				return;
			}
			performanceIndex = lookupPerformanceIndex((866729052), songName, instrumentNumber);
			performanceStartTime = getGameTime();
		}
		
		if (!notShapechanged(actor))
		{
			return;
		}
		
		if (performanceIndex == 0)
		{
			performanceMessageToSelf(actor, null, SID_MUSIC_INVALID_SONG);
			return;
		}
		
		if (!canUseInstrument(actor, performanceIndex))
		{
			performanceMessageToSelf(actor, null, SID_MUSIC_LACK_SKILL_INSTRUMENT);
			return;
		}
		
		if (!canPerformSong(actor, performanceIndex))
		{
			if (activeSong)
			{
				performanceMessageToSelf(actor, null, SID_MUSIC_LACK_SKILL_SONG_BAND);
			}
			else
			{
				performanceMessageToSelf(actor, null, SID_MUSIC_LACK_SKILL_SONG_SELF);
			}
			return;
		}
		
		startPlaying(actor, performanceIndex, performanceStartTime, instrumentNumber);
		
		if (hasScript(actor, "theme_park.new_player.new_player"))
		{
			dictionary webster = new dictionary();
			webster.put("playingMusic", 1);
			messageTo(actor, "handleNewPlayerEntertainerAction", webster, 1, false);
		}
		
		boolean bandMemberLackSkill = false;
		for (int i = 0; i < bandMembers.length; ++i)
		{
			testAbortScript();
			obj_id player = bandMembers[i];
			if (player == actor)
			{
				continue;
			}
			
			if (!isPlayer(player) && !pet_lib.isDroidPet(player) && !isHologramPerformer(player))
			{
				continue;
			}
			
			if (getPerformanceType(player) != 0)
			{
				if (!hasScript(actor, MUSIC_HEARTBEAT_SCRIPT) && !hasScript(actor, DANCE_HEARTBEAT_SCRIPT))
				{
					stopPlaying(actor);
					
					if (hasObjVar(actor, performance.VAR_PERFORM))
					{
						removeObjVar(actor, performance.VAR_PERFORM);
					}
				}
				else
				{
					continue;
				}
			}
			
			if (getDistance(actor, player) > PERFORMANCE_BAND_MEMBER_RANGE)
			{
				continue;
			}
			
			int memberInstrumentNumber = 0;
			
			if (!pet_lib.isDroidPet(player))
			{
				memberInstrumentNumber = getInstrumentAudioId(player);
				if (memberInstrumentNumber == 0)
				{
					continue;
				}
			}
			
			int memberPerformanceIndex = 0;
			
			if (!pet_lib.isDroidPet(player))
			{
				memberPerformanceIndex = findMatchingPerformanceIndex(performanceIndex, memberInstrumentNumber);
			}
			else
			{
				memberPerformanceIndex = findMatchingDroidPerformanceIndex(performanceIndex, player);
			}
			
			if (memberPerformanceIndex != 0)
			{
				if (!notShapechanged(actor))
				{
					continue;
				}
				if (!pet_lib.isDroidPet(player) && !canUseInstrument(player, memberPerformanceIndex))
				{
					performanceMessageToSelf(player, null, SID_MUSIC_LACK_SKILL_INSTRUMENT);
					continue;
				}
				
				if (!pet_lib.isDroidPet(player))
				{
					
					if (canPerformSong(player, memberPerformanceIndex))
					{
						startPlaying(player, memberPerformanceIndex, performanceStartTime, memberInstrumentNumber);
					}
					else
					{
						performanceMessageToSelf(player, null, SID_MUSIC_LACK_SKILL_SONG_BAND);
						bandMemberLackSkill = true;
					}
					
				}
				else
				{
					
					memberInstrumentNumber = getInstrumentFromPerformanceType(memberPerformanceIndex);
					startPlaying(player, memberPerformanceIndex, performanceStartTime, memberInstrumentNumber);
				}
				
			}
			else
			{
				
				if (pet_lib.isDroidPet(player))
				{
					performanceMessageToMaster(player, null, SID_MUSIC_TRACK_NOT_AVAILABLE);
				}
			}
		}
		
		if (activeSong)
		{
			performanceMessageToSelf(actor, null, SID_MUSIC_BAND_JOIN_SONG_SELF);
			performanceMessageToSelf(actor, null, SID_MUSIC_BAND_JOIN_SONG_MEMBERS);
		}
		else
		{
			performanceMessageToSelf(actor, null, SID_MUSIC_START_BAND_SELF);
			performanceMessageToBand(actor, null, SID_MUSIC_START_BAND_MEMBERS);
		}
		performanceMessageToBandListeners(actor, null, SID_MUSIC_START_BAND_OTHER);
		
		if (bandMemberLackSkill)
		{
			performanceMessageToSelf(actor, null, SID_MUSIC_LACK_SKILL_BAND_MEMBER);
		}
	}
	
	
	public static void startBand(obj_id actor, String songName) throws InterruptedException
	{
		startBand(actor, songName, 0);
	}
	
	
	public static void startTroupe(obj_id actor, String danceName) throws InterruptedException
	{
		
		boolean activeDance = false;
		int performanceIndex = 0;
		int performanceStartTime = 0;
		obj_id[] bandMembers = getBandMembers(actor);
		
		for (int i = 0; i < bandMembers.length; ++i)
		{
			testAbortScript();
			obj_id player = bandMembers[i];
			
			if (hasScript(player, DANCE_HEARTBEAT_SCRIPT))
			{
				stopDance(player);
			}
		}
		
		for (int i = 0; i < bandMembers.length; ++i)
		{
			testAbortScript();
			obj_id player = bandMembers[i];
			
			danceName = utils.getStringScriptVar(player, VAR_CURRENT_DANCE);
			utils.removeScriptVar(player, VAR_CURRENT_DANCE);
			startDance(player, danceName);
		}
	}
	
	
	public static void stopMusic(obj_id actor) throws InterruptedException
	{
		
		if (hasScript(actor, DANCE_HEARTBEAT_SCRIPT))
		{
			performanceMessageToSelf(actor, null, SID_MUSIC_NOT_PERFORMING);
		}
		
		else if (hasScript(actor, MUSIC_HEARTBEAT_SCRIPT))
		{
			performanceMessageToSelf(actor, null, SID_MUSIC_PREPARE_STOP_SELF);
			
			specialFlourish(actor, SPECIAL_FLOURISH_OUTRO);
			
			setObjVar(actor, VAR_PERFORM_OUTRO, 1);
			attachScript(actor, POST_PERFORMANCE);
			messageTo(actor, "OnClearOutro", null, PERFORMANCE_OUTRO_ROUNDTIME, false);
		}
		else
		{
			stopPlaying(actor);
			
			if (hasObjVar(actor, performance.VAR_PERFORM))
			{
				removeObjVar(actor, performance.VAR_PERFORM);
			}
			
			if (hasObjVar(actor, performance.VAR_PERFORM_OUTRO))
			{
				removeObjVar(actor, performance.VAR_PERFORM_OUTRO);
			}
			
			performanceMessageToSelf(actor, null, SID_MUSIC_NOT_PERFORMING);
		}
	}
	
	
	public static void stopMusicNow(obj_id actor) throws InterruptedException
	{
		if (hasScript(actor, MUSIC_HEARTBEAT_SCRIPT))
		{
			performanceMessageToSelf(actor, null, SID_MUSIC_STOP_SELF);
			if (!isHologramPerformer(actor))
			{
				performanceMessageToBand(actor, null, SID_MUSIC_STOP_OTHER);
				performanceMessageToBandListeners(actor, null, SID_MUSIC_STOP_OTHER);
			}
		}
		
		stopPlaying(actor);
	}
	
	
	public static void stopBand(obj_id actor) throws InterruptedException
	{
		obj_id[] bandMembers = getBandMembers(actor);
		if (bandMembers != null)
		{
			performanceMessageToSelf(actor, null, SID_MUSIC_PREPARE_STOP_BAND_SELF);
			performanceMessageToBand(actor, null, SID_MUSIC_PREPARE_STOP_BAND_MEMBERS);
			
			boolean bandStop = false;
			for (int i = 0; i < bandMembers.length; ++i)
			{
				testAbortScript();
				obj_id player = bandMembers[i];
				if (hasScript(player, MUSIC_HEARTBEAT_SCRIPT))
				{
					specialFlourish(player, SPECIAL_FLOURISH_OUTRO);
					
					dictionary params = new dictionary();
					
					if (player == actor)
					{
						params.put("leader", 1 );
					}
					else
					{
						params.put("leader", 0 );
					}
					
					setObjVar(player, VAR_PERFORM_OUTRO, 1);
					attachScript(player, POST_PERFORMANCE);
					messageTo(player, "OnClearBandOutro", params, PERFORMANCE_OUTRO_ROUNDTIME, false);
				}
			}
		}
	}
	
	
	public static void changeMusic(obj_id actor, String songName, int instrumentNumber) throws InterruptedException
	{
		
		if (!hasScript(actor, MUSIC_HEARTBEAT_SCRIPT))
		{
			performanceMessageToSelf(actor, null, SID_MUSIC_MUST_BE_PERFORMING_SELF);
			return;
		}
		
		instrumentNumber = getInstrumentAudioId(actor);
		if (instrumentNumber == 0)
		{
			performanceMessageToSelf(actor, null, SID_MUSIC_NO_INSTRUMENT);
			return;
		}
		
		if (getIntObjVar(actor, VAR_PERFORM_OUTRO) == 1)
		{
			performanceMessageToSelf(actor, null, SID_MUSIC_OUTRO_WAIT);
			return;
		}
		
		boolean activeBandSong = false;
		obj_id[] bandMembers = getBandMembers(actor);
		
		for (int i = 0; i < bandMembers.length; ++i)
		{
			testAbortScript();
			obj_id player = bandMembers[i];
			if (player == actor)
			{
				continue;
			}
			
			if (!hasScript(player, MUSIC_HEARTBEAT_SCRIPT))
			{
				continue;
			}
			
			int memberPerformanceIndex = getPerformanceType(player);
			if (memberPerformanceIndex != 0)
			{
				activeBandSong = true;
				break;
			}
		}
		
		if (activeBandSong)
		{
			performanceMessageToSelf(actor, null, SID_MUSIC_MUST_MATCH_BAND);
			return;
		}
		
		if (songName.length() < 1)
		{
			if (!displayAvailableMusic(actor, false, true))
			{
				performanceMessageToSelf(actor, null, SID_MUSIC_LACK_SKILL_SONG_SELF);
			}
			return;
		}
		
		int performanceIndex = lookupPerformanceIndex((866729052), songName, instrumentNumber);
		int performanceStartTime = getGameTime();
		
		if (performanceIndex == 0)
		{
			performanceMessageToSelf(actor, null, SID_MUSIC_INVALID_SONG);
			return;
		}
		
		if (!notShapechanged(actor))
		{
			return;
		}
		
		if (!canPerformSong(actor, performanceIndex))
		{
			performanceMessageToSelf(actor, null, SID_MUSIC_LACK_SKILL_SONG_SELF);
			return;
		}
		
		setPerformanceType(actor, performanceIndex);
		queueCommand(actor, (181882275), null, "1", COMMAND_PRIORITY_IMMEDIATE);
		
		dictionary d = new dictionary();
		d.put("songName", songName);
		d.put("commandType", HOLOGRAM_COMMAND_CHANGE_SONG);
		
		broadcastMessage("handleHologramPerformanceCommand", d);
	}
	
	
	public static void changeMusic(obj_id actor, String songName) throws InterruptedException
	{
		changeMusic(actor, songName, 0);
	}
	
	
	public static void changeBandMusic(obj_id actor, String songName, int instrumentNumber) throws InterruptedException
	{
		
		if (!hasScript(actor, MUSIC_HEARTBEAT_SCRIPT))
		{
			performanceMessageToSelf(actor, null, SID_MUSIC_MUST_BE_PERFORMING_SELF);
			return;
		}
		
		instrumentNumber = getInstrumentAudioId(actor);
		if (instrumentNumber == 0)
		{
			performanceMessageToSelf(actor, null, SID_MUSIC_NO_INSTRUMENT);
			return;
		}
		
		if (getIntObjVar(actor, VAR_PERFORM_OUTRO) == 1)
		{
			performanceMessageToSelf(actor, null, SID_MUSIC_OUTRO_WAIT);
			return;
		}
		
		if (songName.length() < 1)
		{
			if (!displayAvailableMusic(actor, true, true))
			{
				performanceMessageToSelf(actor, null, SID_MUSIC_LACK_SKILL_SONG_SELF);
			}
			return;
		}
		
		int performanceIndex = lookupPerformanceIndex((866729052), songName, instrumentNumber);
		int performanceStartTime = getGameTime();
		
		if (performanceIndex == 0)
		{
			performanceMessageToSelf(actor, null, SID_MUSIC_INVALID_SONG);
			return;
		}
		
		if (!notShapechanged(actor))
		{
			return;
		}
		
		if (!canPerformSong(actor, performanceIndex))
		{
			performanceMessageToSelf(actor, null, SID_MUSIC_LACK_SKILL_SONG_SELF);
			return;
		}
		
		setPerformanceType(actor, performanceIndex);
		queueCommand(actor, (181882275), null, "1", COMMAND_PRIORITY_IMMEDIATE);
		
		obj_id[] bandMembers = getBandMembers(actor);
		
		for (int i = 0; i < bandMembers.length; ++i)
		{
			testAbortScript();
			obj_id player = bandMembers[i];
			if (player == actor)
			{
				continue;
			}
			if (!hasScript(actor, MUSIC_HEARTBEAT_SCRIPT))
			{
				continue;
			}
			
			if (!pet_lib.isDroidPet(player))
			{
				
				int memberInstrumentNumber = getInstrumentAudioId(player);
				if (memberInstrumentNumber == 0)
				{
					continue;
				}
				int memberPerformanceIndex = findMatchingPerformanceIndex(performanceIndex, memberInstrumentNumber);
				
				if (!notShapechanged(actor))
				{
					stopMusicNow(player);
				}
				
				if (memberPerformanceIndex != 0 && canPerformSong(player, memberPerformanceIndex))
				{
					
					if (!hasScript(player, MUSIC_HEARTBEAT_SCRIPT))
					{
						startPlaying(player, memberPerformanceIndex, 1, memberInstrumentNumber);
					}
					
					setPerformanceType(player, memberPerformanceIndex);
					queueCommand(player, (181882275), null, "1", COMMAND_PRIORITY_IMMEDIATE);
				}
				else
				{
					performanceMessageToSelf(player, null, SID_MUSIC_LACK_SKILL_SONG_BAND);
					stopMusicNow(player);
				}
				
				if (!canUseInstrument(player, memberInstrumentNumber))
				{
					performanceMessageToSelf(player, null, SID_MUSIC_LACK_SKILL_INSTRUMENT);
					stopMusicNow(player);
				}
				
			}
			else
			{
				
				int memberPerformanceIndex = findMatchingDroidPerformanceIndex(performanceIndex, player);
				
				if (memberPerformanceIndex != 0)
				{
					
					setPerformanceType(player, memberPerformanceIndex);
					
				}
				else
				{
					
					performanceMessageToMaster(player, null, SID_MUSIC_TRACK_NOT_AVAILABLE);
					stopMusicNow(player);
				}
			}
		}
	}
	
	
	public static void changeBandMusic(obj_id actor, String songName) throws InterruptedException
	{
		changeBandMusic(actor, songName, 0);
	}
	
	
	public static void startDance(obj_id actor, String danceName) throws InterruptedException
	{
		session.logActivity(actor, session.ACTIVITY_ENTERTAINED);
		
		if (danceName.length() < 1)
		{
			displayAvailableDances(actor, false);
			return;
		}
		
		if (getPerformanceType(actor) != 0)
		{
			performanceMessageToSelf(actor, null, SID_ALREADY_PERFORMING_SELF);
			return;
		}
		
		int performanceIndex = lookupPerformanceIndex((-1788534963), danceName, 0);
		if (performanceIndex == 0)
		{
			performanceMessageToSelf(actor, null, SID_DANCE_UNKNOWN_SELF);
			return;
		}
		
		if (!notShapechanged(actor))
		{
			return;
		}
		
		if (!canPerformDance(actor, performanceIndex))
		{
			performanceMessageToSelf(actor, null, SID_DANCE_LACK_SKILL_SELF);
			return;
		}
		
		setClientUsesAnimationLocomotion(actor, true);
		
		performanceMessageToSelf(actor, null, SID_DANCE_START_SELF);
		performanceMessageToWatchers(actor, null, SID_DANCE_START_OTHER);
		
		if (!isIdValid(getPerformanceWatchTarget(actor)))
		{
			setPerformanceWatchTarget(actor, actor);
		}
		
		dictionary params;
		int startTime = getGameTime();
		
		setPerformanceType(actor, performanceIndex);
		setPerformanceStartTime(actor, startTime);
		
		attachScript(actor, DANCE_HEARTBEAT_SCRIPT);
		
		if (!isHologramPerformer(actor))
		{
			sendNpcsEntertainmentMessage(actor, NPC_ENTERTAINMENT_MESSAGE_START, PERFORMANCE_TYPE_DANCE);
			
			params = new dictionary();
			params.put("performer", actor);
			broadcastMessage("handlePerformerStartPerforming", params);
			
			dictionary d = new dictionary();
			d.put("danceName", danceName);
			d.put("commandType", HOLOGRAM_COMMAND_START_DANCE);
			
			utils.setScriptVar(actor, VAR_CURRENT_DANCE, danceName);
			
			broadcastMessage("handleHologramPerformanceCommand", d);
			
			if (hasScript(actor, "theme_park.new_player.new_player"))
			{
				dictionary webster = new dictionary();
				webster.put("startedDancing", 1);
				messageTo(actor, "handleNewPlayerEntertainerAction", webster, 1, false);
			}
		}
		
		params = new dictionary();
		params.put("performanceIndex", performanceIndex);
		params.put("sequence", 0);
		messageTo(actor, "OnPulse", params, PERFORMANCE_HEARTBEAT_TIME, false);
	}
	
	
	public static void stopDance(obj_id actor) throws InterruptedException
	{
		int type = getPerformanceType(actor);
		
		if (type != 0)
		{
			utils.setScriptVar(actor, "performance.performance_delay", getGameTime());
		}
		
		setClientUsesAnimationLocomotion(actor, false);
		queueCommand(actor, (-1219480501), null, "0", COMMAND_PRIORITY_IMMEDIATE);
		
		if (utils.hasScriptVar(actor, "event.dance_party"))
		{
			utils.removeScriptVar(actor, "event.dance_party");
		}
		
		if (hasScript(actor, DANCE_HEARTBEAT_SCRIPT))
		{
			
			if (!isHologramPerformer(actor))
			{
				dictionary params = new dictionary();
				params.put("performer", actor);
				params.put("check", "musician");
				broadcastMessage("handlePerformerStopPerforming", params);
				
				if (!utils.hasScriptVar(actor, "stopDanceMessage"))
				{
					utils.setScriptVar(actor, "stopDanceMessage", 1);
					performanceMessageToSelf(actor, null, SID_DANCE_STOP_SELF);
					messageTo(actor, "handleStopDanceMessage", null, 1.0f, false);
					
					makeOthersStopWatching(actor);
				}
				
				performanceMessageToWatchers(actor, null, SID_DANCE_STOP_OTHER);
				
				dictionary d = new dictionary();
				d.put("commandType", HOLOGRAM_COMMAND_STOP_DANCE);
				
				broadcastMessage("handleHologramPerformanceCommand", d);
			}
			
			messageTo(actor, "OnPerformanceEnd", null, 0.0f, false);
			setPerformanceType(actor, 0);
			setPerformanceStartTime(actor, 0);
		}
		else
		{
			if (hasObjVar(actor, performance.VAR_PERFORM))
			{
				removeObjVar(actor, performance.VAR_PERFORM);
			}
			performanceMessageToSelf(actor, null, SID_DANCE_NOT_PERFORMING);
		}
		
		if (utils.hasScriptVar(actor, VAR_PERFORM_INSPIRATION))
		{
			stopInspiration(actor);
		}
	}
	
	
	public static void makeOthersStopWatching(obj_id actor) throws InterruptedException
	{
		obj_id[] watcherToStop = getPerformanceWatchersInRange(actor, PERFORMANCE_HEAL_RANGE);
		for (int i = 0; i < watcherToStop.length; i++)
		{
			testAbortScript();
			if (watcherToStop[i] != actor)
			{
				queueCommand(watcherToStop[i], (1716628890), null, "", COMMAND_PRIORITY_DEFAULT);
			}
		}
	}
	
	
	public static void makeOthersStopListening(obj_id actor) throws InterruptedException
	{
		obj_id[] watcherToStop = getPerformanceListenersInRange(actor, PERFORMANCE_HEAL_RANGE);
		for (int i = 0; i < watcherToStop.length; i++)
		{
			testAbortScript();
			if (watcherToStop[i] != actor)
			{
				queueCommand(watcherToStop[i], (-1025190704), null, "", COMMAND_PRIORITY_DEFAULT);
			}
		}
	}
	
	
	public static void changeDance(obj_id actor, String danceName) throws InterruptedException
	{
		
		if (danceName.length() < 1)
		{
			displayAvailableDances(actor, true);
			return;
		}
		
		if (!hasScript(actor, DANCE_HEARTBEAT_SCRIPT))
		{
			performanceMessageToSelf(actor, null, SID_DANCE_MUST_BE_PERFORMING_SELF);
			return;
		}
		
		int performanceIndex = lookupPerformanceIndex((-1788534963), danceName, 0);
		if (performanceIndex == 0)
		{
			performanceMessageToSelf(actor, null, SID_DANCE_UNKNOWN_SELF);
			return;
		}
		
		if (!notShapechanged(actor))
		{
			return;
		}
		
		if (!canPerformDance(actor, performanceIndex))
		{
			performanceMessageToSelf(actor, null, SID_DANCE_LACK_SKILL_SELF);
			return;
		}
		
		setPerformanceType(actor, performanceIndex);
		queueCommand(actor, (-1219480501), null, "1", COMMAND_PRIORITY_IMMEDIATE);
		
		dictionary d = new dictionary();
		d.put("danceName", danceName);
		d.put("commandType", HOLOGRAM_COMMAND_CHANGE_DANCE);
		
		broadcastMessage("handleHologramPerformanceCommand", d);
	}
	
	
	public static void joinBand(obj_id actor) throws InterruptedException
	{
		if (getPerformanceType(actor) == 0)
		{
			return;
		}
		
		boolean activeBandSong = false;
		int performanceIndex = 0;
		int performanceStartTime = 0;
		int instrumentNumber = getInstrumentAudioId(actor);
		obj_id[] bandMembers = getBandMembers(actor);
		
		for (int i = 0; i < bandMembers.length; ++i)
		{
			testAbortScript();
			obj_id player = bandMembers[i];
			if (player == actor)
			{
				continue;
			}
			
			if (!hasScript(player, MUSIC_HEARTBEAT_SCRIPT))
			{
				continue;
			}
			
			int memberPerformanceIndex = getPerformanceType(player);
			if (memberPerformanceIndex != 0)
			{
				performanceIndex = findMatchingPerformanceIndex(memberPerformanceIndex, instrumentNumber);
				performanceStartTime = getPerformanceStartTime(player);
				activeBandSong = true;
				break;
			}
		}
		
		if (activeBandSong)
		{
			if (getPerformanceType(actor) == performanceIndex)
			{
				setPerformanceStartTime(actor, performanceStartTime);
			}
			else
			{
				stopPlaying(actor);
				performanceMessageToSelf(actor, null, SID_MUSIC_JOIN_BAND_STOP);
			}
		}
		
		return;
	}
	
	
	public static void sendNpcsEntertainmentMessage(obj_id actor, String message, String perf_type) throws InterruptedException
	{
		int count = 0;
		obj_id[] npcs = getAllNpcs( getLocation(actor), 10.0f );
		if (npcs != null)
		{
			dictionary params = new dictionary();
			params.put("entertainer", actor );
			params.put("perf_type", perf_type);
			for (int i = 0; i < npcs.length; i++)
			{
				testAbortScript();
				if (hasScript( npcs[i], "ai.ai" ))
				{
					if (!utils.hasScriptVar( npcs[i], "ai.listeningTo" ))
					{
						messageTo( npcs[i], message, params, 0, false );
						count++;
						
						if (count > 4)
						{
							return;
						}
					}
				}
			}
		}
	}
	
	
	public static void flourish(obj_id actor, int index) throws InterruptedException
	{
		
		if (getPerformanceType(actor) == 0)
		{
			performanceMessageToSelf(actor, null, SID_FLOURISH_NOT_PERFORMING);
			return;
		}
		
		if ((index < 1) || (index > 8))
		{
			performanceMessageToSelf(actor, null, SID_FLOURISH_NOT_VALID);
			return;
		}
		
		if (hasObjVar(actor, VAR_PERFORM_FLOURISH_TOTAL))
		{
			int flourish_total = getIntObjVar(actor, VAR_PERFORM_FLOURISH_TOTAL);
			if (flourish_total > 4)
			{
				return;
			}
		}
		
		if (getIntObjVar(actor, VAR_PERFORM_FLOURISH) == 1)
		{
			performanceMessageToSelf(actor, null, SID_FLOURISH_WAIT_SELF);
			return;
		}
		else
		{
			setObjVar(actor, VAR_PERFORM_FLOURISH, 1);
			dictionary params = new dictionary();
			messageTo(actor, "OnClearFlourish", params, PERFORMANCE_FLOURISH_ROUNDTIME, false);
		}
		
		float flourishCost = 0;
		String entertainmentType = "";
		if (hasScript(actor, MUSIC_HEARTBEAT_SCRIPT))
		{
			flourishCost = 0.5f;
			entertainmentType = "music";
		}
		else if (hasScript(actor, DANCE_HEARTBEAT_SCRIPT))
		{
			flourishCost = 1f;
			entertainmentType = "dance";
		}
		else if (hasScript(actor, JUGGLE_HEARTBEAT_SCRIPT))
		{
			flourishCost = 1f;
			entertainmentType = "juggle";
		}
		
		if (applyPerformanceActionCost(actor, flourishCost))
		{
			performanceMessageToSelf(actor, null, SID_FLOURISH_PERFORM);
		}
		else
		{
			performanceMessageToSelf(actor, null, SID_FLOURISH_TOO_TIRED);
			return;
		}
		
		int flourish_count = getIntObjVar(actor, VAR_PERFORM_FLOURISH_COUNT);
		flourish_count++;
		
		int flourish_max = 2;
		
		LOG("entertainer_xp", "Performance Loop Duration = "+ getPerformanceLoopDuration(getPerformanceType(actor)));
		LOG("entertainer_xp", "Max Fourishes = "+ flourish_max);
		
		if (flourish_count > flourish_max)
		{
			flourish_count = flourish_max;
		}
		setObjVar(actor, VAR_PERFORM_FLOURISH_COUNT, flourish_count);
		
		flourish_count = getIntObjVar(actor, VAR_PERFORM_FLOURISH_TOTAL);
		flourish_count++;
		setObjVar(actor, VAR_PERFORM_FLOURISH_TOTAL, flourish_count);
		
		if (hasScript(actor, "theme_park.new_player.new_player"))
		{
			dictionary webster = new dictionary();
			webster.put("addingFlourish", 1);
			webster.put("entertainmentType", entertainmentType);
			messageTo(actor, "handleNewPlayerEntertainerAction", webster, 1, false);
		}
		
		dictionary params = new dictionary();
		params.put("performanceIndex", getPerformanceType(actor));
		params.put("flourishIndex", index);
		params.put("mistakeChance", getPerformanceMistakeChance(actor));
		messageTo(actor, "OnFlourish", params, 0.0f, false);
	}
	
	
	public static void specialFlourish(obj_id actor, int index) throws InterruptedException
	{
		
		if (getPerformanceType(actor) == 0)
		{
			return;
		}
		
		dictionary params = new dictionary();
		params.put("performanceIndex", getPerformanceType(actor));
		params.put("flourishIndex", index);
		messageTo(actor, "OnSpecialFlourish", params, 0.0f, false);
	}
	
	
	public static void bandFlourish(obj_id actor, int index, String instrument) throws InterruptedException
	{
		
		if (getPerformanceType(actor) == 0)
		{
			performanceMessageToSelf(actor, null, SID_FLOURISH_NOT_PERFORMING);
			return;
		}
		
		if ((index < 0) || (index > 8))
		{
			performanceMessageToSelf(actor, null, SID_FLOURISH_NOT_VALID);
			return;
		}
		
		int instrumentNum = -1;
		if (instrument != null)
		{
			
			if (!pet_lib.isDroidPet(actor))
			{
				instrumentNum = lookupInstrumentNumber(actor, instrument);
			}
			else
			{
				instrumentNum = getInstrumentFromPerformanceType(getPerformanceType(actor));
			}
		}
		
		LOG ("performance_music", "Band Flourish: "+ index + ", Instrument: "+ instrument + "("+ instrumentNum + ")");
		
		int band_size = 0;
		obj_id[] band_members = new obj_id[20];
		
		if (instrumentNum > 0)
		{
			
			obj_id group = getGroupObject(actor);
			if (isIdValid(group))
			{
				obj_id[] members = getGroupMemberIds(group);
				for (int i = 0; i < members.length; ++i)
				{
					testAbortScript();
					if (!pet_lib.isDroidPet(members[i]))
					{
						LOG ("performance_music", "My instrument is "+ getInstrumentAudioId(members[i]));
						if (hasScript(members[i], MUSIC_HEARTBEAT_SCRIPT) && (getInstrumentAudioId(members[i]) == instrumentNum))
						{
							band_members[band_size] = members[i];
							++band_size;
						}
						
					}
					else
					{
						
						int instrumentID = getInstrumentFromPerformanceType(getPerformanceType(members[i]));
						
						if (hasScript(members[i], MUSIC_HEARTBEAT_SCRIPT) && (instrumentID == instrumentNum))
						{
							band_members[band_size] = members[i];
							++band_size;
						}
					}
				}
			}
		}
		else if (instrumentNum == -1)
		{
			String heartbeat_script;
			
			if (hasScript(actor, DANCE_HEARTBEAT_SCRIPT))
			{
				heartbeat_script = DANCE_HEARTBEAT_SCRIPT;
			}
			else if (hasScript(actor, MUSIC_HEARTBEAT_SCRIPT))
			{
				heartbeat_script = MUSIC_HEARTBEAT_SCRIPT;
			}
			else if (hasScript(actor, JUGGLE_HEARTBEAT_SCRIPT))
			{
				heartbeat_script = JUGGLE_HEARTBEAT_SCRIPT;
			}
			else
			{
				performanceMessageToSelf(actor, null, SID_FLOURISH_NOT_PERFORMING);
				return;
			}
			
			obj_id group = getGroupObject(actor);
			if (isIdValid(group))
			{
				obj_id[] members = getGroupMemberIds(group);
				for (int i = 0; i < members.length; ++i)
				{
					testAbortScript();
					if (hasScript(members[i], heartbeat_script))
					{
						band_members[band_size] = members[i];
						++band_size;
					}
				}
			}
		}
		
		for (int i = 0; i < band_size; i++)
		{
			testAbortScript();
			obj_id player = band_members[i];
			
			if (hasObjVar(player, VAR_PERFORM_NO_BAND_FLOURISH))
			{
				continue;
			}
			
			if (getDistance(actor, player) > PERFORMANCE_BAND_MEMBER_RANGE)
			{
				continue;
			}
			
			if (getIntObjVar(player, VAR_PERFORM_FLOURISH) == 1)
			{
				performanceMessageToSelf(player, null, SID_FLOURISH_WAIT_SELF);
				continue;
			}
			else
			{
				setObjVar(player, VAR_PERFORM_FLOURISH, 1);
				dictionary params = new dictionary();
				messageTo(player, "OnClearFlourish", params, PERFORMANCE_FLOURISH_ROUNDTIME, false);
			}
			
			if (applyPerformanceActionCost(player, 0.5f))
			{
				performanceMessageToSelf(player, null, SID_FLOURISH_PERFORM);
			}
			else
			{
				performanceMessageToSelf(player, null, SID_FLOURISH_TOO_TIRED);
				continue;
			}
			
			if (player == actor)
			{
				int flourish_count = getIntObjVar(player, VAR_PERFORM_FLOURISH_COUNT);
				flourish_count++;
				
				int flourish_max = (int) (10 / getPerformanceLoopDuration(getPerformanceType(player)));
				if (flourish_count > flourish_max)
				{
					flourish_count = flourish_max;
				}
				setObjVar(player, VAR_PERFORM_FLOURISH_COUNT, flourish_count);
			}
			else
			{
				performanceMessageToPerson(player, actor, actor, SID_FLOURISH_PERFORM_BAND_MEMBER);
			}
			
			int flourish_count = getIntObjVar(actor, VAR_PERFORM_FLOURISH_TOTAL);
			flourish_count++;
			setObjVar(actor, VAR_PERFORM_FLOURISH_TOTAL, flourish_count);
			
			dictionary params = new dictionary();
			
			if (index == SPECIAL_FLOURISH_PAUSE_MUSIC)
			{
				specialFlourish(player, index);
				
			}
			else
			{
				params.put("performanceIndex", getPerformanceType(player));
				params.put("flourishIndex", index);
				params.put("mistakeChance", getPerformanceMistakeChance(player));
				messageTo(player, "OnFlourish", params, 0.0f, false);
				performanceMessageToSelf(actor, null, SID_FLOURISH_PERFORM_BAND_SELF);
				
			}
		}
		
	}
	
	
	public static int lookupInstrumentNumber(obj_id actor, String instrument) throws InterruptedException
	{
		int instrumentCount = dataTableGetNumRows(DATATABLE_INSTRUMENT);
		String instrumentName[] = dataTableGetStringColumn(DATATABLE_INSTRUMENT, DATATABLE_COL_VISIBLE_NAME);
		
		for (int i = 0; i < instrumentCount; i++)
		{
			testAbortScript();
			if (instrumentName[i].startsWith(instrument))
			{
				return dataTableGetInt(DATATABLE_INSTRUMENT, i, DATATABLE_COL_INSTRUMENT_ID);
			}
		}
		
		performanceMessageToSelf(actor, null, SID_FLOURISH_INSTRUMENT_UNKNOWN);
		return 0;
	}
	
	
	public static void effect(obj_id actor, int effectId, int effectLevel) throws InterruptedException
	{
		if (effectLevel < 1)
		{
			effectLevel = 3;
		}
		
		boolean mustPerform = getEffectPerformReq(effectId, effectLevel);
		int performanceIndex = getPerformanceType(actor);
		
		if (mustPerform && (performanceIndex == 0))
		{
			performanceMessageToSelf(actor, null, SID_EFFECT_NOT_PERFORMING);
			return;
		}
		
		if (getIntObjVar(actor, VAR_PERFORM_EFFECT) == 1)
		{
			performanceMessageToSelf(actor, null, SID_EFFECT_WAIT_SELF);
			return;
		}
		
		float effectRoundTime = getEffectDuration(effectId, effectLevel);
		setObjVar(actor, VAR_PERFORM_EFFECT, 1);
		dictionary params = new dictionary();
		messageTo(actor, "OnClearEffect", params, effectRoundTime, false);
		
		params.put("performanceIndex", getPerformanceType(actor));
		params.put("effectId", effectId);
		params.put("effectLevel", effectLevel);
		messageTo(actor, "OnEffect", params, 0.0f, false);
	}
	
	
	public static void playEffectTarget(obj_id self, obj_id target, String effectName, string_id effectString) throws InterruptedException
	{
		
		performanceMessageToSelf(self, null, effectString);
		
		playClientEffectObj(self, effectName, target, "");
	}
	
	
	public static void playEffectLoc(obj_id self, location loc, String effectName, string_id effectString) throws InterruptedException
	{
		performanceMessageToSelf(self, null, effectString);
		
		playClientEffectLoc(self, effectName, loc, 0.0f);
	}
	
	
	public static int getEffectIndex(int effectId, int effectLevel) throws InterruptedException
	{
		return ((effectId - 1) * 3) + (effectLevel - 1);
	}
	
	
	public static boolean getEffectPerformReq(int effectId, int effectLevel) throws InterruptedException
	{
		int effectIndex = getEffectIndex (effectId, effectLevel);
		
		return (dataTableGetInt(DATATABLE_PERFORMANCE_EFFECT, effectIndex, DATATABLE_COL_EFFECT_REQ_PERFORM) == 1);
	}
	
	
	public static String getEffectPerformanceType(int effectId, int effectLevel) throws InterruptedException
	{
		int effectIndex = getEffectIndex (effectId, effectLevel);
		
		return dataTableGetString(DATATABLE_PERFORMANCE_EFFECT, effectIndex, DATATABLE_COL_EFFECT_PERFORM_TYPE);
	}
	
	
	public static int getEffectPerformSkillLevel(int effectId, int effectLevel) throws InterruptedException
	{
		int effectIndex = getEffectIndex (effectId, effectLevel);
		
		return dataTableGetInt(DATATABLE_PERFORMANCE_EFFECT, effectIndex, DATATABLE_COL_EFFECT_REQ_SKILL_VAL);
	}
	
	
	public static float getEffectDuration(int effectId, int effectLevel) throws InterruptedException
	{
		int effectIndex = getEffectIndex (effectId, effectLevel);
		
		return dataTableGetFloat(DATATABLE_PERFORMANCE_EFFECT, effectIndex, DATATABLE_COL_EFFECT_DURATION);
	}
	
	
	public static int getEffectActionCost(int effectId, int effectLevel) throws InterruptedException
	{
		int effectIndex = getEffectIndex (effectId, effectLevel);
		
		return dataTableGetInt(DATATABLE_PERFORMANCE_EFFECT, effectIndex, DATATABLE_COL_EFFECT_ACTION_COST);
	}
	
	
	public static int getEffectTargetType(int effectId, int effectLevel) throws InterruptedException
	{
		int effectIndex = getEffectIndex (effectId, effectLevel);
		
		return dataTableGetInt(DATATABLE_PERFORMANCE_EFFECT, effectIndex, DATATABLE_COL_EFFECT_TARGET_TYPE);
	}
	
	
	public static int checkEffectLevel(obj_id actor, int effectId, int effectLevel) throws InterruptedException
	{
		boolean sendMessage = true;
		
		if (effectLevel == -1)
		{
			effectLevel = 3;
			sendMessage = false;
		}
		
		String requiredSkill = null;
		String performanceType = getEffectPerformanceType(effectId, effectLevel);
		
		if (hasScript(actor, MUSIC_HEARTBEAT_SCRIPT))
		{
			requiredSkill = "healing_music_ability";
		}
		else if (hasScript(actor, DANCE_HEARTBEAT_SCRIPT))
		{
			requiredSkill = "healing_dance_ability";
		}
		else if (hasScript(actor, JUGGLE_HEARTBEAT_SCRIPT))
		{
			requiredSkill = "healing_juggle_ability";
		}
		
		int requiredSkillVal = getEffectPerformSkillLevel(effectId, effectLevel);
		
		LOG ("entertainer_effect", "Performing Effect: "+ effectId + ", Level: "+ effectLevel);
		LOG ("entertainer_effect", "Required Skill: "+ requiredSkill);
		
		int playerSkill = getSkillStatMod(actor, requiredSkill);
		
		LOG ("entertainer_effect", "Required Value: "+ requiredSkillVal + ", Player Skill: "+ playerSkill);
		
		while ((requiredSkillVal > playerSkill) && (effectLevel > 0))
		{
			testAbortScript();
			effectLevel -= 1;
			requiredSkillVal = getEffectPerformSkillLevel(effectId, effectLevel);
			
			if (sendMessage)
			{
				performanceMessageToSelf(actor, null, SID_EFFECT_LEVEL_TOO_HIGH);
				sendMessage = false;
			}
		}
		
		return effectLevel;
		
	}
	
	
	public static void performanceMessageToPerson(obj_id person, obj_id actor, obj_id target, string_id message) throws InterruptedException
	{
		prose_package pp = new prose_package();
		pp.stringId = message;
		if (isIdValid(actor))
		{
			pp.actor.set(getEncodedName(actor));
		}
		if (isIdValid(target))
		{
			pp.target.set(getEncodedName(target));
		}
		sendSystemMessageProse(person, pp);
	}
	
	
	public static void performanceMessageToMaster(obj_id actor, obj_id target, string_id message) throws InterruptedException
	{
		if (!pet_lib.isDroidPet(actor))
		{
			return;
		}
		
		obj_id master = getMaster(actor);
		
		if (isIdValid(master))
		{
			performanceMessageToPerson(master, actor, target, message);
		}
	}
	
	
	public static void performanceMessageToSelf(obj_id actor, obj_id target, string_id message) throws InterruptedException
	{
		performanceMessageToPerson(actor, actor, target, message);
	}
	
	
	public static void performanceMessageToArray(obj_id[] people, obj_id actor, obj_id target, string_id message) throws InterruptedException
	{
		if (people != null)
		{
			for (int i = 0; i < people.length; ++i)
			{
				testAbortScript();
				if (people[i] != actor)
				{
					performanceMessageToPerson(people[i], actor, target, message);
				}
			}
		}
	}
	
	
	public static void performanceMessageToBand(obj_id actor, obj_id target, string_id message) throws InterruptedException
	{
		obj_id[] members = getBandMembers(actor);
		performanceMessageToArray(members, actor, target, message);
	}
	
	
	public static void performanceMessageToBandListeners(obj_id actor, obj_id target, string_id message) throws InterruptedException
	{
		obj_id[] members = getBandMembers(actor);
		if (members != null)
		{
			for (int i = 0; i < members.length; ++i)
			{
				testAbortScript();
				performanceMessageToListeners(members[i], target, message);
			}
		}
	}
	
	
	public static void performanceMessageToListeners(obj_id actor, obj_id target, string_id message) throws InterruptedException
	{
		performanceMessageToArray(getPerformanceListenersInRange(actor, PERFORMANCE_HEAL_RANGE), actor, target, message);
	}
	
	
	public static void performanceMessageToWatchers(obj_id actor, obj_id target, string_id message) throws InterruptedException
	{
		performanceMessageToArray(getPerformanceWatchersInRange(actor, PERFORMANCE_HEAL_RANGE), actor, target, message);
	}
	
	
	public static int getPerformanceHealWoundMod(int perf_index) throws InterruptedException
	{
		if (perf_index == 0)
		{
			return 0;
		}
		return dataTableGetInt(DATATABLE_PERFORMANCE, perf_index - 1, "healMindWound");
	}
	
	
	public static int getPerformanceHealShockMod(int perf_index) throws InterruptedException
	{
		if (perf_index == 0)
		{
			return 0;
		}
		return dataTableGetInt(DATATABLE_PERFORMANCE, perf_index - 1, DATATABLE_COL_HEAL_SHOCK);
	}
	
	
	public static int getPerformanceActionCost(int perf_index) throws InterruptedException
	{
		if (perf_index == 0)
		{
			return 0;
		}
		return dataTableGetInt(DATATABLE_PERFORMANCE, perf_index - 1, DATATABLE_COL_ACTION_COST);
	}
	
	
	public static float getPerformanceLoopDuration(int perf_index) throws InterruptedException
	{
		if (perf_index == 0)
		{
			return 0;
		}
		return dataTableGetFloat(DATATABLE_PERFORMANCE, perf_index - 1, DATATABLE_COL_LOOP_DURATION);
	}
	
	
	public static int getPerformanceBaseXp(int perf_index) throws InterruptedException
	{
		if (perf_index == 0)
		{
			return 0;
		}
		return dataTableGetInt(DATATABLE_PERFORMANCE, perf_index - 1, DATATABLE_COL_BASE_XP);
	}
	
	
	public static int getFlourishXpMod(int perf_index) throws InterruptedException
	{
		if (perf_index == 0)
		{
			return 0;
		}
		return dataTableGetInt(DATATABLE_PERFORMANCE, perf_index - 1, DATATABLE_COL_FLOURISH_XP_MOD);
	}
	
	
	public static boolean performanceHeal(obj_id actor, String perf_type, float modifier) throws InterruptedException
	{
		
		if (!isIdValid(actor))
		{
			return false;
		}
		
		if (!isPlayer(actor))
		{
			return false;
		}
		
		int can_heal = canPerformanceHeal(actor);
		if (can_heal == 0)
		{
			return false;
		}
		
		obj_id[] audience;
		
		if (perf_type.equals(PERFORMANCE_TYPE_DANCE))
		{
			audience = getPerformanceWatchersInRange(actor, PERFORMANCE_INSPIRE_RANGE);
		}
		else if (perf_type.equals(PERFORMANCE_TYPE_MUSIC))
		{
			audience = getPerformanceListenersInRange(actor, PERFORMANCE_INSPIRE_RANGE);
		}
		else
		{
			return false;
		}
		
		if (audience.length < 1)
		{
			return false;
		}
		
		int audienceMod = 0;
		
		prose_package pt = new prose_package();
		pt.target.set(actor);
		pt.stringId = new string_id("spam", "cured_clonesick");
		
		prose_package pa = new prose_package();
		pa.stringId = new string_id("spam", "cured_clonesick_actor");
		
		for (int i = 0; i < audience.length; i++)
		{
			testAbortScript();
			if (!isIdValid(audience[i]) || !exists(audience[i]))
			{
				continue;
			}
			
			if (checkDenyService(actor, audience[i]))
			{
				continue;
			}
			
			if (utils.hasScriptVar(audience[i], VAR_PERFORM_PAY_WAIT))
			{
				continue;
			}
			
			if (buff.hasBuff(audience[i], "cloning_sickness"))
			{
				
				buff.removeBuff(audience[i], "cloning_sickness");
				playClientEffectObj(audience[i], "appearance/pt_heal.prt", audience[i], "");
				
				if (!utils.hasScriptVar(audience[i], VAR_PERFROM_ALREADY_PAID))
				{
					int charge = utils.getIntScriptVar(audience[i], performance.VAR_PERFORM_PAY_AGREE);
					if (charge > 0)
					{
						money.systemPayout(money.ACCT_PERFORM_ESCROW, actor, charge, "handlePayment", null);
						utils.setScriptVar(audience[i], VAR_PERFROM_ALREADY_PAID, 1);
					}
				}
				
				if (audience[i] != actor)
				{
					sendSystemMessageProse(audience[i], pt);
				}
				
				pa.target.set(audience[i]);
				sendSystemMessageProse(actor, pa);
				audienceMod++;
			}
			
			if (buff.hasBuff(audience[i],"gcw_fatigue"))
			{
				if (groundquests.isQuestActive(actor, "gcw_entertain_fatigue") && factions.isSameFactionorFactionHelper(actor, audience[i]))
				{
					pt.stringId = new string_id("spam", "cured_fatigue");
					pa.stringId = new string_id("spam", "cured_fatigued_actor");
					
					int stackSize = (int)buff.getBuffStackCount(audience[i], "gcw_fatigue");
					if (stackSize <= 0)
					{
						continue;
					}
					
					else if (stackSize <= 5)
					{
						
						buff.removeBuff(audience[i], "gcw_fatigue");
					}
					
					else if (stackSize > 5)
					{
						stackSize = stackSize -5;
						buff.removeBuff(audience[i], "gcw_fatigue");
						buff.applyBuffWithStackCount(audience[i], "gcw_fatigue", stackSize);
					}
					
					groundquests.sendSignal(actor, "cureFatigue");
					
					obj_id cityEggId = gcw.getInvasionSequencerNearby(actor);
					if (isValidId(cityEggId))
					{
						trial.addNonInstanceFactionParticipant(actor, cityEggId);
					}
					
					prose_package gcw_fatigue_pt = new prose_package();
					pt.target.set(actor);
					pt.stringId = new string_id("spam", "cured_fatigue");
					
					prose_package gcw_fatigue_pa = new prose_package();
					pa.stringId = new string_id("spam", "cured_fatigued_actor");
					
					playClientEffectObj(audience[i], "appearance/pt_heal.prt", audience[i], "");
					
					if (!utils.hasScriptVar(audience[i], VAR_PERFROM_ALREADY_PAID))
					{
						int charge = utils.getIntScriptVar(audience[i], performance.VAR_PERFORM_PAY_AGREE);
						if (charge > 0)
						{
							money.systemPayout(money.ACCT_PERFORM_ESCROW, actor, charge, "handlePayment", null);
							utils.setScriptVar(audience[i], VAR_PERFROM_ALREADY_PAID, 1);
						}
					}
					
					if (audience[i] != actor)
					{
						sendSystemMessageProse(audience[i], pt);
					}
					
					pa.target.set(audience[i]);
					sendSystemMessageProse(actor, pa);
					audienceMod++;
				}
			}
			
		}
		
		int experience = (CURE_CLONING_SICKNESS_XP * audienceMod);
		obj_id[] band = getBandMembers(actor);
		
		for (int i = 0; i < band.length; i++)
		{
			testAbortScript();
			xp.grantSocialStyleXp(band[i], xp.ENTERTAINER, experience);
		}
		
		return true;
	}
	
	
	public static float inspireGetMaxDuration(obj_id actor) throws InterruptedException
	{
		int can_heal = canPerformanceHeal(actor);
		
		float pctComplete = respec.getPercentageCompletion(actor, getSkillTemplate(actor));
		float totInspireSkill = (pctComplete * 300.0f);
		
		int city_id = city.checkCity(actor, false);
		if (city_id > 0 && (city.cityHasSpec(city_id, city.SF_SPEC_ENTERTAINER)))
		{
			totInspireSkill = totInspireSkill * 2.2f;
		}
		
		float expertiseDurationIncreaseMod = getEnhancedSkillStatisticModifierUncapped(actor, "expertise_en_inspire_buff_duration_increase")*60;
		float maxDuration = (12100.0f - (12100.0f * (1.0f / ((totInspireSkill + (1.0f / 0.01f)) * 0.01f)))) + 3600.0f;
		
		maxDuration += expertiseDurationIncreaseMod;
		
		obj_id camp = camping.getCurrentAdvancedCamp(actor);
		
		if (isIdValid(camp))
		{
			if (camping.isInEntertainmentCamp(actor, camp))
			{
				float campEffectiveness = getFloatObjVar(camp, "modules.entertainer");
				if (campEffectiveness < 1.0f)
				{
					maxDuration = maxDuration * campEffectiveness;
				}
			}
		}
		
		if (can_heal == 3)
		{
			maxDuration = maxDuration * 0.2f;
		}
		
		return maxDuration;
	}
	
	
	public static boolean inspire(obj_id actor, String perf_type) throws InterruptedException
	{
		if (!isIdValid(actor))
		{
			return false;
		}
		
		if (!isPlayer(actor))
		{
			return false;
		}
		
		int can_heal = canPerformanceHeal(actor);
		if (can_heal == 0)
		{
			return false;
		}
		
		obj_id[] audience;
		
		if (perf_type.equals(PERFORMANCE_TYPE_DANCE))
		{
			audience = getPerformanceWatchersInRange(actor, PERFORMANCE_INSPIRE_RANGE);
		}
		else if (perf_type.equals(PERFORMANCE_TYPE_MUSIC))
		{
			audience = getPerformanceListenersInRange(actor, PERFORMANCE_INSPIRE_RANGE);
		}
		else
		{
			return false;
		}
		
		if (audience.length < 1)
		{
			return false;
		}
		
		float maxDuration = inspireGetMaxDuration(actor);
		
		int audienceMod = 0;
		for (int i = 0; i < audience.length; i++)
		{
			testAbortScript();
			if (audience[i] == actor)
			{
				continue;
			}
			
			if (checkDenyService(actor, audience[i]))
			{
				continue;
			}
			
			if (utils.hasScriptVar(audience[i], VAR_PERFORM_PAY_WAIT))
			{
				continue;
			}
			
			float inspirationScriptVarVal = 0.0f;
			
			if (utils.hasScriptVar(audience[i], VAR_PERFORM_INSPIRATION))
			{
				inspirationScriptVarVal = utils.getFloatScriptVar(audience[i], VAR_PERFORM_INSPIRATION);
			}
			
			if (inspirationScriptVarVal == maxDuration)
			{
				continue;
			}
			
			int inspireBuffCrc = buff.getBuffOnTargetFromGroup(audience[i], "inspiration");
			
			float expertisePerTickBonusTime = getEnhancedSkillStatisticModifierUncapped(actor, "expertise_en_inspire_pulse_duration_increase")*60;
			
			float perTickMinutesAdded = expertisePerTickBonusTime + INSPIRATION_BUFF_SEGMENT;
			
			int city_id = city.checkCity(actor, false);
			if (city_id > 0 && (city.cityHasSpec(city_id, city.SF_SPEC_ENTERTAINER)))
			{
				perTickMinutesAdded += 180;
			}
			
			inspirationScriptVarVal += perTickMinutesAdded;
			
			if (inspirationScriptVarVal > maxDuration)
			{
				inspirationScriptVarVal = maxDuration;
				
				if (!utils.hasScriptVar(audience[i], "performance.inspireMaxReached"))
				{
					showFlyTextPrivate(audience[i], audience[i], new string_id("performance", "buff_maxtime"), 0.66f, colors.LIGHTPINK.getR(), colors.LIGHTPINK.getG(), colors.LIGHTPINK.getB(), true);
					showFlyTextPrivate(audience[i], actor, new string_id("performance", "buff_maxtime"), 0.66f, colors.LIGHTPINK.getR(), colors.LIGHTPINK.getG(), colors.LIGHTPINK.getB(), true);
					utils.setScriptVar(audience[i], "performance.inspireMaxReached", 1);
				}
			}
			else
			{
				int perTickMinutesAddedInt = (int)perTickMinutesAdded/60;
				string_id strSpam = new string_id("spam", "buff_duration_tick_not_hardcoded");
				prose_package pp = new prose_package();
				pp = prose.setStringId(pp, strSpam);
				pp = prose.setDI(pp, perTickMinutesAddedInt);
				
				showFlyTextPrivate(audience[i], audience[i], pp, 0.66f, colors.LIGHTPINK.getR(), colors.LIGHTPINK.getG(), colors.LIGHTPINK.getB(), true);
				showFlyTextPrivate(audience[i], actor, pp, 0.66f, colors.LIGHTPINK.getR(), colors.LIGHTPINK.getG(), colors.LIGHTPINK.getB(), true);
				
			}
			
			utils.setScriptVar(audience[i], VAR_PERFORM_INSPIRATION, inspirationScriptVarVal);
			audienceMod++;
		}
		
		int experience = (PERFORM_XP_INSPIRATION * audienceMod);
		obj_id[] band = getBandMembers(actor);
		
		for (int i = 0; i < band.length; i++)
		{
			testAbortScript();
			xp.grantSocialStyleXp(band[i], xp.ENTERTAINER, experience);
		}
		
		return true;
	}
	
	
	public static boolean performanceTargetedBuffFlourish(obj_id actor, String perf_type, float modifier) throws InterruptedException
	{
		
		return true;
	}
	
	
	public static boolean performanceSetPerformanceBuffTarget(obj_id actor, String target) throws InterruptedException
	{
		obj_id targetId = null;
		
		if (!isIdValid(targetId))
		{
			performanceMessageToSelf(actor, null, SID_BUFF_INVALID_TARGET_SELF);
			
			return false;
		}
		
		return performanceSetPerformanceBuffTarget(actor, targetId);
	}
	
	
	public static boolean performanceSetPerformanceBuffTarget(obj_id actor, obj_id target) throws InterruptedException
	{
		if (!isIdValid(target))
		{
			performanceMessageToSelf(actor, null, SID_BUFF_INVALID_TARGET_SELF);
			return false;
		}
		
		utils.setScriptVar(actor, VAR_PERFORM_BUFF_TARGET, target);
		
		performanceMessageToSelf(actor, target, SID_BUFF_SET_TARGET_SELF);
		performanceMessageToPerson(target, actor, target, SID_BUFF_SET_TARGET_OTHER);
		
		return true;
	}
	
	
	public static boolean applyPerformanceActionCost(obj_id actor, float modifier) throws InterruptedException
	{
		
		return true;
	}
	
	
	public static float getPerformanceMistakeChance(obj_id actor) throws InterruptedException
	{
		
		return 0f;
	}
	
	
	public static boolean hasInspirationInstrumentUseBuff(obj_id actor, String instrument) throws InterruptedException
	{
		int col = 0;
		
		if (utils.hasScriptVar(actor, "buff.musician_inspiration.value"))
		{
			col = 2;
		}
		else if (utils.hasScriptVar(actor, "buff.entertainer_inspiration.value"))
		{
			col = 1;
		}
		else
		{
			return false;
		}
		
		String required = dataTableGetString("datatables/buff/entertainment_buff.iff", instrument, col);
		
		if (required.equals("none"))
		{
			return true;
		}
		
		return hasCommand(actor, required);
	}
	
	
	public static boolean hasInspirationDancePropUseBuff(obj_id actor, String prop) throws InterruptedException
	{
		int col = 0;
		
		if (utils.hasScriptVar(actor, "buff.dancer_inspiration.value"))
		{
			col = 2;
		}
		else if (utils.hasScriptVar(actor, "buff.entertainer_inspiration.value"))
		{
			col = 1;
		}
		else
		{
			return false;
		}
		
		String required = dataTableGetString("datatables/buff/entertainment_buff.iff", prop, col);
		
		if (required == null || required.equals(""))
		{
			return false;
		}
		
		if (required.equals("none"))
		{
			return true;
		}
		
		return hasCommand(actor, required);
	}
	
	
	public static boolean notShapechanged(obj_id actor) throws InterruptedException
	{
		
		int shapechange = buff.getBuffOnTargetFromGroup(actor, "shapechange");
		
		if (shapechange != 0)
		{
			sendSystemMessage(actor, SHAPECHANGE);
			return false;
		}
		return true;
	}
	
	
	public static boolean canDoPerformance(obj_id actor, int perf_index) throws InterruptedException
	{
		if (perf_index == 0)
		{
			return false;
		}
		
		if (isHologramPerformer(actor))
		{
			obj_id master = getMaster(actor);
			
			if (isIdValid(master))
			{
				return canDoPerformance(master, perf_index);
			}
			else
			{
				return false;
			}
		}
		
		String req_skill_mod = dataTableGetString(DATATABLE_PERFORMANCE, perf_index - 1, DATATABLE_COL_REQ_SKILL);
		int req_skill_value = dataTableGetInt(DATATABLE_PERFORMANCE, perf_index - 1, DATATABLE_COL_REQ_SKILL_VALUE);
		int player_skill_mod = getSkillStatMod(actor, req_skill_mod);
		if (player_skill_mod < req_skill_value)
		{
			return false;
		}
		return true;
	}
	
	
	public static boolean canUseInstrument(obj_id actor, int perf_index) throws InterruptedException
	{
		
		if (perf_index == 0)
		{
			
			return false;
		}
		
		if (isHologramPerformer(actor))
		{
			obj_id master = getMaster(actor);
			
			if (isIdValid(master))
			{
				return canUseInstrument(master, perf_index);
			}
			else
			{
				
				return false;
			}
		}
		
		String req_ability = dataTableGetString(DATATABLE_PERFORMANCE, perf_index - 1, DATATABLE_COL_REQ_INSTRUMENT);
		
		if (req_ability == null || req_ability.equals(""))
		{
			
			return false;
		}
		
		String[] splitAbility = split(req_ability, '_');
		req_ability = splitAbility[0];
		
		if (hasCommand(actor, req_ability) || hasInspirationInstrumentUseBuff(actor, req_ability))
		{
			return true;
		}
		
		return false;
	}
	
	
	public static boolean canPerformSong(obj_id actor, int perf_index) throws InterruptedException
	{
		if (perf_index == 0)
		{
			return false;
		}
		
		if (isHologramPerformer(actor))
		{
			obj_id master = getMaster(actor);
			
			if (isIdValid(master))
			{
				return canPerformSong(master, perf_index);
			}
			else
			{
				return false;
			}
		}
		
		String req_ability = dataTableGetString(DATATABLE_PERFORMANCE, perf_index - 1, DATATABLE_COL_REQ_SONG);
		LOG ("performance", "Required song = "+ req_ability);
		if (req_ability == null || req_ability.equals(""))
		{
			return false;
		}
		
		if (req_ability.indexOf("dirge") > -1)
		{
			if (!utils.isProfession(actor, utils.ENTERTAINER))
			{
				return false;
			}
			
			else if (getLevel(actor) < 82)
			{
				return false;
			}
		}
		
		if (!hasCommand(actor, req_ability))
		{
			return false;
		}
		return true;
	}
	
	
	public static boolean canPerformDance(obj_id actor, int perf_index) throws InterruptedException
	{
		if (perf_index == 0)
		{
			return false;
		}
		
		if (isHologramPerformer(actor))
		{
			obj_id master = getMaster(actor);
			
			if (isIdValid(master))
			{
				return canPerformDance(master, perf_index);
			}
			else
			{
				return false;
			}
		}
		
		if (utils.hasScriptVar(actor, "event.dance_party"))
		{
			return true;
		}
		
		String req_ability = dataTableGetString(DATATABLE_PERFORMANCE, perf_index - 1, DATATABLE_COL_REQ_DANCE);
		LOG ("performance", "Required dance = "+ req_ability);
		if (req_ability == null || req_ability.equals(""))
		{
			return false;
		}
		
		if (!hasCommand(actor, req_ability))
		{
			return false;
		}
		return true;
	}
	
	
	public static boolean canPerformJuggle(obj_id actor, int perf_index) throws InterruptedException
	{
		if (perf_index == 0)
		{
			return false;
		}
		
		if (isHologramPerformer(actor))
		{
			return true;
		}
		
		String req_ability = dataTableGetString(DATATABLE_PERFORMANCE, perf_index - 1, DATATABLE_COL_REQ_JUGGLE);
		LOG ("performance", "Required juggle = "+ req_ability);
		if (req_ability == null || req_ability.equals(""))
		{
			return false;
		}
		
		if (!hasCommand(actor, req_ability))
		{
			return false;
		}
		return true;
	}
	
	
	public static boolean isValidDanceProp(obj_id object) throws InterruptedException
	{
		if (!isValidId(object))
		{
			return false;
		}
		
		String template = getTemplateName(object);
		
		return isGameObjectTypeOf(object, GOT_misc_instrument) && template.startsWith("object/tangible/dance_prop/");
	}
	
	
	public static boolean isDancePropCertified(obj_id player, obj_id object) throws InterruptedException
	{
		if (!isValidDanceProp(object))
		{
			return true;
		}
		
		if (hasObjVar(object, "nocert") && hasSkill(player, "social_entertainer_novice"))
		{
			return true;
		}
		
		String template = getTemplateName(object);
		
		String command = template.substring((template.lastIndexOf("/") + 1), template.lastIndexOf("_"));
		
		if (hasCommand(player, command))
		{
			return true;
		}
		else if (hasInspirationDancePropUseBuff(player, command))
		{
			return true;
		}
		
		return false;
	}
	
	
	public static int calcPerformanceXp(obj_id actor) throws InterruptedException
	{
		if (!isIdValid(actor))
		{
			return 0;
		}
		
		if (!isPlayer(actor))
		{
			return 0;
		}
		
		int perf_index = getPerformanceType(actor);
		int flourish_xp = performance.getFlourishXpMod(perf_index);
		int base_xp = performance.getPerformanceBaseXp(perf_index);
		
		LOG ("entertainer_xp", "Performance Index = "+ perf_index);
		LOG ("entertainer_xp", "Initial Flourish XP = "+ flourish_xp);
		
		int band_size = 1;
		obj_id[] band_members = new obj_id[20];
		band_members[0] = actor;
		
		obj_id group = getGroupObject(actor);
		if (isIdValid(group))
		{
			obj_id[] members = getGroupMemberIds(group);
			for (int i = 0; i < members.length; ++i)
			{
				testAbortScript();
				if (pet_lib.isDroidPet(members[i]) || isHologramPerformer(members[i]))
				{
					continue;
				}
				
				if (getDistance(actor, members[i]) > PERFORMANCE_BAND_MEMBER_RANGE)
				{
					continue;
				}
				
				if ((members[i] != actor && (hasScript(members[i], DANCE_HEARTBEAT_SCRIPT) || hasScript(members[i], MUSIC_HEARTBEAT_SCRIPT) || hasScript(members[i], JUGGLE_HEARTBEAT_SCRIPT))))
				{
					band_members[band_size] = members[i];
					++band_size;
				}
			}
		}
		
		obj_id[] audience = getBandAudienceInRange(band_members, band_size, PERFORMANCE_XP_RANGE);
		int audience_size = (audience.length - 1);
		
		if (audience_size < 0)
		{
			audience_size = 0;
		}
		
		if (audience_size > (PERFORM_XP_AUDIENCE_MAX_1 + PERFORM_XP_AUDIENCE_MAX_2))
		{
			audience_size = PERFORM_XP_AUDIENCE_MAX_1 + PERFORM_XP_AUDIENCE_MAX_2;
		}
		
		int applause_count = getIntObjVar(actor, performance.VAR_PERFORM_APPLAUSE_COUNT);
		setObjVar(actor, performance.VAR_PERFORM_APPLAUSE_COUNT, 0);
		
		int flourish_count = getIntObjVar(actor, performance.VAR_PERFORM_FLOURISH_COUNT);
		setObjVar(actor, performance.VAR_PERFORM_FLOURISH_COUNT, 0);
		setObjVar(actor, performance.VAR_PERFORM_FLOURISH_TOTAL, 0);
		
		LOG ("entertainer_xp", "Flourish Count = "+ flourish_count);
		
		int flourishDec = (int)((float)flourish_xp / 6.0f);
		
		flourish_xp *= flourish_count;
		
		int old_flourish_xp = getIntObjVar(actor, VAR_PERFORM_FLOURISH_XP);
		
		if (old_flourish_xp > flourish_xp)
		{
			flourish_xp = old_flourish_xp;
			if (flourish_xp > 0)
			{
				flourish_xp -= flourishDec;
			}
			
			if (flourish_xp < 0)
			{
				flourish_xp = 0;
			}
		}
		setObjVar(actor, VAR_PERFORM_FLOURISH_XP, flourish_xp);
		
		int xp_amount = base_xp + flourish_xp;
		LOG ("entertainer_xp", "XP Amount = "+ xp_amount + ", (Base XP: "+ base_xp + " + (Flourish XP: "+ flourish_xp + "))");
		
		float band_mod = ((band_size - 1) / ((float)PERFORM_XP_GROUP_MAX)) * PERFORM_XP_GROUP_MOD;
		float applause_mod = applause_count / 100f;
		LOG ("entertainer_xp", "Band Mod = "+ band_mod);
		LOG ("entertainer_xp", "Applaues Mod = "+ applause_mod);
		
		float audience_mod = (float)audience_size / 50f;
		LOG ("entertainer_xp", "Audience Mod = "+ audience_mod);
		
		float xp_mod = 1f + band_mod + audience_mod + applause_mod;
		
		xp_amount = (int) (xp_amount * xp_mod);
		
		LOG ("entertainer_xp", "XP Mod = "+ xp_mod);
		LOG ("entertainer_xp", "Total XP = "+ xp_amount);
		
		return xp_amount;
	}
	
	
	public static obj_id[] getBandAudienceInRange(obj_id[] band_members, int band_size, float range) throws InterruptedException
	{
		Vector audience = new Vector();
		audience.setSize(0);
		
		for (int i = 0; i < band_size; i++)
		{
			testAbortScript();
			obj_id[] member_audience;
			
			if (isIdValid(band_members[i]))
			{
				if (hasScript(band_members[i], DANCE_HEARTBEAT_SCRIPT) || hasScript(band_members[i], JUGGLE_HEARTBEAT_SCRIPT))
				{
					member_audience = getPerformanceWatchersInRange(band_members[i], range);
				}
				else
				{
					member_audience = getPerformanceListenersInRange(band_members[i], range);
				}
				
				if (member_audience != null && member_audience.length > 0)
				{
					for (int j = 0; j < member_audience.length; j++)
					{
						testAbortScript();
						if (!utils.isElementInArray(audience, member_audience[j]))
						{
							if (isIdValid(member_audience[j]))
							{
								if (!checkDenyService(band_members[i], member_audience[j]))
								{
									utils.addElement(audience, member_audience[j]);
								}
							}
						}
					}
				}
			}
		}
		
		obj_id[] _audience = new obj_id[0];
		if (audience != null)
		{
			_audience = new obj_id[audience.size()];
			audience.toArray(_audience);
		}
		return _audience;
	}
	
	
	public static void denyService(obj_id player, obj_id target) throws InterruptedException
	{
		if (checkDenyService(player, target))
		{
			removeDenyService(player, target);
		}
		else
		{
			addDenyService(player, target);
		}
	}
	
	
	public static void addDenyService(obj_id player, obj_id target) throws InterruptedException
	{
		if (utils.hasScriptVar(target, VAR_PERFORM_PAY_AGREE))
		{
			obj_id watched = getPerformanceWatchTarget(target);
			obj_id listened = getPerformanceListenTarget(target);
			
			if ((isIdValid(watched) && watched == player) || (isIdValid(listened) && listened == player))
			{
				sendSystemMessage(player, SID_DENY_SERVICE_COVER_CHARGE);
				return;
			}
		}
		
		Vector denyServiceList = utils.getResizeableObjIdBatchScriptVar(player, "performance.denyService");
		
		if (denyServiceList == null)
		{
			denyServiceList = new Vector();
		}
		
		utils.addElement(denyServiceList, target);
		
		utils.setBatchScriptVar(player, "performance.denyService", denyServiceList);
		
		performanceMessageToSelf(player, target, SID_DENY_SERVICE_ADD_SELF);
		performanceMessageToPerson(target, player, target, SID_DENY_SERVICE_ADD_OTHER);
	}
	
	
	public static void removeDenyService(obj_id player, obj_id target) throws InterruptedException
	{
		Vector denyServiceList = utils.getResizeableObjIdBatchScriptVar(player, "performance.denyService");
		
		if (denyServiceList == null)
		{
			return;
		}
		
		utils.removeElement(denyServiceList, target);
		
		if (denyServiceList.size() == 0)
		{
			utils.removeBatchScriptVar(player, "performance.denyService");
		}
		else
		{
			utils.setBatchScriptVar(player, "performance.denyService", denyServiceList);
		}
		
		performanceMessageToSelf(player, target, SID_DENY_SERVICE_REMOVE_SELF);
		performanceMessageToPerson(target, player, target, SID_DENY_SERVICE_REMOVE_OTHER);
	}
	
	
	public static boolean checkDenyService(obj_id player, obj_id target) throws InterruptedException
	{
		Vector denyServiceList = utils.getResizeableObjIdBatchScriptVar(player, "performance.denyService");
		
		if (denyServiceList == null)
		{
			return false;
		}
		
		return utils.isElementInArray(denyServiceList, target);
	}
	
	
	public static boolean displayAvailableMusic(obj_id actor, boolean band, boolean changeMusic) throws InterruptedException
	{
		if (!isIdValid(actor))
		{
			return false;
		}
		
		if (!notShapechanged(actor))
		{
			return false;
		}
		
		int instrument_equipped = getInstrumentAudioId(actor);
		if (instrument_equipped == 0)
		{
			performanceMessageToSelf(actor, null, SID_MUSIC_NO_INSTRUMENT);
			return true;
		}
		
		Vector available_music = new Vector();
		available_music.setSize(0);
		int num_items = dataTableGetNumRows(DATATABLE_PERFORMANCE);
		for (int i = 0; i < num_items; i++)
		{
			testAbortScript();
			dictionary row = dataTableGetRow(DATATABLE_PERFORMANCE, i);
			int type = row.getInt("type");
			if (type == (866729052))
			{
				
				int instrument_id = row.getInt("instrumentAudioId");
				if (instrument_equipped != instrument_id)
				{
					continue;
				}
				
				String name = row.getString("performanceName");
				
				if (canPerformSong(actor, i + 1))
				{
					name = name.substring(0,1).toUpperCase() + name.substring(1).toLowerCase();
					available_music = utils.addElement(available_music, name);
				}
			}
		}
		
		if (available_music.size() > 0)
		{
			sui.listbox(actor, actor, "@performance:select_song", sui.OK_CANCEL, "@performance:available_songs", available_music, "msgSongSelected");
			
			if (!hasScript(actor, PERFORMANCE_SELECT))
			{
				attachScript(actor, PERFORMANCE_SELECT);
			}
			
			setObjVar(actor, VAR_AVAILABLE_PERFORMANCES, available_music, resizeableArrayTypeString);
			setObjVar(actor, VAR_SELECT_MUSIC_BAND, band);
			setObjVar(actor, VAR_SELECT_MUSIC_CHANGE, changeMusic);
			
			return true;
		}
		return false;
	}
	
	
	public static boolean displayAvailableDances(obj_id actor, boolean changeDance) throws InterruptedException
	{
		if (!isIdValid(actor))
		{
			return false;
		}
		
		if (hasObjVar(actor, VAR_AVAILABLE_PERFORMANCES))
		{
			return false;
		}
		
		if (!notShapechanged(actor))
		{
			return false;
		}
		
		Vector available_dances = new Vector();
		available_dances.setSize(0);
		int num_items = dataTableGetNumRows(DATATABLE_PERFORMANCE);
		for (int i = 0; i < num_items; i++)
		{
			testAbortScript();
			dictionary row = dataTableGetRow(DATATABLE_PERFORMANCE, i);
			int type = row.getInt("type");
			if (type == (-1788534963))
			{
				
				String name = row.getString("performanceName");
				if (canPerformDance(actor, i + 1))
				{
					name = name.substring(0,1).toUpperCase() + name.substring(1).toLowerCase();
					available_dances = utils.addElement(available_dances, name);
				}
			}
		}
		
		if (available_dances.size() > 0)
		{
			sui.listbox(actor, actor, "@performance:select_dance", sui.OK_CANCEL, "@performance:available_dances", available_dances, "msgDanceSelected");
			
			if (!hasScript(actor, PERFORMANCE_SELECT))
			{
				attachScript(actor, PERFORMANCE_SELECT);
			}
			
			setObjVar(actor, VAR_AVAILABLE_PERFORMANCES, available_dances, resizeableArrayTypeString);
			if (changeDance)
			{
				setObjVar(actor, VAR_SELECT_DANCE_CHANGE, 1);
			}
			
			return true;
		}
		return false;
	}
	
	
	public static boolean displayAvailableJuggles(obj_id actor, boolean changeJuggle) throws InterruptedException
	{
		if (!isIdValid(actor))
		{
			return false;
		}
		
		if (hasObjVar(actor, VAR_AVAILABLE_PERFORMANCES))
		{
			return false;
		}
		
		if (!notShapechanged(actor))
		{
			return false;
		}
		
		Vector available_juggles = new Vector();
		available_juggles.setSize(0);
		int num_items = dataTableGetNumRows(DATATABLE_PERFORMANCE);
		for (int i = 0; i < num_items; i++)
		{
			testAbortScript();
			dictionary row = dataTableGetRow(DATATABLE_PERFORMANCE, i);
			int type = row.getInt("type");
			if (type == (124914482))
			{
				
				String name = row.getString("performanceName");
				if (canPerformJuggle(actor, i + 1))
				{
					name = name.substring(0,1).toUpperCase() + name.substring(1).toLowerCase();
					available_juggles = utils.addElement(available_juggles, name);
				}
			}
		}
		
		if (available_juggles.size() > 0)
		{
			sui.listbox(actor, actor, "@performance:select_juggs", sui.OK_CANCEL, "@performance:available_juggs", available_juggles, "msgJuggleSelected");
			
			if (!hasScript(actor, PERFORMANCE_SELECT))
			{
				attachScript(actor, PERFORMANCE_SELECT);
			}
			
			setObjVar(actor, VAR_AVAILABLE_PERFORMANCES, available_juggles, resizeableArrayTypeString);
			if (changeJuggle)
			{
				setObjVar(actor, VAR_SELECT_JUGGLE_CHANGE, 1);
			}
			
			return true;
		}
		return false;
	}
	
	
	public static int canPerformanceHeal(obj_id actor) throws InterruptedException
	{
		if (pet_lib.isDroidPet(actor) || isHologramPerformer(actor))
		{
			return 0;
		}
		
		if (!utils.isProfession(actor, utils.ENTERTAINER))
		{
			return 0;
		}
		
		location loc = getLocation(actor);
		if (isIdValid(loc.cell))
		{
			obj_id building = getTopMostContainer(loc.cell);
			if (!hasObjVar(building, "healing.canhealshock") && !hasObjVar(loc.cell, "healing.canhealshock"))
			{
				return 0;
			}
			return 2;
		}
		
		if (camping.isInEntertainmentCamp(actor))
		{
			return 2;
		}
		
		region[] regs = getRegionsWithGeographicalAtPoint(getLocation(actor), regions.GEO_THEATER);
		if (regs != null && regs.length > 0)
		{
			return 2;
		}
		
		int improv = (int)getSkillStatisticModifier(actor, "expertise_en_improv");
		if (improv == 1)
		{
			return 3;
		}
		
		return 0;
	}
	
	
	public static boolean isInRightBuilding(obj_id objPlayer, obj_id objBuilding) throws InterruptedException
	{
		location locTest = getLocation(objPlayer);
		if (isIdValid(locTest.cell))
		{
			obj_id objOwner = getTopMostContainer(locTest.cell);
			if (objBuilding == objOwner)
			{
				return true;
			}
		}
		return false;
	}
	
	
	public static void checkAndIncrementEntertainerMissions(obj_id objPlayer, String strType) throws InterruptedException
	{
		if (hasObjVar (objPlayer, "newbie_handoff.mission.gig"))
		{
			incrementNewbieHandoffEntertainerMissions(objPlayer, strType);
		}
		
		LOG("missions", "checking for entertainer missions");
		obj_id[] objMissions = getMissionObjects(objPlayer);
		if (objMissions == null)
		{
			LOG("missions", "no missions found");
			return;
		}
		int intI = 0;
		while (intI < objMissions.length)
		{
			testAbortScript();
			obj_id objMissionData = objMissions[intI];
			String strMissionType = getMissionType(objMissionData);
			LOG("missions", "strMissionType is "+strMissionType);
			if (strMissionType.equals(strType))
			{
				obj_id objBuilding = getObjIdObjVar(objMissionData, "objBuilding");
				LOG("mission", "building is "+objBuilding);
				if (isInRightBuilding(objPlayer, objBuilding))
				{
					
					int intDuration = getIntObjVar(objMissionData, "intTime");
					int intGameTime = getGameTime();
					int intStartTime = intGameTime;
					if (hasObjVar(objMissions[intI], "intStartTime"))
					{
						intStartTime = getIntObjVar(objMissions[intI], "intStartTime");
					}
					else
					{
						setObjVar(objMissions[intI], "intStartTime", intStartTime);
						return;
					}
					
					int intElapsedTime = intGameTime - intStartTime;
					LOG("missions", "intElapsedTime is "+intElapsedTime);
					
					if (intElapsedTime >= intDuration)
					{
						messageTo(objMissions[intI], "entertainerSuccess", null, 0, true);
					}
				}
			}
			
			intI = intI+1;
		}
	}
	
	
	public static void stopEntertainerMissionTracking(obj_id objPlayer, String strType) throws InterruptedException
	{
		if (hasObjVar (objPlayer, "newbie_handoff.mission.gig"))
		{
			stopNewbieHandoffEntertainerMissionTracking(objPlayer, strType);
		}
		
		obj_id[] objMissions = getMissionObjects(objPlayer);
		
		if (objMissions == null)
		{
			return;
		}
		
		int intI = 0;
		
		while (intI < objMissions.length)
		{
			testAbortScript();
			obj_id objMissionData = objMissions[intI];
			
			if (!isIdValid(objMissionData))
			{
				LOG("DESIGNER_FATAL", "Mission object with null mission data, id is "+objMissions[intI]+" player is "+objPlayer);
				return;
			}
			
			String strMissionType = getMissionType(objMissionData);
			
			if (strMissionType == null)
			{
				LOG("DESIGNER_FATAL", "strMissionType is null for "+objMissionData+" and player is "+objPlayer+" and objMission is "+objMissions[intI]);
				return;
			}
			
			if (strMissionType.equals(strType))
			{
				if (hasObjVar(objMissions[intI], "intStartTime"))
				{
					removeObjVar(objMissions[intI], "intStartTime");
				}
			}
			
			intI++;
		}
	}
	
	
	public static void incrementNewbieHandoffEntertainerMissions(obj_id player, String type) throws InterruptedException
	{
		String missionType = getStringObjVar( player, "newbie_handoff.mission.gig.type");
		LOG("newbie_gig_mission", "Current mission type: "+missionType+", current performance type: "+type);
		
		if (type.equals(missionType))
		{
			obj_id building = getObjIdObjVar( player, "newbie_handoff.mission.gig.obj");
			LOG("newbie_gig_mission", "Checking to see if in correct building. - "+building);
			
			if (isInRightBuilding(player, building))
			{
				int duration = getIntObjVar( player, "newbie_handoff.mission.gig.duration");
				int curTime = getGameTime();
				int start = curTime;
				
				if (hasObjVar(player, "newbie_handoff.mission.gig.start"))
				{
					start = getIntObjVar( player, "newbie_handoff.mission.gig.start");
					LOG("newbie_gig_mission", "Getting mission start time "+start);
				}
				else
				{
					setObjVar( player, "newbie_handoff.mission.gig.start", start);
					LOG("newbie_gig_mission", "Initializing mission start time "+start);
					return;
				}
				
				int elapsedTime = curTime - start;
				
				if (elapsedTime >= duration)
				{
					LOG("newbie_gig_mission", "Mission Complete");
					messageTo( player, "missionGigComplete", null, 0, true );
				}
			}
		}
	}
	
	
	public static void stopNewbieHandoffEntertainerMissionTracking(obj_id player, String type) throws InterruptedException
	{
		if (hasObjVar(player, "newbie_handoff.mission.gig.start"))
		{
			LOG("newbie_gig_mission", "Removing time tracking objvar");
			removeObjVar( player, "newbie_handoff.mission.gig.start");
		}
	}
	
	
	public static void startJuggling(obj_id actor, String juggleName) throws InterruptedException
	{
		
		if (juggleName == null || juggleName.length() < 1)
		{
			displayAvailableJuggles(actor, false);
			return;
		}
		
		if (getPerformanceType(actor) != 0)
		{
			performanceMessageToSelf(actor, null, SID_ALREADY_PERFORMING_SELF);
			return;
		}
		
		int performanceIndex = lookupPerformanceIndex((124914482), juggleName, 0);
		if (performanceIndex == 0)
		{
			performanceMessageToSelf(actor, null, SID_JUGGLE_UNKNOWN_SELF);
			return;
		}
		
		if (!notShapechanged(actor))
		{
			return;
		}
		
		if (!canPerformJuggle(actor, performanceIndex))
		{
			performanceMessageToSelf(actor, null, SID_JUGGLE_LACK_SKILL_SELF);
			return;
		}
		
		performanceMessageToSelf(actor, null, SID_JUGGLE_START_SELF);
		performanceMessageToWatchers(actor, null, SID_JUGGLE_START_OTHER);
		
		dictionary params;
		int startTime = getGameTime();
		
		setPerformanceType(actor, performanceIndex);
		setPerformanceStartTime(actor, startTime);
		
		setPerformanceWatchTarget(actor, actor);
		
		attachScript(actor, JUGGLE_HEARTBEAT_SCRIPT);
		
		sendNpcsEntertainmentMessage(actor, NPC_ENTERTAINMENT_MESSAGE_START, PERFORMANCE_TYPE_JUGGLE);
		
		params = new dictionary();
		params.put("performer", actor);
		broadcastMessage("handlePerformerStartPerforming", params);
		
		params = new dictionary();
		params.put("performanceIndex", performanceIndex);
		params.put("sequence", 0);
		messageTo(actor, "OnPulse", params, PERFORMANCE_HEARTBEAT_TIME, false);
	}
	
	
	public static void stopJuggling(obj_id actor) throws InterruptedException
	{
		if (hasScript(actor, JUGGLE_HEARTBEAT_SCRIPT))
		{
			
			dictionary params = new dictionary();
			params.put("performer", actor);
			params.put("check", "musician");
			broadcastMessage("handlePerformerStopPerforming", params);
			
			messageTo(actor, "OnPerformanceEnd", null, 0.0f, false);
			setPerformanceType(actor, 0);
			setPerformanceStartTime(actor, 0);
			
			performanceMessageToSelf(actor, null, SID_JUGGLE_STOP_SELF);
			performanceMessageToWatchers(actor, null, SID_JUGGLE_STOP_OTHER);
		}
		else
		{
			if (hasObjVar(actor, performance.VAR_PERFORM))
			{
				removeObjVar(actor, performance.VAR_PERFORM);
			}
			performanceMessageToSelf(actor, null, SID_JUGGLE_NOT_PERFORMING);
		}
	}
	
	
	public static void changeJuggle(obj_id actor, String juggleName) throws InterruptedException
	{
		
		if (juggleName == null || juggleName.length() < 1)
		{
			displayAvailableJuggles(actor, true);
			return;
		}
		
		if (!hasScript(actor, JUGGLE_HEARTBEAT_SCRIPT))
		{
			performanceMessageToSelf(actor, null, SID_JUGGLE_MUST_BE_PERFORMING_SELF);
			return;
		}
		
		int performanceIndex = lookupPerformanceIndex((124914482), juggleName, 0);
		if (performanceIndex == 0)
		{
			performanceMessageToSelf(actor, null, SID_JUGGLE_UNKNOWN_SELF);
			return;
		}
		
		if (!notShapechanged(actor))
		{
			return;
		}
		
		if (!canPerformJuggle(actor, performanceIndex))
		{
			performanceMessageToSelf(actor, null, SID_JUGGLE_LACK_SKILL_SELF);
			return;
		}
		
		setPerformanceType(actor, performanceIndex);
		queueCommand(actor, (-1219480501), null, "1", COMMAND_PRIORITY_IMMEDIATE);
	}
	
	
	public static void openInspireMenu(obj_id self, obj_id target) throws InterruptedException
	{
		if (utils.hasScriptVar(self, VAR_BUFF_SUI))
		{
			int pidClose = utils.getIntScriptVar(self, VAR_BUFF_SUI);
			forceCloseSUIPage(pidClose);
		}
		
		if (!utils.isProfession(self, utils.ENTERTAINER))
		{
			return;
		}
		
		Vector dsrc = new Vector();
		Vector list = new Vector();
		for (int i = 0; i < INSPIRATION_BUFF_ENTERTAINER.length; i++)
		{
			testAbortScript();
			dsrc.addElement("@performance:"+ INSPIRATION_BUFF_ENTERTAINER[i]);
			list.addElement(INSPIRATION_BUFF_ENTERTAINER[i]);
		}
		
		String type = "";
		
		if (hasScript(self, DANCE_HEARTBEAT_SCRIPT))
		{
			obj_id watched = getPerformanceWatchTarget(target);
			if (watched != self)
			{
				sendSystemMessage(self, new string_id("performance", "insp_buff_must_watch"));
				return;
			}
			
			for (int i = 0; i < INSPIRATION_BUFF_DANCER.length; i++)
			{
				testAbortScript();
				dsrc.addElement("@performance:"+ INSPIRATION_BUFF_DANCER[i]);
				list.addElement(INSPIRATION_BUFF_DANCER[i]);
			}
		}
		else if (hasScript(self, MUSIC_HEARTBEAT_SCRIPT))
		{
			obj_id listened = getPerformanceListenTarget(target);
			if (listened != self)
			{
				sendSystemMessage(self, new string_id("performance", "insp_buff_must_listen"));
				return;
			}
			
			for (int i = 0; i < INSPIRATION_BUFF_MUSICIAN.length; i++)
			{
				testAbortScript();
				dsrc.addElement("@performance:"+ INSPIRATION_BUFF_MUSICIAN[i]);
				list.addElement(INSPIRATION_BUFF_MUSICIAN[i]);
			}
		}
		else
		{
			sendSystemMessage(self, new string_id("performance", "insp_buff_must_perform"));
			return;
		}
		
		String prompt = "@performance:inspire_menu_prompt "+ getName(target);
		prompt += "\n\n";
		prompt += "@performance:inspire_menu_prompt2";
		String title = "@performance:inspire_menu_title";
		
		int pid = sui.listbox(self, self, prompt, sui.OK_CANCEL, title, dsrc, "handleInspireMenu");
		
		if (pid >= 0)
		{
			utils.setScriptVar(self, VAR_BUFF_LIST, utils.toStaticStringArray(list));
			utils.setScriptVar(self, VAR_BUFF_TARGET, target);
			utils.setScriptVar(self, VAR_BUFF_SUI, pid);
		}
	}
	
	
	public static boolean hasMaxHolo(obj_id actor) throws InterruptedException
	{
		if (!isIdValid(actor))
		{
			return true;
		}
		
		if (utils.hasScriptVar(actor, "currentHolo"))
		{
			int holoPets = utils.getIntScriptVar(actor, "currentHolo");
			int holoMod = (int)getSkillStatisticModifier(actor, "expertise_en_holographic_additional_backup");
			int maxHoloAllowed = 1;
			maxHoloAllowed += holoMod;
			if (holoPets >= maxHoloAllowed)
			{
				return true;
			}
		}
		
		return false;
	}
	
	
	public static void holographicCleanup(obj_id actor) throws InterruptedException
	{
		obj_id holoDelete = null;
		if (utils.hasScriptVar(actor, "holoId1"))
		{
			holoDelete = utils.getObjIdScriptVar(actor, "holoId1");
			if (exists(holoDelete))
			{
				destroyObject(holoDelete);
			}
		}
		
		if (utils.hasScriptVar(actor, "holoId2"))
		{
			holoDelete = utils.getObjIdScriptVar(actor, "holoId2");
			if (exists(holoDelete))
			{
				destroyObject(holoDelete);
			}
		}
		
		if (utils.hasScriptVar(actor, "holoId3"))
		{
			holoDelete = utils.getObjIdScriptVar(actor, "holoId3");
			if (exists(holoDelete))
			{
				destroyObject(holoDelete);
			}
		}
		
		utils.removeScriptVar(actor, "currentHolo");
		utils.removeScriptVar(actor, "holoMessageTime");
		utils.removeScriptVar(actor, "holoId1");
		utils.removeScriptVar(actor, "holoId2");
		utils.removeScriptVar(actor, "holoId3");
		sendSystemMessage(actor, new string_id("spam", "holo_despawned"));
		return;
	}
	
	
	public static boolean createHolographicBackup(obj_id actor) throws InterruptedException
	{
		if (!isIdValid(actor))
		{
			return false;
		}
		
		if (group.isGrouped(actor))
		{
			obj_id groupId = getGroupObject(actor);
			obj_id[] groupMembers = getGroupMemberIds( groupId );
			if (groupMembers.length == 8)
			{
				sendSystemMessage(actor, new string_id("spam", "holo_group_full"));
				return false;
			}
			
			obj_id groupLeaderId = getGroupLeaderId( groupId );
			if (groupLeaderId != actor)
			{
				sendSystemMessage(actor, new string_id("spam", "holo_not_leader"));
				return false;
			}
		}
		
		location myLoc = getLocation(actor);
		
		int species = getSpecies(actor);
		int gender = getGender(actor);
		
		String template = "object/mobile/hologram/";
		
		String speciesString = "human";
		String genderString = "male";
		
		switch(species)
		{
			case SPECIES_RODIAN: speciesString = "rodian";
			break;
			case SPECIES_TRANDOSHAN: speciesString = "trandoshan";
			break;
			case SPECIES_MON_CALAMARI: speciesString = "moncal";
			break;
			case SPECIES_WOOKIEE: speciesString = "wookiee";
			break;
			case SPECIES_BOTHAN: speciesString = "bothan";
			break;
			case SPECIES_TWILEK: speciesString = "twilek";
			break;
			case SPECIES_ZABRAK: speciesString = "zabrak";
			break;
			case SPECIES_ITHORIAN: speciesString = "ithorian";
			break;
			case SPECIES_SULLUSTAN: speciesString = "sullustan";
			break;
		}
		
		if (gender == GENDER_FEMALE)
		{
			genderString = "female";
		}
		
		template += speciesString + "_"+ genderString + ".iff";
		
		obj_id hologram = createObject(template, myLoc);
		
		if (isIdValid(hologram))
		{
			int holoPets = 1;
			if (utils.hasScriptVar(actor, "currentHolo"))
			{
				holoPets = utils.getIntScriptVar(actor, "currentHolo");
				holoPets = holoPets +1;
				utils.setScriptVar(actor, "currentHolo", holoPets);
			}
			else
			{
				utils.setScriptVar(actor, "currentHolo", holoPets);
			}
			
			int intTime = getGameTime();
			utils.setScriptVar(actor, "holoMessageTime", intTime);
			
			utils.setScriptVar(actor, "holoId"+holoPets, hologram);
			
			dictionary msgTimer = new dictionary();
			msgTimer.put("holoMessageTime", intTime);
			messageTo(actor, "handleHoloTimeout", msgTimer, 3600, false);
			setMaster(hologram, actor);
			setScale(hologram, getScale(actor));
			setYaw(hologram, getYaw(actor));
			setHologramType(hologram, HOLOGRAM_TYPE1_QUALITY4);
			setName(hologram, "Hologram");
			setInvulnerable(hologram, true);
			
			custom_var[] var_list = getAllCustomVars(actor);
			
			if (var_list != null && var_list.length > 0)
			{
				for (int i = 0; i < var_list.length; i++)
				{
					testAbortScript();
					ranged_int_custom_var ricv = (ranged_int_custom_var)var_list[i];
					
					String var = ricv.getVarName();
					int value = ricv.getValue();
					
					if (value != 0)
					{
						setRangedIntCustomVarValue(hologram, var, value);
					}
					
				}
			}
			
			setPalcolorCustomVarClosestColor(hologram,"/shared_owner/index_color_skin", 255,255,255,255);
			
			obj_id[] contents = getAllWornItems(actor, false);
			
			utils.setObjVar(hologram, "hologram_performer", 1);
			if (contents != null && contents.length > 0)
			{
				for (int i = 0; i < contents.length; i++)
				{
					testAbortScript();
					if (isIdValid(contents[i]))
					{
						if (getContainerType(contents[i]) == 0)
						{
							
							if (getGameObjectType(contents[i]) == GOT_misc_appearance_only_invisible)
							{
								continue;
							}
							
							if (!getTemplateName(contents[i]).endsWith("player.iff"))
							{
								createObject(getTemplateName(contents[i]), hologram, "");
							}
						}
					}
				}
			}
			
			setMaster(hologram, actor);
			
			attachScript(hologram, "systems.skills.performance.holographic_backup");
			utils.setScriptVar(hologram, "hologram_performer", 1);
			
			queueCommand(actor, (-2007999144), hologram, "", COMMAND_PRIORITY_DEFAULT);
			messageTo(hologram, "handleGroupInvitation", null, 2f, false);
			
			return true;
		}
		
		return false;
	}
}
