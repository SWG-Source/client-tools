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


public class junk_dealer_generic extends script.base_script
{
	public junk_dealer_generic()
	{
	}
	String c_stringFile = "conversation/junk_dealer_generic";
	
	
	public boolean junk_dealer_generic_condition__defaultCondition(obj_id player, obj_id npc) throws InterruptedException
	{
		return true;
	}
	
	
	public boolean junk_dealer_generic_condition_check_inv(obj_id player, obj_id npc) throws InterruptedException
	{
		return smuggler.checkInventory(player, npc);
	}
	
	
	public boolean junk_dealer_generic_condition_hasNoInv(obj_id player, obj_id npc) throws InterruptedException
	{
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
	
	
	public boolean junk_dealer_generic_condition_checkForKit(obj_id player, obj_id npc) throws InterruptedException
	{
		obj_id[] objContents = utils.getContents(player, true);
		
		if (objContents != null)
		{
			for (int intI = 0; intI<objContents.length; intI++)
			{
				testAbortScript();
				String strItemTemplate = getTemplateName(objContents[intI]);
				if (strItemTemplate.equals("object/tangible/loot/collectible/kits/orange_rug_kit.iff"))
				{
					return false;
				}
				else if (strItemTemplate.equals("object/tangible/loot/collectible/kits/blue_rug_kit.iff"))
				{
					return false;
				}
				else if (strItemTemplate.equals("object/tangible/loot/collectible/kits/gong_kit.iff"))
				{
					return false;
				}
				else if (strItemTemplate.equals("object/tangible/loot/collectible/kits/light_table_kit.iff"))
				{
					return false;
				}
				else if (strItemTemplate.equals("object/tangible/loot/collectible/kits/sculpture_kit.iff"))
				{
					return false;
				}
			}
		}
		return true;
	}
	
	
	public boolean junk_dealer_generic_condition_checkForKitToo(obj_id player, obj_id npc) throws InterruptedException
	{
		obj_id[] objContents = utils.getContents(player, true);
		
		if (objContents != null)
		{
			for (int intI = 0; intI<objContents.length; intI++)
			{
				testAbortScript();
				String strItemTemplate = getTemplateName(objContents[intI]);
				if (strItemTemplate.equals("object/tangible/loot/collectible/kits/orange_rug_kit.iff"))
				{
					return true;
				}
				else if (strItemTemplate.equals("object/tangible/loot/collectible/kits/blue_rug_kit.iff"))
				{
					return true;
				}
				else if (strItemTemplate.equals("object/tangible/loot/collectible/kits/gong_kit.iff"))
				{
					return true;
				}
				else if (strItemTemplate.equals("object/tangible/loot/collectible/kits/light_table_kit.iff"))
				{
					return true;
				}
				else if (strItemTemplate.equals("object/tangible/loot/collectible/kits/sculpture_kit.iff"))
				{
					return true;
				}
			}
		}
		return false;
	}
	
	
	public boolean junk_dealer_generic_condition_hasBuyBackItems(obj_id player, obj_id npc) throws InterruptedException
	{
		obj_id[] listOfBuyBacks = smuggler.getBuyBackItemsInContainer(player);
		return listOfBuyBacks != null && listOfBuyBacks.length > 0;
	}
	
	
	public boolean junk_dealer_generic_condition_checkBuyBackContainer(obj_id player, obj_id npc) throws InterruptedException
	{
		faceTo(npc, player);
		if (!smuggler.hasBuyBackContainer(player))
		{
			junk_dealer_generic_action_createBuyBackContainer(player, npc);
		}
		return true;
	}
	
	
	public void junk_dealer_generic_action_start_dealing(obj_id player, obj_id npc) throws InterruptedException
	{
		dictionary params = new dictionary();
		params.put("player", player);
		
		messageTo(npc, "startDealing", params, 0.0f, false);
	}
	
	
	public void junk_dealer_generic_action_face_to(obj_id player, obj_id npc) throws InterruptedException
	{
		faceTo(npc, player);
	}
	
	
	public void junk_dealer_generic_action_giveOrange(obj_id player, obj_id npc) throws InterruptedException
	{
		obj_id playerInv = getObjectInSlot( player, "inventory");
		if (isIdValid(playerInv))
		{
			obj_id item = createObject( "object/tangible/loot/collectible/kits/orange_rug_kit.iff", playerInv, "");
		}
		return;
	}
	
	
	public void junk_dealer_generic_action_giveBlue(obj_id player, obj_id npc) throws InterruptedException
	{
		obj_id playerInv = getObjectInSlot( player, "inventory");
		if (isIdValid(playerInv))
		{
			obj_id item = createObject( "object/tangible/loot/collectible/kits/blue_rug_kit.iff", playerInv, "");
		}
		return;
	}
	
	
	public void junk_dealer_generic_action_giveGong(obj_id player, obj_id npc) throws InterruptedException
	{
		obj_id playerInv = getObjectInSlot( player, "inventory");
		if (isIdValid(playerInv))
		{
			obj_id item = createObject( "object/tangible/loot/collectible/kits/gong_kit.iff", playerInv, "");
		}
		return;
	}
	
	
	public void junk_dealer_generic_action_giveSculpture(obj_id player, obj_id npc) throws InterruptedException
	{
		obj_id playerInv = getObjectInSlot( player, "inventory");
		if (isIdValid(playerInv))
		{
			obj_id item = createObject( "object/tangible/loot/collectible/kits/sculpture_kit.iff", playerInv, "");
		}
		return;
	}
	
	
	public void junk_dealer_generic_action_giveTable(obj_id player, obj_id npc) throws InterruptedException
	{
		obj_id playerInv = getObjectInSlot( player, "inventory");
		if (isIdValid(playerInv))
		{
			obj_id item = createObject( "object/tangible/loot/collectible/kits/light_table_kit.iff", playerInv, "");
		}
		return;
	}
	
	
	public void junk_dealer_generic_action_startBuyBack(obj_id player, obj_id npc) throws InterruptedException
	{
		dictionary params = new dictionary();
		params.put("player", player);
		
		messageTo(npc, "startBuyBack", params, 0.0f, false);
	}
	
	
	public void junk_dealer_generic_action_createBuyBackContainer(obj_id player, obj_id npc) throws InterruptedException
	{
		if (!isValidId(smuggler.createBuyBackControlDeviceOnPlayer(player)))
		{
			CustomerServiceLog("Junk_Dealer: ", "junk_dealer_generic conversation - Player (OID: "+ player + ") did not get his buy back container set up properly.");
		}
	}
	
	
	public void junk_dealer_generic_action_startFlagNoSale(obj_id player, obj_id npc) throws InterruptedException
	{
		dictionary params = new dictionary();
		params.put("player", player);
		
		messageTo(npc, "startFlaggingItemsNoSale", params, 0.0f, false);
	}
	
	
	public int junk_dealer_generic_handleBranch1(obj_id player, obj_id npc, string_id response) throws InterruptedException
	{
		
		if (response.equals("s_43"))
		{
			
			if (junk_dealer_generic_condition_hasNoInv (player, npc))
			{
				
				string_id message = new string_id (c_stringFile, "s_47");
				utils.removeScriptVar (player, "conversation.junk_dealer_generic.branchId");
				
				npcEndConversationWithMessage (player, message);
				
				return SCRIPT_CONTINUE;
			}
			
			if (junk_dealer_generic_condition__defaultCondition (player, npc))
			{
				junk_dealer_generic_action_startBuyBack (player, npc);
				
				string_id message = new string_id (c_stringFile, "s_44");
				utils.removeScriptVar (player, "conversation.junk_dealer_generic.branchId");
				
				npcEndConversationWithMessage (player, message);
				
				return SCRIPT_CONTINUE;
			}
			
		}
		
		if (response.equals("s_54fab04f"))
		{
			
			if (junk_dealer_generic_condition__defaultCondition (player, npc))
			{
				junk_dealer_generic_action_start_dealing (player, npc);
				
				string_id message = new string_id (c_stringFile, "s_84a67771");
				utils.removeScriptVar (player, "conversation.junk_dealer_generic.branchId");
				
				npcEndConversationWithMessage (player, message);
				
				return SCRIPT_CONTINUE;
			}
			
		}
		
		if (response.equals("s_48"))
		{
			
			if (junk_dealer_generic_condition__defaultCondition (player, npc))
			{
				junk_dealer_generic_action_startFlagNoSale (player, npc);
				
				string_id message = new string_id (c_stringFile, "s_50");
				utils.removeScriptVar (player, "conversation.junk_dealer_generic.branchId");
				
				npcEndConversationWithMessage (player, message);
				
				return SCRIPT_CONTINUE;
			}
			
		}
		
		if (response.equals("s_cd7a3f41"))
		{
			
			if (junk_dealer_generic_condition__defaultCondition (player, npc))
			{
				
				string_id message = new string_id (c_stringFile, "s_4bd9d15e");
				utils.removeScriptVar (player, "conversation.junk_dealer_generic.branchId");
				
				npcEndConversationWithMessage (player, message);
				
				return SCRIPT_CONTINUE;
			}
			
		}
		
		if (response.equals("s_3aa18b2d"))
		{
			
			if (junk_dealer_generic_condition__defaultCondition (player, npc))
			{
				
				string_id message = new string_id (c_stringFile, "s_d9e6b751");
				int numberOfResponses = 0;
				
				boolean hasResponse = false;
				
				boolean hasResponse0 = false;
				if (junk_dealer_generic_condition__defaultCondition (player, npc))
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
						responses[responseIndex++] = new string_id (c_stringFile, "s_6d53d062");
					}
					
					utils.setScriptVar (player, "conversation.junk_dealer_generic.branchId", 7);
					
					npcSpeak (player, message);
					npcSetConversationResponses (player, responses);
				}
				else
				{
					utils.removeScriptVar (player, "conversation.junk_dealer_generic.branchId");
					
					npcEndConversationWithMessage (player, message);
				}
				
				return SCRIPT_CONTINUE;
			}
			
		}
		
		return SCRIPT_DEFAULT;
	}
	
	
	public int junk_dealer_generic_handleBranch7(obj_id player, obj_id npc, string_id response) throws InterruptedException
	{
		
		if (response.equals("s_6d53d062"))
		{
			
			if (junk_dealer_generic_condition__defaultCondition (player, npc))
			{
				
				string_id message = new string_id (c_stringFile, "s_e29f48dc");
				int numberOfResponses = 0;
				
				boolean hasResponse = false;
				
				boolean hasResponse0 = false;
				if (junk_dealer_generic_condition__defaultCondition (player, npc))
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
						responses[responseIndex++] = new string_id (c_stringFile, "s_324b9b0f");
					}
					
					utils.setScriptVar (player, "conversation.junk_dealer_generic.branchId", 8);
					
					npcSpeak (player, message);
					npcSetConversationResponses (player, responses);
				}
				else
				{
					utils.removeScriptVar (player, "conversation.junk_dealer_generic.branchId");
					
					npcEndConversationWithMessage (player, message);
				}
				
				return SCRIPT_CONTINUE;
			}
			
		}
		
		return SCRIPT_DEFAULT;
	}
	
	
	public int junk_dealer_generic_handleBranch8(obj_id player, obj_id npc, string_id response) throws InterruptedException
	{
		
		if (response.equals("s_324b9b0f"))
		{
			
			if (junk_dealer_generic_condition__defaultCondition (player, npc))
			{
				
				string_id message = new string_id (c_stringFile, "s_12fe83a6");
				int numberOfResponses = 0;
				
				boolean hasResponse = false;
				
				boolean hasResponse0 = false;
				if (junk_dealer_generic_condition__defaultCondition (player, npc))
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
						responses[responseIndex++] = new string_id (c_stringFile, "s_e1a103e5");
					}
					
					utils.setScriptVar (player, "conversation.junk_dealer_generic.branchId", 9);
					
					npcSpeak (player, message);
					npcSetConversationResponses (player, responses);
				}
				else
				{
					utils.removeScriptVar (player, "conversation.junk_dealer_generic.branchId");
					
					npcEndConversationWithMessage (player, message);
				}
				
				return SCRIPT_CONTINUE;
			}
			
		}
		
		return SCRIPT_DEFAULT;
	}
	
	
	public int junk_dealer_generic_handleBranch9(obj_id player, obj_id npc, string_id response) throws InterruptedException
	{
		
		if (response.equals("s_e1a103e5"))
		{
			
			if (junk_dealer_generic_condition__defaultCondition (player, npc))
			{
				
				string_id message = new string_id (c_stringFile, "s_4d65752");
				int numberOfResponses = 0;
				
				boolean hasResponse = false;
				
				boolean hasResponse0 = false;
				if (junk_dealer_generic_condition__defaultCondition (player, npc))
				{
					++numberOfResponses;
					hasResponse = true;
					hasResponse0 = true;
				}
				
				boolean hasResponse1 = false;
				if (junk_dealer_generic_condition__defaultCondition (player, npc))
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
						responses[responseIndex++] = new string_id (c_stringFile, "s_d347bee3");
					}
					
					if (hasResponse1)
					{
						responses[responseIndex++] = new string_id (c_stringFile, "s_b60b73f8");
					}
					
					utils.setScriptVar (player, "conversation.junk_dealer_generic.branchId", 10);
					
					npcSpeak (player, message);
					npcSetConversationResponses (player, responses);
				}
				else
				{
					utils.removeScriptVar (player, "conversation.junk_dealer_generic.branchId");
					
					npcEndConversationWithMessage (player, message);
				}
				
				return SCRIPT_CONTINUE;
			}
			
		}
		
		return SCRIPT_DEFAULT;
	}
	
	
	public int junk_dealer_generic_handleBranch10(obj_id player, obj_id npc, string_id response) throws InterruptedException
	{
		
		if (response.equals("s_d347bee3"))
		{
			
			if (junk_dealer_generic_condition_checkForKit (player, npc))
			{
				
				string_id message = new string_id (c_stringFile, "s_3fc7eb45");
				int numberOfResponses = 0;
				
				boolean hasResponse = false;
				
				boolean hasResponse0 = false;
				if (junk_dealer_generic_condition__defaultCondition (player, npc))
				{
					++numberOfResponses;
					hasResponse = true;
					hasResponse0 = true;
				}
				
				boolean hasResponse1 = false;
				if (junk_dealer_generic_condition__defaultCondition (player, npc))
				{
					++numberOfResponses;
					hasResponse = true;
					hasResponse1 = true;
				}
				
				boolean hasResponse2 = false;
				if (junk_dealer_generic_condition__defaultCondition (player, npc))
				{
					++numberOfResponses;
					hasResponse = true;
					hasResponse2 = true;
				}
				
				boolean hasResponse3 = false;
				if (junk_dealer_generic_condition__defaultCondition (player, npc))
				{
					++numberOfResponses;
					hasResponse = true;
					hasResponse3 = true;
				}
				
				boolean hasResponse4 = false;
				if (junk_dealer_generic_condition__defaultCondition (player, npc))
				{
					++numberOfResponses;
					hasResponse = true;
					hasResponse4 = true;
				}
				
				if (hasResponse)
				{
					int responseIndex = 0;
					string_id responses[] = new string_id[numberOfResponses];
					
					if (hasResponse0)
					{
						responses[responseIndex++] = new string_id (c_stringFile, "s_ee977dee");
					}
					
					if (hasResponse1)
					{
						responses[responseIndex++] = new string_id (c_stringFile, "s_8f39769");
					}
					
					if (hasResponse2)
					{
						responses[responseIndex++] = new string_id (c_stringFile, "s_fe657cdd");
					}
					
					if (hasResponse3)
					{
						responses[responseIndex++] = new string_id (c_stringFile, "s_9ede4b84");
					}
					
					if (hasResponse4)
					{
						responses[responseIndex++] = new string_id (c_stringFile, "s_87c5851b");
					}
					
					utils.setScriptVar (player, "conversation.junk_dealer_generic.branchId", 11);
					
					npcSpeak (player, message);
					npcSetConversationResponses (player, responses);
				}
				else
				{
					utils.removeScriptVar (player, "conversation.junk_dealer_generic.branchId");
					
					npcEndConversationWithMessage (player, message);
				}
				
				return SCRIPT_CONTINUE;
			}
			
			if (junk_dealer_generic_condition_checkForKitToo (player, npc))
			{
				
				string_id message = new string_id (c_stringFile, "s_3df21ea0");
				utils.removeScriptVar (player, "conversation.junk_dealer_generic.branchId");
				
				npcEndConversationWithMessage (player, message);
				
				return SCRIPT_CONTINUE;
			}
			
		}
		
		if (response.equals("s_b60b73f8"))
		{
			
			if (junk_dealer_generic_condition__defaultCondition (player, npc))
			{
				
				string_id message = new string_id (c_stringFile, "s_3633b5a5");
				utils.removeScriptVar (player, "conversation.junk_dealer_generic.branchId");
				
				npcEndConversationWithMessage (player, message);
				
				return SCRIPT_CONTINUE;
			}
			
		}
		
		return SCRIPT_DEFAULT;
	}
	
	
	public int junk_dealer_generic_handleBranch11(obj_id player, obj_id npc, string_id response) throws InterruptedException
	{
		
		if (response.equals("s_ee977dee"))
		{
			
			if (!junk_dealer_generic_condition_hasNoInv (player, npc))
			{
				junk_dealer_generic_action_giveOrange (player, npc);
				
				string_id message = new string_id (c_stringFile, "s_14efaaa2");
				utils.removeScriptVar (player, "conversation.junk_dealer_generic.branchId");
				
				npcEndConversationWithMessage (player, message);
				
				return SCRIPT_CONTINUE;
			}
			
			if (junk_dealer_generic_condition_hasNoInv (player, npc))
			{
				
				string_id message = new string_id (c_stringFile, "s_5b10c0b9");
				utils.removeScriptVar (player, "conversation.junk_dealer_generic.branchId");
				
				npcEndConversationWithMessage (player, message);
				
				return SCRIPT_CONTINUE;
			}
			
		}
		
		if (response.equals("s_8f39769"))
		{
			
			if (!junk_dealer_generic_condition_hasNoInv (player, npc))
			{
				junk_dealer_generic_action_giveBlue (player, npc);
				
				string_id message = new string_id (c_stringFile, "s_27");
				utils.removeScriptVar (player, "conversation.junk_dealer_generic.branchId");
				
				npcEndConversationWithMessage (player, message);
				
				return SCRIPT_CONTINUE;
			}
			
			if (junk_dealer_generic_condition_hasNoInv (player, npc))
			{
				
				string_id message = new string_id (c_stringFile, "s_29");
				utils.removeScriptVar (player, "conversation.junk_dealer_generic.branchId");
				
				npcEndConversationWithMessage (player, message);
				
				return SCRIPT_CONTINUE;
			}
			
		}
		
		if (response.equals("s_fe657cdd"))
		{
			
			if (!junk_dealer_generic_condition_hasNoInv (player, npc))
			{
				junk_dealer_generic_action_giveGong (player, npc);
				
				string_id message = new string_id (c_stringFile, "s_32");
				utils.removeScriptVar (player, "conversation.junk_dealer_generic.branchId");
				
				npcEndConversationWithMessage (player, message);
				
				return SCRIPT_CONTINUE;
			}
			
			if (junk_dealer_generic_condition_hasNoInv (player, npc))
			{
				
				string_id message = new string_id (c_stringFile, "s_34");
				utils.removeScriptVar (player, "conversation.junk_dealer_generic.branchId");
				
				npcEndConversationWithMessage (player, message);
				
				return SCRIPT_CONTINUE;
			}
			
		}
		
		if (response.equals("s_9ede4b84"))
		{
			
			if (!junk_dealer_generic_condition_hasNoInv (player, npc))
			{
				junk_dealer_generic_action_giveTable (player, npc);
				
				string_id message = new string_id (c_stringFile, "s_37");
				utils.removeScriptVar (player, "conversation.junk_dealer_generic.branchId");
				
				npcEndConversationWithMessage (player, message);
				
				return SCRIPT_CONTINUE;
			}
			
			if (junk_dealer_generic_condition_hasNoInv (player, npc))
			{
				
				string_id message = new string_id (c_stringFile, "s_39");
				utils.removeScriptVar (player, "conversation.junk_dealer_generic.branchId");
				
				npcEndConversationWithMessage (player, message);
				
				return SCRIPT_CONTINUE;
			}
			
		}
		
		if (response.equals("s_87c5851b"))
		{
			
			if (!junk_dealer_generic_condition_hasNoInv (player, npc))
			{
				junk_dealer_generic_action_giveSculpture (player, npc);
				
				string_id message = new string_id (c_stringFile, "s_42");
				utils.removeScriptVar (player, "conversation.junk_dealer_generic.branchId");
				
				npcEndConversationWithMessage (player, message);
				
				return SCRIPT_CONTINUE;
			}
			
			if (junk_dealer_generic_condition_hasNoInv (player, npc))
			{
				
				string_id message = new string_id (c_stringFile, "s_49");
				utils.removeScriptVar (player, "conversation.junk_dealer_generic.branchId");
				
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
			detachScript(self, "conversation.junk_dealer_generic");
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
		detachScript (self, "conversation.junk_dealer_generic");
		
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
		
		if (junk_dealer_generic_condition_checkBuyBackContainer (player, npc))
		{
			junk_dealer_generic_action_face_to (player, npc);
			
			string_id message = new string_id (c_stringFile, "s_bef51e38");
			int numberOfResponses = 0;
			
			boolean hasResponse = false;
			
			boolean hasResponse0 = false;
			if (junk_dealer_generic_condition_hasBuyBackItems (player, npc))
			{
				++numberOfResponses;
				hasResponse = true;
				hasResponse0 = true;
			}
			
			boolean hasResponse1 = false;
			if (junk_dealer_generic_condition__defaultCondition (player, npc))
			{
				++numberOfResponses;
				hasResponse = true;
				hasResponse1 = true;
			}
			
			boolean hasResponse2 = false;
			if (junk_dealer_generic_condition__defaultCondition (player, npc))
			{
				++numberOfResponses;
				hasResponse = true;
				hasResponse2 = true;
			}
			
			boolean hasResponse3 = false;
			if (junk_dealer_generic_condition__defaultCondition (player, npc))
			{
				++numberOfResponses;
				hasResponse = true;
				hasResponse3 = true;
			}
			
			boolean hasResponse4 = false;
			if (junk_dealer_generic_condition_check_inv (player, npc))
			{
				++numberOfResponses;
				hasResponse = true;
				hasResponse4 = true;
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
				
				if (hasResponse3)
				{
					responses[responseIndex++] = new string_id (c_stringFile, "s_cd7a3f41");
				}
				
				if (hasResponse4)
				{
					responses[responseIndex++] = new string_id (c_stringFile, "s_3aa18b2d");
				}
				
				utils.setScriptVar (player, "conversation.junk_dealer_generic.branchId", 1);
				
				npcStartConversation (player, npc, "junk_dealer_generic", message, responses);
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
		if (!conversationId.equals("junk_dealer_generic"))
		{
			return SCRIPT_CONTINUE;
		}
		
		obj_id npc = self;
		
		int branchId = utils.getIntScriptVar (player, "conversation.junk_dealer_generic.branchId");
		
		if (branchId == 1 && junk_dealer_generic_handleBranch1 (player, npc, response) == SCRIPT_CONTINUE)
		{
			return SCRIPT_CONTINUE;
		}
		
		if (branchId == 7 && junk_dealer_generic_handleBranch7 (player, npc, response) == SCRIPT_CONTINUE)
		{
			return SCRIPT_CONTINUE;
		}
		
		if (branchId == 8 && junk_dealer_generic_handleBranch8 (player, npc, response) == SCRIPT_CONTINUE)
		{
			return SCRIPT_CONTINUE;
		}
		
		if (branchId == 9 && junk_dealer_generic_handleBranch9 (player, npc, response) == SCRIPT_CONTINUE)
		{
			return SCRIPT_CONTINUE;
		}
		
		if (branchId == 10 && junk_dealer_generic_handleBranch10 (player, npc, response) == SCRIPT_CONTINUE)
		{
			return SCRIPT_CONTINUE;
		}
		
		if (branchId == 11 && junk_dealer_generic_handleBranch11 (player, npc, response) == SCRIPT_CONTINUE)
		{
			return SCRIPT_CONTINUE;
		}
		
		chat.chat (npc, "Error: Fell through all branches and responses for OnNpcConversationResponse.");
		
		utils.removeScriptVar (player, "conversation.junk_dealer_generic.branchId");
		
		return SCRIPT_CONTINUE;
	}
	
}
