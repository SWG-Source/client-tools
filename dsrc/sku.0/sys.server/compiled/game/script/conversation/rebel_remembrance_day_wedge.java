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


public class rebel_remembrance_day_wedge extends script.base_script
{
	public rebel_remembrance_day_wedge()
	{
	}
	String c_stringFile = "conversation/rebel_remembrance_day_wedge";
	
	
	public boolean rebel_remembrance_day_wedge_condition__defaultCondition(obj_id player, obj_id npc) throws InterruptedException
	{
		return true;
	}
	
	
	public boolean rebel_remembrance_day_wedge_condition_isImperialPlayer(obj_id player, obj_id npc) throws InterruptedException
	{
		rebel_remembrance_day_wedge_action_faceTo(npc, player);
		return (factions.isImperial(player));
	}
	
	
	public boolean rebel_remembrance_day_wedge_condition_isNeutralPlayer(obj_id player, obj_id npc) throws InterruptedException
	{
		return(!factions.isImperial(player) && !factions.isRebel(player));
	}
	
	
	public void rebel_remembrance_day_wedge_action_faceTo(obj_id player, obj_id npc) throws InterruptedException
	{
		rebel_remembrance_day_wedge_action_faceTo(npc, player);
	}
	
	
	public void rebel_remembrance_day_wedge_action_spawnTroopers(obj_id player, obj_id npc) throws InterruptedException
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
	
	
	public void rebel_remembrance_day_wedge_action_grantFirstMission(obj_id player, obj_id npc) throws InterruptedException
	{
		groundquests.grantQuest(player, holiday.EMPIREDAYQUEST_REB_COMBAT_01);
	}
	
	
	public int rebel_remembrance_day_wedge_handleBranch3(obj_id player, obj_id npc, string_id response) throws InterruptedException
	{
		
		if (response.equals("s_8"))
		{
			
			if (rebel_remembrance_day_wedge_condition__defaultCondition (player, npc))
			{
				doAnimationAction (npc, "nod_head_once");
				
				rebel_remembrance_day_wedge_action_faceTo (player, npc);
				
				string_id message = new string_id (c_stringFile, "s_9");
				int numberOfResponses = 0;
				
				boolean hasResponse = false;
				
				boolean hasResponse0 = false;
				if (rebel_remembrance_day_wedge_condition__defaultCondition (player, npc))
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
						responses[responseIndex++] = new string_id (c_stringFile, "s_10");
					}
					
					utils.setScriptVar (player, "conversation.rebel_remembrance_day_wedge.branchId", 4);
					
					npcSpeak (player, message);
					npcSetConversationResponses (player, responses);
				}
				else
				{
					utils.removeScriptVar (player, "conversation.rebel_remembrance_day_wedge.branchId");
					
					npcEndConversationWithMessage (player, message);
				}
				
				return SCRIPT_CONTINUE;
			}
			
		}
		
		return SCRIPT_DEFAULT;
	}
	
	
	public int rebel_remembrance_day_wedge_handleBranch4(obj_id player, obj_id npc, string_id response) throws InterruptedException
	{
		
		if (response.equals("s_10"))
		{
			
			if (rebel_remembrance_day_wedge_condition__defaultCondition (player, npc))
			{
				doAnimationAction (npc, "rub_chin_thoughtful");
				
				rebel_remembrance_day_wedge_action_faceTo (player, npc);
				
				string_id message = new string_id (c_stringFile, "s_12");
				int numberOfResponses = 0;
				
				boolean hasResponse = false;
				
				boolean hasResponse0 = false;
				if (rebel_remembrance_day_wedge_condition__defaultCondition (player, npc))
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
					
					utils.setScriptVar (player, "conversation.rebel_remembrance_day_wedge.branchId", 5);
					
					npcSpeak (player, message);
					npcSetConversationResponses (player, responses);
				}
				else
				{
					utils.removeScriptVar (player, "conversation.rebel_remembrance_day_wedge.branchId");
					
					npcEndConversationWithMessage (player, message);
				}
				
				return SCRIPT_CONTINUE;
			}
			
		}
		
		return SCRIPT_DEFAULT;
	}
	
	
	public int rebel_remembrance_day_wedge_handleBranch5(obj_id player, obj_id npc, string_id response) throws InterruptedException
	{
		
		if (response.equals("s_17"))
		{
			
			if (rebel_remembrance_day_wedge_condition__defaultCondition (player, npc))
			{
				rebel_remembrance_day_wedge_action_faceTo (player, npc);
				
				string_id message = new string_id (c_stringFile, "s_18");
				int numberOfResponses = 0;
				
				boolean hasResponse = false;
				
				boolean hasResponse0 = false;
				if (rebel_remembrance_day_wedge_condition__defaultCondition (player, npc))
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
					
					utils.setScriptVar (player, "conversation.rebel_remembrance_day_wedge.branchId", 6);
					
					npcSpeak (player, message);
					npcSetConversationResponses (player, responses);
				}
				else
				{
					utils.removeScriptVar (player, "conversation.rebel_remembrance_day_wedge.branchId");
					
					npcEndConversationWithMessage (player, message);
				}
				
				return SCRIPT_CONTINUE;
			}
			
		}
		
		return SCRIPT_DEFAULT;
	}
	
	
	public int rebel_remembrance_day_wedge_handleBranch6(obj_id player, obj_id npc, string_id response) throws InterruptedException
	{
		
		if (response.equals("s_20"))
		{
			
			if (rebel_remembrance_day_wedge_condition__defaultCondition (player, npc))
			{
				rebel_remembrance_day_wedge_action_faceTo (player, npc);
				
				string_id message = new string_id (c_stringFile, "s_22");
				int numberOfResponses = 0;
				
				boolean hasResponse = false;
				
				boolean hasResponse0 = false;
				if (rebel_remembrance_day_wedge_condition__defaultCondition (player, npc))
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
					
					utils.setScriptVar (player, "conversation.rebel_remembrance_day_wedge.branchId", 7);
					
					npcSpeak (player, message);
					npcSetConversationResponses (player, responses);
				}
				else
				{
					utils.removeScriptVar (player, "conversation.rebel_remembrance_day_wedge.branchId");
					
					npcEndConversationWithMessage (player, message);
				}
				
				return SCRIPT_CONTINUE;
			}
			
		}
		
		return SCRIPT_DEFAULT;
	}
	
	
	public int rebel_remembrance_day_wedge_handleBranch7(obj_id player, obj_id npc, string_id response) throws InterruptedException
	{
		
		if (response.equals("s_24"))
		{
			
			if (rebel_remembrance_day_wedge_condition__defaultCondition (player, npc))
			{
				rebel_remembrance_day_wedge_action_faceTo (player, npc);
				
				string_id message = new string_id (c_stringFile, "s_25");
				utils.removeScriptVar (player, "conversation.rebel_remembrance_day_wedge.branchId");
				
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
			detachScript(self, "conversation.rebel_remembrance_day_wedge");
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
		detachScript (self, "conversation.rebel_remembrance_day_wedge");
		
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
		
		if (rebel_remembrance_day_wedge_condition_isImperialPlayer (player, npc))
		{
			rebel_remembrance_day_wedge_action_spawnTroopers (player, npc);
			
			string_id message = new string_id (c_stringFile, "s_26");
			chat.chat (npc, player, message);
			
			return SCRIPT_CONTINUE;
		}
		
		if (rebel_remembrance_day_wedge_condition_isNeutralPlayer (player, npc))
		{
			rebel_remembrance_day_wedge_action_faceTo (player, npc);
			
			string_id message = new string_id (c_stringFile, "s_27");
			chat.chat (npc, player, message);
			
			return SCRIPT_CONTINUE;
		}
		
		if (rebel_remembrance_day_wedge_condition__defaultCondition (player, npc))
		{
			rebel_remembrance_day_wedge_action_faceTo (player, npc);
			
			string_id message = new string_id (c_stringFile, "s_7");
			int numberOfResponses = 0;
			
			boolean hasResponse = false;
			
			boolean hasResponse0 = false;
			if (rebel_remembrance_day_wedge_condition__defaultCondition (player, npc))
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
					responses[responseIndex++] = new string_id (c_stringFile, "s_8");
				}
				
				utils.setScriptVar (player, "conversation.rebel_remembrance_day_wedge.branchId", 3);
				
				npcStartConversation (player, npc, "rebel_remembrance_day_wedge", message, responses);
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
		if (!conversationId.equals("rebel_remembrance_day_wedge"))
		{
			return SCRIPT_CONTINUE;
		}
		
		obj_id npc = self;
		
		int branchId = utils.getIntScriptVar (player, "conversation.rebel_remembrance_day_wedge.branchId");
		
		if (branchId == 3 && rebel_remembrance_day_wedge_handleBranch3 (player, npc, response) == SCRIPT_CONTINUE)
		{
			return SCRIPT_CONTINUE;
		}
		
		if (branchId == 4 && rebel_remembrance_day_wedge_handleBranch4 (player, npc, response) == SCRIPT_CONTINUE)
		{
			return SCRIPT_CONTINUE;
		}
		
		if (branchId == 5 && rebel_remembrance_day_wedge_handleBranch5 (player, npc, response) == SCRIPT_CONTINUE)
		{
			return SCRIPT_CONTINUE;
		}
		
		if (branchId == 6 && rebel_remembrance_day_wedge_handleBranch6 (player, npc, response) == SCRIPT_CONTINUE)
		{
			return SCRIPT_CONTINUE;
		}
		
		if (branchId == 7 && rebel_remembrance_day_wedge_handleBranch7 (player, npc, response) == SCRIPT_CONTINUE)
		{
			return SCRIPT_CONTINUE;
		}
		
		chat.chat (npc, "Error: Fell through all branches and responses for OnNpcConversationResponse.");
		
		utils.removeScriptVar (player, "conversation.rebel_remembrance_day_wedge.branchId");
		
		return SCRIPT_CONTINUE;
	}
	
}
