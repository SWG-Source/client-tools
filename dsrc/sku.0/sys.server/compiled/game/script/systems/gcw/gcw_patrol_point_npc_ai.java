package script.systems.gcw;

import script.*;
import script.base_class.*;
import script.combat_engine.*;
import java.util.Arrays;
import java.util.Hashtable;
import java.util.Vector;
import script.base_script;

import script.library.create;
import script.library.factions;
import script.library.utils;



public class gcw_patrol_point_npc_ai extends script.systems.gcw.gcw_city_kit
{
	public gcw_patrol_point_npc_ai()
	{
	}
	
	public void setupConstructionQuests(obj_id self, obj_id pylon) throws InterruptedException
	{
		setName(pylon, "Turret Construction Site");
		
		attachScript(pylon, "systems.gcw.gcw_city_pylon_turret");
	}
	
	
	public void setupInvasionQuests(obj_id kit) throws InterruptedException
	{
	}
	
	
	public obj_id createFactionKit(int faction, location loc) throws InterruptedException
	{
		if (loc == null)
		{
			return null;
		}
		
		obj_id kit = null;
		
		if (factions.FACTION_FLAG_IMPERIAL == faction)
		{
			kit = create.object("gcw_city_imperial_turret_1", loc);
			setName(kit, "Imperial Turret");
		}
		else if (factions.FACTION_FLAG_REBEL == faction)
		{
			kit = create.object("gcw_city_rebel_turret_1", loc);
			setName(kit, "Rebel Turret");
		}
		
		return kit;
	}
}
