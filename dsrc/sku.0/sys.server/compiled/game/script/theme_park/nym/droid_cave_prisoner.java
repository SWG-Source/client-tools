package script.theme_park.nym;

import script.*;
import script.base_class.*;
import script.combat_engine.*;
import java.util.Arrays;
import java.util.Hashtable;
import java.util.Vector;
import script.base_script;

import script.library.groundquests;
import script.library.utils;


public class droid_cave_prisoner extends script.base_script
{
	public droid_cave_prisoner()
	{
	}
	
	public int OnIncapacitated(obj_id self, obj_id killer) throws InterruptedException
	{
		failRescueQuest(self);
		tellMomIDied(self);
		return SCRIPT_CONTINUE;
	}
	
	
	public int handleUpdatePrisonerRescueQuest(obj_id self, dictionary params) throws InterruptedException
	{
		obj_id player = getObjIdObjVar(self, "player");
		if (!isValidId(player))
		{
			messageTo(self, "handleDestroySelf", null, 1, false);
			return SCRIPT_CONTINUE;
		}
		
		String signal = getStringObjVar(self, "signal");
		if (signal == null || signal.length() <= 0)
		{
			messageTo(self, "handleDestroySelf", null, 1, false);
			return SCRIPT_CONTINUE;
		}
		
		groundquests.sendSignal(player, signal);
		messageTo(self, "handleDestroySelf", null, 1, false);
		return SCRIPT_CONTINUE;
	}
	
	
	public int handleDestroySelf(obj_id self, dictionary params) throws InterruptedException
	{
		tellMomIDied(self);
		destroyObject(self);
		return SCRIPT_CONTINUE;
	}
	
	
	public boolean tellMomIDied(obj_id self) throws InterruptedException
	{
		if (!isValidId(self))
		{
			return false;
		}
		
		obj_id mom = getObjIdObjVar (self, "mom");
		if (!isValidId(mom))
		{
			CustomerServiceLog("bad_spawner_data", "droid_cave_died.OnIncapacitated() Nym Pirate Cave Spawner Failed. Could not retrieve Cave OID to respawn the NPC.");
			return false;
		}
		
		int spawnNum = getIntObjVar (self, "spawn_number");
		if (spawnNum <= 0)
		{
			CustomerServiceLog("bad_spawner_data", "droid_cave_died.OnIncapacitated() Nym Pirate Cave Spawner Failed. Could not retrieve NPC spawnNum from NPC Object.");
			return false;
		}
		
		dictionary info = new dictionary();
		info.put ("spawnNumber", spawnNum );
		info.put ("spawnMob", self );
		
		messageTo(mom, "tellingMomIDied", info, 120, false );
		return true;
	}
	
	
	public boolean failRescueQuest(obj_id self) throws InterruptedException
	{
		obj_id player = getObjIdObjVar(self, "player");
		if (!isValidId(player))
		{
			messageTo(self, "handleDestroySelf", null, 1, false);
			return false;
		}
		
		String failSignal = getStringObjVar(self, "failSignal");
		if (failSignal == null || failSignal.length() <= 0)
		{
			messageTo(self, "handleDestroySelf", null, 1, false);
			return false;
		}
		
		groundquests.sendSignal(player, failSignal);
		messageTo(self, "handleDestroySelf", null, 1, false);
		return true;
	}
}
