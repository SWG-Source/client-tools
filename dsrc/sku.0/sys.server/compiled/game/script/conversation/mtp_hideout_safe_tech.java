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


public class mtp_hideout_safe_tech extends script.base_script
{
	public mtp_hideout_safe_tech()
	{
	}
	String c_stringFile = "conversation/mtp_hideout_safe_tech";
	
	
	public boolean mtp_hideout_safe_tech_condition__defaultCondition(obj_id player, obj_id npc) throws InterruptedException
	{
		return true;
	}
	
	
	public boolean mtp_hideout_safe_tech_condition_isEligible(obj_id player, obj_id npc) throws InterruptedException
	{
		return ( (!groundquests.isQuestActiveOrComplete(player, "quest/mtp_find_infiltrator_1")) && (!groundquests.isQuestActiveOrComplete(player, "quest/mtp_find_infiltrator_2")) && (!groundquests.isQuestActiveOrComplete(player, "quest/mtp_find_infiltrator_3")) && (!groundquests.isQuestActiveOrComplete(player, "quest/mtp_find_infiltrator_4")) && (!groundquests.isQuestActiveOrComplete(player, "quest/mtp_find_infiltrator_5")) );
	}
	
	
	public boolean mtp_hideout_safe_tech_condition_hasCompleted(obj_id player, obj_id npc) throws InterruptedException
	{
		return (groundquests.hasCompletedQuest(player, "quest/mtp_find_infiltrator_1") || groundquests.hasCompletedQuest(player, "quest/mtp_find_infiltrator_2") || groundquests.hasCompletedQuest(player, "quest/mtp_find_infiltrator_3") || groundquests.hasCompletedQuest(player, "quest/mtp_find_infiltrator_4") || groundquests.hasCompletedQuest(player, "quest/mtp_find_infiltrator_5") );
	}
	
	
	public boolean mtp_hideout_safe_tech_condition_isActive(obj_id player, obj_id npc) throws InterruptedException
	{
		return (groundquests.isQuestActive(player, "quest/mtp_find_infiltrator_1") || groundquests.isQuestActive(player, "quest/mtp_find_infiltrator_2") || groundquests.isQuestActive(player, "quest/mtp_find_infiltrator_3") || groundquests.isQuestActive(player, "quest/mtp_find_infiltrator_4") || groundquests.isQuestActive(player, "quest/mtp_find_infiltrator_5") );
	}
	
	
	public boolean mtp_hideout_safe_tech_condition_isReturningSuccess(obj_id player, obj_id npc) throws InterruptedException
	{
		return (groundquests.isTaskActive(player, "quest/mtp_find_infiltrator_1", "returnToLocksmith")) || (groundquests.isTaskActive(player, "quest/mtp_find_infiltrator_2", "returnToLocksmith")) || (groundquests.isTaskActive(player, "quest/mtp_find_infiltrator_3", "returnToLocksmith")) || (groundquests.isTaskActive(player, "quest/mtp_find_infiltrator_4", "returnToLocksmith")) || (groundquests.isTaskActive(player, "quest/mtp_find_infiltrator_5", "returnToLocksmith"));
	}
	
	
	public void mtp_hideout_safe_tech_action_giveQuest(obj_id player, obj_id npc) throws InterruptedException
	{
		groundquests.grantQuest(player, "quest/mtp_find_infiltrator_"+rand(1,5));
	}
	
	
	public void mtp_hideout_safe_tech_action_completeTask(obj_id player, obj_id npc) throws InterruptedException
	{
		groundquests.sendSignal(player, "returnedToMeatlumpLocksmith");
	}
	
	
	public void mtp_hideout_safe_tech_action_giveLocations(obj_id player, obj_id npc) throws InterruptedException
	{
		groundquests.grantQuest(player, "quest/mtp_camp_quest_rori_talus");
	}
	
	
	public int mtp_hideout_safe_tech_handleBranch1(obj_id player, obj_id npc, string_id response) throws InterruptedException
	{
		
		if (response.equals("s_6"))
		{
			
			if (mtp_hideout_safe_tech_condition__defaultCondition (player, npc))
			{
				
				string_id message = new string_id (c_stringFile, "s_8");
				int numberOfResponses = 0;
				
				boolean hasResponse = false;
				
				boolean hasResponse0 = false;
				if (mtp_hideout_safe_tech_condition__defaultCondition (player, npc))
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
						responses[responseIndex++] = new string_id (c_stringFile, "s_10");
					}
					
					utils.setScriptVar (player, "conversation.mtp_hideout_safe_tech.branchId", 2);
					
					npcSpeak (player, message);
					npcSetConversationResponses (player, responses);
				}
				else
				{
					utils.removeScriptVar (player, "conversation.mtp_hideout_safe_tech.branchId");
					
					npcEndConversationWithMessage (player, message);
				}
				
				return SCRIPT_CONTINUE;
			}
			
		}
		
		return SCRIPT_DEFAULT;
	}
	
	
	public int mtp_hideout_safe_tech_handleBranch2(obj_id player, obj_id npc, string_id response) throws InterruptedException
	{
		
		if (response.equals("s_10"))
		{
			
			if (mtp_hideout_safe_tech_condition__defaultCondition (player, npc))
			{
				
				string_id message = new string_id (c_stringFile, "s_12");
				int numberOfResponses = 0;
				
				boolean hasResponse = false;
				
				boolean hasResponse0 = false;
				if (mtp_hideout_safe_tech_condition__defaultCondition (player, npc))
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
					
					utils.setScriptVar (player, "conversation.mtp_hideout_safe_tech.branchId", 3);
					
					npcSpeak (player, message);
					npcSetConversationResponses (player, responses);
				}
				else
				{
					utils.removeScriptVar (player, "conversation.mtp_hideout_safe_tech.branchId");
					
					npcEndConversationWithMessage (player, message);
				}
				
				return SCRIPT_CONTINUE;
			}
			
		}
		
		return SCRIPT_DEFAULT;
	}
	
	
	public int mtp_hideout_safe_tech_handleBranch3(obj_id player, obj_id npc, string_id response) throws InterruptedException
	{
		
		if (response.equals("s_14"))
		{
			
			if (mtp_hideout_safe_tech_condition__defaultCondition (player, npc))
			{
				
				string_id message = new string_id (c_stringFile, "s_16");
				int numberOfResponses = 0;
				
				boolean hasResponse = false;
				
				boolean hasResponse0 = false;
				if (mtp_hideout_safe_tech_condition__defaultCondition (player, npc))
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
						responses[responseIndex++] = new string_id (c_stringFile, "s_18");
					}
					
					utils.setScriptVar (player, "conversation.mtp_hideout_safe_tech.branchId", 4);
					
					npcSpeak (player, message);
					npcSetConversationResponses (player, responses);
				}
				else
				{
					utils.removeScriptVar (player, "conversation.mtp_hideout_safe_tech.branchId");
					
					npcEndConversationWithMessage (player, message);
				}
				
				return SCRIPT_CONTINUE;
			}
			
		}
		
		return SCRIPT_DEFAULT;
	}
	
	
	public int mtp_hideout_safe_tech_handleBranch4(obj_id player, obj_id npc, string_id response) throws InterruptedException
	{
		
		if (response.equals("s_18"))
		{
			
			if (mtp_hideout_safe_tech_condition__defaultCondition (player, npc))
			{
				
				string_id message = new string_id (c_stringFile, "s_20");
				int numberOfResponses = 0;
				
				boolean hasResponse = false;
				
				boolean hasResponse0 = false;
				if (mtp_hideout_safe_tech_condition__defaultCondition (player, npc))
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
						responses[responseIndex++] = new string_id (c_stringFile, "s_22");
					}
					
					utils.setScriptVar (player, "conversation.mtp_hideout_safe_tech.branchId", 5);
					
					npcSpeak (player, message);
					npcSetConversationResponses (player, responses);
				}
				else
				{
					utils.removeScriptVar (player, "conversation.mtp_hideout_safe_tech.branchId");
					
					npcEndConversationWithMessage (player, message);
				}
				
				return SCRIPT_CONTINUE;
			}
			
		}
		
		return SCRIPT_DEFAULT;
	}
	
	
	public int mtp_hideout_safe_tech_handleBranch5(obj_id player, obj_id npc, string_id response) throws InterruptedException
	{
		
		if (response.equals("s_22"))
		{
			
			if (mtp_hideout_safe_tech_condition__defaultCondition (player, npc))
			{
				
				string_id message = new string_id (c_stringFile, "s_24");
				int numberOfResponses = 0;
				
				boolean hasResponse = false;
				
				boolean hasResponse0 = false;
				if (mtp_hideout_safe_tech_condition__defaultCondition (player, npc))
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
						responses[responseIndex++] = new string_id (c_stringFile, "s_26");
					}
					
					utils.setScriptVar (player, "conversation.mtp_hideout_safe_tech.branchId", 6);
					
					npcSpeak (player, message);
					npcSetConversationResponses (player, responses);
				}
				else
				{
					utils.removeScriptVar (player, "conversation.mtp_hideout_safe_tech.branchId");
					
					npcEndConversationWithMessage (player, message);
				}
				
				return SCRIPT_CONTINUE;
			}
			
		}
		
		return SCRIPT_DEFAULT;
	}
	
	
	public int mtp_hideout_safe_tech_handleBranch6(obj_id player, obj_id npc, string_id response) throws InterruptedException
	{
		
		if (response.equals("s_26"))
		{
			
			if (mtp_hideout_safe_tech_condition__defaultCondition (player, npc))
			{
				mtp_hideout_safe_tech_action_giveQuest (player, npc);
				
				string_id message = new string_id (c_stringFile, "s_28");
				utils.removeScriptVar (player, "conversation.mtp_hideout_safe_tech.branchId");
				
				npcEndConversationWithMessage (player, message);
				
				return SCRIPT_CONTINUE;
			}
			
		}
		
		return SCRIPT_DEFAULT;
	}
	
	
	public int mtp_hideout_safe_tech_handleBranch8(obj_id player, obj_id npc, string_id response) throws InterruptedException
	{
		
		if (response.equals("s_36"))
		{
			
			if (mtp_hideout_safe_tech_condition__defaultCondition (player, npc))
			{
				mtp_hideout_safe_tech_action_completeTask (player, npc);
				
				string_id message = new string_id (c_stringFile, "s_37");
				int numberOfResponses = 0;
				
				boolean hasResponse = false;
				
				boolean hasResponse0 = false;
				if (mtp_hideout_safe_tech_condition__defaultCondition (player, npc))
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
						responses[responseIndex++] = new string_id (c_stringFile, "s_38");
					}
					
					utils.setScriptVar (player, "conversation.mtp_hideout_safe_tech.branchId", 9);
					
					npcSpeak (player, message);
					npcSetConversationResponses (player, responses);
				}
				else
				{
					utils.removeScriptVar (player, "conversation.mtp_hideout_safe_tech.branchId");
					
					npcEndConversationWithMessage (player, message);
				}
				
				return SCRIPT_CONTINUE;
			}
			
		}
		
		return SCRIPT_DEFAULT;
	}
	
	
	public int mtp_hideout_safe_tech_handleBranch9(obj_id player, obj_id npc, string_id response) throws InterruptedException
	{
		
		if (response.equals("s_38"))
		{
			
			if (mtp_hideout_safe_tech_condition__defaultCondition (player, npc))
			{
				
				string_id message = new string_id (c_stringFile, "s_39");
				int numberOfResponses = 0;
				
				boolean hasResponse = false;
				
				boolean hasResponse0 = false;
				if (mtp_hideout_safe_tech_condition__defaultCondition (player, npc))
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
						responses[responseIndex++] = new string_id (c_stringFile, "s_40");
					}
					
					utils.setScriptVar (player, "conversation.mtp_hideout_safe_tech.branchId", 10);
					
					npcSpeak (player, message);
					npcSetConversationResponses (player, responses);
				}
				else
				{
					utils.removeScriptVar (player, "conversation.mtp_hideout_safe_tech.branchId");
					
					npcEndConversationWithMessage (player, message);
				}
				
				return SCRIPT_CONTINUE;
			}
			
		}
		
		return SCRIPT_DEFAULT;
	}
	
	
	public int mtp_hideout_safe_tech_handleBranch10(obj_id player, obj_id npc, string_id response) throws InterruptedException
	{
		
		if (response.equals("s_40"))
		{
			
			if (mtp_hideout_safe_tech_condition__defaultCondition (player, npc))
			{
				mtp_hideout_safe_tech_action_giveLocations (player, npc);
				
				string_id message = new string_id (c_stringFile, "s_41");
				utils.removeScriptVar (player, "conversation.mtp_hideout_safe_tech.branchId");
				
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
			detachScript(self, "conversation.mtp_hideout_safe_tech");
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
		detachScript (self, "conversation.mtp_hideout_safe_tech");
		
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
		
		if (mtp_hideout_safe_tech_condition_isEligible (player, npc))
		{
			
			string_id message = new string_id (c_stringFile, "s_4");
			int numberOfResponses = 0;
			
			boolean hasResponse = false;
			
			boolean hasResponse0 = false;
			if (mtp_hideout_safe_tech_condition__defaultCondition (player, npc))
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
					responses[responseIndex++] = new string_id (c_stringFile, "s_6");
				}
				
				utils.setScriptVar (player, "conversation.mtp_hideout_safe_tech.branchId", 1);
				
				npcStartConversation (player, npc, "mtp_hideout_safe_tech", message, responses);
			}
			else
			{
				chat.chat (npc, player, message);
			}
			
			return SCRIPT_CONTINUE;
		}
		
		if (mtp_hideout_safe_tech_condition_isReturningSuccess (player, npc))
		{
			
			string_id message = new string_id (c_stringFile, "s_35");
			int numberOfResponses = 0;
			
			boolean hasResponse = false;
			
			boolean hasResponse0 = false;
			if (mtp_hideout_safe_tech_condition__defaultCondition (player, npc))
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
					responses[responseIndex++] = new string_id (c_stringFile, "s_36");
				}
				
				utils.setScriptVar (player, "conversation.mtp_hideout_safe_tech.branchId", 8);
				
				npcStartConversation (player, npc, "mtp_hideout_safe_tech", message, responses);
			}
			else
			{
				chat.chat (npc, player, message);
			}
			
			return SCRIPT_CONTINUE;
		}
		
		if (mtp_hideout_safe_tech_condition_isActive (player, npc))
		{
			
			string_id message = new string_id (c_stringFile, "s_30");
			chat.chat (npc, player, message);
			
			return SCRIPT_CONTINUE;
		}
		
		if (mtp_hideout_safe_tech_condition_hasCompleted (player, npc))
		{
			
			string_id message = new string_id (c_stringFile, "s_32");
			chat.chat (npc, player, message);
			
			return SCRIPT_CONTINUE;
		}
		
		if (mtp_hideout_safe_tech_condition__defaultCondition (player, npc))
		{
			
			string_id message = new string_id (c_stringFile, "s_34");
			chat.chat (npc, player, message);
			
			return SCRIPT_CONTINUE;
		}
		
		chat.chat (npc, "Error: All conditions for OnStartNpcConversation were false.");
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int OnNpcConversationResponse(obj_id self, String conversationId, obj_id player, string_id response) throws InterruptedException
	{
		if (!conversationId.equals("mtp_hideout_safe_tech"))
		{
			return SCRIPT_CONTINUE;
		}
		
		obj_id npc = self;
		
		int branchId = utils.getIntScriptVar (player, "conversation.mtp_hideout_safe_tech.branchId");
		
		if (branchId == 1 && mtp_hideout_safe_tech_handleBranch1 (player, npc, response) == SCRIPT_CONTINUE)
		{
			return SCRIPT_CONTINUE;
		}
		
		if (branchId == 2 && mtp_hideout_safe_tech_handleBranch2 (player, npc, response) == SCRIPT_CONTINUE)
		{
			return SCRIPT_CONTINUE;
		}
		
		if (branchId == 3 && mtp_hideout_safe_tech_handleBranch3 (player, npc, response) == SCRIPT_CONTINUE)
		{
			return SCRIPT_CONTINUE;
		}
		
		if (branchId == 4 && mtp_hideout_safe_tech_handleBranch4 (player, npc, response) == SCRIPT_CONTINUE)
		{
			return SCRIPT_CONTINUE;
		}
		
		if (branchId == 5 && mtp_hideout_safe_tech_handleBranch5 (player, npc, response) == SCRIPT_CONTINUE)
		{
			return SCRIPT_CONTINUE;
		}
		
		if (branchId == 6 && mtp_hideout_safe_tech_handleBranch6 (player, npc, response) == SCRIPT_CONTINUE)
		{
			return SCRIPT_CONTINUE;
		}
		
		if (branchId == 8 && mtp_hideout_safe_tech_handleBranch8 (player, npc, response) == SCRIPT_CONTINUE)
		{
			return SCRIPT_CONTINUE;
		}
		
		if (branchId == 9 && mtp_hideout_safe_tech_handleBranch9 (player, npc, response) == SCRIPT_CONTINUE)
		{
			return SCRIPT_CONTINUE;
		}
		
		if (branchId == 10 && mtp_hideout_safe_tech_handleBranch10 (player, npc, response) == SCRIPT_CONTINUE)
		{
			return SCRIPT_CONTINUE;
		}
		
		chat.chat (npc, "Error: Fell through all branches and responses for OnNpcConversationResponse.");
		
		utils.removeScriptVar (player, "conversation.mtp_hideout_safe_tech.branchId");
		
		return SCRIPT_CONTINUE;
	}
	
}
