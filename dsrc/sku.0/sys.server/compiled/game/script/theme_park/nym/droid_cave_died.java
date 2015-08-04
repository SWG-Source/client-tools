package script.theme_park.nym;

import script.*;
import script.base_class.*;
import script.combat_engine.*;
import java.util.Arrays;
import java.util.Hashtable;
import java.util.Vector;
import script.base_script;


public class droid_cave_died extends script.base_script
{
	public droid_cave_died()
	{
	}
	
	public int OnDestroy(obj_id self) throws InterruptedException
	{
		obj_id mom = getObjIdObjVar (self, "mom");
		if (!isValidId(mom))
		{
			CustomerServiceLog("bad_spawner_data", "droid_cave_died.OnIncapacitated() Nym Pirate Cave Spawner Failed. Could not retrieve Cave OID to respawn the NPC.");
			return SCRIPT_CONTINUE;
		}
		
		int spawnNum = getIntObjVar (self, "spawn_number");
		if (spawnNum <= 0)
		{
			CustomerServiceLog("bad_spawner_data", "droid_cave_died.OnIncapacitated() Nym Pirate Cave Spawner Failed. Could not retrieve NPC spawnNum from NPC Object.");
			return SCRIPT_CONTINUE;
		}
		
		dictionary info = new dictionary();
		info.put ("spawnNumber", spawnNum );
		info.put ("spawnMob", self );
		
		int respawn = rand(120,200);
		
		messageTo(mom, "tellingMomIDied", info, respawn, false );
		return SCRIPT_CONTINUE;
	}
}
