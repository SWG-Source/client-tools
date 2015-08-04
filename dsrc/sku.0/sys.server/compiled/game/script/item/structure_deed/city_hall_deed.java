package script.item.structure_deed;

import script.*;
import script.base_class.*;
import script.combat_engine.*;
import java.util.Arrays;
import java.util.Hashtable;
import java.util.Vector;
import script.base_script;


import script.library.player_structure;
import script.library.utils;
import script.library.sui;
import script.library.city;



public class city_hall_deed extends script.item.structure_deed.player_structure_deed
{
	public city_hall_deed()
	{
	}
	public static final string_id SID_OBSCENE = new string_id("player_structure", "obscene");
	public static final string_id SID_NOT_UNIQUE = new string_id("player_structure", "cityname_not_unique");
	public static final string_id SID_ALREADY_MAYOR = new string_id("city/city", "already_mayor");
	
	
	public int handleNoReclaimConfirm(obj_id self, dictionary params) throws InterruptedException
	{
		utils.removeScriptVarTree(self, "noreclaim");
		
		if (params == null || params.isEmpty())
		{
			return SCRIPT_OVERRIDE;
		}
		
		int bp = sui.getIntButtonPressed(params);
		if (bp == sui.BP_CANCEL)
		{
			return SCRIPT_OVERRIDE;
		}
		
		obj_id player = sui.getPlayerId(params);
		if (!isIdValid(player))
		{
			return SCRIPT_OVERRIDE;
		}
		
		if (!utils.isNestedWithin(self, player))
		{
			return SCRIPT_OVERRIDE;
		}
		
		location here = getLocation(player);
		String template = player_structure.getDeedTemplate(self);
		
		if (!player_structure.canPlaceStructure(player, template, here, self))
		{
			return SCRIPT_OVERRIDE;
		}
		
		LOG("structure","city_hall_deed");
		
		if (city.isAMayor( player ))
		{
			sendSystemMessage( player, SID_ALREADY_MAYOR );
			return SCRIPT_OVERRIDE;
		}
		
		sui.inputbox( self, player, "@city/city:city_name_d", sui.OK_CANCEL, "@city/city:city_name_t", sui.INPUT_NORMAL, null, "handleSetCityName", null );
		
		return SCRIPT_OVERRIDE;
	}
	
	
	public int handleSetCityName(obj_id self, dictionary params) throws InterruptedException
	{
		
		obj_id player = sui.getPlayerId(params);
		String cityName = sui.getInputBoxText( params );
		int btn = sui.getIntButtonPressed( params );
		if (btn == sui.BP_CANCEL)
		{
			return SCRIPT_CONTINUE;
		}
		
		if ((cityName.equals("")) || isNameReserved(cityName))
		{
			
			sendSystemMessage( player, SID_OBSCENE );
			sui.inputbox( self, player, "@city/city:city_name_d", sui.OK_CANCEL, "@city/city:city_name_t", sui.INPUT_NORMAL, null, "handleSetCityName", null );
			return SCRIPT_CONTINUE;
		}
		
		if (cityName.length() > 24)
		{
			cityName = cityName.substring( 0, 23 );
		}
		
		if (!city.isUniqueCityName( cityName ))
		{
			
			sendSystemMessage( player, SID_NOT_UNIQUE );
			sui.inputbox( self, player, "@city/city:city_name_d", sui.OK_CANCEL, "@city/city:city_name_t", sui.INPUT_NORMAL, null, "handleSetCityName", null );
			return SCRIPT_CONTINUE;
		}
		
		setObjVar( self, "cityName", cityName );
		
		queueCommand( player, (123886506), self, "", COMMAND_PRIORITY_DEFAULT );
		
		return SCRIPT_CONTINUE;
	}
}
