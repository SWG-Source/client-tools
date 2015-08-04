package script.library;

import script.*;
import script.base_class.*;
import script.combat_engine.*;
import java.util.Arrays;
import java.util.Hashtable;
import java.util.Vector;
import script.base_script;


import script.library.chat;
import script.library.skill;
import script.library.prose;
import script.library.luck;
import script.library.static_item;
import script.library.utils;
import script.library.corpse;
import script.library.groundquests;


public class smuggler extends script.base_script
{
	public smuggler()
	{
	}
	public static final String TBL = "datatables/npc/junk_dealer/junk_dealer.iff";
	
	public static final String SCRIPTVAR_JUNK_SUI = "relicdealer.biogenic.sui";
	public static final String SCRIPTVAR_JUNK_IDS = "relicdealer.biogenic.ids";
	
	public static final String SCRIPTVAR_SOLD = "soldAsJunk";
	
	public static final String STF = "loot_dealer";
	
	public static final string_id PROSE_NO_BUY_FENCE = new string_id(STF, "prose_no_buy_smuggler");
	public static final string_id PROSE_NO_BUY_FENCE_ALL = new string_id(STF, "prose_no_buy_all_smuggler");
	public static final string_id PROSE_NO_BUY = new string_id(STF, "prose_no_buy");
	public static final string_id PROSE_NO_BUY_ALL = new string_id(STF, "prose_no_buy_all");
	public static final string_id PROSE_SOLD_JUNK_FENCE = new string_id(STF, "prose_sold_junk_smuggler");
	public static final string_id PROSE_NO_SALE_FENCE = new string_id(STF, "prose_no_sale_smuggler");
	public static final string_id PROSE_SOLD_JUNK = new string_id(STF, "prose_sold_junk");
	public static final string_id PROSE_NO_SALE = new string_id(STF, "prose_no_sale");
	public static final string_id PROSE_PURCHASED_JUNK = new string_id(STF, "prose_buy_back_junk");
	public static final string_id PROSE_NO_BUY_BACK = new string_id(STF, "prose_no_buy_back");
	public static final string_id PROSE_SOLD_ALL_JUNK = new string_id(STF, "prose_sold_all_junk");
	public static final string_id PROSE_NO_ALL_SALE = new string_id(STF, "prose_no_all_sale");
	public static final string_id PROSE_JUNK_SOLD = new string_id(STF, "prose_junk_sold");
	public static final string_id SID_NO_BUY_BACK_ITEMS_FOUND = new string_id(STF, "no_buy_back_items_found");
	public static final string_id PROSE_JUNK_SOLD_FENCE = new string_id(STF, "prose_junk_sold_smuggler");
	public static final string_id PROSE_FENCE_COMM_NOTHING = new string_id(STF, "prose_fence_comm_nothing");
	public static final string_id GENERIC_NO_SALE = new string_id(STF, "generic_no_sale");
	public static final string_id SID_NO_SPACE_IN_INVENTORY = new string_id(STF, "no_space_in_inventory");
	
	public static final string_id MENU_SELL = new string_id("junk_dealer", "mnu_sell_junk");
	public static final string_id MENU_BUY_BACK = new string_id("junk_dealer", "mnu_buy_back_junk");
	public static final string_id MENU_FLAG_NO_SALE = new string_id("junk_dealer", "mnu_flag_no_sale_junk");
	
	public static final String SELL_TITLE = "@"+ STF + ":sell_title";
	public static final String SELL_PROMPT = "@"+ STF + ":sell_prompt";
	public static final String NO_ITEMS_PROMPT = "@"+ STF + ":no_items";
	public static final String BTN_EXAMINE = "@"+ STF + ":examine";
	public static final String BTN_SELL = "@"+ STF + ":btn_sell";
	public static final String BTN_BUY_BACK = "@"+ STF + ":btn_buy_back";
	public static final String BUY_BACK_PROMPT = "@"+ STF + ":buy_back_prompt";
	public static final String BUY_BACK_TITLE = "@"+ STF + ":buy_back_title";
	public static final String SELL_TITLE_FENCE = "@"+ STF + ":sell_title_smuggler";
	public static final String SELL_PROMPT_FENCE = "@"+ STF + ":sell_prompt_smuggler";
	public static final String BTN_SELL_FENCE = "@"+ STF + ":btn_sell_smuggler";
	public static final String BTN_CANCEL_FENCE = "@"+ STF + ":btn_cancel_smuggler";
	public static final String FLAG_NO_SELL_TITLE = "@"+ STF + ":junk_no_sell_title";
	public static final String FLAG_NO_SELL_DESC = "@"+ STF + ":junk_no_sell_desc";
	public static final String FLAG_NO_SELL = "@"+ STF + ":junk_no_sell_button";
	public static final String REFLAG_SELL_TITLE = "@"+ STF + ":junk_reflag_sell_title";
	public static final String REFLAG_SELL_DESC = "@"+ STF + ":junk_reflag_sell_desc";
	public static final String REFLAG_SELL = "@"+ STF + ":junk_reflag_sell_button";
	public static final String FLAG_ITEM_TITLE = "@"+ STF + ":junk_not_found_title";
	public static final String NO_ITEMS_FOUND_PROMPT = "@"+ STF + ":junk_not_found_description";
	
	public static final String BUY_BACK_CONTROL_DEVICE_TEMPLATE = "object/intangible/buy_back/buy_back_control_device.iff";
	
	public static final string_id[] PROSE_FENCE_COMM =
	{
		new string_id(STF, "prose_fence_comm"),
		new string_id(STF, "prose_fence_comm_1"),
		new string_id(STF, "prose_fence_comm_2")
	};
	public static final String[] COMM_SPECIES =
	{
		"object/mobile/dressed_borvos_thief.iff",
		"object/mobile/dressed_corsair_cutthroat_hum_m.iff",
		"object/mobile/dressed_binayre_thief_twk_male_01.iff",
		"object/mobile/dressed_binayre_goon_bith_male_01.iff",
		"object/mobile/dressed_cobral_mugger_human_male_01.iff"
	};
	
	public static final String[] COMM_SOUND =
	{
		"sound/sys_comm_other.snd",
		"sound/sys_comm_generic.snd",
		"sound/sys_comm_imperial.snd",
		"sound/sys_comm_rebel_male.snd"
	};
	
	public static final string_id SID_CONTRABAND_FOUND = new string_id( "smuggler/messages", "contraband_found");
	
	public static final float FENCE_MULTIPLIER_LOW = 0.80f;
	public static final float FENCE_MULTIPLIER = 0.85f;
	public static final float FENCE_MULTIPLIER_HIGH = 0.95f;
	
	public static final int TIER_1_ILLICIT_SELL_VALUE = 1000;
	public static final int TIER_2_ILLICIT_SELL_VALUE = 2500;
	public static final int TIER_3_ILLICIT_SELL_VALUE = 5000;
	public static final int TIER_4_ILLICIT_SELL_VALUE = 7500;
	public static final int TIER_5_ILLICIT_SELL_VALUE = 10000;
	
	public static final int TIER_1_GENERIC_FRONT_COST = 500;
	public static final int TIER_2_GENERIC_FRONT_COST = 1000;
	public static final int TIER_3_GENERIC_FRONT_COST = 2000;
	public static final int TIER_4_GENERIC_FRONT_COST = 3000;
	public static final int TIER_5_GENERIC_FRONT_COST = 5000;
	
	public static final int TIER_4_GENERIC_PVP_FRONT_COST = 13000;
	public static final int TIER_5_GENERIC_PVP_FRONT_COST = 18000;
	
	public static final String BUYBACK_OBJ_TIMESTAMP = "buyback.timeMoved";
	public static final String BUYBACK_OBJ_CREDITS = "buyback.credits";
	public static final String BUYBACK_OBJ_SOLD = "buyback.soldAsJunk";
	public static final String BUYBACK_CONTAINER_VAR = "buyBackContainer";
	public static final String JUNK_DEALKER_NO_SALE_FLAG = "doNotSellFlag";
	
	public static final boolean LOGGING_ON = true;
	public static final String LOGNAME = "junk_log";
	
	
	public static obj_id[] getAllJunkItems(obj_id player) throws InterruptedException
	{
		return getAllJunkItems(player, false);
	}
	
	
	public static obj_id[] getAllJunkItems(obj_id player, boolean getFlaggedNoSale) throws InterruptedException
	{
		
		if (!isIdValid(player))
		{
			return null;
		}
		
		obj_id[] contents = utils.getFilteredPlayerContents(player);
		
		if ((contents != null) && (contents.length > 0))
		{
			Vector junk = new Vector();
			junk.setSize(0);
			
			for (int i = 0; i < contents.length; i++)
			{
				testAbortScript();
				
				if (!validateItemsBeingSold(player, contents[i]))
				{
					continue;
				}
				
				if (!getFlaggedNoSale && hasObjVar(contents[i], JUNK_DEALKER_NO_SALE_FLAG))
				{
					continue;
				}
				
				if (hasObjVar(contents[i], "noTrade") || utils.isEquipped(contents[i]))
				{
					continue;
				}
				
				if (hasObjVar(contents[i], "junkDealer.intPrice"))
				{
					junk = utils.addElement(junk, contents[i]);
				}
				
				else if (static_item.isStaticItem(contents[i]))
				{
					if (static_item.getStaticObjectValue(getStaticItemName(contents[i])) > 0)
					{
						junk = utils.addElement(junk, contents[i]);
					}
				}
				
				else
				{
					if (!isCrafted(contents[i]))
					{
						
						String template = getTemplateName(contents[i]);
						
						if ((template != null) && (!template.equals("")))
						{
							
							if (dataTableGetInt(TBL, template, "price") > 0)
							{
								
								junk = utils.addElement(junk, contents[i]);
							}
						}
					}
				}
			}
			
			if ((junk != null) && (junk.size() > 0))
			{
				
				return utils.toStaticObjIdArray(junk);
			}
		}
		
		return null;
	}
	
	
	public static void showSellJunkSui(obj_id player, obj_id salesman, boolean fence, boolean showCommWindow) throws InterruptedException
	{
		
		if (!isIdValid(player) || !isValidId(salesman))
		{
			return;
		}
		
		if (player != salesman)
		{
			if (utils.outOfRange(salesman, player, 10.0f, true))
			{
				return;
			}
		}
		
		if (utils.hasScriptVar(player, SCRIPTVAR_JUNK_SUI))
		{
			
			int pidClose = utils.getIntScriptVar(player, SCRIPTVAR_JUNK_SUI);
			cleanupSellJunkSui(player);
			
			sui.closeSUI(player, pidClose);
		}
		
		obj_id[] junk = getAllJunkItems(player);
		
		String strSpecies = COMM_SPECIES[rand(0, COMM_SPECIES.length-1)];
		String strSound = COMM_SOUND[rand(0, COMM_SOUND.length-1)];
		
		if (junk != null && junk.length > 0)
		{
			if (showCommWindow && fence)
			{
				prose_package ppfence = prose.getPackage(PROSE_FENCE_COMM[rand(0, PROSE_FENCE_COMM.length-1)]);
				commPlayers(player, strSpecies, strSound, 7f, player, ppfence);
			}
			
			float cashMultiplier = 1.0f;
			if (fence)
			{
				int playerLevel = getLevel(player);
				cashMultiplier = FENCE_MULTIPLIER_LOW;
				
				if (playerLevel > 55)
				{
					cashMultiplier = FENCE_MULTIPLIER;
				}
				if (playerLevel > 80)
				{
					cashMultiplier = FENCE_MULTIPLIER_HIGH;
				}
			}
			
			String[] entries = new String[junk.length];
			
			for (int i = 0; i < junk.length; i++)
			{
				testAbortScript();
				int price = (int)(getPrice(junk[i]));
				if (fence)
				{
					
					price = (int)(getPrice(junk[i]) * cashMultiplier);
				}
				
				String name = getAssignedName(junk[i]);
				if (name.equals("") || name == null)
				{
					name = getString(getNameStringId(junk[i]));
				}
				String entry = "["+ price + "] "+ name;
				
				entries[i] = entry;
				
			}
			
			if (entries != null && entries.length > 0)
			{
				String title = SELL_TITLE;
				String description = SELL_PROMPT;
				String buttonSell = BTN_SELL;
				
				if (fence)
				{
					title = SELL_TITLE_FENCE;
					description = SELL_PROMPT_FENCE;
					buttonSell = BTN_SELL_FENCE;
					utils.setScriptVar(player, "fence", true);
				}
				else
				{
					utils.setScriptVar(player, "fence", false);
				}
				
				int pid = sui.listbox(salesman, player, description, sui.OK_CANCEL_REFRESH, title, entries, "handleSellJunkSui", false, false);
				
				if (pid > -1)
				{
					
					setSUIProperty(pid, sui.LISTBOX_BTN_OK, sui.PROP_TEXT, buttonSell);
					setSUIProperty (pid, sui.LISTBOX_BTN_OTHER, sui.PROP_VISIBLE, "true");
					setSUIProperty (pid, sui.LISTBOX_BTN_OTHER, sui.PROP_TEXT, BTN_EXAMINE);
					subscribeToSUIProperty (pid, "this", sui.LISTBOX_PROP_OTHER_PRESSED);
					showSUIPage(pid);
					
					utils.setScriptVar(player, SCRIPTVAR_JUNK_SUI, pid);
					utils.setBatchScriptVar(player, SCRIPTVAR_JUNK_IDS, junk);
				}
			}
		}
		
		else
		{
			if (fence)
			{
				prose_package ppfence = prose.getPackage(PROSE_FENCE_COMM_NOTHING);
				commPlayers(player, strSpecies, strSound, 7f, player, ppfence);
			}
			else
			{
				int msgPid = sui.msgbox(salesman, player, NO_ITEMS_PROMPT, sui.OK_ONLY, SELL_TITLE, "noHandler");
			}
			cleanupSellJunkSui(player);
		}
	}
	
	
	public static void cleanupSellJunkSui(obj_id player) throws InterruptedException
	{
		utils.removeScriptVar(player, "fence");
		utils.removeScriptVar(player, SCRIPTVAR_JUNK_SUI);
		utils.removeBatchScriptVar(player, SCRIPTVAR_JUNK_IDS);
	}
	
	
	public static void sellJunkItem(obj_id player, obj_id item, boolean fence, boolean reshowSui) throws InterruptedException
	{
		obj_id salesman = getSelf();
		
		if (!isIdValid(player) || !isIdValid(item))
		{
			return;
		}
		
		if (salesman != player)
		{
			blog("smuggler.sellJunkItem() the salesperson is not the player.");
			
			if (utils.outOfRange(salesman, player, 10.0f, true))
			{
				return;
			}
		}
		
		if (!validateItemsBeingSold(player, item))
		{
			prose_package ppSold = prose.getPackage(GENERIC_NO_SALE, item);
			sendSystemMessageProse(player, ppSold);
			return;
		}
		
		if (hasObjVar(item, BUYBACK_OBJ_SOLD))
		{
			prose_package ppSold = prose.getPackage(PROSE_JUNK_SOLD, item);
			sendSystemMessageProse(player, ppSold);
			return;
		}
		
		float cashMultiplier = 1.0f;
		if (fence)
		{
			int playerLevel = getLevel(player);
			cashMultiplier = FENCE_MULTIPLIER_LOW;
			
			if (playerLevel > 55)
			{
				cashMultiplier = FENCE_MULTIPLIER;
			}
			if (playerLevel > 80)
			{
				cashMultiplier = FENCE_MULTIPLIER_HIGH;
			}
		}
		
		int price = (int)(getPrice(item));
		if (fence)
		{
			price = (int)(getPrice(item) * cashMultiplier);
		}
		
		if (price > 0 && !hasObjVar(item, "quest_item"))
		{
			dictionary params = new dictionary();
			params.put("item",item);
			params.put("price",price);
			params.put("reshowSui", reshowSui);
			
			if (luck.isLucky(player, 0.01f))
			{
				float bonus = price * 0.2f;
				if (bonus < 1.0f)
				{
					bonus = 1.0f;
				}
				
				price += bonus;
			}
			
			if (fence)
			{
				blog("smuggler.sellJunkItem() this is a fence player");
				params.put("fence", true);
			}
			else
			{
				blog("smuggler.sellJunkItem() this is NOT a fence player");
				params.put("fence", false);
			}
			
			money.systemPayout(money.ACCT_JUNK_DEALER, player, price, "handleSoldJunk", params);
			
			int calendarTime = getCalendarTime();
			String realTime = getCalendarTimeStringLocal(calendarTime);
			String playerName = getName(player);
			String itemName = getEncodedName(item);
			CustomerServiceLog("Junk_Dealer: ", "smuggler.sellJunkItem() - Player ("+ playerName + " OID: "+ player + ") sold item ("+ itemName + " OID: "+ item + ") for ("+ price + ")credits, at ("+ realTime + ")");
			
			if (!fence && (player != salesman))
			{
				obj_id master = utils.getObjIdScriptVar(salesman, "smugglerMaster");
				
				if (isIdValid(master) && group.inSameGroup(player, master))
				{
					dictionary junkParams = new dictionary();
					params.put("item",item);
					params.put("price",price);
					params.put("reshowSui", false);
					
					int smugglerCut = (int)getSkillStatisticModifier(master, "expertise_junk_dealer_cut");
					
					if (smugglerCut > 0)
					{
						price = (int)((float)price * (float)smugglerCut * 0.01f);
						
						money.systemPayout(money.ACCT_RELIC_DEALER, master, price, "handleSoldJunk", junkParams);
						
						CustomerServiceLog("Junk_Dealer: ", "smuggler.sellJunkItem() - Player ("+ playerName + " OID: "+ player + ") sold item ("+ itemName + " OID: "+ item + ") for ("+ price + ")credits, at ("+ realTime + ")");
						
						int totalProfits = utils.getIntScriptVar(salesman, "totalProfits");
						
						totalProfits += price;
						
						utils.setScriptVar(salesman, "totalProfits", totalProfits);
					}
				}
			}
		}
		
		else
		{
			prose_package ppNoBuy = prose.getPackage(PROSE_NO_BUY, salesman, item);
			if (fence)
			{
				ppNoBuy = prose.getPackage(PROSE_NO_BUY_FENCE, salesman, item);
			}
			sendSystemMessageProse(player, ppNoBuy);
		}
	}
	
	
	public static void sellJunkItem(obj_id player, obj_id item) throws InterruptedException
	{
		sellJunkItem(player, item, false);
	}
	
	
	public static void sellJunkItem(obj_id player, obj_id item, boolean reshowUi) throws InterruptedException
	{
		sellJunkItem(player, item, false, reshowUi);
	}
	
	
	public static int getPrice(obj_id item) throws InterruptedException
	{
		int price = 0;
		
		if (hasObjVar(item, "junkDealer.intPrice"))
		{
			price = getIntObjVar(item, "junkDealer.intPrice");
		}
		else if (static_item.isStaticItem(item))
		{
			price = static_item.getStaticObjectValue(getStaticItemName(item));
		}
		
		else
		{
			String template = getTemplateName(item);
			price = dataTableGetInt(TBL, template, "price");
		}
		int intCount = getCount(item);
		if (intCount > 0)
		{
			price = price * intCount;
		}
		return price;
		
	}
	
	
	public static boolean checkInventory(obj_id player, obj_id npc) throws InterruptedException
	{
		String item = "";
		String itemTemplate = "";
		
		int itemLength = dataTableGetNumRows(TBL);
		
		int x = 0;
		int y = 0;
		
		obj_id inventory = utils.getInventoryContainer(player);
		
		if (isIdValid(inventory))
		{
			obj_id[] contents = utils.getContents(inventory, true);
			if (contents != null)
			{
				for (x = 0; x < contents.length; x++)
				{
					testAbortScript();
					y = 0;
					if (!isCrafted(contents[x]))
					{
						if (static_item.getStaticObjectValue(getStaticItemName(contents[x])) > 0)
						{
							return true;
						}
						
						itemTemplate = getTemplateName(contents[x]);
						while (y < itemLength)
						{
							testAbortScript();
							item = dataTableGetString(TBL, y, "items");
							
							if (itemTemplate.equals(item))
							{
								return true;
							}
							y++;
						}
					}
				}
			}
		}
		return false;
	}
	
	
	public static boolean hasItemsInContainer(obj_id item) throws InterruptedException
	{
		if (getContainerType(item) == 2)
		{
			obj_id[] subContents = utils.getContents(item);
			
			if (subContents.length > 0)
			{
				return true;
			}
		}
		return false;
	}
	
	
	public static boolean validateItemsBeingSold(obj_id player, obj_id item) throws InterruptedException
	{
		if (utils.getContainingPlayer(item) != (player))
		{
			return false;
		}
		
		if (hasItemsInContainer(item))
		{
			return false;
		}
		
		if (utils.isEquipped(item))
		{
			return false;
		}
		
		return true;
	}
	
	
	public static boolean canSlyLie(obj_id playerSmuggler) throws InterruptedException
	{
		location here = getLocation(playerSmuggler);
		obj_id[] objects = getObjectsInRange(here, 25);
		
		for (int i = 0; i < objects.length; i++)
		{
			testAbortScript();
			if (hasObjVar(objects[i], "quest.owner"))
			{
				if (playerSmuggler == getObjIdObjVar(objects[i], "quest.owner"))
				{
					if (utils.hasScriptVar(objects[i], "contrabandCheck"))
					{
						return true;
					}
				}
			}
		}
		
		return false;
	}
	
	
	public static boolean doSlyLie(obj_id playerSmuggler) throws InterruptedException
	{
		location here = getLocation(playerSmuggler);
		obj_id[] objects = getObjectsInRange(here, 25);
		
		for (int i = 0; i < objects.length; i++)
		{
			testAbortScript();
			if (hasObjVar(objects[i], "quest.owner"))
			{
				if (playerSmuggler == getObjIdObjVar(objects[i], "quest.owner"))
				{
					if (utils.hasScriptVar(objects[i], "contrabandCheck"))
					{
						utils.setScriptVar(objects[i], "slyLie", 1);
						return true;
					}
				}
			}
			
		}
		sendSystemMessage(playerSmuggler, new string_id("smuggler/enemy", "sly_lie_failure"));
		
		return false;
	}
	
	
	public static boolean canFastTalk(obj_id playerSmuggler, obj_id target) throws InterruptedException
	{
		if (!hasScript(target, "ai.smuggler_spawn_enemy"))
		{
			
			return false;
		}
		
		if (ai_lib.isInCombat(target))
		{
			obj_id enemyTarget = getHateTarget(target);
			if (enemyTarget != playerSmuggler)
			{
				
				return false;
			}
		}
		
		return true;
	}
	
	
	public static boolean doFastTalk(obj_id playerSmuggler, obj_id target) throws InterruptedException
	{
		if (!hasScript(target, "ai.smuggler_spawn_enemy"))
		{
			
			return false;
		}
		
		if (ai_lib.isInCombat(target))
		{
			obj_id enemyTarget = getHateTarget(target);
			if (enemyTarget != playerSmuggler)
			{
				
				return false;
			}
		}
		
		dictionary params = new dictionary();
		params.put("playerSmuggler", playerSmuggler);
		
		messageTo(target, "fastTalkReaction", params, 0, false);
		
		return true;
	}
	
	
	public static int getSmugglerRank(float underworldFaction) throws InterruptedException
	{
		if (underworldFaction < 1000.0f)
		{
			return 0;
		}
		
		if (underworldFaction < 2000.0f)
		{
			return 1;
		}
		
		if (underworldFaction < 3000.0f)
		{
			return 2;
		}
		
		return 3;
	}
	
	
	public static int getSmuggleTier(float underworldFaction) throws InterruptedException
	{
		if (underworldFaction < 1000.0f)
		{
			return 1;
		}
		
		if (underworldFaction < 1500.0f)
		{
			return 2;
		}
		
		if (underworldFaction < 2000.0f)
		{
			return 3;
		}
		
		if (underworldFaction < 2500.0f)
		{
			return 4;
		}
		
		return 5;
	}
	
	
	public static boolean hasIllicitContraband(obj_id player) throws InterruptedException
	{
		String contrabandName = "item_smuggler_contraband_0";
		obj_id[] contents = getInventoryAndEquipment(player);
		if (contents != null && contents.length > 0)
		{
			for (int i = 0; i < contents.length; i++)
			{
				testAbortScript();
				if (static_item.isStaticItem(contents[i]))
				{
					String itemName = getStaticItemName(contents[i]);
					
					if (itemName.startsWith(contrabandName))
					{
						return true;
					}
				}
			}
		}
		
		return false;
	}
	
	
	public static boolean hasIllicitMissionContraband(obj_id player) throws InterruptedException
	{
		String contrabandName = "item_smuggler_contraband_0";
		obj_id[] contents = getInventoryAndEquipment(player);
		if (contents != null && contents.length > 0)
		{
			for (int i = 0; i < contents.length; i++)
			{
				testAbortScript();
				if (static_item.isStaticItem(contents[i]))
				{
					String itemName = getStaticItemName(contents[i]);
					
					if (itemName.startsWith(contrabandName))
					{
						obj_id[] contrabandToCheck = utils.getAllStaticItemsInPlayerInventory(player, itemName);
						
						int contrabandCount = utils.countOfStackedItemsInArray(contrabandToCheck);
						
						if (contrabandCount >= 5)
						{
							return true;
						}
					}
				}
			}
		}
		
		return false;
	}
	
	
	public static int getIllicitContrabandTier(String itemName) throws InterruptedException
	{
		if (itemName.startsWith("item_smuggler_contraband_01"))
		{
			return 1;
		}
		
		if (itemName.startsWith("item_smuggler_contraband_02"))
		{
			return 2;
		}
		
		if (itemName.startsWith("item_smuggler_contraband_03"))
		{
			return 3;
		}
		
		if (itemName.startsWith("item_smuggler_contraband_04"))
		{
			return 4;
		}
		
		if (itemName.startsWith("item_smuggler_contraband_05"))
		{
			return 5;
		}
		
		return 0;
	}
	
	
	public static String getIllicitMissionContrabandSmugglerTier(obj_id player, int smugglerTier) throws InterruptedException
	{
		
		String contrabandName = "item_smuggler_contraband_0";
		String contrabandFound = "";
		obj_id[] contents = getInventoryAndEquipment(player);
		if (contents != null && contents.length > 0)
		{
			for (int i = 0; i < contents.length; i++)
			{
				testAbortScript();
				if (static_item.isStaticItem(contents[i]))
				{
					String itemName = getStaticItemName(contents[i]);
					if (itemName.startsWith(contrabandName))
					{
						obj_id[] contrabandToCheck = utils.getAllStaticItemsInPlayerInventory(player, itemName);
						
						int contrabandCount = utils.countOfStackedItemsInArray(contrabandToCheck);
						
						if (contrabandCount >= 5)
						{
							int contrabandTier = getIllicitContrabandTier(itemName);
							
							if (contrabandTier == smugglerTier)
							{
								return itemName;
							}
							else
							{
								contrabandFound = itemName;
							}
						}
					}
				}
			}
		}
		
		return contrabandFound;
	}
	
	
	public static boolean hasMissionContrabandForTransaction(obj_id player, int brokerTier) throws InterruptedException
	{
		
		String contrabandName = "item_smuggler_contraband_0";
		obj_id[] contents = getInventoryAndEquipment(player);
		if (contents != null && contents.length > 0)
		{
			for (int i = 0; i < contents.length; i++)
			{
				testAbortScript();
				if (static_item.isStaticItem(contents[i]))
				{
					String itemName = getStaticItemName(contents[i]);
					
					if (itemName.startsWith(contrabandName))
					{
						obj_id[] contrabandToCheck = utils.getAllStaticItemsInPlayerInventory(player, itemName);
						
						int contrabandCount = utils.countOfStackedItemsInArray(contrabandToCheck);
						
						if (contrabandCount >= 5)
						{
							int contrabandTier = getIllicitContrabandTier(itemName);
							if (contrabandTier == brokerTier)
							{
								return true;
							}
						}
					}
				}
			}
		}
		
		return false;
	}
	
	
	public static String getMissionContrabandForTransactionName(obj_id player, int brokerTier) throws InterruptedException
	{
		String contrabandName = "item_smuggler_contraband_0";
		obj_id[] contents = getInventoryAndEquipment(player);
		if (contents != null && contents.length > 0)
		{
			for (int i = 0; i < contents.length; i++)
			{
				testAbortScript();
				if (static_item.isStaticItem(contents[i]))
				{
					String itemName = getStaticItemName(contents[i]);
					
					if (itemName.startsWith(contrabandName))
					{
						obj_id[] contrabandToCheck = utils.getAllStaticItemsInPlayerInventory(player, itemName);
						
						int contrabandCount = utils.countOfStackedItemsInArray(contrabandToCheck);
						
						if (contrabandCount >= 5)
						{
							int contrabandTier = getIllicitContrabandTier(itemName);
							if (contrabandTier == brokerTier)
							{
								return itemName;
							}
						}
					}
				}
			}
			
		}
		
		return "";
	}
	
	
	public static int getMissionCotrabandPointerTier(obj_id player) throws InterruptedException
	{
		
		String contrabandName = "item_smuggler_contraband_0";
		obj_id[] contents = getInventoryAndEquipment(player);
		if (contents != null && contents.length > 0)
		{
			for (int i = 0; i < contents.length; i++)
			{
				testAbortScript();
				if (static_item.isStaticItem(contents[i]))
				{
					String itemName = getStaticItemName(contents[i]);
					
					if (itemName.startsWith(contrabandName))
					{
						obj_id[] contrabandToCheck = utils.getAllStaticItemsInPlayerInventory(player, itemName);
						
						int contrabandCount = utils.countOfStackedItemsInArray(contrabandToCheck);
						
						if (contrabandCount >= 5)
						{
							return(getIllicitContrabandTier(itemName));
						}
					}
				}
			}
		}
		
		return 0;
	}
	
	
	public static void createRandomContrabandTier(obj_id player, int dropTier) throws InterruptedException
	{
		obj_id pInv = utils.getInventoryContainer(player);
		
		int tierItem = rand(1,5);
		
		String contrabandToCreate = "item_smuggler_contraband_0"+dropTier +"_0"+tierItem;
		
		string_id contrabandName = new string_id ("static_item_n", contrabandToCreate);
		
		obj_id newItem = static_item.createNewItemFunction (contrabandToCreate, pInv);
		
		prose_package pp = null;
		
		pp = prose.getPackage(SID_CONTRABAND_FOUND, contrabandName);
		
		sendSystemMessageProse(player, pp);
		
		return;
	}
	
	
	public static void spaceContrabandDropCheck(obj_id player) throws InterruptedException
	{
		float underworldFaction = factions.getFactionStanding( player, "underworld");
		int tier = getSmuggleTier(underworldFaction);
		
		int chance = (12 - tier*2);
		int buffFeelingLucky = 0;
		
		chance += buffFeelingLucky = (int)getSkillStatisticModifier(player, "expertise_increase_smuggler_loot");
		
		if (chance >= rand(1,100))
		{
			createRandomContrabandTier(player, tier);
			if (buffFeelingLucky > 0 && !buff.hasBuff(player, "sm_feeling_lucky") && !buff.hasBuff(player, "sm_feeling_lucky_recourse"))
			{
				
				buff.applyBuff(player, "sm_feeling_lucky");
			}
		}
		return;
	}
	
	
	public static void contrabandDropCheck(obj_id player, obj_id target, int tier, int corpseLevel) throws InterruptedException
	{
		int dropTier = 0;
		
		if (corpseLevel > 0 && corpseLevel < 21)
		{
			dropTier = 1;
		}
		
		if (corpseLevel > 20 && corpseLevel < 41)
		{
			dropTier = 2;
		}
		
		if (corpseLevel > 40 && corpseLevel < 61)
		{
			dropTier = 3;
		}
		
		if (corpseLevel > 60 && corpseLevel < 71)
		{
			dropTier = 4;
		}
		
		if (corpseLevel > 70)
		{
			dropTier = 5;
		}
		
		if (dropTier > tier)
		{
			dropTier = tier;
		}
		
		int chance = (12 - (dropTier*2));
		int buffFeelingLucky = 0;
		
		chance += buffFeelingLucky = (int)getSkillStatisticModifier(player, "expertise_increase_smuggler_loot");
		
		utils.setScriptVar(target, "contrabandChecked", 1);
		if (chance >= rand(1,100))
		{
			createRandomContrabandTier(player, dropTier);
			
			if (buffFeelingLucky > 0 && !buff.hasBuff(player, "sm_feeling_lucky") && !buff.hasBuff(player, "sm_feeling_lucky_recourse"))
			{
				
				buff.applyBuff(player, "sm_feeling_lucky");
			}
			return;
		}
		sendSystemMessage(player, new string_id("smuggler/messages", "no_contraband_found"));
		return;
	}
	
	
	public static void inspectCorpseForContraband(obj_id player, obj_id target) throws InterruptedException
	{
		if (!isIdValid(target))
		{
			return;
		}
		
		if (!isMob(target))
		{
			return;
		}
		
		if (!isIncapacitated(target))
		{
			return;
		}
		
		if (isIncapacitated(player))
		{
			return;
		}
		
		if (pet_lib.isPet (target))
		{
			return;
		}
		
		if (utils.hasScriptVar(target, "contrabandChecked"))
		{
			sendSystemMessage(player, new string_id("smuggler/messages", "contraband_already_checked"));
			return;
		}
		
		if (!corpse.hasLootPermissions(target, player))
		{
			return;
		}
		
		String strLootTable = getStringObjVar(target, "loot.lootTable");
		
		if (!strLootTable.startsWith("npc") && (!strLootTable.startsWith("droid")))
		{
			sendSystemMessage(player, new string_id("smuggler/messages", "contraband_not_possible"));
			return;
		}
		
		float underworldFaction = factions.getFactionStanding( player, "underworld");
		
		int smugglerTier = getSmuggleTier(underworldFaction);
		
		int intLevel = getIntObjVar(target, "intCombatDifficulty");
		
		contrabandDropCheck(player, target, smugglerTier, intLevel);
		
		return;
	}
	
	
	public static void removeFromBountyTerminal(obj_id self, int questCrc, boolean failed) throws InterruptedException
	{
		if (questCrc == (1824374497) || questCrc == (1752998742))
		{
			if (hasObjVar(self, "smuggler.bounty"))
			{
				int bounty = getIntObjVar(self, "smuggler.bounty");
				int current_bounty = 0;
				
				if (failed)
				{
					bounty *= 0.95f;
				}
				
				if (hasObjVar(self, "bounty.amount"))
				{
					current_bounty = getIntObjVar(self, "bounty.amount");
				}
				
				current_bounty -= bounty;
				if (current_bounty <= 0)
				{
					current_bounty = 0;
					removeObjVar(self, "bounty.amount");
					setJediBountyValue(self, 0);
				}
				else
				{
					setObjVar(self, "bounty.amount", current_bounty);
					setJediBountyValue(self, current_bounty);
				}
				
				if (current_bounty < 15000)
				{
					obj_id[] hunters = getJediBounties(self);
					
					dictionary d = new dictionary();
					d.put("target", self);
					d.put("bounty", current_bounty);
					
					if (hunters != null && hunters.length > 0)
					{
						for (int i = 0; i < hunters.length; i++)
						{
							testAbortScript();
							messageTo(hunters[i], "handleBountyMissionIncomplete", d, 0.0f, true);
						}
					}
					
					removeAllJediBounties(self);
				}
				
				removeObjVar(self, "smuggler.bounty");
				updateJediScriptData(self, "smuggler", 0);
			}
		}
		
		return;
	}
	
	
	public static void checkSmugglerMissionBountyFailure(obj_id player, obj_id killer) throws InterruptedException
	{
		if (utils.isProfession(player, utils.SMUGGLER))
		{
			if (groundquests.isQuestActive(player, "quest/smuggle_pvp_4") || groundquests.isQuestActive(player, "quest/smuggle_pvp_5"))
			{
				int amount = 50;
				
				if (groundquests.isQuestActive(player, "quest/smuggle_pvp_5"))
				{
					amount = 100;
				}
				
				groundquests.applyQuestPenalty(killer, "underworld", amount);
				
				removeObjVar(player, "smuggler.bounty");
				updateJediScriptData(player, "smuggler", 0);
				
				messageTo(player, "handleSmugglerMissionFailureSignal", null, 1.0f, true);
			}
		}
	}
	
	
	public static void checkSmugglerTitleGrants(obj_id player, float value) throws InterruptedException
	{
		
		if (value >= 1000.0f)
		{
			if (!hasSkill(player, "sm_title_bootlegger"))
			{
				grantSkill(player, "sm_title_bootlegger");
			}
		}
		else
		{
			if (hasSkill(player, "sm_title_bootlegger"))
			{
				revokeSkill(player, "sm_title_bootlegger");
			}
		}
		
		if (value >= 2000.0f)
		{
			if (!hasSkill(player, "sm_title_pirate"))
			{
				grantSkill(player, "sm_title_pirate");
			}
		}
		else
		{
			if (hasSkill(player, "sm_title_pirate"))
			{
				revokeSkill(player, "sm_title_pirate");
			}
		}
		
		if (value >= 3000.0f)
		{
			if (!hasSkill(player, "sm_title_master_smuggler"))
			{
				grantSkill(player, "sm_title_master_smuggler");
			}
		}
		else
		{
			if (hasSkill(player, "sm_title_master_smuggler"))
			{
				revokeSkill(player, "sm_title_master_smuggler");
			}
		}
		return;
	}
	
	
	public static void checkBountyTitleGrants(obj_id player, float value) throws InterruptedException
	{
		if (value <= -1000.0f)
		{
			if (!hasSkill(player, "bh_title_informant"))
			{
				grantSkill(player, "bh_title_informant");
			}
		}
		
		if (value <= -2000.0f)
		{
			if (!hasSkill(player, "bh_title_inspector"))
			{
				grantSkill(player, "bh_title_inspector");
			}
		}
		
		if (value <= -3000.0f)
		{
			if (!hasSkill(player, "bh_title_agent"))
			{
				grantSkill(player, "bh_title_agent");
			}
		}
		return;
	}
	
	
	public static void checkRewardQuestGrants(obj_id target, float value) throws InterruptedException
	{
		if (value >= 1000.0f)
		{
			if ((!groundquests.isQuestActive(target, "smuggler_reward_1") && (!groundquests.hasCompletedQuest(target, "smuggler_reward_1")) ))
			{
				groundquests.requestGrantQuest(target, "smuggler_reward_1");
			}
		}
		
		if (value >= 2000.0f)
		{
			if ((!groundquests.isQuestActive(target, "smuggler_reward_2") && (!groundquests.hasCompletedQuest(target, "smuggler_reward_2")) ))
			{
				groundquests.requestGrantQuest(target, "smuggler_reward_2");
			}
		}
		
		if (value >= 3000.0f)
		{
			if ((!groundquests.isQuestActive(target, "smuggler_reward_3") && (!groundquests.hasCompletedQuest(target, "smuggler_reward_3")) ))
			{
				groundquests.requestGrantQuest(target, "smuggler_reward_3");
			}
		}
		
		return;
	}
	
	
	public static boolean moveBuyBackObjectIntoContainer(obj_id player, obj_id object, int credits) throws InterruptedException
	{
		if (!isValidId(player) || !exists(player))
		{
			return false;
		}
		
		if (!isValidId(object) || !exists(object))
		{
			return false;
		}
		
		if (credits <= 0)
		{
			CustomerServiceLog("Junk_Dealer: ", "smuggler.moveBuyBackObjectIntoContainer() - Player (OID: "+ player + ") attempted to move an object with a value of 0 or less into his buy back container.");
			return false;
		}
		
		if (!hasObjVar(player, BUYBACK_CONTAINER_VAR))
		{
			CustomerServiceLog("Junk_Dealer: ", "smuggler.moveBuyBackObjectIntoContainer() - Player (OID: "+ player + ") did not havee a buy back conteiner objvar yet!!");
			return false;
		}
		
		obj_id objContainer = getObjIdObjVar(player, BUYBACK_CONTAINER_VAR);
		if (!isIdValid(objContainer))
		{
			CustomerServiceLog("Junk_Dealer: ", "smuggler.moveBuyBackObjectIntoContainer() - Player (OID: "+ player + ") does not have a valid buy back container.");
			return false;
		}
		
		obj_id[] allObjects = getBuyBackItemsInContainer(player);
		if (allObjects != null && allObjects.length == 10)
		{
			if (!isIdValid(allObjects[0]) || getIntObjVar(allObjects[0], BUYBACK_OBJ_TIMESTAMP) <= 0)
			{
				return false;
			}
			
			obj_id oldestObject = allObjects[0];
			int oldestTime = getIntObjVar(oldestObject, BUYBACK_OBJ_TIMESTAMP);
			for (int i = 1; i <= allObjects.length - 1; i++)
			{
				testAbortScript();
				if (!isIdValid(allObjects[i]))
				{
					continue;
				}
				
				int oldTimerCandidate = getIntObjVar(allObjects[i], BUYBACK_OBJ_TIMESTAMP);
				if (oldTimerCandidate <= 0)
				{
					continue;
				}
				
				if (oldTimerCandidate < oldestTime)
				{
					oldestObject = allObjects[i];
					oldestTime = oldTimerCandidate;
				}
			}
			
			CustomerServiceLog("Junk_Dealer: ", "smuggler.moveBuyBackObjectIntoContainer() - Player (OID: "+ player + ") has had object: "+ oldestObject + " deleted in the buy back container: "+objContainer+" because the item was the oldest of the 10 objects in that container.");
			destroyObject(oldestObject);
		}
		
		int timeNow = getCalendarTime();
		setObjVar(object, BUYBACK_OBJ_TIMESTAMP, timeNow);
		setObjVar(object, BUYBACK_OBJ_CREDITS, credits);
		
		if (putIn(object, objContainer, player))
		{
			return true;
		}
		return false;
	}
	
	
	public static boolean moveBuyBackObjectIntoInventory(obj_id player, obj_id object) throws InterruptedException
	{
		if (!isValidId(player) || !exists(player))
		{
			return false;
		}
		
		if (!isValidId(object) || !exists(object))
		{
			return false;
		}
		
		if (!hasObjVar(player, BUYBACK_CONTAINER_VAR))
		{
			CustomerServiceLog("Junk_Dealer: ", "smuggler.moveBuyBackObjectIntoInventory() - Player (OID: "+ player + ") did not havee a buy back conteiner objvar yet!!");
			return false;
		}
		
		obj_id objContainer = utils.getInventoryContainer(player);
		if (!isIdValid(objContainer) || !exists(objContainer))
		{
			CustomerServiceLog("Junk_Dealer: ", "smuggler.moveBuyBackObjectIntoInventory() - Player (OID: "+ player + ") does not have a valid buy back container.");
			return false;
		}
		
		if (putIn(object, objContainer, player))
		{
			removeObjVar(object, BUYBACK_OBJ_TIMESTAMP);
			removeObjVar(object, BUYBACK_OBJ_CREDITS);
			removeObjVar(object, BUYBACK_OBJ_SOLD);
			return true;
		}
		
		CustomerServiceLog("Junk_Dealer: ", "smuggler.moveBuyBackObjectIntoInventory() - Player (OID: "+ player + ") could not put object: "+object+" in buy back container.");
		return false;
	}
	
	
	public static obj_id[] getBuyBackItemsInContainer(obj_id player) throws InterruptedException
	{
		if (!isValidId(player) || !exists(player))
		{
			return null;
		}
		
		if (!hasObjVar(player, BUYBACK_CONTAINER_VAR))
		{
			return null;
		}
		
		obj_id objContainer = getObjIdObjVar(player, BUYBACK_CONTAINER_VAR);
		if (!isIdValid(objContainer) || !exists(objContainer))
		{
			CustomerServiceLog("Junk_Dealer: ", "smuggler.getBuyBackItemsInContainer() - Player (OID: "+ player + ") does not have a valid buy back container.");
			return null;
		}
		
		obj_id[] objContents = utils.getContents(objContainer, true);
		if (objContents == null || objContents.length <= 0)
		{
			return null;
		}
		
		return objContents;
	}
	
	
	public static boolean hasBuyBackContainer(obj_id player) throws InterruptedException
	{
		if (!isValidId(player) || !exists(player))
		{
			return false;
		}
		
		if (!hasObjVar(player, BUYBACK_CONTAINER_VAR))
		{
			return false;
		}
		
		obj_id buyBackContainer = getObjIdObjVar(player, BUYBACK_CONTAINER_VAR);
		if (!isValidId(buyBackContainer) || !exists(buyBackContainer))
		{
			CustomerServiceLog("Junk_Dealer: ", "smuggler.hasBuyBackContainer() - Player (OID: "+ player + ") does not have a valid buy back container.");
			return false;
		}
		
		return true;
	}
	
	
	public static obj_id createBuyBackControlDeviceOnPlayer(obj_id player) throws InterruptedException
	{
		if (!isValidId(player) || !exists(player))
		{
			return null;
		}
		
		obj_id datapad = utils.getPlayerDatapad(player);
		if (!isValidId(datapad))
		{
			CustomerServiceLog("Junk_Dealer: ", "smuggler.createBuyBackControlDeviceOnPlayer() - Player (OID: "+ player + ") did not have a valid datapad.");
			return null;
		}
		
		obj_id[] objContents = utils.getContents(datapad, true);
		if (objContents != null && objContents.length > 0)
		{
			for (int i = 0; i < objContents.length; i++)
			{
				testAbortScript();
				if (isIdValid(objContents[i]))
				{
					String template = getTemplateName(objContents[i]);
					if (template != null && template.length() > 0)
					{
						if (template.equals(BUY_BACK_CONTROL_DEVICE_TEMPLATE))
						{
							CustomerServiceLog("Junk_Dealer: ", "smuggler.createBuyBackControlDeviceOnPlayer() - Player (OID: "+ player + ") had an already valid buy back controller device that somehow got removed!: "+objContents[i]);
							setBuyBackContainerObjVar(player, objContents[i]);
							return objContents[i];
						}
					}
				}
			}
		}
		
		obj_id buyBackControlDevice = createObject(BUY_BACK_CONTROL_DEVICE_TEMPLATE, datapad, "");
		if (!isValidId(buyBackControlDevice))
		{
			CustomerServiceLog("Junk_Dealer: ", "smuggler.createBuyBackControlDeviceOnPlayer() - Player (OID: "+ player + ") did not receive a valid buy back container.");
			return null;
		}
		
		setBuyBackContainerObjVar(player, buyBackControlDevice);
		return buyBackControlDevice;
	}
	
	
	public static void setBuyBackContainerObjVar(obj_id player, obj_id buyBackControlDevice) throws InterruptedException
	{
		obj_id buyBackContainer = utils.getInventoryContainer(buyBackControlDevice);
		if (isIdValid(buyBackContainer))
		{
			setObjVar(player, BUYBACK_CONTAINER_VAR, buyBackContainer);
		}
		else
		{
			CustomerServiceLog("Junk_Dealer: ", "smuggler.setBuyBackContainerObjVar() - Player (OID: "+ player + ") buy back control device did not have a back buy container in its inventory.");
		}
		return;
	}
	
	
	public static obj_id getBuyBackContainerObjVar(obj_id player) throws InterruptedException
	{
		if (hasObjVar(player, BUYBACK_CONTAINER_VAR))
		{
			return getObjIdObjVar(player, BUYBACK_CONTAINER_VAR);
		}
		CustomerServiceLog("Junk_Dealer: ", "smuggler.getBuyBackContainerObjVar() - Player (OID: "+ player + ") did not have a buy back container objvar. Attempting to find buy back control device.");
		return utils.getInventoryContainer(createBuyBackControlDeviceOnPlayer(player));
	}
	
	
	public static boolean hasBuyBackItems(obj_id player) throws InterruptedException
	{
		if (!isValidId(player))
		{
			return false;
		}
		
		obj_id[] listOfBuyBacks = getBuyBackItemsInContainer(player);
		return listOfBuyBacks != null && listOfBuyBacks.length > 0;
	}
	
	
	public static boolean showBuyBackSui(obj_id player, obj_id target) throws InterruptedException
	{
		
		if (!isIdValid(player) || !isIdValid(target))
		{
			return false;
		}
		
		if (utils.outOfRange(target, player, 10.0f, true))
		{
			CustomerServiceLog("Junk_Dealer: ", "smuggler.showBuyBackSui() - Player: "+player+" attempted to get a list of buy back items but the action failed. The player was too far away from the NPC.");
			return false;
		}
		
		obj_id playerInv = utils.getInventoryContainer(player);
		if (isIdValid(playerInv))
		{
			int free_space = getVolumeFree(playerInv);
			if (free_space < 1)
			{
				sendSystemMessage(player, SID_NO_SPACE_IN_INVENTORY);
				CustomerServiceLog("Junk_Dealer: ", "smuggler.showBuyBackSui() - Player: "+player+" could not buy back any more items due to inventory space restrictions.");
				return false;
			}
		}
		
		if (utils.hasScriptVar(player, SCRIPTVAR_JUNK_SUI))
		{
			
			int pidClose = utils.getIntScriptVar(player, SCRIPTVAR_JUNK_SUI);
			cleanupSellJunkSui(player);
			
			sui.closeSUI(player, pidClose);
		}
		
		obj_id[] junk = getBuyBackItemsInContainer(player);
		
		if (junk == null && junk.length <= 0)
		{
			CustomerServiceLog("Junk_Dealer: ", "smuggler.showBuyBackSui() - Player: "+player+" attempted to get a list of buy back items but the action failed. There were no items to buy back.");
			sendSystemMessage(player, SID_NO_BUY_BACK_ITEMS_FOUND);
			return false;
		}
		
		String[] entries = new String[junk.length];
		
		for (int i = 0; i < junk.length; i++)
		{
			testAbortScript();
			int price = getIntObjVar(junk[i], BUYBACK_OBJ_CREDITS);
			
			String name = getAssignedName(junk[i]);
			if (name.equals("") || name == null)
			{
				name = getString(getNameStringId(junk[i]));
			}
			String entry = "["+ price + "] "+ name;
			
			entries[i] = entry;
		}
		
		if (entries == null || entries.length <= 0)
		{
			CustomerServiceLog("Junk_Dealer: ", "smuggler.showBuyBackSui() - Player: "+player+" attempted to get a list of buy back items but the action failed. Somehow the Junk Dealer system failed to create a list of buy back item string names.");
			return false;
		}
		
		int pid = sui.listbox(target, player, BUY_BACK_PROMPT, sui.OK_CANCEL_REFRESH, BUY_BACK_TITLE, entries, "handleBuyBackSui", false, false);
		if (pid <= -1)
		{
			return false;
		}
		
		setSUIProperty(pid, sui.LISTBOX_BTN_OK, sui.PROP_TEXT, BTN_BUY_BACK);
		setSUIProperty (pid, sui.LISTBOX_BTN_OTHER, sui.PROP_VISIBLE, "true");
		setSUIProperty (pid, sui.LISTBOX_BTN_OTHER, sui.PROP_TEXT, BTN_EXAMINE);
		subscribeToSUIProperty (pid, "this", sui.LISTBOX_PROP_OTHER_PRESSED);
		showSUIPage(pid);
		
		utils.setScriptVar(player, SCRIPTVAR_JUNK_SUI, pid);
		utils.setBatchScriptVar(player, SCRIPTVAR_JUNK_IDS, junk);
		return true;
	}
	
	
	public static boolean buyBackJunkItem(obj_id player, obj_id junkDealer, obj_id item, boolean reshowSui) throws InterruptedException
	{
		
		if (!isIdValid(player) || !isIdValid(item) || !isValidId(junkDealer))
		{
			return false;
		}
		
		if (utils.outOfRange(junkDealer, player, 10.0f, true))
		{
			CustomerServiceLog("Junk_Dealer: ", "smuggler.buyBackJunkItem() - Player: "+player+" cannot buy back item: "+item+" because they walked too far from the Junk Dealer: "+junkDealer);
			return false;
		}
		
		if (!hasObjVar(player, smuggler.BUYBACK_CONTAINER_VAR))
		{
			CustomerServiceLog("Junk_Dealer: ", "smuggler.buyBackJunkItem() - Player: "+player+" cannot buy back item: "+item+" because the player doesn't have a buy back container objvar.");
			return false;
		}
		
		if (!hasObjVar(item, smuggler.BUYBACK_OBJ_SOLD))
		{
			CustomerServiceLog("Junk_Dealer: ", "smuggler.buyBackJunkItem() - Player: "+player+" cannot buy back item: "+item+" because it has a sold objvar. This item has already been marked as sold. Aborting.");
			return false;
		}
		
		if (!hasObjVar(item, smuggler.BUYBACK_OBJ_CREDITS) || getIntObjVar(item, smuggler.BUYBACK_OBJ_CREDITS) <= 0)
		{
			CustomerServiceLog("Junk_Dealer: ", "smuggler.buyBackJunkItem() - Player: "+player+" cannot buy back item: "+item+" because the credit amount was not found or invalid.");
			return false;
		}
		
		int price = getIntObjVar(item, smuggler.BUYBACK_OBJ_CREDITS);
		
		dictionary params = new dictionary();
		params.put("item",item);
		params.put("price",price);
		CustomerServiceLog("Junk_Dealer: ", "junk_dealer.buyBackJunkItem() - Player: "+player+" is about to be charged: "+price+" for object: "+item+" as part of the buy back functionality.");
		money.requestPayment(player, junkDealer, price, "handleTheBuyBack", params, true);
		return true;
	}
	
	
	public static void flagJunkSaleSui(obj_id player, obj_id salesman) throws InterruptedException
	{
		
		if (!isIdValid(player) || !isValidId(salesman))
		{
			return;
		}
		
		if (utils.outOfRange(salesman, player, 10.0f, true))
		{
			return;
		}
		
		if (utils.hasScriptVar(player, smuggler.SCRIPTVAR_JUNK_SUI))
		{
			
			int pidClose = utils.getIntScriptVar(player, smuggler.SCRIPTVAR_JUNK_SUI);
			smuggler.cleanupSellJunkSui(player);
			
			sui.closeSUI(player, pidClose);
		}
		
		obj_id[] junk = smuggler.getAllJunkItems(player, true);
		
		if (junk != null && junk.length > 0)
		{
			String[] entries = new String[junk.length];
			
			for (int i = 0; i < junk.length; i++)
			{
				testAbortScript();
				
				String entry = "[ Sellable ] "+getString(getNameStringId(junk[i]));
				if (hasObjVar(junk[i], smuggler.JUNK_DEALKER_NO_SALE_FLAG))
				{
					entry = "[ *No Sell* ] "+getString(getNameStringId(junk[i]));
				}
				
				entries[i] = entry;
			}
			
			if (entries != null && entries.length > 0)
			{
				
				int pid = sui.listbox(salesman, player, smuggler.FLAG_NO_SELL_DESC, sui.OK_CANCEL_REFRESH, smuggler.FLAG_NO_SELL_TITLE, entries, "handleFlagJunkSui", false, false);
				
				if (pid > -1)
				{
					
					setSUIProperty(pid, sui.LISTBOX_BTN_OK, sui.PROP_TEXT, smuggler.FLAG_NO_SELL);
					setSUIProperty (pid, sui.LISTBOX_BTN_OTHER, sui.PROP_VISIBLE, "true");
					setSUIProperty (pid, sui.LISTBOX_BTN_OTHER, sui.PROP_TEXT, smuggler.BTN_EXAMINE);
					subscribeToSUIProperty (pid, "this", sui.LISTBOX_PROP_OTHER_PRESSED);
					showSUIPage(pid);
					
					utils.setScriptVar(player, smuggler.SCRIPTVAR_JUNK_SUI, pid);
					utils.setBatchScriptVar(player, smuggler.SCRIPTVAR_JUNK_IDS, junk);
				}
			}
		}
		
		else
		{
			int msgPid = sui.msgbox(salesman, player, smuggler.NO_ITEMS_FOUND_PROMPT, sui.OK_ONLY, smuggler.FLAG_ITEM_TITLE, "noHandler");
			smuggler.cleanupSellJunkSui(player);
		}
	}
	
	
	public static boolean blog(String txt) throws InterruptedException
	{
		if (LOGGING_ON)
		{
			LOG(LOGNAME, txt);
		}
		return true;
	}
}
