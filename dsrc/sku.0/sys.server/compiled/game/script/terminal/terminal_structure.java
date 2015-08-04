package script.terminal;

import script.*;
import script.base_class.*;
import script.combat_engine.*;
import java.util.Arrays;
import java.util.Hashtable;
import java.util.Vector;
import script.base_script;


import script.library.faction_perk;
import script.library.player_structure;
import script.library.prose;
import script.library.session;
import script.library.static_item;
import script.library.sui;
import script.library.turnstile;
import script.library.utils;


public class terminal_structure extends script.base_script
{
	public terminal_structure()
	{
	}
	public static final String TERMINAL_LOGGING = "special_sign";
	public static final boolean LOGGING_ON = true;
	
	public static final string_id SID_TERMINAL_PERMISSIONS = new string_id("player_structure", "permissions");
	public static final string_id SID_TERMINAL_MANAGEMENT = new string_id("player_structure", "management");
	public static final string_id SID_TERMINAL_PERMISSIONS_ENTER = new string_id("player_structure", "permission_enter");
	public static final string_id SID_TERMINAL_PERMISSIONS_BANNED = new string_id("player_structure", "permission_banned");
	public static final string_id SID_TERMINAL_PERMISSIONS_ADMIN = new string_id("player_structure", "permission_admin");
	public static final string_id SID_TERMINAL_PERMISSIONS_VENDOR = new string_id("player_structure", "permission_vendor");
	public static final string_id SID_TERMINAL_PERMISSIONS_HOPPER = new string_id("player_structure", "permission_hopper");
	public static final string_id SID_TERMINAL_MANAGEMENT_STATUS = new string_id("player_structure", "management_status");
	public static final string_id SID_TERMINAL_MANAGEMENT_PRIVACY = new string_id("player_structure", "management_privacy");
	public static final string_id SID_TERMINAL_MANAGEMENT_TRANSFER = new string_id("player_structure", "management_transfer");
	public static final string_id SID_TERMINAL_MANAGEMENT_RESIDENCE = new string_id("player_structure", "management_residence");
	public static final string_id SID_TERMINAL_MANAGEMENT_DESTROY = new string_id("player_structure", "permission_destroy");
	public static final string_id SID_TERMINAL_MANAGEMENT_PAY = new string_id("player_structure", "management_pay");
	public static final string_id SID_TERMINAL_NAME_STRUCTURE = new string_id("player_structure", "management_name_structure");
	public static final string_id SID_TERMINAL_MANAGEMENT_ADD_TURNSTILE = new string_id("player_structure", "management_add_turnstile");
	public static final string_id SID_TERMINAL_MANAGEMENT_REMOVE_TURNSTILE = new string_id("player_structure", "management_remove_turnstile");
	public static final string_id SID_TERMINAL_ASSIGN_DROID = new string_id("player_structure", "assign_droid");
	public static final string_id SID_TERMINAL_CREATE_VENDOR = new string_id("player_structure", "create_vendor");
	public static final string_id SID_TERMINAL_PACK_HOUSE = new string_id("sui", "packup_house");
	public static final string_id SID_TERMINAL_MANAGEMENT_SPECIAL_SIGNS = new string_id("player_structure", "special_sign_management");
	
	public static final string_id SID_TERMINAL_PERMISSIONS_HARVESTER = new string_id("harvester", "manage");
	public static final string_id SID_TERMINAL_MANAGEMENT_POWER = new string_id("player_structure", "management_power");
	public static final string_id SID_TERMINAL_MANAGEMENT_MINE_INV = new string_id("player_structure", "management_mine_inv");
	public static final string_id SID_TERMINAL_MANAGEMENT_CHANGE_SIGN = new string_id("player_structure", "management_change_sign");
	public static final string_id SID_PUBLIC_ONLY = new string_id("player_structure", "public_only");
	public static final string_id SID_WITHDRAW_MAINTENANCE = new string_id("player_structure", "withdraw_maintenance");
	public static final string_id SID_TERMINAL_REDEED_STORAGE = new string_id("player_structure", "redeed_storage");
	public static final string_id SID_STORAGE_INCREASE_REDEED_TITLE = new string_id("player_structure", "sui_storage_redeed_title");
	public static final string_id SID_STORAGE_INCREASE_REDEED_PROMPT = new string_id("player_structure", "sui_storage_redeed_prompt");
	
	public static final string_id SID_FIND_ALL_HOUSE_ITEMS = new string_id( "player_structure", "find_items_find_all_house_items");
	public static final string_id SID_SEARCH_FOR_HOUSE_ITEMS = new string_id( "player_structure", "find_items_search_for_house_items");
	public static final string_id SID_MOVE_FIRST_ITEM = new string_id( "player_structure", "move_first_item");
	public static final string_id SID_MOVED_FIRST_ITEM = new string_id( "player_structure", "moved_first_item");
	public static final string_id SID_DELETE_ALL_ITEMS = new string_id( "player_structure", "delete_all_items");
	public static final string_id SID_ITEMS_DELETED = new string_id( "player_structure", "items_deleted");
	public static final string_id SID_WHILE_DEAD = new string_id( "player_structure", "while_dead");
	public static final string_id SID_TCG_VENDOR_CTS_WARNING = new string_id( "player_vendor", "create_vendor_cts_warning");
	public static final string_id SID_REVERT_CUSTOM_SIGN = new string_id("player_structure", "revert_sign");
	public static final string_id SID_STRUCTURE_DECOR = new string_id("player_structure", "structure_decor_menu");
	public static final string_id SID_STRUCTURE_ADD_DECOR = new string_id("player_structure", "structure_add_decor");
	public static final string_id SID_STRUCTURE_REMOVE_DECOR = new string_id("player_structure", "structure_remove_decor");
	
	public static final String STRUCTURE_MARK = "turnstile.structureMark";
	
	
	public int OnObjectMenuRequest(obj_id self, obj_id player, menu_info mi) throws InterruptedException
	{
		
		if (isDead(player) || isIncapacitated(player))
		{
			sendSystemMessage( player, SID_WHILE_DEAD );
			return SCRIPT_CONTINUE;
		}
		
		obj_id structure = self;
		blog("terminal_structure::OnObjectMenuRequest");
		
		player_structure.doOldToNewLotConversion(player, structure);
		
		if (!player_structure.isInstallation(structure))
		{
			structure = player_structure.getStructure(player);
			
			if (!isIdValid(structure))
			{
				return SCRIPT_CONTINUE;
			}
		}
		if (player_structure.isStructureCondemned(self) && player_structure.isOwner(player, structure))
		{
			player_structure.doCondemnedSui(self, player);
			return SCRIPT_OVERRIDE;
		}
		
		int got = getGameObjectType(structure);
		
		if (player_structure.isAdmin(structure, player))
		{
			blog("terminal_structure::OnObjectMenuRequest - you are admin");
			
			if (player_structure.isHarvester(structure) || player_structure.isGenerator(structure))
			{
				if (!player_structure.isOwner(structure, player))
				{
					return SCRIPT_CONTINUE;
				}
			}
			
			int management_root = mi.addRootMenu (menu_info_types.SERVER_TERMINAL_MANAGEMENT, SID_TERMINAL_MANAGEMENT);
			
			mi.addSubMenu(management_root, menu_info_types.SERVER_TERMINAL_MANAGEMENT_DESTROY, SID_TERMINAL_MANAGEMENT_DESTROY);
			
			if (got == GOT_installation_minefield)
			{
				return SCRIPT_CONTINUE;
			}
			
			mi.addSubMenu(management_root, menu_info_types.SERVER_TERMINAL_MANAGEMENT_STATUS, SID_TERMINAL_MANAGEMENT_STATUS);
			
			String template = getTemplateName(structure);
			
			if (player_structure.isCivic(structure))
			{
				blog("terminal_structure::OnObjectMenuRequest - I am civic");
				
				if (template.indexOf("cloning_") > -1)
				{
					mi.addSubMenu(management_root, menu_info_types.SET_NAME, new string_id());
				}
				if (template.indexOf("garden_") > -1)
				{
					mi.addSubMenu(management_root, menu_info_types.SET_NAME, new string_id());
				}
				if (!(template.indexOf("cityhall_") > -1))
				{
					return SCRIPT_CONTINUE;
				}
			}
			
			int permissions_root = 0;
			
			if (!player_structure.isHarvester(structure) && !player_structure.isGenerator(structure))
			{
				blog("terminal_structure::OnObjectMenuRequest - I am harvester/generator");
				
				permissions_root = mi.addRootMenu (menu_info_types.SERVER_TERMINAL_PERMISSIONS, SID_TERMINAL_PERMISSIONS);
				mi.addSubMenu(permissions_root, menu_info_types.SERVER_TERMINAL_PERMISSIONS_ADMIN, SID_TERMINAL_PERMISSIONS_ADMIN);
			}
			
			if ((template.indexOf("cityhall_") > -1))
			{
				return SCRIPT_CONTINUE;
			}
			
			mi.addSubMenu(management_root, menu_info_types.SERVER_TERMINAL_MANAGEMENT_PAY, SID_TERMINAL_MANAGEMENT_PAY);
			
			mi.addSubMenu(management_root, menu_info_types.SET_NAME, new string_id());
			
			if (player_structure.hasMaintenanceDroid(player))
			{
				mi.addSubMenu(management_root, menu_info_types.SERVER_MENU5, SID_TERMINAL_ASSIGN_DROID);
			}
			if (player_structure.canPackBuilding(player, structure))
			{
				mi.addSubMenu( management_root, menu_info_types.SERVER_MENU10, SID_TERMINAL_PACK_HOUSE);
			}
			
			if (player_structure.isBuilding(structure))
			{
				blog("terminal_structure::OnObjectMenuRequest - I am a building");
				
				boolean isStructureOwner = player_structure.isOwner(structure, player);
				
				if (permissions_root != 0)
				{
					permissions_root = mi.addRootMenu (menu_info_types.SERVER_TERMINAL_PERMISSIONS, SID_TERMINAL_PERMISSIONS);
				}
				
				mi.addSubMenu(permissions_root, menu_info_types.SERVER_TERMINAL_PERMISSIONS_ADMIN, SID_TERMINAL_PERMISSIONS_ADMIN);
				mi.addSubMenu(permissions_root, menu_info_types.SERVER_TERMINAL_PERMISSIONS_ENTER, SID_TERMINAL_PERMISSIONS_ENTER);
				mi.addSubMenu(management_root, menu_info_types.SERVER_TERMINAL_MANAGEMENT_RESIDENCE, SID_TERMINAL_MANAGEMENT_RESIDENCE);
				mi.addSubMenu(permissions_root, menu_info_types.SERVER_TERMINAL_PERMISSIONS_BANNED, SID_TERMINAL_PERMISSIONS_BANNED);
				
				if (areAllContentsLoaded(structure))
				{
					mi.addSubMenu(management_root, menu_info_types.SERVER_TERMINAL_MANAGEMENT_PRIVACY, SID_TERMINAL_MANAGEMENT_PRIVACY);
					
					if (getSkillStatMod(player, "manage_vendor") > 0)
					{
						mi.addSubMenu(management_root, menu_info_types.SERVER_TERMINAL_CREATE_VENDOR, SID_TERMINAL_CREATE_VENDOR);
					}
					if (utils.isProfession(player, utils.TRADER))
					{
						
						if (turnstile.hasTurnstile( structure ))
						{
							mi.addSubMenu(management_root, menu_info_types.SERVER_MENU3, SID_TERMINAL_MANAGEMENT_REMOVE_TURNSTILE);
						}
						else
						{
							mi.addSubMenu(management_root, menu_info_types.SERVER_MENU4, SID_TERMINAL_MANAGEMENT_ADD_TURNSTILE);
						}
					}
					
					if (player_structure.isGuildHall(structure))
					{
						mi.addSubMenu(management_root, menu_info_types.SERVER_MENU8, SID_WITHDRAW_MAINTENANCE);
					}
					
					mi.addSubMenu( management_root, menu_info_types.SERVER_MENU12, SID_FIND_ALL_HOUSE_ITEMS );
					mi.addSubMenu( management_root, menu_info_types.SERVER_MENU13, SID_SEARCH_FOR_HOUSE_ITEMS );
					mi.addSubMenu( management_root, menu_info_types.SERVER_MENU9, SID_MOVE_FIRST_ITEM );
					mi.addSubMenu( management_root, menu_info_types.SERVER_MENU2, SID_DELETE_ALL_ITEMS );
					
					if (player_structure.isOwner(structure, player))
					{
						
						if (hasObjVar(structure, player_structure.OBJVAR_STRUCTURE_STORAGE_INCREASE))
						{
							
							mi.addSubMenu( management_root, menu_info_types.DICE_ROLL, SID_TERMINAL_REDEED_STORAGE);
						}
					}
					
				}
				else
				{
					blog("terminal_structure::OnObjectMenuRequest - NOT ALL ITEMS LOADED");
				}
				
				if (hasObjVar(structure, player_structure.MODIFIED_HOUSE_SIGN) && isStructureOwner)
				{
					mi.addSubMenu( management_root, menu_info_types.SERVER_MENU11, SID_REVERT_CUSTOM_SIGN );
				}
				if ((hasObjVar(structure, player_structure.SPECIAL_SIGN) || player_structure.hasSpecialSignSkillMod(player, structure)))
				{
					blog("terminal_structure.OnObjectMenuRequest: Creating Special Sign Radial Option");
					mi.addSubMenu(management_root, menu_info_types.SERVER_MENU14, SID_TERMINAL_MANAGEMENT_SPECIAL_SIGNS);
				}
				
				if (player_structure.isOwner(structure, player) && isGod(player))
				{
					int decor_root = mi.addRootMenu(menu_info_types.SERVER_MENU14, SID_STRUCTURE_DECOR);
					mi.addSubMenu(decor_root, menu_info_types.SERVER_MENU15, SID_STRUCTURE_ADD_DECOR);
					mi.addSubMenu(decor_root, menu_info_types.SERVER_MENU16, SID_STRUCTURE_REMOVE_DECOR);
				}
			}
			else if (player_structure.isInstallation(structure))
			{
				
				if (got != GOT_installation_turret)
				{
					
					if ((got == GOT_installation_harvester) || (got == GOT_installation_generator))
					{
						mi.addSubMenu(management_root, menu_info_types.SERVER_HARVESTER_MANAGE, SID_TERMINAL_PERMISSIONS_HARVESTER);
					}
					
					if ((got == GOT_installation_harvester) || (got == GOT_installation_factory))
					{
						mi.addSubMenu(management_root, menu_info_types.SERVER_MENU6, SID_TERMINAL_MANAGEMENT_POWER);
					}
				}
				else
				{
					
				}
			}
		}
		else
		{
			if (player_structure.canPlayerPackAbandonedStructure(player, structure))
			{
				int management_root = mi.addRootMenu (menu_info_types.SERVER_TERMINAL_MANAGEMENT, SID_TERMINAL_MANAGEMENT);
				mi.addSubMenu( management_root, menu_info_types.SERVER_MENU10, SID_TERMINAL_PACK_HOUSE);
			}
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int OnObjectMenuSelect(obj_id self, obj_id player, int item) throws InterruptedException
	{
		sendDirtyObjectMenuNotification(self);
		
		if (isDead(player) || isIncapacitated(player))
		{
			sendSystemMessage( player, SID_WHILE_DEAD );
			return SCRIPT_CONTINUE;
		}
		
		blog("terminal_structure::OnObjectMenuSelect item = "+ item);
		blog("terminal_structure::OnObjectMenuSelect SERVER_TERMINAL_CREATE_VENDOR = "+ menu_info_types.SERVER_TERMINAL_CREATE_VENDOR);
		
		if (utils.hasScriptVar(player, "packup.suiconfirm"))
		{
			sui.closeSUI(player, utils.getIntScriptVar(player, "packup.suiconfirm"));
		}
		
		obj_id structure = self;
		if (!player_structure.isInstallation(structure))
		{
			structure = player_structure.getStructure( player );
			if (!isIdValid(structure))
			{
				return SCRIPT_CONTINUE;
			}
			if (player_structure.isHarvester(structure) || player_structure.isGenerator(structure))
			{
				player_structure.validateHarvestedResources(structure);
			}
		}
		
		if (!player_structure.isAdmin(structure, player) && !player_structure.isAbandoned(structure))
		{
			return SCRIPT_CONTINUE;
		}
		if (player_structure.isStructureCondemned(self) && player_structure.isOwner(player, structure))
		{
			player_structure.doCondemnedSui(self, player);
			return SCRIPT_OVERRIDE;
		}
		
		if ((item == menu_info_types.SERVER_TERMINAL_MANAGEMENT)||(item == menu_info_types.SERVER_TERMINAL_MANAGEMENT_STATUS))
		{
			queueCommand(player, (335013253), null, "", COMMAND_PRIORITY_DEFAULT);
		}
		
		String template = getTemplateName(structure);
		if (player_structure.isCivic(structure))
		{
			
			if (template.indexOf("cloning_") > -1 && item == menu_info_types.SET_NAME)
			{
				queueCommand(player, (-1016613791), null, "", COMMAND_PRIORITY_DEFAULT);
			}
			
			if (template.indexOf("garden_") > -1 && item == menu_info_types.SET_NAME)
			{
				queueCommand(player, (-1016613791), null, "", COMMAND_PRIORITY_DEFAULT);
			}
			
			if (!(template.indexOf("cityhall_") > -1) && (item != menu_info_types.SERVER_TERMINAL_MANAGEMENT_DESTROY))
			{
				return SCRIPT_CONTINUE;
			}
		}
		
		if (player_structure.isHarvester(structure) || player_structure.isGenerator(structure))
		{
			session.logActivity(player, session.ACTIVITY_ACCESS_HARVESTER);
		}
		else if (player_structure.isFactory(structure))
		{
			session.logActivity(player, session.ACTIVITY_ACCESS_FACTORY);
		}
		else
		{
			session.logActivity(player, session.ACTIVITY_ACCESS_STRUCTURE);
		}
		
		if (item == menu_info_types.SERVER_TERMINAL_PERMISSIONS_ENTER)
		{
			
			queueCommand(player, (1768087594), null, "entry", COMMAND_PRIORITY_DEFAULT);
		}
		else if (item == menu_info_types.SERVER_TERMINAL_PERMISSIONS_BANNED)
		{
			queueCommand(player, (1768087594), null, "ban", COMMAND_PRIORITY_DEFAULT);
		}
		else if (item == menu_info_types.SERVER_TERMINAL_PERMISSIONS || item == menu_info_types.SERVER_TERMINAL_PERMISSIONS_ADMIN)
		{
			queueCommand(player, (1768087594), null, "admin", COMMAND_PRIORITY_DEFAULT);
		}
		else if (item == menu_info_types.SERVER_TERMINAL_PERMISSIONS_VENDOR)
		{
			queueCommand(player, (1768087594), null, "vendor", COMMAND_PRIORITY_DEFAULT);
		}
		else if (item == menu_info_types.SERVER_TERMINAL_PERMISSIONS_HOPPER)
		{
			queueCommand(player, (1768087594), null, "hopper", COMMAND_PRIORITY_DEFAULT);
		}
		else if (item == menu_info_types.SERVER_TERMINAL_MANAGEMENT_PRIVACY)
		{
			queueCommand(player, (2020393870), null, "", COMMAND_PRIORITY_DEFAULT);
		}
		else if (item == menu_info_types.SERVER_TERMINAL_MANAGEMENT_RESIDENCE)
		{
			queueCommand(player, (-174179103), null, "", COMMAND_PRIORITY_DEFAULT);
		}
		else if (item == menu_info_types.SERVER_TERMINAL_MANAGEMENT_TRANSFER)
		{
			queueCommand(player, (292280838), null, "", COMMAND_PRIORITY_DEFAULT);
		}
		else if (item == menu_info_types.SERVER_TERMINAL_MANAGEMENT_DESTROY)
		{
			if (player_structure.isCivic(structure))
			{
				
				int city_id = getCityAtLocation(getLocation(structure), 0);
				
				if (city_id > 0)
				{
					obj_id mayor = cityGetLeader(city_id);
					
					if (isIdValid(mayor))
					{
						
						if (mayor != player)
						{
							
							String city_name = cityGetName(city_id);
							string_id message = new string_id("player_structure", "no_longer_mayor_block_destroy");
							prose_package pp = new prose_package();
							prose.setStringId(pp, message);
							prose.setTO(pp, city_name);
							sendSystemMessageProse(player, pp);
							
							CustomerServiceLog("playerStructure", "Civic Structure Destruction: Player: "+ player + " ("+ getName(player) + ") is NO LONGER THE MAYOR and is trying to destroy a Structure ("+ structure + 
							") before the City conversion is complete. The NEW MAYOR is "+ mayor + " ("+ getName(mayor) + ") and is the only one with this authority.");
							return SCRIPT_CONTINUE;
						}
					}
					else
					{
						
						string_id message = new string_id("player_structure", "catastrophic_failure_city_destroy_structure");
						sendSystemMessage(player, message);
						return SCRIPT_CONTINUE;
					}
				}
			}
			queueCommand(player, (419174182), null, "", COMMAND_PRIORITY_DEFAULT);
		}
		else if (item == menu_info_types.SERVER_TERMINAL_MANAGEMENT_PAY)
		{
			queueCommand(player, (-404530384), null, "", COMMAND_PRIORITY_DEFAULT);
		}
		else if (item == menu_info_types.SERVER_MENU1 || item == menu_info_types.SET_NAME)
		{
			queueCommand(player, (-1016613791), null, "", COMMAND_PRIORITY_DEFAULT);
		}
		else if (item == menu_info_types.SERVER_HARVESTER_MANAGE)
		{
			activateHarvesterExtractionPage (player, self);
		}
		else if (item == menu_info_types.SERVER_TERMINAL_CREATE_VENDOR)
		{
			sendSystemMessage(player, SID_TCG_VENDOR_CTS_WARNING);
			queueCommand(player, (-303051094), null, "", COMMAND_PRIORITY_DEFAULT);
		}
		else if (item == menu_info_types.SERVER_MENU5)
		{
			queueCommand(player, (1801226979), null, "", COMMAND_PRIORITY_DEFAULT);
		}
		
		else if (item == menu_info_types.SERVER_MENU12)
		{
			
			if (!player_structure.isBuilding( structure ))
			{
				return SCRIPT_CONTINUE;
			}
			int lockoutEnds = -1;
			if (hasObjVar(self, "findItems.lockout"))
			{
				lockoutEnds = getIntObjVar(self, "findItems.lockout");
			}
			
			int currentTime = getGameTime();
			if (currentTime > lockoutEnds || isGod(player))
			{
				player_structure.initializeFindAllItemsInHouse(self, player);
				setObjVar(self, "findItems.lockout", currentTime + player_structure.HOUSE_ITEMS_SEARCH_LOCKOUT);
			}
			else
			{
				string_id message = new string_id ("player_structure", "find_items_locked_out");
				prose_package pp = prose.getPackage(message, player, player);
				prose.setTO(pp, utils.formatTimeVerbose(lockoutEnds - currentTime));
				sendSystemMessageProse(player, pp);
			}
		}
		else if (item == menu_info_types.SERVER_MENU13)
		{
			
			if (!player_structure.isBuilding( structure ))
			{
				return SCRIPT_CONTINUE;
			}
			
			int lockoutEnds = -1;
			if (hasObjVar(self, "findItems.lockout"))
			{
				lockoutEnds = getIntObjVar(self, "findItems.lockout");
			}
			
			int currentTime = getGameTime();
			if (currentTime > lockoutEnds || isGod(player))
			{
				player_structure.initializeItemSearchInHouse(self, player);
				setObjVar(self, "findItems.lockout", currentTime + player_structure.HOUSE_ITEMS_SEARCH_LOCKOUT);
			}
			else
			{
				string_id message = new string_id ("player_structure", "find_items_locked_out");
				prose_package pp = prose.getPackage(message, player, player);
				prose.setTO(pp, utils.formatTimeVerbose(lockoutEnds - currentTime));
				sendSystemMessageProse(player, pp);
			}
		}
		else if (item == menu_info_types.SERVER_MENU9)
		{
			if (!player_structure.isBuilding( structure ))
			{
				return SCRIPT_CONTINUE;
			}
			
			moveFirstItem( self, player, structure );
		}
		else if (item == menu_info_types.SERVER_MENU2)
		{
			if (!player_structure.isBuilding( structure ))
			{
				return SCRIPT_CONTINUE;
			}
			
			deleteAllItems( self, player, structure );
		}
		else if (item == menu_info_types.SERVER_MENU3)
		{
			
			if (!turnstile.hasTurnstile( structure ))
			{
				return SCRIPT_CONTINUE;
			}
			turnstile.removeTurnstile( structure );
			sendSystemMessage( player, "Your building no longer has an access fee.", null );
		}
		else if (item == menu_info_types.SERVER_MENU4)
		{
			if (!permissionsIsPublic(structure))
			{
				sendSystemMessage( player, SID_PUBLIC_ONLY );
				return SCRIPT_CONTINUE;
			}
			if (!turnstile.canAddTurnstile( player, structure ))
			{
				return SCRIPT_CONTINUE;
			}
			if (turnstile.hasTurnstile( structure ) || utils.hasScriptVar( player, "turnstile.querySetTurnstile" ))
			{
				return SCRIPT_CONTINUE;
			}
			utils.setScriptVar( self, "turnstile.querySetTurnstile", 1 );
			utils.setScriptVar( self, STRUCTURE_MARK, structure);
			
			sui.inputbox( self, player, "@player_structure:access_fee", sui.OK_CANCEL, "@player_structure:access_fee_t", sui.INPUT_NORMAL, null, "handleSetAccessFee", null );
		}
		else if (item == menu_info_types.SERVER_MENU6)
		{
			queueCommand(player, (-1893504550), null, "", COMMAND_PRIORITY_DEFAULT);
		}
		else if (item == menu_info_types.SERVER_MENU8)
		{
			queueCommand(player, (-707821905), null, "", COMMAND_PRIORITY_DEFAULT);
		}
		else if (item == menu_info_types.SERVER_MENU7)
		{
			player_structure.showChangeSignSui(structure, player);
		}
		else if (item == menu_info_types.SERVER_MENU10)
		{
			
			if (player_structure.isAbandoned(structure) && player_structure.isPlayerGatedFromHousePackUp(player))
			{
				return SCRIPT_CONTINUE;
			}
			
			if (!player_structure.canPackStructureWithVendors(player, structure))
			{
				return SCRIPT_CONTINUE;
			}
			
			if (player_structure.isOwner(structure, player) && player_structure.canPackBuilding(player, structure))
			{
				blog("owner is using pack up");
				player_structure.packBuilding(player, structure);
			}
			
			else if (player_structure.canPlayerPackAbandonedStructure(player, structure) && !utils.hasScriptVar(structure, player_structure.SCRIPTVAR_HOUSE_PACKUP_LOCKOUT_TIME))
			{
				blog("NON-OWNER is using pack up");
				dictionary params = new dictionary();
				params.put("player", player);
				params.put("house", structure);
				
				messageTo (player, "handlePlayerStructurePackupLockout", params, 0, false);
				messageTo (structure, "packAbandonedBuilding", params, 4, false);
			}
			
			return SCRIPT_CONTINUE;
		}
		
		else if (item == menu_info_types.SERVER_MENU11)
		{
			player_structure.revertCustomSign(player, structure);
		}
		else if (item == menu_info_types.SERVER_MENU14)
		{
			blog("terminal_structure.OnMenuSelect: Player selected Special Sign Radial Option");
			getSpecialSignManagementMenu(player, structure);
		}
		else if (item == menu_info_types.SERVER_MENU15)
		{
			blog("SERVER_MENU15 - selected");
			player_structure.getStructureDecorMenu(player, structure, template);
		}
		else if (item == menu_info_types.SERVER_MENU16)
		{
			blog("about to REMOVE DECOR");
			player_structure.removeStructureDecor(structure, player);
		}
		else if (item == menu_info_types.DICE_ROLL)
		{
			if (!hasObjVar(structure, player_structure.OBJVAR_STRUCTURE_STORAGE_INCREASE))
			{
				return SCRIPT_CONTINUE;
			}
			
			player_structure.displayAvailableNonGenericStorageTypes(player, self, structure);
			
		}
		return SCRIPT_CONTINUE;
	}
	
	
	public int handleStorageRedeedChoice(obj_id self, dictionary params) throws InterruptedException
	{
		obj_id player = sui.getPlayerId(params);
		String accessFee = sui.getInputBoxText(params);
		int btn = sui.getIntButtonPressed(params);
		
		if (btn == sui.BP_CANCEL)
		{
			return SCRIPT_CONTINUE;
		}
		
		obj_id structure = player_structure.getStructure(player);
		
		if (!player_structure.isOwner(structure, player))
		{
			return SCRIPT_CONTINUE;
		}
		
		if (hasObjVar(structure, player_structure.OBJVAR_STRUCTURE_STORAGE_INCREASE))
		{
			int storageRedeedSelected = 0;
			if (params.containsKey(sui.LISTBOX_LIST + "." + sui.PROP_SELECTEDROW))
			{
				storageRedeedSelected = sui.getListboxSelectedRow(params);
				if (storageRedeedSelected < 0)
				{
					sendSystemMessage(player, new string_id("player_structure", "storage_redeed_no_selection"));
					return SCRIPT_CONTINUE;
				}
			}
			
			if (player_structure.decrementStorageAmount(player, structure, self, storageRedeedSelected))
			{
				sendSystemMessage(player, new string_id("player_structure", "storage_increase_redeeded"));
			}
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int handleSetAccessFee(obj_id self, dictionary params) throws InterruptedException
	{
		
		obj_id player = sui.getPlayerId(params);
		String accessFee = sui.getInputBoxText( params );
		int btn = sui.getIntButtonPressed( params );
		if (btn == sui.BP_CANCEL)
		{
			utils.removeScriptVar( self, "turnstile.querySetTurnstile");
			return SCRIPT_CONTINUE;
		}
		
		int fee = utils.stringToInt( accessFee );
		utils.setScriptVar( self, "turnstile.fee", fee );
		
		if ((fee < 1) || (fee > 50000))
		{
			sui.inputbox( self, player, "@player_structure:access_fee", sui.OK_CANCEL, "@player_structure:access_fee_t", sui.INPUT_NORMAL, null, "handleSetAccessFee", null );
			return SCRIPT_CONTINUE;
		}
		
		sui.inputbox( self, player, "@player_structure:access_time", sui.OK_CANCEL, "@player_structure:access_time_t", sui.INPUT_NORMAL, null, "handleSetAccessLength", null );
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int handleSetAccessLength(obj_id self, dictionary params) throws InterruptedException
	{
		
		obj_id player = sui.getPlayerId(params);
		String accessLength = sui.getInputBoxText( params );
		int btn = sui.getIntButtonPressed( params );
		if (btn == sui.BP_CANCEL)
		{
			utils.removeScriptVar( self, "turnstile.querySetTurnstile");
			return SCRIPT_CONTINUE;
		}
		
		int length = utils.stringToInt( accessLength );
		if (length < 15 || length > 2880)
		{
			sui.inputbox( self, player, "@player_structure:access_time", sui.OK_CANCEL, "@player_structure:access_time_t", sui.INPUT_NORMAL, null, "handleSetAccessLength", null );
			return SCRIPT_CONTINUE;
		}
		length *= 60;
		
		int fee = utils.getIntScriptVar( self, "turnstile.fee");
		
		obj_id structure = self;
		if (!player_structure.isInstallation(structure))
		{
			structure = utils.getObjIdScriptVar(self, STRUCTURE_MARK);
			
			if (!isIdValid(structure) || !structure.isLoaded())
			{
				return SCRIPT_CONTINUE;
			}
		}
		turnstile.addTurnstile( structure, fee, length );
		
		sendSystemMessage( player, "Other players will now be charged "+ fee + " credits to access your building for "+ length/60 + " minutes.", null );
		
		return SCRIPT_CONTINUE;
	}
	
	
	public void moveFirstItem(obj_id self, obj_id player, obj_id structure) throws InterruptedException
	{
		sui.msgbox( self, player, "@player_structure:move_first_item_d", sui.OK_CANCEL, "@player_structure:move_first_item", sui.MSG_QUESTION, "handleMoveFirstItem");
	}
	
	
	public int handleMoveFirstItem(obj_id self, dictionary params) throws InterruptedException
	{
		obj_id player = sui.getPlayerId( params );
		int btn = sui.getIntButtonPressed( params );
		if (btn == sui.BP_CANCEL)
		{
			return SCRIPT_CONTINUE;
		}
		
		obj_id building = player_structure.getStructure( player );
		moveHouseItemToPlayer( building, player, 0 );
		sendSystemMessage( player, SID_MOVED_FIRST_ITEM );
		
		return SCRIPT_CONTINUE;
	}
	
	
	public void deleteAllItems(obj_id self, obj_id player, obj_id structure) throws InterruptedException
	{
		sui.msgbox( self, player, "@player_structure:delete_all_items_d", sui.OK_CANCEL, "@player_structure:delete_all_items", sui.MSG_QUESTION, "handleDeleteSecondConfirm");
	}
	
	
	public int handleDeleteSecondConfirm(obj_id self, dictionary params) throws InterruptedException
	{
		obj_id player = sui.getPlayerId( params );
		int btn = sui.getIntButtonPressed( params );
		if (btn == sui.BP_CANCEL)
		{
			return SCRIPT_CONTINUE;
		}
		
		sui.msgbox( self, player, "@player_structure:delete_all_items_second_d", sui.OK_CANCEL, "@player_structure:delete_all_items", sui.MSG_QUESTION, "handleDeleteAllItemsCodeConfirm");
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int handleDeleteAllItemsCodeConfirm(obj_id self, dictionary params) throws InterruptedException
	{
		obj_id player = sui.getPlayerId(params);
		
		if (utils.hasScriptVar(self, "player_structure.destroyAllItems.pid"))
		{
			sui.closeSUI(player, utils.getIntScriptVar(self, "player_structure.destroyAllItems.pid"));
		}
		
		int key = rand(100000, 999999);
		utils.setScriptVar(self, "player_structure.destroyAllItems.key", key);
		
		int pid = sui.inputbox(self, player, "@player_structure:delete_all_items_prompt"+ "\n\nCode: "+ key, "@player_structure:delete_all_items_title", "handleDeleteAllItemsConfirmed", 6, false, "");
		if (pid > -1)
		{
			utils.setScriptVar(self, "player_structure.destroyAllItems.pid", pid);
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int handleDeleteAllItemsConfirmed(obj_id self, dictionary params) throws InterruptedException
	{
		int bp = sui.getIntButtonPressed(params);
		String text = sui.getInputBoxText(params);
		obj_id player = sui.getPlayerId(params);
		
		if (!isIdValid(player))
		{
			utils.removeScriptVarTree(self, "player_structure.destroyAllItems");
			return SCRIPT_CONTINUE;
		}
		if (bp == sui.BP_CANCEL || text == null || text.equals(""))
		{
			utils.removeScriptVarTree(self, "player_structure.destroyAllItems");
			return SCRIPT_CONTINUE;
		}
		
		int key = utils.getIntScriptVar(self, "player_structure.destroyAllItems.key");
		String skey = Integer.toString(key);
		
		if (text.equals(skey))
		{
			int btn = sui.getIntButtonPressed(params);
			if (btn == sui.BP_CANCEL)
			{
				utils.removeScriptVarTree(self, "player_structure.destroyAllItems");
				return SCRIPT_CONTINUE;
			}
			
			obj_id building = player_structure.getStructure(player);
			if (!isIdValid(building))
			{
				utils.removeScriptVarTree(self, "player_structure.destroyAllItems");
				return SCRIPT_CONTINUE;
			}
			deleteAllHouseItems(building, player);
			fixHouseItemLimit(building);
			sendSystemMessage(player, SID_ITEMS_DELETED);
			CustomerServiceLog("playerStructure", "deleteAllItems (Deleting all objects in house by player's request. Player had to enter a 6 Digit Code to confirm Deleting All Items.) Player: "+ player + " ("+ getName(player) + ") Structure: "+ building);
		}
		else
		{
			sui.msgbox(player, "@player_structure:incorrect_destroy_all_items_code");
			utils.removeScriptVarTree(self, "player_structure.destroyAllItems");
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int clearCollectionMeatlumpCamera(obj_id self, dictionary params) throws InterruptedException
	{
		
		obj_id target = params.getObjId("target");
		utils.removeScriptVar(target, "collection.picture_taken");
		return SCRIPT_CONTINUE;
	}
	
	
	public int handlePlayerStructureFindItemsListResponse(obj_id self, dictionary params) throws InterruptedException
	{
		player_structure.handleFindItemsListResponse(self, params);
		return SCRIPT_CONTINUE;
	}
	
	
	public int handlePlayerStructureFindItemsPageResponse(obj_id self, dictionary params) throws InterruptedException
	{
		player_structure.handleFindItemsChangePageResponse(self, params);
		return SCRIPT_CONTINUE;
	}
	
	
	public int handlePlayerStructureSearchItemsGetKeyword(obj_id self, dictionary params) throws InterruptedException
	{
		player_structure.handleSearchItemsGetKeyword(self, params);
		return SCRIPT_CONTINUE;
	}
	
	
	public int handlePlayerStructureSearchItemsSelectedResponse(obj_id self, dictionary params) throws InterruptedException
	{
		player_structure.handleSearchItemsSelectedResponse(self, params);
		return SCRIPT_CONTINUE;
	}
	
	
	public boolean getSpecialSignManagementMenu(obj_id player, obj_id structure) throws InterruptedException
	{
		blog("terminal_structure.getSpecialSignManagementMenu: init");
		
		if (!isValidId(player) || !isValidId(structure))
		{
			return false;
		}
		if (player_structure.isInstallation(structure) || player_structure.isCivic(structure))
		{
			return false;
		}
		if (!player_structure.hasSpecialSignSkillMod(player, structure) && !hasObjVar(structure, player_structure.SPECIAL_SIGN))
		{
			return false;
		}
		if (sui.hasPid(player, player_structure.VAR_SPECIAL_SIGN_MENU_PID))
		{
			int pid = sui.getPid(player, player_structure.VAR_SPECIAL_SIGN_MENU_PID);
			forceCloseSUIPage(pid);
		}
		blog("terminal_structure.getSpecialSignManagementMenu: validation completed, getting menu options");
		
		Vector menuOptions = new Vector();
		menuOptions.setSize(0);
		Vector menuStrings = new Vector();
		menuStrings.setSize(0);
		boolean replaceSign = false;
		boolean removeSign = false;
		
		if (hasObjVar(structure, player_structure.SPECIAL_SIGN))
		{
			utils.addElement(menuStrings, "@player_structure:remove_current_sign");
			utils.addElement(menuOptions, "remove");
			removeSign = true;
		}
		
		if (player_structure.getSpecialSignList(player, structure))
		{
			
			String[] signList = utils.getStringArrayScriptVar(player, player_structure.VAR_SPECIAL_SIGN_LIST);
			String[] signName = utils.getStringArrayScriptVar(player, player_structure.VAR_SPECIAL_SIGN_NAMES);
			
			if (signList != null && signList.length > 0 && signName != null && signName.length > 0)
			{
				
				utils.addElement(menuStrings, "@player_structure:replace_current_sign");
				utils.addElement(menuOptions, "replace");
				replaceSign = true;
			}
			
			if (replaceSign && !removeSign)
			{
				blog("terminal_structure.getSpecialSignManagementMenu: the only option was replace");
				int pid = sui.listbox(structure, player, "@base_player:special_sign_ui_prompt", sui.OK_CANCEL, "@base_player:special_sign_ui_title", signName, "handleSpecialSignSelection", true);
				sui.setPid(player, pid, player_structure.VAR_SPECIAL_SIGN_MENU_PID);
				return true;
			}
		}
		blog("terminal_structure.getSpecialSignManagementMenu: the menu was either just remove or remove and replace");
		
		String[] signList = new String[menuStrings.size()];
		String[] optionList = new String[menuOptions.size()];
		
		menuStrings.toArray(signList);
		menuOptions.toArray(optionList);
		
		blog("terminal_structure.getSpecialSignManagementMenu: menu options received");
		utils.setScriptVar(player, player_structure.VAR_SPECIAL_SIGN_MENU, optionList);
		
		int pid = sui.listbox(structure, player, "@base_player:special_sign_ui_prompt", sui.OK_CANCEL, "@base_player:special_sign_ui_title", signList, "handleSpecialSignManagementSelection", true);
		sui.setPid(player, pid, player_structure.VAR_SPECIAL_SIGN_MENU_PID);
		return true;
	}
	
	
	public boolean blog(String msg) throws InterruptedException
	{
		if (msg == null || msg.equals(""))
		{
			return false;
		}
		if (LOGGING_ON)
		{
			LOG(TERMINAL_LOGGING, msg);
		}
		
		return true;
	}
}
