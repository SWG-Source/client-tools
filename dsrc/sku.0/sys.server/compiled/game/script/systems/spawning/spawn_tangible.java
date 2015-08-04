package script.systems.spawning;

import script.*;
import script.base_class.*;
import script.combat_engine.*;
import java.util.Arrays;
import java.util.Hashtable;
import java.util.Vector;
import script.base_script;


public class spawn_tangible extends script.base_script
{
	public spawn_tangible()
	{
	}
	
	public int OnInitialize(obj_id self) throws InterruptedException
	{
		requestPreloadCompleteTrigger(self);
		return SCRIPT_CONTINUE;
	}
	
	
	public int OnPreloadComplete(obj_id self) throws InterruptedException
	{
		if (!hasObjVar(self, "spawn.object"))
		{
			return SCRIPT_CONTINUE;
		}
		
		LOG("bomb_spawner","OnPreloadComplete");
		
		if (hasObjVar(self, "spawn.oid"))
		{
			obj_id currentObject = getObjIdObjVar(self, "spawn.oid");
			if (!isValidId(currentObject) || !exists(currentObject))
			{
				removeObjVar(self, "spawn.oid");
			}
			else
			{
				return SCRIPT_CONTINUE;
			}
		}
		
		String objectName = getStringObjVar(self, "spawn.object");
		if (objectName == null || objectName.equals(""))
		{
			return SCRIPT_CONTINUE;
		}
		
		location spawnerLocation = getLocation(self);
		if (spawnerLocation == null)
		{
			return SCRIPT_CONTINUE;
		}
		
		obj_id spawnedObject = createObject(objectName, spawnerLocation);
		if (!isValidId(spawnedObject) || !exists(spawnedObject))
		{
			return SCRIPT_CONTINUE;
		}
		
		setObjVar(self, "spawn.oid", spawnedObject);
		setObjVar(spawnedObject, "spawn.spawner", self);
		
		LOG("bomb_spawner","spawnedObject: "+spawnedObject);
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int handleTangibleRespawn(obj_id self, dictionary params) throws InterruptedException
	{
		if (!hasObjVar(self, "spawn.object"))
		{
			return SCRIPT_CONTINUE;
		}
		
		LOG("bomb_spawner","handleTangibleRespawn");
		
		if (hasObjVar(self, "spawn.oid"))
		{
			obj_id currentObject = getObjIdObjVar(self, "spawn.oid");
			LOG("bomb_spawner","found old oid: "+currentObject);
			
			if (!isValidId(currentObject) || !exists(currentObject))
			{
				LOG("bomb_spawner","removing Oid: "+currentObject);
				removeObjVar(self, "spawn.oid");
			}
			else
			{
				return SCRIPT_CONTINUE;
			}
		}
		
		String objectName = getStringObjVar(self, "spawn.object");
		if (objectName == null || objectName.equals(""))
		{
			return SCRIPT_CONTINUE;
		}
		
		location spawnerLocation = getLocation(self);
		if (spawnerLocation == null)
		{
			return SCRIPT_CONTINUE;
		}
		
		obj_id spawnedObject = createObject(objectName, spawnerLocation);
		if (!isValidId(spawnedObject) || !exists(spawnedObject))
		{
			return SCRIPT_CONTINUE;
		}
		
		setObjVar(self, "spawn.oid", spawnedObject);
		setObjVar(spawnedObject, "spawn.spawner", self);
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int handleTangibleDestroy(obj_id self, dictionary params) throws InterruptedException
	{
		if (!hasObjVar(self, "spawn.oid"))
		{
			return SCRIPT_CONTINUE;
		}
		
		obj_id spawnedObject = getObjIdObjVar(self, "spawn.oid");
		if (!isValidId(spawnedObject) || !exists(spawnedObject))
		{
			return SCRIPT_CONTINUE;
		}
		
		destroyObject(spawnedObject);
		removeObjVar(self, "spawn.oid");
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int OnRemovingFromWorld(obj_id self) throws InterruptedException
	{
		if (!hasObjVar(self, "spawn.oid"))
		{
			return SCRIPT_CONTINUE;
		}
		
		obj_id spawnedObject = getObjIdObjVar(self, "spawn.oid");
		if (!isValidId(spawnedObject) || !exists(spawnedObject))
		{
			return SCRIPT_CONTINUE;
		}
		
		destroyObject(spawnedObject);
		removeObjVar(self, "spawn.oid");
		
		return SCRIPT_CONTINUE;
	}
}
