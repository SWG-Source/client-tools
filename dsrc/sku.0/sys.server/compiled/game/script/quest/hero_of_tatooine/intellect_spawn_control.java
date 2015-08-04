package script.quest.hero_of_tatooine;

import script.*;
import script.base_class.*;
import script.combat_engine.*;
import java.util.Arrays;
import java.util.Hashtable;
import java.util.Vector;
import script.base_script;

import script.library.utils;


public class intellect_spawn_control extends script.base_script
{
	public intellect_spawn_control()
	{
	}
	public static final String SPAWNER_DATATABLE = "datatables/quest/hero_of_tatooine/intellect_spawner.iff";
	public static final String SPAWNER_OBJVAR = "quest.hero_of_tatooine.intellect.spawner";
	public static final String SPAWNER_IDS = SPAWNER_OBJVAR + ".ids";
	public static final String SPAWNER_STATUS = SPAWNER_OBJVAR + ".status";
	public static final String SPAWNER_SPAWN_MODE = SPAWNER_OBJVAR + ".spawning";
	
	public static final float VALIDATION_CHECK_TIME = 180.0f;
	public static final float SPAWN_TIME = 1800.0f;
	public static final float REVALIDATE_TIME = 10800.0f;
	public static final float SPAWN_CHECK_TIME = SPAWN_TIME + VALIDATION_CHECK_TIME;
	
	
	public int OnAttach(obj_id self) throws InterruptedException
	{
		if (!hasObjVar(self, SPAWNER_OBJVAR))
		{
			setupSpawnerObjVars(self);
		}
		
		messageTo(self, "handleForcedValidation", null, 600.0f, false);
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int OnInitialize(obj_id self) throws InterruptedException
	{
		
		if (!hasObjVar(self, SPAWNER_OBJVAR))
		{
			setupSpawnerObjVars(self);
		}
		
		CustomerServiceLog("quest", "HERO OF TATOOINE - Initializing Mark of Intellect spawn control");
		
		messageTo(self, "handleForcedValidation", null, 600.0f, false);
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int OnHearSpeech(obj_id self, obj_id speaker, String text) throws InterruptedException
	{
		
		if (!isGod(speaker))
		{
			return SCRIPT_CONTINUE;
		}
		
		if (toLower(text).equals("force_intellect_spawn"))
		{
			
			sendSystemMessageTestingOnly(speaker, "Beginning spawn sequence. It could take a few minutes...");
			startSpawnSequence(self, 0.0f, true);
			
		}
		else if (toLower(text).equals("find_intellect_spawn"))
		{
			
			sendSystemMessageTestingOnly(speaker, "Requesting spawn location information...");
			findSpawned(self, speaker);
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int handleValidation(obj_id self, dictionary params) throws InterruptedException
	{
		
		int spawn = params.getInt("spawn");
		int number = params.getInt("number");
		
		int[] status = getIntArrayObjVar(self, SPAWNER_STATUS);
		
		if (status == null || status.length == 0)
		{
			return SCRIPT_CONTINUE;
		}
		
		if (number >= status.length)
		{
			return SCRIPT_CONTINUE;
		}
		
		if (status[number] != spawn)
		{
			
			status[number] = spawn;
			setObjVar(self, SPAWNER_STATUS, status);
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int handleSingleValidationCheck(obj_id self, dictionary params) throws InterruptedException
	{
		
		if (countActiveSpawns(self) == 0)
		{
			validateAllSpawners(self);
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int handleAllValidationCheck(obj_id self, dictionary params) throws InterruptedException
	{
		
		if (countActiveSpawns(self) == 0)
		{
			startSpawnSequence(self, SPAWN_TIME);
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int handleSpawnValidation(obj_id self, dictionary params) throws InterruptedException
	{
		
		float time = params.getFloat("time");
		
		if (hasObjVar(self, SPAWNER_SPAWN_MODE))
		{
			removeObjVar(self, SPAWNER_SPAWN_MODE);
		}
		
		if (countActiveSpawns(self) == 0)
		{
			startSpawnSequence(self, time);
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int handleCompletion(obj_id self, dictionary params) throws InterruptedException
	{
		
		int number = params.getInt("number");
		
		int[] status = getIntArrayObjVar(self, SPAWNER_STATUS);
		
		if (status == null || status.length == 0)
		{
			
			CustomerServiceLog("quest", "HERO OF TATOOINE - Intellect spawn control failing; Spawner status not available");
			return SCRIPT_CONTINUE;
		}
		
		status[number] = 0;
		
		setObjVar(self, SPAWNER_STATUS, status);
		
		validateSpawners(self);
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int handleLocationResponse(obj_id self, dictionary params) throws InterruptedException
	{
		
		obj_id seeker = params.getObjId("seeker");
		location loc = params.getLocation("loc");
		
		sendSystemMessageTestingOnly(seeker, "Intellect Spawn: "+ loc.toString());
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int handleForcedValidation(obj_id self, dictionary params) throws InterruptedException
	{
		
		CustomerServiceLog("quest", "HERO OF TATOOINE - Forcing Mark of Intellect validation");
		
		validateAllSpawners(self);
		
		messageTo(self, "handleForcedValidation", null, REVALIDATE_TIME, false);
		
		return SCRIPT_CONTINUE;
	}
	
	
	public void setupSpawnerObjVars(obj_id self) throws InterruptedException
	{
		
		int numRows = dataTableGetNumRows(SPAWNER_DATATABLE);
		obj_id[] ids = new obj_id[numRows];
		int[] status = new int[numRows];
		
		for (int i = 0; i < numRows; i++)
		{
			testAbortScript();
			String strObjId = dataTableGetString(SPAWNER_DATATABLE, i, 0);
			obj_id spawner = utils.stringToObjId(strObjId);
			
			ids[i] = spawner;
			status[i] = 0;
		}
		
		if (ids != null && ids.length > 0 && status != null && status.length > 0)
		{
			setObjVar(self, SPAWNER_IDS, ids);
			setObjVar(self, SPAWNER_STATUS, status);
		}
	}
	
	
	public void validateSpawners(obj_id self) throws InterruptedException
	{
		
		if (countActiveSpawns(self) == 0)
		{
			
			validateAllSpawners(self);
			return;
		}
		
		obj_id[] ids = getObjIdArrayObjVar(self, SPAWNER_IDS);
		int[] status = getIntArrayObjVar(self, SPAWNER_STATUS);
		
		dictionary d = new dictionary();
		d.put("controller", self);
		
		for (int i = 0; i < ids.length; i++)
		{
			testAbortScript();
			
			if (status[i] == 1)
			{
				
				d.put("number", i);
				messageTo(ids[i], "handleValidate", d, 1.0f, true);
			}
		}
		
		messageTo(self, "handleSingleValidationCheck", null, VALIDATION_CHECK_TIME, true);
	}
	
	
	public void validateAllSpawners(obj_id self) throws InterruptedException
	{
		
		obj_id[] ids = getObjIdArrayObjVar(self, SPAWNER_IDS);
		int[] status = getIntArrayObjVar(self, SPAWNER_STATUS);
		
		dictionary d = new dictionary();
		d.put("controller", self);
		
		for (int i = 0; i < ids.length; i++)
		{
			testAbortScript();
			
			d.put("number", i);
			messageTo(ids[i], "handleValidate", d, 1.0f, true);
		}
		
		messageTo(self, "handleAllValidationCheck", null, VALIDATION_CHECK_TIME, true);
	}
	
	
	public void startSpawnSequence(obj_id self, float time, boolean override) throws InterruptedException
	{
		
		if (hasObjVar(self, SPAWNER_SPAWN_MODE) && !override)
		{
			
			int spawn_time = getIntObjVar(self, SPAWNER_SPAWN_MODE);
			int game_time = getGameTime();
			int difference_time = (game_time - spawn_time);
			
			if (difference_time >= 0 && difference_time < 10800)
			{
				CustomerServiceLog("quest", "HERO OF TATOOINE - Intellect spawn control failing; Trying to spawn while currently in spawn mode. Spawn Time: "+spawn_time+" Game Time: "+game_time+". Difference: "+difference_time);
				return;
			}
			else
			{
				CustomerServiceLog("quest", "HERO OF TATOOINE - Intellect autospawner attempting to spawn. Possibly for the first time. Spawn Time: "+spawn_time+" Game Time: "+game_time+". Difference: "+difference_time);
			}
		}
		
		obj_id[] ids = getObjIdArrayObjVar(self, SPAWNER_IDS);
		if (ids == null)
		{
			CustomerServiceLog("quest", "HERO OF TATOOINE - Intellect autospawner cannot find a list of spawner IDs. FAILING.");
			return;
		}
		
		int randSpawn = rand(0, (ids.length - 1));
		
		dictionary d = new dictionary();
		d.put("controller", self);
		d.put("number", randSpawn);
		d.put("time", time);
		
		setObjVar(self, SPAWNER_SPAWN_MODE, getGameTime());
		
		float check_time = time + VALIDATION_CHECK_TIME;
		
		CustomerServiceLog("quest", "HERO OF TATOOINE - Starting Mark of Intellect spawn sequence");
		
		messageTo(ids[randSpawn], "handleSpawn", d, time, true);
		messageTo(self, "handleSpawnValidation", d, check_time, true);
	}
	
	
	public void startSpawnSequence(obj_id self, float time) throws InterruptedException
	{
		
		startSpawnSequence(self, time, false);
	}
	
	
	public void findSpawned(obj_id self, obj_id speaker) throws InterruptedException
	{
		
		obj_id[] ids = getObjIdArrayObjVar(self, SPAWNER_IDS);
		int[] status = getIntArrayObjVar(self, SPAWNER_STATUS);
		
		dictionary d = new dictionary();
		d.put("seeker", speaker);
		d.put("controller", self);
		
		boolean found = false;
		
		for (int i = 0; i < status.length; i++)
		{
			testAbortScript();
			if (status[i] == 1)
			{
				messageTo(ids[i], "handleLocationRequest", d, 1.0f, true);
				found = true;
			}
		}
		
		if (!found)
		{
			sendSystemMessageTestingOnly(speaker, "No active spawns found");
		}
	}
	
	
	public int countActiveSpawns(obj_id self) throws InterruptedException
	{
		
		int[] status = getIntArrayObjVar(self, SPAWNER_STATUS);
		
		if (status == null || status.length == 0)
		{
			return -1;
		}
		
		int count = 0;
		
		for (int i = 0; i < status.length; i++)
		{
			testAbortScript();
			
			if (status[i] == 1)
			{
				count++;
			}
		}
		
		return count;
	}
}
