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


public class inquisitor_admin extends script.base_script
{
	public inquisitor_admin()
	{
	}
	String c_stringFile = "conversation/inquisitor_admin";
	
	
	public boolean inquisitor_admin_condition__defaultCondition(obj_id player, obj_id npc) throws InterruptedException
	{
		return true;
	}
	
	
	public boolean inquisitor_admin_condition_isNeutQuest(obj_id player, obj_id npc) throws InterruptedException
	{
		faceTo(npc, player);
		return groundquests.hasCompletedQuest(player, "outbreak_quest_02_neutral");
	}
	
	
	public boolean inquisitor_admin_condition_isRebQuest(obj_id player, obj_id npc) throws InterruptedException
	{
		faceTo(npc, player);
		return groundquests.hasCompletedQuest(player, "outbreak_quest_02_rebel");
	}
	
	
	public boolean inquisitor_admin_condition_hasQuestForInq(obj_id player, obj_id npc) throws InterruptedException
	{
		faceTo(npc, player);
		return (groundquests.hasCompletedQuest(player, "outbreak_quest_02_imperial") || groundquests.hasCompletedQuest(player, "outbreak_quest_02_neutral") || groundquests.hasCompletedQuest(player, "outbreak_quest_02_rebel"));
	}
	
	
	public boolean inquisitor_admin_condition_hasPreviousQuests(obj_id player, obj_id npc) throws InterruptedException
	{
		return (inquisitor_admin_condition_isImpQuest(player, npc) || inquisitor_admin_condition_isRebQuest(player, npc) || inquisitor_admin_condition_isNeutQuest(player, npc) && inquisitor_admin_condition_hasARandomInquisitorQuest(player, npc));
	}
	
	
	public boolean inquisitor_admin_condition_isPlayerGodMode(obj_id player, obj_id npc) throws InterruptedException
	{
		return isGod(player);
	}
	
	
	public boolean inquisitor_admin_condition_hasARandomInquisitorQuest(obj_id player, obj_id npc) throws InterruptedException
	{
		String faction = "";
		if (inquisitor_admin_condition_isImpQuest(player,npc))
		{
			faction = "imperial";
		}
		else if (inquisitor_admin_condition_isRebQuest(player,npc))
		{
			faction = "rebel";
		}
		else
		{
			faction = "neutral";
		}
		
		if (faction == null || faction.length() <= 0)
		{
			return false;
		}
		
		boolean hasAValidQuest = false;
		for (int i = 1; i <= 5; i++)
		{
			testAbortScript();
			if (!groundquests.isQuestActive(player, "outbreak_quest_administrative_building_" + faction + "_0" + i))
			{
				continue;
			}
			
			hasAValidQuest = true;
			break;
		}
		
		return hasAValidQuest;
	}
	
	
	public boolean inquisitor_admin_condition_hasKey(obj_id player, obj_id npc) throws InterruptedException
	{
		String faction = "";
		if (inquisitor_admin_condition_isImpQuest(player,npc))
		{
			faction = "imperial";
		}
		else if (inquisitor_admin_condition_isRebQuest(player,npc))
		{
			faction = "rebel";
		}
		else
		{
			faction = "neutral";
		}
		
		if (faction == null || faction.length() <= 0)
		{
			return false;
		}
		
		boolean hasAValidTask = false;
		for (int i = 1; i <= 5; i++)
		{
			testAbortScript();
			if (!groundquests.isTaskActive(player, "outbreak_quest_administrative_building_" + faction + "_0" + i, "returnInquisitor"))
			{
				continue;
			}
			
			hasAValidTask = true;
			break;
		}
		
		return hasAValidTask;
	}
	
	
	public boolean inquisitor_admin_condition_isNeutralOrRebelQuest(obj_id player, obj_id npc) throws InterruptedException
	{
		return inquisitor_admin_condition_isRebQuest(player, npc) || inquisitor_admin_condition_isNeutQuest(player, npc);
	}
	
	
	public boolean inquisitor_admin_condition_hasCompletedKeyDeletedDungeon(obj_id player, obj_id npc) throws InterruptedException
	{
		String faction = "";
		if (inquisitor_admin_condition_isImpQuest(player,npc))
		{
			faction = "imperial";
		}
		else if (inquisitor_admin_condition_isRebQuest(player,npc))
		{
			faction = "rebel";
		}
		else
		{
			faction = "neutral";
		}
		
		if (faction == null || faction.length() <= 0)
		{
			return false;
		}
		
		String theKeyQuest = "";
		for (int i = 1; i <= 5; i++)
		{
			testAbortScript();
			if (!groundquests.hasCompletedQuest(player, "outbreak_quest_administrative_building_" + faction + "_0" + i))
			{
				continue;
			}
			
			theKeyQuest = "outbreak_quest_administrative_building_"+ faction + "_0"+ i;
			break;
		}
		
		if (theKeyQuest == null || theKeyQuest.length() <= 0)
		{
			return false;
		}
		
		return groundquests.hasCompletedQuest(player, theKeyQuest);
		
	}
	
	
	public boolean inquisitor_admin_condition_isImpQuest(obj_id player, obj_id npc) throws InterruptedException
	{
		faceTo(npc, player);
		return groundquests.hasCompletedQuest(player, "outbreak_quest_02_imperial");
	}
	
	
	public boolean inquisitor_admin_condition_hasFindKeyTaskActive(obj_id player, obj_id npc) throws InterruptedException
	{
		String faction = "";
		if (inquisitor_admin_condition_isImpQuest(player,npc))
		{
			faction = "imperial";
		}
		else if (inquisitor_admin_condition_isRebQuest(player,npc))
		{
			faction = "rebel";
		}
		else
		{
			faction = "neutral";
		}
		
		if (faction == null || faction.length() <= 0)
		{
			return false;
		}
		
		boolean hasAValidQuest = false;
		for (int i = 1; i <= 5; i++)
		{
			testAbortScript();
			if (!groundquests.isTaskActive(player, "outbreak_quest_administrative_building_" + faction + "_0" + i, "findKey"))
			{
				continue;
			}
			
			hasAValidQuest = true;
			break;
		}
		
		return hasAValidQuest;
	}
	
	
	public boolean inquisitor_admin_condition_hasRadio(obj_id player, obj_id npc) throws InterruptedException
	{
		return groundquests.isTaskActive(player, "outbreak_radio_delivery_03", "findEpsilonContact");
	}
	
	
	public void inquisitor_admin_action_sendSignalGoAdminBuilding(obj_id player, obj_id npc) throws InterruptedException
	{
		faceTo(npc, player);
		groundquests.sendSignal(player, "hasSpokenInquisitor");
	}
	
	
	public void inquisitor_admin_action_grantDungeonQuest(obj_id player, obj_id npc) throws InterruptedException
	{
		
		groundquests.grantQuest(player, "outbreak_quest_facility_01");
	}
	
	
	public void inquisitor_admin_action_sendSignalReturnedInquisitor(obj_id player, obj_id npc) throws InterruptedException
	{
		groundquests.sendSignal(player, "hasReturnedInquisitor");
		inquisitor_admin_action_grantDungeonQuest(player, npc);
	}
	
	
	public int inquisitor_admin_handleBranch2(obj_id player, obj_id npc, string_id response) throws InterruptedException
	{
		
		if (response.equals("s_38"))
		{
			
			if (inquisitor_admin_condition__defaultCondition (player, npc))
			{
				
				string_id message = new string_id (c_stringFile, "s_39");
				int numberOfResponses = 0;
				
				boolean hasResponse = false;
				
				boolean hasResponse0 = false;
				if (inquisitor_admin_condition__defaultCondition (player, npc))
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
					
					utils.setScriptVar (player, "conversation.inquisitor_admin.branchId", 3);
					
					npcSpeak (player, message);
					npcSetConversationResponses (player, responses);
				}
				else
				{
					utils.removeScriptVar (player, "conversation.inquisitor_admin.branchId");
					
					npcEndConversationWithMessage (player, message);
				}
				
				return SCRIPT_CONTINUE;
			}
			
		}
		
		return SCRIPT_DEFAULT;
	}
	
	
	public int inquisitor_admin_handleBranch3(obj_id player, obj_id npc, string_id response) throws InterruptedException
	{
		
		if (response.equals("s_40"))
		{
			
			if (inquisitor_admin_condition__defaultCondition (player, npc))
			{
				
				string_id message = new string_id (c_stringFile, "s_41");
				int numberOfResponses = 0;
				
				boolean hasResponse = false;
				
				boolean hasResponse0 = false;
				if (inquisitor_admin_condition__defaultCondition (player, npc))
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
					
					utils.setScriptVar (player, "conversation.inquisitor_admin.branchId", 4);
					
					npcSpeak (player, message);
					npcSetConversationResponses (player, responses);
				}
				else
				{
					utils.removeScriptVar (player, "conversation.inquisitor_admin.branchId");
					
					npcEndConversationWithMessage (player, message);
				}
				
				return SCRIPT_CONTINUE;
			}
			
		}
		
		return SCRIPT_DEFAULT;
	}
	
	
	public int inquisitor_admin_handleBranch4(obj_id player, obj_id npc, string_id response) throws InterruptedException
	{
		
		if (response.equals("s_42"))
		{
			
			if (inquisitor_admin_condition__defaultCondition (player, npc))
			{
				
				string_id message = new string_id (c_stringFile, "s_43");
				int numberOfResponses = 0;
				
				boolean hasResponse = false;
				
				boolean hasResponse0 = false;
				if (inquisitor_admin_condition__defaultCondition (player, npc))
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
						responses[responseIndex++] = new string_id (c_stringFile, "s_52");
					}
					
					utils.setScriptVar (player, "conversation.inquisitor_admin.branchId", 5);
					
					npcSpeak (player, message);
					npcSetConversationResponses (player, responses);
				}
				else
				{
					utils.removeScriptVar (player, "conversation.inquisitor_admin.branchId");
					
					npcEndConversationWithMessage (player, message);
				}
				
				return SCRIPT_CONTINUE;
			}
			
		}
		
		return SCRIPT_DEFAULT;
	}
	
	
	public int inquisitor_admin_handleBranch5(obj_id player, obj_id npc, string_id response) throws InterruptedException
	{
		
		if (response.equals("s_52"))
		{
			
			if (inquisitor_admin_condition__defaultCondition (player, npc))
			{
				
				string_id message = new string_id (c_stringFile, "s_54");
				int numberOfResponses = 0;
				
				boolean hasResponse = false;
				
				boolean hasResponse0 = false;
				if (inquisitor_admin_condition__defaultCondition (player, npc))
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
						responses[responseIndex++] = new string_id (c_stringFile, "s_56");
					}
					
					utils.setScriptVar (player, "conversation.inquisitor_admin.branchId", 6);
					
					npcSpeak (player, message);
					npcSetConversationResponses (player, responses);
				}
				else
				{
					utils.removeScriptVar (player, "conversation.inquisitor_admin.branchId");
					
					npcEndConversationWithMessage (player, message);
				}
				
				return SCRIPT_CONTINUE;
			}
			
		}
		
		return SCRIPT_DEFAULT;
	}
	
	
	public int inquisitor_admin_handleBranch6(obj_id player, obj_id npc, string_id response) throws InterruptedException
	{
		
		if (response.equals("s_56"))
		{
			
			if (inquisitor_admin_condition__defaultCondition (player, npc))
			{
				
				string_id message = new string_id (c_stringFile, "s_58");
				int numberOfResponses = 0;
				
				boolean hasResponse = false;
				
				boolean hasResponse0 = false;
				if (inquisitor_admin_condition__defaultCondition (player, npc))
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
						responses[responseIndex++] = new string_id (c_stringFile, "s_59");
					}
					
					utils.setScriptVar (player, "conversation.inquisitor_admin.branchId", 7);
					
					npcSpeak (player, message);
					npcSetConversationResponses (player, responses);
				}
				else
				{
					utils.removeScriptVar (player, "conversation.inquisitor_admin.branchId");
					
					npcEndConversationWithMessage (player, message);
				}
				
				return SCRIPT_CONTINUE;
			}
			
		}
		
		return SCRIPT_DEFAULT;
	}
	
	
	public int inquisitor_admin_handleBranch7(obj_id player, obj_id npc, string_id response) throws InterruptedException
	{
		
		if (response.equals("s_59"))
		{
			
			if (inquisitor_admin_condition__defaultCondition (player, npc))
			{
				inquisitor_admin_action_sendSignalReturnedInquisitor (player, npc);
				
				string_id message = new string_id (c_stringFile, "s_61");
				utils.removeScriptVar (player, "conversation.inquisitor_admin.branchId");
				
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
	
	
	public int inquisitor_admin_handleBranch10(obj_id player, obj_id npc, string_id response) throws InterruptedException
	{
		
		if (response.equals("s_20"))
		{
			
			if (inquisitor_admin_condition__defaultCondition (player, npc))
			{
				
				string_id message = new string_id (c_stringFile, "s_22");
				int numberOfResponses = 0;
				
				boolean hasResponse = false;
				
				boolean hasResponse0 = false;
				if (inquisitor_admin_condition__defaultCondition (player, npc))
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
					
					utils.setScriptVar (player, "conversation.inquisitor_admin.branchId", 11);
					
					npcSpeak (player, message);
					npcSetConversationResponses (player, responses);
				}
				else
				{
					utils.removeScriptVar (player, "conversation.inquisitor_admin.branchId");
					
					npcEndConversationWithMessage (player, message);
				}
				
				return SCRIPT_CONTINUE;
			}
			
		}
		
		return SCRIPT_DEFAULT;
	}
	
	
	public int inquisitor_admin_handleBranch11(obj_id player, obj_id npc, string_id response) throws InterruptedException
	{
		
		if (response.equals("s_24"))
		{
			doAnimationAction (player, "handshake_tandem");
			
			if (inquisitor_admin_condition__defaultCondition (player, npc))
			{
				doAnimationAction (npc, "nod_head_once");
				
				string_id message = new string_id (c_stringFile, "s_26");
				int numberOfResponses = 0;
				
				boolean hasResponse = false;
				
				boolean hasResponse0 = false;
				if (inquisitor_admin_condition__defaultCondition (player, npc))
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
					
					utils.setScriptVar (player, "conversation.inquisitor_admin.branchId", 12);
					
					npcSpeak (player, message);
					npcSetConversationResponses (player, responses);
				}
				else
				{
					utils.removeScriptVar (player, "conversation.inquisitor_admin.branchId");
					
					npcEndConversationWithMessage (player, message);
				}
				
				return SCRIPT_CONTINUE;
			}
			
		}
		
		return SCRIPT_DEFAULT;
	}
	
	
	public int inquisitor_admin_handleBranch12(obj_id player, obj_id npc, string_id response) throws InterruptedException
	{
		
		if (response.equals("s_28"))
		{
			
			if (inquisitor_admin_condition__defaultCondition (player, npc))
			{
				doAnimationAction (npc, "shakefist");
				
				string_id message = new string_id (c_stringFile, "s_30");
				int numberOfResponses = 0;
				
				boolean hasResponse = false;
				
				boolean hasResponse0 = false;
				if (inquisitor_admin_condition__defaultCondition (player, npc))
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
					
					utils.setScriptVar (player, "conversation.inquisitor_admin.branchId", 13);
					
					npcSpeak (player, message);
					npcSetConversationResponses (player, responses);
				}
				else
				{
					utils.removeScriptVar (player, "conversation.inquisitor_admin.branchId");
					
					npcEndConversationWithMessage (player, message);
				}
				
				return SCRIPT_CONTINUE;
			}
			
		}
		
		return SCRIPT_DEFAULT;
	}
	
	
	public int inquisitor_admin_handleBranch13(obj_id player, obj_id npc, string_id response) throws InterruptedException
	{
		
		if (response.equals("s_32"))
		{
			
			if (inquisitor_admin_condition__defaultCondition (player, npc))
			{
				
				string_id message = new string_id (c_stringFile, "s_34");
				int numberOfResponses = 0;
				
				boolean hasResponse = false;
				
				boolean hasResponse0 = false;
				if (inquisitor_admin_condition__defaultCondition (player, npc))
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
					
					utils.setScriptVar (player, "conversation.inquisitor_admin.branchId", 14);
					
					npcSpeak (player, message);
					npcSetConversationResponses (player, responses);
				}
				else
				{
					utils.removeScriptVar (player, "conversation.inquisitor_admin.branchId");
					
					npcEndConversationWithMessage (player, message);
				}
				
				return SCRIPT_CONTINUE;
			}
			
		}
		
		return SCRIPT_DEFAULT;
	}
	
	
	public int inquisitor_admin_handleBranch14(obj_id player, obj_id npc, string_id response) throws InterruptedException
	{
		
		if (response.equals("s_36"))
		{
			
			if (inquisitor_admin_condition__defaultCondition (player, npc))
			{
				
				string_id message = new string_id (c_stringFile, "s_45");
				int numberOfResponses = 0;
				
				boolean hasResponse = false;
				
				boolean hasResponse0 = false;
				if (inquisitor_admin_condition__defaultCondition (player, npc))
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
						responses[responseIndex++] = new string_id (c_stringFile, "s_47");
					}
					
					utils.setScriptVar (player, "conversation.inquisitor_admin.branchId", 15);
					
					npcSpeak (player, message);
					npcSetConversationResponses (player, responses);
				}
				else
				{
					utils.removeScriptVar (player, "conversation.inquisitor_admin.branchId");
					
					npcEndConversationWithMessage (player, message);
				}
				
				return SCRIPT_CONTINUE;
			}
			
		}
		
		return SCRIPT_DEFAULT;
	}
	
	
	public int inquisitor_admin_handleBranch15(obj_id player, obj_id npc, string_id response) throws InterruptedException
	{
		
		if (response.equals("s_47"))
		{
			
			if (inquisitor_admin_condition__defaultCondition (player, npc))
			{
				doAnimationAction (npc, "shrug_shoulders");
				
				inquisitor_admin_action_sendSignalGoAdminBuilding (player, npc);
				
				string_id message = new string_id (c_stringFile, "s_49");
				utils.removeScriptVar (player, "conversation.inquisitor_admin.branchId");
				
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
			detachScript(self, "conversation.inquisitor_admin");
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
		detachScript (self, "conversation.inquisitor_admin");
		
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
		
		if (inquisitor_admin_condition_hasCompletedKeyDeletedDungeon (player, npc))
		{
			inquisitor_admin_action_grantDungeonQuest (player, npc);
			
			string_id message = new string_id (c_stringFile, "s_62");
			chat.chat (npc, player, message);
			
			return SCRIPT_CONTINUE;
		}
		
		if (inquisitor_admin_condition_hasKey (player, npc))
		{
			
			string_id message = new string_id (c_stringFile, "s_37");
			int numberOfResponses = 0;
			
			boolean hasResponse = false;
			
			boolean hasResponse0 = false;
			if (inquisitor_admin_condition__defaultCondition (player, npc))
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
				
				utils.setScriptVar (player, "conversation.inquisitor_admin.branchId", 2);
				
				npcStartConversation (player, npc, "inquisitor_admin", message, responses);
			}
			else
			{
				chat.chat (npc, player, message);
			}
			
			return SCRIPT_CONTINUE;
		}
		
		if (inquisitor_admin_condition_hasFindKeyTaskActive (player, npc))
		{
			
			string_id message = new string_id (c_stringFile, "s_50");
			chat.chat (npc, player, message);
			
			return SCRIPT_CONTINUE;
		}
		
		if (inquisitor_admin_condition_hasPreviousQuests (player, npc))
		{
			
			string_id message = new string_id (c_stringFile, "s_73");
			int numberOfResponses = 0;
			
			boolean hasResponse = false;
			
			boolean hasResponse0 = false;
			if (inquisitor_admin_condition_hasQuestForInq (player, npc))
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
				
				utils.setScriptVar (player, "conversation.inquisitor_admin.branchId", 10);
				
				npcStartConversation (player, npc, "inquisitor_admin", message, responses);
			}
			else
			{
				chat.chat (npc, player, message);
			}
			
			return SCRIPT_CONTINUE;
		}
		
		if (inquisitor_admin_condition_hasRadio (player, npc))
		{
			
			string_id message = new string_id (c_stringFile, "s_53");
			chat.chat (npc, player, message);
			
			return SCRIPT_CONTINUE;
		}
		
		if (inquisitor_admin_condition__defaultCondition (player, npc))
		{
			doAnimationAction (npc, "shoo");
			
			string_id message = new string_id (c_stringFile, "s_65");
			chat.chat (npc, player, message);
			
			return SCRIPT_CONTINUE;
		}
		
		chat.chat (npc, "Error: All conditions for OnStartNpcConversation were false.");
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int OnNpcConversationResponse(obj_id self, String conversationId, obj_id player, string_id response) throws InterruptedException
	{
		if (!conversationId.equals("inquisitor_admin"))
		{
			return SCRIPT_CONTINUE;
		}
		
		obj_id npc = self;
		
		int branchId = utils.getIntScriptVar (player, "conversation.inquisitor_admin.branchId");
		
		if (branchId == 2 && inquisitor_admin_handleBranch2 (player, npc, response) == SCRIPT_CONTINUE)
		{
			return SCRIPT_CONTINUE;
		}
		
		if (branchId == 3 && inquisitor_admin_handleBranch3 (player, npc, response) == SCRIPT_CONTINUE)
		{
			return SCRIPT_CONTINUE;
		}
		
		if (branchId == 4 && inquisitor_admin_handleBranch4 (player, npc, response) == SCRIPT_CONTINUE)
		{
			return SCRIPT_CONTINUE;
		}
		
		if (branchId == 5 && inquisitor_admin_handleBranch5 (player, npc, response) == SCRIPT_CONTINUE)
		{
			return SCRIPT_CONTINUE;
		}
		
		if (branchId == 6 && inquisitor_admin_handleBranch6 (player, npc, response) == SCRIPT_CONTINUE)
		{
			return SCRIPT_CONTINUE;
		}
		
		if (branchId == 7 && inquisitor_admin_handleBranch7 (player, npc, response) == SCRIPT_CONTINUE)
		{
			return SCRIPT_CONTINUE;
		}
		
		if (branchId == 10 && inquisitor_admin_handleBranch10 (player, npc, response) == SCRIPT_CONTINUE)
		{
			return SCRIPT_CONTINUE;
		}
		
		if (branchId == 11 && inquisitor_admin_handleBranch11 (player, npc, response) == SCRIPT_CONTINUE)
		{
			return SCRIPT_CONTINUE;
		}
		
		if (branchId == 12 && inquisitor_admin_handleBranch12 (player, npc, response) == SCRIPT_CONTINUE)
		{
			return SCRIPT_CONTINUE;
		}
		
		if (branchId == 13 && inquisitor_admin_handleBranch13 (player, npc, response) == SCRIPT_CONTINUE)
		{
			return SCRIPT_CONTINUE;
		}
		
		if (branchId == 14 && inquisitor_admin_handleBranch14 (player, npc, response) == SCRIPT_CONTINUE)
		{
			return SCRIPT_CONTINUE;
		}
		
		if (branchId == 15 && inquisitor_admin_handleBranch15 (player, npc, response) == SCRIPT_CONTINUE)
		{
			return SCRIPT_CONTINUE;
		}
		
		chat.chat (npc, "Error: Fell through all branches and responses for OnNpcConversationResponse.");
		
		utils.removeScriptVar (player, "conversation.inquisitor_admin.branchId");
		
		return SCRIPT_CONTINUE;
	}
	
}
