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
import script.library.features;
import script.library.groundquests;
import script.library.space_quest;
import script.library.utils;


public class camp_epsilon_leader extends script.base_script
{
	public camp_epsilon_leader()
	{
	}
	String c_stringFile = "conversation/camp_epsilon_leader";
	
	
	public boolean camp_epsilon_leader_condition__defaultCondition(obj_id player, obj_id npc) throws InterruptedException
	{
		return true;
	}
	
	
	public boolean camp_epsilon_leader_condition_hasRadio(obj_id player, obj_id npc) throws InterruptedException
	{
		return groundquests.isTaskActive(player, "outbreak_radio_delivery_03", "findEpsilonContact");
	}
	
	
	public boolean camp_epsilon_leader_condition_hasPerfume(obj_id player, obj_id npc) throws InterruptedException
	{
		return groundquests.isTaskActive(player, "outbreak_perfume_hidden_content", "hasPerfume");
	}
	
	
	public void camp_epsilon_leader_action_deliverRadioSignal(obj_id player, obj_id npc) throws InterruptedException
	{
		groundquests.sendSignal(player, "hasFoundEpsilonContact");
	}
	
	
	public void camp_epsilon_leader_action_hasDeliveredPerfume(obj_id player, obj_id npc) throws InterruptedException
	{
		groundquests.sendSignal(player, "hasDeliveredPerfume");
		if (!hasCompletedCollectionSlot(player, "hidden_content_perfume"))
		{
			modifyCollectionSlotValue(player, "hidden_content_perfume", 1);
		}
	}
	
	
	public int camp_epsilon_leader_handleBranch1(obj_id player, obj_id npc, string_id response) throws InterruptedException
	{
		
		if (response.equals("s_14"))
		{
			
			if (camp_epsilon_leader_condition__defaultCondition (player, npc))
			{
				doAnimationAction (npc, "smell_armpit");
				
				camp_epsilon_leader_action_hasDeliveredPerfume (player, npc);
				
				string_id message = new string_id (c_stringFile, "s_15");
				utils.removeScriptVar (player, "conversation.camp_epsilon_leader.branchId");
				
				npcEndConversationWithMessage (player, message);
				
				return SCRIPT_CONTINUE;
			}
			
		}
		
		return SCRIPT_DEFAULT;
	}
	
	
	public int camp_epsilon_leader_handleBranch3(obj_id player, obj_id npc, string_id response) throws InterruptedException
	{
		
		if (response.equals("s_32"))
		{
			
			if (camp_epsilon_leader_condition__defaultCondition (player, npc))
			{
				
				string_id message = new string_id (c_stringFile, "s_33");
				int numberOfResponses = 0;
				
				boolean hasResponse = false;
				
				boolean hasResponse0 = false;
				if (camp_epsilon_leader_condition__defaultCondition (player, npc))
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
						responses[responseIndex++] = new string_id (c_stringFile, "s_75");
					}
					
					utils.setScriptVar (player, "conversation.camp_epsilon_leader.branchId", 4);
					
					npcSpeak (player, message);
					npcSetConversationResponses (player, responses);
				}
				else
				{
					utils.removeScriptVar (player, "conversation.camp_epsilon_leader.branchId");
					
					npcEndConversationWithMessage (player, message);
				}
				
				return SCRIPT_CONTINUE;
			}
			
		}
		
		return SCRIPT_DEFAULT;
	}
	
	
	public int camp_epsilon_leader_handleBranch4(obj_id player, obj_id npc, string_id response) throws InterruptedException
	{
		
		if (response.equals("s_75"))
		{
			
			if (camp_epsilon_leader_condition__defaultCondition (player, npc))
			{
				camp_epsilon_leader_action_deliverRadioSignal (player, npc);
				
				string_id message = new string_id (c_stringFile, "s_76");
				utils.removeScriptVar (player, "conversation.camp_epsilon_leader.branchId");
				
				npcEndConversationWithMessage (player, message);
				
				return SCRIPT_CONTINUE;
			}
			
		}
		
		return SCRIPT_DEFAULT;
	}
	
	
	public int camp_epsilon_leader_handleBranch6(obj_id player, obj_id npc, string_id response) throws InterruptedException
	{
		
		if (response.equals("s_53"))
		{
			
			if (camp_epsilon_leader_condition__defaultCondition (player, npc))
			{
				doAnimationAction (player, "explain");
				
				string_id message = new string_id (c_stringFile, "s_64");
				int numberOfResponses = 0;
				
				boolean hasResponse = false;
				
				boolean hasResponse0 = false;
				if (camp_epsilon_leader_condition__defaultCondition (player, npc))
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
						responses[responseIndex++] = new string_id (c_stringFile, "s_65");
					}
					
					utils.setScriptVar (player, "conversation.camp_epsilon_leader.branchId", 7);
					
					npcSpeak (player, message);
					npcSetConversationResponses (player, responses);
				}
				else
				{
					utils.removeScriptVar (player, "conversation.camp_epsilon_leader.branchId");
					
					npcEndConversationWithMessage (player, message);
				}
				
				return SCRIPT_CONTINUE;
			}
			
		}
		
		return SCRIPT_DEFAULT;
	}
	
	
	public int camp_epsilon_leader_handleBranch7(obj_id player, obj_id npc, string_id response) throws InterruptedException
	{
		
		if (response.equals("s_65"))
		{
			
			if (camp_epsilon_leader_condition__defaultCondition (player, npc))
			{
				
				string_id message = new string_id (c_stringFile, "s_66");
				utils.removeScriptVar (player, "conversation.camp_epsilon_leader.branchId");
				
				npcEndConversationWithMessage (player, message);
				
				return SCRIPT_CONTINUE;
			}
			
		}
		
		return SCRIPT_DEFAULT;
	}
	
	
	public int OnInitialize(obj_id self) throws InterruptedException
	{
		setCondition (self, CONDITION_CONVERSABLE);
		return SCRIPT_CONTINUE;
	}
	
	
	public int OnAttach(obj_id self) throws InterruptedException
	{
		setCondition (self, CONDITION_CONVERSABLE);
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
		
		if (camp_epsilon_leader_condition_hasPerfume (player, npc))
		{
			
			string_id message = new string_id (c_stringFile, "s_13");
			int numberOfResponses = 0;
			
			boolean hasResponse = false;
			
			boolean hasResponse0 = false;
			if (camp_epsilon_leader_condition__defaultCondition (player, npc))
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
					responses[responseIndex++] = new string_id (c_stringFile, "s_14");
				}
				
				utils.setScriptVar (player, "conversation.camp_epsilon_leader.branchId", 1);
				
				npcStartConversation (player, npc, "camp_epsilon_leader", message, responses);
			}
			else
			{
				chat.chat (npc, player, message);
			}
			
			return SCRIPT_CONTINUE;
		}
		
		if (camp_epsilon_leader_condition_hasRadio (player, npc))
		{
			
			string_id message = new string_id (c_stringFile, "s_31");
			int numberOfResponses = 0;
			
			boolean hasResponse = false;
			
			boolean hasResponse0 = false;
			if (camp_epsilon_leader_condition__defaultCondition (player, npc))
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
					responses[responseIndex++] = new string_id (c_stringFile, "s_32");
				}
				
				utils.setScriptVar (player, "conversation.camp_epsilon_leader.branchId", 3);
				
				npcStartConversation (player, npc, "camp_epsilon_leader", message, responses);
			}
			else
			{
				chat.chat (npc, player, message);
			}
			
			return SCRIPT_CONTINUE;
		}
		
		if (camp_epsilon_leader_condition__defaultCondition (player, npc))
		{
			
			string_id message = new string_id (c_stringFile, "s_34");
			int numberOfResponses = 0;
			
			boolean hasResponse = false;
			
			boolean hasResponse0 = false;
			if (camp_epsilon_leader_condition__defaultCondition (player, npc))
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
					responses[responseIndex++] = new string_id (c_stringFile, "s_53");
				}
				
				utils.setScriptVar (player, "conversation.camp_epsilon_leader.branchId", 6);
				
				npcStartConversation (player, npc, "camp_epsilon_leader", message, responses);
			}
			else
			{
				chat.chat (npc, player, message);
			}
			
			return SCRIPT_CONTINUE;
		}
		
		chat.chat (npc, "Error: All conditions for OnStartNpcConversation were false.");
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int OnNpcConversationResponse(obj_id self, String conversationId, obj_id player, string_id response) throws InterruptedException
	{
		if (!conversationId.equals("camp_epsilon_leader"))
		{
			return SCRIPT_CONTINUE;
		}
		
		obj_id npc = self;
		
		int branchId = utils.getIntScriptVar (player, "conversation.camp_epsilon_leader.branchId");
		
		if (branchId == 1 && camp_epsilon_leader_handleBranch1 (player, npc, response) == SCRIPT_CONTINUE)
		{
			return SCRIPT_CONTINUE;
		}
		
		if (branchId == 3 && camp_epsilon_leader_handleBranch3 (player, npc, response) == SCRIPT_CONTINUE)
		{
			return SCRIPT_CONTINUE;
		}
		
		if (branchId == 4 && camp_epsilon_leader_handleBranch4 (player, npc, response) == SCRIPT_CONTINUE)
		{
			return SCRIPT_CONTINUE;
		}
		
		if (branchId == 6 && camp_epsilon_leader_handleBranch6 (player, npc, response) == SCRIPT_CONTINUE)
		{
			return SCRIPT_CONTINUE;
		}
		
		if (branchId == 7 && camp_epsilon_leader_handleBranch7 (player, npc, response) == SCRIPT_CONTINUE)
		{
			return SCRIPT_CONTINUE;
		}
		
		chat.chat (npc, "Error: Fell through all branches and responses for OnNpcConversationResponse.");
		
		utils.removeScriptVar (player, "conversation.camp_epsilon_leader.branchId");
		
		return SCRIPT_CONTINUE;
	}
	
}
