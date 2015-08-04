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
import script.library.smuggler;
import script.library.utils;


public class junk_dealer_jawa extends script.base_script
{
	public junk_dealer_jawa()
	{
	}
	String c_stringFile = "conversation/junk_dealer_jawa";
	
	
	public boolean junk_dealer_jawa_condition__defaultCondition(obj_id player, obj_id npc) throws InterruptedException
	{
		return true;
	}
	
	
	public boolean junk_dealer_jawa_condition_hasNoInv(obj_id player, obj_id npc) throws InterruptedException
	{
		faceTo(npc, player);
		boolean hasNoInvRoom = false;
		obj_id playerInv = utils.getInventoryContainer(player);
		if (isIdValid(playerInv))
		{
			int free_space = getVolumeFree(playerInv);
			if (free_space < 1)
			{
				hasNoInvRoom = true;
			}
		}
		return hasNoInvRoom;
	}
	
	
	public boolean junk_dealer_jawa_condition_hasBuyBackItems(obj_id player, obj_id npc) throws InterruptedException
	{
		faceTo(npc, player);
		obj_id[] listOfBuyBacks = smuggler.getBuyBackItemsInContainer(player);
		return listOfBuyBacks != null && listOfBuyBacks.length > 0;
	}
	
	
	public boolean junk_dealer_jawa_condition_checkBuyBackContainer(obj_id player, obj_id npc) throws InterruptedException
	{
		faceTo(npc, player);
		if (!smuggler.hasBuyBackContainer(player))
		{
			junk_dealer_jawa_action_createBuyBackContainer(player, npc);
		}
		return true;
	}
	
	
	public void junk_dealer_jawa_action_start_dealing(obj_id player, obj_id npc) throws InterruptedException
	{
		dictionary params = new dictionary();
		params.put("player", player);
		
		messageTo(npc, "startDealing", params, 0.0f, false);
	}
	
	
	public void junk_dealer_jawa_action_face_to(obj_id player, obj_id npc) throws InterruptedException
	{
		faceTo(npc, player);
	}
	
	
	public void junk_dealer_jawa_action_startBuyBack(obj_id player, obj_id npc) throws InterruptedException
	{
		dictionary params = new dictionary();
		params.put("player", player);
		
		messageTo(npc, "startBuyBack", params, 0.0f, false);
	}
	
	
	public void junk_dealer_jawa_action_createBuyBackContainer(obj_id player, obj_id npc) throws InterruptedException
	{
		if (!isValidId(smuggler.createBuyBackControlDeviceOnPlayer(player)))
		{
			CustomerServiceLog("Junk_Dealer: ", "junk_dealer_generic conversation - Player (OID: "+ player + ") did not get his buy back container set up properly.");
		}
	}
	
	
	public void junk_dealer_jawa_action_startFlagNoSale(obj_id player, obj_id npc) throws InterruptedException
	{
		dictionary params = new dictionary();
		params.put("player", player);
		
		messageTo(npc, "startFlaggingItemsNoSale", params, 0.0f, false);
	}
	
	
	public int junk_dealer_jawa_handleBranch1(obj_id player, obj_id npc, string_id response) throws InterruptedException
	{
		
		if (response.equals("s_43"))
		{
			
			if (junk_dealer_jawa_condition_hasNoInv (player, npc))
			{
				
				string_id message = new string_id (c_stringFile, "s_47");
				int numberOfResponses = 0;
				
				boolean hasResponse = false;
				
				boolean hasResponse0 = false;
				if (junk_dealer_jawa_condition__defaultCondition (player, npc))
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
						responses[responseIndex++] = new string_id (c_stringFile, "s_53");
					}
					
					utils.setScriptVar (player, "conversation.junk_dealer_jawa.branchId", 2);
					
					npcSpeak (player, message);
					npcSetConversationResponses (player, responses);
				}
				else
				{
					utils.removeScriptVar (player, "conversation.junk_dealer_jawa.branchId");
					
					npcEndConversationWithMessage (player, message);
				}
				
				return SCRIPT_CONTINUE;
			}
			
			if (junk_dealer_jawa_condition__defaultCondition (player, npc))
			{
				junk_dealer_jawa_action_startBuyBack (player, npc);
				
				string_id message = new string_id (c_stringFile, "s_44");
				utils.removeScriptVar (player, "conversation.junk_dealer_jawa.branchId");
				
				npcEndConversationWithMessage (player, message);
				
				return SCRIPT_CONTINUE;
			}
			
		}
		
		if (response.equals("s_54fab04f"))
		{
			
			if (junk_dealer_jawa_condition__defaultCondition (player, npc))
			{
				junk_dealer_jawa_action_start_dealing (player, npc);
				
				string_id message = new string_id (c_stringFile, "s_84a67771");
				utils.removeScriptVar (player, "conversation.junk_dealer_jawa.branchId");
				
				npcEndConversationWithMessage (player, message);
				
				return SCRIPT_CONTINUE;
			}
			
		}
		
		if (response.equals("s_48"))
		{
			
			if (junk_dealer_jawa_condition__defaultCondition (player, npc))
			{
				junk_dealer_jawa_action_startFlagNoSale (player, npc);
				
				string_id message = new string_id (c_stringFile, "s_50");
				utils.removeScriptVar (player, "conversation.junk_dealer_jawa.branchId");
				
				npcEndConversationWithMessage (player, message);
				
				return SCRIPT_CONTINUE;
			}
			
		}
		
		return SCRIPT_DEFAULT;
	}
	
	
	public int junk_dealer_jawa_handleBranch2(obj_id player, obj_id npc, string_id response) throws InterruptedException
	{
		
		if (response.equals("s_53"))
		{
			
			if (junk_dealer_jawa_condition__defaultCondition (player, npc))
			{
				
				string_id message = new string_id (c_stringFile, "s_54");
				utils.removeScriptVar (player, "conversation.junk_dealer_jawa.branchId");
				
				npcEndConversationWithMessage (player, message);
				
				return SCRIPT_CONTINUE;
			}
			
		}
		
		return SCRIPT_DEFAULT;
	}
	
	
	public int OnInitialize(obj_id self) throws InterruptedException
	{
		if ((!isMob (self)) || (isPlayer (self)))
		{
			detachScript(self, "conversation.junk_dealer_jawa");
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
		detachScript (self, "conversation.junk_dealer_jawa");
		
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
		
		if (junk_dealer_jawa_condition_checkBuyBackContainer (player, npc))
		{
			junk_dealer_jawa_action_face_to (player, npc);
			
			string_id message = new string_id (c_stringFile, "s_bef51e38");
			int numberOfResponses = 0;
			
			boolean hasResponse = false;
			
			boolean hasResponse0 = false;
			if (junk_dealer_jawa_condition_hasBuyBackItems (player, npc))
			{
				++numberOfResponses;
				hasResponse = true;
				hasResponse0 = true;
			}
			
			boolean hasResponse1 = false;
			if (junk_dealer_jawa_condition__defaultCondition (player, npc))
			{
				++numberOfResponses;
				hasResponse = true;
				hasResponse1 = true;
			}
			
			boolean hasResponse2 = false;
			if (junk_dealer_jawa_condition__defaultCondition (player, npc))
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
					responses[responseIndex++] = new string_id (c_stringFile, "s_43");
				}
				
				if (hasResponse1)
				{
					responses[responseIndex++] = new string_id (c_stringFile, "s_54fab04f");
				}
				
				if (hasResponse2)
				{
					responses[responseIndex++] = new string_id (c_stringFile, "s_48");
				}
				
				utils.setScriptVar (player, "conversation.junk_dealer_jawa.branchId", 1);
				
				npcStartConversation (player, npc, "junk_dealer_jawa", message, responses);
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
		if (!conversationId.equals("junk_dealer_jawa"))
		{
			return SCRIPT_CONTINUE;
		}
		
		obj_id npc = self;
		
		int branchId = utils.getIntScriptVar (player, "conversation.junk_dealer_jawa.branchId");
		
		if (branchId == 1 && junk_dealer_jawa_handleBranch1 (player, npc, response) == SCRIPT_CONTINUE)
		{
			return SCRIPT_CONTINUE;
		}
		
		if (branchId == 2 && junk_dealer_jawa_handleBranch2 (player, npc, response) == SCRIPT_CONTINUE)
		{
			return SCRIPT_CONTINUE;
		}
		
		chat.chat (npc, "Error: Fell through all branches and responses for OnNpcConversationResponse.");
		
		utils.removeScriptVar (player, "conversation.junk_dealer_jawa.branchId");
		
		return SCRIPT_CONTINUE;
	}
	
}
