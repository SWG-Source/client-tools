package script.faction_perk.hq;

import script.*;
import script.base_class.*;
import script.combat_engine.*;
import java.util.Arrays;
import java.util.Hashtable;
import java.util.Vector;
import script.base_script;


import java.util.Calendar;
import script.library.hq;
import script.library.sui;
import script.library.utils;
import script.library.prose;
import script.library.money;
import script.library.factions;
import script.library.player_structure;
import script.library.stealth;



public class terminal extends script.terminal.base.base_terminal
{
	public terminal()
	{
	}
	public static final string_id MNU_DONATE = new string_id("hq", "mnu_donate");
	public static final string_id MNU_DONATE_MONEY = new string_id("hq", "mnu_donate_money");
	public static final string_id MNU_DONATE_RESOURCE = new string_id("hq", "mnu_donate_resource");
	public static final string_id MNU_DONATE_DEED = new string_id("hq", "mnu_donate_deed");
	public static final string_id UNDER_ATTACK = new string_id("hq", "under_attack");
	
	public static final string_id MNU_OVERLOAD = new string_id("hq", "mnu_overload");
	public static final string_id MNU_SHUTDOWN = new string_id("hq", "mnu_shutdown");
	
	public static final string_id MNU_DEFENSE_STATUS = new string_id("hq", "mnu_defense_status");
	
	public static final string_id MNU_RESET_VULNERABILITY = new string_id("hq", "mnu_reset_vulnerability");
	
	public static final String[] ACCEPTED_DEED_TYPES =
	{
		"turret",
		"mine",
	};
	
	public static final String SCRIPTVAR_COUNTDOWN = "countdownInProgress";
	
	public static final string_id SID_TERMINAL_MANAGEMENT = new string_id("player_structure", "management");
	public static final string_id SID_TERMINAL_MANAGEMENT_STATUS = new string_id("player_structure", "management_status");
	public static final string_id SID_TERMINAL_MANAGEMENT_PAY = new string_id("player_structure", "management_pay");
	public static final string_id SID_TERMINAL_MANAGEMENT_DESTROY = new string_id("player_structure", "permission_destroy");
	
	public static final string_id SID_VULNERABILITY_RESET = new string_id("hq", "vulnerability_reset");
	
	public static final string_id SID_VULNERABILITY_RESET_BLACKOUT_PERIOD = new string_id("hq", "vulnerability_reset_blackout_period");
	public static final string_id SID_VULNERABILITY_RESET_WRONG_CELL = new string_id("hq", "vulnerability_reset_wrong_cell");
	public static final string_id SID_VULNERABILITY_RESET_NO_LONGER_IN_STRUCTURE = new string_id("hq", "vulnerability_reset_no_longer_in_structure");
	public static final string_id SID_VULNERABILITY_RESET_REQUEST_RECEIVED = new string_id("hq", "vulnerability_reset_request_received");
	public static final string_id SID_VULNERABILITY_RESET_NOT_ALIVE = new string_id("hq", "vulnerability_reset_not_alive");
	
	public static final String STRING_FILE_LOC = "faction/faction_hq/faction_hq_response";
	
	public static final string_id SID_NO_STEALTH = new string_id("hq", "no_stealth");
	
	
	public int OnInitialize(obj_id self) throws InterruptedException
	{
		if (hasScript(self, hq.SCRIPT_TERMINAL_DISABLE))
		{
			detachScript(self, hq.SCRIPT_TERMINAL_DISABLE);
		}
		
		return super.OnInitialize(self);
	}
	
	
	public int OnObjectMenuRequest(obj_id self, obj_id player, menu_info mi) throws InterruptedException
	{
		if (stealth.hasInvisibleBuff(player))
		{
			sendSystemMessage(player, SID_NO_STEALTH);
			return super.OnObjectMenuRequest(self, player, mi);
		}
		
		int intState = getState(player, STATE_FEIGN_DEATH);
		
		if (isDead(player) || isIncapacitated(player) || intState > 0)
		{
			return super.OnObjectMenuRequest(self, player, mi);
		}
		
		obj_id structure = player_structure.getStructure(player);
		if (!isIdValid(structure))
		{
			return super.OnObjectMenuRequest(self, player, mi);
		}
		
		String buildingTemplate = getTemplateName(structure);
		
		if (pvpGetAlignedFaction(player) != pvpGetAlignedFaction(structure))
		{
			if (pvpGetType(player) == PVPTYPE_NEUTRAL)
			{
				sendSystemMessage(player, new string_id(STRING_FILE_LOC, "terminal_response01"));
				return super.OnObjectMenuRequest(self, player, mi);
			}
			
			if (hasObjVar(structure, hq.VAR_OBJECTIVE_TRACKING))
			{
				obj_id[] objectives = getObjIdArrayObjVar(structure, hq.VAR_OBJECTIVE_ID);
				if (objectives == null || objectives.length == 0)
				{
					return super.OnObjectMenuRequest(self, player, mi);
				}
				
				obj_id[] disabled = getObjIdArrayObjVar(structure, hq.VAR_OBJECTIVE_DISABLED);
				if (disabled == null || disabled.length != objectives.length)
				{
					prose_package ppDisableOther = prose.getPackage(hq.PROSE_DISABLE_OTHER, objectives[objectives.length - 1], self);
					sendSystemMessageProse(player, ppDisableOther);
					return super.OnObjectMenuRequest(self, player, mi);
				}
				
				int mnuCountdown = mi.addRootMenu(menu_info_types.ITEM_USE, MNU_OVERLOAD);
			}
			
			return super.OnObjectMenuRequest(self, player, mi);
		}
		
		int management_root = mi.addRootMenu (menu_info_types.ITEM_USE, SID_TERMINAL_MANAGEMENT);
		if (management_root > -1)
		{
			mi.addSubMenu(management_root, menu_info_types.SERVER_TERMINAL_MANAGEMENT_STATUS, SID_TERMINAL_MANAGEMENT_STATUS);
			
			if (player_structure.isAdmin(structure, player))
			{
				if (!player_structure.isFactionPerkBase(buildingTemplate))
				{
					mi.addSubMenu(management_root, menu_info_types.SERVER_TERMINAL_MANAGEMENT_PAY, SID_TERMINAL_MANAGEMENT_PAY);
				}
				
				if (hasObjVar(structure,"isPvpBase" ))
				{
					int stamp = getIntObjVar(structure, "lastReset");
					int now = getGameTime();
					if (now > stamp + 1209600 || !hasObjVar(structure, "lastReset"))
					{
						mi.addSubMenu(management_root, menu_info_types.SERVER_MENU6, MNU_RESET_VULNERABILITY);
					}
				}
				
				mi.addSubMenu(management_root, menu_info_types.SERVER_MENU5, MNU_DEFENSE_STATUS);
				mi.addSubMenu(management_root, menu_info_types.SERVER_TERMINAL_MANAGEMENT_DESTROY, SID_TERMINAL_MANAGEMENT_DESTROY);
			}
		}
		
		int mnuDonate = -1;
		
		if (!player_structure.isFactionPerkBase(buildingTemplate))
		{
			mnuDonate = mi.addRootMenu(menu_info_types.SERVER_MENU1, MNU_DONATE);
			int mnuDonateMoney = mi.addSubMenu(mnuDonate, menu_info_types.SERVER_MENU2, MNU_DONATE_MONEY);
		}
		
		if (hasObjVar(structure, hq.VAR_OBJECTIVE_TRACKING) || isGod(player))
		{
			if (mnuDonate <= -1)
			{
				mnuDonate = mi.addRootMenu(menu_info_types.SERVER_MENU1, MNU_DONATE);
			}
			
			int mnuDonateDeed = mi.addSubMenu(mnuDonate, menu_info_types.SERVER_MENU4, MNU_DONATE_DEED);
		}
		
		if (utils.hasScriptVar(self, SCRIPTVAR_COUNTDOWN) && pvpGetType(player) != PVPTYPE_NEUTRAL)
		{
			if (hasObjVar(structure,"isPvpBase" ))
			{
				int mnuShutdown = mi.addRootMenu(menu_info_types.SERVER_MENU9, MNU_SHUTDOWN);
			}
		}
		
		return super.OnObjectMenuRequest(self, player, mi);
	}
	
	
	public int OnObjectMenuSelect(obj_id self, obj_id player, int item) throws InterruptedException
	{
		if (stealth.hasInvisibleBuff(player))
		{
			sendSystemMessage(player, SID_NO_STEALTH);
			return SCRIPT_CONTINUE;
		}
		
		int intState = getState(player, STATE_FEIGN_DEATH);
		
		if (isDead(player) || isIncapacitated(player) || intState > 0)
		{
			return SCRIPT_CONTINUE;
		}
		
		obj_id structure = player_structure.getStructure(player);
		if (!isIdValid(structure))
		{
			return SCRIPT_CONTINUE;
		}
		
		String buildingTemplate = getTemplateName(structure);
		
		if (pvpGetAlignedFaction(player) != pvpGetAlignedFaction(structure))
		{
			if (pvpGetType(player) == PVPTYPE_NEUTRAL)
			{
				sendSystemMessage(player, new string_id(STRING_FILE_LOC, "terminal_response01"));
				return SCRIPT_CONTINUE;
			}
			
			if (item == menu_info_types.ITEM_USE)
			{
				if (hasObjVar(structure, hq.VAR_OBJECTIVE_TRACKING))
				{
					if (utils.hasScriptVar(self, SCRIPTVAR_COUNTDOWN))
					{
						sendSystemMessage(player, new string_id(STRING_FILE_LOC, "terminal_response02"));
						return SCRIPT_CONTINUE;
					}
					
					obj_id[] objectives = getObjIdArrayObjVar(structure, hq.VAR_OBJECTIVE_ID);
					if (objectives == null || objectives.length == 0)
					{
						return SCRIPT_CONTINUE;
					}
					
					obj_id[] disabled = getObjIdArrayObjVar(structure, hq.VAR_OBJECTIVE_DISABLED);
					if (disabled == null || disabled.length != objectives.length)
					{
						prose_package ppDisableOther = prose.getPackage(hq.PROSE_DISABLE_OTHER, objectives[objectives.length - 1], self);
						sendSystemMessageProse(player, ppDisableOther);
						return SCRIPT_CONTINUE;
					}
					
					if (!hasSkill(player, "class_officer_phase1_novice"))
					{
						sendSystemMessage(player, new string_id(STRING_FILE_LOC, "terminal_response03"));
						return SCRIPT_CONTINUE;
					}
					
					startCountdown(self, player);
					
					return SCRIPT_CONTINUE;
				}
			}
			
			return SCRIPT_OVERRIDE;
		}
		
		if (((item == menu_info_types.SERVER_TERMINAL_MANAGEMENT)||(item == menu_info_types.SERVER_TERMINAL_MANAGEMENT_STATUS)|| (item == menu_info_types.ITEM_USE)))
		{
			queueCommand(player, (335013253), null, "", COMMAND_PRIORITY_DEFAULT);
		}
		else if (item == menu_info_types.SERVER_TERMINAL_MANAGEMENT_PAY)
		{
			if (!player_structure.isFactionPerkBase(buildingTemplate))
			{
				queueCommand(player, (-404530384), null, "", COMMAND_PRIORITY_DEFAULT);
			}
		}
		else if (item == menu_info_types.SERVER_TERMINAL_MANAGEMENT_DESTROY)
		{
			queueCommand(player, (419174182), null, "", COMMAND_PRIORITY_DEFAULT);
		}
		
		if (item == menu_info_types.ITEM_USE)
		{
			
			return SCRIPT_CONTINUE;
		}
		
		if (item == menu_info_types.SERVER_MENU1)
		{
			sendSystemMessage(player, new string_id(STRING_FILE_LOC, "terminal_response04"));
			return SCRIPT_CONTINUE;
		}
		else if (item == menu_info_types.SERVER_MENU2)
		{
			if (!player_structure.isFactionPerkBase(buildingTemplate))
			{
				int total = getTotalMoney(player);
				if (total < 1)
				{
					sendSystemMessage(player, new string_id(STRING_FILE_LOC, "terminal_response05"));
					return SCRIPT_CONTINUE;
				}
				else
				{
					
					String title = utils.packStringId(new string_id(STRING_FILE_LOC, "terminal_response18"));
					String prompt = utils.packStringId(new string_id(STRING_FILE_LOC, "terminal_response17"));
					
					sui.transfer(self, player, prompt, title, "@faction/faction_hq/faction_hq_response:terminal_response30", total, "@faction/faction_hq/faction_hq_response:terminal_response31", 0, "handleRequestDonation");
				}
			}
		}
		else if (item == menu_info_types.SERVER_MENU4)
		{
			
			if (hasObjVar(structure, "donateTerminalOff"))
			{
				
				int terminalOff_time = utils.getIntObjVar(structure, "donateTerminalOff");
				int termOff_remaining = 3600 - (getGameTime() - terminalOff_time);
				String timeOff_remaining = player_structure.assembleTimeRemaining(player_structure.convertSecondsTime(termOff_remaining));
				
				prose_package ppTimeRemaining = prose.getPackage(UNDER_ATTACK, timeOff_remaining);
				sendSystemMessageProse(player, ppTimeRemaining);
				return SCRIPT_CONTINUE;
			}
			else
			{
				if (hasObjVar(structure, hq.VAR_OBJECTIVE_TRACKING) || isGod(player))
				{
					if (isGod(player))
					{
						sendSystemMessage(player, "Warning: You are using donation function while in Godmode.", null);
					}
					showDeedDonationUI(self, player);
				}
			}
		}
		else if (item == menu_info_types.SERVER_MENU5)
		{
			if (player_structure.isAdmin(structure, player))
			{
				showDefenseStatusUI(self, player);
			}
		}
		else if (item == menu_info_types.SERVER_MENU6)
		{
			if (player_structure.isAdmin(structure, player))
			{
				if (hasObjVar(structure,"isPvpBase" ))
				{
					int hqBlackoutPeriodStart = utils.getIntConfigSetting("ScriptFlags", "hqBlackoutPeriodStart");
					if (hqBlackoutPeriodStart == 0)
					{
						hqBlackoutPeriodStart = 2;
					}
					
					int hqBlackoutPeriodEnd = utils.getIntConfigSetting("ScriptFlags", "hqBlackoutPeriodEnd");
					if (hqBlackoutPeriodEnd == 0)
					{
						hqBlackoutPeriodEnd = 6;
					}
					
					Calendar currentCalendar = Calendar.getInstance();
					int currentHourOfDay = currentCalendar.get(Calendar.HOUR_OF_DAY);
					if ((currentHourOfDay >= hqBlackoutPeriodStart) && (currentHourOfDay <= hqBlackoutPeriodEnd))
					{
						
						prose_package pp1a = prose.getPackage(new string_id (STRING_FILE_LOC, "terminal_response32"), hqBlackoutPeriodStart);
						prose_package pp2a = prose.getPackage(new string_id (STRING_FILE_LOC, "terminal_response37"), hqBlackoutPeriodEnd);
						
						String oob1 = packOutOfBandProsePackage(null, pp1a);
						oob1 = packOutOfBandProsePackage(oob1, pp2a);
						
						sendSystemMessageOob(player, oob1);
						sendSystemMessageProse(player, prose.getPackage(new string_id (STRING_FILE_LOC, "terminal_response38"), currentHourOfDay));
						
					}
					else
					{
						long now = System.currentTimeMillis();
						long curTime = ((now / 1000) - 1072224000);
						int currentTime = (int) curTime;
						
						setObjVar(structure, hq.VAR_OBJECTIVE_STAMP, currentTime);
						setObjVar(structure, "lastReset", getGameTime());
						sendSystemMessage(player, SID_VULNERABILITY_RESET);
					}
				}
			}
		}
		else if (item == menu_info_types.SERVER_MENU9)
		{
			if (hasObjVar(structure,"isPvpBase" ))
			{
				if (pvpGetType(player) != PVPTYPE_NEUTRAL)
				{
					if (factions.isNewlyDeclared(player))
					{
						int declared_time = utils.getIntScriptVar(player, factions.VAR_NEWLY_DECLARED);
						int time_remaining = factions.NEWLY_DECLARED_INTERVAL - (getGameTime() - declared_time);
						LOG("LOG_CHANNEL", "declared_time ->"+ declared_time + " time_remaining ->"+ time_remaining);
						if (time_remaining > 0)
						{
							String time_str = player_structure.assembleTimeRemaining(player_structure.convertSecondsTime(time_remaining));
							sendSystemMessageProse(player, prose.getPackage(new string_id (STRING_FILE_LOC, "terminal_response41"), time_str));
						}
						else
						{
							utils.removeScriptVar(player, factions.VAR_NEWLY_DECLARED);
							facilityShutdownDelay(self, player);
						}
					}
					else
					{
						
						facilityShutdownDelay(self, player);
						
					}
				}
				else
				{
					sendSystemMessage(player, new string_id(STRING_FILE_LOC, "terminal_response06"));
				}
			}
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public void shutdownFacility(obj_id self) throws InterruptedException
	{
		attachScript(self, hq.SCRIPT_TERMINAL_DISABLE);
		
		obj_id structure = player_structure.getStructure(self);
		if (isIdValid(structure))
		{
			hq.disableHqTerminals(structure);
		}
		
		abortCountdown(self);
		hq.activateHackAlarms(structure, false);
		messageTo(self, "handleFacilityReboot", null, 30f, false);
	}
	
	
	public void startCountdown(obj_id self, obj_id player) throws InterruptedException
	{
		
		int meleemod = getSkillStatMod(player, "group_melee_defense");
		int rangemod = getSkillStatMod(player, "group_range_defense");
		
		float mod = (100f - (float)(meleemod + rangemod))/100f;
		if (mod < 0f)
		{
			mod = 0f;
		}
		
		float delay = 300f + 300f * mod;
		
		int minutes = Math.round(delay/60f);
		
		obj_id[] players = player_structure.getPlayersInBuilding(getTopMostContainer(self));
		if (players != null && players.length > 0)
		{
			for (int i = 0; i < players.length; i++)
			{
				testAbortScript();
				sendSystemMessageProse(players[i], prose.getPackage(new string_id (STRING_FILE_LOC, "terminal_response40"), minutes));
			}
		}
		
		int stamp = getGameTime() + Math.round(delay);
		utils.setScriptVar(self, SCRIPTVAR_COUNTDOWN, stamp);
		
		dictionary d = new dictionary();
		d.put("player", player);
		d.put("cnt", minutes);
		
		messageTo(self, "handleCountdown", d, 10f, false);
	}
	
	
	public void abortCountdown(obj_id self) throws InterruptedException
	{
		obj_id structure = getTopMostContainer(self);
		hq.activateDestructAlarms(structure, false);
		
		utils.removeScriptVar(self, SCRIPTVAR_COUNTDOWN);
		
		obj_id[] players = player_structure.getPlayersInBuilding(structure);
		if (players != null && players.length > 0)
		{
			for (int i = 0; i < players.length; i++)
			{
				testAbortScript();
				sendSystemMessage(players[i], new string_id(STRING_FILE_LOC, "terminal_response07"));
			}
		}
	}
	
	
	public int handleFacilityReboot(obj_id self, dictionary params) throws InterruptedException
	{
		detachScript(self, hq.SCRIPT_TERMINAL_DISABLE);
		
		obj_id structure = player_structure.getStructure(self);
		if (isIdValid(structure))
		{
			hq.enableHqTerminals(structure);
		}
		
		obj_id[] players = player_structure.getPlayersInBuilding(getTopMostContainer(self));
		if (players != null && players.length > 0)
		{
			for (int i = 0; i < players.length; i++)
			{
				testAbortScript();
				sendSystemMessage(players[i], new string_id(STRING_FILE_LOC, "terminal_response08"));
			}
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int handleCountdown(obj_id self, dictionary params) throws InterruptedException
	{
		if (!utils.hasScriptVar(self, SCRIPTVAR_COUNTDOWN))
		{
			return SCRIPT_CONTINUE;
		}
		
		obj_id player = params.getObjId("player");
		
		if (stealth.hasInvisibleBuff(player))
		{
			sendSystemMessage(player, SID_NO_STEALTH);
			return SCRIPT_CONTINUE;
		}
		
		if (!isIdValid(player))
		{
			abortCountdown(self);
			return SCRIPT_CONTINUE;
		}
		
		int cnt = params.getInt("cnt");
		
		int stamp = utils.getIntScriptVar(self, SCRIPTVAR_COUNTDOWN);
		LOG("hqObjective","stamp = "+ stamp);
		int now = getGameTime();
		LOG("hqObjective","now = "+ now);
		
		int timeLeft = stamp - now;
		int minutes = Math.round(timeLeft/60f);
		
		obj_id structure = player_structure.getStructure(self);
		if (!isIdValid(structure))
		{
			abortCountdown(self);
			return SCRIPT_CONTINUE;
		}
		
		LOG("hqObjective","timeLeft = "+ timeLeft);
		if (timeLeft < 1)
		{
			utils.setScriptVar(structure, "faction_hq.detonator", player);
			hq.detonateHq(structure);
			String hqName = getName(structure);
			CustomerServiceLog("faction_hq", "Initiating destroy for Faction HQ "+ hqName + " ("+ structure + "), by normal terminal overload.");
			return SCRIPT_CONTINUE;
		}
		
		prose_package msg = prose.getPackage(new string_id (STRING_FILE_LOC, "terminal_response39"), minutes);
		if (minutes < 1)
		{
			msg = prose.getPackage(new string_id (STRING_FILE_LOC, "terminal_response39"), timeLeft);
		}
		else if (minutes < cnt)
		{
			obj_id[] players = player_structure.getPlayersInBuilding(structure);
			if (players != null && players.length > 0)
			{
				for (int i = 0; i < players.length; i++)
				{
					testAbortScript();
					sendSystemMessageProse(players[i], msg);
				}
			}
			
			params.put("cnt", minutes);
		}
		
		hq.activateDestructAlarms(structure, true);
		messageTo(self, "handleCountdown", params, 10f, false);
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int handleDonateDeed(obj_id self, dictionary params) throws InterruptedException
	{
		obj_id player = sui.getPlayerId(params);
		if (stealth.hasInvisibleBuff(player))
		{
			sendSystemMessage(player, SID_NO_STEALTH);
			return SCRIPT_CONTINUE;
		}
		
		if (!isIdValid(player))
		{
			return SCRIPT_CONTINUE;
		}
		
		String scriptvar_sui = player + ".deed.sui";
		String scriptvar_opt = player + ".deed.opt";
		
		obj_id[] deeds = utils.getObjIdBatchScriptVar(self, scriptvar_opt);
		
		utils.removeScriptVar(self, scriptvar_sui);
		utils.removeBatchScriptVar(self, scriptvar_opt);
		
		if (deeds == null || deeds.length == 0)
		{
			return SCRIPT_CONTINUE;
		}
		
		int bp = sui.getIntButtonPressed(params);
		if (bp == sui.BP_CANCEL)
		{
			return SCRIPT_CONTINUE;
		}
		
		int idx = sui.getListboxSelectedRow(params);
		if (idx == -1 || idx > deeds.length-1)
		{
			return SCRIPT_CONTINUE;
		}
		
		obj_id deed = deeds[idx];
		if (!isIdValid(deed))
		{
			return SCRIPT_CONTINUE;
		}
		
		if (!utils.isNestedWithin(deed, player))
		{
			sendSystemMessage(player, new string_id(STRING_FILE_LOC, "terminal_response09"));
			return SCRIPT_CONTINUE;
		}
		
		String deedType = getStringObjVar(deed, "perkDeedType");
		if (deedType != null && !deedType.equals(""))
		{
			if (utils.getElementPositionInArray(ACCEPTED_DEED_TYPES, deedType) == -1)
			{
				sendSystemMessage(player, new string_id(STRING_FILE_LOC, "terminal_response10"));
				return SCRIPT_CONTINUE;
			}
			else
			{
				
				if (deedType.equals("turret"))
				{
					obj_id objBuilding = getTopMostContainer(self);
					if (isIdValid(objBuilding))
					{
						if (hasObjVar(objBuilding, "isPvpBase"))
						{
							sendSystemMessage(player, new string_id(STRING_FILE_LOC, "terminal_response_special_forces_no_turrets"));
							return SCRIPT_CONTINUE;
						}
					}
					else
					{
						debugServerConsoleMsg(player, "Trying to donate turrets to a Factional HQ terminal without a topmost container");
						return SCRIPT_CONTINUE;
					}
				}
			}
		}
		
		String displayType = utils.packStringId(new string_id("faction_perk", deedType));
		String template = "";
		
		int mineType = -1;
		
		obj_id structure = player_structure.getStructure(player);
		if (!isIdValid(structure))
		{
			return SCRIPT_CONTINUE;
		}
		
		if (deedType.equals("mine"))
		{
			int numMines = hq.getTotalMines(structure);
			
			if (numMines >= hq.getMaxMines(structure))
			{
				sendSystemMessage(player, new string_id(STRING_FILE_LOC, "max_number_mines"));
				return SCRIPT_CONTINUE;
			}
			
			if (hasObjVar(deed, "mineType"))
			{
				mineType = getIntObjVar(deed, "mineType");
			}
		}
		else
		{
			obj_id[] defensesOfType = getObjIdArrayObjVar(structure, hq.VAR_DEFENSE_BASE + "."+ deedType);
			if (defensesOfType == null || defensesOfType.length == 0)
			{
				sendSystemMessageProse(player, prose.getPackage(new string_id (STRING_FILE_LOC, "terminal_response43"), displayType));
				return SCRIPT_CONTINUE;
			}
			
			int pos = utils.getFirstNonValidIdIndex(defensesOfType);
			
			if (pos < 0 || pos > defensesOfType.length - 1)
			{
				sendSystemMessageProse(player, prose.getPackage(new string_id (STRING_FILE_LOC, "terminal_response44"), displayType));
				return SCRIPT_CONTINUE;
			}
			
			template = player_structure.getDeedTemplate(deed);
			if (template == null || template.equals(""))
			{
				sendSystemMessage(player, new string_id(STRING_FILE_LOC, "terminal_response11"));
				return SCRIPT_CONTINUE;
			}
		}
		
		String sFacName = factions.getFaction(structure);
		if (sFacName == null || sFacName.equals(""))
		{
			sendSystemMessageProse(player, prose.getPackage(new string_id (STRING_FILE_LOC, "terminal_response45"), displayType));
			CustomerServiceLog("faction_hq", "%TU donates a "+ displayType + " deed to Factional HQ "+ structure, player, null);
		}
		else
		{
			prose_package pp1b = prose.getPackage(new string_id (STRING_FILE_LOC, "terminal_response46"), displayType);
			prose_package pp2b = prose.getPackage(new string_id (STRING_FILE_LOC, "terminal_response47"), sFacName);
			
			String oob2 = packOutOfBandProsePackage(null, pp1b);
			oob2 = packOutOfBandProsePackage(oob2, pp2b);
			CustomerServiceLog("faction_hq", "%TU donates a "+ displayType + " deed to Factional HQ "+ structure, player, null);
			sendSystemMessageOob(player, oob2);
		}
		destroyObject(deed);
		
		dictionary d = new dictionary();
		d.put("template", template);
		d.put("type", deedType);
		d.put("mineType", mineType);
		
		messageTo(structure, "handleAddDefense", d, 1, true);
		return SCRIPT_CONTINUE;
	}
	
	
	public int handleDonateResourceType(obj_id self, dictionary params) throws InterruptedException
	{
		obj_id player = sui.getPlayerId(params);
		if (stealth.hasInvisibleBuff(player))
		{
			sendSystemMessage(player, SID_NO_STEALTH);
			return SCRIPT_CONTINUE;
		}
		
		if (!isIdValid(player))
		{
			return SCRIPT_CONTINUE;
		}
		
		String scriptvar_sui = player + ".resource.sui";
		String scriptvar_opt = player + ".resource.opt";
		
		obj_id[] crates = utils.getObjIdBatchScriptVar(self, scriptvar_opt);
		
		utils.removeScriptVar(self, scriptvar_sui);
		utils.removeBatchScriptVar(self, scriptvar_opt);
		
		if (crates == null || crates.length == 0)
		{
			return SCRIPT_CONTINUE;
		}
		
		int bp = sui.getIntButtonPressed(params);
		if (bp == sui.BP_CANCEL)
		{
			return SCRIPT_CONTINUE;
		}
		
		int idx = sui.getListboxSelectedRow(params);
		if (idx == -1 || idx > crates.length-1)
		{
			return SCRIPT_CONTINUE;
		}
		
		obj_id crate = crates[idx];
		if (!isIdValid(crate))
		{
			return SCRIPT_CONTINUE;
		}
		
		if (!utils.isNestedWithin(crate, player))
		{
			sendSystemMessage(player, new string_id(STRING_FILE_LOC, "terminal_response12"));
			return SCRIPT_CONTINUE;
		}
		
		int amt = getResourceContainerQuantity(crate);
		
		String title = utils.packStringId(new string_id(STRING_FILE_LOC, "terminal_response20"));
		String prompt = utils.packStringId(new string_id(STRING_FILE_LOC, "terminal_response19"));
		
		int pid = sui.transfer(self, player, prompt, title, "@faction/faction_hq/faction_hq_response:terminal_response36", amt, "@faction/faction_hq/faction_hq_response:terminal_response31", 0, "handleDonateResourceAmt");
		if (pid > -1)
		{
			utils.setScriptVar(self, scriptvar_sui, pid);
			utils.setScriptVar(self, scriptvar_opt, crate);
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int handleDonateResourceAmt(obj_id self, dictionary params) throws InterruptedException
	{
		obj_id player = sui.getPlayerId(params);
		if (stealth.hasInvisibleBuff(player))
		{
			sendSystemMessage(player, SID_NO_STEALTH);
			return SCRIPT_CONTINUE;
		}
		
		if (!isIdValid(player))
		{
			return SCRIPT_CONTINUE;
		}
		
		String scriptvar_sui = player + ".resource.sui";
		String scriptvar_opt = player + ".resource.opt";
		
		obj_id crate = utils.getObjIdScriptVar(self, scriptvar_opt);
		
		utils.removeScriptVar(self, scriptvar_sui);
		utils.removeScriptVar(self, scriptvar_opt);
		
		if (!isIdValid(crate))
		{
			return SCRIPT_CONTINUE;
		}
		
		if (!utils.isNestedWithin(crate, player))
		{
			sendSystemMessage(player, new string_id(STRING_FILE_LOC, "terminal_response12"));
			return SCRIPT_CONTINUE;
		}
		
		int bp = sui.getIntButtonPressed(params);
		if (bp == sui.BP_CANCEL)
		{
			return SCRIPT_CONTINUE;
		}
		
		int amt = sui.getTransferInputTo(params);
		if (amt == 0)
		{
			return SCRIPT_CONTINUE;
		}
		
		obj_id rtype = getResourceContainerResourceType(crate);
		if (!isIdValid(rtype))
		{
			return SCRIPT_CONTINUE;
		}
		
		String res_name = getResourceName(rtype);
		
		if (removeResourceFromContainer(crate, rtype, amt))
		{
			obj_id structure = player_structure.getStructure(player);
			
			String sFacName = factions.getFaction(structure);
			if (sFacName == null || sFacName.equals(""))
			{
				sendSystemMessageProse(player, prose.getPackage(new string_id (STRING_FILE_LOC, "terminal_response48"), amt));
				CustomerServiceLog("faction_hq", "%TU donates "+ amt + " units of "+ res_name + " to Factional HQ "+ structure, player, null);
			}
			else
			{
				prose_package pp1c = prose.getPackage(new string_id (STRING_FILE_LOC, "terminal_response49"), amt);
				prose_package pp2c = prose.getPackage(new string_id (STRING_FILE_LOC, "terminal_response50"), sFacName);
				
				String oob3 = packOutOfBandProsePackage(null, pp1c);
				oob3 = packOutOfBandProsePackage(oob3, pp2c);
				CustomerServiceLog("faction_hq", "%TU donates "+ amt + " units of "+ res_name + " to Factional HQ "+ structure, player, null);
				sendSystemMessageOob(player, oob3);
			}
			
			if (!isIdValid(structure))
			{
				return SCRIPT_CONTINUE;
			}
			
			int total = getIntObjVar(structure, hq.VAR_HQ_RESOURCE_CNT);
			total += amt;
			setObjVar(structure, hq.VAR_HQ_RESOURCE_CNT, total);
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int handleRequestDonation(obj_id self, dictionary params) throws InterruptedException
	{
		int bp = sui.getIntButtonPressed(params);
		if (bp == sui.BP_CANCEL)
		{
			return SCRIPT_CONTINUE;
		}
		
		obj_id player = sui.getPlayerId(params);
		if (stealth.hasInvisibleBuff(player))
		{
			sendSystemMessage(player, SID_NO_STEALTH);
			return SCRIPT_CONTINUE;
		}
		
		if (!isIdValid(player))
		{
			return SCRIPT_CONTINUE;
		}
		
		int amt = sui.getTransferInputTo(params);
		if (amt == 0)
		{
			return SCRIPT_CONTINUE;
		}
		
		money.requestPayment(player, self, amt, "handleCreditDonation", null, true);
		return SCRIPT_CONTINUE;
	}
	
	
	public int handleCreditDonation(obj_id self, dictionary params) throws InterruptedException
	{
		if (params == null || params.isEmpty())
		{
			return SCRIPT_CONTINUE;
		}
		
		int ret = params.getInt(money.DICT_CODE);
		if (ret == money.RET_FAIL)
		{
			return SCRIPT_CONTINUE;
		}
		
		obj_id player = params.getObjId(money.DICT_PLAYER_ID);
		if (!isIdValid(player))
		{
			return SCRIPT_CONTINUE;
		}
		
		if (stealth.hasInvisibleBuff(player))
		{
			sendSystemMessage(player, SID_NO_STEALTH);
			return SCRIPT_CONTINUE;
		}
		
		obj_id structure = player_structure.getStructure(player);
		
		int amt = params.getInt(money.DICT_TOTAL);
		
		String sFacName = factions.getFaction(structure);
		if (sFacName == null || sFacName.equals(""))
		{
			sendSystemMessageProse(player, prose.getPackage(new string_id (STRING_FILE_LOC, "terminal_response51"), amt));
			CustomerServiceLog("faction_hq", "%TU donated "+ amt + " credits to Faction HQ "+ structure, player, null);
		}
		else
		{
			prose_package pp1d = prose.getPackage(new string_id (STRING_FILE_LOC, "terminal_response52"), amt);
			prose_package pp2d = prose.getPackage(new string_id (STRING_FILE_LOC, "terminal_response53"), sFacName);
			String oob4 = packOutOfBandProsePackage(null, pp1d);
			oob4 = packOutOfBandProsePackage(oob4, pp2d);
			sendSystemMessageOob(player, oob4);
			CustomerServiceLog("faction_hq", "%TU donated "+ amt + " credits to Faction HQ "+ structure, player, null);
		}
		
		if (isIdValid(structure))
		{
			money.bankTo(self, structure, amt);
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int handleDefenseSelection(obj_id self, dictionary params) throws InterruptedException
	{
		obj_id player = sui.getPlayerId(params);
		if (!isIdValid(player))
		{
			return SCRIPT_CONTINUE;
		}
		
		if (stealth.hasInvisibleBuff(player))
		{
			sendSystemMessage(player, SID_NO_STEALTH);
			return SCRIPT_CONTINUE;
		}
		
		String scriptvar_sui = player + ".defense.sui";
		String scriptvar_opt = player + ".defense.opt";
		
		obj_id[] opt = utils.getObjIdBatchScriptVar(self, scriptvar_opt);
		
		utils.removeScriptVar(self, scriptvar_sui);
		utils.removeBatchScriptVar(self, scriptvar_opt);
		
		if (opt == null || opt.length == 0)
		{
			return SCRIPT_CONTINUE;
		}
		
		int bp = sui.getIntButtonPressed(params);
		if (bp == sui.BP_CANCEL)
		{
			return SCRIPT_CONTINUE;
		}
		
		int idx = sui.getListboxSelectedRow(params);
		if (idx == -1 || idx > opt.length-1)
		{
			return SCRIPT_CONTINUE;
		}
		
		obj_id defense = opt[idx];
		if (!isIdValid(defense))
		{
			return SCRIPT_CONTINUE;
		}
		
		if (!exists(defense) || !defense.isLoaded())
		{
			sendSystemMessage(player, new string_id(STRING_FILE_LOC, "terminal_response13"));
			return SCRIPT_CONTINUE;
		}
		
		int max_hp = getMaxHitpoints(defense);
		int cur_hp = getHitpoints(defense);
		
		String title = utils.packStringId(new string_id(STRING_FILE_LOC, "terminal_response24"));
		String prompt = getString(new string_id(STRING_FILE_LOC, "terminal_response25"));
		prompt += "/n/n"+ getString(new string_id(STRING_FILE_LOC, "selected_defense")) + utils.getStringName(defense) + " ["+cur_hp+"/"+max_hp+"]";
		
		if (hasScript(defense, "faction_perk.minefield.advanced_minefield"))
		{
			prompt = getString(new string_id(STRING_FILE_LOC, "terminal_response25a"));
		}
		
		int pid = sui.msgbox(self, player, prompt, sui.YES_NO, title, "handleDefenseRemoveConfirm");
		if (pid > -1)
		{
			utils.setScriptVar(self, scriptvar_sui, pid);
			utils.setScriptVar(self, scriptvar_opt, defense);
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int handleDefenseRemoveConfirm(obj_id self, dictionary params) throws InterruptedException
	{
		obj_id player = sui.getPlayerId(params);
		if (!isIdValid(player))
		{
			return SCRIPT_CONTINUE;
		}
		
		if (stealth.hasInvisibleBuff(player))
		{
			sendSystemMessage(player, SID_NO_STEALTH);
			return SCRIPT_CONTINUE;
		}
		
		int bp = sui.getIntButtonPressed(params);
		if (bp == sui.BP_CANCEL)
		{
			return SCRIPT_CONTINUE;
		}
		
		String scriptvar_sui = player + ".defense.sui";
		String scriptvar_opt = player + ".defense.opt";
		
		obj_id defense = utils.getObjIdScriptVar(self, scriptvar_opt);
		
		utils.removeScriptVar(self, scriptvar_sui);
		utils.removeScriptVar(self, scriptvar_opt);
		
		if (!isIdValid(defense) || !exists(defense) || !defense.isLoaded())
		{
			sendSystemMessage(player, new string_id(STRING_FILE_LOC, "terminal_response13"));
			return SCRIPT_CONTINUE;
		}
		
		sendSystemMessage(player, new string_id(STRING_FILE_LOC, "terminal_response58"));
		utils.setScriptVar(defense, "hq.defense.remover", player);
		
		if (hasScript(defense, "faction_perk.minefield.advanced_minefield"))
		{
			hq.clearMinefield(player_structure.getStructure(player));
		}
		else
		{
			destroyObject(defense);
		}
		return SCRIPT_CONTINUE;
	}
	
	
	public void showResourceDonationUI(obj_id terminal, obj_id player) throws InterruptedException
	{
		if (!isIdValid(terminal) || !isIdValid(player))
		{
			return;
		}
		
		if (stealth.hasInvisibleBuff(player))
		{
			sendSystemMessage(player, SID_NO_STEALTH);
			return;
		}
		
		String scriptvar_sui = player + ".resource.sui";
		String scriptvar_opt = player + ".resource.opt";
		
		if (utils.hasScriptVar(terminal, scriptvar_sui))
		{
			sui.closeSUI(player, utils.getIntScriptVar(terminal, scriptvar_sui));
			utils.removeScriptVar(terminal, scriptvar_sui);
			utils.removeBatchScriptVar(terminal, scriptvar_opt);
		}
		
		obj_id[] crates = utils.getContainedGOTObjects(player, GOT_resource_container, true, true);
		if (crates == null || crates.length == 0)
		{
			sendSystemMessage(player, new string_id(STRING_FILE_LOC, "terminal_response14"));
			return;
		}
		
		Vector entries = new Vector();
		entries.setSize(0);
		for (int i = 0; i < crates.length; i++)
		{
			testAbortScript();
			obj_id crate = crates[i];
			if (isIdValid(crate))
			{
				int inCrate = getResourceContainerQuantity(crate);
				String name = getString(getNameStringId(crate));
				
				entries = utils.addElement(entries, name + " ["+ inCrate + "]");
			}
		}
		
		String title = utils.packStringId(new string_id(STRING_FILE_LOC, "terminal_response20"));
		String prompt = utils.packStringId(new string_id(STRING_FILE_LOC, "terminal_response29"));
		
		int pid = sui.listbox(terminal, player, prompt, sui.OK_CANCEL, title, entries, "handleDonateResourceType");
		if (pid > -1)
		{
			utils.setScriptVar(terminal, scriptvar_sui, pid);
			utils.setBatchScriptVar(terminal, scriptvar_opt, crates);
		}
	}
	
	
	public void showDeedDonationUI(obj_id terminal, obj_id player) throws InterruptedException
	{
		if (!isIdValid(terminal) || !isIdValid(player))
		{
			return;
		}
		
		if (stealth.hasInvisibleBuff(player))
		{
			sendSystemMessage(player, SID_NO_STEALTH);
			return;
		}
		
		LOG("hq"," //***// showDeedDonationUI entered... passed in terminal:"+terminal+" and player:"+player);
		
		String scriptvar_sui = player + ".deed.sui";
		String scriptvar_opt = player + ".deed.opt";
		
		LOG("hq"," //***// showDeedDonationUI ... string scriptvar_sui ="+scriptvar_sui+" and string scriptvar_opt ="+scriptvar_opt);
		
		if (utils.hasScriptVar(terminal, scriptvar_sui))
		{
			LOG("hq"," //***// showDeedDonationUI ... utils.hasScriptVar(terminal, scriptvar_sui) is AFFIRM");
			
			sui.closeSUI(player, utils.getIntScriptVar(terminal, scriptvar_sui));
			utils.removeScriptVar(terminal, scriptvar_sui);
			utils.removeScriptVar(terminal, scriptvar_opt);
		}
		
		obj_id[] deeds = utils.getContainedObjectsWithObjVar(player, "perkDeedType", true);
		if (deeds == null || deeds.length == 0)
		{
			sendSystemMessage(player, new string_id(STRING_FILE_LOC, "terminal_response15"));
			return;
		}
		
		Vector entries = new Vector();
		entries.setSize(0);
		Vector opt = new Vector();
		opt.setSize(0);
		for (int i = 0; i < deeds.length; i++)
		{
			testAbortScript();
			LOG("hq"," //***// showDeedDonationUI ... counting through GOT_deed objects in player inventory. Currently on item# "+i);
			
			obj_id deed = deeds[i];
			if (isIdValid(deed))
			{
				String deedType = getStringObjVar(deed, "perkDeedType");
				if (deedType != null && !deedType.equals(""))
				{
					LOG("hq"," //***// showDeedDonationUI ... deed item# "+i+" has 'perkDeedType' objvar of: "+deedType);
					if (utils.getElementPositionInArray(ACCEPTED_DEED_TYPES, deedType) > -1)
					{
						opt = utils.addElement(opt, deed);
						entries = utils.addElement(entries, getEncodedName(deed));
						
					}
				}
			}
		}
		
		String title = utils.packStringId(new string_id(STRING_FILE_LOC, "terminal_response26"));
		String prompt = utils.packStringId (new string_id(STRING_FILE_LOC, "terminal_response23"));
		
		LOG("hq"," //***// showDeedDonationUI ... title= "+title+" and prompt = "+prompt+" size of 'entries' array is:"+entries.size());
		
		int pid = sui.listbox(terminal, player, prompt, sui.OK_CANCEL, title, entries, "handleDonateDeed");
		if (pid > -1)
		{
			utils.setScriptVar(terminal, scriptvar_sui, pid);
			utils.setBatchScriptVar(terminal, scriptvar_opt, opt);
		}
	}
	
	
	public void showDefenseStatusUI(obj_id terminal, obj_id player) throws InterruptedException
	{
		if (!isIdValid(terminal) || !isIdValid(player))
		{
			return;
		}
		
		if (stealth.hasInvisibleBuff(player))
		{
			sendSystemMessage(player, SID_NO_STEALTH);
			return;
		}
		
		String scriptvar_sui = player + ".defense.sui";
		String scriptvar_opt = player + ".defense.opt";
		
		if (utils.hasScriptVar(terminal, scriptvar_sui))
		{
			sui.closeSUI(player, utils.getIntScriptVar(terminal, scriptvar_sui));
			utils.removeScriptVar(terminal, scriptvar_sui);
			utils.removeScriptVar(terminal, scriptvar_opt);
		}
		
		obj_id structure = player_structure.getStructure(player);
		if (!isIdValid(structure))
		{
			return;
		}
		
		Vector entries = new Vector();
		entries.setSize(0);
		Vector opt = new Vector();
		opt.setSize(0);
		obj_var_list ovl = getObjVarList(structure, hq.VAR_DEFENSE_BASE);
		if (ovl == null)
		{
			sendSystemMessage(player, new string_id(STRING_FILE_LOC, "terminal_response14"));
			return;
		}
		
		int numTypes = ovl.getNumItems();
		for (int i = 0; i < numTypes; i++)
		{
			testAbortScript();
			obj_var ov = ovl.getObjVar(i);
			String ovName = ov.getName();
			entries = utils.addElement(entries, toUpper(ovName));
			opt = utils.addElement(opt, obj_id.NULL_ID);
			
			obj_id[] data = ov.getObjIdArrayData();
			if (data != null && data.length > 0)
			{
				int validCount = 0;
				for (int n = 0; n < data.length; n++)
				{
					testAbortScript();
					if (isIdValid(data[n]))
					{
						String sName = utils.getStringName(data[n]);
						int max_hp = getMaxHitpoints(data[n]);
						int cur_hp = getHitpoints(data[n]);
						
						if (hasScript(data[n], "faction_perk.minefield.advanced_minefield"))
						{
							entries = utils.addElement(entries, "- "+ hq.getTotalMines(structure) + "/"+ hq.getMaxMines(structure) + " mines");
						}
						else
						{
							entries = utils.addElement(entries, "- "+ sName + " ["+cur_hp+"/"+max_hp+"]");
						}
						
						opt = utils.addElement(opt, data[n]);
						validCount++;
					}
				}
				
				if (validCount == 0)
				{
					entries = utils.addElement(entries, "- none");
					opt = utils.addElement(opt, obj_id.NULL_ID);
				}
				
			}
		}
		
		String title = utils.packStringId(new string_id(STRING_FILE_LOC, "terminal_response23"));
		String prompt = utils.packStringId (new string_id(STRING_FILE_LOC, "terminal_response22"));
		
		int pid = sui.listbox(terminal, player, prompt, sui.REMOVE_CANCEL, title, entries, "handleDefenseSelection");
		if (pid > -1)
		{
			utils.setScriptVar(terminal, scriptvar_sui, pid);
			utils.setBatchScriptVar(terminal, scriptvar_opt, opt);
		}
	}
	
	
	public void facilityShutdownDelay(obj_id terminal, obj_id player) throws InterruptedException
	{
		if (!isIdValid(terminal) || !isIdValid(player))
		{
			return;
		}
		if (stealth.hasInvisibleBuff(player))
		{
			sendSystemMessage(player, SID_NO_STEALTH);
			return;
		}
		
		String currentCell = null;
		location locTest = getLocation(player);
		if (isIdValid(locTest.cell))
		{
			currentCell = getCellName(locTest.cell);
		}
		
		sendSystemMessage(player, SID_VULNERABILITY_RESET_REQUEST_RECEIVED);
		
		dictionary params = new dictionary();
		params.put("player", player);
		
		if (!currentCell.equals("null"))
		{
			params.put("cellName", currentCell);
		}
		
		messageTo(terminal, "handleShutdownCountdownComplete", params, 60f, false);
		
		return;
	}
	
	
	public int handleShutdownCountdownComplete(obj_id self, dictionary params) throws InterruptedException
	{
		obj_id player = params.getObjId("player");
		String triggeringCell = params.getString("cellName");
		
		if (!isIdValid(player) || !isIdValid(self))
		{
			return SCRIPT_CONTINUE;
		}
		
		if (stealth.hasInvisibleBuff(player))
		{
			sendSystemMessage(player, SID_NO_STEALTH);
			return SCRIPT_CONTINUE;
		}
		
		obj_id structure = player_structure.getStructure(self);
		if (!isIdValid(structure))
		{
			return SCRIPT_CONTINUE;
		}
		
		if (hasObjVar(structure, hq.VAR_TERMINAL_DISABLE))
		{
			boolean structureState = getBooleanObjVar(structure, hq.VAR_TERMINAL_DISABLE);
			if (structureState == true)
			{
				return SCRIPT_CONTINUE;
			}
		}
		
		String currentCell = null;
		location locTest = getLocation(player);
		if (isIdValid(locTest.cell))
		{
			currentCell = getCellName(locTest.cell);
		}
		
		if ((!isIdValid(locTest.cell)) || (!currentCell.equals(triggeringCell)))
		{
			sendSystemMessage(player, SID_VULNERABILITY_RESET_WRONG_CELL);
			return SCRIPT_CONTINUE;
		}
		
		if (isIncapacitated(player) || isDead(player))
		{
			sendSystemMessage(player, SID_VULNERABILITY_RESET_NOT_ALIVE);
			return SCRIPT_CONTINUE;
		}
		
		String title = utils.packStringId(new string_id(STRING_FILE_LOC, "terminal_response27"));
		String prompt = utils.packStringId(new string_id(STRING_FILE_LOC, "terminal_response28"));
		
		int pid = sui.msgbox(self, player, title, sui.YES_NO, prompt, "handleShutdownConfirm");
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int handleShutdownConfirm(obj_id self, dictionary params) throws InterruptedException
	{
		obj_id player = sui.getPlayerId(params);
		if (!isIdValid(player))
		{
			return SCRIPT_CONTINUE;
		}
		
		if (stealth.hasInvisibleBuff(player))
		{
			sendSystemMessage(player, SID_NO_STEALTH);
			return SCRIPT_CONTINUE;
		}
		
		int bp = sui.getIntButtonPressed(params);
		if (bp == sui.BP_CANCEL)
		{
			return SCRIPT_CONTINUE;
		}
		
		obj_id structure = player_structure.getStructure(self);
		
		if (hasObjVar(structure, hq.VAR_TERMINAL_DISABLE))
		{
			boolean structureState = getBooleanObjVar(structure, hq.VAR_TERMINAL_DISABLE);
			if (structureState == true)
			{
				return SCRIPT_CONTINUE;
			}
		}
		
		obj_id currentStructure = player_structure.getStructure(player);
		
		if (currentStructure != structure)
		{
			sendSystemMessage(player, SID_VULNERABILITY_RESET_NO_LONGER_IN_STRUCTURE);
			return SCRIPT_CONTINUE;
		}
		
		shutdownFacility(self);
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int handleAlarmMute(obj_id self, dictionary params) throws InterruptedException
	{
		obj_id structure = player_structure.getStructure(self);
		if (isIdValid(structure))
		{
			hq.activateHackAlarms(structure, false);
		}
		
		return SCRIPT_CONTINUE;
	}
}
