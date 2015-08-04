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


public class detainment_npc_rescue extends script.base_script
{
	public detainment_npc_rescue()
	{
	}
	String c_stringFile = "conversation/detainment_npc_rescue";
	
	
	public boolean detainment_npc_rescue_condition__defaultCondition(obj_id player, obj_id npc) throws InterruptedException
	{
		return true;
	}
	
	
	public boolean detainment_npc_rescue_condition_hasTheMisionActiveAndIsMyRescuer(obj_id player, obj_id npc) throws InterruptedException
	{
		faceTo(npc, player);
		if (!hasObjVar(npc, "rescuer") || !hasObjVar(npc, "rescuerQuest"))
		{
			return false;
		}
		
		obj_id rescuer = getObjIdObjVar(npc, "rescuer");
		if (!isValidId(rescuer) || !exists(rescuer))
		{
			return false;
		}
		
		if (rescuer != player)
		{
			return false;
		}
		
		String activeQuest = getStringObjVar(npc, "rescuerQuest");
		if (activeQuest == null || activeQuest.length() <= 0)
		{
			return false;
		}
		
		if (!groundquests.isQuestActive(player, activeQuest))
		{
			return false;
		}
		
		return true;
	}
	
	
	public boolean detainment_npc_rescue_condition_isNotMyRescuer(obj_id player, obj_id npc) throws InterruptedException
	{
		faceTo(npc, player);
		if (!hasObjVar(npc, "rescuer") || !hasObjVar(npc, "rescuerQuest"))
		{
			return true;
		}
		
		obj_id rescuer = getObjIdObjVar(npc, "rescuer");
		if (!isValidId(rescuer) || !exists(rescuer))
		{
			return true;
		}
		
		if (rescuer != player)
		{
			return true;
		}
		
		return false;
	}
	
	
	public void detainment_npc_rescue_action_changeIntoDisguiseIncrCollection(obj_id player, obj_id npc) throws InterruptedException
	{
		String disguise = getStringObjVar(npc, "disguise");
		if (disguise == null || disguise.length() <= 0)
		{
			LOG("empire_day_trigger", "The disguise objvar is missing");
			destroyObject(npc);
			return;
		}
		if (disguise.startsWith("reb_"))
		{
			LOG("empire_day_trigger", "CONVERSATION: imp disguise found, reb collection");
			if (hasCompletedCollectionSlotPrereq(player, holiday.IMPERIAL_RESCUE_COUNTER_SLOT))
			{
				modifyCollectionSlotValue(player, holiday.IMPERIAL_RESCUE_COUNTER_SLOT, 1);
			}
		}
		else
		{
			LOG("empire_day_trigger", "CONVERSATION: imp disguise found, reb collection");
			if (hasCompletedCollectionSlotPrereq(player, holiday.REBEL_RESCUE_COUNTER_SLOT))
			{
				modifyCollectionSlotValue(player, holiday.REBEL_RESCUE_COUNTER_SLOT, 1);
			}
		}
		groundquests.sendSignal(player, "hasReceivedDisguise");
		
		location mobLoc = getLocation(npc);
		if (mobLoc == null)
		{
			LOG("empire_day_trigger", "The location of the mob is missing");
			destroyObject(npc);
			return;
		}
		
		obj_id rescuer = getObjIdObjVar(npc, "rescuer");
		if (!isValidId(rescuer) || !exists(rescuer))
		{
			LOG("empire_day_trigger", "The rescuer objvar is missing");
			destroyObject(npc);
			return;
		}
		obj_id mob = create.object(disguise, mobLoc, 100);
		if (!isValidId(mob))
		{
			LOG("empire_day_trigger", "The Mob was not properly created");
		}
		setObjVar(mob, "rescuer", rescuer);
		destroyObject(npc);
	}
	
	
	public String detainment_npc_rescue_tokenTO_timeLeftRecruitment(obj_id player, obj_id npc) throws InterruptedException
	{
		String returnString = holiday.getTimeRemainingBeforeLockoutRemoved(player, holiday.EMPIRE_DAY_RECRUITMENT_TIMESTAMP);
		if (returnString == null || returnString.equals(""))
		{
			return "might have to contact Customer Service. I can't seem to get any time data off your character.";
		}
		return returnString;
	}
	
	
	public String detainment_npc_rescue_tokenTO_timeLeftPropaganda(obj_id player, obj_id npc) throws InterruptedException
	{
		String returnString = holiday.getTimeRemainingBeforeLockoutRemoved(player, holiday.EMPIRE_DAY_PROPAGANDA_TIMESTAMP);
		if (returnString == null || returnString.equals(""))
		{
			return "might have to contact Customer Service. I can't seem to get any time data off your character.";
		}
		return returnString;
	}
	
	
	public int detainment_npc_rescue_tokenDI_notUsed(obj_id player, obj_id npc) throws InterruptedException
	{
		return 0;
	}
	
	
	public int detainment_npc_rescue_handleBranch2(obj_id player, obj_id npc, string_id response) throws InterruptedException
	{
		
		if (response.equals("s_79"))
		{
			
			if (detainment_npc_rescue_condition__defaultCondition (player, npc))
			{
				
				string_id message = new string_id (c_stringFile, "s_80");
				int numberOfResponses = 0;
				
				boolean hasResponse = false;
				
				boolean hasResponse0 = false;
				if (detainment_npc_rescue_condition__defaultCondition (player, npc))
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
					
					utils.setScriptVar (player, "conversation.detainment_npc_rescue.branchId", 3);
					
					npcSpeak (player, message);
					npcSetConversationResponses (player, responses);
				}
				else
				{
					utils.removeScriptVar (player, "conversation.detainment_npc_rescue.branchId");
					
					npcEndConversationWithMessage (player, message);
				}
				
				return SCRIPT_CONTINUE;
			}
			
		}
		
		return SCRIPT_DEFAULT;
	}
	
	
	public int detainment_npc_rescue_handleBranch3(obj_id player, obj_id npc, string_id response) throws InterruptedException
	{
		
		if (response.equals("s_8"))
		{
			
			if (detainment_npc_rescue_condition__defaultCondition (player, npc))
			{
				detainment_npc_rescue_action_changeIntoDisguiseIncrCollection (player, npc);
				
				string_id message = new string_id (c_stringFile, "s_10");
				utils.removeScriptVar (player, "conversation.detainment_npc_rescue.branchId");
				
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
			detachScript(self, "conversation.detainment_npc_rescue");
		}
		
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
	
	
	public int OnIncapacitated(obj_id self, obj_id killer) throws InterruptedException
	{
		clearCondition (self, CONDITION_CONVERSABLE);
		detachScript (self, "conversation.detainment_npc_rescue");
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int OnDetach(obj_id self) throws InterruptedException
	{
		clearCondition (self, CONDITION_CONVERSABLE);
		
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
		
		if (detainment_npc_rescue_condition_isNotMyRescuer (player, npc))
		{
			
			string_id message = new string_id (c_stringFile, "s_81");
			chat.chat (npc, player, message);
			
			return SCRIPT_CONTINUE;
		}
		
		if (detainment_npc_rescue_condition_hasTheMisionActiveAndIsMyRescuer (player, npc))
		{
			
			string_id message = new string_id (c_stringFile, "s_78");
			int numberOfResponses = 0;
			
			boolean hasResponse = false;
			
			boolean hasResponse0 = false;
			if (detainment_npc_rescue_condition__defaultCondition (player, npc))
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
					responses[responseIndex++] = new string_id (c_stringFile, "s_79");
				}
				
				utils.setScriptVar (player, "conversation.detainment_npc_rescue.branchId", 2);
				
				prose_package pp = new prose_package ();
				pp.stringId = message;
				pp.actor.set (player);
				pp.target.set (npc);
				
				npcStartConversation (player, npc, "detainment_npc_rescue", null, pp, responses);
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
		
		chat.chat (npc, "Error: All conditions for OnStartNpcConversation were false.");
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int OnNpcConversationResponse(obj_id self, String conversationId, obj_id player, string_id response) throws InterruptedException
	{
		if (!conversationId.equals("detainment_npc_rescue"))
		{
			return SCRIPT_CONTINUE;
		}
		
		obj_id npc = self;
		
		int branchId = utils.getIntScriptVar (player, "conversation.detainment_npc_rescue.branchId");
		
		if (branchId == 2 && detainment_npc_rescue_handleBranch2 (player, npc, response) == SCRIPT_CONTINUE)
		{
			return SCRIPT_CONTINUE;
		}
		
		if (branchId == 3 && detainment_npc_rescue_handleBranch3 (player, npc, response) == SCRIPT_CONTINUE)
		{
			return SCRIPT_CONTINUE;
		}
		
		chat.chat (npc, "Error: Fell through all branches and responses for OnNpcConversationResponse.");
		
		utils.removeScriptVar (player, "conversation.detainment_npc_rescue.branchId");
		
		return SCRIPT_CONTINUE;
	}
	
}
