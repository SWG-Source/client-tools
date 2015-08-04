package script.space.quest_logic;

import script.*;
import script.base_class.*;
import script.combat_engine.*;
import java.util.Arrays;
import java.util.Hashtable;
import java.util.Vector;
import script.base_script;


import script.library.space_create;
import script.library.space_quest;
import script.library.space_utils;
import script.library.space_transition;
import script.library.utils;
import script.library.ship_ai;
import script.library.prose;
import script.library.money;



public class assassinate extends script.space.quest_logic.recovery
{
	public assassinate()
	{
	}
	public static final String SOUND_SPAWN_ENEMY = "clienteffect/ui_quest_spawn_enemy.cef";
	public static final String SOUND_DESTROYED_ALL = "clienteffect/ui_quest_destroyed_all.cef";
	public static final string_id WARPOUT_FAILURE = new string_id( "space/quest", "warpout_failure");
	
	
	public int OnAttach(obj_id self) throws InterruptedException
	{
		
		String questName = getStringObjVar( self, space_quest.QUEST_NAME );
		String questType = getStringObjVar( self, space_quest.QUEST_TYPE );
		obj_id player = getObjIdObjVar( self, space_quest.QUEST_OWNER );
		if ((questName == null) || (questType == null))
		{
			
			return SCRIPT_OVERRIDE;
		}
		
		String qTable = "datatables/spacequest/"+questType+"/"+questName+".iff";
		dictionary questInfo = dataTableGetRow( qTable, 0 );
		if (questInfo == null)
		{
			
			sendSystemMessageTestingOnly( player, "Debug: Failed to open quest table "+qTable );
			return SCRIPT_OVERRIDE;
		}
		
		setObjVar( self, "targetArrivalDelay", questInfo.getInt("targetArrivalDelay") );
		
		String targetShip = questInfo.getString("targetShipType");
		if (targetShip != null)
		{
			setObjVar( self, "targetShipType", targetShip );
		}
		
		String[] escortShips = dataTableGetStringColumn( qTable, "targetEscort");
		space_quest.cleanArray( self, "escortShips", escortShips );
		setObjVar( self, "mustDestroyEscort", questInfo.getInt( "mustDestroyEscort") );
		
		setObjVar( self, "targetAttackMode", questInfo.getInt("targetAttackMode") );
		
		setObjVar( self, "numResponses", questInfo.getInt( "numResponses") );
		
		String[] escortPath = dataTableGetStringColumn( qTable, "escortPath");
		space_quest.cleanArray( self, "escortPath", escortPath );
		
		String questZone = getStringObjVar( self, space_quest.QUEST_ZONE );
		if (getCurrentSceneName().startsWith( questZone ))
		{
			dictionary outparams = new dictionary();
			outparams.put( "player", player );
			messageTo( self, "initializedQuestPlayer", outparams, 1.f, false );
		}
		
		int questid = questGetQuestId( "spacequest/"+questType+"/"+questName );
		if (questid != 0)
		{
			questActivateTask( questid, 0, player );
			if (questInfo.getInt( "mustDestroyEscort" ) == 1)
			{
				questActivateTask( questid, 2, player );
			}
			
			setObjVar( self, "taskId", 1 );
			questActivateTask( questid, 1, player );
		}
		
		return SCRIPT_OVERRIDE;
	}
	
	
	public int warpInTarget(obj_id self, dictionary params) throws InterruptedException
	{
		obj_id player = getObjIdObjVar( self, space_quest.QUEST_OWNER );
		
		String questZone = getStringObjVar( self, space_quest.QUEST_ZONE );
		if (!getCurrentSceneName().startsWith( questZone ))
		{
			return SCRIPT_OVERRIDE;
		}
		
		int targetAttackMode = getIntObjVar( self, "targetAttackMode");
		
		obj_id ship = createTargetShip( self );
		ship_ai.unitSetLeashDistance( ship, 16000 );
		setObjVar( self, "target", ship );
		setLookAtTarget( player, ship );
		setObjVar( ship, "quest", self );
		attachScript( ship, "space.quest_logic.assassinate_target");
		switch( targetAttackMode )
		{
			case 0:
			ship_ai.unitSetAttackOrders( ship, ship_ai.ATTACK_ORDERS_RETURN_FIRE );
			break;
			case 1:
			ship_ai.unitSetAttackOrders( ship, ship_ai.ATTACK_ORDERS_ATTACK_FREELY );
			break;
			case 2:
			ship_ai.unitSetAttackOrders( ship, ship_ai.ATTACK_ORDERS_HOLD_FIRE );
			break;
		}
		
		space_quest._addMissionCriticalShip( player, self, ship );
		
		playClientEffectObj( player, SOUND_SPAWN_ENEMY, player, "");
		
		setObjVar( ship, "objMissionOwner", player );
		ship_ai.unitAddExclusiveAggro( ship, player );
		
		messageTo( self, "updateTargetWaypoint", null, 1.f, false );
		
		transform[] translist = getPathTransforms( self, true );
		ship_ai.unitAddPatrolPath( ship, translist );
		dictionary outparams = new dictionary();
		outparams.put( "quest", self );
		outparams.put( "loc", getLocationObjVar( self, "last_loc") );
		outparams.put( "player", player );
		outparams.put( "dest", "escape");
		messageTo( ship, "registerDestination", outparams, 1.f, false );
		
		String[] shipTypes = getStringArrayObjVar( self, "escortShips");
		if (shipTypes != null)
		{
			int escortSquad = ship_ai.squadCreateSquadId();
			obj_id[] escorts = new obj_id[shipTypes.length];
			for (int i=0; i<shipTypes.length; i++)
			{
				testAbortScript();
				
				transform t = getTransform_o2w( ship );
				transform spawnLoc = space_quest.getRandomPositionInSphere( t, 10, 20 );
				escorts[i] = space_create.createShipHyperspace( shipTypes[i], spawnLoc );
				attachScript( escorts[i], "space.quest_logic.assassinate_ship");
				ship_ai.unitSetLeashDistance( escorts[i], 16000 );
				ship_ai.unitSetSquadId( escorts[i], escortSquad );
				setObjVar( escorts[i], "quest", self );
				
				space_quest._addMissionCriticalShip( player, self, escorts[i] );
				
				setObjVar( escorts[i], "objMissionOwner", player );
				ship_ai.unitAddExclusiveAggro( escorts[i], player );
			}
			setObjVar( self, "escorts", escorts );
			
			if (ship_ai.squadGetSize(escortSquad) > 0)
			{
				ship_ai.squadFollow( escortSquad, ship, new vector(0,0,1), 10 );
				ship_ai.squadSetGuardTarget( escortSquad, ship_ai.unitGetSquadId(ship) );
				
				switch( targetAttackMode )
				{
					case 0:
					ship_ai.squadSetAttackOrders( escortSquad, ship_ai.ATTACK_ORDERS_RETURN_FIRE );
					break;
					case 1:
					ship_ai.squadSetAttackOrders( escortSquad, ship_ai.ATTACK_ORDERS_ATTACK_FREELY );
					break;
					case 2:
					ship_ai.squadSetAttackOrders( escortSquad, ship_ai.ATTACK_ORDERS_HOLD_FIRE );
					break;
				}
			}
		}
		
		String questName = getStringObjVar( self, space_quest.QUEST_NAME );
		String questType = getStringObjVar( self, space_quest.QUEST_TYPE );
		string_id status_update = new string_id( "spacequest/"+questType+"/"+questName, "arrival_phase_2");
		questUpdate( self, status_update );
		
		return SCRIPT_OVERRIDE;
	}
	
	
	public int assassinateSuccess(obj_id self, dictionary params) throws InterruptedException
	{
		if (params == null)
		{
			return SCRIPT_CONTINUE;
		}
		
		String questName = getStringObjVar( self, space_quest.QUEST_NAME );
		String questType = getStringObjVar( self, space_quest.QUEST_TYPE );
		obj_id player = getObjIdObjVar( self, space_quest.QUEST_OWNER );
		obj_id ship = params.getObjId( "ship");
		int mustDestroyEscort = getIntObjVar( self, "mustDestroyEscort");
		int escortDestroyed = getIntObjVar( self, "escortDestroyed");
		
		int questid = questGetQuestId( "spacequest/"+questType+"/"+questName );
		if ((questid != 0) && questIsTaskActive( questid, 1, player ))
		{
			questCompleteTask( questid, 1, player );
		}
		
		if ((mustDestroyEscort == 1) && (escortDestroyed == 0))
		{
			
			string_id update = new string_id( "spacequest/"+questType+"/"+questName, "destroy_escort");
			questUpdate( self, update );
			
			setObjVar( self, "targetDestroyed", 1 );
		}
		else if ((mustDestroyEscort == 0) || ((mustDestroyEscort == 1) && (escortDestroyed == 1)))
		{
			if (questid != 0 && questIsQuestActive( questid, player ))
			{
				questCompleteQuest( questid, player );
			}
			
			string_id update = new string_id( "spacequest/"+questType+"/"+questName, "assassinate_success");
			questUpdate( self, update );
			
			messageTo( self, "completeQuestMsg", null, 2.f, false );
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int targetEscaped(obj_id self, dictionary params) throws InterruptedException
	{
		if (params == null)
		{
			return SCRIPT_CONTINUE;
		}
		
		String questName = getStringObjVar( self, space_quest.QUEST_NAME );
		String questType = getStringObjVar( self, space_quest.QUEST_TYPE );
		obj_id player = getObjIdObjVar( self, space_quest.QUEST_OWNER );
		obj_id ship = params.getObjId( "target");
		
		string_id status_update = new string_id( "spacequest/"+questType+"/"+questName, "failed_escape");
		questUpdate( self, status_update );
		
		cleanupShips( self );
		questFailed( self );
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int handleShipDestroyed(obj_id self, dictionary params) throws InterruptedException
	{
		if (params == null)
		{
			return SCRIPT_OVERRIDE;
		}
		
		String questName = getStringObjVar( self, space_quest.QUEST_NAME );
		String questType = getStringObjVar( self, space_quest.QUEST_TYPE );
		obj_id player = getObjIdObjVar( self, space_quest.QUEST_OWNER );
		obj_id destroyed = params.getObjId( "ship");
		obj_id[] escorts = getObjIdArrayObjVar( self, "escorts");
		int dead_escorts = getIntObjVar( self, "dead_escorts");
		int mustDestroyEscort = getIntObjVar( self, "mustDestroyEscort");
		int targetDestroyed = getIntObjVar( self, "targetDestroyed");
		
		if (escorts == null)
		{
			return SCRIPT_OVERRIDE;
		}
		
		for (int i=0; i<escorts.length; i++)
		{
			testAbortScript();
			if (escorts[i] == destroyed)
			{
				
				space_quest._removeMissionCriticalShip( player, self, destroyed );
				escorts[i] = obj_id.NULL_ID;
				
				dead_escorts++;
				int remaining = escorts.length - dead_escorts;
				if (remaining == 0)
				{
					
					playClientEffectObj( player, SOUND_DESTROYED_ALL, player, "");
					if (mustDestroyEscort == 1)
					{
						int questid = questGetQuestId( "spacequest/"+questType+"/"+questName );
						if ((questid != 0) && questIsTaskActive( questid, 2, player ))
						{
							questCompleteTask( questid, 2, player );
						}
					}
					if ((mustDestroyEscort == 1) && (targetDestroyed == 1))
					{
						
						string_id update = new string_id( "spacequest/"+questType+"/"+questName, "assassinate_success");
						questUpdate( self, update );
						
						int questid = questGetQuestId( "spacequest/"+questType+"/"+questName );
						if (questid != 0 && questIsQuestActive( questid, player ))
						{
							questCompleteQuest( questid, player );
						}
						
						messageTo( self, "completeQuestMsg", null, 2.f, false );
					}
					else
					{
						string_id update = new string_id( "spacequest/"+questType+"/"+questName, "escort_wiped_out");
						space_quest.sendQuestMessage( player, update );
						setObjVar( self, "escortDestroyed", 1 );
					}
				}
				else
				{
					
					string_id update = new string_id( "spacequest/"+questType+"/"+questName, "escort_remaining");
					prose_package pp = prose.getPackage( update, remaining );
					space_quest.sendQuestMessage( player, pp );
				}
			}
			else if (escorts[i] == null)
			{
				escorts[i] = obj_id.NULL_ID;
			}
		}
		setObjVar( self, "escorts", escorts );
		setObjVar( self, "dead_escorts", dead_escorts );
		
		return SCRIPT_OVERRIDE;
	}
	
	
	public int warpoutFailure(obj_id self, dictionary params) throws InterruptedException
	{
		if (hasObjVar( self, "handling_warpout_failure" ))
		{
			return SCRIPT_OVERRIDE;
		}
		setObjVar( self, "handling_warpout_failure", 1 );
		obj_id player = getObjIdObjVar( self, space_quest.QUEST_OWNER );
		sendQuestSystemMessage( player, WARPOUT_FAILURE );
		clearMissionWaypoint( self );
		cleanupShips( self );
		questFailed( self );
		
		return SCRIPT_OVERRIDE;
	}
}
