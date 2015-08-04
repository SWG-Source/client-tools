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


public class terminal_travel_instant_one_use extends script.base_script
{
	public terminal_travel_instant_one_use()
	{
	}
	public static final string_id SID_LOCATION_NOGOOD_FOR_PICKUP = new string_id("travel", "no_pickup_location");
	public static final string_id SID_TIMEOUT = new string_id("travel", "pickup_timeout");
	public static final string_id SID_LEFT_ME = new string_id("travel", "left_pickup_zone");
	public static final string_id SID_NOT_YOUR_SHIP = new string_id("travel", "not_your_ship");
	public static final String TRIGGER_VOLUME_PICKUP_SHIP = "travel_instant_pickup_interest_range";
	public static final float PICKUP_INTEREST_RADIUS = 64f;
	public static final string_id SID_CALLING_FOR_PICKUP = new string_id("travel", "calling_for_pickup");
	
	public static final boolean CONST_FLAG_DO_LOGGING = true;
	public static final int SHIP_TYPE_INSTANT_XWING_TIE = 1;
	public static final int SHIP_TYPE_INSTANT_PRIVATEER = 2;
	public static final int SHIP_TYPE_INSTANT_ROYAL_SHIP = 3;
	
	
	public int OnInitialize(obj_id self) throws InterruptedException
	{
		return SCRIPT_CONTINUE;
	}
	
	
	public int OnObjectMenuRequest(obj_id self, obj_id player, menu_info mi) throws InterruptedException
	{
		
		int mnu = mi.addRootMenu(menu_info_types.ITEM_USE, new string_id("ui_radial", "item_use"));
		return SCRIPT_CONTINUE;
	}
	
	
	public int OnObjectMenuSelect(obj_id self, obj_id player, int item) throws InterruptedException
	{
		if (item == menu_info_types.ITEM_USE)
		{
			
			sendSystemMessage(player, SID_CALLING_FOR_PICKUP );
			spawnPickupCraft(player, 2);
		}
		return SCRIPT_CONTINUE;
	}
	
	
	public obj_id spawnPickupCraft(obj_id player, int type) throws InterruptedException
	{
		debugLogging("//***// spawnPickupCraft", "////>>>> ENTERED");
		
		if (!isIdValid(player))
		{
			return null;
		}
		
		location here = getLocation(player);
		location spawnLoc = locations.getGoodLocationAroundLocation(here, 1f, 1f, 4f, 4f);
		if (spawnLoc == null)
		{
			debugLogging("//***// spawnPickupCraft", "////>>>> getGoodLocationAroundLocation (player area) returned NULL! OH NO! Telling player that the location is no good.");
			sendSystemMessage( player, SID_LOCATION_NOGOOD_FOR_PICKUP );
			return null;
		}
		
		String pickupCraftType = "object/tangible/terminal/terminal_travel_instant_xwing.iff";
		
		if (type == SHIP_TYPE_INSTANT_XWING_TIE)
		{
			
			int playerFactionID = pvpGetAlignedFaction(player);
			
			if (playerFactionID == (-615855020))
			{
				pickupCraftType = "object/tangible/terminal/terminal_travel_instant_tie.iff";
				spawnLoc.y+= 5f;
			}
		}
		
		if (type == SHIP_TYPE_INSTANT_PRIVATEER)
		{
			pickupCraftType = "object/tangible/terminal/terminal_travel_instant_privateer.iff";
		}
		
		if (type == SHIP_TYPE_INSTANT_ROYAL_SHIP)
		{
			pickupCraftType = "object/tangible/terminal/terminal_travel_instant_royal_ship.iff";
		}
		
		obj_id pickupCraft = create.object(pickupCraftType, spawnLoc);
		debugLogging("//***// spawnPickupCraft", "////>>>> spawned the pickup craft. It was obj_id: "+pickupCraft);
		if (!isIdValid(pickupCraft))
		{
			debugLogging("//***// spawnPickupCraft", "////>>>> Apparently, we DIDN'T SPAWN A SHIP!");
			return null;
		}
		
		utils.setScriptVar(player,"instantTravelShip",pickupCraft);
		utils.setScriptVar(pickupCraft,"playerOwner",player);
		
		if (!utils.hasScriptVar(pickupCraft,"playerOwner"))
		{
			debugLogging("//***// spawnPickupCraft", "////>>>> FAILED TO WRITE PLAYER OID TO SHIP");
		}
		
		setObjVar(pickupCraft,"playerOwner",player);
		if (!hasObjVar(pickupCraft,"playerOwner"))
		{
			debugLogging("//***// spawnPickupCraft", "////>>>> FAILED TO WRITE PLAYER OID TO SHIP - OBJVAR");
		}
		
		messageTo (pickupCraft, "initializeInstaTravelShip", null, 1, false);
		
		return pickupCraft;
	}
	
	
	public void debugLogging(String section, String message) throws InterruptedException
	{
		if (CONST_FLAG_DO_LOGGING)
		{
			LOG("travel_terminal", message);
		}
	}
	
}
