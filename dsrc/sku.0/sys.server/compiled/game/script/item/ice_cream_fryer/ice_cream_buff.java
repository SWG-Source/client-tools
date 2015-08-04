package script.item.ice_cream_fryer;

import script.*;
import script.base_class.*;
import script.combat_engine.*;
import java.util.Arrays;
import java.util.Hashtable;
import java.util.Vector;
import script.base_script;

import script.library.buff;
import script.library.collection;
import script.library.utils;


public class ice_cream_buff extends script.base_script
{
	public ice_cream_buff()
	{
	}
	public static final String ALREADY_SETUP = "itemFoodScriptRemoved";
	public static final String ITEM_FOOD_SCRIPT = "item.food";
	public static final String OWNER_OID = "owner";
	
	
	public int OnAttach(obj_id self) throws InterruptedException
	{
		
		if (!hasObjVar(self, ALREADY_SETUP))
		{
			blog("OnAttach - NO OBJVAR FOUND");
			
			messageTo(self, "removeItemFoodScript", null, 2, false);
			setObjVar(self, "ALREADY_SETUP", true);
		}
		return SCRIPT_CONTINUE;
	}
	
	
	public int OnGetAttributes(obj_id self, obj_id player, String[] names, String[] attribs) throws InterruptedException
	{
		int iceCreamCategory = collection.ICE_CREAM_ALL_PLAYERS;
		if (hasObjVar(self, collection.FRYER_BUFF_TYPE))
		{
			iceCreamCategory = utils.getIntObjVar(self, collection.FRYER_BUFF_TYPE);
		}
		
		int idx = utils.getValidAttributeIndex(names);
		if (idx == -1)
		{
			return super.OnGetAttributes(self, player, names, attribs);
		}
		
		if (iceCreamCategory != collection.ICE_CREAM_DOMESTICS_ONLY)
		{
			
			names[idx] = "proc_name";
			attribs[idx] = "<UNKNOWN>";
			idx++;
		}
		
		names[idx] = "nonspecial_ingredient";
		attribs[idx] = "Cereal Flakes, Exceptional Milk, Exceptional Egg";
		idx++;
		if (hasObjVar(self, "special_ingredient"))
		{
			String specialIngredient = getStringObjVar(self, "special_ingredient");
			string_id templateName = getNameFromTemplate(specialIngredient);
			names[idx] = "special_ingredient";
			attribs[idx] = "@"+templateName;
			idx++;
		}
		return super.OnGetAttributes(self, player, names, attribs);
	}
	
	
	public int removeItemFoodScript(obj_id self, dictionary params) throws InterruptedException
	{
		detachScript(self, ITEM_FOOD_SCRIPT);
		return SCRIPT_CONTINUE;
	}
	
	
	public boolean blog(String message) throws InterruptedException
	{
		
		return true;
	}
}
