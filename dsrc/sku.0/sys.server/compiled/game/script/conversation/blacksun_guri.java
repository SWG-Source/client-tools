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
import script.library.factions;
import script.library.groundquests;
import script.library.utils;


public class blacksun_guri extends script.base_script
{
	public blacksun_guri()
	{
	}
	String c_stringFile = "conversation/blacksun_guri";
	
	
	public boolean blacksun_guri_condition__defaultCondition(obj_id player, obj_id npc) throws InterruptedException
	{
		return true;
	}
	
	
	public boolean blacksun_guri_condition_hasNotStartedQuestLine(obj_id player, obj_id npc) throws InterruptedException
	{
		faceTo(npc, player);
		return (!groundquests.isQuestActiveOrComplete(player, "outbreak_quest_01_imperial") && !groundquests.isQuestActiveOrComplete(player, "outbreak_quest_01_rebel") && !groundquests.isQuestActiveOrComplete(player, "outbreak_quest_01_neutral"));
	}
	
	
	public boolean blacksun_guri_condition_hasFirstQuestNeutral(obj_id player, obj_id npc) throws InterruptedException
	{
		faceTo(npc, player);
		return groundquests.isQuestActive(player, "outbreak_quest_01_neutral");
	}
	
	
	public boolean blacksun_guri_condition_hasSecondQuestImperial(obj_id player, obj_id npc) throws InterruptedException
	{
		faceTo(npc, player);
		return groundquests.isQuestActive(player, "outbreak_quest_02_neutral");
	}
	
	
	public boolean blacksun_guri_condition_hasFoundStormtroopers(obj_id player, obj_id npc) throws InterruptedException
	{
		faceTo(npc, player);
		return groundquests.hasCompletedQuest(player, "outbreak_quest_02_neutral");
	}
	
	
	public boolean blacksun_guri_condition_hasCompletedMissions(obj_id player, obj_id npc) throws InterruptedException
	{
		return groundquests.isTaskActive(player, "outbreak_quest_facility_05_neutral", "speakPietteDebrief");
	}
	
	
	public boolean blacksun_guri_condition_hasCompletedAll(obj_id player, obj_id npc) throws InterruptedException
	{
		return groundquests.hasCompletedQuest(player, "outbreak_quest_facility_05_neutral") && groundquests.hasCompletedQuest(player, "outbreak_quest_final_neutral");
	}
	
	
	public boolean blacksun_guri_condition_isLiveConversion(obj_id player, obj_id npc) throws InterruptedException
	{
		return blacksun_guri_condition_hasNotStartedQuestLine(player,npc) && (groundquests.isTaskActive(player, "outbreak_live_conversion_neutral", "speakGuri") || groundquests.hasCompletedQuest(player, "outbreak_live_conversion_neutral")) || (groundquests.isTaskActive(player, "outbreak_switch_to_neutral", "speakGuri") || groundquests.hasCompletedQuest(player, "outbreak_switch_to_neutral"));
	}
	
	
	public boolean blacksun_guri_condition_isPlayerRebel(obj_id player, obj_id npc) throws InterruptedException
	{
		return factions.isRebel(player);
	}
	
	
	public boolean blacksun_guri_condition_isPlayerImp(obj_id player, obj_id npc) throws InterruptedException
	{
		return factions.isImperial(player);
	}
	
	
	public boolean blacksun_guri_condition_hasDeletedLiveConversion(obj_id player, obj_id npc) throws InterruptedException
	{
		if ((groundquests.hasCompletedQuest(player, "quest_08_dathomir_outpost_final") || groundquests.hasCompletedQuest(player, "quest_08_dathomir_outpost")))
		{
			if ((!groundquests.isQuestActiveOrComplete(player, "outbreak_switch_to_rebel") && !groundquests.isQuestActiveOrComplete(player, "outbreak_switch_to_imperial") && !groundquests.isQuestActiveOrComplete(player, "outbreak_switch_to_neutral") && !groundquests.isQuestActiveOrComplete(player, "outbreak_live_conversion_neutral") && !groundquests.isQuestActiveOrComplete(player, "outbreak_live_conversion_rebel") && !groundquests.isQuestActiveOrComplete(player, "outbreak_live_conversion_imperial")))
			{
				return true;
			}
		}
		return false;
	}
	
	
	public void blacksun_guri_action_grantMissionOne(obj_id player, obj_id npc) throws InterruptedException
	{
		faceTo(npc, player);
		groundquests.grantQuest(player, "quest/outbreak_quest_01_neutral");
	}
	
	
	public void blacksun_guri_action_completeQuest5(obj_id player, obj_id npc) throws InterruptedException
	{
		groundquests.sendSignal(player, "hasSpokenPietteDebreif");
		if (!hasCompletedCollectionSlot(player, "outbreak_flare_s_slot"))
		{
			modifyCollectionSlotValue(player, "outbreak_flare_s_slot", 1);
		}
	}
	
	
	public void blacksun_guri_action_clearLiveConversion(obj_id player, obj_id npc) throws InterruptedException
	{
		if ((!groundquests.isTaskActive(player, "outbreak_live_conversion_neutral", "speakGuri") && !groundquests.isTaskActive(player, "outbreak_switch_to_neutral", "speakGuri")))
		{
			return;
		}
		
		groundquests.sendSignal(player, "liveConversionSpokeGuri");
	}
	
	
	public void blacksun_guri_action_revokeNeutralGiveImperial(obj_id player, obj_id npc) throws InterruptedException
	{
		if (groundquests.isQuestActiveOrComplete(player, "outbreak_live_conversion_neutral"))
		{
			groundquests.clearQuest(player, "outbreak_live_conversion_neutral");
			groundquests.grantQuest(player, "outbreak_switch_to_imperial");
		}
		if (groundquests.isQuestActiveOrComplete(player, "outbreak_switch_to_neutral"))
		{
			groundquests.clearQuest(player, "outbreak_switch_to_neutral");
			groundquests.grantQuest(player, "outbreak_switch_to_imperial");
		}
		
	}
	
	
	public void blacksun_guri_action_revokeNeutralGiveRebel(obj_id player, obj_id npc) throws InterruptedException
	{
		if (groundquests.isQuestActiveOrComplete(player, "outbreak_live_conversion_neutral"))
		{
			groundquests.clearQuest(player, "outbreak_live_conversion_neutral");
			groundquests.grantQuest(player, "outbreak_switch_to_rebel");
		}
		if (groundquests.isQuestActiveOrComplete(player, "outbreak_switch_to_neutral"))
		{
			groundquests.clearQuest(player, "outbreak_switch_to_neutral");
			groundquests.grantQuest(player, "outbreak_switch_to_rebel");
		}
	}
	
	
	public void blacksun_guri_action_revokeEntireNeutQuestLine(obj_id player, obj_id npc) throws InterruptedException
	{
		if (groundquests.isQuestActiveOrComplete(player, "outbreak_live_conversion_neutral"))
		{
			groundquests.clearQuest(player, "outbreak_live_conversion_neutral");
		}
		
		if (groundquests.isQuestActiveOrComplete(player, "outbreak_switch_to_neutral"))
		{
			groundquests.clearQuest(player, "outbreak_switch_to_neutral");
		}
		
		if (groundquests.isQuestActiveOrComplete(player, "outbreak_quest_01_neutral"))
		{
			groundquests.clearQuest(player, "outbreak_quest_01_neutral");
		}
		
		if (groundquests.isQuestActiveOrComplete(player, "outbreak_quest_02_neutral"))
		{
			groundquests.clearQuest(player, "outbreak_quest_02_neutral");
		}
		
		if (groundquests.isQuestActiveOrComplete(player, "outbreak_quest_rescue_01"))
		{
			groundquests.clearQuest(player, "outbreak_quest_rescue_01");
		}
		
		if (groundquests.isQuestActiveOrComplete(player, "outbreak_quest_rescue_01b"))
		{
			groundquests.clearQuest(player, "outbreak_quest_rescue_01b");
		}
		
		if (groundquests.isQuestActiveOrComplete(player, "outbreak_quest_rescue_01c"))
		{
			groundquests.clearQuest(player, "outbreak_quest_rescue_01c");
		}
		
		if (groundquests.isQuestActiveOrComplete(player, "outbreak_quest_rescue_01d"))
		{
			groundquests.clearQuest(player, "outbreak_quest_rescue_01d");
		}
		
		if (groundquests.isQuestActiveOrComplete(player, "outbreak_quest_rescue_02"))
		{
			groundquests.clearQuest(player, "outbreak_quest_rescue_02");
		}
		
		if (groundquests.isQuestActiveOrComplete(player, "outbreak_quest_rescue_03"))
		{
			groundquests.clearQuest(player, "outbreak_quest_rescue_03");
		}
		
		if (groundquests.isQuestActiveOrComplete(player, "outbreak_quest_rescue_04"))
		{
			groundquests.clearQuest(player, "outbreak_quest_rescue_04");
		}
		
		if (groundquests.isQuestActiveOrComplete(player, "outbreak_quest_rescue_05"))
		{
			groundquests.clearQuest(player, "outbreak_quest_rescue_05");
		}
		
		if (groundquests.isQuestActiveOrComplete(player, "outbreak_quest_rescue_06"))
		{
			groundquests.clearQuest(player, "outbreak_quest_rescue_06");
		}
		
		if (groundquests.isQuestActiveOrComplete(player, "outbreak_quest_rescue_07"))
		{
			groundquests.clearQuest(player, "outbreak_quest_rescue_07");
		}
		
		if (groundquests.isQuestActiveOrComplete(player, "outbreak_quest_rescue_08"))
		{
			groundquests.clearQuest(player, "outbreak_quest_rescue_08");
		}
		
		if (groundquests.isQuestActiveOrComplete(player, "outbreak_quest_rescue_09"))
		{
			groundquests.clearQuest(player, "outbreak_quest_rescue_09");
		}
		
		if (groundquests.isQuestActiveOrComplete(player, "outbreak_quest_rescue_10"))
		{
			groundquests.clearQuest(player, "outbreak_quest_rescue_10");
		}
		
		if (groundquests.isQuestActiveOrComplete(player, "outbreak_quest_rescue_11"))
		{
			groundquests.clearQuest(player, "outbreak_quest_rescue_11");
		}
		
		if (groundquests.isQuestActiveOrComplete(player, "outbreak_quest_rescue_12"))
		{
			groundquests.clearQuest(player, "outbreak_quest_rescue_12");
		}
		
		if (groundquests.isQuestActiveOrComplete(player, "outbreak_quest_rescue_13"))
		{
			groundquests.clearQuest(player, "outbreak_quest_rescue_13");
		}
		
		if (groundquests.isQuestActiveOrComplete(player, "outbreak_quest_rescue_14"))
		{
			groundquests.clearQuest(player, "outbreak_quest_rescue_14");
		}
		
		if (groundquests.isQuestActiveOrComplete(player, "outbreak_quest_rescue_15"))
		{
			groundquests.clearQuest(player, "outbreak_quest_rescue_15");
		}
		
		if (groundquests.isQuestActiveOrComplete(player, "outbreak_quest_rescue_16"))
		{
			groundquests.clearQuest(player, "outbreak_quest_rescue_16");
		}
		
		if (groundquests.isQuestActiveOrComplete(player, "outbreak_quest_shuttle_parts"))
		{
			groundquests.clearQuest(player, "outbreak_quest_shuttle_parts");
		}
		
		if (groundquests.isQuestActiveOrComplete(player, "outbreak_quest_administrative_building_neutral_01"))
		{
			groundquests.clearQuest(player, "outbreak_quest_administrative_building_neutral_01");
		}
		
		if (groundquests.isQuestActiveOrComplete(player, "outbreak_quest_administrative_building_neutral_02"))
		{
			groundquests.clearQuest(player, "outbreak_quest_administrative_building_neutral_02");
		}
		
		if (groundquests.isQuestActiveOrComplete(player, "outbreak_quest_administrative_building_neutral_03"))
		{
			groundquests.clearQuest(player, "outbreak_quest_administrative_building_neutral_03");
		}
		
		if (groundquests.isQuestActiveOrComplete(player, "outbreak_quest_administrative_building_neutral_04"))
		{
			groundquests.clearQuest(player, "outbreak_quest_administrative_building_neutral_04");
		}
		
		if (groundquests.isQuestActiveOrComplete(player, "outbreak_quest_facility_01"))
		{
			groundquests.clearQuest(player, "outbreak_quest_facility_01");
		}
		
		if (groundquests.isQuestActiveOrComplete(player, "outbreak_quest_facility_02"))
		{
			groundquests.clearQuest(player, "outbreak_quest_facility_02");
		}
		
		if (groundquests.isQuestActiveOrComplete(player, "outbreak_quest_facility_02_neutral"))
		{
			groundquests.clearQuest(player, "outbreak_quest_facility_02_neutral");
		}
		
		if (groundquests.isQuestActiveOrComplete(player, "outbreak_quest_facility_03"))
		{
			groundquests.clearQuest(player, "outbreak_quest_facility_03");
		}
		
		if (groundquests.isQuestActiveOrComplete(player, "outbreak_quest_facility_04"))
		{
			groundquests.clearQuest(player, "outbreak_quest_facility_04");
		}
		
		if (groundquests.isQuestActiveOrComplete(player, "outbreak_quest_facility_05_neutral"))
		{
			groundquests.clearQuest(player, "outbreak_quest_facility_05_neutral");
		}
		
		if (groundquests.isQuestActiveOrComplete(player, "outbreak_quest_final_neutral"))
		{
			groundquests.clearQuest(player, "outbreak_quest_final_neutral");
		}
		
	}
	
	
	public void blacksun_guri_action_grantRebQuest(obj_id player, obj_id npc) throws InterruptedException
	{
		groundquests.grantQuest(player, "outbreak_switch_to_rebel");
	}
	
	
	public void blacksun_guri_action_grantImpQuest(obj_id player, obj_id npc) throws InterruptedException
	{
		groundquests.grantQuest(player, "outbreak_switch_to_imperial");
	}
	
	
	public void blacksun_guri_action_grantNeutQuest(obj_id player, obj_id npc) throws InterruptedException
	{
		groundquests.grantQuest(player, "outbreak_switch_to_neutral");
	}
	
	
	public int blacksun_guri_handleBranch1(obj_id player, obj_id npc, string_id response) throws InterruptedException
	{
		
		if (response.equals("s_95"))
		{
			
			if (blacksun_guri_condition__defaultCondition (player, npc))
			{
				
				string_id message = new string_id (c_stringFile, "s_96");
				int numberOfResponses = 0;
				
				boolean hasResponse = false;
				
				boolean hasResponse0 = false;
				if (blacksun_guri_condition__defaultCondition (player, npc))
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
						responses[responseIndex++] = new string_id (c_stringFile, "s_97");
					}
					
					utils.setScriptVar (player, "conversation.blacksun_guri.branchId", 2);
					
					npcSpeak (player, message);
					npcSetConversationResponses (player, responses);
				}
				else
				{
					utils.removeScriptVar (player, "conversation.blacksun_guri.branchId");
					
					npcEndConversationWithMessage (player, message);
				}
				
				return SCRIPT_CONTINUE;
			}
			
		}
		
		return SCRIPT_DEFAULT;
	}
	
	
	public int blacksun_guri_handleBranch2(obj_id player, obj_id npc, string_id response) throws InterruptedException
	{
		
		if (response.equals("s_97"))
		{
			
			if (blacksun_guri_condition__defaultCondition (player, npc))
			{
				
				string_id message = new string_id (c_stringFile, "s_98");
				int numberOfResponses = 0;
				
				boolean hasResponse = false;
				
				boolean hasResponse0 = false;
				if (blacksun_guri_condition__defaultCondition (player, npc))
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
					
					utils.setScriptVar (player, "conversation.blacksun_guri.branchId", 3);
					
					prose_package pp = new prose_package ();
					pp.stringId = message;
					pp.actor.set (player);
					pp.target.set (npc);
					
					npcSpeak (player, pp);
					npcSetConversationResponses (player, responses);
				}
				else
				{
					utils.removeScriptVar (player, "conversation.blacksun_guri.branchId");
					
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
	
	
	public int blacksun_guri_handleBranch3(obj_id player, obj_id npc, string_id response) throws InterruptedException
	{
		
		if (response.equals("s_99"))
		{
			
			if (blacksun_guri_condition__defaultCondition (player, npc))
			{
				blacksun_guri_action_revokeEntireNeutQuestLine (player, npc);
				
				string_id message = new string_id (c_stringFile, "s_100");
				int numberOfResponses = 0;
				
				boolean hasResponse = false;
				
				boolean hasResponse0 = false;
				if (blacksun_guri_condition__defaultCondition (player, npc))
				{
					++numberOfResponses;
					hasResponse = true;
					hasResponse0 = true;
				}
				
				boolean hasResponse1 = false;
				if (blacksun_guri_condition__defaultCondition (player, npc))
				{
					++numberOfResponses;
					hasResponse = true;
					hasResponse1 = true;
				}
				
				boolean hasResponse2 = false;
				if (blacksun_guri_condition__defaultCondition (player, npc))
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
						responses[responseIndex++] = new string_id (c_stringFile, "s_101");
					}
					
					if (hasResponse1)
					{
						responses[responseIndex++] = new string_id (c_stringFile, "s_102");
					}
					
					if (hasResponse2)
					{
						responses[responseIndex++] = new string_id (c_stringFile, "s_103");
					}
					
					utils.setScriptVar (player, "conversation.blacksun_guri.branchId", 4);
					
					npcSpeak (player, message);
					npcSetConversationResponses (player, responses);
				}
				else
				{
					utils.removeScriptVar (player, "conversation.blacksun_guri.branchId");
					
					npcEndConversationWithMessage (player, message);
				}
				
				return SCRIPT_CONTINUE;
			}
			
		}
		
		return SCRIPT_DEFAULT;
	}
	
	
	public int blacksun_guri_handleBranch4(obj_id player, obj_id npc, string_id response) throws InterruptedException
	{
		
		if (response.equals("s_101"))
		{
			
			if (blacksun_guri_condition__defaultCondition (player, npc))
			{
				blacksun_guri_action_grantNeutQuest (player, npc);
				
				string_id message = new string_id (c_stringFile, "s_104");
				utils.removeScriptVar (player, "conversation.blacksun_guri.branchId");
				
				npcEndConversationWithMessage (player, message);
				
				return SCRIPT_CONTINUE;
			}
			
		}
		
		if (response.equals("s_102"))
		{
			
			if (blacksun_guri_condition__defaultCondition (player, npc))
			{
				blacksun_guri_action_grantRebQuest (player, npc);
				
				string_id message = new string_id (c_stringFile, "s_105");
				utils.removeScriptVar (player, "conversation.blacksun_guri.branchId");
				
				npcEndConversationWithMessage (player, message);
				
				return SCRIPT_CONTINUE;
			}
			
		}
		
		if (response.equals("s_103"))
		{
			
			if (blacksun_guri_condition__defaultCondition (player, npc))
			{
				blacksun_guri_action_grantImpQuest (player, npc);
				
				string_id message = new string_id (c_stringFile, "s_106");
				utils.removeScriptVar (player, "conversation.blacksun_guri.branchId");
				
				npcEndConversationWithMessage (player, message);
				
				return SCRIPT_CONTINUE;
			}
			
		}
		
		return SCRIPT_DEFAULT;
	}
	
	
	public int blacksun_guri_handleBranch8(obj_id player, obj_id npc, string_id response) throws InterruptedException
	{
		
		if (response.equals("s_124"))
		{
			
			if (blacksun_guri_condition__defaultCondition (player, npc))
			{
				doAnimationAction (npc, "handshake_tandem");
				
				doAnimationAction (player, "handshake_tandem");
				
				string_id message = new string_id (c_stringFile, "s_125");
				int numberOfResponses = 0;
				
				boolean hasResponse = false;
				
				boolean hasResponse0 = false;
				if (blacksun_guri_condition__defaultCondition (player, npc))
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
						responses[responseIndex++] = new string_id (c_stringFile, "s_126");
					}
					
					utils.setScriptVar (player, "conversation.blacksun_guri.branchId", 9);
					
					npcSpeak (player, message);
					npcSetConversationResponses (player, responses);
				}
				else
				{
					utils.removeScriptVar (player, "conversation.blacksun_guri.branchId");
					
					npcEndConversationWithMessage (player, message);
				}
				
				return SCRIPT_CONTINUE;
			}
			
		}
		
		return SCRIPT_DEFAULT;
	}
	
	
	public int blacksun_guri_handleBranch9(obj_id player, obj_id npc, string_id response) throws InterruptedException
	{
		
		if (response.equals("s_126"))
		{
			
			if (blacksun_guri_condition__defaultCondition (player, npc))
			{
				
				string_id message = new string_id (c_stringFile, "s_127");
				int numberOfResponses = 0;
				
				boolean hasResponse = false;
				
				boolean hasResponse0 = false;
				if (blacksun_guri_condition__defaultCondition (player, npc))
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
						responses[responseIndex++] = new string_id (c_stringFile, "s_128");
					}
					
					utils.setScriptVar (player, "conversation.blacksun_guri.branchId", 10);
					
					prose_package pp = new prose_package ();
					pp.stringId = message;
					pp.actor.set (player);
					pp.target.set (npc);
					
					npcSpeak (player, pp);
					npcSetConversationResponses (player, responses);
				}
				else
				{
					utils.removeScriptVar (player, "conversation.blacksun_guri.branchId");
					
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
	
	
	public int blacksun_guri_handleBranch10(obj_id player, obj_id npc, string_id response) throws InterruptedException
	{
		
		if (response.equals("s_128"))
		{
			
			if (blacksun_guri_condition__defaultCondition (player, npc))
			{
				doAnimationAction (npc, "nod");
				
				blacksun_guri_action_completeQuest5 (player, npc);
				
				string_id message = new string_id (c_stringFile, "s_129");
				utils.removeScriptVar (player, "conversation.blacksun_guri.branchId");
				
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
	
	
	public int blacksun_guri_handleBranch15(obj_id player, obj_id npc, string_id response) throws InterruptedException
	{
		
		if (response.equals("s_29"))
		{
			
			if (blacksun_guri_condition_isPlayerRebel (player, npc))
			{
				
				string_id message = new string_id (c_stringFile, "s_71");
				int numberOfResponses = 0;
				
				boolean hasResponse = false;
				
				boolean hasResponse0 = false;
				if (blacksun_guri_condition__defaultCondition (player, npc))
				{
					++numberOfResponses;
					hasResponse = true;
					hasResponse0 = true;
				}
				
				boolean hasResponse1 = false;
				if (blacksun_guri_condition__defaultCondition (player, npc))
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
						responses[responseIndex++] = new string_id (c_stringFile, "s_72");
					}
					
					if (hasResponse1)
					{
						responses[responseIndex++] = new string_id (c_stringFile, "s_73");
					}
					
					utils.setScriptVar (player, "conversation.blacksun_guri.branchId", 16);
					
					npcSpeak (player, message);
					npcSetConversationResponses (player, responses);
				}
				else
				{
					utils.removeScriptVar (player, "conversation.blacksun_guri.branchId");
					
					npcEndConversationWithMessage (player, message);
				}
				
				return SCRIPT_CONTINUE;
			}
			
			if (blacksun_guri_condition_isPlayerImp (player, npc))
			{
				
				string_id message = new string_id (c_stringFile, "s_75");
				int numberOfResponses = 0;
				
				boolean hasResponse = false;
				
				boolean hasResponse0 = false;
				if (blacksun_guri_condition__defaultCondition (player, npc))
				{
					++numberOfResponses;
					hasResponse = true;
					hasResponse0 = true;
				}
				
				boolean hasResponse1 = false;
				if (blacksun_guri_condition__defaultCondition (player, npc))
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
						responses[responseIndex++] = new string_id (c_stringFile, "s_76");
					}
					
					if (hasResponse1)
					{
						responses[responseIndex++] = new string_id (c_stringFile, "s_77");
					}
					
					utils.setScriptVar (player, "conversation.blacksun_guri.branchId", 21);
					
					npcSpeak (player, message);
					npcSetConversationResponses (player, responses);
				}
				else
				{
					utils.removeScriptVar (player, "conversation.blacksun_guri.branchId");
					
					npcEndConversationWithMessage (player, message);
				}
				
				return SCRIPT_CONTINUE;
			}
			
			if (blacksun_guri_condition__defaultCondition (player, npc))
			{
				
				string_id message = new string_id (c_stringFile, "s_49");
				int numberOfResponses = 0;
				
				boolean hasResponse = false;
				
				boolean hasResponse0 = false;
				if (blacksun_guri_condition__defaultCondition (player, npc))
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
					
					utils.setScriptVar (player, "conversation.blacksun_guri.branchId", 26);
					
					npcSpeak (player, message);
					npcSetConversationResponses (player, responses);
				}
				else
				{
					utils.removeScriptVar (player, "conversation.blacksun_guri.branchId");
					
					npcEndConversationWithMessage (player, message);
				}
				
				return SCRIPT_CONTINUE;
			}
			
		}
		
		return SCRIPT_DEFAULT;
	}
	
	
	public int blacksun_guri_handleBranch16(obj_id player, obj_id npc, string_id response) throws InterruptedException
	{
		
		if (response.equals("s_72"))
		{
			
			if (blacksun_guri_condition__defaultCondition (player, npc))
			{
				
				string_id message = new string_id (c_stringFile, "s_89");
				int numberOfResponses = 0;
				
				boolean hasResponse = false;
				
				boolean hasResponse0 = false;
				if (blacksun_guri_condition__defaultCondition (player, npc))
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
					
					utils.setScriptVar (player, "conversation.blacksun_guri.branchId", 17);
					
					npcSpeak (player, message);
					npcSetConversationResponses (player, responses);
				}
				else
				{
					utils.removeScriptVar (player, "conversation.blacksun_guri.branchId");
					
					npcEndConversationWithMessage (player, message);
				}
				
				return SCRIPT_CONTINUE;
			}
			
		}
		
		if (response.equals("s_73"))
		{
			
			if (blacksun_guri_condition__defaultCondition (player, npc))
			{
				
				string_id message = new string_id (c_stringFile, "s_84");
				int numberOfResponses = 0;
				
				boolean hasResponse = false;
				
				boolean hasResponse0 = false;
				if (blacksun_guri_condition__defaultCondition (player, npc))
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
						responses[responseIndex++] = new string_id (c_stringFile, "s_85");
					}
					
					utils.setScriptVar (player, "conversation.blacksun_guri.branchId", 19);
					
					npcSpeak (player, message);
					npcSetConversationResponses (player, responses);
				}
				else
				{
					utils.removeScriptVar (player, "conversation.blacksun_guri.branchId");
					
					npcEndConversationWithMessage (player, message);
				}
				
				return SCRIPT_CONTINUE;
			}
			
		}
		
		return SCRIPT_DEFAULT;
	}
	
	
	public int blacksun_guri_handleBranch17(obj_id player, obj_id npc, string_id response) throws InterruptedException
	{
		
		if (response.equals("s_90"))
		{
			
			if (blacksun_guri_condition__defaultCondition (player, npc))
			{
				blacksun_guri_action_revokeNeutralGiveRebel (player, npc);
				
				string_id message = new string_id (c_stringFile, "s_91");
				utils.removeScriptVar (player, "conversation.blacksun_guri.branchId");
				
				npcEndConversationWithMessage (player, message);
				
				return SCRIPT_CONTINUE;
			}
			
		}
		
		return SCRIPT_DEFAULT;
	}
	
	
	public int blacksun_guri_handleBranch19(obj_id player, obj_id npc, string_id response) throws InterruptedException
	{
		
		if (response.equals("s_85"))
		{
			
			if (blacksun_guri_condition__defaultCondition (player, npc))
			{
				
				string_id message = new string_id (c_stringFile, "s_88");
				int numberOfResponses = 0;
				
				boolean hasResponse = false;
				
				boolean hasResponse0 = false;
				if (blacksun_guri_condition__defaultCondition (player, npc))
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
					
					utils.setScriptVar (player, "conversation.blacksun_guri.branchId", 26);
					
					npcSpeak (player, message);
					npcSetConversationResponses (player, responses);
				}
				else
				{
					utils.removeScriptVar (player, "conversation.blacksun_guri.branchId");
					
					npcEndConversationWithMessage (player, message);
				}
				
				return SCRIPT_CONTINUE;
			}
			
		}
		
		return SCRIPT_DEFAULT;
	}
	
	
	public int blacksun_guri_handleBranch20(obj_id player, obj_id npc, string_id response) throws InterruptedException
	{
		
		if (response.equals("s_140"))
		{
			
			if (blacksun_guri_condition__defaultCondition (player, npc))
			{
				
				string_id message = new string_id (c_stringFile, "s_142");
				int numberOfResponses = 0;
				
				boolean hasResponse = false;
				
				boolean hasResponse0 = false;
				if (blacksun_guri_condition__defaultCondition (player, npc))
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
					
					utils.setScriptVar (player, "conversation.blacksun_guri.branchId", 27);
					
					npcSpeak (player, message);
					npcSetConversationResponses (player, responses);
				}
				else
				{
					utils.removeScriptVar (player, "conversation.blacksun_guri.branchId");
					
					npcEndConversationWithMessage (player, message);
				}
				
				return SCRIPT_CONTINUE;
			}
			
		}
		
		return SCRIPT_DEFAULT;
	}
	
	
	public int blacksun_guri_handleBranch21(obj_id player, obj_id npc, string_id response) throws InterruptedException
	{
		
		if (response.equals("s_76"))
		{
			
			if (blacksun_guri_condition__defaultCondition (player, npc))
			{
				
				string_id message = new string_id (c_stringFile, "s_92");
				int numberOfResponses = 0;
				
				boolean hasResponse = false;
				
				boolean hasResponse0 = false;
				if (blacksun_guri_condition__defaultCondition (player, npc))
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
					
					utils.setScriptVar (player, "conversation.blacksun_guri.branchId", 22);
					
					npcSpeak (player, message);
					npcSetConversationResponses (player, responses);
				}
				else
				{
					utils.removeScriptVar (player, "conversation.blacksun_guri.branchId");
					
					npcEndConversationWithMessage (player, message);
				}
				
				return SCRIPT_CONTINUE;
			}
			
		}
		
		if (response.equals("s_77"))
		{
			
			if (blacksun_guri_condition__defaultCondition (player, npc))
			{
				
				string_id message = new string_id (c_stringFile, "s_80");
				int numberOfResponses = 0;
				
				boolean hasResponse = false;
				
				boolean hasResponse0 = false;
				if (blacksun_guri_condition__defaultCondition (player, npc))
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
					
					utils.setScriptVar (player, "conversation.blacksun_guri.branchId", 24);
					
					npcSpeak (player, message);
					npcSetConversationResponses (player, responses);
				}
				else
				{
					utils.removeScriptVar (player, "conversation.blacksun_guri.branchId");
					
					npcEndConversationWithMessage (player, message);
				}
				
				return SCRIPT_CONTINUE;
			}
			
		}
		
		return SCRIPT_DEFAULT;
	}
	
	
	public int blacksun_guri_handleBranch22(obj_id player, obj_id npc, string_id response) throws InterruptedException
	{
		
		if (response.equals("s_93"))
		{
			
			if (blacksun_guri_condition__defaultCondition (player, npc))
			{
				blacksun_guri_action_revokeNeutralGiveImperial (player, npc);
				
				string_id message = new string_id (c_stringFile, "s_94");
				utils.removeScriptVar (player, "conversation.blacksun_guri.branchId");
				
				npcEndConversationWithMessage (player, message);
				
				return SCRIPT_CONTINUE;
			}
			
		}
		
		return SCRIPT_DEFAULT;
	}
	
	
	public int blacksun_guri_handleBranch24(obj_id player, obj_id npc, string_id response) throws InterruptedException
	{
		
		if (response.equals("s_81"))
		{
			
			if (blacksun_guri_condition__defaultCondition (player, npc))
			{
				
				string_id message = new string_id (c_stringFile, "s_83");
				int numberOfResponses = 0;
				
				boolean hasResponse = false;
				
				boolean hasResponse0 = false;
				if (blacksun_guri_condition__defaultCondition (player, npc))
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
					
					utils.setScriptVar (player, "conversation.blacksun_guri.branchId", 26);
					
					npcSpeak (player, message);
					npcSetConversationResponses (player, responses);
				}
				else
				{
					utils.removeScriptVar (player, "conversation.blacksun_guri.branchId");
					
					npcEndConversationWithMessage (player, message);
				}
				
				return SCRIPT_CONTINUE;
			}
			
		}
		
		return SCRIPT_DEFAULT;
	}
	
	
	public int blacksun_guri_handleBranch25(obj_id player, obj_id npc, string_id response) throws InterruptedException
	{
		
		if (response.equals("s_140"))
		{
			
			if (blacksun_guri_condition__defaultCondition (player, npc))
			{
				
				string_id message = new string_id (c_stringFile, "s_142");
				int numberOfResponses = 0;
				
				boolean hasResponse = false;
				
				boolean hasResponse0 = false;
				if (blacksun_guri_condition__defaultCondition (player, npc))
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
					
					utils.setScriptVar (player, "conversation.blacksun_guri.branchId", 27);
					
					npcSpeak (player, message);
					npcSetConversationResponses (player, responses);
				}
				else
				{
					utils.removeScriptVar (player, "conversation.blacksun_guri.branchId");
					
					npcEndConversationWithMessage (player, message);
				}
				
				return SCRIPT_CONTINUE;
			}
			
		}
		
		return SCRIPT_DEFAULT;
	}
	
	
	public int blacksun_guri_handleBranch26(obj_id player, obj_id npc, string_id response) throws InterruptedException
	{
		
		if (response.equals("s_140"))
		{
			
			if (blacksun_guri_condition__defaultCondition (player, npc))
			{
				
				string_id message = new string_id (c_stringFile, "s_142");
				int numberOfResponses = 0;
				
				boolean hasResponse = false;
				
				boolean hasResponse0 = false;
				if (blacksun_guri_condition__defaultCondition (player, npc))
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
					
					utils.setScriptVar (player, "conversation.blacksun_guri.branchId", 27);
					
					npcSpeak (player, message);
					npcSetConversationResponses (player, responses);
				}
				else
				{
					utils.removeScriptVar (player, "conversation.blacksun_guri.branchId");
					
					npcEndConversationWithMessage (player, message);
				}
				
				return SCRIPT_CONTINUE;
			}
			
		}
		
		return SCRIPT_DEFAULT;
	}
	
	
	public int blacksun_guri_handleBranch27(obj_id player, obj_id npc, string_id response) throws InterruptedException
	{
		
		if (response.equals("s_144"))
		{
			
			if (blacksun_guri_condition__defaultCondition (player, npc))
			{
				
				string_id message = new string_id (c_stringFile, "s_146");
				int numberOfResponses = 0;
				
				boolean hasResponse = false;
				
				boolean hasResponse0 = false;
				if (blacksun_guri_condition__defaultCondition (player, npc))
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
						responses[responseIndex++] = new string_id (c_stringFile, "s_148");
					}
					
					utils.setScriptVar (player, "conversation.blacksun_guri.branchId", 28);
					
					npcSpeak (player, message);
					npcSetConversationResponses (player, responses);
				}
				else
				{
					utils.removeScriptVar (player, "conversation.blacksun_guri.branchId");
					
					npcEndConversationWithMessage (player, message);
				}
				
				return SCRIPT_CONTINUE;
			}
			
		}
		
		return SCRIPT_DEFAULT;
	}
	
	
	public int blacksun_guri_handleBranch28(obj_id player, obj_id npc, string_id response) throws InterruptedException
	{
		
		if (response.equals("s_148"))
		{
			
			if (blacksun_guri_condition__defaultCondition (player, npc))
			{
				doAnimationAction (npc, "nod_head_multiple");
				
				string_id message = new string_id (c_stringFile, "s_150");
				int numberOfResponses = 0;
				
				boolean hasResponse = false;
				
				boolean hasResponse0 = false;
				if (blacksun_guri_condition__defaultCondition (player, npc))
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
						responses[responseIndex++] = new string_id (c_stringFile, "s_152");
					}
					
					utils.setScriptVar (player, "conversation.blacksun_guri.branchId", 29);
					
					npcSpeak (player, message);
					npcSetConversationResponses (player, responses);
				}
				else
				{
					utils.removeScriptVar (player, "conversation.blacksun_guri.branchId");
					
					npcEndConversationWithMessage (player, message);
				}
				
				return SCRIPT_CONTINUE;
			}
			
		}
		
		return SCRIPT_DEFAULT;
	}
	
	
	public int blacksun_guri_handleBranch29(obj_id player, obj_id npc, string_id response) throws InterruptedException
	{
		
		if (response.equals("s_152"))
		{
			
			if (blacksun_guri_condition__defaultCondition (player, npc))
			{
				
				string_id message = new string_id (c_stringFile, "s_154");
				int numberOfResponses = 0;
				
				boolean hasResponse = false;
				
				boolean hasResponse0 = false;
				if (blacksun_guri_condition__defaultCondition (player, npc))
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
						responses[responseIndex++] = new string_id (c_stringFile, "s_156");
					}
					
					utils.setScriptVar (player, "conversation.blacksun_guri.branchId", 30);
					
					npcSpeak (player, message);
					npcSetConversationResponses (player, responses);
				}
				else
				{
					utils.removeScriptVar (player, "conversation.blacksun_guri.branchId");
					
					npcEndConversationWithMessage (player, message);
				}
				
				return SCRIPT_CONTINUE;
			}
			
		}
		
		return SCRIPT_DEFAULT;
	}
	
	
	public int blacksun_guri_handleBranch30(obj_id player, obj_id npc, string_id response) throws InterruptedException
	{
		
		if (response.equals("s_156"))
		{
			
			if (blacksun_guri_condition__defaultCondition (player, npc))
			{
				
				string_id message = new string_id (c_stringFile, "s_242");
				int numberOfResponses = 0;
				
				boolean hasResponse = false;
				
				boolean hasResponse0 = false;
				if (blacksun_guri_condition__defaultCondition (player, npc))
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
						responses[responseIndex++] = new string_id (c_stringFile, "s_243");
					}
					
					utils.setScriptVar (player, "conversation.blacksun_guri.branchId", 31);
					
					npcSpeak (player, message);
					npcSetConversationResponses (player, responses);
				}
				else
				{
					utils.removeScriptVar (player, "conversation.blacksun_guri.branchId");
					
					npcEndConversationWithMessage (player, message);
				}
				
				return SCRIPT_CONTINUE;
			}
			
		}
		
		return SCRIPT_DEFAULT;
	}
	
	
	public int blacksun_guri_handleBranch31(obj_id player, obj_id npc, string_id response) throws InterruptedException
	{
		
		if (response.equals("s_243"))
		{
			
			if (blacksun_guri_condition__defaultCondition (player, npc))
			{
				
				string_id message = new string_id (c_stringFile, "s_244");
				int numberOfResponses = 0;
				
				boolean hasResponse = false;
				
				boolean hasResponse0 = false;
				if (blacksun_guri_condition__defaultCondition (player, npc))
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
						responses[responseIndex++] = new string_id (c_stringFile, "s_245");
					}
					
					utils.setScriptVar (player, "conversation.blacksun_guri.branchId", 32);
					
					npcSpeak (player, message);
					npcSetConversationResponses (player, responses);
				}
				else
				{
					utils.removeScriptVar (player, "conversation.blacksun_guri.branchId");
					
					npcEndConversationWithMessage (player, message);
				}
				
				return SCRIPT_CONTINUE;
			}
			
		}
		
		return SCRIPT_DEFAULT;
	}
	
	
	public int blacksun_guri_handleBranch32(obj_id player, obj_id npc, string_id response) throws InterruptedException
	{
		
		if (response.equals("s_245"))
		{
			
			if (blacksun_guri_condition__defaultCondition (player, npc))
			{
				
				string_id message = new string_id (c_stringFile, "s_247");
				int numberOfResponses = 0;
				
				boolean hasResponse = false;
				
				boolean hasResponse0 = false;
				if (blacksun_guri_condition__defaultCondition (player, npc))
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
						responses[responseIndex++] = new string_id (c_stringFile, "s_249");
					}
					
					utils.setScriptVar (player, "conversation.blacksun_guri.branchId", 33);
					
					npcSpeak (player, message);
					npcSetConversationResponses (player, responses);
				}
				else
				{
					utils.removeScriptVar (player, "conversation.blacksun_guri.branchId");
					
					npcEndConversationWithMessage (player, message);
				}
				
				return SCRIPT_CONTINUE;
			}
			
		}
		
		return SCRIPT_DEFAULT;
	}
	
	
	public int blacksun_guri_handleBranch33(obj_id player, obj_id npc, string_id response) throws InterruptedException
	{
		
		if (response.equals("s_249"))
		{
			
			if (blacksun_guri_condition__defaultCondition (player, npc))
			{
				
				string_id message = new string_id (c_stringFile, "s_260");
				int numberOfResponses = 0;
				
				boolean hasResponse = false;
				
				boolean hasResponse0 = false;
				if (blacksun_guri_condition__defaultCondition (player, npc))
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
						responses[responseIndex++] = new string_id (c_stringFile, "s_261");
					}
					
					utils.setScriptVar (player, "conversation.blacksun_guri.branchId", 34);
					
					npcSpeak (player, message);
					npcSetConversationResponses (player, responses);
				}
				else
				{
					utils.removeScriptVar (player, "conversation.blacksun_guri.branchId");
					
					npcEndConversationWithMessage (player, message);
				}
				
				return SCRIPT_CONTINUE;
			}
			
		}
		
		return SCRIPT_DEFAULT;
	}
	
	
	public int blacksun_guri_handleBranch34(obj_id player, obj_id npc, string_id response) throws InterruptedException
	{
		
		if (response.equals("s_261"))
		{
			
			if (blacksun_guri_condition__defaultCondition (player, npc))
			{
				
				string_id message = new string_id (c_stringFile, "s_262");
				int numberOfResponses = 0;
				
				boolean hasResponse = false;
				
				boolean hasResponse0 = false;
				if (blacksun_guri_condition__defaultCondition (player, npc))
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
						responses[responseIndex++] = new string_id (c_stringFile, "s_263");
					}
					
					utils.setScriptVar (player, "conversation.blacksun_guri.branchId", 35);
					
					npcSpeak (player, message);
					npcSetConversationResponses (player, responses);
				}
				else
				{
					utils.removeScriptVar (player, "conversation.blacksun_guri.branchId");
					
					npcEndConversationWithMessage (player, message);
				}
				
				return SCRIPT_CONTINUE;
			}
			
		}
		
		return SCRIPT_DEFAULT;
	}
	
	
	public int blacksun_guri_handleBranch35(obj_id player, obj_id npc, string_id response) throws InterruptedException
	{
		
		if (response.equals("s_263"))
		{
			
			if (blacksun_guri_condition__defaultCondition (player, npc))
			{
				
				string_id message = new string_id (c_stringFile, "s_264");
				int numberOfResponses = 0;
				
				boolean hasResponse = false;
				
				boolean hasResponse0 = false;
				if (blacksun_guri_condition__defaultCondition (player, npc))
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
						responses[responseIndex++] = new string_id (c_stringFile, "s_265");
					}
					
					utils.setScriptVar (player, "conversation.blacksun_guri.branchId", 36);
					
					npcSpeak (player, message);
					npcSetConversationResponses (player, responses);
				}
				else
				{
					utils.removeScriptVar (player, "conversation.blacksun_guri.branchId");
					
					npcEndConversationWithMessage (player, message);
				}
				
				return SCRIPT_CONTINUE;
			}
			
		}
		
		return SCRIPT_DEFAULT;
	}
	
	
	public int blacksun_guri_handleBranch36(obj_id player, obj_id npc, string_id response) throws InterruptedException
	{
		
		if (response.equals("s_265"))
		{
			
			if (blacksun_guri_condition__defaultCondition (player, npc))
			{
				
				string_id message = new string_id (c_stringFile, "s_266");
				int numberOfResponses = 0;
				
				boolean hasResponse = false;
				
				boolean hasResponse0 = false;
				if (blacksun_guri_condition__defaultCondition (player, npc))
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
						responses[responseIndex++] = new string_id (c_stringFile, "s_267");
					}
					
					utils.setScriptVar (player, "conversation.blacksun_guri.branchId", 37);
					
					prose_package pp = new prose_package ();
					pp.stringId = message;
					pp.actor.set (player);
					pp.target.set (npc);
					
					npcSpeak (player, pp);
					npcSetConversationResponses (player, responses);
				}
				else
				{
					utils.removeScriptVar (player, "conversation.blacksun_guri.branchId");
					
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
	
	
	public int blacksun_guri_handleBranch37(obj_id player, obj_id npc, string_id response) throws InterruptedException
	{
		
		if (response.equals("s_267"))
		{
			
			if (blacksun_guri_condition__defaultCondition (player, npc))
			{
				
				string_id message = new string_id (c_stringFile, "s_268");
				int numberOfResponses = 0;
				
				boolean hasResponse = false;
				
				boolean hasResponse0 = false;
				if (blacksun_guri_condition__defaultCondition (player, npc))
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
						responses[responseIndex++] = new string_id (c_stringFile, "s_269");
					}
					
					utils.setScriptVar (player, "conversation.blacksun_guri.branchId", 38);
					
					npcSpeak (player, message);
					npcSetConversationResponses (player, responses);
				}
				else
				{
					utils.removeScriptVar (player, "conversation.blacksun_guri.branchId");
					
					npcEndConversationWithMessage (player, message);
				}
				
				return SCRIPT_CONTINUE;
			}
			
		}
		
		return SCRIPT_DEFAULT;
	}
	
	
	public int blacksun_guri_handleBranch38(obj_id player, obj_id npc, string_id response) throws InterruptedException
	{
		
		if (response.equals("s_269"))
		{
			doAnimationAction (player, "shrug_hands");
			
			if (blacksun_guri_condition__defaultCondition (player, npc))
			{
				
				string_id message = new string_id (c_stringFile, "s_271");
				int numberOfResponses = 0;
				
				boolean hasResponse = false;
				
				boolean hasResponse0 = false;
				if (blacksun_guri_condition__defaultCondition (player, npc))
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
						responses[responseIndex++] = new string_id (c_stringFile, "s_273");
					}
					
					utils.setScriptVar (player, "conversation.blacksun_guri.branchId", 39);
					
					prose_package pp = new prose_package ();
					pp.stringId = message;
					pp.actor.set (player);
					pp.target.set (npc);
					
					npcSpeak (player, pp);
					npcSetConversationResponses (player, responses);
				}
				else
				{
					utils.removeScriptVar (player, "conversation.blacksun_guri.branchId");
					
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
	
	
	public int blacksun_guri_handleBranch39(obj_id player, obj_id npc, string_id response) throws InterruptedException
	{
		
		if (response.equals("s_273"))
		{
			
			if (blacksun_guri_condition__defaultCondition (player, npc))
			{
				
				string_id message = new string_id (c_stringFile, "s_275");
				int numberOfResponses = 0;
				
				boolean hasResponse = false;
				
				boolean hasResponse0 = false;
				if (blacksun_guri_condition__defaultCondition (player, npc))
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
						responses[responseIndex++] = new string_id (c_stringFile, "s_277");
					}
					
					utils.setScriptVar (player, "conversation.blacksun_guri.branchId", 40);
					
					npcSpeak (player, message);
					npcSetConversationResponses (player, responses);
				}
				else
				{
					utils.removeScriptVar (player, "conversation.blacksun_guri.branchId");
					
					npcEndConversationWithMessage (player, message);
				}
				
				return SCRIPT_CONTINUE;
			}
			
		}
		
		return SCRIPT_DEFAULT;
	}
	
	
	public int blacksun_guri_handleBranch40(obj_id player, obj_id npc, string_id response) throws InterruptedException
	{
		
		if (response.equals("s_277"))
		{
			
			if (blacksun_guri_condition__defaultCondition (player, npc))
			{
				
				string_id message = new string_id (c_stringFile, "s_279");
				int numberOfResponses = 0;
				
				boolean hasResponse = false;
				
				boolean hasResponse0 = false;
				if (blacksun_guri_condition__defaultCondition (player, npc))
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
						responses[responseIndex++] = new string_id (c_stringFile, "s_280");
					}
					
					utils.setScriptVar (player, "conversation.blacksun_guri.branchId", 41);
					
					npcSpeak (player, message);
					npcSetConversationResponses (player, responses);
				}
				else
				{
					utils.removeScriptVar (player, "conversation.blacksun_guri.branchId");
					
					npcEndConversationWithMessage (player, message);
				}
				
				return SCRIPT_CONTINUE;
			}
			
		}
		
		return SCRIPT_DEFAULT;
	}
	
	
	public int blacksun_guri_handleBranch41(obj_id player, obj_id npc, string_id response) throws InterruptedException
	{
		
		if (response.equals("s_280"))
		{
			
			if (blacksun_guri_condition__defaultCondition (player, npc))
			{
				
				string_id message = new string_id (c_stringFile, "s_281");
				int numberOfResponses = 0;
				
				boolean hasResponse = false;
				
				boolean hasResponse0 = false;
				if (blacksun_guri_condition__defaultCondition (player, npc))
				{
					++numberOfResponses;
					hasResponse = true;
					hasResponse0 = true;
				}
				
				boolean hasResponse1 = false;
				if (blacksun_guri_condition__defaultCondition (player, npc))
				{
					++numberOfResponses;
					hasResponse = true;
					hasResponse1 = true;
				}
				
				boolean hasResponse2 = false;
				if (blacksun_guri_condition__defaultCondition (player, npc))
				{
					++numberOfResponses;
					hasResponse = true;
					hasResponse2 = true;
				}
				
				boolean hasResponse3 = false;
				if (blacksun_guri_condition__defaultCondition (player, npc))
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
						responses[responseIndex++] = new string_id (c_stringFile, "s_283");
					}
					
					if (hasResponse1)
					{
						responses[responseIndex++] = new string_id (c_stringFile, "s_293");
					}
					
					if (hasResponse2)
					{
						responses[responseIndex++] = new string_id (c_stringFile, "s_303");
					}
					
					if (hasResponse3)
					{
						responses[responseIndex++] = new string_id (c_stringFile, "s_309");
					}
					
					utils.setScriptVar (player, "conversation.blacksun_guri.branchId", 42);
					
					prose_package pp = new prose_package ();
					pp.stringId = message;
					pp.actor.set (player);
					pp.target.set (npc);
					
					npcSpeak (player, pp);
					npcSetConversationResponses (player, responses);
				}
				else
				{
					utils.removeScriptVar (player, "conversation.blacksun_guri.branchId");
					
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
	
	
	public int blacksun_guri_handleBranch42(obj_id player, obj_id npc, string_id response) throws InterruptedException
	{
		
		if (response.equals("s_283"))
		{
			
			if (blacksun_guri_condition__defaultCondition (player, npc))
			{
				
				string_id message = new string_id (c_stringFile, "s_285");
				int numberOfResponses = 0;
				
				boolean hasResponse = false;
				
				boolean hasResponse0 = false;
				if (blacksun_guri_condition__defaultCondition (player, npc))
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
						responses[responseIndex++] = new string_id (c_stringFile, "s_287");
					}
					
					utils.setScriptVar (player, "conversation.blacksun_guri.branchId", 43);
					
					npcSpeak (player, message);
					npcSetConversationResponses (player, responses);
				}
				else
				{
					utils.removeScriptVar (player, "conversation.blacksun_guri.branchId");
					
					npcEndConversationWithMessage (player, message);
				}
				
				return SCRIPT_CONTINUE;
			}
			
		}
		
		if (response.equals("s_293"))
		{
			
			if (blacksun_guri_condition__defaultCondition (player, npc))
			{
				
				string_id message = new string_id (c_stringFile, "s_295");
				int numberOfResponses = 0;
				
				boolean hasResponse = false;
				
				boolean hasResponse0 = false;
				if (blacksun_guri_condition__defaultCondition (player, npc))
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
						responses[responseIndex++] = new string_id (c_stringFile, "s_297");
					}
					
					utils.setScriptVar (player, "conversation.blacksun_guri.branchId", 46);
					
					npcSpeak (player, message);
					npcSetConversationResponses (player, responses);
				}
				else
				{
					utils.removeScriptVar (player, "conversation.blacksun_guri.branchId");
					
					npcEndConversationWithMessage (player, message);
				}
				
				return SCRIPT_CONTINUE;
			}
			
		}
		
		if (response.equals("s_303"))
		{
			
			if (blacksun_guri_condition__defaultCondition (player, npc))
			{
				
				string_id message = new string_id (c_stringFile, "s_305");
				int numberOfResponses = 0;
				
				boolean hasResponse = false;
				
				boolean hasResponse0 = false;
				if (blacksun_guri_condition__defaultCondition (player, npc))
				{
					++numberOfResponses;
					hasResponse = true;
					hasResponse0 = true;
				}
				
				boolean hasResponse1 = false;
				if (blacksun_guri_condition__defaultCondition (player, npc))
				{
					++numberOfResponses;
					hasResponse = true;
					hasResponse1 = true;
				}
				
				boolean hasResponse2 = false;
				if (blacksun_guri_condition__defaultCondition (player, npc))
				{
					++numberOfResponses;
					hasResponse = true;
					hasResponse2 = true;
				}
				
				boolean hasResponse3 = false;
				if (blacksun_guri_condition__defaultCondition (player, npc))
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
						responses[responseIndex++] = new string_id (c_stringFile, "s_283");
					}
					
					if (hasResponse1)
					{
						responses[responseIndex++] = new string_id (c_stringFile, "s_293");
					}
					
					if (hasResponse2)
					{
						responses[responseIndex++] = new string_id (c_stringFile, "s_303");
					}
					
					if (hasResponse3)
					{
						responses[responseIndex++] = new string_id (c_stringFile, "s_309");
					}
					
					utils.setScriptVar (player, "conversation.blacksun_guri.branchId", 42);
					
					npcSpeak (player, message);
					npcSetConversationResponses (player, responses);
				}
				else
				{
					utils.removeScriptVar (player, "conversation.blacksun_guri.branchId");
					
					npcEndConversationWithMessage (player, message);
				}
				
				return SCRIPT_CONTINUE;
			}
			
		}
		
		if (response.equals("s_309"))
		{
			
			if (blacksun_guri_condition__defaultCondition (player, npc))
			{
				blacksun_guri_action_grantMissionOne (player, npc);
				
				string_id message = new string_id (c_stringFile, "s_311");
				utils.removeScriptVar (player, "conversation.blacksun_guri.branchId");
				
				npcEndConversationWithMessage (player, message);
				
				return SCRIPT_CONTINUE;
			}
			
		}
		
		return SCRIPT_DEFAULT;
	}
	
	
	public int blacksun_guri_handleBranch43(obj_id player, obj_id npc, string_id response) throws InterruptedException
	{
		
		if (response.equals("s_287"))
		{
			
			if (blacksun_guri_condition__defaultCondition (player, npc))
			{
				
				string_id message = new string_id (c_stringFile, "s_289");
				int numberOfResponses = 0;
				
				boolean hasResponse = false;
				
				boolean hasResponse0 = false;
				if (blacksun_guri_condition__defaultCondition (player, npc))
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
						responses[responseIndex++] = new string_id (c_stringFile, "s_291");
					}
					
					utils.setScriptVar (player, "conversation.blacksun_guri.branchId", 44);
					
					npcSpeak (player, message);
					npcSetConversationResponses (player, responses);
				}
				else
				{
					utils.removeScriptVar (player, "conversation.blacksun_guri.branchId");
					
					npcEndConversationWithMessage (player, message);
				}
				
				return SCRIPT_CONTINUE;
			}
			
		}
		
		return SCRIPT_DEFAULT;
	}
	
	
	public int blacksun_guri_handleBranch44(obj_id player, obj_id npc, string_id response) throws InterruptedException
	{
		
		if (response.equals("s_291"))
		{
			
			if (blacksun_guri_condition__defaultCondition (player, npc))
			{
				
				string_id message = new string_id (c_stringFile, "s_312");
				int numberOfResponses = 0;
				
				boolean hasResponse = false;
				
				boolean hasResponse0 = false;
				if (blacksun_guri_condition__defaultCondition (player, npc))
				{
					++numberOfResponses;
					hasResponse = true;
					hasResponse0 = true;
				}
				
				boolean hasResponse1 = false;
				if (blacksun_guri_condition__defaultCondition (player, npc))
				{
					++numberOfResponses;
					hasResponse = true;
					hasResponse1 = true;
				}
				
				boolean hasResponse2 = false;
				if (blacksun_guri_condition__defaultCondition (player, npc))
				{
					++numberOfResponses;
					hasResponse = true;
					hasResponse2 = true;
				}
				
				boolean hasResponse3 = false;
				if (blacksun_guri_condition__defaultCondition (player, npc))
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
						responses[responseIndex++] = new string_id (c_stringFile, "s_283");
					}
					
					if (hasResponse1)
					{
						responses[responseIndex++] = new string_id (c_stringFile, "s_293");
					}
					
					if (hasResponse2)
					{
						responses[responseIndex++] = new string_id (c_stringFile, "s_303");
					}
					
					if (hasResponse3)
					{
						responses[responseIndex++] = new string_id (c_stringFile, "s_309");
					}
					
					utils.setScriptVar (player, "conversation.blacksun_guri.branchId", 42);
					
					npcSpeak (player, message);
					npcSetConversationResponses (player, responses);
				}
				else
				{
					utils.removeScriptVar (player, "conversation.blacksun_guri.branchId");
					
					npcEndConversationWithMessage (player, message);
				}
				
				return SCRIPT_CONTINUE;
			}
			
		}
		
		return SCRIPT_DEFAULT;
	}
	
	
	public int blacksun_guri_handleBranch45(obj_id player, obj_id npc, string_id response) throws InterruptedException
	{
		
		if (response.equals("s_283"))
		{
			
			if (blacksun_guri_condition__defaultCondition (player, npc))
			{
				
				string_id message = new string_id (c_stringFile, "s_285");
				int numberOfResponses = 0;
				
				boolean hasResponse = false;
				
				boolean hasResponse0 = false;
				if (blacksun_guri_condition__defaultCondition (player, npc))
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
						responses[responseIndex++] = new string_id (c_stringFile, "s_287");
					}
					
					utils.setScriptVar (player, "conversation.blacksun_guri.branchId", 43);
					
					npcSpeak (player, message);
					npcSetConversationResponses (player, responses);
				}
				else
				{
					utils.removeScriptVar (player, "conversation.blacksun_guri.branchId");
					
					npcEndConversationWithMessage (player, message);
				}
				
				return SCRIPT_CONTINUE;
			}
			
		}
		
		if (response.equals("s_293"))
		{
			
			if (blacksun_guri_condition__defaultCondition (player, npc))
			{
				
				string_id message = new string_id (c_stringFile, "s_295");
				int numberOfResponses = 0;
				
				boolean hasResponse = false;
				
				boolean hasResponse0 = false;
				if (blacksun_guri_condition__defaultCondition (player, npc))
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
						responses[responseIndex++] = new string_id (c_stringFile, "s_297");
					}
					
					utils.setScriptVar (player, "conversation.blacksun_guri.branchId", 46);
					
					npcSpeak (player, message);
					npcSetConversationResponses (player, responses);
				}
				else
				{
					utils.removeScriptVar (player, "conversation.blacksun_guri.branchId");
					
					npcEndConversationWithMessage (player, message);
				}
				
				return SCRIPT_CONTINUE;
			}
			
		}
		
		if (response.equals("s_303"))
		{
			
			if (blacksun_guri_condition__defaultCondition (player, npc))
			{
				
				string_id message = new string_id (c_stringFile, "s_305");
				int numberOfResponses = 0;
				
				boolean hasResponse = false;
				
				boolean hasResponse0 = false;
				if (blacksun_guri_condition__defaultCondition (player, npc))
				{
					++numberOfResponses;
					hasResponse = true;
					hasResponse0 = true;
				}
				
				boolean hasResponse1 = false;
				if (blacksun_guri_condition__defaultCondition (player, npc))
				{
					++numberOfResponses;
					hasResponse = true;
					hasResponse1 = true;
				}
				
				boolean hasResponse2 = false;
				if (blacksun_guri_condition__defaultCondition (player, npc))
				{
					++numberOfResponses;
					hasResponse = true;
					hasResponse2 = true;
				}
				
				boolean hasResponse3 = false;
				if (blacksun_guri_condition__defaultCondition (player, npc))
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
						responses[responseIndex++] = new string_id (c_stringFile, "s_283");
					}
					
					if (hasResponse1)
					{
						responses[responseIndex++] = new string_id (c_stringFile, "s_293");
					}
					
					if (hasResponse2)
					{
						responses[responseIndex++] = new string_id (c_stringFile, "s_303");
					}
					
					if (hasResponse3)
					{
						responses[responseIndex++] = new string_id (c_stringFile, "s_309");
					}
					
					utils.setScriptVar (player, "conversation.blacksun_guri.branchId", 42);
					
					npcSpeak (player, message);
					npcSetConversationResponses (player, responses);
				}
				else
				{
					utils.removeScriptVar (player, "conversation.blacksun_guri.branchId");
					
					npcEndConversationWithMessage (player, message);
				}
				
				return SCRIPT_CONTINUE;
			}
			
		}
		
		if (response.equals("s_309"))
		{
			
			if (blacksun_guri_condition__defaultCondition (player, npc))
			{
				blacksun_guri_action_grantMissionOne (player, npc);
				
				string_id message = new string_id (c_stringFile, "s_311");
				utils.removeScriptVar (player, "conversation.blacksun_guri.branchId");
				
				npcEndConversationWithMessage (player, message);
				
				return SCRIPT_CONTINUE;
			}
			
		}
		
		return SCRIPT_DEFAULT;
	}
	
	
	public int blacksun_guri_handleBranch46(obj_id player, obj_id npc, string_id response) throws InterruptedException
	{
		
		if (response.equals("s_297"))
		{
			
			if (blacksun_guri_condition__defaultCondition (player, npc))
			{
				
				string_id message = new string_id (c_stringFile, "s_299");
				int numberOfResponses = 0;
				
				boolean hasResponse = false;
				
				boolean hasResponse0 = false;
				if (blacksun_guri_condition__defaultCondition (player, npc))
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
						responses[responseIndex++] = new string_id (c_stringFile, "s_301");
					}
					
					utils.setScriptVar (player, "conversation.blacksun_guri.branchId", 47);
					
					npcSpeak (player, message);
					npcSetConversationResponses (player, responses);
				}
				else
				{
					utils.removeScriptVar (player, "conversation.blacksun_guri.branchId");
					
					npcEndConversationWithMessage (player, message);
				}
				
				return SCRIPT_CONTINUE;
			}
			
		}
		
		return SCRIPT_DEFAULT;
	}
	
	
	public int blacksun_guri_handleBranch47(obj_id player, obj_id npc, string_id response) throws InterruptedException
	{
		
		if (response.equals("s_301"))
		{
			
			if (blacksun_guri_condition__defaultCondition (player, npc))
			{
				
				string_id message = new string_id (c_stringFile, "s_313");
				int numberOfResponses = 0;
				
				boolean hasResponse = false;
				
				boolean hasResponse0 = false;
				if (blacksun_guri_condition__defaultCondition (player, npc))
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
						responses[responseIndex++] = new string_id (c_stringFile, "s_314");
					}
					
					utils.setScriptVar (player, "conversation.blacksun_guri.branchId", 48);
					
					npcSpeak (player, message);
					npcSetConversationResponses (player, responses);
				}
				else
				{
					utils.removeScriptVar (player, "conversation.blacksun_guri.branchId");
					
					npcEndConversationWithMessage (player, message);
				}
				
				return SCRIPT_CONTINUE;
			}
			
		}
		
		return SCRIPT_DEFAULT;
	}
	
	
	public int blacksun_guri_handleBranch48(obj_id player, obj_id npc, string_id response) throws InterruptedException
	{
		
		if (response.equals("s_314"))
		{
			
			if (blacksun_guri_condition__defaultCondition (player, npc))
			{
				
				string_id message = new string_id (c_stringFile, "s_315");
				int numberOfResponses = 0;
				
				boolean hasResponse = false;
				
				boolean hasResponse0 = false;
				if (blacksun_guri_condition__defaultCondition (player, npc))
				{
					++numberOfResponses;
					hasResponse = true;
					hasResponse0 = true;
				}
				
				boolean hasResponse1 = false;
				if (blacksun_guri_condition__defaultCondition (player, npc))
				{
					++numberOfResponses;
					hasResponse = true;
					hasResponse1 = true;
				}
				
				boolean hasResponse2 = false;
				if (blacksun_guri_condition__defaultCondition (player, npc))
				{
					++numberOfResponses;
					hasResponse = true;
					hasResponse2 = true;
				}
				
				boolean hasResponse3 = false;
				if (blacksun_guri_condition__defaultCondition (player, npc))
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
						responses[responseIndex++] = new string_id (c_stringFile, "s_283");
					}
					
					if (hasResponse1)
					{
						responses[responseIndex++] = new string_id (c_stringFile, "s_293");
					}
					
					if (hasResponse2)
					{
						responses[responseIndex++] = new string_id (c_stringFile, "s_303");
					}
					
					if (hasResponse3)
					{
						responses[responseIndex++] = new string_id (c_stringFile, "s_309");
					}
					
					utils.setScriptVar (player, "conversation.blacksun_guri.branchId", 42);
					
					npcSpeak (player, message);
					npcSetConversationResponses (player, responses);
				}
				else
				{
					utils.removeScriptVar (player, "conversation.blacksun_guri.branchId");
					
					npcEndConversationWithMessage (player, message);
				}
				
				return SCRIPT_CONTINUE;
			}
			
		}
		
		return SCRIPT_DEFAULT;
	}
	
	
	public int blacksun_guri_handleBranch49(obj_id player, obj_id npc, string_id response) throws InterruptedException
	{
		
		if (response.equals("s_283"))
		{
			
			if (blacksun_guri_condition__defaultCondition (player, npc))
			{
				
				string_id message = new string_id (c_stringFile, "s_285");
				int numberOfResponses = 0;
				
				boolean hasResponse = false;
				
				boolean hasResponse0 = false;
				if (blacksun_guri_condition__defaultCondition (player, npc))
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
						responses[responseIndex++] = new string_id (c_stringFile, "s_287");
					}
					
					utils.setScriptVar (player, "conversation.blacksun_guri.branchId", 43);
					
					npcSpeak (player, message);
					npcSetConversationResponses (player, responses);
				}
				else
				{
					utils.removeScriptVar (player, "conversation.blacksun_guri.branchId");
					
					npcEndConversationWithMessage (player, message);
				}
				
				return SCRIPT_CONTINUE;
			}
			
		}
		
		if (response.equals("s_293"))
		{
			
			if (blacksun_guri_condition__defaultCondition (player, npc))
			{
				
				string_id message = new string_id (c_stringFile, "s_295");
				int numberOfResponses = 0;
				
				boolean hasResponse = false;
				
				boolean hasResponse0 = false;
				if (blacksun_guri_condition__defaultCondition (player, npc))
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
						responses[responseIndex++] = new string_id (c_stringFile, "s_297");
					}
					
					utils.setScriptVar (player, "conversation.blacksun_guri.branchId", 46);
					
					npcSpeak (player, message);
					npcSetConversationResponses (player, responses);
				}
				else
				{
					utils.removeScriptVar (player, "conversation.blacksun_guri.branchId");
					
					npcEndConversationWithMessage (player, message);
				}
				
				return SCRIPT_CONTINUE;
			}
			
		}
		
		if (response.equals("s_303"))
		{
			
			if (blacksun_guri_condition__defaultCondition (player, npc))
			{
				
				string_id message = new string_id (c_stringFile, "s_305");
				int numberOfResponses = 0;
				
				boolean hasResponse = false;
				
				boolean hasResponse0 = false;
				if (blacksun_guri_condition__defaultCondition (player, npc))
				{
					++numberOfResponses;
					hasResponse = true;
					hasResponse0 = true;
				}
				
				boolean hasResponse1 = false;
				if (blacksun_guri_condition__defaultCondition (player, npc))
				{
					++numberOfResponses;
					hasResponse = true;
					hasResponse1 = true;
				}
				
				boolean hasResponse2 = false;
				if (blacksun_guri_condition__defaultCondition (player, npc))
				{
					++numberOfResponses;
					hasResponse = true;
					hasResponse2 = true;
				}
				
				boolean hasResponse3 = false;
				if (blacksun_guri_condition__defaultCondition (player, npc))
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
						responses[responseIndex++] = new string_id (c_stringFile, "s_283");
					}
					
					if (hasResponse1)
					{
						responses[responseIndex++] = new string_id (c_stringFile, "s_293");
					}
					
					if (hasResponse2)
					{
						responses[responseIndex++] = new string_id (c_stringFile, "s_303");
					}
					
					if (hasResponse3)
					{
						responses[responseIndex++] = new string_id (c_stringFile, "s_309");
					}
					
					utils.setScriptVar (player, "conversation.blacksun_guri.branchId", 42);
					
					npcSpeak (player, message);
					npcSetConversationResponses (player, responses);
				}
				else
				{
					utils.removeScriptVar (player, "conversation.blacksun_guri.branchId");
					
					npcEndConversationWithMessage (player, message);
				}
				
				return SCRIPT_CONTINUE;
			}
			
		}
		
		if (response.equals("s_309"))
		{
			
			if (blacksun_guri_condition__defaultCondition (player, npc))
			{
				blacksun_guri_action_grantMissionOne (player, npc);
				
				string_id message = new string_id (c_stringFile, "s_311");
				utils.removeScriptVar (player, "conversation.blacksun_guri.branchId");
				
				npcEndConversationWithMessage (player, message);
				
				return SCRIPT_CONTINUE;
			}
			
		}
		
		return SCRIPT_DEFAULT;
	}
	
	
	public int blacksun_guri_handleBranch50(obj_id player, obj_id npc, string_id response) throws InterruptedException
	{
		
		if (response.equals("s_283"))
		{
			
			if (blacksun_guri_condition__defaultCondition (player, npc))
			{
				
				string_id message = new string_id (c_stringFile, "s_285");
				int numberOfResponses = 0;
				
				boolean hasResponse = false;
				
				boolean hasResponse0 = false;
				if (blacksun_guri_condition__defaultCondition (player, npc))
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
						responses[responseIndex++] = new string_id (c_stringFile, "s_287");
					}
					
					utils.setScriptVar (player, "conversation.blacksun_guri.branchId", 43);
					
					npcSpeak (player, message);
					npcSetConversationResponses (player, responses);
				}
				else
				{
					utils.removeScriptVar (player, "conversation.blacksun_guri.branchId");
					
					npcEndConversationWithMessage (player, message);
				}
				
				return SCRIPT_CONTINUE;
			}
			
		}
		
		if (response.equals("s_293"))
		{
			
			if (blacksun_guri_condition__defaultCondition (player, npc))
			{
				
				string_id message = new string_id (c_stringFile, "s_295");
				int numberOfResponses = 0;
				
				boolean hasResponse = false;
				
				boolean hasResponse0 = false;
				if (blacksun_guri_condition__defaultCondition (player, npc))
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
						responses[responseIndex++] = new string_id (c_stringFile, "s_297");
					}
					
					utils.setScriptVar (player, "conversation.blacksun_guri.branchId", 46);
					
					npcSpeak (player, message);
					npcSetConversationResponses (player, responses);
				}
				else
				{
					utils.removeScriptVar (player, "conversation.blacksun_guri.branchId");
					
					npcEndConversationWithMessage (player, message);
				}
				
				return SCRIPT_CONTINUE;
			}
			
		}
		
		if (response.equals("s_303"))
		{
			
			if (blacksun_guri_condition__defaultCondition (player, npc))
			{
				
				string_id message = new string_id (c_stringFile, "s_305");
				int numberOfResponses = 0;
				
				boolean hasResponse = false;
				
				boolean hasResponse0 = false;
				if (blacksun_guri_condition__defaultCondition (player, npc))
				{
					++numberOfResponses;
					hasResponse = true;
					hasResponse0 = true;
				}
				
				boolean hasResponse1 = false;
				if (blacksun_guri_condition__defaultCondition (player, npc))
				{
					++numberOfResponses;
					hasResponse = true;
					hasResponse1 = true;
				}
				
				boolean hasResponse2 = false;
				if (blacksun_guri_condition__defaultCondition (player, npc))
				{
					++numberOfResponses;
					hasResponse = true;
					hasResponse2 = true;
				}
				
				boolean hasResponse3 = false;
				if (blacksun_guri_condition__defaultCondition (player, npc))
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
						responses[responseIndex++] = new string_id (c_stringFile, "s_283");
					}
					
					if (hasResponse1)
					{
						responses[responseIndex++] = new string_id (c_stringFile, "s_293");
					}
					
					if (hasResponse2)
					{
						responses[responseIndex++] = new string_id (c_stringFile, "s_303");
					}
					
					if (hasResponse3)
					{
						responses[responseIndex++] = new string_id (c_stringFile, "s_309");
					}
					
					utils.setScriptVar (player, "conversation.blacksun_guri.branchId", 42);
					
					npcSpeak (player, message);
					npcSetConversationResponses (player, responses);
				}
				else
				{
					utils.removeScriptVar (player, "conversation.blacksun_guri.branchId");
					
					npcEndConversationWithMessage (player, message);
				}
				
				return SCRIPT_CONTINUE;
			}
			
		}
		
		if (response.equals("s_309"))
		{
			
			if (blacksun_guri_condition__defaultCondition (player, npc))
			{
				blacksun_guri_action_grantMissionOne (player, npc);
				
				string_id message = new string_id (c_stringFile, "s_311");
				utils.removeScriptVar (player, "conversation.blacksun_guri.branchId");
				
				npcEndConversationWithMessage (player, message);
				
				return SCRIPT_CONTINUE;
			}
			
		}
		
		return SCRIPT_DEFAULT;
	}
	
	
	public int blacksun_guri_handleBranch52(obj_id player, obj_id npc, string_id response) throws InterruptedException
	{
		
		if (response.equals("s_149"))
		{
			
			if (blacksun_guri_condition__defaultCondition (player, npc))
			{
				blacksun_guri_action_grantNeutQuest (player, npc);
				
				string_id message = new string_id (c_stringFile, "s_153");
				utils.removeScriptVar (player, "conversation.blacksun_guri.branchId");
				
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
			detachScript(self, "conversation.blacksun_guri");
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
		detachScript (self, "conversation.blacksun_guri");
		
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
		
		if (blacksun_guri_condition_hasCompletedAll (player, npc))
		{
			
			string_id message = new string_id (c_stringFile, "s_130");
			int numberOfResponses = 0;
			
			boolean hasResponse = false;
			
			boolean hasResponse0 = false;
			if (blacksun_guri_condition__defaultCondition (player, npc))
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
					responses[responseIndex++] = new string_id (c_stringFile, "s_95");
				}
				
				utils.setScriptVar (player, "conversation.blacksun_guri.branchId", 1);
				
				prose_package pp = new prose_package ();
				pp.stringId = message;
				pp.actor.set (player);
				pp.target.set (npc);
				
				npcStartConversation (player, npc, "blacksun_guri", null, pp, responses);
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
		
		if (blacksun_guri_condition_hasCompletedMissions (player, npc))
		{
			
			string_id message = new string_id (c_stringFile, "s_123");
			int numberOfResponses = 0;
			
			boolean hasResponse = false;
			
			boolean hasResponse0 = false;
			if (blacksun_guri_condition__defaultCondition (player, npc))
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
					responses[responseIndex++] = new string_id (c_stringFile, "s_124");
				}
				
				utils.setScriptVar (player, "conversation.blacksun_guri.branchId", 8);
				
				prose_package pp = new prose_package ();
				pp.stringId = message;
				pp.actor.set (player);
				pp.target.set (npc);
				
				npcStartConversation (player, npc, "blacksun_guri", null, pp, responses);
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
		
		if (blacksun_guri_condition_hasFoundStormtroopers (player, npc))
		{
			
			string_id message = new string_id (c_stringFile, "s_122");
			chat.chat (npc, player, message);
			
			return SCRIPT_CONTINUE;
		}
		
		if (blacksun_guri_condition_hasSecondQuestImperial (player, npc))
		{
			
			string_id message = new string_id (c_stringFile, "s_121");
			prose_package pp = new prose_package ();
			pp.stringId = message;
			pp.actor.set (player);
			pp.target.set (npc);
			
			chat.chat (npc, player, null, null, pp);
			return SCRIPT_CONTINUE;
		}
		
		if (blacksun_guri_condition_hasFirstQuestNeutral (player, npc))
		{
			
			string_id message = new string_id (c_stringFile, "s_74");
			prose_package pp = new prose_package ();
			pp.stringId = message;
			pp.actor.set (player);
			pp.target.set (npc);
			
			chat.chat (npc, player, null, null, pp);
			return SCRIPT_CONTINUE;
		}
		
		if (blacksun_guri_condition_isLiveConversion (player, npc))
		{
			doAnimationAction (npc, "nod");
			
			blacksun_guri_action_clearLiveConversion (player, npc);
			
			string_id message = new string_id (c_stringFile, "s_27");
			int numberOfResponses = 0;
			
			boolean hasResponse = false;
			
			boolean hasResponse0 = false;
			if (blacksun_guri_condition__defaultCondition (player, npc))
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
					responses[responseIndex++] = new string_id (c_stringFile, "s_29");
				}
				
				utils.setScriptVar (player, "conversation.blacksun_guri.branchId", 15);
				
				prose_package pp = new prose_package ();
				pp.stringId = message;
				pp.actor.set (player);
				pp.target.set (npc);
				
				npcStartConversation (player, npc, "blacksun_guri", null, pp, responses);
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
		
		if (blacksun_guri_condition_hasDeletedLiveConversion (player, npc))
		{
			
			string_id message = new string_id (c_stringFile, "s_147");
			int numberOfResponses = 0;
			
			boolean hasResponse = false;
			
			boolean hasResponse0 = false;
			if (blacksun_guri_condition__defaultCondition (player, npc))
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
					responses[responseIndex++] = new string_id (c_stringFile, "s_149");
				}
				
				utils.setScriptVar (player, "conversation.blacksun_guri.branchId", 52);
				
				npcStartConversation (player, npc, "blacksun_guri", message, responses);
			}
			else
			{
				chat.chat (npc, player, message);
			}
			
			return SCRIPT_CONTINUE;
		}
		
		if (blacksun_guri_condition__defaultCondition (player, npc))
		{
			
			string_id message = new string_id (c_stringFile, "s_157");
			chat.chat (npc, player, message);
			
			return SCRIPT_CONTINUE;
		}
		
		chat.chat (npc, "Error: All conditions for OnStartNpcConversation were false.");
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int OnNpcConversationResponse(obj_id self, String conversationId, obj_id player, string_id response) throws InterruptedException
	{
		if (!conversationId.equals("blacksun_guri"))
		{
			return SCRIPT_CONTINUE;
		}
		
		obj_id npc = self;
		
		int branchId = utils.getIntScriptVar (player, "conversation.blacksun_guri.branchId");
		
		if (branchId == 1 && blacksun_guri_handleBranch1 (player, npc, response) == SCRIPT_CONTINUE)
		{
			return SCRIPT_CONTINUE;
		}
		
		if (branchId == 2 && blacksun_guri_handleBranch2 (player, npc, response) == SCRIPT_CONTINUE)
		{
			return SCRIPT_CONTINUE;
		}
		
		if (branchId == 3 && blacksun_guri_handleBranch3 (player, npc, response) == SCRIPT_CONTINUE)
		{
			return SCRIPT_CONTINUE;
		}
		
		if (branchId == 4 && blacksun_guri_handleBranch4 (player, npc, response) == SCRIPT_CONTINUE)
		{
			return SCRIPT_CONTINUE;
		}
		
		if (branchId == 8 && blacksun_guri_handleBranch8 (player, npc, response) == SCRIPT_CONTINUE)
		{
			return SCRIPT_CONTINUE;
		}
		
		if (branchId == 9 && blacksun_guri_handleBranch9 (player, npc, response) == SCRIPT_CONTINUE)
		{
			return SCRIPT_CONTINUE;
		}
		
		if (branchId == 10 && blacksun_guri_handleBranch10 (player, npc, response) == SCRIPT_CONTINUE)
		{
			return SCRIPT_CONTINUE;
		}
		
		if (branchId == 15 && blacksun_guri_handleBranch15 (player, npc, response) == SCRIPT_CONTINUE)
		{
			return SCRIPT_CONTINUE;
		}
		
		if (branchId == 16 && blacksun_guri_handleBranch16 (player, npc, response) == SCRIPT_CONTINUE)
		{
			return SCRIPT_CONTINUE;
		}
		
		if (branchId == 17 && blacksun_guri_handleBranch17 (player, npc, response) == SCRIPT_CONTINUE)
		{
			return SCRIPT_CONTINUE;
		}
		
		if (branchId == 19 && blacksun_guri_handleBranch19 (player, npc, response) == SCRIPT_CONTINUE)
		{
			return SCRIPT_CONTINUE;
		}
		
		if (branchId == 20 && blacksun_guri_handleBranch20 (player, npc, response) == SCRIPT_CONTINUE)
		{
			return SCRIPT_CONTINUE;
		}
		
		if (branchId == 21 && blacksun_guri_handleBranch21 (player, npc, response) == SCRIPT_CONTINUE)
		{
			return SCRIPT_CONTINUE;
		}
		
		if (branchId == 22 && blacksun_guri_handleBranch22 (player, npc, response) == SCRIPT_CONTINUE)
		{
			return SCRIPT_CONTINUE;
		}
		
		if (branchId == 24 && blacksun_guri_handleBranch24 (player, npc, response) == SCRIPT_CONTINUE)
		{
			return SCRIPT_CONTINUE;
		}
		
		if (branchId == 25 && blacksun_guri_handleBranch25 (player, npc, response) == SCRIPT_CONTINUE)
		{
			return SCRIPT_CONTINUE;
		}
		
		if (branchId == 26 && blacksun_guri_handleBranch26 (player, npc, response) == SCRIPT_CONTINUE)
		{
			return SCRIPT_CONTINUE;
		}
		
		if (branchId == 27 && blacksun_guri_handleBranch27 (player, npc, response) == SCRIPT_CONTINUE)
		{
			return SCRIPT_CONTINUE;
		}
		
		if (branchId == 28 && blacksun_guri_handleBranch28 (player, npc, response) == SCRIPT_CONTINUE)
		{
			return SCRIPT_CONTINUE;
		}
		
		if (branchId == 29 && blacksun_guri_handleBranch29 (player, npc, response) == SCRIPT_CONTINUE)
		{
			return SCRIPT_CONTINUE;
		}
		
		if (branchId == 30 && blacksun_guri_handleBranch30 (player, npc, response) == SCRIPT_CONTINUE)
		{
			return SCRIPT_CONTINUE;
		}
		
		if (branchId == 31 && blacksun_guri_handleBranch31 (player, npc, response) == SCRIPT_CONTINUE)
		{
			return SCRIPT_CONTINUE;
		}
		
		if (branchId == 32 && blacksun_guri_handleBranch32 (player, npc, response) == SCRIPT_CONTINUE)
		{
			return SCRIPT_CONTINUE;
		}
		
		if (branchId == 33 && blacksun_guri_handleBranch33 (player, npc, response) == SCRIPT_CONTINUE)
		{
			return SCRIPT_CONTINUE;
		}
		
		if (branchId == 34 && blacksun_guri_handleBranch34 (player, npc, response) == SCRIPT_CONTINUE)
		{
			return SCRIPT_CONTINUE;
		}
		
		if (branchId == 35 && blacksun_guri_handleBranch35 (player, npc, response) == SCRIPT_CONTINUE)
		{
			return SCRIPT_CONTINUE;
		}
		
		if (branchId == 36 && blacksun_guri_handleBranch36 (player, npc, response) == SCRIPT_CONTINUE)
		{
			return SCRIPT_CONTINUE;
		}
		
		if (branchId == 37 && blacksun_guri_handleBranch37 (player, npc, response) == SCRIPT_CONTINUE)
		{
			return SCRIPT_CONTINUE;
		}
		
		if (branchId == 38 && blacksun_guri_handleBranch38 (player, npc, response) == SCRIPT_CONTINUE)
		{
			return SCRIPT_CONTINUE;
		}
		
		if (branchId == 39 && blacksun_guri_handleBranch39 (player, npc, response) == SCRIPT_CONTINUE)
		{
			return SCRIPT_CONTINUE;
		}
		
		if (branchId == 40 && blacksun_guri_handleBranch40 (player, npc, response) == SCRIPT_CONTINUE)
		{
			return SCRIPT_CONTINUE;
		}
		
		if (branchId == 41 && blacksun_guri_handleBranch41 (player, npc, response) == SCRIPT_CONTINUE)
		{
			return SCRIPT_CONTINUE;
		}
		
		if (branchId == 42 && blacksun_guri_handleBranch42 (player, npc, response) == SCRIPT_CONTINUE)
		{
			return SCRIPT_CONTINUE;
		}
		
		if (branchId == 43 && blacksun_guri_handleBranch43 (player, npc, response) == SCRIPT_CONTINUE)
		{
			return SCRIPT_CONTINUE;
		}
		
		if (branchId == 44 && blacksun_guri_handleBranch44 (player, npc, response) == SCRIPT_CONTINUE)
		{
			return SCRIPT_CONTINUE;
		}
		
		if (branchId == 45 && blacksun_guri_handleBranch45 (player, npc, response) == SCRIPT_CONTINUE)
		{
			return SCRIPT_CONTINUE;
		}
		
		if (branchId == 46 && blacksun_guri_handleBranch46 (player, npc, response) == SCRIPT_CONTINUE)
		{
			return SCRIPT_CONTINUE;
		}
		
		if (branchId == 47 && blacksun_guri_handleBranch47 (player, npc, response) == SCRIPT_CONTINUE)
		{
			return SCRIPT_CONTINUE;
		}
		
		if (branchId == 48 && blacksun_guri_handleBranch48 (player, npc, response) == SCRIPT_CONTINUE)
		{
			return SCRIPT_CONTINUE;
		}
		
		if (branchId == 49 && blacksun_guri_handleBranch49 (player, npc, response) == SCRIPT_CONTINUE)
		{
			return SCRIPT_CONTINUE;
		}
		
		if (branchId == 50 && blacksun_guri_handleBranch50 (player, npc, response) == SCRIPT_CONTINUE)
		{
			return SCRIPT_CONTINUE;
		}
		
		if (branchId == 52 && blacksun_guri_handleBranch52 (player, npc, response) == SCRIPT_CONTINUE)
		{
			return SCRIPT_CONTINUE;
		}
		
		chat.chat (npc, "Error: Fell through all branches and responses for OnNpcConversationResponse.");
		
		utils.removeScriptVar (player, "conversation.blacksun_guri.branchId");
		
		return SCRIPT_CONTINUE;
	}
	
}
