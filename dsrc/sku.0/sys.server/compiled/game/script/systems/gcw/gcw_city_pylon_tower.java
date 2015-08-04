package script.systems.gcw;

import script.*;
import script.base_class.*;
import script.combat_engine.*;
import java.util.Arrays;
import java.util.Hashtable;
import java.util.Vector;
import script.base_script;

import script.library.factions;
import script.library.gcw;
import script.library.groundquests;
import script.library.resource;
import script.library.skill;
import script.library.utils;



public class gcw_city_pylon_tower extends script.systems.gcw.gcw_city_pylon
{
	public gcw_city_pylon_tower()
	{
	}
	
	public int OnAttach(obj_id self) throws InterruptedException
	{
		setObjVar(self, gcw.GCW_TOOL_TEMPLATE_OBJVAR, "object/tangible/gcw/crafting_quest/gcw_tower_tool.iff");
		return super.OnAttach(self);
	}
	
	
	public String getConstructionQuest() throws InterruptedException
	{
		return "gcw_construct_tower";
	}
}
