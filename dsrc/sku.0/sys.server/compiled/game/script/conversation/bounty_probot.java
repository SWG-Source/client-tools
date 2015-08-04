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
import script.library.utils;


public class bounty_probot extends script.base_script
{
	public bounty_probot()
	{
	}
	String c_stringFile = "conversation/bounty_probot";
	
	
	public boolean bounty_probot_condition__defaultCondition(obj_id player, obj_id npc) throws InterruptedException
	{
		return true;
	}
	
	
	public boolean bounty_probot_condition_notMyHunter(obj_id player, obj_id npc) throws InterruptedException
	{
		obj_id objPlayer = getObjIdObjVar(npc, "objPlayer");
		obj_id objBountyMission = null;
		obj_id objMission = getObjIdObjVar(npc, "objMission");
		
		if (player != null)
		{
			obj_id[] objMissionArray = getMissionObjects(objPlayer);
			if (objMissionArray != null)
			{
				for (int intI = 0; intI<objMissionArray.length; intI++)
				{
					testAbortScript();
					String strType = getMissionType(objMissionArray[intI]);
					if (strType.equals("bounty"))
					{
						objBountyMission = objMissionArray[intI];
						
					}
					
				}
				
			}
		}
		
		if ((player != objPlayer || isIdNull(objBountyMission) || objMission != objBountyMission || hasObjVar(npc, "intDone") ))
		{
			return true;
		}
		else
		{
			return false;
		}
	}
	
	
	public boolean bounty_probot_condition_noBioSig(obj_id player, obj_id npc) throws InterruptedException
	{
		obj_id objMission = getObjIdObjVar(npc, "objMission");
		int intState = getIntObjVar(objMission, "intState");
		
		if (intState != 1)
		{
			string_id strSpam = new string_id("mission/mission_generic", "bounty_no_signature");
			sendSystemMessage(player, strSpam);
			return true;
		}
		else
		{
			return false;
		}
		
	}
	
	
	public boolean bounty_probot_condition_readyToTrack(obj_id player, obj_id npc) throws InterruptedException
	{
		obj_id objPlayer = getObjIdObjVar(npc, "objPlayer");
		
		if (player == objPlayer)
		{
			return true;
		}
		else
		{
			return false;
		}
	}
	
	
	public void bounty_probot_action_takeOff(obj_id player, obj_id npc) throws InterruptedException
	{
		setObjVar(npc, "intDone", 1);
		string_id strResponse = new string_id("mission/mission_generic", "probe_droid_takeoff");
		sendSystemMessage(player, strResponse);
		
		messageTo(npc, "take_Off", null, 2, true);
	}
	
	
	public int bounty_probot_handleBranch3(obj_id player, obj_id npc, string_id response) throws InterruptedException
	{
		
		if (response.equals("s_8"))
		{
			bounty_probot_action_takeOff (player, npc);
			
			if (bounty_probot_condition__defaultCondition (player, npc))
			{
				
				string_id message = new string_id (c_stringFile, "s_9");
				utils.removeScriptVar (player, "conversation.bounty_probot.branchId");
				
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
			detachScript(self, "conversation.bounty_probot");
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
		detachScript (self, "conversation.bounty_probot");
		
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
		
		if (bounty_probot_condition_notMyHunter (player, npc))
		{
			
			string_id message = new string_id (c_stringFile, "s_3");
			chat.chat (npc, player, message);
			
			return SCRIPT_CONTINUE;
		}
		
		if (bounty_probot_condition_noBioSig (player, npc))
		{
			
			string_id message = new string_id (c_stringFile, "s_4");
			chat.chat (npc, player, message);
			
			return SCRIPT_CONTINUE;
		}
		
		if (bounty_probot_condition_readyToTrack (player, npc))
		{
			
			string_id message = new string_id (c_stringFile, "s_5");
			int numberOfResponses = 0;
			
			boolean hasResponse = false;
			
			boolean hasResponse0 = false;
			if (bounty_probot_condition__defaultCondition (player, npc))
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
				
				utils.setScriptVar (player, "conversation.bounty_probot.branchId", 3);
				
				npcStartConversation (player, npc, "bounty_probot", message, responses);
			}
			else
			{
				chat.chat (npc, player, message);
			}
			
			return SCRIPT_CONTINUE;
		}
		
		if (bounty_probot_condition__defaultCondition (player, npc))
		{
			
			string_id message = new string_id (c_stringFile, "s_6");
			chat.chat (npc, player, message);
			
			return SCRIPT_CONTINUE;
		}
		
		chat.chat (npc, "Error: All conditions for OnStartNpcConversation were false.");
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int OnNpcConversationResponse(obj_id self, String conversationId, obj_id player, string_id response) throws InterruptedException
	{
		if (!conversationId.equals("bounty_probot"))
		{
			return SCRIPT_CONTINUE;
		}
		
		obj_id npc = self;
		
		int branchId = utils.getIntScriptVar (player, "conversation.bounty_probot.branchId");
		
		if (branchId == 3 && bounty_probot_handleBranch3 (player, npc, response) == SCRIPT_CONTINUE)
		{
			return SCRIPT_CONTINUE;
		}
		
		chat.chat (npc, "Error: Fell through all branches and responses for OnNpcConversationResponse.");
		
		utils.removeScriptVar (player, "conversation.bounty_probot.branchId");
		
		return SCRIPT_CONTINUE;
	}
	
}
