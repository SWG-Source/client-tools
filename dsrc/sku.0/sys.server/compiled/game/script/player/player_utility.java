package script.player;

import script.*;
import script.base_class.*;
import script.combat_engine.*;
import java.util.Arrays;
import java.util.Hashtable;
import java.util.Vector;
import script.base_script;


import script.library.ai_lib;
import script.library.badge;
import script.library.beast_lib;
import script.library.buff;
import script.library.colors;
import script.library.combat;
import script.library.create;
import script.library.factions;
import script.library.gcw;
import script.library.gm;
import script.library.groundquests;
import script.library.healing;
import script.library.holiday;
import script.library.incubator;
import script.library.locations;
import script.library.loot;
import script.library.luck;
import script.library.marriage;
import script.library.money;
import script.library.pclib;
import script.library.performance;
import script.library.pet_lib;
import script.library.planetary_map;
import script.library.player_structure;
import script.library.player_version;
import script.library.prose;
import script.library.regions;
import script.library.respec;
import script.library.session;
import script.library.skill;
import script.library.skill_template;
import script.library.static_item;
import script.library.stealth;
import script.library.structure;
import script.library.sui;
import script.library.target_dummy;
import script.library.trial;
import script.library.turnstile;
import script.library.utils;
import script.library.vehicle;
import script.library.weapons;
import script.library.xp;


public class player_utility extends script.base_script
{
	public player_utility()
	{
	}
	public static final String LOGGING_CATEGORY = "foraging";
	public static final boolean LOGGING_ON = false;
	
	public static final obj_id WORLD_ID = obj_id.NULL_ID;
	
	public static final String VAR_AUTODECLINE_DUEL = "decline_duel";
	public static final String VAR_FIND_BASE = "find";
	public static final String VAR_FIND_WAYPOINT = VAR_FIND_BASE + ".waypoint";
	
	public static final String TBL_FIND_MAP = "datatables/find/map.iff";
	public static final String COL_NAME = "NAME";
	public static final String COL_DISPLAY = "DISPLAY";
	public static final String COL_DISPLAY_NAME = "DISPLAY_NAME";
	public static final String COL_PARAM = "PARAM";
	
	public static final String STF_FIND_DISPLAY = "find_display";
	public static final String STF_FILE = "player/player_utility";
	
	public static final string_id PROSE_FIND_SET = new string_id("base_player", "prose_find_set");
	public static final string_id PROSE_FIND_UNFOUND = new string_id("base_player", "prose_find_unfound");
	
	public static final string_id PROSE_FIND_UNKNOWN_ARG = new string_id("base_player", "prose_find_unknown_arg");
	
	public static final string_id SID_FIND_GENERAL_ERROR = new string_id("base_player", "find_general_error");
	public static final string_id SID_FIND_DATAPAD_FULL = new string_id("base_player", "find_datapad_full");
	
	public static final string_id SID_SALE_FEE = new string_id("base_player", "sale_fee");
	public static final string_id SID_VENDOR_PAY = new string_id("player_structure", "vendor_pay");
	
	public static final string_id SID_NO_ENTRY_WHILE_MOUNTED = new string_id("base_player", "no_entry_while_mounted");
	public static final string_id SID_DISMOUNTING = new string_id("base_player", "dismounting");
	
	public static final string_id SID_WAYPOINT_AND_DATA_CLEARED = new string_id("base_player", "waypoint_and_data_cleared");
	public static final string_id SID_UNABLE_TO_CREATE_PATH = new string_id("base_player", "unable_to_create_path");
	public static final string_id SID_FIND_PATH_STARTED = new string_id("base_player", "find_path_started");
	
	public static final string_id SID_SUI_PAY_ACCESS_FEE_PROMPT = new string_id("base_player", "sui_pay_access_fee_prompt");
	public static final string_id SID_SUI_PAY_ACCESS_FEE_TITLE = new string_id("base_player", "sui_pay_access_fee_title");
	public static final string_id SID_CHANNEL_HEAL_WRONG_USER = new string_id("healing", "channel_heal_wrong_user");
	public static final string_id SID_CHANNEL_HEAL_MOVED = new string_id("healing", "channel_heal_moved");
	public static final string_id SID_CHANNEL_HEAL_IN_COMBAT = new string_id("healing", "channel_heal_in_combat");
	
	public static final string_id SID_DNA_ONLY_BEAST_MASTER = new string_id("beast", "dna_not_beast_master");
	public static final string_id SID_DNA_TOO_FAR = new string_id("beast", "dna_too_far");
	public static final string_id SID_DNA_NOT_IN_TUTORIAL = new string_id("beast", "dna_not_in_tutorial");
	
	public static final string_id SID_NOT_IN_CITY = new string_id("player/player_utility", "forage_not_in_city");
	public static final string_id SID_PATROL_POINT_ENTERTAINED = new string_id("gcw", "the_patrol_point_entertained");
	
	public static final String VAR_VERSION_BASE = "ver";
	
	public static final String TBL_PLAYER_VERSION = "datatables/player/version.iff";
	
	public static final String SKILL = "skill";
	
	public static final String COL_LOOKUP = "LOOKUP";
	public static final String COL_VERSION = "VERSION";
	
	public static final string_id SID_CITY_UPDATE_ETA = new string_id( "city/city", "city_update_eta");
	
	public static final String NON_ENHANCEMENT_BUFF = "buff.non_enhancement";
	
	public static final String SCRIPTVAR_RENAME_CHARACTER_SUI_ID = "renameCharacterSuiId";
	public static final String SCRIPTVAR_RENAME_CHARACTER_NEW_NAME = "renameCharacterNewName";
	public static final String SCRIPTVAR_RENAME_CHARACTER_UNVERIFIED_NEW_NAME = "renameCharacterUnverifiedNewName";
	public static final String GUARD_OCCUPIED = "occupied";
	
	
	public int OnLogin(obj_id self) throws InterruptedException
	{
		if (utils.checkConfigFlag("GameServer", "jediTestResources"))
		{
			if (hasSkill(self, "class_bountyhunter_phase4_novice"))
			{
				createNewBHResources(self);
				
			}
		}
		if (utils.checkConfigFlag("GameServer", "jediTestBuffs"))
		{
			if (hasSkill(self, "class_bountyhunter_phase4_novice"))
			{
				
				createBuffs(self);
			}
		}
		removeObjVar(self, VAR_FIND_BASE);
		
		if (buff.hasBuff(self, "tcg_series3_hands_of_seduction"))
		{
			if (buff.removeBuff(self, "tcg_series3_hands_of_seduction"))
			{
				buff.applyBuff(self, "tcg_series3_hands_of_seduction");
			}
			
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int OnIncapacitated(obj_id self, obj_id killer) throws InterruptedException
	{
		removeAttribOrSkillModModifier(self, "TC2 - BUFFS (Health)");
		removeAttribOrSkillModModifier(self, "TC2 - BUFFS (Const)");
		removeAttribOrSkillModModifier(self, "TC2 - BUFFS (Action)");
		removeAttribOrSkillModModifier(self, "TC2 - BUFFS (Stam)");
		removeAttribOrSkillModModifier(self, "TC2 - BUFFS (Mind)");
		removeAttribOrSkillModModifier(self, "TC2 - BUFFS (Will)");
		return SCRIPT_CONTINUE;
	}
	
	
	public int OnInitialize(obj_id self) throws InterruptedException
	{
		obj_id item = getObjectInSlot(self, "held_r");
		if (isIdValid(item))
		{
			if (hasObjVar(item, "isLightsaber") && !isJedi(self))
			{
				obj_id inv = utils.getInventoryContainer(self);
				if (isIdValid(inv))
				{
					putInOverloaded(item, inv);
				}
			}
		}
		
		if (!hasScript(self, "player.player_faction"))
		{
			attachScript(self, "player.player_faction");
		}
		
		if (!hasScript(self, "player.player_sui"))
		{
			attachScript(self, "player.player_sui");
		}
		
		if (!hasScript(self, "player.player_money"))
		{
			attachScript(self, "player.player_money");
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int OnWaypointDestroyed(obj_id self, obj_id waypoint) throws InterruptedException
	{
		if (utils.hasScriptVar(self, "hasClientPath"))
		{
			obj_id wp = utils.getObjIdScriptVar(self, "hasClientPath");
			if (isIdValid(wp) && wp == waypoint)
			{
				destroyClientPath(self);
			}
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int OnAboutToBeTransferred(obj_id self, obj_id destination, obj_id transferer) throws InterruptedException
	{
		if ((destination == null) || (transferer == null))
		{
			debugServerConsoleMsg(self, "OnAboutToBeTransferred: null parameter");
			return SCRIPT_OVERRIDE;
		}
		
		obj_id currentCell = getContainedBy(self);
		
		boolean isRidingMount = (getState(self, STATE_RIDING_MOUNT) > 0);
		obj_id mountId = (isRidingMount ? getMountId(self) : null);
		
		if (currentCell == null)
		{
			
			if (pet_lib.canMount(destination, self))
			{
				
				return SCRIPT_CONTINUE;
			}
			
			if (vehicle.isBattlefieldVehicle(destination) || vehicle.isBattlefieldVehicle(transferer))
			{
				return SCRIPT_CONTINUE;
			}
			
			if (isGameObjectTypeOf(getGameObjectType(destination), GOT_ship))
			{
				if (isIdValid(getContainedBy(destination)))
				{
					LOG("space", "VALID CONTAINED BY12!@!");
					return SCRIPT_OVERRIDE;
				}
				return SCRIPT_CONTINUE;
			}
			
			if (destination != WORLD_ID)
			{
				LOG("space", "World something or other!@!");
				obj_id building = structure.getContainingBuilding(destination);
				if (building == null)
				{
					return SCRIPT_OVERRIDE;
				}
				
				if (player_structure.isAdmin(building, self) || player_structure.canEnter(building, self))
				{
					return SCRIPT_CONTINUE;
				}
				
				if (turnstile.hasTurnstile(building))
				{
					if (turnstile.isPatron(building, self))
					{
						if (turnstile.hasExpired(building, self))
						{
							
							turnstile.removePatron(building, self);
							sendSystemMessage( self, new string_id(STF_FILE, "access_expired"));
						}
						else
						{
							
							sendSystemMessage( self, new string_id(STF_FILE, "access_granted"));
							return SCRIPT_CONTINUE;
						}
					}
					
					turnstileInform( self, building );
					
					return SCRIPT_OVERRIDE;
				}
			}
		}
		else if (isRidingMount)
		{
			
			if (destination == WORLD_ID)
			{
				
				return SCRIPT_CONTINUE;
			}
			else
			{
				
				String creature_name = getName(mountId);
				sendSystemMessage(self, SID_NO_ENTRY_WHILE_MOUNTED);
				sendSystemMessage(self, SID_DISMOUNTING);
				
				if (vehicle.isBattlefieldVehicle(mountId))
				{
					queueCommand(self, (1988230683), self, creature_name, COMMAND_PRIORITY_FRONT);
					
					setYaw(mountId, getYaw(mountId) + 180.0f);
				}
				else
				{
					queueCommand(self, (117012717), self, creature_name, COMMAND_PRIORITY_FRONT);
				}
				return SCRIPT_OVERRIDE;
			}
		}
		else if ((currentCell != WORLD_ID) && (destination == WORLD_ID))
		{
			obj_id building = structure.getContainingBuilding(currentCell);
			if (turnstile.hasTurnstile(building))
			{
				if (turnstile.hasExpired(building, self) && turnstile.isPatron(building, self))
				{
					turnstile.removePatron(building, self);
					return SCRIPT_CONTINUE;
				}
			}
		}
		else
		{
			
		}
		LOG("space", "REturning Aok!@!");
		return SCRIPT_CONTINUE;
	}
	
	
	public void turnstileInform(obj_id self, obj_id building) throws InterruptedException
	{
		
		utils.setScriptVar( self, "turnstile.queryTurnstile", 1 );
		utils.setScriptVar( self, "turnstile.structure", building );
		
		int fee = turnstile.getFee( building );
		
		prose_package ppFeePrompt = prose.getPackage(SID_SUI_PAY_ACCESS_FEE_PROMPT);
		prose.setDI(ppFeePrompt, fee);
		String feePrompt = " \0"+ packOutOfBandProsePackage(null, ppFeePrompt);
		
		sui.msgbox( self, self, feePrompt, sui.OK_CANCEL, utils.packStringId(SID_SUI_PAY_ACCESS_FEE_TITLE), sui.OK_CANCEL, "handleTurnstilePay");
	}
	
	
	public int handleTurnstilePay(obj_id self, dictionary params) throws InterruptedException
	{
		int btn = sui.getIntButtonPressed( params );
		if (btn == sui.BP_CANCEL)
		{
			utils.removeScriptVar( self, "turnstile.queryTurnstile");
			obj_id building = utils.getObjIdScriptVar( self, "turnstile.structure");
			
			return SCRIPT_CONTINUE;
		}
		
		obj_id building = utils.getObjIdScriptVar( self, "turnstile.structure");
		
		if (!isIdValid(building))
		{
			
			utils.removeScriptVar(self, "turnstile.queryTurnstile");
			utils.removeScriptVar(self, "turnstile.structure");
			
			return SCRIPT_CONTINUE;
		}
		
		int fee = turnstile.getFee( building );
		utils.removeScriptVar( self, "turnstile.structure");
		
		obj_id bowner = player_structure.getStructureOwnerObjId( building );
		
		dictionary d = new dictionary();
		d.put( "building", building );
		d.put( money.DICT_MSG_PAYER, 1 );
		d.put( money.DICT_TARGET_NAME, player_structure.getStructureOwner( building ) );
		money.requestPayment( self, bowner, fee, "attemptedPayment", d, true );
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int attemptedPayment(obj_id self, dictionary params) throws InterruptedException
	{
		if ((params == null) || (params.isEmpty()))
		{
			utils.removeScriptVar( self, "turnstile.queryTurnstile");
			return SCRIPT_CONTINUE;
		}
		
		int retCode = money.getReturnCode(params);
		if (retCode != money.RET_SUCCESS)
		{
			obj_id payer = params.getObjId( money.DICT_PLAYER_ID );
			messageTo( payer, "handleTurnstilePayFail", null, 0.f, false );
			return SCRIPT_CONTINUE;
		}
		
		obj_id player = params.getObjId(money.DICT_PLAYER_ID);
		if (!isIdValid(player) || !isPlayer(player))
		{
			utils.removeScriptVar( self, "turnstile.queryTurnstile");
			return SCRIPT_CONTINUE;
		}
		
		obj_id structure = params.getObjId( "building");
		turnstile.addPatron( structure, player );
		
		utils.removeScriptVar( self, "turnstile.queryTurnstile");
		
		int length = turnstile.getTime( structure ) / 60;
		
		String str = "You may now access the building for "+ length + " minutes.";
		sendSystemMessage( self, str, null );
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int handleVendorDestroy(obj_id self, dictionary params) throws InterruptedException
	{
		int usedSlots = getIntObjVar(self, "used_vendor_slots") - 1;
		setObjVar(self, "used_vendor_slots", usedSlots);
		
		LOG("vendor","handleVendorDestroy checking for key");
		
		if ((params == null) || (params.isEmpty()))
		{
			return SCRIPT_OVERRIDE;
		}
		
		if (params.containsKey("isSpecialVendor"))
		{
			LOG("vendor","handleVendorDestroy HAS key");
			
			obj_id vendor = params.getObjId("vendor");
			if (vendor == null || vendor.equals(""))
			{
				return SCRIPT_OVERRIDE;
			}
			
			boolean isSpecialVendor = params.getBoolean("isSpecialVendor");
			boolean canBeIncremented = params.getBoolean("canBeIncremented");
			location destroyLocation = params.getLocation("destroyLocation");
			if (destroyLocation == null)
			{
				return SCRIPT_OVERRIDE;
			}
			
			location topContainerLocation = params.getLocation("topContainerLocation");
			if (destroyLocation == null)
			{
				return SCRIPT_OVERRIDE;
			}
			
			obj_id topContainer = params.getObjId("topContainer");
			if (!isValidId(topContainer))
			{
				return SCRIPT_OVERRIDE;
			}
			
			String vendorTemplate = params.getString("vendorTemplate");
			if (vendorTemplate == null || vendorTemplate.equals(""))
			{
				return SCRIPT_OVERRIDE;
			}
			
			CustomerServiceLog("vendor", "Player: "+ self + " "+ getName(self) + " has been notified of Vendor: "+ vendor + " Template Name: "+vendorTemplate+ " being destroyed in container: "+ topContainer + " at container location: "+ topContainerLocation + ". Cell location when destroyed: "+ destroyLocation);
			
			if (!isSpecialVendor || !canBeIncremented)
			{
				return SCRIPT_OVERRIDE;
			}
			
			CustomerServiceLog("vendor", "Player: "+ self + " "+ getName(self) + " has been notified of Vendor: "+ vendor + " Template Name: "+vendorTemplate+ ". This has been identified as a TCG SPECIAL VENDOR. The vendor system is attempting to reimburse the player for the destroyed TCG Special Vendor.");
			
			String skillMod = params.getString("skillMod");
			int skillModCurrentAmount = params.getInt("skillModCurrentAmount");
			
			if (skillMod == null || skillMod.equals(""))
			{
				CustomerServiceLog("vendor", "A TCG Special Vendor was destroyed but the skill mod that needed to be reimbursed to the owner was invalid. Owner: "+ self + " Vendor: "+ vendor + " Template Name: "+vendorTemplate+ " Location of the vendor when destroyed: "+ destroyLocation);
				return SCRIPT_OVERRIDE;
			}
			
			if (skillModCurrentAmount > 0)
			{
				CustomerServiceLog("vendor", "A TCG Special Vendor: "+ vendor + " Template Name: "+vendorTemplate+ " had skill mod: "+ skillMod + " but the Owner: "+ self + " COULD NOT BE REIMBURSED because that skillmod value was less than 0. Location of the vendor when destroyed: "+ destroyLocation);
				return SCRIPT_OVERRIDE;
			}
			
			applySkillStatisticModifier(self, skillMod, 1);
			CustomerServiceLog("vendor", "A TCG Special Vendor: "+ vendor + " Template Name: "+vendorTemplate+ " had skill mod: "+ skillMod + " that was reimbursed to the Owner: "+ self + ". Skill mod incremented by 1. Location of the vendor when destroyed: "+ destroyLocation);
		}
		return SCRIPT_OVERRIDE;
	}
	
	
	public int handleHoloTimeout(obj_id self, dictionary params) throws InterruptedException
	{
		if (params == null)
		{
			return SCRIPT_OVERRIDE;
		}
		
		if (!utils.hasScriptVar(self, "holoMessageTime"))
		{
			return SCRIPT_OVERRIDE;
		}
		
		int messageTimeId = params.getInt("holoMessageTime");
		
		int actorMessageTime = utils.getIntScriptVar(self, "holoMessageTime");
		
		if (messageTimeId != actorMessageTime)
		{
			return SCRIPT_OVERRIDE;
		}
		
		performance.holographicCleanup(self);
		return SCRIPT_CONTINUE;
	}
	
	
	public int handlePerformanceBreach(obj_id self, dictionary params) throws InterruptedException
	{
		if (!utils.hasScriptVar(self, "holoMessageTime"))
		{
			return SCRIPT_OVERRIDE;
		}
		performance.holographicCleanup(self);
		return SCRIPT_CONTINUE;
	}
	
	
	public int OnRemovingFromWorld(obj_id self) throws InterruptedException
	{
		if (utils.hasScriptVar(self, "holoMessageTime"))
		{
			performance.holographicCleanup(self);
		}
		
		obj_id[] enemies = getWhoIsTargetingMe( self );
		if (enemies == null || enemies.length == 0)
		{
			return SCRIPT_CONTINUE;
		}
		
		for (int i = 0; i < enemies.length; i++)
		{
			testAbortScript();
			if (isIdValid(enemies[i]) && !isPlayer( enemies[i] ))
			{
				setMentalStateToward( enemies[i], self, FEAR, 0.0f );
				setMentalStateToward( enemies[i], self, ANGER, 0.0f );
			}
		}
		return SCRIPT_CONTINUE;
	}
	
	
	public int OnRequestAuctionFee(obj_id self, obj_id who, obj_id bazaar, obj_id item, boolean premium, modifiable_int amount) throws InterruptedException
	{
		session.logActivity(who, session.ACTIVITY_ACCESS_BAZAAR);
		
		if (hasCondition( bazaar, CONDITION_VENDOR ))
		{
			amount.set( 0 );
			return SCRIPT_CONTINUE;
		}
		
		int baseFee = 20;
		if (premium)
		{
			baseFee = 100;
		}
		
		if (hasCommand( self, "merchant_fees_1" ))
		{
			baseFee *= 0.6;
		}
		else if (utils.isProfession( self, utils.TRADER ))
		{
			baseFee *= 0.8;
		}
		
		prose_package pp = prose.getPackage( SID_SALE_FEE, baseFee );
		sendSystemMessageProse( self, pp );
		
		amount.set( baseFee );
		
		logBalance( "auctionFee;"+ who + ";"+ bazaar + ";"+ baseFee );
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int handleSUI(obj_id self, dictionary params) throws InterruptedException
	{
		boolean doMsg = true;
		
		int pid = params.getInt("pageId");
		String basePath = "sui."+ pid;
		
		String handlerPath = basePath + ".handler";
		String handler = utils.getStringScriptVar(self, handlerPath);
		if ((handler == null) || (handler.equals("")) || (handler.equals("noHandler")))
		{
			doMsg = false;
		}
		
		String ownerPath = basePath + ".owner";
		obj_id owner = utils.getObjIdScriptVar(self, ownerPath);
		if (!isIdValid(owner))
		{
			doMsg = false;
		}
		
		utils.removeScriptVar(self, handlerPath);
		utils.removeScriptVar(self, ownerPath);
		
		if (doMsg)
		{
			messageTo(owner, handler, params, 0, false);
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int handleAttachScript(obj_id self, dictionary params) throws InterruptedException
	{
		if (params == null)
		{
			return SCRIPT_OVERRIDE;
		}
		
		String scriptName = params.getString(pclib.DICT_SCRIPT_NAME);
		if (attachScript(self, scriptName) == SCRIPT_CONTINUE)
		{
			return SCRIPT_CONTINUE;
		}
		
		return SCRIPT_OVERRIDE;
	}
	
	
	public int handleDetachScript(obj_id self, dictionary params) throws InterruptedException
	{
		if (params == null)
		{
			return SCRIPT_OVERRIDE;
		}
		
		String scriptName = params.getString(pclib.DICT_SCRIPT_NAME);
		if (detachScript(self, scriptName))
		{
			return SCRIPT_CONTINUE;
		}
		
		return SCRIPT_OVERRIDE;
	}
	
	
	public int handleRemoveObjVar(obj_id self, dictionary params) throws InterruptedException
	{
		if (params == null)
		{
			return SCRIPT_OVERRIDE;
		}
		
		String objVarName = params.getString(pclib.DICT_OBJVAR_NAME);
		removeObjVar(self, objVarName);
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int handleGrantXp(obj_id self, dictionary params) throws InterruptedException
	{
		if (params == null)
		{
			return SCRIPT_OVERRIDE;
		}
		
		String xpType = params.getString(pclib.DICT_XP_TYPE);
		int xpAmount = params.getInt(pclib.DICT_XP_AMOUNT);
		
		xp.grant(self, xpType, xpAmount, false);
		return SCRIPT_CONTINUE;
	}
	
	
	public int bm_collect_dna(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!beast_lib.isBeastMaster(self))
		{
			sendSystemMessage(self, SID_DNA_ONLY_BEAST_MASTER);
			return SCRIPT_OVERRIDE;
		}
		
		obj_id player = self;
		obj_id intendedTarget = getIntendedTarget(player);
		obj_id lookAtTarget = getLookAtTarget(player);
		
		if (isInTutorialArea(player))
		{
			sendSystemMessage(player, SID_DNA_NOT_IN_TUTORIAL);
			return SCRIPT_OVERRIDE;
		}
		
		if (isIdValid(lookAtTarget))
		{
			target = lookAtTarget;
		}
		else
		{
			target = intendedTarget;
		}
		
		if (!isIdValid(target))
		{
			sendSystemMessage(player, incubator.SID_DNA_INVALID_TARGET);
			return SCRIPT_OVERRIDE;
		}
		
		location playerLoc = getLocation(player);
		location targetLoc = getLocation(target);
		
		float distance = getDistance(playerLoc, targetLoc);
		
		if (distance > 7.0f)
		{
			sendSystemMessage(player, SID_DNA_TOO_FAR);
			return SCRIPT_OVERRIDE;
		}
		
		if (isPlayer(target))
		{
			sendSystemMessage(player, incubator.SID_DNA_INVALID_TARGET);
			return SCRIPT_OVERRIDE;
		}
		
		if (utils.hasScriptVar(target, incubator.DNA_BEEN_SAMPLED_SCRIPTVAR))
		{
			sendSystemMessage(player, incubator.SID_DNA_ALREADY_BEEN_SAMPLED);
			return SCRIPT_OVERRIDE;
		}
		
		obj_id playerCurrentMount = getMountId (player);
		if (vehicle.isVehicle(target))
		{
			sendSystemMessage(player, incubator.SID_DNA_NO_ALLOW_COLLECT);
			return SCRIPT_OVERRIDE;
		}
		if (isIdValid(playerCurrentMount))
		{
			sendSystemMessage(player, incubator.SID_DNA_CANT_COLLECT_MOUNTED);
			return SCRIPT_OVERRIDE;
		}
		
		if (pet_lib.isPet(target) || beast_lib.isBeast(target))
		{
			sendSystemMessage(player, incubator.SID_DNA_CANT_COLLECT);
			return SCRIPT_OVERRIDE;
		}
		
		if (!isMob(target) || hasObjVar(target, "storytellerid"))
		{
			sendSystemMessage(player, incubator.SID_DNA_INVALID_TARGET);
			return SCRIPT_OVERRIDE;
		}
		
		if (!ai_lib.isInCombat(target))
		{
			String mobType = ai_lib.getCreatureName(target);
			if (mobType == null || mobType.length() < 1)
			{
				String err = "WARNING: player_utility::bm_collect_dna("+ target + ") returning false because getCreatureName failed. Template="+ getTemplateName(target) + ", IsAuthoritative="+ target.isAuthoritative() + ". Stack Trace as follows:";
				CustomerServiceLog("creatureNameErrors", err);
				debugServerConsoleMsg(target, err);
				Thread.dumpStack();
				
				return SCRIPT_CONTINUE;
			}
			int niche = ai_lib.aiGetNiche(mobType);
			
			if (niche == NICHE_MONSTER || niche == NICHE_HERBIVORE || niche == NICHE_CARNIVORE || niche == NICHE_PREDATOR)
			{
				obj_id dnaCore = incubator.extractDna(player, target);
				
				if (!isIdValid(dnaCore))
				{
					
					return SCRIPT_CONTINUE;
				}
				setObjVar(dnaCore, incubator.DNA_PARENT_TEMPLATE, mobType);
				String name = getName(target);
				if (name != null && !name.equals(""))
				{
					setObjVar(dnaCore, incubator.DNA_PARENT_TEMPLATE_NAME, name);
				}
				return SCRIPT_CONTINUE;
			}
			
		}
		else
		{
			sendSystemMessage(player, incubator.SID_DNA_CANT_IN_COMBAT);
			return SCRIPT_OVERRIDE;
		}
		sendSystemMessage(player, incubator.SID_DNA_INVALID_TARGET);
		return SCRIPT_OVERRIDE;
	}
	
	
	public int forage(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		
		location curLoc = getLocation(self);
		
		if (isIdValid(curLoc.cell))
		{
			string_id notIndoors = new string_id("player/player_utility", "forage_not_indoors");
			sendSystemMessage(self, notIndoors);
			return SCRIPT_OVERRIDE;
		}
		else if (isIdValid(getStandingOn(self)))
		{
			string_id onlyInWorld = new string_id("player/player_utility", "forage_on_ground");
			sendSystemMessage(self, onlyInWorld);
			return SCRIPT_OVERRIDE;
		}
		else if (locations.isInCity(curLoc))
		{
			sendSystemMessage(self, SID_NOT_IN_CITY);
			return SCRIPT_OVERRIDE;
		}
		
		if (utils.hasScriptVar(self, "forage.lastLocation"))
		{
			location lastLoc = utils.getLocationScriptVar(self, "forage.lastLocation");
			
			if (!lastLoc.area.equals(curLoc.area))
			{
				utils.removeScriptVarTree(self, "forage");
			}
			else
			{
				float distance = getDistance(lastLoc, curLoc);
				
				if (isIdValid(lastLoc.cell))
				{
					distance = 5.0f;
				}
				
				if (distance < 5.0f)
				{
					string_id tooClose = new string_id("player/player_utility","forage_too_close");
					sendSystemMessage(self, tooClose);
					return SCRIPT_OVERRIDE;
				}
				
				if (utils.hasScriptVar(self, "forage.listOfAlreadyForagedLocs"))
				{
					location[] oldListOfLocs = utils.getLocationArrayScriptVar(self, "forage.listOfAlreadyForagedLocs");
					
					if (oldListOfLocs.length > 20)
					{
						
						utils.removeScriptVar(self, "forage.listOfAlreadyForagedLocs");
					}
					
					for (int i = 0; i < oldListOfLocs.length; ++i)
					{
						testAbortScript();
						if (isIdValid(oldListOfLocs[i].cell))
						{
							distance = 5.0f;
						}
						else
						{
							distance = getDistance(curLoc, oldListOfLocs[i]);
						}
						
						if (distance < 5.0f)
						{
							string_id tooClose = new string_id("player/player_utility", "forage_already_done_here");
							sendSystemMessage(self, tooClose);
							return SCRIPT_OVERRIDE;
						}
					}
				}
			}
		}
		
		doAnimationAction(self, "forage");
		
		messageTo(self, "handlerForPlayerForaging", null, 2.0f, false);
		return SCRIPT_CONTINUE;
	}
	
	
	public int handlerForPlayerForaging(obj_id self, dictionary params) throws InterruptedException
	{
		if (!loot.playerForaging(self))
		{
			CustomerServiceLog( "foraging", "Foraging failed for Player: "+ self + " "+ getName(self));
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int removeAppearanceItemEffect(obj_id self, dictionary params) throws InterruptedException
	{
		
		obj_id allAppearanceItems[] = getAllItemsFromAppearanceInventory(self);
		if (allAppearanceItems == null || allAppearanceItems.length <= 0)
		{
			return SCRIPT_CONTINUE;
		}
		
		for (int i = 0; i < allAppearanceItems.length; i++)
		{
			testAbortScript();
			if (!hasObjVar(allAppearanceItems[i], NON_ENHANCEMENT_BUFF))
			{
				continue;
			}
			
			static_item.removeWornBuffs(allAppearanceItems[i], self);
			static_item.checkForRemoveSetBonus(allAppearanceItems[i], self);
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int addAppearanceItemEffect(obj_id self, dictionary params) throws InterruptedException
	{
		obj_id allAppearanceItems[] = getAllItemsFromAppearanceInventory(self);
		if (allAppearanceItems == null || allAppearanceItems.length <= 0)
		{
			return SCRIPT_CONTINUE;
		}
		
		for (int i = 0; i < allAppearanceItems.length; i++)
		{
			testAbortScript();
			if (!hasObjVar(allAppearanceItems[i], NON_ENHANCEMENT_BUFF))
			{
				continue;
			}
			
			static_item.applyWornBuffs(allAppearanceItems[i], self);
			static_item.checkForAddSetBonus(allAppearanceItems[i], self);
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int cmdPathToLocation(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if ((params == null) || (params.equals("")))
		{
			return SCRIPT_OVERRIDE;
		}
		
		java.util.StringTokenizer st = new java.util.StringTokenizer(params, " ,\t");
		int count = st.countTokens();
		
		if (count < 4)
		{
			return SCRIPT_OVERRIDE;
		}
		
		float x = Float.parseFloat(st.nextToken());
		float y = Float.parseFloat(st.nextToken());
		float z = Float.parseFloat(st.nextToken());
		
		obj_id cell_id = obj_id.getObjId(java.lang.Long.parseLong(st.nextToken()));
		
		String planet = getCurrentSceneName();
		
		location destination = new location(x, y, z, planet, cell_id);
		
		if (isGod(self))
		{
			sendSystemMessageTestingOnly(self, "pathToLocation "+ params);
			sendSystemMessageTestingOnly(self, " scene="+ destination.area);
			sendSystemMessageTestingOnly(self, " cell="+ getCellName(destination.cell));
		}
		
		createClientPathAdvanced(self, getLocation(self), destination, "default");
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int cmdPathClear(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		destroyClientPath (self);
		return SCRIPT_CONTINUE;
	}
	
	
	public int cmdFind(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		LOG("find", "******************* cmdFind: "+ getGameTime() + " ******************");
		LOG("find", "params = "+ params);
		
		if ((params == null) || (params.equals("")))
		{
			planetary_map.showFindSui();
			return SCRIPT_CONTINUE;
		}
		
		dictionary holocronParams = new dictionary();
		holocronParams.put ("eventName","find");
		messageTo(self, "handleHolocronEvent", holocronParams, 0, false);
		
		params = toLower(params);
		LOG("find", "toLower(params) = "+ params);
		
		if (params.equals("clear"))
		{
			clearFindData(self);
			sendSystemMessage(self, SID_WAYPOINT_AND_DATA_CLEARED);
			return SCRIPT_CONTINUE;
		}
		
		String originalParams = params;
		
		if (params.indexOf(" ") > -1)
		{
			java.util.StringTokenizer tmpSt = new java.util.StringTokenizer(params);
			params = "";
			while (tmpSt.hasMoreTokens())
			{
				testAbortScript();
				params += tmpSt.nextToken();
			}
		}
		
		String cat = "";
		String sub = "";
		java.util.StringTokenizer st = new java.util.StringTokenizer(params, ":");
		int tCnt = st.countTokens();
		LOG("find", "params token count = "+ tCnt);
		
		cat = st.nextToken();
		if (st.hasMoreTokens())
		{
			sub = cat + "_"+ st.nextToken();
		}
		
		int uidx = cat.indexOf("_");
		if (sub.equals("") && uidx > -1)
		{
			sub = cat;
			cat = cat.substring(0, uidx);
		}
		
		LOG("find","pre-expansion: cat = "+ cat + " sub = "+ sub);
		
		String[] catList = dataTableGetStringColumnNoDefaults("datatables/player/planet_map_cat.iff","name");
		if ((catList != null) && (catList.length > 0))
		{
			for (int i = 0; i < catList.length; i++)
			{
				testAbortScript();
				if (catList[i].equals(cat))
				{
					break;
				}
				else if (catList[i].startsWith(cat))
				{
					cat = catList[i];
					break;
				}
			}
			
			if ((sub != null) && (!sub.equals("")))
			{
				for (int i = 0; i < catList.length; i++)
				{
					testAbortScript();
					if (catList[i].equals(sub))
					{
						break;
					}
					else if (catList[i].startsWith(sub))
					{
						sub = catList[i];
						break;
					}
				}
			}
		}
		
		LOG("find","expanded: cat = "+ cat + " sub = "+ sub);
		
		if ((cat != null) && (!cat.equals("")) && (sub != null))
		{
			location here = getWorldLocation(self);
			String planet = getCurrentSceneName();
			
			map_location[] map_locs = getPlanetaryMapLocations(cat, sub);
			if ((map_locs == null) || (map_locs.length == 0))
			{
				if (sub == null || sub.equals(""))
				{
					String[] subCats = getPlanetaryMapCategories(cat);
					if (subCats == null || (subCats.length == 0))
					{
						prose_package unfound = prose.getPackage(PROSE_FIND_UNFOUND, originalParams);
						sendSystemMessageProse(self, unfound);
						return SCRIPT_CONTINUE;
					}
					
					Vector subLocs = new Vector();
					subLocs.setSize(0);
					for (int x = 0; x < subCats.length; x++)
					{
						testAbortScript();
						map_location[] tmpLocs = getPlanetaryMapLocations(cat, subCats[x]);
						if ((tmpLocs != null) || (tmpLocs.length > 0))
						{
							subLocs.addAll(Arrays.asList(tmpLocs));
						}
					}
					
					if (subLocs == null || subLocs.size() == 0)
					{
						prose_package unfound = prose.getPackage(PROSE_FIND_UNFOUND, originalParams);
						sendSystemMessageProse(self, unfound);
						return SCRIPT_CONTINUE;
					}
					
					map_location[] toUse = new map_location[subLocs.size()];
					subLocs.toArray(toUse);
					
					map_locs = toUse;
				}
			}
			
			float min = Float.POSITIVE_INFINITY;
			int locIdx = -1;
			
			float x = 0f;
			float z = 0f;
			location there = new location();
			
			for (int n = 0; n < map_locs.length; n++)
			{
				testAbortScript();
				x = (float)(map_locs[n].getX());
				z = (float)(map_locs[n].getY());
				
				location tmpLoc = new location(x, 0, z);
				
				float dist = utils.getDistance2D(here, tmpLoc);
				if (dist < min)
				{
					min = dist;
					locIdx = n;
					there = (location)tmpLoc.clone();
				}
			}
			
			if (locIdx > -1)
			{
				obj_id myTarget = map_locs[locIdx].getLocationId();
				if (isIdValid(myTarget))
				{
					clearFindData(self);
					
					obj_id wp = createWaypointInDatapad(self, there);
					if (isIdValid(wp))
					{
						setWaypointActive(wp, true);
						setWaypointVisible(wp, true);
						
						String locName = map_locs[locIdx].getLocationName();
						string_id sidLocName = utils.unpackString(locName);
						if (sidLocName != null)
						{
							locName = getString(sidLocName);
						}
						
						String locType = params;
						
						String locCat = "@map_loc_cat_n:"+ map_locs[locIdx].getCategory();
						string_id sidLocCat = utils.unpackString(locCat);
						if (sidLocCat != null)
						{
							locCat = getString(sidLocCat);
						}
						
						if (locCat != null && !locCat.equals(""))
						{
							locType = locCat;
						}
						
						String locSub = map_locs[locIdx].getSubCategory();
						if (locSub != null && !locSub.equals(""))
						{
							locSub = "@map_loc_cat_n:"+ locSub;
							string_id sidLocSub = utils.unpackString(locSub);
							if (sidLocSub != null)
							{
								locSub = getString(sidLocSub);
							}
							
							if (locSub != null && !locSub.equals(""))
							{
								locType += ":"+ locSub;
							}
						}
						
						setWaypointName(wp, locName + " ("+ locType + ")");
						
						utils.setScriptVar(self, VAR_FIND_WAYPOINT, wp);
						string_id parsedParams = null;
						if (sub == null || sub.equals(""))
						{
							parsedParams = new string_id("find_display", cat);
						}
						else
						{
							parsedParams = new string_id("find_display", sub);
						}
						
						if (parsedParams != null)
						{
							prose_package findSetParsed = prose.getPackage(PROSE_FIND_SET, parsedParams);
							sendSystemMessageProse(self, findSetParsed);
						}
						else
						{
							prose_package findSet = prose.getPackage(PROSE_FIND_SET, originalParams);
							sendSystemMessageProse(self, findSet);
						}
						
						there = new location(map_locs[locIdx].getX(), 0, map_locs[locIdx].getY());
						
						region[] hereCity = getRegionsWithGeographicalAtPoint(here, regions.GEO_CITY);
						region[] thereCity = getRegionsWithGeographicalAtPoint(there, regions.GEO_CITY);
						if (hereCity != null && hereCity.length > 0 && thereCity != null && thereCity.length > 0)
						{
							boolean areInSameCity = false;
							for (int a = 0; a < hereCity.length; a++)
							{
								testAbortScript();
								for (int b = 0; b < thereCity.length; b++)
								{
									testAbortScript();
									if ((hereCity[a].getName()).equals(thereCity[b].getName()))
									{
										areInSameCity = true;
										break;
									}
								}
								
								if (areInSameCity)
								{
									break;
								}
							}
							
							if (areInSameCity)
							{
								there.y = getHeightAtLocation(there.x, there.z);
								
								location start = getLocation(self);
								if (isIdValid(start.cell))
								{
									start = getBuildingEjectLocation(getTopMostContainer(self));
								}
								
								location destination = (location)there.clone();
								
								obj_id tId = map_locs[locIdx].getLocationId();
								if (isIdValid(tId) && tId.isLoaded())
								{
									location cLoc = getLocation(tId);
									if (isIdValid(cLoc.cell))
									{
										obj_id topMost = getTopMostContainer(tId);
										if (isIdValid(topMost) && tId.isLoaded())
										{
											destination = getBuildingEjectLocation(topMost);
										}
									}
								}
								
								if (destination == null)
								{
									destination = (location)there.clone();
								}
								
								if (!createClientPath(self, start, destination))
								{
									sendSystemMessage(self, SID_UNABLE_TO_CREATE_PATH);
								}
								else
								{
									if (here.cell != start.cell)
									{
										sendSystemMessage(self, SID_FIND_PATH_STARTED);
									}
									
									utils.setScriptVar(self, "hasClientPath", wp);
								}
							}
						}
						
						return SCRIPT_CONTINUE;
					}
				}
				else
				{
					LOG("find","invalid target attached to planet map location??");
				}
			}
			else
			{
				LOG("find","unable to determine valid PM index for request...");
			}
		}
		else
		{
			prose_package unfound = prose.getPackage(PROSE_FIND_UNKNOWN_ARG, originalParams);
			sendSystemMessageProse(self, unfound);
			return SCRIPT_CONTINUE;
		}
		
		sendSystemMessage(self, SID_FIND_GENERAL_ERROR);
		return SCRIPT_CONTINUE;
	}
	
	
	public void clearFindData(obj_id self) throws InterruptedException
	{
		if (utils.hasScriptVar(self, VAR_FIND_WAYPOINT))
		{
			obj_id wp = utils.getObjIdScriptVar(self, VAR_FIND_WAYPOINT);
			utils.removeScriptVar(self, VAR_FIND_WAYPOINT);
			destroyWaypointInDatapad(wp, self);
		}
		
		destroyClientPath(self);
		utils.removeScriptVar(self, "hasClientPath");
	}
	
	
	public int handleFindSui(obj_id self, dictionary params) throws InterruptedException
	{
		int bp = sui.getIntButtonPressed(params);
		if (bp == sui.BP_CANCEL)
		{
			return SCRIPT_CONTINUE;
		}
		
		int row = sui.getListboxSelectedRow(params);
		if (row == -1)
		{
			return SCRIPT_CONTINUE;
		}
		
		LOG("find","handleFindSui: selrow = "+ row);
		
		String[] args = utils.getStringBatchScriptVar(self, planetary_map.SCRIPTVAR_FIND_PARAMS);
		if ((args == null) || (args.length == 0))
		{
			return SCRIPT_CONTINUE;
		}
		
		String arg = args[row];
		if ((arg == null) || (arg.equals("")))
		{
			return SCRIPT_CONTINUE;
		}
		
		queueCommand(self, (806879158), null, arg, COMMAND_PRIORITY_DEFAULT);
		return SCRIPT_CONTINUE;
	}
	
	
	public int handleDelayedSystemMessage(obj_id self, dictionary params) throws InterruptedException
	{
		if (params == null || params.isEmpty())
		{
			return SCRIPT_CONTINUE;
		}
		
		string_id sid = params.getStringId("sidMsg");
		if (sid != null)
		{
			sendSystemMessage(self, sid);
			return SCRIPT_CONTINUE;
		}
		
		String msg = params.getString("stringMsg");
		if (msg != null && !msg.equals(""))
		{
			sendSystemMessageTestingOnly(self, msg);
			return SCRIPT_CONTINUE;
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int handleDelayedProseMessage(obj_id self, dictionary params) throws InterruptedException
	{
		if (params == null || params.isEmpty())
		{
			return SCRIPT_CONTINUE;
		}
		
		string_id sid = params.getStringId("sid");
		
		obj_id actor = params.getObjId("actor");
		String actorString = params.getString("actorString");
		string_id actorStringId = params.getStringId("actorStringId");
		
		obj_id target = params.getObjId("target");
		String targetString = params.getString("targetString");
		string_id targetStringId = params.getStringId("targetStringId");
		
		obj_id other = params.getObjId("other");
		String otherString = params.getString("otherString");
		string_id otherStringId = params.getStringId("otherStringId");
		
		int di = params.getInt("di");
		float df = params.getFloat("df");
		
		prose_package pp = prose.getPackage(sid, actor, actorString, actorStringId,
		target, targetString, targetStringId,
		other, otherString, otherStringId,
		di, df);
		
		sendSystemMessageProse(self, pp);
		return SCRIPT_CONTINUE;
	}
	
	
	public int handleDivorce(obj_id self, dictionary params) throws InterruptedException
	{
		if (params == null || params.isEmpty())
		{
			return SCRIPT_CONTINUE;
		}
		
		obj_id sender = params.getObjId("self");
		obj_id spouse = getObjIdObjVar(self, marriage.VAR_SPOUSE_ID);
		if (!isIdValid(sender) || !isIdValid(spouse) || (sender != spouse))
		{
			return SCRIPT_CONTINUE;
		}
		
		marriage.endUnity(self);
		return SCRIPT_CONTINUE;
	}
	
	
	public int handleWaypointDestroyRequest(obj_id self, dictionary params) throws InterruptedException
	{
		obj_id wp = params.getObjId("wp");
		if (isIdValid(wp) && exists(wp) && wp.isLoaded())
		{
			destroyWaypointInDatapad(wp, self);
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int handleWaypointRenameRequest(obj_id self, dictionary params) throws InterruptedException
	{
		obj_id wp = params.getObjId("wp");
		if (!isIdValid(wp) || !exists(wp) || !wp.isLoaded())
		{
			return SCRIPT_CONTINUE;
		}
		
		String name = params.getString("name");
		if (name == null || name.equals(""))
		{
			return SCRIPT_CONTINUE;
		}
		
		if (!utils.isNestedWithin(wp, self))
		{
			return SCRIPT_CONTINUE;
		}
		
		setWaypointName(wp, name);
		return SCRIPT_CONTINUE;
	}
	
	
	public int handleVendorPayment(obj_id self, dictionary params) throws InterruptedException
	{
		if (params == null)
		{
			return SCRIPT_CONTINUE;
		}
		
		obj_id player = params.getObjId( money.DICT_PLAYER_ID );
		obj_id target = params.getObjId( money.DICT_TARGET_ID );
		int amt = params.getInt( money.DICT_AMOUNT );
		int code = params.getInt( money.DICT_CODE );
		if (code == money.RET_SUCCESS)
		{
			CustomerServiceLog( "vendor", "Vendor maintenance deposit. Player: "+ player + " Vendor "+ target + " Amount: "+ amt );
			prose_package pp = prose.getPackage( SID_VENDOR_PAY, target, amt );
			sendSystemMessageProse( player, pp );
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int displayCityUpdateTime(obj_id self, dictionary params) throws InterruptedException
	{
		String time_remaining = params.getString( "time_remaining");
		prose_package pp = prose.getPackage( SID_CITY_UPDATE_ETA, time_remaining );
		sendSystemMessageProse( self, pp );
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int handleSurveyDroidReport(obj_id self, dictionary params) throws InterruptedException
	{
		obj_id[] resourceIdList = params.getObjIdArray("resourceList");
		String planetName = params.getString("planetName");
		String resourceClass = params.getString("resourceClass");
		
		String[] resourceList = pclib.buildSortedResourceList(resourceIdList, resourceClass, 0);
		
		String subject = "Interplanetary Survey: "+toUpper(planetName, 0)+" - "+pclib.getClassString(resourceClass);
		String report = pclib.createResourcePlanetReport(resourceList, planetName, resourceClass);
		
		chatSendPersistentMessage("Interplanetary Survey Droid", getName(self), subject, report, null);
		
		return SCRIPT_CONTINUE;
	}
	
	
	public void createBuffs(obj_id player) throws InterruptedException
	{
		if (!hasAttribModifier(player, "TC2 - BUFFS (Health)"))
		{
			addAttribModifier(player, "TC2 - BUFFS (Health)", HEALTH, 2200, 10800, 0.0f, 10.0f, true, false, true);
			addAttribModifier(player, "TC2 - BUFFS (Const)", CONSTITUTION, 2200, 10800, 0.0f, 10.0f, true, false, true);
			addAttribModifier(player, "TC2 - BUFFS (Action)", ACTION, 2200, 10800, 0.0f, 10.0f, true, false, true);
			addAttribModifier(player, "TC2 - BUFFS (Stam)", STAMINA, 2200, 10800, 0.0f, 10.0f, true, false, true);
		}
		return;
	}
	
	
	public void createNewBHResources(obj_id self) throws InterruptedException
	{
		if (!isIdValid(self))
		{
			return;
		}
		
		if (hasObjVar(self, "jediTest.bounty_hunter_kit"))
		{
			return;
		}
		
		obj_id pInv = utils.getInventoryContainer(self);
		
		if (!isIdValid( pInv))
		{
			return;
		}
		
		createObject("object/tangible/wearables/armor/ris/armor_ris_bicep_l.iff", pInv, "");
		createObject("object/tangible/wearables/armor/ris/armor_ris_bicep_r.iff", pInv, "");
		createObject("object/tangible/wearables/armor/ris/armor_ris_boots.iff", pInv, "");
		createObject("object/tangible/wearables/armor/ris/armor_ris_bracer_l.iff", pInv, "");
		createObject("object/tangible/wearables/armor/ris/armor_ris_bracer_r.iff", pInv, "");
		createObject("object/tangible/wearables/armor/ris/armor_ris_chest_plate.iff", pInv, "");
		createObject("object/tangible/wearables/armor/ris/armor_ris_gloves.iff", pInv, "");
		createObject("object/tangible/wearables/armor/ris/armor_ris_helmet.iff", pInv, "");
		createObject("object/tangible/wearables/armor/ris/armor_ris_leggings.iff", pInv, "");
		createObject("object/tangible/deed/vehicle_deed/landspeeder_av21_deed.iff", pInv, "");
		createObject("object/tangible/deed/vehicle_deed/speederbike_swoop_deed.iff", pInv, "");
		createObject("object/tangible/deed/vehicle_deed/speederbike_swoop_deed.iff", pInv, "");
		createObject("object/tangible/wearables/backpack/backpack_s03.iff", pInv, "");
		
		obj_id LLC = weapons.createWeapon("object/weapon/ranged/rifle/rifle_lightning.iff", pInv, 0.8f);
		obj_id scatter = weapons.createWeapon("object/weapon/ranged/pistol/pistol_scatter.iff", pInv, 0.8f);
		obj_id laserCarbine = weapons.createWeapon("object/weapon/ranged/carbine/carbine_laser.iff", pInv, 0.8f);
		obj_id dxr6Carbine = weapons.createWeapon("object/weapon/ranged/carbine/carbine_dxr6.iff", pInv, 0.8f);
		
		createObject("object/tangible/mission/mission_bounty_droid_probot.iff", pInv, "");
		createObject("object/tangible/mission/mission_bounty_droid_probot.iff", pInv, "");
		createObject("object/tangible/mission/mission_bounty_droid_probot.iff", pInv, "");
		createObject("object/tangible/mission/mission_bounty_droid_probot.iff", pInv, "");
		createObject("object/tangible/mission/mission_bounty_droid_probot.iff", pInv, "");
		createObject("object/tangible/mission/mission_bounty_droid_probot.iff", pInv, "");
		createObject("object/tangible/mission/mission_bounty_droid_probot.iff", pInv, "");
		createObject("object/tangible/mission/mission_bounty_droid_probot.iff", pInv, "");
		createObject("object/tangible/mission/mission_bounty_droid_probot.iff", pInv, "");
		createObject("object/tangible/mission/mission_bounty_droid_probot.iff", pInv, "");
		createObject("object/tangible/mission/mission_bounty_droid_seeker.iff", pInv, "");
		createObject("object/tangible/mission/mission_bounty_droid_seeker.iff", pInv, "");
		createObject("object/tangible/mission/mission_bounty_droid_seeker.iff", pInv, "");
		createObject("object/tangible/mission/mission_bounty_droid_seeker.iff", pInv, "");
		createObject("object/tangible/mission/mission_bounty_droid_seeker.iff", pInv, "");
		createObject("object/tangible/mission/mission_bounty_droid_seeker.iff", pInv, "");
		createObject("object/tangible/mission/mission_bounty_droid_seeker.iff", pInv, "");
		createObject("object/tangible/mission/mission_bounty_droid_seeker.iff", pInv, "");
		createObject("object/tangible/mission/mission_bounty_droid_seeker.iff", pInv, "");
		createObject("object/tangible/mission/mission_bounty_droid_seeker.iff", pInv, "");
		createObject("object/tangible/mission/mission_bounty_droid_probot.iff", pInv, "");
		createObject("object/tangible/mission/mission_bounty_droid_probot.iff", pInv, "");
		createObject("object/tangible/mission/mission_bounty_droid_probot.iff", pInv, "");
		createObject("object/tangible/mission/mission_bounty_droid_probot.iff", pInv, "");
		createObject("object/tangible/mission/mission_bounty_droid_probot.iff", pInv, "");
		createObject("object/tangible/mission/mission_bounty_droid_probot.iff", pInv, "");
		createObject("object/tangible/mission/mission_bounty_droid_probot.iff", pInv, "");
		createObject("object/tangible/mission/mission_bounty_droid_probot.iff", pInv, "");
		createObject("object/tangible/mission/mission_bounty_droid_probot.iff", pInv, "");
		createObject("object/tangible/mission/mission_bounty_droid_probot.iff", pInv, "");
		
		obj_id hotpants = createObject("object/tangible/wearables/pants/pants_s26.iff", pInv, "");
		{
			if (isIdValid(hotpants))
			{
				
				int color = rand(151, 255);
				custom_var myVar = getCustomVarByName(hotpants, "private/index_color_2");
				if (myVar.isPalColor())
				{
					palcolor_custom_var pcVar = (palcolor_custom_var)myVar;
					pcVar.setValue(color);
				}
			}
		}
		
		gm.setBalance(self, money.MT_BANK, "500000");
		setObjVar(self, "jediTest.bounty_hunter_kit", 1);
		return;
	}
	
	
	public int handleTurnstilePayFail(obj_id self, dictionary params) throws InterruptedException
	{
		utils.removeScriptVar( self, "turnstile.queryTurnstile");
		sendSystemMessage( self, new string_id(STF_FILE, "not_enough_money"));
		return SCRIPT_CONTINUE;
	}
	
	
	public int handleGmGrantSkillOptions(obj_id self, dictionary params) throws InterruptedException
	{
		obj_id target = utils.getObjIdScriptVar(self, "gmGrantSkill.target");
		
		if (!isIdValid(target))
		{
			sendSystemMessageTestingOnly(self, "Target is no longer valid.");
			utils.removeScriptVarTree(self, "gmGrantSkill");
			return SCRIPT_CONTINUE;
		}
		
		int btn = sui.getIntButtonPressed(params);
		int idx = sui.getListboxSelectedRow(params);
		
		if (btn == sui.BP_CANCEL)
		{
			utils.removeScriptVarTree(self, "gmGrantSkill");
			return SCRIPT_CONTINUE;
		}
		
		if (idx == -1)
		{
			utils.removeScriptVarTree(self, "gmGrantSkill");
			return SCRIPT_CONTINUE;
		}
		
		switch(idx)
		{
			case 0:
			gmGrantSkillRoadmapChoice(self, target);
			break;
			case 1:
			String skillName = getWorkingSkill(target);
			
			dictionary xpReqs = getSkillPrerequisiteExperience(skillName);
			if (xpReqs == null || xpReqs.isEmpty())
			{
				sendSystemMessageTestingOnly(self, "Player does not have a valid working skill");
				return SCRIPT_CONTINUE;
			}
			
			java.util.Enumeration e = xpReqs.keys();
			String xpType = (String)(e.nextElement());
			int xpCost = xpReqs.getInt(xpType);
			int curXP = getExperiencePoints(target, xpType);
			if (curXP < xpCost)
			{
				grantExperiencePoints(target, xpType, xpCost - curXP);
			}
			skill_template.earnWorkingSkill(target);
			redisplayGrantSkillSui(self, target);
			break;
			case 2:
			String template = getSkillTemplate(target);
			gmGrantSkillRoadmapProgression(self, target, template);
			break;
			
			default:
			utils.removeScriptVarTree(self, "gmGrantSkill");
			return SCRIPT_CONTINUE;
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public void redisplayGrantSkillSui(obj_id self, obj_id target) throws InterruptedException
	{
		utils.setScriptVar(self, "gmGrantSkill.target", target);
		
		String prompt = "You are attempting to modify the skills for \\#pcontrast3 "+ getName(target) + "\\#..\n\n";
		prompt += "Please choose an option:";
		
		int pid = sui.listbox(self, self, prompt, sui.OK_CANCEL, "GM GRANT SKILL", gm.ROADMAP_SKILL_OPTIONS, "handleGmGrantSkillOptions", true, false);
	}
	
	
	public void gmGrantSkillRoadmapChoice(obj_id self, obj_id target) throws InterruptedException
	{
		String[] roadmapList = gm.getRoadmapList();
		
		String prompt = "You are attempting to modify the skills for \\#pcontrast3 "+ getName(target) + "\\#..\n\n";
		prompt += "Please select a profession roadmap:";
		
		int pid = sui.listbox(self, self, prompt, sui.OK_CANCEL, "GM GRANT SKILL", gm.convertRoadmapNames(roadmapList), "handleGmGrantSkillRoadmapSelect", true, false);
		
		utils.setBatchScriptVar(self, "gmGrantSkill.roadmap_list", roadmapList);
		utils.setScriptVar(self, "gmGrantSkill.target", target);
	}
	
	
	public int handleGmGrantSkillRoadmapSelect(obj_id self, dictionary params) throws InterruptedException
	{
		obj_id target = utils.getObjIdScriptVar(self, "gmGrantSkill.target");
		
		if (!isIdValid(target))
		{
			sendSystemMessageTestingOnly(self, "Target is no longer valid.");
			utils.removeScriptVarTree(self, "gmGrantSkill");
			return SCRIPT_CONTINUE;
		}
		
		int idx = sui.getListboxSelectedRow(params);
		int btn = sui.getIntButtonPressed(params);
		
		if (btn == sui.BP_CANCEL)
		{
			utils.removeScriptVarTree(self, "gmGrantSkill");
			redisplayGrantSkillSui(self, target);
			return SCRIPT_CONTINUE;
		}
		
		if (idx == -1)
		{
			utils.removeScriptVarTree(self, "gmGrantSkill");
			gmGrantSkillRoadmapChoice(self, target);
			return SCRIPT_CONTINUE;
		}
		
		if (exists(target))
		{
			String[] roadmapList = utils.getStringBatchScriptVar(self, "gmGrantSkill.roadmap_list");
			
			gmGrantSkillRoadmapProgression(self, target, roadmapList[idx]);
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public void gmGrantSkillRoadmapProgression(obj_id self, obj_id target, String template) throws InterruptedException
	{
		String templateSkills = dataTableGetString(skill_template.TEMPLATE_TABLE, template, "template");
		String[] skillList = split(templateSkills, ',');
		
		if (skillList == null || skillList.length == 0)
		{
			sendSystemMessageTestingOnly(self, "No skills found for this profession");
			utils.removeScriptVarTree(self, "gmGrantSkill");
			return;
		}
		else
		{
			sendSystemMessageTestingOnly(self, "Revoking all skills and experience for "+ getName(target));
			respec.revokeAllSkillsAndExperience(target);
			skill.recalcPlayerPools(target, true);
			
			if (!template.equals(getSkillTemplate(target)))
			{
				setSkillTemplate(target, template);
			}
			
			setWorkingSkill(target, skillList[0]);
			
			utils.setScriptVar(self, "gmGrantSkill.skill_template", template);
			utils.setBatchScriptVar(self, "gmGrantSkill.roadmap_skills", skillList);
		}
		
		String prompt = "You are attempting to modify the skills for \\#pcontrast3 "+ getName(target) + "\\#..\n\n";
		prompt += "Please enter the level for this player:";
		
		int pid = sui.inputbox(self, self, prompt, "handleGmGrantSkillLevelInput");
		
	}
	
	
	public int handleGmGrantSkillSkillSelect(obj_id self, dictionary params) throws InterruptedException
	{
		obj_id target = utils.getObjIdScriptVar(self, "gmGrantSkill.target");
		
		if (!isIdValid(target))
		{
			sendSystemMessageTestingOnly(self, "Target is no longer valid.");
			utils.removeScriptVarTree(self, "gmGrantSkill");
			return SCRIPT_CONTINUE;
		}
		
		int idx = sui.getListboxSelectedRow(params);
		int btn = sui.getIntButtonPressed(params);
		
		if (btn == sui.BP_CANCEL)
		{
			utils.removeScriptVarTree(self, "gmGrantSkill");
			return SCRIPT_CONTINUE;
		}
		
		String skillTemplate = utils.getStringScriptVar(self, "gmGrantSkill.skill_template");
		if (skillTemplate == null || skillTemplate.equals(""))
		{
			utils.removeScriptVarTree(self, "gmGrantSkill");
			return SCRIPT_CONTINUE;
		}
		
		if (idx == -1)
		{
			sendSystemMessageTestingOnly(self, "Please make a selection.");
			gmGrantSkillRoadmapProgression(self, target, skillTemplate);
			return SCRIPT_CONTINUE;
		}
		
		if (exists(target))
		{
			String[] roadmapSkills = utils.getStringBatchScriptVar(self, "gmGrantSkill.roadmap_skills");
			
			setWorkingSkill(target, roadmapSkills[0]);
			
			for (int i = 0; i < idx; i++)
			{
				testAbortScript();
				grantSkill(target, roadmapSkills[i]);
				skill_template.grantRoadmapItem(target);
				
				setWorkingSkill(target, skill_template.getNextWorkingSkill(target));
			}
			
			sendSystemMessageTestingOnly(self, "Skill Granting for "+ getName(target) + " is complete");
		}
		
		utils.removeScriptVarTree(self, "gmGrantSkill");
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int handleGmGrantSkillLevelInput(obj_id self, dictionary params) throws InterruptedException
	{
		obj_id target = utils.getObjIdScriptVar(self, "gmGrantSkill.target");
		
		if (!isIdValid(target))
		{
			sendSystemMessageTestingOnly(self, "Target is no longer valid.");
			utils.removeScriptVarTree(self, "gmGrantSkill");
			return SCRIPT_CONTINUE;
		}
		
		String text = sui.getInputBoxText(params);
		
		int level = utils.stringToInt(text);
		
		if (level < 1 || level > 90)
		{
			sendSystemMessageTestingOnly(self, "Invalid level entered!");
			return SCRIPT_CONTINUE;
		}
		else
		{
			respec.autoLevelPlayer(target, level, true);
		}
		
		sendSystemMessageTestingOnly(self, "Skill Granting for "+ getName(target) + " is complete");
		
		utils.removeScriptVarTree(self, "gmGrantSkill");
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int cmdAutoDeclineDuel(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (hasObjVar(self, VAR_AUTODECLINE_DUEL))
		{
			removeObjVar(self, VAR_AUTODECLINE_DUEL);
			sendSystemMessage(self, new string_id("spam", "autodecline_duel_off"));
		}
		else
		{
			setObjVar(self, VAR_AUTODECLINE_DUEL, 1);
			sendSystemMessage(self, new string_id("spam", "autodecline_duel_on"));
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int handleSmugglerGetLocationInfo(obj_id self, dictionary params) throws InterruptedException
	{
		if (params == null || params.isEmpty())
		{
			return SCRIPT_CONTINUE;
		}
		
		obj_id smuggler = params.getObjId("smuggler");
		
		if (!isIdValid(smuggler))
		{
			return SCRIPT_CONTINUE;
		}
		
		location loc = getLocation(self);
		
		params = new dictionary();
		params.put("location", loc);
		
		messageTo(smuggler, "handleSmugglerDistanceCheck", params, 1.0f, false);
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int handleSmugglerDistanceCheck(obj_id self, dictionary params) throws InterruptedException
	{
		if (params == null || params.isEmpty())
		{
			return SCRIPT_CONTINUE;
		}
		
		location targetLoc = params.getLocation("location");
		location myLoc = getLocation(self);
		
		String myLocArea = myLoc.area;
		String targetLocArea = targetLoc.area;
		
		if (targetLoc != null && myLocArea.equals(targetLocArea))
		{
			int distance = (int)getDistance(myLoc, targetLoc);
			
			if (distance > 1 && distance < 1000)
			{
				prose_package pp = new prose_package ();
				pp.stringId = new string_id ("bounty_hunter", "sm_bounty_hunter_distance");
				prose.setDI(pp, distance);
				
				sendSystemMessageProse(self, pp);
			}
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int channelHeal(obj_id self, dictionary params) throws InterruptedException
	{
		
		if (params == null || params.isEmpty())
		{
			return SCRIPT_CONTINUE;
		}
		
		obj_id user = params.getObjId("user");
		int tick = params.getInt("tick");
		obj_id item = params.getObjId("item");
		int healPerTick = params.getInt("healPerTick");
		location healLoc = params.getLocation("healLoc");
		int attrib = params.getInt("attrib");
		int pid = utils.getIntScriptVar(self, "channelHeal.suiPid");
		
		location currentLoc = getLocation(self);
		
		prose_package pp = new prose_package();
		
		if (user != self)
		{
			
			sendSystemMessage(self, SID_CHANNEL_HEAL_WRONG_USER);
			buff.removeBuff(self, "channel_healing");
			return SCRIPT_CONTINUE;
		}
		
		if (!healLoc.equals(currentLoc))
		{
			
			sendSystemMessage(self, SID_CHANNEL_HEAL_MOVED);
			buff.removeBuff(self, "channel_healing");
			return SCRIPT_CONTINUE;
		}
		
		if (!buff.hasBuff(self, "channel_healing") && tick != 0)
		{
			
			sendSystemMessage(self, SID_CHANNEL_HEAL_IN_COMBAT);
			return SCRIPT_CONTINUE;
		}
		
		if (luck.isLucky(self, 0.005f))
		{
			float bonus = healPerTick * 0.2f;
			if (bonus < 1)
			{
				bonus = 1;
			}
			
			healPerTick += bonus;
		}
		
		if (tick == 0)
		{
			buff.applyBuff(self, "channel_healing");
		}
		
		int before = getAttrib(self, attrib);
		
		healing.healDamage(self, self, attrib, healPerTick);
		
		int delta = getAttrib(self, attrib) - before;
		
		pp = prose.setStringId(pp, new string_id("healing", "heal_fly"));
		pp = prose.setDI(pp, delta);
		pp = prose.setTO(pp, healing.ATTRIBUTES[attrib]);
		showFlyTextPrivateProseWithFlags(self, self, pp, 2.0f, colors.SEAGREEN, healing.FLY_TEXT_FLAG_IS_HEAL);
		
		pp = prose.setStringId(pp, healing.SID_ITEM_PLAYER_HEAL);
		pp = prose.setDI(pp, delta);
		pp = prose.setTT(pp, self);
		healing.sendMedicalSpam(self, pp, healing.COMBAT_RESULT_MEDICAL);
		healing.doHealingAnimationAndEffect(self, self);
		
		tick += 1;
		
		params.put("tick", tick);
		
		if (tick < healing.ITEM_CHANNEL_HEAL_TICKS)
		{
			messageTo( self, "channelHeal", params, 2, false);
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int residentLinkFalse(obj_id self, dictionary params) throws InterruptedException
	{
		setHouseId(self, obj_id.NULL_ID);
		return SCRIPT_CONTINUE;
	}
	
	
	public boolean blog(String msg) throws InterruptedException
	{
		if (LOGGING_ON)
		{
			LOG(LOGGING_CATEGORY, msg);
		}
		return true;
	}
	
	
	public int handleTargetDummyCombatReport(obj_id self, dictionary params) throws InterruptedException
	{
		if ((params == null) || (params.isEmpty()))
		{
			return SCRIPT_CONTINUE;
		}
		
		int button = sui.getIntButtonPressed( params );
		String revert = params.getString(sui.MSGBOX_BTN_REVERT+".RevertWasPressed");
		if (button == sui.BP_CANCEL)
		{
			return SCRIPT_CONTINUE;
		}
		else
		{
			if (revert != null && !revert.equals(""))
			{
				
				if (utils.hasScriptVarTree(self, target_dummy.BASE_TARGET_DUMMY_VAR))
				{
					utils.removeScriptVarTree(self, target_dummy.BASE_TARGET_DUMMY_VAR);
					sendSystemMessage(self, new string_id("target_dummy", "combat_data_all_clearing"));
					sendSystemMessage(self, new string_id("target_dummy", "combat_data_all_cleared"));
				}
				else
				{
					
					sendSystemMessage(self, new string_id("target_dummy", "placement_no_combat_data"));
				}
			}
			else
			{
				
				target_dummy.reportCombatData(self, self);
			}
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int reimburseTcgItem(obj_id self, dictionary params) throws InterruptedException
	{
		if ((params == null) || (params.isEmpty()))
		{
			CustomerServiceLog("tcg", "TCG Reimbursement failed because of lack of parameters for reimbursement function for: "+self+".");
			return SCRIPT_CONTINUE;
		}
		
		if (!params.containsKey("reimburseItem"))
		{
			CustomerServiceLog("tcg", "TCG Reimbursement item parameter not found. Reimbursement function failed for: "+self+".");
			return SCRIPT_CONTINUE;
		}
		
		String reimbursementItem = params.getString("reimburseItem");
		if (reimbursementItem == null || reimbursementItem.equals(""))
		{
			CustomerServiceLog("tcg", "TCG Reimbursement item parameter was invalid. Reimbursement function failed for: "+self+".");
			return SCRIPT_CONTINUE;
		}
		
		obj_id pInv = utils.getInventoryContainer(self);
		if (!isValidId(pInv) || !exists(pInv))
		{
			CustomerServiceLog("tcg", "TCG Reimbursement function failed for: "+self+". Player inventory Id failed.");
			return SCRIPT_CONTINUE;
		}
		
		CustomerServiceLog("tcg", "TCG Reimbursement item: "+reimbursementItem+" being given to player: "+self+".");
		obj_id newItem = static_item.createNewItemFunction(reimbursementItem, pInv);
		if (!isValidId(newItem))
		{
			CustomerServiceLog("tcg", "TCG Reimbursement item: "+reimbursementItem+" failed. New object could not be created for: "+self+".");
			return SCRIPT_CONTINUE;
		}
		
		CustomerServiceLog("tcg", "TCG Reimbursement item: "+reimbursementItem+" was created: "+newItem+" and given to player: "+self+".");
		return SCRIPT_CONTINUE;
	}
	
	
	public boolean applyCostume(obj_id self, String costumeCommand) throws InterruptedException
	{
		String ibuff = stealth.getInvisBuff(self);
		if (ibuff != null && ibuff.length() > 0)
		{
			sendSystemMessage(self, new string_id("spam", "costume_not_while_invis"));
			return false;
		}
		
		if (getState(self, STATE_GLOWING_JEDI) != 0)
		{
			sendSystemMessage(self, new string_id("spam", "costume_not_while_blue_glowie"));
			return false;
		}
		
		String datatable = "datatables/item/costume/costumes.iff";
		dictionary costumeData = dataTableGetRow(datatable, costumeCommand);
		if (costumeData != null)
		{
			String costumeBuff = costumeData.getString("costume_buff");
			String costumeComponentName = costumeData.getString("costume_component");
			
			if (costumeBuff != null && costumeBuff.length() > 0)
			{
				
				if (costumeComponentName != null && costumeComponentName.length() > 0 && !costumeComponentName.equals("none"))
				{
					obj_id costumeComponentId = utils.getStaticItemInInventory(self, costumeComponentName);
					if (!isIdValid(costumeComponentId) || getCount(costumeComponentId) <= 0)
					{
						string_id message = new string_id("spam", "costume_component_missing");
						prose_package pp = prose.getPackage(message, self, self);
						prose.setTO(pp, getString(new string_id("static_item_n", costumeComponentName)));
						sendSystemMessageProse(self, pp);
						return false;
					}
					else
					{
						decrementCount(costumeComponentId);
					}
				}
				
				buff.applyBuff(self, costumeBuff);
				return true;
			}
		}
		return false;
	}
	
	
	public int costume_loveday_ewok_cupid(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!applyCostume(self, "costume_loveday_ewok_cupid"))
		{
			return SCRIPT_OVERRIDE;
		}
		return SCRIPT_CONTINUE;
	}
	
	public int costume_jawa(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!applyCostume(self, "costume_jawa"))
		{
			return SCRIPT_OVERRIDE;
		}
		return SCRIPT_CONTINUE;
	}
	
	public int costume_gungan(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!applyCostume(self, "costume_gungan"))
		{
			return SCRIPT_OVERRIDE;
		}
		return SCRIPT_CONTINUE;
	}
	
	public int costume_ewok(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!applyCostume(self, "costume_ewok"))
		{
			return SCRIPT_OVERRIDE;
		}
		return SCRIPT_CONTINUE;
	}
	
	public int costume_gamorrean_guard(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!applyCostume(self, "costume_gamorrean_guard"))
		{
			return SCRIPT_OVERRIDE;
		}
		return SCRIPT_CONTINUE;
	}
	
	
	public int costume_battle_droid(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!applyCostume(self, "costume_battle_droid"))
		{
			return SCRIPT_OVERRIDE;
		}
		return SCRIPT_CONTINUE;
	}
	
	public int costume_droideka(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!applyCostume(self, "costume_droideka"))
		{
			return SCRIPT_OVERRIDE;
		}
		return SCRIPT_CONTINUE;
	}
	
	public int costume_monkey_lizard(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!applyCostume(self, "costume_monkey_lizard"))
		{
			return SCRIPT_OVERRIDE;
		}
		return SCRIPT_CONTINUE;
	}
	
	public int costume_toydarian(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!applyCostume(self, "costume_toydarian"))
		{
			return SCRIPT_OVERRIDE;
		}
		return SCRIPT_CONTINUE;
	}
	
	public int costume_muftak(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!applyCostume(self, "costume_muftak"))
		{
			return SCRIPT_OVERRIDE;
		}
		return SCRIPT_CONTINUE;
	}
	
	public int costume_dantari_m(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!applyCostume(self, "costume_dantari_m"))
		{
			return SCRIPT_OVERRIDE;
		}
		return SCRIPT_CONTINUE;
	}
	
	public int costume_4lom(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!applyCostume(self, "costume_4lom"))
		{
			return SCRIPT_OVERRIDE;
		}
		return SCRIPT_CONTINUE;
	}
	
	public int costume_dantari_f(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!applyCostume(self, "costume_dantari_f"))
		{
			return SCRIPT_OVERRIDE;
		}
		return SCRIPT_CONTINUE;
	}
	
	public int costume_royal_guard(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!applyCostume(self, "costume_royal_guard"))
		{
			return SCRIPT_OVERRIDE;
		}
		return SCRIPT_CONTINUE;
	}
	
	public int costume_droid_8t88(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!applyCostume(self, "costume_droid_8t88"))
		{
			return SCRIPT_OVERRIDE;
		}
		return SCRIPT_CONTINUE;
	}
	
	public int costume_darktrooper(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!applyCostume(self, "costume_darktrooper"))
		{
			return SCRIPT_OVERRIDE;
		}
		return SCRIPT_CONTINUE;
	}
	
	public int costume_mustafarian_01(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!applyCostume(self, "costume_mustafarian_01"))
		{
			return SCRIPT_OVERRIDE;
		}
		return SCRIPT_CONTINUE;
	}
	
	public int costume_chevin(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!applyCostume(self, "costume_chevin"))
		{
			return SCRIPT_OVERRIDE;
		}
		return SCRIPT_CONTINUE;
	}
	
	public int costume_ig106(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!applyCostume(self, "costume_ig106"))
		{
			return SCRIPT_OVERRIDE;
		}
		return SCRIPT_CONTINUE;
	}
	
	public int costume_super_battle_droid(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!applyCostume(self, "costume_super_battle_droid"))
		{
			return SCRIPT_OVERRIDE;
		}
		return SCRIPT_CONTINUE;
	}
	
	public int costume_spider_droid(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!applyCostume(self, "costume_spider_droid"))
		{
			return SCRIPT_OVERRIDE;
		}
		return SCRIPT_CONTINUE;
	}
	
	public int costume_undead_wookie(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!applyCostume(self, "costume_undead_wookie"))
		{
			return SCRIPT_OVERRIDE;
		}
		return SCRIPT_CONTINUE;
	}
	
	public int costume_burning_ewok(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!applyCostume(self, "costume_burning_ewok"))
		{
			return SCRIPT_OVERRIDE;
		}
		return SCRIPT_CONTINUE;
	}
	
	public int costume_chiss_01(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!applyCostume(self, "costume_chiss_01"))
		{
			return SCRIPT_OVERRIDE;
		}
		return SCRIPT_CONTINUE;
	}
	
	public int costume_exar_kun_cultist(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!applyCostume(self, "costume_exar_kun_cultist"))
		{
			return SCRIPT_OVERRIDE;
		}
		return SCRIPT_CONTINUE;
	}
	
	public int costume_lifeday_monkey_lizard(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!applyCostume(self, "costume_lifeday_monkey_lizard"))
		{
			return SCRIPT_OVERRIDE;
		}
		return SCRIPT_CONTINUE;
	}
	
	
	public int cmdShowHousePackUpData(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		String housePackUpData = sui.colorGreen()+"";
		String timeMessage = "";
		String displayTime = "";
		int dailyCount = 0;
		
		int meritPts = player_structure.getPlayerPackUpMeritPoints(self);
		if (meritPts < 0)
		{
			meritPts = 0;
		}
		housePackUpData +="Total Merit Points: "+sui.colorWhite() + meritPts + sui.colorGreen();
		housePackUpData += sui.newLine();
		
		int totalPacked = 0;
		int[] housePackingTally = getIntArrayObjVar(self, player_structure.HOUSE_PACKUP_ARRAY_OBJVAR);
		if (housePackingTally != null && housePackingTally.length == player_structure.ARRAY_LENGTH_FOR_HOUSE_PACKUP)
		{
			totalPacked = housePackingTally[1];
			if (totalPacked < 0)
			{
				totalPacked = 0;
			}
			
		}
		housePackUpData += "Total Structures Ever Packed: "+sui.colorWhite() + totalPacked + sui.colorGreen();
		housePackUpData += sui.newLine();
		
		if (hasObjVar(self, player_structure.HOUSE_PACKUP_DAILY_TIMER_OBJVAR))
		{
			int resetTime = getIntObjVar(self, player_structure.HOUSE_PACKUP_DAILY_TIMER_OBJVAR);
			if (getGameTime() < resetTime)
			{
				dailyCount = getIntObjVar(self, player_structure.HOUSE_PACKUP_DAILY_TALLY_OBJVAR);
				if (dailyCount < 0)
				{
					dailyCount = 0;
				}
				
				if (dailyCount > 0)
				{
					housePackUpData += "Total Structures Packed in Last 24 hours: "+sui.colorWhite() + dailyCount + sui.colorGreen();
					housePackUpData += sui.newLine();
				}
			}
		}
		
		if (!player_structure.canPackMoreToday(self) && hasObjVar(self, player_structure.HOUSE_PACKUP_DAILY_TIMER_OBJVAR))
		{
			int timeLeft = getIntObjVar(self, player_structure.HOUSE_PACKUP_DAILY_TIMER_OBJVAR);
			timeLeft -= getGameTime();
			displayTime = utils.assembleTimeRemainToUse(timeLeft, false);
			if (displayTime != null && displayTime.length() > 0)
			{
				housePackUpData += "Time Before Next Structure Can Be Packed: "+sui.colorWhite() + displayTime + sui.colorGreen();
				housePackUpData += sui.newLine();
			}
		}
		else if (player_structure.canPackMoreToday(self) && utils.hasScriptVar(self, player_structure.SCRIPTVAR_HOUSE_PACKUP_LOCKOUT_TIME))
		{
			int timeNeeded = utils.getIntScriptVar(self, player_structure.SCRIPTVAR_HOUSE_PACKUP_LOCKOUT_TIME) + (int)player_structure.HOUSE_PACKUP_LOCKOUT_TIMER;
			int timeNow = getGameTime();
			int timeDiff = timeNeeded - timeNow;
			timeMessage = utils.formatTimeVerbose(timeDiff);
			if (timeMessage != null && timeMessage.length() > 0)
			{
				housePackUpData += "Time Before Next Structure Can Be Packed: "+sui.colorWhite() + timeMessage + sui.colorGreen();
				housePackUpData += sui.newLine();
			}
		}
		
		housePackUpData += sui.colorWhite();
		sendConsoleMessage(self, housePackUpData);
		return SCRIPT_CONTINUE;
	}
	
	
	public int cmdRename(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		
		if (utils.hasScriptVar(self, SCRIPTVAR_RENAME_CHARACTER_SUI_ID))
		{
			int savedPageId = utils.getIntScriptVar(self, SCRIPTVAR_RENAME_CHARACTER_SUI_ID);
			utils.removeScriptVar(self, SCRIPTVAR_RENAME_CHARACTER_SUI_ID);
			forceCloseSUIPage(savedPageId);
		}
		
		if (utils.hasScriptVar(self, SCRIPTVAR_RENAME_CHARACTER_NEW_NAME))
		{
			String newName = utils.getStringScriptVar(self, SCRIPTVAR_RENAME_CHARACTER_NEW_NAME);
			utils.removeScriptVar(self, SCRIPTVAR_RENAME_CHARACTER_NEW_NAME);
			
			if ((newName != null) && (newName.length() > 0))
			{
				renameCharacterReleaseNameReservation(self);
			}
		}
		
		utils.removeScriptVar(self, SCRIPTVAR_RENAME_CHARACTER_UNVERIFIED_NEW_NAME);
		
		final int daysOld = getCurrentBirthDate() - getPlayerBirthDate(self);
		if (daysOld < 90)
		{
			if (isGod(self))
			{
				sendSystemMessage(self, "GOD MODE bypass for 90 days character age requirement for character rename.", "");
			}
			else
			{
				sendSystemMessage(self, "This character is only "+ daysOld + " days old. The character must be at least 90 days old before you can rename the character.", "");
				return SCRIPT_CONTINUE;
			}
		}
		
		if (hasObjVar(self, "renameCharacterRequest.requestTime"))
		{
			final int previousRenameTime = getIntObjVar(self, "renameCharacterRequest.requestTime");
			final int nextRenameTime = previousRenameTime + (90 * 24 * 60 * 60);
			final int timeNow = getCalendarTime();
			
			if (nextRenameTime > timeNow)
			{
				final String previousRenameName = getStringObjVar(self, "renameCharacterRequest.requestNewName");
				final String currentName = getName(self);
				
				if ((currentName != null) && (previousRenameName != null) && !previousRenameName.equals(currentName))
				{
					
					final int renameRequestTimeOut = previousRenameTime + (24 * 60 * 60);
					
					if (renameRequestTimeOut > timeNow)
					{
						int[] convertedGameTime = player_structure.convertSecondsTime(renameRequestTimeOut - timeNow);
						if ((convertedGameTime != null) && (convertedGameTime.length == 4))
						{
							sendSystemMessage(self, "There is already an outstanding rename request for this character. If the request is not completed in "+ convertedGameTime[1] + "h:"+ convertedGameTime[2] + "m:"+ convertedGameTime[3] + "s, you can try to rename this character again.", "");
						}
						else
						{
							sendSystemMessage(self, "There is already an outstanding rename request for this character.", "");
						}
						
						return SCRIPT_CONTINUE;
					}
				}
				else
				{
					
					if (isGod(self))
					{
						sendSystemMessage(self, "GOD MODE bypass for 90 days wait between character rename.", "");
					}
					else
					{
						int[] convertedGameTime = player_structure.convertSecondsTime(nextRenameTime - timeNow);
						if ((convertedGameTime != null) && (convertedGameTime.length == 4))
						{
							sendSystemMessage(self, "You must wait "+ convertedGameTime[0] + "d:"+ convertedGameTime[1] + "h:"+ convertedGameTime[2] + "m:"+ convertedGameTime[3] + "s before you can rename this character again.", "");
						}
						else
						{
							sendSystemMessage(self, "There is a 90 days wait after the character has been renamed before you can rename the character again.", "");
						}
						
						return SCRIPT_CONTINUE;
					}
				}
			}
		}
		
		renameCharacterDisplayNameInput(self, getName(self), "Enter a new name for the character.");
		return SCRIPT_CONTINUE;
	}
	
	
	public int trigger_city_house_packup(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		obj_id player = self;
		
		if (!isPlayer(player) || !isIdValid(player) || !exists(player))
		{
			return SCRIPT_CONTINUE;
		}
		
		location here = getLocation(player);
		obj_id[] potentialSigns = getObjectsInRange(here, 10.0f);
		
		for (int i = 0; i < potentialSigns.length; i++)
		{
			testAbortScript();
			if (!hasScript(potentialSigns[i], "systems.sign.sign"))
			{
				continue;
			}
			else
			{
				
				if (!utils.hasScriptVar(potentialSigns[i], "player_structure.parent"))
				{
					sendSystemMessage(player, new string_id(STF_FILE, "unknown_city_packup_error"));
					CustomerServiceLog("playerStructure", "Player("+ player +") attempted to use the Remote City Packup Command but the House Sign("+ potentialSigns[i] +") has invalid data.");
					LOG("sissynoid", "Invalid Sign - No Parent ObjVar - fail");
					return SCRIPT_CONTINUE;
				}
				
				obj_id structure = utils.getObjIdScriptVar(potentialSigns[i], "player_structure.parent");
				if (!isIdValid(structure))
				{
					LOG("sissynoid", "Invalid Structure ID: "+ structure);
					CustomerServiceLog("playerStructure", "Player("+ player +") attempted to use the Remote City Packup Command but the House Obj ID has invalid.");
					return SCRIPT_CONTINUE;
				}
				
				if (!player_structure.isCityAbandoned(structure))
				{
					sendSystemMessage(player, new string_id(STF_FILE, "structure_not_city_abandoned"));
					CustomerServiceLog("playerStructure", "Player("+ player +") attempted to use the Remote City Packup Command but the House("+ structure +") was not City Abandoned.");
					LOG("sissynoid", "This structure is not City Abandoned - fail");
					return SCRIPT_CONTINUE;
				}
				else
				{
					LOG("sissynoid", "I found a sign: "+ potentialSigns[i]);
					dictionary dict = new dictionary();
					dict.put("player", player);
					dict.put("sign", potentialSigns[i]);
					messageTo(potentialSigns[i], "handleRemoteCommandCityHousePackup", dict, 1.0f, false);
				}
			}
		}
		return SCRIPT_CONTINUE;
	}
	
	
	public int handleRenameCharacterNameInput(obj_id self, dictionary params) throws InterruptedException
	{
		if (!utils.hasScriptVar(self, SCRIPTVAR_RENAME_CHARACTER_SUI_ID))
		{
			return SCRIPT_CONTINUE;
		}
		
		if (params.getInt("pageId") != utils.getIntScriptVar(self, SCRIPTVAR_RENAME_CHARACTER_SUI_ID))
		{
			return SCRIPT_CONTINUE;
		}
		
		utils.removeScriptVar(self, SCRIPTVAR_RENAME_CHARACTER_SUI_ID);
		
		int bp = sui.getIntButtonPressed(params);
		if (bp == sui.BP_OK)
		{
			String newName = sui.getInputBoxText(params);
			if (newName != null)
			{
				java.util.StringTokenizer st = new java.util.StringTokenizer(newName);
				
				newName = null;
				while (st.hasMoreTokens())
				{
					testAbortScript();
					if (newName == null)
					{
						newName = st.nextToken();
					}
					else
					{
						newName += " ";
						newName += st.nextToken();
					}
				}
			}
			
			String currentName = getName(self);
			if ((newName != null) && (newName.length() > 0) && (currentName != null) && (currentName.length() > 0) && !newName.equals(currentName))
			{
				utils.setScriptVar(self, SCRIPTVAR_RENAME_CHARACTER_UNVERIFIED_NEW_NAME, newName);
				validateRenameCharacter(self, newName);
			}
			else
			{
				if (currentName == null)
				{
					currentName = "";
				}
				
				renameCharacterDisplayNameInput(self, currentName, "Please enter a new name for the character.");
			}
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int renameCharacterNameValidationFail(obj_id self, dictionary params) throws InterruptedException
	{
		if (utils.hasScriptVar(self, SCRIPTVAR_RENAME_CHARACTER_SUI_ID))
		{
			return SCRIPT_CONTINUE;
		}
		
		if (!utils.hasScriptVar(self, SCRIPTVAR_RENAME_CHARACTER_UNVERIFIED_NEW_NAME))
		{
			return SCRIPT_CONTINUE;
		}
		
		String unverifiedNewName = utils.getStringScriptVar(self, SCRIPTVAR_RENAME_CHARACTER_UNVERIFIED_NEW_NAME);
		utils.removeScriptVar(self, SCRIPTVAR_RENAME_CHARACTER_UNVERIFIED_NEW_NAME);
		
		if ((unverifiedNewName != null) && (unverifiedNewName.length() > 0))
		{
			renameCharacterDisplayNameInput(self, unverifiedNewName, params.getString("reason"));
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int renameCharacterNameValidationSuccess(obj_id self, dictionary params) throws InterruptedException
	{
		if (utils.hasScriptVar(self, SCRIPTVAR_RENAME_CHARACTER_SUI_ID))
		{
			return SCRIPT_CONTINUE;
		}
		
		if (!utils.hasScriptVar(self, SCRIPTVAR_RENAME_CHARACTER_UNVERIFIED_NEW_NAME))
		{
			return SCRIPT_CONTINUE;
		}
		
		String newName = utils.getStringScriptVar(self, SCRIPTVAR_RENAME_CHARACTER_UNVERIFIED_NEW_NAME);
		utils.removeScriptVar(self, SCRIPTVAR_RENAME_CHARACTER_UNVERIFIED_NEW_NAME);
		
		if ((newName != null) && (newName.length() > 0))
		{
			String announcement = "Please confirm that you want to rename this character to \"";
			announcement += newName;
			announcement += "\".\n\n";
			
			java.util.StringTokenizer st = new java.util.StringTokenizer(newName);
			if (!st.hasMoreTokens())
			{
				return SCRIPT_CONTINUE;
			}
			
			String currentFirstName = getFirstName(self);
			if ((currentFirstName == null) || (currentFirstName.length() <= 0))
			{
				return SCRIPT_CONTINUE;
			}
			
			if (!currentFirstName.equalsIgnoreCase(st.nextToken()))
			{
				String disableRenameChatAvatar = getConfigSetting("CharacterRename", "disableRenameChatAvatar");
				if (disableRenameChatAvatar == null)
				{
					disableRenameChatAvatar = "false";
				}
				
				if (disableRenameChatAvatar.equalsIgnoreCase("true") || disableRenameChatAvatar.equalsIgnoreCase("yes"))
				{
					announcement += "Please note that this character's mail, friends list, and ignore list will be lost after the rename.\n\n";
				}
				else
				{
					announcement += "Please note that even though this character's mail, friends list, and ignore list will be preserved with the rename, it is still recommended that you make a backup of this character's mail, friends list, and ignore list.\n\n";
				}
			}
			
			announcement += "Once the rename request has been submitted (by clicking the Yes button below), you will be disconnected and will not be able to log in this character until the rename request is completed. It can take up to 30 minutes for the rename request to complete.\n\n";
			announcement += "After the rename, there will be a 90 days wait before you can rename this character again.\n\n";
			announcement += "Do you wish to rename this character?";
			
			int pid = sui.msgbox(self, self, announcement, sui.YES_NO, "Rename Character", "handleRenameCharacterConfirmation");
			utils.setScriptVar(self, SCRIPTVAR_RENAME_CHARACTER_SUI_ID, pid);
			utils.setScriptVar(self, SCRIPTVAR_RENAME_CHARACTER_NEW_NAME, newName);
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int handleRenameCharacterConfirmation(obj_id self, dictionary params) throws InterruptedException
	{
		if (!utils.hasScriptVar(self, SCRIPTVAR_RENAME_CHARACTER_SUI_ID))
		{
			return SCRIPT_CONTINUE;
		}
		
		if (!utils.hasScriptVar(self, SCRIPTVAR_RENAME_CHARACTER_NEW_NAME))
		{
			return SCRIPT_CONTINUE;
		}
		
		if (params.getInt("pageId") != utils.getIntScriptVar(self, SCRIPTVAR_RENAME_CHARACTER_SUI_ID))
		{
			return SCRIPT_CONTINUE;
		}
		
		String newName = utils.getStringScriptVar(self, SCRIPTVAR_RENAME_CHARACTER_NEW_NAME);
		
		utils.removeScriptVar(self, SCRIPTVAR_RENAME_CHARACTER_SUI_ID);
		utils.removeScriptVar(self, SCRIPTVAR_RENAME_CHARACTER_NEW_NAME);
		
		if ((newName == null) || (newName.length() <= 0))
		{
			return SCRIPT_CONTINUE;
		}
		
		int bp = sui.getIntButtonPressed(params);
		if (bp == sui.BP_OK)
		{
			
			renameCharacter(self, newName);
		}
		else
		{
			
			renameCharacterReleaseNameReservation(self);
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public void renameCharacterDisplayNameInput(obj_id self, String newName, String reason) throws InterruptedException
	{
		if (!isIdValid(self) || !exists(self))
		{
			return;
		}
		
		String announcement = "Enter a new name for the character.";
		if ((reason != null) && (reason.length() > 0))
		{
			announcement = reason;
		}
		
		int pid = sui.inputbox(self, self, announcement, "Rename Character", "handleRenameCharacterNameInput", 125, false, "");
		sui.setSUIProperty(pid, sui.INPUTBOX_INPUT, sui.PROP_TEXT, newName);
		sui.setSUIProperty(pid, sui.INPUTBOX_INPUT, "MaxLength", String.valueOf(125));
		utils.setScriptVar(self, SCRIPTVAR_RENAME_CHARACTER_SUI_ID, pid);
		showSUIPage(pid);
	}
	
	
	public int closeLeaderSui(obj_id self, dictionary params) throws InterruptedException
	{
		obj_id player = sui.getPlayerId(params);
		if (!isValidId(player))
		{
			return SCRIPT_CONTINUE;
		}
		
		sui.removePid(player, holiday.LEADERSUI_PID_NAME);
		return SCRIPT_CONTINUE;
	}
	
	
	public int handlerCommunityBadgeGrant(obj_id self, dictionary params) throws InterruptedException
	{
		if (params == null || params.isEmpty())
		{
			return SCRIPT_CONTINUE;
		}
		
		obj_id communityRep = params.getObjId("communityRep");
		String badgeName = params.getString("badgeName");
		
		badge.grantBadge(self, badgeName);
		
		params.put("player", self);
		
		messageTo(communityRep, "handlerCommunityBadgeGrantedToPlayer", params, 0, true);
		CustomerServiceLog("EventPerk", "[EventTool] 'handlerCommunityBadgeGrant' player "+ getPlayerName(self) + ":"+ self + " was awarded badge "+ badgeName + " by "+ getPlayerName(communityRep) + ":"+ communityRep + " .", null);
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int recalculateCampDefenseUndeadArray(obj_id self, dictionary params) throws InterruptedException
	{
		if ((params == null) || (params.isEmpty()))
		{
			CustomerServiceLog("outbreak_themepark", "player_utility.recalculateCampDefenseUndeadArray() Params invalid.");
			return SCRIPT_CONTINUE;
			
		}
		
		if (!params.containsKey("questName"))
		{
			CustomerServiceLog("outbreak_themepark", "player_utility.recalculateCampDefenseUndeadArray() questName param not found.");
			return SCRIPT_CONTINUE;
		}
		
		if (!params.containsKey("survivalTaskName"))
		{
			CustomerServiceLog("outbreak_themepark", "player_utility.recalculateCampDefenseUndeadArray() survivalTaskName param not found.");
			return SCRIPT_CONTINUE;
		}
		
		if (!params.containsKey("cleanUpTaskName"))
		{
			CustomerServiceLog("outbreak_themepark", "player_utility.recalculateCampDefenseUndeadArray() cleanUpTaskName param not found.");
			return SCRIPT_CONTINUE;
		}
		String questName = params.getString("questName");
		if (questName == null || questName.length() <= 0)
		{
			CustomerServiceLog("outbreak_themepark", "player_utility.recalculateCampDefenseUndeadArray() player "+self+" has received an invalid quest name.");
			return SCRIPT_CONTINUE;
		}
		
		String survivalTaskName = params.getString("survivalTaskName");
		if (survivalTaskName == null || survivalTaskName.length() <= 0)
		{
			CustomerServiceLog("outbreak_themepark", "player_utility.recalculateCampDefenseUndeadArray() player "+self+" has received an invalid quest survivalTaskName name.");
			return SCRIPT_CONTINUE;
		}
		
		String cleanUpTaskName = params.getString("cleanUpTaskName");
		if (cleanUpTaskName == null || cleanUpTaskName.length() <= 0)
		{
			CustomerServiceLog("outbreak_themepark", "player_utility.recalculateCampDefenseUndeadArray() player "+self+" has received an invalid quest cleanUpTaskName name.");
			return SCRIPT_CONTINUE;
		}
		
		if (!utils.hasScriptVar(self, "enemyList"))
		{
			CustomerServiceLog("outbreak_themepark", "player_utility.recalculateCampDefenseUndeadArray() Has no script var list of enemies, sending signal.");
			if (!groundquests.isQuestActive(self, questName))
			{
				CustomerServiceLog("outbreak_themepark", "player_utility.recalculateCampDefenseUndeadArray() Player "+self+" did not have the appropriate quest active. Aborting quest signal.");
				return SCRIPT_CONTINUE;
			}
			
			if (groundquests.isTaskActive(self, questName, survivalTaskName))
			{
				CustomerServiceLog("outbreak_themepark", "camp_defense.updatePlayerDefense() Player "+self+" did not have the appropriate quest TASK active. Sending to new handler.");
				
				messageTo(self, "recalculateCampDefenseUndeadArray", params, 3, false );
				
				return SCRIPT_CONTINUE;
			}
			
			groundquests.sendSignal(self, "hasKilledWave");
			removeGuardPostFlag(self);
			
			return SCRIPT_CONTINUE;
		}
		
		obj_id[] enemies = utils.getObjIdArrayScriptVar(self, "enemyList");
		if (enemies == null || enemies.length == 0)
		{
			CustomerServiceLog("outbreak_themepark", "player_utility.recalculateCampDefenseUndeadArray() Has the scriptvar but could not find a valid enemy list on player, sending signal.");
			if (!groundquests.isQuestActive(self, questName))
			{
				CustomerServiceLog("outbreak_themepark", "player_utility.recalculateCampDefenseUndeadArray() Player "+self+" did not have the appropriate quest active. Destroying all enemies.");
				for (int i = 0; i < enemies.length; i++)
				{
					testAbortScript();
					if ((!isIdValid(enemies[i])) || !exists(enemies[i]) || isDead(enemies[i]))
					{
						continue;
					}
					destroyObject(enemies[i]);
				}
				utils.removeScriptVar(self,"enemyList");
				return SCRIPT_CONTINUE;
			}
			
			if (groundquests.isTaskActive(self, questName, survivalTaskName))
			{
				CustomerServiceLog("outbreak_themepark", "camp_defense.updatePlayerDefense() Player "+self+" did not have the appropriate quest TASK active. Sending to new handler.");
				messageTo(self, "recalculateCampDefenseUndeadArray", params, 3, false );
				
				return SCRIPT_CONTINUE;
			}
			
			groundquests.sendSignal(self, "hasKilledWave");
			
			removeGuardPostFlag(self);
			
			return SCRIPT_CONTINUE;
		}
		
		Vector latestGuardList = new Vector();
		for (int i = 0; i < enemies.length; i++)
		{
			testAbortScript();
			if ((isIdNull(enemies[i])) || !exists(enemies[i]) || isDead(enemies[i]))
			{
				continue;
			}
			utils.addElement(latestGuardList, enemies[i]);
		}
		obj_id[] newGuardArray = new obj_id[latestGuardList.size()];
		if (newGuardArray == null || newGuardArray.length == 0)
		{
			CustomerServiceLog("outbreak_themepark", "player_utility.recalculateCampDefenseUndeadArray() could not find a valid enemy list on player, sending signal.");
			if (!groundquests.isQuestActive(self, questName))
			{
				CustomerServiceLog("outbreak_themepark", "player_utility.recalculateCampDefenseUndeadArray() Player "+self+" did not have the appropriate quest active. Aborting quest signal.");
				return SCRIPT_CONTINUE;
			}
			
			if (groundquests.isTaskActive(self, questName, survivalTaskName))
			{
				CustomerServiceLog("outbreak_themepark", "player_utility.recalculateCampDefenseUndeadArray() Player "+self+" did not have the appropriate quest TASK active. Handling Timer.");
				
				messageTo(self, "recalculateCampDefenseUndeadArray", params, 3, false );
				
				return SCRIPT_CONTINUE;
			}
			
			groundquests.sendSignal(self, "hasKilledWave");
			removeGuardPostFlag(self);
			
			return SCRIPT_CONTINUE;
		}
		CustomerServiceLog("outbreak_themepark", "player_utility.recalculateCampDefenseUndeadArray() player: "+self+" still has enemies to kill.");
		
		latestGuardList.toArray(newGuardArray);
		utils.setScriptVar(self, "enemyList", newGuardArray);
		return SCRIPT_CONTINUE;
	}
	
	
	public boolean removeGuardPostFlag(obj_id player) throws InterruptedException
	{
		if (!isValidId(player) || !exists(player))
		{
			return false;
		}
		CustomerServiceLog("outbreak_themepark", "player_utility.removeGuardPostFlag() Player "+player+" is attempting to remove the guard post occupied flagging.");
		
		if (utils.hasScriptVar(player, GUARD_OCCUPIED))
		{
			obj_id guardPost = utils.getObjIdScriptVar(player, GUARD_OCCUPIED);
			if (isValidId(guardPost) && exists(guardPost))
			{
				utils.removeScriptVar(guardPost, GUARD_OCCUPIED);
				return true;
			}
		}
		CustomerServiceLog("outbreak_themepark", "player_utility.removeGuardPostFlag() Player "+player+" did not have the guard post oid or the oid was invalid.");
		return false;
	}
	
	
	public int handleEntertainingGcwTroops(obj_id self, dictionary params) throws InterruptedException
	{
		if (params == null)
		{
			return SCRIPT_CONTINUE;
		}
		
		if (!utils.hasScriptVar(self, gcw.GCW_SCRIPTVAR_PARENT + ".gcwNpc"))
		{
			LOG("gcw_entertainer","no gcwNpc");
			
			utils.removeScriptVarTree(self, gcw.GCW_SCRIPTVAR_PARENT);
			sui.removePid(self, gcw.ENTERTAIN_GCW_TROOPS_PID);
			return SCRIPT_CONTINUE;
		}
		
		obj_id npc = utils.getObjIdScriptVar(self, gcw.GCW_SCRIPTVAR_PARENT + ".gcwNpc");
		if (!isValidId(npc) || !exists(npc) || ai_lib.aiIsDead(npc))
		{
			LOG("gcw_entertainer","npc not valid");
			
			utils.removeScriptVarTree(self, gcw.GCW_SCRIPTVAR_PARENT);
			sui.removePid(self, gcw.ENTERTAIN_GCW_TROOPS_PID);
			return SCRIPT_CONTINUE;
		}
		
		if (utils.hasScriptVar(npc, gcw.GCW_STOPPED_ENTERTAINING))
		{
			LOG("gcw_entertainer","had gcw.GCW_STOPPED_ENTERTAINING");
			
			utils.removeScriptVarTree(self, gcw.GCW_SCRIPTVAR_PARENT);
			cleanUpGuardPostNpc(npc);
			sui.removePid(self, gcw.ENTERTAIN_GCW_TROOPS_PID);
			return SCRIPT_CONTINUE;
		}
		
		int pid = params.getInt("id");
		int test_pid = getIntObjVar(self, sui.COUNTDOWNTIMER_SUI_VAR);
		
		if (pid != test_pid)
		{
			LOG("gcw_entertainer","pid != test_pid -- just ignored!");
			return SCRIPT_CONTINUE;
		}
		
		if (!sui.hasPid(self, gcw.ENTERTAIN_GCW_TROOPS_PID))
		{
			LOG("gcw_entertainer","no gcw.ENTERTAIN_GCW_TROOPS_PID");
			
			utils.removeScriptVarTree(self, gcw.GCW_SCRIPTVAR_PARENT);
			cleanUpGuardPostNpc(npc);
			sui.removePid(self, gcw.ENTERTAIN_GCW_TROOPS_PID);
			return SCRIPT_CONTINUE;
		}
		
		test_pid = sui.getPid(self, gcw.ENTERTAIN_GCW_TROOPS_PID);
		
		if (test_pid != pid)
		{
			LOG("gcw_entertainer","gcw.ENTERTAIN_GCW_TROOPS_PID != pid");
			
			utils.removeScriptVarTree(self, gcw.GCW_SCRIPTVAR_PARENT);
			cleanUpGuardPostNpc(npc);
			sui.removePid(self, gcw.ENTERTAIN_GCW_TROOPS_PID);
			return SCRIPT_CONTINUE;
		}
		
		if (!hasObjVar(self, sui.COUNTDOWNTIMER_SUI_VAR))
		{
			LOG("gcw_entertainer","no sui.COUNTDOWNTIMER_SUI_VAR");
			
			utils.removeScriptVarTree(self, gcw.GCW_SCRIPTVAR_PARENT);
			cleanUpGuardPostNpc(npc);
			sui.removePid(self, gcw.ENTERTAIN_GCW_TROOPS_PID);
			return SCRIPT_CONTINUE;
		}
		
		int bp = sui.getIntButtonPressed(params);
		if (bp == sui.BP_CANCEL)
		{
			LOG("gcw_entertainer","had BP_CANCEL");
			
			utils.removeScriptVarTree(self, gcw.GCW_SCRIPTVAR_PARENT);
			cleanUpGuardPostNpc(npc);
			sui.removePid(self, gcw.ENTERTAIN_GCW_TROOPS_PID);
			return SCRIPT_CONTINUE;
		}
		else if (bp == sui.BP_REVERT)
		{
			LOG("gcw_entertainer","had BP_REVERT");
			
			int event = params.getInt("event");
			
			if (event == sui.CD_EVENT_LOCOMOTION)
			{
				sendSystemMessage(self, gcw.SID_COUNTDOWN_LOCOMOTION);
			}
			else if (event == sui.CD_EVENT_INCAPACITATE)
			{
				sendSystemMessage(self, gcw.SID_INTERRUPTED_INCAPACITATED);
			}
			else if (event == sui.CD_EVENT_DAMAGED || event == sui.CD_EVENT_COMBAT)
			{
				sendSystemMessage(self, gcw.SID_INTERRUPTED_DAMAGED);
			}
			
			utils.removeScriptVarTree(self, gcw.GCW_SCRIPTVAR_PARENT);
			cleanUpGuardPostNpc(npc);
			sui.removePid(self, gcw.ENTERTAIN_GCW_TROOPS_PID);
			return SCRIPT_CONTINUE;
		}
		
		if (!utils.hasScriptVar(self, gcw.GCW_SCRIPTVAR_PARENT + ".playerEntertainmentStart"))
		{
			LOG("gcw_entertainer","missing gcw.GCW_SCRIPTVAR_PARENT playerEntertainmentStart");
			
			utils.removeScriptVarTree(self, gcw.GCW_SCRIPTVAR_PARENT);
			sui.removePid(self, gcw.ENTERTAIN_GCW_TROOPS_PID);
			cleanUpGuardPostNpc(npc);
			return SCRIPT_CONTINUE;
		}
		
		forceCloseSUIPage(pid);
		
		utils.removeScriptVarTree(self, gcw.GCW_SCRIPTVAR_PARENT);
		utils.removeScriptVarTree(npc, gcw.GCW_SCRIPTVAR_PARENT);
		sui.removePid(self, gcw.ENTERTAIN_GCW_TROOPS_PID);
		
		utils.removeScriptVar(npc, "ai.oldEntertainerSkillMod");
		utils.removeScriptVar(npc, "ai.listeningTo");
		stop(npc);
		messageTo(npc, "resumeDefaultCalmBehavior", null, 0, false);
		
		LOG("gcw_entertainer","YOU WIN!");
		if (factions.isRebelorRebelHelper(self))
		{
			LOG("gcw_entertainer","handleEntertainingGcwTroops Rebel task was signaled");
			groundquests.sendSignal(self, gcw.GCW_REB_ENTERTAIN_PATROL_SIGNAL);
		}
		else if (factions.isImperialorImperialHelper(self))
		{
			LOG("gcw_entertainer","handleEntertainingGcwTroops Imp task was signaled");
			groundquests.sendSignal(self, gcw.GCW_IMP_ENTERTAIN_PATROL_SIGNAL);
		}
		else
		{
			LOG("gcw_entertainer","handleEntertainingGcwTroops NO FACTION");
		}
		
		if (hasObjVar(npc, gcw.GCW_PATROL_OBJ))
		{
			LOG("gcw_entertainer","Has patrol_point var");
			
			obj_id patrolPoint = getObjIdObjVar(npc, gcw.GCW_PATROL_OBJ);
			if (isValidId(patrolPoint))
			{
				LOG("gcw_entertainer","Flagging obj: "+patrolPoint+" as entertained");
				messageTo(patrolPoint, "flagGCWPatrolEntertained", null, 0, false);
				sendSystemMessage(self, SID_PATROL_POINT_ENTERTAINED);
				
				trial.addNonInstanceFactionParticipant(self, patrolPoint);
			}
		}
		
		cleanUpGuardPostNpc(npc);
		
		return SCRIPT_CONTINUE;
	}
	
	
	public boolean cleanUpGuardPostNpc(obj_id npc) throws InterruptedException
	{
		if (!isValidId(npc) || !exists(npc))
		{
			return false;
		}
		
		if (utils.hasScriptVar(npc, gcw.GCW_NPC_CLEANUP_FLAG))
		{
			
			trial.cleanupObject(npc);
		}
		else
		{
			utils.removeScriptVarTree(npc, gcw.GCW_SCRIPTVAR_PARENT);
		}
		return true;
	}
	
	
	public int handleTraderRepairQuest(obj_id self, dictionary params) throws InterruptedException
	{
		if (params == null)
		{
			removeTraderRepairScriptVars(self);
			return SCRIPT_CONTINUE;
		}
		int pid = params.getInt("id");
		int playerPid = getIntObjVar(self, sui.COUNTDOWNTIMER_SUI_VAR);
		if (pid != playerPid)
		{
			removeTraderRepairScriptVars(self);
			return SCRIPT_CONTINUE;
		}
		
		int bp = sui.getIntButtonPressed(params);
		if (bp == sui.BP_CANCEL)
		{
			LOG("gcw_patrol_point","handleTraderRepairQuest had BP_CANCEL");
			
			removeTraderRepairScriptVars(self);
			return SCRIPT_CONTINUE;
		}
		else if (bp == sui.BP_REVERT)
		{
			LOG("gcw_patrol_point","handleTraderRepairQuest had BP_REVERT");
			
			int event = params.getInt("event");
			
			if (event == sui.CD_EVENT_LOCOMOTION)
			{
				sendSystemMessage(self, gcw.SID_COUNTDOWN_LOCOMOTION);
			}
			else if (event == sui.CD_EVENT_INCAPACITATE)
			{
				sendSystemMessage(self, gcw.SID_INTERRUPTED_INCAPACITATED);
			}
			else if (event == sui.CD_EVENT_DAMAGED || event == sui.CD_EVENT_COMBAT)
			{
				sendSystemMessage(self, gcw.SID_INTERRUPTED_DAMAGED);
			}
			
			removeTraderRepairScriptVars(self);
			return SCRIPT_CONTINUE;
		}
		
		if (!utils.hasScriptVar(self, gcw.OBJECT_TO_REPAIR))
		{
			removeTraderRepairScriptVars(self);
			return SCRIPT_CONTINUE;
		}
		if (!utils.hasScriptVar(self, gcw.GCW_REPAIR_QUEST))
		{
			removeTraderRepairScriptVars(self);
			return SCRIPT_CONTINUE;
		}
		if (!utils.hasScriptVar(self, gcw.GCW_REPAIR_RESOURCE_COUNT))
		{
			removeTraderRepairScriptVars(self);
			return SCRIPT_CONTINUE;
		}
		
		LOG("gcw_patrol_point","handleTraderRepairQuest passed validation");
		
		String questName = utils.getStringScriptVar(self, gcw.GCW_REPAIR_QUEST);
		if (questName == null || questName.length() <= 0)
		{
			removeTraderRepairScriptVars(self);
			return SCRIPT_CONTINUE;
		}
		
		if (!groundquests.isQuestActive(self, questName))
		{
			removeTraderRepairScriptVars(self);
			return SCRIPT_CONTINUE;
		}
		
		LOG("gcw_constrction","handleTraderRepairQuest checkign for construction tasks");
		
		if (!groundquests.isTaskActive(self, questName, gcw.GCW_CONSTRUCTION_TASK+"_"+questName))
		{
			removeTraderRepairScriptVars(self);
			return SCRIPT_CONTINUE;
		}
		
		LOG("gcw_constrction","handleTraderRepairQuest a task was found");
		
		obj_id gcwObject = utils.getObjIdScriptVar(self, gcw.OBJECT_TO_REPAIR);
		if (!isValidId(gcwObject) || !exists(gcwObject))
		{
			removeTraderRepairScriptVars(self);
			return SCRIPT_CONTINUE;
		}
		
		int resourceCount = utils.getIntScriptVar(self, gcw.GCW_REPAIR_RESOURCE_COUNT);
		if (resourceCount <= 0)
		{
			removeTraderRepairScriptVars(self);
			return SCRIPT_CONTINUE;
		}
		
		LOG("gcw_patrol_point","handleTraderRepairQuest");
		if (!gcw.repairGcwObject(gcwObject, self, resourceCount))
		{
			removeTraderRepairScriptVars(self);
			return SCRIPT_CONTINUE;
		}
		
		LOG("gcw_constrction","handleTraderRepairQuest task was signaled");
		groundquests.sendSignal(self, gcw.GCW_CONSTRUCTION_SIGNAL+"_"+questName);
		
		buff.applyBuff(self, "gcw_fatigue");
		trial.addNonInstanceFactionParticipant(self, gcwObject);
		removeTraderRepairScriptVars(self);
		return SCRIPT_CONTINUE;
	}
	
	
	public int handleOpposingFactionScoutQuest(obj_id self, dictionary params) throws InterruptedException
	{
		LOG("gcw_patrol_point","handleOpposingFactionScoutQuest init");
		
		if (params == null)
		{
			return SCRIPT_CONTINUE;
		}
		
		int pid = params.getInt("id");
		int playerPid = getIntObjVar(self, sui.COUNTDOWNTIMER_SUI_VAR);
		if (pid != playerPid)
		{
			return SCRIPT_CONTINUE;
		}
		
		int bp = sui.getIntButtonPressed(params);
		if (bp == sui.BP_CANCEL)
		{
			LOG("gcw_patrol_point","handleOpposingFactionScoutQuest had BP_CANCEL");
			utils.removeScriptVar(self, "spyPatrolPoint");
			sui.removePid(self, gcw.SPY_SCOUT_PID);
			return SCRIPT_CONTINUE;
		}
		else if (bp == sui.BP_REVERT)
		{
			LOG("gcw_patrol_point","handleOpposingFactionScoutQuest had BP_REVERT");
			
			int event = params.getInt("event");
			
			if (event == sui.CD_EVENT_LOCOMOTION)
			{
				sendSystemMessage(self, gcw.SID_COUNTDOWN_LOCOMOTION);
			}
			else if (event == sui.CD_EVENT_INCAPACITATE)
			{
				sendSystemMessage(self, gcw.SID_INTERRUPTED_INCAPACITATED);
			}
			else if (event == sui.CD_EVENT_DAMAGED || event == sui.CD_EVENT_COMBAT)
			{
				sendSystemMessage(self, gcw.SID_INTERRUPTED_DAMAGED);
			}
			utils.removeScriptVar(self, "spyPatrolPoint");
			sui.removePid(self, gcw.SPY_SCOUT_PID);
			return SCRIPT_CONTINUE;
		}
		
		LOG("gcw_patrol_point","handleOpposingFactionScoutQuest passed validation");
		
		if (!utils.hasScriptVar(self, "spyPatrolPoint"))
		{
			sui.removePid(self, gcw.SPY_DESTROY_PID);
			return SCRIPT_CONTINUE;
		}
		
		obj_id patrolPoint = utils.getObjIdScriptVar(self, "spyPatrolPoint");
		if (!isValidId(patrolPoint) || !exists(patrolPoint))
		{
			utils.removeScriptVar(self, "spyPatrolPoint");
			sui.removePid(self, gcw.SPY_DESTROY_PID);
			return SCRIPT_CONTINUE;
		}
		
		if (!groundquests.isQuestActive(self, gcw.GCW_SPY_PATROL_SCOUT_QUEST))
		{
			utils.removeScriptVar(self, "spyPatrolPoint");
			sui.removePid(self, gcw.SPY_SCOUT_PID);
			return SCRIPT_CONTINUE;
		}
		if ((!groundquests.isTaskActive(self, gcw.GCW_SPY_PATROL_SCOUT_QUEST, gcw.GCW_REB_FIND_PATROL_SIGNAL) && !groundquests.isTaskActive(self, gcw.GCW_SPY_PATROL_SCOUT_QUEST, gcw.GCW_IMP_FIND_PATROL_SIGNAL)))
		{
			utils.removeScriptVar(self, "spyPatrolPoint");
			sui.removePid(self, gcw.SPY_SCOUT_PID);
			return SCRIPT_CONTINUE;
		}
		
		if (factions.isRebelorRebelHelper(self))
		{
			LOG("gcw_patrol_point","handleOpposingFactionScoutQuest Rebel task was signaled");
			groundquests.sendSignal(self, gcw.GCW_REB_FIND_PATROL_SIGNAL);
		}
		else if (factions.isImperialorImperialHelper(self))
		{
			LOG("gcw_patrol_point","handleOpposingFactionScoutQuest Imp task was signaled");
			groundquests.sendSignal(self, gcw.GCW_IMP_FIND_PATROL_SIGNAL);
		}
		else
		{
			LOG("gcw_patrol_point","handleOpposingFactionScoutQuest NO FACTION");
			utils.removeScriptVar(self, "spyPatrolPoint");
			sui.removePid(self, gcw.SPY_SCOUT_PID);
			return SCRIPT_CONTINUE;
		}
		
		trial.addNonInstanceFactionParticipant(self, patrolPoint);
		utils.removeScriptVar(self, "spyPatrolPoint");
		sui.removePid(self, gcw.SPY_SCOUT_PID);
		return SCRIPT_CONTINUE;
	}
	
	
	public int handleOpposingFactionDestroyQuest(obj_id self, dictionary params) throws InterruptedException
	{
		if (params == null)
		{
			return SCRIPT_CONTINUE;
		}
		
		int pid = params.getInt("id");
		int playerPid = getIntObjVar(self, sui.COUNTDOWNTIMER_SUI_VAR);
		if (pid != playerPid)
		{
			utils.removeScriptVar(self, "spyPatrolPoint");
			sui.removePid(self, gcw.SPY_DESTROY_PID);
			return SCRIPT_CONTINUE;
		}
		
		int bp = sui.getIntButtonPressed(params);
		if (bp == sui.BP_CANCEL)
		{
			LOG("gcw_patrol_point","handleOpposingFactionDestroyQuest had BP_CANCEL");
			utils.removeScriptVar(self, "spyPatrolPoint");
			sui.removePid(self, gcw.SPY_DESTROY_PID);
			return SCRIPT_CONTINUE;
		}
		else if (bp == sui.BP_REVERT)
		{
			LOG("gcw_patrol_point","handleOpposingFactionDestroyQuest had BP_REVERT");
			
			int event = params.getInt("event");
			
			if (event == sui.CD_EVENT_LOCOMOTION)
			{
				sendSystemMessage(self, gcw.SID_COUNTDOWN_LOCOMOTION);
			}
			else if (event == sui.CD_EVENT_INCAPACITATE)
			{
				sendSystemMessage(self, gcw.SID_INTERRUPTED_INCAPACITATED);
			}
			else if (event == sui.CD_EVENT_DAMAGED || event == sui.CD_EVENT_COMBAT)
			{
				sendSystemMessage(self, gcw.SID_INTERRUPTED_DAMAGED);
			}
			
			utils.removeScriptVar(self, "spyPatrolPoint");
			sui.removePid(self, gcw.SPY_DESTROY_PID);
			return SCRIPT_CONTINUE;
		}
		
		if (!utils.hasScriptVar(self, "spyPatrolPoint"))
		{
			utils.removeScriptVar(self, "spyPatrolPoint");
			sui.removePid(self, gcw.SPY_DESTROY_PID);
			return SCRIPT_CONTINUE;
		}
		
		if (!groundquests.isQuestActive(self, gcw.GCW_SPY_PATROL_DESTROY_QUEST))
		{
			utils.removeScriptVar(self, "spyPatrolPoint");
			sui.removePid(self, gcw.SPY_DESTROY_PID);
			return SCRIPT_CONTINUE;
		}
		
		LOG("gcw_patrol_point","handleOpposingFactionDestroyQuest passed validation");
		obj_id patrolPoint = utils.getObjIdScriptVar(self, "spyPatrolPoint");
		if (!isValidId(patrolPoint) || !exists(patrolPoint))
		{
			utils.removeScriptVar(self, "spyPatrolPoint");
			sui.removePid(self, gcw.SPY_DESTROY_PID);
			return SCRIPT_CONTINUE;
		}
		
		if (!buff.hasBuff(self, gcw.BUFF_SPY_EXPLOSIVES))
		{
			utils.removeScriptVar(self, "spyPatrolPoint");
			sui.removePid(self, gcw.SPY_DESTROY_PID);
			return SCRIPT_CONTINUE;
		}
		
		long counter = buff.getBuffStackCount(self, gcw.BUFF_SPY_EXPLOSIVES);
		if (counter <= 0)
		{
			utils.removeScriptVar(self, "spyPatrolPoint");
			sui.removePid(self, gcw.SPY_DESTROY_PID);
			return SCRIPT_CONTINUE;
		}
		
		if (!buff.decrementBuffStack(self, gcw.BUFF_SPY_EXPLOSIVES))
		{
			utils.removeScriptVar(self, "spyPatrolPoint");
			sui.removePid(self, gcw.SPY_DESTROY_PID);
			return SCRIPT_CONTINUE;
		}
		
		int currentHp = getHitpoints(patrolPoint);
		int maxHp = getMaxHitpoints(patrolPoint);
		
		if (currentHp > 0)
		{
			if (maxHp > 0)
			{
				double dmg = maxHp * .2;
				if (dmg < currentHp)
				{
					setHitpoints(patrolPoint, (currentHp - (int)dmg));
					playClientEffectLoc(patrolPoint, "clienteffect/combat_explosion_lair_large.cef", getLocation(patrolPoint), 0);
					return SCRIPT_CONTINUE;
				}
				else
				{
					setHitpoints(patrolPoint, 0);
				}
			}
		}
		
		if (factions.isRebelorRebelHelper(self))
		{
			LOG("gcw_constrction","handleTraderRepairQuest Rebel task was signaled");
			groundquests.sendSignal(self, gcw.GCW_REB_DESTROY_PATROL_SIGNAL);
		}
		else if (factions.isImperialorImperialHelper(self))
		{
			LOG("gcw_constrction","handleTraderRepairQuest Imp task was signaled");
			groundquests.sendSignal(self, gcw.GCW_IMP_DESTROY_PATROL_SIGNAL);
		}
		else
		{
			LOG("gcw_constrction","handleTraderRepairQuest NO FACTION");
			utils.removeScriptVar(self, "spyPatrolPoint");
			sui.removePid(self, gcw.SPY_DESTROY_PID);
			return SCRIPT_CONTINUE;
		}
		
		trial.addNonInstanceFactionParticipant(self, patrolPoint);
		buff.removeBuff(self, gcw.BUFF_SPY_EXPLOSIVES);
		groundquests.sendSignal(self, "hasDestroyedPatrolPoint");
		
		utils.removeScriptVar(self, "spyPatrolPoint");
		sui.removePid(self, gcw.SPY_DESTROY_PID);
		return SCRIPT_CONTINUE;
	}
	
	
	public boolean removeTraderRepairScriptVars(obj_id player) throws InterruptedException
	{
		if (!isValidId(player) || !exists(player))
		{
			return false;
		}
		sui.removePid(player, gcw.TRADER_REPAIR_PID);
		utils.removeScriptVar(player, gcw.OBJECT_TO_REPAIR);
		utils.removeScriptVar(player, gcw.GCW_REPAIR_RESOURCE_COUNT);
		utils.removeScriptVar(player, gcw.GCW_REPAIR_QUEST);
		return true;
	}
	
	
	public int notifyPlayerOfGcwCityEventAnnouncement(obj_id self, dictionary params) throws InterruptedException
	{
		LOG("gcw_announcement", "notifyPlayerOfGcwCityEventAnnouncement init");
		
		if (params == null)
		{
			return SCRIPT_CONTINUE;
		}
		
		if ((!params.containsKey("string_file") || !params.containsKey("planetName") || !params.containsKey("cityName")))
		{
			return SCRIPT_CONTINUE;
		}
		
		String stringFile = params.getString("string_file");
		if (stringFile == null || stringFile.length() <= 0)
		{
			return SCRIPT_CONTINUE;
		}
		
		String imperialAnnouncement = "";
		if (params.containsKey("imperial_announcement"))
		{
			imperialAnnouncement = params.getString("imperial_announcement");
			if (imperialAnnouncement == null || imperialAnnouncement.length() <= 0)
			{
				return SCRIPT_CONTINUE;
			}
		}
		
		String rebelAnnouncement = "";
		if (params.containsKey("rebel_announcement"))
		{
			rebelAnnouncement = params.getString("rebel_announcement");
			if (rebelAnnouncement == null || rebelAnnouncement.length() <= 0)
			{
				return SCRIPT_CONTINUE;
			}
		}
		
		String neutralAnnouncement = "";
		if (params.containsKey("neutral_announcement"))
		{
			neutralAnnouncement = params.getString("neutral_announcement");
			if (neutralAnnouncement == null || neutralAnnouncement.length() <= 0)
			{
				return SCRIPT_CONTINUE;
			}
		}
		String planetName = params.getString("planetName");
		if (planetName == null || planetName.length() <= 0)
		{
			return SCRIPT_CONTINUE;
		}
		
		String cityName = params.getString("cityName");
		if (cityName == null || cityName.length() <= 0)
		{
			return SCRIPT_CONTINUE;
		}
		
		String npc = "object/mobile/dressed_imperial_moff_m.iff";
		String sound = "sound/sys_comm_imperial.snd";
		prose_package pp = new prose_package();
		
		if (factions.isImperial(self) && imperialAnnouncement.length() > 0)
		{
			LOG("gcw_announcement", "notifyPlayerOfGcwCityEventAnnouncement player is Imperial");
			prose.setStringId(pp, new string_id(stringFile, imperialAnnouncement + planetName +"_"+cityName));
		}
		else if (factions.isRebel(self) && rebelAnnouncement.length() > 0)
		{
			LOG("gcw_announcement", "notifyPlayerOfGcwCityEventAnnouncement player is Rebel");
			npc = "object/mobile/dressed_rebel_high_general_human_male_01.iff";
			sound = "sound/sys_comm_rebel_male.snd";
			prose.setStringId(pp, new string_id(stringFile, rebelAnnouncement + planetName +"_"+cityName));
		}
		else
		{
			LOG("gcw_announcement", "notifyPlayerOfGcwCityEventAnnouncement player is NEUTRAL");
			npc = "object/mobile/dressed_noble_human_male_01.iff";
			sound = "sound/sys_comm_rebel_male.snd";
			prose.setStringId(pp, new string_id(stringFile, neutralAnnouncement + planetName +"_"+cityName));
		}
		
		commPlayers(self, npc, sound, 12f, self, pp);
		return SCRIPT_CONTINUE;
	}
	
	
	public int onGcwFactionalPresenceTableDictionaryResponse(obj_id self, dictionary params) throws InterruptedException
	{
		if (utils.hasScriptVar(self, "gcw.factionalPresenceTablePid"))
		{
			utils.removeScriptVar(self, "gcw.factionalPresenceTablePid");
		}
		
		return SCRIPT_CONTINUE;
	}
	
}
