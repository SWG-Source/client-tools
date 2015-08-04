package script.systems.spawning;

import script.*;
import script.base_class.*;
import script.combat_engine.*;
import java.util.Arrays;
import java.util.Hashtable;
import java.util.Vector;
import script.base_script;

import script.library.utils;


public class spawned_tracker extends script.base_script
{
	public spawned_tracker()
	{
	}
	
	public int OnIncapacitated(obj_id self, obj_id objAttacker) throws InterruptedException
	{
		if (!utils.hasScriptVar(self, "intCleanedUp"))
		{
			utils.setScriptVar(self, "intCleanedUp", 1);
			obj_id objParent = getObjIdObjVar(self, "objParent");
			float fltRespawnTime = getFloatObjVar(self, "fltRespawnTime");
			
			if (utils.hasScriptVar(self, "deathTracker"))
			{
				location loc = utils.getLocationScriptVar(self, "deathTracker");
				dictionary params = new dictionary ();
				params.put ("loc", loc );
				messageTo ( objParent, "spawnDestroyed", params, fltRespawnTime, false );
				return SCRIPT_CONTINUE;
			}
			messageTo(objParent, "spawnDestroyed", null, fltRespawnTime, false);
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int OnDestroy(obj_id self) throws InterruptedException
	{
		
		if (hasObjVar(self, "dontCountDeath"))
		{
			return SCRIPT_CONTINUE;
		}
		
		if (!utils.hasScriptVar(self, "intCleanedUp"))
		{
			utils.setScriptVar(self, "intCleanedUp", 1);
			obj_id objParent = getObjIdObjVar(self, "objParent");
			
			if (!isIdValid(objParent))
			{
				return SCRIPT_CONTINUE;
			}
			
			float fltRespawnTime = getFloatObjVar(self, "fltRespawnTime");
			if (utils.hasScriptVar(self, "deathTracker"))
			{
				location loc = utils.getLocationScriptVar(self, "deathTracker");
				dictionary params = new dictionary ();
				params.put ("loc", loc );
				messageTo ( objParent, "spawnDestroyed", params, fltRespawnTime, false );
				return SCRIPT_CONTINUE;
			}
			messageTo(objParent, "spawnDestroyed", null, fltRespawnTime, false);
			
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int selfDestruct(obj_id self, dictionary params) throws InterruptedException
	{
		if (hasObjVar(self, "force_cleanup"))
		{
			
			setPosture(self, POSTURE_INCAPACITATED);
		}
		
		destroyObject(self);
		return SCRIPT_CONTINUE;
	}
	
	
	public int OnRemovingFromWorld(obj_id self) throws InterruptedException
	{
		return SCRIPT_CONTINUE;
	}
}
