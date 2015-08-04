package script.library;

import script.*;
import script.base_class.*;
import script.combat_engine.*;
import java.util.Arrays;
import java.util.Hashtable;
import java.util.Vector;
import script.base_script;


import script.library.gcw;
import script.library.groundquests;
import script.library.utils;


public class holiday extends script.base_script
{
	public holiday()
	{
	}
	public static final String PLANET_VAR_EVENT_PREFIX = "event";
	public static final String PLANET_VAR_SCORE = ".score";
	public static final String PLANET_VAR_EMPIRE_DAY = ".empire_day";
	public static final String PLANET_VAR_SCORE_TIMESTAMP = PLANET_VAR_SCORE + ".timestamp";
	public static final String PLANET_VAR_PLAYER_NAME = ".player_name";
	public static final String PLANET_VAR_PLAYER_SCORE = ".player_score";
	public static final String PLANET_VAR_PLAYER_FACTION_IMP = ".imperial";
	public static final String PLANET_VAR_PLAYER_FACTION_REB = ".rebel";
	
	public static final String EMPIRE_DAY_RECEIVED_VI = "gift.empire08";
	public static final String EMPIRE_DAY_RECEIVED_VII = "gift.empire09";
	public static final String EMPIRE_DAY_CHAMPION_BADGE = "empire_day_champion_badge_2009";
	public static final String REMEMBRANCE_DAY_CHAMPION_BADGE = "remembrance_day_champion_badge_2009";
	public static final String SPAWNER_PREFIX_OBJVAR = "spawned.";
	
	public static final String LEADER_BOARD_TITLE = "Leader Boards Empire & Remembrance Day";
	public static final String LEADERSUI_PID_NAME = "leaderBoardPid";
	
	public static final String PLAYER_EMPIRE_DAY_PREFIX = PLANET_VAR_EVENT_PREFIX + PLANET_VAR_EMPIRE_DAY;
	public static final String PLAYER_EMPIRE_DAY_SCORE = PLAYER_EMPIRE_DAY_PREFIX + PLANET_VAR_SCORE;
	public static final String PLAYER_EMPIRE_DAY_NEUTRAL = PLAYER_EMPIRE_DAY_PREFIX + ".neutral";
	
	public static final String IMPERIAL_AND_REBEL_EVENT_GIFT = "item_empire_day_6yr_painting";
	public static final String IMPERIAL_EMPIRE_DAY_GIFT = "item_empire_day_6yr_greeter_tie_pilot";
	public static final String REBEL_REMEMBRANCE_DAY_GIFT = "item_empire_day_6yr_greeter_xwing_pilot";
	
	public static final String EMOTE_DATATABLE = "datatables/npc/emperors_day/badge_emotes.iff";
	
	public static final String LIFEDAY = "event/life_day";
	public static final String EMP_DAY = "event/emperors_day";
	public static final String EMPIRE_DAY = "event/empire_day";
	
	public static final string_id GIFT_GRANTED = new string_id(EMP_DAY, "gift_granted_empire");
	
	public static final string_id SID_REM_EMP_DAY_BADGE = new string_id(EMP_DAY, "empire_day_badge_removed");
	public static final string_id SID_REM_REM_DAY_BADGE = new string_id(EMP_DAY, "remembrance_day_badge_removed");
	public static final string_id SID_REMOVED_REWARD_FLAG = new string_id(EMP_DAY, "reward_flag_removed");
	public static final string_id SID_MOUNTED = new string_id(LIFEDAY, "mounted");
	public static final string_id SID_REACHED_LIMIT = new string_id(LIFEDAY, "reached_limit");
	public static final string_id SID_COMPLETED_QUOTA = new string_id(EMPIRE_DAY, "completed_collection");
	public static final string_id SID_IMPERIAL_COMPETITIVE_START = new string_id(EMPIRE_DAY, "imperial_competitive_start");
	public static final string_id SID_REBEL_COMPETITIVE_START = new string_id(EMPIRE_DAY, "rebel_competitive_start");
	public static final string_id SID_LOCKED_OUT = new string_id(LIFEDAY, "locked_out");
	public static final string_id SID_IMPERIAL_START = new string_id(EMPIRE_DAY, "imperial_start");
	public static final string_id SID_REBEL_START = new string_id(EMPIRE_DAY, "rebel_start");
	public static final string_id SID_GCW_POINTS = new string_id(EMPIRE_DAY, "you_received_gcw_points");
	public static final string_id SID_CONGRATS_QUOTA_IMPERIAL = new string_id(EMPIRE_DAY, "congratz_quota_accomplished_imp");
	public static final string_id SID_OOPS_QUOTA_FAILED_IMPERIAL = new string_id(EMPIRE_DAY, "oops_quota_failed_imp");
	public static final string_id SID_CONGRATS_QUOTA_REBEL = new string_id(EMPIRE_DAY, "congratz_quota_accomplished_reb");
	public static final string_id SID_OOPS_QUOTA_FAILED_REBEL = new string_id(EMPIRE_DAY, "oops_quota_failed_reb");
	public static final string_id SID_NOT_SF = new string_id(EMPIRE_DAY, "no_longer_sf_redeclare");
	public static final string_id SID_NOT_WHILE_MOUNTED = new string_id(EMPIRE_DAY, "not_while_monted");
	public static final string_id SID_TOKEN_REWARDED = new string_id(EMPIRE_DAY, "token_rewarded");
	public static final string_id SID_TOKENS_REWARDED = new string_id(EMPIRE_DAY, "tokens_rewarded");
	
	public static final String EVENT_TRACKER_SCRIPT = "systems.spawning.spawned_tracker";
	public static final String SMOKE_CLIENT_EFFECT = "appearance/pt_smoke_puff.prt";
	
	public static final String REBEL_RESISTANCE_COLLECTION = "remembrance_day_resistance_event";
	public static final String REBEL_VANDAL_COLLECTION = "remembrance_day_vandal_event";
	public static final String REBEL_ENGINEERING_COLLECTION = "remembrance_day_engineering_quest";
	public static final String REBEL_ENTERTAINER_COLLECTION = "remembrance_day_performance_quest";
	public static final String REBEL_RESCUE_COLLECTION = "remembrance_day_rescue_quest";
	
	public static final String IMPERIAL_RECRUITING_COLLECTION = "empire_day_recruiting_event";
	public static final String IMPERIAL_ANTIPROP_COLLECTION = "empire_day_anti_propaganda_event";
	public static final String IMPERIAL_ENGINEERING_COLLECTION = "empire_day_engineering_quest";
	public static final String IMPERIAL_ENTERTAINER_COLLECTION = "empire_day_performance_quest";
	public static final String IMPERIAL_RESCUE_COLLECTION = "empire_day_rescue_quest";
	
	public static final String REBEL_RESISTANCE_START_SLOT = "remembrance_day_resistance_starter";
	public static final String REBEL_VANDAL_START_SLOT = "remembrance_day_vandal_starter";
	public static final String REBEL_ENGINEERING_START_SLOT = "remembrance_day_engineering_starter";
	public static final String REBEL_ENTERTAINER_START_SLOT = "remembrance_day_performance_starter";
	public static final String REBEL_RESCUE_START_SLOT = "remembrance_day_rescue_starter";
	
	public static final String IMPERIAL_RECRUITING_START_SLOT = "empire_day_recruitment_starter";
	public static final String IMPERIAL_ANTIPROP_START_SLOT = "empire_day_anti_propaganda_starter";
	public static final String IMPERIAL_ENGINEERING_START_SLOT = "empire_day_engineering_starter";
	public static final String IMPERIAL_ENTERTAINER_START_SLOT = "empire_day_performance_starter";
	public static final String IMPERIAL_RESCUE_START_SLOT = "empire_day_rescue_starter";
	
	public static final String REBEL_RESISTANCE_COUNTER_SLOT = "remembrance_day_resistance_counter";
	public static final String REBEL_VANDAL_COUNTER_SLOT = "remembrance_day_vandal_counter";
	public static final String REBEL_ENGINEERING_COUNTER_SLOT = "remembrance_day_engineering_counter";
	public static final String REBEL_ENTERTAINER_COUNTER_SLOT = "remembrance_day_performance_counter";
	public static final String REBEL_RESCUE_COUNTER_SLOT = "remembrance_day_rescue_counter";
	
	public static final String IMPERIAL_RECRUITING_COUNTER_SLOT = "empire_day_recruitment_counter";
	public static final String IMPERIAL_ANTIPROP_COUNTER_SLOT = "empire_day_anti_propaganda_counter";
	public static final String IMPERIAL_ENGINEERING_COUNTER_SLOT = "empire_day_engineering_counter";
	public static final String IMPERIAL_ENTERTAINER_COUNTER_SLOT = "empire_day_performance_counter";
	public static final String IMPERIAL_RESCUE_COUNTER_SLOT = "empire_day_rescue_counter";
	
	public static final String IMPERIAL_EMPTY_KIOSK = "object/tangible/event_perk/empire_day_missing_sign.iff";
	public static final String REBEL_EMPTY_KIOSK = "object/tangible/event_perk/remembrance_day_missing_sign.iff";
	
	public static final String IMPERIAL_NONPLAYER_RESISTANCE_KIOSK = "object/tangible/event_perk/remembrance_day_resistance_sign.iff";
	public static final String IMPERIAL_PLAYER_RECRUITMENT_KIOSK = "object/tangible/event_perk/empire_day_player_placed_recruitment_sign.iff";
	public static final String IMPERIAL_PLAYER_DEFACED_KIOSK = "object/tangible/event_perk/remembrance_day_player_defaced_sign.iff";
	public static final String REBEL_NONPLAYER_RECRUITMENT_KIOSK = "object/tangible/event_perk/empire_day_recruitment_sign.iff";
	public static final String REBEL_PLAYER_RESISTANCE_KIOSK = "object/tangible/event_perk/remembrance_day_player_resistance_sign.iff";
	public static final String REBEL_PLAYER_DEFACED_KIOSK = "object/tangible/event_perk/empire_day_player_defaced_recruitment_sign.iff";
	
	public static final String BUFF_IMP_EMPIREDAY_RECRUITMENT_COMBATANT = "event_empireday_imperial_recruitment_combatant";
	public static final String BUFF_REB_EMPIREDAY_RESISTANCE_COMBATANT = "event_empireday_rebel_resistance_combatant";
	public static final String BUFF_IMP_EMPIREDAY_RECRUITMENT_SF = "event_empireday_imperial_recruitment_special_forces";
	public static final String BUFF_REB_EMPIREDAY_RESISTANCE_SF = "event_empireday_rebel_resistance_special_forces";
	public static final String BUFF_IMP_EMPIREDAY_ANTIPROP_COMBATANT = "event_empireday_imperial_antipropaganda_combatant";
	public static final String BUFF_REB_EMPIREDAY_VANDAL_COMBATANT = "event_empireday_rebel_vandal_combatant";
	public static final String BUFF_IMP_EMPIREDAY_ANTIPROP_SF = "event_empireday_imperial_antipropaganda_special_forces";
	public static final String BUFF_REB_EMPIREDAY_VANDAL_SF = "event_empireday_rebel_vandal_special_forces";
	
	public static final String BUFF_IMPERIAL_RECRUITMENT_COUNTER = "event_empireday_imperial_recruitment_counter";
	public static final String BUFF_REBEL_RESISTANCE_COUNTER = "event_empireday_rebel_resistance_counter";
	public static final String BUFF_IMPERIAL_ANTIPROPAGANDA_COUNTER = "event_empireday_imperial_propaganda_counter";
	public static final String BUFF_REBEL_PLAYER_VANDAL_COUNTER = "event_empireday_rebel_vandal_counter";
	
	public static final String EMPIRE_DAY_BUFF_TRACKER = "event_empireday_first_buff_ended";
	
	public static final String BUFF_COMBATANT_GROUP = "combatant";
	public static final String BUFF_SF_GROUP = "special_forces";
	public static final String BUFF_COUNTER_GROUP = "counter";
	public static final String TOKEN_IMPERIAL_EMPIRE_DAY = "item_empire_day_imperial_token";
	public static final String TOKEN_REBEL_EMPIRE_DAY = "item_empire_day_rebel_token";
	
	public static final String EVENT_PREFIX = "empire_day_event.";
	public static final String EMPIRE_DAY_RECRUITMENT_LOCKED_OUT = EVENT_PREFIX + "recruitment_locked_out";
	public static final String EMPIRE_DAY_RESISTANCE_LOCKED_OUT = EVENT_PREFIX + "resistance_locked_out";
	public static final String EMPIRE_DAY_PROPAGANDA_LOCKED_OUT = EVENT_PREFIX + "propaganda_locked_out";
	public static final String EMPIRE_DAY_VANDAL_LOCKED_OUT = EVENT_PREFIX + "vandal_locked_out";
	public static final String EMPIRE_DAY_RECRUITMENT_TIMESTAMP = EVENT_PREFIX + "event_recruitment_time_stamp";
	public static final String EMPIRE_DAY_PROPAGANDA_TIMESTAMP = EVENT_PREFIX + "event_propaganda_time_stamp";
	public static final String EMPIRE_DAY_RESISTANCE_TIMESTAMP = EVENT_PREFIX + "event_resistance_time_stamp";
	public static final String EMPIRE_DAY_VANDAL_TIMESTAMP = EVENT_PREFIX + "event_vandal_time_stamp";
	
	public static final String EMPIREDAYQUEST_IMP_CRASHSITE_01_DOMESTICS = "crash_site_01_imp_domestics";
	public static final String EMPIREDAYQUEST_IMP_CRASHSITE_02_DOMESTICS = "crash_site_02_imp_domestics";
	public static final String EMPIREDAYQUEST_IMP_CRASHSITE_03_DOMESTICS = "crash_site_03_imp_domestics";
	public static final String EMPIREDAYQUEST_IMP_CRASHSITE_04_DOMESTICS = "crash_site_04_imp_domestics";
	public static final String EMPIREDAYQUEST_IMP_CRASHSITE_05_DOMESTICS = "crash_site_05_imp_domestics";
	public static final String EMPIREDAYQUEST_IMP_CRASHSITE_06_DOMESTICS = "crash_site_06_imp_domestics";
	public static final String EMPIREDAYQUEST_IMP_CRASHSITE_01_ENG = "crash_site_01_imp_engineering";
	public static final String EMPIREDAYQUEST_IMP_CRASHSITE_02_ENG = "crash_site_02_imp_engineering";
	public static final String EMPIREDAYQUEST_IMP_CRASHSITE_03_ENG = "crash_site_03_imp_engineering";
	public static final String EMPIREDAYQUEST_IMP_CRASHSITE_04_ENG = "crash_site_04_imp_engineering";
	public static final String EMPIREDAYQUEST_IMP_CRASHSITE_05_ENG = "crash_site_05_imp_engineering";
	public static final String EMPIREDAYQUEST_IMP_CRASHSITE_06_ENG = "crash_site_06_imp_engineering";
	public static final String EMPIREDAYQUEST_IMP_CRASHSITE_01_MUNITIONS = "crash_site_01_imp_munitions";
	public static final String EMPIREDAYQUEST_IMP_CRASHSITE_02_MUNITIONS = "crash_site_02_imp_munitions";
	public static final String EMPIREDAYQUEST_IMP_CRASHSITE_03_MUNITIONS = "crash_site_03_imp_munitions";
	public static final String EMPIREDAYQUEST_IMP_CRASHSITE_04_MUNITIONS = "crash_site_04_imp_munitions";
	public static final String EMPIREDAYQUEST_IMP_CRASHSITE_05_MUNITIONS = "crash_site_05_imp_munitions";
	public static final String EMPIREDAYQUEST_IMP_CRASHSITE_06_MUNITIONS = "crash_site_06_imp_munitions";
	public static final String EMPIREDAYQUEST_IMP_CRASHSITE_01_STRUC = "crash_site_01_imp_structures";
	public static final String EMPIREDAYQUEST_IMP_CRASHSITE_02_STRUC = "crash_site_02_imp_structures";
	public static final String EMPIREDAYQUEST_IMP_CRASHSITE_03_STRUC = "crash_site_03_imp_structures";
	public static final String EMPIREDAYQUEST_IMP_CRASHSITE_04_STRUC = "crash_site_04_imp_structures";
	public static final String EMPIREDAYQUEST_IMP_CRASHSITE_05_STRUC = "crash_site_05_imp_structures";
	public static final String EMPIREDAYQUEST_IMP_CRASHSITE_06_STRUC = "crash_site_06_imp_structures";
	
	public static final String EMPIREDAYQUEST_REB_CRASHSITE_01_DOMESTICS = "crash_site_01_reb_domestics";
	public static final String EMPIREDAYQUEST_REB_CRASHSITE_02_DOMESTICS = "crash_site_02_reb_domestics";
	public static final String EMPIREDAYQUEST_REB_CRASHSITE_03_DOMESTICS = "crash_site_03_reb_domestics";
	public static final String EMPIREDAYQUEST_REB_CRASHSITE_04_DOMESTICS = "crash_site_04_reb_domestics";
	public static final String EMPIREDAYQUEST_REB_CRASHSITE_05_DOMESTICS = "crash_site_05_reb_domestics";
	public static final String EMPIREDAYQUEST_REB_CRASHSITE_06_DOMESTICS = "crash_site_06_reb_domestics";
	public static final String EMPIREDAYQUEST_REB_CRASHSITE_01_ENG = "crash_site_01_reb_engineering";
	public static final String EMPIREDAYQUEST_REB_CRASHSITE_02_ENG = "crash_site_02_reb_engineering";
	public static final String EMPIREDAYQUEST_REB_CRASHSITE_03_ENG = "crash_site_03_reb_engineering";
	public static final String EMPIREDAYQUEST_REB_CRASHSITE_04_ENG = "crash_site_04_reb_engineering";
	public static final String EMPIREDAYQUEST_REB_CRASHSITE_05_ENG = "crash_site_05_reb_engineering";
	public static final String EMPIREDAYQUEST_REB_CRASHSITE_06_ENG = "crash_site_06_reb_engineering";
	public static final String EMPIREDAYQUEST_REB_CRASHSITE_01_MUNITIONS = "crash_site_01_reb_munitions";
	public static final String EMPIREDAYQUEST_REB_CRASHSITE_02_MUNITIONS = "crash_site_02_reb_munitions";
	public static final String EMPIREDAYQUEST_REB_CRASHSITE_03_MUNITIONS = "crash_site_03_reb_munitions";
	public static final String EMPIREDAYQUEST_REB_CRASHSITE_04_MUNITIONS = "crash_site_04_reb_munitions";
	public static final String EMPIREDAYQUEST_REB_CRASHSITE_05_MUNITIONS = "crash_site_05_reb_munitions";
	public static final String EMPIREDAYQUEST_REB_CRASHSITE_06_MUNITIONS = "crash_site_06_reb_munitions";
	public static final String EMPIREDAYQUEST_REB_CRASHSITE_01_STRUC = "crash_site_01_reb_structures";
	public static final String EMPIREDAYQUEST_REB_CRASHSITE_02_STRUC = "crash_site_02_reb_structures";
	public static final String EMPIREDAYQUEST_REB_CRASHSITE_03_STRUC = "crash_site_03_reb_structures";
	public static final String EMPIREDAYQUEST_REB_CRASHSITE_04_STRUC = "crash_site_04_reb_structures";
	public static final String EMPIREDAYQUEST_REB_CRASHSITE_05_STRUC = "crash_site_05_reb_structures";
	public static final String EMPIREDAYQUEST_REB_CRASHSITE_06_STRUC = "crash_site_06_reb_structures";
	
	public static final String EMPIREDAYQUEST_REB_ENTERTAINER_01 = "entertainer_rebel_01";
	public static final String EMPIREDAYQUEST_REB_ENTERTAINER_02 = "entertainer_rebel_02";
	public static final String EMPIREDAYQUEST_REB_ENTERTAINER_03 = "entertainer_rebel_03";
	public static final String EMPIREDAYQUEST_REB_ENTERTAINER_04 = "entertainer_rebel_04";
	public static final String EMPIREDAYQUEST_REB_ENTERTAINER_05 = "entertainer_rebel_05";
	public static final String EMPIREDAYQUEST_REB_ENTERTAINER_06 = "entertainer_rebel_06";
	
	public static final String EMPIREDAYQUEST_IMP_ENTERTAINER_01 = "entertainer_imperial_01";
	public static final String EMPIREDAYQUEST_IMP_ENTERTAINER_02 = "entertainer_imperial_02";
	public static final String EMPIREDAYQUEST_IMP_ENTERTAINER_03 = "entertainer_imperial_03";
	public static final String EMPIREDAYQUEST_IMP_ENTERTAINER_04 = "entertainer_imperial_04";
	public static final String EMPIREDAYQUEST_IMP_ENTERTAINER_05 = "entertainer_imperial_05";
	public static final String EMPIREDAYQUEST_IMP_ENTERTAINER_06 = "entertainer_imperial_06";
	
	public static final String EMPIREDAYQUEST_REB_COMBAT_01 = "recovery_quest_rebel_01";
	public static final String EMPIREDAYQUEST_REB_COMBAT_02 = "recovery_quest_rebel_02";
	public static final String EMPIREDAYQUEST_REB_COMBAT_03 = "recovery_quest_rebel_03";
	public static final String EMPIREDAYQUEST_REB_COMBAT_04 = "recovery_quest_rebel_04";
	public static final String EMPIREDAYQUEST_REB_COMBAT_05 = "recovery_quest_rebel_05";
	public static final String EMPIREDAYQUEST_REB_COMBAT_06 = "recovery_quest_rebel_06";
	public static final String EMPIREDAYQUEST_REB_COMBAT_07 = "recovery_quest_rebel_07";
	public static final String EMPIREDAYQUEST_REB_COMBAT_AUTOGRAPH = "recovery_quest_rebel_autograph";
	
	public static final String EMPIREDAYQUEST_REB_CORPSE_SIGNAL = "foundIntelRebelEmpireDay";
	public static final String EMPIREDAYQUEST_REB_CORPSE_TASK = "waitForIntelRebelEmpireDay";
	
	public static final String EMPIREDAYQUEST_IMP_COMBAT_01 = "recovery_quest_imperial_01";
	public static final String EMPIREDAYQUEST_IMP_COMBAT_02 = "recovery_quest_imperial_02";
	public static final String EMPIREDAYQUEST_IMP_COMBAT_03 = "recovery_quest_imperial_03";
	public static final String EMPIREDAYQUEST_IMP_COMBAT_04 = "recovery_quest_imperial_04";
	public static final String EMPIREDAYQUEST_IMP_COMBAT_05 = "recovery_quest_imperial_05";
	public static final String EMPIREDAYQUEST_IMP_COMBAT_06 = "recovery_quest_imperial_06";
	public static final String EMPIREDAYQUEST_IMP_COMBAT_07 = "recovery_quest_imperial_07";
	public static final String EMPIREDAYQUEST_IMP_COMBAT_NUMBER = "recovery_quest_imperial_number";
	
	public static final String EMPIREDAYQUEST_IMP_CORPSE_SIGNAL = "foundIntelImperialEmpireDay";
	public static final String EMPIREDAYQUEST_IMP_CORPSE_TASK = "waitForIntelImperialEmpireDay";
	
	public static final String VOL_TRIGGER_NAME_PREFIX = "empire_day_detainment_";
	
	public static final String IMPERIAL_FLYBY_PARTICLE_01 = "appearance/pt_trailing_distant_tie_fighters_slower_s01.prt";
	public static final String IMPERIAL_FLYBY_PARTICLE_02 = "appearance/pt_trailing_distant_tie_fighters_slower_s02.prt";
	public static final String REBEL_FLYBY_PARTICLE_01 = "appearance/pt_trailing_distant_rebel_ships_slow_s01.prt";
	public static final String REBEL_FLYBY_PARTICLE_02 = "appearance/pt_trailing_distant_rebel_ships_slow_s02.prt";
	public static final String REBEL_FLYBY_PARTICLE_03 = "appearance/pt_trailing_distant_rebel_ships_slow_s03.prt";
	public static final String REBEL_FLYBY_PARTICLE_04 = "appearance/pt_trailing_distant_rebel_ships_slow_s04.prt";
	
	public static final string_id DARTH_BADGE_1 = new string_id(EMP_DAY, "imp_darth_badge_1");
	
	public static final string_id DARTH_BADGE_2 = new string_id(EMP_DAY, "imp_darth_badge_2");
	
	public static final string_id LEIA_BADGE_1 = new string_id(EMP_DAY, "leia_badge_1");
	
	public static final string_id LEIA_BADGE_2 = new string_id(EMP_DAY, "leia_badge_2");
	
	public static final string_id BOW = new string_id(EMP_DAY, "bow");
	
	public static final int MAXIMUM_EMPIRE_DAY_WINNERS = 3;
	
	public static final int EMPIRE_DAY_SECONDS_TO_START = 900;
	public static final float OBJECT_NEAR_CHECK_RANGE_100M = 100.0f;
	public static final float OBJECT_NEAR_CHECK_RANGE_2M = 2.0f;
	public static final float OBJECT_NEAR_CHECK_RANGE_10M = 10.0f;
	public static final float NPC_ATTACK_RANGE = 30.0f;
	
	public static final int IMPERIAL_PLAYER = 1;
	public static final int REBEL_PLAYER = 2;
	public static final int NEUTRAL_PLAYER = 3;
	
	public static final int MIN_DESTROY_TIME = 60;
	public static final int MAX_DESTROY_TIME = 180;
	public static final int DAILY_COUNTER_LIMIT = 9;
	public static final int TOKEN_AMOUNT_COMBATANT = 1;
	public static final int TOKEN_AMOUNT_SF = 2;
	public static final int GCW_AMOUNT_SF = 10;
	public static final int GCW_AMOUNT_COMBATANT = 5;
	public static final int DAILY_TIME = 10;
	
	public static final int MAX_NUMBER_OF_PLANET_HIGH_SCORES = 5;
	public static final float VOL_TRIGGER_RADIUS = 10f;
	
	public static final float EMPIRE_DAY_EVENT_START_DELAY = 700.f;
	
	public static final String LOVEDAY_CUPID_MANAGER_NAME = "loveday_cupid_manager";
	public static final String LOVEDAY_CUPID_ELEMENT_NAME = "loveday_cupid_spawner_";
	
	public static final String LOVEDAY_CUPID_SPAWNER_CITY_VAR = "loveday_city";
	
	public static final String GETTING_CUPID_SPAWNER_IDS = "manager_gettingCupidSpawnerIds";
	
	public static final String[] LOVEDAY_LOCATIONS =
	{
		"endor", "kaadara", "tyrena"
	};
	
	public static final String ROMANCE_TRAITS_TABLE = "datatables/event/loveday/playing_cupid.iff";
	
	public static final String[] MATCHMAKING_COMPANIONS =
	{
		"loveday_romance_seeker_familiar_male_01",
		"loveday_romance_seeker_familiar_male_02",
		"loveday_romance_seeker_familiar_male_03",
		"loveday_romance_seeker_familiar_male_04",
		"loveday_romance_seeker_familiar_female_01",
		"loveday_romance_seeker_familiar_female_02",
		"loveday_romance_seeker_familiar_female_03",
		"loveday_romance_seeker_familiar_female_04"
	};
	
	public static final String[] MATCHMAKING_COMPANIONS_SLOTS =
	{
		"loveday_2009_matched_bob",
		"loveday_2009_matched_jim",
		"loveday_2009_matched_gary",
		"loveday_2009_matched_harvey",
		"loveday_2009_matched_jane",
		"loveday_2009_matched_betty",
		"loveday_2009_matched_margaret",
		"loveday_2009_matched_karen"
	};
	
	public static final String[] ALL_IMPERIAL_CRATE_TEMPLATES = 
	{
		"object/tangible/holiday/empire_day/e11_gas_cartridge_crate.iff",
		"object/tangible/holiday/empire_day/imperial_second_lieutenant_rank_crate.iff",
		"object/tangible/holiday/empire_day/improved_mouse_droid_arm_crate.iff",
		"object/tangible/holiday/empire_day/command_console_comscan_subcomponent_crate.iff"
	};
	
	public static final String[] ALL_REBEL_CRATE_TEMPLATES = 
	{
		"object/tangible/holiday/empire_day/aa1n_vebrobrain_protocol_droid_component_crate.iff",
		"object/tangible/holiday/empire_day/rebel_trooper_field_rations_crate.iff",
		"object/tangible/holiday/empire_day/crew_carrier_steering_mechanism_crate.iff",
		"object/tangible/holiday/empire_day/dh17_gas_cartridge_crate.iff"
	};
	
	public static final String[] ALL_IMPERIAL_QUESTS = 
	{
		EMPIREDAYQUEST_IMP_CRASHSITE_01_DOMESTICS,
		EMPIREDAYQUEST_IMP_CRASHSITE_02_DOMESTICS,
		EMPIREDAYQUEST_IMP_CRASHSITE_03_DOMESTICS,
		EMPIREDAYQUEST_IMP_CRASHSITE_04_DOMESTICS,
		EMPIREDAYQUEST_IMP_CRASHSITE_05_DOMESTICS,
		EMPIREDAYQUEST_IMP_CRASHSITE_06_DOMESTICS,
		EMPIREDAYQUEST_IMP_CRASHSITE_01_ENG,
		EMPIREDAYQUEST_IMP_CRASHSITE_02_ENG,
		EMPIREDAYQUEST_IMP_CRASHSITE_03_ENG,
		EMPIREDAYQUEST_IMP_CRASHSITE_04_ENG,
		EMPIREDAYQUEST_IMP_CRASHSITE_05_ENG,
		EMPIREDAYQUEST_IMP_CRASHSITE_06_ENG,
		EMPIREDAYQUEST_IMP_CRASHSITE_01_MUNITIONS,
		EMPIREDAYQUEST_IMP_CRASHSITE_02_MUNITIONS,
		EMPIREDAYQUEST_IMP_CRASHSITE_03_MUNITIONS,
		EMPIREDAYQUEST_IMP_CRASHSITE_04_MUNITIONS,
		EMPIREDAYQUEST_IMP_CRASHSITE_05_MUNITIONS,
		EMPIREDAYQUEST_IMP_CRASHSITE_06_MUNITIONS,
		EMPIREDAYQUEST_IMP_CRASHSITE_01_STRUC,
		EMPIREDAYQUEST_IMP_CRASHSITE_02_STRUC,
		EMPIREDAYQUEST_IMP_CRASHSITE_03_STRUC,
		EMPIREDAYQUEST_IMP_CRASHSITE_04_STRUC,
		EMPIREDAYQUEST_IMP_CRASHSITE_05_STRUC,
		EMPIREDAYQUEST_IMP_CRASHSITE_06_STRUC	
	};
	public static final String[] ALL_REBEL_QUESTS = 
	{
		EMPIREDAYQUEST_REB_CRASHSITE_01_DOMESTICS,
		EMPIREDAYQUEST_REB_CRASHSITE_02_DOMESTICS,
		EMPIREDAYQUEST_REB_CRASHSITE_03_DOMESTICS,
		EMPIREDAYQUEST_REB_CRASHSITE_04_DOMESTICS,
		EMPIREDAYQUEST_REB_CRASHSITE_05_DOMESTICS,
		EMPIREDAYQUEST_REB_CRASHSITE_06_DOMESTICS,
		EMPIREDAYQUEST_REB_CRASHSITE_01_ENG,
		EMPIREDAYQUEST_REB_CRASHSITE_02_ENG,
		EMPIREDAYQUEST_REB_CRASHSITE_03_ENG,
		EMPIREDAYQUEST_REB_CRASHSITE_04_ENG,
		EMPIREDAYQUEST_REB_CRASHSITE_05_ENG,
		EMPIREDAYQUEST_REB_CRASHSITE_06_ENG,
		EMPIREDAYQUEST_REB_CRASHSITE_01_MUNITIONS,
		EMPIREDAYQUEST_REB_CRASHSITE_02_MUNITIONS,
		EMPIREDAYQUEST_REB_CRASHSITE_03_MUNITIONS,
		EMPIREDAYQUEST_REB_CRASHSITE_04_MUNITIONS,
		EMPIREDAYQUEST_REB_CRASHSITE_05_MUNITIONS,
		EMPIREDAYQUEST_REB_CRASHSITE_06_MUNITIONS,
		EMPIREDAYQUEST_REB_CRASHSITE_01_STRUC,
		EMPIREDAYQUEST_REB_CRASHSITE_02_STRUC,
		EMPIREDAYQUEST_REB_CRASHSITE_03_STRUC,
		EMPIREDAYQUEST_REB_CRASHSITE_04_STRUC,
		EMPIREDAYQUEST_REB_CRASHSITE_05_STRUC,
		EMPIREDAYQUEST_REB_CRASHSITE_06_STRUC	
	};
	
	public static final String[] ALL_IMPERIAL_QUEST_TEMPLATES = 
	{
		ALL_IMPERIAL_CRATE_TEMPLATES[1],
		ALL_IMPERIAL_CRATE_TEMPLATES[1],
		ALL_IMPERIAL_CRATE_TEMPLATES[1],
		ALL_IMPERIAL_CRATE_TEMPLATES[1],
		ALL_IMPERIAL_CRATE_TEMPLATES[1],
		ALL_IMPERIAL_CRATE_TEMPLATES[1],
		ALL_IMPERIAL_CRATE_TEMPLATES[2],
		ALL_IMPERIAL_CRATE_TEMPLATES[2],
		ALL_IMPERIAL_CRATE_TEMPLATES[2],
		ALL_IMPERIAL_CRATE_TEMPLATES[2],
		ALL_IMPERIAL_CRATE_TEMPLATES[2],
		ALL_IMPERIAL_CRATE_TEMPLATES[2],
		ALL_IMPERIAL_CRATE_TEMPLATES[0],
		ALL_IMPERIAL_CRATE_TEMPLATES[0],
		ALL_IMPERIAL_CRATE_TEMPLATES[0],
		ALL_IMPERIAL_CRATE_TEMPLATES[0],
		ALL_IMPERIAL_CRATE_TEMPLATES[0],
		ALL_IMPERIAL_CRATE_TEMPLATES[0],
		ALL_IMPERIAL_CRATE_TEMPLATES[3],
		ALL_IMPERIAL_CRATE_TEMPLATES[3],
		ALL_IMPERIAL_CRATE_TEMPLATES[3],
		ALL_IMPERIAL_CRATE_TEMPLATES[3],
		ALL_IMPERIAL_CRATE_TEMPLATES[3],
		ALL_IMPERIAL_CRATE_TEMPLATES[3]	
	};
	public static final String[] ALL_REBEL_QUEST_TEMPLATES = 
	{
		ALL_REBEL_CRATE_TEMPLATES[1],
		ALL_REBEL_CRATE_TEMPLATES[1],
		ALL_REBEL_CRATE_TEMPLATES[1],
		ALL_REBEL_CRATE_TEMPLATES[1],
		ALL_REBEL_CRATE_TEMPLATES[1],
		ALL_REBEL_CRATE_TEMPLATES[1],
		ALL_REBEL_CRATE_TEMPLATES[0],
		ALL_REBEL_CRATE_TEMPLATES[0],
		ALL_REBEL_CRATE_TEMPLATES[0],
		ALL_REBEL_CRATE_TEMPLATES[0],
		ALL_REBEL_CRATE_TEMPLATES[0],
		ALL_REBEL_CRATE_TEMPLATES[0],
		ALL_REBEL_CRATE_TEMPLATES[3],
		ALL_REBEL_CRATE_TEMPLATES[3],
		ALL_REBEL_CRATE_TEMPLATES[3],
		ALL_REBEL_CRATE_TEMPLATES[3],
		ALL_REBEL_CRATE_TEMPLATES[3],
		ALL_REBEL_CRATE_TEMPLATES[3],
		ALL_REBEL_CRATE_TEMPLATES[2],
		ALL_REBEL_CRATE_TEMPLATES[2],
		ALL_REBEL_CRATE_TEMPLATES[2],
		ALL_REBEL_CRATE_TEMPLATES[2],
		ALL_REBEL_CRATE_TEMPLATES[2],
		ALL_REBEL_CRATE_TEMPLATES[2]	
	};
	
	public static final String[] ALL_REBEL_ENTERTAINER_QUESTS = 
	{
		EMPIREDAYQUEST_REB_ENTERTAINER_01,
		EMPIREDAYQUEST_REB_ENTERTAINER_02,
		EMPIREDAYQUEST_REB_ENTERTAINER_03,
		EMPIREDAYQUEST_REB_ENTERTAINER_04,
		EMPIREDAYQUEST_REB_ENTERTAINER_05,
		EMPIREDAYQUEST_REB_ENTERTAINER_06
	};
	
	public static final String[] ALL_IMPERIAL_ENTERTAINER_QUESTS = 
	{
		EMPIREDAYQUEST_IMP_ENTERTAINER_01,
		EMPIREDAYQUEST_IMP_ENTERTAINER_02,
		EMPIREDAYQUEST_IMP_ENTERTAINER_03,
		EMPIREDAYQUEST_IMP_ENTERTAINER_04,
		EMPIREDAYQUEST_IMP_ENTERTAINER_05,
		EMPIREDAYQUEST_IMP_ENTERTAINER_06
	};
	
	public static final String[] DEFAULT_HIGH_SCORE_LIST = 
	{
		"DevH",
		"Loche",
		"Milbarge",
		"Jaskell",
		"AdeptStrain",
		"DeadMeat",
		"CancelAutoRun",
		"Dotanuki",
		"Tereb",
		"Hanse",
		"Teesquared",
		"DevH",
		"Loche",
		"Milbarge",
		"Jaskell",
		"AdeptStrain",
		"Swede",
		"Blixtev",
		"DeadMeat",
		"CancelAutoRun",
		"Dotanuki",
		"Tereb",
		"Hanse",
		"Teesquared"
	};
	
	
	public static boolean hasLovedayRomanceSeekerCompanion(obj_id player) throws InterruptedException
	{
		return isIdValid(getLovedayRomanceSeekerCompanionControlDevice(player));
	}
	
	
	public static obj_id getLovedayRomanceSeekerCompanionControlDevice(obj_id player) throws InterruptedException
	{
		obj_id datapad = utils.getPlayerDatapad(player);
		
		if (isIdValid(datapad))
		{
			obj_id[] dataItems = getContents(datapad);
			if (dataItems != null || dataItems.length > 0)
			{
				for (int i = 0; i < dataItems.length; i++)
				{
					testAbortScript();
					obj_id dataItem = dataItems[i];
					if (hasObjVar(dataItem, "pet.creatureName"))
					{
						if (getStringObjVar(dataItem, "pet.creatureName").startsWith("loveday_romance_seeker_familiar"))
						{
							return dataItem;
						}
					}
				}
			}
		}
		
		return obj_id.NULL_ID;
	}
	
	
	public static boolean grantLovedayRomanceSeekerCompanion(obj_id player) throws InterruptedException
	{
		if (!hasLovedayRomanceSeekerCompanion(player))
		{
			String controlTemplate = "object/intangible/pet/loveday_romance_seeker.iff";
			obj_id datapad = utils.getPlayerDatapad(player);
			obj_id controlDevice = createObject(controlTemplate, datapad, "");
			
			String seekerChoice = MATCHMAKING_COMPANIONS[rand(0, MATCHMAKING_COMPANIONS.length-1)];
			if (utils.hasScriptVar(player, "loveday.todaysCompanion"))
			{
				seekerChoice = utils.getStringScriptVar(player, "loveday.todaysCompanion");
			}
			
			setObjVar(controlDevice, "pet.creatureName", seekerChoice);
			
			attachScript( controlDevice, "ai.pet_control_device");
			setName( controlDevice, getString(new string_id("mob/creature_names", seekerChoice)) );
			int petType = pet_lib.PET_TYPE_FAMILIAR;
			setObjVar( controlDevice, "ai.pet.type", petType );
			
			setObjVar(controlDevice, "item.lifespan", 86400);
			attachScript(controlDevice, "item.special.temporary_item");
			
			int chosenSet = getRomanticTraitsSet();
			if (utils.hasScriptVar(player, "loveday.todaysCompanionTraits"))
			{
				chosenSet = utils.getIntScriptVar(player, "loveday.todaysCompanionTraits");
			}
			
			setRomanticTraits(controlDevice, chosenSet);
			
			utils.setScriptVar(player, "loveday.todaysCompanionTraits", chosenSet);
			return true;
		}
		
		return false;
	}
	
	
	public static void checkForAndGrantLovedayMatchmakingSlot(obj_id player, String companionType) throws InterruptedException
	{
		if (!isIdValid(player))
		{
			return;
		}
		if (companionType == null || companionType.length() < 1)
		{
			return;
		}
		
		int index = utils.getElementPositionInArray(MATCHMAKING_COMPANIONS, companionType);
		if (index > -1)
		{
			String companionSlotName = MATCHMAKING_COMPANIONS_SLOTS[index];
			if (!hasCompletedCollectionSlot(player, companionSlotName))
			{
				modifyCollectionSlotValue(player, companionSlotName, 1);
			}
		}
		
		return;
	}
	
	
	public static int getNumRomanticTraitsSets() throws InterruptedException
	{
		int numRows = dataTableGetNumRows(ROMANCE_TRAITS_TABLE);
		return numRows;
	}
	
	
	public static int getRomanticTraitsSet() throws InterruptedException
	{
		int numSets = getNumRomanticTraitsSets();
		int chosenSet = rand(0, numSets - 1);
		return chosenSet;
	}
	
	
	public static void setRomanticTraits(obj_id npc, int chosenSet) throws InterruptedException
	{
		dictionary traits = dataTableGetRow(ROMANCE_TRAITS_TABLE, chosenSet);
		
		setObjVar(npc, "playingCupid.traitSet", chosenSet);
		setObjVar(npc, "playingCupid.trait_01", traits.getString("trait_01"));
		setObjVar(npc, "playingCupid.trait_02", traits.getString("trait_02"));
		setObjVar(npc, "playingCupid.trait_03", traits.getString("trait_03"));
		setObjVar(npc, "playingCupid.trait_04", traits.getString("trait_04"));
		setObjVar(npc, "playingCupid.trait_05", traits.getString("trait_05"));
		return;
	}
	
	
	public static boolean playEmpireDayFireWorksAndFlyBys(obj_id masterObject, int faction) throws InterruptedException
	{
		CustomerServiceLog("holidayEvent", "emperor_statue.playFireWorksAndFlyBys: Function initialized.");
		
		if (!isValidId(masterObject) || !exists(masterObject))
		{
			return false;
		}
		if (faction <= 0)
		{
			return false;
		}
		
		messageTo(masterObject, "launchRandomFirework", null, 3.0f, false);
		messageTo(masterObject, "launchRandomFirework", null, 5.0f, false);
		messageTo(masterObject, "launchRandomFirework", null, 6.0f, false);
		messageTo(masterObject, "launchRandomFirework", null, 10.0f, false);
		messageTo(masterObject, "launchRandomFirework", null, 5.0f, false);
		messageTo(masterObject, "launchRandomFirework", null, 7.0f, false);
		messageTo(masterObject, "launchRandomFirework", null, 9.0f, false);
		messageTo(masterObject, "launchRandomFirework", null, 11.0f, false);
		messageTo(masterObject, "launchRandomFirework", null, 13.0f, false);
		messageTo(masterObject, "launchRandomFirework", null, 15.0f, false);
		messageTo(masterObject, "launchRandomFirework", null, 16.0f, false);
		messageTo(masterObject, "launchRandomFirework", null, 17.0f, false);
		messageTo(masterObject, "launchRandomFirework", null, 18.0f, false);
		messageTo(masterObject, "launchRandomFirework", null, 18.0f, false);
		messageTo(masterObject, "launchRandomFirework", null, 18.0f, false);
		messageTo(masterObject, "launchRandomFirework", null, 18.0f, false);
		messageTo(masterObject, "launchRandomFirework", null, 19.0f, false);
		messageTo(masterObject, "launchRandomFirework", null, 19.0f, false);
		messageTo(masterObject, "launchRandomFirework", null, 19.0f, false);
		messageTo(masterObject, "launchRandomFirework", null, 19.0f, false);
		messageTo(masterObject, "launchRandomFirework", null, 20.0f, false);
		messageTo(masterObject, "launchRandomFirework", null, 20.0f, false);
		messageTo(masterObject, "launchRandomFirework", null, 20.0f, false);
		messageTo(masterObject, "launchRandomFirework", null, 20.0f, false);
		
		if (faction == IMPERIAL_PLAYER)
		{
			messageTo(masterObject, "launchRandomTieFighterFlyBy", null, 4, false);
			messageTo(masterObject, "launchRandomTieFighterFlyBy", null, 8, false);
			messageTo(masterObject, "launchRandomTieFighterFlyBy", null, 12, false);
			messageTo(masterObject, "launchRandomTieFighterFlyBy", null, 2, false);
			messageTo(masterObject, "launchRandomTieFighterFlyBy", null, 6, false);
			messageTo(masterObject, "launchRandomTieFighterFlyBy", null, 10, false);
		}
		else
		{
			messageTo(masterObject, "launchRandomRebelShipFlyBy", null, 4, false);
			messageTo(masterObject, "launchRandomRebelShipFlyBy", null, 8, false);
			messageTo(masterObject, "launchRandomRebelShipFlyBy", null, 12, false);
			messageTo(masterObject, "launchRandomRebelShipFlyBy", null, 2, false);
			messageTo(masterObject, "launchRandomRebelShipFlyBy", null, 6, false);
			messageTo(masterObject, "launchRandomRebelShipFlyBy", null, 10, false);
			
		}
		return true;
		
	}
	
	
	public static String getEmpireDayEligibility(obj_id player, int faction) throws InterruptedException
	{
		if (!isValidId(player) || !exists(player))
		{
			return null;
		}
		if (faction <= 0)
		{
			return null;
		}
		
		if (holiday.isEmpireDayPlayerEligible(player, faction))
		{
			if (faction == IMPERIAL_PLAYER)
			{
				return "You are eligible for the Champion of Empire Day badge from Darth Vader.";
			}
			else
			{
				return "You are eligible for the Champion of Remembrance Day badge from Princess Leia.";
			}
		}
		if (faction == IMPERIAL_PLAYER)
		{
			
			return "You are NOT eligible for the Champion of Empire Day badge from Darth Vader.";
		}
		else
		{
			return "You are NOT eligible for the Champion of Remembrance Day badge from Princess Leia.";
		}
		
	}
	
	
	public static boolean isEmpireDayPlayerEligible(obj_id player, int faction) throws InterruptedException
	{
		if (!isValidId(player) || !exists(player))
		{
			return false;
		}
		
		if (faction == IMPERIAL_PLAYER && !factions.isImperial(player))
		{
			return false;
		}
		
		if (faction == REBEL_PLAYER && !factions.isRebel(player))
		{
			return false;
		}
		
		int tooYoung = getCurrentBirthDate() - getPlayerBirthDate(player);
		if (tooYoung < 10)
		{
			return false;
		}
		
		if (( isPlayerConnected(player) && !isAwayFromKeyBoard(player) && !hasCompletedCollectionSlot(player, holiday.EMPIRE_DAY_CHAMPION_BADGE) && !hasCompletedCollectionSlot(player, holiday.REMEMBRANCE_DAY_CHAMPION_BADGE) && !isGod(player) && !isDead(player) && !ai_lib.isInCombat(player) && !isIncapacitated(player) && !utils.isFreeTrial(player) && isInWorldCell(player) ))
		{
			return true;
		}
		return false;
	}
	
	
	public static obj_id[] getEmpireDayWinningPlayers(obj_id holidayObject, Vector eligiblePlayers) throws InterruptedException
	{
		CustomerServiceLog("holidayEvent", "holiday.getEmpireDayWinningPlayers: init");
		
		if (!isIdValid(holidayObject) || !exists(holidayObject))
		{
			return null;
		}
		
		if (eligiblePlayers == null || eligiblePlayers.size() <= 0)
		{
			return null;
		}
		
		int maxEmpireDayWinners = MAXIMUM_EMPIRE_DAY_WINNERS;
		
		String empiredayWinnerConfig = getConfigSetting("GameServer", "empireday_ceremony_winners");
		if (empiredayWinnerConfig != null && utils.stringToInt(empiredayWinnerConfig) > -1)
		{
			CustomerServiceLog("holidayEvent", "holiday.getEmpireDayWinningPlayers: The max badge winners was: "+maxEmpireDayWinners+" but there is a server config that might overwrite this amount.");
			int configWinners = utils.stringToInt(empiredayWinnerConfig);
			CustomerServiceLog("holidayEvent", "holiday.getEmpireDayWinningPlayers: The server config shows the max badge winners should be: "+configWinners+" checking to see if that is less or more than the current amount.");
			if (configWinners != maxEmpireDayWinners)
			{
				CustomerServiceLog("holidayEvent", "holiday.getEmpireDayWinningPlayers: The server config if a different amount than the current max so we are using the server config.");
				maxEmpireDayWinners = configWinners;
			}
		}
		
		int originalListSize = eligiblePlayers.size();
		
		if (originalListSize <= maxEmpireDayWinners)
		{
			CustomerServiceLog("holidayEvent", "holiday.getEmpireDayWinningPlayers: There is less than "+maxEmpireDayWinners+" or exactly "+maxEmpireDayWinners+" winning players");
			obj_id[] _eligiblePlayers = new obj_id[0];
			if (eligiblePlayers != null)
			{
				_eligiblePlayers = new obj_id[eligiblePlayers.size()];
				eligiblePlayers.toArray(_eligiblePlayers);
			}
			return _eligiblePlayers;
		}
		
		CustomerServiceLog("holidayEvent", "holiday.getEmpireDayWinningPlayers: creating lists of players and winners.");
		Vector winningPlayers = new Vector();
		winningPlayers.setSize(0);
		CustomerServiceLog("holidayEvent", "holiday.getEmpireDayWinningPlayers: originalListSize: "+originalListSize);
		
		for (int i = 0; i < maxEmpireDayWinners; i++)
		{
			testAbortScript();
			int randomWinner = rand(0, (eligiblePlayers.size()-1));
			
			winningPlayers.add(((obj_id)(eligiblePlayers.get(randomWinner))));
			
			eligiblePlayers.remove(((obj_id)(eligiblePlayers.get(randomWinner))));
			
			if (eligiblePlayers.size() >= (originalListSize - i))
			{
				CustomerServiceLog("holidayEvent", "holiday.getEmpireDayWinningPlayers: Error with eligiblePlayers list. It did not remove a player on iteration: "+i);
				break;
			}
			else if (winningPlayers.size() <= (0 + i))
			{
				CustomerServiceLog("holidayEvent", "holiday.getEmpireDayWinningPlayers: Error with winningPlayers list. Did not add a player properly on iteration: "+i);
				break;
			}
		}
		obj_id[] _winningPlayers = new obj_id[0];
		if (winningPlayers != null)
		{
			_winningPlayers = new obj_id[winningPlayers.size()];
			winningPlayers.toArray(_winningPlayers);
		}
		return _winningPlayers;
	}
	
	
	public static String currentYearObjVar() throws InterruptedException
	{
		
		return EMPIRE_DAY_RECEIVED_VII;
	}
	
	
	public static boolean grantEmpireDayGift(obj_id player, int faction) throws InterruptedException
	{
		if (!isIdValid(player))
		{
			return false;
		}
		
		if (faction <= 0)
		{
			return false;
		}
		
		CustomerServiceLog("grantGift", getFirstName(player) + "("+ player + ") is about to receive event gifts");
		
		String eventGiftOne = IMPERIAL_AND_REBEL_EVENT_GIFT;
		String eventGiftTwo = "";
		if (faction == REBEL_PLAYER)
		{
			CustomerServiceLog("grantGift", getFirstName(player) + "("+ player + ") has been identified as faction: "+faction+" so the player will receive appropriate factional gifts");
			eventGiftTwo = REBEL_REMEMBRANCE_DAY_GIFT;
		}
		else
		{
			CustomerServiceLog("grantGift", getFirstName(player) + "("+ player + ") has been identified as faction: "+faction+" so the player will receive appropriate factional gifts");
			eventGiftTwo = IMPERIAL_EMPIRE_DAY_GIFT;
		}
		
		obj_id inv = utils.getInventoryContainer(player);
		if (!isValidId(inv) || !exists(inv))
		{
			CustomerServiceLog("grantGift", getFirstName(player) + "("+ player + ") did not have a valid inventory. exiting without giving gifts.");
			return false;
		}
		
		obj_id giftOne = static_item.createNewItemFunction(eventGiftOne, inv);
		if (!isValidId(giftOne) || !exists(giftOne))
		{
			CustomerServiceLog("grantGift", getFirstName(player) + "("+ player + ") failed to receive gift one: "+eventGiftOne+" this may be due to having a full inventory or the item being corrupted.");
			return false;
		}
		
		obj_id giftTwo = static_item.createNewItemFunction(eventGiftTwo, inv);
		if (!isValidId(giftTwo) || !exists(giftTwo))
		{
			CustomerServiceLog("grantGift", getFirstName(player) + "("+ player + ") failed to receive gift one: "+eventGiftTwo+" this may be due to having a full inventory or the item being corrupted.");
			return false;
		}
		
		setObjVar(player, holiday.currentYearObjVar(), 1);
		sendSystemMessage(player, GIFT_GRANTED);
		CustomerServiceLog("grantGift", getFirstName(player) + "("+ player + ") has received "+faction+" gifts: "+eventGiftOne+" and "+eventGiftTwo);
		
		return true;
	}
	
	
	public static int getEmoteBeligerence(String emote) throws InterruptedException
	{
		int emote_row = dataTableSearchColumnForString(emote, 0, EMOTE_DATATABLE);
		int st_emote_type = dataTableGetInt(EMOTE_DATATABLE, emote_row, 1);
		
		if (st_emote_type == 2)
		{
			return 2;
		}
		else if (st_emote_type == 1)
		{
			return 1;
		}
		else if (st_emote_type == 0)
		{
			return 0;
		}
		else
		{
			return -1;
		}
	}
	
	
	public static obj_id getEmpireDayWaypointObjectObjId(obj_id object, String waypoint, float range) throws InterruptedException
	{
		if (!isIdValid(object) || !exists(object))
		{
			return null;
		}
		if (waypoint == null || waypoint.equals(""))
		{
			return null;
		}
		if (range <= 0)
		{
			return null;
		}
		
		location loc = getLocation(object);
		if (loc == null)
		{
			return null;
		}
		
		obj_id[] objects = getObjectsInRange(loc, range);
		for (int i = 0; i < objects.length; i++)
		{
			testAbortScript();
			if (isIdValid(objects[i]))
			{
				if (hasObjVar(objects[i], waypoint))
				{
					obj_id waypointObjId = objects[i];
					return waypointObjId;
				}
			}
		}
		
		return null;
	}
	
	
	public static obj_id masterObjectObjId(obj_id self) throws InterruptedException
	{
		location loc = getLocation(self);
		obj_id[] objects = getObjectsInRange(loc, OBJECT_NEAR_CHECK_RANGE_100M);
		for (int i = 0; i < objects.length; i++)
		{
			testAbortScript();
			if (isIdValid(objects[i]))
			{
				if (hasObjVar(objects[i], "emperorsday.master"))
				{
					obj_id masterObjId = objects[i];
					return masterObjId;
				}
			}
		}
		return null;
	}
	
	
	public static boolean giveCeremonyAreaMobsHate(obj_id player, float range) throws InterruptedException
	{
		if (!isIdValid(player) || !exists(player))
		{
			return false;
		}
		if (range <= 0)
		{
			range = 10.0f;
		}
		if (range > 100)
		{
			range = 100.0f;
		}
		
		location playerLoc = getLocation(player);
		if (playerLoc == null)
		{
			return false;
		}
		
		obj_id[] allMobsCombat = getAllNpcs(playerLoc, range);
		if (allMobsCombat == null || allMobsCombat.length <= 0)
		{
			return false;
		}
		
		for (int i = 0; i < allMobsCombat.length; i++)
		{
			testAbortScript();
			
			if (allMobsCombat[i] == player)
			{
				continue;
			}
			if (isInvulnerable(allMobsCombat[i]))
			{
				continue;
			}
			if (!pvpCanAttack(allMobsCombat[i], player))
			{
				continue;
			}
			
			startCombat(allMobsCombat[i], player);
		}
		return true;
	}
	
	
	public static boolean grantEmperorDayBadge(obj_id player, obj_id celeb, obj_id[] listOfWinners, int faction) throws InterruptedException
	{
		
		obj_id masterObject = holiday.masterObjectObjId(celeb);
		if (!isIdValid(masterObject) && !exists(masterObject))
		{
			CustomerServiceLog("holidayEvent", "imperial_npc.grantEmperorDayBadge: No Emperor Statue Found! Player: "+player+" cannot receive rewards from Vader!");
			return false;
		}
		
		if (listOfWinners == null || listOfWinners.length <= 0)
		{
			CustomerServiceLog("holidayEvent", "imperial_npc.grantEmperorDayBadge: The list of winners was not found. Vader cannot give the player his badge.");
			return false;
		}
		
		boolean foundPlayerObjVar = false;
		for (int i = 0; i < listOfWinners.length; i++)
		{
			testAbortScript();
			obj_id playerOid = getObjIdObjVar(celeb, "readyForBadge.winner_"+(i+1));
			if (playerOid != player)
			{
				continue;
			}
			CustomerServiceLog("holidayEvent", "imperial_npc.grantEmperorDayBadge: The player: "+player+" OID was found on Vader, removing it and awarding player badge.");
			
			foundPlayerObjVar = true;
			removeObjVar(celeb, "readyForBadge.winner_"+(i+1));
		}
		
		if (!foundPlayerObjVar)
		{
			CustomerServiceLog("holidayEvent", "imperial_npc.grantEmperorDayBadge: The player's objvar was not found. Vader cannot give the player his badge.");
			return false;
		}
		
		doAnimationAction(celeb, "nod_head_once");
		doAnimationAction(player, "bow2");
		utils.removeScriptVar(player, "emperorsDayBadge");
		if (faction == IMPERIAL_PLAYER)
		{
			if (!modifyCollectionSlotValue(player, holiday.EMPIRE_DAY_CHAMPION_BADGE, 1))
			{
				CustomerServiceLog("holidayEvent", "imperial_npc.grantEmperorDayBadge: Could not modify badge slot: "+holiday.EMPIRE_DAY_CHAMPION_BADGE+" for Player: "+player+". Player cannot receive rewards from Vader!");
				return false;
			}
			
			factions.addFactionStanding(player, factions.FACTION_IMPERIAL, 250);
			prose_package pp2 = prose.getPackage(DARTH_BADGE_1);
			prose.setTT (pp2, player);
			chat.chat(celeb, player, pp2);
			CustomerServiceLog("holidayEvent", "imperial_npc.grantEmperorDayBadge: Player: "+player+" has received their badge.");
			return true;
		}
		
		if (!modifyCollectionSlotValue(player, holiday.REMEMBRANCE_DAY_CHAMPION_BADGE, 1))
		{
			CustomerServiceLog("holidayEvent", "imperial_npc.grantEmperorDayBadge: Could not modify badge slot: "+holiday.EMPIRE_DAY_CHAMPION_BADGE+" for Player: "+player+". Player cannot receive rewards from Vader!");
			return false;
		}
		
		factions.addFactionStanding(player, factions.FACTION_REBEL, 250);
		prose_package pp2 = prose.getPackage(LEIA_BADGE_1);
		prose.setTT (pp2, player);
		chat.chat(celeb, player, pp2);
		doAnimationAction(celeb, "clap_rousing");
		
		messageTo(masterObject, "hanHappy", null, 2.0f, false);
		return true;
	}
	
	
	public static boolean slapPlayerDownForBeingRude(obj_id npcObject, obj_id playerToBeIncapped) throws InterruptedException
	{
		if (!isValidId(npcObject) && !exists(npcObject))
		{
			return false;
		}
		if (!isValidId(playerToBeIncapped) && !exists(playerToBeIncapped))
		{
			return false;
		}
		
		stealth.testInvisNonCombatAction(playerToBeIncapped, npcObject);
		doAnimationAction(npcObject, "stop");
		
		dictionary eDict = new dictionary();
		eDict.put("myTarget", playerToBeIncapped);
		messageTo(npcObject, "playerKnockedOut", eDict, 0, false);
		return true;
	}
	
	
	public static boolean replaceEventObject(obj_id eventObject, obj_id player, String objectTemplate) throws InterruptedException
	{
		if (!isValidId(eventObject) || !exists(eventObject))
		{
			return false;
		}
		if (!isValidId(player) || !exists(player))
		{
			return false;
		}
		if (objectTemplate == null || objectTemplate.equals(""))
		{
			return false;
		}
		
		if (!hasObjVar(eventObject, "objParent"))
		{
			CustomerServiceLog("holidayEvent", "holiday.replaceEventObject: object "+ eventObject + " doesn't have the object parent objvar so it will be destroyed and the player receives no credit.");
			trial.cleanupObject(eventObject);
			return false;
		}
		
		obj_id objParent = getObjIdObjVar(eventObject, "objParent");
		if (!isValidId(objParent) || !exists(objParent))
		{
			CustomerServiceLog("holidayEvent", "holiday.replaceEventObject: object "+ eventObject + " could not find parent object: "+objParent+" so it will be destroyed and the player receives no credit.");
			trial.cleanupObject(eventObject);
			return false;
		}
		
		if (!utils.hasScriptVar(eventObject, "deathTracker"))
		{
			CustomerServiceLog("holidayEvent", "holiday.replaceEventObject: object "+ eventObject + " did not have the deathTracker scriptvar so it will be destroyed and the player receives no credit.");
			trial.cleanupObject(eventObject);
			return false;
		}
		
		float respawnTime = 20.f;
		if (hasObjVar(eventObject, "fltRespawnTime"))
		{
			respawnTime = getFloatObjVar(eventObject, "fltRespawnTime");
		}
		
		location loc = utils.getLocationScriptVar(eventObject, "deathTracker");
		if (loc == null)
		{
			CustomerServiceLog("holidayEvent", "holiday.replaceEventObject: object "+ eventObject + " had a corrupt location so it will be destroyed and the player receives no credit.");
			trial.cleanupObject(eventObject);
			return false;
		}
		
		obj_id replacementEventObject = createObject(objectTemplate, loc);
		if (!isValidId(objParent) || !exists(objParent))
		{
			CustomerServiceLog("holidayEvent", "holiday.replaceEventObject: object "+ eventObject + " could not spawn a replacement object. This is likely due to an incorrect replacement template string. The event object will be destroyed and the player receives no credit.");
			trial.cleanupObject(eventObject);
			return false;
		}
		
		float yaw = getYaw(eventObject);
		
		CustomerServiceLog("holidayEvent", "holiday.replaceEventObject: object "+ eventObject + " did not have the deathTracker scriptvar so it will be destroyed and the plaeyr receives no credit.");
		
		setObjVar(eventObject, "dontCountDeath", 1);
		setObjVar(replacementEventObject, "objParent", objParent);
		setObjVar(replacementEventObject, "fltRespawnTime", respawnTime);
		utils.setScriptVar(replacementEventObject, "deathTracker", loc);
		setYaw(replacementEventObject, yaw);
		attachScript(replacementEventObject, "systems.spawning.spawned_tracker");
		playClientEffectLoc(player, SMOKE_CLIENT_EFFECT, loc, 0);
		trial.cleanupObject(eventObject);
		return true;
	}
	
	
	public static boolean rewardEmpireDayPlayer(obj_id player, int playerFaction, String parentBuffName, String childBuffName, String collectionSlot, String eventName) throws InterruptedException
	{
		CustomerServiceLog("holidayEvent", "holiday.rewardEmpireDayPlayer: Player "+ player + " is attempting to increment the childcounter buff: "+childBuffName+".");
		
		if (!isValidId(player) || !exists(player))
		{
			return false;
		}
		if (playerFaction < 0 || (playerFaction != IMPERIAL_PLAYER && playerFaction != REBEL_PLAYER))
		{
			return false;
		}
		if (parentBuffName == null || parentBuffName.equals(""))
		{
			return false;
		}
		if (childBuffName == null || childBuffName.equals(""))
		{
			return false;
		}
		if (eventName == null || eventName.equals(""))
		{
			return false;
		}
		if (collectionSlot == null || collectionSlot.equals(""))
		{
			return false;
		}
		if (!hasCompletedCollectionSlotPrereq(player, collectionSlot))
		{
			return false;
		}
		
		if (!buff.hasBuff(player, parentBuffName))
		{
			CustomerServiceLog("holidayEvent", "holiday.rewardEmpireDayPlayer: Player "+ player + " no longer has required parent buff: "+parentBuffName+" so they cannot receive the counter buff: "+childBuffName+".");
			return false;
		}
		
		CustomerServiceLog("holidayEvent", "holiday.rewardEmpireDayPlayer: Player "+ player + " has passed validation.");
		
		String eventType = buff.getStringGroupTwo(parentBuffName);
		if (eventType == null || eventType.equals(""))
		{
			CustomerServiceLog("holidayEvent", "holiday.rewardEmpireDayPlayer: Player "+ player + " tried to apply childbuff: "+childBuffName+" but it had no group2 data so it failed. exiting without giving player credit.");
			return false;
		}
		
		if (!buff.hasBuff(player, childBuffName))
		{
			if (!eventType.startsWith(BUFF_COMBATANT_GROUP) && !eventType.startsWith(BUFF_SF_GROUP))
			{
				CustomerServiceLog("holidayEvent", "holiday.rewardEmpireDayPlayer: Player "+ player + " tried to apply childbuff: "+childBuffName+" but the group2 is invalid. exiting without giving player credit.");
				return false;
			}
			
			if (playerFaction == IMPERIAL_PLAYER)
			{
				if (eventType.startsWith(BUFF_COMBATANT_GROUP))
				{
					if (!getEventTokens(player, TOKEN_IMPERIAL_EMPIRE_DAY, TOKEN_AMOUNT_COMBATANT))
					{
						
					}
					pvpModifyCurrentGcwPoints(player, GCW_AMOUNT_COMBATANT);
					sendSystemMessage(player, SID_GCW_POINTS);
				}
				else if (eventType.startsWith(BUFF_SF_GROUP))
				{
					if (!factions.isDeclared(player))
					{
						sendSystemMessage(player, SID_NOT_SF);
						return false;
					}
					
					if (!getEventTokens(player, TOKEN_IMPERIAL_EMPIRE_DAY, TOKEN_AMOUNT_SF))
					{
						
					}
					pvpModifyCurrentGcwPoints(player, GCW_AMOUNT_SF);
					sendSystemMessage(player, SID_GCW_POINTS);
				}
				buff.applyBuff(player, childBuffName);
			}
			else if (playerFaction == REBEL_PLAYER)
			{
				if (eventType.startsWith(BUFF_COMBATANT_GROUP))
				{
					if (!getEventTokens(player, TOKEN_REBEL_EMPIRE_DAY, TOKEN_AMOUNT_COMBATANT))
					{
						
					}
					pvpModifyCurrentGcwPoints(player, GCW_AMOUNT_COMBATANT);
					sendSystemMessage(player, SID_GCW_POINTS);
				}
				else if (eventType.startsWith(BUFF_SF_GROUP))
				{
					if (!factions.isDeclared(player))
					{
						sendSystemMessage(player, SID_NOT_SF);
						return false;
					}
					
					if (!getEventTokens(player, TOKEN_REBEL_EMPIRE_DAY, TOKEN_AMOUNT_SF))
					{
						
					}
					pvpModifyCurrentGcwPoints(player, GCW_AMOUNT_SF);
					sendSystemMessage(player, SID_GCW_POINTS);
				}
				buff.applyBuff(player, childBuffName);
			}
			play2dNonLoopingSound(player, groundquests.MUSIC_QUEST_COMPLETED);
			CustomerServiceLog("holidayEvent", "holiday.rewardEmpireDayPlayer: Player "+ player + " no longer has required parent buff: "+parentBuffName+" so they cannot receive the counter buff: "+childBuffName+".");
		}
		
		else
		{
			if (eventType.startsWith(BUFF_COMBATANT_GROUP))
			{
				String tokenReward = TOKEN_REBEL_EMPIRE_DAY;
				if (playerFaction == IMPERIAL_PLAYER)
				{
					tokenReward = TOKEN_IMPERIAL_EMPIRE_DAY;
				}
				
				if (!getEventTokens(player, tokenReward, TOKEN_AMOUNT_COMBATANT))
				{
					
				}
				pvpModifyCurrentGcwPoints(player, GCW_AMOUNT_COMBATANT);
				sendSystemMessage(player, SID_GCW_POINTS);
			}
			else if (eventType.startsWith(BUFF_SF_GROUP))
			{
				if (!factions.isDeclared(player))
				{
					
					sendSystemMessage(player, SID_NOT_SF);
					return false;
				}
				
				String tokenReward = TOKEN_REBEL_EMPIRE_DAY;
				if (playerFaction == IMPERIAL_PLAYER)
				{
					tokenReward = TOKEN_IMPERIAL_EMPIRE_DAY;
				}
				
				if (!getEventTokens(player, tokenReward, TOKEN_AMOUNT_SF))
				{
					
				}
				
				pvpModifyCurrentGcwPoints(player, GCW_AMOUNT_SF);
				sendSystemMessage(player, SID_GCW_POINTS);
			}
			
			play2dNonLoopingSound(player, groundquests.MUSIC_QUEST_INCREMENT_COUNTER);
			buff.applyBuff(player, childBuffName);
		}
		return true;
	}
	
	
	public static boolean setEventLockOutTimeStamp(obj_id player, String eventLockOut) throws InterruptedException
	{
		if (!isValidId(player) || !exists(player))
		{
			return false;
		}
		if (eventLockOut == null || eventLockOut.equals(""))
		{
			return false;
		}
		if (hasObjVar(player, eventLockOut))
		{
			return false;
		}
		
		int now = getCalendarTime();
		int secondsUntil = getEmpireDayLockoutDelay();
		int then = now + secondsUntil;
		CustomerServiceLog("holidayEvent", "holiday.setEventLockOutTimeStamp: Setting lockout for player: "+player+" to expire on: "+then);
		setObjVar(player, eventLockOut, then);
		return true;
	}
	
	
	public static boolean checkEventLockOutRemoval(obj_id player, String eventLockTimer, String eventLockFlag) throws InterruptedException
	{
		if (!isValidId(player) || !exists(player))
		{
			return false;
		}
		if (eventLockTimer == null || eventLockTimer.length() <= 0)
		{
			return false;
		}
		if (eventLockFlag == null || eventLockFlag.length() <= 0)
		{
			return false;
		}
		if (!hasObjVar(player, eventLockTimer) && !hasObjVar(player, eventLockFlag))
		{
			return true;
		}
		if (!hasObjVar(player, eventLockTimer) && hasObjVar(player, eventLockFlag))
		{
			CustomerServiceLog("holidayEvent", "holiday.checkEventLockOutRemoval: Player: "+player+" had lockout flag: "+eventLockFlag+" but not timer: "+eventLockTimer+". Removing flag.");
			removeObjVar(player, eventLockFlag);
			return true;
		}
		
		int now = getCalendarTime();
		int then = getIntObjVar(player, eventLockTimer);
		if (then <= 0)
		{
			CustomerServiceLog("holidayEvent", "holiday.checkEventLockOutRemoval: Player: "+player+" had invalid lockout timer: "+eventLockTimer+" for flag: "+eventLockFlag+". POSSIBLE EXPLOIT. Removing both objvars.");
			removeObjVar(player, eventLockTimer);
			removeObjVar(player, eventLockFlag);
			removeObjVar(player, PLAYER_EMPIRE_DAY_SCORE);
			removeObjVar(player, EMPIRE_DAY_BUFF_TRACKER);
			return true;
		}
		
		if (now > then)
		{
			CustomerServiceLog("holidayEvent", "holiday.checkEventLockOutRemoval: Player: "+player+" has lockout timer that has expired: "+eventLockTimer+" for flag: "+eventLockFlag+". Removing both objvars to allow participation in event.");
			removeObjVar(player, eventLockTimer);
			removeObjVar(player, eventLockFlag);
			removeObjVar(player, PLAYER_EMPIRE_DAY_SCORE);
			removeObjVar(player, EMPIRE_DAY_BUFF_TRACKER);
			return true;
		}
		
		return false;
	}
	
	
	public static String getTimeRemainingBeforeLockoutRemoved(obj_id player, String eventLockOut) throws InterruptedException
	{
		if (!isValidId(player) || !exists(player))
		{
			return null;
		}
		if (eventLockOut == null || eventLockOut.equals(""))
		{
			return null;
		}
		if (!hasObjVar(player, eventLockOut))
		{
			return null;
		}
		
		int now = getCalendarTime();
		int then = getIntObjVar(player, eventLockOut);
		if (then <= 0)
		{
			
			return null;
		}
		
		if (now > then)
		{
			return "can start another task now";
		}
		
		int timeLeft = (then - now);
		return "you have "+utils.formatTimeVerbose(timeLeft)+" left before you are available for that task";
	}
	
	
	public static boolean getEventTokens(obj_id player, String tokenStaticItem, int amount) throws InterruptedException
	{
		if (!isValidId(player) || !exists(player))
		{
			return false;
		}
		if (tokenStaticItem == null || tokenStaticItem.equals(""))
		{
			return false;
		}
		if (amount <= 0)
		{
			return false;
		}
		
		obj_id pInv = utils.getInventoryContainer(player);
		if (!isValidId(pInv) || !exists(pInv))
		{
			return false;
		}
		
		if (!isValidId(static_item.createNewItemFunction(tokenStaticItem, pInv, amount)))
		{
			
			return false;
		}
		
		if (!hasObjVar(player, PLAYER_EMPIRE_DAY_SCORE))
		{
			setObjVar(player, PLAYER_EMPIRE_DAY_SCORE, getIntObjVar(player, PLAYER_EMPIRE_DAY_SCORE)+ amount);
			CustomerServiceLog("holidayEvent", "holiday.getEventTokens Player: "+player+" received their very first event points of "+amount+". Now the player has a score of: "+getIntObjVar(player, PLAYER_EMPIRE_DAY_SCORE));
		}
		else
		{
			setObjVar(player, PLAYER_EMPIRE_DAY_SCORE, getIntObjVar(player, PLAYER_EMPIRE_DAY_SCORE)+ amount);
			CustomerServiceLog("holidayEvent", "holiday.getEventTokens Player: "+player+" received "+amount+" points and now has a score of: "+getIntObjVar(player, PLAYER_EMPIRE_DAY_SCORE));
		}
		
		if (amount == 1)
		{
			sendSystemMessage(player, SID_TOKEN_REWARDED);
		}
		else
		{
			sendSystemMessage(player, SID_TOKENS_REWARDED);
		}
		return true;
	}
	
	
	public static int getEmpireDayLockoutDelay() throws InterruptedException
	{
		
		return secondsUntilNextDailyTime(DAILY_TIME, 0, 0);
		
	}
	
	
	public static boolean createEventLeaderBoardUI(obj_id player, String title, String combinedString) throws InterruptedException
	{
		if (!isValidId(player) || !isValidId(player))
		{
			return false;
		}
		if (title == null || title.length() <= 0)
		{
			return false;
		}
		if (combinedString == null || combinedString.length() <= 0)
		{
			return false;
		}
		
		dictionary params = new dictionary();
		params.put("player", player);
		
		closeOldWindow(player);
		
		int pid = createSUIPage("/Script.messageBox", player, player);
		setSUIAssociatedLocation(pid, player);
		setSUIMaxRangeToObject(pid, 8);
		params.put("callingPid", pid);
		sui.setPid(player, pid, LEADERSUI_PID_NAME);
		
		setSUIProperty(pid, "Prompt.lblPrompt", "LocalText", combinedString);
		setSUIProperty(pid, "bg.caption.lblTitle", "Text", title);
		setSUIProperty(pid, "Prompt.lblPrompt", "Editable", "false");
		setSUIProperty(pid, "Prompt.lblPrompt", "GetsInput", "true");
		setSUIProperty(pid, "btnCancel", "Visible", "false");
		setSUIProperty(pid, "btnRevert", "Visible", "false");
		setSUIProperty(pid, "btnOk", sui.PROP_TEXT, "Close");
		
		subscribeToSUIEvent(pid, sui_event_type.SET_onClosedOk, "%button0%", "closeLeaderSui");
		subscribeToSUIEvent(pid, sui_event_type.SET_onClosedCancel, "%button0%", "closeLeaderSui");
		
		showSUIPage(pid);
		flushSUIPage(pid);
		return true;
	}
	
	
	public static void closeOldWindow(obj_id player) throws InterruptedException
	{
		int pid = sui.getPid(player, LEADERSUI_PID_NAME);
		if (pid > -1)
		{
			forceCloseSUIPage(pid);
			sui.removePid(player, LEADERSUI_PID_NAME);
		}
	}
	
	
	public static String getEventHighScores(obj_id planet, String scoreObjVar, boolean faction) throws InterruptedException
	{
		if (!isValidId(planet) || !exists(planet))
		{
			return null;
		}
		if (scoreObjVar == null || scoreObjVar.length() <= 0)
		{
			return null;
		}
		
		String data = "";
		if (!faction)
		{
			for (int i = 1; i <= MAX_NUMBER_OF_PLANET_HIGH_SCORES; i++)
			{
				testAbortScript();
				String name = getStringObjVar(planet, scoreObjVar+".slot_"+ i + holiday.PLANET_VAR_PLAYER_NAME);
				int score = getIntObjVar(planet, scoreObjVar+".slot_"+ i + holiday.PLANET_VAR_PLAYER_SCORE);
				data += "\\^005"+ i + ". \\%015"+ name + " \\%015"+ score + "\n";
			}
		}
		else
		{
			data += sui.colorBlue()+"IMPERIAL\n"+sui.colorWhite();
			for (int i = 1; i <= MAX_NUMBER_OF_PLANET_HIGH_SCORES; i++)
			{
				testAbortScript();
				String name = getStringObjVar(planet, scoreObjVar+".slot_"+ i + holiday.PLANET_VAR_PLAYER_FACTION_IMP + holiday.PLANET_VAR_PLAYER_NAME);
				int score = getIntObjVar(planet, scoreObjVar+".slot_"+ i + holiday.PLANET_VAR_PLAYER_FACTION_IMP + holiday.PLANET_VAR_PLAYER_SCORE);
				data += i + "\t"+ name + "\t"+ score + "\n";
				CustomerServiceLog("holidayEventScore", "getEventHighScores: name: "+name+" score: "+score);
			}
			
			data += sui.colorRed()+"\nREBEL\n"+sui.colorWhite();
			for (int i = 1; i <= MAX_NUMBER_OF_PLANET_HIGH_SCORES; i++)
			{
				testAbortScript();
				String name = getStringObjVar(planet, scoreObjVar+".slot_"+ i + holiday.PLANET_VAR_PLAYER_FACTION_REB + holiday.PLANET_VAR_PLAYER_NAME);
				int score = getIntObjVar(planet, scoreObjVar+".slot_"+ i + holiday.PLANET_VAR_PLAYER_FACTION_REB + holiday.PLANET_VAR_PLAYER_SCORE);
				data += i + "\t"+ name + "\t"+ score + "\n";
			}
			
		}
		return data;
	}
	
	
	public static boolean setEmpireDayLeaderScores(obj_id planet, obj_id player, String scoreObjVar, int newScore, String playerName, int faction) throws InterruptedException
	{
		CustomerServiceLog("holidayEventScore", "setEmpireDayLeaderScores: scoreObjVar: "+scoreObjVar+" score: "+newScore);
		
		if (!isValidId(planet) || !exists(planet))
		{
			return false;
		}
		if (newScore <= 0)
		{
			return false;
		}
		if (scoreObjVar == null || scoreObjVar.length() <= 0)
		{
			return false;
		}
		if (playerName == null || playerName.length() <= 0)
		{
			return false;
		}
		
		CustomerServiceLog("holidayEventScore", "setEmpireDayLeaderScores: passed validation");
		
		String postAppendName = "";
		String postAppendScore = "";
		
		if (faction == holiday.IMPERIAL_PLAYER)
		{
			CustomerServiceLog("holidayEventScore", "setEmpireDayLeaderScores: Is Imp Player");
			
			postAppendName = holiday.PLANET_VAR_PLAYER_FACTION_IMP + holiday.PLANET_VAR_PLAYER_NAME;
			postAppendScore = holiday.PLANET_VAR_PLAYER_FACTION_IMP + holiday.PLANET_VAR_PLAYER_SCORE;
		}
		else if (faction == holiday.REBEL_PLAYER)
		{
			CustomerServiceLog("holidayEventScore", "setEmpireDayLeaderScores: Is Reb Player");
			
			postAppendName = holiday.PLANET_VAR_PLAYER_FACTION_REB + holiday.PLANET_VAR_PLAYER_NAME;
			postAppendScore = holiday.PLANET_VAR_PLAYER_FACTION_REB + holiday.PLANET_VAR_PLAYER_SCORE;
		}
		
		if (postAppendName == null || postAppendName.length() <= 0)
		{
			return false;
		}
		if (postAppendScore == null || postAppendScore.length() <= 0)
		{
			return false;
		}
		
		if (newScore <= getIntObjVar(planet, scoreObjVar + ".slot_5" + postAppendScore))
		{
			CustomerServiceLog("holidayEventScore", "setEmpireDayLeaderScores: Player doesn't have a high enough score");
			CustomerServiceLog("holidayEventScore", "setEmpireDayLeaderScores: Lowest score: "+getIntObjVar(planet, scoreObjVar + ".slot_5"+ postAppendScore));
			return true;
		}
		
		for (int i = 1; i <= MAX_NUMBER_OF_PLANET_HIGH_SCORES; i++)
		{
			testAbortScript();
			String name = getStringObjVar(planet, scoreObjVar+".slot_"+ i + postAppendName);
			if (name == null || name.length() <= 0)
			{
				continue;
			}
			
			if (!playerName.equals(name))
			{
				CustomerServiceLog("holidayEventScore", "setEmpireDayLeaderScores: NO match. Name: "+name+ " and player name: "+playerName );
				CustomerServiceLog("holidayEventScore", "Name length: "+name.length()+ " and player name: "+playerName.length());
				continue;
			}
			
			CustomerServiceLog("holidayEventScore", "setEmpireDayLeaderScores: found name match");
			int score = getIntObjVar(planet, scoreObjVar+".slot_"+ i + postAppendScore);
			if (newScore == score)
			{
				CustomerServiceLog("holidayEventScore", "setEmpireDayLeaderScores: Player already has a score on leader board.");
				return true;
			}
		}
		
		int newHsSlot = -1;
		
		for (int i = MAX_NUMBER_OF_PLANET_HIGH_SCORES - 1; i >= 1; i--)
		{
			testAbortScript();
			int existingScore = getIntObjVar(planet, scoreObjVar + ".slot_"+ i + holiday.PLANET_VAR_PLAYER_FACTION_IMP + holiday.PLANET_VAR_PLAYER_SCORE);
			if (newScore >= existingScore)
			{
				CustomerServiceLog("holidayEventScore", newScore+ " is greater than "+existingScore);
				newHsSlot = i;
			}
			else
			{
				break;
			}
		}
		
		if (newHsSlot < 0)
		{
			return true;
		}
		
		for (int i = MAX_NUMBER_OF_PLANET_HIGH_SCORES; i >= newHsSlot; i--)
		{
			testAbortScript();
			if (i < MAX_NUMBER_OF_PLANET_HIGH_SCORES && i > 0)
			{
				String moveName = getStringObjVar(planet, scoreObjVar + ".slot_"+ i + postAppendName);
				int moveScore = getIntObjVar(planet, scoreObjVar + ".slot_"+ i + postAppendScore);
				setObjVar(planet, scoreObjVar + ".slot_"+ (i+1) + postAppendName, moveName);
				setObjVar(planet, scoreObjVar + ".slot_"+ (i+1) + postAppendScore, moveScore);
			}
		}
		setObjVar(planet, scoreObjVar + ".slot_"+ newHsSlot + postAppendScore, newScore);
		setObjVar(planet, scoreObjVar + ".slot_"+ newHsSlot + postAppendName, playerName);
		
		return true;
	}
}
