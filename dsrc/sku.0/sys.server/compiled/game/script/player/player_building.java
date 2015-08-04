package script.player;

import script.*;
import script.base_class.*;
import script.combat_engine.*;
import java.util.Arrays;
import java.util.Hashtable;
import java.util.Vector;
import script.base_script;


import java.lang.Math;
import script.library.ai_lib;
import script.library.callable;
import script.library.city;
import script.library.colors_hex;
import script.library.create;
import script.library.dressup;
import script.library.hq;
import script.library.incubator;
import script.library.money;
import script.library.pclib;
import script.library.performance;
import script.library.player_structure;
import script.library.prose;
import script.library.resource;
import script.library.session;
import script.library.skill;
import script.library.static_item;
import script.library.sui;
import script.library.tcg;
import script.library.trial;
import script.library.turnstile;
import script.library.utils;
import script.library.vendor_lib;
import script.library.xp;


public class player_building extends script.base_script
{
	public player_building()
	{
	}
	public static final String LOGGING_CATEGORY = "vendor";
	public static final boolean LOGGING_ON = true;
	
	public static final String DATATABLE_HEIGHT = "datatables/structure/cell_height.iff";
	public static final String STF = "player_structure";
	
	public static final String SCRIPTVAR_SAVE_RESTORE_DECORATION_SUI_ID = "saveRestoreDecorationSuiId";
	public static final String SCRIPTVAR_SAVE_RESTORE_DECORATION_SAVE_SLOT_ID = "saveRestoreDecorationSaveSlotId";
	public static final String SCRIPTVAR_RESTORE_DECORATION_SAVE_SLOT_CHOICE = "restoreDecorationSaveSlotChoice";
	public static final String SCRIPTVAR_RESTORE_DECORATION_TARGET_ROOMS = "restoreDecorationTargetRooms";
	public static final String SCRIPTVAR_RESTORE_DECORATION_TARGET_ROOM_INDEX = "restoreDecorationTargetRoomIndex";
	public static final String SCRIPTVAR_RESTORE_DECORATION_CURRENT_POB_ROOMS = "restoreDecorationCurrentPobRooms";
	
	public static final string_id SID_TRIAL_STRUCTURE = new string_id(STF, "trial_structure");
	public static final string_id SID_TRIAL_NO_MODIFY = new string_id(STF, "trial_no_modify");
	public static final string_id SID_NO_DECLARE = new string_id(STF, "trial_no_declare");
	public static final string_id SID_NO_TRANSFER = new string_id(STF, "trial_no_transfer");
	public static final string_id SID_NO_VENDOR = new string_id(STF, "trial_no_vendor");
	public static final string_id SID_NOT_IN_BUILDING = new string_id(STF, "not_in_building");
	public static final string_id SID_CONTENTS_NOT_LOADED = new string_id(STF, "contents_not_loaded");
	public static final string_id SID_BUILDING_HAS_NOTRADE = new string_id(STF, "building_has_notrade");
	
	public static final string_id SID_CANT_TRANSFER_TO_CITY_BANNED = new string_id("city/city", "cant_transfer_to_city_banned");
	
	public static final string_id SID_CONSTRUCTION_COMPLETE = new string_id("player_structure", "construction_complete");
	public static final string_id SID_CONSTRUCTION_COMPLETE_LOT_LIMIT_EXCEEDED = new string_id("player_structure", "construction_complete_lot_limit_exceeded");
	
	public static final string_id SID_CONSTRUCTION_COMPLETE_SUBJECT = new string_id("player_structure", "construction_complete_subject");
	public static final string_id SID_CONSTRUCTION_COMPLETE_SENDER = new string_id("player_structure", "construction_complete_sender");
	
	public static final string_id SID_SYS_FULL_VENDORS = new string_id("player_structure", "full_vendors");
	public static final string_id SID_SYS_ALREADY_CREATING = new string_id("player_structure", "already_creating_vendor_greeter");
	public static final string_id SID_SYS_VENDOR_NOT_INITIALIZED = new string_id("player_structure", "need_to_initialize_vendor");
	public static final string_id SID_SYS_CREATE_FAILED = new string_id("player_structure", "create_failed");
	public static final string_id SID_SYS_CREATE_SUCCESS = new string_id("player_structure", "create_success");
	public static final string_id SID_SYS_CREATE_BAZAAR_FAILED = new string_id("player_structure", "create_bazaar_failed");
	public static final string_id SID_SYS_CREATE_BAZAAR_SUCCESS = new string_id("player_structure", "create_bazaar_success");
	public static final string_id SID_CANT_VENDOR = new string_id("player_structure", "cant_move_vendor");
	public static final string_id SID_CANT_MOVE_OBJECT = new string_id("player_structure", "cant_move_object");
	public static final string_id SID_TURNSTILE_EXPIRE = new string_id("player_structure", "turnstile_expire");
	public static final string_id SID_CAN_RECLAIM_DEED = new string_id("player_structure", "can_reclaim_deed");
	public static final string_id SID_DROID_ASSIGNED_TO_MAINTAIN = new string_id("player_structure", "droid_assigned_to_maintain");
	public static final string_id SID_SUI_AMOUNT_TO_WITHDRAW = new string_id("player_structure", "sui_amount_to_withdraw");
	public static final string_id INVENTORY_FULL_GENERIC = new string_id("player_structure", "inventory_full_generic");
	public static final string_id SID_PACKUP_TOO_FAR_AWAY = new string_id("player_structure", "too_far_away_to_pack_house");
	public static final string_id SID_HOUSE_PACKUP_FAILED = new string_id("player_structure", "house_packup_failed");
	
	public static final String TBL_VENDOR_TYPES = "datatables/vendor/vendor_types.iff";
	public static final String TBL_TERMINAL_TYPES = "datatables/vendor/vendor_terminal_types.iff";
	public static final String TBL_DROID_TYPES = "datatables/vendor/vendor_droid_types.iff";
	public static final String TBL_PLAYER_TYPES = "datatables/vendor/vendor_player_types.iff";
	public static final String TBL_ALLNPC_TYPES = "datatables/vendor/vendor_allnpc_types.iff";
	public static final String TBL_SPECIAL_PLAYER_VENDOR_TYPES = "datatables/vendor/vendor_player_types_special.iff";
	
	public static final String VENDOR_VAR_PREFIX = "vendor_prefix";
	public static final String CREATING_VENDOR = VENDOR_VAR_PREFIX+".creatingVendor";
	
	public static final string_id SID_VENDOR_MAINT_ACCEPTED = new string_id("player_structure", "vendor_maint_accepted");
	
	public static final string_id SID_OBSCENE = new string_id("player_structure", "obscene");
	
	public static final string_id SID_SET_NAME_TITLE = new string_id("sui","set_name_title");
	public static final string_id SID_SET_NAME_PROMPT = new string_id("sui","set_name_prompt");
	
	public static final string_id SID_ASSIGN_DROID_TITLE = new string_id("sui","assign_droid_title");
	public static final string_id SID_ASSIGN_DROID_PROMPT = new string_id("sui","assign_droid_prompt");
	
	public static final string_id PROSE_SIGN_NAME_UPDATED = new string_id("player_structure","prose_sign_name_updated");
	
	public static final string_id SID_DEACTIVATE_FACTORY_FOR_DELETE = new string_id("player_structure", "deactivate_factory_for_delete");
	public static final string_id SID_REMOVE_SCHEMATIC_FOR_DELETE = new string_id("player_structure", "remove_schematic_for_delete");
	public static final string_id SID_CLEAR_INPUT_HOPPER_FOR_DELETE = new string_id("player_structure", "clear_input_hopper_for_delete");
	public static final string_id SID_CLEAR_OUTPUT_HOPPER_FOR_DELETE = new string_id("player_structure", "clear_output_hopper_for_delete");
	
	public static final string_id SID_CLEAR_BUILDING_FOR_DELETE = new string_id("player_structure", "clear_building_for_delete");
	public static final string_id SID_STORE_PETS_FOR_DELETE = new string_id("player_structure", "store_pets_for_delete");
	
	public static final string_id SID_PENDING_DESTROY = new string_id("player_structure", "pending_destroy");
	
	public static final string_id SID_CANT_PLACE_CIVIC = new string_id("player_structure", "cant_place_civic");
	public static final string_id SID_CANT_PLACE_UNIQUE = new string_id("player_structure", "cant_place_unique");
	public static final string_id SID_CANT_PLACE_JEDI = new string_id("player_structure", "cant_place_jedi");
	public static final string_id SID_CANT_PLACE_MOUNTED = new string_id("player_structure", "cant_place_mounted");
	
	public static final string_id SID_WRONG_STATE = new string_id("error_message","wrong_state");
	
	public static final string_id SID_PLACED_LAST_BASE = new string_id("faction_perk", "faction_base_unit_last");
	public static final string_id SID_PLACED_NEXT_TO_LAST = new string_id("faction_perk", "faction_base_one_more");
	public static final string_id SID_NO_RIGHTS = new string_id("player_structure", "no_transfer_rights");
	
	public static final string_id SID_EVACUATE_FACILITY = new string_id("hq", "self_destruct_evacuate");
	
	public static final string_id MOVED_CTRL_OBJ = new string_id("spam", "magic_painting_ctrl_moved");
	public static final string_id MOVED_BACKDROP_GENERATOR = new string_id("spam", "backdrop_generator_moved");
	public static final string_id MOVED_PAINTING = new string_id("spam", "magic_painting_moved");
	public static final string_id NO_MOVE_ITEM = new string_id("spam", "no_move_item");
	public static final string_id SID_UNABLE_TO_PARSE = new string_id(STF, "unable_to_parse");
	public static final string_id SID_VALID_AMOUNT = new string_id(STF, "enter_valid_over_zero");
	public static final string_id SID_RESERVE_REPORT = new string_id(STF, "reserve_report");
	public static final string_id SID_DEPOSIT_SUCCESS = new string_id(STF, "deposit_successful");
	public static final string_id SID_NOT_ENOUGH_ENERGY = new string_id(STF, "not_enough_energy");
	public static final string_id SID_POWER_DEPOSIT_INCOMPLETE = new string_id(STF, "power_deposit_incomplete");
	public static final string_id SID_ERROR_POWER_DEPOSIT = new string_id(STF, "error_power_deposit");
	public static final string_id SID_POWER_DEPOSIT_FAIL = new string_id(STF, "power_deposit_failed");
	public static final string_id SID_SELECT_POWER_AMOUNT = new string_id(STF, "select_power_amount");
	public static final string_id SID_CURRENT_POWER_AMOUNT = new string_id(STF, "current_power_value");
	public static final string_id SID_POWER_NAME = new string_id(STF, "power_name");
	public static final string_id SID_POWER_QUALITY = new string_id(STF, "power_quality");
	public static final string_id SID_NO_GEO_POWER = new string_id(STF, "incubator_no_geothermal");
	public static final string_id SID_POWER_REMOVED = new string_id(STF, "power_removed");
	
	public static final string_id SID_TCG_VENDOR_CTS_WARNING = new string_id("player_vendor", "tcg_vendor_in_player_inventory");
	
	public static final string_id SID_ROTATE_CONFIRM_SAVE = new string_id(STF, "rotate_confirm_save");
	public static final string_id SID_ROTATE_RESTORE_EMPTY_SLOT = new string_id(STF, "rotate_restore_empty_slot");
	
	public static final string_id SID_DESTRUCTION_LOCKED = new string_id("city/city", "destruction_locked");
	
	public static final int VAR_TRUE = 1;
	
	
	public int OnPlaceStructure(obj_id self, obj_id player, obj_id deed, location position, int rotation) throws InterruptedException
	{
		LOG("LOG_CHANNEL", "player_building::OnPlaceStructure");
		LOG("LOG_CHANNEL", "player ->"+ player + " deed ->"+ deed + " pos ->"+ position + " rotation ->"+ rotation);
		
		if (!isIdValid(player))
		{
			LOG("LOG_CHANNEL", "player_structure::OnPlaceStructure -- player is invalid.");
			return SCRIPT_CONTINUE;
		}
		
		if (!isIdValid(deed))
		{
			LOG("LOG_CHANNEL", "player_structure::OnPlaceStructure -- deed is invalid.");
			return SCRIPT_CONTINUE;
		}
		
		obj_id owner = utils.getContainingPlayer(deed);
		
		if (player == owner)
		{
			
		}
		else
		{
			sendSystemMessage(player, new string_id(STF, "no_possession"));
			
			if (isIdValid(owner))
			{
				CustomerServiceLog("playerStructure", "%TU tried to place a building, but no longer possesses the deed ("+ deed + ")! %TT currently has the deed.", player, owner);
			}
			else
			{
				CustomerServiceLog("playerStructure", "%TU tried to place a building, but no longer possesses the deed ("+ deed + ")!", player);
			}
			return SCRIPT_CONTINUE;
		}
		
		String template = player_structure.getDeedTemplate(deed);
		if (template == null)
		{
			LOG("LOG_CHANNEL", "player_structure::OnPlaceStructure -- Unable to find template on deed.");
			return SCRIPT_CONTINUE;
		}
		
		String fp_template = player_structure.getFootprintTemplate(template);
		if (fp_template == null || fp_template.equals(""))
		{
			return SCRIPT_CONTINUE;
		}
		
		float placement_height = canPlaceStructure(fp_template, position, rotation);
		
		LOG("LOG_CHANNEL", "placement_height ->"+ placement_height);
		
		if (placement_height == -9997.0f)
		{
			LOG("LOG_CHANNEL", player + " ->Internal code error: canPlaceStructure");
			sendSystemMessage(player, new string_id(STF, "error_canplacestructure"));
			return SCRIPT_CONTINUE;
		}
		
		if (placement_height == -9998.0f)
		{
			LOG("LOG_CHANNEL", player + " ->Internal script error: OnPlaceStructure");
			sendSystemMessage(player, new string_id(STF, "error_onplacestructure"));
			return SCRIPT_CONTINUE;
		}
		
		if (placement_height == -9999.0f)
		{
			LOG("LOG_CHANNEL", player + " ->There is no room to place the structure here.");
			sendSystemMessage(player, new string_id(STF, "no_room"));
			return SCRIPT_CONTINUE;
		}
		
		if (player_structure.isCivicTemplate(template) && (template.indexOf("cityhall") == -1))
		{
			
			if (!canPlaceCivic(player, deed, position, template))
			{
				sendSystemMessage(player, SID_CANT_PLACE_CIVIC);
				return SCRIPT_CONTINUE;
			}
			
			if (!canPlaceUnique(player, deed, position, template))
			{
				sendSystemMessage(player, SID_CANT_PLACE_UNIQUE);
				return SCRIPT_CONTINUE;
			}
		}
		
		if (!player_structure.canPlaceStructure(self, template, position, deed))
		{
			return SCRIPT_CONTINUE;
		}
		
		position.y = getHeightAtLocation (position.x, position.z);
		
		if (isGameObjectTypeOf(getGameObjectType(deed), GOT_data_house_control_device))
		{
			player_structure.replacePackedStructure(deed, self, position, rotation, placement_height);
			return SCRIPT_CONTINUE;
		}
		
		obj_id structure = player_structure.createTemporaryStructure(deed, self, position, rotation, placement_height);
		if (!isIdValid(structure))
		{
			debugSpeakMsg(self, "OnPlaceStructure: createObject failed");
			
			return SCRIPT_OVERRIDE;
		}
		
		if (player_structure.isFactionPerkBase(template))
		{
			
			if (hasObjVar(player,"factionBaseCount"))
			{
				int factionBaseCount = getIntObjVar(player,"factionBaseCount");
				int updatedFactionBaseCount = factionBaseCount + 1;
				setObjVar(player,"factionBaseCount",updatedFactionBaseCount);
				if (updatedFactionBaseCount == player_structure.MAX_BASE_COUNT)
				{
					sendSystemMessage(self, SID_PLACED_LAST_BASE);
				}
				else if (updatedFactionBaseCount == player_structure.MAX_BASE_COUNT-1)
				{
					sendSystemMessage(self, SID_PLACED_NEXT_TO_LAST);
				}
				else
				{
					sendSystemMessageProse(self, prose.getPackage(new string_id ("faction_perk", "faction_base_unit_used"), (player_structure.MAX_BASE_COUNT-updatedFactionBaseCount)));
				}
				
			}
			else
			{
				setObjVar(player,"factionBaseCount",1);
				sendSystemMessageProse(self, prose.getPackage(new string_id ("faction_perk", "faction_base_unit_used"), (player_structure.MAX_BASE_COUNT-1)));
				
			}
		}
		return SCRIPT_CONTINUE;
	}
	
	
	public int OnPermissionListModify(obj_id self, obj_id player, String name, String listName, String action) throws InterruptedException
	{
		if (utils.isFreeTrial(player))
		{
			sendSystemMessage(player, SID_TRIAL_NO_MODIFY);
			return SCRIPT_CONTINUE;
		}
		
		LOG("debug", "player_building::OnPermissionListModify");
		
		obj_id structure = player_structure.getStructure(self);
		if ((structure == null)||(structure == obj_id.NULL_ID))
		{
			LOG("LOG_CHANNEL", "You must be in a building, be near an installation, or have one targeted to do that.");
			sendSystemMessage(player, new string_id(STF, "no_building"));
			return SCRIPT_CONTINUE;
		}
		
		if (name == null)
		{
			return SCRIPT_CONTINUE;
		}
		
		if (name.length() > 40)
		{
			sendSystemMessage(player, new string_id(STF, "permission_40_char"));
			return SCRIPT_CONTINUE;
		}
		
		if (!player_structure.isInAdminRange(structure, self))
		{
			return SCRIPT_CONTINUE;
		}
		if (listName.equals("ENTRY"))
		{
			player_structure.modifyEntryList(structure, name, self);
		}
		
		if (listName.equals("BAN"))
		{
			player_structure.modifyBanList(structure, name, self);
		}
		
		if (listName.equals("ADMIN"))
		{
			player_structure.modifyAdminList(structure, name, self);
		}
		
		if (listName.equals("HOPPER"))
		{
			player_structure.modifyHopperList(structure, name, self);
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int OnContentsLoadedForBuildingToBePacked(obj_id self, obj_id structure) throws InterruptedException
	{
		LOG("sissynoid", "Loading Finished - Triggered OnContentsLoadedForBuildingToBePacked");
		if (!isValidId(structure))
		{
			return SCRIPT_CONTINUE;
		}
		
		if (!player_structure.canPackStructureWithVendors(self, structure))
		{
			if (player_structure.isAbandoned(structure))
			{
				messageTo(self, "handleFailedStructurePackup", null, 0, false);
			}
			if (player_structure.isCityAbandoned(structure))
			{
				sendSystemMessage(self, new string_id("player_structure", "unable_to_city_pack"));
			}
			return SCRIPT_CONTINUE;
		}
		
		if (player_structure.isAbandoned(structure))
		{
			player_structure.finalizePackUp(self, structure);
			return SCRIPT_CONTINUE;
		}
		
		if (player_structure.isCityAbandoned(structure))
		{
			player_structure.finalizeCityPackUp(self, structure);
			LOG("sissynoid", "Contents are Now Loaded - Messaging packUpCityAbandonedStructure");
			return SCRIPT_CONTINUE;
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int OnMadeAuthoritative(obj_id self) throws InterruptedException
	{
		LOG("sissynoid", "OnMadeAuthoritative: HouseAndPlayer have been made authoritative");
		if (utils.hasScriptVar(self, "requestedSameServerToAbandonHouse"))
		{
			LOG("sissynoid", "Shoud not be here!");
			obj_id house = utils.getObjIdScriptVar(self, "requestedSameServerToAbandonHouse");
			
			utils.removeScriptVarTree(self, "requestedSameServerToAbandonHouse");
			utils.removeScriptVarTree(self, "timeOfLastSameServerRequest");
			
			if (!player_structure.canPlayerPackAbandonedStructure(self, house))
			{
				messageTo(self, "handleFailedStructurePackup", null, 0, false);
				sendSystemMessage(self, new string_id("player_structure", "abandoned_structure_pack_up_try_again_later"));
				return SCRIPT_CONTINUE;
			}
			
			dictionary params = new dictionary();
			params.put("house", house);
			params.put("player", self);
			
			messageTo(house, "packAbandonedBuilding", params, 4, false);
			
			if (!hasObjVar(self, "housePackup"))
			{
				String recipient = getPlayerName(self);
				utils.sendMail(new string_id("spam", "email_title"), new string_id("spam", "email_body"), recipient, "Galactic Vacant Building Demolishing Movement");
			}
		}
		if (utils.hasScriptVar(self, "cityRequestedSameServerToAbandonHouse"))
		{
			obj_id player = self;
			obj_id house = utils.getObjIdScriptVar(self, "cityRequestedSameServerToAbandonHouse");
			LOG("sissynoid", "OnMadeAuthoritative: City Packup is a Go for Structure("+ house +") by player ("+ player +")");
			utils.removeScriptVarTree(self, "cityRequestedSameServerToAbandonHouse");
			utils.removeScriptVarTree(self, "cityTimeOfLastSameServerRequest");
			
			player_structure.confirmCityAbandonedAndPack(house, player);
		}
		return SCRIPT_CONTINUE;
	}
	
	
	public int OnPlayerVendorCountReply(obj_id self, int count) throws InterruptedException
	{
		if (count < 0)
		{
			
			removeVendorVars(self);
			return SCRIPT_CONTINUE;
		}
		
		int creatingVendor = 0;
		if (utils.hasScriptVar(self, CREATING_VENDOR))
		{
			creatingVendor = utils.getIntScriptVar(self, CREATING_VENDOR);
		}
		
		blog("player_building.OnPlayerVendorCountReply -- "+ creatingVendor + " / "+ count);
		
		if (creatingVendor == 1)
		{
			
			setObjVar(self, "used_vendor_slots", count);
			
			if (getSkillStatMod(self, "manage_vendor") <= count)
			{
				
				removeVendorVars(self);
				sendSystemMessage(self, SID_SYS_FULL_VENDORS);
				return SCRIPT_CONTINUE;
			}
			
			String[] rawVendorTypes = dataTableGetStringColumn(TBL_VENDOR_TYPES, 0);
			if (rawVendorTypes == null)
			{
				removeVendorVars(self);
				return SCRIPT_CONTINUE;
			}
			
			int hiringMod = getSkillStatMod(self, "hiring");
			blog("player_building.OnPlayerVendorCountReply hiringMod: "+ hiringMod);
			int hiringRequirement = 10;
			Vector vendorTypes = new Vector();
			vendorTypes.setSize(0);
			
			blog("player_building.OnPlayerVendorCountReply rawVendorTypes.length: "+ rawVendorTypes.length);
			
			String[] possibleVendorTypes = new String[rawVendorTypes.length];
			
			for (int i = 0; i < rawVendorTypes.length; i++)
			{
				testAbortScript();
				possibleVendorTypes[i] = "@player_structure:"+rawVendorTypes[i];
			}
			
			for (int i=0; i<3; i++)
			{
				testAbortScript();
				if (hiringMod >= hiringRequirement)
				{
					vendorTypes = utils.addElement(vendorTypes, possibleVendorTypes[i]);
				}
				hiringRequirement += 10;
			}
			
			if (vendorTypes == null)
			{
				
				removeVendorVars(self);
				return SCRIPT_CONTINUE;
			}
			sui.listbox(self, self, "@player_structure:vendor_type_d", sui.OK_CANCEL, "@player_structure:vendor_type_t", vendorTypes, "handleVendorTypeSelect", true);
		}
		return SCRIPT_CONTINUE;
	}
	
	
	public int rotateFurniture(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (hasScript(self, performance.MUSIC_HEARTBEAT_SCRIPT))
		{
			string_id message = new string_id("player_structure", "cant_move_while_entertaining");
			sendSystemMessage(self, message);
			return SCRIPT_CONTINUE;
		}
		
		boolean canRollAndPitch = (player_structure.canRotateFurnitureInPitchRollAxes(self) || isGod(self));
		
		java.util.StringTokenizer st = new java.util.StringTokenizer(params);
		if (st.countTokens() == 0)
		{
			if (!canRollAndPitch)
			{
				sendSystemMessage(self, new string_id(STF, "formet_rotratefurniture_degrees"));
			}
			else
			{
				sendSystemMessage(self, new string_id(STF, "formet_enhanced_rotratefurniture_degrees"));
			}
			
			return SCRIPT_CONTINUE;
		}
		
		String direction = st.nextToken().toUpperCase();
		boolean actionYaw = false;
		boolean actionPitch = false;
		boolean actionRoll = false;
		boolean actionRandom = false;
		boolean actionReset = false;
		boolean actionCopy = false;
		boolean actionSave = false;
		boolean actionRestore = false;
		boolean actionSetQuaternion = false;
		if (direction.equals("YAW"))
		{
			actionYaw = true;
		}
		else if (direction.equals("PITCH"))
		{
			if (!canRollAndPitch)
			{
				sendSystemMessage(self, new string_id(STF, "formet_rotratefurniture_degrees"));
				return SCRIPT_CONTINUE;
			}
			
			actionPitch = true;
		}
		else if (direction.equals("ROLL"))
		{
			if (!canRollAndPitch)
			{
				sendSystemMessage(self, new string_id(STF, "formet_rotratefurniture_degrees"));
				return SCRIPT_CONTINUE;
			}
			
			actionRoll = true;
		}
		else if (direction.equals("RANDOM"))
		{
			if (!canRollAndPitch)
			{
				sendSystemMessage(self, new string_id(STF, "formet_rotratefurniture_degrees"));
				return SCRIPT_CONTINUE;
			}
			
			actionRandom = true;
		}
		else if (direction.equals("RESET"))
		{
			actionReset = true;
		}
		else if (direction.equals("COPY"))
		{
			actionCopy = true;
		}
		else if (direction.equals("SAVE"))
		{
			actionSave = true;
		}
		else if (direction.equals("RESTORE"))
		{
			actionRestore = true;
		}
		else if (direction.equals("(^-,=+_)INTERNAL_USE_ONLY_" + self + "_QUATERNION(,+-=_^)"))
		{
			if (!canRollAndPitch)
			{
				sendSystemMessage(self, new string_id(STF, "formet_rotratefurniture_degrees"));
				return SCRIPT_CONTINUE;
			}
			
			actionSetQuaternion = true;
		}
		else
		{
			if (!canRollAndPitch)
			{
				sendSystemMessage(self, new string_id(STF, "formet_rotratefurniture_degrees"));
			}
			else
			{
				sendSystemMessage(self, new string_id(STF, "formet_enhanced_rotratefurniture_degrees"));
			}
			
			return SCRIPT_CONTINUE;
		}
		
		int saveSlot = 0;
		int rotation = 0;
		float qw = 1.0f;
		float qx = 0.0f;
		float qy = 0.0f;
		float qz = 0.0f;
		if (actionYaw || actionPitch || actionRoll)
		{
			if (st.hasMoreTokens())
			{
				String rot_str = st.nextToken().toUpperCase();
				if (rot_str.equals("RANDOM"))
				{
					actionRandom = true;
				}
				else
				{
					
					try
					{
						rotation = Integer.parseInt(rot_str);
					}
					catch (NumberFormatException err)
					{
						sendSystemMessage(self, new string_id(STF, "rotate_params"));
						return SCRIPT_CONTINUE;
					}
					
					if (rotation < -180 || rotation > 180)
					{
						sendSystemMessage(self, new string_id(STF, "rotate_params"));
						return SCRIPT_CONTINUE;
					}
				}
			}
			else
			{
				rotation = getFurnitureRotationDegree(self);
			}
			
			if ((rotation == 0) && !actionRandom)
			{
				return SCRIPT_CONTINUE;
			}
		}
		else if (actionSave || actionRestore)
		{
			if (st.hasMoreTokens())
			{
				saveSlot = utils.stringToInt(st.nextToken());
				
				if (saveSlot < 1 || saveSlot > 10)
				{
					sendSystemMessage(self, new string_id(STF, "rotate_save_slot_params"));
					return SCRIPT_CONTINUE;
				}
			}
			else
			{
				if (!canRollAndPitch)
				{
					sendSystemMessage(self, new string_id(STF, "formet_rotratefurniture_degrees"));
				}
				else
				{
					sendSystemMessage(self, new string_id(STF, "formet_enhanced_rotratefurniture_degrees"));
				}
				
				return SCRIPT_CONTINUE;
			}
		}
		else if (actionSetQuaternion)
		{
			if (!st.hasMoreTokens())
			{
				return SCRIPT_CONTINUE;
			}
			qw = utils.stringToFloat(st.nextToken());
			
			if (!st.hasMoreTokens())
			{
				return SCRIPT_CONTINUE;
			}
			qx = utils.stringToFloat(st.nextToken());
			
			if (!st.hasMoreTokens())
			{
				return SCRIPT_CONTINUE;
			}
			qy = utils.stringToFloat(st.nextToken());
			
			if (!st.hasMoreTokens())
			{
				return SCRIPT_CONTINUE;
			}
			qz = utils.stringToFloat(st.nextToken());
			
			if (!st.hasMoreTokens())
			{
				return SCRIPT_CONTINUE;
			}
		}
		
		obj_id intendedTarget = getIntendedTarget(self);
		obj_id lookAtTarget = getLookAtTarget(self);
		
		if (st.hasMoreTokens())
		{
			target = obj_id.getObjId(Long.valueOf(st.nextToken()));
		}
		else
		{
			if (!actionCopy)
			{
				if (isIdValid(intendedTarget))
				{
					target = intendedTarget;
				}
				else if (isIdValid(lookAtTarget))
				{
					target = lookAtTarget;
				}
			}
			else
			{
				if (isIdValid(lookAtTarget))
				{
					target = lookAtTarget;
				}
			}
		}
		
		if (!isIdValid(target))
		{
			sendSystemMessage(self, new string_id(STF, "rotate_what"));
			return SCRIPT_CONTINUE;
		}
		
		if (actionCopy && !isIdValid(intendedTarget))
		{
			sendSystemMessage(self, new string_id(STF, "rotate_move_copy_no_intended_target"));
			return SCRIPT_CONTINUE;
		}
		
		boolean isVendor = hasCondition(target, CONDITION_VENDOR);
		if (isVendor && (actionPitch || actionRoll || actionCopy || actionRestore || actionSetQuaternion))
		{
			sendSystemMessage(self, new string_id(STF, "cant_rotate_vendor"));
			return SCRIPT_CONTINUE;
		}
		
		if (!actionSave)
		{
			if (!isMoveCommandValid(self, target))
			{
				return SCRIPT_CONTINUE;
			}
			
			session.logActivity(self, session.ACTIVITY_DECORATE);
		}
		
		if (actionYaw)
		{
			if (actionRandom)
			{
				modifyYaw(target, rand(-180, 180));
			}
			else
			{
				modifyYaw(target, rotation);
			}
		}
		else if (actionPitch)
		{
			if (actionRandom)
			{
				modifyPitch(target, rand(-180, 180));
			}
			else
			{
				modifyPitch(target, rotation);
			}
		}
		else if (actionRoll)
		{
			if (actionRandom)
			{
				modifyRoll(target, rand(-180, 180));
			}
			else
			{
				modifyRoll(target, rotation);
			}
		}
		else if (actionRandom)
		{
			modifyYaw(target, rand(-180, 180));
			
			if (!isVendor)
			{
				modifyPitch(target, rand(-180, 180));
				modifyRoll(target, rand(-180, 180));
			}
		}
		else if (actionReset)
		{
			setQuaternion(target, 1.0f, 0.0f, 0.0f, 0.0f);
		}
		else if (actionCopy)
		{
			if (target != intendedTarget)
			{
				float[] quaternion = getQuaternion(intendedTarget);
				if ((quaternion != null) && (quaternion.length == 4))
				{
					setQuaternion(target, quaternion[0], quaternion[1], quaternion[2], quaternion[3]);
				}
			}
		}
		else if (actionSave)
		{
			if (saveSlot >= 1 && saveSlot <= 10)
			{
				float[] quaternion = getQuaternion(target);
				if ((quaternion != null) && (quaternion.length == 4))
				{
					setObjVar(self, "rotateFurnitureSaveSlot."+ saveSlot, quaternion);
					
					prose_package pp = prose.getPackage(SID_ROTATE_CONFIRM_SAVE);
					prose.setDI(pp, saveSlot);
					sendSystemMessageProse(self, pp);
				}
			}
		}
		else if (actionRestore)
		{
			if (saveSlot >= 1 && saveSlot <= 10)
			{
				float[] quaternion = getFloatArrayObjVar(self, "rotateFurnitureSaveSlot."+ saveSlot);
				if ((quaternion != null) && (quaternion.length == 4))
				{
					setQuaternion(target, quaternion[0], quaternion[1], quaternion[2], quaternion[3]);
				}
				else
				{
					prose_package pp = prose.getPackage(SID_ROTATE_RESTORE_EMPTY_SLOT);
					prose.setDI(pp, saveSlot);
					sendSystemMessageProse(self, pp);
				}
			}
		}
		else if (actionSetQuaternion)
		{
			setQuaternion(target, qw, qx, qy, qz);
		}
		
		messageTo(target, "furniture_rotated", null, 1.0f, false);
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int moveFurniture(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		
		if (hasScript(self, performance.MUSIC_HEARTBEAT_SCRIPT))
		{
			string_id message = new string_id("player_structure", "cant_move_while_entertaining");
			sendSystemMessage(self, message);
			return SCRIPT_CONTINUE;
		}
		java.util.StringTokenizer st = new java.util.StringTokenizer(params);
		if (st.countTokens() == 0)
		{
			sendSystemMessage(self, new string_id(STF, "format_movefurniture_distance"));
			return SCRIPT_CONTINUE;
		}
		
		String direction = st.nextToken().toUpperCase();
		String dist_str = null;
		
		if (!direction.equals("FORWARD") && !direction.equals("BACK") && !direction.equals("LEFT") && !direction.equals("RIGHT") && !direction.equals("UP") && !direction.equals("DOWN") && !direction.equals("COPY"))
		{
			String targetName = getEncodedName(target).toUpperCase();
			if (targetName.startsWith("@"))
			{
				targetName = localize(getNameStringId(target)).toUpperCase();
			}
			
			if (!targetName.startsWith("FORWARD") && !targetName.startsWith("BACK") && !targetName.startsWith("LEFT") && !targetName.startsWith("RIGHT") && !targetName.startsWith("UP") && !targetName.startsWith("DOWN") && !targetName.startsWith("COPY"))
			{
				sendSystemMessage(self, new string_id(STF, "format_movefurniture_distance"));
				return SCRIPT_CONTINUE;
			}
			else
			{
				dist_str = direction;
				
				if (targetName.startsWith("UP"))
				{
					direction = "UP";
				}
				else if (targetName.startsWith("DOWN"))
				{
					direction = "DOWN";
				}
				else if (targetName.startsWith("BACK"))
				{
					direction = "BACK";
				}
				else if (targetName.startsWith("FORWARD"))
				{
					direction = "FORWARD";
				}
				else if (targetName.startsWith("LEFT"))
				{
					direction = "LEFT";
				}
				else if (targetName.startsWith("RIGHT"))
				{
					direction = "RIGHT";
				}
				else if (targetName.startsWith("COPY"))
				{
					direction = "COPY";
				}
			}
		}
		
		int distance = 0;
		boolean copyLocation = false;
		boolean copyHeight = false;
		if (st.hasMoreTokens() && dist_str == null)
		{
			dist_str = st.nextToken().toUpperCase();
			if (direction.equals("COPY"))
			{
				if (dist_str.equals("LOCATION"))
				{
					copyLocation = true;
				}
				else if (dist_str.equals("HEIGHT"))
				{
					copyHeight = true;
				}
				else
				{
					sendSystemMessage(self, new string_id(STF, "format_movefurniture_distance"));
					return SCRIPT_CONTINUE;
				}
			}
			else
			{
				int dist_int = utils.stringToInt(dist_str);
				if (dist_int != -1)
				{
					distance = dist_int;
				}
				
				if (distance < 1 || distance > 500)
				{
					sendSystemMessage(self, new string_id(STF, "movefurniture_params"));
					return SCRIPT_CONTINUE;
				}
			}
		}
		else if (dist_str != null)
		{
			dist_str = dist_str.toUpperCase();
			if (direction.equals("COPY"))
			{
				if (dist_str.equals("LOCATION"))
				{
					copyLocation = true;
				}
				else if (dist_str.equals("HEIGHT"))
				{
					copyHeight = true;
				}
				else
				{
					sendSystemMessage(self, new string_id(STF, "format_movefurniture_distance"));
					return SCRIPT_CONTINUE;
				}
			}
			else
			{
				int dist_int = utils.stringToInt(dist_str);
				if (dist_int != -1)
				{
					distance = dist_int;
				}
				
				if (distance < 1 || distance > 500)
				{
					sendSystemMessage(self, new string_id(STF, "movefurniture_params"));
					return SCRIPT_CONTINUE;
				}
			}
		}
		else if (direction.equals("COPY"))
		{
			sendSystemMessage(self, new string_id(STF, "format_movefurniture_distance"));
			return SCRIPT_CONTINUE;
		}
		else
		{
			distance = 100;
		}
		
		obj_id intendedTarget = getIntendedTarget(self);
		obj_id lookAtTarget = getLookAtTarget(self);
		
		if (st.hasMoreTokens())
		{
			target = obj_id.getObjId(Long.valueOf(st.nextToken()));
		}
		else
		{
			if (!direction.equals("COPY"))
			{
				if (isIdValid(intendedTarget))
				{
					target = intendedTarget;
				}
				else if (isIdValid(lookAtTarget))
				{
					target = lookAtTarget;
				}
			}
			else
			{
				if (isIdValid(lookAtTarget))
				{
					target = lookAtTarget;
				}
			}
		}
		
		if (!isIdValid(target))
		{
			sendSystemMessage(self, new string_id(STF, "move_what"));
			return SCRIPT_CONTINUE;
		}
		
		if ((direction.equals("COPY")) && !isIdValid(intendedTarget))
		{
			sendSystemMessage(self, new string_id(STF, "rotate_move_copy_no_intended_target"));
			return SCRIPT_CONTINUE;
		}
		
		if (hasObjVar(target, "unmoveable"))
		{
			sendSystemMessage(self, SID_CANT_MOVE_OBJECT);
			return SCRIPT_CONTINUE;
		}
		
		if (hasCondition(target, CONDITION_VENDOR))
		{
			sendSystemMessage(self, SID_CANT_VENDOR);
			return SCRIPT_CONTINUE;
		}
		
		if (!isMoveCommandValid(self, target))
		{
			return SCRIPT_CONTINUE;
		}
		
		location move_loc = null;
		session.logActivity(self, session.ACTIVITY_DECORATE);
		if (direction.equals("FORWARD") || direction.equals("BACK") || direction.equals("LEFT") || direction.equals("RIGHT"))
		{
			
			location loc = getLocation(target);
			float facing = getYaw(self);
			
			if (direction.equals("LEFT"))
			{
				facing -= 90;
				direction = "FORWARD";
			}
			else if (direction.equals("RIGHT"))
			{
				facing += 90;
				direction = "FORWARD";
			}
			
			float dist_scaled = (float)distance / 100.0f;
			
			float facing_rad = (float)Math.toRadians(facing);
			float x = dist_scaled * (float) Math.sin(facing_rad);
			float z = dist_scaled * (float)Math.cos(facing_rad);
			
			if (direction.equals("BACK"))
			{
				x = x * -1;
				z = z * -1;
			}
			
			LOG("LOG_CHANNEL", "x ->"+ x + " z ->"+ z + " dist ->"+ dist_scaled);
			move_loc = new location (x + loc.x, loc.y, z + loc.z, loc.area, loc.cell);
			LOG("LOG_CHANNEL", "move_loc ->"+ move_loc);
			
			if (!isValidInteriorLocation(move_loc))
			{
				sendSystemMessage(self, new string_id(STF, "not_valid_location"));
				return SCRIPT_CONTINUE;
			}
		}
		else if (direction.equals("UP") || direction.equals("DOWN"))
		{
			
			location loc = getLocation(target);
			float facing = getYaw(self);
			
			float dist_scaled = (float)distance / 100.0f;
			float y = dist_scaled;
			
			if (direction.equals("DOWN"))
			{
				y = y * -1;
			}
			
			LOG("LOG_CHANNEL", "y ->"+ y + " dist ->"+ dist_scaled);
			move_loc = new location (loc.x, y + loc.y, loc.z, loc.area, loc.cell);
			LOG("LOG_CHANNEL", "move_loc ->"+ move_loc);
			
			obj_id building = getTopMostContainer(target);
			String bldgstr = getTemplateName(building);
			String cellname = getCellName(building, loc.cell);
			float new_y = y + loc.y;
			
			if ((!utils.hasScriptVar(target, "vertical.template") || !utils.hasScriptVar(target, "vertical.cell") || !utils.hasScriptVar(target, "vertical.min_height") || !utils.hasScriptVar(target, "vertical.max_height") || !utils.getStringScriptVar(target, "vertical.template").equals(bldgstr) || !utils.getStringScriptVar(target, "vertical.cell").equals(cellname)))
			{
				String[] template = dataTableGetStringColumn(DATATABLE_HEIGHT, "template");
				String[] cell = dataTableGetStringColumn(DATATABLE_HEIGHT, "cell");
				float[] max_height = dataTableGetFloatColumn(DATATABLE_HEIGHT, "max_height");
				float[] min_height = dataTableGetFloatColumn(DATATABLE_HEIGHT, "min_height");
				
				for (int i = 0; i < template.length; i++)
				{
					testAbortScript();
					if ((template[i].equals(bldgstr)) && (cell[i].equals(cellname)))
					{
						utils.setScriptVar (target, "vertical.template", template[i]);
						utils.setScriptVar(target, "vertical.cell", cell[i]);
						utils.setScriptVar(target, "vertical.min_height", min_height[i]);
						utils.setScriptVar(target, "vertical.max_height", max_height[i]);
					}
				}
			}
			
			if (!utils.hasScriptVar(target, "vertical.min_height") || !utils.hasScriptVar(target, "vertical.max_height"))
			{
				sendSystemMessage(self, new string_id(STF, "error_move_item"));
				return SCRIPT_CONTINUE;
			}
			
			if (new_y < utils.getFloatScriptVar(target, "vertical.min_height") || new_y > utils.getFloatScriptVar(target, "vertical.max_height"))
			{
				sendSystemMessage(self, new string_id(STF, "not_valid_location"));
				return SCRIPT_CONTINUE;
			}
		}
		else if (target != intendedTarget)
		{
			location sourceLoc = getLocation(intendedTarget);
			move_loc = getLocation(target);
			
			if ((sourceLoc != null) && (move_loc != null) && (isIdValid(sourceLoc.cell)) && (isIdValid(move_loc.cell)))
			{
				if (sourceLoc.cell == move_loc.cell)
				{
					if (copyLocation)
					{
						move_loc.x = sourceLoc.x;
						move_loc.z = sourceLoc.z;
					}
					else if (copyHeight)
					{
						move_loc.y = sourceLoc.y;
					}
					else
					{
						return SCRIPT_CONTINUE;
					}
				}
				else
				{
					sendSystemMessage(self, new string_id(STF, "move_copy_objects_not_in_same_cell"));
					return SCRIPT_CONTINUE;
				}
			}
			else
			{
				sendSystemMessage(self, new string_id(STF, "move_copy_objects_not_in_same_cell"));
				return SCRIPT_CONTINUE;
			}
		}
		
		if (move_loc != null)
		{
			setLocation(target, move_loc);
			location new_loc = getLocation(target);
			LOG("LOG_CHANNEL", "new_loc ->"+ new_loc);
			
			dictionary parameters = new dictionary();
			
			parameters.put("newLoc", move_loc);
			
			messageTo(target, "furniture_moved", parameters, 1.0f, false);
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int DoRestoreItemDecorationLayout(obj_id self, obj_id item, obj_id cell, location loc, boolean restoreRotation, float qw, float qx, float qy, float qz) throws InterruptedException
	{
		if (!isIdValid(item) || !exists(item) || !isIdValid(cell) || !exists(cell) || (loc == null))
		{
			return SCRIPT_OVERRIDE;
		}
		
		if (hasScript(self, performance.MUSIC_HEARTBEAT_SCRIPT))
		{
			string_id message = new string_id("player_structure", "cant_move_while_entertaining");
			sendSystemMessage(self, message);
			return SCRIPT_OVERRIDE;
		}
		
		if (hasObjVar(item, "unmoveable"))
		{
			sendSystemMessage(self, SID_CANT_MOVE_OBJECT);
			return SCRIPT_OVERRIDE;
		}
		
		if (hasCondition(item, CONDITION_VENDOR))
		{
			sendSystemMessage(self, SID_CANT_VENDOR);
			return SCRIPT_OVERRIDE;
		}
		
		if (!isMoveCommandValid(self, item))
		{
			return SCRIPT_OVERRIDE;
		}
		
		session.logActivity(self, session.ACTIVITY_DECORATE);
		
		loc.cell = cell;
		setLocation(item, loc);
		
		if (restoreRotation)
		{
			setQuaternion(item, qw, qx, qy, qz);
		}
		
		dictionary parameters = new dictionary();
		parameters.put("newLoc", loc);
		messageTo(item, "furniture_moved", parameters, 1.0f, false);
		
		if (restoreRotation)
		{
			messageTo(item, "furniture_rotated", null, 1.0f, false);
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int DoRestoreItemDecorationLayoutRotationOnly(obj_id self, obj_id item, float qw, float qx, float qy, float qz) throws InterruptedException
	{
		if (!isIdValid(item) || !exists(item))
		{
			return SCRIPT_OVERRIDE;
		}
		
		if (hasScript(self, performance.MUSIC_HEARTBEAT_SCRIPT))
		{
			string_id message = new string_id("player_structure", "cant_move_while_entertaining");
			sendSystemMessage(self, message);
			return SCRIPT_OVERRIDE;
		}
		
		if (!isMoveCommandValid(self, item))
		{
			return SCRIPT_OVERRIDE;
		}
		
		session.logActivity(self, session.ACTIVITY_DECORATE);
		
		setQuaternion(item, qw, qx, qy, qz);
		
		messageTo(item, "furniture_rotated", null, 1.0f, false);
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int nameStructure(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		
		obj_id structure = player_structure.getStructure(self);
		if (!isIdValid(structure))
		{
			LOG("LOG_CHANNEL", self + " ->You must in a building or near an installation to use that command.");
			sendSystemMessage(self, new string_id(STF, "command_no_building"));
			return SCRIPT_CONTINUE;
		}
		
		String template = getTemplateName(structure);
		
		if (player_structure.isCivic(structure))
		{
			
			if (!(template.indexOf("cloning_") > -1) && !(template.indexOf("garden_") > -1))
			{
				return SCRIPT_CONTINUE;
			}
		}
		
		if (!player_structure.isInAdminRange(structure, self))
		{
			return SCRIPT_CONTINUE;
		}
		
		if (!player_structure.isOwner(structure, self))
		{
			LOG("LOG_CHANNEL", "You must be the owner to rename a structure.");
			sendSystemMessage(self, new string_id(STF, "rename_must_be_owner"));
			return SCRIPT_CONTINUE;
		}
		
		if (hasObjVar(structure, hq.VAR_HQ_BASE))
		{
			sendSystemMessage(self, new string_id(STF, "no_rename_hq"));
			return SCRIPT_CONTINUE;
		}
		
		obj_id nameTarget = structure;
		
		if (player_structure.isBuilding(structure))
		{
			obj_id sign = getObjIdObjVar(structure, player_structure.VAR_SIGN_ID);
			if (isIdValid(sign))
			{
				nameTarget = sign;
			}
		}
		
		int i = 0;
		int textlen = params.length();
		while (textlen > i && '@' == params.charAt(i))
		{
			testAbortScript();
			++i;
		}
		params = params.substring(i, textlen);
		
		if (params == null || params.length() < 1)
		{
			String title = utils.packStringId (SID_SET_NAME_TITLE);
			String prompt = utils.packStringId (SID_SET_NAME_PROMPT);
			
			sui.filteredInputbox(self, self, prompt, title, "msgNameStructure", player_structure.getStructureName(structure));
			return SCRIPT_CONTINUE;
		}
		
		if (!isAppropriateText(params))
		{
			LOG("LOG_CHANNEL", self + " ->That is not a valid name.");
			sendSystemMessage(self, new string_id(STF, "not_valid_name"));
			return SCRIPT_CONTINUE;
		}
		
		String abandonedText = "";
		
		if (player_structure.isAbandoned(structure) || player_structure.isPreAbandoned(structure))
		{
			
			if (params.indexOf(player_structure.ABANDONED_TEXT) == -1)
			{
				if (params.length() > 100)
				{
					params = params.substring(0, 99);
				}
				
				abandonedText = player_structure.ABANDONED_TEXT;
			}
		}
		
		setName(nameTarget, params + abandonedText);
		
		LOG("LOG_CHANNEL", nameTarget + " ->Structure renamed to '"+ params + abandonedText + "'.");
		
		if (nameTarget == structure)
		{
			sendSystemMessage(self, new string_id(STF, "structure_renamed"));
		}
		else
		{
			setObjVar(structure, player_structure.VAR_SIGN_NAME, params);
			prose_package ppSignNamed = prose.getPackage(PROSE_SIGN_NAME_UPDATED, params);
			sendSystemMessageProse(self, ppSignNamed);
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int paWithdraw(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		
		obj_id structure = player_structure.getStructure(self);
		if (utils.isFreeTrial(self))
		{
			return SCRIPT_CONTINUE;
		}
		
		if (!player_structure.isGuildHall(structure))
		{
			LOG("LOG_CHANNEL", self + " ->You can only do that in a guild hall.");
			sendSystemMessage(self, new string_id(STF, "only_in_guildhall"));
			return SCRIPT_CONTINUE;
		}
		
		if (!player_structure.isInAdminRange(structure, self))
		{
			return SCRIPT_CONTINUE;
		}
		
		if (!player_structure.isAdmin(structure, self))
		{
			LOG("LOG_CHANNEL", self + " ->You must be an administrator to remove credits from the treasury.");
			sendSystemMessage(self, new string_id(STF, "withdraw_admin_only"));
			return SCRIPT_CONTINUE;
		}
		
		java.util.StringTokenizer st = new java.util.StringTokenizer(params);
		if (st.countTokens() == 0)
		{
			
			int pid = sui.inputbox(self, self, utils.packStringId(SID_SUI_AMOUNT_TO_WITHDRAW), "msgWithdrawMaintenance");
			
			return SCRIPT_CONTINUE;
		}
		
		String amt_str = st.nextToken();
		int amt = utils.stringToInt(amt_str);
		if (amt < 1)
		{
			LOG("LOG_CHANNEL", self + " ->The amount must be greater than zero.");
			sendSystemMessage(self, new string_id(STF, "amt_greater_than_zero"));
			return SCRIPT_CONTINUE;
		}
		
		if (!money.hasFunds(structure, money.MT_TOTAL, amt))
		{
			LOG("LOG_CHANNEL", self + " ->Insufficent funds for withdrawal.");
			sendSystemMessage(self, new string_id(STF, "insufficient_funds_withdrawal"));
		}
		else
		{
			player_structure.withdrawMaintenance(self, structure, amt);
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int itemRotateRight(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		int rotateDegrees = getFurnitureRotationDegree(self);
		queueCommand(self, (-1460986552), target, "yaw -"+ rotateDegrees + " "+ target, COMMAND_PRIORITY_DEFAULT);
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int itemRotateLeft(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		int rotateDegrees = getFurnitureRotationDegree(self);
		queueCommand(self, (-1460986552), target, "yaw "+ rotateDegrees + " "+ target, COMMAND_PRIORITY_DEFAULT);
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int itemRotateForward(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (player_structure.canRotateFurnitureInPitchRollAxes(self) || isGod(self))
		{
			int rotateDegrees = getFurnitureRotationDegree(self);
			queueCommand(self, (-1460986552), target, "pitch "+ rotateDegrees + " "+ target, COMMAND_PRIORITY_DEFAULT);
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int itemRotateBackward(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (player_structure.canRotateFurnitureInPitchRollAxes(self) || isGod(self))
		{
			int rotateDegrees = getFurnitureRotationDegree(self);
			queueCommand(self, (-1460986552), target, "pitch -"+ rotateDegrees + " "+ target, COMMAND_PRIORITY_DEFAULT);
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int itemRotateClockwise(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (player_structure.canRotateFurnitureInPitchRollAxes(self) || isGod(self))
		{
			int rotateDegrees = getFurnitureRotationDegree(self);
			queueCommand(self, (-1460986552), target, "roll "+ rotateDegrees + " "+ target, COMMAND_PRIORITY_DEFAULT);
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int itemRotateCounterclockwise(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (player_structure.canRotateFurnitureInPitchRollAxes(self) || isGod(self))
		{
			int rotateDegrees = getFurnitureRotationDegree(self);
			queueCommand(self, (-1460986552), target, "roll -"+ rotateDegrees + " "+ target, COMMAND_PRIORITY_DEFAULT);
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int itemRotateRandom(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (player_structure.canRotateFurnitureInPitchRollAxes(self) || isGod(self))
		{
			queueCommand(self, (-1460986552), target, "random "+target, COMMAND_PRIORITY_DEFAULT);
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int itemRotateRandomYaw(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		queueCommand(self, (-1460986552), target, "yaw random "+target, COMMAND_PRIORITY_DEFAULT);
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int itemRotateRandomPitch(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (player_structure.canRotateFurnitureInPitchRollAxes(self) || isGod(self))
		{
			queueCommand(self, (-1460986552), target, "pitch random "+target, COMMAND_PRIORITY_DEFAULT);
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int itemRotateRandomRoll(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (player_structure.canRotateFurnitureInPitchRollAxes(self) || isGod(self))
		{
			queueCommand(self, (-1460986552), target, "roll random "+target, COMMAND_PRIORITY_DEFAULT);
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int itemRotateReset(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		queueCommand(self, (-1460986552), target, "reset "+target, COMMAND_PRIORITY_DEFAULT);
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int itemRotateCopy(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		queueCommand(self, (-1460986552), target, "copy "+target, COMMAND_PRIORITY_DEFAULT);
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int itemMoveForward(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		queueCommand(self, (1265935363), target, "forward 10 "+target, COMMAND_PRIORITY_DEFAULT);
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int itemMoveBack(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		queueCommand(self, (1265935363), target, "back 10 "+target, COMMAND_PRIORITY_DEFAULT);
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int itemMoveLeft(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		queueCommand(self, (1265935363), target, "left 10 "+target, COMMAND_PRIORITY_DEFAULT);
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int itemMoveRight(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		queueCommand(self, (1265935363), target, "right 10 "+target, COMMAND_PRIORITY_DEFAULT);
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int itemMoveUp(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		queueCommand(self, (1265935363), target, "up 1 "+target, COMMAND_PRIORITY_DEFAULT);
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int itemMoveDown(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		queueCommand(self, (1265935363), target, "down 1 "+target, COMMAND_PRIORITY_DEFAULT);
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int itemMoveCopyLocation(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		queueCommand(self, (1265935363), target, "copy location "+target, COMMAND_PRIORITY_DEFAULT);
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int itemMoveCopyHeight(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		queueCommand(self, (1265935363), target, "copy height "+target, COMMAND_PRIORITY_DEFAULT);
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int cmdAddPower(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		LOG("LOG_CHANNEL", "player_building::cmdAddPower");
		if (utils.hasScriptVar(self, "addPower.pid"))
		{
			int oldpid = utils.getIntScriptVar(self, "addPower.pid");
			sui.closeSUI(self, oldpid);
			utils.removeScriptVarTree(self, "addPower");
		}
		
		obj_id structure = player_structure.getStructure(self);
		if (!player_structure.isOwner(structure, self))
		{
			if (utils.isFreeTrial(self))
			{
				sendSystemMessage(self, SID_TRIAL_STRUCTURE);
				return SCRIPT_CONTINUE;
			}
		}
		
		if (!isIdValid(structure))
		{
			LOG("LOG_CHANNEL", "You must be in a building, be near an installation, or have one targeted to do that.");
			sendSystemMessage(self, new string_id(STF, "no_building"));
			return SCRIPT_CONTINUE;
		}
		
		if (!player_structure.isAdmin(structure, self))
		{
			LOG("LOG_CHANNEL", "You must be a building admin to do that.");
			sendSystemMessage(self, new string_id(STF, "must_be_admin"));
			return SCRIPT_CONTINUE;
		}
		
		if (!player_structure.isInAdminRange(structure, self))
		{
			return SCRIPT_CONTINUE;
		}
		
		if (player_structure.isCivic(structure))
		{
			return SCRIPT_CONTINUE;
		}
		
		if (getPowerRate(structure) <= 0)
		{
			sendSystemMessage(self, new string_id(STF, "no_power_needed"));
			return SCRIPT_CONTINUE;
		}
		
		int amt = 0;
		if (params == null || params.equals(""))
		{
			
			int totalEnergy = resource.getEnergyTotalOnTarget(self);
			int maint_pool = getBankBalance(structure);
			int pid = sui.transfer(self, self, getString(new string_id (STF, "select_power_amount")) + "\n\n"+ getString(new string_id (STF, "current_power_value")) + (int)getPowerValue(structure), "@player_structure:add_power", "@player_structure:total_energy", totalEnergy, "@player_structure:to_deposit", 0, "msgAddPower");
			if (pid > -1)
			{
				utils.setScriptVar(self, "addPower.pid", pid);
				utils.setScriptVar(self, "addPower.target", structure);
			}
			return SCRIPT_CONTINUE;
		}
		else
		{
			amt = utils.stringToInt(params);
			if (amt < 1)
			{
				sendSystemMessage(self, SID_UNABLE_TO_PARSE);
				sendSystemMessage(self, SID_VALID_AMOUNT);
				return SCRIPT_CONTINUE;
			}
		}
		
		int inInv = 0;
		dictionary d = new dictionary();
		
		obj_id[] contents = utils.getFilteredPlayerContents(self);
		if ((contents == null) || (contents.length == 0))
		{
			return SCRIPT_CONTINUE;
		}
		
		for (int n = 0; n < contents.length; n++)
		{
			testAbortScript();
			int got = getGameObjectType(contents[n]);
			if (isGameObjectTypeOf(got, GOT_resource_container))
			{
				obj_id crate = contents[n];
				if (isIdValid(crate))
				{
					obj_id rId = getResourceContainerResourceType(crate);
					if (isIdValid(rId))
					{
						if (isResourceDerivedFrom(rId, "energy") || isResourceDerivedFrom(rId, "radioactive"))
						{
							
							int inCrate = resource.getPotentialEnergyValue(crate);
							inInv += inCrate;
							d.put(crate.toString(), inCrate);
						}
					}
				}
			}
		}
		
		if (inInv < 1)
		{
			return SCRIPT_CONTINUE;
		}
		
		if (amt > inInv)
		{
			
			sendSystemMessage(self, SID_NOT_ENOUGH_ENERGY);
			sendSystemMessage(self, SID_POWER_DEPOSIT_INCOMPLETE);
			return SCRIPT_CONTINUE;
		}
		
		int toTransfer = amt;
		
		java.util.Enumeration keys = d.keys();
		while (keys.hasMoreElements())
		{
			testAbortScript();
			String key = (String)(keys.nextElement());
			obj_id crate = utils.stringToObjId(key);
			if (isIdValid(crate))
			{
				int inCrate = d.getInt(key);
				if (toTransfer < inCrate)
				{
					obj_id rId = getResourceContainerResourceType(crate);
					if (isIdValid(rId))
					{
						int toRemove = (int)(Math.ceil(toTransfer/resource.getEnergyPowerRatio(rId)));
						removeResourceFromContainer(crate, rId, toRemove);
						toTransfer -= toTransfer;
					}
				}
				else
				{
					destroyObject(crate);
					toTransfer -= inCrate;
				}
			}
			
			if (toTransfer < 1)
			{
				break;
			}
		}
		
		if (toTransfer > 0)
		{
			sendSystemMessage(self, SID_ERROR_POWER_DEPOSIT);
			return SCRIPT_CONTINUE;
		}
		
		float cur = player_structure.powerInstallation(structure, amt);
		if (cur > -1f)
		{
			sendSystemMessageProse(self, prose.getPackage(new string_id (STF, "deposit_successful"), amt));
			sendSystemMessageProse(self, prose.getPackage(new string_id (STF, "reserve_report"), (int)cur));
		}
		else
		{
			sendSystemMessage(self, SID_POWER_DEPOSIT_FAIL);
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int assignDroid(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		obj_id structure = player_structure.getStructure(self);
		
		if (!player_structure.isOwner(structure, self))
		{
			if (utils.isFreeTrial(self))
			{
				sendSystemMessage(self, SID_TRIAL_STRUCTURE);
				return SCRIPT_CONTINUE;
			}
		}
		
		if (!isIdValid(structure))
		{
			LOG("LOG_CHANNEL", self + " ->You must in a building or near an installation to use that command.");
			sendSystemMessage(self, new string_id(STF, "command_no_building"));
			return SCRIPT_CONTINUE;
		}
		
		if (player_structure.isCivic(structure))
		{
			return SCRIPT_CONTINUE;
		}
		
		if (!player_structure.isInAdminRange(structure, self))
		{
			return SCRIPT_CONTINUE;
		}
		
		if (!player_structure.isAdmin(structure, self))
		{
			LOG("LOG_CHANNEL", "You must be a building admin to do that.");
			sendSystemMessage(self, new string_id(STF, "must_be_admin"));
			return SCRIPT_CONTINUE;
		}
		
		obj_id[] droid_list = player_structure.getMaintenanceDroids(self);
		
		if (droid_list.length <= 0)
		{
			LOG("LOG_CHANNEL", "You do not have any maintenance capable droids.");
			sendSystemMessage(self, new string_id(STF, "no_droids"));
			return SCRIPT_CONTINUE;
		}
		
		String[] list = new String[droid_list.length];
		for (int i = 0; i < list.length; i++)
		{
			testAbortScript();
			list[i] = getEncodedName(droid_list[i]);
		}
		
		String title = utils.packStringId (SID_ASSIGN_DROID_TITLE);
		String prompt = utils.packStringId (SID_ASSIGN_DROID_PROMPT);
		
		sui.listbox(self, self, prompt, sui.OK_CANCEL, title, list, "msgAssignDroid");
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int cmdDepositPowerIncubator(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		obj_id player = self;
		obj_id station = target;
		int amt = 0;
		
		if (!incubator.validateActiveUser(station, player))
		{
			return SCRIPT_CONTINUE;
		}
		
		if (utils.hasScriptVar(player, "depositPowerIncubator.pid"))
		{
			int oldpid = utils.getIntScriptVar(player, "depositPowerIncubator.pid");
			sui.closeSUI(player, oldpid);
			utils.removeScriptVarTree(player, "depositPowerIncubator");
		}
		
		if (params == null || params.equals(""))
		{
			dictionary energyInfo = resource.getGeoThermalEnergyInformationOnTarget(player);
			int totalEnergy = energyInfo.getInt("cnt");
			int resourceQuality = energyInfo.getInt("resourceQuality");
			String resourceName = energyInfo.getString("resourceName");
			
			if (resourceName == null || resourceName.equals(""))
			{
				return SCRIPT_CONTINUE;
			}
			
			if (totalEnergy > 0)
			{
				
				if (totalEnergy >= incubator.RESOURCE_POWER_AMOUNT_CAP)
				{
					totalEnergy = incubator.RESOURCE_POWER_AMOUNT_CAP;
				}
				
				if (incubator.hasPower(station))
				{
					int amountInStation = incubator.getStationPowerAmount(station);
					int totalEnergyCap = incubator.RESOURCE_POWER_AMOUNT_CAP - amountInStation;
					
					if (totalEnergy >= totalEnergyCap)
					{
						totalEnergy = totalEnergyCap;
					}
				}
				
				if (!incubator.addPowerPreCheck(station, player, totalEnergy, resourceName))
				{
					return SCRIPT_CONTINUE;
				}
				
				int pid = sui.transfer(player, player, getString(SID_SELECT_POWER_AMOUNT) + "\n"+ getString(SID_CURRENT_POWER_AMOUNT) + incubator.getStationPowerAmount(station) +"\n"+ getString(SID_POWER_NAME) + resourceName +"\n"+ getString(SID_POWER_QUALITY) + resourceQuality , "@player_structure:add_power", "@player_structure:total_energy", totalEnergy, "@player_structure:to_deposit", 0, "msgDepositPowerIncubator");
				if (pid > -1)
				{
					utils.setScriptVar(self, "depositPowerIncubator.pid", pid);
					utils.setScriptVar(self, "depositPowerIncubator.target", station);
					
					utils.setScriptVar(self, "powerIncubator.resourceName", resourceName);
				}
			}
			else
			{
				
				sendSystemMessage(player, SID_NO_GEO_POWER);
			}
			return SCRIPT_CONTINUE;
		}
		else
		{
			amt = utils.stringToInt(params);
			if (amt < 1)
			{
				
				sendSystemMessage(player, SID_UNABLE_TO_PARSE);
				sendSystemMessage(player, SID_VALID_AMOUNT);
				return SCRIPT_CONTINUE;
			}
		}
		
		int inInventory = 0;
		int resourceQuality = 0;
		dictionary resourceInfo = new dictionary();
		obj_id[] contents = utils.getFilteredPlayerContents(player);
		Vector tempList = new Vector();
		tempList.setSize(0);
		String resourceName = utils.getStringScriptVar(self, "powerIncubator.resourceName");
		
		if (resourceName != null && !resourceName.equals(""))
		{
			tempList.addElement(resourceName);
		}
		
		if ((contents == null) || (contents.length == 0) || (tempList.size() == 0))
		{
			return SCRIPT_CONTINUE;
		}
		
		for (int n = 0; n < contents.length; n++)
		{
			testAbortScript();
			int got = getGameObjectType(contents[n]);
			if (isGameObjectTypeOf(got, GOT_resource_container))
			{
				obj_id resourceCrate = contents[n];
				if (isIdValid(resourceCrate))
				{
					obj_id resourceId = getResourceContainerResourceType(resourceCrate);
					if (isIdValid(resourceId))
					{
						if (isResourceDerivedFrom(resourceId, "energy_renewable_site_limited_geothermal"))
						{
							String tempResourceName = getResourceName(resourceId);
							
							if (tempList.contains(tempResourceName))
							{
								resourceQuality = getResourceAttribute(resourceId, "res_quality");
								int inCrate = getResourceContainerQuantity(resourceCrate);
								inInventory += inCrate;
								resourceInfo.put(resourceCrate.toString(), inCrate);
							}
						}
					}
				}
			}
		}
		
		utils.removeScriptVarTree(player, "powerIncubator");
		
		if (inInventory < 1)
		{
			return SCRIPT_CONTINUE;
		}
		
		if (amt > inInventory)
		{
			
			sendSystemMessage(player, SID_NOT_ENOUGH_ENERGY);
			sendSystemMessage(player, SID_POWER_DEPOSIT_INCOMPLETE);
			return SCRIPT_CONTINUE;
		}
		
		if (!incubator.addPowerPreCheck(station, player, amt, resourceName))
		{
			return SCRIPT_CONTINUE;
		}
		
		int toTransfer = amt;
		java.util.Enumeration keys = resourceInfo.keys();
		while (keys.hasMoreElements())
		{
			testAbortScript();
			String key = (String)(keys.nextElement());
			obj_id resourceCrate = utils.stringToObjId(key);
			if (isIdValid(resourceCrate))
			{
				int inCrate = resourceInfo.getInt(key);
				if (toTransfer < inCrate)
				{
					obj_id resourceId = getResourceContainerResourceType(resourceCrate);
					if (isIdValid(resourceId))
					{
						int toRemove = toTransfer;
						removeResourceFromContainer(resourceCrate, resourceId, toRemove);
						toTransfer -= toTransfer;
					}
				}
				else
				{
					destroyObject(resourceCrate);
					toTransfer -= inCrate;
				}
			}
			
			if (toTransfer < 1)
			{
				break;
			}
		}
		
		if (toTransfer > 0)
		{
			
			sendSystemMessage(player, SID_ERROR_POWER_DEPOSIT);
			return SCRIPT_CONTINUE;
		}
		
		if (incubator.addPowerIncubator(station, player, amt, resourceQuality , resourceName))
		{
			
			sendSystemMessageProse(player, prose.getPackage((SID_DEPOSIT_SUCCESS), amt));
			
			sendSystemMessageProse(player, prose.getPackage((SID_RESERVE_REPORT), incubator.getStationPowerAmount(station)));
		}
		else
		{
			
			sendSystemMessage(player, SID_POWER_DEPOSIT_FAIL);
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int cmdRemovePowerIncubator(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		obj_id player = self;
		obj_id station = target;
		
		if (!incubator.validateActiveUser(station, player))
		{
			sendSystemMessage(player, incubator.SID_NOT_YOUR_INCUBATOR);
			return SCRIPT_CONTINUE;
		}
		
		if (incubator.removeAllPowerIncubator(station, player))
		{
			sendSystemMessage(player, SID_POWER_REMOVED);
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int placeStructureMode(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		player_structure.tryEnterPlacementMode(target, self);
		return SCRIPT_CONTINUE;
	}
	
	
	public int failPlaceStructureMode(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		LOG("LOG_CHANNEL", "player_building::failPlaceStructureMode");
		
		sendSystemMessage(self, SID_WRONG_STATE);
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int setPermission(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		LOG("LOG_CHANNEL", "player_building::setPermission-- params ->"+ params + " target ->"+ target);
		
		obj_id structure = player_structure.getStructure(self);
		
		if (utils.isFreeTrial(self))
		{
			sendSystemMessage(self, SID_TRIAL_NO_MODIFY);
			return SCRIPT_CONTINUE;
		}
		
		LOG("LOG_CHANNEL", "structure ->"+ structure);
		if (!isIdValid(structure))
		{
			LOG("LOG_CHANNEL", "You must be in a building, be near an installation, or have one targeted to do that.");
			sendSystemMessage(self, new string_id(STF, "no_building"));
			return SCRIPT_CONTINUE;
		}
		
		if (hasObjVar(structure, hq.VAR_HQ_BASE))
		{
			return SCRIPT_CONTINUE;
		}
		
		String template = getTemplateName(structure);
		if (player_structure.isCivic(structure))
		{
			if (!(template.indexOf("cityhall_") > -1))
			{
				return SCRIPT_CONTINUE;
			}
		}
		
		if (player_structure.isHarvester(structure) || player_structure.isGenerator(structure))
		{
			return SCRIPT_CONTINUE;
		}
		
		if (!player_structure.isAdmin(structure, self))
		{
			LOG("LOG_CHANNEL", "You must be a building admin to do that.");
			string_id strSpam = new string_id("player_structure", "not_admin");
			sendSystemMessage(self, strSpam);
			
			return SCRIPT_CONTINUE;
		}
		
		if (!player_structure.isInAdminRange(structure, self))
		{
			return SCRIPT_CONTINUE;
		}
		
		if (!isPlayer(target) && !isMob(target))
		{
			
			target = obj_id.NULL_ID;
		}
		
		java.util.StringTokenizer st = new java.util.StringTokenizer(params);
		if (st.countTokens() == 0)
		{
			LOG("LOG_CHANNEL", "Format: /setPermission <type> <player>");
			sendSystemMessage(self, new string_id(STF, "format_setpermission_type_player"));
			return SCRIPT_CONTINUE;
		}
		
		String perm_type = st.nextToken().toUpperCase();
		
		String target_str = "";
		if (st.hasMoreTokens())
		{
			
			target_str = st.nextToken();
			if (target_str.length() > 40)
			{
				sendSystemMessage(self, new string_id(STF, "permission_40_char"));
				return SCRIPT_CONTINUE;
			}
		}
		
		if ((target == null)||(target == obj_id.NULL_ID))
		{
			
			if (target_str.equals(""))
			{
				target = getLookAtTarget(self);
				
				if (!isPlayer(target) && !isMob(target))
				{
					target = obj_id.NULL_ID;
				}
			}
		}
		
		LOG("LOG_CHANNEL", "perm_type ->"+ perm_type + " target ->"+ target + " target_str ->"+ target_str);
		
		int perm_switch = -1;
		
		if (perm_type.equals("ENTRY"))
		{
			perm_switch = 1;
		}
		if (perm_type.equals("BAN"))
		{
			perm_switch = 2;
		}
		if (perm_type.equals("ADMIN"))
		{
			perm_switch = 3;
		}
		
		if (perm_type.equals("HOPPER"))
		{
			perm_switch = 5;
		}
		
		if (player_structure.isBuilding(structure))
		{
			if (perm_switch == 5)
			{
				LOG("LOG_CHANNEL", "Buildings do not have that permission list.");
				sendSystemMessage(self, new string_id(STF, "building_no_permission_list"));
				return SCRIPT_CONTINUE;
			}
		}
		
		if (player_structure.isInstallation(structure))
		{
			if (perm_switch == 1 || perm_switch == 2 || perm_switch == 4)
			{
				LOG("LOG_CHANNEL", "Installations do not have that permission list.");
				sendSystemMessage(self, new string_id(STF, "installations_no_permission_list"));
				return SCRIPT_CONTINUE;
			}
		}
		
		if (isGameObjectTypeOf(getGameObjectType(structure), GOT_ship))
		{
			
			if (perm_switch != 3)
			{
				sendSystemMessage(self, new string_id(STF, "ships_no_permission_list"));
				return SCRIPT_CONTINUE;
			}
		}
		
		if ((perm_switch == 3) && (getAccountNumLots(getPlayerObject(self)) > player_structure.MAX_LOTS))
		{
			obj_id lotOverlimitStructure = getObjIdObjVar(self, "lotOverlimit.structure_id");
			if (isIdValid(lotOverlimitStructure) && (lotOverlimitStructure == structure))
			{
				sendSystemMessage(self, new string_id(STF, "lot_overlimit_cannot_edit_admin_list"));
				return SCRIPT_CONTINUE;
			}
		}
		
		if (((target == null) || (target == obj_id.NULL_ID)) && (target_str.equals("")))
		{
			String[] dsrc;
			String title;
			String handler;
			switch (perm_switch)
			{
				case 1:
				if (isGod(self))
				{
					dsrc = player_structure.getCompleteEntryList(structure);
				}
				else
				{
					dsrc = player_structure.getEntryList(structure);
				}
				title = getString(new string_id(STF, "entry_permissions_list"));
				handler = "msgEnterPermissions";
				break;
				case 2:
				dsrc = player_structure.getBanList(structure);
				title = getString(new string_id(STF, "ban_list"));
				handler = "msgBanPermissions";
				break;
				case 3:
				dsrc = player_structure.getAdminListNames(structure);
				title = getString(new string_id(STF, "admin_permissions_list"));
				handler = "msgAdminPermissions";
				break;
				case 4:
				dsrc = player_structure.getVendorList(structure);
				title = getString(new string_id(STF, "vendor_permissions_list"));
				handler = "msgVendorPermissions";
				break;
				case 5:
				dsrc = player_structure.getHopperListNames(structure);
				title = getString(new string_id(STF, "hopper_permissions_list"));
				handler = "msgHopperPermissions";
				break;
				default:
				LOG("LOG_CHANNEL", "You must specify a valid permission list (Entry, Ban, Admin, Hopper)");
				sendSystemMessage(self, new string_id(STF, "must_specify_list"));
				return SCRIPT_CONTINUE;
			}
			if (dsrc == null)
			{
				
				dsrc = new String[0];
			}
			
			obj_id[] players;
			if (player_structure.isBuilding(structure))
			{
				
				players = getPlayerCreaturesInRange(structure, 30.0f);
			}
			else
			{
				players = player_structure.getPlayersNearInstallation(structure);
			}
			
			Vector player_names = new Vector();
			player_names.setSize(0);
			if (players != null)
			{
				for (int i = 0; i < players.length; i++)
				{
					testAbortScript();
					String player_name = getFirstName(players[i]);
					LOG("LOG_CHANNEL", "Found "+ player_name);
					int idx = utils.getElementPositionInArray(dsrc, player_name);
					if (idx == -1)
					{
						LOG("LOG_CHANNEL", " ...Adding "+ player_name);
						player_names = utils.addElement(player_names, player_name);
					}
				}
			}
			
			LOG("LOG_CHANNEL", "players #->"+ player_names.size() + " self ->"+ self);
			String[] player_names_array = new String[0];
			if (player_names != null)
			{
				player_names_array = new String[player_names.size()];
				player_names.toArray(player_names_array);
			}
			displayStructurePermissionData(self, dsrc, player_names_array, perm_type);
			
		}
		else
		{
			
			if (target_str.equals(""))
			{
				
				switch (perm_switch)
				{
					case 1:
					player_structure.modifyEntryList(structure, target, self);
					break;
					case 2:
					player_structure.modifyBanList(structure, target, self);
					break;
					case 3:
					player_structure.modifyAdminList(structure, target, self);
					break;
					case 4:
					player_structure.modifyVendorList(structure, target, self);
					break;
					case 5:
					player_structure.modifyHopperList(structure, target, self);
					break;
					default:
					LOG("LOG_CHANNEL", "You must specify a valid permission list (Entry, Ban, Admin, Hopper)");
					sendSystemMessage(self, new string_id(STF, "must_specify_list"));
					return SCRIPT_CONTINUE;
				}
			}
			else
			{
				
				switch (perm_switch)
				{
					case 1:
					player_structure.modifyEntryList(structure, target_str, self);
					break;
					case 2:
					player_structure.modifyBanList(structure, target_str, self);
					break;
					case 3:
					player_structure.modifyAdminList(structure, target_str, self);
					break;
					case 4:
					player_structure.modifyVendorList(structure, target_str, self);
					break;
					case 5:
					player_structure.modifyHopperList(structure, target_str, self);
					break;
					default:
					LOG("LOG_CHANNEL", "You must specify a valid permission list (Entry, Ban, Admin, Hopper)");
					sendSystemMessage(self, new string_id(STF, "must_specify_list"));
					return SCRIPT_CONTINUE;
				}
			}
			
		}
		return SCRIPT_CONTINUE;
	}
	
	
	public int structureStatus(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		LOG("house", "commandHandler:structureStatus");
		if (utils.hasScriptVar(self, "player_structure.status.pid"))
		{
			int oldpid = utils.getIntScriptVar(self, "player_structure.status.pid");
			sui.closeSUI(self, oldpid);
			utils.removeScriptVarTree(self, "player_structure.status");
		}
		
		LOG("LOG_CHANNEL", "player_building::structureStatus");
		obj_id structure = null;
		if (isIdValid(target))
		{
			if (player_structure.isBuilding(target) || player_structure.isInstallation(target))
			{
				structure = target;
			}
			else
			{
				structure = player_structure.getStructure(target);
			}
		}
		if (structure == null)
		{
			structure = player_structure.getStructure(self);
		}
		
		LOG("house", "structureStatus - structure = "+ structure);
		if (!isIdValid(structure))
		{
			LOG("LOG_CHANNEL", "You must be in a building, be near an installation, or have one targeted to do that.");
			sendSystemMessage(self, new string_id(STF, "no_building"));
			return SCRIPT_CONTINUE;
		}
		
		String buildingTemplateName = getTemplateName(structure);
		
		int sGot = getGameObjectType(structure);
		if (!player_structure.isAdmin(structure, self))
		{
			boolean allowContinue = false;
			
			if (sGot == GOT_building_factional || sGot == GOT_installation_turret || sGot == GOT_installation_minefield)
			{
				int pFac = pvpGetAlignedFaction(self);
				int sFac = pvpGetAlignedFaction(structure);
				if (pFac == sFac)
				{
					allowContinue = true;
				}
			}
			
			if (!allowContinue)
			{
				LOG("LOG_CHANNEL", "You must be a building admin to do that.");
				sendSystemMessage(self, new string_id(STF, "must_be_admin"));
				return SCRIPT_CONTINUE;
			}
		}
		
		if (!player_structure.isInAdminRange(structure, self))
		{
			return SCRIPT_CONTINUE;
		}
		
		Vector dsrc = new Vector();
		dsrc.setSize(0);
		String owner_name = player_structure.getStructureOwner(structure);
		if (owner_name != null)
		{
			owner_name = toUpper(owner_name, 0);
			string_id owner_prompt_text = new string_id(STF, "owner_prompt");
			dsrc = utils.addElement(dsrc, getString(owner_prompt_text) + owner_name);
		}
		
		if (player_structure.isResidence(structure, self))
		{
			string_id residency_declaration = new string_id(STF, "declared_residency");
			dsrc = utils.addElement(dsrc, getString(residency_declaration));
		}
		
		int civic_var = 0;
		int m_rate = 0;
		int m_rate_base = 0;
		
		if (!player_structure.isCivic(structure))
		{
			
			if (permissionsIsPublic(structure))
			{
				string_id public_structure = new string_id(STF, "structure_public");
				dsrc = utils.addElement(dsrc, getString(public_structure));
			}
			else
			{
				string_id private_structure = new string_id(STF, "structure_private");
				dsrc = utils.addElement(dsrc, getString(private_structure));
			}
			
			int condition = player_structure.getStructureCondition(structure);
			int max_condition = player_structure.getMaxCondition(structure);
			
			int perc_condition = condition * 100 / max_condition;
			
			if (perc_condition == 100)
			{
				string_id condition_prompt_text = new string_id(STF, "condition_prompt");
				dsrc = utils.addElement(dsrc, getString(condition_prompt_text) + perc_condition + "%");
			}
			else
			{
				int maint_rate = player_structure.getBaseMaintenanceRate(structure);
				int total_cost = (int)((max_condition - condition) * maint_rate / 3.0f);
				string_id condition_prompt_text_damaged = new string_id(STF, "condition_prompt");
				string_id credits_to_repair_suffix = new string_id(STF, "credits_to_repair");
				
				if (buildingTemplateName.indexOf ("object/building/faction_perk/hq/") == -1)
				{
					dsrc = utils.addElement(dsrc, getString(condition_prompt_text_damaged) + perc_condition + "%"+ " ("+ total_cost + getString(credits_to_repair_suffix));
				}
				else
				{
					dsrc = utils.addElement(dsrc, getString(condition_prompt_text_damaged) + perc_condition + "%");
				}
			}
			
			int m_pool = player_structure.getMaintenancePool(structure);
			m_rate = player_structure.getMaintenanceRate(structure);
			
			m_rate_base = player_structure.getBaseMaintenanceRate(structure);
			
			double time_remaining = (double)m_pool * (double)player_structure.getMaintenanceHeartbeat() / (double)m_rate;
			if (time_remaining < 0 || time_remaining > Integer.MAX_VALUE)
			{
				time_remaining = Integer.MAX_VALUE;
			}
			
			if (m_pool > 0)
			{
				
				String time_str = utils.assembleTimeRemainToUse((int)time_remaining, false);
				
				if (player_structure.isGuildHall(structure))
				{
					string_id treasury_prompt_text = new string_id(STF, "treasury_prompt");
					dsrc = utils.addElement(dsrc, getString(treasury_prompt_text) + m_pool + " ("+ time_str + ")");
				}
				else
				{
					
					if (buildingTemplateName.indexOf ("object/building/faction_perk/hq/") == -1)
					{
						
						string_id maintenance_prompt_text = new string_id(STF, "maintenance_pool_prompt");
						dsrc = utils.addElement(dsrc, getString(maintenance_prompt_text) + m_pool + " ("+ time_str + ")");
					}
				}
			}
			else
			{
				if (player_structure.isGuildHall(structure))
				{
					string_id treasury_empty_text = new string_id(STF, "treasury_prompt");
					dsrc = utils.addElement(dsrc, getString(treasury_empty_text) + m_pool);
				}
				else
				{
					
					if (buildingTemplateName.indexOf ("object/building/faction_perk/hq/") == -1)
					{
						
						string_id maintenance_empty_text = new string_id(STF, "maintenance_pool_prompt");
						dsrc = utils.addElement(dsrc, getString(maintenance_empty_text) + m_pool);
					}
				}
			}
			
		}
		int hourly_m_rate = m_rate*(3600/player_structure.getMaintenanceHeartbeat());
		
		civic_var = getIntObjVar(structure, player_structure.VAR_CIVIC);
		if (civic_var == 1)
		{
			hourly_m_rate = 0;
		}
		
		if (buildingTemplateName.indexOf ("object/building/faction_perk/hq/") == -1)
		{
			
			if (m_rate == m_rate_base)
			{
				string_id maintenance_rate_text = new string_id(STF, "maintenance_rate_prompt");
				dsrc = utils.addElement(dsrc, getString(maintenance_rate_text) + Integer.toString(hourly_m_rate) + " cr/hr");
			}
			else
			{
				int hourly_m_rate_base = m_rate_base*(3600/player_structure.getMaintenanceHeartbeat());
				string_id maintenance_rate_mod_text = new string_id(STF, "maintenance_rate_prompt");
				dsrc = utils.addElement(dsrc, getString(maintenance_rate_mod_text) + Integer.toString(hourly_m_rate) + " cr/hr ("+hourly_m_rate_base+")");
			}
		}
		
		if (buildingTemplateName.indexOf ("object/building/faction_perk/hq/") == -1)
		{
			
			int property_tax = city.getPropertyTax(structure);
			if (property_tax > 0)
			{
				string_id maintenance_rate_mod_text = new string_id(STF, "property_tax_rate_prompt");
				dsrc = utils.addElement(dsrc, getString(maintenance_rate_mod_text) + property_tax + "%");
			}
		}
		
		if (player_structure.isHarvester(structure) || player_structure.isFactory(structure))
		{
			float p_pool = getPowerValue(structure);
			float p_rate = getPowerRate(structure);
			int p_time = Math.round(p_pool / p_rate * 3600f);
			
			if (p_pool > 0)
			{
				
				String p_time_string = utils.assembleTimeRemainToUse(p_time, false);
				
				string_id power_reserve_empty_text = new string_id(STF, "power_reserve_prompt");
				dsrc = utils.addElement(dsrc, getString(power_reserve_empty_text) + (int)p_pool + " ("+ p_time_string + ")");
			}
			else
			{
				string_id power_reserve_text = new string_id(STF, "power_reserve_prompt");
				dsrc = utils.addElement(dsrc, getString(power_reserve_text) + (int)p_pool);
			}
			string_id power_consumption_text = new string_id(STF, "power_consumption_prompt");
			string_id units_per_hour_text = new string_id(STF, "units_per_hour");
			dsrc = utils.addElement(dsrc, getString(power_consumption_text) + Integer.toString((int)p_rate) + getString(units_per_hour_text));
		}
		
		if (sGot == GOT_building_factional && hasObjVar(structure, hq.VAR_HQ_BASE))
		{
			if (player_structure.isAdmin(structure, self) || pvpGetCurrentGcwRank(self) >= 6)
			{
				int now = getGameTime();
				
				string_id resource_reserves_text = new string_id(STF, "resource_reserves_prompt");
				string_id units_suffix_text = new string_id(STF, "units_suffix");
				dsrc = utils.addElement(dsrc, getString(resource_reserves_text) + getIntObjVar(structure, hq.VAR_HQ_RESOURCE_CNT) + getString(units_suffix_text));
				
				if (hasObjVar(structure, player_structure.VAR_LAST_MAINTANENCE))
				{
					int stamp = getIntObjVar(structure, player_structure.VAR_LAST_MAINTANENCE);
					int maint_delta = stamp + player_structure.getMaintenanceHeartbeat() - now;
					if (maint_delta > 0)
					{
						int[] mTime = player_structure.convertSecondsTime(maint_delta);
						String m_time_string = player_structure.assembleTimeRemaining(mTime);
						string_id repair_cycle_text = new string_id(STF, "next_repair_cycle");
						dsrc = utils.addElement(dsrc, getString(repair_cycle_text) + m_time_string);
					}
				}
				
				if (hasObjVar(structure, hq.VAR_OBJECTIVE_STAMP))
				{
					long current = System.currentTimeMillis();
					long curTime = ((current / 1000) - 1072224000);
					
					int currentTime = (int) curTime;
					
					int stamp = getIntObjVar(structure, hq.VAR_OBJECTIVE_STAMP);
					int objective_delta = stamp + (int)(hq.VULNERABILITY_CYCLE) - currentTime;
					if (objective_delta > 0)
					{
						int[] oTime = player_structure.convertSecondsTime(objective_delta);
						String o_time_string = player_structure.assembleTimeRemaining(oTime);
						string_id vulnerability_text = new string_id(STF, "next_vulnerability_prompt");
						dsrc = utils.addElement(dsrc, getString(vulnerability_text) + o_time_string);
					}
				}
			}
		}
		
		if (player_structure.isBuilding(structure))
		{
			int numItems = player_structure.getStructureNumItems(structure);
			string_id building_items_text = new string_id(STF, "items_in_building_prompt");
			dsrc = utils.addElement(dsrc, getString(building_items_text) + numItems);
			
			int itemLimit = getPobBaseItemLimit(structure);
			
			if (hasObjVar(structure, "structureChange.storageIncrease"))
			{
				int storageIncrease = getIntObjVar(structure, "structureChange.storageIncrease");
				itemLimit += storageIncrease;
			}
			dsrc = utils.addElement(dsrc, "@player_structure:total_house_storage"+ " "+ itemLimit);
		}
		
		if (hasObjVar(structure, "structureChange.storageIncrease"))
		{
			int storageIncrease = getIntObjVar(structure, "structureChange.storageIncrease");
			string_id storage_increase = new string_id(STF, "structure_storage_increase");
			dsrc = utils.addElement(dsrc, getString(storage_increase) + storageIncrease);
		}
		
		String prompt = "";
		
		String structure_name = player_structure.getStructureName(structure);
		if (structure_name != null && !structure_name.equals(""))
		{
			string_id structure_name_text = new string_id(STF, "structure_name_prompt");
			prompt += getString(structure_name_text) + structure_name;
		}
		int maint_stamp = getIntObjVar(structure, player_structure.VAR_LAST_MAINTANENCE);
		int now = getGameTime();
		if (now < maint_stamp)
		{
			string_id free_maintenance_text = new string_id(STF, "free_maintenance_time");
			prompt += "\n\\#pcontrast1"+ getString(free_maintenance_text) + "\\#. ";
			
			int prepaid_delta = maint_stamp - now;
			int[] prepaidTime = player_structure.convertSecondsTime(prepaid_delta);
			String prepaidTimeString = player_structure.assembleTimeRemaining(prepaidTime);
			if (prepaidTimeString != null && !prepaidTimeString.equals(""))
			{
				prompt += prepaidTimeString;
			}
			else
			{
				string_id seconds_suffix_text = new string_id(STF, "seconds_suffix");
				prompt += Integer.toString(prepaid_delta) + getString(seconds_suffix_text);
			}
		}
		
		String maint_mods_string = "";
		if (civic_var == 1)
		{
			string_id civic_structure_alert_text = new string_id(STF, "civic_structure_alert");
			maint_mods_string += "\n- "+ getString(civic_structure_alert_text);
		}
		else
		{
			if (hasObjVar(structure, player_structure.VAR_MAINTENANCE_MOD_MERCHANT))
			{
				float merchant_mod = getFloatObjVar(structure, player_structure.VAR_MAINTENANCE_MOD_MERCHANT);
				string_id merch_mod_text = new string_id(STF, "merch_mod_prompt");
				maint_mods_string += "\n- "+ getString(merch_mod_text) +Integer.toString((int)(merchant_mod*100f))+"%";
			}
			
			int maint_property_tax = player_structure.getMaintenancePropertyTax(structure);
			if (maint_property_tax > 0)
			{
				int hourly_tax = Math.round(maint_property_tax * (3600f/player_structure.getMaintenanceHeartbeat()));
				string_id property_tax_rate_text = new string_id(STF, "property_tax_rate_prompt");
				maint_mods_string += "\n- "+ getString(property_tax_rate_text) +Integer.toString(hourly_tax)+"cr/hr";
			}
		}
		
		if (maint_mods_string != null && !maint_mods_string.equals(""))
		{
			string_id maintenance_mods_text = new string_id(STF, "maintenance_mods_prompt");
			prompt += "\n\n"+ getString(maintenance_mods_text) + maint_mods_string;
		}
		
		int pid = sui.listbox(self, self, prompt, sui.REFRESH_CANCEL, "@player_structure:structure_status_t", dsrc, "handleStatusUi");
		if (pid > -1)
		{
			utils.setScriptVar(self, "player_structure.status.pid", pid);
			utils.setScriptVar(self, "player_structure.status.target", structure);
			if (isIdValid(target))
			{
				LOG("house", "StructureStatus:: FROM SCD = true");
				utils.setScriptVar(self, "player_structure.status.fromScd", 1);
			}
		}
		return SCRIPT_CONTINUE;
	}
	
	
	public int payMaintenance(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		obj_id structure = null;
		if (isIdValid(target))
		{
			if (player_structure.isBuilding(target) || player_structure.isInstallation(target))
			{
				structure = target;
			}
			else
			{
				structure = player_structure.getStructure(target);
			}
		}
		if (structure == null)
		{
			structure = player_structure.getStructure(self);
		}
		
		if (!isIdValid(structure))
		{
			sendSystemMessage(self, new string_id(STF, "no_building"));
			return SCRIPT_CONTINUE;
		}
		LOG("house", "payMaintenance - structure = "+ structure);
		
		if (!player_structure.isOwner(structure, self))
		{
			if (utils.isFreeTrial(self))
			{
				sendSystemMessage(self, SID_TRIAL_STRUCTURE);
				return SCRIPT_CONTINUE;
			}
		}
		
		if (player_structure.isCivic(structure))
		{
			return SCRIPT_CONTINUE;
		}
		
		String buildingTemplate = getTemplateName(structure);
		if (buildingTemplate == null)
		{
			LOG("LOG_CHANNEL", "player_structure::payMaintenance -- Unable to find template for building.");
			return SCRIPT_CONTINUE;
		}
		
		if (player_structure.isFactionPerkBase(buildingTemplate))
		{
			return SCRIPT_CONTINUE;
		}
		
		if (utils.hasScriptVar(self, "payMaintenance.pid"))
		{
			int oldpid = utils.getIntScriptVar(self, "payMaintenance.pid");
			sui.closeSUI(self, oldpid);
			utils.removeScriptVarTree(self, "payMaintenance");
		}
		
		if (!player_structure.isAdmin(structure, self))
		{
			sendSystemMessage(self, new string_id(STF, "must_be_admin"));
			return SCRIPT_CONTINUE;
		}
		
		if (player_structure.isHarvester(structure) || player_structure.isGenerator(structure))
		{
			if (!player_structure.isOwner(structure, self))
			{
				sendSystemMessage(self, new string_id(STF, "must_be_owner"));
				return SCRIPT_CONTINUE;
			}
		}
		
		if (!player_structure.isInAdminRange(structure, self))
		{
			return SCRIPT_CONTINUE;
		}
		
		java.util.StringTokenizer st = new java.util.StringTokenizer(params);
		if (st.countTokens() == 0)
		{
			
			int total = getTotalMoney(self);
			if (total > 0)
			{
				int maint_pool = getBankBalance(structure);
				string_id select_amount = new string_id(STF, "select_maint_amount");
				String prompt = getString(select_amount) + "\n\n ";
				
				prompt += getString(new string_id(STF, "current_maint_pool")) + maint_pool + "cr.";
				
				int pid = sui.transfer(self, self, prompt, "@player_structure:select_amount", "@player_structure:total_funds", total, "@player_structure:to_pay", 0, "msgPayMaintenance");
				if (pid > -1)
				{
					utils.setScriptVar(self, "payMaintenance.pid", pid);
					utils.setScriptVar(self, "payMaintenance.target", structure);
				}
			}
			else
			{
				sendSystemMessage(self, new string_id(STF, "no_money"));
			}
			
			return SCRIPT_CONTINUE;
		}
		
		String amt_str = st.nextToken();
		int amt = utils.stringToInt(amt_str);
		if (amt < 1 || amt > 100000)
		{
			LOG("LOG_CHANNEL", "The amount must be between 1 and 100000");
			sendSystemMessage(self, new string_id(STF, "amount_params"));
			return SCRIPT_CONTINUE;
		}
		
		player_structure.payMaintenance(self, structure, amt);
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int setPrivacy(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		LOG("LOG_CHANNEL", "player_building::setPrivacy");
		
		obj_id structure = player_structure.getStructure(self);
		if (!player_structure.isOwner(structure, self))
		{
			if (utils.isFreeTrial(self))
			{
				return SCRIPT_CONTINUE;
			}
		}
		
		if (!isIdValid(structure))
		{
			LOG("LOG_CHANNEL", "You must be in a building to do that.");
			sendSystemMessage(self, new string_id(STF, "must_be_in_building"));
			return SCRIPT_CONTINUE;
		}
		
		if (!player_structure.isBuilding(structure))
		{
			LOG("LOG_CHANNEL", self + " ->You can only alter privacy on a building.");
			sendSystemMessage(self, new string_id(STF, "privacy_building_only"));
			return SCRIPT_CONTINUE;
		}
		
		if (player_structure.isCivic(structure))
		{
			return SCRIPT_CONTINUE;
		}
		
		if (hasObjVar(structure, hq.VAR_HQ_BASE))
		{
			sendSystemMessage(self, new string_id(STF, "no_privacy_faction_hq"));
			return SCRIPT_CONTINUE;
		}
		
		if (!player_structure.isAdmin(structure, self))
		{
			LOG("LOG_CHANNEL", "You must be a building admin to do that.");
			sendSystemMessage(self, new string_id(STF, "must_be_admin"));
			return SCRIPT_CONTINUE;
		}
		
		if (hasObjVar(structure, "force_public"))
		{
			sendSystemMessage(self, new string_id(STF, "force_public"));
			return SCRIPT_CONTINUE;
		}
		
		if (permissionsIsPublic(structure))
		{
			
			String[] cells = getCellNames(structure);
			if (cells != null)
			{
				for (int i=0; i<cells.length; i++)
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
								sendSystemMessage(self, new string_id(STF, "vendor_no_private"));
								return SCRIPT_CONTINUE;
							}
						}
					}
				}
			}
			
			LOG("LOG_CHANNEL", "This structure is now private.");
			sendSystemMessage(self, new string_id(STF, "structure_now_private"));
			
			obj_id[] objects = player_structure.getObjectsInBuilding(structure);
			String[] entry_list = player_structure.getCompleteEntryList(structure);
			if (objects != null)
			{
				for (int i = 0; i < objects.length; i++)
				{
					testAbortScript();
					if (hasObjVar(objects[i], "noEject"))
					{
						continue;
					}
					
					if (isPlayer(objects[i]) || (isMob(objects[i]) && isIdValid(getMaster(objects[i]))) || (isMob(objects[i]) && (getLevel(objects[i])) >= 10))
					{
						int idx = utils.getElementPositionInArray(entry_list, getFirstName(objects[i]));
						if (idx == -1)
						{
							expelFromBuilding(objects[i]);
						}
					}
				}
			}
			
			permissionsMakePrivate(structure);
		}
		else
		{
			LOG("LOG_CHANNEL", "This structure is now public.");
			sendSystemMessage(self, new string_id(STF, "structure_now_public"));
			permissionsMakePublic(structure);
			
			if (turnstile.hasTurnstile(structure))
			{
				turnstile.removeTurnstile(structure);
			}
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int declareResidence(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		obj_id structure = player_structure.getStructure(self);
		if (utils.isFreeTrial(self))
		{
			sendSystemMessage(self, SID_NO_DECLARE);
			return SCRIPT_CONTINUE;
		}
		
		if (!isIdValid(structure))
		{
			LOG("LOG_CHANNEL", "You must be in a building to do that.");
			sendSystemMessage(self, new string_id(STF, "must_be_in_building"));
			return SCRIPT_CONTINUE;
		}
		
		if (!player_structure.isBuilding(structure))
		{
			LOG("LOG_CHANNEL", "Your declared residence must be a building.");
			sendSystemMessage(self, new string_id(STF, "must_be_in_building"));
			return SCRIPT_CONTINUE;
		}
		
		if (player_structure.isCivic(structure))
		{
			return SCRIPT_CONTINUE;
		}
		
		if (!player_structure.isOwner(structure, self))
		{
			LOG("LOG_CHANNEL", "You must be the owner of the building to declare residence.");
			sendSystemMessage(self, new string_id(STF, "declare_must_be_owner"));
			return SCRIPT_CONTINUE;
		}
		
		if (hasObjVar(structure, hq.VAR_HQ_BASE))
		{
			sendSystemMessage(self, new string_id(STF, "no_hq_residence"));
			return SCRIPT_CONTINUE;
		}
		
		int current_time = getGameTime();
		if (hasObjVar(self, player_structure.VAR_RESIDENCE_CAN_DECLARE))
		{
			
			int declared_time = getIntObjVar(self, player_structure.VAR_RESIDENCE_CAN_DECLARE);
			int duration = current_time - declared_time;
			if (duration < player_structure.MIN_RESIDENCE_DURATION)
			{
				int time_remaining = player_structure.MIN_RESIDENCE_DURATION - duration;
				
				String time_str = utils.assembleTimeRemainToUse(time_remaining, false);
				prose_package pp = new prose_package();
				prose.setStringId(pp, new string_id (STF, "change_residence_time"));
				prose.setTT(pp, time_str);
				LOG("LOG_CHANNEL", "You cannot change residence for "+ time_str);
				sendSystemMessageProse(self, pp);
				return SCRIPT_CONTINUE;
			}
		}
		if (player_structure.isResidence(structure, self))
		{
			LOG("LOG_CHANNEL", "This building is already your residence.");
			sendSystemMessage(self, new string_id(STF, "already_residence"));
			return SCRIPT_CONTINUE;
		}
		
		int currentCity = getCitizenOfCityId(self);
		obj_id currentCityMayor = cityGetLeader(currentCity);
		if (self == currentCityMayor)
		{
			sendSystemMessage(self, city.SID_MAYOR_RESIDENCE_CHANGE);
			return SCRIPT_CONTINUE;
		}
		
		obj_id oldresidence = player_structure.getResidence(self);
		
		LOG("LOG_CHANNEL", "You change your residence to this building");
		sendSystemMessage(self, new string_id(STF, "change_residence"));
		setHouseId(self, structure);
		setObjVar(self, player_structure.VAR_RESIDENCE_CAN_DECLARE, current_time);
		
		setObjVar(structure, player_structure.VAR_RESIDENCE_BUILDING, self);
		
		messageTo(oldresidence, "removeResidentVar", null, 0.f, true);
		city.setCityResidence(self, structure);
		
		skill.grantAllPoliticianSkills(self);
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int transferStructure(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		obj_id structure = player_structure.getStructure(self);
		if (!isIdValid(structure))
		{
			LOG("LOG_CHANNEL", "You must be in a building, be near an installation, or have one targeted to do that.");
			sendSystemMessage(self, new string_id(STF, "no_building"));
			return SCRIPT_CONTINUE;
		}
		
		if (!player_structure.isInAdminRange(structure, self))
		{
			return SCRIPT_CONTINUE;
		}
		
		if (!player_structure.isOwner(structure, self))
		{
			LOG("LOG_CHANNEL", self + "-> You are not the owner of this structure.");
			sendSystemMessage(self, new string_id(STF, "not_owner"));
			return SCRIPT_CONTINUE;
		}
		
		if (player_structure.isCivic(structure))
		{
			return SCRIPT_CONTINUE;
		}
		
		String structureTemplateName = getTemplateName(structure);
		if (structureTemplateName.indexOf ("object/building/faction_perk/hq/") != -1)
		{
			sendSystemMessage(self, new string_id(STF, "faction_base"));
			return SCRIPT_CONTINUE;
		}
		
		if (structureTemplateName.equals("object/building/player/player_house_mustafar_lg.iff"))
		{
			sendSystemMessage(self, new string_id(STF, "mustafar_house"));
			return SCRIPT_CONTINUE;
		}
		if (structureTemplateName.equals("object/building/player/city/barn_no_planet_restriction.iff"))
		{
			sendSystemMessage(self, new string_id(STF, "barn_no_transfer"));
			return SCRIPT_CONTINUE;
		}
		if (structureTemplateName.equals("object/building/player/city/diner_no_planet_restriction.iff"))
		{
			sendSystemMessage(self, new string_id(STF, "diner_no_transfer"));
			return SCRIPT_CONTINUE;
		}
		if ((structureTemplateName.equals("object/building/player/player_house_jedi_meditation_room.iff") || structureTemplateName.equals("object/building/player/player_house_sith_meditation_room.iff")))
		{
			sendSystemMessage(self, new string_id(STF, "meditation_room_no_transfer"));
			return SCRIPT_CONTINUE;
		}
		if (structureTemplateName.equals("object/building/player/player_house_tcg_relaxation_pool.iff"))
		{
			sendSystemMessage(self, new string_id(STF, "relaxation_pool_no_transfer"));
			return SCRIPT_CONTINUE;
		}
		if (structureTemplateName.equals("object/building/player/player_house_hangar.iff"))
		{
			sendSystemMessage(self, new string_id(STF, "hangar_no_transfer"));
			return SCRIPT_CONTINUE;
		}
		if (structureTemplateName.equals("object/building/player/player_house_sandcrawler.iff"))
		{
			sendSystemMessage(self, new string_id(STF, "sandcrawler_house_no_transfer"));
			return SCRIPT_CONTINUE;
		}
		if (structureTemplateName.equals("object/building/player/player_house_atat.iff"))
		{
			sendSystemMessage(self, new string_id(STF, "atat_house_no_transfer"));
			return SCRIPT_CONTINUE;
		}
		if (structureTemplateName.equals("object/building/player/player_house_tcg_emperors_spire.iff"))
		{
			sendSystemMessage(self, new string_id(STF, "tcg_emperors_spire_house_no_transfer"));
			return SCRIPT_CONTINUE;
		}
		if (structureTemplateName.equals("object/building/player/player_house_tcg_rebel_spire.iff"))
		{
			sendSystemMessage(self, new string_id(STF, "tcg_rebel_spire_house_no_transfer"));
			return SCRIPT_CONTINUE;
		}
		if (hasObjVar(structure, "playerStructure_noTransfer"))
		{
			sendSystemMessage(self, new string_id(STF, "tcg_player_house_no_transfer"));
			return SCRIPT_CONTINUE;
		}
		
		if (hasObjVar(structure, "structureChange.storageIncrease"))
		{
			sendSystemMessage(self, new string_id(STF, "storage_increase_transfer"));
			return SCRIPT_CONTINUE;
		}
		
		obj_id notradeItem = null;
		if (player_structure.isBuilding(structure))
		{
			
			obj_id[] buildingPlayers = player_structure.getPlayersInBuilding(structure);
			boolean found = false;
			if (buildingPlayers != null)
			{
				for (int i = 0; i < buildingPlayers.length; ++i)
				{
					testAbortScript();
					if (buildingPlayers[i] == self)
					{
						found = true;
						break;
					}
				}
			}
			if (!found)
			{
				sendSystemMessage(self, SID_NOT_IN_BUILDING);
				return SCRIPT_CONTINUE;
			}
			
			if (!areAllContentsLoaded(structure))
			{
				sendSystemMessage(self, SID_CONTENTS_NOT_LOADED);
				return SCRIPT_CONTINUE;
			}
			
			notradeItem = utils.findNoTradeItemNotVendor(player_structure.getObjectsInBuilding(structure), false);
		}
		else
		{
			
			obj_id[] structureArray = new obj_id[1];
			structureArray[0] = structure;
			notradeItem = utils.findNoTradeItem(structureArray, false);
		}
		
		if (isIdValid(notradeItem))
		{
			prose_package pp = new prose_package();
			pp.stringId = SID_BUILDING_HAS_NOTRADE;
			pp.target.id = notradeItem;
			sendSystemMessageProse(self, pp);
			return SCRIPT_CONTINUE;
		}
		
		if (!isIdValid(target))
		{
			target = getLookAtTarget(self);
			if (!isIdValid(target))
			{
				LOG("LOG_CHANNEL", "You must specify a player with whom to transfer ownership");
				sendSystemMessage(self, new string_id(STF, "no_transfer_target"));
				return SCRIPT_CONTINUE;
			}
		}
		
		if (target == self)
		{
			LOG("LOG_CHANNEL", "You are already the owner.");
			sendSystemMessage(self, new string_id(STF, "already_owner"));
			return SCRIPT_CONTINUE;
		}
		
		if (!isPlayer(target))
		{
			LOG("LOG_CHANNEL", "The target must be a player.");
			sendSystemMessage(self, new string_id(STF, "target_not_player"));
			return SCRIPT_CONTINUE;
		}
		
		if (hasObjVar(structure, player_structure.SPECIAL_SIGN_OWNER_ONLY) && getBooleanObjVar(structure, player_structure.SPECIAL_SIGN_OWNER_ONLY))
		{
			LOG("LOG_CHANNEL", "You cannot transfer ownership with a special sign attached");
			sendSystemMessage(self, player_structure.SID_SPECIAL_SIGN_NO_TRANSFER);
			return SCRIPT_CONTINUE;
		}
		
		if (utils.isFreeTrial(target))
		{
			sendSystemMessage(self, SID_NO_TRANSFER);
			return SCRIPT_CONTINUE;
		}
		
		if (player_structure.isBanned(structure, target))
		{
			LOG("LOG_CHANNEL", "You cannot transfer ownership to a banned player");
			sendSystemMessage(self, new string_id(STF, "no_banned_player"));
			return SCRIPT_CONTINUE;
		}
		
		int city_id = getCityAtLocation(getLocation(structure), 0);
		if (city.isCityBanned(target, city_id))
		{
			sendSystemMessage(self, SID_CANT_TRANSFER_TO_CITY_BANNED);
			return SCRIPT_CONTINUE;
		}
		
		if (!player_structure.canOwnStructure(structure, target))
		{
			LOG("LOG_CHANNEL", getFirstName(target) + " is not able to own this structure.");
			sendSystemMessageProse(self, prose.getPackage(new string_id (STF, "not_able_to_own"), target));
			return SCRIPT_CONTINUE;
		}
		
		if (cityExists(city_id) && city.isCityZoned(city_id) && isIdValid(target))
		{
			
			if (!city.hasZoningRights(target, city_id))
			{
				sendSystemMessage(self, SID_NO_RIGHTS);
				return SCRIPT_CONTINUE;
			}
		}
		
		if (!structure.isAuthoritative() || !target.isAuthoritative())
		{
			obj_id targetTopmostContainer = getTopMostContainer(target);
			if (!isIdValid(targetTopmostContainer))
			{
				targetTopmostContainer = target;
			}
			
			requestSameServer(targetTopmostContainer, structure);
			
			if (!structure.isAuthoritative())
			{
				obj_id selfTopmostContainer = getTopMostContainer(self);
				if (!isIdValid(selfTopmostContainer))
				{
					selfTopmostContainer = self;
				}
				
				requestSameServer(selfTopmostContainer, structure);
			}
			
			sendSystemMessage(self, new string_id(STF, "not_authoritative"));
			return SCRIPT_CONTINUE;
		}
		
		player_structure.removeStructure(structure, self);
		player_structure.addStructure(structure, target);
		
		LOG("LOG_CHANNEL", "Ownership of the structure has been transfered to "+ getFirstName(target));
		sendSystemMessageProse(self, prose.getPackage(new string_id (STF, "ownership_transferred_out"), target));
		LOG("LOG_CHANNEL", target + " ->"+ getFirstName(self) + " has transfered ownership of the structure to you");
		sendSystemMessageProse(target, prose.getPackage(new string_id (STF, "ownership_transferred_in"), self));
		CustomerServiceLog("playerStructure", "STRUCTURE("+ structure +") TEMPLATE: "+ structureTemplateName +" was transferred FROM: "+ getFirstName(self) +"("+ self +") TO"+ getFirstName(target) + "("+ target + ").");
		CustomerServiceLog("c","Structure "+ structure + " has been transfered from "+ getFirstName(self) + " to "+ getFirstName(target));
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int destroyStructure(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!isFactionBaseEmpty(trial.getTop(self), self))
		{
			sendSystemMessage(self, SID_EVACUATE_FACILITY);
			return SCRIPT_CONTINUE;
		}
		
		LOG("LOG_CHANNEL", "player_building::destroyStructure");
		obj_id structure = player_structure.getStructure(self);
		
		if (!isIdValid(structure))
		{
			LOG("LOG_CHANNEL", self + " ->You must in a building or near an installation to use that command.");
			sendSystemMessage(self, new string_id(STF, "command_no_building"));
			return SCRIPT_CONTINUE;
		}
		
		if (utils.hasScriptVar(self, "player_structure.destroy.pid"))
		{
			int oldpid = utils.getIntScriptVar(self, "player_structure.destroy.pid");
			sui.closeSUI(self, oldpid);
			utils.removeScriptVarTree(self, "player_structure.destroy");
		}
		
		if (!player_structure.isCivic(structure) && !player_structure.isInAdminRange(structure, self))
		{
			return SCRIPT_CONTINUE;
		}
		
		String template = getTemplateName(structure);
		if (template.indexOf("cityhall_") > -1)
		{
			
			if (hasObjVar(structure, "cityVoteInterval"))
			{
				int cityVoteInterval = getIntObjVar(structure, "cityVoteInterval");
				if (cityVoteInterval == 2)
				{
					sendSystemMessage(self, SID_DESTRUCTION_LOCKED);
					return SCRIPT_CONTINUE;
				}
			}
		}
		
		if (!player_structure.isOwner(structure, self))
		{
			LOG("LOG_CHANNEL", "You must be the owner to destroy a structure.");
			sendSystemMessage(self, new string_id(STF, "destroy_must_be_owner"));
			return SCRIPT_CONTINUE;
		}
		
		if (player_structure.isGenerator(structure))
		{
			if (isHarvesterActive(structure))
			{
				sendSystemMessage(self, new string_id(STF, "destroy_deactivate_generator_first"));
				return SCRIPT_CONTINUE;
			}
			
			if (!isHarvesterEmpty(structure))
			{
				sendSystemMessage(self, new string_id(STF, "destroy_empty_generator_hopper"));
				return SCRIPT_CONTINUE;
			}
		}
		
		if (player_structure.isHarvester(structure))
		{
			if (isHarvesterActive(structure))
			{
				sendSystemMessage(self, new string_id(STF, "destroy_deactivate_first"));
				return SCRIPT_CONTINUE;
			}
			
			if (!isHarvesterEmpty(structure))
			{
				sendSystemMessage(self, new string_id(STF, "destroy_empty_hopper"));
				return SCRIPT_CONTINUE;
			}
		}
		else if (player_structure.isFactory(structure))
		{
			if (isHarvesterActive(structure))
			{
				prose_package pp = prose.getPackage(SID_DEACTIVATE_FACTORY_FOR_DELETE, self);
				if (pp != null)
				{
					sendSystemMessageProse(self, pp);
				}
				return SCRIPT_CONTINUE;
			}
			String currentSchematicName = getManufactureStationSchematic(structure);
			if (currentSchematicName != null)
			{
				prose_package pp = prose.getPackage(SID_REMOVE_SCHEMATIC_FOR_DELETE, self);
				if (pp != null)
				{
					sendSystemMessageProse(self, pp);
				}
				return SCRIPT_CONTINUE;
			}
			obj_id inputHopper = getManufactureStationInputHopper(structure);
			if (isIdValid(inputHopper))
			{
				if (getNumItemsIn(inputHopper) > 0)
				{
					prose_package pp = prose.getPackage(SID_CLEAR_INPUT_HOPPER_FOR_DELETE, self);
					if (pp != null)
					{
						sendSystemMessageProse(self, pp);
					}
					return SCRIPT_CONTINUE;
				}
			}
			obj_id outputHopper = getManufactureStationOutputHopper(structure);
			if (isIdValid(outputHopper))
			{
				if (getNumItemsIn(outputHopper) > 0)
				{
					prose_package pp = prose.getPackage(SID_CLEAR_OUTPUT_HOPPER_FOR_DELETE, self);
					if (pp != null)
					{
						sendSystemMessageProse(self, pp);
					}
					return SCRIPT_CONTINUE;
				}
			}
		}
		else if (player_structure.isBuilding(structure))
		{
			if (hasObjVar(structure, player_structure.SPECIAL_SIGN_OWNER_ONLY) && getBooleanObjVar(structure, player_structure.SPECIAL_SIGN_OWNER_ONLY))
			{
				
				sendSystemMessage(self, player_structure.SID_REMOVE_SPCIAL_SIGN_TO_DESTROY);
				return SCRIPT_CONTINUE;
				
			}
			if (!areAllContentsLoaded(structure))
			{
				prose_package pp = prose.getPackage(SID_CONTENTS_NOT_LOADED, self);
				if (pp != null)
				{
					sendSystemMessageProse(self, pp);
				}
				return SCRIPT_CONTINUE;
			}
			
			fixHouseItemLimit(structure);
			
			int numItems = player_structure.getStructureNumItems(structure);
			if (numItems > 0)
			{
				prose_package pp = prose.getPackage(SID_CLEAR_BUILDING_FOR_DELETE, self);
				if (pp != null)
				{
					sendSystemMessageProse(self, pp);
				}
				return SCRIPT_CONTINUE;
			}
			
			obj_id[] houseContents = trial.getAllObjectsInDungeon(structure);
			if (houseContents != null && houseContents.length > 0)
			{
				for (int i = 0; i < houseContents.length; i++)
				{
					testAbortScript();
					if (isMob(houseContents[i]))
					{
						int callableType = callable.getCallableType(houseContents[i]);
						if (callableType > 0)
						{
							
							prose_package pp = prose.getPackage(SID_STORE_PETS_FOR_DELETE, self);
							if (pp != null)
							{
								sendSystemMessageProse(self, pp);
							}
							
							return SCRIPT_CONTINUE;
						}
					}
				}
			}
			
			if (!player_structure.canPackStructureWithVendors(self, structure))
			{
				return SCRIPT_CONTINUE;
			}
		}
		
		int key = utils.getIntScriptVar(self, "player_structure.destroy.key");
		boolean redeedVar = utils.getBooleanScriptVar(self, "player_structure.destroy.willredeed");
		if (params != null && key != 0 && params.equals(Integer.toString(key)))
		{
			if (utils.getObjIdScriptVar(self, player_structure.VAR_CONFIRM_DESTROY) != structure)
			{
				sendSystemMessage(self, SID_PENDING_DESTROY);
				return SCRIPT_CONTINUE;
			}
			utils.removeScriptVar(self, player_structure.VAR_CONFIRM_DESTROY);
			
			String sname = player_structure.getStructureName(structure);
			CustomerServiceLog("DestroyStructure", "("+self+") "+getFirstName(self)+" has confirmed /destroyStructure on "+structure+" (Template: "+ template + " Name: "+ sname + "). Owner is "+player_structure.getStructureOwner(structure)+". Will redeed = "+ redeedVar);
			CustomerServiceLog("DestroyStructure", getFirstName(self) + "has initiated a destroyStructure on "+ structure + " (Template: "+ template + " Name: "+ sname + "). Owner is "+ player_structure.getStructureOwner(structure));
			
			if (redeedVar)
			{
				if (player_structure.destroyStructure(structure, true))
				{
					LOG("LOG_CHANNEL", self + " ->Structure destroyed and deed reclaimed.");
					sendSystemMessage(self, new string_id(STF, "deed_reclaimed"));
				}
				else
				{
					sendSystemMessage(self, new string_id(STF, "deed_reclaimed_failed"));
					return SCRIPT_CONTINUE;
				}
			}
			else
			{
				if (player_structure.destroyStructure(structure, false))
				{
					LOG("LOG_CHANNEL", self + " ->Structure destroyed.");
					sendSystemMessage(self, new string_id(STF, "structure_destroyed"));
				}
				else
				{
					sendSystemMessage(self, new string_id(STF, "structure_destroy_failed"));
					return SCRIPT_CONTINUE;
				}
			}
		}
		else
		{
			
			string_id confirm_destruction_title = new string_id(STF, "confirm_destruction_t");
			String title = getString(confirm_destruction_title);
			string_id confirm_destruction_detail1 = new string_id(STF, "confirm_destruction_d1");
			String prompt = getString(confirm_destruction_detail1);
			string_id confirm_destruction_detail2 = new string_id(STF, "confirm_destruction_d2");
			prompt += getString(confirm_destruction_detail2) + "\n\n";
			string_id confirm_destruction_detail3a = new string_id(STF, "confirm_destruction_d3a");
			string_id confirm_destruction_detail3b = new string_id(STF, "confirm_destruction_d3b");
			prompt += getString(confirm_destruction_detail3a) + " \\"+ colors_hex.LIMEGREEN + getString(confirm_destruction_detail3b) + "\\#.";
			string_id confirm_destruction_detail4 = new string_id(STF, "confirm_destruction_d4");
			prompt += getString(confirm_destruction_detail4);
			
			Vector entries = new Vector();
			entries.setSize(0);
			
			String structure_name = player_structure.getStructureName(structure);
			if (structure_name != null && !structure_name.equals(""))
			{
				title = structure_name;
			}
			
			boolean canRedeed = false;
			boolean willRedeed = true;
			if (player_structure.canReclaimDeed(structure))
			{
				canRedeed = true;
			}
			
			if (canRedeed)
			{
				string_id redeed_alert_text = new string_id(STF, "can_redeed_alert");
				string_id redeed_yes_alert_text = new string_id(STF, "can_redeed_yes_suffix");
				entries = utils.addElement(entries, getString(redeed_alert_text) + "\\"+colors_hex.LIMEGREEN+getString(redeed_yes_alert_text));
				
				int max_condition = player_structure.getMaxCondition(structure);
				int condition = player_structure.getStructureCondition(structure);
				
				String conditionColor = colors_hex.LIMEGREEN;
				if (condition < max_condition)
				{
					conditionColor = colors_hex.TOMATO;
					willRedeed = false;
				}
				
				string_id redeed_condition_text = new string_id(STF, "redeed_condition");
				entries = utils.addElement(entries, " - "+ getString(redeed_condition_text) + "\\"+conditionColor+condition+"/"+max_condition);
				
				int pool = player_structure.getMaintenancePool(structure);
				int reclaim_pool = player_structure.getRedeedCost(structure);
				
				String maintColor = colors_hex.LIMEGREEN;
				if (pool < reclaim_pool)
				{
					maintColor = colors_hex.TOMATO;
					willRedeed = false;
				}
				string_id redeed_maintenance_text = new string_id(STF, "redeed_maintenance");
				entries = utils.addElement(entries, " - "+ getString(redeed_maintenance_text) + "\\"+maintColor+pool+"/"+reclaim_pool);
			}
			else
			{
				string_id redeed_alert_text = new string_id(STF, "can_redeed_alert");
				string_id redeed_no_alert_text = new string_id(STF, "can_redeed_no_suffix");
				entries = utils.addElement(entries, getString(redeed_alert_text) + "\\"+colors_hex.TOMATO+getString(redeed_no_alert_text));
				willRedeed = false;
			}
			
			if (willRedeed)
			{
				string_id redeed_confirmation_text = new string_id(STF, "redeed_confirmation");
				string_id redeed_yes_alert_text = new string_id(STF, "can_redeed_yes_suffix");
				prompt += "\n"+ getString(redeed_confirmation_text) + "\\"+colors_hex.LIMEGREEN+getString(redeed_yes_alert_text);
			}
			else
			{
				string_id redeed_confirmation_text = new string_id(STF, "redeed_confirmation");
				string_id redeed_no_alert_text = new string_id(STF, "can_redeed_no_suffix");
				prompt += "\n"+ getString(redeed_confirmation_text) + "\\"+colors_hex.TOMATO+getString(redeed_no_alert_text);
			}
			
			int pid = sui.listbox(self, self, prompt, sui.YES_NO, title, entries, "handleDestroyUi");
			if (pid > -1)
			{
				utils.setScriptVar(self, "player_structure.destroy.pid", pid);
				utils.setScriptVar(self, "player_structure.destroy.target", structure);
				utils.setScriptVar(self, "player_structure.destroy.willredeed", willRedeed);
				utils.setScriptVar(self, "player_structure.destroy.key", rand(100000,999999));
			}
			
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int createVendor(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		
		blog("player_building:createVendor");
		
		if (!validateVendorPlacement(self))
		{
			return SCRIPT_CONTINUE;
		}
		
		utils.setScriptVar(self, CREATING_VENDOR, 1);
		
		requestPlayerVendorCount(self);
		return SCRIPT_CONTINUE;
	}
	
	
	public int handleSaveDecorationLayout(obj_id self, dictionary params) throws InterruptedException
	{
		
		if (utils.hasScriptVar(self, SCRIPTVAR_SAVE_RESTORE_DECORATION_SUI_ID))
		{
			int savedPageId = utils.getIntScriptVar(self, SCRIPTVAR_SAVE_RESTORE_DECORATION_SUI_ID);
			utils.removeScriptVar(self, SCRIPTVAR_SAVE_RESTORE_DECORATION_SUI_ID);
			forceCloseSUIPage(savedPageId);
		}
		
		utils.removeScriptVar(self, SCRIPTVAR_SAVE_RESTORE_DECORATION_SAVE_SLOT_ID);
		utils.removeScriptVar(self, SCRIPTVAR_RESTORE_DECORATION_SAVE_SLOT_CHOICE);
		utils.removeScriptVar(self, SCRIPTVAR_RESTORE_DECORATION_TARGET_ROOMS);
		utils.removeScriptVar(self, SCRIPTVAR_RESTORE_DECORATION_TARGET_ROOM_INDEX);
		utils.removeScriptVar(self, SCRIPTVAR_RESTORE_DECORATION_CURRENT_POB_ROOMS);
		
		obj_id playerObject = getPlayerObject(self);
		if (!isValidId(playerObject))
		{
			return SCRIPT_CONTINUE;
		}
		
		String[][] columnData = new String[3][4];
		
		columnData[0][0] = "1";
		if (hasObjVar(playerObject, "savedDecoration1.saveTime"))
		{
			columnData[0][1] = getStringObjVar(playerObject, "savedDecoration1.pobName");
			if (columnData[0][1] == null)
			{
				columnData[0][1] = "";
			}
			
			int saveTime = getIntObjVar(playerObject, "savedDecoration1.saveTime");
			if (saveTime > 0)
			{
				columnData[0][2] = getCalendarTimeStringLocal(saveTime);
			}
			else
			{
				columnData[0][2] = "";
			}
			
			columnData[0][3] = getStringObjVar(playerObject, "savedDecoration1.description");
			if (columnData[0][3] == null)
			{
				columnData[0][3] = "";
			}
		}
		else
		{
			columnData[0][1] = "<EMPTY>";
			columnData[0][2] = "<EMPTY>";
			columnData[0][3] = "<EMPTY>";
		}
		
		columnData[1][0] = "2";
		if (hasObjVar(playerObject, "savedDecoration2.saveTime"))
		{
			columnData[1][1] = getStringObjVar(playerObject, "savedDecoration2.pobName");
			if (columnData[1][1] == null)
			{
				columnData[1][1] = "";
			}
			
			int saveTime = getIntObjVar(playerObject, "savedDecoration2.saveTime");
			if (saveTime > 0)
			{
				columnData[1][2] = getCalendarTimeStringLocal(saveTime);
			}
			else
			{
				columnData[1][2] = "";
			}
			
			columnData[1][3] = getStringObjVar(playerObject, "savedDecoration2.description");
			if (columnData[1][3] == null)
			{
				columnData[1][3] = "";
			}
		}
		else
		{
			columnData[1][1] = "<EMPTY>";
			columnData[1][2] = "<EMPTY>";
			columnData[1][3] = "<EMPTY>";
		}
		
		columnData[2][0] = "3";
		if (hasObjVar(playerObject, "savedDecoration3.saveTime"))
		{
			columnData[2][1] = getStringObjVar(playerObject, "savedDecoration3.pobName");
			if (columnData[2][1] == null)
			{
				columnData[2][1] = "";
			}
			
			int saveTime = getIntObjVar(playerObject, "savedDecoration3.saveTime");
			if (saveTime > 0)
			{
				columnData[2][2] = getCalendarTimeStringLocal(saveTime);
			}
			else
			{
				columnData[2][2] = "";
			}
			
			columnData[2][3] = getStringObjVar(playerObject, "savedDecoration3.description");
			if (columnData[2][3] == null)
			{
				columnData[2][3] = "";
			}
		}
		else
		{
			columnData[2][1] = "<EMPTY>";
			columnData[2][2] = "<EMPTY>";
			columnData[2][3] = "<EMPTY>";
		}
		
		String[] columnHeader =
		{
			"Save Slot #", "Structure/Ship Type", "Save Time", "Description"
		};
		String[] columnHeaderType =
		{
			"integer", "text", "text", "text"
		};
		
		int pid = sui.tableRowMajor(self, self, sui.OK_CANCEL, "Save Decoration Layout", "handleSelectSaveDecorationLayout", "Select the save slot where you want to save this decoration layout.", columnHeader, columnHeaderType, columnData);
		utils.setScriptVar(self, SCRIPTVAR_SAVE_RESTORE_DECORATION_SUI_ID, pid);
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int handleSelectSaveDecorationLayout(obj_id self, dictionary params) throws InterruptedException
	{
		if (!utils.hasScriptVar(self, SCRIPTVAR_SAVE_RESTORE_DECORATION_SUI_ID))
		{
			return SCRIPT_CONTINUE;
		}
		
		if (params.getInt("pageId") != utils.getIntScriptVar(self, SCRIPTVAR_SAVE_RESTORE_DECORATION_SUI_ID))
		{
			return SCRIPT_CONTINUE;
		}
		
		utils.removeScriptVar(self, SCRIPTVAR_SAVE_RESTORE_DECORATION_SUI_ID);
		
		int bp = sui.getIntButtonPressed(params);
		if (bp == sui.BP_OK)
		{
			int saveSlotId = sui.getTableLogicalIndex(params) + 1;
			if ((saveSlotId < 1) || (saveSlotId > 3))
			{
				return SCRIPT_CONTINUE;
			}
			
			obj_id playerObject = getPlayerObject(self);
			if (!isValidId(playerObject))
			{
				return SCRIPT_CONTINUE;
			}
			
			utils.setScriptVar(self, SCRIPTVAR_SAVE_RESTORE_DECORATION_SAVE_SLOT_ID, saveSlotId);
			if (hasObjVar(playerObject, "savedDecoration" + saveSlotId + ".saveTime"))
			{
				int pid = sui.msgbox(self, self, "Are you sure you want to overwrite the decoration layout currently saved in slot "+ saveSlotId + "?", sui.YES_NO, "Save Decoration Layout", "handleConfirmOverwriteSaveDecorationLayout");
				utils.setScriptVar(self, SCRIPTVAR_SAVE_RESTORE_DECORATION_SUI_ID, pid);
			}
			else
			{
				int pid = sui.inputbox(self, self, "Enter an optional (40 characters max) description for the decoration layout that will be saved in slot "+ saveSlotId + ".", "Save Decoration Layout", "handleSaveDecorationLayoutEnterDescription", 39, false, "");
				setSUIProperty(pid, sui.INPUTBOX_INPUT, "MaxLength", String.valueOf(39));
				utils.setScriptVar(self, SCRIPTVAR_SAVE_RESTORE_DECORATION_SUI_ID, pid);
				showSUIPage(pid);
			}
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int handleConfirmOverwriteSaveDecorationLayout(obj_id self, dictionary params) throws InterruptedException
	{
		if (!utils.hasScriptVar(self, SCRIPTVAR_SAVE_RESTORE_DECORATION_SUI_ID))
		{
			return SCRIPT_CONTINUE;
		}
		
		if (!utils.hasScriptVar(self, SCRIPTVAR_SAVE_RESTORE_DECORATION_SAVE_SLOT_ID))
		{
			return SCRIPT_CONTINUE;
		}
		
		if (params.getInt("pageId") != utils.getIntScriptVar(self, SCRIPTVAR_SAVE_RESTORE_DECORATION_SUI_ID))
		{
			return SCRIPT_CONTINUE;
		}
		
		int saveSlotId = utils.getIntScriptVar(self, SCRIPTVAR_SAVE_RESTORE_DECORATION_SAVE_SLOT_ID);
		
		utils.removeScriptVar(self, SCRIPTVAR_SAVE_RESTORE_DECORATION_SUI_ID);
		utils.removeScriptVar(self, SCRIPTVAR_SAVE_RESTORE_DECORATION_SAVE_SLOT_ID);
		
		int bp = sui.getIntButtonPressed(params);
		if (bp == sui.BP_OK)
		{
			int pid = sui.inputbox(self, self, "Enter an optional (40 characters max) description for the decoration layout that will be saved in slot "+ saveSlotId + ".", "Save Decoration Layout", "handleSaveDecorationLayoutEnterDescription", 39, false, "");
			setSUIProperty(pid, sui.INPUTBOX_INPUT, "MaxLength", String.valueOf(39));
			utils.setScriptVar(self, SCRIPTVAR_SAVE_RESTORE_DECORATION_SUI_ID, pid);
			utils.setScriptVar(self, SCRIPTVAR_SAVE_RESTORE_DECORATION_SAVE_SLOT_ID, saveSlotId);
			showSUIPage(pid);
		}
		else
		{
			messageTo(self, "handleSaveDecorationLayout", null, 0.1f, false);
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int handleSaveDecorationLayoutEnterDescription(obj_id self, dictionary params) throws InterruptedException
	{
		if (!utils.hasScriptVar(self, SCRIPTVAR_SAVE_RESTORE_DECORATION_SUI_ID))
		{
			return SCRIPT_CONTINUE;
		}
		
		if (!utils.hasScriptVar(self, SCRIPTVAR_SAVE_RESTORE_DECORATION_SAVE_SLOT_ID))
		{
			return SCRIPT_CONTINUE;
		}
		
		if (params.getInt("pageId") != utils.getIntScriptVar(self, SCRIPTVAR_SAVE_RESTORE_DECORATION_SUI_ID))
		{
			return SCRIPT_CONTINUE;
		}
		
		int saveSlotId = utils.getIntScriptVar(self, SCRIPTVAR_SAVE_RESTORE_DECORATION_SAVE_SLOT_ID);
		
		utils.removeScriptVar(self, SCRIPTVAR_SAVE_RESTORE_DECORATION_SUI_ID);
		utils.removeScriptVar(self, SCRIPTVAR_SAVE_RESTORE_DECORATION_SAVE_SLOT_ID);
		
		int bp = sui.getIntButtonPressed(params);
		if (bp == sui.BP_OK)
		{
			obj_id structure = getContainedBy(self);
			if (isValidId(structure))
			{
				structure = getContainedBy(structure);
			}
			
			if (!isValidId(structure))
			{
				sendSystemMessage(self, "You must be standing inside a structure or POB ship to do this.", null );
			}
			else
			{
				String description = sui.getInputBoxText(params);
				if (description == null)
				{
					description = "";
				}
				
				saveDecorationLayout(self, structure, saveSlotId, description);
			}
		}
		else
		{
			messageTo(self, "handleSaveDecorationLayout", null, 0.1f, false);
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public void displayRestoreDecorationRoomMapping(obj_id player, String[] savedRooms, String[] targetRooms, int saveSlotId) throws InterruptedException
	{
		if (!isIdValid(player) || !exists(player))
		{
			return;
		}
		
		String[] columnHeader =
		{
			"Items that were in this room at the time the layout was saved", "Will be moved/restored into this room"
		};
		String[] columnHeaderType =
		{
			"text", "text"
		};
		String[][] columnData = new String[2][0];
		
		String[] tempSavedRooms = new String[savedRooms.length + 1];
		for (int i = 0; i < savedRooms.length; ++i)
		{
			testAbortScript();
			tempSavedRooms[i] = savedRooms[i];
		}
		tempSavedRooms[savedRooms.length] = "Restore Decoration Layout";
		
		String[] tempTargetRooms = new String[targetRooms.length + 1];
		for (int i = 0; i < targetRooms.length; ++i)
		{
			testAbortScript();
			tempTargetRooms[i] = targetRooms[i];
		}
		tempTargetRooms[targetRooms.length] = "";
		
		columnData[0] = tempSavedRooms;
		columnData[1] = tempTargetRooms;
		
		int pid = sui.tableColumnMajor(player, player, sui.OK_CANCEL, "Restore Decoration Layout", "handleSelectRestoreDecorationRoomMapping", "@player_structure:restore_decoration_layout_specify_target_room", columnHeader, columnHeaderType, columnData);
		utils.setScriptVar(player, SCRIPTVAR_SAVE_RESTORE_DECORATION_SUI_ID, pid);
		utils.setScriptVar(player, SCRIPTVAR_SAVE_RESTORE_DECORATION_SAVE_SLOT_ID, saveSlotId);
		utils.setScriptVar(player, SCRIPTVAR_RESTORE_DECORATION_TARGET_ROOMS, targetRooms);
	}
	
	
	public int handleRestoreDecorationLayout(obj_id self, dictionary params) throws InterruptedException
	{
		
		if (utils.hasScriptVar(self, SCRIPTVAR_SAVE_RESTORE_DECORATION_SUI_ID))
		{
			int savedPageId = utils.getIntScriptVar(self, SCRIPTVAR_SAVE_RESTORE_DECORATION_SUI_ID);
			utils.removeScriptVar(self, SCRIPTVAR_SAVE_RESTORE_DECORATION_SUI_ID);
			forceCloseSUIPage(savedPageId);
		}
		
		utils.removeScriptVar(self, SCRIPTVAR_SAVE_RESTORE_DECORATION_SAVE_SLOT_ID);
		utils.removeScriptVar(self, SCRIPTVAR_RESTORE_DECORATION_SAVE_SLOT_CHOICE);
		utils.removeScriptVar(self, SCRIPTVAR_RESTORE_DECORATION_TARGET_ROOMS);
		utils.removeScriptVar(self, SCRIPTVAR_RESTORE_DECORATION_TARGET_ROOM_INDEX);
		utils.removeScriptVar(self, SCRIPTVAR_RESTORE_DECORATION_CURRENT_POB_ROOMS);
		
		obj_id playerObject = getPlayerObject(self);
		if (!isValidId(playerObject))
		{
			return SCRIPT_CONTINUE;
		}
		
		int numberOfSavedSlots = 0;
		
		String[] saveSlot1 = null;
		if (hasObjVar(playerObject, "savedDecoration1.saveTime"))
		{
			int saveTime = getIntObjVar(playerObject, "savedDecoration1.saveTime");
			if (saveTime > 0)
			{
				++numberOfSavedSlots;
				
				saveSlot1 = new String[4];
				
				saveSlot1[0] = "1";
				
				saveSlot1[1] = getStringObjVar(playerObject, "savedDecoration1.pobName");
				if (saveSlot1[1] == null)
				{
					saveSlot1[1] = "";
				}
				
				saveSlot1[2] = getCalendarTimeStringLocal(saveTime);
				
				saveSlot1[3] = getStringObjVar(playerObject, "savedDecoration1.description");
				if (saveSlot1[3] == null)
				{
					saveSlot1[3] = "";
				}
			}
		}
		
		String[] saveSlot2 = null;
		if (hasObjVar(playerObject, "savedDecoration2.saveTime"))
		{
			int saveTime = getIntObjVar(playerObject, "savedDecoration2.saveTime");
			if (saveTime > 0)
			{
				++numberOfSavedSlots;
				
				saveSlot2 = new String[4];
				
				saveSlot2[0] = "2";
				
				saveSlot2[1] = getStringObjVar(playerObject, "savedDecoration2.pobName");
				if (saveSlot2[1] == null)
				{
					saveSlot2[1] = "";
				}
				
				saveSlot2[2] = getCalendarTimeStringLocal(saveTime);
				
				saveSlot2[3] = getStringObjVar(playerObject, "savedDecoration2.description");
				if (saveSlot2[3] == null)
				{
					saveSlot2[3] = "";
				}
			}
		}
		
		String[] saveSlot3 = null;
		if (hasObjVar(playerObject, "savedDecoration3.saveTime"))
		{
			int saveTime = getIntObjVar(playerObject, "savedDecoration3.saveTime");
			if (saveTime > 0)
			{
				++numberOfSavedSlots;
				
				saveSlot3 = new String[4];
				
				saveSlot3[0] = "3";
				
				saveSlot3[1] = getStringObjVar(playerObject, "savedDecoration3.pobName");
				if (saveSlot3[1] == null)
				{
					saveSlot3[1] = "";
				}
				
				saveSlot3[2] = getCalendarTimeStringLocal(saveTime);
				
				saveSlot3[3] = getStringObjVar(playerObject, "savedDecoration3.description");
				if (saveSlot3[3] == null)
				{
					saveSlot3[3] = "";
				}
			}
		}
		
		if (numberOfSavedSlots <= 0)
		{
			sendSystemMessage(self, "You do not have any saved decoration layout.", null);
			return SCRIPT_CONTINUE;
		}
		
		String[][] columnData = new String[numberOfSavedSlots][0];
		int[] choicesSaveSlot =
		{
			0, 0, 0
		};
		int choicesSaveSlotIndex = 0;
		
		if (saveSlot1 != null)
		{
			columnData[choicesSaveSlotIndex] = saveSlot1;
			choicesSaveSlot[choicesSaveSlotIndex] = 1;
			++choicesSaveSlotIndex;
		}
		
		if (saveSlot2 != null)
		{
			columnData[choicesSaveSlotIndex] = saveSlot2;
			choicesSaveSlot[choicesSaveSlotIndex] = 2;
			++choicesSaveSlotIndex;
		}
		
		if (saveSlot3 != null)
		{
			columnData[choicesSaveSlotIndex] = saveSlot3;
			choicesSaveSlot[choicesSaveSlotIndex] = 3;
			++choicesSaveSlotIndex;
		}
		
		String[] columnHeader =
		{
			"Save Slot #", "Structure/Ship Type", "Save Time", "Description"
		};
		String[] columnHeaderType =
		{
			"integer", "text", "text", "text"
		};
		
		int pid = sui.tableRowMajor(self, self, sui.OK_CANCEL, "Restore Decoration Layout", "handleSelectRestoreDecorationLayout", "Select the decoration layout save slot you wish to restore.", columnHeader, columnHeaderType, columnData);
		utils.setScriptVar(self, SCRIPTVAR_SAVE_RESTORE_DECORATION_SUI_ID, pid);
		utils.setScriptVar(self, SCRIPTVAR_RESTORE_DECORATION_SAVE_SLOT_CHOICE, choicesSaveSlot);
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int handleSelectRestoreDecorationLayout(obj_id self, dictionary params) throws InterruptedException
	{
		if (!utils.hasScriptVar(self, SCRIPTVAR_SAVE_RESTORE_DECORATION_SUI_ID))
		{
			return SCRIPT_CONTINUE;
		}
		
		if (!utils.hasScriptVar(self, SCRIPTVAR_RESTORE_DECORATION_SAVE_SLOT_CHOICE))
		{
			return SCRIPT_CONTINUE;
		}
		
		if (params.getInt("pageId") != utils.getIntScriptVar(self, SCRIPTVAR_SAVE_RESTORE_DECORATION_SUI_ID))
		{
			return SCRIPT_CONTINUE;
		}
		
		int[] choicesSaveSlot = utils.getIntArrayScriptVar(self, SCRIPTVAR_RESTORE_DECORATION_SAVE_SLOT_CHOICE);
		
		utils.removeScriptVar(self, SCRIPTVAR_SAVE_RESTORE_DECORATION_SUI_ID);
		utils.removeScriptVar(self, SCRIPTVAR_RESTORE_DECORATION_SAVE_SLOT_CHOICE);
		
		if ((choicesSaveSlot == null) || (choicesSaveSlot.length != 3))
		{
			return SCRIPT_CONTINUE;
		}
		
		int bp = sui.getIntButtonPressed(params);
		if (bp == sui.BP_OK)
		{
			int rowsSelected = sui.getTableLogicalIndex(params);
			if ((rowsSelected < 0) || (rowsSelected >= choicesSaveSlot.length))
			{
				return SCRIPT_CONTINUE;
			}
			
			int saveSlotId = choicesSaveSlot[rowsSelected];
			if ((saveSlotId < 1) || (saveSlotId > 3))
			{
				return SCRIPT_CONTINUE;
			}
			
			obj_id playerObject = getPlayerObject(self);
			if (!isValidId(playerObject))
			{
				return SCRIPT_CONTINUE;
			}
			
			String[] savedRooms = getStringArrayObjVar(playerObject, "savedDecoration"+ saveSlotId + ".rooms");
			if ((savedRooms == null) || (savedRooms.length <= 0))
			{
				return SCRIPT_CONTINUE;
			}
			
			obj_id structure = getContainedBy(self);
			if (isValidId(structure))
			{
				structure = getContainedBy(structure);
			}
			
			if (!isValidId(structure))
			{
				sendSystemMessage(self, "You must be standing inside a structure or POB ship to do this.", null);
				return SCRIPT_CONTINUE;
			}
			
			String[] roomsOfCurrentPob = getCellNames(structure);
			if ((roomsOfCurrentPob == null) || (roomsOfCurrentPob.length <= 0))
			{
				sendSystemMessage(self, "You must be standing inside a structure or POB ship to do this.", null);
				return SCRIPT_CONTINUE;
			}
			
			String[] targetRooms = getStringArrayObjVar(playerObject, "savedDecoration"+ saveSlotId + ".targetRooms");
			if ((targetRooms != null) && (targetRooms.length == savedRooms.length))
			{
				
				for (int i = 0; i < targetRooms.length; ++i)
				{
					testAbortScript();
					boolean found = false;
					for (int j = 0; j < roomsOfCurrentPob.length; ++j)
					{
						testAbortScript();
						if (targetRooms[i].equals(roomsOfCurrentPob[j]))
						{
							found = true;
							break;
						}
					}
					
					if (!found)
					{
						targetRooms[i] = "(will not be moved/restored)";
					}
				}
			}
			else
			{
				
				targetRooms = new String[savedRooms.length];
				
				for (int i = 0; i < savedRooms.length; ++i)
				{
					testAbortScript();
					boolean found = false;
					for (int j = 0; j < roomsOfCurrentPob.length; ++j)
					{
						testAbortScript();
						if (savedRooms[i].equals(roomsOfCurrentPob[j]))
						{
							targetRooms[i] = savedRooms[i];
							found = true;
							break;
						}
					}
					
					if (!found)
					{
						targetRooms[i] = "(will not be moved/restored)";
					}
				}
			}
			
			displayRestoreDecorationRoomMapping(self, savedRooms, targetRooms, saveSlotId);
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int handleSelectRestoreDecorationRoomMapping(obj_id self, dictionary params) throws InterruptedException
	{
		if (!utils.hasScriptVar(self, SCRIPTVAR_SAVE_RESTORE_DECORATION_SUI_ID))
		{
			return SCRIPT_CONTINUE;
		}
		
		if (!utils.hasScriptVar(self, SCRIPTVAR_SAVE_RESTORE_DECORATION_SAVE_SLOT_ID))
		{
			return SCRIPT_CONTINUE;
		}
		
		if (!utils.hasScriptVar(self, SCRIPTVAR_RESTORE_DECORATION_TARGET_ROOMS))
		{
			return SCRIPT_CONTINUE;
		}
		
		if (params.getInt("pageId") != utils.getIntScriptVar(self, SCRIPTVAR_SAVE_RESTORE_DECORATION_SUI_ID))
		{
			return SCRIPT_CONTINUE;
		}
		
		int saveSlotId = utils.getIntScriptVar(self, SCRIPTVAR_SAVE_RESTORE_DECORATION_SAVE_SLOT_ID);
		String[] targetRooms = utils.getStringArrayScriptVar(self, SCRIPTVAR_RESTORE_DECORATION_TARGET_ROOMS);
		
		utils.removeScriptVar(self, SCRIPTVAR_SAVE_RESTORE_DECORATION_SUI_ID);
		utils.removeScriptVar(self, SCRIPTVAR_SAVE_RESTORE_DECORATION_SAVE_SLOT_ID);
		utils.removeScriptVar(self, SCRIPTVAR_RESTORE_DECORATION_TARGET_ROOMS);
		
		int bp = sui.getIntButtonPressed(params);
		if (bp == sui.BP_OK)
		{
			obj_id playerObject = getPlayerObject(self);
			if (!isValidId(playerObject))
			{
				return SCRIPT_CONTINUE;
			}
			
			String[] savedRooms = getStringArrayObjVar(playerObject, "savedDecoration"+ saveSlotId + ".rooms");
			if ((savedRooms == null) || (savedRooms.length <= 0))
			{
				return SCRIPT_CONTINUE;
			}
			
			if ((targetRooms == null) || (targetRooms.length != savedRooms.length))
			{
				return SCRIPT_CONTINUE;
			}
			
			int rowsSelected = sui.getTableLogicalIndex(params);
			if (rowsSelected == savedRooms.length)
			{
				
				obj_id structure = getContainedBy(self);
				if (isValidId(structure))
				{
					structure = getContainedBy(structure);
				}
				
				if (!isValidId(structure))
				{
					int pid = sui.msgbox(self, self, "You must be standing inside a structure or POB ship to do this.", sui.OK_ONLY, "Restore Decoration Layout", "handleRestoreDecorationReturnToRoomMapping");
					utils.setScriptVar(self, SCRIPTVAR_SAVE_RESTORE_DECORATION_SUI_ID, pid);
					utils.setScriptVar(self, SCRIPTVAR_SAVE_RESTORE_DECORATION_SAVE_SLOT_ID, saveSlotId);
					utils.setScriptVar(self, SCRIPTVAR_RESTORE_DECORATION_TARGET_ROOMS, targetRooms);
					
					return SCRIPT_CONTINUE;
				}
				
				String[] roomsOfCurrentPob = getCellNames(structure);
				if ((roomsOfCurrentPob == null) || (roomsOfCurrentPob.length <= 0))
				{
					int pid = sui.msgbox(self, self, "You must be standing inside a structure or POB ship to do this.", sui.OK_ONLY, "Restore Decoration Layout", "handleRestoreDecorationReturnToRoomMapping");
					utils.setScriptVar(self, SCRIPTVAR_SAVE_RESTORE_DECORATION_SUI_ID, pid);
					utils.setScriptVar(self, SCRIPTVAR_SAVE_RESTORE_DECORATION_SAVE_SLOT_ID, saveSlotId);
					utils.setScriptVar(self, SCRIPTVAR_RESTORE_DECORATION_TARGET_ROOMS, targetRooms);
					
					return SCRIPT_CONTINUE;
				}
				
				boolean hasValidTargetRoom = false;
				for (int i = 0; i < targetRooms.length; ++i)
				{
					testAbortScript();
					if ((targetRooms[i] == null) || (targetRooms[i].length() <= 0) || targetRooms[i].equals("(will not be moved/restored)"))
					{
						continue;
					}
					
					boolean found = false;
					for (int j = 0; j < roomsOfCurrentPob.length; ++j)
					{
						testAbortScript();
						if (targetRooms[i].equals(roomsOfCurrentPob[j]))
						{
							hasValidTargetRoom = true;
							found = true;
							break;
						}
					}
					
					if (!found)
					{
						targetRooms[i] = "(will not be moved/restored)";
					}
				}
				
				if (!hasValidTargetRoom)
				{
					int pid = sui.msgbox(self, self, "You have not specified any room into which to move/restore the decoration layout.", sui.OK_ONLY, "Restore Decoration Layout", "handleRestoreDecorationReturnToRoomMapping");
					utils.setScriptVar(self, SCRIPTVAR_SAVE_RESTORE_DECORATION_SUI_ID, pid);
					utils.setScriptVar(self, SCRIPTVAR_SAVE_RESTORE_DECORATION_SAVE_SLOT_ID, saveSlotId);
					utils.setScriptVar(self, SCRIPTVAR_RESTORE_DECORATION_TARGET_ROOMS, targetRooms);
					
					return SCRIPT_CONTINUE;
				}
				
				for (int k = 0; k < targetRooms.length; ++k)
				{
					testAbortScript();
					if ((targetRooms[k] == null) || targetRooms[k].equals("(will not be moved/restored)"))
					{
						targetRooms[k] = "";
					}
				}
				
				setObjVar(playerObject, "savedDecoration"+ saveSlotId + ".targetRooms", targetRooms);
				restoreDecorationLayout(self, structure, saveSlotId);
				
				return SCRIPT_CONTINUE;
			}
			
			if ((rowsSelected < 0) || (rowsSelected >= savedRooms.length))
			{
				int pid = sui.msgbox(self, self, "Please select an action to perform.", sui.OK_ONLY, "Restore Decoration Layout", "handleRestoreDecorationReturnToRoomMapping");
				utils.setScriptVar(self, SCRIPTVAR_SAVE_RESTORE_DECORATION_SUI_ID, pid);
				utils.setScriptVar(self, SCRIPTVAR_SAVE_RESTORE_DECORATION_SAVE_SLOT_ID, saveSlotId);
				utils.setScriptVar(self, SCRIPTVAR_RESTORE_DECORATION_TARGET_ROOMS, targetRooms);
				
				return SCRIPT_CONTINUE;
			}
			
			obj_id structure = getContainedBy(self);
			if (isValidId(structure))
			{
				structure = getContainedBy(structure);
			}
			
			if (!isValidId(structure))
			{
				int pid = sui.msgbox(self, self, "You must be standing inside a structure or POB ship to do this.", sui.OK_ONLY, "Restore Decoration Layout", "handleRestoreDecorationReturnToRoomMapping");
				utils.setScriptVar(self, SCRIPTVAR_SAVE_RESTORE_DECORATION_SUI_ID, pid);
				utils.setScriptVar(self, SCRIPTVAR_SAVE_RESTORE_DECORATION_SAVE_SLOT_ID, saveSlotId);
				utils.setScriptVar(self, SCRIPTVAR_RESTORE_DECORATION_TARGET_ROOMS, targetRooms);
				
				return SCRIPT_CONTINUE;
			}
			
			String[] roomsOfCurrentPob = getCellNames(structure);
			if ((roomsOfCurrentPob == null) || (roomsOfCurrentPob.length <= 0))
			{
				int pid = sui.msgbox(self, self, "You must be standing inside a structure or POB ship to do this.", sui.OK_ONLY, "Restore Decoration Layout", "handleRestoreDecorationReturnToRoomMapping");
				utils.setScriptVar(self, SCRIPTVAR_SAVE_RESTORE_DECORATION_SUI_ID, pid);
				utils.setScriptVar(self, SCRIPTVAR_SAVE_RESTORE_DECORATION_SAVE_SLOT_ID, saveSlotId);
				utils.setScriptVar(self, SCRIPTVAR_RESTORE_DECORATION_TARGET_ROOMS, targetRooms);
				
				return SCRIPT_CONTINUE;
			}
			
			String[] roomsSelection = new String[roomsOfCurrentPob.length + 1];
			roomsSelection[0] = "do not move/restore items in room " + savedRooms[rowsSelected];
			for (int i = 1; i < roomsSelection.length; ++i)
			{
				testAbortScript();
				roomsSelection[i] = roomsOfCurrentPob[i-1];
			}
			
			int pid = sui.listbox(self, self, "Select the room into which you wish to move/restore the items that were in room "+ savedRooms[rowsSelected] + " at the time the decoration layout was saved.", sui.OK_CANCEL, "Restore Decoration Layout", roomsSelection, "handleSelectRestoreDecorationTargetRoom", true, true);
			utils.setScriptVar(self, SCRIPTVAR_SAVE_RESTORE_DECORATION_SUI_ID, pid);
			utils.setScriptVar(self, SCRIPTVAR_SAVE_RESTORE_DECORATION_SAVE_SLOT_ID, saveSlotId);
			utils.setScriptVar(self, SCRIPTVAR_RESTORE_DECORATION_TARGET_ROOMS, targetRooms);
			utils.setScriptVar(self, SCRIPTVAR_RESTORE_DECORATION_TARGET_ROOM_INDEX, rowsSelected);
			utils.setScriptVar(self, SCRIPTVAR_RESTORE_DECORATION_CURRENT_POB_ROOMS, roomsSelection);
		}
		else
		{
			messageTo(self, "handleRestoreDecorationLayout", null, 0.1f, false);
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int handleRestoreDecorationReturnToRoomMapping(obj_id self, dictionary params) throws InterruptedException
	{
		if (!utils.hasScriptVar(self, SCRIPTVAR_SAVE_RESTORE_DECORATION_SUI_ID))
		{
			return SCRIPT_CONTINUE;
		}
		
		if (!utils.hasScriptVar(self, SCRIPTVAR_SAVE_RESTORE_DECORATION_SAVE_SLOT_ID))
		{
			return SCRIPT_CONTINUE;
		}
		
		if (!utils.hasScriptVar(self, SCRIPTVAR_RESTORE_DECORATION_TARGET_ROOMS))
		{
			return SCRIPT_CONTINUE;
		}
		
		if (params.getInt("pageId") != utils.getIntScriptVar(self, SCRIPTVAR_SAVE_RESTORE_DECORATION_SUI_ID))
		{
			return SCRIPT_CONTINUE;
		}
		
		int saveSlotId = utils.getIntScriptVar(self, SCRIPTVAR_SAVE_RESTORE_DECORATION_SAVE_SLOT_ID);
		String[] targetRooms = utils.getStringArrayScriptVar(self, SCRIPTVAR_RESTORE_DECORATION_TARGET_ROOMS);
		
		utils.removeScriptVar(self, SCRIPTVAR_SAVE_RESTORE_DECORATION_SUI_ID);
		utils.removeScriptVar(self, SCRIPTVAR_SAVE_RESTORE_DECORATION_SAVE_SLOT_ID);
		utils.removeScriptVar(self, SCRIPTVAR_RESTORE_DECORATION_TARGET_ROOMS);
		
		obj_id playerObject = getPlayerObject(self);
		if (!isValidId(playerObject))
		{
			return SCRIPT_CONTINUE;
		}
		
		String[] savedRooms = getStringArrayObjVar(playerObject, "savedDecoration"+ saveSlotId + ".rooms");
		if ((savedRooms == null) || (savedRooms.length <= 0))
		{
			return SCRIPT_CONTINUE;
		}
		
		if ((targetRooms == null) || (targetRooms.length != savedRooms.length))
		{
			return SCRIPT_CONTINUE;
		}
		
		displayRestoreDecorationRoomMapping(self, savedRooms, targetRooms, saveSlotId);
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int handleSelectRestoreDecorationTargetRoom(obj_id self, dictionary params) throws InterruptedException
	{
		if (!utils.hasScriptVar(self, SCRIPTVAR_SAVE_RESTORE_DECORATION_SUI_ID))
		{
			return SCRIPT_CONTINUE;
		}
		
		if (!utils.hasScriptVar(self, SCRIPTVAR_SAVE_RESTORE_DECORATION_SAVE_SLOT_ID))
		{
			return SCRIPT_CONTINUE;
		}
		
		if (!utils.hasScriptVar(self, SCRIPTVAR_RESTORE_DECORATION_TARGET_ROOMS))
		{
			return SCRIPT_CONTINUE;
		}
		
		if (!utils.hasScriptVar(self, SCRIPTVAR_RESTORE_DECORATION_TARGET_ROOM_INDEX))
		{
			return SCRIPT_CONTINUE;
		}
		
		if (!utils.hasScriptVar(self, SCRIPTVAR_RESTORE_DECORATION_CURRENT_POB_ROOMS))
		{
			return SCRIPT_CONTINUE;
		}
		
		if (params.getInt("pageId") != utils.getIntScriptVar(self, SCRIPTVAR_SAVE_RESTORE_DECORATION_SUI_ID))
		{
			return SCRIPT_CONTINUE;
		}
		
		int saveSlotId = utils.getIntScriptVar(self, SCRIPTVAR_SAVE_RESTORE_DECORATION_SAVE_SLOT_ID);
		String[] targetRooms = utils.getStringArrayScriptVar(self, SCRIPTVAR_RESTORE_DECORATION_TARGET_ROOMS);
		int targetRoomIndex = utils.getIntScriptVar(self, SCRIPTVAR_RESTORE_DECORATION_TARGET_ROOM_INDEX);
		String[] roomsOfCurrentPob = utils.getStringArrayScriptVar(self, SCRIPTVAR_RESTORE_DECORATION_CURRENT_POB_ROOMS);
		
		utils.removeScriptVar(self, SCRIPTVAR_SAVE_RESTORE_DECORATION_SUI_ID);
		utils.removeScriptVar(self, SCRIPTVAR_SAVE_RESTORE_DECORATION_SAVE_SLOT_ID);
		utils.removeScriptVar(self, SCRIPTVAR_RESTORE_DECORATION_TARGET_ROOMS);
		utils.removeScriptVar(self, SCRIPTVAR_RESTORE_DECORATION_TARGET_ROOM_INDEX);
		utils.removeScriptVar(self, SCRIPTVAR_RESTORE_DECORATION_CURRENT_POB_ROOMS);
		
		obj_id playerObject = getPlayerObject(self);
		if (!isValidId(playerObject))
		{
			return SCRIPT_CONTINUE;
		}
		
		String[] savedRooms = getStringArrayObjVar(playerObject, "savedDecoration"+ saveSlotId + ".rooms");
		if ((savedRooms == null) || (savedRooms.length <= 0))
		{
			return SCRIPT_CONTINUE;
		}
		
		if ((targetRooms == null) || (targetRooms.length != savedRooms.length))
		{
			return SCRIPT_CONTINUE;
		}
		
		if ((targetRoomIndex < 0) || (targetRoomIndex >= targetRooms.length))
		{
			return SCRIPT_CONTINUE;
		}
		
		int bp = sui.getIntButtonPressed(params);
		if (bp == sui.BP_OK)
		{
			int rowsSelected = sui.getListboxSelectedRow(params);
			if ((rowsSelected >= 0) && (roomsOfCurrentPob != null) && (roomsOfCurrentPob.length > 0) && (rowsSelected < roomsOfCurrentPob.length))
			{
				if (rowsSelected == 0)
				{
					targetRooms[targetRoomIndex] = "(will not be moved/restored)";
				}
				else
				{
					targetRooms[targetRoomIndex] = roomsOfCurrentPob[rowsSelected];
				}
			}
		}
		
		displayRestoreDecorationRoomMapping(self, savedRooms, targetRooms, saveSlotId);
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int handleVendorTypeSelect(obj_id self, dictionary params) throws InterruptedException
	{
		if (params == null || params.isEmpty())
		{
			return SCRIPT_CONTINUE;
		}
		
		blog("player_building.handleVendorTypeSelect: init");
		
		int idx = sui.getListboxSelectedRow(params);
		if (idx < 0)
		{
			idx = 0;
		}
		
		obj_id player = sui.getPlayerId(params);
		if (!isValidId(player) || !exists(player))
		{
			removeVendorVars(self);
			return SCRIPT_CONTINUE;
		}
		
		int btn = sui.getIntButtonPressed(params);
		if (btn == sui.BP_CANCEL)
		{
			removeVendorVars(self);
			return SCRIPT_CONTINUE;
		}
		
		blog("player_building.handleVendorTypeSelect: idx: "+idx);
		
		String[] randomOnly =
		{
			"@player_structure:random"
		};
		utils.setScriptVar(player, "vendor.checkGender", 0);
		int hiringMod = getSkillStatMod(player, "hiring");
		blog("handleVendorTypeSelect - hiringMod: "+hiringMod);
		switch (idx)
		{
			case 0: 
			blog("player_building.handleVendorTypeSelect: selected TERMINAL VENDOR TYPE: ");
			
			String[] rawTerminalTypes = dataTableGetStringColumn(TBL_TERMINAL_TYPES, 0);
			
			int test = 10;
			Vector vendorTypes = new Vector();
			vendorTypes.setSize(0);
			for (int i=0; i<rawTerminalTypes.length; i++)
			{
				testAbortScript();
				if (hiringMod >= test)
				{
					vendorTypes = utils.addElement(vendorTypes, "@player_structure:terminal_"+rawTerminalTypes[i]);
				}
				test += 20;
			}
			sui.listbox(player, player, "@player_structure:terminal_type_d", sui.OK_CANCEL, "@player_structure:terminal_type_t", vendorTypes, "handleTerminalDesignSelect", true);
			break;
			case 1: 
			blog("player_building.handleVendorTypeSelect: selected TERMINAL DROID TYPE: ");
			
			String[] rawDroidTypes = dataTableGetStringColumn(TBL_DROID_TYPES, 0);
			
			int dtest = 20;
			Vector droidTypes = null;
			for (int i=0; i<rawDroidTypes.length; i++)
			{
				testAbortScript();
				if (hiringMod >= dtest)
				{
					droidTypes = utils.addElement(droidTypes, "@player_structure:droid_"+rawDroidTypes[i]);
				}
				dtest += 20;
			}
			sui.listbox(player, player, "@player_structure:droid_type_d", sui.OK_CANCEL, "@player_structure:droid_type_t", droidTypes, "handleDroidModelSelect", true);
			break;
			case 2: 
			blog("player_building.handleVendorTypeSelect: selected TERMINAL NPC TYPE: ");
			if (hiringMod < 50)
			{
				blog("player_building.handleVendorTypeSelect: player hiringMod < 50");
				sui.listbox(player, player, "@player_structure:race_type_d", sui.OK_CANCEL, "@player_structure:race_type_t", randomOnly, "handleVendorRaceSelect", true);
				break;
			}
			else
			{
				blog("player_building.handleVendorTypeSelect: player hiringMod > 50");
				
				String[] allSpecialVendors = getSpecialVendors(self, hiringMod);
				blog("player_building.handleVendorTypeSelect: allSpecialVendors.length: "+allSpecialVendors.length);
				
				Vector localizedRaceTypes = new Vector();
				localizedRaceTypes.setSize(0);
				
				Vector modifiedRawRaceTypes = new Vector();
				modifiedRawRaceTypes.setSize(0);
				
				if (hiringMod >= 60)
				{
					blog("player_building.handleVendorTypeSelect: player hiringMod >= 60");
					
					String[] rawRaceTypesHiLvl = dataTableGetStringColumn(TBL_ALLNPC_TYPES, 0);
					if (rawRaceTypesHiLvl == null)
					{
						break;
					}
					
					for (int i = 0; i < rawRaceTypesHiLvl.length; i++)
					{
						testAbortScript();
						utils.addElement(localizedRaceTypes, "@player_structure:race_"+rawRaceTypesHiLvl[i]);
						utils.addElement(modifiedRawRaceTypes, rawRaceTypesHiLvl[i]);
					}
					
					utils.setScriptVar(self, "vendor.checkGender", 1);
				}
				else
				{
					
					String[] rawRaceTypesLowLvl = dataTableGetStringColumn(TBL_PLAYER_TYPES, 0);
					if (rawRaceTypesLowLvl == null)
					{
						break;
					}
					
					for (int i = 0; i < rawRaceTypesLowLvl.length; i++)
					{
						testAbortScript();
						utils.addElement(localizedRaceTypes, "@player_structure:race_"+rawRaceTypesLowLvl[i]);
						utils.addElement(modifiedRawRaceTypes, rawRaceTypesLowLvl[i]);
					}
				}
				
				if (allSpecialVendors != null)
				{
					
					for (int i = 0; i < allSpecialVendors.length; i++)
					{
						testAbortScript();
						utils.addElement(localizedRaceTypes, "@player_structure:race_"+allSpecialVendors[i]);
						utils.addElement(modifiedRawRaceTypes, allSpecialVendors[i]);
					}
					
					utils.setScriptVar(self, "vendor.checkGender", 1);
				}
				
				if (localizedRaceTypes == null || modifiedRawRaceTypes == null)
				{
					break;
				}
				
				utils.setScriptVar(player, "vendor.races", modifiedRawRaceTypes);
				sui.listbox(self, self, "@player_structure:race_type_d", sui.OK_CANCEL, "@player_structure:race_type_t", localizedRaceTypes, "handleVendorRaceSelect", true);
			}
			break;
			default: 
			String[] defaultTerminalTypes = dataTableGetStringColumn(TBL_TERMINAL_TYPES, 0);
			
			int hireVar = 10;
			Vector defaultVendorTypes = new Vector();
			defaultVendorTypes.setSize(0);
			for (int i=0; i < defaultTerminalTypes.length; i++)
			{
				testAbortScript();
				if (hiringMod >= hireVar)
				{
					vendorTypes = utils.addElement(defaultVendorTypes, "@player_structure:terminal_"+defaultTerminalTypes[i]);
				}
				hireVar += 20;
			}
			sui.listbox(player, player, "@player_structure:terminal_type_d", sui.OK_CANCEL, "@player_structure:terminal_type_t", defaultVendorTypes, "handleTerminalDesignSelect", true);
			break;
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int handleTerminalDesignSelect(obj_id self, dictionary params) throws InterruptedException
	{
		
		int idx = sui.getListboxSelectedRow(params);
		if (idx < 0)
		{
			idx = 0;
		}
		obj_id player = sui.getPlayerId(params);
		int btn = sui.getIntButtonPressed(params);
		if (btn == sui.BP_CANCEL)
		{
			removeVendorVars(self);
			return SCRIPT_CONTINUE;
		}
		
		String[] rawTerminalTypes = dataTableGetStringColumn(TBL_TERMINAL_TYPES, 0);
		String terminalsuffix = "terminal_"+ rawTerminalTypes[idx] + ".iff";
		
		utils.setScriptVar(player, "vendor.terminalSuffix", terminalsuffix);
		
		sui.inputbox(player, player, "@player_structure:name_d", sui.OK_CANCEL, "@player_structure:name_t", sui.INPUT_NORMAL, null, "handleSetVendorName", null);
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int handleDroidModelSelect(obj_id self, dictionary params) throws InterruptedException
	{
		
		int idx = sui.getListboxSelectedRow(params);
		if (idx < 0)
		{
			idx = 0;
		}
		obj_id player = sui.getPlayerId(params);
		int btn = sui.getIntButtonPressed(params);
		if (btn == sui.BP_CANCEL)
		{
			removeVendorVars(self);
			return SCRIPT_CONTINUE;
		}
		
		String[] rawDroidTypes = dataTableGetStringColumn(TBL_DROID_TYPES, 0);
		String droidsuffix = "droid_"+ rawDroidTypes[idx] + ".iff";
		
		utils.setScriptVar(player, "vendor.terminalSuffix", droidsuffix);
		
		sui.inputbox(player, player, "@player_structure:name_d", sui.OK_CANCEL, "@player_structure:name_t", sui.INPUT_NORMAL, null, "handleSetVendorName", null);
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int handleVendorRaceSelect(obj_id self, dictionary params) throws InterruptedException
	{
		blog("player_building.handleVendorRaceSelect: init");
		
		if (params == null || params.isEmpty())
		{
			removeVendorVars(self);
			return SCRIPT_CONTINUE;
		}
		
		int idx = sui.getListboxSelectedRow(params);
		if (idx < 0)
		{
			idx = 0;
		}
		obj_id player = sui.getPlayerId(params);
		int btn = sui.getIntButtonPressed(params);
		if (btn == sui.BP_CANCEL)
		{
			removeVendorVars(self);
			return SCRIPT_CONTINUE;
		}
		
		utils.setScriptVar(player, "vendor.raceIndex", idx);
		utils.setScriptVar(player, "vendor.terminalSuffix", "NPC");
		
		int checkGender = utils.getIntScriptVar(player, "vendor.checkGender");
		blog("player_building.handleVendorRaceSelect: checkGender: "+checkGender);
		if (checkGender == 1)
		{
			
			String[] raceTypes = utils.getStringArrayScriptVar(player, "vendor.races");
			String raceChosen = raceTypes[idx];
			blog("player_building.handleVendorRaceSelect: raceChosen: "+raceChosen);
			
			if (raceChosen.startsWith("special_vendor_"))
			{
				blog("player_building.handleVendorRaceSelect: Special Vendor = TRUE");
				
				int rowNum = dataTableSearchColumnForString(raceChosen, 0, TBL_SPECIAL_PLAYER_VENDOR_TYPES);
				if (rowNum < 0)
				{
					blog("player_building.handleVendorRaceSelect: rowNum is BAD");
					removeVendorVars(self);
					return SCRIPT_CONTINUE;
				}
				blog("player_building.handleVendorRaceSelect: rowNum: "+rowNum);
				
				dictionary dict = dataTableGetRow(TBL_SPECIAL_PLAYER_VENDOR_TYPES, rowNum);
				if (dict == null)
				{
					blog("player_building.handleVendorRaceSelect: Dictionary is NULL");
					
					removeVendorVars(self);
					return SCRIPT_CONTINUE;
				}
				blog("player_building.handleVendorRaceSelect: dict: "+dict);
				
				int vendorSpeaksBasic = dict.getInt("vendor_basic");
				if (vendorSpeaksBasic > 0)
				{
					utils.setScriptVar(player, "vendor.special_vendor_basic", 1);
				}
				else
				{
					utils.setScriptVar(player, "vendor.special_vendor_basic", 0);
				}
				
				blog("player_building.handleVendorRaceSelect: vendorSpeaksBasic: "+vendorSpeaksBasic);
				
				int vendor_clothing = dict.getInt("vendor_give");
				if (vendor_clothing > 0)
				{
					utils.setScriptVar(player, "vendor.special_vendor_clothing", 1);
				}
				else
				{
					utils.setScriptVar(player, "vendor.special_vendor_clothing", 0);
				}
				
				blog("player_building.handleVendorRaceSelect: vendor_clothing: "+vendor_clothing);
				
				int decr_skillmod = dict.getInt("decrement_mod");
				blog("player_building.handleVendorRaceSelect: decr_skillmod equals: "+decr_skillmod);
				
				if (decr_skillmod > 0)
				{
					blog("player_building.handleVendorRaceSelect: decrementing skillmod: "+decr_skillmod);
					
					String skillModName = dict.getString("skill_mod");
					blog("player_building.handleVendorRaceSelect: skillModName: "+skillModName);
					
					if (skillModName != null && !skillModName.equals(""))
					{
						blog("player_building.handleVendorRaceSelect: setting skill mod to decrement");
						
						utils.setScriptVar(player, "vendor.special_decrement_skillmod", true);
						utils.setScriptVar(player, "vendor.special_vendor_skillmod", skillModName);
					}
					
				}
				
				int specialVendorMale = dict.getInt("vendor_has_male");
				int specialVendorFemale = dict.getInt("vendor_has_female");
				if (specialVendorMale == 0 && specialVendorFemale == 0)
				{
					blog("player_building.handleVendorRaceSelect: both Male and Female == 0");
					
					removeVendorVars(self);
					return SCRIPT_CONTINUE;
				}
				
				if (specialVendorMale == 1 && specialVendorFemale == 0)
				{
					
					utils.setScriptVar(player, "vendor.genderIndex", 0);
					utils.setScriptVar(player, "vendor.gender.male_only_no_female", true);
					utils.setScriptVar(player, "vendor.gender.female_only_no_male", false);
				}
				else if (specialVendorMale == 0 && specialVendorFemale == 1)
				{
					blog("player_building.handleVendorRaceSelect:Female ONLY");
					
					utils.setScriptVar(player, "vendor.genderIndex", 1);
					utils.setScriptVar(player, "vendor.gender.female_only_no_male", true);
					utils.setScriptVar(player, "vendor.gender.male_only_no_female", false);
				}
				
			}
			else
			{
				blog("player_building.handleVendorRaceSelect: NOT A Special Vendor");
				
				int row = dataTableSearchColumnForString(raceChosen, "VENDOR_TYPES", TBL_ALLNPC_TYPES);
				dictionary dict = dataTableGetRow(TBL_ALLNPC_TYPES, row);
				int hasFemale = dict.getInt("HAS_FEMALE");
				blog("player_building.handleVendorRaceSelect: Has Female(0=no, 1=yes): "+hasFemale);
				if (hasFemale == 0)
				{
					blog("player_building.handleVendorRaceSelect: No female, no gender UI needed");
					
					utils.setScriptVar(player, "vendor.genderIndex", 0);
					utils.setScriptVar(player, "vendor.gender.male_only_no_female", true);
				}
				else
				{
					blog("player_building.handleVendorRaceSelect: Female exists, we'll need to decide which gender");
					
					utils.setScriptVar(player, "vendor.genderIndex", 1);
				}
			}
		}
		
		if (utils.getBooleanScriptVar(player, "vendor.gender.male_only_no_female") || utils.getBooleanScriptVar(player, "vendor.gender.female_only_no_male"))
		{
			blog("player_building.handleVendorRaceSelect: Either there is No female or No Male in this vendor");
			
			if (utils.getIntScriptVar(player, "vendor.genderIndex") == 0 || utils.getIntScriptVar(player, "vendor.genderIndex") == 1)
			{
				blog("player_building.handleVendorRaceSelect: Male only for this vendor");
				sui.inputbox(player, player, "@player_structure:name_d", sui.OK_CANCEL, "@player_structure:name_t", sui.INPUT_NORMAL, null, "handleSetVendorName", null);
				return SCRIPT_CONTINUE;
			}
		}
		
		utils.setScriptVar(player, "vendor.gender.female_only_no_male", false);
		utils.setScriptVar(player, "vendor.gender.male_only_no_female", false);
		blog("player_building.handleVendorRaceSelect: Setting female and male only to false");
		
		String[] possibleNPCGender =
		{
			"@player_structure:male", "@player_structure:female"
		};
		sui.listbox(player, player, "@player_structure:gender_d", sui.OK_CANCEL, "@player_structure:gender_t", possibleNPCGender, "handleVendorGenderSelect", true);
		return SCRIPT_CONTINUE;
	}
	
	
	public int handleVendorGenderSelect(obj_id self, dictionary params) throws InterruptedException
	{
		blog("player_building.handleVendorGenderSelect: init");
		
		if (params == null || params.isEmpty())
		{
			removeVendorVars(self);
			return SCRIPT_CONTINUE;
		}
		
		int idx = sui.getListboxSelectedRow(params);
		if (idx < 0)
		{
			idx = 0;
		}
		obj_id player = sui.getPlayerId(params);
		int btn = sui.getIntButtonPressed(params);
		if (btn == sui.BP_CANCEL)
		{
			removeVendorVars(self);
			return SCRIPT_CONTINUE;
		}
		
		utils.setScriptVar(player, "vendor.genderIndex", idx);
		blog("player_building.handleVendorGenderSelect: idx: "+idx);
		
		sui.inputbox(player, player, "@player_structure:name_d", sui.OK_CANCEL, "@player_structure:name_t", sui.INPUT_NORMAL, null, "handleSetVendorName", null);
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int handleSetVendorName(obj_id self, dictionary params) throws InterruptedException
	{
		blog("player_building.handleSetVendorName: init");
		
		if (params == null || params.isEmpty())
		{
			removeVendorVars(self);
			return SCRIPT_CONTINUE;
		}
		
		obj_id player = sui.getPlayerId(params);
		String vendorName = sui.getInputBoxText(params);
		int btn = sui.getIntButtonPressed(params);
		if (btn == sui.BP_CANCEL)
		{
			removeVendorVars(self);
			return SCRIPT_CONTINUE;
		}
		
		if ((vendorName.equals("")) || isNameReserved(vendorName))
		{
			
			sendSystemMessage(player, SID_OBSCENE);
			sui.inputbox(player, player, "@player_structure:name_d", sui.OK_CANCEL, "@player_structure:name_t", sui.INPUT_NORMAL, null, "handleSetVendorName", null);
			return SCRIPT_CONTINUE;
		}
		
		if (vendorName.length() > 40)
		{
			vendorName = vendorName.substring(0, 39);
		}
		
		blog("player_building.handleSetVendorName: vendorName: "+vendorName);
		
		utils.setScriptVar(player, "vendor.vendorName", vendorName);
		
		messageTo(player, "buildVendor", null, 0, false);
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int buildVendor(obj_id self, dictionary params) throws InterruptedException
	{
		blog("player_building.buildVendor: init");
		obj_id player = self;
		
		String terminalSuffix = utils.getStringScriptVar(player, "vendor.terminalSuffix");
		if (terminalSuffix == null || terminalSuffix.equals(""))
		{
			removeVendorVars(player);
			return SCRIPT_CONTINUE;
		}
		
		String vendorName = utils.getStringScriptVar(player, "vendor.vendorName");
		if (vendorName == null || vendorName.equals(""))
		{
			blog("player_building.buildVendor: vendorName was null");
			removeVendorVars(player);
			return SCRIPT_CONTINUE;
		}
		if (utils.isFreeTrial(player))
		{
			removeVendorVars(player);
			return SCRIPT_CONTINUE;
		}
		
		blog("player_building.buildVendor: vendor data initial validation pass");
		
		obj_id structure = player_structure.getStructure(player);
		obj_id inventory = getObjectInSlot(player, "inventory");
		if (!isIdValid(structure))
		{
			removeVendorVars(player);
			return SCRIPT_CONTINUE;
		}
		if (inventory == null || inventory == obj_id.NULL_ID)
		{
			removeVendorVars(player);
			return SCRIPT_CONTINUE;
		}
		
		obj_id vendor = null;
		if (terminalSuffix.equals("NPC"))
		{
			String[] genderList =
			{
				"male", "female"
			};
			String creatureName = "vendor";
			String templateName;
			
			int raceIndex = utils.getIntScriptVar(player, "vendor.raceIndex");
			if (raceIndex < 0)
			{
				removeVendorVars(player);
				return SCRIPT_CONTINUE;
			}
			
			int genderIndex = utils.getIntScriptVar(player, "vendor.genderIndex");
			if (genderIndex < 0 || genderIndex > 1)
			{
				genderIndex = 0;
			}
			
			if (raceIndex == 0)
			{
				String[] playerTypes = dataTableGetStringColumn(TBL_PLAYER_TYPES, 0);
				if (playerTypes == null)
				{
					removeVendorVars(player);
					return SCRIPT_CONTINUE;
				}
				
				templateName = playerTypes[rand(1,playerTypes.length-1)] + "_"+ genderList[genderIndex] + ".iff";
			}
			
			else
			{
				
				String[] raceTypes = utils.getStringArrayScriptVar(player, "vendor.races");
				if (raceTypes == null)
				{
					removeVendorVars(player);
					return SCRIPT_CONTINUE;
				}
				
				String specificRace = raceTypes[raceIndex];
				if (specificRace == null || specificRace.equals(""))
				{
					return SCRIPT_CONTINUE;
				}
				
				boolean maleOnly = utils.getBooleanScriptVar(player, "vendor.gender.male_only_no_female");
				boolean femaleOnly = utils.getBooleanScriptVar(player, "vendor.gender.female_only_no_male");
				
				if (maleOnly)
				{
					genderIndex = 0;
				}
				else if (femaleOnly)
				{
					genderIndex = 1;
				}
				
				templateName = specificRace + "_"+ genderList[genderIndex] + ".iff";
			}
			
			if (templateName == null || templateName.equals(""))
			{
				removeVendorVars(player);
				return SCRIPT_CONTINUE;
			}
			
			vendor = createObject(create.TEMPLATE_PREFIX + "vendor/"+ templateName, inventory, "");
			if (!isIdValid(vendor) || !exists(vendor))
			{
				removeVendorVars(player);
				return SCRIPT_CONTINUE;
			}
			
			obj_id inv = getObjectInSlot(self, "inventory");
			if ((inv == null) || (inv == obj_id.NULL_ID))
			{
				sendSystemMessage(player, SID_SYS_CREATE_FAILED);
				destroyObject(vendor);
				removeVendorVars(self);
				return SCRIPT_CONTINUE;
			}
			
			if ((vendor != null))
			{
				
				setInvulnerable(vendor, true);
				ai_lib.setDefaultCalmBehavior(vendor, ai_lib.BEHAVIOR_SENTINEL);
				setObjVar(vendor, "vendor.NPC", 1);
				
				detachAllScripts(vendor);
				attachScript(vendor, "terminal.vendor");
				attachScript(vendor, "terminal.npc_vendor");
				
				boolean specialVendor = templateName.startsWith("special_vendor_");
				
				if (specialVendor)
				{
					
					sendSystemMessage(player, SID_TCG_VENDOR_CTS_WARNING);
					
					setObjVar(vendor, vendor_lib.SPECIAL_VENDOR_IDENTIFIER, true);
					
					if (utils.hasScriptVar(player, "vendor.special_vendor_basic"))
					{
						setObjVar(vendor, "vendor.special_vendor_basic", utils.getIntScriptVar(player, "vendor.special_vendor_basic"));
					}
					
					if (utils.hasScriptVar(player, "vendor.special_vendor_clothing"))
					{
						setObjVar(vendor, "vendor.special_vendor_clothing", utils.getIntScriptVar(player, "vendor.special_vendor_clothing"));
					}
					
					if (utils.hasScriptVar(player, "vendor.special_decrement_skillmod") && utils.hasScriptVar(player, "vendor.special_vendor_skillmod"))
					{
						String skillMod = utils.getStringScriptVar(player, "vendor.special_vendor_skillmod");
						if (skillMod == null || skillMod.equals(""))
						{
							sendSystemMessage(player, SID_SYS_CREATE_FAILED);
							destroyObject(vendor);
							removeVendorVars(self);
							return SCRIPT_CONTINUE;
						}
						
						setObjVar(vendor, "vendor.special_decrement_skillmod", true);
						setObjVar(vendor, "vendor.special_vendor_skillmod", skillMod);
						
						CustomerServiceLog("vendor", "Vendor requires a skill mod: "+skillMod+" that is being decremented on the owner. Owner: "+ player + " Vendor: "+ vendor + " Location created is player inventory");
						if (getSkillStatMod(self, skillMod) <= 0)
						{
							CustomerServiceLog("vendor", " POSSIBLE EXPLOIT!!! Vendor: "+ vendor + " had skill mod: "+ skillMod + " that was supposed to be decremented on Owner: "+ player + " but the owner did not have the skill mod. Possible exploit or other problematic issue. Notify design.");
							sendSystemMessage(player, SID_SYS_CREATE_FAILED);
							destroyObject(vendor);
							removeVendorVars(self);
							return SCRIPT_CONTINUE;
						}
						applySkillStatisticModifier(player, skillMod, -1);
						CustomerServiceLog("vendor", " Vendor: "+ self + " had skill mod: "+ skillMod + " that was reimbursed to the Owner: "+ player + ". Skill mod decremented by 1. Location created is player inventory");
					}
				}
				else if (templateName.indexOf("ithorian") > -1)
				{
					dressup.dressNpc(vendor, "random_ithorian", true);
					setObjVar(vendor, "dressed", 1);
				}
				else
				{
					dressup.dressNpc(vendor, "rich_no_jacket");
				}
			}
			else
			{
				sendSystemMessage(player, SID_SYS_CREATE_FAILED);
				destroyObject(vendor);
				removeVendorVars(self);
				return SCRIPT_CONTINUE;
			}
		}
		else
		{
			
			String vendorTemplate = "object/tangible/vendor/vendor_"+ terminalSuffix;
			if (vendorTemplate == null || vendorTemplate.equals(""))
			{
				CustomerServiceLog("playerStructure", "Vendor Placement: Player: "+getName(self)+" "+self+" selected vendor type: "+terminalSuffix+" but the selection type failed.");
				return SCRIPT_CONTINUE;
			}
			vendor = createObject(vendorTemplate, inventory, "");
		}
		if (!isIdValid(vendor))
		{
			sendSystemMessage(player, SID_SYS_CREATE_FAILED);
			removeVendorVars(self);
			return SCRIPT_CONTINUE;
		}
		
		persistObject(vendor);
		setCondition(vendor, CONDITION_VENDOR);
		setName(vendor, "Vendor: "+ vendorName);
		
		int usedSlots = getIntObjVar(player, "used_vendor_slots") + 1;
		setObjVar(player, "used_vendor_slots", usedSlots);
		setObjVar(vendor, "vendor_owner", player);
		setOwner(vendor, player);
		setObjVar(player, "vendor_not_initialized", vendor);
		removeVendorVars(self);
		
		int condition = 1000;
		int items_stored = 0;
		setObjVar(vendor, vendor_lib.VAR_MAINTENANCE_RATE, vendor_lib.BASE_MAINT_RATE);
		setObjVar(vendor, vendor_lib.VAR_DECAY_RATE, vendor_lib.BASE_DECAY_RATE);
		if (condition > 0)
		{
			setObjVar(vendor, vendor_lib.VAR_MAX_CONDITION, 1000);
			setObjVar(vendor, vendor_lib.VAR_CONDITION, 1000);
		}
		
		int time_stamp = getGameTime();
		setObjVar(vendor, vendor_lib.VAR_LAST_MAINTANENCE, time_stamp);
		dictionary outparams = new dictionary();
		outparams.put("timestamp", time_stamp);
		
		messageTo(vendor, "OnMaintenanceLoop", outparams, vendor_lib.MAINTENANCE_HEARTBEAT, false);
		
		sendSystemMessage(player, SID_SYS_CREATE_SUCCESS);
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int msgAssignDroid(obj_id self, dictionary params) throws InterruptedException
	{
		LOG("LOG_CHANNEL", "player_building::msgAssignDroid --"+ self);
		
		obj_id[] droid_list = player_structure.getMaintenanceDroids(self);
		
		if (droid_list.length <= 0)
		{
			LOG("LOG_CHANNEL", "Error retrieving your list of maintenance capable droids.");
			sendSystemMessage(self, new string_id(STF, "error_droid_list"));
			return SCRIPT_CONTINUE;
		}
		
		int bp = sui.getIntButtonPressed(params);
		
		if (bp == sui.BP_REVERT)
		{
			return SCRIPT_CONTINUE;
		}
		else if (bp == sui.BP_OK)
		{
			
			int row_selected = sui.getListboxSelectedRow(params);
			
			if (row_selected != -1)
			{
				int max_maint = (((getIntObjVar(droid_list[row_selected], "module_data.struct_maint") - 1) / 5) + 1) * 3;
				obj_id[] maint_list = getObjIdArrayObjVar(droid_list[row_selected], "module_data.maint_list.ids");
				
				if (maint_list == null || maint_list.length == 0)
				{
					maint_list = new obj_id[max_maint];
				}
				
				obj_id structure = player_structure.getStructure(self);
				
				if (!isIdValid(structure))
				{
					return SCRIPT_CONTINUE;
				}
				
				for (int i = 0; i < maint_list.length; i++)
				{
					testAbortScript();
					if (maint_list[i] == structure)
					{
						sendSystemMessage(self, new string_id(STF, "structure_on_list"));
						return SCRIPT_CONTINUE;
					}
					
					if (!isIdValid(maint_list[i]))
					{
						maint_list[i] = structure;
						setObjVar(droid_list[row_selected], "module_data.maint_list.ids", maint_list);
						setObjVar(droid_list[row_selected], "module_data.maint_list.loc_"+i, getLocation(structure));
						
						if (callable.hasCDCallable(droid_list[row_selected]))
						{
							obj_id droid = callable.getCDCallable(droid_list[row_selected]);
							
							setObjVar(droid, "module_data.maint_list.ids", maint_list);
							setObjVar(droid, "module_data.maint_list.loc_"+i, getLocation(structure));
						}
						
						String structName = "";
						
						prose_package ppDroidAssigned = prose.getPackage(SID_DROID_ASSIGNED_TO_MAINTAIN);
						prose.setTT(ppDroidAssigned, droid_list[row_selected]);
						
						if (!player_structure.isInstallation(structure) && hasObjVar(structure, player_structure.VAR_SIGN_NAME))
						{
							prose.setTO(ppDroidAssigned, getStringObjVar(structure, player_structure.VAR_SIGN_NAME));
						}
						else
						{
							prose.setTO(ppDroidAssigned, structure);
						}
						
						sendSystemMessageProse(self, ppDroidAssigned);
						
						return SCRIPT_CONTINUE;
					}
				}
				
				sendSystemMessage(self, new string_id(STF, "droid_full"));
			}
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int turnstileExpire(obj_id self, dictionary params) throws InterruptedException
	{
		if (params == null)
		{
			return SCRIPT_CONTINUE;
		}
		
		obj_id building = params.getObjId("building");
		if (isIdValid(building))
		{
			obj_id cont = getTopMostContainer(self);
			if (cont == building)
			{
				params.put("sender", building);
				messageTo(self, "handleEjection", params, 1.f, false);
				sendSystemMessage(self, SID_TURNSTILE_EXPIRE);
			}
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int msgEnterPermissions(obj_id self, dictionary params) throws InterruptedException
	{
		LOG("LOG_CHANNEL", "player_building::msgEnterPermissions--"+ params);
		int row_selected = sui.getListboxSelectedRow(params);
		obj_id structure = player_structure.getStructure(self);
		
		if ((structure == null)||(structure == obj_id.NULL_ID))
		{
			return SCRIPT_CONTINUE;
		}
		
		if (row_selected != -1)
		{
			String[] enter_list;
			if (isGod(self))
			{
				enter_list = player_structure.getCompleteEntryList(structure);
			}
			else
			{
				enter_list = player_structure.getEntryList(structure);
			}
			String player_name = enter_list[row_selected];
			player_structure.modifyEntryList(structure, player_name, self);
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int msgBanPermissions(obj_id self, dictionary params) throws InterruptedException
	{
		LOG("LOG_CHANNEL", "player_building::msgBanPermissions--"+ params);
		int row_selected = sui.getListboxSelectedRow(params);
		obj_id structure = player_structure.getStructure(self);
		
		if ((structure == null)||(structure == obj_id.NULL_ID))
		{
			return SCRIPT_CONTINUE;
		}
		
		if (row_selected != -1)
		{
			String[] ban_list = player_structure.getBanList(structure);
			String player_name = ban_list[row_selected];
			player_structure.modifyBanList(structure, player_name, self);
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int msgAdminPermissions(obj_id self, dictionary params) throws InterruptedException
	{
		LOG("LOG_CHANNEL", "player_building::msgAdminPermissions--"+ params);
		int row_selected = sui.getListboxSelectedRow(params);
		obj_id structure = player_structure.getStructure(self);
		
		if ((structure == null)||(structure == obj_id.NULL_ID))
		{
			return SCRIPT_CONTINUE;
		}
		
		if (row_selected != -1)
		{
			String[] admin_list = player_structure.getAdminListNames(structure);
			String player = admin_list[row_selected];
			player_structure.modifyAdminList(structure, player, self);
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int msgVendorPermissions(obj_id self, dictionary params) throws InterruptedException
	{
		LOG("LOG_CHANNEL", "player_building::msgVendorPermissions--"+ params);
		int row_selected = sui.getListboxSelectedRow(params);
		obj_id structure = player_structure.getStructure(self);
		
		if ((structure == null)||(structure == obj_id.NULL_ID))
		{
			return SCRIPT_CONTINUE;
		}
		
		if (row_selected != -1)
		{
			String[] vendor_list = player_structure.getVendorList(structure);
			String player_name = vendor_list[row_selected];
			player_structure.modifyVendorList(structure, player_name, self);
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int msgHopperPermissions(obj_id self, dictionary params) throws InterruptedException
	{
		LOG("LOG_CHANNEL", "player_building::msgHopperPermissions--"+ params);
		int row_selected = sui.getListboxSelectedRow(params);
		obj_id structure = player_structure.getStructure(self);
		
		if ((structure == null)||(structure == obj_id.NULL_ID))
		{
			return SCRIPT_CONTINUE;
		}
		
		if (row_selected != -1)
		{
			String[] hopper_list = player_structure.getHopperListNames(structure);
			String player = hopper_list[row_selected];
			player_structure.modifyHopperList(structure, player, self);
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int msgPayMaintenance(obj_id self, dictionary params) throws InterruptedException
	{
		LOG("LOG_CHANNEL", "player_building::msgPayMaintenance --"+ self);
		obj_id target = utils.getObjIdScriptVar(self, "payMaintenance.target");
		boolean fromScd = pclib.isContainedByPlayer(self, target);
		
		utils.removeScriptVarTree(self, "payMaintenance");
		
		if (!isIdValid(target))
		{
			return SCRIPT_CONTINUE;
		}
		
		if (!fromScd && player_structure.getStructure(self) != target)
		{
			sendSystemMessage(self, new string_id(STF, "pay_out_of_range"));
			return SCRIPT_CONTINUE;
		}
		
		int bp = sui.getIntButtonPressed(params);
		if (bp == sui.BP_CANCEL)
		{
			return SCRIPT_CONTINUE;
		}
		
		int amt = sui.getTransferInputTo(params);
		if (amt < 1)
		{
			return SCRIPT_CONTINUE;
		}
		
		if (fromScd)
		{
			queueCommand(self, (-404530384), target, Integer.toString(amt), COMMAND_PRIORITY_DEFAULT);
		}
		else
		{
			queueCommand(self, (-404530384), null, Integer.toString(amt), COMMAND_PRIORITY_DEFAULT);
		}
		return SCRIPT_CONTINUE;
	}
	
	
	public int msgAddPower(obj_id self, dictionary params) throws InterruptedException
	{
		LOG("LOG_CHANNEL", "player_building::msgAddPower --"+ self);
		obj_id target = utils.getObjIdScriptVar(self, "addPower.target");
		utils.removeScriptVarTree(self, "addPower");
		
		if (isIdValid(target) && player_structure.getStructure(self) != target)
		{
			sendSystemMessage(self, new string_id(STF, "power_out_of_range"));
			return SCRIPT_CONTINUE;
		}
		
		int bp = sui.getIntButtonPressed(params);
		if (bp == sui.BP_CANCEL)
		{
			return SCRIPT_CONTINUE;
		}
		
		int amt = sui.getTransferInputTo(params);
		if (amt < 1)
		{
			return SCRIPT_CONTINUE;
		}
		
		queueCommand(self, (-1893504550), null, Integer.toString(amt), COMMAND_PRIORITY_DEFAULT);
		return SCRIPT_CONTINUE;
	}
	
	
	public int msgDepositPowerIncubator(obj_id self, dictionary params) throws InterruptedException
	{
		obj_id target = utils.getObjIdScriptVar(self, "depositPowerIncubator.target");
		utils.removeScriptVarTree(self, "depositPowerIncubator");
		
		if (isIdValid(target) && !incubator.validateActiveUser(target, self))
		{
			
			sendSystemMessage(self, incubator.SID_NOT_YOUR_INCUBATOR);
			return SCRIPT_CONTINUE;
		}
		
		int bp = sui.getIntButtonPressed(params);
		if (bp == sui.BP_CANCEL)
		{
			
			utils.removeScriptVarTree(self, "powerIncubator");
			return SCRIPT_CONTINUE;
		}
		
		int amt = sui.getTransferInputTo(params);
		if (amt < 1)
		{
			return SCRIPT_CONTINUE;
		}
		
		queueCommand(self, (1833062743), target, Integer.toString(amt), COMMAND_PRIORITY_DEFAULT);
		return SCRIPT_CONTINUE;
	}
	
	
	public int msgWithdrawMaintenance(obj_id self, dictionary params) throws InterruptedException
	{
		LOG("LOG_CHANNEL", "player_building::msgWithdrawMaintenance --"+ self);
		
		String amount_str = sui.getInputBoxText(params);
		
		if (utils.stringToInt(amount_str) > 0)
		{
			queueCommand(self, (-707821905), null, amount_str, COMMAND_PRIORITY_DEFAULT);
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int msgNameStructure(obj_id self, dictionary params) throws InterruptedException
	{
		LOG("LOG_CHANNEL", "player_building::msgNameStructure --"+ self);
		
		int bp = sui.getIntButtonPressed(params);
		if (bp == sui.BP_CANCEL)
		{
			return SCRIPT_CONTINUE;
		}
		
		String name = sui.getInputBoxText(params);
		if (name == null || name.length() < 1)
		{
			LOG("LOG_CHANNEL", self + " ->That is not a valid name.");
			sendSystemMessage(self, new string_id(STF, "not_valid_name"));
			return SCRIPT_CONTINUE;
		}
		
		queueCommand(self, (-1016613791), null, name, COMMAND_PRIORITY_DEFAULT);
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int OnAddStructure(obj_id self, dictionary params) throws InterruptedException
	{
		LOG("LOG_CHANNEL", "player_building::OnAddStructure");
		
		obj_id structure = params.getObjId("structure");
		String template = params.getString("template");
		String structure_name = params.getString("structure_name");
		location structure_loc = params.getLocation("structure_loc");
		
		LOG("LOG_CHANNEL", "structure_name ->"+ structure_name);
		
		player_structure.addStructure(structure, template, structure_name, structure_loc, self, getFirstName(self), false, true);
		
		if (structure_loc != null)
		{
			player_structure.addStructureWaypoint(self, structure_loc, structure_name, structure);
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int OnRemoveStructure(obj_id self, dictionary params) throws InterruptedException
	{
		LOG("LOG_CHANNEL", "player_building::OnRemoveStructure");
		
		obj_id structure = params.getObjId("structure");
		String template = params.getString("template");
		obj_id waypoint = params.getObjId("waypoint");
		
		player_structure.removeStructure(structure, template, self, getFirstName(self), false, true);
		
		if (waypoint != null)
		{
			destroyWaypointInDatapad(waypoint, self);
		}
		
		obj_id[] callables = callable.getDatapadCallablesByType(self, callable.CALLABLE_TYPE_COMBAT_PET);
		
		if (callables == null || callables.length <= 0)
		{
			return SCRIPT_CONTINUE;
		}
		
		for (int i = 0, j = callables.length; i < j; i++)
		{
			testAbortScript();
			if (!isIdValid(callables[i]) || !exists(callables[i]))
			{
				continue;
			}
			
			if (!hasObjVar(callables[i], "module_data.maint_list.ids"))
			{
				continue;
			}
			
			obj_id[] struct_list = getObjIdArrayObjVar(callables[i], "module_data.maint_list.ids");
			
			if (struct_list == null || struct_list.length <= 0)
			{
				continue;
			}
			
			int structureIndex = utils.getElementPositionInArray(struct_list, structure);
			
			if (structureIndex < 0)
			{
				continue;
			}
			
			obj_id droid = callable.getCDCallable(callables[i]);
			
			player_structure.removeStructureFromMaintenance(droid, callables[i], structureIndex);
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int handleAdjustLotCount(obj_id self, dictionary params) throws InterruptedException
	{
		int numLots = params.getInt("lotAdjust");
		adjustLotCount(getPlayerObject(self), numLots);
		return SCRIPT_CONTINUE;
	}
	
	
	public int OnClearConfirmDestroy(obj_id self, dictionary params) throws InterruptedException
	{
		LOG("LOG_CHANNEL", "player_building::OnClearConfirmDestroy");
		if (utils.hasScriptVar(self, player_structure.VAR_CONFIRM_DESTROY))
		{
			utils.removeScriptVar(self, player_structure.VAR_CONFIRM_DESTROY);
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int OnConstructionComplete(obj_id self, dictionary params) throws InterruptedException
	{
		
		string_id structure_name = params.getStringId("structure_name");
		int lots_remaining = player_structure.MAX_LOTS - getAccountNumLots(getPlayerObject(self));
		
		prose_package pp_msg = new prose_package();
		pp_msg.stringId = ((lots_remaining >= 0) ? SID_CONSTRUCTION_COMPLETE : SID_CONSTRUCTION_COMPLETE_LOT_LIMIT_EXCEEDED);
		pp_msg.actor.set(self);
		pp_msg.other.set(structure_name);
		pp_msg.digitInteger = ((lots_remaining >= 0) ? lots_remaining : -lots_remaining);
		
		String send_msg = "@"+ SID_CONSTRUCTION_COMPLETE_SENDER.toString ();
		String subject_str = "@"+ SID_CONSTRUCTION_COMPLETE_SUBJECT.toString ();
		String body_msg = chatMakePersistentMessageOutOfBandBody (null, pp_msg);
		chatSendPersistentMessage(send_msg, getFirstName(self), subject_str, null, body_msg);
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int msgPAWithdrawSuccess(obj_id self, dictionary params) throws InterruptedException
	{
		
		if (params != null)
		{
			int amt = params.getInt("amount");
			if (amt > 0)
			{
				sendSystemMessageProse(self, prose.getPackage(new string_id (STF, "withdraw_credits"), amt));
			}
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int msgPAWithdrawFail(obj_id self, dictionary params) throws InterruptedException
	{
		sendSystemMessage(self, new string_id(STF, "withdrawal_failed"));
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int handleStatusUi(obj_id self, dictionary params) throws InterruptedException
	{
		obj_id target = utils.getObjIdScriptVar(self, "player_structure.status.target");
		boolean fromScd = utils.hasScriptVar(self, "player_structure.status.fromScd");
		utils.removeScriptVarTree(self, "player_structure.status");
		
		obj_id newTarget = fromScd? target : player_structure.getStructure(self);
		int bp = sui.getIntButtonPressed(params);
		if (bp == sui.BP_OK)
		{
			if (!isIdValid(target))
			{
				sendSystemMessage(self, new string_id(STF, "no_valid_structurestatus"));
			}
			else
			{
				if (newTarget == target)
				{
					queueCommand(self, (335013253), fromScd? newTarget : null, "", COMMAND_PRIORITY_DEFAULT);
				}
				else
				{
					sendSystemMessage(self, new string_id(STF, "changed_structurestatus"));
				}
			}
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int handleDestroyUi(obj_id self, dictionary params) throws InterruptedException
	{
		utils.removeScriptVar(self, "player_structure.destroy.pid");
		
		int bp = sui.getIntButtonPressed(params);
		if (bp == sui.BP_CANCEL)
		{
			utils.removeScriptVarTree(self, "player_structure.destroy");
			return SCRIPT_CONTINUE;
		}
		
		obj_id target = utils.getObjIdScriptVar(self, "player_structure.destroy.target");
		obj_id structure = player_structure.getStructure(self);
		if (!confirmDestroyTarget(structure, target))
		{
			return SCRIPT_CONTINUE;
		}
		
		string_id confirm_destruction_title = new string_id(STF, "confirm_destruction_t");
		String title = getString(confirm_destruction_title);
		
		string_id redeed_structure_prefix = new string_id(STF, "your_structure_prefix");
		String prompt = getString(redeed_structure_prefix) + "\\";
		
		boolean willRedeed = utils.getBooleanScriptVar(self, "player_structure.destroy.willredeed");
		if (willRedeed)
		{
			string_id will_redeed_confirm_text = new string_id(STF, "will_redeed_confirm");
			prompt += colors_hex.LIMEGREEN+getString(will_redeed_confirm_text);
		}
		else
		{
			string_id will_not_redeed_confirm_text = new string_id(STF, "will_not_redeed_confirm");
			prompt += colors_hex.TOMATO+getString(will_not_redeed_confirm_text);
		}
		
		prompt += "\\#.";
		string_id will_redeed_confirm_suffix_text = new string_id(STF, "will_redeed_suffix");
		prompt += getString(will_redeed_confirm_suffix_text);
		
		int key = utils.getIntScriptVar(self, "player_structure.destroy.key");
		if (key == 0)
		{
			key = rand(100000, 999999);
			utils.setScriptVar(self, "player_structure.destroy.key", key);
		}
		
		prompt += "\n\nCode: "+key;
		
		int pid = sui.inputbox(self, self, prompt, title, "handleDestroyConfirm", 6, false, "");
		if (pid > -1)
		{
			utils.setScriptVar(self, "player_structure.destroy.pid", pid);
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int handleDestroyConfirm(obj_id self, dictionary params) throws InterruptedException
	{
		utils.removeScriptVar(self, "player_structure.destroy.pid");
		
		int bp = sui.getIntButtonPressed(params);
		String text = sui.getInputBoxText(params);
		if (bp == sui.BP_CANCEL || text == null || text.equals(""))
		{
			utils.removeScriptVarTree(self, "player_structure.destroy");
			return SCRIPT_CONTINUE;
		}
		
		obj_id target = utils.getObjIdScriptVar(self, "player_structure.destroy.target");
		obj_id structure = player_structure.getStructure(self);
		if (!confirmDestroyTarget(structure, target))
		{
			return SCRIPT_CONTINUE;
		}
		
		int key = utils.getIntScriptVar(self, "player_structure.destroy.key");
		String skey = Integer.toString(key);
		
		if (text.equals(skey))
		{
			if (tcg.isBarnStructure(structure) && tcg.getTotalBarnStoredBeasts(structure) > 0)
			{
				String title = utils.packStringId(new string_id("tcg", "barn_confirm_destroy_with_beasts_title"));
				String testMsg = utils.packStringId(new string_id("tcg", "barn_confirm_destroy_with_beasts_prompt"));
				String ok_button = utils.packStringId(new string_id ("quest/ground/util/quest_giver_object", "button_accept"));
				String cancel_button = utils.packStringId(new string_id ("quest/ground/util/quest_giver_object", "button_decline"));
				
				int pid = sui.createSUIPage(sui.SUI_MSGBOX, self, self, "handleDestroyBarnAndBeastsConfirm");
				
				setSUIProperty(pid, sui.MSGBOX_TITLE, sui.PROP_TEXT, title);
				setSUIProperty(pid, sui.MSGBOX_PROMPT, sui.PROP_TEXT, testMsg);
				
				sui.msgboxButtonSetup(pid, sui.OK_ONLY);
				
				utils.setScriptVar(self, "player_structure.destroy.barn_text", text);
				sui.showSUIPage(pid);
			}
			else
			{
				utils.setScriptVar(self, player_structure.VAR_CONFIRM_DESTROY, target);
				sendSystemMessage(self, new string_id(STF, "processing_destruction"));
				queueCommand(self, (419174182), null, text, COMMAND_PRIORITY_DEFAULT);
			}
		}
		else
		{
			sui.msgbox(self, "@player_structure:incorrect_destroy_code");
			utils.removeScriptVarTree(self, "player_structure.destroy");
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int handleDestroyBarnAndBeastsConfirm(obj_id self, dictionary params) throws InterruptedException
	{
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int handlePlayerStructurePackupLockout(obj_id self, dictionary params) throws InterruptedException
	{
		
		if (!params.containsKey("house"))
		{
			blog("player_buidling.handlePlayerStructurePackupLockout() PARAMS MISSING, ABORTING.");
			return SCRIPT_CONTINUE;
		}
		
		obj_id structure = params.getObjId ("house");
		if (!isValidId(structure))
		{
			blog("player_buidling.handlePlayerStructurePackupLockout() Invalid Structure OID, ABORTING.");
			return SCRIPT_CONTINUE;
		}
		
		blog("player_buidling.packedAbandonedStructure() Message received by player to start lockout timer.");
		
		utils.setScriptVar(self, player_structure.SCRIPTVAR_HOUSE_PACKUP_LOCKOUT_TIME, getGameTime());
		messageTo(self, "handlePlayerStructurePackupLockoutRemoval", null, player_structure.HOUSE_PACKUP_LOCKOUT_TIMER, false);
		
		utils.setScriptVar(structure, player_structure.SCRIPTVAR_HOUSE_PACKUP_LOCKOUT_TIME, getGameTime());
		messageTo(structure, "handleStructurePackupLockoutRemoval", params, player_structure.HOUSE_PACKUP_LOCKOUT_TIMER, false);
		
		if (params != null && !params.equals(""))
		{
			CustomerServiceLog("housepackup", "Player: "+ getPlayerName(self) + " ("+ self+ ") is" + " attempting to pack up a structure and has received lockout" + " timer for packup event that will expire in " + player_structure.HOUSE_PACKUP_LOCKOUT_TIMER + "." );
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int handlePlayerStructurePackupLockoutRemoval(obj_id self, dictionary params) throws InterruptedException
	{
		blog("player_buidling.handlePlayerStructurePackupLockoutRemoval() Message received by player to remove lockout timer.");
		
		utils.removeScriptVarTree(self, player_structure.SCRIPTVAR_HOUSE_PACKUP_TREE_PREFIX);
		return SCRIPT_CONTINUE;
	}
	
	
	public int handleFailedStructurePackup(obj_id self, dictionary params) throws InterruptedException
	{
		blog("player_buidling.handleFailedStructurePackup() Message received by player to handle failed packup.");
		
		messageTo(self, "handlePlayerStructurePackupLockoutRemoval", null, 0, false);
		
		if (player_structure.canPackMoreToday(self))
		{
			sendSystemMessage(self, SID_HOUSE_PACKUP_FAILED);
		}
		
		CustomerServiceLog("housepackup", "Player: "+ getPlayerName(self) + " ("+ self+ ") has" + " had lockout timer removed early due to structure packup problems." );
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int callAirStrikePackAbandonedStructure(obj_id self, dictionary params) throws InterruptedException
	{
		blog("player_buidling.callAirStrikePackAbandonedStructure() Message received by player to send animation, tally success.");
		
		if ((params == null) || (params.isEmpty()))
		{
			blog("player_buidling.callAirStrikePackAbandonedStructure() ALL PARAMS MISSING, ABORTING.");
			messageTo(self, "handlePlayerStructurePackupLockoutRemoval", null, 0, false);
			return SCRIPT_CONTINUE;
		}
		
		if (!params.containsKey("structure") || !params.containsKey("structure_owner"))
		{
			blog("player_buidling.callAirStrikePackAbandonedStructure() PARAMS MISSING, ABORTING.");
			messageTo(self, "handlePlayerStructurePackupLockoutRemoval", null, 0, false);
			return SCRIPT_CONTINUE;
		}
		
		blog("player_buidling.callAirStrikePackAbandonedStructure() about to send animation request.");
		
		obj_id structure = params.getObjId ("structure");
		if (!isValidId(structure))
		{
			blog("player_building.moveStructureToSCD() MISSING PARAMS. ABORTING AIRSTRIKE");
			messageTo(self, "handlePlayerStructurePackupLockoutRemoval", null, 0, false);
			return SCRIPT_CONTINUE;
		}
		
		location where = getLocation(structure);
		if (player_structure.canPackMoreToday(self))
		{
			prose_package p = prose.getPackage(new string_id("spam", "house_packup_airstrike"));
			commPlayers(self, "object/mobile/dressed_hiddendagger_pilot_m_01.iff", "sound/sys_comm_other.snd", 5f, self, p);
			playClientEffectLoc(self, "clienteffect/house_packup_airstrike.cef", where, 0);
		}
		
		params.put("player", self);
		messageTo (structure, "moveStructureToSCD", params, 3, false);
		
		blog("player_buidling.callAirStrikePackAbandonedStructure() The animation will eventually play.");
		return SCRIPT_CONTINUE;
	}
	
	
	public int msgConfirmHousePackup(obj_id self, dictionary params) throws InterruptedException
	{
		if (!utils.hasScriptVar(self, "packup.structure"))
		{
			return SCRIPT_CONTINUE;
		}
		
		obj_id structure = utils.getObjIdScriptVar(self, "packup.structure");
		utils.removeScriptVar(self, "packup.structure");
		
		int bp = sui.getIntButtonPressed(params);
		if (bp == sui.BP_CANCEL)
		{
			LOG("house", "Declined packup.");
			return SCRIPT_CONTINUE;
		}
		
		if (!player_structure.canPackBuilding(self, structure))
		{
			return SCRIPT_CONTINUE;
		}
		
		if (!player_structure.canPackStructureWithVendors(self, structure))
		{
			return SCRIPT_CONTINUE;
		}
		
		location structLoc = getLocation(structure);
		location playerLoc = getLocation(self);
		if (structLoc == null || playerLoc == null)
		{
			return SCRIPT_CONTINUE;
		}
		if (getDistance(structLoc, playerLoc) > 10.0f)
		{
			sendSystemMessage(self, SID_PACKUP_TOO_FAR_AWAY);
			return SCRIPT_CONTINUE;
		}
		
		params.put("structure", structure);
		messageTo(self, "msgFinalizePackup", params, 0, false);
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int msgFinalizePackup(obj_id self, dictionary params) throws InterruptedException
	{
		blog("player_building.msgFinalizePackup() init!");
		
		obj_id structure = params.getObjId("structure");
		blog("player_building.msgFinalizePackup() STRUCTURE CONTENTS LOADED? "+areAllContentsLoaded(structure));
		
		blog("player_building.msgFinalizePackup() STRUCTURE LOADED? "+structure.isLoaded());
		
		if (!isIdValid(structure))
		{
			return SCRIPT_CONTINUE;
		}
		
		if (!structure.isLoaded() || !structure.isAuthoritative())
		{
			blog("player_building.msgFinalizePackup() STRUCTURE LOADED? "+structure.isLoaded());
			blog("player_building.msgFinalizePackup() STRUCTURE CONTENTS LOADED? "+areAllContentsLoaded(structure));
			
			CustomerServiceLog("housepackup", "Structure ("+ structure + ") could not be packed by player (" + getPlayerName(self) + ") msgFinalizePackup -" + " isLoaded = "+ structure.isLoaded()
			+ " isAuthoritative = "+ structure.isAuthoritative()
			);
			
			return SCRIPT_CONTINUE;
		}
		
		if (!player_structure.isInstallation(structure) && !areAllContentsLoaded(structure) && !hasObjVar(structure, "structure.player.pack"))
		{
			
			setObjVar(structure, "structure.player.pack", self);
			
			loadBuildingContents(self, structure);
			return SCRIPT_CONTINUE;
		}
		
		if (!hasObjVar(structure, "structure.player.pack"))
		{
			blog("player_building.msgFinalizePackup() STRUCTURE IS GOOD FOR PACKUP");
			player_structure.finalizePackUp(self, structure);
		}
		return SCRIPT_CONTINUE;
	}
	
	
	public int packUpCityAbandonedStructure(obj_id self, dictionary params) throws InterruptedException
	{
		if ((params == null) || (params.isEmpty()))
		{
			return SCRIPT_CONTINUE;
		}
		
		LOG("sissynoid", "Entered: packUpCityAbandonedStructure");
		obj_id structure = params.getObjId("house");
		
		if (!isIdValid(structure))
		{
			return SCRIPT_CONTINUE;
		}
		if (!isIdValid(self))
		{
			return SCRIPT_CONTINUE;
		}
		
		if (!player_structure.isCityPackupAuthoritative(self, structure))
		{
			LOG("sissynoid", "packUpCityAbandonedStructure: Structure is not Loaded or Authoritative: ");
			CustomerServiceLog("city_house_packup",
			"'packUpCityAbandonedStructure' - Structure ("+ structure +
			") could not be packed by player (" + getPlayerName(self) + ") player -"+
			" isLoaded(Player) = "+ self.isLoaded() +
			" isAuthoritative(Player) = "+ self.isAuthoritative() +
			" isLoaded(Structure) = "+ structure.isLoaded() +
			" isAuthoritative(Structure) = "+ structure.isAuthoritative());
			return SCRIPT_CONTINUE;
		}
		
		if (!player_structure.isInstallation(structure) && !areAllContentsLoaded(structure) && !hasObjVar(structure, "structure.player.pack"))
		{
			LOG("sissynoid", "packUpCityAbandonedStructure: Not Installation, Not Loaded, No Objvar");
			
			setObjVar(structure, "structure.player.pack", self);
			
			loadBuildingContents(self, structure);
			return SCRIPT_CONTINUE;
		}
		
		if (!hasObjVar(structure, "structure.player.pack"))
		{
			LOG("sissynoid", "Go! All Contents are Loaded!");
			player_structure.finalizeCityPackUp(self, structure);
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int msgConfirmVendorPackup(obj_id self, dictionary params) throws InterruptedException
	{
		if (!utils.hasScriptVar(self, "packup.vendor"))
		{
			return SCRIPT_CONTINUE;
		}
		
		if (utils.hasScriptVar(self, "packup.vendor.suiconfirm"))
		{
			utils.removeScriptVar(self, "packup.vendor.suiconfirm");
		}
		
		obj_id vendor = utils.getObjIdScriptVar(self, "packup.vendor");
		utils.removeScriptVar(self, "packup.vendor");
		
		int bp = sui.getIntButtonPressed(params);
		if (bp == sui.BP_CANCEL)
		{
			LOG("vendor", "Declined packup.");
			return SCRIPT_CONTINUE;
		}
		
		obj_id vendorOwner = getObjIdObjVar(vendor, "vendor_owner");
		if (!isIdValid(vendorOwner))
		{
			vendorOwner = getOwner(vendor);
		}
		
		vendor_lib.finalizePackUp(vendorOwner, vendor, self, false);
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int getVendorInfoForPlayerRsp(obj_id self, dictionary params) throws InterruptedException
	{
		String summary = params.getString("summary");
		if (summary == null)
		{
			return SCRIPT_CONTINUE;
		}
		
		String[] name = params.getStringArray("name");
		if (name == null)
		{
			return SCRIPT_CONTINUE;
		}
		
		String[] location = params.getStringArray("location");
		if ((location == null) || (location.length != name.length))
		{
			return SCRIPT_CONTINUE;
		}
		
		String[] tax = params.getStringArray("tax");
		if ((tax == null) || (tax.length != name.length))
		{
			return SCRIPT_CONTINUE;
		}
		
		String[] taxCity = params.getStringArray("taxCity");
		if ((taxCity == null) || (taxCity.length != name.length))
		{
			return SCRIPT_CONTINUE;
		}
		
		String[] emptyDate = params.getStringArray("emptyDate");
		if ((emptyDate == null) || (emptyDate.length != name.length))
		{
			return SCRIPT_CONTINUE;
		}
		
		String[] lastAccessDate = params.getStringArray("lastAccessDate");
		if ((lastAccessDate == null) || (lastAccessDate.length != name.length))
		{
			return SCRIPT_CONTINUE;
		}
		
		String[] inactiveDate = params.getStringArray("inactiveDate");
		if ((inactiveDate == null) || (inactiveDate.length != name.length))
		{
			return SCRIPT_CONTINUE;
		}
		
		String[] status = params.getStringArray("status");
		if ((status == null) || (status.length != name.length))
		{
			return SCRIPT_CONTINUE;
		}
		
		String[] searchable = params.getStringArray("searchable");
		if ((searchable == null) || (searchable.length != name.length))
		{
			return SCRIPT_CONTINUE;
		}
		
		String[] entranceCharge = params.getStringArray("entranceCharge");
		if ((entranceCharge == null) || (entranceCharge.length != name.length))
		{
			return SCRIPT_CONTINUE;
		}
		
		String[] itemCount = params.getStringArray("itemCount");
		if ((itemCount == null) || (itemCount.length != name.length))
		{
			return SCRIPT_CONTINUE;
		}
		
		String[] offerCount = params.getStringArray("offerCount");
		if ((offerCount == null) || (offerCount.length != name.length))
		{
			return SCRIPT_CONTINUE;
		}
		
		String[] stockRoomCount = params.getStringArray("stockRoomCount");
		if ((stockRoomCount == null) || (stockRoomCount.length != name.length))
		{
			return SCRIPT_CONTINUE;
		}
		
		String[] columnHeader =
		{
			"Name", "Location", "Status", "Active Items", "Stockroom Items", "Offered Items", "% Tax", "Tax City", "Searchable", "Entrance Fee", "Last Access Date", "Empty Date", "Inactive Date"
		};
		String[] columnHeaderType =
		{
			"text", "text", "text", "integer", "integer", "integer", "integer", "text", "text", "integer", "text", "text", "text"
		};
		String[][] columnData = new String[13][0];
		columnData[0] = name;
		columnData[1] = location;
		columnData[2] = status;
		columnData[3] = itemCount;
		columnData[4] = stockRoomCount;
		columnData[5] = offerCount;
		columnData[6] = tax;
		columnData[7] = taxCity;
		columnData[8] = searchable;
		columnData[9] = entranceCharge;
		columnData[10] = lastAccessDate;
		columnData[11] = emptyDate;
		columnData[12] = inactiveDate;
		
		sui.tableColumnMajor(self, self, sui.OK_ONLY, "Vendor Information", null, summary, columnHeader, columnHeaderType, columnData, true);
		
		return SCRIPT_CONTINUE;
	}
	
	
	public boolean isMoveCommandValid(obj_id player, obj_id target) throws InterruptedException
	{
		location loc = getLocation(player);
		obj_id structure = getTopMostContainer(loc.cell);
		
		if (!isIdValid(structure) || !isIdValid(target))
		{
			return false;
		}
		
		if (hasObjVar (target, "paintingId"))
		{
			obj_id painting = getObjIdObjVar (target, "paintingId");
			destroyObject (painting);
			removeObjVar (target, "paintingId");
			
			sendSystemMessage (player, MOVED_CTRL_OBJ);
			
		}
		
		if (static_item.isStaticItem(target))
		{
			String paintingName = static_item.getStaticItemName(target);
			if (paintingName.startsWith ("item_publish_gift_magic_painting_"))
			{
				sendSystemMessage (player, MOVED_PAINTING);
				return false;
			}
		}
		
		if (hasObjVar (target, "noMoveItem"))
		{
			sendSystemMessage (player, NO_MOVE_ITEM);
			return false;
			
		}
		
		if (getGameObjectType(structure) == GOT_building_factional && hasObjVar(structure, "hq"))
		{
			sendSystemMessage(player, new string_id(STF, "no_move_hq"));
			return false;
		}
		
		if (!player_structure.isAdmin(structure, player))
		{
			LOG("LOG_CHANNEL", player + " ->You must be an admin to manipulate objects.");
			sendSystemMessage(player, new string_id(STF, "admin_move_only"));
			return false;
		}
		if (!canManipulateSkipNoTradeCheck(player, target, true, false, 0, true))
		{
			sendSystemMessage(player, new string_id(STF, "cant_manipulate"));
			return false;
		}
		
		if (isPlayer(target) || (isMob(target) && !hasCondition(target, CONDITION_VENDOR)))
		{
			LOG("LOG_CHANNEL", player + " ->You can't manipulate that.");
			sendSystemMessage(player, new string_id(STF, "cant_manipulate"));
			return false;
		}
		
		obj_id structure_obj = getTopMostContainer(target);
		if (structure != structure_obj || target == structure_obj)
		{
			LOG("LOG_CHANNEL", player + " -> That object is not within the building.");
			sendSystemMessage(player, new string_id(STF, "item_not_in_building"));
			return false;
		}
		
		obj_id[] base_objects = player_structure.getStructureBaseObjects(structure);
		if (base_objects != null)
		{
			for (int i = 0; i < base_objects.length; i++)
			{
				testAbortScript();
				if (target == base_objects[i])
				{
					LOG("LOG_CHANNEL", player + " -> You cannot move that object.");
					sendSystemMessage(player, new string_id(STF, "cant_move_item"));
					return false;
				}
			}
		}
		
		return true;
	}
	
	
	public String getCellName(obj_id building, obj_id cell) throws InterruptedException
	{
		
		String[] cellNames = getCellNames(building);
		
		for (int i = 0; i < cellNames.length; i++)
		{
			testAbortScript();
			
			String cellName = cellNames[i];
			obj_id thisCell = getCellId(building, cellName);
			
			if (thisCell == cell)
			{
				
				return cellName;
			}
		}
		
		return "";
	}
	
	
	public boolean canPlaceCivic(obj_id player, obj_id deed, location position, String template) throws InterruptedException
	{
		
		if (utils.isFreeTrial(player))
		{
			return false;
		}
		
		int city_id = getCityAtLocation(position, 0);
		if (!cityExists(city_id))
		{
			return false;
		}
		
		obj_id mayor = cityGetLeader(city_id);
		if (mayor != player)
		{
			return false;
		}
		
		return true;
	}
	
	
	public boolean canPlaceUnique(obj_id player, obj_id deed, location position, String template) throws InterruptedException
	{
		int city_id = getCityAtLocation(position, 0);
		if (!cityExists(city_id))
		{
			return false;
		}
		
		if (utils.isFreeTrial(player))
		{
			return false;
		}
		
		if (player_structure.isShuttleportTemplate(template))
		{
			
			int travelCost = cityGetTravelCost(city_id);
			if (travelCost > 0)
			{
				return false;
			}
			else
			{
				return true;
			}
		}
		else if (player_structure.isCloneTemplate(template))
		{
			
			obj_id clone = cityGetCloneId(city_id);
			if (isIdValid(clone))
			{
				
				obj_id[] structures = cityGetStructureIds(city_id);
				for (int i=0; i<structures.length; i++)
				{
					testAbortScript();
					if ((structures[i] == clone) && (structures[i] != null))
					{
						return false;
					}
				}
				
				return true;
			}
			else
			{
				return true;
			}
		}
		
		return true;
	}
	
	
	public boolean confirmDestroyTarget(obj_id structure, obj_id target) throws InterruptedException
	{
		obj_id self = getSelf();
		if (!isIdValid(target))
		{
			sendSystemMessage(self, new string_id(STF, "invalid_target"));
			return false;
		}
		
		if (!isIdValid(structure) || structure != target)
		{
			sendSystemMessage(self, new string_id(STF, "changed_target"));
			return false;
		}
		
		return true;
	}
	
	
	public boolean isFactionBaseEmpty(obj_id base, obj_id player) throws InterruptedException
	{
		if (!hasScript(base, "faction_perk.hq.loader"))
		{
			return true;
		}
		
		obj_id[] players = trial.getPlayersInDungeon(base);
		
		if (players == null || players.length == 0)
		{
			return true;
		}
		
		if (players.length > 1)
		{
			return false;
		}
		
		return true;
	}
	
	
	public boolean removeVendorVars(obj_id self) throws InterruptedException
	{
		if (!isValidId(self) || !exists(self))
		{
			return false;
		}
		utils.removeScriptVarTree(self, VENDOR_VAR_PREFIX);
		utils.removeScriptVarTree(self, "vendor");
		blog("player_building.removeVendorVars: removed VENDOR_VAR_PREFIX Vars ");
		return true;
	}
	
	
	public boolean handleVendorTypeSelect(obj_id self) throws InterruptedException
	{
		blog("player_building:handleVendorTypeSelect() - about to requestPlayerVendorCount");
		if (!isValidId(self) || !exists(self))
		{
			return false;
		}
		else if (!validateVendorPlacement(self))
		{
			return false;
		}
		
		blog("player_building:handleVendorTypeSelect() - SETTING CREATING_VENDOR to 1");
		utils.setScriptVar(self, CREATING_VENDOR, 1);
		requestPlayerVendorCount(self);
		
		return true;
	}
	
	
	public boolean validateVendorPlacement(obj_id self) throws InterruptedException
	{
		if (!isValidId(self) || !exists(self))
		{
			return false;
		}
		if (player_structure.isCivic(self))
		{
			blog("player_building:validateVendorPlacement() - isCivic = true");
			return false;
		}
		if (utils.isFreeTrial(self))
		{
			blog("player_building:validateVendorPlacement() - Player is free trial");
			sendSystemMessage(self, SID_NO_VENDOR);
			return false;
		}
		if (hasObjVar(self, "vendor_not_initialized"))
		{
			blog("player_building:validateVendorPlacement() - has vendor_not_initialized");
			
			sendSystemMessage(self, SID_SYS_VENDOR_NOT_INITIALIZED);
			return false;
		}
		blog("player_building:validateVendorPlacement() - Everything is good, player can place.");
		
		return true;
	}
	
	
	public String[] getSpecialVendors(obj_id self, int hireMod) throws InterruptedException
	{
		blog("player_building.getSpecialVendors: init");
		
		if (!isValidId(self) || !exists(self))
		{
			return null;
		}
		else if (hireMod < 50)
		{
			return null;
		}
		
		blog("player_building.getSpecialVendors: initial validation passes");
		
		String[] specialVendorNames = dataTableGetStringColumn(TBL_SPECIAL_PLAYER_VENDOR_TYPES, "special_vendor_name");
		if (specialVendorNames == null)
		{
			return null;
		}
		
		blog("player_building.getSpecialVendors: specialVendorNames received");
		
		String[] specialVendorSkillMod = dataTableGetStringColumn(TBL_SPECIAL_PLAYER_VENDOR_TYPES, "skill_mod");
		if (specialVendorSkillMod == null)
		{
			return null;
		}
		
		blog("player_building.getSpecialVendors: specialVendorSkillMod received");
		
		int[] requiredHireMod = dataTableGetIntColumn(TBL_SPECIAL_PLAYER_VENDOR_TYPES, "requires_hiring_mod");
		if (requiredHireMod == null)
		{
			return null;
		}
		
		blog("player_building.getSpecialVendors: requiredHireMod received");
		blog("player_building.getSpecialVendors: received specialVendorNames, specialVendorSkillMod, requiredHireMod");
		
		Vector allSpecialVendorsOfPlayerHireMod = new Vector();
		allSpecialVendorsOfPlayerHireMod.setSize(0);
		Vector allSpecialVendorsMods = new Vector();
		allSpecialVendorsMods.setSize(0);
		
		blog("player_building.getSpecialVendors: about to grab special vendor list");
		
		for (int i = 0; i < specialVendorNames.length; i++)
		{
			testAbortScript();
			if (requiredHireMod[i] <= hireMod)
			{
				blog("player_building.getSpecialVendors: "+specialVendorNames[i]+" had a hire mod of: "+requiredHireMod[i]+" and was added to list");
				utils.addElement(allSpecialVendorsOfPlayerHireMod, specialVendorNames[i]);
				utils.addElement(allSpecialVendorsMods, specialVendorSkillMod[i]);
				blog("player_building.getSpecialVendors: "+specialVendorNames[i]+" has a skill mod: "+specialVendorSkillMod[i]);
			}
		}
		if (allSpecialVendorsOfPlayerHireMod == null || allSpecialVendorsMods == null)
		{
			return null;
		}
		
		blog("player_building.getSpecialVendors: specialVendorNames.length: "+specialVendorNames.length);
		
		int vendorNameSize = allSpecialVendorsOfPlayerHireMod.size();
		int vendorModSize = allSpecialVendorsMods.size();
		if (vendorNameSize != vendorModSize)
		{
			return null;
		}
		else if (vendorNameSize == 0 || vendorModSize == 0)
		{
			return null;
		}
		
		String[] vendorNames = new String[vendorNameSize];
		allSpecialVendorsOfPlayerHireMod.toArray(vendorNames);
		String[] vendorMods = new String[vendorModSize];
		allSpecialVendorsMods.toArray(vendorMods);
		
		Vector allSpecialVendors = new Vector();
		allSpecialVendors.setSize(0);
		
		blog("player_building.getSpecialVendors: about to test each of the special vendors to see if I have the necessary skill mods.");
		
		for (int b = 0; b < specialVendorSkillMod.length; b++)
		{
			testAbortScript();
			if (getSkillStatMod(self, vendorMods[b]) <= 0)
			{
				continue;
			}
			
			blog("player_building.getSpecialVendors: Found a skill mod I have: "+vendorMods[b]);
			utils.addElement(allSpecialVendors, vendorNames[b]);
		}
		blog("player_building.getSpecialVendors: allSpecialVendors.size(): "+allSpecialVendors.size());
		return (String[])allSpecialVendors.toArray(new String[0]);
	}
	
	
	public boolean blog(String msg) throws InterruptedException
	{
		if (LOGGING_ON && msg != null && !msg.equals(""))
		{
			LOG(LOGGING_CATEGORY, msg);
		}
		return true;
	}
}
