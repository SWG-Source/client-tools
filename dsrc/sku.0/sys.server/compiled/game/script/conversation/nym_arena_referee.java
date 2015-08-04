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


public class nym_arena_referee extends script.base_script
{
	public nym_arena_referee()
	{
	}
	String c_stringFile = "conversation/nym_arena_referee";
	
	
	public boolean nym_arena_referee_condition__defaultCondition(obj_id player, obj_id npc) throws InterruptedException
	{
		return true;
	}
	
	
	public boolean nym_arena_referee_condition_hasFirstArenaTask(obj_id player, obj_id npc) throws InterruptedException
	{
		faceTo(npc, player);
		return groundquests.isTaskActive(player, "u16_nym_themepark_nym_arena", "hasFirstArenaFight");
	}
	
	
	public boolean nym_arena_referee_condition_hasSecondArenaTask(obj_id player, obj_id npc) throws InterruptedException
	{
		faceTo(npc, player);
		return groundquests.isTaskActive(player, "u16_nym_themepark_nym_arena", "hasSecondArenaFight");
	}
	
	
	public boolean nym_arena_referee_condition_hasThirdArenaTask(obj_id player, obj_id npc) throws InterruptedException
	{
		faceTo(npc, player);
		return groundquests.isTaskActive(player, "u16_nym_themepark_nym_arena", "hasThirdArenaFight");
	}
	
	
	public boolean nym_arena_referee_condition_hasFourthArenaTask(obj_id player, obj_id npc) throws InterruptedException
	{
		faceTo(npc, player);
		return groundquests.isTaskActive(player, "u16_nym_themepark_nym_arena", "hasFourthArenaFight");
	}
	
	
	public boolean nym_arena_referee_condition_hasFifthAreanaTask(obj_id player, obj_id npc) throws InterruptedException
	{
		faceTo(npc, player);
		return groundquests.isTaskActive(player, "u16_nym_themepark_nym_arena", "hasFifthArenaFight");
	}
	
	
	public boolean nym_arena_referee_condition_hasCompletedArenaTask(obj_id player, obj_id npc) throws InterruptedException
	{
		faceTo(npc, player);
		return groundquests.isTaskActive(player, "u16_nym_themepark_nym_arena", "hasCompletedArenaForNym") || groundquests.hasCompletedQuest(player, "u16_nym_themepark_nym_arena");
	}
	
	
	public boolean nym_arena_referee_condition_wave_event_active(obj_id player, obj_id npc) throws InterruptedException
	{
		faceTo(npc, player);
		int wave = utils.getIntScriptVar(npc, "waveEventCurrentWave");
		return wave > 0;
	}
	
	
	public void nym_arena_referee_action_spawnOpponent(obj_id player, obj_id npc) throws InterruptedException
	{
		dictionary webster = new dictionary();
		webster.put("player", player);
		messageTo(npc, "waveEventControllerNPCStart", webster, 0, false);
	}
	
	
	public int nym_arena_referee_handleBranch3(obj_id player, obj_id npc, string_id response) throws InterruptedException
	{
		
		if (response.equals("s_34"))
		{
			
			if (nym_arena_referee_condition__defaultCondition (player, npc))
			{
				doAnimationAction (npc, "salute2");
				
				nym_arena_referee_action_spawnOpponent (player, npc);
				
				string_id message = new string_id (c_stringFile, "s_36");
				utils.removeScriptVar (player, "conversation.nym_arena_referee.branchId");
				
				npcEndConversationWithMessage (player, message);
				
				return SCRIPT_CONTINUE;
			}
			
		}
		
		return SCRIPT_DEFAULT;
	}
	
	
	public int nym_arena_referee_handleBranch5(obj_id player, obj_id npc, string_id response) throws InterruptedException
	{
		
		if (response.equals("s_32"))
		{
			
			if (nym_arena_referee_condition__defaultCondition (player, npc))
			{
				doAnimationAction (npc, "clap_rousing");
				
				nym_arena_referee_action_spawnOpponent (player, npc);
				
				string_id message = new string_id (c_stringFile, "s_33");
				utils.removeScriptVar (player, "conversation.nym_arena_referee.branchId");
				
				npcEndConversationWithMessage (player, message);
				
				return SCRIPT_CONTINUE;
			}
			
		}
		
		return SCRIPT_DEFAULT;
	}
	
	
	public int nym_arena_referee_handleBranch7(obj_id player, obj_id npc, string_id response) throws InterruptedException
	{
		
		if (response.equals("s_30"))
		{
			
			if (nym_arena_referee_condition__defaultCondition (player, npc))
			{
				doAnimationAction (npc, "bow3");
				
				nym_arena_referee_action_spawnOpponent (player, npc);
				
				string_id message = new string_id (c_stringFile, "s_31");
				utils.removeScriptVar (player, "conversation.nym_arena_referee.branchId");
				
				npcEndConversationWithMessage (player, message);
				
				return SCRIPT_CONTINUE;
			}
			
		}
		
		return SCRIPT_DEFAULT;
	}
	
	
	public int nym_arena_referee_handleBranch9(obj_id player, obj_id npc, string_id response) throws InterruptedException
	{
		
		if (response.equals("s_28"))
		{
			
			if (nym_arena_referee_condition__defaultCondition (player, npc))
			{
				doAnimationAction (npc, "applause_polite");
				
				nym_arena_referee_action_spawnOpponent (player, npc);
				
				string_id message = new string_id (c_stringFile, "s_29");
				utils.removeScriptVar (player, "conversation.nym_arena_referee.branchId");
				
				npcEndConversationWithMessage (player, message);
				
				return SCRIPT_CONTINUE;
			}
			
		}
		
		return SCRIPT_DEFAULT;
	}
	
	
	public int nym_arena_referee_handleBranch11(obj_id player, obj_id npc, string_id response) throws InterruptedException
	{
		
		if (response.equals("s_22"))
		{
			
			if (nym_arena_referee_condition__defaultCondition (player, npc))
			{
				doAnimationAction (npc, "celebrate1");
				
				nym_arena_referee_action_spawnOpponent (player, npc);
				
				string_id message = new string_id (c_stringFile, "s_23");
				utils.removeScriptVar (player, "conversation.nym_arena_referee.branchId");
				
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
			detachScript(self, "conversation.nym_arena_referee");
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
		detachScript (self, "conversation.nym_arena_referee");
		
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
		
		if (nym_arena_referee_condition_wave_event_active (player, npc))
		{
			
			string_id message = new string_id (c_stringFile, "s_20");
			chat.chat (npc, player, message);
			
			return SCRIPT_CONTINUE;
		}
		
		if (nym_arena_referee_condition_hasCompletedArenaTask (player, npc))
		{
			
			string_id message = new string_id (c_stringFile, "s_37");
			prose_package pp = new prose_package ();
			pp.stringId = message;
			pp.actor.set (player);
			pp.target.set (npc);
			
			chat.chat (npc, player, null, null, pp);
			return SCRIPT_CONTINUE;
		}
		
		if (nym_arena_referee_condition_hasFifthAreanaTask (player, npc))
		{
			
			string_id message = new string_id (c_stringFile, "s_24");
			int numberOfResponses = 0;
			
			boolean hasResponse = false;
			
			boolean hasResponse0 = false;
			if (nym_arena_referee_condition__defaultCondition (player, npc))
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
				
				utils.setScriptVar (player, "conversation.nym_arena_referee.branchId", 3);
				
				prose_package pp = new prose_package ();
				pp.stringId = message;
				pp.actor.set (player);
				pp.target.set (npc);
				
				npcStartConversation (player, npc, "nym_arena_referee", null, pp, responses);
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
		
		if (nym_arena_referee_condition_hasFourthArenaTask (player, npc))
		{
			
			string_id message = new string_id (c_stringFile, "s_25");
			int numberOfResponses = 0;
			
			boolean hasResponse = false;
			
			boolean hasResponse0 = false;
			if (nym_arena_referee_condition__defaultCondition (player, npc))
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
				
				utils.setScriptVar (player, "conversation.nym_arena_referee.branchId", 5);
				
				prose_package pp = new prose_package ();
				pp.stringId = message;
				pp.actor.set (player);
				pp.target.set (npc);
				
				npcStartConversation (player, npc, "nym_arena_referee", null, pp, responses);
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
		
		if (nym_arena_referee_condition_hasThirdArenaTask (player, npc))
		{
			
			string_id message = new string_id (c_stringFile, "s_26");
			int numberOfResponses = 0;
			
			boolean hasResponse = false;
			
			boolean hasResponse0 = false;
			if (nym_arena_referee_condition__defaultCondition (player, npc))
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
				
				utils.setScriptVar (player, "conversation.nym_arena_referee.branchId", 7);
				
				prose_package pp = new prose_package ();
				pp.stringId = message;
				pp.actor.set (player);
				pp.target.set (npc);
				
				npcStartConversation (player, npc, "nym_arena_referee", null, pp, responses);
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
		
		if (nym_arena_referee_condition_hasSecondArenaTask (player, npc))
		{
			
			string_id message = new string_id (c_stringFile, "s_27");
			int numberOfResponses = 0;
			
			boolean hasResponse = false;
			
			boolean hasResponse0 = false;
			if (nym_arena_referee_condition__defaultCondition (player, npc))
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
				
				utils.setScriptVar (player, "conversation.nym_arena_referee.branchId", 9);
				
				prose_package pp = new prose_package ();
				pp.stringId = message;
				pp.actor.set (player);
				pp.target.set (npc);
				
				npcStartConversation (player, npc, "nym_arena_referee", null, pp, responses);
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
		
		if (nym_arena_referee_condition_hasFirstArenaTask (player, npc))
		{
			
			string_id message = new string_id (c_stringFile, "s_21");
			int numberOfResponses = 0;
			
			boolean hasResponse = false;
			
			boolean hasResponse0 = false;
			if (nym_arena_referee_condition__defaultCondition (player, npc))
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
				
				utils.setScriptVar (player, "conversation.nym_arena_referee.branchId", 11);
				
				prose_package pp = new prose_package ();
				pp.stringId = message;
				pp.actor.set (player);
				pp.target.set (npc);
				
				npcStartConversation (player, npc, "nym_arena_referee", null, pp, responses);
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
		
		if (nym_arena_referee_condition__defaultCondition (player, npc))
		{
			
			string_id message = new string_id (c_stringFile, "s_100");
			prose_package pp = new prose_package ();
			pp.stringId = message;
			pp.actor.set (player);
			pp.target.set (npc);
			
			chat.chat (npc, player, null, null, pp);
			return SCRIPT_CONTINUE;
		}
		
		chat.chat (npc, "Error: All conditions for OnStartNpcConversation were false.");
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int OnNpcConversationResponse(obj_id self, String conversationId, obj_id player, string_id response) throws InterruptedException
	{
		if (!conversationId.equals("nym_arena_referee"))
		{
			return SCRIPT_CONTINUE;
		}
		
		obj_id npc = self;
		
		int branchId = utils.getIntScriptVar (player, "conversation.nym_arena_referee.branchId");
		
		if (branchId == 3 && nym_arena_referee_handleBranch3 (player, npc, response) == SCRIPT_CONTINUE)
		{
			return SCRIPT_CONTINUE;
		}
		
		if (branchId == 5 && nym_arena_referee_handleBranch5 (player, npc, response) == SCRIPT_CONTINUE)
		{
			return SCRIPT_CONTINUE;
		}
		
		if (branchId == 7 && nym_arena_referee_handleBranch7 (player, npc, response) == SCRIPT_CONTINUE)
		{
			return SCRIPT_CONTINUE;
		}
		
		if (branchId == 9 && nym_arena_referee_handleBranch9 (player, npc, response) == SCRIPT_CONTINUE)
		{
			return SCRIPT_CONTINUE;
		}
		
		if (branchId == 11 && nym_arena_referee_handleBranch11 (player, npc, response) == SCRIPT_CONTINUE)
		{
			return SCRIPT_CONTINUE;
		}
		
		chat.chat (npc, "Error: Fell through all branches and responses for OnNpcConversationResponse.");
		
		utils.removeScriptVar (player, "conversation.nym_arena_referee.branchId");
		
		return SCRIPT_CONTINUE;
	}
	
}
