package script.player;

import script.*;
import script.base_class.*;
import script.combat_engine.*;
import java.util.Arrays;
import java.util.Hashtable;
import java.util.Vector;
import script.base_script;


import script.library.colors_hex;
import script.library.factions;
import script.library.gcw;
import script.library.instance;
import script.library.prose;
import script.library.pvp;
import script.library.regions;
import script.library.sui;
import script.library.utils;
import script.library.ai_lib;


public class player_faction extends script.base_script
{
	public player_faction()
	{
	}
	public static final string_id SID_ABORT_RESIGNATION = new string_id("faction_recruiter", "abort_resignation");
	public static final string_id SID_SUI_RESIG_COMPLETE_IN_5 = new string_id("faction_recruiter", "sui_resig_complete_in_5");
	
	public static final string_id SID_NOT_ALIGNED = new string_id("faction_recruiter", "not_aligned");
	public static final string_id SID_PVP_STATUS_CHANGING = new string_id("faction_recruiter", "pvp_status_changing");
	public static final string_id SID_ON_LEAVE_TO_COVERT = new string_id("faction_recruiter", "on_leave_to_covert");
	public static final string_id SID_COVERT_TO_OVERT = new string_id("faction_recruiter", "covert_to_overt");
	public static final string_id SID_OVERT_TO_COVERT = new string_id("faction_recruiter", "overt_to_covert");
	public static final string_id SID_DUNGEON_NOCHANGE = new string_id("faction_recruiter", "dungeon_nochange");
	
	public static final String COLOR_REBELS = "\\"+ colors_hex.COLOR_REBELS;
	public static final String COLOR_IMPERIALS = "\\"+ colors_hex.COLOR_IMPERIALS;
	
	
	public int cmdPVP(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (factions.isInAdhocPvpArea(self))
		{
			pvpMakeDeclared(self);
			return SCRIPT_CONTINUE;
		}
		
		String recruiter = "";
		
		String planet = getCurrentSceneName();
		
		if ((planet == null) || (planet.equals("")))
		{
			return SCRIPT_OVERRIDE;
		}
		else if (planet.equals("dungeon1"))
		{
			sendSystemMessage(self, SID_DUNGEON_NOCHANGE);
			return SCRIPT_OVERRIDE;
		}
		
		if (factions.isRebel(self))
		{
			recruiter = "object/mobile/dressed_rebel_major_human_male_01.iff";
		}
		else if (factions.isImperial(self))
		{
			recruiter = "object/mobile/dressed_imperial_officer_m.iff";
		}
		else
		{
			sendSystemMessage(self, SID_NOT_ALIGNED);
			return SCRIPT_OVERRIDE;
		}
		
		if (factions.isPVPStatusChanging(self))
		{
			sendSystemMessage(self, SID_PVP_STATUS_CHANGING);
			return SCRIPT_OVERRIDE;
		}
		
		prose_package pp = new prose_package();
		
		if (factions.isOnLeave(self))
		{
			pp = prose.setStringId(pp, SID_ON_LEAVE_TO_COVERT);
			commPlayer(self, self, pp, recruiter);
			
			factions.goCovertWithDelay(self, 1f);
			return SCRIPT_CONTINUE;
		}
		
		if (pvpGetType(self) == PVPTYPE_DECLARED)
		{
			pp = prose.setStringId(pp, SID_OVERT_TO_COVERT);
			commPlayer(self, self, pp, recruiter);
			
			factions.goCovertWithDelay(self, 300f);
			return SCRIPT_CONTINUE;
		}
		
		if (pvpGetType(self) == PVPTYPE_COVERT)
		{
			pp = prose.setStringId(pp, SID_COVERT_TO_OVERT);
			commPlayer(self, self, pp, recruiter);
			
			factions.goOvertWithDelay(self, 30f);
			return SCRIPT_CONTINUE;
		}
		
		return SCRIPT_OVERRIDE;
	}
	
	
	public int OnPvpTypeChanged(obj_id self, int oldType, int newType) throws InterruptedException
	{
		
		if (newType == PVPTYPE_DECLARED)
		{
			utils.setScriptVar(self, factions.VAR_NEWLY_DECLARED, getGameTime());
			messageTo(self, "msgNewlyDeclared", null, factions.NEWLY_DECLARED_INTERVAL, false);
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int OnPvpFactionChanged(obj_id self, int oldFaction, int newFaction) throws InterruptedException
	{
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int OnEnterRegion(obj_id self, String planet, String name) throws InterruptedException
	{
		if (name.startsWith("gcw_") && name.endsWith("_start"))
		{
			location baseLoc = new location();
			baseLoc.area = planet;
			
			String waypointName = "";
			
			if (hasObjVar(self, "gcw.static_base.waypoint." + planet))
			{
				obj_id waypoint = getObjIdObjVar(self, "gcw.static_base.waypoint."+ planet);
				
				if (exists(waypoint))
				{
					return SCRIPT_CONTINUE;
				}
			}
			
			if (planet.equals("talus"))
			{
				baseLoc.x = -4938f;
				baseLoc.y = 0f;
				baseLoc.z = -3107f;
				waypointName = "Weapons Depot";
			}
			else if (planet.equals("corellia"))
			{
				baseLoc.x = 4772f;
				baseLoc.y = 0f;
				baseLoc.z = -5233f;
				waypointName = "Tactical Training Facility";
			}
			else if (planet.equals("naboo"))
			{
				baseLoc.x = 1019f;
				baseLoc.y = 0f;
				baseLoc.z = -1508f;
				waypointName = "Weapons Development Facility";
			}
			else
			{
				return SCRIPT_CONTINUE;
			}
			
			obj_id waypoint = createWaypointInDatapad(self, baseLoc);
			
			if (isIdValid(waypoint))
			{
				setWaypointName(waypoint, waypointName);
				setWaypointColor(waypoint, "white");
				setWaypointActive(waypoint, true);
				setWaypointVisible(waypoint, true);
				
				setObjVar(self, "gcw.static_base.waypoint."+ planet, waypoint);
			}
			
			return SCRIPT_CONTINUE;
		}
		
		region enteredRegion = getRegion(planet, name);
		if (enteredRegion != null && enteredRegion.getPvPType() == regions.PVP_REGION_TYPE_ADVANCED)
		{
			if (hasObjVar(self, "gcw.static_base.waypoint." + planet))
			{
				obj_id waypoint = getObjIdObjVar(self, "gcw.static_base.waypoint."+ planet);
				destroyWaypointInDatapad(waypoint, self);
				
				removeObjVar(self, "gcw.static_base.waypoint."+ planet);
			}
			
			return SCRIPT_CONTINUE;
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int OnEnteredCombat(obj_id self) throws InterruptedException
	{
		pvp.validatePlayerDamageTracking(self);
		return SCRIPT_CONTINUE;
	}
	
	
	public int OnCreatureDamaged(obj_id self, obj_id attacker, obj_id wpn, int[] damage) throws InterruptedException
	{
		pvp.updatePlayerDamageTracking(self, attacker, wpn, damage);
		return SCRIPT_CONTINUE;
	}
	
	
	public int handleCloneRespawn(obj_id self, dictionary params) throws InterruptedException
	{
		if (utils.hasScriptVar(self, factions.IN_ADHOC_PVP_AREA) && !getLocation(self).area.equals("adventure2"))
		{
			utils.removeScriptVar(self, factions.IN_ADHOC_PVP_AREA);
		}
		
		if (!hasScript (self, "theme_park.dungeon.corvette.timer") && !instance.isInInstanceArea(self))
		{
			if ((pvpGetType(self)==PVPTYPE_DECLARED)||(pvpGetType(self)==PVPTYPE_COVERT))
			{
				factions.goOnLeaveWithDelay(self, 0f);
			}
		}
		pvp.clearPlayerDamageTracking(self);
		return SCRIPT_CONTINUE;
	}
	
	
	public int msgResignFromFaction(obj_id self, dictionary params) throws InterruptedException
	{
		if (hasSkill(self, "force_rank_light_novice") || hasSkill(self, "force_rank_dark_novice"))
		{
			sendSystemMessage(self, new string_id("faction_recruiter", "jedi_cant_resign"));
			return SCRIPT_CONTINUE;
		}
		factions.resignFromFaction(self);
		return SCRIPT_CONTINUE;
	}
	
	
	public int msgGoCovert(obj_id self, dictionary params) throws InterruptedException
	{
		obj_id top = getTopMostContainer (self);
		if (isIdValid(top))
		{
			String bldgName = getTemplateName (top);
			if (bldgName.equals("object/building/general/space_dungeon_corellian_corvette.iff") || bldgName.equals("object/building/general/space_dungeon_corellian_corvette_imperial.iff") || bldgName.equals("object/building/general/space_dungeon_corellian_corvette_rebel.iff"))
			{
				setObjVar (self, "corl_corvette.made_overt", 1);
				removeObjVar(self, "intChangingFactionStatus");
				
				return SCRIPT_CONTINUE;
			}
		}
		if (hasScript(self, "force_rank_light_novice") || hasScript(self, "force_rank_dark_novice"))
		{
			sendSystemMessage(self, new string_id("faction_recruiter", "jedi_cant_go_covert"));
			removeObjVar(self, "intChangingFactionStatus");
			
			return SCRIPT_CONTINUE;
		}
		
		region[] pvpRegions = getRegionsWithPvPAtPoint(getLocation(self), regions.PVP_REGION_TYPE_ADVANCED);
		
		if (pvpRegions != null && pvpRegions.length > 0)
		{
			removeObjVar(self, "intChangingFactionStatus");
			
			sendSystemMessage(self, new string_id("gcw", "pvp_advanced_region_cannot_go_covert"));
			return SCRIPT_CONTINUE;
		}
		
		factions.goCovert(self);
		return SCRIPT_CONTINUE;
	}
	
	
	public int msgGoOnLeave(obj_id self, dictionary params) throws InterruptedException
	{
		if (hasScript(self, "force_rank_light_novice") || hasScript(self, "force_rank_dark_novice"))
		{
			sendSystemMessage(self, new string_id("faction_recruiter", "jedi_cant_go_covert"));
			return SCRIPT_CONTINUE;
		}
		
		factions.goOnLeave(self);
		return SCRIPT_CONTINUE;
		
	}
	
	
	public int msgGoOvert(obj_id self, dictionary params) throws InterruptedException
	{
		if (hasScript(self, "force_rank_light_novice") || hasScript(self, "force_rank_dark_novice"))
		{
			sendSystemMessage(self, new string_id("faction_recruiter", "jedi_cant_go_covert"));
			return SCRIPT_CONTINUE;
		}
		
		int intFaction = pvpGetAlignedFaction(self);
		if (intFaction == 0)
		{
			sendSystemMessage(self, new string_id("spam", "no_spec_force_neutral") );
			removeObjVar(self, "intChangingFactionStatus");
			return SCRIPT_CONTINUE;
		}
		
		factions.goOvert(self);
		return SCRIPT_CONTINUE;
	}
	
	
	public int gcwStatus(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		
		string_id strSpam = new string_id("gcw", "gcw_status_info");
		sendSystemMessage(self, strSpam);
		return SCRIPT_CONTINUE;
		
	}
	
	
	public int cmdDelegateFactionPoints(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		
		factions.delegateFactionPoints(self, target, params);
		return SCRIPT_CONTINUE;
	}
	
	
	public int handleDelegateSui(obj_id self, dictionary params) throws InterruptedException
	{
		obj_id target = utils.getObjIdScriptVar(self, "delegate.target");
		utils.removeScriptVarTree(self, "delegate");
		if (!isIdValid(target))
		{
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
		
		queueCommand(self, (252954210), target, Integer.toString(amt), COMMAND_PRIORITY_DEFAULT);
		return SCRIPT_CONTINUE;
	}
	
	
	public int handleLeaveFactionSui(obj_id self, dictionary params) throws InterruptedException
	{
		LOG("gcw", "got mst");
		obj_id player = sui.getPlayerId(params);
		if (!isIdValid(player))
		{
			return SCRIPT_CONTINUE;
		}
		
		int bp = sui.getIntButtonPressed(params);
		String txt = sui.getInputBoxText(params);
		if (bp == sui.BP_CANCEL || txt == null || !toLower(txt).equals("yes"))
		{
			removeObjVar(player, factions.VAR_RESIGNING);
			sendSystemMessage(player, SID_ABORT_RESIGNATION);
			return SCRIPT_CONTINUE;
		}
		
		if (hasSkill(player, "force_rank_light_novice") || hasSkill(player, "force_rank_dark_novice"))
		{
			sendSystemMessage(player, new string_id("faction_recruiter", "jedi_cant_resign"));
			return SCRIPT_CONTINUE;
		}
		
		sui.msgbox(player, utils.packStringId(SID_SUI_RESIG_COMPLETE_IN_5));
		setObjVar(self, "intChangingFactionStatus", 1);
		
		params.put("faction_id", pvpGetAlignedFaction(self));
		messageTo(player, "msgResignFromFaction", params, factions.RESIGN_TIME, true);
		CustomerServiceLog( "player_faction", "PLAYER-FACTION ALTERED|TIME:"+getGameTime()+"|PLAYER:"+player+"|PLAYER NAME:"+getName(player)+"|ZONE:"+getCurrentSceneName()+"|PLAYER CONFIRMED DESIRE TO LEAVE THEIR FACTION VIA RECRUITER. TOLD PLAYER OBJECT TO RESIGN FROM FACTION");
		return SCRIPT_CONTINUE;
	}
	
	
	public int handleGoCovert(obj_id self, dictionary params) throws InterruptedException
	{
		
		if (hasObjVar(self, "intChangingFactionStatus"))
		{
			LOG("gcw", "objvar");
			return SCRIPT_CONTINUE;
		}
		
		if (pvpGetType(self)==PVPTYPE_DECLARED)
		{
			string_id strSpam = new string_id("gcw", "cannot_change_from_combatant_in_field");
			sendSystemMessage(self, strSpam);
			return SCRIPT_CONTINUE;
		}
		obj_id player = sui.getPlayerId(params);
		if (!isIdValid(player))
		{
			return SCRIPT_CONTINUE;
		}
		
		int bp = sui.getIntButtonPressed(params);
		String txt = sui.getInputBoxText(params);
		if (bp == sui.BP_CANCEL || txt == null || !toLower(txt).equals("yes"))
		{
			string_id strSpam = new string_id("gcw", "abort_field_change");
			sendSystemMessage(self, strSpam);
			return SCRIPT_CONTINUE;
		}
		
		if (hasSkill(player, "force_rank_light_novice") || hasSkill(player, "force_rank_dark_novice"))
		{
			sendSystemMessage(player, new string_id("faction_recruiter", "jedi_cant_go_covert"));
			return SCRIPT_CONTINUE;
		}
		string_id strSpam = new string_id("gcw", "handle_go_covert");
		CustomerServiceLog( "player_faction", "PLAYER-FACTION ALTERED|TIME:"+getGameTime()+"|PLAYER:"+self+"|PLAYER NAME:"+getName(self)+"|ZONE:"+getCurrentSceneName()+"|Player confirmed desire to change in-field faction status to covert. This will apply in 30 seconds");
		sui.msgbox(player, utils.packStringId(strSpam));
		setObjVar(self, "intChangingFactionStatus", 1);
		factions.goCovertWithDelay(self, 30);
		return SCRIPT_CONTINUE;
	}
	
	
	public int handleGoOvert(obj_id self, dictionary params) throws InterruptedException
	{
		
		if (hasObjVar(self, "intChangingFactionStatus"))
		{
			LOG("gcw", "objvar");
			return SCRIPT_CONTINUE;
		}
		obj_id player = sui.getPlayerId(params);
		if (!isIdValid(player))
		{
			return SCRIPT_CONTINUE;
		}
		
		int bp = sui.getIntButtonPressed(params);
		String txt = sui.getInputBoxText(params);
		if (bp == sui.BP_CANCEL || txt == null || !toLower(txt).equals("yes"))
		{
			string_id strSpam = new string_id("gcw", "abort_field_change");
			sendSystemMessage(self, strSpam);
			return SCRIPT_CONTINUE;
		}
		
		if (hasSkill(player, "force_rank_light_novice") || hasSkill(player, "force_rank_dark_novice"))
		{
			sendSystemMessage(player, new string_id("faction_recruiter", "jedi_cant_go_covert"));
			return SCRIPT_CONTINUE;
		}
		string_id strSpam = new string_id("gcw", "handle_go_covert");
		CustomerServiceLog( "player_faction", "PLAYER-FACTION ALTERED|TIME:"+getGameTime()+"|PLAYER:"+self+"|PLAYER NAME:"+getName(self)+"|ZONE:"+getCurrentSceneName()+"|Player confirmed desire to change in-field faction status to overt. This will apply in 300 seconds.");
		sui.msgbox(player, utils.packStringId(strSpam));
		setObjVar(self, "intChangingFactionStatus", 1);
		factions.goOvertWithDelay(self, 300);
		return SCRIPT_CONTINUE;
	}
	
	
	public int recievePvpRegionBonus(obj_id self, dictionary params) throws InterruptedException
	{
		if (!utils.hasScriptVar(self, gcw.PVP_REGION_ACTIVITY_PERFORMED))
		{
			return SCRIPT_CONTINUE;
		}
		else
		{
			utils.removeScriptVar(self, gcw.PVP_REGION_ACTIVITY_PERFORMED);
		}
		
		int points = params.getInt("points");
		String information = params.getString("info");
		gcw.grantModifiedGcwPoints(self, points, gcw.GCW_POINT_TYPE_GROUND_PVP_REGION, information);
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int cmdFactionalHelper(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		factions.neutralMercenaryStatusMenu(self);
		return SCRIPT_CONTINUE;
	}
	
	
	public int handleFactionalHelperChoice(obj_id self, dictionary params) throws InterruptedException
	{
		if (!utils.hasScriptVar(self, factions.SCRIPTVAR_FACTIONAL_HELPER_SUI_ID))
		{
			return SCRIPT_CONTINUE;
		}
		
		int pid = utils.getIntScriptVar(self, factions.SCRIPTVAR_FACTIONAL_HELPER_SUI_ID);
		utils.removeScriptVar(self, factions.SCRIPTVAR_FACTIONAL_HELPER_SUI_ID);
		
		if (!utils.hasScriptVar(self, factions.SCRIPTVAR_FACTIONAL_HELPER_SUI_CHOICES))
		{
			return SCRIPT_CONTINUE;
		}
		
		String[] choices = utils.getStringArrayScriptVar(self, factions.SCRIPTVAR_FACTIONAL_HELPER_SUI_CHOICES);
		utils.removeScriptVar(self, factions.SCRIPTVAR_FACTIONAL_HELPER_SUI_CHOICES);
		
		if (choices == null)
		{
			return SCRIPT_CONTINUE;
		}
		
		if (pid != params.getInt("pageId"))
		{
			return SCRIPT_CONTINUE;
		}
		
		if (0 != pvpGetAlignedFaction(self))
		{
			sendSystemMessage(self, "You must be a Civilian to be a factional helper.", "");
			return SCRIPT_CONTINUE;
		}
		
		if (ai_lib.isInCombat(self))
		{
			sendSystemMessage(self, "You cannot change your factional helper status while in combat.", "");
			return SCRIPT_CONTINUE;
		}
		
		int bp = sui.getIntButtonPressed(params);
		if (bp == sui.BP_OK)
		{
			int rowSelected = sui.getListboxSelectedRow(params);
			if ((rowSelected >= 0) && (rowSelected < choices.length))
			{
				if (choices[rowSelected].equals(factions.MERC_END_COVERT_IMPERIAL))
				{
					sendSystemMessage(self, factions.SID_MERC_IMPERIAL_COMBATANT_END);
				}
				else if (choices[rowSelected].equals(factions.MERC_END_OVERT_IMPERIAL))
				{
					sendSystemMessage(self, factions.SID_MERC_IMPERIAL_SF_END);
				}
				else if (choices[rowSelected].equals(factions.MERC_END_COVERT_REBEL))
				{
					sendSystemMessage(self, factions.SID_MERC_REBEL_COMBATANT_END);
				}
				else if (choices[rowSelected].equals(factions.MERC_END_OVERT_REBEL))
				{
					sendSystemMessage(self, factions.SID_MERC_REBEL_SF_END);
				}
				else if (choices[rowSelected].equals(factions.MERC_BEGIN_COVERT_IMPERIAL))
				{
					sendSystemMessage(self, factions.SID_MERC_IMPERIAL_COMBATANT_BEGIN);
				}
				else if (choices[rowSelected].equals(factions.MERC_BEGIN_OVERT_IMPERIAL))
				{
					sendSystemMessage(self, factions.SID_MERC_IMPERIAL_SF_BEGIN);
				}
				else if (choices[rowSelected].equals(factions.MERC_BEGIN_COVERT_REBEL))
				{
					sendSystemMessage(self, factions.SID_MERC_REBEL_COMBATANT_BEGIN);
				}
				else if (choices[rowSelected].equals(factions.MERC_BEGIN_OVERT_REBEL))
				{
					sendSystemMessage(self, factions.SID_MERC_REBEL_SF_BEGIN);
				}
				else
				{
					return SCRIPT_CONTINUE;
				}
				
				dictionary messageToParams = new dictionary();
				messageToParams.put("mercenary_type", choices[rowSelected]);
				
				if (isGod(self))
				{
					sendSystemMessage(self, "Reducing wait to 5 seconds ***BECAUSE YOU ARE IN GOD MODE***.", "");
					messageTo(self, "executeFactionalHelperChoice", messageToParams, 5.0f, false);
				}
				else
				{
					messageTo(self, "executeFactionalHelperChoice", messageToParams, 60.0f, false);
				}
			}
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int executeFactionalHelperChoice(obj_id self, dictionary params) throws InterruptedException
	{
		
		if (0 != pvpGetAlignedFaction(self))
		{
			sendSystemMessage(self, "You must be a Civilian to be a factional helper.", "");
			return SCRIPT_CONTINUE;
		}
		
		if (ai_lib.isInCombat(self))
		{
			sendSystemMessage(self, "You cannot change your factional helper status while in combat.", "");
			return SCRIPT_CONTINUE;
		}
		
		String mercenary_type = params.getString("mercenary_type");
		if (mercenary_type == null)
		{
			return SCRIPT_CONTINUE;
		}
		
		if (mercenary_type.equals(factions.MERC_END_COVERT_IMPERIAL))
		{
			pvpNeutralSetMercenaryFaction(self, 0, false);
			sendSystemMessage(self, factions.SID_MERC_IMPERIAL_COMBATANT_TERMINATED);
		}
		else if (mercenary_type.equals(factions.MERC_END_OVERT_IMPERIAL))
		{
			pvpNeutralSetMercenaryFaction(self, 0, false);
			sendSystemMessage(self, factions.SID_MERC_IMPERIAL_SF_TERMINATED);
		}
		else if (mercenary_type.equals(factions.MERC_END_COVERT_REBEL))
		{
			pvpNeutralSetMercenaryFaction(self, 0, false);
			sendSystemMessage(self, factions.SID_MERC_REBEL_COMBATANT_TERMINATED);
		}
		else if (mercenary_type.equals(factions.MERC_END_OVERT_REBEL))
		{
			pvpNeutralSetMercenaryFaction(self, 0, false);
			sendSystemMessage(self, factions.SID_MERC_REBEL_SF_TERMINATED);
		}
		else if (mercenary_type.equals(factions.MERC_BEGIN_COVERT_IMPERIAL))
		{
			pvpNeutralSetMercenaryFaction(self, (-615855020), false);
			sendSystemMessage(self, factions.SID_MERC_IMPERIAL_COMBATANT_GRANTED);
		}
		else if (mercenary_type.equals(factions.MERC_BEGIN_OVERT_IMPERIAL))
		{
			pvpNeutralSetMercenaryFaction(self, (-615855020), true);
			sendSystemMessage(self, factions.SID_MERC_IMPERIAL_SF_GRANTED);
		}
		else if (mercenary_type.equals(factions.MERC_BEGIN_COVERT_REBEL))
		{
			pvpNeutralSetMercenaryFaction(self, (370444368), false);
			sendSystemMessage(self, factions.SID_MERC_REBEL_COMBATANT_GRANTED);
		}
		else if (mercenary_type.equals(factions.MERC_BEGIN_OVERT_REBEL))
		{
			pvpNeutralSetMercenaryFaction(self, (370444368), true);
			sendSystemMessage(self, factions.SID_MERC_REBEL_SF_GRANTED);
		}
		else
		{
			return SCRIPT_CONTINUE;
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int cmdGcwScore(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		showGcwScoreboard(self);
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int displayGcwScoreSui(obj_id self, dictionary params) throws InterruptedException
	{
		showGcwScoreboard(self);
		
		return SCRIPT_CONTINUE;
	}
	
	
	public void showGcwScoreboard(obj_id self) throws InterruptedException
	{
		if (!isIdValid(self) || !exists(self))
		{
			return;
		}
		
		if (utils.hasScriptVar(self, "gcw.score.pid"))
		{
			int pid = utils.getIntScriptVar(self, "gcw.score.pid");
			
			utils.removeScriptVar(self, "gcw.score.pid");
			
			forceCloseSUIPage(pid);
		}
		
		String[][] scoreData = gcw_score.getAllGcwData();
		
		if (scoreData == null || scoreData.length <= 0)
		{
			sendSystemMessage(self, new string_id("spam", "gcw_score_empty"));
			return;
		}
		
		String[] table_titles =
		{
			"@guild:table_title_name", "@spam:table_title_faction", "@spam:table_title_profession", "@spam:table_title_level", 
			"@spam:table_title_gcw", "@spam:table_title_pvp_kills", "@spam:table_title_kills",
			"@spam:table_title_assists", "@spam:table_title_crafted", "@spam:table_title_destroyed"
		};
		
		String[] table_types =
		{
			"text", "text", "text", "integer", "integer", "integer", "integer", "integer", "integer", "integer"
		};
		
		String statisticsTitle = "Invasion Statistics";
		
		int phase = gcw_score.getGcwPhase();
		int winner = gcw_score.getGcwWinner();
		
		String invasionPhase = "";
		String invasionWinner = "";
		
		switch(phase)
		{
			case gcw.GCW_CITY_PHASE_UNKNOWN:
			invasionPhase = "Invasion Over";
			
			if (winner == factions.FACTION_FLAG_REBEL)
			{
				invasionWinner = COLOR_REBELS + " Rebels won!";
			}
			else if (winner == factions.FACTION_FLAG_IMPERIAL)
			{
				invasionWinner = COLOR_IMPERIALS + " Imperials won!";
			}
			break;
			case gcw.GCW_CITY_PHASE_CONSTRUCTION:
			invasionPhase = "Construction Underway";
			break;
			case gcw.GCW_CITY_PHASE_COMBAT:
			invasionPhase = "Invasion Underway";
			break;
			default:
			invasionPhase = "Unknown";
			break;
		}
		
		String statisticsInfoStr = "Invasion phase: "+ invasionPhase + invasionWinner;
		
		int pid = sui.tableRowMajor(self, self, sui.OK_CANCEL, statisticsTitle, "onGcwScore", statisticsInfoStr, table_titles, table_types, scoreData, true);
		
		utils.setScriptVar(self, "gcw.score.pid", pid);
	}
}
