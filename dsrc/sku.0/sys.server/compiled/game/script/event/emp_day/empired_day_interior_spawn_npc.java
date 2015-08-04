package script.event.emp_day;

import script.*;
import script.base_class.*;
import script.combat_engine.*;
import java.util.Arrays;
import java.util.Hashtable;
import java.util.Vector;
import script.base_script;


public class empired_day_interior_spawn_npc extends script.base_script
{
	public empired_day_interior_spawn_npc()
	{
	}
	
	public int OnAttach(obj_id self) throws InterruptedException
	{
		messageTo(self, "removeConversationScripts", null, 10, false);
		return SCRIPT_CONTINUE;
	}
	
	
	public int OnInitialize(obj_id self) throws InterruptedException
	{
		messageTo(self, "removeConversationScripts", null, 10, false);
		return SCRIPT_CONTINUE;
	}
	
	
	public int removeConversationScripts(obj_id self, dictionary params) throws InterruptedException
	{
		if (hasScript(self, "npc.faction_recruiter.recruiter_setup"))
		{
			detachScript(self, "npc.faction_recruiter.recruiter_setup");
		}
		return SCRIPT_CONTINUE;
	}
}
