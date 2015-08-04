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
import script.library.collection;
import script.library.conversation;
import script.library.groundquests;
import script.library.static_item;
import script.library.utils;


public class ran_machado extends script.base_script
{
	public ran_machado()
	{
	}
	String c_stringFile = "conversation/ran_machado";
	
	
	public boolean ran_machado_condition__defaultCondition(obj_id player, obj_id npc) throws InterruptedException
	{
		return true;
	}
	
	
	public boolean ran_machado_condition_isElligibleFirstTime(obj_id player, obj_id npc) throws InterruptedException
	{
		faceTo(npc,player);
		return !groundquests.isQuestActiveOrComplete(player, "u16_nym_themepark_weed_pulling");
	}
	
	
	public boolean ran_machado_condition_hasCompletedReq(obj_id player, obj_id npc) throws InterruptedException
	{
		faceTo(npc,player);
		return groundquests.isTaskActive(player, "u16_nym_themepark_weed_pulling", "returnForPay");
	}
	
	
	public boolean ran_machado_condition_isElligibleSecondTime(obj_id player, obj_id npc) throws InterruptedException
	{
		faceTo(npc,player);
		return !groundquests.isQuestActive(player, "u16_nym_themepark_weed_pulling") && groundquests.hasCompletedQuest(player, "u16_nym_themepark_weed_pulling");
	}
	
	
	public boolean ran_machado_condition_hasQuestActive(obj_id player, obj_id npc) throws InterruptedException
	{
		faceTo(npc,player);
		return groundquests.isQuestActive(player, "u16_nym_themepark_weed_pulling");
	}
	
	
	public boolean ran_machado_condition_hasFailQuest(obj_id player, obj_id npc) throws InterruptedException
	{
		return groundquests.isQuestActive(player, "u16_nym_themepark_weed_pulling_fail");
	}
	
	
	public void ran_machado_action_giveQuest(obj_id player, obj_id npc) throws InterruptedException
	{
		if (groundquests.hasCompletedQuest(player, "u16_nym_themepark_weed_pulling"))
		{
			groundquests.clearQuest(player, "u16_nym_themepark_weed_pulling");
		}
		if (!groundquests.isQuestActive(player, "u16_nym_themepark_weed_pulling"))
		{
			groundquests.grantQuest(player, "u16_nym_themepark_weed_pulling");
		}
		
	}
	
	
	public void ran_machado_action_rewardPlayer(obj_id player, obj_id npc) throws InterruptedException
	{
		obj_id pInv = utils.getInventoryContainer(player);
		if (isValidId(pInv) && exists(pInv))
		{
			groundquests.sendSignal(player, "hasBeenPaid");
			collection.getRandomCollectionItemShowLootBox(player, pInv, "datatables/loot/dungeon/nym_weed_loot.iff", "weeds");
		}
	}
	
	
	public void ran_machado_action_clearFail(obj_id player, obj_id npc) throws InterruptedException
	{
		groundquests.clearQuest(player, "u16_nym_themepark_weed_pulling_fail");
	}
	
	
	public int ran_machado_handleBranch2(obj_id player, obj_id npc, string_id response) throws InterruptedException
	{
		
		if (response.equals("s_34"))
		{
			
			if (ran_machado_condition__defaultCondition (player, npc))
			{
				ran_machado_action_giveQuest (player, npc);
				
				string_id message = new string_id (c_stringFile, "s_36");
				utils.removeScriptVar (player, "conversation.ran_machado.branchId");
				
				npcEndConversationWithMessage (player, message);
				
				return SCRIPT_CONTINUE;
			}
			
		}
		
		return SCRIPT_DEFAULT;
	}
	
	
	public int ran_machado_handleBranch5(obj_id player, obj_id npc, string_id response) throws InterruptedException
	{
		
		if (response.equals("s_19"))
		{
			
			if (ran_machado_condition__defaultCondition (player, npc))
			{
				doAnimationAction (npc, "thank");
				
				ran_machado_action_giveQuest (player, npc);
				
				string_id message = new string_id (c_stringFile, "s_20");
				utils.removeScriptVar (player, "conversation.ran_machado.branchId");
				
				prose_package pp = new prose_package ();
				pp.stringId = message;
				pp.actor.set (player);
				pp.target.set (npc);
				
				npcEndConversationWithMessage (player, pp);
				
				return SCRIPT_CONTINUE;
			}
			
		}
		
		return SCRIPT_DEFAULT;
	}
	
	
	public int ran_machado_handleBranch7(obj_id player, obj_id npc, string_id response) throws InterruptedException
	{
		
		if (response.equals("s_14"))
		{
			
			if (ran_machado_condition__defaultCondition (player, npc))
			{
				doAnimationAction (npc, "explain");
				
				string_id message = new string_id (c_stringFile, "s_21");
				int numberOfResponses = 0;
				
				boolean hasResponse = false;
				
				boolean hasResponse0 = false;
				if (ran_machado_condition__defaultCondition (player, npc))
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
						responses[responseIndex++] = new string_id (c_stringFile, "s_23");
					}
					
					utils.setScriptVar (player, "conversation.ran_machado.branchId", 8);
					
					npcSpeak (player, message);
					npcSetConversationResponses (player, responses);
				}
				else
				{
					utils.removeScriptVar (player, "conversation.ran_machado.branchId");
					
					npcEndConversationWithMessage (player, message);
				}
				
				return SCRIPT_CONTINUE;
			}
			
		}
		
		return SCRIPT_DEFAULT;
	}
	
	
	public int ran_machado_handleBranch8(obj_id player, obj_id npc, string_id response) throws InterruptedException
	{
		
		if (response.equals("s_23"))
		{
			
			if (ran_machado_condition__defaultCondition (player, npc))
			{
				doAnimationAction (npc, "forage");
				
				string_id message = new string_id (c_stringFile, "s_25");
				int numberOfResponses = 0;
				
				boolean hasResponse = false;
				
				boolean hasResponse0 = false;
				if (ran_machado_condition__defaultCondition (player, npc))
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
						responses[responseIndex++] = new string_id (c_stringFile, "s_27");
					}
					
					utils.setScriptVar (player, "conversation.ran_machado.branchId", 9);
					
					npcSpeak (player, message);
					npcSetConversationResponses (player, responses);
				}
				else
				{
					utils.removeScriptVar (player, "conversation.ran_machado.branchId");
					
					npcEndConversationWithMessage (player, message);
				}
				
				return SCRIPT_CONTINUE;
			}
			
		}
		
		return SCRIPT_DEFAULT;
	}
	
	
	public int ran_machado_handleBranch9(obj_id player, obj_id npc, string_id response) throws InterruptedException
	{
		
		if (response.equals("s_27"))
		{
			
			if (ran_machado_condition__defaultCondition (player, npc))
			{
				doAnimationAction (npc, "point_left");
				
				string_id message = new string_id (c_stringFile, "s_29");
				int numberOfResponses = 0;
				
				boolean hasResponse = false;
				
				boolean hasResponse0 = false;
				if (ran_machado_condition__defaultCondition (player, npc))
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
					
					utils.setScriptVar (player, "conversation.ran_machado.branchId", 10);
					
					npcSpeak (player, message);
					npcSetConversationResponses (player, responses);
				}
				else
				{
					utils.removeScriptVar (player, "conversation.ran_machado.branchId");
					
					npcEndConversationWithMessage (player, message);
				}
				
				return SCRIPT_CONTINUE;
			}
			
		}
		
		return SCRIPT_DEFAULT;
	}
	
	
	public int ran_machado_handleBranch10(obj_id player, obj_id npc, string_id response) throws InterruptedException
	{
		
		if (response.equals("s_31"))
		{
			
			if (ran_machado_condition__defaultCondition (player, npc))
			{
				doAnimationAction (npc, "shoo");
				
				ran_machado_action_giveQuest (player, npc);
				
				string_id message = new string_id (c_stringFile, "s_35");
				utils.removeScriptVar (player, "conversation.ran_machado.branchId");
				
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
			detachScript(self, "conversation.ran_machado");
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
		detachScript (self, "conversation.ran_machado");
		
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
		
		if (ran_machado_condition_hasCompletedReq (player, npc))
		{
			doAnimationAction (npc, "celebrate");
			
			ran_machado_action_rewardPlayer (player, npc);
			
			string_id message = new string_id (c_stringFile, "s_17");
			chat.chat (npc, player, message);
			
			return SCRIPT_CONTINUE;
		}
		
		if (ran_machado_condition_hasFailQuest (player, npc))
		{
			doAnimationAction (npc, "backhand_threaten");
			
			ran_machado_action_clearFail (player, npc);
			
			string_id message = new string_id (c_stringFile, "s_33");
			int numberOfResponses = 0;
			
			boolean hasResponse = false;
			
			boolean hasResponse0 = false;
			if (ran_machado_condition__defaultCondition (player, npc))
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
				
				utils.setScriptVar (player, "conversation.ran_machado.branchId", 2);
				
				prose_package pp = new prose_package ();
				pp.stringId = message;
				pp.actor.set (player);
				pp.target.set (npc);
				
				npcStartConversation (player, npc, "ran_machado", null, pp, responses);
			}
			else
			{
				prose_package pp = new prose_package ();
				pp.stringId = message;
				pp.actor.set (player);
				pp.target.set (npc);
				
				chat.chat (npc, player, null, null, pp);
			}
			
			return SCRIPT_CONTINUE;
		}
		
		if (ran_machado_condition_hasQuestActive (player, npc))
		{
			doAnimationAction (npc, "shakefist");
			
			string_id message = new string_id (c_stringFile, "s_16");
			chat.chat (npc, player, message);
			
			return SCRIPT_CONTINUE;
		}
		
		if (ran_machado_condition_isElligibleSecondTime (player, npc))
		{
			doAnimationAction (npc, "celebrate");
			
			string_id message = new string_id (c_stringFile, "s_18");
			int numberOfResponses = 0;
			
			boolean hasResponse = false;
			
			boolean hasResponse0 = false;
			if (ran_machado_condition__defaultCondition (player, npc))
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
					responses[responseIndex++] = new string_id (c_stringFile, "s_19");
				}
				
				utils.setScriptVar (player, "conversation.ran_machado.branchId", 5);
				
				npcStartConversation (player, npc, "ran_machado", message, responses);
			}
			else
			{
				chat.chat (npc, player, message);
			}
			
			return SCRIPT_CONTINUE;
		}
		
		if (ran_machado_condition_isElligibleFirstTime (player, npc))
		{
			doAnimationAction (npc, "explain");
			
			string_id message = new string_id (c_stringFile, "s_12");
			int numberOfResponses = 0;
			
			boolean hasResponse = false;
			
			boolean hasResponse0 = false;
			if (ran_machado_condition__defaultCondition (player, npc))
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
				
				utils.setScriptVar (player, "conversation.ran_machado.branchId", 7);
				
				npcStartConversation (player, npc, "ran_machado", message, responses);
			}
			else
			{
				chat.chat (npc, player, message);
			}
			
			return SCRIPT_CONTINUE;
		}
		
		if (ran_machado_condition__defaultCondition (player, npc))
		{
			
			string_id message = new string_id (c_stringFile, "s_38");
			chat.chat (npc, player, message);
			
			return SCRIPT_CONTINUE;
		}
		
		chat.chat (npc, "Error: All conditions for OnStartNpcConversation were false.");
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int OnNpcConversationResponse(obj_id self, String conversationId, obj_id player, string_id response) throws InterruptedException
	{
		if (!conversationId.equals("ran_machado"))
		{
			return SCRIPT_CONTINUE;
		}
		
		obj_id npc = self;
		
		int branchId = utils.getIntScriptVar (player, "conversation.ran_machado.branchId");
		
		if (branchId == 2 && ran_machado_handleBranch2 (player, npc, response) == SCRIPT_CONTINUE)
		{
			return SCRIPT_CONTINUE;
		}
		
		if (branchId == 5 && ran_machado_handleBranch5 (player, npc, response) == SCRIPT_CONTINUE)
		{
			return SCRIPT_CONTINUE;
		}
		
		if (branchId == 7 && ran_machado_handleBranch7 (player, npc, response) == SCRIPT_CONTINUE)
		{
			return SCRIPT_CONTINUE;
		}
		
		if (branchId == 8 && ran_machado_handleBranch8 (player, npc, response) == SCRIPT_CONTINUE)
		{
			return SCRIPT_CONTINUE;
		}
		
		if (branchId == 9 && ran_machado_handleBranch9 (player, npc, response) == SCRIPT_CONTINUE)
		{
			return SCRIPT_CONTINUE;
		}
		
		if (branchId == 10 && ran_machado_handleBranch10 (player, npc, response) == SCRIPT_CONTINUE)
		{
			return SCRIPT_CONTINUE;
		}
		
		chat.chat (npc, "Error: Fell through all branches and responses for OnNpcConversationResponse.");
		
		utils.removeScriptVar (player, "conversation.ran_machado.branchId");
		
		return SCRIPT_CONTINUE;
	}
	
}
