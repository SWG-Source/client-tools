package script.systems.gcw;

import script.*;
import script.base_class.*;
import script.combat_engine.*;
import java.util.Arrays;
import java.util.Hashtable;
import java.util.Vector;
import script.base_script;

import script.library.utils;
import script.library.factions;
import script.library.ai_lib;


public class gcw_entertainer_faction_quest extends script.base_script
{
	public gcw_entertainer_faction_quest()
	{
	}
	
	public int OnAttach(obj_id self) throws InterruptedException
	{
		if (!isValidId(self) || !exists(self))
		{
			return SCRIPT_CONTINUE;
		}
		setObjVar(self, "faction_recruit_quest", 0);
		return SCRIPT_CONTINUE;
	}
	
	
	public int readyToTalk(obj_id self, dictionary params) throws InterruptedException
	{
		
		if (!isValidId(self) || !exists(self))
		{
			return SCRIPT_CONTINUE;
		}
		
		obj_id entertainer = utils.getObjIdScriptVar(self, "ai.listeningTo");
		obj_id faction_entertainer = utils.getObjIdScriptVar(self, "faction_ent");
		
		if (!isValidId(entertainer) || (entertainer != faction_entertainer))
		{
			return SCRIPT_CONTINUE;
			
		}
		
		utils.setObjVar(self, "faction_recruit_quest", 1);
		utils.setScriptVar(self, "readyToTalk", faction_entertainer);
		
		String particle = "";
		if (factions.isImperialorImperialHelper(faction_entertainer))
		{
			particle = "appearance/pt_imperial_symbol.prt";
		}
		
		else if (factions.isRebelorRebelHelper(faction_entertainer))
		{
			particle = "appearance/pt_rebel_symbol.prt";
		}
		
		else
		{
			cleanup(self);
			return SCRIPT_CONTINUE;
		}
		
		obj_id[] particleList = new obj_id[1];
		particleList[0] = faction_entertainer;
		playClientEffectObj(particleList, particle, self, null);
		return SCRIPT_CONTINUE;
	}
	
	
	public int lostInterest(obj_id self, dictionary params) throws InterruptedException
	{
		if (!isValidId(self) || !exists(self) || params == null)
		{
			return SCRIPT_CONTINUE;
			
		}
		
		int paramsTime = params.getInt("lostInterestTime");
		int scriptVarTime = utils.getIntScriptVar(self, "lostInterestTime");
		
		if (paramsTime == scriptVarTime)
		{
			cleanup(self);
			
		}
		return SCRIPT_CONTINUE;
	}
	
	
	public int convinced(obj_id self, dictionary params) throws InterruptedException
	{
		obj_id parent = utils.getObjIdScriptVar(self, "spawnedBy");
		
		params.put("convincedTrooper", self);
		
		messageTo(parent, "createFightingNpc", params, 1.0f, false);
		LOG("mikkel","sent message to spawn a fighting npc");
		return SCRIPT_CONTINUE;
	}
	
	
	public int notConvinced(obj_id self, dictionary params) throws InterruptedException
	{
		LOG("mikkel","notConvinced, calling cleanup");
		cleanup(self);
		
		return SCRIPT_CONTINUE;
	}
	
	
	public void cleanup(obj_id self) throws InterruptedException
	{
		if (isValidId(self) && exists(self))
		{
			
			utils.removeScriptVar(self, "ai.oldEntertainerSkillMod");
			utils.removeScriptVar(self, "ai.listeningTo");
			utils.setObjVar(self, "faction_recruit_quest", 0);
			utils.removeScriptVar(self, "faction_ent");
			stop(self);
			messageTo(self, "resumeDefaultCalmBehavior", null, 0, false);
			
		}
		
	}
}
