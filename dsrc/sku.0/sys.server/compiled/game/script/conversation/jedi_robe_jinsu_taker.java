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
import script.library.static_item;
import script.library.utils;


public class jedi_robe_jinsu_taker extends script.base_script
{
	public jedi_robe_jinsu_taker()
	{
	}
	String c_stringFile = "conversation/jedi_robe_jinsu_taker";
	
	
	public boolean jedi_robe_jinsu_taker_condition__defaultCondition(obj_id player, obj_id npc) throws InterruptedException
	{
		return true;
	}
	
	
	public boolean jedi_robe_jinsu_taker_condition_readyToHandOverJinsu(obj_id player, obj_id npc) throws InterruptedException
	{
		if (hasCompletedCollectionSlotPrereq(player, "col_lightsaber_1h_01_02"))
		{
			if (!hasCompletedCollectionSlot(player, "col_lightsaber_1h_01_02"))
			{
				String jinsuTemplate = "object/weapon/melee/sword/crafted_saber/sword_lightsaber_mandalorian.iff";
				if (utils.playerHasItemByTemplateInInventoryOrEquipped(player, jinsuTemplate))
				{
					return true;
				}
			}
		}
		
		return false;
	}
	
	
	public boolean jedi_robe_jinsu_taker_condition_alreadyGaveJinsu(obj_id player, obj_id npc) throws InterruptedException
	{
		return hasCompletedCollectionSlot(player, "col_lightsaber_1h_01_02");
	}
	
	
	public boolean jedi_robe_jinsu_taker_condition_takeJinsu(obj_id player, obj_id npc) throws InterruptedException
	{
		obj_id jinsuSaber = obj_id.NULL_ID;
		String desiredTemplate = "object/weapon/melee/sword/crafted_saber/sword_lightsaber_mandalorian.iff";
		
		obj_id[] contents = getInventoryAndEquipment(player);
		if (contents != null)
		{
			for (int i = 0; i<contents.length; i++)
			{
				testAbortScript();
				obj_id inventoryObject = contents[i];
				String inventoryObjectTemplate = getTemplateName(contents[i]);
				if (inventoryObjectTemplate.equals(desiredTemplate))
				{
					jinsuSaber = inventoryObject;
					break;
				}
			}
		}
		
		obj_id playerInv = utils.getInventoryContainer(player);
		if (isIdValid(jinsuSaber) && isIdValid(playerInv))
		{
			putInOverloaded(jinsuSaber, playerInv);
			
			obj_id jinsuInv = getObjectInSlot(jinsuSaber, "saber_inv");
			if (isIdValid(jinsuInv))
			{
				obj_id[] jinsuCrystals = getContents(jinsuInv);
				if (jinsuCrystals != null && jinsuCrystals.length > 0)
				{
					for (int i = 0; i < jinsuCrystals.length; i++)
					{
						testAbortScript();
						obj_id crystal = jinsuCrystals[i];
						if (isIdValid(crystal))
						{
							putInOverloaded(crystal, playerInv);
						}
					}
				}
			}
			
			destroyObject(jinsuSaber);
			modifyCollectionSlotValue(player, "col_lightsaber_1h_01_02", 1);
			return true;
		}
		
		return false;
	}
	
	
	public boolean jedi_robe_jinsu_taker_condition_canReceive5GenPvP(obj_id player, obj_id npc) throws InterruptedException
	{
		obj_id pvpSaberSchematic1Hand = utils.getStaticItemInInventory(player, "item_schematic_pvp_bf_saber_03_01");
		if (isIdValid(pvpSaberSchematic1Hand) && hasCompletedCollection(player, "col_lightsaber_1h_01"))
		{
			return true;
		}
		
		obj_id pvpSaberSchematic2Hand = utils.getStaticItemInInventory(player, "item_schematic_pvp_bf_saber_03_02");
		if (isIdValid(pvpSaberSchematic2Hand) && hasCompletedCollection(player, "col_lightsaber_2h_01"))
		{
			return true;
		}
		
		obj_id pvpSaberSchematicStaff = utils.getStaticItemInInventory(player, "item_schematic_pvp_bf_saber_03_03");
		if (isIdValid(pvpSaberSchematicStaff) && hasCompletedCollection(player, "col_lightsaber_polearm_01"))
		{
			return true;
		}
		
		return false;
	}
	
	
	public boolean jedi_robe_jinsu_taker_condition_canTrade1Hand(obj_id player, obj_id npc) throws InterruptedException
	{
		return isIdValid(utils.getStaticItemInInventory(player, "item_schematic_pvp_bf_saber_03_01"));
	}
	
	
	public boolean jedi_robe_jinsu_taker_condition_canTrade2Hand(obj_id player, obj_id npc) throws InterruptedException
	{
		return isIdValid(utils.getStaticItemInInventory(player, "item_schematic_pvp_bf_saber_03_02"));
	}
	
	
	public boolean jedi_robe_jinsu_taker_condition_canTradeStaff(obj_id player, obj_id npc) throws InterruptedException
	{
		return isIdValid(utils.getStaticItemInInventory(player, "item_schematic_pvp_bf_saber_03_03"));
	}
	
	
	public boolean jedi_robe_jinsu_taker_condition_canReceive5thGenSaber(obj_id player, obj_id npc) throws InterruptedException
	{
		return jedi_robe_jinsu_taker_condition_canReceive5GenPvP(player, npc) || jedi_robe_jinsu_taker_condition_canReceive5GenGCW(player, npc);
	}
	
	
	public boolean jedi_robe_jinsu_taker_condition_canReceive5GenGCW(obj_id player, obj_id npc) throws InterruptedException
	{
		obj_id gcwSaberSchematic1Hand = utils.getStaticItemInInventory(player, "item_schematic_pvp_gcw_saber_1h_gen4");
		if (isIdValid(gcwSaberSchematic1Hand) && hasCompletedCollection(player, "col_lightsaber_1h_01"))
		{
			return true;
		}
		
		obj_id gcwSaberSchematic2Hand = utils.getStaticItemInInventory(player, "item_schematic_pvp_gcw_saber_2h_gen4");
		if (isIdValid(gcwSaberSchematic2Hand) && hasCompletedCollection(player, "col_lightsaber_2h_01"))
		{
			return true;
		}
		return false;
	}
	
	
	public boolean jedi_robe_jinsu_taker_condition_canTradeGcw1Hand(obj_id player, obj_id npc) throws InterruptedException
	{
		return isIdValid(utils.getStaticItemInInventory(player, "item_schematic_pvp_gcw_saber_1h_gen4"));
	}
	
	
	public boolean jedi_robe_jinsu_taker_condition_canTradeGcw2Hand(obj_id player, obj_id npc) throws InterruptedException
	{
		return isIdValid(utils.getStaticItemInInventory(player, "item_schematic_pvp_gcw_saber_2h_gen4"));
	}
	
	
	public void jedi_robe_jinsu_taker_action_upgrade1HandPvP(obj_id player, obj_id npc) throws InterruptedException
	{
		obj_id oldSchematic = utils.getStaticItemInInventory(player, "item_schematic_pvp_bf_saber_03_01");
		obj_id newSchematic = static_item.createNewItemFunction("item_schematic_pvp_bf_saber_03_04", player);
		
		if ((isIdValid(oldSchematic) && utils.getContainingPlayer(oldSchematic) == player) && isIdValid(newSchematic))
		{
			CustomerServiceLog("jediSaberTrade", getFirstName(player) + "("+ player + ") has upgraded their saber schematic "+ getStaticItemName(oldSchematic)+ "("+ oldSchematic + ") to a 5th Gen version "+ getStaticItemName(newSchematic) +"("+newSchematic+"). This will destroy the old one.");
			destroyObject(oldSchematic);
		}
		else
		{
			sendSystemMessage(player, new string_id("spam", "couldnt_upgrade_saber_schematic"));
			CustomerServiceLog("jediSaberTrade", getFirstName(player) + "("+ player + ") could NOT upgrade their schematic "+ getStaticItemName(oldSchematic)+ "("+ oldSchematic + ") to a 5th Gen version. One of these statements returned false: (isIdValid(oldSchematic) && utils.getContainingPlayer(oldSchematic) == player) = "+ (isIdValid(oldSchematic) && utils.getContainingPlayer(oldSchematic) == player)+ "or isIdValid(newSchematic) = "+ isIdValid(newSchematic));
		}
	}
	
	
	public void jedi_robe_jinsu_taker_action_upgrade2HandPvP(obj_id player, obj_id npc) throws InterruptedException
	{
		obj_id oldSchematic = utils.getStaticItemInInventory(player, "item_schematic_pvp_bf_saber_03_02");
		obj_id newSchematic = static_item.createNewItemFunction("item_schematic_pvp_bf_saber_03_05", player);
		
		if ((isIdValid(oldSchematic) && utils.getContainingPlayer(oldSchematic) == player) && isIdValid(newSchematic))
		{
			CustomerServiceLog("jediSaberTrade", getFirstName(player) + "("+ player + ") has upgraded their saber schematic "+ getStaticItemName(oldSchematic)+ "("+ oldSchematic + ") to a 5th Gen version "+ getStaticItemName(newSchematic) +"("+newSchematic+"). This will destroy the old one.");
			destroyObject(oldSchematic);
		}
		else
		{
			sendSystemMessage(player, new string_id("spam", "couldnt_upgrade_saber_schematic"));
			CustomerServiceLog("jediSaberTrade", getFirstName(player) + "("+ player + ") could NOT upgrade their schematic "+ getStaticItemName(oldSchematic)+ "("+ oldSchematic + ") to a 5th Gen version. One of these statements returned false: (isIdValid(oldSchematic) && utils.getContainingPlayer(oldSchematic) == player) = "+ (isIdValid(oldSchematic) && utils.getContainingPlayer(oldSchematic) == player)+ "or isIdValid(newSchematic) = "+ isIdValid(newSchematic));
		}
	}
	
	
	public void jedi_robe_jinsu_taker_action_upgradeStaffPvP(obj_id player, obj_id npc) throws InterruptedException
	{
		obj_id oldSchematic = utils.getStaticItemInInventory(player, "item_schematic_pvp_bf_saber_03_03");
		obj_id newSchematic = static_item.createNewItemFunction("item_schematic_pvp_bf_saber_03_06", player);
		
		if ((isIdValid(oldSchematic) && utils.getContainingPlayer(oldSchematic) == player) && isIdValid(newSchematic))
		{
			CustomerServiceLog("jediSaberTrade", getFirstName(player) + "("+ player + ") has upgraded their saber schematic "+ getStaticItemName(oldSchematic)+ "("+ oldSchematic + ") to a 5th Gen version "+ getStaticItemName(newSchematic) +"("+newSchematic+"). This will destroy the old one.");
			destroyObject(oldSchematic);
		}
		else
		{
			sendSystemMessage(player, new string_id("spam", "couldnt_upgrade_saber_schematic"));
			CustomerServiceLog("jediSaberTrade", getFirstName(player) + "("+ player + ") could NOT upgrade their schematic "+ getStaticItemName(oldSchematic)+ "("+ oldSchematic + ") to a 5th Gen version. One of these statements returned false: (isIdValid(oldSchematic) && utils.getContainingPlayer(oldSchematic) == player) = "+ (isIdValid(oldSchematic) && utils.getContainingPlayer(oldSchematic) == player)+ "or isIdValid(newSchematic) = "+ isIdValid(newSchematic));
		}
	}
	
	
	public void jedi_robe_jinsu_taker_action_upgradeGcw1Handed(obj_id player, obj_id npc) throws InterruptedException
	{
		obj_id oldSchematic = utils.getStaticItemInInventory(player, "item_schematic_pvp_gcw_saber_1h_gen4");
		obj_id newSchematic = static_item.createNewItemFunction("item_schematic_pvp_gcw_saber_1h_gen5", player);
		
		if ((isIdValid(oldSchematic) && utils.getContainingPlayer(oldSchematic) == player) && isIdValid(newSchematic))
		{
			CustomerServiceLog("jediSaberTrade", getFirstName(player) + "("+ player + ") has upgraded their saber schematic "+ getStaticItemName(oldSchematic)+ "("+ oldSchematic + ") to a 5th Gen version "+ getStaticItemName(newSchematic) +"("+newSchematic+"). This will destroy the old one.");
			destroyObject(oldSchematic);
		}
		else
		{
			sendSystemMessage(player, new string_id("spam", "couldnt_upgrade_saber_schematic"));
			CustomerServiceLog("jediSaberTrade", getFirstName(player) + "("+ player + ") could NOT upgrade their schematic "+ getStaticItemName(oldSchematic)+ "("+ oldSchematic + ") to a 5th Gen version. One of these statements returned false: (isIdValid(oldSchematic) && utils.getContainingPlayer(oldSchematic) == player) = "+ (isIdValid(oldSchematic) && utils.getContainingPlayer(oldSchematic) == player)+ "or isIdValid(newSchematic) = "+ isIdValid(newSchematic));
		}
	}
	
	
	public void jedi_robe_jinsu_taker_action_upgradeGcw2Handed(obj_id player, obj_id npc) throws InterruptedException
	{
		obj_id oldSchematic = utils.getStaticItemInInventory(player, "item_schematic_pvp_gcw_saber_2h_gen4");
		obj_id newSchematic = static_item.createNewItemFunction("item_schematic_pvp_gcw_saber_2h_gen5", player);
		
		if ((isIdValid(oldSchematic) && utils.getContainingPlayer(oldSchematic) == player) && isIdValid(newSchematic))
		{
			CustomerServiceLog("jediSaberTrade", getFirstName(player) + "("+ player + ") has upgraded their saber schematic "+ getStaticItemName(oldSchematic)+ "("+ oldSchematic + ") to a 5th Gen version "+ getStaticItemName(newSchematic) +"("+newSchematic+"). This will destroy the old one.");
			destroyObject(oldSchematic);
		}
		else
		{
			sendSystemMessage(player, new string_id("spam", "couldnt_upgrade_saber_schematic"));
			CustomerServiceLog("jediSaberTrade", getFirstName(player) + "("+ player + ") could NOT upgrade their schematic "+ getStaticItemName(oldSchematic)+ "("+ oldSchematic + ") to a 5th Gen version. One of these statements returned false: (isIdValid(oldSchematic) && utils.getContainingPlayer(oldSchematic) == player) = "+ (isIdValid(oldSchematic) && utils.getContainingPlayer(oldSchematic) == player)+ "or isIdValid(newSchematic) = "+ isIdValid(newSchematic));
		}
	}
	
	
	public int jedi_robe_jinsu_taker_handleBranch1(obj_id player, obj_id npc, string_id response) throws InterruptedException
	{
		
		if (response.equals("s_29"))
		{
			
			if (jedi_robe_jinsu_taker_condition__defaultCondition (player, npc))
			{
				
				string_id message = new string_id (c_stringFile, "s_35");
				int numberOfResponses = 0;
				
				boolean hasResponse = false;
				
				boolean hasResponse0 = false;
				if (jedi_robe_jinsu_taker_condition_canTrade1Hand (player, npc))
				{
					++numberOfResponses;
					hasResponse = true;
					hasResponse0 = true;
				}
				
				boolean hasResponse1 = false;
				if (jedi_robe_jinsu_taker_condition_canTrade2Hand (player, npc))
				{
					++numberOfResponses;
					hasResponse = true;
					hasResponse1 = true;
				}
				
				boolean hasResponse2 = false;
				if (jedi_robe_jinsu_taker_condition_canTradeStaff (player, npc))
				{
					++numberOfResponses;
					hasResponse = true;
					hasResponse2 = true;
				}
				
				boolean hasResponse3 = false;
				if (jedi_robe_jinsu_taker_condition_canTradeGcw1Hand (player, npc))
				{
					++numberOfResponses;
					hasResponse = true;
					hasResponse3 = true;
				}
				
				boolean hasResponse4 = false;
				if (jedi_robe_jinsu_taker_condition_canTradeGcw2Hand (player, npc))
				{
					++numberOfResponses;
					hasResponse = true;
					hasResponse4 = true;
				}
				
				boolean hasResponse5 = false;
				if (jedi_robe_jinsu_taker_condition__defaultCondition (player, npc))
				{
					++numberOfResponses;
					hasResponse = true;
					hasResponse5 = true;
				}
				
				if (hasResponse)
				{
					int responseIndex = 0;
					string_id responses[] = new string_id[numberOfResponses];
					
					if (hasResponse0)
					{
						responses[responseIndex++] = new string_id (c_stringFile, "s_38");
					}
					
					if (hasResponse1)
					{
						responses[responseIndex++] = new string_id (c_stringFile, "s_45");
					}
					
					if (hasResponse2)
					{
						responses[responseIndex++] = new string_id (c_stringFile, "s_46");
					}
					
					if (hasResponse3)
					{
						responses[responseIndex++] = new string_id (c_stringFile, "s_63");
					}
					
					if (hasResponse4)
					{
						responses[responseIndex++] = new string_id (c_stringFile, "s_64");
					}
					
					if (hasResponse5)
					{
						responses[responseIndex++] = new string_id (c_stringFile, "s_56");
					}
					
					utils.setScriptVar (player, "conversation.jedi_robe_jinsu_taker.branchId", 2);
					
					npcSpeak (player, message);
					npcSetConversationResponses (player, responses);
				}
				else
				{
					utils.removeScriptVar (player, "conversation.jedi_robe_jinsu_taker.branchId");
					
					npcEndConversationWithMessage (player, message);
				}
				
				return SCRIPT_CONTINUE;
			}
			
		}
		
		if (response.equals("s_33"))
		{
			
			if (jedi_robe_jinsu_taker_condition__defaultCondition (player, npc))
			{
				
				string_id message = new string_id (c_stringFile, "s_51");
				utils.removeScriptVar (player, "conversation.jedi_robe_jinsu_taker.branchId");
				
				npcEndConversationWithMessage (player, message);
				
				return SCRIPT_CONTINUE;
			}
			
		}
		
		return SCRIPT_DEFAULT;
	}
	
	
	public int jedi_robe_jinsu_taker_handleBranch2(obj_id player, obj_id npc, string_id response) throws InterruptedException
	{
		
		if (response.equals("s_38"))
		{
			
			if (jedi_robe_jinsu_taker_condition_canTrade1Hand (player, npc))
			{
				jedi_robe_jinsu_taker_action_upgrade1HandPvP (player, npc);
				
				string_id message = new string_id (c_stringFile, "s_40");
				int numberOfResponses = 0;
				
				boolean hasResponse = false;
				
				boolean hasResponse0 = false;
				if (jedi_robe_jinsu_taker_condition__defaultCondition (player, npc))
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
						responses[responseIndex++] = new string_id (c_stringFile, "s_42");
					}
					
					utils.setScriptVar (player, "conversation.jedi_robe_jinsu_taker.branchId", 3);
					
					npcSpeak (player, message);
					npcSetConversationResponses (player, responses);
				}
				else
				{
					utils.removeScriptVar (player, "conversation.jedi_robe_jinsu_taker.branchId");
					
					npcEndConversationWithMessage (player, message);
				}
				
				return SCRIPT_CONTINUE;
			}
			
			if (jedi_robe_jinsu_taker_condition__defaultCondition (player, npc))
			{
				
				string_id message = new string_id (c_stringFile, "s_53");
				utils.removeScriptVar (player, "conversation.jedi_robe_jinsu_taker.branchId");
				
				npcEndConversationWithMessage (player, message);
				
				return SCRIPT_CONTINUE;
			}
			
		}
		
		if (response.equals("s_45"))
		{
			
			if (jedi_robe_jinsu_taker_condition_canTrade2Hand (player, npc))
			{
				jedi_robe_jinsu_taker_action_upgrade2HandPvP (player, npc);
				
				string_id message = new string_id (c_stringFile, "s_48");
				int numberOfResponses = 0;
				
				boolean hasResponse = false;
				
				boolean hasResponse0 = false;
				if (jedi_robe_jinsu_taker_condition__defaultCondition (player, npc))
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
						responses[responseIndex++] = new string_id (c_stringFile, "s_42");
					}
					
					utils.setScriptVar (player, "conversation.jedi_robe_jinsu_taker.branchId", 3);
					
					npcSpeak (player, message);
					npcSetConversationResponses (player, responses);
				}
				else
				{
					utils.removeScriptVar (player, "conversation.jedi_robe_jinsu_taker.branchId");
					
					npcEndConversationWithMessage (player, message);
				}
				
				return SCRIPT_CONTINUE;
			}
			
			if (jedi_robe_jinsu_taker_condition__defaultCondition (player, npc))
			{
				
				string_id message = new string_id (c_stringFile, "s_54");
				utils.removeScriptVar (player, "conversation.jedi_robe_jinsu_taker.branchId");
				
				npcEndConversationWithMessage (player, message);
				
				return SCRIPT_CONTINUE;
			}
			
		}
		
		if (response.equals("s_46"))
		{
			
			if (jedi_robe_jinsu_taker_condition_canTradeStaff (player, npc))
			{
				jedi_robe_jinsu_taker_action_upgradeStaffPvP (player, npc);
				
				string_id message = new string_id (c_stringFile, "s_50");
				int numberOfResponses = 0;
				
				boolean hasResponse = false;
				
				boolean hasResponse0 = false;
				if (jedi_robe_jinsu_taker_condition__defaultCondition (player, npc))
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
						responses[responseIndex++] = new string_id (c_stringFile, "s_42");
					}
					
					utils.setScriptVar (player, "conversation.jedi_robe_jinsu_taker.branchId", 3);
					
					npcSpeak (player, message);
					npcSetConversationResponses (player, responses);
				}
				else
				{
					utils.removeScriptVar (player, "conversation.jedi_robe_jinsu_taker.branchId");
					
					npcEndConversationWithMessage (player, message);
				}
				
				return SCRIPT_CONTINUE;
			}
			
			if (jedi_robe_jinsu_taker_condition__defaultCondition (player, npc))
			{
				
				string_id message = new string_id (c_stringFile, "s_55");
				utils.removeScriptVar (player, "conversation.jedi_robe_jinsu_taker.branchId");
				
				npcEndConversationWithMessage (player, message);
				
				return SCRIPT_CONTINUE;
			}
			
		}
		
		if (response.equals("s_63"))
		{
			
			if (jedi_robe_jinsu_taker_condition_canTradeGcw1Hand (player, npc))
			{
				jedi_robe_jinsu_taker_action_upgradeGcw1Handed (player, npc);
				
				string_id message = new string_id (c_stringFile, "s_65");
				utils.removeScriptVar (player, "conversation.jedi_robe_jinsu_taker.branchId");
				
				npcEndConversationWithMessage (player, message);
				
				return SCRIPT_CONTINUE;
			}
			
			if (jedi_robe_jinsu_taker_condition__defaultCondition (player, npc))
			{
				
				string_id message = new string_id (c_stringFile, "s_67");
				utils.removeScriptVar (player, "conversation.jedi_robe_jinsu_taker.branchId");
				
				npcEndConversationWithMessage (player, message);
				
				return SCRIPT_CONTINUE;
			}
			
		}
		
		if (response.equals("s_64"))
		{
			
			if (jedi_robe_jinsu_taker_condition_canTradeGcw2Hand (player, npc))
			{
				jedi_robe_jinsu_taker_action_upgradeGcw2Handed (player, npc);
				
				string_id message = new string_id (c_stringFile, "s_66");
				utils.removeScriptVar (player, "conversation.jedi_robe_jinsu_taker.branchId");
				
				npcEndConversationWithMessage (player, message);
				
				return SCRIPT_CONTINUE;
			}
			
			if (jedi_robe_jinsu_taker_condition__defaultCondition (player, npc))
			{
				
				string_id message = new string_id (c_stringFile, "s_68");
				utils.removeScriptVar (player, "conversation.jedi_robe_jinsu_taker.branchId");
				
				npcEndConversationWithMessage (player, message);
				
				return SCRIPT_CONTINUE;
			}
			
		}
		
		if (response.equals("s_56"))
		{
			
			if (jedi_robe_jinsu_taker_condition__defaultCondition (player, npc))
			{
				
				string_id message = new string_id (c_stringFile, "s_57");
				utils.removeScriptVar (player, "conversation.jedi_robe_jinsu_taker.branchId");
				
				npcEndConversationWithMessage (player, message);
				
				return SCRIPT_CONTINUE;
			}
			
		}
		
		return SCRIPT_DEFAULT;
	}
	
	
	public int jedi_robe_jinsu_taker_handleBranch3(obj_id player, obj_id npc, string_id response) throws InterruptedException
	{
		
		if (response.equals("s_42"))
		{
			
			if (jedi_robe_jinsu_taker_condition__defaultCondition (player, npc))
			{
				
				string_id message = new string_id (c_stringFile, "s_44");
				utils.removeScriptVar (player, "conversation.jedi_robe_jinsu_taker.branchId");
				
				npcEndConversationWithMessage (player, message);
				
				return SCRIPT_CONTINUE;
			}
			
		}
		
		return SCRIPT_DEFAULT;
	}
	
	
	public int jedi_robe_jinsu_taker_handleBranch6(obj_id player, obj_id npc, string_id response) throws InterruptedException
	{
		
		if (response.equals("s_42"))
		{
			
			if (jedi_robe_jinsu_taker_condition__defaultCondition (player, npc))
			{
				
				string_id message = new string_id (c_stringFile, "s_44");
				utils.removeScriptVar (player, "conversation.jedi_robe_jinsu_taker.branchId");
				
				npcEndConversationWithMessage (player, message);
				
				return SCRIPT_CONTINUE;
			}
			
		}
		
		return SCRIPT_DEFAULT;
	}
	
	
	public int jedi_robe_jinsu_taker_handleBranch8(obj_id player, obj_id npc, string_id response) throws InterruptedException
	{
		
		if (response.equals("s_42"))
		{
			
			if (jedi_robe_jinsu_taker_condition__defaultCondition (player, npc))
			{
				
				string_id message = new string_id (c_stringFile, "s_44");
				utils.removeScriptVar (player, "conversation.jedi_robe_jinsu_taker.branchId");
				
				npcEndConversationWithMessage (player, message);
				
				return SCRIPT_CONTINUE;
			}
			
		}
		
		return SCRIPT_DEFAULT;
	}
	
	
	public int jedi_robe_jinsu_taker_handleBranch17(obj_id player, obj_id npc, string_id response) throws InterruptedException
	{
		
		if (response.equals("s_36"))
		{
			
			if (jedi_robe_jinsu_taker_condition__defaultCondition (player, npc))
			{
				
				string_id message = new string_id (c_stringFile, "s_39");
				int numberOfResponses = 0;
				
				boolean hasResponse = false;
				
				boolean hasResponse0 = false;
				if (jedi_robe_jinsu_taker_condition__defaultCondition (player, npc))
				{
					++numberOfResponses;
					hasResponse = true;
					hasResponse0 = true;
				}
				
				boolean hasResponse1 = false;
				if (jedi_robe_jinsu_taker_condition__defaultCondition (player, npc))
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
						responses[responseIndex++] = new string_id (c_stringFile, "s_43");
					}
					
					if (hasResponse1)
					{
						responses[responseIndex++] = new string_id (c_stringFile, "s_60");
					}
					
					utils.setScriptVar (player, "conversation.jedi_robe_jinsu_taker.branchId", 18);
					
					npcSpeak (player, message);
					npcSetConversationResponses (player, responses);
				}
				else
				{
					utils.removeScriptVar (player, "conversation.jedi_robe_jinsu_taker.branchId");
					
					npcEndConversationWithMessage (player, message);
				}
				
				return SCRIPT_CONTINUE;
			}
			
		}
		
		if (response.equals("s_70"))
		{
			
			if (jedi_robe_jinsu_taker_condition__defaultCondition (player, npc))
			{
				doAnimationAction (npc, "applause_polite");
				
				string_id message = new string_id (c_stringFile, "s_72");
				utils.removeScriptVar (player, "conversation.jedi_robe_jinsu_taker.branchId");
				
				npcEndConversationWithMessage (player, message);
				
				return SCRIPT_CONTINUE;
			}
			
		}
		
		return SCRIPT_DEFAULT;
	}
	
	
	public int jedi_robe_jinsu_taker_handleBranch18(obj_id player, obj_id npc, string_id response) throws InterruptedException
	{
		
		if (response.equals("s_43"))
		{
			
			if (jedi_robe_jinsu_taker_condition_takeJinsu (player, npc))
			{
				doAnimationAction (npc, "bow");
				
				string_id message = new string_id (c_stringFile, "s_49");
				utils.removeScriptVar (player, "conversation.jedi_robe_jinsu_taker.branchId");
				
				npcEndConversationWithMessage (player, message);
				
				return SCRIPT_CONTINUE;
			}
			
			if (jedi_robe_jinsu_taker_condition__defaultCondition (player, npc))
			{
				doAnimationAction (npc, "shoo");
				
				string_id message = new string_id (c_stringFile, "s_58");
				utils.removeScriptVar (player, "conversation.jedi_robe_jinsu_taker.branchId");
				
				npcEndConversationWithMessage (player, message);
				
				return SCRIPT_CONTINUE;
			}
			
		}
		
		if (response.equals("s_60"))
		{
			
			if (jedi_robe_jinsu_taker_condition__defaultCondition (player, npc))
			{
				doAnimationAction (npc, "shake_head_disgust");
				
				string_id message = new string_id (c_stringFile, "s_62");
				utils.removeScriptVar (player, "conversation.jedi_robe_jinsu_taker.branchId");
				
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
			detachScript(self, "conversation.jedi_robe_jinsu_taker");
		}
		
		setCondition (self, CONDITION_CONVERSABLE);
		ai_lib.setDefaultCalmBehavior(self, ai_lib.BEHAVIOR_SENTINEL);
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int OnAttach(obj_id self) throws InterruptedException
	{
		setCondition (self, CONDITION_CONVERSABLE);
		ai_lib.setDefaultCalmBehavior(self, ai_lib.BEHAVIOR_SENTINEL);
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int OnObjectMenuRequest(obj_id self, obj_id player, menu_info menuInfo) throws InterruptedException
	{
		int menu = menuInfo.addRootMenu (menu_info_types.CONVERSE_START, null);
		menu_info_data menuInfoData = menuInfo.getMenuItemById (menu);
		menuInfoData.setServerNotify (false);
		setCondition (self, CONDITION_CONVERSABLE);
		faceTo( self, player );
		return SCRIPT_CONTINUE;
	}
	
	
	public int OnIncapacitated(obj_id self, obj_id killer) throws InterruptedException
	{
		clearCondition (self, CONDITION_CONVERSABLE);
		detachScript (self, "conversation.jedi_robe_jinsu_taker");
		
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
		
		if (jedi_robe_jinsu_taker_condition_canReceive5thGenSaber (player, npc))
		{
			
			string_id message = new string_id (c_stringFile, "s_27");
			int numberOfResponses = 0;
			
			boolean hasResponse = false;
			
			boolean hasResponse0 = false;
			if (jedi_robe_jinsu_taker_condition__defaultCondition (player, npc))
			{
				++numberOfResponses;
				hasResponse = true;
				hasResponse0 = true;
			}
			
			boolean hasResponse1 = false;
			if (jedi_robe_jinsu_taker_condition__defaultCondition (player, npc))
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
					responses[responseIndex++] = new string_id (c_stringFile, "s_29");
				}
				
				if (hasResponse1)
				{
					responses[responseIndex++] = new string_id (c_stringFile, "s_33");
				}
				
				utils.setScriptVar (player, "conversation.jedi_robe_jinsu_taker.branchId", 1);
				
				prose_package pp = new prose_package ();
				pp.stringId = message;
				pp.actor.set (player);
				pp.target.set (npc);
				
				npcStartConversation (player, npc, "jedi_robe_jinsu_taker", null, pp, responses);
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
		
		if (jedi_robe_jinsu_taker_condition_alreadyGaveJinsu (player, npc))
		{
			
			string_id message = new string_id (c_stringFile, "s_30");
			chat.chat (npc, player, message);
			
			return SCRIPT_CONTINUE;
		}
		
		if (jedi_robe_jinsu_taker_condition_readyToHandOverJinsu (player, npc))
		{
			
			string_id message = new string_id (c_stringFile, "s_32");
			int numberOfResponses = 0;
			
			boolean hasResponse = false;
			
			boolean hasResponse0 = false;
			if (jedi_robe_jinsu_taker_condition__defaultCondition (player, npc))
			{
				++numberOfResponses;
				hasResponse = true;
				hasResponse0 = true;
			}
			
			boolean hasResponse1 = false;
			if (jedi_robe_jinsu_taker_condition__defaultCondition (player, npc))
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
					responses[responseIndex++] = new string_id (c_stringFile, "s_36");
				}
				
				if (hasResponse1)
				{
					responses[responseIndex++] = new string_id (c_stringFile, "s_70");
				}
				
				utils.setScriptVar (player, "conversation.jedi_robe_jinsu_taker.branchId", 17);
				
				npcStartConversation (player, npc, "jedi_robe_jinsu_taker", message, responses);
			}
			else
			{
				chat.chat (npc, player, message);
			}
			
			return SCRIPT_CONTINUE;
		}
		
		if (jedi_robe_jinsu_taker_condition__defaultCondition (player, npc))
		{
			
			string_id message = new string_id (c_stringFile, "s_74");
			chat.chat (npc, player, message);
			
			return SCRIPT_CONTINUE;
		}
		
		chat.chat (npc, "Error: All conditions for OnStartNpcConversation were false.");
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int OnNpcConversationResponse(obj_id self, String conversationId, obj_id player, string_id response) throws InterruptedException
	{
		if (!conversationId.equals("jedi_robe_jinsu_taker"))
		{
			return SCRIPT_CONTINUE;
		}
		
		obj_id npc = self;
		
		int branchId = utils.getIntScriptVar (player, "conversation.jedi_robe_jinsu_taker.branchId");
		
		if (branchId == 1 && jedi_robe_jinsu_taker_handleBranch1 (player, npc, response) == SCRIPT_CONTINUE)
		{
			return SCRIPT_CONTINUE;
		}
		
		if (branchId == 2 && jedi_robe_jinsu_taker_handleBranch2 (player, npc, response) == SCRIPT_CONTINUE)
		{
			return SCRIPT_CONTINUE;
		}
		
		if (branchId == 3 && jedi_robe_jinsu_taker_handleBranch3 (player, npc, response) == SCRIPT_CONTINUE)
		{
			return SCRIPT_CONTINUE;
		}
		
		if (branchId == 6 && jedi_robe_jinsu_taker_handleBranch6 (player, npc, response) == SCRIPT_CONTINUE)
		{
			return SCRIPT_CONTINUE;
		}
		
		if (branchId == 8 && jedi_robe_jinsu_taker_handleBranch8 (player, npc, response) == SCRIPT_CONTINUE)
		{
			return SCRIPT_CONTINUE;
		}
		
		if (branchId == 17 && jedi_robe_jinsu_taker_handleBranch17 (player, npc, response) == SCRIPT_CONTINUE)
		{
			return SCRIPT_CONTINUE;
		}
		
		if (branchId == 18 && jedi_robe_jinsu_taker_handleBranch18 (player, npc, response) == SCRIPT_CONTINUE)
		{
			return SCRIPT_CONTINUE;
		}
		
		chat.chat (npc, "Error: Fell through all branches and responses for OnNpcConversationResponse.");
		
		utils.removeScriptVar (player, "conversation.jedi_robe_jinsu_taker.branchId");
		
		return SCRIPT_CONTINUE;
	}
	
}
