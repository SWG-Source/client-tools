package script.terminal;

import script.*;
import script.base_class.*;
import script.combat_engine.*;
import java.util.Arrays;
import java.util.Hashtable;
import java.util.Vector;
import script.base_script;


import script.library.city;
import script.library.create;
import script.library.locations;
import script.library.pet_lib;
import script.library.player_structure;
import script.library.travel;
import script.library.utils;


public class terminal_travel_instant extends script.base_script
{
	public terminal_travel_instant()
	{
	}
	public static final string_id SID_TIMEOUT = new string_id("travel", "pickup_timeout");
	public static final string_id SID_LEFT_ME = new string_id("travel", "left_pickup_zone");
	public static final string_id SID_NOT_YOUR_SHIP = new string_id("travel", "not_your_ship");
	public static final String TRIGGER_VOLUME_PICKUP_SHIP = "travel_instant_pickup_interest_range";
	public static final float PICKUP_INTEREST_RADIUS = 64f;
	
	public static final boolean CONST_FLAG_DO_LOGGING = false;
	
	
	public int OnInitialize(obj_id self) throws InterruptedException
	{
		debugLogging("//***// OnInitialize: ", "////>>>> ENTERED. ");
		
		messageTo(self, "timeOutSelfExpire", null, 60, false);
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int OnObjectMenuRequest(obj_id self, obj_id player, menu_info mi) throws InterruptedException
	{
		obj_id owner = (playerCheck(self, "OnObjectMenuRequest - "));
		if (owner != player)
		{
			sendSystemMessage( player, SID_NOT_YOUR_SHIP );
			return SCRIPT_CONTINUE;
		}
		
		menu_info_data data = mi.getMenuItemByType (menu_info_types.ITEM_USE);
		
		if (data != null)
		{
			data.setServerNotify (true);
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int OnObjectMenuSelect(obj_id self, obj_id player, int item) throws InterruptedException
	{
		obj_id owner = (playerCheck(self, "OnObjectMenuSelect - "));
		if (owner != player)
		{
			return SCRIPT_CONTINUE;
		}
		
		if (getState(player, STATE_RIDING_MOUNT) == 1)
		{
			pet_lib.doDismountNow(player, true);
		}
		
		if (item == menu_info_types.ITEM_USE)
		{
			String planet = getCurrentSceneName();
			String travel_point = "Starfighter";
			
			int cityId = getCityAtLocation(getLocation(player), 1000);
			debugLogging("//***// OnObjectMenuSelect: ", "////>>>> cityId at player's location is: "+cityId);
			if (cityId != 0)
			{
				travel_point = cityGetName(cityId);
				debugLogging("//***// OnObjectMenuSelect: ", "////>>>> city name at player's location is: "+travel_point);
			}
			
			LOG("LOG_CHANNEL", "player ->"+ player + " planet ->"+ planet + " travel_point ->"+ travel_point);
			
			String config = getConfigSetting("GameServer", "disableTravelSystem");
			if (config != null)
			{
				if (config.equals("on"))
				{
					return SCRIPT_CONTINUE;
				}
			}
			
			utils.setScriptVar(player, travel.SCRIPT_VAR_TERMINAL, self);
			utils.setScriptVar(player, "instantTravel", true);
			boolean success = enterClientTicketPurchaseMode(player, planet, travel_point, true);
			if (success)
			{
				
				utils.setScriptVar(self, "transport", 1);
			}
		}
		return SCRIPT_CONTINUE;
	}
	
	
	public int OnTriggerVolumeExited(obj_id self, String volumeName, obj_id breacher) throws InterruptedException
	{
		obj_id player = (playerCheck(self, "OnTriggerVolumeEntered - "));
		debugLogging("//***// OnTriggerVolumeExited: ", "////>>>> player is leaving the instant travel terminal behind (the ship). Destroying self");
		sendSystemMessage( player, SID_LEFT_ME );
		messageTo(player, "cleanupInstantTravelScriptVars", null, 0, false);
		destroyObject(self);
		return SCRIPT_CONTINUE;
	}
	
	
	public int OnDestroy(obj_id self) throws InterruptedException
	{
		obj_id player = utils.getObjIdScriptVar(self, "player");
		if (isIdValid(player))
		{
			messageTo(player, "cleanupInstantTravelScriptVars", null, 0, false);
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public void debugLogging(String section, String message) throws InterruptedException
	{
		if (CONST_FLAG_DO_LOGGING)
		{
			LOG("debug/terminal_travel_instant/"+section, message);
		}
	}
	
	
	public void pickupObjectTriggerVolumeInitializer(obj_id self) throws InterruptedException
	{
		obj_id player = (playerCheck(self, "pickupObjectTriggerVolumeInitializer"));
		
		if (!hasTriggerVolume(self, TRIGGER_VOLUME_PICKUP_SHIP))
		{
			
			debugLogging("//***// pickupObjectTriggerVolumeInitializer: ", "////>>>> created new trigger volume TRIGGER_VOLUME_PICKUP_SHIP");
			createTriggerVolume(TRIGGER_VOLUME_PICKUP_SHIP, PICKUP_INTEREST_RADIUS, false);
			addTriggerVolumeEventSource(TRIGGER_VOLUME_PICKUP_SHIP, player);
		}
	}
	
	
	public obj_id playerCheck(obj_id self, String section) throws InterruptedException
	{
		if (utils.hasScriptVar(self,"playerOwner"))
		{
			obj_id player = utils.getObjIdScriptVar(self, "playerOwner");
			if (isIdValid(player))
			{
				return player;
			}
			else
			{
				debugLogging(section, "////>>>> the player owner scriptvar isn't valid, so destroying self");
			}
		}
		else
		{
			debugLogging(section, "////>>>> we don't have a player owner scriptvar, so no way to know what to track. Destroying self");
		}
		
		destroyObject(self);
		return null;
	}
	
	
	public int timeOutSelfExpire(obj_id self, dictionary params) throws InterruptedException
	{
		if (utils.hasScriptVar(self, "playerOwner"))
		{
			obj_id player = utils.getObjIdScriptVar(self, "playerOwner");
			if (isIdValid(player))
			{
				messageTo(player, "cleanupInstantTravelScriptVars", null, 0, false);
			}
			if (isIdValid(player) && !utils.hasScriptVar(self, "transport"))
			{
				sendSystemMessage( player, SID_TIMEOUT );
			}
			
		}
		destroyObject(self);
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int initializeInstaTravelShip(obj_id self, dictionary params) throws InterruptedException
	{
		debugLogging("//***// OnInitialize: ", "////>>>> ENTERED. ");
		
		pickupObjectTriggerVolumeInitializer(self);
		
		if (utils.hasScriptVar(self,"playerOwner"))
		{
			obj_id player = utils.getObjIdScriptVar(self,"playerOwner");
			if (isIdValid(player))
			{
				
			}
			else
			{
				debugLogging("//// OnInitialize: ", "////>>>> we don't have a valid scriptvar pointing to our owning player, so destroy self.");
				destroyObject(self);
			}
		}
		else
		{
			debugLogging("//// OnInitialize: ", "////>>>> apparently we don't have a playerOwner scriptvar, so destroy self.");
			destroyObject(self);
		}
		
		return SCRIPT_CONTINUE;
	}
}
