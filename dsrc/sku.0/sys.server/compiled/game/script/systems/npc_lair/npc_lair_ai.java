package script.systems.npc_lair;

import script.*;
import script.base_class.*;
import script.combat_engine.*;
import java.util.Arrays;
import java.util.Hashtable;
import java.util.Vector;
import script.base_script;


import script.library.ai_lib;
import script.ai.ai_combat;
import script.ai.ai_combat_movement;
import script.library.pet_lib;
import script.library.healing;
import script.library.utils;
import script.library.factions;



public class npc_lair_ai extends script.theme_park.poi.base
{
	public npc_lair_ai()
	{
	}
	public static final String TERMINAL_LOGGING = "lair_attack";
	public static final boolean LOGGING_ON = true;
	
	
	public int OnLostTarget(obj_id self, obj_id oldTarget) throws InterruptedException
	{
		blog("npc_lair_ai.OnLostTarget - I lost my target.");
		return SCRIPT_CONTINUE;
	}
	
	
	public int OnEnteredCombat(obj_id self) throws InterruptedException
	{
		blog("npc_lair_ai.OnEnteredCombat - I have entered combat.");
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int OnExitedCombat(obj_id self) throws InterruptedException
	{
		blog("npc_lair_ai.OnExitedCombat - I have exited combat.");
		messageTo( self, "handleResumeJob", null, 3, false );
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int OnAttach(obj_id self) throws InterruptedException
	{
		obj_id baseObj = poiGetBaseObject( self );
		if (!isIdValid(baseObj))
		{
			messageTo( self, "handleDestruction", null, 5, false );
			return SCRIPT_CONTINUE;
		}
		if (!isMob(self))
		{
			messageTo( self, "handleDetachment", null, 5, false );
			return SCRIPT_CONTINUE;
		}
		
		listenToMessage( baseObj, "handleNpcLairCustomAi");
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int handleDestruction(obj_id self, dictionary params) throws InterruptedException
	{
		destroyObject( self );
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int OnDestroy(obj_id self) throws InterruptedException
	{
		if (hasScript( self, "ai.pet" ))
		{
			detachScript( self, "systems.npc_lair.npc_lair_ai");
			return SCRIPT_OVERRIDE;
		}
		
		obj_id baseObj = poiGetBaseObject( self );
		if (isIdValid(baseObj))
		{
			stopListeningToMessage(baseObj, "handleNpcLairCustomAi");
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int OnDetach(obj_id self) throws InterruptedException
	{
		obj_id baseObj = poiGetBaseObject( self );
		if (isIdValid(baseObj))
		{
			stopListeningToMessage(baseObj, "handleNpcLairCustomAi");
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int handleDetachment(obj_id self, dictionary params) throws InterruptedException
	{
		detachScript( self, "systems.npc_lair.npc_lair_ai");
		return SCRIPT_CONTINUE;
	}
	
	public static final int JOB_NONE = 0;
	public static final int JOB_LOITER = 1;
	public static final int JOB_SCOUT = 2;
	public static final int JOB_REST = 3;
	public static final int JOB_DEFEND = 4;
	public static final int JOB_HEAL = 5;
	public static final int JOB_CONVERSE = 6;
	public static final int JOB_DANCE = 7;
	
	
	public void doImmediateCommand(obj_id mobile, int job, dictionary params) throws InterruptedException
	{
		blog("npc_lair_ai.doImmediateCommand - init");
		
		switch ( job )
		{
			case JOB_DEFEND :
			obj_id target = params.getObjId( "target");
			respondToScoutAlarm( mobile, target );
			return;
			case JOB_HEAL :
			if (!ai_lib.isMonster(mobile))
			{
				return;
			}
			
			obj_id myTarget = getTarget( mobile );
			if (!isIdValid(myTarget))
			{
				doLairHealing( mobile, params );
			}
			else if (getTarget( myTarget ) != mobile)
			{
				doLairHealing( mobile, params );
			}
		}
		
	}
	
	
	public void respondToScoutAlarm(obj_id mobile, obj_id target) throws InterruptedException
	{
		blog("npc_lair_ai.respondToScoutAlarm - init");
		
		if (!isIdValid(target) || ai_lib.isAiDead( target ))
		{
			return;
		}
		
		if (!ai_lib.isInCombat(mobile))
		{
			
			if (pvpGetAlignedFaction( mobile ) == (221551254))
			{
				
				factions.setFaction( mobile, "angry");
			}
			
			if (getDistance( mobile,target)>60.0f)
			{
				utils.setScriptVar( mobile, "pathingToAttack", target );
				pathTo( mobile, getLocation( target ));
			}
			else
			{
				blog("npc_lair_ai.respondToScoutAlarm - starting combat with target: "+target);
				startCombat( mobile, target );
			}
		}
	}
	
	
	public int handleNpcLairCustomAi(obj_id self, dictionary params) throws InterruptedException
	{
		blog("npc_lair_ai.handleNpcLairCustomAi - init");
		blog("npc_lair_ai.handleNpcLairCustomAi - params: "+params);
		
		if (pet_lib.isPet(self))
		{
			detachScript( self, "systems.npc_lair.npc_lair_ai");
			return SCRIPT_CONTINUE;
		}
		
		if (ai_lib.isAiDead(self))
		{
			detachScript( self, "systems.npc_lair.npc_lair_ai");
			return SCRIPT_CONTINUE;
		}
		
		int jobToDo = params.getInt( "job");
		if (jobToDo > 0)
		{
			
			doImmediateCommand( self, jobToDo, params );
			return SCRIPT_CONTINUE;
		}
		
		blog("npc_lair_ai.handleNpcLairCustomAi - no immediate command: ");
		
		if (!hasObjVar( self, "npc_lair.mobileNumber"))
		{
			return SCRIPT_CONTINUE;
		}
		
		obj_id baseObj = params.getObjId( "baseObj");
		if (!isIdValid( baseObj ))
		{
			detachScript( self, "systems.npc_lair.npc_lair_ai");
			return SCRIPT_CONTINUE;
		}
		if (!exists( baseObj ) || !isInWorld( baseObj ))
		{
			detachScript( self, "systems.npc_lair.npc_lair_ai");
			return SCRIPT_CONTINUE;
		}
		
		if (ai_lib.isHerdingCreature( self ))
		{
			obj_id myLair = ai_lib.getLair( self );
			if (!isIdValid( myLair ))
			{
				detachScript( self, "systems.npc_lair.npc_lair_ai");
				return SCRIPT_CONTINUE;
			}
		}
		
		int mobileNum = getIntObjVar( self, "npc_lair.mobileNumber");
		blog("npc_lair_ai.handleNpcLairCustomAi - mobileNum: "+mobileNum);
		
		int[] jobAssignment = params.getIntArray( "jobAssignments");
		blog("npc_lair_ai.handleNpcLairCustomAi - jobAssignment.length: "+jobAssignment.length);
		for (int i = 0; i < jobAssignment.length; i++)
		{
			testAbortScript();
			blog("npc_lair_ai.handleNpcLairCustomAi - jobAssignment["+i+"]: "+jobAssignment[i]);
		}
		
		if (jobAssignment == null || jobAssignment.length == 0)
		{
			blog("npc_lair_ai.handleNpcLairCustomAi - jobAssignment == null or length == 0");
			
			detachScript( self, "systems.npc_lair.npc_lair_ai");
			return SCRIPT_CONTINUE;
		}
		if (mobileNum >= jobAssignment.length)
		{
			blog("npc_lair_ai.handleNpcLairCustomAi - mobileNum >= jobAssignment.length");
			
			detachScript( self, "systems.npc_lair.npc_lair_ai");
			return SCRIPT_CONTINUE;
		}
		
		blog("npc_lair_ai.handleNpcLairCustomAi - mobileNum: "+mobileNum);
		
		int job = jobAssignment[mobileNum ];
		blog("npc_lair_ai.handleNpcLairCustomAi - job: "+job);
		
		setObjVar( self, "npc_lair.job", job );
		switch ( job )
		{
			case JOB_NONE :
			
			break;
			case JOB_LOITER :
			setHomeLocation( self, getLocation( baseObj ));
			ai_lib.setDefaultCalmBehavior( self, ai_lib.BEHAVIOR_LOITER );
			break;
			case JOB_SCOUT :
			setHomeLocation( self, getLocation( baseObj ));
			ai_lib.setDefaultCalmBehavior( self, ai_lib.BEHAVIOR_WANDER );
			
			String myName = getAssignedName( self );
			if (myName != null && !myName.equals(""))
			{
				if (!myName.equals("null"))
				{
					setName( self, myName + " (scout)");
				}
				else
				{
					String crName = ai_lib.getCreatureName( self );
					debugServerConsoleMsg( self, "WARNING: "+ crName + " has an assigned name of Null - why is that?");
				}
			}
			else
			{
				if (!myName.equals("null"))
				{
					setName( self, (getString( getNameStringId(self) ) + " (scout)"));
				}
				else
				{
					String crName = ai_lib.getCreatureName( self );
					debugServerConsoleMsg( self, "WARNING: "+ crName + " has an assigned name of Null - why is that?");
				}
			}
			break;
			case JOB_REST :
			if (ai_lib.isMonster(self))
			{
				if (ai_lib.canSit( self ))
				{
					setHomeLocationNearLair( self, baseObj );
					ai_lib.setDefaultCalmBehavior( self, ai_lib.BEHAVIOR_SENTINEL );
				}
				else if (ai_lib.canLieDown(self))
				{
					setHomeLocationNearLair( self, baseObj );
					ai_lib.setDefaultCalmBehavior( self, ai_lib.BEHAVIOR_SENTINEL );
				}
				else
				{
					
					setHomeLocation( self, getLocation( baseObj ));
					ai_lib.setDefaultCalmBehavior( self, ai_lib.BEHAVIOR_LOITER );
				}
			}
			else if (ai_lib.aiGetNiche(self)==NICHE_NPC)
			{
				location danceLoc = getLocationObjVar( baseObj, "npc_lair.danceLoc");
				obj_id dancer = getObjIdObjVar( baseObj, "npc_lair.dancer");
				if (isIdValid( dancer ))
				{
					setObjVar( self, "npc_lair.dancer", dancer );
				}
				
				setObjVar( baseObj, "npc_lair.watcher", self );
				boolean entertainment = getBooleanObjVar( baseObj, "npc_lair.entertainment");
				location startLoc = getLocation(self);
				if (startLoc.x > danceLoc.x)
				{
					danceLoc.x += (rand(2,3));
				}
				else
				{
					danceLoc.x -= (rand(2,3));
				}
				
				if (startLoc.z > danceLoc.z)
				{
					danceLoc.z += (rand(2,3));
				}
				else
				{
					danceLoc.z -= (rand(2,3));
				}
				
				setHomeLocation( self, danceLoc );
				ai_lib.setDefaultCalmBehavior( self, ai_lib.BEHAVIOR_SENTINEL );
				{
					switch ( rand(1,10) )
					{
						case 1 :
						ai_lib.setDefaultCalmMood( self, "conversation");
						break;
						case 2 :
						ai_lib.setDefaultCalmMood( self, "npc_accusing");
						break;
						case 3 :
						ai_lib.setDefaultCalmMood( self, "npc_angry");
						break;
						case 4 :
						ai_lib.setDefaultCalmMood( self, "npc_consoling");
						break;
						case 5 :
						ai_lib.setDefaultCalmMood( self, "npc_sad");
						break;
						case 6 :
						case 7 :
						case 8 :
						case 9 :
						case 10 :
						ai_lib.setDefaultCalmMood( self, "npc_sitting_ground");
						break;
					}
				}
			}
			break;
			case JOB_CONVERSE :
			ai_lib.setDefaultCalmBehavior( self, ai_lib.BEHAVIOR_SENTINEL );
			if (mobileNum != 0)
			{
				
				return SCRIPT_CONTINUE;
			}
			obj_id[] allNpcs = getAllNpcs(getLocation(self), 45.0f);
			if (allNpcs == null || allNpcs.length == 0)
			{
				
				return SCRIPT_CONTINUE;
			}
			for (int i = 0; i < allNpcs.length; i++)
			{
				testAbortScript();
				if (allNpcs[i] != self)
				{
					int yourNum = getIntObjVar( allNpcs[i], "npc_lair.mobileNumber");
					if (yourNum == 1)
					{
						
						faceToBehavior( self, allNpcs[i] );
						ai_lib.setDefaultCalmMood( self, "conversation");
						dictionary parms = new dictionary();
						parms.put("converseWith", self);
						setObjVar( self, "npc_lair.converseWith", allNpcs[i] );
						messageTo( allNpcs[i], "handleRequestToConverse", parms, 5, false );
						return SCRIPT_CONTINUE;
					}
				}
			}
			
			break;
			case JOB_DANCE :
			location danceLoc = getLocationObjVar( baseObj, "npc_lair.danceLoc");
			setObjVar( baseObj, "npc_lair.dancer", self );
			danceLoc.x += rand(-2,2);
			danceLoc.z += rand(-2,2);
			setHomeLocation( self, danceLoc );
			ai_lib.setDefaultCalmBehavior( self, ai_lib.BEHAVIOR_SENTINEL );
			switch ( rand(1,6) )
			{
				case 1 :
				ai_lib.setDefaultCalmMood( self, "conversation");
				break;
				case 2 :
				case 3 :
				case 4 :
				if (getGender(self)==GENDER_FEMALE)
				{
					ai_lib.setDefaultCalmMood( self, "themepark_oola");
				}
				else
				{
					ai_lib.setDefaultCalmMood( self, "entertained");
				}
				
				setObjVar( baseObj, "npc_lair.entertainment", true );
				break;
				case 5 :
				ai_lib.setDefaultCalmMood( self, "npc_accusing");
				break;
				case 6 :
				ai_lib.setDefaultCalmMood( self, "npc_angry");
				break;
			}
			break;
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int OnMovePathComplete(obj_id self) throws InterruptedException
	{
		int job = getIntObjVar( self, "npc_lair.job");
		
		if (ai_lib.isInCombat( self ) && (getIntObjVar( self, "ai.combat.moveMode")==ai_combat_movement.MOVEMODE_PATHING_HOME))
		{
			
			if (job == JOB_SCOUT)
			{
				
				obj_id poiObj = poiGetBaseObject( self );
				if (isIdValid(poiObj))
				{
					obj_id myTarget = getTarget(self);
					if (isIdValid(myTarget))
					{
						
						dictionary parms = new dictionary();
						parms.put( "target", myTarget );
						messageTo(poiObj,"handleScoutAlarm", parms, 0, false);
					}
				}
			}
			return SCRIPT_CONTINUE;
		}
		
		if (utils.hasScriptVar( self, "pathingToHealLair" ))
		{
			utils.removeScriptVar( self, "pathingToHealLair");
			
			doLairHealing( self, null );
			return SCRIPT_OVERRIDE;
		}
		
		if (utils.hasScriptVar( self, "pathingToAttack" ))
		{
			obj_id target = utils.getObjIdScriptVar( self, "pathingToAttack");
			if (isIdValid( target ) && exists( target ) && !ai_lib.aiIsDead( target ))
			{
				if (getDistance( self, target ) > 60.0f)
				{
					pathTo( self, getLocation( target ) );
					return SCRIPT_CONTINUE;
				}
				else
				{
					startCombat( self, target );
				}
			}
			utils.removeScriptVar( self, "pathingToAttack");
		}
		
		if (getBehavior(self) != BEHAVIOR_CALM)
		{
			return SCRIPT_CONTINUE;
		}
		
		if (ai_lib.isMonster( self ))
		{
			if (job == JOB_REST)
			{
				if (ai_lib.canSit( self ))
				{
					if (rand(1,2)==1)
					{
						ai_lib.aiSetPosture(self, POSTURE_SITTING );
					}
					else
					{
						ai_lib.aiSetPosture(self, POSTURE_LYING_DOWN );
					}
				}
				else if (ai_lib.canLieDown(self))
				{
					ai_lib.aiSetPosture(self, POSTURE_LYING_DOWN );
				}
				return SCRIPT_OVERRIDE;
			}
		}
		
		if (job == JOB_CONVERSE)
		{
			utils.removeScriptVar( self, "npc_lair.pathingToConverse");
			obj_id partner = getObjIdObjVar( self, "npc_lair.converseWith");
			if (isIdValid( partner ) && exists( partner ))
			{
				faceToBehavior( self, partner );
				ai_lib.setDefaultCalmMood( self, "conversation");
				setHomeLocation( self, getLocation( self ));
			}
		}
		else if (hasObjVar( self, "npc_lair.dancer" ))
		{
			obj_id dancer = getObjIdObjVar( self, "npc_lair.dancer");
			if (isIdValid(dancer) && exists( dancer ))
			{
				faceToBehavior( self, dancer );
				
			}
			else
			{
				
				obj_id[] creatures = getCreaturesInRange( getLocation( self ), 6.0f );
				if (creatures != null && creatures.length > 0)
				{
					for (int i = 0; i < creatures.length; i++)
					{
						testAbortScript();
						int yourJob = getIntObjVar( creatures[i], "npc_lair.job");
						if (yourJob == JOB_DANCE)
						{
							faceToBehavior( self, creatures[i] );
							faceToBehavior( creatures[i], self );
							return SCRIPT_CONTINUE;
						}
					}
				}
			}
		}
		else if (job == JOB_DANCE)
		{
			obj_id poiObj = poiGetBaseObject( self );
			if (isIdValid(poiObj))
			{
				obj_id watcher = getObjIdObjVar( poiObj, "npc_lair.watcher");
				if (isIdValid( watcher ) && exists( watcher ))
				{
					faceToBehavior( self, watcher );
					
				}
				else
				{
					
					obj_id[] creatures = getCreaturesInRange( getLocation( self), 6.0f );
					if (creatures != null && creatures.length > 0)
					{
						for (int i = 0; i < creatures.length; i++)
						{
							testAbortScript();
							if (hasObjVar( creatures[i], "npc_lair.dancer" ))
							{
								faceToBehavior( self, creatures[i] );
								faceToBehavior( creatures[i], self );
								return SCRIPT_CONTINUE;
							}
						}
					}
				}
			}
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int OnMovePathBlocked(obj_id self) throws InterruptedException
	{
		if (utils.hasScriptVar( self, "pathingToHealLair" ))
		{
			
			utils.removeScriptVar( self, "pathingToHealLair");
			removeObjVar( self, "ai.combat.ignoreCombat");
			utils.removeScriptVar( self, "npc_lair.willheal");
			return SCRIPT_OVERRIDE;
		}
		return SCRIPT_CONTINUE;
	}
	
	
	public int OnMovePathNotFound(obj_id self) throws InterruptedException
	{
		if (utils.hasScriptVar( self, "pathingToHealLair" ))
		{
			
			utils.removeScriptVar( self, "pathingToHealLair");
			removeObjVar( self, "ai.combat.ignoreCombat");
			utils.removeScriptVar( self, "npc_lair.willheal");
			return SCRIPT_OVERRIDE;
		}
		return SCRIPT_CONTINUE;
	}
	
	
	public void setHomeLocationNearLair(obj_id mobile, obj_id baseObj) throws InterruptedException
	{
		if (!isIdValid( baseObj ))
		{
			return;
		}
		
		location baseLoc = getLocation( baseObj );
		if ((baseLoc.x < 1.0f && baseLoc.x > -1.0f ) && (baseLoc.z < 1.0f && baseLoc.z > 1.0f))
		{
			return;
		}
		
		baseLoc.x += rand(7,7);
		baseLoc.z += rand(7,7);
		setHomeLocation(mobile,baseLoc);
	}
	
	
	public void doLairHealing(obj_id mobile, dictionary params) throws InterruptedException
	{
		
		blog("npc_lair_ai.doLairHealing - init");
		if (!isIdValid( mobile ) || !exists( mobile ))
		{
			return;
		}
		
		if (ai_lib.isAiDead( mobile ))
		{
			return;
		}
		
		if (!utils.hasScriptVar( mobile,"npc_lair.willheal"))
		{
			if (rand(1,3)==1)
			{
				utils.setScriptVar( mobile, "npc_lair.willheal", true );
			}
			else
			{
				return;
			}
		}
		
		obj_id lair = getObjIdObjVar(mobile,"npc_lair.target");
		if (!isIdValid(lair))
		{
			if (params != null)
			{
				lair = params.getObjId( "lair");
				if (isIdValid(lair) && exists( lair ))
				{
					setObjVar( mobile, "npc_lair.target", lair );
				}
				else
				{
					removeObjVar( mobile, "ai.combat.ignoreCombat");
					utils.setScriptVar( mobile, "npc_lair.willheal", false );
					return;
				}
			}
			else
			{
				return;
			}
		}
		
		if (!exists(lair) || !isInWorld(lair))
		{
			removeObjVar( mobile, "ai.combat.ignoreCombat");
			utils.setScriptVar( mobile, "npc_lair.willheal", false );
			return;
		}
		
		int maxHP = getMaxHitpoints( lair );
		int curHP = getHitpoints(lair);
		if (curHP == maxHP)
		{
			
			removeObjVar( mobile, "ai.combat.ignoreCombat");
			utils.removeScriptVar( mobile,"npc_lair.willheal");
			if (!ai_lib.isInCombat(mobile))
			{
				messageTo( mobile, "resumeDefaultCalmBehavior", null, rand(3,5), false );
			}
			return;
		}
		
		int amountToHeal = (maxHP/5);
		
		location baseLoc = getLocation(lair);
		
		if (getDistance( lair, mobile ) > 3.0f)
		{
			
			setObjVar( mobile, "ai.combat.ignoreCombat", true );
			utils.setScriptVar( mobile, "pathingToHealLair", true );
			stop( mobile );
			setMovementRun( mobile );
			pathToWithinTwoMetersOf( mobile, baseLoc );
			return;
		}
		
		switch ( rand(0,3) )
		{
			case 0 :
			debugSpeakMsgc(aiLoggingEnabled(mobile), mobile, "eat");
			
			stop( mobile );
			doAnimationAction( mobile, "eat");
			break;
			case 1 :
			debugSpeakMsgc(aiLoggingEnabled(mobile), mobile, "vocalize");
			
			stop( mobile );
			doAnimationAction( mobile, "vocalize");
			break;
			case 2 :
			debugSpeakMsgc(aiLoggingEnabled(mobile), mobile, "loiter");
			
			final location anchorLocation = getLocation(lair);
			final float minDistance = 1.0f;
			final float maxDistance = 3.0f;
			final float minDelay = 1.0f;
			final float maxDelay = 2.0f;
			
			loiterLocation(mobile, anchorLocation, minDistance, maxDistance, minDelay, maxDelay);
			break;
		}
		
		obj_id[] players = getAllPlayers(baseLoc, healing.VAR_EFFECT_DISPLAY_RADIUS);
		if (players != null)
		{
			for (int i = 0; i < players.length; i++)
			{
				testAbortScript();
				playClientEffectObj(players[i], "clienteffect/healing_healdamage.cef", lair, "");
				playClientEffectObj(players[i], "clienteffect/healing_healdamage.cef", mobile, "");
			}
		}
		
		curHP += amountToHeal;
		if (curHP < maxHP)
		{
			setHitpoints(lair,curHP);
		}
		else
		{
			setHitpoints(lair,maxHP);
		}
		
		dictionary parms = new dictionary();
		parms.put( "lair", lair );
		messageTo( mobile, "handleRecoverFromHealingLair", parms, rand(3,5), false );
		
	}
	
	
	public int resumeDefaultCalmBehavior(obj_id self, dictionary params) throws InterruptedException
	{
		obj_id lair = getObjIdObjVar( self, "npc_lair.target");
		if (isIdValid(lair))
		{
			if (!ai_lib.isInCombat(self))
			{
				int maxHP = getMaxHitpoints( lair );
				int curHP = getHitpoints(lair);
				if (curHP < maxHP)
				{
					params.put("lair",lair);
					doLairHealing( self, params );
				}
				
				return SCRIPT_OVERRIDE;
			}
		}
		return SCRIPT_CONTINUE;
	}
	
	
	public int handleRecoverFromHealingLair(obj_id self, dictionary params) throws InterruptedException
	{
		
		obj_id lair = params.getObjId( "lair");
		int maxHP = getMaxHitpoints( lair );
		int curHP = getHitpoints(lair);
		if (curHP < maxHP)
		{
			doLairHealing( self, params );
		}
		else
		{
			
			removeObjVar( self, "ai.combat.ignoreCombat");
			utils.removeScriptVar( self, "pathingToHealLair");
			utils.removeScriptVar( self, "npc_lair.willheal");
			if (!ai_lib.isInCombat(self))
			{
				messageTo( self, "resumeDefaultCalmBehavior", null, 5, false );
			}
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public void pathToWithinTwoMetersOf(obj_id mobile, location destLoc) throws InterruptedException
	{
		location startLoc = getLocation( mobile );
		location baseLoc = new location( destLoc );
		if (startLoc.x > baseLoc.x)
		{
			baseLoc.x += 1.5f;
		}
		else
		{
			baseLoc.x -= 1.5f;
		}
		
		if (startLoc.z > baseLoc.z)
		{
			baseLoc.z += 1.5f;
		}
		else
		{
			baseLoc.z -= 1.5f;
		}
		
		pathTo( mobile, baseLoc );
	}
	
	
	public int OnSawAttack(obj_id self, obj_id defender, obj_id[] attackers) throws InterruptedException
	{
		if (hasObjVar( self, "ai.combat.ignoreCombat" ))
		{
			
			return SCRIPT_OVERRIDE;
		}
		else
		{
			return SCRIPT_CONTINUE;
		}
	}
	
	
	public int OnCreatureDamaged(obj_id self, obj_id attacker, obj_id weapon, int[] damage) throws InterruptedException
	{
		if (hasObjVar( self, "ai.combat.ignoreCombat" ))
		{
			
			obj_id lair = getObjIdObjVar( self, "npc_lair.target");
			utils.removeScriptVar( self, "npc_lair.willheal");
			removeObjVar( self, "ai.combat.ignoreCombat");
		}
		return SCRIPT_CONTINUE;
	}
	
	
	public int handleRequestToConverse(obj_id self, dictionary params) throws InterruptedException
	{
		
		obj_id partner = params.getObjId("converseWith");
		if (!isIdValid(partner) || !exists( partner ))
		{
			
			return SCRIPT_CONTINUE;
		}
		
		setObjVar( self, "npc_lair.converseWith", partner );
		utils.setScriptVar( self, "npc_lair.pathingToConverse", true );
		location destLoc = new location( getLocation( partner ));
		location myLoc = getLocation( self );
		if (myLoc.x < destLoc.x)
		{
			destLoc.x -= 1;
		}
		else
		{
			destLoc.x += 1;
		}
		if (myLoc.z < destLoc.z)
		{
			destLoc.z -= 1;
		}
		else
		{
			destLoc.z += 1;
		}
		
		pathTo( self, destLoc);
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int handleResumeJob(obj_id self, dictionary params) throws InterruptedException
	{
		
		return SCRIPT_CONTINUE;
	}
	
	
	public boolean blog(String msg) throws InterruptedException
	{
		if (msg == null || msg.equals(""))
		{
			return false;
		}
		if (LOGGING_ON)
		{
			LOG(TERMINAL_LOGGING, msg);
		}
		
		return true;
	}
	
}
