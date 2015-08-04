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
import script.library.groundquests;
import script.library.utils;


public class novicecollector extends script.base_script
{
	public novicecollector()
	{
	}
	String c_stringFile = "conversation/novicecollector";
	
	
	public boolean novicecollector_condition__defaultCondition(obj_id player, obj_id npc) throws InterruptedException
	{
		return true;
	}
	
	
	public boolean novicecollector_condition_hasPublishGiftToken(obj_id player, obj_id npc) throws InterruptedException
	{
		return groundquests.isQuestActive(player, "chapter7_publish_gift_activation");
		
	}
	
	
	public boolean novicecollector_condition_hasFoundClickCollection(obj_id player, obj_id npc) throws InterruptedException
	{
		return groundquests.isTaskActive(player, "chapter7_beginning_click_collection", "publishClickCollectionReturnToCollectionNPC");
	}
	
	
	public boolean novicecollector_condition_hasCompletedClickCollection(obj_id player, obj_id npc) throws InterruptedException
	{
		return groundquests.hasCompletedQuest(player, "chapter7_beginning_click_collection");
	}
	
	
	public boolean novicecollector_condition_hasFoundInventoryCollection(obj_id player, obj_id npc) throws InterruptedException
	{
		if (groundquests.isTaskActive(player, "chapter7_beginning_inventory_collection_01", "returnToCollectorInventoryCollectionComplete"))
		{
			return true;
		}
		else if (groundquests.isTaskActive(player, "chapter7_beginning_inventory_collection_02", "returnToCollectorInventoryCollectionComplete"))
		{
			return true;
		}
		else if (groundquests.isTaskActive(player, "chapter7_beginning_inventory_collection_03", "returnToCollectorInventoryCollectionComplete"))
		{
			return true;
		}
		else if (groundquests.isTaskActive(player, "chapter7_beginning_inventory_collection_04", "returnToCollectorInventoryCollectionComplete"))
		{
			return true;
		}
		return false;
	}
	
	
	public boolean novicecollector_condition_hasCompletedClickInventoryCollection(obj_id player, obj_id npc) throws InterruptedException
	{
		if (groundquests.hasCompletedQuest(player, "chapter7_beginning_click_collection") && groundquests.hasCompletedQuest(player, "chapter7_beginning_inventory_collection_01"))
		{
			return true;
		}
		else if (groundquests.hasCompletedQuest(player, "chapter7_beginning_click_collection") && groundquests.hasCompletedQuest(player, "chapter7_beginning_inventory_collection_02"))
		{
			return true;
		}
		else if (groundquests.hasCompletedQuest(player, "chapter7_beginning_click_collection") && groundquests.hasCompletedQuest(player, "chapter7_beginning_inventory_collection_03"))
		{
			return true;
		}
		else if (groundquests.hasCompletedQuest(player, "chapter7_beginning_click_collection") && groundquests.hasCompletedQuest(player, "chapter7_beginning_inventory_collection_04"))
		{
			return true;
		}
		return false;
	}
	
	
	public boolean novicecollector_condition_hasFulfilledCounterQuota(obj_id player, obj_id npc) throws InterruptedException
	{
		return groundquests.isTaskActive(player, "chapter7_beginning_counter_collection", "publishCounterReturn");
	}
	
	
	public boolean novicecollector_condition_allCollectionsComplete(obj_id player, obj_id npc) throws InterruptedException
	{
		return groundquests.hasCompletedQuest(player, "chapter7_beginning_counter_collection");
	}
	
	
	public boolean novicecollector_condition_clickCollectionActive(obj_id player, obj_id npc) throws InterruptedException
	{
		return groundquests.isQuestActive(player, "chapter7_beginning_click_collection");
	}
	
	
	public boolean novicecollector_condition_inventoryCollectionActive(obj_id player, obj_id npc) throws InterruptedException
	{
		if (groundquests.isQuestActive(player, "chapter7_beginning_inventory_collection_01") || groundquests.isQuestActive(player, "chapter7_beginning_inventory_collection_02") || groundquests.isQuestActive(player, "chapter7_beginning_inventory_collection_03") || groundquests.isQuestActive(player, "chapter7_beginning_inventory_collection_04"))
		{
			return true;
		}
		
		return false;
	}
	
	
	public boolean novicecollector_condition_counterCollectionActive(obj_id player, obj_id npc) throws InterruptedException
	{
		return (groundquests.isQuestActive(player, "chapter7_beginning_counter_collection"));
	}
	
	
	public void novicecollector_action_grantCollectionQuest(obj_id player, obj_id npc) throws InterruptedException
	{
		
		groundquests.sendSignal(player, "publishClickCollectionActivation");
		
		modifyCollectionSlotValue(player, "publish_chapter7_click_activation_03", 1);
		
		groundquests.grantQuest(player, "chapter7_beginning_click_collection");
	}
	
	
	public void novicecollector_action_completeClickCollection(obj_id player, obj_id npc) throws InterruptedException
	{
		groundquests.sendSignal(player, "publishClickCollectionReturnToCollectionNPC");
	}
	
	
	public void novicecollector_action_grantInventoryQuest(obj_id player, obj_id npc) throws InterruptedException
	{
		
		modifyCollectionSlotValue(player, "publish_chapter7_activation_02", 1);
		int number = rand(1,4);
		groundquests.grantQuest(player, "chapter7_beginning_inventory_collection_0"+number);
	}
	
	
	public void novicecollector_action_completeInventoryCollection(obj_id player, obj_id npc) throws InterruptedException
	{
		groundquests.sendSignal(player, "returnToCollectorInventoryCollectionComplete");
	}
	
	
	public void novicecollector_action_grantCounterQuest(obj_id player, obj_id npc) throws InterruptedException
	{
		modifyCollectionSlotValue(player, "kill_tusken_activate", 1);
		groundquests.grantQuest(player, "chapter7_beginning_counter_collection");
	}
	
	
	public void novicecollector_action_completeCounterCollection(obj_id player, obj_id npc) throws InterruptedException
	{
		modifyCollectionSlotValue(player, "kill_tusken_complete", 1);
		groundquests.sendSignal(player, "publishCounterReturn");
	}
	
	
	public void novicecollector_action_sendActivationSignal(obj_id player, obj_id npc) throws InterruptedException
	{
		groundquests.sendSignal(player, "publishClickCollectionActivation");
	}
	
	
	public int novicecollector_handleBranch2(obj_id player, obj_id npc, string_id response) throws InterruptedException
	{
		
		if (response.equals("s_53"))
		{
			
			if (novicecollector_condition__defaultCondition (player, npc))
			{
				
				string_id message = new string_id (c_stringFile, "s_54");
				utils.removeScriptVar (player, "conversation.novicecollector.branchId");
				
				npcEndConversationWithMessage (player, message);
				
				return SCRIPT_CONTINUE;
			}
			
		}
		
		return SCRIPT_DEFAULT;
	}
	
	
	public int novicecollector_handleBranch5(obj_id player, obj_id npc, string_id response) throws InterruptedException
	{
		
		if (response.equals("s_26"))
		{
			
			if (novicecollector_condition__defaultCondition (player, npc))
			{
				novicecollector_action_grantCounterQuest (player, npc);
				
				string_id message = new string_id (c_stringFile, "s_28");
				utils.removeScriptVar (player, "conversation.novicecollector.branchId");
				
				npcEndConversationWithMessage (player, message);
				
				return SCRIPT_CONTINUE;
			}
			
		}
		
		if (response.equals("s_34"))
		{
			
			if (novicecollector_condition__defaultCondition (player, npc))
			{
				
				string_id message = new string_id (c_stringFile, "s_35");
				utils.removeScriptVar (player, "conversation.novicecollector.branchId");
				
				npcEndConversationWithMessage (player, message);
				
				return SCRIPT_CONTINUE;
			}
			
		}
		
		return SCRIPT_DEFAULT;
	}
	
	
	public int novicecollector_handleBranch10(obj_id player, obj_id npc, string_id response) throws InterruptedException
	{
		
		if (response.equals("s_18"))
		{
			
			if (novicecollector_condition__defaultCondition (player, npc))
			{
				novicecollector_action_grantInventoryQuest (player, npc);
				
				string_id message = new string_id (c_stringFile, "s_20");
				utils.removeScriptVar (player, "conversation.novicecollector.branchId");
				
				npcEndConversationWithMessage (player, message);
				
				return SCRIPT_CONTINUE;
			}
			
		}
		
		if (response.equals("s_32"))
		{
			
			if (novicecollector_condition__defaultCondition (player, npc))
			{
				
				string_id message = new string_id (c_stringFile, "s_33");
				utils.removeScriptVar (player, "conversation.novicecollector.branchId");
				
				npcEndConversationWithMessage (player, message);
				
				return SCRIPT_CONTINUE;
			}
			
		}
		
		return SCRIPT_DEFAULT;
	}
	
	
	public int novicecollector_handleBranch15(obj_id player, obj_id npc, string_id response) throws InterruptedException
	{
		
		if (response.equals("s_31"))
		{
			
			if (novicecollector_condition__defaultCondition (player, npc))
			{
				
				string_id message = new string_id (c_stringFile, "s_39");
				int numberOfResponses = 0;
				
				boolean hasResponse = false;
				
				boolean hasResponse0 = false;
				if (novicecollector_condition__defaultCondition (player, npc))
				{
					++numberOfResponses;
					hasResponse = true;
					hasResponse0 = true;
				}
				
				boolean hasResponse1 = false;
				if (novicecollector_condition__defaultCondition (player, npc))
				{
					++numberOfResponses;
					hasResponse = true;
					hasResponse1 = true;
				}
				
				if (hasResponse)
				{
					int responseIndex = 0;
					string_id responses[] = new string_id[numberOfResponses];
					
					if (hasResponse0)
					{
						responses[responseIndex++] = new string_id (c_stringFile, "s_41");
					}
					
					if (hasResponse1)
					{
						responses[responseIndex++] = new string_id (c_stringFile, "s_46");
					}
					
					utils.setScriptVar (player, "conversation.novicecollector.branchId", 16);
					
					npcSpeak (player, message);
					npcSetConversationResponses (player, responses);
				}
				else
				{
					utils.removeScriptVar (player, "conversation.novicecollector.branchId");
					
					npcEndConversationWithMessage (player, message);
				}
				
				return SCRIPT_CONTINUE;
			}
			
		}
		
		return SCRIPT_DEFAULT;
	}
	
	
	public int novicecollector_handleBranch16(obj_id player, obj_id npc, string_id response) throws InterruptedException
	{
		
		if (response.equals("s_41"))
		{
			
			if (novicecollector_condition__defaultCondition (player, npc))
			{
				novicecollector_action_grantCollectionQuest (player, npc);
				
				string_id message = new string_id (c_stringFile, "s_43");
				utils.removeScriptVar (player, "conversation.novicecollector.branchId");
				
				npcEndConversationWithMessage (player, message);
				
				return SCRIPT_CONTINUE;
			}
			
		}
		
		if (response.equals("s_46"))
		{
			
			if (novicecollector_condition__defaultCondition (player, npc))
			{
				
				string_id message = new string_id (c_stringFile, "s_48");
				utils.removeScriptVar (player, "conversation.novicecollector.branchId");
				
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
			detachScript(self, "conversation.novicecollector");
		}
		
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
	
	
	public int OnIncapacitated(obj_id self, obj_id killer) throws InterruptedException
	{
		clearCondition (self, CONDITION_CONVERSABLE);
		detachScript (self, "conversation.novicecollector");
		
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
		
		if (novicecollector_condition_allCollectionsComplete (player, npc))
		{
			doAnimationAction (npc, "goodbye");
			
			string_id message = new string_id (c_stringFile, "s_37");
			chat.chat (npc, player, message);
			
			return SCRIPT_CONTINUE;
		}
		
		if (novicecollector_condition_hasFulfilledCounterQuota (player, npc))
		{
			doAnimationAction (npc, "applause_polite");
			
			doAnimationAction (player, "bow");
			
			novicecollector_action_completeCounterCollection (player, npc);
			
			string_id message = new string_id (c_stringFile, "s_36");
			int numberOfResponses = 0;
			
			boolean hasResponse = false;
			
			boolean hasResponse0 = false;
			if (novicecollector_condition__defaultCondition (player, npc))
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
				
				utils.setScriptVar (player, "conversation.novicecollector.branchId", 2);
				
				npcStartConversation (player, npc, "novicecollector", message, responses);
			}
			else
			{
				chat.chat (npc, player, message);
			}
			
			return SCRIPT_CONTINUE;
		}
		
		if (novicecollector_condition_counterCollectionActive (player, npc))
		{
			
			string_id message = new string_id (c_stringFile, "s_51");
			chat.chat (npc, player, message);
			
			return SCRIPT_CONTINUE;
		}
		
		if (novicecollector_condition_hasCompletedClickInventoryCollection (player, npc))
		{
			
			string_id message = new string_id (c_stringFile, "s_24");
			int numberOfResponses = 0;
			
			boolean hasResponse = false;
			
			boolean hasResponse0 = false;
			if (novicecollector_condition__defaultCondition (player, npc))
			{
				++numberOfResponses;
				hasResponse = true;
				hasResponse0 = true;
			}
			
			boolean hasResponse1 = false;
			if (novicecollector_condition__defaultCondition (player, npc))
			{
				++numberOfResponses;
				hasResponse = true;
				hasResponse1 = true;
			}
			
			if (hasResponse)
			{
				int responseIndex = 0;
				string_id responses[] = new string_id[numberOfResponses];
				
				if (hasResponse0)
				{
					responses[responseIndex++] = new string_id (c_stringFile, "s_26");
				}
				
				if (hasResponse1)
				{
					responses[responseIndex++] = new string_id (c_stringFile, "s_34");
				}
				
				utils.setScriptVar (player, "conversation.novicecollector.branchId", 5);
				
				npcStartConversation (player, npc, "novicecollector", message, responses);
			}
			else
			{
				chat.chat (npc, player, message);
			}
			
			return SCRIPT_CONTINUE;
		}
		
		if (novicecollector_condition_hasFoundInventoryCollection (player, npc))
		{
			doAnimationAction (npc, "thumbs_up");
			
			novicecollector_action_completeInventoryCollection (player, npc);
			
			string_id message = new string_id (c_stringFile, "s_23");
			chat.chat (npc, player, message);
			
			return SCRIPT_CONTINUE;
		}
		
		if (novicecollector_condition_inventoryCollectionActive (player, npc))
		{
			
			string_id message = new string_id (c_stringFile, "s_50");
			chat.chat (npc, player, message);
			
			return SCRIPT_CONTINUE;
		}
		
		if (novicecollector_condition_hasCompletedClickCollection (player, npc))
		{
			
			string_id message = new string_id (c_stringFile, "s_16");
			int numberOfResponses = 0;
			
			boolean hasResponse = false;
			
			boolean hasResponse0 = false;
			if (novicecollector_condition__defaultCondition (player, npc))
			{
				++numberOfResponses;
				hasResponse = true;
				hasResponse0 = true;
			}
			
			boolean hasResponse1 = false;
			if (novicecollector_condition__defaultCondition (player, npc))
			{
				++numberOfResponses;
				hasResponse = true;
				hasResponse1 = true;
			}
			
			if (hasResponse)
			{
				int responseIndex = 0;
				string_id responses[] = new string_id[numberOfResponses];
				
				if (hasResponse0)
				{
					responses[responseIndex++] = new string_id (c_stringFile, "s_18");
				}
				
				if (hasResponse1)
				{
					responses[responseIndex++] = new string_id (c_stringFile, "s_32");
				}
				
				utils.setScriptVar (player, "conversation.novicecollector.branchId", 10);
				
				npcStartConversation (player, npc, "novicecollector", message, responses);
			}
			else
			{
				chat.chat (npc, player, message);
			}
			
			return SCRIPT_CONTINUE;
		}
		
		if (novicecollector_condition_hasFoundClickCollection (player, npc))
		{
			doAnimationAction (npc, "thumb_up");
			
			novicecollector_action_completeClickCollection (player, npc);
			
			string_id message = new string_id (c_stringFile, "s_22");
			chat.chat (npc, player, message);
			
			return SCRIPT_CONTINUE;
		}
		
		if (novicecollector_condition_clickCollectionActive (player, npc))
		{
			
			string_id message = new string_id (c_stringFile, "s_49");
			chat.chat (npc, player, message);
			
			return SCRIPT_CONTINUE;
		}
		
		if (novicecollector_condition_hasPublishGiftToken (player, npc))
		{
			doAnimationAction (npc, "curtsey");
			
			string_id message = new string_id (c_stringFile, "s_29");
			int numberOfResponses = 0;
			
			boolean hasResponse = false;
			
			boolean hasResponse0 = false;
			if (novicecollector_condition__defaultCondition (player, npc))
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
					responses[responseIndex++] = new string_id (c_stringFile, "s_31");
				}
				
				utils.setScriptVar (player, "conversation.novicecollector.branchId", 15);
				
				npcStartConversation (player, npc, "novicecollector", message, responses);
			}
			else
			{
				chat.chat (npc, player, message);
			}
			
			return SCRIPT_CONTINUE;
		}
		
		if (novicecollector_condition__defaultCondition (player, npc))
		{
			
			string_id message = new string_id (c_stringFile, "s_52");
			chat.chat (npc, player, message);
			
			return SCRIPT_CONTINUE;
		}
		
		chat.chat (npc, "Error: All conditions for OnStartNpcConversation were false.");
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int OnNpcConversationResponse(obj_id self, String conversationId, obj_id player, string_id response) throws InterruptedException
	{
		if (!conversationId.equals("novicecollector"))
		{
			return SCRIPT_CONTINUE;
		}
		
		obj_id npc = self;
		
		int branchId = utils.getIntScriptVar (player, "conversation.novicecollector.branchId");
		
		if (branchId == 2 && novicecollector_handleBranch2 (player, npc, response) == SCRIPT_CONTINUE)
		{
			return SCRIPT_CONTINUE;
		}
		
		if (branchId == 5 && novicecollector_handleBranch5 (player, npc, response) == SCRIPT_CONTINUE)
		{
			return SCRIPT_CONTINUE;
		}
		
		if (branchId == 10 && novicecollector_handleBranch10 (player, npc, response) == SCRIPT_CONTINUE)
		{
			return SCRIPT_CONTINUE;
		}
		
		if (branchId == 15 && novicecollector_handleBranch15 (player, npc, response) == SCRIPT_CONTINUE)
		{
			return SCRIPT_CONTINUE;
		}
		
		if (branchId == 16 && novicecollector_handleBranch16 (player, npc, response) == SCRIPT_CONTINUE)
		{
			return SCRIPT_CONTINUE;
		}
		
		chat.chat (npc, "Error: Fell through all branches and responses for OnNpcConversationResponse.");
		
		utils.removeScriptVar (player, "conversation.novicecollector.branchId");
		
		return SCRIPT_CONTINUE;
	}
	
}
