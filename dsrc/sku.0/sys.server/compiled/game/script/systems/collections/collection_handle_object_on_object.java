package script.systems.collections;

import script.*;
import script.base_class.*;
import script.combat_engine.*;
import java.util.Arrays;
import java.util.Hashtable;
import java.util.Vector;
import script.base_script;


import script.library.utils;
import script.library.static_item;
import script.library.buff;
import script.library.collection;
import script.library.sui;
import script.library.utils;


public class collection_handle_object_on_object extends script.base_script
{
	public collection_handle_object_on_object()
	{
	}
	
	public int incapaciteSelf(obj_id self, dictionary params) throws InterruptedException
	{
		setPosture(self, POSTURE_INCAPACITATED);
		return SCRIPT_CONTINUE;
	}
}
