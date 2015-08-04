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


public class doctor_griffax_jin extends script.base_script
{
	public doctor_griffax_jin()
	{
	}
	String c_stringFile = "conversation/doctor_griffax_jin";
	
	
	public boolean doctor_griffax_jin_condition__defaultCondition(obj_id player, obj_id npc) throws InterruptedException
	{
		return true;
	}
	
	
	public boolean doctor_griffax_jin_condition_hasTask(obj_id player, obj_id npc) throws InterruptedException
	{
		return groundquests.isTaskActive(player, "outbreak_quest_facility_04", "findJin") || groundquests.isTaskActive(player, "outbreak_quest_facility_04", "defeatJin");
	}
	
	
	public void doctor_griffax_jin_action_transformMonster(obj_id player, obj_id npc) throws InterruptedException
	{
		npcEndConversation (npc);
		string_id barkString = new string_id("theme_park/outbreak/outbreak", "griffax_jin_last_words");
		chat.chat(npc, barkString);
		
		dictionary params = new dictionary();
		params.put("player", player);
		messageTo(npc, "transformIntoMonster", params, 4, false);
		ai_lib.setMood(npc, "npc_dead_02");
	}
	
	
	public int doctor_griffax_jin_handleBranch1(obj_id player, obj_id npc, string_id response) throws InterruptedException
	{
		
		if (response.equals("s_39"))
		{
			
			if (doctor_griffax_jin_condition__defaultCondition (player, npc))
			{
				
				string_id message = new string_id (c_stringFile, "s_40");
				int numberOfResponses = 0;
				
				boolean hasResponse = false;
				
				boolean hasResponse0 = false;
				if (doctor_griffax_jin_condition__defaultCondition (player, npc))
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
						responses[responseIndex++] = new string_id (c_stringFile, "s_41");
					}
					
					utils.setScriptVar (player, "conversation.doctor_griffax_jin.branchId", 2);
					
					npcSpeak (player, message);
					npcSetConversationResponses (player, responses);
				}
				else
				{
					utils.removeScriptVar (player, "conversation.doctor_griffax_jin.branchId");
					
					npcEndConversationWithMessage (player, message);
				}
				
				return SCRIPT_CONTINUE;
			}
			
		}
		
		return SCRIPT_DEFAULT;
	}
	
	
	public int doctor_griffax_jin_handleBranch2(obj_id player, obj_id npc, string_id response) throws InterruptedException
	{
		
		if (response.equals("s_41"))
		{
			
			if (doctor_griffax_jin_condition__defaultCondition (player, npc))
			{
				
				string_id message = new string_id (c_stringFile, "s_42");
				int numberOfResponses = 0;
				
				boolean hasResponse = false;
				
				boolean hasResponse0 = false;
				if (doctor_griffax_jin_condition__defaultCondition (player, npc))
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
						responses[responseIndex++] = new string_id (c_stringFile, "s_43");
					}
					
					utils.setScriptVar (player, "conversation.doctor_griffax_jin.branchId", 3);
					
					npcSpeak (player, message);
					npcSetConversationResponses (player, responses);
				}
				else
				{
					utils.removeScriptVar (player, "conversation.doctor_griffax_jin.branchId");
					
					npcEndConversationWithMessage (player, message);
				}
				
				return SCRIPT_CONTINUE;
			}
			
		}
		
		return SCRIPT_DEFAULT;
	}
	
	
	public int doctor_griffax_jin_handleBranch3(obj_id player, obj_id npc, string_id response) throws InterruptedException
	{
		
		if (response.equals("s_43"))
		{
			
			if (doctor_griffax_jin_condition__defaultCondition (player, npc))
			{
				
				string_id message = new string_id (c_stringFile, "s_44");
				int numberOfResponses = 0;
				
				boolean hasResponse = false;
				
				boolean hasResponse0 = false;
				if (doctor_griffax_jin_condition__defaultCondition (player, npc))
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
						responses[responseIndex++] = new string_id (c_stringFile, "s_45");
					}
					
					utils.setScriptVar (player, "conversation.doctor_griffax_jin.branchId", 4);
					
					npcSpeak (player, message);
					npcSetConversationResponses (player, responses);
				}
				else
				{
					utils.removeScriptVar (player, "conversation.doctor_griffax_jin.branchId");
					
					npcEndConversationWithMessage (player, message);
				}
				
				return SCRIPT_CONTINUE;
			}
			
		}
		
		return SCRIPT_DEFAULT;
	}
	
	
	public int doctor_griffax_jin_handleBranch4(obj_id player, obj_id npc, string_id response) throws InterruptedException
	{
		
		if (response.equals("s_45"))
		{
			
			if (doctor_griffax_jin_condition__defaultCondition (player, npc))
			{
				
				string_id message = new string_id (c_stringFile, "s_47");
				int numberOfResponses = 0;
				
				boolean hasResponse = false;
				
				boolean hasResponse0 = false;
				if (doctor_griffax_jin_condition__defaultCondition (player, npc))
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
						responses[responseIndex++] = new string_id (c_stringFile, "s_49");
					}
					
					utils.setScriptVar (player, "conversation.doctor_griffax_jin.branchId", 5);
					
					npcSpeak (player, message);
					npcSetConversationResponses (player, responses);
				}
				else
				{
					utils.removeScriptVar (player, "conversation.doctor_griffax_jin.branchId");
					
					npcEndConversationWithMessage (player, message);
				}
				
				return SCRIPT_CONTINUE;
			}
			
		}
		
		return SCRIPT_DEFAULT;
	}
	
	
	public int doctor_griffax_jin_handleBranch5(obj_id player, obj_id npc, string_id response) throws InterruptedException
	{
		
		if (response.equals("s_49"))
		{
			
			if (doctor_griffax_jin_condition__defaultCondition (player, npc))
			{
				doctor_griffax_jin_action_transformMonster (player, npc);
				
				string_id message = new string_id (c_stringFile, "s_56");
				utils.removeScriptVar (player, "conversation.doctor_griffax_jin.branchId");
				
				npcEndConversationWithMessage (player, message);
				
				return SCRIPT_CONTINUE;
			}
			
		}
		
		return SCRIPT_DEFAULT;
	}
	
	public static final String MENU_STRING_FILE = "theme_park/outbreak/outbreak";
	public static final string_id SID_NOT_ENOUGH_POWER = new string_id(MENU_STRING_FILE, "not_enough_power");
	public static final string_id SID_MAIN_PWR_ON = new string_id(MENU_STRING_FILE, "main_power_on");
	public static final string_id SID_MAIN_PWR_OFF = new string_id(MENU_STRING_FILE, "main_power_off");
	public static final string_id SID_BKUP_PWR_ON = new string_id(MENU_STRING_FILE, "backup_power_on");
	public static final string_id SID_BKUP_PWR_OFF = new string_id(MENU_STRING_FILE, "backup_power_off");
	public static final string_id SID_DMG_CTRL_SYS_ON = new string_id(MENU_STRING_FILE, "damage_control_info_sys_on");
	public static final string_id SID_DMG_CTRL_SYS_OFF = new string_id(MENU_STRING_FILE, "damage_control_info_sys_off");
	public static final string_id SID_COMM_SYS_ON = new string_id(MENU_STRING_FILE, "internal_communications_sys_on");
	public static final string_id SID_COMM_SYS_OFF = new string_id(MENU_STRING_FILE, "internal_communications_sys_off");
	public static final string_id SID_ACCESS_DENIED = new string_id(MENU_STRING_FILE, "access_denied");
	public static final string_id SID_NO_BKUP_PWR = new string_id(MENU_STRING_FILE, "no_bkup_power");
	
	
	public int OnInitialize(obj_id self) throws InterruptedException
	{
		if ((!isTangible (self)) || (isPlayer (self)))
		{
			detachScript(self, "conversation.doctor_griffax_jin");
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
		detachScript (self, "conversation.doctor_griffax_jin");
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int transformIntoMonster(obj_id self, dictionary params) throws InterruptedException
	{
		if ((params == null) || (params.isEmpty()))
		{
			return SCRIPT_CONTINUE;
		}
		if (!params.containsKey("player"))
		{
			CustomerServiceLog("outbreak_themepark", "doctor_griffax_jin.transformIntoMonster() player param not found.");
			return SCRIPT_CONTINUE;
		}
		obj_id player = params.getObjId("player");
		if (!isValidId(player) || !exists(player))
		{
			CustomerServiceLog("outbreak_themepark", "doctor_griffax_jin.transformIntoMonster() player OID invalid.");
			return SCRIPT_CONTINUE;
		}
		npcEndConversation (self);
		groundquests.sendSignal(player, "hasFoundJin");
		location curLoc = getLocation(self);
		if (curLoc == null)
		{
			return SCRIPT_CONTINUE;
		}
		
		obj_id mob = create.object("outbreak_scientist_griffax_jin_afflicted", curLoc);
		if (!isValidId(mob) || !exists(mob))
		{
			return SCRIPT_CONTINUE;
		}
		
		setObjVar(mob, "owner", player);
		startCombat(mob, player);
		
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
		
		if (doctor_griffax_jin_condition_hasTask (player, npc))
		{
			
			string_id message = new string_id (c_stringFile, "s_38");
			int numberOfResponses = 0;
			
			boolean hasResponse = false;
			
			boolean hasResponse0 = false;
			if (doctor_griffax_jin_condition__defaultCondition (player, npc))
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
					responses[responseIndex++] = new string_id (c_stringFile, "s_39");
				}
				
				utils.setScriptVar (player, "conversation.doctor_griffax_jin.branchId", 1);
				
				npcStartConversation (player, npc, "doctor_griffax_jin", message, responses);
			}
			else
			{
				chat.chat (npc, player, message);
			}
			
			return SCRIPT_CONTINUE;
		}
		
		if (doctor_griffax_jin_condition__defaultCondition (player, npc))
		{
			doAnimationAction (player, "shrug_shoulders");
			
			string_id message = new string_id (c_stringFile, "s_65");
			chat.chat (npc, player, message);
			
			return SCRIPT_CONTINUE;
		}
		
		chat.chat (npc, "Error: All conditions for OnStartNpcConversation were false.");
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int OnNpcConversationResponse(obj_id self, String conversationId, obj_id player, string_id response) throws InterruptedException
	{
		if (!conversationId.equals("doctor_griffax_jin"))
		{
			return SCRIPT_CONTINUE;
		}
		
		obj_id npc = self;
		
		int branchId = utils.getIntScriptVar (player, "conversation.doctor_griffax_jin.branchId");
		
		if (branchId == 1 && doctor_griffax_jin_handleBranch1 (player, npc, response) == SCRIPT_CONTINUE)
		{
			return SCRIPT_CONTINUE;
		}
		
		if (branchId == 2 && doctor_griffax_jin_handleBranch2 (player, npc, response) == SCRIPT_CONTINUE)
		{
			return SCRIPT_CONTINUE;
		}
		
		if (branchId == 3 && doctor_griffax_jin_handleBranch3 (player, npc, response) == SCRIPT_CONTINUE)
		{
			return SCRIPT_CONTINUE;
		}
		
		if (branchId == 4 && doctor_griffax_jin_handleBranch4 (player, npc, response) == SCRIPT_CONTINUE)
		{
			return SCRIPT_CONTINUE;
		}
		
		if (branchId == 5 && doctor_griffax_jin_handleBranch5 (player, npc, response) == SCRIPT_CONTINUE)
		{
			return SCRIPT_CONTINUE;
		}
		
		chat.chat (npc, "Error: Fell through all branches and responses for OnNpcConversationResponse.");
		
		utils.removeScriptVar (player, "conversation.doctor_griffax_jin.branchId");
		
		return SCRIPT_CONTINUE;
	}
	
}
