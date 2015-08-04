package script.systems.crafting.furniture;

import script.*;
import script.base_class.*;
import script.combat_engine.*;
import java.util.Arrays;
import java.util.Hashtable;
import java.util.Vector;
import script.base_script;


import script.library.utils;
import script.library.craftinglib;



public class crafting_base_furniture_customization extends script.systems.crafting.crafting_base
{
	public crafting_base_furniture_customization()
	{
	}
	public static final String VERSION = "v1.00.00";
	
	
	public void calcAndSetPrototypeProperties(obj_id prototype, draft_schematic.attribute[] itemAttributes, dictionary craftingValuesDictionary) throws InterruptedException
	{
		
		for (int i = 0; i < itemAttributes.length; ++i)
		{
			testAbortScript();
			if (itemAttributes[i] == null)
			{
				continue;
			}
			
			if (itemAttributes[i].name.getAsciiId().equals("charges"))
			{
				itemAttributes[i].currentValue = (itemAttributes[i].minValue + itemAttributes[i].maxValue) + itemAttributes[i].currentValue;
			}
		}
		
		super.calcAndSetPrototypeProperties ( prototype, itemAttributes, craftingValuesDictionary );
	}
	
	
	public void calcAndSetPrototypeProperties(obj_id prototype, draft_schematic.attribute[] itemAttributes) throws InterruptedException
	{
		
		debugServerConsoleMsg(null, "Beginning assembly-phase prototype property setting");
		for (int i = 0; i < itemAttributes.length; ++i)
		{
			testAbortScript();
			if (itemAttributes[i] == null)
			{
				continue;
			}
			
			if (!calcAndSetPrototypeProperty ( prototype, itemAttributes[i]))
			{
				
				setObjVar (prototype, craftinglib.COMPONENT_ATTRIBUTE_OBJVAR_NAME + "."+ itemAttributes[i].name.getAsciiId(),
				(int)itemAttributes[i].currentValue);
				
				setCount(prototype, (int)itemAttributes[i].currentValue);
			}
		}
	}
	
}
