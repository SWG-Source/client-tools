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
import script.library.utils;


public class kusak extends script.base_script
{
	public kusak()
	{
	}
	String c_stringFile = "conversation/kusak";
	
	
	public boolean kusak_condition__defaultCondition(obj_id player, obj_id npc) throws InterruptedException
	{
		return true;
	}
	
	
	public boolean kusak_condition_hasWeed(obj_id player, obj_id npc) throws InterruptedException
	{
		if (hasObjVar(npc, "alreadyFed"))
		{
			return false;
		}
		
		if (!hasObjVar(npc, "kusak"))
		{
			return false;
		}
		
		int kusakNum = getIntObjVar(npc, "kusak");
		if (kusakNum < 0 || kusakNum > 10)
		{
			return false;
		}
		
		String collectionSlot = "icon_nyms_kusak_feeding_"+kusakNum;
		
		if (hasCompletedCollectionSlot(player, collectionSlot))
		{
			LOG("kusak","hasWeed player has slot");
			return false;
		}
		
		for (int i=0; i < ALL_WEED_TEMPLATES.length; i++)
		{
			testAbortScript();
			obj_id object = utils.getItemByTemplateInInventoryOrEquipped(player, ALL_WEED_TEMPLATES[i]);
			if (isValidId(object))
			{
				return true;
			}
		}
		return false;
	}
	
	
	public boolean kusak_condition_hasWeedType1(obj_id player, obj_id npc) throws InterruptedException
	{
		return isValidId(utils.getItemByTemplateInInventoryOrEquipped(player, ALL_WEED_TEMPLATES[0])) && hasCompletedCollectionSlot(player, "icon_nyms_duty_01");
	}
	
	
	public boolean kusak_condition_hasWeedType10(obj_id player, obj_id npc) throws InterruptedException
	{
		return isValidId(utils.getItemByTemplateInInventoryOrEquipped(player, ALL_WEED_TEMPLATES[9])) && hasCompletedCollectionSlot(player, "icon_nyms_duty_10");
	}
	
	
	public boolean kusak_condition_hasWeedType9(obj_id player, obj_id npc) throws InterruptedException
	{
		return isValidId(utils.getItemByTemplateInInventoryOrEquipped(player, ALL_WEED_TEMPLATES[8])) && hasCompletedCollectionSlot(player, "icon_nyms_duty_09");
	}
	
	
	public boolean kusak_condition_hasWeedType8(obj_id player, obj_id npc) throws InterruptedException
	{
		return isValidId(utils.getItemByTemplateInInventoryOrEquipped(player, ALL_WEED_TEMPLATES[7])) && hasCompletedCollectionSlot(player, "icon_nyms_duty_08");
	}
	
	
	public boolean kusak_condition_hasWeedType7(obj_id player, obj_id npc) throws InterruptedException
	{
		return isValidId(utils.getItemByTemplateInInventoryOrEquipped(player, ALL_WEED_TEMPLATES[6])) && hasCompletedCollectionSlot(player, "icon_nyms_duty_07");
	}
	
	
	public boolean kusak_condition_hasWeedType6(obj_id player, obj_id npc) throws InterruptedException
	{
		return isValidId(utils.getItemByTemplateInInventoryOrEquipped(player, ALL_WEED_TEMPLATES[5])) && hasCompletedCollectionSlot(player, "icon_nyms_duty_06");
	}
	
	
	public boolean kusak_condition_hasWeedType4(obj_id player, obj_id npc) throws InterruptedException
	{
		return isValidId(utils.getItemByTemplateInInventoryOrEquipped(player, ALL_WEED_TEMPLATES[3])) && hasCompletedCollectionSlot(player, "icon_nyms_duty_04");
	}
	
	
	public boolean kusak_condition_hasWeedType3(obj_id player, obj_id npc) throws InterruptedException
	{
		return isValidId(utils.getItemByTemplateInInventoryOrEquipped(player, ALL_WEED_TEMPLATES[2])) && hasCompletedCollectionSlot(player, "icon_nyms_duty_03");
	}
	
	
	public boolean kusak_condition_hasWeedType2(obj_id player, obj_id npc) throws InterruptedException
	{
		return isValidId(utils.getItemByTemplateInInventoryOrEquipped(player, ALL_WEED_TEMPLATES[1])) && hasCompletedCollectionSlot(player, "icon_nyms_duty_02");
	}
	
	
	public boolean kusak_condition_hasWeedType5(obj_id player, obj_id npc) throws InterruptedException
	{
		return isValidId(utils.getItemByTemplateInInventoryOrEquipped(player, ALL_WEED_TEMPLATES[4])) && hasCompletedCollectionSlot(player, "icon_nyms_duty_05");
	}
	
	
	public void kusak_action_validateWeedOne(obj_id player, obj_id npc) throws InterruptedException
	{
		LOG("kusak","validateWeedOne init");
		if (!hasObjVar(npc, "weed_type"))
		{
			LOG("kusak","validateWeedOne no var");
			doAnimationAction(npc, "trick_1");
			return;
		}
		
		int weedType = getIntObjVar(npc, "weed_type");
		if (weedType != WEEDTYPE01)
		{
			LOG("kusak","validateWeedOne type not "+WEEDTYPE01);
			doAnimationAction(npc, "ashamed");
			return;
		}
		
		setObjVar(npc, "weedObj", utils.getItemByTemplateInInventoryOrEquipped(player, ALL_WEED_TEMPLATES[0]));
		
		kusak_action_awardPlayer(player, npc);
	}
	
	
	public void kusak_action_validateWeedTen(obj_id player, obj_id npc) throws InterruptedException
	{
		LOG("kusak","validateWeedTen init");
		if (!hasObjVar(npc, "weed_type"))
		{
			LOG("kusak","validateWeedTen no var");
			doAnimationAction(npc, "trick_1");
			return;
		}
		
		int weedType = getIntObjVar(npc, "weed_type");
		if (weedType != WEEDTYPE10)
		{
			LOG("kusak","validateWeedTen type not "+WEEDTYPE10);
			doAnimationAction(npc, "ashamed");
			return;
		}
		setObjVar(npc, "weedObj", utils.getItemByTemplateInInventoryOrEquipped(player, ALL_WEED_TEMPLATES[9]));
		kusak_action_awardPlayer(player, npc);
	}
	
	
	public void kusak_action_validateWeedNine(obj_id player, obj_id npc) throws InterruptedException
	{
		LOG("kusak","validateWeedNine init");
		if (!hasObjVar(npc, "weed_type"))
		{
			LOG("kusak","validateWeedNine no var");
			doAnimationAction(npc, "trick_1");
			return;
		}
		
		int weedType = getIntObjVar(npc, "weed_type");
		if (weedType != WEEDTYPE09)
		{
			LOG("kusak","validateWeedNine type not "+WEEDTYPE09);
			doAnimationAction(npc, "ashamed");
			return;
		}
		setObjVar(npc, "weedObj", utils.getItemByTemplateInInventoryOrEquipped(player, ALL_WEED_TEMPLATES[8]));
		kusak_action_awardPlayer(player, npc);
	}
	
	
	public void kusak_action_validateWeedEight(obj_id player, obj_id npc) throws InterruptedException
	{
		LOG("kusak","validateWeedEight init");
		if (!hasObjVar(npc, "weed_type"))
		{
			LOG("kusak","validateWeedEight no var");
			doAnimationAction(npc, "trick_1");
			return;
		}
		
		int weedType = getIntObjVar(npc, "weed_type");
		if (weedType != WEEDTYPE08)
		{
			LOG("kusak","validateWeedEight type not "+WEEDTYPE08);
			doAnimationAction(npc, "ashamed");
			return;
		}
		setObjVar(npc, "weedObj", utils.getItemByTemplateInInventoryOrEquipped(player, ALL_WEED_TEMPLATES[7]));
		kusak_action_awardPlayer(player, npc);
	}
	
	
	public void kusak_action_validateWeedSeven(obj_id player, obj_id npc) throws InterruptedException
	{
		LOG("kusak","validateWeedSeven init");
		if (!hasObjVar(npc, "weed_type"))
		{
			LOG("kusak","validateWeedSeven no var");
			doAnimationAction(npc, "trick_1");
			return;
		}
		
		int weedType = getIntObjVar(npc, "weed_type");
		if (weedType != WEEDTYPE07)
		{
			LOG("kusak","validateWeedSeven type not "+WEEDTYPE07);
			doAnimationAction(npc, "ashamed");
			return;
		}
		setObjVar(npc, "weedObj", utils.getItemByTemplateInInventoryOrEquipped(player, ALL_WEED_TEMPLATES[6]));
		kusak_action_awardPlayer(player, npc);
	}
	
	
	public void kusak_action_validateWeedSix(obj_id player, obj_id npc) throws InterruptedException
	{
		LOG("kusak","validateWeedSix init");
		if (!hasObjVar(npc, "weed_type"))
		{
			LOG("kusak","validateWeedSix no var");
			doAnimationAction(npc, "trick_1");
			return;
		}
		
		int weedType = getIntObjVar(npc, "weed_type");
		if (weedType != WEEDTYPE06)
		{
			LOG("kusak","validateWeedSix type not "+WEEDTYPE06);
			doAnimationAction(npc, "ashamed");
			return;
		}
		setObjVar(npc, "weedObj", utils.getItemByTemplateInInventoryOrEquipped(player, ALL_WEED_TEMPLATES[5]));
		kusak_action_awardPlayer(player, npc);
	}
	
	
	public void kusak_action_validateWeedFive(obj_id player, obj_id npc) throws InterruptedException
	{
		LOG("kusak","validateWeedFive init");
		if (!hasObjVar(npc, "weed_type"))
		{
			LOG("kusak","validateWeedFive no var");
			doAnimationAction(npc, "trick_1");
			return;
		}
		
		int weedType = getIntObjVar(npc, "weed_type");
		if (weedType != WEEDTYPE05)
		{
			LOG("kusak","validateWeedFive type not "+WEEDTYPE05);
			doAnimationAction(npc, "ashamed");
			return;
		}
		setObjVar(npc, "weedObj", utils.getItemByTemplateInInventoryOrEquipped(player, ALL_WEED_TEMPLATES[4]));
		kusak_action_awardPlayer(player, npc);
	}
	
	
	public void kusak_action_validateWeedFour(obj_id player, obj_id npc) throws InterruptedException
	{
		LOG("kusak","validateWeedFour init");
		if (!hasObjVar(npc, "weed_type"))
		{
			LOG("kusak","validateWeedFour no var");
			doAnimationAction(npc, "trick_1");
			return;
		}
		
		int weedType = getIntObjVar(npc, "weed_type");
		if (weedType != WEEDTYPE04)
		{
			LOG("kusak","validateWeedFour type not "+WEEDTYPE04);
			doAnimationAction(npc, "ashamed");
			return;
		}
		setObjVar(npc, "weedObj", utils.getItemByTemplateInInventoryOrEquipped(player, ALL_WEED_TEMPLATES[3]));
		kusak_action_awardPlayer(player, npc);
	}
	
	
	public void kusak_action_validateWeedThree(obj_id player, obj_id npc) throws InterruptedException
	{
		LOG("kusak","validateWeedThree init");
		if (!hasObjVar(npc, "weed_type"))
		{
			LOG("kusak","validateWeedThree no var");
			doAnimationAction(npc, "trick_1");
			return;
		}
		
		int weedType = getIntObjVar(npc, "weed_type");
		if (weedType != WEEDTYPE03)
		{
			LOG("kusak","validateWeedThree type not "+WEEDTYPE03);
			doAnimationAction(npc, "ashamed");
			return;
		}
		setObjVar(npc, "weedObj", utils.getItemByTemplateInInventoryOrEquipped(player, ALL_WEED_TEMPLATES[2]));
		kusak_action_awardPlayer(player, npc);
	}
	
	
	public void kusak_action_validateWeedTwo(obj_id player, obj_id npc) throws InterruptedException
	{
		LOG("kusak","validateWeedTwo init");
		if (!hasObjVar(npc, "weed_type"))
		{
			LOG("kusak","validateWeedTwo no var");
			doAnimationAction(npc, "trick_1");
			return;
		}
		
		int weedType = getIntObjVar(npc, "weed_type");
		if (weedType != WEEDTYPE02)
		{
			LOG("kusak","validateWeedTwo type not "+WEEDTYPE02);
			doAnimationAction(npc, "ashamed");
			return;
		}
		setObjVar(npc, "weedObj", utils.getItemByTemplateInInventoryOrEquipped(player, ALL_WEED_TEMPLATES[1]));
		kusak_action_awardPlayer(player, npc);
	}
	
	
	public void kusak_action_awardPlayer(obj_id player, obj_id npc) throws InterruptedException
	{
		if (!hasObjVar(npc, "kusak"))
		{
			return;
		}
		
		int kusakNum = getIntObjVar(npc, "kusak");
		if (kusakNum < 0 || kusakNum > 10)
		{
			return;
		}
		
		if (!hasObjVar(npc, "weedObj"))
		{
			return;
		}
		
		obj_id weedObj = getObjIdObjVar(npc, "weedObj");
		if (!isValidId(weedObj) || !exists(weedObj))
		{
			return;
		}
		
		String collectionSlot = "icon_nyms_kusak_feeding_"+kusakNum;
		
		if (!hasCompletedCollectionSlot(player, collectionSlot))
		{
			if (modifyCollectionSlotValue(player, collectionSlot, 1))
			{
				decrementCount(weedObj);
				String trickName = "trick_"+rand(1,2);
				doAnimationAction(npc, trickName);
				messageTo(npc, "destroySelf", null, 3, false);
				setObjVar(npc, "alreadyFed", 1);
			}
		}
	}
	
	
	public int kusak_handleBranch1(obj_id player, obj_id npc, string_id response) throws InterruptedException
	{
		
		if (response.equals("s_6"))
		{
			
			if (kusak_condition__defaultCondition (player, npc))
			{
				
				string_id message = new string_id (c_stringFile, "s_19");
				int numberOfResponses = 0;
				
				boolean hasResponse = false;
				
				boolean hasResponse0 = false;
				if (kusak_condition_hasWeedType1 (player, npc))
				{
					++numberOfResponses;
					hasResponse = true;
					hasResponse0 = true;
				}
				
				boolean hasResponse1 = false;
				if (kusak_condition_hasWeedType2 (player, npc))
				{
					++numberOfResponses;
					hasResponse = true;
					hasResponse1 = true;
				}
				
				boolean hasResponse2 = false;
				if (kusak_condition_hasWeedType3 (player, npc))
				{
					++numberOfResponses;
					hasResponse = true;
					hasResponse2 = true;
				}
				
				boolean hasResponse3 = false;
				if (kusak_condition_hasWeedType4 (player, npc))
				{
					++numberOfResponses;
					hasResponse = true;
					hasResponse3 = true;
				}
				
				boolean hasResponse4 = false;
				if (kusak_condition_hasWeedType5 (player, npc))
				{
					++numberOfResponses;
					hasResponse = true;
					hasResponse4 = true;
				}
				
				boolean hasResponse5 = false;
				if (kusak_condition_hasWeedType6 (player, npc))
				{
					++numberOfResponses;
					hasResponse = true;
					hasResponse5 = true;
				}
				
				boolean hasResponse6 = false;
				if (kusak_condition_hasWeedType7 (player, npc))
				{
					++numberOfResponses;
					hasResponse = true;
					hasResponse6 = true;
				}
				
				boolean hasResponse7 = false;
				if (kusak_condition_hasWeedType8 (player, npc))
				{
					++numberOfResponses;
					hasResponse = true;
					hasResponse7 = true;
				}
				
				boolean hasResponse8 = false;
				if (kusak_condition_hasWeedType9 (player, npc))
				{
					++numberOfResponses;
					hasResponse = true;
					hasResponse8 = true;
				}
				
				boolean hasResponse9 = false;
				if (kusak_condition_hasWeedType10 (player, npc))
				{
					++numberOfResponses;
					hasResponse = true;
					hasResponse9 = true;
				}
				
				if (hasResponse)
				{
					int responseIndex = 0;
					string_id responses[] = new string_id[numberOfResponses];
					
					if (hasResponse0)
					{
						responses[responseIndex++] = new string_id (c_stringFile, "s_20");
					}
					
					if (hasResponse1)
					{
						responses[responseIndex++] = new string_id (c_stringFile, "s_21");
					}
					
					if (hasResponse2)
					{
						responses[responseIndex++] = new string_id (c_stringFile, "s_22");
					}
					
					if (hasResponse3)
					{
						responses[responseIndex++] = new string_id (c_stringFile, "s_23");
					}
					
					if (hasResponse4)
					{
						responses[responseIndex++] = new string_id (c_stringFile, "s_24");
					}
					
					if (hasResponse5)
					{
						responses[responseIndex++] = new string_id (c_stringFile, "s_25");
					}
					
					if (hasResponse6)
					{
						responses[responseIndex++] = new string_id (c_stringFile, "s_28");
					}
					
					if (hasResponse7)
					{
						responses[responseIndex++] = new string_id (c_stringFile, "s_38");
					}
					
					if (hasResponse8)
					{
						responses[responseIndex++] = new string_id (c_stringFile, "s_42");
					}
					
					if (hasResponse9)
					{
						responses[responseIndex++] = new string_id (c_stringFile, "s_46");
					}
					
					utils.setScriptVar (player, "conversation.kusak.branchId", 2);
					
					npcSpeak (player, message);
					npcSetConversationResponses (player, responses);
				}
				else
				{
					utils.removeScriptVar (player, "conversation.kusak.branchId");
					
					npcEndConversationWithMessage (player, message);
				}
				
				return SCRIPT_CONTINUE;
			}
			
		}
		
		return SCRIPT_DEFAULT;
	}
	
	
	public int kusak_handleBranch2(obj_id player, obj_id npc, string_id response) throws InterruptedException
	{
		
		if (response.equals("s_20"))
		{
			
			if (kusak_condition__defaultCondition (player, npc))
			{
				kusak_action_validateWeedOne (player, npc);
				
				string_id message = new string_id (c_stringFile, "s_30");
				utils.removeScriptVar (player, "conversation.kusak.branchId");
				
				npcEndConversationWithMessage (player, message);
				
				return SCRIPT_CONTINUE;
			}
			
		}
		
		if (response.equals("s_21"))
		{
			
			if (kusak_condition__defaultCondition (player, npc))
			{
				kusak_action_validateWeedTwo (player, npc);
				
				string_id message = new string_id (c_stringFile, "s_31");
				utils.removeScriptVar (player, "conversation.kusak.branchId");
				
				npcEndConversationWithMessage (player, message);
				
				return SCRIPT_CONTINUE;
			}
			
		}
		
		if (response.equals("s_22"))
		{
			
			if (kusak_condition__defaultCondition (player, npc))
			{
				kusak_action_validateWeedThree (player, npc);
				
				string_id message = new string_id (c_stringFile, "s_32");
				utils.removeScriptVar (player, "conversation.kusak.branchId");
				
				npcEndConversationWithMessage (player, message);
				
				return SCRIPT_CONTINUE;
			}
			
		}
		
		if (response.equals("s_23"))
		{
			
			if (kusak_condition__defaultCondition (player, npc))
			{
				kusak_action_validateWeedFour (player, npc);
				
				string_id message = new string_id (c_stringFile, "s_33");
				utils.removeScriptVar (player, "conversation.kusak.branchId");
				
				npcEndConversationWithMessage (player, message);
				
				return SCRIPT_CONTINUE;
			}
			
		}
		
		if (response.equals("s_24"))
		{
			
			if (kusak_condition__defaultCondition (player, npc))
			{
				kusak_action_validateWeedFive (player, npc);
				
				string_id message = new string_id (c_stringFile, "s_34");
				utils.removeScriptVar (player, "conversation.kusak.branchId");
				
				npcEndConversationWithMessage (player, message);
				
				return SCRIPT_CONTINUE;
			}
			
		}
		
		if (response.equals("s_25"))
		{
			
			if (kusak_condition__defaultCondition (player, npc))
			{
				kusak_action_validateWeedSix (player, npc);
				
				string_id message = new string_id (c_stringFile, "s_35");
				utils.removeScriptVar (player, "conversation.kusak.branchId");
				
				npcEndConversationWithMessage (player, message);
				
				return SCRIPT_CONTINUE;
			}
			
		}
		
		if (response.equals("s_28"))
		{
			
			if (kusak_condition__defaultCondition (player, npc))
			{
				kusak_action_validateWeedSeven (player, npc);
				
				string_id message = new string_id (c_stringFile, "s_36");
				utils.removeScriptVar (player, "conversation.kusak.branchId");
				
				npcEndConversationWithMessage (player, message);
				
				return SCRIPT_CONTINUE;
			}
			
		}
		
		if (response.equals("s_38"))
		{
			
			if (kusak_condition__defaultCondition (player, npc))
			{
				kusak_action_validateWeedEight (player, npc);
				
				string_id message = new string_id (c_stringFile, "s_40");
				utils.removeScriptVar (player, "conversation.kusak.branchId");
				
				npcEndConversationWithMessage (player, message);
				
				return SCRIPT_CONTINUE;
			}
			
		}
		
		if (response.equals("s_42"))
		{
			
			if (kusak_condition__defaultCondition (player, npc))
			{
				kusak_action_validateWeedNine (player, npc);
				
				string_id message = new string_id (c_stringFile, "s_44");
				utils.removeScriptVar (player, "conversation.kusak.branchId");
				
				npcEndConversationWithMessage (player, message);
				
				return SCRIPT_CONTINUE;
			}
			
		}
		
		if (response.equals("s_46"))
		{
			
			if (kusak_condition__defaultCondition (player, npc))
			{
				kusak_action_validateWeedTen (player, npc);
				
				string_id message = new string_id (c_stringFile, "s_48");
				utils.removeScriptVar (player, "conversation.kusak.branchId");
				
				npcEndConversationWithMessage (player, message);
				
				return SCRIPT_CONTINUE;
			}
			
		}
		
		return SCRIPT_DEFAULT;
	}
	
	public static final String[] ALL_WEED_TEMPLATES =  
	{
		"object/tangible/collection/col_nym_weed_01.iff",
		"object/tangible/collection/col_nym_weed_02.iff",
		"object/tangible/collection/col_nym_weed_03.iff",
		"object/tangible/collection/col_nym_weed_04.iff",
		"object/tangible/collection/col_nym_weed_05.iff",
		"object/tangible/collection/col_nym_weed_06.iff",
		"object/tangible/collection/col_nym_weed_07.iff",
		"object/tangible/collection/col_nym_weed_08.iff",
		"object/tangible/collection/col_nym_weed_09.iff",
		"object/tangible/collection/col_nym_weed_10.iff"
	};
	
	public static final int WEEDTYPE01 = 1;
	public static final int WEEDTYPE02 = 2;
	public static final int WEEDTYPE03 = 3;
	public static final int WEEDTYPE04 = 4;
	public static final int WEEDTYPE05 = 5;
	public static final int WEEDTYPE06 = 6;
	public static final int WEEDTYPE07 = 7;
	public static final int WEEDTYPE08 = 8;
	public static final int WEEDTYPE09 = 9;
	public static final int WEEDTYPE10 = 10;
	
	
	public int OnInitialize(obj_id self) throws InterruptedException
	{
		if ((!isTangible (self)) || (isPlayer (self)))
		{
			detachScript(self, "conversation.kusak");
		}
		
		setCondition (self, CONDITION_CONVERSABLE);
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int OnAttach(obj_id self) throws InterruptedException
	{
		setCondition (self, CONDITION_CONVERSABLE);
		messageTo(self, "setupWeedVar", null, 3, false);
		
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
		detachScript (self, "conversation.kusak");
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int setupWeedVar(obj_id self, dictionary params) throws InterruptedException
	{
		if (!isValidId(self) || !exists(self))
		{
			return SCRIPT_CONTINUE;
		}
		
		setObjVar(self, "weed_type", rand(1,10));
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int destroySelf(obj_id self, dictionary params) throws InterruptedException
	{
		if (!isValidId(self) || !exists(self))
		{
			return SCRIPT_CONTINUE;
		}
		
		destroyObject(self);
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
		
		if (kusak_condition_hasWeed (player, npc))
		{
			
			string_id message = new string_id (c_stringFile, "s_4");
			int numberOfResponses = 0;
			
			boolean hasResponse = false;
			
			boolean hasResponse0 = false;
			if (kusak_condition__defaultCondition (player, npc))
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
				
				utils.setScriptVar (player, "conversation.kusak.branchId", 1);
				
				npcStartConversation (player, npc, "kusak", message, responses);
			}
			else
			{
				chat.chat (npc, player, message);
			}
			
			return SCRIPT_CONTINUE;
		}
		
		if (kusak_condition__defaultCondition (player, npc))
		{
			
			string_id message = new string_id (c_stringFile, "s_50");
			chat.chat (npc, player, message);
			
			return SCRIPT_CONTINUE;
		}
		
		chat.chat (npc, "Error: All conditions for OnStartNpcConversation were false.");
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int OnNpcConversationResponse(obj_id self, String conversationId, obj_id player, string_id response) throws InterruptedException
	{
		if (!conversationId.equals("kusak"))
		{
			return SCRIPT_CONTINUE;
		}
		
		obj_id npc = self;
		
		int branchId = utils.getIntScriptVar (player, "conversation.kusak.branchId");
		
		if (branchId == 1 && kusak_handleBranch1 (player, npc, response) == SCRIPT_CONTINUE)
		{
			return SCRIPT_CONTINUE;
		}
		
		if (branchId == 2 && kusak_handleBranch2 (player, npc, response) == SCRIPT_CONTINUE)
		{
			return SCRIPT_CONTINUE;
		}
		
		chat.chat (npc, "Error: Fell through all branches and responses for OnNpcConversationResponse.");
		
		utils.removeScriptVar (player, "conversation.kusak.branchId");
		
		return SCRIPT_CONTINUE;
	}
	
}
