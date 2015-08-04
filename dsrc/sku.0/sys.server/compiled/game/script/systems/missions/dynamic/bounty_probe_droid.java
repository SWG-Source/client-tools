package script.systems.missions.dynamic;

import script.*;
import script.base_class.*;
import script.combat_engine.*;
import java.util.Arrays;
import java.util.Hashtable;
import java.util.Vector;
import script.base_script;

import script.library.ai_lib;
import script.library.regions;
import script.library.locations;
import script.library.groundquests;
import script.library.utils;
import script.library.bounty_hunter;



public class bounty_probe_droid extends script.systems.missions.base.mission_dynamic_base
{
	public bounty_probe_droid()
	{
	}
	
	public int OnAttach(obj_id self) throws InterruptedException
	{
		
		int intDroidType = getIntObjVar(self, "intDroidType");
		if (intDroidType == DROID_PROBOT)
		{
			setCount(self, 10);
			
		}
		else if (intDroidType == DROID_SEEKER)
		{
			setCount(self, 20);
		}
		
		return SCRIPT_CONTINUE;
		
	}
	
	
	public int OnObjectMenuRequest(obj_id self, obj_id player, menu_info mi) throws InterruptedException
	{
		
		int intRootMenu;
		
		obj_id objBountyMission = getBountyMission(player);
		if (objBountyMission == null)
		{
			return SCRIPT_CONTINUE;
		}
		
		if (!utils.isProfession( player, utils.BOUNTY_HUNTER ))
		{
			string_id strResponse = new string_id("mission/mission_generic", "bounty_no_ability");
			sendSystemMessage(player, strResponse);
			return SCRIPT_CONTINUE;
		}
		
		int intDroidType = getIntObjVar(self, "intDroidType");
		if (intDroidType == DROID_PROBOT)
		{
			string_id strProbeId = new string_id("mission/mission_generic", "probe_droid_summon");
			intRootMenu = mi.addRootMenu (menu_info_types.SERVER_PROBE_DROID_FIND_TARGET, strProbeId);
			return SCRIPT_CONTINUE;
		}
		
		string_id strActivateId = new string_id("mission/mission_generic", "probe_droid_activate");
		string_id strFindId = new string_id("mission/mission_generic", "probe_droid_find_target");
		string_id strTrackId = new string_id("mission/mission_generic", "probe_droid_track_target");
		intRootMenu = mi.addRootMenu (menu_info_types.SERVER_PROBE_DROID_ACTIVATE, strActivateId);
		
		mi.addSubMenu(intRootMenu, menu_info_types.SERVER_PROBE_DROID_FIND_TARGET, strFindId);
		
		if (hasCommand(player, "droid_track"))
		{
			mi.addSubMenu(intRootMenu, menu_info_types.SERVER_PROBE_DROID_TRACK_TARGET, strTrackId );
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int OnObjectMenuSelect(obj_id self, obj_id player, int item) throws InterruptedException
	{
		
		if (item == menu_info_types.EXAMINE)
		{
			return SCRIPT_CONTINUE;
			
		}
		if (item == menu_info_types.ITEM_DESTROY)
		{
			return SCRIPT_CONTINUE;
		}
		
		int intDroidType = getIntObjVar(self, "intDroidType");
		
		obj_id objBountyMission = getBountyMission(player);
		if (objBountyMission == null)
		{
			string_id strSpam = new string_id("mission/mission_generic", "bounty_no_mission");
			sendSystemMessage(player, strSpam);
			return SCRIPT_CONTINUE;
		}
		
		if (!utils.isProfession( player, utils.BOUNTY_HUNTER ))
		{
			return SCRIPT_CONTINUE;
		}
		
		obj_id objMission = getBountyMission(player);
		
		int intState = getIntObjVar(objMission, "intState");
		
		if (intState != STATE_BOUNTY_PROBE)
		{
			string_id strSpam = new string_id("mission/mission_generic", "bounty_no_signature");
			sendSystemMessage(player, strSpam);
			return SCRIPT_CONTINUE;
			
		}
		if (hasObjVar(objMission, "intTracking"))
		{
			string_id strSpam = new string_id("mission/mission_generic", "bounty_already_tracking");
			sendSystemMessage(player, strSpam);
			return SCRIPT_CONTINUE;
			
		}
		
		if (hasObjVar(objMission, "objTarget"))
		{
			
			obj_id objTarget = getObjIdObjVar(objMission, "objTarget");
			if (objTarget != null)
			{
				dictionary dctJediInfo = requestJedi(objTarget);
				if (dctJediInfo == null)
				{
					
					string_id strSpam = new string_id("mission/mission_generic", "jedi_not_online");
					sendSystemMessage(player, strSpam);
					return SCRIPT_CONTINUE;
				}
				else
				{
					boolean boolOnline = dctJediInfo.getBoolean("online");
					if (!boolOnline)
					{
						
						string_id strSpam = new string_id("mission/mission_generic", "jedi_not_online");
						sendSystemMessage(player, strSpam);
						return SCRIPT_CONTINUE;
					}
				}
			}
			
		}
		
		dictionary dctParams = new dictionary();
		dctParams.put("objPlayer", player);
		setObjVar(self, "objPlayer", player);
		debugServerConsoleMsg(self, "Droid Type is "+intDroidType);
		if (item == menu_info_types.SERVER_PROBE_DROID_TRACK_TARGET)
		{
			bounty_hunter.probeDroidTrackTarget(player, self);
		}
		else if (item == menu_info_types.SERVER_PROBE_DROID_FIND_TARGET)
		{
			location currentPlayerLocation = getLocation(player);
			if (isValidId(currentPlayerLocation.cell))
			{
				string_id strResponse = new string_id("mission/mission_generic", "not_in_house");
				sendSystemMessage(player, strResponse);
				return SCRIPT_CONTINUE;
			}
			
			if (intDroidType == DROID_PROBOT)
			{
				debugServerConsoleMsg(self, "PROBOT, STARTING FIND");
				dctParams.put("intDroidType", intDroidType);
				dctParams.put("objDroid", self);
				dctParams.put("intTrackType", DROID_FIND_TARGET);
				
				if (!hasObjVar(objMission, "intMissionDynamic"))
				{
					dctParams.put("playerBounty", 1);
				}
				
				messageTo(self, "droid_Probot_Start", dctParams, 0, true);
				
			}
			if (intDroidType == DROID_SEEKER)
			{
				string_id strSpam = new string_id("mission/mission_generic", "seeker_droid_launched");
				sendSystemMessage(player, strSpam);
				debugServerConsoleMsg(self, "seeker");
				dctParams.put("intDroidType", intDroidType);
				dctParams.put("objDroid", self);
				dctParams.put("intTrackType", DROID_FIND_TARGET);
				
				if (!hasObjVar(objMission, "intMissionDynamic"))
				{
					dctParams.put("playerBounty", 1);
				}
				
				location locHeading = getHeading(player);
				location locSpawnLocation = getLocation(player);
				
				locSpawnLocation.x = locSpawnLocation.x + locHeading.x;
				locSpawnLocation.z = locSpawnLocation.z + locHeading.z;
				
				obj_id objSeeker = createObject("object/creature/npc/droid/bounty_seeker.iff", locSpawnLocation);
				messageTo(objSeeker, "takeOff", null, 5, true);
				messageTo(objMission, "halfwayNotification", dctParams, 40, true);
				messageTo(objMission, "halfwayNotification", dctParams, 60, true);
				messageTo(objMission, "findTarget", dctParams, 20, true);
				int intCount = getCount(self);
				intCount = intCount - 1;
				if (intCount < 0)
				{
					destroyObject(self);
				}
				else
				{
					setCount(self, intCount);
				}
			}
			
		}
		else if (item == menu_info_types.SERVER_PROBE_DROID_ACTIVATE)
		{
			
		}
		
		return SCRIPT_CONTINUE;
		
	}
	
	
	public int droid_Probot_Start(obj_id self, dictionary params) throws InterruptedException
	{
		
		debugServerConsoleMsg(self, "PROBE_DROID STARTED");
		obj_id objPlayer = params.getObjId("objPlayer");
		
		location locCurrentLocation = getLocation(objPlayer);
		
		region rgnCities[] = getRegionsWithMunicipalAtPoint(locCurrentLocation, regions.MUNI_TRUE);
		if (rgnCities != null)
		{
			debugServerConsoleMsg(self, "cities!");
			string_id strResponse = new string_id("mission/mission_generic", "probe_droid_bad_location");
			sendSystemMessage(objPlayer, strResponse);
			return SCRIPT_CONTINUE;
			
		}
		location locSpawnLocation = groundquests.getRandom2DLocationAroundLocation(objPlayer, 3, 3, 64, 64);
		if (locSpawnLocation != null)
		{
			if (isSpaceScene())
			{
				string_id strResponse = new string_id("mission/mission_generic", "in_space");
				sendSystemMessage(objPlayer, strResponse);
				return SCRIPT_CONTINUE;
			}
			rgnCities = getRegionsWithMunicipalAtPoint(locSpawnLocation, regions.MUNI_TRUE);
			if (rgnCities != null)
			{
				debugServerConsoleMsg(self, "cities2");
				string_id strResponse = new string_id("mission/mission_generic", "probe_droid_bad_location");
				sendSystemMessage(objPlayer, strResponse);
				return SCRIPT_CONTINUE;
				
			}
			
			setObjVar(self, "locSpawnLocation", locSpawnLocation);
			
			int intTrackType = params.getInt("intTrackType");
			obj_id objMission = getBountyMission(objPlayer);
			if (objMission == null)
			{
				return SCRIPT_CONTINUE;
			}
			setObjVar(objMission, "intTracking", 1);
			setObjVar(self, "objMission", objMission);
			setObjVar(self, "intTrackType", intTrackType);
			
			dictionary dctParams = new dictionary();
			
			string_id strResponse = new string_id("mission/mission_generic", "probe_droid_launch_prep");
			sendSystemMessage(objPlayer, strResponse);
			
			dctParams.put("intCountDown", 6);
			messageTo(self, "probe_Droid_Countdown", dctParams, 20, true);
			
		}
		else
		{
			
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int probe_Droid_Countdown(obj_id self, dictionary params) throws InterruptedException
	{
		int intCountDown = params.getInt("intCountDown");
		debugServerConsoleMsg(self, "intCountDown is "+intCountDown);
		obj_id objPlayer = getObjIdObjVar(self, "objPlayer");
		intCountDown = intCountDown-1;
		if (intCountDown == 0)
		{
			
			location locSpawnLocation = getLocationObjVar(self, "locSpawnLocation");
			
			location locTest = getLocation(objPlayer);
			locSpawnLocation.y = locTest.y;
			
			playClientEffectLoc(objPlayer, "clienteffect/probot_delivery.cef", locSpawnLocation, 0);
			
			messageTo(self, "probe_Droid_Spawn", null, 4, true);
			
		}
		else
		{
			string_id strResponse = new string_id("mission/mission_generic", "probe_droid_arrival_"+intCountDown);
			sendSystemMessage(objPlayer, strResponse);
			dictionary dctParams = new dictionary();
			dctParams.put("intCountDown", intCountDown);
			messageTo(self, "probe_Droid_Countdown", dctParams, 1, true);
			
		}
		return SCRIPT_CONTINUE;
		
	}
	
	public int probe_Droid_Spawn(obj_id self, dictionary params) throws InterruptedException
	{
		location locSpawnLocation = getLocationObjVar(self, "locSpawnLocation");
		string_id strResponse = new string_id("mission/mission_generic", "probe_droid_arrival");
		
		obj_id objPlayer = getObjIdObjVar(self, "objPlayer");
		sendSystemMessage(objPlayer, strResponse);
		int intTrackType = getIntObjVar(self, "intTrackType");
		int intDroidType = getIntObjVar(self, "intDroidType");
		
		obj_id objMission = getBountyMission(objPlayer);
		if (objMission == null)
		{
			return SCRIPT_CONTINUE;
		}
		
		obj_id objDroid = createObject( "object/creature/npc/droid/imperial_probot_bounty.iff", locSpawnLocation);
		if (objDroid == null)
		{
			
			debugServerConsoleMsg(self, "Null probe droid!");
			
		}
		attachScript(objDroid, "systems.missions.dynamic.bounty_probot");
		setMovementRun(objDroid);
		follow(objDroid, objPlayer, 3.f, 5.f);
		
		dictionary dctParams = new dictionary();
		dctParams.put("objPlayer", objPlayer);
		dctParams.put("intTrackType", intTrackType);
		dctParams.put("intDroidType", intDroidType);
		dctParams.put("objMission", objMission);
		
		messageTo(objDroid, "setup_Droid", dctParams, 0, true);
		int intCount = getCount(self);
		intCount = intCount - 1;
		if (intCount < 0)
		{
			destroyObject(self);
		}
		else
		{
			setCount(self, intCount);
		}
		return SCRIPT_CONTINUE;
	}
}
