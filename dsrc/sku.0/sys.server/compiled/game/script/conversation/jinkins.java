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


public class jinkins extends script.base_script
{
	public jinkins()
	{
	}
	String c_stringFile = "conversation/jinkins";
	
	
	public boolean jinkins_condition__defaultCondition(obj_id player, obj_id npc) throws InterruptedException
	{
		return true;
	}
	
	
	public boolean jinkins_condition_hasCompletedPirate(obj_id player, obj_id npc) throws InterruptedException
	{
		faceTo(npc, player);
		return groundquests.isQuestActiveOrComplete(player, "u16_nym_themepark_vana_to_jinkins") && !groundquests.isQuestActiveOrComplete(player, "u16_nym_themepark_interview_moore");
	}
	
	
	public boolean jinkins_condition_hasReturnedFromMoore(obj_id player, obj_id npc) throws InterruptedException
	{
		faceTo(npc, player);
		return groundquests.isTaskActive(player, "u16_nym_themepark_interview_moore", "tellJinkinsMoore") || (groundquests.hasCompletedQuest(player, "u16_nym_themepark_interview_moore") && !groundquests.isQuestActiveOrComplete(player, "u16_nym_themepark_mine"));
	}
	
	
	public boolean jinkins_condition_hasMineQuestIncomplete(obj_id player, obj_id npc) throws InterruptedException
	{
		return groundquests.isQuestActive(player, "u16_nym_themepark_mine");
	}
	
	
	public boolean jinkins_condition_hasMineFilter(obj_id player, obj_id npc) throws InterruptedException
	{
		faceTo(npc, player);
		return groundquests.isTaskActive(player, "u16_nym_themepark_mine", "returnMineComplete");
	}
	
	
	public boolean jinkins_condition_isDoneMineQuest(obj_id player, obj_id npc) throws InterruptedException
	{
		return groundquests.hasCompletedQuest(player, "u16_nym_themepark_mine");
	}
	
	
	public boolean jinkins_condition_allQuestsComplete(obj_id player, obj_id npc) throws InterruptedException
	{
		return groundquests.hasCompletedQuest(player, "u16_nym_themepark_jinkins_kole");
	}
	
	
	public boolean jinkins_condition_hasCompletedThemePark(obj_id player, obj_id npc) throws InterruptedException
	{
		faceTo(npc, player);
		return groundquests.hasCompletedQuest(player, "u16_nym_themepark_shuttle_ambush");
	}
	
	
	public boolean jinkins_condition_hasNotCompletedPirate(obj_id player, obj_id npc) throws InterruptedException
	{
		faceTo(npc, player);
		return !groundquests.hasCompletedQuest(player, "u16_nym_themepark_pirate_hideout") || !groundquests.hasCompletedQuest(player, "u16_nym_themepark_vana_to_jinkins");
	}
	
	
	public void jinkins_action_giveMooreQuest(obj_id player, obj_id npc) throws InterruptedException
	{
		groundquests.grantQuest(player, "u16_nym_themepark_interview_moore");
	}
	
	
	public void jinkins_action_grantMineQuest(obj_id player, obj_id npc) throws InterruptedException
	{
		groundquests.grantQuest(player, "u16_nym_themepark_mine");
		
		if (!hasCompletedCollectionSlot(player, "kill_nyms_themepark_enraged_miner_activate"))
		{
			modifyCollectionSlotValue(player, "kill_nyms_themepark_enraged_miner_activate", 1);
		}
		
		if (!hasCompletedCollectionSlot(player, "kill_nyms_themepark_mine_droideka_activate"))
		{
			modifyCollectionSlotValue(player, "kill_nyms_themepark_mine_droideka_activate", 1);
		}
	}
	
	
	public void jinkins_action_completeMooreQuest(obj_id player, obj_id npc) throws InterruptedException
	{
		faceTo(npc, player);
		groundquests.sendSignal(player, "hasToldJinkinsMoore");
	}
	
	
	public void jinkins_action_completeMineQuest(obj_id player, obj_id npc) throws InterruptedException
	{
		faceTo(npc, player);
		groundquests.sendSignal(player, "hasCompletedMine");
	}
	
	
	public void jinkins_action_hasFoundJinkins(obj_id player, obj_id npc) throws InterruptedException
	{
		groundquests.sendSignal(player, "hasFoundJinkins");
	}
	
	
	public void jinkins_action_gotoKole(obj_id player, obj_id npc) throws InterruptedException
	{
		groundquests.grantQuest(player, "u16_nym_themepark_jinkins_kole");
	}
	
	
	public int jinkins_handleBranch4(obj_id player, obj_id npc, string_id response) throws InterruptedException
	{
		
		if (response.equals("s_39"))
		{
			
			if (jinkins_condition__defaultCondition (player, npc))
			{
				jinkins_action_completeMineQuest (player, npc);
				
				string_id message = new string_id (c_stringFile, "s_40");
				utils.removeScriptVar (player, "conversation.jinkins.branchId");
				
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
	
	
	public int jinkins_handleBranch6(obj_id player, obj_id npc, string_id response) throws InterruptedException
	{
		
		if (response.equals("s_147"))
		{
			
			if (jinkins_condition__defaultCondition (player, npc))
			{
				doAnimationAction (npc, "backhand");
				
				string_id message = new string_id (c_stringFile, "s_148");
				utils.removeScriptVar (player, "conversation.jinkins.branchId");
				
				npcEndConversationWithMessage (player, message);
				
				return SCRIPT_CONTINUE;
			}
			
		}
		
		return SCRIPT_DEFAULT;
	}
	
	
	public int jinkins_handleBranch8(obj_id player, obj_id npc, string_id response) throws InterruptedException
	{
		
		if (response.equals("s_136"))
		{
			
			if (jinkins_condition__defaultCondition (player, npc))
			{
				
				string_id message = new string_id (c_stringFile, "s_137");
				int numberOfResponses = 0;
				
				boolean hasResponse = false;
				
				boolean hasResponse0 = false;
				if (jinkins_condition__defaultCondition (player, npc))
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
						responses[responseIndex++] = new string_id (c_stringFile, "s_138");
					}
					
					utils.setScriptVar (player, "conversation.jinkins.branchId", 9);
					
					npcSpeak (player, message);
					npcSetConversationResponses (player, responses);
				}
				else
				{
					utils.removeScriptVar (player, "conversation.jinkins.branchId");
					
					npcEndConversationWithMessage (player, message);
				}
				
				return SCRIPT_CONTINUE;
			}
			
		}
		
		return SCRIPT_DEFAULT;
	}
	
	
	public int jinkins_handleBranch9(obj_id player, obj_id npc, string_id response) throws InterruptedException
	{
		
		if (response.equals("s_138"))
		{
			
			if (jinkins_condition__defaultCondition (player, npc))
			{
				
				string_id message = new string_id (c_stringFile, "s_139");
				int numberOfResponses = 0;
				
				boolean hasResponse = false;
				
				boolean hasResponse0 = false;
				if (jinkins_condition__defaultCondition (player, npc))
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
						responses[responseIndex++] = new string_id (c_stringFile, "s_140");
					}
					
					utils.setScriptVar (player, "conversation.jinkins.branchId", 10);
					
					npcSpeak (player, message);
					npcSetConversationResponses (player, responses);
				}
				else
				{
					utils.removeScriptVar (player, "conversation.jinkins.branchId");
					
					npcEndConversationWithMessage (player, message);
				}
				
				return SCRIPT_CONTINUE;
			}
			
		}
		
		return SCRIPT_DEFAULT;
	}
	
	
	public int jinkins_handleBranch10(obj_id player, obj_id npc, string_id response) throws InterruptedException
	{
		
		if (response.equals("s_140"))
		{
			
			if (jinkins_condition__defaultCondition (player, npc))
			{
				
				string_id message = new string_id (c_stringFile, "s_141");
				int numberOfResponses = 0;
				
				boolean hasResponse = false;
				
				boolean hasResponse0 = false;
				if (jinkins_condition__defaultCondition (player, npc))
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
						responses[responseIndex++] = new string_id (c_stringFile, "s_142");
					}
					
					utils.setScriptVar (player, "conversation.jinkins.branchId", 11);
					
					npcSpeak (player, message);
					npcSetConversationResponses (player, responses);
				}
				else
				{
					utils.removeScriptVar (player, "conversation.jinkins.branchId");
					
					npcEndConversationWithMessage (player, message);
				}
				
				return SCRIPT_CONTINUE;
			}
			
		}
		
		return SCRIPT_DEFAULT;
	}
	
	
	public int jinkins_handleBranch11(obj_id player, obj_id npc, string_id response) throws InterruptedException
	{
		
		if (response.equals("s_142"))
		{
			
			if (jinkins_condition__defaultCondition (player, npc))
			{
				
				string_id message = new string_id (c_stringFile, "s_143");
				int numberOfResponses = 0;
				
				boolean hasResponse = false;
				
				boolean hasResponse0 = false;
				if (jinkins_condition__defaultCondition (player, npc))
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
						responses[responseIndex++] = new string_id (c_stringFile, "s_144");
					}
					
					utils.setScriptVar (player, "conversation.jinkins.branchId", 12);
					
					npcSpeak (player, message);
					npcSetConversationResponses (player, responses);
				}
				else
				{
					utils.removeScriptVar (player, "conversation.jinkins.branchId");
					
					npcEndConversationWithMessage (player, message);
				}
				
				return SCRIPT_CONTINUE;
			}
			
		}
		
		return SCRIPT_DEFAULT;
	}
	
	
	public int jinkins_handleBranch12(obj_id player, obj_id npc, string_id response) throws InterruptedException
	{
		
		if (response.equals("s_144"))
		{
			
			if (jinkins_condition__defaultCondition (player, npc))
			{
				
				string_id message = new string_id (c_stringFile, "s_145");
				int numberOfResponses = 0;
				
				boolean hasResponse = false;
				
				boolean hasResponse0 = false;
				if (jinkins_condition__defaultCondition (player, npc))
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
						responses[responseIndex++] = new string_id (c_stringFile, "s_46");
					}
					
					utils.setScriptVar (player, "conversation.jinkins.branchId", 13);
					
					npcSpeak (player, message);
					npcSetConversationResponses (player, responses);
				}
				else
				{
					utils.removeScriptVar (player, "conversation.jinkins.branchId");
					
					npcEndConversationWithMessage (player, message);
				}
				
				return SCRIPT_CONTINUE;
			}
			
		}
		
		return SCRIPT_DEFAULT;
	}
	
	
	public int jinkins_handleBranch13(obj_id player, obj_id npc, string_id response) throws InterruptedException
	{
		
		if (response.equals("s_46"))
		{
			
			if (jinkins_condition__defaultCondition (player, npc))
			{
				jinkins_action_grantMineQuest (player, npc);
				
				string_id message = new string_id (c_stringFile, "s_47");
				utils.removeScriptVar (player, "conversation.jinkins.branchId");
				
				npcEndConversationWithMessage (player, message);
				
				return SCRIPT_CONTINUE;
			}
			
		}
		
		return SCRIPT_DEFAULT;
	}
	
	
	public int jinkins_handleBranch15(obj_id player, obj_id npc, string_id response) throws InterruptedException
	{
		
		if (response.equals("s_101"))
		{
			
			if (jinkins_condition__defaultCondition (player, npc))
			{
				
				string_id message = new string_id (c_stringFile, "s_102");
				int numberOfResponses = 0;
				
				boolean hasResponse = false;
				
				boolean hasResponse0 = false;
				if (jinkins_condition__defaultCondition (player, npc))
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
						responses[responseIndex++] = new string_id (c_stringFile, "s_104");
					}
					
					utils.setScriptVar (player, "conversation.jinkins.branchId", 16);
					
					npcSpeak (player, message);
					npcSetConversationResponses (player, responses);
				}
				else
				{
					utils.removeScriptVar (player, "conversation.jinkins.branchId");
					
					npcEndConversationWithMessage (player, message);
				}
				
				return SCRIPT_CONTINUE;
			}
			
		}
		
		if (response.equals("s_103"))
		{
			
			if (jinkins_condition__defaultCondition (player, npc))
			{
				
				string_id message = new string_id (c_stringFile, "s_106");
				int numberOfResponses = 0;
				
				boolean hasResponse = false;
				
				boolean hasResponse0 = false;
				if (jinkins_condition__defaultCondition (player, npc))
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
						responses[responseIndex++] = new string_id (c_stringFile, "s_107");
					}
					
					utils.setScriptVar (player, "conversation.jinkins.branchId", 17);
					
					npcSpeak (player, message);
					npcSetConversationResponses (player, responses);
				}
				else
				{
					utils.removeScriptVar (player, "conversation.jinkins.branchId");
					
					npcEndConversationWithMessage (player, message);
				}
				
				return SCRIPT_CONTINUE;
			}
			
		}
		
		return SCRIPT_DEFAULT;
	}
	
	
	public int jinkins_handleBranch16(obj_id player, obj_id npc, string_id response) throws InterruptedException
	{
		
		if (response.equals("s_104"))
		{
			
			if (jinkins_condition__defaultCondition (player, npc))
			{
				
				string_id message = new string_id (c_stringFile, "s_106");
				int numberOfResponses = 0;
				
				boolean hasResponse = false;
				
				boolean hasResponse0 = false;
				if (jinkins_condition__defaultCondition (player, npc))
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
						responses[responseIndex++] = new string_id (c_stringFile, "s_107");
					}
					
					utils.setScriptVar (player, "conversation.jinkins.branchId", 17);
					
					npcSpeak (player, message);
					npcSetConversationResponses (player, responses);
				}
				else
				{
					utils.removeScriptVar (player, "conversation.jinkins.branchId");
					
					npcEndConversationWithMessage (player, message);
				}
				
				return SCRIPT_CONTINUE;
			}
			
		}
		
		return SCRIPT_DEFAULT;
	}
	
	
	public int jinkins_handleBranch17(obj_id player, obj_id npc, string_id response) throws InterruptedException
	{
		
		if (response.equals("s_107"))
		{
			
			if (jinkins_condition__defaultCondition (player, npc))
			{
				
				string_id message = new string_id (c_stringFile, "s_108");
				int numberOfResponses = 0;
				
				boolean hasResponse = false;
				
				boolean hasResponse0 = false;
				if (jinkins_condition__defaultCondition (player, npc))
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
						responses[responseIndex++] = new string_id (c_stringFile, "s_109");
					}
					
					utils.setScriptVar (player, "conversation.jinkins.branchId", 18);
					
					npcSpeak (player, message);
					npcSetConversationResponses (player, responses);
				}
				else
				{
					utils.removeScriptVar (player, "conversation.jinkins.branchId");
					
					npcEndConversationWithMessage (player, message);
				}
				
				return SCRIPT_CONTINUE;
			}
			
		}
		
		return SCRIPT_DEFAULT;
	}
	
	
	public int jinkins_handleBranch18(obj_id player, obj_id npc, string_id response) throws InterruptedException
	{
		
		if (response.equals("s_109"))
		{
			
			if (jinkins_condition__defaultCondition (player, npc))
			{
				
				string_id message = new string_id (c_stringFile, "s_110");
				int numberOfResponses = 0;
				
				boolean hasResponse = false;
				
				boolean hasResponse0 = false;
				if (jinkins_condition__defaultCondition (player, npc))
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
						responses[responseIndex++] = new string_id (c_stringFile, "s_119");
					}
					
					utils.setScriptVar (player, "conversation.jinkins.branchId", 19);
					
					npcSpeak (player, message);
					npcSetConversationResponses (player, responses);
				}
				else
				{
					utils.removeScriptVar (player, "conversation.jinkins.branchId");
					
					npcEndConversationWithMessage (player, message);
				}
				
				return SCRIPT_CONTINUE;
			}
			
		}
		
		return SCRIPT_DEFAULT;
	}
	
	
	public int jinkins_handleBranch19(obj_id player, obj_id npc, string_id response) throws InterruptedException
	{
		
		if (response.equals("s_119"))
		{
			
			if (jinkins_condition__defaultCondition (player, npc))
			{
				
				string_id message = new string_id (c_stringFile, "s_121");
				int numberOfResponses = 0;
				
				boolean hasResponse = false;
				
				boolean hasResponse0 = false;
				if (jinkins_condition__defaultCondition (player, npc))
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
						responses[responseIndex++] = new string_id (c_stringFile, "s_123");
					}
					
					utils.setScriptVar (player, "conversation.jinkins.branchId", 20);
					
					npcSpeak (player, message);
					npcSetConversationResponses (player, responses);
				}
				else
				{
					utils.removeScriptVar (player, "conversation.jinkins.branchId");
					
					npcEndConversationWithMessage (player, message);
				}
				
				return SCRIPT_CONTINUE;
			}
			
		}
		
		return SCRIPT_DEFAULT;
	}
	
	
	public int jinkins_handleBranch20(obj_id player, obj_id npc, string_id response) throws InterruptedException
	{
		
		if (response.equals("s_123"))
		{
			
			if (jinkins_condition__defaultCondition (player, npc))
			{
				
				string_id message = new string_id (c_stringFile, "s_125");
				int numberOfResponses = 0;
				
				boolean hasResponse = false;
				
				boolean hasResponse0 = false;
				if (jinkins_condition__defaultCondition (player, npc))
				{
					++numberOfResponses;
					hasResponse = true;
					hasResponse0 = true;
				}
				
				boolean hasResponse1 = false;
				if (jinkins_condition__defaultCondition (player, npc))
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
						responses[responseIndex++] = new string_id (c_stringFile, "s_127");
					}
					
					if (hasResponse1)
					{
						responses[responseIndex++] = new string_id (c_stringFile, "s_132");
					}
					
					utils.setScriptVar (player, "conversation.jinkins.branchId", 21);
					
					npcSpeak (player, message);
					npcSetConversationResponses (player, responses);
				}
				else
				{
					utils.removeScriptVar (player, "conversation.jinkins.branchId");
					
					npcEndConversationWithMessage (player, message);
				}
				
				return SCRIPT_CONTINUE;
			}
			
		}
		
		return SCRIPT_DEFAULT;
	}
	
	
	public int jinkins_handleBranch21(obj_id player, obj_id npc, string_id response) throws InterruptedException
	{
		
		if (response.equals("s_127"))
		{
			
			if (jinkins_condition__defaultCondition (player, npc))
			{
				
				string_id message = new string_id (c_stringFile, "s_129");
				int numberOfResponses = 0;
				
				boolean hasResponse = false;
				
				boolean hasResponse0 = false;
				if (jinkins_condition__defaultCondition (player, npc))
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
						responses[responseIndex++] = new string_id (c_stringFile, "s_133");
					}
					
					utils.setScriptVar (player, "conversation.jinkins.branchId", 22);
					
					npcSpeak (player, message);
					npcSetConversationResponses (player, responses);
				}
				else
				{
					utils.removeScriptVar (player, "conversation.jinkins.branchId");
					
					npcEndConversationWithMessage (player, message);
				}
				
				return SCRIPT_CONTINUE;
			}
			
		}
		
		if (response.equals("s_132"))
		{
			
			if (jinkins_condition__defaultCondition (player, npc))
			{
				jinkins_action_giveMooreQuest (player, npc);
				
				string_id message = new string_id (c_stringFile, "s_134");
				utils.removeScriptVar (player, "conversation.jinkins.branchId");
				
				npcEndConversationWithMessage (player, message);
				
				return SCRIPT_CONTINUE;
			}
			
		}
		
		return SCRIPT_DEFAULT;
	}
	
	
	public int jinkins_handleBranch22(obj_id player, obj_id npc, string_id response) throws InterruptedException
	{
		
		if (response.equals("s_133"))
		{
			
			if (jinkins_condition__defaultCondition (player, npc))
			{
				jinkins_action_giveMooreQuest (player, npc);
				
				string_id message = new string_id (c_stringFile, "s_134");
				utils.removeScriptVar (player, "conversation.jinkins.branchId");
				
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
			detachScript(self, "conversation.jinkins");
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
		detachScript (self, "conversation.jinkins");
		
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
		
		if (jinkins_condition_hasCompletedThemePark (player, npc))
		{
			
			string_id message = new string_id (c_stringFile, "s_52");
			prose_package pp = new prose_package ();
			pp.stringId = message;
			pp.actor.set (player);
			pp.target.set (npc);
			
			chat.chat (npc, player, null, null, pp);
			return SCRIPT_CONTINUE;
		}
		
		if (jinkins_condition_allQuestsComplete (player, npc))
		{
			
			string_id message = new string_id (c_stringFile, "s_48");
			prose_package pp = new prose_package ();
			pp.stringId = message;
			pp.actor.set (player);
			pp.target.set (npc);
			
			chat.chat (npc, player, null, null, pp);
			return SCRIPT_CONTINUE;
		}
		
		if (jinkins_condition_isDoneMineQuest (player, npc))
		{
			jinkins_action_gotoKole (player, npc);
			
			string_id message = new string_id (c_stringFile, "s_50");
			prose_package pp = new prose_package ();
			pp.stringId = message;
			pp.actor.set (player);
			pp.target.set (npc);
			
			chat.chat (npc, player, null, null, pp);
			return SCRIPT_CONTINUE;
		}
		
		if (jinkins_condition_hasMineFilter (player, npc))
		{
			doAnimationAction (npc, "nod_head_multiple");
			
			string_id message = new string_id (c_stringFile, "s_38");
			int numberOfResponses = 0;
			
			boolean hasResponse = false;
			
			boolean hasResponse0 = false;
			if (jinkins_condition__defaultCondition (player, npc))
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
				
				utils.setScriptVar (player, "conversation.jinkins.branchId", 4);
				
				npcStartConversation (player, npc, "jinkins", message, responses);
			}
			else
			{
				chat.chat (npc, player, message);
			}
			
			return SCRIPT_CONTINUE;
		}
		
		if (jinkins_condition_hasMineQuestIncomplete (player, npc))
		{
			
			string_id message = new string_id (c_stringFile, "s_146");
			int numberOfResponses = 0;
			
			boolean hasResponse = false;
			
			boolean hasResponse0 = false;
			if (jinkins_condition__defaultCondition (player, npc))
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
					responses[responseIndex++] = new string_id (c_stringFile, "s_147");
				}
				
				utils.setScriptVar (player, "conversation.jinkins.branchId", 6);
				
				npcStartConversation (player, npc, "jinkins", message, responses);
			}
			else
			{
				chat.chat (npc, player, message);
			}
			
			return SCRIPT_CONTINUE;
		}
		
		if (jinkins_condition_hasReturnedFromMoore (player, npc))
		{
			doAnimationAction (npc, "adjust");
			
			jinkins_action_completeMooreQuest (player, npc);
			
			string_id message = new string_id (c_stringFile, "s_135");
			int numberOfResponses = 0;
			
			boolean hasResponse = false;
			
			boolean hasResponse0 = false;
			if (jinkins_condition__defaultCondition (player, npc))
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
					responses[responseIndex++] = new string_id (c_stringFile, "s_136");
				}
				
				utils.setScriptVar (player, "conversation.jinkins.branchId", 8);
				
				npcStartConversation (player, npc, "jinkins", message, responses);
			}
			else
			{
				chat.chat (npc, player, message);
			}
			
			return SCRIPT_CONTINUE;
		}
		
		if (jinkins_condition_hasCompletedPirate (player, npc))
		{
			doAnimationAction (npc, "clap_rousing");
			
			jinkins_action_hasFoundJinkins (player, npc);
			
			string_id message = new string_id (c_stringFile, "s_99");
			int numberOfResponses = 0;
			
			boolean hasResponse = false;
			
			boolean hasResponse0 = false;
			if (jinkins_condition__defaultCondition (player, npc))
			{
				++numberOfResponses;
				hasResponse = true;
				hasResponse0 = true;
			}
			
			boolean hasResponse1 = false;
			if (jinkins_condition__defaultCondition (player, npc))
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
					responses[responseIndex++] = new string_id (c_stringFile, "s_101");
				}
				
				if (hasResponse1)
				{
					responses[responseIndex++] = new string_id (c_stringFile, "s_103");
				}
				
				utils.setScriptVar (player, "conversation.jinkins.branchId", 15);
				
				prose_package pp = new prose_package ();
				pp.stringId = message;
				pp.actor.set (player);
				pp.target.set (npc);
				
				npcStartConversation (player, npc, "jinkins", null, pp, responses);
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
		
		if (jinkins_condition_hasNotCompletedPirate (player, npc))
		{
			doAnimationAction (npc, "shoo");
			
			string_id message = new string_id (c_stringFile, "s_105");
			chat.chat (npc, player, message);
			
			return SCRIPT_CONTINUE;
		}
		
		if (jinkins_condition__defaultCondition (player, npc))
		{
			doAnimationAction (npc, "tap_foot");
			
			string_id message = new string_id (c_stringFile, "s_100");
			chat.chat (npc, player, message);
			
			return SCRIPT_CONTINUE;
		}
		
		chat.chat (npc, "Error: All conditions for OnStartNpcConversation were false.");
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int OnNpcConversationResponse(obj_id self, String conversationId, obj_id player, string_id response) throws InterruptedException
	{
		if (!conversationId.equals("jinkins"))
		{
			return SCRIPT_CONTINUE;
		}
		
		obj_id npc = self;
		
		int branchId = utils.getIntScriptVar (player, "conversation.jinkins.branchId");
		
		if (branchId == 4 && jinkins_handleBranch4 (player, npc, response) == SCRIPT_CONTINUE)
		{
			return SCRIPT_CONTINUE;
		}
		
		if (branchId == 6 && jinkins_handleBranch6 (player, npc, response) == SCRIPT_CONTINUE)
		{
			return SCRIPT_CONTINUE;
		}
		
		if (branchId == 8 && jinkins_handleBranch8 (player, npc, response) == SCRIPT_CONTINUE)
		{
			return SCRIPT_CONTINUE;
		}
		
		if (branchId == 9 && jinkins_handleBranch9 (player, npc, response) == SCRIPT_CONTINUE)
		{
			return SCRIPT_CONTINUE;
		}
		
		if (branchId == 10 && jinkins_handleBranch10 (player, npc, response) == SCRIPT_CONTINUE)
		{
			return SCRIPT_CONTINUE;
		}
		
		if (branchId == 11 && jinkins_handleBranch11 (player, npc, response) == SCRIPT_CONTINUE)
		{
			return SCRIPT_CONTINUE;
		}
		
		if (branchId == 12 && jinkins_handleBranch12 (player, npc, response) == SCRIPT_CONTINUE)
		{
			return SCRIPT_CONTINUE;
		}
		
		if (branchId == 13 && jinkins_handleBranch13 (player, npc, response) == SCRIPT_CONTINUE)
		{
			return SCRIPT_CONTINUE;
		}
		
		if (branchId == 15 && jinkins_handleBranch15 (player, npc, response) == SCRIPT_CONTINUE)
		{
			return SCRIPT_CONTINUE;
		}
		
		if (branchId == 16 && jinkins_handleBranch16 (player, npc, response) == SCRIPT_CONTINUE)
		{
			return SCRIPT_CONTINUE;
		}
		
		if (branchId == 17 && jinkins_handleBranch17 (player, npc, response) == SCRIPT_CONTINUE)
		{
			return SCRIPT_CONTINUE;
		}
		
		if (branchId == 18 && jinkins_handleBranch18 (player, npc, response) == SCRIPT_CONTINUE)
		{
			return SCRIPT_CONTINUE;
		}
		
		if (branchId == 19 && jinkins_handleBranch19 (player, npc, response) == SCRIPT_CONTINUE)
		{
			return SCRIPT_CONTINUE;
		}
		
		if (branchId == 20 && jinkins_handleBranch20 (player, npc, response) == SCRIPT_CONTINUE)
		{
			return SCRIPT_CONTINUE;
		}
		
		if (branchId == 21 && jinkins_handleBranch21 (player, npc, response) == SCRIPT_CONTINUE)
		{
			return SCRIPT_CONTINUE;
		}
		
		if (branchId == 22 && jinkins_handleBranch22 (player, npc, response) == SCRIPT_CONTINUE)
		{
			return SCRIPT_CONTINUE;
		}
		
		chat.chat (npc, "Error: Fell through all branches and responses for OnNpcConversationResponse.");
		
		utils.removeScriptVar (player, "conversation.jinkins.branchId");
		
		return SCRIPT_CONTINUE;
	}
	
}
