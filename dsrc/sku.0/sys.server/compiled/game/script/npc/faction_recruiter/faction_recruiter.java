package script.npc.faction_recruiter;

import script.*;
import script.base_class.*;
import script.combat_engine.*;
import java.util.Arrays;
import java.util.Hashtable;
import java.util.Vector;
import script.base_script;


import script.library.faction_perk;
import script.library.factions;
import script.library.chat;
import script.library.gcw;
import script.library.prose;
import script.library.utils;
import script.library.money;
import script.library.sui;
import script.library.badge;
import script.library.xp;
import java.util.Vector;
import java.util.Arrays;


public class faction_recruiter extends script.base_script
{
	public faction_recruiter()
	{
	}
	public static final String DATATABLE_ALLOWED_XP_TYPES = "datatables/npc/faction_recruiter/allowed_xp_types.iff";
	
	public static final String CONVERSE_FACTION_PERKS = "faction_perks";
	public static final string_id SID_GREETING = new string_id("faction_recruiter", "greeting");
	public static final string_id SID_GREETING_PROSE = new string_id("faction_recruiter", "greeting_prose");
	public static final string_id SID_GREETING_MEMBER = new string_id("faction_recruiter", "greeting_member");
	public static final string_id SID_GREETING_HATE = new string_id("faction_recruiter", "greeting_hate");
	public static final string_id SID_NOT_MEMBER = new string_id("faction_recruiter", "not_member");
	public static final string_id SID_INVALID_RESPONSE = new string_id("faction_recruiter", "invalid_response");
	public static final string_id SID_JOIN_FACTION_CONFIRM = new string_id("faction_recruiter", "join_faction_confirm");
	public static final string_id SID_JOIN_FACTION_DENIED = new string_id("faction_recruiter", "join_faction_denied");
	public static final string_id SID_JOIN_FACTION_PLAYER_NO = new string_id("faction_recruiter", "join_faction_player_no");
	public static final string_id SID_WELCOME_TO_FACTION = new string_id("faction_recruiter", "welcome_to_faction");
	public static final string_id SID_SHOW_OPTIONS = new string_id("faction_recruiter", "show_options");
	public static final string_id SID_RESIGN_FACTION_CONFIRM = new string_id("faction_recruiter", "resign_faction_confirm");
	public static final string_id SID_RESIGN_FACTION_NO = new string_id("faction_recruiter", "resign_faction_no");
	public static final string_id SID_RESIGN_FACTION_YES = new string_id("faction_recruiter", "resign_faction_yes");
	public static final string_id SID_GO_DECLARED_CONFIRM = new string_id("faction_recruiter", "go_declared_confirm");
	public static final string_id SID_GO_COVERT_CONFIRM = new string_id("faction_recruiter", "go_covert_confirm");
	public static final string_id SID_GO_DECLARED_YES = new string_id("faction_recruiter", "go_declared_yes");
	public static final string_id SID_GO_COVERT_YES = new string_id("faction_recruiter", "go_covert_yes");
	public static final string_id SID_SKILL_TRAINING_SELECT = new string_id("faction_recruiter", "skill_training_select");
	public static final string_id SID_SHOW_ITEM_CATEGORIES = new string_id("faction_recruiter", "show_item_categories");
	public static final string_id SID_SHOW_ITEMS = new string_id("faction_recruiter", "show_items");
	public static final string_id SID_NO_ITEMS_AVAILABLE = new string_id("faction_recruiter", "no_items_available");
	public static final string_id SID_NOT_ENOUGH_FOR_PROMOTION = new string_id("faction_recruiter", "not_enough_for_promotion");
	public static final string_id SID_PROMOTION_CONFIRM = new string_id("faction_recruiter", "promotion_confirm");
	public static final string_id SID_PROMOTION_MAX_RANK = new string_id("faction_recruiter", "promotion_max_rank");
	public static final string_id SID_RESIGNATION_RECINDED = new string_id("faction_recruiter", "resignation_recinded");
	public static final string_id SID_ENEMY_PILOT = new string_id("faction_recruiter", "enemy_pilot");
	
	public static final string_id SID_JOIN_FACTION = new string_id("faction_recruiter", "response_join_faction");
	public static final string_id SID_CHANGE_FACTION = new string_id("faction_recruiter", "response_change_faction");
	public static final string_id SID_NOTHING = new string_id("faction_recruiter", "response_nothing");
	public static final string_id SID_YES_TO_JOIN = new string_id("faction_recruiter", "response_yes_to_join");
	public static final string_id SID_NO_TO_JOIN = new string_id("faction_recruiter", "response_no_to_join");
	public static final string_id SID_YES_TO_SEE_OPTIONS = new string_id("faction_recruiter", "response_yes_to_see_options");
	public static final string_id SID_NO_TO_SEE_OPTIONS = new string_id("faction_recruiter", "response_no_to_see_options");
	public static final string_id SID_YES_TO_RESIGN = new string_id("faction_recruiter", "response_yes_to_resign");
	public static final string_id SID_NO_TO_RESIGN = new string_id("faction_recruiter", "response_no_to_resign");
	public static final string_id SID_YES_TO_GO_DECLARED = new string_id("faction_recruiter", "response_yes_to_go_declared");
	public static final string_id SID_NO_TO_GO_DECLARED = new string_id("faction_recruiter", "response_no_to_go_declared");
	public static final string_id SID_YES_TO_GO_COVERT = new string_id("faction_recruiter", "response_yes_to_go_covert");
	public static final string_id SID_NO_TO_GO_COVERT = new string_id("faction_recruiter", "response_no_to_go_covert");
	public static final string_id SID_YES_TO_PROMOTION = new string_id("faction_recruiter", "response_yes_to_promotion");
	public static final string_id SID_NO_TO_PROMOTION = new string_id("faction_recruiter", "response_no_to_promotion");
	public static final string_id SID_PROMOTION_COMPLETE = new string_id("faction_recruiter", "response_promotion_complete");
	
	public static final string_id SID_SELL_SECRETS = new string_id("faction_recruiter", "sell_secrets");
	public static final string_id SID_NO_SECRETS[] =
	{
		new string_id("faction_recruiter", "no_secrets_1"),
		new string_id("faction_recruiter", "no_secrets_2"),
		new string_id("faction_recruiter", "no_secrets_3")
	};
	public static final string_id SID_INTERESTED_SECRETS_REB[]=
	{
		new string_id("faction_recruiter", "interested_secrets_reb_1"),
		new string_id("faction_recruiter", "interested_secrets_reb_2"),
		new string_id("faction_recruiter", "interested_secrets_reb_3")
	};
	public static final string_id SID_INTERESTED_SECRETS_IMP[]=
	{
		new string_id("faction_recruiter", "interested_secrets_imp_1"),
		new string_id("faction_recruiter", "interested_secrets_imp_2"),
		new string_id("faction_recruiter", "interested_secrets_imp_3")
	};
	public static final string_id SID_NEVERMIND = new string_id("faction_recruiter", "nevermind");
	public static final string_id SID_HERE_YOU_GO = new string_id("faction_recruiter", "here_you_go");
	public static final string_id SID_GET_LOST = new string_id("faction_recruiter", "get_lost");
	public static final string_id SID_GOOD_STUFF[] =
	{
		new string_id("faction_recruiter", "good_stuff_1"),
		new string_id("faction_recruiter", "good_stuff_2"),
		new string_id("faction_recruiter", "good_stuff_3")
	};
	public static final string_id SID_BAD_STUFF[] =
	{
		new string_id("faction_recruiter", "bad_stuff_1"),
		new string_id("faction_recruiter", "bad_stuff_2")
	};
	public static final string_id SID_BUY_FACTION = new string_id("faction_recruiter", "buy_faction");
	public static final string_id SID_NO_FACTION_REB = new string_id("faction_recruiter", "no_faction_reb");
	public static final string_id SID_NO_FACTION_IMP = new string_id("faction_recruiter", "no_faction_imp");
	public static final string_id SID_NO_FACTION = new string_id("faction_recruiter", "no_faction");
	public static final string_id SID_YES_FACTION[] =
	{
		new string_id("faction_recruiter", "yes_faction_onek"),
		new string_id("faction_recruiter", "yes_faction_tenk")
	};
	public static final string_id SID_INTERESTED_FACTION_REB = new string_id("faction_recruiter", "interested_faction_reb");
	public static final string_id SID_INTERESTED_FACTION_IMP = new string_id("faction_recruiter", "interested_faction_imp");
	public static final string_id SID_GET_LOST_FACTION = new string_id("faction_recruiter", "get_lost_faction");
	public static final string_id SID_TOO_MUCH_FACTION = new string_id("faction_recruiter", "too_much_faction");
	public static final string_id SID_CHECKING_CREDIT = new string_id("faction_recruiter", "checking_credit");
	public static final string_id SID_BRIBE_FAIL = new string_id("faction_recruiter", "bribe_fail");
	public static final string_id SID_BRIBE_SUCCESS = new string_id("faction_recruiter", "bribe_success");
	
	public static final string_id SID_RESIGN_FROM_FACTION = new string_id("faction_recruiter", "option_resign_from_faction");
	public static final string_id SID_RESCIND_RESIGNATION = new string_id("faction_recruiter", "option_recind_resignation");
	
	public static final string_id SID_GO_DECLARED = new string_id("faction_recruiter", "option_go_declared");
	public static final string_id SID_GO_COVERT = new string_id("faction_recruiter", "option_go_covert");
	public static final string_id SID_SKILL_TRAINING = new string_id("faction_recruiter", "option_skill_training");
	public static final string_id SID_PURCHASE_ITEMS = new string_id("faction_recruiter", "option_purchase_items");
	public static final string_id SID_PURCHASE_FURNITURE = new string_id("faction_recruiter", "option_purchase_furniture");
	public static final string_id SID_PURCHASE_UNIFORMS = new string_id("faction_recruiter", "option_purchase_uniforms");
	public static final string_id SID_PURCHASE_WEAPONS_ARMOR = new string_id("faction_recruiter", "option_purchase_weapons_armor");
	public static final string_id SID_PURCHASE_INSTALLATION = new string_id("faction_recruiter", "option_purchase_installation");
	public static final string_id SID_PURCHASE_SCHEMATICS = new string_id("faction_recruiter", "option_purchase_schematics");
	public static final string_id SID_HIRELINGS = new string_id("faction_recruiter", "option_hirelings");
	public static final string_id SID_FACTION_ORDERS = new string_id("faction_recruiter", "option_faction_orders");
	public static final string_id SID_PROMOTION = new string_id("faction_recruiter", "option_promotion");
	
	public static final string_id PROSE_REWARD = new string_id("faction_recruiter", "prose_reward");
	
	public static final string_id SID_YOU_LACK_FACTION = new string_id("faction_recruiter", "you_lack_faction");
	public static final string_id SID_ALREADY_AT_CAP = new string_id("faction_recruiter", "already_at_cap");
	
	public static final string_id SID_SUI_EXPERIENCE_TRAINING_TITLE = new string_id("faction_recruiter", "sui_experience_training_title");
	public static final string_id SID_SUI_EXPERIENCE_TRAINING_PROMPT = new string_id("faction_recruiter", "sui_experience_training_prompt");
	public static final string_id SID_SUI_EXPERIENCE_TRAINING_HEADER = new string_id("faction_recruiter", "sui_experience_training_header");
	
	public static final String VAR_FACTION = "faction_recruiter.faction";
	public static final String VAR_PROGRESS = "faction_recruiter.progress";
	
	public static final String VAR_GOING_COVERT = "faction_recruiter.going_covert";
	public static final String VAR_AVAILABLE_ITEMS = "faction_recruiter.available_items";
	public static final String VAR_GOING_OVERT_FACTION_ID = "faction_recruiter.factionId";
	
	
	public int OnAttach(obj_id self) throws InterruptedException
	{
		
		if (hasObjVar(self, VAR_FACTION))
		{
			
			if (!hasScript(self, "systems.gcw.gcw_data_updater"))
			{
				attachScript(self, "systems.gcw.gcw_data_updater");
			}
			if (hasScript(self, "conversation.faction_recruiter_general"))
			{
				detachScript(self, "conversation.faction_recruiter_general");
			}
			
			String faction = getStringObjVar(self, VAR_FACTION);
			
			String strScript = "conversation.faction_recruiter_"+toLower(faction);
			if (!hasScript(self, strScript))
			{
				attachScript(self, strScript);
			}
			
			int faction_id = getFactionId(faction);
			
			if (faction_id == 0)
			{
				LOG("LOG_CHANNEL", "faction_recruiter::OnAttach ("+ self + ") -- an NPC with no faction cannot be a recruiter");
				detachScript(self, faction_perk.SCRIPT_FACTION_RECRUITER);
			}
			else if (faction_id != getFactionId(factions.FACTION_REBEL) && faction_id != getFactionId(factions.FACTION_IMPERIAL))
			{
				LOG("LOG_CHANNEL", "faction_recruiter::OnAttach ("+ self + ") -- a faction recruiter must be either of Rebel or Imperial faction.");
				detachScript(self, faction_perk.SCRIPT_FACTION_RECRUITER);
			}
			else
			{
				pvpSetAlignedFaction(self, faction_id);
				pvpMakeDeclared(self);
				
				if (faction_id == getFactionId(factions.FACTION_REBEL))
				{
					setObjVar(self, "item.vendor.vendor_table", "gcw_rebel_recruiter_vendor");
					setObjVar(self, "item.token.type", gcw.GCW_REBEL_TOKEN);
					attachScript(self, "npc.vendor.vendor");
				}
				else if (faction_id == getFactionId(factions.FACTION_IMPERIAL))
				{
					setObjVar(self, "item.vendor.vendor_table", "gcw_imperial_recruiter_vendor");
					setObjVar(self, "item.token.type", gcw.GCW_IMPERIAL_TOKEN);
					attachScript(self, "npc.vendor.vendor");
				}
			}
		}
		requestPreloadCompleteTrigger(self);
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int OnInitialize(obj_id self) throws InterruptedException
	{
		if (!hasScript(self, "systems.gcw.gcw_data_updater"))
		{
			attachScript(self, "systems.gcw.gcw_data_updater");
		}
		if (hasScript(self, "conversation.faction_recruiter_general"))
		{
			detachScript(self, "conversation.faction_recruiter_general");
		}
		String faction = getStringObjVar(self, VAR_FACTION);
		
		if ((!isMob(self)) || (isPlayer(self)))
		{
			detachScript(self, faction_perk.SCRIPT_FACTION_RECRUITER);
		}
		
		String strScript = "conversation.faction_recruiter_"+toLower(faction);
		if (!hasScript(self, strScript))
		{
			attachScript(self, strScript);
		}
		
		int rec_faction_id = pvpGetAlignedFaction(self);
		if (rec_faction_id == 0)
		{
			if (hasObjVar(self, VAR_FACTION))
			{
				int faction_id = getFactionId(faction);
				
				if (faction_id == 0)
				{
					LOG("LOG_CHANNEL", "faction_recruiter::OnInitialize ("+ self + ") -- an NPC with no faction cannot be a recruiter");
					detachScript(self, faction_perk.SCRIPT_FACTION_RECRUITER);
				}
				else if (faction_id != getFactionId(factions.FACTION_REBEL) && faction_id != getFactionId(factions.FACTION_IMPERIAL))
				{
					LOG("LOG_CHANNEL", "faction_recruiter::OnInitialize ("+ self + ") -- a faction recruiter must be either of Rebel or Imperial faction.");
					detachScript(self, faction_perk.SCRIPT_FACTION_RECRUITER);
				}
				else
				{
					pvpSetAlignedFaction(self, faction_id);
					pvpMakeDeclared(self);
				}
			}
			else
			{
				detachScript(self, faction_perk.SCRIPT_FACTION_RECRUITER);
			}
		}
		
		requestPreloadCompleteTrigger(self);
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int OnIncapacitated(obj_id self, obj_id killer) throws InterruptedException
	{
		detachScript(self, faction_perk.SCRIPT_FACTION_RECRUITER);
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int OnObjectMenuRequest(obj_id self, obj_id player, menu_info mi) throws InterruptedException
	{
		int mnu = mi.addRootMenu (menu_info_types.CONVERSE_START, null);
		menu_info_data mdata = mi.getMenuItemById(mnu);
		mdata.setServerNotify(false);
		return SCRIPT_CONTINUE;
	}
	
	
	public int payBribe(obj_id self, dictionary params) throws InterruptedException
	{
		int rec_faction_id = pvpGetAlignedFaction(self);
		String rec_faction = factions.getFactionNameByHashCode(rec_faction_id);
		
		obj_id player = params.getObjId( money.DICT_PLAYER_ID );
		int code = params.getInt( money.DICT_CODE );
		int amt = params.getInt( money.DICT_AMOUNT );
		if (code == money.RET_FAIL)
		{
			chat.chat( self, chat.CHAT_SAY, chat.MOOD_ANGRY, SID_BRIBE_FAIL );
		}
		else
		{
			chat.chat( self, chat.CHAT_SAY, chat.MOOD_HAPPY, SID_BRIBE_SUCCESS );
			
			if (amt == 20000)
			{
				factions.addFactionStanding( player, rec_faction, 250.f );
			}
			else
			{
				factions.addFactionStanding( player, rec_faction, 1250.f );
			}
		}
		
		obj_id topMost = getTopMostContainer(self);
		if (isIdValid(topMost))
		{
			int topGOT = getGameObjectType(topMost);
			if (topGOT == GOT_building_factional)
			{
				money.bankTo(self, topMost, amt);
			}
		}
		return SCRIPT_CONTINUE;
	}
	
	
	public prose_package createProsePackage(string_id sid, obj_id actor, obj_id target, String str, int number) throws InterruptedException
	{
		prose_package pp = new prose_package();
		pp.stringId = sid;
		pp.actor.set(actor);
		pp.target.set(target);
		pp.other.set(str);
		pp.digitInteger = number;
		
		return pp;
	}
	
	
	public int getFactionId(String faction) throws InterruptedException
	{
		if (faction == null)
		{
			return -1;
		}
		
		int faction_num = factions.getFactionNumber(faction);
		if (faction_num == -1)
		{
			return 0;
		}
		
		int faction_id = dataTableGetInt("datatables/faction/faction.iff", faction_num, "pvpFaction");
		
		return faction_id;
	}
	
	
	public String getProgressObjVarName(obj_id player) throws InterruptedException
	{
		if (!isIdValid(player))
		{
			return null;
		}
		
		return VAR_PROGRESS + "." + player.toString();
	}
	
	
	public string_id[] getConversationResponses(obj_id player, obj_id npc, int progress) throws InterruptedException
	{
		LOG("LOG_CHANNEL", "faction_recruiter::getConversationResponses -- "+ progress);
		
		Vector responses = new Vector();
		responses.setSize(0);
		int pvp_type = pvpGetType(player);
		
		obj_id self = getSelf();
		int faction_id = pvpGetAlignedFaction(self);
		String faction = factions.getFactionNameByHashCode(faction_id);
		
		if (pvpGetAlignedFaction(player) == pvpGetAlignedFaction(npc) && hasObjVar(player, factions.VAR_RESIGNING))
		{
			responses = utils.addElement(null, SID_RESCIND_RESIGNATION);
			return (string_id[])responses.toArray(new string_id[0]);
		}
		
		if (pvpGetType( player ) == PVPTYPE_NEUTRAL && progress == 3)
		{
			progress = 1;
		}
		
		switch (progress)
		{
			case 1:
			responses = utils.addElement(responses, SID_JOIN_FACTION);
			break;
			
			case 3:
			
			responses = utils.addElement(responses, SID_PROMOTION);
			
			if (pvp_type == PVPTYPE_COVERT)
			{
				
				responses = utils.addElement(responses, SID_GO_DECLARED);
			}
			else if (pvp_type == PVPTYPE_DECLARED)
			{
				
				if (!hasObjVar(player, VAR_GOING_COVERT))
				{
					responses = utils.addElement(responses, SID_GO_COVERT);
				}
				
				responses = utils.addElement(responses, SID_HIRELINGS);
				
			}
			
			responses = utils.addElement(responses, SID_PURCHASE_ITEMS);
			
			responses = utils.addElement(responses, SID_SKILL_TRAINING);
			
			responses = utils.addElement(responses, SID_RESIGN_FROM_FACTION);
			
			break;
			
			case 4:
			responses = utils.addElement(responses, SID_YES_TO_JOIN);
			responses = utils.addElement(responses, SID_NO_TO_JOIN);
			break;
			
			case 5:
			responses = utils.addElement(responses, SID_YES_TO_SEE_OPTIONS);
			responses = utils.addElement(responses, SID_NO_TO_SEE_OPTIONS);
			break;
			
			case 6:
			responses = utils.addElement(responses, SID_YES_TO_RESIGN);
			responses = utils.addElement(responses, SID_NO_TO_RESIGN);
			break;
			
			case 7:
			responses = utils.addElement(responses, SID_YES_TO_GO_DECLARED);
			responses = utils.addElement(responses, SID_NO_TO_GO_DECLARED);
			break;
			
			case 8:
			responses = utils.addElement(responses, SID_YES_TO_GO_COVERT);
			responses = utils.addElement(responses, SID_NO_TO_GO_COVERT);
			break;
			
			case 9:
			responses = utils.addElement(responses, SID_PURCHASE_FURNITURE);
			responses = utils.addElement(responses, SID_PURCHASE_WEAPONS_ARMOR);
			responses = utils.addElement(responses, SID_PURCHASE_SCHEMATICS);
			
			if (pvp_type == PVPTYPE_DECLARED)
			{
				responses = utils.addElement(responses, SID_PURCHASE_INSTALLATION);
				
				if (!toLower(faction).equals("rebel"))
				{
					responses = utils.addElement(responses, SID_PURCHASE_UNIFORMS);
				}
			}
			
			break;
			
			case 10:
			responses = utils.addElement(responses, SID_YES_TO_PROMOTION);
			responses = utils.addElement(responses, SID_NO_TO_PROMOTION);
			break;
			
			default:
			responses = utils.addElement(responses, SID_NOTHING);
			break;
		}
		
		if (hasSkill( player, "class_smuggler_phase3_novice" ))
		{
			responses = utils.addElement( responses, SID_SELL_SECRETS );
		}
		if (hasSkill( player, "class_smuggler_phase4_novice" ))
		{
			responses = utils.addElement( responses, SID_BUY_FACTION );
		}
		
		return (string_id[])responses.toArray(new string_id[0]);
	}
	
	
	public prose_package[] getProseConversationResponses(obj_id player, obj_id npc, String prose_string, int prose_digit, int progress) throws InterruptedException
	{
		LOG("LOG_CHANNEL", "faction_recruiter::getProseConversationResponses -- "+ progress);
		
		prose_package[] responses = new prose_package[0];
		
		switch(progress)
		{
			case 2:
			int faction_id = pvpGetAlignedFaction(npc);
			String faction = factions.getFactionNameByHashCode(faction_id);
			prose_package pp = prose.getPackage(SID_CHANGE_FACTION, faction);
			responses = assemblePackageArray(responses, pp);
			break;
		}
		
		return responses;
	}
	
	
	public prose_package[] assemblePackageArray(prose_package[] array, prose_package element) throws InterruptedException
	{
		if (element == null)
		{
			return new prose_package[0];
		}
		
		if (array == null)
		{
			return assemblePackageArray(new prose_package[0], element);
		}
		
		Vector vector = new Vector(Arrays.asList(array));
		vector.add(element);
		
		array = new prose_package[vector.size()];
		vector.toArray(array);
		return array;
	}
	
	
	public int[] convertSecondsTime(int time) throws InterruptedException
	{
		if (time < 1)
		{
			return null;
		}
		
		int mod_day = time % 86400;
		int mod_hour = mod_day % 3600;
		
		int days = time / 86400;
		int hours = mod_day / 3600;
		int minutes = mod_hour / 60;
		int seconds = mod_hour % 60;
		
		int[] converted_time =
		{
			days, hours, minutes, seconds
		};
		
		return converted_time;
	}
	
	
	public void invalidResponse(obj_id player) throws InterruptedException
	{
		if (!isIdValid(player))
		{
			LOG("LOG_CHANNEL", "faction_recruiter::isFactionMember -- player is null");
			return;
		}
		
		string_id[] responses = new string_id[0];
		npcSpeak(player, SID_INVALID_RESPONSE);
		npcSetConversationResponses(player, responses);
		
		return;
	}
	
	
	public boolean displayTrainingSUI(obj_id player) throws InterruptedException
	{
		if (!isIdValid(player))
		{
			LOG("LOG_CHANNEL", "faction_recruiter::displayTrainingSUI -- player is null");
			return false;
		}
		
		obj_id self = getSelf();
		
		if (utils.hasScriptVar(self, player + ".training.pid"))
		{
			int oldpid = utils.getIntScriptVar(self, player + ".training.pid");
			sui.closeSUI(player, oldpid);
			utils.removeScriptVarTree(self, player+".training");
		}
		
		String[] xp_types = dataTableGetStringColumn(DATATABLE_ALLOWED_XP_TYPES, "xp_type");
		if (xp_types == null || xp_types.length == 0)
		{
			return false;
		}
		
		Vector names = new Vector();
		names.setSize(0);
		for (int i = 0; i < xp_types.length; i++)
		{
			testAbortScript();
			names = utils.addElement(names, "@exp_n:"+xp_types[i]);
		}
		
		if (names == null || names.size() == 0)
		{
			return false;
		}
		
		int pid = sui.listbox(self, player, "@faction_recruiter:select_skill_type", sui.OK_CANCEL, "@faction_recruiter:faction_training", names, "msgFactionTrainingTypeSelected");
		if (pid > 0)
		{
			utils.setScriptVar(self, player + ".training.pid", pid);
		}
		
		return true;
	}
	
	
	public int msgFactionItemPurchaseSelected(obj_id self, dictionary params) throws InterruptedException
	{
		
		obj_id player = sui.getPlayerId(params);
		if (!sui.outOfRange(self , player, 10.0f, true))
		{
			faction_perk.factionItemPurchased(params);
		}
		return SCRIPT_CONTINUE;
	}
	
	
	public int msgFactionItemRankSelected(obj_id self, dictionary params) throws InterruptedException
	{
		if ((params == null) || (params.isEmpty()))
		{
			
			return SCRIPT_CONTINUE;
		}
		
		obj_id player = sui.getPlayerId(params);
		
		int btn = sui.getIntButtonPressed(params);
		int idx = sui.getListboxSelectedRow(params);
		
		String scriptvar_path = "recruiter.item_rank."+ player;
		String oldPidVar = scriptvar_path + ".pid";
		String ranksVar = scriptvar_path + ".ranks";
		
		if (utils.hasScriptVar(self, oldPidVar))
		{
			utils.removeScriptVar(self, oldPidVar);
		}
		
		if (btn == sui.BP_CANCEL)
		{
			return SCRIPT_CONTINUE;
		}
		
		int rankSelected = pvpGetCurrentGcwRank(player) - idx;
		
		int playerFactionId = pvpGetAlignedFaction(player);
		String playerGcwFaction = factions.getFactionNameByHashCode(playerFactionId);
		faction_perk.displayItemPurchaseSUI(player, rankSelected, playerGcwFaction, self);
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int OnGiveItem(obj_id self, obj_id item, obj_id player) throws InterruptedException
	{
		if (hasScript( item, "systems.encoded_disk.base.message_assembled" ))
		{
			String entryName = getStringObjVar( item, "scenarioName");
			int faction_id = pvpGetAlignedFaction(self);
			String faction = toLower(factions.getFactionNameByHashCode(faction_id));
			
			String entry = "response_"+ entryName + "_"+ faction;
			string_id res = new string_id( "encoded_disk/message_fragment", entry );
			String response = getString( res );
			if (response == null || response.equals(""))
			{
				return SCRIPT_CONTINUE;
			}
			
			chat.chat( self, chat.CHAT_SAY, chat.MOOD_DISCREET, res );
			
			int amt = rand( 500, 1500 );
			dictionary outparams = new dictionary();
			outparams.put( "amt", amt );
			outparams.put( "player", player );
			transferBankCreditsFromNamedAccount( money.ACCT_NPC_LOOT, self, amt, "fromAccountPass", "xferFail", outparams );
			utils.moneyInMetric(self, money.ACCT_NPC_LOOT, amt);
			
			int badgeIndex = getIntObjVar( item, "badgeIndex");
			String badgeName = getCollectionSlotName(badgeIndex);
			if ((badgeIndex > 0) && (badgeName != null) && (badgeName.length() > 0))
			{
				if (!badge.hasBadge( player, badgeName ))
				{
					
					int factionReward = getIntObjVar( item, "factionReward");
					if (factionReward > 0)
					{
						
						factions.awardFactionStanding( player, factions.getFactionNameByHashCode(faction_id), factionReward );
					}
					
					badge.grantBadge( player, badgeName );
					
					logBalance( "eventReward;"+ faction + ";"+ player + ";"+ entryName );
				}
			}
			
			return SCRIPT_OVERRIDE;
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int xferFail(obj_id self, dictionary params) throws InterruptedException
	{
		return SCRIPT_CONTINUE;
	}
	
	
	public int fromAccountPass(obj_id self, dictionary params) throws InterruptedException
	{
		
		int amt = params.getInt( "amt");
		if (amt > 0)
		{
			withdrawCashFromBank( self, amt, "xferPass", "xferFail", params );
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int xferPass(obj_id self, dictionary params) throws InterruptedException
	{
		
		int cash = getCashBalance( self );
		if (cash <= 0)
		{
			return SCRIPT_CONTINUE;
		}
		
		obj_id player = params.getObjId( "player");
		transferCashTo( self, player, cash, "gaveReward", "xferFail", params );
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int gaveReward(obj_id self, dictionary params) throws InterruptedException
	{
		if (params == null)
		{
			return SCRIPT_CONTINUE;
		}
		
		obj_id player = params.getObjId( "player");
		int amt = params.getInt( "amt");
		prose_package pp = prose.getPackage( PROSE_REWARD, self, player, amt );
		sendSystemMessageProse( player, pp );
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int msgFactionTrainingTypeSelected(obj_id self, dictionary params) throws InterruptedException
	{
		obj_id player = sui.getPlayerId(params);
		if (!isIdValid(player))
		{
			return SCRIPT_CONTINUE;
		}
		
		utils.removeScriptVar(self, player + ".training.pid");
		
		int bp = sui.getIntButtonPressed(params);
		if (bp == sui.BP_CANCEL)
		{
			return SCRIPT_CONTINUE;
		}
		
		int idx = sui.getListboxSelectedRow(params);
		if (idx < 0)
		{
			return SCRIPT_CONTINUE;
		}
		
		dictionary row = dataTableGetRow(factions.DATATABLE_ALLOWED_XP_TYPES, idx);
		if (row == null || row.isEmpty())
		{
			sendSystemMessage(player, factions.SID_INVALID_TRAINING_TYPE);
			return SCRIPT_CONTINUE;
		}
		
		String faction = factions.getFaction(self);
		
		int available = factions.getAvailableFactionPoints(player, faction);
		if (available < 1)
		{
			sendSystemMessage(player, SID_YOU_LACK_FACTION);
			return SCRIPT_CONTINUE;
		}
		
		String xp_type = row.getString("xp_type");
		if (xp_type == null || xp_type.equals(""))
		{
			return SCRIPT_CONTINUE;
		}
		
		string_id xp_name = new string_id("exp_n",xp_type);
		if (xp_name == null)
		{
			return SCRIPT_CONTINUE;
		}
		
		int ratioFrom = row.getInt("ratioFrom");
		int ratioTo = row.getInt("ratioTo");
		if (ratioFrom <= 0 || ratioTo <= 0)
		{
			return SCRIPT_CONTINUE;
		}
		
		String title = utils.packStringId(SID_SUI_EXPERIENCE_TRAINING_TITLE);
		prose_package ppSuiExpTrain = prose.getPackage(SID_SUI_EXPERIENCE_TRAINING_PROMPT);
		prose.setTT(ppSuiExpTrain, xp_name);
		prose.setDI(ppSuiExpTrain, ratioFrom);
		if (ratioTo == 1)
		{
			prose.setTO(ppSuiExpTrain, "");
		}
		else
		{
			prose.setTO(ppSuiExpTrain, ""+ ratioTo);
		}
		String prompt = " \0"+ packOutOfBandProsePackage(null, ppSuiExpTrain);
		
		int pid = sui.transfer(self, player, prompt, title, utils.packStringId(SID_SUI_EXPERIENCE_TRAINING_HEADER), available, "Experience", 0, "msgFactionTrainingAmountSelected", ratioFrom, ratioTo);
		if (pid > -1)
		{
			utils.setScriptVar(self, player + ".training.pid", pid);
			utils.setScriptVar(self, player + ".training.idx", idx);
			
			float ratio = ratioFrom/ratioTo;
			utils.setScriptVar(self, player + ".training.ratio", ratio);
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int msgFactionTrainingAmountSelected(obj_id self, dictionary params) throws InterruptedException
	{
		obj_id player = sui.getPlayerId(params);
		if (!isIdValid(player))
		{
			return SCRIPT_CONTINUE;
		}
		
		int idx = utils.getIntScriptVar(self, player + ".training.idx");
		float ratio = utils.getFloatScriptVar(self, player+".training.ratio");
		utils.removeScriptVarTree(self, player + ".training");
		
		int bp = sui.getIntButtonPressed(params);
		if (bp == sui.BP_CANCEL)
		{
			return SCRIPT_CONTINUE;
		}
		
		int amt = sui.getTransferInputTo(params);
		if (amt < 1)
		{
			return SCRIPT_CONTINUE;
		}
		
		String xp_type = dataTableGetString(factions.DATATABLE_ALLOWED_XP_TYPES, idx, "xp_type");
		if (xp_type == null || xp_type.equals(""))
		{
			return SCRIPT_CONTINUE;
		}
		
		String faction = factions.getFaction(self);
		
		int cost = Math.round(amt/ratio);
		int available = factions.getAvailableFactionPoints(player, faction);
		if (cost > available)
		{
			prose_package pp = prose.getPackage(factions.SID_NOT_ENOUGH_STANDING_SPEND, faction, (int)factions.FACTION_RATING_DECLARABLE_MIN);
			sendSystemMessageProse(player, pp);
			return SCRIPT_CONTINUE;
		}
		
		CustomerServiceLog("faction_perk", "("+ player + ")"+ getName(player) + " is attempting to purchase "+ amt + " "+ xp_type + " xp @ cost of "+ cost + " "+ faction + " fp");
		
		dictionary resultData = new dictionary();
		resultData.put("player", player);
		resultData.put("xp_type", xp_type);
		resultData.put("amt", amt);
		resultData.put("ratio", ratio);
		resultData.put("faction", faction);
		if (!xp.grantUnmodifiedExperience(player, xp_type, amt, "msgFactionTrainingAmountSelectedXpResult", resultData))
		{
			CustomerServiceLog("faction_perk", "("+ player + ")"+ getName(player) + " xp purchase aborted! already at xp cap for type: "+ xp_type +"!");
			
			string_id xp_name = new string_id("exp_n",xp_type);
			if (xp_name == null)
			{
				return SCRIPT_CONTINUE;
			}
			prose_package ppAtCap = prose.getPackage(SID_ALREADY_AT_CAP);
			prose.setTO(ppAtCap, xp_name);
			sendSystemMessageProse(player, ppAtCap);
		}
		return SCRIPT_CONTINUE;
	}
	
	
	public int msgFactionTrainingAmountSelectedXpResult(obj_id self, dictionary params) throws InterruptedException
	{
		int granted = params.getInt(xp.GRANT_XP_RESULT_VALUE);
		dictionary data = params.getDictionary(xp.GRANT_XP_RETURN_DATA);
		obj_id player = data.getObjId("player");
		String xp_type = data.getString("xp_type");
		int amt = data.getInt("amt");
		float ratio = data.getFloat("ratio");
		String faction = data.getString("faction");
		
		if (granted <= 0)
		{
			CustomerServiceLog("faction_perk", "("+ player + ")"+ getName(player) + " xp purchase aborted! already at xp cap for type: "+ xp_type +"!");
			
			string_id xp_name = new string_id("exp_n",xp_type);
			if (xp_name == null)
			{
				return SCRIPT_CONTINUE;
			}
			prose_package ppAtCap = prose.getPackage(SID_ALREADY_AT_CAP);
			prose.setTO(ppAtCap, xp_name);
			sendSystemMessageProse(player, ppAtCap);
			return SCRIPT_CONTINUE;
		}
		
		int cost = Math.round(amt/ratio);
		if (granted != amt)
		{
			
			cost = Math.round(granted/ratio);
		}
		
		if (factions.addFactionStanding(player, faction, -cost))
		{
			CustomerServiceLog("faction_perk", "("+ player + ")"+ getName(player) + " fp->xp purchase complete: "+cost+"fp->"+granted+"xp of "+xp_type);
		}
		else
		{
			CustomerServiceLog("faction_perk", "("+ player + ")"+ getName(player) + " fp->xp purchase failed! revoking xp");
			grantExperiencePoints(player, xp_type, -amt);
		}
		return SCRIPT_CONTINUE;
	}
}
