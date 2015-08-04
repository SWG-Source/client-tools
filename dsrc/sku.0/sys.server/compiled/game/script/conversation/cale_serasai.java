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


public class cale_serasai extends script.base_script
{
	public cale_serasai()
	{
	}
	String c_stringFile = "conversation/cale_serasai";
	
	
	public boolean cale_serasai_condition__defaultCondition(obj_id player, obj_id npc) throws InterruptedException
	{
		return true;
	}
	
	
	public boolean cale_serasai_condition_isAmbassador(obj_id player, obj_id npc) throws InterruptedException
	{
		return groundquests.isTaskActive(player, "u16_nym_themepark_negotiate_peace", "speakToAmbassador");
	}
	
	
	public boolean cale_serasai_condition_deliverMessage(obj_id player, obj_id npc) throws InterruptedException
	{
		return groundquests.isTaskActive(player, "u16_nym_themepark_negotiate_peace", "deliverMessage");
	}
	
	
	public boolean cale_serasai_condition_moveThroughKill(obj_id player, obj_id npc) throws InterruptedException
	{
		return groundquests.isTaskActive(player, "u16_nym_themepark_negotiate_peace", "killTheHenchmen");
	}
	
	
	public boolean cale_serasai_condition_speakLastTime(obj_id player, obj_id npc) throws InterruptedException
	{
		return groundquests.isTaskActive(player, "u16_nym_themepark_negotiate_peace", "speakToAmbassadorTwo") || (groundquests.hasCompletedQuest(player, "u16_nym_themepark_negotiate_peace") && !groundquests.isQuestActiveOrComplete(player, "u16_nym_themepark_rescue_sheeli"));
	}
	
	
	public boolean cale_serasai_condition_hasNotCompletedWave(obj_id player, obj_id npc) throws InterruptedException
	{
		return groundquests.isTaskActive(player, "u16_nym_themepark_negotiate_peace", "killTheHenchmen");
	}
	
	
	public boolean cale_serasai_condition_failedExtraction(obj_id player, obj_id npc) throws InterruptedException
	{
		return groundquests.isTaskActive(player, "u16_nym_themepark_rescue_sheeli_fail", "failedExtraction");
	}
	
	
	public void cale_serasai_action_signalMessageReceived(obj_id player, obj_id npc) throws InterruptedException
	{
		groundquests.sendSignal(player, "hasDeliveredMessage");
		
		LOG("nym_wave_event", "sending wave signal");
		
		dictionary dict = new dictionary();
		dict.put("player", player);
		messageTo(npc, "waveEventControllerNPCStart", dict, 2, false);
	}
	
	
	public void cale_serasai_action_giveRescueMission(obj_id player, obj_id npc) throws InterruptedException
	{
		
	}
	
	
	public void cale_serasai_action_signalAmbassador(obj_id player, obj_id npc) throws InterruptedException
	{
		groundquests.sendSignal(player, "hasSpokenToAmbassador");
	}
	
	
	public void cale_serasai_action_signalKill(obj_id player, obj_id npc) throws InterruptedException
	{
		groundquests.sendSignal(player, "hasKilledTheHenchmen");
	}
	
	
	public void cale_serasai_action_signalSpokenLastTime(obj_id player, obj_id npc) throws InterruptedException
	{
		groundquests.sendSignal(player, "hasSpokenToAmbassadorTwo");
	}
	
	
	public void cale_serasai_action_clearFail(obj_id player, obj_id npc) throws InterruptedException
	{
		groundquests.sendSignal(player, "hasSpokenCaleRemoveFail");
		cale_serasai_action_giveRescueMission(player,npc);
	}
	
	
	public int cale_serasai_handleBranch1(obj_id player, obj_id npc, string_id response) throws InterruptedException
	{
		
		if (response.equals("s_18"))
		{
			
			if (cale_serasai_condition__defaultCondition (player, npc))
			{
				doAnimationAction (npc, "udaman");
				
				cale_serasai_action_giveRescueMission (player, npc);
				
				string_id message = new string_id (c_stringFile, "s_19");
				utils.removeScriptVar (player, "conversation.cale_serasai.branchId");
				
				npcEndConversationWithMessage (player, message);
				
				return SCRIPT_CONTINUE;
			}
			
		}
		
		return SCRIPT_DEFAULT;
	}
	
	
	public int cale_serasai_handleBranch4(obj_id player, obj_id npc, string_id response) throws InterruptedException
	{
		
		if (response.equals("s_14"))
		{
			
			if (cale_serasai_condition__defaultCondition (player, npc))
			{
				
				string_id message = new string_id (c_stringFile, "s_20");
				int numberOfResponses = 0;
				
				boolean hasResponse = false;
				
				boolean hasResponse0 = false;
				if (cale_serasai_condition__defaultCondition (player, npc))
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
						responses[responseIndex++] = new string_id (c_stringFile, "s_35");
					}
					
					utils.setScriptVar (player, "conversation.cale_serasai.branchId", 5);
					
					npcSpeak (player, message);
					npcSetConversationResponses (player, responses);
				}
				else
				{
					utils.removeScriptVar (player, "conversation.cale_serasai.branchId");
					
					npcEndConversationWithMessage (player, message);
				}
				
				return SCRIPT_CONTINUE;
			}
			
		}
		
		return SCRIPT_DEFAULT;
	}
	
	
	public int cale_serasai_handleBranch5(obj_id player, obj_id npc, string_id response) throws InterruptedException
	{
		
		if (response.equals("s_35"))
		{
			
			if (cale_serasai_condition__defaultCondition (player, npc))
			{
				doAnimationAction (npc, "slit_throat");
				
				string_id message = new string_id (c_stringFile, "s_36");
				int numberOfResponses = 0;
				
				boolean hasResponse = false;
				
				boolean hasResponse0 = false;
				if (cale_serasai_condition__defaultCondition (player, npc))
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
					
					utils.setScriptVar (player, "conversation.cale_serasai.branchId", 6);
					
					npcSpeak (player, message);
					npcSetConversationResponses (player, responses);
				}
				else
				{
					utils.removeScriptVar (player, "conversation.cale_serasai.branchId");
					
					npcEndConversationWithMessage (player, message);
				}
				
				return SCRIPT_CONTINUE;
			}
			
		}
		
		return SCRIPT_DEFAULT;
	}
	
	
	public int cale_serasai_handleBranch6(obj_id player, obj_id npc, string_id response) throws InterruptedException
	{
		
		if (response.equals("s_37"))
		{
			
			if (cale_serasai_condition__defaultCondition (player, npc))
			{
				doAnimationAction (npc, "thumb_down");
				
				cale_serasai_action_signalMessageReceived (player, npc);
				
				string_id message = new string_id (c_stringFile, "s_38");
				utils.removeScriptVar (player, "conversation.cale_serasai.branchId");
				
				npcEndConversationWithMessage (player, message);
				
				return SCRIPT_CONTINUE;
			}
			
		}
		
		return SCRIPT_DEFAULT;
	}
	
	
	public int cale_serasai_handleBranch8(obj_id player, obj_id npc, string_id response) throws InterruptedException
	{
		
		if (response.equals("s_24"))
		{
			
			if (cale_serasai_condition__defaultCondition (player, npc))
			{
				doAnimationAction (npc, "nod_head_once");
				
				string_id message = new string_id (c_stringFile, "s_26");
				int numberOfResponses = 0;
				
				boolean hasResponse = false;
				
				boolean hasResponse0 = false;
				if (cale_serasai_condition__defaultCondition (player, npc))
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
					
					utils.setScriptVar (player, "conversation.cale_serasai.branchId", 9);
					
					npcSpeak (player, message);
					npcSetConversationResponses (player, responses);
				}
				else
				{
					utils.removeScriptVar (player, "conversation.cale_serasai.branchId");
					
					npcEndConversationWithMessage (player, message);
				}
				
				return SCRIPT_CONTINUE;
			}
			
		}
		
		return SCRIPT_DEFAULT;
	}
	
	
	public int cale_serasai_handleBranch9(obj_id player, obj_id npc, string_id response) throws InterruptedException
	{
		
		if (response.equals("s_29"))
		{
			
			if (cale_serasai_condition__defaultCondition (player, npc))
			{
				doAnimationAction (npc, "shake_head_no");
				
				string_id message = new string_id (c_stringFile, "s_30");
				int numberOfResponses = 0;
				
				boolean hasResponse = false;
				
				boolean hasResponse0 = false;
				if (cale_serasai_condition__defaultCondition (player, npc))
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
					
					utils.setScriptVar (player, "conversation.cale_serasai.branchId", 10);
					
					npcSpeak (player, message);
					npcSetConversationResponses (player, responses);
				}
				else
				{
					utils.removeScriptVar (player, "conversation.cale_serasai.branchId");
					
					npcEndConversationWithMessage (player, message);
				}
				
				return SCRIPT_CONTINUE;
			}
			
		}
		
		return SCRIPT_DEFAULT;
	}
	
	
	public int cale_serasai_handleBranch10(obj_id player, obj_id npc, string_id response) throws InterruptedException
	{
		
		if (response.equals("s_32"))
		{
			
			if (cale_serasai_condition__defaultCondition (player, npc))
			{
				
				string_id message = new string_id (c_stringFile, "s_34");
				int numberOfResponses = 0;
				
				boolean hasResponse = false;
				
				boolean hasResponse0 = false;
				if (cale_serasai_condition__defaultCondition (player, npc))
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
					
					utils.setScriptVar (player, "conversation.cale_serasai.branchId", 11);
					
					prose_package pp = new prose_package ();
					pp.stringId = message;
					pp.actor.set (player);
					pp.target.set (npc);
					
					npcSpeak (player, pp);
					npcSetConversationResponses (player, responses);
				}
				else
				{
					utils.removeScriptVar (player, "conversation.cale_serasai.branchId");
					
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
	
	
	public int cale_serasai_handleBranch11(obj_id player, obj_id npc, string_id response) throws InterruptedException
	{
		
		if (response.equals("s_40"))
		{
			
			if (cale_serasai_condition__defaultCondition (player, npc))
			{
				cale_serasai_action_signalAmbassador (player, npc);
				
				string_id message = new string_id (c_stringFile, "s_42");
				utils.removeScriptVar (player, "conversation.cale_serasai.branchId");
				
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
			detachScript(self, "conversation.cale_serasai");
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
		detachScript (self, "conversation.cale_serasai");
		
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
		
		if (cale_serasai_condition_speakLastTime (player, npc))
		{
			doAnimationAction (npc, "scared");
			
			cale_serasai_action_signalSpokenLastTime (player, npc);
			
			string_id message = new string_id (c_stringFile, "s_11");
			int numberOfResponses = 0;
			
			boolean hasResponse = false;
			
			boolean hasResponse0 = false;
			if (cale_serasai_condition__defaultCondition (player, npc))
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
				
				utils.setScriptVar (player, "conversation.cale_serasai.branchId", 1);
				
				npcStartConversation (player, npc, "cale_serasai", message, responses);
			}
			else
			{
				chat.chat (npc, player, message);
			}
			
			return SCRIPT_CONTINUE;
		}
		
		if (cale_serasai_condition_hasNotCompletedWave (player, npc))
		{
			cale_serasai_action_signalMessageReceived (player, npc);
			
			string_id message = new string_id (c_stringFile, "s_43");
			chat.chat (npc, player, message);
			
			return SCRIPT_CONTINUE;
		}
		
		if (cale_serasai_condition_deliverMessage (player, npc))
		{
			
			string_id message = new string_id (c_stringFile, "s_13");
			int numberOfResponses = 0;
			
			boolean hasResponse = false;
			
			boolean hasResponse0 = false;
			if (cale_serasai_condition__defaultCondition (player, npc))
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
					responses[responseIndex++] = new string_id (c_stringFile, "s_14");
				}
				
				utils.setScriptVar (player, "conversation.cale_serasai.branchId", 4);
				
				npcStartConversation (player, npc, "cale_serasai", message, responses);
			}
			else
			{
				chat.chat (npc, player, message);
			}
			
			return SCRIPT_CONTINUE;
		}
		
		if (cale_serasai_condition_isAmbassador (player, npc))
		{
			doAnimationAction (npc, "sigh_deeply");
			
			string_id message = new string_id (c_stringFile, "s_22");
			int numberOfResponses = 0;
			
			boolean hasResponse = false;
			
			boolean hasResponse0 = false;
			if (cale_serasai_condition__defaultCondition (player, npc))
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
					responses[responseIndex++] = new string_id (c_stringFile, "s_24");
				}
				
				utils.setScriptVar (player, "conversation.cale_serasai.branchId", 8);
				
				prose_package pp = new prose_package ();
				pp.stringId = message;
				pp.actor.set (player);
				pp.target.set (npc);
				
				npcStartConversation (player, npc, "cale_serasai", null, pp, responses);
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
		
		if (cale_serasai_condition__defaultCondition (player, npc))
		{
			
			string_id message = new string_id (c_stringFile, "s_46");
			chat.chat (npc, player, message);
			
			return SCRIPT_CONTINUE;
		}
		
		chat.chat (npc, "Error: All conditions for OnStartNpcConversation were false.");
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int OnNpcConversationResponse(obj_id self, String conversationId, obj_id player, string_id response) throws InterruptedException
	{
		if (!conversationId.equals("cale_serasai"))
		{
			return SCRIPT_CONTINUE;
		}
		
		obj_id npc = self;
		
		int branchId = utils.getIntScriptVar (player, "conversation.cale_serasai.branchId");
		
		if (branchId == 1 && cale_serasai_handleBranch1 (player, npc, response) == SCRIPT_CONTINUE)
		{
			return SCRIPT_CONTINUE;
		}
		
		if (branchId == 4 && cale_serasai_handleBranch4 (player, npc, response) == SCRIPT_CONTINUE)
		{
			return SCRIPT_CONTINUE;
		}
		
		if (branchId == 5 && cale_serasai_handleBranch5 (player, npc, response) == SCRIPT_CONTINUE)
		{
			return SCRIPT_CONTINUE;
		}
		
		if (branchId == 6 && cale_serasai_handleBranch6 (player, npc, response) == SCRIPT_CONTINUE)
		{
			return SCRIPT_CONTINUE;
		}
		
		if (branchId == 8 && cale_serasai_handleBranch8 (player, npc, response) == SCRIPT_CONTINUE)
		{
			return SCRIPT_CONTINUE;
		}
		
		if (branchId == 9 && cale_serasai_handleBranch9 (player, npc, response) == SCRIPT_CONTINUE)
		{
			return SCRIPT_CONTINUE;
		}
		
		if (branchId == 10 && cale_serasai_handleBranch10 (player, npc, response) == SCRIPT_CONTINUE)
		{
			return SCRIPT_CONTINUE;
		}
		
		if (branchId == 11 && cale_serasai_handleBranch11 (player, npc, response) == SCRIPT_CONTINUE)
		{
			return SCRIPT_CONTINUE;
		}
		
		chat.chat (npc, "Error: Fell through all branches and responses for OnNpcConversationResponse.");
		
		utils.removeScriptVar (player, "conversation.cale_serasai.branchId");
		
		return SCRIPT_CONTINUE;
	}
	
}
