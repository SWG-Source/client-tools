package script.conversation;

import script.*;
import script.base_class.*;
import script.combat_engine.*;
import java.util.Arrays;
import java.util.Hashtable;
import java.util.Vector;
import script.base_script;


import script.library.ai_lib;
import script.library.buff;
import script.library.chat;
import script.library.conversation;
import script.library.groundquests;
import script.library.utils;


public class zahara_cody extends script.base_script
{
	public zahara_cody()
	{
	}
	String c_stringFile = "conversation/zahara_cody";
	
	
	public boolean zahara_cody_condition__defaultCondition(obj_id player, obj_id npc) throws InterruptedException
	{
		return true;
	}
	
	
	public boolean zahara_cody_condition_isImperialNotImmunized(obj_id player, obj_id npc) throws InterruptedException
	{
		faceTo(npc, player);
		return (groundquests.isQuestActive(player, "outbreak_quest_01_imperial") || groundquests.isQuestActive(player, "outbreak_quest_01_rebel") || groundquests.isQuestActive(player, "outbreak_quest_01_neutral"));
	}
	
	
	public boolean zahara_cody_condition_hasTrigTask(obj_id player, obj_id npc) throws InterruptedException
	{
		return (groundquests.isTaskActive(player, "outbreak_quest_01_b_imperial", "talkToTrig") || groundquests.isTaskActive(player, "outbreak_quest_01_b_rebel", "talkToTrig") || groundquests.isTaskActive(player, "outbreak_quest_01_b_neutral", "talkToTrig") );
	}
	
	
	public boolean zahara_cody_condition_hasFoundEpsilon(obj_id player, obj_id npc) throws InterruptedException
	{
		return groundquests.hasCompletedQuest(player, "outbreak_quest_02_imperial") || groundquests.hasCompletedQuest(player, "outbreak_quest_02_rebel") || groundquests.hasCompletedQuest(player, "outbreak_quest_02_neutral");
	}
	
	
	public boolean zahara_cody_condition_hasDeliveredRadioEpsilon(obj_id player, obj_id npc) throws InterruptedException
	{
		return groundquests.hasCompletedQuest(player, "outbreak_radio_delivery_03");
	}
	
	
	public boolean zahara_cody_condition_hasCompletedQuest01(obj_id player, obj_id npc) throws InterruptedException
	{
		return groundquests.hasCompletedQuest(player, "outbreak_quest_01_imperial") || groundquests.hasCompletedQuest(player, "outbreak_quest_01_rebel") || groundquests.hasCompletedQuest(player, "outbreak_quest_01_neutral");
	}
	
	
	public boolean zahara_cody_condition_hasCompletedQuestLine(obj_id player, obj_id npc) throws InterruptedException
	{
		return groundquests.hasCompletedQuest(player, "outbreak_quest_final_imperial") || groundquests.hasCompletedQuest(player, "outbreak_quest_final_neutral") || groundquests.hasCompletedQuest(player, "outbreak_quest_final_rebel");
	}
	
	
	public boolean zahara_cody_condition_hasDeletedQuest01b(obj_id player, obj_id npc) throws InterruptedException
	{
		return (groundquests.hasCompletedQuest(player, "outbreak_quest_01_imperial") && !groundquests.isQuestActiveOrComplete(player, "outbreak_quest_01_b_imperial")) || (groundquests.hasCompletedQuest(player, "outbreak_quest_01_rebel") && !groundquests.isQuestActiveOrComplete(player, "outbreak_quest_01_b_rebel")) || (groundquests.hasCompletedQuest(player, "outbreak_quest_01_neutral") && !groundquests.isQuestActiveOrComplete(player, "outbreak_quest_01_b_neutral"));
	}
	
	
	public void zahara_cody_action_giveAntiVirus(obj_id player, obj_id npc) throws InterruptedException
	{
		faceTo(npc, player);
		groundquests.sendSignal(player, "hasBeenInnoculated");
		utils.setScriptVar(player, "outbreak.innoculated", 1);
		
		buff.removeBuff(player, "death_troopers_infection_1");
		buff.removeBuff(player, "death_troopers_infection_2");
		buff.removeBuff(player, "death_troopers_infection_3");
	}
	
	
	public void zahara_cody_action_makeTrigWave(obj_id player, obj_id npc) throws InterruptedException
	{
		if (!hasObjVar(npc, "myTrig"))
		{
			CustomerServiceLog("outbreak_themepark", "outbreak_zahara_cody conversation: Han has no Trig objvar. Self: "+npc);
			return;
		}
		
		obj_id myTrig = getObjIdObjVar(npc, "myTrig");
		if (!isValidId(myTrig) || !exists(myTrig))
		{
			CustomerServiceLog("outbreak_themepark", "outbreak_zahara_cody conversation: Cody has a trig objvar that is invalid. Self: "+ npc);
			return;
		}
		faceTo(myTrig, player);
		doAnimationAction(myTrig, "wave1");
	}
	
	
	public void zahara_cody_action_giveQuest1b(obj_id player, obj_id npc) throws InterruptedException
	{
		if ((groundquests.hasCompletedQuest(player, "outbreak_quest_01_imperial") && !groundquests.isQuestActiveOrComplete(player, "outbreak_quest_01_b_imperial")))
		{
			groundquests.grantQuest(player, "outbreak_quest_01_b_imperial");
			return;
		}
		
		if ((groundquests.hasCompletedQuest(player, "outbreak_quest_01_rebel") && !groundquests.isQuestActiveOrComplete(player, "outbreak_quest_01_b_rebel")))
		{
			groundquests.grantQuest(player, "outbreak_quest_01_b_rebel");
			return;
		}
		
		if ((groundquests.hasCompletedQuest(player, "outbreak_quest_01_neutral") && !groundquests.isQuestActiveOrComplete(player, "outbreak_quest_01_b_neutral")))
		{
			groundquests.grantQuest(player, "outbreak_quest_01_b_neutral");
			return;
		}
	}
	
	
	public int zahara_cody_handleBranch7(obj_id player, obj_id npc, string_id response) throws InterruptedException
	{
		
		if (response.equals("s_12"))
		{
			
			if (zahara_cody_condition__defaultCondition (player, npc))
			{
				zahara_cody_action_makeTrigWave (player, npc);
				
				string_id message = new string_id (c_stringFile, "s_123");
				int numberOfResponses = 0;
				
				boolean hasResponse = false;
				
				boolean hasResponse0 = false;
				if (zahara_cody_condition__defaultCondition (player, npc))
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
					
					utils.setScriptVar (player, "conversation.zahara_cody.branchId", 8);
					
					npcSpeak (player, message);
					npcSetConversationResponses (player, responses);
				}
				else
				{
					utils.removeScriptVar (player, "conversation.zahara_cody.branchId");
					
					npcEndConversationWithMessage (player, message);
				}
				
				return SCRIPT_CONTINUE;
			}
			
		}
		
		return SCRIPT_DEFAULT;
	}
	
	
	public int zahara_cody_handleBranch8(obj_id player, obj_id npc, string_id response) throws InterruptedException
	{
		
		if (response.equals("s_124"))
		{
			
			if (zahara_cody_condition__defaultCondition (player, npc))
			{
				
				string_id message = new string_id (c_stringFile, "s_128");
				int numberOfResponses = 0;
				
				boolean hasResponse = false;
				
				boolean hasResponse0 = false;
				if (zahara_cody_condition__defaultCondition (player, npc))
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
					
					utils.setScriptVar (player, "conversation.zahara_cody.branchId", 9);
					
					npcSpeak (player, message);
					npcSetConversationResponses (player, responses);
				}
				else
				{
					utils.removeScriptVar (player, "conversation.zahara_cody.branchId");
					
					npcEndConversationWithMessage (player, message);
				}
				
				return SCRIPT_CONTINUE;
			}
			
		}
		
		return SCRIPT_DEFAULT;
	}
	
	
	public int zahara_cody_handleBranch9(obj_id player, obj_id npc, string_id response) throws InterruptedException
	{
		
		if (response.equals("s_64"))
		{
			
			if (zahara_cody_condition__defaultCondition (player, npc))
			{
				
				string_id message = new string_id (c_stringFile, "s_66");
				int numberOfResponses = 0;
				
				boolean hasResponse = false;
				
				boolean hasResponse0 = false;
				if (zahara_cody_condition__defaultCondition (player, npc))
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
					
					utils.setScriptVar (player, "conversation.zahara_cody.branchId", 10);
					
					npcSpeak (player, message);
					npcSetConversationResponses (player, responses);
				}
				else
				{
					utils.removeScriptVar (player, "conversation.zahara_cody.branchId");
					
					npcEndConversationWithMessage (player, message);
				}
				
				return SCRIPT_CONTINUE;
			}
			
		}
		
		return SCRIPT_DEFAULT;
	}
	
	
	public int zahara_cody_handleBranch10(obj_id player, obj_id npc, string_id response) throws InterruptedException
	{
		
		if (response.equals("s_68"))
		{
			
			if (zahara_cody_condition__defaultCondition (player, npc))
			{
				
				string_id message = new string_id (c_stringFile, "s_70");
				int numberOfResponses = 0;
				
				boolean hasResponse = false;
				
				boolean hasResponse0 = false;
				if (zahara_cody_condition__defaultCondition (player, npc))
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
						responses[responseIndex++] = new string_id (c_stringFile, "s_72");
					}
					
					utils.setScriptVar (player, "conversation.zahara_cody.branchId", 11);
					
					npcSpeak (player, message);
					npcSetConversationResponses (player, responses);
				}
				else
				{
					utils.removeScriptVar (player, "conversation.zahara_cody.branchId");
					
					npcEndConversationWithMessage (player, message);
				}
				
				return SCRIPT_CONTINUE;
			}
			
		}
		
		return SCRIPT_DEFAULT;
	}
	
	
	public int zahara_cody_handleBranch11(obj_id player, obj_id npc, string_id response) throws InterruptedException
	{
		
		if (response.equals("s_72"))
		{
			
			if (zahara_cody_condition__defaultCondition (player, npc))
			{
				
				string_id message = new string_id (c_stringFile, "s_75");
				int numberOfResponses = 0;
				
				boolean hasResponse = false;
				
				boolean hasResponse0 = false;
				if (zahara_cody_condition__defaultCondition (player, npc))
				{
					++numberOfResponses;
					hasResponse = true;
					hasResponse0 = true;
				}
				
				boolean hasResponse1 = false;
				if (zahara_cody_condition__defaultCondition (player, npc))
				{
					++numberOfResponses;
					hasResponse = true;
					hasResponse1 = true;
				}
				
				boolean hasResponse2 = false;
				if (zahara_cody_condition__defaultCondition (player, npc))
				{
					++numberOfResponses;
					hasResponse = true;
					hasResponse2 = true;
				}
				
				boolean hasResponse3 = false;
				if (zahara_cody_condition__defaultCondition (player, npc))
				{
					++numberOfResponses;
					hasResponse = true;
					hasResponse3 = true;
				}
				
				boolean hasResponse4 = false;
				if (zahara_cody_condition__defaultCondition (player, npc))
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
						responses[responseIndex++] = new string_id (c_stringFile, "s_77");
					}
					
					if (hasResponse1)
					{
						responses[responseIndex++] = new string_id (c_stringFile, "s_97");
					}
					
					if (hasResponse2)
					{
						responses[responseIndex++] = new string_id (c_stringFile, "s_105");
					}
					
					if (hasResponse3)
					{
						responses[responseIndex++] = new string_id (c_stringFile, "s_169");
					}
					
					if (hasResponse4)
					{
						responses[responseIndex++] = new string_id (c_stringFile, "s_207");
					}
					
					utils.setScriptVar (player, "conversation.zahara_cody.branchId", 12);
					
					npcSpeak (player, message);
					npcSetConversationResponses (player, responses);
				}
				else
				{
					utils.removeScriptVar (player, "conversation.zahara_cody.branchId");
					
					npcEndConversationWithMessage (player, message);
				}
				
				return SCRIPT_CONTINUE;
			}
			
		}
		
		return SCRIPT_DEFAULT;
	}
	
	
	public int zahara_cody_handleBranch12(obj_id player, obj_id npc, string_id response) throws InterruptedException
	{
		
		if (response.equals("s_77"))
		{
			
			if (zahara_cody_condition__defaultCondition (player, npc))
			{
				
				string_id message = new string_id (c_stringFile, "s_79");
				int numberOfResponses = 0;
				
				boolean hasResponse = false;
				
				boolean hasResponse0 = false;
				if (zahara_cody_condition__defaultCondition (player, npc))
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
					
					utils.setScriptVar (player, "conversation.zahara_cody.branchId", 13);
					
					npcSpeak (player, message);
					npcSetConversationResponses (player, responses);
				}
				else
				{
					utils.removeScriptVar (player, "conversation.zahara_cody.branchId");
					
					npcEndConversationWithMessage (player, message);
				}
				
				return SCRIPT_CONTINUE;
			}
			
		}
		
		if (response.equals("s_97"))
		{
			
			if (zahara_cody_condition__defaultCondition (player, npc))
			{
				
				string_id message = new string_id (c_stringFile, "s_99");
				int numberOfResponses = 0;
				
				boolean hasResponse = false;
				
				boolean hasResponse0 = false;
				if (zahara_cody_condition__defaultCondition (player, npc))
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
					
					utils.setScriptVar (player, "conversation.zahara_cody.branchId", 16);
					
					npcSpeak (player, message);
					npcSetConversationResponses (player, responses);
				}
				else
				{
					utils.removeScriptVar (player, "conversation.zahara_cody.branchId");
					
					npcEndConversationWithMessage (player, message);
				}
				
				return SCRIPT_CONTINUE;
			}
			
		}
		
		if (response.equals("s_105"))
		{
			
			if (zahara_cody_condition__defaultCondition (player, npc))
			{
				
				string_id message = new string_id (c_stringFile, "s_107");
				int numberOfResponses = 0;
				
				boolean hasResponse = false;
				
				boolean hasResponse0 = false;
				if (zahara_cody_condition__defaultCondition (player, npc))
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
					
					utils.setScriptVar (player, "conversation.zahara_cody.branchId", 18);
					
					npcSpeak (player, message);
					npcSetConversationResponses (player, responses);
				}
				else
				{
					utils.removeScriptVar (player, "conversation.zahara_cody.branchId");
					
					npcEndConversationWithMessage (player, message);
				}
				
				return SCRIPT_CONTINUE;
			}
			
		}
		
		if (response.equals("s_169"))
		{
			
			if (zahara_cody_condition__defaultCondition (player, npc))
			{
				
				string_id message = new string_id (c_stringFile, "s_173");
				int numberOfResponses = 0;
				
				boolean hasResponse = false;
				
				boolean hasResponse0 = false;
				if (zahara_cody_condition__defaultCondition (player, npc))
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
						responses[responseIndex++] = new string_id (c_stringFile, "s_177");
					}
					
					utils.setScriptVar (player, "conversation.zahara_cody.branchId", 21);
					
					npcSpeak (player, message);
					npcSetConversationResponses (player, responses);
				}
				else
				{
					utils.removeScriptVar (player, "conversation.zahara_cody.branchId");
					
					npcEndConversationWithMessage (player, message);
				}
				
				return SCRIPT_CONTINUE;
			}
			
		}
		
		if (response.equals("s_207"))
		{
			
			if (zahara_cody_condition__defaultCondition (player, npc))
			{
				doAnimationAction (npc, "pet_creature_medium");
				
				zahara_cody_action_giveAntiVirus (player, npc);
				
				string_id message = new string_id (c_stringFile, "s_208");
				utils.removeScriptVar (player, "conversation.zahara_cody.branchId");
				
				npcEndConversationWithMessage (player, message);
				
				return SCRIPT_CONTINUE;
			}
			
		}
		
		return SCRIPT_DEFAULT;
	}
	
	
	public int zahara_cody_handleBranch13(obj_id player, obj_id npc, string_id response) throws InterruptedException
	{
		
		if (response.equals("s_81"))
		{
			
			if (zahara_cody_condition__defaultCondition (player, npc))
			{
				
				string_id message = new string_id (c_stringFile, "s_167");
				int numberOfResponses = 0;
				
				boolean hasResponse = false;
				
				boolean hasResponse0 = false;
				if (zahara_cody_condition__defaultCondition (player, npc))
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
						responses[responseIndex++] = new string_id (c_stringFile, "s_168");
					}
					
					utils.setScriptVar (player, "conversation.zahara_cody.branchId", 14);
					
					npcSpeak (player, message);
					npcSetConversationResponses (player, responses);
				}
				else
				{
					utils.removeScriptVar (player, "conversation.zahara_cody.branchId");
					
					npcEndConversationWithMessage (player, message);
				}
				
				return SCRIPT_CONTINUE;
			}
			
		}
		
		return SCRIPT_DEFAULT;
	}
	
	
	public int zahara_cody_handleBranch14(obj_id player, obj_id npc, string_id response) throws InterruptedException
	{
		
		if (response.equals("s_168"))
		{
			
			if (zahara_cody_condition__defaultCondition (player, npc))
			{
				
				string_id message = new string_id (c_stringFile, "s_199");
				int numberOfResponses = 0;
				
				boolean hasResponse = false;
				
				boolean hasResponse0 = false;
				if (zahara_cody_condition__defaultCondition (player, npc))
				{
					++numberOfResponses;
					hasResponse = true;
					hasResponse0 = true;
				}
				
				boolean hasResponse1 = false;
				if (zahara_cody_condition__defaultCondition (player, npc))
				{
					++numberOfResponses;
					hasResponse = true;
					hasResponse1 = true;
				}
				
				boolean hasResponse2 = false;
				if (zahara_cody_condition__defaultCondition (player, npc))
				{
					++numberOfResponses;
					hasResponse = true;
					hasResponse2 = true;
				}
				
				boolean hasResponse3 = false;
				if (zahara_cody_condition__defaultCondition (player, npc))
				{
					++numberOfResponses;
					hasResponse = true;
					hasResponse3 = true;
				}
				
				boolean hasResponse4 = false;
				if (zahara_cody_condition__defaultCondition (player, npc))
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
						responses[responseIndex++] = new string_id (c_stringFile, "s_77");
					}
					
					if (hasResponse1)
					{
						responses[responseIndex++] = new string_id (c_stringFile, "s_97");
					}
					
					if (hasResponse2)
					{
						responses[responseIndex++] = new string_id (c_stringFile, "s_105");
					}
					
					if (hasResponse3)
					{
						responses[responseIndex++] = new string_id (c_stringFile, "s_169");
					}
					
					if (hasResponse4)
					{
						responses[responseIndex++] = new string_id (c_stringFile, "s_207");
					}
					
					utils.setScriptVar (player, "conversation.zahara_cody.branchId", 12);
					
					npcSpeak (player, message);
					npcSetConversationResponses (player, responses);
				}
				else
				{
					utils.removeScriptVar (player, "conversation.zahara_cody.branchId");
					
					npcEndConversationWithMessage (player, message);
				}
				
				return SCRIPT_CONTINUE;
			}
			
		}
		
		return SCRIPT_DEFAULT;
	}
	
	
	public int zahara_cody_handleBranch15(obj_id player, obj_id npc, string_id response) throws InterruptedException
	{
		
		if (response.equals("s_77"))
		{
			
			if (zahara_cody_condition__defaultCondition (player, npc))
			{
				
				string_id message = new string_id (c_stringFile, "s_79");
				int numberOfResponses = 0;
				
				boolean hasResponse = false;
				
				boolean hasResponse0 = false;
				if (zahara_cody_condition__defaultCondition (player, npc))
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
					
					utils.setScriptVar (player, "conversation.zahara_cody.branchId", 13);
					
					npcSpeak (player, message);
					npcSetConversationResponses (player, responses);
				}
				else
				{
					utils.removeScriptVar (player, "conversation.zahara_cody.branchId");
					
					npcEndConversationWithMessage (player, message);
				}
				
				return SCRIPT_CONTINUE;
			}
			
		}
		
		if (response.equals("s_97"))
		{
			
			if (zahara_cody_condition__defaultCondition (player, npc))
			{
				
				string_id message = new string_id (c_stringFile, "s_99");
				int numberOfResponses = 0;
				
				boolean hasResponse = false;
				
				boolean hasResponse0 = false;
				if (zahara_cody_condition__defaultCondition (player, npc))
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
					
					utils.setScriptVar (player, "conversation.zahara_cody.branchId", 16);
					
					npcSpeak (player, message);
					npcSetConversationResponses (player, responses);
				}
				else
				{
					utils.removeScriptVar (player, "conversation.zahara_cody.branchId");
					
					npcEndConversationWithMessage (player, message);
				}
				
				return SCRIPT_CONTINUE;
			}
			
		}
		
		if (response.equals("s_105"))
		{
			
			if (zahara_cody_condition__defaultCondition (player, npc))
			{
				
				string_id message = new string_id (c_stringFile, "s_107");
				int numberOfResponses = 0;
				
				boolean hasResponse = false;
				
				boolean hasResponse0 = false;
				if (zahara_cody_condition__defaultCondition (player, npc))
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
					
					utils.setScriptVar (player, "conversation.zahara_cody.branchId", 18);
					
					npcSpeak (player, message);
					npcSetConversationResponses (player, responses);
				}
				else
				{
					utils.removeScriptVar (player, "conversation.zahara_cody.branchId");
					
					npcEndConversationWithMessage (player, message);
				}
				
				return SCRIPT_CONTINUE;
			}
			
		}
		
		if (response.equals("s_169"))
		{
			
			if (zahara_cody_condition__defaultCondition (player, npc))
			{
				
				string_id message = new string_id (c_stringFile, "s_173");
				int numberOfResponses = 0;
				
				boolean hasResponse = false;
				
				boolean hasResponse0 = false;
				if (zahara_cody_condition__defaultCondition (player, npc))
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
						responses[responseIndex++] = new string_id (c_stringFile, "s_177");
					}
					
					utils.setScriptVar (player, "conversation.zahara_cody.branchId", 21);
					
					npcSpeak (player, message);
					npcSetConversationResponses (player, responses);
				}
				else
				{
					utils.removeScriptVar (player, "conversation.zahara_cody.branchId");
					
					npcEndConversationWithMessage (player, message);
				}
				
				return SCRIPT_CONTINUE;
			}
			
		}
		
		if (response.equals("s_207"))
		{
			
			if (zahara_cody_condition__defaultCondition (player, npc))
			{
				doAnimationAction (npc, "pet_creature_medium");
				
				zahara_cody_action_giveAntiVirus (player, npc);
				
				string_id message = new string_id (c_stringFile, "s_208");
				utils.removeScriptVar (player, "conversation.zahara_cody.branchId");
				
				npcEndConversationWithMessage (player, message);
				
				return SCRIPT_CONTINUE;
			}
			
		}
		
		return SCRIPT_DEFAULT;
	}
	
	
	public int zahara_cody_handleBranch16(obj_id player, obj_id npc, string_id response) throws InterruptedException
	{
		
		if (response.equals("s_101"))
		{
			
			if (zahara_cody_condition__defaultCondition (player, npc))
			{
				
				string_id message = new string_id (c_stringFile, "s_103");
				int numberOfResponses = 0;
				
				boolean hasResponse = false;
				
				boolean hasResponse0 = false;
				if (zahara_cody_condition__defaultCondition (player, npc))
				{
					++numberOfResponses;
					hasResponse = true;
					hasResponse0 = true;
				}
				
				boolean hasResponse1 = false;
				if (zahara_cody_condition__defaultCondition (player, npc))
				{
					++numberOfResponses;
					hasResponse = true;
					hasResponse1 = true;
				}
				
				boolean hasResponse2 = false;
				if (zahara_cody_condition__defaultCondition (player, npc))
				{
					++numberOfResponses;
					hasResponse = true;
					hasResponse2 = true;
				}
				
				boolean hasResponse3 = false;
				if (zahara_cody_condition__defaultCondition (player, npc))
				{
					++numberOfResponses;
					hasResponse = true;
					hasResponse3 = true;
				}
				
				boolean hasResponse4 = false;
				if (zahara_cody_condition__defaultCondition (player, npc))
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
						responses[responseIndex++] = new string_id (c_stringFile, "s_77");
					}
					
					if (hasResponse1)
					{
						responses[responseIndex++] = new string_id (c_stringFile, "s_97");
					}
					
					if (hasResponse2)
					{
						responses[responseIndex++] = new string_id (c_stringFile, "s_105");
					}
					
					if (hasResponse3)
					{
						responses[responseIndex++] = new string_id (c_stringFile, "s_169");
					}
					
					if (hasResponse4)
					{
						responses[responseIndex++] = new string_id (c_stringFile, "s_207");
					}
					
					utils.setScriptVar (player, "conversation.zahara_cody.branchId", 12);
					
					npcSpeak (player, message);
					npcSetConversationResponses (player, responses);
				}
				else
				{
					utils.removeScriptVar (player, "conversation.zahara_cody.branchId");
					
					npcEndConversationWithMessage (player, message);
				}
				
				return SCRIPT_CONTINUE;
			}
			
		}
		
		return SCRIPT_DEFAULT;
	}
	
	
	public int zahara_cody_handleBranch17(obj_id player, obj_id npc, string_id response) throws InterruptedException
	{
		
		if (response.equals("s_77"))
		{
			
			if (zahara_cody_condition__defaultCondition (player, npc))
			{
				
				string_id message = new string_id (c_stringFile, "s_79");
				int numberOfResponses = 0;
				
				boolean hasResponse = false;
				
				boolean hasResponse0 = false;
				if (zahara_cody_condition__defaultCondition (player, npc))
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
					
					utils.setScriptVar (player, "conversation.zahara_cody.branchId", 13);
					
					npcSpeak (player, message);
					npcSetConversationResponses (player, responses);
				}
				else
				{
					utils.removeScriptVar (player, "conversation.zahara_cody.branchId");
					
					npcEndConversationWithMessage (player, message);
				}
				
				return SCRIPT_CONTINUE;
			}
			
		}
		
		if (response.equals("s_97"))
		{
			
			if (zahara_cody_condition__defaultCondition (player, npc))
			{
				
				string_id message = new string_id (c_stringFile, "s_99");
				int numberOfResponses = 0;
				
				boolean hasResponse = false;
				
				boolean hasResponse0 = false;
				if (zahara_cody_condition__defaultCondition (player, npc))
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
					
					utils.setScriptVar (player, "conversation.zahara_cody.branchId", 16);
					
					npcSpeak (player, message);
					npcSetConversationResponses (player, responses);
				}
				else
				{
					utils.removeScriptVar (player, "conversation.zahara_cody.branchId");
					
					npcEndConversationWithMessage (player, message);
				}
				
				return SCRIPT_CONTINUE;
			}
			
		}
		
		if (response.equals("s_105"))
		{
			
			if (zahara_cody_condition__defaultCondition (player, npc))
			{
				
				string_id message = new string_id (c_stringFile, "s_107");
				int numberOfResponses = 0;
				
				boolean hasResponse = false;
				
				boolean hasResponse0 = false;
				if (zahara_cody_condition__defaultCondition (player, npc))
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
					
					utils.setScriptVar (player, "conversation.zahara_cody.branchId", 18);
					
					npcSpeak (player, message);
					npcSetConversationResponses (player, responses);
				}
				else
				{
					utils.removeScriptVar (player, "conversation.zahara_cody.branchId");
					
					npcEndConversationWithMessage (player, message);
				}
				
				return SCRIPT_CONTINUE;
			}
			
		}
		
		if (response.equals("s_169"))
		{
			
			if (zahara_cody_condition__defaultCondition (player, npc))
			{
				
				string_id message = new string_id (c_stringFile, "s_173");
				int numberOfResponses = 0;
				
				boolean hasResponse = false;
				
				boolean hasResponse0 = false;
				if (zahara_cody_condition__defaultCondition (player, npc))
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
						responses[responseIndex++] = new string_id (c_stringFile, "s_177");
					}
					
					utils.setScriptVar (player, "conversation.zahara_cody.branchId", 21);
					
					npcSpeak (player, message);
					npcSetConversationResponses (player, responses);
				}
				else
				{
					utils.removeScriptVar (player, "conversation.zahara_cody.branchId");
					
					npcEndConversationWithMessage (player, message);
				}
				
				return SCRIPT_CONTINUE;
			}
			
		}
		
		if (response.equals("s_207"))
		{
			
			if (zahara_cody_condition__defaultCondition (player, npc))
			{
				doAnimationAction (npc, "pet_creature_medium");
				
				zahara_cody_action_giveAntiVirus (player, npc);
				
				string_id message = new string_id (c_stringFile, "s_208");
				utils.removeScriptVar (player, "conversation.zahara_cody.branchId");
				
				npcEndConversationWithMessage (player, message);
				
				return SCRIPT_CONTINUE;
			}
			
		}
		
		return SCRIPT_DEFAULT;
	}
	
	
	public int zahara_cody_handleBranch18(obj_id player, obj_id npc, string_id response) throws InterruptedException
	{
		
		if (response.equals("s_109"))
		{
			
			if (zahara_cody_condition__defaultCondition (player, npc))
			{
				
				string_id message = new string_id (c_stringFile, "s_111");
				int numberOfResponses = 0;
				
				boolean hasResponse = false;
				
				boolean hasResponse0 = false;
				if (zahara_cody_condition__defaultCondition (player, npc))
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
					
					utils.setScriptVar (player, "conversation.zahara_cody.branchId", 19);
					
					npcSpeak (player, message);
					npcSetConversationResponses (player, responses);
				}
				else
				{
					utils.removeScriptVar (player, "conversation.zahara_cody.branchId");
					
					npcEndConversationWithMessage (player, message);
				}
				
				return SCRIPT_CONTINUE;
			}
			
		}
		
		return SCRIPT_DEFAULT;
	}
	
	
	public int zahara_cody_handleBranch19(obj_id player, obj_id npc, string_id response) throws InterruptedException
	{
		
		if (response.equals("s_113"))
		{
			
			if (zahara_cody_condition__defaultCondition (player, npc))
			{
				
				string_id message = new string_id (c_stringFile, "s_115");
				int numberOfResponses = 0;
				
				boolean hasResponse = false;
				
				boolean hasResponse0 = false;
				if (zahara_cody_condition__defaultCondition (player, npc))
				{
					++numberOfResponses;
					hasResponse = true;
					hasResponse0 = true;
				}
				
				boolean hasResponse1 = false;
				if (zahara_cody_condition__defaultCondition (player, npc))
				{
					++numberOfResponses;
					hasResponse = true;
					hasResponse1 = true;
				}
				
				boolean hasResponse2 = false;
				if (zahara_cody_condition__defaultCondition (player, npc))
				{
					++numberOfResponses;
					hasResponse = true;
					hasResponse2 = true;
				}
				
				boolean hasResponse3 = false;
				if (zahara_cody_condition__defaultCondition (player, npc))
				{
					++numberOfResponses;
					hasResponse = true;
					hasResponse3 = true;
				}
				
				boolean hasResponse4 = false;
				if (zahara_cody_condition__defaultCondition (player, npc))
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
						responses[responseIndex++] = new string_id (c_stringFile, "s_77");
					}
					
					if (hasResponse1)
					{
						responses[responseIndex++] = new string_id (c_stringFile, "s_97");
					}
					
					if (hasResponse2)
					{
						responses[responseIndex++] = new string_id (c_stringFile, "s_105");
					}
					
					if (hasResponse3)
					{
						responses[responseIndex++] = new string_id (c_stringFile, "s_169");
					}
					
					if (hasResponse4)
					{
						responses[responseIndex++] = new string_id (c_stringFile, "s_207");
					}
					
					utils.setScriptVar (player, "conversation.zahara_cody.branchId", 12);
					
					npcSpeak (player, message);
					npcSetConversationResponses (player, responses);
				}
				else
				{
					utils.removeScriptVar (player, "conversation.zahara_cody.branchId");
					
					npcEndConversationWithMessage (player, message);
				}
				
				return SCRIPT_CONTINUE;
			}
			
		}
		
		return SCRIPT_DEFAULT;
	}
	
	
	public int zahara_cody_handleBranch20(obj_id player, obj_id npc, string_id response) throws InterruptedException
	{
		
		if (response.equals("s_77"))
		{
			
			if (zahara_cody_condition__defaultCondition (player, npc))
			{
				
				string_id message = new string_id (c_stringFile, "s_79");
				int numberOfResponses = 0;
				
				boolean hasResponse = false;
				
				boolean hasResponse0 = false;
				if (zahara_cody_condition__defaultCondition (player, npc))
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
					
					utils.setScriptVar (player, "conversation.zahara_cody.branchId", 13);
					
					npcSpeak (player, message);
					npcSetConversationResponses (player, responses);
				}
				else
				{
					utils.removeScriptVar (player, "conversation.zahara_cody.branchId");
					
					npcEndConversationWithMessage (player, message);
				}
				
				return SCRIPT_CONTINUE;
			}
			
		}
		
		if (response.equals("s_97"))
		{
			
			if (zahara_cody_condition__defaultCondition (player, npc))
			{
				
				string_id message = new string_id (c_stringFile, "s_99");
				int numberOfResponses = 0;
				
				boolean hasResponse = false;
				
				boolean hasResponse0 = false;
				if (zahara_cody_condition__defaultCondition (player, npc))
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
					
					utils.setScriptVar (player, "conversation.zahara_cody.branchId", 16);
					
					npcSpeak (player, message);
					npcSetConversationResponses (player, responses);
				}
				else
				{
					utils.removeScriptVar (player, "conversation.zahara_cody.branchId");
					
					npcEndConversationWithMessage (player, message);
				}
				
				return SCRIPT_CONTINUE;
			}
			
		}
		
		if (response.equals("s_105"))
		{
			
			if (zahara_cody_condition__defaultCondition (player, npc))
			{
				
				string_id message = new string_id (c_stringFile, "s_107");
				int numberOfResponses = 0;
				
				boolean hasResponse = false;
				
				boolean hasResponse0 = false;
				if (zahara_cody_condition__defaultCondition (player, npc))
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
					
					utils.setScriptVar (player, "conversation.zahara_cody.branchId", 18);
					
					npcSpeak (player, message);
					npcSetConversationResponses (player, responses);
				}
				else
				{
					utils.removeScriptVar (player, "conversation.zahara_cody.branchId");
					
					npcEndConversationWithMessage (player, message);
				}
				
				return SCRIPT_CONTINUE;
			}
			
		}
		
		if (response.equals("s_169"))
		{
			
			if (zahara_cody_condition__defaultCondition (player, npc))
			{
				
				string_id message = new string_id (c_stringFile, "s_173");
				int numberOfResponses = 0;
				
				boolean hasResponse = false;
				
				boolean hasResponse0 = false;
				if (zahara_cody_condition__defaultCondition (player, npc))
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
						responses[responseIndex++] = new string_id (c_stringFile, "s_177");
					}
					
					utils.setScriptVar (player, "conversation.zahara_cody.branchId", 21);
					
					npcSpeak (player, message);
					npcSetConversationResponses (player, responses);
				}
				else
				{
					utils.removeScriptVar (player, "conversation.zahara_cody.branchId");
					
					npcEndConversationWithMessage (player, message);
				}
				
				return SCRIPT_CONTINUE;
			}
			
		}
		
		if (response.equals("s_207"))
		{
			
			if (zahara_cody_condition__defaultCondition (player, npc))
			{
				doAnimationAction (npc, "pet_creature_medium");
				
				zahara_cody_action_giveAntiVirus (player, npc);
				
				string_id message = new string_id (c_stringFile, "s_208");
				utils.removeScriptVar (player, "conversation.zahara_cody.branchId");
				
				npcEndConversationWithMessage (player, message);
				
				return SCRIPT_CONTINUE;
			}
			
		}
		
		return SCRIPT_DEFAULT;
	}
	
	
	public int zahara_cody_handleBranch21(obj_id player, obj_id npc, string_id response) throws InterruptedException
	{
		
		if (response.equals("s_177"))
		{
			
			if (zahara_cody_condition__defaultCondition (player, npc))
			{
				
				string_id message = new string_id (c_stringFile, "s_181");
				int numberOfResponses = 0;
				
				boolean hasResponse = false;
				
				boolean hasResponse0 = false;
				if (zahara_cody_condition__defaultCondition (player, npc))
				{
					++numberOfResponses;
					hasResponse = true;
					hasResponse0 = true;
				}
				
				boolean hasResponse1 = false;
				if (zahara_cody_condition__defaultCondition (player, npc))
				{
					++numberOfResponses;
					hasResponse = true;
					hasResponse1 = true;
				}
				
				boolean hasResponse2 = false;
				if (zahara_cody_condition__defaultCondition (player, npc))
				{
					++numberOfResponses;
					hasResponse = true;
					hasResponse2 = true;
				}
				
				boolean hasResponse3 = false;
				if (zahara_cody_condition__defaultCondition (player, npc))
				{
					++numberOfResponses;
					hasResponse = true;
					hasResponse3 = true;
				}
				
				boolean hasResponse4 = false;
				if (zahara_cody_condition__defaultCondition (player, npc))
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
						responses[responseIndex++] = new string_id (c_stringFile, "s_77");
					}
					
					if (hasResponse1)
					{
						responses[responseIndex++] = new string_id (c_stringFile, "s_97");
					}
					
					if (hasResponse2)
					{
						responses[responseIndex++] = new string_id (c_stringFile, "s_105");
					}
					
					if (hasResponse3)
					{
						responses[responseIndex++] = new string_id (c_stringFile, "s_169");
					}
					
					if (hasResponse4)
					{
						responses[responseIndex++] = new string_id (c_stringFile, "s_207");
					}
					
					utils.setScriptVar (player, "conversation.zahara_cody.branchId", 12);
					
					npcSpeak (player, message);
					npcSetConversationResponses (player, responses);
				}
				else
				{
					utils.removeScriptVar (player, "conversation.zahara_cody.branchId");
					
					npcEndConversationWithMessage (player, message);
				}
				
				return SCRIPT_CONTINUE;
			}
			
		}
		
		return SCRIPT_DEFAULT;
	}
	
	
	public int zahara_cody_handleBranch22(obj_id player, obj_id npc, string_id response) throws InterruptedException
	{
		
		if (response.equals("s_77"))
		{
			
			if (zahara_cody_condition__defaultCondition (player, npc))
			{
				
				string_id message = new string_id (c_stringFile, "s_79");
				int numberOfResponses = 0;
				
				boolean hasResponse = false;
				
				boolean hasResponse0 = false;
				if (zahara_cody_condition__defaultCondition (player, npc))
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
					
					utils.setScriptVar (player, "conversation.zahara_cody.branchId", 13);
					
					npcSpeak (player, message);
					npcSetConversationResponses (player, responses);
				}
				else
				{
					utils.removeScriptVar (player, "conversation.zahara_cody.branchId");
					
					npcEndConversationWithMessage (player, message);
				}
				
				return SCRIPT_CONTINUE;
			}
			
		}
		
		if (response.equals("s_97"))
		{
			
			if (zahara_cody_condition__defaultCondition (player, npc))
			{
				
				string_id message = new string_id (c_stringFile, "s_99");
				int numberOfResponses = 0;
				
				boolean hasResponse = false;
				
				boolean hasResponse0 = false;
				if (zahara_cody_condition__defaultCondition (player, npc))
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
					
					utils.setScriptVar (player, "conversation.zahara_cody.branchId", 16);
					
					npcSpeak (player, message);
					npcSetConversationResponses (player, responses);
				}
				else
				{
					utils.removeScriptVar (player, "conversation.zahara_cody.branchId");
					
					npcEndConversationWithMessage (player, message);
				}
				
				return SCRIPT_CONTINUE;
			}
			
		}
		
		if (response.equals("s_105"))
		{
			
			if (zahara_cody_condition__defaultCondition (player, npc))
			{
				
				string_id message = new string_id (c_stringFile, "s_107");
				int numberOfResponses = 0;
				
				boolean hasResponse = false;
				
				boolean hasResponse0 = false;
				if (zahara_cody_condition__defaultCondition (player, npc))
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
					
					utils.setScriptVar (player, "conversation.zahara_cody.branchId", 18);
					
					npcSpeak (player, message);
					npcSetConversationResponses (player, responses);
				}
				else
				{
					utils.removeScriptVar (player, "conversation.zahara_cody.branchId");
					
					npcEndConversationWithMessage (player, message);
				}
				
				return SCRIPT_CONTINUE;
			}
			
		}
		
		if (response.equals("s_169"))
		{
			
			if (zahara_cody_condition__defaultCondition (player, npc))
			{
				
				string_id message = new string_id (c_stringFile, "s_173");
				int numberOfResponses = 0;
				
				boolean hasResponse = false;
				
				boolean hasResponse0 = false;
				if (zahara_cody_condition__defaultCondition (player, npc))
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
						responses[responseIndex++] = new string_id (c_stringFile, "s_177");
					}
					
					utils.setScriptVar (player, "conversation.zahara_cody.branchId", 21);
					
					npcSpeak (player, message);
					npcSetConversationResponses (player, responses);
				}
				else
				{
					utils.removeScriptVar (player, "conversation.zahara_cody.branchId");
					
					npcEndConversationWithMessage (player, message);
				}
				
				return SCRIPT_CONTINUE;
			}
			
		}
		
		if (response.equals("s_207"))
		{
			
			if (zahara_cody_condition__defaultCondition (player, npc))
			{
				doAnimationAction (npc, "pet_creature_medium");
				
				zahara_cody_action_giveAntiVirus (player, npc);
				
				string_id message = new string_id (c_stringFile, "s_208");
				utils.removeScriptVar (player, "conversation.zahara_cody.branchId");
				
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
			detachScript(self, "conversation.zahara_cody");
		}
		
		setCondition (self, CONDITION_CONVERSABLE);
		messageTo(self, "findNpc", null, 2, false);
		CustomerServiceLog("outbreak_themepark", "outbreak_zahara_cody conversation: Cody is looking for Trig. Self: "+self);
		return SCRIPT_CONTINUE;
	}
	
	
	public int OnAttach(obj_id self) throws InterruptedException
	{
		setCondition (self, CONDITION_CONVERSABLE);
		messageTo(self, "findNpc", null, 2, false);
		CustomerServiceLog("outbreak_themepark", "outbreak_zahara_cody conversation: Cody is looking for Trig. Self: "+self);
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
		detachScript (self, "conversation.zahara_cody");
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int findNpc(obj_id self, dictionary params) throws InterruptedException
	{
		CustomerServiceLog("outbreak_themepark", "outbreak_zahara_cody findNpc(): Initialized");
		obj_id[] npcList = getAllObjectsWithObjVar(getLocation(self), 3.f, "trig");
		if (npcList == null || npcList.length <= 0)
		{
			CustomerServiceLog("outbreak_themepark", "outbreak_zahara_cody findNpc(): Failed to find NPC var on any objects within range.");
			messageTo(self, "findNpc", null, 60, false);
			return SCRIPT_CONTINUE;
		}
		CustomerServiceLog("outbreak_themepark", "outbreak_han_solo findNpc(): Found NPC within range.");
		setObjVar(self, "myTrig", npcList[0]);
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
		
		if (zahara_cody_condition_hasCompletedQuestLine (player, npc))
		{
			
			string_id message = new string_id (c_stringFile, "s_212");
			chat.chat (npc, player, message);
			
			return SCRIPT_CONTINUE;
		}
		
		if (zahara_cody_condition_hasDeliveredRadioEpsilon (player, npc))
		{
			
			string_id message = new string_id (c_stringFile, "s_210");
			prose_package pp = new prose_package ();
			pp.stringId = message;
			pp.actor.set (player);
			pp.target.set (npc);
			
			chat.chat (npc, player, null, null, pp);
			return SCRIPT_CONTINUE;
		}
		
		if (zahara_cody_condition_hasFoundEpsilon (player, npc))
		{
			
			string_id message = new string_id (c_stringFile, "s_209");
			chat.chat (npc, player, message);
			
			return SCRIPT_CONTINUE;
		}
		
		if (zahara_cody_condition_hasDeletedQuest01b (player, npc))
		{
			zahara_cody_action_giveQuest1b (player, npc);
			
			string_id message = new string_id (c_stringFile, "s_213");
			chat.chat (npc, player, message);
			
			return SCRIPT_CONTINUE;
		}
		
		if (zahara_cody_condition_hasCompletedQuest01 (player, npc))
		{
			
			string_id message = new string_id (c_stringFile, "s_211");
			chat.chat (npc, player, message);
			
			return SCRIPT_CONTINUE;
		}
		
		if (zahara_cody_condition_hasTrigTask (player, npc))
		{
			
			string_id message = new string_id (c_stringFile, "s_59");
			chat.chat (npc, player, message);
			
			return SCRIPT_CONTINUE;
		}
		
		if (zahara_cody_condition_isImperialNotImmunized (player, npc))
		{
			doAnimationAction (npc, "nod");
			
			string_id message = new string_id (c_stringFile, "s_10");
			int numberOfResponses = 0;
			
			boolean hasResponse = false;
			
			boolean hasResponse0 = false;
			if (zahara_cody_condition__defaultCondition (player, npc))
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
					responses[responseIndex++] = new string_id (c_stringFile, "s_12");
				}
				
				utils.setScriptVar (player, "conversation.zahara_cody.branchId", 7);
				
				npcStartConversation (player, npc, "zahara_cody", message, responses);
			}
			else
			{
				chat.chat (npc, player, message);
			}
			
			return SCRIPT_CONTINUE;
		}
		
		if (zahara_cody_condition__defaultCondition (player, npc))
		{
			
			string_id message = new string_id (c_stringFile, "s_73");
			chat.chat (npc, player, message);
			
			return SCRIPT_CONTINUE;
		}
		
		chat.chat (npc, "Error: All conditions for OnStartNpcConversation were false.");
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int OnNpcConversationResponse(obj_id self, String conversationId, obj_id player, string_id response) throws InterruptedException
	{
		if (!conversationId.equals("zahara_cody"))
		{
			return SCRIPT_CONTINUE;
		}
		
		obj_id npc = self;
		
		int branchId = utils.getIntScriptVar (player, "conversation.zahara_cody.branchId");
		
		if (branchId == 7 && zahara_cody_handleBranch7 (player, npc, response) == SCRIPT_CONTINUE)
		{
			return SCRIPT_CONTINUE;
		}
		
		if (branchId == 8 && zahara_cody_handleBranch8 (player, npc, response) == SCRIPT_CONTINUE)
		{
			return SCRIPT_CONTINUE;
		}
		
		if (branchId == 9 && zahara_cody_handleBranch9 (player, npc, response) == SCRIPT_CONTINUE)
		{
			return SCRIPT_CONTINUE;
		}
		
		if (branchId == 10 && zahara_cody_handleBranch10 (player, npc, response) == SCRIPT_CONTINUE)
		{
			return SCRIPT_CONTINUE;
		}
		
		if (branchId == 11 && zahara_cody_handleBranch11 (player, npc, response) == SCRIPT_CONTINUE)
		{
			return SCRIPT_CONTINUE;
		}
		
		if (branchId == 12 && zahara_cody_handleBranch12 (player, npc, response) == SCRIPT_CONTINUE)
		{
			return SCRIPT_CONTINUE;
		}
		
		if (branchId == 13 && zahara_cody_handleBranch13 (player, npc, response) == SCRIPT_CONTINUE)
		{
			return SCRIPT_CONTINUE;
		}
		
		if (branchId == 14 && zahara_cody_handleBranch14 (player, npc, response) == SCRIPT_CONTINUE)
		{
			return SCRIPT_CONTINUE;
		}
		
		if (branchId == 15 && zahara_cody_handleBranch15 (player, npc, response) == SCRIPT_CONTINUE)
		{
			return SCRIPT_CONTINUE;
		}
		
		if (branchId == 16 && zahara_cody_handleBranch16 (player, npc, response) == SCRIPT_CONTINUE)
		{
			return SCRIPT_CONTINUE;
		}
		
		if (branchId == 17 && zahara_cody_handleBranch17 (player, npc, response) == SCRIPT_CONTINUE)
		{
			return SCRIPT_CONTINUE;
		}
		
		if (branchId == 18 && zahara_cody_handleBranch18 (player, npc, response) == SCRIPT_CONTINUE)
		{
			return SCRIPT_CONTINUE;
		}
		
		if (branchId == 19 && zahara_cody_handleBranch19 (player, npc, response) == SCRIPT_CONTINUE)
		{
			return SCRIPT_CONTINUE;
		}
		
		if (branchId == 20 && zahara_cody_handleBranch20 (player, npc, response) == SCRIPT_CONTINUE)
		{
			return SCRIPT_CONTINUE;
		}
		
		if (branchId == 21 && zahara_cody_handleBranch21 (player, npc, response) == SCRIPT_CONTINUE)
		{
			return SCRIPT_CONTINUE;
		}
		
		if (branchId == 22 && zahara_cody_handleBranch22 (player, npc, response) == SCRIPT_CONTINUE)
		{
			return SCRIPT_CONTINUE;
		}
		
		chat.chat (npc, "Error: Fell through all branches and responses for OnNpcConversationResponse.");
		
		utils.removeScriptVar (player, "conversation.zahara_cody.branchId");
		
		return SCRIPT_CONTINUE;
	}
	
}
