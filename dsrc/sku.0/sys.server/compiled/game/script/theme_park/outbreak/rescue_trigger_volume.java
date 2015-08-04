package script.theme_park.outbreak;

import script.*;
import script.base_class.*;
import script.combat_engine.*;
import java.util.Arrays;
import java.util.Hashtable;
import java.util.Vector;
import script.base_script;

import script.library.create;
import script.library.groundquests;
import script.library.stealth;
import script.library.utils;


public class rescue_trigger_volume extends script.base_script
{
	public rescue_trigger_volume()
	{
	}
	public static final boolean LOGGING_ON = true;
	
	public static final String TRIGGER_NAME_PREFIX = "rescue_trigger_";
	public static final String TRIGGER = "meatlump_spy_trigger";
	public static final String MEATLUMP_ENEMY_SCRIPT = "creature.dynamic_enemy";
	public static final String PHRASE_STRING_FILE = "phrase_string_file";
	public static final String SPAM_STRING_FILE = "spam";
	public static final String SCRIPT_LOG = "outbreak_trigger";
	public static final String CLIENT_EFFECT = "appearance/pt_smoke_puff.prt";
	public static final String QUEST_STRING = "quest/outbreak_quest_rescue_0";
	public static final String QUEST_TASK_NAME = "spawnSurvivor";
	public static final String QUEST_SIGNAL = "survivorHasBeenSpawned";
	public static final String WAYPOINT_LIST = "wayPointList";
	public static final String WAYPOINT_LOCS = "wayPtLocs";
	
	public static final float TRIGGER_RADIUS = 3f;
	public static final int RADIUS = 300;
	
	public static final String[] MOB_FOR_QUEST =  
	{
		"outbreak_rescue_inquisitor",
		"outbreak_rescue_officer",
		"outbreak_rescue_prisoner",
		"outbreak_rescue_scientist",
		"outbreak_rescue_inquisitor",
		"outbreak_rescue_inquisitor",
		"outbreak_rescue_inquisitor",
		"outbreak_rescue_officer",
		"outbreak_rescue_officer",
		"outbreak_rescue_officer",
		"outbreak_rescue_prisoner",
		"outbreak_rescue_prisoner",
		"outbreak_rescue_prisoner",
		"outbreak_rescue_scientist",
		"outbreak_rescue_scientist",
		"outbreak_rescue_scientist"
	};
	
	
	public int OnInitialize(obj_id self) throws InterruptedException
	{
		
		createTriggerVolume(TRIGGER_NAME_PREFIX + self, TRIGGER_RADIUS, true);
		messageTo(self, "getAllEscortPathNodes", null, 5, false);
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int OnTriggerVolumeEntered(obj_id self, String volumeName, obj_id whoTriggeredMe) throws InterruptedException
	{
		if (!isPlayer(whoTriggeredMe))
		{
			return SCRIPT_CONTINUE;
		}
		
		if (!hasObjVar(self, "spawner"))
		{
			return SCRIPT_CONTINUE;
		}
		
		if (!hasObjVar(self, WAYPOINT_LIST))
		{
			CustomerServiceLog("outbreak_themepark", "rescue_trigger_volume.OnTriggerVolumeEntered() Spawner: "+self+" could not find the WAYPOINT LIST OF OBJVARS on self.");
			
			return SCRIPT_CONTINUE;
		}
		if (!hasObjVar(self, WAYPOINT_LOCS))
		{
			CustomerServiceLog("outbreak_themepark", "rescue_trigger_volume.OnTriggerVolumeEntered() Spawner: "+self+" could not find the WAYPOINT LIST OF LOCATIONS on self.");
			return SCRIPT_CONTINUE;
		}
		
		int spawner = getIntObjVar(self, "spawner");
		
		location[] wayPtLocs = getLocationArrayObjVar(self, WAYPOINT_LOCS);
		if (wayPtLocs == null || wayPtLocs.length <= 0)
		{
			CustomerServiceLog("outbreak_themepark", "rescue_trigger_volume.OnTriggerVolumeEntered() Spawner: "+self+" FAILED TO FIND wayPtLocs OBJVAR.");
			return SCRIPT_CONTINUE;
		}
		
		obj_id[] wayPointList = getObjIdArrayObjVar(self, WAYPOINT_LIST);
		if (wayPointList == null || wayPointList.length <= 0)
		{
			CustomerServiceLog("outbreak_themepark", "rescue_trigger_volume.OnTriggerVolumeEntered() Spawner: "+self+" FAILED TO FIND wayPointList OBJVAR.");
			return SCRIPT_CONTINUE;
		}
		
		if (!groundquests.isQuestActive(whoTriggeredMe, QUEST_STRING+spawner))
		{
			return SCRIPT_CONTINUE;
		}
		
		CustomerServiceLog("outbreak_themepark", "rescue_trigger_volume.OnTriggerVolumeEntered() Ground Quest found for Player: "+getPlayerName(whoTriggeredMe)+" ("+whoTriggeredMe+ ")");
		
		if (!groundquests.isTaskActive(whoTriggeredMe, QUEST_STRING+spawner, QUEST_TASK_NAME))
		{
			return SCRIPT_CONTINUE;
		}
		
		CustomerServiceLog("outbreak_themepark", "rescue_trigger_volume.OnTriggerVolumeEntered() Ground Quest Task found for Player: "+getPlayerName(whoTriggeredMe)+" ("+whoTriggeredMe+ ")");
		
		String invis = stealth.getInvisBuff(whoTriggeredMe);
		if (invis != null)
		{
			stealth.checkForAndMakeVisibleNoRecourse(self);
		}
		
		CustomerServiceLog("outbreak_themepark", "rescue_trigger_volume.OnTriggerVolumeEntered() Sending Ground Quest signal for Player: "+getPlayerName(whoTriggeredMe)+" ("+whoTriggeredMe+ ")");
		groundquests.sendSignal(whoTriggeredMe, QUEST_SIGNAL);
		
		location curLoc = utils.getRandomLocationInRing(getLocation(self), 0.01f, 1.0f);
		
		CustomerServiceLog("outbreak_themepark", "rescue_trigger_volume.OnTriggerVolumeEntered() Preparing to spawn NPC at: "+curLoc+" for Player: "+getPlayerName(whoTriggeredMe)+" ("+whoTriggeredMe+ ")");
		obj_id mob = create.object(MOB_FOR_QUEST[spawner-1], curLoc);
		if (!isValidId(mob) || !exists(mob))
		{
			CustomerServiceLog("outbreak_themepark", "rescue_trigger_volume.OnTriggerVolumeEntered() Mob: "+MOB_FOR_QUEST[spawner-1]+" could not be created!");
			return SCRIPT_CONTINUE;
		}
		CustomerServiceLog("outbreak_themepark", "rescue_trigger_volume.OnTriggerVolumeEntered() Mob: "+MOB_FOR_QUEST[spawner-1]+" WAS SICCESSFULLY CREATED for Player: "+getPlayerName(whoTriggeredMe)+" ("+whoTriggeredMe+ ")");
		
		setObjVar(mob, "myEscort", whoTriggeredMe);
		playClientEffectObj(mob, CLIENT_EFFECT, whoTriggeredMe, "");
		setObjVar(mob, "clientEffect", CLIENT_EFFECT);
		setObjVar(mob, WAYPOINT_LIST, wayPointList);
		setObjVar(mob, WAYPOINT_LOCS, wayPtLocs);
		
		pathTo(mob,getLocation(whoTriggeredMe));
		
		messageTo(mob, "cleanUpNpcTimer", null, 60, false);
		setObjVar(mob, "neverSpokenTo", true);
		
		CustomerServiceLog("outbreak_themepark", "rescue_trigger_volume.OnTriggerVolumeEntered() NPC spawned "+mob+" for Player: "+getPlayerName(whoTriggeredMe)+" ("+whoTriggeredMe+ "). This NPC is now ready to talk to the player");
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int OnTriggerVolumeExited(obj_id self, String volumeName, obj_id whoTriggeredMe) throws InterruptedException
	{
		return SCRIPT_CONTINUE;
	}
	
	
	public int getAllEscortPathNodes(obj_id self, dictionary params) throws InterruptedException
	{
		CustomerServiceLog("outbreak_themepark", "rescue_trigger_volume.getAllEscortPathNodes() the spawner "+self+" is attempting to get all path nodes for escort quests.");
		
		int spawner = getIntObjVar(self, "spawner");
		if (spawner < 0)
		{
			CustomerServiceLog("outbreak_themepark", "rescue_trigger_volume.getAllEscortPathNodes() the spawner "+self+" could not find the appropriate spawner integer objvar on self.");
			return SCRIPT_CONTINUE;
		}
		
		CustomerServiceLog("outbreak_themepark", "survivor_pathing.getAllEscortPathNodes() spawner: "+spawner);
		
		String creatureType = MOB_FOR_QUEST[spawner-1];
		if (creatureType == null || creatureType.length() <= 0)
		{
			CustomerServiceLog("outbreak_themepark", "survivor_pathing.getAllEscortPathNodes() the NPC creature_type was not valid");
			return SCRIPT_CONTINUE;
		}
		
		CustomerServiceLog("outbreak_themepark", "survivor_pathing.getAllEscortPathNodes() the NPC creature_type is: "+creatureType);
		
		obj_id[] wayPointList = getAllObjectsWithObjVar(getLocation(self), RADIUS, creatureType);
		if (wayPointList == null || wayPointList.length <= 0)
		{
			CustomerServiceLog("outbreak_themepark", "survivor_pathing.startSurvivorPathing() the NPC failed to find any objects within "+RADIUS+" of "+getLocation(self));
			return SCRIPT_CONTINUE;
		}
		
		location[] wayPtLocs = new location[wayPointList.length];
		
		for (int i = 0; i < wayPointList.length; i++)
		{
			testAbortScript();
			int orderNumber = getIntObjVar(wayPointList[i], creatureType) - 1;
			if (orderNumber < 0)
			{
				
				continue;
			}
			wayPtLocs[orderNumber] = getLocation(wayPointList[i]);
		}
		
		if (wayPtLocs == null || wayPtLocs.length <= 0)
		{
			CustomerServiceLog("outbreak_themepark", "survivor_pathing.startSurvivorPathing() unable to get a list of locations for hte NPC");
			return SCRIPT_CONTINUE;
		}
		setObjVar(self, WAYPOINT_LIST, wayPointList);
		setObjVar(self, WAYPOINT_LOCS, wayPtLocs);
		
		return SCRIPT_CONTINUE;
		
	}
	
}
