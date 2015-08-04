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


public class water_treatment extends script.base_script
{
	public water_treatment()
	{
	}
	String c_stringFile = "conversation/water_treatment";
	
	
	public boolean water_treatment_condition__defaultCondition(obj_id player, obj_id npc) throws InterruptedException
	{
		return true;
	}
	
	
	public boolean water_treatment_condition_hasInitialQuest(obj_id player, obj_id npc) throws InterruptedException
	{
		faceTo(npc,player);
		return groundquests.isTaskActive(player, "u16_nym_meet_townspeople", "findFiltrationGuy") || (groundquests.hasCompletedQuest(player, "u16_nym_meet_townspeople") && !groundquests.isQuestActiveOrComplete(player, "u16_nym_themepark_parts_1"));
	}
	
	
	public boolean water_treatment_condition_isFinishingFirstQuest(obj_id player, obj_id npc) throws InterruptedException
	{
		faceTo(npc,player);
		return groundquests.isTaskActive(player, "u16_nym_themepark_repair_mixing_chamber", "returnChemical") || (groundquests.hasCompletedQuest(player, "u16_nym_themepark_repair_mixing_chamber") && !groundquests.isQuestActiveOrComplete(player, "u16_nym_themepark_parts_2"));
	}
	
	
	public boolean water_treatment_condition_isFinishingSecondQuest(obj_id player, obj_id npc) throws InterruptedException
	{
		faceTo(npc,player);
		return groundquests.isTaskActive(player, "u16_nym_themepark_under_pressure", "returnValves") || (groundquests.hasCompletedQuest(player, "u16_nym_themepark_under_pressure") && !groundquests.isQuestActiveOrComplete(player, "u16_nym_themepark_parts_3"));
	}
	
	
	public boolean water_treatment_condition_isFinishingThirdQuest(obj_id player, obj_id npc) throws InterruptedException
	{
		faceTo(npc,player);
		return groundquests.isTaskActive(player, "u16_nym_themepark_repair_flocculation_chamber", "returnRegulator") || (groundquests.hasCompletedQuest(player, "u16_nym_themepark_repair_flocculation_chamber") && !groundquests.isQuestActiveOrComplete(player, "u16_nym_return_townsperson"));
	}
	
	
	public boolean water_treatment_condition_hasFinishedAllQuests(obj_id player, obj_id npc) throws InterruptedException
	{
		faceTo(npc,player);
		return groundquests.isQuestActiveOrComplete(player, "u16_nym_return_townsperson");
		
	}
	
	
	public boolean water_treatment_condition_hasFirstQuest(obj_id player, obj_id npc) throws InterruptedException
	{
		faceTo(npc,player);
		return groundquests.isTaskActive(player, "u16_nym_themepark_parts_1", "findQuartermaster") || (groundquests.hasCompletedQuest(player, "u16_nym_themepark_parts_1") && !groundquests.isQuestActiveOrComplete(player, "u16_nym_themepark_repair_mixing_chamber"));
	}
	
	
	public boolean water_treatment_condition_hasSecondQuest(obj_id player, obj_id npc) throws InterruptedException
	{
		faceTo(npc,player);
		return groundquests.isTaskActive(player, "u16_nym_themepark_parts_2", "goQuartermaster") || (groundquests.hasCompletedQuest(player, "u16_nym_themepark_parts_2") && !groundquests.isQuestActiveOrComplete(player, "u16_nym_themepark_under_pressure"));
	}
	
	
	public boolean water_treatment_condition_hasThirdQuest(obj_id player, obj_id npc) throws InterruptedException
	{
		faceTo(npc,player);
		return groundquests.isTaskActive(player, "u16_nym_themepark_parts_3", "goQuartermasterAgain") || (groundquests.hasCompletedQuest(player, "u16_nym_themepark_parts_3") && !groundquests.isQuestActiveOrComplete(player, "u16_nym_themepark_save_agent"));
	}
	
	
	public boolean water_treatment_condition_hasNotFixedMixer(obj_id player, obj_id npc) throws InterruptedException
	{
		faceTo(npc,player);
		return groundquests.isTaskActive(player, "u16_nym_themepark_repair_mixing_chamber", "fixMixingChamber");
	}
	
	
	public boolean water_treatment_condition_hasNotFixedSediment(obj_id player, obj_id npc) throws InterruptedException
	{
		faceTo(npc,player);
		return groundquests.isTaskActive(player, "u16_nym_themepark_under_pressure", "fixSedimentation");
	}
	
	
	public boolean water_treatment_condition_hasNotFixedFlocculation(obj_id player, obj_id npc) throws InterruptedException
	{
		faceTo(npc,player);
		return groundquests.isTaskActive(player, "u16_nym_themepark_repair_flocculation_chamber", "fixFlocculation");
	}
	
	
	public void water_treatment_action_signalMeetingFirstTime(obj_id player, obj_id npc) throws InterruptedException
	{
		groundquests.sendSignal(player, "hasFoundFiltrationGuy");
	}
	
	
	public void water_treatment_action_giveFirstQuest(obj_id player, obj_id npc) throws InterruptedException
	{
		groundquests.grantQuest(player, "u16_nym_themepark_parts_1");
	}
	
	
	public void water_treatment_action_giveSecondQuest(obj_id player, obj_id npc) throws InterruptedException
	{
		groundquests.grantQuest(player, "u16_nym_themepark_parts_2");
	}
	
	
	public void water_treatment_action_giveThirdQuest(obj_id player, obj_id npc) throws InterruptedException
	{
		groundquests.grantQuest(player, "u16_nym_themepark_parts_3");
	}
	
	
	public void water_treatment_action_finishUpMixingChamber(obj_id player, obj_id npc) throws InterruptedException
	{
		groundquests.sendSignal(player, "hasReturnedChemical");
	}
	
	
	public void water_treatment_action_grantReturnTownPerson(obj_id player, obj_id npc) throws InterruptedException
	{
		groundquests.grantQuest(player, "u16_nym_return_townsperson");
	}
	
	
	public void water_treatment_action_finishUpFloc(obj_id player, obj_id npc) throws InterruptedException
	{
		groundquests.sendSignal(player, "hasReturnedRegulator");
	}
	
	
	public void water_treatment_action_finishUpPressure(obj_id player, obj_id npc) throws InterruptedException
	{
		groundquests.sendSignal(player, "hasReturnedValves");
	}
	
	
	public int water_treatment_handleBranch2(obj_id player, obj_id npc, string_id response) throws InterruptedException
	{
		
		if (response.equals("s_18"))
		{
			
			if (water_treatment_condition__defaultCondition (player, npc))
			{
				doAnimationAction (npc, "shrug_shoulders");
				
				string_id message = new string_id (c_stringFile, "s_19");
				int numberOfResponses = 0;
				
				boolean hasResponse = false;
				
				boolean hasResponse0 = false;
				if (water_treatment_condition__defaultCondition (player, npc))
				{
					++numberOfResponses;
					hasResponse = true;
					hasResponse0 = true;
				}
				
				if (hasResponse)
				{
					int responseIndex = 0;
					string_id responses[] = new string_id[numberOfResponses];
					
					if (hasResponse0)
					{
						responses[responseIndex++] = new string_id (c_stringFile, "s_42");
					}
					
					utils.setScriptVar (player, "conversation.water_treatment.branchId", 3);
					
					npcSpeak (player, message);
					npcSetConversationResponses (player, responses);
				}
				else
				{
					utils.removeScriptVar (player, "conversation.water_treatment.branchId");
					
					npcEndConversationWithMessage (player, message);
				}
				
				return SCRIPT_CONTINUE;
			}
			
		}
		
		return SCRIPT_DEFAULT;
	}
	
	
	public int water_treatment_handleBranch3(obj_id player, obj_id npc, string_id response) throws InterruptedException
	{
		
		if (response.equals("s_42"))
		{
			
			if (water_treatment_condition__defaultCondition (player, npc))
			{
				doAnimationAction (npc, "thank");
				
				water_treatment_action_grantReturnTownPerson (player, npc);
				
				string_id message = new string_id (c_stringFile, "s_43");
				utils.removeScriptVar (player, "conversation.water_treatment.branchId");
				
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
	
	
	public int water_treatment_handleBranch6(obj_id player, obj_id npc, string_id response) throws InterruptedException
	{
		
		if (response.equals("s_66"))
		{
			doAnimationAction (player, "standing_placate");
			
			if (water_treatment_condition__defaultCondition (player, npc))
			{
				doAnimationAction (npc, "udaman");
				
				string_id message = new string_id (c_stringFile, "s_67");
				utils.removeScriptVar (player, "conversation.water_treatment.branchId");
				
				npcEndConversationWithMessage (player, message);
				
				return SCRIPT_CONTINUE;
			}
			
		}
		
		return SCRIPT_DEFAULT;
	}
	
	
	public int water_treatment_handleBranch8(obj_id player, obj_id npc, string_id response) throws InterruptedException
	{
		
		if (response.equals("s_16"))
		{
			
			if (water_treatment_condition__defaultCondition (player, npc))
			{
				doAnimationAction (npc, "shake_head_no");
				
				string_id message = new string_id (c_stringFile, "s_21");
				int numberOfResponses = 0;
				
				boolean hasResponse = false;
				
				boolean hasResponse0 = false;
				if (water_treatment_condition__defaultCondition (player, npc))
				{
					++numberOfResponses;
					hasResponse = true;
					hasResponse0 = true;
				}
				
				if (hasResponse)
				{
					int responseIndex = 0;
					string_id responses[] = new string_id[numberOfResponses];
					
					if (hasResponse0)
					{
						responses[responseIndex++] = new string_id (c_stringFile, "s_40");
					}
					
					utils.setScriptVar (player, "conversation.water_treatment.branchId", 9);
					
					npcSpeak (player, message);
					npcSetConversationResponses (player, responses);
				}
				else
				{
					utils.removeScriptVar (player, "conversation.water_treatment.branchId");
					
					npcEndConversationWithMessage (player, message);
				}
				
				return SCRIPT_CONTINUE;
			}
			
		}
		
		return SCRIPT_DEFAULT;
	}
	
	
	public int water_treatment_handleBranch9(obj_id player, obj_id npc, string_id response) throws InterruptedException
	{
		
		if (response.equals("s_40"))
		{
			
			if (water_treatment_condition__defaultCondition (player, npc))
			{
				doAnimationAction (npc, "nod");
				
				water_treatment_action_giveThirdQuest (player, npc);
				
				string_id message = new string_id (c_stringFile, "s_41");
				utils.removeScriptVar (player, "conversation.water_treatment.branchId");
				
				npcEndConversationWithMessage (player, message);
				
				return SCRIPT_CONTINUE;
			}
			
		}
		
		return SCRIPT_DEFAULT;
	}
	
	
	public int water_treatment_handleBranch13(obj_id player, obj_id npc, string_id response) throws InterruptedException
	{
		
		if (response.equals("s_29"))
		{
			
			if (water_treatment_condition__defaultCondition (player, npc))
			{
				doAnimationAction (npc, "shake_head_no");
				
				string_id message = new string_id (c_stringFile, "s_37");
				int numberOfResponses = 0;
				
				boolean hasResponse = false;
				
				boolean hasResponse0 = false;
				if (water_treatment_condition__defaultCondition (player, npc))
				{
					++numberOfResponses;
					hasResponse = true;
					hasResponse0 = true;
				}
				
				if (hasResponse)
				{
					int responseIndex = 0;
					string_id responses[] = new string_id[numberOfResponses];
					
					if (hasResponse0)
					{
						responses[responseIndex++] = new string_id (c_stringFile, "s_38");
					}
					
					utils.setScriptVar (player, "conversation.water_treatment.branchId", 14);
					
					npcSpeak (player, message);
					npcSetConversationResponses (player, responses);
				}
				else
				{
					utils.removeScriptVar (player, "conversation.water_treatment.branchId");
					
					npcEndConversationWithMessage (player, message);
				}
				
				return SCRIPT_CONTINUE;
			}
			
		}
		
		return SCRIPT_DEFAULT;
	}
	
	
	public int water_treatment_handleBranch14(obj_id player, obj_id npc, string_id response) throws InterruptedException
	{
		
		if (response.equals("s_38"))
		{
			
			if (water_treatment_condition__defaultCondition (player, npc))
			{
				doAnimationAction (npc, "nod_head_once");
				
				water_treatment_action_giveSecondQuest (player, npc);
				
				string_id message = new string_id (c_stringFile, "s_39");
				utils.removeScriptVar (player, "conversation.water_treatment.branchId");
				
				npcEndConversationWithMessage (player, message);
				
				return SCRIPT_CONTINUE;
			}
			
		}
		
		return SCRIPT_DEFAULT;
	}
	
	
	public int water_treatment_handleBranch18(obj_id player, obj_id npc, string_id response) throws InterruptedException
	{
		
		if (response.equals("s_45"))
		{
			
			if (water_treatment_condition__defaultCondition (player, npc))
			{
				doAnimationAction (npc, "nod");
				
				string_id message = new string_id (c_stringFile, "s_47");
				int numberOfResponses = 0;
				
				boolean hasResponse = false;
				
				boolean hasResponse0 = false;
				if (water_treatment_condition__defaultCondition (player, npc))
				{
					++numberOfResponses;
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
					
					utils.setScriptVar (player, "conversation.water_treatment.branchId", 19);
					
					npcSpeak (player, message);
					npcSetConversationResponses (player, responses);
				}
				else
				{
					utils.removeScriptVar (player, "conversation.water_treatment.branchId");
					
					npcEndConversationWithMessage (player, message);
				}
				
				return SCRIPT_CONTINUE;
			}
			
		}
		
		return SCRIPT_DEFAULT;
	}
	
	
	public int water_treatment_handleBranch19(obj_id player, obj_id npc, string_id response) throws InterruptedException
	{
		
		if (response.equals("s_49"))
		{
			
			if (water_treatment_condition__defaultCondition (player, npc))
			{
				doAnimationAction (npc, "sigh_deeply");
				
				string_id message = new string_id (c_stringFile, "s_51");
				int numberOfResponses = 0;
				
				boolean hasResponse = false;
				
				boolean hasResponse0 = false;
				if (water_treatment_condition__defaultCondition (player, npc))
				{
					++numberOfResponses;
					hasResponse = true;
					hasResponse0 = true;
				}
				
				if (hasResponse)
				{
					int responseIndex = 0;
					string_id responses[] = new string_id[numberOfResponses];
					
					if (hasResponse0)
					{
						responses[responseIndex++] = new string_id (c_stringFile, "s_53");
					}
					
					utils.setScriptVar (player, "conversation.water_treatment.branchId", 20);
					
					npcSpeak (player, message);
					npcSetConversationResponses (player, responses);
				}
				else
				{
					utils.removeScriptVar (player, "conversation.water_treatment.branchId");
					
					npcEndConversationWithMessage (player, message);
				}
				
				return SCRIPT_CONTINUE;
			}
			
		}
		
		return SCRIPT_DEFAULT;
	}
	
	
	public int water_treatment_handleBranch20(obj_id player, obj_id npc, string_id response) throws InterruptedException
	{
		
		if (response.equals("s_53"))
		{
			
			if (water_treatment_condition__defaultCondition (player, npc))
			{
				doAnimationAction (npc, "slump_head");
				
				string_id message = new string_id (c_stringFile, "s_55");
				int numberOfResponses = 0;
				
				boolean hasResponse = false;
				
				boolean hasResponse0 = false;
				if (water_treatment_condition__defaultCondition (player, npc))
				{
					++numberOfResponses;
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
					
					utils.setScriptVar (player, "conversation.water_treatment.branchId", 21);
					
					npcSpeak (player, message);
					npcSetConversationResponses (player, responses);
				}
				else
				{
					utils.removeScriptVar (player, "conversation.water_treatment.branchId");
					
					npcEndConversationWithMessage (player, message);
				}
				
				return SCRIPT_CONTINUE;
			}
			
		}
		
		return SCRIPT_DEFAULT;
	}
	
	
	public int water_treatment_handleBranch21(obj_id player, obj_id npc, string_id response) throws InterruptedException
	{
		
		if (response.equals("s_59"))
		{
			
			if (water_treatment_condition__defaultCondition (player, npc))
			{
				doAnimationAction (npc, "explain");
				
				string_id message = new string_id (c_stringFile, "s_68");
				int numberOfResponses = 0;
				
				boolean hasResponse = false;
				
				boolean hasResponse0 = false;
				if (water_treatment_condition__defaultCondition (player, npc))
				{
					++numberOfResponses;
					hasResponse = true;
					hasResponse0 = true;
				}
				
				if (hasResponse)
				{
					int responseIndex = 0;
					string_id responses[] = new string_id[numberOfResponses];
					
					if (hasResponse0)
					{
						responses[responseIndex++] = new string_id (c_stringFile, "s_70");
					}
					
					utils.setScriptVar (player, "conversation.water_treatment.branchId", 22);
					
					npcSpeak (player, message);
					npcSetConversationResponses (player, responses);
				}
				else
				{
					utils.removeScriptVar (player, "conversation.water_treatment.branchId");
					
					npcEndConversationWithMessage (player, message);
				}
				
				return SCRIPT_CONTINUE;
			}
			
		}
		
		return SCRIPT_DEFAULT;
	}
	
	
	public int water_treatment_handleBranch22(obj_id player, obj_id npc, string_id response) throws InterruptedException
	{
		
		if (response.equals("s_70"))
		{
			
			if (water_treatment_condition__defaultCondition (player, npc))
			{
				doAnimationAction (npc, "wave1");
				
				water_treatment_action_giveFirstQuest (player, npc);
				
				string_id message = new string_id (c_stringFile, "s_72");
				utils.removeScriptVar (player, "conversation.water_treatment.branchId");
				
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
			detachScript(self, "conversation.water_treatment");
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
		detachScript (self, "conversation.water_treatment");
		
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
		
		if (water_treatment_condition_hasFinishedAllQuests (player, npc))
		{
			doAnimationAction (player, "udaman");
			
			string_id message = new string_id (c_stringFile, "s_57");
			chat.chat (npc, player, message);
			
			return SCRIPT_CONTINUE;
		}
		
		if (water_treatment_condition_isFinishingThirdQuest (player, npc))
		{
			doAnimationAction (npc, "celebrate1");
			
			water_treatment_action_finishUpFloc (player, npc);
			
			string_id message = new string_id (c_stringFile, "s_17");
			int numberOfResponses = 0;
			
			boolean hasResponse = false;
			
			boolean hasResponse0 = false;
			if (water_treatment_condition__defaultCondition (player, npc))
			{
				++numberOfResponses;
				hasResponse = true;
				hasResponse0 = true;
			}
			
			if (hasResponse)
			{
				int responseIndex = 0;
				string_id responses[] = new string_id[numberOfResponses];
				
				if (hasResponse0)
				{
					responses[responseIndex++] = new string_id (c_stringFile, "s_18");
				}
				
				utils.setScriptVar (player, "conversation.water_treatment.branchId", 2);
				
				npcStartConversation (player, npc, "water_treatment", message, responses);
			}
			else
			{
				chat.chat (npc, player, message);
			}
			
			return SCRIPT_CONTINUE;
		}
		
		if (water_treatment_condition_hasThirdQuest (player, npc))
		{
			doAnimationAction (npc, "yawn");
			
			doAnimationAction (player, "explain");
			
			string_id message = new string_id (c_stringFile, "s_62");
			chat.chat (npc, player, message);
			
			return SCRIPT_CONTINUE;
		}
		
		if (water_treatment_condition_hasNotFixedFlocculation (player, npc))
		{
			doAnimationAction (npc, "applause_excited");
			
			string_id message = new string_id (c_stringFile, "s_65");
			int numberOfResponses = 0;
			
			boolean hasResponse = false;
			
			boolean hasResponse0 = false;
			if (water_treatment_condition__defaultCondition (player, npc))
			{
				++numberOfResponses;
				hasResponse = true;
				hasResponse0 = true;
			}
			
			if (hasResponse)
			{
				int responseIndex = 0;
				string_id responses[] = new string_id[numberOfResponses];
				
				if (hasResponse0)
				{
					responses[responseIndex++] = new string_id (c_stringFile, "s_66");
				}
				
				utils.setScriptVar (player, "conversation.water_treatment.branchId", 6);
				
				npcStartConversation (player, npc, "water_treatment", message, responses);
			}
			else
			{
				chat.chat (npc, player, message);
			}
			
			return SCRIPT_CONTINUE;
		}
		
		if (water_treatment_condition_isFinishingSecondQuest (player, npc))
		{
			doAnimationAction (npc, "thank");
			
			water_treatment_action_finishUpPressure (player, npc);
			
			string_id message = new string_id (c_stringFile, "s_14");
			int numberOfResponses = 0;
			
			boolean hasResponse = false;
			
			boolean hasResponse0 = false;
			if (water_treatment_condition__defaultCondition (player, npc))
			{
				++numberOfResponses;
				hasResponse = true;
				hasResponse0 = true;
			}
			
			if (hasResponse)
			{
				int responseIndex = 0;
				string_id responses[] = new string_id[numberOfResponses];
				
				if (hasResponse0)
				{
					responses[responseIndex++] = new string_id (c_stringFile, "s_16");
				}
				
				utils.setScriptVar (player, "conversation.water_treatment.branchId", 8);
				
				npcStartConversation (player, npc, "water_treatment", message, responses);
			}
			else
			{
				chat.chat (npc, player, message);
			}
			
			return SCRIPT_CONTINUE;
		}
		
		if (water_treatment_condition_hasSecondQuest (player, npc))
		{
			doAnimationAction (player, "slump_head");
			
			string_id message = new string_id (c_stringFile, "s_61");
			chat.chat (npc, player, message);
			
			return SCRIPT_CONTINUE;
		}
		
		if (water_treatment_condition_hasNotFixedSediment (player, npc))
		{
			doAnimationAction (npc, "nod_head_multiple");
			
			string_id message = new string_id (c_stringFile, "s_64");
			chat.chat (npc, player, message);
			
			return SCRIPT_CONTINUE;
		}
		
		if (water_treatment_condition_isFinishingFirstQuest (player, npc))
		{
			doAnimationAction (npc, "applause_polite");
			
			water_treatment_action_finishUpMixingChamber (player, npc);
			
			string_id message = new string_id (c_stringFile, "s_27");
			int numberOfResponses = 0;
			
			boolean hasResponse = false;
			
			boolean hasResponse0 = false;
			if (water_treatment_condition__defaultCondition (player, npc))
			{
				++numberOfResponses;
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
				
				utils.setScriptVar (player, "conversation.water_treatment.branchId", 13);
				
				npcStartConversation (player, npc, "water_treatment", message, responses);
			}
			else
			{
				chat.chat (npc, player, message);
			}
			
			return SCRIPT_CONTINUE;
		}
		
		if (water_treatment_condition_hasFirstQuest (player, npc))
		{
			doAnimationAction (npc, "shrug_hands");
			
			string_id message = new string_id (c_stringFile, "s_58");
			chat.chat (npc, player, message);
			
			return SCRIPT_CONTINUE;
		}
		
		if (water_treatment_condition_hasNotFixedMixer (player, npc))
		{
			doAnimationAction (npc, "standing_placate");
			
			string_id message = new string_id (c_stringFile, "s_63");
			chat.chat (npc, player, message);
			
			return SCRIPT_CONTINUE;
		}
		
		if (water_treatment_condition_hasInitialQuest (player, npc))
		{
			doAnimationAction (npc, "gesticulate_wildly");
			
			water_treatment_action_signalMeetingFirstTime (player, npc);
			
			string_id message = new string_id (c_stringFile, "s_36");
			int numberOfResponses = 0;
			
			boolean hasResponse = false;
			
			boolean hasResponse0 = false;
			if (water_treatment_condition__defaultCondition (player, npc))
			{
				++numberOfResponses;
				hasResponse = true;
				hasResponse0 = true;
			}
			
			if (hasResponse)
			{
				int responseIndex = 0;
				string_id responses[] = new string_id[numberOfResponses];
				
				if (hasResponse0)
				{
					responses[responseIndex++] = new string_id (c_stringFile, "s_45");
				}
				
				utils.setScriptVar (player, "conversation.water_treatment.branchId", 18);
				
				npcStartConversation (player, npc, "water_treatment", message, responses);
			}
			else
			{
				chat.chat (npc, player, message);
			}
			
			return SCRIPT_CONTINUE;
		}
		
		if (water_treatment_condition__defaultCondition (player, npc))
		{
			doAnimationAction (npc, "sneeze");
			
			string_id message = new string_id (c_stringFile, "s_74");
			chat.chat (npc, player, message);
			
			return SCRIPT_CONTINUE;
		}
		
		chat.chat (npc, "Error: All conditions for OnStartNpcConversation were false.");
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int OnNpcConversationResponse(obj_id self, String conversationId, obj_id player, string_id response) throws InterruptedException
	{
		if (!conversationId.equals("water_treatment"))
		{
			return SCRIPT_CONTINUE;
		}
		
		obj_id npc = self;
		
		int branchId = utils.getIntScriptVar (player, "conversation.water_treatment.branchId");
		
		if (branchId == 2 && water_treatment_handleBranch2 (player, npc, response) == SCRIPT_CONTINUE)
		{
			return SCRIPT_CONTINUE;
		}
		
		if (branchId == 3 && water_treatment_handleBranch3 (player, npc, response) == SCRIPT_CONTINUE)
		{
			return SCRIPT_CONTINUE;
		}
		
		if (branchId == 6 && water_treatment_handleBranch6 (player, npc, response) == SCRIPT_CONTINUE)
		{
			return SCRIPT_CONTINUE;
		}
		
		if (branchId == 8 && water_treatment_handleBranch8 (player, npc, response) == SCRIPT_CONTINUE)
		{
			return SCRIPT_CONTINUE;
		}
		
		if (branchId == 9 && water_treatment_handleBranch9 (player, npc, response) == SCRIPT_CONTINUE)
		{
			return SCRIPT_CONTINUE;
		}
		
		if (branchId == 13 && water_treatment_handleBranch13 (player, npc, response) == SCRIPT_CONTINUE)
		{
			return SCRIPT_CONTINUE;
		}
		
		if (branchId == 14 && water_treatment_handleBranch14 (player, npc, response) == SCRIPT_CONTINUE)
		{
			return SCRIPT_CONTINUE;
		}
		
		if (branchId == 18 && water_treatment_handleBranch18 (player, npc, response) == SCRIPT_CONTINUE)
		{
			return SCRIPT_CONTINUE;
		}
		
		if (branchId == 19 && water_treatment_handleBranch19 (player, npc, response) == SCRIPT_CONTINUE)
		{
			return SCRIPT_CONTINUE;
		}
		
		if (branchId == 20 && water_treatment_handleBranch20 (player, npc, response) == SCRIPT_CONTINUE)
		{
			return SCRIPT_CONTINUE;
		}
		
		if (branchId == 21 && water_treatment_handleBranch21 (player, npc, response) == SCRIPT_CONTINUE)
		{
			return SCRIPT_CONTINUE;
		}
		
		if (branchId == 22 && water_treatment_handleBranch22 (player, npc, response) == SCRIPT_CONTINUE)
		{
			return SCRIPT_CONTINUE;
		}
		
		chat.chat (npc, "Error: Fell through all branches and responses for OnNpcConversationResponse.");
		
		utils.removeScriptVar (player, "conversation.water_treatment.branchId");
		
		return SCRIPT_CONTINUE;
	}
	
}
