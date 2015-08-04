package script.npc.jawa_traders;

import script.*;
import script.base_class.*;
import script.combat_engine.*;
import java.util.Arrays;
import java.util.Hashtable;
import java.util.Vector;
import script.base_script;


import script.library.factions;
import script.library.chat;
import script.library.utils;
import script.library.ai_lib;
import script.library.locations;
import script.library.money;
import script.library.quests;
import script.library.skill;
import script.library.sui;
import script.library.prose;
import script.library.luck;
import script.library.static_item;
import script.library.smuggler;


public class jawa_trader_01 extends script.base_script
{
	public jawa_trader_01()
	{
	}
	public static final String FACETO_VOLUME_NAME = "faceToTriggerVolume";
	public static final String CONVO = "jawa_trader";
	
	public static final String TBL = "datatables/npc/junk_dealer/junk_dealer.iff";
	
	public static final String SCRIPTVAR_JUNK_SUI = "relicdealer.biogenic.sui";
	public static final String SCRIPTVAR_JUNK_IDS = "relicdealer.biogenic.ids";
	public static final String SCRIPTVAR_SOLD = "soldAsJunk";
	
	public static final String STF = "loot_dealer";
	
	public static final string_id PROSE_NO_BUY = new string_id(STF, "prose_no_buy");
	public static final string_id PROSE_NO_BUY_ALL = new string_id(STF, "prose_no_buy_all");
	public static final string_id PROSE_SOLD_JUNK = new string_id(STF, "prose_sold_junk");
	public static final string_id PROSE_NO_SALE = new string_id(STF, "prose_no_sale");
	public static final string_id PROSE_SOLD_ALL_JUNK = new string_id(STF, "prose_sold_all_junk");
	public static final string_id PROSE_NO_ALL_SALE = new string_id(STF, "prose_no_all_sale");
	public static final string_id PROSE_JUNK_SOLD = new string_id(STF, "prose_junk_sold");
	
	public static final string_id MENU_SELL = new string_id("junk_dealer", "mnu_sell_junk");
	public static final String SELL_TITLE = "@"+ STF + ":sell_title";
	public static final String SELL_PROMPT = "@"+ STF + ":sell_prompt";
	public static final String NO_ITEMS_PROMPT = "@"+ STF + ":no_items";
	public static final String BTN_SELL = "@"+ STF + ":btn_sell";
	public static final String BTN_SELL_ALL = "@"+ STF + ":btn_sell_all";
	
	
	public int OnAttach(obj_id self) throws InterruptedException
	{
		ai_lib.setDefaultCalmBehavior( self, ai_lib.BEHAVIOR_SENTINEL );
		setInvulnerable (self, true);
		createTriggerVolume( FACETO_VOLUME_NAME, 8.0f, true );
		setName ( self, "Ribbo Qua`lu (a Jawa Trader)");
		return SCRIPT_CONTINUE;
	}
	
	
	public int OnTriggerVolumeEntered(obj_id self, String volumeName, obj_id breacher) throws InterruptedException
	{
		if (!isPlayer( breacher ))
		{
			
			return SCRIPT_CONTINUE;
		}
		
		if (!volumeName.equals(FACETO_VOLUME_NAME))
		{
			return SCRIPT_CONTINUE;
		}
		
		if (isInNpcConversation(self))
		{
			return SCRIPT_CONTINUE;
		}
		
		if (canSee( self, breacher ))
		{
			faceTo( self, breacher );
		}
		
		return SCRIPT_CONTINUE;
	}
}
