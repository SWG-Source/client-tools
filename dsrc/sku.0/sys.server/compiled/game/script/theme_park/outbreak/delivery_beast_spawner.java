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
import script.library.chat;
import script.library.create;
import script.library.groundquests;
import script.library.utils;


public class delivery_beast_spawner extends script.base_script
{
	public delivery_beast_spawner()
	{
	}
	public static final boolean LOGGING_ON = true;
	
	public static final String SCRIPT_LOG = "outbreak_delivery_trigger";
	public static final String CREATURE_NAME_OBJVAR = "creatureName";
	public static final String QUEST_NAME_OBJVAR = "questName";
	public static final String TASK_NAME_OBJVAR = "taskName";
	public static final String UPDATE_TASK_NAME_OBJVAR = "updateTaskName";
	public static final String FAIL_SIGNAL_NAME_OBJVAR = "failSignalName";
	public static final String ESCORT_TASK_NAME_OBJVAR = "escortTaskName";
	public static final String SUCCESS_SIGNAL_NAME_OBJVAR = "successSignal";
	public static final String TRIGGER_NAME_PREFIX = "camp_delivery_trigger_";
	public static final String CREATURE_TABLE = "datatables/mob/creatures.iff";
	public static final String MAX_LEVEL = "intMaxLevel";
	public static final String WAYPOINT_LIST = "wayPointList";
	public static final String WAYPOINT_LOCS = "wayPtLocs";
	public static final String COLLECTION_SLOT = "collectionSlot";
	
	public static final int WAIT_DELAY = 5;
	public static final float TRIGGER_RADIUS = 5f;
	public static final int RADIUS = 2000;
	
	
	public int OnInitialize(obj_id self) throws InterruptedException
	{
		CustomerServiceLog("outbreak_themepark", "delivery_beast_spawner.OnInitialize() initializing a spawner for camp defense.");
		
		if (!hasObjVar(self, CREATURE_NAME_OBJVAR))
		{
			CustomerServiceLog("outbreak_themepark", "delivery_beast_spawner.OnInitialize() initializing failed. The spawner, "+self+" had no creature name objvar. Location: "+getLocation(self));
			return SCRIPT_CONTINUE;
		}
		
		if (!hasObjVar(self, QUEST_NAME_OBJVAR))
		{
			CustomerServiceLog("outbreak_themepark", "delivery_beast_spawner.OnInitialize() initializing failed. The spawner, "+self+" had no camp name objvar. Location: "+getLocation(self));
			return SCRIPT_CONTINUE;
		}
		
		if (!hasObjVar(self, TASK_NAME_OBJVAR))
		{
			CustomerServiceLog("outbreak_themepark", "delivery_beast_spawner.OnInitialize() initializing failed. The spawner, "+self+" had no quest task name objvar. Location: "+getLocation(self));
			return SCRIPT_CONTINUE;
		}
		
		if (!hasObjVar(self, UPDATE_TASK_NAME_OBJVAR))
		{
			CustomerServiceLog("outbreak_themepark", "delivery_beast_spawner.OnInitialize() initializing failed. The spawner, "+self+" had no quest task name objvar. Location: "+getLocation(self));
			return SCRIPT_CONTINUE;
		}
		
		if (!hasObjVar(self, ESCORT_TASK_NAME_OBJVAR))
		{
			CustomerServiceLog("outbreak_themepark", "delivery_beast_spawner.OnInitialize() initializing failed. The spawner, "+self+" had no quest task name objvar. Location: "+getLocation(self));
			return SCRIPT_CONTINUE;
		}
		
		if (!hasObjVar(self, FAIL_SIGNAL_NAME_OBJVAR))
		{
			CustomerServiceLog("outbreak_themepark", "delivery_beast_spawner.OnInitialize() initializing failed. The spawner, "+self+" had no Fail Signal name objvar. Location: "+getLocation(self));
			return SCRIPT_CONTINUE;
		}
		if (!hasObjVar(self, SUCCESS_SIGNAL_NAME_OBJVAR))
		{
			CustomerServiceLog("outbreak_themepark", "delivery_beast_spawner.OnInitialize() initializing failed. The spawner, "+self+" had no Fail Signal name objvar. Location: "+getLocation(self));
			return SCRIPT_CONTINUE;
		}
		if (!hasObjVar(self, COLLECTION_SLOT))
		{
			CustomerServiceLog("outbreak_themepark", "delivery_beast_spawner.OnInitialize() initializing failed. The spawner, "+self+" had no Collection Slot name objvar. Location: "+getLocation(self));
			return SCRIPT_CONTINUE;
		}
		messageTo(self, "getAllEscortPathNodes", null, 5, false);
		messageTo(self, "spawnVolunteerNPC", null, 15, false);
		return SCRIPT_CONTINUE;
	}
	
	
	public int spawnVolunteerNPC(obj_id self, dictionary params) throws InterruptedException
	{
		CustomerServiceLog("outbreak_themepark", "camp_survivor_spawning.spawnVolunteerNPC() Spawner: "+self+" is attempting to spawn a delivery volunteer.");
		
		if (!hasObjVar(self, CREATURE_NAME_OBJVAR))
		{
			return SCRIPT_CONTINUE;
		}
		if (!hasObjVar(self, QUEST_NAME_OBJVAR))
		{
			return SCRIPT_CONTINUE;
		}
		if (!hasObjVar(self, TASK_NAME_OBJVAR))
		{
			return SCRIPT_CONTINUE;
		}
		if (!hasObjVar(self, UPDATE_TASK_NAME_OBJVAR))
		{
			return SCRIPT_CONTINUE;
		}
		if (!hasObjVar(self, ESCORT_TASK_NAME_OBJVAR))
		{
			return SCRIPT_CONTINUE;
		}
		if (!hasObjVar(self, FAIL_SIGNAL_NAME_OBJVAR))
		{
			return SCRIPT_CONTINUE;
		}
		if (!hasObjVar(self, SUCCESS_SIGNAL_NAME_OBJVAR))
		{
			return SCRIPT_CONTINUE;
		}
		if (!hasObjVar(self, COLLECTION_SLOT))
		{
			return SCRIPT_CONTINUE;
		}
		if (!hasObjVar(self, WAYPOINT_LIST))
		{
			CustomerServiceLog("outbreak_themepark", "camp_survivor_spawning.spawnVolunteerNPC() Spawner: "+self+" could not find the WAYPOINT LIST OF OBJVARS on self. Attempting to use failsafe in approx 15 seconds.");
			return SCRIPT_CONTINUE;
		}
		if (!hasObjVar(self, WAYPOINT_LOCS))
		{
			CustomerServiceLog("outbreak_themepark", "camp_survivor_spawning.spawnVolunteerNPC() Spawner: "+self+" could not find the WAYPOINT LIST OF LOCATIONS on self. Attempting to use failsafe in approx 15 seconds.");
			return SCRIPT_CONTINUE;
		}
		
		CustomerServiceLog("outbreak_themepark", "camp_survivor_spawning.spawnVolunteerNPC() Initial validation completed.");
		
		String questName = getStringObjVar(self, QUEST_NAME_OBJVAR);
		if (questName == null || questName.length() <= 0)
		{
			CustomerServiceLog("outbreak_themepark", "delivery_beast_spawner.spawnVolunteerNPC() trigger failed. The spawner, "+self+" had an INVALID quest name objvar. Location: "+getLocation(self));
			return SCRIPT_CONTINUE;
		}
		
		if (hasObjVar(self, "objChild"))
		{
			obj_id objChild = getObjIdObjVar(self,"objChild");
			if (isValidId(objChild) && exists(objChild))
			{
				CustomerServiceLog("outbreak_themepark", "camp_survivor_spawning.spawnVolunteerNPC() Spawner: "+self+" was not ready for a spawner. deliveryVolunteerReady var was none existant or false");
				return SCRIPT_CONTINUE;
			}
		}
		
		CustomerServiceLog("outbreak_themepark", "camp_survivor_spawning.spawnVolunteerNPC() No existing objChild for spawner: "+questName);
		
		String collectionSlot = getStringObjVar(self, COLLECTION_SLOT);
		if (collectionSlot == null || collectionSlot.length() <= 0)
		{
			CustomerServiceLog("outbreak_themepark", "delivery_beast_spawner.spawnVolunteerNPC() trigger failed. The spawner, "+self+" had an INVALID collectionSlot name objvar. Location: "+getLocation(self));
			return SCRIPT_CONTINUE;
		}
		
		String creatureName = getStringObjVar(self, CREATURE_NAME_OBJVAR);
		if (creatureName == null || creatureName.length() <= 0)
		{
			CustomerServiceLog("outbreak_themepark", "delivery_beast_spawner.spawnVolunteerNPC() trigger failed. The spawner, "+self+" had an INVALID creature name objvar. Location: "+getLocation(self));
			return SCRIPT_CONTINUE;
		}
		
		String taskName = getStringObjVar(self, TASK_NAME_OBJVAR);
		if (taskName == null || taskName.length() <= 0)
		{
			CustomerServiceLog("outbreak_themepark", "delivery_beast_spawner.spawnVolunteerNPC() trigger failed. The spawner, "+self+" had an INVALID task name objvar. Location: "+getLocation(self));
			return SCRIPT_CONTINUE;
		}
		
		String updateTaskName = getStringObjVar(self, UPDATE_TASK_NAME_OBJVAR);
		if (updateTaskName == null || updateTaskName.length() <= 0)
		{
			CustomerServiceLog("outbreak_themepark", "delivery_beast_spawner.spawnVolunteerNPC() trigger failed. The spawner, "+self+" had an INVALID updateTaskName objvar. Location: "+getLocation(self));
			return SCRIPT_CONTINUE;
		}
		
		String escortTask = getStringObjVar(self, ESCORT_TASK_NAME_OBJVAR);
		if (escortTask == null || escortTask.length() <= 0)
		{
			CustomerServiceLog("outbreak_themepark", "delivery_beast_spawner.spawnVolunteerNPC() trigger failed. The spawner, "+self+" had an INVALID escortTask objvar. Location: "+getLocation(self));
			return SCRIPT_CONTINUE;
		}
		
		String failSignalName = getStringObjVar(self, FAIL_SIGNAL_NAME_OBJVAR);
		if (failSignalName == null || failSignalName.length() <= 0)
		{
			CustomerServiceLog("outbreak_themepark", "delivery_beast_spawner.spawnVolunteerNPC() trigger failed. The spawner, "+self+" had an INVALID failSignalName objvar. Location: "+getLocation(self));
			return SCRIPT_CONTINUE;
		}
		
		String successSignalName = getStringObjVar(self, SUCCESS_SIGNAL_NAME_OBJVAR);
		if (successSignalName == null || successSignalName.length() <= 0)
		{
			CustomerServiceLog("outbreak_themepark", "delivery_beast_spawner.spawnVolunteerNPC() trigger failed. The spawner, "+self+" had an INVALID successSignalName objvar. Location: "+getLocation(self));
			return SCRIPT_CONTINUE;
		}
		
		int maxLevel = getIntObjVar(self, MAX_LEVEL);
		if (maxLevel <= 0)
		{
			CustomerServiceLog("outbreak_themepark", "delivery_beast_spawner.spawnVolunteerNPC() trigger failed. The spawner, "+self+" had an INVALID maxLevel objvar. Location: "+getLocation(self));
			return SCRIPT_CONTINUE;
		}
		
		CustomerServiceLog("outbreak_themepark", "camp_survivor_spawning.spawnVolunteerNPC() Variables created. Getting all waypoints for the NPC path");
		
		location[] wayPtLocs = getLocationArrayObjVar(self, WAYPOINT_LOCS);
		if (wayPtLocs == null || wayPtLocs.length <= 0)
		{
			CustomerServiceLog("outbreak_themepark", "beast_pathing.startBeastDeliveryPathing() Mob: "+self+" FAILED TO FIND wayPtLocs OBJVAR.");
			return SCRIPT_CONTINUE;
		}
		
		obj_id[] wayPointList = getObjIdArrayObjVar(self, WAYPOINT_LIST);
		if (wayPointList == null || wayPointList.length <= 0)
		{
			CustomerServiceLog("outbreak_themepark", "beast_pathing.startBeastDeliveryPathing() Mob: "+self+" FAILED TO FIND wayPointList OBJVAR.");
			return SCRIPT_CONTINUE;
		}
		
		obj_id mob = create.object(creatureName, getLocation(self));
		if (!isValidId(mob) || !exists(mob))
		{
			CustomerServiceLog("outbreak_themepark", "delivery_beast_spawner.spawnVolunteerNPC() Mob: "+mob+" could not be created!");
			return SCRIPT_CONTINUE;
		}
		CustomerServiceLog("outbreak_themepark", "camp_survivor_spawning.spawnSurvivor() Mob: "+mob+" WAS SUCCESSFULLY CREATED for spawner: "+self);
		setObjVar(self, "objChild", mob);
		
		setObjVar(mob, "objParent", self);
		setObjVar(mob, "maxLevel", maxLevel);
		setObjVar(mob, "ai.defaultCalmBehavior",1);
		setObjVar(mob, "questName", questName);
		setObjVar(mob, "escortTask", escortTask);
		setObjVar(mob, "failSignalName", failSignalName);
		setObjVar(mob, "successSignalName", successSignalName);
		setObjVar(mob, WAYPOINT_LIST, wayPointList);
		setObjVar(mob, WAYPOINT_LOCS, wayPtLocs);
		setObjVar(mob, COLLECTION_SLOT, collectionSlot);
		
		setAttributeAttained(mob, attrib.OUTBREAK_SURVIVOR);
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int getAllEscortPathNodes(obj_id self, dictionary params) throws InterruptedException
	{
		CustomerServiceLog("outbreak_themepark", "delivery_beast_spawner.getAllEscortPathNodes() the spawner "+self+" is attempting to get all path nodes for escort quests.");
		
		if (!hasObjVar(self, QUEST_NAME_OBJVAR))
		{
			CustomerServiceLog("outbreak_themepark", "delivery_beast_spawner.getAllEscortPathNodes() Failed. The spawner, "+self+" had no camp name objvar. Location: "+getLocation(self));
			return SCRIPT_CONTINUE;
		}
		
		String questName = getStringObjVar(self, QUEST_NAME_OBJVAR);
		if (questName == null || questName.length() <= 0)
		{
			CustomerServiceLog("outbreak_themepark", "delivery_beast_spawner.getAllEscortPathNodes() Failed. The spawner, "+self+" had an INVALID quest name objvar. Location: "+getLocation(self));
			return SCRIPT_CONTINUE;
		}
		
		obj_id[] wayPointList = getAllObjectsWithObjVar(getLocation(self), RADIUS, questName);
		if (wayPointList == null || wayPointList.length <= 0)
		{
			CustomerServiceLog("outbreak_themepark", "delivery_beast_spawner.getAllEscortPathNodes() the spawner "+self+" failed to find any node objects within "+RADIUS+" of "+getLocation(self));
			return SCRIPT_CONTINUE;
		}
		
		setObjVar(self, WAYPOINT_LIST, wayPointList);
		
		location[] wayPtLocs = new location[wayPointList.length];
		
		for (int i = 0; i < wayPointList.length; i++)
		{
			testAbortScript();
			int orderNumber = getIntObjVar(wayPointList[i], "pathNode") - 1;
			if (orderNumber < 0)
			{
				continue;
			}
			wayPtLocs[orderNumber] = getLocation(wayPointList[i]);
		}
		
		setObjVar(self, WAYPOINT_LOCS, wayPtLocs);
		
		return SCRIPT_CONTINUE;
		
	}
}
