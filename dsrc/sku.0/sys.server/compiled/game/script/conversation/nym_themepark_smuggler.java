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


public class nym_themepark_smuggler extends script.base_script
{
	public nym_themepark_smuggler()
	{
	}
	String c_stringFile = "conversation/nym_themepark_smuggler";
	
	
	public boolean nym_themepark_smuggler_condition__defaultCondition(obj_id player, obj_id npc) throws InterruptedException
	{
		return true;
	}
	
	
	public boolean nym_themepark_smuggler_condition_isReadyToGetChems(obj_id player, obj_id npc) throws InterruptedException
	{
		return groundquests.isTaskActive(player, "u16_nym_themepark_smuggler_01", "findSmuggler") || (groundquests.hasCompletedQuest(player, "u16_nym_themepark_smuggler_01") && !groundquests.isQuestActiveOrComplete(player, "u16_nym_themepark_gather_chemicals"));
	}
	
	
	public boolean nym_themepark_smuggler_condition_hasChems(obj_id player, obj_id npc) throws InterruptedException
	{
		return groundquests.isTaskActive(player, "u16_nym_themepark_gather_chemicals", "goSeeSmuggler");
	}
	
	
	public boolean nym_themepark_smuggler_condition_deletedFixQuest(obj_id player, obj_id npc) throws InterruptedException
	{
		return groundquests.hasCompletedQuest(player, "u16_nym_themepark_gather_chemicals") && !groundquests.isQuestActiveOrComplete(player, "u16_nym_themepark_repair_mixing_chamber");
	}
	
	
	public void nym_themepark_smuggler_action_giveChemQuest(obj_id player, obj_id npc) throws InterruptedException
	{
		groundquests.grantQuest(player, "u16_nym_themepark_gather_chemicals");
	}
	
	
	public void nym_themepark_smuggler_action_finishUpchem(obj_id player, obj_id npc) throws InterruptedException
	{
		groundquests.sendSignal(player, "hasSeenSmuggler");
	}
	
	
	public void nym_themepark_smuggler_action_finishUpJumper(obj_id player, obj_id npc) throws InterruptedException
	{
		groundquests.sendSignal(player, "hasFoundSmuggler");
	}
	
	
	public void nym_themepark_smuggler_action_giveFixQuest(obj_id player, obj_id npc) throws InterruptedException
	{
		groundquests.grantQuest(player, "u16_nym_themepark_repair_mixing_chamber");
	}
	
	
	public int nym_themepark_smuggler_handleBranch2(obj_id player, obj_id npc, string_id response) throws InterruptedException
	{
		
		if (response.equals("s_11"))
		{
			
			if (nym_themepark_smuggler_condition__defaultCondition (player, npc))
			{
				nym_themepark_smuggler_action_giveFixQuest (player, npc);
				
				string_id message = new string_id (c_stringFile, "s_12");
				utils.removeScriptVar (player, "conversation.nym_themepark_smuggler.branchId");
				
				npcEndConversationWithMessage (player, message);
				
				return SCRIPT_CONTINUE;
			}
			
		}
		
		return SCRIPT_DEFAULT;
	}
	
	
	public int nym_themepark_smuggler_handleBranch4(obj_id player, obj_id npc, string_id response) throws InterruptedException
	{
		
		if (response.equals("s_9"))
		{
			
			if (nym_themepark_smuggler_condition__defaultCondition (player, npc))
			{
				
				string_id message = new string_id (c_stringFile, "s_14");
				int numberOfResponses = 0;
				
				boolean hasResponse = false;
				
				boolean hasResponse0 = false;
				if (nym_themepark_smuggler_condition__defaultCondition (player, npc))
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
					
					utils.setScriptVar (player, "conversation.nym_themepark_smuggler.branchId", 5);
					
					npcSpeak (player, message);
					npcSetConversationResponses (player, responses);
				}
				else
				{
					utils.removeScriptVar (player, "conversation.nym_themepark_smuggler.branchId");
					
					npcEndConversationWithMessage (player, message);
				}
				
				return SCRIPT_CONTINUE;
			}
			
		}
		
		return SCRIPT_DEFAULT;
	}
	
	
	public int nym_themepark_smuggler_handleBranch5(obj_id player, obj_id npc, string_id response) throws InterruptedException
	{
		
		if (response.equals("s_17"))
		{
			
			if (nym_themepark_smuggler_condition__defaultCondition (player, npc))
			{
				
				string_id message = new string_id (c_stringFile, "s_18");
				int numberOfResponses = 0;
				
				boolean hasResponse = false;
				
				boolean hasResponse0 = false;
				if (nym_themepark_smuggler_condition__defaultCondition (player, npc))
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
					
					utils.setScriptVar (player, "conversation.nym_themepark_smuggler.branchId", 6);
					
					npcSpeak (player, message);
					npcSetConversationResponses (player, responses);
				}
				else
				{
					utils.removeScriptVar (player, "conversation.nym_themepark_smuggler.branchId");
					
					npcEndConversationWithMessage (player, message);
				}
				
				return SCRIPT_CONTINUE;
			}
			
		}
		
		return SCRIPT_DEFAULT;
	}
	
	
	public int nym_themepark_smuggler_handleBranch6(obj_id player, obj_id npc, string_id response) throws InterruptedException
	{
		
		if (response.equals("s_20"))
		{
			
			if (nym_themepark_smuggler_condition__defaultCondition (player, npc))
			{
				
				string_id message = new string_id (c_stringFile, "s_22");
				int numberOfResponses = 0;
				
				boolean hasResponse = false;
				
				boolean hasResponse0 = false;
				if (nym_themepark_smuggler_condition__defaultCondition (player, npc))
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
					
					utils.setScriptVar (player, "conversation.nym_themepark_smuggler.branchId", 7);
					
					npcSpeak (player, message);
					npcSetConversationResponses (player, responses);
				}
				else
				{
					utils.removeScriptVar (player, "conversation.nym_themepark_smuggler.branchId");
					
					npcEndConversationWithMessage (player, message);
				}
				
				return SCRIPT_CONTINUE;
			}
			
		}
		
		return SCRIPT_DEFAULT;
	}
	
	
	public int nym_themepark_smuggler_handleBranch7(obj_id player, obj_id npc, string_id response) throws InterruptedException
	{
		
		if (response.equals("s_24"))
		{
			
			if (nym_themepark_smuggler_condition__defaultCondition (player, npc))
			{
				
				string_id message = new string_id (c_stringFile, "s_26");
				int numberOfResponses = 0;
				
				boolean hasResponse = false;
				
				boolean hasResponse0 = false;
				if (nym_themepark_smuggler_condition__defaultCondition (player, npc))
				{
					++numberOfResponses;
					hasResponse = true;
					hasResponse0 = true;
				}
				
				boolean hasResponse1 = false;
				if (nym_themepark_smuggler_condition__defaultCondition (player, npc))
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
						responses[responseIndex++] = new string_id (c_stringFile, "s_28");
					}
					
					if (hasResponse1)
					{
						responses[responseIndex++] = new string_id (c_stringFile, "s_32");
					}
					
					utils.setScriptVar (player, "conversation.nym_themepark_smuggler.branchId", 8);
					
					npcSpeak (player, message);
					npcSetConversationResponses (player, responses);
				}
				else
				{
					utils.removeScriptVar (player, "conversation.nym_themepark_smuggler.branchId");
					
					npcEndConversationWithMessage (player, message);
				}
				
				return SCRIPT_CONTINUE;
			}
			
		}
		
		return SCRIPT_DEFAULT;
	}
	
	
	public int nym_themepark_smuggler_handleBranch8(obj_id player, obj_id npc, string_id response) throws InterruptedException
	{
		
		if (response.equals("s_28"))
		{
			
			if (nym_themepark_smuggler_condition__defaultCondition (player, npc))
			{
				nym_themepark_smuggler_action_giveChemQuest (player, npc);
				
				string_id message = new string_id (c_stringFile, "s_30");
				utils.removeScriptVar (player, "conversation.nym_themepark_smuggler.branchId");
				
				npcEndConversationWithMessage (player, message);
				
				return SCRIPT_CONTINUE;
			}
			
		}
		
		if (response.equals("s_32"))
		{
			
			if (nym_themepark_smuggler_condition__defaultCondition (player, npc))
			{
				doAnimationAction (npc, "wave_on_dismissing");
				
				string_id message = new string_id (c_stringFile, "s_34");
				utils.removeScriptVar (player, "conversation.nym_themepark_smuggler.branchId");
				
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
			detachScript(self, "conversation.nym_themepark_smuggler");
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
		detachScript (self, "conversation.nym_themepark_smuggler");
		
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
		
		if (nym_themepark_smuggler_condition_deletedFixQuest (player, npc))
		{
			nym_themepark_smuggler_action_giveFixQuest (player, npc);
			
			string_id message = new string_id (c_stringFile, "s_37");
			chat.chat (npc, player, message);
			
			return SCRIPT_CONTINUE;
		}
		
		if (nym_themepark_smuggler_condition_hasChems (player, npc))
		{
			nym_themepark_smuggler_action_finishUpchem (player, npc);
			
			string_id message = new string_id (c_stringFile, "s_10");
			int numberOfResponses = 0;
			
			boolean hasResponse = false;
			
			boolean hasResponse0 = false;
			if (nym_themepark_smuggler_condition__defaultCondition (player, npc))
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
				
				utils.setScriptVar (player, "conversation.nym_themepark_smuggler.branchId", 2);
				
				npcStartConversation (player, npc, "nym_themepark_smuggler", message, responses);
			}
			else
			{
				chat.chat (npc, player, message);
			}
			
			return SCRIPT_CONTINUE;
		}
		
		if (nym_themepark_smuggler_condition_isReadyToGetChems (player, npc))
		{
			doAnimationAction (npc, "gesticulate_wildly");
			
			nym_themepark_smuggler_action_finishUpJumper (player, npc);
			
			string_id message = new string_id (c_stringFile, "s_7");
			int numberOfResponses = 0;
			
			boolean hasResponse = false;
			
			boolean hasResponse0 = false;
			if (nym_themepark_smuggler_condition__defaultCondition (player, npc))
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
					responses[responseIndex++] = new string_id (c_stringFile, "s_9");
				}
				
				utils.setScriptVar (player, "conversation.nym_themepark_smuggler.branchId", 4);
				
				npcStartConversation (player, npc, "nym_themepark_smuggler", message, responses);
			}
			else
			{
				chat.chat (npc, player, message);
			}
			
			return SCRIPT_CONTINUE;
		}
		
		if (nym_themepark_smuggler_condition__defaultCondition (player, npc))
		{
			
			string_id message = new string_id (c_stringFile, "s_36");
			chat.chat (npc, player, message);
			
			return SCRIPT_CONTINUE;
		}
		
		chat.chat (npc, "Error: All conditions for OnStartNpcConversation were false.");
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int OnNpcConversationResponse(obj_id self, String conversationId, obj_id player, string_id response) throws InterruptedException
	{
		if (!conversationId.equals("nym_themepark_smuggler"))
		{
			return SCRIPT_CONTINUE;
		}
		
		obj_id npc = self;
		
		int branchId = utils.getIntScriptVar (player, "conversation.nym_themepark_smuggler.branchId");
		
		if (branchId == 2 && nym_themepark_smuggler_handleBranch2 (player, npc, response) == SCRIPT_CONTINUE)
		{
			return SCRIPT_CONTINUE;
		}
		
		if (branchId == 4 && nym_themepark_smuggler_handleBranch4 (player, npc, response) == SCRIPT_CONTINUE)
		{
			return SCRIPT_CONTINUE;
		}
		
		if (branchId == 5 && nym_themepark_smuggler_handleBranch5 (player, npc, response) == SCRIPT_CONTINUE)
		{
			return SCRIPT_CONTINUE;
		}
		
		if (branchId == 6 && nym_themepark_smuggler_handleBranch6 (player, npc, response) == SCRIPT_CONTINUE)
		{
			return SCRIPT_CONTINUE;
		}
		
		if (branchId == 7 && nym_themepark_smuggler_handleBranch7 (player, npc, response) == SCRIPT_CONTINUE)
		{
			return SCRIPT_CONTINUE;
		}
		
		if (branchId == 8 && nym_themepark_smuggler_handleBranch8 (player, npc, response) == SCRIPT_CONTINUE)
		{
			return SCRIPT_CONTINUE;
		}
		
		chat.chat (npc, "Error: Fell through all branches and responses for OnNpcConversationResponse.");
		
		utils.removeScriptVar (player, "conversation.nym_themepark_smuggler.branchId");
		
		return SCRIPT_CONTINUE;
	}
	
}
