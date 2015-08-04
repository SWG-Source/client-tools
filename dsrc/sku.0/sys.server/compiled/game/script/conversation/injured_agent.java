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


public class injured_agent extends script.base_script
{
	public injured_agent()
	{
	}
	String c_stringFile = "conversation/injured_agent";
	
	
	public boolean injured_agent_condition__defaultCondition(obj_id player, obj_id npc) throws InterruptedException
	{
		return true;
	}
	
	
	public boolean injured_agent_condition_hasDelivery(obj_id player, obj_id npc) throws InterruptedException
	{
		return groundquests.isTaskActive(player, "u16_nym_themepark_save_agent", "findAgent") || (groundquests.hasCompletedQuest(player, "u16_nym_themepark_save_agent") && !groundquests.isQuestActiveOrComplete(player, "u16_nym_themepark_safe"));
	}
	
	
	public void injured_agent_action_clearDelivery(obj_id player, obj_id npc) throws InterruptedException
	{
		groundquests.sendSignal(player, "hasCompletedAgentTask");
	}
	
	
	public void injured_agent_action_giveReturnQuest(obj_id player, obj_id npc) throws InterruptedException
	{
		groundquests.grantQuest(player, "u16_nym_themepark_safe");
	}
	
	
	public int injured_agent_handleBranch1(obj_id player, obj_id npc, string_id response) throws InterruptedException
	{
		
		if (response.equals("s_6"))
		{
			
			if (injured_agent_condition__defaultCondition (player, npc))
			{
				injured_agent_action_giveReturnQuest (player, npc);
				
				string_id message = new string_id (c_stringFile, "s_8");
				utils.removeScriptVar (player, "conversation.injured_agent.branchId");
				
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
			detachScript(self, "conversation.injured_agent");
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
		detachScript (self, "conversation.injured_agent");
		
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
		
		if (injured_agent_condition_hasDelivery (player, npc))
		{
			injured_agent_action_clearDelivery (player, npc);
			
			string_id message = new string_id (c_stringFile, "s_4");
			int numberOfResponses = 0;
			
			boolean hasResponse = false;
			
			boolean hasResponse0 = false;
			if (injured_agent_condition__defaultCondition (player, npc))
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
					responses[responseIndex++] = new string_id (c_stringFile, "s_6");
				}
				
				utils.setScriptVar (player, "conversation.injured_agent.branchId", 1);
				
				npcStartConversation (player, npc, "injured_agent", message, responses);
			}
			else
			{
				chat.chat (npc, player, message);
			}
			
			return SCRIPT_CONTINUE;
		}
		
		if (injured_agent_condition__defaultCondition (player, npc))
		{
			
			string_id message = new string_id (c_stringFile, "s_10");
			chat.chat (npc, player, message);
			
			return SCRIPT_CONTINUE;
		}
		
		chat.chat (npc, "Error: All conditions for OnStartNpcConversation were false.");
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int OnNpcConversationResponse(obj_id self, String conversationId, obj_id player, string_id response) throws InterruptedException
	{
		if (!conversationId.equals("injured_agent"))
		{
			return SCRIPT_CONTINUE;
		}
		
		obj_id npc = self;
		
		int branchId = utils.getIntScriptVar (player, "conversation.injured_agent.branchId");
		
		if (branchId == 1 && injured_agent_handleBranch1 (player, npc, response) == SCRIPT_CONTINUE)
		{
			return SCRIPT_CONTINUE;
		}
		
		chat.chat (npc, "Error: Fell through all branches and responses for OnNpcConversationResponse.");
		
		utils.removeScriptVar (player, "conversation.injured_agent.branchId");
		
		return SCRIPT_CONTINUE;
	}
	
}
