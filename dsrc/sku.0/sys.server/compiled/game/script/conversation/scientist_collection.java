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
import script.library.utils;


public class scientist_collection extends script.base_script
{
	public scientist_collection()
	{
	}
	String c_stringFile = "conversation/scientist_collection";
	
	
	public boolean scientist_collection_condition__defaultCondition(obj_id player, obj_id npc) throws InterruptedException
	{
		return true;
	}
	
	
	public boolean scientist_collection_condition_isSlotOne(obj_id player, obj_id npc) throws InterruptedException
	{
		faceTo(npc, player);
		return getStringObjVar(npc, "collection.slotName").equals("nyms_question_scientist:icon_nyms_question_scientist_1");
	}
	
	
	public boolean scientist_collection_condition_isSlotTen(obj_id player, obj_id npc) throws InterruptedException
	{
		faceTo(npc, player);
		return getStringObjVar(npc, "collection.slotName").equals("nyms_question_scientist:icon_nyms_question_scientist_10");
	}
	
	
	public boolean scientist_collection_condition_isSlotNine(obj_id player, obj_id npc) throws InterruptedException
	{
		faceTo(npc, player);
		return getStringObjVar(npc, "collection.slotName").equals("nyms_question_scientist:icon_nyms_question_scientist_9");
	}
	
	
	public boolean scientist_collection_condition_isSlotEight(obj_id player, obj_id npc) throws InterruptedException
	{
		faceTo(npc, player);
		return getStringObjVar(npc, "collection.slotName").equals("nyms_question_scientist:icon_nyms_question_scientist_8");
	}
	
	
	public boolean scientist_collection_condition_isSlotSeven(obj_id player, obj_id npc) throws InterruptedException
	{
		faceTo(npc, player);
		return getStringObjVar(npc, "collection.slotName").equals("nyms_question_scientist:icon_nyms_question_scientist_7");
	}
	
	
	public boolean scientist_collection_condition_isSlotSix(obj_id player, obj_id npc) throws InterruptedException
	{
		faceTo(npc, player);
		return getStringObjVar(npc, "collection.slotName").equals("nyms_question_scientist:icon_nyms_question_scientist_6");
	}
	
	
	public boolean scientist_collection_condition_isSlotFive(obj_id player, obj_id npc) throws InterruptedException
	{
		faceTo(npc, player);
		return getStringObjVar(npc, "collection.slotName").equals("nyms_question_scientist:icon_nyms_question_scientist_5");
	}
	
	
	public boolean scientist_collection_condition_isSlotFour(obj_id player, obj_id npc) throws InterruptedException
	{
		faceTo(npc, player);
		return getStringObjVar(npc, "collection.slotName").equals("nyms_question_scientist:icon_nyms_question_scientist_4");
	}
	
	
	public boolean scientist_collection_condition_isSlotThree(obj_id player, obj_id npc) throws InterruptedException
	{
		faceTo(npc, player);
		return getStringObjVar(npc, "collection.slotName").equals("nyms_question_scientist:icon_nyms_question_scientist_3");
	}
	
	
	public boolean scientist_collection_condition_isSlotTwo(obj_id player, obj_id npc) throws InterruptedException
	{
		faceTo(npc, player);
		return getStringObjVar(npc, "collection.slotName").equals("nyms_question_scientist:icon_nyms_question_scientist_2");
	}
	
	
	public boolean scientist_collection_condition_playerHasQuestAndCollection(obj_id player, obj_id npc) throws InterruptedException
	{
		faceTo(npc, player);
		return groundquests.isQuestActive(player, "u16_nym_themepark_question_scientist_collection") && getCollectionSlotValue(player, "nym_question_scientist_activation") > 0;
	}
	
	
	public boolean scientist_collection_condition_ifNothing(obj_id player, obj_id npc) throws InterruptedException
	{
		return (!scientist_collection_condition_isSlotEight(player, npc) && !scientist_collection_condition_isSlotNine(player, npc) && !scientist_collection_condition_isSlotTen(player, npc) && !scientist_collection_condition_isSlotOne(player, npc) && !scientist_collection_condition_isSlotTwo(player, npc) && !scientist_collection_condition_isSlotThree(player, npc) && !scientist_collection_condition_isSlotFour(player, npc) && !scientist_collection_condition_isSlotFive(player, npc) && !scientist_collection_condition_isSlotSix(player, npc) && !scientist_collection_condition_isSlotSeven(player, npc));
	}
	
	
	public boolean scientist_collection_condition_playerAlreadyQuestioned(obj_id player, obj_id npc) throws InterruptedException
	{
		if (!scientist_collection_condition_playerHasQuestAndCollection(player, npc))
		{
			return false;
		}
		
		String baseSlotName = getStringObjVar(npc, "collection.slotName");
		if (baseSlotName == null || baseSlotName.length() <= 0)
		{
			return false;
		}
		
		String[] splitSlotNames = split(baseSlotName, ':');
		if (splitSlotNames == null || splitSlotNames.length < 0)
		{
			return false;
		}
		
		String slotName = splitSlotNames[1];
		if (slotName == null || slotName.length() <= 0)
		{
			return false;
		}
		
		return hasCompletedCollectionSlot(player, slotName);
	}
	
	
	public void scientist_collection_action_awardSlotOne(obj_id player, obj_id npc) throws InterruptedException
	{
		if (!hasCompletedCollectionSlot(player, "icon_nyms_question_scientist_1"))
		{
			modifyCollectionSlotValue(player, "icon_nyms_question_scientist_1", 1);
		}
	}
	
	
	public void scientist_collection_action_awardSlotTwo(obj_id player, obj_id npc) throws InterruptedException
	{
		if (!hasCompletedCollectionSlot(player, "icon_nyms_question_scientist_2"))
		{
			modifyCollectionSlotValue(player, "icon_nyms_question_scientist_2", 1);
		}
	}
	
	
	public void scientist_collection_action_awardSlotThree(obj_id player, obj_id npc) throws InterruptedException
	{
		if (!hasCompletedCollectionSlot(player, "icon_nyms_question_scientist_3"))
		{
			modifyCollectionSlotValue(player, "icon_nyms_question_scientist_3", 1);
		}
	}
	
	
	public void scientist_collection_action_awardSlotFour(obj_id player, obj_id npc) throws InterruptedException
	{
		if (!hasCompletedCollectionSlot(player, "icon_nyms_question_scientist_4"))
		{
			modifyCollectionSlotValue(player, "icon_nyms_question_scientist_4", 1);
		}
	}
	
	
	public void scientist_collection_action_awardSlotFive(obj_id player, obj_id npc) throws InterruptedException
	{
		if (!hasCompletedCollectionSlot(player, "icon_nyms_question_scientist_5"))
		{
			modifyCollectionSlotValue(player, "icon_nyms_question_scientist_5", 1);
		}
	}
	
	
	public void scientist_collection_action_awardSlotSix(obj_id player, obj_id npc) throws InterruptedException
	{
		if (!hasCompletedCollectionSlot(player, "icon_nyms_question_scientist_6"))
		{
			modifyCollectionSlotValue(player, "icon_nyms_question_scientist_6", 1);
		}
	}
	
	
	public void scientist_collection_action_awardSlotSeven(obj_id player, obj_id npc) throws InterruptedException
	{
		if (!hasCompletedCollectionSlot(player, "icon_nyms_question_scientist_7"))
		{
			modifyCollectionSlotValue(player, "icon_nyms_question_scientist_7", 1);
		}
	}
	
	
	public void scientist_collection_action_awardSlotEight(obj_id player, obj_id npc) throws InterruptedException
	{
		if (!hasCompletedCollectionSlot(player, "icon_nyms_question_scientist_8"))
		{
			modifyCollectionSlotValue(player, "icon_nyms_question_scientist_8", 1);
		}
	}
	
	
	public void scientist_collection_action_awardSlotNine(obj_id player, obj_id npc) throws InterruptedException
	{
		if (!hasCompletedCollectionSlot(player, "icon_nyms_question_scientist_9"))
		{
			modifyCollectionSlotValue(player, "icon_nyms_question_scientist_9", 1);
		}
	}
	
	
	public void scientist_collection_action_awardSlotTen(obj_id player, obj_id npc) throws InterruptedException
	{
		if (!hasCompletedCollectionSlot(player, "icon_nyms_question_scientist_10"))
		{
			modifyCollectionSlotValue(player, "icon_nyms_question_scientist_10", 1);
		}
	}
	
	
	public int scientist_collection_handleBranch2(obj_id player, obj_id npc, string_id response) throws InterruptedException
	{
		
		if (response.equals("s_6"))
		{
			doAnimationAction (player, "backhand_threaten");
			
			if (scientist_collection_condition__defaultCondition (player, npc))
			{
				doAnimationAction (npc, "taken_aback");
				
				string_id message = new string_id (c_stringFile, "s_8");
				int numberOfResponses = 0;
				
				boolean hasResponse = false;
				
				boolean hasResponse0 = false;
				if (scientist_collection_condition__defaultCondition (player, npc))
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
					
					utils.setScriptVar (player, "conversation.scientist_collection.branchId", 3);
					
					npcSpeak (player, message);
					npcSetConversationResponses (player, responses);
				}
				else
				{
					utils.removeScriptVar (player, "conversation.scientist_collection.branchId");
					
					npcEndConversationWithMessage (player, message);
				}
				
				return SCRIPT_CONTINUE;
			}
			
		}
		
		return SCRIPT_DEFAULT;
	}
	
	
	public int scientist_collection_handleBranch3(obj_id player, obj_id npc, string_id response) throws InterruptedException
	{
		
		if (response.equals("s_10"))
		{
			doAnimationAction (player, "point_accusingly");
			
			if (scientist_collection_condition__defaultCondition (player, npc))
			{
				doAnimationAction (npc, "scared");
				
				string_id message = new string_id (c_stringFile, "s_12");
				int numberOfResponses = 0;
				
				boolean hasResponse = false;
				
				boolean hasResponse0 = false;
				if (scientist_collection_condition_isSlotOne (player, npc))
				{
					++numberOfResponses;
					hasResponse = true;
					hasResponse0 = true;
				}
				
				boolean hasResponse1 = false;
				if (scientist_collection_condition_isSlotTwo (player, npc))
				{
					++numberOfResponses;
					hasResponse = true;
					hasResponse1 = true;
				}
				
				boolean hasResponse2 = false;
				if (scientist_collection_condition_isSlotThree (player, npc))
				{
					++numberOfResponses;
					hasResponse = true;
					hasResponse2 = true;
				}
				
				boolean hasResponse3 = false;
				if (scientist_collection_condition_isSlotFour (player, npc))
				{
					++numberOfResponses;
					hasResponse = true;
					hasResponse3 = true;
				}
				
				boolean hasResponse4 = false;
				if (scientist_collection_condition_isSlotFive (player, npc))
				{
					++numberOfResponses;
					hasResponse = true;
					hasResponse4 = true;
				}
				
				boolean hasResponse5 = false;
				if (scientist_collection_condition_isSlotSix (player, npc))
				{
					++numberOfResponses;
					hasResponse = true;
					hasResponse5 = true;
				}
				
				boolean hasResponse6 = false;
				if (scientist_collection_condition_isSlotSeven (player, npc))
				{
					++numberOfResponses;
					hasResponse = true;
					hasResponse6 = true;
				}
				
				boolean hasResponse7 = false;
				if (scientist_collection_condition_isSlotEight (player, npc))
				{
					++numberOfResponses;
					hasResponse = true;
					hasResponse7 = true;
				}
				
				boolean hasResponse8 = false;
				if (scientist_collection_condition_isSlotNine (player, npc))
				{
					++numberOfResponses;
					hasResponse = true;
					hasResponse8 = true;
				}
				
				boolean hasResponse9 = false;
				if (scientist_collection_condition_isSlotTen (player, npc))
				{
					++numberOfResponses;
					hasResponse = true;
					hasResponse9 = true;
				}
				
				boolean hasResponse10 = false;
				if (scientist_collection_condition_ifNothing (player, npc))
				{
					++numberOfResponses;
					hasResponse = true;
					hasResponse10 = true;
				}
				
				if (hasResponse)
				{
					int responseIndex = 0;
					string_id responses[] = new string_id[numberOfResponses];
					
					if (hasResponse0)
					{
						responses[responseIndex++] = new string_id (c_stringFile, "s_14");
					}
					
					if (hasResponse1)
					{
						responses[responseIndex++] = new string_id (c_stringFile, "s_17");
					}
					
					if (hasResponse2)
					{
						responses[responseIndex++] = new string_id (c_stringFile, "s_20");
					}
					
					if (hasResponse3)
					{
						responses[responseIndex++] = new string_id (c_stringFile, "s_26");
					}
					
					if (hasResponse4)
					{
						responses[responseIndex++] = new string_id (c_stringFile, "s_30");
					}
					
					if (hasResponse5)
					{
						responses[responseIndex++] = new string_id (c_stringFile, "s_54");
					}
					
					if (hasResponse6)
					{
						responses[responseIndex++] = new string_id (c_stringFile, "s_58");
					}
					
					if (hasResponse7)
					{
						responses[responseIndex++] = new string_id (c_stringFile, "s_62");
					}
					
					if (hasResponse8)
					{
						responses[responseIndex++] = new string_id (c_stringFile, "s_66");
					}
					
					if (hasResponse9)
					{
						responses[responseIndex++] = new string_id (c_stringFile, "s_70");
					}
					
					if (hasResponse10)
					{
						responses[responseIndex++] = new string_id (c_stringFile, "s_73");
					}
					
					utils.setScriptVar (player, "conversation.scientist_collection.branchId", 4);
					
					npcSpeak (player, message);
					npcSetConversationResponses (player, responses);
				}
				else
				{
					utils.removeScriptVar (player, "conversation.scientist_collection.branchId");
					
					npcEndConversationWithMessage (player, message);
				}
				
				return SCRIPT_CONTINUE;
			}
			
		}
		
		return SCRIPT_DEFAULT;
	}
	
	
	public int scientist_collection_handleBranch4(obj_id player, obj_id npc, string_id response) throws InterruptedException
	{
		
		if (response.equals("s_14"))
		{
			
			if (scientist_collection_condition__defaultCondition (player, npc))
			{
				scientist_collection_action_awardSlotOne (player, npc);
				
				string_id message = new string_id (c_stringFile, "s_21");
				utils.removeScriptVar (player, "conversation.scientist_collection.branchId");
				
				npcEndConversationWithMessage (player, message);
				
				return SCRIPT_CONTINUE;
			}
			
		}
		
		if (response.equals("s_17"))
		{
			
			if (scientist_collection_condition__defaultCondition (player, npc))
			{
				scientist_collection_action_awardSlotTwo (player, npc);
				
				string_id message = new string_id (c_stringFile, "s_22");
				utils.removeScriptVar (player, "conversation.scientist_collection.branchId");
				
				npcEndConversationWithMessage (player, message);
				
				return SCRIPT_CONTINUE;
			}
			
		}
		
		if (response.equals("s_20"))
		{
			
			if (scientist_collection_condition__defaultCondition (player, npc))
			{
				scientist_collection_action_awardSlotThree (player, npc);
				
				string_id message = new string_id (c_stringFile, "s_24");
				utils.removeScriptVar (player, "conversation.scientist_collection.branchId");
				
				npcEndConversationWithMessage (player, message);
				
				return SCRIPT_CONTINUE;
			}
			
		}
		
		if (response.equals("s_26"))
		{
			
			if (scientist_collection_condition__defaultCondition (player, npc))
			{
				scientist_collection_action_awardSlotFour (player, npc);
				
				string_id message = new string_id (c_stringFile, "s_28");
				utils.removeScriptVar (player, "conversation.scientist_collection.branchId");
				
				npcEndConversationWithMessage (player, message);
				
				return SCRIPT_CONTINUE;
			}
			
		}
		
		if (response.equals("s_30"))
		{
			
			if (scientist_collection_condition__defaultCondition (player, npc))
			{
				scientist_collection_action_awardSlotFive (player, npc);
				
				string_id message = new string_id (c_stringFile, "s_32");
				utils.removeScriptVar (player, "conversation.scientist_collection.branchId");
				
				npcEndConversationWithMessage (player, message);
				
				return SCRIPT_CONTINUE;
			}
			
		}
		
		if (response.equals("s_54"))
		{
			
			if (scientist_collection_condition__defaultCondition (player, npc))
			{
				scientist_collection_action_awardSlotSix (player, npc);
				
				string_id message = new string_id (c_stringFile, "s_56");
				utils.removeScriptVar (player, "conversation.scientist_collection.branchId");
				
				npcEndConversationWithMessage (player, message);
				
				return SCRIPT_CONTINUE;
			}
			
		}
		
		if (response.equals("s_58"))
		{
			
			if (scientist_collection_condition__defaultCondition (player, npc))
			{
				scientist_collection_action_awardSlotSeven (player, npc);
				
				string_id message = new string_id (c_stringFile, "s_60");
				utils.removeScriptVar (player, "conversation.scientist_collection.branchId");
				
				npcEndConversationWithMessage (player, message);
				
				return SCRIPT_CONTINUE;
			}
			
		}
		
		if (response.equals("s_62"))
		{
			
			if (scientist_collection_condition__defaultCondition (player, npc))
			{
				scientist_collection_action_awardSlotEight (player, npc);
				
				string_id message = new string_id (c_stringFile, "s_64");
				utils.removeScriptVar (player, "conversation.scientist_collection.branchId");
				
				npcEndConversationWithMessage (player, message);
				
				return SCRIPT_CONTINUE;
			}
			
		}
		
		if (response.equals("s_66"))
		{
			
			if (scientist_collection_condition__defaultCondition (player, npc))
			{
				scientist_collection_action_awardSlotNine (player, npc);
				
				string_id message = new string_id (c_stringFile, "s_68");
				utils.removeScriptVar (player, "conversation.scientist_collection.branchId");
				
				npcEndConversationWithMessage (player, message);
				
				return SCRIPT_CONTINUE;
			}
			
		}
		
		if (response.equals("s_70"))
		{
			
			if (scientist_collection_condition__defaultCondition (player, npc))
			{
				scientist_collection_action_awardSlotTen (player, npc);
				
				string_id message = new string_id (c_stringFile, "s_72");
				utils.removeScriptVar (player, "conversation.scientist_collection.branchId");
				
				npcEndConversationWithMessage (player, message);
				
				return SCRIPT_CONTINUE;
			}
			
		}
		
		if (response.equals("s_73"))
		{
			
			if (scientist_collection_condition__defaultCondition (player, npc))
			{
				
				string_id message = new string_id (c_stringFile, "s_74");
				utils.removeScriptVar (player, "conversation.scientist_collection.branchId");
				
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
			detachScript(self, "conversation.scientist_collection");
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
		detachScript (self, "conversation.scientist_collection");
		
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
		
		if (scientist_collection_condition_playerAlreadyQuestioned (player, npc))
		{
			doAnimationAction (npc, "apologize");
			
			string_id message = new string_id (c_stringFile, "s_49");
			chat.chat (npc, player, message);
			
			return SCRIPT_CONTINUE;
		}
		
		if (scientist_collection_condition_playerHasQuestAndCollection (player, npc))
		{
			doAnimationAction (npc, "blame");
			
			string_id message = new string_id (c_stringFile, "s_4");
			int numberOfResponses = 0;
			
			boolean hasResponse = false;
			
			boolean hasResponse0 = false;
			if (scientist_collection_condition__defaultCondition (player, npc))
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
				
				utils.setScriptVar (player, "conversation.scientist_collection.branchId", 2);
				
				npcStartConversation (player, npc, "scientist_collection", message, responses);
			}
			else
			{
				chat.chat (npc, player, message);
			}
			
			return SCRIPT_CONTINUE;
		}
		
		if (scientist_collection_condition__defaultCondition (player, npc))
		{
			
			string_id message = new string_id (c_stringFile, "s_46");
			chat.chat (npc, player, message);
			
			return SCRIPT_CONTINUE;
		}
		
		chat.chat (npc, "Error: All conditions for OnStartNpcConversation were false.");
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int OnNpcConversationResponse(obj_id self, String conversationId, obj_id player, string_id response) throws InterruptedException
	{
		if (!conversationId.equals("scientist_collection"))
		{
			return SCRIPT_CONTINUE;
		}
		
		obj_id npc = self;
		
		int branchId = utils.getIntScriptVar (player, "conversation.scientist_collection.branchId");
		
		if (branchId == 2 && scientist_collection_handleBranch2 (player, npc, response) == SCRIPT_CONTINUE)
		{
			return SCRIPT_CONTINUE;
		}
		
		if (branchId == 3 && scientist_collection_handleBranch3 (player, npc, response) == SCRIPT_CONTINUE)
		{
			return SCRIPT_CONTINUE;
		}
		
		if (branchId == 4 && scientist_collection_handleBranch4 (player, npc, response) == SCRIPT_CONTINUE)
		{
			return SCRIPT_CONTINUE;
		}
		
		chat.chat (npc, "Error: Fell through all branches and responses for OnNpcConversationResponse.");
		
		utils.removeScriptVar (player, "conversation.scientist_collection.branchId");
		
		return SCRIPT_CONTINUE;
	}
	
}
