package script.systems.missions.dynamic;

import script.*;
import script.base_class.*;
import script.combat_engine.*;
import java.util.Arrays;
import java.util.Hashtable;
import java.util.Vector;
import script.base_script;

import script.library.locations;
import script.library.utils;
import script.library.ai_lib;
import script.library.jedi;
import script.library.missions;
import script.library.stealth;



public class mission_bounty extends script.systems.missions.base.mission_dynamic_base
{
	public mission_bounty()
	{
	}
	
	public int OnInitialize(obj_id self) throws InterruptedException
	{
		if (!hasScript (self, "systems.missions.base.mission_cleanup_tracker"))
		{
			attachScript (self, "systems.missions.base.mission_cleanup_tracker");
		}
		
		if (hasObjVar(self, "movingTarget"))
		{
			if (hasObjVar(self, "intTargetSpawned"))
			{
				removeObjVar(self, "movingTarget");
			}
			else
			{
				dictionary d = new dictionary();
				d.put("moveTargetSequence", utils.getIntScriptVar(self, "moveTargetSequence"));
				messageTo(self, "moveTarget", d, rand(30, 60), false);
			}
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int onArrivedAtLocation(obj_id self, dictionary params) throws InterruptedException
	{
		debugServerConsoleMsg(self, "Arrived at location message");
		obj_id objPlayer = getMissionHolder(self);
		dictionary dctParams = new dictionary();
		
		dctParams.put("objMission", self);
		messageTo(objPlayer, "bounty_Arrival", dctParams, 0, true);
		return SCRIPT_CONTINUE;
	}
	
	
	public int abortMission(obj_id self, dictionary params) throws InterruptedException
	{
		obj_id target = getObjIdObjVar(self, "objTarget");
		obj_id owner = getMissionHolder(self);
		if (isIdValid(target) && isIdValid(owner))
		{
			removeJediBounty(target, owner);
		}
		
		if (isIdValid(owner))
		{
			setObjVar(owner, "intState", missions.STATE_MISSION_COMPLETE);
		}
		
		dictionary newParams = new dictionary();
		if (target == null)
		{
			target = obj_id.NULL_ID;
		}
		newParams.put("target", target);
		bountyIncomplete(self, newParams);
		return SCRIPT_CONTINUE;
	}
	
	
	public int timedOutMission(obj_id self, dictionary params) throws InterruptedException
	{
		obj_id target = getObjIdObjVar(self, "objTarget");
		obj_id owner = getMissionHolder(self);
		if (isIdValid(target) && isIdValid(owner))
		{
			removeJediBounty(target, owner);
		}
		
		if (isIdValid(owner))
		{
			setObjVar(owner, "intState", missions.STATE_MISSION_COMPLETE);
		}
		
		dictionary newParams = new dictionary();
		if (target == null)
		{
			target = obj_id.NULL_ID;
		}
		newParams.put("target", target);
		bountyTimedOut(self, newParams);
		return SCRIPT_CONTINUE;
	}
	
	
	public int OnStartMission(obj_id self, dictionary params) throws InterruptedException
	{
		
		setupInvisibleWaypoint(self);
		
		if (!hasObjVar(self, "objTarget"))
		{
			setObjVar(self, "intMissionDynamic", 1);
			location locSpawnLocation = getLocationObjVar(self, "locSpawnLocation");
			setObjVar(self, "locSpawnLocation", locSpawnLocation);
			setupSpawn(self, locSpawnLocation);
			
		}
		else
		{
			obj_id objTarget = getObjIdObjVar(self, "objTarget");
			
			dictionary dctParams = new dictionary();
			dctParams.put("objMission", self);
			dctParams.put("objKiller", getMissionHolder(self));
			messageTo(objTarget, "addJediListener", dctParams, 0, true);
			
			dictionary d = new dictionary();
			d.put("gametime", getGameTime());
			messageTo(objTarget, "handleUpdateBountyMissionTime", d, 0.0f, true);
		}
		
		setObjVar(self, "intState", STATE_BOUNTY_INFORMANT);
		
		obj_id objMissionData = getMissionData(self);
		obj_id objPlayer = getMissionHolder(self);
		
		int intMissionDifficulty = getIntObjVar(objMissionData, "intBountyDifficulty");
		String strEventName = "";
		if (intMissionDifficulty == 1)
		{
			strEventName = "EasyBountyMissionGranted";
		}
		else
		{
			strEventName = "HardBountyMissionGranted";
		}
		
		dictionary dctParams = new dictionary();
		dctParams.put("eventName",strEventName);
		messageTo(objPlayer, "handleHolocronEvent", dctParams, 0, false);
		return SCRIPT_CONTINUE;
		
	}
	
	
	public int bountyIncomplete(obj_id self, dictionary params) throws InterruptedException
	{
		obj_id objPlayer = getMissionHolder(self);
		String strTitleString = MISSION_INCOMPLETE_PERSISTENT_MESSAGE;
		obj_id objMissionData = getMissionData(self);
		int intStringId = getIntObjVar(objMissionData, "intStringId");
		String strMessageString = "m"+intStringId+"d";
		string_id strMessage = new string_id(GENERIC_MISSION_MESSAGE_STRING_FILE, "incomplete");
		sendSystemMessage(objPlayer, strMessage);
		sendMissionPersistentMessage(self, "incomplete");
		
		if (hasObjVar(objMissionData, "jediBonusReward"))
		{
			missions.increaseBountyJediKillTracking(objPlayer, missions.ABORTS);
		}
		endMission(self);
		return SCRIPT_CONTINUE;
	}
	
	
	public int bountyTimedOut(obj_id self, dictionary params) throws InterruptedException
	{
		obj_id objPlayer = getMissionHolder(self);
		String strTitleString = MISSION_TIMED_OUT_PERSISTENT_MESSAGE;
		obj_id objMissionData = getMissionData(self);
		int intStringId = getIntObjVar(objMissionData, "intStringId");
		String strMessageString = "m"+intStringId+"d";
		string_id strMessage = new string_id(GENERIC_MISSION_MESSAGE_STRING_FILE, "timed_out");
		sendSystemMessage(objPlayer, strMessage);
		sendMissionPersistentMessage(self, "timed_out");
		
		if (hasObjVar(objMissionData, "jediBonusReward"))
		{
			missions.increaseBountyJediKillTracking(objPlayer, missions.TIMEOUTS);
		}
		endMission(self);
		return SCRIPT_CONTINUE;
	}
	
	
	public int bountyFailure(obj_id self, dictionary params) throws InterruptedException
	{
		obj_id objPlayer = getMissionHolder(self);
		string_id strMessage = new string_id(GENERIC_MISSION_MESSAGE_STRING_FILE, "failed");
		sendSystemMessage(objPlayer, strMessage);
		
		String strTitleString = MISSION_FAILURE_PERSISTENT_MESSAGE;
		
		obj_id objMissionData = getMissionData(self);
		int intStringId = getIntObjVar(objMissionData, "intStringId");
		String strMessageString = "m"+intStringId+"d";
		
		sendMissionPersistentMessage(self, "fail");
		
		if (hasObjVar(objMissionData, "jediBonusReward"))
		{
			missions.increaseBountyJediKillTracking(objPlayer, missions.LOSSES);
		}
		endMission(self);
		return SCRIPT_CONTINUE;
	}
	
	
	public int bountySuccess(obj_id self, dictionary params) throws InterruptedException
	{
		if (!hasObjVar(self, "intCompleted"))
		{
			setObjVar(self, "intCompleted", 1);
			obj_id objPlayer = getMissionHolder(self);
			deliverReward(self);
			playMusic(objPlayer, "sound/music_mission_complete.snd");
			endMission(self);
		}
		return SCRIPT_CONTINUE;
		
	}
	
	
	public int OnEndMission(obj_id self, dictionary params) throws InterruptedException
	{
		
		cleanupObjects(self);
		debugServerConsoleMsg(self, "Ending mission");
		
		cleanupLocationTargets(self);
		debugServerConsoleMsg(self, "cleaned up locationt argets, getting objwaypoint objvars");
		obj_id objTargetWaypoint = getObjIdObjVar(self, "objTargetWaypoint");
		debugServerConsoleMsg(self, "got waypoint objvar");
		
		if (objTargetWaypoint == null)
		{
			
			debugServerConsoleMsg(self, "objTargetWaypoint ObjVar is null!");
			return SCRIPT_CONTINUE;
			
		}
		debugServerConsoleMsg(self, "ObjTargetWaypoint is "+objTargetWaypoint);
		setWaypointActive(objTargetWaypoint, false);
		setWaypointVisible(objTargetWaypoint, false);
		
		destroyObject(objTargetWaypoint);
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int findTarget(obj_id self, dictionary params) throws InterruptedException
	{
		
		debugServerConsoleMsg(self, "Tracking target");
		dictionary dctParams = new dictionary();
		obj_id objPlayer = getMissionHolder(self);
		setObjVar(self, "intTracking", 1);
		
		obj_id objDroid = params.getObjId("objDroid");
		int intDroidType = params.getInt("intDroidType");
		int intTrackType = params.getInt("intTrackType");
		location locCurrentLocation = getLocation(objPlayer);
		String strHomePlanet = locCurrentLocation.area;
		debugServerConsoleMsg(self, "strHomePlanet is "+strHomePlanet);
		
		obj_id objMissionData = getMissionData(self);
		
		int intTrackChance = getSkillStatMod(objPlayer, "droid_track_chance");
		int intFindChance = getSkillStatMod(objPlayer, "droid_find_chance");
		int intTracksLeft = getSkillStatMod(objPlayer, "droid_tracks");
		int intTrackSpeed = getSkillStatMod(objPlayer, "droid_track_speed");
		int intFindSpeed = getSkillStatMod(objPlayer, "droid_find_speed");
		
		LOG("missions", "intTrackSpeed = "+intTrackSpeed);
		LOG("missions", "intFindSpeed = "+intFindSpeed);
		
		location locPlayerLocation = getLocation(objPlayer);
		
		float fltDistance = 0;
		
		if (!hasObjVar(self, "objTarget"))
		{
			location locSpawnLocation = getLocationObjVar(self, "locSpawnLocation");
			
			fltDistance = getDistance(locPlayerLocation, locSpawnLocation);
			if (locPlayerLocation.area != locSpawnLocation.area)
			{
				fltDistance = fltDistance + 8000;
			}
		}
		else
		{
			fltDistance = rand(5000, 15000);
		}
		
		LOG("missions", "fltDistance is "+fltDistance);
		
		float fltBaseFindSpeed = fltDistance/150;
		float fltBaseTrackSpeed = fltDistance/75;
		LOG("missions", "base find speed is "+fltBaseFindSpeed);
		LOG("missions", "base track speed is "+fltBaseTrackSpeed);
		
		fltBaseFindSpeed = fltBaseFindSpeed + BOUNTY_FIND_SPEED;
		fltBaseTrackSpeed = fltBaseTrackSpeed + BOUNTY_TRACK_SPEED;
		
		LOG("missions", "base1 find speed is "+fltBaseFindSpeed);
		LOG("missions", "base1 track speed is "+fltBaseTrackSpeed);
		
		fltBaseFindSpeed = fltBaseFindSpeed + rand(60, 180);
		fltBaseTrackSpeed = fltBaseTrackSpeed + rand(60, 300);
		
		LOG("missions", "base2 find speed is "+fltBaseFindSpeed);
		LOG("missions", "base2 track speed is "+fltBaseTrackSpeed);
		
		float fltTrackSpeed = (float)(intTrackSpeed);
		float fltFindSpeed = (float)(intFindSpeed);
		fltTrackSpeed = 1f - (fltTrackSpeed/ 100);
		fltFindSpeed = 1f - (fltFindSpeed / 100);
		
		if (fltTrackSpeed <= 0)
		{
			fltTrackSpeed = .01f;
		}
		if (fltFindSpeed <= 0)
		{
			fltFindSpeed = .01f;
		}
		
		fltTrackSpeed = fltBaseTrackSpeed* fltTrackSpeed;
		fltFindSpeed = fltBaseFindSpeed* fltFindSpeed;
		
		if (fltTrackSpeed < 30)
		{
			fltTrackSpeed = 30.0f;
		}
		
		if (fltFindSpeed < 60)
		{
			fltFindSpeed = 60.0f;
			
		}
		
		int intRoll = rand(1, 100);
		
		if (intTrackType == DROID_TRACK_TARGET)
		{
			debugServerConsoleMsg(self, "track type is DROID_TRACK_TARGER");
			
			if (intRoll <= intFindChance)
			{
				if (intRoll <= intTrackChance)
				{
					
					setObjVar(self, "intTracking", 1);
					dctParams.put("intDroidType", intDroidType);
					dctParams.put("strHomePlanet", locCurrentLocation.area);
					dctParams.put("intTracksLeft", intTracksLeft);
					dctParams.put("fltTrackSpeed", fltTrackSpeed);
					dctParams.put("intTrackType", intTrackType);
					messageTo(self, "probe_Droid_Response", dctParams, fltTrackSpeed, true);
					
				}
				else
				{
					
					setObjVar(self, "intTracking", 1);
					dctParams.put("intDroidType", intDroidType);
					dctParams.put("strHomePlanet", locCurrentLocation.area);
					dctParams.put("intTracksLeft", 1);
					dctParams.put("intTrackType", intTrackType);
					messageTo(self, "probe_Droid_Response", dctParams, fltFindSpeed, true);
				}
			}
			else
			{
				messageTo(self, "probe_Droid_Find_Fail", null, rand(0, 10), true);
				return SCRIPT_CONTINUE;
				
			}
		}
		else if (intTrackType == DROID_FIND_TARGET)
		{
			
			if (intRoll <= intFindChance)
			{
				debugServerConsoleMsg(self, "finding target at speed "+fltFindSpeed);
				setObjVar(self, "intTracking", 1);
				dctParams.put("intDroidType", intDroidType);
				dctParams.put("strHomePlanet", locCurrentLocation.area);
				dctParams.put("intTracksLeft", 1);
				dctParams.put("intTrackType", intTrackType);
				messageTo(self, "probe_Droid_Response", dctParams, fltFindSpeed, true);
				
			}
			else
			{
				messageTo(self, "probe_Droid_Find_Fail", null, rand(0, 10), true);
				return SCRIPT_CONTINUE;
				
			}
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int probe_Droid_Find_Fail(obj_id self, dictionary params) throws InterruptedException
	{
		string_id strSpam = new string_id("mission/mission_generic", "target_not_found_"+rand(1,6));
		sendSystemMessage(getMissionHolder(self), strSpam);
		removeObjVar(self, "intTracking");
		return SCRIPT_CONTINUE;
	}
	
	
	public int probe_Droid_Track_Lost(obj_id self, dictionary params) throws InterruptedException
	{
		string_id strSpam = new string_id("mission/mission_generic", "target_track_lost");
		sendSystemMessage(getMissionHolder(self), strSpam);
		return SCRIPT_CONTINUE;
	}
	
	
	public int probe_Droid_Response(obj_id self, dictionary params) throws InterruptedException
	{
		
		dictionary dctParams = new dictionary();
		obj_id objMissionData = getMissionData(self);
		obj_id objPlayer = getMissionHolder(self);
		String strHomePlanet = params.getString("strHomePlanet");
		
		int intTracksLeft = params.getInt("intTracksLeft");
		intTracksLeft = intTracksLeft - 1;
		if (intTracksLeft < 0)
		{
			
			debugServerConsoleMsg(self, "intTracksLeft is "+intTracksLeft);
			removeObjVar(self, "intTracking");
			messageTo(self, "probe_Droid_Track_Lost", dctParams, rand(10, 20), false);
			return SCRIPT_CONTINUE;
		}
		
		boolean canBeResolved = true;
		if (strHomePlanet != null && strHomePlanet.equals("dungeon1"))
		{
			string_id strSpam = new string_id("mission/mission_generic", "out_in_space");
			sendSystemMessage(objPlayer, strSpam);
			removeObjVar(self, "intTracking");
			messageTo(self, "probe_Droid_Track_Lost", dctParams, rand(10, 20), false);
			return SCRIPT_CONTINUE;
		}
		
		location locSpawnLocation = new location();
		obj_id objTarget = null;
		
		if (hasObjVar(objMissionData, "objTarget"))
		{
			
			objTarget = getObjIdObjVar(objMissionData, "objTarget");
			if (objTarget == null)
			{
				CustomerServiceLog("BountyHunter", "Lost target for hunter %TU", self);
				removeObjVar(self, "intTracking");
				messageTo(self, "probe_Droid_Track_Lost", dctParams, rand(10, 20), false);
				return SCRIPT_CONTINUE;
			}
			
			location tmpLocation = getLocation(objTarget);
			if (tmpLocation != null)
			{
				obj_id topmost = getTopMostContainer(objTarget);
				if (topmost != null)
				{
					tmpLocation = getLocation(topmost);
				}
				locSpawnLocation = (location)tmpLocation.clone();
			}
			else
			{
				canBeResolved = false;
				dictionary dctJediInfo = requestJedi(objTarget);
				if (dctJediInfo == null)
				{
					
					string_id strSpam = new string_id("mission/mission_generic", "jedi_not_online");
					sendSystemMessage(objPlayer, strSpam);
					removeObjVar(self, "intTracking");
					messageTo(self, "probe_Droid_Track_Lost", dctParams, rand(10, 20), false);
					return SCRIPT_CONTINUE;
				}
				
				boolean boolOnline = dctJediInfo.getBoolean("online");
				if (boolOnline)
				{
					locSpawnLocation = dctJediInfo.getLocation("location");
					if (locSpawnLocation == null)
					{
						CustomerServiceLog("BountyHunter", "Got null location from Jedi manager for Jedi %TT hunted by %TU", self, objTarget);
						removeObjVar(self, "intTracking");
						messageTo(self, "probe_Droid_Track_Lost", dctParams, rand(10, 20), false);
						return SCRIPT_CONTINUE;
					}
				}
				else
				{
					
					string_id strSpam = new string_id("mission/mission_generic", "jedi_not_online");
					sendSystemMessage(objPlayer, strSpam);
					removeObjVar(self, "intTracking");
					messageTo(self, "probe_Droid_Track_Lost", dctParams, rand(10, 20), false);
					return SCRIPT_CONTINUE;
				}
			}
		}
		else if (hasObjVar(objMissionData, "intTargetCreated"))
		{
			locSpawnLocation = getMissionStartLocation(objMissionData);
			
		}
		else
		{
			locSpawnLocation = getLocationObjVar(self, "locSpawnLocation");
		}
		
		float fltTrackSpeed = params.getFloat("fltTrackSpeed");
		
		fltTrackSpeed = fltTrackSpeed / 4;
		params.put("intTracksLeft", intTracksLeft);
		
		int intDroidType = params.getInt("intDroidType");
		
		boolean sendProbotResponse = false;
		boolean sendSeekerResponse = false;
		boolean booleanCoverState = params.getBoolean("booleanCoverState");
		
		if (toLower(locSpawnLocation.area).startsWith("kashyyyk") && intDroidType == DROID_PROBOT)
		{
			sendSystemMessage(objPlayer, new string_id("mission/mission_generic", "no_seek_kashyyyk"));
			removeObjVar(self, "intTracking");
			return SCRIPT_CONTINUE;
		}
		
		if (toLower(locSpawnLocation.area).startsWith("mustafar") && intDroidType == DROID_PROBOT)
		{
			sendSystemMessage(objPlayer, new string_id("mission/mission_generic", "no_seek_mustafar"));
			removeObjVar(self, "intTracking");
			return SCRIPT_CONTINUE;
		}
		
		if (!locSpawnLocation.area.equals(strHomePlanet))
		{
			if (intDroidType == DROID_PROBOT)
			{
				sendProbotResponse = true;
			}
			else
			{
				string_id strSpam = new string_id("mission/mission_generic", "target_not_on_planet");
				sendSystemMessage(objPlayer, strSpam);
				removeObjVar(self, "intTracking");
				return SCRIPT_CONTINUE;
			}
		}
		else if (canBeResolved)
		{
			if (intDroidType == DROID_PROBOT)
			{
				sendProbotResponse = true;
			}
			else
			{
				sendSeekerResponse = true;
			}
		}
		else if (objTarget != null)
		{
			findObjectAnywhere(objTarget, self);
			messageTo(self, "probe_Droid_Response", params, fltTrackSpeed, true);
		}
		else
		{
			CustomerServiceLog("BountyHunter", "Could not acquire target for hunter %TU", self);
			removeObjVar(self, "intTracking");
			messageTo(self, "probe_Droid_Track_Lost", dctParams, rand(10, 20), false);
			return SCRIPT_CONTINUE;
		}
		
		if (sendProbotResponse)
		{
			updateMissionWaypoint(self, locSpawnLocation);
			string_id strSpam = new string_id("mission/mission_generic", "target_location_updated");
			sendSystemMessage(objPlayer, strSpam);
			String strTargetLocation = "target_located_"+ locSpawnLocation.area;
			strSpam = new string_id("mission/mission_generic", strTargetLocation);
			sendSystemMessage(objPlayer, strSpam);
			
			messageTo(self, "probe_Droid_Response", params, fltTrackSpeed, true);
		}
		else if (sendSeekerResponse)
		{
			if (isIdValid(objTarget) && (stealth.hasServerCoverState(objTarget) || stealth.hasInvisibleBuff(objTarget)))
			{
				sendSystemMessage(objPlayer, new string_id("mission/mission_generic", "target_location_stealth"));
			}
			else
			{
				int intTrackType = params.getInt("intTrackType");
				
				updateMissionWaypoint(self, locSpawnLocation);
				string_id strSpam = new string_id("mission/mission_generic", "target_location_updated");
				sendSystemMessage(objPlayer, strSpam);
				
				if (intTrackType == DROID_TRACK_TARGET)
				{
					if (intTracksLeft > 0)
					{
						strSpam =new string_id( "mission/mission_generic", "target_continue_tracking");
						sendSystemMessage(objPlayer, strSpam);
					}
				}
				else if (intTrackType == DROID_FIND_TARGET)
				{
					if (hasObjVar(objMissionData, "strTargetName"))
					{
						String msg = utils.packStringId(new string_id("mission/mission_generic", "seeker_droid_seek_target"));
						msg += " "+getStringObjVar(objMissionData, "strTargetName");
						sendSystemMessage(objPlayer, msg, null);
					}
				}
			}
			
			messageTo(self, "probe_Droid_Response", params, fltTrackSpeed, true);
		}
		return SCRIPT_CONTINUE;
	}
	
	
	public int foundObject(obj_id self, dictionary params) throws InterruptedException
	{
		String sceneId = params.getString("scene");
		location loc = params.getLocation("location");
		loc.area = sceneId;
		obj_id objPlayer = getMissionHolder(self);
		
		updateMissionWaypoint(self, loc);
		string_id strSpam = new string_id("mission/mission_generic", "target_location_updated");
		sendSystemMessage(objPlayer, strSpam);
		strSpam =new string_id( "mission/mission_generic", "target_continue_tracking");
		sendSystemMessage(objPlayer, strSpam);
		return SCRIPT_CONTINUE;
	}
	
	
	public int moveTarget(obj_id self, dictionary params) throws InterruptedException
	{
		int sequence = utils.getIntScriptVar(self, "moveTargetSequence");
		if (params.getInt("moveTargetSequence") != sequence)
		{
			return SCRIPT_CONTINUE;
		}
		
		if (hasObjVar(self, "intTargetSpawned"))
		{
			return SCRIPT_CONTINUE;
		}
		
		debugServerConsoleMsg(self, "Moving Target");
		
		final int MOVEMENT_RATE_PER_SECOND = 1;
		final int MOVEMENT_RATE_MIN_CALLBACK = 40;
		final int MOVEMENT_RATE_MAX_CALLBACK = 75;
		int intDelay = rand(MOVEMENT_RATE_MIN_CALLBACK, MOVEMENT_RATE_MAX_CALLBACK);
		
		obj_id objMissionData = getMissionData(self);
		int intMissionDifficulty = getIntObjVar(objMissionData, "intBountyDifficulty");
		float fltMovementRate = MOVEMENT_RATE_PER_SECOND + intMissionDifficulty;
		float fltOffPlanetChance = intMissionDifficulty * 10;
		if (hasObjVar(self, "intMovedOffPlanet"))
		{
			fltOffPlanetChance = 0;
		}
		if (fltMovementRate > 4)
		{
			fltMovementRate = 4;
		}
		
		float fltDelay;
		
		location locSpawnLocation = getLocationObjVar(self, "locSpawnLocation");
		
		if (!hasObjVar(self, "locDestination"))
		{
			
			int intRoll = rand(1,100);
			if ((intRoll < fltOffPlanetChance)&&(hasStarPort(locSpawnLocation)))
			{
				
				String strNewPlanet = PLANETS[rand(0, PLANETS.length-1)];
				location locDestination = locations.getBountyLocation(strNewPlanet);
				setObjVar(self, "locSpawnLocation", locDestination);
				setObjVar(self, "intMovedOffPlanet", 1);
				fltDelay = rand(100, 300);
			}
			else
			{
				location locDestination = locations.getBountyLocation(locSpawnLocation.area);
				setObjVar(self, "locDestination", locDestination);
				fltDelay = rand(300, 900);
			}
		}
		else
		{
			location locDestination = getLocationObjVar(self, "locDestination");
			
			float fltDistance = 0;
			float fltTime = params.getFloat("fltTime");
			
			fltDistance = fltMovementRate * fltTime;
			locSpawnLocation = locations.moveLocationTowardsLocation(locSpawnLocation, locDestination, fltDistance);
			setObjVar(self, "locSpawnLocation", locSpawnLocation);
			
			fltDistance = getDistance(locDestination, locSpawnLocation);
			if (fltDistance < 128)
			{
				setObjVar(self, "locSpawnLocation", locDestination);
				locSpawnLocation = (location)locDestination.clone();
				removeObjVar(self, "locDestination");
			}
			setupSpawn(self, locSpawnLocation);
			fltDelay = rand(MOVEMENT_RATE_MIN_CALLBACK, MOVEMENT_RATE_MAX_CALLBACK);
		}
		
		dictionary dctParams = new dictionary();
		dctParams.put("fltTime", fltDelay);
		
		sequence = sequence + 1;
		utils.setScriptVar(self, "moveTargetSequence", sequence);
		dctParams.put("moveTargetSequence", sequence);
		messageTo(self, "moveTarget", dctParams, fltDelay, false);
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int stopTracking(obj_id self, dictionary params) throws InterruptedException
	{
		removeObjVar(self, "intTracking");
		return SCRIPT_CONTINUE;
		
	}
	
	
	public int halfwayNotification(obj_id self, dictionary params) throws InterruptedException
	{
		if ((params == null) || (params.isEmpty()))
		{
			return SCRIPT_CONTINUE;
		}
		
		obj_id objPlayer = params.getObjId("objPlayer");
		
		string_id strSpam = new string_id("mission/mission_generic", "seeker_droid_standby");
		sendSystemMessage(objPlayer, strSpam);
		return SCRIPT_CONTINUE;
	}
}
