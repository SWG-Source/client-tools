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


public class disgruntled_leader extends script.base_script
{
	public disgruntled_leader()
	{
	}
	String c_stringFile = "conversation/disgruntled_leader";
	
	
	public boolean disgruntled_leader_condition__defaultCondition(obj_id player, obj_id npc) throws InterruptedException
	{
		return true;
	}
	
	
	public boolean disgruntled_leader_condition_hasQuest(obj_id player, obj_id npc) throws InterruptedException
	{
		faceTo(npc, player);
		return groundquests.isTaskActive(player, "u16_nym_meet_townspeople", "findTownsPeople");
		
	}
	
	
	public boolean disgruntled_leader_condition_hasFinishedQuest(obj_id player, obj_id npc) throws InterruptedException
	{
		faceTo(npc, player);
		return groundquests.isTaskActive(player, "u16_nym_return_townsperson", "returnToTownsPerson");
	}
	
	
	public boolean disgruntled_leader_condition_isWorkingOnIt(obj_id player, obj_id npc) throws InterruptedException
	{
		faceTo(npc,player);
		return groundquests.isTaskActive(player, "u16_nym_meet_townspeople", "findFiltrationGuy") || (groundquests.hasCompletedQuest(player, "u16_nym_meet_townspeople") && !groundquests.isQuestActiveOrComplete(player, "u16_nym_return_townsperson"));
	}
	
	
	public void disgruntled_leader_action_signalToGoToNym(obj_id player, obj_id npc) throws InterruptedException
	{
		groundquests.sendSignal(player, "hasFoundTownsPeopleAgain");
	}
	
	
	public void disgruntled_leader_action_hasMetTownsPeople(obj_id player, obj_id npc) throws InterruptedException
	{
		groundquests.sendSignal(player, "hasFoundTownsPeople");
	}
	
	
	public int disgruntled_leader_handleBranch1(obj_id player, obj_id npc, string_id response) throws InterruptedException
	{
		
		if (response.equals("s_17"))
		{
			doAnimationAction (player, "shake_head_disgust");
			
			if (disgruntled_leader_condition__defaultCondition (player, npc))
			{
				doAnimationAction (npc, "gesticulate_wildly");
				
				disgruntled_leader_action_signalToGoToNym (player, npc);
				
				string_id message = new string_id (c_stringFile, "s_18");
				utils.removeScriptVar (player, "conversation.disgruntled_leader.branchId");
				
				npcEndConversationWithMessage (player, message);
				
				return SCRIPT_CONTINUE;
			}
			
		}
		
		return SCRIPT_DEFAULT;
	}
	
	
	public int disgruntled_leader_handleBranch4(obj_id player, obj_id npc, string_id response) throws InterruptedException
	{
		
		if (response.equals("s_10"))
		{
			doAnimationAction (player, "point_to_self");
			
			if (disgruntled_leader_condition__defaultCondition (player, npc))
			{
				doAnimationAction (npc, "point_accusingly");
				
				string_id message = new string_id (c_stringFile, "s_12");
				int numberOfResponses = 0;
				
				boolean hasResponse = false;
				
				boolean hasResponse0 = false;
				if (disgruntled_leader_condition__defaultCondition (player, npc))
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
					
					utils.setScriptVar (player, "conversation.disgruntled_leader.branchId", 5);
					
					npcSpeak (player, message);
					npcSetConversationResponses (player, responses);
				}
				else
				{
					utils.removeScriptVar (player, "conversation.disgruntled_leader.branchId");
					
					npcEndConversationWithMessage (player, message);
				}
				
				return SCRIPT_CONTINUE;
			}
			
		}
		
		return SCRIPT_DEFAULT;
	}
	
	
	public int disgruntled_leader_handleBranch5(obj_id player, obj_id npc, string_id response) throws InterruptedException
	{
		
		if (response.equals("s_14"))
		{
			doAnimationAction (player, "taken_aback");
			
			if (disgruntled_leader_condition__defaultCondition (player, npc))
			{
				doAnimationAction (npc, "poke");
				
				string_id message = new string_id (c_stringFile, "s_19");
				int numberOfResponses = 0;
				
				boolean hasResponse = false;
				
				boolean hasResponse0 = false;
				if (disgruntled_leader_condition__defaultCondition (player, npc))
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
						responses[responseIndex++] = new string_id (c_stringFile, "s_21");
					}
					
					utils.setScriptVar (player, "conversation.disgruntled_leader.branchId", 6);
					
					npcSpeak (player, message);
					npcSetConversationResponses (player, responses);
				}
				else
				{
					utils.removeScriptVar (player, "conversation.disgruntled_leader.branchId");
					
					npcEndConversationWithMessage (player, message);
				}
				
				return SCRIPT_CONTINUE;
			}
			
		}
		
		return SCRIPT_DEFAULT;
	}
	
	
	public int disgruntled_leader_handleBranch6(obj_id player, obj_id npc, string_id response) throws InterruptedException
	{
		
		if (response.equals("s_21"))
		{
			doAnimationAction (player, "taken_aback");
			
			if (disgruntled_leader_condition__defaultCondition (player, npc))
			{
				doAnimationAction (npc, "gesticulate_wildly");
				
				string_id message = new string_id (c_stringFile, "s_23");
				int numberOfResponses = 0;
				
				boolean hasResponse = false;
				
				boolean hasResponse0 = false;
				if (disgruntled_leader_condition__defaultCondition (player, npc))
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
						responses[responseIndex++] = new string_id (c_stringFile, "s_25");
					}
					
					utils.setScriptVar (player, "conversation.disgruntled_leader.branchId", 7);
					
					npcSpeak (player, message);
					npcSetConversationResponses (player, responses);
				}
				else
				{
					utils.removeScriptVar (player, "conversation.disgruntled_leader.branchId");
					
					npcEndConversationWithMessage (player, message);
				}
				
				return SCRIPT_CONTINUE;
			}
			
		}
		
		return SCRIPT_DEFAULT;
	}
	
	
	public int disgruntled_leader_handleBranch7(obj_id player, obj_id npc, string_id response) throws InterruptedException
	{
		
		if (response.equals("s_25"))
		{
			
			if (disgruntled_leader_condition__defaultCondition (player, npc))
			{
				doAnimationAction (npc, "point_away");
				
				disgruntled_leader_action_hasMetTownsPeople (player, npc);
				
				string_id message = new string_id (c_stringFile, "s_27");
				utils.removeScriptVar (player, "conversation.disgruntled_leader.branchId");
				
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
			detachScript(self, "conversation.disgruntled_leader");
		}
		
		setCondition (self, CONDITION_CONVERSABLE);
		setCondition( self, CONDITION_INTERESTING );
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int OnAttach(obj_id self) throws InterruptedException
	{
		setCondition (self, CONDITION_CONVERSABLE);
		setCondition( self, CONDITION_INTERESTING );
		
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
		detachScript (self, "conversation.disgruntled_leader");
		
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
		
		if (disgruntled_leader_condition_hasFinishedQuest (player, npc))
		{
			doAnimationAction (npc, "gesticulate_wildly");
			
			string_id message = new string_id (c_stringFile, "s_16");
			int numberOfResponses = 0;
			
			boolean hasResponse = false;
			
			boolean hasResponse0 = false;
			if (disgruntled_leader_condition__defaultCondition (player, npc))
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
					responses[responseIndex++] = new string_id (c_stringFile, "s_17");
				}
				
				utils.setScriptVar (player, "conversation.disgruntled_leader.branchId", 1);
				
				npcStartConversation (player, npc, "disgruntled_leader", message, responses);
			}
			else
			{
				chat.chat (npc, player, message);
			}
			
			return SCRIPT_CONTINUE;
		}
		
		if (disgruntled_leader_condition_isWorkingOnIt (player, npc))
		{
			doAnimationAction (npc, "gesticulate_wildly");
			
			doAnimationAction (player, "standing_placate");
			
			string_id message = new string_id (c_stringFile, "s_29");
			chat.chat (npc, player, message);
			
			return SCRIPT_CONTINUE;
		}
		
		if (disgruntled_leader_condition_hasQuest (player, npc))
		{
			doAnimationAction (npc, "gesticulate_wildly");
			
			string_id message = new string_id (c_stringFile, "s_8");
			int numberOfResponses = 0;
			
			boolean hasResponse = false;
			
			boolean hasResponse0 = false;
			if (disgruntled_leader_condition__defaultCondition (player, npc))
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
				
				utils.setScriptVar (player, "conversation.disgruntled_leader.branchId", 4);
				
				npcStartConversation (player, npc, "disgruntled_leader", message, responses);
			}
			else
			{
				chat.chat (npc, player, message);
			}
			
			return SCRIPT_CONTINUE;
		}
		
		if (disgruntled_leader_condition__defaultCondition (player, npc))
		{
			doAnimationAction (npc, "gesticulate_wildly");
			
			string_id message = new string_id (c_stringFile, "s_30");
			chat.chat (npc, player, message);
			
			return SCRIPT_CONTINUE;
		}
		
		chat.chat (npc, "Error: All conditions for OnStartNpcConversation were false.");
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int OnNpcConversationResponse(obj_id self, String conversationId, obj_id player, string_id response) throws InterruptedException
	{
		if (!conversationId.equals("disgruntled_leader"))
		{
			return SCRIPT_CONTINUE;
		}
		
		obj_id npc = self;
		
		int branchId = utils.getIntScriptVar (player, "conversation.disgruntled_leader.branchId");
		
		if (branchId == 1 && disgruntled_leader_handleBranch1 (player, npc, response) == SCRIPT_CONTINUE)
		{
			return SCRIPT_CONTINUE;
		}
		
		if (branchId == 4 && disgruntled_leader_handleBranch4 (player, npc, response) == SCRIPT_CONTINUE)
		{
			return SCRIPT_CONTINUE;
		}
		
		if (branchId == 5 && disgruntled_leader_handleBranch5 (player, npc, response) == SCRIPT_CONTINUE)
		{
			return SCRIPT_CONTINUE;
		}
		
		if (branchId == 6 && disgruntled_leader_handleBranch6 (player, npc, response) == SCRIPT_CONTINUE)
		{
			return SCRIPT_CONTINUE;
		}
		
		if (branchId == 7 && disgruntled_leader_handleBranch7 (player, npc, response) == SCRIPT_CONTINUE)
		{
			return SCRIPT_CONTINUE;
		}
		
		chat.chat (npc, "Error: Fell through all branches and responses for OnNpcConversationResponse.");
		
		utils.removeScriptVar (player, "conversation.disgruntled_leader.branchId");
		
		return SCRIPT_CONTINUE;
	}
	
}
