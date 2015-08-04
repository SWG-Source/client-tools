package script.theme_park.dungeon;

import script.*;
import script.base_class.*;
import script.combat_engine.*;
import java.util.Arrays;
import java.util.Hashtable;
import java.util.Vector;
import script.base_script;

import script.ai.ai;
import script.library.ai_lib;
import script.library.utils;
import script.library.create;
import script.library.holiday;


public class empire_day_interior_npc_spawner extends script.base_script
{
	public empire_day_interior_npc_spawner()
	{
	}
	
	public int OnAttach(obj_id self) throws InterruptedException
	{
		CustomerServiceLog("holidayEvent", "empire_day_interior_npc_spawner.OnAttach: Initialized.");
		
		messageTo (self, "beginEmpireDaySpawning", null, (holiday.EMPIRE_DAY_EVENT_START_DELAY + 30), false);
		return SCRIPT_CONTINUE;
	}
	
	
	public int OnInitialize(obj_id self) throws InterruptedException
	{
		CustomerServiceLog("holidayEvent", "empire_day_interior_npc_spawner.OnInitialize: Initialized.");
		
		messageTo (self, "beginEmpireDaySpawning", null, (holiday.EMPIRE_DAY_EVENT_START_DELAY + 30), false);
		return SCRIPT_CONTINUE;
	}
	
	
	public int beginEmpireDaySpawning(obj_id self, dictionary params) throws InterruptedException
	{
		if (!isEmpireDayRunning())
		{
			return SCRIPT_CONTINUE;
		}
		
		CustomerServiceLog("holidayEvent", "empire_day_interior_npc_spawner.beginEmpireDaySpawning: Initialized.");
		
		String datatable = getStringObjVar(self, "empire_day_spawn_table");
		if (datatable == null || datatable.length() <= 0)
		{
			CustomerServiceLog("holidayEvent", "empire_day_interior_npc_spawner.beginEmpireDaySpawning: The datatable objvar for the event spawning was not found.");
			return SCRIPT_CONTINUE;
		}
		
		CustomerServiceLog("holidayEvent", "empire_day_interior_npc_spawner.beginEmpireDaySpawning: datatable: "+datatable);
		
		int numberOfCreaturesToSpawn = dataTableGetNumRows(datatable);
		if (numberOfCreaturesToSpawn <= 0)
		{
			CustomerServiceLog("holidayEvent", "empire_day_interior_npc_spawner.beginEmpireDaySpawning: The datatable objvar was found but the table reference was corrupt or had no rows.");
			return SCRIPT_CONTINUE;
		}
		
		CustomerServiceLog("holidayEvent", "empire_day_interior_npc_spawner.beginEmpireDaySpawning: numberOfCreaturesToSpawn: "+numberOfCreaturesToSpawn);
		
		int x = utils.getIntScriptVar(self, "spawnCounter");
		CustomerServiceLog("holidayEvent", "empire_day_interior_npc_spawner.beginEmpireDaySpawning: spawnCounter: "+x);
		
		while (x < numberOfCreaturesToSpawn)
		{
			testAbortScript();
			
			if (hasObjVar (self, "empire_day_spawned" + x))
			{
				obj_id check = getObjIdObjVar (self, "empire_day_spawned"+ x);
				if (!check.isLoaded())
				{
					spawnCreatures(x, datatable, self );
				}
			}
			else
			{
				spawnCreatures(x, datatable, self );
			}
			x = x + 1;
		}
		
		if (dataTableHasColumn (datatable, "special_room"))
		{
			attachRoomScripts (self, datatable);
		}
		
		if (dataTableHasColumn (datatable, "room_objvar"))
		{
			setRoomObjVars (self, datatable);
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int tellingMomIDied(obj_id self, dictionary params) throws InterruptedException
	{
		int spawn_num = params.getInt ("spawnNumber");
		obj_id spawn_mob = params.getObjId ("spawnMob");
		
		if (hasObjVar (self, "empire_day_spawned" + spawn_num) && (spawn_mob == getObjIdObjVar (self, "empire_day_spawned" + spawn_num)))
		{
			String datatable = getStringObjVar (self, "empire_day_spawn_table");
			spawnCreatures(spawn_num, datatable, self );
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int randomCreatureDied(obj_id self, dictionary params) throws InterruptedException
	{
		return SCRIPT_CONTINUE;
	}
	
	
	public void attachRoomScripts(obj_id self, String datatable) throws InterruptedException
	{
		String[] roomsToLock = dataTableGetStringColumnNoDefaults (datatable, "special_room");
		int numRooms = roomsToLock.length;
		
		int passThrough = 0;
		
		while (passThrough < numRooms)
		{
			testAbortScript();
			String roomName = dataTableGetString (datatable, passThrough, "special_room");
			String roomScript = dataTableGetString (datatable, passThrough, "special_room_script");
			
			if (roomName == null && roomName.equals(""))
			{
				setObjVar (self, "problem", "No room name");
				return;
			}
			
			if (roomScript == null && roomScript.equals(""))
			{
				setObjVar (self, "problem", "No Script");
				return;
			}
			
			obj_id roomObj = self;
			
			if (roomName.equals("self"))
			{
				roomObj = self;
			}
			else
			{
				roomObj = getCellId (self, roomName);
			}
			
			attachScript (roomObj, roomScript);
			setObjVar (self, "set_room", passThrough);
			passThrough = passThrough + 1;
		}
		return;
	}
	
	
	public void setRoomObjVars(obj_id self, String datatable) throws InterruptedException
	{
		String[] roomsToSet = dataTableGetStringColumnNoDefaults (datatable, "room_objvar");
		int numRooms = roomsToSet.length;
		
		if (numRooms == 0)
		{
			return;
		}
		
		int passThrough = 0;
		
		while (passThrough < numRooms)
		{
			testAbortScript();
			String roomName = dataTableGetString (datatable, passThrough, "room_objvar");
			String roomObjVar = dataTableGetString (datatable, passThrough, "room_objvar_name");
			String roomObjVarValue = dataTableGetString (datatable, passThrough, "room_objvar_value");
			
			if (roomName == null && roomName.equals(""))
			{
				setObjVar (self, "problem", "No room name");
				return;
			}
			
			if (roomObjVar == null && roomObjVar.equals(""))
			{
				setObjVar (self, "problem", "No ObjVar Name");
				return;
			}
			
			obj_id roomObj = getCellId (self, roomName);
			
			setObjVar(roomObj, roomObjVar, roomObjVarValue);
			
			passThrough = passThrough + 1;
		}
		return;
	}
	
	
	public void spawnCreatures(int x, String datatable, obj_id self) throws InterruptedException
	{
		CustomerServiceLog("holidayEvent", "empire_day_interior_npc_spawner.spawnCreatures: Init");
		
		String spawn = dataTableGetString (datatable, x, "spawns");
		
		float xCoord = dataTableGetFloat (datatable, x, "loc_x");
		float yCoord = dataTableGetFloat (datatable, x, "loc_y");
		float zCoord = dataTableGetFloat (datatable, x, "loc_z");
		
		location myself = getLocation (self);
		CustomerServiceLog("holidayEvent", "empire_day_interior_npc_spawner.spawnCreatures: my location: "+myself);
		
		String planet = myself.area;
		String spawnRoom = dataTableGetString (datatable, x, "room");
		obj_id room = getCellId (self, spawnRoom);
		
		if (!isIdValid (room) && (!spawnRoom.equals("world")))
		{
			CustomerServiceLog("bad_spawner_data", "spawner "+ self + ", datatable "+ datatable + ", row "+ x + ", mob "+ spawn + ", room "+ spawnRoom + " doesn't exist");
			removeObjVar (self, "empire_day_spawned"+ x);
			
			return;
		}
		
		location spawnPoint = new location (xCoord, yCoord, zCoord, planet, room);
		
		obj_id spawnedCreature = create.object (spawn, spawnPoint);
		if (!isValidId(spawnedCreature) || !exists(spawnedCreature))
		{
			return;
		}
		
		CustomerServiceLog("holidayEvent", "empire_day_interior_npc_spawner.spawnCreatures: spawnedCreature: "+spawnedCreature);
		
		if (dataTableHasColumn (datatable, "convo_script"))
		{
			String convoScript = dataTableGetString (datatable, x, "convo_script");
			if (convoScript != null && !convoScript.equals(""))
			{
				attachScript (spawnedCreature, convoScript);
			}
		}
		
		String creatureName = dataTableGetString (datatable, x, "name");
		if (creatureName != null && !creatureName.equals(""))
		{
			setName (spawnedCreature, creatureName);
		}
		
		setObjVar (spawnedCreature, "spawn_number", x);
		setObjVar (spawnedCreature, "mom", self );
		setObjVar (self, "empire_day_spawned"+ x, spawnedCreature);
		attachScript (spawnedCreature, "theme_park.dungeon.clone_relic.spawned_creature_died");
		
		String scriptList = dataTableGetString (datatable, x, "script");
		
		if (scriptList != null && !scriptList.equals(""))
		{
			String[] scriptArray = split(scriptList, ',');
			
			for (int i = 0; i < scriptArray.length; i++)
			{
				testAbortScript();
				attachScript( spawnedCreature, scriptArray[i] );
			}
			
		}
		
		if (dataTableHasColumn (datatable, "yaw"))
		{
			float spawnYaw = dataTableGetFloat (datatable, x, "yaw");
			if (spawnYaw != 0)
			{
				setYaw (spawnedCreature, spawnYaw);
			}
		}
		
		if (dataTableHasColumn (datatable, "animation_mood"))
		{
			dictionary outparams = new dictionary ();
			outparams.put ("x", x );
			outparams.put ("datatable", datatable );
			outparams.put ("spawnedCreature", spawnedCreature );
			messageTo ( self, "animatedMood", outparams, 3, false );
		}
		
		if (dataTableHasColumn (datatable, "mood"))
		{
			String creatureMood = dataTableGetString (datatable, x, "mood");
			if (creatureMood != null && !creatureMood.equals(""))
			{
				ai_lib.setDefaultCalmMood (spawnedCreature, creatureMood);
			}
		}
		
		if (dataTableHasColumn (datatable, "spawn_objvar2"))
		{
			
			String spawnObjVar = dataTableGetString (datatable, x, "spawn_objvar");
			String objvarValue1 = dataTableGetString (datatable, x, "spawn_objvar_value");
			String spawnObjVar2 = dataTableGetString (datatable, x, "spawn_objvar2");
			String spawnValue2 = dataTableGetString (datatable, x, "spawn_objvar_value2");
			
			if (spawnObjVar != null && !spawnObjVar.equals(""))
			{
				if (objvarValue1 != null && !objvarValue1.equals(""))
				{
					setObjVar(spawnedCreature, spawnObjVar, objvarValue1);
				}
			}
			if (spawnObjVar2 != null && !spawnObjVar2.equals(""))
			{
				if (spawnValue2 != null && !spawnValue2.equals(""))
				{
					setObjVar(spawnedCreature, spawnObjVar2, spawnValue2);
				}
			}
		}
		
		else
		{
			String objVarList = dataTableGetString (datatable, x, "spawn_objvar");
			setCreatureObjVars ( spawnedCreature, objVarList );
		}
		return;
	}
	
	
	public int animatedMood(obj_id self, dictionary params) throws InterruptedException
	{
		int x = params.getInt( "x");
		String datatable = params.getString( "datatable");
		obj_id spawnedCreature = params.getObjId( "spawnedCreature");
		String creatureAnimationMood = dataTableGetString (datatable, x, "animation_mood");
		
		if (creatureAnimationMood != null && !creatureAnimationMood.equals(""))
		{
			ai_lib.setDefaultCalmMood (spawnedCreature, creatureAnimationMood);
		}
		
		if (creatureAnimationMood.equals("npc_sitting_drinking") || creatureAnimationMood.equals("npc_standing_drinking"))
		{
			obj_id cup = createObject ("object/tangible/item/con_drinking_glass_01.iff", spawnedCreature, "");
			equip(cup, spawnedCreature);
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public void setCreatureObjVars(obj_id creature, String objVarList) throws InterruptedException
	{
		if (objVarList == null || objVarList.equals(""))
		{
			return;
		}
		
		String[] pairs = split(objVarList, ',');
		for (int i = 0; i < pairs.length; i++)
		{
			testAbortScript();
			
			String[] objVarToSet = split(pairs[i], '=');
			String objVarValue = objVarToSet[1];
			
			String[] objVarNameAndType = split(objVarToSet[0], ':');
			String objVarType = objVarNameAndType[0];
			String objVarName = objVarNameAndType[1];
			
			if (objVarType.equals("string"))
			{
				setObjVar(creature, objVarName, objVarValue);
			}
			else if (objVarType.equals("int"))
			{
				setObjVar(creature, objVarName, utils.stringToInt( objVarValue));
			}
			else if (objVarType.equals("float"))
			{
				setObjVar(creature, objVarName, utils.stringToFloat( objVarValue));
			}
			else if (objVarType.equals("boolean") || objVarType.equals("bool"))
			{
				setObjVar(creature, objVarName, utils.stringToInt( objVarValue));
			}
			else
			{
				setObjVar(creature, objVarName, objVarValue);
			}
		}
	}
	
	
	public boolean isEmpireDayRunning() throws InterruptedException
	{
		CustomerServiceLog("holidayEvent", "empire_day_interior_npc_spawner.isEmpireDayRunning: Initializing check for building.");
		
		String empiredayRunning = getConfigSetting("GameServer", "empireday_ceremony");
		
		String empiredayString = getCurrentUniverseWideEvents();
		int empireday = empiredayString.indexOf("empireday_ceremony");
		if (empireday < 0)
		{
			CustomerServiceLog("holidayEvent", "empire_day_interior_npc_spawner.isEmpireDayRunning: event is NOT runing or starting.");
			if (empiredayRunning.equals("true") || empiredayRunning.equals("1"))
			{
				obj_id self = getSelf();
				CustomerServiceLog("holidayEvent", "empire_day_interior_npc_spawner.isEmpireDayRunning: There is a config that shows the event is running. Sending a message to later to see if we tried to spawn too soon.");
				messageTo (self, "beginEmpireDaySpawning", null, (holiday.EMPIRE_DAY_EVENT_START_DELAY + 300), false);
			}
			else
			{
				CustomerServiceLog("holidayEvent", "empire_day_interior_npc_spawner.isEmpireDayRunning: There is a config that shows the event is NOT running.");
				
			}
			return false;
		}
		CustomerServiceLog("holidayEvent", "empire_day_interior_npc_spawner.isEmpireDayRunning: event is runing or starting.");
		return true;
	}
}
