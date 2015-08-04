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
import script.library.collection;
import script.library.conversation;
import script.library.groundquests;
import script.library.static_item;
import script.library.utils;


public class mtp_corellia_times_contact extends script.base_script
{
	public mtp_corellia_times_contact()
	{
	}
	String c_stringFile = "conversation/mtp_corellia_times_contact";
	
	
	public boolean mtp_corellia_times_contact_condition__defaultCondition(obj_id player, obj_id npc) throws InterruptedException
	{
		return true;
	}
	
	
	public boolean mtp_corellia_times_contact_condition_isVaniPointerActive(obj_id player, obj_id npc) throws InterruptedException
	{
		return groundquests.isQuestActive(player, "mtp_hideout_pointer");
	}
	
	
	public boolean mtp_corellia_times_contact_condition_allMeatlumpCollectionsComplete(obj_id player, obj_id npc) throws InterruptedException
	{
		return ((mtp_corellia_times_contact_condition_isMeatlumpFoodCollectionComplete(player, npc) && mtp_corellia_times_contact_condition_isMeatlumpWeaponCollectionComplete(player, npc) && mtp_corellia_times_contact_condition_isMeatlumpBombCollectionComplete(player, npc) && mtp_corellia_times_contact_condition_isMeatlumpMapCollectionComplete(player, npc) && mtp_corellia_times_contact_condition_isMeatlumpContainerCollectionComplete(player, npc)) && mtp_corellia_times_contact_condition_isMeatlumpSafeCollectionComplete(player, npc));
	}
	
	
	public boolean mtp_corellia_times_contact_condition_isMeatlumpSafeCollectionComplete(obj_id player, obj_id npc) throws InterruptedException
	{
		return hasCompletedCollection(player, "col_meatlump_safe_01");
	}
	
	
	public boolean mtp_corellia_times_contact_condition_isMeatlumpContainerCollectionComplete(obj_id player, obj_id npc) throws InterruptedException
	{
		return hasCompletedCollection(player, "col_meatlump_container_01");
	}
	
	
	public boolean mtp_corellia_times_contact_condition_isMeatlumpMapCollectionComplete(obj_id player, obj_id npc) throws InterruptedException
	{
		return hasCompletedCollection(player, "col_meatlump_map_01");
	}
	
	
	public boolean mtp_corellia_times_contact_condition_isMeatlumpBombCollectionComplete(obj_id player, obj_id npc) throws InterruptedException
	{
		return hasCompletedCollection(player, "col_meatlump_bomb_sabotage_01");
	}
	
	
	public boolean mtp_corellia_times_contact_condition_isMeatlumpWeaponCollectionComplete(obj_id player, obj_id npc) throws InterruptedException
	{
		return hasCompletedCollection(player, "col_meatlump_weapon_sabotage_01");
	}
	
	
	public boolean mtp_corellia_times_contact_condition_isMeatlumpFoodCollectionComplete(obj_id player, obj_id npc) throws InterruptedException
	{
		return hasCompletedCollection(player, "col_meatlump_food_sabotage_01");
	}
	
	
	public boolean mtp_corellia_times_contact_condition_mealtumpCollectionCompleteQuestActive(obj_id player, obj_id npc) throws InterruptedException
	{
		return (mtp_corellia_times_contact_condition_allMeatlumpCollectionsComplete(player, npc) && groundquests.isTaskActive(player, "mtp_collection_tracking", "goBackToHaldenWes"));
	}
	
	
	public boolean mtp_corellia_times_contact_condition_repeatMeatlumpQuestComplete(obj_id player, obj_id npc) throws InterruptedException
	{
		return (mtp_corellia_times_contact_condition_allMeatlumpCollectionsComplete(player, npc) && groundquests.isTaskActive(player, "mtp_collection_tracking_02", "goBackToHaldenWes"));
	}
	
	
	public boolean mtp_corellia_times_contact_condition_meatlumpCollectionCompleteQuestComplete(obj_id player, obj_id npc) throws InterruptedException
	{
		return (mtp_corellia_times_contact_condition_allMeatlumpCollectionsComplete(player, npc) && groundquests.hasCompletedQuest(player, "mtp_collection_tracking"));
	}
	
	
	public boolean mtp_corellia_times_contact_condition_hasSafeDeviceStack(obj_id player, obj_id npc) throws InterruptedException
	{
		return utils.playerHasItemByTemplateInBankOrInventory(player, "object/tangible/meatlump/event/slicing_device_meatlump_safe.iff");
	}
	
	
	public boolean mtp_corellia_times_contact_condition_hasContainerDeviceStack(obj_id player, obj_id npc) throws InterruptedException
	{
		return utils.playerHasItemByTemplateInBankOrInventory(player, "object/tangible/meatlump/event/slicing_device_meatlump_container.iff");
	}
	
	
	public boolean mtp_corellia_times_contact_condition_hasMapDeviceStack(obj_id player, obj_id npc) throws InterruptedException
	{
		return utils.playerHasItemByTemplateInBankOrInventory(player, "object/tangible/meatlump/event/slicing_device_meatlump_map.iff");
	}
	
	
	public boolean mtp_corellia_times_contact_condition_hasWeaponDeviceStack(obj_id player, obj_id npc) throws InterruptedException
	{
		return utils.playerHasItemByTemplateInBankOrInventory(player, "object/tangible/meatlump/event/slicing_device_meatlump_weapon.iff");
	}
	
	
	public boolean mtp_corellia_times_contact_condition_hasFoodDeviceStack(obj_id player, obj_id npc) throws InterruptedException
	{
		return utils.playerHasItemByTemplateInBankOrInventory(player, "object/tangible/meatlump/event/slicing_device_meatlump_food.iff");
	}
	
	
	public boolean mtp_corellia_times_contact_condition_hasBombDeviceStack(obj_id player, obj_id npc) throws InterruptedException
	{
		return utils.playerHasItemByTemplateInBankOrInventory(player, "object/tangible/meatlump/event/slicing_device_meatlump_bomb.iff");
	}
	
	
	public boolean mtp_corellia_times_contact_condition_hasAllDevices(obj_id player, obj_id npc) throws InterruptedException
	{
		return (mtp_corellia_times_contact_condition_hasBombDeviceStack(player, npc) && mtp_corellia_times_contact_condition_hasFoodDeviceStack(player, npc) && mtp_corellia_times_contact_condition_hasWeaponDeviceStack(player, npc) && mtp_corellia_times_contact_condition_hasMapDeviceStack(player, npc) && mtp_corellia_times_contact_condition_hasContainerDeviceStack(player, npc) && mtp_corellia_times_contact_condition_hasSafeDeviceStack(player, npc));
	}
	
	
	public boolean mtp_corellia_times_contact_condition_hasCollectionQuestActive(obj_id player, obj_id npc) throws InterruptedException
	{
		return groundquests.isQuestActive(player, "mtp_collection_tracking");
	}
	
	
	public boolean mtp_corellia_times_contact_condition_hasQuestButNotAllDevices(obj_id player, obj_id npc) throws InterruptedException
	{
		return (mtp_corellia_times_contact_condition_hasCollectionQuestActive(player, npc) && !mtp_corellia_times_contact_condition_hasAllDevices(player, npc));
	}
	
	
	public boolean mtp_corellia_times_contact_condition_isReadyForCollectionQuest(obj_id player, obj_id npc) throws InterruptedException
	{
		return (!mtp_corellia_times_contact_condition_hasCollectionQuestActive(player, npc) && (mtp_corellia_times_contact_condition_isVaniPointerActive(player, npc)) || mtp_corellia_times_contact_condition_hasCompletedPrerequisites(player, npc));
	}
	
	
	public boolean mtp_corellia_times_contact_condition_hasCompletedPrerequisites(obj_id player, obj_id npc) throws InterruptedException
	{
		return (groundquests.hasCompletedQuest(player, "mtp_hideout_pointer") && !groundquests.isQuestActive(player, "mtp_corellia_times_contact"));
	}
	
	
	public boolean mtp_corellia_times_contact_condition_isSomehowBroken(obj_id player, obj_id npc) throws InterruptedException
	{
		return (mtp_corellia_times_contact_condition_isMeatlumpBombCollectionComplete(player, npc) && !mtp_corellia_times_contact_condition_hasCollectionQuestActive(player, npc) || mtp_corellia_times_contact_condition_isMeatlumpContainerCollectionComplete(player, npc) && !mtp_corellia_times_contact_condition_hasCollectionQuestActive(player, npc) || mtp_corellia_times_contact_condition_isMeatlumpFoodCollectionComplete(player, npc) && !mtp_corellia_times_contact_condition_hasCollectionQuestActive(player, npc) || mtp_corellia_times_contact_condition_isMeatlumpMapCollectionComplete(player, npc) && !mtp_corellia_times_contact_condition_hasCollectionQuestActive(player, npc) || mtp_corellia_times_contact_condition_isMeatlumpSafeCollectionComplete(player, npc) && !mtp_corellia_times_contact_condition_hasCollectionQuestActive(player, npc) || mtp_corellia_times_contact_condition_isMeatlumpWeaponCollectionComplete(player, npc) && !mtp_corellia_times_contact_condition_hasCollectionQuestActive(player, npc));
	}
	
	
	public void mtp_corellia_times_contact_action_sendVaniPointerSignal(obj_id player, obj_id npc) throws InterruptedException
	{
		if (mtp_corellia_times_contact_condition_isVaniPointerActive(player, npc))
		{
			groundquests.sendSignal(player, "mtp_hideout_pointer_03");
		}
	}
	
	
	public void mtp_corellia_times_contact_action_removeAndReactivateMeatlumpCollectionsAndQuest(obj_id player, obj_id npc) throws InterruptedException
	{
		mtp_corellia_times_contact_action_removeMeatlumpCollections(player, npc);
		mtp_corellia_times_contact_action_activateMeatlumpCollections(player, npc);
		groundquests.clearQuest(player, "mtp_collection_tracking");
		groundquests.grantQuest(player, "mtp_collection_tracking");
	}
	
	
	public void mtp_corellia_times_contact_action_grantMeatlumpCollectionAndQuest(obj_id player, obj_id npc) throws InterruptedException
	{
		mtp_corellia_times_contact_action_activateMeatlumpCollections(player, npc);
		mtp_corellia_times_contact_action_grantCollectionQuest(player, npc);
	}
	
	
	public void mtp_corellia_times_contact_action_activateMeatlumpCollections(obj_id player, obj_id npc) throws InterruptedException
	{
		String activationStringList[] = 
		{
			"meatlump_safe_activation_01",
			"meatlump_container_activation_01",
			"meatlump_map_activation_01",
			"meatlump_bomb_activation_01",
			"meatlump_weapon_activation_01",
			"meatlump_food_activation_01"
		};
		
		for (int b = 0; b < activationStringList.length; b++)
		{
			testAbortScript();
			modifyCollectionSlotValue(player, activationStringList[b], 1);
		}
	}
	
	
	public void mtp_corellia_times_contact_action_removeMeatlumpCollections(obj_id player, obj_id npc) throws InterruptedException
	{
		String collectionStringList[] = 
		{
			"col_meatlump_safe_01",
			"col_meatlump_container_01",
			"col_meatlump_map_01",
			"col_meatlump_bomb_sabotage_01",
			"col_meatlump_weapon_sabotage_01",
			"col_meatlump_food_sabotage_01"
		};
		for (int a = 0; a < collectionStringList.length; a++)
		{
			testAbortScript();
			collection.removeCompletedCollection(player, collectionStringList[a]);
		}
	}
	
	
	public void mtp_corellia_times_contact_action_completeCollectionQuest(obj_id player, obj_id npc) throws InterruptedException
	{
		groundquests.sendSignal(player, "returnToHaldenWes");
	}
	
	
	public void mtp_corellia_times_contact_action_giveSafeDevices(obj_id player, obj_id npc) throws InterruptedException
	{
		obj_id inv = utils.getInventoryContainer(player);
		if (!isIdValid(inv) || !exists(inv))
		{
			return;
		}
		
		static_item.createNewItemFunction("item_meatlump_device_safe_01", inv);
	}
	
	
	public void mtp_corellia_times_contact_action_giveContainerDevices(obj_id player, obj_id npc) throws InterruptedException
	{
		obj_id inv = utils.getInventoryContainer(player);
		if (!isIdValid(inv) || !exists(inv))
		{
			return;
		}
		
		static_item.createNewItemFunction("item_meatlump_device_container_01", inv);
	}
	
	
	public void mtp_corellia_times_contact_action_giveMapDevices(obj_id player, obj_id npc) throws InterruptedException
	{
		obj_id inv = utils.getInventoryContainer(player);
		if (!isIdValid(inv) || !exists(inv))
		{
			return;
		}
		
		static_item.createNewItemFunction("item_meatlump_device_map_01", inv);
	}
	
	
	public void mtp_corellia_times_contact_action_giveBombDevices(obj_id player, obj_id npc) throws InterruptedException
	{
		obj_id inv = utils.getInventoryContainer(player);
		if (!isIdValid(inv) || !exists(inv))
		{
			return;
		}
		
		static_item.createNewItemFunction("item_meatlump_device_bomb_01", inv);
	}
	
	
	public void mtp_corellia_times_contact_action_giveWeaponDevices(obj_id player, obj_id npc) throws InterruptedException
	{
		obj_id inv = utils.getInventoryContainer(player);
		if (!isIdValid(inv) || !exists(inv))
		{
			return;
		}
		
		static_item.createNewItemFunction("item_meatlump_device_weapon_01", inv);
	}
	
	
	public void mtp_corellia_times_contact_action_giveFoodDevices(obj_id player, obj_id npc) throws InterruptedException
	{
		obj_id inv = utils.getInventoryContainer(player);
		if (!isIdValid(inv) || !exists(inv))
		{
			return;
		}
		
		static_item.createNewItemFunction("item_meatlump_device_food_01", inv);
	}
	
	
	public void mtp_corellia_times_contact_action_giveAllDevices(obj_id player, obj_id npc) throws InterruptedException
	{
		mtp_corellia_times_contact_action_giveFoodDevices(player, npc);
		mtp_corellia_times_contact_action_giveWeaponDevices(player, npc);
		mtp_corellia_times_contact_action_giveBombDevices(player, npc);
		mtp_corellia_times_contact_action_giveMapDevices(player, npc);
		mtp_corellia_times_contact_action_giveContainerDevices(player, npc);
		mtp_corellia_times_contact_action_giveSafeDevices(player, npc);
	}
	
	
	public void mtp_corellia_times_contact_action_grantCollectionQuest(obj_id player, obj_id npc) throws InterruptedException
	{
		groundquests.grantQuest(player, "mtp_collection_tracking");
	}
	
	
	public int mtp_corellia_times_contact_handleBranch1(obj_id player, obj_id npc, string_id response) throws InterruptedException
	{
		
		if (response.equals("s_39"))
		{
			
			if (mtp_corellia_times_contact_condition__defaultCondition (player, npc))
			{
				mtp_corellia_times_contact_action_removeAndReactivateMeatlumpCollectionsAndQuest (player, npc);
				
				string_id message = new string_id (c_stringFile, "s_41");
				utils.removeScriptVar (player, "conversation.mtp_corellia_times_contact.branchId");
				
				npcEndConversationWithMessage (player, message);
				
				return SCRIPT_CONTINUE;
			}
			
		}
		
		if (response.equals("s_43"))
		{
			
			if (mtp_corellia_times_contact_condition__defaultCondition (player, npc))
			{
				
				string_id message = new string_id (c_stringFile, "s_45");
				utils.removeScriptVar (player, "conversation.mtp_corellia_times_contact.branchId");
				
				npcEndConversationWithMessage (player, message);
				
				return SCRIPT_CONTINUE;
			}
			
		}
		
		return SCRIPT_DEFAULT;
	}
	
	
	public int mtp_corellia_times_contact_handleBranch5(obj_id player, obj_id npc, string_id response) throws InterruptedException
	{
		
		if (response.equals("s_12"))
		{
			
			if (mtp_corellia_times_contact_condition__defaultCondition (player, npc))
			{
				
				string_id message = new string_id (c_stringFile, "s_14");
				int numberOfResponses = 0;
				
				boolean hasResponse = false;
				
				boolean hasResponse0 = false;
				if (mtp_corellia_times_contact_condition__defaultCondition (player, npc))
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
						responses[responseIndex++] = new string_id (c_stringFile, "s_16");
					}
					
					utils.setScriptVar (player, "conversation.mtp_corellia_times_contact.branchId", 6);
					
					npcSpeak (player, message);
					npcSetConversationResponses (player, responses);
				}
				else
				{
					utils.removeScriptVar (player, "conversation.mtp_corellia_times_contact.branchId");
					
					npcEndConversationWithMessage (player, message);
				}
				
				return SCRIPT_CONTINUE;
			}
			
		}
		
		return SCRIPT_DEFAULT;
	}
	
	
	public int mtp_corellia_times_contact_handleBranch6(obj_id player, obj_id npc, string_id response) throws InterruptedException
	{
		
		if (response.equals("s_16"))
		{
			
			if (mtp_corellia_times_contact_condition__defaultCondition (player, npc))
			{
				
				string_id message = new string_id (c_stringFile, "s_18");
				int numberOfResponses = 0;
				
				boolean hasResponse = false;
				
				boolean hasResponse0 = false;
				if (mtp_corellia_times_contact_condition__defaultCondition (player, npc))
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
						responses[responseIndex++] = new string_id (c_stringFile, "s_21");
					}
					
					utils.setScriptVar (player, "conversation.mtp_corellia_times_contact.branchId", 7);
					
					npcSpeak (player, message);
					npcSetConversationResponses (player, responses);
				}
				else
				{
					utils.removeScriptVar (player, "conversation.mtp_corellia_times_contact.branchId");
					
					npcEndConversationWithMessage (player, message);
				}
				
				return SCRIPT_CONTINUE;
			}
			
		}
		
		return SCRIPT_DEFAULT;
	}
	
	
	public int mtp_corellia_times_contact_handleBranch7(obj_id player, obj_id npc, string_id response) throws InterruptedException
	{
		
		if (response.equals("s_21"))
		{
			doAnimationAction (player, "laugh");
			
			if (mtp_corellia_times_contact_condition__defaultCondition (player, npc))
			{
				
				string_id message = new string_id (c_stringFile, "s_23");
				int numberOfResponses = 0;
				
				boolean hasResponse = false;
				
				boolean hasResponse0 = false;
				if (mtp_corellia_times_contact_condition__defaultCondition (player, npc))
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
						responses[responseIndex++] = new string_id (c_stringFile, "s_25");
					}
					
					utils.setScriptVar (player, "conversation.mtp_corellia_times_contact.branchId", 8);
					
					npcSpeak (player, message);
					npcSetConversationResponses (player, responses);
				}
				else
				{
					utils.removeScriptVar (player, "conversation.mtp_corellia_times_contact.branchId");
					
					npcEndConversationWithMessage (player, message);
				}
				
				return SCRIPT_CONTINUE;
			}
			
		}
		
		return SCRIPT_DEFAULT;
	}
	
	
	public int mtp_corellia_times_contact_handleBranch8(obj_id player, obj_id npc, string_id response) throws InterruptedException
	{
		
		if (response.equals("s_25"))
		{
			
			if (mtp_corellia_times_contact_condition__defaultCondition (player, npc))
			{
				
				string_id message = new string_id (c_stringFile, "s_27");
				int numberOfResponses = 0;
				
				boolean hasResponse = false;
				
				boolean hasResponse0 = false;
				if (mtp_corellia_times_contact_condition__defaultCondition (player, npc))
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
					
					utils.setScriptVar (player, "conversation.mtp_corellia_times_contact.branchId", 9);
					
					npcSpeak (player, message);
					npcSetConversationResponses (player, responses);
				}
				else
				{
					utils.removeScriptVar (player, "conversation.mtp_corellia_times_contact.branchId");
					
					npcEndConversationWithMessage (player, message);
				}
				
				return SCRIPT_CONTINUE;
			}
			
		}
		
		return SCRIPT_DEFAULT;
	}
	
	
	public int mtp_corellia_times_contact_handleBranch9(obj_id player, obj_id npc, string_id response) throws InterruptedException
	{
		
		if (response.equals("s_29"))
		{
			
			if (mtp_corellia_times_contact_condition__defaultCondition (player, npc))
			{
				
				string_id message = new string_id (c_stringFile, "s_31");
				int numberOfResponses = 0;
				
				boolean hasResponse = false;
				
				boolean hasResponse0 = false;
				if (mtp_corellia_times_contact_condition__defaultCondition (player, npc))
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
						responses[responseIndex++] = new string_id (c_stringFile, "s_33");
					}
					
					utils.setScriptVar (player, "conversation.mtp_corellia_times_contact.branchId", 10);
					
					npcSpeak (player, message);
					npcSetConversationResponses (player, responses);
				}
				else
				{
					utils.removeScriptVar (player, "conversation.mtp_corellia_times_contact.branchId");
					
					npcEndConversationWithMessage (player, message);
				}
				
				return SCRIPT_CONTINUE;
			}
			
		}
		
		return SCRIPT_DEFAULT;
	}
	
	
	public int mtp_corellia_times_contact_handleBranch10(obj_id player, obj_id npc, string_id response) throws InterruptedException
	{
		
		if (response.equals("s_33"))
		{
			
			if (mtp_corellia_times_contact_condition__defaultCondition (player, npc))
			{
				mtp_corellia_times_contact_action_giveAllDevices (player, npc);
				
				string_id message = new string_id (c_stringFile, "s_35");
				int numberOfResponses = 0;
				
				boolean hasResponse = false;
				
				boolean hasResponse0 = false;
				if (mtp_corellia_times_contact_condition__defaultCondition (player, npc))
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
						responses[responseIndex++] = new string_id (c_stringFile, "s_38");
					}
					
					utils.setScriptVar (player, "conversation.mtp_corellia_times_contact.branchId", 11);
					
					npcSpeak (player, message);
					npcSetConversationResponses (player, responses);
				}
				else
				{
					utils.removeScriptVar (player, "conversation.mtp_corellia_times_contact.branchId");
					
					npcEndConversationWithMessage (player, message);
				}
				
				return SCRIPT_CONTINUE;
			}
			
		}
		
		return SCRIPT_DEFAULT;
	}
	
	
	public int mtp_corellia_times_contact_handleBranch11(obj_id player, obj_id npc, string_id response) throws InterruptedException
	{
		
		if (response.equals("s_38"))
		{
			
			if (mtp_corellia_times_contact_condition__defaultCondition (player, npc))
			{
				doAnimationAction (npc, "salute2");
				
				string_id message = new string_id (c_stringFile, "s_42");
				int numberOfResponses = 0;
				
				boolean hasResponse = false;
				
				boolean hasResponse0 = false;
				if (mtp_corellia_times_contact_condition__defaultCondition (player, npc))
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
						responses[responseIndex++] = new string_id (c_stringFile, "s_46");
					}
					
					utils.setScriptVar (player, "conversation.mtp_corellia_times_contact.branchId", 12);
					
					npcSpeak (player, message);
					npcSetConversationResponses (player, responses);
				}
				else
				{
					utils.removeScriptVar (player, "conversation.mtp_corellia_times_contact.branchId");
					
					npcEndConversationWithMessage (player, message);
				}
				
				return SCRIPT_CONTINUE;
			}
			
		}
		
		return SCRIPT_DEFAULT;
	}
	
	
	public int mtp_corellia_times_contact_handleBranch12(obj_id player, obj_id npc, string_id response) throws InterruptedException
	{
		
		if (response.equals("s_46"))
		{
			
			if (mtp_corellia_times_contact_condition__defaultCondition (player, npc))
			{
				mtp_corellia_times_contact_action_grantMeatlumpCollectionAndQuest (player, npc);
				
				string_id message = new string_id (c_stringFile, "s_48");
				utils.removeScriptVar (player, "conversation.mtp_corellia_times_contact.branchId");
				
				npcEndConversationWithMessage (player, message);
				
				return SCRIPT_CONTINUE;
			}
			
		}
		
		return SCRIPT_DEFAULT;
	}
	
	
	public int mtp_corellia_times_contact_handleBranch14(obj_id player, obj_id npc, string_id response) throws InterruptedException
	{
		
		if (response.equals("s_52"))
		{
			
			if (mtp_corellia_times_contact_condition__defaultCondition (player, npc))
			{
				mtp_corellia_times_contact_action_giveBombDevices (player, npc);
				
				string_id message = new string_id (c_stringFile, "s_54");
				utils.removeScriptVar (player, "conversation.mtp_corellia_times_contact.branchId");
				
				npcEndConversationWithMessage (player, message);
				
				return SCRIPT_CONTINUE;
			}
			
		}
		
		if (response.equals("s_56"))
		{
			
			if (mtp_corellia_times_contact_condition__defaultCondition (player, npc))
			{
				mtp_corellia_times_contact_action_giveFoodDevices (player, npc);
				
				string_id message = new string_id (c_stringFile, "s_58");
				utils.removeScriptVar (player, "conversation.mtp_corellia_times_contact.branchId");
				
				npcEndConversationWithMessage (player, message);
				
				return SCRIPT_CONTINUE;
			}
			
		}
		
		if (response.equals("s_60"))
		{
			
			if (mtp_corellia_times_contact_condition__defaultCondition (player, npc))
			{
				mtp_corellia_times_contact_action_giveWeaponDevices (player, npc);
				
				string_id message = new string_id (c_stringFile, "s_62");
				utils.removeScriptVar (player, "conversation.mtp_corellia_times_contact.branchId");
				
				npcEndConversationWithMessage (player, message);
				
				return SCRIPT_CONTINUE;
			}
			
		}
		
		if (response.equals("s_64"))
		{
			
			if (mtp_corellia_times_contact_condition__defaultCondition (player, npc))
			{
				mtp_corellia_times_contact_action_giveMapDevices (player, npc);
				
				string_id message = new string_id (c_stringFile, "s_66");
				utils.removeScriptVar (player, "conversation.mtp_corellia_times_contact.branchId");
				
				npcEndConversationWithMessage (player, message);
				
				return SCRIPT_CONTINUE;
			}
			
		}
		
		if (response.equals("s_68"))
		{
			
			if (mtp_corellia_times_contact_condition__defaultCondition (player, npc))
			{
				mtp_corellia_times_contact_action_giveContainerDevices (player, npc);
				
				string_id message = new string_id (c_stringFile, "s_70");
				utils.removeScriptVar (player, "conversation.mtp_corellia_times_contact.branchId");
				
				npcEndConversationWithMessage (player, message);
				
				return SCRIPT_CONTINUE;
			}
			
		}
		
		if (response.equals("s_72"))
		{
			
			if (mtp_corellia_times_contact_condition__defaultCondition (player, npc))
			{
				mtp_corellia_times_contact_action_giveSafeDevices (player, npc);
				
				string_id message = new string_id (c_stringFile, "s_74");
				utils.removeScriptVar (player, "conversation.mtp_corellia_times_contact.branchId");
				
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
			detachScript(self, "conversation.mtp_corellia_times_contact");
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
		detachScript (self, "conversation.mtp_corellia_times_contact");
		
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
		
		if (mtp_corellia_times_contact_condition_meatlumpCollectionCompleteQuestComplete (player, npc))
		{
			
			string_id message = new string_id (c_stringFile, "s_37");
			int numberOfResponses = 0;
			
			boolean hasResponse = false;
			
			boolean hasResponse0 = false;
			if (mtp_corellia_times_contact_condition__defaultCondition (player, npc))
			{
				++numberOfResponses;
				hasResponse = true;
				hasResponse0 = true;
			}
			
			boolean hasResponse1 = false;
			if (mtp_corellia_times_contact_condition__defaultCondition (player, npc))
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
					responses[responseIndex++] = new string_id (c_stringFile, "s_39");
				}
				
				if (hasResponse1)
				{
					responses[responseIndex++] = new string_id (c_stringFile, "s_43");
				}
				
				utils.setScriptVar (player, "conversation.mtp_corellia_times_contact.branchId", 1);
				
				npcStartConversation (player, npc, "mtp_corellia_times_contact", message, responses);
			}
			else
			{
				chat.chat (npc, player, message);
			}
			
			return SCRIPT_CONTINUE;
		}
		
		if (mtp_corellia_times_contact_condition_mealtumpCollectionCompleteQuestActive (player, npc))
		{
			doAnimationAction (npc, "bow");
			
			doAnimationAction (player, "thumb_up");
			
			mtp_corellia_times_contact_action_completeCollectionQuest (player, npc);
			
			string_id message = new string_id (c_stringFile, "s_19");
			chat.chat (npc, player, message);
			
			return SCRIPT_CONTINUE;
		}
		
		if (mtp_corellia_times_contact_condition_isReadyForCollectionQuest (player, npc))
		{
			mtp_corellia_times_contact_action_sendVaniPointerSignal (player, npc);
			
			string_id message = new string_id (c_stringFile, "s_10");
			int numberOfResponses = 0;
			
			boolean hasResponse = false;
			
			boolean hasResponse0 = false;
			if (mtp_corellia_times_contact_condition__defaultCondition (player, npc))
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
					responses[responseIndex++] = new string_id (c_stringFile, "s_12");
				}
				
				utils.setScriptVar (player, "conversation.mtp_corellia_times_contact.branchId", 5);
				
				npcStartConversation (player, npc, "mtp_corellia_times_contact", message, responses);
			}
			else
			{
				chat.chat (npc, player, message);
			}
			
			return SCRIPT_CONTINUE;
		}
		
		if (mtp_corellia_times_contact_condition_hasQuestButNotAllDevices (player, npc))
		{
			
			string_id message = new string_id (c_stringFile, "s_50");
			int numberOfResponses = 0;
			
			boolean hasResponse = false;
			
			boolean hasResponse0 = false;
			if (!mtp_corellia_times_contact_condition_hasBombDeviceStack (player, npc))
			{
				++numberOfResponses;
				hasResponse = true;
				hasResponse0 = true;
			}
			
			boolean hasResponse1 = false;
			if (!mtp_corellia_times_contact_condition_hasFoodDeviceStack (player, npc))
			{
				++numberOfResponses;
				hasResponse = true;
				hasResponse1 = true;
			}
			
			boolean hasResponse2 = false;
			if (!mtp_corellia_times_contact_condition_hasWeaponDeviceStack (player, npc))
			{
				++numberOfResponses;
				hasResponse = true;
				hasResponse2 = true;
			}
			
			boolean hasResponse3 = false;
			if (!mtp_corellia_times_contact_condition_hasMapDeviceStack (player, npc))
			{
				++numberOfResponses;
				hasResponse = true;
				hasResponse3 = true;
			}
			
			boolean hasResponse4 = false;
			if (!mtp_corellia_times_contact_condition_hasContainerDeviceStack (player, npc))
			{
				++numberOfResponses;
				hasResponse = true;
				hasResponse4 = true;
			}
			
			boolean hasResponse5 = false;
			if (!mtp_corellia_times_contact_condition_hasSafeDeviceStack (player, npc))
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
					responses[responseIndex++] = new string_id (c_stringFile, "s_52");
				}
				
				if (hasResponse1)
				{
					responses[responseIndex++] = new string_id (c_stringFile, "s_56");
				}
				
				if (hasResponse2)
				{
					responses[responseIndex++] = new string_id (c_stringFile, "s_60");
				}
				
				if (hasResponse3)
				{
					responses[responseIndex++] = new string_id (c_stringFile, "s_64");
				}
				
				if (hasResponse4)
				{
					responses[responseIndex++] = new string_id (c_stringFile, "s_68");
				}
				
				if (hasResponse5)
				{
					responses[responseIndex++] = new string_id (c_stringFile, "s_72");
				}
				
				utils.setScriptVar (player, "conversation.mtp_corellia_times_contact.branchId", 14);
				
				npcStartConversation (player, npc, "mtp_corellia_times_contact", message, responses);
			}
			else
			{
				chat.chat (npc, player, message);
			}
			
			return SCRIPT_CONTINUE;
		}
		
		if (mtp_corellia_times_contact_condition__defaultCondition (player, npc))
		{
			
			string_id message = new string_id (c_stringFile, "s_76");
			chat.chat (npc, player, message);
			
			return SCRIPT_CONTINUE;
		}
		
		chat.chat (npc, "Error: All conditions for OnStartNpcConversation were false.");
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int OnNpcConversationResponse(obj_id self, String conversationId, obj_id player, string_id response) throws InterruptedException
	{
		if (!conversationId.equals("mtp_corellia_times_contact"))
		{
			return SCRIPT_CONTINUE;
		}
		
		obj_id npc = self;
		
		int branchId = utils.getIntScriptVar (player, "conversation.mtp_corellia_times_contact.branchId");
		
		if (branchId == 1 && mtp_corellia_times_contact_handleBranch1 (player, npc, response) == SCRIPT_CONTINUE)
		{
			return SCRIPT_CONTINUE;
		}
		
		if (branchId == 5 && mtp_corellia_times_contact_handleBranch5 (player, npc, response) == SCRIPT_CONTINUE)
		{
			return SCRIPT_CONTINUE;
		}
		
		if (branchId == 6 && mtp_corellia_times_contact_handleBranch6 (player, npc, response) == SCRIPT_CONTINUE)
		{
			return SCRIPT_CONTINUE;
		}
		
		if (branchId == 7 && mtp_corellia_times_contact_handleBranch7 (player, npc, response) == SCRIPT_CONTINUE)
		{
			return SCRIPT_CONTINUE;
		}
		
		if (branchId == 8 && mtp_corellia_times_contact_handleBranch8 (player, npc, response) == SCRIPT_CONTINUE)
		{
			return SCRIPT_CONTINUE;
		}
		
		if (branchId == 9 && mtp_corellia_times_contact_handleBranch9 (player, npc, response) == SCRIPT_CONTINUE)
		{
			return SCRIPT_CONTINUE;
		}
		
		if (branchId == 10 && mtp_corellia_times_contact_handleBranch10 (player, npc, response) == SCRIPT_CONTINUE)
		{
			return SCRIPT_CONTINUE;
		}
		
		if (branchId == 11 && mtp_corellia_times_contact_handleBranch11 (player, npc, response) == SCRIPT_CONTINUE)
		{
			return SCRIPT_CONTINUE;
		}
		
		if (branchId == 12 && mtp_corellia_times_contact_handleBranch12 (player, npc, response) == SCRIPT_CONTINUE)
		{
			return SCRIPT_CONTINUE;
		}
		
		if (branchId == 14 && mtp_corellia_times_contact_handleBranch14 (player, npc, response) == SCRIPT_CONTINUE)
		{
			return SCRIPT_CONTINUE;
		}
		
		chat.chat (npc, "Error: Fell through all branches and responses for OnNpcConversationResponse.");
		
		utils.removeScriptVar (player, "conversation.mtp_corellia_times_contact.branchId");
		
		return SCRIPT_CONTINUE;
	}
	
}
