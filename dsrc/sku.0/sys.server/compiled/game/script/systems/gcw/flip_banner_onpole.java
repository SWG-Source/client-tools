package script.systems.gcw;

import script.*;
import script.base_class.*;
import script.combat_engine.*;
import java.util.Arrays;
import java.util.Hashtable;
import java.util.Vector;
import script.base_script;


import script.library.create;
import script.library.ai_lib;
import script.library.locations;
import script.library.utils;
import script.library.gcw;



public class flip_banner_onpole extends script.systems.gcw.flip_banner
{
	public flip_banner_onpole()
	{
	}
	
	public int OnInitialize(obj_id self) throws InterruptedException
	{
		return super.OnInitialize(self);
	}
	
	
	public int OnDestroy(obj_id self) throws InterruptedException
	{
		return super.OnDestroy(self);
	}
	
	
	public int OnUnloadedFromMemory(obj_id self) throws InterruptedException
	{
		return super.OnUnloadedFromMemory(self);
	}
	
	
	public void spawnBanner(obj_id self, String faction) throws InterruptedException
	{
		
		String empiredayRunning = getConfigSetting("GameServer", "empireday_ceremony");
		if (empiredayRunning != null)
		{
			if (empiredayRunning.equals("true") || empiredayRunning.equals("1"))
			{
				
				location here = getLocation( self );
				String city = locations.getCityName( here );
				if (city == null)
				{
					city = locations.getGuardSpawnerRegionName( here );
				}
				
				if (city != null && city.length() > 0)
				{
					if (city.equals("coronet"))
					{
						faction = "rebel";
					}
					else if (city.equals("theed"))
					{
						faction = "imperial";
					}
				}
			}
		}
		
		transform t = getTransform_o2w( self );
		obj_id banner = createObject( "object/tangible/gcw/flip_banner_onpole_"+faction+".iff", t, null );
		
		if (banner == null || !isIdValid( banner ))
		{
			
			return;
		}
		
		setObjVar( banner, "spawner", self );
		utils.setScriptVar( self, "banner", banner );
		
		return;
	}
}
