package script.event.emp_day;

import script.*;
import script.base_class.*;
import script.combat_engine.*;
import java.util.Arrays;
import java.util.Hashtable;
import java.util.Vector;
import script.base_script;

import script.library.ai_lib;
import script.library.factions;
import script.library.groundquests;
import script.library.holiday;
import script.library.static_item;
import script.library.utils;


public class give_multiple_items_to_supply_npc extends script.base_script
{
	public give_multiple_items_to_supply_npc()
	{
	}
	public static final boolean LOGGING_ON = true;
	public static final String LOGGING_CATEGORY = "holidayTrader";
	
	public static final string_id noGiveDuringCombat = new string_id("quest/ground/system_message", "no_give_during_combat");
	
	
	public int OnGiveItem(obj_id self, obj_id item, obj_id giver) throws InterruptedException
	{
		if (ai_lib.isInCombat(giver))
		{
			blog("give_multiple_items_to_supply_npc.OnGiveItem: isInCombat");
			sendQuestSystemMessage(giver, noGiveDuringCombat);
			return SCRIPT_CONTINUE;
		}
		
		int faction = getIntObjVar(self, "holidayFaction");
		if (faction < holiday.IMPERIAL_PLAYER || faction > holiday.REBEL_PLAYER)
		{
			blog("give_multiple_items_to_supply_npc.OnGiveItem: not a faction");
			
			doAnimationAction(self, "wave_on_dismissing");
			return SCRIPT_CONTINUE;
		}
		
		if (isOpposingFaction(giver, faction))
		{
			blog("give_multiple_items_to_supply_npc.OnGiveItem: opposing faction");
			
			doAnimationAction(self, "wave_on_dismissing");
			return SCRIPT_CONTINUE;
		}
		
		if (!utils.isProfession(giver, utils.TRADER))
		{
			blog("give_multiple_items_to_supply_npc.OnGiveItem: not trader");
			
			doAnimationAction(self, "wave_on_dismissing");
			return SCRIPT_CONTINUE;
		}
		
		String itemName = getTemplateName(item);
		if (itemName == null || itemName.equals(""))
		{
			blog("give_multiple_items_to_supply_npc.OnGiveItem: template corrupt");
			
			doAnimationAction(self, "wave_on_dismissing");
			return SCRIPT_CONTINUE;
		}
		
		if (!hasEmpireDayQuest(giver, faction))
		{
			blog("give_multiple_items_to_supply_npc.OnGiveItem: no quest");
			
			doAnimationAction(self, "wave_on_dismissing");
			return SCRIPT_CONTINUE;
		}
		
		if (!isCrateItem(giver, itemName, faction))
		{
			blog("give_multiple_items_to_supply_npc.OnGiveItem: not crate item");
			
			doAnimationAction(self, "wave_on_dismissing");
			return SCRIPT_CONTINUE;
		}
		
		String crateRequired = getEmpireDayCrateForQuestName(giver, faction);
		if (crateRequired == null || crateRequired.length() <= 0)
		{
			blog("give_multiple_items_to_supply_npc.OnGiveItem: not quest for crate item");
			
			doAnimationAction(self, "wave_on_dismissing");
			return SCRIPT_CONTINUE;
		}
		
		if (!crateRequired.equals(itemName))
		{
			blog("give_multiple_items_to_supply_npc.OnGiveItem: not the crate I want for quest");
			
			doAnimationAction(self, "wave_on_dismissing");
			return SCRIPT_CONTINUE;
		}
		
		String questName = getEmpireDayQuestName(giver, faction);
		if (questName == null || questName.length() <= 0)
		{
			blog("give_multiple_items_to_supply_npc.OnGiveItem: quest name wrong");
			
			doAnimationAction(self, "wave_on_dismissing");
			return SCRIPT_CONTINUE;
		}
		
		int questCrc = groundquests.getQuestIdFromString(questName);
		
		int taskId = groundquests.getTaskId (questCrc, questName);
		questCompleteTask(questCrc, taskId, giver);
		if (!incrementCollection(self, giver, faction))
		{
			blog("give_multiple_items_to_supply_npc.OnGiveItem: collection incr failed.");
			
		}
		return SCRIPT_OVERRIDE;
	}
	
	
	public boolean isOpposingFaction(obj_id player, int npcFaction) throws InterruptedException
	{
		if (npcFaction == holiday.IMPERIAL_PLAYER && factions.isRebel(player))
		{
			return true;
		}
		if (npcFaction == holiday.REBEL_PLAYER && factions.isImperial(player))
		{
			return true;
		}
		
		return false;
	}
	
	
	public boolean isCrateItem(obj_id player, String itemName, int faction) throws InterruptedException
	{
		blog("give_multiple_items_to_supply_npc.isCrateItem: init.");
		
		if (!isValidId(player) || !exists(player))
		{
			return false;
		}
		if (itemName == null || itemName.length() <= 0)
		{
			return false;
		}
		
		blog("give_multiple_items_to_supply_npc.isCrateItem: validation.");
		
		boolean foundMatch = false;
		if (faction == holiday.IMPERIAL_PLAYER)
		{
			for (int i = 0; i < holiday.ALL_IMPERIAL_CRATE_TEMPLATES.length; i++)
			{
				testAbortScript();
				if (!itemName.equals(holiday.ALL_IMPERIAL_CRATE_TEMPLATES[i]))
				{
					continue;
				}
				
				foundMatch = true;
			}
		}
		else
		{
			blog("give_multiple_items_to_supply_npc.isCrateItem: reb player.");
			
			for (int i = 0; i < holiday.ALL_REBEL_CRATE_TEMPLATES.length; i++)
			{
				testAbortScript();
				if (!itemName.equals(holiday.ALL_REBEL_CRATE_TEMPLATES[i]))
				{
					continue;
				}
				
				foundMatch = true;
			}
		}
		
		return foundMatch;
	}
	
	
	public boolean hasEmpireDayQuest(obj_id player, int faction) throws InterruptedException
	{
		if (!isValidId(player) || !exists(player))
		{
			return false;
		}
		
		boolean hasQuest = false;
		if (faction == holiday.IMPERIAL_PLAYER)
		{
			for (int i = 0; i < holiday.ALL_IMPERIAL_QUESTS.length; i++)
			{
				testAbortScript();
				if (!groundquests.isQuestActive(player, holiday.ALL_IMPERIAL_QUESTS[i]))
				{
					continue;
				}
				
				hasQuest = true;
			}
		}
		else
		{
			for (int i = 0; i < holiday.ALL_REBEL_QUESTS.length; i++)
			{
				testAbortScript();
				if (!groundquests.isQuestActive(player, holiday.ALL_REBEL_QUESTS[i]))
				{
					continue;
				}
				
				hasQuest = true;
			}
		}
		return hasQuest;
	}
	
	
	public String getEmpireDayCrateForQuestName(obj_id player, int faction) throws InterruptedException
	{
		if (!isValidId(player) || !exists(player))
		{
			return null;
		}
		
		if (faction == holiday.IMPERIAL_PLAYER)
		{
			for (int i = 0; i < holiday.ALL_IMPERIAL_QUESTS.length; i++)
			{
				testAbortScript();
				if (!groundquests.isQuestActive(player, holiday.ALL_IMPERIAL_QUESTS[i]))
				{
					continue;
				}
				
				return holiday.ALL_IMPERIAL_QUEST_TEMPLATES[i];
			}
		}
		else
		{
			for (int i = 0; i < holiday.ALL_REBEL_QUESTS.length; i++)
			{
				testAbortScript();
				if (!groundquests.isQuestActive(player, holiday.ALL_REBEL_QUESTS[i]))
				{
					continue;
				}
				
				return holiday.ALL_REBEL_QUEST_TEMPLATES[i];
			}
		}
		return null;
	}
	
	
	public String getEmpireDayQuestName(obj_id player, int faction) throws InterruptedException
	{
		if (!isValidId(player) || !exists(player))
		{
			return null;
		}
		
		if (faction == holiday.IMPERIAL_PLAYER)
		{
			for (int i = 0; i < holiday.ALL_IMPERIAL_QUESTS.length; i++)
			{
				testAbortScript();
				if (!groundquests.isQuestActive(player, holiday.ALL_IMPERIAL_QUESTS[i]))
				{
					continue;
				}
				
				return holiday.ALL_IMPERIAL_QUESTS[i];
			}
		}
		else
		{
			for (int i = 0; i < holiday.ALL_REBEL_QUESTS.length; i++)
			{
				testAbortScript();
				if (!groundquests.isQuestActive(player, holiday.ALL_REBEL_QUESTS[i]))
				{
					continue;
				}
				
				return holiday.ALL_REBEL_QUESTS[i];
			}
		}
		return null;
	}
	
	
	public boolean incrementCollection(obj_id npc, obj_id player, int faction) throws InterruptedException
	{
		blog("give_multiple_items_to_supply_npc.incrementCollection init.");
		
		if (!isValidId(player) || !exists(player))
		{
			return false;
		}
		
		if (faction == holiday.IMPERIAL_PLAYER)
		{
			blog("give_multiple_items_to_supply_npc.incrementCollection IMPERIAL.");
			
			if (getCollectionSlotValue(player, holiday.IMPERIAL_ENGINEERING_START_SLOT) <= 0)
			{
				blog("give_multiple_items_to_supply_npc.incrementCollection IMPERIAL start slot not found.");
				
				return false;
			}
			if (!modifyCollectionSlotValue(player, holiday.IMPERIAL_ENGINEERING_COUNTER_SLOT, 1))
			{
				blog("give_multiple_items_to_supply_npc.incrementCollection COULDNT MODIFY COLLECTION.");
				
				return false;
			}
			if (hasCompletedCollection(player, holiday.IMPERIAL_ENGINEERING_COLLECTION))
			{
				faceTo(npc, player);
				doAnimationAction(npc, "celebrate");
			}
			
		}
		else
		{
			if (getCollectionSlotValue(player, holiday.REBEL_ENGINEERING_START_SLOT) <= 0)
			{
				
				return false;
			}
			
			if (!modifyCollectionSlotValue(player, holiday.REBEL_ENGINEERING_COUNTER_SLOT, 1))
			{
				return false;
			}
			if (hasCompletedCollection(player, holiday.REBEL_ENGINEERING_COLLECTION))
			{
				faceTo(npc, player);
				doAnimationAction(npc, "celebrate");
			}
		}
		return true;
	}
	
	
	public boolean blog(String msg) throws InterruptedException
	{
		if (LOGGING_ON && msg != null && !msg.equals(""))
		{
			LOG(LOGGING_CATEGORY, msg);
		}
		return true;
	}
}
