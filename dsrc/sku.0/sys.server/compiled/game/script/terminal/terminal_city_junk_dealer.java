package script.terminal;

import script.*;
import script.base_class.*;
import script.combat_engine.*;
import java.util.Arrays;
import java.util.Hashtable;
import java.util.Vector;
import script.base_script;

import script.library.city;
import script.library.money;
import script.library.smuggler;
import script.library.sui;
import script.library.utils;


public class terminal_city_junk_dealer extends script.base_script
{
	public terminal_city_junk_dealer()
	{
	}
	public static final string_id SID_MT_REMOVE = new string_id("city/city", "mt_remove");
	public static final string_id SID_MT_REMOVED = new string_id("city/city", "mt_removed");
	public static final boolean LOGGING_ON = true;
	public static final String LOGNAME = "junk_log";
	
	
	public int OnInitialize(obj_id self) throws InterruptedException
	{
		
		city.validateSpecialStructure( self );
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int OnDestroy(obj_id self) throws InterruptedException
	{
		city.removeMissionTerminal( self );
		return SCRIPT_CONTINUE;
	}
	
	
	public int OnObjectMenuRequest(obj_id self, obj_id player, menu_info mi) throws InterruptedException
	{
		int city_id = city.checkMayorCity(player, false);
		
		if (city_id != 0)
		{
			mi.addRootMenu( menu_info_types.SERVER_MENU1, SID_MT_REMOVE );
		}
		
		int mnuSellJunk = mi.addRootMenu(menu_info_types.ITEM_USE, smuggler.MENU_SELL);
		mi.addRootMenu( menu_info_types.SERVER_MENU2, smuggler.MENU_FLAG_NO_SALE);
		if (smuggler.hasBuyBackItems(player))
		{
			mi.addRootMenu( menu_info_types.SERVER_MENU3, smuggler.MENU_BUY_BACK);
		}
		return SCRIPT_CONTINUE;
	}
	
	
	public int OnObjectMenuSelect(obj_id self, obj_id player, int item) throws InterruptedException
	{
		if (item == menu_info_types.ITEM_USE)
		{
			dictionary params = new dictionary();
			params.put("player", player);
			
			messageTo(self, "startDealing", params, 0.0f, false);
		}
		if (item == menu_info_types.SERVER_MENU1)
		{
			destroyObject(self);
		}
		if (item == menu_info_types.SERVER_MENU2)
		{
			dictionary params = new dictionary();
			params.put("player", player);
			
			messageTo(self, "startFlaggingItemsNoSale", params, 0.0f, false);
		}
		if (item == menu_info_types.SERVER_MENU3)
		{
			dictionary params = new dictionary();
			params.put("player", player);
			
			messageTo(self, "startBuyBack", params, 0.0f, false);
		}
		sendDirtyObjectMenuNotification(self);
		return SCRIPT_CONTINUE;
	}
	
	
	public int startDealing(obj_id self, dictionary params) throws InterruptedException
	{
		
		obj_id player = params.getObjId("player");
		
		if (utils.outOfRange(self, player, 10.0f, true))
		{
			return SCRIPT_CONTINUE;
		}
		
		smuggler.showSellJunkSui(player, self, false, false);
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int startBuyBack(obj_id self, dictionary params) throws InterruptedException
	{
		
		obj_id player = params.getObjId("player");
		
		if (utils.outOfRange(self, player, 10.0f, true))
		{
			return SCRIPT_CONTINUE;
		}
		
		smuggler.showBuyBackSui(player, self);
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int handleSellJunkSui(obj_id self, dictionary params) throws InterruptedException
	{
		
		obj_id player = sui.getPlayerId(params);
		
		if (!isIdValid(player))
		{
			return SCRIPT_CONTINUE;
		}
		
		if (utils.outOfRange(self, player, 10.0f, true))
		{
			return SCRIPT_CONTINUE;
		}
		
		utils.setScriptVar(player, "fence", false);
		messageTo(player, "handleSellJunkSui", params, 0.0f, false);
		return SCRIPT_CONTINUE;
	}
	
	
	public int handleSoldJunk(obj_id self, dictionary params) throws InterruptedException
	{
		
		if (params == null || params.isEmpty())
		{
			return SCRIPT_CONTINUE;
		}
		
		obj_id player = params.getObjId(money.DICT_TARGET_ID);
		if (!isIdValid(player))
		{
			return SCRIPT_CONTINUE;
		}
		
		if (utils.outOfRange(self, player, 10.0f, true))
		{
			return SCRIPT_CONTINUE;
		}
		
		blog("junk_dealer.handleSoldJunk() - setting fence to false");
		params.put("fence",false);
		messageTo(player, "handleSoldJunk", params, 0.0f, false);
		return SCRIPT_CONTINUE;
	}
	
	
	public int handleBuyBackSui(obj_id self, dictionary params) throws InterruptedException
	{
		
		obj_id player = sui.getPlayerId(params);
		
		if (!isIdValid(player))
		{
			return SCRIPT_CONTINUE;
		}
		
		if (utils.outOfRange(self, player, 10.0f, true))
		{
			return SCRIPT_CONTINUE;
		}
		params.put("dealer",self);
		messageTo(player, "handleBuyBackSui", params, 0.0f, false);
		return SCRIPT_CONTINUE;
	}
	
	
	public int handleTheBuyBack(obj_id self, dictionary params) throws InterruptedException
	{
		if (params == null || params.isEmpty())
		{
			return SCRIPT_CONTINUE;
		}
		
		obj_id player = params.getObjId(money.DICT_PLAYER_ID);
		if (!isIdValid(player))
		{
			CustomerServiceLog("Junk_Dealer: ", "terminal_city_junk_dealer.handleTheBuyBack() - Failed to sell back object because player was invalid.");
			return SCRIPT_CONTINUE;
		}
		
		if (utils.outOfRange(self, player, 10.0f, true))
		{
			CustomerServiceLog("Junk_Dealer: ", "terminal_city_junk_dealer.handleTheBuyBack() - Player: "+player+" cannot buy back an item because they walked too far from the Junk Dealer: "+self);
			return SCRIPT_CONTINUE;
		}
		
		blog("terminal_city_junk_dealer.handleTheBuyBack() - setting fence to false");
		params.put("dealer",self);
		messageTo(player, "handleTheBuyBack", params, 0.0f, false);
		return SCRIPT_CONTINUE;
	}
	
	
	public int startFlaggingItemsNoSale(obj_id self, dictionary params) throws InterruptedException
	{
		
		obj_id player = params.getObjId("player");
		
		if (utils.outOfRange(self, player, 10.0f, true))
		{
			return SCRIPT_CONTINUE;
		}
		
		smuggler.flagJunkSaleSui(player, self);
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int handleFlagJunkSui(obj_id self, dictionary params) throws InterruptedException
	{
		obj_id player = sui.getPlayerId(params);
		
		if (!isIdValid(player))
		{
			return SCRIPT_CONTINUE;
		}
		
		if (utils.outOfRange(self, player, 10.0f, true))
		{
			return SCRIPT_CONTINUE;
		}
		
		blog("junk_dealer.handleFlagJunkSui() - sending params to player.");
		params.put("dealer",self);
		messageTo(player, "handleFlagJunkSui", params, 0.0f, false);
		return SCRIPT_CONTINUE;
	}
	
	
	public boolean blog(String txt) throws InterruptedException
	{
		if (LOGGING_ON)
		{
			LOG(LOGNAME, txt);
		}
		return true;
	}
}
