package script.theme_park.nym;

import script.*;
import script.base_class.*;
import script.combat_engine.*;
import java.util.Arrays;
import java.util.Hashtable;
import java.util.Vector;
import script.base_script;

import script.library.ai_lib;
import script.library.create;
import script.library.spawning;
import script.library.utils;


public class droid_cave extends script.base_script
{
	public droid_cave()
	{
	}
	public static final String NYM_OBJECT_DATATABLE = "datatables/spawning/theme_park/nym_droid_cave_objects.iff";
	public static final String SPAWNER_DATATABLE = "datatables/spawning/theme_park/nym_droid_cave.iff";
	
	
	public int OnInitialize(obj_id self) throws InterruptedException
	{
		messageTo(self, "spawnQuestObjects", null, 3, false);
		messageTo(self, "beginSpawnEnemies", null, 3, false);
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int spawnQuestObjects(obj_id self, dictionary params) throws InterruptedException
	{
		obj_id living = getCellId (self, "r28");
		if (!isValidId(living))
		{
			CustomerServiceLog("bad_spawner_data", "Nym Sulfur Pirate Cave spawner "+ self + ", droid_cave.spawnNonMobile handler could not find the cell named 'r28'");
			return SCRIPT_CONTINUE;
		}
		
		attachScript (living, "theme_park.nym.droid_cave_gate");
		
		obj_id foyer = getCellId (self, "r25");
		if (!isValidId(foyer))
		{
			CustomerServiceLog("bad_spawner_data", "Nym Sulfur Pirate Cave spawner "+ self + ", droid_cave.spawnNonMobile handler could not find the cell named 'r25'");
			return SCRIPT_CONTINUE;
		}
		
		location hackerLoc = new location (-62.52f, -65.80f, -142.43f, "lok", foyer);
		
		obj_id hacker = create.object("nym_themepark_slicer", hackerLoc);
		setYaw (hacker, 74);
		
		ai_lib.setCustomIdleAnimation(hacker, "manipulate_medium");
		
		if (!utils.hasScriptVar(self, "planet"))
		{
			location myself = getLocation(self);
			String planet = myself.area;
			if (planet == null || planet.length() <= 0)
			{
				CustomerServiceLog("bad_spawner_data", "Nym Sulfur Pirate Cave spawner "+ self + ", Could not retrieve planet name from location.");
				return SCRIPT_CONTINUE;
			}
			utils.setScriptVar(self, "planet", planet);
			
		}
		
		String planet = utils.getStringScriptVar(self, "planet");
		if (planet == null || planet.length() <= 0)
		{
			CustomerServiceLog("bad_spawner_data", "Nym Sulfur Pirate Cave spawner "+ self + ", Could not retrieve planet name script var.");
			return SCRIPT_CONTINUE;
		}
		
		spawning.spawnObjectsInDungeonFromTable(self, planet, NYM_OBJECT_DATATABLE);
		return SCRIPT_CONTINUE;
	}
	
	
	public int beginSpawnEnemies(obj_id self, dictionary params) throws InterruptedException
	{
		int numberOfCreaturesToSpawn = dataTableGetNumRows(SPAWNER_DATATABLE);
		if (numberOfCreaturesToSpawn < 0)
		{
			CustomerServiceLog("bad_spawner_data", "Nym Sulfur Pirate Cave spawner "+ self + ", droid_cave.beginSpawnEnemies handler could not open datatables/spawning/theme_park/nym_droid_cave.iff");
			return SCRIPT_CONTINUE;
		}
		
		int x = 0;
		if (utils.hasScriptVar(self, "spawnCounter"))
		{
			x = utils.getIntScriptVar (self, "spawnCounter");
		}
		
		String spawn = dataTableGetString (SPAWNER_DATATABLE, x, "spawns");
		
		float xCoord = dataTableGetFloat (SPAWNER_DATATABLE, x, "loc_x");
		float yCoord = dataTableGetFloat (SPAWNER_DATATABLE, x, "loc_y");
		float zCoord = dataTableGetFloat (SPAWNER_DATATABLE, x, "loc_z");
		
		if (!utils.hasScriptVar(self, "planet"))
		{
			location myself = getLocation(self);
			String planet = myself.area;
			if (planet == null || planet.length() <= 0)
			{
				CustomerServiceLog("bad_spawner_data", "Nym Sulfur Pirate Cave spawner "+ self + ", Could not retrieve planet Name from location.");
				return SCRIPT_CONTINUE;
			}
			utils.setScriptVar(self, "planet", planet);
			
		}
		
		String planet = utils.getStringScriptVar(self, "planet");
		if (planet == null || planet.length() <= 0)
		{
			CustomerServiceLog("bad_spawner_data", "Nym Sulfur Pirate Cave spawner "+ self + ", Could not retrieve planet name script var.");
			return SCRIPT_CONTINUE;
		}
		
		String spawnRoom = dataTableGetString (SPAWNER_DATATABLE, x, "room");
		obj_id room = getCellId (self, spawnRoom);
		
		if (!isIdValid (room) || (spawnRoom == null || spawnRoom.equals("")) && !spawnRoom.equals("world"))
		{
			CustomerServiceLog("bad_spawner_data", "spawner "+ self + ", datatable "+ SPAWNER_DATATABLE + ", row "+ x + ", mob "+ spawn + ", room "+ spawnRoom + " doesn't exist");
		}
		else
		{
			location spawnPoint = new location (xCoord, yCoord, zCoord, planet, room);
			
			obj_id spawnedCreature = create.object (spawn, spawnPoint);
			
			String geoScript = dataTableGetString (SPAWNER_DATATABLE, x, "script");
			
			if (geoScript != null && !geoScript.equals(""))
			{
				
				attachScript (spawnedCreature, geoScript);
			}
			
			String creatureName = dataTableGetString (SPAWNER_DATATABLE, x, "name");
			
			if (creatureName != null && !creatureName.equals(""))
			{
				setName (spawnedCreature, creatureName);
			}
			
			setObjVar (spawnedCreature, "spawn_number", x);
			setObjVar (spawnedCreature, "mom", self );
			utils.setScriptVar (self, "spawned"+ x, spawnedCreature);
		}
		
		x = x + 1;
		utils.setScriptVar (self, "spawnCounter", x);
		
		if (x < numberOfCreaturesToSpawn)
		{
			messageTo(self, "beginSpawnEnemies", null, 1, false );
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int tellingMomIDied(obj_id self, dictionary params) throws InterruptedException
	{
		LOG("nym_elite_log","droid cave NPC: "+self+" tellingMomIDied init");
		
		if (params == null)
		{
			CustomerServiceLog("bad_spawner_data", "Nym Pirate Cave Spawner Failed for "+self+" dungeon. Could not retrieve params from NPC messageTo.");
			return SCRIPT_CONTINUE;
		}
		
		int spawn_num = params.getInt ("spawnNumber");
		if (spawn_num <= 0)
		{
			CustomerServiceLog("bad_spawner_data", "Nym Pirate Cave Spawner Failed for "+self+" dungeon. Could not retrieve NPC spawnNum from Params.");
			return SCRIPT_CONTINUE;
		}
		
		obj_id spawn_mob = params.getObjId ("spawnMob");
		
		if (utils.hasScriptVar (self, "spawned" + spawn_num) && (spawn_mob == utils.getObjIdScriptVar (self, "spawned" + spawn_num)))
		{
			LOG("nym_elite_log","droid cave NPC: "+self+" spawning NPC correctly");
			
			doSpawn (spawn_num);
		}
		return SCRIPT_CONTINUE;
	}
	
	
	public void doSpawn(int spawn_num) throws InterruptedException
	{
		obj_id self = getSelf();
		if (!isValidId(self))
		{
			CustomerServiceLog("bad_spawner_data", "droid_cave.doSpawn - Nym Pirate Cave Spawner Failed.Could not retrieve Cave OID.");
			return;
		}
		
		int numberOfCreaturesToSpawn = dataTableGetNumRows(SPAWNER_DATATABLE);
		if (numberOfCreaturesToSpawn < 0)
		{
			CustomerServiceLog("bad_spawner_data", "Nym Sulfur Pirate Cave spawner "+ self + ", droid_cave.beginSpawnEnemies handler could not open datatables/spawning/theme_park/nym_droid_cave.iff");
			return;
		}
		
		int x = spawn_num;
		
		String spawn = dataTableGetString (SPAWNER_DATATABLE, x, "spawns");
		
		float xCoord = dataTableGetFloat (SPAWNER_DATATABLE, x, "loc_x");
		float yCoord = dataTableGetFloat (SPAWNER_DATATABLE, x, "loc_y");
		float zCoord = dataTableGetFloat (SPAWNER_DATATABLE, x, "loc_z");
		
		location myself = getLocation (self);
		String planet = myself.area;
		
		String spawnRoom = dataTableGetString (SPAWNER_DATATABLE, x, "room");
		obj_id room = getCellId (self, spawnRoom);
		
		if (!isIdValid (room) && !spawnRoom.equals("world"))
		{
			CustomerServiceLog("bad_spawner_data", "spawner "+ self + ", datatable "+ SPAWNER_DATATABLE + ", row "+ x + ", mob "+ spawn + ", room "+ spawnRoom + " doesn't exist");
			utils.removeScriptVar (self, "spawned"+ x);
			return;
		}
		
		location spawnPoint = new location (xCoord, yCoord, zCoord, planet, room);
		
		obj_id spawnedCreature = create.object (spawn, spawnPoint);
		
		String geoScript = dataTableGetString (SPAWNER_DATATABLE, x, "script");
		
		if (geoScript != null && !geoScript.equals(""))
		{
			
			attachScript (spawnedCreature, geoScript);
		}
		
		String creatureName = dataTableGetString (SPAWNER_DATATABLE, x, "name");
		
		if (creatureName != null && !creatureName.equals(""))
		{
			setName (spawnedCreature, creatureName);
		}
		
		setObjVar (spawnedCreature, "spawn_number", x);
		setObjVar (spawnedCreature, "mom", self );
		utils.setScriptVar (self, "spawned"+ x, spawnedCreature);
		
		return;
	}
}
