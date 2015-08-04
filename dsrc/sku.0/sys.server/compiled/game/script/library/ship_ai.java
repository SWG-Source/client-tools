package script.library;

import script.*;
import script.base_class.*;
import script.combat_engine.*;
import java.util.Arrays;
import java.util.Hashtable;
import java.util.Vector;
import script.base_script;


import script.library.utils;
import java.util.Random;
import java.util.Vector;
import script.transform;
import script.vector;


public class ship_ai extends script.base_script
{
	public ship_ai()
	{
	}
	public static final float DEFAULT_FOLLOW_DISTANCE = 40.0f;
	
	public static final int BEHAVIOR_IDLE = 0;
	public static final int BEHAVIOR_TRACK = 1;
	public static final int BEHAVIOR_MOVETO = 2;
	public static final int BEHAVIOR_PATROL = 3;
	public static final int BEHAVIOR_FOLLOW = 4;
	
	public static final int ATTACK_ORDERS_HOLD_FIRE = 0;
	public static final int ATTACK_ORDERS_RETURN_FIRE = 1;
	public static final int ATTACK_ORDERS_ATTACK_FREELY = 2;
	
	public static final int FORMATION_INVALID = 0;
	public static final int FORMATION_CLAW = 1;
	public static final int FORMATION_WALL = 2;
	public static final int FORMATION_SPHERE = 3;
	public static final int FORMATION_DELTA = 4;
	public static final int FORMATION_BROAD = 5;
	public static final int FORMATION_X = 6;
	
	
	public static int unitGetBehavior(obj_id unit) throws InterruptedException
	{
		return _spaceUnitGetBehavior(unit);
	}
	
	
	public static void unitIdle(obj_id unit) throws InterruptedException
	{
		_spaceUnitIdle(unit);
	}
	
	
	public static void squadIdle(int squadId) throws InterruptedException
	{
		_spaceSquadIdle(squadId);
	}
	
	
	public static void unitTrack(obj_id unit, obj_id target) throws InterruptedException
	{
		_spaceUnitTrack(unit, target);
	}
	
	
	public static void squadTrack(int squadId, obj_id target) throws InterruptedException
	{
		_spaceSquadTrack(squadId, target);
	}
	
	
	public static void unitMoveTo(obj_id unit, transform[] path) throws InterruptedException
	{
		_spaceUnitMoveTo(unit, path);
	}
	
	
	public static void squadMoveTo(int squadId, transform[] path) throws InterruptedException
	{
		_spaceSquadMoveTo(squadId, path);
	}
	
	
	public static void unitMoveTo(obj_id unit, transform position) throws InterruptedException
	{
		transform path[] = new transform[1];
		path[0] = position;
		_spaceUnitMoveTo(unit, path);
	}
	
	
	public static void squadMoveTo(int squadId, transform position) throws InterruptedException
	{
		transform path[] = new transform[1];
		path[0] = position;
		_spaceSquadMoveTo(squadId, path);
	}
	
	
	public static void unitAddPatrolPath(obj_id unit, transform[] path) throws InterruptedException
	{
		_spaceUnitAddPatrolPath(unit, path);
	}
	
	
	public static void squadAddPatrolPath(int squadId, transform[] path) throws InterruptedException
	{
		_spaceSquadAddPatrolPath(squadId, path);
	}
	
	
	public static void unitPatrol(obj_id unit, transform[] path) throws InterruptedException
	{
		unitClearPatrolPath(unit);
		unitAddPatrolPath(unit, path);
	}
	
	
	public static void squadPatrol(int squadId, transform[] path) throws InterruptedException
	{
		squadClearPatrolPath(squadId);
		squadAddPatrolPath(squadId, path);
	}
	
	
	public static transform[] createPatrolPathCircle(vector position_w, float radius) throws InterruptedException
	{
		final int points = 20;
		transform path[] = new transform[points];
		
		for (int i = 0; i < points; ++i)
		{
			testAbortScript();
			final float radian = (float)Math.PI * 2.0f * ((float)i / (float)points);
			final float x = position_w.x + (float)Math.sin(radian) * radius;
			final float y = position_w.y;
			final float z = position_w.z + (float)Math.cos(radian) * radius;
			path[i] = transform.identity.setPosition_p(x, y, z);
		}
		
		return path;
	}
	
	
	public static transform[] createPatrolPathLoiter(transform transform_w, float minDist, float maxDist) throws InterruptedException
	{
		final int points = 20;
		transform path[] = new transform[points];
		for (int i = 0; i < points; ++i)
		{
			testAbortScript();
			final vector direction = vector.randomUnit();
			vector position = transform_w.getPosition_p().add(direction.multiply(minDist + (float)Math.random() * (maxDist - minDist)));
			path[i] = transform.identity.setPosition_p(position);
		}
		
		return path;
	}
	
	
	public static void unitLoiter(obj_id unit, transform transform_w, float minDist, float maxDist) throws InterruptedException
	{
		unitPatrol(unit, createPatrolPathLoiter(transform_w, minDist, maxDist));
	}
	
	
	public static void squadLoiter(int squadId, transform transform_w, float minDist, float maxDist) throws InterruptedException
	{
		squadPatrol(squadId, createPatrolPathLoiter(transform_w, minDist, maxDist));
	}
	
	
	public static void unitClearPatrolPath(obj_id unit) throws InterruptedException
	{
		_spaceUnitClearPatrolPath(unit);
	}
	
	
	public static void squadClearPatrolPath(int squadId) throws InterruptedException
	{
		obj_id[] unitList = squadGetUnitList(squadId);
		
		for (int i = 0; i < unitList.length; ++i)
		{
			testAbortScript();
			_spaceUnitClearPatrolPath(unitList[i]);
		}
	}
	
	
	public static void unitFollow(obj_id unit, obj_id followedUnit, vector direction_o, float distance) throws InterruptedException
	{
		_spaceUnitFollow(unit, followedUnit, direction_o, distance);
	}
	
	
	public static void squadFollow(int squadId, obj_id followedUnit, vector direction_o, float distance) throws InterruptedException
	{
		_spaceSquadFollow(squadId, followedUnit, direction_o, distance);
	}
	
	
	public static void unitAddDamageTaken(obj_id unit, obj_id targetUnit, float damage) throws InterruptedException
	{
		_spaceUnitAddDamageTaken(unit, targetUnit, damage);
	}
	
	
	public static void unitSetAttackOrders(obj_id unit, int attackOrders) throws InterruptedException
	{
		
		LOG("space_debug_ai", "Attack Orders received for: "+ unit + " order: "+ attackOrders);
		
		_spaceUnitSetAttackOrders(unit, attackOrders);
	}
	
	
	public static void squadSetAttackOrders(int squadId, int attackOrders) throws InterruptedException
	{
		obj_id[] unitList = squadGetUnitList(squadId);
		
		for (int i = 0; i < unitList.length; ++i)
		{
			testAbortScript();
			unitSetAttackOrders(unitList[i], attackOrders);
		}
	}
	
	
	public static void unitSetLeashDistance(obj_id unit, float distance) throws InterruptedException
	{
		_spaceUnitSetLeashDistance(unit, distance);
	}
	
	
	public static void squadSetLeashDistance(int squadId, float distance) throws InterruptedException
	{
		obj_id[] unitList = squadGetUnitList(squadId);
		
		for (int i = 0; i < unitList.length; ++i)
		{
			testAbortScript();
			unitSetLeashDistance(unitList[i], distance);
		}
	}
	
	
	public static void unitSetPilotType(obj_id unit, String pilotType) throws InterruptedException
	{
		_spaceUnitSetPilotType(unit, pilotType);
	}
	
	
	public static String unitGetPilotType(obj_id unit) throws InterruptedException
	{
		return _spaceUnitGetPilotType(unit);
	}
	
	
	public static obj_id unitGetPrimaryAttackTarget(obj_id unit) throws InterruptedException
	{
		return _spaceUnitGetPrimaryAttackTarget(unit);
	}
	
	
	public static obj_id[] unitGetAttackTargetList(obj_id unit) throws InterruptedException
	{
		return _spaceUnitGetAttackTargetList(unit);
	}
	
	
	public static obj_id[] unitGetWhoIsTargetingMe(obj_id unit) throws InterruptedException
	{
		return _spaceUnitGetWhoIsTargetingMe(unit);
	}
	
	
	public static boolean unitIsAttacking(obj_id unit) throws InterruptedException
	{
		return _spaceUnitIsAttacking(unit);
	}
	
	
	public static void unitIncreaseHate(obj_id unit, obj_id unitToHate, float amountToHate, float hateDelay, int maxRecursions) throws InterruptedException
	{
		if ((!isIdValid(unit)) || isPlayer(unit) || !isIdValid(unitToHate))
		{
			return;
		}
		
		if (!exists(unitToHate) || !unitToHate.isLoaded())
		{
			return;
		}
		
		--maxRecursions;
		if (maxRecursions < 1)
		{
			return;
		}
		else if (maxRecursions > 20)
		{
			maxRecursions = 20;
		}
		
		if (hateDelay < 10.0f)
		{
			hateDelay = 10.0f;
		}
		
		if (unitGetPrimaryAttackTarget(unit) != unitToHate)
		{
			unitAddDamageTaken(unit, unitToHate, amountToHate);
		}
		
		dictionary parms = new dictionary();
		parms.put( "unitToHate", unitToHate );
		parms.put( "amountToHate", amountToHate );
		parms.put( "hateDelay", hateDelay );
		parms.put( "maxRecursions", maxRecursions );
		messageTo( unit, "handleUnitIncreaseHate", parms, hateDelay, false );
	}
	
	
	public static void unitRemoveAttackTarget(obj_id unit, obj_id unitToRemove) throws InterruptedException
	{
		_spaceUnitRemoveAttackTarget(unit, unitToRemove);
	}
	
	
	public static void unitRemoveFromAllAttackTargetLists(obj_id unit) throws InterruptedException
	{
		obj_id[] whoIsTargetingMeList = unitGetWhoIsTargetingMe(unit);
		
		for (int index = 0; index < whoIsTargetingMeList.length; ++index)
		{
			testAbortScript();
			if (exists(whoIsTargetingMeList[index]) && (whoIsTargetingMeList[index].isLoaded()))
			{
				if (!space_utils.isPlayerControlledShip(whoIsTargetingMeList[index]))
				{
					unitRemoveAttackTarget(whoIsTargetingMeList[index], unit);
				}
				else
				{
					LOG("space", whoIsTargetingMeList[index] + " was passed into getWhoistargetingme but it's a PLAYER");
				}
			}
			else
			{
				LOG("space", whoIsTargetingMeList[index] + " was passed into unitGetWhoIsTargetingMe but doesn't exist on the server");
				
			}
		}
	}
	
	
	public static void unitSetPrimaryTarget(obj_id unit, obj_id targetUnit) throws InterruptedException
	{
		unitAddDamageTaken(unit, targetUnit, 100000.0f);
	}
	
	
	public static void unitAddTarget(obj_id unit, obj_id targetUnit) throws InterruptedException
	{
		unitAddDamageTaken(unit, targetUnit, 100.0f);
	}
	
	
	public static void squadSetPrimaryTarget(int squadId, obj_id targetUnit) throws InterruptedException
	{
		obj_id[] unitList = squadGetUnitList(squadId);
		
		for (int i = 0; i < unitList.length; ++i)
		{
			testAbortScript();
			unitAddDamageTaken(unitList[i], targetUnit, 100000.0f);
		}
	}
	
	
	public static void squadAddTarget(int squadId, obj_id targetUnit) throws InterruptedException
	{
		obj_id[] unitList = squadGetUnitList(squadId);
		
		for (int i = 0; i < unitList.length; ++i)
		{
			testAbortScript();
			unitAddDamageTaken(unitList[i], targetUnit, 100.0f);
		}
	}
	
	
	public static void squadSetPrimaryTarget(int squadId, int targetSquadId) throws InterruptedException
	{
		obj_id[] unitList = squadGetUnitList(squadId);
		obj_id[] targetUnitList = squadGetUnitList(targetSquadId);
		
		for (int i = 0; i < unitList.length; ++i)
		{
			testAbortScript();
			for (int j = 0; j < targetUnitList.length; ++j)
			{
				testAbortScript();
				unitAddDamageTaken(unitList[i], targetUnitList[j], (float)Math.random() * 100000.0f + 100000.0f);
			}
		}
	}
	
	
	public static void squadAddTarget(int squadId, int targetSquadId) throws InterruptedException
	{
		obj_id[] unitList = squadGetUnitList(squadId);
		obj_id[] targetUnitList = squadGetUnitList(targetSquadId);
		
		for (int i = 0; i < unitList.length; ++i)
		{
			testAbortScript();
			for (int j = 0; j < targetUnitList.length; ++j)
			{
				testAbortScript();
				unitAddDamageTaken(unitList[i], targetUnitList[j], (float)Math.random() * 100.0f + 100.0f);
			}
		}
	}
	
	
	public static void unitSetSquadId(obj_id unit, int squadId) throws InterruptedException
	{
		LOG("space_debug_ai", "unit: "+ unit + " unitGetSquadId: "+_spaceUnitGetSquadId(unit));
		
		_spaceUnitSetSquadId(unit, squadId);
		
		LOG("space_debug_ai", "unit: "+ unit + " unitGetSquadId: "+_spaceUnitGetSquadId(unit));
	}
	
	
	public static int unitGetSquadId(obj_id unit) throws InterruptedException
	{
		return _spaceUnitGetSquadId(unit);
	}
	
	
	public static int squadCreateSquadId() throws InterruptedException
	{
		return _spaceSquadCreateSquadId();
	}
	
	
	public static int squadRemoveUnit(obj_id unit) throws InterruptedException
	{
		return _spaceSquadRemoveUnit(unit);
	}
	
	
	public static int squadCombine(int squadId1, int squadId2) throws InterruptedException
	{
		return _spaceSquadCombine(squadId1, squadId2);
	}
	
	
	public static int squadGetSize(int squadId) throws InterruptedException
	{
		return _spaceSquadGetSize(squadId);
	}
	
	
	public static obj_id[] squadGetUnitList(int squadId) throws InterruptedException
	{
		return _spaceSquadGetUnitList(squadId);
	}
	
	
	public static void squadSetFormation(int squadId, int formation) throws InterruptedException
	{
		_spaceSquadSetFormation(squadId, formation);
	}
	
	
	public static void squadSetFormationRandom(int squadId) throws InterruptedException
	{
		Random random = new Random();
		final int value = Math.abs(random.nextInt()) % 6;
		int formation = FORMATION_INVALID;
		
		switch (value)
		{
			case 0:
			{
				formation = FORMATION_CLAW;
			}
			break;
			case 1:
			{
				formation = FORMATION_WALL;
			}
			break;
			case 2:
			{
				formation = FORMATION_SPHERE;
			}
			break;
			case 3:
			{
				formation = FORMATION_DELTA;
			}
			break;
			case 4:
			{
				formation = FORMATION_BROAD;
			}
			break;
			case 5:
			{
				formation = FORMATION_X;
			}
			break;
		}
		_spaceSquadSetFormation(squadId, formation);
	}
	
	
	public static void squadSetFormationSpacing(int squadId, float scale) throws InterruptedException
	{
		_spaceSquadSetFormationSpacing(squadId, scale);
	}
	
	
	public static int squadGetFormation(int squadId) throws InterruptedException
	{
		return _spaceSquadGetFormation(squadId);
	}
	
	
	public static void squadSetLeader(int squadId, obj_id unit) throws InterruptedException
	{
		_spaceSquadSetLeader(squadId, unit);
	}
	
	
	public static obj_id squadGetLeader(int squadId) throws InterruptedException
	{
		return _spaceSquadGetLeader(squadId);
	}
	
	
	public static void squadSetGuardTarget(int squad, int targetSquad) throws InterruptedException
	{
		_spaceSquadSetGuardTarget(squad, targetSquad);
	}
	
	
	public static boolean squadIsGuarding(int squad) throws InterruptedException
	{
		return (squadGetGuardTarget(squad) == 0);
	}
	
	
	public static int squadGetGuardTarget(int squad) throws InterruptedException
	{
		return _spaceSquadGetGuardTarget(squad);
	}
	
	
	public static void squadRemoveGuardTarget(int squad) throws InterruptedException
	{
		_spaceSquadRemoveGuardTarget(squad);
	}
	
	
	public static void unitDock(obj_id unit, obj_id dockTarget, float timeAtDock) throws InterruptedException
	{
		_spaceUnitDock(unit, dockTarget, timeAtDock);
	}
	
	
	public static void unitDock(obj_id unit, obj_id dockTarget) throws InterruptedException
	{
		unitDock(unit, dockTarget, -1.0f);
	}
	
	
	public static void unitUnDock(obj_id unit) throws InterruptedException
	{
		_spaceUnitUnDock(unit);
	}
	
	
	public static boolean unitIsDocked(obj_id unit) throws InterruptedException
	{
		return _spaceUnitIsDocked(unit);
	}
	
	
	public static boolean unitIsDocking(obj_id unit) throws InterruptedException
	{
		return _spaceUnitIsDocking(unit);
	}
	
	
	public static void unitSetAutoAggroImmuneTime(obj_id unit, float time) throws InterruptedException
	{
		_spaceUnitSetAutoAggroImmuneTime(unit, time);
	}
	
	
	public static void unitSetAutoAggroImmune(obj_id unit, boolean enabled) throws InterruptedException
	{
		_spaceUnitSetAutoAggroImmuneTime(unit, enabled ? -1.0f : 0.0f);
	}
	
	
	public static boolean unitIsAutoAggroImmune(obj_id unit) throws InterruptedException
	{
		return _spaceUnitIsAutoAggroImmune(unit);
	}
	
	
	public static void unitSetDamageAggroImmune(obj_id unit, boolean enabled) throws InterruptedException
	{
		_spaceUnitSetDamageAggroImmune(unit, enabled);
	}
	
	
	public static transform unitGetDockTransform(obj_id dockTarget, obj_id dockingUnit) throws InterruptedException
	{
		return _spaceUnitGetDockTransform(dockTarget, dockingUnit);
	}
	
	
	public static void unitAddExclusiveAggro(obj_id unit, obj_id pilot) throws InterruptedException
	{
		_spaceUnitAddExclusiveAggro(unit, pilot);
	}
	
	
	public static void unitRemoveExclusiveAggro(obj_id unit, obj_id pilot) throws InterruptedException
	{
		_spaceUnitRemoveExclusiveAggro(unit, pilot);
	}
	
	
	public static boolean isSquadIdValid(int squadId) throws InterruptedException
	{
		return _spaceSquadIsSquadIdValid(squadId);
	}
	
	
	public static boolean isShipDead(obj_id ship) throws InterruptedException
	{
		if (!isIdValid( ship ))
		{
			return true;
		}
		
		if (!ship.isLoaded())
		{
			return true;
		}
		
		return ( hasObjVar( ship, "ship.isDead" ) );
	}
	
	
	public static boolean isPlayerShip(obj_id ship) throws InterruptedException
	{
		obj_id pilot = getPilotId( ship );
		if (isIdValid( pilot ))
		{
			return ( isPlayer( pilot ) );
		}
		else
		{
			return false;
		}
	}
	
	
	public static boolean isShipAggro(obj_id ship) throws InterruptedException
	{
		if (isPlayerShip(ship))
		{
			return false;
		}
		
		if (!hasObjVar( ship, "ship.shipName" ))
		{
			return true;
		}
		
		String shipName = getStringObjVar( ship, "ship.shipName");
		return ( dataTableGetInt( "datatables/space_mobile/space_mobile.iff", shipName, "isAggro" ) == 1 );
	}
	
	
	public static boolean isShipAggroToward(obj_id ship, obj_id target) throws InterruptedException
	{
		String shipFaction = getShipFaction( ship );
		String targetFaction = getShipFaction( target );
		if (shipFaction == null || targetFaction == null)
		{
			return false;
		}
		
		if (shipFaction.equals(targetFaction))
		{
			return false;
		}
		
		if (!hasObjVar( ship, "ship.shipName" ))
		{
			return false;
		}
		
		String shipName = getStringObjVar( ship, "ship.shipName");
		String enemyFactionList = dataTableGetString( "datatables/space_mobile/space_mobile.iff", shipName, "enemyFactions");
		if (enemyFactionList == null)
		{
			return false;
		}
		
		String[] enemiesList = split( enemyFactionList, ',');
		for (int i = 0; i < enemiesList.length; i++)
		{
			testAbortScript();
			if (enemiesList[i].equals( targetFaction ))
			{
				return true;
			}
		}
		
		return false;
	}
	
	
	public static boolean isShipAlliedWith(obj_id ship, obj_id target) throws InterruptedException
	{
		if (!isShipSocial( ship ))
		{
			return false;
		}
		
		String shipFaction = getShipFaction( ship );
		String targetFaction = getShipFaction( target );
		if (shipFaction == null || targetFaction == null)
		{
			return false;
		}
		
		if (shipFaction.equals(targetFaction))
		{
			return true;
		}
		
		if (!hasObjVar( ship, "ship.shipName" ))
		{
			return false;
		}
		
		String shipName = getStringObjVar( ship, "ship.shipName");
		String allyFactionList = dataTableGetString( "datatables/space_mobile/space_mobile.iff", shipName, "alliedFactions");
		if (allyFactionList == null)
		{
			return false;
		}
		
		String[] allyList = split( allyFactionList, ',');
		for (int i = 0; i < allyList.length; i++)
		{
			testAbortScript();
			if (allyList[i].equals( targetFaction ))
			{
				return true;
			}
		}
		
		return false;
	}
	
	
	public static boolean isShipSocial(obj_id ship) throws InterruptedException
	{
		if (!hasObjVar( ship, "ship.shipName" ))
		{
			return true;
		}
		
		String shipName = getStringObjVar( ship, "ship.shipName");
		
		String enemyFactionList = dataTableGetString( "datatables/space_mobile/space_mobile.iff", shipName, "enemyFactions");
		String allyFactionList = dataTableGetString( "datatables/space_mobile/space_mobile.iff", shipName, "alliedFactions");
		if (enemyFactionList == null || allyFactionList == null)
		{
			return false;
		}
		
		if (enemyFactionList.equals("") || allyFactionList.equals(""))
		{
			return false;
		}
		
		return true;
	}
	
	
	public static boolean isSameFaction(obj_id ship, obj_id target) throws InterruptedException
	{
		String shipFaction = getShipFaction( ship );
		String targetFaction = getShipFaction( target );
		if (shipFaction == null || targetFaction == null)
		{
			return false;
		}
		
		return (shipFaction.equals(targetFaction));
	}
	
	
	public static obj_id[] getGroupShips(obj_id group, obj_id primaryTarget) throws InterruptedException
	{
		obj_id[] groupMembers = getGroupMemberIds( group );
		Vector groupShips = new Vector();
		groupShips.add( primaryTarget );
		for (int i = 0; i < groupMembers.length; i++)
		{
			testAbortScript();
			
			if (groupMembers[i].isLoaded())
			{
				obj_id ship = getPilotedShip( groupMembers[i] );
				
				if (isIdValid(ship) && ship != primaryTarget && getDistance( ship, primaryTarget ) < 120.0f)
				{
					groupShips.add( ship );
				}
			}
		}
		
		obj_id[] returnArray = new obj_id[groupShips.size()];
		groupShips.toArray( returnArray );
		return returnArray;
	}
	
	
	public static String unitGetBehaviorString(int behavior) throws InterruptedException
	{
		String result;
		
		switch (behavior)
		{
			default:
			{
				result = "invalid";
			}
			break;
			case BEHAVIOR_IDLE:
			{
				result = "BEHAVIOR_IDLE";
			}
			break;
			case BEHAVIOR_MOVETO:
			{
				result = "BEHAVIOR_MOVETO";
			}
			break;
			case BEHAVIOR_PATROL:
			{
				result = "BEHAVIOR_PATROL";
			}
			break;
			case BEHAVIOR_FOLLOW:
			{
				result = "BEHAVIOR_FOLLOW";
			}
			break;
			case BEHAVIOR_TRACK:
			{
				result = "BEHAVIOR_TRACK";
			}
			break;
		}
		
		LOG("space_debug_ai", "unitGetBehaviorString: "+ result);
		
		return result;
	}
	
	
	public static String unitGetAttackOrdersString(int attackOrders) throws InterruptedException
	{
		String result;
		
		switch (attackOrders)
		{
			default:
			{
				result = "invalid";
			}
			break;
			case ATTACK_ORDERS_HOLD_FIRE:
			{
				result = "ATTACK_ORDERS_HOLD_FIRE";
			}
			break;
			case ATTACK_ORDERS_RETURN_FIRE:
			{
				result = "ATTACK_ORDERS_DEFEND";
			}
			break;
			case ATTACK_ORDERS_ATTACK_FREELY:
			{
				result = "ATTACK_ORDERS_ATTACK_FREELY";
			}
			break;
		}
		LOG("space_debug_ai", "unitGetAttackOrdersString: "+ result);
		
		return result;
	}
	
	
	public static void spaceAttack(obj_id ship, obj_id target) throws InterruptedException
	{
		unitAddDamageTaken(ship, target, 100000.0f);
	}
	
	
	public static void spaceAttack(obj_id ship, obj_id[] targets) throws InterruptedException
	{
		
		for (int i = 0; i < targets.length; i++)
		{
			testAbortScript();
			unitAddDamageTaken(ship, targets[i], 100000.0f);
		}
	}
	
	
	public static obj_id spaceGetPrimaryTarget(obj_id ship) throws InterruptedException
	{
		return unitGetPrimaryAttackTarget(ship);
	}
	
	
	public static boolean spaceIsInCombat(obj_id ship) throws InterruptedException
	{
		return unitIsAttacking(ship);
	}
	
	
	public static void spaceStopAttack(obj_id ship) throws InterruptedException
	{
		unitSetAttackOrders(ship, ATTACK_ORDERS_HOLD_FIRE);
	}
	
	
	public static void spaceStopAttack(obj_id ship, obj_id target) throws InterruptedException
	{
		unitRemoveAttackTarget(ship, target);
	}
	
	
	public static void spaceGuard(obj_id ship, obj_id target) throws InterruptedException
	{
		debugServerConsoleMsg( ship, "Guarding is not supported ATM");
	}
	
	
	public static void spaceGuard(obj_id ship, obj_id[] targets) throws InterruptedException
	{
		debugServerConsoleMsg( ship, "Guarding is not supported ATM");
	}
	
	
	public static Vector spaceGetGuardList(obj_id ship) throws InterruptedException
	{
		debugServerConsoleMsg( ship, "Guarding is not supported ATM");
		return null;
	}
	
	
	public static boolean spaceIsGuarding(obj_id ship, obj_id target) throws InterruptedException
	{
		debugServerConsoleMsg( ship, "Guarding is not supported ATM");
		return false;
	}
	
	
	public static void spaceStopGuarding(obj_id ship, obj_id target) throws InterruptedException
	{
		debugServerConsoleMsg( ship, "Guarding is not supported ATM");
	}
	
	
	public static void spaceMoveTo(obj_id ship, transform position) throws InterruptedException
	{
		unitMoveTo(ship, position);
	}
	
	
	public static void spaceMoveTo(obj_id ship, transform[] path) throws InterruptedException
	{
		unitMoveTo(ship, path);
	}
	
	
	public static void spacePatrol(obj_id ship, transform[] transforms) throws InterruptedException
	{
		unitAddPatrolPath(ship, transforms);
	}
	
	
	public static void spaceStop(obj_id ship) throws InterruptedException
	{
		unitIdle(ship);
	}
	
	
	public static void spaceLoiter(obj_id ship, transform transform_w, float minDist, float maxDist) throws InterruptedException
	{
		unitLoiter(ship, transform_w, minDist, maxDist);
	}
	
	
	public static void spaceFollow(obj_id ship, obj_id target, float dist) throws InterruptedException
	{
		unitFollow(ship, target, new vector(0.0f, 0.0f, 1.0f), dist);
	}
	
	
	public static void spaceFollow(obj_id ship, obj_id target) throws InterruptedException
	{
		spaceFollow( ship, target, DEFAULT_FOLLOW_DISTANCE );
	}
	
	
	public static void spaceFollowInFormation(obj_id ship, obj_id target, int formationType) throws InterruptedException
	{
		spaceFollow( ship, target, DEFAULT_FOLLOW_DISTANCE );
	}
	
	
	public static int spaceGetShipBehavior(obj_id ship) throws InterruptedException
	{
		return unitGetBehavior(ship);
	}
	
	
	public static obj_id[] getWhoIsTargetingShip(obj_id ship) throws InterruptedException
	{
		return unitGetWhoIsTargetingMe(ship);
	}
}
