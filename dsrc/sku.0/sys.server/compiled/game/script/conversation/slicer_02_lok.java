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
import script.library.prose;
import script.library.utils;


public class slicer_02_lok extends script.base_script
{
	public slicer_02_lok()
	{
	}
	String c_stringFile = "conversation/slicer_02_lok";
	
	
	public boolean slicer_02_lok_condition__defaultCondition(obj_id player, obj_id npc) throws InterruptedException
	{
		return true;
	}
	
	
	public boolean slicer_02_lok_condition_isGoingToDungeon(obj_id player, obj_id npc) throws InterruptedException
	{
		faceTo(npc, player);
		return groundquests.isTaskActive(player, "quest_06_conspire_cale", "GoToLabratory");
	}
	
	
	public boolean slicer_02_lok_condition_isGoingToDathomir(obj_id player, obj_id npc) throws InterruptedException
	{
		faceTo(npc, player);
		return groundquests.isTaskActive(player, "quest_08_dathomir_outpost", "travelDathomir");
	}
	
	
	public boolean slicer_02_lok_condition_isReadyForDungeon(obj_id player, obj_id npc) throws InterruptedException
	{
		faceTo(npc, player);
		return groundquests.isTaskActive(player, "quest_06_conspire_cale", "TravelToLokOutpost");
	}
	
	
	public boolean slicer_02_lok_condition_hasCompletedlokDungeon(obj_id player, obj_id npc) throws InterruptedException
	{
		faceTo(npc, player);
		return groundquests.isTaskActive(player, "quest_07_descend_into_labratory", "TakeDataPadCale");
	}
	
	
	public boolean slicer_02_lok_condition_canceledQuest06(obj_id player, obj_id npc) throws InterruptedException
	{
		faceTo(npc, player);
		return !groundquests.isQuestActiveOrComplete(player, "quest_06_conspire_cale") && (groundquests.hasCompletedQuest(player, "quest_05_frame_valarians_failed_assassin") || groundquests.hasCompletedQuest(player, "quest_05_frame_valarians_success_assassin"));
	}
	
	
	public boolean slicer_02_lok_condition_canceledQuest07(obj_id player, obj_id npc) throws InterruptedException
	{
		faceTo(npc, player);
		return groundquests.hasCompletedQuest(player, "quest_06_conspire_cale") && !groundquests.isQuestActiveOrComplete(player, "quest_07_descend_into_labratory");
	}
	
	
	public boolean slicer_02_lok_condition_canceledQuest08(obj_id player, obj_id npc) throws InterruptedException
	{
		faceTo(npc, player);
		return groundquests.hasCompletedQuest(player, "quest_07_descend_into_labratory") && !groundquests.isQuestActiveOrComplete(player, "quest_08_dathomir_outpost");
	}
	
	
	public void slicer_02_lok_action_sendSignalLokDungeon(obj_id player, obj_id npc) throws InterruptedException
	{
		faceTo(npc, player);
		groundquests.sendSignal(player, "TravelToLokOutpostComplete");
	}
	
	
	public void slicer_02_lok_action_grantQuest07WithWaypoint(obj_id player, obj_id npc) throws InterruptedException
	{
		faceTo(npc, player);
		groundquests.grantQuest(player, "quest_07_descend_into_labratory");
		slicer_02_lok_action_giveDungeonWaypoint(player,npc);
	}
	
	
	public void slicer_02_lok_action_sendDathomirSignal(obj_id player, obj_id npc) throws InterruptedException
	{
		faceTo(npc, player);
		groundquests.sendSignal(player, "TookDataPadCale");
	}
	
	
	public void slicer_02_lok_action_giveDungeonWaypoint(obj_id player, obj_id npc) throws InterruptedException
	{
		faceTo(npc, player);
		location loc = new location(-415, 92, 7608, "lok");
		obj_id maraWpt = createWaypointInDatapad(player, loc);
		setWaypointName(maraWpt, "Secret Laboratory");
		setWaypointActive(maraWpt, true);
	}
	
	
	public void slicer_02_lok_action_giveQuest08(obj_id player, obj_id npc) throws InterruptedException
	{
		groundquests.grantQuest(player, "quest_08_dathomir_outpost");
	}
	
	
	public int slicer_02_lok_handleBranch6(obj_id player, obj_id npc, string_id response) throws InterruptedException
	{
		
		if (response.equals("s_131"))
		{
			
			if (slicer_02_lok_condition__defaultCondition (player, npc))
			{
				
				string_id message = new string_id (c_stringFile, "s_135");
				int numberOfResponses = 0;
				
				boolean hasResponse = false;
				
				boolean hasResponse0 = false;
				if (slicer_02_lok_condition__defaultCondition (player, npc))
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
						responses[responseIndex++] = new string_id (c_stringFile, "s_136");
					}
					
					utils.setScriptVar (player, "conversation.slicer_02_lok.branchId", 7);
					
					npcSpeak (player, message);
					npcSetConversationResponses (player, responses);
				}
				else
				{
					utils.removeScriptVar (player, "conversation.slicer_02_lok.branchId");
					
					npcEndConversationWithMessage (player, message);
				}
				
				return SCRIPT_CONTINUE;
			}
			
		}
		
		return SCRIPT_DEFAULT;
	}
	
	
	public int slicer_02_lok_handleBranch7(obj_id player, obj_id npc, string_id response) throws InterruptedException
	{
		
		if (response.equals("s_136"))
		{
			
			if (slicer_02_lok_condition__defaultCondition (player, npc))
			{
				
				string_id message = new string_id (c_stringFile, "s_137");
				int numberOfResponses = 0;
				
				boolean hasResponse = false;
				
				boolean hasResponse0 = false;
				if (slicer_02_lok_condition__defaultCondition (player, npc))
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
					
					utils.setScriptVar (player, "conversation.slicer_02_lok.branchId", 8);
					
					npcSpeak (player, message);
					npcSetConversationResponses (player, responses);
				}
				else
				{
					utils.removeScriptVar (player, "conversation.slicer_02_lok.branchId");
					
					npcEndConversationWithMessage (player, message);
				}
				
				return SCRIPT_CONTINUE;
			}
			
		}
		
		return SCRIPT_DEFAULT;
	}
	
	
	public int slicer_02_lok_handleBranch8(obj_id player, obj_id npc, string_id response) throws InterruptedException
	{
		
		if (response.equals("s_138"))
		{
			
			if (slicer_02_lok_condition__defaultCondition (player, npc))
			{
				
				string_id message = new string_id (c_stringFile, "s_139");
				int numberOfResponses = 0;
				
				boolean hasResponse = false;
				
				boolean hasResponse0 = false;
				if (slicer_02_lok_condition__defaultCondition (player, npc))
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
					
					utils.setScriptVar (player, "conversation.slicer_02_lok.branchId", 9);
					
					npcSpeak (player, message);
					npcSetConversationResponses (player, responses);
				}
				else
				{
					utils.removeScriptVar (player, "conversation.slicer_02_lok.branchId");
					
					npcEndConversationWithMessage (player, message);
				}
				
				return SCRIPT_CONTINUE;
			}
			
		}
		
		return SCRIPT_DEFAULT;
	}
	
	
	public int slicer_02_lok_handleBranch9(obj_id player, obj_id npc, string_id response) throws InterruptedException
	{
		
		if (response.equals("s_140"))
		{
			
			if (slicer_02_lok_condition__defaultCondition (player, npc))
			{
				
				string_id message = new string_id (c_stringFile, "s_141");
				int numberOfResponses = 0;
				
				boolean hasResponse = false;
				
				boolean hasResponse0 = false;
				if (slicer_02_lok_condition__defaultCondition (player, npc))
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
						responses[responseIndex++] = new string_id (c_stringFile, "s_142");
					}
					
					utils.setScriptVar (player, "conversation.slicer_02_lok.branchId", 10);
					
					npcSpeak (player, message);
					npcSetConversationResponses (player, responses);
				}
				else
				{
					utils.removeScriptVar (player, "conversation.slicer_02_lok.branchId");
					
					npcEndConversationWithMessage (player, message);
				}
				
				return SCRIPT_CONTINUE;
			}
			
		}
		
		return SCRIPT_DEFAULT;
	}
	
	
	public int slicer_02_lok_handleBranch10(obj_id player, obj_id npc, string_id response) throws InterruptedException
	{
		
		if (response.equals("s_142"))
		{
			
			if (slicer_02_lok_condition__defaultCondition (player, npc))
			{
				
				string_id message = new string_id (c_stringFile, "s_143");
				int numberOfResponses = 0;
				
				boolean hasResponse = false;
				
				boolean hasResponse0 = false;
				if (slicer_02_lok_condition__defaultCondition (player, npc))
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
					
					utils.setScriptVar (player, "conversation.slicer_02_lok.branchId", 11);
					
					npcSpeak (player, message);
					npcSetConversationResponses (player, responses);
				}
				else
				{
					utils.removeScriptVar (player, "conversation.slicer_02_lok.branchId");
					
					npcEndConversationWithMessage (player, message);
				}
				
				return SCRIPT_CONTINUE;
			}
			
		}
		
		return SCRIPT_DEFAULT;
	}
	
	
	public int slicer_02_lok_handleBranch11(obj_id player, obj_id npc, string_id response) throws InterruptedException
	{
		
		if (response.equals("s_144"))
		{
			
			if (slicer_02_lok_condition__defaultCondition (player, npc))
			{
				
				string_id message = new string_id (c_stringFile, "s_145");
				int numberOfResponses = 0;
				
				boolean hasResponse = false;
				
				boolean hasResponse0 = false;
				if (slicer_02_lok_condition__defaultCondition (player, npc))
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
						responses[responseIndex++] = new string_id (c_stringFile, "s_146");
					}
					
					utils.setScriptVar (player, "conversation.slicer_02_lok.branchId", 12);
					
					npcSpeak (player, message);
					npcSetConversationResponses (player, responses);
				}
				else
				{
					utils.removeScriptVar (player, "conversation.slicer_02_lok.branchId");
					
					npcEndConversationWithMessage (player, message);
				}
				
				return SCRIPT_CONTINUE;
			}
			
		}
		
		return SCRIPT_DEFAULT;
	}
	
	
	public int slicer_02_lok_handleBranch12(obj_id player, obj_id npc, string_id response) throws InterruptedException
	{
		
		if (response.equals("s_146"))
		{
			
			if (slicer_02_lok_condition__defaultCondition (player, npc))
			{
				
				string_id message = new string_id (c_stringFile, "s_147");
				int numberOfResponses = 0;
				
				boolean hasResponse = false;
				
				boolean hasResponse0 = false;
				if (slicer_02_lok_condition__defaultCondition (player, npc))
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
					
					utils.setScriptVar (player, "conversation.slicer_02_lok.branchId", 13);
					
					npcSpeak (player, message);
					npcSetConversationResponses (player, responses);
				}
				else
				{
					utils.removeScriptVar (player, "conversation.slicer_02_lok.branchId");
					
					npcEndConversationWithMessage (player, message);
				}
				
				return SCRIPT_CONTINUE;
			}
			
		}
		
		return SCRIPT_DEFAULT;
	}
	
	
	public int slicer_02_lok_handleBranch13(obj_id player, obj_id npc, string_id response) throws InterruptedException
	{
		
		if (response.equals("s_148"))
		{
			
			if (slicer_02_lok_condition__defaultCondition (player, npc))
			{
				slicer_02_lok_action_sendSignalLokDungeon (player, npc);
				
				string_id message = new string_id (c_stringFile, "s_149");
				utils.removeScriptVar (player, "conversation.slicer_02_lok.branchId");
				
				npcEndConversationWithMessage (player, message);
				
				return SCRIPT_CONTINUE;
			}
			
		}
		
		return SCRIPT_DEFAULT;
	}
	
	
	public int slicer_02_lok_handleBranch15(obj_id player, obj_id npc, string_id response) throws InterruptedException
	{
		
		if (response.equals("s_150"))
		{
			
			if (slicer_02_lok_condition__defaultCondition (player, npc))
			{
				
				string_id message = new string_id (c_stringFile, "s_151");
				int numberOfResponses = 0;
				
				boolean hasResponse = false;
				
				boolean hasResponse0 = false;
				if (slicer_02_lok_condition__defaultCondition (player, npc))
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
					
					utils.setScriptVar (player, "conversation.slicer_02_lok.branchId", 16);
					
					npcSpeak (player, message);
					npcSetConversationResponses (player, responses);
				}
				else
				{
					utils.removeScriptVar (player, "conversation.slicer_02_lok.branchId");
					
					npcEndConversationWithMessage (player, message);
				}
				
				return SCRIPT_CONTINUE;
			}
			
		}
		
		return SCRIPT_DEFAULT;
	}
	
	
	public int slicer_02_lok_handleBranch16(obj_id player, obj_id npc, string_id response) throws InterruptedException
	{
		
		if (response.equals("s_152"))
		{
			
			if (slicer_02_lok_condition__defaultCondition (player, npc))
			{
				
				string_id message = new string_id (c_stringFile, "s_155");
				int numberOfResponses = 0;
				
				boolean hasResponse = false;
				
				boolean hasResponse0 = false;
				if (slicer_02_lok_condition__defaultCondition (player, npc))
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
					
					utils.setScriptVar (player, "conversation.slicer_02_lok.branchId", 17);
					
					npcSpeak (player, message);
					npcSetConversationResponses (player, responses);
				}
				else
				{
					utils.removeScriptVar (player, "conversation.slicer_02_lok.branchId");
					
					npcEndConversationWithMessage (player, message);
				}
				
				return SCRIPT_CONTINUE;
			}
			
		}
		
		return SCRIPT_DEFAULT;
	}
	
	
	public int slicer_02_lok_handleBranch17(obj_id player, obj_id npc, string_id response) throws InterruptedException
	{
		
		if (response.equals("s_156"))
		{
			
			if (slicer_02_lok_condition__defaultCondition (player, npc))
			{
				
				string_id message = new string_id (c_stringFile, "s_157");
				int numberOfResponses = 0;
				
				boolean hasResponse = false;
				
				boolean hasResponse0 = false;
				if (slicer_02_lok_condition__defaultCondition (player, npc))
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
					
					utils.setScriptVar (player, "conversation.slicer_02_lok.branchId", 18);
					
					npcSpeak (player, message);
					npcSetConversationResponses (player, responses);
				}
				else
				{
					utils.removeScriptVar (player, "conversation.slicer_02_lok.branchId");
					
					npcEndConversationWithMessage (player, message);
				}
				
				return SCRIPT_CONTINUE;
			}
			
		}
		
		return SCRIPT_DEFAULT;
	}
	
	
	public int slicer_02_lok_handleBranch18(obj_id player, obj_id npc, string_id response) throws InterruptedException
	{
		
		if (response.equals("s_158"))
		{
			
			if (slicer_02_lok_condition__defaultCondition (player, npc))
			{
				slicer_02_lok_action_sendDathomirSignal (player, npc);
				
				string_id message = new string_id (c_stringFile, "s_159");
				utils.removeScriptVar (player, "conversation.slicer_02_lok.branchId");
				
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
			detachScript(self, "conversation.slicer_02_lok");
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
		detachScript (self, "conversation.slicer_02_lok");
		
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
		
		if (slicer_02_lok_condition_canceledQuest08 (player, npc))
		{
			slicer_02_lok_action_giveQuest08 (player, npc);
			
			string_id message = new string_id (c_stringFile, "s_36");
			chat.chat (npc, player, message);
			
			return SCRIPT_CONTINUE;
		}
		
		if (slicer_02_lok_condition_canceledQuest07 (player, npc))
		{
			slicer_02_lok_action_grantQuest07WithWaypoint (player, npc);
			
			string_id message = new string_id (c_stringFile, "s_35");
			chat.chat (npc, player, message);
			
			return SCRIPT_CONTINUE;
		}
		
		if (slicer_02_lok_condition_canceledQuest06 (player, npc))
		{
			
			string_id message = new string_id (c_stringFile, "s_34");
			chat.chat (npc, player, message);
			
			return SCRIPT_CONTINUE;
		}
		
		if (slicer_02_lok_condition_isGoingToDungeon (player, npc))
		{
			
			string_id message = new string_id (c_stringFile, "s_30");
			chat.chat (npc, player, message);
			
			return SCRIPT_CONTINUE;
		}
		
		if (slicer_02_lok_condition_isGoingToDathomir (player, npc))
		{
			doAnimationAction (npc, "laugh_cackle");
			
			doAnimationAction (player, "shake_head_no");
			
			string_id message = new string_id (c_stringFile, "s_31");
			chat.chat (npc, player, message);
			
			return SCRIPT_CONTINUE;
		}
		
		if (slicer_02_lok_condition_isReadyForDungeon (player, npc))
		{
			
			string_id message = new string_id (c_stringFile, "s_127");
			int numberOfResponses = 0;
			
			boolean hasResponse = false;
			
			boolean hasResponse0 = false;
			if (slicer_02_lok_condition__defaultCondition (player, npc))
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
				
				utils.setScriptVar (player, "conversation.slicer_02_lok.branchId", 6);
				
				npcStartConversation (player, npc, "slicer_02_lok", message, responses);
			}
			else
			{
				chat.chat (npc, player, message);
			}
			
			return SCRIPT_CONTINUE;
		}
		
		if (slicer_02_lok_condition_hasCompletedlokDungeon (player, npc))
		{
			
			string_id message = new string_id (c_stringFile, "s_129");
			int numberOfResponses = 0;
			
			boolean hasResponse = false;
			
			boolean hasResponse0 = false;
			if (slicer_02_lok_condition__defaultCondition (player, npc))
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
				
				utils.setScriptVar (player, "conversation.slicer_02_lok.branchId", 15);
				
				npcStartConversation (player, npc, "slicer_02_lok", message, responses);
			}
			else
			{
				chat.chat (npc, player, message);
			}
			
			return SCRIPT_CONTINUE;
		}
		
		if (slicer_02_lok_condition__defaultCondition (player, npc))
		{
			slicer_02_lok_action_giveQuest08 (player, npc);
			
			string_id message = new string_id (c_stringFile, "s_132");
			chat.chat (npc, player, message);
			
			return SCRIPT_CONTINUE;
		}
		
		chat.chat (npc, "Error: All conditions for OnStartNpcConversation were false.");
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int OnNpcConversationResponse(obj_id self, String conversationId, obj_id player, string_id response) throws InterruptedException
	{
		if (!conversationId.equals("slicer_02_lok"))
		{
			return SCRIPT_CONTINUE;
		}
		
		obj_id npc = self;
		
		int branchId = utils.getIntScriptVar (player, "conversation.slicer_02_lok.branchId");
		
		if (branchId == 6 && slicer_02_lok_handleBranch6 (player, npc, response) == SCRIPT_CONTINUE)
		{
			return SCRIPT_CONTINUE;
		}
		
		if (branchId == 7 && slicer_02_lok_handleBranch7 (player, npc, response) == SCRIPT_CONTINUE)
		{
			return SCRIPT_CONTINUE;
		}
		
		if (branchId == 8 && slicer_02_lok_handleBranch8 (player, npc, response) == SCRIPT_CONTINUE)
		{
			return SCRIPT_CONTINUE;
		}
		
		if (branchId == 9 && slicer_02_lok_handleBranch9 (player, npc, response) == SCRIPT_CONTINUE)
		{
			return SCRIPT_CONTINUE;
		}
		
		if (branchId == 10 && slicer_02_lok_handleBranch10 (player, npc, response) == SCRIPT_CONTINUE)
		{
			return SCRIPT_CONTINUE;
		}
		
		if (branchId == 11 && slicer_02_lok_handleBranch11 (player, npc, response) == SCRIPT_CONTINUE)
		{
			return SCRIPT_CONTINUE;
		}
		
		if (branchId == 12 && slicer_02_lok_handleBranch12 (player, npc, response) == SCRIPT_CONTINUE)
		{
			return SCRIPT_CONTINUE;
		}
		
		if (branchId == 13 && slicer_02_lok_handleBranch13 (player, npc, response) == SCRIPT_CONTINUE)
		{
			return SCRIPT_CONTINUE;
		}
		
		if (branchId == 15 && slicer_02_lok_handleBranch15 (player, npc, response) == SCRIPT_CONTINUE)
		{
			return SCRIPT_CONTINUE;
		}
		
		if (branchId == 16 && slicer_02_lok_handleBranch16 (player, npc, response) == SCRIPT_CONTINUE)
		{
			return SCRIPT_CONTINUE;
		}
		
		if (branchId == 17 && slicer_02_lok_handleBranch17 (player, npc, response) == SCRIPT_CONTINUE)
		{
			return SCRIPT_CONTINUE;
		}
		
		if (branchId == 18 && slicer_02_lok_handleBranch18 (player, npc, response) == SCRIPT_CONTINUE)
		{
			return SCRIPT_CONTINUE;
		}
		
		chat.chat (npc, "Error: Fell through all branches and responses for OnNpcConversationResponse.");
		
		utils.removeScriptVar (player, "conversation.slicer_02_lok.branchId");
		
		return SCRIPT_CONTINUE;
	}
	
}
