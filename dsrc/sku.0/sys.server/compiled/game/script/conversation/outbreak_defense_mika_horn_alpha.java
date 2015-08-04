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
import script.library.features;
import script.library.groundquests;
import script.library.space_quest;
import script.library.utils;


public class outbreak_defense_mika_horn_alpha extends script.base_script
{
	public outbreak_defense_mika_horn_alpha()
	{
	}
	String c_stringFile = "conversation/outbreak_defense_mika_horn_alpha";
	
	
	public boolean outbreak_defense_mika_horn_alpha_condition__defaultCondition(obj_id player, obj_id npc) throws InterruptedException
	{
		return true;
	}
	
	
	public boolean outbreak_defense_mika_horn_alpha_condition_hasCompletedDefenseQuest(obj_id player, obj_id npc) throws InterruptedException
	{
		return (groundquests.hasCompletedQuest(player, "camp_alpha_defense") || groundquests.hasCompletedQuest(player, "camp_alpha_defense_02") || groundquests.hasCompletedQuest(player, "camp_alpha_defense_03") || groundquests.hasCompletedQuest(player, "camp_alpha_defense_04"));
	}
	
	
	public boolean outbreak_defense_mika_horn_alpha_condition_isWaitingForRewardSignal(obj_id player, obj_id npc) throws InterruptedException
	{
		return groundquests.isTaskActive(player, "camp_alpha_defense", "collectRewardAlpha") || groundquests.isTaskActive(player, "camp_alpha_defense_02", "collectRewardAlpha") || groundquests.isTaskActive(player, "camp_alpha_defense_03", "collectRewardAlpha") || groundquests.isTaskActive(player, "camp_alpha_defense_04", "collectRewardAlpha");
	}
	
	
	public boolean outbreak_defense_mika_horn_alpha_condition_hasNeverSpoken(obj_id player, obj_id npc) throws InterruptedException
	{
		
		return (hasCompletedCollectionSlot(player, "exp_icon_deathtroopers_camp_alpha") && !groundquests.isQuestActiveOrComplete(player, "camp_alpha_defense") && !groundquests.isQuestActiveOrComplete(player, "camp_alpha_defense_02") && !groundquests.isQuestActiveOrComplete(player, "camp_alpha_defense_03") && !groundquests.isQuestActiveOrComplete(player, "camp_alpha_defense_04") && outbreak_defense_mika_horn_alpha_condition_hasAvailableGuardPosts(player,npc));
		
	}
	
	
	public boolean outbreak_defense_mika_horn_alpha_condition_isGodPlayerTester(obj_id player, obj_id npc) throws InterruptedException
	{
		return isGod(player);
	}
	
	
	public boolean outbreak_defense_mika_horn_alpha_condition_levelTooHigh(obj_id player, obj_id npc) throws InterruptedException
	{
		return getLevel(player) >= 15;
	}
	
	
	public boolean outbreak_defense_mika_horn_alpha_condition_hasDefenseQuest(obj_id player, obj_id npc) throws InterruptedException
	{
		return (groundquests.isQuestActive(player, "camp_alpha_defense") || groundquests.isQuestActive(player, "camp_alpha_defense_02") || groundquests.isQuestActive(player, "camp_alpha_defense_03") || groundquests.isQuestActive(player, "camp_alpha_defense_04"));
	}
	
	
	public boolean outbreak_defense_mika_horn_alpha_condition_noAntiVirus(obj_id player, obj_id npc) throws InterruptedException
	{
		return (!groundquests.hasCompletedQuest(player, "outbreak_quest_01_imperial") && !groundquests.hasCompletedQuest(player, "outbreak_quest_01_rebel") && !groundquests.hasCompletedQuest(player, "outbreak_quest_01_neutral"));
	}
	
	
	public boolean outbreak_defense_mika_horn_alpha_condition_hasAvailableGuardPosts(obj_id player, obj_id npc) throws InterruptedException
	{
		
		if (!utils.hasScriptVar(npc, "guardPostList"))
		{
			CustomerServiceLog("outbreak_themepark", "camp_defense.hasAvailableGuardPosts() the npc, "+npc+" has no guardPostList script var.");
			return false;
		}
		obj_id[] guardPostList = utils.getObjIdArrayScriptVar(npc, "guardPostList");
		if (guardPostList == null || guardPostList.length <= 0)
		{
			CustomerServiceLog("outbreak_themepark", "camp_defense.hasAvailableGuardPosts() the npc, "+npc+" has an invalid guardPostList script var.");
			return false;
		}
		
		for (int i = 0; i < guardPostList.length; i++)
		{
			testAbortScript();
			if (!isValidId(guardPostList[i]) || !exists(guardPostList[i]))
			{
				continue;
			}
			if (!utils.hasScriptVar(guardPostList[i], "occupied"))
			{
				return true;
			}
		}
		
		return false;
	}
	
	
	public void outbreak_defense_mika_horn_alpha_action_giveQuest(obj_id player, obj_id npc) throws InterruptedException
	{
		if (groundquests.hasCompletedQuest(player, "camp_alpha_defense"))
		{
			groundquests.clearQuest(player, "camp_alpha_defense");
		}
		if (groundquests.hasCompletedQuest(player, "camp_alpha_defense_02"))
		{
			groundquests.clearQuest(player, "camp_alpha_defense_02");
		}
		if (groundquests.hasCompletedQuest(player, "camp_alpha_defense_03"))
		{
			groundquests.clearQuest(player, "camp_alpha_defense_03");
		}
		if (groundquests.hasCompletedQuest(player, "camp_alpha_defense_04"))
		{
			groundquests.clearQuest(player, "camp_alpha_defense_04");
		}
		
		if (!utils.hasScriptVar(npc, "guardPostList"))
		{
			CustomerServiceLog("outbreak_themepark", "camp_defense.giveQuest() the npc, "+npc+" has no guardPostList script var.");
		}
		else
		{
			obj_id[] guardPostList = utils.getObjIdArrayScriptVar(npc, "guardPostList");
			if (guardPostList == null || guardPostList.length <= 0)
			{
				CustomerServiceLog("outbreak_themepark", "camp_defense.giveQuest() the npc, "+npc+" has an invalid guardPostList script var.");
			}
			else
			{
				
				for (int i = 0; i < guardPostList.length; i++)
				{
					testAbortScript();
					if (!isValidId(guardPostList[i]) || !exists(guardPostList[i]))
					{
						continue;
					}
					if (!utils.hasScriptVar(guardPostList[i], "occupied"))
					{
						String questName = getStringObjVar(guardPostList[i], "questObject");
						if (questName == null || questName.length() <=0)
						{
							CustomerServiceLog("outbreak_themepark", "camp_defense.giveQuest() the npc, "+npc+" has an invalid guardPostList script var.");
						}
						else
						{
							CustomerServiceLog("outbreak_themepark", "camp_defense.giveQuest() the npc, "+npc+" is granting the player: "+player+" has an invalid guardPostList script var.");
							groundquests.grantQuest(player, questName);
						}
					}
				}
			}
		}
	}
	
	
	public void outbreak_defense_mika_horn_alpha_action_sendQuestSignal(obj_id player, obj_id npc) throws InterruptedException
	{
		groundquests.sendSignal(player, "hasCompletedAlpha");
		
	}
	
	
	public void outbreak_defense_mika_horn_alpha_action_action0001(obj_id player, obj_id npc) throws InterruptedException
	{
		groundquests.grantQuest(player, "camp_alpha_defense");
	}
	
	
	public void outbreak_defense_mika_horn_alpha_action_action0002(obj_id player, obj_id npc) throws InterruptedException
	{
		groundquests.grantQuest(player, "camp_alpha_defense_02");
	}
	
	
	public void outbreak_defense_mika_horn_alpha_action_action0003(obj_id player, obj_id npc) throws InterruptedException
	{
		groundquests.grantQuest(player, "camp_alpha_defense_03");
	}
	
	
	public void outbreak_defense_mika_horn_alpha_action_action0004(obj_id player, obj_id npc) throws InterruptedException
	{
		groundquests.grantQuest(player, "camp_alpha_defense_04");
	}
	
	
	public int outbreak_defense_mika_horn_alpha_handleBranch3(obj_id player, obj_id npc, string_id response) throws InterruptedException
	{
		
		if (response.equals("s_22"))
		{
			
			if (outbreak_defense_mika_horn_alpha_condition__defaultCondition (player, npc))
			{
				outbreak_defense_mika_horn_alpha_action_action0001 (player, npc);
				
				string_id message = new string_id (c_stringFile, "s_28");
				utils.removeScriptVar (player, "conversation.outbreak_defense_mika_horn_alpha.branchId");
				
				npcEndConversationWithMessage (player, message);
				
				return SCRIPT_CONTINUE;
			}
			
		}
		
		if (response.equals("s_23"))
		{
			
			if (outbreak_defense_mika_horn_alpha_condition__defaultCondition (player, npc))
			{
				outbreak_defense_mika_horn_alpha_action_action0002 (player, npc);
				
				string_id message = new string_id (c_stringFile, "s_29");
				utils.removeScriptVar (player, "conversation.outbreak_defense_mika_horn_alpha.branchId");
				
				npcEndConversationWithMessage (player, message);
				
				return SCRIPT_CONTINUE;
			}
			
		}
		
		if (response.equals("s_26"))
		{
			
			if (outbreak_defense_mika_horn_alpha_condition__defaultCondition (player, npc))
			{
				outbreak_defense_mika_horn_alpha_action_action0003 (player, npc);
				
				string_id message = new string_id (c_stringFile, "s_30");
				utils.removeScriptVar (player, "conversation.outbreak_defense_mika_horn_alpha.branchId");
				
				npcEndConversationWithMessage (player, message);
				
				return SCRIPT_CONTINUE;
			}
			
		}
		
		if (response.equals("s_27"))
		{
			
			if (outbreak_defense_mika_horn_alpha_condition__defaultCondition (player, npc))
			{
				outbreak_defense_mika_horn_alpha_action_action0004 (player, npc);
				
				string_id message = new string_id (c_stringFile, "s_31");
				utils.removeScriptVar (player, "conversation.outbreak_defense_mika_horn_alpha.branchId");
				
				npcEndConversationWithMessage (player, message);
				
				return SCRIPT_CONTINUE;
			}
			
		}
		
		return SCRIPT_DEFAULT;
	}
	
	
	public int outbreak_defense_mika_horn_alpha_handleBranch8(obj_id player, obj_id npc, string_id response) throws InterruptedException
	{
		
		if (response.equals("s_62"))
		{
			
			if (outbreak_defense_mika_horn_alpha_condition__defaultCondition (player, npc))
			{
				
				string_id message = new string_id (c_stringFile, "s_63");
				int numberOfResponses = 0;
				
				boolean hasResponse = false;
				
				boolean hasResponse0 = false;
				if (outbreak_defense_mika_horn_alpha_condition__defaultCondition (player, npc))
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
					
					utils.setScriptVar (player, "conversation.outbreak_defense_mika_horn_alpha.branchId", 9);
					
					npcSpeak (player, message);
					npcSetConversationResponses (player, responses);
				}
				else
				{
					utils.removeScriptVar (player, "conversation.outbreak_defense_mika_horn_alpha.branchId");
					
					npcEndConversationWithMessage (player, message);
				}
				
				return SCRIPT_CONTINUE;
			}
			
		}
		
		return SCRIPT_DEFAULT;
	}
	
	
	public int outbreak_defense_mika_horn_alpha_handleBranch9(obj_id player, obj_id npc, string_id response) throws InterruptedException
	{
		
		if (response.equals("s_24"))
		{
			
			if (outbreak_defense_mika_horn_alpha_condition__defaultCondition (player, npc))
			{
				outbreak_defense_mika_horn_alpha_action_giveQuest (player, npc);
				
				string_id message = new string_id (c_stringFile, "s_32");
				utils.removeScriptVar (player, "conversation.outbreak_defense_mika_horn_alpha.branchId");
				
				npcEndConversationWithMessage (player, message);
				
				return SCRIPT_CONTINUE;
			}
			
		}
		
		return SCRIPT_DEFAULT;
	}
	
	
	public int outbreak_defense_mika_horn_alpha_handleBranch12(obj_id player, obj_id npc, string_id response) throws InterruptedException
	{
		
		if (response.equals("s_37"))
		{
			
			if (outbreak_defense_mika_horn_alpha_condition__defaultCondition (player, npc))
			{
				
				string_id message = new string_id (c_stringFile, "s_39");
				int numberOfResponses = 0;
				
				boolean hasResponse = false;
				
				boolean hasResponse0 = false;
				if (outbreak_defense_mika_horn_alpha_condition__defaultCondition (player, npc))
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
						responses[responseIndex++] = new string_id (c_stringFile, "s_41");
					}
					
					utils.setScriptVar (player, "conversation.outbreak_defense_mika_horn_alpha.branchId", 13);
					
					npcSpeak (player, message);
					npcSetConversationResponses (player, responses);
				}
				else
				{
					utils.removeScriptVar (player, "conversation.outbreak_defense_mika_horn_alpha.branchId");
					
					npcEndConversationWithMessage (player, message);
				}
				
				return SCRIPT_CONTINUE;
			}
			
		}
		
		return SCRIPT_DEFAULT;
	}
	
	
	public int outbreak_defense_mika_horn_alpha_handleBranch13(obj_id player, obj_id npc, string_id response) throws InterruptedException
	{
		
		if (response.equals("s_41"))
		{
			
			if (outbreak_defense_mika_horn_alpha_condition__defaultCondition (player, npc))
			{
				
				string_id message = new string_id (c_stringFile, "s_43");
				int numberOfResponses = 0;
				
				boolean hasResponse = false;
				
				boolean hasResponse0 = false;
				if (outbreak_defense_mika_horn_alpha_condition__defaultCondition (player, npc))
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
					
					utils.setScriptVar (player, "conversation.outbreak_defense_mika_horn_alpha.branchId", 14);
					
					npcSpeak (player, message);
					npcSetConversationResponses (player, responses);
				}
				else
				{
					utils.removeScriptVar (player, "conversation.outbreak_defense_mika_horn_alpha.branchId");
					
					npcEndConversationWithMessage (player, message);
				}
				
				return SCRIPT_CONTINUE;
			}
			
		}
		
		return SCRIPT_DEFAULT;
	}
	
	
	public int outbreak_defense_mika_horn_alpha_handleBranch14(obj_id player, obj_id npc, string_id response) throws InterruptedException
	{
		
		if (response.equals("s_49"))
		{
			
			if (outbreak_defense_mika_horn_alpha_condition__defaultCondition (player, npc))
			{
				outbreak_defense_mika_horn_alpha_action_giveQuest (player, npc);
				
				string_id message = new string_id (c_stringFile, "s_77");
				utils.removeScriptVar (player, "conversation.outbreak_defense_mika_horn_alpha.branchId");
				
				npcEndConversationWithMessage (player, message);
				
				return SCRIPT_CONTINUE;
			}
			
		}
		
		return SCRIPT_DEFAULT;
	}
	
	
	public int OnInitialize(obj_id self) throws InterruptedException
	{
		setCondition (self, CONDITION_CONVERSABLE);
		setCondition (self, CONDITION_INTERESTING);
		messageTo(self, "findGuardPosts", null, 5, false);
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int OnAttach(obj_id self) throws InterruptedException
	{
		setCondition (self, CONDITION_CONVERSABLE);
		setCondition (self, CONDITION_INTERESTING);
		messageTo(self, "findGuardPosts", null, 5, false);
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
	
	
	public int findGuardPosts(obj_id self, dictionary params) throws InterruptedException
	{
		obj_id[] guardPostList = getAllObjectsWithObjVar(getLocation(self), 200, "questObject");
		if (guardPostList == null || guardPostList.length <= 0)
		{
			CustomerServiceLog("outbreak_themepark", "camp_defense.findGuardPosts() the npc, "+self+" failed to find any valid guard post objects within 100 of "+getLocation(self));
			return SCRIPT_CONTINUE;
		}
		
		Vector guardPostsFound = null;
		for (int i = 0; i < guardPostList.length; i++)
		{
			testAbortScript();
			String spawnerQuestNameMatch = getStringObjVar(guardPostList[i], "questObject");
			if (spawnerQuestNameMatch == null || spawnerQuestNameMatch.length() <= 0)
			{
				CustomerServiceLog("outbreak_themepark", "camp_defense.findGuardPosts() the npc, "+self+" found a guard post object that did not have a quest string. Guard Post: "+guardPostList[i]);
				continue;
			}
			if (!spawnerQuestNameMatch.startsWith("camp_alpha_defense"))
			{
				continue;
			}
			
			CustomerServiceLog("outbreak_themepark", "camp_defense.findGuardPosts() the npc, "+self+" found a guard post object. Adding the object to the list. Guard Post: "+guardPostList[i]);
			utils.addElement(guardPostsFound, guardPostList[i]);
		}
		
		CustomerServiceLog("outbreak_themepark", "camp_defense.findGuardPosts() the npc, "+self+" loop for all the available guard posts has finished. Length of list: "+guardPostList.length);
		if (guardPostList.length <= 0)
		{
			CustomerServiceLog("outbreak_themepark", "camp_defense.findGuardPosts() the npc, "+self+" had a problem finding guard posts.");
			return SCRIPT_CONTINUE;
		}
		if (guardPostList.length != 4)
		{
			CustomerServiceLog("outbreak_themepark", "camp_defense.findGuardPosts() the npc, "+self+" found less or greater than 4 guard posts!");
		}
		
		utils.setScriptVar(self, "guardPostList", guardPostList);
		CustomerServiceLog("outbreak_themepark", "camp_defense.findGuardPosts() the npc, "+self+" has a list of guard posts as scriptvar.");
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
		
		if (outbreak_defense_mika_horn_alpha_condition_noAntiVirus (player, npc))
		{
			doAnimationAction (npc, "wave_on_dismissing");
			
			string_id message = new string_id (c_stringFile, "s_69");
			chat.chat (npc, player, message);
			
			return SCRIPT_CONTINUE;
		}
		
		if (outbreak_defense_mika_horn_alpha_condition_isWaitingForRewardSignal (player, npc))
		{
			outbreak_defense_mika_horn_alpha_action_sendQuestSignal (player, npc);
			
			string_id message = new string_id (c_stringFile, "s_64");
			chat.chat (npc, player, message);
			
			return SCRIPT_CONTINUE;
		}
		
		if (outbreak_defense_mika_horn_alpha_condition_isGodPlayerTester (player, npc))
		{
			
			string_id message = new string_id (c_stringFile, "s_21");
			int numberOfResponses = 0;
			
			boolean hasResponse = false;
			
			boolean hasResponse0 = false;
			if (outbreak_defense_mika_horn_alpha_condition__defaultCondition (player, npc))
			{
				++numberOfResponses;
				hasResponse = true;
				hasResponse0 = true;
			}
			
			boolean hasResponse1 = false;
			if (outbreak_defense_mika_horn_alpha_condition__defaultCondition (player, npc))
			{
				++numberOfResponses;
				hasResponse = true;
				hasResponse1 = true;
			}
			
			boolean hasResponse2 = false;
			if (outbreak_defense_mika_horn_alpha_condition__defaultCondition (player, npc))
			{
				++numberOfResponses;
				hasResponse = true;
				hasResponse2 = true;
			}
			
			boolean hasResponse3 = false;
			if (outbreak_defense_mika_horn_alpha_condition__defaultCondition (player, npc))
			{
				++numberOfResponses;
				hasResponse = true;
				hasResponse3 = true;
			}
			
			if (hasResponse)
			{
				int responseIndex = 0;
				string_id responses[] = new string_id[numberOfResponses];
				
				if (hasResponse0)
				{
					responses[responseIndex++] = new string_id (c_stringFile, "s_22");
				}
				
				if (hasResponse1)
				{
					responses[responseIndex++] = new string_id (c_stringFile, "s_23");
				}
				
				if (hasResponse2)
				{
					responses[responseIndex++] = new string_id (c_stringFile, "s_26");
				}
				
				if (hasResponse3)
				{
					responses[responseIndex++] = new string_id (c_stringFile, "s_27");
				}
				
				utils.setScriptVar (player, "conversation.outbreak_defense_mika_horn_alpha.branchId", 3);
				
				npcStartConversation (player, npc, "outbreak_defense_mika_horn_alpha", message, responses);
			}
			else
			{
				chat.chat (npc, player, message);
			}
			
			return SCRIPT_CONTINUE;
		}
		
		if (outbreak_defense_mika_horn_alpha_condition_hasCompletedDefenseQuest (player, npc))
		{
			
			string_id message = new string_id (c_stringFile, "s_55");
			int numberOfResponses = 0;
			
			boolean hasResponse = false;
			
			boolean hasResponse0 = false;
			if (outbreak_defense_mika_horn_alpha_condition__defaultCondition (player, npc))
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
					responses[responseIndex++] = new string_id (c_stringFile, "s_62");
				}
				
				utils.setScriptVar (player, "conversation.outbreak_defense_mika_horn_alpha.branchId", 8);
				
				npcStartConversation (player, npc, "outbreak_defense_mika_horn_alpha", message, responses);
			}
			else
			{
				chat.chat (npc, player, message);
			}
			
			return SCRIPT_CONTINUE;
		}
		
		if (outbreak_defense_mika_horn_alpha_condition_hasDefenseQuest (player, npc))
		{
			
			string_id message = new string_id (c_stringFile, "s_47");
			chat.chat (npc, player, message);
			
			return SCRIPT_CONTINUE;
		}
		
		if (outbreak_defense_mika_horn_alpha_condition_hasNeverSpoken (player, npc))
		{
			doAnimationAction (npc, "beckon");
			
			string_id message = new string_id (c_stringFile, "s_35");
			int numberOfResponses = 0;
			
			boolean hasResponse = false;
			
			boolean hasResponse0 = false;
			if (outbreak_defense_mika_horn_alpha_condition__defaultCondition (player, npc))
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
					responses[responseIndex++] = new string_id (c_stringFile, "s_37");
				}
				
				utils.setScriptVar (player, "conversation.outbreak_defense_mika_horn_alpha.branchId", 12);
				
				npcStartConversation (player, npc, "outbreak_defense_mika_horn_alpha", message, responses);
			}
			else
			{
				chat.chat (npc, player, message);
			}
			
			return SCRIPT_CONTINUE;
		}
		
		if (!outbreak_defense_mika_horn_alpha_condition_hasAvailableGuardPosts (player, npc))
		{
			
			string_id message = new string_id (c_stringFile, "s_44");
			chat.chat (npc, player, message);
			
			return SCRIPT_CONTINUE;
		}
		
		if (outbreak_defense_mika_horn_alpha_condition__defaultCondition (player, npc))
		{
			
			string_id message = new string_id (c_stringFile, "s_90");
			chat.chat (npc, player, message);
			
			return SCRIPT_CONTINUE;
		}
		
		chat.chat (npc, "Error: All conditions for OnStartNpcConversation were false.");
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int OnNpcConversationResponse(obj_id self, String conversationId, obj_id player, string_id response) throws InterruptedException
	{
		if (!conversationId.equals("outbreak_defense_mika_horn_alpha"))
		{
			return SCRIPT_CONTINUE;
		}
		
		obj_id npc = self;
		
		int branchId = utils.getIntScriptVar (player, "conversation.outbreak_defense_mika_horn_alpha.branchId");
		
		if (branchId == 3 && outbreak_defense_mika_horn_alpha_handleBranch3 (player, npc, response) == SCRIPT_CONTINUE)
		{
			return SCRIPT_CONTINUE;
		}
		
		if (branchId == 8 && outbreak_defense_mika_horn_alpha_handleBranch8 (player, npc, response) == SCRIPT_CONTINUE)
		{
			return SCRIPT_CONTINUE;
		}
		
		if (branchId == 9 && outbreak_defense_mika_horn_alpha_handleBranch9 (player, npc, response) == SCRIPT_CONTINUE)
		{
			return SCRIPT_CONTINUE;
		}
		
		if (branchId == 12 && outbreak_defense_mika_horn_alpha_handleBranch12 (player, npc, response) == SCRIPT_CONTINUE)
		{
			return SCRIPT_CONTINUE;
		}
		
		if (branchId == 13 && outbreak_defense_mika_horn_alpha_handleBranch13 (player, npc, response) == SCRIPT_CONTINUE)
		{
			return SCRIPT_CONTINUE;
		}
		
		if (branchId == 14 && outbreak_defense_mika_horn_alpha_handleBranch14 (player, npc, response) == SCRIPT_CONTINUE)
		{
			return SCRIPT_CONTINUE;
		}
		
		chat.chat (npc, "Error: Fell through all branches and responses for OnNpcConversationResponse.");
		
		utils.removeScriptVar (player, "conversation.outbreak_defense_mika_horn_alpha.branchId");
		
		return SCRIPT_CONTINUE;
	}
	
}
