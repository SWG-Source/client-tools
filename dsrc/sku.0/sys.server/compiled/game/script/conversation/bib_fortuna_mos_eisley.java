package script.conversation;

import script.*;
import script.base_class.*;
import script.combat_engine.*;
import java.util.Arrays;
import java.util.Hashtable;
import java.util.Vector;
import script.base_script;


import script.library.ai_lib;
import script.library.badge;
import script.library.buff;
import script.library.chat;
import script.library.collection;
import script.library.conversation;
import script.library.create;
import script.library.factions;
import script.library.groundquests;
import script.library.space_quest;
import script.library.static_item;
import script.library.utils;


public class bib_fortuna_mos_eisley extends script.base_script
{
	public bib_fortuna_mos_eisley()
	{
	}
	String c_stringFile = "conversation/bib_fortuna_mos_eisley";
	
	
	public boolean bib_fortuna_mos_eisley_condition__defaultCondition(obj_id player, obj_id npc) throws InterruptedException
	{
		return true;
	}
	
	
	public boolean bib_fortuna_mos_eisley_condition_hasComlinkQuest01Neut(obj_id player, obj_id npc) throws InterruptedException
	{
		faceTo(npc, player);
		return groundquests.isQuestActive(player, "quest_01_comlink_call_to_adventure_neut_faction");
	}
	
	
	public boolean bib_fortuna_mos_eisley_condition_hasComlinkQuest01Pos(obj_id player, obj_id npc) throws InterruptedException
	{
		faceTo(npc, player);
		return groundquests.isQuestActive(player, "quest_01_comlink_call_to_adventure_pos_faction");
	}
	
	
	public boolean bib_fortuna_mos_eisley_condition_hasComlinkQuest01Neg(obj_id player, obj_id npc) throws InterruptedException
	{
		faceTo(npc, player);
		return groundquests.isQuestActive(player, "quest_01_comlink_call_to_adventure_neg_faction");
	}
	
	
	public boolean bib_fortuna_mos_eisley_condition_hasTheBiolabClue(obj_id player, obj_id npc) throws InterruptedException
	{
		faceTo(npc, player);
		return ( (groundquests.isQuestActive(player, "quest_02_investigate_murder") && groundquests.isTaskActive(player, "quest_02_investigate_murder", "talkBibFortunaBiolabClue")) || (groundquests.hasCompletedQuest(player, "quest_02_investigate_murder") && !groundquests.isQuestActiveOrComplete(player, "quest_03_investigate_biolab")) );
	}
	
	
	public boolean bib_fortuna_mos_eisley_condition_hasBiolabQuest(obj_id player, obj_id npc) throws InterruptedException
	{
		faceTo(npc, player);
		return groundquests.hasCompletedQuest(player, "quest_02_investigate_murder") && groundquests.isQuestActive(player, "quest_03_investigate_biolab");
	}
	
	
	public boolean bib_fortuna_mos_eisley_condition_hasCompletedBiolabQuest(obj_id player, obj_id npc) throws InterruptedException
	{
		faceTo(npc, player);
		return groundquests.hasCompletedQuest(player, "quest_03_investigate_biolab") && groundquests.isQuestActive(player, "quest_04_meet_cale_herron");
	}
	
	
	public boolean bib_fortuna_mos_eisley_condition_needsToGatherClues(obj_id player, obj_id npc) throws InterruptedException
	{
		return groundquests.isTaskActive(player, "quest_02_investigate_murder", "speakHapper") || groundquests.isTaskActive(player, "quest_02_investigate_murder", "identifyBody") || groundquests.isTaskActive(player, "quest_02_investigate_murder", "getClues");
	}
	
	
	public boolean bib_fortuna_mos_eisley_condition_hasQuestOneNotQuestTwo(obj_id player, obj_id npc) throws InterruptedException
	{
		return ( !groundquests.isQuestActiveOrComplete(player, "quest_02_investigate_murder") && (groundquests.hasCompletedQuest(player, "quest_01_comlink_call_to_adventure_pos_faction") || groundquests.hasCompletedQuest(player, "quest_01_comlink_call_to_adventure_neg_faction") || groundquests.hasCompletedQuest(player, "quest_01_comlink_call_to_adventure_neut_faction")) );
	}
	
	
	public boolean bib_fortuna_mos_eisley_condition_hasCompletedBiolabButCancelHerron(obj_id player, obj_id npc) throws InterruptedException
	{
		return groundquests.hasCompletedQuest(player, "quest_03_investigate_biolab") && !groundquests.isQuestActiveOrComplete(player, "quest_04_meet_cale_herron");
	}
	
	
	public boolean bib_fortuna_mos_eisley_condition_hasCompletedInvestigationCancelBiolab(obj_id player, obj_id npc) throws InterruptedException
	{
		return groundquests.hasCompletedQuest(player, "quest_02_investigate_murder") && !groundquests.isQuestActiveOrComplete(player, "quest_03_investigate_biolab");
	}
	
	
	public void bib_fortuna_mos_eisley_action_comlinkQuestOnceSignal(obj_id player, obj_id npc) throws InterruptedException
	{
		groundquests.sendSignal(player, "bibWaitingForPlayer");
	}
	
	
	public void bib_fortuna_mos_eisley_action_grantQuest02Investigate(obj_id player, obj_id npc) throws InterruptedException
	{
		bib_fortuna_mos_eisley_action_comlinkQuestOnceSignal(player, npc);
		groundquests.grantQuest(player, "quest_02_investigate_murder");
	}
	
	
	public void bib_fortuna_mos_eisley_action_sendSignalAndGrantQuest03Biolab(obj_id player, obj_id npc) throws InterruptedException
	{
		groundquests.sendSignal(player, "quest02PlayerHasClues");
	}
	
	
	public void bib_fortuna_mos_eisley_action_grantQuest03(obj_id player, obj_id npc) throws InterruptedException
	{
		groundquests.grantQuest(player, "quest_03_investigate_biolab");
	}
	
	
	public void bib_fortuna_mos_eisley_action_clearQuest03(obj_id player, obj_id npc) throws InterruptedException
	{
		groundquests.clearQuest(player, "quest_03_investigate_biolab");
	}
	
	
	public void bib_fortuna_mos_eisley_action_clearThenGrantQuest03(obj_id player, obj_id npc) throws InterruptedException
	{
		bib_fortuna_mos_eisley_action_clearQuest03(player, npc);
		bib_fortuna_mos_eisley_action_grantQuest03(player, npc);
	}
	
	
	public int bib_fortuna_mos_eisley_handleBranch1(obj_id player, obj_id npc, string_id response) throws InterruptedException
	{
		
		if (response.equals("s_42"))
		{
			
			if (bib_fortuna_mos_eisley_condition__defaultCondition (player, npc))
			{
				bib_fortuna_mos_eisley_action_clearThenGrantQuest03 (player, npc);
				
				string_id message = new string_id (c_stringFile, "s_43");
				utils.removeScriptVar (player, "conversation.bib_fortuna_mos_eisley.branchId");
				
				npcEndConversationWithMessage (player, message);
				
				return SCRIPT_CONTINUE;
			}
			
		}
		
		return SCRIPT_DEFAULT;
	}
	
	
	public int bib_fortuna_mos_eisley_handleBranch6(obj_id player, obj_id npc, string_id response) throws InterruptedException
	{
		
		if (response.equals("s_23"))
		{
			
			if (bib_fortuna_mos_eisley_condition__defaultCondition (player, npc))
			{
				
				string_id message = new string_id (c_stringFile, "s_24");
				int numberOfResponses = 0;
				
				boolean hasResponse = false;
				
				boolean hasResponse0 = false;
				if (bib_fortuna_mos_eisley_condition__defaultCondition (player, npc))
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
					
					utils.setScriptVar (player, "conversation.bib_fortuna_mos_eisley.branchId", 7);
					
					npcSpeak (player, message);
					npcSetConversationResponses (player, responses);
				}
				else
				{
					utils.removeScriptVar (player, "conversation.bib_fortuna_mos_eisley.branchId");
					
					npcEndConversationWithMessage (player, message);
				}
				
				return SCRIPT_CONTINUE;
			}
			
		}
		
		return SCRIPT_DEFAULT;
	}
	
	
	public int bib_fortuna_mos_eisley_handleBranch7(obj_id player, obj_id npc, string_id response) throws InterruptedException
	{
		
		if (response.equals("s_25"))
		{
			
			if (bib_fortuna_mos_eisley_condition__defaultCondition (player, npc))
			{
				
				string_id message = new string_id (c_stringFile, "s_26");
				int numberOfResponses = 0;
				
				boolean hasResponse = false;
				
				boolean hasResponse0 = false;
				if (bib_fortuna_mos_eisley_condition__defaultCondition (player, npc))
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
					
					utils.setScriptVar (player, "conversation.bib_fortuna_mos_eisley.branchId", 8);
					
					npcSpeak (player, message);
					npcSetConversationResponses (player, responses);
				}
				else
				{
					utils.removeScriptVar (player, "conversation.bib_fortuna_mos_eisley.branchId");
					
					npcEndConversationWithMessage (player, message);
				}
				
				return SCRIPT_CONTINUE;
			}
			
		}
		
		return SCRIPT_DEFAULT;
	}
	
	
	public int bib_fortuna_mos_eisley_handleBranch8(obj_id player, obj_id npc, string_id response) throws InterruptedException
	{
		
		if (response.equals("s_27"))
		{
			
			if (bib_fortuna_mos_eisley_condition__defaultCondition (player, npc))
			{
				bib_fortuna_mos_eisley_action_sendSignalAndGrantQuest03Biolab (player, npc);
				
				string_id message = new string_id (c_stringFile, "s_28");
				utils.removeScriptVar (player, "conversation.bib_fortuna_mos_eisley.branchId");
				
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
	
	
	public int bib_fortuna_mos_eisley_handleBranch11(obj_id player, obj_id npc, string_id response) throws InterruptedException
	{
		
		if (response.equals("s_30"))
		{
			
			if (bib_fortuna_mos_eisley_condition__defaultCondition (player, npc))
			{
				
				string_id message = new string_id (c_stringFile, "s_32");
				int numberOfResponses = 0;
				
				boolean hasResponse = false;
				
				boolean hasResponse0 = false;
				if (bib_fortuna_mos_eisley_condition__defaultCondition (player, npc))
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
					
					utils.setScriptVar (player, "conversation.bib_fortuna_mos_eisley.branchId", 14);
					
					npcSpeak (player, message);
					npcSetConversationResponses (player, responses);
				}
				else
				{
					utils.removeScriptVar (player, "conversation.bib_fortuna_mos_eisley.branchId");
					
					npcEndConversationWithMessage (player, message);
				}
				
				return SCRIPT_CONTINUE;
			}
			
		}
		
		return SCRIPT_DEFAULT;
	}
	
	
	public int bib_fortuna_mos_eisley_handleBranch12(obj_id player, obj_id npc, string_id response) throws InterruptedException
	{
		
		if (response.equals("s_30"))
		{
			
			if (bib_fortuna_mos_eisley_condition__defaultCondition (player, npc))
			{
				
				string_id message = new string_id (c_stringFile, "s_32");
				int numberOfResponses = 0;
				
				boolean hasResponse = false;
				
				boolean hasResponse0 = false;
				if (bib_fortuna_mos_eisley_condition__defaultCondition (player, npc))
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
					
					utils.setScriptVar (player, "conversation.bib_fortuna_mos_eisley.branchId", 14);
					
					npcSpeak (player, message);
					npcSetConversationResponses (player, responses);
				}
				else
				{
					utils.removeScriptVar (player, "conversation.bib_fortuna_mos_eisley.branchId");
					
					npcEndConversationWithMessage (player, message);
				}
				
				return SCRIPT_CONTINUE;
			}
			
		}
		
		return SCRIPT_DEFAULT;
	}
	
	
	public int bib_fortuna_mos_eisley_handleBranch13(obj_id player, obj_id npc, string_id response) throws InterruptedException
	{
		
		if (response.equals("s_30"))
		{
			
			if (bib_fortuna_mos_eisley_condition__defaultCondition (player, npc))
			{
				
				string_id message = new string_id (c_stringFile, "s_32");
				int numberOfResponses = 0;
				
				boolean hasResponse = false;
				
				boolean hasResponse0 = false;
				if (bib_fortuna_mos_eisley_condition__defaultCondition (player, npc))
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
					
					utils.setScriptVar (player, "conversation.bib_fortuna_mos_eisley.branchId", 14);
					
					npcSpeak (player, message);
					npcSetConversationResponses (player, responses);
				}
				else
				{
					utils.removeScriptVar (player, "conversation.bib_fortuna_mos_eisley.branchId");
					
					npcEndConversationWithMessage (player, message);
				}
				
				return SCRIPT_CONTINUE;
			}
			
		}
		
		return SCRIPT_DEFAULT;
	}
	
	
	public int bib_fortuna_mos_eisley_handleBranch14(obj_id player, obj_id npc, string_id response) throws InterruptedException
	{
		
		if (response.equals("s_34"))
		{
			
			if (bib_fortuna_mos_eisley_condition__defaultCondition (player, npc))
			{
				
				string_id message = new string_id (c_stringFile, "s_36");
				int numberOfResponses = 0;
				
				boolean hasResponse = false;
				
				boolean hasResponse0 = false;
				if (bib_fortuna_mos_eisley_condition__defaultCondition (player, npc))
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
						responses[responseIndex++] = new string_id (c_stringFile, "s_39");
					}
					
					utils.setScriptVar (player, "conversation.bib_fortuna_mos_eisley.branchId", 15);
					
					npcSpeak (player, message);
					npcSetConversationResponses (player, responses);
				}
				else
				{
					utils.removeScriptVar (player, "conversation.bib_fortuna_mos_eisley.branchId");
					
					npcEndConversationWithMessage (player, message);
				}
				
				return SCRIPT_CONTINUE;
			}
			
		}
		
		return SCRIPT_DEFAULT;
	}
	
	
	public int bib_fortuna_mos_eisley_handleBranch15(obj_id player, obj_id npc, string_id response) throws InterruptedException
	{
		
		if (response.equals("s_39"))
		{
			
			if (bib_fortuna_mos_eisley_condition__defaultCondition (player, npc))
			{
				bib_fortuna_mos_eisley_action_grantQuest02Investigate (player, npc);
				
				string_id message = new string_id (c_stringFile, "s_44");
				utils.removeScriptVar (player, "conversation.bib_fortuna_mos_eisley.branchId");
				
				npcEndConversationWithMessage (player, message);
				
				return SCRIPT_CONTINUE;
			}
			
		}
		
		return SCRIPT_DEFAULT;
	}
	
	
	public int bib_fortuna_mos_eisley_handleBranch17(obj_id player, obj_id npc, string_id response) throws InterruptedException
	{
		
		if (response.equals("s_39"))
		{
			
			if (bib_fortuna_mos_eisley_condition__defaultCondition (player, npc))
			{
				bib_fortuna_mos_eisley_action_grantQuest02Investigate (player, npc);
				
				string_id message = new string_id (c_stringFile, "s_44");
				utils.removeScriptVar (player, "conversation.bib_fortuna_mos_eisley.branchId");
				
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
			detachScript(self, "conversation.bib_fortuna_mos_eisley");
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
		detachScript (self, "conversation.bib_fortuna_mos_eisley");
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int OnDetach(obj_id self) throws InterruptedException
	{
		clearCondition (self, CONDITION_CONVERSABLE);
		
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
		
		if (bib_fortuna_mos_eisley_condition_hasCompletedBiolabButCancelHerron (player, npc))
		{
			
			string_id message = new string_id (c_stringFile, "s_41");
			int numberOfResponses = 0;
			
			boolean hasResponse = false;
			
			boolean hasResponse0 = false;
			if (bib_fortuna_mos_eisley_condition__defaultCondition (player, npc))
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
					responses[responseIndex++] = new string_id (c_stringFile, "s_42");
				}
				
				utils.setScriptVar (player, "conversation.bib_fortuna_mos_eisley.branchId", 1);
				
				npcStartConversation (player, npc, "bib_fortuna_mos_eisley", message, responses);
			}
			else
			{
				chat.chat (npc, player, message);
			}
			
			return SCRIPT_CONTINUE;
		}
		
		if (bib_fortuna_mos_eisley_condition_hasCompletedBiolabQuest (player, npc))
		{
			
			string_id message = new string_id (c_stringFile, "s_21");
			chat.chat (npc, player, message);
			
			return SCRIPT_CONTINUE;
		}
		
		if (bib_fortuna_mos_eisley_condition_hasBiolabQuest (player, npc))
		{
			
			string_id message = new string_id (c_stringFile, "s_19");
			prose_package pp = new prose_package ();
			pp.stringId = message;
			pp.actor.set (player);
			pp.target.set (npc);
			
			chat.chat (npc, player, null, null, pp);
			return SCRIPT_CONTINUE;
		}
		
		if (bib_fortuna_mos_eisley_condition_hasCompletedInvestigationCancelBiolab (player, npc))
		{
			bib_fortuna_mos_eisley_action_grantQuest03 (player, npc);
			
			string_id message = new string_id (c_stringFile, "s_47");
			prose_package pp = new prose_package ();
			pp.stringId = message;
			pp.actor.set (player);
			pp.target.set (npc);
			
			chat.chat (npc, player, null, null, pp);
			return SCRIPT_CONTINUE;
		}
		
		if (bib_fortuna_mos_eisley_condition_hasTheBiolabClue (player, npc))
		{
			
			string_id message = new string_id (c_stringFile, "s_22");
			int numberOfResponses = 0;
			
			boolean hasResponse = false;
			
			boolean hasResponse0 = false;
			if (bib_fortuna_mos_eisley_condition__defaultCondition (player, npc))
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
				
				utils.setScriptVar (player, "conversation.bib_fortuna_mos_eisley.branchId", 6);
				
				npcStartConversation (player, npc, "bib_fortuna_mos_eisley", message, responses);
			}
			else
			{
				chat.chat (npc, player, message);
			}
			
			return SCRIPT_CONTINUE;
		}
		
		if (bib_fortuna_mos_eisley_condition_needsToGatherClues (player, npc))
		{
			
			string_id message = new string_id (c_stringFile, "s_37");
			chat.chat (npc, player, message);
			
			return SCRIPT_CONTINUE;
		}
		
		if (bib_fortuna_mos_eisley_condition_hasComlinkQuest01Neg (player, npc))
		{
			
			string_id message = new string_id (c_stringFile, "s_201");
			int numberOfResponses = 0;
			
			boolean hasResponse = false;
			
			boolean hasResponse0 = false;
			if (bib_fortuna_mos_eisley_condition__defaultCondition (player, npc))
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
					responses[responseIndex++] = new string_id (c_stringFile, "s_30");
				}
				
				utils.setScriptVar (player, "conversation.bib_fortuna_mos_eisley.branchId", 13);
				
				prose_package pp = new prose_package ();
				pp.stringId = message;
				pp.actor.set (player);
				pp.target.set (npc);
				
				npcStartConversation (player, npc, "bib_fortuna_mos_eisley", null, pp, responses);
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
		
		if (bib_fortuna_mos_eisley_condition_hasComlinkQuest01Neut (player, npc))
		{
			
			string_id message = new string_id (c_stringFile, "s_203");
			int numberOfResponses = 0;
			
			boolean hasResponse = false;
			
			boolean hasResponse0 = false;
			if (bib_fortuna_mos_eisley_condition__defaultCondition (player, npc))
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
					responses[responseIndex++] = new string_id (c_stringFile, "s_30");
				}
				
				utils.setScriptVar (player, "conversation.bib_fortuna_mos_eisley.branchId", 13);
				
				prose_package pp = new prose_package ();
				pp.stringId = message;
				pp.actor.set (player);
				pp.target.set (npc);
				
				npcStartConversation (player, npc, "bib_fortuna_mos_eisley", null, pp, responses);
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
		
		if (bib_fortuna_mos_eisley_condition_hasComlinkQuest01Pos (player, npc))
		{
			
			string_id message = new string_id (c_stringFile, "s_205");
			int numberOfResponses = 0;
			
			boolean hasResponse = false;
			
			boolean hasResponse0 = false;
			if (bib_fortuna_mos_eisley_condition__defaultCondition (player, npc))
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
					responses[responseIndex++] = new string_id (c_stringFile, "s_30");
				}
				
				utils.setScriptVar (player, "conversation.bib_fortuna_mos_eisley.branchId", 13);
				
				prose_package pp = new prose_package ();
				pp.stringId = message;
				pp.actor.set (player);
				pp.target.set (npc);
				
				npcStartConversation (player, npc, "bib_fortuna_mos_eisley", null, pp, responses);
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
		
		if (bib_fortuna_mos_eisley_condition_hasQuestOneNotQuestTwo (player, npc))
		{
			
			string_id message = new string_id (c_stringFile, "s_46");
			int numberOfResponses = 0;
			
			boolean hasResponse = false;
			
			boolean hasResponse0 = false;
			if (bib_fortuna_mos_eisley_condition__defaultCondition (player, npc))
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
					responses[responseIndex++] = new string_id (c_stringFile, "s_39");
				}
				
				utils.setScriptVar (player, "conversation.bib_fortuna_mos_eisley.branchId", 15);
				
				npcStartConversation (player, npc, "bib_fortuna_mos_eisley", message, responses);
			}
			else
			{
				chat.chat (npc, player, message);
			}
			
			return SCRIPT_CONTINUE;
		}
		
		if (bib_fortuna_mos_eisley_condition__defaultCondition (player, npc))
		{
			doAnimationAction (npc, "wave_on_dismissing");
			
			string_id message = new string_id (c_stringFile, "s_195");
			chat.chat (npc, player, message);
			
			return SCRIPT_CONTINUE;
		}
		
		chat.chat (npc, "Error: All conditions for OnStartNpcConversation were false.");
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int OnNpcConversationResponse(obj_id self, String conversationId, obj_id player, string_id response) throws InterruptedException
	{
		if (!conversationId.equals("bib_fortuna_mos_eisley"))
		{
			return SCRIPT_CONTINUE;
		}
		
		obj_id npc = self;
		
		int branchId = utils.getIntScriptVar (player, "conversation.bib_fortuna_mos_eisley.branchId");
		
		if (branchId == 1 && bib_fortuna_mos_eisley_handleBranch1 (player, npc, response) == SCRIPT_CONTINUE)
		{
			return SCRIPT_CONTINUE;
		}
		
		if (branchId == 6 && bib_fortuna_mos_eisley_handleBranch6 (player, npc, response) == SCRIPT_CONTINUE)
		{
			return SCRIPT_CONTINUE;
		}
		
		if (branchId == 7 && bib_fortuna_mos_eisley_handleBranch7 (player, npc, response) == SCRIPT_CONTINUE)
		{
			return SCRIPT_CONTINUE;
		}
		
		if (branchId == 8 && bib_fortuna_mos_eisley_handleBranch8 (player, npc, response) == SCRIPT_CONTINUE)
		{
			return SCRIPT_CONTINUE;
		}
		
		if (branchId == 11 && bib_fortuna_mos_eisley_handleBranch11 (player, npc, response) == SCRIPT_CONTINUE)
		{
			return SCRIPT_CONTINUE;
		}
		
		if (branchId == 12 && bib_fortuna_mos_eisley_handleBranch12 (player, npc, response) == SCRIPT_CONTINUE)
		{
			return SCRIPT_CONTINUE;
		}
		
		if (branchId == 13 && bib_fortuna_mos_eisley_handleBranch13 (player, npc, response) == SCRIPT_CONTINUE)
		{
			return SCRIPT_CONTINUE;
		}
		
		if (branchId == 14 && bib_fortuna_mos_eisley_handleBranch14 (player, npc, response) == SCRIPT_CONTINUE)
		{
			return SCRIPT_CONTINUE;
		}
		
		if (branchId == 15 && bib_fortuna_mos_eisley_handleBranch15 (player, npc, response) == SCRIPT_CONTINUE)
		{
			return SCRIPT_CONTINUE;
		}
		
		if (branchId == 17 && bib_fortuna_mos_eisley_handleBranch17 (player, npc, response) == SCRIPT_CONTINUE)
		{
			return SCRIPT_CONTINUE;
		}
		
		chat.chat (npc, "Error: Fell through all branches and responses for OnNpcConversationResponse.");
		
		utils.removeScriptVar (player, "conversation.bib_fortuna_mos_eisley.branchId");
		
		return SCRIPT_CONTINUE;
	}
	
}
