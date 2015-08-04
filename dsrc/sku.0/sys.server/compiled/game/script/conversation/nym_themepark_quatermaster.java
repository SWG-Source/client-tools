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


public class nym_themepark_quatermaster extends script.base_script
{
	public nym_themepark_quatermaster()
	{
	}
	String c_stringFile = "conversation/nym_themepark_quatermaster";
	
	
	public boolean nym_themepark_quatermaster_condition__defaultCondition(obj_id player, obj_id npc) throws InterruptedException
	{
		return true;
	}
	
	
	public boolean nym_themepark_quatermaster_condition_isOnFirstQuest(obj_id player, obj_id npc) throws InterruptedException
	{
		faceTo(npc,player);
		return groundquests.isTaskActive(player, "u16_nym_themepark_parts_1", "findQuartermaster") || (groundquests.hasCompletedQuest(player, "u16_nym_themepark_parts_1") && !groundquests.isQuestActiveOrComplete(player, "u16_nym_themepark_smuggler_01"));
	}
	
	
	public boolean nym_themepark_quatermaster_condition_isOnSecondQuest(obj_id player, obj_id npc) throws InterruptedException
	{
		faceTo(npc,player);
		return groundquests.isTaskActive(player, "u16_nym_themepark_parts_2", "goQuartermaster") || (groundquests.hasCompletedQuest(player, "u16_nym_themepark_parts_2") && !groundquests.isQuestActiveOrComplete(player, "u16_nym_themepark_under_pressure"));
	}
	
	
	public boolean nym_themepark_quatermaster_condition_isOnThridQuest(obj_id player, obj_id npc) throws InterruptedException
	{
		faceTo(npc,player);
		return groundquests.isTaskActive(player, "u16_nym_themepark_parts_3", "goQuartermasterAgain") || (groundquests.hasCompletedQuest(player, "u16_nym_themepark_parts_3") && !groundquests.isQuestActiveOrComplete(player, "u16_nym_themepark_save_agent"));
	}
	
	
	public boolean nym_themepark_quatermaster_condition_isAlmostDone(obj_id player, obj_id npc) throws InterruptedException
	{
		faceTo(npc,player);
		return groundquests.isTaskActive(player, "u16_nym_themepark_safe", "backQuartermaster") || (groundquests.hasCompletedQuest(player, "u16_nym_themepark_safe") && !groundquests.isQuestActiveOrComplete(player, "u16_nym_themepark_repair_flocculation_chamber"));
	}
	
	
	public boolean nym_themepark_quatermaster_condition_hasFailedDelivery(obj_id player, obj_id npc) throws InterruptedException
	{
		faceTo(npc,player);
		return groundquests.isTaskActive(player, "u16_nym_themepark_save_agent_fail", "returnRaliaFail");
	}
	
	
	public boolean nym_themepark_quatermaster_condition_hasSmugglerQuestNotComplete(obj_id player, obj_id npc) throws InterruptedException
	{
		faceTo(npc, player);
		return groundquests.isQuestActive(player, "u16_nym_themepark_smuggler_01");
	}
	
	
	public boolean nym_themepark_quatermaster_condition_hasAgentQuestNotComplete(obj_id player, obj_id npc) throws InterruptedException
	{
		faceTo(npc, player);
		return groundquests.isQuestActive(player, "u16_nym_themepark_save_agent");
	}
	
	
	public boolean nym_themepark_quatermaster_condition_hasPressureQuestNotComplete(obj_id player, obj_id npc) throws InterruptedException
	{
		faceTo(npc, player);
		return groundquests.isQuestActive(player, "u16_nym_themepark_under_pressure");
	}
	
	
	public void nym_themepark_quatermaster_action_signalFirstJumperQuestDone(obj_id player, obj_id npc) throws InterruptedException
	{
		groundquests.sendSignal(player, "hasFoundQuartermaster");
	}
	
	
	public void nym_themepark_quatermaster_action_signalSecondJumperQuestDone(obj_id player, obj_id npc) throws InterruptedException
	{
		groundquests.sendSignal(player, "hasGoneQuartermaster");
	}
	
	
	public void nym_themepark_quatermaster_action_signalAgentSafe(obj_id player, obj_id npc) throws InterruptedException
	{
		groundquests.sendSignal(player, "hasGoneBackQuartermaster");
	}
	
	
	public void nym_themepark_quatermaster_action_grantSmugglerQuest(obj_id player, obj_id npc) throws InterruptedException
	{
		groundquests.grantQuest(player, "u16_nym_themepark_smuggler_01");
	}
	
	
	public void nym_themepark_quatermaster_action_grantAgentQuest(obj_id player, obj_id npc) throws InterruptedException
	{
		groundquests.grantQuest(player, "u16_nym_themepark_save_agent");
	}
	
	
	public void nym_themepark_quatermaster_action_grantPressureQuest(obj_id player, obj_id npc) throws InterruptedException
	{
		groundquests.grantQuest(player, "u16_nym_themepark_under_pressure");
	}
	
	
	public void nym_themepark_quatermaster_action_sendPlayerToFixChamber(obj_id player, obj_id npc) throws InterruptedException
	{
		groundquests.grantQuest(player, "u16_nym_themepark_repair_flocculation_chamber");
	}
	
	
	public void nym_themepark_quatermaster_action_clearDeliveryFailQuest(obj_id player, obj_id npc) throws InterruptedException
	{
		groundquests.clearQuest(player, "u16_nym_themepark_save_agent_fail");
		groundquests.clearQuest(player, "u16_nym_themepark_save_agent");
		nym_themepark_quatermaster_action_grantAgentQuest(player, npc);
	}
	
	
	public void nym_themepark_quatermaster_action_signalThirdJumperQuestDone(obj_id player, obj_id npc) throws InterruptedException
	{
		groundquests.sendSignal(player, "hasGoneQuartermasterAgain");
	}
	
	
	public int nym_themepark_quatermaster_handleBranch1(obj_id player, obj_id npc, string_id response) throws InterruptedException
	{
		
		if (response.equals("s_17"))
		{
			
			if (nym_themepark_quatermaster_condition__defaultCondition (player, npc))
			{
				doAnimationAction (npc, "thank");
				
				nym_themepark_quatermaster_action_sendPlayerToFixChamber (player, npc);
				
				string_id message = new string_id (c_stringFile, "s_18");
				utils.removeScriptVar (player, "conversation.nym_themepark_quatermaster.branchId");
				
				npcEndConversationWithMessage (player, message);
				
				return SCRIPT_CONTINUE;
			}
			
		}
		
		return SCRIPT_DEFAULT;
	}
	
	
	public int nym_themepark_quatermaster_handleBranch3(obj_id player, obj_id npc, string_id response) throws InterruptedException
	{
		
		if (response.equals("s_20"))
		{
			doAnimationAction (player, "shake_head_disgust");
			
			if (nym_themepark_quatermaster_condition__defaultCondition (player, npc))
			{
				doAnimationAction (npc, "point_down");
				
				nym_themepark_quatermaster_action_clearDeliveryFailQuest (player, npc);
				
				string_id message = new string_id (c_stringFile, "s_21");
				utils.removeScriptVar (player, "conversation.nym_themepark_quatermaster.branchId");
				
				npcEndConversationWithMessage (player, message);
				
				return SCRIPT_CONTINUE;
			}
			
		}
		
		return SCRIPT_DEFAULT;
	}
	
	
	public int nym_themepark_quatermaster_handleBranch5(obj_id player, obj_id npc, string_id response) throws InterruptedException
	{
		
		if (response.equals("s_74"))
		{
			doAnimationAction (player, "taken_aback");
			
			if (nym_themepark_quatermaster_condition__defaultCondition (player, npc))
			{
				doAnimationAction (npc, "point_away");
				
				string_id message = new string_id (c_stringFile, "s_75");
				utils.removeScriptVar (player, "conversation.nym_themepark_quatermaster.branchId");
				
				npcEndConversationWithMessage (player, message);
				
				return SCRIPT_CONTINUE;
			}
			
		}
		
		return SCRIPT_DEFAULT;
	}
	
	
	public int nym_themepark_quatermaster_handleBranch7(obj_id player, obj_id npc, string_id response) throws InterruptedException
	{
		
		if (response.equals("s_15"))
		{
			
			if (nym_themepark_quatermaster_condition__defaultCondition (player, npc))
			{
				doAnimationAction (npc, "wave_finger_warning");
				
				string_id message = new string_id (c_stringFile, "s_23");
				int numberOfResponses = 0;
				
				boolean hasResponse = false;
				
				boolean hasResponse0 = false;
				if (nym_themepark_quatermaster_condition__defaultCondition (player, npc))
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
					
					utils.setScriptVar (player, "conversation.nym_themepark_quatermaster.branchId", 8);
					
					npcSpeak (player, message);
					npcSetConversationResponses (player, responses);
				}
				else
				{
					utils.removeScriptVar (player, "conversation.nym_themepark_quatermaster.branchId");
					
					npcEndConversationWithMessage (player, message);
				}
				
				return SCRIPT_CONTINUE;
			}
			
		}
		
		return SCRIPT_DEFAULT;
	}
	
	
	public int nym_themepark_quatermaster_handleBranch8(obj_id player, obj_id npc, string_id response) throws InterruptedException
	{
		
		if (response.equals("s_46"))
		{
			
			if (nym_themepark_quatermaster_condition__defaultCondition (player, npc))
			{
				doAnimationAction (npc, "explain");
				
				string_id message = new string_id (c_stringFile, "s_47");
				int numberOfResponses = 0;
				
				boolean hasResponse = false;
				
				boolean hasResponse0 = false;
				if (nym_themepark_quatermaster_condition__defaultCondition (player, npc))
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
					
					utils.setScriptVar (player, "conversation.nym_themepark_quatermaster.branchId", 9);
					
					npcSpeak (player, message);
					npcSetConversationResponses (player, responses);
				}
				else
				{
					utils.removeScriptVar (player, "conversation.nym_themepark_quatermaster.branchId");
					
					npcEndConversationWithMessage (player, message);
				}
				
				return SCRIPT_CONTINUE;
			}
			
		}
		
		return SCRIPT_DEFAULT;
	}
	
	
	public int nym_themepark_quatermaster_handleBranch9(obj_id player, obj_id npc, string_id response) throws InterruptedException
	{
		
		if (response.equals("s_48"))
		{
			
			if (nym_themepark_quatermaster_condition__defaultCondition (player, npc))
			{
				doAnimationAction (npc, "gesticulate_wildly");
				
				string_id message = new string_id (c_stringFile, "s_49");
				int numberOfResponses = 0;
				
				boolean hasResponse = false;
				
				boolean hasResponse0 = false;
				if (nym_themepark_quatermaster_condition__defaultCondition (player, npc))
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
					
					utils.setScriptVar (player, "conversation.nym_themepark_quatermaster.branchId", 10);
					
					npcSpeak (player, message);
					npcSetConversationResponses (player, responses);
				}
				else
				{
					utils.removeScriptVar (player, "conversation.nym_themepark_quatermaster.branchId");
					
					npcEndConversationWithMessage (player, message);
				}
				
				return SCRIPT_CONTINUE;
			}
			
		}
		
		return SCRIPT_DEFAULT;
	}
	
	
	public int nym_themepark_quatermaster_handleBranch10(obj_id player, obj_id npc, string_id response) throws InterruptedException
	{
		
		if (response.equals("s_50"))
		{
			doAnimationAction (player, "taken_aback");
			
			if (nym_themepark_quatermaster_condition__defaultCondition (player, npc))
			{
				doAnimationAction (npc, "point_away");
				
				nym_themepark_quatermaster_action_grantAgentQuest (player, npc);
				
				string_id message = new string_id (c_stringFile, "s_51");
				utils.removeScriptVar (player, "conversation.nym_themepark_quatermaster.branchId");
				
				npcEndConversationWithMessage (player, message);
				
				return SCRIPT_CONTINUE;
			}
			
		}
		
		return SCRIPT_DEFAULT;
	}
	
	
	public int nym_themepark_quatermaster_handleBranch12(obj_id player, obj_id npc, string_id response) throws InterruptedException
	{
		
		if (response.equals("s_71"))
		{
			
			if (nym_themepark_quatermaster_condition__defaultCondition (player, npc))
			{
				doAnimationAction (npc, "tap_foot");
				
				string_id message = new string_id (c_stringFile, "s_72");
				utils.removeScriptVar (player, "conversation.nym_themepark_quatermaster.branchId");
				
				npcEndConversationWithMessage (player, message);
				
				return SCRIPT_CONTINUE;
			}
			
		}
		
		return SCRIPT_DEFAULT;
	}
	
	
	public int nym_themepark_quatermaster_handleBranch14(obj_id player, obj_id npc, string_id response) throws InterruptedException
	{
		
		if (response.equals("s_36"))
		{
			doAnimationAction (player, "shake_head_no");
			
			if (nym_themepark_quatermaster_condition__defaultCondition (player, npc))
			{
				doAnimationAction (npc, "rub_chin_thoughtful");
				
				string_id message = new string_id (c_stringFile, "s_38");
				int numberOfResponses = 0;
				
				boolean hasResponse = false;
				
				boolean hasResponse0 = false;
				if (nym_themepark_quatermaster_condition__defaultCondition (player, npc))
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
					
					utils.setScriptVar (player, "conversation.nym_themepark_quatermaster.branchId", 15);
					
					npcSpeak (player, message);
					npcSetConversationResponses (player, responses);
				}
				else
				{
					utils.removeScriptVar (player, "conversation.nym_themepark_quatermaster.branchId");
					
					npcEndConversationWithMessage (player, message);
				}
				
				return SCRIPT_CONTINUE;
			}
			
		}
		
		return SCRIPT_DEFAULT;
	}
	
	
	public int nym_themepark_quatermaster_handleBranch15(obj_id player, obj_id npc, string_id response) throws InterruptedException
	{
		
		if (response.equals("s_40"))
		{
			
			if (nym_themepark_quatermaster_condition__defaultCondition (player, npc))
			{
				
				string_id message = new string_id (c_stringFile, "s_42");
				int numberOfResponses = 0;
				
				boolean hasResponse = false;
				
				boolean hasResponse0 = false;
				if (nym_themepark_quatermaster_condition__defaultCondition (player, npc))
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
						responses[responseIndex++] = new string_id (c_stringFile, "s_44");
					}
					
					utils.setScriptVar (player, "conversation.nym_themepark_quatermaster.branchId", 16);
					
					npcSpeak (player, message);
					npcSetConversationResponses (player, responses);
				}
				else
				{
					utils.removeScriptVar (player, "conversation.nym_themepark_quatermaster.branchId");
					
					npcEndConversationWithMessage (player, message);
				}
				
				return SCRIPT_CONTINUE;
			}
			
		}
		
		return SCRIPT_DEFAULT;
	}
	
	
	public int nym_themepark_quatermaster_handleBranch16(obj_id player, obj_id npc, string_id response) throws InterruptedException
	{
		
		if (response.equals("s_44"))
		{
			
			if (nym_themepark_quatermaster_condition__defaultCondition (player, npc))
			{
				doAnimationAction (npc, "explain");
				
				string_id message = new string_id (c_stringFile, "s_52");
				int numberOfResponses = 0;
				
				boolean hasResponse = false;
				
				boolean hasResponse0 = false;
				if (nym_themepark_quatermaster_condition__defaultCondition (player, npc))
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
						responses[responseIndex++] = new string_id (c_stringFile, "s_54");
					}
					
					utils.setScriptVar (player, "conversation.nym_themepark_quatermaster.branchId", 17);
					
					npcSpeak (player, message);
					npcSetConversationResponses (player, responses);
				}
				else
				{
					utils.removeScriptVar (player, "conversation.nym_themepark_quatermaster.branchId");
					
					npcEndConversationWithMessage (player, message);
				}
				
				return SCRIPT_CONTINUE;
			}
			
		}
		
		return SCRIPT_DEFAULT;
	}
	
	
	public int nym_themepark_quatermaster_handleBranch17(obj_id player, obj_id npc, string_id response) throws InterruptedException
	{
		
		if (response.equals("s_54"))
		{
			
			if (nym_themepark_quatermaster_condition__defaultCondition (player, npc))
			{
				nym_themepark_quatermaster_action_grantPressureQuest (player, npc);
				
				string_id message = new string_id (c_stringFile, "s_56");
				utils.removeScriptVar (player, "conversation.nym_themepark_quatermaster.branchId");
				
				npcEndConversationWithMessage (player, message);
				
				return SCRIPT_CONTINUE;
			}
			
		}
		
		return SCRIPT_DEFAULT;
	}
	
	
	public int nym_themepark_quatermaster_handleBranch19(obj_id player, obj_id npc, string_id response) throws InterruptedException
	{
		
		if (response.equals("s_68"))
		{
			
			if (nym_themepark_quatermaster_condition__defaultCondition (player, npc))
			{
				doAnimationAction (npc, "tap_foot");
				
				string_id message = new string_id (c_stringFile, "s_69");
				utils.removeScriptVar (player, "conversation.nym_themepark_quatermaster.branchId");
				
				npcEndConversationWithMessage (player, message);
				
				return SCRIPT_CONTINUE;
			}
			
		}
		
		return SCRIPT_DEFAULT;
	}
	
	
	public int nym_themepark_quatermaster_handleBranch21(obj_id player, obj_id npc, string_id response) throws InterruptedException
	{
		
		if (response.equals("s_63"))
		{
			
			if (nym_themepark_quatermaster_condition__defaultCondition (player, npc))
			{
				
				string_id message = new string_id (c_stringFile, "s_65");
				int numberOfResponses = 0;
				
				boolean hasResponse = false;
				
				boolean hasResponse0 = false;
				if (nym_themepark_quatermaster_condition__defaultCondition (player, npc))
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
					
					utils.setScriptVar (player, "conversation.nym_themepark_quatermaster.branchId", 22);
					
					npcSpeak (player, message);
					npcSetConversationResponses (player, responses);
				}
				else
				{
					utils.removeScriptVar (player, "conversation.nym_themepark_quatermaster.branchId");
					
					npcEndConversationWithMessage (player, message);
				}
				
				return SCRIPT_CONTINUE;
			}
			
		}
		
		return SCRIPT_DEFAULT;
	}
	
	
	public int nym_themepark_quatermaster_handleBranch22(obj_id player, obj_id npc, string_id response) throws InterruptedException
	{
		
		if (response.equals("s_76"))
		{
			
			if (nym_themepark_quatermaster_condition__defaultCondition (player, npc))
			{
				doAnimationAction (npc, "rub_chin_thoughtful");
				
				string_id message = new string_id (c_stringFile, "s_78");
				int numberOfResponses = 0;
				
				boolean hasResponse = false;
				
				boolean hasResponse0 = false;
				if (nym_themepark_quatermaster_condition__defaultCondition (player, npc))
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
						responses[responseIndex++] = new string_id (c_stringFile, "s_80");
					}
					
					utils.setScriptVar (player, "conversation.nym_themepark_quatermaster.branchId", 23);
					
					npcSpeak (player, message);
					npcSetConversationResponses (player, responses);
				}
				else
				{
					utils.removeScriptVar (player, "conversation.nym_themepark_quatermaster.branchId");
					
					npcEndConversationWithMessage (player, message);
				}
				
				return SCRIPT_CONTINUE;
			}
			
		}
		
		return SCRIPT_DEFAULT;
	}
	
	
	public int nym_themepark_quatermaster_handleBranch23(obj_id player, obj_id npc, string_id response) throws InterruptedException
	{
		
		if (response.equals("s_80"))
		{
			
			if (nym_themepark_quatermaster_condition__defaultCondition (player, npc))
			{
				nym_themepark_quatermaster_action_grantSmugglerQuest (player, npc);
				
				string_id message = new string_id (c_stringFile, "s_82");
				utils.removeScriptVar (player, "conversation.nym_themepark_quatermaster.branchId");
				
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
			detachScript(self, "conversation.nym_themepark_quatermaster");
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
		detachScript (self, "conversation.nym_themepark_quatermaster");
		
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
		
		if (nym_themepark_quatermaster_condition_isAlmostDone (player, npc))
		{
			doAnimationAction (npc, "celebrate");
			
			nym_themepark_quatermaster_action_signalAgentSafe (player, npc);
			
			string_id message = new string_id (c_stringFile, "s_16");
			int numberOfResponses = 0;
			
			boolean hasResponse = false;
			
			boolean hasResponse0 = false;
			if (nym_themepark_quatermaster_condition__defaultCondition (player, npc))
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
					responses[responseIndex++] = new string_id (c_stringFile, "s_17");
				}
				
				utils.setScriptVar (player, "conversation.nym_themepark_quatermaster.branchId", 1);
				
				npcStartConversation (player, npc, "nym_themepark_quatermaster", message, responses);
			}
			else
			{
				chat.chat (npc, player, message);
			}
			
			return SCRIPT_CONTINUE;
		}
		
		if (nym_themepark_quatermaster_condition_hasFailedDelivery (player, npc))
		{
			doAnimationAction (npc, "gesticulate_wildly");
			
			string_id message = new string_id (c_stringFile, "s_19");
			int numberOfResponses = 0;
			
			boolean hasResponse = false;
			
			boolean hasResponse0 = false;
			if (nym_themepark_quatermaster_condition__defaultCondition (player, npc))
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
					responses[responseIndex++] = new string_id (c_stringFile, "s_20");
				}
				
				utils.setScriptVar (player, "conversation.nym_themepark_quatermaster.branchId", 3);
				
				npcStartConversation (player, npc, "nym_themepark_quatermaster", message, responses);
			}
			else
			{
				chat.chat (npc, player, message);
			}
			
			return SCRIPT_CONTINUE;
		}
		
		if (nym_themepark_quatermaster_condition_hasAgentQuestNotComplete (player, npc))
		{
			doAnimationAction (npc, "point_accusingly");
			
			string_id message = new string_id (c_stringFile, "s_73");
			int numberOfResponses = 0;
			
			boolean hasResponse = false;
			
			boolean hasResponse0 = false;
			if (nym_themepark_quatermaster_condition__defaultCondition (player, npc))
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
					responses[responseIndex++] = new string_id (c_stringFile, "s_74");
				}
				
				utils.setScriptVar (player, "conversation.nym_themepark_quatermaster.branchId", 5);
				
				npcStartConversation (player, npc, "nym_themepark_quatermaster", message, responses);
			}
			else
			{
				chat.chat (npc, player, message);
			}
			
			return SCRIPT_CONTINUE;
		}
		
		if (nym_themepark_quatermaster_condition_isOnThridQuest (player, npc))
		{
			doAnimationAction (npc, "tap_foot");
			
			nym_themepark_quatermaster_action_signalThirdJumperQuestDone (player, npc);
			
			string_id message = new string_id (c_stringFile, "s_13");
			int numberOfResponses = 0;
			
			boolean hasResponse = false;
			
			boolean hasResponse0 = false;
			if (nym_themepark_quatermaster_condition__defaultCondition (player, npc))
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
					responses[responseIndex++] = new string_id (c_stringFile, "s_15");
				}
				
				utils.setScriptVar (player, "conversation.nym_themepark_quatermaster.branchId", 7);
				
				npcStartConversation (player, npc, "nym_themepark_quatermaster", message, responses);
			}
			else
			{
				chat.chat (npc, player, message);
			}
			
			return SCRIPT_CONTINUE;
		}
		
		if (nym_themepark_quatermaster_condition_hasPressureQuestNotComplete (player, npc))
		{
			doAnimationAction (npc, "cover_mouth");
			
			string_id message = new string_id (c_stringFile, "s_70");
			int numberOfResponses = 0;
			
			boolean hasResponse = false;
			
			boolean hasResponse0 = false;
			if (nym_themepark_quatermaster_condition__defaultCondition (player, npc))
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
				
				utils.setScriptVar (player, "conversation.nym_themepark_quatermaster.branchId", 12);
				
				npcStartConversation (player, npc, "nym_themepark_quatermaster", message, responses);
			}
			else
			{
				chat.chat (npc, player, message);
			}
			
			return SCRIPT_CONTINUE;
		}
		
		if (nym_themepark_quatermaster_condition_isOnSecondQuest (player, npc))
		{
			nym_themepark_quatermaster_action_signalSecondJumperQuestDone (player, npc);
			
			string_id message = new string_id (c_stringFile, "s_34");
			int numberOfResponses = 0;
			
			boolean hasResponse = false;
			
			boolean hasResponse0 = false;
			if (nym_themepark_quatermaster_condition__defaultCondition (player, npc))
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
					responses[responseIndex++] = new string_id (c_stringFile, "s_36");
				}
				
				utils.setScriptVar (player, "conversation.nym_themepark_quatermaster.branchId", 14);
				
				npcStartConversation (player, npc, "nym_themepark_quatermaster", message, responses);
			}
			else
			{
				chat.chat (npc, player, message);
			}
			
			return SCRIPT_CONTINUE;
		}
		
		if (nym_themepark_quatermaster_condition_hasSmugglerQuestNotComplete (player, npc))
		{
			doAnimationAction (npc, "tap_foot");
			
			string_id message = new string_id (c_stringFile, "s_67");
			int numberOfResponses = 0;
			
			boolean hasResponse = false;
			
			boolean hasResponse0 = false;
			if (nym_themepark_quatermaster_condition__defaultCondition (player, npc))
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
				
				utils.setScriptVar (player, "conversation.nym_themepark_quatermaster.branchId", 19);
				
				npcStartConversation (player, npc, "nym_themepark_quatermaster", message, responses);
			}
			else
			{
				chat.chat (npc, player, message);
			}
			
			return SCRIPT_CONTINUE;
		}
		
		if (nym_themepark_quatermaster_condition_isOnFirstQuest (player, npc))
		{
			doAnimationAction (npc, "tap_foot");
			
			nym_themepark_quatermaster_action_signalFirstJumperQuestDone (player, npc);
			
			string_id message = new string_id (c_stringFile, "s_61");
			int numberOfResponses = 0;
			
			boolean hasResponse = false;
			
			boolean hasResponse0 = false;
			if (nym_themepark_quatermaster_condition__defaultCondition (player, npc))
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
				
				utils.setScriptVar (player, "conversation.nym_themepark_quatermaster.branchId", 21);
				
				npcStartConversation (player, npc, "nym_themepark_quatermaster", message, responses);
			}
			else
			{
				chat.chat (npc, player, message);
			}
			
			return SCRIPT_CONTINUE;
		}
		
		if (nym_themepark_quatermaster_condition__defaultCondition (player, npc))
		{
			doAnimationAction (npc, "point_away");
			
			string_id message = new string_id (c_stringFile, "s_84");
			chat.chat (npc, player, message);
			
			return SCRIPT_CONTINUE;
		}
		
		chat.chat (npc, "Error: All conditions for OnStartNpcConversation were false.");
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int OnNpcConversationResponse(obj_id self, String conversationId, obj_id player, string_id response) throws InterruptedException
	{
		if (!conversationId.equals("nym_themepark_quatermaster"))
		{
			return SCRIPT_CONTINUE;
		}
		
		obj_id npc = self;
		
		int branchId = utils.getIntScriptVar (player, "conversation.nym_themepark_quatermaster.branchId");
		
		if (branchId == 1 && nym_themepark_quatermaster_handleBranch1 (player, npc, response) == SCRIPT_CONTINUE)
		{
			return SCRIPT_CONTINUE;
		}
		
		if (branchId == 3 && nym_themepark_quatermaster_handleBranch3 (player, npc, response) == SCRIPT_CONTINUE)
		{
			return SCRIPT_CONTINUE;
		}
		
		if (branchId == 5 && nym_themepark_quatermaster_handleBranch5 (player, npc, response) == SCRIPT_CONTINUE)
		{
			return SCRIPT_CONTINUE;
		}
		
		if (branchId == 7 && nym_themepark_quatermaster_handleBranch7 (player, npc, response) == SCRIPT_CONTINUE)
		{
			return SCRIPT_CONTINUE;
		}
		
		if (branchId == 8 && nym_themepark_quatermaster_handleBranch8 (player, npc, response) == SCRIPT_CONTINUE)
		{
			return SCRIPT_CONTINUE;
		}
		
		if (branchId == 9 && nym_themepark_quatermaster_handleBranch9 (player, npc, response) == SCRIPT_CONTINUE)
		{
			return SCRIPT_CONTINUE;
		}
		
		if (branchId == 10 && nym_themepark_quatermaster_handleBranch10 (player, npc, response) == SCRIPT_CONTINUE)
		{
			return SCRIPT_CONTINUE;
		}
		
		if (branchId == 12 && nym_themepark_quatermaster_handleBranch12 (player, npc, response) == SCRIPT_CONTINUE)
		{
			return SCRIPT_CONTINUE;
		}
		
		if (branchId == 14 && nym_themepark_quatermaster_handleBranch14 (player, npc, response) == SCRIPT_CONTINUE)
		{
			return SCRIPT_CONTINUE;
		}
		
		if (branchId == 15 && nym_themepark_quatermaster_handleBranch15 (player, npc, response) == SCRIPT_CONTINUE)
		{
			return SCRIPT_CONTINUE;
		}
		
		if (branchId == 16 && nym_themepark_quatermaster_handleBranch16 (player, npc, response) == SCRIPT_CONTINUE)
		{
			return SCRIPT_CONTINUE;
		}
		
		if (branchId == 17 && nym_themepark_quatermaster_handleBranch17 (player, npc, response) == SCRIPT_CONTINUE)
		{
			return SCRIPT_CONTINUE;
		}
		
		if (branchId == 19 && nym_themepark_quatermaster_handleBranch19 (player, npc, response) == SCRIPT_CONTINUE)
		{
			return SCRIPT_CONTINUE;
		}
		
		if (branchId == 21 && nym_themepark_quatermaster_handleBranch21 (player, npc, response) == SCRIPT_CONTINUE)
		{
			return SCRIPT_CONTINUE;
		}
		
		if (branchId == 22 && nym_themepark_quatermaster_handleBranch22 (player, npc, response) == SCRIPT_CONTINUE)
		{
			return SCRIPT_CONTINUE;
		}
		
		if (branchId == 23 && nym_themepark_quatermaster_handleBranch23 (player, npc, response) == SCRIPT_CONTINUE)
		{
			return SCRIPT_CONTINUE;
		}
		
		chat.chat (npc, "Error: Fell through all branches and responses for OnNpcConversationResponse.");
		
		utils.removeScriptVar (player, "conversation.nym_themepark_quatermaster.branchId");
		
		return SCRIPT_CONTINUE;
	}
	
}
