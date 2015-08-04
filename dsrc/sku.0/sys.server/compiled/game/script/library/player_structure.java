package script.library;

import script.*;
import script.base_class.*;
import script.combat_engine.*;
import java.util.Arrays;
import java.util.Hashtable;
import java.util.Vector;
import script.base_script;


import java.lang.Math;
import script.library.city;
import script.library.prose;
import script.library.regions;
import script.library.static_item;
import script.library.sui;
import script.library.target_dummy;
import script.library.trace;
import script.library.trial;
import script.library.utils;
import script.library.vendor_lib;


public class player_structure extends script.base_script
{
	public player_structure()
	{
	}
	public static final boolean LOGGING_ON = false;
	public static final String LOGGING_CATEGORY = "special_sign";
	
	public static final String SCRIPT_TERMINAL_STRUCTURE = "terminal.terminal_structure";
	public static final String SCRIPT_PERMANENT_STRUCTURE = "structure.permanent_structure";
	
	public static final String PLAYER_STRUCTURE_DATATABLE = "datatables/structure/player_structure.iff";
	public static final String PLAYER_STRUCTURE_VALIDATION_DATATABLE = "datatables/structure/player_structure_validation.iff";
	public static final String TBL_SIGN = "datatables/structure/player_structure_sign.iff";
	public static final String TBL_SPECIAL_SIGNS = "datatables/structure/special_sign.iff";
	public static final String SIGN_ADJUST_SIGN_BY_TEMPLATE = "datatables/structure/sign_adjust_by_template.iff";
	
	public static final String DATATABLE_COL_STRUCTURE = "STRUCTURE";
	public static final String DATATABLE_COL_TEMP_STRUCTURE = "TEMP_STRUCTURE";
	public static final String DATATABLE_COL_FOOTPRINT = "FOOTPRINT_TEMPLATE";
	public static final String DATATABLE_COL_EJECT_RANGE = "EJECT_RANGE";
	public static final String DATATABLE_COL_X = "X";
	public static final String DATATABLE_COL_Y = "Y";
	public static final String DATATABLE_COL_Z = "Z";
	public static final String DATATABLE_COL_CELL = "CELL";
	public static final String DATATABLE_COL_HEADING = "HEADING";
	public static final String DATATABLE_COL_OBJECT = "OBJECT";
	public static final String DATATABLE_COL_CIVIC = "CIVIC";
	public static final String DATATABLE_COL_CITY_RANK = "CITY_RANK";
	public static final String DATATABLE_COL_CITY_COST = "CITY_COST";
	public static final String DATATABLE_COL_MAINT_RATE = "MAINT_RATE";
	public static final String DATATABLE_COL_DECAY_RATE = "DECAY_RATE";
	public static final String DATATABLE_COL_REDEED_COST = "REDEED_COST";
	public static final String DATATABLE_COL_CONDITION = "CONDITION";
	public static final String DATATABLE_COL_SKILL_MOD = "SKILLMOD";
	public static final String DATATABLE_COL_SKILL_MOD_VALUE = "SKILLMODVALUE";
	public static final String DATATABLE_COL_SKILL_MOD_MESSAGE = "SKILLMOD_MESSAGE";
	public static final String DATATABLE_COL_VERSION = "VERSION";
	public static final String DATATABLE_COL_NO_LOT_REQ = "NO_LOT_REQUIREMENT";
	public static final String DATATABLE_COL_LOT_REDUCTION = "LOT_REDUCTION";
	public static final String DATATABLE_COL_HAS_SIGN = "HAS_SIGN";
	public static final String DATATABLE_COL_POWER_RATE = "POWER_RATE";
	public static final String WORLD_DELTA = "WORLD_DELTA";
	
	public static final String DATATABLE_COL_TEMPLATE = "TEMPLATE";
	public static final String DATATABLE_COL_HOPPER_MIN = "HOPPER_MIN";
	public static final String DATATABLE_COL_HOPPER_MAX = "HOPPER_MAX";
	
	public static final String VAR_PLAYER_STRUCTURE = "player_structure";
	
	public static final String VAR_TERMINAL_HEADING = "player_structure.intendedHeading";
	public static final String OBJVAR_STORED_HANGAR_LITE = "hangar_lite.storedInHangarLite";
	
	public static final String VAR_OWNER = "player_structure.owner";
	public static final String VAR_OWNER_FACTION = "player_structure.faction";
	public static final String VAR_ENTER_LIST = "player_structure.enter.enterList";
	public static final String VAR_BAN_LIST = "player_structure.ban.banList";
	public static final String VAR_ADMIN_LIST = "player_structure.admin.adminList";
	public static final String VAR_VENDOR_LIST = "player_structure.vendor.vendorList";
	public static final String VAR_HOPPER_LIST = "player_structure.hopper.hopperList";
	public static final String VAR_CIVIC = "player_structure.civic";
	public static final String VAR_CITY_RANK = "player_structure.city_rank";
	public static final String VAR_CITY_COST = "player_structure.city_cost";
	public static final String VAR_MAINTENANCE_MOD = "player_structure.maintanence.mod";
	public static final String VAR_DECAY_RATE = "player_structure.maintanence.decay";
	public static final String VAR_LAST_MAINTANENCE = "player_structure.last_maintanence";
	public static final String VAR_CONDITION = "player_structure.condition";
	public static final String VAR_MAX_CONDITION = "player_structure.max_condition";
	public static final String VAR_BASE_OBJECT = "player_structure.base_objects";
	public static final String VAR_MAX_EXTRACTION = "player_structure.maxExtraction";
	public static final String VAR_CURRENT_EXTRACTION = "player_structure.currentExtraction";
	public static final String VAR_MAX_HOPPER = "player_structure.maxHopper";
	public static final String VAR_VERSION = "player_structure.version";
	public static final String VAR_IS_GUILD_HALL = "player_structure.is_guild_hall";
	public static final String VAR_LAST_MESSAGE = "player_structure.last_message";
	public static final String VAR_PERMISSIONS_CONVERTED = "player_structure.permissions_converted";
	public static final String VAR_ABANDONED = "player_structure.abandoned";
	public static final String OBJVAR_HANGAR_CREATED = "hangar.created";
	
	public static final String VAR_MAINTENANCE_MODIFIERS = "player_structure.maintanence.modifiers";
	public static final String VAR_MAINTENANCE_MOD_FACTORY = VAR_MAINTENANCE_MODIFIERS + ".factory";
	public static final String VAR_MAINTENANCE_MOD_HARVESTER = VAR_MAINTENANCE_MODIFIERS + ".harvester";
	public static final String VAR_MAINTENANCE_MOD_MERCHANT = VAR_MAINTENANCE_MODIFIERS + ".merchant";
	public static final String VAR_MAINTENANCE_MOD_TAX = VAR_MAINTENANCE_MODIFIERS + ".tax";
	
	public static final String VAR_POWER_MOD_FACTORY = "player_structure.power.modifiers.factory";
	public static final String VAR_POWER_MOD_HARVESTER = "player_structure.power.modifiers.harvester";
	
	public static final String VAR_SIGN_BASE = "player_structure.sign";
	public static final String VAR_SIGN_ID = VAR_SIGN_BASE + ".id";
	public static final String VAR_SIGN_NAME = VAR_SIGN_BASE + ".name";
	public static final String VAR_SIGN_TYPE = VAR_SIGN_BASE + ".type";
	
	public static final String SCRIPT_TEMPORARY_STRUCTURE = "structure.temporary_structure";
	public static final String VAR_DEED_TEMPLATE = "player_structure.deed.template";
	public static final String VAR_DEED_TEMPLATE_CRC = "player_structure.deed.template_crc";
	public static final String VAR_DEED_BUILDTIME = "player_structure.deed.buildtime";
	public static final String VAR_DEED_SCENE = "player_structure.deed.scene";
	public static final String VAR_DEED_TIMESTAMP = "player_structure.deed.timestamp";
	public static final String VAR_DEED_MAX_EXTRACTION = "player_structure.deed.maxExtractionRate";
	public static final String VAR_DEED_CURRENT_EXTRACTION = "player_structure.deed.currentExtractionRate";
	public static final String VAR_DEED_MAX_HOPPER = "player_structure.deed.maxHopperSize";
	
	public static final String VAR_DEED_SURPLUS_BASE = "player_structure.deed.surplus";
	public static final String VAR_DEED_SURPLUS_POWER = "player_structure.deed.surplus.power";
	public static final String VAR_DEED_SURPLUS_MAINTENANCE = "player_structure.deed.surplus.maintenance";
	
	public static final String VAR_TEMP_TEMPLATE = "player_structure.construction.template";
	public static final String VAR_TEMP_OWNER = "player_structure.construction.owner";
	public static final String VAR_TEMP_OWNER_NAME = "player_structure.construction.ownerName";
	public static final String VAR_TEMP_OWNER_FACTION = "player_structure.construction.ownerFaction";
	public static final String VAR_TEMP_ROTATION = "player_structure.construction.rotation";
	public static final String VAR_TEMP_PLACEMENT_HEIGHT = "player_structure.construction.placementHeight";
	
	public static final String VAR_LOTS_USED = "player_structure.lotsUsed";
	public static final String VAR_RESIDENCE_BUILDING = "player_structure.residence.building";
	public static final String VAR_RESIDENCE_CAN_DECLARE = "player_structure.residence.canDeclare";
	public static final String VAR_CONFIRM_DESTROY = "player_structure.confirmDestroy";
	
	public static final String VAR_IS_PLACED_OBJECT = "player_structure.is_placed_object";
	
	public static final String VAR_WAYPOINT_STRUCTURE = "player_structure.waypoint";
	
	public static final float INSTALLATION_RANGE = 30.0f;
	public static final float CIVIC_INSTALLATION_RANGE = 30.0f;
	public static final float RATE_POWER_MIN = 1f;
	public static final float MERCHANT_SALES_MODIFIER = -0.2f;
	public static final int MIN_RESIDENCE_DURATION = 86400;
	public static final int MAX_LOTS = 10;
	public static final int MAX_LIST_SIZE = 50;
	public static final int MAIL_WARNING_INTERVAL = 86400;
	public static final int TIME_TO_NEXT_PACKUP = 86400;
	
	public static final float HOUSE_PACKUP_FAILURE_TIMER = 0.f;
	public static final float HOUSE_PACKUP_LOCKOUT_TIMER = 300.0f;
	public static final int ARRAY_LENGTH_FOR_HOUSE_PACKUP = 2;
	
	public static final String SCRIPTVAR_HOUSE_PACKUP_TREE_PREFIX = "packup_structure";
	public static final String SCRIPTVAR_HOUSE_PACKUP_LOCKOUT_TIME = SCRIPTVAR_HOUSE_PACKUP_TREE_PREFIX + ".time_started";
	
	public static final String HOUSE_PACKUP_ARRAY_OBJVAR = "housePackup";
	public static final String HOUSE_PACKUP_DAILY_TALLY_OBJVAR = "dailyHousePackup";
	public static final String HOUSE_PACKUP_DAILY_TIMER_OBJVAR = "maxHousePackupTimer";
	
	public static final string_id SID_LOCKOUT_MESSAGE = new string_id("player_structure", "house_lockout_remaining");
	
	public static final String[] SIGN_TYPE =
	{
		"object/tangible/sign/player/house_address.iff",
		"object/tangible/sign/player/shop_sign_s01.iff",
		"object/tangible/sign/player/shop_sign_s02.iff",
		"object/tangible/sign/player/shop_sign_s03.iff",
		"object/tangible/sign/player/shop_sign_s04.iff",
		"object/tangible/sign/player/house_address_naboo.iff",
		"object/tangible/sign/player/house_address_corellia.iff",
		"object/tangible/sign/player/house_address_tatooine.iff",
	};
	public static final string_id[] SIGN_NAMES =
	{
		new string_id("player_structure","house_address"),
		new string_id("player_structure","shop_sign1"),
		new string_id("player_structure","shop_sign2"),
		new string_id("player_structure","shop_sign3"),
		new string_id("player_structure","shop_sign4")
	};
	
	public static final String MODIFIED_HOUSE_SIGN = "structure.modifiedSign";
	
	public static final String MODIFIED_HOUSE_SIGN_MODEL = "structure.modifiedSignModel";
	
	public static final String SPECIAL_SIGN = "structure.special_sign.hasSpecialSign";
	public static final String SPECIAL_SIGN_TEMPLATE = "structure.special_sign.specialSignTemplate";
	public static final String SPECIAL_SIGN_SKILLMOD = "structure.special_sign.specialSignSkillMod";
	public static final String SPECIAL_SIGN_DECREMENT_MOD = "structure.special_sign.specialSignDecrementSkillMod";
	public static final String SPECIAL_SIGN_OWNER_ONLY = "structure.special_sign.specialSignOwnerOnly";
	
	public static final String VAR_SPECIAL_SIGN_TREE = "special_sign";
	
	public static final String VAR_SPECIAL_SIGN_LIST = VAR_SPECIAL_SIGN_TREE+".sign_list";
	public static final String VAR_SPECIAL_SIGN_NAMES = VAR_SPECIAL_SIGN_TREE+".sign_name";
	public static final String VAR_SPECIAL_SIGN_MENU = VAR_SPECIAL_SIGN_TREE+".sign_menu";
	public static final String VAR_SPECIAL_SIGN_MENU_PID = VAR_SPECIAL_SIGN_TREE+".sign_menu_pid";
	
	public static final string_id SID_MAIL_STRUCTURE_DAMAGE_CONDEMN = new string_id("player_structure", "mail_structure_damage_condemn");
	public static final string_id SID_MAIL_STRUCTURE_DAMAGE_SUB = new string_id("player_structure", "mail_structure_damage_sub");
	public static final string_id SID_MAIL_STRUCTURE_DAMAGE = new string_id("player_structure", "mail_structure_damage");
	public static final string_id SID_BUILD_NO_CITY = new string_id("city/city", "build_no_city");
	public static final string_id SID_RANK_REQ = new string_id("city/city", "rank_req");
	public static final string_id SID_NO_RIGHTS = new string_id("player_structure", "no_rights");
	public static final string_id SID_CIVIC_COUNT = new string_id("city/city", "civic_count");
	public static final string_id CANNOT_PLACE_MINEFIELD = new string_id("player_structure", "cannot_place_minefield");
	public static final string_id CANNOT_PLACE_TURRET = new string_id("player_structure", "cannot_place_turret");
	public static final string_id SID_CANNOT_BUILD_GARAGE_TOO_CLOSE = new string_id("player_structure", "cannot_build_garage_too_close");
	
	public static final string_id SID_SUI_CONFIRM_PLACEMENT_TITLE = new string_id("player_structure", "sui_confirm_placement_title");
	public static final string_id SID_SUI_CONFIRM_PLACEMENT_PROMPT = new string_id("player_structure", "sui_confirm_placement_prompt");
	public static final string_id SID_NO_FREE_TRIAL = new string_id("player_structure", "no_free_trial");
	
	public static final string_id PACKUP_EMAIL_TITLE = new string_id("spam", "packup_email_title");
	public static final string_id PACKUP_EMAIL_BODY = new string_id("spam", "packup_email_body");
	public static final string_id SELF_PACKUP_EMAIL_TITLE = new string_id("spam", "packup_email_title_self");
	public static final string_id SELF_PACKUP_EMAIL_BODY = new string_id("spam", "packup_email_body_self");
	
	public static final string_id CANT_APPLY_SKILLMOD = new string_id("base_player", "cant_use_item");
	public static final string_id SID_SPECIAL_SIGN_FAILED = new string_id("base_player", "special_sign_failed");
	public static final string_id SID_SPECIAL_SIGN_SUCCESS = new string_id("base_player", "special_sign_success");
	public static final string_id SID_SPECIAL_SIGN_REVERT_SUCCESS = new string_id("base_player", "special_sign_revert_success");
	public static final string_id SID_SPECIAL_SIGN_REMOVED = new string_id("base_player", "special_sign_removed");
	public static final string_id SID_SPECIAL_SIGN_UI_TITLE = new string_id("base_player", "special_sign_ui_title");
	public static final string_id SID_SPECIAL_SIGN_UI_PROMPT = new string_id("base_player", "special_sign_ui_prompt");
	public static final string_id SID_REMOVE_SPCIAL_SIGN_TO_DESTROY = new string_id("player_structure", "remove_sign_before_structure_destroy");
	public static final string_id SID_SPECIAL_SIGN_NO_TRANSFER = new string_id("player_structure", "remove_sign_before_structure_transfer");
	public static final string_id SID_HALLOWEEN_SIGN_UPDATED = new string_id("player_structure", "halloween_sign_updated");
	public static final string_id SID_ONLY_OWNER_CAN_REMOVE = new string_id("player_structure", "only_owner_can_remove_sign");
	
	public static final String STF_FILE = "player_structure";
	public static final String SELFPOWERED_DEED = "object/tangible/veteran_reward/harvester.iff";
	public static final String SELFPOWERED_DEED_ELITE = "object/tangible/veteran_reward/harvester_elite.iff";
	
	public static final int MAX_BASE_COUNT = 3;
	
	public static final int HARVESTER_MAX_EXTRACTION_RATE = 70;
	public static final int HARVESTER_MAX_HOPPER_SIZE = 600000;
	
	public static final int DESTROY_RESULT_SUCCESS = 0;
	public static final int DESTROY_RESULT_SUCCESS_SELFPOWERED = 1;
	public static final int DESTROY_RESULT_FAIL_COULD_NOT_CREATE_DEED = 2;
	public static final int DESTROY_RESULT_FAIL_COULD_NOT_DETERMINE_DEED_FOR_STRUCTURE = 3;
	public static final int DESTROY_RESULT_FAIL_TARGET_CONTAINER_FULL = 4;
	public static final int DESTROY_RESULT_FAIL_TARGET_CONTAINER_FULL_SELFPOWERED = 5;
	public static final int DESTROY_RESULT_INVALID_STRUCTURE = 6;
	public static final int DESTROY_RESULT_INVALID_STATIC_ITEM = 7;
	public static final int DESTROY_RESULT_INVALID_CONTAINER = 8;
	
	public static final int PLAYER_STRUCTURE_PRE_ABANDONED = 2;
	public static final int PLAYER_STRUCTURE_ABANDONED = 1;
	public static final int MAX_PACKUP_PER_DAY = 5;
	public static final int NUM_NEEDED_PACKUP_FIRST_BADGE = 10;
	public static final int NUM_NEEDED_PACKUP_SECOND_BADGE = 50;
	public static final int NUM_NEEDED_PACKUP_THIRD_BADGE = 100;
	
	public static final String ABANDONED_TEXT = "\\#ff0000 (Abandoned)\\#.";
	public static final String CITY_ABANDONED_TEXT = "\\#ff0000 (Zoning Violation)\\#.";
	
	public static final string_id MSG_STORAGE_OVER_CAP = new string_id(STF_FILE, "msg_over_cap");
	public static final String OBJVAR_STRUCTURE_STORAGE_INCREASE = "structureChange.storageIncrease";
	public static final String OBJVAR_STORAGE_AMOUNT = "increaseAmount";
	public static final int STORAGE_AMOUNT_CAP = 5000;
	public static final int STORAGE_AMOUNT_DECREASE_UNIT = 100;
	public static final String NON_GENERIC_STORAGE_ITEM_OBJVAR = "nonGenericStorageType";
	
	public static final String TCG_STATIC_ITEM_DEED_OBJVAR = "tcg.static_item_deed";
	
	public static final String VAR_CITY_ABANDONED = "player_structure.city_abandoned";
	public static final int PLAYER_STRUCTURE_CITY_ABANDONED = 1;
	
	public static final int HOUSE_ITEMS_SEARCH_LOCKOUT = 5*60;
	
	public static final string_id SID_NOT_CITY_ABANDONED = new string_id("city/city", "not_city_abandoned");
	public static final string_id SID_NO_CITY = new string_id("city/city", "no_city_during_packup");
	public static final string_id SID_GENERIC_CITY_PACKUP_ERROR = new string_id("city/city", "city_packup_generic_fail");
	public static final string_id SID_MAYOR_PROTECTED = new string_id("city/city", "city_packup_mayor_protected");
	
	
	public static obj_id createPlayerStructure(String template, obj_id owner, location loc, int rotation, dictionary deed_info) throws InterruptedException
	{
		
		if (template == null)
		{
			
			return null;
		}
		if (!isIdValid(owner))
		{
			
			return null;
		}
		if (loc == null)
		{
			
			return null;
		}
		
		debugSpeakMsg(owner, "I am making my house at "+ loc);
		
		if (!canPlaceGarage(loc, 120f, template))
		{
			sendSystemMessage(owner, new string_id(STF_FILE, "proximity_build_failed"));
			return null;
		}
		
		obj_id structure = createObject(template, loc);
		if (structure == null || structure == obj_id.NULL_ID)
		{
			
			return null;
		}
		
		float rot_float = (float)(90 * rotation);
		setYaw(structure, rot_float);
		
		persistObject(structure);
		
		initializeStructure(structure, owner, deed_info);
		
		dictionary outparams = new dictionary();
		outparams.put( "rotation", rotation );
		messageTo( structure, "createStructureObjects", outparams, 10.f, false );
		
		if (!isBuilding( structure ))
		{
			
			if (!isCivic( structure ))
			{
				attachScript( structure, SCRIPT_TERMINAL_STRUCTURE );
			}
		}
		
		dictionary params = new dictionary();
		params.put("structure_name", getNameStringId(structure));
		messageTo(owner, "OnConstructionComplete", params, 30.0f, true);
		
		CustomerServiceLog("playerStructure","Structure "+ structure + "("+ getName(structure) + ") placed by "+ owner + " at location "+ loc);
		
		float ejectRange = dataTableGetFloat(PLAYER_STRUCTURE_DATATABLE, template, DATATABLE_COL_EJECT_RANGE);
		if (ejectRange > 0f)
		{
			dictionary ejectmsg = new dictionary();
			ejectmsg.put("sender", structure);
			messageToRange(ejectRange, "handleEjection", ejectmsg, 1);
		}
		
		return structure;
	}
	
	
	public static obj_id createTemporaryStructure(obj_id deed, obj_id owner, location loc, int rot, float placementHeight) throws InterruptedException
	{
		
		if (!isIdValid(deed))
		{
			return null;
		}
		
		if (!isIdValid(owner))
		{
			return null;
		}
		
		if (loc == null)
		{
			return null;
		}
		
		String cityName = getStringObjVar( deed, "cityName");
		String template = getDeedTemplate(deed);
		String deed_template = getTemplateName(deed);
		int build_time = getDeedBuildTime(deed);
		int time_stamp = getGameTime();
		String owner_name = getPlayerName(owner);
		String scene = getDeedScene(deed);
		int max_extraction = -1;
		int current_extraction = -1;
		int max_hopper = -1;
		
		if (hasObjVar(deed, VAR_DEED_MAX_EXTRACTION))
		{
			max_extraction = getIntObjVar(deed, VAR_DEED_MAX_EXTRACTION);
		}
		
		if (hasObjVar(deed, VAR_DEED_CURRENT_EXTRACTION))
		{
			current_extraction = getIntObjVar(deed, VAR_DEED_CURRENT_EXTRACTION);
		}
		
		if (hasObjVar(deed, VAR_DEED_MAX_HOPPER))
		{
			max_hopper = getIntObjVar(deed, VAR_DEED_MAX_HOPPER);
		}
		
		int storageIncrease = 0;
		
		if (hasObjVar(deed, "structureChange.storageIncrease"))
		{
			storageIncrease = getIntObjVar(deed, "structureChange.storageIncrease");
		}
		
		float surplusPower = getFloatObjVar(deed, VAR_DEED_SURPLUS_POWER);
		int maintPool = getIntObjVar(deed, VAR_DEED_SURPLUS_MAINTENANCE);
		
		if (maintPool > 0)
		{
			CustomerServiceLog("playerStructure", "Deed "+ deed + " with suplus maintenance used. Amount: "+ maintPool + ". Owner is %TU.", owner, null);
		}
		
		if (template == null)
		{
			
			return null;
		}
		
		if (build_time == -1)
		{
			
			return null;
		}
		
		if (!destroyObject(deed))
		{
			
			return null;
		}
		
		int idx = getStructureTableIndex(template);
		
		if (idx == -1)
		{
			return null;
		}
		
		String temp_template = dataTableGetString(PLAYER_STRUCTURE_DATATABLE, idx, DATATABLE_COL_TEMP_STRUCTURE);
		
		loc.y = getHeightAtLocation(loc.x, loc.z);
		obj_id structure = createObject(temp_template, loc);
		
		if (!isIdValid(structure))
		{
			
			structure = createObject("object/building/player/construction_structure.iff", loc);
			
			if (!isIdValid(structure))
			{
				return null;
			}
		}
		
		persistObject(structure);
		
		float rot_fl = (float)(rot * 90);
		setYaw(structure, rot_fl);
		
		setObjVar(structure, VAR_DEED_BUILDTIME, build_time);
		setObjVar(structure, VAR_DEED_TIMESTAMP, time_stamp);
		
		String structure_name = localize(getNameStringId(structure));
		addStructure(structure, template, structure_name, loc, owner, owner_name, true, true);
		
		int maintenance_mod = getSkillStatMod(owner, "structure_maintenance_mod");
		
		int factory_mod = getSkillStatMod(owner, "factory_efficiency");
		
		dictionary deed_info = new dictionary();
		deed_info.put("template", deed_template);
		deed_info.put("build_time", build_time);
		deed_info.put("owner_name", owner_name);
		
		if (storageIncrease > 0)
		{
			deed_info.put("structure_storage", storageIncrease);
		}
		
		if (surplusPower > 0)
		{
			deed_info.put("power", surplusPower);
		}
		
		if (maintPool > 0)
		{
			deed_info.put("maintenance_pool", maintPool);
		}
		
		setObjVar(structure, VAR_TEMP_TEMPLATE, template);
		setObjVar(structure, VAR_TEMP_OWNER, owner);
		setObjVar(structure, VAR_TEMP_OWNER_NAME, owner_name);
		setObjVar(structure, VAR_TEMP_ROTATION, rot);
		setObjVar(structure, VAR_TEMP_PLACEMENT_HEIGHT, placementHeight);
		
		setDeedTemplate(structure, deed_template);
		
		setObjVar(structure, VAR_DEED_BUILDTIME, build_time);
		setObjVar(structure, VAR_DEED_SCENE, scene);
		
		if (cityName != null && !cityName.equals(""))
		{
			setObjVar( structure, "cityName", cityName );
			deed_info.put( "cityName", cityName );
		}
		
		String owner_faction = factions.getFaction(owner);
		
		if (owner_faction != null && !owner_faction.equals(""))
		{
			deed_info.put("owner_faction", owner_faction);
			setObjVar(structure, VAR_TEMP_OWNER_FACTION, owner_faction);
		}
		
		if (scene != null)
		{
			deed_info.put("scene", scene);
		}
		
		if (max_extraction != -1)
		{
			deed_info.put("max_extraction", max_extraction);
			
			setObjVar(structure, VAR_DEED_MAX_EXTRACTION, max_extraction);
		}
		
		if (current_extraction != -1)
		{
			deed_info.put("current_extraction", current_extraction);
			
			setObjVar(structure, VAR_DEED_CURRENT_EXTRACTION, current_extraction);
		}
		
		if (max_hopper != -1)
		{
			deed_info.put("max_hopper", max_hopper);
			
			setObjVar(structure, VAR_DEED_MAX_HOPPER, max_hopper);
		}
		
		if (maintenance_mod > 0)
		{
			deed_info.put("maintenance_mod", maintenance_mod);
		}
		
		if (factory_mod > 0)
		{
			deed_info.put("factory_mod", factory_mod);
		}
		
		if (hasCommand(owner, "place_merchant_tent"))
		{
			deed_info.put("merchant_mod", MERCHANT_SALES_MODIFIER);
		}
		
		dictionary params = new dictionary();
		params.put("template", template);
		params.put("owner", owner);
		params.put("deed_info", deed_info);
		params.put("buildTime", build_time);
		params.put("rotation", rot);
		params.put("timestamp", time_stamp);
		params.put("placementHeight", placementHeight);
		messageTo(structure, "OnBuildingComplete", params, 5.0f, true);
		
		return structure;
	}
	
	
	public static boolean createStructureObjects(obj_id structure, int rotation) throws InterruptedException
	{
		if (structure == obj_id.NULL_ID)
		{
			
			return false;
		}
		
		String sceneName = getCurrentSceneName();
		String structureTemplate = getTemplateName(structure);
		int numItems = 0;
		
		if (dataTableOpen(PLAYER_STRUCTURE_DATATABLE))
		{
			numItems = dataTableGetNumRows(PLAYER_STRUCTURE_DATATABLE);
		}
		else
		{
			
			return false;
		}
		
		if (numItems == 0)
		{
			
			return false;
		}
		
		boolean object_set = false;
		boolean object_mode = false;
		obj_id object = obj_id.NULL_ID;
		Vector base_object_list = new Vector();
		base_object_list.setSize(0);
		for (int i = 0; i < numItems; i++)
		{
			testAbortScript();
			String buildingTemplate = dataTableGetString(PLAYER_STRUCTURE_DATATABLE, i, DATATABLE_COL_STRUCTURE);
			
			if (buildingTemplate.equals(structureTemplate))
			{
				object_mode = true;
				continue;
			}
			
			if (object_mode)
			{
				
				if (buildingTemplate.length() != 0)
				{
					break;
				}
				
				dictionary item = dataTableGetRow(PLAYER_STRUCTURE_DATATABLE, i);
				String TEMPLATE = item.getString(DATATABLE_COL_OBJECT);
				
				float X = item.getFloat(DATATABLE_COL_X);
				float Y = item.getFloat(DATATABLE_COL_Y);
				float Z = item.getFloat(DATATABLE_COL_Z);
				float HEADING = item.getFloat(DATATABLE_COL_HEADING);
				
				String CELL_NAME = item.getString(DATATABLE_COL_CELL);
				
				obj_id CELL_ID = obj_id.NULL_ID;
				location s_loc = getLocation(structure);
				
				if (CELL_NAME.equals(WORLD_DELTA))
				{
					
					if (rotation != 0)
					{
						
						float[] delta_trans = transformDeltaWorldCoord(X, Z, rotation);
						X = delta_trans[0];
						Z = delta_trans[1];
						
						HEADING = HEADING + (float)(rotation * 90);
						if (HEADING > 360)
						{
							HEADING = HEADING - 360;
						}
						
					}
					location obj_loc = new location(s_loc.x - X, s_loc.y - Y, s_loc.z - Z, sceneName, obj_id.NULL_ID);
					
					object = createObject(TEMPLATE, obj_loc);
					if (isCivic( structure ))
					{
						setOwner( object, structure );
					}
					
				}
				else
				{
					CELL_ID = getCellId(structure, CELL_NAME);
					if (CELL_ID == null || CELL_ID == obj_id.NULL_ID)
					{
						
						continue;
					}
					else
					{
						location spot = new location(X, Y, Z, sceneName, CELL_ID);
						object = createObjectInCell(TEMPLATE, structure, CELL_NAME, spot);
						
					}
				}
				if ((object != null) && (object != obj_id.NULL_ID))
				{
					
					setYaw(object, HEADING);
					
					base_object_list = utils.addElement(base_object_list, object);
					
					object_set = true;
					
					object = obj_id.NULL_ID;
				}
			}
			
		}
		if (!object_set)
		{
			
			return false;
		}
		else
		{
			
			if (base_object_list.size() > 0)
			{
				
				setObjVar(structure, VAR_BASE_OBJECT, base_object_list, resizeableArrayTypeobj_id);
				return true;
				
			}
			else
			{
				
				return false;
			}
		}
	}
	
	
	public static boolean initializeStructure(obj_id structure, obj_id owner, dictionary deed_info) throws InterruptedException
	{
		if (structure == obj_id.NULL_ID)
		{
			return false;
		}
		
		if (owner == obj_id.NULL_ID)
		{
			return false;
		}
		
		LOG("player_structure","deed_info = "+ deed_info.toString());
		
		String template = getTemplateName(structure);
		
		String owner_name = getPlayerName(owner);
		
		if (owner_name == null)
		{
			
			return false;
		}
		
		String[] adminList = new String[1];
		adminList[0] = owner.toString();
		
		int decay_rate = 10;
		int condition = 1000;
		int civic = 0;
		int city_rank = 0;
		int city_cost = 0;
		float power_value = 0f;
		float power_rate = getBasePowerRate(structure);
		int items_stored = 0;
		int idx = getStructureTableIndex(template);
		
		if (idx == -1)
		{
			
		}
		else
		{
			decay_rate = dataTableGetInt(PLAYER_STRUCTURE_DATATABLE, idx, DATATABLE_COL_DECAY_RATE);
			condition = dataTableGetInt(PLAYER_STRUCTURE_DATATABLE, idx, DATATABLE_COL_CONDITION);
			civic = dataTableGetInt(PLAYER_STRUCTURE_DATATABLE, idx, DATATABLE_COL_CIVIC);
			city_rank = dataTableGetInt(PLAYER_STRUCTURE_DATATABLE, idx, DATATABLE_COL_CITY_RANK);
			city_cost = dataTableGetInt(PLAYER_STRUCTURE_DATATABLE, idx, DATATABLE_COL_CITY_COST);
		}
		
		int version = dataTableGetInt(PLAYER_STRUCTURE_DATATABLE, idx, DATATABLE_COL_VERSION);
		
		int got = getGameObjectType(structure);
		
		setOwner(structure, owner);
		setObjVar(structure, VAR_OWNER, owner_name);
		
		String owner_faction = deed_info.getString("owner_faction");
		
		if (owner_faction != null && !owner_faction.equals(""))
		{
			factions.setFaction(structure, owner_faction);
		}
		
		String cityName = deed_info.getString("cityName");
		
		if (cityName != null && !cityName.equals(""))
		{
			setObjVar( structure, "cityName", cityName );
		}
		
		setObjVar(structure, VAR_CIVIC, civic);
		
		if (city_rank > 0)
		{
			setObjVar(structure, VAR_CITY_RANK, city_rank);
		}
		
		if (city_cost > 0)
		{
			setObjVar(structure, VAR_CITY_COST, city_cost);
		}
		
		setObjVar(structure, VAR_ADMIN_LIST, adminList);
		permissionsAddAllowed(structure, owner_name);
		setObjVar(structure, VAR_DECAY_RATE, decay_rate);
		
		if (condition > 0)
		{
			setMaxHitpoints(structure, condition);
			setInvulnerableHitpoints(structure, condition);
		}
		
		setObjVar(structure, VAR_VERSION, version);
		
		if (isBuilding(structure))
		{
			
			setObjVar(structure, "healing.canhealwound", 1);
			setObjVar(structure, "healing.canhealshock", 1);
			setObjVar(structure, "safeLogout", 1);
		}
		
		if (isInstallation(structure) && got != GOT_installation_turret)
		{
			String[] hopperList = new String[1];
			hopperList[0] = owner.toString();
			setObjVar(structure, VAR_HOPPER_LIST, hopperList);
			
			if (got == GOT_installation_factory || got == GOT_installation_harvester)
			{
				if (deed_info.containsKey("power"))
				{
					setPowerValue(structure, deed_info.getFloat("power"));
				}
				else
				{
					setPowerValue(structure, 0.0f);
				}
				
				power_rate = expertiseModifyPowerRate(structure, owner, power_rate);
				
				if (power_rate < 0)
				{
					power_rate = 0;
				}
				
				setPowerRate(structure, power_rate);
			}
		}
		
		setDeedTemplate(structure, deed_info.getString("template"));
		
		setObjVar(structure, VAR_DEED_BUILDTIME, deed_info.getInt("build_time"));
		setObjVar(structure, VAR_DEED_SCENE, deed_info.getString("scene"));
		
		if (deed_info.containsKey("current_extraction") && (isHarvester(structure) || isGenerator(structure)))
		{
			int expertiseExtractionIncrease = (int)getSkillStatisticModifier(owner, "expertise_harvester_collection_increase");
			
			int currentExtraction = deed_info.getInt("current_extraction");
			int maxExtraction = deed_info.getInt("max_extraction");
			
			setObjVar(structure, "current_extraction", currentExtraction);
			setObjVar(structure, "max_extraction", maxExtraction);
			
			if (expertiseExtractionIncrease > 0)
			{
				currentExtraction += (int)(currentExtraction * (float)(expertiseExtractionIncrease / 100.0f));
				maxExtraction += (int)(maxExtraction * (float)(expertiseExtractionIncrease / 100.0f));
			}
			
			if (currentExtraction > HARVESTER_MAX_EXTRACTION_RATE)
			{
				currentExtraction = HARVESTER_MAX_EXTRACTION_RATE;
			}
			
			if (maxExtraction > HARVESTER_MAX_EXTRACTION_RATE)
			{
				maxExtraction = HARVESTER_MAX_EXTRACTION_RATE;
			}
			
			setCurrentExtractionRate(structure, currentExtraction);
			
			setMaxExtractionRate(structure, maxExtraction);
		}
		
		if (deed_info.containsKey("max_hopper"))
		{
			int max_hopper = deed_info.getInt("max_hopper");
			
			setObjVar(structure, VAR_DEED_MAX_HOPPER, max_hopper);
			
			if (isHarvester(structure) || isGenerator(structure))
			{
				int expertiseHopperIncrease = (int)getSkillStatisticModifier(owner, "expertise_havester_storage_increase");
				
				if (expertiseHopperIncrease > 0)
				{
					max_hopper += (int)(max_hopper * (float)(expertiseHopperIncrease / 100.0f));
				}
			}
			
			if (max_hopper > HARVESTER_MAX_HOPPER_SIZE)
			{
				max_hopper = HARVESTER_MAX_HOPPER_SIZE;
			}
			
			setMaxHopperAmount(structure, max_hopper);
		}
		
		if (deed_info.containsKey("maintenance_pool"))
		{
			int maintPool = deed_info.getInt("maintenance_pool");
			
			transferBankCreditsFromNamedAccount(money.ACCT_STRUCTURE_DESTROYED, structure, maintPool, "noHandler", "noHandler", new dictionary());
			CustomerServiceLog("playerStructure", "Structure "+ structure + " was created with surplus maintenance. Amount: "+ maintPool + ". Owner is %TU.", owner, null);
		}
		
		if (deed_info.containsKey("merchant_mod") && got != GOT_building_factional)
		{
			setObjVar(structure, VAR_MAINTENANCE_MOD_MERCHANT, deed_info.getFloat("merchant_mod"));
		}
		
		if (deed_info.containsKey("structure_storage"))
		{
			setObjVar(structure, "structureChange.storageIncrease", deed_info.getInt("structure_storage"));
		}
		
		if (civic == 1 || hasObjVar(structure, "force_public"))
		{
			permissionsMakePublic(structure);
		}
		else if (got != GOT_building_factional)
		{
			permissionsMakePrivate(structure);
		}
		
		attachScript(structure, SCRIPT_PERMANENT_STRUCTURE);
		
		dictionary new_params = new dictionary();
		new_params.put("structure", structure);
		new_params.put("structure_name", localize(getNameStringId(structure)));
		new_params.put("structure_loc", getLocation(structure));
		new_params.put("template", template);
		messageTo(owner, "OnAddStructure", new_params, 0, true);
		
		dictionary params = new dictionary();
		int time_stamp = getGameTime();
		setObjVar(structure, VAR_LAST_MAINTANENCE, time_stamp);
		
		if (civic == 0)
		{
			params.put("timestamp", time_stamp);
			LOG("LOG_CHANNEL", "Messaging "+structure+" to run maintenance loop");
			messageTo(structure, "OnMaintenanceLoop", params, 30, false);
		}
		
		return true;
	}
	
	
	public static float getBasePowerRate(obj_id structure) throws InterruptedException
	{
		String template = getTemplateName(structure);
		int idx = getStructureTableIndex(template);
		float power_rate = dataTableGetFloat(PLAYER_STRUCTURE_DATATABLE, idx, DATATABLE_COL_POWER_RATE);
		
		if (power_rate < 0f)
		{
			power_rate = 0;
		}
		
		if (hasObjVar(structure, "selfpowered"))
		{
			power_rate = 0f;
		}
		
		return power_rate;
	}
	
	
	public static float expertiseModifyPowerRate(obj_id structure, obj_id owner, float power_rate) throws InterruptedException
	{
		if (!isIdValid(structure))
		{
			return power_rate;
		}
		
		if (power_rate <= 0)
		{
			return 0;
		}
		
		int got = getGameObjectType(structure);
		
		int expertisePowerReduction = 0;
		
		if (!isIdValid(owner) || !exists(owner))
		{
			if (got == GOT_installation_factory && hasObjVar(structure, VAR_POWER_MOD_FACTORY))
			{
				expertisePowerReduction = getIntObjVar(structure, VAR_POWER_MOD_FACTORY);
			}
			
			if (got == GOT_installation_harvester && hasObjVar(structure, VAR_POWER_MOD_HARVESTER))
			{
				expertisePowerReduction = getIntObjVar(structure, VAR_POWER_MOD_HARVESTER);
			}
		}
		else
		{
			if (got == GOT_installation_factory)
			{
				expertisePowerReduction = (int)getSkillStatisticModifier(owner, "expertise_factory_energy_decrease");
				
				setObjVar(structure, VAR_POWER_MOD_FACTORY, expertisePowerReduction);
			}
			
			if (got == GOT_installation_harvester)
			{
				expertisePowerReduction = (int)getSkillStatisticModifier(owner, "expertise_harvester_energy_decrease");
				
				setObjVar(structure, VAR_POWER_MOD_HARVESTER, expertisePowerReduction);
			}
		}
		
		if (expertisePowerReduction > 0)
		{
			power_rate -= (int)(power_rate * (float)(expertisePowerReduction / 100.0f));
		}
		
		return power_rate;
	}
	
	
	public static boolean addStructure(obj_id structure, String template, String structure_name, location structure_location, obj_id player, String player_name, boolean struct_loaded, boolean player_loaded) throws InterruptedException
	{
		if (structure == null || structure == obj_id.NULL_ID)
		{
			return false;
		}
		
		if (player == null || player == obj_id.NULL_ID)
		{
			return false;
		}
		
		String fp_template = getFootprintTemplate( template );
		if (fp_template == null || fp_template.equals(""))
		{
			return false;
		}
		
		if (player_loaded)
		{
			
			int idx = getStructureTableIndex( template );
			if (idx == -1)
			{
				return false;
			}
			
			int ignore_lots = dataTableGetInt( PLAYER_STRUCTURE_DATATABLE, idx, DATATABLE_COL_NO_LOT_REQ );
			if (ignore_lots != 1)
			{
				int struct_lots = (getNumberOfLots(fp_template) / 4) - dataTableGetInt( PLAYER_STRUCTURE_DATATABLE, idx, DATATABLE_COL_LOT_REDUCTION );
				if (struct_lots < 1)
				{
					struct_lots = 1;
				}
				
				flagConverted( template, structure );
				
				if ((getAccountNumLots(getPlayerObject(player)) + struct_lots) > MAX_LOTS)
				{
					String realStructureTemplate = template;
					if (exists(structure))
					{
						realStructureTemplate = getTemplateName(structure);
					}
					
					if ((realStructureTemplate != null) && (realStructureTemplate.length() > 0) && (realStructureTemplate.indexOf("object/building/player/player_house_") == 0))
					{
						setObjVar(player, "lotOverlimit.structure_id", structure);
						
						if ((structure_name != null) && (structure_name.length() > 0))
						{
							setObjVar(player, "lotOverlimit.structure_name", structure_name);
						}
						
						if (structure_location != null)
						{
							setObjVar(player, "lotOverlimit.structure_location", localize(new string_id("planet_n", structure_location.area)) + " ("+ (int)structure_location.x + ", "+ (int)structure_location.z + ")");
						}
						
						setObjVar(player, "lotOverlimit.violation_time", getCalendarTime());
					}
				}
				
				adjustLotCount( getPlayerObject(player), struct_lots );
			}
		}
		
		if (struct_loaded)
		{
			
			setObjVar(structure, VAR_OWNER, getPlayerName(player));
			setOwner(structure, player);
			if (!isAdmin(structure, getPlayerName(player)))
			{
				modifyList(structure, getPlayerName(player), null, VAR_ADMIN_LIST, false);
			}
		}
		
		if (player_loaded && struct_loaded)
		{
			
		}
		
		city.addStructureToCity( structure );
		
		return true;
	}
	
	
	public static boolean addStructure(obj_id structure, obj_id player) throws InterruptedException
	{
		String template = getTemplateName(structure);
		String structure_name = localize(getNameStringId(structure));
		location structure_location = getWorldLocation(structure);
		String player_name = getPlayerName(player);
		
		return addStructure(structure, template, structure_name, structure_location, player, player_name, true, true);
	}
	
	
	public static boolean removeStructure(obj_id structure, String template, obj_id player, String player_name, boolean struct_loaded, boolean player_loaded) throws InterruptedException
	{
		if (structure == null || structure == obj_id.NULL_ID)
		{
			return false;
		}
		
		if (player == null || player == obj_id.NULL_ID)
		{
			return false;
		}
		
		String fp_template = getFootprintTemplate( template );
		if (fp_template == null || fp_template.equals(""))
		{
			return false;
		}
		
		if (player_loaded)
		{
			
			int idx = getStructureTableIndex( template );
			if (idx == -1)
			{
				return false;
			}
			
			int ignore_lots = dataTableGetInt( PLAYER_STRUCTURE_DATATABLE, idx, DATATABLE_COL_NO_LOT_REQ );
			if (ignore_lots != 1)
			{
				
				int struct_lots = (getNumberOfLots(fp_template) / 4) - dataTableGetInt( PLAYER_STRUCTURE_DATATABLE, idx, DATATABLE_COL_LOT_REDUCTION );
				if (struct_lots < 1)
				{
					struct_lots = 1;
				}
				
				CustomerServiceLog("playerStructure","Structure "+ template + " "+ structure + "("+ ") REMOVED by "+ player + " at location "+ getLocation(player));
				adjustLotCount(getPlayerObject(player), -struct_lots);
				
				int current_lots = getAccountNumLots( getPlayerObject(player) );
				if (current_lots < 0)
				{
					adjustLotCount( getPlayerObject(player), -current_lots );
				}
			}
			
			if (isResidence( structure, player ))
			{
				setHouseId( player, obj_id.NULL_ID );
				removeObjVar( structure, VAR_RESIDENCE_BUILDING );
				
				city.removeCitizen( player, structure );
			}
		}
		
		if (struct_loaded)
		{
			removeObjVar( structure, VAR_OWNER );
			
			if (isAdmin(structure, getPlayerName(player)))
			{
				modifyList( structure, getPlayerName(player), null, VAR_ADMIN_LIST, false );
			}
		}
		
		return true;
	}
	
	
	public static boolean removeStructure(obj_id structure, obj_id player) throws InterruptedException
	{
		String template = getTemplateName(structure);
		String player_name = getPlayerName(player);
		
		return removeStructure( structure, template, player, player_name, true, true );
	}
	
	
	public static boolean updateStructureVersion(obj_id structure) throws InterruptedException
	{
		
		if (!isIdValid(structure))
		{
			
			return false;
		}
		
		String template = getTemplateName(structure);
		
		if (template == null)
		{
			return false;
		}
		
		int idx = getStructureTableIndex(template);
		if (idx == -1)
		{
			
			return false;
		}
		
		float yaw = getYaw(structure);
		if (yaw < 0.0f)
		{
			yaw = yaw + 360.0f;
		}
		
		int rotation = (int)(yaw + 1) / 90;
		
		updateStructureData(structure);
		
		createStructureObjects(structure, rotation);
		
		createStructureSign(structure);
		
		return true;
	}
	
	
	public static boolean updateStructureData(obj_id structure) throws InterruptedException
	{
		
		if (!isIdValid(structure))
		{
			
			return false;
		}
		
		String template = getTemplateName(structure);
		if (template == null)
		{
			return false;
		}
		
		int idx = getStructureTableIndex(template);
		
		if (idx == -1)
		{
			
			return false;
		}
		
		dictionary row = dataTableGetRow(PLAYER_STRUCTURE_DATATABLE, idx);
		int decay_rate = row.getInt(DATATABLE_COL_DECAY_RATE);
		int condition = row.getInt(DATATABLE_COL_CONDITION);
		
		setObjVar(structure, VAR_DECAY_RATE, decay_rate);
		
		float power_rate = 0.0f;
		
		if (!hasObjVar(structure, "selfpowered"))
		{
			power_rate = row.getFloat(DATATABLE_COL_POWER_RATE);
			power_rate = expertiseModifyPowerRate(structure, null, power_rate);
		}
		
		setPowerRate(structure, power_rate);
		
		if (condition > 0)
		{
			int current_hp = getHitpoints(structure);
			int max_hp = getMaxHitpoints(structure);
			
			setMaxHitpoints(structure, condition);
			
			if (current_hp == max_hp)
			{
				setInvulnerableHitpoints(structure, condition);
			}
		}
		
		return true;
	}
	
	
	public static boolean canPlaceStructure(obj_id player, String template, location loc, obj_id deed) throws InterruptedException
	{
		if (isFreeTrialAccount(player))
		{
			sendSystemMessage(player, new string_id(STF_FILE, "no_trial_accounts"));
			return false;
		}
		
		int template_idx = getStructureTableIndex(template);
		LOG("LOG_CHANNEL", "player_structure::canPlaceStructure -- template_idx ->"+ template_idx);
		if (template_idx == -1)
		{
			LOG("LOG_CHANNEL", "player_structure::OnObjectMenuSelect -- Unable to find template on datatable.");
			return false;
		}
		
		String strPlanet = loc.area;
		if (strPlanet == null)
		{
			
			return false;
			
		}
		if ((strPlanet.equals("yavin4"))||(strPlanet.equals("dathomir"))||(strPlanet.equals("endor")))
		{
			int intIndex = template.indexOf("installation");
			if (intIndex < 0)
			{
				sendSystemMessage(player, new string_id(STF_FILE, "cannot_use_deed_here"));
				return false;
			}
		}
		
		if (isIdValid(deed))
		{
			if (hasObjVar( deed, VAR_DEED_SCENE ))
			{
				String scene = getCurrentSceneName();
				String deed_scene = getDeedScene(deed);
				
				boolean match = false;
				java.util.StringTokenizer st = new java.util.StringTokenizer( deed_scene, ",");
				while (st.hasMoreTokens())
				{
					testAbortScript();
					String curScene = st.nextToken();
					if (scene.equals(curScene))
					{
						match = true;
						break;
					}
				}
				if (!match)
				{
					if (!isGod(player))
					{
						sendSystemMessage(player, new string_id(STF_FILE, "cannot_use_deed_here"));
						return false;
					}
					else
					{
						sendSystemMessageTestingOnly(player, "God-Mode Placement Override: Structure is not native to this planet!");
					}
				}
			}
		}
		
		if (hasScript(deed, "faction_perk.minefield.field_deed"))
		{
			sendSystemMessage(player, CANNOT_PLACE_MINEFIELD);
			return false;
		}
		
		if (hasScript(deed, "faction_perk.turret.turret_deed"))
		{
			sendSystemMessage(player, CANNOT_PLACE_TURRET);
			return false;
		}
		
		if (loc.cell != null && loc.cell != obj_id.NULL_ID)
		{
			
			sendSystemMessage(player, new string_id(STF_FILE, "not_inside"));
			return false;
		}
		
		region[] rgnTest = getRegionsWithBuildableAtPoint( loc, regions.BUILD_FALSE );
		if (rgnTest != null)
		{
			sendSystemMessage(player, new string_id(STF_FILE, "not_permitted"));
			return false;
		}
		
		if (!isGameObjectTypeOf (getGameObjectType(deed), GOT_data_house_control_device))
		{
			
			if (!canOwnStructure(template, player, true))
			{
				return false;
			}
		}
		
		if (template.indexOf( "cityhall" ) != -1)
		{
			
			int cities_on_planet = 0;
			String planet_name = getCurrentSceneName();
			int[] city_ids = getAllCityIds();
			for (int i=0; i<city_ids.length; i++)
			{
				testAbortScript();
				location city_loc = cityGetLocation( city_ids[i] );
				if (city_loc.area.equals(planet_name))
				{
					cities_on_planet++;
				}
			}
			
			int max_cities = 0;
			String[] planets = dataTableGetStringColumn( "datatables/city/city_limits.iff", "SCENE");
			for (int i=0; i<planets.length; i++)
			{
				testAbortScript();
				if (planets[i].equals(planet_name))
				{
					max_cities = dataTableGetInt( "datatables/city/city_limits.iff", i, "MAX_CITIES");
					break;
				}
			}
			
			int godLevel = getGodLevel(player);
			if (godLevel < 9)
			{
				if (cities_on_planet >= max_cities)
				{
					sendSystemMessage(player, new string_id(STF_FILE, "max_cities"));
					return false;
				}
			}
			
			int blocking_city = city.canBuildCityHere( player, loc );
			if (blocking_city > -1)
			{
				String block_name = cityGetName( blocking_city );
				location block_loc = cityGetLocation( blocking_city );
				sendSystemMessageProse(player, prose.getPackage(new string_id (STF_FILE, "city_too_close"), block_name));
				return false;
			}
		}
		
		int city_rank_req = dataTableGetInt( PLAYER_STRUCTURE_DATATABLE, template_idx, DATATABLE_COL_CITY_RANK );
		int civic = dataTableGetInt( PLAYER_STRUCTURE_DATATABLE, template_idx, DATATABLE_COL_CIVIC );
		if (city_rank_req > 0)
		{
			int city_id = getCityAtLocation( loc, 0 );
			if (!cityExists( city_id ))
			{
				sendSystemMessage( player, SID_BUILD_NO_CITY );
				return false;
			}
			
			int city_rank = city.getCityRank( city_id );
			if (city_rank < city_rank_req)
			{
				
				prose_package pp = prose.getPackage( SID_RANK_REQ, new string_id( "city/city", "rank"+city_rank_req ), city_rank_req );
				sendSystemMessageProse( player, pp );
				return false;
			}
		}
		
		int city_id = getCityAtLocation( loc, 0 );
		obj_id mayor = cityGetLeader( city_id );
		if (cityExists( city_id ) && city.isCityZoned( city_id ) && isIdValid(player))
		{
			if (!city.hasZoningRights(player, city_id))
			{
				sendSystemMessage( player, SID_NO_RIGHTS );
				return false;
			}
		}
		
		if (civic == 1)
		{
			int civic_count = city.getCivicCount( city_id );
			int max_civic = city.getMaxCivicCount( city_id );
			if (civic_count >= max_civic)
			{
				prose_package pp = prose.getPackage( SID_CIVIC_COUNT, max_civic );
				sendSystemMessageProse( player, pp );
				return false;
			}
		}
		
		return true;
	}
	
	
	public static boolean canPlaceStructure(obj_id player, String template) throws InterruptedException
	{
		location loc = getLocation(player);
		
		return canPlaceStructure(player, template, loc, null);
	}
	
	
	public static boolean canOwnStructure(String template, obj_id player, boolean allowOverLotLimitIfPlayerHouse) throws InterruptedException
	{
		if (player == null || player == obj_id.NULL_ID)
		{
			return false;
		}
		
		int idx = getStructureTableIndex( template );
		if (idx == -1)
		{
			return false;
		}
		
		int ignore_lots = dataTableGetInt( PLAYER_STRUCTURE_DATATABLE, idx, DATATABLE_COL_NO_LOT_REQ );
		if (ignore_lots != 1)
		{
			
			int lots = getAccountNumLots( getPlayerObject(player) );
			int lots_needed = (getNumberOfLots(template) / 4) - dataTableGetInt( PLAYER_STRUCTURE_DATATABLE, idx, DATATABLE_COL_LOT_REDUCTION );
			if (lots_needed < 1)
			{
				lots_needed = 1;
			}
			
			if (lots > MAX_LOTS)
			{
				sendSystemMessageProse(player, prose.getPackage(new string_id (STF_FILE, "not_enough_lots"), lots_needed));
				return false;
			}
			
			if ((lots + lots_needed) > MAX_LOTS)
			{
				if (allowOverLotLimitIfPlayerHouse && (template.indexOf("object/building/player/player_house_") == 0))
				{
					
				}
				else
				{
					sendSystemMessageProse(player, prose.getPackage(new string_id (STF_FILE, "not_enough_lots"), lots_needed));
					return false;
				}
			}
		}
		else if (isFactionPerkBase(template))
		{
			if (hasObjVar(player,"factionBaseCount" ))
			{
				int factionBaseCount = getIntObjVar(player,"factionBaseCount");
				if (factionBaseCount > MAX_BASE_COUNT-1)
				{
					string_id strSpam = new string_id("faction_perk", "not_enough_base_units");
					sendSystemMessage(player, strSpam);
					getClusterWideData("gcw_player_base", "base_cwdata_manager*", true, player);
					return false;
				}
			}
		}
		
		String skill_mod = dataTableGetString( PLAYER_STRUCTURE_DATATABLE, idx, DATATABLE_COL_SKILL_MOD );
		if (skill_mod.length() > 0)
		{
			int skill_mod_value = dataTableGetInt( PLAYER_STRUCTURE_DATATABLE, idx, DATATABLE_COL_SKILL_MOD_VALUE );
			int player_skill = getSkillStatMod( player, skill_mod );
			if (player_skill < skill_mod_value)
			{
				String msg = dataTableGetString( PLAYER_STRUCTURE_DATATABLE, idx, DATATABLE_COL_SKILL_MOD_MESSAGE );
				if ((msg == null) || (msg.equals("")))
				{
					sendSystemMessage(player, new string_id(STF_FILE, "no_skill"));
				}
				else
				{
					string_id msgsid = new string_id( STF_FILE, msg );
					sendSystemMessage( player, msgsid );
				}
				
				return false;
			}
		}
		
		return true;
	}
	
	
	public static boolean canOwnStructure(obj_id structure, obj_id player) throws InterruptedException
	{
		if (structure == null || structure == obj_id.NULL_ID)
		{
			return false;
		}
		
		String template = getTemplateName(structure);
		return canOwnStructure(template, player, false);
	}
	
	
	public static String getFootprintTemplate(String template) throws InterruptedException
	{
		if (template == null || template.equals(""))
		{
			return null;
		}
		
		String fp_template = dataTableGetString(PLAYER_STRUCTURE_DATATABLE, template, DATATABLE_COL_FOOTPRINT);
		LOG("LOG_CHANNEL","retrieved fp_template = "+ fp_template);
		if (fp_template == null || fp_template.equals(""))
		{
			return template;
		}
		
		return fp_template;
	}
	
	
	public static boolean canPlaceObject(obj_id structure, obj_id player) throws InterruptedException
	{
		if (structure == null || structure == obj_id.NULL_ID)
		{
			return false;
		}
		
		if (player == null || player == obj_id.NULL_ID)
		{
			return false;
		}
		
		if (!isAdmin(structure, player))
		{
			
			sendSystemMessage(player, new string_id(STF_FILE, "admin_only"));
			return false;
		}
		
		return true;
	}
	
	
	public static boolean isInAdminRange(obj_id structure, obj_id player) throws InterruptedException
	{
		if (structure == null || structure == obj_id.NULL_ID)
		{
			return false;
		}
		
		if (player == null || player == obj_id.NULL_ID)
		{
			return false;
		}
		
		if (pclib.isContainedByPlayer(player, structure))
		{
			return true;
		}
		
		location loc_structure = getLocation(structure);
		location loc_player = getLocation(player);
		
		obj_id building = getTopMostContainer(loc_player.cell);
		
		if (building == structure)
		{
			return true;
		}
		
		float distance = loc_structure.distance(loc_player);
		
		if (distance < getInstallationRange(structure))
		{
			return true;
		}
		else
		{
			
			sendSystemMessage(player, new string_id(STF_FILE, "too_far"));
			return false;
		}
	}
	
	
	public static float getInstallationRange(obj_id structure) throws InterruptedException
	{
		if (isCivic( structure ))
		{
			return CIVIC_INSTALLATION_RANGE;
		}
		else
		{
			return INSTALLATION_RANGE;
		}
	}
	
	
	public static String getDeedTemplate(obj_id deed) throws InterruptedException
	{
		if (!hasObjVar(deed, VAR_DEED_TEMPLATE))
		{
			if (hasObjVar(deed, VAR_DEED_TEMPLATE_CRC))
			{
				return getObjectTemplateName(getDeedTemplateCrc(deed));
			}
			
			return null;
		}
		
		return getStringObjVar(deed, VAR_DEED_TEMPLATE);
	}
	
	
	public static int getDeedTemplateCrc(obj_id deed) throws InterruptedException
	{
		return getIntObjVar(deed, VAR_DEED_TEMPLATE_CRC);
	}
	
	
	public static boolean setDeedTemplate(obj_id deed, String template) throws InterruptedException
	{
		if (!isIdValid(deed) || template == null || template.equals(""))
		{
			return false;
		}
		
		removeObjVar(deed, VAR_DEED_TEMPLATE);
		return setObjVar(deed, VAR_DEED_TEMPLATE_CRC, getObjectTemplateCrc(template));
	}
	
	
	public static int getDeedBuildTime(obj_id deed) throws InterruptedException
	{
		if (!hasObjVar(deed, VAR_DEED_BUILDTIME))
		{
			return -1;
		}
		
		return getIntObjVar(deed, VAR_DEED_BUILDTIME);
	}
	
	
	public static String getDeedScene(obj_id deed) throws InterruptedException
	{
		if (!hasObjVar(deed, VAR_DEED_SCENE))
		{
			return null;
		}
		
		return getStringObjVar(deed, VAR_DEED_SCENE);
	}
	
	
	public static int getDeedMaxExtraction(obj_id deed) throws InterruptedException
	{
		if (!hasObjVar(deed, VAR_DEED_MAX_EXTRACTION))
		{
			return -1;
		}
		
		return getIntObjVar(deed, VAR_DEED_MAX_EXTRACTION);
	}
	
	
	public static int getDeedCurrentExtraction(obj_id deed) throws InterruptedException
	{
		if (!hasObjVar(deed, VAR_DEED_CURRENT_EXTRACTION))
		{
			return -1;
		}
		
		return getIntObjVar(deed, VAR_DEED_CURRENT_EXTRACTION);
	}
	
	
	public static int getDeedMaxHopper(obj_id deed) throws InterruptedException
	{
		if (!hasObjVar(deed, VAR_DEED_MAX_HOPPER))
		{
			return -1;
		}
		
		return getIntObjVar(deed, VAR_DEED_MAX_HOPPER);
	}
	
	
	public static String getStructureOwner(obj_id structure) throws InterruptedException
	{
		
		return getPlayerName(getOwner(structure));
	}
	
	
	public static obj_id getStructureOwnerObjId(obj_id structure) throws InterruptedException
	{
		return getOwner(structure);
	}
	
	
	public static int getMaintenanceRate(obj_id structure) throws InterruptedException
	{
		int civic = getIntObjVar( structure, VAR_CIVIC );
		if (civic == 1)
		{
			return 0;
		}
		else
		{
			
			int maint = getBaseMaintenanceRate(structure);
			float merchant_mod = getMaintenanceMerchantMod(structure);
			
			if (merchant_mod != 0f)
			{
				maint = Math.round(maint * (1f+merchant_mod));
			}
			
			float factory_mod = getMaintenanceFactoryMod(structure);
			
			if (factory_mod != 0f)
			{
				maint -= Math.round(maint * (factory_mod / 100.0f));
			}
			
			float harvester_mod = getMaintenanceHarvesterMod(structure);
			
			if (harvester_mod != 0f)
			{
				maint -= Math.round(maint * (harvester_mod / 100.0f));
			}
			
			int tax = getMaintenancePropertyTax(structure);
			if (tax > 0)
			{
				maint += tax;
			}
			
			return maint;
		}
	}
	
	
	public static int getBaseMaintenanceRate(obj_id structure) throws InterruptedException
	{
		int idx = getStructureTableIndex( getTemplateName( structure ) );
		return dataTableGetInt( PLAYER_STRUCTURE_DATATABLE, idx, DATATABLE_COL_MAINT_RATE );
	}
	
	
	public static int getRedeedCost(obj_id structure) throws InterruptedException
	{
		int idx = getStructureTableIndex( getTemplateName( structure ) );
		return dataTableGetInt( PLAYER_STRUCTURE_DATATABLE, idx, DATATABLE_COL_REDEED_COST );
	}
	
	
	public static float getMaintenanceMerchantMod(obj_id structure) throws InterruptedException
	{
		if (!isIdValid(structure))
		{
			return 0f;
		}
		
		return getFloatObjVar(structure, VAR_MAINTENANCE_MOD_MERCHANT);
	}
	
	
	public static float getMaintenanceFactoryMod(obj_id structure) throws InterruptedException
	{
		if (!isIdValid(structure))
		{
			return 0f;
		}
		
		return getFloatObjVar(structure, VAR_MAINTENANCE_MOD_FACTORY);
	}
	
	
	public static float getMaintenanceHarvesterMod(obj_id structure) throws InterruptedException
	{
		if (!isIdValid(structure))
		{
			return 0f;
		}
		
		return getFloatObjVar(structure, VAR_MAINTENANCE_MOD_HARVESTER);
	}
	
	
	public static int getMaintenancePropertyTax(obj_id structure) throws InterruptedException
	{
		if (!isIdValid(structure))
		{
			return -1;
		}
		
		int maint = getBaseMaintenanceRate( structure );
		if (maint > 0)
		{
			return Math.round(maint * city.getPropertyTax(structure)/100.f);
		}
		
		return 0;
	}
	
	
	public static int getMaintenancePool(obj_id structure) throws InterruptedException
	{
		return getBankBalance(structure);
	}
	
	
	public static int getDecayRate(obj_id structure) throws InterruptedException
	{
		return getIntObjVar(structure, VAR_DECAY_RATE);
	}
	
	
	public static int getStructureCondition(obj_id structure) throws InterruptedException
	{
		
		return getHitpoints(structure);
	}
	
	
	public static int getMaxCondition(obj_id structure) throws InterruptedException
	{
		
		return getMaxHitpoints(structure);
	}
	
	
	public static int getMaxExtraction(obj_id structure) throws InterruptedException
	{
		if (!hasObjVar(structure, VAR_MAX_EXTRACTION))
		{
			return -1;
		}
		
		return getIntObjVar(structure, VAR_MAX_EXTRACTION);
	}
	
	
	public static int getCurrentExtraction(obj_id structure) throws InterruptedException
	{
		if (!hasObjVar(structure, VAR_CURRENT_EXTRACTION))
		{
			return -1;
		}
		
		return getIntObjVar(structure, VAR_CURRENT_EXTRACTION);
	}
	
	
	public static int getMaxHopper(obj_id structure) throws InterruptedException
	{
		if (!hasObjVar(structure, VAR_MAX_HOPPER))
		{
			return -1;
		}
		
		return getIntObjVar(structure, VAR_MAX_HOPPER);
	}
	
	
	public static int getStructureVersion(obj_id structure) throws InterruptedException
	{
		return getIntObjVar(structure, VAR_VERSION);
	}
	
	
	public static obj_id getResidence(obj_id player) throws InterruptedException
	{
		if (player == null || player == obj_id.NULL_ID)
		{
			return null;
		}
		
		return getHouseId( player );
	}
	
	
	public static int getStructureNumItems(obj_id structure) throws InterruptedException
	{
		if (structure == null || structure == obj_id.NULL_ID)
		{
			return -1;
		}
		
		return getIntObjVar(structure, "itemLimit");
	}
	
	
	public static obj_id[] getStructureBaseObjects(obj_id structure) throws InterruptedException
	{
		if (structure == null || structure == obj_id.NULL_ID)
		{
			return null;
		}
		
		return getObjIdArrayObjVar(structure, VAR_BASE_OBJECT);
	}
	
	
	public static String[] getEntryList(obj_id structure) throws InterruptedException
	{
		if (!isIdValid( structure ))
		{
			return null;
		}
		
		String[] fullEntryList = permissionsGetAllowed(structure);
		
		Vector filteredEntryList = new Vector();
		filteredEntryList.setSize(0);
		for (int i = 0; i < fullEntryList.length; i++)
		{
			testAbortScript();
			String entryItemString = fullEntryList[i];
			obj_id entryItemId = utils.stringToObjId(entryItemString);
			if (isIdValid(entryItemId))
			{
			}
			else
			{
				if (entryItemString.toLowerCase().indexOf("guild:") > -1)
				{
					int guild_id = findGuild(entryItemString.substring(6).toLowerCase().trim());
					if (guild_id > 0)
					{
						utils.addElement(filteredEntryList, entryItemString);
					}
					else
					{
						
					}
				}
				else
				{
					utils.addElement(filteredEntryList, entryItemString);
				}
			}
		}
		
		String[] returnList = new String[0];
		if (filteredEntryList != null)
		{
			returnList = new String[filteredEntryList.size()];
			filteredEntryList.toArray(returnList);
		}
		return returnList;
	}
	
	
	public static String[] getCompleteEntryList(obj_id structure) throws InterruptedException
	{
		if (!isIdValid( structure ))
		{
			return null;
		}
		
		return permissionsGetAllowed(structure);
	}
	
	
	public static boolean isNameOnEntryList(obj_id structure, String player_or_guild_name) throws InterruptedException
	{
		String[] entry_list = getCompleteEntryList(structure);
		if (entry_list != null)
		{
			if (isNameOnPermissionList( entry_list, player_or_guild_name ))
			{
				return true;
			}
		}
		
		return false;
	}
	
	
	public static String[] getBanList(obj_id structure) throws InterruptedException
	{
		if (!isIdValid( structure ))
		{
			return null;
		}
		
		return permissionsGetBanned(structure);
	}
	
	
	public static boolean isNameOnBanList(obj_id structure, String player_or_guild_name) throws InterruptedException
	{
		String[] ban_list = getBanList(structure);
		if (ban_list != null)
		{
			if (isNameOnPermissionList( ban_list, player_or_guild_name ))
			{
				return true;
			}
		}
		
		return false;
	}
	
	
	public static String[] getAdminListRaw(obj_id structure) throws InterruptedException
	{
		if (!isIdValid( structure ))
		{
			return null;
		}
		
		return getStringArrayObjVar(structure, VAR_ADMIN_LIST);
	}
	
	
	public static String[] getAdminListNames(obj_id structure) throws InterruptedException
	{
		return convertRawListToNames( getAdminListRaw(structure) );
	}
	
	
	public static boolean isNameOnAdminList(obj_id structure, String player_or_guild_name) throws InterruptedException
	{
		String[] admin_list = getAdminListRaw(structure);
		if (admin_list != null)
		{
			if (isNameOnRawList( admin_list, player_or_guild_name ))
			{
				return true;
			}
		}
		
		return false;
	}
	
	
	public static boolean adminListAddName(obj_id structure, String nameToAdd) throws InterruptedException
	{
		if (isIdValid( structure ))
		{
			Vector adminList = getResizeableStringArrayObjVar(structure, VAR_ADMIN_LIST);
			
			if (adminList == null)
			{
				adminList = new Vector();
			}
			
			if (addNameToRawList( adminList, nameToAdd ))
			{
				if (adminList.size() > 0)
				{
					setObjVar(structure, VAR_ADMIN_LIST, adminList, resizeableArrayTypeString);
					return true;
				}
			}
		}
		
		return false;
	}
	
	
	public static boolean adminListRemoveName(obj_id structure, String nameToRemove) throws InterruptedException
	{
		if (isIdValid( structure ))
		{
			Vector adminList = getResizeableStringArrayObjVar(structure, VAR_ADMIN_LIST);
			if (removeNameFromRawList( adminList, nameToRemove ))
			{
				if (adminList.size() == 0)
				{
					removeObjVar( structure, VAR_ADMIN_LIST );
				}
				else
				{
					setObjVar(structure, VAR_ADMIN_LIST, adminList, resizeableArrayTypeString);
				}
				return true;
			}
		}
		
		return false;
	}
	
	
	public static String[] getVendorList(obj_id structure) throws InterruptedException
	{
		if (!isIdValid( structure ))
		{
			return null;
		}
		
		String[] vendor_list = new String[0];
		return vendor_list;
	}
	
	
	public static String[] getHopperListRaw(obj_id structure) throws InterruptedException
	{
		if (!isIdValid( structure ))
		{
			return null;
		}
		
		return getStringArrayObjVar(structure, VAR_HOPPER_LIST);
	}
	
	
	public static String[] getHopperListNames(obj_id structure) throws InterruptedException
	{
		return convertRawListToNames( getHopperListRaw(structure) );
	}
	
	
	public static boolean isNameOnHopperList(obj_id structure, String player_or_guild_name) throws InterruptedException
	{
		String[] hopper_list = getHopperListRaw(structure);
		if (hopper_list != null)
		{
			if (isNameOnRawList( hopper_list, player_or_guild_name ))
			{
				return true;
			}
		}
		
		return false;
	}
	
	
	public static boolean hopperListAddName(obj_id structure, String nameToAdd) throws InterruptedException
	{
		if (isIdValid( structure ))
		{
			Vector hopperList = getResizeableStringArrayObjVar(structure, VAR_HOPPER_LIST);
			
			if (hopperList == null)
			{
				hopperList = new Vector();
			}
			
			if (addNameToRawList( hopperList, nameToAdd ))
			{
				if (hopperList.size() > 0)
				{
					setObjVar(structure, VAR_HOPPER_LIST, hopperList, resizeableArrayTypeString);
					return true;
				}
			}
		}
		
		return false;
	}
	
	
	public static boolean hopperListRemoveName(obj_id structure, String nameToRemove) throws InterruptedException
	{
		if (isIdValid( structure ))
		{
			Vector hopperList = getResizeableStringArrayObjVar(structure, VAR_HOPPER_LIST);
			if (removeNameFromRawList( hopperList, nameToRemove ))
			{
				if (hopperList.size() == 0)
				{
					removeObjVar( structure, VAR_HOPPER_LIST );
				}
				else
				{
					setObjVar(structure, VAR_HOPPER_LIST, hopperList, resizeableArrayTypeString);
				}
				return true;
			}
		}
		
		return false;
	}
	
	
	public static obj_id getStructure(obj_id object) throws InterruptedException
	{
		location loc = getLocation(object);
		obj_id structure = getContainedBy(loc.cell);
		if ((structure == null)||(structure == obj_id.NULL_ID))
		{
			
			obj_id obj_owner = getOwner( object );
			if (isInstallation( obj_owner ) && isCivic ( obj_owner ))
			{
				return obj_owner;
			}
			
			if (isPlayer(object))
			{
				structure = getLookAtTarget(object);
				
				if (!isInstallation(structure))
				{
					obj_id target = structure;
					structure = obj_id.NULL_ID;
					
					obj_id[] items = getObjectsInRange(loc, INSTALLATION_RANGE);
					
					if (items == null)
					{
						return null;
					}
					
					for (int i = 0; i < items.length; i++)
					{
						testAbortScript();
						
						if (isInstallation(items[i]))
						{
							
							if (isAdmin(items[i], object))
							{
								
								obj_id[] base_objects = getObjIdArrayObjVar(items[i], "player_structure.base_objects");
								
								if (base_objects != null && base_objects.length > 0)
								{
									
									for (int j = 0; j < base_objects.length; j++)
									{
										testAbortScript();
										
										if (base_objects[j] == target)
										{
											
											return items[i];
										}
									}
									
								}
								
								if ((!isIdValid(structure)) || (getLocation(items[i]).distance(loc) < getLocation(structure).distance(loc)))
								{
									structure = items[i];
								}
							}
						}
					}
				}
			}
		}
		return structure;
	}
	
	
	public static obj_id[] getObjectsInBuilding(obj_id structure) throws InterruptedException
	{
		if (structure == null || structure == obj_id.NULL_ID)
		{
			return null;
		}
		if (!isBuilding(structure))
		{
			return null;
		}
		
		obj_id[] cells = getCellIds(structure);
		Vector objects = new Vector();
		objects.setSize(0);
		if (cells != null)
		{
			for (int i = 0; i < cells.length; i++)
			{
				testAbortScript();
				if (isIdValid(cells[i]))
				{
					obj_id[] cell_contents = getContents(cells[i]);
					if (cell_contents.length > 0)
					{
						objects = utils.concatArrays(objects, cell_contents);
					}
				}
			}
		}
		
		obj_id[] _objects = new obj_id[0];
		if (objects != null)
		{
			_objects = new obj_id[objects.size()];
			objects.toArray(_objects);
		}
		return _objects;
	}
	
	
	public static obj_id[] getPlayersInBuilding(obj_id structure) throws InterruptedException
	{
		if (structure == null || structure == obj_id.NULL_ID)
		{
			return null;
		}
		if (!isBuilding(structure))
		{
			return null;
		}
		
		Vector players = new Vector();
		players.setSize(0);
		obj_id[] objects = getObjectsInBuilding(structure);
		if (objects != null && objects.length > 0)
		{
			for (int i = 0; i < objects.length; i++)
			{
				testAbortScript();
				if (isIdValid(objects[i]) && isPlayer(objects[i]))
				{
					players = utils.addElement(players, objects[i]);
				}
			}
		}
		
		obj_id[] _players = new obj_id[0];
		if (players != null)
		{
			_players = new obj_id[players.size()];
			players.toArray(_players);
		}
		return _players;
	}
	
	
	public static obj_id[] getPlayersNearInstallation(obj_id structure) throws InterruptedException
	{
		if (structure == null || structure == obj_id.NULL_ID)
		{
			return null;
		}
		
		location loc = getLocation(structure);
		Vector players = new Vector();
		players.setSize(0);
		obj_id[] items = getObjectsInRange(loc, getInstallationRange(structure));
		if (items == null)
		{
			obj_id[] _players = new obj_id[0];
			if (players != null)
			{
				_players = new obj_id[players.size()];
				players.toArray(_players);
			}
			return _players;
		}
		
		for (int i = 0; i < items.length; i++)
		{
			testAbortScript();
			if (isPlayer(items[i]))
			{
				players = utils.addElement(players, items[i]);
			}
		}
		
		obj_id[] _players = new obj_id[0];
		if (players != null)
		{
			_players = new obj_id[players.size()];
			players.toArray(_players);
		}
		return _players;
	}
	
	
	public static void confirmNoReclaimPlacement(obj_id deedOrScd, obj_id player, String template) throws InterruptedException
	{
		if (utils.hasScriptVar(deedOrScd, "noreclaim.pid"))
		{
			int oldpid = utils.getIntScriptVar(deedOrScd, "noreclaim.pid");
			obj_id oldplayer = utils.getObjIdScriptVar(deedOrScd, "noreclaim.player");
			
			utils.removeScriptVarTree(deedOrScd, "noreclaim");
			
			if (isIdValid(oldplayer))
			{
				sui.closeSUI(oldplayer, oldpid);
			}
		}
		
		String title = utils.packStringId(SID_SUI_CONFIRM_PLACEMENT_TITLE);
		String prompt = utils.packStringId(SID_SUI_CONFIRM_PLACEMENT_PROMPT);
		
		int pid = sui.msgbox(deedOrScd, player, prompt, sui.YES_NO, title, "handleNoReclaimConfirm");
		if (pid > - 1)
		{
			utils.setScriptVar(deedOrScd, "noreclaim.pid", pid);
			utils.setScriptVar(deedOrScd, "noreclaim.player", player);
			utils.setScriptVar(deedOrScd, "noreclaim.deedOrScd", deedOrScd);
		}
	}
	
	
	public static boolean tryQueueStructurePlacement(obj_id deedOrScd, obj_id player) throws InterruptedException
	{
		String template = player_structure.getDeedTemplate(deedOrScd);
		if (!player_structure.canReclaimDeed(template))
		{
			if (!player_structure.isFactionPerkBase(template))
			{
				confirmNoReclaimPlacement(deedOrScd, player, template);
				return false;
			}
		}
		
		location loc = getLocation(player);
		float range = 200f;
		if (!player_structure.canPlaceGarage(loc, range, template))
		{
			sendSystemMessage(player, SID_CANNOT_BUILD_GARAGE_TOO_CLOSE);
			return false;
		}
		
		if (!player_structure.canPlaceFactionPerkDeed(deedOrScd, player))
		{
			return false;
		}
		
		queueCommand(player, (123886506), deedOrScd, "", COMMAND_PRIORITY_DEFAULT);
		return true;
	}
	
	
	public static boolean tryEnterPlacementMode(obj_id deedOrScd, obj_id player) throws InterruptedException
	{
		obj_id target = deedOrScd;
		if (!isIdValid(target))
		{
			return false;
		}
		
		if ((!isGameObjectTypeOf (getGameObjectType(target), GOT_deed) && !isGameObjectTypeOf (getGameObjectType(target), GOT_data_house_control_device)))
		{
			sendSystemMessage(player, new string_id("player_structure", "not_a_deed"));
			return false;
		}
		
		String template = player_structure.getDeedTemplate(target);
		if (template == null)
		{
			return false;
		}
		
		location here = getLocation(player);
		if (!player_structure.canPlaceStructure(player, template, here, target))
		{
			return false;
		}
		
		String fp_template = player_structure.getFootprintTemplate(template);
		if (!fp_template.equals(template))
		{
			if (!player_structure.canPlaceStructure(player, fp_template, here, null))
			{
				return false;
			}
		}
		
		enterClientStructurePlacementMode(player, target, fp_template);
		
		return true;
	}
	
	
	public static boolean canPackBuilding(obj_id player, obj_id structure) throws InterruptedException
	{
		return !utils.isFreeTrial(player) && !isCivic(structure) && !isHarvester(structure) && !isGenerator(structure) && isOwner(structure, player);
	}
	
	
	public static void packBuilding(obj_id player, obj_id structure) throws InterruptedException
	{
		utils.setScriptVar(player, "packup.structure", structure);
		if (utils.hasScriptVar(player, "packup.suiconfirm"))
		{
			sui.closeSUI(player, utils.getIntScriptVar(player, "packup.suiconfirm"));
		}
		
		String stringId = vendor_lib.isVendorPackUpEnabled() ? "confirm_packup": "confirm_packup_no_vendors";
		if (isInstallation( structure ))
		{
			stringId += "_factory";
		}
		if (!isOwner(structure, player))
		{
			stringId += "_abandoned";
		}
		
		utils.setScriptVar(player, "packup.suiconfirm", sui.msgbox(player, player, utils.packStringId(new string_id("sui", stringId)), sui.YES_NO, "msgConfirmHousePackup"));
		
		return;
	}
	
	
	public static void finalizePackUp(obj_id player, obj_id structure) throws InterruptedException
	{
		blog("player_structure.finalizePackUp init.");
		final boolean isAbandoned = isAbandoned(structure);
		blog("player_structure.finalizePackUp isAbandoned: "+isAbandoned);
		
		obj_id structure_owner = getOwner(structure);
		
		if (structure_owner != player && isAbandoned)
		{
			CustomerServiceLog("housepackup", "Structure ("+ structure + ") was abandoned and was not being packed by Owner (" + getPlayerName(structure_owner) + ") "+ structure_owner + ". A player ( "+ getPlayerName(player)
			+ " is packing this structure. Sending job to the Abandoned Structure Pack Up function." );
			
			finalizeAbandonedStructurePackUp(player, structure);
			return;
		}
		blog("player_structure.finalizePackUp PLAYER IS THE OWNER.");
		
		if (!structure_owner.isLoaded() || !structure_owner.isAuthoritative())
		{
			
			CustomerServiceLog("housepackup", "Structure ("+ structure + ") could not be packed by player/owner (" + getPlayerName(player) + ") OID ("+player+ ") The cause: Player/owner -" + " isLoaded = "+ player.isLoaded()
			+ " isAuthoritative = "+ player.isAuthoritative()
			);
			return;
		}
		
		CustomerServiceLog("housepackup", "Player/Owner "+ getPlayerName(player) +
		" OID ("+ player + ") is attempting to pack structure ("+ structure + ",abandoned=" + isAbandoned + ") Structure is loaded and is Authoritative." );
		
		if (!structure.isLoaded() || !structure.isAuthoritative())
		{
			blog("player_structure.finalizePackUp structure NOT LOADED or NOT AUTHORITATIVE.");
			
			CustomerServiceLog("housepackup", "Structure ("+ structure + ") could not be packed by player/owner (" + getPlayerName(player) + ") OID ("+ player + ") because structure -" + " isLoaded = "+ structure.isLoaded()
			+ " isAuthoritative = "+ structure.isAuthoritative()
			);
			return;
		}
		blog("player_structure.finalizePackUp getting datapad");
		
		obj_id datapad = utils.getPlayerDatapad(player);
		if (!isIdValid(datapad))
		{
			blog("player_structure.finalizePackUp datapad not valid.");
			
			CustomerServiceLog("housepackup", "Structure ("+ structure + ") could not be packed by player/owner (" + getPlayerName(player) + ") - datapad was not valid" );
			return;
		}
		
		blog("player_structure.finalizePackUp creating SCD");
		
		obj_id scd = createObjectOverloaded("object/intangible/house/generic_house_control_device.iff", datapad);
		if (!isIdValid(scd))
		{
			CustomerServiceLog("housepackup", "Structure ("+ structure + ") could not be packed by player/owner (" + getPlayerName(player) + ") - scd was not valid" );
			return;
		}
		attachScript(scd, "structure.house_control_device");
		
		if (player_structure.isFactory(structure) && isHarvesterActive(structure))
		{
			CustomerServiceLog("housepackup", "Structure ("+ structure + ") was a factory that was active. Turning off factory for player/owner (" + getPlayerName(player) + ") OID ("+ player + ")" );
			deactivate(structure);
		}
		
		obj_id[] players = getPlayersInBuilding(structure);
		if (players != null)
		{
			blog("player_structure.finalizePackUp Players are in the building that is about to be packed.");
			
			for (int i = 0; i < players.length; i++)
			{
				testAbortScript();
				expelFromBuilding(players[i]);
			}
		}
		
		String[] cells = getCellNames(structure);
		if (cells != null)
		{
			for (int i = 0; i < cells.length; i++)
			{
				testAbortScript();
				obj_id cellid = getCellId(structure, cells[i]);
				obj_id contents[] = getContents(cellid);
				if (contents != null)
				{
					for (int j=0; j<contents.length; j++)
					{
						testAbortScript();
						if (hasCondition(contents[j], CONDITION_VENDOR))
						{
							obj_id owner = getObjIdObjVar(contents[j], "vendor_owner");
							if (!isIdValid(owner))
							{
								owner = getOwner( contents[j] );
							}
							
							vendor_lib.finalizePackUp(owner, contents[j], player, isAbandoned);
						}
						
						if (isIdValid(contents[j]))
						{
							messageTo(contents[j], "OnPack", null, 1.0f, false);
						}
					}
				}
			}
		}
		
		String structName = getStructureName(structure);
		if (structName == null || structName.length() < 1)
		{
			structName = "Building";
		}
		
		if (hasObjVar(structure, VAR_WAYPOINT_STRUCTURE))
		{
			obj_id waypoint = getObjIdObjVar(structure, VAR_WAYPOINT_STRUCTURE);
			if (isIdValid(waypoint))
			{
				destroyWaypointInDatapad(waypoint, structure_owner);
				removeObjVar(structure, VAR_WAYPOINT_STRUCTURE);
			}
		}
		
		destroyStructureSign(structure);
		setName(scd, structName);
		
		if (hasBeenDeclaredResidence( structure ))
		{
			obj_id resident = getResidentPlayer(structure);
			setHouseId( resident, obj_id.NULL_ID );
			removeObjVar( structure, VAR_RESIDENCE_BUILDING );
			
			int cityId = getCitizenOfCityId(resident);
			if (cityId != 0)
			{
				cityRemoveCitizen( cityId, resident );
			}
		}
		
		city.removeStructureFromCity(structure);
		
		String template = getTemplateName(structure);
		setDeedTemplate(scd, template);
		
		if (hasObjVar(structure, VAR_DEED_SCENE))
		{
			setObjVar(scd, VAR_DEED_SCENE, getDeedScene(structure));
		}
		
		location where = getLocation(structure);
		
		if (isIdValid(structure_owner) && exists(structure_owner))
		{
			obj_id lotOverlimitStructure = getObjIdObjVar(structure_owner, "lotOverlimit.structure_id");
			if (isIdValid(lotOverlimitStructure) && (lotOverlimitStructure == structure))
			{
				setObjVar(structure_owner, "lotOverlimit.structure_location", "Datapad");
			}
		}
		
		putIn(structure, scd);
		if (!structure_owner.isLoaded())
		{
			final int maxDepth = isFactory(structure) ? 101 : 1;
			
			moveToOfflinePlayerDatapadAndUnload(scd, structure_owner, maxDepth + 1);
			fixLoadWith(structure, structure_owner, maxDepth);
		}
		
		CustomerServiceLog("housepackup", "Player/Owner "+ getPlayerName(player) +
		" ("+ player + ") packed structure ("+ structure + ",abandoned=" + isAbandoned + ","+ where.toString()
		+ " into device ("+ scd + ")");
		
		if (city.isInCity(structure))
		{
			int city_id = getCityAtLocation( getLocation( structure ), 0 );
			
			obj_id mayor = cityGetLeader( city_id );
			String mayor_name = cityGetCitizenName( city_id, mayor );
			String structure_name = localize( getNameStringId( structure ));
			String ownerName = player_structure.getStructureOwner(structure);
			
			prose_package pp = new prose_package();
			prose.setStringId(pp, SELF_PACKUP_EMAIL_BODY);
			prose.setTT(pp, ownerName);
			prose.setTU(pp, structure_name);
			
			utils.sendMail(SELF_PACKUP_EMAIL_TITLE, pp, mayor, "Galactic Housing Authority");
		}
		return;
	}
	
	
	public static void finalizeAbandonedStructurePackUp(obj_id player, obj_id structure) throws InterruptedException
	{
		blog("player_structure.finalizeAbandonedStructurePackUp init.");
		final boolean isAbandoned = isAbandoned(structure);
		if (!isAbandoned)
		{
			
			return;
		}
		
		obj_id structure_owner = getOwner(structure);
		
		CustomerServiceLog("housepackup", "Player "+ getPlayerName(player) +
		" ("+ player + ") is attempting to pack structure ("+ structure + ",abandoned="+ isAbandoned + ") owned by player "+
		getPlayerName(structure_owner) + " ("+ structure_owner + ")");
		
		if (!structure.isLoaded() || !structure.isAuthoritative())
		{
			blog("player_structure.finalizeAbandonedStructurePackUp structure NOT LOADED or NOT AUTHORITATIVE.");
			
			CustomerServiceLog("housepackup", "Structure ("+ structure + ") could not be packed by player (" + getPlayerName(player) + ") structure -" + " isLoaded = "+ structure.isLoaded()
			+ " isAuthoritative = "+ structure.isAuthoritative()
			);
			return;
		}
		
		obj_id datapad = utils.getPlayerDatapad(player);
		if (!isIdValid(datapad))
		{
			blog("player_structure.finalizeAbandonedStructurePackUp datapad not valid.");
			
			CustomerServiceLog("housepackup", "Structure ("+ structure + ") could not be packed by player (" + getPlayerName(player) + ") - datapad was not valid" );
			return;
		}
		
		if (structure_owner.isLoaded() && structure_owner.isAuthoritative())
		{
			blog("player_structure.finalizeAbandonedStructurePackUp The owner is loaded in game.");
			
			obj_id owner_datapad = utils.getPlayerDatapad(structure_owner);
			if (isIdValid(owner_datapad))
			{
				datapad = owner_datapad;
			}
		}
		else if (!player.isLoaded() || !player.isAuthoritative())
		{
			blog("player_structure.finalizeAbandonedStructurePackUp The owner is NOT loaded in game and the player packing is not loaded or not authoritative.");
			
			CustomerServiceLog("housepackup", "Structure ("+ structure + ") could not be packed by player (" + getPlayerName(player) + ") player -" + " isLoaded = "+ player.isLoaded()
			+ " isAuthoritative = "+ player.isAuthoritative()
			);
			return;
		}
		
		location where = getLocation(structure);
		
		dictionary params = new dictionary();
		params.put("structure", structure);
		params.put("structure_owner", structure_owner);
		
		blog("player_structure.finalizePackUp STRUCTURE WAS ABANDONED. Sending message back to player to send animation, tally success.");
		messageTo( player, "callAirStrikePackAbandonedStructure", params, 0, false );
		
		CustomerServiceLog("housepackup", "Player "+ getPlayerName(player) +
		" ("+ player + ") is ABOUT to pack abandoned structure ("+ structure + ",abandoned="+ isAbandoned + ","+ where.toString() + ") owned by player "+
		getPlayerName(structure_owner) + " ("+ structure_owner + ")");
		
		if (city.isInCity(structure))
		{
			int city_id = getCityAtLocation( getLocation( structure ), 0 );
			
			obj_id mayor = cityGetLeader( city_id );
			String mayor_name = cityGetCitizenName( city_id, mayor );
			String structure_name = localize( getNameStringId( structure ));
			String ownerName = player_structure.getStructureOwner(structure);
			
			prose_package pp = new prose_package();
			prose.setStringId(pp, PACKUP_EMAIL_BODY);
			prose.setTT(pp, ownerName);
			prose.setTU(pp, structure_name);
			
			utils.sendMail(PACKUP_EMAIL_TITLE, pp, mayor, "Galactic Vacant Building Demolishing Movement");
		}
		
		return;
	}
	
	
	public static void confirmCityAbandonedAndPack(obj_id structure, obj_id player) throws InterruptedException
	{
		if (!isIdValid(player))
		{
			if (hasObjVar(player, "qa.city_packup"))
			{
				sendSystemMessageTestingOnly(player, "Packup Failed to confirm due to player(you) Obj_ID being invalid");
			}
			LOG("sissynoid", "City Pack Up System - Player ("+ player +")"+ getPlayerFullName(player) +" attempted to pack a structure("+ structure +"), but the Player ID is invalid");
			CustomerServiceLog("city_house_packup", "City Pack Up System - Player ("+ player +")"+ getPlayerFullName(player) +" attempted to pack a structure("+ structure +"), but the Player ID is invalid");
			return;
		}
		if (!isIdValid(structure))
		{
			if (hasObjVar(player, "qa.city_packup"))
			{
				sendSystemMessageTestingOnly(player, "Packup Failed to confirm due to structure Obj_ID being invalid");
			}
			LOG("sissynoid", "City Pack Up System - Player ("+ player +")"+ getPlayerFullName(player) +" attempted to pack a structure("+ structure +"), but the house ID is invalid");
			CustomerServiceLog("city_house_packup", "City Pack Up System - Player ("+ player +")"+ getPlayerFullName(player) +" attempted to pack a structure("+ structure +"), but the house ID is invalid");
			sendSystemMessage(player, SID_GENERIC_CITY_PACKUP_ERROR);
			return;
		}
		
		obj_id owner = getOwner(structure);
		if (!isPlayer(owner))
		{
			if (hasObjVar(player, "qa.city_packup"))
			{
				sendSystemMessageTestingOnly(player, "Packup Failed to confirm due to owner Obj_ID not existing - he has likely been deleted.");
			}
			LOG("sissynoid", "City Pack Up System - Player ("+ player +")"+ getPlayerFullName(player) +" attempted to pack structure("+ structure +") but the owner("+ getOwner(structure) +") Obj ID is not a player - Character has been deleted.");
			CustomerServiceLog("city_house_packup", "City Pack Up System - Player ("+ player +")"+ getPlayerFullName(player) +" attempted to pack structure("+ structure +") but the owner("+ getOwner(structure) +") Obj ID is not a player - Character has been deleted.");
			sendSystemMessage(player, SID_GENERIC_CITY_PACKUP_ERROR);
			return;
		}
		if (!isIdValid(owner))
		{
			if (hasObjVar(player, "qa.city_packup"))
			{
				sendSystemMessageTestingOnly(player, "Packup Failed to confirm due to invalid owner Obj_ID");
			}
			LOG("sissynoid", "City Pack Up System - Player ("+ player +")"+ getPlayerFullName(player) +" attempted to pack structure("+ structure +") owned by Player("+ getOwner(structure) +") - but owner ID of the structure is invalid.");
			CustomerServiceLog("city_house_packup", "City Pack Up System - Player ("+ player +")"+ getPlayerFullName(player) +" attempted to pack structure("+ structure +") owned by Player("+ getOwner(structure) +") - but owner ID of the structure is invalid.");
			sendSystemMessage(player, SID_GENERIC_CITY_PACKUP_ERROR);
			return;
		}
		
		int loginTime = getPlayerLastLoginTime(owner);
		int curTime = getCalendarTime();
		
		if ((curTime - loginTime) < cityGetInactivePackupInactiveTimeSeconds())
		{
			if (hasObjVar(player, "qa.city_packup"))
			{
				sendSystemMessageTestingOnly(player, "Packup Failed to confirm due to the owner not being in violation (has not been logged out long enough)");
			}
			
			LOG("sissynoid", "City Pack Up System - Player ("+ player +")"+ getPlayerFullName(player) +" attempted to pack structure("+ structure +") owned by Player("+ getOwner(structure) +") - but owner has not been offline for 90 days - Removing City Abandoned Flag.");
			CustomerServiceLog("city_house_packup", "City Pack Up System - Player ("+ player +")"+ getPlayerFullName(player) +" attempted to pack structure("+ structure +") owned by Player("+ getOwner(structure) +") - but owner has not been offline for 90 days - Removing City Abandoned Flag.");
			player_structure.removeCityAbandoned(structure);
			sendSystemMessage(player, SID_NOT_CITY_ABANDONED);
			return;
		}
		
		int city_id = getCityAtLocation(getLocation(structure), 0);
		
		if (!cityExists(city_id))
		{
			if (hasObjVar(player, "qa.city_packup"))
			{
				sendSystemMessageTestingOnly(player, "Packup Failed to confirm due to the offending structure no longer residing inside of a player city.");
			}
			
			LOG("sissynoid", "City Pack Up System - Player ("+ player +")"+ getPlayerFullName(player) +" attempted to pack structure("+ structure +") owned by Player("+ getOwner(structure) +") but the structure is no longer in a valid Player City. Removing City Abandoned Tag.");
			CustomerServiceLog("city_house_packup", "City Pack Up System - Player ("+ player +")"+ getPlayerFullName(player) +" attempted to pack structure("+ structure +") owned by Player("+ getOwner(structure) +") but the structure is no longer in a valid Player City. Removing City Abandoned Tag.");
			player_structure.removeCityAbandoned(structure);
			sendSystemMessage(player, SID_NO_CITY);
			return;
		}
		if (city.hasMayorProtectionFlag(owner, city_id))
		{
			if (hasObjVar(player, "qa.city_packup"))
			{
				sendSystemMessageTestingOnly(player, "Packup Failed to confirm due to the Mayor protecting this Player's House.");
			}
			LOG("sissynoid", "City Pack Up System - Player ("+ player +")"+ getPlayerFullName(player) +" attempted to pack structure("+ structure +") owned by Player("+ getOwner(structure) +") - but owner is protected by the Mayor("+cityGetLeader(city_id)+") of City("+ city_id +")");
			CustomerServiceLog("city_house_packup", "City Pack Up System - Player ("+ player +")"+ getPlayerFullName(player) +" attempted to pack structure("+ structure +") owned by Player("+ getOwner(structure) +") - but owner is protected by the Mayor("+cityGetLeader(city_id)+") of City("+ city_id +")");
			sendSystemMessage(player, SID_MAYOR_PROTECTED);
			return;
		}
		
		if (player_structure.isOwner(structure, player))
		{
			if (hasObjVar(player, "qa.city_packup"))
			{
				sendSystemMessageTestingOnly(player, "Packup Failed to confirm due to YOU being the OWNER! (God Mode makes you the owner of everything - '/setGod 0' please)");
			}
			
			LOG("sissynoid", "City Pack Up System - Player ("+ player +")"+ getPlayerFullName(player) +" attempted to pack his own structure - Removing City Abandoned Tag.");
			CustomerServiceLog("city_house_packup", "City Pack Up System - Player ("+ player +")"+ getPlayerFullName(player) +" attempted to pack his own structure - Removing City Abandoned Tag.");
			player_structure.removeCityAbandoned(structure);
			return;
		}
		
		if (!player_structure.isCityAbandoned(structure))
		{
			if (hasObjVar(player, "qa.city_packup"))
			{
				sendSystemMessageTestingOnly(player, "Packup Failed to confirm due to Structure no longer being CityAbandoned");
			}
			LOG("sissynoid", "City Pack Up System - Player ("+ player +")"+ getPlayerFullName(player) +" attempted to pack this structure("+ structure +") - but it is not abandoned - bail");
			CustomerServiceLog("city_house_packup", "City Pack Up System - Player ("+ player +")"+ getPlayerFullName(player) +" attempted to pack this structure("+ structure +") - but it is not abandoned - bail");
			sendSystemMessage(player, SID_NOT_CITY_ABANDONED);
			return;
		}
		
		if (!player_structure.isCityPackupAuthoritative(player, structure))
		{
			if (hasObjVar(player, "qa.city_packup"))
			{
				sendSystemMessageTestingOnly(player, "Packup Failed to confirm due to Player and House being on different game servers.");
			}
			
			LOG("sissynoid", "City Pack Up System - Player ("+ player +")"+ getPlayerFullName(player) +" attempted to pack this structure - but the structure("+ structure +") is on a different server than the player.");
			CustomerServiceLog("city_house_packup", "City Pack Up System - Player ("+ player +")"+ getPlayerFullName(player) +" attempted to pack this structure - but the structure("+ structure +") is on a different server than the player.");
			sendSystemMessage(player, SID_GENERIC_CITY_PACKUP_ERROR);
			return;
		}
		
		if (!player_structure.canPackStructureWithVendors(player, structure))
		{
			if (hasObjVar(player, "qa.city_packup"))
			{
				sendSystemMessageTestingOnly(player, "Packup Failed to confirm due to Server Vendor Config preventing Vendor Packups.");
			}
			LOG("sissynoid", "City Pack Up System - Player ("+ player +")"+ getPlayerFullName(player) +" attempted to pack structure - but the structure("+ structure +") can't be packed due to Vendors Config");
			CustomerServiceLog("city_house_packup", "City Pack Up System - Player ("+ player +")"+ getPlayerFullName(player) +" attempted to pack structure - but the structure("+ structure +") can't be packed due to Vendors Config");
			sendSystemMessage(player, SID_GENERIC_CITY_PACKUP_ERROR);
			return;
		}
		LOG("sissynoid", "CHECKING NULL: SIGN CALL: HOUSE("+ structure +"), PLAYER(PACKER)("+ player +")");
		dictionary params = new dictionary();
		params.put("house", structure);
		params.put("player", player);
		messageTo(player, "packUpCityAbandonedStructure", params, 1.0f, false);
	}
	
	
	public static void finalizeCityPackUp(obj_id player, obj_id structure) throws InterruptedException
	{
		LOG("sissynoid", "Entered: finalizeCityPackUp");
		if (!isIdValid(player))
		{
			LOG("sissynoid", "finalizeCityPackUp - Player is Invalid");
			return;
		}
		if (!isIdValid(structure))
		{
			LOG("sissynoid", "finalizeCityPackUp - Structure is Invalid");
			return;
		}
		
		if (!isCityAbandoned(structure))
		{
			LOG("sissynoid", "finalizeCityPackUp - Is NOT CITY ABANDONED!");
			CustomerServiceLog("city_house_packup", "City House Packup was called on Structure("+ structure +
			") by player("+ player +") but house was not City Abandoned!");
			return;
		}
		
		int city_id = getCityAtLocation(getLocation(structure), 0);
		if (!cityExists(city_id))
		{
			LOG("sissynoid", "finalizeCityPackUp: No Longer inside of a City - Remove isCityAbandoned!");
			
			CustomerServiceLog("city_house_packup", "City House Packup was called on Structure("+ structure +
			") by player("+ player +") but house is no longer in a City - Removing isCityAbandoned Flag.");
			removeCityAbandoned(structure);
			return;
		}
		
		obj_id structure_owner = getOwner(structure);
		
		CustomerServiceLog("city_house_packup", "Player "+ getPlayerName(player) +" ("+ player +
		") is attempting to pack structure ("+ structure + ", City Abandoned = "+ isCityAbandoned(structure) +
		") owned by player "+ getPlayerName(structure_owner) + " ("+ structure_owner + ")");
		
		obj_id datapad = utils.getPlayerDatapad(player);
		if (!isIdValid(datapad))
		{
			LOG("sissynoid", "finalizeCityPackUp: Datapad ID is Invalid");
			CustomerServiceLog("city_house_packup", "Structure ("+ structure + ") could not be packed by player ("+
			getPlayerName(player) + ") - datapad was not valid");
			return;
		}
		
		if (!player_structure.isCityPackupAuthoritative(player, structure))
		{
			LOG("sissynoid", "finalizeCityPackUp: Player(Packer) is not Loaded or Authoritative");
			CustomerServiceLog("city_house_packup",
			"'finalizeCityPackUp' - Structure ("+ structure +
			") could not be packed by player (" + getPlayerName(player) + ") player -"+
			" isLoaded(Player) = "+ player.isLoaded() +
			" isAuthoritative(Player) = "+ player.isAuthoritative() +
			" isLoaded(Structure) = "+ structure.isLoaded() +
			" isAuthoritative(Structure) = "+ structure.isAuthoritative());
			return;
		}
		
		location where = getLocation(structure);
		
		CustomerServiceLog("city_house_packup", "Player "+ getPlayerName(player) +" ("+ player +
		") is ABOUT to pack abandoned structure ("+ structure + ",abandoned="+ isCityAbandoned(structure) +
		","+ where.toString() + ") owned by player "+ getPlayerName(structure_owner) + " ("+ structure_owner + ")");
		
		if (cityExists(city_id))
		{
			LOG("sissynoid", "Mailing Mayor");
			obj_id mayor = cityGetLeader(city_id);
			String mayor_name = cityGetCitizenName(city_id, mayor);
			String structure_name = localize(getNameStringId(structure));
			String ownerName = player_structure.getStructureOwner(structure);
			LOG("sissynoid", "CHECKING MAIL FOR NULL: MAYOR("+ mayor +"), MAYOR_NAME("+ mayor_name +"), STRUCTURE_NAME("+ structure_name +"), STRUCTURE_OWNER("+ ownerName +")");
			prose_package pp = new prose_package();
			prose.setStringId(pp, PACKUP_EMAIL_BODY);
			prose.setTT(pp, ownerName);
			prose.setTU(pp, structure_name);
			
			utils.sendMail(PACKUP_EMAIL_TITLE, pp, mayor, "Galactic Housing Authority");
		}
		LOG("sissynoid", "checking for Null Value: PLAYER("+ player +"), STRUCTURE("+ structure +"), STRUCTURE_OWNER("+ structure_owner +")");
		
		dictionary params = new dictionary();
		params.put("player", player);
		params.put("structure", structure);
		params.put("structure_owner", structure_owner);
		messageTo (structure, "cityMoveStructureToSCD", params, 1.0f, false);
		LOG("sissynoid", "Calling cityMoveStructureToSCD - Finally starting the packup procedure.");
		return;
	}
	
	
	public static int getPlayerPackUpMeritPoints(obj_id player) throws InterruptedException
	{
		int noMerit = 0;
		if (!isValidId(player) || !exists(player))
		{
			return noMerit;
		}
		
		if (!hasObjVar(player, HOUSE_PACKUP_ARRAY_OBJVAR))
		{
			return noMerit;
		}
		
		int[] housePackingTally = getIntArrayObjVar(player, HOUSE_PACKUP_ARRAY_OBJVAR);
		if (housePackingTally == null || housePackingTally.length != ARRAY_LENGTH_FOR_HOUSE_PACKUP)
		{
			return noMerit;
		}
		
		int currentAmountAvailable = housePackingTally[0];
		if (currentAmountAvailable <= 0)
		{
			return noMerit;
		}
		
		return currentAmountAvailable;
	}
	
	
	public static boolean housePackingPointDecrease(obj_id player) throws InterruptedException
	{
		return housePackingPointDecrease(player, 1);
	}
	
	
	public static boolean housePackingPointDecrease(obj_id player, int changeAmount) throws InterruptedException
	{
		if (!isValidId(player) || !exists(player))
		{
			return false;
		}
		if (changeAmount == 0)
		{
			return false;
		}
		if (changeAmount < 0)
		{
			changeAmount = Math.abs(changeAmount);
		}
		
		if (!hasObjVar(player, HOUSE_PACKUP_ARRAY_OBJVAR))
		{
			return false;
		}
		
		int[] housePackingTally = getIntArrayObjVar(player, HOUSE_PACKUP_ARRAY_OBJVAR);
		if (housePackingTally == null || housePackingTally.length != ARRAY_LENGTH_FOR_HOUSE_PACKUP)
		{
			return false;
		}
		
		int currentAmountAvailable = housePackingTally[0];
		if (currentAmountAvailable <= 0)
		{
			return false;
		}
		
		if (currentAmountAvailable < changeAmount)
		{
			changeAmount = currentAmountAvailable;
		}
		
		for (int i = 0; i < changeAmount; i++)
		{
			testAbortScript();
			housePackingTally[0]--;
		}
		
		setObjVar(player, HOUSE_PACKUP_ARRAY_OBJVAR, housePackingTally);
		return true;
	}
	
	
	public static boolean housePackingPointIncrease(obj_id player) throws InterruptedException
	{
		return housePackingPointIncrease(player, 1);
	}
	
	
	public static boolean housePackingPointIncrease(obj_id player, int changeAmount) throws InterruptedException
	{
		blog("player_structure.housePackingPointIncrease init - changeAmount: "+changeAmount);
		
		if (!isValidId(player) || !exists(player))
		{
			return false;
		}
		
		if (changeAmount <= 0)
		{
			return false;
		}
		
		if (!hasObjVar(player, HOUSE_PACKUP_ARRAY_OBJVAR))
		{
			blog("player_structure.housePackingPointIncrease HOUSE_PACKUP_ARRAY_OBJVAR first time today packing house");
			int[] newPackUpTrackingArray = new int[ARRAY_LENGTH_FOR_HOUSE_PACKUP];
			
			for (int i = 0; i < ARRAY_LENGTH_FOR_HOUSE_PACKUP; i++)
			{
				testAbortScript();
				newPackUpTrackingArray[i] = 0;
			}
			setObjVar(player, HOUSE_PACKUP_ARRAY_OBJVAR, newPackUpTrackingArray);
		}
		
		int[] housePackUpTrackingArray = getIntArrayObjVar(player, HOUSE_PACKUP_ARRAY_OBJVAR);
		if (housePackUpTrackingArray.length != ARRAY_LENGTH_FOR_HOUSE_PACKUP)
		{
			return false;
		}
		
		for (int s = 0; s < ARRAY_LENGTH_FOR_HOUSE_PACKUP; s++)
		{
			testAbortScript();
			blog("player_structure.housePackingPointIncrease changing housePackUpTrackingArray["+ s +"] from: "+housePackUpTrackingArray[s]);
			
			housePackUpTrackingArray[s] += changeAmount;
			blog("to: "+housePackUpTrackingArray[s]);
			
		}
		
		setObjVar(player, HOUSE_PACKUP_ARRAY_OBJVAR, housePackUpTrackingArray);
		
		if (!hasObjVar(player, HOUSE_PACKUP_DAILY_TALLY_OBJVAR))
		{
			
			setObjVar(player, HOUSE_PACKUP_DAILY_TALLY_OBJVAR, 1);
			int resetTime = getGameTime() + player_structure.TIME_TO_NEXT_PACKUP;
			blog("housePackingPointIncrease - resetting time to next packup: "+resetTime);
			setObjVar(player, HOUSE_PACKUP_DAILY_TIMER_OBJVAR, resetTime);
		}
		else
		{
			
			int tally = getIntObjVar(player, HOUSE_PACKUP_DAILY_TALLY_OBJVAR);
			
			tally += changeAmount;
			
			setObjVar(player, HOUSE_PACKUP_DAILY_TALLY_OBJVAR, tally);
			
		}
		
		if (!badge.hasBadge(player, "house_packup_badge"))
		{
			
			if (housePackUpTrackingArray[1] >= player_structure.NUM_NEEDED_PACKUP_FIRST_BADGE)
			{
				badge.grantBadge (player, "house_packup_badge");
			}
			
			return true;
		}
		
		if (!badge.hasBadge(player, "house_packup_badge_master"))
		{
			
			if (housePackUpTrackingArray[1] >= player_structure.NUM_NEEDED_PACKUP_SECOND_BADGE)
			{
				badge.grantBadge (player, "house_packup_badge_master");
			}
			
			return true;
		}
		
		if (!badge.hasBadge(player, "house_packup_badge_mogul"))
		{
			
			if (housePackUpTrackingArray[1] >= player_structure.NUM_NEEDED_PACKUP_THIRD_BADGE)
			{
				badge.grantBadge (player, "house_packup_badge_mogul");
			}
			
			return true;
		}
		
		return true;
	}
	
	
	public static boolean hasBeenDeclaredResidence(obj_id structure) throws InterruptedException
	{
		return hasObjVar(structure, VAR_RESIDENCE_BUILDING);
	}
	
	
	public static obj_id getResidentPlayer(obj_id structure) throws InterruptedException
	{
		if (!hasBeenDeclaredResidence(structure))
		{
			return null;
		}
		
		obj_id resident = getObjIdObjVar(structure, VAR_RESIDENCE_BUILDING);
		
		return resident;
	}
	
	
	public static boolean replacePackedStructure(obj_id scd, obj_id owner, location loc, int rot, float placementHeight) throws InterruptedException
	{
		obj_id[] contents = getContents(scd);
		if (contents.length != 1)
		{
			
			trace.log("housepackup", "%TU attempted to unpack a house, but the SCD did not contain one.", owner, trace.TL_CS_LOG | trace.TL_ERROR_LOG);
			return false;
		}
		
		obj_id house = contents[0];
		
		if (isIdValid(house))
		{
			String houseTemplateName = getTemplateName(house);
			if ((houseTemplateName != null) && player_structure.isBuildingName(houseTemplateName))
			{
				loc.y = placementHeight;
			}
		}
		
		if (!setLocation(house, loc))
		{
			return false;
		}
		
		if (isIdValid(owner) && exists(owner))
		{
			obj_id lotOverlimitStructure = getObjIdObjVar(owner, "lotOverlimit.structure_id");
			if (isIdValid(lotOverlimitStructure) && (lotOverlimitStructure == house))
			{
				setObjVar(owner, "lotOverlimit.structure_location", localize(new string_id("planet_n", loc.area)) + " ("+ (int)loc.x + ", "+ (int)loc.z + ")");
			}
		}
		
		if (hasObjVar(house, "packed_house.fix_contained_scene_ids"))
		{
			removeObjVar(house, "packed_house.fix_contained_scene_ids");
		}
		
		if (player_structure.isAbandoned( house ))
		{
			removeObjVar( house, "player_structure.abandoned");
			if (hasObjVar( house, "player_structure.name.original" ))
			{
				obj_var originalName = getObjVar( house, "player_structure.name.original");
				setName( house, originalName.getStringData() );
			}
		}
		if (player_structure.isCityAbandoned(house))
		{
			removeObjVar(house, "player_structure.city_abandoned");
			if (hasObjVar(house, "player_structure.name.original"))
			{
				obj_var originalName = getObjVar(house, "player_structure.name.original");
				setName(house, originalName.getStringData());
			}
		}
		
		float rot_float = (float)(90 * rot);
		setYaw(house, rot_float);
		persistObject(house);
		createStructureSign(house, rot_float);
		addStructureWaypoint(owner, loc, getStructureName(house), house);
		
		city.addStructureToCity(house);
		trace.log("housepackup", "%TU unpacked their house ("+ house + ") from structure control device ( "+ scd + ") to new location "+ loc.toString(), owner, trace.TL_CS_LOG);
		
		obj_id[] houseContents = trial.getAllObjectsInDungeon(house);
		
		for (int i = 0, j = houseContents.length; i < j; i++)
		{
			testAbortScript();
			messageTo(houseContents[i], "OnUnpack", null, 1.0f, false);
		}
		
		return true;
	}
	
	
	public static boolean hasVendors(obj_id structure) throws InterruptedException
	{
		
		String[] cells = getCellNames(structure);
		if (cells != null)
		{
			for (int i = 0; i < cells.length; i++)
			{
				testAbortScript();
				obj_id cellid = getCellId(structure, cells[i]);
				obj_id contents[] = getContents(cellid);
				if (contents != null)
				{
					for (int j=0; j<contents.length; j++)
					{
						testAbortScript();
						if (hasCondition(contents[j], CONDITION_VENDOR))
						{
							return true;
						}
					}
				}
			}
		}
		
		return false;
	}
	
	
	public static boolean isBuilding(obj_id object) throws InterruptedException
	{
		int got = getGameObjectType(object);
		if (isGameObjectTypeOf(got, GOT_building))
		{
			
			return true;
		}
		
		return false;
	}
	
	
	public static boolean isBuildingName(String objectTemplateName) throws InterruptedException
	{
		return objectTemplateName.indexOf ("building") != -1;
	}
	
	
	public static boolean isInstallation(obj_id object) throws InterruptedException
	{
		if (object == null || object == obj_id.NULL_ID)
		{
			return false;
		}
		
		int got = getGameObjectType(object);
		if (isGameObjectTypeOf(got, GOT_installation))
		{
			
			return true;
		}
		
		return false;
	}
	
	
	public static boolean isGuildHall(obj_id object) throws InterruptedException
	{
		if (!isIdValid(object))
		{
			return false;
		}
		
		if (!isBuilding(object))
		{
			return false;
		}
		
		if (hasObjVar(object, VAR_IS_GUILD_HALL))
		{
			return true;
		}
		else
		{
			String templateName = getTemplateName(object);
			if (templateName.indexOf("guildhall") > 0 || templateName.indexOf("player_mustafar_house_lg") > 0 || templateName.indexOf("player_house_mustafar_lg") > 0)
			{
				return true;
			}
			else
			{
				return false;
			}
		}
	}
	
	
	public static boolean isCivic(obj_id object) throws InterruptedException
	{
		if (!isIdValid(object))
		{
			return false;
		}
		
		if (exists(object) && getIntObjVar(object, VAR_CIVIC) == 1)
		{
			return true;
		}
		else
		{
			return false;
		}
	}
	
	
	public static boolean isAbandoned(obj_id object) throws InterruptedException
	{
		if (!isIdValid(object))
		{
			return false;
		}
		
		return ( getIntObjVar(object, VAR_ABANDONED) == PLAYER_STRUCTURE_ABANDONED );
	}
	
	public static void setAbandoned(obj_id object) throws InterruptedException
	{
		setObjVar(object, VAR_ABANDONED, PLAYER_STRUCTURE_ABANDONED);
	}
	
	
	public static boolean isPreAbandoned(obj_id object) throws InterruptedException
	{
		if (!isIdValid(object))
		{
			return false;
		}
		
		return ( getIntObjVar(object, VAR_ABANDONED) == PLAYER_STRUCTURE_PRE_ABANDONED );
	}
	
	
	public static void setPreAbandoned(obj_id object) throws InterruptedException
	{
		setObjVar(object, VAR_ABANDONED, PLAYER_STRUCTURE_PRE_ABANDONED);
	}
	
	
	public static void setStructureAsNotAbandoned(obj_id object) throws InterruptedException
	{
		removeObjVar(object, VAR_ABANDONED);
	}
	
	
	public static boolean isCityAbandoned(obj_id object) throws InterruptedException
	{
		if (!isIdValid(object))
		{
			return false;
		}
		
		if (hasObjVar(object, VAR_CITY_ABANDONED))
		{
			return true;
		}
		
		return false;
	}
	
	
	public static void setCityAbandoned(obj_id object) throws InterruptedException
	{
		if (!isIdValid(object))
		{
			return;
		}
		
		setObjVar(object, VAR_CITY_ABANDONED, PLAYER_STRUCTURE_CITY_ABANDONED);
	}
	
	
	public static void removeCityAbandoned(obj_id object) throws InterruptedException
	{
		if (!isIdValid(object))
		{
			return;
		}
		
		removeCityAbandonedTagFromHouse(object);
		
		if (hasObjVar(object, VAR_CITY_ABANDONED))
		{
			removeObjVar(object, VAR_CITY_ABANDONED);
		}
	}
	
	
	public static boolean isCityPackupAuthoritative(obj_id player, obj_id structure) throws InterruptedException
	{
		if (!player.isLoaded() || !player.isAuthoritative())
		{
			return false;
		}
		
		if (!structure.isLoaded() || !structure.isAuthoritative())
		{
			return false;
		}
		
		return true;
	}
	
	
	public static void removeCityAbandonedTagFromHouse(obj_id structure) throws InterruptedException
	{
		LOG("sissynoid", "Removing Zoning Violation Text.");
		
		if (!isIdValid(structure))
		{
			return;
		}
		
		if (player_structure.isFactory(structure))
		{
			
			if (hasObjVar(structure, "player_structure.name.original"))
			{
				String signOriginal = getStringObjVar(structure, "player_structure.name.original");
				setName(structure, signOriginal);
			}
			return;
		}
		else
		{
			if (!hasObjVar(structure, "player_structure.sign.id"))
			{
				LOG("sissynoid", "Structure("+ structure +") does not have a House Sign ObjVar");
				return;
			}
			
			obj_id signId = getObjIdObjVar(structure, "player_structure.sign.id");
			String signText = getName(signId);
			int indexOfAbandonedText = signText.indexOf(player_structure.CITY_ABANDONED_TEXT);
			if (indexOfAbandonedText > -1)
			{
				
				signText = signText.substring(0, indexOfAbandonedText);
				setName(signId, signText);
				LOG("sissynoid", "Removed Zoning Violation - New Text: "+ signText);
			}
		}
	}
	
	
	public static void updateCityAbandonedFactoryName(obj_id factory) throws InterruptedException
	{
		if (player_structure.isFactory(factory))
		{
			LOG("sissynoid", "Factory - Updating Sign");
			String assignedName = getAssignedName(factory);
			
			if (!hasObjVar(factory, "player_structure.name.original"))
			{
				setObjVar(factory, "player_structure.name.original", assignedName == null ? "": assignedName);
			}
			else
			{
				
				if (assignedName != null && assignedName.indexOf(player_structure.CITY_ABANDONED_TEXT) != -1)
				{
					
					assignedName = getStringObjVar(factory, "player_structure.name.original");
				}
				else
				{
					
					setObjVar(factory, "player_structure.name.original", assignedName == null ? "": assignedName);
				}
			}
			
			if (assignedName == null || assignedName.length () == 0)
			{
				String templateName = getTemplateName(factory);
				
				if (templateName.equals("object/installation/manufacture/food_factory.iff"))
				{
					assignedName = "Food and Chemical Factory";
				}
				else if (templateName.equals("object/installation/manufacture/clothing_factory.iff"))
				{
					assignedName = "Wearables Factory";
				}
				else if (templateName.equals("object/installation/manufacture/weapon_factory.iff"))
				{
					assignedName = "Equipment Factory";
				}
				else if (templateName.equals("object/installation/manufacture/structure_factory.iff"))
				{
					assignedName = "Structure Factory";
				}
				else
				{
					assignedName = "Factory";
				}
			}
			
			setName(factory, assignedName + player_structure.CITY_ABANDONED_TEXT);
		}
	}
	
	
	public static boolean isCommercial(obj_id object) throws InterruptedException
	{
		if (!isIdValid(object))
		{
			return false;
		}
		
		if (!isCivic(object) && getStructureCityRank(object)>0)
		{
			return true;
		}
		return false;
	}
	
	
	public static int getStructureCityRank(obj_id object) throws InterruptedException
	{
		if (!isIdValid(object))
		{
			return 0;
		}
		
		return getIntObjVar( object, VAR_CITY_RANK );
	}
	
	
	public static int getStructureCityCost(obj_id object) throws InterruptedException
	{
		if (!isIdValid(object))
		{
			return 0;
		}
		
		return getIntObjVar( object, VAR_CITY_COST );
	}
	
	
	public static boolean isCivicTemplate(String template) throws InterruptedException
	{
		String[] structures = dataTableGetStringColumn( PLAYER_STRUCTURE_DATATABLE, "STRUCTURE");
		for (int i=0; i<structures.length; i++)
		{
			testAbortScript();
			if (structures[i].equals(template))
			{
				int civic = dataTableGetInt( PLAYER_STRUCTURE_DATATABLE, i, "CIVIC");
				if (civic == 1)
				{
					return true;
				}
				else
				{
					return false;
				}
			}
		}
		
		return false;
	}
	
	
	public static boolean isShuttleportTemplate(String template) throws InterruptedException
	{
		String[] structures = dataTableGetStringColumn( PLAYER_STRUCTURE_DATATABLE, "STRUCTURE");
		for (int i=0; i<structures.length; i++)
		{
			testAbortScript();
			if (structures[i].equals(template))
			{
				int shuttleport = dataTableGetInt( PLAYER_STRUCTURE_DATATABLE, i, "SHUTTLEPORT");
				if (shuttleport == 1)
				{
					return true;
				}
				else
				{
					return false;
				}
			}
		}
		
		return false;
	}
	
	
	public static boolean isCloneTemplate(String template) throws InterruptedException
	{
		String[] structures = dataTableGetStringColumn( PLAYER_STRUCTURE_DATATABLE, "STRUCTURE");
		for (int i=0; i<structures.length; i++)
		{
			testAbortScript();
			if (structures[i].equals(template))
			{
				int clone = dataTableGetInt( PLAYER_STRUCTURE_DATATABLE, i, "CLONE_FACILITY");
				if (clone == 1)
				{
					return true;
				}
				else
				{
					return false;
				}
			}
		}
		
		return false;
	}
	
	
	public static boolean isGarageTemplate(String template) throws InterruptedException
	{
		
		String[] structures = dataTableGetStringColumn(PLAYER_STRUCTURE_DATATABLE, "GARAGE");
		for (int i=0; i<structures.length; i++)
		{
			testAbortScript();
			if (structures[i].equals(template))
			{
				int garage = dataTableGetInt( PLAYER_STRUCTURE_DATATABLE, i, "GARAGE");
				if (garage == 1)
				{
					return true;
				}
				else
				{
					return false;
				}
			}
			
		}
		return false;
	}
	
	
	public static boolean isFactory(obj_id object) throws InterruptedException
	{
		if (object == null || object == obj_id.NULL_ID)
		{
			return false;
		}
		
		if (getGameObjectType(object) == GOT_installation_factory)
		{
			return true;
		}
		else
		{
			return false;
		}
	}
	
	
	public static boolean isHarvester(obj_id object) throws InterruptedException
	{
		if (object == null || object == obj_id.NULL_ID)
		{
			return false;
		}
		
		if (getGameObjectType(object) == GOT_installation_harvester)
		{
			return true;
		}
		else
		{
			return false;
		}
	}
	
	
	public static boolean isGenerator(obj_id object) throws InterruptedException
	{
		if (object == null || object == obj_id.NULL_ID)
		{
			return false;
		}
		
		if (getGameObjectType(object) == GOT_installation_generator)
		{
			return true;
		}
		else
		{
			return false;
		}
	}
	
	
	public static boolean isAdmin(obj_id structure, String player_or_guild_name) throws InterruptedException
	{
		if (!isIdValid(structure))
		{
			return false;
		}
		
		String[] admin_list = getAdminListRaw(structure);
		if (admin_list != null)
		{
			
			if (isNameOnRawList( admin_list, player_or_guild_name ))
			{
				return true;
			}
			else
			{
				
			}
		}
		
		return false;
	}
	
	
	public static boolean isAdmin(obj_id structure, obj_id player) throws InterruptedException
	{
		if (!isIdValid(structure))
		{
			return false;
		}
		
		if (!isIdValid(player))
		{
			return false;
		}
		
		if (isGod(player))
		{
			return true;
		}
		
		String player_name = getPlayerName(player);
		boolean admin = isAdmin(structure, player_name);
		
		if (!admin)
		{
			
			if (isOwner(structure, player))
			{
				
				setObjVar(structure, VAR_OWNER, player_name);
				
				modifyList(structure, player_name, null, VAR_ADMIN_LIST, false);
				
				return true;
			}
		}
		
		return admin;
	}
	
	
	public static boolean isVendor(obj_id structure, String player_name) throws InterruptedException
	{
		if (structure == null || structure == obj_id.NULL_ID)
		{
			return false;
		}
		
		String[] vendor_list = getVendorList(structure);
		if (vendor_list != null)
		{
			String playerNameUpperCase = player_name.toUpperCase();
			for (int i = 0; i < vendor_list.length; i++)
			{
				testAbortScript();
				if (playerNameUpperCase.equals(vendor_list[i].toUpperCase()))
				{
					return true;
				}
			}
		}
		
		return false;
	}
	
	
	public static boolean isVendor(obj_id structure, obj_id player) throws InterruptedException
	{
		if (isIdValid(player))
		{
			return isVendor(structure, getPlayerName(player));
		}
		else
		{
			return false;
		}
	}
	
	
	public static boolean canEnter(obj_id structure, String player_or_guild_name) throws InterruptedException
	{
		if (!isIdValid(structure))
		{
			return false;
		}
		
		String[] entry_list = getCompleteEntryList(structure);
		if (entry_list != null)
		{
			if (isNameOnPermissionList( entry_list, player_or_guild_name ))
			{
				return true;
			}
			else
			{
				
			}
		}
		
		return false;
	}
	
	
	public static boolean canEnter(obj_id structure, obj_id player) throws InterruptedException
	{
		if (isIdValid(player))
		{
			return canEnter(structure, getPlayerName(player));
		}
		else
		{
			return false;
		}
	}
	
	
	public static boolean canHopper(obj_id structure, String player_or_guild_name) throws InterruptedException
	{
		if (!isIdValid(structure))
		{
			return false;
		}
		
		String[] hopper_list = getHopperListRaw(structure);
		if (hopper_list != null)
		{
			
			if (isNameOnRawList( hopper_list, player_or_guild_name ))
			{
				return true;
			}
			else
			{
				
			}
		}
		
		return false;
	}
	
	
	public static boolean canHopper(obj_id structure, obj_id player) throws InterruptedException
	{
		if (isIdValid(player))
		{
			return canHopper(structure, getPlayerName(player));
		}
		else
		{
			return false;
		}
	}
	
	
	public static boolean isBanned(obj_id structure, String player_or_guild_name) throws InterruptedException
	{
		if (!isIdValid(structure))
		{
			return false;
		}
		
		String[] ban_list = getBanList(structure);
		if (ban_list != null)
		{
			if (isNameOnPermissionList( ban_list, player_or_guild_name ))
			{
				return true;
			}
			else
			{
				
			}
		}
		
		return false;
	}
	
	
	public static boolean isBanned(obj_id structure, obj_id player) throws InterruptedException
	{
		if (isIdValid(player))
		{
			return isBanned(structure, getPlayerName(player));
		}
		else
		{
			return false;
		}
	}
	
	
	public static boolean isOwner(obj_id structure, String player_name) throws InterruptedException
	{
		if (structure == null || structure == obj_id.NULL_ID)
		{
			return false;
		}
		
		String owner_name = getStructureOwner(structure);
		
		if (owner_name.toUpperCase().equals(player_name.toUpperCase()))
		{
			return true;
		}
		else
		{
			return false;
		}
	}
	
	
	public static boolean isOwner(obj_id structure, obj_id player) throws InterruptedException
	{
		if (player == null || player == obj_id.NULL_ID)
		{
			return false;
		}
		
		if (isGod(player))
		{
			return true;
		}
		
		obj_id owner = getOwner(structure);
		if (owner == player)
		{
			return true;
		}
		else
		{
			return false;
		}
	}
	
	
	public static boolean isResidence(obj_id structure, obj_id player) throws InterruptedException
	{
		if (structure == null || structure == obj_id.NULL_ID)
		{
			return false;
		}
		
		if (player == null || player == obj_id.NULL_ID)
		{
			return false;
		}
		
		obj_id residence = getResidence(player);
		if (residence == structure)
		{
			return true;
		}
		else
		{
			return false;
		}
	}
	
	
	public static boolean applyManagementMods(obj_id structure, dictionary mods) throws InterruptedException
	{
		if (structure == null || structure == obj_id.NULL_ID)
		{
			return false;
		}
		
		int mm = 0;
		if (mods.containsKey("maintenance_mod"))
		{
			mm = mm + mods.getInt("maintenance_mod");
		}
		
		if (isFactory(structure))
		{
			if (mods.containsKey("factory_mod"))
			{
				mm = mm + mods.getInt("factory_mod");
			}
		}
		
		String template = getTemplateName(structure);
		int idx = getStructureTableIndex(template);
		int maint_rate = dataTableGetInt(PLAYER_STRUCTURE_DATATABLE, idx, DATATABLE_COL_MAINT_RATE);
		
		maint_rate = maint_rate * 100 / (100 + mm);
		
		if (mm > 0)
		{
			setObjVar(structure, VAR_MAINTENANCE_MOD, mm);
		}
		
		return true;
	}
	
	
	public static boolean applyManagementMods(obj_id structure, obj_id player) throws InterruptedException
	{
		if (player == null || player == obj_id.NULL_ID)
		{
			return false;
		}
		
		dictionary mods = new dictionary();
		int maintenance_mod = getSkillStatMod(player, "structure_maintenance_mod");
		if (maintenance_mod > 0)
		{
			mods.put("maintenance_mod", maintenance_mod);
		}
		
		int factory_mod = getSkillStatMod(player, "factory_efficiency");
		
		if (isFactory(structure))
		{
			maintenance_mod += (int)getSkillStatisticModifier(player, "expertise_factory_maintenance_decrease");
		}
		
		if (isHarvester(structure))
		{
			maintenance_mod += (int)getSkillStatisticModifier(player, "expertise_harvester_maintenance_decrease");
		}
		
		if (factory_mod > 0)
		{
			mods.put("factory_mod", maintenance_mod);
		}
		
		return applyManagementMods(structure, mods);
	}
	
	
	public static boolean applyManagementMods(obj_id structure, int mods) throws InterruptedException
	{
		if (!isIdValid(structure))
		{
			return false;
		}
		
		if (mods < 1)
		{
			return false;
		}
		
		dictionary mods_dict = new dictionary();
		mods_dict.put("maintenance_mod", mods);
		
		return applyManagementMods(structure, mods_dict);
	}
	
	
	public static boolean removeManagementMods(obj_id structure) throws InterruptedException
	{
		
		return true;
	}
	
	
	public static boolean isGuildIdOnRawList(String[] nameList, int guildId) throws InterruptedException
	{
		if ((nameList != null) && (guildId != 0))
		{
			
			String guildIdAsString = "guild:"+ guildId;
			
			return ( utils.getElementPositionInArray( nameList, guildIdAsString ) != -1 );
		}
		
		return false;
	}
	
	
	public static boolean isGuildNameOnRawList(String[] nameList, String guildNameToFind) throws InterruptedException
	{
		if (guildNameToFind != null)
		{
			return isGuildIdOnRawList( nameList, findGuild( guildNameToFind.trim().toLowerCase() ) );
		}
		
		return false;
	}
	
	
	public static boolean isPlayerObjIdOnRawList(String[] nameList, obj_id playerObjId) throws InterruptedException
	{
		if ((nameList != null) && isIdValid(playerObjId))
		{
			
			String playerObjIdAsString = playerObjId.toString();
			
			return ( utils.getElementPositionInArray( nameList, playerObjIdAsString ) != -1 );
		}
		
		return false;
	}
	
	
	public static boolean isPlayerNameOnRawList(String[] nameList, String playerNameToFind) throws InterruptedException
	{
		if (playerNameToFind != null)
		{
			return isPlayerObjIdOnRawList( nameList, getPlayerIdFromFirstName( playerNameToFind.trim().toLowerCase() ) );
		}
		
		return false;
	}
	
	
	public static boolean isNameOnRawList(String[] nameList, String nameToFind) throws InterruptedException
	{
		if (nameToFind != null)
		{
			
			if (nameToFind.toLowerCase().indexOf("guild:") > -1)
			{
				
				return isGuildNameOnRawList( nameList, nameToFind.substring(6).toLowerCase().trim() );
			}
			else
			{
				
				return isPlayerNameOnRawList( nameList, nameToFind );
			}
		}
		
		return false;
	}
	
	
	public static boolean isNameOnPermissionList(String[] permissionList, String nameToFind) throws InterruptedException
	{
		if ((permissionList != null) && (nameToFind != null))
		{
			
			if (nameToFind.toLowerCase().indexOf("guild:") > -1)
			{
				
				String guildName = nameToFind.substring(6).toLowerCase().trim();
				
				int guildId = findGuild( guildName );
				
				for (int i = 0; i < permissionList.length; i++)
				{
					testAbortScript();
					
					if (permissionList[i].toLowerCase().indexOf("guild:") > -1)
					{
						String permissionListGuildName = permissionList[i].substring(6).toLowerCase().trim();
						
						if (guildId == 0)
						{
							if (guildName.equals(permissionListGuildName))
							{
								return true;
							}
						}
						else
						{
							
							int permissionListGuildId = findGuild( permissionListGuildName );
							if (guildId == permissionListGuildId)
							{
								return true;
							}
						}
					}
				}
			}
			else
			{
				String playerName = nameToFind.toLowerCase().trim();
				
				for (int i = 0; i < permissionList.length; i++)
				{
					testAbortScript();
					if (permissionList[i].toLowerCase().trim().equals( playerName ))
					{
						return true;
					}
				}
			}
		}
		
		return false;
	}
	
	
	public static boolean addGuildIdToRawList(Vector nameList, int guildId) throws InterruptedException
	{
		if (guildId != 0)
		{
			
			if (nameList == null)
			{
				return false;
			}
			
			String guildIdAsString = "guild:"+ guildId;
			
			if (utils.getElementPositionInArray( nameList, guildIdAsString ) == -1)
			{
				nameList.add( guildIdAsString );
				return true;
			}
			
			return true;
		}
		
		return false;
	}
	
	
	public static boolean addGuildNameToRawList(Vector nameList, String guildNameToAdd) throws InterruptedException
	{
		if (guildNameToAdd != null)
		{
			return addGuildIdToRawList( nameList, findGuild( guildNameToAdd.trim().toLowerCase() ) );
		}
		
		return false;
	}
	
	
	public static boolean addPlayerObjIdToRawList(Vector nameList, obj_id playerObjId) throws InterruptedException
	{
		if (isIdValid(playerObjId))
		{
			
			if (nameList == null)
			{
				return false;
			}
			
			String playerObjIdAsString = playerObjId.toString();
			
			if (utils.getElementPositionInArray( nameList, playerObjIdAsString ) == -1)
			{
				nameList.add( playerObjIdAsString );
				return true;
			}
		}
		
		return false;
	}
	
	
	public static boolean addPlayerNameToRawList(Vector nameList, String playerNameToAdd) throws InterruptedException
	{
		if (playerNameToAdd != null)
		{
			return addPlayerObjIdToRawList( nameList, getPlayerIdFromFirstName( playerNameToAdd.trim().toLowerCase() ) );
		}
		
		return false;
	}
	
	
	public static boolean addNameToRawList(Vector nameList, String nameToAdd) throws InterruptedException
	{
		if (nameToAdd != null)
		{
			
			if (nameToAdd.toLowerCase().indexOf("guild:") > -1)
			{
				
				return addGuildNameToRawList( nameList, nameToAdd.substring(6).toLowerCase().trim() );
			}
			else
			{
				
				return addPlayerNameToRawList( nameList, nameToAdd );
			}
		}
		
		return false;
	}
	
	
	public static boolean removeGuildIdFromRawList(Vector nameList, int guildId) throws InterruptedException
	{
		if ((nameList != null) && (guildId != 0))
		{
			
			String guildIdAsString = "guild:"+ guildId;
			
			int deleteIndex = utils.getElementPositionInArray( nameList, guildIdAsString );
			if (deleteIndex != -1)
			{
				
				nameList.remove( deleteIndex );
				return true;
			}
		}
		
		return false;
	}
	
	
	public static boolean removeGuildNameFromRawList(Vector nameList, String guildNameToRemove) throws InterruptedException
	{
		if (guildNameToRemove != null)
		{
			return removeGuildIdFromRawList( nameList, findGuild( guildNameToRemove.trim().toLowerCase() ) );
		}
		
		return false;
	}
	
	
	public static boolean removePlayerObjIdFromRawList(Vector nameList, obj_id playerObjId) throws InterruptedException
	{
		if ((nameList != null) && isIdValid(playerObjId))
		{
			
			String playerObjIdAsString = playerObjId.toString();
			
			int deleteIndex = utils.getElementPositionInArray( nameList, playerObjIdAsString );
			if (deleteIndex != -1)
			{
				
				nameList.remove( deleteIndex );
				return true;
			}
		}
		
		return false;
	}
	
	
	public static boolean removePlayerNameFromRawList(Vector nameList, String playerNameToRemove) throws InterruptedException
	{
		if (playerNameToRemove != null)
		{
			return removePlayerObjIdFromRawList( nameList, getPlayerIdFromFirstName( playerNameToRemove.trim().toLowerCase() ) );
		}
		
		return false;
	}
	
	
	public static boolean removeNameFromRawList(Vector nameList, String nameToRemove) throws InterruptedException
	{
		if (nameToRemove != null)
		{
			
			if (nameToRemove.toLowerCase().indexOf("guild:") > -1)
			{
				
				return removeGuildNameFromRawList( nameList, nameToRemove.substring(6).toLowerCase().trim() );
			}
			else
			{
				
				return removePlayerNameFromRawList( nameList, nameToRemove );
			}
		}
		
		return false;
	}
	
	
	public static String[] convertRawListToNames(String[] nameListToConvert) throws InterruptedException
	{
		Vector converted_list = new Vector();
		converted_list.setSize(0);
		
		if (nameListToConvert != null)
		{
			for (int i = 0; i < nameListToConvert.length; i++)
			{
				testAbortScript();
				
				if (nameListToConvert[i].toLowerCase().indexOf("guild:") > -1)
				{
					String guildIdString = nameListToConvert[i].substring(6).toLowerCase().trim();
					
					int guildId = utils.stringToInt( guildIdString );
					if (guildId > 0)
					{
						String guildAbbrev = guildGetAbbrev( guildId );
						if ((guildAbbrev != null) && (!guildAbbrev.equals("")))
						{
							converted_list.add( "Guild:"+ guildAbbrev );
						}
					}
				}
				else
				{
					
					obj_id player = utils.stringToObjId(nameListToConvert[i]);
					if (isIdValid(player))
					{
						
						String player_full_name = getPlayerFullName( player );
						if (player_full_name != null)
						{
							java.util.StringTokenizer st = new java.util.StringTokenizer(player_full_name);
							if (st.hasMoreTokens())
							{
								String player_first_name = st.nextToken();
								converted_list.add( player_first_name );
							}
						}
					}
				}
			}
		}
		
		return (String[])converted_list.toArray(new String[0]);
	}
	
	
	public static boolean modifyList(obj_id structure, String player_or_guild_name, obj_id admin, String objvar, boolean verbose) throws InterruptedException
	{
		
		if (!isIdValid(structure))
		{
			return false;
		}
		
		if (!isIdValid(admin) && verbose)
		{
			return false;
		}
		
		if (isIdValid(admin))
		{
			if (!isAdmin(structure, admin))
			{
				
				if (verbose)
				{
					sendSystemMessage(admin, new string_id(STF_FILE, "must_be_admin"));
				}
				return false;
			}
		}
		
		if (player_or_guild_name == null)
		{
			return false;
		}
		
		boolean is_name_on_list = false;
		if (objvar.equals(VAR_ENTER_LIST))
		{
			is_name_on_list = isNameOnEntryList(structure, player_or_guild_name);
		}
		else if (objvar.equals(VAR_BAN_LIST))
		{
			is_name_on_list = isNameOnBanList(structure, player_or_guild_name);
		}
		else if (objvar.equals(VAR_ADMIN_LIST))
		{
			is_name_on_list = isNameOnAdminList(structure, player_or_guild_name);
		}
		else if (objvar.equals(VAR_HOPPER_LIST))
		{
			is_name_on_list = isNameOnHopperList(structure, player_or_guild_name);
		}
		else
		{
			return false;
		}
		
		if (!is_name_on_list)
		{
			
			{
				String[] listOfNames = new String[0];
				
				if (objvar.equals(VAR_ENTER_LIST))
				{
					listOfNames = getEntryList(structure);
				}
				else if (objvar.equals(VAR_BAN_LIST))
				{
					listOfNames = getBanList(structure);
				}
				else if (objvar.equals(VAR_ADMIN_LIST))
				{
					listOfNames = getAdminListRaw(structure);
				}
				else if (objvar.equals(VAR_HOPPER_LIST))
				{
					listOfNames = getHopperListRaw(structure);
				}
				
				if (listOfNames != null)
				{
					if (listOfNames.length >= MAX_LIST_SIZE)
					{
						if (verbose && isIdValid(admin))
						{
							
							sendSystemMessage(admin, new string_id(STF_FILE, "too_many_entries"));
						}
						return false;
					}
				}
			}
			
			if (player_or_guild_name.toLowerCase().indexOf("guild:") > -1)
			{
				
				if (objvar.equals(VAR_ADMIN_LIST) || objvar.equals(VAR_HOPPER_LIST))
				{
					
					if (verbose && isIdValid(admin))
					{
						sendSystemMessageProse(admin, prose.getPackage(new string_id (STF_FILE, "modify_list_invalid_player"), player_or_guild_name));
					}
					return false;
				}
				
				int guild_id = findGuild(player_or_guild_name.substring(6).toLowerCase().trim());
				if (guild_id == 0)
				{
					if (verbose && isIdValid(admin))
					{
						sendSystemMessageProse(admin, prose.getPackage(new string_id (STF_FILE, "modify_list_invalid_guild"), player_or_guild_name.substring(6).trim()));
					}
					return false;
				}
			}
			else
			{
				if (!isIdValid(utils.stringToObjId(player_or_guild_name)))
				{
					
					obj_id player_objId = getPlayerIdFromFirstName(player_or_guild_name.toLowerCase().trim());
					if (!isIdValid(player_objId))
					{
						if (verbose && isIdValid(admin))
						{
							sendSystemMessageProse(admin, prose.getPackage(new string_id (STF_FILE, "modify_list_invalid_player"), player_or_guild_name));
						}
						return false;
					}
				}
			}
			
			if (objvar.equals(VAR_ENTER_LIST))
			{
				permissionsAddAllowed(structure, player_or_guild_name);
			}
			else if (objvar.equals(VAR_BAN_LIST))
			{
				permissionsAddBanned(structure, player_or_guild_name);
			}
			else if (objvar.equals(VAR_ADMIN_LIST))
			{
				adminListAddName(structure, player_or_guild_name);
			}
			else if (objvar.equals(VAR_HOPPER_LIST))
			{
				hopperListAddName(structure, player_or_guild_name);
			}
			
			if (verbose && isIdValid(admin))
			{
				sendSystemMessageProse(admin, prose.getPackage(new string_id (STF_FILE, "player_added"), player_or_guild_name));
			}
			
			if (objvar.equals(VAR_ADMIN_LIST))
			{
				
				if (isBuilding(structure))
				{
					if (!isNameOnEntryList(structure, player_or_guild_name))
					{
						modifyList(structure, player_or_guild_name, admin, VAR_ENTER_LIST, false);
					}
				}
				
				if (isInstallation(structure))
				{
					if (!isNameOnHopperList(structure, player_or_guild_name))
					{
						modifyList(structure, player_or_guild_name, admin, VAR_HOPPER_LIST, false);
					}
				}
			}
			
			if (objvar.equals(VAR_BAN_LIST))
			{
				
				if (isBuilding(structure))
				{
					if (isNameOnEntryList(structure, player_or_guild_name))
					{
						modifyList(structure, player_or_guild_name, admin, VAR_ENTER_LIST, false);
					}
				}
				
				if (isInstallation(structure))
				{
					if (isNameOnHopperList(structure, player_or_guild_name))
					{
						modifyList(structure, player_or_guild_name, admin, VAR_HOPPER_LIST, false);
					}
				}
			}
		}
		else
		{
			if (objvar.equals(VAR_ENTER_LIST))
			{
				permissionsRemoveAllowed(structure, player_or_guild_name);
			}
			else if (objvar.equals(VAR_BAN_LIST))
			{
				permissionsRemoveBanned(structure, player_or_guild_name);
			}
			else if (objvar.equals(VAR_ADMIN_LIST))
			{
				adminListRemoveName(structure, player_or_guild_name);
			}
			else if (objvar.equals(VAR_HOPPER_LIST))
			{
				hopperListRemoveName(structure, player_or_guild_name);
			}
			
			if (verbose && isIdValid(admin))
			{
				sendSystemMessageProse(admin, prose.getPackage(new string_id (STF_FILE, "player_removed"), player_or_guild_name));
			}
		}
		
		return true;
	}
	
	
	public static boolean modifyEntryList(obj_id structure, obj_id player, obj_id admin) throws InterruptedException
	{
		if (isIdValid(player))
		{
			return modifyEntryList( structure, getPlayerName( player ), admin );
		}
		else
		{
			
			return false;
		}
	}
	
	
	public static boolean modifyEntryList(obj_id structure, String player_or_guild_name, obj_id admin) throws InterruptedException
	{
		if (!isBuilding(structure))
		{
			
			sendSystemMessage(admin, new string_id(STF_FILE, "no_entry_list"));
			return false;
		}
		
		if (canEnter(structure, player_or_guild_name))
		{
			
			if (isAdmin(structure, player_or_guild_name))
			{
				
				sendSystemMessage(admin, new string_id(STF_FILE, "no_remove_admin"));
				return false;
			}
		}
		else
		{
			
			if (isBanned(structure, player_or_guild_name))
			{
				
				sendSystemMessage(admin, new string_id(STF_FILE, "no_banned"));
				return false;
			}
		}
		
		return modifyList(structure, player_or_guild_name, admin, VAR_ENTER_LIST, true);
	}
	
	
	public static boolean modifyBanList(obj_id structure, String player_or_guild_name, obj_id admin) throws InterruptedException
	{
		
		if (!isBuilding(structure))
		{
			
			sendSystemMessage(admin, new string_id(STF_FILE, "no_banned"));
			return false;
		}
		
		if (!isAdmin(structure, admin))
		{
			
			sendSystemMessage(admin, new string_id(STF_FILE, "admin_only_modify"));
			return false;
		}
		
		if (isAdmin(structure, player_or_guild_name))
		{
			
			sendSystemMessage(admin, new string_id(STF_FILE, "cannot_ban_admin"));
			return false;
		}
		
		return modifyList(structure, player_or_guild_name, admin, VAR_BAN_LIST, true);
	}
	
	
	public static boolean modifyBanList(obj_id structure, obj_id player, obj_id admin) throws InterruptedException
	{
		if (isIdValid(player))
		{
			boolean result = modifyBanList( structure, getPlayerName( player ), admin );
			if (result)
			{
				
				if (player.isLoaded() && player.isAuthoritative())
				{
					obj_id ban_structure = getStructure(player);
					
					if (ban_structure.equals( structure ))
					{
						expelFromBuilding(player);
					}
				}
			}
			
			return result;
		}
		else
		{
			
			return false;
		}
	}
	
	
	public static boolean modifyAdminList(obj_id structure, obj_id player, obj_id admin) throws InterruptedException
	{
		return modifyAdminList( structure, getPlayerName( player ), admin );
	}
	
	
	public static boolean modifyAdminList(obj_id structure, String player_or_guild_name, obj_id admin) throws InterruptedException
	{
		if (player_or_guild_name.equals(getPlayerName(admin)))
		{
			
			sendSystemMessage(admin, new string_id(STF_FILE, "cannot_remove_self"));
			return false;
			
		}
		
		if (isOwner(structure, player_or_guild_name))
		{
			
			sendSystemMessage(admin, new string_id(STF_FILE, "cannot_remove_owner"));
			return false;
		}
		
		if (isBanned(structure, player_or_guild_name))
		{
			
			sendSystemMessageProse(admin, prose.getPackage(new string_id (STF_FILE, "cannot_add_banned"), player_or_guild_name));
			return false;
		}
		
		return modifyList(structure, player_or_guild_name, admin, VAR_ADMIN_LIST, true);
	}
	
	
	public static boolean modifyVendorList(obj_id structure, String player_name, obj_id admin) throws InterruptedException
	{
		return false;
	}
	
	
	public static boolean modifyVendorList(obj_id structure, obj_id player, obj_id admin) throws InterruptedException
	{
		String player_name = getPlayerName(player);
		return modifyVendorList(structure, player_name, admin);
	}
	
	
	public static boolean modifyHopperList(obj_id structure, obj_id player, obj_id admin) throws InterruptedException
	{
		return modifyHopperList( structure, getPlayerName( player ), admin );
	}
	
	
	public static boolean modifyHopperList(obj_id structure, String player_or_guild_name, obj_id admin) throws InterruptedException
	{
		if (!isInstallation(structure))
		{
			
			sendSystemMessage(admin, new string_id(STF_FILE, "only_installations"));
			return false;
		}
		
		if (canHopper(structure, player_or_guild_name))
		{
			
			if (isAdmin(structure, player_or_guild_name))
			{
				
				sendSystemMessage(admin, new string_id(STF_FILE, "hopper_cannot_remove_admin"));
				return false;
			}
		}
		else
		{
			
			if (isBanned(structure, player_or_guild_name))
			{
				
				sendSystemMessage(admin, new string_id(STF_FILE, "no_banned_vendors"));
				return false;
			}
		}
		
		return modifyList(structure, player_or_guild_name, admin, VAR_HOPPER_LIST, true);
	}
	
	
	public static int getStructureTableIndex(String template) throws InterruptedException
	{
		if (template == null || template.equals(""))
		{
			return -1;
		}
		
		int idx = dataTableSearchColumnForString(template, DATATABLE_COL_STRUCTURE, PLAYER_STRUCTURE_DATATABLE);
		if (idx == -1)
		{
			idx = dataTableSearchColumnForString(template, DATATABLE_COL_FOOTPRINT, PLAYER_STRUCTURE_DATATABLE);
		}
		
		return idx;
	}
	
	
	public static obj_id addStructureWaypoint(obj_id player, location loc, String name, obj_id structure) throws InterruptedException
	{
		LOG("LOG_CHANNEL", "player_structure.addStructureWaypoint -- "+ name);
		
		if (!isIdValid(player))
		{
			return null;
		}
		
		if (loc == null)
		{
			return null;
		}
		
		if (name == null)
		{
			return null;
		}
		
		obj_id waypoint = createWaypointInDatapad(player, loc);
		if (!isIdValid(waypoint))
		{
			return null;
		}
		
		setWaypointVisible(waypoint, true);
		setWaypointActive(waypoint, true);
		setWaypointName(waypoint, name);
		
		if (structure.isLoaded())
		{
			setObjVar(structure, VAR_WAYPOINT_STRUCTURE, waypoint);
		}
		else
		{
			dictionary params = new dictionary();
			params.put("waypoint", waypoint);
			messageTo(structure, "msgAddWaypointData", params, 0.0f, true);
		}
		
		return waypoint;
	}
	
	
	public static boolean payMaintenance(obj_id player, obj_id structure, int amt) throws InterruptedException
	{
		
		if (!isIdValid(player) || !isIdValid(structure))
		{
			return false;
		}
		
		if (amt < 1 || amt > 100000)
		{
			return false;
		}
		
		if (isOwner(structure, player))
		{
			doOldToNewLotConversion( player, structure );
		}
		
		return money.pay(player, structure, amt, "handlePayment", null);
	}
	
	
	public static boolean withdrawMaintenance(obj_id player, obj_id structure, int amt) throws InterruptedException
	{
		if (!isIdValid(player) || !isIdValid(structure))
		{
			return false;
		}
		
		if (amt < 1)
		{
			return false;
		}
		
		dictionary d = new dictionary();
		d.put("player", player);
		d.put("structure", structure);
		d.put("amount", amt);
		
		CustomerServiceLog("playerStructure", "%TU has initiated a withdraw of "+ amt + " from structure "+ structure + ".", player, null);
		
		return transferBankCreditsTo(structure, player, amt, "msgPAWithdrawSuccess", "msgPAWithdrawFail", d);
	}
	
	
	public static int decrementMaintenancePool(obj_id structure, int amt) throws InterruptedException
	{
		if (getConfigSetting("GameServer", "disableStructureFees") != null)
		{
			return 1;
		}
		
		if (!isIdValid(structure))
		{
			return -1;
		}
		
		if (amt < 1)
		{
			return -1;
		}
		
		int pool = getBankBalance(structure);
		if (amt > pool)
		{
			return -2;
		}
		
		int property_tax = city.getPropertyTax(structure);
		int pretax_amt = 0, tax_amt = 0;
		int base_amt = amt;
		if (property_tax > 0)
		{
			pretax_amt = Math.round(amt / (1+(property_tax/100.f)));
			tax_amt = base_amt - pretax_amt;
			
			if (money.bankTo(structure, money.ACCT_STRUCTURE_MAINTENANCE, pretax_amt))
			{
				pool -= pretax_amt;
				if ((tax_amt > 0) && money.bankTo( structure, city.getCityHall(structure), tax_amt ))
				{
					pool -= tax_amt;
				}
				return pool;
			}
		}
		else
		{
			if (money.bankTo(structure, money.ACCT_STRUCTURE_MAINTENANCE, amt))
			{
				pool -= amt;
				return pool;
			}
		}
		
		return -1;
	}
	
	
	public static int damageStructure(obj_id structure, int amt, boolean boolCondemn) throws InterruptedException
	{
		if (structure == null || structure == obj_id.NULL_ID)
		{
			return -1;
		}
		
		if (amt < 1)
		{
			return -1;
		}
		
		int condition = getStructureCondition(structure);
		condition = condition - amt;
		
		if (condition < 1)
		{
			condition = 0;
			if (hasCondition( structure, CONDITION_VENDOR ))
			{
				destroyObject( structure );
			}
			else
			{
				
				if (isBuilding(structure) || isFactory(structure))
				{
					if (boolCondemn)
					{
						setInvulnerableHitpoints(structure, 1);
						return 0;
					}
				}
				CustomerServiceLog("playerStructure","Structure "+ structure + " destroyed for lack of maintenance.");
				destroyStructure(structure);
			}
		}
		else
		{
			
			setInvulnerableHitpoints(structure, condition);
		}
		
		return condition;
	}
	
	
	public static int damageStructure(obj_id structure, int amt) throws InterruptedException
	{
		return damageStructure(structure, amt, false);
	}
	
	
	public static int repairStructure(obj_id structure, int amt) throws InterruptedException
	{
		if (structure == null || structure == obj_id.NULL_ID)
		{
			LOG("building", "Returning 1");
			return -1;
		}
		
		if (amt < 1)
		{
			LOG("building", "Returning 12");
			return -1;
		}
		
		int condition = getStructureCondition(structure);
		
		int max_condition = getMaxCondition(structure);
		
		condition = condition + amt;
		LOG("building", "new condition is "+condition);
		
		if (condition > max_condition)
		{
			condition = max_condition;
		}
		
		setInvulnerableHitpoints(structure, condition);
		
		if (isStructureCondemned(structure))
		{
			removeObjVar(structure, "player_structure.condemned");
		}
		
		return condition;
	}
	
	
	public static boolean initiateRepairCheck(obj_id structure) throws InterruptedException
	{
		if (!isIdValid(structure))
		{
			return false;
		}
		
		int condition = getStructureCondition(structure);
		int max_condition = getMaxCondition(structure);
		if (condition != max_condition)
		{
			int damage = max_condition - condition;
			int maint_pool = getBankBalance(structure);
			
			float per_point_cost = getBaseMaintenanceRate( structure ) / 3.0f;
			
			int repair_cost = (int)(damage * per_point_cost);
			int amt_paid = 0;
			int amt_repaired = 0;
			
			if (maint_pool >= per_point_cost)
			{
				LOG("LOG_CHANNEL", "permanent_structure::OnMaintenanceLoop -- initiating repairs.");
				LOG("LOG_CHANNEL", "cond ->"+ condition);
				if (maint_pool < repair_cost)
				{
					
					amt_repaired = (int)(maint_pool / per_point_cost);
					amt_paid = maint_pool;
				}
				else
				{
					
					amt_paid = (int)(damage * per_point_cost);
					amt_repaired = damage;
				}
				
				int result = decrementMaintenancePool(structure, amt_paid);
				if (result > -1)
				{
					LOG("LOG_CHANNEL", "amt_paid ->"+ amt_paid + " amt_repaired ->"+ amt_repaired);
					condition = repairStructure(structure, amt_repaired);
					LOG("LOG_CHANNEL", "2cond ->"+ condition);
					
				}
				else
				{
					LOG("LOG_CHANNEL", "permanent_structure::OnMaintenanceLoop -- unable to pay repair costs.");
				}
			}
		}
		
		return true;
	}
	
	
	public static boolean sendMaintenanceMail(obj_id structure) throws InterruptedException
	{
		if (!isIdValid(structure))
		{
			return false;
		}
		
		int last_message = 0;
		if (hasObjVar(structure, VAR_LAST_MESSAGE))
		{
			last_message = getIntObjVar(structure, VAR_LAST_MESSAGE);
		}
		
		if (last_message <= getGameTime() && !isStructureCondemned(structure))
		{
			int condition = player_structure.getStructureCondition(structure);
			int max_condition = player_structure.getMaxCondition(structure);
			int perc_condition = (condition * 100) / max_condition;
			
			if (perc_condition <= 90)
			{
				
				obj_id owner = getStructureOwnerObjId(structure);
				if (isIdValid(owner))
				{
					location loc = getLocation(structure);
					String area = loc.area;
					if (area != null)
					{
						area = area.substring(0,1).toUpperCase() + area.substring(1);
					}
					
					String loc_str = "("+ loc.x + ", "+ loc.z + " "+ loc.area + ")";
					string_id strSpam = player_structure.SID_MAIL_STRUCTURE_DAMAGE;
					prose_package pp = prose.getPackage(strSpam, null, structure, loc_str, perc_condition);
					utils.sendMail(player_structure.SID_MAIL_STRUCTURE_DAMAGE_SUB, pp, getPlayerName(owner), "@player_structure:management");
					utils.setObjVar(structure, VAR_LAST_MESSAGE, getGameTime()+ MAIL_WARNING_INTERVAL);
					return true;
				}
			}
			else if (hasObjVar(structure, VAR_LAST_MESSAGE))
			{
				removeObjVar(structure, VAR_LAST_MESSAGE);
			}
		}
		
		return false;
	}
	
	
	public static int destroyStructure(obj_id structure, obj_id containerForDeed, boolean reclaim, boolean overloaded) throws InterruptedException
	{
		if (!isValidId(structure) || !exists(structure))
		{
			return DESTROY_RESULT_INVALID_STRUCTURE;
		}
		
		String template = getTemplateName(structure);
		obj_id owner = getStructureOwnerObjId(structure);
		obj_id self = getSelf();
		
		String sname = getStructureName( structure );
		CustomerServiceLog("playerStructure","Initiating destroy for structure "+ structure + " (Template: "+ template + " Name: "+ sname + "). Owner is "+ getStructureOwner(structure) + " ("+ owner + ") Location: "+ getLocation(structure) + " Reclaim: "+ reclaim);
		
		boolean bGetSelfPowered = false;
		
		if (hasObjVar(structure, "selfpowered"))
		{
			bGetSelfPowered = true;
		}
		
		if (reclaim)
		{
			
			obj_id deed = null;
			boolean isStaticItemDeed = false;
			String deed_template = getDeedTemplate(structure);
			if (deed_template == null || deed_template.equals(""))
			{
				CustomerServiceLog("playerStructure","Attempting to reclaim deed, but can't find deed information for "+ structure + ". Owner is "+ getStructureOwner(structure) + "("+ owner + ") Location: "+ getLocation(structure));
				return DESTROY_RESULT_FAIL_COULD_NOT_DETERMINE_DEED_FOR_STRUCTURE;
			}
			
			if (!overloaded && getVolumeFree(containerForDeed) < 1)
			{
				return DESTROY_RESULT_FAIL_TARGET_CONTAINER_FULL;
			}
			
			if (!overloaded && getVolumeFree(containerForDeed) < 2 && bGetSelfPowered)
			{
				return DESTROY_RESULT_FAIL_TARGET_CONTAINER_FULL_SELFPOWERED;
			}
			
			if (hasObjVar(structure, TCG_STATIC_ITEM_DEED_OBJVAR))
			{
				CustomerServiceLog("playerStructure", "DestroyStructure -- Player has TCG STRUCTURE: "+ structure + ". Owner is %TU. destroyStructure initiated by %TT", owner, self);
				
				String staticItemDeed = getStringObjVar(structure, TCG_STATIC_ITEM_DEED_OBJVAR);
				if (staticItemDeed == null || staticItemDeed.equals(""))
				{
					return DESTROY_RESULT_INVALID_STATIC_ITEM;
				}
				
				if (!static_item.isStaticItem(staticItemDeed))
				{
					return DESTROY_RESULT_INVALID_STATIC_ITEM;
				}
				
				deed = static_item.createNewItemFunction(staticItemDeed, containerForDeed);
				isStaticItemDeed = true;
				CustomerServiceLog("playerStructure", "DestroyStructure -- Owner has TCG STRUCTURE: "+ structure + " and has the correct static item: "+staticItemDeed+". Owner is %TU. destroyStructure initiated by %TT", owner, self);
			}
			else
			{
				deed = overloaded ? createObjectOverloaded(deed_template, containerForDeed) : createObject(deed_template, containerForDeed, "");
			}
			
			if (!isValidId(deed) || !exists(deed))
			{
				CustomerServiceLog("playerStructure","structure("+structure+")|owner("+owner+") was unable to create the deed for caller("+self+") and is dumping out without destroying the structure");
				return DESTROY_RESULT_FAIL_COULD_NOT_CREATE_DEED;
			}
			
			if (!isStaticItemDeed)
			{
				setDeedTemplate(deed, template);
				setObjVar(deed, VAR_DEED_BUILDTIME, getDeedBuildTime(structure));
				String deed_scene = getDeedScene( structure );
				
				if (deed_scene != null)
				{
					if (deed_scene.equals( "tatooine" ))
					{
						deed_scene = "tatooine,lok,dantooine";
					}
					else if (deed_scene.equals( "naboo" ))
					{
						deed_scene = "naboo,rori,dantooine";
					}
					else if (deed_scene.equals( "corellia" ))
					{
						deed_scene = "corellia,talus";
					}
				}
				setObjVar(deed, VAR_DEED_SCENE, deed_scene);
			}
			
			if (hasObjVar(structure, "structureChange.storageIncrease"))
			{
				int storageIncrease = getIntObjVar(structure, "structureChange.storageIncrease");
				setObjVar( deed, "structureChange.storageIncrease", storageIncrease);
				setObjVar( deed, "noTrade", 1);
				attachScript( deed, "item.special.nomove");
				
			}
			
			int maxExtractionRate = getIntObjVar(structure, "max_extraction");
			if (maxExtractionRate != -1 && isHarvester(structure) || isGenerator(structure))
			{
				setObjVar(deed, VAR_DEED_MAX_EXTRACTION, maxExtractionRate);
			}
			
			int currentExtractionRate = getIntObjVar(structure, "current_extraction");
			if (currentExtractionRate != -1 && isHarvester(structure) || isGenerator(structure))
			{
				setObjVar(deed, VAR_DEED_CURRENT_EXTRACTION, currentExtractionRate);
			}
			
			int maxHopperAmount = getIntObjVar(structure, VAR_DEED_MAX_HOPPER);
			
			if (maxHopperAmount > 0)
			{
				setObjVar(deed, VAR_DEED_MAX_HOPPER, maxHopperAmount);
			}
			
			int maintPool = getBankBalance(structure);
			int reclaimCost = getRedeedCost(structure);
			transferBankCreditsToNamedAccount(structure, money.ACCT_DEED_RECLAIM, reclaimCost, "noHandler", "noHandler", new dictionary());
			
			int surplus = maintPool - reclaimCost;
			if (surplus > 0)
			{
				setObjVar(deed, VAR_DEED_SURPLUS_MAINTENANCE, surplus);
				transferBankCreditsToNamedAccount(structure, money.ACCT_STRUCTURE_DESTROYED, surplus, "noHandler", "noHandler", new dictionary());
				CustomerServiceLog("playerStructure", "DestroyStructure -- transferring maintenance to deed "+ deed + " from structure "+ structure + " Amount: "+ surplus + ". Owner is %TU. destroyStructure initiated by %TT", owner, self);
			}
			
			float power = getPowerValue(structure);
			
			if (power > 0f && (isFactory(structure) || isHarvester(structure) || isGenerator(structure)))
			{
				setObjVar(deed, VAR_DEED_SURPLUS_POWER, power);
			}
			
		}
		else
		{
			if (isIdValid(containerForDeed) && getVolumeFree(containerForDeed)<1 && bGetSelfPowered)
			{
				return DESTROY_RESULT_FAIL_TARGET_CONTAINER_FULL_SELFPOWERED;
			}
			
			transferBankCreditsToNamedAccount(structure, money.ACCT_STRUCTURE_DESTROYED, getBankBalance(structure), "noHandler", "noHandler", new dictionary());
		}
		
		if (bGetSelfPowered)
		{
			String veteranRewardDeedToReturn = null;
			if (isEliteHarvester(structure))
			{
				veteranRewardDeedToReturn = SELFPOWERED_DEED_ELITE;
			}
			else
			{
				veteranRewardDeedToReturn = SELFPOWERED_DEED;
			}
			
			if (overloaded)
			{
				createObjectOverloaded( veteranRewardDeedToReturn, containerForDeed);
			}
			else
			{
				createObject( veteranRewardDeedToReturn, containerForDeed, "");
			}
		}
		
		city.removeStructureFromCity( structure );
		
		if (isBuilding(structure))
		{
			obj_id[] players = getPlayersInBuilding(structure);
			if (players != null)
			{
				for (int i = 0; i < players.length; i++)
				{
					testAbortScript();
					
					expelFromBuilding(players[i]);
				}
			}
			
		}
		destroyBaseObjects(structure);
		destroyStructureSign(structure);
		
		dictionary params = new dictionary();
		params.put("structure", structure);
		params.put("template", template);
		if (hasObjVar(structure, VAR_WAYPOINT_STRUCTURE))
		{
			params.put("waypoint", getObjIdObjVar(structure, VAR_WAYPOINT_STRUCTURE));
		}
		
		adjustLotsForOfflinePlayer( self, structure );
		
		messageTo(owner, "OnRemoveStructure", params, 0.1f, true);
		
		if (vendor_lib.isVendorPackUpEnabled())
		{
			
			String[] cells = getCellNames(structure);
			if (cells != null)
			{
				for (int i = 0; i < cells.length; i++)
				{
					testAbortScript();
					obj_id cellid = getCellId(structure, cells[i]);
					obj_id contents[] = getContents(cellid);
					if (contents != null)
					{
						for (int j=0; j<contents.length; j++)
						{
							testAbortScript();
							if (hasCondition(contents[j], CONDITION_VENDOR))
							{
								obj_id vendorOwner = getObjIdObjVar(contents[j], "vendor_owner");
								if (!isIdValid(vendorOwner))
								{
									vendorOwner = getOwner( contents[j] );
								}
								
								vendor_lib.finalizePackUp(vendorOwner, contents[j], self, false);
							}
						}
					}
				}
			}
		}
		
		destroyObject(structure);
		return bGetSelfPowered ? DESTROY_RESULT_SUCCESS_SELFPOWERED : DESTROY_RESULT_SUCCESS;
	}
	
	
	public static boolean destroyStructure(obj_id structure, boolean reclaim) throws InterruptedException
	{
		return destroyStructure(structure, reclaim, false);
	}
	
	public static boolean destroyStructure(obj_id structure, boolean reclaim, boolean overrideOwner) throws InterruptedException
	{
		obj_id owner = overrideOwner ? getSelf() : getStructureOwnerObjId(structure);
		if (!isIdValid(owner))
		{
			return false;
		}
		obj_id self = getSelf();
		if (isPlayer(self) && !isGod(self) && self != owner)
		{
			sendSystemMessage(self, new string_id(STF_FILE, "destroy_must_be_owner"));
			return false;
		}
		
		obj_id inv = utils.getInventoryContainer(self);
		
		int resultCode = destroyStructure(structure, inv, reclaim, false);
		switch(resultCode)
		{
			case DESTROY_RESULT_FAIL_COULD_NOT_CREATE_DEED:
			sendSystemMessage(self, new string_id(STF_FILE, "repack_design_error"));
			return false;
			case DESTROY_RESULT_FAIL_COULD_NOT_DETERMINE_DEED_FOR_STRUCTURE:
			sendSystemMessage(self, new string_id(STF_FILE, "contact_cs"));
			return false;
			case DESTROY_RESULT_FAIL_TARGET_CONTAINER_FULL:
			sendSystemMessage(self, new string_id(STF_FILE, "inventory_full"));
			return false;
			case DESTROY_RESULT_FAIL_TARGET_CONTAINER_FULL_SELFPOWERED:
			sendSystemMessage(self, new string_id(STF_FILE, "inventory_full_selfpowered"));
			return false;
			case DESTROY_RESULT_SUCCESS_SELFPOWERED :
			sendSystemMessage(self, new string_id(STF_FILE, "selfpowered"));
			break;
			case DESTROY_RESULT_INVALID_STATIC_ITEM :
			sendSystemMessage(self, new string_id(STF_FILE, "invalid_static_item"));
			return false;
			case DESTROY_RESULT_INVALID_CONTAINER :
			sendSystemMessage(self, new string_id(STF_FILE, "inventory_invalid"));
			return false;
		}
		
		return true;
	}
	
	
	public static boolean destroyStructure(obj_id structure) throws InterruptedException
	{
		return destroyStructure(structure, false);
	}
	
	
	public static boolean destroyBaseObjects(obj_id structure) throws InterruptedException
	{
		if (structure == null || structure == obj_id.NULL_ID)
		{
			return false;
		}
		
		obj_id[] base_objects = getObjIdArrayObjVar(structure, VAR_BASE_OBJECT);
		if (base_objects == null)
		{
			
			return false;
		}
		for (int i = 0; i < base_objects.length; i++)
		{
			testAbortScript();
			if (isIdValid(base_objects[i]))
			{
				destroyObject(base_objects[i]);
			}
		}
		
		return true;
	}
	
	
	public static boolean validateBaseObjects(obj_id structure) throws InterruptedException
	{
		if (!isIdValid(structure))
		{
			return false;
		}
		
		boolean create_objects = false;
		obj_id[] base_objects = getStructureBaseObjects(structure);
		if (base_objects != null)
		{
			for (int i = 0; i < base_objects.length; i++)
			{
				testAbortScript();
				if (!isIdValid(base_objects[i]) || !base_objects[i].isLoaded())
				{
					create_objects = true;
				}
				else
				{
					obj_id container = getTopMostContainer(base_objects[i]);
					if (container == null || structure != container)
					{
						create_objects = true;
					}
				}
			}
		}
		
		if (create_objects)
		{
			destroyBaseObjects(structure);
			float yaw = getYaw(structure);
			if (yaw < 0.0f)
			{
				yaw = yaw + 360.0f;
			}
			
			int rotation = (int)(yaw + 1) / 90;
			createStructureObjects(structure, rotation);
		}
		
		return true;
	}
	
	
	public static int[] convertSecondsTime(int time) throws InterruptedException
	{
		if (time < 1)
		{
			return null;
		}
		
		if (time == Integer.MAX_VALUE)
		{
			int[] ret =
			{
				Integer.MAX_VALUE
			};
			return ret;
		}
		
		int mod_day = time % 86400;
		int mod_hour = mod_day % 3600;
		
		int days = time / 86400;
		int hours = mod_day / 3600;
		int minutes = mod_hour / 60;
		int seconds = mod_hour % 60;
		
		int[] converted_time =
		{
			days, hours, minutes, seconds
		};
		
		return converted_time;
	}
	
	
	public static String assembleTimeRemaining(int[] time_values) throws InterruptedException
	{
		if (time_values == null || time_values.length == 0 || time_values.length > 4)
		{
			return "error";
		}
		
		if (time_values.length == 1 && time_values[0] == Integer.MAX_VALUE)
		{
			return "3550+ weeks";
		}
		
		String[] time_strings =
		{
			"day", "hour", "minute", "second"
		};
		int num_times = 0;
		for (int i = 0; i < time_values.length; i++)
		{
			testAbortScript();
			if (time_values[i] != 0)
			{
				num_times++;
			}
		}
		
		int num_left = num_times;
		
		StringBuffer sb = new StringBuffer();
		
		for (int i = 0; i < time_values.length; i++)
		{
			testAbortScript();
			if (time_values[i] > 0)
			{
				sb.append(time_values[i] + " "+ time_strings[i]);
				if (time_values[i] > 1)
				{
					sb.append("s");
				}
				
				if (num_left == 1)
				{
					sb.append(".");
				}
				
				else if (num_left == 2)
				{
					if (num_times > 2)
					{
						sb.append(", and ");
					}
					else
					{
						sb.append(" and ");
					}
					
				}
				else
				{
					sb.append(", ");
				}
				
				num_left--;
			}
		}
		
		return sb.toString();
	}
	
	
	public static boolean convertPermissionsLists(obj_id structure) throws InterruptedException
	{
		if (!isIdValid(structure))
		{
			LOG("player_structure", "player_structure.convertPermissionsLists -- structure is invalid.");
			return false;
		}
		
		if (!hasObjVar(structure, VAR_PERMISSIONS_CONVERTED))
		{
			obj_id[] admin_list_as_objIds = getObjIdArrayObjVar(structure, VAR_ADMIN_LIST);
			if (admin_list_as_objIds != null)
			{
				
				String[] new_admin_list = convertObjIdPermissionList(admin_list_as_objIds);
				if (new_admin_list != null)
				{
					if (new_admin_list.length > 0)
					{
						setObjVar(structure, VAR_ADMIN_LIST, new_admin_list);
					}
					else
					{
						removeObjVar(structure, VAR_ADMIN_LIST);
					}
				}
			}
			else
			{
				String[] admin_list_as_strings = getStringArrayObjVar(structure, VAR_ADMIN_LIST);
				if (admin_list_as_strings != null)
				{
					
					String[] new_admin_list = convertStringPermissionList(admin_list_as_strings);
					if (new_admin_list != null)
					{
						if (new_admin_list.length > 0)
						{
							setObjVar(structure, VAR_ADMIN_LIST, new_admin_list);
						}
						else
						{
							removeObjVar(structure, VAR_ADMIN_LIST);
						}
					}
				}
			}
			
			obj_id[] hopper_list_as_objIds = getObjIdArrayObjVar(structure, VAR_HOPPER_LIST);
			if (hopper_list_as_objIds != null)
			{
				
				String[] new_hopper_list = convertObjIdPermissionList(hopper_list_as_objIds);
				if (new_hopper_list != null)
				{
					if (new_hopper_list.length > 0)
					{
						setObjVar(structure, VAR_HOPPER_LIST, new_hopper_list);
					}
					else
					{
						removeObjVar(structure, VAR_HOPPER_LIST);
					}
				}
			}
			else
			{
				String[] hopper_list_as_strings = getStringArrayObjVar(structure, VAR_HOPPER_LIST);
				if (hopper_list_as_strings != null)
				{
					
					String[] new_hopper_list = convertStringPermissionList(hopper_list_as_strings);
					if (new_hopper_list != null)
					{
						if (new_hopper_list.length > 0)
						{
							setObjVar(structure, VAR_HOPPER_LIST, new_hopper_list);
						}
						else
						{
							removeObjVar(structure, VAR_HOPPER_LIST);
						}
					}
				}
			}
			
			setObjVar(structure, VAR_PERMISSIONS_CONVERTED, 1);
		}
		else
		{
			
		}
		
		return true;
	}
	
	
	public static String[] convertObjIdPermissionList(obj_id[] objId_list) throws InterruptedException
	{
		if (objId_list == null || objId_list.length < 1)
		{
			LOG("player_structure", "player_structure.objIdListToNameList -- obj_id_list is null or empty.");
			return null;
		}
		
		Vector new_name_list = new Vector();
		new_name_list.setSize(0);
		for (int i = 0; i < objId_list.length; i++)
		{
			testAbortScript();
			if (!addPlayerObjIdToRawList( new_name_list, objId_list[i] ))
			{
				LOG("player_structure", "player_structure.convertObjIdPermissionList -- invalid object ID "+ objId_list[i].toString());
			}
		}
		
		if (new_name_list.size() > 0)
		{
			return (String[])new_name_list.toArray(new String[0]);
		}
		else
		{
			return null;
		}
	}
	
	
	public static String[] convertStringPermissionList(String[] name_list) throws InterruptedException
	{
		if (name_list == null || name_list.length < 1)
		{
			LOG("player_structure", "player_structure.objIdListToNameList -- obj_id_list is null or empty.");
			return null;
		}
		
		Vector new_name_list = new Vector();
		new_name_list.setSize(0);
		for (int i = 0; i < name_list.length; i++)
		{
			testAbortScript();
			if (!addNameToRawList( new_name_list, name_list[i] ))
			{
				LOG("player_structure", "player_structure.convertStringPermissionList -- couldn't convert the name "+ name_list[i]);
			}
		}
		
		if (new_name_list.size() > 0)
		{
			return (String[])new_name_list.toArray(new String[0]);
		}
		else
		{
			return null;
		}
	}
	
	
	public static float[] transformDeltaWorldCoord(float X, float Z, int rotation) throws InterruptedException
	{
		
		float x = X;
		float z = Z;
		
		switch(rotation)
		{
			case 0:
			break;
			case 1:
			X = z;
			Z = -x;
			break;
			case 2:
			Z = -z;
			X = -x;
			break;
			case 3:
			X = -z;
			Z = x;
			break;
		}
		
		float[] delta_world =
		{
			X, Z
		};
		
		return delta_world;
	}
	
	
	public static location transformDeltaWorldCoord(location here, float dx, float dz, float yaw) throws InterruptedException
	{
		yaw = yaw % 360f;
		if (yaw < 0)
		{
			yaw += 360f;
		}
		
		int rotation = java.lang.Math.round(yaw/90f);
		float[] xz = transformDeltaWorldCoord(dx, dz, rotation);
		if (xz != null & xz.length == 2)
		{
			location there = (location)here.clone();
			
			there.x += xz[0];
			there.z += xz[1];
			
			return there;
		}
		
		return null;
	}
	
	
	public static boolean hasSign(obj_id structure) throws InterruptedException
	{
		if (!isIdValid(structure))
		{
			return false;
		}
		
		return hasObjVar(structure, VAR_SIGN_ID);
	}
	
	
	public static boolean isPackedUp(obj_id structure) throws InterruptedException
	{
		obj_id container = getContainedBy(structure);
		if (!isIdValid(container))
		{
			return false;
		}
		
		String template = getTemplateName(container);
		return template.equals("object/intangible/house/generic_house_control_device.iff");
	}
	
	
	public static obj_id createStructureSign(obj_id structure, float sYaw) throws InterruptedException
	{
		
		if (!isIdValid(structure))
		{
			return null;
		}
		
		if (isPackedUp(structure))
		{
			
			return null;
		}
		
		String structureTemplate = getTemplateName(structure);
		
		int needSign = dataTableGetInt(PLAYER_STRUCTURE_DATATABLE, structureTemplate, DATATABLE_COL_HAS_SIGN);
		
		if (needSign == 1)
		{
			destroyStructureSign(structure);
			
			location here = getLocation(structure);
			
			for (int x = 0; x < SIGN_TYPE.length; x++)
			{
				testAbortScript();
				obj_id[] nc = getAllObjectsWithTemplate(here, 64f, SIGN_TYPE[x]);
				if (nc != null && nc.length > 0)
				{
					for (int i = 0; i < nc.length; i++)
					{
						testAbortScript();
						if (isObjectPersisted(nc[i]) && !utils.hasScriptVar(nc[i],"player_structure.parent"))
						{
							CustomerServiceLog("sign_destruction", "destroying sign "+ nc[i] + " whose name is '"+ getName(nc[i]) + "'");
							destroyObject(nc[i]);
						}
					}
				}
			}
			
			dictionary params = dataTableGetRow(TBL_SIGN, structureTemplate);
			if ((params != null) && (!params.isEmpty()))
			{
				int signType = 0;
				float x = params.getFloat("X");
				float y = params.getFloat("Y");
				float z = params.getFloat("Z");
				float heading = params.getFloat("HEADING");
				String signTemplate = params.getString("DEFAULT_SIGN");
				
				String signName = "Unnamed Structure";
				
				if (isCivic( structure ))
				{
					signName = utils.getStringName( structure );
				}
				else
				{
					String[] admins = getAdminListNames(structure);
					if ((admins != null) && (admins.length != 0))
					{
						String admin_name = admins[0];
						if (admin_name != null)
						{
							if (structureTemplate.indexOf("guild") > -1)
							{
								signName = admin_name + "'s Guildhall";
							}
							else if (structureTemplate.indexOf("barn") > -1)
							{
								signName = admin_name + "'s Barn";
							}
							else if (structureTemplate.indexOf("diner") > -1)
							{
								signName = admin_name + "'s Diner";
							}
							else if (structureTemplate.indexOf("cantina") > -1)
							{
								signName = admin_name + "'s Cantina";
							}
							else if (structureTemplate.indexOf("hospital") > -1)
							{
								signName = admin_name + "'s Medical Center";
							}
							else if (structureTemplate.indexOf("pgc_merchant_tent") > -1)
							{
								signName = admin_name + "'s Chronicles Tent";
							}
							else if (structureTemplate.indexOf("merchant_tent") > -1)
							{
								signName = admin_name + "'s Merchant Tent";
							}
							else if (structureTemplate.indexOf("theater") > -1)
							{
								signName = admin_name + "'s Theater";
							}
							else if (structureTemplate.indexOf("garage") > -1)
							{
								signName = admin_name + "'s Garage";
							}
							else
							{
								signName = admin_name + "'s House";
							}
						}
						else
						{
							if (structureTemplate.indexOf("guild") > -1)
							{
								signName = "Guildhall";
							}
							else if (structureTemplate.indexOf("barn") > -1)
							{
								signName = admin_name + "Barn";
							}
							else if (structureTemplate.indexOf("diner") > -1)
							{
								signName = admin_name + "Diner";
							}
							else if (structureTemplate.indexOf("cantina") > -1)
							{
								signName = admin_name + "Cantina";
							}
							else if (structureTemplate.indexOf("hospital") > -1)
							{
								signName = admin_name + "Medical Center";
							}
							else if (structureTemplate.indexOf("pgc_merchant_tent") > -1)
							{
								signName = admin_name + "'s Chronicles Tent";
							}
							else if (structureTemplate.indexOf("merchant_tent") > -1)
							{
								signName = admin_name + "Merchant Tent";
							}
							else if (structureTemplate.indexOf("theater") > -1)
							{
								signName = "Theater";
							}
							else if (structureTemplate.indexOf("garage") > -1)
							{
								signName = "Garage";
							}
							
							else
							{
								signName = "House";
							}
						}
					}
				}
				
				if (hasObjVar(structure, VAR_SIGN_BASE))
				{
					if (hasObjVar(structure, VAR_SIGN_NAME))
					{
						String storedName = getStringObjVar(structure, VAR_SIGN_NAME);
						if (storedName != null && !storedName.equals(""))
						{
							signName = storedName;
						}
					}
					
					if (hasObjVar(structure, VAR_SIGN_TYPE))
					{
						signType = getIntObjVar(structure, VAR_SIGN_TYPE);
						
						if (structureTemplate.equals("object/building/player/player_house_mustafar_lg.iff") && hasObjVar(structure, VAR_ABANDONED))
						{
							signType = 0;
						}
						
						if (signType > 0)
						{
							signTemplate = SIGN_TYPE[signType];
							x = params.getFloat("ALT_X");
							y = params.getFloat("ALT_Y");
							z = params.getFloat("ALT_Z");
							heading = params.getFloat("ALT_HEADING");
						}
					}
				}
				
				location there = new location(here.x + x, here.y + y, here.z + z, here.area, here.cell);
				
				if (hasObjVar(structure, MODIFIED_HOUSE_SIGN))
				{
					signTemplate = getStringObjVar(structure, MODIFIED_HOUSE_SIGN);
					
					if ((structureTemplate.equals("object/building/player/player_house_mustafar_lg.iff") && signTemplate.startsWith("object/tangible/sign/player/house_address") && !signTemplate.startsWith("object/tangible/sign/player/house_address_corellia")))
					{
						
						there.z -= 0.4f;
						
						z -= 0.4f;
					}
					
					if (structureTemplate.equals("object/building/player/player_house_generic_large_style_01.iff"))
					{
						if ((signTemplate.startsWith("object/tangible/sign/player/shop_sign_") && !signTemplate.startsWith("object/tangible/sign/player/shop_sign_s0")))
						{
							
							there.z += 0.8f;
							
							z += 0.8f;
						}
					}
					
					if ((structureTemplate.startsWith("object/building/player/city/cantina_naboo") && signTemplate.startsWith("object/tangible/sign/player/house_address") && !signTemplate.startsWith("object/tangible/sign/municipal/municipal_sign_hanging_cantina")))
					{
						
						there.z += 0.3f;
						
						z += 0.3f;
					}
					
					if ((structureTemplate.startsWith("object/building/player/city/cantina_corellia") && signTemplate.startsWith("object/tangible/sign/player/house_address") && !signTemplate.startsWith("object/tangible/sign/municipal/municipal_sign_hanging_cantina")))
					{
						
						there.z += 0.1f;
						
						z += 0.1f;
					}
				}
				else if (hasObjVar(structure, SPECIAL_SIGN))
				{
					signTemplate = getStringObjVar(structure, player_structure.SPECIAL_SIGN_TEMPLATE);
					
					if (signTemplate.startsWith("object/tangible/tcg/series5/house_sign_tcg_hanging"))
					{
						there.y += 1.5f;
						y += 1.5f;
					}
					
					if (signTemplate.startsWith("object/tangible/sign/player/pgc_sign_hanging.iff"))
					{
						
						there.x -= 0.45f;
						
						x -= 0.45f;
						
					}
					
					if (structureTemplate.startsWith("object/building/player/player_merchant_tent"))
					{
						
						if ((!signTemplate.startsWith("object/tangible/tcg/series3/house_sign_tcg_s02") && signTemplate.startsWith("object/tangible/tcg/")))
						{
							
							there.x -= 0.4f;
							there.z -= 0.4f;
							
							x -= 0.4f;
							z -= 0.4f;
						}
						
						if (signTemplate.startsWith("object/tangible/tcg/series3/house_sign_tcg_s02"))
						{
							
							there.x += 0.6f;
							there.z += 0.1f;
							
							x += 0.6f;
							z += 0.1f;
						}
						
						if (signTemplate.startsWith("object/tangible/sign/player/house_address_halloween"))
						{
							
							there.x -= 0.2f;
							there.z -= 0.2f;
							
							x -= 0.2f;
							z -= 0.2f;
						}
						
						if ((signTemplate.startsWith("object/tangible/sign/player/imperial_empire_day_2009_sign_hanging") || signTemplate.startsWith("object/tangible/sign/player/rebel_remembrance_day_2009_sign_hanging")))
						{
							
							there.x -= 0.37f;
							there.z -= 0.37f;
							
							x -= 0.37f;
							z -= 0.37f;
						}
						
						if ((signTemplate.startsWith("object/tangible/sign/player/imperial_empire_day_2009_sign_standing") || signTemplate.startsWith("object/tangible/sign/player/rebel_remembrance_day_2009_sign_standing")))
						{
							
							there.x += 0.6f;
							there.z += 0.1f;
							
							x += 0.6f;
							z += 0.1f;
						}
					}
					
					if (structureTemplate.startsWith("object/building/player/player_house_mustafar_lg"))
					{
						
						if (signTemplate.startsWith("object/tangible/tcg/series3/house_sign_tcg_s01"))
						{
							
							there.z -= 0.4f;
							
							z -= 0.4f;
						}
						
						if ((signTemplate.startsWith("object/tangible/sign/player/imperial_empire_day_2009_sign_hanging") || signTemplate.startsWith("object/tangible/sign/player/rebel_remembrance_day_2009_sign_hanging")))
						{
							
							there.z -= 0.4f;
							
							z -= 0.4f;
						}
						
						if (signTemplate.startsWith("object/tangible/tcg/series5/house_sign_tcg_hanging"))
						{
							
							there.z -= 0.4f;
							
							z -= 0.4f;
						}
					}
					
					if (structureTemplate.startsWith("object/building/player/city/theater_corellia"))
					{
						
						if (signTemplate.startsWith("object/tangible/tcg/series5/house_sign_tcg_hanging"))
						{
							there.x -= 0.4f;
							x -= 0.4f;
						}
					}
					
					if (structureTemplate.startsWith("object/building/player/player_guildhall_corellia"))
					{
						
						if (signTemplate.startsWith("object/tangible/tcg/series5/house_sign_tcg_hanging"))
						{
							there.z -= 0.4f;
							there.y -= 0.1f;
							z -= 0.4f;
							y -= 0.1f;
						}
					}
					
					if ((structureTemplate.equals("object/building/player/player_house_generic_large_style_01.iff") || structureTemplate.equals("object/building/player/player_house_corellia_large_style_01.iff")))
					{
						
						if (signTemplate.startsWith("object/tangible/tcg/series5/house_sign_tcg_standing"))
						{
							there.z += 1.2f;
							z += 1.2f;
						}
						
						if ((signTemplate.startsWith("object/tangible/sign/player/rebel_remembrance_day_2009_sign_standing.iff") || signTemplate.startsWith("object/tangible/sign/player/imperial_empire_day_2009_sign_standing.iff")))
						{
							there.z += 0.6f;
							z += 0.6f;
						}
						
						if ((signTemplate.startsWith("object/tangible/sign/player/rebel_remembrance_day_2009_sign_hanging.iff") || signTemplate.startsWith("object/tangible/sign/player/imperial_empire_day_2009_sign_hanging.iff")))
						{
							there.z -= 0.3f;
							z -= 0.3f;
						}
						
					}
					
					if (structureTemplate.startsWith("object/building/player/player_house_generic_medium_windowed_s02.iff"))
					{
						
						if (signTemplate.startsWith("object/tangible/tcg/series5/house_sign_tcg_hanging.iff"))
						{
							there.z -= 0.25f;
							z -= 0.25f;
						}
						
						if ((signTemplate.startsWith("object/tangible/sign/player/rebel_remembrance_day_2009_sign_hanging.iff") || signTemplate.startsWith("object/tangible/sign/player/imperial_empire_day_2009_sign_hanging.iff")))
						{
							there.z -= 0.25f;
							z -= 0.25f;
						}
					}
					
					if (signTemplate.startsWith("object/tangible/sign/player/pgc_sign_hanging.iff"))
					{
						if (structureTemplate.startsWith("object/building/player/player_house_naboo_small_style_02.iff"))
						{
							there.x += 0.5f;
							x += 0.5f;
						}
						if (structureTemplate.startsWith("object/building/player/player_house_corellia_large_style_01.iff"))
						{
							heading += 90;
							there.z -= 0.5f;
							z -= 0.5f;
						}
						if ((structureTemplate.startsWith("object/building/player/player_house_corellia_large_style_02.iff") || structureTemplate.startsWith("object/building/player/player_house_corellia_small_style_02.iff") || structureTemplate.startsWith("object/building/player/player_house_corellia_small_style_02_floorplan_02.iff")))
						{
							heading += 90;
							there.z -= 0.1f;
							z -= 0.1f;
						}
						if (structureTemplate.startsWith("object/building/player/player_house_corellia_medium_style_01.iff"))
						{
							heading += 90;
						}
						if ((structureTemplate.startsWith("object/building/player/player_house_corellia_small_style_01.iff") || structureTemplate.startsWith("object/building/player/player_house_corellia_small_style_01_floorplan_02.iff")))
						{
							heading += 90;
							there.x += 0.5f;
							x += 0.5f;
						}
						if ((structureTemplate.startsWith("object/building/player/player_merchant_tent_style_01.iff") || structureTemplate.startsWith("object/building/player/player_merchant_tent_style_02.iff") || structureTemplate.startsWith("object/building/player/player_merchant_tent_style_03.iff")))
						{
							heading += 10;
							there.x += 0.3f;
							there.z -= 0.1f;
							x += 0.3f;
							z -= 0.1f;
						}
						if (structureTemplate.startsWith("object/building/player/player_house_tatooine_large_style_01.iff"))
						{
							there.z -= 0.1f;
							z -= 0.1f;
						}
						if (structureTemplate.startsWith("object/building/player/player_house_mustafar_lg.iff"))
						{
							heading += 180;
							there.x -= 0.3f;
							x -= 0.3f;
						}
						if ((structureTemplate.startsWith("object/building/player/player_house_generic_large_style_01.iff") || structureTemplate.startsWith("object/building/player/player_house_generic_large_style_02.iff") || structureTemplate.startsWith("object/building/player/player_house_generic_medium_style_02.iff") || structureTemplate.startsWith("object/building/player/player_house_generic_small_style_02.iff")))
						{
							there.z += 0.2f;
							z += 0.2f;
						}
						if (structureTemplate.startsWith("object/building/player/player_house_generic_medium_windowed_s02.iff"))
						{
							there.z -= 0.2f;
							z -= 0.2f;
						}
						if (structureTemplate.startsWith("object/building/player/player_house_generic_small_style_01.iff"))
						{
							there.z += 0.2f;
							there.x += 0.7f;
							z += 0.2f;
							x += 0.7f;
						}
						if (structureTemplate.startsWith("object/building/player/player_house_naboo_small_style_01.iff"))
						{
							heading -= 10;
							there.x += 0.4f;
							x += 0.4f;
						}
						if (structureTemplate.startsWith("object/building/player/player_house_generic_small_windowed.iff"))
						{
							there.x += 0.5f;
							x += 0.5f;
						}
						if (structureTemplate.startsWith("object/building/player/player_house_generic_small_window_style_03.iff"))
						{
							there.x += 1;
							there.z -= 0.2f;
							x += 1;
							z -= 0.2f;
						}
						if (structureTemplate.startsWith("object/building/player/player_house_naboo_small_window_style_01.iff"))
						{
							heading -= 12;
							there.x += 0.6f;
							x += 0.6f;
						}
					}
					
					if ((structureTemplate.startsWith("object/building/player/player_house_corellia_small_style_02_floorplan_02.iff") || structureTemplate.startsWith("object/building/player/player_house_corellia_small_style_02.iff") || structureTemplate.startsWith("object/building/player/player_house_tatooine_small_style_02.iff") || structureTemplate.startsWith("object/building/player/player_house_tatooine_large_style_01.iff")))
					{
						
						if ((signTemplate.startsWith("object/tangible/sign/player/rebel_remembrance_day_2009_sign_hanging.iff") || signTemplate.startsWith("object/tangible/sign/player/imperial_empire_day_2009_sign_hanging.iff")))
						{
							there.z -= 0.2f;
							z -= 0.2f;
						}
					}
					
					if (structureTemplate.startsWith("object/building/player/player_house_tcg_vehicle_garage.iff"))
					{
						if (( signTemplate.startsWith("object/tangible/tcg/series3/house_sign_tcg_s01.iff") || signTemplate.startsWith("object/tangible/sign/player/house_address_halloween_sign.iff") || signTemplate.startsWith("object/tangible/sign/player/imperial_empire_day_2009_sign_hanging.iff") || signTemplate.startsWith("object/tangible/sign/player/rebel_remembrance_day_2009_sign_hanging.iff") || signTemplate.startsWith("object/tangible/tcg/series5/house_sign_tcg_hanging.iff") || signTemplate.startsWith("object/tangible/sign/player/pgc_sign_hanging.iff")))
						{
							heading += 90;
							there.z -= 0.3f;
							z -= 0.3f;
						}
					}
					
					if (structureTemplate.startsWith("object/building/player/player_house_tcg_vip_bunker.iff"))
					{
						if (( signTemplate.startsWith("object/tangible/tcg/series3/house_sign_tcg_s01.iff") || signTemplate.startsWith("object/tangible/sign/player/house_address_halloween_sign.iff") || signTemplate.startsWith("object/tangible/sign/player/imperial_empire_day_2009_sign_hanging.iff") || signTemplate.startsWith("object/tangible/sign/player/rebel_remembrance_day_2009_sign_hanging.iff") || signTemplate.startsWith("object/tangible/tcg/series5/house_sign_tcg_hanging.iff") || signTemplate.startsWith("object/tangible/sign/player/pgc_sign_hanging.iff")))
						{
							heading += 90;
							there.z += 0.3f;
							z += 0.3f;
						}
					}
					
					if (structureTemplate.startsWith("object/building/player/player_house_tcg_commando_bunker.iff"))
					{
						if (( signTemplate.startsWith("object/tangible/tcg/series3/house_sign_tcg_s01.iff") || signTemplate.startsWith("object/tangible/sign/player/house_address_halloween_sign.iff") || signTemplate.startsWith("object/tangible/sign/player/imperial_empire_day_2009_sign_hanging.iff") || signTemplate.startsWith("object/tangible/sign/player/rebel_remembrance_day_2009_sign_hanging.iff") || signTemplate.startsWith("object/tangible/tcg/series5/house_sign_tcg_hanging.iff") || signTemplate.startsWith("object/tangible/sign/player/pgc_sign_hanging.iff")))
						{
							heading += 90;
							there.z -= 0.6f;
							z -= 0.6f;
						}
					}
				}
				
				location spawn = (location)there.clone();
				
				if (sYaw != 0f)
				{
					location newspawn = transformDeltaWorldCoord(here, x, z, sYaw);
					newspawn.y = there.y;
					spawn = (location)newspawn.clone();
					
				}
				
				if ((signTemplate != null) && (!signTemplate.equals("")))
				{
					
					obj_id sign = createObject(signTemplate, spawn);
					if (isIdValid(sign))
					{
						String preSignName = "";
						String postSignName = "";
						String viewSignName = signName;
						if (isAbandoned( structure ) || isPreAbandoned( structure ))
						{
							int indexOfAbandonedText = signName.indexOf(ABANDONED_TEXT);
							String filteredSignName = "";
							boolean foundAbandonedText = false;
							
							while (indexOfAbandonedText != -1)
							{
								testAbortScript();
								filteredSignName += signName.substring(0, indexOfAbandonedText);
								signName = signName.substring(indexOfAbandonedText + ABANDONED_TEXT.length());
								indexOfAbandonedText = signName.indexOf(ABANDONED_TEXT);
								foundAbandonedText = true;
							}
							
							if (foundAbandonedText)
							{
								signName = filteredSignName + signName;
								viewSignName = signName;
							}
							
							postSignName = ABANDONED_TEXT;
							
							if (viewSignName.length() > 100)
							{
								viewSignName = viewSignName.substring(0, 99);
							}
						}
						
						if (isCityAbandoned(structure) && cityIsInactivePackupActive())
						{
							int indexOfAbandonedText = signName.indexOf(CITY_ABANDONED_TEXT);
							String filteredSignName = "";
							boolean foundAbandonedText = false;
							
							while (indexOfAbandonedText != -1)
							{
								testAbortScript();
								filteredSignName += signName.substring(0, indexOfAbandonedText);
								signName = signName.substring(indexOfAbandonedText + CITY_ABANDONED_TEXT.length());
								indexOfAbandonedText = signName.indexOf(CITY_ABANDONED_TEXT);
								foundAbandonedText = true;
							}
							
							if (foundAbandonedText)
							{
								signName = filteredSignName + signName;
								viewSignName = signName;
							}
							
							postSignName = CITY_ABANDONED_TEXT;
							
							if (viewSignName.length() > 100)
							{
								viewSignName = viewSignName.substring(0, 99);
							}
						}
						
						setName(sign, preSignName + viewSignName + postSignName);
						
						setObjVar(structure, VAR_SIGN_ID, sign);
						setObjVar(structure, VAR_SIGN_NAME, signName);
						setObjVar(structure, VAR_SIGN_TYPE, signType);
						
						float tYaw = sYaw + heading;
						if (tYaw != 0f)
						{
							
							setYaw(sign, tYaw);
						}
						
						utils.setScriptVar(sign, "player_structure.parent", structure);
						return sign;
					}
				}
				else
				{
					
				}
			}
			else
			{
				
			}
		}
		else
		{
			
		}
		
		return null;
	}
	
	
	public static obj_id createStructureSign(obj_id structure) throws InterruptedException
	{
		if (!isIdValid(structure))
		{
			return null;
		}
		
		float sYaw = getYaw(structure);
		sYaw = (float)(Math.round(sYaw));
		
		return createStructureSign(structure, sYaw);
	}
	
	
	public static void destroyStructureSign(obj_id structure) throws InterruptedException
	{
		if (!isIdValid(structure))
		{
			return;
		}
		
		obj_id sign = getObjIdObjVar(structure, VAR_SIGN_ID);
		if (isIdValid(sign) && exists(sign))
		{
			destroyObject(sign);
		}
		
		removeObjVar(structure, VAR_SIGN_ID);
	}
	
	
	public static float powerInstallation(obj_id bldg, obj_id energon) throws InterruptedException
	{
		
		if (!isIdValid(bldg) || !isIdValid(energon))
		{
			
			return -1f;
		}
		
		int energonGOT = getGameObjectType(energon);
		if (isGameObjectTypeOf(energonGOT, GOT_resource_container))
		{
			
			return -1f;
		}
		
		int bldgGOT = getGameObjectType(bldg);
		if (!isGameObjectTypeOf(bldgGOT, GOT_installation))
		{
			
			return -1f;
		}
		
		obj_id rId = getResourceContainerResourceType(energon);
		if (!isIdValid(rId))
		{
			
			return -1f;
		}
		
		if (!isResourceDerivedFrom(rId, "energy") || !isResourceDerivedFrom(rId, "radioactive"))
		{
			
			return -1f;
		}
		
		int amt = resource.getPotentialEnergyValue(energon);
		if (amt > 0f)
		{
			
			return powerInstallation(bldg, amt);
		}
		
		return -1f;
	}
	
	
	public static float powerInstallation(obj_id bldg, int amt) throws InterruptedException
	{
		if (!isIdValid(bldg) || (amt <= 0f))
		{
			return -1f;
		}
		
		incrementPowerValue(bldg, amt);
		return getPowerValue(bldg);
	}
	
	
	public static void validateHarvestedResources(obj_id structure) throws InterruptedException
	{
		if (!isIdValid(structure))
		{
			return;
		}
		
		if (utils.hasScriptVar(structure, "validatedResources"))
		{
			return;
		}
		
		if (!isHarvester(structure) && !isGenerator(structure))
		{
			return;
		}
		
		String listPath = "resource";
		if (hasObjVar(structure, listPath))
		{
			obj_var_list ovl = getObjVarList(structure, listPath);
			if (ovl != null)
			{
				int numItems = ovl.getNumItems();
				for (int i = 0; i < numItems; i++)
				{
					testAbortScript();
					obj_var ov = ovl.getObjVar(i);
					if (ov != null)
					{
						String ovName = ov.getName();
						obj_id rId = utils.stringToObjId(ovName);
						if (!isValidResourceId(rId))
						{
							
							float amt = ov.getFloatData();
							
							String mType = getHarvesterMasterResource(structure);
							if (mType != null && !mType.equals(""))
							{
								obj_id newId = pickRandomNonDepeletedResource(mType);
								if (isIdValid(newId))
								{
									setObjVar(structure, "resource."+ newId, amt);
								}
								else
								{
									CustomerServiceLog("trade","bad resouce found in installation #"+ structure + " and the system was unable to find a suitable replacement!");
									CustomerServiceLog("trade","installation #"+ structure + " report: bad resource id = "+ ovName + " amt = "+ Math.round(amt));
								}
							}
							else
							{
								CustomerServiceLog("trade","bad resouce found in installation #"+ structure + " and the system was unable to determine a suitable resource type for replacement!");
								CustomerServiceLog("trade","installation #"+ structure + " report: bad resource id = "+ ovName + " amt = "+ Math.round(amt));
							}
							
							removeObjVar(structure, listPath + "."+ ovName);
						}
					}
				}
			}
		}
		
		utils.setScriptVar(structure, "validatedResources", true);
	}
	
	
	public static String getHarvesterMasterResource(obj_id structure) throws InterruptedException
	{
		if (!isIdValid(structure))
		{
			return null;
		}
		
		if (!isHarvester(structure) && !isGenerator(structure))
		{
			return null;
		}
		
		String template = getTemplateName(structure);
		if (template == null || template.equals(""))
		{
			return null;
		}
		
		if (isHarvester(structure))
		{
			if (template.indexOf("mining_ore") > -1)
			{
				return "mineral";
			}
			
			if (template.indexOf("mining_gas") > -1)
			{
				return "gas";
			}
			
			if (template.indexOf("mining_liquid") > -1)
			{
				if (template.indexOf("moisture") > -1)
				{
					return "water";
				}
				
				return "chemical";
			}
			
			if (template.indexOf("mining_organic") > -1)
			{
				if (template.indexOf("creature") > -1)
				{
					return "creature_resources";
				}
				
				return "flora_resources";
			}
		}
		else if (isGenerator(structure))
		{
			if (template.indexOf("fusion") > -1)
			{
				return "radioactive";
			}
			
			if (template.indexOf("solar") > -1)
			{
				return "energy_renewable_unlimited_solar";
			}
			
			if (template.indexOf("wind") > -1)
			{
				return "energy_renewable_unlimited_wind";
			}
		}
		
		return null;
	}
	
	
	public static boolean canReclaimDeed(obj_id structure) throws InterruptedException
	{
		if (!isIdValid(structure))
		{
			return false;
		}
		
		String template = getTemplateName(structure);
		return canReclaimDeed(template);
	}
	
	
	public static boolean canReclaimDeed(String structureTemplate) throws InterruptedException
	{
		if (structureTemplate == null || structureTemplate.equals(""))
		{
			return false;
		}
		
		return dataTableGetInt(PLAYER_STRUCTURE_DATATABLE, structureTemplate, "CAN_RECLAIM") == 1;
	}
	
	
	public static void showChangeSignSui(obj_id structure, obj_id player) throws InterruptedException
	{
		if (!isIdValid(structure) || !isIdValid(player))
		{
			return;
		}
		
		int skillmod = getSkillStatMod(player, "shop_sign");
		if (skillmod < 1)
		{
			return;
		}
		
		Vector entries = new Vector();
		entries.setSize(0);
		for (int i = 0; i <= skillmod; i++)
		{
			testAbortScript();
			if (i < SIGN_NAMES.length)
			{
				entries = utils.addElement(entries, utils.packStringId(SIGN_NAMES[i]));
			}
			else
			{
				break;
			}
		}
		
		if (entries != null && entries.size() > 0)
		{
			String prompt = "@player_structure:changesign_prompt";
			String title = "@player_structure:changesign_title";
			sui.listbox(structure, player, prompt, sui.OK_CANCEL, title, entries, "handleChangeSignSui");
		}
	}
	
	
	public static String getStructureName(obj_id structure) throws InterruptedException
	{
		if (!isIdValid(structure))
		{
			return null;
		}
		
		if (isBuilding(structure) && hasObjVar(structure, VAR_SIGN_NAME))
		{
			String signName = getStringObjVar(structure, VAR_SIGN_NAME);
			if (signName != null && !signName.equals(""))
			{
				return signName;
			}
		}
		
		return utils.getStringName(structure);
	}
	
	
	public static boolean canPlaceGarage(location loc, float range, String template) throws InterruptedException
	{
		
		if (1 == dataTableGetInt(player_structure.PLAYER_STRUCTURE_DATATABLE, template, "GARAGE"))
		{
			obj_id check = getFirstObjectWithTemplate(loc, range, template);
			if (check != null)
			{
				return false;
			}
		}
		
		return true;
		
	}
	
	
	public static boolean hasMaintenanceDroid(obj_id player) throws InterruptedException
	{
		obj_id[] datapad = getContents(utils.getPlayerDatapad(player));
		
		for (int i = 0; i < datapad.length; i++)
		{
			testAbortScript();
			if (hasObjVar(datapad[i], "module_data.struct_maint"))
			{
				return true;
			}
		}
		
		return false;
	}
	
	
	public static obj_id[] getMaintenanceDroids(obj_id player) throws InterruptedException
	{
		Vector droids = new Vector();
		droids.setSize(0);
		obj_id[] datapad = getContents(utils.getPlayerDatapad(player));
		
		for (int i = 0; i < datapad.length; i++)
		{
			testAbortScript();
			if (hasObjVar(datapad[i], "module_data.struct_maint"))
			{
				droids = utils.addElement(droids, datapad[i]);
			}
		}
		
		obj_id[] _droids = new obj_id[0];
		if (droids != null)
		{
			_droids = new obj_id[droids.size()];
			droids.toArray(_droids);
		}
		return _droids;
	}
	
	
	public static void flagConverted(String template, obj_id structure) throws InterruptedException
	{
		
		if (template.indexOf( "_guildhall_" ) != -1)
		{
			
			setObjVar(structure, "newBuilding", true );
			setObjVar(structure, "newBuildPartTwo", true );
		}
		else if (template.indexOf( "structure_factory" ) != -1)
		{
			
			setObjVar(structure, "newBuilding", true );
			setObjVar(structure, "newBuildPartTwo", true );
		}
		else if (template.indexOf( "player_house_" ) != -1)
		{
			if (template.indexOf( "_large_style_" ) != -1)
			{
				setObjVar(structure, "newBuilding", true );
				setObjVar(structure, "newBuildPartTwo", true );
			}
		}
	}
	
	
	public static int convertLotCosts(obj_id structure, String template, int struct_lots) throws InterruptedException
	{
		if (!hasObjVar(structure, "newBuilding" ) || !hasObjVar(structure, "newBuildPartTwo" ))
		{
			
			if (template.indexOf( "_guildhall_" ) != -1)
			{
				
				if (template.indexOf("_guildhall_tatooine_") == -1)
				{
					
					struct_lots = 9;
				}
				else
				{
					
					struct_lots = 7;
				}
			}
			else if (template.indexOf( "structure_factory" ) != -1)
			{
				
				struct_lots = 2;
			}
			else if (template.indexOf( "player_house_" ) != -1)
			{
				if (template.indexOf( "_large_style_" ) != -1)
				{
					
					struct_lots = 6;
				}
			}
		}
		return struct_lots;
	}
	
	
	public static void doOldToNewLotConversion(obj_id player, obj_id structure) throws InterruptedException
	{
		if (!isOwner(structure, player))
		{
			return;
		}
		
		if (!hasObjVar(structure, "newBuilding" ))
		{
			String template = getTemplateName(structure);
			
			if (template.indexOf( "_guildhall_" ) != -1)
			{
				
				if (template.indexOf("_guildhall_tatooine_") == -1)
				{
					
					adjustLotCount(getPlayerObject(player), -4);
					setObjVar(structure, "newBuilding", true );
					setObjVar(structure, "newBuildPartTwo", true );
				}
				else
				{
					
					adjustLotCount(getPlayerObject(player), -2);
					setObjVar(structure, "newBuilding", true );
					setObjVar(structure, "newBuildPartTwo", true );
				}
			}
			else if (template.indexOf( "structure_factory" ) != -1)
			{
				
				adjustLotCount(getPlayerObject(player), -1);
				setObjVar(structure, "newBuilding", true );
			}
			else if (template.indexOf( "player_house_" ) != -1)
			{
				if (template.indexOf( "_large_style_" ) != -1)
				{
					
					adjustLotCount(getPlayerObject(player), -1);
					setObjVar(structure, "newBuilding", true );
					setObjVar(structure, "newBuildPartTwo", true );
				}
			}
		}
		else if (!hasObjVar(structure, "newBuildPartTwo" ))
		{
			String template = getTemplateName(structure);
			
			if (template.indexOf( "_guildhall_" ) != -1)
			{
				
				adjustLotCount(getPlayerObject(player), -2);
				setObjVar(structure, "newBuildPartTwo", true );
			}
			else if (template.indexOf( "player_house_" ) != -1)
			{
				if (template.indexOf( "_large_style_" ) != -1)
				{
					
					adjustLotCount(getPlayerObject(player), -1);
					setObjVar(structure, "newBuildPartTwo", true );
				}
			}
		}
	}
	
	
	public static void adjustLotsForOfflinePlayer(obj_id owner, obj_id structure) throws InterruptedException
	{
		dictionary params = new dictionary();
		
		if (!hasObjVar(structure, "newBuilding" ))
		{
			String template = getTemplateName(structure);
			
			if (template.indexOf( "_guildhall_" ) != -1)
			{
				
				if (template.indexOf("_guildhall_tatooine_") == -1)
				{
					
					params.put( "lotAdjust", -4 );
					setObjVar(structure, "newBuilding", true );
					setObjVar(structure, "newBuildPartTwo", true );
					messageTo( owner, "handleAdjustLotCount", params, 0, true );
				}
				else
				{
					
					params.put( "lotAdjust", -2 );
					setObjVar(structure, "newBuilding", true );
					setObjVar(structure, "newBuildPartTwo", true );
					messageTo( owner, "handleAdjustLotCount", params, 0, true );
				}
			}
			else if (template.indexOf( "structure_factory" ) != -1)
			{
				
				params.put( "lotAdjust", -1 );
				setObjVar(structure, "newBuilding", true );
				setObjVar(structure, "newBuildPartTwo", true );
				messageTo( owner, "handleAdjustLotCount", params, 0, true );
			}
			else if (template.indexOf( "player_house_" ) != -1)
			{
				if (template.indexOf( "_large_style_" ) != -1)
				{
					
					params.put( "lotAdjust", -1 );
					setObjVar(structure, "newBuilding", true );
					setObjVar(structure, "newBuildPartTwo", true );
					messageTo( owner, "handleAdjustLotCount", params, 0, true );
				}
			}
		}
		else if (!hasObjVar(structure, "newBuildPartTwo" ))
		{
			String template = getTemplateName(structure);
			
			if (template.indexOf( "_guildhall_" ) != -1)
			{
				
				params.put( "lotAdjust", -2 );
				setObjVar(structure, "newBuildPartTwo", true );
				messageTo( owner, "handleAdjustLotCount", params, 0, true );
			}
			else if (template.indexOf( "player_house_" ) != -1)
			{
				if (template.indexOf( "_large_style_" ) != -1)
				{
					
					params.put( "lotAdjust", -1 );
					setObjVar(structure, "newBuildPartTwo", true );
					messageTo( owner, "handleAdjustLotCount", params, 0, true );
				}
			}
		}
	}
	
	
	public static void doCondemnedSui(obj_id objStructure, obj_id objItem) throws InterruptedException
	{
		
		obj_id objOwner = getStructureOwnerObjId(objStructure);
		
		LOG("test", "objOwner is "+objOwner+" and item is "+objItem);
		if (isPlayer(objItem))
		{
			if (objOwner == objItem || player_structure.isAdmin(objStructure, objItem))
			{
				int intRepairCost = getStructureRepairCost(objStructure);
				int intBankCredits = getBankBalance(objItem);
				
				if (intRepairCost <= intBankCredits)
				{
					
					String strTitle = "@player_structure:fix_condemned_title";
					string_id strPrompt = new string_id(STF_FILE, "structure_condemned_owner_has_credits");
					String strConfirm = "@player_structure:confirm_pay";
					prose_package pp = new prose_package();
					prose.setStringId(pp, strPrompt);
					prose.setDI(pp, intRepairCost);
					
					int pid = sui.msgbox(objStructure, objItem, pp, sui.OK_CANCEL, strTitle, "payCondemnedFees");
					
					if (pid > -1)
					{
						
						showSUIPage(pid);
					}
					return;
				}
				else
				{
					string_id strSpam = new string_id(STF_FILE, "structure_condemned_owner_no_credits");
					prose_package pp = new prose_package();
					prose.setStringId(pp, strSpam);
					prose.setDI(pp, intRepairCost);
					sendSystemMessageProse(objItem, pp);
					return;
				}
			}
			else
			{
				string_id strSpam = new string_id(STF_FILE, "structure_condemned_not_owner");
				sendSystemMessage(objItem, strSpam);
				return;
			}
			
		}
		
	}
	
	
	public static int getStructureRepairCost(obj_id structure) throws InterruptedException
	{
		
		int condition = getStructureCondition(structure);
		int max_condition = getMaxCondition(structure);
		int damage = max_condition - condition;
		int maint_pool = getBankBalance(structure);
		
		float per_point_cost = getBaseMaintenanceRate( structure ) / 3.0f;
		
		int repair_cost = (int)(damage * per_point_cost);
		return repair_cost;
	}
	
	
	public static boolean isStructureCondemned(obj_id objStructure) throws InterruptedException
	{
		return hasObjVar(objStructure, "player_structure.condemned");
		
	}
	
	
	public static void sendCondemnedMail(obj_id objStructure) throws InterruptedException
	{
		obj_id objOwner = getOwner(objStructure);
		String strName = getPlayerName(objOwner);
		string_id strSubject = new string_id(STF_FILE, "structure_condemned_subject");
		string_id strBody = new string_id(STF_FILE, "structure_condemned_body");
		int intRepairCost = getStructureRepairCost(objStructure);
		location loc = getLocation(objStructure);
		String area = loc.area;
		if (area != null)
		{
			area = area.substring(0,1).toUpperCase() + area.substring(1);
		}
		String loc_str = "("+ loc.x + ", "+ loc.z + " "+ loc.area + ")";
		
		prose_package pp = new prose_package();
		
		prose.setStringId(pp, strBody);
		
		prose.setTO(pp, loc_str);
		prose.setTT(pp, objStructure);
		prose.setDI(pp, intRepairCost);
		
		utils.sendMail(strSubject, pp, strName , "@player_structure:management");
		return;
	}
	
	public static int getMaintenanceHeartbeat() throws InterruptedException
	{
		final int MAINTENANCE_HEARTBEAT = 1800;
		String strConfigSetting = getConfigSetting("GameServer", "maintenanceHeartbeat");
		if ((strConfigSetting != null)&&(!strConfigSetting.equals("")))
		{
			int intHeartBeat = utils.stringToInt(strConfigSetting);
			if (intHeartBeat > 0)
			{
				return intHeartBeat;
			}
		}
		return MAINTENANCE_HEARTBEAT;
		
	}
	
	public static void sendOutOfMaintenanceMail(obj_id objStructure) throws InterruptedException
	{
		
		obj_id objOwner = getOwner(objStructure);
		String strName = getPlayerName(objOwner);
		string_id strSubject = new string_id(STF_FILE, "structure_maintenance_empty_subject");
		string_id strBody = new string_id(STF_FILE, "structure_maintenance_empty_body");
		
		location loc = getLocation(objStructure);
		String area = loc.area;
		if (area != null)
		{
			area = area.substring(0,1).toUpperCase() + area.substring(1);
		}
		String loc_str = "("+ loc.x + ", "+ loc.z + " "+ loc.area + ")";
		
		prose_package pp = new prose_package();
		prose.setStringId(pp, strBody);
		prose.setTO(pp, loc_str);
		prose.setTT(pp, objStructure);
		
		String strTest = getString(strSubject);
		
		strTest = getString(strBody);
		
		utils.sendMail(strSubject, pp, strName , "@player_structure:management");
		return;
	}
	
	
	public static boolean isFactionPerkBase(String objectTemplateName) throws InterruptedException
	{
		return objectTemplateName.indexOf ("object/building/faction_perk/hq/") != -1;
	}
	
	
	public static boolean isFactionPerkBaseDeed(obj_id objDeed) throws InterruptedException
	{
		return isFactionPerkBaseDeed(getTemplateName(objDeed));
		
	}
	
	public static boolean isFactionPerkBaseDeed(String deedObjectTemplateName) throws InterruptedException
	{
		return deedObjectTemplateName.indexOf ("object/tangible/deed/faction_perk/hq/") != -1;
	}
	
	
	public static boolean canPlaceFactionPerkDeed(obj_id objDeed, obj_id objPlayer) throws InterruptedException
	{
		final float CHECK_RANGE = 600f;
		final int BASES_ALLOWED = 3;
		if (player_structure.isFactionPerkBaseDeed(objDeed))
		{
			location locTest = getLocation(objPlayer);
			obj_id[] objBases = getAllObjectsWithScript(locTest, CHECK_RANGE, "faction_perk.hq.loader");
			if ((objBases != null)&&(objBases.length >= BASES_ALLOWED))
			{
				string_id strSpam = new string_id("gcw", "too_many_bases");
				sendSystemMessage(objPlayer, strSpam);
				return false;
			}
		}
		return true;
	}
	
	
	public static int validateHopperSize(obj_id structureObject) throws InterruptedException
	{
		int objvar_hopper = 0;
		
		if (!isIdValid(structureObject))
		{
			return 0;
		}
		
		String templateName = getTemplateName(structureObject);
		
		if (templateName == null || templateName.length() <= 0)
		{
			return 0;
		}
		
		dictionary dict = utils.dataTableGetRow(PLAYER_STRUCTURE_VALIDATION_DATATABLE, templateName);
		
		if (dict == null)
		{
			return -1;
		}
		
		int minHopper = dataTableGetInt(PLAYER_STRUCTURE_VALIDATION_DATATABLE, templateName, DATATABLE_COL_HOPPER_MIN);
		int maxHopper = dataTableGetInt(PLAYER_STRUCTURE_VALIDATION_DATATABLE, templateName, DATATABLE_COL_HOPPER_MAX);
		
		if (minHopper <= 0 || maxHopper <= 0)
		{
			return 0;
		}
		
		if (hasObjVar(structureObject, VAR_DEED_MAX_HOPPER))
		{
			objvar_hopper = getIntObjVar(structureObject, VAR_DEED_MAX_HOPPER);
		}
		else
		{
			objvar_hopper = (minHopper + maxHopper) / 2;
			
			return objvar_hopper;
		}
		
		if (objvar_hopper <= minHopper)
		{
			objvar_hopper = (minHopper + maxHopper) / 2;
		}
		
		if (objvar_hopper > HARVESTER_MAX_HOPPER_SIZE)
		{
			objvar_hopper = HARVESTER_MAX_HOPPER_SIZE;
		}
		
		return objvar_hopper;
	}
	
	
	public static boolean doesUnmarkedStructureQualifyForHousePackup(obj_id structure) throws InterruptedException
	{
		if (!isBuilding(structure) && !isInstallation(structure))
		{
			return false;
		}
		
		if (isInstallation(structure) && !isFactory(structure))
		{
			return false;
		}
		
		if (isCivic(structure))
		{
			return false;
		}
		
		String templateName = getTemplateName(structure);
		
		if (templateName == null || isFactionPerkBase(templateName))
		{
			return false;
		}
		
		return true;
	}
	
	
	public static boolean needsPreAbandonCheck(obj_id structure) throws InterruptedException
	{
		if (!isIdValid(structure))
		{
			return false;
		}
		
		if (!utils.checkConfigFlag("GameServer", "enableHousePackup"))
		{
			return false;
		}
		
		if (isAbandoned(structure) || isPreAbandoned(structure))
		{
			return true;
		}
		
		if (!doesUnmarkedStructureQualifyForHousePackup(structure))
		{
			return false;
		}
		
		return true;
	}
	
	
	public static boolean needsAbandonCheck(obj_id structure) throws InterruptedException
	{
		if (!isIdValid(structure))
		{
			return false;
		}
		
		if (!utils.checkConfigFlag("GameServer", "enableAbandonedHousePackup"))
		{
			return false;
		}
		
		if (isPreAbandoned(structure) || isAbandoned(structure))
		{
			return true;
		}
		
		if (!doesUnmarkedStructureQualifyForHousePackup(structure))
		{
			return false;
		}
		
		return true;
	}
	
	
	public static boolean needsCityAbandonCheck(obj_id structure) throws InterruptedException
	{
		if (!isIdValid(structure))
		{
			LOG("sissynoid", "Failed 'needsCityAbandonedCheck - Invalid STructure");
			return false;
		}
		if (!cityIsInactivePackupActive())
		{
			LOG("sissynoid", "Failed 'needsCityAbandonedCheck - cityIsInactivePackupActive Config Inactive");
			return false;
		}
		if (!doesUnmarkedStructureQualifyForHousePackup(structure))
		{
			LOG("sissynoid", "Failed 'needsCityAbandonedCheck - Structure is unpackable");
			return false;
		}
		
		location loc = getLocation(structure);
		int city_id = getCityAtLocation(loc, 0);
		if (!cityExists(city_id))
		{
			LOG("sissynoid", "Failed 'needsCityAbandonedCheck - No City Exists at this Location");
			return false;
		}
		return true;
	}
	
	
	public static String getStructureInfo(obj_id structure) throws InterruptedException
	{
		final String yes = "Y|";
		final String no = "N|";
		final String blank = "|";
		
		final String templateName = getTemplateName(structure);
		final obj_id ownerId = getOwner(structure);
		
		String result = "|"+ structure + "|"+ ownerId + "|"+ templateName + "|";
		
		result += needsPreAbandonCheck(structure) ? yes : no;
		
		result += needsAbandonCheck(structure) ? yes : no;
		
		result += doesUnmarkedStructureQualifyForHousePackup(structure) ? yes : no;
		
		result += isIdValid(structure) ? yes : no;
		
		result += isAccountQualifiedForHousePackup(ownerId) ? yes : no;
		
		result += isBuilding(structure) ? yes : no;
		
		result += isInstallation(structure) ? yes : no;
		
		result += isFactory(structure) ? yes : no;
		
		result += isFactionPerkBase(templateName) ? yes : no;
		
		result += isCivic(structure) ? yes : no;
		
		result += isInWorld(structure) ? yes : no;
		
		result += isInWorldCell(structure) ? yes : no;
		
		if (hasObjVar(structure, VAR_CIVIC))
		{
			result += getIntObjVar(structure, VAR_CIVIC) + "|";
		}
		else
		{
			result += blank;
		}
		
		if (hasObjVar(structure, VAR_ABANDONED))
		{
			result += getIntObjVar(structure, VAR_ABANDONED);
		}
		else
		{
			result += blank;
		}
		
		if (hasObjVar(structure, VAR_LAST_MAINTANENCE))
		{
			result += getIntObjVar(structure, VAR_LAST_MAINTANENCE) + "|";
		}
		else
		{
			result += blank;
		}
		
		return result;
	}
	
	
	public static boolean isEliteHarvester(obj_id structure) throws InterruptedException
	{
		if (isIdValid(structure))
		{
			String templateName = getTemplateName(structure);
			if (templateName.endsWith("_style_4.iff") || templateName.endsWith("_elite.iff"))
			{
				return true;
			}
		}
		return false;
	}
	
	
	public static boolean canShowPackOption(obj_id player, obj_id structure) throws InterruptedException
	{
		if (!isIdValid(structure) || !isIdValid(player))
		{
			return false;
		}
		
		if (!utils.checkConfigFlag("GameServer", "allowPlayersToPackAbandonedStructures"))
		{
			return false;
		}
		
		if (!player_structure.isAbandoned(structure))
		{
			return false;
		}
		
		if (utils.isFreeTrial(player))
		{
			return false;
		}
		
		if (!isCommoditiesServerAvailable())
		{
			return false;
		}
		
		return true;
	}
	
	
	public static boolean canPlayerPackAbandonedStructure(obj_id player, obj_id structure) throws InterruptedException
	{
		if (!canShowPackOption(player, structure))
		{
			return false;
		}
		
		if (isAtPendingLoadRequestLimit())
		{
			return false;
		}
		
		if (!player.isLoaded() || !player.isAuthoritative())
		{
			return false;
		}
		
		if (!structure.isLoaded() || !structure.isAuthoritative())
		{
			return false;
		}
		
		return true;
	}
	
	
	public static boolean canPackStructureWithVendors(obj_id player, obj_id structure) throws InterruptedException
	{
		final boolean hasVendors = hasVendors(structure);
		
		if (hasVendors && !vendor_lib.isVendorPackUpEnabled())
		{
			sendSystemMessage(player, new string_id("sui", "cant_pack_vendors"));
			return false;
		}
		
		if (hasVendors && !isCommoditiesServerAvailable())
		{
			sendSystemMessage(player, new string_id("sui", "cannot_pack_now"));
			return false;
		}
		
		return true;
	}
	
	
	public static void packAbandonedBuilding(obj_id player, obj_id structure, dictionary params) throws InterruptedException
	{
		
		if (hasObjVar(player, HOUSE_PACKUP_DAILY_TIMER_OBJVAR))
		{
			int resetTime = getIntObjVar(player, HOUSE_PACKUP_DAILY_TIMER_OBJVAR);
			if (getGameTime() >= resetTime)
			{
				removeObjVar(player, HOUSE_PACKUP_DAILY_TIMER_OBJVAR);
				removeObjVar(player, HOUSE_PACKUP_DAILY_TALLY_OBJVAR);
				
				params.put("structure", structure);
				messageTo(player, "msgFinalizePackup", params, 0, false);
				return;
			}
		}
		
		if (!hasObjVar(player, HOUSE_PACKUP_DAILY_TALLY_OBJVAR))
		{
			params.put("structure", structure);
			messageTo(player, "msgFinalizePackup", params, 0, false);
			return;
		}
		
		int dailyHousePacking = getIntObjVar (player, HOUSE_PACKUP_DAILY_TALLY_OBJVAR);
		if (dailyHousePacking < 0)
		{
			dailyHousePacking = 0;
		}
		if (dailyHousePacking > MAX_PACKUP_PER_DAY)
		{
			dailyHousePacking = MAX_PACKUP_PER_DAY;
		}
		
		if (dailyHousePacking < MAX_PACKUP_PER_DAY)
		{
			
			if (!hasObjVar (player, HOUSE_PACKUP_DAILY_TIMER_OBJVAR))
			{
				removeObjVar (player, HOUSE_PACKUP_DAILY_TALLY_OBJVAR);
			}
			
			params.put("structure", structure);
			messageTo(player, "msgFinalizePackup", params, 0, false);
			return;
		}
		
		else if (!canPackMoreToday(player))
		{
			
			if (!hasObjVar (player, HOUSE_PACKUP_DAILY_TIMER_OBJVAR))
			{
				int resetTime = getGameTime() + TIME_TO_NEXT_PACKUP;
				blog("packAbandonedBuilding - resetting time to next packup: "+resetTime);
				
				setObjVar(player, HOUSE_PACKUP_DAILY_TIMER_OBJVAR, resetTime);
			}
			
			int timeLeft = getIntObjVar (player, HOUSE_PACKUP_DAILY_TIMER_OBJVAR);
			timeLeft -= getGameTime();
			String displayTime = utils.assembleTimeRemainToUse(timeLeft, false);
			string_id stringId = new string_id("spam", "wait_twenty_four");
			prose_package p = new prose_package();
			prose.setStringId(p, stringId);
			prose.setDI(p, MAX_PACKUP_PER_DAY);
			prose.setTT(p, displayTime);
			commPlayers(player, "object/mobile/dressed_hiddendagger_pilot_m_01.iff", "sound/sys_comm_other.snd", 5f, player, p);
			messageTo(player, "handleFailedStructurePackup", null, 0, false);
			
			messageTo(player, "handlePlayerStructurePackupLockoutRemoval", null, 0, false);
			
		}
		
		else
		{
			blog("player_structure.packAbandonedBuilding I HAVER FALLEN THROUGH THE NORMAL CONDITIONS!!!!");
			
			removeObjVar (player, HOUSE_PACKUP_DAILY_TIMER_OBJVAR);
			removeObjVar (player, HOUSE_PACKUP_DAILY_TALLY_OBJVAR);
			messageTo (structure, "packAbandonedBuilding", params, 4, false);
		}
	}
	
	
	public static boolean canPackMoreToday(obj_id player) throws InterruptedException
	{
		blog("canPackMoreToday - Init.");
		if (!isIdValid(player))
		{
			return false;
		}
		
		if (!hasObjVar(player, HOUSE_PACKUP_DAILY_TALLY_OBJVAR))
		{
			blog("canPackMoreToday - Player has no tally at all and can pack more.");
			return true;
		}
		blog("canPackMoreToday - Player has pack up DAILY TALLY objvar");
		
		if (!hasObjVar (player, HOUSE_PACKUP_DAILY_TIMER_OBJVAR))
		{
			blog("canPackMoreToday - The Player CAN PACK MORE");
			return true;
		}
		
		blog("canPackMoreToday - Player has pack up DAILY TIMER objvar");
		
		int dailyHousePacking = getIntObjVar (player, HOUSE_PACKUP_DAILY_TALLY_OBJVAR);
		if (dailyHousePacking < 0)
		{
			CustomerServiceLog("playerStructure","House Pack Up - Player "+ player + "("+ getName(player) + ") had an INVALID pack up tally amount of less than zero. We are correcting by setting to ZERO");
			dailyHousePacking = 0;
		}
		blog("canPackMoreToday - Player has dailyHousePacking: "+dailyHousePacking);
		
		if (dailyHousePacking > MAX_PACKUP_PER_DAY)
		{
			dailyHousePacking = MAX_PACKUP_PER_DAY;
		}
		
		blog("canPackMoreToday - Player has dailyHousePacking: "+MAX_PACKUP_PER_DAY);
		
		if (dailyHousePacking < MAX_PACKUP_PER_DAY)
		{
			blog("canPackMoreToday - The Player CAN PACK MORE");
			return true;
		}
		
		blog("canPackMoreToday - the player has met or exceeded the MAX_PACKUP_PER_DAY but we need to check to make sure that wasn't yesterday's timer.");
		int resetTime = getIntObjVar (player, HOUSE_PACKUP_DAILY_TIMER_OBJVAR);
		blog("canPackMoreToday - timer resetTime: "+resetTime);
		blog("canPackMoreToday - timer resetTime - gameTime: "+(resetTime - getGameTime()));
		if (resetTime - getGameTime() <= 0)
		{
			blog("canPackMoreToday - timer has expired.");
			return true;
			
		}
		blog("canPackMoreToday - timer has not expired.");
		return false;
	}
	
	
	public static boolean isPlayerGatedFromHousePackUp(obj_id player) throws InterruptedException
	{
		blog("player_structure.isPlayerGatedFromHousePackUp init");
		if (!isValidId(player) || !exists(player))
		{
			return true;
		}
		
		blog("player_structure.isPlayerGatedFromHousePackUp is calling canPackMoreToday(player)");
		
		boolean canPackMore = canPackMoreToday(player);
		boolean playerLockedOut = isPlayerLockedOutOfHousePackup(player);
		
		if (canPackMore && !playerLockedOut)
		{
			blog("player_structure.isPlayerGatedFromHousePackUp - canPackMoreToday(player) && !utils.hasScriptVar(player, SCRIPTVAR_HOUSE_PACKUP_LOCKOUT_TIME)");
			
			return false;
		}
		else if (playerLockedOut && canPackMore)
		{
			
			blog("player_structure.isPlayerLockedOutOfHousePackup PLAYER HAS BEEN LOCKED OUT");
			
			int timeNeeded = utils.getIntScriptVar(player, SCRIPTVAR_HOUSE_PACKUP_LOCKOUT_TIME) + (int)HOUSE_PACKUP_LOCKOUT_TIMER;
			int timeNow = getGameTime();
			
			blog("player_structure.isPlayerLockedOutOfHousePackup timeNeeded: "+timeNeeded);
			blog("player_structure.isPlayerLockedOutOfHousePackup timeNow: "+timeNow);
			blog("player_structure.isPlayerLockedOutOfHousePackup timeNow - timeNeeded: "+(timeNow - timeNeeded));
			int timeDiff = timeNeeded - timeNow;
			if (timeDiff <= 0)
			{
				blog("player_structure.isPlayerLockedOutOfHousePackup THE LOCK OUT TIME EXPIRED ALREADY");
				
				messageTo(player, "handlePlayerStructurePackupLockoutRemoval", null, 0, false);
				return false;
			}
			
			String timeMessage = utils.formatTimeVerbose(timeDiff);
			prose_package pp = new prose_package();
			pp = prose.setStringId(pp, SID_LOCKOUT_MESSAGE);
			pp = prose.setTO(pp, timeMessage);
			sendSystemMessageProse(player, pp);
		}
		else if (!canPackMore)
		{
			blog("player_structure.isPlayerGatedFromHousePackUp - !canPackMoreToday(player)");
			
			if (!hasObjVar (player, HOUSE_PACKUP_DAILY_TIMER_OBJVAR))
			{
				int resetTime = getGameTime() + TIME_TO_NEXT_PACKUP;
				blog("isPlayerGatedFromHousePackUp - resetting time to next packup: "+resetTime);
				setObjVar(player, HOUSE_PACKUP_DAILY_TIMER_OBJVAR, resetTime);
			}
			
			int timeLeft = getIntObjVar (player, HOUSE_PACKUP_DAILY_TIMER_OBJVAR);
			timeLeft -= getGameTime();
			String displayTime = utils.assembleTimeRemainToUse(timeLeft, false);
			string_id stringId = new string_id("spam", "wait_twenty_four");
			prose_package p = new prose_package();
			prose.setStringId(p, stringId);
			prose.setDI(p, MAX_PACKUP_PER_DAY);
			prose.setTT(p, displayTime);
			commPlayers(player, "object/mobile/dressed_hiddendagger_pilot_m_01.iff", "sound/sys_comm_other.snd", 5f, player, p);
			messageTo(player, "handleFailedStructurePackup", null, 0, false);
			
			messageTo(player, "handlePlayerStructurePackupLockoutRemoval", null, 0, false);
		}
		return true;
	}
	
	
	public static boolean isPlayerLockedOutOfHousePackup(obj_id player) throws InterruptedException
	{
		blog("player_structure.isPlayerLockedOutOfHousePackup init");
		
		if (!isValidId(player) || !exists(player))
		{
			return true;
		}
		
		if (utils.hasScriptVar(player, SCRIPTVAR_HOUSE_PACKUP_LOCKOUT_TIME))
		{
			return true;
		}
		
		blog("player_structure.isPlayerLockedOutOfHousePackup FALSE........FALSE on LOCKOUT");
		
		return false;
	}
	
	
	public static int getTotalIncreasedStorageAmountStructure(obj_id structure) throws InterruptedException
	{
		if (isIdValid(structure))
		{
			if (hasObjVar(structure, OBJVAR_STRUCTURE_STORAGE_INCREASE))
			{
				return getIntObjVar(structure, OBJVAR_STRUCTURE_STORAGE_INCREASE);
			}
		}
		
		return 0;
	}
	
	
	public static boolean validSafeStorageRemoval(obj_id structure, int decreaseAmount) throws InterruptedException
	{
		int numItems = player_structure.getStructureNumItems(structure);
		int baseItemLimit = getPobBaseItemLimit(structure);
		int storageIncreased = getTotalIncreasedStorageAmountStructure(structure);
		
		int projectedAmount = baseItemLimit + (storageIncreased - decreaseAmount);
		
		if ((numItems > projectedAmount) || (projectedAmount < baseItemLimit))
		{
			return false;
		}
		
		return true;
	}
	
	
	public static boolean incrementStorageAmount(obj_id player, obj_id structure, obj_id deed, int increaseAmount) throws InterruptedException
	{
		if (isIdValid(structure) && increaseAmount > 0)
		{
			
			int baseStorageIncrease = getTotalIncreasedStorageAmountStructure(structure);
			
			int newStrucStorage = baseStorageIncrease + increaseAmount;
			
			if (newStrucStorage > STORAGE_AMOUNT_CAP)
			{
				sendSystemMessage (player, MSG_STORAGE_OVER_CAP);
				return false;
			}
			
			if (newStrucStorage > baseStorageIncrease)
			{
				setObjVar(structure, OBJVAR_STRUCTURE_STORAGE_INCREASE, newStrucStorage);
				if (isNonGenericLootItem(deed))
				{
					setNonGenericStorageIncreaseOfTypeObjVar(structure, deed, increaseAmount);
				}
				return true;
			}
		}
		return false;
	}
	
	
	public static boolean decrementStorageAmount(obj_id player, obj_id structure, obj_id terminal, int selectedRedeedTypeIndex) throws InterruptedException
	{
		if (isIdValid(structure) && isIdValid(terminal))
		{
			String crate = "item_storage_increase_05_03";
			if (utils.hasScriptVar(terminal, "storageTypeNamesRef"))
			{
				String[] storageTypeNamesArrayRef = utils.getStringBatchScriptVar(terminal, "storageTypeNamesRef");
				crate = storageTypeNamesArrayRef[selectedRedeedTypeIndex];
			}
			int decreaseAmount = STORAGE_AMOUNT_DECREASE_UNIT;
			String nonGenericStorageTypeObjVar = NON_GENERIC_STORAGE_ITEM_OBJVAR+"."+crate;
			if (hasObjVar(structure, nonGenericStorageTypeObjVar))
			{
				decreaseAmount = getIntObjVar(structure, nonGenericStorageTypeObjVar);
			}
			
			int baseStorageIncrease = getTotalIncreasedStorageAmountStructure(structure);
			
			int newStrucStorage = baseStorageIncrease - decreaseAmount;
			
			if (newStrucStorage >= 0 && validSafeStorageRemoval(structure, decreaseAmount))
			{
				obj_id pInv = utils.getInventoryContainer(player);
				
				if (!isIdValid(pInv))
				{
					return false;
				}
				
				static_item.createNewItemFunction(crate, pInv);
				removeNonGenericStorageIncreaseOfTypeObjVar(structure, crate);
				
				if (newStrucStorage == 0)
				{
					
					removeObjVar(structure, OBJVAR_STRUCTURE_STORAGE_INCREASE);
					return true;
				}
				
				setObjVar(structure, OBJVAR_STRUCTURE_STORAGE_INCREASE, newStrucStorage);
				return true;
			}
			else
			{
				sendSystemMessage(player, new string_id("player_structure", "storage_item_cannot_remove"));
				player_structure.displayAvailableNonGenericStorageTypes(player, terminal, structure);
			}
		}
		return false;
	}
	
	
	public static boolean isNonGenericLootItem(obj_id deed) throws InterruptedException
	{
		return hasObjVar(deed, "nonGenericStorageItem");
	}
	
	
	public static boolean alreadyHasNonGenericStorageIncreaseOfType(obj_id structure, obj_id deed) throws InterruptedException
	{
		String deedType = getStaticItemName(deed);
		String nonGenericStorageTypeObjVar = NON_GENERIC_STORAGE_ITEM_OBJVAR+"."+deedType;
		return hasObjVar(structure, nonGenericStorageTypeObjVar);
	}
	
	
	public static void setNonGenericStorageIncreaseOfTypeObjVar(obj_id structure, obj_id deed, int increaseAmount) throws InterruptedException
	{
		String deedType = getStaticItemName(deed);
		String nonGenericStorageTypeObjVar = NON_GENERIC_STORAGE_ITEM_OBJVAR+"."+deedType;
		setObjVar(structure, nonGenericStorageTypeObjVar, increaseAmount);
		return;
	}
	
	
	public static void removeNonGenericStorageIncreaseOfTypeObjVar(obj_id structure, String type) throws InterruptedException
	{
		String nonGenericStorageTypeObjVar = NON_GENERIC_STORAGE_ITEM_OBJVAR+"."+type;
		if (hasObjVar(structure, nonGenericStorageTypeObjVar))
		{
			removeObjVar(structure, nonGenericStorageTypeObjVar);
		}
		return;
	}
	
	
	public static void displayAvailableNonGenericStorageTypes(obj_id player, obj_id terminal, obj_id structure) throws InterruptedException
	{
		if (utils.hasScriptVar(player, "structureStorageRedeedPid"))
		{
			int oldSui = utils.getIntScriptVar(player, "structureStorageRedeedPid");
			forceCloseSUIPage(oldSui);
		}
		
		String[] storageTypeNamesArrayRef = getStorageTypeNamesArray(structure);
		
		if (!hasObjVar(structure, NON_GENERIC_STORAGE_ITEM_OBJVAR))
		{
			if (validSafeStorageRemoval(structure, STORAGE_AMOUNT_DECREASE_UNIT))
			{
				handleStorageReclaimConfirmation(player, terminal);
			}
			else
			{
				sendSystemMessage(player, new string_id("sui", "storage_item_count_over_base"));
			}
		}
		else
		{
			String title = utils.packStringId(new string_id("player_structure", "sui_storage_nongeneric_redeed_title"));
			String prompt = utils.packStringId(new string_id("player_structure","sui_storage_nongeneric_redeed_prompt"));
			
			String[] storageTypeNamesArrayStrings = new String[storageTypeNamesArrayRef.length];
			for (int i = 0; i < storageTypeNamesArrayRef.length; i++)
			{
				testAbortScript();
				String nonGenericStorateType = storageTypeNamesArrayRef[i];
				
				int decreaseAmount = STORAGE_AMOUNT_DECREASE_UNIT;
				String nonGenericStorageTypeObjVar = NON_GENERIC_STORAGE_ITEM_OBJVAR+"."+nonGenericStorateType;
				if (hasObjVar(structure, nonGenericStorageTypeObjVar))
				{
					decreaseAmount = getIntObjVar(structure, nonGenericStorageTypeObjVar);
				}
				
				if (validSafeStorageRemoval(structure, decreaseAmount))
				{
					storageTypeNamesArrayStrings[i] = utils.packStringId(new string_id("static_item_n", nonGenericStorateType));
				}
				else
				{
					
					String greyedOut = utils.packStringId(new string_id("static_item_n", nonGenericStorateType));
					string_id contrastText_sid = new string_id("faction_recruiter","rank_list_empty");
					prose_package pp = prose.getPackage(contrastText_sid);
					prose.setTO(pp, greyedOut);
					storageTypeNamesArrayStrings[i] = "\0" + packOutOfBandProsePackage(null, pp);
				}
			}
			
			int newSui = sui.listbox(terminal, player, prompt, sui.OK_CANCEL, title, storageTypeNamesArrayStrings, "handleStorageRedeedChoice");
			utils.setScriptVar(player, "structureStorageRedeedPid", newSui);
		}
		
		utils.setBatchScriptVar(terminal, "storageTypeNamesRef", storageTypeNamesArrayRef);
		return;
	}
	
	
	public static String[] getStorageTypeNamesArray(obj_id structure) throws InterruptedException
	{
		Vector storageIncreaseTypes = new Vector();
		storageIncreaseTypes.setSize(0);
		int totalNonGenericStorageIncrease = 0;
		
		if (hasObjVar(structure, NON_GENERIC_STORAGE_ITEM_OBJVAR))
		{
			obj_var_list nonGenericStorageObjvarList = getObjVarList(structure, NON_GENERIC_STORAGE_ITEM_OBJVAR);
			if (nonGenericStorageObjvarList != null)
			{
				int numItem = nonGenericStorageObjvarList.getNumItems();
				for (int i = 0; i < numItem; i++)
				{
					testAbortScript();
					obj_var nonGenericStorageObjvar = nonGenericStorageObjvarList.getObjVar(i);
					String nonGenericStorageObjvarName = nonGenericStorageObjvar.getName();
					totalNonGenericStorageIncrease = totalNonGenericStorageIncrease + nonGenericStorageObjvar.getIntData();
					
					if (nonGenericStorageObjvarName != null && nonGenericStorageObjvarName.length() > 0)
					{
						storageIncreaseTypes = utils.addElement(storageIncreaseTypes, nonGenericStorageObjvarName);
					}
				}
			}
		}
		
		if (getTotalIncreasedStorageAmountStructure(structure) > totalNonGenericStorageIncrease)
		{
			storageIncreaseTypes = utils.addElement(storageIncreaseTypes, "item_storage_increase_05_03");
		}
		
		return (String[])storageIncreaseTypes.toArray(new String[0]);
	}
	
	
	public static boolean handleStorageReclaimConfirmation(obj_id player, obj_id terminal) throws InterruptedException
	{
		String title = utils.packStringId(new string_id("player_structure", "sui_storage_redeed_title"));
		String prompt = utils.packStringId(new string_id("player_structure", "sui_storage_redeed_prompt"));
		sui.msgbox(terminal, player, prompt, sui.YES_NO, title, "handleStorageRedeedChoice");
		
		return false;
	}
	
	
	public static boolean canRotateFurnitureInPitchRollAxes(obj_id player) throws InterruptedException
	{
		if (!isIdValid(player))
		{
			return false;
		}
		
		return hasCompletedCollection(player, "force_shui_tier_02");
	}
	
	
	public static boolean isPlayerStructure(obj_id target) throws InterruptedException
	{
		if (!isValidId(target) || !exists(target))
		{
			return false;
		}
		
		if (!isBuilding(target))
		{
			return false;
		}
		
		if (!hasObjVar(target, "player_structure"))
		{
			return false;
		}
		
		String template = getTemplateName(target);
		if (template == null || template.equals(""))
		{
			return false;
		}
		
		if (!template.startsWith("object/building/player/"))
		{
			return false;
		}
		
		if (!isGameObjectTypeOf(target, GOT_building_player))
		{
			return false;
		}
		
		return true;
	}
	
	
	public static boolean hasSpecialSignSkillMod(obj_id player, obj_id structure) throws InterruptedException
	{
		blog("player_structure.hasSpecialSignSkillMod: init");
		
		if (!isValidId(player) || !exists(player))
		{
			return false;
		}
		
		boolean owner = isOwner(structure, player);
		
		blog("player_structure.hasSpecialSignSkillMod: initial validation passes");
		
		String[] specialSignSkillMods = dataTableGetStringColumn(TBL_SPECIAL_SIGNS, "skillmod_name");
		int[] adminSpecialSign = dataTableGetIntColumn(TBL_SPECIAL_SIGNS, "admin_placement");
		
		if (specialSignSkillMods == null || specialSignSkillMods.length <= 0)
		{
			return false;
		}
		
		blog("player_structure.hasSpecialSignSkillMod: about to test each of the special signs to see if I have the necessary skill mods.");
		
		for (int i = 0; i < specialSignSkillMods.length; i++)
		{
			testAbortScript();
			if (getSkillStatMod(player, specialSignSkillMods[i]) <= 0)
			{
				continue;
			}
			
			if (!owner && adminSpecialSign[i] == 0)
			{
				continue;
			}
			
			blog("player_structure.hasSpecialSignSkillMod: Found a skill mod I have: "+specialSignSkillMods[i]);
			return true;
		}
		
		return false;
	}
	
	
	public static boolean getSpecialSignList(obj_id player, obj_id structure) throws InterruptedException
	{
		blog("player_structure.getSpecialSignList: init");
		
		if (!isValidId(player) || !exists(player))
		{
			return false;
		}
		
		boolean owner = isOwner(structure, player);
		
		blog("player_structure.getSpecialSignList: initial validation passes");
		
		String[] specialSignSkillMods = dataTableGetStringColumn(TBL_SPECIAL_SIGNS, "skillmod_name");
		String[] specialSignTemplates = dataTableGetStringColumn(TBL_SPECIAL_SIGNS, "sign_template");
		String[] specialSignNames = dataTableGetStringColumn(TBL_SPECIAL_SIGNS, "string_mapping");
		int[] adminSpecialSign = dataTableGetIntColumn(TBL_SPECIAL_SIGNS, "admin_placement");
		
		if (specialSignSkillMods == null || specialSignSkillMods.length <= 0)
		{
			return false;
		}
		else if (specialSignTemplates == null || specialSignTemplates.length <= 0)
		{
			return false;
		}
		else if (specialSignSkillMods.length != specialSignTemplates.length)
		{
			return false;
		}
		
		Vector allSignTemplates = new Vector();
		allSignTemplates.setSize(0);
		Vector allSignStrings = new Vector();
		allSignStrings.setSize(0);
		
		blog("player_structure.getSpecialSignList: about to test each of the special signs to see if I have the necessary skill mods.");
		
		for (int i = 0; i < specialSignSkillMods.length; i++)
		{
			testAbortScript();
			if (getSkillStatMod(player, specialSignSkillMods[i]) <= 0)
			{
				continue;
			}
			
			if (!owner && adminSpecialSign[i] <= 0)
			{
				continue;
			}
			
			blog("player_structure.getSpecialSignList: Found a skill mod I have: "+specialSignSkillMods[i]);
			utils.addElement(allSignTemplates, specialSignTemplates[i]);
			utils.addElement(allSignStrings, "@"+specialSignNames[i]);
		}
		
		if (allSignTemplates.size() <= 0)
		{
			return false;
		}
		if (allSignStrings.size() <= 0)
		{
			return false;
		}
		
		String[] signList = new String[allSignTemplates.size()];
		allSignTemplates.toArray(signList);
		
		String[] signName = new String[allSignStrings.size()];
		allSignStrings.toArray(signName);
		
		utils.setScriptVar(player, VAR_SPECIAL_SIGN_LIST, signList);
		utils.setScriptVar(player, VAR_SPECIAL_SIGN_NAMES, signName);
		return true;
	}
	
	
	public static boolean removeSpecialSign(obj_id player, obj_id structure, boolean revertMessage) throws InterruptedException
	{
		blog("player_structure.removeSpecialSign - init");
		
		if (!isValidId(player) || !isValidId(structure))
		{
			return false;
		}
		
		if (hasObjVar(structure, MODIFIED_HOUSE_SIGN_MODEL))
		{
			revertCustomSign(player, structure);
			return true;
		}
		
		boolean owner = isOwner(structure, player);
		if (hasObjVar(structure, SPECIAL_SIGN_OWNER_ONLY))
		{
			boolean needsOwnerToRemove = getBooleanObjVar(structure, SPECIAL_SIGN_OWNER_ONLY);
			if (!owner && needsOwnerToRemove)
			{
				sendSystemMessage(player, SID_ONLY_OWNER_CAN_REMOVE);
				return false;
			}
		}
		
		String skillmod = getStringObjVar(structure, SPECIAL_SIGN_SKILLMOD);
		if (skillmod == null || skillmod.length() <= 0)
		{
			return false;
		}
		blog("player_structure.removeSpecialSign - validation completed");
		
		String skillmodRecovery = getStringObjVar(structure, SPECIAL_SIGN_SKILLMOD);
		
		boolean ownerOnlyRecovery = false;
		if (hasObjVar(structure, SPECIAL_SIGN_OWNER_ONLY) && getBooleanObjVar(structure, SPECIAL_SIGN_OWNER_ONLY))
		{
			ownerOnlyRecovery = true;
		}
		
		String signTemplateRecovery = getStringObjVar(structure, SPECIAL_SIGN_TEMPLATE);
		int signTypeRecovery = 0;
		if (hasObjVar(structure, VAR_SIGN_TYPE))
		{
			signTypeRecovery = getIntObjVar(structure, VAR_SIGN_TYPE);
		}
		
		if (signTemplateRecovery == null || signTemplateRecovery.length() <= 0 || signTypeRecovery <= -1)
		{
			blog("player_structure.removeSpecialSign - signTemplateRecovery failed");
			sendSystemMessage(player, SID_SPECIAL_SIGN_FAILED);
			CustomerServiceLog("playerStructure","Special Sign could not be removed and replaced with default sign on structure "+ structure + ". The player: "+player+". Reason: signTemplateRecovery failed.");
			return false;
		}
		
		removeObjVar(structure, SPECIAL_SIGN);
		removeObjVar(structure, SPECIAL_SIGN_TEMPLATE);
		removeObjVar(structure, VAR_SIGN_TYPE);
		removeObjVar(structure, SPECIAL_SIGN_SKILLMOD);
		removeObjVar(structure, SPECIAL_SIGN_OWNER_ONLY);
		
		if (!isValidId(player_structure.createStructureSign(structure)))
		{
			setObjVar(structure, SPECIAL_SIGN, true);
			setObjVar(structure, SPECIAL_SIGN_TEMPLATE, signTemplateRecovery);
			setObjVar(structure, VAR_SIGN_TYPE, signTypeRecovery);
			setObjVar(structure, VAR_SIGN_TYPE, skillmodRecovery);
			setObjVar(structure, SPECIAL_SIGN_OWNER_ONLY, ownerOnlyRecovery);
			
			blog("player_structure.removeSpecialSign - Default Sign Creation failed!!!!!!!!!!!!!!!!!");
			sendSystemMessage(player, SID_SPECIAL_SIGN_FAILED);
			CustomerServiceLog("playerStructure","Special Sign could not be removed and replaced with default sign on structure "+ structure + ". The player: "+player+". Reason: createStructureSign failed.");
			return false;
		}
		
		if (hasObjVar(structure, SPECIAL_SIGN_DECREMENT_MOD))
		{
			if (!applySkillStatisticModifier(player, skillmod, 1))
			{
				blog("player_structure.removeSpecialSign - Applying skillmod failed!!!!!!!!!!!!!!!!!");
				CustomerServiceLog("playerStructure","Player: "+player+" attempted to remove special sign from structure: "+ structure + ". During the destroy process the skillmod: "+ skillmod + " failed to be reimbursed so the process was aborted.");
				sendSystemMessage(player, SID_SPECIAL_SIGN_FAILED);
				return false;
			}
			removeObjVar(structure, player_structure.SPECIAL_SIGN_DECREMENT_MOD);
		}
		blog("player_structure.removeSpecialSign - skillmod reimbursed and default sign created");
		if (revertMessage)
		{
			sendSystemMessage(player, SID_SPECIAL_SIGN_REVERT_SUCCESS);
		}
		
		CustomerServiceLog("playerStructure","Player: "+player+" removed special sign from structure: "+ structure + ". During the destroy process the skillmod: "+ skillmod + " was reimbursed.");
		return true;
	}
	
	
	public static boolean blog(String msg) throws InterruptedException
	{
		if (LOGGING_ON && msg != null && !msg.equals(""))
		{
			LOG(LOGGING_CATEGORY, msg);
		}
		return true;
	}
	
	
	public static void revertCustomSign(obj_id player, obj_id structure) throws InterruptedException
	{
		blog("revertCustomSign - revertCustomSign init");
		player_structure.destroyStructureSign(structure);
		removeObjVar(structure, player_structure.MODIFIED_HOUSE_SIGN);
		String customSign = getStringObjVar(structure, player_structure.MODIFIED_HOUSE_SIGN_MODEL);
		String newSign = "";
		
		blog("revertCustomSign - customSign: "+customSign);
		
		if (customSign.equals("item_event_halloween_house_sign"))
		{
			blog("revertCustomSign - customSign: item_event_halloween_house_sign "+customSign);
			
			newSign = "item_special_sign_halloween_hanging_sign";
		}
		else if (customSign.equals("item_event_halloween_house_sign_standing"))
		{
			blog("revertCustomSign - customSign: item_event_halloween_house_sign_standing "+customSign);
			
			newSign = "item_special_sign_halloween_standing_sign";
		}
		
		if (newSign == null || newSign.length() <= 0)
		{
			blog("revertCustomSign - customSign: FAIL!!!!!!!!!!!");
			
			obj_id sign = static_item.createNewItemFunction(customSign, player);
			if (isValidId(sign))
			{
				CustomerServiceLog("playerStructure","Player: "+player+" attempted to remove their old Galactic Moon Festival Sign: "+customSign+" from structure: "+structure+" but it failed to update to the new special sign system. The player was reimbursed with the old sign object: "+sign+".");
			}
			else
			{
				CustomerServiceLog("playerStructure","Player: "+player+" attempted to remove their old Galactic Moon Festival Sign: "+customSign+" from structure: "+structure+" but it failed to update to the new special sign system. The player was reimbursed with the old sign object: "+sign+".");
			}
		}
		else
		{
			blog("revertCustomSign - customSign: SUCCESS: "+newSign);
			
			obj_id sign = static_item.createNewItemFunction(newSign, player);
			if (isValidId(sign))
			{
				blog("revertCustomSign - customSign: Object ID: "+sign);
				
				CustomerServiceLog("playerStructure","Player: "+player+" remove their old Galactic Moon Festival Sign: "+customSign+" from structure: "+structure+" and received a new skillmod sign object: "+sign+".");
				sendSystemMessage(player, SID_HALLOWEEN_SIGN_UPDATED);
			}
			else
			{
				blog("revertCustomSign - customSign: Object COULD NOT BE CREATED");
				
				CustomerServiceLog("playerStructure","Player: "+player+" attempted to remove their old Galactic Moon Festival Sign: "+customSign+" from structure: "+structure+" but it failed to update to the new special sign system. The player was reimbursed with the old sign object: "+sign+".");
			}
		}
		removeObjVar(structure, player_structure.MODIFIED_HOUSE_SIGN_MODEL);
		player_structure.createStructureSign(structure);
	}
	
	
	public static void initializeFindAllItemsInHouse(obj_id self, obj_id player) throws InterruptedException
	{
		cleanupFindItemScriptVars(player);
		cleanupFindItemScriptVars(self);
		
		showFindItemsListboxPage(self, player, 0);
		return;
	}
	
	
	public static void initializeItemSearchInHouse(obj_id self, obj_id player) throws InterruptedException
	{
		String title = utils.packStringId(new string_id("player_structure", "find_items_search_keyword_title"));
		String prompt = getString(new string_id("player_structure","find_items_search_keyword_prompt"));
		int pid = sui.inputbox(self, player, prompt, sui.OK_CANCEL, title, sui.INPUT_NORMAL, null, "handlePlayerStructureSearchItemsGetKeyword");
		if (pid > -1)
		{
			sui.showSUIPage(pid);
			
			utils.setScriptVar(player, "findItems.pid", pid);
		}
		
		return;
	}
	
	
	public static void showFindItemsListboxPage(obj_id self, obj_id player, int startingIndex) throws InterruptedException
	{
		
		cleanupFindItemScriptVars(player);
		cleanupFindItemScriptVars(self);
		
		Vector myItemsInBuilding = getMyItemsInBuilding(player);
		
		if (myItemsInBuilding != null && myItemsInBuilding.size() > 0)
		{
			int totalNumItems = myItemsInBuilding.size();
			if (totalNumItems <= 49)
			{
				startingIndex = 0;
			}
			
			int endingIndex = startingIndex + 49;
			if (endingIndex >= totalNumItems)
			{
				endingIndex = totalNumItems - 1;
			}
			
			String[] itemsPageNamesList = new String[(endingIndex + 1) - startingIndex];
			obj_id[] itemsPageList = new obj_id[(endingIndex + 1) - startingIndex];
			
			int j = 0;
			for (int i = startingIndex; i <= endingIndex; i++)
			{
				testAbortScript();
				obj_id item = ((obj_id)(myItemsInBuilding.get(i)));
				String itemName = getEncodedName(item);
				
				itemsPageNamesList[j] = itemName;
				itemsPageList[j] = item;
				
				++j;
			}
			
			showFindItemsListboxSui(self, player, startingIndex, totalNumItems, itemsPageList, itemsPageNamesList);
		}
		else
		{
			sendSystemMessage(player, new string_id("player_structure", "find_items_no_items_found"));
			cleanupFindItemScriptVars(player);
			cleanupFindItemScriptVars(self);
		}
		
		return;
	}
	
	
	public static void repeatFindItemsListboxPage(obj_id self, obj_id player, int startingIndex, int totalNumItems, obj_id[] itemsPageList) throws InterruptedException
	{
		cleanupFindItemScriptVars(player);
		cleanupFindItemScriptVars(self);
		
		String[] itemsPageNamesList = new String[itemsPageList.length];
		for (int i = 0; i < itemsPageList.length; i++)
		{
			testAbortScript();
			obj_id item = itemsPageList[i];
			String itemName = getEncodedName(item);
			itemsPageNamesList[i] = itemName;
		}
		
		showFindItemsListboxSui(self, player, startingIndex, totalNumItems, itemsPageList, itemsPageNamesList);
		return;
	}
	
	
	public static void showFindItemsListboxSui(obj_id self, obj_id player, int startingIndex, int totalNumItems, obj_id[] itemsPageList, String[] itemsPageNamesList) throws InterruptedException
	{
		String title = utils.packStringId(new string_id("player_structure", "find_items_title"));
		String prompt = utils.packStringId(new string_id("player_structure", "find_items_prompt"));
		
		prompt += target_dummy.addLineBreaks(2);
		prompt += target_dummy.ORANGE + "Number of Items Found: "+ target_dummy.WHITE + totalNumItems;
		
		int endingIndex = startingIndex + 49;
		if (endingIndex >= totalNumItems)
		{
			endingIndex = totalNumItems - 1;
		}
		prompt += target_dummy.addLineBreaks(1);
		prompt += target_dummy.ORANGE + "Displaying Items: "+ target_dummy.WHITE + (startingIndex+1) + " through "+ (endingIndex+1);
		
		int pid = sui.listbox(self, player, prompt, sui.OK_CANCEL_REFRESH, title, itemsPageNamesList, "handlePlayerStructureFindItemsListResponse");
		if (pid > -1)
		{
			if (totalNumItems > 50)
			{
				sui.listboxUseOtherButton(pid, "Change Page");
			}
			else
			{
				sui.listboxUseOtherButton(pid, "Refresh Page");
			}
			
			sui.setSUIProperty(pid, sui.LISTBOX_BTN_OK, sui.PROP_TEXT,"Retrieve Object");
			sui.showSUIPage(pid);
			
			utils.setScriptVar(player, "findItems.pid", pid);
			utils.setScriptVar(player, "findItems.startingIndex", startingIndex);
			utils.setScriptVar(player, "findItems.totalNumItems", totalNumItems);
			utils.setScriptVar(self, "findItems.itemsPageList", itemsPageList);
		}
		
		return;
	}
	
	
	public static Vector getMyItemsInBuilding(obj_id player) throws InterruptedException
	{
		Vector myItemsInBuilding = new Vector();
		myItemsInBuilding.setSize(0);
		
		location here = getLocation(player);
		obj_id myCell = here.cell;
		if (isIdValid(myCell))
		{
			obj_id building = getContainedBy(myCell);
			if (isIdValid(building))
			{
				obj_id buildingOwner = getOwner(building);
				if ((isIdValid(buildingOwner) && player == buildingOwner) || isGod(player))
				{
					obj_id[] cellList = getContents(building);
					if (cellList != null && cellList.length > 0)
					{
						for (int i = 0; i < cellList.length; i++)
						{
							testAbortScript();
							obj_id buildingCell = cellList[i];
							if (getTemplateName(buildingCell).equals(structure.TEMPLATE_CELL))
							{
								addPlayerItemsFromContainerToList(buildingCell, player, myItemsInBuilding);
							}
						}
					}
				}
				else
				{
					sendSystemMessage(player, new string_id("player_structure", "find_items_not_building_owner"));
				}
			}
			else
			{
				sendSystemMessage(player, new string_id("player_structure", "find_items_not_in_a_cell"));
			}
		}
		else
		{
			sendSystemMessage(player, new string_id("player_structure", "find_items_not_in_a_building"));
		}
		
		if (myItemsInBuilding.size() > 0)
		{
			return myItemsInBuilding;
		}
		
		return null;
	}
	
	
	public static void addPlayerItemsFromContainerToList(obj_id container, obj_id player, Vector myItemsInBuilding) throws InterruptedException
	{
		obj_id[] contents = getContents(container);
		if (contents != null && contents.length > 0)
		{
			for (int j = 0; j < contents.length; j++)
			{
				testAbortScript();
				obj_id thing = contents[j];
				if (isIdValid(thing))
				{
					boolean isValidPlayerOwnedObject = true;
					
					obj_id thingOwner = getOwner(thing);
					if (player != thingOwner && !isGod(player))
					{
						isValidPlayerOwnedObject = false;
					}
					
					if (!isObjectPersisted(thing))
					{
						isValidPlayerOwnedObject = false;
					}
					
					if (isPlayer(thing))
					{
						isValidPlayerOwnedObject = false;
					}
					
					if (isMob(thing))
					{
						isValidPlayerOwnedObject = false;
					}
					
					obj_id ship = space_transition.getContainingShip(thing);
					if (isIdValid(ship))
					{
						obj_id lootBox = obj_id.NULL_ID;
						if (hasObjVar(ship, "objLootBox"))
						{
							lootBox = getObjIdObjVar(ship, "objLootBox");
						}
						
						if (isIdValid(lootBox) && lootBox == thing)
						{
							isValidPlayerOwnedObject = false;
						}
					}
					
					if (hasCondition(thing, CONDITION_VENDOR))
					{
						isValidPlayerOwnedObject = false;
					}
					
					if (isValidPlayerOwnedObject)
					{
						if (getContainerType(thing) != 0 && getGameObjectType(thing) != GOT_misc_factory_crate)
						{
							addPlayerItemsFromContainerToList(thing, player, myItemsInBuilding);
						}
						
						utils.addElement(myItemsInBuilding, thing);
					}
				}
			}
		}
	}
	
	
	public static boolean moveItemInBuildingToPlayer(obj_id player, obj_id selectedItem) throws InterruptedException
	{
		location here = getLocation(player);
		obj_id myCell = here.cell;
		if (isIdValid(myCell))
		{
			obj_id building = getContainedBy(myCell);
			if (isIdValid(building))
			{
				obj_id buildingOwner = getOwner(building);
				if ((isIdValid(buildingOwner) && player == buildingOwner) || isGod(player))
				{
					if (exists(selectedItem))
					{
						if (!utils.isNestedWithinAPlayer(selectedItem))
						{
							obj_id itemTopMost = getTopMostContainer(selectedItem);
							obj_id myTopMost = getTopMostContainer(player);
							if (itemTopMost == myTopMost)
							{
								obj_id topNonCellContainer = getTopNonCellContainer(selectedItem);
								
								String itemName = getEncodedName(selectedItem);
								String containerName = getEncodedName(topNonCellContainer);
								string_id message = new string_id("player_structure", "find_items_item_moved");
								if (topNonCellContainer != selectedItem)
								{
									message = new string_id("player_structure", "find_items_item_container_moved");
								}
								prose_package pp = prose.getPackage(message);
								prose.setTO(pp, itemName);
								prose.setTT(pp, containerName);
								sendQuestSystemMessage(player, pp);
								
								setLocation(topNonCellContainer, here);
								return true;
							}
							else
							{
								sendSystemMessage(player, new string_id("player_structure", "find_items_failed_not_in_same_building"));
							}
						}
						else
						{
							if (utils.isNestedWithin(selectedItem, player))
							{
								sendSystemMessage(player, new string_id("player_structure", "find_items_failed_nested_within_you"));
							}
							else
							{
								sendSystemMessage(player, new string_id("player_structure", "find_items_failed_nested_within_other_player"));
							}
						}
					}
					else
					{
						sendSystemMessage(player, new string_id("player_structure", "find_items_failed_no_longer_exists"));
					}
				}
				else
				{
					sendSystemMessage(player, new string_id("player_structure", "find_items_failed_not_owner"));
				}
			}
			else
			{
				sendSystemMessage(player, new string_id("player_structure", "find_items_failed_uncontained_cell"));
			}
		}
		else
		{
			sendSystemMessage(player, new string_id("player_structure", "find_items_failed_not_outside"));
		}
		
		return false;
	}
	
	
	public static obj_id getTopNonCellContainer(obj_id item) throws InterruptedException
	{
		obj_id container = getContainedBy(item);
		if (!isIdValid(container))
		{
			
			return item;
		}
		else if (getTemplateName(container).equals(structure.TEMPLATE_CELL))
		{
			
			return item;
		}
		else
		{
			
			return getTopNonCellContainer(container);
		}
	}
	
	
	public static void cleanupFindItemScriptVars(obj_id target) throws InterruptedException
	{
		utils.removeScriptVarTree(target, "findItems");
		return;
	}
	
	
	public static void handleFindItemsListResponse(obj_id self, dictionary params) throws InterruptedException
	{
		if (params == null || params.isEmpty())
		{
			return;
		}
		
		obj_id player = sui.getPlayerId(params);
		if (!isIdValid(player))
		{
			return;
		}
		
		int button = sui.getIntButtonPressed(params);
		if (button == sui.BP_CANCEL)
		{
			sendSystemMessage(player, new string_id("player_structure", "find_items_search_cancelled"));
			cleanupFindItemScriptVars(player);
			cleanupFindItemScriptVars(self);
			return;
		}
		
		int startingIndex = utils.getIntScriptVar(player, "findItems.startingIndex");
		int totalNumItems = utils.getIntScriptVar(player, "findItems.totalNumItems");
		
		obj_id[] itemsPageList = utils.getObjIdArrayScriptVar(self, "findItems.itemsPageList");
		if (itemsPageList == null || itemsPageList.length < 1)
		{
			sendSystemMessage(player, new string_id("player_structure", "find_items_no_items_found"));
			cleanupFindItemScriptVars(player);
			cleanupFindItemScriptVars(self);
			return;
		}
		
		if (button == sui.BP_REVERT)
		{
			if (totalNumItems > 50)
			{
				
				String title = utils.packStringId(new string_id("player_structure", "find_items_page_title"));
				String prompt = "";
				
				int pid = sui.createSUIPage(sui.SUI_MSGBOX, self, player, "handlePlayerStructureFindItemsPageResponse");
				if (startingIndex == 0)
				{
					if (pid > -1)
					{
						setSUIProperty(pid, sui.MSGBOX_TITLE, sui.PROP_TEXT, title);
						prompt = utils.packStringId(new string_id("player_structure", "find_items_page_next_prompt"));
						setSUIProperty(pid, sui.MSGBOX_PROMPT, sui.PROP_TEXT, prompt);
						
						sui.msgboxButtonSetup(pid, sui.YES_NO);
						setSUIProperty(pid, sui.MSGBOX_BTN_OK, sui.PROP_TEXT, "Next Page");
						setSUIProperty(pid, sui.MSGBOX_BTN_CANCEL, sui.PROP_TEXT, "Cancel");
						
						sui.showSUIPage(pid);
					}
				}
				else if (startingIndex + 50 >= totalNumItems)
				{
					if (pid > -1)
					{
						setSUIProperty(pid, sui.MSGBOX_TITLE, sui.PROP_TEXT, title);
						prompt = utils.packStringId(new string_id("player_structure", "find_items_page_previous_prompt"));
						setSUIProperty(pid, sui.MSGBOX_PROMPT, sui.PROP_TEXT, prompt);
						
						sui.msgboxButtonSetup(pid, sui.YES_NO);
						setSUIProperty(pid, sui.MSGBOX_BTN_OK, sui.PROP_TEXT, "Previous Page");
						setSUIProperty(pid, sui.MSGBOX_BTN_CANCEL, sui.PROP_TEXT, "Cancel");
						
						utils.setScriptVar(player, "findItems.lastPage", 1);
						
						sui.showSUIPage(pid);
					}
				}
				else
				{
					if (pid > -1)
					{
						setSUIProperty(pid, sui.MSGBOX_TITLE, sui.PROP_TEXT, title);
						prompt = utils.packStringId(new string_id("player_structure", "find_items_page_next_previous_prompt"));
						setSUIProperty(pid, sui.MSGBOX_PROMPT, sui.PROP_TEXT, prompt);
						
						sui.msgboxButtonSetup(pid, sui.YES_NO_CANCEL);
						setSUIProperty(pid, sui.MSGBOX_BTN_REVERT, sui.PROP_TEXT, "Previous Page");
						setSUIProperty(pid, sui.MSGBOX_BTN_OK, sui.PROP_TEXT, "Next Page");
						setSUIProperty(pid, sui.MSGBOX_BTN_CANCEL, sui.PROP_TEXT, "Cancel");
						
						setSUIProperty(pid, sui.MSGBOX_BTN_REVERT, "OnPress", "RevertWasPressed=1\r\nparent.btnOk.press=t");
						subscribeToSUIProperty(pid, sui.MSGBOX_BTN_REVERT, "RevertWasPressed");
						
						sui.showSUIPage(pid);
					}
				}
			}
			else
			{
				repeatFindItemsListboxPage(self, player, startingIndex, totalNumItems, itemsPageList);
			}
			
			return;
		}
		
		int item_selected = sui.getListboxSelectedRow(params);
		if (item_selected < 0)
		{
			sendSystemMessage(player, new string_id("player_structure", "find_items_no_selection"));
			cleanupFindItemScriptVars(player);
			cleanupFindItemScriptVars(self);
			return;
		}
		
		obj_id selectedItem = itemsPageList[item_selected];
		if (isIdValid(selectedItem))
		{
			moveItemInBuildingToPlayer(player, selectedItem);
			repeatFindItemsListboxPage(self, player, startingIndex, totalNumItems, itemsPageList);
			return;
		}
		
		cleanupFindItemScriptVars(player);
		cleanupFindItemScriptVars(self);
		return;
	}
	
	
	public static void handleFindItemsChangePageResponse(obj_id self, dictionary params) throws InterruptedException
	{
		if (params == null || params.isEmpty())
		{
			return;
		}
		
		obj_id player = sui.getPlayerId(params);
		if (!isIdValid(player))
		{
			return;
		}
		
		String revert = params.getString(sui.MSGBOX_BTN_REVERT+".RevertWasPressed");
		int button = sui.getIntButtonPressed( params );
		
		if (button == sui.BP_CANCEL)
		{
			sendSystemMessage(player, new string_id("player_structure", "find_items_search_cancelled"));
			cleanupFindItemScriptVars(player);
			cleanupFindItemScriptVars(self);
			return;
		}
		
		int startingIndex = utils.getIntScriptVar(player, "findItems.startingIndex");
		if (revert != null && !revert.equals(""))
		{
			
			startingIndex = startingIndex - 50;
		}
		else if (utils.hasScriptVar(player, "findItems.lastPage"))
		{
			
			startingIndex = startingIndex - 50;
		}
		else
		{
			
			startingIndex = startingIndex + 50;
		}
		
		if (startingIndex < 0)
		{
			startingIndex = 0;
		}
		
		cleanupFindItemScriptVars(player);
		cleanupFindItemScriptVars(self);
		
		showFindItemsListboxPage(self, player, startingIndex);
		
		return;
	}
	
	
	public static void handleSearchItemsGetKeyword(obj_id self, dictionary params) throws InterruptedException
	{
		if (params == null || params.isEmpty())
		{
			return;
		}
		
		obj_id player = sui.getPlayerId(params);
		if (!isIdValid(player))
		{
			return;
		}
		
		int btn = sui.getIntButtonPressed(params);
		if (btn == sui.BP_CANCEL)
		{
			sendSystemMessage(player, new string_id("player_structure", "find_items_search_cancelled"));
			return;
		}
		
		String keyword = toLower(sui.getInputBoxText(params));
		
		if (keyword == null || keyword.length() < 1)
		{
			sendSystemMessage(player, new string_id("player_structure", "find_items_search_no_keyword"));
			return;
		}
		
		Vector myItemsInBuilding = player_structure.getMyItemsInBuilding(player);
		Vector keywordMatchedIds = new Vector();
		keywordMatchedIds.setSize(0);
		Vector keywordMatchedNames = new Vector();
		keywordMatchedNames.setSize(0);
		
		int numMatches = 0;
		String prompt = utils.packStringId(new string_id("player_structure", "find_items_search_list_prompt"));
		
		for (int i = 0; i < myItemsInBuilding.size(); i++)
		{
			testAbortScript();
			obj_id thing = ((obj_id)(myItemsInBuilding.get(i)));
			String name = getAssignedName(thing);
			if (name == null || name.length () < 1)
			{
				string_id sidName = getNameStringId(thing);
				if (sidName != null && !sidName.isEmpty())
				{
					sidName = utils.unpackString("@"+ sidName.toString());
					name = getString(sidName);
					if (name.startsWith("@"))
					{
						CustomerServiceLog("findItemInStructure", "Server localization for this item failed: "+getTemplateName(thing)+" ("+thing+").");
					}
				}
			}
			
			String lowerCaseName = toLower(name);
			int stringCheck = lowerCaseName.indexOf(keyword);
			if (stringCheck > -1)
			{
				utils.addElement(keywordMatchedIds, thing);
				utils.addElement(keywordMatchedNames, name);
				++numMatches;
			}
			
			if (numMatches >= 50)
			{
				prompt = utils.packStringId(new string_id("player_structure", "find_items_search_list_too_long_prompt"));
				break;
			}
		}
		
		if (keywordMatchedIds == null || keywordMatchedIds.size() < 1)
		{
			sendSystemMessage(player, new string_id("player_structure", "find_items_search_not_found"));
			return;
		}
		
		if (keywordMatchedNames == null || keywordMatchedNames.size() < 1 || keywordMatchedIds.size() != keywordMatchedNames.size())
		{
			
			return;
		}
		
		String[] matchingItemsNameList = utils.toStaticStringArray(keywordMatchedNames);
		obj_id[] matchingItemsList = utils.toStaticObjIdArray(keywordMatchedIds);
		
		showSearchItemsListboxSui(self, player, matchingItemsList, matchingItemsNameList, prompt);
		
		return;
	}
	
	
	public static void repeatSearchItemsKeywordList(obj_id self, obj_id player, obj_id[] matchingItemsList) throws InterruptedException
	{
		cleanupFindItemScriptVars(player);
		cleanupFindItemScriptVars(self);
		
		String[] matchingItemsNamesList = new String[matchingItemsList.length];
		for (int i = 0; i < matchingItemsList.length; i++)
		{
			testAbortScript();
			obj_id item = matchingItemsList[i];
			String itemName = getEncodedName(item);
			matchingItemsNamesList[i] = itemName;
		}
		
		String prompt = utils.packStringId(new string_id("player_structure", "find_items_search_list_prompt"));
		if (matchingItemsList.length >= 50)
		{
			prompt = utils.packStringId(new string_id("player_structure", "find_items_search_list_too_long_prompt"));
		}
		
		showSearchItemsListboxSui(self, player, matchingItemsList, matchingItemsNamesList, prompt);
		return;
	}
	
	
	public static void showSearchItemsListboxSui(obj_id self, obj_id player, obj_id[] matchingItemsList, String[] matchingItemsNamesList, String prompt) throws InterruptedException
	{
		String title = utils.packStringId(new string_id("player_structure", "find_items_search_list_title"));
		int pid = sui.listbox(self, player, prompt, sui.OK_CANCEL, title, matchingItemsNamesList, "handlePlayerStructureSearchItemsSelectedResponse");
		if (pid > -1)
		{
			sui.setSUIProperty(pid, sui.LISTBOX_BTN_OK, sui.PROP_TEXT,"Retrieve Object");
			sui.showSUIPage(pid);
			
			utils.setScriptVar(player, "findItems.pid", pid);
			utils.setScriptVar(self, "findItems.searchList", matchingItemsList);
		}
		
		return;
	}
	
	
	public static void handleSearchItemsSelectedResponse(obj_id self, dictionary params) throws InterruptedException
	{
		if (params == null || params.isEmpty())
		{
			return;
		}
		
		obj_id player = sui.getPlayerId(params);
		if (!isIdValid(player))
		{
			return;
		}
		
		int button = sui.getIntButtonPressed(params);
		if (button == sui.BP_CANCEL)
		{
			sendSystemMessage(player, new string_id("player_structure", "find_items_search_cancelled"));
			cleanupFindItemScriptVars(player);
			cleanupFindItemScriptVars(self);
			return;
		}
		
		obj_id[] searchList = utils.getObjIdArrayScriptVar(self, "findItems.searchList");
		if (searchList == null || searchList.length < 1)
		{
			cleanupFindItemScriptVars(player);
			cleanupFindItemScriptVars(self);
			return;
		}
		
		int item_selected = sui.getListboxSelectedRow(params);
		if (item_selected < 0)
		{
			sendSystemMessage(player, new string_id("player_structure", "find_items_no_selection"));
			cleanupFindItemScriptVars(player);
			cleanupFindItemScriptVars(self);
			return;
		}
		
		obj_id selectedItem = searchList[item_selected];
		if (isIdValid(selectedItem))
		{
			moveItemInBuildingToPlayer(player, selectedItem);
			repeatSearchItemsKeywordList(self, player, searchList);
			return;
		}
		
		cleanupFindItemScriptVars(player);
		cleanupFindItemScriptVars(self);
		return;
	}
	
	
	public static void removeStructureFromMaintenance(obj_id droid, obj_id pcd, int indexOfStructure) throws InterruptedException
	{
		if (!isIdValid(pcd))
		{
			return;
		}
		
		obj_id[] struct_list = getObjIdArrayObjVar(pcd, "module_data.maint_list.ids");
		
		if (struct_list == null || struct_list.length <= 0)
		{
			return;
		}
		
		if (indexOfStructure < 0 || indexOfStructure >= struct_list.length)
		{
			return;
		}
		
		obj_id[] new_struct_list = new obj_id[struct_list.length - 1];
		
		int currentIndex = 0;
		
		for (int i = 0; i < struct_list.length; i++)
		{
			testAbortScript();
			if (i == indexOfStructure)
			{
				if (isIdValid(droid))
				{
					removeObjVar(droid, "module_data.maint_list.loc_"+ i);
				}
				
				removeObjVar(pcd, "module_data.maint_list.loc_"+ i);
				continue;
			}
			
			if (i < indexOfStructure)
			{
				new_struct_list[i] = struct_list[i];
			}
			else
			{
				new_struct_list[i - 1] = struct_list[i];
				
				if (hasObjVar(pcd, "module_data.maint_list.loc_" + i))
				{
					if (isIdValid(droid))
					{
						setObjVar(droid, "module_data.maint_list.loc_"+ (i - 1), getLocationObjVar(droid, "module_data.maint_list.loc_"+ i));
					}
					
					setObjVar(pcd, "module_data.maint_list.loc_"+ (i - 1), getLocationObjVar(pcd, "module_data.maint_list.loc_"+ i));
				}
				else
				{
					if (isIdValid(droid))
					{
						removeObjVar(droid, "module_data.maint_list.loc_"+ (i - 1));
					}
					
					removeObjVar(pcd, "module_data.maint_list.loc_"+ (i - 1));
				}
			}
		}
		
		if (isIdValid(droid))
		{
			setObjVar(droid, "module_data.maint_list.ids", new_struct_list);
		}
		
		setObjVar(pcd, "module_data.maint_list.ids", new_struct_list);
	}
}
