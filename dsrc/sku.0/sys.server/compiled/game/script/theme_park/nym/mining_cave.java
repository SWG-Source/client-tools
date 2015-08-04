package script.theme_park.nym;

import script.*;
import script.base_class.*;
import script.combat_engine.*;
import java.util.Arrays;
import java.util.Hashtable;
import java.util.Vector;
import script.base_script;

import script.ai.ai;
import script.ai.ai_combat;
import script.library.ai_lib;
import script.library.create;
import script.library.spawning;
import script.library.utils;


public class mining_cave extends script.base_script
{
	public mining_cave()
	{
	}
	public static final String NYM_OBJECT_DATATABLE = "datatables/spawning/theme_park/nym_mining_cave_objects.iff";
	public static final String SPAWNER_DATATABLE = "datatables/spawning/theme_park/nym_imperial_mine.iff";
	
	
	public int OnInitialize(obj_id self) throws InterruptedException
	{
		messageTo(self, "spawnQuestObjects", null, 3, false);
		messageTo(self, "handleSpawns", null, 3, false);
		messageTo(self, "handleEliteSpawns", null, 3, false);
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int spawnQuestObjects(obj_id self, dictionary params) throws InterruptedException
	{
		if (!isValidId(self) || !exists(self))
		{
			return SCRIPT_CONTINUE;
		}
		
		if (!utils.hasScriptVar(self, "planet"))
		{
			location myself = getLocation(self);
			String planet = myself.area;
			if (planet == null || planet.length() <= 0)
			{
				CustomerServiceLog("bad_spawner_data", "Nym Sulfur Pirate Cave spawner "+ self + ", Could not retrieve planet name from location.");
				return SCRIPT_OVERRIDE;
			}
			utils.setScriptVar(self, "planet", planet);
			
		}
		
		String planet = utils.getStringScriptVar(self, "planet");
		if (planet == null || planet.length() <= 0)
		{
			CustomerServiceLog("bad_spawner_data", "Nym Sulfur Pirate Cave spawner "+ self + ", Could not retrieve planet name script var.");
			return SCRIPT_OVERRIDE;
		}
		
		spawning.spawnObjectsInDungeonFromTable(self, planet, NYM_OBJECT_DATATABLE);
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int handleSpawns(obj_id self, dictionary params) throws InterruptedException
	{
		if (!isValidId(self) || !exists(self))
		{
			return SCRIPT_CONTINUE;
		}
		
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
		
		int numberOfCreaturesToSpawn = dataTableGetNumRows(SPAWNER_DATATABLE);
		if (numberOfCreaturesToSpawn <= 0)
		{
			return SCRIPT_CONTINUE;
		}
		
		for (int i = 0; i < numberOfCreaturesToSpawn; i++)
		{
			testAbortScript();
			doSpawn(i, planet, self);
		}
		return SCRIPT_CONTINUE;
	}
	
	
	public int tellingMomIDied(obj_id self, dictionary params) throws InterruptedException
	{
		LOG("nym_elite_log","NPC: "+self+" tellingMomIDied init");
		
		if (!isValidId(self) || !exists(self))
		{
			return SCRIPT_CONTINUE;
		}
		
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
		
		int spawn_num = params.getInt("spawnNumber");
		if (spawn_num < 0)
		{
			return SCRIPT_CONTINUE;
		}
		
		obj_id spawn_mob = params.getObjId("spawnMob");
		
		if (utils.hasScriptVar (self, "spawned" + spawn_num) && (spawn_mob == utils.getObjIdScriptVar (self, "spawned" + spawn_num)))
		{
			LOG("nym_elite_log","NPC: "+self+" spawning NPC correctly");
			
			doSpawn(spawn_num, planet, self);
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public boolean doSpawn(int spawn_num, String planet, obj_id self) throws InterruptedException
	{
		if (spawn_num < 0)
		{
			return false;
		}
		
		if (planet == null || planet.length() <= 0)
		{
			return false;
		}
		
		if (!isValidId(self) || !exists(self))
		{
			return false;
		}
		
		dictionary objToSpawn = dataTableGetRow(SPAWNER_DATATABLE, spawn_num);
		if (objToSpawn == null)
		{
			return false;
		}
		
		String object = objToSpawn.getString("spawns");
		if (object == null || object.length() <= 0)
		{
			return false;
		}
		
		float xCoord = objToSpawn.getFloat("loc_x");
		float yCoord = objToSpawn.getFloat("loc_y");
		float zCoord = objToSpawn.getFloat("loc_z");
		float yaw = objToSpawn.getFloat("yaw");
		
		String spawnRoom = objToSpawn.getString("room");
		if (spawnRoom == null || spawnRoom.length() <= 0)
		{
			CustomerServiceLog("bad_spawner_data", "mining_cave - doSpawn The creature "+object+" could not be spawned in room: "+spawnRoom+" in dungeon: "+self);
			return false;
		}
		
		obj_id room = getCellId(self, spawnRoom);
		if (!isValidId(room))
		{
			CustomerServiceLog("bad_spawner_data", "mining_cave - doSpawn The creature "+object+" could not be spawned in room: "+spawnRoom+" because the OID for that room was invalid. Dungeon: "+self);
			return false;
		}
		
		location objectLocation = new location (xCoord, yCoord, zCoord, planet, room);
		
		obj_id objectCreated = create.object (object, objectLocation);
		if (!isValidId(objectCreated))
		{
			CustomerServiceLog("bad_spawner_data", "mining_cave - doSpawn The creature "+object+" could not be spawned in room: "+spawnRoom+" because the creature was invalid or location wrong. Dungeon: "+self);
			return false;
		}
		
		String script = objToSpawn.getString("script");
		if (script != null && script.length() > 0)
		{
			
			String[] scripts = split(script, ',');
			for (int j = 0; j < scripts.length; j++)
			{
				testAbortScript();
				if (!hasScript(objectCreated, scripts[j]))
				{
					attachScript(objectCreated, scripts[j]);
				}
			}
		}
		
		String objVars = objToSpawn.getString("objvar");
		if (objVars != null && objVars.length() > 0)
		{
			utils.setObjVarsListUsingSemiColon(objectCreated, objVars);
		}
		
		String objName = objToSpawn.getString("name");
		if (objName != null && objName.length() > 0)
		{
			setName(objectCreated, objName);
		}
		
		setObjVar(objectCreated, "spawn_number", spawn_num);
		setObjVar(objectCreated, "mom", self );
		utils.setScriptVar (self, "spawned"+ spawn_num, objectCreated);
		
		return true;
	}
	
}
