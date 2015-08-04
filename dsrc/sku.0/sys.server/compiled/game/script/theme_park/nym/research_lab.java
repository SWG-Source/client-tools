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


public class research_lab extends script.base_script
{
	public research_lab()
	{
	}
	public static final String NYM_OBJECT_DATATABLE = "datatables/spawning/theme_park/nym_research_lab_objects.iff";
	public static final String SPAWNER_DATATABLE = "datatables/spawning/theme_park/nym_research_lab.iff";
	
	
	public int OnInitialize(obj_id self) throws InterruptedException
	{
		messageTo(self, "spawnQuestObjects", null, 3, false);
		messageTo(self, "spawnOtherObjects", null, 3, false);
		messageTo(self, "beginSpawn", null, 3, false);
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
	
	
	public int beginSpawn(obj_id self, dictionary params) throws InterruptedException
	{
		if (!isValidId(self) || !exists(self))
		{
			return SCRIPT_CONTINUE;
		}
		
		int numberOfCreaturesToSpawn = dataTableGetNumRows (SPAWNER_DATATABLE);
		if (numberOfCreaturesToSpawn < 0)
		{
			CustomerServiceLog("bad_spawner_data", "Nym Research Lab spawner "+ self + ", research_lab.beginSpawn handler could not open datatable.");
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
		
		location myself = getLocation (self);
		String planet = myself.area;
		
		String spawnRoom = dataTableGetString (SPAWNER_DATATABLE, x, "room");
		obj_id room = null;
		
		if (spawnRoom != null && spawnRoom.length() > 0)
		{
			room = getCellId (self, spawnRoom);
		}
		else
		{
			spawnRoom = "";
		}
		
		if (!isIdValid (room) && !spawnRoom.equals("world"))
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
			String objVars = dataTableGetString (SPAWNER_DATATABLE, x, "objvar");
			if (objVars != null && objVars.length() > 0)
			{
				utils.setObjVarsListUsingSemiColon(spawnedCreature, objVars);
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
		
		if (x <= numberOfCreaturesToSpawn)
		{
			messageTo ( self, "beginSpawn", null, 1, false );
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int tellingMomIDied(obj_id self, dictionary params) throws InterruptedException
	{
		
		int spawn_num = params.getInt ("spawnNumber");
		obj_id spawn_mob = params.getObjId ("spawnMob");
		
		if (utils.hasScriptVar (self, "spawned" + spawn_num) && (spawn_mob == utils.getObjIdScriptVar (self, "spawned" + spawn_num)))
		{
			doSpawn (spawn_num);
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public void doSpawn(int spawn_num) throws InterruptedException
	{
		obj_id self = getSelf();
		
		int numberOfCreaturesToSpawn = dataTableGetNumRows (SPAWNER_DATATABLE);
		if (numberOfCreaturesToSpawn < 0)
		{
			CustomerServiceLog("bad_spawner_data", "Nym Research Lab spawner "+ self + ", research_lab.beginSpawn handler could not open datatable.");
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
