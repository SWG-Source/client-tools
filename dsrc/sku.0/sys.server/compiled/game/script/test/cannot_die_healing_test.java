package script.test;

import script.*;
import script.base_class.*;
import script.combat_engine.*;
import java.util.Arrays;
import java.util.Hashtable;
import java.util.Vector;
import script.base_script;


public class cannot_die_healing_test extends script.base_script
{
	public cannot_die_healing_test()
	{
	}
	
	public int OnCreatureDamaged(obj_id self, obj_id attacker, obj_id weapon, int[] damage) throws InterruptedException
	{
		setAttrib(self, HEALTH, 100);
		return SCRIPT_CONTINUE;
	}
	
	
	public int OnObjectDamaged(obj_id self, obj_id attacker, obj_id weapon, int damage) throws InterruptedException
	{
		setHitpoints(self, 100);
		return SCRIPT_CONTINUE;
	}
	
	
	public int OnIncapacitated(obj_id self, obj_id killer) throws InterruptedException
	{
		return SCRIPT_OVERRIDE;
	}
	
}
