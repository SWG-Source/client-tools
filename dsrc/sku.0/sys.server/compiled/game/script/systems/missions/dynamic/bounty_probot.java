package script.systems.missions.dynamic;

import script.*;
import script.base_class.*;
import script.combat_engine.*;
import java.util.Arrays;
import java.util.Hashtable;
import java.util.Vector;
import script.base_script;

import script.library.utils;



public class bounty_probot extends script.systems.missions.base.mission_dynamic_base
{
	public bounty_probot()
	{
	}
	
	public int OnAttach(obj_id self) throws InterruptedException
	{
		setInvulnerable(self, true);
		messageTo(self, "destroySelf", null, 180, true);
		
		if (!hasScript(self, "conversation.bounty_probot"))
		{
			attachScript(self, "conversation.bounty_probot");
		}
		
		return SCRIPT_CONTINUE;
		
	}
	
	
	public int setup_Droid(obj_id self, dictionary params) throws InterruptedException
	{
		
		obj_id objWaypoint = params.getObjId("objWaypoint");
		obj_id objPlayer = params.getObjId("objPlayer");
		obj_id objMission = params.getObjId("objMission");
		int intTrackType = params.getInt("intTrackType");
		int intDroidType = params.getInt("intDroidType");
		setObjVar(self, "objPlayer", objPlayer);
		setObjVar(self, "objMission", objMission);
		setObjVar(self, "intTrackType", intTrackType);
		setObjVar(self, "intDroidType", intDroidType);
		setObjVar(self, "objWaypoint", objWaypoint);
		return SCRIPT_CONTINUE;
		
	}
	
	
	public int destroySelf(obj_id self, dictionary params) throws InterruptedException
	{
		
		obj_id[] objPlayers = getAllPlayers(getLocation(self), 64);
		
		if (objPlayers != null && objPlayers.length > 0)
		{
			playClientEffectLoc(objPlayers[0], "clienteffect/combat_explosion_lair_large.cef", getLocation(self), 0);
		}
		
		obj_id objMission = getObjIdObjVar(self, "objMission");
		messageTo(objMission, "stopTracking", null, 0, true);
		messageTo(self, "delete_Self", null, 0, true);
		
		return SCRIPT_CONTINUE;
		
	}
	
	
	public int delete_Self(obj_id self, dictionary params) throws InterruptedException
	{
		
		obj_id objWaypoint = getObjIdObjVar(self, "objWaypoint");
		destroyObject(objWaypoint);
		
		destroyObject(self);
		return SCRIPT_CONTINUE;
	}
	
	
	public int take_Off(obj_id self, dictionary params) throws InterruptedException
	{
		doAnimationAction(self, "sp_13");
		dictionary dctParams = new dictionary();
		obj_id objMission = getObjIdObjVar(self, "objMission");
		int intTrackType = getIntObjVar(self, "intTrackType");
		int intDroidType = getIntObjVar(self, "intDroidType");
		dctParams.put("intDroidType", intDroidType);
		dctParams.put("intTrackType", DROID_FIND_TARGET);
		messageTo(objMission, "findTarget", dctParams, 20, true);
		utils.sendPostureChange(self, POSTURE_SITTING);
		messageTo(self, "delete_Self", null, 10, true);
		
		return SCRIPT_CONTINUE;
		
	}
}
