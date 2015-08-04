package script.space.quest_logic;

import script.*;
import script.base_class.*;
import script.combat_engine.*;
import java.util.Arrays;
import java.util.Hashtable;
import java.util.Vector;
import script.base_script;


import script.library.space_quest;
import script.library.space_utils;
import script.library.utils;
import script.library.prose;


public class assassinate_ship extends script.base_script
{
	public assassinate_ship()
	{
	}
	
	public int OnAttach(obj_id self) throws InterruptedException
	{
		
		messageTo( self, "warpOut", null, 1200.f, false );
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int warpOut(obj_id self, dictionary params) throws InterruptedException
	{
		
		setObjVar( self, "nowin", 1 );
		
		obj_id quest = getObjIdObjVar( self, "quest");
		if (isIdValid( quest ))
		{
			dictionary outparams = new dictionary();
			outparams.put( "ship", self );
			
			space_utils.notifyObject( quest, "warpoutFailure", outparams );
		}
		
		destroyObjectHyperspace( self );
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int objectDestroyed(obj_id self, dictionary params) throws InterruptedException
	{
		if (hasObjVar( self, "nowin" ))
		{
			return SCRIPT_CONTINUE;
		}
		if (!hasObjVar( self, "selfDestruct" ))
		{
			return SCRIPT_CONTINUE;
		}
		
		obj_id quest = getObjIdObjVar( self, "quest");
		dictionary outparams = new dictionary();
		outparams.put( "ship", self );
		space_utils.notifyObject( quest, "handleShipDestroyed", outparams );
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int missionAbort(obj_id self, dictionary params) throws InterruptedException
	{
		destroyObjectHyperspace( self );
		
		return SCRIPT_CONTINUE;
	}
}
