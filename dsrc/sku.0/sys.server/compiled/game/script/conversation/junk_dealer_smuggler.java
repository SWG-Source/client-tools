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
import script.library.group;
import script.library.quests;
import script.library.smuggler;
import script.library.utils;


public class junk_dealer_smuggler extends script.base_script
{
	public junk_dealer_smuggler()
	{
	}
	String c_stringFile = "conversation/junk_dealer_smuggler";
	
	
	public boolean junk_dealer_smuggler_condition__defaultCondition(obj_id player, obj_id npc) throws InterruptedException
	{
		return true;
	}
	
	
	public boolean junk_dealer_smuggler_condition_check_inv(obj_id player, obj_id npc) throws InterruptedException
	{
		faceTo(npc, player);
		return smuggler.checkInventory(player, npc);
	}
	
	
	public boolean junk_dealer_smuggler_condition_hasNoInv(obj_id player, obj_id npc) throws InterruptedException
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
	
	
	public boolean junk_dealer_smuggler_condition_check_master(obj_id player, obj_id npc) throws InterruptedException
	{
		faceTo(npc, player);
		obj_id master = utils.getObjIdScriptVar(npc, "smugglerMaster");
		
		if (master != player)
		{
			return false;
		}
		
		return true;
	}
	
	
	public boolean junk_dealer_smuggler_condition_isInParty(obj_id player, obj_id npc) throws InterruptedException
	{
		faceTo(npc, player);
		obj_id master = utils.getObjIdScriptVar(npc, "smugglerMaster");
		
		if (player == master)
		{
			return true;
		}
		
		if (group.isGrouped(master))
		{
			Vector party = group.getPCMembersInRange(master, 32f);
			
			if (party != null)
			{
				for (int i = 0; i < party.size(); i++)
				{
					testAbortScript();
					obj_id who = (obj_id)party.elementAt(i);
					
					if (player == who)
					{
						return true;
					}
				}
			}
		}
		
		return false;
	}
	
	
	public boolean junk_dealer_smuggler_condition_canGetPistolQuest(obj_id player, obj_id npc) throws InterruptedException
	{
		faceTo(npc, player);
		obj_id module = utils.getStaticItemInInventory(player, "item_reward_modify_pistol_01_01");
		
		if (!isIdValid(module) || getCount(module) <= 0)
		{
			return false;
		}
		
		if (groundquests.isQuestActive(player, "quest/smuggler_modules_for_pistol"))
		{
			return false;
		}
		
		return true;
	}
	
	
	public boolean junk_dealer_smuggler_condition_canGetPistolQuestComplete(obj_id player, obj_id npc) throws InterruptedException
	{
		faceTo(npc, player);
		obj_id module = utils.getStaticItemInInventory(player, "item_reward_modify_pistol_01_01");
		
		if (!isIdValid(module) || getCount(module) < 100)
		{
			return false;
		}
		
		if (!groundquests.isQuestActive(player, "quest/smuggler_modules_for_pistol"))
		{
			return false;
		}
		
		return true;
	}
	
	
	public boolean junk_dealer_smuggler_condition_hasBuyBackItems(obj_id player, obj_id npc) throws InterruptedException
	{
		obj_id[] listOfBuyBacks = smuggler.getBuyBackItemsInContainer(player);
		return listOfBuyBacks != null && listOfBuyBacks.length > 0;
	}
	
	
	public boolean junk_dealer_smuggler_condition_isInPartyWithBuyBack(obj_id player, obj_id npc) throws InterruptedException
	{
		return junk_dealer_smuggler_condition_isInParty(player, npc) && junk_dealer_smuggler_condition_hasBuyBackItems(player, npc);
	}
	
	
	public boolean junk_dealer_smuggler_condition_checkBuyBackContainer(obj_id player, obj_id npc) throws InterruptedException
	{
		faceTo(npc, player);
		if (!smuggler.hasBuyBackContainer(player))
		{
			junk_dealer_smuggler_action_createBuyBackContainer(player, npc);
		}
		return true;
	}
	
	
	public void junk_dealer_smuggler_action_start_dealing(obj_id player, obj_id npc) throws InterruptedException
	{
		dictionary params = new dictionary();
		params.put("player", player);
		
		messageTo(npc, "startDealing", params, 0.0f, false);
	}
	
	
	public void junk_dealer_smuggler_action_face_to(obj_id player, obj_id npc) throws InterruptedException
	{
		faceTo(npc, player);
	}
	
	
	public void junk_dealer_smuggler_action_grant_pistol_quest(obj_id player, obj_id npc) throws InterruptedException
	{
		groundquests.requestGrantQuest(player, "quest/smuggler_modules_for_pistol", true);
	}
	
	
	public void junk_dealer_smuggler_action_complete_pistol_quest(obj_id player, obj_id npc) throws InterruptedException
	{
		obj_id module = utils.getStaticItemInInventory(player, "item_reward_modify_pistol_01_01");
		
		if (!isIdValid(module) || getCount(module) < 100)
		{
			return;
		}
		
		int count = getCount(module);
		
		count -= 100;
		
		if (count <= 0)
		{
			destroyObject(module);
		}
		else
		{
			setCount(module, count);
		}
		
		groundquests.sendSignal(player, "smugglerModulesCollected");
	}
	
	
	public void junk_dealer_smuggler_action_startBuyBack(obj_id player, obj_id npc) throws InterruptedException
	{
		dictionary params = new dictionary();
		params.put("player", player);
		
		messageTo(npc, "startBuyBack", params, 0.0f, false);
	}
	
	
	public void junk_dealer_smuggler_action_createBuyBackContainer(obj_id player, obj_id npc) throws InterruptedException
	{
		if (!isValidId(smuggler.createBuyBackControlDeviceOnPlayer(player)))
		{
			CustomerServiceLog("Junk_Dealer: ", "junk_dealer_generic conversation - Player (OID: "+ player + ") did not get his buy back container set up properly.");
		}
	}
	
	
	public void junk_dealer_smuggler_action_dismiss(obj_id player, obj_id npc) throws InterruptedException
	{
		dictionary params = new dictionary();
		params.put("player", player);
		
		messageTo(npc, "dismissDealer", params, 1.0f, false);
	}
	
	
	public void junk_dealer_smuggler_action_startFlagNoSale(obj_id player, obj_id npc) throws InterruptedException
	{
		dictionary params = new dictionary();
		params.put("player", player);
		
		messageTo(npc, "startFlaggingItemsNoSale", params, 0.0f, false);
	}
	
	
	public int junk_dealer_smuggler_handleBranch1(obj_id player, obj_id npc, string_id response) throws InterruptedException
	{
		
		if (response.equals("s_17"))
		{
			
			if (junk_dealer_smuggler_condition_hasNoInv (player, npc))
			{
				
				string_id message = new string_id (c_stringFile, "s_18");
				utils.removeScriptVar (player, "conversation.junk_dealer_smuggler.branchId");
				
				npcEndConversationWithMessage (player, message);
				
				return SCRIPT_CONTINUE;
			}
			
			if (junk_dealer_smuggler_condition__defaultCondition (player, npc))
			{
				junk_dealer_smuggler_action_startBuyBack (player, npc);
				
				string_id message = new string_id (c_stringFile, "s_19");
				utils.removeScriptVar (player, "conversation.junk_dealer_smuggler.branchId");
				
				npcEndConversationWithMessage (player, message);
				
				return SCRIPT_CONTINUE;
			}
			
		}
		
		if (response.equals("s_54fab04f"))
		{
			
			if (junk_dealer_smuggler_condition__defaultCondition (player, npc))
			{
				junk_dealer_smuggler_action_start_dealing (player, npc);
				
				string_id message = new string_id (c_stringFile, "s_84a67771");
				utils.removeScriptVar (player, "conversation.junk_dealer_smuggler.branchId");
				
				npcEndConversationWithMessage (player, message);
				
				return SCRIPT_CONTINUE;
			}
			
		}
		
		if (response.equals("s_20"))
		{
			
			if (junk_dealer_smuggler_condition__defaultCondition (player, npc))
			{
				junk_dealer_smuggler_action_startFlagNoSale (player, npc);
				
				string_id message = new string_id (c_stringFile, "s_23");
				utils.removeScriptVar (player, "conversation.junk_dealer_smuggler.branchId");
				
				npcEndConversationWithMessage (player, message);
				
				return SCRIPT_CONTINUE;
			}
			
		}
		
		if (response.equals("s_cd7a3f41"))
		{
			
			if (junk_dealer_smuggler_condition__defaultCondition (player, npc))
			{
				
				string_id message = new string_id (c_stringFile, "s_4bd9d15e");
				utils.removeScriptVar (player, "conversation.junk_dealer_smuggler.branchId");
				
				npcEndConversationWithMessage (player, message);
				
				return SCRIPT_CONTINUE;
			}
			
		}
		
		if (response.equals("s_43"))
		{
			
			if (junk_dealer_smuggler_condition__defaultCondition (player, npc))
			{
				junk_dealer_smuggler_action_dismiss (player, npc);
				
				string_id message = new string_id (c_stringFile, "s_44");
				utils.removeScriptVar (player, "conversation.junk_dealer_smuggler.branchId");
				
				npcEndConversationWithMessage (player, message);
				
				return SCRIPT_CONTINUE;
			}
			
		}
		
		if (response.equals("s_15"))
		{
			
			if (junk_dealer_smuggler_condition__defaultCondition (player, npc))
			{
				junk_dealer_smuggler_action_grant_pistol_quest (player, npc);
				
				string_id message = new string_id (c_stringFile, "s_21");
				utils.removeScriptVar (player, "conversation.junk_dealer_smuggler.branchId");
				
				npcEndConversationWithMessage (player, message);
				
				return SCRIPT_CONTINUE;
			}
			
		}
		
		if (response.equals("s_24"))
		{
			
			if (junk_dealer_smuggler_condition__defaultCondition (player, npc))
			{
				junk_dealer_smuggler_action_complete_pistol_quest (player, npc);
				
				string_id message = new string_id (c_stringFile, "s_26");
				utils.removeScriptVar (player, "conversation.junk_dealer_smuggler.branchId");
				
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
			detachScript(self, "conversation.junk_dealer_smuggler");
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
		detachScript (self, "conversation.junk_dealer_smuggler");
		
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
		
		if (junk_dealer_smuggler_condition_checkBuyBackContainer (player, npc))
		{
			
			string_id message = new string_id (c_stringFile, "s_bef51e38");
			int numberOfResponses = 0;
			
			boolean hasResponse = false;
			
			boolean hasResponse0 = false;
			if (junk_dealer_smuggler_condition_isInPartyWithBuyBack (player, npc))
			{
				++numberOfResponses;
				hasResponse = true;
				hasResponse0 = true;
			}
			
			boolean hasResponse1 = false;
			if (junk_dealer_smuggler_condition_isInParty (player, npc))
			{
				++numberOfResponses;
				hasResponse = true;
				hasResponse1 = true;
			}
			
			boolean hasResponse2 = false;
			if (junk_dealer_smuggler_condition__defaultCondition (player, npc))
			{
				++numberOfResponses;
				hasResponse = true;
				hasResponse2 = true;
			}
			
			boolean hasResponse3 = false;
			if (junk_dealer_smuggler_condition__defaultCondition (player, npc))
			{
				++numberOfResponses;
				hasResponse = true;
				hasResponse3 = true;
			}
			
			boolean hasResponse4 = false;
			if (junk_dealer_smuggler_condition_check_master (player, npc))
			{
				++numberOfResponses;
				hasResponse = true;
				hasResponse4 = true;
			}
			
			boolean hasResponse5 = false;
			if (junk_dealer_smuggler_condition_canGetPistolQuest (player, npc))
			{
				++numberOfResponses;
				hasResponse = true;
				hasResponse5 = true;
			}
			
			boolean hasResponse6 = false;
			if (junk_dealer_smuggler_condition_canGetPistolQuestComplete (player, npc))
			{
				++numberOfResponses;
				hasResponse = true;
				hasResponse6 = true;
			}
			
			if (hasResponse)
			{
				int responseIndex = 0;
				string_id responses[] = new string_id[numberOfResponses];
				
				if (hasResponse0)
				{
					responses[responseIndex++] = new string_id (c_stringFile, "s_17");
				}
				
				if (hasResponse1)
				{
					responses[responseIndex++] = new string_id (c_stringFile, "s_54fab04f");
				}
				
				if (hasResponse2)
				{
					responses[responseIndex++] = new string_id (c_stringFile, "s_20");
				}
				
				if (hasResponse3)
				{
					responses[responseIndex++] = new string_id (c_stringFile, "s_cd7a3f41");
				}
				
				if (hasResponse4)
				{
					responses[responseIndex++] = new string_id (c_stringFile, "s_43");
				}
				
				if (hasResponse5)
				{
					responses[responseIndex++] = new string_id (c_stringFile, "s_15");
				}
				
				if (hasResponse6)
				{
					responses[responseIndex++] = new string_id (c_stringFile, "s_24");
				}
				
				utils.setScriptVar (player, "conversation.junk_dealer_smuggler.branchId", 1);
				
				npcStartConversation (player, npc, "junk_dealer_smuggler", message, responses);
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
		if (!conversationId.equals("junk_dealer_smuggler"))
		{
			return SCRIPT_CONTINUE;
		}
		
		obj_id npc = self;
		
		int branchId = utils.getIntScriptVar (player, "conversation.junk_dealer_smuggler.branchId");
		
		if (branchId == 1 && junk_dealer_smuggler_handleBranch1 (player, npc, response) == SCRIPT_CONTINUE)
		{
			return SCRIPT_CONTINUE;
		}
		
		chat.chat (npc, "Error: Fell through all branches and responses for OnNpcConversationResponse.");
		
		utils.removeScriptVar (player, "conversation.junk_dealer_smuggler.branchId");
		
		return SCRIPT_CONTINUE;
	}
	
}
