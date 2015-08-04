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
import script.library.prose;
import script.library.utils;


public class slicer_01_mos_taike extends script.base_script
{
	public slicer_01_mos_taike()
	{
	}
	String c_stringFile = "conversation/slicer_01_mos_taike";
	
	
	public boolean slicer_01_mos_taike_condition__defaultCondition(obj_id player, obj_id npc) throws InterruptedException
	{
		return true;
	}
	
	
	public boolean slicer_01_mos_taike_condition_shouldBeOnLok(obj_id player, obj_id npc) throws InterruptedException
	{
		faceTo(npc, player);
		return groundquests.isQuestActive(player, "quest_06_conspire_cale") && groundquests.isTaskActive(player, "quest_06_conspire_cale", "TravelToLokOutpost");
	}
	
	
	public boolean slicer_01_mos_taike_condition_hasInitialQuest(obj_id player, obj_id npc) throws InterruptedException
	{
		faceTo(npc, player);
		return groundquests.isQuestActive(player, "quest_04_meet_cale_herron");
	}
	
	
	public boolean slicer_01_mos_taike_condition_shouldBeLadyValerian(obj_id player, obj_id npc) throws InterruptedException
	{
		faceTo(npc, player);
		return groundquests.isTaskActive(player, "quest_05_frame_valarians", "gotoLuckyDespot");
	}
	
	
	public boolean slicer_01_mos_taike_condition_hasCompletedInitQuest(obj_id player, obj_id npc) throws InterruptedException
	{
		faceTo(npc, player);
		return groundquests.isTaskActive(player, "quest_04_meet_cale_herron","speakCaleCompleteQuest");
	}
	
	
	public boolean slicer_01_mos_taike_condition_hasCompletedValarian(obj_id player, obj_id npc) throws InterruptedException
	{
		faceTo(npc, player);
		return (slicer_01_mos_taike_condition_failedToDefeatAssassin(player, npc) || slicer_01_mos_taike_condition_successDefeatAssassin(player, npc)) && !groundquests.isQuestActiveOrComplete(player, "quest_07_descend_into_labratory");
	}
	
	
	public boolean slicer_01_mos_taike_condition_failedToDefeatAssassin(obj_id player, obj_id npc) throws InterruptedException
	{
		faceTo(npc, player);
		return groundquests.hasCompletedQuest(player, "quest_05_frame_valarians_failed_assassin") && groundquests.isTaskActive(player, "quest_06_conspire_cale", "visitCaleHerron");
	}
	
	
	public boolean slicer_01_mos_taike_condition_successDefeatAssassin(obj_id player, obj_id npc) throws InterruptedException
	{
		faceTo(npc, player);
		return groundquests.hasCompletedQuest(player, "quest_05_frame_valarians_success_assassin") && groundquests.isTaskActive(player, "quest_06_conspire_cale", "visitCaleHerron");
	}
	
	
	public boolean slicer_01_mos_taike_condition_hasCompletedDathomir(obj_id player, obj_id npc) throws InterruptedException
	{
		faceTo(npc, player);
		return groundquests.isTaskActive(player, "quest_08_dathomir_outpost_final", "finishUpProlog") || groundquests.isTaskActive(player, "quest_08_dathomir_outpost", "finishUpProlog");
	}
	
	
	public boolean slicer_01_mos_taike_condition_isRebelCheck(obj_id player, obj_id npc) throws InterruptedException
	{
		faceTo(npc, player);
		return (factions.isRebel(player));
	}
	
	
	public boolean slicer_01_mos_taike_condition_isNeutralCheck(obj_id player, obj_id npc) throws InterruptedException
	{
		faceTo(npc, player);
		return (!factions.isImperial(player) && !factions.isRebel(player));
	}
	
	
	public boolean slicer_01_mos_taike_condition_isImperialCheck(obj_id player, obj_id npc) throws InterruptedException
	{
		faceTo(npc, player);
		return (factions.isImperial(player));
	}
	
	
	public boolean slicer_01_mos_taike_condition_isRebelOrNeutral(obj_id player, obj_id npc) throws InterruptedException
	{
		faceTo(npc, player);
		return slicer_01_mos_taike_condition_isNeutralCheck(player, npc) || slicer_01_mos_taike_condition_isRebelCheck(player, npc);
	}
	
	
	public boolean slicer_01_mos_taike_condition_hasCompletedProlog(obj_id player, obj_id npc) throws InterruptedException
	{
		return groundquests.hasCompletedQuest(player, "quest_08_dathomir_outpost_final");
	}
	
	
	public boolean slicer_01_mos_taike_condition_canceledQuest06(obj_id player, obj_id npc) throws InterruptedException
	{
		return !groundquests.isQuestActiveOrComplete(player, "quest_06_conspire_cale") && (groundquests.hasCompletedQuest(player, "quest_05_frame_valarians_failed_assassin") || groundquests.hasCompletedQuest(player, "quest_05_frame_valarians_success_assassin"));
	}
	
	
	public boolean slicer_01_mos_taike_condition_isCheckingWithBib(obj_id player, obj_id npc) throws InterruptedException
	{
		faceTo(npc, player);
		return groundquests.isTaskActive(player, "quest_04_meet_cale_herron", "invisTimer") || groundquests.isTaskActive(player, "quest_04_meet_cale_herron", "bibCommCheckin");
	}
	
	
	public boolean slicer_01_mos_taike_condition_completedQuest04Deleted05(obj_id player, obj_id npc) throws InterruptedException
	{
		faceTo(npc, player);
		return groundquests.hasCompletedQuest(player, "quest_04_meet_cale_herron") && !groundquests.isQuestActiveOrComplete(player, "quest_05_frame_valarians") ;
		
	}
	
	
	public void slicer_01_mos_taike_action_sendSignalSpokeCale(obj_id player, obj_id npc) throws InterruptedException
	{
		groundquests.sendSignal(player, "playerSpokeToCaleHerron");
	}
	
	
	public void slicer_01_mos_taike_action_startCommunication(obj_id player, obj_id npc) throws InterruptedException
	{
		prose_package pp = new prose_package();
		prose.setStringId(pp, new string_id("theme_park/outbreak/outbreak", "derg_pratis"));
		commPlayers(player, "object/mobile/dressed_biologist_04.iff", "", 10f, player, pp);
	}
	
	
	public void slicer_01_mos_taike_action_grantFrameValarainQuest(obj_id player, obj_id npc) throws InterruptedException
	{
		groundquests.grantQuest(player, "quest_05_frame_valarians");
	}
	
	
	public void slicer_01_mos_taike_action_sendSignalGoLok(obj_id player, obj_id npc) throws InterruptedException
	{
		groundquests.sendSignal(player, "metWithCaleHerron");
	}
	
	
	public void slicer_01_mos_taike_action_finishPrologGetRewards(obj_id player, obj_id npc) throws InterruptedException
	{
		groundquests.sendSignal(player, "PlayerFinishedUp");
		slicer_01_mos_taike_action_grantOutbreakQuest(player,npc);
	}
	
	
	public void slicer_01_mos_taike_action_grantQuest06(obj_id player, obj_id npc) throws InterruptedException
	{
		groundquests.grantQuest(player, "quest_06_conspire_cale");
	}
	
	
	public void slicer_01_mos_taike_action_sendSignalCompleteQuest04(obj_id player, obj_id npc) throws InterruptedException
	{
		groundquests.sendSignal(player, "spokenCale");
	}
	
	
	public void slicer_01_mos_taike_action_grantOutbreakQuest(obj_id player, obj_id npc) throws InterruptedException
	{
		if (factions.isRebel(player))
		{
			groundquests.grantQuest(player, "outbreak_live_conversion_rebel");
			return;
		}
		else if (factions.isImperial(player))
		{
			groundquests.grantQuest(player, "outbreak_live_conversion_imperial");
			return;
		}
		
		groundquests.grantQuest(player, "outbreak_live_conversion_neutral");
		return;
		
	}
	
	
	public int slicer_01_mos_taike_handleBranch2(obj_id player, obj_id npc, string_id response) throws InterruptedException
	{
		
		if (response.equals("s_129"))
		{
			
			if (slicer_01_mos_taike_condition__defaultCondition (player, npc))
			{
				
				string_id message = new string_id (c_stringFile, "s_131");
				int numberOfResponses = 0;
				
				boolean hasResponse = false;
				
				boolean hasResponse0 = false;
				if (slicer_01_mos_taike_condition__defaultCondition (player, npc))
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
					
					utils.setScriptVar (player, "conversation.slicer_01_mos_taike.branchId", 3);
					
					npcSpeak (player, message);
					npcSetConversationResponses (player, responses);
				}
				else
				{
					utils.removeScriptVar (player, "conversation.slicer_01_mos_taike.branchId");
					
					npcEndConversationWithMessage (player, message);
				}
				
				return SCRIPT_CONTINUE;
			}
			
		}
		
		return SCRIPT_DEFAULT;
	}
	
	
	public int slicer_01_mos_taike_handleBranch3(obj_id player, obj_id npc, string_id response) throws InterruptedException
	{
		
		if (response.equals("s_135"))
		{
			
			if (slicer_01_mos_taike_condition_isRebelOrNeutral (player, npc))
			{
				
				string_id message = new string_id (c_stringFile, "s_136");
				int numberOfResponses = 0;
				
				boolean hasResponse = false;
				
				boolean hasResponse0 = false;
				if (slicer_01_mos_taike_condition__defaultCondition (player, npc))
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
						responses[responseIndex++] = new string_id (c_stringFile, "s_141");
					}
					
					utils.setScriptVar (player, "conversation.slicer_01_mos_taike.branchId", 4);
					
					prose_package pp = new prose_package ();
					pp.stringId = message;
					pp.actor.set (player);
					pp.target.set (npc);
					
					npcSpeak (player, pp);
					npcSetConversationResponses (player, responses);
				}
				else
				{
					utils.removeScriptVar (player, "conversation.slicer_01_mos_taike.branchId");
					
					prose_package pp = new prose_package ();
					pp.stringId = message;
					pp.actor.set (player);
					pp.target.set (npc);
					
					npcEndConversationWithMessage (player, pp);
				}
				
				return SCRIPT_CONTINUE;
			}
			
			if (slicer_01_mos_taike_condition__defaultCondition (player, npc))
			{
				
				string_id message = new string_id (c_stringFile, "s_140");
				int numberOfResponses = 0;
				
				boolean hasResponse = false;
				
				boolean hasResponse0 = false;
				if (slicer_01_mos_taike_condition__defaultCondition (player, npc))
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
						responses[responseIndex++] = new string_id (c_stringFile, "s_143");
					}
					
					utils.setScriptVar (player, "conversation.slicer_01_mos_taike.branchId", 6);
					
					prose_package pp = new prose_package ();
					pp.stringId = message;
					pp.actor.set (player);
					pp.target.set (npc);
					
					npcSpeak (player, pp);
					npcSetConversationResponses (player, responses);
				}
				else
				{
					utils.removeScriptVar (player, "conversation.slicer_01_mos_taike.branchId");
					
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
	
	
	public int slicer_01_mos_taike_handleBranch4(obj_id player, obj_id npc, string_id response) throws InterruptedException
	{
		
		if (response.equals("s_141"))
		{
			
			if (slicer_01_mos_taike_condition__defaultCondition (player, npc))
			{
				slicer_01_mos_taike_action_finishPrologGetRewards (player, npc);
				
				string_id message = new string_id (c_stringFile, "s_142");
				utils.removeScriptVar (player, "conversation.slicer_01_mos_taike.branchId");
				
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
	
	
	public int slicer_01_mos_taike_handleBranch6(obj_id player, obj_id npc, string_id response) throws InterruptedException
	{
		
		if (response.equals("s_143"))
		{
			
			if (slicer_01_mos_taike_condition__defaultCondition (player, npc))
			{
				slicer_01_mos_taike_action_finishPrologGetRewards (player, npc);
				
				string_id message = new string_id (c_stringFile, "s_142");
				utils.removeScriptVar (player, "conversation.slicer_01_mos_taike.branchId");
				
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
	
	
	public int slicer_01_mos_taike_handleBranch9(obj_id player, obj_id npc, string_id response) throws InterruptedException
	{
		
		if (response.equals("s_71"))
		{
			
			if (slicer_01_mos_taike_condition__defaultCondition (player, npc))
			{
				
				string_id message = new string_id (c_stringFile, "s_74");
				int numberOfResponses = 0;
				
				boolean hasResponse = false;
				
				boolean hasResponse0 = false;
				if (slicer_01_mos_taike_condition__defaultCondition (player, npc))
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
					
					utils.setScriptVar (player, "conversation.slicer_01_mos_taike.branchId", 11);
					
					npcSpeak (player, message);
					npcSetConversationResponses (player, responses);
				}
				else
				{
					utils.removeScriptVar (player, "conversation.slicer_01_mos_taike.branchId");
					
					npcEndConversationWithMessage (player, message);
				}
				
				return SCRIPT_CONTINUE;
			}
			
		}
		
		if (response.equals("s_72"))
		{
			
			if (slicer_01_mos_taike_condition__defaultCondition (player, npc))
			{
				
				string_id message = new string_id (c_stringFile, "s_73");
				int numberOfResponses = 0;
				
				boolean hasResponse = false;
				
				boolean hasResponse0 = false;
				if (slicer_01_mos_taike_condition__defaultCondition (player, npc))
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
					
					utils.setScriptVar (player, "conversation.slicer_01_mos_taike.branchId", 11);
					
					prose_package pp = new prose_package ();
					pp.stringId = message;
					pp.actor.set (player);
					pp.target.set (npc);
					
					npcSpeak (player, pp);
					npcSetConversationResponses (player, responses);
				}
				else
				{
					utils.removeScriptVar (player, "conversation.slicer_01_mos_taike.branchId");
					
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
	
	
	public int slicer_01_mos_taike_handleBranch10(obj_id player, obj_id npc, string_id response) throws InterruptedException
	{
		
		if (response.equals("s_75"))
		{
			
			if (slicer_01_mos_taike_condition__defaultCondition (player, npc))
			{
				doAnimationAction (npc, "apologize");
				
				string_id message = new string_id (c_stringFile, "s_76");
				int numberOfResponses = 0;
				
				boolean hasResponse = false;
				
				boolean hasResponse0 = false;
				if (slicer_01_mos_taike_condition__defaultCondition (player, npc))
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
						responses[responseIndex++] = new string_id (c_stringFile, "s_77");
					}
					
					utils.setScriptVar (player, "conversation.slicer_01_mos_taike.branchId", 12);
					
					npcSpeak (player, message);
					npcSetConversationResponses (player, responses);
				}
				else
				{
					utils.removeScriptVar (player, "conversation.slicer_01_mos_taike.branchId");
					
					npcEndConversationWithMessage (player, message);
				}
				
				return SCRIPT_CONTINUE;
			}
			
		}
		
		return SCRIPT_DEFAULT;
	}
	
	
	public int slicer_01_mos_taike_handleBranch11(obj_id player, obj_id npc, string_id response) throws InterruptedException
	{
		
		if (response.equals("s_75"))
		{
			
			if (slicer_01_mos_taike_condition__defaultCondition (player, npc))
			{
				doAnimationAction (npc, "apologize");
				
				string_id message = new string_id (c_stringFile, "s_76");
				int numberOfResponses = 0;
				
				boolean hasResponse = false;
				
				boolean hasResponse0 = false;
				if (slicer_01_mos_taike_condition__defaultCondition (player, npc))
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
						responses[responseIndex++] = new string_id (c_stringFile, "s_77");
					}
					
					utils.setScriptVar (player, "conversation.slicer_01_mos_taike.branchId", 12);
					
					npcSpeak (player, message);
					npcSetConversationResponses (player, responses);
				}
				else
				{
					utils.removeScriptVar (player, "conversation.slicer_01_mos_taike.branchId");
					
					npcEndConversationWithMessage (player, message);
				}
				
				return SCRIPT_CONTINUE;
			}
			
		}
		
		return SCRIPT_DEFAULT;
	}
	
	
	public int slicer_01_mos_taike_handleBranch12(obj_id player, obj_id npc, string_id response) throws InterruptedException
	{
		
		if (response.equals("s_77"))
		{
			
			if (slicer_01_mos_taike_condition__defaultCondition (player, npc))
			{
				
				string_id message = new string_id (c_stringFile, "s_78");
				int numberOfResponses = 0;
				
				boolean hasResponse = false;
				
				boolean hasResponse0 = false;
				if (slicer_01_mos_taike_condition__defaultCondition (player, npc))
				{
					++numberOfResponses;
					hasResponse = true;
					hasResponse0 = true;
				}
				
				boolean hasResponse1 = false;
				if (slicer_01_mos_taike_condition__defaultCondition (player, npc))
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
						responses[responseIndex++] = new string_id (c_stringFile, "s_79");
					}
					
					if (hasResponse1)
					{
						responses[responseIndex++] = new string_id (c_stringFile, "s_80");
					}
					
					utils.setScriptVar (player, "conversation.slicer_01_mos_taike.branchId", 13);
					
					npcSpeak (player, message);
					npcSetConversationResponses (player, responses);
				}
				else
				{
					utils.removeScriptVar (player, "conversation.slicer_01_mos_taike.branchId");
					
					npcEndConversationWithMessage (player, message);
				}
				
				return SCRIPT_CONTINUE;
			}
			
		}
		
		return SCRIPT_DEFAULT;
	}
	
	
	public int slicer_01_mos_taike_handleBranch13(obj_id player, obj_id npc, string_id response) throws InterruptedException
	{
		
		if (response.equals("s_79"))
		{
			
			if (slicer_01_mos_taike_condition__defaultCondition (player, npc))
			{
				
				string_id message = new string_id (c_stringFile, "s_81");
				int numberOfResponses = 0;
				
				boolean hasResponse = false;
				
				boolean hasResponse0 = false;
				if (slicer_01_mos_taike_condition__defaultCondition (player, npc))
				{
					++numberOfResponses;
					hasResponse = true;
					hasResponse0 = true;
				}
				
				boolean hasResponse1 = false;
				if (slicer_01_mos_taike_condition__defaultCondition (player, npc))
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
						responses[responseIndex++] = new string_id (c_stringFile, "s_82");
					}
					
					if (hasResponse1)
					{
						responses[responseIndex++] = new string_id (c_stringFile, "s_83");
					}
					
					utils.setScriptVar (player, "conversation.slicer_01_mos_taike.branchId", 14);
					
					npcSpeak (player, message);
					npcSetConversationResponses (player, responses);
				}
				else
				{
					utils.removeScriptVar (player, "conversation.slicer_01_mos_taike.branchId");
					
					npcEndConversationWithMessage (player, message);
				}
				
				return SCRIPT_CONTINUE;
			}
			
		}
		
		if (response.equals("s_80"))
		{
			
			if (slicer_01_mos_taike_condition__defaultCondition (player, npc))
			{
				
				string_id message = new string_id (c_stringFile, "s_86");
				int numberOfResponses = 0;
				
				boolean hasResponse = false;
				
				boolean hasResponse0 = false;
				if (slicer_01_mos_taike_condition__defaultCondition (player, npc))
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
					
					utils.setScriptVar (player, "conversation.slicer_01_mos_taike.branchId", 17);
					
					npcSpeak (player, message);
					npcSetConversationResponses (player, responses);
				}
				else
				{
					utils.removeScriptVar (player, "conversation.slicer_01_mos_taike.branchId");
					
					npcEndConversationWithMessage (player, message);
				}
				
				return SCRIPT_CONTINUE;
			}
			
		}
		
		return SCRIPT_DEFAULT;
	}
	
	
	public int slicer_01_mos_taike_handleBranch14(obj_id player, obj_id npc, string_id response) throws InterruptedException
	{
		
		if (response.equals("s_82"))
		{
			
			if (slicer_01_mos_taike_condition__defaultCondition (player, npc))
			{
				slicer_01_mos_taike_action_sendSignalGoLok (player, npc);
				
				string_id message = new string_id (c_stringFile, "s_92");
				utils.removeScriptVar (player, "conversation.slicer_01_mos_taike.branchId");
				
				npcEndConversationWithMessage (player, message);
				
				return SCRIPT_CONTINUE;
			}
			
		}
		
		if (response.equals("s_83"))
		{
			
			if (slicer_01_mos_taike_condition__defaultCondition (player, npc))
			{
				
				string_id message = new string_id (c_stringFile, "s_85");
				int numberOfResponses = 0;
				
				boolean hasResponse = false;
				
				boolean hasResponse0 = false;
				if (slicer_01_mos_taike_condition__defaultCondition (player, npc))
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
						responses[responseIndex++] = new string_id (c_stringFile, "s_91");
					}
					
					utils.setScriptVar (player, "conversation.slicer_01_mos_taike.branchId", 15);
					
					npcSpeak (player, message);
					npcSetConversationResponses (player, responses);
				}
				else
				{
					utils.removeScriptVar (player, "conversation.slicer_01_mos_taike.branchId");
					
					npcEndConversationWithMessage (player, message);
				}
				
				return SCRIPT_CONTINUE;
			}
			
		}
		
		return SCRIPT_DEFAULT;
	}
	
	
	public int slicer_01_mos_taike_handleBranch15(obj_id player, obj_id npc, string_id response) throws InterruptedException
	{
		
		if (response.equals("s_91"))
		{
			
			if (slicer_01_mos_taike_condition__defaultCondition (player, npc))
			{
				slicer_01_mos_taike_action_sendSignalGoLok (player, npc);
				
				string_id message = new string_id (c_stringFile, "s_92");
				utils.removeScriptVar (player, "conversation.slicer_01_mos_taike.branchId");
				
				npcEndConversationWithMessage (player, message);
				
				return SCRIPT_CONTINUE;
			}
			
		}
		
		return SCRIPT_DEFAULT;
	}
	
	
	public int slicer_01_mos_taike_handleBranch17(obj_id player, obj_id npc, string_id response) throws InterruptedException
	{
		
		if (response.equals("s_87"))
		{
			
			if (slicer_01_mos_taike_condition_isRebelOrNeutral (player, npc))
			{
				
				string_id message = new string_id (c_stringFile, "s_88");
				int numberOfResponses = 0;
				
				boolean hasResponse = false;
				
				boolean hasResponse0 = false;
				if (slicer_01_mos_taike_condition__defaultCondition (player, npc))
				{
					++numberOfResponses;
					hasResponse = true;
					hasResponse0 = true;
				}
				
				boolean hasResponse1 = false;
				if (slicer_01_mos_taike_condition__defaultCondition (player, npc))
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
						responses[responseIndex++] = new string_id (c_stringFile, "s_89");
					}
					
					if (hasResponse1)
					{
						responses[responseIndex++] = new string_id (c_stringFile, "s_90");
					}
					
					utils.setScriptVar (player, "conversation.slicer_01_mos_taike.branchId", 18);
					
					npcSpeak (player, message);
					npcSetConversationResponses (player, responses);
				}
				else
				{
					utils.removeScriptVar (player, "conversation.slicer_01_mos_taike.branchId");
					
					npcEndConversationWithMessage (player, message);
				}
				
				return SCRIPT_CONTINUE;
			}
			
			if (slicer_01_mos_taike_condition_isImperialCheck (player, npc))
			{
				
				string_id message = new string_id (c_stringFile, "s_137");
				int numberOfResponses = 0;
				
				boolean hasResponse = false;
				
				boolean hasResponse0 = false;
				if (slicer_01_mos_taike_condition__defaultCondition (player, npc))
				{
					++numberOfResponses;
					hasResponse = true;
					hasResponse0 = true;
				}
				
				boolean hasResponse1 = false;
				if (slicer_01_mos_taike_condition__defaultCondition (player, npc))
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
						responses[responseIndex++] = new string_id (c_stringFile, "s_138");
					}
					
					if (hasResponse1)
					{
						responses[responseIndex++] = new string_id (c_stringFile, "s_139");
					}
					
					utils.setScriptVar (player, "conversation.slicer_01_mos_taike.branchId", 22);
					
					npcSpeak (player, message);
					npcSetConversationResponses (player, responses);
				}
				else
				{
					utils.removeScriptVar (player, "conversation.slicer_01_mos_taike.branchId");
					
					npcEndConversationWithMessage (player, message);
				}
				
				return SCRIPT_CONTINUE;
			}
			
		}
		
		return SCRIPT_DEFAULT;
	}
	
	
	public int slicer_01_mos_taike_handleBranch18(obj_id player, obj_id npc, string_id response) throws InterruptedException
	{
		
		if (response.equals("s_89"))
		{
			
			if (slicer_01_mos_taike_condition__defaultCondition (player, npc))
			{
				
				string_id message = new string_id (c_stringFile, "s_93");
				int numberOfResponses = 0;
				
				boolean hasResponse = false;
				
				boolean hasResponse0 = false;
				if (slicer_01_mos_taike_condition__defaultCondition (player, npc))
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
					
					utils.setScriptVar (player, "conversation.slicer_01_mos_taike.branchId", 19);
					
					npcSpeak (player, message);
					npcSetConversationResponses (player, responses);
				}
				else
				{
					utils.removeScriptVar (player, "conversation.slicer_01_mos_taike.branchId");
					
					npcEndConversationWithMessage (player, message);
				}
				
				return SCRIPT_CONTINUE;
			}
			
		}
		
		if (response.equals("s_90"))
		{
			
			if (slicer_01_mos_taike_condition__defaultCondition (player, npc))
			{
				
				string_id message = new string_id (c_stringFile, "s_81");
				int numberOfResponses = 0;
				
				boolean hasResponse = false;
				
				boolean hasResponse0 = false;
				if (slicer_01_mos_taike_condition__defaultCondition (player, npc))
				{
					++numberOfResponses;
					hasResponse = true;
					hasResponse0 = true;
				}
				
				boolean hasResponse1 = false;
				if (slicer_01_mos_taike_condition__defaultCondition (player, npc))
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
						responses[responseIndex++] = new string_id (c_stringFile, "s_82");
					}
					
					if (hasResponse1)
					{
						responses[responseIndex++] = new string_id (c_stringFile, "s_83");
					}
					
					utils.setScriptVar (player, "conversation.slicer_01_mos_taike.branchId", 14);
					
					npcSpeak (player, message);
					npcSetConversationResponses (player, responses);
				}
				else
				{
					utils.removeScriptVar (player, "conversation.slicer_01_mos_taike.branchId");
					
					npcEndConversationWithMessage (player, message);
				}
				
				return SCRIPT_CONTINUE;
			}
			
		}
		
		return SCRIPT_DEFAULT;
	}
	
	
	public int slicer_01_mos_taike_handleBranch19(obj_id player, obj_id npc, string_id response) throws InterruptedException
	{
		
		if (response.equals("s_94"))
		{
			
			if (slicer_01_mos_taike_condition__defaultCondition (player, npc))
			{
				
				string_id message = new string_id (c_stringFile, "s_95");
				int numberOfResponses = 0;
				
				boolean hasResponse = false;
				
				boolean hasResponse0 = false;
				if (slicer_01_mos_taike_condition__defaultCondition (player, npc))
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
					
					utils.setScriptVar (player, "conversation.slicer_01_mos_taike.branchId", 20);
					
					npcSpeak (player, message);
					npcSetConversationResponses (player, responses);
				}
				else
				{
					utils.removeScriptVar (player, "conversation.slicer_01_mos_taike.branchId");
					
					npcEndConversationWithMessage (player, message);
				}
				
				return SCRIPT_CONTINUE;
			}
			
		}
		
		return SCRIPT_DEFAULT;
	}
	
	
	public int slicer_01_mos_taike_handleBranch20(obj_id player, obj_id npc, string_id response) throws InterruptedException
	{
		
		if (response.equals("s_96"))
		{
			
			if (slicer_01_mos_taike_condition__defaultCondition (player, npc))
			{
				
				string_id message = new string_id (c_stringFile, "s_97");
				int numberOfResponses = 0;
				
				boolean hasResponse = false;
				
				boolean hasResponse0 = false;
				if (slicer_01_mos_taike_condition__defaultCondition (player, npc))
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
						responses[responseIndex++] = new string_id (c_stringFile, "s_98");
					}
					
					utils.setScriptVar (player, "conversation.slicer_01_mos_taike.branchId", 21);
					
					npcSpeak (player, message);
					npcSetConversationResponses (player, responses);
				}
				else
				{
					utils.removeScriptVar (player, "conversation.slicer_01_mos_taike.branchId");
					
					npcEndConversationWithMessage (player, message);
				}
				
				return SCRIPT_CONTINUE;
			}
			
		}
		
		return SCRIPT_DEFAULT;
	}
	
	
	public int slicer_01_mos_taike_handleBranch21(obj_id player, obj_id npc, string_id response) throws InterruptedException
	{
		
		if (response.equals("s_98"))
		{
			
			if (slicer_01_mos_taike_condition__defaultCondition (player, npc))
			{
				slicer_01_mos_taike_action_sendSignalGoLok (player, npc);
				
				string_id message = new string_id (c_stringFile, "s_92");
				utils.removeScriptVar (player, "conversation.slicer_01_mos_taike.branchId");
				
				npcEndConversationWithMessage (player, message);
				
				return SCRIPT_CONTINUE;
			}
			
		}
		
		return SCRIPT_DEFAULT;
	}
	
	
	public int slicer_01_mos_taike_handleBranch22(obj_id player, obj_id npc, string_id response) throws InterruptedException
	{
		
		if (response.equals("s_138"))
		{
			
			if (slicer_01_mos_taike_condition__defaultCondition (player, npc))
			{
				slicer_01_mos_taike_action_sendSignalGoLok (player, npc);
				
				string_id message = new string_id (c_stringFile, "s_92");
				utils.removeScriptVar (player, "conversation.slicer_01_mos_taike.branchId");
				
				npcEndConversationWithMessage (player, message);
				
				return SCRIPT_CONTINUE;
			}
			
		}
		
		if (response.equals("s_139"))
		{
			
			if (slicer_01_mos_taike_condition__defaultCondition (player, npc))
			{
				
				string_id message = new string_id (c_stringFile, "s_81");
				int numberOfResponses = 0;
				
				boolean hasResponse = false;
				
				boolean hasResponse0 = false;
				if (slicer_01_mos_taike_condition__defaultCondition (player, npc))
				{
					++numberOfResponses;
					hasResponse = true;
					hasResponse0 = true;
				}
				
				boolean hasResponse1 = false;
				if (slicer_01_mos_taike_condition__defaultCondition (player, npc))
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
						responses[responseIndex++] = new string_id (c_stringFile, "s_82");
					}
					
					if (hasResponse1)
					{
						responses[responseIndex++] = new string_id (c_stringFile, "s_83");
					}
					
					utils.setScriptVar (player, "conversation.slicer_01_mos_taike.branchId", 14);
					
					npcSpeak (player, message);
					npcSetConversationResponses (player, responses);
				}
				else
				{
					utils.removeScriptVar (player, "conversation.slicer_01_mos_taike.branchId");
					
					npcEndConversationWithMessage (player, message);
				}
				
				return SCRIPT_CONTINUE;
			}
			
		}
		
		return SCRIPT_DEFAULT;
	}
	
	
	public int slicer_01_mos_taike_handleBranch25(obj_id player, obj_id npc, string_id response) throws InterruptedException
	{
		
		if (response.equals("s_55"))
		{
			doAnimationAction (player, "wave_on_dismissing");
			
			if (slicer_01_mos_taike_condition__defaultCondition (player, npc))
			{
				doAnimationAction (npc, "wave_finger_warning");
				
				string_id message = new string_id (c_stringFile, "s_56");
				int numberOfResponses = 0;
				
				boolean hasResponse = false;
				
				boolean hasResponse0 = false;
				if (slicer_01_mos_taike_condition__defaultCondition (player, npc))
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
					
					utils.setScriptVar (player, "conversation.slicer_01_mos_taike.branchId", 26);
					
					npcSpeak (player, message);
					npcSetConversationResponses (player, responses);
				}
				else
				{
					utils.removeScriptVar (player, "conversation.slicer_01_mos_taike.branchId");
					
					npcEndConversationWithMessage (player, message);
				}
				
				return SCRIPT_CONTINUE;
			}
			
		}
		
		return SCRIPT_DEFAULT;
	}
	
	
	public int slicer_01_mos_taike_handleBranch26(obj_id player, obj_id npc, string_id response) throws InterruptedException
	{
		
		if (response.equals("s_57"))
		{
			doAnimationAction (player, "shrug_hands");
			
			if (slicer_01_mos_taike_condition__defaultCondition (player, npc))
			{
				
				string_id message = new string_id (c_stringFile, "s_170");
				int numberOfResponses = 0;
				
				boolean hasResponse = false;
				
				boolean hasResponse0 = false;
				if (slicer_01_mos_taike_condition__defaultCondition (player, npc))
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
						responses[responseIndex++] = new string_id (c_stringFile, "s_171");
					}
					
					utils.setScriptVar (player, "conversation.slicer_01_mos_taike.branchId", 27);
					
					npcSpeak (player, message);
					npcSetConversationResponses (player, responses);
				}
				else
				{
					utils.removeScriptVar (player, "conversation.slicer_01_mos_taike.branchId");
					
					npcEndConversationWithMessage (player, message);
				}
				
				return SCRIPT_CONTINUE;
			}
			
		}
		
		return SCRIPT_DEFAULT;
	}
	
	
	public int slicer_01_mos_taike_handleBranch27(obj_id player, obj_id npc, string_id response) throws InterruptedException
	{
		
		if (response.equals("s_171"))
		{
			
			if (slicer_01_mos_taike_condition__defaultCondition (player, npc))
			{
				doAnimationAction (npc, "poke");
				
				string_id message = new string_id (c_stringFile, "s_175");
				int numberOfResponses = 0;
				
				boolean hasResponse = false;
				
				boolean hasResponse0 = false;
				if (slicer_01_mos_taike_condition__defaultCondition (player, npc))
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
						responses[responseIndex++] = new string_id (c_stringFile, "s_179");
					}
					
					utils.setScriptVar (player, "conversation.slicer_01_mos_taike.branchId", 28);
					
					npcSpeak (player, message);
					npcSetConversationResponses (player, responses);
				}
				else
				{
					utils.removeScriptVar (player, "conversation.slicer_01_mos_taike.branchId");
					
					npcEndConversationWithMessage (player, message);
				}
				
				return SCRIPT_CONTINUE;
			}
			
		}
		
		return SCRIPT_DEFAULT;
	}
	
	
	public int slicer_01_mos_taike_handleBranch28(obj_id player, obj_id npc, string_id response) throws InterruptedException
	{
		
		if (response.equals("s_179"))
		{
			
			if (slicer_01_mos_taike_condition__defaultCondition (player, npc))
			{
				
				string_id message = new string_id (c_stringFile, "s_183");
				int numberOfResponses = 0;
				
				boolean hasResponse = false;
				
				boolean hasResponse0 = false;
				if (slicer_01_mos_taike_condition__defaultCondition (player, npc))
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
						responses[responseIndex++] = new string_id (c_stringFile, "s_187");
					}
					
					utils.setScriptVar (player, "conversation.slicer_01_mos_taike.branchId", 29);
					
					npcSpeak (player, message);
					npcSetConversationResponses (player, responses);
				}
				else
				{
					utils.removeScriptVar (player, "conversation.slicer_01_mos_taike.branchId");
					
					npcEndConversationWithMessage (player, message);
				}
				
				return SCRIPT_CONTINUE;
			}
			
		}
		
		return SCRIPT_DEFAULT;
	}
	
	
	public int slicer_01_mos_taike_handleBranch29(obj_id player, obj_id npc, string_id response) throws InterruptedException
	{
		
		if (response.equals("s_187"))
		{
			
			if (slicer_01_mos_taike_condition__defaultCondition (player, npc))
			{
				
				string_id message = new string_id (c_stringFile, "s_191");
				int numberOfResponses = 0;
				
				boolean hasResponse = false;
				
				boolean hasResponse0 = false;
				if (slicer_01_mos_taike_condition__defaultCondition (player, npc))
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
						responses[responseIndex++] = new string_id (c_stringFile, "s_194");
					}
					
					utils.setScriptVar (player, "conversation.slicer_01_mos_taike.branchId", 30);
					
					npcSpeak (player, message);
					npcSetConversationResponses (player, responses);
				}
				else
				{
					utils.removeScriptVar (player, "conversation.slicer_01_mos_taike.branchId");
					
					npcEndConversationWithMessage (player, message);
				}
				
				return SCRIPT_CONTINUE;
			}
			
		}
		
		return SCRIPT_DEFAULT;
	}
	
	
	public int slicer_01_mos_taike_handleBranch30(obj_id player, obj_id npc, string_id response) throws InterruptedException
	{
		
		if (response.equals("s_194"))
		{
			
			if (slicer_01_mos_taike_condition__defaultCondition (player, npc))
			{
				
				string_id message = new string_id (c_stringFile, "s_196");
				int numberOfResponses = 0;
				
				boolean hasResponse = false;
				
				boolean hasResponse0 = false;
				if (slicer_01_mos_taike_condition__defaultCondition (player, npc))
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
						responses[responseIndex++] = new string_id (c_stringFile, "s_198");
					}
					
					utils.setScriptVar (player, "conversation.slicer_01_mos_taike.branchId", 31);
					
					npcSpeak (player, message);
					npcSetConversationResponses (player, responses);
				}
				else
				{
					utils.removeScriptVar (player, "conversation.slicer_01_mos_taike.branchId");
					
					npcEndConversationWithMessage (player, message);
				}
				
				return SCRIPT_CONTINUE;
			}
			
		}
		
		return SCRIPT_DEFAULT;
	}
	
	
	public int slicer_01_mos_taike_handleBranch31(obj_id player, obj_id npc, string_id response) throws InterruptedException
	{
		
		if (response.equals("s_198"))
		{
			
			if (slicer_01_mos_taike_condition__defaultCondition (player, npc))
			{
				doAnimationAction (npc, "shrug_shoulders");
				
				string_id message = new string_id (c_stringFile, "s_200");
				int numberOfResponses = 0;
				
				boolean hasResponse = false;
				
				boolean hasResponse0 = false;
				if (slicer_01_mos_taike_condition__defaultCondition (player, npc))
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
						responses[responseIndex++] = new string_id (c_stringFile, "s_202");
					}
					
					utils.setScriptVar (player, "conversation.slicer_01_mos_taike.branchId", 32);
					
					npcSpeak (player, message);
					npcSetConversationResponses (player, responses);
				}
				else
				{
					utils.removeScriptVar (player, "conversation.slicer_01_mos_taike.branchId");
					
					npcEndConversationWithMessage (player, message);
				}
				
				return SCRIPT_CONTINUE;
			}
			
		}
		
		return SCRIPT_DEFAULT;
	}
	
	
	public int slicer_01_mos_taike_handleBranch32(obj_id player, obj_id npc, string_id response) throws InterruptedException
	{
		
		if (response.equals("s_202"))
		{
			doAnimationAction (player, "shake_head_no");
			
			if (slicer_01_mos_taike_condition__defaultCondition (player, npc))
			{
				
				string_id message = new string_id (c_stringFile, "s_204");
				int numberOfResponses = 0;
				
				boolean hasResponse = false;
				
				boolean hasResponse0 = false;
				if (slicer_01_mos_taike_condition__defaultCondition (player, npc))
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
						responses[responseIndex++] = new string_id (c_stringFile, "s_206");
					}
					
					utils.setScriptVar (player, "conversation.slicer_01_mos_taike.branchId", 33);
					
					npcSpeak (player, message);
					npcSetConversationResponses (player, responses);
				}
				else
				{
					utils.removeScriptVar (player, "conversation.slicer_01_mos_taike.branchId");
					
					npcEndConversationWithMessage (player, message);
				}
				
				return SCRIPT_CONTINUE;
			}
			
		}
		
		return SCRIPT_DEFAULT;
	}
	
	
	public int slicer_01_mos_taike_handleBranch33(obj_id player, obj_id npc, string_id response) throws InterruptedException
	{
		
		if (response.equals("s_206"))
		{
			
			if (slicer_01_mos_taike_condition__defaultCondition (player, npc))
			{
				
				string_id message = new string_id (c_stringFile, "s_208");
				int numberOfResponses = 0;
				
				boolean hasResponse = false;
				
				boolean hasResponse0 = false;
				if (slicer_01_mos_taike_condition__defaultCondition (player, npc))
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
						responses[responseIndex++] = new string_id (c_stringFile, "s_210");
					}
					
					utils.setScriptVar (player, "conversation.slicer_01_mos_taike.branchId", 34);
					
					npcSpeak (player, message);
					npcSetConversationResponses (player, responses);
				}
				else
				{
					utils.removeScriptVar (player, "conversation.slicer_01_mos_taike.branchId");
					
					npcEndConversationWithMessage (player, message);
				}
				
				return SCRIPT_CONTINUE;
			}
			
		}
		
		return SCRIPT_DEFAULT;
	}
	
	
	public int slicer_01_mos_taike_handleBranch34(obj_id player, obj_id npc, string_id response) throws InterruptedException
	{
		
		if (response.equals("s_210"))
		{
			doAnimationAction (player, "shake_head_disgust");
			
			if (slicer_01_mos_taike_condition__defaultCondition (player, npc))
			{
				doAnimationAction (npc, "slit_throat");
				
				string_id message = new string_id (c_stringFile, "s_212");
				int numberOfResponses = 0;
				
				boolean hasResponse = false;
				
				boolean hasResponse0 = false;
				if (slicer_01_mos_taike_condition__defaultCondition (player, npc))
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
						responses[responseIndex++] = new string_id (c_stringFile, "s_214");
					}
					
					utils.setScriptVar (player, "conversation.slicer_01_mos_taike.branchId", 35);
					
					npcSpeak (player, message);
					npcSetConversationResponses (player, responses);
				}
				else
				{
					utils.removeScriptVar (player, "conversation.slicer_01_mos_taike.branchId");
					
					npcEndConversationWithMessage (player, message);
				}
				
				return SCRIPT_CONTINUE;
			}
			
		}
		
		return SCRIPT_DEFAULT;
	}
	
	
	public int slicer_01_mos_taike_handleBranch35(obj_id player, obj_id npc, string_id response) throws InterruptedException
	{
		
		if (response.equals("s_214"))
		{
			doAnimationAction (player, "nervous");
			
			if (slicer_01_mos_taike_condition__defaultCondition (player, npc))
			{
				slicer_01_mos_taike_action_sendSignalCompleteQuest04 (player, npc);
				
				string_id message = new string_id (c_stringFile, "s_216");
				utils.removeScriptVar (player, "conversation.slicer_01_mos_taike.branchId");
				
				npcEndConversationWithMessage (player, message);
				
				return SCRIPT_CONTINUE;
			}
			
		}
		
		return SCRIPT_DEFAULT;
	}
	
	
	public int slicer_01_mos_taike_handleBranch38(obj_id player, obj_id npc, string_id response) throws InterruptedException
	{
		
		if (response.equals("s_109"))
		{
			
			if (slicer_01_mos_taike_condition__defaultCondition (player, npc))
			{
				
				string_id message = new string_id (c_stringFile, "s_111");
				int numberOfResponses = 0;
				
				boolean hasResponse = false;
				
				boolean hasResponse0 = false;
				if (slicer_01_mos_taike_condition__defaultCondition (player, npc))
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
					
					utils.setScriptVar (player, "conversation.slicer_01_mos_taike.branchId", 39);
					
					npcSpeak (player, message);
					npcSetConversationResponses (player, responses);
				}
				else
				{
					utils.removeScriptVar (player, "conversation.slicer_01_mos_taike.branchId");
					
					npcEndConversationWithMessage (player, message);
				}
				
				return SCRIPT_CONTINUE;
			}
			
		}
		
		return SCRIPT_DEFAULT;
	}
	
	
	public int slicer_01_mos_taike_handleBranch39(obj_id player, obj_id npc, string_id response) throws InterruptedException
	{
		
		if (response.equals("s_113"))
		{
			
			if (slicer_01_mos_taike_condition__defaultCondition (player, npc))
			{
				doAnimationAction (npc, "squirm");
				
				string_id message = new string_id (c_stringFile, "s_115");
				int numberOfResponses = 0;
				
				boolean hasResponse = false;
				
				boolean hasResponse0 = false;
				if (slicer_01_mos_taike_condition__defaultCondition (player, npc))
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
					
					utils.setScriptVar (player, "conversation.slicer_01_mos_taike.branchId", 40);
					
					npcSpeak (player, message);
					npcSetConversationResponses (player, responses);
				}
				else
				{
					utils.removeScriptVar (player, "conversation.slicer_01_mos_taike.branchId");
					
					npcEndConversationWithMessage (player, message);
				}
				
				return SCRIPT_CONTINUE;
			}
			
		}
		
		return SCRIPT_DEFAULT;
	}
	
	
	public int slicer_01_mos_taike_handleBranch40(obj_id player, obj_id npc, string_id response) throws InterruptedException
	{
		
		if (response.equals("s_117"))
		{
			
			if (slicer_01_mos_taike_condition__defaultCondition (player, npc))
			{
				
				string_id message = new string_id (c_stringFile, "s_119");
				int numberOfResponses = 0;
				
				boolean hasResponse = false;
				
				boolean hasResponse0 = false;
				if (slicer_01_mos_taike_condition__defaultCondition (player, npc))
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
						responses[responseIndex++] = new string_id (c_stringFile, "s_121");
					}
					
					utils.setScriptVar (player, "conversation.slicer_01_mos_taike.branchId", 41);
					
					npcSpeak (player, message);
					npcSetConversationResponses (player, responses);
				}
				else
				{
					utils.removeScriptVar (player, "conversation.slicer_01_mos_taike.branchId");
					
					npcEndConversationWithMessage (player, message);
				}
				
				return SCRIPT_CONTINUE;
			}
			
		}
		
		return SCRIPT_DEFAULT;
	}
	
	
	public int slicer_01_mos_taike_handleBranch41(obj_id player, obj_id npc, string_id response) throws InterruptedException
	{
		
		if (response.equals("s_121"))
		{
			
			if (slicer_01_mos_taike_condition__defaultCondition (player, npc))
			{
				doAnimationAction (npc, "nod");
				
				slicer_01_mos_taike_action_startCommunication (player, npc);
				
				string_id message = new string_id (c_stringFile, "s_123");
				int numberOfResponses = 0;
				
				boolean hasResponse = false;
				
				boolean hasResponse0 = false;
				if (slicer_01_mos_taike_condition__defaultCondition (player, npc))
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
						responses[responseIndex++] = new string_id (c_stringFile, "s_125");
					}
					
					utils.setScriptVar (player, "conversation.slicer_01_mos_taike.branchId", 42);
					
					npcSpeak (player, message);
					npcSetConversationResponses (player, responses);
				}
				else
				{
					utils.removeScriptVar (player, "conversation.slicer_01_mos_taike.branchId");
					
					npcEndConversationWithMessage (player, message);
				}
				
				return SCRIPT_CONTINUE;
			}
			
		}
		
		return SCRIPT_DEFAULT;
	}
	
	
	public int slicer_01_mos_taike_handleBranch42(obj_id player, obj_id npc, string_id response) throws InterruptedException
	{
		
		if (response.equals("s_125"))
		{
			
			if (slicer_01_mos_taike_condition__defaultCondition (player, npc))
			{
				doAnimationAction (npc, "pound_fist_palm");
				
				string_id message = new string_id (c_stringFile, "s_128");
				int numberOfResponses = 0;
				
				boolean hasResponse = false;
				
				boolean hasResponse0 = false;
				if (slicer_01_mos_taike_condition__defaultCondition (player, npc))
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
					
					utils.setScriptVar (player, "conversation.slicer_01_mos_taike.branchId", 43);
					
					npcSpeak (player, message);
					npcSetConversationResponses (player, responses);
				}
				else
				{
					utils.removeScriptVar (player, "conversation.slicer_01_mos_taike.branchId");
					
					npcEndConversationWithMessage (player, message);
				}
				
				return SCRIPT_CONTINUE;
			}
			
		}
		
		return SCRIPT_DEFAULT;
	}
	
	
	public int slicer_01_mos_taike_handleBranch43(obj_id player, obj_id npc, string_id response) throws InterruptedException
	{
		
		if (response.equals("s_159"))
		{
			
			if (slicer_01_mos_taike_condition__defaultCondition (player, npc))
			{
				
				string_id message = new string_id (c_stringFile, "s_161");
				int numberOfResponses = 0;
				
				boolean hasResponse = false;
				
				boolean hasResponse0 = false;
				if (slicer_01_mos_taike_condition__defaultCondition (player, npc))
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
						responses[responseIndex++] = new string_id (c_stringFile, "s_163");
					}
					
					utils.setScriptVar (player, "conversation.slicer_01_mos_taike.branchId", 44);
					
					npcSpeak (player, message);
					npcSetConversationResponses (player, responses);
				}
				else
				{
					utils.removeScriptVar (player, "conversation.slicer_01_mos_taike.branchId");
					
					npcEndConversationWithMessage (player, message);
				}
				
				return SCRIPT_CONTINUE;
			}
			
		}
		
		return SCRIPT_DEFAULT;
	}
	
	
	public int slicer_01_mos_taike_handleBranch44(obj_id player, obj_id npc, string_id response) throws InterruptedException
	{
		
		if (response.equals("s_163"))
		{
			
			if (slicer_01_mos_taike_condition__defaultCondition (player, npc))
			{
				doAnimationAction (npc, "scared");
				
				string_id message = new string_id (c_stringFile, "s_165");
				int numberOfResponses = 0;
				
				boolean hasResponse = false;
				
				boolean hasResponse0 = false;
				if (slicer_01_mos_taike_condition__defaultCondition (player, npc))
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
						responses[responseIndex++] = new string_id (c_stringFile, "s_167");
					}
					
					utils.setScriptVar (player, "conversation.slicer_01_mos_taike.branchId", 45);
					
					npcSpeak (player, message);
					npcSetConversationResponses (player, responses);
				}
				else
				{
					utils.removeScriptVar (player, "conversation.slicer_01_mos_taike.branchId");
					
					npcEndConversationWithMessage (player, message);
				}
				
				return SCRIPT_CONTINUE;
			}
			
		}
		
		return SCRIPT_DEFAULT;
	}
	
	
	public int slicer_01_mos_taike_handleBranch45(obj_id player, obj_id npc, string_id response) throws InterruptedException
	{
		
		if (response.equals("s_167"))
		{
			
			if (slicer_01_mos_taike_condition__defaultCondition (player, npc))
			{
				doAnimationAction (npc, "handshake_tandem");
				
				doAnimationAction (player, "handshake_tandem");
				
				slicer_01_mos_taike_action_sendSignalSpokeCale (player, npc);
				
				string_id message = new string_id (c_stringFile, "s_169");
				utils.removeScriptVar (player, "conversation.slicer_01_mos_taike.branchId");
				
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
			detachScript(self, "conversation.slicer_01_mos_taike");
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
		detachScript (self, "conversation.slicer_01_mos_taike");
		
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
		
		if (slicer_01_mos_taike_condition_hasCompletedProlog (player, npc))
		{
			
			string_id message = new string_id (c_stringFile, "s_134");
			chat.chat (npc, player, message);
			
			return SCRIPT_CONTINUE;
		}
		
		if (slicer_01_mos_taike_condition_hasCompletedDathomir (player, npc))
		{
			
			string_id message = new string_id (c_stringFile, "s_127");
			int numberOfResponses = 0;
			
			boolean hasResponse = false;
			
			boolean hasResponse0 = false;
			if (slicer_01_mos_taike_condition__defaultCondition (player, npc))
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
					responses[responseIndex++] = new string_id (c_stringFile, "s_129");
				}
				
				utils.setScriptVar (player, "conversation.slicer_01_mos_taike.branchId", 2);
				
				npcStartConversation (player, npc, "slicer_01_mos_taike", message, responses);
			}
			else
			{
				chat.chat (npc, player, message);
			}
			
			return SCRIPT_CONTINUE;
		}
		
		if (slicer_01_mos_taike_condition_canceledQuest06 (player, npc))
		{
			slicer_01_mos_taike_action_grantQuest06 (player, npc);
			
			string_id message = new string_id (c_stringFile, "s_146");
			chat.chat (npc, player, message);
			
			return SCRIPT_CONTINUE;
		}
		
		if (slicer_01_mos_taike_condition_shouldBeOnLok (player, npc))
		{
			
			string_id message = new string_id (c_stringFile, "s_30");
			chat.chat (npc, player, message);
			
			return SCRIPT_CONTINUE;
		}
		
		if (slicer_01_mos_taike_condition_hasCompletedValarian (player, npc))
		{
			
			string_id message = new string_id (c_stringFile, "s_70");
			int numberOfResponses = 0;
			
			boolean hasResponse = false;
			
			boolean hasResponse0 = false;
			if (slicer_01_mos_taike_condition_failedToDefeatAssassin (player, npc))
			{
				++numberOfResponses;
				hasResponse = true;
				hasResponse0 = true;
			}
			
			boolean hasResponse1 = false;
			if (slicer_01_mos_taike_condition_successDefeatAssassin (player, npc))
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
					responses[responseIndex++] = new string_id (c_stringFile, "s_71");
				}
				
				if (hasResponse1)
				{
					responses[responseIndex++] = new string_id (c_stringFile, "s_72");
				}
				
				utils.setScriptVar (player, "conversation.slicer_01_mos_taike.branchId", 9);
				
				npcStartConversation (player, npc, "slicer_01_mos_taike", message, responses);
			}
			else
			{
				chat.chat (npc, player, message);
			}
			
			return SCRIPT_CONTINUE;
		}
		
		if (slicer_01_mos_taike_condition_shouldBeLadyValerian (player, npc))
		{
			
			string_id message = new string_id (c_stringFile, "s_48");
			chat.chat (npc, player, message);
			
			return SCRIPT_CONTINUE;
		}
		
		if (slicer_01_mos_taike_condition_completedQuest04Deleted05 (player, npc))
		{
			slicer_01_mos_taike_action_grantFrameValarainQuest (player, npc);
			
			string_id message = new string_id (c_stringFile, "s_149");
			chat.chat (npc, player, message);
			
			return SCRIPT_CONTINUE;
		}
		
		if (slicer_01_mos_taike_condition_hasCompletedInitQuest (player, npc))
		{
			
			string_id message = new string_id (c_stringFile, "s_54");
			int numberOfResponses = 0;
			
			boolean hasResponse = false;
			
			boolean hasResponse0 = false;
			if (slicer_01_mos_taike_condition__defaultCondition (player, npc))
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
				
				utils.setScriptVar (player, "conversation.slicer_01_mos_taike.branchId", 25);
				
				npcStartConversation (player, npc, "slicer_01_mos_taike", message, responses);
			}
			else
			{
				chat.chat (npc, player, message);
			}
			
			return SCRIPT_CONTINUE;
		}
		
		if (slicer_01_mos_taike_condition_isCheckingWithBib (player, npc))
		{
			
			string_id message = new string_id (c_stringFile, "s_147");
			chat.chat (npc, player, message);
			
			return SCRIPT_CONTINUE;
		}
		
		if (slicer_01_mos_taike_condition_hasInitialQuest (player, npc))
		{
			doAnimationAction (npc, "wave_finger_warning");
			
			string_id message = new string_id (c_stringFile, "s_107");
			int numberOfResponses = 0;
			
			boolean hasResponse = false;
			
			boolean hasResponse0 = false;
			if (slicer_01_mos_taike_condition__defaultCondition (player, npc))
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
				
				utils.setScriptVar (player, "conversation.slicer_01_mos_taike.branchId", 38);
				
				npcStartConversation (player, npc, "slicer_01_mos_taike", message, responses);
			}
			else
			{
				chat.chat (npc, player, message);
			}
			
			return SCRIPT_CONTINUE;
		}
		
		if (slicer_01_mos_taike_condition__defaultCondition (player, npc))
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
		if (!conversationId.equals("slicer_01_mos_taike"))
		{
			return SCRIPT_CONTINUE;
		}
		
		obj_id npc = self;
		
		int branchId = utils.getIntScriptVar (player, "conversation.slicer_01_mos_taike.branchId");
		
		if (branchId == 2 && slicer_01_mos_taike_handleBranch2 (player, npc, response) == SCRIPT_CONTINUE)
		{
			return SCRIPT_CONTINUE;
		}
		
		if (branchId == 3 && slicer_01_mos_taike_handleBranch3 (player, npc, response) == SCRIPT_CONTINUE)
		{
			return SCRIPT_CONTINUE;
		}
		
		if (branchId == 4 && slicer_01_mos_taike_handleBranch4 (player, npc, response) == SCRIPT_CONTINUE)
		{
			return SCRIPT_CONTINUE;
		}
		
		if (branchId == 6 && slicer_01_mos_taike_handleBranch6 (player, npc, response) == SCRIPT_CONTINUE)
		{
			return SCRIPT_CONTINUE;
		}
		
		if (branchId == 9 && slicer_01_mos_taike_handleBranch9 (player, npc, response) == SCRIPT_CONTINUE)
		{
			return SCRIPT_CONTINUE;
		}
		
		if (branchId == 10 && slicer_01_mos_taike_handleBranch10 (player, npc, response) == SCRIPT_CONTINUE)
		{
			return SCRIPT_CONTINUE;
		}
		
		if (branchId == 11 && slicer_01_mos_taike_handleBranch11 (player, npc, response) == SCRIPT_CONTINUE)
		{
			return SCRIPT_CONTINUE;
		}
		
		if (branchId == 12 && slicer_01_mos_taike_handleBranch12 (player, npc, response) == SCRIPT_CONTINUE)
		{
			return SCRIPT_CONTINUE;
		}
		
		if (branchId == 13 && slicer_01_mos_taike_handleBranch13 (player, npc, response) == SCRIPT_CONTINUE)
		{
			return SCRIPT_CONTINUE;
		}
		
		if (branchId == 14 && slicer_01_mos_taike_handleBranch14 (player, npc, response) == SCRIPT_CONTINUE)
		{
			return SCRIPT_CONTINUE;
		}
		
		if (branchId == 15 && slicer_01_mos_taike_handleBranch15 (player, npc, response) == SCRIPT_CONTINUE)
		{
			return SCRIPT_CONTINUE;
		}
		
		if (branchId == 17 && slicer_01_mos_taike_handleBranch17 (player, npc, response) == SCRIPT_CONTINUE)
		{
			return SCRIPT_CONTINUE;
		}
		
		if (branchId == 18 && slicer_01_mos_taike_handleBranch18 (player, npc, response) == SCRIPT_CONTINUE)
		{
			return SCRIPT_CONTINUE;
		}
		
		if (branchId == 19 && slicer_01_mos_taike_handleBranch19 (player, npc, response) == SCRIPT_CONTINUE)
		{
			return SCRIPT_CONTINUE;
		}
		
		if (branchId == 20 && slicer_01_mos_taike_handleBranch20 (player, npc, response) == SCRIPT_CONTINUE)
		{
			return SCRIPT_CONTINUE;
		}
		
		if (branchId == 21 && slicer_01_mos_taike_handleBranch21 (player, npc, response) == SCRIPT_CONTINUE)
		{
			return SCRIPT_CONTINUE;
		}
		
		if (branchId == 22 && slicer_01_mos_taike_handleBranch22 (player, npc, response) == SCRIPT_CONTINUE)
		{
			return SCRIPT_CONTINUE;
		}
		
		if (branchId == 25 && slicer_01_mos_taike_handleBranch25 (player, npc, response) == SCRIPT_CONTINUE)
		{
			return SCRIPT_CONTINUE;
		}
		
		if (branchId == 26 && slicer_01_mos_taike_handleBranch26 (player, npc, response) == SCRIPT_CONTINUE)
		{
			return SCRIPT_CONTINUE;
		}
		
		if (branchId == 27 && slicer_01_mos_taike_handleBranch27 (player, npc, response) == SCRIPT_CONTINUE)
		{
			return SCRIPT_CONTINUE;
		}
		
		if (branchId == 28 && slicer_01_mos_taike_handleBranch28 (player, npc, response) == SCRIPT_CONTINUE)
		{
			return SCRIPT_CONTINUE;
		}
		
		if (branchId == 29 && slicer_01_mos_taike_handleBranch29 (player, npc, response) == SCRIPT_CONTINUE)
		{
			return SCRIPT_CONTINUE;
		}
		
		if (branchId == 30 && slicer_01_mos_taike_handleBranch30 (player, npc, response) == SCRIPT_CONTINUE)
		{
			return SCRIPT_CONTINUE;
		}
		
		if (branchId == 31 && slicer_01_mos_taike_handleBranch31 (player, npc, response) == SCRIPT_CONTINUE)
		{
			return SCRIPT_CONTINUE;
		}
		
		if (branchId == 32 && slicer_01_mos_taike_handleBranch32 (player, npc, response) == SCRIPT_CONTINUE)
		{
			return SCRIPT_CONTINUE;
		}
		
		if (branchId == 33 && slicer_01_mos_taike_handleBranch33 (player, npc, response) == SCRIPT_CONTINUE)
		{
			return SCRIPT_CONTINUE;
		}
		
		if (branchId == 34 && slicer_01_mos_taike_handleBranch34 (player, npc, response) == SCRIPT_CONTINUE)
		{
			return SCRIPT_CONTINUE;
		}
		
		if (branchId == 35 && slicer_01_mos_taike_handleBranch35 (player, npc, response) == SCRIPT_CONTINUE)
		{
			return SCRIPT_CONTINUE;
		}
		
		if (branchId == 38 && slicer_01_mos_taike_handleBranch38 (player, npc, response) == SCRIPT_CONTINUE)
		{
			return SCRIPT_CONTINUE;
		}
		
		if (branchId == 39 && slicer_01_mos_taike_handleBranch39 (player, npc, response) == SCRIPT_CONTINUE)
		{
			return SCRIPT_CONTINUE;
		}
		
		if (branchId == 40 && slicer_01_mos_taike_handleBranch40 (player, npc, response) == SCRIPT_CONTINUE)
		{
			return SCRIPT_CONTINUE;
		}
		
		if (branchId == 41 && slicer_01_mos_taike_handleBranch41 (player, npc, response) == SCRIPT_CONTINUE)
		{
			return SCRIPT_CONTINUE;
		}
		
		if (branchId == 42 && slicer_01_mos_taike_handleBranch42 (player, npc, response) == SCRIPT_CONTINUE)
		{
			return SCRIPT_CONTINUE;
		}
		
		if (branchId == 43 && slicer_01_mos_taike_handleBranch43 (player, npc, response) == SCRIPT_CONTINUE)
		{
			return SCRIPT_CONTINUE;
		}
		
		if (branchId == 44 && slicer_01_mos_taike_handleBranch44 (player, npc, response) == SCRIPT_CONTINUE)
		{
			return SCRIPT_CONTINUE;
		}
		
		if (branchId == 45 && slicer_01_mos_taike_handleBranch45 (player, npc, response) == SCRIPT_CONTINUE)
		{
			return SCRIPT_CONTINUE;
		}
		
		chat.chat (npc, "Error: Fell through all branches and responses for OnNpcConversationResponse.");
		
		utils.removeScriptVar (player, "conversation.slicer_01_mos_taike.branchId");
		
		return SCRIPT_CONTINUE;
	}
	
}
