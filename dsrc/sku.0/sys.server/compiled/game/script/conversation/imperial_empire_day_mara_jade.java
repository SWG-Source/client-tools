package script.conversation;

import script.*;
import script.base_class.*;
import script.combat_engine.*;
import java.util.Arrays;
import java.util.Hashtable;
import java.util.Vector;
import script.base_script;


import script.library.ai_lib;
import script.library.badge;
import script.library.buff;
import script.library.chat;
import script.library.collection;
import script.library.conversation;
import script.library.create;
import script.library.factions;
import script.library.groundquests;
import script.library.holiday;
import script.library.static_item;
import script.library.utils;


public class imperial_empire_day_mara_jade extends script.base_script
{
	public imperial_empire_day_mara_jade()
	{
	}
	String c_stringFile = "conversation/imperial_empire_day_mara_jade";
	
	
	public boolean imperial_empire_day_mara_jade_condition__defaultCondition(obj_id player, obj_id npc) throws InterruptedException
	{
		return true;
	}
	
	
	public boolean imperial_empire_day_mara_jade_condition_isBelow60thLevelOrTraderEntertainer(obj_id player, obj_id npc) throws InterruptedException
	{
		return (imperial_empire_day_mara_jade_condition_isBelow60thLevel(player, npc) || imperial_empire_day_mara_jade_condition_isEntertainer(player, npc) || imperial_empire_day_mara_jade_condition_isTrader(player, npc));
	}
	
	
	public boolean imperial_empire_day_mara_jade_condition_isEntertainer(obj_id player, obj_id npc) throws InterruptedException
	{
		faceTo(npc, player);
		return utils.isProfession(player, utils.ENTERTAINER);
	}
	
	
	public boolean imperial_empire_day_mara_jade_condition_isTrader(obj_id player, obj_id npc) throws InterruptedException
	{
		faceTo(npc, player);
		return utils.isProfession(player, utils.TRADER);
	}
	
	
	public boolean imperial_empire_day_mara_jade_condition_isBelow60thLevel(obj_id player, obj_id npc) throws InterruptedException
	{
		return (60 > getLevel(player));
	}
	
	
	public boolean imperial_empire_day_mara_jade_condition_hasNotCompletedMission1(obj_id player, obj_id npc) throws InterruptedException
	{
		faceTo(npc, player);
		return groundquests.isQuestActive(player, holiday.EMPIREDAYQUEST_IMP_COMBAT_01) && !groundquests.hasCompletedQuest(player, holiday.EMPIREDAYQUEST_IMP_COMBAT_01);
	}
	
	
	public boolean imperial_empire_day_mara_jade_condition_hasAnIncompleteCombatMission(obj_id player, obj_id npc) throws InterruptedException
	{
		return (!imperial_empire_day_mara_jade_condition_completed7thQuest(player, npc) && (imperial_empire_day_mara_jade_condition_hasCompleted1stMissionNot2nd(player, npc) || imperial_empire_day_mara_jade_condition_hasCompleted2ndMissionNot3rd(player, npc) || imperial_empire_day_mara_jade_condition_hasCompleted3rdMissionNot4th(player, npc) || imperial_empire_day_mara_jade_condition_hasCompleted4thMissionNot5th(player, npc) || imperial_empire_day_mara_jade_condition_hasCompleted5thMissionNot6th(player, npc) || imperial_empire_day_mara_jade_condition_hasCompleted6thMissionNot7th(player, npc)));
	}
	
	
	public boolean imperial_empire_day_mara_jade_condition_isOnSecondMission(obj_id player, obj_id npc) throws InterruptedException
	{
		return (groundquests.isQuestActive(player, holiday.EMPIREDAYQUEST_IMP_COMBAT_02) && !groundquests.isQuestActiveOrComplete(player, holiday.EMPIREDAYQUEST_IMP_COMBAT_03));
	}
	
	
	public boolean imperial_empire_day_mara_jade_condition_isChampion(obj_id player, obj_id npc) throws InterruptedException
	{
		faceTo(npc, player);
		return hasCompletedCollectionSlot(player, holiday.REMEMBRANCE_DAY_CHAMPION_BADGE);
	}
	
	
	public boolean imperial_empire_day_mara_jade_condition_completed7thQuest(obj_id player, obj_id npc) throws InterruptedException
	{
		return groundquests.hasCompletedQuest(player, holiday.EMPIREDAYQUEST_IMP_COMBAT_07);
	}
	
	
	public boolean imperial_empire_day_mara_jade_condition_completed7thQuestNoAutograph(obj_id player, obj_id npc) throws InterruptedException
	{
		return imperial_empire_day_mara_jade_condition_completed7thQuest(player, npc) && !groundquests.isQuestActiveOrComplete(player, holiday.EMPIREDAYQUEST_IMP_COMBAT_NUMBER);
	}
	
	
	public boolean imperial_empire_day_mara_jade_condition_isRebelPlayer(obj_id player, obj_id npc) throws InterruptedException
	{
		faceTo(npc, player);
		return (factions.isRebel(player));
	}
	
	
	public boolean imperial_empire_day_mara_jade_condition_isNeutralPlayer(obj_id player, obj_id npc) throws InterruptedException
	{
		faceTo(npc, player);
		return(!factions.isImperial(player) && !factions.isRebel(player));
	}
	
	
	public boolean imperial_empire_day_mara_jade_condition_completed7thQuestHasAutograph(obj_id player, obj_id npc) throws InterruptedException
	{
		return imperial_empire_day_mara_jade_condition_completed7thQuest(player, npc) && groundquests.isQuestActiveOrComplete(player, holiday.EMPIREDAYQUEST_IMP_COMBAT_NUMBER);
	}
	
	
	public boolean imperial_empire_day_mara_jade_condition_hasCompleted7thMission(obj_id player, obj_id npc) throws InterruptedException
	{
		faceTo(npc, player);
		return (groundquests.hasCompletedQuest(player, holiday.EMPIREDAYQUEST_IMP_COMBAT_07));
	}
	
	
	public boolean imperial_empire_day_mara_jade_condition_hasCompleted1stMissionNot2nd(obj_id player, obj_id npc) throws InterruptedException
	{
		faceTo(npc, player);
		return (groundquests.hasCompletedQuest(player, holiday.EMPIREDAYQUEST_IMP_COMBAT_01) && !groundquests.hasCompletedQuest(player, holiday.EMPIREDAYQUEST_IMP_COMBAT_02));
	}
	
	
	public boolean imperial_empire_day_mara_jade_condition_hasCompleted2ndMissionNot3rd(obj_id player, obj_id npc) throws InterruptedException
	{
		faceTo(npc, player);
		return (groundquests.hasCompletedQuest(player, holiday.EMPIREDAYQUEST_IMP_COMBAT_02) && !groundquests.hasCompletedQuest(player, holiday.EMPIREDAYQUEST_IMP_COMBAT_03));
	}
	
	
	public boolean imperial_empire_day_mara_jade_condition_hasCompleted3rdMissionNot4th(obj_id player, obj_id npc) throws InterruptedException
	{
		faceTo(npc, player);
		return (groundquests.hasCompletedQuest(player, holiday.EMPIREDAYQUEST_IMP_COMBAT_03) && !groundquests.hasCompletedQuest(player, holiday.EMPIREDAYQUEST_IMP_COMBAT_04));
	}
	
	
	public boolean imperial_empire_day_mara_jade_condition_hasCompleted4thMissionNot5th(obj_id player, obj_id npc) throws InterruptedException
	{
		faceTo(npc, player);
		return (groundquests.hasCompletedQuest(player, holiday.EMPIREDAYQUEST_IMP_COMBAT_04) && !groundquests.hasCompletedQuest(player, holiday.EMPIREDAYQUEST_IMP_COMBAT_05));
	}
	
	
	public boolean imperial_empire_day_mara_jade_condition_hasCompleted5thMissionNot6th(obj_id player, obj_id npc) throws InterruptedException
	{
		faceTo(npc, player);
		return (groundquests.hasCompletedQuest(player, holiday.EMPIREDAYQUEST_IMP_COMBAT_05) && !groundquests.hasCompletedQuest(player, holiday.EMPIREDAYQUEST_IMP_COMBAT_06));
	}
	
	
	public boolean imperial_empire_day_mara_jade_condition_hasCompleted6thMissionNot7th(obj_id player, obj_id npc) throws InterruptedException
	{
		faceTo(npc, player);
		return groundquests.hasCompletedQuest(player, holiday.EMPIREDAYQUEST_IMP_COMBAT_06) && !groundquests.hasCompletedQuest(player, holiday.EMPIREDAYQUEST_IMP_COMBAT_07);
	}
	
	
	public void imperial_empire_day_mara_jade_action_grantMissionOne(obj_id player, obj_id npc) throws InterruptedException
	{
		groundquests.grantQuest(player, holiday.EMPIREDAYQUEST_IMP_COMBAT_01);
	}
	
	
	public void imperial_empire_day_mara_jade_action_grantMissionTwo(obj_id player, obj_id npc) throws InterruptedException
	{
		if (getCollectionSlotValue(player, holiday.IMPERIAL_RESCUE_START_SLOT) <= 0)
		{
			modifyCollectionSlotValue(player, holiday.IMPERIAL_RESCUE_START_SLOT, 1);
		}
		
		groundquests.grantQuest(player, holiday.EMPIREDAYQUEST_IMP_COMBAT_02);
	}
	
	
	public void imperial_empire_day_mara_jade_action_revokeMission2Regrant(obj_id player, obj_id npc) throws InterruptedException
	{
		if (groundquests.isQuestActive(player, holiday.EMPIREDAYQUEST_IMP_COMBAT_02))
		{
			groundquests.clearQuest(player, holiday.EMPIREDAYQUEST_IMP_COMBAT_02);
		}
		
		groundquests.grantQuest(player, holiday.EMPIREDAYQUEST_IMP_COMBAT_02);
	}
	
	
	public void imperial_empire_day_mara_jade_action_revokeMission3Regrant(obj_id player, obj_id npc) throws InterruptedException
	{
		if (groundquests.isQuestActive(player, holiday.EMPIREDAYQUEST_IMP_COMBAT_03))
		{
			groundquests.clearQuest(player, holiday.EMPIREDAYQUEST_IMP_COMBAT_03);
		}
		
		groundquests.grantQuest(player, holiday.EMPIREDAYQUEST_IMP_COMBAT_03);
	}
	
	
	public void imperial_empire_day_mara_jade_action_revokeMission4Regrant(obj_id player, obj_id npc) throws InterruptedException
	{
		if (groundquests.isQuestActive(player, holiday.EMPIREDAYQUEST_IMP_COMBAT_04))
		{
			groundquests.clearQuest(player, holiday.EMPIREDAYQUEST_IMP_COMBAT_04);
		}
		
		groundquests.grantQuest(player, holiday.EMPIREDAYQUEST_IMP_COMBAT_04);
	}
	
	
	public void imperial_empire_day_mara_jade_action_spawnTroopers(obj_id player, obj_id npc) throws InterruptedException
	{
		
		for (int i = 0; i < 3; i++)
		{
			testAbortScript();
			location guardLocation = groundquests.getRandom2DLocationAroundLocation(npc, 1, 1, 5, 12);
			int mobLevel = getLevel(player);
			obj_id guard = create.object("rebel_emperorsday_ceremony_sentry", guardLocation, mobLevel);
			attachScript(guard, "event.emp_day.factional_guard_self_destruct");
			startCombat(guard, player);
		}
	}
	
	
	public void imperial_empire_day_mara_jade_action_revokeMission5Regrant(obj_id player, obj_id npc) throws InterruptedException
	{
		if (groundquests.isQuestActive(player, holiday.EMPIREDAYQUEST_IMP_COMBAT_05))
		{
			groundquests.clearQuest(player, holiday.EMPIREDAYQUEST_IMP_COMBAT_05);
		}
		
		groundquests.grantQuest(player, holiday.EMPIREDAYQUEST_IMP_COMBAT_05);
	}
	
	
	public void imperial_empire_day_mara_jade_action_revokeMission6Regrant(obj_id player, obj_id npc) throws InterruptedException
	{
		if (groundquests.isQuestActive(player, holiday.EMPIREDAYQUEST_IMP_COMBAT_06))
		{
			groundquests.clearQuest(player, holiday.EMPIREDAYQUEST_IMP_COMBAT_06);
		}
		
		groundquests.grantQuest(player, holiday.EMPIREDAYQUEST_IMP_COMBAT_06);
	}
	
	
	public void imperial_empire_day_mara_jade_action_revokeMission7Regrant(obj_id player, obj_id npc) throws InterruptedException
	{
		if (groundquests.isQuestActive(player, holiday.EMPIREDAYQUEST_IMP_COMBAT_07))
		{
			groundquests.clearQuest(player, holiday.EMPIREDAYQUEST_IMP_COMBAT_07);
		}
		
		groundquests.grantQuest(player, holiday.EMPIREDAYQUEST_IMP_COMBAT_07);
	}
	
	
	public void imperial_empire_day_mara_jade_action_grantAutograph(obj_id player, obj_id npc) throws InterruptedException
	{
		groundquests.grantQuest(player, holiday.EMPIREDAYQUEST_IMP_COMBAT_NUMBER);
	}
	
	
	public void imperial_empire_day_mara_jade_action_giveWaypoint(obj_id player, obj_id npc) throws InterruptedException
	{
		location loc = new location(4000, 37, -6168, "corellia");
		obj_id wpt = createWaypointInDatapad(player, loc);
		setWaypointName(wpt, "Detention Facility");
		setWaypointActive(wpt, true);
	}
	
	
	public String imperial_empire_day_mara_jade_tokenTO_timeLeftRecruitment(obj_id player, obj_id npc) throws InterruptedException
	{
		String returnString = holiday.getTimeRemainingBeforeLockoutRemoved(player, holiday.EMPIRE_DAY_RECRUITMENT_TIMESTAMP);
		if (returnString == null || returnString.equals(""))
		{
			return "might have to contact Customer Service. I can't seem to get any time data off your character.";
		}
		return returnString;
	}
	
	
	public String imperial_empire_day_mara_jade_tokenTO_timeLeftPropaganda(obj_id player, obj_id npc) throws InterruptedException
	{
		String returnString = holiday.getTimeRemainingBeforeLockoutRemoved(player, holiday.EMPIRE_DAY_PROPAGANDA_TIMESTAMP);
		if (returnString == null || returnString.equals(""))
		{
			return "might have to contact Customer Service. I can't seem to get any time data off your character.";
		}
		return returnString;
	}
	
	
	public int imperial_empire_day_mara_jade_tokenDI_notUsed(obj_id player, obj_id npc) throws InterruptedException
	{
		return 0;
	}
	
	
	public int imperial_empire_day_mara_jade_handleBranch3(obj_id player, obj_id npc, string_id response) throws InterruptedException
	{
		
		if (response.equals("s_83"))
		{
			
			if (imperial_empire_day_mara_jade_condition__defaultCondition (player, npc))
			{
				imperial_empire_day_mara_jade_action_giveWaypoint (player, npc);
				
				string_id message = new string_id (c_stringFile, "s_85");
				utils.removeScriptVar (player, "conversation.imperial_empire_day_mara_jade.branchId");
				
				npcEndConversationWithMessage (player, message);
				
				return SCRIPT_CONTINUE;
			}
			
		}
		
		if (response.equals("s_84"))
		{
			
			if (imperial_empire_day_mara_jade_condition__defaultCondition (player, npc))
			{
				
				string_id message = new string_id (c_stringFile, "s_87");
				int numberOfResponses = 0;
				
				boolean hasResponse = false;
				
				boolean hasResponse0 = false;
				if (imperial_empire_day_mara_jade_condition_hasCompleted1stMissionNot2nd (player, npc))
				{
					++numberOfResponses;
					hasResponse = true;
					hasResponse0 = true;
				}
				
				boolean hasResponse1 = false;
				if (imperial_empire_day_mara_jade_condition_hasCompleted2ndMissionNot3rd (player, npc))
				{
					++numberOfResponses;
					hasResponse = true;
					hasResponse1 = true;
				}
				
				boolean hasResponse2 = false;
				if (imperial_empire_day_mara_jade_condition_hasCompleted3rdMissionNot4th (player, npc))
				{
					++numberOfResponses;
					hasResponse = true;
					hasResponse2 = true;
				}
				
				boolean hasResponse3 = false;
				if (imperial_empire_day_mara_jade_condition_hasCompleted4thMissionNot5th (player, npc))
				{
					++numberOfResponses;
					hasResponse = true;
					hasResponse3 = true;
				}
				
				boolean hasResponse4 = false;
				if (imperial_empire_day_mara_jade_condition_hasCompleted5thMissionNot6th (player, npc))
				{
					++numberOfResponses;
					hasResponse = true;
					hasResponse4 = true;
				}
				
				boolean hasResponse5 = false;
				if (imperial_empire_day_mara_jade_condition_hasCompleted6thMissionNot7th (player, npc))
				{
					++numberOfResponses;
					hasResponse = true;
					hasResponse5 = true;
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
						responses[responseIndex++] = new string_id (c_stringFile, "s_93");
					}
					
					if (hasResponse2)
					{
						responses[responseIndex++] = new string_id (c_stringFile, "s_101");
					}
					
					if (hasResponse3)
					{
						responses[responseIndex++] = new string_id (c_stringFile, "s_112");
					}
					
					if (hasResponse4)
					{
						responses[responseIndex++] = new string_id (c_stringFile, "s_122");
					}
					
					if (hasResponse5)
					{
						responses[responseIndex++] = new string_id (c_stringFile, "s_140");
					}
					
					utils.setScriptVar (player, "conversation.imperial_empire_day_mara_jade.branchId", 5);
					
					npcSpeak (player, message);
					npcSetConversationResponses (player, responses);
				}
				else
				{
					utils.removeScriptVar (player, "conversation.imperial_empire_day_mara_jade.branchId");
					
					npcEndConversationWithMessage (player, message);
				}
				
				return SCRIPT_CONTINUE;
			}
			
		}
		
		return SCRIPT_DEFAULT;
	}
	
	
	public int imperial_empire_day_mara_jade_handleBranch5(obj_id player, obj_id npc, string_id response) throws InterruptedException
	{
		
		if (response.equals("s_89"))
		{
			
			if (imperial_empire_day_mara_jade_condition__defaultCondition (player, npc))
			{
				
				string_id message = new string_id (c_stringFile, "s_99");
				int numberOfResponses = 0;
				
				boolean hasResponse = false;
				
				boolean hasResponse0 = false;
				if (imperial_empire_day_mara_jade_condition__defaultCondition (player, npc))
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
						responses[responseIndex++] = new string_id (c_stringFile, "s_100");
					}
					
					utils.setScriptVar (player, "conversation.imperial_empire_day_mara_jade.branchId", 6);
					
					npcSpeak (player, message);
					npcSetConversationResponses (player, responses);
				}
				else
				{
					utils.removeScriptVar (player, "conversation.imperial_empire_day_mara_jade.branchId");
					
					npcEndConversationWithMessage (player, message);
				}
				
				return SCRIPT_CONTINUE;
			}
			
		}
		
		if (response.equals("s_93"))
		{
			
			if (imperial_empire_day_mara_jade_condition__defaultCondition (player, npc))
			{
				
				string_id message = new string_id (c_stringFile, "s_103");
				int numberOfResponses = 0;
				
				boolean hasResponse = false;
				
				boolean hasResponse0 = false;
				if (imperial_empire_day_mara_jade_condition__defaultCondition (player, npc))
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
					
					utils.setScriptVar (player, "conversation.imperial_empire_day_mara_jade.branchId", 8);
					
					npcSpeak (player, message);
					npcSetConversationResponses (player, responses);
				}
				else
				{
					utils.removeScriptVar (player, "conversation.imperial_empire_day_mara_jade.branchId");
					
					npcEndConversationWithMessage (player, message);
				}
				
				return SCRIPT_CONTINUE;
			}
			
		}
		
		if (response.equals("s_101"))
		{
			
			if (imperial_empire_day_mara_jade_condition__defaultCondition (player, npc))
			{
				
				string_id message = new string_id (c_stringFile, "s_107");
				int numberOfResponses = 0;
				
				boolean hasResponse = false;
				
				boolean hasResponse0 = false;
				if (imperial_empire_day_mara_jade_condition__defaultCondition (player, npc))
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
					
					utils.setScriptVar (player, "conversation.imperial_empire_day_mara_jade.branchId", 10);
					
					npcSpeak (player, message);
					npcSetConversationResponses (player, responses);
				}
				else
				{
					utils.removeScriptVar (player, "conversation.imperial_empire_day_mara_jade.branchId");
					
					npcEndConversationWithMessage (player, message);
				}
				
				return SCRIPT_CONTINUE;
			}
			
		}
		
		if (response.equals("s_112"))
		{
			
			if (imperial_empire_day_mara_jade_condition__defaultCondition (player, npc))
			{
				
				string_id message = new string_id (c_stringFile, "s_115");
				int numberOfResponses = 0;
				
				boolean hasResponse = false;
				
				boolean hasResponse0 = false;
				if (imperial_empire_day_mara_jade_condition__defaultCondition (player, npc))
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
					
					utils.setScriptVar (player, "conversation.imperial_empire_day_mara_jade.branchId", 12);
					
					npcSpeak (player, message);
					npcSetConversationResponses (player, responses);
				}
				else
				{
					utils.removeScriptVar (player, "conversation.imperial_empire_day_mara_jade.branchId");
					
					npcEndConversationWithMessage (player, message);
				}
				
				return SCRIPT_CONTINUE;
			}
			
		}
		
		if (response.equals("s_122"))
		{
			
			if (imperial_empire_day_mara_jade_condition__defaultCondition (player, npc))
			{
				
				string_id message = new string_id (c_stringFile, "s_127");
				int numberOfResponses = 0;
				
				boolean hasResponse = false;
				
				boolean hasResponse0 = false;
				if (imperial_empire_day_mara_jade_condition__defaultCondition (player, npc))
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
					
					utils.setScriptVar (player, "conversation.imperial_empire_day_mara_jade.branchId", 14);
					
					npcSpeak (player, message);
					npcSetConversationResponses (player, responses);
				}
				else
				{
					utils.removeScriptVar (player, "conversation.imperial_empire_day_mara_jade.branchId");
					
					npcEndConversationWithMessage (player, message);
				}
				
				return SCRIPT_CONTINUE;
			}
			
		}
		
		if (response.equals("s_140"))
		{
			
			if (imperial_empire_day_mara_jade_condition__defaultCondition (player, npc))
			{
				
				string_id message = new string_id (c_stringFile, "s_145");
				int numberOfResponses = 0;
				
				boolean hasResponse = false;
				
				boolean hasResponse0 = false;
				if (imperial_empire_day_mara_jade_condition__defaultCondition (player, npc))
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
					
					utils.setScriptVar (player, "conversation.imperial_empire_day_mara_jade.branchId", 16);
					
					npcSpeak (player, message);
					npcSetConversationResponses (player, responses);
				}
				else
				{
					utils.removeScriptVar (player, "conversation.imperial_empire_day_mara_jade.branchId");
					
					npcEndConversationWithMessage (player, message);
				}
				
				return SCRIPT_CONTINUE;
			}
			
		}
		
		return SCRIPT_DEFAULT;
	}
	
	
	public int imperial_empire_day_mara_jade_handleBranch6(obj_id player, obj_id npc, string_id response) throws InterruptedException
	{
		
		if (response.equals("s_100"))
		{
			imperial_empire_day_mara_jade_action_revokeMission2Regrant (player, npc);
			
			if (imperial_empire_day_mara_jade_condition__defaultCondition (player, npc))
			{
				
				string_id message = new string_id (c_stringFile, "s_191");
				utils.removeScriptVar (player, "conversation.imperial_empire_day_mara_jade.branchId");
				
				npcEndConversationWithMessage (player, message);
				
				return SCRIPT_CONTINUE;
			}
			
		}
		
		return SCRIPT_DEFAULT;
	}
	
	
	public int imperial_empire_day_mara_jade_handleBranch8(obj_id player, obj_id npc, string_id response) throws InterruptedException
	{
		
		if (response.equals("s_104"))
		{
			imperial_empire_day_mara_jade_action_revokeMission3Regrant (player, npc);
			
			if (imperial_empire_day_mara_jade_condition__defaultCondition (player, npc))
			{
				
				string_id message = new string_id (c_stringFile, "s_106");
				int numberOfResponses = 0;
				
				boolean hasResponse = false;
				
				boolean hasResponse0 = false;
				if (imperial_empire_day_mara_jade_condition__defaultCondition (player, npc))
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
					
					utils.setScriptVar (player, "conversation.imperial_empire_day_mara_jade.branchId", 9);
					
					npcSpeak (player, message);
					npcSetConversationResponses (player, responses);
				}
				else
				{
					utils.removeScriptVar (player, "conversation.imperial_empire_day_mara_jade.branchId");
					
					npcEndConversationWithMessage (player, message);
				}
				
				return SCRIPT_CONTINUE;
			}
			
		}
		
		return SCRIPT_DEFAULT;
	}
	
	
	public int imperial_empire_day_mara_jade_handleBranch9(obj_id player, obj_id npc, string_id response) throws InterruptedException
	{
		
		if (response.equals("s_123"))
		{
			
			if (imperial_empire_day_mara_jade_condition__defaultCondition (player, npc))
			{
				imperial_empire_day_mara_jade_action_giveWaypoint (player, npc);
				
				string_id message = new string_id (c_stringFile, "s_85");
				utils.removeScriptVar (player, "conversation.imperial_empire_day_mara_jade.branchId");
				
				npcEndConversationWithMessage (player, message);
				
				return SCRIPT_CONTINUE;
			}
			
		}
		
		return SCRIPT_DEFAULT;
	}
	
	
	public int imperial_empire_day_mara_jade_handleBranch10(obj_id player, obj_id npc, string_id response) throws InterruptedException
	{
		
		if (response.equals("s_111"))
		{
			imperial_empire_day_mara_jade_action_revokeMission4Regrant (player, npc);
			
			if (imperial_empire_day_mara_jade_condition__defaultCondition (player, npc))
			{
				
				string_id message = new string_id (c_stringFile, "s_113");
				int numberOfResponses = 0;
				
				boolean hasResponse = false;
				
				boolean hasResponse0 = false;
				if (imperial_empire_day_mara_jade_condition__defaultCondition (player, npc))
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
					
					utils.setScriptVar (player, "conversation.imperial_empire_day_mara_jade.branchId", 11);
					
					npcSpeak (player, message);
					npcSetConversationResponses (player, responses);
				}
				else
				{
					utils.removeScriptVar (player, "conversation.imperial_empire_day_mara_jade.branchId");
					
					npcEndConversationWithMessage (player, message);
				}
				
				return SCRIPT_CONTINUE;
			}
			
		}
		
		return SCRIPT_DEFAULT;
	}
	
	
	public int imperial_empire_day_mara_jade_handleBranch11(obj_id player, obj_id npc, string_id response) throws InterruptedException
	{
		
		if (response.equals("s_124"))
		{
			
			if (imperial_empire_day_mara_jade_condition__defaultCondition (player, npc))
			{
				imperial_empire_day_mara_jade_action_giveWaypoint (player, npc);
				
				string_id message = new string_id (c_stringFile, "s_85");
				utils.removeScriptVar (player, "conversation.imperial_empire_day_mara_jade.branchId");
				
				npcEndConversationWithMessage (player, message);
				
				return SCRIPT_CONTINUE;
			}
			
		}
		
		return SCRIPT_DEFAULT;
	}
	
	
	public int imperial_empire_day_mara_jade_handleBranch12(obj_id player, obj_id npc, string_id response) throws InterruptedException
	{
		
		if (response.equals("s_117"))
		{
			imperial_empire_day_mara_jade_action_revokeMission5Regrant (player, npc);
			
			if (imperial_empire_day_mara_jade_condition__defaultCondition (player, npc))
			{
				
				string_id message = new string_id (c_stringFile, "s_119");
				int numberOfResponses = 0;
				
				boolean hasResponse = false;
				
				boolean hasResponse0 = false;
				if (imperial_empire_day_mara_jade_condition__defaultCondition (player, npc))
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
					
					utils.setScriptVar (player, "conversation.imperial_empire_day_mara_jade.branchId", 13);
					
					npcSpeak (player, message);
					npcSetConversationResponses (player, responses);
				}
				else
				{
					utils.removeScriptVar (player, "conversation.imperial_empire_day_mara_jade.branchId");
					
					npcEndConversationWithMessage (player, message);
				}
				
				return SCRIPT_CONTINUE;
			}
			
		}
		
		return SCRIPT_DEFAULT;
	}
	
	
	public int imperial_empire_day_mara_jade_handleBranch13(obj_id player, obj_id npc, string_id response) throws InterruptedException
	{
		
		if (response.equals("s_125"))
		{
			imperial_empire_day_mara_jade_action_revokeMission5Regrant (player, npc);
			
			if (imperial_empire_day_mara_jade_condition__defaultCondition (player, npc))
			{
				imperial_empire_day_mara_jade_action_giveWaypoint (player, npc);
				
				string_id message = new string_id (c_stringFile, "s_85");
				utils.removeScriptVar (player, "conversation.imperial_empire_day_mara_jade.branchId");
				
				npcEndConversationWithMessage (player, message);
				
				return SCRIPT_CONTINUE;
			}
			
		}
		
		return SCRIPT_DEFAULT;
	}
	
	
	public int imperial_empire_day_mara_jade_handleBranch14(obj_id player, obj_id npc, string_id response) throws InterruptedException
	{
		
		if (response.equals("s_129"))
		{
			imperial_empire_day_mara_jade_action_revokeMission6Regrant (player, npc);
			
			if (imperial_empire_day_mara_jade_condition__defaultCondition (player, npc))
			{
				
				string_id message = new string_id (c_stringFile, "s_131");
				int numberOfResponses = 0;
				
				boolean hasResponse = false;
				
				boolean hasResponse0 = false;
				if (imperial_empire_day_mara_jade_condition__defaultCondition (player, npc))
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
					
					utils.setScriptVar (player, "conversation.imperial_empire_day_mara_jade.branchId", 15);
					
					npcSpeak (player, message);
					npcSetConversationResponses (player, responses);
				}
				else
				{
					utils.removeScriptVar (player, "conversation.imperial_empire_day_mara_jade.branchId");
					
					npcEndConversationWithMessage (player, message);
				}
				
				return SCRIPT_CONTINUE;
			}
			
		}
		
		return SCRIPT_DEFAULT;
	}
	
	
	public int imperial_empire_day_mara_jade_handleBranch15(obj_id player, obj_id npc, string_id response) throws InterruptedException
	{
		
		if (response.equals("s_135"))
		{
			
			if (imperial_empire_day_mara_jade_condition__defaultCondition (player, npc))
			{
				imperial_empire_day_mara_jade_action_giveWaypoint (player, npc);
				
				string_id message = new string_id (c_stringFile, "s_85");
				utils.removeScriptVar (player, "conversation.imperial_empire_day_mara_jade.branchId");
				
				npcEndConversationWithMessage (player, message);
				
				return SCRIPT_CONTINUE;
			}
			
		}
		
		return SCRIPT_DEFAULT;
	}
	
	
	public int imperial_empire_day_mara_jade_handleBranch16(obj_id player, obj_id npc, string_id response) throws InterruptedException
	{
		
		if (response.equals("s_149"))
		{
			imperial_empire_day_mara_jade_action_revokeMission7Regrant (player, npc);
			
			if (imperial_empire_day_mara_jade_condition__defaultCondition (player, npc))
			{
				
				string_id message = new string_id (c_stringFile, "s_153");
				int numberOfResponses = 0;
				
				boolean hasResponse = false;
				
				boolean hasResponse0 = false;
				if (imperial_empire_day_mara_jade_condition__defaultCondition (player, npc))
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
					
					utils.setScriptVar (player, "conversation.imperial_empire_day_mara_jade.branchId", 17);
					
					npcSpeak (player, message);
					npcSetConversationResponses (player, responses);
				}
				else
				{
					utils.removeScriptVar (player, "conversation.imperial_empire_day_mara_jade.branchId");
					
					npcEndConversationWithMessage (player, message);
				}
				
				return SCRIPT_CONTINUE;
			}
			
		}
		
		return SCRIPT_DEFAULT;
	}
	
	
	public int imperial_empire_day_mara_jade_handleBranch17(obj_id player, obj_id npc, string_id response) throws InterruptedException
	{
		
		if (response.equals("s_156"))
		{
			
			if (imperial_empire_day_mara_jade_condition__defaultCondition (player, npc))
			{
				imperial_empire_day_mara_jade_action_giveWaypoint (player, npc);
				
				string_id message = new string_id (c_stringFile, "s_85");
				utils.removeScriptVar (player, "conversation.imperial_empire_day_mara_jade.branchId");
				
				npcEndConversationWithMessage (player, message);
				
				return SCRIPT_CONTINUE;
			}
			
		}
		
		return SCRIPT_DEFAULT;
	}
	
	
	public int imperial_empire_day_mara_jade_handleBranch19(obj_id player, obj_id npc, string_id response) throws InterruptedException
	{
		
		if (response.equals("s_158"))
		{
			doAnimationAction (player, "thumb_up");
			
			if (imperial_empire_day_mara_jade_condition__defaultCondition (player, npc))
			{
				doAnimationAction (npc, "explain");
				
				string_id message = new string_id (c_stringFile, "s_160");
				int numberOfResponses = 0;
				
				boolean hasResponse = false;
				
				boolean hasResponse0 = false;
				if (imperial_empire_day_mara_jade_condition__defaultCondition (player, npc))
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
					
					utils.setScriptVar (player, "conversation.imperial_empire_day_mara_jade.branchId", 20);
					
					npcSpeak (player, message);
					npcSetConversationResponses (player, responses);
				}
				else
				{
					utils.removeScriptVar (player, "conversation.imperial_empire_day_mara_jade.branchId");
					
					npcEndConversationWithMessage (player, message);
				}
				
				return SCRIPT_CONTINUE;
			}
			
		}
		
		return SCRIPT_DEFAULT;
	}
	
	
	public int imperial_empire_day_mara_jade_handleBranch20(obj_id player, obj_id npc, string_id response) throws InterruptedException
	{
		
		if (response.equals("s_161"))
		{
			doAnimationAction (player, "embarrassed");
			
			if (imperial_empire_day_mara_jade_condition__defaultCondition (player, npc))
			{
				doAnimationAction (npc, "laugh");
				
				string_id message = new string_id (c_stringFile, "s_162");
				int numberOfResponses = 0;
				
				boolean hasResponse = false;
				
				boolean hasResponse0 = false;
				if (imperial_empire_day_mara_jade_condition__defaultCondition (player, npc))
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
					
					utils.setScriptVar (player, "conversation.imperial_empire_day_mara_jade.branchId", 21);
					
					npcSpeak (player, message);
					npcSetConversationResponses (player, responses);
				}
				else
				{
					utils.removeScriptVar (player, "conversation.imperial_empire_day_mara_jade.branchId");
					
					npcEndConversationWithMessage (player, message);
				}
				
				return SCRIPT_CONTINUE;
			}
			
		}
		
		return SCRIPT_DEFAULT;
	}
	
	
	public int imperial_empire_day_mara_jade_handleBranch21(obj_id player, obj_id npc, string_id response) throws InterruptedException
	{
		
		if (response.equals("s_163"))
		{
			doAnimationAction (player, "shrug_shoulders");
			
			if (imperial_empire_day_mara_jade_condition__defaultCondition (player, npc))
			{
				doAnimationAction (npc, "nod");
				
				string_id message = new string_id (c_stringFile, "s_164");
				int numberOfResponses = 0;
				
				boolean hasResponse = false;
				
				boolean hasResponse0 = false;
				if (imperial_empire_day_mara_jade_condition__defaultCondition (player, npc))
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
						responses[responseIndex++] = new string_id (c_stringFile, "s_173");
					}
					
					utils.setScriptVar (player, "conversation.imperial_empire_day_mara_jade.branchId", 22);
					
					npcSpeak (player, message);
					npcSetConversationResponses (player, responses);
				}
				else
				{
					utils.removeScriptVar (player, "conversation.imperial_empire_day_mara_jade.branchId");
					
					npcEndConversationWithMessage (player, message);
				}
				
				return SCRIPT_CONTINUE;
			}
			
		}
		
		return SCRIPT_DEFAULT;
	}
	
	
	public int imperial_empire_day_mara_jade_handleBranch22(obj_id player, obj_id npc, string_id response) throws InterruptedException
	{
		
		if (response.equals("s_173"))
		{
			doAnimationAction (player, "cough_polite");
			
			if (imperial_empire_day_mara_jade_condition__defaultCondition (player, npc))
			{
				doAnimationAction (npc, "kiss_blow_kiss");
				
				imperial_empire_day_mara_jade_action_grantAutograph (player, npc);
				
				string_id message = new string_id (c_stringFile, "s_174");
				utils.removeScriptVar (player, "conversation.imperial_empire_day_mara_jade.branchId");
				
				npcEndConversationWithMessage (player, message);
				
				return SCRIPT_CONTINUE;
			}
			
		}
		
		return SCRIPT_DEFAULT;
	}
	
	
	public int imperial_empire_day_mara_jade_handleBranch24(obj_id player, obj_id npc, string_id response) throws InterruptedException
	{
		
		if (response.equals("s_51"))
		{
			
			if (imperial_empire_day_mara_jade_condition_isBelow60thLevel (player, npc))
			{
				
				string_id message = new string_id (c_stringFile, "s_53");
				int numberOfResponses = 0;
				
				boolean hasResponse = false;
				
				boolean hasResponse0 = false;
				if (imperial_empire_day_mara_jade_condition__defaultCondition (player, npc))
				{
					++numberOfResponses;
					hasResponse = true;
					hasResponse0 = true;
				}
				
				boolean hasResponse1 = false;
				if (imperial_empire_day_mara_jade_condition__defaultCondition (player, npc))
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
						responses[responseIndex++] = new string_id (c_stringFile, "s_55");
					}
					
					if (hasResponse1)
					{
						responses[responseIndex++] = new string_id (c_stringFile, "s_59");
					}
					
					utils.setScriptVar (player, "conversation.imperial_empire_day_mara_jade.branchId", 25);
					
					prose_package pp = new prose_package ();
					pp.stringId = message;
					pp.actor.set (player);
					pp.target.set (npc);
					
					npcSpeak (player, pp);
					npcSetConversationResponses (player, responses);
				}
				else
				{
					utils.removeScriptVar (player, "conversation.imperial_empire_day_mara_jade.branchId");
					
					prose_package pp = new prose_package ();
					pp.stringId = message;
					pp.actor.set (player);
					pp.target.set (npc);
					
					npcEndConversationWithMessage (player, pp);
				}
				
				return SCRIPT_CONTINUE;
			}
			
			if (imperial_empire_day_mara_jade_condition_isTrader (player, npc))
			{
				
				string_id message = new string_id (c_stringFile, "s_67");
				int numberOfResponses = 0;
				
				boolean hasResponse = false;
				
				boolean hasResponse0 = false;
				if (imperial_empire_day_mara_jade_condition__defaultCondition (player, npc))
				{
					++numberOfResponses;
					hasResponse = true;
					hasResponse0 = true;
				}
				
				boolean hasResponse1 = false;
				if (imperial_empire_day_mara_jade_condition__defaultCondition (player, npc))
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
						responses[responseIndex++] = new string_id (c_stringFile, "s_69");
					}
					
					if (hasResponse1)
					{
						responses[responseIndex++] = new string_id (c_stringFile, "s_73");
					}
					
					utils.setScriptVar (player, "conversation.imperial_empire_day_mara_jade.branchId", 29);
					
					prose_package pp = new prose_package ();
					pp.stringId = message;
					pp.actor.set (player);
					pp.target.set (npc);
					
					npcSpeak (player, pp);
					npcSetConversationResponses (player, responses);
				}
				else
				{
					utils.removeScriptVar (player, "conversation.imperial_empire_day_mara_jade.branchId");
					
					prose_package pp = new prose_package ();
					pp.stringId = message;
					pp.actor.set (player);
					pp.target.set (npc);
					
					npcEndConversationWithMessage (player, pp);
				}
				
				return SCRIPT_CONTINUE;
			}
			
			if (imperial_empire_day_mara_jade_condition_isEntertainer (player, npc))
			{
				
				string_id message = new string_id (c_stringFile, "s_75");
				int numberOfResponses = 0;
				
				boolean hasResponse = false;
				
				boolean hasResponse0 = false;
				if (imperial_empire_day_mara_jade_condition__defaultCondition (player, npc))
				{
					++numberOfResponses;
					hasResponse = true;
					hasResponse0 = true;
				}
				
				boolean hasResponse1 = false;
				if (imperial_empire_day_mara_jade_condition__defaultCondition (player, npc))
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
						responses[responseIndex++] = new string_id (c_stringFile, "s_77");
					}
					
					if (hasResponse1)
					{
						responses[responseIndex++] = new string_id (c_stringFile, "s_80");
					}
					
					utils.setScriptVar (player, "conversation.imperial_empire_day_mara_jade.branchId", 31);
					
					prose_package pp = new prose_package ();
					pp.stringId = message;
					pp.actor.set (player);
					pp.target.set (npc);
					
					npcSpeak (player, pp);
					npcSetConversationResponses (player, responses);
				}
				else
				{
					utils.removeScriptVar (player, "conversation.imperial_empire_day_mara_jade.branchId");
					
					prose_package pp = new prose_package ();
					pp.stringId = message;
					pp.actor.set (player);
					pp.target.set (npc);
					
					npcEndConversationWithMessage (player, pp);
				}
				
				return SCRIPT_CONTINUE;
			}
			
			if (imperial_empire_day_mara_jade_condition__defaultCondition (player, npc))
			{
				
				string_id message = new string_id (c_stringFile, "s_82");
				int numberOfResponses = 0;
				
				boolean hasResponse = false;
				
				boolean hasResponse0 = false;
				if (imperial_empire_day_mara_jade_condition__defaultCondition (player, npc))
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
					
					utils.setScriptVar (player, "conversation.imperial_empire_day_mara_jade.branchId", 32);
					
					prose_package pp = new prose_package ();
					pp.stringId = message;
					pp.actor.set (player);
					pp.target.set (npc);
					
					npcSpeak (player, pp);
					npcSetConversationResponses (player, responses);
				}
				else
				{
					utils.removeScriptVar (player, "conversation.imperial_empire_day_mara_jade.branchId");
					
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
	
	
	public int imperial_empire_day_mara_jade_handleBranch25(obj_id player, obj_id npc, string_id response) throws InterruptedException
	{
		
		if (response.equals("s_55"))
		{
			
			if (imperial_empire_day_mara_jade_condition__defaultCondition (player, npc))
			{
				
				string_id message = new string_id (c_stringFile, "s_57");
				utils.removeScriptVar (player, "conversation.imperial_empire_day_mara_jade.branchId");
				
				prose_package pp = new prose_package ();
				pp.stringId = message;
				pp.actor.set (player);
				pp.target.set (npc);
				
				npcEndConversationWithMessage (player, pp);
				
				return SCRIPT_CONTINUE;
			}
			
		}
		
		if (response.equals("s_59"))
		{
			
			if (imperial_empire_day_mara_jade_condition__defaultCondition (player, npc))
			{
				
				string_id message = new string_id (c_stringFile, "s_61");
				int numberOfResponses = 0;
				
				boolean hasResponse = false;
				
				boolean hasResponse0 = false;
				if (imperial_empire_day_mara_jade_condition__defaultCondition (player, npc))
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
					
					utils.setScriptVar (player, "conversation.imperial_empire_day_mara_jade.branchId", 27);
					
					npcSpeak (player, message);
					npcSetConversationResponses (player, responses);
				}
				else
				{
					utils.removeScriptVar (player, "conversation.imperial_empire_day_mara_jade.branchId");
					
					npcEndConversationWithMessage (player, message);
				}
				
				return SCRIPT_CONTINUE;
			}
			
		}
		
		return SCRIPT_DEFAULT;
	}
	
	
	public int imperial_empire_day_mara_jade_handleBranch27(obj_id player, obj_id npc, string_id response) throws InterruptedException
	{
		
		if (response.equals("s_63"))
		{
			
			if (imperial_empire_day_mara_jade_condition__defaultCondition (player, npc))
			{
				
				string_id message = new string_id (c_stringFile, "s_65");
				int numberOfResponses = 0;
				
				boolean hasResponse = false;
				
				boolean hasResponse0 = false;
				if (imperial_empire_day_mara_jade_condition__defaultCondition (player, npc))
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
					
					utils.setScriptVar (player, "conversation.imperial_empire_day_mara_jade.branchId", 32);
					
					prose_package pp = new prose_package ();
					pp.stringId = message;
					pp.actor.set (player);
					pp.target.set (npc);
					
					npcSpeak (player, pp);
					npcSetConversationResponses (player, responses);
				}
				else
				{
					utils.removeScriptVar (player, "conversation.imperial_empire_day_mara_jade.branchId");
					
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
	
	
	public int imperial_empire_day_mara_jade_handleBranch28(obj_id player, obj_id npc, string_id response) throws InterruptedException
	{
		
		if (response.equals("s_88"))
		{
			
			if (imperial_empire_day_mara_jade_condition__defaultCondition (player, npc))
			{
				
				string_id message = new string_id (c_stringFile, "s_91");
				int numberOfResponses = 0;
				
				boolean hasResponse = false;
				
				boolean hasResponse0 = false;
				if (imperial_empire_day_mara_jade_condition__defaultCondition (player, npc))
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
					
					utils.setScriptVar (player, "conversation.imperial_empire_day_mara_jade.branchId", 33);
					
					npcSpeak (player, message);
					npcSetConversationResponses (player, responses);
				}
				else
				{
					utils.removeScriptVar (player, "conversation.imperial_empire_day_mara_jade.branchId");
					
					npcEndConversationWithMessage (player, message);
				}
				
				return SCRIPT_CONTINUE;
			}
			
		}
		
		return SCRIPT_DEFAULT;
	}
	
	
	public int imperial_empire_day_mara_jade_handleBranch29(obj_id player, obj_id npc, string_id response) throws InterruptedException
	{
		
		if (response.equals("s_69"))
		{
			
			if (imperial_empire_day_mara_jade_condition__defaultCondition (player, npc))
			{
				
				string_id message = new string_id (c_stringFile, "s_71");
				utils.removeScriptVar (player, "conversation.imperial_empire_day_mara_jade.branchId");
				
				prose_package pp = new prose_package ();
				pp.stringId = message;
				pp.actor.set (player);
				pp.target.set (npc);
				
				npcEndConversationWithMessage (player, pp);
				
				return SCRIPT_CONTINUE;
			}
			
		}
		
		if (response.equals("s_73"))
		{
			
			if (imperial_empire_day_mara_jade_condition__defaultCondition (player, npc))
			{
				
				string_id message = new string_id (c_stringFile, "s_61");
				int numberOfResponses = 0;
				
				boolean hasResponse = false;
				
				boolean hasResponse0 = false;
				if (imperial_empire_day_mara_jade_condition__defaultCondition (player, npc))
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
					
					utils.setScriptVar (player, "conversation.imperial_empire_day_mara_jade.branchId", 27);
					
					npcSpeak (player, message);
					npcSetConversationResponses (player, responses);
				}
				else
				{
					utils.removeScriptVar (player, "conversation.imperial_empire_day_mara_jade.branchId");
					
					npcEndConversationWithMessage (player, message);
				}
				
				return SCRIPT_CONTINUE;
			}
			
		}
		
		return SCRIPT_DEFAULT;
	}
	
	
	public int imperial_empire_day_mara_jade_handleBranch31(obj_id player, obj_id npc, string_id response) throws InterruptedException
	{
		
		if (response.equals("s_77"))
		{
			
			if (imperial_empire_day_mara_jade_condition__defaultCondition (player, npc))
			{
				
				string_id message = new string_id (c_stringFile, "s_71");
				utils.removeScriptVar (player, "conversation.imperial_empire_day_mara_jade.branchId");
				
				prose_package pp = new prose_package ();
				pp.stringId = message;
				pp.actor.set (player);
				pp.target.set (npc);
				
				npcEndConversationWithMessage (player, pp);
				
				return SCRIPT_CONTINUE;
			}
			
		}
		
		if (response.equals("s_80"))
		{
			
			if (imperial_empire_day_mara_jade_condition__defaultCondition (player, npc))
			{
				
				string_id message = new string_id (c_stringFile, "s_61");
				int numberOfResponses = 0;
				
				boolean hasResponse = false;
				
				boolean hasResponse0 = false;
				if (imperial_empire_day_mara_jade_condition__defaultCondition (player, npc))
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
					
					utils.setScriptVar (player, "conversation.imperial_empire_day_mara_jade.branchId", 27);
					
					npcSpeak (player, message);
					npcSetConversationResponses (player, responses);
				}
				else
				{
					utils.removeScriptVar (player, "conversation.imperial_empire_day_mara_jade.branchId");
					
					npcEndConversationWithMessage (player, message);
				}
				
				return SCRIPT_CONTINUE;
			}
			
		}
		
		return SCRIPT_DEFAULT;
	}
	
	
	public int imperial_empire_day_mara_jade_handleBranch32(obj_id player, obj_id npc, string_id response) throws InterruptedException
	{
		
		if (response.equals("s_88"))
		{
			
			if (imperial_empire_day_mara_jade_condition__defaultCondition (player, npc))
			{
				
				string_id message = new string_id (c_stringFile, "s_91");
				int numberOfResponses = 0;
				
				boolean hasResponse = false;
				
				boolean hasResponse0 = false;
				if (imperial_empire_day_mara_jade_condition__defaultCondition (player, npc))
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
					
					utils.setScriptVar (player, "conversation.imperial_empire_day_mara_jade.branchId", 33);
					
					npcSpeak (player, message);
					npcSetConversationResponses (player, responses);
				}
				else
				{
					utils.removeScriptVar (player, "conversation.imperial_empire_day_mara_jade.branchId");
					
					npcEndConversationWithMessage (player, message);
				}
				
				return SCRIPT_CONTINUE;
			}
			
		}
		
		return SCRIPT_DEFAULT;
	}
	
	
	public int imperial_empire_day_mara_jade_handleBranch33(obj_id player, obj_id npc, string_id response) throws InterruptedException
	{
		
		if (response.equals("s_94"))
		{
			
			if (imperial_empire_day_mara_jade_condition__defaultCondition (player, npc))
			{
				
				string_id message = new string_id (c_stringFile, "s_96");
				int numberOfResponses = 0;
				
				boolean hasResponse = false;
				
				boolean hasResponse0 = false;
				if (imperial_empire_day_mara_jade_condition__defaultCondition (player, npc))
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
					
					utils.setScriptVar (player, "conversation.imperial_empire_day_mara_jade.branchId", 34);
					
					npcSpeak (player, message);
					npcSetConversationResponses (player, responses);
				}
				else
				{
					utils.removeScriptVar (player, "conversation.imperial_empire_day_mara_jade.branchId");
					
					npcEndConversationWithMessage (player, message);
				}
				
				return SCRIPT_CONTINUE;
			}
			
		}
		
		return SCRIPT_DEFAULT;
	}
	
	
	public int imperial_empire_day_mara_jade_handleBranch34(obj_id player, obj_id npc, string_id response) throws InterruptedException
	{
		
		if (response.equals("s_98"))
		{
			
			if (imperial_empire_day_mara_jade_condition__defaultCondition (player, npc))
			{
				
				string_id message = new string_id (c_stringFile, "s_105");
				int numberOfResponses = 0;
				
				boolean hasResponse = false;
				
				boolean hasResponse0 = false;
				if (imperial_empire_day_mara_jade_condition__defaultCondition (player, npc))
				{
					++numberOfResponses;
					hasResponse = true;
					hasResponse0 = true;
				}
				
				boolean hasResponse1 = false;
				if (imperial_empire_day_mara_jade_condition__defaultCondition (player, npc))
				{
					++numberOfResponses;
					hasResponse = true;
					hasResponse1 = true;
				}
				
				boolean hasResponse2 = false;
				if (imperial_empire_day_mara_jade_condition__defaultCondition (player, npc))
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
						responses[responseIndex++] = new string_id (c_stringFile, "s_109");
					}
					
					if (hasResponse1)
					{
						responses[responseIndex++] = new string_id (c_stringFile, "s_121");
					}
					
					if (hasResponse2)
					{
						responses[responseIndex++] = new string_id (c_stringFile, "s_137");
					}
					
					utils.setScriptVar (player, "conversation.imperial_empire_day_mara_jade.branchId", 35);
					
					npcSpeak (player, message);
					npcSetConversationResponses (player, responses);
				}
				else
				{
					utils.removeScriptVar (player, "conversation.imperial_empire_day_mara_jade.branchId");
					
					npcEndConversationWithMessage (player, message);
				}
				
				return SCRIPT_CONTINUE;
			}
			
		}
		
		return SCRIPT_DEFAULT;
	}
	
	
	public int imperial_empire_day_mara_jade_handleBranch35(obj_id player, obj_id npc, string_id response) throws InterruptedException
	{
		
		if (response.equals("s_109"))
		{
			
			if (imperial_empire_day_mara_jade_condition__defaultCondition (player, npc))
			{
				
				string_id message = new string_id (c_stringFile, "s_114");
				int numberOfResponses = 0;
				
				boolean hasResponse = false;
				
				boolean hasResponse0 = false;
				if (imperial_empire_day_mara_jade_condition__defaultCondition (player, npc))
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
					
					utils.setScriptVar (player, "conversation.imperial_empire_day_mara_jade.branchId", 36);
					
					npcSpeak (player, message);
					npcSetConversationResponses (player, responses);
				}
				else
				{
					utils.removeScriptVar (player, "conversation.imperial_empire_day_mara_jade.branchId");
					
					npcEndConversationWithMessage (player, message);
				}
				
				return SCRIPT_CONTINUE;
			}
			
		}
		
		if (response.equals("s_121"))
		{
			
			if (imperial_empire_day_mara_jade_condition__defaultCondition (player, npc))
			{
				doAnimationAction (npc, "backhand_threaten");
				
				string_id message = new string_id (c_stringFile, "s_128");
				int numberOfResponses = 0;
				
				boolean hasResponse = false;
				
				boolean hasResponse0 = false;
				if (imperial_empire_day_mara_jade_condition__defaultCondition (player, npc))
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
					
					utils.setScriptVar (player, "conversation.imperial_empire_day_mara_jade.branchId", 37);
					
					npcSpeak (player, message);
					npcSetConversationResponses (player, responses);
				}
				else
				{
					utils.removeScriptVar (player, "conversation.imperial_empire_day_mara_jade.branchId");
					
					npcEndConversationWithMessage (player, message);
				}
				
				return SCRIPT_CONTINUE;
			}
			
		}
		
		if (response.equals("s_137"))
		{
			doAnimationAction (player, "shake_head_no");
			
			if (imperial_empire_day_mara_jade_condition__defaultCondition (player, npc))
			{
				
				string_id message = new string_id (c_stringFile, "s_139");
				int numberOfResponses = 0;
				
				boolean hasResponse = false;
				
				boolean hasResponse0 = false;
				if (imperial_empire_day_mara_jade_condition__defaultCondition (player, npc))
				{
					++numberOfResponses;
					hasResponse = true;
					hasResponse0 = true;
				}
				
				boolean hasResponse1 = false;
				if (imperial_empire_day_mara_jade_condition__defaultCondition (player, npc))
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
						responses[responseIndex++] = new string_id (c_stringFile, "s_175");
					}
					
					if (hasResponse1)
					{
						responses[responseIndex++] = new string_id (c_stringFile, "s_177");
					}
					
					utils.setScriptVar (player, "conversation.imperial_empire_day_mara_jade.branchId", 38);
					
					prose_package pp = new prose_package ();
					pp.stringId = message;
					pp.actor.set (player);
					pp.target.set (npc);
					
					npcSpeak (player, pp);
					npcSetConversationResponses (player, responses);
				}
				else
				{
					utils.removeScriptVar (player, "conversation.imperial_empire_day_mara_jade.branchId");
					
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
	
	
	public int imperial_empire_day_mara_jade_handleBranch36(obj_id player, obj_id npc, string_id response) throws InterruptedException
	{
		
		if (response.equals("s_118"))
		{
			doAnimationAction (player, "shrug_shoulders");
			
			if (imperial_empire_day_mara_jade_condition__defaultCondition (player, npc))
			{
				
				string_id message = new string_id (c_stringFile, "s_139");
				int numberOfResponses = 0;
				
				boolean hasResponse = false;
				
				boolean hasResponse0 = false;
				if (imperial_empire_day_mara_jade_condition__defaultCondition (player, npc))
				{
					++numberOfResponses;
					hasResponse = true;
					hasResponse0 = true;
				}
				
				boolean hasResponse1 = false;
				if (imperial_empire_day_mara_jade_condition__defaultCondition (player, npc))
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
						responses[responseIndex++] = new string_id (c_stringFile, "s_175");
					}
					
					if (hasResponse1)
					{
						responses[responseIndex++] = new string_id (c_stringFile, "s_177");
					}
					
					utils.setScriptVar (player, "conversation.imperial_empire_day_mara_jade.branchId", 38);
					
					prose_package pp = new prose_package ();
					pp.stringId = message;
					pp.actor.set (player);
					pp.target.set (npc);
					
					npcSpeak (player, pp);
					npcSetConversationResponses (player, responses);
				}
				else
				{
					utils.removeScriptVar (player, "conversation.imperial_empire_day_mara_jade.branchId");
					
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
	
	
	public int imperial_empire_day_mara_jade_handleBranch37(obj_id player, obj_id npc, string_id response) throws InterruptedException
	{
		
		if (response.equals("s_132"))
		{
			doAnimationAction (player, "shrug_shoulders");
			
			if (imperial_empire_day_mara_jade_condition__defaultCondition (player, npc))
			{
				
				string_id message = new string_id (c_stringFile, "s_139");
				int numberOfResponses = 0;
				
				boolean hasResponse = false;
				
				boolean hasResponse0 = false;
				if (imperial_empire_day_mara_jade_condition__defaultCondition (player, npc))
				{
					++numberOfResponses;
					hasResponse = true;
					hasResponse0 = true;
				}
				
				boolean hasResponse1 = false;
				if (imperial_empire_day_mara_jade_condition__defaultCondition (player, npc))
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
						responses[responseIndex++] = new string_id (c_stringFile, "s_175");
					}
					
					if (hasResponse1)
					{
						responses[responseIndex++] = new string_id (c_stringFile, "s_177");
					}
					
					utils.setScriptVar (player, "conversation.imperial_empire_day_mara_jade.branchId", 38);
					
					prose_package pp = new prose_package ();
					pp.stringId = message;
					pp.actor.set (player);
					pp.target.set (npc);
					
					npcSpeak (player, pp);
					npcSetConversationResponses (player, responses);
				}
				else
				{
					utils.removeScriptVar (player, "conversation.imperial_empire_day_mara_jade.branchId");
					
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
	
	
	public int imperial_empire_day_mara_jade_handleBranch38(obj_id player, obj_id npc, string_id response) throws InterruptedException
	{
		
		if (response.equals("s_175"))
		{
			
			if (imperial_empire_day_mara_jade_condition__defaultCondition (player, npc))
			{
				doAnimationAction (npc, "kiss");
				
				string_id message = new string_id (c_stringFile, "s_176");
				int numberOfResponses = 0;
				
				boolean hasResponse = false;
				
				boolean hasResponse0 = false;
				if (imperial_empire_day_mara_jade_condition__defaultCondition (player, npc))
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
					
					utils.setScriptVar (player, "conversation.imperial_empire_day_mara_jade.branchId", 39);
					
					npcSpeak (player, message);
					npcSetConversationResponses (player, responses);
				}
				else
				{
					utils.removeScriptVar (player, "conversation.imperial_empire_day_mara_jade.branchId");
					
					npcEndConversationWithMessage (player, message);
				}
				
				return SCRIPT_CONTINUE;
			}
			
		}
		
		if (response.equals("s_177"))
		{
			
			if (imperial_empire_day_mara_jade_condition__defaultCondition (player, npc))
			{
				doAnimationAction (npc, "salute1");
				
				doAnimationAction (player, "salute1");
				
				imperial_empire_day_mara_jade_action_grantMissionTwo (player, npc);
				
				string_id message = new string_id (c_stringFile, "s_178");
				utils.removeScriptVar (player, "conversation.imperial_empire_day_mara_jade.branchId");
				
				npcEndConversationWithMessage (player, message);
				
				return SCRIPT_CONTINUE;
			}
			
		}
		
		return SCRIPT_DEFAULT;
	}
	
	
	public int imperial_empire_day_mara_jade_handleBranch39(obj_id player, obj_id npc, string_id response) throws InterruptedException
	{
		
		if (response.equals("s_179"))
		{
			doAnimationAction (player, "embarrassed");
			
			if (imperial_empire_day_mara_jade_condition__defaultCondition (player, npc))
			{
				doAnimationAction (npc, "point_away");
				
				imperial_empire_day_mara_jade_action_grantMissionTwo (player, npc);
				
				string_id message = new string_id (c_stringFile, "s_180");
				utils.removeScriptVar (player, "conversation.imperial_empire_day_mara_jade.branchId");
				
				npcEndConversationWithMessage (player, message);
				
				return SCRIPT_CONTINUE;
			}
			
		}
		
		return SCRIPT_DEFAULT;
	}
	
	
	public int imperial_empire_day_mara_jade_handleBranch43(obj_id player, obj_id npc, string_id response) throws InterruptedException
	{
		
		if (response.equals("s_154"))
		{
			
			if (imperial_empire_day_mara_jade_condition__defaultCondition (player, npc))
			{
				doAnimationAction (npc, "nod_head_once");
				
				string_id message = new string_id (c_stringFile, "s_159");
				int numberOfResponses = 0;
				
				boolean hasResponse = false;
				
				boolean hasResponse0 = false;
				if (imperial_empire_day_mara_jade_condition__defaultCondition (player, npc))
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
					
					utils.setScriptVar (player, "conversation.imperial_empire_day_mara_jade.branchId", 44);
					
					prose_package pp = new prose_package ();
					pp.stringId = message;
					pp.actor.set (player);
					pp.target.set (npc);
					
					npcSpeak (player, pp);
					npcSetConversationResponses (player, responses);
				}
				else
				{
					utils.removeScriptVar (player, "conversation.imperial_empire_day_mara_jade.branchId");
					
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
	
	
	public int imperial_empire_day_mara_jade_handleBranch44(obj_id player, obj_id npc, string_id response) throws InterruptedException
	{
		
		if (response.equals("s_167"))
		{
			
			if (imperial_empire_day_mara_jade_condition__defaultCondition (player, npc))
			{
				doAnimationAction (npc, "rub_chin_thoughtful");
				
				string_id message = new string_id (c_stringFile, "s_169");
				int numberOfResponses = 0;
				
				boolean hasResponse = false;
				
				boolean hasResponse0 = false;
				if (imperial_empire_day_mara_jade_condition__defaultCondition (player, npc))
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
					
					utils.setScriptVar (player, "conversation.imperial_empire_day_mara_jade.branchId", 45);
					
					npcSpeak (player, message);
					npcSetConversationResponses (player, responses);
				}
				else
				{
					utils.removeScriptVar (player, "conversation.imperial_empire_day_mara_jade.branchId");
					
					npcEndConversationWithMessage (player, message);
				}
				
				return SCRIPT_CONTINUE;
			}
			
		}
		
		return SCRIPT_DEFAULT;
	}
	
	
	public int imperial_empire_day_mara_jade_handleBranch45(obj_id player, obj_id npc, string_id response) throws InterruptedException
	{
		
		if (response.equals("s_171"))
		{
			
			if (imperial_empire_day_mara_jade_condition__defaultCondition (player, npc))
			{
				
				string_id message = new string_id (c_stringFile, "s_181");
				int numberOfResponses = 0;
				
				boolean hasResponse = false;
				
				boolean hasResponse0 = false;
				if (imperial_empire_day_mara_jade_condition__defaultCondition (player, npc))
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
						responses[responseIndex++] = new string_id (c_stringFile, "s_183");
					}
					
					utils.setScriptVar (player, "conversation.imperial_empire_day_mara_jade.branchId", 46);
					
					prose_package pp = new prose_package ();
					pp.stringId = message;
					pp.actor.set (player);
					pp.target.set (npc);
					
					npcSpeak (player, pp);
					npcSetConversationResponses (player, responses);
				}
				else
				{
					utils.removeScriptVar (player, "conversation.imperial_empire_day_mara_jade.branchId");
					
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
	
	
	public int imperial_empire_day_mara_jade_handleBranch46(obj_id player, obj_id npc, string_id response) throws InterruptedException
	{
		
		if (response.equals("s_183"))
		{
			doAnimationAction (player, "salute1");
			
			if (imperial_empire_day_mara_jade_condition__defaultCondition (player, npc))
			{
				
				string_id message = new string_id (c_stringFile, "s_185");
				int numberOfResponses = 0;
				
				boolean hasResponse = false;
				
				boolean hasResponse0 = false;
				if (imperial_empire_day_mara_jade_condition__defaultCondition (player, npc))
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
					
					utils.setScriptVar (player, "conversation.imperial_empire_day_mara_jade.branchId", 47);
					
					npcSpeak (player, message);
					npcSetConversationResponses (player, responses);
				}
				else
				{
					utils.removeScriptVar (player, "conversation.imperial_empire_day_mara_jade.branchId");
					
					npcEndConversationWithMessage (player, message);
				}
				
				return SCRIPT_CONTINUE;
			}
			
		}
		
		return SCRIPT_DEFAULT;
	}
	
	
	public int imperial_empire_day_mara_jade_handleBranch47(obj_id player, obj_id npc, string_id response) throws InterruptedException
	{
		
		if (response.equals("s_187"))
		{
			doAnimationAction (player, "salute1");
			
			if (imperial_empire_day_mara_jade_condition__defaultCondition (player, npc))
			{
				imperial_empire_day_mara_jade_action_grantMissionOne (player, npc);
				
				string_id message = new string_id (c_stringFile, "s_190");
				utils.removeScriptVar (player, "conversation.imperial_empire_day_mara_jade.branchId");
				
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
			detachScript(self, "conversation.imperial_empire_day_mara_jade");
		}
		
		setCondition (self, CONDITION_CONVERSABLE);
		setCondition (self, CONDITION_INTERESTING);
		return SCRIPT_CONTINUE;
	}
	
	
	public int OnAttach(obj_id self) throws InterruptedException
	{
		
		setCondition (self, CONDITION_CONVERSABLE);
		setCondition (self, CONDITION_INTERESTING);
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
		detachScript (self, "conversation.imperial_empire_day_mara_jade");
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int OnDetach(obj_id self) throws InterruptedException
	{
		clearCondition (self, CONDITION_CONVERSABLE);
		
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
		
		if (imperial_empire_day_mara_jade_condition_isRebelPlayer (player, npc))
		{
			doAnimationAction (npc, "weeping");
			
			string_id message = new string_id (c_stringFile, "s_133");
			chat.chat (npc, player, message);
			
			return SCRIPT_CONTINUE;
		}
		
		if (imperial_empire_day_mara_jade_condition_isNeutralPlayer (player, npc))
		{
			
			string_id message = new string_id (c_stringFile, "s_134");
			chat.chat (npc, player, message);
			
			return SCRIPT_CONTINUE;
		}
		
		if (imperial_empire_day_mara_jade_condition_hasAnIncompleteCombatMission (player, npc))
		{
			
			string_id message = new string_id (c_stringFile, "s_79");
			int numberOfResponses = 0;
			
			boolean hasResponse = false;
			
			boolean hasResponse0 = false;
			if (imperial_empire_day_mara_jade_condition__defaultCondition (player, npc))
			{
				++numberOfResponses;
				hasResponse = true;
				hasResponse0 = true;
			}
			
			boolean hasResponse1 = false;
			if (imperial_empire_day_mara_jade_condition__defaultCondition (player, npc))
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
					responses[responseIndex++] = new string_id (c_stringFile, "s_83");
				}
				
				if (hasResponse1)
				{
					responses[responseIndex++] = new string_id (c_stringFile, "s_84");
				}
				
				utils.setScriptVar (player, "conversation.imperial_empire_day_mara_jade.branchId", 3);
				
				npcStartConversation (player, npc, "imperial_empire_day_mara_jade", message, responses);
			}
			else
			{
				chat.chat (npc, player, message);
			}
			
			return SCRIPT_CONTINUE;
		}
		
		if (imperial_empire_day_mara_jade_condition_completed7thQuestHasAutograph (player, npc))
		{
			
			string_id message = new string_id (c_stringFile, "s_165");
			prose_package pp = new prose_package ();
			pp.stringId = message;
			pp.actor.set (player);
			pp.target.set (npc);
			
			chat.chat (npc, player, null, null, pp);
			return SCRIPT_CONTINUE;
		}
		
		if (imperial_empire_day_mara_jade_condition_completed7thQuestNoAutograph (player, npc))
		{
			doAnimationAction (npc, "greet");
			
			string_id message = new string_id (c_stringFile, "s_157");
			int numberOfResponses = 0;
			
			boolean hasResponse = false;
			
			boolean hasResponse0 = false;
			if (imperial_empire_day_mara_jade_condition__defaultCondition (player, npc))
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
					responses[responseIndex++] = new string_id (c_stringFile, "s_158");
				}
				
				utils.setScriptVar (player, "conversation.imperial_empire_day_mara_jade.branchId", 19);
				
				prose_package pp = new prose_package ();
				pp.stringId = message;
				pp.actor.set (player);
				pp.target.set (npc);
				
				npcStartConversation (player, npc, "imperial_empire_day_mara_jade", null, pp, responses);
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
		
		if (imperial_empire_day_mara_jade_condition_hasCompleted1stMissionNot2nd (player, npc))
		{
			
			string_id message = new string_id (c_stringFile, "s_49");
			int numberOfResponses = 0;
			
			boolean hasResponse = false;
			
			boolean hasResponse0 = false;
			if (imperial_empire_day_mara_jade_condition__defaultCondition (player, npc))
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
					responses[responseIndex++] = new string_id (c_stringFile, "s_51");
				}
				
				utils.setScriptVar (player, "conversation.imperial_empire_day_mara_jade.branchId", 24);
				
				npcStartConversation (player, npc, "imperial_empire_day_mara_jade", message, responses);
			}
			else
			{
				chat.chat (npc, player, message);
			}
			
			return SCRIPT_CONTINUE;
		}
		
		if (imperial_empire_day_mara_jade_condition_hasNotCompletedMission1 (player, npc))
		{
			
			string_id message = new string_id (c_stringFile, "s_189");
			chat.chat (npc, player, message);
			
			return SCRIPT_CONTINUE;
		}
		
		if (imperial_empire_day_mara_jade_condition__defaultCondition (player, npc))
		{
			
			string_id message = new string_id (c_stringFile, "s_151");
			int numberOfResponses = 0;
			
			boolean hasResponse = false;
			
			boolean hasResponse0 = false;
			if (imperial_empire_day_mara_jade_condition__defaultCondition (player, npc))
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
					responses[responseIndex++] = new string_id (c_stringFile, "s_154");
				}
				
				utils.setScriptVar (player, "conversation.imperial_empire_day_mara_jade.branchId", 43);
				
				npcStartConversation (player, npc, "imperial_empire_day_mara_jade", message, responses);
			}
			else
			{
				chat.chat (npc, player, message);
			}
			
			return SCRIPT_CONTINUE;
		}
		
		chat.chat (npc, "Error: All conditions for OnStartNpcConversation were false.");
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int OnNpcConversationResponse(obj_id self, String conversationId, obj_id player, string_id response) throws InterruptedException
	{
		if (!conversationId.equals("imperial_empire_day_mara_jade"))
		{
			return SCRIPT_CONTINUE;
		}
		
		obj_id npc = self;
		
		int branchId = utils.getIntScriptVar (player, "conversation.imperial_empire_day_mara_jade.branchId");
		
		if (branchId == 3 && imperial_empire_day_mara_jade_handleBranch3 (player, npc, response) == SCRIPT_CONTINUE)
		{
			return SCRIPT_CONTINUE;
		}
		
		if (branchId == 5 && imperial_empire_day_mara_jade_handleBranch5 (player, npc, response) == SCRIPT_CONTINUE)
		{
			return SCRIPT_CONTINUE;
		}
		
		if (branchId == 6 && imperial_empire_day_mara_jade_handleBranch6 (player, npc, response) == SCRIPT_CONTINUE)
		{
			return SCRIPT_CONTINUE;
		}
		
		if (branchId == 8 && imperial_empire_day_mara_jade_handleBranch8 (player, npc, response) == SCRIPT_CONTINUE)
		{
			return SCRIPT_CONTINUE;
		}
		
		if (branchId == 9 && imperial_empire_day_mara_jade_handleBranch9 (player, npc, response) == SCRIPT_CONTINUE)
		{
			return SCRIPT_CONTINUE;
		}
		
		if (branchId == 10 && imperial_empire_day_mara_jade_handleBranch10 (player, npc, response) == SCRIPT_CONTINUE)
		{
			return SCRIPT_CONTINUE;
		}
		
		if (branchId == 11 && imperial_empire_day_mara_jade_handleBranch11 (player, npc, response) == SCRIPT_CONTINUE)
		{
			return SCRIPT_CONTINUE;
		}
		
		if (branchId == 12 && imperial_empire_day_mara_jade_handleBranch12 (player, npc, response) == SCRIPT_CONTINUE)
		{
			return SCRIPT_CONTINUE;
		}
		
		if (branchId == 13 && imperial_empire_day_mara_jade_handleBranch13 (player, npc, response) == SCRIPT_CONTINUE)
		{
			return SCRIPT_CONTINUE;
		}
		
		if (branchId == 14 && imperial_empire_day_mara_jade_handleBranch14 (player, npc, response) == SCRIPT_CONTINUE)
		{
			return SCRIPT_CONTINUE;
		}
		
		if (branchId == 15 && imperial_empire_day_mara_jade_handleBranch15 (player, npc, response) == SCRIPT_CONTINUE)
		{
			return SCRIPT_CONTINUE;
		}
		
		if (branchId == 16 && imperial_empire_day_mara_jade_handleBranch16 (player, npc, response) == SCRIPT_CONTINUE)
		{
			return SCRIPT_CONTINUE;
		}
		
		if (branchId == 17 && imperial_empire_day_mara_jade_handleBranch17 (player, npc, response) == SCRIPT_CONTINUE)
		{
			return SCRIPT_CONTINUE;
		}
		
		if (branchId == 19 && imperial_empire_day_mara_jade_handleBranch19 (player, npc, response) == SCRIPT_CONTINUE)
		{
			return SCRIPT_CONTINUE;
		}
		
		if (branchId == 20 && imperial_empire_day_mara_jade_handleBranch20 (player, npc, response) == SCRIPT_CONTINUE)
		{
			return SCRIPT_CONTINUE;
		}
		
		if (branchId == 21 && imperial_empire_day_mara_jade_handleBranch21 (player, npc, response) == SCRIPT_CONTINUE)
		{
			return SCRIPT_CONTINUE;
		}
		
		if (branchId == 22 && imperial_empire_day_mara_jade_handleBranch22 (player, npc, response) == SCRIPT_CONTINUE)
		{
			return SCRIPT_CONTINUE;
		}
		
		if (branchId == 24 && imperial_empire_day_mara_jade_handleBranch24 (player, npc, response) == SCRIPT_CONTINUE)
		{
			return SCRIPT_CONTINUE;
		}
		
		if (branchId == 25 && imperial_empire_day_mara_jade_handleBranch25 (player, npc, response) == SCRIPT_CONTINUE)
		{
			return SCRIPT_CONTINUE;
		}
		
		if (branchId == 27 && imperial_empire_day_mara_jade_handleBranch27 (player, npc, response) == SCRIPT_CONTINUE)
		{
			return SCRIPT_CONTINUE;
		}
		
		if (branchId == 28 && imperial_empire_day_mara_jade_handleBranch28 (player, npc, response) == SCRIPT_CONTINUE)
		{
			return SCRIPT_CONTINUE;
		}
		
		if (branchId == 29 && imperial_empire_day_mara_jade_handleBranch29 (player, npc, response) == SCRIPT_CONTINUE)
		{
			return SCRIPT_CONTINUE;
		}
		
		if (branchId == 31 && imperial_empire_day_mara_jade_handleBranch31 (player, npc, response) == SCRIPT_CONTINUE)
		{
			return SCRIPT_CONTINUE;
		}
		
		if (branchId == 32 && imperial_empire_day_mara_jade_handleBranch32 (player, npc, response) == SCRIPT_CONTINUE)
		{
			return SCRIPT_CONTINUE;
		}
		
		if (branchId == 33 && imperial_empire_day_mara_jade_handleBranch33 (player, npc, response) == SCRIPT_CONTINUE)
		{
			return SCRIPT_CONTINUE;
		}
		
		if (branchId == 34 && imperial_empire_day_mara_jade_handleBranch34 (player, npc, response) == SCRIPT_CONTINUE)
		{
			return SCRIPT_CONTINUE;
		}
		
		if (branchId == 35 && imperial_empire_day_mara_jade_handleBranch35 (player, npc, response) == SCRIPT_CONTINUE)
		{
			return SCRIPT_CONTINUE;
		}
		
		if (branchId == 36 && imperial_empire_day_mara_jade_handleBranch36 (player, npc, response) == SCRIPT_CONTINUE)
		{
			return SCRIPT_CONTINUE;
		}
		
		if (branchId == 37 && imperial_empire_day_mara_jade_handleBranch37 (player, npc, response) == SCRIPT_CONTINUE)
		{
			return SCRIPT_CONTINUE;
		}
		
		if (branchId == 38 && imperial_empire_day_mara_jade_handleBranch38 (player, npc, response) == SCRIPT_CONTINUE)
		{
			return SCRIPT_CONTINUE;
		}
		
		if (branchId == 39 && imperial_empire_day_mara_jade_handleBranch39 (player, npc, response) == SCRIPT_CONTINUE)
		{
			return SCRIPT_CONTINUE;
		}
		
		if (branchId == 43 && imperial_empire_day_mara_jade_handleBranch43 (player, npc, response) == SCRIPT_CONTINUE)
		{
			return SCRIPT_CONTINUE;
		}
		
		if (branchId == 44 && imperial_empire_day_mara_jade_handleBranch44 (player, npc, response) == SCRIPT_CONTINUE)
		{
			return SCRIPT_CONTINUE;
		}
		
		if (branchId == 45 && imperial_empire_day_mara_jade_handleBranch45 (player, npc, response) == SCRIPT_CONTINUE)
		{
			return SCRIPT_CONTINUE;
		}
		
		if (branchId == 46 && imperial_empire_day_mara_jade_handleBranch46 (player, npc, response) == SCRIPT_CONTINUE)
		{
			return SCRIPT_CONTINUE;
		}
		
		if (branchId == 47 && imperial_empire_day_mara_jade_handleBranch47 (player, npc, response) == SCRIPT_CONTINUE)
		{
			return SCRIPT_CONTINUE;
		}
		
		chat.chat (npc, "Error: Fell through all branches and responses for OnNpcConversationResponse.");
		
		utils.removeScriptVar (player, "conversation.imperial_empire_day_mara_jade.branchId");
		
		return SCRIPT_CONTINUE;
	}
	
}
