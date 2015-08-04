package script.systems.sign;

import script.*;
import script.base_class.*;
import script.combat_engine.*;
import java.util.Arrays;
import java.util.Hashtable;
import java.util.Vector;
import script.base_script;


import script.library.sui;
import script.library.utils;
import script.library.player_structure;
import script.library.prose;
import script.library.utils;


public class sign extends script.base_script
{
	public sign()
	{
	}
	public static final boolean LOGGING_ON = false;
	public static final String LOGGING_CATEGORY = "packup";
	
	public static final string_id SID_TERMINAL_MANAGEMENT = new string_id("player_structure", "management");
	public static final string_id SID_TERMINAL_PACK_HOUSE = new string_id("sui", "packup_house");
	public static final string_id EMAIL_TITLE = new string_id("spam", "email_title");
	public static final string_id EMAIL_BODY = new string_id("spam", "email_body");
	public static final string_id SID_OWNER_PACKUP_AT_TERMINAL = new string_id("player_structure", "onwer_packup_at_terminal");
	public static final int minTimeDelayBetweenSameServerRequests = 300;
	public static final String timeOfLastSameServerRequest = "timeOfLastSameServerRequest";
	
	
	public int OnObjectMenuRequest(obj_id self, obj_id player, menu_info mi) throws InterruptedException
	{
		menu_info_data menuData = mi.getMenuItemByType(menu_info_types.ITEM_USE);
		if (menuData != null)
		{
			menuData.setServerNotify(true);
		}
		
		deltadictionary scriptvars = self.getScriptVars();
		obj_id house = scriptvars.getObjId("player_structure.parent");
		
		if (player_structure.canShowPackOption(player, house))
		{
			int management_root = mi.addRootMenu (menu_info_types.SERVER_TERMINAL_MANAGEMENT, SID_TERMINAL_MANAGEMENT);
			mi.addSubMenu( management_root, menu_info_types.SERVER_MENU10, SID_TERMINAL_PACK_HOUSE);
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int OnObjectMenuSelect(obj_id self, obj_id player, int item) throws InterruptedException
	{
		if (item == menu_info_types.ITEM_USE)
		{
			string_id desc = getDescriptionStringId(self);
			String text = utils.packStringId(desc);
			sui.msgbox(self, player, text, "noHandlerNeeded");
		}
		else if (item == menu_info_types.SERVER_MENU10)
		{
			
			deltadictionary scriptvars = self.getScriptVars();
			obj_id house = scriptvars.getObjId("player_structure.parent");
			
			if (player_structure.isOwner(house, player))
			{
				sendSystemMessage(player, SID_OWNER_PACKUP_AT_TERMINAL);
				return SCRIPT_CONTINUE;
			}
			
			if (player_structure.isPlayerGatedFromHousePackUp(player))
			{
				return SCRIPT_CONTINUE;
			}
			
			if (!player_structure.canPlayerPackAbandonedStructure(player, house))
			{
				
				if (isAtPendingLoadRequestLimit())
				{
					sendSystemMessage(player, new string_id("player_structure", "abandoned_structure_pack_up_try_again_later"));
				}
				else if (player_structure.isAbandoned(house) && (!house.isAuthoritative() || !player.isAuthoritative()))
				{
					if ((!utils.hasScriptVar(player, timeOfLastSameServerRequest) || utils.getIntScriptVar(player, timeOfLastSameServerRequest) < getGameTime()))
					{
						requestSameServer(player, house);
						utils.setScriptVar(player, timeOfLastSameServerRequest, getGameTime() + minTimeDelayBetweenSameServerRequests);
						utils.setScriptVar(player, "requestedSameServerToAbandonHouse", house);
					}
					
					sendSystemMessage(player, new string_id("player_structure", "abandoned_structure_pack_up_please_wait_processing"));
				}
				
				return SCRIPT_CONTINUE;
			}
			
			if (isIdValid(house))
			{
				dictionary params = new dictionary();
				params.put( "house", house );
				params.put( "player", player );
				
				messageTo (house, "packAbandonedBuilding", params, 4, false);
				messageTo (player, "handlePlayerStructurePackupLockout", null, 0, false);
				
				if (!hasObjVar(player, player_structure.HOUSE_PACKUP_ARRAY_OBJVAR))
				{
					String recipient = getPlayerName(player);
					utils.sendMail(EMAIL_TITLE, EMAIL_BODY, recipient, "Galactic Vacant Building Demolishing Movement");
				}
			}
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public boolean blog(String msg) throws InterruptedException
	{
		if (LOGGING_ON && msg != null && !msg.equals(""))
		{
			LOG(LOGGING_CATEGORY, msg);
		}
		return true;
	}
}
