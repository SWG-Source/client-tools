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
import script.library.create;
import script.library.features;
import script.library.groundquests;
import script.library.space_quest;
import script.library.utils;


public class boss_terminal_research_barnet extends script.base_script
{
	public boss_terminal_research_barnet()
	{
	}
	String c_stringFile = "conversation/boss_terminal_research_barnet";
	
	
	public boolean boss_terminal_research_barnet_condition__defaultCondition(obj_id player, obj_id npc) throws InterruptedException
	{
		return true;
	}
	
	
	public boolean boss_terminal_research_barnet_condition_hasQuest(obj_id player, obj_id npc) throws InterruptedException
	{
		return groundquests.isQuestActive(player, "outbreak_undead_rancor_boss_fight");
	}
	
	
	public boolean boss_terminal_research_barnet_condition_hasGroup(obj_id player, obj_id npc) throws InterruptedException
	{
		obj_id groupid = getGroupObject(player);
		if (!isValidId(groupid))
		{
			
			return false;
		}
		return true;
	}
	
	
	public boolean boss_terminal_research_barnet_condition_hasBossTask(obj_id player, obj_id npc) throws InterruptedException
	{
		return (groundquests.isTaskActive(player, "u16_nym_themepark_lab_boss", "findUdoBarnet") || groundquests.isTaskActive(player, "u16_nym_themepark_lab_boss", "killUdoBarnet")) && hasCompletedCollectionSlot(player, "kill_nyms_themepark_boss_lab_activate");
	}
	
	
	public boolean boss_terminal_research_barnet_condition_hasQuestAndGroup(obj_id player, obj_id npc) throws InterruptedException
	{
		if (!boss_terminal_research_barnet_condition_hasGroup(player,npc))
		{
			return false;
		}
		if (!boss_terminal_research_barnet_condition_hasQuest(player,npc))
		{
			return false;
		}
		return groundquests.isTaskActive(player, "outbreak_undead_rancor_boss_fight", "defeatUndeadRancor");
	}
	
	
	public boolean boss_terminal_research_barnet_condition_wave_event_active(obj_id player, obj_id npc) throws InterruptedException
	{
		int wave = utils.getIntScriptVar(npc, "waveEventCurrentWave");
		return wave > 0;
	}
	
	
	public void boss_terminal_research_barnet_action_startEvent(obj_id player, obj_id npc) throws InterruptedException
	{
		boss_terminal_research_barnet_action_foundTerminal(player,npc);
		
		dictionary dict = new dictionary();
		dict.put("player", player);
		messageTo(npc, "waveEventControllerNPCStart", dict, 4, false);
	}
	
	
	public void boss_terminal_research_barnet_action_foundTerminal(obj_id player, obj_id npc) throws InterruptedException
	{
		groundquests.sendSignal(player, "hasFoundUdoBarnet");
		obj_id myGroup = getGroupObject(player);
		if (isValidId(myGroup))
		{
			obj_id[] members = getGroupMemberIds(myGroup);
			
			for (int i = 0; i < members.length; i++)
			{
				testAbortScript();
				if (!isIdValid(members[i]) || !exists(members[i]))
				{
					continue;
				}
				
				if (getDistance(members[i], npc) > 200)
				{
					continue;
				}
				
				if (boss_terminal_research_barnet_condition_hasBossTask(members[i], npc))
				{
					groundquests.sendSignal(members[i], "hasFoundUdoBarnet");
				}
			}
		}
	}
	
	
	public void boss_terminal_research_barnet_action_unauthorizedUse(obj_id player, obj_id npc) throws InterruptedException
	{
		string_id barkString = new string_id("theme_park_nym/messages", "terminal_unauthorized");
		chat.chat(npc, barkString);
	}
	
	
	public void boss_terminal_research_barnet_action_busyWithOtherPlayer(obj_id player, obj_id npc) throws InterruptedException
	{
		string_id barkString = new string_id("theme_park_nym/messages", "terminal_busy");
		chat.chat(npc, barkString);
	}
	
	
	public int boss_terminal_research_barnet_handleBranch1(obj_id player, obj_id npc, string_id response) throws InterruptedException
	{
		
		if (response.equals("s_11"))
		{
			
			if (boss_terminal_research_barnet_condition__defaultCondition (player, npc))
			{
				
				string_id message = new string_id (c_stringFile, "s_12");
				utils.removeScriptVar (player, "conversation.boss_terminal_research_barnet.branchId");
				
				npcEndConversationWithMessage (player, message);
				
				return SCRIPT_CONTINUE;
			}
			
		}
		
		return SCRIPT_DEFAULT;
	}
	
	
	public int boss_terminal_research_barnet_handleBranch3(obj_id player, obj_id npc, string_id response) throws InterruptedException
	{
		
		if (response.equals("s_36"))
		{
			
			if (boss_terminal_research_barnet_condition__defaultCondition (player, npc))
			{
				boss_terminal_research_barnet_action_startEvent (player, npc);
				
				string_id message = new string_id (c_stringFile, "s_37");
				utils.removeScriptVar (player, "conversation.boss_terminal_research_barnet.branchId");
				
				npcEndConversationWithMessage (player, message);
				
				return SCRIPT_CONTINUE;
			}
			
		}
		
		return SCRIPT_DEFAULT;
	}
	
	
	public int boss_terminal_research_barnet_handleBranch5(obj_id player, obj_id npc, string_id response) throws InterruptedException
	{
		
		if (response.equals("s_14"))
		{
			
			if (boss_terminal_research_barnet_condition__defaultCondition (player, npc))
			{
				
				string_id message = new string_id (c_stringFile, "s_16");
				utils.removeScriptVar (player, "conversation.boss_terminal_research_barnet.branchId");
				
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
	
	
	public int spawnEnemies(obj_id self, dictionary params) throws InterruptedException
	{
		if (params == null)
		{
			return SCRIPT_CONTINUE;
		}
		
		messageTo(self, "waveEventControllerNPCStart", params, 0, false);
		return SCRIPT_CONTINUE;
	}
	
	
	public int handleQuestFlavorObjectCleanup(obj_id self, dictionary params) throws InterruptedException
	{
		if (!utils.hasScriptVar(self, "handleQuestFlavorObjectCleanup"))
		{
			return SCRIPT_CONTINUE;
		}
		
		obj_id objList[] = utils.getObjIdArrayScriptVar(self, "handleQuestFlavorObjectCleanup");
		if (objList == null || objList.length <= 0)
		{
			return SCRIPT_CONTINUE;
		}
		
		for (int i = 0; 1 < objList.length; i++)
		{
			testAbortScript();
			if (!isValidId(objList[i]))
			{
				continue;
			}
			
			messageTo(objList[i], "destroySelf", null, 0, false);
		}
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
		
		if (boss_terminal_research_barnet_condition_wave_event_active (player, npc))
		{
			
			string_id message = new string_id (c_stringFile, "s_7");
			int numberOfResponses = 0;
			
			boolean hasResponse = false;
			
			boolean hasResponse0 = false;
			if (boss_terminal_research_barnet_condition__defaultCondition (player, npc))
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
				
				utils.setScriptVar (player, "conversation.boss_terminal_research_barnet.branchId", 1);
				
				npcStartConversation (player, npc, "boss_terminal_research_barnet", message, responses);
			}
			else
			{
				chat.chat (npc, player, message);
			}
			
			return SCRIPT_CONTINUE;
		}
		
		if (boss_terminal_research_barnet_condition_hasBossTask (player, npc))
		{
			
			string_id message = new string_id (c_stringFile, "s_35");
			int numberOfResponses = 0;
			
			boolean hasResponse = false;
			
			boolean hasResponse0 = false;
			if (boss_terminal_research_barnet_condition__defaultCondition (player, npc))
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
				
				utils.setScriptVar (player, "conversation.boss_terminal_research_barnet.branchId", 3);
				
				npcStartConversation (player, npc, "boss_terminal_research_barnet", message, responses);
			}
			else
			{
				chat.chat (npc, player, message);
			}
			
			return SCRIPT_CONTINUE;
		}
		
		if (boss_terminal_research_barnet_condition__defaultCondition (player, npc))
		{
			boss_terminal_research_barnet_action_unauthorizedUse (player, npc);
			
			string_id message = new string_id (c_stringFile, "s_34");
			int numberOfResponses = 0;
			
			boolean hasResponse = false;
			
			boolean hasResponse0 = false;
			if (boss_terminal_research_barnet_condition__defaultCondition (player, npc))
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
				
				utils.setScriptVar (player, "conversation.boss_terminal_research_barnet.branchId", 5);
				
				npcStartConversation (player, npc, "boss_terminal_research_barnet", message, responses);
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
		if (!conversationId.equals("boss_terminal_research_barnet"))
		{
			return SCRIPT_CONTINUE;
		}
		
		obj_id npc = self;
		
		int branchId = utils.getIntScriptVar (player, "conversation.boss_terminal_research_barnet.branchId");
		
		if (branchId == 1 && boss_terminal_research_barnet_handleBranch1 (player, npc, response) == SCRIPT_CONTINUE)
		{
			return SCRIPT_CONTINUE;
		}
		
		if (branchId == 3 && boss_terminal_research_barnet_handleBranch3 (player, npc, response) == SCRIPT_CONTINUE)
		{
			return SCRIPT_CONTINUE;
		}
		
		if (branchId == 5 && boss_terminal_research_barnet_handleBranch5 (player, npc, response) == SCRIPT_CONTINUE)
		{
			return SCRIPT_CONTINUE;
		}
		
		chat.chat (npc, "Error: Fell through all branches and responses for OnNpcConversationResponse.");
		
		utils.removeScriptVar (player, "conversation.boss_terminal_research_barnet.branchId");
		
		return SCRIPT_CONTINUE;
	}
	
}
