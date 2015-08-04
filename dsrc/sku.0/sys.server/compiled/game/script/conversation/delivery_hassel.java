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
import script.library.create;
import script.library.groundquests;
import script.library.utils;


public class delivery_hassel extends script.base_script
{
	public delivery_hassel()
	{
	}
	String c_stringFile = "conversation/delivery_hassel";
	
	
	public boolean delivery_hassel_condition__defaultCondition(obj_id player, obj_id npc) throws InterruptedException
	{
		return true;
	}
	
	
	public boolean delivery_hassel_condition_noAntivirus(obj_id player, obj_id npc) throws InterruptedException
	{
		faceTo(npc, player);
		return (!groundquests.hasCompletedQuest(player, "outbreak_quest_01_imperial") && !groundquests.hasCompletedQuest(player, "outbreak_quest_01_rebel") && !groundquests.hasCompletedQuest(player, "outbreak_quest_01_neutral"));
	}
	
	
	public boolean delivery_hassel_condition_hasNotFoundNextCamp(obj_id player, obj_id npc) throws InterruptedException
	{
		faceTo(npc, player);
		if (!hasObjVar(npc, "questName"))
		{
			return false;
		}
		
		String questName = getStringObjVar(npc, "questName");
		if (questName == null || questName.length() <= 0)
		{
			return false;
		}
		
		String collectionSlot = "";
		if (questName.startsWith("camp_alpha_delivery_0"))
		{
			collectionSlot = "exp_icon_deathtroopers_camp_beta";
		}
		else if (questName.startsWith("camp_beta_delivery_0"))
		{
			collectionSlot = "exp_icon_deathtroopers_camp_gamma";
		}
		else if (questName.startsWith("camp_gamma_delivery_0"))
		{
			collectionSlot = "exp_icon_deathtroopers_camp_delta";
		}
		else if (questName.startsWith("camp_delta_delivery_0"))
		{
			collectionSlot = "exp_icon_deathtroopers_camp_epsilon";
		}
		
		if (collectionSlot == null || collectionSlot.length() <= 0)
		{
			return false;
		}
		
		return !hasCompletedCollectionSlot(player, collectionSlot);
	}
	
	
	public boolean delivery_hassel_condition_neverDelivered(obj_id player, obj_id npc) throws InterruptedException
	{
		faceTo(npc, player);
		if (!hasObjVar(npc, "questName"))
		{
			return false;
		}
		
		String questName = getStringObjVar(npc, "questName");
		if (questName == null || questName.length() <= 0)
		{
			return false;
		}
		return (!groundquests.isQuestActiveOrComplete(player, questName));
	}
	
	
	public boolean delivery_hassel_condition_hasDeliveredBefore(obj_id player, obj_id npc) throws InterruptedException
	{
		faceTo(npc, player);
		if (!hasObjVar(npc, "questName"))
		{
			return false;
		}
		
		String questName = getStringObjVar(npc, "questName");
		if (questName == null || questName.length() <= 0)
		{
			return false;
		}
		
		return groundquests.hasCompletedQuest(player, questName);
	}
	
	
	public boolean delivery_hassel_condition_hasQuestCompleted(obj_id player, obj_id npc) throws InterruptedException
	{
		faceTo(npc, player);
		if (!hasObjVar(npc, "questName"))
		{
			return false;
		}
		
		String questName = getStringObjVar(npc, "questName");
		if (questName == null || questName.length() <= 0)
		{
			return false;
		}
		
		return (groundquests.hasCompletedQuest(player, questName));
	}
	
	
	public void delivery_hassel_action_giveQuest(obj_id player, obj_id npc) throws InterruptedException
	{
		faceTo(npc, player);
		if (!hasObjVar(npc, "questName"))
		{
			CustomerServiceLog("outbreak_themepark", "conversation.delivery_hassel() npc "+npc+" has no questName objvar. Aborting the camp defense functionality.");
			return;
		}
		
		String questName = getStringObjVar(npc, "questName");
		if (questName == null || questName.length() <= 0)
		{
			CustomerServiceLog("outbreak_themepark", "conversation.delivery_hassel() npc "+npc+" has an invalid questName objvar. Aborting the camp defense functionality.");
			return;
		}
		
		if (groundquests.hasCompletedQuest(player, questName))
		{
			groundquests.clearQuest(player, questName);
		}
		
		CustomerServiceLog("outbreak_themepark", "conversation.delivery_hassel() NPC "+npc+" has questName: "+questName+".");
		
		String creatureName = getStringObjVar(npc, "creature_type");
		if (creatureName == null || creatureName.length() <= 0)
		{
			CustomerServiceLog("outbreak_themepark", "conversation.delivery_hassel() NPC "+npc+" has an invalid creatureName. Aborting the camp defense functionality.");
			return;
		}
		
		int combatLevel = getLevel(player);
		if (combatLevel < 0 || combatLevel > 90)
		{
			CustomerServiceLog("outbreak_themepark", "conversation.delivery_hassel() Player "+player+" has a level that is invalid. Player level is: "+combatLevel+". Aborting the camp defense.");
			return;
		}
		
		int maxLevel = getIntObjVar(npc, "maxLevel");
		if (maxLevel < 0 || maxLevel > 90)
		{
			CustomerServiceLog("outbreak_themepark", "conversation.delivery_hassel() NPC "+npc+" has an invalid maxLevel. Aborting the camp defense.");
			return;
		}
		
		if (combatLevel > maxLevel)
		{
			combatLevel = maxLevel;
		}
		
		CustomerServiceLog("outbreak_themepark", "conversation.delivery_hassel() NPC "+npc+" is attempting to give player: "+player+" the quest: "+questName+".");
		
		groundquests.grantQuest(player, questName);
		
		setObjVar(npc, "combatLevel", combatLevel);
		setObjVar(npc, "owner", player);
		setObjVar(npc, create.INITIALIZE_CREATURE_DO_NOT_SCALE_OBJVAR, 1);
		
		dictionary creatureDict = utils.dataTableGetRow("datatables/mob/creatures.iff", creatureName);
		if (creatureDict != null)
		{
			create.initializeCreature (npc, creatureName, creatureDict, combatLevel);
		}
		
		setName(npc, "Delivery Volunteer ("+getPlayerName(player)+")");
		setInvulnerable(npc, false);
		utils.setScriptVar(npc, ai_lib.SCRIPTVAR_CACHED_PATROL_TYPE, 0);
		clearCondition(npc, CONDITION_CONVERSABLE);
		clearCondition(npc, CONDITION_INTERESTING);
		setAttrib(npc, HEALTH, getMaxAttrib( npc, HEALTH ));
		messageTo(npc, "startBeastDeliveryPathing", null, 1, false);
	}
	
	
	public void delivery_hassel_action_clearDeliveryQuest(obj_id player, obj_id npc) throws InterruptedException
	{
		faceTo(npc, player);
		if (!hasObjVar(npc, "questName"))
		{
			return;
		}
		
		String questName = getStringObjVar(npc, "questName");
		if (questName == null || questName.length() <= 0)
		{
			return;
		}
		groundquests.clearQuest(player, questName);
	}
	
	
	public int delivery_hassel_handleBranch3(obj_id player, obj_id npc, string_id response) throws InterruptedException
	{
		
		if (response.equals("s_96"))
		{
			
			if (delivery_hassel_condition__defaultCondition (player, npc))
			{
				doAnimationAction (npc, "explain");
				
				string_id message = new string_id (c_stringFile, "s_97");
				int numberOfResponses = 0;
				
				boolean hasResponse = false;
				
				boolean hasResponse0 = false;
				if (delivery_hassel_condition__defaultCondition (player, npc))
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
						responses[responseIndex++] = new string_id (c_stringFile, "s_13");
					}
					
					utils.setScriptVar (player, "conversation.delivery_hassel.branchId", 4);
					
					npcSpeak (player, message);
					npcSetConversationResponses (player, responses);
				}
				else
				{
					utils.removeScriptVar (player, "conversation.delivery_hassel.branchId");
					
					npcEndConversationWithMessage (player, message);
				}
				
				return SCRIPT_CONTINUE;
			}
			
		}
		
		return SCRIPT_DEFAULT;
	}
	
	
	public int delivery_hassel_handleBranch4(obj_id player, obj_id npc, string_id response) throws InterruptedException
	{
		
		if (response.equals("s_13"))
		{
			
			if (delivery_hassel_condition__defaultCondition (player, npc))
			{
				
				string_id message = new string_id (c_stringFile, "s_17");
				int numberOfResponses = 0;
				
				boolean hasResponse = false;
				
				boolean hasResponse0 = false;
				if (delivery_hassel_condition__defaultCondition (player, npc))
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
						responses[responseIndex++] = new string_id (c_stringFile, "s_19");
					}
					
					utils.setScriptVar (player, "conversation.delivery_hassel.branchId", 5);
					
					npcSpeak (player, message);
					npcSetConversationResponses (player, responses);
				}
				else
				{
					utils.removeScriptVar (player, "conversation.delivery_hassel.branchId");
					
					npcEndConversationWithMessage (player, message);
				}
				
				return SCRIPT_CONTINUE;
			}
			
		}
		
		return SCRIPT_DEFAULT;
	}
	
	
	public int delivery_hassel_handleBranch5(obj_id player, obj_id npc, string_id response) throws InterruptedException
	{
		
		if (response.equals("s_19"))
		{
			
			if (delivery_hassel_condition__defaultCondition (player, npc))
			{
				
				string_id message = new string_id (c_stringFile, "s_21");
				int numberOfResponses = 0;
				
				boolean hasResponse = false;
				
				boolean hasResponse0 = false;
				if (delivery_hassel_condition__defaultCondition (player, npc))
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
					
					utils.setScriptVar (player, "conversation.delivery_hassel.branchId", 6);
					
					npcSpeak (player, message);
					npcSetConversationResponses (player, responses);
				}
				else
				{
					utils.removeScriptVar (player, "conversation.delivery_hassel.branchId");
					
					npcEndConversationWithMessage (player, message);
				}
				
				return SCRIPT_CONTINUE;
			}
			
		}
		
		return SCRIPT_DEFAULT;
	}
	
	
	public int delivery_hassel_handleBranch6(obj_id player, obj_id npc, string_id response) throws InterruptedException
	{
		
		if (response.equals("s_38"))
		{
			
			if (delivery_hassel_condition__defaultCondition (player, npc))
			{
				delivery_hassel_action_giveQuest (player, npc);
				
				string_id message = new string_id (c_stringFile, "s_40");
				utils.removeScriptVar (player, "conversation.delivery_hassel.branchId");
				
				npcEndConversationWithMessage (player, message);
				
				return SCRIPT_CONTINUE;
			}
			
		}
		
		return SCRIPT_DEFAULT;
	}
	
	
	public int delivery_hassel_handleBranch8(obj_id player, obj_id npc, string_id response) throws InterruptedException
	{
		
		if (response.equals("s_33"))
		{
			
			if (delivery_hassel_condition__defaultCondition (player, npc))
			{
				delivery_hassel_action_giveQuest (player, npc);
				
				string_id message = new string_id (c_stringFile, "s_35");
				utils.removeScriptVar (player, "conversation.delivery_hassel.branchId");
				
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
			detachScript(self, "conversation.delivery_hassel");
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
		detachScript (self, "conversation.delivery_hassel");
		
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
		
		if (delivery_hassel_condition_noAntivirus (player, npc))
		{
			
			string_id message = new string_id (c_stringFile, "s_7");
			chat.chat (npc, player, message);
			
			return SCRIPT_CONTINUE;
		}
		
		if (delivery_hassel_condition_hasNotFoundNextCamp (player, npc))
		{
			
			string_id message = new string_id (c_stringFile, "s_10");
			chat.chat (npc, player, message);
			
			return SCRIPT_CONTINUE;
		}
		
		if (delivery_hassel_condition_neverDelivered (player, npc))
		{
			doAnimationAction (npc, "thumb_up");
			
			string_id message = new string_id (c_stringFile, "s_73");
			int numberOfResponses = 0;
			
			boolean hasResponse = false;
			
			boolean hasResponse0 = false;
			if (delivery_hassel_condition__defaultCondition (player, npc))
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
					responses[responseIndex++] = new string_id (c_stringFile, "s_96");
				}
				
				utils.setScriptVar (player, "conversation.delivery_hassel.branchId", 3);
				
				npcStartConversation (player, npc, "delivery_hassel", message, responses);
			}
			else
			{
				chat.chat (npc, player, message);
			}
			
			return SCRIPT_CONTINUE;
		}
		
		if (delivery_hassel_condition_hasDeliveredBefore (player, npc))
		{
			
			string_id message = new string_id (c_stringFile, "s_31");
			int numberOfResponses = 0;
			
			boolean hasResponse = false;
			
			boolean hasResponse0 = false;
			if (delivery_hassel_condition__defaultCondition (player, npc))
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
				
				utils.setScriptVar (player, "conversation.delivery_hassel.branchId", 8);
				
				npcStartConversation (player, npc, "delivery_hassel", message, responses);
			}
			else
			{
				chat.chat (npc, player, message);
			}
			
			return SCRIPT_CONTINUE;
		}
		
		if (delivery_hassel_condition__defaultCondition (player, npc))
		{
			
			string_id message = new string_id (c_stringFile, "s_60");
			chat.chat (npc, player, message);
			
			return SCRIPT_CONTINUE;
		}
		
		chat.chat (npc, "Error: All conditions for OnStartNpcConversation were false.");
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int OnNpcConversationResponse(obj_id self, String conversationId, obj_id player, string_id response) throws InterruptedException
	{
		if (!conversationId.equals("delivery_hassel"))
		{
			return SCRIPT_CONTINUE;
		}
		
		obj_id npc = self;
		
		int branchId = utils.getIntScriptVar (player, "conversation.delivery_hassel.branchId");
		
		if (branchId == 3 && delivery_hassel_handleBranch3 (player, npc, response) == SCRIPT_CONTINUE)
		{
			return SCRIPT_CONTINUE;
		}
		
		if (branchId == 4 && delivery_hassel_handleBranch4 (player, npc, response) == SCRIPT_CONTINUE)
		{
			return SCRIPT_CONTINUE;
		}
		
		if (branchId == 5 && delivery_hassel_handleBranch5 (player, npc, response) == SCRIPT_CONTINUE)
		{
			return SCRIPT_CONTINUE;
		}
		
		if (branchId == 6 && delivery_hassel_handleBranch6 (player, npc, response) == SCRIPT_CONTINUE)
		{
			return SCRIPT_CONTINUE;
		}
		
		if (branchId == 8 && delivery_hassel_handleBranch8 (player, npc, response) == SCRIPT_CONTINUE)
		{
			return SCRIPT_CONTINUE;
		}
		
		chat.chat (npc, "Error: Fell through all branches and responses for OnNpcConversationResponse.");
		
		utils.removeScriptVar (player, "conversation.delivery_hassel.branchId");
		
		return SCRIPT_CONTINUE;
	}
	
}
