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


public class radio_delivery_deathtrooper_conversation extends script.base_script
{
	public radio_delivery_deathtrooper_conversation()
	{
	}
	String c_stringFile = "conversation/radio_delivery_deathtrooper_conversation";
	
	
	public boolean radio_delivery_deathtrooper_conversation_condition__defaultCondition(obj_id player, obj_id npc) throws InterruptedException
	{
		return true;
	}
	
	
	public boolean radio_delivery_deathtrooper_conversation_condition_hasDeliveryQuestActive(obj_id player, obj_id npc) throws InterruptedException
	{
		boolean returnVal = false;
		
		for (int i = 1; i <= 3; i++)
		{
			testAbortScript();
			if ((groundquests.isTaskActive(player, "outbreak_radio_delivery_0"+i, "findCamp") || groundquests.isTaskActive(player, "outbreak_radio_delivery_0"+i, "hiddenSearchArea") || groundquests.isTaskActive(player, "outbreak_radio_delivery_0"+i, "findBossInCharge") || groundquests.isTaskActive(player, "outbreak_radio_delivery_0"+i, "goodWork")))
			{
				returnVal = true;
			}
			if (i == 3 && !returnVal)
			{
				if ((groundquests.isTaskActive(player, "outbreak_radio_delivery_03", "findEpsilonContact") || groundquests.isTaskActive(player, "outbreak_radio_delivery_03", "comGoodWork2")))
				{
					returnVal = true;
				}
			}
		}
		return returnVal;
	}
	
	
	public boolean radio_delivery_deathtrooper_conversation_condition_hasRadioQuest1Complete(obj_id player, obj_id npc) throws InterruptedException
	{
		return groundquests.hasCompletedQuest(player, "outbreak_radio_delivery_01");
	}
	
	
	public boolean radio_delivery_deathtrooper_conversation_condition_hasNeverSpoken(obj_id player, obj_id npc) throws InterruptedException
	{
		return !groundquests.isQuestActiveOrComplete(player, "outbreak_radio_delivery_01");
	}
	
	
	public boolean radio_delivery_deathtrooper_conversation_condition_hasRadioOneDelivered(obj_id player, obj_id npc) throws InterruptedException
	{
		return groundquests.isTaskActive(player, "outbreak_radio_delivery_01", "talkToMaris");
	}
	
	
	public boolean radio_delivery_deathtrooper_conversation_condition_hasRadioTwoDelivered(obj_id player, obj_id npc) throws InterruptedException
	{
		return groundquests.isTaskActive(player, "outbreak_radio_delivery_02", "talkToMaris");
	}
	
	
	public boolean radio_delivery_deathtrooper_conversation_condition_hasRadioQuest2Complete(obj_id player, obj_id npc) throws InterruptedException
	{
		return groundquests.hasCompletedQuest(player, "outbreak_radio_delivery_02");
	}
	
	
	public boolean radio_delivery_deathtrooper_conversation_condition_noAntiVirus(obj_id player, obj_id npc) throws InterruptedException
	{
		return !groundquests.hasCompletedQuest(player, "outbreak_quest_01_imperial") && !groundquests.hasCompletedQuest(player, "outbreak_quest_01_rebel") && !groundquests.hasCompletedQuest(player, "outbreak_quest_01_neutral");
	}
	
	
	public boolean radio_delivery_deathtrooper_conversation_condition_hasLastRadiosDelivered(obj_id player, obj_id npc) throws InterruptedException
	{
		return groundquests.isTaskActive(player, "outbreak_radio_delivery_03", "talkToMaris");
	}
	
	
	public boolean radio_delivery_deathtrooper_conversation_condition_hasCompletedAll(obj_id player, obj_id npc) throws InterruptedException
	{
		return groundquests.hasCompletedQuest(player, "outbreak_radio_delivery_03");
	}
	
	
	public boolean radio_delivery_deathtrooper_conversation_condition_hasHelmet(obj_id player, obj_id npc) throws InterruptedException
	{
		return groundquests.isTaskActive(player, "outbreak_bomber_helmet_hidden_content", "hasHelmet");
	}
	
	
	public void radio_delivery_deathtrooper_conversation_action_giveFirstDeliveryQuest(obj_id player, obj_id npc) throws InterruptedException
	{
		groundquests.grantQuest(player, "outbreak_radio_delivery_01");
	}
	
	
	public void radio_delivery_deathtrooper_conversation_action_deliveryOneCompleted(obj_id player, obj_id npc) throws InterruptedException
	{
		groundquests.sendSignal(player, "spokenToZebMaris");
	}
	
	
	public void radio_delivery_deathtrooper_conversation_action_giveSecondDeliveryQuest(obj_id player, obj_id npc) throws InterruptedException
	{
		groundquests.grantQuest(player, "outbreak_radio_delivery_02");
	}
	
	
	public void radio_delivery_deathtrooper_conversation_action_giveThirdDeliveryQuest(obj_id player, obj_id npc) throws InterruptedException
	{
		groundquests.grantQuest(player, "outbreak_radio_delivery_03");
	}
	
	
	public void radio_delivery_deathtrooper_conversation_action_deliverTwoCompleted(obj_id player, obj_id npc) throws InterruptedException
	{
		groundquests.sendSignal(player, "spokenToZebMarisGamma");
	}
	
	
	public void radio_delivery_deathtrooper_conversation_action_deliverThreeCompleted(obj_id player, obj_id npc) throws InterruptedException
	{
		groundquests.sendSignal(player, "spokenToZebMarisDelta");
	}
	
	
	public void radio_delivery_deathtrooper_conversation_action_hasDeliveredHelmet(obj_id player, obj_id npc) throws InterruptedException
	{
		groundquests.sendSignal(player, "hasDeliveredHelmet");
		if (!hasCompletedCollectionSlot(player, "hidden_content_tie_helmet"))
		{
			modifyCollectionSlotValue(player, "hidden_content_tie_helmet", 1);
		}
	}
	
	
	public int radio_delivery_deathtrooper_conversation_handleBranch1(obj_id player, obj_id npc, string_id response) throws InterruptedException
	{
		
		if (response.equals("s_92"))
		{
			
			if (radio_delivery_deathtrooper_conversation_condition__defaultCondition (player, npc))
			{
				doAnimationAction (npc, "celebrate1");
				
				radio_delivery_deathtrooper_conversation_action_hasDeliveredHelmet (player, npc);
				
				string_id message = new string_id (c_stringFile, "s_93");
				utils.removeScriptVar (player, "conversation.radio_delivery_deathtrooper_conversation.branchId");
				
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
	
	
	public int radio_delivery_deathtrooper_conversation_handleBranch5(obj_id player, obj_id npc, string_id response) throws InterruptedException
	{
		
		if (response.equals("s_71"))
		{
			
			if (radio_delivery_deathtrooper_conversation_condition__defaultCondition (player, npc))
			{
				radio_delivery_deathtrooper_conversation_action_deliverThreeCompleted (player, npc);
				
				string_id message = new string_id (c_stringFile, "s_72");
				utils.removeScriptVar (player, "conversation.radio_delivery_deathtrooper_conversation.branchId");
				
				npcEndConversationWithMessage (player, message);
				
				return SCRIPT_CONTINUE;
			}
			
		}
		
		if (response.equals("s_73"))
		{
			
			if (radio_delivery_deathtrooper_conversation_condition__defaultCondition (player, npc))
			{
				radio_delivery_deathtrooper_conversation_action_deliverThreeCompleted (player, npc);
				
				string_id message = new string_id (c_stringFile, "s_74");
				utils.removeScriptVar (player, "conversation.radio_delivery_deathtrooper_conversation.branchId");
				
				npcEndConversationWithMessage (player, message);
				
				return SCRIPT_CONTINUE;
			}
			
		}
		
		return SCRIPT_DEFAULT;
	}
	
	
	public int radio_delivery_deathtrooper_conversation_handleBranch8(obj_id player, obj_id npc, string_id response) throws InterruptedException
	{
		
		if (response.equals("s_41"))
		{
			
			if (radio_delivery_deathtrooper_conversation_condition__defaultCondition (player, npc))
			{
				
				string_id message = new string_id (c_stringFile, "s_53");
				int numberOfResponses = 0;
				
				boolean hasResponse = false;
				
				boolean hasResponse0 = false;
				if (radio_delivery_deathtrooper_conversation_condition__defaultCondition (player, npc))
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
					
					utils.setScriptVar (player, "conversation.radio_delivery_deathtrooper_conversation.branchId", 9);
					
					npcSpeak (player, message);
					npcSetConversationResponses (player, responses);
				}
				else
				{
					utils.removeScriptVar (player, "conversation.radio_delivery_deathtrooper_conversation.branchId");
					
					npcEndConversationWithMessage (player, message);
				}
				
				return SCRIPT_CONTINUE;
			}
			
		}
		
		return SCRIPT_DEFAULT;
	}
	
	
	public int radio_delivery_deathtrooper_conversation_handleBranch9(obj_id player, obj_id npc, string_id response) throws InterruptedException
	{
		
		if (response.equals("s_64"))
		{
			
			if (radio_delivery_deathtrooper_conversation_condition__defaultCondition (player, npc))
			{
				
				string_id message = new string_id (c_stringFile, "s_65");
				int numberOfResponses = 0;
				
				boolean hasResponse = false;
				
				boolean hasResponse0 = false;
				if (radio_delivery_deathtrooper_conversation_condition__defaultCondition (player, npc))
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
					
					utils.setScriptVar (player, "conversation.radio_delivery_deathtrooper_conversation.branchId", 10);
					
					npcSpeak (player, message);
					npcSetConversationResponses (player, responses);
				}
				else
				{
					utils.removeScriptVar (player, "conversation.radio_delivery_deathtrooper_conversation.branchId");
					
					npcEndConversationWithMessage (player, message);
				}
				
				return SCRIPT_CONTINUE;
			}
			
		}
		
		return SCRIPT_DEFAULT;
	}
	
	
	public int radio_delivery_deathtrooper_conversation_handleBranch10(obj_id player, obj_id npc, string_id response) throws InterruptedException
	{
		
		if (response.equals("s_66"))
		{
			
			if (radio_delivery_deathtrooper_conversation_condition__defaultCondition (player, npc))
			{
				radio_delivery_deathtrooper_conversation_action_giveThirdDeliveryQuest (player, npc);
				
				string_id message = new string_id (c_stringFile, "s_67");
				utils.removeScriptVar (player, "conversation.radio_delivery_deathtrooper_conversation.branchId");
				
				npcEndConversationWithMessage (player, message);
				
				return SCRIPT_CONTINUE;
			}
			
		}
		
		return SCRIPT_DEFAULT;
	}
	
	
	public int radio_delivery_deathtrooper_conversation_handleBranch13(obj_id player, obj_id npc, string_id response) throws InterruptedException
	{
		
		if (response.equals("s_55"))
		{
			
			if (radio_delivery_deathtrooper_conversation_condition__defaultCondition (player, npc))
			{
				
				string_id message = new string_id (c_stringFile, "s_56");
				int numberOfResponses = 0;
				
				boolean hasResponse = false;
				
				boolean hasResponse0 = false;
				if (radio_delivery_deathtrooper_conversation_condition__defaultCondition (player, npc))
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
					
					utils.setScriptVar (player, "conversation.radio_delivery_deathtrooper_conversation.branchId", 14);
					
					npcSpeak (player, message);
					npcSetConversationResponses (player, responses);
				}
				else
				{
					utils.removeScriptVar (player, "conversation.radio_delivery_deathtrooper_conversation.branchId");
					
					npcEndConversationWithMessage (player, message);
				}
				
				return SCRIPT_CONTINUE;
			}
			
		}
		
		return SCRIPT_DEFAULT;
	}
	
	
	public int radio_delivery_deathtrooper_conversation_handleBranch14(obj_id player, obj_id npc, string_id response) throws InterruptedException
	{
		
		if (response.equals("s_57"))
		{
			
			if (radio_delivery_deathtrooper_conversation_condition__defaultCondition (player, npc))
			{
				
				string_id message = new string_id (c_stringFile, "s_58");
				int numberOfResponses = 0;
				
				boolean hasResponse = false;
				
				boolean hasResponse0 = false;
				if (radio_delivery_deathtrooper_conversation_condition__defaultCondition (player, npc))
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
					
					utils.setScriptVar (player, "conversation.radio_delivery_deathtrooper_conversation.branchId", 15);
					
					npcSpeak (player, message);
					npcSetConversationResponses (player, responses);
				}
				else
				{
					utils.removeScriptVar (player, "conversation.radio_delivery_deathtrooper_conversation.branchId");
					
					npcEndConversationWithMessage (player, message);
				}
				
				return SCRIPT_CONTINUE;
			}
			
		}
		
		return SCRIPT_DEFAULT;
	}
	
	
	public int radio_delivery_deathtrooper_conversation_handleBranch15(obj_id player, obj_id npc, string_id response) throws InterruptedException
	{
		
		if (response.equals("s_59"))
		{
			
			if (radio_delivery_deathtrooper_conversation_condition__defaultCondition (player, npc))
			{
				
				string_id message = new string_id (c_stringFile, "s_60");
				int numberOfResponses = 0;
				
				boolean hasResponse = false;
				
				boolean hasResponse0 = false;
				if (radio_delivery_deathtrooper_conversation_condition__defaultCondition (player, npc))
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
					
					utils.setScriptVar (player, "conversation.radio_delivery_deathtrooper_conversation.branchId", 16);
					
					npcSpeak (player, message);
					npcSetConversationResponses (player, responses);
				}
				else
				{
					utils.removeScriptVar (player, "conversation.radio_delivery_deathtrooper_conversation.branchId");
					
					npcEndConversationWithMessage (player, message);
				}
				
				return SCRIPT_CONTINUE;
			}
			
		}
		
		return SCRIPT_DEFAULT;
	}
	
	
	public int radio_delivery_deathtrooper_conversation_handleBranch16(obj_id player, obj_id npc, string_id response) throws InterruptedException
	{
		
		if (response.equals("s_62"))
		{
			
			if (radio_delivery_deathtrooper_conversation_condition__defaultCondition (player, npc))
			{
				radio_delivery_deathtrooper_conversation_action_giveSecondDeliveryQuest (player, npc);
				
				string_id message = new string_id (c_stringFile, "s_63");
				utils.removeScriptVar (player, "conversation.radio_delivery_deathtrooper_conversation.branchId");
				
				npcEndConversationWithMessage (player, message);
				
				return SCRIPT_CONTINUE;
			}
			
		}
		
		return SCRIPT_DEFAULT;
	}
	
	
	public int radio_delivery_deathtrooper_conversation_handleBranch20(obj_id player, obj_id npc, string_id response) throws InterruptedException
	{
		
		if (response.equals("s_37"))
		{
			
			if (radio_delivery_deathtrooper_conversation_condition__defaultCondition (player, npc))
			{
				
				string_id message = new string_id (c_stringFile, "s_39");
				int numberOfResponses = 0;
				
				boolean hasResponse = false;
				
				boolean hasResponse0 = false;
				if (radio_delivery_deathtrooper_conversation_condition__defaultCondition (player, npc))
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
					
					utils.setScriptVar (player, "conversation.radio_delivery_deathtrooper_conversation.branchId", 21);
					
					npcSpeak (player, message);
					npcSetConversationResponses (player, responses);
				}
				else
				{
					utils.removeScriptVar (player, "conversation.radio_delivery_deathtrooper_conversation.branchId");
					
					npcEndConversationWithMessage (player, message);
				}
				
				return SCRIPT_CONTINUE;
			}
			
		}
		
		return SCRIPT_DEFAULT;
	}
	
	
	public int radio_delivery_deathtrooper_conversation_handleBranch21(obj_id player, obj_id npc, string_id response) throws InterruptedException
	{
		
		if (response.equals("s_42"))
		{
			
			if (radio_delivery_deathtrooper_conversation_condition__defaultCondition (player, npc))
			{
				
				string_id message = new string_id (c_stringFile, "s_44");
				int numberOfResponses = 0;
				
				boolean hasResponse = false;
				
				boolean hasResponse0 = false;
				if (radio_delivery_deathtrooper_conversation_condition__defaultCondition (player, npc))
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
					
					utils.setScriptVar (player, "conversation.radio_delivery_deathtrooper_conversation.branchId", 22);
					
					npcSpeak (player, message);
					npcSetConversationResponses (player, responses);
				}
				else
				{
					utils.removeScriptVar (player, "conversation.radio_delivery_deathtrooper_conversation.branchId");
					
					npcEndConversationWithMessage (player, message);
				}
				
				return SCRIPT_CONTINUE;
			}
			
		}
		
		return SCRIPT_DEFAULT;
	}
	
	
	public int radio_delivery_deathtrooper_conversation_handleBranch22(obj_id player, obj_id npc, string_id response) throws InterruptedException
	{
		
		if (response.equals("s_46"))
		{
			
			if (radio_delivery_deathtrooper_conversation_condition__defaultCondition (player, npc))
			{
				
				string_id message = new string_id (c_stringFile, "s_48");
				int numberOfResponses = 0;
				
				boolean hasResponse = false;
				
				boolean hasResponse0 = false;
				if (radio_delivery_deathtrooper_conversation_condition__defaultCondition (player, npc))
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
						responses[responseIndex++] = new string_id (c_stringFile, "s_50");
					}
					
					utils.setScriptVar (player, "conversation.radio_delivery_deathtrooper_conversation.branchId", 23);
					
					npcSpeak (player, message);
					npcSetConversationResponses (player, responses);
				}
				else
				{
					utils.removeScriptVar (player, "conversation.radio_delivery_deathtrooper_conversation.branchId");
					
					npcEndConversationWithMessage (player, message);
				}
				
				return SCRIPT_CONTINUE;
			}
			
		}
		
		return SCRIPT_DEFAULT;
	}
	
	
	public int radio_delivery_deathtrooper_conversation_handleBranch23(obj_id player, obj_id npc, string_id response) throws InterruptedException
	{
		
		if (response.equals("s_50"))
		{
			
			if (radio_delivery_deathtrooper_conversation_condition__defaultCondition (player, npc))
			{
				
				string_id message = new string_id (c_stringFile, "s_52");
				int numberOfResponses = 0;
				
				boolean hasResponse = false;
				
				boolean hasResponse0 = false;
				if (radio_delivery_deathtrooper_conversation_condition__defaultCondition (player, npc))
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
					
					utils.setScriptVar (player, "conversation.radio_delivery_deathtrooper_conversation.branchId", 24);
					
					npcSpeak (player, message);
					npcSetConversationResponses (player, responses);
				}
				else
				{
					utils.removeScriptVar (player, "conversation.radio_delivery_deathtrooper_conversation.branchId");
					
					npcEndConversationWithMessage (player, message);
				}
				
				return SCRIPT_CONTINUE;
			}
			
		}
		
		return SCRIPT_DEFAULT;
	}
	
	
	public int radio_delivery_deathtrooper_conversation_handleBranch24(obj_id player, obj_id npc, string_id response) throws InterruptedException
	{
		
		if (response.equals("s_77"))
		{
			
			if (radio_delivery_deathtrooper_conversation_condition__defaultCondition (player, npc))
			{
				
				string_id message = new string_id (c_stringFile, "s_79");
				int numberOfResponses = 0;
				
				boolean hasResponse = false;
				
				boolean hasResponse0 = false;
				if (radio_delivery_deathtrooper_conversation_condition__defaultCondition (player, npc))
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
					
					utils.setScriptVar (player, "conversation.radio_delivery_deathtrooper_conversation.branchId", 25);
					
					npcSpeak (player, message);
					npcSetConversationResponses (player, responses);
				}
				else
				{
					utils.removeScriptVar (player, "conversation.radio_delivery_deathtrooper_conversation.branchId");
					
					npcEndConversationWithMessage (player, message);
				}
				
				return SCRIPT_CONTINUE;
			}
			
		}
		
		return SCRIPT_DEFAULT;
	}
	
	
	public int radio_delivery_deathtrooper_conversation_handleBranch25(obj_id player, obj_id npc, string_id response) throws InterruptedException
	{
		
		if (response.equals("s_81"))
		{
			
			if (radio_delivery_deathtrooper_conversation_condition__defaultCondition (player, npc))
			{
				
				string_id message = new string_id (c_stringFile, "s_83");
				int numberOfResponses = 0;
				
				boolean hasResponse = false;
				
				boolean hasResponse0 = false;
				if (radio_delivery_deathtrooper_conversation_condition__defaultCondition (player, npc))
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
					
					utils.setScriptVar (player, "conversation.radio_delivery_deathtrooper_conversation.branchId", 26);
					
					npcSpeak (player, message);
					npcSetConversationResponses (player, responses);
				}
				else
				{
					utils.removeScriptVar (player, "conversation.radio_delivery_deathtrooper_conversation.branchId");
					
					npcEndConversationWithMessage (player, message);
				}
				
				return SCRIPT_CONTINUE;
			}
			
		}
		
		return SCRIPT_DEFAULT;
	}
	
	
	public int radio_delivery_deathtrooper_conversation_handleBranch26(obj_id player, obj_id npc, string_id response) throws InterruptedException
	{
		
		if (response.equals("s_85"))
		{
			
			if (radio_delivery_deathtrooper_conversation_condition__defaultCondition (player, npc))
			{
				
				string_id message = new string_id (c_stringFile, "s_87");
				int numberOfResponses = 0;
				
				boolean hasResponse = false;
				
				boolean hasResponse0 = false;
				if (radio_delivery_deathtrooper_conversation_condition__defaultCondition (player, npc))
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
					
					utils.setScriptVar (player, "conversation.radio_delivery_deathtrooper_conversation.branchId", 27);
					
					npcSpeak (player, message);
					npcSetConversationResponses (player, responses);
				}
				else
				{
					utils.removeScriptVar (player, "conversation.radio_delivery_deathtrooper_conversation.branchId");
					
					npcEndConversationWithMessage (player, message);
				}
				
				return SCRIPT_CONTINUE;
			}
			
		}
		
		return SCRIPT_DEFAULT;
	}
	
	
	public int radio_delivery_deathtrooper_conversation_handleBranch27(obj_id player, obj_id npc, string_id response) throws InterruptedException
	{
		
		if (response.equals("s_89"))
		{
			
			if (radio_delivery_deathtrooper_conversation_condition__defaultCondition (player, npc))
			{
				radio_delivery_deathtrooper_conversation_action_giveFirstDeliveryQuest (player, npc);
				
				string_id message = new string_id (c_stringFile, "s_94");
				utils.removeScriptVar (player, "conversation.radio_delivery_deathtrooper_conversation.branchId");
				
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
		
		if (radio_delivery_deathtrooper_conversation_condition_hasHelmet (player, npc))
		{
			
			string_id message = new string_id (c_stringFile, "s_91");
			int numberOfResponses = 0;
			
			boolean hasResponse = false;
			
			boolean hasResponse0 = false;
			if (radio_delivery_deathtrooper_conversation_condition__defaultCondition (player, npc))
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
				
				utils.setScriptVar (player, "conversation.radio_delivery_deathtrooper_conversation.branchId", 1);
				
				npcStartConversation (player, npc, "radio_delivery_deathtrooper_conversation", message, responses);
			}
			else
			{
				chat.chat (npc, player, message);
			}
			
			return SCRIPT_CONTINUE;
		}
		
		if (radio_delivery_deathtrooper_conversation_condition_hasCompletedAll (player, npc))
		{
			
			string_id message = new string_id (c_stringFile, "s_75");
			chat.chat (npc, player, message);
			
			return SCRIPT_CONTINUE;
		}
		
		if (radio_delivery_deathtrooper_conversation_condition_hasDeliveryQuestActive (player, npc))
		{
			
			string_id message = new string_id (c_stringFile, "s_31");
			chat.chat (npc, player, message);
			
			return SCRIPT_CONTINUE;
		}
		
		if (radio_delivery_deathtrooper_conversation_condition_hasLastRadiosDelivered (player, npc))
		{
			
			string_id message = new string_id (c_stringFile, "s_70");
			int numberOfResponses = 0;
			
			boolean hasResponse = false;
			
			boolean hasResponse0 = false;
			if (radio_delivery_deathtrooper_conversation_condition__defaultCondition (player, npc))
			{
				++numberOfResponses;
				hasResponse = true;
				hasResponse0 = true;
			}
			
			boolean hasResponse1 = false;
			if (radio_delivery_deathtrooper_conversation_condition__defaultCondition (player, npc))
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
					responses[responseIndex++] = new string_id (c_stringFile, "s_73");
				}
				
				utils.setScriptVar (player, "conversation.radio_delivery_deathtrooper_conversation.branchId", 5);
				
				npcStartConversation (player, npc, "radio_delivery_deathtrooper_conversation", message, responses);
			}
			else
			{
				chat.chat (npc, player, message);
			}
			
			return SCRIPT_CONTINUE;
		}
		
		if (radio_delivery_deathtrooper_conversation_condition_hasRadioQuest2Complete (player, npc))
		{
			
			string_id message = new string_id (c_stringFile, "s_33");
			int numberOfResponses = 0;
			
			boolean hasResponse = false;
			
			boolean hasResponse0 = false;
			if (radio_delivery_deathtrooper_conversation_condition__defaultCondition (player, npc))
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
				
				utils.setScriptVar (player, "conversation.radio_delivery_deathtrooper_conversation.branchId", 8);
				
				npcStartConversation (player, npc, "radio_delivery_deathtrooper_conversation", message, responses);
			}
			else
			{
				chat.chat (npc, player, message);
			}
			
			return SCRIPT_CONTINUE;
		}
		
		if (radio_delivery_deathtrooper_conversation_condition_hasRadioTwoDelivered (player, npc))
		{
			radio_delivery_deathtrooper_conversation_action_deliverTwoCompleted (player, npc);
			
			string_id message = new string_id (c_stringFile, "s_68");
			chat.chat (npc, player, message);
			
			return SCRIPT_CONTINUE;
		}
		
		if (radio_delivery_deathtrooper_conversation_condition_hasRadioQuest1Complete (player, npc))
		{
			
			string_id message = new string_id (c_stringFile, "s_54");
			int numberOfResponses = 0;
			
			boolean hasResponse = false;
			
			boolean hasResponse0 = false;
			if (radio_delivery_deathtrooper_conversation_condition__defaultCondition (player, npc))
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
				
				utils.setScriptVar (player, "conversation.radio_delivery_deathtrooper_conversation.branchId", 13);
				
				npcStartConversation (player, npc, "radio_delivery_deathtrooper_conversation", message, responses);
			}
			else
			{
				chat.chat (npc, player, message);
			}
			
			return SCRIPT_CONTINUE;
		}
		
		if (radio_delivery_deathtrooper_conversation_condition_hasRadioOneDelivered (player, npc))
		{
			radio_delivery_deathtrooper_conversation_action_deliveryOneCompleted (player, npc);
			
			string_id message = new string_id (c_stringFile, "s_61");
			chat.chat (npc, player, message);
			
			return SCRIPT_CONTINUE;
		}
		
		if (radio_delivery_deathtrooper_conversation_condition_noAntiVirus (player, npc))
		{
			
			string_id message = new string_id (c_stringFile, "s_69");
			chat.chat (npc, player, message);
			
			return SCRIPT_CONTINUE;
		}
		
		if (radio_delivery_deathtrooper_conversation_condition_hasNeverSpoken (player, npc))
		{
			
			string_id message = new string_id (c_stringFile, "s_35");
			int numberOfResponses = 0;
			
			boolean hasResponse = false;
			
			boolean hasResponse0 = false;
			if (radio_delivery_deathtrooper_conversation_condition__defaultCondition (player, npc))
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
				
				utils.setScriptVar (player, "conversation.radio_delivery_deathtrooper_conversation.branchId", 20);
				
				npcStartConversation (player, npc, "radio_delivery_deathtrooper_conversation", message, responses);
			}
			else
			{
				chat.chat (npc, player, message);
			}
			
			return SCRIPT_CONTINUE;
		}
		
		if (radio_delivery_deathtrooper_conversation_condition__defaultCondition (player, npc))
		{
			
			string_id message = new string_id (c_stringFile, "s_96");
			chat.chat (npc, player, message);
			
			return SCRIPT_CONTINUE;
		}
		
		chat.chat (npc, "Error: All conditions for OnStartNpcConversation were false.");
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int OnNpcConversationResponse(obj_id self, String conversationId, obj_id player, string_id response) throws InterruptedException
	{
		if (!conversationId.equals("radio_delivery_deathtrooper_conversation"))
		{
			return SCRIPT_CONTINUE;
		}
		
		obj_id npc = self;
		
		int branchId = utils.getIntScriptVar (player, "conversation.radio_delivery_deathtrooper_conversation.branchId");
		
		if (branchId == 1 && radio_delivery_deathtrooper_conversation_handleBranch1 (player, npc, response) == SCRIPT_CONTINUE)
		{
			return SCRIPT_CONTINUE;
		}
		
		if (branchId == 5 && radio_delivery_deathtrooper_conversation_handleBranch5 (player, npc, response) == SCRIPT_CONTINUE)
		{
			return SCRIPT_CONTINUE;
		}
		
		if (branchId == 8 && radio_delivery_deathtrooper_conversation_handleBranch8 (player, npc, response) == SCRIPT_CONTINUE)
		{
			return SCRIPT_CONTINUE;
		}
		
		if (branchId == 9 && radio_delivery_deathtrooper_conversation_handleBranch9 (player, npc, response) == SCRIPT_CONTINUE)
		{
			return SCRIPT_CONTINUE;
		}
		
		if (branchId == 10 && radio_delivery_deathtrooper_conversation_handleBranch10 (player, npc, response) == SCRIPT_CONTINUE)
		{
			return SCRIPT_CONTINUE;
		}
		
		if (branchId == 13 && radio_delivery_deathtrooper_conversation_handleBranch13 (player, npc, response) == SCRIPT_CONTINUE)
		{
			return SCRIPT_CONTINUE;
		}
		
		if (branchId == 14 && radio_delivery_deathtrooper_conversation_handleBranch14 (player, npc, response) == SCRIPT_CONTINUE)
		{
			return SCRIPT_CONTINUE;
		}
		
		if (branchId == 15 && radio_delivery_deathtrooper_conversation_handleBranch15 (player, npc, response) == SCRIPT_CONTINUE)
		{
			return SCRIPT_CONTINUE;
		}
		
		if (branchId == 16 && radio_delivery_deathtrooper_conversation_handleBranch16 (player, npc, response) == SCRIPT_CONTINUE)
		{
			return SCRIPT_CONTINUE;
		}
		
		if (branchId == 20 && radio_delivery_deathtrooper_conversation_handleBranch20 (player, npc, response) == SCRIPT_CONTINUE)
		{
			return SCRIPT_CONTINUE;
		}
		
		if (branchId == 21 && radio_delivery_deathtrooper_conversation_handleBranch21 (player, npc, response) == SCRIPT_CONTINUE)
		{
			return SCRIPT_CONTINUE;
		}
		
		if (branchId == 22 && radio_delivery_deathtrooper_conversation_handleBranch22 (player, npc, response) == SCRIPT_CONTINUE)
		{
			return SCRIPT_CONTINUE;
		}
		
		if (branchId == 23 && radio_delivery_deathtrooper_conversation_handleBranch23 (player, npc, response) == SCRIPT_CONTINUE)
		{
			return SCRIPT_CONTINUE;
		}
		
		if (branchId == 24 && radio_delivery_deathtrooper_conversation_handleBranch24 (player, npc, response) == SCRIPT_CONTINUE)
		{
			return SCRIPT_CONTINUE;
		}
		
		if (branchId == 25 && radio_delivery_deathtrooper_conversation_handleBranch25 (player, npc, response) == SCRIPT_CONTINUE)
		{
			return SCRIPT_CONTINUE;
		}
		
		if (branchId == 26 && radio_delivery_deathtrooper_conversation_handleBranch26 (player, npc, response) == SCRIPT_CONTINUE)
		{
			return SCRIPT_CONTINUE;
		}
		
		if (branchId == 27 && radio_delivery_deathtrooper_conversation_handleBranch27 (player, npc, response) == SCRIPT_CONTINUE)
		{
			return SCRIPT_CONTINUE;
		}
		
		chat.chat (npc, "Error: Fell through all branches and responses for OnNpcConversationResponse.");
		
		utils.removeScriptVar (player, "conversation.radio_delivery_deathtrooper_conversation.branchId");
		
		return SCRIPT_CONTINUE;
	}
	
}
