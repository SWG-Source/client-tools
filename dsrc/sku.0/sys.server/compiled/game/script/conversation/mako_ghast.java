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


public class mako_ghast extends script.base_script
{
	public mako_ghast()
	{
	}
	String c_stringFile = "conversation/mako_ghast";
	
	
	public boolean mako_ghast_condition__defaultCondition(obj_id player, obj_id npc) throws InterruptedException
	{
		return true;
	}
	
	
	public boolean mako_ghast_condition_canDoEliteHideout(obj_id player, obj_id npc) throws InterruptedException
	{
		faceTo(npc, player);
		return !groundquests.isQuestActiveOrComplete(player, "u16_nym_themepark_elite_kill_pirate_cave_collection") && groundquests.isQuestActiveOrComplete(player, "u16_nym_themepark_pirate_hideout");
	}
	
	
	public boolean mako_ghast_condition_hasEliteHideoutNotComplete(obj_id player, obj_id npc) throws InterruptedException
	{
		faceTo(npc, player);
		return groundquests.isQuestActive(player, "u16_nym_themepark_elite_kill_pirate_cave_collection");
	}
	
	
	public boolean mako_ghast_condition_hasReturnedEliteHideout(obj_id player, obj_id npc) throws InterruptedException
	{
		faceTo(npc, player);
		return groundquests.isTaskActive(player, "u16_nym_themepark_elite_kill_pirate_cave_collection", "returnKillCaveTrollComplete");
	}
	
	
	public boolean mako_ghast_condition_canDoEliteCave(obj_id player, obj_id npc) throws InterruptedException
	{
		faceTo(npc, player);
		return !groundquests.isQuestActiveOrComplete(player, "u16_nym_themepark_elite_kill_miner_cave_collection") && groundquests.hasCompletedQuest(player, "u16_nym_themepark_elite_kill_pirate_cave_collection") && groundquests.isQuestActiveOrComplete(player, "u16_nym_themepark_mine");
	}
	
	
	public boolean mako_ghast_condition_hasEliteCaveNotComplete(obj_id player, obj_id npc) throws InterruptedException
	{
		faceTo(npc, player);
		return groundquests.isQuestActive(player, "u16_nym_themepark_elite_kill_miner_cave_collection");
	}
	
	
	public boolean mako_ghast_condition_hasReturnedEliteCave(obj_id player, obj_id npc) throws InterruptedException
	{
		faceTo(npc, player);
		return groundquests.isTaskActive(player, "u16_nym_themepark_elite_kill_miner_cave_collection", "returnMineElitesComplete");
	}
	
	
	public boolean mako_ghast_condition_canDoEliteLab(obj_id player, obj_id npc) throws InterruptedException
	{
		faceTo(npc, player);
		return !groundquests.isQuestActiveOrComplete(player, "u16_nym_themepark_elite_kill_imperial_facility_collection") && groundquests.hasCompletedQuest(player, "u16_nym_themepark_elite_kill_miner_cave_collection") && groundquests.isQuestActiveOrComplete(player, "u16_nym_themepark_research_facility");
	}
	
	
	public boolean mako_ghast_condition_hasEliteLabNotComplete(obj_id player, obj_id npc) throws InterruptedException
	{
		faceTo(npc, player);
		return groundquests.isQuestActive(player, "u16_nym_themepark_elite_kill_imperial_facility_collection");
	}
	
	
	public boolean mako_ghast_condition_hasReturnedEliteLab(obj_id player, obj_id npc) throws InterruptedException
	{
		faceTo(npc, player);
		return groundquests.isTaskActive(player, "u16_nym_themepark_elite_kill_imperial_facility_collection", "returnResearchFacilityElitesComplete");
	}
	
	
	public boolean mako_ghast_condition_hasntDoneLab(obj_id player, obj_id npc) throws InterruptedException
	{
		faceTo(npc, player);
		return !groundquests.isQuestActiveOrComplete(player, "u16_nym_themepark_research_facility") && groundquests.hasCompletedQuest(player, "u16_nym_themepark_mine") && groundquests.hasCompletedQuest(player, "u16_nym_themepark_elite_kill_miner_cave_collection");
	}
	
	
	public boolean mako_ghast_condition_hasntDoneCave(obj_id player, obj_id npc) throws InterruptedException
	{
		faceTo(npc, player);
		return !groundquests.isQuestActiveOrComplete(player, "u16_nym_themepark_mine") && groundquests.hasCompletedQuest(player, "u16_nym_themepark_pirate_hideout") && groundquests.hasCompletedQuest(player, "u16_nym_themepark_elite_kill_pirate_cave_collection");
	}
	
	
	public boolean mako_ghast_condition_hasntDoneHideout(obj_id player, obj_id npc) throws InterruptedException
	{
		faceTo(npc, player);
		return !groundquests.isQuestActiveOrComplete(player, "u16_nym_themepark_pirate_hideout");
	}
	
	
	public boolean mako_ghast_condition_hasCompletedPirateEliteCollection(obj_id player, obj_id npc) throws InterruptedException
	{
		return hasCompletedCollection(player, "kill_nyms_themepark_elite_hideout") && groundquests.isQuestActive(player, "u16_nym_themepark_elite_kill_pirate_cave_collection");
	}
	
	
	public boolean mako_ghast_condition_hasCompletedLabEliteCollection(obj_id player, obj_id npc) throws InterruptedException
	{
		return hasCompletedCollection(player, "kill_nyms_themepark_elite_lab") && groundquests.isQuestActive(player, "u16_nym_themepark_elite_kill_imperial_facility_collection");
	}
	
	
	public boolean mako_ghast_condition_hasCompletedMinerEliteCollection(obj_id player, obj_id npc) throws InterruptedException
	{
		return hasCompletedCollection(player, "kill_nyms_themepark_elite_cave") && groundquests.isQuestActive(player, "u16_nym_themepark_elite_kill_miner_cave_collection");
	}
	
	
	public void mako_ghast_action_grantHideoutKill(obj_id player, obj_id npc) throws InterruptedException
	{
		groundquests.grantQuest(player, "u16_nym_themepark_elite_kill_pirate_cave_collection");
		if (!hasCompletedCollectionSlot(player, "kill_nyms_themepark_cave_troll_activate"))
		{
			modifyCollectionSlotValue(player, "kill_nyms_themepark_cave_troll_activate", 1);
		}
	}
	
	
	public void mako_ghast_action_completeHideoutKill(obj_id player, obj_id npc) throws InterruptedException
	{
		groundquests.sendSignal(player, "hasCompletedKillCaveTroll");
	}
	
	
	public void mako_ghast_action_grantCaveKill(obj_id player, obj_id npc) throws InterruptedException
	{
		groundquests.grantQuest(player, "u16_nym_themepark_elite_kill_miner_cave_collection");
		if (!hasCompletedCollectionSlot(player, "kill_nyms_themepark_elite_cave_activate"))
		{
			modifyCollectionSlotValue(player, "kill_nyms_themepark_elite_cave_activate", 1);
		}
	}
	
	
	public void mako_ghast_action_completeCaveKill(obj_id player, obj_id npc) throws InterruptedException
	{
		groundquests.sendSignal(player, "hasCompletedMineElites");
	}
	
	
	public void mako_ghast_action_grantLabKill(obj_id player, obj_id npc) throws InterruptedException
	{
		groundquests.grantQuest(player, "u16_nym_themepark_elite_kill_imperial_facility_collection");
		if (!hasCompletedCollectionSlot(player, "kill_nyms_themepark_elite_lab_activate"))
		{
			modifyCollectionSlotValue(player, "kill_nyms_themepark_elite_lab_activate", 1);
		}
	}
	
	
	public void mako_ghast_action_completeLabKill(obj_id player, obj_id npc) throws InterruptedException
	{
		groundquests.sendSignal(player, "hasCompletedResearchFacilityElites");
	}
	
	
	public void mako_ghast_action_BruteForceCompletePirateElite(obj_id player, obj_id npc) throws InterruptedException
	{
		if (mako_ghast_condition_hasCompletedPirateEliteCollection(player,npc))
		{
			if (groundquests.isQuestActive(player, "u16_nym_themepark_elite_kill_pirate_cave_collection"))
			{
				CustomerServiceLog("nyms_themepark", "NPC Conversation - Brute Force completing quest: u16_nym_themepark_elite_kill_pirate_cave_collection for player: "+player+" so they do not remailn bugged.");
				int questid = questGetQuestId("quest/u16_nym_themepark_elite_kill_pirate_cave_collection");
				if ((questid != 0) && questIsQuestActive(questid, player))
				{
					questCompleteQuest(questid, player);
				}
			}
		}
	}
	
	
	public void mako_ghast_action_BruteForceCompleteLabElite(obj_id player, obj_id npc) throws InterruptedException
	{
		if (mako_ghast_condition_hasCompletedLabEliteCollection(player,npc))
		{
			if (groundquests.isQuestActive(player, "u16_nym_themepark_elite_kill_imperial_facility_collection"))
			{
				CustomerServiceLog("nyms_themepark", "NPC Conversation - Brute Force completing quest: u16_nym_themepark_elite_kill_imperial_facility_collection for player: "+player+" so they do not remailn bugged.");
				int questid = questGetQuestId("quest/u16_nym_themepark_elite_kill_imperial_facility_collection");
				if ((questid != 0) && questIsQuestActive(questid, player))
				{
					questCompleteQuest(questid, player);
				}
			}
		}
	}
	
	
	public void mako_ghast_action_BruteForceCompleteMinerElite(obj_id player, obj_id npc) throws InterruptedException
	{
		if (mako_ghast_condition_hasCompletedMinerEliteCollection(player,npc))
		{
			if (groundquests.isQuestActive(player, "u16_nym_themepark_elite_kill_miner_cave_collection"))
			{
				CustomerServiceLog("nyms_themepark", "NPC Conversation - Brute Force completing quest: u16_nym_themepark_elite_kill_miner_cave_collection for player: "+player+" so they do not remailn bugged.");
				int questid = questGetQuestId("quest/u16_nym_themepark_elite_kill_miner_cave_collection");
				if ((questid != 0) && questIsQuestActive(questid, player))
				{
					questCompleteQuest(questid, player);
				}
			}
		}
	}
	
	
	public int mako_ghast_handleBranch1(obj_id player, obj_id npc, string_id response) throws InterruptedException
	{
		
		if (response.equals("s_36"))
		{
			
			if (mako_ghast_condition__defaultCondition (player, npc))
			{
				
				string_id message = new string_id (c_stringFile, "s_37");
				int numberOfResponses = 0;
				
				boolean hasResponse = false;
				
				boolean hasResponse0 = false;
				if (mako_ghast_condition__defaultCondition (player, npc))
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
						responses[responseIndex++] = new string_id (c_stringFile, "s_94");
					}
					
					utils.setScriptVar (player, "conversation.mako_ghast.branchId", 2);
					
					prose_package pp = new prose_package ();
					pp.stringId = message;
					pp.actor.set (player);
					pp.target.set (npc);
					
					npcSpeak (player, pp);
					npcSetConversationResponses (player, responses);
				}
				else
				{
					utils.removeScriptVar (player, "conversation.mako_ghast.branchId");
					
					prose_package pp = new prose_package ();
					pp.stringId = message;
					pp.actor.set (player);
					pp.target.set (npc);
					
					npcEndConversationWithMessage (player, pp);
				}
				
				return SCRIPT_CONTINUE;
			}
			
		}
		
		return SCRIPT_DEFAULT;
	}
	
	
	public int mako_ghast_handleBranch2(obj_id player, obj_id npc, string_id response) throws InterruptedException
	{
		
		if (response.equals("s_94"))
		{
			
			if (mako_ghast_condition__defaultCondition (player, npc))
			{
				mako_ghast_action_completeLabKill (player, npc);
				
				string_id message = new string_id (c_stringFile, "s_95");
				utils.removeScriptVar (player, "conversation.mako_ghast.branchId");
				
				npcEndConversationWithMessage (player, message);
				
				return SCRIPT_CONTINUE;
			}
			
		}
		
		return SCRIPT_DEFAULT;
	}
	
	
	public int mako_ghast_handleBranch5(obj_id player, obj_id npc, string_id response) throws InterruptedException
	{
		
		if (response.equals("s_34"))
		{
			
			if (mako_ghast_condition__defaultCondition (player, npc))
			{
				
				string_id message = new string_id (c_stringFile, "s_35");
				utils.removeScriptVar (player, "conversation.mako_ghast.branchId");
				
				npcEndConversationWithMessage (player, message);
				
				return SCRIPT_CONTINUE;
			}
			
		}
		
		return SCRIPT_DEFAULT;
	}
	
	
	public int mako_ghast_handleBranch7(obj_id player, obj_id npc, string_id response) throws InterruptedException
	{
		
		if (response.equals("s_32"))
		{
			
			if (mako_ghast_condition__defaultCondition (player, npc))
			{
				
				string_id message = new string_id (c_stringFile, "s_33");
				int numberOfResponses = 0;
				
				boolean hasResponse = false;
				
				boolean hasResponse0 = false;
				if (mako_ghast_condition__defaultCondition (player, npc))
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
						responses[responseIndex++] = new string_id (c_stringFile, "s_88");
					}
					
					utils.setScriptVar (player, "conversation.mako_ghast.branchId", 8);
					
					npcSpeak (player, message);
					npcSetConversationResponses (player, responses);
				}
				else
				{
					utils.removeScriptVar (player, "conversation.mako_ghast.branchId");
					
					npcEndConversationWithMessage (player, message);
				}
				
				return SCRIPT_CONTINUE;
			}
			
		}
		
		return SCRIPT_DEFAULT;
	}
	
	
	public int mako_ghast_handleBranch8(obj_id player, obj_id npc, string_id response) throws InterruptedException
	{
		
		if (response.equals("s_88"))
		{
			
			if (mako_ghast_condition__defaultCondition (player, npc))
			{
				
				string_id message = new string_id (c_stringFile, "s_89");
				int numberOfResponses = 0;
				
				boolean hasResponse = false;
				
				boolean hasResponse0 = false;
				if (mako_ghast_condition__defaultCondition (player, npc))
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
						responses[responseIndex++] = new string_id (c_stringFile, "s_90");
					}
					
					utils.setScriptVar (player, "conversation.mako_ghast.branchId", 9);
					
					npcSpeak (player, message);
					npcSetConversationResponses (player, responses);
				}
				else
				{
					utils.removeScriptVar (player, "conversation.mako_ghast.branchId");
					
					npcEndConversationWithMessage (player, message);
				}
				
				return SCRIPT_CONTINUE;
			}
			
		}
		
		return SCRIPT_DEFAULT;
	}
	
	
	public int mako_ghast_handleBranch9(obj_id player, obj_id npc, string_id response) throws InterruptedException
	{
		
		if (response.equals("s_90"))
		{
			
			if (mako_ghast_condition__defaultCondition (player, npc))
			{
				
				string_id message = new string_id (c_stringFile, "s_91");
				int numberOfResponses = 0;
				
				boolean hasResponse = false;
				
				boolean hasResponse0 = false;
				if (mako_ghast_condition__defaultCondition (player, npc))
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
						responses[responseIndex++] = new string_id (c_stringFile, "s_92");
					}
					
					utils.setScriptVar (player, "conversation.mako_ghast.branchId", 10);
					
					npcSpeak (player, message);
					npcSetConversationResponses (player, responses);
				}
				else
				{
					utils.removeScriptVar (player, "conversation.mako_ghast.branchId");
					
					npcEndConversationWithMessage (player, message);
				}
				
				return SCRIPT_CONTINUE;
			}
			
		}
		
		return SCRIPT_DEFAULT;
	}
	
	
	public int mako_ghast_handleBranch10(obj_id player, obj_id npc, string_id response) throws InterruptedException
	{
		
		if (response.equals("s_92"))
		{
			
			if (mako_ghast_condition__defaultCondition (player, npc))
			{
				mako_ghast_action_grantLabKill (player, npc);
				
				string_id message = new string_id (c_stringFile, "s_93");
				utils.removeScriptVar (player, "conversation.mako_ghast.branchId");
				
				npcEndConversationWithMessage (player, message);
				
				return SCRIPT_CONTINUE;
			}
			
		}
		
		return SCRIPT_DEFAULT;
	}
	
	
	public int mako_ghast_handleBranch13(obj_id player, obj_id npc, string_id response) throws InterruptedException
	{
		
		if (response.equals("s_27"))
		{
			
			if (mako_ghast_condition__defaultCondition (player, npc))
			{
				mako_ghast_action_completeCaveKill (player, npc);
				
				string_id message = new string_id (c_stringFile, "s_28");
				utils.removeScriptVar (player, "conversation.mako_ghast.branchId");
				
				npcEndConversationWithMessage (player, message);
				
				return SCRIPT_CONTINUE;
			}
			
		}
		
		return SCRIPT_DEFAULT;
	}
	
	
	public int mako_ghast_handleBranch16(obj_id player, obj_id npc, string_id response) throws InterruptedException
	{
		
		if (response.equals("s_41"))
		{
			
			if (mako_ghast_condition__defaultCondition (player, npc))
			{
				
				string_id message = new string_id (c_stringFile, "s_43");
				utils.removeScriptVar (player, "conversation.mako_ghast.branchId");
				
				npcEndConversationWithMessage (player, message);
				
				return SCRIPT_CONTINUE;
			}
			
		}
		
		return SCRIPT_DEFAULT;
	}
	
	
	public int mako_ghast_handleBranch18(obj_id player, obj_id npc, string_id response) throws InterruptedException
	{
		
		if (response.equals("s_47"))
		{
			
			if (mako_ghast_condition__defaultCondition (player, npc))
			{
				
				string_id message = new string_id (c_stringFile, "s_49");
				int numberOfResponses = 0;
				
				boolean hasResponse = false;
				
				boolean hasResponse0 = false;
				if (mako_ghast_condition__defaultCondition (player, npc))
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
						responses[responseIndex++] = new string_id (c_stringFile, "s_67");
					}
					
					utils.setScriptVar (player, "conversation.mako_ghast.branchId", 19);
					
					npcSpeak (player, message);
					npcSetConversationResponses (player, responses);
				}
				else
				{
					utils.removeScriptVar (player, "conversation.mako_ghast.branchId");
					
					npcEndConversationWithMessage (player, message);
				}
				
				return SCRIPT_CONTINUE;
			}
			
		}
		
		return SCRIPT_DEFAULT;
	}
	
	
	public int mako_ghast_handleBranch19(obj_id player, obj_id npc, string_id response) throws InterruptedException
	{
		
		if (response.equals("s_67"))
		{
			
			if (mako_ghast_condition__defaultCondition (player, npc))
			{
				
				string_id message = new string_id (c_stringFile, "s_68");
				int numberOfResponses = 0;
				
				boolean hasResponse = false;
				
				boolean hasResponse0 = false;
				if (mako_ghast_condition__defaultCondition (player, npc))
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
					
					utils.setScriptVar (player, "conversation.mako_ghast.branchId", 20);
					
					npcSpeak (player, message);
					npcSetConversationResponses (player, responses);
				}
				else
				{
					utils.removeScriptVar (player, "conversation.mako_ghast.branchId");
					
					npcEndConversationWithMessage (player, message);
				}
				
				return SCRIPT_CONTINUE;
			}
			
		}
		
		return SCRIPT_DEFAULT;
	}
	
	
	public int mako_ghast_handleBranch20(obj_id player, obj_id npc, string_id response) throws InterruptedException
	{
		
		if (response.equals("s_69"))
		{
			
			if (mako_ghast_condition__defaultCondition (player, npc))
			{
				
				string_id message = new string_id (c_stringFile, "s_70");
				int numberOfResponses = 0;
				
				boolean hasResponse = false;
				
				boolean hasResponse0 = false;
				if (mako_ghast_condition__defaultCondition (player, npc))
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
						responses[responseIndex++] = new string_id (c_stringFile, "s_71");
					}
					
					utils.setScriptVar (player, "conversation.mako_ghast.branchId", 21);
					
					npcSpeak (player, message);
					npcSetConversationResponses (player, responses);
				}
				else
				{
					utils.removeScriptVar (player, "conversation.mako_ghast.branchId");
					
					npcEndConversationWithMessage (player, message);
				}
				
				return SCRIPT_CONTINUE;
			}
			
		}
		
		return SCRIPT_DEFAULT;
	}
	
	
	public int mako_ghast_handleBranch21(obj_id player, obj_id npc, string_id response) throws InterruptedException
	{
		
		if (response.equals("s_71"))
		{
			
			if (mako_ghast_condition__defaultCondition (player, npc))
			{
				
				string_id message = new string_id (c_stringFile, "s_72");
				int numberOfResponses = 0;
				
				boolean hasResponse = false;
				
				boolean hasResponse0 = false;
				if (mako_ghast_condition__defaultCondition (player, npc))
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
					
					utils.setScriptVar (player, "conversation.mako_ghast.branchId", 22);
					
					npcSpeak (player, message);
					npcSetConversationResponses (player, responses);
				}
				else
				{
					utils.removeScriptVar (player, "conversation.mako_ghast.branchId");
					
					npcEndConversationWithMessage (player, message);
				}
				
				return SCRIPT_CONTINUE;
			}
			
		}
		
		return SCRIPT_DEFAULT;
	}
	
	
	public int mako_ghast_handleBranch22(obj_id player, obj_id npc, string_id response) throws InterruptedException
	{
		
		if (response.equals("s_73"))
		{
			
			if (mako_ghast_condition__defaultCondition (player, npc))
			{
				
				string_id message = new string_id (c_stringFile, "s_74");
				int numberOfResponses = 0;
				
				boolean hasResponse = false;
				
				boolean hasResponse0 = false;
				if (mako_ghast_condition__defaultCondition (player, npc))
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
						responses[responseIndex++] = new string_id (c_stringFile, "s_75");
					}
					
					utils.setScriptVar (player, "conversation.mako_ghast.branchId", 23);
					
					npcSpeak (player, message);
					npcSetConversationResponses (player, responses);
				}
				else
				{
					utils.removeScriptVar (player, "conversation.mako_ghast.branchId");
					
					npcEndConversationWithMessage (player, message);
				}
				
				return SCRIPT_CONTINUE;
			}
			
		}
		
		return SCRIPT_DEFAULT;
	}
	
	
	public int mako_ghast_handleBranch23(obj_id player, obj_id npc, string_id response) throws InterruptedException
	{
		
		if (response.equals("s_75"))
		{
			
			if (mako_ghast_condition__defaultCondition (player, npc))
			{
				mako_ghast_action_grantCaveKill (player, npc);
				
				string_id message = new string_id (c_stringFile, "s_76");
				utils.removeScriptVar (player, "conversation.mako_ghast.branchId");
				
				npcEndConversationWithMessage (player, message);
				
				return SCRIPT_CONTINUE;
			}
			
		}
		
		return SCRIPT_DEFAULT;
	}
	
	
	public int mako_ghast_handleBranch26(obj_id player, obj_id npc, string_id response) throws InterruptedException
	{
		
		if (response.equals("s_64"))
		{
			
			if (mako_ghast_condition__defaultCondition (player, npc))
			{
				mako_ghast_action_completeHideoutKill (player, npc);
				
				string_id message = new string_id (c_stringFile, "s_66");
				utils.removeScriptVar (player, "conversation.mako_ghast.branchId");
				
				npcEndConversationWithMessage (player, message);
				
				return SCRIPT_CONTINUE;
			}
			
		}
		
		return SCRIPT_DEFAULT;
	}
	
	
	public int mako_ghast_handleBranch29(obj_id player, obj_id npc, string_id response) throws InterruptedException
	{
		
		if (response.equals("s_81"))
		{
			
			if (mako_ghast_condition__defaultCondition (player, npc))
			{
				
				string_id message = new string_id (c_stringFile, "s_83");
				utils.removeScriptVar (player, "conversation.mako_ghast.branchId");
				
				npcEndConversationWithMessage (player, message);
				
				return SCRIPT_CONTINUE;
			}
			
		}
		
		return SCRIPT_DEFAULT;
	}
	
	
	public int mako_ghast_handleBranch31(obj_id player, obj_id npc, string_id response) throws InterruptedException
	{
		
		if (response.equals("s_87"))
		{
			
			if (mako_ghast_condition__defaultCondition (player, npc))
			{
				
				string_id message = new string_id (c_stringFile, "s_97");
				int numberOfResponses = 0;
				
				boolean hasResponse = false;
				
				boolean hasResponse0 = false;
				if (mako_ghast_condition__defaultCondition (player, npc))
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
						responses[responseIndex++] = new string_id (c_stringFile, "s_99");
					}
					
					utils.setScriptVar (player, "conversation.mako_ghast.branchId", 32);
					
					npcSpeak (player, message);
					npcSetConversationResponses (player, responses);
				}
				else
				{
					utils.removeScriptVar (player, "conversation.mako_ghast.branchId");
					
					npcEndConversationWithMessage (player, message);
				}
				
				return SCRIPT_CONTINUE;
			}
			
		}
		
		return SCRIPT_DEFAULT;
	}
	
	
	public int mako_ghast_handleBranch32(obj_id player, obj_id npc, string_id response) throws InterruptedException
	{
		
		if (response.equals("s_99"))
		{
			
			if (mako_ghast_condition__defaultCondition (player, npc))
			{
				
				string_id message = new string_id (c_stringFile, "s_101");
				int numberOfResponses = 0;
				
				boolean hasResponse = false;
				
				boolean hasResponse0 = false;
				if (mako_ghast_condition__defaultCondition (player, npc))
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
						responses[responseIndex++] = new string_id (c_stringFile, "s_103");
					}
					
					utils.setScriptVar (player, "conversation.mako_ghast.branchId", 33);
					
					npcSpeak (player, message);
					npcSetConversationResponses (player, responses);
				}
				else
				{
					utils.removeScriptVar (player, "conversation.mako_ghast.branchId");
					
					npcEndConversationWithMessage (player, message);
				}
				
				return SCRIPT_CONTINUE;
			}
			
		}
		
		return SCRIPT_DEFAULT;
	}
	
	
	public int mako_ghast_handleBranch33(obj_id player, obj_id npc, string_id response) throws InterruptedException
	{
		
		if (response.equals("s_103"))
		{
			
			if (mako_ghast_condition__defaultCondition (player, npc))
			{
				mako_ghast_action_grantHideoutKill (player, npc);
				
				string_id message = new string_id (c_stringFile, "s_107");
				utils.removeScriptVar (player, "conversation.mako_ghast.branchId");
				
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
			detachScript(self, "conversation.mako_ghast");
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
		detachScript (self, "conversation.mako_ghast");
		
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
		
		if (mako_ghast_condition_hasReturnedEliteLab (player, npc))
		{
			
			string_id message = new string_id (c_stringFile, "s_31");
			int numberOfResponses = 0;
			
			boolean hasResponse = false;
			
			boolean hasResponse0 = false;
			if (mako_ghast_condition__defaultCondition (player, npc))
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
				
				utils.setScriptVar (player, "conversation.mako_ghast.branchId", 1);
				
				npcStartConversation (player, npc, "mako_ghast", message, responses);
			}
			else
			{
				chat.chat (npc, player, message);
			}
			
			return SCRIPT_CONTINUE;
		}
		
		if (mako_ghast_condition_hasCompletedLabEliteCollection (player, npc))
		{
			mako_ghast_action_BruteForceCompleteLabElite (player, npc);
			
			string_id message = new string_id (c_stringFile, "s_109");
			chat.chat (npc, player, message);
			
			return SCRIPT_CONTINUE;
		}
		
		if (mako_ghast_condition_hasEliteLabNotComplete (player, npc))
		{
			
			string_id message = new string_id (c_stringFile, "s_30");
			int numberOfResponses = 0;
			
			boolean hasResponse = false;
			
			boolean hasResponse0 = false;
			if (mako_ghast_condition__defaultCondition (player, npc))
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
				
				utils.setScriptVar (player, "conversation.mako_ghast.branchId", 5);
				
				npcStartConversation (player, npc, "mako_ghast", message, responses);
			}
			else
			{
				chat.chat (npc, player, message);
			}
			
			return SCRIPT_CONTINUE;
		}
		
		if (mako_ghast_condition_canDoEliteLab (player, npc))
		{
			
			string_id message = new string_id (c_stringFile, "s_29");
			int numberOfResponses = 0;
			
			boolean hasResponse = false;
			
			boolean hasResponse0 = false;
			if (mako_ghast_condition__defaultCondition (player, npc))
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
				
				utils.setScriptVar (player, "conversation.mako_ghast.branchId", 7);
				
				prose_package pp = new prose_package ();
				pp.stringId = message;
				pp.actor.set (player);
				pp.target.set (npc);
				
				npcStartConversation (player, npc, "mako_ghast", null, pp, responses);
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
		
		if (mako_ghast_condition_hasntDoneLab (player, npc))
		{
			
			string_id message = new string_id (c_stringFile, "s_105");
			chat.chat (npc, player, message);
			
			return SCRIPT_CONTINUE;
		}
		
		if (mako_ghast_condition_hasReturnedEliteCave (player, npc))
		{
			
			string_id message = new string_id (c_stringFile, "s_23");
			int numberOfResponses = 0;
			
			boolean hasResponse = false;
			
			boolean hasResponse0 = false;
			if (mako_ghast_condition__defaultCondition (player, npc))
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
				
				utils.setScriptVar (player, "conversation.mako_ghast.branchId", 13);
				
				npcStartConversation (player, npc, "mako_ghast", message, responses);
			}
			else
			{
				chat.chat (npc, player, message);
			}
			
			return SCRIPT_CONTINUE;
		}
		
		if (mako_ghast_condition_hasCompletedMinerEliteCollection (player, npc))
		{
			mako_ghast_action_BruteForceCompleteMinerElite (player, npc);
			
			string_id message = new string_id (c_stringFile, "s_110");
			chat.chat (npc, player, message);
			
			return SCRIPT_CONTINUE;
		}
		
		if (mako_ghast_condition_hasEliteCaveNotComplete (player, npc))
		{
			
			string_id message = new string_id (c_stringFile, "s_39");
			int numberOfResponses = 0;
			
			boolean hasResponse = false;
			
			boolean hasResponse0 = false;
			if (mako_ghast_condition__defaultCondition (player, npc))
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
				
				utils.setScriptVar (player, "conversation.mako_ghast.branchId", 16);
				
				npcStartConversation (player, npc, "mako_ghast", message, responses);
			}
			else
			{
				chat.chat (npc, player, message);
			}
			
			return SCRIPT_CONTINUE;
		}
		
		if (mako_ghast_condition_canDoEliteCave (player, npc))
		{
			
			string_id message = new string_id (c_stringFile, "s_45");
			int numberOfResponses = 0;
			
			boolean hasResponse = false;
			
			boolean hasResponse0 = false;
			if (mako_ghast_condition__defaultCondition (player, npc))
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
				
				utils.setScriptVar (player, "conversation.mako_ghast.branchId", 18);
				
				npcStartConversation (player, npc, "mako_ghast", message, responses);
			}
			else
			{
				chat.chat (npc, player, message);
			}
			
			return SCRIPT_CONTINUE;
		}
		
		if (mako_ghast_condition_hasntDoneCave (player, npc))
		{
			
			string_id message = new string_id (c_stringFile, "s_104");
			chat.chat (npc, player, message);
			
			return SCRIPT_CONTINUE;
		}
		
		if (mako_ghast_condition_hasReturnedEliteHideout (player, npc))
		{
			
			string_id message = new string_id (c_stringFile, "s_62");
			int numberOfResponses = 0;
			
			boolean hasResponse = false;
			
			boolean hasResponse0 = false;
			if (mako_ghast_condition__defaultCondition (player, npc))
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
					responses[responseIndex++] = new string_id (c_stringFile, "s_64");
				}
				
				utils.setScriptVar (player, "conversation.mako_ghast.branchId", 26);
				
				npcStartConversation (player, npc, "mako_ghast", message, responses);
			}
			else
			{
				chat.chat (npc, player, message);
			}
			
			return SCRIPT_CONTINUE;
		}
		
		if (mako_ghast_condition_hasCompletedPirateEliteCollection (player, npc))
		{
			mako_ghast_action_BruteForceCompletePirateElite (player, npc);
			
			string_id message = new string_id (c_stringFile, "s_111");
			chat.chat (npc, player, message);
			
			return SCRIPT_CONTINUE;
		}
		
		if (mako_ghast_condition_hasEliteHideoutNotComplete (player, npc))
		{
			
			string_id message = new string_id (c_stringFile, "s_79");
			int numberOfResponses = 0;
			
			boolean hasResponse = false;
			
			boolean hasResponse0 = false;
			if (mako_ghast_condition__defaultCondition (player, npc))
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
					responses[responseIndex++] = new string_id (c_stringFile, "s_81");
				}
				
				utils.setScriptVar (player, "conversation.mako_ghast.branchId", 29);
				
				npcStartConversation (player, npc, "mako_ghast", message, responses);
			}
			else
			{
				chat.chat (npc, player, message);
			}
			
			return SCRIPT_CONTINUE;
		}
		
		if (mako_ghast_condition_canDoEliteHideout (player, npc))
		{
			
			string_id message = new string_id (c_stringFile, "s_85");
			int numberOfResponses = 0;
			
			boolean hasResponse = false;
			
			boolean hasResponse0 = false;
			if (mako_ghast_condition__defaultCondition (player, npc))
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
					responses[responseIndex++] = new string_id (c_stringFile, "s_87");
				}
				
				utils.setScriptVar (player, "conversation.mako_ghast.branchId", 31);
				
				prose_package pp = new prose_package ();
				pp.stringId = message;
				pp.actor.set (player);
				pp.target.set (npc);
				
				npcStartConversation (player, npc, "mako_ghast", null, pp, responses);
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
		
		if (mako_ghast_condition_hasntDoneHideout (player, npc))
		{
			
			string_id message = new string_id (c_stringFile, "s_112");
			chat.chat (npc, player, message);
			
			return SCRIPT_CONTINUE;
		}
		
		if (mako_ghast_condition__defaultCondition (player, npc))
		{
			
			string_id message = new string_id (c_stringFile, "s_114");
			chat.chat (npc, player, message);
			
			return SCRIPT_CONTINUE;
		}
		
		chat.chat (npc, "Error: All conditions for OnStartNpcConversation were false.");
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int OnNpcConversationResponse(obj_id self, String conversationId, obj_id player, string_id response) throws InterruptedException
	{
		if (!conversationId.equals("mako_ghast"))
		{
			return SCRIPT_CONTINUE;
		}
		
		obj_id npc = self;
		
		int branchId = utils.getIntScriptVar (player, "conversation.mako_ghast.branchId");
		
		if (branchId == 1 && mako_ghast_handleBranch1 (player, npc, response) == SCRIPT_CONTINUE)
		{
			return SCRIPT_CONTINUE;
		}
		
		if (branchId == 2 && mako_ghast_handleBranch2 (player, npc, response) == SCRIPT_CONTINUE)
		{
			return SCRIPT_CONTINUE;
		}
		
		if (branchId == 5 && mako_ghast_handleBranch5 (player, npc, response) == SCRIPT_CONTINUE)
		{
			return SCRIPT_CONTINUE;
		}
		
		if (branchId == 7 && mako_ghast_handleBranch7 (player, npc, response) == SCRIPT_CONTINUE)
		{
			return SCRIPT_CONTINUE;
		}
		
		if (branchId == 8 && mako_ghast_handleBranch8 (player, npc, response) == SCRIPT_CONTINUE)
		{
			return SCRIPT_CONTINUE;
		}
		
		if (branchId == 9 && mako_ghast_handleBranch9 (player, npc, response) == SCRIPT_CONTINUE)
		{
			return SCRIPT_CONTINUE;
		}
		
		if (branchId == 10 && mako_ghast_handleBranch10 (player, npc, response) == SCRIPT_CONTINUE)
		{
			return SCRIPT_CONTINUE;
		}
		
		if (branchId == 13 && mako_ghast_handleBranch13 (player, npc, response) == SCRIPT_CONTINUE)
		{
			return SCRIPT_CONTINUE;
		}
		
		if (branchId == 16 && mako_ghast_handleBranch16 (player, npc, response) == SCRIPT_CONTINUE)
		{
			return SCRIPT_CONTINUE;
		}
		
		if (branchId == 18 && mako_ghast_handleBranch18 (player, npc, response) == SCRIPT_CONTINUE)
		{
			return SCRIPT_CONTINUE;
		}
		
		if (branchId == 19 && mako_ghast_handleBranch19 (player, npc, response) == SCRIPT_CONTINUE)
		{
			return SCRIPT_CONTINUE;
		}
		
		if (branchId == 20 && mako_ghast_handleBranch20 (player, npc, response) == SCRIPT_CONTINUE)
		{
			return SCRIPT_CONTINUE;
		}
		
		if (branchId == 21 && mako_ghast_handleBranch21 (player, npc, response) == SCRIPT_CONTINUE)
		{
			return SCRIPT_CONTINUE;
		}
		
		if (branchId == 22 && mako_ghast_handleBranch22 (player, npc, response) == SCRIPT_CONTINUE)
		{
			return SCRIPT_CONTINUE;
		}
		
		if (branchId == 23 && mako_ghast_handleBranch23 (player, npc, response) == SCRIPT_CONTINUE)
		{
			return SCRIPT_CONTINUE;
		}
		
		if (branchId == 26 && mako_ghast_handleBranch26 (player, npc, response) == SCRIPT_CONTINUE)
		{
			return SCRIPT_CONTINUE;
		}
		
		if (branchId == 29 && mako_ghast_handleBranch29 (player, npc, response) == SCRIPT_CONTINUE)
		{
			return SCRIPT_CONTINUE;
		}
		
		if (branchId == 31 && mako_ghast_handleBranch31 (player, npc, response) == SCRIPT_CONTINUE)
		{
			return SCRIPT_CONTINUE;
		}
		
		if (branchId == 32 && mako_ghast_handleBranch32 (player, npc, response) == SCRIPT_CONTINUE)
		{
			return SCRIPT_CONTINUE;
		}
		
		if (branchId == 33 && mako_ghast_handleBranch33 (player, npc, response) == SCRIPT_CONTINUE)
		{
			return SCRIPT_CONTINUE;
		}
		
		chat.chat (npc, "Error: Fell through all branches and responses for OnNpcConversationResponse.");
		
		utils.removeScriptVar (player, "conversation.mako_ghast.branchId");
		
		return SCRIPT_CONTINUE;
	}
	
}
