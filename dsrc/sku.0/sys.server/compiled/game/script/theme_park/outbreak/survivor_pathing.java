package script.theme_park.outbreak;

import script.*;
import script.base_class.*;
import script.combat_engine.*;
import java.util.Arrays;
import java.util.Hashtable;
import java.util.Vector;
import script.base_script;

import script.library.ai_lib;
import script.library.attrib;
import script.library.create;
import script.library.groundquests;
import script.library.stealth;
import script.library.utils;


public class survivor_pathing extends script.base_script
{
	public survivor_pathing()
	{
	}
	public static final boolean LOGGING_ON = true;
	public static final String SCRIPT_LOG = "outbreak_pathing";
	public static final String PATHING_NODE = "pathing_node";
	public static final String SPAWNER_PATH = "spawner_path";
	public static final String SUCCESS_SIGNAL = "success_signal";
	public static final String FAIL_SIGNAL = "fail_signal";
	public static final String UPDATE_SIGNAL = "update_signal";
	public static final String WAYPOINT_LIST = "wayPointList";
	public static final String WAYPOINT_LOCS = "wayPtLocs";
	
	public static final int RADIUS = 300;
	
	
	public int OnIncapacitated(obj_id self, obj_id killer) throws InterruptedException
	{
		if (hasObjVar(self, "rescue") || hasObjVar(self, "incap"))
		{
			return SCRIPT_CONTINUE;
		}
		
		setObjVar(self, "incap", true);
		playerFailedQuest(self);
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int OnDestroy(obj_id self) throws InterruptedException
	{
		obj_id player = getObjIdObjVar(self, "myEscort");
		if (isValidId(player))
		{
			npcEndConversation(player);
		}
		
		if (hasObjVar(self, "rescue") || hasObjVar(self, "incap"))
		{
			return SCRIPT_CONTINUE;
		}
		
		playerFailedQuest(self);
		return SCRIPT_CONTINUE;
	}
	
	
	public int OnMovePathComplete(obj_id self) throws InterruptedException
	{
		
		setMovementRun(self);
		setBaseRunSpeed(self, (getBaseRunSpeed(self)-8));
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int OnMoveMoving(obj_id self) throws InterruptedException
	{
		
		setMovementRun(self);
		setBaseRunSpeed(self, (getBaseRunSpeed(self)-8));
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int startSurvivorPathing(obj_id self, dictionary params) throws InterruptedException
	{
		
		String creatureType = getStringObjVar(self, "creature_type");
		if (creatureType == null || creatureType.length() <= 0)
		{
			CustomerServiceLog("outbreak_themepark", "survivor_pathing.startSurvivorPathing() the NPC creature_type was not valid");
			return SCRIPT_CONTINUE;
		}
		
		if (!hasObjVar(self, WAYPOINT_LOCS))
		{
			CustomerServiceLog("outbreak_themepark", "survivor_pathing.startSurvivorPathing() Mob: "+self+" FAILED TO FIND waypointPathNodeLocations OBJVAR.");
			return SCRIPT_CONTINUE;
		}
		
		location[] wayPtLocs = getLocationArrayObjVar(self, WAYPOINT_LOCS);
		if (wayPtLocs == null || wayPtLocs.length <= 0)
		{
			CustomerServiceLog("outbreak_themepark", "survivor_pathing.startSurvivorPathing() Mob: "+self+" FAILED TO FIND waypointPathNodeLocations OBJVAR.");
			return SCRIPT_CONTINUE;
		}
		
		setMovementRun(self);
		setBaseRunSpeed(self, (getBaseRunSpeed(self)-8));
		
		setInvulnerable(self, false);
		
		utils.setScriptVar(self, ai_lib.SCRIPTVAR_CACHED_PATROL_NAMED_PATH, wayPtLocs);
		utils.setScriptVar(self, ai_lib.SCRIPTVAR_CACHED_PATROL_TYPE, 0);
		patrolOnce(self, wayPtLocs, 0);
		detachScript(self, "conversation.survivor_01");
		clearCondition(self, CONDITION_CONVERSABLE);
		setAttributeAttained(self, attrib.OUTBREAK_SURVIVOR);
		
		messageTo(self, "checkOwnerValidity", null, 10, false);
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int creditPlayerRescue(obj_id self, dictionary params) throws InterruptedException
	{
		
		if (!playerSuccessQuest(self))
		{
			CustomerServiceLog("outbreak_themepark", "survivor_pathing.creditPlayerRescue() ERROR - Player could not be properly credited for rescuing this NPC.");
		}
		else
		{
			CustomerServiceLog("outbreak_themepark", "survivor_pathing.creditPlayerRescue() Player was properly credited for rescuing this NPC.");
			destroyObject(self);
		}
		return SCRIPT_CONTINUE;
	}
	
	
	public int cleanUpNpcTimer(obj_id self, dictionary params) throws InterruptedException
	{
		
		if (!isValidId(self) || !exists(self))
		{
			return SCRIPT_CONTINUE;
		}
		
		if (!hasObjVar(self, "neverSpokenTo"))
		{
			CustomerServiceLog("outbreak_themepark", "survivor_pathing.cleanUpNpcTimer() The player owner has alread spoken to this NPC, this NPC will not destroy self early.");
			return SCRIPT_CONTINUE;
		}
		
		destroyObject(self);
		return SCRIPT_CONTINUE;
	}
	
	
	public boolean playerFailedQuest(obj_id self) throws InterruptedException
	{
		
		obj_id player = getObjIdObjVar(self, "myEscort");
		if (!isValidId(player))
		{
			CustomerServiceLog("outbreak_themepark", "survivor_pathing.playerFailedQuest() Player: "+player+" IOD was invalid so the player could not receive the fail signal.");
			return false;
		}
		
		String signalFail = getStringObjVar(self, FAIL_SIGNAL);
		if (signalFail == null || signalFail.length() <= 0)
		{
			CustomerServiceLog("outbreak_themepark", "survivor_pathing.playerFailedQuest() The Rescued NPC did not have the appropriate FAIL signal as an objvar for Player: "+player+" ("+getPlayerName(player)+").");
			return false;
		}
		
		groundquests.sendSignal(player, signalFail);
		return true;
	}
	
	
	public boolean playerSuccessQuest(obj_id self) throws InterruptedException
	{
		
		if (hasObjVar(self, "incap"))
		{
			CustomerServiceLog("outbreak_themepark", "survivor_pathing.playerSuccessQuest() The NPC was incapacitated and the player actually didn't rescue the NPC before death. Exiting this function.");
			return false;
		}
		
		obj_id player = getObjIdObjVar(self, "myEscort");
		if (!isValidId(player))
		{
			CustomerServiceLog("outbreak_themepark", "survivor_pathing.playerSuccessQuest() Player: "+player+" IOD was invalid so the player could not receive the fail signal.");
			return false;
		}
		
		String signalUpdate = getStringObjVar(self, UPDATE_SIGNAL);
		if (signalUpdate == null || signalUpdate.length() <= 0)
		{
			CustomerServiceLog("outbreak_themepark", "survivor_pathing.playerSuccessQuest() The Rescued NPC did not have the appropriate signal to update the players quest for Player: "+player+" ("+getPlayerName(player)+"). The player may not receive credit for the rescue.");
			return false;
		}
		
		groundquests.sendSignal(player, signalUpdate);
		
		String signalSuccess = getStringObjVar(self, SUCCESS_SIGNAL);
		if (signalSuccess == null || signalSuccess.length() <= 0)
		{
			CustomerServiceLog("outbreak_themepark", "survivor_pathing.playerSuccessQuest() The Rescued NPC did not have the appropriate signal as an objvar for Player: "+player+" ("+getPlayerName(player)+"). The player may not receive credit for the rescue.");
			return false;
		}
		
		groundquests.sendSignal(player, signalSuccess);
		
		setObjVar(self, "rescue", true);
		return true;
	}
	
	
	public int checkOwnerValidity(obj_id self, dictionary params) throws InterruptedException
	{
		
		if (hasScript(self, "conversation.survivor_02"))
		{
			return SCRIPT_CONTINUE;
		}
		
		if (!hasObjVar(self,"myEscort"))
		{
			return SCRIPT_CONTINUE;
		}
		
		obj_id owner = getObjIdObjVar(self, "myEscort");
		if (!isValidId(owner) || !exists(owner))
		{
			CustomerServiceLog("outbreak_themepark", "survivor_pathing.checkOwnerValidity() Mob: "+self+" FAILED TO FIND OWNER OBJVAR.");
			return SCRIPT_CONTINUE;
		}
		
		if (!hasObjVar(self,"creature_type"))
		{
			return SCRIPT_CONTINUE;
		}
		
		String creatureType = getStringObjVar(self, "creature_type");
		if (creatureType == null || creatureType.length() <= 0)
		{
			CustomerServiceLog("outbreak_themepark", "survivor_pathing.startSurvivorPathing() the NPC creature_type was not valid");
			return SCRIPT_CONTINUE;
		}
		
		if (!ai_lib.isInCombat(self))
		{
			
			if (!hasObjVar(self, "lastLocation"))
			{
				CustomerServiceLog("outbreak_themepark", "survivor_pathing.checkOwnerValidity() Mob: "+self+" is checking its very first location. Will check again later to see if stuck.");
				setObjVar(self,"lastLocation", getLocation(self));
			}
			else
			{
				
				location currentLocation = getLocation(self);
				location lastLocation = getLocationObjVar(self, "lastLocation");
				if (Math.abs(currentLocation.x - lastLocation.x) < 0.5 && Math.abs(currentLocation.z - lastLocation.z) < 0.5)
				{
					
					if (!hasObjVar(self, WAYPOINT_LIST))
					{
						CustomerServiceLog("outbreak_themepark", "survivor_pathing.checkOwnerValidity() Mob: "+self+" is STUCK and doesnt have waypoint object list. Blowing up this NPC!");
						messageTo(self, "blowUp", null, 2, false);
					}
					if (!hasObjVar(self, WAYPOINT_LOCS))
					{
						CustomerServiceLog("outbreak_themepark", "survivor_pathing.checkOwnerValidity() Mob: "+self+" is STUCK and doesnt have waypoint location list. Blowing up this NPC!");
						messageTo(self, "blowUp", null, 2, false);
					}
					
					obj_id waypointList[] = getObjIdArrayObjVar(self, WAYPOINT_LIST);
					if (waypointList == null || waypointList.length <= 0)
					{
						CustomerServiceLog("outbreak_themepark", "survivor_pathing.checkOwnerValidity() Mob: "+self+" is STUCK and doesnt have a valid waypoint object list. Blowing up this NPC!");
						messageTo(self, "blowUp", null, 2, false);
					}
					
					location waypointLocList[] = getLocationArrayObjVar(self, WAYPOINT_LOCS);
					if (waypointLocList == null || waypointLocList.length <= 0)
					{
						CustomerServiceLog("outbreak_themepark", "survivor_pathing.checkOwnerValidity() Mob: "+self+" is STUCK and doesnt have a valid waypoint location list. Blowing up this NPC!");
						messageTo(self, "blowUp", null, 2, false);
					}
					
					float smallestDist = 300;
					obj_id closestObj = obj_id.NULL_ID;
					boolean modified = false;
					for (int i = 0; i < waypointList.length; i++)
					{
						testAbortScript();
						if (!isValidId(waypointList[i]) || !exists(waypointList[i]))
						{
							continue;
						}
						
						float npcAndWaypointDist = getDistance(self, waypointList[i]);
						if (npcAndWaypointDist > smallestDist)
						{
							
							continue;
						}
						else
						{
							CustomerServiceLog("outbreak_themepark", "survivor_pathing.checkOwnerValidity() Mob: "+self+" has found a node ("+waypointList[i]+") that is closer than any previously found node. Previous dist: "+smallestDist+". New Dist: "+npcAndWaypointDist);
							
							smallestDist = npcAndWaypointDist;
							closestObj = waypointList[i];
							modified = true;
						}
					}
					
					if (isValidId(closestObj) && exists(closestObj))
					{
						
						if (!hasObjVar(closestObj, creatureType))
						{
							CustomerServiceLog("outbreak_themepark", "survivor_pathing.checkOwnerValidity() Mob: "+self+" is STUCK and the closest pathNode has no pathNode number!");
							return SCRIPT_CONTINUE;
						}
						
						int pathNodePriority = getIntObjVar(closestObj, creatureType);
						int newArrayLength = (waypointList.length - (pathNodePriority - 1));
						
						if (newArrayLength < 0)
						{
							CustomerServiceLog("outbreak_themepark", "survivor_pathing.checkOwnerValidity() Mob: "+self+" pathNode priority was: "+pathNodePriority+" while the waypoint length was: "+waypointList.length+". subtracting the 2 gave an invalid number!");
							return SCRIPT_CONTINUE;
							
						}
						else if (newArrayLength == 0 || newArrayLength == 1)
						{
							
							pathTo(self, waypointLocList[waypointLocList.length-1]);
							return SCRIPT_CONTINUE;
						}
						else
						{
							
							location[] newPathLocs = new location[newArrayLength];
							System.arraycopy(waypointLocList, (pathNodePriority-1), newPathLocs, 0, newArrayLength);
							patrolOnce(self,newPathLocs);
						}
					}
					
				}
				else
				{
					
					setObjVar(self,"lastLocation", getLocation(self));
				}
			}
		}
		else
		{
			
		}
		messageTo(self, "checkOwnerValidity", null, 10, false);
		
		return SCRIPT_CONTINUE;
	}
}
