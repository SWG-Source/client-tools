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


public class captain_piett extends script.base_script
{
	public captain_piett()
	{
	}
	String c_stringFile = "conversation/captain_piett";
	
	
	public boolean captain_piett_condition__defaultCondition(obj_id player, obj_id npc) throws InterruptedException
	{
		return true;
	}
	
	
	public boolean captain_piett_condition_hasNotStartedQuestLine(obj_id player, obj_id npc) throws InterruptedException
	{
		faceTo(npc, player);
		return (!groundquests.isQuestActiveOrComplete(player, "outbreak_quest_01_imperial") && !groundquests.isQuestActiveOrComplete(player, "outbreak_quest_01_rebel") && !groundquests.isQuestActiveOrComplete(player, "outbreak_quest_01_neutral"));
	}
	
	
	public boolean captain_piett_condition_hasFirstQuestImperial(obj_id player, obj_id npc) throws InterruptedException
	{
		faceTo(npc, player);
		return groundquests.isQuestActive(player, "outbreak_quest_01_imperial");
	}
	
	
	public boolean captain_piett_condition_hasSecondQuestImperial(obj_id player, obj_id npc) throws InterruptedException
	{
		faceTo(npc, player);
		return groundquests.isQuestActive(player, "outbreak_quest_02_imperial");
	}
	
	
	public boolean captain_piett_condition_hasFoundStormtroopers(obj_id player, obj_id npc) throws InterruptedException
	{
		faceTo(npc, player);
		return groundquests.hasCompletedQuest(player, "outbreak_quest_02_imperial");
	}
	
	
	public boolean captain_piett_condition_hasCompletedMissions(obj_id player, obj_id npc) throws InterruptedException
	{
		return groundquests.isTaskActive(player, "outbreak_quest_facility_05_imperial", "speakPietteDebrief");
	}
	
	
	public boolean captain_piett_condition_hasCompletedAll(obj_id player, obj_id npc) throws InterruptedException
	{
		return groundquests.hasCompletedQuest(player, "outbreak_quest_facility_05_imperial") && groundquests.hasCompletedQuest(player, "outbreak_quest_final_imperial");
	}
	
	
	public boolean captain_piett_condition_isLiveConversion(obj_id player, obj_id npc) throws InterruptedException
	{
		
		return captain_piett_condition_hasNotStartedQuestLine(player,npc) && (groundquests.isTaskActive(player, "outbreak_live_conversion_imperial", "speakPiett") || groundquests.hasCompletedQuest(player, "outbreak_live_conversion_imperial")) || (groundquests.isTaskActive(player, "outbreak_switch_to_imperial", "speakPiett") || groundquests.hasCompletedQuest(player, "outbreak_switch_to_imperial"));
	}
	
	
	public boolean captain_piett_condition_isPlayerNeutral(obj_id player, obj_id npc) throws InterruptedException
	{
		return captain_piett_condition_isLiveConversion(player, npc) && !factions.isImperial(player) && !factions.isRebel(player);
	}
	
	
	public boolean captain_piett_condition_isPlayerRebel(obj_id player, obj_id npc) throws InterruptedException
	{
		return captain_piett_condition_isLiveConversion(player, npc) && factions.isRebel(player);
	}
	
	
	public boolean captain_piett_condition_hasDeletedLiveConversion(obj_id player, obj_id npc) throws InterruptedException
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
	
	
	public void captain_piett_action_grantMissionOne(obj_id player, obj_id npc) throws InterruptedException
	{
		faceTo(npc, player);
		groundquests.grantQuest(player, "quest/outbreak_quest_01_imperial");
	}
	
	
	public void captain_piett_action_completeQuest5(obj_id player, obj_id npc) throws InterruptedException
	{
		groundquests.sendSignal(player, "hasSpokenPietteDebreif");
		if (!hasCompletedCollectionSlot(player, "outbreak_flare_s_slot"))
		{
			modifyCollectionSlotValue(player, "outbreak_flare_s_slot", 1);
		}
	}
	
	
	public void captain_piett_action_clearLiveConversion(obj_id player, obj_id npc) throws InterruptedException
	{
		if ((!groundquests.isTaskActive(player, "outbreak_live_conversion_imperial", "speakPiett") && !groundquests.isTaskActive(player, "outbreak_switch_to_imperial", "speakPiett")))
		{
			return;
		}
		groundquests.sendSignal(player, "liveConversionSpokePiett");
	}
	
	
	public void captain_piett_action_revokeImpGrantReb(obj_id player, obj_id npc) throws InterruptedException
	{
		if (groundquests.isQuestActiveOrComplete(player, "outbreak_live_conversion_imperial"))
		{
			groundquests.clearQuest(player, "outbreak_live_conversion_imperial");
			groundquests.grantQuest(player, "outbreak_switch_to_rebel");
		}
		if (groundquests.isQuestActiveOrComplete(player, "outbreak_switch_to_imperial"))
		{
			groundquests.clearQuest(player, "outbreak_switch_to_imperial");
			groundquests.grantQuest(player, "outbreak_switch_to_rebel");
		}
	}
	
	
	public void captain_piett_action_revokeImpGrantNeut(obj_id player, obj_id npc) throws InterruptedException
	{
		if (groundquests.isQuestActiveOrComplete(player, "outbreak_live_conversion_imperial"))
		{
			groundquests.clearQuest(player, "outbreak_live_conversion_imperial");
			groundquests.grantQuest(player, "outbreak_switch_to_neutral");
		}
		if (groundquests.isQuestActiveOrComplete(player, "outbreak_switch_to_imperial"))
		{
			groundquests.clearQuest(player, "outbreak_switch_to_imperial");
			groundquests.grantQuest(player, "outbreak_switch_to_neutral");
		}
	}
	
	
	public void captain_piett_action_revokeEntireImpQuestLine(obj_id player, obj_id npc) throws InterruptedException
	{
		if (groundquests.isQuestActiveOrComplete(player, "outbreak_live_conversion_imperial"))
		{
			groundquests.clearQuest(player, "outbreak_live_conversion_imperial");
		}
		
		if (groundquests.isQuestActiveOrComplete(player, "outbreak_switch_to_imperial"))
		{
			groundquests.clearQuest(player, "outbreak_switch_to_imperial");
		}
		
		if (groundquests.isQuestActiveOrComplete(player, "outbreak_quest_01_imperial"))
		{
			groundquests.clearQuest(player, "outbreak_quest_01_imperial");
		}
		
		if (groundquests.isQuestActiveOrComplete(player, "outbreak_quest_01_b_imperial"))
		{
			groundquests.clearQuest(player, "outbreak_quest_01_b_imperial");
		}
		
		if (groundquests.isQuestActiveOrComplete(player, "outbreak_quest_02_imperial"))
		{
			groundquests.clearQuest(player, "outbreak_quest_02_imperial");
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
		
		if (groundquests.isQuestActiveOrComplete(player, "outbreak_quest_administrative_building_imperial_01"))
		{
			groundquests.clearQuest(player, "outbreak_quest_administrative_building_imperial_01");
		}
		
		if (groundquests.isQuestActiveOrComplete(player, "outbreak_quest_administrative_building_imperial_02"))
		{
			groundquests.clearQuest(player, "outbreak_quest_administrative_building_imperial_02");
		}
		
		if (groundquests.isQuestActiveOrComplete(player, "outbreak_quest_administrative_building_imperial_03"))
		{
			groundquests.clearQuest(player, "outbreak_quest_administrative_building_imperial_03");
		}
		
		if (groundquests.isQuestActiveOrComplete(player, "outbreak_quest_administrative_building_imperial_04"))
		{
			groundquests.clearQuest(player, "outbreak_quest_administrative_building_imperial_04");
		}
		
		if (groundquests.isQuestActiveOrComplete(player, "outbreak_quest_facility_01"))
		{
			groundquests.clearQuest(player, "outbreak_quest_facility_01");
		}
		
		if (groundquests.isQuestActiveOrComplete(player, "outbreak_quest_facility_02"))
		{
			groundquests.clearQuest(player, "outbreak_quest_facility_02");
		}
		
		if (groundquests.isQuestActiveOrComplete(player, "outbreak_quest_facility_02_imperial"))
		{
			groundquests.clearQuest(player, "outbreak_quest_facility_02_imperial");
		}
		
		if (groundquests.isQuestActiveOrComplete(player, "outbreak_quest_facility_03"))
		{
			groundquests.clearQuest(player, "outbreak_quest_facility_03");
		}
		
		if (groundquests.isQuestActiveOrComplete(player, "outbreak_quest_facility_04"))
		{
			groundquests.clearQuest(player, "outbreak_quest_facility_04");
		}
		
		if (groundquests.isQuestActiveOrComplete(player, "outbreak_quest_facility_05_imperial"))
		{
			groundquests.clearQuest(player, "outbreak_quest_facility_05_imperial");
		}
		
		if (groundquests.isQuestActiveOrComplete(player, "outbreak_quest_final_imperial"))
		{
			groundquests.clearQuest(player, "outbreak_quest_final_imperial");
		}
		
	}
	
	
	public void captain_piett_action_grantRebQuest(obj_id player, obj_id npc) throws InterruptedException
	{
		
		groundquests.grantQuest(player, "outbreak_switch_to_rebel");
	}
	
	
	public void captain_piett_action_grantImpQuest(obj_id player, obj_id npc) throws InterruptedException
	{
		groundquests.grantQuest(player, "outbreak_switch_to_imperial");
	}
	
	
	public void captain_piett_action_grantNeutQuest(obj_id player, obj_id npc) throws InterruptedException
	{
		groundquests.grantQuest(player, "outbreak_switch_to_neutral");
	}
	
	
	public int captain_piett_handleBranch1(obj_id player, obj_id npc, string_id response) throws InterruptedException
	{
		
		if (response.equals("s_157"))
		{
			
			if (captain_piett_condition__defaultCondition (player, npc))
			{
				
				string_id message = new string_id (c_stringFile, "s_158");
				int numberOfResponses = 0;
				
				boolean hasResponse = false;
				
				boolean hasResponse0 = false;
				if (captain_piett_condition__defaultCondition (player, npc))
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
						responses[responseIndex++] = new string_id (c_stringFile, "s_159");
					}
					
					utils.setScriptVar (player, "conversation.captain_piett.branchId", 2);
					
					npcSpeak (player, message);
					npcSetConversationResponses (player, responses);
				}
				else
				{
					utils.removeScriptVar (player, "conversation.captain_piett.branchId");
					
					npcEndConversationWithMessage (player, message);
				}
				
				return SCRIPT_CONTINUE;
			}
			
		}
		
		return SCRIPT_DEFAULT;
	}
	
	
	public int captain_piett_handleBranch2(obj_id player, obj_id npc, string_id response) throws InterruptedException
	{
		
		if (response.equals("s_159"))
		{
			
			if (captain_piett_condition__defaultCondition (player, npc))
			{
				
				string_id message = new string_id (c_stringFile, "s_160");
				int numberOfResponses = 0;
				
				boolean hasResponse = false;
				
				boolean hasResponse0 = false;
				if (captain_piett_condition__defaultCondition (player, npc))
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
						responses[responseIndex++] = new string_id (c_stringFile, "s_161");
					}
					
					utils.setScriptVar (player, "conversation.captain_piett.branchId", 3);
					
					prose_package pp = new prose_package ();
					pp.stringId = message;
					pp.actor.set (player);
					pp.target.set (npc);
					
					npcSpeak (player, pp);
					npcSetConversationResponses (player, responses);
				}
				else
				{
					utils.removeScriptVar (player, "conversation.captain_piett.branchId");
					
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
	
	
	public int captain_piett_handleBranch3(obj_id player, obj_id npc, string_id response) throws InterruptedException
	{
		
		if (response.equals("s_161"))
		{
			
			if (captain_piett_condition__defaultCondition (player, npc))
			{
				captain_piett_action_revokeEntireImpQuestLine (player, npc);
				
				string_id message = new string_id (c_stringFile, "s_162");
				int numberOfResponses = 0;
				
				boolean hasResponse = false;
				
				boolean hasResponse0 = false;
				if (captain_piett_condition__defaultCondition (player, npc))
				{
					++numberOfResponses;
					hasResponse = true;
					hasResponse0 = true;
				}
				
				boolean hasResponse1 = false;
				if (captain_piett_condition__defaultCondition (player, npc))
				{
					++numberOfResponses;
					hasResponse = true;
					hasResponse1 = true;
				}
				
				boolean hasResponse2 = false;
				if (captain_piett_condition__defaultCondition (player, npc))
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
						responses[responseIndex++] = new string_id (c_stringFile, "s_163");
					}
					
					if (hasResponse1)
					{
						responses[responseIndex++] = new string_id (c_stringFile, "s_165");
					}
					
					if (hasResponse2)
					{
						responses[responseIndex++] = new string_id (c_stringFile, "s_166");
					}
					
					utils.setScriptVar (player, "conversation.captain_piett.branchId", 4);
					
					npcSpeak (player, message);
					npcSetConversationResponses (player, responses);
				}
				else
				{
					utils.removeScriptVar (player, "conversation.captain_piett.branchId");
					
					npcEndConversationWithMessage (player, message);
				}
				
				return SCRIPT_CONTINUE;
			}
			
		}
		
		return SCRIPT_DEFAULT;
	}
	
	
	public int captain_piett_handleBranch4(obj_id player, obj_id npc, string_id response) throws InterruptedException
	{
		
		if (response.equals("s_163"))
		{
			
			if (captain_piett_condition__defaultCondition (player, npc))
			{
				captain_piett_action_grantImpQuest (player, npc);
				
				string_id message = new string_id (c_stringFile, "s_164");
				utils.removeScriptVar (player, "conversation.captain_piett.branchId");
				
				npcEndConversationWithMessage (player, message);
				
				return SCRIPT_CONTINUE;
			}
			
		}
		
		if (response.equals("s_165"))
		{
			
			if (captain_piett_condition__defaultCondition (player, npc))
			{
				captain_piett_action_grantRebQuest (player, npc);
				
				string_id message = new string_id (c_stringFile, "s_167");
				utils.removeScriptVar (player, "conversation.captain_piett.branchId");
				
				npcEndConversationWithMessage (player, message);
				
				return SCRIPT_CONTINUE;
			}
			
		}
		
		if (response.equals("s_166"))
		{
			
			if (captain_piett_condition__defaultCondition (player, npc))
			{
				captain_piett_action_grantNeutQuest (player, npc);
				
				string_id message = new string_id (c_stringFile, "s_168");
				utils.removeScriptVar (player, "conversation.captain_piett.branchId");
				
				npcEndConversationWithMessage (player, message);
				
				return SCRIPT_CONTINUE;
			}
			
		}
		
		return SCRIPT_DEFAULT;
	}
	
	
	public int captain_piett_handleBranch8(obj_id player, obj_id npc, string_id response) throws InterruptedException
	{
		
		if (response.equals("s_124"))
		{
			
			if (captain_piett_condition__defaultCondition (player, npc))
			{
				doAnimationAction (npc, "handshake_tandem");
				
				doAnimationAction (player, "handshake_tandem");
				
				string_id message = new string_id (c_stringFile, "s_125");
				int numberOfResponses = 0;
				
				boolean hasResponse = false;
				
				boolean hasResponse0 = false;
				if (captain_piett_condition__defaultCondition (player, npc))
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
					
					utils.setScriptVar (player, "conversation.captain_piett.branchId", 9);
					
					npcSpeak (player, message);
					npcSetConversationResponses (player, responses);
				}
				else
				{
					utils.removeScriptVar (player, "conversation.captain_piett.branchId");
					
					npcEndConversationWithMessage (player, message);
				}
				
				return SCRIPT_CONTINUE;
			}
			
		}
		
		return SCRIPT_DEFAULT;
	}
	
	
	public int captain_piett_handleBranch9(obj_id player, obj_id npc, string_id response) throws InterruptedException
	{
		
		if (response.equals("s_126"))
		{
			
			if (captain_piett_condition__defaultCondition (player, npc))
			{
				
				string_id message = new string_id (c_stringFile, "s_127");
				int numberOfResponses = 0;
				
				boolean hasResponse = false;
				
				boolean hasResponse0 = false;
				if (captain_piett_condition__defaultCondition (player, npc))
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
					
					utils.setScriptVar (player, "conversation.captain_piett.branchId", 10);
					
					npcSpeak (player, message);
					npcSetConversationResponses (player, responses);
				}
				else
				{
					utils.removeScriptVar (player, "conversation.captain_piett.branchId");
					
					npcEndConversationWithMessage (player, message);
				}
				
				return SCRIPT_CONTINUE;
			}
			
		}
		
		return SCRIPT_DEFAULT;
	}
	
	
	public int captain_piett_handleBranch10(obj_id player, obj_id npc, string_id response) throws InterruptedException
	{
		
		if (response.equals("s_128"))
		{
			
			if (captain_piett_condition__defaultCondition (player, npc))
			{
				doAnimationAction (npc, "salute1");
				
				doAnimationAction (player, "salute1");
				
				captain_piett_action_completeQuest5 (player, npc);
				
				string_id message = new string_id (c_stringFile, "s_129");
				utils.removeScriptVar (player, "conversation.captain_piett.branchId");
				
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
	
	
	public int captain_piett_handleBranch15(obj_id player, obj_id npc, string_id response) throws InterruptedException
	{
		
		if (response.equals("s_29"))
		{
			doAnimationAction (player, "salute1");
			
			if (captain_piett_condition_isPlayerNeutral (player, npc))
			{
				
				string_id message = new string_id (c_stringFile, "s_139");
				int numberOfResponses = 0;
				
				boolean hasResponse = false;
				
				boolean hasResponse0 = false;
				if (captain_piett_condition__defaultCondition (player, npc))
				{
					++numberOfResponses;
					hasResponse = true;
					hasResponse0 = true;
				}
				
				boolean hasResponse1 = false;
				if (captain_piett_condition__defaultCondition (player, npc))
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
						responses[responseIndex++] = new string_id (c_stringFile, "s_141");
					}
					
					if (hasResponse1)
					{
						responses[responseIndex++] = new string_id (c_stringFile, "s_143");
					}
					
					utils.setScriptVar (player, "conversation.captain_piett.branchId", 16);
					
					npcSpeak (player, message);
					npcSetConversationResponses (player, responses);
				}
				else
				{
					utils.removeScriptVar (player, "conversation.captain_piett.branchId");
					
					npcEndConversationWithMessage (player, message);
				}
				
				return SCRIPT_CONTINUE;
			}
			
			if (captain_piett_condition_isPlayerRebel (player, npc))
			{
				
				string_id message = new string_id (c_stringFile, "s_140");
				int numberOfResponses = 0;
				
				boolean hasResponse = false;
				
				boolean hasResponse0 = false;
				if (captain_piett_condition__defaultCondition (player, npc))
				{
					++numberOfResponses;
					hasResponse = true;
					hasResponse0 = true;
				}
				
				boolean hasResponse1 = false;
				if (captain_piett_condition__defaultCondition (player, npc))
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
						responses[responseIndex++] = new string_id (c_stringFile, "s_142");
					}
					
					if (hasResponse1)
					{
						responses[responseIndex++] = new string_id (c_stringFile, "s_144");
					}
					
					utils.setScriptVar (player, "conversation.captain_piett.branchId", 21);
					
					npcSpeak (player, message);
					npcSetConversationResponses (player, responses);
				}
				else
				{
					utils.removeScriptVar (player, "conversation.captain_piett.branchId");
					
					npcEndConversationWithMessage (player, message);
				}
				
				return SCRIPT_CONTINUE;
			}
			
			if (captain_piett_condition__defaultCondition (player, npc))
			{
				
				string_id message = new string_id (c_stringFile, "s_49");
				int numberOfResponses = 0;
				
				boolean hasResponse = false;
				
				boolean hasResponse0 = false;
				if (captain_piett_condition__defaultCondition (player, npc))
				{
					++numberOfResponses;
					hasResponse = true;
					hasResponse0 = true;
				}
				
				boolean hasResponse1 = false;
				if (captain_piett_condition__defaultCondition (player, npc))
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
						responses[responseIndex++] = new string_id (c_stringFile, "s_51");
					}
					
					if (hasResponse1)
					{
						responses[responseIndex++] = new string_id (c_stringFile, "s_192");
					}
					
					utils.setScriptVar (player, "conversation.captain_piett.branchId", 26);
					
					npcSpeak (player, message);
					npcSetConversationResponses (player, responses);
				}
				else
				{
					utils.removeScriptVar (player, "conversation.captain_piett.branchId");
					
					npcEndConversationWithMessage (player, message);
				}
				
				return SCRIPT_CONTINUE;
			}
			
		}
		
		return SCRIPT_DEFAULT;
	}
	
	
	public int captain_piett_handleBranch16(obj_id player, obj_id npc, string_id response) throws InterruptedException
	{
		
		if (response.equals("s_141"))
		{
			
			if (captain_piett_condition__defaultCondition (player, npc))
			{
				
				string_id message = new string_id (c_stringFile, "s_145");
				int numberOfResponses = 0;
				
				boolean hasResponse = false;
				
				boolean hasResponse0 = false;
				if (captain_piett_condition__defaultCondition (player, npc))
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
					
					utils.setScriptVar (player, "conversation.captain_piett.branchId", 17);
					
					npcSpeak (player, message);
					npcSetConversationResponses (player, responses);
				}
				else
				{
					utils.removeScriptVar (player, "conversation.captain_piett.branchId");
					
					npcEndConversationWithMessage (player, message);
				}
				
				return SCRIPT_CONTINUE;
			}
			
		}
		
		if (response.equals("s_143"))
		{
			
			if (captain_piett_condition__defaultCondition (player, npc))
			{
				
				string_id message = new string_id (c_stringFile, "s_146");
				int numberOfResponses = 0;
				
				boolean hasResponse = false;
				
				boolean hasResponse0 = false;
				if (captain_piett_condition__defaultCondition (player, npc))
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
						responses[responseIndex++] = new string_id (c_stringFile, "s_150");
					}
					
					utils.setScriptVar (player, "conversation.captain_piett.branchId", 19);
					
					npcSpeak (player, message);
					npcSetConversationResponses (player, responses);
				}
				else
				{
					utils.removeScriptVar (player, "conversation.captain_piett.branchId");
					
					npcEndConversationWithMessage (player, message);
				}
				
				return SCRIPT_CONTINUE;
			}
			
		}
		
		return SCRIPT_DEFAULT;
	}
	
	
	public int captain_piett_handleBranch17(obj_id player, obj_id npc, string_id response) throws InterruptedException
	{
		
		if (response.equals("s_149"))
		{
			
			if (captain_piett_condition__defaultCondition (player, npc))
			{
				captain_piett_action_revokeImpGrantNeut (player, npc);
				
				string_id message = new string_id (c_stringFile, "s_153");
				utils.removeScriptVar (player, "conversation.captain_piett.branchId");
				
				npcEndConversationWithMessage (player, message);
				
				return SCRIPT_CONTINUE;
			}
			
		}
		
		return SCRIPT_DEFAULT;
	}
	
	
	public int captain_piett_handleBranch19(obj_id player, obj_id npc, string_id response) throws InterruptedException
	{
		
		if (response.equals("s_150"))
		{
			
			if (captain_piett_condition__defaultCondition (player, npc))
			{
				
				string_id message = new string_id (c_stringFile, "s_156");
				int numberOfResponses = 0;
				
				boolean hasResponse = false;
				
				boolean hasResponse0 = false;
				if (captain_piett_condition__defaultCondition (player, npc))
				{
					++numberOfResponses;
					hasResponse = true;
					hasResponse0 = true;
				}
				
				boolean hasResponse1 = false;
				if (captain_piett_condition__defaultCondition (player, npc))
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
						responses[responseIndex++] = new string_id (c_stringFile, "s_51");
					}
					
					if (hasResponse1)
					{
						responses[responseIndex++] = new string_id (c_stringFile, "s_192");
					}
					
					utils.setScriptVar (player, "conversation.captain_piett.branchId", 26);
					
					prose_package pp = new prose_package ();
					pp.stringId = message;
					pp.actor.set (player);
					pp.target.set (npc);
					
					npcSpeak (player, pp);
					npcSetConversationResponses (player, responses);
				}
				else
				{
					utils.removeScriptVar (player, "conversation.captain_piett.branchId");
					
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
	
	
	public int captain_piett_handleBranch20(obj_id player, obj_id npc, string_id response) throws InterruptedException
	{
		
		if (response.equals("s_51"))
		{
			
			if (captain_piett_condition__defaultCondition (player, npc))
			{
				
				string_id message = new string_id (c_stringFile, "s_53");
				int numberOfResponses = 0;
				
				boolean hasResponse = false;
				
				boolean hasResponse0 = false;
				if (captain_piett_condition__defaultCondition (player, npc))
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
						responses[responseIndex++] = new string_id (c_stringFile, "s_55");
					}
					
					utils.setScriptVar (player, "conversation.captain_piett.branchId", 27);
					
					npcSpeak (player, message);
					npcSetConversationResponses (player, responses);
				}
				else
				{
					utils.removeScriptVar (player, "conversation.captain_piett.branchId");
					
					npcEndConversationWithMessage (player, message);
				}
				
				return SCRIPT_CONTINUE;
			}
			
		}
		
		if (response.equals("s_192"))
		{
			
			if (captain_piett_condition__defaultCondition (player, npc))
			{
				
				string_id message = new string_id (c_stringFile, "s_194");
				int numberOfResponses = 0;
				
				boolean hasResponse = false;
				
				boolean hasResponse0 = false;
				if (captain_piett_condition__defaultCondition (player, npc))
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
						responses[responseIndex++] = new string_id (c_stringFile, "s_196");
					}
					
					utils.setScriptVar (player, "conversation.captain_piett.branchId", 49);
					
					npcSpeak (player, message);
					npcSetConversationResponses (player, responses);
				}
				else
				{
					utils.removeScriptVar (player, "conversation.captain_piett.branchId");
					
					npcEndConversationWithMessage (player, message);
				}
				
				return SCRIPT_CONTINUE;
			}
			
		}
		
		return SCRIPT_DEFAULT;
	}
	
	
	public int captain_piett_handleBranch21(obj_id player, obj_id npc, string_id response) throws InterruptedException
	{
		
		if (response.equals("s_142"))
		{
			
			if (captain_piett_condition__defaultCondition (player, npc))
			{
				
				string_id message = new string_id (c_stringFile, "s_147");
				int numberOfResponses = 0;
				
				boolean hasResponse = false;
				
				boolean hasResponse0 = false;
				if (captain_piett_condition__defaultCondition (player, npc))
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
						responses[responseIndex++] = new string_id (c_stringFile, "s_151");
					}
					
					utils.setScriptVar (player, "conversation.captain_piett.branchId", 22);
					
					npcSpeak (player, message);
					npcSetConversationResponses (player, responses);
				}
				else
				{
					utils.removeScriptVar (player, "conversation.captain_piett.branchId");
					
					npcEndConversationWithMessage (player, message);
				}
				
				return SCRIPT_CONTINUE;
			}
			
		}
		
		if (response.equals("s_144"))
		{
			
			if (captain_piett_condition__defaultCondition (player, npc))
			{
				
				string_id message = new string_id (c_stringFile, "s_148");
				int numberOfResponses = 0;
				
				boolean hasResponse = false;
				
				boolean hasResponse0 = false;
				if (captain_piett_condition__defaultCondition (player, npc))
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
					
					utils.setScriptVar (player, "conversation.captain_piett.branchId", 24);
					
					npcSpeak (player, message);
					npcSetConversationResponses (player, responses);
				}
				else
				{
					utils.removeScriptVar (player, "conversation.captain_piett.branchId");
					
					npcEndConversationWithMessage (player, message);
				}
				
				return SCRIPT_CONTINUE;
			}
			
		}
		
		return SCRIPT_DEFAULT;
	}
	
	
	public int captain_piett_handleBranch22(obj_id player, obj_id npc, string_id response) throws InterruptedException
	{
		
		if (response.equals("s_151"))
		{
			
			if (captain_piett_condition__defaultCondition (player, npc))
			{
				captain_piett_action_revokeImpGrantReb (player, npc);
				
				string_id message = new string_id (c_stringFile, "s_154");
				utils.removeScriptVar (player, "conversation.captain_piett.branchId");
				
				npcEndConversationWithMessage (player, message);
				
				return SCRIPT_CONTINUE;
			}
			
		}
		
		return SCRIPT_DEFAULT;
	}
	
	
	public int captain_piett_handleBranch24(obj_id player, obj_id npc, string_id response) throws InterruptedException
	{
		
		if (response.equals("s_152"))
		{
			
			if (captain_piett_condition__defaultCondition (player, npc))
			{
				
				string_id message = new string_id (c_stringFile, "s_155");
				int numberOfResponses = 0;
				
				boolean hasResponse = false;
				
				boolean hasResponse0 = false;
				if (captain_piett_condition__defaultCondition (player, npc))
				{
					++numberOfResponses;
					hasResponse = true;
					hasResponse0 = true;
				}
				
				boolean hasResponse1 = false;
				if (captain_piett_condition__defaultCondition (player, npc))
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
						responses[responseIndex++] = new string_id (c_stringFile, "s_51");
					}
					
					if (hasResponse1)
					{
						responses[responseIndex++] = new string_id (c_stringFile, "s_192");
					}
					
					utils.setScriptVar (player, "conversation.captain_piett.branchId", 26);
					
					prose_package pp = new prose_package ();
					pp.stringId = message;
					pp.actor.set (player);
					pp.target.set (npc);
					
					npcSpeak (player, pp);
					npcSetConversationResponses (player, responses);
				}
				else
				{
					utils.removeScriptVar (player, "conversation.captain_piett.branchId");
					
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
	
	
	public int captain_piett_handleBranch25(obj_id player, obj_id npc, string_id response) throws InterruptedException
	{
		
		if (response.equals("s_51"))
		{
			
			if (captain_piett_condition__defaultCondition (player, npc))
			{
				
				string_id message = new string_id (c_stringFile, "s_53");
				int numberOfResponses = 0;
				
				boolean hasResponse = false;
				
				boolean hasResponse0 = false;
				if (captain_piett_condition__defaultCondition (player, npc))
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
						responses[responseIndex++] = new string_id (c_stringFile, "s_55");
					}
					
					utils.setScriptVar (player, "conversation.captain_piett.branchId", 27);
					
					npcSpeak (player, message);
					npcSetConversationResponses (player, responses);
				}
				else
				{
					utils.removeScriptVar (player, "conversation.captain_piett.branchId");
					
					npcEndConversationWithMessage (player, message);
				}
				
				return SCRIPT_CONTINUE;
			}
			
		}
		
		if (response.equals("s_192"))
		{
			
			if (captain_piett_condition__defaultCondition (player, npc))
			{
				
				string_id message = new string_id (c_stringFile, "s_194");
				int numberOfResponses = 0;
				
				boolean hasResponse = false;
				
				boolean hasResponse0 = false;
				if (captain_piett_condition__defaultCondition (player, npc))
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
						responses[responseIndex++] = new string_id (c_stringFile, "s_196");
					}
					
					utils.setScriptVar (player, "conversation.captain_piett.branchId", 49);
					
					npcSpeak (player, message);
					npcSetConversationResponses (player, responses);
				}
				else
				{
					utils.removeScriptVar (player, "conversation.captain_piett.branchId");
					
					npcEndConversationWithMessage (player, message);
				}
				
				return SCRIPT_CONTINUE;
			}
			
		}
		
		return SCRIPT_DEFAULT;
	}
	
	
	public int captain_piett_handleBranch26(obj_id player, obj_id npc, string_id response) throws InterruptedException
	{
		
		if (response.equals("s_51"))
		{
			
			if (captain_piett_condition__defaultCondition (player, npc))
			{
				
				string_id message = new string_id (c_stringFile, "s_53");
				int numberOfResponses = 0;
				
				boolean hasResponse = false;
				
				boolean hasResponse0 = false;
				if (captain_piett_condition__defaultCondition (player, npc))
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
						responses[responseIndex++] = new string_id (c_stringFile, "s_55");
					}
					
					utils.setScriptVar (player, "conversation.captain_piett.branchId", 27);
					
					npcSpeak (player, message);
					npcSetConversationResponses (player, responses);
				}
				else
				{
					utils.removeScriptVar (player, "conversation.captain_piett.branchId");
					
					npcEndConversationWithMessage (player, message);
				}
				
				return SCRIPT_CONTINUE;
			}
			
		}
		
		if (response.equals("s_192"))
		{
			
			if (captain_piett_condition__defaultCondition (player, npc))
			{
				
				string_id message = new string_id (c_stringFile, "s_194");
				int numberOfResponses = 0;
				
				boolean hasResponse = false;
				
				boolean hasResponse0 = false;
				if (captain_piett_condition__defaultCondition (player, npc))
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
						responses[responseIndex++] = new string_id (c_stringFile, "s_196");
					}
					
					utils.setScriptVar (player, "conversation.captain_piett.branchId", 49);
					
					npcSpeak (player, message);
					npcSetConversationResponses (player, responses);
				}
				else
				{
					utils.removeScriptVar (player, "conversation.captain_piett.branchId");
					
					npcEndConversationWithMessage (player, message);
				}
				
				return SCRIPT_CONTINUE;
			}
			
		}
		
		return SCRIPT_DEFAULT;
	}
	
	
	public int captain_piett_handleBranch27(obj_id player, obj_id npc, string_id response) throws InterruptedException
	{
		
		if (response.equals("s_55"))
		{
			
			if (captain_piett_condition__defaultCondition (player, npc))
			{
				
				string_id message = new string_id (c_stringFile, "s_57");
				int numberOfResponses = 0;
				
				boolean hasResponse = false;
				
				boolean hasResponse0 = false;
				if (captain_piett_condition__defaultCondition (player, npc))
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
					
					utils.setScriptVar (player, "conversation.captain_piett.branchId", 28);
					
					npcSpeak (player, message);
					npcSetConversationResponses (player, responses);
				}
				else
				{
					utils.removeScriptVar (player, "conversation.captain_piett.branchId");
					
					npcEndConversationWithMessage (player, message);
				}
				
				return SCRIPT_CONTINUE;
			}
			
		}
		
		return SCRIPT_DEFAULT;
	}
	
	
	public int captain_piett_handleBranch28(obj_id player, obj_id npc, string_id response) throws InterruptedException
	{
		
		if (response.equals("s_59"))
		{
			
			if (captain_piett_condition__defaultCondition (player, npc))
			{
				
				string_id message = new string_id (c_stringFile, "s_61");
				int numberOfResponses = 0;
				
				boolean hasResponse = false;
				
				boolean hasResponse0 = false;
				if (captain_piett_condition__defaultCondition (player, npc))
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
						responses[responseIndex++] = new string_id (c_stringFile, "s_63");
					}
					
					utils.setScriptVar (player, "conversation.captain_piett.branchId", 29);
					
					npcSpeak (player, message);
					npcSetConversationResponses (player, responses);
				}
				else
				{
					utils.removeScriptVar (player, "conversation.captain_piett.branchId");
					
					npcEndConversationWithMessage (player, message);
				}
				
				return SCRIPT_CONTINUE;
			}
			
		}
		
		return SCRIPT_DEFAULT;
	}
	
	
	public int captain_piett_handleBranch29(obj_id player, obj_id npc, string_id response) throws InterruptedException
	{
		
		if (response.equals("s_63"))
		{
			
			if (captain_piett_condition__defaultCondition (player, npc))
			{
				doAnimationAction (npc, "nervous");
				
				string_id message = new string_id (c_stringFile, "s_65");
				int numberOfResponses = 0;
				
				boolean hasResponse = false;
				
				boolean hasResponse0 = false;
				if (captain_piett_condition__defaultCondition (player, npc))
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
					
					utils.setScriptVar (player, "conversation.captain_piett.branchId", 30);
					
					npcSpeak (player, message);
					npcSetConversationResponses (player, responses);
				}
				else
				{
					utils.removeScriptVar (player, "conversation.captain_piett.branchId");
					
					npcEndConversationWithMessage (player, message);
				}
				
				return SCRIPT_CONTINUE;
			}
			
		}
		
		return SCRIPT_DEFAULT;
	}
	
	
	public int captain_piett_handleBranch30(obj_id player, obj_id npc, string_id response) throws InterruptedException
	{
		
		if (response.equals("s_67"))
		{
			
			if (captain_piett_condition__defaultCondition (player, npc))
			{
				
				string_id message = new string_id (c_stringFile, "s_69");
				int numberOfResponses = 0;
				
				boolean hasResponse = false;
				
				boolean hasResponse0 = false;
				if (captain_piett_condition__defaultCondition (player, npc))
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
					
					utils.setScriptVar (player, "conversation.captain_piett.branchId", 31);
					
					npcSpeak (player, message);
					npcSetConversationResponses (player, responses);
				}
				else
				{
					utils.removeScriptVar (player, "conversation.captain_piett.branchId");
					
					npcEndConversationWithMessage (player, message);
				}
				
				return SCRIPT_CONTINUE;
			}
			
		}
		
		return SCRIPT_DEFAULT;
	}
	
	
	public int captain_piett_handleBranch31(obj_id player, obj_id npc, string_id response) throws InterruptedException
	{
		
		if (response.equals("s_71"))
		{
			
			if (captain_piett_condition__defaultCondition (player, npc))
			{
				
				string_id message = new string_id (c_stringFile, "s_73");
				int numberOfResponses = 0;
				
				boolean hasResponse = false;
				
				boolean hasResponse0 = false;
				if (captain_piett_condition__defaultCondition (player, npc))
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
						responses[responseIndex++] = new string_id (c_stringFile, "s_76");
					}
					
					utils.setScriptVar (player, "conversation.captain_piett.branchId", 32);
					
					prose_package pp = new prose_package ();
					pp.stringId = message;
					pp.actor.set (player);
					pp.target.set (npc);
					
					npcSpeak (player, pp);
					npcSetConversationResponses (player, responses);
				}
				else
				{
					utils.removeScriptVar (player, "conversation.captain_piett.branchId");
					
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
	
	
	public int captain_piett_handleBranch32(obj_id player, obj_id npc, string_id response) throws InterruptedException
	{
		
		if (response.equals("s_76"))
		{
			
			if (captain_piett_condition__defaultCondition (player, npc))
			{
				
				string_id message = new string_id (c_stringFile, "s_78");
				int numberOfResponses = 0;
				
				boolean hasResponse = false;
				
				boolean hasResponse0 = false;
				if (captain_piett_condition__defaultCondition (player, npc))
				{
					++numberOfResponses;
					hasResponse = true;
					hasResponse0 = true;
				}
				
				boolean hasResponse1 = false;
				if (captain_piett_condition__defaultCondition (player, npc))
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
						responses[responseIndex++] = new string_id (c_stringFile, "s_80");
					}
					
					if (hasResponse1)
					{
						responses[responseIndex++] = new string_id (c_stringFile, "s_190");
					}
					
					utils.setScriptVar (player, "conversation.captain_piett.branchId", 33);
					
					npcSpeak (player, message);
					npcSetConversationResponses (player, responses);
				}
				else
				{
					utils.removeScriptVar (player, "conversation.captain_piett.branchId");
					
					npcEndConversationWithMessage (player, message);
				}
				
				return SCRIPT_CONTINUE;
			}
			
		}
		
		return SCRIPT_DEFAULT;
	}
	
	
	public int captain_piett_handleBranch33(obj_id player, obj_id npc, string_id response) throws InterruptedException
	{
		
		if (response.equals("s_80"))
		{
			
			if (captain_piett_condition__defaultCondition (player, npc))
			{
				
				string_id message = new string_id (c_stringFile, "s_82");
				int numberOfResponses = 0;
				
				boolean hasResponse = false;
				
				boolean hasResponse0 = false;
				if (captain_piett_condition__defaultCondition (player, npc))
				{
					++numberOfResponses;
					hasResponse = true;
					hasResponse0 = true;
				}
				
				boolean hasResponse1 = false;
				if (captain_piett_condition__defaultCondition (player, npc))
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
						responses[responseIndex++] = new string_id (c_stringFile, "s_84");
					}
					
					if (hasResponse1)
					{
						responses[responseIndex++] = new string_id (c_stringFile, "s_184");
					}
					
					utils.setScriptVar (player, "conversation.captain_piett.branchId", 34);
					
					npcSpeak (player, message);
					npcSetConversationResponses (player, responses);
				}
				else
				{
					utils.removeScriptVar (player, "conversation.captain_piett.branchId");
					
					npcEndConversationWithMessage (player, message);
				}
				
				return SCRIPT_CONTINUE;
			}
			
		}
		
		if (response.equals("s_190"))
		{
			
			if (captain_piett_condition__defaultCondition (player, npc))
			{
				
				string_id message = new string_id (c_stringFile, "s_86");
				int numberOfResponses = 0;
				
				boolean hasResponse = false;
				
				boolean hasResponse0 = false;
				if (captain_piett_condition__defaultCondition (player, npc))
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
					
					utils.setScriptVar (player, "conversation.captain_piett.branchId", 35);
					
					npcSpeak (player, message);
					npcSetConversationResponses (player, responses);
				}
				else
				{
					utils.removeScriptVar (player, "conversation.captain_piett.branchId");
					
					npcEndConversationWithMessage (player, message);
				}
				
				return SCRIPT_CONTINUE;
			}
			
		}
		
		return SCRIPT_DEFAULT;
	}
	
	
	public int captain_piett_handleBranch34(obj_id player, obj_id npc, string_id response) throws InterruptedException
	{
		
		if (response.equals("s_84"))
		{
			
			if (captain_piett_condition__defaultCondition (player, npc))
			{
				
				string_id message = new string_id (c_stringFile, "s_86");
				int numberOfResponses = 0;
				
				boolean hasResponse = false;
				
				boolean hasResponse0 = false;
				if (captain_piett_condition__defaultCondition (player, npc))
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
					
					utils.setScriptVar (player, "conversation.captain_piett.branchId", 35);
					
					npcSpeak (player, message);
					npcSetConversationResponses (player, responses);
				}
				else
				{
					utils.removeScriptVar (player, "conversation.captain_piett.branchId");
					
					npcEndConversationWithMessage (player, message);
				}
				
				return SCRIPT_CONTINUE;
			}
			
		}
		
		if (response.equals("s_184"))
		{
			
			if (captain_piett_condition__defaultCondition (player, npc))
			{
				
				string_id message = new string_id (c_stringFile, "s_186");
				int numberOfResponses = 0;
				
				boolean hasResponse = false;
				
				boolean hasResponse0 = false;
				if (captain_piett_condition__defaultCondition (player, npc))
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
						responses[responseIndex++] = new string_id (c_stringFile, "s_188");
					}
					
					utils.setScriptVar (player, "conversation.captain_piett.branchId", 48);
					
					npcSpeak (player, message);
					npcSetConversationResponses (player, responses);
				}
				else
				{
					utils.removeScriptVar (player, "conversation.captain_piett.branchId");
					
					npcEndConversationWithMessage (player, message);
				}
				
				return SCRIPT_CONTINUE;
			}
			
		}
		
		return SCRIPT_DEFAULT;
	}
	
	
	public int captain_piett_handleBranch35(obj_id player, obj_id npc, string_id response) throws InterruptedException
	{
		
		if (response.equals("s_88"))
		{
			
			if (captain_piett_condition__defaultCondition (player, npc))
			{
				
				string_id message = new string_id (c_stringFile, "s_90");
				int numberOfResponses = 0;
				
				boolean hasResponse = false;
				
				boolean hasResponse0 = false;
				if (captain_piett_condition__defaultCondition (player, npc))
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
					
					utils.setScriptVar (player, "conversation.captain_piett.branchId", 36);
					
					npcSpeak (player, message);
					npcSetConversationResponses (player, responses);
				}
				else
				{
					utils.removeScriptVar (player, "conversation.captain_piett.branchId");
					
					npcEndConversationWithMessage (player, message);
				}
				
				return SCRIPT_CONTINUE;
			}
			
		}
		
		return SCRIPT_DEFAULT;
	}
	
	
	public int captain_piett_handleBranch36(obj_id player, obj_id npc, string_id response) throws InterruptedException
	{
		
		if (response.equals("s_92"))
		{
			
			if (captain_piett_condition__defaultCondition (player, npc))
			{
				
				string_id message = new string_id (c_stringFile, "s_94");
				int numberOfResponses = 0;
				
				boolean hasResponse = false;
				
				boolean hasResponse0 = false;
				if (captain_piett_condition__defaultCondition (player, npc))
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
						responses[responseIndex++] = new string_id (c_stringFile, "s_96");
					}
					
					utils.setScriptVar (player, "conversation.captain_piett.branchId", 37);
					
					npcSpeak (player, message);
					npcSetConversationResponses (player, responses);
				}
				else
				{
					utils.removeScriptVar (player, "conversation.captain_piett.branchId");
					
					npcEndConversationWithMessage (player, message);
				}
				
				return SCRIPT_CONTINUE;
			}
			
		}
		
		return SCRIPT_DEFAULT;
	}
	
	
	public int captain_piett_handleBranch37(obj_id player, obj_id npc, string_id response) throws InterruptedException
	{
		
		if (response.equals("s_96"))
		{
			
			if (captain_piett_condition__defaultCondition (player, npc))
			{
				
				string_id message = new string_id (c_stringFile, "s_98");
				int numberOfResponses = 0;
				
				boolean hasResponse = false;
				
				boolean hasResponse0 = false;
				if (captain_piett_condition__defaultCondition (player, npc))
				{
					++numberOfResponses;
					hasResponse = true;
					hasResponse0 = true;
				}
				
				boolean hasResponse1 = false;
				if (captain_piett_condition__defaultCondition (player, npc))
				{
					++numberOfResponses;
					hasResponse = true;
					hasResponse1 = true;
				}
				
				boolean hasResponse2 = false;
				if (captain_piett_condition__defaultCondition (player, npc))
				{
					++numberOfResponses;
					hasResponse = true;
					hasResponse2 = true;
				}
				
				boolean hasResponse3 = false;
				if (captain_piett_condition__defaultCondition (player, npc))
				{
					++numberOfResponses;
					hasResponse = true;
					hasResponse3 = true;
				}
				
				boolean hasResponse4 = false;
				if (captain_piett_condition__defaultCondition (player, npc))
				{
					++numberOfResponses;
					hasResponse = true;
					hasResponse4 = true;
				}
				
				if (hasResponse)
				{
					int responseIndex = 0;
					string_id responses[] = new string_id[numberOfResponses];
					
					if (hasResponse0)
					{
						responses[responseIndex++] = new string_id (c_stringFile, "s_100");
					}
					
					if (hasResponse1)
					{
						responses[responseIndex++] = new string_id (c_stringFile, "s_114");
					}
					
					if (hasResponse2)
					{
						responses[responseIndex++] = new string_id (c_stringFile, "s_134");
					}
					
					if (hasResponse3)
					{
						responses[responseIndex++] = new string_id (c_stringFile, "s_174");
					}
					
					if (hasResponse4)
					{
						responses[responseIndex++] = new string_id (c_stringFile, "s_180");
					}
					
					utils.setScriptVar (player, "conversation.captain_piett.branchId", 38);
					
					npcSpeak (player, message);
					npcSetConversationResponses (player, responses);
				}
				else
				{
					utils.removeScriptVar (player, "conversation.captain_piett.branchId");
					
					npcEndConversationWithMessage (player, message);
				}
				
				return SCRIPT_CONTINUE;
			}
			
		}
		
		return SCRIPT_DEFAULT;
	}
	
	
	public int captain_piett_handleBranch38(obj_id player, obj_id npc, string_id response) throws InterruptedException
	{
		
		if (response.equals("s_100"))
		{
			
			if (captain_piett_condition__defaultCondition (player, npc))
			{
				
				string_id message = new string_id (c_stringFile, "s_102");
				int numberOfResponses = 0;
				
				boolean hasResponse = false;
				
				boolean hasResponse0 = false;
				if (captain_piett_condition__defaultCondition (player, npc))
				{
					++numberOfResponses;
					hasResponse = true;
					hasResponse0 = true;
				}
				
				boolean hasResponse1 = false;
				if (captain_piett_condition__defaultCondition (player, npc))
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
						responses[responseIndex++] = new string_id (c_stringFile, "s_104");
					}
					
					if (hasResponse1)
					{
						responses[responseIndex++] = new string_id (c_stringFile, "s_108");
					}
					
					utils.setScriptVar (player, "conversation.captain_piett.branchId", 39);
					
					npcSpeak (player, message);
					npcSetConversationResponses (player, responses);
				}
				else
				{
					utils.removeScriptVar (player, "conversation.captain_piett.branchId");
					
					npcEndConversationWithMessage (player, message);
				}
				
				return SCRIPT_CONTINUE;
			}
			
		}
		
		if (response.equals("s_114"))
		{
			
			if (captain_piett_condition__defaultCondition (player, npc))
			{
				
				string_id message = new string_id (c_stringFile, "s_116");
				int numberOfResponses = 0;
				
				boolean hasResponse = false;
				
				boolean hasResponse0 = false;
				if (captain_piett_condition__defaultCondition (player, npc))
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
						responses[responseIndex++] = new string_id (c_stringFile, "s_118");
					}
					
					utils.setScriptVar (player, "conversation.captain_piett.branchId", 42);
					
					npcSpeak (player, message);
					npcSetConversationResponses (player, responses);
				}
				else
				{
					utils.removeScriptVar (player, "conversation.captain_piett.branchId");
					
					npcEndConversationWithMessage (player, message);
				}
				
				return SCRIPT_CONTINUE;
			}
			
		}
		
		if (response.equals("s_134"))
		{
			
			if (captain_piett_condition__defaultCondition (player, npc))
			{
				
				string_id message = new string_id (c_stringFile, "s_136");
				int numberOfResponses = 0;
				
				boolean hasResponse = false;
				
				boolean hasResponse0 = false;
				if (captain_piett_condition__defaultCondition (player, npc))
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
					
					utils.setScriptVar (player, "conversation.captain_piett.branchId", 44);
					
					npcSpeak (player, message);
					npcSetConversationResponses (player, responses);
				}
				else
				{
					utils.removeScriptVar (player, "conversation.captain_piett.branchId");
					
					npcEndConversationWithMessage (player, message);
				}
				
				return SCRIPT_CONTINUE;
			}
			
		}
		
		if (response.equals("s_174"))
		{
			
			if (captain_piett_condition__defaultCondition (player, npc))
			{
				
				string_id message = new string_id (c_stringFile, "s_176");
				int numberOfResponses = 0;
				
				boolean hasResponse = false;
				
				boolean hasResponse0 = false;
				if (captain_piett_condition__defaultCondition (player, npc))
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
						responses[responseIndex++] = new string_id (c_stringFile, "s_178");
					}
					
					utils.setScriptVar (player, "conversation.captain_piett.branchId", 46);
					
					npcSpeak (player, message);
					npcSetConversationResponses (player, responses);
				}
				else
				{
					utils.removeScriptVar (player, "conversation.captain_piett.branchId");
					
					npcEndConversationWithMessage (player, message);
				}
				
				return SCRIPT_CONTINUE;
			}
			
		}
		
		if (response.equals("s_180"))
		{
			
			if (captain_piett_condition__defaultCondition (player, npc))
			{
				captain_piett_action_grantMissionOne (player, npc);
				
				string_id message = new string_id (c_stringFile, "s_182");
				utils.removeScriptVar (player, "conversation.captain_piett.branchId");
				
				npcEndConversationWithMessage (player, message);
				
				return SCRIPT_CONTINUE;
			}
			
		}
		
		return SCRIPT_DEFAULT;
	}
	
	
	public int captain_piett_handleBranch39(obj_id player, obj_id npc, string_id response) throws InterruptedException
	{
		
		if (response.equals("s_104"))
		{
			
			if (captain_piett_condition__defaultCondition (player, npc))
			{
				captain_piett_action_grantMissionOne (player, npc);
				
				string_id message = new string_id (c_stringFile, "s_106");
				utils.removeScriptVar (player, "conversation.captain_piett.branchId");
				
				npcEndConversationWithMessage (player, message);
				
				return SCRIPT_CONTINUE;
			}
			
		}
		
		if (response.equals("s_108"))
		{
			
			if (captain_piett_condition__defaultCondition (player, npc))
			{
				
				string_id message = new string_id (c_stringFile, "s_110");
				int numberOfResponses = 0;
				
				boolean hasResponse = false;
				
				boolean hasResponse0 = false;
				if (captain_piett_condition__defaultCondition (player, npc))
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
						responses[responseIndex++] = new string_id (c_stringFile, "s_112");
					}
					
					utils.setScriptVar (player, "conversation.captain_piett.branchId", 41);
					
					npcSpeak (player, message);
					npcSetConversationResponses (player, responses);
				}
				else
				{
					utils.removeScriptVar (player, "conversation.captain_piett.branchId");
					
					npcEndConversationWithMessage (player, message);
				}
				
				return SCRIPT_CONTINUE;
			}
			
		}
		
		return SCRIPT_DEFAULT;
	}
	
	
	public int captain_piett_handleBranch41(obj_id player, obj_id npc, string_id response) throws InterruptedException
	{
		
		if (response.equals("s_112"))
		{
			
			if (captain_piett_condition__defaultCondition (player, npc))
			{
				
				string_id message = new string_id (c_stringFile, "s_98");
				int numberOfResponses = 0;
				
				boolean hasResponse = false;
				
				boolean hasResponse0 = false;
				if (captain_piett_condition__defaultCondition (player, npc))
				{
					++numberOfResponses;
					hasResponse = true;
					hasResponse0 = true;
				}
				
				boolean hasResponse1 = false;
				if (captain_piett_condition__defaultCondition (player, npc))
				{
					++numberOfResponses;
					hasResponse = true;
					hasResponse1 = true;
				}
				
				boolean hasResponse2 = false;
				if (captain_piett_condition__defaultCondition (player, npc))
				{
					++numberOfResponses;
					hasResponse = true;
					hasResponse2 = true;
				}
				
				boolean hasResponse3 = false;
				if (captain_piett_condition__defaultCondition (player, npc))
				{
					++numberOfResponses;
					hasResponse = true;
					hasResponse3 = true;
				}
				
				boolean hasResponse4 = false;
				if (captain_piett_condition__defaultCondition (player, npc))
				{
					++numberOfResponses;
					hasResponse = true;
					hasResponse4 = true;
				}
				
				if (hasResponse)
				{
					int responseIndex = 0;
					string_id responses[] = new string_id[numberOfResponses];
					
					if (hasResponse0)
					{
						responses[responseIndex++] = new string_id (c_stringFile, "s_100");
					}
					
					if (hasResponse1)
					{
						responses[responseIndex++] = new string_id (c_stringFile, "s_114");
					}
					
					if (hasResponse2)
					{
						responses[responseIndex++] = new string_id (c_stringFile, "s_134");
					}
					
					if (hasResponse3)
					{
						responses[responseIndex++] = new string_id (c_stringFile, "s_174");
					}
					
					if (hasResponse4)
					{
						responses[responseIndex++] = new string_id (c_stringFile, "s_180");
					}
					
					utils.setScriptVar (player, "conversation.captain_piett.branchId", 38);
					
					npcSpeak (player, message);
					npcSetConversationResponses (player, responses);
				}
				else
				{
					utils.removeScriptVar (player, "conversation.captain_piett.branchId");
					
					npcEndConversationWithMessage (player, message);
				}
				
				return SCRIPT_CONTINUE;
			}
			
		}
		
		return SCRIPT_DEFAULT;
	}
	
	
	public int captain_piett_handleBranch42(obj_id player, obj_id npc, string_id response) throws InterruptedException
	{
		
		if (response.equals("s_118"))
		{
			
			if (captain_piett_condition__defaultCondition (player, npc))
			{
				
				string_id message = new string_id (c_stringFile, "s_120");
				int numberOfResponses = 0;
				
				boolean hasResponse = false;
				
				boolean hasResponse0 = false;
				if (captain_piett_condition__defaultCondition (player, npc))
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
						responses[responseIndex++] = new string_id (c_stringFile, "s_132");
					}
					
					utils.setScriptVar (player, "conversation.captain_piett.branchId", 43);
					
					npcSpeak (player, message);
					npcSetConversationResponses (player, responses);
				}
				else
				{
					utils.removeScriptVar (player, "conversation.captain_piett.branchId");
					
					npcEndConversationWithMessage (player, message);
				}
				
				return SCRIPT_CONTINUE;
			}
			
		}
		
		return SCRIPT_DEFAULT;
	}
	
	
	public int captain_piett_handleBranch43(obj_id player, obj_id npc, string_id response) throws InterruptedException
	{
		
		if (response.equals("s_132"))
		{
			
			if (captain_piett_condition__defaultCondition (player, npc))
			{
				
				string_id message = new string_id (c_stringFile, "s_98");
				int numberOfResponses = 0;
				
				boolean hasResponse = false;
				
				boolean hasResponse0 = false;
				if (captain_piett_condition__defaultCondition (player, npc))
				{
					++numberOfResponses;
					hasResponse = true;
					hasResponse0 = true;
				}
				
				boolean hasResponse1 = false;
				if (captain_piett_condition__defaultCondition (player, npc))
				{
					++numberOfResponses;
					hasResponse = true;
					hasResponse1 = true;
				}
				
				boolean hasResponse2 = false;
				if (captain_piett_condition__defaultCondition (player, npc))
				{
					++numberOfResponses;
					hasResponse = true;
					hasResponse2 = true;
				}
				
				boolean hasResponse3 = false;
				if (captain_piett_condition__defaultCondition (player, npc))
				{
					++numberOfResponses;
					hasResponse = true;
					hasResponse3 = true;
				}
				
				boolean hasResponse4 = false;
				if (captain_piett_condition__defaultCondition (player, npc))
				{
					++numberOfResponses;
					hasResponse = true;
					hasResponse4 = true;
				}
				
				if (hasResponse)
				{
					int responseIndex = 0;
					string_id responses[] = new string_id[numberOfResponses];
					
					if (hasResponse0)
					{
						responses[responseIndex++] = new string_id (c_stringFile, "s_100");
					}
					
					if (hasResponse1)
					{
						responses[responseIndex++] = new string_id (c_stringFile, "s_114");
					}
					
					if (hasResponse2)
					{
						responses[responseIndex++] = new string_id (c_stringFile, "s_134");
					}
					
					if (hasResponse3)
					{
						responses[responseIndex++] = new string_id (c_stringFile, "s_174");
					}
					
					if (hasResponse4)
					{
						responses[responseIndex++] = new string_id (c_stringFile, "s_180");
					}
					
					utils.setScriptVar (player, "conversation.captain_piett.branchId", 38);
					
					npcSpeak (player, message);
					npcSetConversationResponses (player, responses);
				}
				else
				{
					utils.removeScriptVar (player, "conversation.captain_piett.branchId");
					
					npcEndConversationWithMessage (player, message);
				}
				
				return SCRIPT_CONTINUE;
			}
			
		}
		
		return SCRIPT_DEFAULT;
	}
	
	
	public int captain_piett_handleBranch44(obj_id player, obj_id npc, string_id response) throws InterruptedException
	{
		
		if (response.equals("s_138"))
		{
			
			if (captain_piett_condition__defaultCondition (player, npc))
			{
				
				string_id message = new string_id (c_stringFile, "s_170");
				int numberOfResponses = 0;
				
				boolean hasResponse = false;
				
				boolean hasResponse0 = false;
				if (captain_piett_condition__defaultCondition (player, npc))
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
						responses[responseIndex++] = new string_id (c_stringFile, "s_172");
					}
					
					utils.setScriptVar (player, "conversation.captain_piett.branchId", 45);
					
					npcSpeak (player, message);
					npcSetConversationResponses (player, responses);
				}
				else
				{
					utils.removeScriptVar (player, "conversation.captain_piett.branchId");
					
					npcEndConversationWithMessage (player, message);
				}
				
				return SCRIPT_CONTINUE;
			}
			
		}
		
		return SCRIPT_DEFAULT;
	}
	
	
	public int captain_piett_handleBranch45(obj_id player, obj_id npc, string_id response) throws InterruptedException
	{
		
		if (response.equals("s_172"))
		{
			
			if (captain_piett_condition__defaultCondition (player, npc))
			{
				
				string_id message = new string_id (c_stringFile, "s_98");
				int numberOfResponses = 0;
				
				boolean hasResponse = false;
				
				boolean hasResponse0 = false;
				if (captain_piett_condition__defaultCondition (player, npc))
				{
					++numberOfResponses;
					hasResponse = true;
					hasResponse0 = true;
				}
				
				boolean hasResponse1 = false;
				if (captain_piett_condition__defaultCondition (player, npc))
				{
					++numberOfResponses;
					hasResponse = true;
					hasResponse1 = true;
				}
				
				boolean hasResponse2 = false;
				if (captain_piett_condition__defaultCondition (player, npc))
				{
					++numberOfResponses;
					hasResponse = true;
					hasResponse2 = true;
				}
				
				boolean hasResponse3 = false;
				if (captain_piett_condition__defaultCondition (player, npc))
				{
					++numberOfResponses;
					hasResponse = true;
					hasResponse3 = true;
				}
				
				boolean hasResponse4 = false;
				if (captain_piett_condition__defaultCondition (player, npc))
				{
					++numberOfResponses;
					hasResponse = true;
					hasResponse4 = true;
				}
				
				if (hasResponse)
				{
					int responseIndex = 0;
					string_id responses[] = new string_id[numberOfResponses];
					
					if (hasResponse0)
					{
						responses[responseIndex++] = new string_id (c_stringFile, "s_100");
					}
					
					if (hasResponse1)
					{
						responses[responseIndex++] = new string_id (c_stringFile, "s_114");
					}
					
					if (hasResponse2)
					{
						responses[responseIndex++] = new string_id (c_stringFile, "s_134");
					}
					
					if (hasResponse3)
					{
						responses[responseIndex++] = new string_id (c_stringFile, "s_174");
					}
					
					if (hasResponse4)
					{
						responses[responseIndex++] = new string_id (c_stringFile, "s_180");
					}
					
					utils.setScriptVar (player, "conversation.captain_piett.branchId", 38);
					
					npcSpeak (player, message);
					npcSetConversationResponses (player, responses);
				}
				else
				{
					utils.removeScriptVar (player, "conversation.captain_piett.branchId");
					
					npcEndConversationWithMessage (player, message);
				}
				
				return SCRIPT_CONTINUE;
			}
			
		}
		
		return SCRIPT_DEFAULT;
	}
	
	
	public int captain_piett_handleBranch46(obj_id player, obj_id npc, string_id response) throws InterruptedException
	{
		
		if (response.equals("s_178"))
		{
			
			if (captain_piett_condition__defaultCondition (player, npc))
			{
				
				string_id message = new string_id (c_stringFile, "s_98");
				int numberOfResponses = 0;
				
				boolean hasResponse = false;
				
				boolean hasResponse0 = false;
				if (captain_piett_condition__defaultCondition (player, npc))
				{
					++numberOfResponses;
					hasResponse = true;
					hasResponse0 = true;
				}
				
				boolean hasResponse1 = false;
				if (captain_piett_condition__defaultCondition (player, npc))
				{
					++numberOfResponses;
					hasResponse = true;
					hasResponse1 = true;
				}
				
				boolean hasResponse2 = false;
				if (captain_piett_condition__defaultCondition (player, npc))
				{
					++numberOfResponses;
					hasResponse = true;
					hasResponse2 = true;
				}
				
				boolean hasResponse3 = false;
				if (captain_piett_condition__defaultCondition (player, npc))
				{
					++numberOfResponses;
					hasResponse = true;
					hasResponse3 = true;
				}
				
				boolean hasResponse4 = false;
				if (captain_piett_condition__defaultCondition (player, npc))
				{
					++numberOfResponses;
					hasResponse = true;
					hasResponse4 = true;
				}
				
				if (hasResponse)
				{
					int responseIndex = 0;
					string_id responses[] = new string_id[numberOfResponses];
					
					if (hasResponse0)
					{
						responses[responseIndex++] = new string_id (c_stringFile, "s_100");
					}
					
					if (hasResponse1)
					{
						responses[responseIndex++] = new string_id (c_stringFile, "s_114");
					}
					
					if (hasResponse2)
					{
						responses[responseIndex++] = new string_id (c_stringFile, "s_134");
					}
					
					if (hasResponse3)
					{
						responses[responseIndex++] = new string_id (c_stringFile, "s_174");
					}
					
					if (hasResponse4)
					{
						responses[responseIndex++] = new string_id (c_stringFile, "s_180");
					}
					
					utils.setScriptVar (player, "conversation.captain_piett.branchId", 38);
					
					npcSpeak (player, message);
					npcSetConversationResponses (player, responses);
				}
				else
				{
					utils.removeScriptVar (player, "conversation.captain_piett.branchId");
					
					npcEndConversationWithMessage (player, message);
				}
				
				return SCRIPT_CONTINUE;
			}
			
		}
		
		return SCRIPT_DEFAULT;
	}
	
	
	public int captain_piett_handleBranch48(obj_id player, obj_id npc, string_id response) throws InterruptedException
	{
		
		if (response.equals("s_188"))
		{
			
			if (captain_piett_condition__defaultCondition (player, npc))
			{
				
				string_id message = new string_id (c_stringFile, "s_86");
				int numberOfResponses = 0;
				
				boolean hasResponse = false;
				
				boolean hasResponse0 = false;
				if (captain_piett_condition__defaultCondition (player, npc))
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
					
					utils.setScriptVar (player, "conversation.captain_piett.branchId", 35);
					
					npcSpeak (player, message);
					npcSetConversationResponses (player, responses);
				}
				else
				{
					utils.removeScriptVar (player, "conversation.captain_piett.branchId");
					
					npcEndConversationWithMessage (player, message);
				}
				
				return SCRIPT_CONTINUE;
			}
			
		}
		
		return SCRIPT_DEFAULT;
	}
	
	
	public int captain_piett_handleBranch49(obj_id player, obj_id npc, string_id response) throws InterruptedException
	{
		
		if (response.equals("s_196"))
		{
			
			if (captain_piett_condition__defaultCondition (player, npc))
			{
				
				string_id message = new string_id (c_stringFile, "s_78");
				int numberOfResponses = 0;
				
				boolean hasResponse = false;
				
				boolean hasResponse0 = false;
				if (captain_piett_condition__defaultCondition (player, npc))
				{
					++numberOfResponses;
					hasResponse = true;
					hasResponse0 = true;
				}
				
				boolean hasResponse1 = false;
				if (captain_piett_condition__defaultCondition (player, npc))
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
						responses[responseIndex++] = new string_id (c_stringFile, "s_80");
					}
					
					if (hasResponse1)
					{
						responses[responseIndex++] = new string_id (c_stringFile, "s_190");
					}
					
					utils.setScriptVar (player, "conversation.captain_piett.branchId", 33);
					
					npcSpeak (player, message);
					npcSetConversationResponses (player, responses);
				}
				else
				{
					utils.removeScriptVar (player, "conversation.captain_piett.branchId");
					
					npcEndConversationWithMessage (player, message);
				}
				
				return SCRIPT_CONTINUE;
			}
			
		}
		
		return SCRIPT_DEFAULT;
	}
	
	
	public int captain_piett_handleBranch50(obj_id player, obj_id npc, string_id response) throws InterruptedException
	{
		
		if (response.equals("s_200"))
		{
			
			if (captain_piett_condition__defaultCondition (player, npc))
			{
				captain_piett_action_grantImpQuest (player, npc);
				
				string_id message = new string_id (c_stringFile, "s_202");
				utils.removeScriptVar (player, "conversation.captain_piett.branchId");
				
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
			detachScript(self, "conversation.captain_piett");
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
		detachScript (self, "conversation.captain_piett");
		
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
		
		if (captain_piett_condition_hasCompletedAll (player, npc))
		{
			
			string_id message = new string_id (c_stringFile, "s_130");
			int numberOfResponses = 0;
			
			boolean hasResponse = false;
			
			boolean hasResponse0 = false;
			if (captain_piett_condition__defaultCondition (player, npc))
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
					responses[responseIndex++] = new string_id (c_stringFile, "s_157");
				}
				
				utils.setScriptVar (player, "conversation.captain_piett.branchId", 1);
				
				prose_package pp = new prose_package ();
				pp.stringId = message;
				pp.actor.set (player);
				pp.target.set (npc);
				
				npcStartConversation (player, npc, "captain_piett", null, pp, responses);
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
		
		if (captain_piett_condition_hasCompletedMissions (player, npc))
		{
			
			string_id message = new string_id (c_stringFile, "s_123");
			int numberOfResponses = 0;
			
			boolean hasResponse = false;
			
			boolean hasResponse0 = false;
			if (captain_piett_condition__defaultCondition (player, npc))
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
				
				utils.setScriptVar (player, "conversation.captain_piett.branchId", 8);
				
				npcStartConversation (player, npc, "captain_piett", message, responses);
			}
			else
			{
				chat.chat (npc, player, message);
			}
			
			return SCRIPT_CONTINUE;
		}
		
		if (captain_piett_condition_hasFoundStormtroopers (player, npc))
		{
			
			string_id message = new string_id (c_stringFile, "s_122");
			chat.chat (npc, player, message);
			
			return SCRIPT_CONTINUE;
		}
		
		if (captain_piett_condition_hasSecondQuestImperial (player, npc))
		{
			
			string_id message = new string_id (c_stringFile, "s_121");
			prose_package pp = new prose_package ();
			pp.stringId = message;
			pp.actor.set (player);
			pp.target.set (npc);
			
			chat.chat (npc, player, null, null, pp);
			return SCRIPT_CONTINUE;
		}
		
		if (captain_piett_condition_hasFirstQuestImperial (player, npc))
		{
			
			string_id message = new string_id (c_stringFile, "s_74");
			prose_package pp = new prose_package ();
			pp.stringId = message;
			pp.actor.set (player);
			pp.target.set (npc);
			
			chat.chat (npc, player, null, null, pp);
			return SCRIPT_CONTINUE;
		}
		
		if (captain_piett_condition_isLiveConversion (player, npc))
		{
			doAnimationAction (npc, "nod");
			
			captain_piett_action_clearLiveConversion (player, npc);
			
			string_id message = new string_id (c_stringFile, "s_27");
			int numberOfResponses = 0;
			
			boolean hasResponse = false;
			
			boolean hasResponse0 = false;
			if (captain_piett_condition__defaultCondition (player, npc))
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
				
				utils.setScriptVar (player, "conversation.captain_piett.branchId", 15);
				
				npcStartConversation (player, npc, "captain_piett", message, responses);
			}
			else
			{
				chat.chat (npc, player, message);
			}
			
			return SCRIPT_CONTINUE;
		}
		
		if (captain_piett_condition_hasDeletedLiveConversion (player, npc))
		{
			
			string_id message = new string_id (c_stringFile, "s_199");
			int numberOfResponses = 0;
			
			boolean hasResponse = false;
			
			boolean hasResponse0 = false;
			if (captain_piett_condition__defaultCondition (player, npc))
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
					responses[responseIndex++] = new string_id (c_stringFile, "s_200");
				}
				
				utils.setScriptVar (player, "conversation.captain_piett.branchId", 50);
				
				npcStartConversation (player, npc, "captain_piett", message, responses);
			}
			else
			{
				chat.chat (npc, player, message);
			}
			
			return SCRIPT_CONTINUE;
		}
		
		if (captain_piett_condition__defaultCondition (player, npc))
		{
			
			string_id message = new string_id (c_stringFile, "s_204");
			chat.chat (npc, player, message);
			
			return SCRIPT_CONTINUE;
		}
		
		chat.chat (npc, "Error: All conditions for OnStartNpcConversation were false.");
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int OnNpcConversationResponse(obj_id self, String conversationId, obj_id player, string_id response) throws InterruptedException
	{
		if (!conversationId.equals("captain_piett"))
		{
			return SCRIPT_CONTINUE;
		}
		
		obj_id npc = self;
		
		int branchId = utils.getIntScriptVar (player, "conversation.captain_piett.branchId");
		
		if (branchId == 1 && captain_piett_handleBranch1 (player, npc, response) == SCRIPT_CONTINUE)
		{
			return SCRIPT_CONTINUE;
		}
		
		if (branchId == 2 && captain_piett_handleBranch2 (player, npc, response) == SCRIPT_CONTINUE)
		{
			return SCRIPT_CONTINUE;
		}
		
		if (branchId == 3 && captain_piett_handleBranch3 (player, npc, response) == SCRIPT_CONTINUE)
		{
			return SCRIPT_CONTINUE;
		}
		
		if (branchId == 4 && captain_piett_handleBranch4 (player, npc, response) == SCRIPT_CONTINUE)
		{
			return SCRIPT_CONTINUE;
		}
		
		if (branchId == 8 && captain_piett_handleBranch8 (player, npc, response) == SCRIPT_CONTINUE)
		{
			return SCRIPT_CONTINUE;
		}
		
		if (branchId == 9 && captain_piett_handleBranch9 (player, npc, response) == SCRIPT_CONTINUE)
		{
			return SCRIPT_CONTINUE;
		}
		
		if (branchId == 10 && captain_piett_handleBranch10 (player, npc, response) == SCRIPT_CONTINUE)
		{
			return SCRIPT_CONTINUE;
		}
		
		if (branchId == 15 && captain_piett_handleBranch15 (player, npc, response) == SCRIPT_CONTINUE)
		{
			return SCRIPT_CONTINUE;
		}
		
		if (branchId == 16 && captain_piett_handleBranch16 (player, npc, response) == SCRIPT_CONTINUE)
		{
			return SCRIPT_CONTINUE;
		}
		
		if (branchId == 17 && captain_piett_handleBranch17 (player, npc, response) == SCRIPT_CONTINUE)
		{
			return SCRIPT_CONTINUE;
		}
		
		if (branchId == 19 && captain_piett_handleBranch19 (player, npc, response) == SCRIPT_CONTINUE)
		{
			return SCRIPT_CONTINUE;
		}
		
		if (branchId == 20 && captain_piett_handleBranch20 (player, npc, response) == SCRIPT_CONTINUE)
		{
			return SCRIPT_CONTINUE;
		}
		
		if (branchId == 21 && captain_piett_handleBranch21 (player, npc, response) == SCRIPT_CONTINUE)
		{
			return SCRIPT_CONTINUE;
		}
		
		if (branchId == 22 && captain_piett_handleBranch22 (player, npc, response) == SCRIPT_CONTINUE)
		{
			return SCRIPT_CONTINUE;
		}
		
		if (branchId == 24 && captain_piett_handleBranch24 (player, npc, response) == SCRIPT_CONTINUE)
		{
			return SCRIPT_CONTINUE;
		}
		
		if (branchId == 25 && captain_piett_handleBranch25 (player, npc, response) == SCRIPT_CONTINUE)
		{
			return SCRIPT_CONTINUE;
		}
		
		if (branchId == 26 && captain_piett_handleBranch26 (player, npc, response) == SCRIPT_CONTINUE)
		{
			return SCRIPT_CONTINUE;
		}
		
		if (branchId == 27 && captain_piett_handleBranch27 (player, npc, response) == SCRIPT_CONTINUE)
		{
			return SCRIPT_CONTINUE;
		}
		
		if (branchId == 28 && captain_piett_handleBranch28 (player, npc, response) == SCRIPT_CONTINUE)
		{
			return SCRIPT_CONTINUE;
		}
		
		if (branchId == 29 && captain_piett_handleBranch29 (player, npc, response) == SCRIPT_CONTINUE)
		{
			return SCRIPT_CONTINUE;
		}
		
		if (branchId == 30 && captain_piett_handleBranch30 (player, npc, response) == SCRIPT_CONTINUE)
		{
			return SCRIPT_CONTINUE;
		}
		
		if (branchId == 31 && captain_piett_handleBranch31 (player, npc, response) == SCRIPT_CONTINUE)
		{
			return SCRIPT_CONTINUE;
		}
		
		if (branchId == 32 && captain_piett_handleBranch32 (player, npc, response) == SCRIPT_CONTINUE)
		{
			return SCRIPT_CONTINUE;
		}
		
		if (branchId == 33 && captain_piett_handleBranch33 (player, npc, response) == SCRIPT_CONTINUE)
		{
			return SCRIPT_CONTINUE;
		}
		
		if (branchId == 34 && captain_piett_handleBranch34 (player, npc, response) == SCRIPT_CONTINUE)
		{
			return SCRIPT_CONTINUE;
		}
		
		if (branchId == 35 && captain_piett_handleBranch35 (player, npc, response) == SCRIPT_CONTINUE)
		{
			return SCRIPT_CONTINUE;
		}
		
		if (branchId == 36 && captain_piett_handleBranch36 (player, npc, response) == SCRIPT_CONTINUE)
		{
			return SCRIPT_CONTINUE;
		}
		
		if (branchId == 37 && captain_piett_handleBranch37 (player, npc, response) == SCRIPT_CONTINUE)
		{
			return SCRIPT_CONTINUE;
		}
		
		if (branchId == 38 && captain_piett_handleBranch38 (player, npc, response) == SCRIPT_CONTINUE)
		{
			return SCRIPT_CONTINUE;
		}
		
		if (branchId == 39 && captain_piett_handleBranch39 (player, npc, response) == SCRIPT_CONTINUE)
		{
			return SCRIPT_CONTINUE;
		}
		
		if (branchId == 41 && captain_piett_handleBranch41 (player, npc, response) == SCRIPT_CONTINUE)
		{
			return SCRIPT_CONTINUE;
		}
		
		if (branchId == 42 && captain_piett_handleBranch42 (player, npc, response) == SCRIPT_CONTINUE)
		{
			return SCRIPT_CONTINUE;
		}
		
		if (branchId == 43 && captain_piett_handleBranch43 (player, npc, response) == SCRIPT_CONTINUE)
		{
			return SCRIPT_CONTINUE;
		}
		
		if (branchId == 44 && captain_piett_handleBranch44 (player, npc, response) == SCRIPT_CONTINUE)
		{
			return SCRIPT_CONTINUE;
		}
		
		if (branchId == 45 && captain_piett_handleBranch45 (player, npc, response) == SCRIPT_CONTINUE)
		{
			return SCRIPT_CONTINUE;
		}
		
		if (branchId == 46 && captain_piett_handleBranch46 (player, npc, response) == SCRIPT_CONTINUE)
		{
			return SCRIPT_CONTINUE;
		}
		
		if (branchId == 48 && captain_piett_handleBranch48 (player, npc, response) == SCRIPT_CONTINUE)
		{
			return SCRIPT_CONTINUE;
		}
		
		if (branchId == 49 && captain_piett_handleBranch49 (player, npc, response) == SCRIPT_CONTINUE)
		{
			return SCRIPT_CONTINUE;
		}
		
		if (branchId == 50 && captain_piett_handleBranch50 (player, npc, response) == SCRIPT_CONTINUE)
		{
			return SCRIPT_CONTINUE;
		}
		
		chat.chat (npc, "Error: Fell through all branches and responses for OnNpcConversationResponse.");
		
		utils.removeScriptVar (player, "conversation.captain_piett.branchId");
		
		return SCRIPT_CONTINUE;
	}
	
}
