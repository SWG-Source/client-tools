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
import script.library.utils;


public class delivery_chardle extends script.base_script
{
	public delivery_chardle()
	{
	}
	String c_stringFile = "conversation/delivery_chardle";
	
	
	public boolean delivery_chardle_condition__defaultCondition(obj_id player, obj_id npc) throws InterruptedException
	{
		return true;
	}
	
	
	public boolean delivery_chardle_condition_noAntivirus(obj_id player, obj_id npc) throws InterruptedException
	{
		faceTo(npc, player);
		return (!groundquests.hasCompletedQuest(player, "outbreak_quest_01_imperial") && !groundquests.hasCompletedQuest(player, "outbreak_quest_01_rebel") && !groundquests.hasCompletedQuest(player, "outbreak_quest_01_neutral"));
	}
	
	
	public boolean delivery_chardle_condition_hasNotFoundBeta(obj_id player, obj_id npc) throws InterruptedException
	{
		return !hasCompletedCollectionSlot(player, "exp_icon_deathtroopers_camp_epsilon");
	}
	
	
	public boolean delivery_chardle_condition_neverDelivered(obj_id player, obj_id npc) throws InterruptedException
	{
		faceTo(npc, player);
		return (!groundquests.isQuestActiveOrComplete(player, "camp_delta_delivery_01"));
	}
	
	
	public boolean delivery_chardle_condition_hasMeetingTask(obj_id player, obj_id npc) throws InterruptedException
	{
		return groundquests.isTaskActive(player, "camp_delta_delivery_01", "getBeastDelta");
	}
	
	
	public boolean delivery_chardle_condition_hasEscortTask(obj_id player, obj_id npc) throws InterruptedException
	{
		return groundquests.isTaskActive(player, "camp_delta_delivery_01", "escortTaskDelta");
	}
	
	
	public boolean delivery_chardle_condition_hasDeliveredBefore(obj_id player, obj_id npc) throws InterruptedException
	{
		return groundquests.hasCompletedQuest(player, "camp_delta_delivery_01");
	}
	
	
	public boolean delivery_chardle_condition_tooLowLevel(obj_id player, obj_id npc) throws InterruptedException
	{
		return getLevel(player) < 60;
	}
	
	
	public void delivery_chardle_action_giveQuest(obj_id player, obj_id npc) throws InterruptedException
	{
		faceTo(npc, player);
		if (groundquests.hasCompletedQuest(player, "camp_delta_delivery_01"))
		{
			delivery_chardle_action_clearDeliveryQuest(player, npc);
		}
		groundquests.grantQuest(player, "camp_delta_delivery_01");
	}
	
	
	public void delivery_chardle_action_clearDeliveryQuest(obj_id player, obj_id npc) throws InterruptedException
	{
		groundquests.clearQuest(player, "camp_delta_delivery_01");
	}
	
	
	public int delivery_chardle_handleBranch6(obj_id player, obj_id npc, string_id response) throws InterruptedException
	{
		
		if (response.equals("s_96"))
		{
			
			if (delivery_chardle_condition__defaultCondition (player, npc))
			{
				doAnimationAction (npc, "explain");
				
				string_id message = new string_id (c_stringFile, "s_97");
				int numberOfResponses = 0;
				
				boolean hasResponse = false;
				
				boolean hasResponse0 = false;
				if (delivery_chardle_condition__defaultCondition (player, npc))
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
					
					utils.setScriptVar (player, "conversation.delivery_chardle.branchId", 7);
					
					npcSpeak (player, message);
					npcSetConversationResponses (player, responses);
				}
				else
				{
					utils.removeScriptVar (player, "conversation.delivery_chardle.branchId");
					
					npcEndConversationWithMessage (player, message);
				}
				
				return SCRIPT_CONTINUE;
			}
			
		}
		
		return SCRIPT_DEFAULT;
	}
	
	
	public int delivery_chardle_handleBranch7(obj_id player, obj_id npc, string_id response) throws InterruptedException
	{
		
		if (response.equals("s_14"))
		{
			
			if (delivery_chardle_condition__defaultCondition (player, npc))
			{
				
				string_id message = new string_id (c_stringFile, "s_18");
				int numberOfResponses = 0;
				
				boolean hasResponse = false;
				
				boolean hasResponse0 = false;
				if (delivery_chardle_condition__defaultCondition (player, npc))
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
						responses[responseIndex++] = new string_id (c_stringFile, "s_20");
					}
					
					utils.setScriptVar (player, "conversation.delivery_chardle.branchId", 8);
					
					npcSpeak (player, message);
					npcSetConversationResponses (player, responses);
				}
				else
				{
					utils.removeScriptVar (player, "conversation.delivery_chardle.branchId");
					
					npcEndConversationWithMessage (player, message);
				}
				
				return SCRIPT_CONTINUE;
			}
			
		}
		
		return SCRIPT_DEFAULT;
	}
	
	
	public int delivery_chardle_handleBranch8(obj_id player, obj_id npc, string_id response) throws InterruptedException
	{
		
		if (response.equals("s_20"))
		{
			
			if (delivery_chardle_condition__defaultCondition (player, npc))
			{
				
				string_id message = new string_id (c_stringFile, "s_22");
				int numberOfResponses = 0;
				
				boolean hasResponse = false;
				
				boolean hasResponse0 = false;
				if (delivery_chardle_condition__defaultCondition (player, npc))
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
						responses[responseIndex++] = new string_id (c_stringFile, "s_24");
					}
					
					utils.setScriptVar (player, "conversation.delivery_chardle.branchId", 9);
					
					npcSpeak (player, message);
					npcSetConversationResponses (player, responses);
				}
				else
				{
					utils.removeScriptVar (player, "conversation.delivery_chardle.branchId");
					
					npcEndConversationWithMessage (player, message);
				}
				
				return SCRIPT_CONTINUE;
			}
			
		}
		
		return SCRIPT_DEFAULT;
	}
	
	
	public int delivery_chardle_handleBranch9(obj_id player, obj_id npc, string_id response) throws InterruptedException
	{
		
		if (response.equals("s_24"))
		{
			
			if (delivery_chardle_condition__defaultCondition (player, npc))
			{
				
				string_id message = new string_id (c_stringFile, "s_26");
				int numberOfResponses = 0;
				
				boolean hasResponse = false;
				
				boolean hasResponse0 = false;
				if (delivery_chardle_condition__defaultCondition (player, npc))
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
						responses[responseIndex++] = new string_id (c_stringFile, "s_28");
					}
					
					utils.setScriptVar (player, "conversation.delivery_chardle.branchId", 10);
					
					npcSpeak (player, message);
					npcSetConversationResponses (player, responses);
				}
				else
				{
					utils.removeScriptVar (player, "conversation.delivery_chardle.branchId");
					
					npcEndConversationWithMessage (player, message);
				}
				
				return SCRIPT_CONTINUE;
			}
			
		}
		
		return SCRIPT_DEFAULT;
	}
	
	
	public int delivery_chardle_handleBranch10(obj_id player, obj_id npc, string_id response) throws InterruptedException
	{
		
		if (response.equals("s_28"))
		{
			
			if (delivery_chardle_condition__defaultCondition (player, npc))
			{
				delivery_chardle_action_giveQuest (player, npc);
				
				string_id message = new string_id (c_stringFile, "s_30");
				utils.removeScriptVar (player, "conversation.delivery_chardle.branchId");
				
				npcEndConversationWithMessage (player, message);
				
				return SCRIPT_CONTINUE;
			}
			
		}
		
		return SCRIPT_DEFAULT;
	}
	
	
	public int delivery_chardle_handleBranch12(obj_id player, obj_id npc, string_id response) throws InterruptedException
	{
		
		if (response.equals("s_34"))
		{
			
			if (delivery_chardle_condition__defaultCondition (player, npc))
			{
				delivery_chardle_action_giveQuest (player, npc);
				
				string_id message = new string_id (c_stringFile, "s_36");
				utils.removeScriptVar (player, "conversation.delivery_chardle.branchId");
				
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
			detachScript(self, "conversation.delivery_chardle");
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
		detachScript (self, "conversation.delivery_chardle");
		
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
		
		if (delivery_chardle_condition_noAntivirus (player, npc))
		{
			
			string_id message = new string_id (c_stringFile, "s_7");
			chat.chat (npc, player, message);
			
			return SCRIPT_CONTINUE;
		}
		
		if (delivery_chardle_condition_tooLowLevel (player, npc))
		{
			
			string_id message = new string_id (c_stringFile, "s_37");
			chat.chat (npc, player, message);
			
			return SCRIPT_CONTINUE;
		}
		
		if (delivery_chardle_condition_hasNotFoundBeta (player, npc))
		{
			
			string_id message = new string_id (c_stringFile, "s_10");
			chat.chat (npc, player, message);
			
			return SCRIPT_CONTINUE;
		}
		
		if (delivery_chardle_condition_hasEscortTask (player, npc))
		{
			delivery_chardle_action_clearDeliveryQuest (player, npc);
			
			string_id message = new string_id (c_stringFile, "s_16");
			chat.chat (npc, player, message);
			
			return SCRIPT_CONTINUE;
		}
		
		if (delivery_chardle_condition_hasMeetingTask (player, npc))
		{
			
			string_id message = new string_id (c_stringFile, "s_15");
			chat.chat (npc, player, message);
			
			return SCRIPT_CONTINUE;
		}
		
		if (delivery_chardle_condition_neverDelivered (player, npc))
		{
			doAnimationAction (npc, "thumb_up");
			
			string_id message = new string_id (c_stringFile, "s_73");
			int numberOfResponses = 0;
			
			boolean hasResponse = false;
			
			boolean hasResponse0 = false;
			if (delivery_chardle_condition__defaultCondition (player, npc))
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
					responses[responseIndex++] = new string_id (c_stringFile, "s_96");
				}
				
				utils.setScriptVar (player, "conversation.delivery_chardle.branchId", 6);
				
				npcStartConversation (player, npc, "delivery_chardle", message, responses);
			}
			else
			{
				chat.chat (npc, player, message);
			}
			
			return SCRIPT_CONTINUE;
		}
		
		if (delivery_chardle_condition_hasDeliveredBefore (player, npc))
		{
			
			string_id message = new string_id (c_stringFile, "s_32");
			int numberOfResponses = 0;
			
			boolean hasResponse = false;
			
			boolean hasResponse0 = false;
			if (delivery_chardle_condition__defaultCondition (player, npc))
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
					responses[responseIndex++] = new string_id (c_stringFile, "s_34");
				}
				
				utils.setScriptVar (player, "conversation.delivery_chardle.branchId", 12);
				
				npcStartConversation (player, npc, "delivery_chardle", message, responses);
			}
			else
			{
				chat.chat (npc, player, message);
			}
			
			return SCRIPT_CONTINUE;
		}
		
		if (delivery_chardle_condition__defaultCondition (player, npc))
		{
			
			string_id message = new string_id (c_stringFile, "s_60");
			chat.chat (npc, player, message);
			
			return SCRIPT_CONTINUE;
		}
		
		chat.chat (npc, "Error: All conditions for OnStartNpcConversation were false.");
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int OnNpcConversationResponse(obj_id self, String conversationId, obj_id player, string_id response) throws InterruptedException
	{
		if (!conversationId.equals("delivery_chardle"))
		{
			return SCRIPT_CONTINUE;
		}
		
		obj_id npc = self;
		
		int branchId = utils.getIntScriptVar (player, "conversation.delivery_chardle.branchId");
		
		if (branchId == 6 && delivery_chardle_handleBranch6 (player, npc, response) == SCRIPT_CONTINUE)
		{
			return SCRIPT_CONTINUE;
		}
		
		if (branchId == 7 && delivery_chardle_handleBranch7 (player, npc, response) == SCRIPT_CONTINUE)
		{
			return SCRIPT_CONTINUE;
		}
		
		if (branchId == 8 && delivery_chardle_handleBranch8 (player, npc, response) == SCRIPT_CONTINUE)
		{
			return SCRIPT_CONTINUE;
		}
		
		if (branchId == 9 && delivery_chardle_handleBranch9 (player, npc, response) == SCRIPT_CONTINUE)
		{
			return SCRIPT_CONTINUE;
		}
		
		if (branchId == 10 && delivery_chardle_handleBranch10 (player, npc, response) == SCRIPT_CONTINUE)
		{
			return SCRIPT_CONTINUE;
		}
		
		if (branchId == 12 && delivery_chardle_handleBranch12 (player, npc, response) == SCRIPT_CONTINUE)
		{
			return SCRIPT_CONTINUE;
		}
		
		chat.chat (npc, "Error: Fell through all branches and responses for OnNpcConversationResponse.");
		
		utils.removeScriptVar (player, "conversation.delivery_chardle.branchId");
		
		return SCRIPT_CONTINUE;
	}
	
}
