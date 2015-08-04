package script.event.emp_day;

import script.*;
import script.base_class.*;
import script.combat_engine.*;
import java.util.Arrays;
import java.util.Hashtable;
import java.util.Vector;
import script.base_script;

import script.library.performance;


public class empire_day_interior_spawn_npc extends script.base_script
{
	public empire_day_interior_spawn_npc()
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
		if (hasScript(self, "conversation.faction_recruiter_rebel"))
		{
			detachScript(self, "conversation.faction_recruiter_rebel");
		}
		if (hasScript(self, "conversation.faction_recruiter_imperial"))
		{
			detachScript(self, "conversation.faction_recruiter_imperial");
		}
		if (hasScript(self, "npc.faction_recruiter.faction_recruiter"))
		{
			detachScript(self, "npc.faction_recruiter.faction_recruiter");
		}
		if (hasScript(self, "npc.faction_recruiter.recruiter_setup"))
		{
			detachScript(self, "npc.faction_recruiter.recruiter_setup");
		}
		if (hasScript(self, "systems.gcw.gcw_data_updater"))
		{
			detachScript(self, "systems.gcw.gcw_data_updater");
		}
		if (hasScript(self, "ai.soldier"))
		{
			detachScript(self, "ai.soldier");
		}
		if (!hasScript(self, "ai.ai"))
		{
			attachScript(self, "ai.ai");
		}
		
		clearCondition (self, CONDITION_CONVERSABLE);
		if (hasObjVar(self, performance.NPC_ENTERTAINMENT_NO_ENTERTAIN))
		{
			removeObjVar(self,performance.NPC_ENTERTAINMENT_NO_ENTERTAIN);
		}
		
		return SCRIPT_CONTINUE;
	}
}
