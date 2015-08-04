package script.systems.gcw;

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
import script.library.holiday;
import script.library.utils;


public class gcw_poi_guard_spawning extends script.base_script
{
	public gcw_poi_guard_spawning()
	{
	}
	
	public int OnAttach(obj_id self) throws InterruptedException
	{
		messageTo(self, "spawnSentry", null, 10, false);
		return SCRIPT_CONTINUE;
	}
	
	
	public int OnDestroy(obj_id self) throws InterruptedException
	{
		if (!utils.hasScriptVar(self, "child"))
		{
			return SCRIPT_CONTINUE;
		}
		
		obj_id child = utils.getObjIdScriptVar(self, "child");
		if (!isValidId(child) || !exists(child))
		{
			return SCRIPT_CONTINUE;
		}
		
		destroyObject(child);
		return SCRIPT_CONTINUE;
	}
	
	
	public int spawnSentry(obj_id self, dictionary params) throws InterruptedException
	{
		LOG("guard_spawning","spawnSentry init");
		if (hasObjVar(self, "mob"))
		{
			messageTo(self, "spawnSentry", null, 10, false);
			return SCRIPT_CONTINUE;
		}
		LOG("guard_spawning","spawnSentry has mob objvar");
		
		String creatureType = getStringObjVar(self, "mob");
		if (creatureType == null || creatureType.length() <= 0)
		{
			return SCRIPT_CONTINUE;
		}
		LOG("guard_spawning","spawnSentry creatureType: "+creatureType);
		
		obj_id mob = create.object(creatureType, getLocation(self));
		if (!isValidId(mob) || !exists(mob))
		{
			return SCRIPT_CONTINUE;
		}
		LOG("guard_spawning","spawnSentry mob: "+mob);
		
		CustomerServiceLog("outbreak_themepark", "camp_survivor_spawning.spawnSurvivor() Mob: "+mob+" WAS SUCCESSFULLY CREATED for spawner: "+self);
		
		setObjVar(mob, "mySpawner", self);
		utils.setScriptVar(self, "child", mob);
		return SCRIPT_CONTINUE;
	}
}
