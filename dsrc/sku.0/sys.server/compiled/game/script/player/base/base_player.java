package script.player.base;

import script.*;
import script.base_class.*;
import script.combat_engine.*;
import java.util.Arrays;
import java.util.Hashtable;
import java.util.Vector;
import script.base_script;


import script.ai.ai_combat;
import java.lang.Math;
import java.util.Iterator;
import java.util.Set;
import script.library.ai_lib;
import script.library.anims;
import script.library.armor;
import script.library.badge;
import script.library.beast_lib;
import script.library.bio_engineer;
import script.library.bounty_hunter;
import script.library.buff;
import script.library.callable;
import script.library.chat;
import script.library.city;
import script.library.cloninglib;
import script.library.colors;
import script.library.colors_hex;
import script.library.collection;
import script.library.combat;
import script.library.consumable;
import script.library.corpse;
import script.library.cts;
import script.library.dot;
import script.library.dump;
import script.library.expertise;
import script.library.event_perk;
import script.library.factions;
import script.library.features;
import script.library.food;
import script.library.force_rank;
import script.library.force_rank;
import script.library.fs_quests;
import script.library.gm;
import script.library.gmlib;
import script.library.groundquests;
import script.library.group;
import script.library.gcw;
import script.library.guild;
import script.library.healing;
import script.library.hq;
import script.library.hue;
import script.library.instance;
import script.library.jedi;
import script.library.jedi_trials;
import script.library.list;
import script.library.locations;
import script.library.loot;
import script.library.luck;
import script.library.meditation;
import script.library.metrics;
import script.library.money;
import script.library.movement;
import script.library.npe;
import script.library.pclib;
import script.library.performance;
import script.library.permissions;
import script.library.pet_lib;
import script.library.pgc_quests;
import script.library.player_stomach;
import script.library.player_structure;
import script.library.proc;
import script.library.prose;
import script.library.pvp;
import script.library.regions;
import script.library.resource;
import script.library.respec;
import script.library.scout;
import script.library.skill;
import script.library.skill_template;
import script.library.slicing;
import script.library.smuggler;
import script.library.space_combat;
import script.library.space_crafting;
import script.library.space_dungeon;
import script.library.space_dungeon_data;
import script.library.space_flags;
import script.library.space_quest;
import script.library.space_skill;
import script.library.space_transition;
import script.library.space_utils;
import script.library.squad_leader;
import script.library.static_item;
import script.library.stealth;
import script.library.storyteller;
import script.library.structure;
import script.library.sui;
import script.library.temp_schematic;
import script.library.township;
import script.library.trace;
import script.library.travel;
import script.library.trial;
import script.library.utils;
import script.library.vehicle;
import script.library.vendor_lib;
import script.library.veteran_deprecated;
import script.library.weapons;
import script.library.xp;
import script.library.restuss_event;
import script.library.session;
import script.library.incubator;
import script.library.missions;
import script.library.reverse_engineering;

import java.util.Arrays;
import java.util.Vector;
import java.util.Enumeration;


public class base_player extends script.base_script
{
	public base_player()
	{
	}
	public static final int TIME_DEATH = 5;
	
	public static final float RANGE_COUP_DE_GRACE = 3.0f;
	
	public static final String JEDI_CLOAK_TEMPLATE = "object/tangible/wearable/robe/robe_s05.iff";
	
	public static final String MSG_REVIVE_TITLE = "@base_player:revive_title";
	public static final String MSG_REVIVE_MSG = "@base_player:revive_msg";
	
	public static final String MSG_REVIVE_CLOSEST_INSURED = "@base_player:revive_closest_insured";
	public static final String MSG_REVIVE_CLOSEST_UNINSURED = "@base_player:revive_closest_uninsured";
	public static final String MSG_REVIVE_BIND_INSURED = "@base_player:revive_bind_insured";
	public static final String MSG_REVIVE_BIND_UNINSURED = "@base_player:revive_bind_uninsured";
	
	public static final String MSG_REVIVE_CLOSEST = "@base_player:revive_closest";
	public static final String MSG_REVIVE_BIND = "@base_player:revive_bind";
	
	public static final string_id SID_VICTIM_INCAPACITATED = new string_id("base_player", "victim_incapacitated");
	public static final string_id SID_KILLER_TARGET_INCAPACITATED = new string_id("base_player", "killer_target_incapacitated");
	
	public static final string_id PROSE_VICTIM_INCAP = new string_id("base_player", "prose_victim_incap");
	public static final string_id PROSE_TARGET_INCAP = new string_id("base_player", "prose_target_incap");
	
	public static final string_id SID_COUP_DE_GRACE = new string_id("base_player", "death_blow");
	public static final string_id SID_REVIVE = new string_id("base_player", "revive_player");
	public static final string_id SID_RESUSCITATE = new string_id("base_player", "resuscitate_player");
	
	public static final string_id SID_TARGET_OUT_OF_RANGE = new string_id("error_message","target_out_of_range");
	public static final string_id SID_TARGET_NOT_ATTACKABLE = new string_id("error_message","target_not_attackable");
	
	public static final string_id SID_SYNTAX_UNCONSENT = new string_id("error_message","syntax_unconsent");
	
	public static final string_id SID_CONSENT_TO_EMPTY = new string_id("error_message","consent_to_empty");
	public static final string_id SID_CONSENT_FROM_EMPTY = new string_id("error_message","consent_from_empty");
	
	public static final string_id SID_HAVE_CONSENT_TRUE = new string_id("base_player","haveconsent_true");
	public static final string_id SID_HAVE_CONSENT_FALSE = new string_id("base_player","haveconsent_false");
	
	public static final string_id SID_CONSENT_SUCCESSFUL = new string_id("base_player","consent_successful");
	public static final string_id SID_UNCONSENT_SUCCESSFUL = new string_id("base_player","unconsent_successful");
	
	public static final string_id SID_CONSENT_TO_LISTBOX_PROMPT = new string_id("base_player","consent_to_listbox_prompt");
	public static final string_id SID_CONSENT_TO_LISTBOX_TITLE = new string_id("base_player","consent_to_listbox_title");
	
	public static final string_id SID_CONSENT_FROM_LISTBOX_PROMPT = new string_id("base_player","consent_from_listbox_prompt");
	public static final string_id SID_CONSENT_FROM_LISTBOX_TITLE = new string_id("base_player","consent_from_listbox_title");
	
	public static final string_id SID_UNCONSENT_LISTBOX_PROMPT = new string_id("base_player","unconsent_listbox_prompt");
	public static final string_id SID_UNCONSENT_LISTBOX_TITLE = new string_id("base_player","unconsent_listbox_title");
	
	public static final string_id PROSE_GOT_CONSENT = new string_id("base_player","prose_got_consent");
	public static final string_id PROSE_LOST_CONSENT = new string_id("base_player","prose_lost_consent");
	
	public static final string_id SID_CORPSE_EXPIRED = new string_id("base_player","corpse_expired");
	
	public static final string_id SID_PERM_NO_MOVE = new string_id("error_message","perm_no_move");
	
	public static final string_id SID_CMD_OPEN_FAIL = new string_id("error_message","prose_open_fail");
	
	public static final string_id SID_CMD_NO_EFFECT = new string_id("error_message","cmd_no_effect");
	public static final string_id SID_CMD_NO_EFFECT_ON_TARGET = new string_id("error_message","cmd_no_effect_on_target");
	
	public static final string_id SID_NO_UNINSURED_INSURABLES = new string_id("error_message","no_uninsured_insurables");
	public static final string_id SID_BAD_INSURE_REQUEST = new string_id("error_message","bad_insure_request");
	
	public static final string_id SID_INSURE_FAIL = new string_id("error_message","insure_fail");
	public static final string_id PROSE_INSURE_FAIL = new string_id("error_message","prose_insure_fail");
	
	public static final string_id SID_CLONE_FAIL = new string_id("error_message","clone_fail");
	public static final string_id SID_CLONE_FAIL_SELECTION = new string_id("base_player", "clone_fail_no_selection");
	
	public static final string_id SID_TARGET_MUST_BE_PLAYER = new string_id("error_message","target_must_be_player");
	
	public static final string_id SID_ERROR_INVITE_RANGE = new string_id("error_message","error_invite_range");
	public static final string_id PROSE_INVITE_RANGE = new string_id("error_message","prose_invite_range");
	
	public static final string_id SID_INSTANCE_REVIVE = new string_id("base_player", "instance_revive");
	public static final string_id SID_INSTANCE_REVIVE_TITLE = new string_id("base_player", "instance_revive_title");
	
	public static final String HANDLER_PLAYER_REVIVE = "handlePlayerRevive";
	
	public static final String HANDLER_UNCONSENT_MENU = "handleUnconsentMenu";
	
	public static final String HANDLER_CASH_LOOTED = "handleCashLooted";
	
	public static final string_id PROSE_COIN_LOOT = new string_id("base_player","prose_coin_loot");
	public static final string_id PROSE_COIN_LOOT_NO_TARGET = new string_id("base_player","prose_coin_loot_no_target");
	public static final string_id PROSE_COIN_LOOT_FAILED = new string_id("error_message","prose_coin_loot_fail");
	public static final int LOGOUT_TIME = 30;
	public static final int LOGOUT_COUNT_INTERVAL = 5;
	
	public static final string_id SID_SYS_SCENTMASK_NOSKILL = new string_id("skl_use","sys_scentmask_noskill");
	public static final string_id SID_SYS_SCENTMASK_START = new string_id("skl_use","sys_scentmask_start");
	public static final string_id SID_SYS_SCENTMASK_STOP = new string_id("skl_use","sys_scentmask_stop");
	public static final string_id SID_SYS_SCENTMASK_FAIL = new string_id("skl_use","sys_scentmask_fail");
	public static final string_id SID_SYS_SCENTMASK_ALREADY = new string_id("skl_use","sys_scentmask_already");
	public static final string_id SID_SYS_SCENTMASK_DELAY = new string_id("skl_use","sys_scentmask_delay");
	public static final string_id SID_SYS_SCENTMASK_CONCEALED = new string_id("skl_use","sys_scentmask_concealed");
	
	public static final string_id SID_SYS_CONCEAL_NOSKILL = new string_id("skl_use","sys_conceal_noskill");
	public static final string_id SID_SYS_CONCEAL_START = new string_id("skl_use","sys_conceal_start");
	public static final string_id SID_SYS_CONCEAL_STOP = new string_id("skl_use","sys_conceal_stop");
	public static final string_id SID_SYS_CONCEAL_FAIL = new string_id("skl_use","sys_conceal_fail");
	public static final string_id SID_SYS_CONCEAL_CANT = new string_id("skl_use","sys_conceal_cant");
	public static final string_id SID_SYS_CONCEAL_NOTPLAYER = new string_id("skl_use","sys_conceal_notplayer");
	public static final string_id SID_SYS_TARGET_CONCEALED = new string_id("skl_use","sys_target_concealed");
	public static final string_id SID_SYS_CONCEAL_REMOVE = new string_id("skl_use","sys_conceal_remove");
	public static final string_id SID_SYS_CONCEAL_NOKIT = new string_id("skl_use","sys_conceal_nokit");
	public static final string_id SID_SYS_CONCEAL_SCENTMASKED = new string_id("skl_use","sys_conceal_scentmasked");
	public static final string_id SID_SYS_CONCEAL_APPLY = new string_id("skl_use","sys_conceal_apply");
	public static final string_id SID_SYS_CONCEAL_DELAY = new string_id("skl_use","sys_conceal_delay");
	
	public static final string_id SID_SYS_SCAN_NOTHING = new string_id("skl_use","sys_scan_nothing");
	public static final string_id SID_SYS_SCAN_BEGIN = new string_id("skl_use","sys_scan_begin");
	public static final string_id SID_SYS_SCAN_ALREADY = new string_id("skl_use","sys_scan_already");
	public static final string_id SID_SYS_SCAN_INSIDE = new string_id("skl_use","sys_scan_inside");
	public static final string_id SID_SYS_SCAN_MOVED = new string_id("skl_use","sys_scan_moved");
	public static final string_id SID_SYS_SCAN_COMBAT = new string_id("skl_use","sys_scan_combat");
	
	public static final string_id SID_SURVEY_WAYPOINT_NAME = new string_id("skl_use", "survey_waypoint_name");
	public static final string_id SID_SURVEY_WAYPOINT = new string_id("survey", "survey_waypoint");
	public static final string_id SID_SAMPLE_CANCEL = new string_id("survey", "sample_cancel");
	public static final string_id SID_SAMPLE_CANCEL_ATTACK = new string_id("survey", "sample_cancel_attack");
	public static final string_id SID_NODE_WAYPOINT = new string_id("survey", "node_waypoint");
	public static final string_id SID_GAMBLE_NO_ACTION = new string_id("survey", "gamble_no_action");
	
	public static final string_id SID_TRAP_CANCEL = new string_id("trap/trap", "sys_traplay_cancel");
	public static final string_id SID_TRAP_CANCEL_ATTACK = new string_id("trap/trap", "sys_traplay_cancel_attack");
	public static final string_id SID_TRAP_COUNT = new string_id("trap/trap", "sys_traplay_count");
	
	public static final string_id SID_SYS_EJECT_REQUEST = new string_id("error_message", "sys_eject_request");
	public static final string_id SID_SYS_EJECT_FAIL_MOVE = new string_id("error_message", "sys_eject_fail_move");
	public static final string_id SID_SYS_EJECT_FAIL_CONTAINED = new string_id("error_message", "sys_eject_fail_contained");
	public static final string_id SID_SYS_EJECT_FAIL_PROXIMITY = new string_id("error_message", "sys_eject_fail_proximity");
	public static final string_id SID_SYS_EJECT_FAIL_GROUND = new string_id("error_message", "sys_eject_fail_ground");
	public static final string_id SID_SYS_EJECT_SUCCESS = new string_id("error_message", "sys_eject_success");
	public static final string_id SID_SYS_EJECT_DUNGEON = new string_id("error_messaage", "sys_eject_dungeon_pp");
	
	public static final string_id SID_NOT_MILITIA = new string_id("city/city", "not_militia");
	public static final string_id SID_CITY_WARNED = new string_id("city/city", "city_warned");
	public static final string_id SID_CITY_WARN_DONE = new string_id("city/city", "city_warn_done");
	public static final string_id SID_ALREADY_WARNED = new string_id("city/city", "already_warned");
	public static final string_id SID_CITY_BANNED = new string_id("city/city", "city_banned");
	public static final string_id SID_CITY_BAN_DONE = new string_id("city/city", "city_ban_done");
	public static final string_id SID_CITY_PARDONED = new string_id("city/city", "city_pardoned");
	public static final string_id SID_CITY_PARDON_DONE = new string_id("city/city", "city_pardon_done");
	public static final string_id SID_LEAVE_CITY = new string_id("city/city", "city_leave_city");
	public static final string_id SID_ENTER_CITY = new string_id("city/city", "city_enter_city");
	public static final string_id SID_CITY_WARNING = new string_id("city/city", "city_warning");
	public static final string_id SID_CITY_WARNING_TARGET = new string_id("city/city", "city_warning_target");
	public static final string_id SID_NOT_CITIZEN_BAN = new string_id("city/city", "not_citizen_ban");
	public static final string_id SID_NOT_CSR_BAN = new string_id("city/city", "not_csr_ban");
	public static final string_id SID_CSR_BAN_ATTEMPT_MSG = new string_id("city/city", "csr_ban_attempt_msg");
	
	public static final string_id SID_RIGHTS_GRANTED = new string_id("city/city", "rights_granted");
	public static final string_id SID_GRANT_RIGHTS_FAIL = new string_id("city/city", "grant_rights_fail");
	public static final string_id SID_RIGHTS_GRANTED_SELF = new string_id("city/city", "rights_granted_self");
	public static final String CITY_MISSION_TERMINALS = "datatables/city/mission_terminals.iff";
	public static final string_id SID_NO_MORE_MT = new string_id("city/city", "no_more_mt");
	public static final String CITY_SKILL_TRAINERS = "datatables/city/skill_trainers.iff";
	public static final string_id SID_NO_MORE_TRAINERS = new string_id("city/city", "no_more_trainers");
	public static final string_id SID_CITY_NO_MONEY = new string_id("city/city", "action_no_money");
	public static final string_id SID_NO_FACTIONAL = new string_id("city/city", "no_factional");
	public static final string_id SID_RIGHTS_REVOKED = new string_id("city/city", "rights_revoked");
	public static final string_id SID_RIGHTS_REVOKED_OTHER = new string_id("city/city", "rights_revoked_other");
	public static final string_id SID_CIVIC_ONLY = new string_id("city/city", "civic_only");
	
	public static final String[] ZONING_RIGHTS_ARRAY = new String[]
	{
		"@city/city:full_zoning_rights",
		"@city/city:st_zoning_rights",
	};
	public static final string_id SID_ST_RIGHTS_GRANTED = new string_id("city/city", "st_rights_granted");
	public static final string_id SID_ST_GRANT_RIGHTS_FAIL = new string_id("city/city", "st_grant_rights_fail");
	public static final string_id SID_ST_RIGHTS_GRANTED_SELF = new string_id("city/city", "st_rights_granted_self");
	public static final string_id SID_ST_RIGHTS_REVOKED = new string_id("city/city", "st_rights_revoked");
	public static final string_id SID_ST_RIGHTS_REVOKED_OTHER = new string_id("city/city", "st_rights_revoked_other");
	public static final string_id SID_NOT_IN_CITY_LIMITS = new string_id("city/city", "not_in_city_limits");
	
	public static final String NOVICE_MARKSMAN = "combat_marksman_novice";
	public static final String NOVICE_BRAWLER = "combat_brawler_novice";
	public static final String NOVICE_MEDIC = "science_medic_novice";
	public static final String NOVICE_ARTISAN = "crafting_artisan_novice";
	public static final String NOVICE_ENTERTAINER = "social_entertainer_novice";
	public static final String NOVICE_SCOUT = "outdoors_scout_novice";
	
	public static final string_id SID_INVITE_TO_RIDE = new string_id("base_player", "invite_to_ride");
	public static final string_id SID_CANNOT_OFFER_RIDE_TO = new string_id("base_player", "cannot_offer_ride_to");
	public static final string_id SID_OFFER_RIDE_TO = new string_id("base_player", "offer_ride_to");
	public static final string_id SID_UNABLE_TO_ACCEPT_RIDE = new string_id("base_player", "unable_to_accept_ride");
	public static final string_id SID_ACCEPTED_RIDE = new string_id("base_player", "accepted_ride");
	public static final string_id SID_DECLINED_RIDE = new string_id("base_player", "declined_ride");
	public static final string_id SID_NO_ROOM_FOR_PASSENGER = new string_id("base_player", "no_room_for_passenger");
	
	public static final string_id SID_SPICE_DOWNER_DURATION_REDUCED = new string_id("base_player", "spice_downer_duration_reduced");
	public static final string_id SID_SPICE_DOWNER_DURATION_NEGATED = new string_id("base_player", "spice_downer_duration_negated");
	public static final string_id SID_INCAPACITATION_DURATION_REDUCED = new string_id("base_player", "incapacitation_duration_reduced");
	
	public static final String NEW_PLAYER_QUESTS_SCRIPT = "theme_park.new_player.new_player";
	
	public static final String SPACE_CERTIFICATION = "private_cert_space";
	
	public static final string_id SID_CAN_PLACE_ONE_BASE = new string_id("faction_perk", "faction_base_refunded_one");
	public static final String COUPE_DE_GRACE_TARGET = "coupeDeGraceTarget";
	public static final String[] ATTRIBUTES = 
	{
		"HEALTH",
		"CONSTITUTION",
		"ACTION",
		"STAMINA",
		"MIND",
		"WILLPOWER"
	};
	
	public static final int TRANSFER_SHIPS = 1;
	public static final int TRANSFER_DROIDS = 1;
	
	public static final string_id SID_CANT_MILK_MOUNTED = new string_id("skl_use","milk_mounted");
	public static final string_id SID_CANT_MILK = new string_id("skl_use","milk_cant");
	public static final string_id SID_CANT_MILK_COMBAT = new string_id("skl_use", "milk_combat");
	public static final string_id SID_NO_MILK = new string_id("skl_use", "milk_none");
	public static final string_id SID_NO_ALLOW_MILK = new string_id("skl_use", "milk_unmilkeable");
	public static final string_id MILK_TOO_FAR_START = new string_id("skl_use", "milk_too_far_start");
	public static final string_id MILK_NO_TARGET = new string_id("skl_use", "milk_no_target");
	
	public static final String CTS_LOT_COUNT = "cts.lotcount";
	
	public static final String PVP_SKILL_1 = "retaliation_ability";
	public static final String PVP_SKILL_2 = "adrenaline_ability";
	public static final String PVP_SKILL_3 = "unstoppable_ability";
	public static final String PVP_SKILL_4 = "last_man_ability";
	public static final String PVP_SKILL_5 = "aura_buff_self";
	public static final String PVP_SKILL_6 = "airstrike_ability";
	
	public static final string_id COVERCHARGE_DANCER_MESSAGE = new string_id("base_player", "covercharge_dancer_message");
	
	public static final string_id SID_FOUND_NOTHING = new string_id("lair_n", "found_nothing");
	public static final string_id TOO_FAR_FROM_LAIR = new string_id("lair_n", "too_far_from_lair");
	public static final string_id LAIR_NOT_TARGETED = new string_id("lair_n", "lair_not_targeted");
	
	public static final string_id SHAPECHANGE = new string_id("spam", "shapechange_combat");
	
	public static final String[] WAYPOINT_GROUND_PLANETS_EXTERNAL = 
	{
		"tatooine",  
		"naboo",  
		"corellia",  
		"rori",  
		"talus",  
		"yavin4",  
		"endor",  
		"lok",  
		"dantooine",  
		"dathomir",  
		"kachirho",  
		"etyyy",  
		"khowir",  
		"mustafar",  
	};
	
	public static final String[] WAYPOINT_GROUND_PLANETS_INTERNAL = 
	{
		"tatooine",  
		"naboo",  
		"corellia",  
		"rori",  
		"talus",  
		"yavin4",  
		"endor",  
		"lok",  
		"dantooine",  
		"dathomir",  
		"kashyyyk_main",  
		"kashyyyk_hunting",  
		"kashyyyk_dead_forest",  
		"mustafar",  
	};
	
	public static final vector[] WAYPOINT_GROUND_PLANETS_BUILDOUT_COORDS = 
	{
		null,  
		null,  
		null,  
		null,  
		null,  
		null,  
		null,  
		null,  
		null,  
		null,  
		new vector(0.0f, 0.0f, 0.0f),  
		new vector(0.0f, 0.0f, 0.0f),  
		new vector(0.0f, 0.0f, 0.0f),  
		new vector(-2304.0f, 0.0f, 2848.0f), 
	};
	
	public static final String[] WAYPOINT_SPACE_ZONES_EXTERNAL = 
	{
		"space_tatooine",  
		"space_naboo",  
		"space_corellia",  
		"space_deep",  
		"space_kessel",  
		"space_yavin4",  
		"space_endor",  
		"space_lok",  
		"space_dantooine",  
		"space_dathomir",  
		"space_kashyyyk",  
		"space_nova_orion", 
	};
	
	public static final String[] WAYPOINT_SPACE_ZONES_INTERNAL = 
	{
		"space_tatooine",  
		"space_naboo",  
		"space_corellia",  
		"space_heavy1",  
		"space_light1",  
		"space_yavin4",  
		"space_endor",  
		"space_lok",  
		"space_dantooine",  
		"space_dathomir",  
		"space_kashyyyk",  
		"space_nova_orion", 
	};
	
	public static final boolean LOGGING_ON = true;
	public static final String LOGNAME = "junk_log";
	
	
	public int OnCustomizeFinished(obj_id self, obj_id object, String params) throws InterruptedException
	{
		if (utils.hasScriptVar(self, "armor_colorize.tool_oid") || utils.hasScriptVar(self, "structure_colorize.tool_oid"))
		{
			obj_id tool = obj_id.NULL_ID;
			if (utils.hasScriptVar(self, "armor_colorize.tool_oid"))
			{
				tool = utils.getObjIdScriptVar(self, "armor_colorize.tool_oid");
			}
			else if (utils.hasScriptVar(self, "structure_colorize.tool_oid"))
			{
				tool = utils.getObjIdScriptVar(self, "structure_colorize.tool_oid");
			}
			
			if (!isValidId(tool) || !exists(tool))
			{
				return SCRIPT_CONTINUE;
			}
			
			colorizeObject(self, object, tool, params);
			
		}
		if (utils.hasScriptVar(self, vendor_lib.GREETER_PLAYTERCOLOR_SCRVAR))
		{
			vendor_lib.colorizeGreeterFromWidget(self, object, params);
		}
		
		if (utils.hasScriptVar(self, "veteranRewardItemColor.color_setting"))
		{
			utils.colorizeItemFromWidget(self, object, params);
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int OnEnteredCombat(obj_id self) throws InterruptedException
	{
		int shapechange = buff.getBuffOnTargetFromGroup(self, "shapechange");
		
		if (shapechange != 0)
		{
			buff.removeBuff(self, shapechange);
			sendSystemMessage(self, SHAPECHANGE);
		}
		return SCRIPT_CONTINUE;
	}
	
	
	public int OnAttach(obj_id self) throws InterruptedException
	{
		final String[] NEWBIE_PROFICIENCIES =
		{
			"knife_stone","knife_survival","lance_staff_wood_s1","axe_heavy","rifle_cdef","pistol_cdef","carbine_cdef"
		};
		setObjVar(self, pclib.VAR_NEWBIE_DEATH, 1);
		
		skill.setPlayerStatsForLevel(self, 1);
		return SCRIPT_CONTINUE;
	}
	
	
	public int OnSkillAboutToBeRevoked(obj_id self, String skill) throws InterruptedException
	{
		
		if (toLower(skill).startsWith("pilot"))
		{
			
			int godLevel = -1;
			
			if (isGod(self))
			{
				godLevel = getGodLevel(self);
			}
			
			if (!utils.hasScriptVar(self, "revokePilotSkill") && (godLevel < 50))
			{
				space_skill.retireWarning(self, skill);
				return SCRIPT_OVERRIDE;
			}
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int OnCombatLevelChanged(obj_id self, int oldCombatLevel, int newCombatLevel) throws InterruptedException
	{
		if (newCombatLevel <= 1 || oldCombatLevel >= newCombatLevel)
		{
			return SCRIPT_CONTINUE;
		}
		
		if (!utils.hasScriptVar(self, "respec.conceal"))
		{
			skill.doPlayerLeveling(self, oldCombatLevel, newCombatLevel);
		}
		else
		{
			skill.setPlayerStatsForLevel(self, newCombatLevel);
			utils.removeScriptVar(self, "respec.conceal");
		}
		
		if ((utils.isProfession(self, utils.TRADER )) &&(!hasSkill(self, "class_trader")))
		{
			grantSkill(self, "class_trader");
		}
		
		if ((!hasSkill(self, "expertise"))&&(newCombatLevel == 10))
		{
			expertise.autoAllocateExpertiseByLevel(self, true);
		}
		
		recomputeCommandSeries(self);
		
		skill.sendlevelUpStatChangeSystemMessages(self, oldCombatLevel, newCombatLevel);
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int updateGCWStanding(obj_id self, dictionary params) throws InterruptedException
	{
		int intScore = params.getInt("intScore");
		
		gcw.checkAndUpdateGCWStanding(self, intScore);
		return SCRIPT_CONTINUE;
		
	}
	
	
	public int OnSkillRevoked(obj_id self, String strSkill) throws InterruptedException
	{
		
		if (strSkill.equals( "pilot_rebel_navy_novice" ) || strSkill.equals( "pilot_imperial_navy_novice" ) || strSkill.equals( "pilot_neutral_novice" ))
		{
			space_flags.clearSpaceTrack( self );
			space_skill.revokeExperienceForRetire(self, strSkill);
			if (space_quest.hasQuest( self ))
			{
				obj_id datapad = utils.getPlayerDatapad( self );
				if (isIdValid( datapad ))
				{
					obj_id[] dpobjs = getContents( datapad );
					for (int i=0; i<dpobjs.length; i++)
					{
						testAbortScript();
						if (hasObjVar( dpobjs[i], space_quest.QUEST_TYPE ))
						{
							space_quest.setQuestAborted( self, dpobjs[i] );
						}
					}
				}
			}
			
			removeObjVar(self, "space_quest.series");
			
			obj_var_list questList = getObjVarList(self, space_quest.QUEST_STATUS);
			LOG("revoke","QUESTLIST: QuestList "+questList);
			if (questList != null)
			{
				
				int spaceQuests = questList.getNumItems();
				
				for (int i = 0; i < spaceQuests; ++i)
				{
					testAbortScript();
					obj_var spaceFields = questList.getObjVar(i);
					String questType = spaceFields.getName();
					
					obj_var_list typeList = questList.getObjVarList(questType);
					int questCount = typeList.getNumItems();
					
					for (int j = 0; j < questCount; j++)
					{
						testAbortScript();
						obj_var quest = typeList.getObjVar(j);
						String questName = quest.getName();
						
						String spaceTable = "/datatables/spacequest/"+ questType + "/"+ questName + ".iff";
						String questSeries = dataTableGetString( spaceTable, 0, "questSeries");
						if (questSeries == null)
						{
							LOG("revoke_space_quest","QUESTSERIES IS NULL IN DATATABLE: "+ spaceTable);
							continue;
						}
						
						if (!questSeries.equals("") && questSeries.equals("pilot_profession"))
						{
							space_quest.clearQuestFlags( self, questType, questName );
							
						}
					}
				}
				
			}
			
		}
		
		utils.unequipAndNotifyUncerted(self);
		if (strSkill.equals("outdoors_ranger_movement_03"))
		{
			if (hasSchematic(self, "object/draft_schematic/scout/item_camokit_kashyyyk.iff"))
			{
				revokeSchematic(self, "object/draft_schematic/scout/item_camokit_kashyyyk.iff");
			}
		}
		
		if (strSkill.startsWith("expertise_"))
		{
			expertise.cacheExpertiseProcReacList(self);
			armor.recalculateArmorForPlayer(self);
			obj_id tempWeapon = getObjectInSlot(self, "hold_r");
			
			if (isIdValid(tempWeapon))
			{
				weapons.adjustWeaponRangeForExpertise(self, tempWeapon, true);
			}
			
			skill.recalcPlayerPools(self, false);
		}
		
		trial.bumpSession(self, "displayDefensiveMods");
		messageTo(self, "setDisplayOnlyDefensiveMods", trial.getSessionDict(self, "displayDefensiveMods") , 5, false);
		
		recomputeCommandSeries(self);
		
		beast_lib.verifyAndUpdateCalledBeastStats(self);
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int OnInitialize(obj_id self) throws InterruptedException
	{
		
		if ((utils.isProfession(self, utils.TRADER )) &&(!hasSkill(self, "class_trader")))
		{
			grantSkill(self, "class_trader");
		}
		
		if (!hasSkill(self, "expertise"))
		{
			expertise.autoAllocateExpertiseByLevel(self, false);
		}
		
		city.initCitizen( self );
		
		respecNewEntertainerSkills(self);
		respecNewCrafterSkills(self);
		
		boolean setting = utils.checkConfigFlag("TestCenterPlayer", "ApplyTCMark");
		if (setting)
		{
			setObjVar(self, "tcTester", 1);
		}
		
		if (getForcePower(self) > 0)
		{
			
			if (getJediState(self) == JEDI_STATE_NONE)
			{
				setJediState(self, JEDI_STATE_FORCE_SENSITIVE);
			}
		}
		
		if (hasSkill(self, "jedi_padawan_novice"))
		{
			setSkillTemplate(self, "a");
			attachScript(self, "player.player_jedi_conversion");
		}
		
		if (!isJedi(self))
		{
			if (hasObjVar(self, "jedi.postponeGrant"))
			{
				if (!jedi.postponeGrantJedi())
				{
					
					LOG("jedi", "MAKING "+ self + " A JEDI FROM ON_INITIALIZE");
					messageTo(self, "makeJedi", null, 0.1f, false);
				}
			}
			else if (hasObjVar(self, "jedi.timeStamp"))
			{
				
				LOG("jedi", "MAKING "+ self + " A JEDI FROM ON_INITIALIZE");
				messageTo(self, "makeJedi", null, 0.1f, false);
			}
		}
		
		skill.fixTerrainNegotiationMods(self);
		
		if (isIdValid(getHouseId(self)))
		{
			skill.grantAllPoliticianSkills(self);
		}
		else
		{
			int currentCity = getCitizenOfCityId(self);
			obj_id currentCityMayor = cityGetLeader(currentCity);
			if (self == currentCityMayor)
			{
				skill.grantAllPoliticianSkills(self);
			}
		}
		
		int vehicleBuff = buff.getBuffOnTargetFromGroup(self, "vehicle");
		if (vehicleBuff != 0)
		{
			buff.removeBuff(self, vehicleBuff);
		}
		
		if (buff.hasBuff(self, "forceRun"))
		{
			buff.removeBuff(self, "forceRun");
		}
		if (buff.hasBuff(self, "forceRun_1"))
		{
			buff.removeBuff(self, "forceRun_1");
		}
		if (buff.hasBuff(self, "forceRun_2"))
		{
			buff.removeBuff(self, "forceRun_2");
		}
		
		jedi.jediSaberPearlRestore(self);
		
		if (!hasSkill(self, "social_language_wookiee_comprehend"))
		{
			grantSkill(self, "social_language_wookiee_comprehend");
		}
		
		if (!group.isGrouped(self))
		{
			int[] buffList = buff.getGroupBuffEffects(self);
			if (buffList != null && buffList.length > 0)
			{
				buff.removeGroupBuffEffect(self, buffList);
			}
		}
		
		if (features.isSpaceEdition(self))
		{
			grantCommand(self, SPACE_CERTIFICATION);
		}
		else
		{
			revokeCommand(self, SPACE_CERTIFICATION);
		}
		
		removeObjVar( self, pclib.VAR_SAFE_LOGOUT );
		recalculateLevel( self );
		
		removeObjVar(self, "clickRespec");
		removeObjVar(self, "npcRespec");
		
		detachScript( self, group.SCRIPT_GROUP_MEMBER );
		
		if (!hasScript(self, dot.SCRIPT_PLAYER_DOT))
		{
			setState( self, STATE_BLEEDING, false );
			setState( self, STATE_POISONED, false );
			setState( self, STATE_DISEASED, false );
			setState( self, STATE_ON_FIRE, false );
		}
		
		if (buff.hasBuff(self, "healOverTime"))
		{
			buff.removeBuff(self, "healOverTime");
		}
		
		chat.resetTempAnimationMood( self );
		
		if (hasObjVar(self, cloninglib.VAR_BIND_FACILITY_TEMPLATE))
		{
			obj_id facilityId = getObjIdObjVar(self, cloninglib.VAR_BIND_FACILITY);
			dictionary data = new dictionary();
			data.put("requester", self);
			messageTo(facilityId, "requestCloningData", data, 0, false);
		}
		
		if (hasObjVar(self, "coa3"))
		{
			removeObjVar(self, "coa3");
		}
		
		utils.validatePlayerHairStyle(self);
		
		badge.grantThemeParkBadges(self);
		
		if (hasSchematic(self, getObjectTemplateCrc("object/draft_schematic/weapon/razor_knuckler.iff")))
		{
			temp_schematic.revoke(self, "object/draft_schematic/weapon/razor_knuckler.iff");
		}
		
		if (bounty_hunter.hasMaxBountyMissionsOnTarget(self))
		{
			messageTo(self, "handleBountyMissionTimeout", null, 30.0f, true);
		}
		
		int totalMoney = getTotalMoney(self);
		utils.setScriptVar(self, "profit", totalMoney);
		
		String balanceThreshold = getConfigSetting("GameServer", "wealthLoggingThreshold");
		if (balanceThreshold == null || balanceThreshold.equals(""))
		{
			
			balanceThreshold = "10000000";
		}
		if (utils.stringToInt(balanceThreshold) > -1)
		{
			
			if (totalMoney > utils.stringToInt(balanceThreshold))
			{
				
				CustomerServiceLog("Wealth", "Extraordinary Wealth: "+ getName(self) + " ("+ self + ") logged in with "+ totalMoney + " credits");
			}
		}
		
		utils.unequipAndNotifyUncerted(self);
		utils.checkInventoryForSnowflakeItemSwaps(self);
		if (hasObjVar(self, "item_reimbursement_list"))
		{
			String[] strReimbursementList = getStringArrayObjVar(self, "item_reimbursement_list");
			removeObjVar(self,"item_reimbursement_list");
			for (int intI = 0; intI < strReimbursementList.length; intI++)
			{
				testAbortScript();
				obj_id objTest = createObjectInInventoryAllowOverload(strReimbursementList[intI], self);
				if (isIdValid(objTest))
				{
					
					if (armor.isArmorComponent(objTest))
					{
						if (!isGameObjectTypeOf(objTest, GOT_armor_foot) && !isGameObjectTypeOf(objTest, GOT_armor_hand))
						{
							
							final String[] ARMOR_SET =
							{
								
								"object/tangible/wearables/armor/clone_trooper/armor_clone_trooper_imperial_s01_leggings.iff",
								"object/tangible/wearables/armor/clone_trooper/armor_clone_trooper_imperial_s01_helmet.iff",
								"object/tangible/wearables/armor/clone_trooper/armor_clone_trooper_imperial_s01_gloves.iff",
								"object/tangible/wearables/armor/clone_trooper/armor_clone_trooper_imperial_s01_chest_plate.iff",
								"object/tangible/wearables/armor/clone_trooper/armor_clone_trooper_imperial_s01_bracer_r.iff",
								"object/tangible/wearables/armor/clone_trooper/armor_clone_trooper_imperial_s01_bracer_l.iff",
								"object/tangible/wearables/armor/clone_trooper/armor_clone_trooper_imperial_s01_boots.iff",
								"object/tangible/wearables/armor/clone_trooper/armor_clone_trooper_imperial_s01_bicep_r.iff",
								"object/tangible/wearables/armor/clone_trooper/armor_clone_trooper_imperial_s01_bicep_l.iff",
								"object/tangible/wearables/armor/clone_trooper/armor_clone_trooper_imperial_s01_belt.iff",
								
								"object/tangible/wearables/armor/clone_trooper/armor_clone_trooper_rebel_s01_leggings.iff",
								"object/tangible/wearables/armor/clone_trooper/armor_clone_trooper_rebel_s01_helmet.iff",
								"object/tangible/wearables/armor/clone_trooper/armor_clone_trooper_rebel_s01_gloves.iff",
								"object/tangible/wearables/armor/clone_trooper/armor_clone_trooper_rebel_s01_chest_plate.iff",
								"object/tangible/wearables/armor/clone_trooper/armor_clone_trooper_rebel_s01_bracer_r.iff",
								"object/tangible/wearables/armor/clone_trooper/armor_clone_trooper_rebel_s01_bracer_l.iff",
								"object/tangible/wearables/armor/clone_trooper/armor_clone_trooper_rebel_s01_boots.iff",
								"object/tangible/wearables/armor/clone_trooper/armor_clone_trooper_rebel_s01_bicep_r.iff",
								"object/tangible/wearables/armor/clone_trooper/armor_clone_trooper_rebel_s01_bicep_l.iff",
								"object/tangible/wearables/armor/clone_trooper/armor_clone_trooper_rebel_s01_belt.iff",
								
								"object/tangible/wearables/armor/clone_trooper/armor_clone_trooper_neutral_s01_leggings.iff",
								"object/tangible/wearables/armor/clone_trooper/armor_clone_trooper_neutral_s01_helmet.iff",
								"object/tangible/wearables/armor/clone_trooper/armor_clone_trooper_neutral_s01_gloves.iff",
								"object/tangible/wearables/armor/clone_trooper/armor_clone_trooper_neutral_s01_chest_plate.iff",
								"object/tangible/wearables/armor/clone_trooper/armor_clone_trooper_neutral_s01_bracer_r.iff",
								"object/tangible/wearables/armor/clone_trooper/armor_clone_trooper_neutral_s01_bracer_l.iff",
								"object/tangible/wearables/armor/clone_trooper/armor_clone_trooper_neutral_s01_boots.iff",
								"object/tangible/wearables/armor/clone_trooper/armor_clone_trooper_neutral_s01_bicep_r.iff",
								"object/tangible/wearables/armor/clone_trooper/armor_clone_trooper_neutral_s01_bicep_l.iff",
								"object/tangible/wearables/armor/clone_trooper/armor_clone_trooper_neutral_s01_belt.iff"
							};
							
							for (int intJ = 0; intJ < ARMOR_SET.length; intJ++)
							{
								testAbortScript();
								if (strReimbursementList[intI].equals(ARMOR_SET[intJ]))
								{
									armor.setArmorDataPercent(objTest, 2, 1, 0.94f, 0.95f);
									break;
								}
							}
						}
					}
					
					CustomerServiceLog("item_reimbursement", "Created item of type "+strReimbursementList[intI]+" and Id of "+objTest+ " in the inventory of %TU", self);
				}
				else
				{
					CustomerServiceLog("item_reimbursement", "Failed to reimburse item of type "+strReimbursementList[intI]+" in the inventory of %TU", self);
				}
			}
		}
		
		int fatigue = getShockWound(self);
		if (fatigue > 0)
		{
			setShockWound(self, 0);
		}
		
		obj_var_list buffList = getObjVarList(self, "clickItem");
		if (buffList != null)
		{
			int count = buffList.getNumItems();
			int curTime = getGameTime();
			for (int i = 0; i < count; i++)
			{
				testAbortScript();
				obj_var item = buffList.getObjVar(i);
				int reuseTime = item.getIntData();
				
				if (reuseTime < curTime)
				{
					String cooldownGroup = item.getName();
					removeObjVar(self, "clickItem."+cooldownGroup);
				}
			}
		}
		
		if (!hasScript( self, "player.live_conversions" ))
		{
			attachScript( self, "player.live_conversions");
		}
		
		if (!hasScript(self, "systems.skills.stealth.player_stealth"))
		{
			attachScript( self, "systems.skills.stealth.player_stealth");
		}
		
		skill_template.validateWorkingSkill(self);
		skill.setPlayerStatsForLevel(self, getLevel(self));
		
		LOG("base_player - OnInitialize", "calling armor.recalculateArmorForPlayer(self); with a 'self' of: "+self);
		armor.recalculateArmorForPlayer(self);
		
		obj_id objWeapon = getDefaultWeapon(self);
		String strTemplate = getTemplateName(objWeapon);
		if (isIdValid(objWeapon) && strTemplate.equals("object/weapon/melee/unarmed/unarmed_default_player.iff"))
		{
			if (!hasScript(objWeapon, "systems.combat.combat_weapon"))
			{
				attachScript(objWeapon, "systems.combat.combat_weapon");
				messageTo(objWeapon, "weaponConversion", null, 2, false);
			}
			
			float fltWeaponSpeed = getWeaponAttackSpeed(objWeapon);
			if (fltWeaponSpeed != .50f)
			{
				setWeaponAttackSpeed(objWeapon, .50f);
			}
		}
		
		expertise.cacheExpertiseProcReacList(self);
		proc.buildCurrentProcList(self);
		proc.buildCurrentReacList(self);
		
		givePublishGift(self);
		
		sendSmugglerSystemBootstrap(self);
		
		recomputeCommandSeries(self);
		
		trial.bumpSession(self, "displayDefensiveMods");
		messageTo(self, "setDisplayOnlyDefensiveMods", trial.getSessionDict(self, "displayDefensiveMods") , 5, false);
		
		missions.initializeDailyOnLogin(self);
		
		int lastDecayTime = getIntObjVar(self, "bounty.decayTime");
		int currentTime = getCalendarTime();
		
		if (lastDecayTime == 0)
		{
			setObjVar(self, "bounty.decayTime", currentTime);
		}
		else
		{
			int bounty = getIntObjVar(self, "bounty.amount");
			
			if (currentTime - lastDecayTime > 604800)
			{
				setObjVar(self, "bounty.decayTime", currentTime);
				
				if (bounty < 10000)
				{
					removeObjVar(self, "bounty.amount");
				}
			}
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int handleRetireWarning(obj_id self, dictionary params) throws InterruptedException
	{
		
		int btn = sui.getIntButtonPressed(params);
		
		if (btn == sui.BP_CANCEL)
		{
			
			location loc = utils.getLocationScriptVar(self, "revokePilotSkillWarningLoc");
			String wp_name = utils.getStringScriptVar(self, "revokePilotSkillWarningWpName");
			
			if (loc == null)
			{
				return SCRIPT_CONTINUE;
			}
			
			obj_id wp = createWaypointInDatapad(self, loc);
			setWaypointName(wp, wp_name);
			setWaypointColor(wp, "blue");
			setWaypointActive(wp, true);
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int OnPvpRankingChanged(obj_id self, int oldRank, int newRank) throws InterruptedException
	{
		String faction = "";
		boolean isImperial = false;
		
		if (factions.isImperial(self))
		{
			faction = "pvp_imperial_";
			isImperial = true;
		}
		
		if (factions.isRebel(self))
		{
			faction = "pvp_rebel_";
		}
		
		if (faction.equals(""))
		{
			factions.removeAllPvpSkills (self);
			CustomerServiceLog("pvp_rank_error", "Player %TU somehow changed PvP rank without being Rebel or Imperial, removing all PvP skills.", self);
			return SCRIPT_CONTINUE;
		}
		
		if (oldRank <= 6 && newRank > 6)
		{
			skill.grantSkill(self, faction + PVP_SKILL_1);
			
			if (isImperial)
			{
				badge.grantBadge(self, "pvp_imperial_lieutenant");
			}
			else
			{
				badge.grantBadge(self, "pvp_rebel_lieutenant");
			}
		}
		
		if (oldRank <= 7 && newRank > 7)
		{
			skill.grantSkill(self, faction + PVP_SKILL_2);
			
			if (isImperial)
			{
				badge.grantBadge(self, "pvp_imperial_captain");
			}
			else
			{
				badge.grantBadge(self, "pvp_rebel_captain");
			}
		}
		
		if (oldRank <= 8 && newRank > 8)
		{
			skill.grantSkill(self, faction + PVP_SKILL_3);
			
			if (isImperial)
			{
				badge.grantBadge(self, "pvp_imperial_major");
			}
			else
			{
				badge.grantBadge(self, "pvp_rebel_major");
			}
		}
		
		if (oldRank <= 9 && newRank > 9)
		{
			skill.grantSkill(self, faction + PVP_SKILL_4);
			
			if (isImperial)
			{
				badge.grantBadge(self, "pvp_imperial_lt_colonel");
			}
			else
			{
				badge.grantBadge(self, "pvp_rebel_commander");
			}
		}
		
		if (oldRank <= 10 && newRank > 10)
		{
			skill.grantSkill(self, faction + PVP_SKILL_5);
			
			if (isImperial)
			{
				badge.grantBadge(self, "pvp_imperial_colonel");
			}
			else
			{
				badge.grantBadge(self, "pvp_rebel_colonel");
			}
		}
		
		if (oldRank <= 11 && newRank > 11)
		{
			skill.grantSkill(self, faction + PVP_SKILL_6);
			
			if (isImperial)
			{
				badge.grantBadge(self, "pvp_imperial_general");
			}
			else
			{
				badge.grantBadge(self, "pvp_rebel_general");
			}
		}
		
		if (oldRank == 12 && newRank <= 11)
		{
			skill.revokeSkill(self, faction + PVP_SKILL_6);
			
			if (newRank == 10)
			{
				skill.revokeSkill(self, faction + PVP_SKILL_5);
			}
			if (newRank == 9)
			{
				skill.revokeSkill(self, faction + PVP_SKILL_4);
				skill.revokeSkill(self, faction + PVP_SKILL_5);
			}
			if (newRank == 8)
			{
				skill.revokeSkill(self, faction + PVP_SKILL_3);
				skill.revokeSkill(self, faction + PVP_SKILL_4);
				skill.revokeSkill(self, faction + PVP_SKILL_5);
			}
			if (newRank == 7)
			{
				skill.revokeSkill(self, faction + PVP_SKILL_2);
				skill.revokeSkill(self, faction + PVP_SKILL_3);
				skill.revokeSkill(self, faction + PVP_SKILL_4);
				skill.revokeSkill(self, faction + PVP_SKILL_5);
			}
			if (newRank < 7)
			{
				skill.revokeSkill(self, faction + PVP_SKILL_1);
				skill.revokeSkill(self, faction + PVP_SKILL_2);
				skill.revokeSkill(self, faction + PVP_SKILL_3);
				skill.revokeSkill(self, faction + PVP_SKILL_4);
				skill.revokeSkill(self, faction + PVP_SKILL_5);
			}
		}
		
		if (oldRank == 11 && newRank <= 10)
		{
			skill.revokeSkill(self, faction + PVP_SKILL_5);
			
			if (newRank == 9)
			{
				skill.revokeSkill(self, faction + PVP_SKILL_4);
			}
			if (newRank == 8)
			{
				skill.revokeSkill(self, faction + PVP_SKILL_3);
				skill.revokeSkill(self, faction + PVP_SKILL_4);
			}
			if (newRank == 7)
			{
				skill.revokeSkill(self, faction + PVP_SKILL_2);
				skill.revokeSkill(self, faction + PVP_SKILL_3);
				skill.revokeSkill(self, faction + PVP_SKILL_4);
			}
			if (newRank < 7)
			{
				skill.revokeSkill(self, faction + PVP_SKILL_1);
				skill.revokeSkill(self, faction + PVP_SKILL_2);
				skill.revokeSkill(self, faction + PVP_SKILL_3);
				skill.revokeSkill(self, faction + PVP_SKILL_4);
			}
		}
		
		if (oldRank == 10 && newRank <= 9)
		{
			skill.revokeSkill(self, faction + PVP_SKILL_4);
			
			if (newRank == 8)
			{
				skill.revokeSkill(self, faction + PVP_SKILL_3);
			}
			if (newRank == 7)
			{
				skill.revokeSkill(self, faction + PVP_SKILL_2);
				skill.revokeSkill(self, faction + PVP_SKILL_3);
			}
			if (newRank < 7)
			{
				skill.revokeSkill(self, faction + PVP_SKILL_1);
				skill.revokeSkill(self, faction + PVP_SKILL_2);
				skill.revokeSkill(self, faction + PVP_SKILL_3);
			}
		}
		
		if (oldRank == 9 && newRank <= 8)
		{
			skill.revokeSkill(self, faction + PVP_SKILL_3);
			
			if (newRank == 7)
			{
				skill.revokeSkill(self, faction + PVP_SKILL_2);
			}
			if (newRank < 7)
			{
				skill.revokeSkill(self, faction + PVP_SKILL_1);
				skill.revokeSkill(self, faction + PVP_SKILL_2);
			}
		}
		
		if (oldRank == 8 && newRank <= 7)
		{
			skill.revokeSkill(self, faction + PVP_SKILL_2);
			
			if (newRank < 7)
			{
				skill.revokeSkill(self, faction + PVP_SKILL_1);
			}
		}
		
		if (oldRank == 7 && newRank < 7)
		{
			skill.revokeSkill(self, faction + PVP_SKILL_1);
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int OnEnvironmentalDeath(obj_id self) throws InterruptedException
	{
		
		if (!hasObjVar(self, pclib.VAR_BEEN_COUPDEGRACED))
		{
			setObjVar(self, pclib.VAR_DEATHBLOW_KILLER, self);
			setObjVar(self, pclib.VAR_DEATHBLOW_STAMP, getGameTime());
		}
		
		messageTo(self, pclib.HANDLER_PLAYER_DEATH, null, 0, false);
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int OnAboutToBeIncapacitated(obj_id self, obj_id killer) throws InterruptedException
	{
		if (performCriticalHeal(self))
		{
			return SCRIPT_OVERRIDE;
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int OnIncapacitated(obj_id self, obj_id killer) throws InterruptedException
	{
		
		utils.setScriptVar(self, "lastKiller", killer);
		
		if (hasScript(killer, "ai.smuggler_spawn_enemy"))
		{
			int exceptionSmuggler = 0;
			if (hasObjVar(self, "quest.owner"))
			{
				obj_id smuggler = (getObjIdObjVar(self, "quest.owner"));
				if (self != smuggler)
				{
					exceptionSmuggler = 1;
				}
			}
			
			if (exceptionSmuggler == 0)
			{
				
				groundquests.sendSignal(self, "smugglerEnemyIncap");
				messageTo(killer, "smugglerKilled", null, 1f, false);
			}
		}
		
		if (callable.hasAnyCallable(self))
		{
			obj_id objCallable = callable.getCallable(self, callable.CALLABLE_TYPE_RIDEABLE);
			if (exists(objCallable) && isIdValid(objCallable))
			{
				callable.storeCallable(self, objCallable);
			}
		}
		
		int vehicleBuff = buff.getBuffOnTargetFromGroup(self, "vehicle");
		if (vehicleBuff != 0)
		{
			buff.removeBuff(self, vehicleBuff);
		}
		
		if (buff.hasBuff(self, "incapWeaken"))
		{
			pclib.killPlayer(self, killer, true);
			return SCRIPT_CONTINUE;
		}
		else
		{
			buff.applyBuff(self, "incapWeaken");
		}
		
		int recapacitateTimer = 10;
		
		float recapacitateModified = ((float)getEnhancedSkillStatisticModifierUncapped(self, "resistance_incapacitation"));
		if (recapacitateModified > 0)
		{
			recapacitateTimer -= recapacitateModified;
			if (recapacitateTimer <= 2)
			{
				recapacitateTimer = 2;
			}
			sendSystemMessage(self, SID_INCAPACITATION_DURATION_REDUCED);
		}
		
		CustomerServiceLog("player_death", "Player %TU is incapacitated by %TT", self, killer);
		
		if (!isIdValid(killer))
		{
			killer = self;
		}
		
		if (killer == self)
		{
			sendSystemMessage(self, SID_VICTIM_INCAPACITATED);
		}
		else
		{
			prose_package ppToVictim = prose.getPackage(PROSE_VICTIM_INCAP, killer);
			sendSystemMessageProse(self, ppToVictim);
		}
		
		if (killer != self)
		{
			prose_package ppToKiller = prose.getPackage(PROSE_TARGET_INCAP, self);
			sendSystemMessageProse(killer, ppToKiller);
		}
		
		utils.removeScriptVar(self, COUPE_DE_GRACE_TARGET);
		
		utils.setScriptVar(self, "incap.timeStamp", (getGameTime() + recapacitateTimer));
		setCount(self, recapacitateTimer);
		
		setHealth( self, 0);
		combat.doCombatDebuffs(self);
		messageTo(self, "recapacitationDelay", null, recapacitateTimer, false);
		
		clearAllAiEnemyFlags( self );
		dot.removeAllDots(self);
		
		if (stealth.hasInvisibleBuff(self))
		{
			stealth.checkForAndMakeVisible(self);
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public void clearAllAiEnemyFlags(obj_id self) throws InterruptedException
	{
		obj_id allEnemies[] = pvpGetPersonalEnemyIds(self);
		if (allEnemies == null)
		{
			return;
		}
		
		for (int i = 0; i < allEnemies.length; ++i)
		{
			testAbortScript();
			if (isIdValid(allEnemies[i]) && allEnemies[i].isLoaded() && (!isPlayer(allEnemies[i])))
			{
				pvpRemoveTempEnemyFlags(self, allEnemies[i]);
				pvpRemoveTempEnemyFlags(allEnemies[i], self);
			}
		}
	}
	
	
	public int recapacitationDelay(obj_id self, dictionary params) throws InterruptedException
	{
		if (getPosture(self) != POSTURE_INCAPACITATED)
		{
			return SCRIPT_CONTINUE;
		}
		
		if (!combat.clearCombatDebuffs(self))
		{
			
			messageTo(self, "recapacitationDelay", null, 1, false);
			return SCRIPT_CONTINUE;
		}
		
		setHealth( self, 1);
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int requestMissionLocations(obj_id self, dictionary params) throws InterruptedException
	{
		
		obj_id groupObject = getGroupObject(self);
		if ((isIdValid(groupObject)) && (params != null) && (params.containsKey("requestMissionLocationsNumber")))
		{
			
			obj_id[] missions = getMissionObjects(self);
			Vector missionLocation = new Vector();
			missionLocation.setSize(0);
			
			if ((missions != null) && (missions.length > 0))
			{
				
				int missionIndex = 0;
				
				for (missionIndex = 0; missionIndex < missions.length; ++missionIndex)
				{
					testAbortScript();
					if (isIdValid(missions[missionIndex]))
					{
						
						if (getMissionStatus(missions[missionIndex]) > 0)
						{
							
							utils.addElement(missionLocation, getMissionStartLocation(missions[missionIndex]));
						}
					}
				}
			}
			
			dictionary response = new dictionary();
			response.put("requestMissionLocationsNumber", params.getInt("requestMissionLocationsNumber"));
			response.put("sender", self);
			response.put("senderLocation", getLocation(self));
			
			if (missionLocation.size() > 0)
			{
				response.put("missionLocation", missionLocation);
			}
			
			messageTo(groupObject, "missionLocationResponse", response, 0, false);
		}
		
		group.destroyGroupWaypoint(self);
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int OnAddedToGroup(obj_id self, obj_id groupId) throws InterruptedException
	{
		if (!hasScript(self, group.SCRIPT_GROUP_MEMBER))
		{
			attachScript(self, group.SCRIPT_GROUP_MEMBER);
		}
		
		if (beast_lib.getBeastOnPlayer(self) != null)
		{
			obj_id beast = beast_lib.getBeastOnPlayer(self);
			obj_id beastBCD = beast_lib.getBeastBCD(beast);
			messageTo (beastBCD, "ownerGrouped", null, 1, false);
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int OnRemovedFromGroup(obj_id self, obj_id groupId) throws InterruptedException
	{
		if (utils.hasScriptVar(self, "currentHolo"))
		{
			performance.holographicCleanup(self);
		}
		
		squad_leader.clearRallyPoint(self);
		
		detachScript(self, group.SCRIPT_GROUP_MEMBER);
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int OnGroupLeaderChanged(obj_id self, obj_id groupId, obj_id newLeader, obj_id oldLeader) throws InterruptedException
	{
		return SCRIPT_CONTINUE;
	}
	
	
	public int OnGroupDisbanded(obj_id self, obj_id group) throws InterruptedException
	{
		squad_leader.clearRallyPoint(self);
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int OnObjectMenuRequest(obj_id self, obj_id player, menu_info mi) throws InterruptedException
	{
		removeObjVar( self, "noTrade");
		
		menu_info_data mid = mi.getMenuItemByType(menu_info_types.COMBAT_DEATH_BLOW);
		if (mid == null)
		{
			int myPosture = getPosture(self);
			if (myPosture == POSTURE_INCAPACITATED)
			{
				if (pvpCanAttack(player, self))
				{
					mi.addRootMenu (menu_info_types.COMBAT_DEATH_BLOW, new string_id("",""));
				}
			}
			else if (myPosture == POSTURE_DEAD)
			{
				if ((group.inSameGroup(self, player) || pclib.hasConsent(player, self)))
				{
					if (hasCommand(player, "revivePlayer"))
					{
						int reviveRoot = mi.addRootMenu(menu_info_types.SERVER_MENU1, SID_REVIVE);
						
						if (hasCommand(player, "resuscitatePlayer"))
						{
							mi.addSubMenu(reviveRoot, menu_info_types.SERVER_MENU9, SID_RESUSCITATE);
						}
					}
					else if (hasCommand(player, "resuscitatePlayer"))
					{
						mi.addRootMenu(menu_info_types.SERVER_MENU9, SID_RESUSCITATE);
					}
				}
			}
		}
		else
		{
			mid.setServerNotify(true);
		}
		
		{
			
			obj_id listenTarget = getPerformanceListenTarget(player);
			if (isIdValid(listenTarget) && group.inSameGroup(listenTarget, self))
			{
				mi.addRootMenu(menu_info_types.SERVER_PERFORMANCE_LISTEN_STOP, performance.SID_RADIAL_PERFORMANCE_LISTEN_STOP);
			}
			
			if (hasScript(self, performance.MUSIC_HEARTBEAT_SCRIPT) && (!isIdValid(listenTarget) || !group.inSameGroup(self, listenTarget)))
			{
				mi.addRootMenu(menu_info_types.SERVER_PERFORMANCE_LISTEN, performance.SID_RADIAL_PERFORMANCE_LISTEN);
			}
			
			obj_id watchTarget = getPerformanceWatchTarget(player);
			
			if (watchTarget == self)
			{
				mi.addRootMenu(menu_info_types.SERVER_PERFORMANCE_WATCH_STOP, performance.SID_RADIAL_PERFORMANCE_WATCH_STOP);
			}
			else
			{
				
				if (hasScript(self, performance.DANCE_HEARTBEAT_SCRIPT))
				{
					mi.addRootMenu(menu_info_types.SERVER_PERFORMANCE_WATCH, performance.SID_RADIAL_PERFORMANCE_WATCH);
				}
			}
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int OnObjectMenuSelect(obj_id self, obj_id player, int item) throws InterruptedException
	{
		if (item == menu_info_types.COMBAT_DEATH_BLOW)
		{
			
		}
		else if (item == menu_info_types.SERVER_MENU9)
		{
			queueCommand(player, (-1360827551), self, "", COMMAND_PRIORITY_DEFAULT);
		}
		else if (item == menu_info_types.SERVER_MENU1)
		{
			queueCommand(player, (-915040138), self, "", COMMAND_PRIORITY_DEFAULT);
		}
		else if (item == menu_info_types.SERVER_PERFORMANCE_LISTEN)
		{
			
			dictionary params = new dictionary();
			params.put("target", self);
			messageTo(player, "handleListenRadialCmd", params, 0, false);
			
			sendDirtyObjectMenuNotification(self);
		}
		else if (item == menu_info_types.SERVER_PERFORMANCE_WATCH)
		{
			
			dictionary params = new dictionary();
			params.put("target", self);
			messageTo(player, "handleWatchRadialCmd", params, 0, false);
			
			sendDirtyObjectMenuNotification(self);
		}
		else if (item == menu_info_types.SERVER_PERFORMANCE_LISTEN_STOP)
		{
			
			dictionary params = new dictionary();
			messageTo(player, "handleListenRadialCmd", params, 0, false);
			
			sendDirtyObjectMenuNotification(self);
		}
		else if (item == menu_info_types.SERVER_PERFORMANCE_WATCH_STOP)
		{
			
			dictionary params = new dictionary();
			messageTo(player, "handleWatchRadialCmd", params, 0, false);
			
			sendDirtyObjectMenuNotification(self);
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int OnMadeAuthoritative(obj_id self) throws InterruptedException
	{
		
		String flag = getConfigSetting("GameServer", "skipTutorial");
		if (flag != null && (flag.equals("true") || flag.equals("1")))
		{
			setObjVar(self, "skipTutorial", 1);
		}
		return SCRIPT_CONTINUE;
	}
	
	
	public int OnLogin(obj_id self) throws InterruptedException
	{
		
		boolean ctsDisconnectRequested = false;
		if (hasObjVar(self, "disableLoginCtsInProgress"))
		{
			int timeOut = getIntObjVar(self, "disableLoginCtsInProgress");
			if (timeOut > getGameTime())
			{
				ctsDisconnectRequested = true;
				messageTo(self, "disconnectPlayerCtsCompletedOrInProgress", null, 0.1f, false);
				CustomerServiceLog("Login", "dropping character ("+ self + ": "+ getName(self) + ") because of CTS completed or in progress");
			}
			else
			{
				removeObjVar(self, "disableLoginCtsInProgress");
			}
		}
		
		if (!ctsDisconnectRequested && hasObjVar(self, "disableLoginCtsCompleted") && !isUsingAdminLogin(self))
		{
			ctsDisconnectRequested = true;
			messageTo(self, "disconnectPlayerCtsCompletedOrInProgress", null, 0.1f, false);
			CustomerServiceLog("Login", "dropping character ("+ self + ": "+ getName(self) + ") because of CTS completed or in progress");
		}
		
		if (!utils.hasScriptVar(self, "galaxyMessage.showmessage"))
		{
			obj_id planetId = getPlanetByName("tatooine");
			if (!isIdNull(planetId))
			{
				utils.setScriptVar(self, "galaxyMessage.showmessage", true);
				if (hasObjVar(planetId, "galaxyMessage"))
				{
					String strGalaxyMessage = "\\#FF0000"+utils.getStringObjVar(planetId, "galaxyMessage")+"\\#FFFFFF";
					sendConsoleMessage(self, strGalaxyMessage);
				}
				else
				{
					String strGalaxyMessage = "\\#FF0000"+"Welcome to Star Wars Galaxies"+"\\#FFFFFF";
					sendConsoleMessage(self, strGalaxyMessage);
				}
				
				boolean warden = isWarden(self);
				if (warden || (getGodLevel(self) >= 10))
				{
					String strGalaxyMessage = "\\#FF0000";
					if (!warden)
					{
						strGalaxyMessage += "Displaying warden MOTD to you even though you are not a warden, because you are of the appropriate god level:\n";
					}
					
					if (hasObjVar(planetId, "galaxyWardenMessage"))
					{
						strGalaxyMessage += utils.getStringObjVar(planetId, "galaxyWardenMessage");
					}
					else
					{
						strGalaxyMessage += "Welcome warden";
					}
					
					strGalaxyMessage += "\\#FFFFFF";
					sendConsoleMessage(self, strGalaxyMessage);
				}
			}
		}
		
		removeObjVar( self, "noTrade");
		
		if (hasObjVar (self, "comingFromTutorial"))
		{
			removeObjVar (self, "comingFromTutorial");
			attachScript (self, "theme_park.newbie_tutorial.new_player_ribbon");
		}
		
		if (!hasObjVar( self, "combatLevel") && hasObjVar( self, "clickRespec.combatLevel"))
		{
			setObjVar(self, "combatLevel", getIntObjVar(self,"clickRespec.combatLevel"));
			removeObjVar(self,"clickRespec.combatLevel");
		}
		
		if (getConfigSetting("GameServer", "combatUpgradeReward") != null)
		{
			if (!hasScript( self, "cureward.cureward" ))
			{
				attachScript( self, "cureward.cureward");
			}
		}
		
		obj_id bldg = getTopMostContainer( self );
		if (isIdValid(bldg))
		{
			dictionary parms = new dictionary();
			parms.put( "player", self );
			messageTo( bldg, "handlePlayerLogin", parms, 0, false );
			
			String bldgName = getTemplateName (bldg);
			String setting = getConfigSetting("Dungeon", "Death_Watch");
			
			if (bldgName.equals("object/building/general/bunker_allum_mine.iff"))
			{
				if (setting == null || setting.equals("false") || setting.equals("0"))
				{
					CustomerServiceLog("DUNGEON_DeathWatchBunker", "*Death Watch Unauthorized Entry: %TU entered the death watch bunker while it was turned off.", self);
				}
			}
			
			if (hasObjVar(bldg, "player_structure.condemned"))
			{
				player_structure.doCondemnedSui(bldg, self);
				expelFromBuilding(self);
			}
			
		}
		
		chatEnterRoom("SWG.system");
		
		chatEnterRoom("SWG."+ getGalaxyName() + ".system");
		
		chatEnterRoom("SWG."+ getGalaxyName() + "."+ getCurrentSceneName() + ".system");
		
		float curScale = getScale(self);
		int gender = getGender(self);
		int intSpecies = getSpecies(self);
		String species = utils.getPlayerSpeciesName(intSpecies);
		
		float min = 1.0f;
		float max = 1.0f;
		
		if (gender == GENDER_MALE)
		{
			min = dataTableGetFloat("datatables/player/racial_scale.iff", species, "MALE_MIN");
			max = dataTableGetFloat("datatables/player/racial_scale.iff", species, "MALE_MAX");
		}
		else
		{
			min = dataTableGetFloat("datatables/player/racial_scale.iff", species, "FEMALE_MIN");
			max = dataTableGetFloat("datatables/player/racial_scale.iff", species, "FEMALE_MAX");
		}
		
		if (min > 0 && max > 0)
		{
			if (curScale < min)
			{
				setScale(self, min);
			}
			else if (curScale > max)
			{
				setScale(self, max);
			}
		}
		
		if (getState(self, STATE_MASK_SCENT) == 1)
		{
			if (utils.hasScriptVar( self, "scentmask.camokit" ))
			{
				String kitPlanet = utils.getStringScriptVar(self, "scentmask.camokit");
				String planetName = getCurrentSceneName();
				if (planetName.startsWith("kashyyyk"))
				{
					planetName = "kashyyyk";
				}
				if (!kitPlanet.equals(planetName))
				{
					messageTo(self, "removeScentMaskNoNotify", null, 0, false);
				}
				
			}
			else
			{
				messageTo(self, "removeScentMaskNoNotify", null, 0, false);
			}
		}
		if (hasSkill(self, "demo_combat"))
		{
			revokeSkill(self, "demo_combat");
		}
		
		boolean needsPrerequisites = true;
		int attempts = 0;
		while (needsPrerequisites && attempts < 100)
		{
			testAbortScript();
			++attempts;
			needsPrerequisites = false;
			String[] skillList = getSkillListingForPlayer(self);
			if (skillList != null)
			{
				int i = 0;
				for (i = 0; i < skillList.length; ++i)
				{
					testAbortScript();
					String[] prereqs = getSkillPrerequisiteSkills(skillList[i]);
					if (prereqs != null)
					{
						int j = 0;
						for (j = 0; j < prereqs.length; ++j)
						{
							testAbortScript();
							if (! hasSkill(self, prereqs[j]))
							{
								grantSkill(self, prereqs[j]);
								needsPrerequisites = true;
							}
						}
					}
				}
			}
		}
		
		if (hasObjVar( self, player_structure.VAR_RESIDENCE_BUILDING ))
		{
			setHouseId( self, getObjIdObjVar( self, player_structure.VAR_RESIDENCE_BUILDING ) );
			removeObjVar( self, player_structure.VAR_RESIDENCE_BUILDING );
		}
		
		slicing.clearSlicing( self );
		
		hq.ejectEnemyFactionOnLogin( self );
		
		if (!isIdValid(getGroupObject(self)))
		{
			obj_id groupWaypoint = getObjIdObjVar(self, "groupWaypoint");
			if (isIdValid(groupWaypoint))
			{
				destroyWaypointInDatapad(groupWaypoint, self);
				removeObjVar(self, "groupWaypoint");
			}
		}
		
		veteran_deprecated.updateVeteranTime(self);
		
		if (hasObjVar(self, "renamePerformed"))
		{
			String old_name = getStringObjVar(self, "renamePerformed");
			removeObjVar(self, "renamePerformed");
			if (old_name != null)
			{
				if (force_rank.isForceRanked(self))
				{
					
					force_rank.renameRankMember(self, old_name);
				}
			}
		}
		
		meditation.endMeditation(self, false);
		
		if (isIdValid(getGroupObject(self)) && (!hasScript(self, group.SCRIPT_GROUP_MEMBER)))
		{
			attachScript(self, group.SCRIPT_GROUP_MEMBER);
		}
		
		utils.removeScriptVar(self, COUPE_DE_GRACE_TARGET);
		
		if (!isInTutorialArea(self))
		{
			space_dungeon.verifyPlayerSession(self);
			space_dungeon.validateInstanceControllerId(self);
		}
		
		int campXp = getExperiencePoints(self, "camp");
		if (campXp > 0)
		{
			grantExperiencePoints(self, "scout", campXp );
			grantExperiencePoints(self, "camp", 0-campXp );
		}
		
		if (!utils.hasScriptVar(self,"bountyConsistencyCheck"))
		{
			
			Vector playerBountyMissionTargetId = new Vector();
			playerBountyMissionTargetId.setSize(0);
			
			obj_id[] objMissions = getMissionObjects(self);
			if (objMissions != null)
			{
				for (int i = 0; i < objMissions.length; ++i)
				{
					testAbortScript();
					String missionType = getMissionType(objMissions[i]);
					if (missionType.equals("bounty"))
					{
						if (hasObjVar(objMissions[i], "objTarget"))
						{
							playerBountyMissionTargetId = utils.addElement(playerBountyMissionTargetId, getObjIdObjVar(objMissions[i], "objTarget"));
						}
					}
				}
			}
			
			obj_id[] bounties = getBountyHunterBounties(self);
			if (bounties == null)
			{
				bounties = new obj_id[0];
			}
			
			boolean foundMatch = false;
			for (int i = 0; i < playerBountyMissionTargetId.size(); ++i)
			{
				testAbortScript();
				foundMatch = false;
				
				for (int j = 0; j < bounties.length; ++j)
				{
					testAbortScript();
					if (((obj_id)(playerBountyMissionTargetId.get(i))) == bounties[j])
					{
						foundMatch = true;
						break;
					}
				}
				
				if (!foundMatch)
				{
					CustomerServiceLog("bounty_inconsistency", self + " ("+ getName(self) + ") has a bounty mission for "+ ((obj_id)(playerBountyMissionTargetId.get(i))) + " ("+ getPlayerName(((obj_id)(playerBountyMissionTargetId.get(i)))) + ") but doesn't have a corresponding bounty");
					
					dictionary d = new dictionary();
					d.put("target", ((obj_id)(playerBountyMissionTargetId.get(i))));
					messageTo(self, "handleBountyMissionIncomplete", d, 10.0f, false);
				}
			}
			
			for (int k = 0; k < bounties.length; ++k)
			{
				testAbortScript();
				foundMatch = false;
				
				for (int l = 0; l < playerBountyMissionTargetId.size(); ++l)
				{
					testAbortScript();
					if (((obj_id)(playerBountyMissionTargetId.get(l))) == bounties[k])
					{
						foundMatch = true;
						break;
					}
				}
				
				if (!foundMatch)
				{
					CustomerServiceLog("bounty_inconsistency", self + " ("+ getName(self) + ") has a bounty for "+ bounties[k] + " ("+ getPlayerName(bounties[k]) + ") but doesn't have a corresponding bounty misssion");
					
					removeJediBounty(bounties[k], self);
				}
			}
			
			utils.setScriptVar(self,"bountyConsistencyCheck",1);
		}
		
		if (!utils.hasScriptVar(self,"shipShieldOneTimeRebalance"))
		{
			obj_id[] objPcds = space_transition.findShipControlDevicesForPlayer(self);
			if (objPcds != null && objPcds.length > 0)
			{
				for (int i = 0; i < objPcds.length; ++i)
				{
					testAbortScript();
					obj_id objShip = space_transition.getShipFromShipControlDevice(objPcds[i]);
					if (isIdValid(objShip))
					{
						space_combat.simpleShieldRatioRebalance(objShip);
					}
				}
			}
			
			utils.setScriptVar(self,"shipShieldOneTimeRebalance",1);
		}
		
		armor.calculateWornArmor(self);
		
		squad_leader.validateRallyPoint(self);
		
		if (!utils.hasScriptVar(self, "waitingOnCloneRespawn"))
		{
			int posture = getPosture(self);
			if (posture == POSTURE_DEAD)
			{
				if (!hasObjVar(self, pclib.VAR_BEEN_COUPDEGRACED))
				{
					setObjVar(self, pclib.VAR_DEATHBLOW_KILLER, self);
					setObjVar(self, pclib.VAR_DEATHBLOW_STAMP, getGameTime());
				}
				
				messageTo(self, pclib.HANDLER_PLAYER_DEATH, null, 5, false);
			}
			else
			{
				if (hasObjVar(self, pclib.VAR_BEEN_COUPDEGRACED))
				{
					removeObjVar(self, pclib.VAR_BEEN_COUPDEGRACED);
				}
			}
		}
		else
		{
			
			messageTo(self, "handleCloneRespawn", null, 1f, false);
		}
		
		if (groundquests.hasCompletedQuest(self, "ep3_avatar_self_destruct"))
		{
			badge.grantBadge(self, "bdg_kash_avatar_zssik");
		}
		
		if (!utils.hasScriptVar(self, "performance.buildabuff.buffComponentKeys") && buff.hasBuff(self, "buildabuff_inspiration"))
		{
			buff.removeBuff(self, "buildabuff_inspiration");
		}
		
		if (getLocation(self).area == "dungeon1")
		{
			if (trial.getTop(self) == self)
			{
				warpPlayer( self, "tatooine", 0, 0, 0, null, 0, 0, 0, null, false );
			}
		}
		
		if (utils.isProfession(self, utils.FORCE_SENSITIVE) && getLevel(self) > 3)
		{
			if (!buff.isInStance(self) && !buff.isInFocus(self))
			{
				messageTo(self, "applyJediStance", null, 1.0f, false);
			}
		}
		
		if (utils.isProfession(self, utils.SMUGGLER))
		{
			messageTo(self, "applySmugglingBonuses", null, 1.0f, false);
		}
		
		int cityId = getCitizenOfCityId(self);
		if (cityId > 0 && !utils.hasScriptVar(self, "recieved_city_motd"))
		{
			obj_id cityHall = cityGetCityHall(cityId);
			dictionary outparams = new dictionary();
			outparams.put("player", self);
			messageTo(cityHall, "msgCheckMyCityMotd", outparams, 0, false);
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int disconnectPlayerCtsCompletedOrInProgress(obj_id self, dictionary params) throws InterruptedException
	{
		setObjVar(self, "safeLogout", 1);
		disconnectPlayer(self);
		return SCRIPT_CONTINUE;
	}
	
	
	public int ctsCompletedForCharacter(obj_id self, dictionary params) throws InterruptedException
	{
		
		if (utils.hasScriptVar(self, cts.SCRIPTVAR_CTS_ITEM_ID))
		{
			final obj_id ctsItem = utils.getObjIdScriptVar(self, cts.SCRIPTVAR_CTS_ITEM_ID);
			if (isIdValid(ctsItem))
			{
				CustomerServiceLog("CharacterTransfer", "destroying item "+ ctsItem + " used for ingame CTS by character "+ self);
				destroyObject(ctsItem);
			}
		}
		
		setObjVar(self, "disableLoginCtsCompleted", getCalendarTime());
		messageTo(self, "disconnectPlayerCtsCompletedOrInProgress", null, 0.1f, false);
		return SCRIPT_CONTINUE;
	}
	
	
	public int applyJediStance(obj_id self, dictionary params) throws InterruptedException
	{
		if (hasSkill(self, "expertise_fs_path_cautious_nature_1"))
		{
			buff.applyBuff(self, self, jedi.JEDI_STANCE);
		}
		else
		{
			buff.applyBuff(self, self, jedi.JEDI_FOCUS);
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int applySmugglingBonuses(obj_id self, dictionary params) throws InterruptedException
	{
		removeSmugglingBuffs(self);
		
		messageTo(self, "addSmugglingBuffs", null, 1.0f, false);
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int addSmugglingBuffs(obj_id self, dictionary params) throws InterruptedException
	{
		int smUnderworld = (int)getSkillStatisticModifier(self, "expertise_underworld_boss_bonus");
		int rangeBonus = (int)getSkillStatisticModifier(self, "expertise_sm_rank_range_buff");
		int damageBonus = (int)getSkillStatisticModifier(self, "expertise_sm_rank_damage_buff");
		
		float underworldFaction = factions.getFactionStanding(self, "underworld");
		
		int smugglerTier = smuggler.getSmugglerRank(underworldFaction);
		
		switch(smugglerTier)
		{
			case 1:
			if (smUnderworld > 0)
			{
				buff.applyBuff(self, "sm_underworld_boss_1");
			}
			if (rangeBonus > 0)
			{
				buff.applyBuff(self, "sm_underworld_range_1");
			}
			if (damageBonus > 0)
			{
				buff.applyBuff(self, "sm_underworld_damage_1");
			}
			break;
			case 2:
			if (smUnderworld > 0)
			{
				buff.applyBuff(self, "sm_underworld_boss_2");
			}
			if (rangeBonus > 0)
			{
				buff.applyBuff(self, "sm_underworld_range_2");
			}
			if (damageBonus > 0)
			{
				buff.applyBuff(self, "sm_underworld_damage_2");
			}
			break;
			case 3:
			if (smUnderworld > 0)
			{
				buff.applyBuff(self, "sm_underworld_boss_3");
			}
			if (rangeBonus > 0)
			{
				buff.applyBuff(self, "sm_underworld_range_3");
			}
			if (damageBonus > 0)
			{
				buff.applyBuff(self, "sm_underworld_damage_3");
			}
			break;
			default: break;
		}
		
		messageTo(self, "recalcArmor", null, 1, false);
		messageTo(self, "recalcWeaponRange", null, 1, false);
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int recalcWeaponRange(obj_id self, dictionary params) throws InterruptedException
	{
		obj_id tempWeapon = getObjectInSlot(self, "hold_r");
		
		if (isIdValid(tempWeapon))
		{
			weapons.adjustWeaponRangeForExpertise(self, tempWeapon, true);
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public void removeSmugglingBuffs(obj_id who) throws InterruptedException
	{
		buff.removeBuff(who, "sm_underworld_boss_1");
		buff.removeBuff(who, "sm_underworld_boss_2");
		buff.removeBuff(who, "sm_underworld_boss_3");
		buff.removeBuff(who, "sm_underworld_range_1");
		buff.removeBuff(who, "sm_underworld_range_2");
		buff.removeBuff(who, "sm_underworld_range_3");
		buff.removeBuff(who, "sm_underworld_damage_1");
		buff.removeBuff(who, "sm_underworld_damage_2");
		buff.removeBuff(who, "sm_underworld_damage_3");
		
		messageTo(who, "recalcArmor", null, 1, false);
		messageTo(who, "recalcWeaponRange", null, 1, false);
	}
	
	
	public int removeSmugglingBonuses(obj_id self, dictionary params) throws InterruptedException
	{
		removeSmugglingBuffs(self);
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int handlerRemoveGroupBuffEffect(obj_id self, dictionary params) throws InterruptedException
	{
		if (params == null || params.isEmpty())
		{
			return SCRIPT_CONTINUE;
		}
		
		int[] buffList = params.getIntArray("buffList");
		
		if (buffList == null || buffList.length <= 0)
		{
			return SCRIPT_CONTINUE;
		}
		
		buff.removeGroupBuffEffect(self, buffList);
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int OnNewbieTutorialResponse(obj_id self, String action) throws InterruptedException
	{
		if (action.equals("clientReady"))
		{
			
			int playerState = getState(self, STATE_DISGUISE);
			
			if (playerState > 0)
			{
				int shapechange = buff.getBuffOnTargetFromGroup(self, "shapechange");
				
				if (shapechange == 0)
				{
					revertObjectAppearance(self);
				}
			}
			
			location myLoc = getLocation(self);
			
			if (hasTriggerVolume(self, "group_buff_breach"))
			{
				obj_id groupId = getGroupObject(self);
				
				if (isIdValid(groupId))
				{
					
					obj_id[] groupMembers = getGroupMemberIds(groupId);
					
					if (groupMembers != null && groupMembers.length > 0)
					{
						
						int[] buffList = buff.getOwnedGroupBuffs(self);
						dictionary dict = new dictionary();
						dict.put("buffList", buffList);
						
						for (int i = 0; i < groupMembers.length; ++i)
						{
							testAbortScript();
							
							location playerLoc = getLocation(groupMembers[i]);
							
							if (playerLoc == null)
							{
								messageTo(groupMembers[i], "handlerRemoveGroupBuffEffect", dict, 1, false);
								continue;
							}
							
							if (!myLoc.area.equals(playerLoc.area))
							{
								messageTo(groupMembers[i], "handlerRemoveGroupBuffEffect", dict, 1, false);
								continue;
							}
							
							float distance = getDistance(myLoc, playerLoc);
							
							if (distance > 110.0f)
							{
								messageTo(groupMembers[i], "handlerRemoveGroupBuffEffect", dict, 1, false);
								continue;
							}
						}
					}
				}
			}
			
			int[] allBuffs = buff.getAllBuffs(self);
			
			if (allBuffs != null && allBuffs.length > 0)
			{
				for (int i = 0; i < allBuffs.length; ++i)
				{
					testAbortScript();
					if (buff.isAuraBuff(allBuffs[i]))
					{
						obj_id caster = buff.getBuffCaster(self, allBuffs[i]);
						
						if (!isIdValid(caster) || !exists(caster))
						{
							buff.removeBuff(self, allBuffs[i]);
							continue;
						}
						
						location casterLoc = getLocation(caster);
						
						if (!myLoc.area.equals(casterLoc.area))
						{
							buff.removeBuff(self, allBuffs[i]);
							continue;
						}
						
						float distance = getDistance(myLoc, casterLoc);
						
						if (distance > 110.0f)
						{
							buff.removeBuff(self, allBuffs[i]);
							continue;
						}
					}
				}
			}
			
			String halloweenRunning = getConfigSetting("GameServer", "halloween");
			
			if (halloweenRunning == null)
			{
				if (hasObjVar(self, event_perk.COUNTER_TIMESTAMP))
				{
					removeObjVar(self, event_perk.COUNTER_TIMESTAMP);
				}
				
				if (hasObjVar(self, event_perk.COUNTER))
				{
					removeObjVar(self, event_perk.COUNTER);
				}
				
				if (hasObjVar(self, event_perk.COUNTER_RESTARTTIME))
				{
					removeObjVar(self, event_perk.COUNTER_RESTARTTIME);
				}
			}
			
			String lifedayRunning = getConfigSetting("GameServer", "lifeday");
			
			if (lifedayRunning == null)
			{
				if (hasObjVar(self, "lifeday"))
				{
					removeObjVar(self, "lifeday");
				}
			}
			if (lifedayRunning != null)
			{
				if (lifeDayNewDay(self))
				{
					if (hasObjVar(self, "lifeday"))
					{
						removeObjVar(self, "lifeday");
					}
					
					if (buff.hasBuff(self, "event_lifeday_imperial_competitive"))
					{
						buff.removeBuff(self, "event_lifeday_imperial_competitive");
					}
					if (buff.hasBuff(self, "event_lifeday_rebel_competitive"))
					{
						buff.removeBuff(self, "event_lifeday_rebel_competitive");
					}
					if (buff.hasBuff(self, "event_lifeday_imperial_present_counter"))
					{
						buff.removeBuff(self, "event_lifeday_imperial_present_counter");
					}
					if (buff.hasBuff(self, "event_lifeday_rebel_tree_counter"))
					{
						buff.removeBuff(self, "event_lifeday_rebel_tree_counter");
					}
				}
			}
			
			if (hasScript(self, "systems.respec.click_combat_respec"))
			{
				detachScript(self, "systems.respec.click_combat_respec");
			}
			
			String skillTemplate = getSkillTemplate(self);
			String workingSkill = getWorkingSkill(self);
			if (skillTemplate == null || skillTemplate.equals("") || skillTemplate.equals("a"))
			{
				int combatLevel = respec.getOldCombatLevel(self);
				obj_id respecDevice = utils.getStaticItemInBankOrInventory(self, "item_respec_token_01_01");
				
				if (hasObjVar(self, "combatLevel"))
				{
					combatLevel = getIntObjVar(self, "combatLevel");
				}
				
				else
				{
					if (isIdValid(respecDevice))
					{
						combatLevel = getIntObjVar(respecDevice, "combatLevel");
					}
				}
				
				if (combatLevel > 80)
				{
					combatLevel = 80;
				}
				
				setObjVar( self, "combatLevel", combatLevel);
				
				setObjVar( self, "clickRespec.combatLevel", combatLevel);
				setObjVar( self, "clickRespec.npeRespec", 1);
				
				if (!isIdValid(respecDevice))
				{
					respecDevice = static_item.createNewItemFunction("item_respec_token_01_01", self);
					if (isIdValid(respecDevice))
					{
						setObjVar(respecDevice, "combatLevel", combatLevel);
						setObjVar(respecDevice, "highestLevel", true );
					}
				}
				
				attachScript(self, "systems.respec.click_combat_respec");
				
				messageTo(self, "delayRespecInstructions", null, 1, false);
			}
			
			if (hasObjVar(self, "npe.skippingTutorial"))
			{
				location origin = getLocation (self);
				location fighting = new location (3521f, 0.0f, -4821f, origin.area);
				location crafty = new location (3309.0f, 6.0f, -4785.0f, origin.area);
				String profession = getSkillTemplate (self);
				
				obj_id objInv = utils.getInventoryContainer(self);
				
				String questNewbieStart = "quest/speeder_quest";
				String questNewbieStartBH = "quest/speeder_quest";
				String questCrafterEntertainer = "quest/tatooine_eisley_noncombat";
				
				int crafter = profession.indexOf ("trader");
				int entertainer = profession.indexOf ("entertainer");
				int bountyhunter = profession.indexOf ("bounty_hunter");
				if (crafter > -1 || entertainer > -1)
				{
					if (!groundquests.isQuestActiveOrComplete(self, questCrafterEntertainer))
					{
						
						groundquests.grantQuest(self, questCrafterEntertainer);
					}
				}
				else if (bountyhunter > -1)
				{
					if (groundquests.hasCompletedQuest (self, questNewbieStartBH) || groundquests.isQuestActive (self, questNewbieStartBH))
					{
						detachScript (self, "npe.handoff_to_tatooine");
					}
					else
					{
						groundquests.requestGrantQuest (self, questNewbieStartBH);
					}
					
				}
				else
				{
					if (groundquests.hasCompletedQuest (self, questNewbieStart) || groundquests.isQuestActive (self, questNewbieStart))
					{
						detachScript (self, "npe.handoff_to_tatooine");
					}
					else
					{
						groundquests.requestGrantQuest (self, questNewbieStart);
					}
					
				}
				
				if (crafter > -1)
				{
					messageTo(self, "handleSurveyToolbarSetup", null, 5, false);
				}
				
				static_item.createNewItemFunction("item_npe_uniform_crate_01_01", objInv);
				
				npe.giveProfessionWeapon(self);
				
				removeObjVar(self, "npe.skippingTutorial");
				
				int combatLevel = getLevel( self );
				if (combatLevel < 5)
				{
					respec.autoLevelPlayer(self, 5, false);
				}
			}
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int OnCSCreateItem(obj_id self, obj_id container, String staticItemName) throws InterruptedException
	{
		gmlib.generateStaticItem( container, staticItemName );
		return SCRIPT_CONTINUE;
	}
	
	
	public int delayRespecInstructions(obj_id self, dictionary params) throws InterruptedException
	{
		int pid = createSUIPage( sui.SUI_MSGBOX, self, self, "noHandler");
		setSUIProperty(pid, "this", "Size", "475,575");
		setSUIProperty(pid, "this", "Location", "100,100");
		setSUIProperty(pid, sui.MSGBOX_TITLE, sui.PROP_TEXT, "@click_respec:respec_title");
		setSUIProperty(pid, sui.MSGBOX_PROMPT, sui.PROP_TEXT, "@click_respec:respec_message");
		sui.msgboxButtonSetup(pid, sui.OK_ONLY);
		showSUIPage(pid);
		flushSUIPage(pid);
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int OnRemovingFromWorld(obj_id self) throws InterruptedException
	{
		removeObjVar( self, "conversation");
		return SCRIPT_CONTINUE;
	}
	
	
	public int handleLoginLocResolved(obj_id self, dictionary params) throws InterruptedException
	{
		CustomerServiceLog("LOGIN_LOC", "("+self+":"+getPlayerObject(self)+") "+getName(self)+": warp callback entered @ loc="+getLocation(self).toString()+" world_loc="+getWorldLocation(self).toString());
		
		String prompt = "Due to a system error involving invalid login coordinates, an attempt has been made to warp you to a local starport.";
		prompt += " We apologize for the inconvenience.";
		
		sui.msgbox(self, prompt);
		return SCRIPT_CONTINUE;
	}
	
	
	public int handleStatProfileSUI(obj_id self, dictionary params) throws InterruptedException
	{
		if (!utils.hasScriptVar(self, "screwedStats.opt"))
		{
			return SCRIPT_CONTINUE;
		}
		
		String[] opt = utils.getStringArrayScriptVar(self, "screwedStats.opt");
		utils.removeScriptVar(self, "screwedStats.opt");
		
		int idx = sui.getListboxSelectedRow(params);
		if (idx == -1 || idx == 0)
		{
			sendSystemMessageTestingOnly(self, "You will be asked the previous question on each log-in until the problem is rectified.");
			return SCRIPT_CONTINUE;
		}
		
		if (opt == null || opt.length == 0)
		{
			return SCRIPT_CONTINUE;
		}
		
		if (idx < 1 || idx >= opt.length)
		{
			return SCRIPT_CONTINUE;
		}
		
		dictionary racial_mods = null;
		String template = utils.getTemplateFilenameNoPath(self);
		if (template != null && !template.equals(""))
		{
			if (template.endsWith(".iff"))
			{
				template = template.substring(0, template.length() - 4);
			}
			
			String racial_tbl = "datatables/creation/racial_mods.iff";
			int templateRow = 0;
			if (getGender(self) == GENDER_MALE)
			{
				templateRow = dataTableSearchColumnForString(template, 0, racial_tbl);
			}
			else
			{
				templateRow = dataTableSearchColumnForString(template, 1, racial_tbl);
			}
			
			racial_mods = dataTableGetRow(racial_tbl, templateRow);
		}
		
		if (racial_mods == null || racial_mods.isEmpty())
		{
			return SCRIPT_CONTINUE;
		}
		
		String profile = opt[idx];
		if (profile == null || profile.equals("") || profile.equals("none"))
		{
			return SCRIPT_CONTINUE;
		}
		
		dictionary prof_mods = dataTableGetRow("datatables/creation/profession_mods.iff", profile);
		for (int x = 0; x < NUM_ATTRIBUTES; x++)
		{
			testAbortScript();
			String aname = "";
			switch ( x )
			{
				case 0:
				aname = "health";
				break;
				case 1:
				aname = "constitution";
				break;
				case 2:
				aname = "action";
				break;
				case 3:
				aname = "stamina";
				break;
				case 4:
				aname = "mind";
				break;
				case 5:
				aname = "willpower";
				break;
			}
			
			int newval = racial_mods.getInt(aname) + prof_mods.getInt(aname);
			setMaxAttrib(self, x, newval);
		}
		
		healing.fullHeal(self);
		
		String prompt = " We apologize for any inconveniences this may cause.";
		
		sui.msgbox(self, prompt);
		return SCRIPT_CONTINUE;
	}
	
	
	public int OnLogout(obj_id self) throws InterruptedException
	{
		if (hasObjVar(self, pclib.VAR_CONSENT_FROM_ID))
		{
			pclib.relinquishConsents(self);
		}
		
		self.getScriptVars().remove(veteran_deprecated.SCRIPTVAR_VETERAN_LOGGED_IN);
		
		obj_id objBuilding = getTopMostContainer(self);
		if (isIdValid(objBuilding) && objBuilding != self)
		{
			if (hasScript(objBuilding, "planet_map.active_registered"))
			{
				dictionary dctParams = new dictionary();
				dctParams.put("objItem", self);
				messageTo(objBuilding, "removeRegistrant", dctParams, 0, false);
			}
			
			location buildingEjectionPoint = getBuildingEjectLocation(objBuilding);
			if (buildingEjectionPoint != null)
			{
				setObjVar(self, "building_ejection_point", buildingEjectionPoint);
			}
		}
		else
		{
			removeObjVar(self, "building_ejection_point");
		}
		
		if (utils.hasScriptVar(self, "battlefield.queue"))
		{
			String battlefieldName = utils.getStringScriptVar(self, "battlefield.queue");
			obj_id controller = null;
			
			if (battlefieldName != null || battlefieldName.length() > 0)
			{
				controller = utils.getObjIdScriptVar(self, "battlefield."+ battlefieldName);
			}
			
			if (isIdValid(controller))
			{
				dictionary params = new dictionary();
				
				params.put("player", self);
				
				if (factions.isRebel(self))
				{
					params.put("faction", factions.FACTION_FLAG_REBEL);
				}
				else if (factions.isImperial(self))
				{
					params.put("faction", factions.FACTION_FLAG_IMPERIAL);
				}
				
				messageTo(controller, "removePlayerFromQueue", params, 1.0f, false);
			}
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int OnUnloadedFromMemory(obj_id self) throws InterruptedException
	{
		chatExitRoom("SWG.system");
		
		chatExitRoom("SWG."+ getGalaxyName() + ".system");
		
		chatExitRoom("SWG."+ getGalaxyName() + "."+ getCurrentSceneName() + ".system");
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int OnImmediateLogout(obj_id self) throws InterruptedException
	{
		self.getScriptVars().remove(veteran_deprecated.SCRIPTVAR_VETERAN_LOGGED_IN);
		
		if (utils.hasScriptVar(self, "profit"))
		{
			
			int startMoney = utils.getIntScriptVar(self, "profit");
			int endMoney = getTotalMoney(self);
			
			utils.removeScriptVar(self, "profit");
			
			String profitThreshold = getConfigSetting("GameServer", "profitLoggingThreshold");
			if (profitThreshold == null || profitThreshold.equals(""))
			{
				
				profitThreshold = "500000";
			}
			if (utils.stringToInt(profitThreshold) > -1)
			{
				
				if ((endMoney - startMoney) > utils.stringToInt(profitThreshold))
				{
					
					CustomerServiceLog("Wealth", "Extraordinary Profit: "+ getName(self) + " ("+ self + ") logged in with "+ startMoney + " credits and logged out with "+ endMoney + " credits, for a profit of "+ (endMoney - startMoney) + " credits");
				}
			}
		}
		
		if (guild.hasWindowPid(self))
		{
			guild.removeWindowPid(self);
		}
		
		int vehicleBuff = buff.getBuffOnTargetFromGroup(self, "vehicle");
		if (vehicleBuff != 0)
		{
			buff.removeBuff(self, vehicleBuff);
		}
		
		if (callable.hasAnyCallable(self))
		{
			callable.storeCallables(self);
		}
		
		removeObjVar(self, "surveying.sampleLocation");
		
		if (hasObjVar(self, "safeLogout"))
		{
			if (getIntObjVar(self, "safeLogout") == 1)
			{
				return SCRIPT_CONTINUE;
			}
			
		}
		obj_id building = getTopMostContainer(self);
		
		if (isIdValid(building))
		{
			if (hasObjVar(building, "safeLogout") && getIntObjVar(building, "safeLogout") == 1)
			{
				return SCRIPT_CONTINUE;
			}
			
			if (hasScript(building, force_rank.SCRIPT_ENCLAVE_CONTROLLER))
			{
				if (!force_rank.isPlayersEnclave(building, self))
				{
					expelFromBuilding(self);
				}
			}
		}
		
		return SCRIPT_OVERRIDE;
	}
	
	
	public int OnAboutToReceiveItem(obj_id self, obj_id srcContainer, obj_id transferer, obj_id item) throws InterruptedException
	{
		int got = getGameObjectType(item);
		
		if (isGameObjectTypeOf(got, GOT_misc_container_wearable))
		{
			obj_id belt = getObjectInSlot(self, "utility_belt");
			if (isIdValid(belt) && isGameObjectTypeOf(belt, GOT_misc_container_wearable))
			{
				
				obj_id inv = utils.getInventoryContainer(self);
				putInOverloaded(belt, inv);
			}
		}
		return SCRIPT_CONTINUE;
	}
	
	
	public int OnReceivedItem(obj_id self, obj_id srcContainer, obj_id transferer, obj_id item) throws InterruptedException
	{
		int got = getGameObjectType(item);
		if (isGameObjectTypeOf(got, GOT_armor) && !isGameObjectTypeOf(got, GOT_armor_psg))
		{
			armor.calculateWornArmor(self);
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int OnLostItem(obj_id self, obj_id destContainer, obj_id transferer, obj_id item) throws InterruptedException
	{
		int got = getGameObjectType(item);
		if (isGameObjectTypeOf(got, GOT_armor) && !isGameObjectTypeOf(got, GOT_armor_psg))
		{
			armor.calculateWornArmor(self);
		}
		
		if (isGameObjectTypeOf(got, GOT_weapon))
		{
			combat.cacheCombatData(self);
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int OnTransferred(obj_id self, obj_id src, obj_id dest, obj_id transferer) throws InterruptedException
	{
		if ((pet_lib.isMount(src)) || (pet_lib.isMount(dest)) || (vehicle.isVehicle(src)) || (vehicle.isVehicle(dest)))
		{
			movement.refresh(self);
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int handleEjection(obj_id self, dictionary params) throws InterruptedException
	{
		obj_id sender = null;
		if (params == null || params.isEmpty())
		{
			sender = getTopMostContainer(self);
		}
		else
		{
			sender = params.getObjId("sender");
		}
		
		if (!isIdValid(sender))
		{
			return SCRIPT_CONTINUE;
		}
		
		location there = getBuildingEjectLocation(sender);
		if (there == null)
		{
			return SCRIPT_CONTINUE;
		}
		
		pclib.sendToAnyLocation(self, there, null);
		return SCRIPT_CONTINUE;
	}
	
	
	public int cmdEject(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		
		dictionary outparams = new dictionary();
		outparams.put( "loc", getLocation( self ) );
		messageTo( self, "handleDelayedEjection", outparams, 10.f, false );
		
		sendSystemMessage( self, SID_SYS_EJECT_REQUEST );
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int handleDelayedEjection(obj_id self, dictionary params) throws InterruptedException
	{
		if (params == null)
		{
			return SCRIPT_CONTINUE;
		}
		
		if (getLocation(self).area == "tutorial")
		{
			return SCRIPT_CONTINUE;
		}
		
		location loc = params.getLocation( "loc");
		location curloc = getLocation( self );
		if (getDistance( loc, curloc ) > 1f)
		{
			
			sendSystemMessage( self, SID_SYS_EJECT_FAIL_MOVE );
			return SCRIPT_CONTINUE;
		}
		
		obj_id instanceController = instance.getAreaInstanceController(self);
		
		if (isIdValid(instanceController))
		{
			String instanceName = instance.getInstanceName(instanceController);
			instance.requestExitPlayer(instanceName, self);
			return SCRIPT_CONTINUE;
		}
		
		{
			obj_id dungeon_id = space_dungeon.getDungeonIdForPlayer(self);
			if (isValidId(dungeon_id) && exists(dungeon_id))
			{
				return SCRIPT_CONTINUE;
				
			}
		}
		
		obj_id container = getTopMostContainer( self );
		obj_id building = null;
		if (isIdValid(container))
		{
			if (isGameObjectTypeOf( container, GOT_building ) && hasScript( container, "structure.permanent_structure" ))
			{
				ejectPlayerFromBuilding(self, container);
			}
			return SCRIPT_CONTINUE;
		}
		
		obj_id[] near = getNonCreaturesInRange( getWorldLocation(self), 128f );
		
		location here = getWorldLocation(self);
		
		obj_id closest = null;
		float maxdist = 9999.f;
		for (int i=0; i<near.length; i++)
		{
			testAbortScript();
			
			if (isGameObjectTypeOf( near[i], GOT_building ) || isGameObjectTypeOf(near[i], GOT_installation))
			{
				
				float dist = getDistance( here, getWorldLocation( near[i] ) );
				if (dist < maxdist)
				{
					maxdist = dist;
					closest = near[i];
				}
			}
		}
		
		if (isIdValid(closest))
		{
			if (isGameObjectTypeOf(closest, GOT_building))
			{
				ejectPlayerFromBuilding(self, closest);
			}
			else if (isGameObjectTypeOf(closest, GOT_installation))
			{
				float yaw = getYaw(closest);
				location there = getWorldLocation(closest);
				location loc1 = utils.rotatePointXZ(there, 8f, yaw+90f);
				location loc2 = utils.rotatePointXZ(there, 8f, yaw-90f);
				
				float d1 = getDistance(here, loc1);
				float d2 = getDistance(here, loc2);
				
				if (d1 < d2)
				{
					pclib.sendToAnyLocation( self, loc1, null );
				}
				else
				{
					pclib.sendToAnyLocation( self, loc2, null );
				}
				
				sendSystemMessage( self, SID_SYS_EJECT_SUCCESS );
			}
		}
		else
		{
			
			sendSystemMessage( self, SID_SYS_EJECT_FAIL_PROXIMITY );
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public void ejectPlayerFromBuilding(obj_id player, obj_id building) throws InterruptedException
	{
		
		location there = getBuildingEjectLocation( building );
		if (there == null)
		{
			return;
		}
		
		pclib.sendToAnyLocation( player, there, null );
		sendSystemMessage( player, SID_SYS_EJECT_SUCCESS );
	}
	
	
	public int handleWatchRadialCmd(obj_id self, dictionary params) throws InterruptedException
	{
		obj_id target = params.getObjId("target");
		
		if (isIdValid(target))
		{
			performance.watch(self, target);
		}
		else
		{
			performance.stopWatch(self);
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int handleListenRadialCmd(obj_id self, dictionary params) throws InterruptedException
	{
		obj_id target = params.getObjId("target");
		
		if (isIdValid(target))
		{
			performance.listen(self, target);
		}
		else
		{
			performance.stopListen(self);
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int handleSkillPurchase(obj_id self, dictionary params) throws InterruptedException
	{
		String boxTitle = params.getString(sui.LISTBOX_TITLE + "."+ sui.PROP_TEXT);
		String skillName = boxTitle.substring(boxTitle.indexOf(":") + 1);
		
		if (!skill.purchaseSkill(self, skillName))
		{
			
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int handleRequestCoinLoot(obj_id self, dictionary params) throws InterruptedException
	{
		if (params == null || params.isEmpty())
		{
			return SCRIPT_CONTINUE;
		}
		
		obj_id corpseId = params.getObjId(corpse.DICT_CORPSE_ID);
		if (!isIdValid(corpseId))
		{
			return SCRIPT_CONTINUE;
		}
		
		int cash = params.getInt(corpse.DICT_COINS);
		
		int safeCash = group.getSafeDifference(self, cash);
		if (safeCash < cash)
		{
			cash = safeCash;
			params.put(corpse.DICT_COINS, cash);
		}
		
		if (cash > 0)
		{
			if (transferBankCreditsFromNamedAccount(money.ACCT_NPC_LOOT, self, cash, null, null, null))
			{
				utils.moneyInMetric(corpseId, money.ACCT_NPC_LOOT, cash);
				
				withdrawCashFromBank(self, cash, corpse.HANDLER_COINS_LOOTED, corpse.HANDLER_COINLOOT_FAILED, params);
			}
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int handleUnconsentMenu(obj_id self, dictionary params) throws InterruptedException
	{
		int btnPressed = sui.getIntButtonPressed(params);
		switch(btnPressed)
		{
			case sui.BP_CANCEL:
			return SCRIPT_CONTINUE;
		}
		
		int idx = sui.getListboxSelectedRow(params);
		if (idx < 0)
		{
			return SCRIPT_CONTINUE;
		}
		
		if (hasObjVar(self, pclib.VAR_CONSENT_TO_ID))
		{
			obj_id[] consentTo = getObjIdArrayObjVar(self, pclib.VAR_CONSENT_TO_ID);
			if ((consentTo == null) || (consentTo.length == 0))
			{
				
			}
			else
			{
				obj_id target = consentTo[idx];
				queueCommand(self, (-1562441686), null, getPlayerFullName(target), COMMAND_PRIORITY_DEFAULT);
				return SCRIPT_CONTINUE;
			}
		}
		
		sendSystemMessage(self, SID_CONSENT_TO_EMPTY);
		return SCRIPT_CONTINUE;
	}
	
	
	public int handleConsentToLogout(obj_id self, dictionary params) throws InterruptedException
	{
		if (params == null)
		{
			return SCRIPT_OVERRIDE;
		}
		
		obj_id consented = params.getObjId(corpse.DICT_PLAYER_ID);
		obj_id[] corpses = getObjIdArrayObjVar(self, pclib.VAR_CORPSE_ID);
		if (corpses != null)
		{
			corpse.revokeCorpseConsent(corpses, consented);
		}
		
		Vector consentToList = getResizeableObjIdArrayObjVar(self, pclib.VAR_CONSENT_TO_ID);
		if (consentToList != null)
		{
			consentToList = utils.removeElement(consentToList, consented);
			if ((consentToList == null) || (consentToList.size() == 0))
			{
				removeObjVar(self, pclib.VAR_CONSENT_TO_ID);
				return SCRIPT_CONTINUE;
			}
			
			setObjVar(self, pclib.VAR_CONSENT_TO_ID, consentToList, resizeableArrayTypeobj_id);
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int handleReceivedConsent(obj_id self, dictionary params) throws InterruptedException
	{
		obj_id consenter = params.getObjId(pclib.DICT_CONSENTER_ID);
		
		Vector consentFrom = getResizeableObjIdArrayObjVar(self, pclib.VAR_CONSENT_FROM_ID);
		if (consentFrom == null)
		{
			Vector newConsentFrom = new Vector();
			newConsentFrom.setSize(0);
			consentFrom = newConsentFrom;
		}
		else
		{
			if (utils.getElementPositionInArray(consentFrom, consenter) > -1)
			{
				return SCRIPT_CONTINUE;
			}
		}
		
		consentFrom = utils.addElement(consentFrom, consenter);
		
		setObjVar(self, pclib.VAR_CONSENT_FROM_ID, consentFrom, resizeableArrayTypeobj_id);
		
		prose_package pp = prose.getPackage(PROSE_GOT_CONSENT, getPlayerFullName(consenter));
		sendSystemMessageProse(self, pp);
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int handleReceivedUnconsent(obj_id self, dictionary params) throws InterruptedException
	{
		obj_id consenter = params.getObjId(pclib.DICT_CONSENTER_ID);
		
		Vector consentFrom = getResizeableObjIdArrayObjVar(self, pclib.VAR_CONSENT_FROM_ID);
		if (consentFrom != null)
		{
			int idx = utils.getElementPositionInArray(consentFrom, consenter);
			if (idx > -1)
			{
				consentFrom = utils.removeElementAt(consentFrom, idx);
				
				if ((consentFrom == null) || (consentFrom.size() == 0))
				{
					removeObjVar(self, pclib.VAR_CONSENT_FROM_BASE);
				}
				else
				{
					setObjVar(self, pclib.VAR_CONSENT_FROM_ID, consentFrom, resizeableArrayTypeobj_id);
				}
				
				prose_package pp = prose.getPackage(PROSE_LOST_CONSENT, getPlayerFullName(consenter));
				sendSystemMessageProse(self, pp);
			}
		}
		return SCRIPT_CONTINUE;
	}
	
	
	public int handleCorpseCoinLoot(obj_id self, dictionary params) throws InterruptedException
	{
		if (params == null || params.isEmpty())
		{
			return SCRIPT_CONTINUE;
		}
		
		obj_id corpseId = params.getObjId(corpse.DICT_CORPSE_ID);
		int cash = params.getInt(corpse.DICT_COINS);
		
		if (isIdValid(corpseId))
		{
			obj_id nullId = null;
			queueCommand(self, (-1939113026), null, Integer.toString(cash), COMMAND_PRIORITY_DEFAULT);
			
			obj_id gid = getGroupObject(self);
			if (isIdValid(gid))
			{
				group.notifyCoinLoot(gid, self, corpseId, cash);
			}
			else
			{
				prose_package pp = prose.getPackage(PROSE_COIN_LOOT_NO_TARGET, null, nullId, cash);
				sendSystemMessageProse(self, pp);
			}
			CustomerServiceLog("Loot", "("+ self + ") "+ getName(self) + " looted "+ cash + " credits from ("+ corpseId + ") "+ getName(corpseId));
			
			obj_id cInv = utils.getInventoryContainer(corpseId);
			if (isIdValid(cInv))
			{
				obj_id[] corpseContents = corpse.getValidLootContents(getContents(cInv));
				if (corpseContents == null || corpseContents.length <= 0)
				{
					messageTo(corpseId, "handleCorpseExpire", null, 2, true);
					return SCRIPT_CONTINUE;
				}
				else
				{
					return SCRIPT_CONTINUE;
				}
			}
			
			messageTo(corpseId, "handleCorpseExpire", null, 2, true);
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int handleCorpseCoinLootFailed(obj_id self, dictionary params) throws InterruptedException
	{
		if (params == null)
		{
			return SCRIPT_CONTINUE;
		}
		
		obj_id corpseId = params.getObjId(corpse.DICT_CORPSE_ID);
		int cash = params.getInt(corpse.DICT_COINS);
		
		prose_package pp = prose.getPackage(PROSE_COIN_LOOT_FAILED, null, corpseId, cash);
		sendSystemMessageProse(self, pp);
		
		int got = getGameObjectType(corpseId);
		if (got == GOT_corpse)
		{
			CustomerServiceLog("Death", "("+ self + ") "+ getName(self) + " FAILED to loot "+ cash + " credits from ("+ corpseId + ") "+ getName(corpseId));
		}
		else
		{
			CustomerServiceLog("Loot", "("+ self + ") "+ getName(self) + " FAILED to loot "+ cash + " credits from ("+ corpseId + ") "+ getName(corpseId));
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int handleCorpseExpire(obj_id self, dictionary params) throws InterruptedException
	{
		if (params == null)
		{
			return SCRIPT_OVERRIDE;
		}
		
		obj_id corpseId = params.getObjId(corpse.DICT_CORPSE_ID);
		if (isIdValid(corpseId))
		{
			obj_id[] corpses = getObjIdArrayObjVar(self, pclib.VAR_CORPSE_ID);
			if (utils.getElementPositionInArray(corpses, corpseId) > -1)
			{
				sendSystemMessage(self, SID_CORPSE_EXPIRED);
			}
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int handleCorpseWaypointUpdate(obj_id self, dictionary params) throws InterruptedException
	{
		if (params == null)
		{
			return SCRIPT_OVERRIDE;
		}
		
		obj_id waypoint = params.getObjId(corpse.DICT_CORPSE_WAYPOINT);
		location loc = params.getLocation(corpse.DICT_CORPSE_LOC);
		
		setWaypointLocation(waypoint, loc);
		return SCRIPT_CONTINUE;
	}
	
	
	public int handleCorpseCleanup(obj_id self, dictionary params) throws InterruptedException
	{
		if (params == null)
		{
			
			return SCRIPT_OVERRIDE;
		}
		
		obj_id corpseId = params.getObjId(corpse.DICT_CORPSE_ID);
		obj_id waypoint = params.getObjId(corpse.DICT_CORPSE_WAYPOINT);
		
		destroyWaypointInDatapad(waypoint, self);
		
		Vector corpses = utils.getResizeableObjIdBatchObjVar(self, pclib.VAR_CORPSE_ID);
		Vector killers = utils.getResizeableObjIdBatchObjVar(self, pclib.VAR_CORPSE_KILLER);
		Vector stamps = utils.getResizeableIntBatchObjVar(self, pclib.VAR_CORPSE_STAMP);
		
		if ((corpses == null) || (killers == null))
		{
			
			return SCRIPT_OVERRIDE;
		}
		
		int idx = utils.getElementPositionInArray(corpses, corpseId);
		if (idx > -1)
		{
			corpses = utils.removeElementAt(corpses, idx);
			killers = utils.removeElementAt(killers, idx);
			stamps = utils.removeElementAt(stamps, idx);
			if ((corpses == null) || (corpses.size() == 0))
			{
				removeObjVar(self, pclib.VAR_CORPSE_BASE);
			}
			else
			{
				utils.setResizeableBatchObjVar(self, pclib.VAR_CORPSE_ID, corpses);
				utils.setResizeableBatchObjVar(self, pclib.VAR_CORPSE_KILLER, killers);
				utils.setResizeableBatchObjVar(self, pclib.VAR_CORPSE_STAMP, stamps);
			}
		}
		else
		{
			
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int handleIncapCounterDeath(obj_id self, dictionary params) throws InterruptedException
	{
		int posture = getPosture(self);
		obj_id objKiller = params.getObjId("objKiller");
		if (posture == POSTURE_INCAPACITATED && !hasObjVar(self, pclib.VAR_BEEN_COUPDEGRACED))
		{
			pclib.coupDeGrace(self, objKiller, false);
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int reportJediDeathCount(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int handleJediDeathCount(obj_id self, dictionary params) throws InterruptedException
	{
		if (isJedi(self))
		{
			int jediDeaths = getIntObjVar(self, jedi.VAR_JEDI_DEATHS);
			
			string_id msg = new string_id("jedi_spam", "jedi_death");
			prose_package proseMsg = prose.getPackage(msg, jediDeaths);
			
			sendSystemMessageProse(self, proseMsg);
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int decayJediDeath(obj_id self, dictionary params) throws InterruptedException
	{
		
		int jediDeaths = getIntObjVar(self, jedi.VAR_JEDI_DEATHS);
		jediDeaths--;
		if (jediDeaths < 0)
		{
			jediDeaths = 0;
		}
		
		setObjVar(self, jedi.VAR_JEDI_DEATHS, jediDeaths);
		
		if (isGod(self))
		{
			string_id msg = new string_id("jedi_spam", "jedi_death_decay");
			prose_package proseMsg = prose.getPackage(msg, jediDeaths);
			sendSystemMessageProse(self, proseMsg);
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int handleHealOverTimeTick(obj_id self, dictionary params) throws InterruptedException
	{
		float duration = params.getFloat("duration");
		float tick = params.getFloat("tick");
		int heal = params.getInt("heal");
		int hot_id = params.getInt("id");
		int isCombat = params.getInt("combat");
		obj_id medic = params.getObjId("medic");
		
		if (isDead(self))
		{
			if (buff.hasBuff(self, "healOverTime"))
			{
				buff.removeBuff(self, "healOverTime");
			}
			
			return SCRIPT_CONTINUE;
		}
		
		if (!utils.hasScriptVar(self, healing.VAR_PLAYER_HOT_ID))
		{
			if (buff.hasBuff(self, "healOverTime"))
			{
				buff.removeBuff(self, "healOverTime");
			}
			
			return SCRIPT_CONTINUE;
		}
		
		int player_id = utils.getIntScriptVar(self, healing.VAR_PLAYER_HOT_ID);
		
		if (hot_id != player_id)
		{
			return SCRIPT_CONTINUE;
		}
		
		if (!isIncapacitated(self))
		{
			int delta = healing.healDamage(medic, self, HEALTH, heal);
			
			if (isCombat == 1 && isIdValid(medic))
			{
				prose_package pp = new prose_package();
				pp = prose.setStringId(pp, new string_id("healing", "heal_fly"));
				pp = prose.setDI(pp, delta);
				pp = prose.setTO(pp, ATTRIBUTES[HEALTH]);
				showFlyTextPrivateProseWithFlags(self, self, pp, 2.0f, colors.SEAGREEN, FLY_TEXT_FLAG_IS_HEAL);
				
				if (medic != self)
				{
					showFlyTextPrivateProseWithFlags(self, medic, pp, 2.0f, colors.SEAGREEN, FLY_TEXT_FLAG_IS_HEAL);
				}
				
				pp = prose.setStringId(pp, healing.SID_PERFORM_HEAL_DAMAGE_SUCCESS);
				pp = prose.setTT(pp, medic);
				pp = prose.setTO(pp, self);
				pp = prose.setDI(pp, delta);
				healing.sendMedicalSpam(medic, self, pp, true, true, true, COMBAT_RESULT_MEDICAL);
				
				delta = (int)(delta*healing.HEAL_OVER_TIME_AGGRO_REDUCER);
				healing._addMedicalHate(medic, self, delta);
			}
		}
		
		duration -= tick;
		if (duration <= 0.0f)
		{
			if (buff.hasBuff(self, "healOverTime"))
			{
				buff.removeBuff(self, "healOverTime");
			}
			
			return SCRIPT_CONTINUE;
		}
		
		dictionary d = new dictionary();
		d.put("duration", duration);
		d.put("tick", tick);
		d.put("heal", heal);
		d.put("id", hot_id);
		d.put("combat", isCombat);
		d.put("medic", medic);
		
		if (tick >= 0)
		{
			messageTo(self, healing.MSG_HEAL_OVER_TIME, d, tick, false);
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int handlePlayerDeath(obj_id self, dictionary params) throws InterruptedException
	{
		
		groundquests.sendSignal(self, "smugglerEnemyIncap");
		
		if (utils.hasScriptVar(self, pclib.VAR_SUI_CLONE))
		{
			int oldpid = utils.getIntScriptVar(self, pclib.VAR_SUI_CLONE);
			forceCloseSUIPage(oldpid);
			
			utils.removeScriptVar(self, pclib.VAR_SUI_CLONE);
		}
		
		if (!isDead(self))
		{
			return SCRIPT_CONTINUE;
		}
		pvpRemoveAllTempEnemyFlags(self);
		
		clearBuffIcon(self, "food");
		
		if (isPvpRelatedDeath(self))
		{
			utils.setScriptVar(self, "buffDecay", 1);
			buff.decayAllBuffsFromPvpDeath(self);
		}
		else
		{
			buff.removeAllBuffs(self, true);
		}
		
		if (!hasObjVar(self, pclib.VAR_BEEN_COUPDEGRACED))
		{
			return SCRIPT_CONTINUE;
		}
		
		Vector options = new Vector();
		options.setSize(0);
		Vector cloneLocs = new Vector();
		cloneLocs.setSize(0);
		Vector spawnLocs = new Vector();
		spawnLocs.setSize(0);
		Vector damage = new Vector();
		damage.setSize(0);
		
		boolean isNewbieDeath = false;
		
		location deathLoc = pclib.getEffectiveDeathLocation(self);
		String planetName = deathLoc.area;
		obj_id planet = getPlanetByName(planetName);
		
		location worldLoc = getWorldLocation(self);
		String areaName = getBuildoutAreaName(worldLoc.x, worldLoc.z);
		
		obj_id bound = getObjIdObjVar(self, cloninglib.VAR_BIND_FACILITY);
		boolean isBindListed = false;
		
		obj_id controller = utils.getObjIdScriptVar(self, "battlefield.active");
		
		if (isIdValid(controller) && exists(controller))
		{
			location cloneLoc = null;
			String cloneName = "Clone Center";
			String battlefieldName = utils.getStringObjVar(controller, "battlefieldName");
			
			if (factions.isRebel(self))
			{
				cloneLoc = utils.getLocationScriptVar(controller, "battlefieldRebelSpawn");
				cloneName = "Rebel Cloning Center - "+ localize(new string_id("spam", battlefieldName));
			}
			else if (factions.isImperial(self))
			{
				cloneLoc = utils.getLocationScriptVar(controller, "battlefieldImperialSpawn");
				cloneName = "Imperial Cloning Center - "+ localize(new string_id("spam", battlefieldName));
			}
			
			options = utils.addElement(options, cloneName);
			cloneLocs = utils.addElement(cloneLocs, cloneLoc);
			spawnLocs = utils.addElement(spawnLocs, cloneLoc);
		}
		else
		{
			if (cloninglib.canUseBindFacility(planetName, areaName))
			{
				String bindName = "@base_player:clone_location_registered_select_begin ";
				
				if (hasObjVar(self, cloninglib.VAR_BIND_BASE))
				{
					String cloneName = getStringObjVar(self, cloninglib.VAR_BIND_CITY_NAME);
					location bindLoc = getLocationObjVar(self, cloninglib.VAR_BIND_LOCATION);
					location spawnLoc = getLocationObjVar(self, cloninglib.VAR_BIND_SPAWN_LOC);
					
					if (bindLoc != null)
					{
						if (isIdValid(bound))
						{
							if (city.isCityBanned(self, bound))
							{
								bindName += " @base_player:clone_location_city_banned";
								bindLoc = null;
								spawnLoc = null;
							}
							else
							{
								bindName += cloneName;
							}
						}
						else
						{
							bindName += "@base_player:clone_location_unknown";
							bindLoc = null;
							spawnLoc = null;
						}
					}
					else
					{
						bindName += "@base_player:clone_location_none";
						bindLoc = null;
						spawnLoc = null;
					}
					
					bindName += " @base_player:clone_location_registered_select_end";
					options = utils.addElement(options, bindName);
					cloneLocs = utils.addElement(cloneLocs, bindLoc);
					spawnLocs = utils.addElement(spawnLocs, spawnLoc);
					
					if (pvp.isPvpDeath(self) || isNewbieDeath)
					{
						damage = utils.addElement(damage, 0);
					}
					else
					{
						damage = utils.addElement(damage, cloninglib.CLONE_DAMAGE_LOW);
					}
					isBindListed = true;
				}
			}
			
			if (isIdValid(planet))
			{
				if (utils.hasScriptVar(planet, cloninglib.VAR_PLANET_CLONE_LOC))
				{
					Vector cloneList = cloninglib.getAvailableCloningFacilities(self);
					int playerFaction = factions.getFactionFlag(self);
					obj_id tatooinePlanet = getPlanetByName("tatooine");
					
					for (int i = 0; i < cloneList.size(); i++)
					{
						testAbortScript();
						dictionary cloneData = (dictionary)(cloneList.get(i));
						
						obj_id facility = cloneData.getObjId("faciltyId");
						String cloneName = cloneData.getString("cloneName");
						location cloneLoc = cloneData.getLocation("facilityLoc");
						location spawnLoc = cloneData.getLocation("spawnLoc");
						
						if (facility == bound && !isBindListed)
						{
							cloneName = "@base_player:clone_location_registered_select_begin "+
							getStringObjVar(self, cloninglib.VAR_BIND_CITY_NAME)+
							" @base_player:clone_location_registered_select_end";
							
							isBindListed = true;
						}
						
						if (cloneName.equals("@naboo_region_names:theed") && cloneLoc.z > 4100)
						{
							cloneName+= "_north";
						}
						
						if (playerFaction == factions.FACTION_FLAG_IMPERIAL || playerFaction == factions.FACTION_FLAG_REBEL)
						{
							if (cloneName.equals("@tatooine_region_names:bestine") && utils.hasScriptVar(tatooinePlanet, "gcw.invasionRunning.bestine"))
							{
								int factionDefending = utils.getIntScriptVar(tatooinePlanet, "gcw.factionDefending.bestine");
								
								if (( (factionDefending == factions.FACTION_FLAG_IMPERIAL || factionDefending == factions.FACTION_FLAG_REBEL) && playerFaction != factionDefending))
								{
									continue;
								}
							}
							
							if (cloneName.equals("@talus_region_names:dearic") && utils.hasScriptVar(tatooinePlanet, "gcw.invasionRunning.dearic"))
							{
								int factionDefending = utils.getIntScriptVar(tatooinePlanet, "gcw.factionDefending.dearic");
								
								if (( (factionDefending == factions.FACTION_FLAG_IMPERIAL || factionDefending == factions.FACTION_FLAG_REBEL) && playerFaction != factionDefending))
								{
									continue;
								}
							}
							
							if (cloneName.equals("@naboo_region_names:keren") && utils.hasScriptVar(tatooinePlanet, "gcw.invasionRunning.keren"))
							{
								int factionDefending = utils.getIntScriptVar(tatooinePlanet, "gcw.factionDefending.keren");
								
								if (( (factionDefending == factions.FACTION_FLAG_IMPERIAL || factionDefending == factions.FACTION_FLAG_REBEL) && playerFaction != factionDefending))
								{
									continue;
								}
							}
						}
						
						options = utils.addElement(options, cloneName);
						cloneLocs = utils.addElement(cloneLocs, cloneLoc);
						spawnLocs = utils.addElement(spawnLocs, spawnLoc);
					}
				}
			}
		}
		
		if (options == null || options.size() < 1)
		{
			
		}
		
		int suiClone = createCloningSui(self, options);
		
		if (suiClone == -1)
		{
			debugServerConsoleMsg(self, "PLAYER DEATH ERROR: unable to create sui for player = "+ self);
			debugServerConsoleMsg(self, "* reviving player at cuttent location...");
			
			return SCRIPT_CONTINUE;
		}
		
		utils.setScriptVar(self, pclib.VAR_SUI_CLONE, suiClone);
		utils.setScriptVar(self, pclib.VAR_REVIVE_OPTIONS, options);
		utils.setScriptVar(self, pclib.VAR_REVIVE_CLONE, cloneLocs);
		utils.setScriptVar(self, pclib.VAR_REVIVE_SPAWN, spawnLocs);
		
		dictionary hrem = new dictionary();
		hrem.put("time", healing.REVIVE_TIMER);
		hrem.put("sui", suiClone);
		utils.setScriptVar(self, combat.DIED_RECENTLY, true);
		messageTo(self, "clearRecentDeathScriptVar", null, combat.RECENT_DEATH_EXPIRATION, false);
		messageTo(self, "handleReviveExpirationMessage", hrem, 0.0f, false);
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int createCloningSui(obj_id player, Vector list) throws InterruptedException
	{
		String title = MSG_REVIVE_TITLE;
		String prompt = buildCloningPrompt(player, -1);
		
		int pid = createSUIPage(sui.SUI_LISTBOX, player, player, HANDLER_PLAYER_REVIVE);
		
		setSUIProperty(pid, sui.LISTBOX_TITLE, sui.PROP_TEXT, title);
		setSUIProperty(pid, sui.LISTBOX_PROMPT, sui.PROP_TEXT, prompt);
		
		sui.listboxButtonSetup(pid, sui.OK_ONLY);
		
		clearSUIDataSource(pid, sui.LISTBOX_DATASOURCE);
		
		for (int i = 0; i < list.size(); i++)
		{
			testAbortScript();
			addSUIDataItem(pid, sui.LISTBOX_DATASOURCE, ""+ i);
			setSUIProperty(pid, sui.LISTBOX_DATASOURCE + "."+ i, sui.PROP_TEXT, ((String)(list.get(i))));
		}
		
		subscribeToSUIEvent(pid, sui_event_type.SET_onGenericSelection, sui.LISTBOX_LIST, "handleCloneSuiUpdate");
		subscribeToSUIPropertyForEvent(pid, sui_event_type.SET_onGenericSelection, sui.LISTBOX_LIST, sui.LISTBOX_LIST, sui.PROP_SELECTEDROW);
		
		subscribeToSUIProperty(pid, sui.LISTBOX_LIST, sui.PROP_SELECTEDROW);
		subscribeToSUIProperty(pid, sui.LISTBOX_TITLE, sui.PROP_TEXT);
		
		showSUIPage(pid);
		flushSUIPage(pid);
		
		return pid;
	}
	
	
	public int handleCloneSuiUpdate(obj_id self, dictionary params) throws InterruptedException
	{
		if ((params == null) || (params.isEmpty()))
		{
			return SCRIPT_CONTINUE;
		}
		
		int pid = utils.getIntScriptVar(self, pclib.VAR_SUI_CLONE);
		int idx = sui.getListboxSelectedRow(params);
		
		String prompt = buildCloningPrompt(self, idx);
		
		setSUIProperty(pid, sui.LISTBOX_PROMPT, sui.PROP_TEXT, prompt);
		flushSUIPage(pid);
		
		return SCRIPT_CONTINUE;
	}
	
	
	public String buildCloningPrompt(obj_id player, int index) throws InterruptedException
	{
		String[] options = utils.getStringArrayScriptVar(player, pclib.VAR_REVIVE_OPTIONS);
		location[] cloneLocs = utils.getLocationArrayScriptVar(player, pclib.VAR_REVIVE_CLONE);
		
		String prompt = "@base_player:clone_prompt_header ";
		
		if (index > -1)
		{
			prompt += "\n\n @base_player:clone_prompt_data ";
			prompt += "\n @base_player:clone_prompt_location "+options[index];
			prompt += "\n @base_player:clone_prompt_distance "+distanceToString(getDistance(player, cloneLocs[index]));
		}
		
		return prompt;
	}
	
	
	public String distanceToString(float dist) throws InterruptedException
	{
		String text;
		
		if (dist < 0)
		{
			text = "@base_player:clone_location_unknown";
		}
		else if (dist < 1000)
		{
			text = ""+(int)dist+"m";
		}
		else
		{
			text = ""+(int)(dist/1000f)+"km";
		}
		
		return text;
	}
	
	
	public int handleCloningDataUpdate(obj_id self, dictionary params) throws InterruptedException
	{
		obj_id facilityId = params.getObjId("faciltyId");
		location facilityLoc = params.getLocation("facilityLoc");
		location spawnLoc = params.getLocation("spawnLoc");
		String cloneName = params.getString("cloneName");
		
		setObjVar(self, cloninglib.VAR_BIND_FACILITY, facilityId);
		setObjVar(self, cloninglib.VAR_BIND_LOCATION, facilityLoc);
		setObjVar(self, cloninglib.VAR_BIND_SPAWN_LOC, spawnLoc);
		setObjVar(self, cloninglib.VAR_BIND_CITY_NAME, cloneName);
		
		removeObjVar(self, cloninglib.VAR_BIND_FACILITY_TEMPLATE);
		removeObjVar(self, cloninglib.VAR_BIND_FACILITY_CELL);
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int clearRecentDeathScriptVar(obj_id self, dictionary params) throws InterruptedException
	{
		if (utils.hasScriptVar(self, combat.DIED_RECENTLY))
		{
			utils.removeScriptVar(self, combat.DIED_RECENTLY);
		}
		return SCRIPT_CONTINUE;
	}
	
	
	public int handleReviveExpirationMessage(obj_id self, dictionary params) throws InterruptedException
	{
		if (!utils.hasScriptVar(self, pclib.VAR_SUI_CLONE))
		{
			return SCRIPT_CONTINUE;
		}
		
		int remaining_time = params.getInt("time");
		int sui_id = params.getInt("sui");
		int next_msg_time = 600;
		
		int sui_var = utils.getIntScriptVar(self, pclib.VAR_SUI_CLONE);
		if (sui_id != sui_var)
		{
			return SCRIPT_CONTINUE;
		}
		
		prose_package pp = new prose_package();
		pp = prose.setStringId(pp, new string_id("base_player", "revive_exp_msg"));
		
		if (remaining_time <= 0)
		{
			sendSystemMessage(self, new string_id("base_player", "revive_expired"));
			forceCloseSUIPage(sui_id);
			
			if (space_dungeon.isCloningInstance(self))
			{
				space_dungeon.handleInstanceClone(self);
				return SCRIPT_CONTINUE;
			}
			
			location[] cloneLocs = utils.getLocationArrayScriptVar(self, pclib.VAR_REVIVE_CLONE);
			location[] spawnLocs = utils.getLocationArrayScriptVar(self, pclib.VAR_REVIVE_SPAWN);
			
			location clone = cloneLocs[0];
			location spawn = spawnLocs[0];
			
			if (!pclib.playerRevive(self, clone, spawn, 0))
			{
				messageTo(self, pclib.HANDLER_CLONE_RESPAWN, null, 1, true);
			}
			
			return SCRIPT_CONTINUE;
		}
		else if (remaining_time <= 60)
		{
			pp = prose.setStringId(pp, new string_id("base_player", "revive_exp_msg_last"));
			next_msg_time = remaining_time;
		}
		else if (remaining_time <= 300)
		{
			next_msg_time = 120;
		}
		else if (remaining_time <= 600)
		{
			next_msg_time = 300;
		}
		
		pp = prose.setTT(pp, player_structure.assembleTimeRemaining(player_structure.convertSecondsTime(remaining_time)));
		sendSystemMessageProse(self, pp);
		
		remaining_time -= next_msg_time;
		
		dictionary d = new dictionary();
		d.put("time", remaining_time);
		d.put("sui", sui_id);
		
		messageTo(self, "handleReviveExpirationMessage", d, (float)next_msg_time, false);
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int handleInstanceClonePause(obj_id self, dictionary params) throws InterruptedException
	{
		String prompt = "@"+SID_INSTANCE_REVIVE;
		String title = "@"+SID_INSTANCE_REVIVE_TITLE;
		int pid = sui.msgbox(self, self, prompt, sui.OK_ONLY, title, "handleInstanceCloneSelection");
		showSUIPage(pid);
		
		utils.setScriptVar(self, pclib.VAR_SUI_CLONE, pid);
		dictionary hrem = new dictionary();
		hrem.put("time", healing.REVIVE_TIMER);
		hrem.put("sui", pid);
		messageTo(self, "handleReviveExpirationMessage", hrem, 0.0f, false);
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int handleInstanceCloneSelection(obj_id self, dictionary params) throws InterruptedException
	{
		int posture = getPosture(self);
		if (posture != POSTURE_DEAD)
		{
			return SCRIPT_CONTINUE;
		}
		
		space_dungeon.handleInstanceClone(self);
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int showReviveSUI(obj_id self, dictionary params) throws InterruptedException
	{
		if (utils.hasScriptVar(self, "reviveSUI.pid"))
		{
			int oldpid = utils.getIntScriptVar(self, "reviveSUI.pid");
			sui.closeSUI(self, oldpid);
			utils.removeScriptVar(self, "reviveSUI.pid");
		}
		
		obj_id medic = params.getObjId("medic");
		int weak = params.getInt("weak");
		
		string_id reviveSUITitleSID = new string_id("spam", "revive_sui_title");
		prose_package ppPrompt = new prose_package();
		ppPrompt = prose.setStringId(ppPrompt, new string_id("spam", "revive_sui_prompt"));
		ppPrompt = prose.setTT(ppPrompt, medic);
		
		if (weak == 0)
		{
			reviveSUITitleSID = new string_id("spam", "revive_sui_title_weak");
			ppPrompt = prose.setStringId(ppPrompt, new string_id("spam", "revive_sui_prompt_weak"));
		}
		
		String title = "@"+ reviveSUITitleSID;
		
		utils.setScriptVar(self, "reviveSUI.medic", medic);
		utils.setScriptVar(self, "reviveSUI.weak", weak);
		
		int pid = sui.msgbox(self, self, ppPrompt, sui.YES_NO, title, "handleReviveSUI");
		utils.setScriptVar(self, "reviveSUI.pid", pid);
		
		showSUIPage(pid);
		
		params.put("pid", pid);
		messageTo(self, "handleReviveSUITimeout", params, 120, false);
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int handleReviveSUITimeout(obj_id self, dictionary params) throws InterruptedException
	{
		int pidThisMessageRefersToo = params.getInt("pid");
		
		if (utils.hasScriptVar(self, "reviveSUI.pid"))
		{
			int storedPid = utils.getIntScriptVar(self, "reviveSUI.pid");
			
			if (storedPid == pidThisMessageRefersToo)
			{
				sui.closeSUI(self, storedPid);
				utils.removeScriptVar(self, "reviveSUI.pid");
			}
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int handleReviveSUI(obj_id self, dictionary params) throws InterruptedException
	{
		int bp = sui.getIntButtonPressed(params);
		obj_id medic = utils.getObjIdScriptVar(self, "reviveSUI.medic");
		int weak = utils.getIntScriptVar(self, "reviveSUI.weak");
		
		if (bp == sui.BP_OK)
		{
			pclib.clearEffectsForDeath(self);
			int maxHealth = getWoundedMaxAttrib(self, HEALTH);
			int maxAction = getWoundedMaxAttrib(self, ACTION);
			
			int actionVal = weak * (maxAction/2);
			int toHeal = (maxHealth / 3) + 1;
			
			if (weak == 0)
			{
				toHeal = (maxHealth / 10) + 1;
			}
			
			setAttrib(self, HEALTH, toHeal);
			setAttrib(self, ACTION, actionVal);
			setAttrib(self, MIND, 0);
			
			utils.removeScriptVar(self, "reviveSUI.weak");
			utils.removeScriptVar(self, "reviveSUI.pid");
			messageTo(self, "handlePlayerResuscitated", null, 0, true);
		}
		else
		{
			prose_package pp = new prose_package();
			pp = prose.setStringId(pp, new string_id("spam", "revive_attempt_denied") );
			pp = prose.setTO(pp, self);
			sendSystemMessageProse(medic, pp);
			
			utils.removeScriptVar(self, "reviveSUI.weak");
			utils.removeScriptVar(self, "reviveSUI.medic");
			utils.removeScriptVar(self, "reviveSUI.pid");
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int handlePlayerRevive(obj_id self, dictionary params) throws InterruptedException
	{
		if (!utils.hasScriptVar(self, pclib.VAR_SUI_CLONE))
		{
			return SCRIPT_CONTINUE;
		}
		
		utils.removeScriptVar(self, pclib.VAR_SUI_CLONE);
		
		location clone = null;
		location spawn = null;
		int damage = cloninglib.CLONE_DAMAGE_HIGH;
		int idx = -1;
		
		int delayedClone = 0;
		
		if (params != null)
		{
			idx = sui.getListboxSelectedRow(params);
			
			if (idx > -1)
			{
				location[] cloneLocs = utils.getLocationArrayScriptVar(self, pclib.VAR_REVIVE_CLONE);
				location[] spawnLocs = utils.getLocationArrayScriptVar(self, pclib.VAR_REVIVE_SPAWN);
				
				clone = cloneLocs[idx];
				spawn = spawnLocs[idx];
				
				location deathLoc = getLocation(self);
				
				region[] respawnRegions = getRegionsWithPvPAtPoint(deathLoc, regions.PVP_REGION_TYPE_ADVANCED);
				
				if ((respawnRegions != null && respawnRegions.length > 0) || utils.hasScriptVar(self, "battlefield.active"))
				{
					delayedClone = 15;
					utils.setScriptVar(self, "no_cloning_sickness", 1);
				}
			}
			else
			{
				sendSystemMessage(self, SID_CLONE_FAIL_SELECTION);
			}
		}
		
		if ((clone != null && clone.compareTo(new location()) != 0 && spawn != null && spawn.compareTo(new location()) != 0))
		{
			if (delayedClone > 0)
			{
				int gameTime = getGameTime();
				int timeRemaining = delayedClone - (gameTime % delayedClone);
				
				int pid = sui.countdownTimerSUI(self, self, "waitingToClone", new string_id("gcw", "static_base_waiting_to_clone"), (delayedClone - timeRemaining), delayedClone, "handleDelayedCloneCancel");
				
				if (pid > -1)
				{
					dictionary d = new dictionary();
					d.put("clone", clone);
					d.put("spawn", spawn);
					d.put("damage", damage);
					d.put("id", pid);
					
					messageTo(self, "handleDelayedClone", d, timeRemaining, true);
					
					utils.setScriptVar(self, "waiting_to_clone", pid);
					
					return SCRIPT_CONTINUE;
				}
			}
			else if (pclib.playerRevive(self, clone, spawn, 0))
			{
				return SCRIPT_CONTINUE;
			}
		}
		
		messageTo(self, pclib.HANDLER_PLAYER_DEATH, null, 0, false);
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int handleDelayedClone(obj_id self, dictionary params) throws InterruptedException
	{
		if (!utils.hasScriptVar(self, "waiting_to_clone"))
		{
			return SCRIPT_CONTINUE;
		}
		
		int pid = utils.getIntScriptVar(self, "waiting_to_clone");
		
		int id = params.getInt("id");
		if (pid != id)
		{
			return SCRIPT_CONTINUE;
		}
		
		forceCloseSUIPage(pid);
		
		location clone = params.getLocation("clone");
		location spawn = params.getLocation("spawn");
		int damage = params.getInt("damage");
		
		if ((clone != null && clone.compareTo(new location()) != 0 && spawn != null && spawn.compareTo(new location()) != 0))
		{
			pclib.playerRevive(self, clone, spawn, damage);
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int handleDelayedCloneCancel(obj_id self, dictionary params) throws InterruptedException
	{
		if (!utils.hasScriptVar(self, "waiting_to_clone"))
		{
			return SCRIPT_CONTINUE;
		}
		
		utils.removeScriptVar(self, "waiting_to_clone");
		
		messageTo(self, pclib.HANDLER_PLAYER_DEATH, null, 0.0f, true);
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int handleCloneRespawn(obj_id self, dictionary params) throws InterruptedException
	{
		
		utils.removeScriptVar(self, pclib.VAR_SUI_CLONE);
		utils.removeScriptVar(self, pclib.VAR_REVIVE_OPTIONS);
		utils.removeScriptVar(self, pclib.VAR_REVIVE_CLONE);
		utils.removeScriptVar(self, pclib.VAR_REVIVE_SPAWN);
		
		utils.removeScriptVar(self, "waitingOnCloneRespawn");
		
		removeObjVar(self, pclib.VAR_BEEN_COUPDEGRACED);
		
		pvpRemoveAllTempEnemyFlags(self);
		
		int intGuildId = getGuildId(self);
		if (intGuildId != 0)
		{
			int[] enemies_A_to_B = guildGetEnemies(intGuildId);
			if (enemies_A_to_B != null && enemies_A_to_B.length > 0)
			{
				int[] enemies_B_to_A = getGuildsAtWarWith(intGuildId);
				if (enemies_B_to_A != null && enemies_B_to_A.length > 0)
				{
					for (int i = 0; i < enemies_A_to_B.length; ++i)
					{
						testAbortScript();
						if (guild.findIntTableOffset(enemies_B_to_A, enemies_A_to_B[i]) != -1)
						{
							pvpSetGuildWarCoolDownPeriodEnemyFlag(self);
							break;
						}
					}
				}
			}
		}
		
		pclib.checkCovertRespawn(self);
		
		pclib.clearEffectsForDeath(self);
		
		utils.removeScriptVar(self, "jedi.rankedDeath");
		
		if (hasObjVar(self, "fullHealClone"))
		{
			removeObjVar(self, "fullHealClone");
			healing.healClone(self, false);
		}
		else
		{
			healing.healClone(self, true);
		}
		
		setPosture(self, POSTURE_UPRIGHT);
		utils.removeScriptVar(self, "pvp_death");
		
		queueCommand(self, (-1465754503), self, "", COMMAND_PRIORITY_IMMEDIATE);
		
		playClientEffectObj(self, "clienteffect/player_clone_compile.cef", self, null);
		
		if (!utils.hasScriptVar(self, "no_cloning_sickness") && !instance.isInInstanceArea(self))
		{
			buff.applyBuff(self, "cloning_sickness");
		}
		else if (utils.hasScriptVar(self, "no_cloning_sickness"))
		{
			utils.removeScriptVar(self, "no_cloning_sickness");
		}
		
		if (0 == pvpGetAlignedFaction(self))
		{
			int currentMercenaryFaction = pvpNeutralGetMercenaryFaction(self);
			if ((0 != currentMercenaryFaction) && pvpNeutralIsMercenaryDeclared(self))
			{
				pvpNeutralSetMercenaryFaction(self, currentMercenaryFaction, false);
			}
		}
		
		CustomerServiceLog("Death", "("+ self + ") "+ getName(self) + " has clone respawned at "+ getLocation(self).toString());
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int handlePlayerResuscitated(obj_id self, dictionary params) throws InterruptedException
	{
		int suiClone = utils.getIntScriptVar(self, pclib.VAR_SUI_CLONE);
		LOG("resuscitate","self = "+ self + ": clone sui = "+ suiClone);
		if (suiClone > -1)
		{
			LOG("resuscitate","attempting to force closure of clone sui: sui #"+ suiClone);
			forceCloseSUIPage(suiClone);
		}
		
		utils.removeScriptVar(self, pclib.VAR_SUI_CLONE);
		utils.removeScriptVar(self, pclib.VAR_REVIVE_OPTIONS);
		pvpRemoveAllTempEnemyFlags(self);
		
		removeObjVar(self, pclib.VAR_BEEN_COUPDEGRACED);
		
		obj_id medic = utils.getObjIdScriptVar(self, "reviveSUI.medic");
		utils.removeScriptVar(self, "reviveSUI.medic");
		
		if (isIdValid(medic))
		{
			prose_package pp = new prose_package();
			pp = prose.setStringId(pp, healing.SID_PERFORM_REVIVE_SUCCESS);
			pp = prose.setTT(pp, medic);
			pp = prose.setTO(pp, self);
			sendSystemMessageProse(medic, pp);
			sendSystemMessageProse(self, pp);
		}
		else
		{
			sendSystemMessage(self, new string_id("healing", "perform_revive_success_self_only"));
		}
		
		if (hasObjVar(self, "outbreak.usedGate"))
		{
			buff.applyBuff(self, "death_troopers_no_vehicle");
		}
		
		setPosture(self, POSTURE_UPRIGHT);
		utils.removeScriptVar(self, "pvp_death");
		
		CustomerServiceLog("Death", "("+ self + ") "+ getName(self) + " has been resuscitated at "+ getLocation(self).toString());
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int handleReviveStand(obj_id self, dictionary params) throws InterruptedException
	{
		setPosture(self, POSTURE_UPRIGHT);
		return SCRIPT_CONTINUE;
	}
	
	
	public int handleInstanceRessurect(obj_id self, dictionary params) throws InterruptedException
	{
		pclib.resurrectPlayer(self, true);
		return SCRIPT_CONTINUE;
	}
	
	public int handleDecayReport(obj_id self, dictionary params) throws InterruptedException
	{
		utils.removeScriptVarTree(self, "decayReport");
		return SCRIPT_CONTINUE;
	}
	
	
	public int handleRepairItems(obj_id self, dictionary params) throws InterruptedException
	{
		obj_id[] repairList = params.getObjIdArray("items");
		cloninglib.repairItems(self, repairList);
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int handleChatPersistentMessage(obj_id self, dictionary params) throws InterruptedException
	{
		if (params == null)
		{
			return SCRIPT_OVERRIDE;
		}
		
		String from = params.getString("from");
		if (from == null)
		{
			return SCRIPT_OVERRIDE;
		}
		
		String subject = params.getString("subject");
		if (subject == null)
		{
			return SCRIPT_OVERRIDE;
		}
		
		String message = params.getString("message");
		if (message == null)
		{
			return SCRIPT_OVERRIDE;
		}
		
		String outOfBand = params.getString("outofband");
		
		String avatarName = getChatName(self);
		if (avatarName == null)
		{
			return SCRIPT_OVERRIDE;
		}
		
		chatSendPersistentMessage(from, avatarName, subject, message, outOfBand);
		return SCRIPT_CONTINUE;
	}
	
	
	public int gmRevive(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		target = self;
		params = toLower(params);
		if (params.equals("-target"))
		{
			target = getLookAtTarget(self);
		}
		
		if (isIdValid(target))
		{
			obj_id targetCreature = null;
			obj_id targetShip = null;
			int got = getGameObjectType(target);
			if (isGameObjectTypeOf(got, GOT_ship))
			{
				targetShip = target;
				targetCreature = getShipPilot(target);
			}
			else if (isGameObjectTypeOf(got, GOT_creature))
			{
				targetShip = getPilotedShip(target);
				targetCreature = target;
			}
			
			if (!isIdValid(targetShip) && !isIdValid(targetCreature))
			{
				sendSystemMessageTestingOnly(self, "The /gmRevive command may only be used on creature or ship type objects");
				return SCRIPT_CONTINUE;
			}
			
			if (isIdValid(targetShip))
			{
				
				space_crafting.repairDamage(targetCreature, targetShip, 1.f, 0.f);
				
				if (isShipSlotInstalled(targetShip, ship_chassis_slot_type.SCST_shield_0))
				{
					setShipShieldHitpointsFrontCurrent(targetShip, getShipShieldHitpointsFrontMaximum(targetShip));
					setShipShieldHitpointsBackCurrent(targetShip, getShipShieldHitpointsBackMaximum(targetShip));
				}
				
				if (isIdValid(targetCreature))
				{
					sendSystemMessageTestingOnly(targetCreature, "Ship Repaired.");
				}
				
				CustomerServiceLog("Death", "("+ targetShip + ") "+ getName(targetShip) + " has been /gmRevive'd at "+ getLocation(self).toString() + " by ("+ self + ") "+ getName(self));
			}
			
			if (isIdValid(targetCreature))
			{
				
				obj_id mountId = getMountId(targetCreature);
				if (isIdValid(mountId))
				{
					utils.dismountRiderJetpackCheck(targetCreature);
				}
				
				if (isJedi(targetCreature))
				{
					setForcePower(targetCreature, getMaxForcePower(targetCreature));
					sendSystemMessageTestingOnly(targetCreature, "Force Power Maxed");
				}
				if (pclib.resurrectPlayer(targetCreature))
				{
					sendSystemMessageTestingOnly(targetCreature, "You have been restored...");
					
					if (isPlayer(targetCreature))
					{
						pclib.clearCombatData(targetCreature);
						buff.removeAllBuffs(targetCreature, true);
						
						removeObjVar(targetCreature, "combat.intIncapacitationCount");
						
						setPosture(targetCreature, POSTURE_UPRIGHT);
						queueCommand(targetCreature, (-1465754503), targetCreature, "", COMMAND_PRIORITY_IMMEDIATE);
						queueCommand(targetCreature, (-562996732), targetCreature, "", COMMAND_PRIORITY_IMMEDIATE);
						utils.removeScriptVar(targetCreature, "pvp_death");
						
						sendSystemMessageTestingOnly(self, "You have /gmRevived "+getName(targetCreature));
					}
				}
			}
			
			CustomerServiceLog("Death", "("+ targetCreature + ") "+ getName(targetCreature) + " has been /gmRevive'd at "+ getLocation(self).toString() + " by ("+ self + ") "+ getName(self));
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int cmdEat(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!isIdValid(target) && exists(target))
		{
			return SCRIPT_OVERRIDE;
		}
		
		consumable.consumeItem(self, target);
		return SCRIPT_CONTINUE;
	}
	
	
	public int cmdUseSkillBuff(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		LOG("skillBuff", "cmdUseSkillBuff");
		if (!isIdValid(target) && exists(target))
		{
			return SCRIPT_OVERRIDE;
		}
		
		dictionary childparams = new dictionary();
		childparams.put("player", self);
		messageTo( target, "handleUseSkillBuff", childparams, 1f, true );
		return SCRIPT_CONTINUE;
	}
	
	
	public int cmdTip(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		
		if ((params == null) || (params.equals("")))
		{
			showTipSyntax(self);
			return SCRIPT_CONTINUE;
		}
		
		location here = getLocation(self);
		if ((here.area).equals("tutorial"))
		{
			sendSystemMessageTestingOnly( self, "You cannot use this command from within the tutorial.");
			return SCRIPT_CONTINUE;
		}
		
		CustomerServiceLog( "Trade", "Tip Invoked -- Self: "+ self + " Target: "+ target + " Params: "+ params );
		
		if (params.equals("clear"))
		{
			CustomerServiceLog( "Trade", " Tip -- Self: "+ self + " "+ getName(self) + " -- User requested tip data clear.");
			sendSystemMessage( self, pclib.SID_TIP_CLEAR );
			pclib.cleanupTipSui( self );
			
			return SCRIPT_CONTINUE;
		}
		
		if (utils.hasScriptVar(self, pclib.VAR_TIP_SUI))
		{
			int suiConfirm = utils.getIntScriptVar( self, pclib.VAR_TIP_SUI );
			forceCloseSUIPage( suiConfirm );
			
			obj_id oldTarget = utils.getObjIdScriptVar( self, pclib.VAR_TIP_TARGET );
			if (isIdValid(oldTarget) && exists(oldTarget) && oldTarget.isLoaded())
			{
				prose_package tipAbort = prose.getPackage( pclib.PROSE_TIP_ABORT, oldTarget );
				sendSystemMessageProse( self, tipAbort );
			}
			else
			{
				sendSystemMessage( self, pclib.SID_TIP_ABORT );
			}
			
			CustomerServiceLog( "Trade", " Tip -- Self: "+ self + " "+ getName(self) + " -- Cancelled prior tip due to new tip attempt.");
			pclib.cleanupTipSui( self );
		}
		
		if (target == self)
		{
			CustomerServiceLog( "Trade", " Tip -- Self: "+ self + " "+ getName(self) + " -- Tip aborted: (target == self)");
			sendSystemMessageTestingOnly( self, "You can't use yourself as a target for /tip!");
			return SCRIPT_CONTINUE;
		}
		
		boolean useCash = true;
		if (params.indexOf("bank") > -1)
		{
			params = gm.removeKeyword( params, "bank");
			useCash = false;
		}
		
		java.util.StringTokenizer st = new java.util.StringTokenizer(params);
		int amt = 0;
		String name = null;
		int cnt = st.countTokens();
		switch ( cnt )
		{
			case 1:
			if (!isIdValid(target))
			{
				showTipSyntax(self);
				return SCRIPT_CONTINUE;
			}
			break;
			
			case 2:
			name = st.nextToken();
			break;
			
			default:
			showTipSyntax(self);
			return SCRIPT_CONTINUE;
		}
		boolean areMoreTokens = st.hasMoreTokens();
		if (areMoreTokens)
		{
			String sAmt = st.nextToken();
			amt = utils.stringToInt(sAmt);
			if (amt < 1)
			{
				prose_package invalidParam = prose.getPackage( pclib.PROSE_INVALID_TIP_PARAM, sAmt );
				sendSystemMessageProse( self, invalidParam );
				return SCRIPT_CONTINUE;
			}
			
			if ((name == null) || (name.equals("")))
			{
				pclib.giveTip( self, target, getName(target), amt, useCash );
			}
			else
			{
				obj_id playerId = getPlayerIdFromFirstName( name );
				if (isIdValid(playerId))
				{
					pclib.giveTip( self, playerId, name, amt, useCash );
				}
			}
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int cmdTipFail(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		sendSystemMessageTestingOnly(self, "/TIP command failed in command table...");
		return SCRIPT_CONTINUE;
	}
	
	
	public int cmdCheckForceStatus(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int handleWireConfirm(obj_id self, dictionary params) throws InterruptedException
	{
		
		int bp = sui.getIntButtonPressed(params);
		if (bp == sui.BP_CANCEL)
		{
			CustomerServiceLog( "Trade", " Tip -- Player: "+ self + " "+ getName(self) + " -- Tip aborted: Bank-to-Bank transfer canceled by player's request.");
			pclib.cleanupTipSui( self );
			return SCRIPT_CONTINUE;
		}
		
		obj_id target = utils.getObjIdScriptVar( self, pclib.VAR_TIP_TARGET );
		if (!isIdValid(target))
		{
			sendSystemMessage( self, pclib.SID_TIP_ERROR );
			CustomerServiceLog( "Trade", " Tip -- Player: "+ self + " "+ getName(self) + " Target: "+ target + " -- Tip aborted: Target was invalid after tip fee dialog was accepted.");
			pclib.cleanupTipSui( self );
			return SCRIPT_CONTINUE;
		}
		
		int amt = utils.getIntScriptVar( self, pclib.VAR_TIP_AMT );
		int fee = (int)(amt * pclib.TIP_WIRE_SURCHARGE);
		if (fee < 1)
		{
			fee = 1;
		}
		int total = amt + fee;
		
		String targetName = utils.getStringScriptVar( self, pclib.VAR_TIP_TARGET_NAME );
		if (getBankBalance(self) < total)
		{
			
			CustomerServiceLog( "Trade", " Tip -- Player: "+ self + " "+ getName(self) + " Target: "+ target + " -- Tip aborted: Player couldn't afford the wire fee.");
			
			prose_package nsfWire = prose.getPackage( pclib.PROSE_TIP_NSF_WIRE, self, getName(self), null, target, targetName, null, null, null, null, total, 0f );
			sendSystemMessageProse( self, nsfWire );
		}
		else
		{
			dictionary d = new dictionary();
			d.put("target", target);
			d.put("amt", amt);
			d.put("useCash", false);
			d.put("targetName", targetName);
			
			CustomerServiceLog( "Trade", " Tip -- Player: "+ self + " "+ getName(self) + " Target: "+ target + " -- Transferring wire bank money to escrow account! Amt: "+ amt );
			transferBankCreditsToNamedAccount( self, money.ACCT_TIP_ESCROW, amt, "handleTipSuccess", "handleTipFail", d );
			
			if (utils.isFreeTrial(self, target))
			{
				pclib.doTipLogging(self, target, amt);
			}
			
			CustomerServiceLog( "Trade", " Tip -- Player: "+ self + " "+ getName(self) + " Target: "+ target + " -- Transferring wire bank fee to surcharge account! Amt: "+ fee );
			transferBankCreditsToNamedAccount( self, money.ACCT_TIP_SURCHARGE, fee, "noHandler", "noHandler", d );
			
			utils.moneyOutMetric( self, money.ACCT_TIP_SURCHARGE, fee );
		}
		
		pclib.cleanupTipSui( self );
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int handleTipSuccess(obj_id self, dictionary params) throws InterruptedException
	{
		String myName = getName(self);
		obj_id po = getPlayerObject(self);
		
		CustomerServiceLog( "Trade", " Tip -- Player: "+ self + " "+ getName(self) + " -- Handling a successful transfer.");
		
		obj_id target = params.getObjId("target");
		if (params == null || params.isEmpty() || !isIdValid(target))
		{
			CustomerServiceLog( "Trade", " Tip -- Player: "+ self + " "+ getName(self) + " -- Error! Can't handle successful tip transfer! The target data is invalid.");
			return SCRIPT_CONTINUE;
		}
		
		int amt = params.getInt("amt");
		boolean useCash = params.getBoolean("useCash");
		String targetName = params.getString("targetName");
		params.put( "actor", self );
		params.put( "actorName", myName );
		obj_id tpo = getPlayerObject( target );
		
		if (useCash)
		{
			
			prose_package toSelf = prose.getPackage( pclib.PROSE_TIP_PASS_SELF, target, amt );
			sendSystemMessageProse( self, toSelf );
			prose_package toTarget = prose.getPackage( pclib.PROSE_TIP_PASS_TARGET, self, amt );
			sendSystemMessageProse( target, toTarget );
			
			CustomerServiceLog( "Trade", " Tip -- Player: "+ self + " "+ getName(self) + " Target: "+ target + " "+ getName(target) + " -- Cash tip transfer successfully completed. Amt: "+ amt );
		}
		else
		{
			CustomerServiceLog( "Trade", " Tip -- Player: "+ self + " "+ getName(self) + " Target: "+ target + " "+ getName(target) + " -- Bank->Escrow wire transfer complete. Telling target to request escrow funds. Amt: "+ amt );
			messageTo( target, "handleTipEscrowRequest", params, 1f, true );
			prose_package bankToEscrow = prose.getPackage( new string_id( "base_player", "bank_to_escrow"), targetName, amt );
			sendSystemMessageProse( self, bankToEscrow );
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int handleTipEscrowRequest(obj_id self, dictionary params) throws InterruptedException
	{
		int amt = params.getInt("amt");
		obj_id player = params.getObjId("actor");
		String playerName = params.getString("actorName");
		CustomerServiceLog( "Trade", " Tip -- Player: "+ player + " "+ playerName + " Target: "+ self + " "+ getName(self) + " -- Target is now requesting funds from the escrow account. Amt: "+ amt );
		
		if (amt >= 1 && amt <= 1000000)
		{
			boolean success = transferBankCreditsFromNamedAccount( money.ACCT_TIP_ESCROW, self, amt, "handleTipEscrowPass", "handleTipEscrowFail", params );
			if (!success)
			{
				
				CustomerServiceLog( "Trade", " Tip -- Player: "+ player + " "+ playerName + " Target: "+ self + " -- Tip aborted: Unable to transfer money from bank to escrow. Amt: "+ amt );
				
				refundEscrow( player, playerName, self, getName(self), amt );
			}
		}
		else
		{
			CustomerServiceLog( "Trade", " Tip -- Player: "+ player + " "+ playerName + " Target: "+ self + " "+ getName(self) + " -- Target was unable to request tip funds from escrow because the amount is invalid! Amt: "+ amt );
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int handleTipEscrowPass(obj_id self, dictionary params) throws InterruptedException
	{
		obj_id actor = params.getObjId("actor");
		String actorName = params.getString("actorName");
		
		obj_id target = params.getObjId("target");
		String targetName = params.getString("targetName");
		
		if (!isIdValid(actor) || !isIdValid(target) || targetName == null || actorName == null)
		{
			CustomerServiceLog( "Trade", " Tip -- Player: "+ actor + " "+ actorName + " Target: "+ target + " "+ getName(target) + " -- Unable to handle successful transfer Escrow->Bank because of bad parameters.");
			return SCRIPT_CONTINUE;
		}
		
		int amt = params.getInt("amt");
		boolean useCash = params.getBoolean("useCash");
		
		CustomerServiceLog( "Trade", " Tip -- Player: "+ actor + " "+ actorName + " Target: "+ target + " "+ getName(target) + " -- Target successfully withdrew the money from escrow. Wire transfer complete. Amt: "+ amt );
		
		prose_package bodyToTarget = prose.getPackage( pclib.PROSE_WIRE_MAIL_TARGET, actorName, amt );
		utils.sendMail( pclib.SID_WIRE_MAIL_SUBJECT, bodyToTarget, targetName, "@money/acct_n:bank");
		prose_package bodyToSelf = prose.getPackage( pclib.PROSE_WIRE_MAIL_SELF, targetName, amt );
		utils.sendMail( pclib.SID_WIRE_MAIL_SUBJECT, bodyToSelf, actorName, targetName );
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int handleTipEscrowFail(obj_id self, dictionary params) throws InterruptedException
	{
		obj_id actor = params.getObjId("actor");
		String actorName = params.getString("actorName");
		
		obj_id target = params.getObjId("target");
		String targetName = params.getString("targetName");
		
		if (!isIdValid(actor) || !isIdValid(target) || targetName == null || actorName == null)
		{
			CustomerServiceLog( "Trade", " Tip -- Player: "+ actor + " "+ actorName + " Target: "+ target + " "+ getName(target) + " -- Unable to handle FAILED transfer Escrow->Bank because of bad parameters.");
			return SCRIPT_CONTINUE;
		}
		
		int amt = params.getInt("amt");
		boolean useCash = params.getBoolean("useCash");
		
		CustomerServiceLog( "Trade", " Tip -- Player: "+ actor + " "+ actorName + " Target: "+ target + " "+ getName(target) + " -- Target failed to withdraw the money from escrow! Wire transfer FAILED! Attempting to refund. Amt: "+ amt );
		
		prose_package escrowFailed = prose.getPackage( new string_id("base_player", "escrow_withdraw_failed"), actorName, amt );
		utils.sendMail( pclib.SID_WIRE_MAIL_SUBJECT, escrowFailed, targetName, actorName );
		
		refundEscrow( actor, actorName, target, targetName, amt );
		
		return SCRIPT_CONTINUE;
	}
	
	
	public void refundEscrow(obj_id player, String playerName, obj_id target, String targetName, int amt) throws InterruptedException
	{
		
		dictionary params = new dictionary();
		params.put( "player", player );
		params.put( "playerName", playerName );
		params.put( "target", target );
		params.put( "targetName", targetName );
		params.put( "amt", amt );
		messageTo( player, "handleTipRefundRequest", params, 1f, true );
	}
	
	
	public int handleTipRefundRequest(obj_id self, dictionary params) throws InterruptedException
	{
		obj_id player = params.getObjId( "player");
		String playerName = params.getString( "playerName");
		obj_id target = params.getObjId( "target");
		String targetName = params.getString( "targetName");
		int amt = params.getInt( "amt");
		
		if (amt < 1 || amt > 1000000)
		{
			CustomerServiceLog( "Trade", " Tip -- Player: "+ player + " "+ playerName + " Target: "+ target + " "+ targetName + " -- Unable to refund escrow because the amount is invalid! Amt:"+ amt );
			return SCRIPT_CONTINUE;
		}
		
		boolean success = transferBankCreditsFromNamedAccount( money.ACCT_TIP_ESCROW, player, amt, "handleTipRefundPass", "handleTipRefundFail", params );
		CustomerServiceLog( "Trade", " Tip -- Player: "+ player + " "+ playerName + " Target: "+ target + " "+ targetName + " -- Calling transferBankCreditsFromNamedAccount to transfer refund from escrow. Amt:"+ amt );
		if (!success)
		{
			
			CustomerServiceLog( "Trade", " Tip -- Player: "+ player + " "+ playerName + " Target: "+ target + " "+ targetName + " -- transferBankCreditsFromNamedAccount failed immediately while attempting to refund escrow! Amt:"+ amt );
			notifyFailedRefund( player, playerName, target, targetName, amt );
			return SCRIPT_CONTINUE;
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int handleTipRefundPass(obj_id self, dictionary params) throws InterruptedException
	{
		obj_id player = params.getObjId( "player");
		String playerName = params.getString( "playerName");
		obj_id target = params.getObjId( "target");
		String targetName = params.getString( "targetName");
		int amt = params.getInt( "amt");
		
		prose_package refundToSelf = prose.getPackage( new string_id("base_player","received_escrow_refund"), targetName, amt );
		utils.sendMail( pclib.SID_WIRE_MAIL_SUBJECT, refundToSelf, playerName, targetName );
		CustomerServiceLog( "Trade", " Tip -- Player: "+ player + " "+ playerName + " Target: "+ target + " "+ targetName + " -- Refund successfully delivered. Amt:"+ amt );
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int handleTipRefundFail(obj_id self, dictionary params) throws InterruptedException
	{
		obj_id player = params.getObjId( "player");
		String playerName = params.getString( "playerName");
		obj_id target = params.getObjId( "target");
		String targetName = params.getString( "targetName");
		int amt = params.getInt( "amt");
		
		notifyFailedRefund( player, playerName, target, targetName, amt );
		
		return SCRIPT_CONTINUE;
	}
	
	
	public void notifyFailedRefund(obj_id player, String playerName, obj_id target, String targetName, int amt) throws InterruptedException
	{
		CustomerServiceLog( "Trade", " Tip -- Player: "+ player + " "+ playerName + " Target: "+ target + " "+ targetName + " -- Failed to refund player from escrow account! Amt:"+ amt );
		prose_package refundToSelf = prose.getPackage( new string_id("base_player","failed_escrow_refund"), targetName, amt );
		utils.sendMail( pclib.SID_WIRE_MAIL_SUBJECT, refundToSelf, playerName, targetName );
	}
	
	
	public int handleTipFail(obj_id self, dictionary params) throws InterruptedException
	{
		CustomerServiceLog( "Trade", " Tip -- Player: "+ self + " "+ getName(self) + " -- Tip cash transfer or bank wire escrow transfer failed!");
		
		utils.removeScriptVar( self, pclib.VAR_TIP_SUI );
		utils.removeScriptVar( self, pclib.VAR_TIP_TARGET );
		utils.removeScriptVar( self, pclib.VAR_TIP_AMT );
		
		obj_id target = params.getObjId("target");
		if (!isIdValid(target))
		{
			return SCRIPT_CONTINUE;
		}
		
		if (!exists(target) || !target.isLoaded())
		{
			sendSystemMessage( self, pclib.SID_TIP_ERROR );
			return SCRIPT_CONTINUE;
		}
		
		int amt = params.getInt("amt");
		boolean useCash = params.getBoolean("useCash");
		if (useCash)
		{
			CustomerServiceLog( "Trade", " Tip -- Player: "+ self + " "+ getName(self) + " Target: "+ target + " "+ getName(target) + " -- Cash transfer failed in transferCashTo! Amt: "+ amt );
			prose_package nsfCash = prose.getPackage( pclib.PROSE_TIP_NSF_CASH, target, amt );
			sendSystemMessageProse( self, nsfCash );
		}
		else
		{
			CustomerServiceLog( "Trade", " Tip -- Player: "+ self + " "+ getName(self) + " Target: "+ target + " "+ getName(target) + " -- Bank->Escrow transfer failed in transferBankCreditsToNamedAccount! Amt: "+ amt );
			prose_package nsfBank = prose.getPackage( pclib.PROSE_TIP_NSF_BANK, target, amt );
			sendSystemMessageProse( self, nsfBank );
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public void showTipSyntax(obj_id self) throws InterruptedException
	{
		sendSystemMessage(self, pclib.SID_TIP_SYNTAX);
	}
	
	
	public int cmdOpenFail(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!isIdValid(target))
		{
			return SCRIPT_OVERRIDE;
		}
		
		prose_package pp = prose.getPackage(SID_CMD_OPEN_FAIL, target);
		sendSystemMessageProse(self, pp);
		return SCRIPT_CONTINUE;
	}
	
	
	public int cmdLoot(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		obj_id player = self;
		
		if (!isIdValid(target))
		{
			return SCRIPT_CONTINUE;
		}
		
		if (!isMob(target))
		{
			return SCRIPT_CONTINUE;
		}
		
		if (!isIncapacitated(target))
		{
			return SCRIPT_CONTINUE;
		}
		
		if (isIncapacitated(self))
		{
			return SCRIPT_CONTINUE;
		}
		
		if (pet_lib.isPet (target))
		{
			return SCRIPT_CONTINUE;
		}
		
		int got = getGameObjectType(target);
		if (got == GOT_corpse)
		{
			queueCommand(self, (1421655546), target, "", COMMAND_PRIORITY_DEFAULT);
			return SCRIPT_CONTINUE;
		}
		
		if (isMob(target) && !isPlayer (target))
		{
			if (group.isGrouped(self))
			{
				obj_id team = getGroupObject (self);
				int lootRule = getGroupLootRule(team);
				
				if (lootRule == 3)
				{
					if (hasObjVar (target, "autoLootComplete" ))
					{
						obj_id corpseInv = utils.getInventoryContainer(target);
						queueCommand(player, (1880585606), corpseInv, "", COMMAND_PRIORITY_DEFAULT);
					}
					
					obj_id[] objMembersWhoExist = utils.getLocalGroupMemberIds(team);
					int teamNumber = objMembersWhoExist.length;
					
					if (teamNumber > 0)
					{
						int which = rand(0, teamNumber-1);
						player = objMembersWhoExist[which];
					}
				}
				
				if (loot.doGroupLootAllCheck(self, target))
				{
					loot.lootAiCorpse(player, target);
				}
				return SCRIPT_CONTINUE;
			}
			loot.lootAiCorpse(self, target);
			return SCRIPT_CONTINUE;
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int cmdLootPlayer(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!isIdValid(target))
		{
			return SCRIPT_OVERRIDE;
		}
		
		if (!utils.isOwner(target, self) && !isGod(self))
		{
			return SCRIPT_OVERRIDE;
		}
		
		int got = getGameObjectType(target);
		if (got != GOT_corpse)
		{
			return SCRIPT_OVERRIDE;
		}
		
		corpse.lootPlayerCorpse(self, target);
		return SCRIPT_CONTINUE;
	}
	
	
	public int cmdActivateClone(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (params.equals("now"))
		{
			
			if (isGod(self))
			{
				if (utils.hasScriptVar(self, pclib.VAR_SUI_CLONE))
				{
					int cloneSui = utils.getIntScriptVar(self, pclib.VAR_SUI_CLONE);
					utils.removeScriptVar(self, pclib.VAR_SUI_CLONE);
					forceCloseSUIPage(cloneSui);
				}
				
				if (!pclib.playerRevive(self))
				{
					
					sendSystemMessageTestingOnly(self, "You may only use this command to force a clone spawn if you are dead.");
				}
				
				return SCRIPT_CONTINUE;
			}
		}
		
		if (utils.hasScriptVar(self, pclib.VAR_SUI_CLONE))
		{
			sendSystemMessageTestingOnly(self, "You already have a cloning options menu open!");
			return SCRIPT_CONTINUE;
		}
		
		if (!isDead(self))
		{
			return SCRIPT_CONTINUE;
		}
		
		messageTo(self, pclib.HANDLER_PLAYER_DEATH, null, 0, false);
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int cmdActivateCloneFail(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		sendSystemMessageTestingOnly(self, "You may only use this command to activate the cloning options menu if you are dead.");
		return SCRIPT_CONTINUE;
	}
	
	
	public int cmdSetBind(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		
		if (!isIdValid(target))
		{
			return SCRIPT_CONTINUE;
		}
		
		int got = getGameObjectType(target);
		if (got != GOT_terminal_cloning)
		{
			return SCRIPT_CONTINUE;
		}
		
		cloninglib.requestBind(self, target);
		return SCRIPT_CONTINUE;
	}
	
	
	public boolean canDeathBlow(obj_id killer, obj_id victim) throws InterruptedException
	{
		
		if ((!isIdValid(victim)) || (!isPlayer(victim)))
		{
			debugServerConsoleMsg(killer, "cmdCoupDeGrace: ("+ victim + ")target is not player or is not incapacitated!!");
			combat.sendCombatSpamMessage(killer, new string_id("cbt_spam", "invalid_target"));
			showFlyTextPrivate(killer, killer, new string_id("combat_effects", "target_invalid_fly"), 1.5f, colors.WHITE);
			return false;
		}
		
		if (!pvpCanAttack(killer, victim))
		{
			showFlyTextPrivate(killer, killer, new string_id("combat_effects", "target_invalid_fly"), 1.5f, colors.WHITE);
			combat.sendCombatSpamMessage(killer, new string_id("cbt_spam", "invalid_pvp_target"));
			return false;
		}
		
		if (isIncapacitated(victim))
		{
			if (hasObjVar(victim, pclib.VAR_BEEN_COUPDEGRACED))
			{
				combat.sendCombatSpamMessage(killer, new string_id("cbt_spam", "invalid_target"));
				showFlyTextPrivate(killer, killer, new string_id("combat_effects", "target_invalid_fly"), 1.5f, colors.WHITE);
				return false;
			}
			location myPos = getLocation(killer);
			location targetPos = getLocation(victim);
			if (myPos != null && targetPos != null)
			{
				float distance = myPos.distance(targetPos);
				float maxDistance = getCommandMaxRange((1917386623));
				debugServerConsoleMsg(null, "Deathblow distance to target ");
				if (distance < maxDistance)
				{
					
					stealth.checkForAndMakeVisibleNoRecourse(killer);
					
					if (isIncapacitated(killer) || isDead(killer))
					{
						return false;
					}
					
					if (getState(killer, STATE_FEIGN_DEATH) == 1)
					{
						return false;
					}
					
					int killerPosture = getPosture( killer );
					if (killerPosture == POSTURE_KNOCKED_DOWN)
					{
						return false;
					}
					
					return true;
				}
				else
				{
					showFlyTextPrivate(killer, killer, new string_id("combat_effects", "range_too_far"), 1.5f, colors.MEDIUMTURQUOISE);
					combat.sendCombatSpamMessage(killer, new string_id("cbt_spam", "out_of_range_far"), COMBAT_RESULT_OUT_OF_RANGE);
					
					CustomerServiceLog("Pvp", "Player %TU received deathblow message for target %TT, but has moved too "+
					"far away ("+ distance + ")", killer, victim);
				}
			}
			else
			{
				combat.sendCombatSpamMessage(killer, new string_id("cbt_spam", "invalid_target"));
				
				showFlyTextPrivate(killer, killer, new string_id("combat_effects", "target_invalid_fly"), 1.5f, colors.WHITE);
				CustomerServiceLog("Pvp", "Player %TU received deathblow message for target %TT, but we could "+
				"not get the player's positions", killer, victim);
			}
		}
		else
		{
			combat.sendCombatSpamMessage(killer, new string_id("cbt_spam", "invalid_target"));
			showFlyTextPrivate(killer, killer, new string_id("combat_effects", "target_invalid_fly"), 1.5f, colors.WHITE);
		}
		
		return false;
	}
	
	
	public int cmdCoupDeGrace(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (canDeathBlow(self, target))
		{
			pclib.coupDeGrace(target, self);
		}
		else
		{
			return SCRIPT_OVERRIDE;
		}
		return SCRIPT_CONTINUE;
	}
	
	
	public int handleCoupDeGrace(obj_id self, dictionary params) throws InterruptedException
	{
		if (utils.hasScriptVar(self, COUPE_DE_GRACE_TARGET))
		{
			obj_id target = utils.getObjIdScriptVar(self, COUPE_DE_GRACE_TARGET);
			utils.removeScriptVar(self, COUPE_DE_GRACE_TARGET);
			debugServerConsoleMsg(null, "Performing deathblow on "+ target);
			if (isIdValid(target))
			{
				
				if (isIncapacitated(target))
				{
					
					location myPos = getLocation(self);
					location targetPos = getLocation(target);
					if (myPos != null && targetPos != null)
					{
						float distance = myPos.distance(targetPos);
						float maxDistance = getCommandMaxRange((1917386623));
						debugServerConsoleMsg(null, "Deathblow distance to target ");
						if (distance < maxDistance)
						{
							CustomerServiceLog("Pvp", "Player %TU received deathblow message for target %TT, and is performing the blow");
							pclib.coupDeGrace(target, self);
						}
						else
						{
							
							CustomerServiceLog("Pvp", "Player %TU received deathblow message for target %TT, but has moved too "+
							"far away ("+ distance + ")", self, target);
						}
					}
					else
					{
						CustomerServiceLog("Pvp", "Player %TU received deathblow message for target %TT, but we could "+
						"not get the player's positions", self, target);
					}
				}
				else
				{
					CustomerServiceLog("Pvp", "Player %TU received deathblow message, but target %TT is no longer incapped ", self, target);
				}
			}
			else
			{
				CustomerServiceLog("Pvp", "Player %TU received deathblow message, but has invalid deathblow scriptvar "+ target, self);
			}
		}
		else
		{
			CustomerServiceLog("Pvp", "Player %TU received deathblow message, but has no deathblow scriptvar ", self);
		}
		if (!isIncapacitated(self))
		{
			setCount(self, 0);
		}
		setState(self, STATE_PERFORM_DEATHBLOW, false);
		return SCRIPT_CONTINUE;
	}
	
	
	public int cmdCorpse(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		queueCommand(self, (-304150016), target, "", COMMAND_PRIORITY_DEFAULT);
		
		return SCRIPT_CONTINUE;
		
	}
	
	
	public int cmdConsent(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		
		if (params.equals(""))
		{
			target = getLookAtTarget(self);
			
			if (isIdValid(target) && isPlayer(target))
			{
				pclib.consent(self, target);
			}
			else
			{
				if (hasObjVar(self, pclib.VAR_CONSENT_TO_ID))
				{
					obj_id[] consentTo = getObjIdArrayObjVar(self, pclib.VAR_CONSENT_TO_ID);
					if ((consentTo == null) || (consentTo.length == 0))
					{
						sendSystemMessage(self, SID_CONSENT_TO_EMPTY);
						return SCRIPT_OVERRIDE;
					}
					
					Vector consentToName = new Vector();
					consentToName.setSize(0);
					for (int i = 0; i < consentTo.length; ++i)
					{
						testAbortScript();
						consentToName = utils.addElement(consentToName, getPlayerFullName(consentTo[i]));
					}
					
					sui.listbox(self, "@"+ SID_CONSENT_TO_LISTBOX_PROMPT, "@"+ SID_CONSENT_TO_LISTBOX_TITLE, consentToName);
					
					return SCRIPT_CONTINUE;
				}
				sendSystemMessage(self, SID_CONSENT_TO_EMPTY);
				return SCRIPT_CONTINUE;
			}
		}
		else
		{
			
			if (params.startsWith(">"))
			{
				java.util.StringTokenizer st = new java.util.StringTokenizer(params);
				params = params.substring(1);
			}
			
			if (params.indexOf(",") > 0)
			{
				
				java.util.StringTokenizer st = new java.util.StringTokenizer(params);
				do
				{
					testAbortScript();
					String arg = st.nextToken();
					if (arg.startsWith(">"))
					{
						arg = arg.substring(1);
					}
					
					obj_id playerId = getPlayerIdFromFirstName(arg);
					if (isIdValid(playerId))
					{
						pclib.consent(self, playerId);
					}
					
				}
				while (st.hasMoreTokens());
			}
			else
			{
				obj_id playerId = getPlayerIdFromFirstName(params);
				if (isIdValid(playerId))
				{
					pclib.consent(self, playerId);
				}
			}
		}
		return SCRIPT_CONTINUE;
	}
	
	
	public int cmdUnconsent(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		
		if (params.equals(""))
		{
			target = getLookAtTarget(self);
			
			if (isIdValid(target) && isPlayer(target))
			{
				pclib.unconsent(self, target);
			}
			else
			{
				if (hasObjVar(self, pclib.VAR_CONSENT_TO_ID))
				{
					obj_id[] consentTo = getObjIdArrayObjVar(self, pclib.VAR_CONSENT_TO_ID);
					if ((consentTo == null) || (consentTo.length == 0))
					{
						sendSystemMessage(self, SID_CONSENT_TO_EMPTY);
						return SCRIPT_OVERRIDE;
					}
					
					Vector consentToName = new Vector();
					consentToName.setSize(0);
					for (int i = 0; i < consentTo.length; ++i)
					{
						testAbortScript();
						consentToName = utils.addElement(consentToName, getPlayerFullName(consentTo[i]));
					}
					
					sui.listbox(self, self, "@"+ SID_UNCONSENT_LISTBOX_PROMPT, sui.OK_CANCEL, "@"+ SID_UNCONSENT_LISTBOX_TITLE, consentToName, HANDLER_UNCONSENT_MENU);
					
					return SCRIPT_CONTINUE;
				}
				sendSystemMessage(self, SID_CONSENT_TO_EMPTY);
				return SCRIPT_CONTINUE;
			}
		}
		else
		{
			
			if (params.startsWith(">"))
			{
				params = params.substring(1);
			}
			
			if (params.indexOf(",") > 0)
			{
				
				java.util.StringTokenizer st = new java.util.StringTokenizer(params);
				do
				{
					testAbortScript();
					String arg = st.nextToken();
					if (arg.startsWith(">"))
					{
						arg = arg.substring(1);
					}
					
					obj_id playerId = getPlayerIdFromFirstName(arg);
					if (isIdValid(playerId))
					{
						pclib.unconsent(self, playerId);
					}
					
				}
				while (st.hasMoreTokens());
			}
			else
			{
				obj_id playerId = getPlayerIdFromFirstName(params);
				if (isIdValid(playerId))
				{
					pclib.unconsent(self, playerId);
				}
			}
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int cmdHaveConsent(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		
		if (params.equals(""))
		{
			target = getLookAtTarget(self);
			
			if (isIdValid(target) && isPlayer(target))
			{
				pclib.hasConsent(self, target, true);
			}
			else
			{
				if (hasObjVar(self, pclib.VAR_CONSENT_FROM_ID))
				{
					obj_id[] consentFrom = getObjIdArrayObjVar(self, pclib.VAR_CONSENT_FROM_ID);
					if ((consentFrom == null) || (consentFrom.length == 0))
					{
						sendSystemMessage(self, SID_CONSENT_TO_EMPTY);
						return SCRIPT_OVERRIDE;
					}
					
					Vector consentFromName = new Vector();
					consentFromName.setSize(0);
					for (int i = 0; i < consentFrom.length; ++i)
					{
						testAbortScript();
						consentFromName = utils.addElement(consentFromName, getPlayerFullName(consentFrom[i]));
					}
					
					sui.listbox(self, "@"+ SID_CONSENT_FROM_LISTBOX_PROMPT, "@"+ SID_CONSENT_FROM_LISTBOX_TITLE, consentFromName);
					
					return SCRIPT_CONTINUE;
				}
				sendSystemMessage(self, SID_CONSENT_FROM_EMPTY);
				return SCRIPT_CONTINUE;
			}
		}
		else
		{
			
			if (params.startsWith(">"))
			{
				java.util.StringTokenizer st = new java.util.StringTokenizer(params);
				params = params.substring(1);
			}
			
			if (params.indexOf(",") > 0)
			{
				
				java.util.StringTokenizer st = new java.util.StringTokenizer(params);
				do
				{
					testAbortScript();
					String arg = st.nextToken();
					if (arg.startsWith(">"))
					{
						arg = arg.substring(1);
					}
					
					obj_id playerId = getPlayerIdFromFirstName(arg);
					if (isIdValid(playerId))
					{
						pclib.hasConsent(self, playerId, true);
					}
					
				}
				while (st.hasMoreTokens());
			}
			else
			{
				
				obj_id playerId = getPlayerIdFromFirstName(params);
				if (isIdValid(playerId))
				{
					pclib.hasConsent(self, playerId, true);
				}
			}
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int startTraceLogging(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		
		enableTraceLogging();
		return SCRIPT_CONTINUE;
		
	}
	
	
	public int stopTraceLogging(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		
		disableTraceLogging();
		return SCRIPT_CONTINUE;
		
	}
	
	
	public int createCreature(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		String strCreatureType = params;
		location locSpawnLocation = getLocation(self);
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int createNPC(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		String strNPCType = params;
		location locSpawnLocation = getLocation(self);
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int cmdStartLogout(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!hasScript(self, "player.player_logout"))
		{
			attachScript(self, "player.player_logout");
			dictionary d = new dictionary();
			if (isGod(self))
			{
				d.put("timeLeft", 1);
			}
			else
			{
				d.put("timeLeft", LOGOUT_TIME);
			}
			d.put("countInterval", LOGOUT_COUNT_INTERVAL);
			messageTo(self, "OnLogoutPulse", d, 0.1f, false);
		}
		return SCRIPT_CONTINUE;
	}
	
	
	public int maskscent(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		return SCRIPT_CONTINUE;
		
	}
	
	
	public int failMaskscent(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		
		sendSystemMessage( self, SID_SYS_SCENTMASK_FAIL );
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int milkCreature(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		obj_id playerCurrentMount = getMountId(self);
		
		obj_id intendedTarget = getIntendedTarget(self);
		obj_id lookAtTarget = getLookAtTarget(self);
		
		String milkType_lookAt = ai_lib.getMilkType(lookAtTarget);
		String milkType_intended = ai_lib.getMilkType(intendedTarget);
		if (milkType_lookAt != null && !milkType_lookAt.equals("none"))
		{
			target = lookAtTarget;
		}
		else if (milkType_intended != null && !milkType_intended.equals("none"))
		{
			target = intendedTarget;
		}
		
		else if (!isIdValid(lookAtTarget) && !isIdValid(intendedTarget))
		{
			sendSystemMessage(self, MILK_NO_TARGET);
			return SCRIPT_OVERRIDE;
		}
		else
		{
			sendSystemMessage(self, SID_NO_ALLOW_MILK);
			return SCRIPT_OVERRIDE;
		}
		
		if (vehicle.isVehicle(target) || (ai_lib.aiGetNiche(target) == NICHE_DROID) || (ai_lib.aiGetNiche(target) == NICHE_ANDROID))
		{
			sendSystemMessage(self, SID_NO_ALLOW_MILK);
			return SCRIPT_OVERRIDE;
		}
		if (isIdValid(playerCurrentMount))
		{
			sendSystemMessage(self, SID_CANT_MILK_MOUNTED);
			return SCRIPT_OVERRIDE;
		}
		if (pet_lib.isPet(target))
		{
			sendSystemMessage(self, SID_CANT_MILK);
			return SCRIPT_OVERRIDE;
		}
		
		float distStart = utils.getDistance2D(self, target);
		if ((distStart < -3) || (distStart > 3))
		{
			sendSystemMessage(self, MILK_TOO_FAR_START);
			return SCRIPT_OVERRIDE;
		}
		if (!ai_lib.isAiDead(target) && !hasScript(target, "ai.pet_advance") && !ai_lib.isInCombat(target))
		{
			dictionary dict = new dictionary();
			dict.put("player", self);
			messageTo(target, "attemptMilkHandler", dict, 0, false);
		}
		return SCRIPT_OVERRIDE;
	}
	
	
	public int searchLair(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		
		obj_id intendedTarget = getIntendedTarget(self);
		obj_id lookAtTarget = getLookAtTarget(self);
		
		int GOT_type_intended = getGameObjectType(intendedTarget);
		int GOT_type_look = getGameObjectType(lookAtTarget);
		if (GOT_type_intended == GOT_lair)
		{
			target = intendedTarget;
		}
		else if (GOT_type_look == GOT_lair)
		{
			target = lookAtTarget;
		}
		else
		{
			sendSystemMessage(self, LAIR_NOT_TARGETED);
			return SCRIPT_OVERRIDE;
		}
		float maxDistance = 10f;
		float distanceToLair = utils.getDistance2D(self, target);
		
		if (hasScript(target, "theme_park.dungeon.mustafar_trials.valley_battleground.battlefield_destructable"))
		{
			return SCRIPT_OVERRIDE;
		}
		
		if (distanceToLair > maxDistance)
		{
			sendSystemMessage(self, TOO_FAR_FROM_LAIR );
			return SCRIPT_OVERRIDE;
		}
		else
		{
			if (!utils.hasScriptVar( target, "lair.searched" ) && !isIncapacitated( self ))
			{
				
				collection.collectionResource(self, "egg");
				
				dictionary dict = new dictionary();
				dict.put("player", self);
				messageTo(target, "searchLair", dict, 0, false);
			}
			else
			{
				sendSystemMessage(self, SID_FOUND_NOTHING );
			}
		}
		return SCRIPT_CONTINUE;
	}
	
	
	public int removeScentMask(obj_id self, dictionary params) throws InterruptedException
	{
		
		int scentMasked = getState( self, STATE_MASK_SCENT );
		if (scentMasked == 0)
		{
			buff.removeBuff(self, "mask_scent");
			return SCRIPT_CONTINUE;
		}
		
		int scount = params.getInt( "count");
		int count = utils.getIntScriptVar( self, "scentmask.count");
		if (count != scount)
		{
			return SCRIPT_CONTINUE;
		}
		
		setState( self, STATE_MASK_SCENT, false );
		buff.removeBuff(self, "mask_scent");
		sendSystemMessage( self, SID_SYS_SCENTMASK_STOP );
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int removeScentMaskNoNotify(obj_id self, dictionary params) throws InterruptedException
	{
		return SCRIPT_CONTINUE;
		
	}
	
	
	public int concealEnable(obj_id self, dictionary params) throws InterruptedException
	{
		return SCRIPT_CONTINUE;
		
	}
	
	
	public int failConceal(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		return SCRIPT_CONTINUE;
	}
	
	
	public int removeConceal(obj_id self, dictionary params) throws InterruptedException
	{
		return SCRIPT_CONTINUE;
		
	}
	
	
	public boolean itemIsRare(obj_id self, int skillMod) throws InterruptedException
	{
		
		boolean rare = false;
		if (skillMod >= 65)
		{
			int difficultyCheck = (skillMod-50) + 30;
			int dieRoll = rand( 1, 100 );
			if (dieRoll > difficultyCheck)
			{
				
				dieRoll = rand( 1, 4 );
				if (dieRoll == 1)
				{
					rare = true;
				}
			}
		}
		return rare;
	}
	
	
	public int sysGroup(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (hasSkill( self, "class_officer_phase1_master" ))
		{
			squad_leader.sendSquadLeaderCommand( self, params );
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int groupWaypoint(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (hasSkill( self, "class_officer_phase1_master" ))
		{
			
			if (params.equals("a") || params.equals("c"))
			{
				squad_leader.sendSquadWaypoint(self, getLocation(self));
			}
			else
			{
				
				location wayLoc = combat.getCommandGroundTargetLocation(params);
				squad_leader.sendSquadWaypoint(self, wayLoc);
			}
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int createOfficerGroupWaypoint(obj_id self, dictionary params) throws InterruptedException
	{
		if (params == null || params.isEmpty())
		{
			return SCRIPT_CONTINUE;
		}
		
		location wayLoc = params.getLocation("wayLoc");
		
		obj_id groupWaypoint = createWaypointInDatapad(self, wayLoc);
		setWaypointColor(groupWaypoint, "green");
		setWaypointName(groupWaypoint, "Group Waypoint");
		setWaypointActive(groupWaypoint, true);
		
		dictionary dict = new dictionary();
		dict.put("groupWaypoint", groupWaypoint);
		
		messageTo(self, "removeOfficerGroupWaypoint", dict, 600, false);
		
		return SCRIPT_CONTINUE;
		
	}
	
	
	public int removeOfficerGroupWaypoint(obj_id self, dictionary params) throws InterruptedException
	{
		if (params == null || params.isEmpty())
		{
			return SCRIPT_CONTINUE;
		}
		
		obj_id groupWaypoint = params.getObjId("groupWaypoint");
		
		if (!isIdValid(groupWaypoint) || !exists(groupWaypoint))
		{
			return SCRIPT_CONTINUE;
		}
		
		destroyWaypointInDatapad(groupWaypoint, self);
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int OnSurveyDataReceived(obj_id self, float[] xVals, float[] zVals, float[] efficiencies) throws InterruptedException
	{
		if (efficiencies == null)
		{
			return SCRIPT_CONTINUE;
		}
		
		int highest = -1;
		float highVal = 0.f;
		for (int i=0; i<efficiencies.length; i++)
		{
			testAbortScript();
			if (efficiencies[i] > highVal)
			{
				highest = i;
				highVal = efficiencies[i];
			}
		}
		
		if ((highest > -1) && (highVal > 0.1))
		{
			
			location point = new location();
			point.x = xVals[highest];
			point.y = 0;
			point.z = zVals[highest];
			
			obj_id waypoint = getSurveyWaypoint( self );
			boolean newWpCreated = false;
			if (!isIdValid(waypoint))
			{
				waypoint = createWaypointInDatapad( self, point );
				newWpCreated = true;
			}
			
			if (isIdValid(waypoint))
			{
				setWaypointVisible( waypoint, true );
				setWaypointActive( waypoint, true );
				setWaypointLocation( waypoint, point );
				setWaypointName( waypoint, "Resource Survey");
				
				if (newWpCreated)
				{
					setWaypointColor( waypoint, "orange");
				}
				
				setObjVar( self, "survey_waypoint", waypoint );
			}
			
			sendSystemMessage( self, SID_SURVEY_WAYPOINT );
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public obj_id getSurveyWaypoint(obj_id self) throws InterruptedException
	{
		
		obj_id[] data = getWaypointsInDatapad( self );
		if (data != null)
		{
			for (int i=0; i<data.length; i++)
			{
				testAbortScript();
				if (!isIdValid(data[i]))
				{
					continue;
				}
				String waypointName = getWaypointName( data[i] );
				if ((waypointName != null) && waypointName.equals( "Resource Survey" ))
				{
					
					return data[i];
				}
			}
		}
		
		return null;
	}
	
	
	public int OnChangedPosture(obj_id self, int before, int after) throws InterruptedException
	{
		
		if (meditation.isMeditating(self))
		{
			meditation.endMeditation(self);
		}
		
		if (before == POSTURE_CROUCHED)
		{
			if (utils.hasScriptVar( self, "surveying.takingSamples" ))
			{
				obj_id tool = utils.getObjIdScriptVar( self, "surveying.tool");
				resource.cleanupTool(self, tool);
				sendSystemMessage( self, SID_SAMPLE_CANCEL );
				
			}
			else if (utils.hasScriptVar( self, "trapdrop.settrap" ))
			{
				obj_id trap = utils.getObjIdScriptVar( self, "trapdrop.settrap");
				messageTo( trap, "stopDeploy", null, 0.f, false );
				sendSystemMessage( self, SID_TRAP_CANCEL );
				utils.removeScriptVar( self, "trapdrop.settrap");
			}
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int OnCreatureDamaged(obj_id self, obj_id attacker, obj_id weapon, int[] damage) throws InterruptedException
	{
		
		if (meditation.isMeditating(self))
		{
			meditation.endMeditation(self);
		}
		
		if (utils.hasScriptVar( self, "surveying.takingSamples" ))
		{
			obj_id tool = utils.getObjIdScriptVar( self, "surveying.tool");
			resource.cleanupTool(self, tool);
			sendSystemMessage( self, SID_SAMPLE_CANCEL_ATTACK );
		}
		else if (utils.hasScriptVar( self, "trapdrop.settrap" ))
		{
			obj_id trap = utils.getObjIdScriptVar( self, "trapdrop.settrap");
			messageTo( trap, "stopDeploy", null, 0.f, false );
			sendSystemMessage( self, SID_TRAP_CANCEL_ATTACK );
			utils.removeScriptVar( self, "trapdrop.settrap");
		}
		
		metrics.logArmorStatus(self);
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int OnTargeted(obj_id self, obj_id attacker) throws InterruptedException
	{
		if (utils.hasScriptVar( self, "surveying.takingSamples" ))
		{
			obj_id tool = utils.getObjIdScriptVar( self, "surveying.tool");
			resource.cleanupTool(self, tool);
			return SCRIPT_CONTINUE;
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int handleSurveyNodeChoice(obj_id self, dictionary params) throws InterruptedException
	{
		if (!sui.hasPid(self, resource.PID_NAME))
		{
			return SCRIPT_CONTINUE;
		}
		
		sui.removePid(self, resource.PID_NAME);
		
		if (!utils.hasScriptVar( self, "surveying.takingSamples" ))
		{
			return SCRIPT_CONTINUE;
		}
		obj_id curtool = utils.getObjIdScriptVar( self, "surveying.tool");
		obj_id tool = utils.getObjIdScriptVar( self, "survey_event.tool");
		if (tool != curtool)
		{
			return SCRIPT_CONTINUE;
		}
		
		int idx = sui.getListboxSelectedRow( params );
		if (idx < 0)
		{
			idx = 0;
		}
		obj_id player = sui.getPlayerId( params );
		int btn = sui.getIntButtonPressed( params );
		if ((btn == sui.BP_CANCEL) || (idx == 0))
		{
			
			dictionary outparams = new dictionary();
			outparams.put( "player", self );
			messageTo( tool, "continueSampleLoop", outparams, 0.f, false );
			return SCRIPT_CONTINUE;
		}
		
		else if (idx == 2)
		{
			if (!hasCompletedCollection(player, "sampling_pet_collection"))
			{
				modifyCollectionSlotValue(player, "col_pet_resource_sampling", 1);
			}
			
			int dieRoll = rand( 1, 100 );
			if (dieRoll < 75)
			{
				
				utils.setScriptVar( player, "survey_event.gamble", 3 );
			}
			else
			{
				
				utils.setScriptVar( player, "survey_event.gamble", 4 );
			}
			
			dictionary outparams = new dictionary();
			outparams.put( "player", self );
			messageTo( tool, "continueSampleLoop", outparams, 0.f, false );
			return SCRIPT_CONTINUE;
		}
		
		dictionary outparams = new dictionary();
		outparams.put( "player", self );
		messageTo( tool, "stopSampleEvent", outparams, 0.f, false );
		
		location point = locations.getGoodLocationAroundLocation( getLocation(self), 10, 10, 50, 50, true, false );
		
		if (point == null)
		{
			return SCRIPT_CONTINUE;
		}
		
		obj_id waypoint = getSurveyWaypoint( self );
		if (!isIdValid(waypoint))
		{
			waypoint = createWaypointInDatapad( self, point );
		}
		if (isIdValid(waypoint))
		{
			setWaypointVisible( waypoint, true );
			setWaypointActive( waypoint, true );
			setWaypointLocation( waypoint, point );
			setWaypointName( waypoint, "Resource Survey");
			setWaypointColor( waypoint, "blue");
		}
		
		sendSystemMessage( self, SID_NODE_WAYPOINT );
		
		utils.setScriptVar( self, "survey_event.location", point );
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int handleSurveyGambleChoice(obj_id self, dictionary params) throws InterruptedException
	{
		if (!sui.hasPid(self, resource.PID_NAME))
		{
			return SCRIPT_CONTINUE;
		}
		
		sui.removePid(self, resource.PID_NAME);
		
		if (!utils.hasScriptVar( self, "surveying.takingSamples" ))
		{
			return SCRIPT_CONTINUE;
		}
		obj_id curtool = utils.getObjIdScriptVar( self, "surveying.tool");
		obj_id tool = utils.getObjIdScriptVar( self, "survey_event.tool");
		if (tool != curtool)
		{
			return SCRIPT_CONTINUE;
		}
		
		int idx = sui.getListboxSelectedRow( params );
		if (idx < 0)
		{
			idx = 0;
		}
		obj_id player = sui.getPlayerId( params );
		int btn = sui.getIntButtonPressed( params );
		if ((btn == sui.BP_CANCEL) || (idx == 0))
		{
			
			dictionary outparams = new dictionary();
			outparams.put( "player", self );
			messageTo( tool, "continueSampleLoop", outparams, 0.f, false );
			return SCRIPT_CONTINUE;
		}
		
		int action = getAttrib( player, ACTION );
		int actioncost = 2000;
		if (!drainAttributes( player, actioncost, 0 ))
		{
			
			sendSystemMessage( player, SID_GAMBLE_NO_ACTION );
			dictionary outparams = new dictionary();
			outparams.put( "player", self );
			messageTo( tool, "continueSampleLoop", outparams, 0.f, false );
			return SCRIPT_CONTINUE;
		}
		
		if (idx == 2)
		{
			String resource_class = getStringObjVar(tool, resource.VAR_SURVEY_CLASS);
			String collectionName = "col_resource_"+ resource_class + "_01";
			modifyCollectionSlotValue(player, collectionName, 1);
			utils.setScriptVar( player, "survey_event.gamble", 5 );
			
			dictionary outparams = new dictionary();
			outparams.put( "player", self );
			messageTo( tool, "continueSampleLoop", outparams, 0.f, false );
			return SCRIPT_CONTINUE;
		}
		
		int dieRoll = rand( 1, 100 );
		if (dieRoll < 75)
		{
			
			utils.setScriptVar( player, "survey_event.gamble", 1 );
		}
		else
		{
			
			utils.setScriptVar( player, "survey_event.gamble", 2 );
		}
		
		dictionary outparams = new dictionary();
		outparams.put( "player", self );
		messageTo( tool, "continueSampleLoop", outparams, 0.f, false );
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int cmdHarvestDNA(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!isIdValid(target))
		{
			return SCRIPT_CONTINUE;
		}
		
		bio_engineer.harvestCreatureDNA (self, target);
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int cmdHarvestDNAFail(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!isIdValid(target))
		{
			sendSystemMessage(self, bio_engineer.SID_HARVEST_DNA_FAILED);
		}
		else
		{
			prose_package pp = prose.getPackage (bio_engineer.PROSE_HARVEST_DNA_FAILED, target);
			sendSystemMessageProse (self, pp);
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int completeHarvestDNA(obj_id self, dictionary params) throws InterruptedException
	{
		
		bio_engineer.completeHarvest (self);
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int cmdWaypoint(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		String wpColor = null;
		float displayCoordX = 0.0f;
		float displayCoordZ = 0.0f;
		boolean wpDifferentPlanet = false;
		obj_id lookAtTarget = null;
		obj_id intendedTarget = null;
		
		int parsedIndex = 0;
		Vector parsedParams = new Vector();
		parsedParams.setSize(0);
		if (params != null && !params.equals(""))
		{
			java.util.StringTokenizer st = new java.util.StringTokenizer(params);
			while (st.hasMoreTokens())
			{
				testAbortScript();
				String token = st.nextToken();
				if (wpColor == null)
				{
					for (int i = 0; i < utils.WAYPOINT_COLORS.length; ++i)
					{
						testAbortScript();
						if (token.toLowerCase().equals(utils.WAYPOINT_COLORS[i]))
						{
							wpColor = utils.WAYPOINT_COLORS[i];
							break;
						}
					}
					
					if (wpColor == null)
					{
						utils.addElement(parsedParams, token);
					}
				}
				else
				{
					utils.addElement(parsedParams, token);
				}
			}
		}
		
		location spot = null;
		if (parsedParams == null || (parsedParams.size() <= 0))
		{
			spot = getWorldLocation(self);
			
			if (spot != null)
			{
				displayCoordX = spot.x;
				displayCoordZ = spot.z;
				float[] baInfo = getBuildoutAreaSizeAndCenter(spot.x, spot.z, spot.area, true, true);
				if ((baInfo != null) && (baInfo.length == 4))
				{
					displayCoordX -= baInfo[2];
					displayCoordZ -= baInfo[3];
				}
			}
		}
		else
		{
			String wpGroundPlanet = null;
			vector wpGroundPLanetBuildoutCoords = null;
			String wpSpaceZone = null;
			float wpCoord1 = Float.NEGATIVE_INFINITY;
			float wpCoord2 = Float.NEGATIVE_INFINITY;
			float wpCoord3 = Float.NEGATIVE_INFINITY;
			boolean useTarget = false;
			
			wpCoord1 = utils.stringToFloat(((String)(parsedParams.get(parsedIndex))));
			if (wpCoord1 == Float.NEGATIVE_INFINITY)
			{
				
				String planet = ((String)(parsedParams.get(parsedIndex))).toLowerCase();
				
				if (planet.equals("-target"))
				{
					useTarget = true;
					++parsedIndex;
				}
				else
				{
					for (int i = 0; i < WAYPOINT_GROUND_PLANETS_EXTERNAL.length; ++i)
					{
						testAbortScript();
						if ((planet.equals(WAYPOINT_GROUND_PLANETS_EXTERNAL[i])) && (i < WAYPOINT_GROUND_PLANETS_INTERNAL.length))
						{
							wpGroundPlanet = WAYPOINT_GROUND_PLANETS_INTERNAL[i];
							
							if (i < WAYPOINT_GROUND_PLANETS_BUILDOUT_COORDS.length)
							{
								wpGroundPLanetBuildoutCoords = WAYPOINT_GROUND_PLANETS_BUILDOUT_COORDS[i];
							}
							
							++parsedIndex;
							break;
						}
					}
					
					if (wpGroundPlanet == null)
					{
						for (int i = 0; i < WAYPOINT_SPACE_ZONES_EXTERNAL.length; ++i)
						{
							testAbortScript();
							if ((planet.equals(WAYPOINT_SPACE_ZONES_EXTERNAL[i])) && (i < WAYPOINT_SPACE_ZONES_INTERNAL.length))
							{
								wpSpaceZone = WAYPOINT_SPACE_ZONES_INTERNAL[i];
								
								++parsedIndex;
								break;
							}
						}
					}
				}
				
				if (((wpGroundPlanet != null) || (wpSpaceZone != null)) && (parsedIndex < parsedParams.size()))
				{
					wpCoord1 = utils.stringToFloat(((String)(parsedParams.get(parsedIndex))));
				}
			}
			
			if (wpCoord1 != Float.NEGATIVE_INFINITY)
			{
				++parsedIndex;
				if (parsedIndex < parsedParams.size())
				{
					wpCoord2 = utils.stringToFloat(((String)(parsedParams.get(parsedIndex))));
					if (wpCoord2 != Float.NEGATIVE_INFINITY)
					{
						++parsedIndex;
						if (parsedIndex < parsedParams.size())
						{
							wpCoord3 = utils.stringToFloat(((String)(parsedParams.get(parsedIndex))));
							if (wpCoord3 != Float.NEGATIVE_INFINITY)
							{
								++parsedIndex;
							}
							else if (wpSpaceZone != null)
							{
								wpCoord1 = Float.NEGATIVE_INFINITY;
								wpCoord2 = Float.NEGATIVE_INFINITY;
								--parsedIndex;
								--parsedIndex;
							}
						}
						else if (wpSpaceZone != null)
						{
							wpCoord1 = Float.NEGATIVE_INFINITY;
							wpCoord2 = Float.NEGATIVE_INFINITY;
							--parsedIndex;
							--parsedIndex;
						}
					}
					else
					{
						wpCoord1 = Float.NEGATIVE_INFINITY;
						--parsedIndex;
					}
				}
				else
				{
					wpCoord1 = Float.NEGATIVE_INFINITY;
					--parsedIndex;
				}
			}
			
			if ((wpGroundPlanet != null) && ((wpCoord1 == Float.NEGATIVE_INFINITY) || (wpCoord2 == Float.NEGATIVE_INFINITY)))
			{
				
				wpGroundPlanet = null;
				wpCoord1 = Float.NEGATIVE_INFINITY;
				wpCoord2 = Float.NEGATIVE_INFINITY;
				wpCoord3 = Float.NEGATIVE_INFINITY;
				parsedIndex = 0;
			}
			
			if ((wpSpaceZone != null) && ((wpCoord1 == Float.NEGATIVE_INFINITY) || (wpCoord2 == Float.NEGATIVE_INFINITY) || (wpCoord3 == Float.NEGATIVE_INFINITY)))
			{
				
				wpSpaceZone = null;
				wpCoord1 = Float.NEGATIVE_INFINITY;
				wpCoord2 = Float.NEGATIVE_INFINITY;
				wpCoord3 = Float.NEGATIVE_INFINITY;
				parsedIndex = 0;
			}
			
			if (wpCoord1 == Float.NEGATIVE_INFINITY)
			{
				if (useTarget)
				{
					intendedTarget = getIntendedTarget(self);
					if (isIdValid(intendedTarget))
					{
						spot = getWorldLocation(intendedTarget);
					}
					
					if (spot == null)
					{
						intendedTarget = null;
						lookAtTarget = getLookAtTarget(self);
						if (isIdValid(lookAtTarget))
						{
							spot = getWorldLocation(lookAtTarget);
						}
					}
				}
				
				if (spot == null)
				{
					intendedTarget = null;
					lookAtTarget = null;
					spot = getWorldLocation(self);
				}
				
				if (spot != null)
				{
					displayCoordX = spot.x;
					displayCoordZ = spot.z;
					float[] baInfo = getBuildoutAreaSizeAndCenter(spot.x, spot.z, spot.area, true, true);
					if ((baInfo != null) && (baInfo.length == 4))
					{
						displayCoordX -= baInfo[2];
						displayCoordZ -= baInfo[3];
					}
				}
			}
			else
			{
				if (wpCoord3 == Float.NEGATIVE_INFINITY)
				{
					spot = new location(wpCoord1, 0.0f, wpCoord2);
				}
				else
				{
					spot = new location(wpCoord1, wpCoord2, wpCoord3);
				}
				
				if (spot != null)
				{
					displayCoordX = spot.x;
					displayCoordZ = spot.z;
					
					float[] baInfo = null;
					
					if ((wpGroundPlanet != null) && (!wpGroundPlanet.equals(spot.area)))
					{
						if (wpGroundPLanetBuildoutCoords != null)
						{
							baInfo = getBuildoutAreaSizeAndCenter(wpGroundPLanetBuildoutCoords.x, wpGroundPLanetBuildoutCoords.z, wpGroundPlanet, true, true);
						}
						
						wpDifferentPlanet = true;
						spot.area = wpGroundPlanet;
					}
					else if ((wpSpaceZone != null) && (!wpSpaceZone.equals(spot.area)))
					{
						wpDifferentPlanet = true;
						spot.area = wpSpaceZone;
					}
					else
					{
						location currentLoc = getWorldLocation(self);
						if (currentLoc != null)
						{
							baInfo = getBuildoutAreaSizeAndCenter(currentLoc.x, currentLoc.z, currentLoc.area, true, true);
						}
					}
					
					if ((baInfo != null) && (baInfo.length == 4))
					{
						final float halfWidth = baInfo[0]/2;
						if (displayCoordX > halfWidth)
						{
							displayCoordX = halfWidth;
						}
						else if (displayCoordX < -halfWidth)
						{
							displayCoordX = -halfWidth;
						}
						
						final float halfHeight = baInfo[1]/2;
						if (displayCoordZ > halfHeight)
						{
							displayCoordZ = halfHeight;
						}
						else if (displayCoordZ < -halfHeight)
						{
							displayCoordZ = -halfHeight;
						}
						
						spot.x = displayCoordX + baInfo[2];
						spot.z = displayCoordZ + baInfo[3];
					}
				}
			}
		}
		
		if (spot == null)
		{
			sendSystemMessageTestingOnly(self, "Waypoint: The system was unable to parse a valid waypoint location.");
		}
		else
		{
			obj_id wp = createWaypointInDatapadWithLimits(self, spot);
			if (isIdValid(wp))
			{
				if (!wpDifferentPlanet)
				{
					setWaypointActive(wp, true);
				}
				
				setWaypointVisible(wp, true);
				
				String wpName = null;
				for (int i = parsedIndex; i < parsedParams.size(); ++i)
				{
					testAbortScript();
					if (wpName == null)
					{
						wpName = ((String)(parsedParams.get(i)));
					}
					else
					{
						wpName += " ";
						wpName += ((String)(parsedParams.get(i)));
					}
				}
				
				if (wpName == null)
				{
					wpName = "Waypoint";
				}
				
				setWaypointName(wp, wpName);
				
				if (wpColor != null && !wpColor.equals(""))
				{
					setWaypointColor(wp, wpColor);
				}
				
				if (!wpDifferentPlanet)
				{
					if (isIdValid(intendedTarget))
					{
						sendSystemMessageTestingOnly(self, "Waypoint: New waypoint \""+ wpName + "\" created for intended target location ("+ (int)displayCoordX + ", "+ (int)spot.y + ", "+ (int)displayCoordZ + ")");
					}
					else if (isIdValid(lookAtTarget))
					{
						sendSystemMessageTestingOnly(self, "Waypoint: New waypoint \""+ wpName + "\" created for look at target location ("+ (int)displayCoordX + ", "+ (int)spot.y + ", "+ (int)displayCoordZ + ")");
					}
					else
					{
						sendSystemMessageTestingOnly(self, "Waypoint: New waypoint \""+ wpName + "\" created for location ("+ (int)displayCoordX + ", "+ (int)spot.y + ", "+ (int)displayCoordZ + ")");
					}
				}
				else
				{
					final String localizedPlanetName = localize(new string_id("planet_n", spot.area));
					sendSystemMessageTestingOnly(self, "Waypoint: New waypoint \""+ wpName + "\" created for location "+ localizedPlanetName + " ("+ (int)displayCoordX + ", "+ (int)spot.y + ", "+ (int)displayCoordZ + ")");
				}
			}
			else
			{
				string_id errorMessage = new string_id("base_player", "too_many_waypoints");
				sendSystemMessage(self, errorMessage);
			}
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int cmdSpaceWaypoint(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		String wpColor = null;
		float displayCoordX = 0.0f;
		float displayCoordZ = 0.0f;
		boolean wpDifferentPlanet = false;
		
		int parsedIndex = 0;
		Vector parsedParams = new Vector();
		parsedParams.setSize(0);
		if (params != null && !params.equals(""))
		{
			java.util.StringTokenizer st = new java.util.StringTokenizer(params);
			while (st.hasMoreTokens())
			{
				testAbortScript();
				String token = st.nextToken();
				if (wpColor == null)
				{
					for (int i = 0; i < utils.WAYPOINT_COLORS.length; ++i)
					{
						testAbortScript();
						if (token.toLowerCase().equals(utils.WAYPOINT_COLORS[i]))
						{
							wpColor = utils.WAYPOINT_COLORS[i];
							break;
						}
					}
					
					if (wpColor == null)
					{
						utils.addElement(parsedParams, token);
					}
				}
				else
				{
					utils.addElement(parsedParams, token);
				}
			}
		}
		
		location spot = null;
		if (parsedParams == null || (parsedParams.size() <= 0))
		{
			if (!isIdValid(target))
			{
				target = space_transition.getContainingShip(self);
			}
			else
			{
				obj_id objContainer = getTopMostContainer(target);
				if (isIdValid(objContainer))
				{
					target = objContainer;
				}
			}
			
			spot = getWorldLocation(target);
			
			if (spot != null)
			{
				displayCoordX = spot.x;
				displayCoordZ = spot.z;
			}
		}
		else
		{
			String wpGroundPlanet = null;
			vector wpGroundPLanetBuildoutCoords = null;
			String wpSpaceZone = null;
			float wpCoord1 = Float.NEGATIVE_INFINITY;
			float wpCoord2 = Float.NEGATIVE_INFINITY;
			float wpCoord3 = Float.NEGATIVE_INFINITY;
			
			wpCoord1 = utils.stringToFloat(((String)(parsedParams.get(parsedIndex))));
			if (wpCoord1 == Float.NEGATIVE_INFINITY)
			{
				
				String planet = ((String)(parsedParams.get(parsedIndex))).toLowerCase();
				for (int i = 0; i < WAYPOINT_GROUND_PLANETS_EXTERNAL.length; ++i)
				{
					testAbortScript();
					if ((planet.equals(WAYPOINT_GROUND_PLANETS_EXTERNAL[i])) && (i < WAYPOINT_GROUND_PLANETS_INTERNAL.length))
					{
						wpGroundPlanet = WAYPOINT_GROUND_PLANETS_INTERNAL[i];
						
						if (i < WAYPOINT_GROUND_PLANETS_BUILDOUT_COORDS.length)
						{
							wpGroundPLanetBuildoutCoords = WAYPOINT_GROUND_PLANETS_BUILDOUT_COORDS[i];
						}
						
						++parsedIndex;
						break;
					}
				}
				
				if (wpGroundPlanet == null)
				{
					for (int i = 0; i < WAYPOINT_SPACE_ZONES_EXTERNAL.length; ++i)
					{
						testAbortScript();
						if ((planet.equals(WAYPOINT_SPACE_ZONES_EXTERNAL[i])) && (i < WAYPOINT_SPACE_ZONES_INTERNAL.length))
						{
							wpSpaceZone = WAYPOINT_SPACE_ZONES_INTERNAL[i];
							
							++parsedIndex;
							break;
						}
					}
				}
				
				if (((wpGroundPlanet != null) || (wpSpaceZone != null)) && (parsedIndex < parsedParams.size()))
				{
					wpCoord1 = utils.stringToFloat(((String)(parsedParams.get(parsedIndex))));
				}
			}
			
			if (wpCoord1 != Float.NEGATIVE_INFINITY)
			{
				++parsedIndex;
				if (parsedIndex < parsedParams.size())
				{
					wpCoord2 = utils.stringToFloat(((String)(parsedParams.get(parsedIndex))));
					if (wpCoord2 != Float.NEGATIVE_INFINITY)
					{
						++parsedIndex;
						if (parsedIndex < parsedParams.size())
						{
							wpCoord3 = utils.stringToFloat(((String)(parsedParams.get(parsedIndex))));
							if (wpCoord3 != Float.NEGATIVE_INFINITY)
							{
								++parsedIndex;
							}
							else if (wpGroundPlanet == null)
							{
								wpCoord1 = Float.NEGATIVE_INFINITY;
								wpCoord2 = Float.NEGATIVE_INFINITY;
								--parsedIndex;
								--parsedIndex;
							}
						}
						else if (wpGroundPlanet == null)
						{
							wpCoord1 = Float.NEGATIVE_INFINITY;
							wpCoord2 = Float.NEGATIVE_INFINITY;
							--parsedIndex;
							--parsedIndex;
						}
					}
					else
					{
						wpCoord1 = Float.NEGATIVE_INFINITY;
						--parsedIndex;
					}
				}
				else
				{
					wpCoord1 = Float.NEGATIVE_INFINITY;
					--parsedIndex;
				}
			}
			
			if ((wpGroundPlanet != null) && ((wpCoord1 == Float.NEGATIVE_INFINITY) || (wpCoord2 == Float.NEGATIVE_INFINITY)))
			{
				
				wpGroundPlanet = null;
				wpCoord1 = Float.NEGATIVE_INFINITY;
				wpCoord2 = Float.NEGATIVE_INFINITY;
				wpCoord3 = Float.NEGATIVE_INFINITY;
				parsedIndex = 0;
			}
			
			if ((wpSpaceZone != null) && ((wpCoord1 == Float.NEGATIVE_INFINITY) || (wpCoord2 == Float.NEGATIVE_INFINITY) || (wpCoord3 == Float.NEGATIVE_INFINITY)))
			{
				
				wpSpaceZone = null;
				wpCoord1 = Float.NEGATIVE_INFINITY;
				wpCoord2 = Float.NEGATIVE_INFINITY;
				wpCoord3 = Float.NEGATIVE_INFINITY;
				parsedIndex = 0;
			}
			
			if (wpCoord1 == Float.NEGATIVE_INFINITY)
			{
				if (!isIdValid(target))
				{
					target = space_transition.getContainingShip(self);
				}
				else
				{
					obj_id objContainer = getTopMostContainer(target);
					if (isIdValid(objContainer))
					{
						target = objContainer;
					}
				}
				
				spot = getWorldLocation(target);
				
				if (spot != null)
				{
					displayCoordX = spot.x;
					displayCoordZ = spot.z;
				}
			}
			else
			{
				if (wpCoord3 == Float.NEGATIVE_INFINITY)
				{
					spot = new location(wpCoord1, 0.0f, wpCoord2);
				}
				else
				{
					spot = new location(wpCoord1, wpCoord2, wpCoord3);
				}
				
				if (spot != null)
				{
					displayCoordX = spot.x;
					displayCoordZ = spot.z;
					
					if ((wpGroundPlanet != null) && (!wpGroundPlanet.equals(spot.area)))
					{
						if (wpGroundPLanetBuildoutCoords != null)
						{
							float[] baInfo = getBuildoutAreaSizeAndCenter(wpGroundPLanetBuildoutCoords.x, wpGroundPLanetBuildoutCoords.z, wpGroundPlanet, true, true);
							
							if ((baInfo != null) && (baInfo.length == 4))
							{
								final float halfWidth = baInfo[0]/2;
								if (displayCoordX > halfWidth)
								{
									displayCoordX = halfWidth;
								}
								else if (displayCoordX < -halfWidth)
								{
									displayCoordX = -halfWidth;
								}
								
								final float halfHeight = baInfo[1]/2;
								if (displayCoordZ > halfHeight)
								{
									displayCoordZ = halfHeight;
								}
								else if (displayCoordZ < -halfHeight)
								{
									displayCoordZ = -halfHeight;
								}
								
								spot.x = displayCoordX + baInfo[2];
								spot.z = displayCoordZ + baInfo[3];
							}
						}
						
						wpDifferentPlanet = true;
						spot.area = wpGroundPlanet;
					}
					else if ((wpSpaceZone != null) && (!wpSpaceZone.equals(spot.area)))
					{
						wpDifferentPlanet = true;
						spot.area = wpSpaceZone;
					}
				}
			}
		}
		
		if (spot == null)
		{
			sendSystemMessageTestingOnly(self, "Waypoint: The system was unable to parse a valid waypoint location.");
		}
		else
		{
			obj_id wp = createWaypointInDatapadWithLimits(self, spot);
			if (isIdValid(wp))
			{
				if (!wpDifferentPlanet)
				{
					setWaypointActive(wp, true);
				}
				
				setWaypointVisible(wp, true);
				
				String wpName = null;
				for (int i = parsedIndex; i < parsedParams.size(); ++i)
				{
					testAbortScript();
					if (wpName == null)
					{
						wpName = ((String)(parsedParams.get(i)));
					}
					else
					{
						wpName += " ";
						wpName += ((String)(parsedParams.get(i)));
					}
				}
				
				if (wpName == null)
				{
					wpName = "Waypoint";
				}
				
				setWaypointName(wp, wpName);
				
				if ((wpColor == null) && !wpDifferentPlanet)
				{
					wpColor = "space";
				}
				
				if (wpColor != null && !wpColor.equals(""))
				{
					setWaypointColor(wp, wpColor);
				}
				
				if (!wpDifferentPlanet)
				{
					sendSystemMessageTestingOnly(self, "Waypoint: New waypoint \""+ wpName + "\" created for location ("+ (int)displayCoordX + ", "+ (int)spot.y + ", "+ (int)displayCoordZ + ")");
				}
				else
				{
					final String localizedPlanetName = localize(new string_id("planet_n", spot.area));
					sendSystemMessageTestingOnly(self, "Waypoint: New waypoint \""+ wpName + "\" created for location "+ localizedPlanetName + " ("+ (int)displayCoordX + ", "+ (int)spot.y + ", "+ (int)displayCoordZ + ")");
				}
			}
			else
			{
				string_id errorMessage = new string_id("base_player", "too_many_waypoints");
				sendSystemMessage(self, errorMessage);
			}
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int cmdClearQueue(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		queueClear(self);
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int getPrototype(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!isIdValid(target))
		{
			return SCRIPT_CONTINUE;
		}
		
		if (!hasObjVar(target, "crafting.tool"))
		{
			return SCRIPT_CONTINUE;
		}
		
		messageTo(target, "getPrototype", null, 0, false);
		return SCRIPT_CONTINUE;
	}
	
	
	public int cmdCityban(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (target == self)
		{
			return SCRIPT_CONTINUE;
		}
		
		if (!isPlayer( target ))
		{
			return SCRIPT_CONTINUE;
		}
		
		int city_id = city.checkCity( self, true );
		if (city_id == 0)
		{
			return SCRIPT_CONTINUE;
		}
		
		obj_id mayor = cityGetLeader( city_id );
		if (!city.isMilitiaOfCity( self, city_id ) && (self != mayor))
		{
			sendSystemMessage( self, SID_NOT_MILITIA );
			return SCRIPT_CONTINUE;
		}
		
		if (isGod(target) || hasObjVar(target, "gm"))
		{
			
			sendSystemMessage(self, SID_NOT_CSR_BAN);
			
			prose_package pp = prose.getPackage(SID_CSR_BAN_ATTEMPT_MSG, getFirstName(self), cityGetName(city_id));
			sendSystemMessageProse(target, pp);
			
			return SCRIPT_CONTINUE;
		}
		
		if (target == mayor)
		{
			return SCRIPT_CONTINUE;
		}
		
		if (city.isCitizenOfCity( target, city_id ))
		{
			sendSystemMessage( self, SID_NOT_CITIZEN_BAN );
			return SCRIPT_CONTINUE;
		}
		
		sendSystemMessage( target, SID_CITY_BANNED );
		int[] banCities = getIntArrayObjVar( target, "city.banlist");
		if (banCities == null)
		{
			int[] newBanCities = new int[1];
			newBanCities[0] = city_id;
			
			setObjVar( target, "city.banlist", newBanCities );
		}
		else
		{
			
			for (int i=0; i<banCities.length; i++)
			{
				testAbortScript();
				if (banCities[i] == city_id)
				{
					return SCRIPT_CONTINUE;
				}
			}
			
			int[] newBanCities = new int[banCities.length+1];
			for (int i=0; i<banCities.length; i++)
			{
				testAbortScript();
				newBanCities[i+1] = banCities[i];
			}
			newBanCities[0] = city_id;
			
			setObjVar( target, "city.banlist", newBanCities );
		}
		
		prose_package pp = prose.getPackage( SID_CITY_BAN_DONE, target );
		sendSystemMessageProse( self, pp );
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int cmdCitypardon(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (target == self)
		{
			return SCRIPT_CONTINUE;
		}
		
		if (!isPlayer( target ))
		{
			return SCRIPT_CONTINUE;
		}
		
		int city_id = city.checkCity( self, true );
		if (city_id == 0)
		{
			return SCRIPT_CONTINUE;
		}
		
		obj_id mayor = cityGetLeader( city_id );
		if (!city.isMilitiaOfCity( self, city_id ) && (self != mayor))
		{
			sendSystemMessage( self, SID_NOT_MILITIA );
			return SCRIPT_CONTINUE;
		}
		
		if (target == mayor)
		{
			return SCRIPT_CONTINUE;
		}
		
		int[] banCities = getIntArrayObjVar( target, "city.banlist");
		if (banCities != null)
		{
			int found = 0;
			for (int i=0; i<banCities.length; i++)
			{
				testAbortScript();
				if (banCities[i] == city_id)
				{
					found = 1;
					break;
				}
			}
			if (found == 0)
			{
				return SCRIPT_CONTINUE;
			}
			
			if (banCities.length == 1)
			{
				removeObjVar( target, "city.banlist");
			}
			else
			{
				int j = 0;
				int[] newBanCities = new int[banCities.length-1];
				for (int i=0; i<banCities.length; i++)
				{
					testAbortScript();
					if (banCities[i] != city_id)
					{
						newBanCities[j] = banCities[i];
						j++;
					}
				}
				setObjVar( target, "city.banlist", newBanCities );
			}
		}
		sendSystemMessage( target, SID_CITY_PARDONED );
		
		prose_package pp = prose.getPackage( SID_CITY_PARDON_DONE, target );
		sendSystemMessageProse( self, pp );
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int OnCityChanged(obj_id self, int oldCityId, int newCityId) throws InterruptedException
	{
		if ((oldCityId != 0) && cityExists(oldCityId))
		{
			String city_name = cityGetName( oldCityId );
			prose_package pp = prose.getPackage( SID_LEAVE_CITY, city_name );
			sendSystemMessageProse( self, pp );
		}
		
		if ((newCityId != 0) && cityExists(newCityId))
		{
			int city_rank = city.getCityRank( newCityId );
			string_id rank_name = new string_id( "city/city", "rank"+city_rank );
			String city_name = cityGetName( newCityId );
			String spec = city.cityGetSpecString( newCityId );
			prose_package pp = new prose_package();
			pp.stringId = SID_ENTER_CITY;
			pp.target.set( city_name );
			String specpart = localize(rank_name);
			if (spec != null && !spec.equals("null"))
			{
				specpart = specpart + ", "+ localize( new string_id( "city/city", spec ) );
			}
			
			int factionId = cityGetFaction(newCityId);
			if ((-615855020) == factionId)
			{
				specpart = specpart + ", Imperial aligned";
			}
			else if ((370444368) == factionId)
			{
				specpart = specpart + ", Rebel aligned";
			}
			
			pp.other.set( specpart );
			sendSystemMessageProse( self, pp );
			
			obj_id cityHallId = cityGetCityHall(newCityId);
			String cityName = cityGetName(newCityId);
			if (hasObjVar(cityHallId, "city_visitor_message"))
			{
				String cityVisitorMessage = getStringObjVar(cityHallId, "city_visitor_message");
				sendConsoleMessage(self, "City("+ cityName + ") Message: "+ cityVisitorMessage + "\\#DFDFDF");
			}
		}
		return SCRIPT_CONTINUE;
	}
	
	
	public int cmdGrantZoningRights(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (target == self)
		{
			return SCRIPT_CONTINUE;
		}
		
		if (!isPlayer(target))
		{
			return SCRIPT_CONTINUE;
		}
		
		utils.setScriptVar(self, "zoning_rights_target", target);
		
		sui.listbox(self, self, "@city/city:zoning_rights_p", sui.OK_CANCEL, "@city/city:zoning_rights_t", ZONING_RIGHTS_ARRAY, "handleZoningRightsSelect", true);
		
		return SCRIPT_CONTINUE;
		
	}
	
	
	public int handleZoningRightsSelect(obj_id self, dictionary params) throws InterruptedException
	{
		obj_id player = sui.getPlayerId(params);
		
		int btn = sui.getIntButtonPressed(params);
		if (btn == sui.BP_CANCEL)
		{
			utils.removeScriptVar(player, "zoning_rights_target");
			return SCRIPT_CONTINUE;
		}
		
		if (!utils.hasScriptVar(player, "zoning_rights_target"))
		{
			return SCRIPT_CONTINUE;
		}
		obj_id target = utils.getObjIdScriptVar(player, "zoning_rights_target");
		utils.removeScriptVar(player, "zoning_rights_target");
		
		obj_id top = getTopMostContainer(player);
		int city_id = getCityAtLocation(getLocation(top), 0);
		
		if (city_id <= 0)
		{
			sendSystemMessage(player, SID_NOT_IN_CITY_LIMITS);
			return SCRIPT_CONTINUE;
		}
		
		int idx = sui.getListboxSelectedRow(params);
		
		if (idx < 0)
		{
			return SCRIPT_CONTINUE;
		}
		
		switch (idx)
		{
			case 0:
			handleCmdGrantZoningRights(player, target, city_id);
			break;
			case 1:
			handleStorytellerZoningRights(player, target, city_id);
			break;
		}
		return SCRIPT_CONTINUE;
	}
	
	
	public void handleCmdGrantZoningRights(obj_id player, obj_id target, int city_id) throws InterruptedException
	{
		if (!isIdValid(player) || !isIdValid(target))
		{
			return;
		}
		if (target == player)
		{
			return;
		}
		if (!isPlayer(target))
		{
			return;
		}
		
		obj_id mayor = cityGetLeader(city_id);
		
		if ((mayor != player) && !city.isMilitiaOfCity(player, city_id))
		{
			sendSystemMessage(player, SID_GRANT_RIGHTS_FAIL);
			return;
		}
		
		if (hasObjVar(target, "city.zoning_rights"))
		{
			int other_city_id = getIntObjVar(target, "city.zoning_rights");
			if (other_city_id == city_id)
			{
				int curt = getGameTime();
				int rights_time = getIntObjVar(target, "city.zoning_rights_time");
				if (curt - rights_time < 24*60*60)
				{
					removeObjVar(target, "city.zoning_rights");
					removeObjVar(target, "city.zoning_rights_time");
					sendSystemMessage(player, SID_RIGHTS_REVOKED);
					sendSystemMessage(target, SID_RIGHTS_REVOKED_OTHER);
					return;
				}
			}
		}
		
		setObjVar(target, "city.zoning_rights", city_id);
		setObjVar(target, "city.zoning_rights_time", getGameTime());
		prose_package pp = prose.getPackage(SID_RIGHTS_GRANTED, cityGetName(city_id));
		sendSystemMessageProse(target, pp);
		pp = prose.getPackage(SID_RIGHTS_GRANTED_SELF, getName(target));
		sendSystemMessageProse(player, pp);
		
		return;
	}
	
	
	public void handleStorytellerZoningRights(obj_id player, obj_id target, int city_id) throws InterruptedException
	{
		
		if (!isIdValid(player) || !isIdValid(target))
		{
			return;
		}
		if (target == player)
		{
			return;
		}
		if (!isPlayer(target))
		{
			return;
		}
		
		obj_id mayor = cityGetLeader(city_id);
		
		if ((mayor != player) && !city.isMilitiaOfCity(player, city_id))
		{
			sendSystemMessage(player, SID_ST_GRANT_RIGHTS_FAIL);
			return;
		}
		
		if (hasObjVar(target, "city.st_zoning_rights"))
		{
			
			int[] city_id_array = getIntArrayObjVar(target, "city.st_zoning_rights");
			int[] zoning_time_array = getIntArrayObjVar(target, "city.st_zoning_rights_time");
			Vector ids = new Vector();
			if (ids != null)
			{
				ids.setSize(city_id_array.length);
				for (int _i = 0; _i < city_id_array.length; ++_i)
				{
					ids.set(_i, new Integer(city_id_array[_i]));
				}
			}
			Vector times = new Vector();
			if (times != null)
			{
				times.setSize(zoning_time_array.length);
				for (int _i = 0; _i < zoning_time_array.length; ++_i)
				{
					times.set(_i, new Integer(zoning_time_array[_i]));
				}
			}
			
			for (int i = 0; i < city_id_array.length; i++)
			{
				testAbortScript();
				if (city_id_array[i] == city_id)
				{
					
					int curt = getGameTime();
					if (curt - zoning_time_array[i] < 24*60*60)
					{
						
						ids.removeElementAt(i);
						times.removeElementAt(i);
						sendSystemMessage(player, SID_ST_RIGHTS_REVOKED);
						sendSystemMessage(target, SID_ST_RIGHTS_REVOKED_OTHER);
						
						int[] stampIdsArray = new int[0];
						if (ids != null)
						{
							stampIdsArray = new int[ids.size()];
							for (int _i = 0; _i < ids.size(); ++_i)
							{
								stampIdsArray[_i] = ((Integer)ids.get(_i)).intValue();
							}
						}
						int[] stampTimesArray = new int[0];
						if (times != null)
						{
							stampTimesArray = new int[times.size()];
							for (int _i = 0; _i < times.size(); ++_i)
							{
								stampTimesArray[_i] = ((Integer)times.get(_i)).intValue();
							}
						}
						setObjVar(target, "city.st_zoning_rights", stampIdsArray);
						setObjVar(target, "city.st_zoning_rights_time", stampTimesArray);
						return;
					}
					else
					{
						
						ids.removeElementAt(i);
						times.removeElementAt(i);
						
						int[] stampIdsArray = new int[0];
						if (ids != null)
						{
							stampIdsArray = new int[ids.size()];
							for (int _i = 0; _i < ids.size(); ++_i)
							{
								stampIdsArray[_i] = ((Integer)ids.get(_i)).intValue();
							}
						}
						int[] stampTimesArray = new int[0];
						if (times != null)
						{
							stampTimesArray = new int[times.size()];
							for (int _i = 0; _i < times.size(); ++_i)
							{
								stampTimesArray[_i] = ((Integer)times.get(_i)).intValue();
							}
						}
						setObjVar(target, "city.st_zoning_rights", stampIdsArray);
						setObjVar(target, "city.st_zoning_rights_time", stampTimesArray);
						return;
					}
				}
			}
			
			prose_package pp = prose.getPackage(SID_ST_RIGHTS_GRANTED, cityGetName(city_id));
			sendSystemMessageProse(target, pp);
			pp = prose.getPackage(SID_ST_RIGHTS_GRANTED_SELF, getName(target));
			sendSystemMessageProse(player, pp);
			
			utils.addElement(ids, city_id);
			utils.addElement(times, getGameTime());
			
			int[] stampIdsArray = new int[0];
			if (ids != null)
			{
				stampIdsArray = new int[ids.size()];
				for (int _i = 0; _i < ids.size(); ++_i)
				{
					stampIdsArray[_i] = ((Integer)ids.get(_i)).intValue();
				}
			}
			int[] stampTimesArray = new int[0];
			if (times != null)
			{
				stampTimesArray = new int[times.size()];
				for (int _i = 0; _i < times.size(); ++_i)
				{
					stampTimesArray[_i] = ((Integer)times.get(_i)).intValue();
				}
			}
			setObjVar(target, "city.st_zoning_rights", stampIdsArray);
			setObjVar(target, "city.st_zoning_rights_time", stampTimesArray);
			
		}
		
		else
		{
			
			int[] id_array = new int[]
			{
				city_id
			};
			int[] time_array = new int[]
			{
				getGameTime()
			};
			setObjVar(target, "city.st_zoning_rights", id_array);
			setObjVar(target, "city.st_zoning_rights_time", time_array);
			prose_package pp = prose.getPackage(SID_ST_RIGHTS_GRANTED, cityGetName(city_id));
			sendSystemMessageProse(target, pp);
			pp = prose.getPackage(SID_ST_RIGHTS_GRANTED_SELF, getName(target));
			sendSystemMessageProse(player, pp);
		}
		return;
	}
	
	
	public int cmdGrantStorytellerZoningRights(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		
		if (!isIdValid(self) || !isIdValid(target))
		{
			return SCRIPT_CONTINUE;
		}
		obj_id player = self;
		obj_id top = getTopMostContainer(player);
		int city_id = getCityAtLocation(getLocation(top), 0);
		
		if (city_id <= 0)
		{
			sendSystemMessage(player, SID_NOT_IN_CITY_LIMITS);
			return SCRIPT_CONTINUE;
		}
		
		handleStorytellerZoningRights(player, target, city_id);
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int cmdInstallMissionTerminal(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		
		int city_id = city.checkMayorCity( self, true );
		if (city_id == 0)
		{
			return SCRIPT_CONTINUE;
		}
		
		String[] rawTerminalTypes = dataTableGetStringColumn( CITY_MISSION_TERMINALS, "STRING");
		String[] terminalTypes = new String[rawTerminalTypes.length+1];
		terminalTypes[0] = "@city/city:current_mt " + city.getMTCount( city_id ) + "/" + city.getMaxMTCount( city_id );
		for (int i=0; i<rawTerminalTypes.length; i++)
		{
			testAbortScript();
			terminalTypes[i+1] = rawTerminalTypes[i];
		}
		sui.listbox( self, self, "@city/city:job_d", sui.OK_CANCEL, "@city/city:job_n", terminalTypes, "handleInstallMissionTerminal");
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int handleInstallMissionTerminal(obj_id self, dictionary params) throws InterruptedException
	{
		int idx = sui.getListboxSelectedRow( params );
		if (idx < 0)
		{
			idx = 0;
		}
		int btn = sui.getIntButtonPressed( params );
		if (btn == sui.BP_CANCEL)
		{
			return SCRIPT_CONTINUE;
		}
		if (idx == 0)
		{
			return SCRIPT_CONTINUE;
		}
		
		int city_id = city.checkMayorCity( self, true );
		if (city_id == 0)
		{
			return SCRIPT_CONTINUE;
		}
		
		int maxmt = city.getMaxMTCount( city_id );
		int curmt = city.getMTCount( city_id );
		if (curmt+1 > maxmt)
		{
			sendSystemMessage( self, SID_NO_MORE_MT );
			return SCRIPT_CONTINUE;
		}
		
		int factional = dataTableGetInt( CITY_MISSION_TERMINALS, idx-1, "FACTION");
		if ((factional == 1) && !hasSkill( self, "social_politician_martial_04" ))
		{
			sendSystemMessage( self, SID_NO_FACTIONAL );
			return SCRIPT_CONTINUE;
		}
		
		obj_id structure = getTopMostContainer( self );
		if (isIdValid(structure) && (structure != self))
		{
			
			if (!player_structure.isCivic( structure ))
			{
				sendSystemMessage( self, SID_CIVIC_ONLY );
				return SCRIPT_CONTINUE;
			}
		}
		
		int cost = 1000;
		
		dictionary payparams = new dictionary();
		payparams.put( "city_id", city_id );
		payparams.put( "cost", cost );
		payparams.put( "idx", idx );
		transferBankCreditsToNamedAccount( cityGetCityHall( city_id ), money.ACCT_CITY, cost, "handleMTFeeSuccess", "handleMTFeeFail", payparams );
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int handleMTFeeSuccess(obj_id self, dictionary params) throws InterruptedException
	{
		int city_id = params.getInt( "city_id");
		int idx = params.getInt( "idx");
		String[] terminalTemplates = dataTableGetStringColumn( CITY_MISSION_TERMINALS, "TEMPLATE");
		city.addMissionTerminal( city_id, self, terminalTemplates[idx-1] );
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int handleMTFeeFail(obj_id self, dictionary params) throws InterruptedException
	{
		int cost = params.getInt( "cost");
		prose_package pp = prose.getPackage( SID_CITY_NO_MONEY, cost );
		sendSystemMessageProse( self, pp );
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int handleSTFeeSuccess(obj_id self, dictionary params) throws InterruptedException
	{
		int city_id = params.getInt( "city_id");
		int idx = params.getInt( "idx");
		String[] trainerTemplates = dataTableGetStringColumn( CITY_SKILL_TRAINERS, "TEMPLATE");
		city.addSkillTrainer( city_id, self, trainerTemplates[idx-1] );
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int handleSTFeeFail(obj_id self, dictionary params) throws InterruptedException
	{
		int cost = params.getInt( "cost");
		prose_package pp = prose.getPackage( SID_CITY_NO_MONEY, cost );
		sendSystemMessageProse( self, pp );
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int cmdCityInfo(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		
		int[] cities = getAllCityIds();
		if ((cities == null) || (cities.length == 0))
		{
			sendSystemMessage( self, new string_id( "city/city", "csr_no_cities") );
			return SCRIPT_CONTINUE;
		}
		
		String[] planets =
		{
			"corellia", "dantooine", "lok", "naboo", "rori", "talus", "tatooine"
		};
		java.util.Map counts = new java.util.HashMap();
		
		String[] cityPlanet = new String[cities.length];
		String[] cityName = new String[cities.length];
		for (int i=0; i<cities.length; i++)
		{
			testAbortScript();
			
			location cityLoc = cityGetLocation( cities[i] );
			cityPlanet[i] = cityLoc.area;
			cityName[i] = cityGetName( cities[i] );
			
			int count = 0;
			Integer icount = (Integer) counts.get( cityPlanet[i] );
			if (icount != null)
			{
				count = icount.intValue();
			}
			count++;
			counts.put( cityPlanet[i], new Integer( count ) );
		}
		
		String[] planetList = new String[planets.length];
		for (int i=0; i<planets.length; i++)
		{
			testAbortScript();
			Integer count = (Integer) counts.get( planets[i] );
			if (count == null)
			{
				planetList[i] = "\\#888888 " + planets[i] + " (0)";
			}
			else
			{
				planetList[i] = "\\#00FF00 " + planets[i] + "\\#FFFFFF (" + count + ")";
			}
		}
		
		utils.setScriptVar( self, "planetList", planetList );
		sui.listbox( self, self, "@city/city:cityplanets_d", sui.OK_CANCEL, "@city/city:cityinfo_n", planetList, "handleSelectPlanet");
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int handleSelectPlanet(obj_id self, dictionary params) throws InterruptedException
	{
		int idx = sui.getListboxSelectedRow( params );
		if (idx < 0)
		{
			idx = 0;
		}
		int btn = sui.getIntButtonPressed( params );
		if (btn == sui.BP_CANCEL)
		{
			return SCRIPT_CONTINUE;
		}
		
		String[] planets =
		{
			"corellia", "dantooine", "lok", "naboo", "rori", "talus", "tatooine"
		};
		String planet = planets[idx];
		int[] cities = getAllCityIds();
		
		int count = 0;
		int[] planetCities = new int[cities.length];
		for (int i=0; i<cities.length; i++)
		{
			testAbortScript();
			location cityLoc = cityGetLocation( cities[i] );
			if (cityLoc.area.equals( planet ))
			{
				planetCities[i] = 1;
				count++;
			}
			else
			{
				planetCities[i] = 0;
			}
		}
		
		String[] cityList = new String[count];
		int[] planetCityIds = new int[count];
		
		int j = 0;
		for (int i=0; i<planetCities.length; i++)
		{
			testAbortScript();
			if (planetCities[i] == 1)
			{
				int cityRank = city.getCityRank( cities[i] );
				String cityName = cityGetName( cities[i] );
				planetCityIds[j] = cities[i];
				cityList[j++] = "\\#00FF00 " + cityName + "\\#FFFFFF (" + cities[i] + ")" + "\\#BBBBBB Rank " + cityRank;
			}
		}
		
		utils.setScriptVar( self, "cities", planetCityIds );
		utils.setScriptVar( self, "cityList", cityList );
		sui.listbox( self, self, "@city/city:cityinfo_d", sui.OK_CANCEL, "@city/city:cityinfo_n", cityList, "handleSelectCity", true, false );
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int handleSelectCity(obj_id self, dictionary params) throws InterruptedException
	{
		int idx = sui.getListboxSelectedRow( params );
		if (idx < 0)
		{
			idx = 0;
		}
		int btn = sui.getIntButtonPressed( params );
		if (btn == sui.BP_CANCEL)
		{
			String[] planetList = utils.getStringArrayScriptVar( self, "planetList");
			sui.listbox( self, self, "@city/city:cityplanets_d", sui.OK_CANCEL, "@city/city:cityinfo_n", planetList, "handleSelectPlanet");
			return SCRIPT_CONTINUE;
		}
		
		int[] cities = utils.getIntArrayScriptVar( self, "cities");
		int city_id = cities[idx];
		
		String cityName = cityGetName( city_id );
		obj_id cityHall = cityGetCityHall( city_id );
		location cityLoc = cityGetLocation( city_id );
		obj_id mayor = cityGetLeader( city_id );
		String mayorName = cityGetCitizenName( city_id, mayor );
		obj_id[] citizens = cityGetCitizenIds( city_id );
		obj_id[] structures = cityGetStructureIds( city_id );
		int cityRank = city.getCityRank( city_id );
		int numCitizens = citizens.length;
		int numStructures = structures.length;
		
		int incomeTax = cityGetIncomeTax( city_id );
		int propertyTax = cityGetPropertyTax( city_id );
		int salesTax = cityGetSalesTax( city_id );
		
		String[] cityInfo = new String[9];
		
		cityInfo[0] = "\\#00FF00 Name: \\#FFFFFF " + cityName;
		cityInfo[1] = "\\#00FF00 Hall ID: \\#FFFFFF " + cityHall;
		cityInfo[2] = "\\#00FF00 Location: \\#FFFFFF " + cityLoc;
		cityInfo[3] = "\\#00FF00 Mayor: \\#FFFFFF " + mayorName + " (" + mayor + ")";
		cityInfo[4] = "\\#00FF00 Rank: \\#FFFFFF " + cityRank;
		cityInfo[5] = "\\#00FF00 # Citizens: \\#FFFFFF " + numCitizens;
		cityInfo[6] = "\\#00FF00 # Structures: \\#FFFFFF " + numStructures;
		cityInfo[7] = "\\#00FF00 Taxes (Income/Property/Sales): \\#FFFFFF " + incomeTax + "/" + propertyTax + "/" + salesTax;
		cityInfo[8] = "\\#00FF00 Voting Information";
		
		utils.setScriptVar( self, "cityinfo", city_id );
		utils.setScriptVar( self, "cityloc", cityLoc );
		
		utils.setScriptVar( self, "cityInfo", cityInfo );
		sui.listbox( self, self, "@city/city:cityinfos_d", sui.OK_CANCEL, "@city/city:cityinfos_n", cityInfo, "handleSelectCityInfo");
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int handleSelectCityInfo(obj_id self, dictionary params) throws InterruptedException
	{
		int idx = sui.getListboxSelectedRow( params );
		if (idx < 0)
		{
			idx = 0;
		}
		int btn = sui.getIntButtonPressed( params );
		if (btn == sui.BP_CANCEL)
		{
			String[] cityList = utils.getStringArrayScriptVar( self, "cityList");
			sui.listbox( self, self, "@city/city:cityinfo_d", sui.OK_CANCEL, "@city/city:cityinfo_n", cityList, "handleSelectCity", true, false );
			return SCRIPT_CONTINUE;
		}
		
		int city_id = utils.getIntScriptVar( self, "cityinfo");
		
		if (idx == 2)
		{
			
			location loc = utils.getLocationScriptVar( self, "cityloc");
			warpPlayer( self, loc.area, loc.x, loc.y, loc.z, loc.cell, 0, 0, 0, null, false );
		}
		else if (idx == 5)
		{
			
			obj_id[] citizens = cityGetCitizenIds( city_id );
			String[] civInfo = new String[citizens.length];
			
			for (int i=0; i<citizens.length; i++)
			{
				testAbortScript();
				String militia = "";
				if (city.hasMilitiaFlag( citizens[i], city_id ))
				{
					militia = "\\#FF0000 Militia";
				}
				
				String citName = cityGetCitizenName( city_id, citizens[i] );
				civInfo[i] = "\\#00FF00 " + citName + "\\#FFFFFF (" + citizens[i] + ")" + militia;
			}
			
			sui.listbox( self, self, "@city/city:civinfo_d", sui.OK_CANCEL, "@city/city:cityinfo_n", civInfo, "handleNothing", true, false );
		}
		else if (idx == 6)
		{
			
			obj_id[] structures = cityGetStructureIds( city_id );
			String[] strInfo = new String[structures.length];
			
			for (int i=0; i<structures.length; i++)
			{
				testAbortScript();
				int typeflags = cityGetStructureType( city_id, structures[i] );
				String typestring = "";
				if (0 != (typeflags & city.SF_MISSION_TERMINAL))
				{
					typestring = "\\#AAAAAA (Mission Terminal)";
				}
				else if (0 != (typeflags & city.SF_SKILL_TRAINER))
				{
					typestring = "\\#AAAAAA (Skill Trainer)";
				}
				else if (0 != (typeflags & city.SF_DECORATION))
				{
					typestring = "\\#AAAAAA (Decoration)";
				}
				else if (0 != (typeflags & city.SF_COST_CITY_HALL))
				{
					typestring = "\\#AAAAAA (City Hall)";
				}
				
				strInfo[i] = "\\#00FF00 " + getTemplateName( structures[i] ) + "\\#FFFFFF (" + structures[i] + ")" + typestring;
			}
			
			sui.listbox( self, self, "@city/city:strinfo_d", sui.OK_CANCEL, "@city/city:cityinfo_n", strInfo, "handleNothing", true, false );
		}
		else if (idx == 8)
		{
			
			Vector vote_names = new Vector();
			vote_names.setSize(0);
			Vector vote_ids = new Vector();
			vote_ids.setSize(0);
			Vector vote_counts = new Vector();
			vote_counts.setSize(0);
			obj_id[] citizens = cityGetCitizenIds( city_id );
			for (int i=0; i<citizens.length; i++)
			{
				testAbortScript();
				if (!isIdValid( citizens[i] ))
				{
					continue;
				}
				
				boolean found = false;
				
				obj_id vote = cityGetCitizenAllegiance( city_id, citizens[i] );
				if (isIdValid(vote))
				{
					for (int j=0; (j < vote_ids.size()); j++)
					{
						testAbortScript();
						
						if (((obj_id)(vote_ids.get(j))) == vote)
						{
							vote_counts.set(j, new Integer(((Integer)(vote_counts.get(j))).intValue() + 1));
							found = true;
							break;
						}
					}
					
					if (!found)
					{
						
						utils.addElement( vote_ids, vote );
						utils.addElement( vote_counts, 1 );
						utils.addElement( vote_names, cityGetCitizenName( city_id, citizens[i] ) );
					}
				}
			}
			
			utils.setScriptVar( self, "votes_ids", vote_ids );
			
			obj_id mayor = cityGetLeader( city_id );
			String[] voteInfo = new String[vote_names.size()];
			for (int i=0; i<voteInfo.length; i++)
			{
				testAbortScript();
				if (((obj_id)(vote_ids.get(i))) == mayor)
				{
					voteInfo[i] = "\\#00FF00 "+ ((String)(vote_names.get(i))) + "\\#AAAAFF Votes: " + ((Integer)(vote_counts.get(i))).intValue() + "\\#FFFFFF (" + ((obj_id)(vote_ids.get(i))) + ") " + "\\#AAAAAA (Incumbent)";
				}
				else
				{
					voteInfo[i] = "\\#00FF00 "+ ((String)(vote_names.get(i))) + "\\#AAAAFF Votes: " + ((Integer)(vote_counts.get(i))).intValue() + "\\#FFFFFF (" + ((obj_id)(vote_ids.get(i))) + ")";
				}
			}
			
			sui.listbox( self, self, "@city/city:voteinfo_d", sui.OK_CANCEL, "@city/city:cityinfo_n", voteInfo, "handleSelectRunner", true, false );
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int handleSelectRunner(obj_id self, dictionary params) throws InterruptedException
	{
		int idx = sui.getListboxSelectedRow( params );
		if (idx < 0)
		{
			idx = 0;
		}
		int btn = sui.getIntButtonPressed( params );
		if (btn == sui.BP_CANCEL)
		{
			return SCRIPT_CONTINUE;
		}
		
		int city_id = utils.getIntScriptVar( self, "cityinfo");
		obj_id[] runners = utils.getObjIdArrayScriptVar( self, "votes_ids");
		obj_id runner = runners[idx];
		
		Vector vote_names = new Vector();
		vote_names.setSize(0);
		Vector vote_ids = new Vector();
		vote_ids.setSize(0);
		obj_id[] citizens = cityGetCitizenIds( city_id );
		for (int i=0; i<citizens.length; i++)
		{
			testAbortScript();
			if (!isIdValid( citizens[i] ))
			{
				continue;
			}
			boolean found = false;
			obj_id vote = cityGetCitizenAllegiance( city_id, citizens[i] );
			if (vote == runner)
			{
				utils.addElement( vote_ids, citizens[i] );
				utils.addElement( vote_names, cityGetCitizenName( city_id, citizens[i] ) );
			}
		}
		
		String[] voteInfo = new String[vote_names.size()];
		for (int i=0; i<voteInfo.length; i++)
		{
			testAbortScript();
			voteInfo[i] = "\\#00FF00 "+ ((String)(vote_names.get(i))) + "\\#FFFFFF ("+ ((obj_id)(vote_ids.get(i))) +")";
		}
		
		sui.listbox( self, self, "@city/city:voterinfo_d", sui.OK_CANCEL, "@city/city:cityinfo_n", voteInfo, "handleNothing", true, false );
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int OnSkillGranted(obj_id self, String skillName) throws InterruptedException
	{
		if (!hasObjVar(self, "clickRespec.granting") && !hasObjVar(self, "npcRespec.inProgress"))
		{
			if (getLevel( self ) <= 1)
			{
				playClientEffectObj(self, "clienteffect/skill_granted.cef", self, null);
				showFlyText(self, new string_id("cbt_spam", "skill_up"), 2.5f, colors.YELLOWGREEN);
			}
		}
		
		badge.grantMasterSkillBadge( self, skillName );
		
		static_item.validateWornEffects(self);
		
		if (skillName.endsWith("_novice"))
		{
			
			if (!setupNovicePilotSkill( self, skillName ))
			{
				return SCRIPT_OVERRIDE;
			}
		}
		
		if (!allowedBySpaceExpansion( self, skillName ))
		{
			return SCRIPT_OVERRIDE;
		}
		
		if (skillName.startsWith("expertise_"))
		{
			expertise.cacheExpertiseProcReacList(self);
			armor.recalculateArmorForPlayer(self);
			
			if (utils.isProfession(self, utils.SMUGGLER))
			{
				messageTo(self, "applySmugglingBonuses", null, 1.0f, false);
			}
			
			if (utils.isProfession(self, utils.FORCE_SENSITIVE))
			{
				if (buff.isInStance(self))
				{
					buff.removeBuff(self, jedi.JEDI_STANCE);
					buff.applyBuff(self, jedi.JEDI_STANCE);
				}
				
				if (buff.isInFocus(self))
				{
					buff.removeBuff(self, jedi.JEDI_FOCUS);
					buff.applyBuff(self, jedi.JEDI_FOCUS);
				}
			}
			
			if (!expertise.isProfAllowedSkill(self, skillName))
			{
				
				revokeSkillSilent(self, skillName);
				return SCRIPT_OVERRIDE;
			}
			
			obj_id tempWeapon = getObjectInSlot(self, "hold_r");
			
			if (isIdValid(tempWeapon))
			{
				weapons.adjustWeaponRangeForExpertise(self, tempWeapon, true);
			}
			
			skill.recalcPlayerPools(self, false);
		}
		
		recomputeCommandSeries(self);
		
		beast_lib.verifyAndUpdateCalledBeastStats(self);
		
		trial.bumpSession(self, "displayDefensiveMods");
		messageTo(self, "setDisplayOnlyDefensiveMods", trial.getSessionDict(self, "displayDefensiveMods") , 5, false);
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int handleStartJediKnightTrials(obj_id self, dictionary params) throws InterruptedException
	{
		if (!hasScript(self, jedi_trials.KNIGHT_TRIALS_SCRIPT))
		{
			attachScript(self, jedi_trials.KNIGHT_TRIALS_SCRIPT);
		}
		return SCRIPT_CONTINUE;
	}
	
	
	public int OnStartCharacterUpload(obj_id self, byte[] results, modifiable_int resultSize, boolean w, boolean ao) throws InterruptedException
	{
		Boolean withItems = new Boolean(w);
		dictionary params = new dictionary();
		params.put("withItems", w);
		params.put("allowOverride", ao);
		Object[] triggerParams = new Object[2];
		triggerParams[0] = self;
		triggerParams[1] = params;
		
		CustomerServiceLog("CharacterTransfer", "OnStartCharacterUpload(withItems="+ withItems + ", allowOverride="+ ao + ")");
		
		resultSize.set(0);
		String result = null;
		try
		{
			int err = script_entry.runScripts("OnUploadCharacter", triggerParams);
			if (err == SCRIPT_CONTINUE)
			{
				byte[] tmp = params.pack();
				if (tmp.length < results.length)
				{
					int i;
					for (i = 0; i < tmp.length; ++i)
					{
						testAbortScript();
						results[i] = tmp[i];
					}
					resultSize.set(tmp.length);
				}
				else
				{
					CustomerServiceLog("CharacterTransfer", "OnStartCharacterUpload(withItems="+ withItems + ") tmp.length="+ tmp.length + " is >= results.length="+ results.length + ", not enough buffer space to pack the dictionary! character transfer will FAIL");
					return SCRIPT_OVERRIDE;
				}
			}
			else
			{
				CustomerServiceLog("CharacterTransfer", "OnStartCharacterUpload(withItems="+ withItems + ") runScripts(\"OnUploadCharacter\") returned SCRIPT_OVERRIDE, character transfer will FAIL");
				final string_id mailTransferFailedScriptError = new string_id("character_transfer", "failed_internal_script_error");
				final string_id mailTransferFailedSubject = new string_id("character_transfer", "failed_subject");
				utils.sendMail(mailTransferFailedScriptError, mailTransferFailedScriptError, self, "System");
				
				resultSize.set(0);
				return SCRIPT_OVERRIDE;
			}
		}
		catch(Throwable t)
		{
			CustomerServiceLog("CharacterTransfer", "OnStartCharacterUpload(withItems="+ withItems + ") : an exception was thrown, character transfer will FAIL : "+ t);
			final string_id mailTransferFailedScriptError = new string_id("character_transfer", "failed_internal_script_error");
			final string_id mailTransferFailedSubject = new string_id("character_transfer", "failed_subject");
			utils.sendMail(mailTransferFailedScriptError, mailTransferFailedScriptError, self, "System");
			
			result = null;
			return SCRIPT_OVERRIDE;
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public boolean ctsCheckAndLogUniqueStaticItemAlreadyExists(obj_id container, dictionary itemDictionary) throws InterruptedException
	{
		if (isIdNull(container) || (itemDictionary == null))
		{
			return false;
		}
		
		if (itemDictionary.containsKey("staticItemName"))
		{
			String staticItemName = itemDictionary.getString("staticItemName");
			if (staticItemName == null)
			{
				return false;
			}
			
			if (!static_item.isUniqueStaticItem(staticItemName))
			{
				return false;
			}
			
			obj_id containerOwnerId = getOwner(container);
			if (isIdNull(containerOwnerId))
			{
				return false;
			}
			
			if (!isPlayer(containerOwnerId))
			{
				return false;
			}
			
			if (utils.playerHasStaticItemInBankOrInventory(containerOwnerId, staticItemName))
			{
				CustomerServiceLog("CharacterTransfer", "unique static item "+ staticItemName + " already exists");
				return true;
			}
			
			if (utils.playerHasStaticItemInAppearanceInventory(containerOwnerId, staticItemName))
			{
				CustomerServiceLog("CharacterTransfer", "unique static item "+ staticItemName + " already exists");
				return true;
			}
		}
		
		return false;
	}
	
	
	public boolean ctsHasExceededResourceTransferLimit(int crateCountLimit, int currentCrateCount, long unitCountLimit, long currentUnitCount) throws InterruptedException
	{
		if ((crateCountLimit > 0) && (currentCrateCount >= crateCountLimit))
		{
			return true;
		}
		
		if ((unitCountLimit > 0L) && (currentUnitCount >= unitCountLimit))
		{
			return true;
		}
		
		return false;
	}
	
	
	public obj_id unpackItem(obj_id container, dictionary itemDictionary) throws InterruptedException
	{
		obj_id newItem = null;
		
		try
		{
			obj_id self = getSelf();
			if (!isIdValid(self))
			{
				return null;
			}
			
			LIVE_LOG("CharacterTransfer", "unpackItem("+ container + ", "+ itemDictionary + ")");
			
			if (! itemDictionary.containsKey("objectTemplateCrc"))
			{
				CustomerServiceLog("CharacterTransfer", "unpackItem() : could not retrieve object template crc from item dictionary. TRANSFER FAILED");
				return null;
			}
			int objectTemplateCrc = itemDictionary.getInt("objectTemplateCrc");
			
			String objectTemplateName = "UNKNOWN ITEM";
			if (itemDictionary.containsKey("objectTemplateName"))
			{
				objectTemplateName = itemDictionary.getString("objectTemplateName");
			}
			
			boolean isEquipped = false;
			if (itemDictionary.containsKey("isEquipped"))
			{
				isEquipped = itemDictionary.getBoolean("isEquipped");
			}
			
			boolean isAppearanceEquipped = false;
			if (itemDictionary.containsKey("isAppearanceEquipped"))
			{
				isAppearanceEquipped = itemDictionary.getBoolean("isAppearanceEquipped");
			}
			
			if (itemDictionary.containsKey("staticItemName"))
			{
				String staticItemName = itemDictionary.getString("staticItemName");
				newItem = static_item.createNewItemFunction(staticItemName, container);
				CustomerServiceLog("CharacterTransfer", "unpackItem() : createNewItemFunction("+ staticItemName + ", "+ container + ") = "+ newItem);
			}
			else if (itemDictionary.containsKey("msoDraftSchematicCrc"))
			{
				int schematicCrc = itemDictionary.getInt("msoDraftSchematicCrc");
				newItem = createSchematic(schematicCrc, container);
				CustomerServiceLog("CharacterTransfer", "unpackItem() : createSchematic("+ schematicCrc + ", "+ container + ") = "+ newItem);
			}
			else if (itemDictionary.containsKey("schematicTemplateCrc"))
			{
				
				return null;
			}
			else
			{
				if (isEquipped || isAppearanceEquipped)
				{
					
					newItem = createObjectInInventoryAllowOverload(objectTemplateCrc, self);
					CustomerServiceLog("CharacterTransfer", "unpackItem() : createObjectInInventoryAllowOverload("+ objectTemplateCrc + "="+ objectTemplateName + ", "+ self + ") = "+ newItem);
				}
				else if (itemDictionary.containsKey("bypassVolumeCheck"))
				{
					newItem = createObjectInInventoryAllowOverload(objectTemplateCrc, self);
				}
				else
				{
					int containerType = getContainerType(container);
					if (containerType == 2)
					{
						newItem = createObjectOverloaded(objectTemplateCrc, container);
					}
					else
					{
						newItem = createObject(objectTemplateCrc, container, "");
					}
					CustomerServiceLog("CharacterTransfer", "unpackItem() : createObjectOverloaded("+ objectTemplateCrc + "="+ objectTemplateName + ", "+ container + ") = "+ newItem);
				}
			}
			
			if (!isIdValid(newItem))
			{
				CustomerServiceLog("CharacterTransfer", "unpackItem() : FAILED to create object in container "+ container + ": "+ itemDictionary);
				return null;
			}
			else
			{
				setOwner(newItem, self);
				
				utils.setLocalVar(newItem, "ctsBeingUnpacked", true);
			}
			
			if (! itemDictionary.containsKey("packedObjvars"))
			{
				CustomerServiceLog("CharacterTransfer", "unpackItem() : FAILED to retrieve packedObjvars from item dictionary: "+ itemDictionary);
				return null;
			}
			
			String packedObjvars = itemDictionary.getString("packedObjvars");
			if (packedObjvars == null)
			{
				CustomerServiceLog("CharacterTransfer", "unpackItem() : FAILED to get a non-null packedObjvars value from item dictionary: "+ itemDictionary);
				return null;
			}
			
			setPackedObjvars(newItem, packedObjvars);
			
			int objectType = getGameObjectType(newItem);
			if (!static_item.isStaticItem(newItem))
			{
				
				if (itemDictionary.containsKey("attribute_types") && itemDictionary.containsKey("attribute_values"))
				{
					int[] itemAttribTypes = itemDictionary.getIntArray("attribute_types");
					int[] itemAttribValues = itemDictionary.getIntArray("attribute_values");
					if (itemAttribTypes != null && itemAttribValues != null)
					{
						attribute[] itemAttribs = new attribute[itemAttribTypes.length];
						int itemAttribsIter;
						for (itemAttribsIter = 0; itemAttribsIter < itemAttribTypes.length; ++itemAttribsIter)
						{
							testAbortScript();
							itemAttribs[itemAttribsIter] = new attribute(itemAttribTypes[itemAttribsIter], itemAttribValues[itemAttribsIter]);
						}
						if (!setMaxAttribs(newItem, itemAttribs))
						{
							CustomerServiceLog("CharacterTransfer", "setMaxAttribs( "+ newItem + ", ...) FAILED");
						}
						if (!setAttribs(newItem, itemAttribs))
						{
							CustomerServiceLog("CharacterTransfer", "setAttribs( "+ newItem + ", ...) FAILED");
						}
					}
				}
				
				if (itemDictionary.containsKey("maxHitpoints"))
				{
					int maxHitpoints = itemDictionary.getInt("maxHitpoints");
					if (!setMaxHitpoints(newItem, maxHitpoints))
					{
						CustomerServiceLog("CharacterTransfer", "setMaxHitpoints( "+ newItem + ", "+ maxHitpoints + ") FAILED");
					}
				}
				
				if (itemDictionary.containsKey("hitpoints"))
				{
					int hitpoints = itemDictionary.getInt("hitpoints");
					if (!setInvulnerableHitpoints(newItem, hitpoints))
					{
						CustomerServiceLog("CharacterTransfer", "setInvulnerableHitpoints( "+ newItem + ", "+ hitpoints + ") FAILED");
					}
				}
				
				if (itemDictionary.containsKey("conditionFlags"))
				{
					int conditionFlags = itemDictionary.getInt("conditionFlags");
					if (!setCondition(newItem, conditionFlags))
					{
						CustomerServiceLog("CharacterTransfer", "setCondition( "+ newItem + ", "+ conditionFlags + ") FAILED");
					}
				}
				
				if (itemDictionary.containsKey("scripts"))
				{
					String[] itemScripts = itemDictionary.getStringArray("scripts");
					if (itemScripts != null && itemScripts.length > 0)
					{
						int scriptIter;
						for (scriptIter = 0; scriptIter < itemScripts.length; ++scriptIter)
						{
							testAbortScript();
							if (!hasScript(newItem, itemScripts[scriptIter]))
							{
								attachScript(newItem, itemScripts[scriptIter]);
							}
						}
					}
				}
				
				if (itemDictionary.containsKey("objectName"))
				{
					String objectName = itemDictionary.getString("objectName");
					setName(newItem, objectName);
				}
				
				if (itemDictionary.containsKey("isWeaponItem"))
				{
					boolean isWeaponItem = itemDictionary.getBoolean("isWeaponItem");
					if (isWeaponItem)
					{
						
						if (itemDictionary.containsKey("currentVersion"))
						{
							
							int currentVersion = itemDictionary.getInt("currentVersion");
							setConversionId(newItem, currentVersion);
						}
						
						int minDamage = itemDictionary.getInt("minDamage");
						if (!setWeaponMinDamage(newItem, minDamage))
						{
							CustomerServiceLog("CharacterTransfer", "setWeaponMinDamage( "+ newItem + ", "+ minDamage + ") FAILED");
						}
						
						int maxDamage = itemDictionary.getInt("maxDamage");
						if (!setWeaponMaxDamage(newItem, maxDamage))
						{
							CustomerServiceLog("CharacterTransfer", "setWeaponMaxDamage( "+ newItem + ", "+ maxDamage + ") FAILED");
						}
						
						float attackSpeed = itemDictionary.getFloat("attackSpeed");
						if (!setWeaponAttackSpeed(newItem, attackSpeed))
						{
							CustomerServiceLog("CharacterTransfer", "setWeaponAttackSpeed( "+ newItem + ", "+ attackSpeed + ") FAILED");
						}
						
						range_info rangeInfo = new range_info();
						rangeInfo.minRange = itemDictionary.getFloat("rangeInfoMinRange");
						rangeInfo.maxRange = itemDictionary.getFloat("rangeInfoMaxRange");
						if (!setWeaponRangeInfo(newItem, rangeInfo))
						{
							CustomerServiceLog("CharacterTransfer", "setWeaponRangeInfo( "+ newItem + ", "+ rangeInfo + ") FAILED");
						}
						
						float woundChance = itemDictionary.getFloat("woundChance");
						if (!setWeaponWoundChance(newItem, woundChance))
						{
							CustomerServiceLog("CharacterTransfer", "setWeaponWoundChance( "+ newItem + ", "+ woundChance + ") FAILED");
						}
						
						float damageRadius = itemDictionary.getFloat("damageRadius");
						if (!setWeaponDamageRadius(newItem, damageRadius))
						{
							CustomerServiceLog("CharacterTransfer", "setWeaponDamageRadius( "+ newItem + ", "+ damageRadius + ") FAILED");
						}
						
						int accuracy = itemDictionary.getInt("accuracy");
						if (!setWeaponAccuracy(newItem, accuracy))
						{
							CustomerServiceLog("CharacterTransfer", "setWeaponAccuracy( "+ newItem + ", "+ accuracy + ") FAILED");
						}
						
						int elementalType = itemDictionary.getInt("elementalType");
						if (!setWeaponElementalType(newItem, elementalType))
						{
							CustomerServiceLog("CharacterTransfer", "setWeaponElemetalType( "+ newItem + ", "+ elementalType + ") FAILED");
						}
						
						int elementalValue = itemDictionary.getInt("elementalValue");
						if (!setWeaponElementalValue(newItem, elementalValue))
						{
							CustomerServiceLog("CharacterTransfer", "setWeaponElemetalValue( "+ newItem + ", "+ elementalValue + ") FAILED");
						}
						
						int damageType = itemDictionary.getInt("damageType");
						if (!setWeaponDamageType(newItem, damageType))
						{
							CustomerServiceLog("CharacterTransfer", "setWeaponDamageType( "+ newItem + ", "+ damageType + ") FAILED");
						}
						
						int attackCost = itemDictionary.getInt("attackCost");
						if (!setWeaponAttackCost(newItem, attackCost))
						{
							CustomerServiceLog("CharacterTransfer", "setWeaponAttackCost( "+ newItem + ", "+ attackCost + ") FAILED");
						}
						
						weapons.setWeaponData(newItem);
					}
				}
				
				if (itemDictionary.containsKey("colorData"))
				{
					dictionary colorData = itemDictionary.getDictionary("colorData");
					hue.setPalcolorData(newItem, colorData);
				}
				
				if (itemDictionary.containsKey("customAppearance"))
				{
					byte[] customAppearance = itemDictionary.getByteArray("customAppearance");
					setAutoVariableFromByteStream(newItem, "customAppearance", customAppearance);
				}
				
				if (itemDictionary.containsKey("appearanceData"))
				{
					byte[] appearanceData = itemDictionary.getByteArray("appearanceData");
					setAutoVariableFromByteStream(newItem, "appearanceData", appearanceData);
				}
			}
			
			if (hasCondition(newItem, CONDITION_LOCKED))
			{
				clearUserAccessList(newItem);
				clearGuildAccessList(newItem);
				addUserToAccessList(newItem, self);
				
				if (hasObjVar(newItem, "lock_owner"))
				{
					setObjVar(newItem, "lock_owner", self);
				}
				if (hasObjVar(newItem, "lock_owner_name"))
				{
					setObjVar(newItem, "lock_owner_name", getName(self));
				}
			}
			
			if (itemDictionary.containsKey("count"))
			{
				int count = itemDictionary.getInt("count");
				if (!setCount(newItem, count))
				{
					CustomerServiceLog("CharacterTransfer", "setCount( "+ newItem + ", "+ count + ") FAILED");
				}
			}
			
			if (itemDictionary.containsKey("bankBalance"))
			{
				byte[] bankBalance = itemDictionary.getByteArray("bankBalance");
				setAutoVariableFromByteStream(newItem, "bankBalance", bankBalance);
			}
			
			if (isGameObjectTypeOf(objectType, GOT_ship))
			{
				
				if ((objectTemplateName != null) && (objectTemplateName.endsWith("player_sorosuub_space_yacht.iff")))
				{
					
				}
				else
				{
					
					space_crafting.uninstallAll(newItem);
				}
				
				float mass = itemDictionary.getFloat("ship_mass");
				float maxHp = itemDictionary.getFloat("ship_hp");
				float currentHp = itemDictionary.getFloat("ship_current_hp");
				
				setChassisComponentMassMaximum(newItem, mass);
				setShipMaximumChassisHitPoints(newItem, maxHp);
				setShipCurrentChassisHitPoints(newItem, currentHp);
				
				if (itemDictionary.containsKey("droid_control_device"))
				{
					obj_id objDroidControlDevice = itemDictionary.getObjId("droid_control_device");
					if (isIdValid(objDroidControlDevice))
					{
						utils.setLocalVar(newItem, "droid_control_device", objDroidControlDevice);
					}
				}
				
				if (itemDictionary.containsKey("shipComponents"))
				{
					
					obj_id datapad = getContainedBy(container);
					obj_id player = getOwner(datapad);
					
					if (isIdValid(player))
					{
						dictionary componentsDictionary = itemDictionary.getDictionary("shipComponents");
						if (componentsDictionary != null)
						{
							Set keySet = componentsDictionary.keySet();
							Integer[] slots = new Integer[keySet.size()];
							keySet.toArray(slots);
							for (int i = 0; i < slots.length; ++i)
							{
								testAbortScript();
								
								Integer slot = slots[i];
								
								dictionary componentDictionary = componentsDictionary.getDictionary(slot);
								if (ctsCheckAndLogUniqueStaticItemAlreadyExists(player, componentDictionary))
								{
									continue;
								}
								
								componentDictionary.put("bypassVolumeCheck", 1);
								obj_id newComponent = unpackItem(player, componentsDictionary.getDictionary(slot));
								
								if (isIdValid(newComponent))
								{
									shipInstallComponent(player, newItem, slot.intValue(), newComponent);
								}
							}
						}
					}
				}
			}
			
			if (itemDictionary.containsKey("isCrystalOwner"))
			{
				
				if (hasObjVar(self, "preserve_existing_lightsaber_crystal_owner_objvar"))
				{
					String namePrefix = getStringObjVar(self, "preserve_existing_lightsaber_crystal_owner_objvar");
					if ((namePrefix != null) && (namePrefix.length() > 0) && !namePrefix.equals("none"))
					{
						String existingName = getStringObjVar(newItem, jedi.VAR_CRYSTAL_OWNER_NAME);
						if ((existingName != null) && (existingName.length() > 0))
						{
							setObjVar(newItem, jedi.VAR_CRYSTAL_OWNER_NAME, namePrefix + existingName);
						}
					}
					
					setObjVar(newItem, jedi.VAR_CRYSTAL_OWNER_ID, 0);
				}
				else
				{
					if (itemDictionary.getBoolean("isCrystalOwner"))
					{
						setObjVar(newItem, jedi.VAR_CRYSTAL_OWNER_ID, self);
						setObjVar(newItem, jedi.VAR_CRYSTAL_OWNER_NAME, getName(self));
					}
					else
					{
						setObjVar(newItem, jedi.VAR_CRYSTAL_OWNER_ID, 0);
						setObjVar(newItem, jedi.VAR_CRYSTAL_OWNER_NAME, "None");
					}
				}
				
				String name = getEncodedName(newItem);
				if (name.startsWith("@"))
				{
					name = localize(getNameStringId(newItem));
				}
				name = "\\"+colors_hex.GREEN+""+name + " (tuned)\\#.";
				setName(newItem, name);
			}
			
			if (itemDictionary.containsKey("biolink"))
			{
				obj_id biolink = itemDictionary.getObjId("biolink");
				if (itemDictionary.getBoolean("isBiolinkOwner"))
				{
					biolink = self;
				}
				if (!setBioLink(newItem, biolink))
				{
					CustomerServiceLog("CharacterTransfer", "setBioLink( "+ newItem + ", "+ biolink + ") FAILED");
				}
			}
			
			if (isGameObjectTypeOf(objectType, GOT_resource_container))
			{
				
				if (objectType != GOT_resource_container_pseudo)
				{
					if (itemDictionary.containsKey("resourceQuantity") && itemDictionary.containsKey("resourceData"))
					{
						int quantity = itemDictionary.getInt("resourceQuantity");
						String resourceData = itemDictionary.getString("resourceData");
						
						CustomerServiceLog("CharacterTransfer", "unpackItem() : created resource container "+ newItem + ": resourceQuantity="+ quantity + ", resourceData="+ resourceData);
						
						setResourceCtsData(newItem, quantity, resourceData);
					}
				}
				else
				{
					CustomerServiceLog("CharacterTransfer", "unpackItem() : created pseudo resource container "+ newItem);
				}
			}
			
			if (objectType == GOT_misc_factory_crate)
			{
				recomputeCrateAttributes(newItem);
			}
			
			if ((objectType == GOT_data_manufacturing_schematic) && itemDictionary.containsKey("msoCtsPackUnpack"))
			{
				byte[] data = itemDictionary.getByteArray("msoCtsPackUnpack");
				setAutoVariableFromByteStream(newItem, "msoCtsPackUnpack", data);
				
				if (itemDictionary.containsKey("msoConversionId"))
				{
					setConversionId(newItem, itemDictionary.getInt("msoConversionId"));
				}
			}
			
			if (objectType == GOT_data_mission_object)
			{
				if (itemDictionary.containsKey("moCtsPackUnpack"))
				{
					byte[] data = itemDictionary.getByteArray("moCtsPackUnpack");
					setAutoVariableFromByteStream(newItem, "moCtsPackUnpack", data);
				}
				
				if (itemDictionary.containsKey("missionAge"))
				{
					int timeCreated = (getGameTime() - itemDictionary.getInt("missionAge"));
					if (timeCreated <= 0)
					{
						timeCreated = 1;
					}
					
					setObjVar(newItem, "time_created", timeCreated);
				}
			}
			
			if (objectType == GOT_chronicles_quest_holocron || objectType == GOT_chronicles_quest_holocron_recipe)
			{
				if (itemDictionary.containsKey("PGCPacked"))
				{
					byte[] data = itemDictionary.getByteArray("PGCPacked");
					setAutoVariableFromByteStream(newItem, "PGCPacked", data);
				}
				
				if (objectType == GOT_chronicles_quest_holocron_recipe)
				{
					
					setPlayerQuestCreator(newItem, self);
					setObjVar(newItem, pgc_quests.PCG_QUEST_CREATOR_ID_OBJVAR, self);
					setObjVar(newItem, pgc_quests.PCG_QUEST_CREATOR_STATIONID_OBJVAR, getPlayerStationId(self));
					setObjVar(newItem, pgc_quests.PCG_QUEST_CREATOR_NAME_OBJVAR, getName(self));
				}
			}
			
			if ((objectTemplateCrc == (2082993503)) && (hasObjVar(newItem, incubator.RESOURCE_POWER_NAME)))
			{
				if (itemDictionary.containsKey("incubatorResourceData"))
				{
					byte[] data = itemDictionary.getByteArray("incubatorResourceData");
					setAutoVariableFromByteStream(newItem, "incubatorResourceData", data);
				}
			}
			
			if (objectType == GOT_installation_factory)
			{
				
				setObjVar(newItem, player_structure.VAR_OWNER, getPlayerName(self));
				
				String[] admin_list = new String[1];
				admin_list[0] = self.toString();
				setObjVar(newItem, player_structure.VAR_ADMIN_LIST, admin_list);
				
				setObjVar(newItem, player_structure.VAR_HOPPER_LIST, admin_list);
				
				removeObjVar(newItem, player_structure.VAR_ENTER_LIST);
				removeObjVar(newItem, player_structure.VAR_BAN_LIST);
				
				removeObjVar(newItem, player_structure.VAR_VENDOR_LIST);
				
				removeObjVar(newItem, "city_id");
				
				removeObjVar(newItem, player_structure.VAR_IS_GUILD_HALL);
				
				if (itemDictionary.containsKey("powerValue"))
				{
					byte[] powerValue = itemDictionary.getByteArray("powerValue");
					setAutoVariableFromByteStream(newItem, "powerValue", powerValue);
				}
				
				if (itemDictionary.containsKey("powerRate"))
				{
					byte[] powerRate = itemDictionary.getByteArray("powerRate");
					setAutoVariableFromByteStream(newItem, "powerRate", powerRate);
				}
				
				if (itemDictionary.containsKey("manf_schematic"))
				{
					dictionary dictManfSchematic = itemDictionary.getDictionary("manf_schematic");
					if (dictManfSchematic != null)
					{
						
						obj_id manfSchematic = unpackItem(utils.getPlayerDatapad(self), dictManfSchematic);
						if (isIdValid(manfSchematic))
						{
							transferManufactureSchematicToStation(manfSchematic, newItem);
						}
					}
				}
				
				if (itemDictionary.containsKey("ingr_hopper_contents"))
				{
					dictionary dictIngrHopperContents = itemDictionary.getDictionary("ingr_hopper_contents");
					if (dictIngrHopperContents != null)
					{
						obj_id ingrHopper = getObjectInSlot(newItem, "ingredient_hopper");
						if (isIdValid(ingrHopper))
						{
							Set keySet = dictIngrHopperContents.keySet();
							Iterator contentsIterator = keySet.iterator();
							while (contentsIterator.hasNext())
							{
								testAbortScript();
								obj_id key = (obj_id)contentsIterator.next();
								
								dictionary containedItemDict = dictIngrHopperContents.getDictionary(key);
								if (containedItemDict == null)
								{
									continue;
								}
								
								if (ctsCheckAndLogUniqueStaticItemAlreadyExists(ingrHopper, containedItemDict))
								{
									continue;
								}
								
								unpackItem(ingrHopper, containedItemDict);
							}
						}
					}
				}
				
				if (itemDictionary.containsKey("output_hopper_contents"))
				{
					dictionary dictOutputHopperContents = itemDictionary.getDictionary("output_hopper_contents");
					if (dictOutputHopperContents != null)
					{
						obj_id outputHopper = getObjectInSlot(newItem, "output_hopper");
						if (isIdValid(outputHopper))
						{
							Set keySet = dictOutputHopperContents.keySet();
							Iterator contentsIterator = keySet.iterator();
							while (contentsIterator.hasNext())
							{
								testAbortScript();
								obj_id key = (obj_id)contentsIterator.next();
								
								dictionary containedItemDict = dictOutputHopperContents.getDictionary(key);
								if (containedItemDict == null)
								{
									continue;
								}
								
								if (ctsCheckAndLogUniqueStaticItemAlreadyExists(outputHopper, containedItemDict))
								{
									continue;
								}
								
								unpackItem(outputHopper, containedItemDict);
							}
						}
					}
				}
			}
			
			if (objectType == GOT_misc_crafting_station)
			{
				
				if (itemDictionary.containsKey("input_hopper_contents"))
				{
					dictionary dictInputHopperContents = itemDictionary.getDictionary("input_hopper_contents");
					if (dictInputHopperContents != null)
					{
						obj_id inputHopper = getObjectInSlot(newItem, "ingredient_hopper");
						if (isIdValid(inputHopper))
						{
							Set keySet = dictInputHopperContents.keySet();
							Iterator contentsIterator = keySet.iterator();
							while (contentsIterator.hasNext())
							{
								testAbortScript();
								obj_id key = (obj_id)contentsIterator.next();
								
								dictionary containedItemDict = dictInputHopperContents.getDictionary(key);
								if (containedItemDict == null)
								{
									continue;
								}
								
								if (ctsCheckAndLogUniqueStaticItemAlreadyExists(inputHopper, containedItemDict))
								{
									continue;
								}
								
								unpackItem(inputHopper, containedItemDict);
							}
						}
					}
				}
			}
			
			if (itemDictionary.containsKey("isBlueprintDesigner"))
			{
				if (itemDictionary.getBoolean("isBlueprintDesigner"))
				{
					setObjVar(newItem, storyteller.BLUEPRINT_AUTHOR_OBJVAR, self);
				}
				else
				{
					CustomerServiceLog("CharacterTransfer", "Player is not blueprint's designer: player = "+ self + "; blueprint = "+ newItem);
				}
			}
			
			if (isGameObjectTypeOf(objectType, GOT_building))
			{
				
				if (itemDictionary.containsKey("lotOverlimitStructureId"))
				{
					obj_id lotOverlimitStructureOnCharacter = getObjIdObjVar(self, "lotOverlimit.structure_id");
					if (isIdValid(lotOverlimitStructureOnCharacter))
					{
						obj_id lotOverlimitStructure = itemDictionary.getObjId("lotOverlimitStructureId");
						if (isIdValid(lotOverlimitStructure) && (lotOverlimitStructure == lotOverlimitStructureOnCharacter))
						{
							setObjVar(self, "lotOverlimit.structure_id", newItem);
							setObjVar(self, "lotOverlimit.structure_location", "Datapad");
						}
					}
				}
			}
			
			if (itemDictionary.containsKey("house_contents"))
			{
				
				setObjVar(newItem, player_structure.VAR_OWNER, getPlayerName(self));
				
				String[] admin_list = new String[1];
				admin_list[0] = self.toString();
				setObjVar(newItem, player_structure.VAR_ADMIN_LIST, admin_list);
				
				removeObjVar(newItem, player_structure.VAR_ENTER_LIST);
				removeObjVar(newItem, player_structure.VAR_BAN_LIST);
				
				removeObjVar(newItem, player_structure.VAR_VENDOR_LIST);
				
				removeObjVar(newItem, player_structure.VAR_HOPPER_LIST);
				
				removeObjVar(newItem, player_structure.VAR_IS_GUILD_HALL);
				
				dictionary contentsDict = itemDictionary.getDictionary("house_contents");
				Set keySet = contentsDict.keySet();
				Iterator contentsIterator = keySet.iterator();
				while (contentsIterator.hasNext())
				{
					testAbortScript();
					obj_id key = (obj_id)contentsIterator.next();
					if (isIdNull(key))
					{
						CustomerServiceLog("CharacterTransfer", "unpackItem() FAILED: key "+ key + " not valid in house contents dictionary: "+ contentsDict);
						return null;
					}
					dictionary containedItemDict = contentsDict.getDictionary(key);
					if (containedItemDict == null)
					{
						continue;
					}
					
					if (ctsCheckAndLogUniqueStaticItemAlreadyExists(utils.getInventoryContainer(self), containedItemDict))
					{
						continue;
					}
					
					containedItemDict.put("bypassVolumeCheck", 1);
					obj_id house_item = unpackItem(utils.getInventoryContainer(self), containedItemDict);
					if (isIdValid(house_item))
					{
						location loc = containedItemDict.getLocation("house_cell_loc");
						
						String cell_name = containedItemDict.getString("house_cell");
						obj_id cellid = getCellId(newItem, cell_name);
						if (isIdValid(cellid))
						{
							loc.cell = cellid;
							setLocation(house_item, loc);
						}
						
						float[] quaternion = containedItemDict.getFloatArray("quaternion");
						if ((quaternion != null) && (quaternion.length == 4))
						{
							setQuaternion(house_item, quaternion[0], quaternion[1], quaternion[2], quaternion[3]);
						}
						
						requestSendPositionUpdate(house_item);
					}
				}
				
				if (isGameObjectTypeOf(objectType, GOT_building))
				{
					loadBuildingContents(self, newItem);
				}
			}
			
			if ((objectType == GOT_installation_factory) || isGameObjectTypeOf(objectType, GOT_building))
			{
				if (hasObjVar(newItem, player_structure.VAR_LAST_MAINTANENCE))
				{
					setObjVar(newItem, player_structure.VAR_LAST_MAINTANENCE, getGameTime());
				}
			}
			
			int containerType = itemDictionary.getInt("containerType");
			
			if (containerType > 0 || jedi.isLightsaber(newItem) || isGameObjectTypeOf(objectType, GOT_data_ship_control_device))
			{
				CustomerServiceLog("CharacterTransfer", "unpackItem() : unpacking contents of container "+ newItem + "(type "+ containerType + ")");
				
				if (itemDictionary.containsKey("contents"))
				{
					
					if ((objectTemplateName.equals("object/intangible/ship/navicomputer_1.iff") || objectTemplateName.equals("object/intangible/ship/navicomputer_2.iff") || objectTemplateName.equals("object/intangible/ship/navicomputer_3.iff") || objectTemplateName.equals("object/intangible/ship/navicomputer_4.iff") || objectTemplateName.equals("object/intangible/ship/navicomputer_5.iff") || objectTemplateName.equals("object/intangible/ship/navicomputer_6.iff")))
					{
						obj_id datapad = utils.getDatapad(newItem);
						if (isIdValid(datapad))
						{
							destroyObject(datapad);
						}
					}
					
					dictionary contentsDict = itemDictionary.getDictionary("contents");
					Set keySet = contentsDict.keySet();
					Iterator contentsIterator = keySet.iterator();
					while (contentsIterator.hasNext())
					{
						testAbortScript();
						obj_id key = (obj_id)contentsIterator.next();
						if (isIdNull(key))
						{
							CustomerServiceLog("CharacterTransfer", "unpackItem() FAILED: key "+ key + " not valid in container contents dictionary: "+ contentsDict);
							return null;
						}
						
						dictionary containedItemDict = contentsDict.getDictionary(key);
						
						obj_id unpackedItem = null;
						obj_id saberInv = null;
						
						if (jedi.isLightsaber(newItem))
						{
							if (ctsCheckAndLogUniqueStaticItemAlreadyExists(container, containedItemDict))
							{
								continue;
							}
							
							saberInv = getObjectInSlot(newItem, "saber_inv");
							unpackedItem = unpackItem(container, containedItemDict);
						}
						else
						{
							if (ctsCheckAndLogUniqueStaticItemAlreadyExists(newItem, containedItemDict))
							{
								continue;
							}
							
							unpackedItem = unpackItem(newItem, containedItemDict);
						}
						
						if (!isIdValid(unpackedItem))
						{
							if (isIdNull(unpackedItem))
							{
								CustomerServiceLog("CharacterTransfer", "unpackItem() : unpackItem() returned NULL id for item: "+ containedItemDict);
								return null;
							}
							else
							{
								CustomerServiceLog("CharacterTransfer", "unpackItem() : unpackItem() returned invalid id "+ unpackedItem + " for item: "+ containedItemDict);
								continue;
							}
						}
						
						if (isIdValid(saberInv))
						{
							if (jedi.hasColorCrystal(newItem) && jedi.isColorCrystal(unpackedItem))
							{
								removeObjVar(newItem, jedi.VAR_SABER_BASE + "."+ jedi.VAR_COLOR);
							}
							if (!putIn(unpackedItem, saberInv))
							{
								CustomerServiceLog("CharacterTransfer", "unpackItem() : FAILED to move lightsaber crystal "+ unpackedItem + " into saber "+ saberInv);
							}
						}
					}
				}
				
				if (objectType == GOT_misc_factory_crate)
				{
					boolean factoryCrateHasCreator = false;
					if (itemDictionary.containsKey("factoryCrateHasCreator"))
					{
						factoryCrateHasCreator = itemDictionary.getBoolean("factoryCrateHasCreator");
					}
					
					boolean factoryCrateCreatorIsSelf = false;
					if (itemDictionary.containsKey("factoryCrateCreatorIsSelf"))
					{
						factoryCrateCreatorIsSelf = itemDictionary.getBoolean("factoryCrateCreatorIsSelf");
					}
					
					obj_id factoryCrateCreator = (factoryCrateCreatorIsSelf ? self : obj_id.getObjId(10000000));
					
					if (factoryCrateHasCreator)
					{
						setCrafter(newItem, factoryCrateCreator);
					}
					
					obj_id[] factoryCrateContents = getContents(newItem);
					if ((factoryCrateContents != null) && (factoryCrateContents.length > 0))
					{
						for (int i = 0; i < factoryCrateContents.length; ++i)
						{
							testAbortScript();
							setCrafter(factoryCrateContents[i], factoryCrateCreator);
						}
					}
				}
			}
			
			if (isEquipped)
			{
				if (equip(newItem, self))
				{
					CustomerServiceLog("CharacterTransfer", "unpackItem() : equipped item ("+ newItem + ")");
				}
				else
				{
					CustomerServiceLog("CharacterTransfer", "unpackItem() : FAILED to equip item ("+ newItem + ")");
				}
			}
			if (isAppearanceEquipped)
			{
				obj_id appearanceInventory = getAppearanceInventory(self);
				if (!isIdValid(appearanceInventory))
				{
					CustomerServiceLog("CharacterTransfer", "unpackItem() : Failed to equip appearance item ("+ newItem +"). Character has no valid appearance inventory.");
					return null;
				}
				
				if (putIn(newItem, appearanceInventory))
				{
					CustomerServiceLog("CharacterTransfer", "unpackItem() : equipped appearance item ("+ newItem + ")");
				}
				else
				{
					CustomerServiceLog("CharacterTransfer", "unpackItem() : FAILED to equip appearance item ("+ newItem + ")");
				}
				
			}
			if (isGameObjectTypeOf(objectType, GOT_data_ship_control_device))
			{
				
				obj_id ship = space_transition.getShipFromShipControlDevice(newItem);
				setObjVar(newItem, "ship", ship);
				setObjVar(ship, "shipControlDevice", newItem);
				if (!isIdValid(ship))
				{
					return null;
				}
			}
			
		}
		catch(Throwable t)
		{
			CustomerServiceLog("CharacterTransfer", "unpackItem() FAILED : container="+ container + ", itemDictionary="+ itemDictionary + " EXCEPTION: "+ t);
			return null;
		}
		
		utils.removeLocalVar(newItem, "ctsBeingUnpacked");
		return newItem;
	}
	
	
	public obj_id unpackWaypoint(dictionary waypointDictionary) throws InterruptedException
	{
		obj_id self = getSelf();
		
		if (!isIdValid(self))
		{
			return null;
		}
		obj_id newWaypoint = null;
		location loc = waypointDictionary.getLocation("location");
		
		if (!isIdNull(loc.cell))
		{
			
			return null;
		}
		
		location newloc = new location(loc.x, loc.y, loc.z, loc.area);
		String name = waypointDictionary.getString("name");
		newWaypoint = createWaypointInDatapad(self, newloc);
		setWaypointName(newWaypoint, name);
		return newWaypoint;
	}
	
	
	public boolean itemIsAllowedToTransfer(obj_id item, obj_id player) throws InterruptedException
	{
		if (!isValidId(item) || !isValidId(player))
		{
			return false;
		}
		
		if (utils.hasScriptVar(player, cts.SCRIPTVAR_CTS_ITEM_ID))
		{
			final obj_id ctsItem = utils.getObjIdScriptVar(player, cts.SCRIPTVAR_CTS_ITEM_ID);
			if (isValidId(ctsItem) && (ctsItem == item))
			{
				return false;
			}
		}
		
		return true;
	}
	
	
	public dictionary packWaypoint(obj_id waypoint) throws InterruptedException
	{
		obj_id self = getSelf();
		dictionary waypointDictionary = null;
		try
		{
			waypointDictionary = new dictionary();
			if (!isValidId(waypoint))
			{
				return null;
			}
			
			location loc = getWaypointLocation(waypoint);
			waypointDictionary.put("location", loc);
			
			String name = getWaypointName(waypoint);
			waypointDictionary.put("name", name);
			
		}
		catch(Throwable t)
		{
			return null;
		}
		return waypointDictionary;
	}
	
	
	public void doItemPrepack(obj_id item) throws InterruptedException
	{
		if (!isValidId(item))
		{
			return;
		}
		
		{
			dictionary d = new dictionary();
			int result = script_entry.callMessageHandlers("OnPack", item, d);
			if (result != SCRIPT_CONTINUE)
			{
				utils.setScriptVar(item, "do_not_pack", 1);
				return;
			}
		}
		obj_id[] contents = null;
		
		contents = getContents(item);
		if (contents != null)
		{
			for (int i = 0; i < contents.length; ++i)
			{
				testAbortScript();
				doItemPrepack( contents[i] );
			}
		}
	}
	
	
	public dictionary packItem(obj_id item, int objectType, boolean allowOverride, int resourceCrateCountLimit, int[] resourceCurrentCrateCount, long resourceUnitCountLimit, long[] resourceCurrentUnitCount) throws InterruptedException
	{
		
		dictionary itemDictionary = null;
		
		try
		{
			final obj_id self = getSelf();
			
			if (getName(item).equalsIgnoreCase( "object/cell/cell.iff"))
			{
				return null;
			}
			
			if (hasCondition(item, CONDITION_VENDOR))
			{
				return null;
			}
			
			if (utils.hasScriptVar(item, "do_not_pack"))
			{
				return null;
			}
			
			String name = getName(item);
			String objectTypeName = getGameObjectTypeName(objectType);
			String objectTemplateName = getTemplateName(item);
			int objectTemplateCrc = getObjectTemplateCrc(objectTemplateName);
			
			if (objectType == GOT_misc_factory_crate)
			{
				int factoryCrateCount = getCount(item);
				if (factoryCrateCount <= 0)
				{
					CustomerServiceLog("CharacterTransfer", "packItem("+ item + ") skipping factory crate because it has a count of "+ factoryCrateCount);
					return null;
				}
				
				obj_id[] factoryCrateContents = getContents(item);
				if ((factoryCrateContents == null) || (factoryCrateContents.length <= 0))
				{
					CustomerServiceLog("CharacterTransfer", "packItem("+ item + ") skipping factory crate because it is empty");
					return null;
				}
				
				String crateItemTemplateName = getTemplateName(factoryCrateContents[0]);
				
				if (crateItemTemplateName == null || crateItemTemplateName.length() <= 0)
				{
					CustomerServiceLog("CharacterTransfer", "packItem("+ item + ") skipping factory crate because its contents returned a null string as a template name.");
					return null;
				}
				
				if ((crateItemTemplateName.equals("object/tangible/food/crafted/dessert_felbar.iff") || crateItemTemplateName.equals("object/tangible/food/crafted/dessert_blap_biscuit.iff") || crateItemTemplateName.equals("object/tangible/food/crafted/dessert_pastebread.iff") || crateItemTemplateName.equals("object/tangible/food/crafted/dessert_sweesonberry_rolls.iff")))
				{
					CustomerServiceLog("CharacterTransfer", "packItem("+ item + ") skipping factory crate because it contained old pet food("+crateItemTemplateName+"), that is supposed to be deleted.");
					return null;
				}
				
			}
			
			itemDictionary = new dictionary();
			itemDictionary.put("objectTemplateCrc", objectTemplateCrc);
			itemDictionary.put("objectTemplateName", objectTemplateName);
			
			String packedObjvars = getPackedObjvars(item);
			itemDictionary.put("packedObjvars", packedObjvars);
			
			String staticItemName = getStaticItemName(item);
			if (static_item.isStaticItem(item) && !staticItemName.equals("item_respec_token_01_01"))
			{
				itemDictionary.put("staticItemName", staticItemName);
				CustomerServiceLog("CharacterTransfer", "packItem("+ item + ", "+ allowOverride + ") staticItemName="+ staticItemName);
			}
			else
			{
				CustomerServiceLog("CharacterTransfer", "packItem("+ item + ", "+ allowOverride + ") objectTemplateName="+ objectTemplateName + ", name="+ name);
				
				String[] itemScripts = getScriptList(item);
				if (itemScripts != null && itemScripts.length > 0)
				{
					
					for (int i = 0; i < itemScripts.length; ++i)
					{
						testAbortScript();
						if (itemScripts[i].startsWith("script."))
						{
							itemScripts[i] = itemScripts[i].substring(7);
						}
						
					}
					itemDictionary.put("scripts", itemScripts);
				}
				
				String objectName = getAssignedName(item);
				
				if (objectName == null || objectName.equals(""))
				{
					objectName = utils.packStringId(getNameStringId(item));
				}
				if (objectName == null)
				{
					objectName = "";
				}
				
				itemDictionary.put("objectName", objectName);
				
				dictionary colorData = hue.getPalcolorData(item);
				if (colorData != null)
				{
					itemDictionary.put("colorData", colorData);
				}
				
				if (isTangible(item))
				{
					byte[] customAppearance = getByteStreamFromAutoVariable(item, "customAppearance");
					if (customAppearance != null && customAppearance.length > 0)
					{
						itemDictionary.put("customAppearance", customAppearance);
					}
					
					byte[] appearanceData = getByteStreamFromAutoVariable(item, "appearanceData");
					if (appearanceData != null && appearanceData.length > 0)
					{
						itemDictionary.put("appearanceData", appearanceData);
					}
					
					if (isMob(item))
					{
						CustomerServiceLog("CharacterTransfer", "packItem() : getAttribs("+ item + ") on a creature object");
						
						attribute[] itemAttribs = getAttribs(item);
						if (itemAttribs != null)
						{
							
							int[] itemAttribTypes = new int[itemAttribs.length];
							int[] itemAttribValues = new int[itemAttribs.length];
							
							int itemAttribsIter;
							CustomerServiceLog("CharacterTransfer", "packItem() : retrieving "+ itemAttribs.length + " attributes");
							for (itemAttribsIter = 0; itemAttribsIter < itemAttribs.length; ++itemAttribsIter)
							{
								testAbortScript();
								itemAttribTypes[itemAttribsIter] = itemAttribs[itemAttribsIter].getType();
								itemAttribValues[itemAttribsIter] = getUnmodifiedMaxAttrib(item, itemAttribsIter);
								CustomerServiceLog("CharacterTransfer", "packItem() : itemAttribTypes["+ itemAttribsIter + "] = "+ itemAttribTypes[itemAttribsIter] + ", itemAttribsValues["+ itemAttribsIter + "] = "+ itemAttribValues[itemAttribsIter]);
							}
							
							itemDictionary.put("attribute_types", itemAttribTypes);
							itemDictionary.put("attribute_values", itemAttribValues);
						}
						else
						{
							CustomerServiceLog("CharacterTransfer", "packItem() FAILED: getAttribs("+ item + ") = null.");
							return null;
						}
					}
					
					else
					{
						
						int maxHitpoints = getMaxHitpoints(item);
						if (maxHitpoints != ATTRIB_ERROR)
						{
							itemDictionary.put("maxHitpoints", maxHitpoints);
						}
						else
						{
							CustomerServiceLog("CharacterTransfer", "packItem() FAILED: getMaxHitpoints("+ item + ") failed!");
							return null;
						}
						
						int hitpoints = getHitpoints(item);
						if (hitpoints != ATTRIB_ERROR)
						{
							itemDictionary.put("hitpoints", hitpoints);
						}
						else
						{
							CustomerServiceLog("CharacterTransfer", "packItem() FAILED: getHitpoints("+ item + ") failed!");
							return null;
						}
						
						itemDictionary.put("conditionFlags", getCondition(item));
						
						boolean isWeaponItem = isWeapon(item);
						itemDictionary.put("isWeaponItem", isWeaponItem);
						if (isWeaponItem)
						{
							
							int currentVersion = getConversionId(item);
							if (currentVersion > 0)
							{
								
								itemDictionary.put("currentVersion", currentVersion);
							}
							int minDamage = getWeaponMinDamage(item);
							itemDictionary.put("minDamage", minDamage);
							
							int maxDamage = getWeaponMaxDamage(item);
							itemDictionary.put("maxDamage", maxDamage);
							
							float attackSpeed = getWeaponAttackSpeed(item);
							itemDictionary.put("attackSpeed", attackSpeed);
							
							range_info rangeInfo = getWeaponRangeInfo(item);
							itemDictionary.put("rangeInfoMinRange", rangeInfo.minRange);
							itemDictionary.put("rangeInfoMaxRange", rangeInfo.maxRange);
							
							float woundChance = getWeaponWoundChance(item);
							itemDictionary.put("woundChance", woundChance);
							
							float damageRadius = getWeaponDamageRadius(item);
							itemDictionary.put("damageRadius", damageRadius);
							
							int accuracy = getWeaponAccuracy(item);
							itemDictionary.put("accuracy", accuracy);
							
							int elementalType = getWeaponElementalType(item);
							itemDictionary.put("elementalType", elementalType);
							
							int elementalValue = getWeaponElementalValue(item);
							itemDictionary.put("elementalValue", elementalValue);
							
							int damageType = getWeaponDamageType(item);
							itemDictionary.put("damageType", damageType);
							
							int attackCost = getWeaponAttackCost(item);
							itemDictionary.put("attackCost", attackCost);
						}
					}
				}
			}
			
			int count = getCount(item);
			if (count > 0)
			{
				itemDictionary.put("count", count);
			}
			
			if (getBankBalance(item) > 0)
			{
				byte[] bankBalance = getByteStreamFromAutoVariable(item, "bankBalance");
				if (bankBalance != null && bankBalance.length > 0)
				{
					itemDictionary.put("bankBalance", bankBalance);
				}
			}
			
			if (hasScript(item, "systems.jedi.jedi_saber_component"))
			{
				if (jedi.isCrystalTuned(item))
				{
					if (jedi.isCrystalOwner(self,item))
					{
						itemDictionary.put("isCrystalOwner", true);
					}
					else
					{
						itemDictionary.put("isCrystalOwner", false);
					}
				}
			}
			
			obj_id biolink = getBioLink(item);
			if (isIdValid(biolink))
			{
				itemDictionary.put("biolink", biolink);
				if (biolink == self)
				{
					itemDictionary.put("isBiolinkOwner", true);
				}
				else
				{
					itemDictionary.put("isBiolinkOwner", false);
				}
			}
			
			if (hasObjVar(item, storyteller.BLUEPRINT_AUTHOR_OBJVAR))
			{
				obj_id blueprintDesigner = getObjIdObjVar(item, storyteller.BLUEPRINT_AUTHOR_OBJVAR);
				if (isIdValid(blueprintDesigner))
				{
					if (blueprintDesigner == self)
					{
						itemDictionary.put("isBlueprintDesigner", true);
					}
					else
					{
						itemDictionary.put("isBlueprintDesigner", false);
					}
				}
			}
			
			if (getContainedBy(item) == self)
			{
				itemDictionary.put("isEquipped", true);
			}
			else
			{
				itemDictionary.put("isEquipped", false);
			}
			
			obj_id appearanceInventory = getAppearanceInventory(self);
			if (isIdValid(appearanceInventory) && getContainedBy(item) == appearanceInventory)
			{
				itemDictionary.put("isAppearanceEquipped", true);
			}
			else
			{
				itemDictionary.put("isAppearanceEquipped", false);
			}
			
			if (isGameObjectTypeOf(objectType, GOT_resource_container))
			{
				
				if (objectType != GOT_resource_container_pseudo)
				{
					int quantity = getResourceContainerQuantity(item);
					if (quantity <= 0)
					{
						return null;
					}
					
					String resourceData = getResourceCtsData(item);
					if ((resourceData == null) || (resourceData.length() <= 0))
					{
						return null;
					}
					
					if ((resourceUnitCountLimit > 0L) && ((resourceCurrentUnitCount[0] + quantity) > resourceUnitCountLimit))
					{
						quantity = (int)(resourceUnitCountLimit - resourceCurrentUnitCount[0]);
						
						if (quantity <= 0)
						{
							return null;
						}
					}
					
					resourceCurrentCrateCount[0] = resourceCurrentCrateCount[0] + 1;
					resourceCurrentUnitCount[0] = resourceCurrentUnitCount[0] + quantity;
					
					CustomerServiceLog("CharacterTransfer", "packItem("+ item + ") packed resource container ("+ resourceCurrentCrateCount[0] + "/"+ resourceCrateCountLimit + ") ("+ resourceCurrentUnitCount[0] + "/"+ resourceUnitCountLimit + "): resourceQuantity="+ quantity + ", resourceData="+ resourceData);
					
					itemDictionary.put("resourceQuantity", quantity);
					itemDictionary.put("resourceData", resourceData);
				}
				else
				{
					
					resourceCurrentCrateCount[0] = resourceCurrentCrateCount[0] + 1;
					
					CustomerServiceLog("CharacterTransfer", "packItem("+ item + ") packed pseudo resource container ("+ resourceCurrentCrateCount[0] + "/"+ resourceCrateCountLimit + ") ("+ resourceCurrentUnitCount[0] + "/"+ resourceUnitCountLimit + ")");
				}
			}
			
			if (objectType == GOT_misc_factory_crate)
			{
				obj_id factoryCrateCreator = getCrafter(item);
				if (isIdValid(factoryCrateCreator))
				{
					itemDictionary.put("factoryCrateHasCreator", true);
					
					if (factoryCrateCreator == self)
					{
						itemDictionary.put("factoryCrateCreatorIsSelf", true);
					}
					else
					{
						itemDictionary.put("factoryCrateCreatorIsSelf", false);
					}
				}
				else
				{
					itemDictionary.put("factoryCrateHasCreator", false);
				}
			}
			
			if (objectType == GOT_data_manufacturing_schematic)
			{
				byte[] data = getByteStreamFromAutoVariable(item, "msoCtsPackUnpack");
				if (data != null && data.length > 0)
				{
					itemDictionary.put("msoCtsPackUnpack", data);
				}
				
				itemDictionary.put("msoDraftSchematicCrc", getDraftSchematicCrc(item));
				
				int conversionId = getConversionId(item);
				if (conversionId > 0)
				{
					itemDictionary.put("msoConversionId", conversionId);
				}
			}
			
			if (objectType == GOT_data_mission_object)
			{
				byte[] data = getByteStreamFromAutoVariable(item, "moCtsPackUnpack");
				if (data != null && data.length > 0)
				{
					itemDictionary.put("moCtsPackUnpack", data);
				}
				
				if (hasObjVar(item, "time_created"))
				{
					int secondsSinceCreated = (getGameTime() - getIntObjVar(item, "time_created"));
					if (secondsSinceCreated < 0)
					{
						secondsSinceCreated = 0;
					}
					
					itemDictionary.put("missionAge", secondsSinceCreated);
				}
			}
			
			if (objectType == GOT_chronicles_quest_holocron || objectType == GOT_chronicles_quest_holocron_recipe)
			{
				byte[] data = getByteStreamFromAutoVariable(item, "PGCPacked");
				if (data != null && data.length > 0)
				{
					itemDictionary.put("PGCPacked", data);
				}
				
			}
			
			if ((objectTemplateCrc == (2082993503)) && (hasObjVar(item, incubator.RESOURCE_POWER_NAME)))
			{
				byte[] data = getByteStreamFromAutoVariable(item, "incubatorResourceData");
				if (data != null && data.length > 0)
				{
					itemDictionary.put("incubatorResourceData", data);
				}
			}
			
			if (isGameObjectTypeOf(objectType, GOT_ship))
			{
				float mass = getChassisComponentMassMaximum(item);
				float hp = getShipMaximumChassisHitPoints(item);
				float currentHp = getShipCurrentChassisHitPoints(item);
				
				itemDictionary.put("ship_mass", mass);
				itemDictionary.put("ship_hp", hp);
				itemDictionary.put("ship_current_hp", currentHp);
				
				obj_id objDroidControlDevice = getDroidControlDeviceForShip(item);
				if (isIdValid(objDroidControlDevice))
				{
					itemDictionary.put("droid_control_device", objDroidControlDevice);
				}
				
				if ((objectTemplateName != null) && (objectTemplateName.endsWith("player_sorosuub_space_yacht.iff")))
				{
					
				}
				else
				{
					dictionary shipDict = new dictionary();
					
					int[] shipSlots = getShipChassisSlots(item);
					
					for (int i = 0; i < shipSlots.length; ++i)
					{
						testAbortScript();
						if (isShipSlotInstalled(item, shipSlots[i]))
						{
							
							obj_id component = shipUninstallComponentAllowOverload(getOwner(item), item, shipSlots[i], utils.getInventoryContainer(self));
							
							if (isIdValid(component))
							{
								int ot = getGameObjectType(component);
								if (itemIsAllowedToTransfer(component, self) && (!isGameObjectTypeOf(ot, GOT_resource_container) || !ctsHasExceededResourceTransferLimit(resourceCrateCountLimit, resourceCurrentCrateCount[0], resourceUnitCountLimit, resourceCurrentUnitCount[0])))
								{
									dictionary component_dict = packItem(component, ot, true, resourceCrateCountLimit, resourceCurrentCrateCount, resourceUnitCountLimit, resourceCurrentUnitCount);
									if (component_dict != null)
									{
										shipDict.put(new Integer(shipSlots[i]), component_dict);
									}
								}
							}
							
							shipInstallComponent(getOwner(item), item, shipSlots[i], component);
						}
					}
					itemDictionary.put("shipComponents", shipDict);
				}
			}
			
			if (objectType == GOT_installation_factory)
			{
				
				byte[] powerValue = getByteStreamFromAutoVariable(item, "powerValue");
				if (powerValue != null && powerValue.length > 0)
				{
					itemDictionary.put("powerValue", powerValue);
				}
				
				byte[] powerRate = getByteStreamFromAutoVariable(item, "powerRate");
				if (powerRate != null && powerRate.length > 0)
				{
					itemDictionary.put("powerRate", powerRate);
				}
				
				obj_id manfSchematic = getObjectInSlot(item, "manf_schematic");
				if (isIdValid(manfSchematic))
				{
					int ot = getGameObjectType(manfSchematic);
					if (itemIsAllowedToTransfer(manfSchematic, self) && (!isGameObjectTypeOf(ot, GOT_resource_container) || !ctsHasExceededResourceTransferLimit(resourceCrateCountLimit, resourceCurrentCrateCount[0], resourceUnitCountLimit, resourceCurrentUnitCount[0])))
					{
						dictionary dictManfSchematic = packItem(manfSchematic, ot, true, resourceCrateCountLimit, resourceCurrentCrateCount, resourceUnitCountLimit, resourceCurrentUnitCount);
						if (dictManfSchematic != null)
						{
							itemDictionary.put("manf_schematic", dictManfSchematic);
						}
					}
				}
				
				obj_id ingrHopper = getObjectInSlot(item, "ingredient_hopper");
				if (isIdValid(ingrHopper))
				{
					obj_id ingrHopperContents[] = getContents(ingrHopper);
					if (ingrHopperContents != null && ingrHopperContents.length > 0)
					{
						dictionary dictIngrHopperContents = new dictionary();
						
						for (int i = 0; i < ingrHopperContents.length; ++i)
						{
							testAbortScript();
							obj_id ingrHopperContent = ingrHopperContents[i];
							
							if (!isObjectPersisted(ingrHopperContent))
							{
								continue;
							}
							
							int ot = getGameObjectType(ingrHopperContent);
							if (itemIsAllowedToTransfer(ingrHopperContent, self) && (!isGameObjectTypeOf(ot, GOT_resource_container) || !ctsHasExceededResourceTransferLimit(resourceCrateCountLimit, resourceCurrentCrateCount[0], resourceUnitCountLimit, resourceCurrentUnitCount[0])))
							{
								dictionary dictIngrHopperContent = packItem(ingrHopperContent, ot, true, resourceCrateCountLimit, resourceCurrentCrateCount, resourceUnitCountLimit, resourceCurrentUnitCount);
								if (dictIngrHopperContent != null)
								{
									dictIngrHopperContents.put(ingrHopperContent, dictIngrHopperContent);
								}
							}
						}
						
						itemDictionary.put("ingr_hopper_contents", dictIngrHopperContents);
					}
				}
				
				obj_id outputHopper = getObjectInSlot(item, "output_hopper");
				if (isIdValid(outputHopper))
				{
					obj_id outputHopperContents[] = getContents(outputHopper);
					if (outputHopperContents != null && outputHopperContents.length > 0)
					{
						dictionary dictOutputHopperContents = new dictionary();
						
						for (int i = 0; i < outputHopperContents.length; ++i)
						{
							testAbortScript();
							obj_id outputHopperContent = outputHopperContents[i];
							
							if (!isObjectPersisted(outputHopperContent))
							{
								continue;
							}
							
							int ot = getGameObjectType(outputHopperContent);
							if (itemIsAllowedToTransfer(outputHopperContent, self) && (!isGameObjectTypeOf(ot, GOT_resource_container) || !ctsHasExceededResourceTransferLimit(resourceCrateCountLimit, resourceCurrentCrateCount[0], resourceUnitCountLimit, resourceCurrentUnitCount[0])))
							{
								dictionary dictOutputHopperContent = packItem(outputHopperContent, ot, true, resourceCrateCountLimit, resourceCurrentCrateCount, resourceUnitCountLimit, resourceCurrentUnitCount);
								if (dictOutputHopperContent != null)
								{
									dictOutputHopperContents.put(outputHopperContent, dictOutputHopperContent);
								}
							}
						}
						
						itemDictionary.put("output_hopper_contents", dictOutputHopperContents);
					}
				}
			}
			
			if (objectType == GOT_misc_crafting_station)
			{
				
				obj_id inputHopper = getObjectInSlot(item, "ingredient_hopper");
				if (isIdValid(inputHopper))
				{
					obj_id inputHopperContents[] = getContents(inputHopper);
					if (inputHopperContents != null && inputHopperContents.length > 0)
					{
						dictionary dictInputHopperContents = new dictionary();
						
						for (int i = 0; i < inputHopperContents.length; ++i)
						{
							testAbortScript();
							obj_id inputHopperContent = inputHopperContents[i];
							
							if (!isObjectPersisted(inputHopperContent))
							{
								continue;
							}
							
							int ot = getGameObjectType(inputHopperContent);
							if (itemIsAllowedToTransfer(inputHopperContent, self) && (!isGameObjectTypeOf(ot, GOT_resource_container) || !ctsHasExceededResourceTransferLimit(resourceCrateCountLimit, resourceCurrentCrateCount[0], resourceUnitCountLimit, resourceCurrentUnitCount[0])))
							{
								dictionary dictInputHopperContent = packItem(inputHopperContent, ot, true, resourceCrateCountLimit, resourceCurrentCrateCount, resourceUnitCountLimit, resourceCurrentUnitCount);
								if (dictInputHopperContent != null)
								{
									dictInputHopperContents.put(inputHopperContent, dictInputHopperContent);
								}
							}
						}
						
						itemDictionary.put("input_hopper_contents", dictInputHopperContents);
					}
				}
			}
			
			{
				if (isGameObjectTypeOf(objectType, GOT_building) || space_utils.isPobType(objectTemplateName))
				{
					if (isGameObjectTypeOf(objectType, GOT_building))
					{
						
						obj_id lotOverlimitStructure = getObjIdObjVar(self, "lotOverlimit.structure_id");
						if (isIdValid(lotOverlimitStructure) && (lotOverlimitStructure == item))
						{
							itemDictionary.put("lotOverlimitStructureId", item);
						}
					}
					
					String[] cells = getCellNames(item);
					if (cells != null)
					{
						dictionary house_contents = new dictionary();
						for (int i = 0; i < cells.length; i++)
						{
							testAbortScript();
							obj_id cellid = getCellId(item, cells[i]);
							obj_id contents[] = getContents(cellid);
							if (contents != null && contents.length > 0)
							{
								for (int j=0; j<contents.length; j++)
								{
									testAbortScript();
									obj_id house_item = contents[j];
									
									if (!isObjectPersisted(house_item))
									{
										continue;
									}
									
									int ot = getGameObjectType(house_item);
									if (itemIsAllowedToTransfer(house_item, self) && (!isGameObjectTypeOf(ot, GOT_resource_container) || !ctsHasExceededResourceTransferLimit(resourceCrateCountLimit, resourceCurrentCrateCount[0], resourceUnitCountLimit, resourceCurrentUnitCount[0])))
									{
										dictionary house_item_dict = packItem(house_item, ot, true, resourceCrateCountLimit, resourceCurrentCrateCount, resourceUnitCountLimit, resourceCurrentUnitCount);
										if (house_item_dict != null)
										{
											
											house_item_dict.put("house_cell", cells[i]);
											
											house_item_dict.put("house_cell_loc", getLocation(house_item));
											
											float[] quaternion = getQuaternion(house_item);
											if ((quaternion != null) && (quaternion.length == 4))
											{
												house_item_dict.put("quaternion", quaternion);
											}
											
											house_contents.put(house_item, house_item_dict);
										}
									}
								}
							}
						}
						itemDictionary.put("house_contents", house_contents);
					}
				}
			}
			
			int containerType = getContainerType(item);
			itemDictionary.put("containerType", containerType);
			
			obj_id[] contents = null;
			obj_id saberInv = null;
			boolean b_is_ship = false;
			
			if (containerType == 2 && !isGameObjectTypeOf(objectType, GOT_building))
			{
				contents = getContents(item);
			}
			else if (jedi.isLightsaber(item))
			{
				saberInv = getObjectInSlot(item, "saber_inv");
				if (isIdValid(saberInv))
				{
					contents = getContents(saberInv);
				}
			}
			else if (isGameObjectTypeOf( objectType, GOT_data_ship_control_device ))
			{
				contents = getContents(item);
			}
			else if (isGameObjectTypeOf(objectType, GOT_ship))
			{
				contents = getContents(item);
			}
			else if (isGameObjectTypeOf( objectType, GOT_data_droid_control_device ))
			{
				contents = getContents(item);
			}
			else if (isGameObjectTypeOf(objectType, GOT_data_house_control_device))
			{
				contents = getContents(item);
			}
			
			if (contents != null && contents.length > 0)
			{
				dictionary container = new dictionary();
				
				int i = 0;
				for (i = 0; i < contents.length; ++i)
				{
					testAbortScript();
					if (guild.STR_GUILD_REMOTE_DEVICE.equals(getTemplateName(contents[i])))
					{
						continue;
					}
					
					int ot = getGameObjectType(contents[i]);
					if (itemIsAllowedToTransfer(contents[i], self) && (!isGameObjectTypeOf(ot, GOT_resource_container) || !ctsHasExceededResourceTransferLimit(resourceCrateCountLimit, resourceCurrentCrateCount[0], resourceUnitCountLimit, resourceCurrentUnitCount[0])))
					{
						dictionary containedItem = packItem(contents[i], ot, allowOverride, resourceCrateCountLimit, resourceCurrentCrateCount, resourceUnitCountLimit, resourceCurrentUnitCount);
						if (containedItem != null)
						{
							container.put(contents[i], containedItem);
							
							if (objectType == GOT_misc_factory_crate)
							{
								break;
							}
						}
						else
						{
							if (! allowOverride)
							{
								return null;
							}
						}
					}
				}
				itemDictionary.put("contents", container);
			}
		}
		catch(Throwable t)
		{
			CustomerServiceLog("CharacterTransfer", "packItem() FAILED: item="+ item + ", allowOverride="+ allowOverride + ", itemDictionary= "+ itemDictionary + " EXCEPTION: "+ t);
			return null;
		}
		
		return itemDictionary;
	}
	
	
	public int OnUploadCharacter(obj_id self, dictionary characterData) throws InterruptedException
	{
		int resourceCrateCountLimit = 0;
		String config = getConfigSetting("GameServer", "ctsResourceCrateCountLimit");
		if (config != null)
		{
			resourceCrateCountLimit = utils.stringToInt(config);
		}
		
		int[] resourceCurrentCrateCount = new int[1];
		resourceCurrentCrateCount[0] = 0;
		
		long resourceUnitCountLimit = 0L;
		config = getConfigSetting("GameServer", "ctsResourceUnitCountLimit");
		if (config != null)
		{
			resourceUnitCountLimit = utils.stringToLong(config);
		}
		
		long[] resourceCurrentUnitCount = new long[1];
		resourceCurrentUnitCount[0] = 0L;
		
		try
		{
			final boolean withItems = characterData.getBoolean("withItems");
			final boolean allowOverride = characterData.getBoolean("allowOverride");
			CustomerServiceLog("CharacterTransfer", "OnUploadCharacter(withItems="+ withItems + ", allowOverride="+ allowOverride + ") begin");
			
			if (hasObjVar(self, "disableLoginCtsInProgress"))
			{
				int timeOut = getIntObjVar(self, "disableLoginCtsInProgress");
				if (timeOut > getGameTime())
				{
					CustomerServiceLog("CharacterTransfer", "OnUploadCharacter() : FAILED because of CTS completed or in progress for character");
					return SCRIPT_OVERRIDE;
				}
				else
				{
					removeObjVar(self, "disableLoginCtsInProgress");
				}
			}
			
			if (hasObjVar(self, "disableLoginCtsCompleted"))
			{
				CustomerServiceLog("CharacterTransfer", "OnUploadCharacter() : FAILED because of CTS completed or in progress for character");
				return SCRIPT_OVERRIDE;
			}
			
			String skillTemplate = getSkillTemplate(self);
			if (skillTemplate == null || skillTemplate.equals("") || skillTemplate.equals("a"))
			{
				CustomerServiceLog("CharacterTransfer", "OnUploadCharacter() : skillTemplate not set! (pre-NGE source characters must choose profession prior to transfer) TRANSFER FAILED");
				return SCRIPT_OVERRIDE;
			}
			characterData.put("skillTemplate", skillTemplate);
			
			int combatLevel = getLevel(self);
			characterData.put("combatLevel", combatLevel);
			
			String workingSkill = getWorkingSkill(self);
			if (workingSkill == null || workingSkill.equals(""))
			{
				CustomerServiceLog("CharacterTransfer", "OnUploadCharacter() : workingSkill not set! (pre-NGE source characters must choose profession prior to transfer) TRANSFER FAILED");
				return SCRIPT_OVERRIDE;
			}
			characterData.put("workingSkill", workingSkill);
			
			CustomerServiceLog("CharacterTransfer", "OnUploadCharacter() : skillTemplate == "+ skillTemplate + ", workingSkill == "+ workingSkill + ", combatLevel == "+ combatLevel);
			
			characterData.put("commands", getCommandListingForPlayer(self));
			characterData.put("skills", getSkillListingForPlayer(self));
			
			final String[] strObjVarLists = 
			{
				"badge",
				factions.FACTION,
				groundquests.questBaseObjVar,
				"live_conversions",
				space_flags.IMPERIAL_CORELLIA,
				space_flags.IMPERIAL_NABOO,
				space_flags.IMPERIAL_TATOOINE,
				space_flags.PRIVATEER_CORELLIA,
				space_flags.PRIVATEER_NABOO,
				space_flags.PRIVATEER_TATOOINE,
				space_flags.REBEL_CORELLIA,
				space_flags.REBEL_NABOO,
				space_flags.REBEL_TATOOINE,
				space_quest.QUEST_STATUS,
				township.OBJVAR_NOVA_ORION_FACTION,
				pgc_quests.PGC_GRANTED_ROADMAP_REWARDS_OBJVAR,
				"spaceTrackFlagListName",
				"veteran_rewards",
				"_notskill.schematics",
				"_notskill.mods",
				"respec_voucher",
				"mand.acknowledge",
				respec.EXPERTISE_VERSION_OBJVAR,
				"publish_gift",
				"legacy",
				temp_schematic.VAR_TEMP_SCHEMATIC_BASE,
				"cts",
				"ctsHistory",
				utils.CTS_OBJVAR_HISTORY,
				"hoth.flawless_reward",
				"instance_player_protected_data",
				"bornOnGalaxyTitleSet",
				"beast_master",
				"playerRespec",
				"respecsBought",
				player_structure.HOUSE_PACKUP_ARRAY_OBJVAR,
				"space.dantooine_landing_permission",
				"lotOverlimit",
				"renameCharacterRequest.requestTime",
				"galactic_reserve",
				"preserve_existing_lightsaber_crystal_owner_objvar",
				"testing_only_force_cts_failure_testing_only", 
			};
			
			dictionary experiencePoints = getExperiencePoints(self);
			if (experiencePoints != null)
			{
				characterData.put("experience_points", experiencePoints);
			}
			
			characterData.put("pvp_type", pvpGetType(self));
			characterData.put("pvp_aligned_faction", pvpGetAlignedFaction(self));
			
			characterData.put("gcw_current_point", pvpGetCurrentGcwPoints(self));
			characterData.put("gcw_current_rating", pvpGetCurrentGcwRating(self));
			characterData.put("gcw_current_pvp_kill", pvpGetCurrentPvpKills(self));
			characterData.put("gcw_lifetime_point", pvpGetLifetimeGcwPoints(self));
			characterData.put("gcw_max_imperial_rating", pvpGetMaxGcwImperialRating(self));
			characterData.put("gcw_max_rebel_rating", pvpGetMaxGcwRebelRating(self));
			characterData.put("gcw_lifetime_pvp_kill", pvpGetLifetimePvpKills(self));
			characterData.put("gcw_next_rating_calc_time", pvpGetNextGcwRatingCalcTime(self));
			
			characterData.put("source_character_oid", self);
			characterData.put("source_character_name", getName(self));
			characterData.put("source_cluster", getClusterName());
			
			if (hasObjVar(self, "freeCtsTransactionRuleName"))
			{
				characterData.put("freeCtsTransactionRuleName", getStringObjVar(self, "freeCtsTransactionRuleName"));
			}
			
			obj_id playerObject = getPlayerObject(self);
			if (isIdValid(playerObject))
			{
				byte[] quests = getByteStreamFromAutoVariable(playerObject, "quests");
				if (quests != null && quests.length > 0)
				{
					characterData.put("quests", quests);
				}
				
				byte[] activeQuests = getByteStreamFromAutoVariable(playerObject, "activeQuests");
				if (activeQuests != null && activeQuests.length > 0)
				{
					characterData.put("activeQuests", activeQuests);
				}
				
				byte[] completedQuests = getByteStreamFromAutoVariable(playerObject, "completedQuests");
				if (completedQuests != null && completedQuests.length > 0)
				{
					characterData.put("completedQuests", completedQuests);
				}
				
				byte[] currentQuest = getByteStreamFromAutoVariable(playerObject, "currentQuest");
				if (currentQuest != null && currentQuest.length > 0)
				{
					characterData.put("currentQuest", currentQuest);
				}
				
				byte[] bornDate = getByteStreamFromAutoVariable(playerObject, "bornDate");
				if (bornDate != null && bornDate.length > 0)
				{
					characterData.put("bornDate", bornDate);
				}
				
				byte[] playedTime = getByteStreamFromAutoVariable(playerObject, "playedTime");
				if (playedTime != null && playedTime.length > 0)
				{
					characterData.put("playedTime", playedTime);
				}
				
				byte[] food = getByteStreamFromAutoVariable(playerObject, "food");
				if (food != null && food.length > 0)
				{
					characterData.put("food", food);
				}
				
				byte[] maxFood = getByteStreamFromAutoVariable(playerObject, "maxFood");
				if (maxFood != null && maxFood.length > 0)
				{
					characterData.put("maxFood", maxFood);
				}
				
				byte[] drink = getByteStreamFromAutoVariable(playerObject, "drink");
				if (drink != null && drink.length > 0)
				{
					characterData.put("drink", drink);
				}
				
				byte[] maxDrink = getByteStreamFromAutoVariable(playerObject, "maxDrink");
				if (maxDrink != null && maxDrink.length > 0)
				{
					characterData.put("maxDrink", maxDrink);
				}
				
				byte[] meds = getByteStreamFromAutoVariable(playerObject, "meds");
				if (meds != null && meds.length > 0)
				{
					characterData.put("meds", meds);
				}
				
				byte[] maxMeds = getByteStreamFromAutoVariable(playerObject, "maxMeds");
				if (maxMeds != null && maxMeds.length > 0)
				{
					characterData.put("maxMeds", maxMeds);
				}
				
				byte[] matchMakingPersonalProfileId = getByteStreamFromAutoVariable(playerObject, "matchMakingPersonalProfileId");
				if (matchMakingPersonalProfileId != null && matchMakingPersonalProfileId.length > 0)
				{
					characterData.put("matchMakingPersonalProfileId", matchMakingPersonalProfileId);
				}
				
				byte[] matchMakingCharacterProfileId = getByteStreamFromAutoVariable(playerObject, "matchMakingCharacterProfileId");
				if (matchMakingCharacterProfileId != null && matchMakingCharacterProfileId.length > 0)
				{
					characterData.put("matchMakingCharacterProfileId", matchMakingCharacterProfileId);
				}
				
				byte[] playerObjectWorkingSkill = getByteStreamFromAutoVariable(playerObject, "workingSkill");
				if (playerObjectWorkingSkill != null && playerObjectWorkingSkill.length > 0)
				{
					characterData.put("workingSkill", playerObjectWorkingSkill);
				}
				
				byte[] playerObjectSkillTemplate = getByteStreamFromAutoVariable(playerObject, "skillTemplate");
				if (playerObjectSkillTemplate != null && playerObjectSkillTemplate.length > 0)
				{
					characterData.put("skillTemplate", playerObjectSkillTemplate);
				}
				
				byte[] collections = getByteStreamFromAutoVariable(playerObject, "collections");
				if (collections != null && collections.length > 0)
				{
					characterData.put("collections", collections);
				}
				
				byte[] collections2 = getByteStreamFromAutoVariable(playerObject, "collections2");
				if (collections2 != null && collections2.length > 0)
				{
					characterData.put("collections2", collections2);
				}
			}
			
			int[] quests = questGetAllActiveQuestIds(self);
			dictionary questWaypoints = new dictionary();
			for (int i = 0; i < quests.length; ++i)
			{
				testAbortScript();
				int questCrc = quests[i];
				for (int taskId = 0; taskId < 16; ++taskId)
				{
					testAbortScript();
					obj_id waypoint = groundquests.getObjIdForWaypoint(questCrc, taskId, self);
					obj_id entryWaypoint = groundquests.getObjIdForEntranceWaypoint(questCrc, taskId, self);
					if (isIdValid(waypoint))
					{
						questWaypoints.put(waypoint, 1);
					}
					if (isIdValid(entryWaypoint))
					{
						questWaypoints.put(entryWaypoint, 1);
					}
				}
				
			}
			
			dictionary staticQuestWaypoints = new dictionary();
			String[] questIDs = dataTableGetStringColumnNoDefaults("datatables/npc/static_quest/all_quest_names.iff", "quest_ids");
			if ((questIDs != null) && (questIDs.length > 0))
			{
				for (int i = 0; i < questIDs.length; ++i)
				{
					testAbortScript();
					if (hasObjVar(self, questIDs[i] + ".waypoint"))
					{
						obj_id waypoint = getObjIdObjVar(self, questIDs[i] + ".waypoint");
						if (isIdValid(waypoint))
						{
							staticQuestWaypoints.put(waypoint, 1);
						}
					}
					
					if (hasObjVar(self, questIDs[i] + ".waypointhome"))
					{
						obj_id waypoint = getObjIdObjVar(self, questIDs[i] + ".waypointhome");
						if (isIdValid(waypoint))
						{
							staticQuestWaypoints.put(waypoint, 1);
						}
					}
				}
			}
			
			obj_id[] waypoints = getWaypointsInDatapad(self);
			if (waypoints != null && waypoints.length > 0)
			{
				dictionary waypointDictionaries = new dictionary();
				for (int i = 0; i < waypoints.length; ++i)
				{
					testAbortScript();
					if (questWaypoints.containsKey(waypoints[i]))
					{
						continue;
					}
					
					if (staticQuestWaypoints.containsKey(waypoints[i]))
					{
						continue;
					}
					
					waypointDictionaries.put(waypoints[i], packWaypoint(waypoints[i]));
				}
				characterData.put("waypoints", waypointDictionaries);
			}
			
			if (withItems == true)
			{
				
				int moneyFromCash = getCashBalance(self);
				CustomerServiceLog("CharacterTransfer", "OnUploadCharacter : packing "+ moneyFromCash + " credits from cash");
				characterData.put("moneyFromCash", moneyFromCash);
				
				int moneyFromBank = getBankBalance(self);
				CustomerServiceLog("CharacterTransfer", "OnUploadCharacter : packing "+ moneyFromBank + " credits from bank");
				characterData.put("moneyFromBank", moneyFromBank);
				
				CustomerServiceLog("CharacterTransfer", "OnUploadCharacter() : packing inventory items");
				dictionary inventoryDictionary = new dictionary();
				obj_id[] inventoryItems = getInventoryAndEquipment(self);
				obj_id playerInventory = getObjectInSlot(self, "inventory");
				if (inventoryItems != null && inventoryItems.length > 0)
				{
					int inventoryIter = 0;
					for (inventoryIter = 0; inventoryIter < inventoryItems.length; ++inventoryIter)
					{
						testAbortScript();
						doItemPrepack(inventoryItems[inventoryIter]);
					}
					for (inventoryIter = 0; inventoryIter < inventoryItems.length; ++inventoryIter)
					{
						testAbortScript();
						if (!isValidId(inventoryItems[inventoryIter]))
						{
							continue;
						}
						
						if (hasScript(inventoryItems[inventoryIter], "event.housepackup.cts_greeter"))
						{
							CustomerServiceLog("CharacterTransfer", "OnUploadCharacter() : SKIPPING pack of inventory item "+ inventoryItems[inventoryIter] + ". This item was a unique object for Free CTS and cannot be transfered to another server.");
							continue;
						}
						
						if (getContainedBy(inventoryItems[inventoryIter]) != self && getContainedBy(inventoryItems[inventoryIter]) != playerInventory)
						{
							CustomerServiceLog("CharacterTransfer", "OnUploadCharacter() : SKIPPING pack of inventory item "+ inventoryItems[inventoryIter] + " due to incorrect containment");
							continue;
						}
						if (hasCondition(inventoryItems[inventoryIter], CONDITION_VENDOR))
						{
							continue;
						}
						
						int ot = getGameObjectType(inventoryItems[inventoryIter]);
						if (itemIsAllowedToTransfer(inventoryItems[inventoryIter], self) && (!isGameObjectTypeOf(ot, GOT_resource_container) || !ctsHasExceededResourceTransferLimit(resourceCrateCountLimit, resourceCurrentCrateCount[0], resourceUnitCountLimit, resourceCurrentUnitCount[0])))
						{
							dictionary itemDictionary = packItem(inventoryItems[inventoryIter], ot, allowOverride, resourceCrateCountLimit, resourceCurrentCrateCount, resourceUnitCountLimit, resourceCurrentUnitCount);
							if (itemDictionary != null)
							{
								inventoryDictionary.put(inventoryItems[inventoryIter], itemDictionary);
							}
							else
							{
								if (! allowOverride)
								{
									return SCRIPT_OVERRIDE;
								}
							}
						}
					}
				}
				characterData.put("inventoryDictionary", inventoryDictionary);
				LIVE_LOG("CharacterTransfer", "OnUploadCharacter() : inventory items packed ("+ inventoryDictionary.toString() + ")");
				
				CustomerServiceLog("CharacterTransfer", "OnUploadCharacter() : packing bank items");
				dictionary bankDictionary = new dictionary();
				obj_id playerBank = utils.getPlayerBank(self);
				if (isIdValid(playerBank))
				{
					obj_id[] bankItems = getContents(playerBank);
					if (bankItems != null && bankItems.length > 0)
					{
						int bankIter = 0;
						for (bankIter = 0; bankIter < bankItems.length; ++bankIter)
						{
							testAbortScript();
							doItemPrepack(bankItems[bankIter]);
						}
						for (bankIter = 0; bankIter < bankItems.length; ++bankIter)
						{
							testAbortScript();
							if (!isValidId(bankItems[bankIter]))
							{
								continue;
							}
							if (getContainedBy(bankItems[bankIter]) != self && getContainedBy(bankItems[bankIter]) != playerBank)
							{
								CustomerServiceLog("CharacterTransfer", "OnUploadCharacter() : SKIPPING pack of bank item "+ bankItems[bankIter] + " due to incorrect containment");
								continue;
							}
							if (hasCondition(bankItems[bankIter], CONDITION_VENDOR))
							{
								continue;
							}
							
							int ot = getGameObjectType(bankItems[bankIter]);
							if (itemIsAllowedToTransfer(bankItems[bankIter], self) && (!isGameObjectTypeOf(ot, GOT_resource_container) || !ctsHasExceededResourceTransferLimit(resourceCrateCountLimit, resourceCurrentCrateCount[0], resourceUnitCountLimit, resourceCurrentUnitCount[0])))
							{
								dictionary itemDictionary = packItem(bankItems[bankIter], ot, allowOverride, resourceCrateCountLimit, resourceCurrentCrateCount, resourceUnitCountLimit, resourceCurrentUnitCount);
								if (itemDictionary != null)
								{
									bankDictionary.put(bankItems[bankIter], itemDictionary);
								}
								else
								{
									if (! allowOverride)
									{
										return SCRIPT_OVERRIDE;
									}
								}
							}
						}
					}
					characterData.put("bankDictionary", bankDictionary);
					LIVE_LOG("CharacterTransfer", "OnUploadCharacter() : bank items packed ("+ bankDictionary.toString() + ")");
				}
				else
				{
					CustomerServiceLog("CharacterTransfer", "OnUploadCharacter() : FAILED to pack bank items. bank object is not valid. TRANSFER FAILED");
					return SCRIPT_OVERRIDE;
				}
				
				CustomerServiceLog("CharacterTransfer", "OnUploadCharacter() : packing datapad items");
				dictionary datapadDictionary = new dictionary();
				obj_id playerDatapad = utils.getPlayerDatapad(self);
				if (isIdValid(playerDatapad))
				{
					obj_id[] datapadItems = getContents(playerDatapad);
					if (datapadItems != null && datapadItems.length > 0)
					{
						int datapadIter = 0;
						for (datapadIter = 0; datapadIter < datapadItems.length; ++datapadIter)
						{
							testAbortScript();
							doItemPrepack(datapadItems[datapadIter]);
						}
						for (datapadIter = 0; datapadIter < datapadItems.length; ++datapadIter)
						{
							testAbortScript();
							if (!isValidId(datapadItems[datapadIter]))
							{
								continue;
							}
							if (getContainedBy(datapadItems[datapadIter]) != self && getContainedBy(datapadItems[datapadIter]) != playerDatapad)
							{
								CustomerServiceLog("CharacterTransfer", "OnUploadCharacter() : SKIPPING pack of datapad item "+ datapadItems[datapadIter] + " due to incorrect containment");
								continue;
							}
							if (guild.STR_GUILD_REMOTE_DEVICE.equals(getTemplateName(datapadItems[datapadIter])))
							{
								continue;
							}
							if (hasCondition(datapadItems[datapadIter], CONDITION_VENDOR))
							{
								continue;
							}
							
							int objtype = getGameObjectType(datapadItems[datapadIter]);
							
							if (isGameObjectTypeOf(objtype, GOT_data_house_control_device))
							{
								
							}
							if (isGameObjectTypeOf(objtype, GOT_data_ship_control_device) && (TRANSFER_SHIPS == 0))
							{
								continue;
							}
							if (isGameObjectTypeOf(objtype, GOT_data_droid_control_device) && (TRANSFER_DROIDS == 0))
							{
								continue;
							}
							if (isGameObjectTypeOf(objtype, GOT_data_manufacturing_schematic))
							{
								
							}
							if (isGameObjectTypeOf(objtype, GOT_data_vendor_control_device))
							{
								continue;
							}
							if (objtype == GOT_data_mission_object)
							{
								
								String missionType = getMissionType(datapadItems[datapadIter]);
								if ((missionType != null) && missionType.equals("bounty"))
								{
									continue;
								}
							}
							
							if (itemIsAllowedToTransfer(datapadItems[datapadIter], self) && (!isGameObjectTypeOf(objtype, GOT_resource_container) || !ctsHasExceededResourceTransferLimit(resourceCrateCountLimit, resourceCurrentCrateCount[0], resourceUnitCountLimit, resourceCurrentUnitCount[0])))
							{
								dictionary itemDictionary = packItem(datapadItems[datapadIter], objtype, allowOverride, resourceCrateCountLimit, resourceCurrentCrateCount, resourceUnitCountLimit, resourceCurrentUnitCount);
								if (itemDictionary != null)
								{
									datapadDictionary.put(datapadItems[datapadIter], itemDictionary);
								}
								else
								{
									if (! allowOverride)
									{
										return SCRIPT_OVERRIDE;
									}
								}
							}
						}
					}
					
					characterData.put("datapadDictionary", datapadDictionary);
					LIVE_LOG("CharacterTransfer", "OnUploadCharacter() : datapad items packed ("+ datapadDictionary.toString() + ")");
					
				}
				else
				{
					CustomerServiceLog("CharacterTransfer", "OnUploadCharacter() : FAILED to pack datapad items. datapad object is not valid. TRANSFER FAILED");
					return SCRIPT_OVERRIDE;
				}
				
				CustomerServiceLog("CharacterTransfer", "OnUploadCharacter() : packing appearance buy back items");
				dictionary buyBackDictionary = new dictionary();
				obj_id buyBackContainer = getObjIdObjVar(self, smuggler.BUYBACK_CONTAINER_VAR);
				if (!isIdValid(buyBackContainer))
				{
					CustomerServiceLog("CharacterTransfer", "OnUploadCharacter() : FAILED to find a buy back container object.");
				}
				else
				{
					CustomerServiceLog("CharacterTransfer", "OnUploadCharacter() : SUCCESS finding a buy back container object.");
					obj_id[] buyBackItems = getContents(buyBackContainer);
					if (buyBackItems != null && buyBackItems.length > 0)
					{
						int buyBackIter = 0;
						for (buyBackIter = 0; buyBackIter < buyBackItems.length; ++buyBackIter)
						{
							testAbortScript();
							doItemPrepack(buyBackItems[buyBackIter]);
						}
						for (buyBackIter = 0; buyBackIter < buyBackItems.length; ++buyBackIter)
						{
							testAbortScript();
							if (!isValidId(buyBackItems[buyBackIter]))
							{
								continue;
							}
							if (getContainedBy(buyBackItems[buyBackIter]) != self && getContainedBy(buyBackItems[buyBackIter]) != buyBackContainer)
							{
								CustomerServiceLog("CharacterTransfer", "OnUploadCharacter() : SKIPPING pack of buy back item "+ buyBackItems[buyBackIter] + " due to incorrect containment");
								continue;
							}
							int objtype = getGameObjectType(buyBackItems[buyBackIter]);
							
							if (isGameObjectTypeOf(objtype, GOT_data_ship_control_device) && (TRANSFER_SHIPS == 0))
							{
								continue;
							}
							if (itemIsAllowedToTransfer(buyBackItems[buyBackIter], self) && (!isGameObjectTypeOf(objtype, GOT_resource_container) || !ctsHasExceededResourceTransferLimit(resourceCrateCountLimit, resourceCurrentCrateCount[0], resourceUnitCountLimit, resourceCurrentUnitCount[0])))
							{
								dictionary itemDictionary = packItem(buyBackItems[buyBackIter], objtype, allowOverride, resourceCrateCountLimit, resourceCurrentCrateCount, resourceUnitCountLimit, resourceCurrentUnitCount);
								if (itemDictionary != null)
								{
									buyBackDictionary.put(buyBackItems[buyBackIter], itemDictionary);
								}
								else
								{
									if (! allowOverride)
									{
										return SCRIPT_OVERRIDE;
									}
								}
							}
						}
					}
					
					characterData.put("buyBackDictionary", buyBackDictionary);
					LIVE_LOG("CharacterTransfer", "OnUploadCharacter() : buy back items packed ("+ buyBackDictionary.toString() + ")");
				}
				
				CustomerServiceLog("CharacterTransfer", "OnUploadCharacter() : packing hangar items");
				dictionary hangarDictionary = new dictionary();
				obj_id playerHangar = utils.getPlayerHangar(self);
				if (isIdValid(playerHangar))
				{
					obj_id[] hangarItems = getContents(playerHangar);
					if (hangarItems != null && hangarItems.length > 0)
					{
						int hangarIter = 0;
						for (hangarIter = 0; hangarIter < hangarItems.length; ++hangarIter)
						{
							testAbortScript();
							doItemPrepack(hangarItems[hangarIter]);
						}
						for (hangarIter = 0; hangarIter < hangarItems.length; ++hangarIter)
						{
							testAbortScript();
							if (!isValidId(hangarItems[hangarIter]))
							{
								continue;
							}
							if (getContainedBy(hangarItems[hangarIter]) != self && getContainedBy(hangarItems[hangarIter]) != playerHangar)
							{
								CustomerServiceLog("CharacterTransfer", "OnUploadCharacter() : SKIPPING pack of hangar item "+ hangarItems[hangarIter] + " due to incorrect containment");
								continue;
							}
							int objtype = getGameObjectType(hangarItems[hangarIter]);
							
							if (isGameObjectTypeOf(objtype, GOT_data_ship_control_device) && (TRANSFER_SHIPS == 0))
							{
								continue;
							}
							if (itemIsAllowedToTransfer(hangarItems[hangarIter], self) && (!isGameObjectTypeOf(objtype, GOT_resource_container) || !ctsHasExceededResourceTransferLimit(resourceCrateCountLimit, resourceCurrentCrateCount[0], resourceUnitCountLimit, resourceCurrentUnitCount[0])))
							{
								dictionary itemDictionary = packItem(hangarItems[hangarIter], objtype, allowOverride, resourceCrateCountLimit, resourceCurrentCrateCount, resourceUnitCountLimit, resourceCurrentUnitCount);
								if (itemDictionary != null)
								{
									hangarDictionary.put(hangarItems[hangarIter], itemDictionary);
								}
								else
								{
									if (! allowOverride)
									{
										return SCRIPT_OVERRIDE;
									}
								}
							}
						}
					}
					
					characterData.put("hangarDictionary", hangarDictionary);
					LIVE_LOG("CharacterTransfer", "OnUploadCharacter() : hangar items packed ("+ hangarDictionary.toString() + ")");
					
				}
				else
				{
					CustomerServiceLog("CharacterTransfer", "OnUploadCharacter() : FAILED to pack hangar items. hangar object is not valid. The player doesnt have a hangar object, transfer continuing.");
				}
				
				CustomerServiceLog("CharacterTransfer", "OnUploadCharacter() : packing appearance inventory items");
				dictionary appearanceDictionary = new dictionary();
				obj_id playerAppearance = getAppearanceInventory(self);
				if (isIdValid(playerAppearance))
				{
					obj_id[] appearanceItems = getContents(playerAppearance);
					if (appearanceItems != null && appearanceItems.length > 0)
					{
						int appearanceIter = 0;
						for (appearanceIter = 0; appearanceIter < appearanceItems.length; ++appearanceIter)
						{
							testAbortScript();
							doItemPrepack(appearanceItems[appearanceIter]);
						}
						for (appearanceIter = 0; appearanceIter < appearanceItems.length; ++appearanceIter)
						{
							testAbortScript();
							if (!isValidId(appearanceItems[appearanceIter]))
							{
								continue;
							}
							
							int objtype = getGameObjectType(appearanceItems[appearanceIter]);
							
							if (itemIsAllowedToTransfer(appearanceItems[appearanceIter], self) && (!isGameObjectTypeOf(objtype, GOT_resource_container) || !ctsHasExceededResourceTransferLimit(resourceCrateCountLimit, resourceCurrentCrateCount[0], resourceUnitCountLimit, resourceCurrentUnitCount[0])))
							{
								dictionary itemDictionary = packItem(appearanceItems[appearanceIter], objtype, allowOverride, resourceCrateCountLimit, resourceCurrentCrateCount, resourceUnitCountLimit, resourceCurrentUnitCount);
								if (itemDictionary != null)
								{
									appearanceDictionary.put(appearanceItems[appearanceIter], itemDictionary);
								}
								else
								{
									if (! allowOverride)
									{
										return SCRIPT_OVERRIDE;
									}
								}
							}
						}
					}
					
					characterData.put("appearanceDictionary", appearanceDictionary);
					LIVE_LOG("CharacterTransfer", "OnUploadCharacter() : appearance inventory items packed ("+ appearanceDictionary.toString() + ")");
					
				}
				else
				{
					CustomerServiceLog("CharacterTransfer", "OnUploadCharacter() : FAILED to pack appearance inventory items. appearance inventory object is not valid. TRANSFER FAILED");
					return SCRIPT_OVERRIDE;
				}
				
			}
			
			if (hasScript(self, "space.quest_logic.player_spacequest"))
			{
				characterData.put("attachSpacequestScript", true);
			}
			
			String[] strObjVars = new String[strObjVarLists.length];
			for (int intI = 0; intI < strObjVarLists.length; intI++)
			{
				testAbortScript();
				String strObjVar = getPackedObjvars(self, strObjVarLists[intI]);
				strObjVars[intI] = strObjVar;
			}
			characterData.put("strObjVars", strObjVars);
		}
		catch(Throwable t)
		{
			CustomerServiceLog("CharacterTransfer", "OnUploadCharacter() : an exception was thrown! characterData("+ characterData + ") TRANSFER FAILED : "+ t);
			return SCRIPT_OVERRIDE;
		}
		
		removeObjVar(self, "cts");
		CustomerServiceLog("CharacterTransfer", "OnUploadCharacter() complete");
		
		setObjVar(self, "disableLoginCtsInProgress", (getGameTime() + 600));
		messageTo(self, "disconnectPlayerCtsCompletedOrInProgress", null, 0.1f, false);
		
		return SCRIPT_CONTINUE;
	}
	
	public static final String[][] STARTING_CLOTHES = 
	{
		
		{
			"object/tangible/wearables/shirt/shirt_s14.iff",
			"object/tangible/wearables/jacket/jacket_s16.iff",
			"object/tangible/wearables/pants/pants_s14.iff",
			"object/tangible/wearables/boots/boots_s05.iff"
		}
		,
		
		{
			"object/tangible/wearables/shirt/shirt_s27.iff",
			"object/tangible/wearables/pants/pants_s07.iff",
			"object/tangible/wearables/boots/boots_s05.iff",
			"object/tangible/wearables/gloves/gloves_s03.iff"
		}
		,
		
		{
			"object/tangible/wearables/shirt/shirt_s27.iff",
			"object/tangible/wearables/pants/pants_s07.iff",
			"object/tangible/wearables/boots/boots_s05.iff",
			"object/tangible/wearables/gloves/gloves_s02.iff"
		}
		,
		{
			"object/tangible/wearables/shirt/shirt_s14.iff",
			"object/tangible/wearables/jacket/jacket_s16.iff",
			"object/tangible/wearables/pants/pants_s14.iff"
		}
		,
		
		{
			"object/tangible/wearables/shirt/shirt_s27.iff",
			"object/tangible/wearables/pants/pants_s07.iff",
			"object/tangible/wearables/gloves/gloves_s03.iff",
		}
	};
	
	
	public void createStartingEquipment(obj_id objPlayer) throws InterruptedException
	{
		obj_id playerInv = utils.getInventoryContainer(objPlayer);
		
		int gender = getGender(objPlayer);
		int species = getSpecies(objPlayer);
		int idx = -1;
		if (gender == GENDER_MALE)
		{
			if ((species == SPECIES_HUMAN || species == SPECIES_ZABRAK || species == SPECIES_BOTHAN || species == SPECIES_MON_CALAMARI || species == SPECIES_RODIAN || species == SPECIES_TWILEK))
			{
				idx = 0;
			}
			
			if (species == SPECIES_TRANDOSHAN)
			{
				idx = 3;
			}
		}
		else
		{
			if ((species == SPECIES_HUMAN || species == SPECIES_ZABRAK || species == SPECIES_BOTHAN || species == SPECIES_RODIAN || species == SPECIES_TWILEK))
			{
				idx = 1;
			}
			
			if (species == SPECIES_MON_CALAMARI)
			{
				idx = 2;
			}
			
			if (species == SPECIES_TRANDOSHAN)
			{
				idx = 4;
			}
		}
		if (idx >= 0)
		{
			for (int i = 0; i < STARTING_CLOTHES[idx].length; i++)
			{
				testAbortScript();
				obj_id newItem = createObject( STARTING_CLOTHES[idx][i], playerInv, "");
				if (isIdValid( newItem))
				{
					equip(newItem, playerInv);
				}
			}
		}
	}
	
	
	public int OnDownloadCharacter(obj_id self, byte[] packedData) throws InterruptedException
	{
		CustomerServiceLog("CharacterTransfer", "OnDownloadCharacter() begin");
		try
		{
			if (hasObjVar(self, "hasTransferred"))
			{
				return SCRIPT_OVERRIDE;
			}
			
			utils.setLocalVar(self, "ctsBeingUnpacked", true);
			
			setObjVar(self, "hasTransferred", 1);
			
			dictionary characterData = dictionary.unpack(packedData);
			
			if (characterData != null)
			{
				String skillTemplate = characterData.getString("skillTemplate");
				String workingSkill = characterData.getString("workingSkill");
				int combatLevel = characterData.getInt("combatLevel");
				
				CustomerServiceLog("CharacterTransfer", "setSkillTemplate("+ self + ", "+ skillTemplate + ") (combatLevel == "+ combatLevel + ", workingSkill == "+ workingSkill + ")");
				setObjVar(self, "clickRespec.combatLevel", combatLevel);
				setObjVar(self, "clickRespec.workingSkill", workingSkill);
				setObjVar(self, "clickRespec.cts", true);
				setSkillTemplate(self, skillTemplate);
				
				String[] skills = characterData.getStringArray("skills");
				for (int i = 0; i < skills.length; ++i)
				{
					testAbortScript();
					if (!hasSkill(self, skills[i]))
					{
						if (grantSkill(self, skills[i]))
						{
							CustomerServiceLog("CharacterTransfer", "grantSkill("+ self + ", "+ skills[i] + ")");
						}
						else
						{
							CustomerServiceLog("CharacterTransfer", "grantSkill("+ self + ", "+ skills[i] + ") FAILED");
							return SCRIPT_OVERRIDE;
						}
					}
				}
				
				String[] commands = characterData.getStringArray("commands");
				for (int i = 0; i < commands.length; ++i)
				{
					testAbortScript();
					if (!hasCommand(self, commands[i]))
					{
						if (grantCommand(self, commands[i]))
						{
							CustomerServiceLog("CharacterTransfer", "grantCommand("+ self + ", "+ commands[i] + ")");
						}
						else
						{
							CustomerServiceLog("CharacterTransfer", "grantCommand("+ self + ", "+ commands[i] + ") FAILED");
							return SCRIPT_OVERRIDE;
						}
					}
				}
				
				dictionary experiencePoints = characterData.getDictionary("experience_points");
				if (experiencePoints != null)
				{
					Set keySet = experiencePoints.keySet();
					Iterator expIterator = keySet.iterator();
					while (expIterator.hasNext())
					{
						testAbortScript();
						String expType = (String)expIterator.next();
						if (expType != null)
						{
							int expValue = experiencePoints.getInt(expType);
							if (getExperiencePoints(self, expType) != expValue)
							{
								int result = grantExperiencePoints(self, expType, expValue);
								if (result == XP_ERROR)
								{
									CustomerServiceLog("CharacterTransfer", "grantExperiencePoints("+ self + ", "+ expType + ", "+ expValue + ") FAILED");
									return SCRIPT_OVERRIDE;
								}
								else
								{
									CustomerServiceLog("CharacterTransfer", "grantExperiencePoints("+ self + ", "+ expType + ", "+ expValue + ") (non-roadmap exp)");
								}
							}
						}
					}
				}
				
				String[] strObjVars = characterData.getStringArray("strObjVars");
				for (int intI = 0; intI < strObjVars.length; intI++)
				{
					testAbortScript();
					setPackedObjvars(self, strObjVars[intI]);
				}
				
				removeObjVar(self, "galactic_reserve_cooldown");
				
				if (hasObjVar(self, CTS_LOT_COUNT))
				{
					int lot_count = getIntObjVar(self, CTS_LOT_COUNT);
					CustomerServiceLog("CharacterTransfer", "adjusting lot count("+ self + ", "+ lot_count + ")");
					player_structure.adjustLotCount(getPlayerObject(self), lot_count);
				}
				
				int sourceCharacterAlignedFaction = characterData.getInt("pvp_aligned_faction");
				int sourceCharacterPvpType = characterData.getInt("pvp_type");
				switch (sourceCharacterPvpType)
				{
					case PVPTYPE_NEUTRAL:
					
					if (sourceCharacterAlignedFaction == 0)
					{
						pvpMakeNeutral(self);
					}
					break;
					case PVPTYPE_COVERT:
					pvpMakeCovert(self);
					break;
					case PVPTYPE_DECLARED:
					pvpMakeDeclared(self);
					break;
				}
				
				pvpSetAlignedFaction(self, sourceCharacterAlignedFaction);
				
				if ((sourceCharacterAlignedFaction != 0) && (sourceCharacterPvpType == PVPTYPE_NEUTRAL))
				{
					pvpMakeOnLeave(self);
				}
				
				ctsUseOnlySetGcwInfo(self, characterData.getInt("gcw_current_point"), characterData.getInt("gcw_current_rating"), characterData.getInt("gcw_current_pvp_kill"), characterData.getLong("gcw_lifetime_point"), characterData.getInt("gcw_max_imperial_rating"), characterData.getInt("gcw_max_rebel_rating"), characterData.getInt("gcw_lifetime_pvp_kill"), characterData.getInt("gcw_next_rating_calc_time"));
				
				obj_id[] old_waypoints = getWaypointsInDatapad(self);
				if (old_waypoints != null && old_waypoints.length > 0)
				{
					for (int i = 0; i < old_waypoints.length; ++i)
					{
						testAbortScript();
						destroyWaypointInDatapad(old_waypoints[i], self);
					}
				}
				dictionary waypoints = characterData.getDictionary("waypoints");
				if (waypoints != null)
				{
					Set keySet = waypoints.keySet();
					Iterator waypointIterator = keySet.iterator();
					while (waypointIterator.hasNext())
					{
						testAbortScript();
						obj_id key = (obj_id)waypointIterator.next();
						dictionary waypointDict = waypoints.getDictionary(key);
						unpackWaypoint(waypointDict);
					}
				}
				
				obj_id playerObject = getPlayerObject(self);
				if (isIdValid(playerObject))
				{
					if (characterData.containsKey("quests"))
					{
						byte[] quests = characterData.getByteArray("quests");
						setAutoVariableFromByteStream(playerObject, "quests", quests);
					}
					
					if (characterData.containsKey("activeQuests"))
					{
						byte[] activeQuests = characterData.getByteArray("activeQuests");
						setAutoVariableFromByteStream(playerObject, "activeQuests", activeQuests);
					}
					
					if (characterData.containsKey("completedQuests"))
					{
						byte[] completedQuests = characterData.getByteArray("completedQuests");
						setAutoVariableFromByteStream(playerObject, "completedQuests", completedQuests);
					}
					
					if (characterData.containsKey("currentQuest"))
					{
						byte[] currentQuest = characterData.getByteArray("currentQuest");
						setAutoVariableFromByteStream(playerObject, "currentQuest", currentQuest);
					}
					
					groundquests.reattachQuestScripts(self);
					
					if (characterData.containsKey("bornDate"))
					{
						byte[] bornDate = characterData.getByteArray("bornDate");
						setAutoVariableFromByteStream(playerObject, "bornDate", bornDate);
					}
					
					if (characterData.containsKey("playedTime"))
					{
						byte[] playedTime = characterData.getByteArray("playedTime");
						setAutoVariableFromByteStream(playerObject, "playedTime", playedTime);
					}
					
					if (characterData.containsKey("food"))
					{
						byte[] food = characterData.getByteArray("food");
						setAutoVariableFromByteStream(playerObject, "food", food);
					}
					
					if (characterData.containsKey("maxFood"))
					{
						byte[] maxFood = characterData.getByteArray("maxFood");
						setAutoVariableFromByteStream(playerObject, "maxFood", maxFood);
					}
					
					if (characterData.containsKey("drink"))
					{
						byte[] drink = characterData.getByteArray("drink");
						setAutoVariableFromByteStream(playerObject, "drink", drink);
					}
					
					if (characterData.containsKey("maxDrink"))
					{
						byte[] maxDrink = characterData.getByteArray("maxDrink");
						setAutoVariableFromByteStream(playerObject, "maxDrink", maxDrink);
					}
					
					if (characterData.containsKey("meds"))
					{
						byte[] meds = characterData.getByteArray("meds");
						setAutoVariableFromByteStream(playerObject, "meds", meds);
					}
					
					if (characterData.containsKey("maxMeds"))
					{
						byte[] maxMeds = characterData.getByteArray("maxMeds");
						setAutoVariableFromByteStream(playerObject, "maxMeds", maxMeds);
					}
					
					if (characterData.containsKey("matchMakingPersonalProfileId"))
					{
						byte[] matchMakingPersonalProfileId = characterData.getByteArray("matchMakingPersonalProfileId");
						setAutoVariableFromByteStream(playerObject, "matchMakingPersonalProfileId", matchMakingPersonalProfileId);
					}
					
					if (characterData.containsKey("matchMakingCharacterProfileId"))
					{
						byte[] matchMakingCharacterProfileId = characterData.getByteArray("matchMakingCharacterProfileId");
						setAutoVariableFromByteStream(playerObject, "matchMakingCharacterProfileId", matchMakingCharacterProfileId);
					}
					
					if (characterData.containsKey("workingSkill"))
					{
						byte[] playerObjectWorkingSkill = characterData.getByteArray("workingSkill");
						setAutoVariableFromByteStream(playerObject, "workingSkill", playerObjectWorkingSkill);
					}
					
					if (characterData.containsKey("skillTemplate"))
					{
						byte[] playerObjectSkillTemplate = characterData.getByteArray("skillTemplate");
						setAutoVariableFromByteStream(playerObject, "skillTemplate", playerObjectSkillTemplate);
					}
					
					if (characterData.containsKey("collections"))
					{
						byte[] collections = characterData.getByteArray("collections");
						setAutoVariableFromByteStream(playerObject, "collections", collections);
					}
					
					if (characterData.containsKey("collections2"))
					{
						byte[] collections2 = characterData.getByteArray("collections2");
						setAutoVariableFromByteStream(playerObject, "collections2", collections2);
					}
					
					if (characterData.containsKey("source_cluster"))
					{
						
						String sourceCluster = characterData.getString("source_cluster");
						
						if (characterData.containsKey("source_character_oid") && characterData.containsKey("source_character_name"))
						{
							int ctsHistoryIndex = 1;
							while (true)
							{
								testAbortScript();
								if (!hasObjVar(self, "ctsHistory." + ctsHistoryIndex))
								{
									setObjVar(self, "ctsHistory."+ ctsHistoryIndex, ""+ getCalendarTime() + " "+ sourceCluster + " "+ characterData.getObjId("source_character_oid") + " "+ characterData.getString("source_character_name"));
									break;
								}
								
								++ctsHistoryIndex;
							}
						}
						
						String currentCluster = getClusterName();
						boolean grantCtsTitle = !sourceCluster.equals(currentCluster);
						
						sourceCluster = toLower(sourceCluster);
						sourceCluster = sourceCluster.replace('-', '_');
						
						if (grantCtsTitle)
						{
							modifyCollectionSlotValue(self, "cts_from_"+ sourceCluster, 1L);
						}
						
						if (characterData.containsKey("freeCtsTransactionRuleName"))
						{
							setObjVar(self, characterData.getString("freeCtsTransactionRuleName") + "."+ sourceCluster, currentCluster);
						}
					}
					
					int[] quests = questGetAllActiveQuestIds(self);
					for (int i = 0; i < quests.length; ++i)
					{
						testAbortScript();
						int questId = quests[i];
						
						for (int taskId = 0; taskId < 16; ++taskId)
						{
							testAbortScript();
							if (questIsTaskActive(questId, taskId, self))
							{
								groundquests.refreshQuestWaypoints(questId, taskId, true, self);
							}
						}
					}
				}
				
				boolean withItems = characterData.getBoolean("withItems");
				if (withItems == true)
				{
					
					obj_id[] existingItems = getInventoryAndEquipment(self);
					obj_id inv = utils.getInventoryContainer(self);
					
					if (existingItems != null)
					{
						int existingIter;
						for (existingIter = 0; existingIter < existingItems.length; ++existingIter)
						{
							testAbortScript();
							if (existingItems[existingIter] != self && existingItems[existingIter] != inv)
							{
								LIVE_LOG("CharacterTransfer", "OnDownloadCharacter() : destroying unwanted pre-existing destination item : "+ getTemplateName(existingItems[existingIter]));
								destroyObject(existingItems[existingIter]);
							}
						}
					}
					
					int maxMoneyTransferAmount = 100000000;
					{
						int tempMaxMoneyTransferAmount = utils.stringToInt( getConfigSetting("GameServer", "maxMoneyTransfer") );
						if (tempMaxMoneyTransferAmount > 0)
						{
							maxMoneyTransferAmount = tempMaxMoneyTransferAmount;
						}
					}
					
					if (characterData.containsKey("moneyFromCash"))
					{
						int moneyFromCash = characterData.getInt("moneyFromCash");
						CustomerServiceLog("CharacterTransfer", "OnDownloadCharacter : unpacking "+ moneyFromCash + " credits to cash");
						
						while (moneyFromCash > 0)
						{
							testAbortScript();
							int transferAmount = utils.clipRange(moneyFromCash, 0, maxMoneyTransferAmount);
							
							if (!transferBankCreditsFromNamedAccount("characterTransfer", self, transferAmount, null, null, null))
							{
								CustomerServiceLog("CharacterTransfer", "transferBankCreditsFromNamedAccount(characterTransfer, "+ self + ", "+ transferAmount + ", null, null, null) FAILED");
								break;
							}
							else
							{
								
								if (!withdrawCashFromBank(self, transferAmount, null, null, null))
								{
									CustomerServiceLog("CharacterTransfer", "withdrawCashFromBank(characterTransfer, "+ self + ", "+ transferAmount + ", null, null, null) FAILED");
									break;
								}
								
								CustomerServiceLog("CharacterTransfer", "OnDownloadCharacter : +"+ transferAmount + " credits to cash");
							}
							
							moneyFromCash -= maxMoneyTransferAmount;
						}
					}
					
					if (characterData.containsKey("moneyFromBank"))
					{
						int moneyFromBank = characterData.getInt("moneyFromBank");
						CustomerServiceLog("CharacterTransfer", "OnDownloadCharacter : unpacking "+ moneyFromBank + " credits to bank");
						
						while (moneyFromBank > 0)
						{
							testAbortScript();
							int transferAmount = utils.clipRange(moneyFromBank, 0, maxMoneyTransferAmount);
							
							if (!transferBankCreditsFromNamedAccount("characterTransfer", self, transferAmount, null, null, null))
							{
								CustomerServiceLog("CharacterTransfer", "transferBankCreditsFromNamedAccount(characterTransfer, "+ self + ", "+ transferAmount + ", null, null, null) FAILED");
								break;
							}
							
							CustomerServiceLog("CharacterTransfer", "OnDownloadCharacter : +"+ transferAmount + " credits to bank");
							
							moneyFromBank -= maxMoneyTransferAmount;
						}
					}
					
					obj_id playerInventory = utils.getInventoryContainer(self);
					obj_id playerBank = utils.getPlayerBank(self);
					
					utils.setScriptVar(self, "autostack.ignoreitems", true);
					
					CustomerServiceLog("CharacterTransfer", "OnDownloadCharacter : unpacking inventory items");
					dictionary inventoryDictionary = characterData.getDictionary("inventoryDictionary");
					if (inventoryDictionary == null)
					{
						return SCRIPT_OVERRIDE;
					}
					else
					{
						Set keySet = inventoryDictionary.keySet();
						Iterator inventoryIterator = keySet.iterator();
						while (inventoryIterator.hasNext())
						{
							testAbortScript();
							obj_id key = (obj_id)inventoryIterator.next();
							if (isIdNull(key))
							{
								CustomerServiceLog("CharacterTransfer", "OnDownloadCharacter() : could not retrieve a key from the inventory dictionary iterator. TRANSFER FAILED");
								return SCRIPT_OVERRIDE;
							}
							
							dictionary itemDictionary = inventoryDictionary.getDictionary(key);
							
							if (ctsCheckAndLogUniqueStaticItemAlreadyExists(playerInventory, itemDictionary))
							{
								continue;
							}
							
							obj_id unpackedItem = unpackItem(playerInventory, itemDictionary);
							if (!isIdValid(unpackedItem))
							{
								if (isIdNull(unpackedItem))
								{
									CustomerServiceLog("CharacterTransfer", "OnDownloadCharacter() : unpackItem() returned NULL id for item: "+ itemDictionary);
									return SCRIPT_OVERRIDE;
								}
								else
								{
									CustomerServiceLog("CharacterTransfer", "OnDownloadCharacter() : unpackItem() returned invalid id "+ unpackedItem + " for item: "+ itemDictionary);
									continue;
								}
							}
						}
					}
					
					CustomerServiceLog("CharacterTransfer", "OnDownloadCharacter : unpacking bank items");
					dictionary bankDictionary = characterData.getDictionary("bankDictionary");
					if (bankDictionary == null)
					{
						return SCRIPT_OVERRIDE;
					}
					else
					{
						Set keySet = bankDictionary.keySet();
						Iterator bankIterator = keySet.iterator();
						while (bankIterator.hasNext())
						{
							testAbortScript();
							obj_id key = (obj_id)bankIterator.next();
							if (isIdNull(key))
							{
								CustomerServiceLog("CharacterTransfer", "OnDownloadCharacter() : could not retrieve a key from the bank dictionary iterator. TRANSFER FAILED");
								return SCRIPT_OVERRIDE;
							}
							
							dictionary itemDictionary = bankDictionary.getDictionary(key);
							
							if (ctsCheckAndLogUniqueStaticItemAlreadyExists(playerInventory, itemDictionary))
							{
								continue;
							}
							
							obj_id unpackedItem = unpackItem(playerInventory, itemDictionary);
							if (!isIdValid(unpackedItem))
							{
								if (isIdNull(unpackedItem))
								{
									CustomerServiceLog("CharacterTransfer", "OnDownloadCharacter() : unpackItem() returned NULL id for item: "+ itemDictionary);
									return SCRIPT_OVERRIDE;
								}
								else
								{
									CustomerServiceLog("CharacterTransfer", "OnDownloadCharacter() : unpackItem() returned invalid id "+ unpackedItem + " for item: "+ itemDictionary);
									continue;
								}
							}
							
							if (putIn(unpackedItem, playerBank))
							{
								CustomerServiceLog("CharacterTransfer", "unpackItem() : moved item "+ unpackedItem + " into bank "+ playerBank);
							}
							else
							{
								CustomerServiceLog("CharacterTransfer", "unpackItem() : FAILED to move item "+ unpackedItem + " into bank "+ playerBank);
							}
						}
					}
					
					obj_id playerDatapad = utils.getPlayerDatapad(self);
					obj_id datapadObjects[] = getContents(playerDatapad);
					for (int oldDatapad = 0; oldDatapad < datapadObjects.length; ++oldDatapad)
					{
						testAbortScript();
						destroyObject(datapadObjects[oldDatapad]);
					}
					CustomerServiceLog("CharacterTransfer", "OnDownloadCharacter : unpacking datapad items");
					dictionary datapadDictionary = characterData.getDictionary("datapadDictionary");
					if (datapadDictionary == null)
					{
						return SCRIPT_OVERRIDE;
					}
					else
					{
						
						dictionary datapadItemOidTranslation = new dictionary();
						
						Vector shipsWithDroidControlDevice = new Vector();
						shipsWithDroidControlDevice.setSize(0);
						
						Set keySet = datapadDictionary.keySet();
						Iterator datapadIterator = keySet.iterator();
						while (datapadIterator.hasNext())
						{
							testAbortScript();
							obj_id key = (obj_id)datapadIterator.next();
							if (isIdNull(key))
							{
								
								CustomerServiceLog("CharacterTransfer", "OnDownloadCharacter() : could not retrieve a key from the datapad dictionary iterator. TRANSFER FAILED");
								return SCRIPT_OVERRIDE;
							}
							
							dictionary itemDictionary = datapadDictionary.getDictionary(key);
							
							if (ctsCheckAndLogUniqueStaticItemAlreadyExists(playerDatapad, itemDictionary))
							{
								continue;
							}
							
							obj_id unpackedItem = unpackItem(playerDatapad, itemDictionary);
							if (!isIdValid(unpackedItem))
							{
								CustomerServiceLog("CharacterTransfer", "unpackItem() : FAILED to unpack datapad item (original oid "+ key + ") into datapad "+ playerDatapad);
							}
							else
							{
								
								datapadItemOidTranslation.put(key, unpackedItem);
								
								if (utils.hasScript(unpackedItem, "space.ship_control_device.ship_control_device"))
								{
									obj_id ship = space_transition.getShipFromShipControlDevice(unpackedItem);
									if (isIdValid(ship) && utils.hasLocalVar(ship, "droid_control_device"))
									{
										utils.addElement(shipsWithDroidControlDevice, ship);
									}
								}
							}
						}
						
						for (int i = 0; i < shipsWithDroidControlDevice.size(); ++i)
						{
							testAbortScript();
							obj_id originalDroidControlDevice = utils.getObjIdLocalVar(((obj_id)(shipsWithDroidControlDevice.get(i))), "droid_control_device");
							if (isIdValid(originalDroidControlDevice) && datapadItemOidTranslation.containsKey(originalDroidControlDevice))
							{
								obj_id newDroidControlDevice = datapadItemOidTranslation.getObjId(originalDroidControlDevice);
								if (isIdValid(newDroidControlDevice))
								{
									associateDroidControlDeviceWithShip(((obj_id)(shipsWithDroidControlDevice.get(i))), newDroidControlDevice);
								}
							}
						}
					}
					
					obj_id buyBackContainer = getObjIdObjVar(self, smuggler.BUYBACK_CONTAINER_VAR);
					if (isIdValid(buyBackContainer))
					{
						CustomerServiceLog("CharacterTransfer", "OnDownloadCharacter : source character had an existing buy back container: "+buyBackContainer);
						
						obj_id buyBackObjects[] = getContents(buyBackContainer);
						if (buyBackObjects != null && buyBackObjects.length > 0)
						{
							for (int oldContainer = 0; oldContainer < buyBackObjects.length; ++oldContainer)
							{
								testAbortScript();
								destroyObject(buyBackObjects[oldContainer]);
							}
						}
					}
					
					CustomerServiceLog("CharacterTransfer", "OnDownloadCharacter : unpacking buy back items");
					dictionary buyBackDictionary = characterData.getDictionary("buyBackDictionary");
					if (buyBackDictionary == null || buyBackDictionary.isEmpty())
					{
						CustomerServiceLog("CharacterTransfer", "OnDownloadCharacter : source character did not have a buy back dictionary data!!");
					}
					else
					{
						
						if (!isIdValid(buyBackContainer))
						{
							CustomerServiceLog("CharacterTransfer", "OnDownloadCharacter : source character did not have a valid buy back container.");
							buyBackContainer = smuggler.getBuyBackContainerObjVar(self);
						}
						
						if (!isIdValid(buyBackContainer))
						{
							CustomerServiceLog("CharacterTransfer", "OnDownloadCharacter : source character could not create buy back container. TRANSFER FAILED");
							return SCRIPT_OVERRIDE;
						}
						
						Set keySet = buyBackDictionary.keySet();
						Iterator buyBackIterator = keySet.iterator();
						while (buyBackIterator.hasNext())
						{
							testAbortScript();
							obj_id key = (obj_id)buyBackIterator.next();
							if (isIdNull(key))
							{
								CustomerServiceLog("CharacterTransfer", "OnDownloadCharacter() : could not retrieve a key from the buy back dictionary iterator. TRANSFER FAILED");
								return SCRIPT_OVERRIDE;
							}
							
							dictionary itemDictionary = buyBackDictionary.getDictionary(key);
							obj_id unpackedItem = unpackItem(buyBackContainer, itemDictionary);
							if (!isIdValid(unpackedItem))
							{
								CustomerServiceLog("CharacterTransfer", "unpackItem() : FAILED to unpack buy back item (original oid "+ key + ") into buy back container "+ buyBackContainer);
							}
							
						}
					}
					
					obj_id playerHangar = utils.getPlayerHangar(self);
					
					if (isIdValid(playerHangar))
					{
						obj_id hangarObjects[] = getContents(playerHangar);
						for (int oldHangar = 0; oldHangar < hangarObjects.length; ++oldHangar)
						{
							testAbortScript();
							destroyObject(hangarObjects[oldHangar]);
						}
					}
					CustomerServiceLog("CharacterTransfer", "OnDownloadCharacter : unpacking hangar items");
					dictionary hangarDictionary = characterData.getDictionary("hangarDictionary");
					if (hangarDictionary == null || hangarDictionary.isEmpty())
					{
						CustomerServiceLog("CharacterTransfer", "OnDownloadCharacter : source character did not have a hangar object.");
					}
					else
					{
						
						if (!isIdValid(playerHangar))
						{
							playerHangar = createObject("object/tangible/datapad/character_hangar_datapad.iff", self, "hangar");
						}
						
						if (!isIdValid(playerHangar))
						{
							CustomerServiceLog("CharacterTransfer", "OnDownloadCharacter : source character could not create hangar. TRANSFER FAILED");
							return SCRIPT_OVERRIDE;
						}
						
						setObjVar(self, player_structure.OBJVAR_HANGAR_CREATED, playerHangar);
						
						persistObject(playerHangar);
						
						dictionary hangarItemOidTranslation = new dictionary();
						
						Vector shipsWithDroidControlDevice = new Vector();
						shipsWithDroidControlDevice.setSize(0);
						
						Set keySet = hangarDictionary.keySet();
						Iterator hangarIterator = keySet.iterator();
						while (hangarIterator.hasNext())
						{
							testAbortScript();
							obj_id key = (obj_id)hangarIterator.next();
							if (isIdNull(key))
							{
								
								CustomerServiceLog("CharacterTransfer", "OnDownloadCharacter() : could not retrieve a key from the hangar dictionary iterator. TRANSFER FAILED");
								return SCRIPT_OVERRIDE;
							}
							
							dictionary itemDictionary = hangarDictionary.getDictionary(key);
							
							if (ctsCheckAndLogUniqueStaticItemAlreadyExists(playerDatapad, itemDictionary))
							{
								continue;
							}
							
							obj_id unpackedItem = unpackItem(playerHangar, itemDictionary);
							if (!isIdValid(unpackedItem))
							{
								CustomerServiceLog("CharacterTransfer", "unpackItem() : FAILED to unpack hangar item (original oid "+ key + ") into hangar "+ playerHangar);
							}
							else
							{
								
								hangarItemOidTranslation.put(key, unpackedItem);
								
								if (utils.hasScript(unpackedItem, "space.ship_control_device.ship_control_device"))
								{
									obj_id ship = space_transition.getShipFromShipControlDevice(unpackedItem);
									if (isIdValid(ship) && utils.hasLocalVar(ship, "droid_control_device"))
									{
										utils.addElement(shipsWithDroidControlDevice, ship);
									}
								}
							}
						}
						
						for (int i = 0; i < shipsWithDroidControlDevice.size(); ++i)
						{
							testAbortScript();
							obj_id originalDroidControlDevice = utils.getObjIdLocalVar(((obj_id)(shipsWithDroidControlDevice.get(i))), "droid_control_device");
							if (isIdValid(originalDroidControlDevice) && hangarItemOidTranslation.containsKey(originalDroidControlDevice))
							{
								obj_id newDroidControlDevice = hangarItemOidTranslation.getObjId(originalDroidControlDevice);
								if (isIdValid(newDroidControlDevice))
								{
									associateDroidControlDeviceWithShip(((obj_id)(shipsWithDroidControlDevice.get(i))), newDroidControlDevice);
								}
							}
						}
					}
					
					obj_id playerAppearance = getAppearanceInventory(self);
					obj_id appearanceObjects[] = getContents(playerAppearance);
					for (int oldAppearance = 0; oldAppearance < appearanceObjects.length; ++oldAppearance)
					{
						testAbortScript();
						destroyObject(appearanceObjects[oldAppearance]);
					}
					CustomerServiceLog("CharacterTransfer", "OnDownloadCharacter : unpacking appearance inventory items");
					dictionary appearanceDictionary = characterData.getDictionary("appearanceDictionary");
					if (appearanceDictionary == null)
					{
						return SCRIPT_OVERRIDE;
					}
					else
					{
						Set keySet = appearanceDictionary.keySet();
						Iterator appearanceIterator = keySet.iterator();
						while (appearanceIterator.hasNext())
						{
							testAbortScript();
							obj_id key = (obj_id)appearanceIterator.next();
							if (isIdNull(key))
							{
								
								CustomerServiceLog("CharacterTransfer", "OnDownloadCharacter() : could not retrieve a key from the appearance dictionary iterator. TRANSFER FAILED");
								return SCRIPT_OVERRIDE;
							}
							
							dictionary itemDictionary = appearanceDictionary.getDictionary(key);
							
							if (ctsCheckAndLogUniqueStaticItemAlreadyExists(playerAppearance, itemDictionary))
							{
								continue;
							}
							
							obj_id unpackedItem = unpackItem(playerInventory, itemDictionary);
							if (!isIdValid(unpackedItem))
							{
								CustomerServiceLog("CharacterTransfer", "unpackItem() : FAILED to unpack appearance inventory item (original oid "+ key + ") into appearance inventory "+ playerAppearance);
							}
						}
						
					}
					
					utils.removeScriptVar(self, "autostack.ignoreitems");
					
				}
				else
				{
					
					obj_id inventory = getObjectInSlot( self, "inventory");
					if (isIdValid(inventory))
					{
						weapons.createWeapon("object/weapon/ranged/pistol/pistol_cdef.iff", inventory, 0.90f);
						createObject("object/tangible/survey_tool/survey_tool_mineral.iff", inventory, "");
						weapons.createWeapon("object/weapon/melee/knife/knife_survival.iff", inventory, 0.90f);
						createObject("object/tangible/crafting/station/generic_tool.iff", inventory, "");
						if (utils.isProfession(self, utils.ENTERTAINER))
						{
							createObject("object/tangible/instrument/slitherhorn.iff", inventory, "");
						}
						createStartingEquipment(self);
					}
				}
				
				if (characterData.containsKey("attachSpacequestScript") && characterData.getBoolean("attachSpacequestScript"))
				{
					attachScript(self, "space.quest_logic.player_spacequest");
				}
			}
			else
			{
				return SCRIPT_OVERRIDE;
			}
		}
		catch(Throwable t)
		{
			removeObjVar(self, "cts");
			utils.removeLocalVar(self, "ctsBeingUnpacked");
			CustomerServiceLog("CharacterTransfer", "OnDownloadCharacter() : an exception was thrown! TRANSFER FAILED : "+ t);
			return SCRIPT_OVERRIDE;
		}
		
		CustomerServiceLog("CharacterTransfer", "OnDownloadCharacter() complete");
		
		removeObjVar(self, "cts");
		removeObjVar(self, "preserve_existing_lightsaber_crystal_owner_objvar");
		utils.removeLocalVar(self, "ctsBeingUnpacked");
		return SCRIPT_CONTINUE;
	}
	
	
	public int OnSkillModDone(obj_id self, String modName, boolean isDead) throws InterruptedException
	{
		if (modName == null)
		{
			return SCRIPT_CONTINUE;
		}
		
		if (modName.indexOf( "food_" ) == 0)
		{
			
			utils.removeScriptVar( self, modName );
			
		}
		
		trial.bumpSession(self, "displayDefensiveMods");
		messageTo(self, "setDisplayOnlyDefensiveMods", trial.getSessionDict(self, "displayDefensiveMods") , 5, false);
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int removeDelayedFoodEffect(obj_id self, dictionary params) throws InterruptedException
	{
		if (params == null)
		{
			return SCRIPT_CONTINUE;
		}
		
		String type = params.getString( "type");
		if (!utils.hasScriptVar( self, "food."+type+".dur" ))
		{
			return SCRIPT_CONTINUE;
		}
		int time = utils.getIntScriptVar( self, "food."+type+".time");
		if (getGameTime() - time < 1799)
		{
			return SCRIPT_CONTINUE;
		}
		
		utils.removeScriptVarTree( self, "food."+type );
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int removeDurationFoodEffect(obj_id self, dictionary params) throws InterruptedException
	{
		if (params == null)
		{
			return SCRIPT_CONTINUE;
		}
		
		String type = params.getString( "type");
		if (!utils.hasScriptVar( self, "food."+type+".dur" ))
		{
			return SCRIPT_CONTINUE;
		}
		int time = utils.getIntScriptVar( self, "food."+type+".time");
		if (getGameTime() - time < 1799)
		{
			return SCRIPT_CONTINUE;
		}
		
		utils.removeScriptVarTree( self, "food."+type );
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int handleSpiceDownerEffect(obj_id self, dictionary params) throws InterruptedException
	{
		if (buff.hasBuff(self, "spice_downer"))
		{
			buff.removeBuff(self, "spice_downer");
		}
		
		float duration = buff.getDuration("spice_downer");
		
		float downerModified = ((float)getEnhancedSkillStatisticModifierUncapped(self, "resistance_spice_downer"));
		if (downerModified > 0)
		{
			duration -= downerModified;
			if (duration <= 0)
			{
				sendSystemMessage(self, SID_SPICE_DOWNER_DURATION_NEGATED);
				return SCRIPT_CONTINUE;
			}
			sendSystemMessage(self, SID_SPICE_DOWNER_DURATION_REDUCED);
		}
		
		if (buff.applyBuff(self, "spice_downer", duration))
		{
			doAnimationAction(self, anims.PLAYER_HEAVY_COUGH_VOMIT);
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int handleGenericDownerEffect(obj_id self, dictionary params) throws InterruptedException
	{
		if (buff.hasBuff(self, "generic_downer"))
		{
			buff.removeBuff(self, "generic_downer");
		}
		
		float duration = buff.getDuration("generic_downer");
		
		if (buff.applyBuff(self, "generic_downer", duration))
		{
			doAnimationAction(self, anims.PLAYER_HEAVY_COUGH_VOMIT);
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int OnStomachUpdate(obj_id self) throws InterruptedException
	{
		player_stomach.getStomach( self, 0 );
		player_stomach.getStomach( self, 1 );
		return SCRIPT_CONTINUE;
	}
	
	
	public int grantUnmodifiedExperienceOnSelf(obj_id self, dictionary params) throws InterruptedException
	{
		
		if (npe.hasReachedMaxTutorialLevel(self))
		{
			int hadNotif = utils.getIntScriptVar(self, "npe.level_capped");
			if (hadNotif == 0)
			{
				string_id chat = new string_id("npe", "pop_level_cap");
				obj_id building = getTopMostContainer(self);
				obj_id droid = utils.getObjIdScriptVar(building, "objDroidInvis");
				npe.commTutorialPlayer(droid, self, 10, chat, "sound/dro_r2_3_danger.snd", "object/mobile/r2.iff");
				utils.setScriptVar(self, "npe.level_capped", 1);
			}
			return SCRIPT_CONTINUE;
		}
		int playerLevel = getLevel(self);
		if (isFreeTrialAccount(self))
		{
			if (playerLevel >= xp.TRIAL_LEVEL_CAP)
			{
				return SCRIPT_CONTINUE;
			}
		}
		
		String xp_type = params.getString("xp_type");
		int amt = params.getInt("amt");
		String fromCallback = params.getString("fromCallback");
		dictionary fromCallbackData = params.getDictionary("fromCallbackData");
		obj_id fromId = params.getObjId("fromId");
		
		if (luck.isLucky(self, 0.001f))
		{
			float bonus = amt * 0.1f;
			if (bonus < 1.0f)
			{
				bonus = 1.0f;
			}
			
			amt += bonus;
		}
		
		xp._grantUnmodifiedExperience(self, xp_type, amt, fromCallback, fromCallbackData, fromId);
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int grantSquadLeaderXpResult(obj_id self, dictionary params) throws InterruptedException
	{
		int granted = params.getInt(xp.GRANT_XP_RESULT_VALUE);
		dictionary data = params.getDictionary(xp.GRANT_XP_RETURN_DATA);
		obj_id player = data.getObjId("player");
		int amt = data.getInt("amt");
		xp.grantSquadLeaderXpResult(player, granted, amt);
		return SCRIPT_CONTINUE;
	}
	
	
	public int cmdSetExperienceResult(obj_id self, dictionary params) throws InterruptedException
	{
		if (isGod(self))
		{
			int granted = params.getInt(xp.GRANT_XP_RESULT_VALUE);
			dictionary data = params.getDictionary(xp.GRANT_XP_RETURN_DATA);
			obj_id target = data.getObjId("target");
			String xp_type = data.getString("xp_type");
			sendSystemMessageTestingOnly(self, "/setExperience: granted ("+ target + ")"+ utils.getStringName(target) + " "+ granted + "pts of "+ getString(new string_id("exp_n",xp_type)) + " ("+ xp_type + ") experience");
		}
		return SCRIPT_CONTINUE;
	}
	
	
	public int OnPerformEmote(obj_id self, String emote) throws InterruptedException
	{
		int[] emotes = dataTableGetIntColumn(veteran_deprecated.EMOTES_DATATABLE, veteran_deprecated.EMOTES_COLUMN_EMOTE);
		if (emotes == null)
		{
			return SCRIPT_CONTINUE;
		}
		
		int emoteCrc = getStringCrc(emote);
		for (int i = 0; i < emotes.length; ++i)
		{
			testAbortScript();
			if (emoteCrc == emotes[i])
			{
				
				int[] playerEmotes = getIntArrayObjVar(self, veteran_deprecated.OBJVAR_VETERAN_EMOTES);
				if (playerEmotes != null)
				{
					int emoteGroup = dataTableGetInt(veteran_deprecated.EMOTES_DATATABLE, i, veteran_deprecated.EMOTES_COLUMN_GROUP);
					for (int j = 0; j < playerEmotes.length; ++j)
					{
						testAbortScript();
						if (emoteGroup == playerEmotes[j])
						{
							
							return SCRIPT_CONTINUE;
						}
					}
				}
				
				return SCRIPT_OVERRIDE;
			}
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int cmdGetVeteranRewardTime(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!"true".equals(getConfigSetting("GameServer", "enableVeteranRewards")))
		{
			return SCRIPT_CONTINUE;
		}
		
		if (isGod(self))
		{
			if (!isIdValid(target))
			{
				target = self;
			}
			if (veteran_deprecated.checkVeteranTarget(target))
			{
				int veteranTime = getIntObjVar(target, veteran_deprecated.OBJVAR_TIME_ACTIVE);
				prose_package pp = new prose_package();
				pp.stringId = veteran_deprecated.SID_VETERAN_TIME_ACTIVE;
				pp.target.id = target;
				pp.digitInteger = veteranTime;
				sendSystemMessageProse(self, pp);
			}
		}
		else
		{
			if (hasObjVar(self, veteran_deprecated.OBJVAR_TIME_ACTIVE))
			{
				int veteranTime = getIntObjVar(self, veteran_deprecated.OBJVAR_TIME_ACTIVE);
				prose_package pp = new prose_package();
				pp.stringId = veteran_deprecated.SID_VETERAN_SELF_TIME_ACTIVE;
				pp.digitInteger = veteranTime;
				sendSystemMessageProse(self, pp);
			}
			else
			{
				sendSystemMessage(self, veteran_deprecated.SID_SYSTEM_INACTIVE);
			}
		}
		return SCRIPT_CONTINUE;
	}
	
	
	public int cmdListVeteranRewards(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!"true".equals(getConfigSetting("GameServer", "enableVeteranRewards")))
		{
			return SCRIPT_CONTINUE;
		}
		
		if (!isGod(self) && !hasObjVar(self, veteran_deprecated.OBJVAR_TIME_ACTIVE))
		{
			
			sendSystemMessage(self, veteran_deprecated.SID_SYSTEM_INACTIVE);
			return SCRIPT_CONTINUE;
		}
		
		int[] templateCrcs = dataTableGetIntColumn(veteran_deprecated.REWARDS_DATATABLE, veteran_deprecated.REWARDS_COLUMN_TEMPLATE);
		if (templateCrcs == null)
		{
			return SCRIPT_CONTINUE;
		}
		int[] milestones = dataTableGetIntColumn(veteran_deprecated.REWARDS_DATATABLE, veteran_deprecated.REWARDS_COLUMN_MILESTONE);
		if (milestones == null)
		{
			return SCRIPT_CONTINUE;
		}
		string_id[] nameIds = getNamesFromTemplates(templateCrcs);
		if (nameIds == null)
		{
			return SCRIPT_CONTINUE;
		}
		
		if (milestones.length != nameIds.length)
		{
			return SCRIPT_CONTINUE;
		}
		
		String[] milestonesText = new String[nameIds.length];
		String daysText = getString(veteran_deprecated.SID_DAYS);
		for (int i = 0; i < nameIds.length; ++i)
		{
			testAbortScript();
			if (nameIds[i] != null)
			{
				milestonesText[i] = "@" + nameIds[i] + " :\\>200" + (milestones[i] * veteran_deprecated.DAYS_PER_MILESTONE) + daysText;
			}
			else
			{
				milestonesText[i] = "<error>";
			}
		}
		
		int pid = sui.listbox(self, "", milestonesText);
		if (pid < 0)
		{
			CustomerServiceLog("veteran", "Could not create sui reward list, error = "+ pid);
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int handleVeteranMilestoneSelected(obj_id self, dictionary params) throws InterruptedException
	{
		if ((params == null) || (params.isEmpty()))
		{
			veteran_deprecated.cleanupPlayerData(self);
			return SCRIPT_CONTINUE;
		}
		
		int bp = sui.getIntButtonPressed(params);
		if (bp == sui.BP_CANCEL)
		{
			veteran_deprecated.cleanupPlayerData(self);
			return SCRIPT_CONTINUE;
		}
		
		int rowSelected = sui.getListboxSelectedRow(params);
		int[] milestones = self.getScriptVars().getIntArray(veteran_deprecated.SCRIPTVAR_AVAILABLE_MILESTONES);
		if (milestones == null || milestones.length < 1 || rowSelected < 0)
		{
			veteran_deprecated.cleanupPlayerData(self);
			sendSystemMessage(self, veteran_deprecated.SID_BAD_MILESTONE);
			return SCRIPT_CONTINUE;
		}
		
		rowSelected = 1;
		
		for (int i = 0; i < milestones.length; ++i)
		{
			testAbortScript();
			if ((rowSelected + 1) == milestones[i])
			{
				veteran_deprecated.requestVeteranRewards(self, milestones[i]);
				return SCRIPT_CONTINUE;
			}
		}
		
		veteran_deprecated.cleanupPlayerData(self);
		sendSystemMessage(self, veteran_deprecated.SID_BAD_MILESTONE);
		return SCRIPT_CONTINUE;
	}
	
	
	public int handleVeteranRewardSelected(obj_id self, dictionary params) throws InterruptedException
	{
		if ((params == null) || (params.isEmpty()))
		{
			veteran_deprecated.cleanupPlayerData(self);
			return SCRIPT_CONTINUE;
		}
		
		int bp = sui.getIntButtonPressed(params);
		if (bp == sui.BP_CANCEL)
		{
			veteran_deprecated.cleanupPlayerData(self);
			return SCRIPT_CONTINUE;
		}
		
		int rowSelected = sui.getListboxSelectedRow(params);
		if (rowSelected < 0)
		{
			veteran_deprecated.cleanupPlayerData(self);
			sendSystemMessage(self, veteran_deprecated.SID_BAD_REWARD);
			return SCRIPT_CONTINUE;
		}
		
		int result = veteran_deprecated.givePlayerReward(self, rowSelected, true);
		if (result == veteran_deprecated.GIVE_PLAYER_REWARD_SUCCESS)
		{
			veteran_deprecated.cleanupPlayerData(self);
			sendSystemMessage(self, veteran_deprecated.SID_REWARD_GIVEN);
		}
		else if (result == veteran_deprecated.GIVE_PLAYER_REWARD_FAILED)
		{
			veteran_deprecated.cleanupPlayerData(self);
			sendSystemMessage(self, veteran_deprecated.SID_REWARD_ERROR);
		}
		else
		{
			
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int handleVeteranRewardConfirmed(obj_id self, dictionary params) throws InterruptedException
	{
		if ((params == null) || (params.isEmpty()))
		{
			veteran_deprecated.cleanupPlayerData(self);
			return SCRIPT_CONTINUE;
		}
		
		int bp = sui.getIntButtonPressed(params);
		if (bp == sui.BP_CANCEL)
		{
			
			if (!self.getScriptVars().hasKey(veteran_deprecated.SCRIPTVAR_SELECTED_MILESTONE))
			{
				veteran_deprecated.cleanupPlayerData(self);
			}
			else
			{
				self.getScriptVars().remove(veteran_deprecated.SCRIPTVAR_SELECTED_REWARD);
				int milestone = self.getScriptVars().getInt(veteran_deprecated.SCRIPTVAR_SELECTED_MILESTONE);
				veteran_deprecated.requestVeteranRewards(self, milestone);
			}
			
			return SCRIPT_CONTINUE;
		}
		
		if (!self.getScriptVars().hasKey(veteran_deprecated.SCRIPTVAR_SELECTED_REWARD))
		{
			CustomerServiceLog("veteran", "handleVeteranRewardConfirmed could not find SCRIPTVAR_SELECTED_REWARD for player %TU", self);
			veteran_deprecated.cleanupPlayerData(self);
			return SCRIPT_CONTINUE;
		}
		
		int rewardIndex = self.getScriptVars().getInt(veteran_deprecated.SCRIPTVAR_SELECTED_REWARD);
		int result = veteran_deprecated.givePlayerReward(self, rewardIndex, false);
		if (result == veteran_deprecated.GIVE_PLAYER_REWARD_SUCCESS)
		{
			veteran_deprecated.cleanupPlayerData(self);
			sendSystemMessage(self, veteran_deprecated.SID_REWARD_GIVEN);
		}
		else
		{
			veteran_deprecated.cleanupPlayerData(self);
			sendSystemMessage(self, veteran_deprecated.SID_REWARD_ERROR);
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int cmdFlashSpeeder(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		String config = getConfigSetting("GameServer", "flashSpeederReward");
		if (config == null || !config.equals("true"))
		{
			return SCRIPT_CONTINUE;
		}
		
		int sub_bits = getGameFeatureBits(self);
		if (hasObjVar(self, "flash_speeder.eligible"))
		{
			sub_bits = getIntObjVar(self, "flash_speeder.eligible");
		}
		
		if (features.isSpaceEdition(self) && utils.checkBit(sub_bits, 3) || features.isJPCollectorEdition(self))
		{
			if (!hasObjVar(self, "flash_speeder.granted"))
			{
				obj_id inv = getObjectInSlot(self, "inventory");
				int free_space = getVolumeFree(inv);
				if (free_space < 1)
				{
					sendSystemMessage(self, new string_id(veteran_deprecated.VETERAN_STRING_TABLE, "flash_speeder_no_inv_space"));
					return SCRIPT_CONTINUE;
				}
				
				if (veteran_deprecated.checkFlashSpeederReward(self))
				{
					sendSystemMessage(self, new string_id(veteran_deprecated.VETERAN_STRING_TABLE, "flash_speeder_granted"));
				}
				else
				{
					sendSystemMessage(self, new string_id(veteran_deprecated.VETERAN_STRING_TABLE, "flash_speeder_grant_failed"));
				}
			}
			else
			{
				
				sui.msgbox(self, self, "@"+ veteran_deprecated.VETERAN_STRING_TABLE + ":flash_speeder_replace_prompt", sui.YES_NO, "msgFlashSpeederConfirmed");
			}
		}
		else
		{
			sendSystemMessage(self, new string_id(veteran_deprecated.VETERAN_STRING_TABLE, "flash_speeder_not_eligible"));
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int msgFlashSpeederConfirmed(obj_id self, dictionary params) throws InterruptedException
	{
		String button = params.getString("buttonPressed");
		if (button.equals("Cancel"))
		{
			return SCRIPT_CONTINUE;
		}
		
		if (getTotalMoney(self) < veteran_deprecated.FLASH_SPEEDER_COST)
		{
			sendSystemMessage(self, new string_id(veteran_deprecated.VETERAN_STRING_TABLE, "flash_speeder_no_credits"));
			return SCRIPT_CONTINUE;
		}
		
		obj_id inv = getObjectInSlot(self, "inventory");
		int free_space = getVolumeFree(inv);
		if (free_space < 1)
		{
			sendSystemMessage(self, new string_id(veteran_deprecated.VETERAN_STRING_TABLE, "flash_speeder_no_inv_space"));
			return SCRIPT_CONTINUE;
		}
		
		money.pay(self, money.ACCT_VEHICLE_REPAIRS, veteran_deprecated.FLASH_SPEEDER_COST, "msgFlashSpeederPaid", null);
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int msgFlashSpeederPaid(obj_id self, dictionary params) throws InterruptedException
	{
		int result = money.getReturnCode(params);
		if (result == money.RET_SUCCESS)
		{
			createObjectInInventoryAllowOverload("object/tangible/deed/vehicle_deed/speederbike_flash_deed.iff", self);
			CustomerServiceLog("flash_speeder", "%TU has purchased a new JtL pre-order Flash Speeder.", self);
			sendSystemMessage(self, new string_id(veteran_deprecated.VETERAN_STRING_TABLE, "flash_speeder_granted"));
		}
		else
		{
			sendSystemMessage(self, new string_id(veteran_deprecated.VETERAN_STRING_TABLE, "flash_speeder_grant_failed"));
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int playDelayedClientEffect(obj_id self, dictionary params) throws InterruptedException
	{
		String clientEffect = params.getString("effect");
		obj_id target = params.getObjId("target");
		
		playClientEffectObj(target, clientEffect, target, "");
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int clearScriptVar(obj_id self, dictionary params) throws InterruptedException
	{
		String scriptVarName = params.getString("name");
		
		utils.removeScriptVar(self, scriptVarName);
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int OnEnterRegion(obj_id self, String planetName, String regionName) throws InterruptedException
	{
		obj_id pvpRegionController = gcw.getPvpRegionControllerIdByName(self, regionName);
		
		if (isIdValid(pvpRegionController) && exists(pvpRegionController))
		{
			gcw.notifyPvpRegionControllerOfPlayerEnter(pvpRegionController, self);
		}
		
		if (regionName.equals(restuss_event.PVP_REGION_NAME))
		{
			
			if ((!factions.isImperial(self) && !factions.isRebel(self)) || !factions.isCovert(self) || getLevel(self) < 75)
			{
				if (getLevel(self) < 75)
				{
					sendSystemMessage(self, new string_id("gcw", "pvp_advanced_region_level_low"));
				}
				else
				{
					sendSystemMessage(self, new string_id("gcw", "pvp_advanced_region_not_allowed"));
				}
				
				int attempts = 0;
				String enterAttempt = "enterRestussAttempt";
				
				if (utils.hasScriptVar(self, enterAttempt))
				{
					attempts = utils.getIntScriptVar(self, enterAttempt);
				}
				
				if (attempts > 5)
				{
					utils.removeScriptVar(self, enterAttempt);
					warpPlayer(self, "rori", 5305, 80, 6188, null, 0, 0, 0);
					return SCRIPT_CONTINUE;
				}
				
				attempts++;
				
				utils.setScriptVar(self, enterAttempt, attempts);
				return SCRIPT_OVERRIDE;
			}
			
			sendSystemMessage(self, new string_id("gcw", "pvp_advanced_region_entered"));
			
			pvpMakeDeclared(self);
			utils.setScriptVar(self, factions.IN_ADHOC_PVP_AREA, true);
			return SCRIPT_CONTINUE;
		}
		else if (regionName.startsWith(gcw.PVP_BATTLEFIELD_REGION))
		{
			
			obj_id controllerOnPlayer = utils.getObjIdScriptVar(self, "battlefield.active");
			
			if (!isIdValid(controllerOnPlayer))
			{
				
				if (utils.hasObjIdBatchScriptVar(pvpRegionController, "battlefield.active_players"))
				{
					
					obj_id[] allPlayers = utils.getObjIdBatchScriptVar(pvpRegionController, "battlefield.active_players");
					if (allPlayers != null && allPlayers.length > 0)
					{
						boolean found = false;
						
						for (int i = 0; i < allPlayers.length; ++i)
						{
							testAbortScript();
							
							if (allPlayers[i] == self)
							{
								utils.setScriptVar(self, "battlefield.active", pvpRegionController);
								found = true;
								pvp.bfLog(pvpRegionController, "Player "+ getName(self) + "("+self+") Entered the Battlefield and was missing the 'battlefield.active_players' scriptvar. Player was just found and the scriptvar was added.");
								break;
							}
						}
						
						if (!found)
						{
							pvp.bfLog(pvpRegionController, "Player "+ getName(self) + "("+self+") Entered the Battlefield and was missing the 'battlefield.active_players' scriptvar. After reviewing the list of players allowed, they were not found. They need to be removed from the Battlefield");
							
						}
						
					}
				}
			}
			
			sendSystemMessage(self, new string_id("gcw", "pvp_advanced_region_entered"));
			
			pvpMakeDeclared(self);
			utils.setScriptVar(self, factions.IN_ADHOC_PVP_AREA, true);
			
			buff.applyBuff(self, "battlefield_radar_invisibility");
		}
		else if (regionName.startsWith(gcw.PVP_PUSHBACK_REGION))
		{
			
			if (((!factions.isImperial(self) && !factions.isRebel(self)) || getLevel(self) < 75) && !isGod(self))
			{
				if (getLevel(self) < 75)
				{
					sendSystemMessage(self, new string_id("gcw", "pvp_advanced_region_level_low"));
				}
				else
				{
					sendSystemMessage(self, new string_id("gcw", "pvp_advanced_region_not_allowed"));
				}
				
				String enterAttempt = "enterBattlefieldAttempt";
				int attempts = utils.getIntScriptVar(self, enterAttempt);
				
				if (attempts > 5)
				{
					utils.removeScriptVar(self, enterAttempt);
				}
				else
				{
					attempts++;
					
					utils.setScriptVar(self, enterAttempt, attempts);
					
					playMusic(self, "sound/wall_of_mist_barrier_os.snd");
					
					return SCRIPT_OVERRIDE;
				}
			}
			
			obj_id controller = gcw.getPushbackControllerByPlayer(self);
			
			if (isIdValid(controller))
			{
				dictionary params = new dictionary();
				
				params.put("player", self);
				
				messageTo(controller, "validateTeamPlayer", params, 1.0f, false);
			}
			
			return SCRIPT_CONTINUE;
		}
		
		else if (regionName.equals("dathomir_fs_village_unpassable"))
		{
			String config = getConfigSetting("GameServer", "fsWallOfMistEnabled");
			if (config != null)
			{
				if (config.equals("false"))
				{
					return SCRIPT_CONTINUE;
				}
			}
			
			if (!township.isTownshipEligible(self) || ai_lib.isInCombat( self ))
			{
				boolean playEffect = false;
				int gameTime = getGameTime();
				
				if (utils.hasScriptVar(self, "lastTownshipUnpassable"))
				{
					int lastNotice = utils.getIntScriptVar(self, "lastTownshipUnpassable");
					if (gameTime - 30 > lastNotice)
					{
						playEffect = true;
					}
				}
				else
				{
					playEffect = true;
				}
				
				if (playEffect)
				{
					playMusic(self, "sound/wall_of_mist_barrier_os.snd");
					
					string_id sid = null;
					if (! fs_quests.isVillageEligible(self))
					{
						sid = new string_id("base_player", "fs_village_unavailable");
					}
					else if (ai_lib.isInCombat(self))
					{
						sid = new string_id("base_player", "fs_village_no_combat");
					}
					
					if (sid != null)
					{
						sendSystemMessage(self, sid);
					}
					
					utils.setScriptVar(self, "lastFsUnpassable", gameTime);
				}
				
				return SCRIPT_OVERRIDE;
			}
		}
		else if (regionName.equals("@dathomir_region_names:black_mesa") && hasObjVar(self, "outbreak.usedGate"))
		{
			buff.applyBuff(self, "death_troopers_no_vehicle");
		}
		else if (( regionName.equals("outbreak_infected_area_01") || regionName.equals("outbreak_infected_area_02") || regionName.equals("outbreak_infected_area_03") || regionName.equals("outbreak_infected_area_04") ))
		{
			if (!isGod(self))
			{
				if (needsBlackwingLightInfection(self))
				{
					
					messageTo(self, "death_troopers_apply_virus", null, 3.0f, false);
				}
				
				if (!buff.hasBuff(self, "death_troopers_no_vehicle"))
				{
					buff.applyBuff(self, "death_troopers_no_vehicle");
				}
				
				obj_id currentMount = getMountId(self);
				if (isIdValid(currentMount))
				{
					sendSystemMessage(self, new string_id("base_player", "dismounting"));
					utils.dismountRiderJetpackCheck(self);
					if (exists(currentMount))
					{
						obj_id petControlDevice = callable.getCallableCD(currentMount);
						vehicle.storeVehicle(petControlDevice, self);
					}
				}
			}
		}
		else
		{
			region enteredRegion = getRegion(planetName, regionName);
			
			if (enteredRegion != null && enteredRegion.getPvPType() == regions.PVP_REGION_TYPE_ADVANCED)
			{
				if ((!factions.isImperial(self) && !factions.isRebel(self)) || !factions.isCovert(self))
				{
					sendSystemMessage(self, new string_id("gcw", "pvp_advanced_region_not_allowed"));
					return SCRIPT_OVERRIDE;
				}
				
				sendSystemMessage(self, new string_id("gcw", "pvp_advanced_region_entered"));
				
				if (hasObjVar(self, "intChangingFactionStatus"))
				{
					sendSystemMessage(self, new string_id("gcw", "pvp_advanced_region_faction_type_change_cancel"));
				}
				
				pvpMakeDeclared(self);
				utils.setScriptVar(self, factions.IN_ADHOC_PVP_AREA, true);
				return SCRIPT_CONTINUE;
			}
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int OnExitRegion(obj_id self, String planetName, String regionName) throws InterruptedException
	{
		
		if (regionName.startsWith(gcw.PVP_BATTLEFIELD_REGION) && !utils.hasScriptVar(self, "battlefield.kicked_out"))
		{
			sendSystemMessage(self, new string_id("gcw", "pvp_advanced_region_leaving_battlefield"));
			
			if (buff.hasBuff(self, "battlefield_communication_run"))
			{
				return SCRIPT_OVERRIDE;
			}
		}
		
		if (regionName.startsWith(gcw.PVP_BATTLEFIELD_REGION))
		{
			buff.removeBuff(self, "battlefield_radar_invisibility");
			utils.removeScriptVar(self, factions.IN_ADHOC_PVP_AREA);
		}
		
		if (regionName.startsWith(gcw.PVP_PUSHBACK_REGION))
		{
			sendSystemMessage(self, new string_id("gcw", "pvp_advanced_region_leaving_pushback"));
		}
		
		if (utils.hasScriptVar(self, "battlefield.kicked_out"))
		{
			utils.removeScriptVar(self, "battlefield.kicked_out");
		}
		
		if (regionName.equals(restuss_event.PVP_REGION_NAME))
		{
			sendSystemMessage(self, restuss_event.SID_PVP_EXIT_MESSAGE);
			if (utils.hasScriptVar(self, factions.IN_ADHOC_PVP_AREA))
			{
				utils.removeScriptVar(self, factions.IN_ADHOC_PVP_AREA);
			}
			
			return SCRIPT_CONTINUE;
		}
		else if (regionName.equals("@dathomir_region_names:black_mesa"))
		{
			if (isGod(self))
			{
				sendSystemMessage(self, "[GodMode] Now Leaving the Infected Zone. We hope you enjoyed your visit.", "");
			}
			
			obj_id building = getTopMostContainer(self);
			if (isIdValid(building) && getTemplateName(building).equals("object/building/dathomir/imperial_blackwing_facility.iff"))
			{
				
				if (isGod(self))
				{
					sendSystemMessage(self, "[GodMode] Please disregard that previous message as you are clearly still within the Blackwing facility. Be sure to enjoy your infection while you are here.", "");
				}
			}
			else
			{
				buff.removeBuff(self, "death_troopers_infection_1");
				buff.removeBuff(self, "death_troopers_infection_2");
				buff.removeBuff(self, "death_troopers_infection_3");
				buff.removeBuff(self, "death_troopers_no_vehicle");
			}
		}
		else
		{
			region exitedRegion = getRegion(planetName, regionName);
			if (exitedRegion != null && exitedRegion.getPvPType() == regions.PVP_REGION_TYPE_ADVANCED)
			{
				
				sendSystemMessage(self, new string_id("gcw", "pvp_advanced_region_exited"));
				
				if (utils.hasScriptVar(self, factions.IN_ADHOC_PVP_AREA))
				{
					utils.removeScriptVar(self, factions.IN_ADHOC_PVP_AREA);
				}
				
				return SCRIPT_CONTINUE;
			}
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public boolean setupNovicePilotSkill(obj_id self, String skillName) throws InterruptedException
	{
		
		if (!skillName.startsWith("pilot_"))
		{
			return true;
		}
		
		if (skillName.equals("pilot_imperial_navy_novice" ))
		{
			int intAlignedFaction = pvpGetAlignedFaction(self);
			if (intAlignedFaction==(370444368))
			{
				sendSystemMessage( self, new string_id( "faction_recruiter", "no_rebel_imp_pilots"));
				if (!isGod(self))
				{
					return false;
				}
				else
				{
					sendSystemMessageTestingOnly( self, "GODMODE MSG: You can only acquire this skill BECAUSE you are in god mode");
				}
			}
			else
			{
				pvpSetAlignedFaction(self, (-615855020) );
				if (pvpGetType( self ) == PVPTYPE_NEUTRAL)
				{
					if (!factions.isOnLeave(self))
					{
						pvpMakeCovert(self);
					}
				}
				else
				{
					
				}
			}
		}
		else if (skillName.equals("pilot_rebel_navy_novice" ))
		{
			int intAlignedFaction = pvpGetAlignedFaction(self);
			if (intAlignedFaction==(-615855020))
			{
				sendSystemMessage( self, new string_id( "faction_recruiter", "no_imp_rebel_pilots"));
				if (!isGod(self))
				{
					return false;
				}
				else
				{
					sendSystemMessageTestingOnly( self, "GODMODE MSG: You can only acquire this skill BECAUSE you are in god mode");
				}
			}
			else
			{
				pvpSetAlignedFaction(self, (370444368) );
				if (pvpGetType( self ) == PVPTYPE_NEUTRAL)
				{
					if (!factions.isOnLeave(self))
					{
						pvpMakeCovert(self);
					}
				}
				else
				{
					
				}
			}
		}
		if (!hasObjVar( self, space_flags.SPACE_TRACK_FLAG ))
		{
			
			if (skillName.equals("pilot_imperial_navy_novice"))
			{
				space_flags.setSpaceTrack( self, space_flags.IMPERIAL_TATOOINE );
			}
			else if (skillName.equals("pilot_rebel_navy_novice"))
			{
				space_flags.setSpaceTrack( self, space_flags.REBEL_TATOOINE );
			}
			else
			{
				space_flags.setSpaceTrack( self, space_flags.PRIVATEER_TATOOINE );
			}
		}
		return true;
	}
	
	
	public boolean allowedBySpaceExpansion(obj_id self, String skillName) throws InterruptedException
	{
		if (skillName.startsWith("pilot_") || skillName.startsWith("crafting_shipwright_" ))
		{
			if (!features.isSpaceEdition( self ))
			{
				
				sendSystemMessage( self, new string_id( "skl_use", "need_space_expansion") );
				return false;
			}
		}
		return true;
	}
	
	
	public int gmForceCommand(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if ((params == null)||(params.equals("")))
		{
			sendSystemMessageTestingOnly(self, "Incorrect syntax. Syntax is /gmForceCommand <command> <target_object_id> <command parameters>. You can use 0 instead of a valid object_id if needed. Params are optionsl");
			return SCRIPT_CONTINUE;
		}
		if (!isIdValid(target))
		{
			target = getLookAtTarget(self);
		}
		
		if (!isIdValid(target))
		{
			sendSystemMessageTestingOnly(self, "You must target the player you wish to call this command on");
			return SCRIPT_CONTINUE;
			
		}
		String[] strText = split(params, ',');
		if (strText.length < 2)
		{
			sendSystemMessageTestingOnly(self, "Incorrect syntax. Syntax is /gmForceCommand <command> <target_object_id> <command parameters>. You can use 0 instead of a valid object_id if needed. Params are optionsl");
			return SCRIPT_CONTINUE;
		}
		String strCommand = strText[0];
		if (strCommand.equals("gmForceCommand"))
		{
			sendSystemMessageTestingOnly(self, "You are not permitted to use this command to force this command. Don't do that");
			return SCRIPT_CONTINUE;
		}
		
		String strCommandTarget = strText[1];
		String strParams = "";
		
		if (strText.length == 3)
		{
			strParams = strText[2];
		}
		
		Long lngId;
		try
		{
			lngId = new Long(strCommandTarget);
		}
		catch (NumberFormatException err)
		{
			sendSystemMessageTestingOnly(self, strCommandTarget+" is not a valid number to use. Please retry.");
			return SCRIPT_CONTINUE;
		}
		obj_id objCommandTarget = obj_id.getObjId(lngId.longValue());
		if (isIdValid(objCommandTarget)&&(!exists(objCommandTarget)))
		{
			sendSystemMessageTestingOnly(self, "That command target does not exist and it is not 0. Object id was "+objCommandTarget);
			return SCRIPT_CONTINUE;
			
		}
		
		dictionary dctParams = new dictionary();
		dctParams.put("objTarget", objCommandTarget);
		int intCommand = getStringCrc(strCommand);
		dctParams.put("intCommand", intCommand);
		dctParams.put("strParams", strParams);
		messageTo(target, "doForcedGmCommand", dctParams, 0, false);
		sendSystemMessageTestingOnly(self, "Forcing "+target+" to do command "+ strCommand+" to target "+objCommandTarget+" with Parameters "+strParams);
		return SCRIPT_CONTINUE;
	}
	
	
	public int doForcedGmCommand(obj_id self, dictionary params) throws InterruptedException
	{
		int intCommand = params.getInt("intCommand");
		obj_id objTarget = params.getObjId("objTarget");
		String strParams = params.getString("strParams");
		queueCommand(self, intCommand, objTarget, strParams, COMMAND_PRIORITY_DEFAULT);
		sendSystemMessageTestingOnly(self, "A GM Forced you to do a command");
		return SCRIPT_CONTINUE;
	}
	
	
	public int factionBaseLotRefund(obj_id self, dictionary params) throws InterruptedException
	{
		int baseLotRefund = params.getInt( "baseLotRefund");
		obj_id playerObject = getPlayerObject(self);
		
		adjustLotCount( playerObject, (baseLotRefund * -1) );
		
		float baseFactionRefund = params.getFloat("baseFactionRefund");
		int baseFaction = params.getInt("baseFaction");
		factions.addFactionStanding( self, baseFaction, baseFactionRefund );
		
		if ((int)(baseFactionRefund) > 0)
		{
			int intBaseFactionRefund = (int)(baseFactionRefund);
			sendSystemMessageProse(self, prose.getPackage(new string_id ("faction_perk", "old_fbase_refunded"), baseLotRefund, intBaseFactionRefund));
		}
		else
		{
			sendSystemMessageProse(self, prose.getPackage(new string_id ("faction_perk", "new_old_fbase_refunded"), baseLotRefund));
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int transferMoneyToNamedAccount(obj_id self, dictionary params) throws InterruptedException
	{
		transferBankCreditsToNamedAccount(self, params.getString("account"), params.getInt("amount"), "transferMoneyToNamedAccount_successCallback", "transferMoneyToNamedAccount_failCallback", params);
		return SCRIPT_CONTINUE;
	}
	
	
	public int transferMoneyToNamedAccount_successCallback(obj_id self, dictionary params) throws InterruptedException
	{
		messageTo(params.getObjId("replyTo"), params.getString("successCallback"), params, 0, false);
		return SCRIPT_CONTINUE;
	}
	
	
	public int transferMoneyToNamedAccount_failCallback(obj_id self, dictionary params) throws InterruptedException
	{
		messageTo(params.getObjId("replyTo"), params.getString("failCallback"), params, 0, false);
		return SCRIPT_CONTINUE;
	}
	
	
	public int sendSystemMessageProseAuthoritative(obj_id self, dictionary params) throws InterruptedException
	{
		if ((params == null) || (params.isEmpty()))
		{
			debugSpeakMsg (self, "sendSystemMessageProseAuthoritative called with no params");
			return SCRIPT_CONTINUE;
		}
		
		String stf = params.getString("stf");
		String message = params.getString("message");
		obj_id TO = params.getObjId("TO");
		obj_id TT = params.getObjId("TT");
		
		prose_package pp = new prose_package();
		string_id msg = new string_id (stf, message);
		pp = prose.setStringId(pp, msg);
		pp = prose.setTO(pp, TO);
		pp = prose.setTT(pp, TT);
		
		sendSystemMessageProse(self, pp);
		return SCRIPT_CONTINUE;
	}
	
	
	public int OnRecapacitated(obj_id self) throws InterruptedException
	{
		dictionary params = new dictionary();
		params.put("player", self);
		broadcastMessage("handleSawRecapacitation", params);
		
		final int aggroImmuneDuration = 12;
		setAggroImmuneDuration(self, aggroImmuneDuration);
		debugSpeakMsgc(aiLoggingEnabled(self), self, "AggroImmune("+ aggroImmuneDuration + "s)");
		utils.removeScriptVar(self, "incap.timeStamp");
		combat.clearCombatDebuffs(self);
		
		static_item.validateWornEffects(self);
		
		reverse_engineering.checkPowerUpReApply(self);
		
		messageTo(self, "setDisplayOnlyDefensiveMods", trial.getSessionDict(self, "displayDefensiveMods") , 5, false);
		
		if (utils.isProfession(self, utils.SMUGGLER))
		{
			messageTo(self, "applySmugglingBonuses", null, 1.0f, false);
		}
		
		if (utils.isProfession(self, utils.FORCE_SENSITIVE) && getLevel(self) > 3)
		{
			if (!buff.isInStance(self) && !buff.isInFocus(self))
			{
				messageTo(self, "applyJediStance", null, 1.0f, false);
			}
		}
		
		if (factions.isInAdhocPvpArea(self))
		{
			pvpMakeDeclared(self);
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int handleInstanceTimeRemainingMessage(obj_id self, dictionary params) throws InterruptedException
	{
		String message = params.getString("message");
		sendSystemMessageTestingOnly(self, message);
		return SCRIPT_CONTINUE;
	}
	
	
	public int setRespecVersion(obj_id self, dictionary params) throws InterruptedException
	{
		respec.setRespecVersion(self);
		return SCRIPT_CONTINUE;
	}
	
	
	public int receiveCreditForKill(obj_id self, dictionary params) throws InterruptedException
	{
		String creatureName = params.getString("creatureName");
		if (creatureName != null && !creatureName.equals(""))
		{
			location creatureLoc = params.getLocation("location");
			location playerLoc = getLocation(self);
			
			if (!playerLoc.area.equals(creatureLoc.area))
			{
				return SCRIPT_CONTINUE;
			}
			
			String col_faction = params.getString("col_faction");
			int col_difficulty = params.getInt("difficultyClass");
			if (col_faction != null && !col_faction.equals(""))
			{
				
				String[] col_factionArray = split(col_faction, ',');
				for (int i = 0; i < col_factionArray.length; i++)
				{
					testAbortScript();
					if (col_faction.equals("rebel"))
					{
						if (col_difficulty == 0)
						{
							modifyCollectionSlotValue(self, "kill_rebel_01", 1);
						}
						else
						{
							modifyCollectionSlotValue(self, "kill_rebel_boss_01", 1);
						}
						
					}
					if (col_faction.equals("imperial"))
					{
						if (col_difficulty == 0)
						{
							modifyCollectionSlotValue(self, "kill_imperial_01", 1);
						}
						else
						{
							modifyCollectionSlotValue(self, "kill_imperial_boss_01", 1);
						}
					}
				}
			}
			
			if (getDistance( getLocation(self), creatureLoc ) <= xp.MAX_DISTANCE)
			{
				badge.checkForCombatTargetBadge(self, creatureName);
				
				String[] slotNames = getAllCollectionSlotsInCategory("kill_"+ creatureName);
				
				if (slotNames == null || slotNames.length <= 0)
				{
					return SCRIPT_CONTINUE;
				}
				
				for (int i = 0; i < slotNames.length; ++i)
				{
					testAbortScript();
					if (!hasCompletedCollectionSlot(self, slotNames[i]))
					{
						modifyCollectionSlotValue(self, slotNames[i], 1);
					}
				}
			}
			
			obj_id target = params.getObjId("target");
			
			if (isValidId(target))
			{
				if (hasObjVar(target, gcw.GCW_PARTICIPATION_FLAG))
				{
					obj_id parent = trial.getParent(target);
					if (isIdValid(parent))
					{
						trial.addNonInstanceFactionParticipant(self, parent);
					}
				}
			}
		}
		
		if (utils.hasScriptVar(self,"currentBounty") && utils.hasScriptVar(self,"currentBountyValue"))
		{
			obj_id target = params.getObjId("target");
			obj_id storedTarget = utils.getObjIdScriptVar(self,"currentBounty");
			
			if (storedTarget == target)
			{
				int amount = utils.getIntScriptVar(self, "currentBountyValue");
				bounty_hunter.awardBounty(self, creatureName,amount);
			}
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int HandleSetCoverCharge(obj_id self, dictionary params) throws InterruptedException
	{
		int bp = sui.getIntButtonPressed(params);
		
		if (bp == sui.BP_CANCEL)
		{
			return SCRIPT_CONTINUE;
		}
		
		String strAmount = sui.getInputBoxText(params);
		int amount = utils.stringToInt(strAmount);
		
		if (amount > 0)
		{
			performance.covercharge(self, amount);
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int HandleCoverCharge(obj_id self, dictionary params) throws InterruptedException
	{
		int bp = sui.getIntButtonPressed(params);
		obj_id audience = sui.getPlayerId(params);
		int charge = utils.getIntScriptVar(audience, performance.VAR_PERFORM_PAY_WAIT);
		if (bp == 0)
		{
			if (money.hasFunds(audience, money.MT_TOTAL, charge))
			{
				utils.removeScriptVar(audience, performance.VAR_PERFORM_PAY_WAIT);
				utils.setScriptVar(audience, performance.VAR_PERFORM_PAY_AGREE, charge);
				money.pay(audience, money.ACCT_PERFORM_ESCROW, charge, "handlePayment", null);
				
				if (utils.hasScriptVar(audience, "dancerID"))
				{
					obj_id targetDancer = utils.getObjIdScriptVar(audience, "dancerID");
					
					String audienceName = getFirstName(audience);
					if ((audienceName == null) || (audienceName.equals("")))
					{
						audienceName = getPlayerName(audience);
					}
					prose_package dancerProse = new prose_package();
					prose.setTU(dancerProse, audienceName);
					prose.setDI(dancerProse, charge);
					prose.setStringId(dancerProse, COVERCHARGE_DANCER_MESSAGE);
					sendSystemMessageProse(targetDancer, dancerProse);
					
					utils.removeScriptVar(audience, "dancerID");
				}
			}
			else
			{
				sendSystemMessage(audience, performance.SID_CC_NO_FUNDS);
				utils.removeScriptVar(audience, performance.VAR_PERFORM_PAY_WAIT);
				performance.stopListen(audience);
				performance.stopWatch(audience);
				
				if (utils.hasScriptVar(audience, "dancerID"))
				{
					utils.removeScriptVar(audience, "dancerID");
				}
				
			}
		}
		else
		{
			utils.removeScriptVar(audience, performance.VAR_PERFORM_PAY_WAIT);
			utils.setScriptVar(audience, performance.VAR_PERFORM_PAY_DISAGREE, charge);
			performance.stopListen(audience);
			performance.stopWatch(audience);
			
			if (utils.hasScriptVar(audience, "dancerID"))
			{
				utils.removeScriptVar(audience, "dancerID");
			}
		}
		return SCRIPT_CONTINUE;
	}
	
	
	public int messageUpdateMissionCriticalObjects(obj_id self, dictionary params) throws InterruptedException
	{
		obj_id[] addObjects = params.getObjIdArray("addObjects");
		obj_id[] removeObjects = params.getObjIdArray("removeObjects");
		
		if (addObjects != null && addObjects.length > 0)
		{
			for (int i=0; i<addObjects.length; ++i)
			{
				testAbortScript();
				addMissionCriticalObject(self, addObjects[i]);
			}
		}
		
		if (removeObjects != null && removeObjects.length > 0)
		{
			for (int i=0; i<removeObjects.length; ++i)
			{
				testAbortScript();
				removeMissionCriticalObject(self, removeObjects[i]);
			}
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int messageSendQuestSystemMessage(obj_id self, dictionary params) throws InterruptedException
	{
		string_id sid = params.getStringId("sid");
		string_id to = params.getStringId("to");
		
		prose_package pp = prose.getPackage(sid, to);
		
		sendQuestSystemMessage(self, pp);
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int cmdShowLockout(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		space_dungeon.displayAllDungeonLockoutTimers(self);
		return SCRIPT_CONTINUE;
	}
	
	public int OnPvpTypeChanged(obj_id self, int oldType, int newType) throws InterruptedException
	{
		if (oldType == PVPTYPE_DECLARED)
		{
			if ((newType == PVPTYPE_COVERT) || (newType == PVPTYPE_NEUTRAL))
			{
				obj_id playerCurrentMount = getMountId (self);
				if (isIdValid( playerCurrentMount ))
				{
					if (group.isGrouped(self))
					{
						obj_id gid = getGroupObject(self);
						obj_id[] members = getGroupMemberIds(gid);
						for (int i = 0; i < members.length; i++)
						{
							testAbortScript();
							obj_id member = members[i];
							if (isIdValid(member) && member.isLoaded() && isPlayer(member))
							{
								obj_id groupMemberCurrentMount = getMountId (member);
								if (isIdValid( groupMemberCurrentMount ))
								{
									if (groupMemberCurrentMount == playerCurrentMount)
									{
										if (factions.isDeclared(member))
										{
											
										}
									}
								}
							}
						}
					}
				}
			}
		}
		return SCRIPT_CONTINUE;
	}
	
	
	public int OnQuestActivated(obj_id self, int questCrc) throws InterruptedException
	{
		if (groundquests.isQuestVisible(questCrc))
		{
			String questCategory = groundquests.getQuestStringDataEntry(questCrc, groundquests.datatableColumnCategory);
			String questTitle = groundquests.getQuestStringDataEntry(questCrc, groundquests.datatableColumnjournalEntryTitle);
			
			prose_package pp = prose.getPackage(groundquests.SID_QUEST_RECEIVED);
			prose.setTT(pp, questCategory);
			prose.setTO(pp, questTitle);
			sendQuestSystemMessage(self, pp);
			
			play2dNonLoopingSound(self, groundquests.MUSIC_QUEST_ACTIVATED);
		}
		return SCRIPT_CONTINUE;
	}
	
	
	public int OnQuestReceivedReward(obj_id self, int questCrc, String exclusiveItemChoice) throws InterruptedException
	{
		
		int questLevel = groundquests.getQuestIntDataEntry (questCrc, groundquests.dataTableColumnLevel);
		int questTier = groundquests.getQuestIntDataEntry (questCrc, groundquests.dataTableColumnTier);
		String experienceType = groundquests.getQuestStringDataEntry(questCrc, groundquests.dataTableColumnQuestRewardExperienceType);
		int experienceAmount = groundquests.getQuestIntDataEntry (questCrc, groundquests.dataTableColumnQuestRewardExperienceAmount);
		String factionName = groundquests.getQuestStringDataEntry(questCrc, groundquests.dataTableColumnQuestRewardFactionName);
		int factionAmount = groundquests.getQuestIntDataEntry (questCrc, groundquests.dataTableColumnQuestRewardFactionAmount);
		boolean grantGcwReward = groundquests.getQuestBoolDataEntry (questCrc, groundquests.dataTableColumnGcwGrant, false);
		int grantGcwOverwriteAmt = groundquests.getQuestIntDataEntry (questCrc, groundquests.dataTableColumnGcwOverwriteAmt);
		int grantGcwSFModifier = groundquests.getQuestIntDataEntry (questCrc, groundquests.dataTableColumnGcwOvrwrtSFMod);
		String grantGcwRebReward = groundquests.getQuestStringDataEntry(questCrc, groundquests.dataTableColumnGcwRebReward);
		int grantGcwRebRewardCount = groundquests.getQuestIntDataEntry (questCrc, groundquests.dataTableColumnGcwRebRewardCnt);
		String grantGcwImpReward = groundquests.getQuestStringDataEntry(questCrc, groundquests.dataTableColumnGcwImpReward);
		int grantGcwImpRewardCount = groundquests.getQuestIntDataEntry (questCrc, groundquests.dataTableColumnGcwImpRewardCnt);
		int grantGcwSFRewardMultip = groundquests.getQuestIntDataEntry (questCrc, groundquests.dataTableColumnGcwRewardMultip);
		int bankCredits = groundquests.getQuestIntDataEntry (questCrc, groundquests.dataTableColumnQuestRewardBankCredits);
		String item = groundquests.getQuestStringDataEntry(questCrc, groundquests.dataTableColumnQuestRewardItem);
		int itemCount = groundquests.getQuestIntDataEntry (questCrc, groundquests.dataTableColumnQuestRewardCount);
		String weapon = groundquests.getQuestStringDataEntry(questCrc, groundquests.dataTableColumnQuestRewardWeapon);
		int weaponCount = groundquests.getQuestIntDataEntry (questCrc, groundquests.dataTableColumnQuestRewardCountWeapon);
		float weaponSpeed = utils.stringToFloat(groundquests.getQuestStringDataEntry (questCrc, groundquests.dataTableColumnQuestRewardSpeed));
		float weaponDamage = utils.stringToFloat(groundquests.getQuestStringDataEntry (questCrc, groundquests.dataTableColumnQuestRewardDamage));
		float weaponEfficiency = utils.stringToFloat(groundquests.getQuestStringDataEntry (questCrc, groundquests.dataTableColumnQuestRewardEfficiency));
		float weaponElementalValue = utils.stringToFloat(groundquests.getQuestStringDataEntry (questCrc, groundquests.dataTableColumnQuestRewardElementalValue));
		String armor = groundquests.getQuestStringDataEntry(questCrc, groundquests.dataTableColumnQuestRewardArmor);
		int armorCount = groundquests.getQuestIntDataEntry (questCrc, groundquests.dataTableColumnQuestRewardCountArmor);
		int armorQuality = groundquests.getQuestIntDataEntry (questCrc, groundquests.dataTableColumnQuestRewardQuality);
		
		String[] inclusiveLootNames = new String[3];
		int[] inclusiveLootCounts = new int[3];
		inclusiveLootNames[0] = groundquests.getQuestStringDataEntry(questCrc, groundquests.dataTableColumnQuestRewardLootName);
		inclusiveLootCounts[0] = groundquests.getQuestIntDataEntry (questCrc, groundquests.dataTableColumnQuestRewardLootCount);
		inclusiveLootNames[1] = groundquests.getQuestStringDataEntry(questCrc, groundquests.dataTableColumnQuestRewardLootName2);
		inclusiveLootCounts[1] = groundquests.getQuestIntDataEntry (questCrc, groundquests.dataTableColumnQuestRewardLootCount2);
		inclusiveLootNames[2] = groundquests.getQuestStringDataEntry(questCrc, groundquests.dataTableColumnQuestRewardLootName3);
		inclusiveLootCounts[2] = groundquests.getQuestIntDataEntry (questCrc, groundquests.dataTableColumnQuestRewardLootCount3);
		
		String[] exclusiveLootNames = new String[10];
		int[] exclusiveLootCounts = new int[10];
		exclusiveLootNames[0] = groundquests.getQuestStringDataEntry(questCrc, groundquests.dataTableColumnQuestRewardExclusiveLootName);
		exclusiveLootCounts[0] = groundquests.getQuestIntDataEntry (questCrc, groundquests.dataTableColumnQuestRewardExclusiveLootCount);
		exclusiveLootNames[1] = groundquests.getQuestStringDataEntry(questCrc, groundquests.dataTableColumnQuestRewardExclusiveLootName2);
		exclusiveLootCounts[1] = groundquests.getQuestIntDataEntry (questCrc, groundquests.dataTableColumnQuestRewardExclusiveLootCount2);
		exclusiveLootNames[2] = groundquests.getQuestStringDataEntry(questCrc, groundquests.dataTableColumnQuestRewardExclusiveLootName3);
		exclusiveLootCounts[2] = groundquests.getQuestIntDataEntry (questCrc, groundquests.dataTableColumnQuestRewardExclusiveLootCount3);
		exclusiveLootNames[3] = groundquests.getQuestStringDataEntry(questCrc, groundquests.dataTableColumnQuestRewardExclusiveLootName4);
		exclusiveLootCounts[3] = groundquests.getQuestIntDataEntry (questCrc, groundquests.dataTableColumnQuestRewardExclusiveLootCount4);
		exclusiveLootNames[4] = groundquests.getQuestStringDataEntry(questCrc, groundquests.dataTableColumnQuestRewardExclusiveLootName5);
		exclusiveLootCounts[4] = groundquests.getQuestIntDataEntry (questCrc, groundquests.dataTableColumnQuestRewardExclusiveLootCount5);
		exclusiveLootNames[5] = groundquests.getQuestStringDataEntry(questCrc, groundquests.dataTableColumnQuestRewardExclusiveLootName6);
		exclusiveLootCounts[5] = groundquests.getQuestIntDataEntry (questCrc, groundquests.dataTableColumnQuestRewardExclusiveLootCount6);
		exclusiveLootNames[6] = groundquests.getQuestStringDataEntry(questCrc, groundquests.dataTableColumnQuestRewardExclusiveLootName7);
		exclusiveLootCounts[6] = groundquests.getQuestIntDataEntry (questCrc, groundquests.dataTableColumnQuestRewardExclusiveLootCount7);
		exclusiveLootNames[7] = groundquests.getQuestStringDataEntry(questCrc, groundquests.dataTableColumnQuestRewardExclusiveLootName8);
		exclusiveLootCounts[7] = groundquests.getQuestIntDataEntry (questCrc, groundquests.dataTableColumnQuestRewardExclusiveLootCount8);
		exclusiveLootNames[8] = groundquests.getQuestStringDataEntry(questCrc, groundquests.dataTableColumnQuestRewardExclusiveLootName9);
		exclusiveLootCounts[8] = groundquests.getQuestIntDataEntry (questCrc, groundquests.dataTableColumnQuestRewardExclusiveLootCount9);
		exclusiveLootNames[9] = groundquests.getQuestStringDataEntry(questCrc, groundquests.dataTableColumnQuestRewardExclusiveLootName10);
		exclusiveLootCounts[9] = groundquests.getQuestIntDataEntry (questCrc, groundquests.dataTableColumnQuestRewardExclusiveLootCount10);
		
		String badge = groundquests.getQuestStringDataEntry(questCrc, groundquests.dataTableColumnBadge);
		
		int exclusiveLootCountChoice = 1;
		
		groundquests.grantQuestReward(self, questCrc, questLevel, questTier, experienceType, experienceAmount, factionName, factionAmount, grantGcwReward, bankCredits, item, itemCount, weapon, weaponCount, weaponSpeed, weaponDamage, weaponEfficiency, weaponElementalValue, armor, armorCount, armorQuality, inclusiveLootNames, inclusiveLootCounts, exclusiveItemChoice, exclusiveLootCountChoice, badge, (questIsQuestForceAccept(questCrc) || !questDoesUseAcceptanceUI(questCrc)), grantGcwOverwriteAmt, grantGcwSFModifier, grantGcwRebReward, grantGcwRebRewardCount, grantGcwImpReward, grantGcwImpRewardCount, grantGcwSFRewardMultip);
		
		experienceAmount = groundquests.getQuestExperienceReward(self, questLevel, questTier, experienceAmount);
		metrics.doQuestMetrics(self, questCrc, questLevel, questTier, experienceType, experienceAmount);
		return SCRIPT_CONTINUE;
	}
	
	
	public int OnQuestCompleted(obj_id self, int questCrc) throws InterruptedException
	{
		
		String conditionalQuestToGrant = groundquests.getQuestStringDataEntry(questCrc, groundquests.dataTableColumnConditionQuestGrantQuest);
		String listOfQuestsThatMustBeCompleted = groundquests.getQuestStringDataEntry(questCrc, groundquests.dataTableColumnConditionQuestGrantListOfCompletedQuests);
		
		if (conditionalQuestToGrant != null && conditionalQuestToGrant.length() > 0 && listOfQuestsThatMustBeCompleted != null && listOfQuestsThatMustBeCompleted.length() > 0)
		{
			
			String[] completedQuestList = split(listOfQuestsThatMustBeCompleted, ',');
			
			boolean grantQuest = completedQuestList.length > 0;
			
			for (int i = 0; i < completedQuestList.length && grantQuest; ++i)
			{
				testAbortScript();
				String quest = completedQuestList[i];
				if (!groundquests.isValidQuestName(quest) || !groundquests.hasCompletedQuest(self, quest))
				{
					grantQuest = false;
				}
			}
			
			if (groundquests.isValidQuestName(conditionalQuestToGrant) && grantQuest)
			{
				groundquests.requestGrantQuest(self, conditionalQuestToGrant);
			}
		}
		
		if (groundquests.isQuestVisible(questCrc))
		{
			play2dNonLoopingSound(self, groundquests.MUSIC_QUEST_COMPLETED);
		}
		
		String questString = questGetQuestName(questCrc);
		int tableRow = dataTableSearchColumnForString(questString, "questName", collection.QUEST__COMPLETED_COLLECTIONS);
		
		if (tableRow > -1)
		{
			collection.grantQuestBasedCollections(questString, self);
		}
		
		smuggler.removeFromBountyTerminal(self, questCrc, false);
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int OnQuestCleared(obj_id self, int questCrc) throws InterruptedException
	{
		String factionName = groundquests.getQuestStringDataEntry(questCrc, groundquests.dataTableColumnQuestPenaltyFactionName);
		int factionAmount = groundquests.getQuestIntDataEntry(questCrc, groundquests.dataTableColumnQuestPenaltyFactionAmount);
		
		LOG("QUEST", "Quest "+ questCrc + " cleared!");
		
		groundquests.applyQuestPenalty(self, factionName, factionAmount);
		smuggler.removeFromBountyTerminal(self, questCrc, true);
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int OnRequestStaticItemData(obj_id self, String itemName) throws InterruptedException
	{
		dictionary itemData = static_item.getMergedItemDictionary(itemName);
		
		if (itemData != null)
		{
			String[] keysStringVector = itemData.keysStringVector();
			String[] valuesStringVector = itemData.valuesStringVector();
			sendStaticItemDataToPlayer(self, keysStringVector, valuesStringVector);
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int handleClientLogin(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		int[] quests = questGetAllActiveQuestIds(self);
		for (int i = 0; i < quests.length; ++i)
		{
			testAbortScript();
			int questCrc = quests[i];
			
			Vector objectsToTellClientAbout = new Vector();
			objectsToTellClientAbout.setSize(0);
			String item = groundquests.getQuestStringDataEntry(questCrc, groundquests.dataTableColumnQuestRewardLootName);
			if (item != null)
			{
				utils.addElement(objectsToTellClientAbout, item);
			}
			item = groundquests.getQuestStringDataEntry(questCrc, groundquests.dataTableColumnQuestRewardLootName2);
			if (item != null)
			{
				utils.addElement(objectsToTellClientAbout, item);
			}
			item = groundquests.getQuestStringDataEntry(questCrc, groundquests.dataTableColumnQuestRewardLootName3);
			if (item != null)
			{
				utils.addElement(objectsToTellClientAbout, item);
			}
			item = groundquests.getQuestStringDataEntry(questCrc, groundquests.dataTableColumnQuestRewardExclusiveLootName);
			if (item != null)
			{
				utils.addElement(objectsToTellClientAbout, item);
			}
			item = groundquests.getQuestStringDataEntry(questCrc, groundquests.dataTableColumnQuestRewardExclusiveLootName2);
			if (item != null)
			{
				utils.addElement(objectsToTellClientAbout, item);
			}
			item = groundquests.getQuestStringDataEntry(questCrc, groundquests.dataTableColumnQuestRewardExclusiveLootName3);
			if (item != null)
			{
				utils.addElement(objectsToTellClientAbout, item);
			}
			item = groundquests.getQuestStringDataEntry(questCrc, groundquests.dataTableColumnQuestRewardExclusiveLootName4);
			if (item != null)
			{
				utils.addElement(objectsToTellClientAbout, item);
			}
			item = groundquests.getQuestStringDataEntry(questCrc, groundquests.dataTableColumnQuestRewardExclusiveLootName5);
			if (item != null)
			{
				utils.addElement(objectsToTellClientAbout, item);
			}
			item = groundquests.getQuestStringDataEntry(questCrc, groundquests.dataTableColumnQuestRewardExclusiveLootName6);
			if (item != null)
			{
				utils.addElement(objectsToTellClientAbout, item);
			}
			item = groundquests.getQuestStringDataEntry(questCrc, groundquests.dataTableColumnQuestRewardExclusiveLootName7);
			if (item != null)
			{
				utils.addElement(objectsToTellClientAbout, item);
			}
			item = groundquests.getQuestStringDataEntry(questCrc, groundquests.dataTableColumnQuestRewardExclusiveLootName8);
			if (item != null)
			{
				utils.addElement(objectsToTellClientAbout, item);
			}
			item = groundquests.getQuestStringDataEntry(questCrc, groundquests.dataTableColumnQuestRewardExclusiveLootName9);
			if (item != null)
			{
				utils.addElement(objectsToTellClientAbout, item);
			}
			item = groundquests.getQuestStringDataEntry(questCrc, groundquests.dataTableColumnQuestRewardExclusiveLootName10);
			if (item != null)
			{
				utils.addElement(objectsToTellClientAbout, item);
			}
			
			for (int j = 0; j < objectsToTellClientAbout.size(); ++j)
			{
				testAbortScript();
				String itemName = ((String)(objectsToTellClientAbout.get(j)));
				if (itemName != null && !itemName.equals(""))
				{
					dictionary itemData = static_item.getMergedItemDictionary(itemName);
					if (itemData != null)
					{
						String[] keysStringVector = itemData.keysStringVector();
						String[] valuesStringVector = itemData.valuesStringVector();
						sendStaticItemDataToPlayer(self, keysStringVector, valuesStringVector);
					}
				}
			}
		}
		
		obj_id building = getTopMostContainer(self);
		if (isIdValid(building) && getTemplateName(building).equals("object/building/dathomir/imperial_blackwing_facility.iff"))
		{
			if (needsBlackwingLightInfection(self))
			{
				buff.applyBuff(self, "death_troopers_infection_1");
			}
		}
		return SCRIPT_CONTINUE;
	}
	
	
	public int OnGetStaticItemsAttributes(obj_id self, String staticItemName, String[] names, String[] attribs) throws InterruptedException
	{
		if ((names == null) || (attribs == null) || (names.length != attribs.length))
		{
			return SCRIPT_CONTINUE;
		}
		static_item.getAttributes(self, staticItemName, names, attribs);
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int handleSmugglerFenceAbility(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		smuggler.showSellJunkSui(self, self, true, true);
		return SCRIPT_CONTINUE;
	}
	
	
	public int handleSellJunkSui(obj_id self, dictionary params) throws InterruptedException
	{
		obj_id player = sui.getPlayerId(params);
		
		if (!isIdValid(player))
		{
			return SCRIPT_CONTINUE;
		}
		
		int idx = sui.getListboxSelectedRow(params);
		int bp = sui.getIntButtonPressed(params);
		
		obj_id[] junk = utils.getObjIdBatchScriptVar(player, smuggler.SCRIPTVAR_JUNK_IDS);
		
		boolean reshowSui = true;
		if (junk.length < 1)
		{
			reshowSui = false;
		}
		
		boolean fence = utils.getBooleanScriptVar(self, "fence");
		
		smuggler.cleanupSellJunkSui(player);
		
		if (bp == sui.BP_CANCEL)
		{
			return SCRIPT_CONTINUE;
		}
		else if (bp == sui.BP_REVERT)
		{
			if (idx >= 0 && idx <= junk.length - 1)
			{
				if ((junk != null) || (junk.length > 0))
				{
					openExamineWindow(player, junk[idx]);
				}
			}
			
			if (fence)
			{
				smuggler.showSellJunkSui(player, self, true, false);
			}
			else
			{
				smuggler.showSellJunkSui(player, self, false, false);
			}
			return SCRIPT_CONTINUE;
		}
		else
		{
			
			if (idx < 0)
			{
				if (reshowSui)
				{
					smuggler.showSellJunkSui(self, self, true, true);
				}
				
				return SCRIPT_CONTINUE;
			}
			
			if ((junk == null) || (junk.length == 0))
			{
				return SCRIPT_CONTINUE;
			}
			
			if (idx > junk.length - 1)
			{
				if (reshowSui)
				{
					smuggler.showSellJunkSui(self, self, true, true);
				}
				
				return SCRIPT_CONTINUE;
			}
			
			smuggler.sellJunkItem(player, junk[idx], fence, reshowSui);
		}
		return SCRIPT_CONTINUE;
	}
	
	
	public int handleSoldJunk(obj_id self, dictionary params) throws InterruptedException
	{
		blog("base_player.handleSoldJunk() - init");
		
		if (params == null || params.isEmpty())
		{
			return SCRIPT_CONTINUE;
		}
		
		obj_id player = params.getObjId(money.DICT_TARGET_ID);
		if (!isIdValid(player))
		{
			CustomerServiceLog("Junk_Dealer: ", "junk_dealer.handleSoldJunk() - Player: "+self+" cannot sell an item because the junk dealer failed to send a valid obj_id.");
			return SCRIPT_CONTINUE;
		}
		
		obj_id item = params.getObjId("item");
		if (!isIdValid(item))
		{
			CustomerServiceLog("Junk_Dealer: ", "junk_dealer.handleSoldJunk() - Player: "+self+" cannot sell an item because the item was found to be invalid.");
			return SCRIPT_CONTINUE;
		}
		
		boolean fence = params.getBoolean("fence");
		blog("base_player.handleSoldJunk() - fence: "+fence);
		
		int retCode = params.getInt(money.DICT_CODE);
		
		if (retCode == money.RET_FAIL)
		{
			prose_package ppNoSale = prose.getPackage(smuggler.PROSE_NO_SALE, self, item);
			if (fence)
			{
				ppNoSale = prose.getPackage(smuggler.PROSE_NO_SALE_FENCE, self, item);
			}
			sendSystemMessageProse(player, ppNoSale);
			
			return SCRIPT_CONTINUE;
		}
		
		int price = params.getInt("price");
		
		prose_package ppSoldJunk = prose.getPackage(smuggler.PROSE_SOLD_JUNK, item, price);
		if (fence)
		{
			ppSoldJunk = prose.getPackage(smuggler.PROSE_SOLD_JUNK_FENCE, item, price);
		}
		sendSystemMessageProse(player, ppSoldJunk);
		
		setObjVar(item, smuggler.BUYBACK_OBJ_SOLD, getGameTime());
		smuggler.moveBuyBackObjectIntoContainer(player, item, price);
		
		CustomerServiceLog("Junk_Dealer: ", "junk_dealer.handleSoldJunk() - Player: "+self+" has sold item: "+item+" and the item has been marked sold. The item is about to be moved to the buy back container.");
		
		boolean reshowSui = params.getBoolean("reshowSui");
		
		if (reshowSui && fence)
		{
			smuggler.showSellJunkSui(player, self, true, false);
		}
		else if (reshowSui && !fence)
		{
			smuggler.showSellJunkSui(player, self, false, false);
		}
		return SCRIPT_CONTINUE;
	}
	
	
	public int OnSkillModsChanged(obj_id self, String[] modNames, int[] modValues) throws InterruptedException
	{
		
		trial.bumpSession(self, "displayDefensiveMods");
		messageTo(self, "setDisplayOnlyDefensiveMods", trial.getSessionDict(self, "displayDefensiveMods") , 5, false);
		
		for (int intI = 0; intI < modNames.length; intI++)
		{
			testAbortScript();
			if ((modNames[intI].startsWith("constitution"))||(modNames[intI].startsWith("stamina")))
			{
				skill.recalcPlayerPools(self, false);
				return SCRIPT_CONTINUE;
			}
			
			if (modNames[intI].startsWith("movement_resist"))
			{
				messageTo(self, "check_movement_immunity", null, 0.1f, false);
			}
		}
		return SCRIPT_CONTINUE;
	}
	
	
	public int handlePlayerBountyEngagement(obj_id self, dictionary params) throws InterruptedException
	{
		obj_id target = params.getObjId("target");
		
		pvpSetPermanentPersonalEnemyFlag(self, target);
		
		messageTo(self, "handlePlayerBountyTimeout", params, bounty_hunter.BOUNTY_COLLECT_TIME_LIMIT, true);
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int handlePlayerBountyTimeout(obj_id self, dictionary params) throws InterruptedException
	{
		obj_id target = params.getObjId("target");
		int id = params.getInt("id");
		
		if (utils.hasScriptVar(self, "bounty.id"))
		{
			int bounty_id = utils.getIntScriptVar(self, "bounty.id");
			
			if (bounty_id != id)
			{
				return SCRIPT_CONTINUE;
			}
		}
		
		if (pvpHasPersonalEnemyFlag(self, target))
		{
			pvpRemovePersonalEnemyFlags(self, target);
			sendSystemMessage(self, new string_id("bounty_hunter", "time_expired"));
		}
		
		utils.removeScriptVarTree(self, "bounty");
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int handleAwardedBountyCheck(obj_id self, dictionary params) throws InterruptedException
	{
		
		if (utils.hasScriptVar(self,"currentBounty"))
		{
			utils.removeScriptVar(self,"currentBounty");
		}
		if (utils.hasScriptVar(self,"currentBountyValue"))
		{
			utils.removeScriptVar(self,"currentBountyValue");
		}
		
		if (params == null || params.isEmpty())
		{
			return SCRIPT_CONTINUE;
		}
		
		obj_id player = params.getObjId(money.DICT_TARGET_ID);
		if (!isIdValid(player))
		{
			return SCRIPT_CONTINUE;
		}
		
		int retCode = params.getInt(money.DICT_CODE);
		
		if (retCode == money.RET_FAIL)
		{
			
			prose_package pp = new prose_package ();
			pp.stringId = new string_id ("bounty_hunter", "no_reward_possible");
			sendSystemMessageProse( player, pp );
			return SCRIPT_CONTINUE;
		}
		
		int amount = params.getInt("amount");
		String creatureName = params.getString("creatureName");
		
		prose_package pp = new prose_package ();
		pp.stringId = new string_id ("bounty_hunter", "reward");
		pp.digitInteger = amount;
		pp.other.set (creatureName);
		sendSystemMessageProse( player, pp );
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int handleAwardedPlayerBounty(obj_id self, dictionary params) throws InterruptedException
	{
		obj_id target = params.getObjId("target");
		int amount = params.getInt("bounty");
		
		int retCode = params.getInt(money.DICT_CODE);
		if (retCode == money.RET_FAIL)
		{
			sendSystemMessage(self, new string_id("bounty_hunter", "player_bounty_reward_fail"));
			CustomerServiceLog("bounty", "%TU has defeated %TT but an error occured during the bounty ("+ amount + " credits) payout!", self, target);
			return SCRIPT_CONTINUE;
		}
		
		CustomerServiceLog("bounty", "%TU has defeated %TT and collected a bounty worth "+ amount + " credits", self, target);
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int handleSurveyToolbarSetup(obj_id self, dictionary params) throws InterruptedException
	{
		obj_id objInv = utils.getInventoryContainer(self);
		createObject("object/tangible/survey_tool/survey_tool_mineral.iff", objInv, "");
		newbieTutorialSetToolbarElement(self, 10, "/survey");
		
		return SCRIPT_CONTINUE;
		
	}
	
	
	public int factionBaseUnitRefund(obj_id self, dictionary params) throws InterruptedException
	{
		
		debugServerConsoleMsg( null, "+++ base_player.factionBaseUnitRefund +++ entered message handler");
		
		if (hasObjVar(self,"factionBaseCount" ))
		{
			
			int factionBaseCount = getIntObjVar(self,"factionBaseCount");
			int updatedFactionBaseCount = factionBaseCount-1;
			
			if (updatedFactionBaseCount > 0)
			{
				setObjVar(self,"factionBaseCount",updatedFactionBaseCount);
			}
			else
			{
				removeObjVar(self,"factionBaseCount");
			}
			
			if (updatedFactionBaseCount == player_structure.MAX_BASE_COUNT -1)
			{
				sendSystemMessage(self, SID_CAN_PLACE_ONE_BASE);
			}
			else
			{
				sendSystemMessageProse(self, prose.getPackage(new string_id ("faction_perk", "faction_base_unit_refunded"), (player_structure.MAX_BASE_COUNT-updatedFactionBaseCount)));
			}
			
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int OnSkillTemplateChanged(obj_id self, String skillTemplateName, boolean clientSelected) throws InterruptedException
	{
		if (skillTemplateName == null || skillTemplateName.equals("") || skillTemplateName.equals("a"))
		{
			if (hasObjVar(self, "clickRespec.tokenId"))
			{
				removeObjVar(self, "clickRespec.tokenId");
			}
			
			if (hasObjVar(self, "npcRespec.inProgress"))
			{
				removeObjVar(self, "npcRespec.inProgress");
			}
			
			return SCRIPT_CONTINUE;
		}
		
		if (!hasObjVar( self, "clickRespec") && !hasObjVar(self, "npcRespec.inProgress"))
		{
			return SCRIPT_CONTINUE;
		}
		
		if (hasObjVar(self, "npcRespec.inProgress"))
		{
			respec.handleNpcRespec(self, skillTemplateName);
			
			respec.setRespecVersion(self);
			
			return SCRIPT_CONTINUE;
		}
		else if (hasObjVar(self, "clickRespec.tokenId"))
		{
			obj_id token = getObjIdObjVar(self, "clickRespec.tokenId");
			
			if (hasObjVar(self,"clickRespec.oldTemplate"))
			{
				String oldTemplate = getStringObjVar(self, "clickRespec.oldTemplate");
				
				if (oldTemplate.equals(getSkillTemplate(self)))
				{
					removeObjVar( self, "clickRespec");
					detachScript(self, "systems.respec.click_combat_respec");
					return SCRIPT_CONTINUE;
				}
				
				static_item.decrementStaticItem(token);
				
				removeObjVar( self, "clickRespec.tokenId");
			}
			
		}
		
		boolean withItems = true;
		if (hasObjVar(self, "clickRespec.cts"))
		{
			withItems = false;
		}
		
		respec.earnProfessionSkills(self, skillTemplateName, withItems);
		
		return SCRIPT_CONTINUE;
	}
	
	
	public void givePublishGift(obj_id self) throws InterruptedException
	{
		final String OBJVAR_PUBLISH_GIFT = "publish_gift";
		final String GIFT_DATATABLE = "datatables/veteran_rewards/publish_gift.iff";
		
		int currentBD = getCurrentBirthDate();
		int playerBD = getPlayerBirthDate(self);
		
		if (playerBD < 0)
		{
			return;
		}
		
		if ((currentBD - playerBD) < 10)
		{
			return;
		}
		
		if (isInTutorialArea(self))
		{
			return;
		}
		
		int old_publish = -1;
		if (hasObjVar(self, OBJVAR_PUBLISH_GIFT))
		{
			old_publish = getIntObjVar(self, OBJVAR_PUBLISH_GIFT);
		}
		
		int[] publish_numbers = dataTableGetIntColumn(GIFT_DATATABLE, "PUBLISH");
		if (publish_numbers == null || publish_numbers.length == 0)
		{
			return;
		}
		
		int last_publish = -1;
		Vector gifts = new Vector();
		
		for (int i=0; i < publish_numbers.length; i++)
		{
			testAbortScript();
			if (publish_numbers[i] > last_publish)
			{
				last_publish = publish_numbers[i];
				
				gifts = new Vector();
				gifts.addElement(dataTableGetString(GIFT_DATATABLE, i, "ITEM"));
			}
			else if (publish_numbers[i] == last_publish)
			{
				gifts.addElement(dataTableGetString(GIFT_DATATABLE, i, "ITEM"));
			}
		}
		
		if (gifts == null || gifts.size() == 0)
		{
			return;
		}
		
		if (old_publish >= last_publish)
		{
			return;
		}
		
		obj_id inv = utils.getInventoryContainer(self);
		Vector objects = new Vector();
		
		boolean noGift = false;
		
		for (int i=0; i < gifts.size(); i++)
		{
			testAbortScript();
			String item = (String)gifts.get(i);
			
			if (item.equals("none"))
			{
				noGift = true;
				continue;
			}
			
			obj_id new_gift = static_item.createNewItemFunction(item, inv);
			
			if (isIdValid(new_gift))
			{
				objects.addElement(new_gift);
				CustomerServiceLog("grantGift", "%TU has received the gift(s) for Publish "+ last_publish + ". Item given was "+ item + ". Item OID was "+ new_gift, self);
			}
		}
		
		obj_id[] object_array = utils.toStaticObjIdArray(objects);
		
		if ((object_array == null || object_array.length == 0))
		{
			if (noGift)
			{
				setObjVar(self, OBJVAR_PUBLISH_GIFT, last_publish);
			}
			
			return;
		}
		
		sendSystemMessage(self, new string_id("base_player", "received_gift"));
		
		setObjVar(self, OBJVAR_PUBLISH_GIFT, last_publish);
		
		showLootBox(self, object_array);
		
	}
	
	
	public void respecNewEntertainerSkills(obj_id self) throws InterruptedException
	{
		String template = getSkillTemplate(self);
		
		if (template == null)
		{
			return;
		}
		
		if (!template.equals("entertainer_2a"))
		{
			return;
		}
		
		String oldSkills = dataTableGetString(skill_template.TEMPLATE_TABLE, "entertainer_2a", "template");
		String[] oldSkillList = split(oldSkills, ',');
		
		if (oldSkillList == null || oldSkillList.length == 0)
		{
			return;
		}
		
		int oldSkillCount = 0;
		float oldSkillPct = 0.0f;
		
		for (int i = 0; i < oldSkillList.length; i++)
		{
			testAbortScript();
			if (hasSkill(self, oldSkillList[i]))
			{
				oldSkillCount++;
			}
		}
		
		oldSkillPct = (float)oldSkillCount / (float)oldSkillList.length;
		
		skill.revokeAllProfessionSkills(self);
		
		dictionary d = new dictionary();
		d.put("oldSkillPct", oldSkillPct);
		
		messageTo(self, "finishEntertainerRespec", d, 1.0f, true);
	}
	
	
	public void respecNewCrafterSkills(obj_id self) throws InterruptedException
	{
		String template = getSkillTemplate(self);
		
		if (template == null)
		{
			return;
		}
		
		if (!template.startsWith("trader_1"))
		{
			return;
		}
		
		float pctDone = respec.getPercentageCompletion(self, template);
		
		String newTemplate = getCorrectTraderTemplate(template);
		
		skill.revokeAllProfessionSkills(self);
		
		setSkillTemplate(self, newTemplate);
		respec.setPercentageCompletion(self, newTemplate, pctDone, true);
		
		skill.recalcPlayerPools(self, true);
		trial.bumpSession(self, "displayDefensiveMods");
		messageTo(self, "setDisplayOnlyDefensiveMods", trial.getSessionDict(self, "displayDefensiveMods") , 5, false);
	}
	
	
	public String getCorrectTraderTemplate(String template) throws InterruptedException
	{
		if (template.endsWith("1a"))
		{
			return "trader_0a";
		}
		
		if (template.endsWith("1b"))
		{
			return "trader_0b";
		}
		
		if (template.endsWith("1c"))
		{
			return "trader_0c";
		}
		
		if (template.endsWith("1d"))
		{
			return "trader_0d";
		}
		
		return "trader_0a";
	}
	
	public int finishEntertainerRespec(obj_id self, dictionary params) throws InterruptedException
	{
		float oldSkillPct = params.getFloat("oldSkillPct");
		
		String newSkills = dataTableGetString(skill_template.TEMPLATE_TABLE, "entertainer_1a", "template");
		String[] newSkillList = split(newSkills, ',');
		
		int newSkillCount = (int)Math.ceil(oldSkillPct * (float)newSkillList.length);
		
		setSkillTemplate(self, "entertainer_1a");
		
		for (int i = 0; i < newSkillCount; i++)
		{
			testAbortScript();
			skill.grantSkillToPlayer(self, newSkillList[i]);
		}
		
		if (newSkillCount < newSkillList.length)
		{
			setWorkingSkill(self, newSkillList[newSkillCount]);
		}
		
		skill.recalcPlayerPools(self, true);
		trial.bumpSession(self, "displayDefensiveMods");
		messageTo(self, "setDisplayOnlyDefensiveMods", trial.getSessionDict(self, "displayDefensiveMods") , 5, false);
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int handleSetBounty(obj_id self, dictionary params) throws InterruptedException
	{
		int bp = sui.getIntButtonPressed(params);
		
		if (bp == sui.BP_CANCEL)
		{
			return SCRIPT_CONTINUE;
		}
		
		if (!utils.hasScriptVar(self, "setbounty.killer"))
		{
			return SCRIPT_CONTINUE;
		}
		
		obj_id killer = utils.getObjIdScriptVar(self, "setbounty.killer");
		utils.removeScriptVar(self, "setbounty.killer");
		
		int amount = utils.stringToInt(sui.getInputBoxText(params));
		
		if (amount < 0)
		{
			sendSystemMessage(self, new string_id("bounty_hunter", "setbounty_invalid_number"));
			bounty_hunter.showSetBountySUI(self, killer);
			
			return SCRIPT_CONTINUE;
		}
		
		if (amount > bounty_hunter.MAX_BOUNTY_SET)
		{
			sendSystemMessage(self, new string_id("bounty_hunter", "setbounty_cap"));
			amount = bounty_hunter.MAX_BOUNTY_SET;
		}
		
		if (amount < bounty_hunter.MIN_BOUNTY_SET)
		{
			sendSystemMessage(self, new string_id("bounty_hunter", "setbounty_too_little"));
			bounty_hunter.showSetBountySUI(self, killer);
			
			return SCRIPT_CONTINUE;
		}
		
		if (hasObjVar(killer, "bounty.amount"))
		{
			int bounty = getIntObjVar(killer, "bounty.amount");
			if (bounty >= bounty_hunter.MAX_BOUNTY)
			{
				sendSystemMessage(self, new string_id("bounty_hunter", "max_bounty"));
				return SCRIPT_CONTINUE;
			}
			else if ((bounty + amount) > bounty_hunter.MAX_BOUNTY)
			{
				amount = (bounty + amount) - bounty_hunter.MAX_BOUNTY;
			}
		}
		
		int total = getTotalMoney(self);
		
		if (amount > total)
		{
			sendSystemMessage(self, new string_id("bounty_hunter", "setbounty_too_much"));
			bounty_hunter.showSetBountySUI(self, killer);
			
			return SCRIPT_CONTINUE;
		}
		
		dictionary d = new dictionary();
		d.put("killer", killer);
		d.put("amt", amount);
		
		money.pay(self, money.ACCT_BOUNTY, amount, "handleSetBountyTransaction", d, true);
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int handleSetBountyTransaction(obj_id self, dictionary params) throws InterruptedException
	{
		int code = money.getReturnCode(params);
		
		if (code == money.RET_FAIL)
		{
			return SCRIPT_CONTINUE;
		}
		
		obj_id killer = params.getObjId("killer");
		int amount = params.getInt("amt");
		
		int bounty = 0;
		
		if (hasObjVar(killer, "bounty.amount"))
		{
			bounty = getIntObjVar(killer, "bounty.amount");
		}
		
		bounty += amount;
		
		if (bounty >= 10000)
		{
			setJediBountyValue(killer, bounty);
		}
		
		setObjVar(killer, "bounty.amount", bounty);
		
		CustomerServiceLog("bounty", "%TU has taken a bounty of "+ amount + " credits out on %TT", self, killer);
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int handleBountyMissionIncomplete(obj_id self, dictionary params) throws InterruptedException
	{
		obj_id target = params.getObjId("target");
		
		removeJediBounty(target, self);
		
		obj_id mission = bounty_hunter.getBountyMission(self, target);
		
		if (isIdValid(mission))
		{
			sendSystemMessage(self, new string_id("bounty_hunter", "bounty_incomplete"));
			endMission(mission);
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int handleUpdateBountyMissionTime(obj_id self, dictionary params) throws InterruptedException
	{
		int gameTime = params.getInt("gameTime");
		int lastMission = 0;
		
		if (hasObjVar(self, "bounty.lastMissionTime"))
		{
			lastMission = getIntObjVar(self, "bounty.lastMissionTime");
		}
		
		if (lastMission < gameTime)
		{
			setObjVar(self, "bounty.lastMissionTime", gameTime);
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int handleBountyMissionTimeout(obj_id self, dictionary params) throws InterruptedException
	{
		int gameTime = getGameTime();
		int lastMission = 0;
		
		if (hasObjVar(self, "bounty.lastMissionTime"))
		{
			lastMission = getIntObjVar(self, "bounty.lastMissionTime");
		}
		
		if ((gameTime - lastMission) > bounty_hunter.BOUNTY_MISSION_TIME_LIMIT)
		{
			obj_id[] hunters = getJediBounties(self);
			
			if (hunters != null && hunters.length > 0)
			{
				for (int i = 0; i < hunters.length; i++)
				{
					testAbortScript();
					dictionary d = new dictionary();
					d.put("target", self);
					
					messageTo(hunters[i], "handleBountyMissionIncomplete", d, 0.0f, true);
				}
			}
			
			removeAllJediBounties(self);
			removeObjVar(self, "bounty.lastMissionTime");
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public boolean performCriticalHeal(obj_id self) throws InterruptedException
	{
		int[] buffs = buff.getAllBuffs(self);
		
		if (buffs == null || buffs.length == 0)
		{
			return false;
		}
		
		for (int i = 0; i<buffs.length; i++)
		{
			testAbortScript();
			String type = buff.getEffectParam(buffs[i], 1);
			if (type.equals("avoid_incap_heal"))
			{
				String subType = dataTableGetString("datatables/buff/effect_mapping.iff", "avoid_incap_heal", "SUBTYPE");
				float value = utils.getFloatScriptVar(self, "buff_handler."+subType);
				
				healing.healDamage(self, self, HEALTH, (int)value);
				buff.removeBuff(self, buff.getBuffNameFromCrc(buffs[i]));
				buff.applyBuff(self, "gcw_base_critical_heal_recourse");
				string_id SID_INCAP_HEAL = new string_id("cbt_spam", "incap_heal");
				prose_package pp = new prose_package();
				pp = prose.setStringId(pp, SID_INCAP_HEAL);
				pp = prose.setDF(pp, value);
				pp = prose.setTU(pp, self);
				sendCombatSpamMessageProse(self, self, pp, true, true, true, COMBAT_RESULT_MEDICAL);
				return true;
			}
		}
		
		return false;
	}
	
	
	public void sendSmugglerSystemBootstrap(obj_id self) throws InterruptedException
	{
		if (utils.isProfession(self, utils.SMUGGLER))
		{
			if (isInTutorialArea(self))
			{
				return;
			}
			
			if (hasObjVar(self, "smuggler_bootstrap"))
			{
				return;
			}
			
			messageTo(self, "handleSendSmugglerBootstrapRequest", null, 120f, false);
		}
	}
	
	
	public int handleSendSmugglerBootstrapRequest(obj_id self, dictionary params) throws InterruptedException
	{
		string_id subject = new string_id("smuggler/messages", "bootstrap_subject");
		string_id body = new string_id("smuggler/messages", "bootstrap_body");
		
		String body_oob = chatMakePersistentMessageOutOfBandBody(null, body);
		body_oob = chatAppendPersistentMessageWaypointData(body_oob, "tatooine", -1049f, -3538f, new string_id("mob/creature_names", "smuggler_broker_barak"), null);
		String subject_str = "@"+ subject.toString ();
		
		chatSendPersistentMessage("Barak", self, subject_str, null, body_oob);
		
		setObjVar(self, "smuggler_bootstrap", 1);
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int handleSmugglerMissionFailureSignal(obj_id self, dictionary params) throws InterruptedException
	{
		groundquests.sendSignal(self, "smugglerEnemyIncap");
		return SCRIPT_CONTINUE;
	}
	
	
	public int OnClusterWideDataResponse(obj_id self, String manage_name, String data_name, int request_id, String[] element_name_list, dictionary[] dungeon_data, int lock_key) throws InterruptedException
	{
		if (dungeon_data == null || dungeon_data.length == 0)
		{
			return SCRIPT_CONTINUE;
		}
		
		if (data_name.startsWith("base_cwdata_manager"))
		{
			int myBases = 0;
			
			for (int i=0; i<dungeon_data.length; i++)
			{
				testAbortScript();
				if (dungeon_data[i].getObjId("ownerId") == self)
				{
					myBases++;
				}
			}
			
			int recordedBases = getIntObjVar(self, "factionBaseCount");
			
			if (recordedBases != myBases)
			{
				int correction = recordedBases - myBases;
				
				prose_package pp = new prose_package();
				pp = prose.setStringId(pp, new string_id("gcw", "faction_base_correction"));
				pp = prose.setDI(pp, correction);
				pp = prose.setTO(pp, ""+myBases);
				
				sendSystemMessageProse(self, pp);
				setObjVar(self, "factionBaseCount", myBases);
			}
			
		}
		return SCRIPT_CONTINUE;
	}
	
	
	public int removeIncubatorFromUser(obj_id self, dictionary params) throws InterruptedException
	{
		obj_id station = params.getObjId("station");
		incubator.forceRemoveIncubatorFromUser(station, self);
		return SCRIPT_CONTINUE;
	}
	
	
	public int handleStoryTellerAssistantRequest(obj_id self, dictionary params) throws InterruptedException
	{
		obj_id storytellerPlayer = params.getObjId("storytellerPlayer");
		String storytellerName = params.getString("storytellerName");
		
		if (isIdValid(storytellerPlayer))
		{
			if (storyteller.doIAutoDeclineStorytellerInvites(self))
			{
				dictionary webster = new dictionary();
				webster.put("targetName", getName(self));
				messageTo(storytellerPlayer, "handleStorytellerAssistantDeclined", webster, 0, false);
			}
			else
			{
				if (!utils.hasScriptVar(self, "storytellerAssistant"))
				{
					storyteller.storyAssistantSui(storytellerPlayer, storytellerName, self);
				}
				else
				{
					boolean alreadyAStorytellersAssistant = false;
					
					obj_id storytellerId = utils.getObjIdScriptVar(self, "storytellerAssistant");
					if (storytellerId == storytellerPlayer)
					{
						alreadyAStorytellersAssistant = true;
					}
					
					dictionary webster = new dictionary();
					webster.put("alreadyAStorytellersAssistant", alreadyAStorytellersAssistant);
					webster.put("targetName", getName(self));
					messageTo(storytellerPlayer, "handleStorytellerAlreadyAnAssistant", webster, 0, false);
				}
			}
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int handleTellStorytellerIDeclinedAssistant(obj_id self, dictionary params) throws InterruptedException
	{
		obj_id storytellerPlayer = params.getObjId("storytellerPlayer");
		if (isIdValid(storytellerPlayer))
		{
			dictionary webster = new dictionary();
			webster.put("targetName", getName(self));
			messageTo(storytellerPlayer, "handleStorytellerAssistantDeclined", webster, 0, false);
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int handleStoryTellerAssistantAccepted(obj_id self, dictionary params) throws InterruptedException
	{
		obj_id storytellerPlayer = params.getObjId("storytellerPlayer");
		String storytellerName = params.getString("storytellerName");
		
		if (isIdValid(storytellerPlayer))
		{
			storyteller.storyAssistantAcepted(storytellerPlayer, storytellerName, self);
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int handleStoryTellerRemoveAssistant(obj_id self, dictionary params) throws InterruptedException
	{
		obj_id storytellerPlayer = params.getObjId("storytellerPlayer");
		String storytellerName = params.getString("storytellerName");
		
		if (isIdValid(storytellerPlayer))
		{
			dictionary webster = new dictionary();
			webster.put("removedPlayerName", getName(self));
			
			if (utils.hasScriptVar(self, "storytellerAssistant"))
			{
				obj_id targetStorytellerId = utils.getObjIdScriptVar(self, "storytellerAssistant");
				if (targetStorytellerId == storytellerPlayer)
				{
					storyteller.storyPlayerRemoveAssistant(storytellerPlayer, storytellerName, self);
				}
				else
				{
					messageTo(storytellerPlayer, "handleStorytellerRemovePlayerNotAssistant", webster, 0, false);
				}
			}
			else
			{
				messageTo(storytellerPlayer, "handleStorytellerRemovePlayerNotAssistant", webster, 0, false);
			}
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int handleStoryTellerInviteRequest(obj_id self, dictionary params) throws InterruptedException
	{
		obj_id storytellerPlayer = params.getObjId("storytellerPlayer");
		String storytellerName = params.getString("storytellerName");
		obj_id storytellerAssistant = params.getObjId("storytellerAssistant");
		
		if (isIdValid(storytellerPlayer))
		{
			if (storyteller.doIAutoDeclineStorytellerInvites(self))
			{
				dictionary webster = new dictionary();
				webster.put("targetName", getName(self));
				messageTo(storytellerPlayer, "handleStorytellerInviteDeclined", webster, 0, false);
			}
			else
			{
				if (!utils.hasScriptVar(self, "storytellerid"))
				{
					if (isIdValid(storytellerAssistant))
					{
						storyteller.storyInviteSui(storytellerAssistant, storytellerName, self);
					}
					else
					{
						storyteller.storyInviteSui(storytellerPlayer, storytellerName, self);
					}
				}
				else
				{
					boolean alreadyInStorytellersStory = false;
					
					obj_id storytellerId = utils.getObjIdScriptVar(self, "storytellerid");
					if (storytellerId == storytellerPlayer)
					{
						alreadyInStorytellersStory = true;
					}
					
					dictionary webster = new dictionary();
					webster.put("alreadyInStorytellersStory", alreadyInStorytellersStory);
					webster.put("targetName", getName(self));
					webster.put("storytellerName", storytellerName);
					
					if (isIdValid(storytellerAssistant))
					{
						messageTo(storytellerAssistant, "handleStorytellerInviteAlreadyInAStory", webster, 0, false);
					}
					else
					{
						messageTo(storytellerPlayer, "handleStorytellerInviteAlreadyInAStory", webster, 0, false);
					}
				}
			}
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int handleTellStorytellerIDeclinedInvite(obj_id self, dictionary params) throws InterruptedException
	{
		obj_id storytellerPlayer = params.getObjId("storytellerPlayer");
		if (isIdValid(storytellerPlayer))
		{
			dictionary webster = new dictionary();
			webster.put("targetName", getName(self));
			messageTo(storytellerPlayer, "handleStorytellerInviteDeclined", webster, 0, false);
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int handleStoryTellerInviteAccepted(obj_id self, dictionary params) throws InterruptedException
	{
		obj_id storytellerPlayer = params.getObjId("storytellerPlayer");
		String storytellerName = params.getString("storytellerName");
		obj_id storytellerAssistant = params.getObjId("storytellerAssistant");
		
		if (isIdValid(storytellerPlayer))
		{
			storyteller.storyInviteAcepted(storytellerPlayer, storytellerName, self, storytellerAssistant);
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int handleStoryTellerRemovedFromStory(obj_id self, dictionary params) throws InterruptedException
	{
		obj_id storytellerPlayer = params.getObjId("storytellerPlayer");
		String storytellerName = params.getString("storytellerName");
		
		if (isIdValid(storytellerPlayer))
		{
			
			dictionary webster = new dictionary();
			webster.put("removedPlayerName", getName(self));
			
			if (utils.hasScriptVar(self, "storytellerid"))
			{
				obj_id targetStorytellerId = utils.getObjIdScriptVar(self, "storytellerid");
				if (targetStorytellerId == storytellerPlayer)
				{
					storyteller.storyPlayerRemovedFromStory(storytellerPlayer, storytellerName, self);
				}
				else
				{
					messageTo(storytellerPlayer, "handleStorytellerRemovePlayerNotInStory", null, 0, false);
				}
			}
			else
			{
				messageTo(storytellerPlayer, "handleStorytellerRemovePlayerNotInStory", null, 0, false);
			}
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int setDisplayOnlyDefensiveMods(obj_id self, dictionary params) throws InterruptedException
	{
		if (!trial.verifySession(self, params, "displayDefensiveMods"))
		{
			return SCRIPT_CONTINUE;
		}
		
		String[] variousMods =
		{
			"display_only_glancing_blow",
			"display_only_dodge",
			"display_only_parry",
			"display_only_block",
			"display_only_tohit",
			"display_only_evasion",
			"display_only_strikethrough",
			"display_only_crit_resistance",
			"display_only_critical",
			"display_only_parry_reduction",
			"display_only_opp_block_reduction",
			"display_only_opp_dodge_reduction",
			"display_only_expertise_critical_hit_reduction",
			"display_only_expertise_critical_hit_pvp_reduction"
		};
		
		float glancingBlowChance = combat.getDefenderGlancingBlowChance(self) * 100;
		float dodgeChance = combat.getDefenderDodgeChance(self) * 100;
		float parryChance = combat.getDefenderParryChance(self) * 100;
		float blockChance = combat.getDefenderBlockChance(self) * 100;
		float evasionChance = combat.getDefenderEvasionChance(self) * 100;
		float strikeThroughChance = combat.getAttackerStrikethroughChance(self) * 100;
		float toHitBonus = combat.getToHitBonus(self) * 100;
		float criticalChance = (combat.getAttackerCritMod(self)) * 100;
		
		float parryRedcution = combat.getAttackerParryReduction(self) * 100;
		float dodgeReduction = combat.getAttackerDodgeReduction(self) * 100;
		float blockReduction = combat.getAttackerBlockReduction(self) * 100;
		float hitReduction = combat.getDefenderCriticalChance(self) * 100;
		float hitPvPReduction = (combat.getDefenderCriticalChance(self) + combat.getPvPHitReductionChance(self)) * 100;
		
		for (int i = 0; i < variousMods.length; i++)
		{
			testAbortScript();
			removeAttribOrSkillModModifier(self, variousMods[i]);
		}
		
		addSkillModModifier(self, "display_only_glancing_blow", "display_only_glancing_blow", (int)glancingBlowChance, -1, false, false);
		addSkillModModifier(self, "display_only_dodge", "display_only_dodge", (int)dodgeChance, -1, false, false);
		addSkillModModifier(self, "display_only_parry", "display_only_parry", (int)parryChance, -1, false, false);
		addSkillModModifier(self, "display_only_block", "display_only_block", (int)blockChance, -1, false, false);
		addSkillModModifier(self, "display_only_tohit", "display_only_tohit", (int)toHitBonus, -1, false, false);
		addSkillModModifier(self, "display_only_evasion", "display_only_evasion", (int)evasionChance, -1, false, false);
		addSkillModModifier(self, "display_only_strikethrough", "display_only_strikethrough", (int)strikeThroughChance, -1, false, false);
		addSkillModModifier(self, "display_only_critical", "display_only_critical", (int)criticalChance, -1, false, false);
		
		addSkillModModifier(self, "display_only_parry_reduction", "display_only_parry_reduction", (int)parryRedcution, -1, false, false);
		addSkillModModifier(self, "display_only_opp_dodge_reduction", "display_only_opp_dodge_reduction", (int)dodgeReduction, -1, false, false);
		addSkillModModifier(self, "display_only_opp_block_reduction", "display_only_opp_block_reduction", (int)blockReduction, -1, false, false);
		addSkillModModifier(self, "display_only_expertise_critical_hit_reduction", "display_only_expertise_critical_hit_reduction", (int)hitReduction, -1, false, false);
		addSkillModModifier(self, "display_only_expertise_critical_hit_pvp_reduction", "display_only_expertise_critical_hit_pvp_reduction", (int)hitPvPReduction, -1, false, false);
		return SCRIPT_CONTINUE;
	}
	
	
	public int OnGetAttributes(obj_id self, obj_id player, String[] names, String[] attribs) throws InterruptedException
	{
		int idx = utils.getValidAttributeIndex(names);
		if (idx == -1)
		{
			return super.OnGetAttributes(self, player, names, attribs);
		}
		
		if (utils.hasScriptVar(self, "storytellerName"))
		{
			String storytellerName = utils.getStringScriptVar(self, "storytellerName");
			names[idx] = "storyteller_name";
			attribs[idx] = storytellerName;
			idx++;
			if (idx >= names.length)
			{
				return super.OnGetAttributes(self, player, names, attribs);
			}
		}
		
		if (utils.hasScriptVar(self, "storytellerAssistantName"))
		{
			String storytellerAssistantName = utils.getStringScriptVar(self, "storytellerAssistantName");
			names[idx] = "storyteller_assistant_name";
			attribs[idx] = storytellerAssistantName;
			idx++;
			if (idx >= names.length)
			{
				return super.OnGetAttributes(self, player, names, attribs);
			}
		}
		
		return super.OnGetAttributes(self, player, names, attribs);
		
	}
	
	
	public int cmdGetGalaxyMessage(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		obj_id planetId = getPlanetByName("tatooine");
		if (!isIdNull(planetId))
		{
			if (hasObjVar(planetId, "galaxyMessage"))
			{
				String strGalaxyMessage = "\\#FF0000"+utils.getStringObjVar(planetId, "galaxyMessage")+"\\#FFFFFF";
				sendConsoleMessage(self, strGalaxyMessage);
			}
			else
			{
				String strGalaxyMessage = "\\#FF0000"+"Welcome to Star Wars Galaxies"+"\\#FFFFFF";
				sendConsoleMessage(self, strGalaxyMessage);
			}
			
			boolean warden = isWarden(self);
			if (warden || (getGodLevel(self) >= 10))
			{
				String strGalaxyMessage = "\\#FF0000";
				if (!warden)
				{
					strGalaxyMessage += "Displaying warden MOTD to you even though you are not a warden, because you are of the appropriate god level:\n";
				}
				
				if (hasObjVar(planetId, "galaxyWardenMessage"))
				{
					strGalaxyMessage += utils.getStringObjVar(planetId, "galaxyWardenMessage");
				}
				else
				{
					strGalaxyMessage += "Welcome warden";
				}
				
				strGalaxyMessage += "\\#FFFFFF";
				sendConsoleMessage(self, strGalaxyMessage);
			}
		}
		else
		{
			sendConsoleMessage(self, "Welcome to Star Wars Galaxies");
		}
		return SCRIPT_CONTINUE;
	}
	
	
	public int myCityMotdResponse(obj_id self, dictionary params) throws InterruptedException
	{
		String city_motd = params.getString("city_motd");
		if (city_motd != null && city_motd.length() > 0)
		{
			sendConsoleMessage(self, city_motd + "\\#DFDFDF");
		}
		
		utils.setScriptVar(self, "recieved_city_motd", 1);
		return SCRIPT_CONTINUE;
	}
	
	
	public int stampDungeonArea(obj_id self, dictionary params) throws InterruptedException
	{
		String dungeon_area = params.getString("buildout_area");
		if (dungeon_area.equals("invalid"))
		{
			return SCRIPT_CONTINUE;
		}
		
		int buildout_row = params.getInt("buildout_row");
		if (buildout_row == -1)
		{
			return SCRIPT_CONTINUE;
		}
		
		setObjVar(self, space_dungeon.VAR_BUILDOUT_AREA, dungeon_area);
		setObjVar(self, space_dungeon.VAR_BUILDOUT_ROW, buildout_row);
		return SCRIPT_CONTINUE;
	}
	
	
	public int removeDungeonArea(obj_id self, dictionary params) throws InterruptedException
	{
		String dungeon_area = params.getString("buildout_area");
		if (dungeon_area.equals("invalid"))
		{
			return SCRIPT_CONTINUE;
		}
		
		int buildout_row = params.getInt("buildout_row");
		if (buildout_row == -1)
		{
			return SCRIPT_CONTINUE;
		}
		
		if (!hasObjVar(self, space_dungeon.VAR_BUILDOUT_AREA) || getStringObjVar(self, space_dungeon.VAR_BUILDOUT_AREA) != dungeon_area)
		{
			return SCRIPT_CONTINUE;
		}
		
		removeObjVar(self, space_dungeon.VAR_BUILDOUT_AREA);
		removeObjVar(self, space_dungeon.VAR_BUILDOUT_ROW);
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int handleStopDanceMessage(obj_id self, dictionary params) throws InterruptedException
	{
		utils.removeScriptVar(self, "stopDanceMessage");
		return SCRIPT_CONTINUE;
	}
	
	
	public int onGuildCreateTerminalDataObject(obj_id self, dictionary params) throws InterruptedException
	{
		
		boolean hasTerminal = utils.playerHasItemByTemplateInDataPad(self, guild.STR_GUILD_REMOTE_DEVICE);
		
		if (!hasTerminal)
		{
			obj_id pDataPad = utils.getPlayerDatapad(self);
			createObjectOverloaded(guild.STR_GUILD_REMOTE_DEVICE, utils.getPlayerDatapad(self));
			
			string_id stringMessage = new string_id("guild", "datapad_terminal_created");
			
			sendSystemMessage(self, stringMessage);
		}
		return SCRIPT_CONTINUE;
	}
	
	
	public int applyMountBuff(obj_id self, dictionary params) throws InterruptedException
	{
		obj_id playerCurrentMount = getMountId(self);
		if (isIdValid(playerCurrentMount))
		{
			vehicle.applyVehicleBuffs(self, playerCurrentMount);
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int handleDailyMissionReset(obj_id self, dictionary params) throws InterruptedException
	{
		missions.clearDailyObjVars(self);
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int cmdMeditate(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		
		if (getPosture(self) != POSTURE_SITTING)
		{
			sendSystemMessage(self, new string_id("jedi_spam", "meditate_not_sitting"));
			return SCRIPT_CONTINUE;
		}
		
		stealth.checkForAndMakeVisibleNoRecourse(self);
		
		if (getState(self, STATE_MEDITATE) == 1)
		{
			sendSystemMessage(self, new string_id("jedi_spam", "already_in_meditative_state"));
			return SCRIPT_CONTINUE;
		}
		
		meditation.startMeditation(self);
		
		return SCRIPT_CONTINUE;
	}
	
	public int cmdMeditateFail(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		sendSystemMessage(self, meditation.SID_MED_FAIL);
		return SCRIPT_CONTINUE;
	}
	
	
	public int handleMeditationTick(obj_id self, dictionary params) throws InterruptedException
	{
		if (!meditation.isMeditating(self))
		{
			meditation.endMeditation(self, false);
			return SCRIPT_CONTINUE;
		}
		
		if (!trial.verifySession(self, params, meditation.HANDLER_MEDITATION_TICK))
		{
			return SCRIPT_CONTINUE;
		}
		
		if (utils.isProfession(self, utils.FORCE_SENSITIVE))
		{
			
			int value = 0;
			
			if (utils.hasScriptVar(self, meditation.VAR_MEDITATION_BASE))
			{
				value = utils.getIntScriptVar(self, meditation.VAR_MEDITATION_BASE);
				value++;
				
				if (value >= 3)
				{
					
					stealth.checkForAndMakeVisibleNoRecourse(self);
					
					String meditateBuff = meditation.MEDITATE_BUFFS[rand(0, meditation.MEDITATE_BUFFS.length-1)];
					
					buff.applyBuff(self, meditateBuff);
					value = 0;
				}
				
			}
			
			utils.setScriptVar(self, meditation.VAR_MEDITATION_BASE, value);
		}
		
		messageTo(self, meditation.HANDLER_MEDITATION_TICK, trial.getSessionDict(self, meditation.HANDLER_MEDITATION_TICK), 10f, false);
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int msgCoupDeGraceAuthoritativeCheck(obj_id self, dictionary params) throws InterruptedException
	{
		if (!utils.hasScriptVar(self, "death.beingCoupDeGraced"))
		{
			return SCRIPT_CONTINUE;
		}
		
		if (params == null || params.isEmpty())
		{
			utils.removeScriptVar(self, "death.beingCoupDeGraced");
			return SCRIPT_CONTINUE;
		}
		
		int numberOfTries = params.getInt("numberOfTries");
		
		obj_id victim = params.getObjId("victim");
		obj_id killer = params.getObjId("killer");
		boolean playAnim = params.getBoolean("playAnim");
		boolean usePVPRules = params.getBoolean("usePVPRules");
		
		if (victim.isAuthoritative() && killer.isAuthoritative())
		{
			utils.removeScriptVar(self, "death.beingCoupDeGraced");
			pclib.coupDeGrace(victim, killer, playAnim, usePVPRules);
		}
		else
		{
			
			if (numberOfTries >= 5)
			{
				CustomerServiceLog("player_death", "coupDeGrace:Player %TU was Death Blowed by %TT, "+ numberOfTries + " attempts have been made to force them onto the same process, and has failed. %TU may be perma-incapped", victim, killer);
				utils.removeScriptVar(self, "death.beingCoupDeGraced");
				return SCRIPT_CONTINUE;
			}
			
			params.put("numberOfTries", ++numberOfTries);
			messageTo(victim, "msgCoupDeGraceAuthoritativeCheck", params, 2, false);
		}
		return SCRIPT_CONTINUE;
	}
	
	
	public boolean lifeDayNewDay(obj_id player) throws InterruptedException
	{
		if (hasObjVar(player, "lifeday.time_stamp"))
		{
			int now = getCalendarTime();
			int then = getIntObjVar(player, "lifeday.time_stamp");
			
			if (now > then)
			{
				return true;
			}
			else
			{
				return false;
			}
		}
		return true;
	}
	
	
	public boolean colorizeObject(obj_id self, obj_id object, obj_id tool, String params) throws InterruptedException
	{
		if (!isValidId(self) || !exists(self))
		{
			return false;
		}
		if (!isValidId(object) || !exists(object))
		{
			return false;
		}
		if (!isValidId(tool) || !exists(tool))
		{
			return false;
		}
		if (params == null || params.equals(""))
		{
			return false;
		}
		
		if (params == null || params.equals(""))
		{
			messageTo(tool, "cancelTool", null, 0, false);
			return false;
		}
		
		String[] colorArray = split(params, ' ');
		if (colorArray == null || colorArray.length <= 0)
		{
			return false;
		}
		
		for (int i = 0; i < colorArray.length; i+=2)
		{
			testAbortScript();
			if (colorArray[i] == null || colorArray[i].equals(""))
			{
				break;
			}
			hue.setColor(object, colorArray[i], utils.stringToInt(colorArray[i+1]));
		}
		
		messageTo(tool, "decrementTool", null, 0, false);
		return true;
	}
	
	
	public boolean isPvpRelatedDeath(obj_id player) throws InterruptedException
	{
		if (!utils.hasScriptVar(player, "lastKiller"))
		{
			return false;
		}
		
		obj_id lastKillerOID = utils.getObjIdScriptVar(player, "lastKiller");
		
		utils.removeScriptVar(player, "lastKiller");
		
		if (!isIdValid(lastKillerOID))
		{
			return false;
		}
		
		if (isPlayer(lastKillerOID))
		{
			return true;
		}
		
		if ((pet_lib.isPet(lastKillerOID) || pet_lib.isDroidPet(lastKillerOID) || pet_lib.isNpcPet(lastKillerOID) || pet_lib.isCombatDroid(lastKillerOID) || beast_lib.isBeast(lastKillerOID)))
		{
			obj_id master = getMaster(lastKillerOID);
			if (isIdValid(master) && isPlayer(master))
			{
				return true;
			}
		}
		
		return false;
	}
	
	
	public boolean needsBlackwingLightInfection(obj_id self) throws InterruptedException
	{
		if (( !buff.hasBuff(self, "death_troopers_infection_1") && !buff.hasBuff(self, "death_troopers_infection_2") && !buff.hasBuff(self, "death_troopers_infection_3") && !buff.hasBuff(self, "death_troopers_inoculation") ))
		{
			return true;
		}
		
		return false;
	}
	
	
	public int death_troopers_apply_virus(obj_id self, dictionary params) throws InterruptedException
	{
		if (needsBlackwingLightInfection(self) && locations.isInRegion(self, "@dathomir_region_names:black_mesa"))
		{
			buff.applyBuff(self, "death_troopers_infection_1");
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int handleBuyBackSui(obj_id self, dictionary params) throws InterruptedException
	{
		if (params == null || params.equals(""))
		{
			CustomerServiceLog("Junk_Dealer: ", "base_player.handleBuyBackSui() - Player: "+self+" attempted to get a list of buy back items but the message handler received not data.");
			return SCRIPT_CONTINUE;
		}
		
		obj_id junkDealer = params.getObjId("dealer");
		
		if (!isIdValid(junkDealer))
		{
			return SCRIPT_CONTINUE;
		}
		
		if (utils.outOfRange(junkDealer, self, 10.0f, true))
		{
			CustomerServiceLog("Junk_Dealer: ", "base_player.handleBuyBackSui() - Player: "+self+" attempted to get a list of buy back items but the action failed. The player was too far away from the TERMINAL.");
			return SCRIPT_CONTINUE;
		}
		
		int idx = sui.getListboxSelectedRow(params);
		int bp = sui.getIntButtonPressed(params);
		
		obj_id[] junk = utils.getObjIdBatchScriptVar(self, smuggler.SCRIPTVAR_JUNK_IDS);
		
		smuggler.cleanupSellJunkSui(self);
		
		if (bp == sui.BP_CANCEL)
		{
			
			return SCRIPT_CONTINUE;
		}
		
		else if (bp == sui.BP_REVERT)
		{
			if (idx >= 0 && idx <= junk.length - 1)
			{
				if ((junk != null) || (junk.length > 0))
				{
					openExamineWindow(self, junk[idx]);
				}
			}
			
			obj_id[] buyBackItems = smuggler.getBuyBackItemsInContainer(self);
			if (buyBackItems != null && buyBackItems.length > 0)
			{
				smuggler.showBuyBackSui(self, junkDealer);
			}
			return SCRIPT_CONTINUE;
		}
		
		else
		{
			
			if (idx < 0)
			{
				return SCRIPT_CONTINUE;
			}
			
			if ((junk == null) || (junk.length == 0))
			{
				return SCRIPT_CONTINUE;
			}
			
			if (idx > junk.length - 1)
			{
				return SCRIPT_CONTINUE;
			}
			
			smuggler.buyBackJunkItem(self, junkDealer, junk[idx], true);
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int handleTheBuyBack(obj_id self, dictionary params) throws InterruptedException
	{
		if (params == null || params.equals(""))
		{
			CustomerServiceLog("Junk_Dealer: ", "base_player.handleBuyBackSui() - Player: "+self+" attempted to get a list of buy back items but the message handler received not data.");
			return SCRIPT_CONTINUE;
		}
		
		obj_id junkDealer = params.getObjId("dealer");
		
		if (!isIdValid(junkDealer))
		{
			return SCRIPT_CONTINUE;
		}
		
		if (utils.outOfRange(junkDealer, self, 10.0f, true))
		{
			CustomerServiceLog("Junk_Dealer: ", "base_player.handleBuyBackSui() - Player: "+self+" attempted to get a list of buy back items but the action failed. The player was too far away from the TERMINAL.");
			return SCRIPT_CONTINUE;
		}
		
		obj_id item = params.getObjId("item");
		if (!isIdValid(item))
		{
			CustomerServiceLog("Junk_Dealer: ", "base_player.handleTheBuyBack() - Player (OID: "+ self + ") could not sell item because the item parameter was invalid.");
			return SCRIPT_CONTINUE;
		}
		
		int price = params.getInt("price");
		if (price <= 0)
		{
			CustomerServiceLog("Junk_Dealer: ", "base_player.handleTheBuyBack() - Player (OID: "+ self + ") could not sell item because the price on the item was invalid.");
			return SCRIPT_CONTINUE;
		}
		
		int retCode = params.getInt(money.DICT_CODE);
		
		if (retCode == money.RET_FAIL)
		{
			
			prose_package ppNoSale = prose.getPackage(smuggler.PROSE_NO_BUY_BACK, item);
			sendSystemMessageProse(self, ppNoSale);
			CustomerServiceLog("Junk_Dealer: ", "base_player.handleTheBuyBack() - Player (OID: "+ self + ") could not sell item ("+ item + ") for ("+ price + ") because the player did not have the credits.");
			return SCRIPT_CONTINUE;
		}
		
		prose_package ppSoldJunk = prose.getPackage(smuggler.PROSE_PURCHASED_JUNK, item, price);
		sendSystemMessageProse(self, ppSoldJunk);;
		CustomerServiceLog("Junk_Dealer: ", "base_player.handleTheBuyBack() - Player (OID: "+ self + ") has paid: "+price+" for a buy back item and is about to retrieve that item: "+item+" from their buy back container.");
		
		smuggler.moveBuyBackObjectIntoInventory(self, item);
		
		obj_id[] buyBackItems = smuggler.getBuyBackItemsInContainer(self);
		if (buyBackItems != null && buyBackItems.length > 0)
		{
			smuggler.showBuyBackSui(self, junkDealer);
		}
		return SCRIPT_CONTINUE;
	}
	
	
	public int handleFlagJunkSui(obj_id self, dictionary params) throws InterruptedException
	{
		if (params == null || params.equals(""))
		{
			CustomerServiceLog("Junk_Dealer: ", "base_player.handleBuyBackSui() - Player: "+self+" attempted to get a list of buy back items but the message handler received not data.");
			return SCRIPT_CONTINUE;
		}
		
		obj_id junkDealer = params.getObjId("dealer");
		
		if (!isIdValid(junkDealer))
		{
			return SCRIPT_CONTINUE;
		}
		
		if (utils.outOfRange(junkDealer, self, 10.0f, true))
		{
			CustomerServiceLog("Junk_Dealer: ", "base_player.handleBuyBackSui() - Player: "+self+" attempted to get a list of buy back items but the action failed. The player was too far away from the TERMINAL.");
			return SCRIPT_CONTINUE;
		}
		
		int idx = sui.getListboxSelectedRow(params);
		int bp = sui.getIntButtonPressed(params);
		
		obj_id[] junk = utils.getObjIdBatchScriptVar(self, smuggler.SCRIPTVAR_JUNK_IDS);
		
		boolean reshowSui = true;
		if (junk.length < 1)
		{
			reshowSui = false;
		}
		
		smuggler.cleanupSellJunkSui(self);
		
		if (bp == sui.BP_CANCEL)
		{
			return SCRIPT_CONTINUE;
		}
		else if (bp == sui.BP_REVERT)
		{
			if (idx >= 0 && idx <= junk.length - 1)
			{
				if ((junk != null) || (junk.length > 0))
				{
					openExamineWindow(self, junk[idx]);
					smuggler.flagJunkSaleSui(self, junkDealer);
				}
			}
			return SCRIPT_CONTINUE;
		}
		else
		{
			
			if (idx < 0)
			{
				if (junk != null && junk.length > 0)
				{
					smuggler.flagJunkSaleSui(self, junkDealer);
				}
				
				return SCRIPT_CONTINUE;
			}
			
			if (idx > junk.length - 1)
			{
				if (junk != null && junk.length > 0)
				{
					smuggler.flagJunkSaleSui(self, junkDealer);
				}
				return SCRIPT_CONTINUE;
			}
			
			if (hasObjVar(junk[idx], smuggler.JUNK_DEALKER_NO_SALE_FLAG))
			{
				removeObjVar(junk[idx], smuggler.JUNK_DEALKER_NO_SALE_FLAG);
			}
			else
			{
				setObjVar(junk[idx], smuggler.JUNK_DEALKER_NO_SALE_FLAG, getGameTime());
			}
			smuggler.flagJunkSaleSui(self, junkDealer);
		}
		return SCRIPT_CONTINUE;
	}
	
	
	public boolean blog(String txt) throws InterruptedException
	{
		if (LOGGING_ON)
		{
			LOG(LOGNAME, txt);
		}
		return true;
	}
	
}
