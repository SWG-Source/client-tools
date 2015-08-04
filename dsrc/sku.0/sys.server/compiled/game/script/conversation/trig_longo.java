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


public class trig_longo extends script.base_script
{
	public trig_longo()
	{
	}
	String c_stringFile = "conversation/trig_longo";
	
	
	public boolean trig_longo_condition__defaultCondition(obj_id player, obj_id npc) throws InterruptedException
	{
		return true;
	}
	
	
	public boolean trig_longo_condition_isNotImmunized(obj_id player, obj_id npc) throws InterruptedException
	{
		faceTo(npc, player);
		return (groundquests.isTaskActive(player, "outbreak_quest_01_imperial", "codyInnoculate") || groundquests.isTaskActive(player, "outbreak_quest_01_rebel", "codyInnoculate") || groundquests.isTaskActive(player, "outbreak_quest_01_neutral", "codyInnoculate") );
	}
	
	
	public boolean trig_longo_condition_hasTrigTask(obj_id player, obj_id npc) throws InterruptedException
	{
		faceTo(npc, player);
		return groundquests.isTaskActive(player, "outbreak_quest_01_b_imperial", "talkToTrig") || groundquests.isTaskActive(player, "outbreak_quest_01_b_rebel", "talkToTrig") || groundquests.isTaskActive(player, "outbreak_quest_01_b_neutral", "talkToTrig");
	}
	
	
	public boolean trig_longo_condition_hasDeletedQuest2(obj_id player, obj_id npc) throws InterruptedException
	{
		faceTo(npc, player);
		return (groundquests.hasCompletedQuest(player, "outbreak_quest_01_b_imperial") && !groundquests.isQuestActiveOrComplete(player, "outbreak_quest_02_imperial")) || (groundquests.hasCompletedQuest(player, "outbreak_quest_01_b_rebel") && !groundquests.isQuestActiveOrComplete(player, "outbreak_quest_02_rebel")) || (groundquests.hasCompletedQuest(player, "outbreak_quest_01_b_neutral") && !groundquests.isQuestActiveOrComplete(player, "outbreak_quest_02_neutral"));
	}
	
	
	public boolean trig_longo_condition_hasCompletedQuestLine(obj_id player, obj_id npc) throws InterruptedException
	{
		return groundquests.hasCompletedQuest(player, "outbreak_quest_final_imperial") || groundquests.hasCompletedQuest(player, "outbreak_quest_final_neutral") || groundquests.hasCompletedQuest(player, "outbreak_quest_final_rebel");
	}
	
	
	public boolean trig_longo_condition_hasDeliveredRadioEpsilon(obj_id player, obj_id npc) throws InterruptedException
	{
		faceTo(npc, player);
		return groundquests.hasCompletedQuest(player, "outbreak_radio_delivery_03");
	}
	
	
	public boolean trig_longo_condition_hasCompletedQuest2(obj_id player, obj_id npc) throws InterruptedException
	{
		faceTo(npc, player);
		return groundquests.hasCompletedQuest(player, "outbreak_quest_02_imperial") || groundquests.hasCompletedQuest(player, "outbreak_quest_02_rebel") || groundquests.hasCompletedQuest(player, "outbreak_quest_02_neutral");
	}
	
	
	public boolean trig_longo_condition_isNeutral(obj_id player, obj_id npc) throws InterruptedException
	{
		faceTo(npc, player);
		return groundquests.hasCompletedQuest(player, "outbreak_quest_01_neutral");
	}
	
	
	public boolean trig_longo_condition_isRebel(obj_id player, obj_id npc) throws InterruptedException
	{
		faceTo(npc, player);
		return groundquests.hasCompletedQuest(player, "outbreak_quest_01_rebel");
	}
	
	
	public void trig_longo_action_updatedQuestWaypoint(obj_id player, obj_id npc) throws InterruptedException
	{
		faceTo(npc, player);
		
		groundquests.sendSignal(player, "spokenToTrigQuest01");
	}
	
	
	public void trig_longo_action_giveQuest2(obj_id player, obj_id npc) throws InterruptedException
	{
		if ((groundquests.hasCompletedQuest(player, "outbreak_quest_01_imperial") && groundquests.hasCompletedQuest(player, "outbreak_quest_01_b_imperial") && !groundquests.isQuestActiveOrComplete(player, "outbreak_quest_02_imperial")))
		{
			groundquests.grantQuest(player, "outbreak_quest_02_imperial");
			return;
		}
		
		if ((groundquests.hasCompletedQuest(player, "outbreak_quest_01_rebel") && groundquests.hasCompletedQuest(player, "outbreak_quest_01_b_rebel") && !groundquests.isQuestActiveOrComplete(player, "outbreak_quest_02_rebel")))
		{
			groundquests.grantQuest(player, "outbreak_quest_02_rebel");
			return;
		}
		
		if ((groundquests.hasCompletedQuest(player, "outbreak_quest_01_neutral") && groundquests.hasCompletedQuest(player, "outbreak_quest_01_b_neutral") && !groundquests.isQuestActiveOrComplete(player, "outbreak_quest_02_neutral")))
		{
			groundquests.grantQuest(player, "outbreak_quest_02_neutral");
			return;
		}
	}
	
	
	public int trig_longo_handleBranch5(obj_id player, obj_id npc, string_id response) throws InterruptedException
	{
		
		if (response.equals("s_61"))
		{
			
			if (trig_longo_condition__defaultCondition (player, npc))
			{
				
				string_id message = new string_id (c_stringFile, "s_62");
				int numberOfResponses = 0;
				
				boolean hasResponse = false;
				
				boolean hasResponse0 = false;
				if (trig_longo_condition__defaultCondition (player, npc))
				{
					++numberOfResponses;
					hasResponse = true;
					hasResponse0 = true;
				}
				
				boolean hasResponse1 = false;
				if (trig_longo_condition_isRebel (player, npc))
				{
					++numberOfResponses;
					hasResponse = true;
					hasResponse1 = true;
				}
				
				boolean hasResponse2 = false;
				if (trig_longo_condition_isNeutral (player, npc))
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
						responses[responseIndex++] = new string_id (c_stringFile, "s_63");
					}
					
					if (hasResponse1)
					{
						responses[responseIndex++] = new string_id (c_stringFile, "s_27");
					}
					
					if (hasResponse2)
					{
						responses[responseIndex++] = new string_id (c_stringFile, "s_33");
					}
					
					utils.setScriptVar (player, "conversation.trig_longo.branchId", 6);
					
					npcSpeak (player, message);
					npcSetConversationResponses (player, responses);
				}
				else
				{
					utils.removeScriptVar (player, "conversation.trig_longo.branchId");
					
					npcEndConversationWithMessage (player, message);
				}
				
				return SCRIPT_CONTINUE;
			}
			
		}
		
		return SCRIPT_DEFAULT;
	}
	
	
	public int trig_longo_handleBranch6(obj_id player, obj_id npc, string_id response) throws InterruptedException
	{
		
		if (response.equals("s_63"))
		{
			
			if (trig_longo_condition__defaultCondition (player, npc))
			{
				
				string_id message = new string_id (c_stringFile, "s_64");
				int numberOfResponses = 0;
				
				boolean hasResponse = false;
				
				boolean hasResponse0 = false;
				if (trig_longo_condition__defaultCondition (player, npc))
				{
					++numberOfResponses;
					hasResponse = true;
					hasResponse0 = true;
				}
				
				if (hasResponse)
				{
					int responseIndex = 0;
					string_id responses[] = new string_id[numberOfResponses];
					
					if (hasResponse0)
					{
						responses[responseIndex++] = new string_id (c_stringFile, "s_65");
					}
					
					utils.setScriptVar (player, "conversation.trig_longo.branchId", 7);
					
					npcSpeak (player, message);
					npcSetConversationResponses (player, responses);
				}
				else
				{
					utils.removeScriptVar (player, "conversation.trig_longo.branchId");
					
					npcEndConversationWithMessage (player, message);
				}
				
				return SCRIPT_CONTINUE;
			}
			
		}
		
		if (response.equals("s_27"))
		{
			
			if (trig_longo_condition__defaultCondition (player, npc))
			{
				
				string_id message = new string_id (c_stringFile, "s_31");
				int numberOfResponses = 0;
				
				boolean hasResponse = false;
				
				boolean hasResponse0 = false;
				if (trig_longo_condition__defaultCondition (player, npc))
				{
					++numberOfResponses;
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
					
					utils.setScriptVar (player, "conversation.trig_longo.branchId", 13);
					
					npcSpeak (player, message);
					npcSetConversationResponses (player, responses);
				}
				else
				{
					utils.removeScriptVar (player, "conversation.trig_longo.branchId");
					
					npcEndConversationWithMessage (player, message);
				}
				
				return SCRIPT_CONTINUE;
			}
			
		}
		
		if (response.equals("s_33"))
		{
			
			if (trig_longo_condition__defaultCondition (player, npc))
			{
				
				string_id message = new string_id (c_stringFile, "s_35");
				int numberOfResponses = 0;
				
				boolean hasResponse = false;
				
				boolean hasResponse0 = false;
				if (trig_longo_condition__defaultCondition (player, npc))
				{
					++numberOfResponses;
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
					
					utils.setScriptVar (player, "conversation.trig_longo.branchId", 14);
					
					npcSpeak (player, message);
					npcSetConversationResponses (player, responses);
				}
				else
				{
					utils.removeScriptVar (player, "conversation.trig_longo.branchId");
					
					npcEndConversationWithMessage (player, message);
				}
				
				return SCRIPT_CONTINUE;
			}
			
		}
		
		return SCRIPT_DEFAULT;
	}
	
	
	public int trig_longo_handleBranch7(obj_id player, obj_id npc, string_id response) throws InterruptedException
	{
		
		if (response.equals("s_65"))
		{
			
			if (trig_longo_condition__defaultCondition (player, npc))
			{
				
				string_id message = new string_id (c_stringFile, "s_66");
				int numberOfResponses = 0;
				
				boolean hasResponse = false;
				
				boolean hasResponse0 = false;
				if (trig_longo_condition__defaultCondition (player, npc))
				{
					++numberOfResponses;
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
					
					utils.setScriptVar (player, "conversation.trig_longo.branchId", 8);
					
					npcSpeak (player, message);
					npcSetConversationResponses (player, responses);
				}
				else
				{
					utils.removeScriptVar (player, "conversation.trig_longo.branchId");
					
					npcEndConversationWithMessage (player, message);
				}
				
				return SCRIPT_CONTINUE;
			}
			
		}
		
		return SCRIPT_DEFAULT;
	}
	
	
	public int trig_longo_handleBranch8(obj_id player, obj_id npc, string_id response) throws InterruptedException
	{
		
		if (response.equals("s_67"))
		{
			
			if (trig_longo_condition__defaultCondition (player, npc))
			{
				doAnimationAction (npc, "slump_head");
				
				string_id message = new string_id (c_stringFile, "s_68");
				int numberOfResponses = 0;
				
				boolean hasResponse = false;
				
				boolean hasResponse0 = false;
				if (trig_longo_condition__defaultCondition (player, npc))
				{
					++numberOfResponses;
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
					
					utils.setScriptVar (player, "conversation.trig_longo.branchId", 9);
					
					npcSpeak (player, message);
					npcSetConversationResponses (player, responses);
				}
				else
				{
					utils.removeScriptVar (player, "conversation.trig_longo.branchId");
					
					npcEndConversationWithMessage (player, message);
				}
				
				return SCRIPT_CONTINUE;
			}
			
		}
		
		return SCRIPT_DEFAULT;
	}
	
	
	public int trig_longo_handleBranch9(obj_id player, obj_id npc, string_id response) throws InterruptedException
	{
		
		if (response.equals("s_75"))
		{
			
			if (trig_longo_condition__defaultCondition (player, npc))
			{
				
				string_id message = new string_id (c_stringFile, "s_77");
				int numberOfResponses = 0;
				
				boolean hasResponse = false;
				
				boolean hasResponse0 = false;
				if (trig_longo_condition__defaultCondition (player, npc))
				{
					++numberOfResponses;
					hasResponse = true;
					hasResponse0 = true;
				}
				
				boolean hasResponse1 = false;
				if (trig_longo_condition__defaultCondition (player, npc))
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
						responses[responseIndex++] = new string_id (c_stringFile, "s_85");
					}
					
					utils.setScriptVar (player, "conversation.trig_longo.branchId", 10);
					
					npcSpeak (player, message);
					npcSetConversationResponses (player, responses);
				}
				else
				{
					utils.removeScriptVar (player, "conversation.trig_longo.branchId");
					
					npcEndConversationWithMessage (player, message);
				}
				
				return SCRIPT_CONTINUE;
			}
			
		}
		
		return SCRIPT_DEFAULT;
	}
	
	
	public int trig_longo_handleBranch10(obj_id player, obj_id npc, string_id response) throws InterruptedException
	{
		
		if (response.equals("s_79"))
		{
			
			if (trig_longo_condition__defaultCondition (player, npc))
			{
				
				string_id message = new string_id (c_stringFile, "s_81");
				int numberOfResponses = 0;
				
				boolean hasResponse = false;
				
				boolean hasResponse0 = false;
				if (trig_longo_condition__defaultCondition (player, npc))
				{
					++numberOfResponses;
					hasResponse = true;
					hasResponse0 = true;
				}
				
				boolean hasResponse1 = false;
				if (trig_longo_condition__defaultCondition (player, npc))
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
						responses[responseIndex++] = new string_id (c_stringFile, "s_85");
					}
					
					utils.setScriptVar (player, "conversation.trig_longo.branchId", 10);
					
					npcSpeak (player, message);
					npcSetConversationResponses (player, responses);
				}
				else
				{
					utils.removeScriptVar (player, "conversation.trig_longo.branchId");
					
					npcEndConversationWithMessage (player, message);
				}
				
				return SCRIPT_CONTINUE;
			}
			
		}
		
		if (response.equals("s_85"))
		{
			
			if (trig_longo_condition__defaultCondition (player, npc))
			{
				trig_longo_action_updatedQuestWaypoint (player, npc);
				
				string_id message = new string_id (c_stringFile, "s_93");
				utils.removeScriptVar (player, "conversation.trig_longo.branchId");
				
				npcEndConversationWithMessage (player, message);
				
				return SCRIPT_CONTINUE;
			}
			
		}
		
		return SCRIPT_DEFAULT;
	}
	
	
	public int trig_longo_handleBranch11(obj_id player, obj_id npc, string_id response) throws InterruptedException
	{
		
		if (response.equals("s_79"))
		{
			
			if (trig_longo_condition__defaultCondition (player, npc))
			{
				
				string_id message = new string_id (c_stringFile, "s_81");
				int numberOfResponses = 0;
				
				boolean hasResponse = false;
				
				boolean hasResponse0 = false;
				if (trig_longo_condition__defaultCondition (player, npc))
				{
					++numberOfResponses;
					hasResponse = true;
					hasResponse0 = true;
				}
				
				boolean hasResponse1 = false;
				if (trig_longo_condition__defaultCondition (player, npc))
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
						responses[responseIndex++] = new string_id (c_stringFile, "s_85");
					}
					
					utils.setScriptVar (player, "conversation.trig_longo.branchId", 10);
					
					npcSpeak (player, message);
					npcSetConversationResponses (player, responses);
				}
				else
				{
					utils.removeScriptVar (player, "conversation.trig_longo.branchId");
					
					npcEndConversationWithMessage (player, message);
				}
				
				return SCRIPT_CONTINUE;
			}
			
		}
		
		if (response.equals("s_85"))
		{
			
			if (trig_longo_condition__defaultCondition (player, npc))
			{
				trig_longo_action_updatedQuestWaypoint (player, npc);
				
				string_id message = new string_id (c_stringFile, "s_93");
				utils.removeScriptVar (player, "conversation.trig_longo.branchId");
				
				npcEndConversationWithMessage (player, message);
				
				return SCRIPT_CONTINUE;
			}
			
		}
		
		return SCRIPT_DEFAULT;
	}
	
	
	public int trig_longo_handleBranch13(obj_id player, obj_id npc, string_id response) throws InterruptedException
	{
		
		if (response.equals("s_32"))
		{
			
			if (trig_longo_condition__defaultCondition (player, npc))
			{
				
				string_id message = new string_id (c_stringFile, "s_66");
				int numberOfResponses = 0;
				
				boolean hasResponse = false;
				
				boolean hasResponse0 = false;
				if (trig_longo_condition__defaultCondition (player, npc))
				{
					++numberOfResponses;
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
					
					utils.setScriptVar (player, "conversation.trig_longo.branchId", 8);
					
					npcSpeak (player, message);
					npcSetConversationResponses (player, responses);
				}
				else
				{
					utils.removeScriptVar (player, "conversation.trig_longo.branchId");
					
					npcEndConversationWithMessage (player, message);
				}
				
				return SCRIPT_CONTINUE;
			}
			
		}
		
		return SCRIPT_DEFAULT;
	}
	
	
	public int trig_longo_handleBranch14(obj_id player, obj_id npc, string_id response) throws InterruptedException
	{
		
		if (response.equals("s_37"))
		{
			
			if (trig_longo_condition__defaultCondition (player, npc))
			{
				
				string_id message = new string_id (c_stringFile, "s_66");
				int numberOfResponses = 0;
				
				boolean hasResponse = false;
				
				boolean hasResponse0 = false;
				if (trig_longo_condition__defaultCondition (player, npc))
				{
					++numberOfResponses;
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
					
					utils.setScriptVar (player, "conversation.trig_longo.branchId", 8);
					
					npcSpeak (player, message);
					npcSetConversationResponses (player, responses);
				}
				else
				{
					utils.removeScriptVar (player, "conversation.trig_longo.branchId");
					
					npcEndConversationWithMessage (player, message);
				}
				
				return SCRIPT_CONTINUE;
			}
			
		}
		
		return SCRIPT_DEFAULT;
	}
	
	
	public int OnInitialize(obj_id self) throws InterruptedException
	{
		if ((!isTangible (self)) || (isPlayer (self)))
		{
			detachScript(self, "conversation.trig_longo");
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
		detachScript (self, "conversation.trig_longo");
		
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
		
		if (trig_longo_condition_hasCompletedQuestLine (player, npc))
		{
			
			string_id message = new string_id (c_stringFile, "s_26");
			chat.chat (npc, player, message);
			
			return SCRIPT_CONTINUE;
		}
		
		if (trig_longo_condition_hasDeliveredRadioEpsilon (player, npc))
		{
			
			string_id message = new string_id (c_stringFile, "s_25");
			chat.chat (npc, player, message);
			
			return SCRIPT_CONTINUE;
		}
		
		if (trig_longo_condition_hasCompletedQuest2 (player, npc))
		{
			
			string_id message = new string_id (c_stringFile, "s_24");
			chat.chat (npc, player, message);
			
			return SCRIPT_CONTINUE;
		}
		
		if (trig_longo_condition_hasDeletedQuest2 (player, npc))
		{
			trig_longo_action_giveQuest2 (player, npc);
			
			string_id message = new string_id (c_stringFile, "s_22");
			chat.chat (npc, player, message);
			
			return SCRIPT_CONTINUE;
		}
		
		if (trig_longo_condition_hasTrigTask (player, npc))
		{
			
			string_id message = new string_id (c_stringFile, "s_59");
			int numberOfResponses = 0;
			
			boolean hasResponse = false;
			
			boolean hasResponse0 = false;
			if (trig_longo_condition__defaultCondition (player, npc))
			{
				++numberOfResponses;
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
				
				utils.setScriptVar (player, "conversation.trig_longo.branchId", 5);
				
				npcStartConversation (player, npc, "trig_longo", message, responses);
			}
			else
			{
				chat.chat (npc, player, message);
			}
			
			return SCRIPT_CONTINUE;
		}
		
		if (trig_longo_condition_isNotImmunized (player, npc))
		{
			
			string_id message = new string_id (c_stringFile, "s_73");
			chat.chat (npc, player, message);
			
			return SCRIPT_CONTINUE;
		}
		
		if (trig_longo_condition__defaultCondition (player, npc))
		{
			
			string_id message = new string_id (c_stringFile, "s_60");
			chat.chat (npc, player, message);
			
			return SCRIPT_CONTINUE;
		}
		
		chat.chat (npc, "Error: All conditions for OnStartNpcConversation were false.");
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int OnNpcConversationResponse(obj_id self, String conversationId, obj_id player, string_id response) throws InterruptedException
	{
		if (!conversationId.equals("trig_longo"))
		{
			return SCRIPT_CONTINUE;
		}
		
		obj_id npc = self;
		
		int branchId = utils.getIntScriptVar (player, "conversation.trig_longo.branchId");
		
		if (branchId == 5 && trig_longo_handleBranch5 (player, npc, response) == SCRIPT_CONTINUE)
		{
			return SCRIPT_CONTINUE;
		}
		
		if (branchId == 6 && trig_longo_handleBranch6 (player, npc, response) == SCRIPT_CONTINUE)
		{
			return SCRIPT_CONTINUE;
		}
		
		if (branchId == 7 && trig_longo_handleBranch7 (player, npc, response) == SCRIPT_CONTINUE)
		{
			return SCRIPT_CONTINUE;
		}
		
		if (branchId == 8 && trig_longo_handleBranch8 (player, npc, response) == SCRIPT_CONTINUE)
		{
			return SCRIPT_CONTINUE;
		}
		
		if (branchId == 9 && trig_longo_handleBranch9 (player, npc, response) == SCRIPT_CONTINUE)
		{
			return SCRIPT_CONTINUE;
		}
		
		if (branchId == 10 && trig_longo_handleBranch10 (player, npc, response) == SCRIPT_CONTINUE)
		{
			return SCRIPT_CONTINUE;
		}
		
		if (branchId == 11 && trig_longo_handleBranch11 (player, npc, response) == SCRIPT_CONTINUE)
		{
			return SCRIPT_CONTINUE;
		}
		
		if (branchId == 13 && trig_longo_handleBranch13 (player, npc, response) == SCRIPT_CONTINUE)
		{
			return SCRIPT_CONTINUE;
		}
		
		if (branchId == 14 && trig_longo_handleBranch14 (player, npc, response) == SCRIPT_CONTINUE)
		{
			return SCRIPT_CONTINUE;
		}
		
		chat.chat (npc, "Error: Fell through all branches and responses for OnNpcConversationResponse.");
		
		utils.removeScriptVar (player, "conversation.trig_longo.branchId");
		
		return SCRIPT_CONTINUE;
	}
	
}
