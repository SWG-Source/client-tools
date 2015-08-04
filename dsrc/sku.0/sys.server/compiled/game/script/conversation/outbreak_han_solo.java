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


public class outbreak_han_solo extends script.base_script
{
	public outbreak_han_solo()
	{
	}
	String c_stringFile = "conversation/outbreak_han_solo";
	
	
	public boolean outbreak_han_solo_condition__defaultCondition(obj_id player, obj_id npc) throws InterruptedException
	{
		return true;
	}
	
	
	public boolean outbreak_han_solo_condition_hasNotStartedQuestLine(obj_id player, obj_id npc) throws InterruptedException
	{
		faceTo(npc, player);
		return (!groundquests.isQuestActiveOrComplete(player, "outbreak_quest_01_imperial") && !groundquests.isQuestActiveOrComplete(player, "outbreak_quest_01_rebel") && !groundquests.isQuestActiveOrComplete(player, "outbreak_quest_01_neutral"));
	}
	
	
	public boolean outbreak_han_solo_condition_hasFirstQuestImperial(obj_id player, obj_id npc) throws InterruptedException
	{
		faceTo(npc, player);
		return groundquests.isQuestActive(player, "outbreak_quest_01_rebel");
	}
	
	
	public boolean outbreak_han_solo_condition_hasSecondQuestImperial(obj_id player, obj_id npc) throws InterruptedException
	{
		faceTo(npc, player);
		return groundquests.isQuestActive(player, "outbreak_quest_02_rebel");
	}
	
	
	public boolean outbreak_han_solo_condition_hasFoundStormtroopers(obj_id player, obj_id npc) throws InterruptedException
	{
		faceTo(npc, player);
		return groundquests.hasCompletedQuest(player, "outbreak_quest_02_rebel");
	}
	
	
	public boolean outbreak_han_solo_condition_hasCompletedMissions(obj_id player, obj_id npc) throws InterruptedException
	{
		return groundquests.isTaskActive(player, "outbreak_quest_facility_05_rebel", "speakPietteDebrief");
	}
	
	
	public boolean outbreak_han_solo_condition_hasCompletedAll(obj_id player, obj_id npc) throws InterruptedException
	{
		return groundquests.hasCompletedQuest(player, "outbreak_quest_final_rebel") && groundquests.hasCompletedQuest(player, "outbreak_quest_final_rebel");
	}
	
	
	public boolean outbreak_han_solo_condition_isLiveConversation(obj_id player, obj_id npc) throws InterruptedException
	{
		return outbreak_han_solo_condition_hasNotStartedQuestLine(player,npc) && (groundquests.isTaskActive(player, "outbreak_live_conversion_rebel", "speakSolo") || groundquests.hasCompletedQuest(player, "outbreak_live_conversion_rebel")) || (groundquests.isTaskActive(player, "outbreak_switch_to_rebel", "speakSolo") || groundquests.hasCompletedQuest(player, "outbreak_switch_to_rebel"));
	}
	
	
	public boolean outbreak_han_solo_condition_isPlayerImperial(obj_id player, obj_id npc) throws InterruptedException
	{
		return outbreak_han_solo_condition_isLiveConversation(player, npc) && factions.isImperial(player);
	}
	
	
	public boolean outbreak_han_solo_condition_isPlayerNeutral(obj_id player, obj_id npc) throws InterruptedException
	{
		return outbreak_han_solo_condition_isLiveConversation(player, npc) && !factions.isImperial(player) && !factions.isRebel(player);
	}
	
	
	public boolean outbreak_han_solo_condition_hasDeletedLiveConversion(obj_id player, obj_id npc) throws InterruptedException
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
	
	
	public void outbreak_han_solo_action_grantMissionOne(obj_id player, obj_id npc) throws InterruptedException
	{
		faceTo(npc, player);
		groundquests.grantQuest(player, "quest/outbreak_quest_01_rebel");
		outbreak_han_solo_action_makeChewieGoodLuck(player,npc);
	}
	
	
	public void outbreak_han_solo_action_completeQuest5(obj_id player, obj_id npc) throws InterruptedException
	{
		groundquests.sendSignal(player, "hasSpokenPietteDebreif");
		if (!hasCompletedCollectionSlot(player, "outbreak_flare_s_slot"))
		{
			modifyCollectionSlotValue(player, "outbreak_flare_s_slot", 1);
		}
	}
	
	
	public void outbreak_han_solo_action_makeChewieSayRight(obj_id player, obj_id npc) throws InterruptedException
	{
		if (!hasObjVar(npc, "myChewie"))
		{
			CustomerServiceLog("outbreak_themepark", "outbreak_han_solo conversation: Han has no Chewie objvar. Self: "+npc);
			return;
		}
		
		obj_id myChewie = getObjIdObjVar(npc, "myChewie");
		if (!isValidId(myChewie) || !exists(myChewie))
		{
			CustomerServiceLog("outbreak_themepark", "outbreak_han_solo conversation: Han has a Chewie objvar that is invalid. Self: "+ npc);
			return;
		}
		
		string_id wookSpeak = new string_id(STRINGFILE, "chewie_hrrrrrnnnn");
		chat.chat(myChewie, wookSpeak);
		faceTo(myChewie, player);
		playClientEffectObj(player, "sound/voc_shyriiwook_blurt_rnd.snd", player, "");
		doAnimationAction(myChewie, "scratch_head");
	}
	
	
	public void outbreak_han_solo_action_makeChewieCheer(obj_id player, obj_id npc) throws InterruptedException
	{
		if (!hasObjVar(npc, "myChewie"))
		{
			CustomerServiceLog("outbreak_themepark", "outbreak_han_solo conversation: Han has no Chewie objvar. Self: "+npc);
			return;
		}
		
		obj_id myChewie = getObjIdObjVar(npc, "myChewie");
		if (!isValidId(myChewie) || !exists(myChewie))
		{
			CustomerServiceLog("outbreak_themepark", "outbreak_han_solo conversation: Han has a Chewie objvar that is invalid. Self: "+ npc);
			return;
		}
		
		string_id wookSpeak = new string_id(STRINGFILE, "chewie_huwaa_muaa_mumwa");
		chat.chat(myChewie, wookSpeak);
		faceTo(myChewie, player);
		playClientEffectObj(player, "sound/voc_shyriiwook_blurt_rnd.snd", player, "");
		doAnimationAction(myChewie, "celebrate1");
	}
	
	
	public void outbreak_han_solo_action_makeChewieImplore(obj_id player, obj_id npc) throws InterruptedException
	{
		if (!hasObjVar(npc, "myChewie"))
		{
			CustomerServiceLog("outbreak_themepark", "outbreak_han_solo conversation: Han has no Chewie objvar. Self: "+npc);
			return;
		}
		
		obj_id myChewie = getObjIdObjVar(npc, "myChewie");
		if (!isValidId(myChewie) || !exists(myChewie))
		{
			CustomerServiceLog("outbreak_themepark", "outbreak_han_solo conversation: Han has a Chewie objvar that is invalid. Self: "+ npc);
			return;
		}
		
		string_id wookSpeak = new string_id(STRINGFILE, "chewie_wwrcahwowhwa");
		chat.chat(myChewie, wookSpeak);
		faceTo(myChewie, player);
		playClientEffectObj(player, "sound/voc_shyriiwook_blurt_rnd.snd", player, "");
		doAnimationAction(myChewie, "force_strength");
	}
	
	
	public void outbreak_han_solo_action_makeChewieGoodLuck(obj_id player, obj_id npc) throws InterruptedException
	{
		if (!hasObjVar(npc, "myChewie"))
		{
			CustomerServiceLog("outbreak_themepark", "outbreak_han_solo conversation: Han has no Chewie objvar. Self: "+npc);
			return;
		}
		
		obj_id myChewie = getObjIdObjVar(npc, "myChewie");
		if (!isValidId(myChewie) || !exists(myChewie))
		{
			CustomerServiceLog("outbreak_themepark", "outbreak_han_solo conversation: Han has a Chewie objvar that is invalid. Self: "+ npc);
			return;
		}
		
		string_id wookSpeak = new string_id(STRINGFILE, "chewie_ur_oh");
		chat.chat(myChewie, wookSpeak);
		faceTo(myChewie, player);
		playClientEffectObj(player, "sound/voc_shyriiwook_blurt_rnd.snd", player, "");
		doAnimationAction(myChewie, "fc_yawn");
		
	}
	
	
	public void outbreak_han_solo_action_makeChewieCheer2(obj_id player, obj_id npc) throws InterruptedException
	{
		if (!hasObjVar(npc, "myChewie"))
		{
			CustomerServiceLog("outbreak_themepark", "outbreak_han_solo conversation: Han has no Chewie objvar. Self: "+npc);
			return;
		}
		
		obj_id myChewie = getObjIdObjVar(npc, "myChewie");
		if (!isValidId(myChewie) || !exists(myChewie))
		{
			CustomerServiceLog("outbreak_themepark", "outbreak_han_solo conversation: Han has a Chewie objvar that is invalid. Self: "+ npc);
			return;
		}
		
		string_id wookSpeak = new string_id(STRINGFILE, "chewie_huwaa_muaa_mumwa");
		chat.chat(myChewie, wookSpeak);
		faceTo(myChewie, player);
		playClientEffectObj(player, "sound/voc_shyriiwook_blurt_rnd.snd", player, "");
		doAnimationAction(myChewie, "celebrate");
	}
	
	
	public void outbreak_han_solo_action_makeChewieSalute(obj_id player, obj_id npc) throws InterruptedException
	{
		if (!hasObjVar(npc, "myChewie"))
		{
			CustomerServiceLog("outbreak_themepark", "outbreak_han_solo conversation: Han has no Chewie objvar. Self: "+npc);
			return;
		}
		
		obj_id myChewie = getObjIdObjVar(npc, "myChewie");
		if (!isValidId(myChewie) || !exists(myChewie))
		{
			CustomerServiceLog("outbreak_themepark", "outbreak_han_solo conversation: Han has a Chewie objvar that is invalid. Self: "+ npc);
			return;
		}
		
		faceTo(myChewie, player);
		playClientEffectObj(player, "sound/voc_shyriiwook_blurt_rnd.snd", player, "");
		doAnimationAction(myChewie, "salute1");
	}
	
	
	public void outbreak_han_solo_action_makeChewieWave(obj_id player, obj_id npc) throws InterruptedException
	{
		if (!hasObjVar(npc, "myChewie"))
		{
			CustomerServiceLog("outbreak_themepark", "outbreak_han_solo conversation: Han has no Chewie objvar. Self: "+npc);
			return;
		}
		
		obj_id myChewie = getObjIdObjVar(npc, "myChewie");
		if (!isValidId(myChewie) || !exists(myChewie))
		{
			CustomerServiceLog("outbreak_themepark", "outbreak_han_solo conversation: Han has a Chewie objvar that is invalid. Self: "+ npc);
			return;
		}
		
		faceTo(myChewie, player);
		playClientEffectObj(player, "sound/voc_shyriiwook_blurt_rnd.snd", player, "");
		doAnimationAction(myChewie, "wave1");
	}
	
	
	public void outbreak_han_solo_action_makeChewieDirect(obj_id player, obj_id npc) throws InterruptedException
	{
		if (!hasObjVar(npc, "myChewie"))
		{
			CustomerServiceLog("outbreak_themepark", "outbreak_han_solo conversation: Han has no Chewie objvar. Self: "+npc);
			return;
		}
		
		obj_id myChewie = getObjIdObjVar(npc, "myChewie");
		if (!isValidId(myChewie) || !exists(myChewie))
		{
			CustomerServiceLog("outbreak_themepark", "outbreak_han_solo conversation: Han has a Chewie objvar that is invalid. Self: "+ npc);
			return;
		}
		
		faceTo(myChewie, player);
		playClientEffectObj(player, "sound/voc_shyriiwook_blurt_rnd.snd", player, "");
		doAnimationAction(myChewie, "wave_on_directing");
		
	}
	
	
	public void outbreak_han_solo_action_clearLiveConversion(obj_id player, obj_id npc) throws InterruptedException
	{
		if ((!groundquests.isTaskActive(player, "outbreak_live_conversion_rebel", "speakSolo") && !groundquests.isTaskActive(player, "outbreak_switch_to_rebel", "speakSolo")))
		{
			return;
		}
		
		groundquests.sendSignal(player, "liveConversionSpokeSolo");
	}
	
	
	public void outbreak_han_solo_action_revokeRebGrantNeu(obj_id player, obj_id npc) throws InterruptedException
	{
		if (groundquests.isQuestActiveOrComplete(player, "outbreak_live_conversion_rebel"))
		{
			groundquests.clearQuest(player, "outbreak_live_conversion_rebel");
			groundquests.grantQuest(player, "outbreak_switch_to_neutral");
		}
		if (groundquests.isQuestActiveOrComplete(player, "outbreak_switch_to_rebel"))
		{
			groundquests.clearQuest(player, "outbreak_switch_to_rebel");
			groundquests.grantQuest(player, "outbreak_switch_to_neutral");
		}
	}
	
	
	public void outbreak_han_solo_action_revokeRebGrantImp(obj_id player, obj_id npc) throws InterruptedException
	{
		if (groundquests.isQuestActiveOrComplete(player, "outbreak_live_conversion_rebel"))
		{
			groundquests.clearQuest(player, "outbreak_live_conversion_rebel");
			groundquests.grantQuest(player, "outbreak_switch_to_imperial");
		}
		if (groundquests.isQuestActiveOrComplete(player, "outbreak_switch_to_rebel"))
		{
			groundquests.clearQuest(player, "outbreak_switch_to_rebel");
			groundquests.grantQuest(player, "outbreak_switch_to_imperial");
		}
	}
	
	
	public void outbreak_han_solo_action_revokeEntireRebQuestLine(obj_id player, obj_id npc) throws InterruptedException
	{
		if (groundquests.isQuestActiveOrComplete(player, "outbreak_live_conversion_rebel"))
		{
			groundquests.clearQuest(player, "outbreak_live_conversion_rebel");
		}
		
		if (groundquests.isQuestActiveOrComplete(player, "outbreak_switch_to_rebel"))
		{
			groundquests.clearQuest(player, "outbreak_switch_to_rebel");
		}
		
		if (groundquests.isQuestActiveOrComplete(player, "outbreak_quest_01_rebel"))
		{
			groundquests.clearQuest(player, "outbreak_quest_01_rebel");
		}
		
		if (groundquests.isQuestActiveOrComplete(player, "outbreak_quest_01_b_rebel"))
		{
			groundquests.clearQuest(player, "outbreak_quest_01_b_rebel");
		}
		
		if (groundquests.isQuestActiveOrComplete(player, "outbreak_quest_02_rebel"))
		{
			groundquests.clearQuest(player, "outbreak_quest_02_rebel");
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
		
		if (groundquests.isQuestActiveOrComplete(player, "outbreak_quest_administrative_building_rebel_01"))
		{
			groundquests.clearQuest(player, "outbreak_quest_administrative_building_rebel_01");
		}
		
		if (groundquests.isQuestActiveOrComplete(player, "outbreak_quest_administrative_building_rebel_02"))
		{
			groundquests.clearQuest(player, "outbreak_quest_administrative_building_rebel_02");
		}
		
		if (groundquests.isQuestActiveOrComplete(player, "outbreak_quest_administrative_building_rebel_03"))
		{
			groundquests.clearQuest(player, "outbreak_quest_administrative_building_rebel_03");
		}
		
		if (groundquests.isQuestActiveOrComplete(player, "outbreak_quest_administrative_building_rebel_04"))
		{
			groundquests.clearQuest(player, "outbreak_quest_administrative_building_rebel_04");
		}
		
		if (groundquests.isQuestActiveOrComplete(player, "outbreak_quest_facility_01"))
		{
			groundquests.clearQuest(player, "outbreak_quest_facility_01");
		}
		
		if (groundquests.isQuestActiveOrComplete(player, "outbreak_quest_facility_02"))
		{
			groundquests.clearQuest(player, "outbreak_quest_facility_02");
		}
		
		if (groundquests.isQuestActiveOrComplete(player, "outbreak_quest_facility_02_rebel"))
		{
			groundquests.clearQuest(player, "outbreak_quest_facility_02_rebel");
		}
		
		if (groundquests.isQuestActiveOrComplete(player, "outbreak_quest_facility_03"))
		{
			groundquests.clearQuest(player, "outbreak_quest_facility_03");
		}
		
		if (groundquests.isQuestActiveOrComplete(player, "outbreak_quest_facility_04"))
		{
			groundquests.clearQuest(player, "outbreak_quest_facility_04");
		}
		
		if (groundquests.isQuestActiveOrComplete(player, "outbreak_quest_facility_05_rebel"))
		{
			groundquests.clearQuest(player, "outbreak_quest_facility_05_rebel");
		}
		
		if (groundquests.isQuestActiveOrComplete(player, "outbreak_quest_final_rebel"))
		{
			groundquests.clearQuest(player, "outbreak_quest_final_rebel");
		}
		
	}
	
	
	public void outbreak_han_solo_action_grantRebQuest(obj_id player, obj_id npc) throws InterruptedException
	{
		groundquests.grantQuest(player, "outbreak_switch_to_rebel");
	}
	
	
	public void outbreak_han_solo_action_grantImpQuest(obj_id player, obj_id npc) throws InterruptedException
	{
		
		groundquests.grantQuest(player, "outbreak_switch_to_imperial");
	}
	
	
	public void outbreak_han_solo_action_grantNeutQuest(obj_id player, obj_id npc) throws InterruptedException
	{
		groundquests.grantQuest(player, "outbreak_switch_to_neutral");
	}
	
	
	public int outbreak_han_solo_handleBranch1(obj_id player, obj_id npc, string_id response) throws InterruptedException
	{
		
		if (response.equals("s_97"))
		{
			
			if (outbreak_han_solo_condition__defaultCondition (player, npc))
			{
				
				string_id message = new string_id (c_stringFile, "s_98");
				int numberOfResponses = 0;
				
				boolean hasResponse = false;
				
				boolean hasResponse0 = false;
				if (outbreak_han_solo_condition__defaultCondition (player, npc))
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
					
					utils.setScriptVar (player, "conversation.outbreak_han_solo.branchId", 2);
					
					npcSpeak (player, message);
					npcSetConversationResponses (player, responses);
				}
				else
				{
					utils.removeScriptVar (player, "conversation.outbreak_han_solo.branchId");
					
					npcEndConversationWithMessage (player, message);
				}
				
				return SCRIPT_CONTINUE;
			}
			
		}
		
		return SCRIPT_DEFAULT;
	}
	
	
	public int outbreak_han_solo_handleBranch2(obj_id player, obj_id npc, string_id response) throws InterruptedException
	{
		
		if (response.equals("s_99"))
		{
			
			if (outbreak_han_solo_condition__defaultCondition (player, npc))
			{
				
				string_id message = new string_id (c_stringFile, "s_100");
				int numberOfResponses = 0;
				
				boolean hasResponse = false;
				
				boolean hasResponse0 = false;
				if (outbreak_han_solo_condition__defaultCondition (player, npc))
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
						responses[responseIndex++] = new string_id (c_stringFile, "s_101");
					}
					
					utils.setScriptVar (player, "conversation.outbreak_han_solo.branchId", 3);
					
					prose_package pp = new prose_package ();
					pp.stringId = message;
					pp.actor.set (player);
					pp.target.set (npc);
					
					npcSpeak (player, pp);
					npcSetConversationResponses (player, responses);
				}
				else
				{
					utils.removeScriptVar (player, "conversation.outbreak_han_solo.branchId");
					
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
	
	
	public int outbreak_han_solo_handleBranch3(obj_id player, obj_id npc, string_id response) throws InterruptedException
	{
		
		if (response.equals("s_101"))
		{
			
			if (outbreak_han_solo_condition__defaultCondition (player, npc))
			{
				outbreak_han_solo_action_revokeEntireRebQuestLine (player, npc);
				
				string_id message = new string_id (c_stringFile, "s_102");
				int numberOfResponses = 0;
				
				boolean hasResponse = false;
				
				boolean hasResponse0 = false;
				if (outbreak_han_solo_condition__defaultCondition (player, npc))
				{
					++numberOfResponses;
					hasResponse = true;
					hasResponse0 = true;
				}
				
				boolean hasResponse1 = false;
				if (outbreak_han_solo_condition__defaultCondition (player, npc))
				{
					++numberOfResponses;
					hasResponse = true;
					hasResponse1 = true;
				}
				
				boolean hasResponse2 = false;
				if (outbreak_han_solo_condition__defaultCondition (player, npc))
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
						responses[responseIndex++] = new string_id (c_stringFile, "s_103");
					}
					
					if (hasResponse1)
					{
						responses[responseIndex++] = new string_id (c_stringFile, "s_104");
					}
					
					if (hasResponse2)
					{
						responses[responseIndex++] = new string_id (c_stringFile, "s_105");
					}
					
					utils.setScriptVar (player, "conversation.outbreak_han_solo.branchId", 4);
					
					npcSpeak (player, message);
					npcSetConversationResponses (player, responses);
				}
				else
				{
					utils.removeScriptVar (player, "conversation.outbreak_han_solo.branchId");
					
					npcEndConversationWithMessage (player, message);
				}
				
				return SCRIPT_CONTINUE;
			}
			
		}
		
		return SCRIPT_DEFAULT;
	}
	
	
	public int outbreak_han_solo_handleBranch4(obj_id player, obj_id npc, string_id response) throws InterruptedException
	{
		
		if (response.equals("s_103"))
		{
			
			if (outbreak_han_solo_condition__defaultCondition (player, npc))
			{
				outbreak_han_solo_action_grantRebQuest (player, npc);
				
				string_id message = new string_id (c_stringFile, "s_106");
				utils.removeScriptVar (player, "conversation.outbreak_han_solo.branchId");
				
				npcEndConversationWithMessage (player, message);
				
				return SCRIPT_CONTINUE;
			}
			
		}
		
		if (response.equals("s_104"))
		{
			
			if (outbreak_han_solo_condition__defaultCondition (player, npc))
			{
				outbreak_han_solo_action_grantImpQuest (player, npc);
				
				string_id message = new string_id (c_stringFile, "s_107");
				utils.removeScriptVar (player, "conversation.outbreak_han_solo.branchId");
				
				npcEndConversationWithMessage (player, message);
				
				return SCRIPT_CONTINUE;
			}
			
		}
		
		if (response.equals("s_105"))
		{
			
			if (outbreak_han_solo_condition__defaultCondition (player, npc))
			{
				outbreak_han_solo_action_grantNeutQuest (player, npc);
				
				string_id message = new string_id (c_stringFile, "s_108");
				utils.removeScriptVar (player, "conversation.outbreak_han_solo.branchId");
				
				npcEndConversationWithMessage (player, message);
				
				return SCRIPT_CONTINUE;
			}
			
		}
		
		return SCRIPT_DEFAULT;
	}
	
	
	public int outbreak_han_solo_handleBranch8(obj_id player, obj_id npc, string_id response) throws InterruptedException
	{
		
		if (response.equals("s_124"))
		{
			
			if (outbreak_han_solo_condition__defaultCondition (player, npc))
			{
				doAnimationAction (npc, "handshake_tandem");
				
				doAnimationAction (player, "handshake_tandem");
				
				string_id message = new string_id (c_stringFile, "s_125");
				int numberOfResponses = 0;
				
				boolean hasResponse = false;
				
				boolean hasResponse0 = false;
				if (outbreak_han_solo_condition__defaultCondition (player, npc))
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
					
					utils.setScriptVar (player, "conversation.outbreak_han_solo.branchId", 9);
					
					npcSpeak (player, message);
					npcSetConversationResponses (player, responses);
				}
				else
				{
					utils.removeScriptVar (player, "conversation.outbreak_han_solo.branchId");
					
					npcEndConversationWithMessage (player, message);
				}
				
				return SCRIPT_CONTINUE;
			}
			
		}
		
		return SCRIPT_DEFAULT;
	}
	
	
	public int outbreak_han_solo_handleBranch9(obj_id player, obj_id npc, string_id response) throws InterruptedException
	{
		
		if (response.equals("s_126"))
		{
			
			if (outbreak_han_solo_condition__defaultCondition (player, npc))
			{
				
				string_id message = new string_id (c_stringFile, "s_127");
				int numberOfResponses = 0;
				
				boolean hasResponse = false;
				
				boolean hasResponse0 = false;
				if (outbreak_han_solo_condition__defaultCondition (player, npc))
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
					
					utils.setScriptVar (player, "conversation.outbreak_han_solo.branchId", 10);
					
					npcSpeak (player, message);
					npcSetConversationResponses (player, responses);
				}
				else
				{
					utils.removeScriptVar (player, "conversation.outbreak_han_solo.branchId");
					
					npcEndConversationWithMessage (player, message);
				}
				
				return SCRIPT_CONTINUE;
			}
			
		}
		
		return SCRIPT_DEFAULT;
	}
	
	
	public int outbreak_han_solo_handleBranch10(obj_id player, obj_id npc, string_id response) throws InterruptedException
	{
		
		if (response.equals("s_128"))
		{
			
			if (outbreak_han_solo_condition__defaultCondition (player, npc))
			{
				doAnimationAction (npc, "nod");
				
				string_id message = new string_id (c_stringFile, "s_129");
				int numberOfResponses = 0;
				
				boolean hasResponse = false;
				
				boolean hasResponse0 = false;
				if (outbreak_han_solo_condition__defaultCondition (player, npc))
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
					
					utils.setScriptVar (player, "conversation.outbreak_han_solo.branchId", 11);
					
					prose_package pp = new prose_package ();
					pp.stringId = message;
					pp.actor.set (player);
					pp.target.set (npc);
					
					npcSpeak (player, pp);
					npcSetConversationResponses (player, responses);
				}
				else
				{
					utils.removeScriptVar (player, "conversation.outbreak_han_solo.branchId");
					
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
	
	
	public int outbreak_han_solo_handleBranch11(obj_id player, obj_id npc, string_id response) throws InterruptedException
	{
		
		if (response.equals("s_71"))
		{
			
			if (outbreak_han_solo_condition__defaultCondition (player, npc))
			{
				doAnimationAction (npc, "wave1");
				
				outbreak_han_solo_action_completeQuest5 (player, npc);
				
				string_id message = new string_id (c_stringFile, "s_72");
				utils.removeScriptVar (player, "conversation.outbreak_han_solo.branchId");
				
				npcEndConversationWithMessage (player, message);
				
				return SCRIPT_CONTINUE;
			}
			
		}
		
		return SCRIPT_DEFAULT;
	}
	
	
	public int outbreak_han_solo_handleBranch16(obj_id player, obj_id npc, string_id response) throws InterruptedException
	{
		
		if (response.equals("s_64"))
		{
			
			if (outbreak_han_solo_condition__defaultCondition (player, npc))
			{
				
				string_id message = new string_id (c_stringFile, "s_65");
				int numberOfResponses = 0;
				
				boolean hasResponse = false;
				
				boolean hasResponse0 = false;
				if (outbreak_han_solo_condition__defaultCondition (player, npc))
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
					
					utils.setScriptVar (player, "conversation.outbreak_han_solo.branchId", 17);
					
					npcSpeak (player, message);
					npcSetConversationResponses (player, responses);
				}
				else
				{
					utils.removeScriptVar (player, "conversation.outbreak_han_solo.branchId");
					
					npcEndConversationWithMessage (player, message);
				}
				
				return SCRIPT_CONTINUE;
			}
			
		}
		
		if (response.equals("s_90"))
		{
			
			if (outbreak_han_solo_condition__defaultCondition (player, npc))
			{
				
				string_id message = new string_id (c_stringFile, "s_91");
				int numberOfResponses = 0;
				
				boolean hasResponse = false;
				
				boolean hasResponse0 = false;
				if (outbreak_han_solo_condition__defaultCondition (player, npc))
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
					
					utils.setScriptVar (player, "conversation.outbreak_han_solo.branchId", 19);
					
					npcSpeak (player, message);
					npcSetConversationResponses (player, responses);
				}
				else
				{
					utils.removeScriptVar (player, "conversation.outbreak_han_solo.branchId");
					
					npcEndConversationWithMessage (player, message);
				}
				
				return SCRIPT_CONTINUE;
			}
			
		}
		
		return SCRIPT_DEFAULT;
	}
	
	
	public int outbreak_han_solo_handleBranch17(obj_id player, obj_id npc, string_id response) throws InterruptedException
	{
		
		if (response.equals("s_67"))
		{
			
			if (outbreak_han_solo_condition__defaultCondition (player, npc))
			{
				outbreak_han_solo_action_revokeRebGrantNeu (player, npc);
				
				string_id message = new string_id (c_stringFile, "s_69");
				utils.removeScriptVar (player, "conversation.outbreak_han_solo.branchId");
				
				npcEndConversationWithMessage (player, message);
				
				return SCRIPT_CONTINUE;
			}
			
		}
		
		return SCRIPT_DEFAULT;
	}
	
	
	public int outbreak_han_solo_handleBranch19(obj_id player, obj_id npc, string_id response) throws InterruptedException
	{
		
		if (response.equals("s_94"))
		{
			
			if (outbreak_han_solo_condition__defaultCondition (player, npc))
			{
				
				string_id message = new string_id (c_stringFile, "s_95");
				int numberOfResponses = 0;
				
				boolean hasResponse = false;
				
				boolean hasResponse0 = false;
				if (outbreak_han_solo_condition__defaultCondition (player, npc))
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
					
					utils.setScriptVar (player, "conversation.outbreak_han_solo.branchId", 26);
					
					prose_package pp = new prose_package ();
					pp.stringId = message;
					pp.actor.set (player);
					pp.target.set (npc);
					
					npcSpeak (player, pp);
					npcSetConversationResponses (player, responses);
				}
				else
				{
					utils.removeScriptVar (player, "conversation.outbreak_han_solo.branchId");
					
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
	
	
	public int outbreak_han_solo_handleBranch20(obj_id player, obj_id npc, string_id response) throws InterruptedException
	{
		
		if (response.equals("s_49"))
		{
			
			if (outbreak_han_solo_condition__defaultCondition (player, npc))
			{
				
				string_id message = new string_id (c_stringFile, "s_51");
				int numberOfResponses = 0;
				
				boolean hasResponse = false;
				
				boolean hasResponse0 = false;
				if (outbreak_han_solo_condition__defaultCondition (player, npc))
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
					
					utils.setScriptVar (player, "conversation.outbreak_han_solo.branchId", 27);
					
					npcSpeak (player, message);
					npcSetConversationResponses (player, responses);
				}
				else
				{
					utils.removeScriptVar (player, "conversation.outbreak_han_solo.branchId");
					
					npcEndConversationWithMessage (player, message);
				}
				
				return SCRIPT_CONTINUE;
			}
			
		}
		
		return SCRIPT_DEFAULT;
	}
	
	
	public int outbreak_han_solo_handleBranch21(obj_id player, obj_id npc, string_id response) throws InterruptedException
	{
		
		if (response.equals("s_63"))
		{
			
			if (outbreak_han_solo_condition__defaultCondition (player, npc))
			{
				
				string_id message = new string_id (c_stringFile, "s_66");
				int numberOfResponses = 0;
				
				boolean hasResponse = false;
				
				boolean hasResponse0 = false;
				if (outbreak_han_solo_condition__defaultCondition (player, npc))
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
						responses[responseIndex++] = new string_id (c_stringFile, "s_68");
					}
					
					utils.setScriptVar (player, "conversation.outbreak_han_solo.branchId", 22);
					
					npcSpeak (player, message);
					npcSetConversationResponses (player, responses);
				}
				else
				{
					utils.removeScriptVar (player, "conversation.outbreak_han_solo.branchId");
					
					npcEndConversationWithMessage (player, message);
				}
				
				return SCRIPT_CONTINUE;
			}
			
		}
		
		if (response.equals("s_89"))
		{
			
			if (outbreak_han_solo_condition__defaultCondition (player, npc))
			{
				
				string_id message = new string_id (c_stringFile, "s_92");
				int numberOfResponses = 0;
				
				boolean hasResponse = false;
				
				boolean hasResponse0 = false;
				if (outbreak_han_solo_condition__defaultCondition (player, npc))
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
					
					utils.setScriptVar (player, "conversation.outbreak_han_solo.branchId", 24);
					
					npcSpeak (player, message);
					npcSetConversationResponses (player, responses);
				}
				else
				{
					utils.removeScriptVar (player, "conversation.outbreak_han_solo.branchId");
					
					npcEndConversationWithMessage (player, message);
				}
				
				return SCRIPT_CONTINUE;
			}
			
		}
		
		return SCRIPT_DEFAULT;
	}
	
	
	public int outbreak_han_solo_handleBranch22(obj_id player, obj_id npc, string_id response) throws InterruptedException
	{
		
		if (response.equals("s_68"))
		{
			
			if (outbreak_han_solo_condition__defaultCondition (player, npc))
			{
				outbreak_han_solo_action_revokeRebGrantImp (player, npc);
				
				string_id message = new string_id (c_stringFile, "s_70");
				utils.removeScriptVar (player, "conversation.outbreak_han_solo.branchId");
				
				npcEndConversationWithMessage (player, message);
				
				return SCRIPT_CONTINUE;
			}
			
		}
		
		return SCRIPT_DEFAULT;
	}
	
	
	public int outbreak_han_solo_handleBranch24(obj_id player, obj_id npc, string_id response) throws InterruptedException
	{
		
		if (response.equals("s_93"))
		{
			
			if (outbreak_han_solo_condition__defaultCondition (player, npc))
			{
				
				string_id message = new string_id (c_stringFile, "s_96");
				int numberOfResponses = 0;
				
				boolean hasResponse = false;
				
				boolean hasResponse0 = false;
				if (outbreak_han_solo_condition__defaultCondition (player, npc))
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
					
					utils.setScriptVar (player, "conversation.outbreak_han_solo.branchId", 26);
					
					prose_package pp = new prose_package ();
					pp.stringId = message;
					pp.actor.set (player);
					pp.target.set (npc);
					
					npcSpeak (player, pp);
					npcSetConversationResponses (player, responses);
				}
				else
				{
					utils.removeScriptVar (player, "conversation.outbreak_han_solo.branchId");
					
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
	
	
	public int outbreak_han_solo_handleBranch25(obj_id player, obj_id npc, string_id response) throws InterruptedException
	{
		
		if (response.equals("s_49"))
		{
			
			if (outbreak_han_solo_condition__defaultCondition (player, npc))
			{
				
				string_id message = new string_id (c_stringFile, "s_51");
				int numberOfResponses = 0;
				
				boolean hasResponse = false;
				
				boolean hasResponse0 = false;
				if (outbreak_han_solo_condition__defaultCondition (player, npc))
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
					
					utils.setScriptVar (player, "conversation.outbreak_han_solo.branchId", 27);
					
					npcSpeak (player, message);
					npcSetConversationResponses (player, responses);
				}
				else
				{
					utils.removeScriptVar (player, "conversation.outbreak_han_solo.branchId");
					
					npcEndConversationWithMessage (player, message);
				}
				
				return SCRIPT_CONTINUE;
			}
			
		}
		
		return SCRIPT_DEFAULT;
	}
	
	
	public int outbreak_han_solo_handleBranch26(obj_id player, obj_id npc, string_id response) throws InterruptedException
	{
		
		if (response.equals("s_49"))
		{
			
			if (outbreak_han_solo_condition__defaultCondition (player, npc))
			{
				
				string_id message = new string_id (c_stringFile, "s_51");
				int numberOfResponses = 0;
				
				boolean hasResponse = false;
				
				boolean hasResponse0 = false;
				if (outbreak_han_solo_condition__defaultCondition (player, npc))
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
					
					utils.setScriptVar (player, "conversation.outbreak_han_solo.branchId", 27);
					
					npcSpeak (player, message);
					npcSetConversationResponses (player, responses);
				}
				else
				{
					utils.removeScriptVar (player, "conversation.outbreak_han_solo.branchId");
					
					npcEndConversationWithMessage (player, message);
				}
				
				return SCRIPT_CONTINUE;
			}
			
		}
		
		return SCRIPT_DEFAULT;
	}
	
	
	public int outbreak_han_solo_handleBranch27(obj_id player, obj_id npc, string_id response) throws InterruptedException
	{
		
		if (response.equals("s_140"))
		{
			
			if (outbreak_han_solo_condition__defaultCondition (player, npc))
			{
				outbreak_han_solo_action_makeChewieSayRight (player, npc);
				
				string_id message = new string_id (c_stringFile, "s_142");
				int numberOfResponses = 0;
				
				boolean hasResponse = false;
				
				boolean hasResponse0 = false;
				if (outbreak_han_solo_condition__defaultCondition (player, npc))
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
					
					utils.setScriptVar (player, "conversation.outbreak_han_solo.branchId", 28);
					
					npcSpeak (player, message);
					npcSetConversationResponses (player, responses);
				}
				else
				{
					utils.removeScriptVar (player, "conversation.outbreak_han_solo.branchId");
					
					npcEndConversationWithMessage (player, message);
				}
				
				return SCRIPT_CONTINUE;
			}
			
		}
		
		return SCRIPT_DEFAULT;
	}
	
	
	public int outbreak_han_solo_handleBranch28(obj_id player, obj_id npc, string_id response) throws InterruptedException
	{
		
		if (response.equals("s_144"))
		{
			
			if (outbreak_han_solo_condition__defaultCondition (player, npc))
			{
				
				string_id message = new string_id (c_stringFile, "s_146");
				int numberOfResponses = 0;
				
				boolean hasResponse = false;
				
				boolean hasResponse0 = false;
				if (outbreak_han_solo_condition__defaultCondition (player, npc))
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
					
					utils.setScriptVar (player, "conversation.outbreak_han_solo.branchId", 29);
					
					npcSpeak (player, message);
					npcSetConversationResponses (player, responses);
				}
				else
				{
					utils.removeScriptVar (player, "conversation.outbreak_han_solo.branchId");
					
					npcEndConversationWithMessage (player, message);
				}
				
				return SCRIPT_CONTINUE;
			}
			
		}
		
		return SCRIPT_DEFAULT;
	}
	
	
	public int outbreak_han_solo_handleBranch29(obj_id player, obj_id npc, string_id response) throws InterruptedException
	{
		
		if (response.equals("s_148"))
		{
			
			if (outbreak_han_solo_condition__defaultCondition (player, npc))
			{
				doAnimationAction (npc, "nod_head_multiple");
				
				string_id message = new string_id (c_stringFile, "s_150");
				int numberOfResponses = 0;
				
				boolean hasResponse = false;
				
				boolean hasResponse0 = false;
				if (outbreak_han_solo_condition__defaultCondition (player, npc))
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
					
					utils.setScriptVar (player, "conversation.outbreak_han_solo.branchId", 30);
					
					npcSpeak (player, message);
					npcSetConversationResponses (player, responses);
				}
				else
				{
					utils.removeScriptVar (player, "conversation.outbreak_han_solo.branchId");
					
					npcEndConversationWithMessage (player, message);
				}
				
				return SCRIPT_CONTINUE;
			}
			
		}
		
		return SCRIPT_DEFAULT;
	}
	
	
	public int outbreak_han_solo_handleBranch30(obj_id player, obj_id npc, string_id response) throws InterruptedException
	{
		
		if (response.equals("s_152"))
		{
			
			if (outbreak_han_solo_condition__defaultCondition (player, npc))
			{
				
				string_id message = new string_id (c_stringFile, "s_154");
				int numberOfResponses = 0;
				
				boolean hasResponse = false;
				
				boolean hasResponse0 = false;
				if (outbreak_han_solo_condition__defaultCondition (player, npc))
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
					
					utils.setScriptVar (player, "conversation.outbreak_han_solo.branchId", 31);
					
					npcSpeak (player, message);
					npcSetConversationResponses (player, responses);
				}
				else
				{
					utils.removeScriptVar (player, "conversation.outbreak_han_solo.branchId");
					
					npcEndConversationWithMessage (player, message);
				}
				
				return SCRIPT_CONTINUE;
			}
			
		}
		
		return SCRIPT_DEFAULT;
	}
	
	
	public int outbreak_han_solo_handleBranch31(obj_id player, obj_id npc, string_id response) throws InterruptedException
	{
		
		if (response.equals("s_156"))
		{
			
			if (outbreak_han_solo_condition__defaultCondition (player, npc))
			{
				
				string_id message = new string_id (c_stringFile, "s_242");
				int numberOfResponses = 0;
				
				boolean hasResponse = false;
				
				boolean hasResponse0 = false;
				if (outbreak_han_solo_condition__defaultCondition (player, npc))
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
					
					utils.setScriptVar (player, "conversation.outbreak_han_solo.branchId", 32);
					
					npcSpeak (player, message);
					npcSetConversationResponses (player, responses);
				}
				else
				{
					utils.removeScriptVar (player, "conversation.outbreak_han_solo.branchId");
					
					npcEndConversationWithMessage (player, message);
				}
				
				return SCRIPT_CONTINUE;
			}
			
		}
		
		return SCRIPT_DEFAULT;
	}
	
	
	public int outbreak_han_solo_handleBranch32(obj_id player, obj_id npc, string_id response) throws InterruptedException
	{
		
		if (response.equals("s_243"))
		{
			
			if (outbreak_han_solo_condition__defaultCondition (player, npc))
			{
				
				string_id message = new string_id (c_stringFile, "s_244");
				int numberOfResponses = 0;
				
				boolean hasResponse = false;
				
				boolean hasResponse0 = false;
				if (outbreak_han_solo_condition__defaultCondition (player, npc))
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
					
					utils.setScriptVar (player, "conversation.outbreak_han_solo.branchId", 33);
					
					npcSpeak (player, message);
					npcSetConversationResponses (player, responses);
				}
				else
				{
					utils.removeScriptVar (player, "conversation.outbreak_han_solo.branchId");
					
					npcEndConversationWithMessage (player, message);
				}
				
				return SCRIPT_CONTINUE;
			}
			
		}
		
		return SCRIPT_DEFAULT;
	}
	
	
	public int outbreak_han_solo_handleBranch33(obj_id player, obj_id npc, string_id response) throws InterruptedException
	{
		
		if (response.equals("s_245"))
		{
			
			if (outbreak_han_solo_condition__defaultCondition (player, npc))
			{
				outbreak_han_solo_action_makeChewieImplore (player, npc);
				
				string_id message = new string_id (c_stringFile, "s_247");
				int numberOfResponses = 0;
				
				boolean hasResponse = false;
				
				boolean hasResponse0 = false;
				if (outbreak_han_solo_condition__defaultCondition (player, npc))
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
					
					utils.setScriptVar (player, "conversation.outbreak_han_solo.branchId", 34);
					
					npcSpeak (player, message);
					npcSetConversationResponses (player, responses);
				}
				else
				{
					utils.removeScriptVar (player, "conversation.outbreak_han_solo.branchId");
					
					npcEndConversationWithMessage (player, message);
				}
				
				return SCRIPT_CONTINUE;
			}
			
		}
		
		return SCRIPT_DEFAULT;
	}
	
	
	public int outbreak_han_solo_handleBranch34(obj_id player, obj_id npc, string_id response) throws InterruptedException
	{
		
		if (response.equals("s_249"))
		{
			
			if (outbreak_han_solo_condition__defaultCondition (player, npc))
			{
				
				string_id message = new string_id (c_stringFile, "s_260");
				int numberOfResponses = 0;
				
				boolean hasResponse = false;
				
				boolean hasResponse0 = false;
				if (outbreak_han_solo_condition__defaultCondition (player, npc))
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
					
					utils.setScriptVar (player, "conversation.outbreak_han_solo.branchId", 35);
					
					npcSpeak (player, message);
					npcSetConversationResponses (player, responses);
				}
				else
				{
					utils.removeScriptVar (player, "conversation.outbreak_han_solo.branchId");
					
					npcEndConversationWithMessage (player, message);
				}
				
				return SCRIPT_CONTINUE;
			}
			
		}
		
		return SCRIPT_DEFAULT;
	}
	
	
	public int outbreak_han_solo_handleBranch35(obj_id player, obj_id npc, string_id response) throws InterruptedException
	{
		
		if (response.equals("s_261"))
		{
			
			if (outbreak_han_solo_condition__defaultCondition (player, npc))
			{
				
				string_id message = new string_id (c_stringFile, "s_83");
				int numberOfResponses = 0;
				
				boolean hasResponse = false;
				
				boolean hasResponse0 = false;
				if (outbreak_han_solo_condition__defaultCondition (player, npc))
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
					
					utils.setScriptVar (player, "conversation.outbreak_han_solo.branchId", 36);
					
					npcSpeak (player, message);
					npcSetConversationResponses (player, responses);
				}
				else
				{
					utils.removeScriptVar (player, "conversation.outbreak_han_solo.branchId");
					
					npcEndConversationWithMessage (player, message);
				}
				
				return SCRIPT_CONTINUE;
			}
			
		}
		
		return SCRIPT_DEFAULT;
	}
	
	
	public int outbreak_han_solo_handleBranch36(obj_id player, obj_id npc, string_id response) throws InterruptedException
	{
		
		if (response.equals("s_85"))
		{
			
			if (outbreak_han_solo_condition__defaultCondition (player, npc))
			{
				
				string_id message = new string_id (c_stringFile, "s_87");
				int numberOfResponses = 0;
				
				boolean hasResponse = false;
				
				boolean hasResponse0 = false;
				if (outbreak_han_solo_condition__defaultCondition (player, npc))
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
					
					utils.setScriptVar (player, "conversation.outbreak_han_solo.branchId", 37);
					
					npcSpeak (player, message);
					npcSetConversationResponses (player, responses);
				}
				else
				{
					utils.removeScriptVar (player, "conversation.outbreak_han_solo.branchId");
					
					npcEndConversationWithMessage (player, message);
				}
				
				return SCRIPT_CONTINUE;
			}
			
		}
		
		return SCRIPT_DEFAULT;
	}
	
	
	public int outbreak_han_solo_handleBranch37(obj_id player, obj_id npc, string_id response) throws InterruptedException
	{
		
		if (response.equals("s_109"))
		{
			
			if (outbreak_han_solo_condition__defaultCondition (player, npc))
			{
				
				string_id message = new string_id (c_stringFile, "s_111");
				int numberOfResponses = 0;
				
				boolean hasResponse = false;
				
				boolean hasResponse0 = false;
				if (outbreak_han_solo_condition__defaultCondition (player, npc))
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
						responses[responseIndex++] = new string_id (c_stringFile, "s_113");
					}
					
					utils.setScriptVar (player, "conversation.outbreak_han_solo.branchId", 38);
					
					npcSpeak (player, message);
					npcSetConversationResponses (player, responses);
				}
				else
				{
					utils.removeScriptVar (player, "conversation.outbreak_han_solo.branchId");
					
					npcEndConversationWithMessage (player, message);
				}
				
				return SCRIPT_CONTINUE;
			}
			
		}
		
		return SCRIPT_DEFAULT;
	}
	
	
	public int outbreak_han_solo_handleBranch38(obj_id player, obj_id npc, string_id response) throws InterruptedException
	{
		
		if (response.equals("s_113"))
		{
			
			if (outbreak_han_solo_condition__defaultCondition (player, npc))
			{
				
				string_id message = new string_id (c_stringFile, "s_115");
				int numberOfResponses = 0;
				
				boolean hasResponse = false;
				
				boolean hasResponse0 = false;
				if (outbreak_han_solo_condition__defaultCondition (player, npc))
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
						responses[responseIndex++] = new string_id (c_stringFile, "s_117");
					}
					
					utils.setScriptVar (player, "conversation.outbreak_han_solo.branchId", 39);
					
					npcSpeak (player, message);
					npcSetConversationResponses (player, responses);
				}
				else
				{
					utils.removeScriptVar (player, "conversation.outbreak_han_solo.branchId");
					
					npcEndConversationWithMessage (player, message);
				}
				
				return SCRIPT_CONTINUE;
			}
			
		}
		
		return SCRIPT_DEFAULT;
	}
	
	
	public int outbreak_han_solo_handleBranch39(obj_id player, obj_id npc, string_id response) throws InterruptedException
	{
		
		if (response.equals("s_117"))
		{
			
			if (outbreak_han_solo_condition__defaultCondition (player, npc))
			{
				
				string_id message = new string_id (c_stringFile, "s_119");
				int numberOfResponses = 0;
				
				boolean hasResponse = false;
				
				boolean hasResponse0 = false;
				if (outbreak_han_solo_condition__defaultCondition (player, npc))
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
						responses[responseIndex++] = new string_id (c_stringFile, "s_131");
					}
					
					utils.setScriptVar (player, "conversation.outbreak_han_solo.branchId", 40);
					
					npcSpeak (player, message);
					npcSetConversationResponses (player, responses);
				}
				else
				{
					utils.removeScriptVar (player, "conversation.outbreak_han_solo.branchId");
					
					npcEndConversationWithMessage (player, message);
				}
				
				return SCRIPT_CONTINUE;
			}
			
		}
		
		return SCRIPT_DEFAULT;
	}
	
	
	public int outbreak_han_solo_handleBranch40(obj_id player, obj_id npc, string_id response) throws InterruptedException
	{
		
		if (response.equals("s_131"))
		{
			
			if (outbreak_han_solo_condition__defaultCondition (player, npc))
			{
				
				string_id message = new string_id (c_stringFile, "s_133");
				int numberOfResponses = 0;
				
				boolean hasResponse = false;
				
				boolean hasResponse0 = false;
				if (outbreak_han_solo_condition__defaultCondition (player, npc))
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
						responses[responseIndex++] = new string_id (c_stringFile, "s_135");
					}
					
					utils.setScriptVar (player, "conversation.outbreak_han_solo.branchId", 41);
					
					npcSpeak (player, message);
					npcSetConversationResponses (player, responses);
				}
				else
				{
					utils.removeScriptVar (player, "conversation.outbreak_han_solo.branchId");
					
					npcEndConversationWithMessage (player, message);
				}
				
				return SCRIPT_CONTINUE;
			}
			
		}
		
		return SCRIPT_DEFAULT;
	}
	
	
	public int outbreak_han_solo_handleBranch41(obj_id player, obj_id npc, string_id response) throws InterruptedException
	{
		
		if (response.equals("s_135"))
		{
			
			if (outbreak_han_solo_condition__defaultCondition (player, npc))
			{
				
				string_id message = new string_id (c_stringFile, "s_137");
				int numberOfResponses = 0;
				
				boolean hasResponse = false;
				
				boolean hasResponse0 = false;
				if (outbreak_han_solo_condition__defaultCondition (player, npc))
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
						responses[responseIndex++] = new string_id (c_stringFile, "s_139");
					}
					
					utils.setScriptVar (player, "conversation.outbreak_han_solo.branchId", 42);
					
					npcSpeak (player, message);
					npcSetConversationResponses (player, responses);
				}
				else
				{
					utils.removeScriptVar (player, "conversation.outbreak_han_solo.branchId");
					
					npcEndConversationWithMessage (player, message);
				}
				
				return SCRIPT_CONTINUE;
			}
			
		}
		
		return SCRIPT_DEFAULT;
	}
	
	
	public int outbreak_han_solo_handleBranch42(obj_id player, obj_id npc, string_id response) throws InterruptedException
	{
		
		if (response.equals("s_139"))
		{
			
			if (outbreak_han_solo_condition__defaultCondition (player, npc))
			{
				outbreak_han_solo_action_makeChewieCheer (player, npc);
				
				string_id message = new string_id (c_stringFile, "s_143");
				int numberOfResponses = 0;
				
				boolean hasResponse = false;
				
				boolean hasResponse0 = false;
				if (outbreak_han_solo_condition__defaultCondition (player, npc))
				{
					++numberOfResponses;
					hasResponse = true;
					hasResponse0 = true;
				}
				
				boolean hasResponse1 = false;
				if (outbreak_han_solo_condition__defaultCondition (player, npc))
				{
					++numberOfResponses;
					hasResponse = true;
					hasResponse1 = true;
				}
				
				boolean hasResponse2 = false;
				if (outbreak_han_solo_condition__defaultCondition (player, npc))
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
						responses[responseIndex++] = new string_id (c_stringFile, "s_147");
					}
					
					if (hasResponse1)
					{
						responses[responseIndex++] = new string_id (c_stringFile, "s_155");
					}
					
					if (hasResponse2)
					{
						responses[responseIndex++] = new string_id (c_stringFile, "s_160");
					}
					
					utils.setScriptVar (player, "conversation.outbreak_han_solo.branchId", 43);
					
					npcSpeak (player, message);
					npcSetConversationResponses (player, responses);
				}
				else
				{
					utils.removeScriptVar (player, "conversation.outbreak_han_solo.branchId");
					
					npcEndConversationWithMessage (player, message);
				}
				
				return SCRIPT_CONTINUE;
			}
			
		}
		
		return SCRIPT_DEFAULT;
	}
	
	
	public int outbreak_han_solo_handleBranch43(obj_id player, obj_id npc, string_id response) throws InterruptedException
	{
		
		if (response.equals("s_147"))
		{
			
			if (outbreak_han_solo_condition__defaultCondition (player, npc))
			{
				
				string_id message = new string_id (c_stringFile, "s_151");
				int numberOfResponses = 0;
				
				boolean hasResponse = false;
				
				boolean hasResponse0 = false;
				if (outbreak_han_solo_condition__defaultCondition (player, npc))
				{
					++numberOfResponses;
					hasResponse = true;
					hasResponse0 = true;
				}
				
				boolean hasResponse1 = false;
				if (outbreak_han_solo_condition__defaultCondition (player, npc))
				{
					++numberOfResponses;
					hasResponse = true;
					hasResponse1 = true;
				}
				
				boolean hasResponse2 = false;
				if (outbreak_han_solo_condition__defaultCondition (player, npc))
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
						responses[responseIndex++] = new string_id (c_stringFile, "s_147");
					}
					
					if (hasResponse1)
					{
						responses[responseIndex++] = new string_id (c_stringFile, "s_155");
					}
					
					if (hasResponse2)
					{
						responses[responseIndex++] = new string_id (c_stringFile, "s_160");
					}
					
					utils.setScriptVar (player, "conversation.outbreak_han_solo.branchId", 43);
					
					npcSpeak (player, message);
					npcSetConversationResponses (player, responses);
				}
				else
				{
					utils.removeScriptVar (player, "conversation.outbreak_han_solo.branchId");
					
					npcEndConversationWithMessage (player, message);
				}
				
				return SCRIPT_CONTINUE;
			}
			
		}
		
		if (response.equals("s_155"))
		{
			
			if (outbreak_han_solo_condition__defaultCondition (player, npc))
			{
				
				string_id message = new string_id (c_stringFile, "s_158");
				int numberOfResponses = 0;
				
				boolean hasResponse = false;
				
				boolean hasResponse0 = false;
				if (outbreak_han_solo_condition__defaultCondition (player, npc))
				{
					++numberOfResponses;
					hasResponse = true;
					hasResponse0 = true;
				}
				
				boolean hasResponse1 = false;
				if (outbreak_han_solo_condition__defaultCondition (player, npc))
				{
					++numberOfResponses;
					hasResponse = true;
					hasResponse1 = true;
				}
				
				boolean hasResponse2 = false;
				if (outbreak_han_solo_condition__defaultCondition (player, npc))
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
						responses[responseIndex++] = new string_id (c_stringFile, "s_147");
					}
					
					if (hasResponse1)
					{
						responses[responseIndex++] = new string_id (c_stringFile, "s_155");
					}
					
					if (hasResponse2)
					{
						responses[responseIndex++] = new string_id (c_stringFile, "s_160");
					}
					
					utils.setScriptVar (player, "conversation.outbreak_han_solo.branchId", 43);
					
					npcSpeak (player, message);
					npcSetConversationResponses (player, responses);
				}
				else
				{
					utils.removeScriptVar (player, "conversation.outbreak_han_solo.branchId");
					
					npcEndConversationWithMessage (player, message);
				}
				
				return SCRIPT_CONTINUE;
			}
			
		}
		
		if (response.equals("s_160"))
		{
			
			if (outbreak_han_solo_condition__defaultCondition (player, npc))
			{
				outbreak_han_solo_action_grantMissionOne (player, npc);
				
				string_id message = new string_id (c_stringFile, "s_162");
				utils.removeScriptVar (player, "conversation.outbreak_han_solo.branchId");
				
				npcEndConversationWithMessage (player, message);
				
				return SCRIPT_CONTINUE;
			}
			
		}
		
		return SCRIPT_DEFAULT;
	}
	
	
	public int outbreak_han_solo_handleBranch44(obj_id player, obj_id npc, string_id response) throws InterruptedException
	{
		
		if (response.equals("s_147"))
		{
			
			if (outbreak_han_solo_condition__defaultCondition (player, npc))
			{
				
				string_id message = new string_id (c_stringFile, "s_151");
				int numberOfResponses = 0;
				
				boolean hasResponse = false;
				
				boolean hasResponse0 = false;
				if (outbreak_han_solo_condition__defaultCondition (player, npc))
				{
					++numberOfResponses;
					hasResponse = true;
					hasResponse0 = true;
				}
				
				boolean hasResponse1 = false;
				if (outbreak_han_solo_condition__defaultCondition (player, npc))
				{
					++numberOfResponses;
					hasResponse = true;
					hasResponse1 = true;
				}
				
				boolean hasResponse2 = false;
				if (outbreak_han_solo_condition__defaultCondition (player, npc))
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
						responses[responseIndex++] = new string_id (c_stringFile, "s_147");
					}
					
					if (hasResponse1)
					{
						responses[responseIndex++] = new string_id (c_stringFile, "s_155");
					}
					
					if (hasResponse2)
					{
						responses[responseIndex++] = new string_id (c_stringFile, "s_160");
					}
					
					utils.setScriptVar (player, "conversation.outbreak_han_solo.branchId", 43);
					
					npcSpeak (player, message);
					npcSetConversationResponses (player, responses);
				}
				else
				{
					utils.removeScriptVar (player, "conversation.outbreak_han_solo.branchId");
					
					npcEndConversationWithMessage (player, message);
				}
				
				return SCRIPT_CONTINUE;
			}
			
		}
		
		if (response.equals("s_155"))
		{
			
			if (outbreak_han_solo_condition__defaultCondition (player, npc))
			{
				
				string_id message = new string_id (c_stringFile, "s_158");
				int numberOfResponses = 0;
				
				boolean hasResponse = false;
				
				boolean hasResponse0 = false;
				if (outbreak_han_solo_condition__defaultCondition (player, npc))
				{
					++numberOfResponses;
					hasResponse = true;
					hasResponse0 = true;
				}
				
				boolean hasResponse1 = false;
				if (outbreak_han_solo_condition__defaultCondition (player, npc))
				{
					++numberOfResponses;
					hasResponse = true;
					hasResponse1 = true;
				}
				
				boolean hasResponse2 = false;
				if (outbreak_han_solo_condition__defaultCondition (player, npc))
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
						responses[responseIndex++] = new string_id (c_stringFile, "s_147");
					}
					
					if (hasResponse1)
					{
						responses[responseIndex++] = new string_id (c_stringFile, "s_155");
					}
					
					if (hasResponse2)
					{
						responses[responseIndex++] = new string_id (c_stringFile, "s_160");
					}
					
					utils.setScriptVar (player, "conversation.outbreak_han_solo.branchId", 43);
					
					npcSpeak (player, message);
					npcSetConversationResponses (player, responses);
				}
				else
				{
					utils.removeScriptVar (player, "conversation.outbreak_han_solo.branchId");
					
					npcEndConversationWithMessage (player, message);
				}
				
				return SCRIPT_CONTINUE;
			}
			
		}
		
		if (response.equals("s_160"))
		{
			
			if (outbreak_han_solo_condition__defaultCondition (player, npc))
			{
				outbreak_han_solo_action_grantMissionOne (player, npc);
				
				string_id message = new string_id (c_stringFile, "s_162");
				utils.removeScriptVar (player, "conversation.outbreak_han_solo.branchId");
				
				npcEndConversationWithMessage (player, message);
				
				return SCRIPT_CONTINUE;
			}
			
		}
		
		return SCRIPT_DEFAULT;
	}
	
	
	public int outbreak_han_solo_handleBranch45(obj_id player, obj_id npc, string_id response) throws InterruptedException
	{
		
		if (response.equals("s_147"))
		{
			
			if (outbreak_han_solo_condition__defaultCondition (player, npc))
			{
				
				string_id message = new string_id (c_stringFile, "s_151");
				int numberOfResponses = 0;
				
				boolean hasResponse = false;
				
				boolean hasResponse0 = false;
				if (outbreak_han_solo_condition__defaultCondition (player, npc))
				{
					++numberOfResponses;
					hasResponse = true;
					hasResponse0 = true;
				}
				
				boolean hasResponse1 = false;
				if (outbreak_han_solo_condition__defaultCondition (player, npc))
				{
					++numberOfResponses;
					hasResponse = true;
					hasResponse1 = true;
				}
				
				boolean hasResponse2 = false;
				if (outbreak_han_solo_condition__defaultCondition (player, npc))
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
						responses[responseIndex++] = new string_id (c_stringFile, "s_147");
					}
					
					if (hasResponse1)
					{
						responses[responseIndex++] = new string_id (c_stringFile, "s_155");
					}
					
					if (hasResponse2)
					{
						responses[responseIndex++] = new string_id (c_stringFile, "s_160");
					}
					
					utils.setScriptVar (player, "conversation.outbreak_han_solo.branchId", 43);
					
					npcSpeak (player, message);
					npcSetConversationResponses (player, responses);
				}
				else
				{
					utils.removeScriptVar (player, "conversation.outbreak_han_solo.branchId");
					
					npcEndConversationWithMessage (player, message);
				}
				
				return SCRIPT_CONTINUE;
			}
			
		}
		
		if (response.equals("s_155"))
		{
			
			if (outbreak_han_solo_condition__defaultCondition (player, npc))
			{
				
				string_id message = new string_id (c_stringFile, "s_158");
				int numberOfResponses = 0;
				
				boolean hasResponse = false;
				
				boolean hasResponse0 = false;
				if (outbreak_han_solo_condition__defaultCondition (player, npc))
				{
					++numberOfResponses;
					hasResponse = true;
					hasResponse0 = true;
				}
				
				boolean hasResponse1 = false;
				if (outbreak_han_solo_condition__defaultCondition (player, npc))
				{
					++numberOfResponses;
					hasResponse = true;
					hasResponse1 = true;
				}
				
				boolean hasResponse2 = false;
				if (outbreak_han_solo_condition__defaultCondition (player, npc))
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
						responses[responseIndex++] = new string_id (c_stringFile, "s_147");
					}
					
					if (hasResponse1)
					{
						responses[responseIndex++] = new string_id (c_stringFile, "s_155");
					}
					
					if (hasResponse2)
					{
						responses[responseIndex++] = new string_id (c_stringFile, "s_160");
					}
					
					utils.setScriptVar (player, "conversation.outbreak_han_solo.branchId", 43);
					
					npcSpeak (player, message);
					npcSetConversationResponses (player, responses);
				}
				else
				{
					utils.removeScriptVar (player, "conversation.outbreak_han_solo.branchId");
					
					npcEndConversationWithMessage (player, message);
				}
				
				return SCRIPT_CONTINUE;
			}
			
		}
		
		if (response.equals("s_160"))
		{
			
			if (outbreak_han_solo_condition__defaultCondition (player, npc))
			{
				outbreak_han_solo_action_grantMissionOne (player, npc);
				
				string_id message = new string_id (c_stringFile, "s_162");
				utils.removeScriptVar (player, "conversation.outbreak_han_solo.branchId");
				
				npcEndConversationWithMessage (player, message);
				
				return SCRIPT_CONTINUE;
			}
			
		}
		
		return SCRIPT_DEFAULT;
	}
	
	
	public int outbreak_han_solo_handleBranch47(obj_id player, obj_id npc, string_id response) throws InterruptedException
	{
		
		if (response.equals("s_166"))
		{
			
			if (outbreak_han_solo_condition__defaultCondition (player, npc))
			{
				outbreak_han_solo_action_grantRebQuest (player, npc);
				
				string_id message = new string_id (c_stringFile, "s_168");
				utils.removeScriptVar (player, "conversation.outbreak_han_solo.branchId");
				
				npcEndConversationWithMessage (player, message);
				
				return SCRIPT_CONTINUE;
			}
			
		}
		
		return SCRIPT_DEFAULT;
	}
	
	public static final String STRINGFILE = "theme_park/outbreak/outbreak";
	
	public int OnInitialize(obj_id self) throws InterruptedException
	{
		if ((!isTangible (self)) || (isPlayer (self)))
		{
			detachScript(self, "conversation.outbreak_han_solo");
		}
		
		setCondition (self, CONDITION_CONVERSABLE);
		messageTo(self, "findChewie", null, 2, false);
		CustomerServiceLog("outbreak_themepark", "outbreak_han_solo conversation: Han is looking for Chewie. Self: "+self);
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int OnAttach(obj_id self) throws InterruptedException
	{
		setCondition (self, CONDITION_CONVERSABLE);
		messageTo(self, "findChewie", null, 2, false);
		CustomerServiceLog("outbreak_themepark", "outbreak_han_solo conversation: Han is looking for Chewie. Self: "+self);
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
		detachScript (self, "conversation.outbreak_han_solo");
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int findChewie(obj_id self, dictionary params) throws InterruptedException
	{
		CustomerServiceLog("outbreak_themepark", "outbreak_han_solo findChewie(): Initialized");
		obj_id[] chewieList = getAllObjectsWithObjVar(getLocation(self), 3.f, "chewie");
		
		if (chewieList == null || chewieList.length <= 0)
		{
			CustomerServiceLog("outbreak_themepark", "outbreak_han_solo findChewie(): Failed to find chewie var on any objects within range.");
			messageTo(self, "findChewie", null, 60, false);
			return SCRIPT_CONTINUE;
		}
		CustomerServiceLog("outbreak_themepark", "outbreak_han_solo findChewie(): Found Chewie within range.");
		setObjVar(self, "myChewie", chewieList[0]);
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
		
		if (outbreak_han_solo_condition_hasCompletedAll (player, npc))
		{
			outbreak_han_solo_action_makeChewieWave (player, npc);
			
			string_id message = new string_id (c_stringFile, "s_130");
			int numberOfResponses = 0;
			
			boolean hasResponse = false;
			
			boolean hasResponse0 = false;
			if (outbreak_han_solo_condition__defaultCondition (player, npc))
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
				
				utils.setScriptVar (player, "conversation.outbreak_han_solo.branchId", 1);
				
				npcStartConversation (player, npc, "outbreak_han_solo", message, responses);
			}
			else
			{
				chat.chat (npc, player, message);
			}
			
			return SCRIPT_CONTINUE;
		}
		
		if (outbreak_han_solo_condition_hasCompletedMissions (player, npc))
		{
			doAnimationAction (npc, "thumbs_up");
			
			outbreak_han_solo_action_makeChewieCheer2 (player, npc);
			
			string_id message = new string_id (c_stringFile, "s_123");
			int numberOfResponses = 0;
			
			boolean hasResponse = false;
			
			boolean hasResponse0 = false;
			if (outbreak_han_solo_condition__defaultCondition (player, npc))
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
				
				utils.setScriptVar (player, "conversation.outbreak_han_solo.branchId", 8);
				
				prose_package pp = new prose_package ();
				pp.stringId = message;
				pp.actor.set (player);
				pp.target.set (npc);
				
				npcStartConversation (player, npc, "outbreak_han_solo", null, pp, responses);
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
		
		if (outbreak_han_solo_condition_hasFoundStormtroopers (player, npc))
		{
			outbreak_han_solo_action_makeChewieSayRight (player, npc);
			
			string_id message = new string_id (c_stringFile, "s_122");
			chat.chat (npc, player, message);
			
			return SCRIPT_CONTINUE;
		}
		
		if (outbreak_han_solo_condition_hasSecondQuestImperial (player, npc))
		{
			
			string_id message = new string_id (c_stringFile, "s_121");
			prose_package pp = new prose_package ();
			pp.stringId = message;
			pp.actor.set (player);
			pp.target.set (npc);
			
			chat.chat (npc, player, null, null, pp);
			return SCRIPT_CONTINUE;
		}
		
		if (outbreak_han_solo_condition_hasFirstQuestImperial (player, npc))
		{
			
			string_id message = new string_id (c_stringFile, "s_74");
			prose_package pp = new prose_package ();
			pp.stringId = message;
			pp.actor.set (player);
			pp.target.set (npc);
			
			chat.chat (npc, player, null, null, pp);
			return SCRIPT_CONTINUE;
		}
		
		if (outbreak_han_solo_condition_isPlayerNeutral (player, npc))
		{
			
			string_id message = new string_id (c_stringFile, "s_61");
			int numberOfResponses = 0;
			
			boolean hasResponse = false;
			
			boolean hasResponse0 = false;
			if (outbreak_han_solo_condition__defaultCondition (player, npc))
			{
				++numberOfResponses;
				hasResponse = true;
				hasResponse0 = true;
			}
			
			boolean hasResponse1 = false;
			if (outbreak_han_solo_condition__defaultCondition (player, npc))
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
					responses[responseIndex++] = new string_id (c_stringFile, "s_64");
				}
				
				if (hasResponse1)
				{
					responses[responseIndex++] = new string_id (c_stringFile, "s_90");
				}
				
				utils.setScriptVar (player, "conversation.outbreak_han_solo.branchId", 16);
				
				npcStartConversation (player, npc, "outbreak_han_solo", message, responses);
			}
			else
			{
				chat.chat (npc, player, message);
			}
			
			return SCRIPT_CONTINUE;
		}
		
		if (outbreak_han_solo_condition_isPlayerImperial (player, npc))
		{
			
			string_id message = new string_id (c_stringFile, "s_62");
			int numberOfResponses = 0;
			
			boolean hasResponse = false;
			
			boolean hasResponse0 = false;
			if (outbreak_han_solo_condition__defaultCondition (player, npc))
			{
				++numberOfResponses;
				hasResponse = true;
				hasResponse0 = true;
			}
			
			boolean hasResponse1 = false;
			if (outbreak_han_solo_condition__defaultCondition (player, npc))
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
					responses[responseIndex++] = new string_id (c_stringFile, "s_63");
				}
				
				if (hasResponse1)
				{
					responses[responseIndex++] = new string_id (c_stringFile, "s_89");
				}
				
				utils.setScriptVar (player, "conversation.outbreak_han_solo.branchId", 21);
				
				npcStartConversation (player, npc, "outbreak_han_solo", message, responses);
			}
			else
			{
				chat.chat (npc, player, message);
			}
			
			return SCRIPT_CONTINUE;
		}
		
		if (outbreak_han_solo_condition_isLiveConversation (player, npc))
		{
			doAnimationAction (npc, "nod");
			
			outbreak_han_solo_action_clearLiveConversion (player, npc);
			
			string_id message = new string_id (c_stringFile, "s_47");
			int numberOfResponses = 0;
			
			boolean hasResponse = false;
			
			boolean hasResponse0 = false;
			if (outbreak_han_solo_condition__defaultCondition (player, npc))
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
				
				utils.setScriptVar (player, "conversation.outbreak_han_solo.branchId", 26);
				
				prose_package pp = new prose_package ();
				pp.stringId = message;
				pp.actor.set (player);
				pp.target.set (npc);
				
				npcStartConversation (player, npc, "outbreak_han_solo", null, pp, responses);
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
		
		if (outbreak_han_solo_condition_hasDeletedLiveConversion (player, npc))
		{
			
			string_id message = new string_id (c_stringFile, "s_165");
			int numberOfResponses = 0;
			
			boolean hasResponse = false;
			
			boolean hasResponse0 = false;
			if (outbreak_han_solo_condition__defaultCondition (player, npc))
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
					responses[responseIndex++] = new string_id (c_stringFile, "s_166");
				}
				
				utils.setScriptVar (player, "conversation.outbreak_han_solo.branchId", 47);
				
				npcStartConversation (player, npc, "outbreak_han_solo", message, responses);
			}
			else
			{
				chat.chat (npc, player, message);
			}
			
			return SCRIPT_CONTINUE;
		}
		
		if (outbreak_han_solo_condition__defaultCondition (player, npc))
		{
			outbreak_han_solo_action_makeChewieDirect (player, npc);
			
			string_id message = new string_id (c_stringFile, "s_170");
			chat.chat (npc, player, message);
			
			return SCRIPT_CONTINUE;
		}
		
		chat.chat (npc, "Error: All conditions for OnStartNpcConversation were false.");
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int OnNpcConversationResponse(obj_id self, String conversationId, obj_id player, string_id response) throws InterruptedException
	{
		if (!conversationId.equals("outbreak_han_solo"))
		{
			return SCRIPT_CONTINUE;
		}
		
		obj_id npc = self;
		
		int branchId = utils.getIntScriptVar (player, "conversation.outbreak_han_solo.branchId");
		
		if (branchId == 1 && outbreak_han_solo_handleBranch1 (player, npc, response) == SCRIPT_CONTINUE)
		{
			return SCRIPT_CONTINUE;
		}
		
		if (branchId == 2 && outbreak_han_solo_handleBranch2 (player, npc, response) == SCRIPT_CONTINUE)
		{
			return SCRIPT_CONTINUE;
		}
		
		if (branchId == 3 && outbreak_han_solo_handleBranch3 (player, npc, response) == SCRIPT_CONTINUE)
		{
			return SCRIPT_CONTINUE;
		}
		
		if (branchId == 4 && outbreak_han_solo_handleBranch4 (player, npc, response) == SCRIPT_CONTINUE)
		{
			return SCRIPT_CONTINUE;
		}
		
		if (branchId == 8 && outbreak_han_solo_handleBranch8 (player, npc, response) == SCRIPT_CONTINUE)
		{
			return SCRIPT_CONTINUE;
		}
		
		if (branchId == 9 && outbreak_han_solo_handleBranch9 (player, npc, response) == SCRIPT_CONTINUE)
		{
			return SCRIPT_CONTINUE;
		}
		
		if (branchId == 10 && outbreak_han_solo_handleBranch10 (player, npc, response) == SCRIPT_CONTINUE)
		{
			return SCRIPT_CONTINUE;
		}
		
		if (branchId == 11 && outbreak_han_solo_handleBranch11 (player, npc, response) == SCRIPT_CONTINUE)
		{
			return SCRIPT_CONTINUE;
		}
		
		if (branchId == 16 && outbreak_han_solo_handleBranch16 (player, npc, response) == SCRIPT_CONTINUE)
		{
			return SCRIPT_CONTINUE;
		}
		
		if (branchId == 17 && outbreak_han_solo_handleBranch17 (player, npc, response) == SCRIPT_CONTINUE)
		{
			return SCRIPT_CONTINUE;
		}
		
		if (branchId == 19 && outbreak_han_solo_handleBranch19 (player, npc, response) == SCRIPT_CONTINUE)
		{
			return SCRIPT_CONTINUE;
		}
		
		if (branchId == 20 && outbreak_han_solo_handleBranch20 (player, npc, response) == SCRIPT_CONTINUE)
		{
			return SCRIPT_CONTINUE;
		}
		
		if (branchId == 21 && outbreak_han_solo_handleBranch21 (player, npc, response) == SCRIPT_CONTINUE)
		{
			return SCRIPT_CONTINUE;
		}
		
		if (branchId == 22 && outbreak_han_solo_handleBranch22 (player, npc, response) == SCRIPT_CONTINUE)
		{
			return SCRIPT_CONTINUE;
		}
		
		if (branchId == 24 && outbreak_han_solo_handleBranch24 (player, npc, response) == SCRIPT_CONTINUE)
		{
			return SCRIPT_CONTINUE;
		}
		
		if (branchId == 25 && outbreak_han_solo_handleBranch25 (player, npc, response) == SCRIPT_CONTINUE)
		{
			return SCRIPT_CONTINUE;
		}
		
		if (branchId == 26 && outbreak_han_solo_handleBranch26 (player, npc, response) == SCRIPT_CONTINUE)
		{
			return SCRIPT_CONTINUE;
		}
		
		if (branchId == 27 && outbreak_han_solo_handleBranch27 (player, npc, response) == SCRIPT_CONTINUE)
		{
			return SCRIPT_CONTINUE;
		}
		
		if (branchId == 28 && outbreak_han_solo_handleBranch28 (player, npc, response) == SCRIPT_CONTINUE)
		{
			return SCRIPT_CONTINUE;
		}
		
		if (branchId == 29 && outbreak_han_solo_handleBranch29 (player, npc, response) == SCRIPT_CONTINUE)
		{
			return SCRIPT_CONTINUE;
		}
		
		if (branchId == 30 && outbreak_han_solo_handleBranch30 (player, npc, response) == SCRIPT_CONTINUE)
		{
			return SCRIPT_CONTINUE;
		}
		
		if (branchId == 31 && outbreak_han_solo_handleBranch31 (player, npc, response) == SCRIPT_CONTINUE)
		{
			return SCRIPT_CONTINUE;
		}
		
		if (branchId == 32 && outbreak_han_solo_handleBranch32 (player, npc, response) == SCRIPT_CONTINUE)
		{
			return SCRIPT_CONTINUE;
		}
		
		if (branchId == 33 && outbreak_han_solo_handleBranch33 (player, npc, response) == SCRIPT_CONTINUE)
		{
			return SCRIPT_CONTINUE;
		}
		
		if (branchId == 34 && outbreak_han_solo_handleBranch34 (player, npc, response) == SCRIPT_CONTINUE)
		{
			return SCRIPT_CONTINUE;
		}
		
		if (branchId == 35 && outbreak_han_solo_handleBranch35 (player, npc, response) == SCRIPT_CONTINUE)
		{
			return SCRIPT_CONTINUE;
		}
		
		if (branchId == 36 && outbreak_han_solo_handleBranch36 (player, npc, response) == SCRIPT_CONTINUE)
		{
			return SCRIPT_CONTINUE;
		}
		
		if (branchId == 37 && outbreak_han_solo_handleBranch37 (player, npc, response) == SCRIPT_CONTINUE)
		{
			return SCRIPT_CONTINUE;
		}
		
		if (branchId == 38 && outbreak_han_solo_handleBranch38 (player, npc, response) == SCRIPT_CONTINUE)
		{
			return SCRIPT_CONTINUE;
		}
		
		if (branchId == 39 && outbreak_han_solo_handleBranch39 (player, npc, response) == SCRIPT_CONTINUE)
		{
			return SCRIPT_CONTINUE;
		}
		
		if (branchId == 40 && outbreak_han_solo_handleBranch40 (player, npc, response) == SCRIPT_CONTINUE)
		{
			return SCRIPT_CONTINUE;
		}
		
		if (branchId == 41 && outbreak_han_solo_handleBranch41 (player, npc, response) == SCRIPT_CONTINUE)
		{
			return SCRIPT_CONTINUE;
		}
		
		if (branchId == 42 && outbreak_han_solo_handleBranch42 (player, npc, response) == SCRIPT_CONTINUE)
		{
			return SCRIPT_CONTINUE;
		}
		
		if (branchId == 43 && outbreak_han_solo_handleBranch43 (player, npc, response) == SCRIPT_CONTINUE)
		{
			return SCRIPT_CONTINUE;
		}
		
		if (branchId == 44 && outbreak_han_solo_handleBranch44 (player, npc, response) == SCRIPT_CONTINUE)
		{
			return SCRIPT_CONTINUE;
		}
		
		if (branchId == 45 && outbreak_han_solo_handleBranch45 (player, npc, response) == SCRIPT_CONTINUE)
		{
			return SCRIPT_CONTINUE;
		}
		
		if (branchId == 47 && outbreak_han_solo_handleBranch47 (player, npc, response) == SCRIPT_CONTINUE)
		{
			return SCRIPT_CONTINUE;
		}
		
		chat.chat (npc, "Error: Fell through all branches and responses for OnNpcConversationResponse.");
		
		utils.removeScriptVar (player, "conversation.outbreak_han_solo.branchId");
		
		return SCRIPT_CONTINUE;
	}
	
}
