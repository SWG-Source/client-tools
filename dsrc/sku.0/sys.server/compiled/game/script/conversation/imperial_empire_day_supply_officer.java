package script.conversation;

import script.*;
import script.base_class.*;
import script.combat_engine.*;
import java.util.Arrays;
import java.util.Hashtable;
import java.util.Vector;
import script.base_script;


import script.library.ai_lib;
import script.library.chat;
import script.library.conversation;
import script.library.groundquests;
import script.library.holiday;
import script.library.utils;


public class imperial_empire_day_supply_officer extends script.base_script
{
	public imperial_empire_day_supply_officer()
	{
	}
	String c_stringFile = "conversation/imperial_empire_day_supply_officer";
	
	
	public boolean imperial_empire_day_supply_officer_condition__defaultCondition(obj_id player, obj_id npc) throws InterruptedException
	{
		return true;
	}
	
	
	public boolean imperial_empire_day_supply_officer_condition_hasNotCraftedSupplies(obj_id player, obj_id npc) throws InterruptedException
	{
		faceTo(npc, player);
		for (int i = 0; i < holiday.ALL_IMPERIAL_QUESTS.length; i++)
		{
			testAbortScript();
			if ((groundquests.isQuestActive(player, holiday.ALL_IMPERIAL_QUESTS[i]) && groundquests.isTaskActive(player, holiday.ALL_IMPERIAL_QUESTS[i], "crafteSupplies")))
			{
				return true;
			}
		}
		return false;
	}
	
	
	public boolean imperial_empire_day_supply_officer_condition_hasCompletedFirstMission(obj_id player, obj_id npc) throws InterruptedException
	{
		faceTo(npc, player);
		return groundquests.hasCompletedQuest(player, holiday.EMPIREDAYQUEST_IMP_CRASHSITE_01_DOMESTICS) || groundquests.hasCompletedQuest(player, holiday.EMPIREDAYQUEST_IMP_CRASHSITE_01_ENG) || groundquests.hasCompletedQuest(player, holiday.EMPIREDAYQUEST_IMP_CRASHSITE_01_MUNITIONS) || groundquests.hasCompletedQuest(player, holiday.EMPIREDAYQUEST_IMP_CRASHSITE_01_STRUC);
	}
	
	
	public boolean imperial_empire_day_supply_officer_condition_hasImperialMissionActiveWithTask(obj_id player, obj_id npc) throws InterruptedException
	{
		faceTo(npc, player);
		for (int i = 0; i < holiday.ALL_IMPERIAL_QUESTS.length; i++)
		{
			testAbortScript();
			if ((groundquests.isQuestActive(player, holiday.ALL_IMPERIAL_QUESTS[i]) && groundquests.isTaskActive(player, holiday.ALL_IMPERIAL_QUESTS[i], holiday.ALL_IMPERIAL_QUESTS[i])))
			{
				return true;
			}
		}
		return false;
	}
	
	
	public void imperial_empire_day_supply_officer_action_activateDeliverySignal(obj_id player, obj_id npc) throws InterruptedException
	{
		groundquests.sendSignal(player, "spokeToSupplyOfficer");
	}
	
	
	public int imperial_empire_day_supply_officer_handleBranch2(obj_id player, obj_id npc, string_id response) throws InterruptedException
	{
		
		if (response.equals("s_7"))
		{
			
			if (!imperial_empire_day_supply_officer_condition_hasCompletedFirstMission (player, npc))
			{
				
				string_id message = new string_id (c_stringFile, "s_9");
				int numberOfResponses = 0;
				
				boolean hasResponse = false;
				
				boolean hasResponse0 = false;
				if (imperial_empire_day_supply_officer_condition__defaultCondition (player, npc))
				{
					++numberOfResponses;
					hasResponse = true;
					hasResponse0 = true;
				}
				
				if (hasResponse)
				{
					int responseIndex = 0;
					string_id responses[] = new string_id[numberOfResponses];
					
					if (hasResponse0)
					{
						responses[responseIndex++] = new string_id (c_stringFile, "s_11");
					}
					
					utils.setScriptVar (player, "conversation.imperial_empire_day_supply_officer.branchId", 3);
					
					npcSpeak (player, message);
					npcSetConversationResponses (player, responses);
				}
				else
				{
					utils.removeScriptVar (player, "conversation.imperial_empire_day_supply_officer.branchId");
					
					npcEndConversationWithMessage (player, message);
				}
				
				return SCRIPT_CONTINUE;
			}
			
			if (imperial_empire_day_supply_officer_condition_hasCompletedFirstMission (player, npc))
			{
				
				string_id message = new string_id (c_stringFile, "s_15");
				utils.removeScriptVar (player, "conversation.imperial_empire_day_supply_officer.branchId");
				
				npcEndConversationWithMessage (player, message);
				
				return SCRIPT_CONTINUE;
			}
			
		}
		
		return SCRIPT_DEFAULT;
	}
	
	
	public int imperial_empire_day_supply_officer_handleBranch3(obj_id player, obj_id npc, string_id response) throws InterruptedException
	{
		
		if (response.equals("s_11"))
		{
			
			if (imperial_empire_day_supply_officer_condition__defaultCondition (player, npc))
			{
				imperial_empire_day_supply_officer_action_activateDeliverySignal (player, npc);
				
				string_id message = new string_id (c_stringFile, "s_13");
				utils.removeScriptVar (player, "conversation.imperial_empire_day_supply_officer.branchId");
				
				npcEndConversationWithMessage (player, message);
				
				return SCRIPT_CONTINUE;
			}
			
		}
		
		return SCRIPT_DEFAULT;
	}
	
	
	public int OnInitialize(obj_id self) throws InterruptedException
	{
		if ((!isTangible (self)) || (isPlayer (self)))
		{
			detachScript(self, "conversation.imperial_empire_day_supply_officer");
		}
		
		setCondition (self, CONDITION_CONVERSABLE);
		setCondition (self, CONDITION_INTERESTING);
		return SCRIPT_CONTINUE;
	}
	
	
	public int OnAttach(obj_id self) throws InterruptedException
	{
		setCondition (self, CONDITION_CONVERSABLE);
		setCondition (self, CONDITION_INTERESTING);
		return SCRIPT_CONTINUE;
	}
	
	
	public int OnObjectMenuRequest(obj_id self, obj_id player, menu_info menuInfo) throws InterruptedException
	{
		int menu = menuInfo.addRootMenu (menu_info_types.CONVERSE_START, null);
		menu_info_data menuInfoData = menuInfo.getMenuItemById (menu);
		menuInfoData.setServerNotify (false);
		setCondition (self, CONDITION_CONVERSABLE);
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int OnIncapacitated(obj_id self, obj_id killer) throws InterruptedException
	{
		clearCondition (self, CONDITION_CONVERSABLE);
		detachScript (self, "conversation.imperial_empire_day_supply_officer");
		
		return SCRIPT_CONTINUE;
	}
	
	
	public boolean npcStartConversation(obj_id player, obj_id npc, String convoName, string_id greetingId, prose_package greetingProse, string_id[] responses) throws InterruptedException
	{
		Object[] objects = new Object[responses.length];
		System.arraycopy(responses, 0, objects, 0, responses.length);
		return npcStartConversation(player, npc, convoName, greetingId, greetingProse, objects);
	}
	
	
	public int OnStartNpcConversation(obj_id self, obj_id player) throws InterruptedException
	{
		obj_id npc = self;
		
		if (ai_lib.isInCombat (npc) || ai_lib.isInCombat (player))
		{
			return SCRIPT_OVERRIDE;
		}
		
		if (imperial_empire_day_supply_officer_condition_hasNotCraftedSupplies (player, npc))
		{
			
			string_id message = new string_id (c_stringFile, "s_17");
			chat.chat (npc, player, message);
			
			return SCRIPT_CONTINUE;
		}
		
		if (imperial_empire_day_supply_officer_condition_hasImperialMissionActiveWithTask (player, npc))
		{
			
			string_id message = new string_id (c_stringFile, "s_5");
			int numberOfResponses = 0;
			
			boolean hasResponse = false;
			
			boolean hasResponse0 = false;
			if (imperial_empire_day_supply_officer_condition__defaultCondition (player, npc))
			{
				++numberOfResponses;
				hasResponse = true;
				hasResponse0 = true;
			}
			
			if (hasResponse)
			{
				int responseIndex = 0;
				string_id responses[] = new string_id[numberOfResponses];
				
				if (hasResponse0)
				{
					responses[responseIndex++] = new string_id (c_stringFile, "s_7");
				}
				
				utils.setScriptVar (player, "conversation.imperial_empire_day_supply_officer.branchId", 2);
				
				npcStartConversation (player, npc, "imperial_empire_day_supply_officer", message, responses);
			}
			else
			{
				chat.chat (npc, player, message);
			}
			
			return SCRIPT_CONTINUE;
		}
		
		if (imperial_empire_day_supply_officer_condition__defaultCondition (player, npc))
		{
			
			string_id message = new string_id (c_stringFile, "s_18");
			chat.chat (npc, player, message);
			
			return SCRIPT_CONTINUE;
		}
		
		chat.chat (npc, "Error: All conditions for OnStartNpcConversation were false.");
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int OnNpcConversationResponse(obj_id self, String conversationId, obj_id player, string_id response) throws InterruptedException
	{
		if (!conversationId.equals("imperial_empire_day_supply_officer"))
		{
			return SCRIPT_CONTINUE;
		}
		
		obj_id npc = self;
		
		int branchId = utils.getIntScriptVar (player, "conversation.imperial_empire_day_supply_officer.branchId");
		
		if (branchId == 2 && imperial_empire_day_supply_officer_handleBranch2 (player, npc, response) == SCRIPT_CONTINUE)
		{
			return SCRIPT_CONTINUE;
		}
		
		if (branchId == 3 && imperial_empire_day_supply_officer_handleBranch3 (player, npc, response) == SCRIPT_CONTINUE)
		{
			return SCRIPT_CONTINUE;
		}
		
		chat.chat (npc, "Error: Fell through all branches and responses for OnNpcConversationResponse.");
		
		utils.removeScriptVar (player, "conversation.imperial_empire_day_supply_officer.branchId");
		
		return SCRIPT_CONTINUE;
	}
	
}
