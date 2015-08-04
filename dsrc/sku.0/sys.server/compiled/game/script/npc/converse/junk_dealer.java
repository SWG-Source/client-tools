package script.npc.converse;

import script.*;
import script.base_class.*;
import script.combat_engine.*;
import java.util.Arrays;
import java.util.Hashtable;
import java.util.Vector;
import script.base_script;

import script.library.sui;
import script.library.chat;
import script.library.group;
import script.library.prose;
import script.library.money;
import script.library.ai_lib;
import script.library.utils;
import script.library.luck;
import script.library.static_item;
import script.library.smuggler;


public class junk_dealer extends script.base_script
{
	public junk_dealer()
	{
	}
	public static final boolean LOGGING_ON = true;
	public static final String LOGNAME = "junk_log";
	
	
	public int OnAttach(obj_id self) throws InterruptedException
	{
		ai_lib.setDefaultCalmBehavior(self, ai_lib.BEHAVIOR_SENTINEL);
		
		String creatureName = getCreatureName(self);
		if (creatureName.equals("junk_jawa"))
		{
			
			return SCRIPT_CONTINUE;
		}
		if (!creatureName.equals("junk_dealer_smuggler"))
		{
			attachScript(self, "conversation.junk_dealer_generic");
		}
		else
		{
			attachScript(self, "conversation.junk_dealer_smuggler");
		}
		
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
			CustomerServiceLog("Junk_Dealer: ", "junk_dealer.handleTheBuyBack() - Failed to sell back object because player was invalid.");
			return SCRIPT_CONTINUE;
		}
		
		if (utils.outOfRange(self, player, 10.0f, true))
		{
			CustomerServiceLog("Junk_Dealer: ", "junk_dealer.handleTheBuyBack() - Player: "+player+" cannot buy back an item because they walked too far from the Junk Dealer: "+self);
			return SCRIPT_CONTINUE;
		}
		
		blog("junk_dealer.handleTheBuyBack() - sending params to player.");
		params.put("dealer",self);
		messageTo(player, "handleTheBuyBack", params, 0.0f, false);
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
