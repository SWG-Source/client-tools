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


public class happer_mos_eisley extends script.base_script
{
	public happer_mos_eisley()
	{
	}
	String c_stringFile = "conversation/happer_mos_eisley";
	
	
	public boolean happer_mos_eisley_condition__defaultCondition(obj_id player, obj_id npc) throws InterruptedException
	{
		return true;
	}
	
	
	public boolean happer_mos_eisley_condition_hasInvestigateQuest(obj_id player, obj_id npc) throws InterruptedException
	{
		faceTo(npc, player);
		return groundquests.isQuestActive(player, "quest_02_investigate_murder");
	}
	
	
	public boolean happer_mos_eisley_condition_hasCompletedInvestigate(obj_id player, obj_id npc) throws InterruptedException
	{
		faceTo(npc, player);
		return groundquests.hasCompletedQuest(player, "quest_02_investigate_murder");
	}
	
	
	public boolean happer_mos_eisley_condition_hasGatheredTheClue(obj_id player, obj_id npc) throws InterruptedException
	{
		faceTo(npc, player);
		return groundquests.hasCompletedTask(player, "quest_02_investigate_murder", "getClues") || happer_mos_eisley_condition_hasCompletedInvestigate(player, npc);
	}
	
	
	public void happer_mos_eisley_action_sendSignalClueUpdate(obj_id player, obj_id npc) throws InterruptedException
	{
		groundquests.sendSignal(player, "getCluesHapper");
	}
	
	
	public void happer_mos_eisley_action_sendSignalIdentifyBody(obj_id player, obj_id npc) throws InterruptedException
	{
		groundquests.sendSignal(player, "happerIdentifyBody");
	}
	
	
	public void happer_mos_eisley_action_sendSignalTalkHapper(obj_id player, obj_id npc) throws InterruptedException
	{
		groundquests.sendSignal(player, "speakWithHapper");
	}
	
	
	public int happer_mos_eisley_handleBranch2(obj_id player, obj_id npc, string_id response) throws InterruptedException
	{
		
		if (response.equals("s_9"))
		{
			
			if (happer_mos_eisley_condition__defaultCondition (player, npc))
			{
				happer_mos_eisley_action_sendSignalIdentifyBody (player, npc);
				
				string_id message = new string_id (c_stringFile, "s_10");
				int numberOfResponses = 0;
				
				boolean hasResponse = false;
				
				boolean hasResponse0 = false;
				if (happer_mos_eisley_condition__defaultCondition (player, npc))
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
					
					utils.setScriptVar (player, "conversation.happer_mos_eisley.branchId", 3);
					
					npcSpeak (player, message);
					npcSetConversationResponses (player, responses);
				}
				else
				{
					utils.removeScriptVar (player, "conversation.happer_mos_eisley.branchId");
					
					npcEndConversationWithMessage (player, message);
				}
				
				return SCRIPT_CONTINUE;
			}
			
		}
		
		return SCRIPT_DEFAULT;
	}
	
	
	public int happer_mos_eisley_handleBranch3(obj_id player, obj_id npc, string_id response) throws InterruptedException
	{
		
		if (response.equals("s_11"))
		{
			
			if (happer_mos_eisley_condition__defaultCondition (player, npc))
			{
				
				string_id message = new string_id (c_stringFile, "s_12");
				int numberOfResponses = 0;
				
				boolean hasResponse = false;
				
				boolean hasResponse0 = false;
				if (happer_mos_eisley_condition__defaultCondition (player, npc))
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
					
					utils.setScriptVar (player, "conversation.happer_mos_eisley.branchId", 4);
					
					npcSpeak (player, message);
					npcSetConversationResponses (player, responses);
				}
				else
				{
					utils.removeScriptVar (player, "conversation.happer_mos_eisley.branchId");
					
					npcEndConversationWithMessage (player, message);
				}
				
				return SCRIPT_CONTINUE;
			}
			
		}
		
		return SCRIPT_DEFAULT;
	}
	
	
	public int happer_mos_eisley_handleBranch4(obj_id player, obj_id npc, string_id response) throws InterruptedException
	{
		
		if (response.equals("s_14"))
		{
			
			if (happer_mos_eisley_condition__defaultCondition (player, npc))
			{
				
				string_id message = new string_id (c_stringFile, "s_16");
				int numberOfResponses = 0;
				
				boolean hasResponse = false;
				
				boolean hasResponse0 = false;
				if (happer_mos_eisley_condition__defaultCondition (player, npc))
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
					
					utils.setScriptVar (player, "conversation.happer_mos_eisley.branchId", 5);
					
					npcSpeak (player, message);
					npcSetConversationResponses (player, responses);
				}
				else
				{
					utils.removeScriptVar (player, "conversation.happer_mos_eisley.branchId");
					
					npcEndConversationWithMessage (player, message);
				}
				
				return SCRIPT_CONTINUE;
			}
			
		}
		
		return SCRIPT_DEFAULT;
	}
	
	
	public int happer_mos_eisley_handleBranch5(obj_id player, obj_id npc, string_id response) throws InterruptedException
	{
		
		if (response.equals("s_18"))
		{
			
			if (happer_mos_eisley_condition__defaultCondition (player, npc))
			{
				
				string_id message = new string_id (c_stringFile, "s_20");
				int numberOfResponses = 0;
				
				boolean hasResponse = false;
				
				boolean hasResponse0 = false;
				if (happer_mos_eisley_condition__defaultCondition (player, npc))
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
					
					utils.setScriptVar (player, "conversation.happer_mos_eisley.branchId", 6);
					
					npcSpeak (player, message);
					npcSetConversationResponses (player, responses);
				}
				else
				{
					utils.removeScriptVar (player, "conversation.happer_mos_eisley.branchId");
					
					npcEndConversationWithMessage (player, message);
				}
				
				return SCRIPT_CONTINUE;
			}
			
		}
		
		return SCRIPT_DEFAULT;
	}
	
	
	public int happer_mos_eisley_handleBranch6(obj_id player, obj_id npc, string_id response) throws InterruptedException
	{
		
		if (response.equals("s_22"))
		{
			
			if (happer_mos_eisley_condition__defaultCondition (player, npc))
			{
				
				string_id message = new string_id (c_stringFile, "s_24");
				int numberOfResponses = 0;
				
				boolean hasResponse = false;
				
				boolean hasResponse0 = false;
				if (happer_mos_eisley_condition__defaultCondition (player, npc))
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
					
					utils.setScriptVar (player, "conversation.happer_mos_eisley.branchId", 7);
					
					npcSpeak (player, message);
					npcSetConversationResponses (player, responses);
				}
				else
				{
					utils.removeScriptVar (player, "conversation.happer_mos_eisley.branchId");
					
					npcEndConversationWithMessage (player, message);
				}
				
				return SCRIPT_CONTINUE;
			}
			
		}
		
		return SCRIPT_DEFAULT;
	}
	
	
	public int happer_mos_eisley_handleBranch7(obj_id player, obj_id npc, string_id response) throws InterruptedException
	{
		
		if (response.equals("s_26"))
		{
			
			if (happer_mos_eisley_condition__defaultCondition (player, npc))
			{
				happer_mos_eisley_action_sendSignalClueUpdate (player, npc);
				
				string_id message = new string_id (c_stringFile, "s_28");
				utils.removeScriptVar (player, "conversation.happer_mos_eisley.branchId");
				
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
			detachScript(self, "conversation.happer_mos_eisley");
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
		detachScript (self, "conversation.happer_mos_eisley");
		
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
		
		if (happer_mos_eisley_condition_hasGatheredTheClue (player, npc))
		{
			
			string_id message = new string_id (c_stringFile, "s_30");
			chat.chat (npc, player, message);
			
			return SCRIPT_CONTINUE;
		}
		
		if (happer_mos_eisley_condition_hasInvestigateQuest (player, npc))
		{
			happer_mos_eisley_action_sendSignalTalkHapper (player, npc);
			
			string_id message = new string_id (c_stringFile, "s_4");
			int numberOfResponses = 0;
			
			boolean hasResponse = false;
			
			boolean hasResponse0 = false;
			if (happer_mos_eisley_condition__defaultCondition (player, npc))
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
				
				utils.setScriptVar (player, "conversation.happer_mos_eisley.branchId", 2);
				
				npcStartConversation (player, npc, "happer_mos_eisley", message, responses);
			}
			else
			{
				chat.chat (npc, player, message);
			}
			
			return SCRIPT_CONTINUE;
		}
		
		if (happer_mos_eisley_condition__defaultCondition (player, npc))
		{
			
			string_id message = new string_id (c_stringFile, "s_32");
			chat.chat (npc, player, message);
			
			return SCRIPT_CONTINUE;
		}
		
		chat.chat (npc, "Error: All conditions for OnStartNpcConversation were false.");
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int OnNpcConversationResponse(obj_id self, String conversationId, obj_id player, string_id response) throws InterruptedException
	{
		if (!conversationId.equals("happer_mos_eisley"))
		{
			return SCRIPT_CONTINUE;
		}
		
		obj_id npc = self;
		
		int branchId = utils.getIntScriptVar (player, "conversation.happer_mos_eisley.branchId");
		
		if (branchId == 2 && happer_mos_eisley_handleBranch2 (player, npc, response) == SCRIPT_CONTINUE)
		{
			return SCRIPT_CONTINUE;
		}
		
		if (branchId == 3 && happer_mos_eisley_handleBranch3 (player, npc, response) == SCRIPT_CONTINUE)
		{
			return SCRIPT_CONTINUE;
		}
		
		if (branchId == 4 && happer_mos_eisley_handleBranch4 (player, npc, response) == SCRIPT_CONTINUE)
		{
			return SCRIPT_CONTINUE;
		}
		
		if (branchId == 5 && happer_mos_eisley_handleBranch5 (player, npc, response) == SCRIPT_CONTINUE)
		{
			return SCRIPT_CONTINUE;
		}
		
		if (branchId == 6 && happer_mos_eisley_handleBranch6 (player, npc, response) == SCRIPT_CONTINUE)
		{
			return SCRIPT_CONTINUE;
		}
		
		if (branchId == 7 && happer_mos_eisley_handleBranch7 (player, npc, response) == SCRIPT_CONTINUE)
		{
			return SCRIPT_CONTINUE;
		}
		
		chat.chat (npc, "Error: Fell through all branches and responses for OnNpcConversationResponse.");
		
		utils.removeScriptVar (player, "conversation.happer_mos_eisley.branchId");
		
		return SCRIPT_CONTINUE;
	}
	
}
