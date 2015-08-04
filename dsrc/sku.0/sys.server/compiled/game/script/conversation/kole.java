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
import script.library.utils;


public class kole extends script.base_script
{
	public kole()
	{
	}
	String c_stringFile = "conversation/kole";
	
	
	public boolean kole_condition__defaultCondition(obj_id player, obj_id npc) throws InterruptedException
	{
		return true;
	}
	
	
	public boolean kole_condition_hasCompletedMine(obj_id player, obj_id npc) throws InterruptedException
	{
		faceTo(npc, player);
		return groundquests.isQuestActiveOrComplete(player, "u16_nym_themepark_jinkins_kole") && !groundquests.isQuestActiveOrComplete(player, "u16_nym_themepark_second_interview_moore");
	}
	
	
	public boolean kole_condition_hasCompletedMoore(obj_id player, obj_id npc) throws InterruptedException
	{
		faceTo(npc, player);
		return groundquests.isTaskActive(player, "u16_nym_themepark_second_interview_moore", "tellKoleMoore");
	}
	
	
	public boolean kole_condition_hasMooreQuest(obj_id player, obj_id npc) throws InterruptedException
	{
		faceTo(npc, player);
		return groundquests.isQuestActive(player, "u16_nym_themepark_second_interview_moore");
	}
	
	
	public boolean kole_condition_hasMooreNotResearch(obj_id player, obj_id npc) throws InterruptedException
	{
		faceTo(npc, player);
		return (groundquests.hasCompletedQuest(player, "u16_nym_themepark_second_interview_moore") && !groundquests.isQuestActiveOrComplete(player, "u16_nym_themepark_research_facility"));
	}
	
	
	public boolean kole_condition_hasDoneResearchFacility(obj_id player, obj_id npc) throws InterruptedException
	{
		faceTo(npc, player);
		return groundquests.isTaskActive(player, "u16_nym_themepark_research_facility", "returnResearchFacilityComplete");
	}
	
	
	public boolean kole_condition_hasCompletedResearchFacilityDoesntHaveArena(obj_id player, obj_id npc) throws InterruptedException
	{
		faceTo(npc, player);
		return groundquests.hasCompletedQuest(player, "u16_nym_themepark_research_facility") && !groundquests.isQuestActiveOrComplete(player, "u16_nym_themepark_nym_arena");
	}
	
	
	public boolean kole_condition_hasResearchQuestOrNeedsQuest(obj_id player, obj_id npc) throws InterruptedException
	{
		faceTo(npc, player);
		return groundquests.isQuestActive(player, "u16_nym_themepark_research_facility") || kole_condition_hasMooreNotResearch(player, npc);
	}
	
	
	public boolean kole_condition_hasCompletedThemepark(obj_id player, obj_id npc) throws InterruptedException
	{
		faceTo(npc, player);
		return groundquests.hasCompletedQuest(player, "u16_nym_themepark_shuttle_ambush");
	}
	
	
	public void kole_action_grantInterviewMoore(obj_id player, obj_id npc) throws InterruptedException
	{
		faceTo(npc, player);
		groundquests.grantQuest(player, "u16_nym_themepark_second_interview_moore");
	}
	
	
	public void kole_action_grantFacilityQuest(obj_id player, obj_id npc) throws InterruptedException
	{
		faceTo(npc, player);
		if (!groundquests.isQuestActiveOrComplete(player, "u16_nym_themepark_research_facility"))
		{
			groundquests.grantQuest(player, "u16_nym_themepark_research_facility");
		}
		
		if (!hasCompletedCollectionSlot(player, "kill_nyms_themepark_mercenary_activate"))
		{
			modifyCollectionSlotValue(player, "kill_nyms_themepark_mercenary_activate", 1);
		}
		
		if (!hasCompletedCollectionSlot(player, "kill_nyms_themepark_lab_droideka_activate"))
		{
			modifyCollectionSlotValue(player, "kill_nyms_themepark_lab_droideka_activate", 1);
		}
		
		if (groundquests.isTaskActive(player, "u16_nym_themepark_second_interview_moore", "tellKoleMoore"))
		{
			groundquests.sendSignal(player, "hasToldKoleMoore");
		}
	}
	
	
	public void kole_action_finishMooreQuest(obj_id player, obj_id npc) throws InterruptedException
	{
		faceTo(npc, player);
		groundquests.sendSignal(player, "hasToldKoleMoore");
	}
	
	
	public void kole_action_completeResearchFacility(obj_id player, obj_id npc) throws InterruptedException
	{
		faceTo(npc, player);
		groundquests.sendSignal(player, "hasCompletedResearchFacility");
	}
	
	
	public void kole_action_hasFoundKole(obj_id player, obj_id npc) throws InterruptedException
	{
		groundquests.sendSignal(player, "hasFoundKole");
	}
	
	
	public void kole_action_gotoNym(obj_id player, obj_id npc) throws InterruptedException
	{
		groundquests.grantQuest(player, "u16_nym_themepark_kole_to_nym");
	}
	
	
	public int kole_handleBranch3(obj_id player, obj_id npc, string_id response) throws InterruptedException
	{
		
		if (response.equals("s_57"))
		{
			
			if (kole_condition__defaultCondition (player, npc))
			{
				
				string_id message = new string_id (c_stringFile, "s_58");
				int numberOfResponses = 0;
				
				boolean hasResponse = false;
				
				boolean hasResponse0 = false;
				if (kole_condition__defaultCondition (player, npc))
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
					
					utils.setScriptVar (player, "conversation.kole.branchId", 4);
					
					npcSpeak (player, message);
					npcSetConversationResponses (player, responses);
				}
				else
				{
					utils.removeScriptVar (player, "conversation.kole.branchId");
					
					npcEndConversationWithMessage (player, message);
				}
				
				return SCRIPT_CONTINUE;
			}
			
		}
		
		return SCRIPT_DEFAULT;
	}
	
	
	public int kole_handleBranch4(obj_id player, obj_id npc, string_id response) throws InterruptedException
	{
		
		if (response.equals("s_59"))
		{
			
			if (kole_condition__defaultCondition (player, npc))
			{
				kole_action_completeResearchFacility (player, npc);
				
				string_id message = new string_id (c_stringFile, "s_60");
				utils.removeScriptVar (player, "conversation.kole.branchId");
				
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
	
	
	public int kole_handleBranch6(obj_id player, obj_id npc, string_id response) throws InterruptedException
	{
		
		if (response.equals("s_73"))
		{
			
			if (kole_condition__defaultCondition (player, npc))
			{
				kole_action_grantFacilityQuest (player, npc);
				
				string_id message = new string_id (c_stringFile, "s_71");
				utils.removeScriptVar (player, "conversation.kole.branchId");
				
				npcEndConversationWithMessage (player, message);
				
				return SCRIPT_CONTINUE;
			}
			
		}
		
		return SCRIPT_DEFAULT;
	}
	
	
	public int kole_handleBranch7(obj_id player, obj_id npc, string_id response) throws InterruptedException
	{
		
		if (response.equals("s_25"))
		{
			
			if (kole_condition__defaultCondition (player, npc))
			{
				
				string_id message = new string_id (c_stringFile, "s_26");
				int numberOfResponses = 0;
				
				boolean hasResponse = false;
				
				boolean hasResponse0 = false;
				if (kole_condition__defaultCondition (player, npc))
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
					
					utils.setScriptVar (player, "conversation.kole.branchId", 8);
					
					npcSpeak (player, message);
					npcSetConversationResponses (player, responses);
				}
				else
				{
					utils.removeScriptVar (player, "conversation.kole.branchId");
					
					npcEndConversationWithMessage (player, message);
				}
				
				return SCRIPT_CONTINUE;
			}
			
		}
		
		return SCRIPT_DEFAULT;
	}
	
	
	public int kole_handleBranch8(obj_id player, obj_id npc, string_id response) throws InterruptedException
	{
		
		if (response.equals("s_27"))
		{
			
			if (kole_condition__defaultCondition (player, npc))
			{
				
				string_id message = new string_id (c_stringFile, "s_28");
				int numberOfResponses = 0;
				
				boolean hasResponse = false;
				
				boolean hasResponse0 = false;
				if (kole_condition__defaultCondition (player, npc))
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
						responses[responseIndex++] = new string_id (c_stringFile, "s_69");
					}
					
					utils.setScriptVar (player, "conversation.kole.branchId", 9);
					
					npcSpeak (player, message);
					npcSetConversationResponses (player, responses);
				}
				else
				{
					utils.removeScriptVar (player, "conversation.kole.branchId");
					
					npcEndConversationWithMessage (player, message);
				}
				
				return SCRIPT_CONTINUE;
			}
			
		}
		
		return SCRIPT_DEFAULT;
	}
	
	
	public int kole_handleBranch9(obj_id player, obj_id npc, string_id response) throws InterruptedException
	{
		
		if (response.equals("s_69"))
		{
			
			if (kole_condition__defaultCondition (player, npc))
			{
				kole_action_grantFacilityQuest (player, npc);
				
				string_id message = new string_id (c_stringFile, "s_71");
				utils.removeScriptVar (player, "conversation.kole.branchId");
				
				npcEndConversationWithMessage (player, message);
				
				return SCRIPT_CONTINUE;
			}
			
		}
		
		return SCRIPT_DEFAULT;
	}
	
	
	public int kole_handleBranch12(obj_id player, obj_id npc, string_id response) throws InterruptedException
	{
		
		if (response.equals("s_22"))
		{
			
			if (kole_condition__defaultCondition (player, npc))
			{
				
				string_id message = new string_id (c_stringFile, "s_29");
				int numberOfResponses = 0;
				
				boolean hasResponse = false;
				
				boolean hasResponse0 = false;
				if (kole_condition__defaultCondition (player, npc))
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
					
					utils.setScriptVar (player, "conversation.kole.branchId", 13);
					
					npcSpeak (player, message);
					npcSetConversationResponses (player, responses);
				}
				else
				{
					utils.removeScriptVar (player, "conversation.kole.branchId");
					
					npcEndConversationWithMessage (player, message);
				}
				
				return SCRIPT_CONTINUE;
			}
			
		}
		
		return SCRIPT_DEFAULT;
	}
	
	
	public int kole_handleBranch13(obj_id player, obj_id npc, string_id response) throws InterruptedException
	{
		
		if (response.equals("s_31"))
		{
			
			if (kole_condition__defaultCondition (player, npc))
			{
				
				string_id message = new string_id (c_stringFile, "s_33");
				int numberOfResponses = 0;
				
				boolean hasResponse = false;
				
				boolean hasResponse0 = false;
				if (kole_condition__defaultCondition (player, npc))
				{
					++numberOfResponses;
					hasResponse = true;
					hasResponse0 = true;
				}
				
				boolean hasResponse1 = false;
				if (kole_condition__defaultCondition (player, npc))
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
						responses[responseIndex++] = new string_id (c_stringFile, "s_35");
					}
					
					if (hasResponse1)
					{
						responses[responseIndex++] = new string_id (c_stringFile, "s_54");
					}
					
					utils.setScriptVar (player, "conversation.kole.branchId", 14);
					
					npcSpeak (player, message);
					npcSetConversationResponses (player, responses);
				}
				else
				{
					utils.removeScriptVar (player, "conversation.kole.branchId");
					
					npcEndConversationWithMessage (player, message);
				}
				
				return SCRIPT_CONTINUE;
			}
			
		}
		
		return SCRIPT_DEFAULT;
	}
	
	
	public int kole_handleBranch14(obj_id player, obj_id npc, string_id response) throws InterruptedException
	{
		
		if (response.equals("s_35"))
		{
			
			if (kole_condition__defaultCondition (player, npc))
			{
				
				string_id message = new string_id (c_stringFile, "s_37");
				int numberOfResponses = 0;
				
				boolean hasResponse = false;
				
				boolean hasResponse0 = false;
				if (kole_condition__defaultCondition (player, npc))
				{
					++numberOfResponses;
					hasResponse = true;
					hasResponse0 = true;
				}
				
				boolean hasResponse1 = false;
				if (kole_condition__defaultCondition (player, npc))
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
						responses[responseIndex++] = new string_id (c_stringFile, "s_39");
					}
					
					if (hasResponse1)
					{
						responses[responseIndex++] = new string_id (c_stringFile, "s_41");
					}
					
					utils.setScriptVar (player, "conversation.kole.branchId", 15);
					
					npcSpeak (player, message);
					npcSetConversationResponses (player, responses);
				}
				else
				{
					utils.removeScriptVar (player, "conversation.kole.branchId");
					
					npcEndConversationWithMessage (player, message);
				}
				
				return SCRIPT_CONTINUE;
			}
			
		}
		
		if (response.equals("s_54"))
		{
			
			if (kole_condition__defaultCondition (player, npc))
			{
				
				string_id message = new string_id (c_stringFile, "s_63");
				int numberOfResponses = 0;
				
				boolean hasResponse = false;
				
				boolean hasResponse0 = false;
				if (kole_condition__defaultCondition (player, npc))
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
						responses[responseIndex++] = new string_id (c_stringFile, "s_65");
					}
					
					utils.setScriptVar (player, "conversation.kole.branchId", 19);
					
					npcSpeak (player, message);
					npcSetConversationResponses (player, responses);
				}
				else
				{
					utils.removeScriptVar (player, "conversation.kole.branchId");
					
					npcEndConversationWithMessage (player, message);
				}
				
				return SCRIPT_CONTINUE;
			}
			
		}
		
		return SCRIPT_DEFAULT;
	}
	
	
	public int kole_handleBranch15(obj_id player, obj_id npc, string_id response) throws InterruptedException
	{
		
		if (response.equals("s_39"))
		{
			
			if (kole_condition__defaultCondition (player, npc))
			{
				
				string_id message = new string_id (c_stringFile, "s_43");
				int numberOfResponses = 0;
				
				boolean hasResponse = false;
				
				boolean hasResponse0 = false;
				if (kole_condition__defaultCondition (player, npc))
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
						responses[responseIndex++] = new string_id (c_stringFile, "s_45");
					}
					
					utils.setScriptVar (player, "conversation.kole.branchId", 16);
					
					npcSpeak (player, message);
					npcSetConversationResponses (player, responses);
				}
				else
				{
					utils.removeScriptVar (player, "conversation.kole.branchId");
					
					npcEndConversationWithMessage (player, message);
				}
				
				return SCRIPT_CONTINUE;
			}
			
		}
		
		if (response.equals("s_41"))
		{
			
			if (kole_condition__defaultCondition (player, npc))
			{
				
				string_id message = new string_id (c_stringFile, "s_43");
				int numberOfResponses = 0;
				
				boolean hasResponse = false;
				
				boolean hasResponse0 = false;
				if (kole_condition__defaultCondition (player, npc))
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
						responses[responseIndex++] = new string_id (c_stringFile, "s_45");
					}
					
					utils.setScriptVar (player, "conversation.kole.branchId", 16);
					
					npcSpeak (player, message);
					npcSetConversationResponses (player, responses);
				}
				else
				{
					utils.removeScriptVar (player, "conversation.kole.branchId");
					
					npcEndConversationWithMessage (player, message);
				}
				
				return SCRIPT_CONTINUE;
			}
			
		}
		
		return SCRIPT_DEFAULT;
	}
	
	
	public int kole_handleBranch16(obj_id player, obj_id npc, string_id response) throws InterruptedException
	{
		
		if (response.equals("s_45"))
		{
			
			if (kole_condition__defaultCondition (player, npc))
			{
				
				string_id message = new string_id (c_stringFile, "s_47");
				int numberOfResponses = 0;
				
				boolean hasResponse = false;
				
				boolean hasResponse0 = false;
				if (kole_condition__defaultCondition (player, npc))
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
						responses[responseIndex++] = new string_id (c_stringFile, "s_49");
					}
					
					utils.setScriptVar (player, "conversation.kole.branchId", 17);
					
					npcSpeak (player, message);
					npcSetConversationResponses (player, responses);
				}
				else
				{
					utils.removeScriptVar (player, "conversation.kole.branchId");
					
					npcEndConversationWithMessage (player, message);
				}
				
				return SCRIPT_CONTINUE;
			}
			
		}
		
		return SCRIPT_DEFAULT;
	}
	
	
	public int kole_handleBranch17(obj_id player, obj_id npc, string_id response) throws InterruptedException
	{
		
		if (response.equals("s_49"))
		{
			
			if (kole_condition__defaultCondition (player, npc))
			{
				kole_action_grantInterviewMoore (player, npc);
				
				string_id message = new string_id (c_stringFile, "s_51");
				utils.removeScriptVar (player, "conversation.kole.branchId");
				
				npcEndConversationWithMessage (player, message);
				
				return SCRIPT_CONTINUE;
			}
			
		}
		
		return SCRIPT_DEFAULT;
	}
	
	
	public int kole_handleBranch19(obj_id player, obj_id npc, string_id response) throws InterruptedException
	{
		
		if (response.equals("s_65"))
		{
			
			if (kole_condition__defaultCondition (player, npc))
			{
				
				string_id message = new string_id (c_stringFile, "s_67");
				int numberOfResponses = 0;
				
				boolean hasResponse = false;
				
				boolean hasResponse0 = false;
				if (kole_condition__defaultCondition (player, npc))
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
						responses[responseIndex++] = new string_id (c_stringFile, "s_70");
					}
					
					utils.setScriptVar (player, "conversation.kole.branchId", 20);
					
					npcSpeak (player, message);
					npcSetConversationResponses (player, responses);
				}
				else
				{
					utils.removeScriptVar (player, "conversation.kole.branchId");
					
					npcEndConversationWithMessage (player, message);
				}
				
				return SCRIPT_CONTINUE;
			}
			
		}
		
		return SCRIPT_DEFAULT;
	}
	
	
	public int kole_handleBranch20(obj_id player, obj_id npc, string_id response) throws InterruptedException
	{
		
		if (response.equals("s_70"))
		{
			
			if (kole_condition__defaultCondition (player, npc))
			{
				
				string_id message = new string_id (c_stringFile, "s_37");
				int numberOfResponses = 0;
				
				boolean hasResponse = false;
				
				boolean hasResponse0 = false;
				if (kole_condition__defaultCondition (player, npc))
				{
					++numberOfResponses;
					hasResponse = true;
					hasResponse0 = true;
				}
				
				boolean hasResponse1 = false;
				if (kole_condition__defaultCondition (player, npc))
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
						responses[responseIndex++] = new string_id (c_stringFile, "s_39");
					}
					
					if (hasResponse1)
					{
						responses[responseIndex++] = new string_id (c_stringFile, "s_41");
					}
					
					utils.setScriptVar (player, "conversation.kole.branchId", 15);
					
					npcSpeak (player, message);
					npcSetConversationResponses (player, responses);
				}
				else
				{
					utils.removeScriptVar (player, "conversation.kole.branchId");
					
					npcEndConversationWithMessage (player, message);
				}
				
				return SCRIPT_CONTINUE;
			}
			
		}
		
		return SCRIPT_DEFAULT;
	}
	
	
	public int OnInitialize(obj_id self) throws InterruptedException
	{
		if ((!isTangible (self)) || (isPlayer (self)))
		{
			detachScript(self, "conversation.kole");
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
		detachScript (self, "conversation.kole");
		
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
		
		if (kole_condition_hasCompletedThemepark (player, npc))
		{
			
			string_id message = new string_id (c_stringFile, "s_75");
			chat.chat (npc, player, message);
			
			return SCRIPT_CONTINUE;
		}
		
		if (kole_condition_hasCompletedResearchFacilityDoesntHaveArena (player, npc))
		{
			kole_action_gotoNym (player, npc);
			
			string_id message = new string_id (c_stringFile, "s_61");
			prose_package pp = new prose_package ();
			pp.stringId = message;
			pp.actor.set (player);
			pp.target.set (npc);
			
			chat.chat (npc, player, null, null, pp);
			return SCRIPT_CONTINUE;
		}
		
		if (kole_condition_hasDoneResearchFacility (player, npc))
		{
			doAnimationAction (npc, "applause_excited");
			
			string_id message = new string_id (c_stringFile, "s_56");
			int numberOfResponses = 0;
			
			boolean hasResponse = false;
			
			boolean hasResponse0 = false;
			if (kole_condition__defaultCondition (player, npc))
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
					responses[responseIndex++] = new string_id (c_stringFile, "s_57");
				}
				
				utils.setScriptVar (player, "conversation.kole.branchId", 3);
				
				npcStartConversation (player, npc, "kole", message, responses);
			}
			else
			{
				chat.chat (npc, player, message);
			}
			
			return SCRIPT_CONTINUE;
		}
		
		if (kole_condition_hasResearchQuestOrNeedsQuest (player, npc))
		{
			
			string_id message = new string_id (c_stringFile, "s_55");
			int numberOfResponses = 0;
			
			boolean hasResponse = false;
			
			boolean hasResponse0 = false;
			if (kole_condition__defaultCondition (player, npc))
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
					responses[responseIndex++] = new string_id (c_stringFile, "s_73");
				}
				
				utils.setScriptVar (player, "conversation.kole.branchId", 6);
				
				npcStartConversation (player, npc, "kole", message, responses);
			}
			else
			{
				chat.chat (npc, player, message);
			}
			
			return SCRIPT_CONTINUE;
		}
		
		if (kole_condition_hasCompletedMoore (player, npc))
		{
			doAnimationAction (npc, "applause_polite");
			
			kole_action_finishMooreQuest (player, npc);
			
			string_id message = new string_id (c_stringFile, "s_24");
			int numberOfResponses = 0;
			
			boolean hasResponse = false;
			
			boolean hasResponse0 = false;
			if (kole_condition__defaultCondition (player, npc))
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
				
				utils.setScriptVar (player, "conversation.kole.branchId", 7);
				
				npcStartConversation (player, npc, "kole", message, responses);
			}
			else
			{
				chat.chat (npc, player, message);
			}
			
			return SCRIPT_CONTINUE;
		}
		
		if (kole_condition_hasMooreQuest (player, npc))
		{
			doAnimationAction (npc, "backhand_threaten");
			
			string_id message = new string_id (c_stringFile, "s_53");
			chat.chat (npc, player, message);
			
			return SCRIPT_CONTINUE;
		}
		
		if (kole_condition_hasCompletedMine (player, npc))
		{
			doAnimationAction (npc, "shoo");
			
			kole_action_hasFoundKole (player, npc);
			
			string_id message = new string_id (c_stringFile, "s_20");
			int numberOfResponses = 0;
			
			boolean hasResponse = false;
			
			boolean hasResponse0 = false;
			if (kole_condition__defaultCondition (player, npc))
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
				
				utils.setScriptVar (player, "conversation.kole.branchId", 12);
				
				npcStartConversation (player, npc, "kole", message, responses);
			}
			else
			{
				chat.chat (npc, player, message);
			}
			
			return SCRIPT_CONTINUE;
		}
		
		if (kole_condition__defaultCondition (player, npc))
		{
			doAnimationAction (npc, "rude");
			
			string_id message = new string_id (c_stringFile, "s_74");
			chat.chat (npc, player, message);
			
			return SCRIPT_CONTINUE;
		}
		
		chat.chat (npc, "Error: All conditions for OnStartNpcConversation were false.");
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int OnNpcConversationResponse(obj_id self, String conversationId, obj_id player, string_id response) throws InterruptedException
	{
		if (!conversationId.equals("kole"))
		{
			return SCRIPT_CONTINUE;
		}
		
		obj_id npc = self;
		
		int branchId = utils.getIntScriptVar (player, "conversation.kole.branchId");
		
		if (branchId == 3 && kole_handleBranch3 (player, npc, response) == SCRIPT_CONTINUE)
		{
			return SCRIPT_CONTINUE;
		}
		
		if (branchId == 4 && kole_handleBranch4 (player, npc, response) == SCRIPT_CONTINUE)
		{
			return SCRIPT_CONTINUE;
		}
		
		if (branchId == 6 && kole_handleBranch6 (player, npc, response) == SCRIPT_CONTINUE)
		{
			return SCRIPT_CONTINUE;
		}
		
		if (branchId == 7 && kole_handleBranch7 (player, npc, response) == SCRIPT_CONTINUE)
		{
			return SCRIPT_CONTINUE;
		}
		
		if (branchId == 8 && kole_handleBranch8 (player, npc, response) == SCRIPT_CONTINUE)
		{
			return SCRIPT_CONTINUE;
		}
		
		if (branchId == 9 && kole_handleBranch9 (player, npc, response) == SCRIPT_CONTINUE)
		{
			return SCRIPT_CONTINUE;
		}
		
		if (branchId == 12 && kole_handleBranch12 (player, npc, response) == SCRIPT_CONTINUE)
		{
			return SCRIPT_CONTINUE;
		}
		
		if (branchId == 13 && kole_handleBranch13 (player, npc, response) == SCRIPT_CONTINUE)
		{
			return SCRIPT_CONTINUE;
		}
		
		if (branchId == 14 && kole_handleBranch14 (player, npc, response) == SCRIPT_CONTINUE)
		{
			return SCRIPT_CONTINUE;
		}
		
		if (branchId == 15 && kole_handleBranch15 (player, npc, response) == SCRIPT_CONTINUE)
		{
			return SCRIPT_CONTINUE;
		}
		
		if (branchId == 16 && kole_handleBranch16 (player, npc, response) == SCRIPT_CONTINUE)
		{
			return SCRIPT_CONTINUE;
		}
		
		if (branchId == 17 && kole_handleBranch17 (player, npc, response) == SCRIPT_CONTINUE)
		{
			return SCRIPT_CONTINUE;
		}
		
		if (branchId == 19 && kole_handleBranch19 (player, npc, response) == SCRIPT_CONTINUE)
		{
			return SCRIPT_CONTINUE;
		}
		
		if (branchId == 20 && kole_handleBranch20 (player, npc, response) == SCRIPT_CONTINUE)
		{
			return SCRIPT_CONTINUE;
		}
		
		chat.chat (npc, "Error: Fell through all branches and responses for OnNpcConversationResponse.");
		
		utils.removeScriptVar (player, "conversation.kole.branchId");
		
		return SCRIPT_CONTINUE;
	}
	
}
