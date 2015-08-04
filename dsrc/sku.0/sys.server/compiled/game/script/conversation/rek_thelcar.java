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
import script.library.static_item;
import script.library.utils;


public class rek_thelcar extends script.base_script
{
	public rek_thelcar()
	{
	}
	String c_stringFile = "conversation/rek_thelcar";
	
	
	public boolean rek_thelcar_condition__defaultCondition(obj_id player, obj_id npc) throws InterruptedException
	{
		return true;
	}
	
	
	public boolean rek_thelcar_condition_isEligibleSurveillance(obj_id player, obj_id npc) throws InterruptedException
	{
		faceTo(npc,player);
		return !groundquests.isQuestActiveOrComplete(player, "u16_nym_themepark_pirate_surveillance_collection") && groundquests.isQuestActiveOrComplete(player, "u16_nym_themepark_pirate_hideout");
	}
	
	
	public boolean rek_thelcar_condition_isElligibleMinerDna(obj_id player, obj_id npc) throws InterruptedException
	{
		faceTo(npc,player);
		return !groundquests.isQuestActiveOrComplete(player, "u16_nym_themepark_miner_dna_collection") && groundquests.hasCompletedQuest(player, "u16_nym_themepark_pirate_surveillance_collection") && groundquests.isQuestActiveOrComplete(player, "u16_nym_themepark_mine");
	}
	
	
	public boolean rek_thelcar_condition_isElligibleQuestion(obj_id player, obj_id npc) throws InterruptedException
	{
		faceTo(npc,player);
		return !groundquests.isQuestActiveOrComplete(player, "u16_nym_themepark_question_scientist_collection") && groundquests.hasCompletedQuest(player, "u16_nym_themepark_miner_dna_collection") && groundquests.isQuestActiveOrComplete(player, "u16_nym_themepark_research_facility");
	}
	
	
	public boolean rek_thelcar_condition_hasReturnedMinerDna(obj_id player, obj_id npc) throws InterruptedException
	{
		faceTo(npc,player);
		return groundquests.isTaskActive(player, "u16_nym_themepark_miner_dna_collection", "returnExtractMinerDnaComplete");
	}
	
	
	public boolean rek_thelcar_condition_hasReturnedQuestion(obj_id player, obj_id npc) throws InterruptedException
	{
		faceTo(npc,player);
		return groundquests.isTaskActive(player, "u16_nym_themepark_question_scientist_collection", "returnQuestionComplete");
	}
	
	
	public boolean rek_thelcar_condition_hasSurveillanceNotComplete(obj_id player, obj_id npc) throws InterruptedException
	{
		faceTo(npc, player);
		return groundquests.isQuestActive(player, "u16_nym_themepark_pirate_surveillance_collection");
	}
	
	
	public boolean rek_thelcar_condition_hasMinerDnaNotComplete(obj_id player, obj_id npc) throws InterruptedException
	{
		faceTo(npc, player);
		return groundquests.isQuestActive(player, "u16_nym_themepark_miner_dna_collection");
	}
	
	
	public boolean rek_thelcar_condition_hasQuestionNotComplete(obj_id player, obj_id npc) throws InterruptedException
	{
		faceTo(npc, player);
		return groundquests.isQuestActive(player, "u16_nym_themepark_question_scientist_collection");
	}
	
	
	public boolean rek_thelcar_condition_hasReturnedSurveillance(obj_id player, obj_id npc) throws InterruptedException
	{
		faceTo(npc,player);
		return groundquests.isTaskActive(player, "u16_nym_themepark_pirate_surveillance_collection", "returnSurveillanceComplete");
	}
	
	
	public boolean rek_thelcar_condition_hasntDoneSurveillance(obj_id player, obj_id npc) throws InterruptedException
	{
		faceTo(npc, player);
		return !groundquests.isQuestActiveOrComplete(player, "u16_nym_themepark_pirate_hideout");
	}
	
	
	public boolean rek_thelcar_condition_hasntDoneMinerDNA(obj_id player, obj_id npc) throws InterruptedException
	{
		faceTo(npc, player);
		return !groundquests.isQuestActiveOrComplete(player, "u16_nym_themepark_mine") && groundquests.hasCompletedQuest(player, "u16_nym_themepark_pirate_hideout") && groundquests.hasCompletedQuest(player, "u16_nym_themepark_pirate_surveillance_collection");
	}
	
	
	public boolean rek_thelcar_condition_hasntDoneQuestion(obj_id player, obj_id npc) throws InterruptedException
	{
		faceTo(npc, player);
		return !groundquests.isQuestActiveOrComplete(player, "u16_nym_themepark_research_facility") && groundquests.hasCompletedQuest(player, "u16_nym_themepark_mine") && groundquests.hasCompletedQuest(player, "u16_nym_themepark_miner_dna_collection");
		
	}
	
	
	public boolean rek_thelcar_condition_hasCompletedScientistCollection(obj_id player, obj_id npc) throws InterruptedException
	{
		return hasCompletedCollection(player, "nyms_question_scientist") && groundquests.isQuestActive(player, "u16_nym_themepark_question_scientist_collection");
	}
	
	
	public boolean rek_thelcar_condition_hasCompletedDNACollection(obj_id player, obj_id npc) throws InterruptedException
	{
		return hasCompletedCollection(player, "nyms_miner_dna") && groundquests.isQuestActive(player, "u16_nym_themepark_miner_dna_collection");
	}
	
	
	public boolean rek_thelcar_condition_hasCompletedDeviceCollection(obj_id player, obj_id npc) throws InterruptedException
	{
		return hasCompletedCollection(player, "nyms_surveillance_device") && groundquests.isQuestActive(player, "u16_nym_themepark_pirate_surveillance_collection");
	}
	
	
	public void rek_thelcar_action_grantSurveillanceQuest(obj_id player, obj_id npc) throws InterruptedException
	{
		groundquests.grantQuest(player, "u16_nym_themepark_pirate_surveillance_collection");
		if (!hasCompletedCollectionSlot(player, "nym_surveillance_activation"))
		{
			modifyCollectionSlotValue(player, "nym_surveillance_activation", 1);
		}
	}
	
	
	public void rek_thelcar_action_grantMinerDnaQuest(obj_id player, obj_id npc) throws InterruptedException
	{
		obj_id playerInv = utils.getInventoryContainer(player);
		static_item.createNewItemFunction("item_nym_themepark_dna_extractor", playerInv);
		groundquests.grantQuest(player, "u16_nym_themepark_miner_dna_collection");
		if (!hasCompletedCollectionSlot(player, "nym_miner_dna_activation"))
		{
			modifyCollectionSlotValue(player, "nym_miner_dna_activation", 1);
		}
	}
	
	
	public void rek_thelcar_action_grantQuestionScientistQuest(obj_id player, obj_id npc) throws InterruptedException
	{
		groundquests.grantQuest(player, "u16_nym_themepark_question_scientist_collection");
		if (!hasCompletedCollectionSlot(player, "nym_question_scientist_activation"))
		{
			modifyCollectionSlotValue(player, "nym_question_scientist_activation", 1);
		}
	}
	
	
	public void rek_thelcar_action_completeSurveillanceQuest(obj_id player, obj_id npc) throws InterruptedException
	{
		groundquests.sendSignal(player, "hasCompletedSurveillance");
		if (!hasCompletedCollectionSlot(player, "icon_nyms_master_collection_1_surveillance"))
		{
			modifyCollectionSlotValue(player, "icon_nyms_master_collection_1_surveillance", 1);
		}
	}
	
	
	public void rek_thelcar_action_completeMinerDnaQuest(obj_id player, obj_id npc) throws InterruptedException
	{
		groundquests.sendSignal(player, "hasCompletedMinerDna");
		if (!hasCompletedCollectionSlot(player, "icon_nyms_master_collection_1_dna"))
		{
			modifyCollectionSlotValue(player, "icon_nyms_master_collection_1_dna", 1);
		}
	}
	
	
	public void rek_thelcar_action_completeQuestionQuest(obj_id player, obj_id npc) throws InterruptedException
	{
		groundquests.sendSignal(player, "hasCompletedQuestion");
		if (!hasCompletedCollectionSlot(player, "icon_nyms_master_collection_1_scientist"))
		{
			modifyCollectionSlotValue(player, "icon_nyms_master_collection_1_scientist", 1);
		}
	}
	
	
	public void rek_thelcar_action_BruteForceCompleteScientist(obj_id player, obj_id npc) throws InterruptedException
	{
		if (rek_thelcar_condition_hasCompletedDNACollection(player,npc))
		{
			if (groundquests.isQuestActive(player, "u16_nym_themepark_question_scientist_collection"))
			{
				CustomerServiceLog("nyms_themepark", "NPC Conversation - Brute Force completing quest: u16_nym_themepark_question_scientist_collection for player: "+player+" so they do not remailn bugged.");
				int questid = questGetQuestId("quest/u16_nym_themepark_question_scientist_collection");
				if ((questid != 0) && questIsQuestActive(questid, player))
				{
					questCompleteQuest(questid, player);
				}
			}
		}
	}
	
	
	public void rek_thelcar_action_BruteForceCompleteDNA(obj_id player, obj_id npc) throws InterruptedException
	{
		if (rek_thelcar_condition_hasCompletedDNACollection(player,npc))
		{
			if (groundquests.isQuestActive(player, "u16_nym_themepark_miner_dna_collection"))
			{
				CustomerServiceLog("nyms_themepark", "NPC Conversation - Brute Force completing quest: u16_nym_themepark_miner_dna_collection for player: "+player+" so they do not remailn bugged.");
				int questid = questGetQuestId("quest/u16_nym_themepark_miner_dna_collection");
				if ((questid != 0) && questIsQuestActive(questid, player))
				{
					questCompleteQuest(questid, player);
				}
			}
		}
	}
	
	
	public void rek_thelcar_action_BruteForceCompleteDevice(obj_id player, obj_id npc) throws InterruptedException
	{
		if (rek_thelcar_condition_hasCompletedDeviceCollection(player,npc))
		{
			if (groundquests.isQuestActive(player, "u16_nym_themepark_pirate_surveillance_collection"))
			{
				CustomerServiceLog("nyms_themepark", "NPC Conversation - Brute Force completing quest: u16_nym_themepark_pirate_surveillance_collection for player: "+player+" so they do not remailn bugged.");
				int questid = questGetQuestId("quest/u16_nym_themepark_pirate_surveillance_collection");
				if ((questid != 0) && questIsQuestActive(questid, player))
				{
					questCompleteQuest(questid, player);
				}
			}
		}
	}
	
	
	public int rek_thelcar_handleBranch1(obj_id player, obj_id npc, string_id response) throws InterruptedException
	{
		
		if (response.equals("s_114"))
		{
			
			if (rek_thelcar_condition__defaultCondition (player, npc))
			{
				doAnimationAction (npc, "whisper");
				
				string_id message = new string_id (c_stringFile, "s_115");
				int numberOfResponses = 0;
				
				boolean hasResponse = false;
				
				boolean hasResponse0 = false;
				if (rek_thelcar_condition__defaultCondition (player, npc))
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
						responses[responseIndex++] = new string_id (c_stringFile, "s_116");
					}
					
					utils.setScriptVar (player, "conversation.rek_thelcar.branchId", 2);
					
					npcSpeak (player, message);
					npcSetConversationResponses (player, responses);
				}
				else
				{
					utils.removeScriptVar (player, "conversation.rek_thelcar.branchId");
					
					npcEndConversationWithMessage (player, message);
				}
				
				return SCRIPT_CONTINUE;
			}
			
		}
		
		return SCRIPT_DEFAULT;
	}
	
	
	public int rek_thelcar_handleBranch2(obj_id player, obj_id npc, string_id response) throws InterruptedException
	{
		
		if (response.equals("s_116"))
		{
			doAnimationAction (player, "cough_polite");
			
			if (rek_thelcar_condition__defaultCondition (player, npc))
			{
				doAnimationAction (npc, "handshake_tandem");
				
				rek_thelcar_action_completeQuestionQuest (player, npc);
				
				string_id message = new string_id (c_stringFile, "s_117");
				utils.removeScriptVar (player, "conversation.rek_thelcar.branchId");
				
				npcEndConversationWithMessage (player, message);
				
				return SCRIPT_CONTINUE;
			}
			
		}
		
		return SCRIPT_DEFAULT;
	}
	
	
	public int rek_thelcar_handleBranch5(obj_id player, obj_id npc, string_id response) throws InterruptedException
	{
		
		if (response.equals("s_105"))
		{
			
			if (rek_thelcar_condition__defaultCondition (player, npc))
			{
				doAnimationAction (npc, "wave_on_dismissing");
				
				string_id message = new string_id (c_stringFile, "s_106");
				utils.removeScriptVar (player, "conversation.rek_thelcar.branchId");
				
				npcEndConversationWithMessage (player, message);
				
				return SCRIPT_CONTINUE;
			}
			
		}
		
		return SCRIPT_DEFAULT;
	}
	
	
	public int rek_thelcar_handleBranch7(obj_id player, obj_id npc, string_id response) throws InterruptedException
	{
		
		if (response.equals("s_44"))
		{
			
			if (rek_thelcar_condition__defaultCondition (player, npc))
			{
				doAnimationAction (npc, "nod_head_once");
				
				string_id message = new string_id (c_stringFile, "s_45");
				int numberOfResponses = 0;
				
				boolean hasResponse = false;
				
				boolean hasResponse0 = false;
				if (rek_thelcar_condition__defaultCondition (player, npc))
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
					
					utils.setScriptVar (player, "conversation.rek_thelcar.branchId", 8);
					
					npcSpeak (player, message);
					npcSetConversationResponses (player, responses);
				}
				else
				{
					utils.removeScriptVar (player, "conversation.rek_thelcar.branchId");
					
					npcEndConversationWithMessage (player, message);
				}
				
				return SCRIPT_CONTINUE;
			}
			
		}
		
		if (response.equals("s_80"))
		{
			doAnimationAction (player, "check_wrist_device");
			
			if (rek_thelcar_condition__defaultCondition (player, npc))
			{
				doAnimationAction (npc, "hug_self");
				
				string_id message = new string_id (c_stringFile, "s_104");
				utils.removeScriptVar (player, "conversation.rek_thelcar.branchId");
				
				npcEndConversationWithMessage (player, message);
				
				return SCRIPT_CONTINUE;
			}
			
		}
		
		return SCRIPT_DEFAULT;
	}
	
	
	public int rek_thelcar_handleBranch8(obj_id player, obj_id npc, string_id response) throws InterruptedException
	{
		
		if (response.equals("s_46"))
		{
			
			if (rek_thelcar_condition__defaultCondition (player, npc))
			{
				doAnimationAction (npc, "shake_head_disgust");
				
				string_id message = new string_id (c_stringFile, "s_47");
				int numberOfResponses = 0;
				
				boolean hasResponse = false;
				
				boolean hasResponse0 = false;
				if (rek_thelcar_condition__defaultCondition (player, npc))
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
						responses[responseIndex++] = new string_id (c_stringFile, "s_48");
					}
					
					utils.setScriptVar (player, "conversation.rek_thelcar.branchId", 9);
					
					npcSpeak (player, message);
					npcSetConversationResponses (player, responses);
				}
				else
				{
					utils.removeScriptVar (player, "conversation.rek_thelcar.branchId");
					
					npcEndConversationWithMessage (player, message);
				}
				
				return SCRIPT_CONTINUE;
			}
			
		}
		
		return SCRIPT_DEFAULT;
	}
	
	
	public int rek_thelcar_handleBranch9(obj_id player, obj_id npc, string_id response) throws InterruptedException
	{
		
		if (response.equals("s_48"))
		{
			
			if (rek_thelcar_condition__defaultCondition (player, npc))
			{
				doAnimationAction (npc, "shake_head_no");
				
				string_id message = new string_id (c_stringFile, "s_49");
				int numberOfResponses = 0;
				
				boolean hasResponse = false;
				
				boolean hasResponse0 = false;
				if (rek_thelcar_condition__defaultCondition (player, npc))
				{
					++numberOfResponses;
					hasResponse = true;
					hasResponse0 = true;
				}
				
				boolean hasResponse1 = false;
				if (rek_thelcar_condition__defaultCondition (player, npc))
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
						responses[responseIndex++] = new string_id (c_stringFile, "s_50");
					}
					
					if (hasResponse1)
					{
						responses[responseIndex++] = new string_id (c_stringFile, "s_52");
					}
					
					utils.setScriptVar (player, "conversation.rek_thelcar.branchId", 10);
					
					npcSpeak (player, message);
					npcSetConversationResponses (player, responses);
				}
				else
				{
					utils.removeScriptVar (player, "conversation.rek_thelcar.branchId");
					
					npcEndConversationWithMessage (player, message);
				}
				
				return SCRIPT_CONTINUE;
			}
			
		}
		
		return SCRIPT_DEFAULT;
	}
	
	
	public int rek_thelcar_handleBranch10(obj_id player, obj_id npc, string_id response) throws InterruptedException
	{
		
		if (response.equals("s_50"))
		{
			doAnimationAction (player, "huh");
			
			if (rek_thelcar_condition__defaultCondition (player, npc))
			{
				doAnimationAction (npc, "shoo");
				
				rek_thelcar_action_grantQuestionScientistQuest (player, npc);
				
				string_id message = new string_id (c_stringFile, "s_51");
				utils.removeScriptVar (player, "conversation.rek_thelcar.branchId");
				
				npcEndConversationWithMessage (player, message);
				
				return SCRIPT_CONTINUE;
			}
			
		}
		
		if (response.equals("s_52"))
		{
			doAnimationAction (player, "wave_on_dismissing");
			
			if (rek_thelcar_condition__defaultCondition (player, npc))
			{
				doAnimationAction (npc, "sigh_deeply");
				
				string_id message = new string_id (c_stringFile, "s_53");
				utils.removeScriptVar (player, "conversation.rek_thelcar.branchId");
				
				npcEndConversationWithMessage (player, message);
				
				return SCRIPT_CONTINUE;
			}
			
		}
		
		return SCRIPT_DEFAULT;
	}
	
	
	public int rek_thelcar_handleBranch15(obj_id player, obj_id npc, string_id response) throws InterruptedException
	{
		
		if (response.equals("s_109"))
		{
			
			if (rek_thelcar_condition__defaultCondition (player, npc))
			{
				doAnimationAction (npc, "rub_chin_thoughtful");
				
				string_id message = new string_id (c_stringFile, "s_110");
				int numberOfResponses = 0;
				
				boolean hasResponse = false;
				
				boolean hasResponse0 = false;
				if (rek_thelcar_condition__defaultCondition (player, npc))
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
						responses[responseIndex++] = new string_id (c_stringFile, "s_111");
					}
					
					utils.setScriptVar (player, "conversation.rek_thelcar.branchId", 16);
					
					npcSpeak (player, message);
					npcSetConversationResponses (player, responses);
				}
				else
				{
					utils.removeScriptVar (player, "conversation.rek_thelcar.branchId");
					
					npcEndConversationWithMessage (player, message);
				}
				
				return SCRIPT_CONTINUE;
			}
			
		}
		
		return SCRIPT_DEFAULT;
	}
	
	
	public int rek_thelcar_handleBranch16(obj_id player, obj_id npc, string_id response) throws InterruptedException
	{
		
		if (response.equals("s_111"))
		{
			
			if (rek_thelcar_condition__defaultCondition (player, npc))
			{
				doAnimationAction (npc, "handshake_tandem");
				
				rek_thelcar_action_completeMinerDnaQuest (player, npc);
				
				string_id message = new string_id (c_stringFile, "s_112");
				utils.removeScriptVar (player, "conversation.rek_thelcar.branchId");
				
				npcEndConversationWithMessage (player, message);
				
				return SCRIPT_CONTINUE;
			}
			
		}
		
		return SCRIPT_DEFAULT;
	}
	
	
	public int rek_thelcar_handleBranch19(obj_id player, obj_id npc, string_id response) throws InterruptedException
	{
		
		if (response.equals("s_102"))
		{
			
			if (rek_thelcar_condition__defaultCondition (player, npc))
			{
				doAnimationAction (npc, "wave_on_dismissing");
				
				string_id message = new string_id (c_stringFile, "s_103");
				utils.removeScriptVar (player, "conversation.rek_thelcar.branchId");
				
				npcEndConversationWithMessage (player, message);
				
				return SCRIPT_CONTINUE;
			}
			
		}
		
		return SCRIPT_DEFAULT;
	}
	
	
	public int rek_thelcar_handleBranch21(obj_id player, obj_id npc, string_id response) throws InterruptedException
	{
		
		if (response.equals("s_38"))
		{
			doAnimationAction (player, "nod");
			
			if (rek_thelcar_condition__defaultCondition (player, npc))
			{
				doAnimationAction (npc, "nod_head_multiple");
				
				string_id message = new string_id (c_stringFile, "s_55");
				int numberOfResponses = 0;
				
				boolean hasResponse = false;
				
				boolean hasResponse0 = false;
				if (rek_thelcar_condition__defaultCondition (player, npc))
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
					
					utils.setScriptVar (player, "conversation.rek_thelcar.branchId", 23);
					
					npcSpeak (player, message);
					npcSetConversationResponses (player, responses);
				}
				else
				{
					utils.removeScriptVar (player, "conversation.rek_thelcar.branchId");
					
					npcEndConversationWithMessage (player, message);
				}
				
				return SCRIPT_CONTINUE;
			}
			
		}
		
		if (response.equals("s_41"))
		{
			doAnimationAction (player, "shake_head_no");
			
			if (rek_thelcar_condition__defaultCondition (player, npc))
			{
				doAnimationAction (npc, "explain");
				
				string_id message = new string_id (c_stringFile, "s_54");
				int numberOfResponses = 0;
				
				boolean hasResponse = false;
				
				boolean hasResponse0 = false;
				if (rek_thelcar_condition__defaultCondition (player, npc))
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
					
					utils.setScriptVar (player, "conversation.rek_thelcar.branchId", 23);
					
					npcSpeak (player, message);
					npcSetConversationResponses (player, responses);
				}
				else
				{
					utils.removeScriptVar (player, "conversation.rek_thelcar.branchId");
					
					npcEndConversationWithMessage (player, message);
				}
				
				return SCRIPT_CONTINUE;
			}
			
		}
		
		if (response.equals("s_79"))
		{
			doAnimationAction (player, "check_wrist_device");
			
			if (rek_thelcar_condition__defaultCondition (player, npc))
			{
				doAnimationAction (npc, "wave_on_dismissing");
				
				string_id message = new string_id (c_stringFile, "s_101");
				utils.removeScriptVar (player, "conversation.rek_thelcar.branchId");
				
				npcEndConversationWithMessage (player, message);
				
				return SCRIPT_CONTINUE;
			}
			
		}
		
		return SCRIPT_DEFAULT;
	}
	
	
	public int rek_thelcar_handleBranch22(obj_id player, obj_id npc, string_id response) throws InterruptedException
	{
		
		if (response.equals("s_57"))
		{
			
			if (rek_thelcar_condition__defaultCondition (player, npc))
			{
				doAnimationAction (npc, "shrug_hands");
				
				string_id message = new string_id (c_stringFile, "s_59");
				int numberOfResponses = 0;
				
				boolean hasResponse = false;
				
				boolean hasResponse0 = false;
				if (rek_thelcar_condition__defaultCondition (player, npc))
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
						responses[responseIndex++] = new string_id (c_stringFile, "s_61");
					}
					
					utils.setScriptVar (player, "conversation.rek_thelcar.branchId", 24);
					
					npcSpeak (player, message);
					npcSetConversationResponses (player, responses);
				}
				else
				{
					utils.removeScriptVar (player, "conversation.rek_thelcar.branchId");
					
					npcEndConversationWithMessage (player, message);
				}
				
				return SCRIPT_CONTINUE;
			}
			
		}
		
		return SCRIPT_DEFAULT;
	}
	
	
	public int rek_thelcar_handleBranch23(obj_id player, obj_id npc, string_id response) throws InterruptedException
	{
		
		if (response.equals("s_57"))
		{
			
			if (rek_thelcar_condition__defaultCondition (player, npc))
			{
				doAnimationAction (npc, "shrug_hands");
				
				string_id message = new string_id (c_stringFile, "s_59");
				int numberOfResponses = 0;
				
				boolean hasResponse = false;
				
				boolean hasResponse0 = false;
				if (rek_thelcar_condition__defaultCondition (player, npc))
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
						responses[responseIndex++] = new string_id (c_stringFile, "s_61");
					}
					
					utils.setScriptVar (player, "conversation.rek_thelcar.branchId", 24);
					
					npcSpeak (player, message);
					npcSetConversationResponses (player, responses);
				}
				else
				{
					utils.removeScriptVar (player, "conversation.rek_thelcar.branchId");
					
					npcEndConversationWithMessage (player, message);
				}
				
				return SCRIPT_CONTINUE;
			}
			
		}
		
		return SCRIPT_DEFAULT;
	}
	
	
	public int rek_thelcar_handleBranch24(obj_id player, obj_id npc, string_id response) throws InterruptedException
	{
		
		if (response.equals("s_61"))
		{
			
			if (rek_thelcar_condition__defaultCondition (player, npc))
			{
				doAnimationAction (npc, "poke");
				
				string_id message = new string_id (c_stringFile, "s_63");
				int numberOfResponses = 0;
				
				boolean hasResponse = false;
				
				boolean hasResponse0 = false;
				if (rek_thelcar_condition__defaultCondition (player, npc))
				{
					++numberOfResponses;
					hasResponse = true;
					hasResponse0 = true;
				}
				
				boolean hasResponse1 = false;
				if (rek_thelcar_condition__defaultCondition (player, npc))
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
						responses[responseIndex++] = new string_id (c_stringFile, "s_65");
					}
					
					if (hasResponse1)
					{
						responses[responseIndex++] = new string_id (c_stringFile, "s_69");
					}
					
					utils.setScriptVar (player, "conversation.rek_thelcar.branchId", 25);
					
					npcSpeak (player, message);
					npcSetConversationResponses (player, responses);
				}
				else
				{
					utils.removeScriptVar (player, "conversation.rek_thelcar.branchId");
					
					npcEndConversationWithMessage (player, message);
				}
				
				return SCRIPT_CONTINUE;
			}
			
		}
		
		return SCRIPT_DEFAULT;
	}
	
	
	public int rek_thelcar_handleBranch25(obj_id player, obj_id npc, string_id response) throws InterruptedException
	{
		
		if (response.equals("s_65"))
		{
			
			if (rek_thelcar_condition__defaultCondition (player, npc))
			{
				doAnimationAction (npc, "shoo");
				
				rek_thelcar_action_grantMinerDnaQuest (player, npc);
				
				string_id message = new string_id (c_stringFile, "s_67");
				utils.removeScriptVar (player, "conversation.rek_thelcar.branchId");
				
				npcEndConversationWithMessage (player, message);
				
				return SCRIPT_CONTINUE;
			}
			
		}
		
		if (response.equals("s_69"))
		{
			doAnimationAction (player, "thumbs_up");
			
			if (rek_thelcar_condition__defaultCondition (player, npc))
			{
				doAnimationAction (npc, "shrug_shoulders");
				
				string_id message = new string_id (c_stringFile, "s_71");
				utils.removeScriptVar (player, "conversation.rek_thelcar.branchId");
				
				npcEndConversationWithMessage (player, message);
				
				return SCRIPT_CONTINUE;
			}
			
		}
		
		return SCRIPT_DEFAULT;
	}
	
	
	public int rek_thelcar_handleBranch30(obj_id player, obj_id npc, string_id response) throws InterruptedException
	{
		
		if (response.equals("s_87"))
		{
			
			if (rek_thelcar_condition__defaultCondition (player, npc))
			{
				doAnimationAction (npc, "hands_above_head");
				
				string_id message = new string_id (c_stringFile, "s_88");
				int numberOfResponses = 0;
				
				boolean hasResponse = false;
				
				boolean hasResponse0 = false;
				if (rek_thelcar_condition__defaultCondition (player, npc))
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
						responses[responseIndex++] = new string_id (c_stringFile, "s_89");
					}
					
					utils.setScriptVar (player, "conversation.rek_thelcar.branchId", 31);
					
					npcSpeak (player, message);
					npcSetConversationResponses (player, responses);
				}
				else
				{
					utils.removeScriptVar (player, "conversation.rek_thelcar.branchId");
					
					npcEndConversationWithMessage (player, message);
				}
				
				return SCRIPT_CONTINUE;
			}
			
		}
		
		return SCRIPT_DEFAULT;
	}
	
	
	public int rek_thelcar_handleBranch31(obj_id player, obj_id npc, string_id response) throws InterruptedException
	{
		
		if (response.equals("s_89"))
		{
			doAnimationAction (player, "shrug_shoulders");
			
			if (rek_thelcar_condition__defaultCondition (player, npc))
			{
				doAnimationAction (npc, "bow5");
				
				rek_thelcar_action_completeSurveillanceQuest (player, npc);
				
				string_id message = new string_id (c_stringFile, "s_90");
				utils.removeScriptVar (player, "conversation.rek_thelcar.branchId");
				
				npcEndConversationWithMessage (player, message);
				
				return SCRIPT_CONTINUE;
			}
			
		}
		
		return SCRIPT_DEFAULT;
	}
	
	
	public int rek_thelcar_handleBranch34(obj_id player, obj_id npc, string_id response) throws InterruptedException
	{
		
		if (response.equals("s_93"))
		{
			
			if (rek_thelcar_condition__defaultCondition (player, npc))
			{
				doAnimationAction (npc, "shoo");
				
				string_id message = new string_id (c_stringFile, "s_95");
				utils.removeScriptVar (player, "conversation.rek_thelcar.branchId");
				
				npcEndConversationWithMessage (player, message);
				
				return SCRIPT_CONTINUE;
			}
			
		}
		
		return SCRIPT_DEFAULT;
	}
	
	
	public int rek_thelcar_handleBranch36(obj_id player, obj_id npc, string_id response) throws InterruptedException
	{
		
		if (response.equals("s_99"))
		{
			doAnimationAction (player, "shrug_shoulders");
			
			if (rek_thelcar_condition__defaultCondition (player, npc))
			{
				doAnimationAction (npc, "explain");
				
				doAnimationAction (player, "rub_chin_thoughtful");
				
				string_id message = new string_id (c_stringFile, "s_107");
				int numberOfResponses = 0;
				
				boolean hasResponse = false;
				
				boolean hasResponse0 = false;
				if (rek_thelcar_condition__defaultCondition (player, npc))
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
					
					utils.setScriptVar (player, "conversation.rek_thelcar.branchId", 37);
					
					npcSpeak (player, message);
					npcSetConversationResponses (player, responses);
				}
				else
				{
					utils.removeScriptVar (player, "conversation.rek_thelcar.branchId");
					
					npcEndConversationWithMessage (player, message);
				}
				
				return SCRIPT_CONTINUE;
			}
			
		}
		
		if (response.equals("s_135"))
		{
			doAnimationAction (player, "check_wrist_device");
			
			if (rek_thelcar_condition__defaultCondition (player, npc))
			{
				doAnimationAction (npc, "shrug_shoulders");
				
				string_id message = new string_id (c_stringFile, "s_139");
				utils.removeScriptVar (player, "conversation.rek_thelcar.branchId");
				
				npcEndConversationWithMessage (player, message);
				
				return SCRIPT_CONTINUE;
			}
			
		}
		
		return SCRIPT_DEFAULT;
	}
	
	
	public int rek_thelcar_handleBranch37(obj_id player, obj_id npc, string_id response) throws InterruptedException
	{
		
		if (response.equals("s_119"))
		{
			
			if (rek_thelcar_condition__defaultCondition (player, npc))
			{
				doAnimationAction (npc, "hands_behind_head");
				
				string_id message = new string_id (c_stringFile, "s_121");
				int numberOfResponses = 0;
				
				boolean hasResponse = false;
				
				boolean hasResponse0 = false;
				if (rek_thelcar_condition__defaultCondition (player, npc))
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
					
					utils.setScriptVar (player, "conversation.rek_thelcar.branchId", 38);
					
					npcSpeak (player, message);
					npcSetConversationResponses (player, responses);
				}
				else
				{
					utils.removeScriptVar (player, "conversation.rek_thelcar.branchId");
					
					npcEndConversationWithMessage (player, message);
				}
				
				return SCRIPT_CONTINUE;
			}
			
		}
		
		return SCRIPT_DEFAULT;
	}
	
	
	public int rek_thelcar_handleBranch38(obj_id player, obj_id npc, string_id response) throws InterruptedException
	{
		
		if (response.equals("s_123"))
		{
			
			if (rek_thelcar_condition__defaultCondition (player, npc))
			{
				doAnimationAction (npc, "laugh");
				
				string_id message = new string_id (c_stringFile, "s_125");
				int numberOfResponses = 0;
				
				boolean hasResponse = false;
				
				boolean hasResponse0 = false;
				if (rek_thelcar_condition__defaultCondition (player, npc))
				{
					++numberOfResponses;
					hasResponse = true;
					hasResponse0 = true;
				}
				
				boolean hasResponse1 = false;
				if (rek_thelcar_condition__defaultCondition (player, npc))
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
						responses[responseIndex++] = new string_id (c_stringFile, "s_131");
					}
					
					utils.setScriptVar (player, "conversation.rek_thelcar.branchId", 39);
					
					npcSpeak (player, message);
					npcSetConversationResponses (player, responses);
				}
				else
				{
					utils.removeScriptVar (player, "conversation.rek_thelcar.branchId");
					
					npcEndConversationWithMessage (player, message);
				}
				
				return SCRIPT_CONTINUE;
			}
			
		}
		
		return SCRIPT_DEFAULT;
	}
	
	
	public int rek_thelcar_handleBranch39(obj_id player, obj_id npc, string_id response) throws InterruptedException
	{
		
		if (response.equals("s_127"))
		{
			
			if (rek_thelcar_condition__defaultCondition (player, npc))
			{
				doAnimationAction (npc, "shoo");
				
				rek_thelcar_action_grantSurveillanceQuest (player, npc);
				
				string_id message = new string_id (c_stringFile, "s_129");
				utils.removeScriptVar (player, "conversation.rek_thelcar.branchId");
				
				npcEndConversationWithMessage (player, message);
				
				return SCRIPT_CONTINUE;
			}
			
		}
		
		if (response.equals("s_131"))
		{
			doAnimationAction (player, "check_wrist_device");
			
			if (rek_thelcar_condition__defaultCondition (player, npc))
			{
				doAnimationAction (npc, "shrug_shoulders");
				
				string_id message = new string_id (c_stringFile, "s_133");
				utils.removeScriptVar (player, "conversation.rek_thelcar.branchId");
				
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
			detachScript(self, "conversation.rek_thelcar");
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
		detachScript (self, "conversation.rek_thelcar");
		
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
		
		if (rek_thelcar_condition_hasReturnedQuestion (player, npc))
		{
			doAnimationAction (npc, "hair_flip");
			
			string_id message = new string_id (c_stringFile, "s_113");
			int numberOfResponses = 0;
			
			boolean hasResponse = false;
			
			boolean hasResponse0 = false;
			if (rek_thelcar_condition__defaultCondition (player, npc))
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
					responses[responseIndex++] = new string_id (c_stringFile, "s_114");
				}
				
				utils.setScriptVar (player, "conversation.rek_thelcar.branchId", 1);
				
				prose_package pp = new prose_package ();
				pp.stringId = message;
				pp.actor.set (player);
				pp.target.set (npc);
				
				npcStartConversation (player, npc, "rek_thelcar", null, pp, responses);
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
		
		if (rek_thelcar_condition_hasCompletedScientistCollection (player, npc))
		{
			rek_thelcar_action_BruteForceCompleteScientist (player, npc);
			
			string_id message = new string_id (c_stringFile, "s_142");
			chat.chat (npc, player, message);
			
			return SCRIPT_CONTINUE;
		}
		
		if (rek_thelcar_condition_hasQuestionNotComplete (player, npc))
		{
			doAnimationAction (npc, "applause_excited");
			
			string_id message = new string_id (c_stringFile, "s_85");
			int numberOfResponses = 0;
			
			boolean hasResponse = false;
			
			boolean hasResponse0 = false;
			if (rek_thelcar_condition__defaultCondition (player, npc))
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
					responses[responseIndex++] = new string_id (c_stringFile, "s_105");
				}
				
				utils.setScriptVar (player, "conversation.rek_thelcar.branchId", 5);
				
				prose_package pp = new prose_package ();
				pp.stringId = message;
				pp.actor.set (player);
				pp.target.set (npc);
				
				npcStartConversation (player, npc, "rek_thelcar", null, pp, responses);
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
		
		if (rek_thelcar_condition_isElligibleQuestion (player, npc))
		{
			doAnimationAction (npc, "greet");
			
			string_id message = new string_id (c_stringFile, "s_43");
			int numberOfResponses = 0;
			
			boolean hasResponse = false;
			
			boolean hasResponse0 = false;
			if (rek_thelcar_condition__defaultCondition (player, npc))
			{
				++numberOfResponses;
				hasResponse = true;
				hasResponse0 = true;
			}
			
			boolean hasResponse1 = false;
			if (rek_thelcar_condition__defaultCondition (player, npc))
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
					responses[responseIndex++] = new string_id (c_stringFile, "s_44");
				}
				
				if (hasResponse1)
				{
					responses[responseIndex++] = new string_id (c_stringFile, "s_80");
				}
				
				utils.setScriptVar (player, "conversation.rek_thelcar.branchId", 7);
				
				prose_package pp = new prose_package ();
				pp.stringId = message;
				pp.actor.set (player);
				pp.target.set (npc);
				
				npcStartConversation (player, npc, "rek_thelcar", null, pp, responses);
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
		
		if (rek_thelcar_condition_hasntDoneQuestion (player, npc))
		{
			
			string_id message = new string_id (c_stringFile, "s_137");
			chat.chat (npc, player, message);
			
			return SCRIPT_CONTINUE;
		}
		
		if (rek_thelcar_condition_hasReturnedMinerDna (player, npc))
		{
			doAnimationAction (npc, "bounce");
			
			string_id message = new string_id (c_stringFile, "s_108");
			int numberOfResponses = 0;
			
			boolean hasResponse = false;
			
			boolean hasResponse0 = false;
			if (rek_thelcar_condition__defaultCondition (player, npc))
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
				
				utils.setScriptVar (player, "conversation.rek_thelcar.branchId", 15);
				
				prose_package pp = new prose_package ();
				pp.stringId = message;
				pp.actor.set (player);
				pp.target.set (npc);
				
				npcStartConversation (player, npc, "rek_thelcar", null, pp, responses);
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
		
		if (rek_thelcar_condition_hasCompletedDNACollection (player, npc))
		{
			rek_thelcar_action_BruteForceCompleteDNA (player, npc);
			
			string_id message = new string_id (c_stringFile, "s_144");
			chat.chat (npc, player, message);
			
			return SCRIPT_CONTINUE;
		}
		
		if (rek_thelcar_condition_hasMinerDnaNotComplete (player, npc))
		{
			doAnimationAction (npc, "bounce");
			
			string_id message = new string_id (c_stringFile, "s_84");
			int numberOfResponses = 0;
			
			boolean hasResponse = false;
			
			boolean hasResponse0 = false;
			if (rek_thelcar_condition__defaultCondition (player, npc))
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
					responses[responseIndex++] = new string_id (c_stringFile, "s_102");
				}
				
				utils.setScriptVar (player, "conversation.rek_thelcar.branchId", 19);
				
				prose_package pp = new prose_package ();
				pp.stringId = message;
				pp.actor.set (player);
				pp.target.set (npc);
				
				npcStartConversation (player, npc, "rek_thelcar", null, pp, responses);
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
		
		if (rek_thelcar_condition_isElligibleMinerDna (player, npc))
		{
			doAnimationAction (npc, "greet");
			
			string_id message = new string_id (c_stringFile, "s_36");
			int numberOfResponses = 0;
			
			boolean hasResponse = false;
			
			boolean hasResponse0 = false;
			if (rek_thelcar_condition__defaultCondition (player, npc))
			{
				++numberOfResponses;
				hasResponse = true;
				hasResponse0 = true;
			}
			
			boolean hasResponse1 = false;
			if (rek_thelcar_condition__defaultCondition (player, npc))
			{
				++numberOfResponses;
				hasResponse = true;
				hasResponse1 = true;
			}
			
			boolean hasResponse2 = false;
			if (rek_thelcar_condition__defaultCondition (player, npc))
			{
				++numberOfResponses;
				hasResponse = true;
				hasResponse2 = true;
			}
			
			if (hasResponse)
			{
				int responseIndex = 0;
				string_id responses[] = new string_id[numberOfResponses];
				
				if (hasResponse0)
				{
					responses[responseIndex++] = new string_id (c_stringFile, "s_38");
				}
				
				if (hasResponse1)
				{
					responses[responseIndex++] = new string_id (c_stringFile, "s_41");
				}
				
				if (hasResponse2)
				{
					responses[responseIndex++] = new string_id (c_stringFile, "s_79");
				}
				
				utils.setScriptVar (player, "conversation.rek_thelcar.branchId", 21);
				
				prose_package pp = new prose_package ();
				pp.stringId = message;
				pp.actor.set (player);
				pp.target.set (npc);
				
				npcStartConversation (player, npc, "rek_thelcar", null, pp, responses);
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
		
		if (rek_thelcar_condition_hasntDoneMinerDNA (player, npc))
		{
			
			string_id message = new string_id (c_stringFile, "s_136");
			chat.chat (npc, player, message);
			
			return SCRIPT_CONTINUE;
		}
		
		if (rek_thelcar_condition_hasReturnedSurveillance (player, npc))
		{
			doAnimationAction (npc, "greet");
			
			string_id message = new string_id (c_stringFile, "s_86");
			int numberOfResponses = 0;
			
			boolean hasResponse = false;
			
			boolean hasResponse0 = false;
			if (rek_thelcar_condition__defaultCondition (player, npc))
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
				
				utils.setScriptVar (player, "conversation.rek_thelcar.branchId", 30);
				
				prose_package pp = new prose_package ();
				pp.stringId = message;
				pp.actor.set (player);
				pp.target.set (npc);
				
				npcStartConversation (player, npc, "rek_thelcar", null, pp, responses);
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
		
		if (rek_thelcar_condition_hasCompletedDeviceCollection (player, npc))
		{
			rek_thelcar_action_BruteForceCompleteDevice (player, npc);
			
			string_id message = new string_id (c_stringFile, "s_146");
			chat.chat (npc, player, message);
			
			return SCRIPT_CONTINUE;
		}
		
		if (rek_thelcar_condition_hasSurveillanceNotComplete (player, npc))
		{
			doAnimationAction (npc, "applause_excited");
			
			string_id message = new string_id (c_stringFile, "s_91");
			int numberOfResponses = 0;
			
			boolean hasResponse = false;
			
			boolean hasResponse0 = false;
			if (rek_thelcar_condition__defaultCondition (player, npc))
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
					responses[responseIndex++] = new string_id (c_stringFile, "s_93");
				}
				
				utils.setScriptVar (player, "conversation.rek_thelcar.branchId", 34);
				
				prose_package pp = new prose_package ();
				pp.stringId = message;
				pp.actor.set (player);
				pp.target.set (npc);
				
				npcStartConversation (player, npc, "rek_thelcar", null, pp, responses);
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
		
		if (rek_thelcar_condition_isEligibleSurveillance (player, npc))
		{
			doAnimationAction (npc, "beckon");
			
			string_id message = new string_id (c_stringFile, "s_97");
			int numberOfResponses = 0;
			
			boolean hasResponse = false;
			
			boolean hasResponse0 = false;
			if (rek_thelcar_condition__defaultCondition (player, npc))
			{
				++numberOfResponses;
				hasResponse = true;
				hasResponse0 = true;
			}
			
			boolean hasResponse1 = false;
			if (rek_thelcar_condition__defaultCondition (player, npc))
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
					responses[responseIndex++] = new string_id (c_stringFile, "s_99");
				}
				
				if (hasResponse1)
				{
					responses[responseIndex++] = new string_id (c_stringFile, "s_135");
				}
				
				utils.setScriptVar (player, "conversation.rek_thelcar.branchId", 36);
				
				prose_package pp = new prose_package ();
				pp.stringId = message;
				pp.actor.set (player);
				pp.target.set (npc);
				
				npcStartConversation (player, npc, "rek_thelcar", null, pp, responses);
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
		
		if (rek_thelcar_condition_hasntDoneSurveillance (player, npc))
		{
			
			string_id message = new string_id (c_stringFile, "s_141");
			chat.chat (npc, player, message);
			
			return SCRIPT_CONTINUE;
		}
		
		if (rek_thelcar_condition__defaultCondition (player, npc))
		{
			doAnimationAction (npc, "hi5_tandem");
			
			string_id message = new string_id (c_stringFile, "s_145");
			chat.chat (npc, player, message);
			
			return SCRIPT_CONTINUE;
		}
		
		chat.chat (npc, "Error: All conditions for OnStartNpcConversation were false.");
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int OnNpcConversationResponse(obj_id self, String conversationId, obj_id player, string_id response) throws InterruptedException
	{
		if (!conversationId.equals("rek_thelcar"))
		{
			return SCRIPT_CONTINUE;
		}
		
		obj_id npc = self;
		
		int branchId = utils.getIntScriptVar (player, "conversation.rek_thelcar.branchId");
		
		if (branchId == 1 && rek_thelcar_handleBranch1 (player, npc, response) == SCRIPT_CONTINUE)
		{
			return SCRIPT_CONTINUE;
		}
		
		if (branchId == 2 && rek_thelcar_handleBranch2 (player, npc, response) == SCRIPT_CONTINUE)
		{
			return SCRIPT_CONTINUE;
		}
		
		if (branchId == 5 && rek_thelcar_handleBranch5 (player, npc, response) == SCRIPT_CONTINUE)
		{
			return SCRIPT_CONTINUE;
		}
		
		if (branchId == 7 && rek_thelcar_handleBranch7 (player, npc, response) == SCRIPT_CONTINUE)
		{
			return SCRIPT_CONTINUE;
		}
		
		if (branchId == 8 && rek_thelcar_handleBranch8 (player, npc, response) == SCRIPT_CONTINUE)
		{
			return SCRIPT_CONTINUE;
		}
		
		if (branchId == 9 && rek_thelcar_handleBranch9 (player, npc, response) == SCRIPT_CONTINUE)
		{
			return SCRIPT_CONTINUE;
		}
		
		if (branchId == 10 && rek_thelcar_handleBranch10 (player, npc, response) == SCRIPT_CONTINUE)
		{
			return SCRIPT_CONTINUE;
		}
		
		if (branchId == 15 && rek_thelcar_handleBranch15 (player, npc, response) == SCRIPT_CONTINUE)
		{
			return SCRIPT_CONTINUE;
		}
		
		if (branchId == 16 && rek_thelcar_handleBranch16 (player, npc, response) == SCRIPT_CONTINUE)
		{
			return SCRIPT_CONTINUE;
		}
		
		if (branchId == 19 && rek_thelcar_handleBranch19 (player, npc, response) == SCRIPT_CONTINUE)
		{
			return SCRIPT_CONTINUE;
		}
		
		if (branchId == 21 && rek_thelcar_handleBranch21 (player, npc, response) == SCRIPT_CONTINUE)
		{
			return SCRIPT_CONTINUE;
		}
		
		if (branchId == 22 && rek_thelcar_handleBranch22 (player, npc, response) == SCRIPT_CONTINUE)
		{
			return SCRIPT_CONTINUE;
		}
		
		if (branchId == 23 && rek_thelcar_handleBranch23 (player, npc, response) == SCRIPT_CONTINUE)
		{
			return SCRIPT_CONTINUE;
		}
		
		if (branchId == 24 && rek_thelcar_handleBranch24 (player, npc, response) == SCRIPT_CONTINUE)
		{
			return SCRIPT_CONTINUE;
		}
		
		if (branchId == 25 && rek_thelcar_handleBranch25 (player, npc, response) == SCRIPT_CONTINUE)
		{
			return SCRIPT_CONTINUE;
		}
		
		if (branchId == 30 && rek_thelcar_handleBranch30 (player, npc, response) == SCRIPT_CONTINUE)
		{
			return SCRIPT_CONTINUE;
		}
		
		if (branchId == 31 && rek_thelcar_handleBranch31 (player, npc, response) == SCRIPT_CONTINUE)
		{
			return SCRIPT_CONTINUE;
		}
		
		if (branchId == 34 && rek_thelcar_handleBranch34 (player, npc, response) == SCRIPT_CONTINUE)
		{
			return SCRIPT_CONTINUE;
		}
		
		if (branchId == 36 && rek_thelcar_handleBranch36 (player, npc, response) == SCRIPT_CONTINUE)
		{
			return SCRIPT_CONTINUE;
		}
		
		if (branchId == 37 && rek_thelcar_handleBranch37 (player, npc, response) == SCRIPT_CONTINUE)
		{
			return SCRIPT_CONTINUE;
		}
		
		if (branchId == 38 && rek_thelcar_handleBranch38 (player, npc, response) == SCRIPT_CONTINUE)
		{
			return SCRIPT_CONTINUE;
		}
		
		if (branchId == 39 && rek_thelcar_handleBranch39 (player, npc, response) == SCRIPT_CONTINUE)
		{
			return SCRIPT_CONTINUE;
		}
		
		chat.chat (npc, "Error: Fell through all branches and responses for OnNpcConversationResponse.");
		
		utils.removeScriptVar (player, "conversation.rek_thelcar.branchId");
		
		return SCRIPT_CONTINUE;
	}
	
}
