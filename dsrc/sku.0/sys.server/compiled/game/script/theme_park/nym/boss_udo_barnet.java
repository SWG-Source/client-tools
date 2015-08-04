package script.theme_park.nym;

import script.*;
import script.base_class.*;
import script.combat_engine.*;
import java.util.Arrays;
import java.util.Hashtable;
import java.util.Vector;
import script.base_script;

import script.library.ai_lib;
import script.library.chat;
import script.library.hue;
import script.library.trial;
import script.library.utils;


public class boss_udo_barnet extends script.base_script
{
	public boss_udo_barnet()
	{
	}
	
	public void findNextTarget(obj_id self) throws InterruptedException
	{
		if (ai_lib.isDead(self))
		{
			return;
		}
		
		obj_id dungeon = getTopMostContainer(self);
		obj_id target = null;
		
		if (!isIdValid(dungeon))
		{
			
			return;
		}
		
		obj_id[] targets = trial.getNonStealthedTargetsInCell(dungeon, "r1");
		
		if (targets == null || targets.length <= 0)
		{
			dictionary sessionDict = new dictionary();
			
			messageTo(dungeon, "ig88_failed", sessionDict, 0, false);
			return;
		}
		
		target = trial.getClosest(self, targets);
		
		if (!isIdValid(target))
		{
			return;
		}
		
		int lastShieldTime = utils.getIntScriptVar(self, "lastShieldTime");
		
		if (getGameTime() - lastShieldTime > 120)
		{
			utils.setScriptVar(self, "lastShieldTime", getGameTime());
			queueCommand(self, (-2086643447), self, "", COMMAND_PRIORITY_DEFAULT);
		}
		
		setHate(self, target, 5000);
	}
	
	
	public int findTarget(obj_id self, dictionary params) throws InterruptedException
	{
		if (!isIdValid(self))
		{
			return SCRIPT_CONTINUE;
		}
		
		findNextTarget(self);
		
		dictionary sessionDict = new dictionary();
		
		messageTo(self, "findTarget", sessionDict, 1, false);
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int OnExitedCombat(obj_id self) throws InterruptedException
	{
		if (!isIdValid(self))
		{
			return SCRIPT_CONTINUE;
		}
		
		findNextTarget(self);
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int OnAttach(obj_id self) throws InterruptedException
	{
		if (!isIdValid(self))
		{
			return SCRIPT_CONTINUE;
		}
		
		obj_id dungeon = getTopMostContainer(self);
		
		if (!isIdValid(dungeon))
		{
			return SCRIPT_CONTINUE;
		}
		
		dictionary sessionDict = new dictionary();
		
		messageTo(self, "colorize", sessionDict, 0.25f, false);
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int colorize(obj_id self, dictionary params) throws InterruptedException
	{
		if (!isIdValid(self))
		{
			return SCRIPT_CONTINUE;
		}
		
		hue.setColor(self, hue.INDEX_1, 7);
		
		return SCRIPT_CONTINUE;
	}
}
