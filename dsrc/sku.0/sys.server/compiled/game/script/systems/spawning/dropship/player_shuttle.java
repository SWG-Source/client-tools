package script.systems.spawning.dropship;

import script.*;
import script.base_class.*;
import script.combat_engine.*;
import java.util.Arrays;
import java.util.Hashtable;
import java.util.Vector;
import script.base_script;

import script.library.utils;
import script.library.ai_lib;



public class player_shuttle extends script.systems.spawning.dropship.base
{
	public player_shuttle()
	{
	}
	
	public int OnAttach(obj_id self) throws InterruptedException
	{
		return super.OnAttach(self);
	}
	
	
	public int landShuttle(obj_id self, dictionary params) throws InterruptedException
	{
		queueCommand(self, (-1465754503), self, "", COMMAND_PRIORITY_FRONT);
		setPosture(self, POSTURE_UPRIGHT);
		return SCRIPT_CONTINUE;
	}
	
	
	public int takeOff(obj_id self, dictionary params) throws InterruptedException
	{
		setPosture(self, POSTURE_PRONE);
		queueCommand(self, (-1114832209), self, "", COMMAND_PRIORITY_FRONT);
		messageTo(self, "destroySelf", null, 60.0f, false);
		return SCRIPT_CONTINUE;
	}
	
	
	public int explodeThenDestroySelf(obj_id self, dictionary params) throws InterruptedException
	{
		if (params != null)
		{
			
			location locList[] = params.getLocationArray("explosionLocaitonList");
			if (locList != null && locList.length > 0)
			{
				for (int i = 0; i < locList.length; i++)
				{
					testAbortScript();
					playClientEffectLoc(self, "clienteffect/combat_explosion_lair_large.cef", locList[i], 0);
				}
			}
			messageTo(self, "destroySelf", null, 0, false);
		}
		else
		{
			location death = getLocation(self);
			playClientEffectLoc(self, "clienteffect/combat_explosion_lair_large.cef", death, 0);
			messageTo(self, "destroySelf", null, 0, false);
		}
		return SCRIPT_CONTINUE;
	}
	
	
	public int destroySelf(obj_id self, dictionary params) throws InterruptedException
	{
		destroyObject(self);
		return SCRIPT_CONTINUE;
	}
}
