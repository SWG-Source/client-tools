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
import script.library.gcw;
import script.library.groundquests;
import script.library.trial;
import script.library.utils;


public class gcw_demoralized_soldier extends script.base_script
{
	public gcw_demoralized_soldier()
	{
	}
	String c_stringFile = "conversation/gcw_demoralized_soldier";
	
	
	public boolean gcw_demoralized_soldier_condition__defaultCondition(obj_id player, obj_id npc) throws InterruptedException
	{
		return true;
	}
	
	
	public boolean gcw_demoralized_soldier_condition_isReadyToTalkImperial(obj_id player, obj_id npc) throws InterruptedException
	{
		if ((utils.hasObjVar(npc, "faction_recruit_quest") && utils.hasScriptVar(npc, "faction_ent") && factions.isImperial(npc) && groundquests.isQuestActive(player, gcw.GCW_ENTERTAIN_RALLY)))
		{
			int questStatus = utils.getIntObjVar(npc, "faction_recruit_quest");
			obj_id faction_ent = utils.getObjIdScriptVar(npc, "faction_ent");
			if ((questStatus == 1) &&(faction_ent == player))
			{
				return true;
			}
		}
		return false;
	}
	
	
	public boolean gcw_demoralized_soldier_condition_isReadyToTalkRebel(obj_id player, obj_id npc) throws InterruptedException
	{
		if ((utils.hasObjVar(npc, "faction_recruit_quest") && utils.hasScriptVar(npc, "faction_ent") && factions.isRebel(npc) && groundquests.isQuestActive(player, gcw.GCW_ENTERTAIN_RALLY)))
		{
			int questStatus = utils.getIntObjVar(npc, "faction_recruit_quest");
			obj_id faction_ent = utils.getObjIdScriptVar(npc, "faction_ent");
			if ((questStatus == 1) &&(faction_ent == player))
			{
				return true;
			}
		}
		return false;
	}
	
	
	public void gcw_demoralized_soldier_action_convinced(obj_id player, obj_id npc) throws InterruptedException
	{
		groundquests.sendSignal(player, "gcw_rally_demoralized");
		trial.addNonInstanceFactionParticipant(player, npc);
		messageTo(npc, "convinced", null, 2.0f, false );
	}
	
	
	public void gcw_demoralized_soldier_action_notConvinced(obj_id player, obj_id npc) throws InterruptedException
	{
		messageTo(npc, "notConvinced", null, 0f, false );
	}
	
	
	public int gcw_demoralized_soldier_handleBranch1(obj_id player, obj_id npc, string_id response) throws InterruptedException
	{
		
		if (response.equals("s_6"))
		{
			
			if (gcw_demoralized_soldier_condition__defaultCondition (player, npc))
			{
				
				string_id message = new string_id (c_stringFile, "s_8");
				int numberOfResponses = 0;
				
				boolean hasResponse = false;
				
				boolean hasResponse0 = false;
				if (gcw_demoralized_soldier_condition__defaultCondition (player, npc))
				{
					++numberOfResponses;
					hasResponse = true;
					hasResponse0 = true;
				}
				
				boolean hasResponse1 = false;
				if (gcw_demoralized_soldier_condition__defaultCondition (player, npc))
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
						responses[responseIndex++] = new string_id (c_stringFile, "s_20");
					}
					
					if (hasResponse1)
					{
						responses[responseIndex++] = new string_id (c_stringFile, "s_17");
					}
					
					utils.setScriptVar (player, "conversation.gcw_demoralized_soldier.branchId", 2);
					
					npcSpeak (player, message);
					npcSetConversationResponses (player, responses);
				}
				else
				{
					utils.removeScriptVar (player, "conversation.gcw_demoralized_soldier.branchId");
					
					npcEndConversationWithMessage (player, message);
				}
				
				return SCRIPT_CONTINUE;
			}
			
		}
		
		if (response.equals("s_14"))
		{
			
			if (gcw_demoralized_soldier_condition__defaultCondition (player, npc))
			{
				
				string_id message = new string_id (c_stringFile, "s_22");
				int numberOfResponses = 0;
				
				boolean hasResponse = false;
				
				boolean hasResponse0 = false;
				if (gcw_demoralized_soldier_condition__defaultCondition (player, npc))
				{
					++numberOfResponses;
					hasResponse = true;
					hasResponse0 = true;
				}
				
				boolean hasResponse1 = false;
				if (gcw_demoralized_soldier_condition__defaultCondition (player, npc))
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
						responses[responseIndex++] = new string_id (c_stringFile, "s_23");
					}
					
					if (hasResponse1)
					{
						responses[responseIndex++] = new string_id (c_stringFile, "s_26");
					}
					
					utils.setScriptVar (player, "conversation.gcw_demoralized_soldier.branchId", 5);
					
					npcSpeak (player, message);
					npcSetConversationResponses (player, responses);
				}
				else
				{
					utils.removeScriptVar (player, "conversation.gcw_demoralized_soldier.branchId");
					
					npcEndConversationWithMessage (player, message);
				}
				
				return SCRIPT_CONTINUE;
			}
			
		}
		
		if (response.equals("s_30"))
		{
			
			if (gcw_demoralized_soldier_condition__defaultCondition (player, npc))
			{
				
				string_id message = new string_id (c_stringFile, "s_32");
				int numberOfResponses = 0;
				
				boolean hasResponse = false;
				
				boolean hasResponse0 = false;
				if (gcw_demoralized_soldier_condition__defaultCondition (player, npc))
				{
					++numberOfResponses;
					hasResponse = true;
					hasResponse0 = true;
				}
				
				boolean hasResponse1 = false;
				if (gcw_demoralized_soldier_condition__defaultCondition (player, npc))
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
						responses[responseIndex++] = new string_id (c_stringFile, "s_34");
					}
					
					if (hasResponse1)
					{
						responses[responseIndex++] = new string_id (c_stringFile, "s_38");
					}
					
					utils.setScriptVar (player, "conversation.gcw_demoralized_soldier.branchId", 8);
					
					npcSpeak (player, message);
					npcSetConversationResponses (player, responses);
				}
				else
				{
					utils.removeScriptVar (player, "conversation.gcw_demoralized_soldier.branchId");
					
					npcEndConversationWithMessage (player, message);
				}
				
				return SCRIPT_CONTINUE;
			}
			
		}
		
		return SCRIPT_DEFAULT;
	}
	
	
	public int gcw_demoralized_soldier_handleBranch2(obj_id player, obj_id npc, string_id response) throws InterruptedException
	{
		
		if (response.equals("s_20"))
		{
			
			if (gcw_demoralized_soldier_condition__defaultCondition (player, npc))
			{
				gcw_demoralized_soldier_action_convinced (player, npc);
				
				string_id message = new string_id (c_stringFile, "s_21");
				utils.removeScriptVar (player, "conversation.gcw_demoralized_soldier.branchId");
				
				npcEndConversationWithMessage (player, message);
				
				return SCRIPT_CONTINUE;
			}
			
		}
		
		if (response.equals("s_17"))
		{
			
			if (gcw_demoralized_soldier_condition__defaultCondition (player, npc))
			{
				gcw_demoralized_soldier_action_notConvinced (player, npc);
				
				string_id message = new string_id (c_stringFile, "s_19");
				utils.removeScriptVar (player, "conversation.gcw_demoralized_soldier.branchId");
				
				npcEndConversationWithMessage (player, message);
				
				return SCRIPT_CONTINUE;
			}
			
		}
		
		return SCRIPT_DEFAULT;
	}
	
	
	public int gcw_demoralized_soldier_handleBranch5(obj_id player, obj_id npc, string_id response) throws InterruptedException
	{
		
		if (response.equals("s_23"))
		{
			
			if (gcw_demoralized_soldier_condition__defaultCondition (player, npc))
			{
				gcw_demoralized_soldier_action_convinced (player, npc);
				
				string_id message = new string_id (c_stringFile, "s_25");
				utils.removeScriptVar (player, "conversation.gcw_demoralized_soldier.branchId");
				
				npcEndConversationWithMessage (player, message);
				
				return SCRIPT_CONTINUE;
			}
			
		}
		
		if (response.equals("s_26"))
		{
			
			if (gcw_demoralized_soldier_condition__defaultCondition (player, npc))
			{
				gcw_demoralized_soldier_action_notConvinced (player, npc);
				
				string_id message = new string_id (c_stringFile, "s_28");
				utils.removeScriptVar (player, "conversation.gcw_demoralized_soldier.branchId");
				
				npcEndConversationWithMessage (player, message);
				
				return SCRIPT_CONTINUE;
			}
			
		}
		
		return SCRIPT_DEFAULT;
	}
	
	
	public int gcw_demoralized_soldier_handleBranch8(obj_id player, obj_id npc, string_id response) throws InterruptedException
	{
		
		if (response.equals("s_34"))
		{
			
			if (gcw_demoralized_soldier_condition__defaultCondition (player, npc))
			{
				gcw_demoralized_soldier_action_convinced (player, npc);
				
				string_id message = new string_id (c_stringFile, "s_36");
				utils.removeScriptVar (player, "conversation.gcw_demoralized_soldier.branchId");
				
				npcEndConversationWithMessage (player, message);
				
				return SCRIPT_CONTINUE;
			}
			
		}
		
		if (response.equals("s_38"))
		{
			
			if (gcw_demoralized_soldier_condition__defaultCondition (player, npc))
			{
				gcw_demoralized_soldier_action_notConvinced (player, npc);
				
				string_id message = new string_id (c_stringFile, "s_40");
				utils.removeScriptVar (player, "conversation.gcw_demoralized_soldier.branchId");
				
				npcEndConversationWithMessage (player, message);
				
				return SCRIPT_CONTINUE;
			}
			
		}
		
		return SCRIPT_DEFAULT;
	}
	
	
	public int gcw_demoralized_soldier_handleBranch11(obj_id player, obj_id npc, string_id response) throws InterruptedException
	{
		
		if (response.equals("s_44"))
		{
			
			if (gcw_demoralized_soldier_condition__defaultCondition (player, npc))
			{
				
				string_id message = new string_id (c_stringFile, "s_46");
				int numberOfResponses = 0;
				
				boolean hasResponse = false;
				
				boolean hasResponse0 = false;
				if (gcw_demoralized_soldier_condition__defaultCondition (player, npc))
				{
					++numberOfResponses;
					hasResponse = true;
					hasResponse0 = true;
				}
				
				boolean hasResponse1 = false;
				if (gcw_demoralized_soldier_condition__defaultCondition (player, npc))
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
						responses[responseIndex++] = new string_id (c_stringFile, "s_48");
					}
					
					if (hasResponse1)
					{
						responses[responseIndex++] = new string_id (c_stringFile, "s_52");
					}
					
					utils.setScriptVar (player, "conversation.gcw_demoralized_soldier.branchId", 12);
					
					npcSpeak (player, message);
					npcSetConversationResponses (player, responses);
				}
				else
				{
					utils.removeScriptVar (player, "conversation.gcw_demoralized_soldier.branchId");
					
					npcEndConversationWithMessage (player, message);
				}
				
				return SCRIPT_CONTINUE;
			}
			
		}
		
		if (response.equals("s_56"))
		{
			
			if (gcw_demoralized_soldier_condition__defaultCondition (player, npc))
			{
				
				string_id message = new string_id (c_stringFile, "s_58");
				int numberOfResponses = 0;
				
				boolean hasResponse = false;
				
				boolean hasResponse0 = false;
				if (gcw_demoralized_soldier_condition__defaultCondition (player, npc))
				{
					++numberOfResponses;
					hasResponse = true;
					hasResponse0 = true;
				}
				
				boolean hasResponse1 = false;
				if (gcw_demoralized_soldier_condition__defaultCondition (player, npc))
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
						responses[responseIndex++] = new string_id (c_stringFile, "s_60");
					}
					
					if (hasResponse1)
					{
						responses[responseIndex++] = new string_id (c_stringFile, "s_64");
					}
					
					utils.setScriptVar (player, "conversation.gcw_demoralized_soldier.branchId", 15);
					
					npcSpeak (player, message);
					npcSetConversationResponses (player, responses);
				}
				else
				{
					utils.removeScriptVar (player, "conversation.gcw_demoralized_soldier.branchId");
					
					npcEndConversationWithMessage (player, message);
				}
				
				return SCRIPT_CONTINUE;
			}
			
		}
		
		if (response.equals("s_68"))
		{
			
			if (gcw_demoralized_soldier_condition__defaultCondition (player, npc))
			{
				
				string_id message = new string_id (c_stringFile, "s_70");
				int numberOfResponses = 0;
				
				boolean hasResponse = false;
				
				boolean hasResponse0 = false;
				if (gcw_demoralized_soldier_condition__defaultCondition (player, npc))
				{
					++numberOfResponses;
					hasResponse = true;
					hasResponse0 = true;
				}
				
				boolean hasResponse1 = false;
				if (gcw_demoralized_soldier_condition__defaultCondition (player, npc))
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
						responses[responseIndex++] = new string_id (c_stringFile, "s_76");
					}
					
					utils.setScriptVar (player, "conversation.gcw_demoralized_soldier.branchId", 18);
					
					npcSpeak (player, message);
					npcSetConversationResponses (player, responses);
				}
				else
				{
					utils.removeScriptVar (player, "conversation.gcw_demoralized_soldier.branchId");
					
					npcEndConversationWithMessage (player, message);
				}
				
				return SCRIPT_CONTINUE;
			}
			
		}
		
		return SCRIPT_DEFAULT;
	}
	
	
	public int gcw_demoralized_soldier_handleBranch12(obj_id player, obj_id npc, string_id response) throws InterruptedException
	{
		
		if (response.equals("s_48"))
		{
			
			if (gcw_demoralized_soldier_condition__defaultCondition (player, npc))
			{
				gcw_demoralized_soldier_action_convinced (player, npc);
				
				string_id message = new string_id (c_stringFile, "s_50");
				utils.removeScriptVar (player, "conversation.gcw_demoralized_soldier.branchId");
				
				npcEndConversationWithMessage (player, message);
				
				return SCRIPT_CONTINUE;
			}
			
		}
		
		if (response.equals("s_52"))
		{
			
			if (gcw_demoralized_soldier_condition__defaultCondition (player, npc))
			{
				gcw_demoralized_soldier_action_notConvinced (player, npc);
				
				string_id message = new string_id (c_stringFile, "s_54");
				utils.removeScriptVar (player, "conversation.gcw_demoralized_soldier.branchId");
				
				npcEndConversationWithMessage (player, message);
				
				return SCRIPT_CONTINUE;
			}
			
		}
		
		return SCRIPT_DEFAULT;
	}
	
	
	public int gcw_demoralized_soldier_handleBranch15(obj_id player, obj_id npc, string_id response) throws InterruptedException
	{
		
		if (response.equals("s_60"))
		{
			
			if (gcw_demoralized_soldier_condition__defaultCondition (player, npc))
			{
				gcw_demoralized_soldier_action_convinced (player, npc);
				
				string_id message = new string_id (c_stringFile, "s_62");
				utils.removeScriptVar (player, "conversation.gcw_demoralized_soldier.branchId");
				
				npcEndConversationWithMessage (player, message);
				
				return SCRIPT_CONTINUE;
			}
			
		}
		
		if (response.equals("s_64"))
		{
			
			if (gcw_demoralized_soldier_condition__defaultCondition (player, npc))
			{
				gcw_demoralized_soldier_action_notConvinced (player, npc);
				
				string_id message = new string_id (c_stringFile, "s_66");
				utils.removeScriptVar (player, "conversation.gcw_demoralized_soldier.branchId");
				
				npcEndConversationWithMessage (player, message);
				
				return SCRIPT_CONTINUE;
			}
			
		}
		
		return SCRIPT_DEFAULT;
	}
	
	
	public int gcw_demoralized_soldier_handleBranch18(obj_id player, obj_id npc, string_id response) throws InterruptedException
	{
		
		if (response.equals("s_72"))
		{
			
			if (gcw_demoralized_soldier_condition__defaultCondition (player, npc))
			{
				gcw_demoralized_soldier_action_convinced (player, npc);
				
				string_id message = new string_id (c_stringFile, "s_74");
				utils.removeScriptVar (player, "conversation.gcw_demoralized_soldier.branchId");
				
				npcEndConversationWithMessage (player, message);
				
				return SCRIPT_CONTINUE;
			}
			
		}
		
		if (response.equals("s_76"))
		{
			
			if (gcw_demoralized_soldier_condition__defaultCondition (player, npc))
			{
				gcw_demoralized_soldier_action_notConvinced (player, npc);
				
				string_id message = new string_id (c_stringFile, "s_78");
				utils.removeScriptVar (player, "conversation.gcw_demoralized_soldier.branchId");
				
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
			detachScript(self, "conversation.gcw_demoralized_soldier");
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
		detachScript (self, "conversation.gcw_demoralized_soldier");
		
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
		
		if (gcw_demoralized_soldier_condition_isReadyToTalkImperial (player, npc))
		{
			
			string_id message = new string_id (c_stringFile, "s_4");
			int numberOfResponses = 0;
			
			boolean hasResponse = false;
			
			boolean hasResponse0 = false;
			if (gcw_demoralized_soldier_condition__defaultCondition (player, npc))
			{
				++numberOfResponses;
				hasResponse = true;
				hasResponse0 = true;
			}
			
			boolean hasResponse1 = false;
			if (gcw_demoralized_soldier_condition__defaultCondition (player, npc))
			{
				++numberOfResponses;
				hasResponse = true;
				hasResponse1 = true;
			}
			
			boolean hasResponse2 = false;
			if (gcw_demoralized_soldier_condition__defaultCondition (player, npc))
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
					responses[responseIndex++] = new string_id (c_stringFile, "s_6");
				}
				
				if (hasResponse1)
				{
					responses[responseIndex++] = new string_id (c_stringFile, "s_14");
				}
				
				if (hasResponse2)
				{
					responses[responseIndex++] = new string_id (c_stringFile, "s_30");
				}
				
				utils.setScriptVar (player, "conversation.gcw_demoralized_soldier.branchId", 1);
				
				npcStartConversation (player, npc, "gcw_demoralized_soldier", message, responses);
			}
			else
			{
				chat.chat (npc, player, message);
			}
			
			return SCRIPT_CONTINUE;
		}
		
		if (gcw_demoralized_soldier_condition_isReadyToTalkRebel (player, npc))
		{
			
			string_id message = new string_id (c_stringFile, "s_42");
			int numberOfResponses = 0;
			
			boolean hasResponse = false;
			
			boolean hasResponse0 = false;
			if (gcw_demoralized_soldier_condition__defaultCondition (player, npc))
			{
				++numberOfResponses;
				hasResponse = true;
				hasResponse0 = true;
			}
			
			boolean hasResponse1 = false;
			if (gcw_demoralized_soldier_condition__defaultCondition (player, npc))
			{
				++numberOfResponses;
				hasResponse = true;
				hasResponse1 = true;
			}
			
			boolean hasResponse2 = false;
			if (gcw_demoralized_soldier_condition__defaultCondition (player, npc))
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
					responses[responseIndex++] = new string_id (c_stringFile, "s_44");
				}
				
				if (hasResponse1)
				{
					responses[responseIndex++] = new string_id (c_stringFile, "s_56");
				}
				
				if (hasResponse2)
				{
					responses[responseIndex++] = new string_id (c_stringFile, "s_68");
				}
				
				utils.setScriptVar (player, "conversation.gcw_demoralized_soldier.branchId", 11);
				
				npcStartConversation (player, npc, "gcw_demoralized_soldier", message, responses);
			}
			else
			{
				chat.chat (npc, player, message);
			}
			
			return SCRIPT_CONTINUE;
		}
		
		if (gcw_demoralized_soldier_condition__defaultCondition (player, npc))
		{
			
			string_id message = new string_id (c_stringFile, "s_80");
			chat.chat (npc, player, message);
			
			return SCRIPT_CONTINUE;
		}
		
		chat.chat (npc, "Error: All conditions for OnStartNpcConversation were false.");
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int OnNpcConversationResponse(obj_id self, String conversationId, obj_id player, string_id response) throws InterruptedException
	{
		if (!conversationId.equals("gcw_demoralized_soldier"))
		{
			return SCRIPT_CONTINUE;
		}
		
		obj_id npc = self;
		
		int branchId = utils.getIntScriptVar (player, "conversation.gcw_demoralized_soldier.branchId");
		
		if (branchId == 1 && gcw_demoralized_soldier_handleBranch1 (player, npc, response) == SCRIPT_CONTINUE)
		{
			return SCRIPT_CONTINUE;
		}
		
		if (branchId == 2 && gcw_demoralized_soldier_handleBranch2 (player, npc, response) == SCRIPT_CONTINUE)
		{
			return SCRIPT_CONTINUE;
		}
		
		if (branchId == 5 && gcw_demoralized_soldier_handleBranch5 (player, npc, response) == SCRIPT_CONTINUE)
		{
			return SCRIPT_CONTINUE;
		}
		
		if (branchId == 8 && gcw_demoralized_soldier_handleBranch8 (player, npc, response) == SCRIPT_CONTINUE)
		{
			return SCRIPT_CONTINUE;
		}
		
		if (branchId == 11 && gcw_demoralized_soldier_handleBranch11 (player, npc, response) == SCRIPT_CONTINUE)
		{
			return SCRIPT_CONTINUE;
		}
		
		if (branchId == 12 && gcw_demoralized_soldier_handleBranch12 (player, npc, response) == SCRIPT_CONTINUE)
		{
			return SCRIPT_CONTINUE;
		}
		
		if (branchId == 15 && gcw_demoralized_soldier_handleBranch15 (player, npc, response) == SCRIPT_CONTINUE)
		{
			return SCRIPT_CONTINUE;
		}
		
		if (branchId == 18 && gcw_demoralized_soldier_handleBranch18 (player, npc, response) == SCRIPT_CONTINUE)
		{
			return SCRIPT_CONTINUE;
		}
		
		chat.chat (npc, "Error: Fell through all branches and responses for OnNpcConversationResponse.");
		
		utils.removeScriptVar (player, "conversation.gcw_demoralized_soldier.branchId");
		
		return SCRIPT_CONTINUE;
	}
	
}
