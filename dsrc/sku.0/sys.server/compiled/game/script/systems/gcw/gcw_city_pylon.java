package script.systems.gcw;

import script.*;
import script.base_class.*;
import script.combat_engine.*;
import java.util.Arrays;
import java.util.Hashtable;
import java.util.Vector;
import script.base_script;

import script.library.buff;
import script.library.factions;
import script.library.gcw;
import script.library.groundquests;
import script.library.prose;
import script.library.skill;
import script.library.stealth;
import script.library.sui;
import script.library.trial;
import script.library.utils;


public class gcw_city_pylon extends script.base_script
{
	public gcw_city_pylon()
	{
	}
	public static final int PYLON_CONSTRUCTION_TIME = 10;
	
	public static final string_id SID_PYLON_CANNOT_USE = new string_id("gcw", "pylon_construction_cannot_use");
	public static final string_id SID_PYLON_CONSTRUCTION_COMPLETE = new string_id("gcw", "pylon_construction_complete");
	public static final string_id SID_PYLON_TOO_FAR = new string_id("gcw", "pylon_too_far");
	public static final string_id SID_PYLON_SUI_PROMPT = new string_id("gcw", "pylon_construction_prompt");
	public static final string_id SID_PYLON_ENTERED_COMBAT = new string_id("gcw", "pylon_construction_entered_combat");
	public static final string_id SID_PYLON_MOVED = new string_id("gcw", "pylon_construction_moved");
	public static final string_id SID_PYLON_INCAPACITATED = new string_id("gcw", "pylon_construction_incapacitated");
	public static final string_id SID_TOO_FATIGUED = new string_id("gcw", "too_fatigued_to_construct");
	
	
	public int OnAttach(obj_id self) throws InterruptedException
	{
		messageTo(self, "handleSetup", null, 1.0f, false);
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int OnHearSpeech(obj_id self, obj_id objSpeaker, String strText) throws InterruptedException
	{
		if (!isGod(objSpeaker))
		{
			return SCRIPT_CONTINUE;
		}
		
		if (strText.equals("gcwbuild"))
		{
			int questsCompleted = getIntObjVar(self, "gcw.constructionQuestsCompleted");
			
			if (questsCompleted < gcw.GCW_CONSTRUCTION_MAXIMUM / 2)
			{
				questsCompleted = gcw.GCW_CONSTRUCTION_MAXIMUM / 2;
			}
			else if (questsCompleted < gcw.GCW_CONSTRUCTION_MAXIMUM)
			{
				questsCompleted = gcw.GCW_CONSTRUCTION_MAXIMUM;
			}
			
			setObjVar(self, "gcw.constructionQuestsCompleted", questsCompleted);
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public void updatePylonName(obj_id self) throws InterruptedException
	{
		String name = utils.getStringScriptVar(self, "gcw.name");
		
		int questsCompleted = getIntObjVar(self, "gcw.constructionQuestsCompleted");
		
		int questsMaximum = getMaximumQuests(self);
		
		if (questsMaximum == 0)
		{
			return;
		}
		
		String suffix = "";
		
		if (questsCompleted >= questsMaximum)
		{
			suffix = " (100 Resources)";
		}
		else if (questsCompleted <= 0)
		{
			suffix = " (0 Resources)";
		}
		else
		{
			suffix = " ("+ questsCompleted + " Resources)";
		}
		
		setName(self, name + suffix);
	}
	
	
	public String getImperialIcon() throws InterruptedException
	{
		return "pt_icon_quest_red.prt";
	}
	
	
	public String getRebelIcon() throws InterruptedException
	{
		return "pt_icon_quest_red.prt";
	}
	
	
	public int handleSetup(obj_id self, dictionary params) throws InterruptedException
	{
		if (!utils.hasScriptVar(self, "faction"))
		{
			return SCRIPT_CONTINUE;
		}
		
		int faction = utils.getIntScriptVar(self, "faction");
		
		if (factions.FACTION_FLAG_IMPERIAL == faction)
		{
			utils.setScriptVar(self, "gcwQuestIcon", getImperialIcon());
		}
		else if (factions.FACTION_FLAG_REBEL == faction)
		{
			utils.setScriptVar(self, "gcwQuestIcon", getRebelIcon());
		}
		
		gcw.playQuestIconParticle(self);
		
		updatePylonName(self);
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int handleUpdateName(obj_id self, dictionary params) throws InterruptedException
	{
		updatePylonName(self);
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int playQuestIcon(obj_id self, dictionary params) throws InterruptedException
	{
		int questsCompleted = getIntObjVar(self, "gcw.constructionQuestsCompleted");
		
		if (questsCompleted < getMaximumQuests(self))
		{
			gcw.playQuestIconHandler(self, params);
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int getMaximumQuests(obj_id self) throws InterruptedException
	{
		return gcw.GCW_CONSTRUCTION_MAXIMUM;
	}
	
	
	public int OnGetAttributes(obj_id self, obj_id player, String[] names, String[] attribs) throws InterruptedException
	{
		int idx = utils.getValidAttributeIndex(names);
		
		if (idx == -1)
		{
			return super.OnGetAttributes(self, player, names, attribs);
		}
		
		int questsCompleted = getIntObjVar(self, "gcw.constructionQuestsCompleted");
		int questsMaximum = getMaximumQuests(self);
		
		if (questsCompleted > questsMaximum)
		{
			questsCompleted = questsMaximum;
		}
		
		if (questsCompleted > 0)
		{
			names[idx] = "construction_completed";
			attribs[idx] = "" + questsCompleted + " (" + questsMaximum + " Maximum)";
			idx++;
		}
		else
		{
			names[idx] = "construction_completed";
			attribs[idx] = "0 (" + questsMaximum + " Maximum)";
			idx++;
		}
		
		if (questsCompleted <= 0)
		{
			names[idx] = "pylon_level";
			attribs[idx] = "0 (No construction)";
			idx++;
		}
		else if (questsCompleted < getMaximumQuests(self) / 2)
		{
			names[idx] = "pylon_level";
			attribs[idx] = "1";
			idx++;
		}
		else if (questsCompleted < getMaximumQuests(self))
		{
			names[idx] = "pylon_level";
			attribs[idx] = "2";
			idx++;
		}
		else if (questsCompleted >= getMaximumQuests(self))
		{
			names[idx] = "pylon_level";
			attribs[idx] = "3";
			idx++;
		}
		
		return super.OnGetAttributes(self, player, names, attribs);
	}
	
	
	public String getConstructionQuest() throws InterruptedException
	{
		return "gcw_construct_barricade";
	}
	
	
	public boolean hasConstructionResources(obj_id player) throws InterruptedException
	{
		return false;
	}
	
	
	public boolean consumeConstructionResources(obj_id player) throws InterruptedException
	{
		return false;
	}
	
	
	public int OnObjectMenuRequest(obj_id self, obj_id player, menu_info mi) throws InterruptedException
	{
		if (!isIdValid(player) || !exists(player))
		{
			return SCRIPT_CONTINUE;
		}
		
		if (!utils.hasScriptVar(self, "faction"))
		{
			return SCRIPT_CONTINUE;
		}
		
		int faction = utils.getIntScriptVar(self, "faction");
		if (faction < 0)
		{
			return SCRIPT_CONTINUE;
		}
		
		menu_info_data data = mi.getMenuItemByType(menu_info_types.ITEM_USE);
		
		if (data != null)
		{
			data.setServerNotify (true);
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int OnObjectMenuSelect(obj_id self, obj_id player, int item) throws InterruptedException
	{
		if (!isIdValid(player) || !exists(player))
		{
			return SCRIPT_CONTINUE;
		}
		
		if (item == menu_info_types.ITEM_USE)
		{
			if (isDead(player) || isIncapacitated(player) || !factions.isPlayerSameGcwFactionAsSchedulerObject(player, self) || factions.isOnLeave(player))
			{
				sendSystemMessage(player, SID_PYLON_CANNOT_USE);
				return SCRIPT_CONTINUE;
			}
			
			if (getDistance(self, player) > 5.0f)
			{
				sendSystemMessage(player, SID_PYLON_TOO_FAR);
				return SCRIPT_CONTINUE;
			}
			
			int questsCompleted = getIntObjVar(self, "gcw.constructionQuestsCompleted");
			
			if (questsCompleted >= getMaximumQuests(self))
			{
				sendSystemMessage(player, SID_PYLON_CONSTRUCTION_COMPLETE);
				return SCRIPT_CONTINUE;
			}
			
			String skillTemplate = getSkillTemplate(player);
			String profession = skill.getProfessionName(skillTemplate);
			
			String questName = getConstructionQuest();
			
			if (groundquests.hasCompletedQuest(player, questName))
			{
				groundquests.clearQuest(player, questName);
			}
			
			if (!groundquests.isQuestActive(player, questName))
			{
				groundquests.grantQuest(player, questName);
				
				return SCRIPT_CONTINUE;
			}
			
			if (groundquests.isQuestActive(player, questName) && gcw.hasConstructionOrRepairTool(player, self))
			{
				startConstructionAttempt(self, player);
				
				faceTo(player, self);
				doAnimationAction(player, "manipulate_medium");
				
				location loc = getLocation(self);
				
				playClientEffectLoc(self, "appearance/pt_gcw_pylon_construction.prt", loc, 0);
			}
			else if (groundquests.isQuestActive(player, questName) && !gcw.hasConstructionOrRepairTool(player, self))
			{
				gcw.playerSystemMessageResourceNeeded(player, self, true);
			}
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public void startConstructionAttempt(obj_id self, obj_id player) throws InterruptedException
	{
		int gameTime = getGameTime();
		
		stealth.testInvisCombatAction(player, self);
		
		int flags = 0;
		flags |= sui.CD_EVENT_LOCOMOTION;
		flags |= sui.CD_EVENT_INCAPACITATE;
		flags |= sui.CD_EVENT_DAMAGED;
		flags |= sui.CD_EVENT_STEALTHED;
		flags |= sui.CD_EVENT_DAMAGE_IMMUNE;
		
		int captureTime = PYLON_CONSTRUCTION_TIME + gcw.getFatigueTimerMod(player);
		
		if (captureTime > gcw.GCW_FATIGUE_TIMER_MAX)
		{
			captureTime = gcw.GCW_FATIGUE_TIMER_MAX;
		}
		
		if (isGod(player))
		{
			captureTime = 3;
		}
		
		int pid = sui.smartCountdownTimerSUI(self, player, "gcwPylonConstruction", SID_PYLON_SUI_PROMPT, 0, captureTime, "handleConstructionAttemptResults", 4.0f, flags);
	}
	
	
	public int handleConstructionAttemptResults(obj_id self, dictionary params) throws InterruptedException
	{
		int pid = params.getInt("id");
		obj_id player = params.getObjId("player");
		
		if (!isIdValid(player) || !exists(player) || factions.isOnLeave(player))
		{
			return SCRIPT_CONTINUE;
		}
		
		int bp = sui.getIntButtonPressed(params);
		
		if (bp == sui.BP_CANCEL)
		{
			detachScript(player, sui.COUNTDOWNTIMER_PLAYER_SCRIPT);
			
			return SCRIPT_CONTINUE;
		}
		else if (bp == sui.BP_REVERT)
		{
			int event = params.getInt("event");
			
			if (event == sui.CD_EVENT_COMBAT)
			{
				sendSystemMessage(player, SID_PYLON_ENTERED_COMBAT);
			}
			else if (event == sui.CD_EVENT_LOCOMOTION)
			{
				sendSystemMessage(player, SID_PYLON_MOVED);
			}
			else if (event == sui.CD_EVENT_INCAPACITATE)
			{
				sendSystemMessage(player, SID_PYLON_INCAPACITATED);
			}
			
			return SCRIPT_CONTINUE;
		}
		
		if (!hasObjVar(player, sui.COUNTDOWNTIMER_SUI_VAR))
		{
			return SCRIPT_CONTINUE;
		}
		
		int test_pid = getIntObjVar(player, sui.COUNTDOWNTIMER_SUI_VAR);
		
		if (pid != test_pid)
		{
			return SCRIPT_CONTINUE;
		}
		
		forceCloseSUIPage(pid);
		
		detachScript(player, sui.COUNTDOWNTIMER_PLAYER_SCRIPT);
		
		if (!factions.isPlayerSameGcwFactionAsSchedulerObject(player, self))
		{
			return SCRIPT_CONTINUE;
		}
		
		int questsCompleted = getIntObjVar(self, "gcw.constructionQuestsCompleted");
		
		if (questsCompleted >= getMaximumQuests(self))
		{
			sendSystemMessage(player, SID_PYLON_CONSTRUCTION_COMPLETE);
			return SCRIPT_CONTINUE;
		}
		
		String questName = getConstructionQuest();
		if (questName == null || questName.length() <= 0)
		{
			return SCRIPT_CONTINUE;
		}
		
		LOG("gcw_constrction","handleConstructionAttemptResults checking for quest: "+questName);
		
		if (!groundquests.isQuestActive(player, questName))
		{
			return SCRIPT_CONTINUE;
		}
		
		LOG("gcw_constrction","handleConstructionAttemptResults checking tasks for quest: "+questName);
		
		if (!groundquests.isTaskActive(player, questName, gcw.GCW_CONSTRUCTION_TASK + "_" + questName))
		{
			return SCRIPT_CONTINUE;
		}
		
		LOG("gcw_constrction","handleConstructionAttemptResults task found");
		
		if (gcw.useConstructionOrRepairTool(player, self))
		{
			LOG("gcw_constrction","handleTraderRepairQuest Rebel task was signaled");
			groundquests.sendSignal(player, gcw.GCW_CONSTRUCTION_SIGNAL + "_"+ questName);
			trial.addNonInstanceFactionParticipant(player, self);
			
			buff.applyBuff(player, "gcw_fatigue");
			
			int gameTime = getGameTime();
			
			params.put("gcwFatigueTime", gameTime);
			utils.setScriptVar(player, "gcw.fatigueTime", gameTime);
			
			messageTo(player, "gcwFatigueDecay", params, 120.0f, false);
			
			int completed = getIntObjVar(self, "gcw.constructionQuestsCompleted");
			
			String skillTemplate = getSkillTemplate(player);
			String profession = skill.getProfessionName(skillTemplate);
			
			if (profession.equals("trader"))
			{
				completed += 10;
			}
			else
			{
				completed++;
			}
			
			setObjVar(self, "gcw.constructionQuestsCompleted", completed);
			
			updatePylonName(self);
		}
		
		return SCRIPT_CONTINUE;
	}
}
