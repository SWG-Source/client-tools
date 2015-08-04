package script.library;

import script.*;
import script.base_class.*;
import script.combat_engine.*;
import java.util.Arrays;
import java.util.Hashtable;
import java.util.Vector;
import script.base_script;

import java.lang.Math;
import java.util.StringTokenizer;
import script.library.ai_lib;
import script.library.beast_lib;
import script.library.buff;
import script.library.city;
import script.library.colors;
import script.library.combat;
import script.library.dot;
import script.library.factions;
import script.library.group;
import script.library.locations;
import script.library.loot;
import script.library.luck;
import script.library.money;
import script.library.pet_lib;
import script.library.prose;
import script.library.static_item;
import script.library.sui;
import script.library.utils;
import script.library.vehicle;
import script.library.xp;
import script.library.weapons;


public class stealth extends script.base_script
{
	public stealth()
	{
	}
	public static final java.text.NumberFormat floatFormat = new java.text.DecimalFormat("###.##");
	
	public static final int MAX_BUILDINGS_FOR_WILDNERNESS = 5;
	public static final int MIN_BLENDIN_CROWD_SIZE = 10;
	public static final String CAMO_KIT_ID = "camoKit";
	public static final String ACTIVE_HEP = "active_hep";
	public static final int MIN_MOVEMENT_PRIORITY = 3;
	public static final int MIN_POSTURE_CHANGE_PRIORITY = 4;
	
	public static final String INVIS_BREAK_RADIUS_FAR = "invis_break_far";
	public static final String INVIS_BREAK_RADIUS_NEAR = "invis_break__near";
	public static final String INVIS_UPKEEP_MSG_DISPATCHED = "invis_upkeep_dispatched";
	public static final String TRAP_PROXIMITY_VOLUME = "trap_proximity";
	public static final String SENSOR_PROXIMITY_VOLUME = "sensor_proximity";
	
	public static final float INVIS_BREAK_MAX_FAR_DISTANCE = 20f;
	public static final float INVIS_BREAK_NEAR_DISTANCE = 8f;
	
	public static final int PASSIVE_BREACH_FAR = 2;
	public static final int PASSIVE_BREACH_NEAR = 1;
	
	public static final float PASSIVE_DETECT_CHANCE_FAR = 15.0f;
	public static final float PASSIVE_DETECT_CHANCE_NEAR = 45.0f;
	public static final float PASSIVE_DETECT_CHANCE_MOB = 20.0f;
	public static final String INCOGNITO_OLD_FACTION = "stealth.incognitoFaction";
	
	public static final float MIN_CHANCE_TO_DETECT_HIDDEN = 1f;
	public static final float MAX_CHANCE_TO_DETECT_HIDDEN = 99f;
	public static final float PASSIVE_DETECT_CHANCE = 20f;
	public static final float ACTIVE_DETECT_CHANCE = 50f;
	public static final float MAX_HIDING_DETECT_SCORE = 220;
	
	public static final int HEP_UPKEEP_COST = 10;
	public static final String HEP_BASE_POWER = "basePower";
	public static final String HEP_EFFECTIVENESS = "effectiveness";
	
	public static final int STEAL_NOTHING = 0;
	public static final int STEAL_CREDITS = 1;
	public static final int STEAL_LOOT = 2;
	public static final int STEAL_TEMPLATE = 4;
	public static final int STEAL_MARKED_ITEMS = 8;
	public static final int STEAL_STEALING_TABLE = 16;
	
	public static final float STEAL_LOOT_CHANCE_MOD = -0.90f;
	public static final int STEAL_FAIL_CHANCE = 50;
	public static final int STEAL_REGULAR_ITEM = 75;
	public static final int STEAL_RARE_ITEM = 98;
	public static final String STEAL_TYPE = "stealing.type";
	public static final String STEAL_TEMPLATE_ITEMS = "stealing.availableLoot";
	public static final String STEAL_ITEM_IS_STEALABLE = "stealing.isStealable";
	
	public static final String TRAP_TR_ACCURACY = "trap.accuracy";
	public static final String TRAP_POWER = "trap.power";
	public static final String TRIGGER_TYPE = "trap.triggerType";
	public static final String TRAP_TYPE = "trap.trapType";
	public static final String TIMER_TIME = "trap.detonateTime";
	public static final String PROXIMITY_RANGE = "trap.proximityRange";
	public static final String TRAP_TRIGGER_ID = "trap.triggerId";
	public static final String TRAP_MY_TRAP = "trap.myTrap";
	public static final String TRAP_MY_PROXIMITY_TRIGGER = "trap.myTrap";
	public static final String TRAP_FACTION = "trap.faction";
	public static final String TRAP_PVP_TYPE = "trap.pvpType";
	public static final String TRAP_DIFFICULTY = "trap.difficulty";
	public static final String TRAP_FREQUENCY = "trap.frequency";
	public static final String TRAP_GROUP = "trap.groupObject";
	public static final String DETECT_EFFECTIVENESS = "trap.detect.effectiveness";
	public static final String CONCEALABLE_DEVICE = "concealable";
	public static final String CONCEALED_OBJECT = "isObjectConcealed";
	public static final String CAMO_SKILL = "camouflageSkill";
	public static final String DETECT_ATTEMPTS = "detectAttempts";
	public static final String DETECT_TIMES = "detectTimes";
	public static final String CAMOUFLAGED_AT_LEVEL = "camouflagedAtLevel";
	public static final String TRAP_LEVEL = "levelOfTrap";
	public static final String BIO_PROBE_TARGET_NAME = "trap.targetName";
	public static final String BIO_PROBE_TRAP_TARGET = "trap.probeTarget";
	public static final String BIO_PROBE_DECAYED = "probe.decayed";
	public static final String BIO_PROBE_SAMPLE_TIME = "probe.sampleTime";
	public static final String BIO_PROBE_STORAGE_TIME = "probe.storageTime";
	public static final String BEACON_BATTERY = "beacon.battery";
	public static final String BEACON_LOCATION = "beacon.location";
	public static final String BEACON_TARGET = "beacon.target";
	public static final String BEACON_TARGET_NAME = "beacon.targetName";
	public static final String BEACON_MSG_DISPATCHED = "beacon.msgDispatched";
	public static final String BEACON_SHUTDOWN_PENDING = "beacon.shutdownPending";
	public static final String BEACON_LAST_KNOWN_LOC = "beacon.lastKnownLoc";
	public static final String BEACON_LAST_KNOWN_TARGET_NAME = "beacon.lastKnownTargetName";
	public static final String BEACON_LAST_KNOWN_TARGET = "beacon.lastKnownTarget";
	public static final String SENSOR_ARMED_TIME = "sensor.armedTime";
	public static final String BUFF_NO_BREAK_INVIS = "no_break_invis";
	
	public static final float SENSOR_RANGE = 10f;
	public static final float BEACON_INTERVAL = 20f;
	
	public static final int TRIGGER_TYPE_TIMER = 1;
	public static final int TRIGGER_TYPE_REMOTE = 2;
	public static final int TRIGGER_TYPE_PROXIMITY = 3;
	
	public static final int TRAP_CALTROP = 1;
	public static final int TRAP_FLASHBANG = 2;
	public static final int TRAP_HX2 = 3;
	public static final int TRAP_KAMINODART = 4;
	
	public static final int[] ALL_TRAP_TYPES = new int[]
	{
		TRAP_CALTROP, TRAP_FLASHBANG, TRAP_HX2, TRAP_KAMINODART
	};
	public static final int[] ALL_TRIGGER_TYPES = new int[]
	{
		TRIGGER_TYPE_TIMER, TRIGGER_TYPE_REMOTE, TRIGGER_TYPE_PROXIMITY
	};
	
	public static final float TRAP_MIN_DISTANCE_BETWEEN = 10f;
	public static final float TRAP_SELF_DESTRUCT_TIMEOUT = 60 * 10;
	public static final float TRAP_HX2_BLAST_RADIUS = 15f;
	public static final float TRAP_FLASHBANG_BLAST_RADIUS = 20f;
	public static final float TRAP_CALTROP_BLAST_RADIUS = 10f;
	public static final float TRAP_KAMINODART_BLAST_RADIUS = 10f;
	
	public static final float KAMINODART_MAX_DURATION = 30;
	public static final float KAMINODART_MAX_POTENCY = 175;
	public static final float KAMINODART_MAX_STRENGTH = 175;
	
	public static final int CALTROP_MAX_SNARE = 65;
	public static final int CALTROP_MAX_DURATION = 30;
	
	public static final int FLASHBANG_MAX_COMBAT_SLOW = -100;
	public static final int FLASHBANG_MAX_DURATION = 60;
	
	public static final int TRAP_HX2_MIN_BASE_DAMAGE = 4000;
	public static final int TRAP_HX2_MAX_BASE_DAMAGE = 5500;
	
	public static final float BASE_DETECT_CAMOUFLAGE_DISTANCE = 20;
	public static final int PLAYER_DETECT_SAFETY_INTERVAL = 60;
	public static final int MAX_INVIS_BUFF_TO_DETECT = 5;
	
	public static final float BASE_DISARM_CHANCE = 55f;
	public static final float MAX_CHANCE_TO_DISARM = 95f;
	public static final float MIN_CHANCE_TO_DISARM = 10f;
	public static final float PERCENT_EFFECT_MOD_DISARM_FAIL = 2f;
	public static final float CHAFF_FLARE_DISTANCE = 15f;
	public static final float MAX_BIO_PROBE_STORAGE_TIME = 3600;
	public static final float MAX_PROBE_DISTANCE = 10f;
	
	public static final String[] TRAP_RESTRICTED_REGIONS =
	{
		"dathomir_fs_village_unpassable"
	};
	
	
	public static void setBioProbeData(obj_id trap, obj_id target, String targetName) throws InterruptedException
	{
	}
	
	
	public static void setTriggerData(obj_id trap, int triggerType, int accuracy, int param) throws InterruptedException
	{
		setObjVar(trap, TRAP_TR_ACCURACY, accuracy);
		setObjVar(trap, TRIGGER_TYPE, triggerType);
		
		switch(triggerType)
		{
			case TRIGGER_TYPE_TIMER:
			setObjVar(trap, TIMER_TIME, param);
			break;
			case TRIGGER_TYPE_REMOTE:
			String frequency = ""+ rand(0, 999) + "."+ rand(0, 999) + "."+ rand(0, 999);
			setObjVar(trap, stealth.TRAP_FREQUENCY, frequency);
			break;
			case TRIGGER_TYPE_PROXIMITY:
			setObjVar(trap, PROXIMITY_RANGE, param);
			break;
		}
	}
	
	
	public static void createRangerLoot(int level, String object, obj_id container, int count) throws InterruptedException
	{
		object = toLower(object);
		String template = "";
		int type = 1;
		int trigType = 1;
		int power = getPowerLevel(level, false);
		
		if (count < 0)
		{
			count = getNumLootItems(level, container);
		}
		
		int trigHash = getStringCrc(object.substring(0, 3));
		int trapHash = getStringCrc(object.substring(4));
		
		switch(trigHash)
		{
			case (265639046):
			trigType = TRIGGER_TYPE_PROXIMITY;
			break;
			case (-1964272009):
			trigType = TRIGGER_TYPE_TIMER;
			break;
			case (-1799474985):
			trigType = TRIGGER_TYPE_REMOTE;
			break;
			default:
			trapHash = getStringCrc(object);
			break;
		}
		
		switch(trapHash)
		{
			case (-2001914999):
			for (int i = 0; i < count; i++)
			{
				testAbortScript();
				type = ALL_TRAP_TYPES[rand(0, ALL_TRAP_TYPES.length-1)];
				trigType = ALL_TRIGGER_TYPES[rand(0, ALL_TRIGGER_TYPES.length-1)];
				createTrapObject(container, getPowerLevel(level, true), type, 1, trigType, getPowerLevel(level, true), rand(5, 20));
			}
			break;
			case (-279247418):
			createMotionSensorObject(container, power, count);
			break;
			case (-1752438899):
			createChaffFlareObject(container, power, count);
			break;
			case (487387167):
			createHEPObject(container, power, count);
			break;
			case (1253827732):
			createBioProbeObject(container, power, count);
			break;
			case (579649147):
			createTrackingBeaconObject(container, power, count);
			break;
			case (240293015):
			createTrapObject(container, power, TRAP_CALTROP, count, trigType, power, 10);
			break;
			case (1254944108):
			createTrapObject(container, power, TRAP_FLASHBANG, count, trigType, power, 10);
			break;
			case (546611802):
			createTrapObject(container, power, TRAP_HX2, count, trigType, power, 10);
			break;
			case (-1152836626):
			createTrapObject(container, power, TRAP_KAMINODART, count, trigType, power, 10);
		}
		return;
	}
	
	
	public static void createRangerLoot(obj_id mob, int level, String object, obj_id container) throws InterruptedException
	{
		createRangerLoot(level, object, container, -1);
		return;
	}
	
	
	public static int getNumLootItems(int level, obj_id destContainer) throws InterruptedException
	{
		float lvl = level;
		int numItems = 1 + (int)Math.ceil(level/30);
		
		int free = getVolumeFree(destContainer);
		if (free < numItems)
		{
			numItems = free;
		}
		
		return numItems;
	}
	
	
	public static int getPowerLevel(int level, boolean includeVariance) throws InterruptedException
	{
		if (includeVariance)
		{
			return loot.getCalculatedAttribute(500, 1100, level, loot.MIN_CREATURE_LEVEL, loot.MAX_CREATURE_LEVEL);
		}
		
		float minPower = 500;
		float maxPower = 1000;
		
		float difference = maxPower - minPower;
		float ratio = level/100f;
		
		return (int)Math.ceil(minPower + (difference * ratio));
	}
	
	
	public static obj_id createHEPObject(obj_id container, int power, int count) throws InterruptedException
	{
		String template = "object/tangible/scout/misc/hep.iff";
		
		obj_id hep = null;
		for (int i = 0; i < count; i++)
		{
			testAbortScript();
			hep = createObject(template, container, "");
			if (!isIdValid(hep))
			{
				return null;
			}
			
			if (rand(0, 1) == 0)
			{
				setObjVar(hep, HEP_BASE_POWER, power);
				setCount(hep, power);
				setObjVar(hep, HEP_EFFECTIVENESS, Math.round(power / 2));
			}
			else
			{
				setObjVar(hep, HEP_EFFECTIVENESS, power);
				power = Math.round(power/2);
				setObjVar(hep, HEP_BASE_POWER, power);
				setCount(hep, power);
			}
		}
		return hep;
	}
	
	
	public static obj_id createTrackingBeaconObject(obj_id container, float power, int count) throws InterruptedException
	{
		String template = "object/tangible/scout/misc/trackingbeacon.iff";
		
		obj_id probe = null;
		for (int i = 0; i < count; i++)
		{
			testAbortScript();
			probe = createObject(template, container, "");
			if (!isIdValid(probe))
			{
				return null;
			}
			
			setObjVar(probe, BEACON_BATTERY, power * 3);
			setObjVar(probe, DETECT_EFFECTIVENESS, power);
			setObjVar(probe, BIO_PROBE_STORAGE_TIME, MAX_BIO_PROBE_STORAGE_TIME);
			
		}
		return probe;
	}
	
	
	public static obj_id createBioProbeObject(obj_id container, float power, int count) throws InterruptedException
	{
		String template = "object/tangible/scout/misc/biosensorprobe.iff";
		
		obj_id probe = null;
		for (int i = 0; i < count; i++)
		{
			testAbortScript();
			
			probe = createObject(template, container, "");
			if (!isIdValid(probe))
			{
				return null;
			}
			
			setObjVar(probe, BIO_PROBE_STORAGE_TIME, power / 1000 * MAX_BIO_PROBE_STORAGE_TIME);
		}
		return probe;
	}
	
	
	public static obj_id createMotionSensorObject(obj_id container, float batteryPower, int count) throws InterruptedException
	{
		String template = "object/tangible/scout/misc/motionsensor.iff";
		
		obj_id sensor = null;
		for (int i = 0; i < count; i++)
		{
			testAbortScript();
			sensor = createObject(template, container, "");
			if (!isIdValid(sensor))
			{
				return null;
			}
			
			setObjVar(sensor, BEACON_BATTERY, batteryPower);
		}
		return sensor;
	}
	
	
	public static obj_id createChaffFlareObject(obj_id container, int power, int count) throws InterruptedException
	{
		String template = "object/tangible/scout/misc/chaffflare.iff";
		
		obj_id flare = null;
		for (int i = 0; i < count; i++)
		{
			testAbortScript();
			flare = createObject(template, container, "");
			if (!isIdValid(flare))
			{
				return null;
			}
			
			setObjVar(flare, DETECT_EFFECTIVENESS, power);
		}
		
		return flare;
	}
	
	
	public static obj_id createTrapObject(obj_id container, int power, int trapType, int count, int triggerType, int triggerAccuracy, int triggerParam) throws InterruptedException
	{
		String template = "";
		switch(trapType)
		{
			case TRAP_CALTROP:
			template = "object/tangible/scout/trap/trap_ap_caltrop.iff";
			break;
			case TRAP_FLASHBANG:
			template = "object/tangible/scout/trap/trap_ap_flashbang.iff";
			break;
			case TRAP_HX2:
			template = "object/tangible/scout/trap/trap_ap_hx2mine.iff";
			break;
			case TRAP_KAMINODART:
			template = "object/tangible/scout/trap/trap_ap_kaminoandart.iff";
			break;
			default:
			template = "object/tangible/scout/trap/trap_ap_hx2mine.iff";
			break;
		}
		
		obj_id trap = null;
		trap = createObject(template, container, "");
		if (!isIdValid(trap))
		{
			return null;
		}
		
		setObjVar(trap, TRAP_POWER, power);
		setTriggerData(trap, triggerType, triggerAccuracy, triggerParam);
		setCount(trap, count);
		
		return trap;
	}
	
	
	public static boolean canPlaceMotionSensor(obj_id player, obj_id sensor) throws InterruptedException
	{
		obj_id[] stuff = getNonCreaturesInRange(getLocation(player), TRAP_MIN_DISTANCE_BETWEEN * 2);
		for (int i = 0; i < stuff.length; i++)
		{
			testAbortScript();
			if (hasObjVar(stuff[i], TRAP_TYPE) || hasObjVar(stuff[i], BEACON_BATTERY))
			{
				sendSystemMessage(player, new string_id("spam", "other_trap"));
				return false;
			}
		}
		
		return true;
	}
	
	
	public static void placeMotionSensor(obj_id player, obj_id sensor) throws InterruptedException
	{
		location loc = getLocation(player);
		if (!setLocation(sensor, loc))
		{
			return;
		}
		
		setObjVar(sensor, "armed", 1);
		messageTo(sensor, "msgBatteryDead", null, getFloatObjVar(sensor, BEACON_BATTERY), true);
		setObjVar(sensor, SENSOR_ARMED_TIME, getGameTime());
		doAnimationAction(player, "point_down");
		setOwner(sensor, player);
		playClientEffectLoc(sensor, "clienteffect/frs_light_vigilance.cef", getLocation(sensor), 0.0f);
		messageTo(sensor, "msgDeploy", null, 1, false);
	}
	
	
	public static void tripMotionSensor(obj_id sensor, obj_id breacher, boolean wasFiltered) throws InterruptedException
	{
		obj_id owner = getOwner(sensor);
		if (!isIdValid(owner))
		{
			return;
		}
		
		if (hasObjVar(sensor, "lastSent"))
		{
			int lastTime = getIntObjVar(sensor, "lastSent");
			if (lastTime + 30 > getGameTime())
			{
				return;
			}
		}
		
		setObjVar(sensor, "lastSent", getGameTime());
		dictionary dic = new dictionary();
		dic.put("sensor", sensor);
		dic.put("breacherId", breacher);
		dic.put("breacherName", isPlayer(breacher)? getName(breacher) : getCreatureName(breacher));
		dic.put("wasFiltered", wasFiltered);
		dic.put("sensorLoc", getLocation(sensor));
		
		messageTo(owner, "msgMotionSensorTripped", dic, 1, false);
		
		return;
	}
	
	
	public static void cleanComponentFromContainer(obj_id component, obj_id container) throws InterruptedException
	{
		if (!isIdValid(container))
		{
			return;
		}
		
		if (!hasObjVar(container, DETECT_EFFECTIVENESS) && !hasObjVar(container, TRAP_TYPE) && !hasObjVar(container, BEACON_BATTERY))
		{
			return;
		}
		
		obj_var_list ovl = getObjVarList(component, "trap");
		if (ovl == null)
		{
			return;
		}
		
		int numItems = ovl.getNumItems();
		String name = "";
		for (int i = 0; i < numItems; i++)
		{
			testAbortScript();
			obj_var ov = ovl.getObjVar(i);
			if (ov != null)
			{
				name = "trap."+ ov.getName();
				removeObjVar(container, name);
			}
		}
		
		return;
	}
	
	
	public static boolean canActivateTrackingBeacon(obj_id player, obj_id beacon, boolean silent) throws InterruptedException
	{
		if ((!utils.testItemClassRequirements(player, beacon, false, "") || !utils.testItemLevelRequirements(player, beacon, false, "")))
		{
			return false;
		}
		
		obj_id target = getBioProbeTarget(beacon);
		if (!isIdValid(target))
		{
			if (!silent)
			{
				sendSystemMessage(player, new string_id("spam", "bio_sample_bad"));
				return false;
			}
		}
		
		if (getFloatObjVar(beacon, BEACON_BATTERY) < BEACON_INTERVAL)
		{
			if (!silent)
			{
				sendSystemMessage(player, new string_id("spam", "traker_power_low"));
				return false;
			}
		}
		
		if (hasObjVar(beacon, BEACON_TARGET))
		{
			if (getObjIdObjVar(beacon, BEACON_TARGET) != target)
			{
				sendSystemMessage(player, new string_id("spam", "different_sample"));
				return false;
			}
		}
		
		return true;
	}
	
	
	public static void startTrackingBeacon(obj_id player, obj_id beacon) throws InterruptedException
	{
		obj_id target = getObjIdObjVar(beacon, BIO_PROBE_TRAP_TARGET);
		if (!isIdValid(target))
		{
			return;
		}
		
		dictionary dic = new dictionary();
		dic.put("sendingPlayer", player);
		dic.put("sendingBeacon", beacon);
		
		sendSystemMessage(player, new string_id("spam", "uploading_tracking"));
		
		removeObjVar(beacon, BEACON_SHUTDOWN_PENDING);
		if (!hasObjVar(beacon, BEACON_MSG_DISPATCHED))
		{
			setObjVar(beacon, BEACON_MSG_DISPATCHED, 1);
			
			messageTo(beacon, "msgTrackingBeaconUpdate", dic, BEACON_INTERVAL, true);
			messageTo(target, "msgTrackingBeaconLocationRequest", dic, 0, false);
		}
		return;
	}
	
	
	public static void stopTrackingBeacon(obj_id beacon, obj_id player) throws InterruptedException
	{
		if (hasObjVar(beacon, BEACON_MSG_DISPATCHED))
		{
			sendSystemMessage(player, new string_id("spam", "requestin_disconnect"));
			setObjVar(beacon, BEACON_SHUTDOWN_PENDING, 1);
		}
		else if (isIdValid(player))
		{
			sendSystemMessage(player, new string_id("spam", "connection_severed"));
			removeObjVar(beacon, BEACON_LOCATION);
			removeObjVar(beacon, BEACON_TARGET);
			removeObjVar(beacon, BEACON_TARGET_NAME);
		}
		
		return;
	}
	
	
	public static void updateTrackingBeaconDisplay(obj_id beacon) throws InterruptedException
	{
		obj_id player = utils.getContainingPlayer(beacon);
		removeObjVar(beacon, BEACON_MSG_DISPATCHED);
		if (hasObjVar(beacon, BEACON_SHUTDOWN_PENDING))
		{
			stopTrackingBeacon(beacon, player);
			return;
		}
		
		float power = getFloatObjVar(beacon, BEACON_BATTERY);
		power -= BEACON_INTERVAL;
		setObjVar(beacon, BEACON_BATTERY, power);
		
		if (!isIdValid(player))
		{
			stopTrackingBeacon(beacon, player);
			return;
		}
		
		obj_id target = getObjIdObjVar(beacon, BEACON_TARGET);
		if (!isIdValid(target))
		{
			sendSystemMessage(player, new string_id("spam", "tracking_failed"));
		}
		else
		{
			setObjVar(beacon, BEACON_LAST_KNOWN_LOC, getLocationObjVar(beacon, BEACON_LOCATION));
			setObjVar(beacon, BEACON_LAST_KNOWN_TARGET_NAME, getStringObjVar(beacon, BEACON_TARGET_NAME));
			setObjVar(beacon, BEACON_LAST_KNOWN_TARGET, target);
			
			prose_package pp = prose.getPackage(new string_id("spam", "tracker_updated"));
			pp = prose.setTT(pp, target);
			sendSystemMessageProse(player, pp);
		}
		
		if (canActivateTrackingBeacon(player, beacon, false))
		{
			if (!hasObjVar(beacon, BEACON_MSG_DISPATCHED))
			{
				dictionary dic = new dictionary();
				dic.put("sendingPlayer", player);
				dic.put("sendingBeacon", beacon);
				sendSystemMessage(player, new string_id("spam", "requesting_target"));
				setObjVar(beacon, BEACON_MSG_DISPATCHED, 1);
				messageTo(beacon, "msgTrackingBeaconUpdate", dic, BEACON_INTERVAL, true);
				messageTo(getObjIdObjVar(beacon, BEACON_TARGET), "msgTrackingBeaconLocationRequest", dic, 0, false);
			}
		}
		else
		{
			stopTrackingBeacon(beacon, player);
		}
		
		return;
	}
	
	
	public static void updateTrackingBeaconData(obj_id beacon, obj_id target, location targetLoc, String targetName) throws InterruptedException
	{
		setObjVar(beacon, BEACON_LOCATION, targetLoc);
		setObjVar(beacon, BEACON_TARGET, target);
		setObjVar(beacon, BEACON_TARGET_NAME, targetName);
	}
	
	
	public static obj_id getBioProbeTarget(obj_id thing) throws InterruptedException
	{
		obj_id target = null;
		if ((hasObjVar(thing, TRAP_TYPE) || hasObjVar(thing, BEACON_BATTERY)) && hasObjVar(thing, BIO_PROBE_TRAP_TARGET))
		{
			if (hasObjVar(thing, BIO_PROBE_STORAGE_TIME))
			{
				
				if (!hasObjVar(thing, BIO_PROBE_DECAYED))
				{
					target = getObjIdObjVar(thing, BIO_PROBE_TRAP_TARGET);
				}
				
			}
		}
		
		return target;
	}
	
	
	public static boolean canBioProbe(obj_id player, obj_id probe, obj_id target) throws InterruptedException
	{
		if (!isIdValid(target))
		{
			showFlyTextPrivate(player, player, new string_id("spam", "bad_target"), 1.5f, colors.TOMATO);
			return false;
		}
		
		if (!ai_lib.isHumanoid(target))
		{
			sendSystemMessage(player, new string_id("spam", "sample_humanoid"));
			return false;
		}
		
		if (!pvpCanAttack(player, target))
		{
			sendSystemMessage(player, new string_id("spam", "cant_sample"));
			return false;
		}
		
		if (getDistance(getLocation(player), getLocation(target)) > MAX_PROBE_DISTANCE)
		{
			showFlyTextPrivate(player, player, new string_id("spam", "too_far"), 1.5f, colors.TOMATO);
			return false;
		}
		
		return true;
	}
	
	
	public static void bioProbe(obj_id player, obj_id probe, obj_id target) throws InterruptedException
	{
		float effectiveness = getIntObjVar(probe, DETECT_EFFECTIVENESS);
		float timeToDecay = getFloatObjVar(probe, BIO_PROBE_STORAGE_TIME);
		
		setObjVar(probe, BIO_PROBE_SAMPLE_TIME, getGameTime());
		messageTo(probe, "msgDecaySample", null, timeToDecay, true);
		
		doAnimationAction(player, "heal_other");
		setObjVar(probe, BIO_PROBE_TRAP_TARGET, target);
		setObjVar(probe, BIO_PROBE_TARGET_NAME, isPlayer(target)? getName(target) : "a "+ getCreatureName(target));
		
		if (isPlayer(target))
		{
			prose_package pp = prose.getPackage(new string_id("spam", "feel_twinge"));
			pp = prose.setTT(pp, player);
			sendSystemMessageProse(target, pp);
		}
		
		prose_package pp = prose.getPackage(new string_id("spam", "got_probe"));
		pp = prose.setTT(pp, target);
		sendSystemMessageProse(player, pp);
		
		return;
	}
	
	
	public static void doChaffFlareEffect(obj_id player, obj_id flare) throws InterruptedException
	{
		doAnimationAction(player, "manipulate_high");
		playClientEffectLoc(player, "appearance/pt_sparking_blast_md.prt", getLocation(player), 0.0f);
		float flareDistance = CHAFF_FLARE_DISTANCE;
		float detectSkill = getEnhancedSkillStatisticModifier(player, "detect_hidden");
		obj_id[] stuff = getObjectsInRange(player, flareDistance);
		boolean foundSomething = false;
		prose_package pp = prose.getPackage(new string_id("spam", "chaff_uncover"));
		flareDistance += (detectSkill / 10);
		
		pp = prose.setTT(pp, player);
		for (int i = 0; i < stuff.length; i++)
		{
			testAbortScript();
			if (!hasObjVar(stuff[i], TRAP_TYPE) || !validateTrapTarget(stuff[i], player))
			{
				continue;
			}
			
			int hidePower = getIntObjVar(stuff[i], CAMO_SKILL);
			int chaffPower = getIntObjVar(flare, DETECT_EFFECTIVENESS);
			
			if (hidePower > chaffPower)
			{
				continue;
			}
			
			pp = prose.setTU(pp, stuff[i]);
			
			foundSomething = true;
			unconcealDevice(player, stuff[i]);
		}
		
		if (foundSomething)
		{
			sendSystemMessage(player, new string_id("spam", "chaff_foundsomething"));
			doAnimationAction(player, "point_forward");
		}
		else
		{
			sendSystemMessage(player, new string_id("spam", "chaff_nothing"));
		}
		
		return;
	}
	
	
	public static boolean canDisarmTrap(obj_id player, obj_id trap) throws InterruptedException
	{
		if (!hasObjVar(trap, TRAP_TYPE) && !hasObjVar(trap, BEACON_BATTERY))
		{
			sendSystemMessage(player, new string_id("spam", "bad_disarm_target"));
			return false;
		}
		
		if (!validateTrapTarget(trap, player) && getOwner(trap) != player)
		{
			sendSystemMessage(player, new string_id("spam", "cant_disarm"));
			return false;
		}
		
		return true;
	}
	
	
	public static void disarmTrap(obj_id player, obj_id trap) throws InterruptedException
	{
		
		if (getOwner(trap) == player || hasObjVar(trap, BEACON_BATTERY))
		{
			showFlyText(trap, new string_id("spam", "disarm_go"), 1.5f, colors.TOMATO);
			sendSystemMessage(player, new string_id("spam", "disarm"));
			playTrapFizzle(trap);
			destroyObject(trap);
			return;
		}
		
		float baseChanceToDisarm = BASE_DISARM_CHANCE;
		
		int trapLevel = getIntObjVar(trap, TRAP_LEVEL);
		int trapSkillMod = getIntObjVar(trap, TRAP_DIFFICULTY);
		float trapperScore = trapLevel + trapSkillMod;
		
		int diffuserLevel = getLevel(player);
		int diffuserSkillMod = getEnhancedSkillStatisticModifier(player, "ranger_trap");
		int spyBonus = utils.isProfession(player, utils.SPY)? 30 : 0;
		float diffuserScore = diffuserLevel + diffuserSkillMod + spyBonus;
		
		float totalDisparity = diffuserScore - trapperScore;
		float chanceToDetect = baseChanceToDisarm;
		float detectRange = MAX_CHANCE_TO_DISARM - MIN_CHANCE_TO_DISARM;
		
		if (totalDisparity > 0)
		{
			chanceToDetect += (totalDisparity / MAX_HIDING_DETECT_SCORE) * detectRange;
		}
		
		if (chanceToDetect > MAX_CHANCE_TO_DISARM)
		{
			chanceToDetect = MAX_CHANCE_TO_DISARM;
		}
		
		float roll = rand(0f, 100f);
		
		if (roll < chanceToDetect)
		{
			
			if (hasObjVar(trap, stealth.TRAP_FREQUENCY))
			{
				obj_id myRemote = getObjIdObjVar(trap, stealth.TRAP_MY_PROXIMITY_TRIGGER);
				
				if (exists(myRemote))
				{
					messageTo(myRemote, "trapDisarmed", null, 1, false);
				}
			}
			
			showFlyText(trap, new string_id("spam", "disarm_go"), 1.5f, colors.TOMATO);
			sendSystemMessage(player, new string_id("spam", "disarm"));
			playTrapFizzle(trap);
			destroyObject(trap);
		}
		else
		{
			
			if (hasObjVar(trap, stealth.TRAP_FREQUENCY))
			{
				obj_id myRemote = getObjIdObjVar(trap, stealth.TRAP_MY_PROXIMITY_TRIGGER);
				
				if (exists(myRemote))
				{
					messageTo(myRemote, "trapDisarmed", null, 1, false);
				}
			}
			
			Vector target = new Vector();
			target.add(player);
			doTrapDetonateEffect(trap, getIntObjVar(trap, TRAP_TYPE), target, PERCENT_EFFECT_MOD_DISARM_FAIL);
			sendSystemMessage(player, new string_id("spam", "fail_disarm"));
		}
		
		return;
	}
	
	
	public static boolean canDetectCamouflage(obj_id player) throws InterruptedException
	{
		return true;
	}
	
	
	public static void detectCamouflage(obj_id player, boolean findMobiles, boolean findInanimate, float distance, float baseChanceToDetect) throws InterruptedException
	{
		obj_id[] stuff = getObjectsInRange(player, distance);
		Vector detected = new Vector();
		Vector times = new Vector();
		int now = getGameTime();
		boolean foundSomething = false;
		obj_id target = null;
		prose_package pp = prose.getPackage(new string_id("spam", "revealed_by"));
		String invis = "";
		boolean objIsMobile = false;
		location playerLoc = getLocation(player);
		
		playClientEffectLoc(getPlayerCreaturesInRange(playerLoc, 100.0f), "appearance/pt_spy_reveal_wave.prt", playerLoc, 0.0f);
		
		if (utils.isProfession(player, utils.SPY))
		{
			invis = getInvisBuff(player);
			
			if (invis == null)
			{
				playClientEffectLoc(player, "appearance/pt_spy_expose_shadows.prt", getLocation(player), 0.0f);
			}
			else
			{
				Vector clients = new Vector();
				clients = utils.addElement(clients, player);
				
				dictionary revealList = getPassiveRevealList(player);
				
				if (revealList != null)
				{
					Vector ids = revealList.getResizeableObjIdArray("id");
					if ((ids != null) && (ids.size() != 0))
					{
						clients = utils.concatArrays(clients, ids);
					}
				}
				
				playClientEffectLoc(utils.toStaticObjIdArray(clients), "appearance/pt_spy_expose_shadows.prt", getLocation(player), 0.0f);
			}
		}
		else
		{
			playClientEffectLoc(player, "appearance/pt_fireworks_secondary_shockwave2.prt", getLocation(player), 0.0f);
		}
		
		for (int i = 0; i < stuff.length; i++)
		{
			testAbortScript();
			objIsMobile = false;
			target = stuff[i];
			
			if (!isIdValid(target))
			{
				continue;
			}
			
			if (isPlayer(target) || ai_lib.isHumanoid(target) || isMob(target))
			{
				objIsMobile = true;
				if (!pvpCanAttack(player, target))
				{
					continue;
				}
				
				invis = getInvisBuff(target);
				if (invis == null)
				{
					continue;
				}
				else if (buff.getPriority(invis) > MAX_INVIS_BUFF_TO_DETECT)
				{
					continue;
				}
				
				if (invis.startsWith("invis_sp_buff_invis_notrace") || invis.startsWith("invis_sp_diversion_stealth"))
				{
					continue;
					
				}
				
				detected = utils.getResizeableStringBatchObjVar(target, DETECT_ATTEMPTS);
				times = utils.getResizeableStringBatchObjVar(target, DETECT_TIMES);
				if (detected == null)
				{
					detected = new Vector();
				}
				
				if (times == null)
				{
					times = new Vector();
				}
				
				if (detected.size() != times.size())
				{
					
					continue;
				}
				
				int idx = detected.indexOf(player);
				if (idx > -1)
				{
					int when = ((Integer)times.get(idx)).intValue();
					if (when - now < PLAYER_DETECT_SAFETY_INTERVAL)
					{
						continue;
					}
					
					times.setElementAt(new Integer(now), idx);
				}
				else
				{
					detected.add(player);
					times.add(new Integer(now));
				}
				
				utils.setResizeableBatchObjVar(target, DETECT_ATTEMPTS, detected);
				utils.setResizeableBatchObjVar(target, DETECT_TIMES, times);
			}
			else if (!hasObjVar(target, CONCEALED_OBJECT) || !validateTrapTarget(target, player))
			{
				continue;
			}
			
			if (activeDetectHiddenTarget(target, player, baseChanceToDetect))
			{
				if (objIsMobile)
				{
					checkForAndMakeVisible(target);
				}
				else
				{
					unconcealDevice(player, target);
				}
				foundSomething = true;
				playClientEffectLoc(target, "clienteffect/lair_med_damage_smoke.cef", getLocation(target), 0.0f);
			}
			
		}
		
		if (foundSomething)
		{
			sendSystemMessage(player, new string_id("spam", "detected_something"));
			doAnimationAction(player, "point_forward");
		}
		else
		{
			sendSystemMessage(player, new string_id("spam", "no_detect"));
		}
		
		return;
	}
	
	
	public static boolean canConcealDevice(obj_id player, obj_id target) throws InterruptedException
	{
		if (!isIdValid(target) || !exists(target))
		{
			
			return false;
		}
		
		if (!hasObjVar(target, CONCEALABLE_DEVICE))
		{
			sendSystemMessage(player, new string_id("spam", "cant_conceal_that"));
			return false;
		}
		
		if (hasObjVar(target, CONCEALED_OBJECT))
		{
			sendSystemMessage(player, new string_id("spam", "already_concealed"));
			return false;
		}
		
		obj_id owner = getOwner(target);
		if (owner != player)
		{
			if (!group.inSameGroup(player, owner))
			{
				sendSystemMessage(player, new string_id("spam", "cant_conceal_owner"));
				return false;
			}
		}
		
		if (getDistance(getLocation(target), getLocation(player)) > 15)
		{
			sendSystemMessage(player, new string_id("spam", "too_far_conceal"));
			return false;
		}
		
		return true;
	}
	
	
	public static void concealDevice(obj_id player, obj_id target) throws InterruptedException
	{
		int camoSkill = getEnhancedSkillStatisticModifier(player, "camouflage");
		camoSkill += getEnhancedSkillStatisticModifier(player, "stealth");
		setObjVar(target, CONCEALED_OBJECT, 1);
		setObjVar(target, CAMO_SKILL, camoSkill);
		setObjVar(target, CAMOUFLAGED_AT_LEVEL, getLevel(player));
		
		hideFromClient(target, true);
	}
	
	
	public static void unconcealDevice(obj_id player, obj_id target) throws InterruptedException
	{
		removeObjVar(target, CONCEALED_OBJECT);
		removeObjVar(target, CAMO_SKILL);
		removeObjVar(target, CAMOUFLAGED_AT_LEVEL);
		hideFromClient(target, false);
		
		location loc = getLocation(target);
		loc.y -= 10;
		playClientEffectLoc(player, "clienteffect/frs_dark_vengeance.cef", loc, 0);
		stopClientEffectObjByLabel(target, "trapConceal");
		doAnimationAction(player, "point_down");
	}
	
	
	public static boolean canSetTrap(obj_id player, obj_id trap) throws InterruptedException
	{
		if (!isIdValid(trap) || !hasScript(trap, "systems.skills.stealth.trap"))
		{
			
			return false;
		}
		
		if (ai_lib.isInCombat(player))
		{
			sendSystemMessage(player, new string_id("spam", "trap_not_in_combat"));
			return false;
		}
		
		if (!utils.isNestedWithin(trap, player))
		{
			sendSystemMessage(player, new string_id("spam", "trap_not_possession"));
			return false;
		}
		
		if (isPvPTrap(trap) && !factions.isDeclared(player))
		{
			sendSystemMessage(player, new string_id("spam", "trap_not_declared"));
			return false;
		}
		
		if (pet_lib.isMounted(player))
		{
			sendSystemMessage(player, new string_id("spam", "trap_not_while_mounted"));
			return false;
		}
		
		if (!hasObjVar(trap, TRIGGER_TYPE))
		{
			sendSystemMessage(player, new string_id("spam", "trap_no_trigger_installed"));
			return false;
		}
		
		if (locations.isInCity(getLocation(player)))
		{
			sendSystemMessage(player, new string_id("spam", "no_trap_city"));
			return false;
		}
		
		if (inTrapRestrictedRegion(player))
		{
			sendSystemMessage(player, new string_id("spam", "trap_restricted_region"));
			return false;
		}
		
		if (utils.isInHouseCellSpace(player))
		{
			obj_id house = getTopMostContainer(player);
			if (hasObjVar( house, "player_structure.admin.adminList" ))
			{
				sendSystemMessage(player, new string_id("spam", "trap_not_house"));
				return false;
			}
			
		}
		
		obj_id[] stuff = getNonCreaturesInRange(getLocation(player), TRAP_MIN_DISTANCE_BETWEEN);
		for (int i = 0; i < stuff.length; i++)
		{
			testAbortScript();
			if (hasObjVar(stuff[i], TRAP_TYPE))
			{
				sendSystemMessage(player, new string_id("spam", "other_trap"));
				return false;
			}
		}
		
		return true;
	}
	
	
	public static boolean inTrapRestrictedRegion(obj_id player) throws InterruptedException
	{
		for (int i = 0; i < TRAP_RESTRICTED_REGIONS.length; i++)
		{
			testAbortScript();
			if (groundquests.isInNamedRegion(player, TRAP_RESTRICTED_REGIONS[i]))
			{
				return true;
			}
		}
		return false;
	}
	
	
	public static void onGetTrapAttributes(obj_id self, obj_id player, String[] names, String[] attribs, int free) throws InterruptedException
	{
		
		if (free == -1)
		{
			return;
		}
		
		if (hasObjVar(self, TRAP_TYPE))
		{
			int trapTypeInt = getIntObjVar(self, TRAP_TYPE);
			if (trapTypeInt == TRAP_CALTROP)
			{
				float trapRadiusFlt = TRAP_CALTROP_BLAST_RADIUS;
				names[free] = "trap_radius";
				attribs[free++] = "" + trapRadiusFlt + "m";
			}
			
			if (trapTypeInt == TRAP_FLASHBANG)
			{
				float trapRadiusFlt = TRAP_FLASHBANG_BLAST_RADIUS;
				names[free] = "trap_radius";
				attribs[free++] = "" + trapRadiusFlt + "m";
			}
			
			if (trapTypeInt == TRAP_HX2)
			{
				float trapRadiusFlt = TRAP_HX2_BLAST_RADIUS;
				names[free] = "trap_radius";
				attribs[free++] = "" + trapRadiusFlt + "m";
			}
			
			if (trapTypeInt == TRAP_KAMINODART)
			{
				float trapRadiusFlt = TRAP_KAMINODART_BLAST_RADIUS;
				names[free] = "trap_radius";
				attribs[free++] = "" + trapRadiusFlt + "m";
			}
		}
		
		if (hasObjVar(self, TRAP_POWER))
		{
			names[free] = "trap_power";
			attribs[free++] = "" + floatFormat.format(getIntObjVar(self, TRAP_POWER) / 10) + "%";
		}
		
		if (hasObjVar(self, TIMER_TIME) && (utils.isNestedWithin(self, player) || getOwner(self) == player))
		{
			names[free] = "trigger_time";
			attribs[free++] = "" + getIntObjVar(self, TIMER_TIME) + " seconds";
			
			names[free] = "tooltip.trap_type";
			attribs[free++] = "@spam:trap_trigger_timed";
			
			names[free] = "trap_type";
			attribs[free++] = "@spam:trap_trig_desc_timed";
		}
		
		if (hasObjVar(self, PROXIMITY_RANGE) && (utils.isNestedWithin(self, player) || getOwner(self) == player))
		{
			names[free] = "trigger_proximity";
			
			int proximityRange = getIntObjVar(self, PROXIMITY_RANGE);
			String meterPluralization = " meters";
			
			if (proximityRange == 1)
			{
				meterPluralization = " meter";
			}
			
			attribs[free++] = "" + proximityRange + meterPluralization;
			
			names[free] = "tooltip.trap_type";
			attribs[free++] = "@spam:trap_trigger_proximity";
			
			names[free] = "trap_type";
			attribs[free++] = "@spam:trap_trig_desc_proximity";
		}
		
		if (hasObjVar(self, TRAP_FREQUENCY) && (utils.isNestedWithin(self, player) || getOwner(self) == player))
		{
			names[free] = "tooltip.trap_type";
			attribs[free++] = "@spam:trap_trigger_remote";
			
			names[free] = "trap_type";
			attribs[free++] = "@spam:trap_trig_desc_remote";
		}
		
		if (hasObjVar(self, TRAP_FREQUENCY) && hasObjVar(self, "armed"))
		{
			names[free] = "trap_frequency";
			attribs[free++] = "" + getStringObjVar(self, TRAP_FREQUENCY) + " mHz";
		}
		
		if (hasObjVar(self, BIO_PROBE_TRAP_TARGET))
		{
			names[free] = "trigger_filter";
			attribs[free++] = "" + getStringObjVar(self, BIO_PROBE_TARGET_NAME);
		}
		
		free = utils.addClassRequirementAttributes(player, self, names, attribs, free, "trap.");
		free = utils.addClassRequirementAttributes(player, self, names, attribs, free, "trap.trigger.");
		
		return;
	}
	
	
	public static boolean tryDetonateTrap(obj_id trap, obj_id target) throws InterruptedException
	{
		if (!hasObjVar(trap, "armed"))
		{
			return false;
		}
		
		int trapType = getIntObjVar(trap, TRAP_TYPE);
		int triggerAccuracy = getIntObjVar(trap, TRAP_TR_ACCURACY);
		boolean success = rand(1, 100) <= triggerAccuracy;
		if (!success)
		{
			playTrapFizzle(trap);
			
			if (getIntObjVar(trap, TRIGGER_TYPE) == TRIGGER_TYPE_TIMER)
			{
				dictionary parms = new dictionary();
				int time = getGameTime();
				parms.put("armed", time);
				float newTime = getIntObjVar(trap, TIMER_TIME) * rand(0.1f, 0.25f);
				messageTo(trap, "msgTryDetonateTimer", parms, newTime, false);
				setObjVar(trap, "armed", time);
				
			}
			
			return false;
		}
		
		Vector targets = getTrapTargets(trap, trapType);
		if (validateTrapTarget(trap, target) && targets.indexOf(target) == -1)
		{
			targets.add(target);
		}
		
		hideFromClient(trap, false);
		doTrapDetonateEffect(trap, trapType, targets, 0, target);
		
		return true;
	}
	
	
	public static Vector getTrapTargets(obj_id trap, int trapType) throws InterruptedException
	{
		obj_id[] targets = new obj_id[0];
		switch(trapType)
		{
			case TRAP_HX2:
			targets = getCreaturesInRange(getLocation(trap), TRAP_HX2_BLAST_RADIUS);
			break;
			case TRAP_FLASHBANG:
			targets = getCreaturesInRange(getLocation(trap), TRAP_FLASHBANG_BLAST_RADIUS);
			break;
			case TRAP_CALTROP:
			targets = getCreaturesInRange(getLocation(trap), TRAP_CALTROP_BLAST_RADIUS);
			break;
			case TRAP_KAMINODART:
			targets = getCreaturesInRange(getLocation(trap), TRAP_KAMINODART_BLAST_RADIUS);
			break;
		}
		
		Vector finalTargets = new Vector();
		obj_id filter = getBioProbeTarget(trap);
		
		for (int i = 0; i < targets.length; i++)
		{
			testAbortScript();
			if (!isIdValid(filter) || targets[i] == filter)
			{
				if (validateTrapTarget(trap, targets[i]))
				{
					finalTargets.add(targets[i]);
				}
			}
		}
		
		return finalTargets;
	}
	
	
	public static void doTrapDetonateEffect(obj_id trap, int trapType, Vector targets, float percentMod, obj_id breacher) throws InterruptedException
	{
		if (targets.size() > 0)
		{
			
			switch(trapType)
			{
				case TRAP_HX2:
				dictionary params = new dictionary();
				obj_id owner = getOwner(trap);
				
				if (!isIdValid(breacher))
				{
					breacher = (obj_id)targets.elementAt(0);
					if (!isIdValid(breacher))
					{
						break;
					}
				}
				
				params.put("target", breacher);
				params.put("owner", owner);
				messageTo(trap, "co_mine_hx2", params, 0, false);
				break;
				case TRAP_FLASHBANG:
				doFlashBangDetonate(trap, targets, percentMod);
				break;
				case TRAP_CALTROP:
				doCaltropDetonate(trap, targets, percentMod);
				break;
				case TRAP_KAMINODART:
				doKaminoDartDetonate(trap, targets, percentMod);
				break;
			}
		}
		else
		{
			playClientEffectLoc(trap, "clienteffect/lair_damage_heavy.cef", getLocation(trap), 0.0f);
		}
		
		destroyObject(trap);
	}
	
	
	public static void doTrapDetonateEffect(obj_id trap, int trapType, Vector targets) throws InterruptedException
	{
		doTrapDetonateEffect(trap, trapType, targets, 0);
	}
	
	
	public static void doTrapDetonateEffect(obj_id trap, int trapType, Vector targets, float percentMod) throws InterruptedException
	{
		doTrapDetonateEffect(trap, trapType, targets, 0, null);
	}
	
	
	public static void doKaminoDartDetonate(obj_id trap, Vector targets) throws InterruptedException
	{
		doKaminoDartDetonate(trap, targets, 0);
	}
	
	
	public static void doKaminoDartDetonate(obj_id trap, Vector targets, float percentMod) throws InterruptedException
	{
		
		float trapPower = getIntObjVar(trap, TRAP_POWER);
		int potency = (int) (KAMINODART_MAX_POTENCY * (trapPower/1000));
		int duration = (int) (KAMINODART_MAX_DURATION * (trapPower/1000));
		int strength = (int) (KAMINODART_MAX_STRENGTH * (trapPower/1000));
		obj_id owner = getOwner(trap);
		boolean isInvis = false;
		String invis = null;
		boolean attackerPresent = false;
		if (isIdValid(owner) && exists(owner))
		{
			attackerPresent = true;
			invis = getInvisBuff(getOwner(trap));
		}
		
		isInvis = invis != null && invis.length() > 0;
		duration += duration * percentMod;
		strength += strength * percentMod;
		
		obj_id curObj = null;
		prose_package pp = prose.getPackage(new string_id("spam", "kamino_trap_hit"));
		playClientEffectLoc(trap, "clienteffect/lair_damage_light.cef", getLocation(curObj), 0.0f);
		
		for (int i = 0; i < targets.size(); i++)
		{
			testAbortScript();
			curObj = (obj_id)targets.get(i);
			playClientEffectLoc(curObj, "clienteffect/lair_damage_light.cef", getLocation(curObj), 0.0f);
			dot.applyDotEffect(curObj, owner, dot.DOT_POISON, "rangerTrapKaminoDart", HEALTH, potency, strength, duration, true, null);
			pp = prose.setTT(pp, curObj);
			
			if (attackerPresent)
			{
				if (isInvis && rand(1, 100) < 50)
				{
					continue;
				}
				
				startCombat(curObj, owner);
			}
		}
		
		return;
	}
	
	
	public static void doCaltropDetonate(obj_id trap, Vector targets) throws InterruptedException
	{
		doCaltropDetonate(trap, targets, 0);
	}
	
	
	public static void doCaltropDetonate(obj_id trap, Vector targets, float percentMod) throws InterruptedException
	{
		
		float trapPower = getIntObjVar(trap, TRAP_POWER);
		float duration = CALTROP_MAX_DURATION * (trapPower / 1000);
		float snareAmount = CALTROP_MAX_SNARE * (trapPower / 1000);
		obj_id curObj = null;
		prose_package pp = prose.getPackage(new string_id("spam", "caltrop_trap_hit"));
		obj_id owner = getOwner(trap);
		boolean isInvis = false;
		String invis = null;
		boolean attackerPresent = false;
		
		if (isIdValid(owner) && exists(owner))
		{
			attackerPresent = true;
			invis = getInvisBuff(getOwner(trap));
		}
		
		isInvis = invis != null && invis.length() > 0;
		
		playClientEffectLoc(trap, "clienteffect/lair_med_damage_smoke.cef", getLocation(trap), 0.0f);
		
		duration += duration * percentMod;
		snareAmount += snareAmount * percentMod;
		if (snareAmount > 95)
		{
			snareAmount = 95;
		}
		for (int i = 0; i < targets.size(); i++)
		{
			testAbortScript();
			curObj = (obj_id)targets.get(i);
			if (buff.canApplyBuff(curObj, "caltropSnare"))
			{
				playClientEffectLoc(curObj, "clienteffect/lair_damage_light.cef", getLocation(curObj), 0.0f);
				buff.applyBuff(curObj, "caltropSnare", duration, snareAmount);
				
				if (attackerPresent)
				{
					if (isInvis && rand(1, 100) < 50)
					{
						continue;
					}
					
					startCombat(curObj, owner);
				}
			}
		}
	}
	
	
	public static void doFlashBangDetonate(obj_id trap, Vector targets) throws InterruptedException
	{
		doFlashBangDetonate(trap, targets, 0);
	}
	
	
	public static void doFlashBangDetonate(obj_id trap, Vector targets, float percentMod) throws InterruptedException
	{
		
		float trapPower = getIntObjVar(trap, TRAP_POWER);
		float duration = FLASHBANG_MAX_DURATION * (trapPower / 1000);
		float amount = FLASHBANG_MAX_COMBAT_SLOW * (trapPower / 1000);
		obj_id curObj = null;
		prose_package pp = prose.getPackage(new string_id("spam", "flashbang_trap_hit"));
		obj_id owner = getOwner(trap);
		boolean isInvis = false;
		String invis = null;
		boolean attackerPresent = false;
		
		if (isIdValid(owner) && exists(owner))
		{
			attackerPresent = true;
			invis = getInvisBuff(getOwner(trap));
		}
		
		isInvis = invis != null && invis.length() > 0;
		
		playClientEffectLoc(trap, "clienteffect/int_camshake_light.cef", getLocation(trap), 0.0f);
		playClientEffectLoc(trap, "appearance/pt_flash_bomb.prt", getLocation(trap), 0.0f);
		
		duration += duration * percentMod;
		amount += amount * percentMod;
		
		for (int i = 0; i < targets.size(); i++)
		{
			testAbortScript();
			curObj = (obj_id)targets.get(i);
			if (buff.canApplyBuff(curObj, "flashBang"))
			{
				if (isPlayer(curObj))
				{
					doAnimationAction(curObj, "cover_ears_mocking");
				}
				buff.applyBuff(curObj, "flashBang", duration, amount);
				
				if (attackerPresent)
				{
					if (isInvis && rand(1, 100) < 50)
					{
						continue;
					}
					startCombat(curObj, owner);
				}
			}
			else
			{
				
			}
		}
		
		return;
	}
	
	
	public static boolean safePvPCanAttack(obj_id trap, obj_id target) throws InterruptedException
	{
		obj_id trapOwner = getOwner(trap);
		if (isIdValid(trapOwner) && exists(trapOwner))
		{
			return pvpCanAttack(trapOwner, target);
		}
		
		if (isInvulnerable(target))
		{
			return false;
		}
		
		if (trapOwner == target)
		{
			return false;
		}
		
		boolean actorIsPlayer = true;
		int actorFaction = getIntObjVar(trap, TRAP_FACTION);
		int actorPvPType = getIntObjVar(trap, TRAP_PVP_TYPE);
		
		int targetFaction = pvpGetAlignedFaction(target);
		boolean targetIsPlayer = isPlayer(target);
		int targetPvPType = pvpGetType(target);
		
		if (targetIsPlayer && actorPvPType != PVPTYPE_DECLARED)
		{
			return false;
		}
		
		if (targetIsPlayer && actorIsPlayer)
		{
			
			if (pvpAreFactionsOpposed(actorFaction, targetFaction))
			{
				
				if (actorPvPType == PVPTYPE_DECLARED && targetPvPType == PVPTYPE_DECLARED)
				{
					return true;
				}
			}
		}
		
		if (actorIsPlayer && !targetIsPlayer)
		{
			
			if (targetFaction != 0)
			{
				
				if (actorFaction != targetFaction)
				{
					
					return pvpAreFactionsOpposed(actorFaction, targetFaction);
				}
				
				return false;
			}
			else
			{
				return true;
			}
			
		}
		
		return false;
	}
	
	
	public static boolean validateTrapTarget(obj_id trap, obj_id target) throws InterruptedException
	{
		if (!isIdValid(target) || trap == target)
		{
			
			return false;
		}
		
		if (isDead(target))
		{
			
			return false;
		}
		
		if (isIncapacitated(target))
		{
			
			return false;
		}
		
		if (!safePvPCanAttack(trap, target))
		{
			
			return false;
		}
		
		location trapLoc = getLocation( trap );
		trapLoc.y += 0.5f;
		if (!canSee(target, trapLoc))
		{
			
			return false;
		}
		
		return true;
	}
	
	
	public static void playTrapFizzle(obj_id trap) throws InterruptedException
	{
		playClientEffectLoc(trap, "clienteffect/frs_light_serenity.cef", getLocation(trap), 0.0f);
	}
	
	
	public static void playSelfDestruct(obj_id trap) throws InterruptedException
	{
		playClientEffectLoc(trap, "clienteffect/combat_grenade_thermal_detonator.cef", getLocation(trap), 0.0f);
	}
	
	
	public static boolean setTrap(obj_id player, obj_id invTrap) throws InterruptedException
	{
		if (!hasObjVar(invTrap, TRIGGER_TYPE))
		{
			return false;
		}
		
		int count = getCount(invTrap);
		if (count < 1)
		{
			destroyObject(invTrap);
			return false;
		}
		
		location loc = getLocation(player);
		int triggerType = getIntObjVar(invTrap, TRIGGER_TYPE);
		String template = getTemplateName(invTrap);
		obj_id trap = utils.cloneObject(invTrap, loc);
		int time = getGameTime();
		
		if (!isIdValid(trap))
		{
			return false;
		}
		
		switch(triggerType)
		{
			case TRIGGER_TYPE_TIMER:
			dictionary parms = new dictionary();
			parms.put("armed", time);
			messageTo(trap, "msgTryDetonateTimer", parms, getIntObjVar(trap, TIMER_TIME), false);
			setObjVar(trap, "armed", time);
			break;
			case TRIGGER_TYPE_REMOTE:
			obj_id pInv = utils.getInventoryContainer(player);
			int free = getVolumeFree(pInv);
			if (free < 1)
			{
				sendSystemMessage(player, new string_id("spam", "no_room_remote_trigger"));
				return false;
			}
			String remote = "object/tangible/scout/trap/trap_ap_tr_remote.iff";
			obj_id remoteTrigger = createObject(remote, pInv, "");
			
			if (!isIdValid(remoteTrigger))
			{
				destroyObject(trap);
				return false;
			}
			
			String frequency = ""+ rand(0, 999) + "."+ rand(0, 999) + "."+ rand(0, 999);;
			setObjVar(remoteTrigger, stealth.TRAP_FREQUENCY, frequency);
			setObjVar(trap, stealth.TRAP_FREQUENCY, frequency);
			
			setObjVar(trap, stealth.TRAP_MY_PROXIMITY_TRIGGER, remoteTrigger);
			setObjVar(remoteTrigger, stealth.TRAP_MY_TRAP, trap);
			
			sendSystemMessage(player, new string_id("spam", "trap_keyed_remote"));
			setObjVar(trap, "armed", time);
			setObjVar(remoteTrigger, "armed", true);
			break;
			case TRIGGER_TYPE_PROXIMITY:
			dictionary timeArmed = new dictionary();
			timeArmed.put("armed", time);
			messageTo(trap, "msgArmProximity", timeArmed, 0, false);
			break;
		}
		
		playClientEffectObj(trap, "appearance/pt_trap_reticule.prt", trap, "", null, "trapReticle");
		doAnimationAction(player, "point_down");
		setOwner(trap, player);
		setObjVar(trap, TRAP_FACTION, pvpGetAlignedFaction(player));
		setObjVar(trap, TRAP_PVP_TYPE, pvpGetType(player));
		setObjVar(trap, TRAP_DIFFICULTY, getEnhancedSkillStatisticModifier(player, "ranger_trap"));
		setObjVar(trap, TRAP_LEVEL, getLevel(player));
		
		if (hasObjVar(trap, "concealable"))
		{
			if (buff.hasBuff(player, "conceal_trap"))
			{
				stealth.concealDevice(player, trap);
			}
		}
		
		setWeaponMinDamage(trap, 1);
		setWeaponMaxDamage(trap, 1);
		setWeaponAttackSpeed(trap, 0.25f);
		
		weapons.setWeaponData(trap);
		
		messageTo(trap, "msgSelfDestructTimeout", null, TRAP_SELF_DESTRUCT_TIMEOUT, false);
		count--;
		if (count < 1)
		{
			destroyObject(invTrap);
		}
		else
		{
			setCount(invTrap, count);
		}
		return true;
	}
	
	
	public static obj_id getTrigger(obj_id trap) throws InterruptedException
	{
		if (hasObjVar(trap, TRAP_MY_PROXIMITY_TRIGGER))
		{
			return getObjIdObjVar(trap, TRAP_MY_PROXIMITY_TRIGGER);
		}
		
		obj_id[] stuff = getContents(trap);
		for (int i = 0; i < stuff.length; i++)
		{
			testAbortScript();
			if (hasScript(stuff[i], "systems.skills.stealth.traptrigger"))
			{
				return stuff[i];
			}
		}
		
		return null;
	}
	
	
	public static boolean doTheftLoot(obj_id thief, obj_id mark) throws InterruptedException
	{
		obj_id thiefInventory = utils.getInventoryContainer(thief);
		int free = getVolumeFree(thiefInventory);
		prose_package pp = new prose_package();
		if (free < 1)
		{
			sendSystemMessage(thief, new string_id("spam", "cant_steal_inventory_full"));
			return false;
		}
		
		int stealFlags = stealFlags = STEAL_CREDITS | STEAL_LOOT;
		
		if (utils.hasScriptVar(mark, STEAL_TYPE))
		{
			stealFlags = utils.getIntScriptVar(mark, STEAL_TYPE);
		}
		
		if (isPlayer(mark))
		{
			stealFlags = STEAL_MARKED_ITEMS;
		}
		else if (hasStealingLootTableEntry(mark))
		{
			stealFlags |= STEAL_STEALING_TABLE;
		}
		
		Vector types = new Vector();
		if (isFlagSet(stealFlags, STEAL_CREDITS))
		{
			types.add(new Integer(STEAL_CREDITS));
		}
		
		if (isFlagSet(stealFlags, STEAL_LOOT))
		{
			types.add(new Integer(STEAL_LOOT));
		}
		
		if (isFlagSet(stealFlags, STEAL_TEMPLATE))
		{
			types.add(new Integer(STEAL_TEMPLATE));
		}
		
		if (isFlagSet(stealFlags, STEAL_MARKED_ITEMS))
		{
			types.add(new Integer(STEAL_MARKED_ITEMS));
		}
		
		if (isFlagSet(stealFlags, STEAL_STEALING_TABLE))
		{
			types.add(new Integer(STEAL_STEALING_TABLE));
		}
		
		boolean gotSomething = false;
		while (types.size() > 0 && !gotSomething)
		{
			testAbortScript();
			int roll = rand(0, types.size()-1);
			int flag = ((Integer)types.get(roll)).intValue();
			int markLevel = getLevel(mark);
			int thiefLevel = getLevel(thief);
			int stealRoll = rand(1,100);
			if (STEAL_FAIL_CHANCE < stealRoll)
			{
				
				switch(flag)
				{
					case STEAL_CREDITS:
					if (isPlayer(mark))
					{
						break;
					}
					int amt = (int)(getNpcCash(mark) * .2f);
					if (amt > 0)
					{
						dictionary d = new dictionary();
						d.put(money.DICT_AMOUNT, amt);
						boolean boolReturn = transferBankCreditsFromNamedAccount(money.ACCT_NPC_LOOT, thief, amt, "handleStealCashSuccess", "handleStealCashFail", d);
						utils.moneyInMetric(mark, money.ACCT_NPC_LOOT, amt);
						gotSomething = true;
						
					}
					break;
					
					case STEAL_LOOT:
					if (isPlayer(mark))
					{
						break;
					}
					
					int lootRoll = rand(1, 100);
					
					if (lootRoll >= STEAL_RARE_ITEM)
					{
						
						if (thiefLevel <= (markLevel + 10))
						{
							if (!hasCompletedCollection(thief, "spy_unarmed_01"))
							{
								long slotValueOne = getCollectionSlotValue(thief, "col_spy_unarmed_01_01");
								long slotValueTwo = getCollectionSlotValue(thief, "col_spy_unarmed_01_02");
								long slotValueThree = getCollectionSlotValue(thief, "col_spy_unarmed_01_03");
								long slotValueFour = getCollectionSlotValue(thief, "col_spy_unarmed_01_04");
								
								if (slotValueOne == 0)
								{
									modifyCollectionSlotValue(thief, "col_spy_unarmed_01_01", 1);
								}
								
								else if (slotValueTwo == 0)
								{
									modifyCollectionSlotValue(thief, "col_spy_unarmed_01_02", 1);
								}
								
								else if (slotValueThree == 0)
								{
									modifyCollectionSlotValue(thief, "col_spy_unarmed_01_03", 1);
								}
								
								else if (slotValueFour == 0)
								{
									modifyCollectionSlotValue(thief, "col_spy_unarmed_01_04", 1);
								}
							}
							
							gotSomething = loot.generateTheftLootRare(thiefInventory, mark, 1, thief);
							break;
						}
						
						gotSomething = loot.generateTheftLoot(thiefInventory, mark, STEAL_LOOT_CHANCE_MOD, 1);
						break;
						
					}
					
					if (lootRoll >= STEAL_REGULAR_ITEM && lootRoll < STEAL_RARE_ITEM)
					{
						if (thiefLevel <= (markLevel + 10))
						{
							
							boolean gotCollectionItem = false;
							gotCollectionItem = loot.addCollectionLoot(mark, true, thief);
							gotSomething = gotCollectionItem;
							
							if (!gotCollectionItem)
							{
								gotSomething = loot.generateTheftLoot(thiefInventory, mark, STEAL_LOOT_CHANCE_MOD, 1);
								break;
							}
							break;
						}
						
						else
						{
							gotSomething = loot.generateTheftLoot(thiefInventory, mark, STEAL_LOOT_CHANCE_MOD, 1);
							break;
							
						}
						
					}
					
					if (lootRoll < STEAL_REGULAR_ITEM)
					{
						gotSomething = loot.generateTheftLoot(thiefInventory, mark, STEAL_LOOT_CHANCE_MOD, 1);
						break;
					}
					
					break;
					
					case STEAL_TEMPLATE:
					Vector itms = getStealableTemplates(mark);
					if (itms.size() < 1)
					{
						break;
					}
					String templateLoot = (String)itms.get(rand(0, itms.size()-1));
					if (templateLoot == null || templateLoot.length() < 1)
					{
						break;
					}
					
					obj_id templateObj = null;
					if (templateLoot.startsWith("static:"))
					{
						java.util.StringTokenizer st = new java.util.StringTokenizer(templateLoot, ":");
						if (st.countTokens() == 2)
						{
							st.nextToken();
							String itemName = st.nextToken();
							templateObj = static_item.createNewItemFunction(templateLoot, thiefInventory);
						}
					}
					else
					{
						templateObj = createObjectOverloaded(templateLoot, thiefInventory);
					}
					
					gotSomething = templateObj != null;
					if (gotSomething)
					{
						utils.setScriptVar(thiefInventory, "theft_in_progress", 1);
						loot.notifyThiefOfItemStolen(thiefInventory, templateObj);
					}
					break;
					
					case STEAL_MARKED_ITEMS:
					obj_id[] stuff = utils.getFilteredPlayerContents(mark);
					Vector eligibleItems = new Vector();
					for (int i = 0; i < stuff.length; i++)
					{
						testAbortScript();
						if (isItemStealable(stuff[i]))
						{
							eligibleItems.add(stuff[i]);
						}
					}
					
					if (eligibleItems.size() < 1)
					{
						break;
					}
					
					obj_id markedObj = (obj_id)eligibleItems.get(rand(0, eligibleItems.size()-1));
					gotSomething = putIn(markedObj, thiefInventory);
					if (gotSomething)
					{
						CustomerServiceLog("stealing", "%TU stole an item ("+ markedObj + " - "+ getName(markedObj) +
						") from player %TT", thief, mark);
						utils.setScriptVar(thiefInventory, "theft_in_progress", 1);
						loot.notifyThiefOfItemStolen(thiefInventory, markedObj);
					}
					break;
					
					case STEAL_STEALING_TABLE:
					String tableTemplate = getStealableTemplateFromTable(mark);
					if (tableTemplate == null || tableTemplate.length() < 1)
					{
						break;
					}
					
					obj_id tableObj = null;
					if (tableTemplate.startsWith("static:"))
					{
						java.util.StringTokenizer st = new java.util.StringTokenizer(tableTemplate, ":");
						if (st.countTokens() == 2)
						{
							st.nextToken();
							String itemName = st.nextToken();
							tableObj = static_item.createNewItemFunction(tableTemplate, thiefInventory);
						}
					}
					else
					{
						tableObj = createObjectOverloaded(tableTemplate, thiefInventory);
					}
					gotSomething = tableObj != null;
					if (gotSomething)
					{
						utils.setScriptVar(thiefInventory, "theft_in_progress", 1);
						loot.notifyThiefOfItemStolen(thiefInventory, tableObj);
					}
					break;
				}
			}
			stealFlags &= ~flag;
			types.removeElementAt(roll);
		}
		
		if (!isPlayer(mark))
		{
			utils.setScriptVar(mark, STEAL_TYPE, stealFlags);
		}
		return gotSomething;
	}
	
	
	public static boolean hasStealingLootTableEntry(obj_id mark) throws InterruptedException
	{
		String name = getCreatureName(mark);
		String type = loot.getCreatureType(name);
		String table = "datatables/loot/stealing_loot.iff";
		int row = dataTableSearchColumnForInt(getStringCrc(type), "npc_type", table);
		
		if (row < 0)
		{
			return false;
		}
		
		return true;
	}
	
	
	public static String getStealableTemplateFromTable(obj_id mark) throws InterruptedException
	{
		Vector stuff = new Vector();
		Vector weights = new Vector();
		
		String name = getCreatureName(mark);
		
		String table = "datatables/loot/stealing_loot.iff";
		int row = dataTableSearchColumnForInt(getStringCrc(name), "npc_type", table);
		
		if (row < 0)
		{
			return null;
		}
		
		int rows = dataTableGetNumRows(table);
		while (true)
		{
			testAbortScript();
			stuff.add(dataTableGetString(table, row, "stealableObjects"));
			weights.add(new Integer(dataTableGetInt(table, row, "weight")));
			
			row++;
			
			if (row >= rows || dataTableGetInt(table, row, "npc_type") != 0)
			{
				break;
			}
		}
		
		int total = 0;
		int[] weightArray = new int[weights.size()];
		for (int i = 0; i < weightArray.length; i++)
		{
			testAbortScript();
			weightArray[i] = ((Integer)weights.get(i)).intValue();
			total += weightArray[i];
		}
		
		if (total < 1)
		{
			total = 1;
		}
		
		int roll = rand(1, total);
		
		int idx = -1;
		int low_range = 0;
		for (int j = 0; j < weightArray.length; j++)
		{
			testAbortScript();
			int high_range = low_range + weightArray[j];
			
			if (roll > low_range && roll <= high_range)
			{
				idx = j;
				break;
			}
			low_range = high_range;
		}
		
		if (idx < stuff.size())
		{
			return (String)stuff.get(idx);
		}
		return (String)stuff.get(rand(0, stuff.size()-1));
	}
	
	
	public static void flagItemAsStealable(obj_id item) throws InterruptedException
	{
		setObjVar(item, STEAL_ITEM_IS_STEALABLE, 1);
	}
	
	
	public static boolean isItemStealable(obj_id item) throws InterruptedException
	{
		return hasObjVar(item, STEAL_ITEM_IS_STEALABLE);
	}
	
	
	public static void unflagItemAsStealable(obj_id item) throws InterruptedException
	{
		removeObjVar(item, STEAL_ITEM_IS_STEALABLE);
	}
	
	
	public static void addStealableTemplate(obj_id mark, String template) throws InterruptedException
	{
		Vector itms = getStealableTemplates(mark);
		itms.add(template);
		utils.setResizeableBatchObjVar(mark, STEAL_TEMPLATE_ITEMS, itms);
	}
	
	
	public static Vector getStealableTemplates(obj_id mark) throws InterruptedException
	{
		return utils.getResizeableStringBatchObjVar(mark, STEAL_TEMPLATE_ITEMS);
	}
	
	
	public static int getNpcCash(obj_id mark) throws InterruptedException
	{
		String mobType = ai_lib.getCreatureName(mark);
		if (mobType == null)
		{
			return 0;
		}
		
		int level = ai_lib.getLevel(mark);
		int cash = loot.getCashForLevel(mobType, level);
		
		if (cash > 0)
		{
			return cash;
		}
		
		cash = rand (1, 10);
		cash = cash * level;
		
		return cash;
	}
	
	
	public static boolean canSteal(obj_id thief, obj_id mark) throws InterruptedException
	{
		if (!pvpCanAttack(thief, mark))
		{
			sendSystemMessage(thief, new string_id("spam", "steal_cant_pvp"));
			return false;
		}
		
		if (!ai_lib.isHumanoid(mark))
		{
			sendSystemMessage(thief, new string_id("spam", "steal_not_humanoid"));
			return false;
		}
		
		if (utils.hasScriptVar(mark, STEAL_TYPE))
		{
			int stealFlags = utils.getIntScriptVar(mark, STEAL_TYPE);
			if (stealFlags == STEAL_NOTHING)
			{
				sendSystemMessage(thief, new string_id("spam", "stolen_got_nothing"));
				return false;
			}
		}
		
		return true;
	}
	
	
	public static boolean steal(obj_id thief, obj_id mark) throws InterruptedException
	{
		if (!luck.isLucky(thief, 0.01f))
		{
			if (isCoughtWhileStealing(thief, mark) || luck.isLucky(mark, 0.01f))
			{
				sendSystemMessageTestingOnly(thief, "STEALING CHECK FAILED: CAUGHT");
				showFlyText(thief, new string_id("spam", "stealing_cought"), 1.5f, colors.TOMATO);
				sendSystemMessage(thief, new string_id("spam", "stolen_cought"));
				String invis = getInvisBuff(thief);
				if (invis != null && invis.length() > 0)
				{
					buff.removeBuff(thief, invis);
				}
				if (isPlayer(mark))
				{
					prose_package pp = prose.getPackage(new string_id("spam", "almost_got_item_stolen"), thief);
					sendSystemMessageProse(mark, pp);
				}
				else
				{
					startCombat(mark, thief);
					startCombat(thief, mark);
				}
				return false;
			}
		}
		
		showFlyTextPrivate(thief, thief, new string_id("spam", "stealin_on"), 1.5f, colors.TOMATO);
		if (doTheftLoot(thief, mark))
		{
			
		}
		else
		{
			sendSystemMessage(thief, new string_id("spam", "stolen_nothing"));
		}
		
		return true;
	}
	
	
	public static boolean isCoughtWhileStealing(obj_id thief, obj_id mark) throws InterruptedException
	{
		return passiveDetectHiddenTarget(thief, mark, PASSIVE_BREACH_NEAR);
	}
	
	
	public static boolean witnessModifiedSkillCheck(obj_id player, String skillMod, float distance, int witnessPenalty, int skillModMod) throws InterruptedException
	{
		int skill = getEnhancedSkillStatisticModifier(player, skillMod) + skillModMod;
		if (skill < 0)
		{
			skill = 0;
		}
		
		obj_id[] stuff = getCreaturesInRange(player, distance);
		
		int cnt = 0;
		int sGuildId = getGuildId(player);
		
		if (stuff.length > 0)
		{
			for (int i = 0; i < stuff.length; i++)
			{
				testAbortScript();
				if (ai_lib.isHumanoid(stuff[i]))
				{
					boolean isInSameGuild = (sGuildId != 0) && (getGuildId(stuff[i]) == sGuildId);
					if (!isInSameGuild && !group.inSameGroup(player, stuff[i]))
					{
						skill -= witnessPenalty;
					}
				}
			}
			
			float rslt = (1/ (1+ ((skill+10) / 10)));
			return (rand(0, 1f) > rslt);
		}
		
		return true;
	}
	
	
	public static void invisBuffRemoved(obj_id player, String sbuff) throws InterruptedException
	{
		
		if (buff.hasBuff(player, BUFF_NO_BREAK_INVIS))
		{
			return;
		}
		
		if (buff.hasBuff(player, "sp_burst_shadows"))
		{
			buff.removeBuff(player, "sp_burst_shadows");
		}
		
		_makeVisible(player, new string_id("spam", "stealthoff"), null);
		
		messageTo(player, "addAppearanceItemEffect", null, 1, false);
		
	}
	
	
	public static void OnLocomotionChange(obj_id player, int oldLoco, int newLoco) throws InterruptedException
	{
		String invis = getInvisBuff(player);
		if (invis == null)
		{
			return;
		}
		
		int pri = buff.getPriority(invis);
		if (pri < MIN_MOVEMENT_PRIORITY)
		{
			sendSystemMessage(player, new string_id("spam", "posturechangereveal"));
			buff.removeBuff(player, invis);
		}
	}
	
	
	public static void OnPostureChanged(obj_id player, int before, int after) throws InterruptedException
	{
		String invis = getInvisBuff(player);
		if (invis == null)
		{
			return;
		}
		
		int pri = buff.getPriority(invis);
		if (pri < MIN_POSTURE_CHANGE_PRIORITY)
		{
			sendSystemMessage(player, new string_id("spam", "posturechangereveal"));
			buff.removeBuff(player, invis);
		}
	}
	
	
	public static int getApplicableInvisSkillMod(obj_id thing, String buffName) throws InterruptedException
	{
		String modName = "camouflage";
		switch(getStringCrc(buffName))
		{
			
			case (-879194497):
			modName = "camouflage";
			break;
			case (1252380095):
			modName = "camouflage";
			break;
			case (815407714):
			case (-1923109470):
			break;
			case (-121504680):
			modName = "cover";
			break;
			case (1206342977):
			case (-50098035):
			modName = "stealth";
			break;
			case (342850470):
			modName = "";
			break;
		}
		
		int mod = 0;
		if (modName.length() > 0)
		{
			mod = getEnhancedSkillStatisticModifierUncapped(thing, modName);
			
			if (modName.equals("camouflage"))
			{
				mod += getEnhancedSkillStatisticModifierUncapped(thing, "stealth");
			}
		}
		
		return mod;
	}
	
	
	public static int getApplicableInvisSkillMod(obj_id thing) throws InterruptedException
	{
		int mod = 0;
		if (isPlayer(thing))
		{
			String invis = getInvisBuff(thing);
			if (invis == null || invis.length() < 1)
			{
				return mod;
			}
			
			mod = getApplicableInvisSkillMod(thing, invis);
		}
		else
		{
			mod = getIntObjVar(thing, "camouflageMod");
		}
		
		return mod;
	}
	
	
	public static float getDetectChance(obj_id target, obj_id detector, float baseChanceToDetect) throws InterruptedException
	{
		String invis = getInvisBuff(target);
		
		if (beast_lib.isBeast(target))
		{
			invis = "invis";
		}
		
		if (invis != null && invis.length() > 0)
		{
			if (buff.getPriority(invis) > MAX_INVIS_BUFF_TO_DETECT)
			{
				return 0.0f;
			}
			
			if (invis.startsWith("invis_sp_buff_invis_notrace"))
			{
				return 0.0f;
			}
		}
		
		float finalChanceToDetect = 0.0f;
		
		int stealtherLevel = getLevel(target);
		int detectorLevel = getLevel(detector);
		
		float levelChanceToDetectMod = 0.0f;
		
		if (stealtherLevel > detectorLevel)
		{
			float levelDiff = stealtherLevel - detectorLevel;
			if (baseChanceToDetect > 30)
			{
				levelChanceToDetectMod = (levelDiff *5.0f);
			}
			else
			{
				levelChanceToDetectMod = (levelDiff *1.5f);
			}
			finalChanceToDetect = baseChanceToDetect - levelChanceToDetectMod;
		}
		
		if (stealtherLevel < detectorLevel)
		{
			float levelDiff = detectorLevel - stealtherLevel;
			levelChanceToDetectMod = (levelDiff*2.5f);
			finalChanceToDetect = baseChanceToDetect + levelChanceToDetectMod;
		}
		
		if (stealtherLevel == detectorLevel)
		{
			finalChanceToDetect = baseChanceToDetect;
		}
		
		float modChanceToDetectMod = 0.0f;
		
		int stealtherSkillMod = getApplicableInvisSkillMod(target);
		
		if (beast_lib.isBeast(target))
		{
			obj_id master = getMaster(target);
			
			if (isIdValid(master) && exists(master))
			{
				stealtherSkillMod = getApplicableInvisSkillMod(master);
			}
		}
		
		int detectorSkillMod = getEnhancedSkillStatisticModifierUncapped(detector, "detect_hidden");
		
		if (stealtherSkillMod > detectorSkillMod)
		{
			float modDiff = (stealtherSkillMod - detectorSkillMod);
			modChanceToDetectMod = modDiff/9.0f;
			finalChanceToDetect = finalChanceToDetect - modChanceToDetectMod;
		}
		
		if (stealtherSkillMod < detectorSkillMod)
		{
			float modDiff = (detectorSkillMod - stealtherSkillMod);
			modChanceToDetectMod = modDiff/4.0f;
			finalChanceToDetect = finalChanceToDetect + modChanceToDetectMod;
		}
		
		if (invis.startsWith("invis_forceCloak") || (invis.startsWith("invis_fs_buff_invis") ))
		{
			
			finalChanceToDetect = (finalChanceToDetect - 10.0f);
		}
		
		if (!isPlayer(detector))
		{
			String name = getCreatureName(detector);
			int difficultyClass = dataTableGetInt ("datatables/mob/creatures.iff", name, "difficultyClass");
			if (difficultyClass == 1)
			{
				
				finalChanceToDetect = (finalChanceToDetect +5.0f);
			}
			if (difficultyClass == 2)
			{
				
				finalChanceToDetect = (finalChanceToDetect +10.0f);
			}
		}
		
		if (finalChanceToDetect > MAX_CHANCE_TO_DETECT_HIDDEN)
		{
			finalChanceToDetect = MAX_CHANCE_TO_DETECT_HIDDEN;
		}
		
		if (finalChanceToDetect < MIN_CHANCE_TO_DETECT_HIDDEN)
		{
			if (isPlayer(detector))
			{
				finalChanceToDetect = MIN_CHANCE_TO_DETECT_HIDDEN;
			}
		}
		
		return finalChanceToDetect;
	}
	
	
	public static float getDetectChanceWithDetailedOutput(obj_id target, obj_id detector, float baseChanceToDetect, obj_id objectToReceiveDetailedOutput) throws InterruptedException
	{
		String invis = getInvisBuff(target);
		if (invis != null && invis.length() > 0)
		{
			sendConsoleMessage(objectToReceiveDetailedOutput, "passiveDetectInfo: stealther="+ target + ", detector="+ detector + ", baseChanceToDetect="+ baseChanceToDetect + ", stealther invisBuff="+ invis);
			
			if (buff.getPriority(invis) > MAX_INVIS_BUFF_TO_DETECT)
			{
				sendConsoleMessage(objectToReceiveDetailedOutput, "passiveDetectInfo: buff.getPriority(stealther invisBuff)="+ buff.getPriority(invis) + " which is > "+ MAX_INVIS_BUFF_TO_DETECT + " so setting finalChanceToDetect to 0.0");
				return 0.0f;
			}
			
			if (invis.startsWith("invis_sp_buff_invis_notrace"))
			{
				sendConsoleMessage(objectToReceiveDetailedOutput, "passiveDetectInfo: stealther invisBuff is invis_sp_buff_invis_notrace* so setting finalChanceToDetect to 0.0");
				return 0.0f;
			}
		}
		float finalChanceToDetect = 0.0f;
		
		int stealtherLevel = getLevel(target);
		int detectorLevel = getLevel(detector);
		
		sendConsoleMessage(objectToReceiveDetailedOutput, "passiveDetectInfo: stealtherLevel="+ stealtherLevel + ", detectorLevel="+ detectorLevel);
		
		float levelChanceToDetectMod = 0.0f;
		
		if (stealtherLevel > detectorLevel)
		{
			float levelDiff = stealtherLevel - detectorLevel;
			if (baseChanceToDetect > 30)
			{
				levelChanceToDetectMod = (levelDiff *5.0f);
				sendConsoleMessage(objectToReceiveDetailedOutput, "passiveDetectInfo: stealtherLevel > detectorLevel, levelDiff="+ levelDiff + ", baseChanceToDetect > 30, levelChanceToDetectMod (levelDiff * 5.0f)="+ levelChanceToDetectMod);
			}
			else
			{
				levelChanceToDetectMod = (levelDiff *1.5f);
				sendConsoleMessage(objectToReceiveDetailedOutput, "passiveDetectInfo: stealtherLevel > detectorLevel, levelDiff="+ levelDiff + ", baseChanceToDetect <= 30, levelChanceToDetectMod (levelDiff * 1.5f)="+ levelChanceToDetectMod);
			}
			finalChanceToDetect = baseChanceToDetect - levelChanceToDetectMod;
			sendConsoleMessage(objectToReceiveDetailedOutput, "passiveDetectInfo: stealtherLevel > detectorLevel, finalChanceToDetect (baseChanceToDetect - levelChanceToDetectMod)="+ finalChanceToDetect);
		}
		
		if (stealtherLevel < detectorLevel)
		{
			float levelDiff = detectorLevel - stealtherLevel;
			levelChanceToDetectMod = (levelDiff*2.5f);
			finalChanceToDetect = baseChanceToDetect + levelChanceToDetectMod;
			
			sendConsoleMessage(objectToReceiveDetailedOutput, "passiveDetectInfo: stealtherLevel < detectorLevel, levelDiff="+ levelDiff + ", levelChanceToDetectMod (levelDiff * 2.5f)="+ levelChanceToDetectMod + ", finalChanceToDetect (baseChanceToDetect + levelChanceToDetectMod)="+ finalChanceToDetect);
		}
		
		if (stealtherLevel == detectorLevel)
		{
			finalChanceToDetect = baseChanceToDetect;
			sendConsoleMessage(objectToReceiveDetailedOutput, "passiveDetectInfo: stealtherLevel = detectorLevel, finalChanceToDetect (baseChanceToDetect)="+ finalChanceToDetect);
		}
		
		float modChanceToDetectMod = 0.0f;
		
		int stealtherSkillMod = getApplicableInvisSkillMod(target);
		
		int detectorSkillMod = getEnhancedSkillStatisticModifierUncapped(detector, "detect_hidden");
		
		sendConsoleMessage(objectToReceiveDetailedOutput, "passiveDetectInfo: stealtherSkillMod="+ stealtherSkillMod + ", detectorSkillMod (detect_hidden)="+ detectorSkillMod);
		
		if (stealtherSkillMod > detectorSkillMod)
		{
			float modDiff = (stealtherSkillMod - detectorSkillMod);
			modChanceToDetectMod = modDiff/9.0f;
			finalChanceToDetect = finalChanceToDetect - modChanceToDetectMod;
			
			sendConsoleMessage(objectToReceiveDetailedOutput, "passiveDetectInfo: stealtherSkillMod > detectorSkillMod, modDiff="+ modDiff + ", modChanceToDetectMod (modDiff / 9.0f)="+ modChanceToDetectMod + ", finalChanceToDetect (finalChanceToDetect - modChanceToDetectMod)="+ finalChanceToDetect);
		}
		
		if (stealtherSkillMod < detectorSkillMod)
		{
			float modDiff = (detectorSkillMod - stealtherSkillMod);
			modChanceToDetectMod = modDiff/4.0f;
			finalChanceToDetect = finalChanceToDetect + modChanceToDetectMod;
			
			sendConsoleMessage(objectToReceiveDetailedOutput, "passiveDetectInfo: stealtherSkillMod < detectorSkillMod, modDiff="+ modDiff + ", modChanceToDetectMod (modDiff / 4.0f)="+ modChanceToDetectMod + ", finalChanceToDetect (finalChanceToDetect + modChanceToDetectMod)="+ finalChanceToDetect);
		}
		
		if (invis.startsWith("invis_forceCloak") || (invis.startsWith("invis_fs_buff_invis") ))
		{
			
			finalChanceToDetect = (finalChanceToDetect - 10.0f);
			
			sendConsoleMessage(objectToReceiveDetailedOutput, "passiveDetectInfo: Jedi with invis_forceCloak* or invis_fs_buff_invis*, finalChanceToDetect (finalChanceToDetect - 10.0f)="+ finalChanceToDetect);
		}
		
		if (!isPlayer(detector))
		{
			String name = getCreatureName(detector);
			int difficultyClass = dataTableGetInt ("datatables/mob/creatures.iff", name, "difficultyClass");
			if (difficultyClass == 1)
			{
				
				finalChanceToDetect = (finalChanceToDetect +5.0f);
				
				sendConsoleMessage(objectToReceiveDetailedOutput, "passiveDetectInfo: detector ("+ name + ", difficultyClass="+ difficultyClass + ") is elite NPC, finalChanceToDetect (finalChanceToDetect + 5.0f)="+ finalChanceToDetect);
			}
			if (difficultyClass == 2)
			{
				
				finalChanceToDetect = (finalChanceToDetect +10.0f);
				
				sendConsoleMessage(objectToReceiveDetailedOutput, "passiveDetectInfo: detector ("+ name + ", difficultyClass="+ difficultyClass + ") is boss NPC, finalChanceToDetect (finalChanceToDetect + 10.0f)="+ finalChanceToDetect);
			}
		}
		
		if (finalChanceToDetect > MAX_CHANCE_TO_DETECT_HIDDEN)
		{
			finalChanceToDetect = MAX_CHANCE_TO_DETECT_HIDDEN;
			
			sendConsoleMessage(objectToReceiveDetailedOutput, "passiveDetectInfo: finalChanceToDetect > "+ MAX_CHANCE_TO_DETECT_HIDDEN + " so finalChanceToDetect set to "+ finalChanceToDetect);
		}
		
		if (finalChanceToDetect < MIN_CHANCE_TO_DETECT_HIDDEN)
		{
			if (isPlayer(detector))
			{
				finalChanceToDetect = MIN_CHANCE_TO_DETECT_HIDDEN;
				
				sendConsoleMessage(objectToReceiveDetailedOutput, "passiveDetectInfo: finalChanceToDetect < "+ MIN_CHANCE_TO_DETECT_HIDDEN + " and detector is player so finalChanceToDetect set to "+ finalChanceToDetect);
			}
		}
		
		sendConsoleMessage(objectToReceiveDetailedOutput, "passiveDetectInfo: ****FINAL CALCULATED*** finalChanceToDetect="+ finalChanceToDetect);
		return finalChanceToDetect;
	}
	
	
	public static boolean activeDetectHiddenTarget(obj_id target, obj_id detector, float baseChanceToDetect) throws InterruptedException
	{
		float finalChanceToDetect = getDetectChance(target, detector, baseChanceToDetect);
		float roll = rand(0f, 100f);
		return (roll < finalChanceToDetect);
	}
	
	
	public static boolean passiveDetectHiddenTarget(obj_id target, obj_id detector, int volume) throws InterruptedException
	{
		
		if (isPlayer(detector))
		{
			if (!pvpCanAttack(detector, target))
			{
				return false;
			}
		}
		else
		{
			if (!ai_lib.isAggroToward(detector, target))
			{
				return false;
			}
		}
		float baseChanceToDetect = PASSIVE_DETECT_CHANCE_MOB;
		
		if (volume == PASSIVE_BREACH_FAR)
		{
			baseChanceToDetect = PASSIVE_DETECT_CHANCE_FAR;
		}
		if (volume == PASSIVE_BREACH_NEAR)
		{
			baseChanceToDetect = PASSIVE_DETECT_CHANCE_NEAR;
		}
		
		float finalChanceToDetect = getDetectChance(target, detector, baseChanceToDetect);
		
		float roll = rand(0.0f, 100.0f);
		
		return (roll < finalChanceToDetect);
	}
	
	
	public static boolean passiveDetectHiddenTargetWithDetailedOutput(obj_id target, obj_id detector, int volume, obj_id objectToReceiveDetailedOutput) throws InterruptedException
	{
		sendConsoleMessage(objectToReceiveDetailedOutput, "passiveDetectInfo: stealther="+ target + ", detector="+ detector + ", volume="+ volume);
		
		if (isPlayer(detector))
		{
			if (!pvpCanAttack(detector, target))
			{
				sendConsoleMessage(objectToReceiveDetailedOutput, "passiveDetectInfo: stealther="+ target + ", detector="+ detector + ", volume="+ volume + ", isPlayer(detector) and !pvpCanAttack(detector, stealther) so returning detection FAIL");
				return false;
			}
		}
		else
		{
			if (!ai_lib.isAggroToward(detector, target))
			{
				sendConsoleMessage(objectToReceiveDetailedOutput, "passiveDetectInfo: stealther="+ target + ", detector="+ detector + ", volume="+ volume + ", !isPlayer(detector) and !ai_lib.isAggroToward(detector, stealther) so returning detection FAIL");
				return false;
			}
		}
		float baseChanceToDetect = PASSIVE_DETECT_CHANCE_MOB;
		sendConsoleMessage(objectToReceiveDetailedOutput, "passiveDetectInfo: setting baseChanceToDetect to default value of PASSIVE_DETECT_CHANCE_MOB("+ baseChanceToDetect + ")");
		
		if (volume == PASSIVE_BREACH_FAR)
		{
			baseChanceToDetect = PASSIVE_DETECT_CHANCE_FAR;
			sendConsoleMessage(objectToReceiveDetailedOutput, "passiveDetectInfo: volume is PASSIVE_BREACH_FAR so setting baseChanceToDetect to PASSIVE_DETECT_CHANCE_FAR("+ baseChanceToDetect + ")");
		}
		if (volume == PASSIVE_BREACH_NEAR)
		{
			baseChanceToDetect = PASSIVE_DETECT_CHANCE_NEAR;
			sendConsoleMessage(objectToReceiveDetailedOutput, "passiveDetectInfo: volume is PASSIVE_BREACH_NEAR so setting baseChanceToDetect to PASSIVE_DETECT_CHANCE_NEAR("+ baseChanceToDetect + ")");
		}
		
		float finalChanceToDetect = getDetectChanceWithDetailedOutput(target, detector, baseChanceToDetect, objectToReceiveDetailedOutput);
		float roll = rand(0.0f, 100.0f);
		
		if (roll < finalChanceToDetect)
		{
			sendConsoleMessage(objectToReceiveDetailedOutput, "passiveDetectInfo: getDetectChanceWithDetailedOutput() returned finalChanceToDetect="+ finalChanceToDetect + ", dice roll="+ roll + ", dice roll < finalChanceToDetect so returning detection SUCCESS");
		}
		else
		{
			sendConsoleMessage(objectToReceiveDetailedOutput, "passiveDetectInfo: getDetectChanceWithDetailedOutput() returned finalChanceToDetect="+ finalChanceToDetect + ", dice roll="+ roll + ", dice roll >= finalChanceToDetect so returning detection FAIL");
		}
		
		return (roll < finalChanceToDetect);
	}
	
	
	public static boolean testBubbleBreached(obj_id bubbleHolder, obj_id breacher, int volume) throws InterruptedException
	{
		if (hasObjVar(bubbleHolder, "debuggingPassiveDetectDetailedOutput"))
		{
			obj_id objectToReceiveDetailedOutput = bubbleHolder;
			
			if (hasObjVar(bubbleHolder, "debuggingPassiveDetectDetailedOutputObject"))
			{
				String objVarValue = getStringObjVar(bubbleHolder, "debuggingPassiveDetectDetailedOutputObject");
				if (objVarValue != null && objVarValue.length() > 0)
				{
					obj_id objVarValueAsOid = utils.stringToObjId(objVarValue.substring(1));
					if (isValidId(objVarValueAsOid) && exists(objVarValueAsOid))
					{
						objectToReceiveDetailedOutput = objVarValueAsOid;
					}
				}
			}
			
			return passiveDetectHiddenTargetWithDetailedOutput(bubbleHolder, breacher, volume, objectToReceiveDetailedOutput);
		}
		else
		{
			return passiveDetectHiddenTarget(bubbleHolder, breacher, volume);
		}
		
	}
	
	
	public static void testUseAction(obj_id player, obj_id object, int menuItem) throws InterruptedException
	{
		String invis = getInvisBuff(player);
		if (invis != null && invis.length() > 0)
		{
			buff.removeBuff(player, invis);
		}
	}
	
	
	public static void testInvisNonCombatAction(obj_id player, obj_id target) throws InterruptedException
	{
		testInvisCombatAction(player, target, -1);
	}
	
	
	public static void testInvisNonCombatAction(obj_id player, obj_id target, combat_data combatData) throws InterruptedException
	{
		
		if (!combatData.specialLine.equals("no_proc"))
		{
			testInvisCombatAction(player, target, combatData.minInvisLevelRequired);
		}
	}
	
	
	public static void testInvisNonCombatAction(obj_id player, obj_id target, int minInvisRequiredForAction) throws InterruptedException
	{
		if (isIdValid(player) && exists(player) && beast_lib.isBeast(player))
		{
			player = getMaster(player);
		}
		
		String invis = getInvisBuff(player);
		
		if (invis != null && !invis.startsWith("invis_"))
		{
			int pri = buff.getPriority(invis);
			if (minInvisRequiredForAction == -1 || pri < minInvisRequiredForAction)
			{
				if (!buff.hasBuff(player, BUFF_NO_BREAK_INVIS))
				{
					buff.removeBuff(player, invis);
				}
			}
		}
	}
	
	
	public static void testInvisCombatAction(obj_id player, obj_id target) throws InterruptedException
	{
		testInvisCombatAction(player, target, -1);
	}
	
	
	public static void testInvisCombatAction(obj_id player, obj_id target, combat_data combatData) throws InterruptedException
	{
		
		if (!combatData.specialLine.equals("no_proc"))
		{
			testInvisCombatAction(player, target, combatData.minInvisLevelRequired);
		}
	}
	
	
	public static void testInvisCombatAction(obj_id player, obj_id target, int minInvisRequiredForAction) throws InterruptedException
	{
		if (isIdValid(player) && exists(player) && beast_lib.isBeast(player))
		{
			player = getMaster(player);
		}
		
		String invis = getInvisBuff(player);
		
		if (invis != null)
		{
			int pri = buff.getPriority(invis);
			
			if (minInvisRequiredForAction == -1 || pri < minInvisRequiredForAction)
			{
				if (!buff.hasBuff(player, BUFF_NO_BREAK_INVIS))
				{
					combat.combatLog(player, target, "combatStandardAction", "Invisible - Clearing invisibility");
					
					if (isPlayer(player))
					{
						utils.setScriptVar(player, "stealth.invisRemoved", invis);
						utils.setScriptVar(player, "stealth.invisRemovedTime", getGameTime());
						utils.setScriptVar(player, "stealth.invisRemovedTimeLeft", buff.getBuffTimeRemaining(player, invis));
					}
					
					buff.removeBuff(player, invis);
					buff.applyBuff(player, "sp_attack_invis_recourse");
				}
			}
		}
	}
	
	
	public static void reinstateInvisFromCombat(obj_id player) throws InterruptedException
	{
		if (!isIdValid(player) || !exists(player) || !isPlayer(player))
		{
			return;
		}
		
		int invisTime = utils.getIntScriptVar(player, "stealth.invisRemovedTime");
		String invisName = utils.getStringScriptVar(player, "stealth.invisRemoved");
		
		if (invisTime > 0 && invisName != null && invisName.length() > 0 && invisTime - getGameTime() < 2)
		{
			float timeLeft = utils.getFloatScriptVar(player, "stealth.invisRemovedTimeLeft");
			
			if (timeLeft > 0)
			{
				buff.applyBuff(player, invisName, timeLeft);
			}
		}
	}
	
	
	public static void clearPreviousInvis(obj_id player) throws InterruptedException
	{
		if (!isPlayer(player))
		{
			return;
		}
		
		utils.removeScriptVar(player, "stealth.invisRemoved");
		utils.removeScriptVar(player, "stealth.invisRemovedTime");
	}
	
	
	public static float calcBreachDistanceForSkill(obj_id player, String skillMod) throws InterruptedException
	{
		int mod = getEnhancedSkillStatisticModifier(player, skillMod);
		float difference = INVIS_BREAK_MAX_FAR_DISTANCE - INVIS_BREAK_NEAR_DISTANCE;
		return INVIS_BREAK_NEAR_DISTANCE + (difference / (1+ ((mod) / 100)));
	}
	
	
	public static void cleanupHate(obj_id player) throws InterruptedException
	{
		if (!isIdValid(player) || !exists(player))
		{
			return;
		}
		
		obj_id[] hateList = getHateList(player);
		
		for (int i = 0; i < hateList.length; i++)
		{
			testAbortScript();
			if (isIdValid(hateList[i]) && exists(hateList[i]))
			{
				removeHateTarget(hateList[i], player);
				
				removeHateTarget(player, hateList[i]);
			}
		}
		
		clearHateList(player);
	}
	
	
	public static obj_id makeBeastInvisible(obj_id player, String clientEffect) throws InterruptedException
	{
		if (!isIdValid(player) || !exists(player) || !beast_lib.isBeastMaster(player))
		{
			return null;
		}
		
		obj_id beast = beast_lib.getBeastOnPlayer(player);
		
		if (!isIdValid(beast) || !exists(beast))
		{
			return null;
		}
		
		attachScript(beast, "systems.skills.stealth.stealth_monitor");
		messageTo(beast, "updatePassiveRevealList", null, 10, false);
		setState(beast, STATE_COVER, true);
		setCreatureCoverVisibility(beast, false);
		
		cleanupHate(beast);
		stopCombat(beast);
		utils.removeScriptVar(beast, "ai.combat.target");
		
		addPassiveReveal(beast, player, 1);
		makePassiveRevealed(beast);
		
		if (clientEffect == null || clientEffect.length() <= 0)
		{
			clientEffect = "appearance/pt_smoke_puff.prt";
		}
		
		playClientEffectObj(beast, clientEffect, beast, "");
		
		int covert_mastery = getEnhancedSkillStatisticModifierUncapped(player, "expertise_covert_mastery");
		
		if (covert_mastery > 0)
		{
			buff.applyBuff(beast, "sp_covert_mastery");
		}
		
		return beast;
	}
	
	
	public static boolean canBeastStealth(obj_id beast) throws InterruptedException
	{
		obj_id master = getMaster(beast);
		
		if (isDead(beast) || ai_lib.isInCombat(beast))
		{
			return false;
		}
		
		if (isIdValid(master) && exists(master) && getState(master, STATE_COVER) == 1)
		{
			return true;
		}
		
		return false;
	}
	
	
	public static void makeCreatureVisible(obj_id creature) throws InterruptedException
	{
		if (!isIdValid(creature) || !exists(creature))
		{
			return;
		}
		
		detachScript(creature, "systems.skills.stealth.stealth_monitor");
		
		setState(creature, STATE_COVER, false);
		
		clearPassiveRevealList(creature);
		setCreatureCoverVisibility(creature, true);
		playClientEffectObj(creature, "appearance/pt_sonic_pulse.prt", creature, "");
	}
	
	
	public static void makeBeastVisible(obj_id player) throws InterruptedException
	{
		if (!isIdValid(player) || !exists(player) || !beast_lib.isBeastMaster(player))
		{
			return;
		}
		
		obj_id beast = beast_lib.getBeastOnPlayer(player);
		
		makeCreatureVisible(beast);
	}
	
	
	public static void makePassiveRevealed(obj_id target) throws InterruptedException
	{
		obj_id beast = null;
		
		if (isIdValid(target) && exists(target) && beast_lib.isBeast(target))
		{
			obj_id master = getMaster(target);
			
			if (isIdValid(master) && exists(master))
			{
				target = master;
			}
		}
		
		if (isPlayer(target))
		{
			beast = beast_lib.getBeastOnPlayer(target);
		}
		
		if (group.isGrouped(target))
		{
			obj_id groupObj = getGroupObject(target);
			obj_id[] groupMembers = getGroupMemberIds(groupObj);
			int numGroupMembers = groupMembers.length;
			
			for (int f = 0; f < numGroupMembers; f++)
			{
				testAbortScript();
				obj_id groupie = groupMembers[f];
				
				if (isIdValid(groupie) && exists(groupie) && groupie != target)
				{
					
					addPassiveReveal(target, groupie, 1);
					
					if (isIdValid(beast) && exists(beast))
					{
						addPassiveReveal(beast, groupie, 1);
					}
				}
			}
		}
	}
	
	
	public static void _makeInvisible(obj_id player, string_id flyText, String clientEffect, string_id msg, float breachDistance) throws InterruptedException
	{
		if (breachDistance <= INVIS_BREAK_NEAR_DISTANCE)
		{
			breachDistance = INVIS_BREAK_NEAR_DISTANCE + 1f;
		}
		
		attachScript(player, "systems.skills.stealth.stealth_monitor");
		messageTo(player, "updatePassiveRevealList", null, 10, false);
		setState(player, STATE_COVER, true);
		setCreatureCoverVisibility(player, false);
		
		cleanupHate(player);
		
		obj_id beast = makeBeastInvisible(player, clientEffect);
		
		makePassiveRevealed(player);
		
		if (flyText != null)
		{
			showFlyTextPrivate(player, player, flyText, 1.5f, colors.TOMATO);
		}
		
		if (clientEffect != null && clientEffect.length() > 0)
		{
			playClientEffectObj(player, clientEffect, player, "");
		}
		
		if (msg != null)
		{
			sendSystemMessage(player, msg);
		}
		
		if (!utils.hasScriptVar(player, INVIS_UPKEEP_MSG_DISPATCHED))
		{
			messageTo(player, "invisibilityUpkeep", null, 1, false);
		}
		
		if (breachDistance > 0)
		{
			createTriggerVolume(INVIS_BREAK_RADIUS_FAR, breachDistance, true );
			createTriggerVolume(INVIS_BREAK_RADIUS_NEAR, INVIS_BREAK_NEAR_DISTANCE, true );
		}
		
	}
	
	
	public static void _makeVisible(obj_id player, string_id flyText, string_id msg) throws InterruptedException
	{
		
		removeTriggerVolume(INVIS_BREAK_RADIUS_FAR);
		removeTriggerVolume(INVIS_BREAK_RADIUS_NEAR);
		clearPassiveRevealList(player);
		detachScript(player, "systems.skills.stealth.stealth_monitor");
		
		if (!utils.hasScriptVar(player, "supressVisible"))
		{
			setState(player, STATE_COVER, false);
			
			setCreatureCoverVisibility(player, true);
			
			makeBeastVisible(player);
			
			if (flyText != null)
			{
				showFlyTextPrivate(player, player, flyText, 1.5f, colors.TOMATO);
			}
			
			if (msg != null)
			{
				sendSystemMessage(player, msg);
			}
			
			playClientEffectObj(player, "appearance/pt_sonic_pulse.prt", player, "");
		}
		else
		{
			utils.removeScriptVar(player, "supressVisible");
		}
	}
	
	
	public static boolean checkRevealRecourse(obj_id player) throws InterruptedException
	{
		if (buff.hasBuff(player, "sp_sneak_recourse"))
		{
			return true;
		}
		return false;
	}
	
	
	public static boolean checkAttackRevealRecourse(obj_id player) throws InterruptedException
	{
		if (buff.hasBuff(player, "sp_attack_invis_recourse"))
		{
			return true;
		}
		return false;
	}
	
	
	public static boolean canPerformForceCloak(obj_id player) throws InterruptedException
	{
		if (combat.isInCombat(player) && (int)getSkillStatisticModifier(player, "expertise_force_cloak_combat_escape") <= 0)
		{
			return false;
		}
		
		if (isDead(player) || isIncapacitated(player))
		{
			sendSystemMessage(player, new string_id("spam", "cant_do_it_state"));
			return false;
		}
		
		if (buff.hasBuff(player, "battlefield_communication_run"))
		{
			return false;
		}
		
		if (checkRevealRecourse(player))
		{
			sendSystemMessage(player, new string_id("spam", "cant_do_it_recourse"));
			return false;
		}
		
		if (utils.hasScriptVar(player, "petCreationPending") && utils.getIntScriptVar(player, "petCreationPending") > getGameTime())
		{
			sendSystemMessage(player, new string_id("spam", "cannot_perform_calling_companion"));
			return false;
		}
		
		return true;
	}
	
	
	public static boolean canPerformHide(obj_id player) throws InterruptedException
	{
		return canPerformStationaryInvis(player);
	}
	
	
	public static void forceCloak(obj_id player) throws InterruptedException
	{
		if (buff.hasBuff(player, "fs_force_run"))
		{
			buff.removeBuff(player, "fs_force_run");
		}
		
		_makeInvisible(player, new string_id("combat_effects", "go_cloak"),
		"clienteffect/combat_special_attacker_cover.cef", null, -1);
	}
	
	
	public static void hide(obj_id player) throws InterruptedException
	{
		_makeInvisible(player, new string_id("spam", "hide_go"),
		"clienteffect/combat_special_attacker_cover.cef", null, -1);
	}
	
	
	public static boolean canPerformSmokeGrenade(obj_id player) throws InterruptedException
	{
		if (isDead(player) || isIncapacitated(player))
		{
			sendSystemMessage(player, new string_id("spam", "cant_do_it_state"));
			return false;
		}
		
		if (!buff.canApplyBuff(player, "invis_sp_buff_invis_1"))
		{
			sendSystemMessage(player, new string_id("spam", "cant_apply_invis_buff"));
			return false;
		}
		
		if (buff.hasBuff(player, "battlefield_communication_run"))
		{
			return false;
		}
		
		if (utils.hasScriptVar(player, "petCreationPending") && utils.getIntScriptVar(player, "petCreationPending") > getGameTime())
		{
			sendSystemMessage(player, new string_id("spam", "cannot_perform_calling_companion"));
			return false;
		}
		if (checkAttackRevealRecourse(player))
		{
			sendSystemMessage(player, new string_id("spam", "cant_do_it_attack_recourse"));
			return false;
		}
		
		return true;
	}
	
	
	public static boolean canPerformWithoutTrace(obj_id player) throws InterruptedException
	{
		if (isDead(player) || isIncapacitated(player))
		{
			sendSystemMessage(player, new string_id("spam", "cant_do_it_state"));
			return false;
		}
		
		if (!buff.canApplyBuff(player, "invis_sp_buff_invis_notrace_1"))
		{
			sendSystemMessage(player, new string_id("spam", "cant_apply_invis_buff"));
			return false;
		}
		
		if (buff.hasBuff(player, "battlefield_communication_run"))
		{
			return false;
		}
		
		if (utils.hasScriptVar(player, "petCreationPending") && utils.getIntScriptVar(player, "petCreationPending") > getGameTime())
		{
			sendSystemMessage(player, new string_id("spam", "cannot_perform_calling_companion"));
			return false;
		}
		
		return true;
	}
	
	
	public static boolean canPerformStationaryInvis(obj_id player) throws InterruptedException
	{
		
		if (isDead(player) || isIncapacitated(player))
		{
			sendSystemMessage(player, new string_id("spam", "cant_do_it_state"));
			return false;
		}
		
		if (!buff.canApplyBuff(player, "invis_blendIn"))
		{
			sendSystemMessage(player, new string_id("spam", "cant_apply_invis_buff"));
			return false;
		}
		
		if (buff.hasBuff(player, "battlefield_communication_run"))
		{
			return false;
		}
		
		if (utils.hasScriptVar(player, "petCreationPending") && utils.getIntScriptVar(player, "petCreationPending") > getGameTime())
		{
			sendSystemMessage(player, new string_id("spam", "cannot_perform_calling_companion"));
			return false;
		}
		
		return true;
	}
	
	
	public static void smokeGrenade(obj_id player) throws InterruptedException
	{
		_makeInvisible(player, new string_id("spam", "smokegrenade"), "appearance/pt_smoke_puff.prt", null, -1);
		buff.applyBuff(player, BUFF_NO_BREAK_INVIS);
	}
	
	
	public static void bothanInnate(obj_id player) throws InterruptedException
	{
		_makeInvisible(player, new string_id("spam", "fly_bothan_ability_1"),
		"appearance/pt_smoke_puff.prt", null, -1);
	}
	
	
	public static boolean canPerformStealth(obj_id player) throws InterruptedException
	{
		if (isDead(player) || isIncapacitated(player))
		{
			sendSystemMessage(player, new string_id("spam", "cant_do_it_state"));
			return false;
		}
		
		if (ai_lib.isInCombat(player))
		{
			sendSystemMessage(player, new string_id("spam", "cant_do_it_combat"));
			return false;
		}
		
		String invis = getInvisBuff(player);
		
		if (invis != null)
		{
			if (invis.startsWith("invis_sp_buff_invis_notrace"))
			{
				sendSystemMessage(player, new string_id("spam", "cant_apply_invis_buff"));
				return false;
			}
		}
		
		if (buff.hasBuff(player, "battlefield_communication_run"))
		{
			return false;
		}
		
		if (!hasServerCoverState(player))
		{
			if (checkRevealRecourse(player))
			{
				sendSystemMessage(player, new string_id("spam", "cant_do_it_recourse"));
				return false;
			}
			
		}
		
		if (utils.hasScriptVar(player, "petCreationPending") && utils.getIntScriptVar(player, "petCreationPending") > getGameTime())
		{
			sendSystemMessage(player, new string_id("spam", "cannot_perform_calling_companion"));
			return false;
		}
		
		return true;
	}
	
	
	public static void stealth(obj_id player) throws InterruptedException
	{
		_makeInvisible(player, new string_id("spam", "stealth"),"appearance/pt_smoke_puff.prt", null, calcBreachDistanceForSkill(player, "camouflage"));
		buff.applyBuff(player, BUFF_NO_BREAK_INVIS);
	}
	
	
	public static void withoutTrace(obj_id player) throws InterruptedException
	{
		_makeInvisible(player, new string_id("spam", "without_trace"),"appearance/pt_smoke_puff.prt", null, -1);
		int covert_mastery = getEnhancedSkillStatisticModifierUncapped(player, "expertise_covert_mastery");
		
		if (covert_mastery > 0)
		{
			buff.applyBuff(player, "sp_covert_mastery");
		}
		
		buff.applyBuff(player, BUFF_NO_BREAK_INVIS);
	}
	
	
	public static void decoyStealth(obj_id player) throws InterruptedException
	{
		_makeInvisible(player, new string_id("spam", "decoy_stealth"),"", null, -1);
		buff.applyBuff(player, BUFF_NO_BREAK_INVIS);
		
	}
	
	
	public static void invisBuffAdded(obj_id mobile, String effectName) throws InterruptedException
	{
		effectName = toLower(effectName);
		
		messageTo(mobile, "addAppearanceItemEffect", null, 1, false);
		
		if (effectName.startsWith("invis_sp_buff_invis_notrace"))
		{
			withoutTrace(mobile);
		}
		else if (effectName.startsWith("invis_sp_buff_invis"))
		{
			smokeGrenade(mobile);
		}
		else if (effectName.startsWith("invis_bothan_ability_1"))
		{
			bothanInnate(mobile);
		}
		else if (effectName.startsWith("invis_sp_buff_stealth"))
		{
			stealth(mobile);
		}
		else if (effectName.startsWith("invis_fs_buff_invis"))
		{
			forceCloak(mobile);
		}
		else if (effectName.startsWith("invis_sm_buff_invis"))
		{
			stealth(mobile);
		}
		else if (effectName.startsWith("invis_sp_diversion_stealth"))
		{
			decoyStealth(mobile);
		}
		return;
	}
	
	
	public static boolean canPerformCamouflageSelf(obj_id player) throws InterruptedException
	{
		if (isDead(player) || isIncapacitated(player))
		{
			sendSystemMessage(player, new string_id("spam", "cant_do_it_state"));
			return false;
		}
		
		if (checkRevealRecourse(player))
		{
			sendSystemMessage(player, new string_id("spam", "cant_do_it_recourse"));
			return false;
		}
		
		if (!buff.canApplyBuff(player, "invis_camouflage"))
		{
			sendSystemMessage(player, new string_id("spam", "cant_apply_invis_buff"));
			return false;
		}
		
		if (buff.hasBuff(player, "battlefield_communication_run"))
		{
			return false;
		}
		
		if (isInUrban(player))
		{
			sendSystemMessage(player, new string_id("spam", "cant_camo_self_urban"));
			return false;
		}
		
		obj_id kit = getCamoKitForCurrentPlanet(player);
		if (!isIdValid(kit))
		{
			sendSystemMessage(player, new string_id("spam", "cant_camo_self_no_kit"));
			return false;
		}
		
		utils.setScriptVar(player, CAMO_KIT_ID, kit);
		return true;
	}
	
	
	public static void camouflageSelf(obj_id player) throws InterruptedException
	{
		obj_id kit = utils.getObjIdScriptVar(player, CAMO_KIT_ID);
		if (!isIdValid(kit))
		{
			return;
		}
		
		consumeCamoKit(kit);
		
		boolean success = witnessModifiedSkillCheck(player, "camouflage", 15, 5, 0);
		if (success)
		{
			_makeInvisible(player, new string_id("spam", "camouflaged"),
			"appearance/pt_smoke_puff.prt", null, calcBreachDistanceForSkill(player, "camouflage"));
		}
		else
		{
			sendSystemMessage(player, new string_id("spam", "failedcamo"));
		}
		
		return;
	}
	
	
	public static boolean canPerformCamouflageAlly(obj_id player, obj_id target) throws InterruptedException
	{
		if (!isIdValid(target))
		{
			sendSystemMessage(player, new string_id("spam", "no_target"));
			return false;
		}
		
		if (!isPlayer(target))
		{
			sendSystemMessage(player, new string_id("spam", "bad_target"));
			return false;
		}
		
		int shapechange = buff.getBuffOnTargetFromGroup(target, "shapechange");
		
		if (shapechange != 0)
		{
			sendSystemMessage(player, new string_id("spam", "not_while_shapechanged"));
			return false;
		}
		
		if (target == player)
		{
			sendSystemMessage(player, new string_id("spam", "cant_apply_invis_self"));
			return false;
		}
		
		if (buff.hasBuff(target, "battlefield_communication_run"))
		{
			return false;
		}
		
		if (isDead(player) || isIncapacitated(player))
		{
			sendSystemMessage(player, new string_id("spam", "cant_do_it_state"));
			return false;
		}
		
		if (getDistance(player, target) > 46)
		{
			sendSystemMessage(player, new string_id("spam", "cant_camo_ally_far"));
			return false;
		}
		
		if (!pvpCanHelp(player, target))
		{
			sendSystemMessage(player, new string_id("spam", "cant_camo_ally_faction"));
			return false;
		}
		
		if (checkRevealRecourse(target))
		{
			sendSystemMessage(player, new string_id("spam", "cant_do_it_recourse_other"));
			return false;
		}
		
		int tGuildId = getGuildId(target);
		int sGuildId = getGuildId(player);
		boolean isInSameGuild = (tGuildId != 0) && (tGuildId == sGuildId);
		
		if (!isInSameGuild && !group.inSameGroup(player, target))
		{
			sendSystemMessage(player, new string_id("spam", "cant_camo_ally_not_allied"));
			return false;
		}
		
		return true;
	}
	
	
	public static boolean canPerformUrbanStealth(obj_id player, obj_id possibleHep) throws InterruptedException
	{
		if (buff.hasBuff(player, "forceRun") || buff.hasBuff(player, "forceRun_1") || buff.hasBuff(player, "forceRun_2"))
		{
			sendSystemMessage(player, new string_id("spam", "cant_fc_run"));
			return false;
		}
		
		if (buff.hasBuff(player, "battlefield_communication_run"))
		{
			return false;
		}
		
		if (isDead(player) || isIncapacitated(player))
		{
			sendSystemMessage(player, new string_id("spam", "cant_do_it_state"));
			return false;
		}
		
		if (checkRevealRecourse(player))
		{
			sendSystemMessage(player, new string_id("spam", "cant_do_it_recourse"));
			return false;
		}
		
		if (!buff.canApplyBuff(player, "invis_urbanStealth"))
		{
			sendSystemMessage(player, new string_id("spam", "cant_apply_invis_buff"));
			return false;
		}
		
		if (!stealth.isInUrban(player))
		{
			sendSystemMessage(player, new string_id("spam", "cant_urban_stealth_environment"));
			return false;
		}
		
		obj_id hep = stealth.getHEPObj(player, possibleHep);
		if (!isIdValid(hep))
		{
			sendSystemMessage(player, new string_id("spam", "cant_urban_stealth_no_hep"));
			return false;
		}
		
		if (!buff.hasBuff(player, "invis_blendIn"))
		{
			int sGuildId = getGuildId(player);
			boolean isInSameGuild = false;
			obj_id[] stuff = getCreaturesInRange(player, 20f);
			
			for (int i = 0; i < stuff.length; i++)
			{
				testAbortScript();
				if (isPlayer(stuff[i]) || ai_lib.isNpc(stuff[i]))
				{
					isInSameGuild = (sGuildId != 0) && (getGuildId(stuff[i]) == sGuildId);
					if (!isInSameGuild && !group.inSameGroup(player, stuff[i]))
					{
						sendSystemMessage(player, new string_id("spam", "cant_urban_stealth_people_in_area"));
						return false;
					}
				}
			}
		}
		
		utils.setScriptVar(player, CAMO_KIT_ID, hep);
		return true;
	}
	
	
	public static void urbanStealth(obj_id player) throws InterruptedException
	{
		obj_id hep = utils.getObjIdScriptVar(player, CAMO_KIT_ID);
		if (!isIdValid(hep))
		{
			return;
		}
		
		if (!testAndDoHepUpkeep(hep, player, false))
		{
			return;
		}
		
		utils.makeItemNoDrop(hep);
		setObjVar(player, ACTIVE_HEP, hep);
		if (hasInvisibleBuff(player))
		{
			
			utils.setScriptVar(player, "supressVisible", 1);
		}
		
		_makeInvisible(player, new string_id("spam", "stealthon"), "appearance/pt_detector.prt", null, calcBreachDistanceForSkill(player, "stealth"));
	}
	
	
	public static boolean canPerformWildernessStealth(obj_id player) throws InterruptedException
	{
		if (buff.hasBuff(player, "forceRun") || buff.hasBuff(player, "forceRun_1") || buff.hasBuff(player, "forceRun_2"))
		{
			sendSystemMessage(player, new string_id("spam", "cant_fc_run"));
			return false;
		}
		
		if (buff.hasBuff(player, "battlefield_communication_run"))
		{
			return false;
		}
		
		if (isDead(player) || isIncapacitated(player))
		{
			sendSystemMessage(player, new string_id("spam", "cant_do_it_state"));
			return false;
		}
		
		if (checkRevealRecourse(player))
		{
			sendSystemMessage(player, new string_id("spam", "cant_do_it_recourse"));
			return false;
		}
		
		if (!buff.canApplyBuff(player, "invis_wildernessStealth"))
		{
			sendSystemMessage(player, new string_id("spam", "cant_apply_invis_buff"));
			return false;
		}
		
		if (!stealth.isInWilderness(player))
		{
			sendSystemMessage(player, new string_id("spam", "cant_wilderness_stealth_environment"));
			return false;
		}
		
		boolean isHidden = hasInvisibleBuff(player);
		if (!isHidden)
		{
			obj_id kit = getCamoKitForCurrentPlanet(player);
			if (!isIdValid(kit))
			{
				sendSystemMessage(player, new string_id("spam", "cant_wilderness_stealth_no_kit"));
				return false;
			}
			
			utils.setScriptVar(player, CAMO_KIT_ID, kit);
		}
		
		return true;
	}
	
	
	public static void wildernessStealth(obj_id player) throws InterruptedException
	{
		obj_id kit = utils.getObjIdScriptVar(player, CAMO_KIT_ID);
		
		int penalty = 0;
		if (!hasInvisibleBuff(player))
		{
			if (!isIdValid(kit))
			{
				return;
			}
			
			consumeCamoKit(kit);
			penalty = 50;
		}
		else
		{
			
			utils.setScriptVar(player, "supressVisible", 1);
		}
		
		boolean success = witnessModifiedSkillCheck(player, "stealth", 20, 10, penalty);
		if (success)
		{
			_makeInvisible(player, new string_id("spam", "stealthon"),
			"appearance/pt_smoke_puff.prt", null, calcBreachDistanceForSkill(player, "stealth"));
		}
		else
		{
			utils.removeScriptVar(player, "supressVisible");
			sendSystemMessage(player, new string_id("spam", "stealthfail"));
		}
		
		return;
	}
	
	
	public static boolean canPerformCover(obj_id player) throws InterruptedException
	{
		if (isDead(player) || isIncapacitated(player))
		{
			sendSystemMessage(player, new string_id("spam", "cant_do_it_state"));
			return false;
		}
		
		if (buff.hasBuff(player, "battlefield_communication_run"))
		{
			return false;
		}
		
		if (checkRevealRecourse(player))
		{
			sendSystemMessage(player, new string_id("spam", "cant_do_it_recourse"));
			return false;
		}
		
		if (!buff.canApplyBuff(player, "invis_cover"))
		{
			sendSystemMessage(player, new string_id("spam", "cant_apply_invis_buff"));
			return false;
		}
		
		return true;
	}
	
	
	public static void cover(obj_id player) throws InterruptedException
	{
		_makeInvisible(player, new string_id("combat_effects", "go_cover"),
		"clienteffect/combat_special_attacker_cover.cef", null, calcBreachDistanceForSkill(player, "camouflage"));
		sendSystemMessage(player, new string_id("cbt_spam", "cover_success_single"));
	}
	
	
	public static void disableHep(obj_id hep, obj_id player) throws InterruptedException
	{
		if (isIdValid(hep) && exists(hep))
		{
			int power = getCount(hep);
			float multiplier = .5f;
			if (power > HEP_UPKEEP_COST)
			{
				multiplier = .95f;
			}
			
			power = (int)(getIntObjVar(hep, HEP_BASE_POWER) * multiplier);
			setCount(hep, power);
			setObjVar(hep, HEP_BASE_POWER, power);
			
			utils.clearNoDropFromItem(hep);
			
			if (power < HEP_UPKEEP_COST)
			{
				sendSystemMessage(player, new string_id("spam", "hep_low_power"));
				setCondition(hep, CONDITION_DISABLED);
			}
		}
		
		if (isIdValid(player))
		{
			removeObjVar(player, ACTIVE_HEP);
		}
	}
	
	
	public static boolean testAndDoHepUpkeep(obj_id hep, obj_id player, boolean silent) throws InterruptedException
	{
		int power = getCount(hep);
		if (power < HEP_UPKEEP_COST)
		{
			if (!silent)
			{
				sendSystemMessage(player, new string_id("spam", "hep_out_of_juice"));
			}
			
			return false;
		}
		
		power = getCount(hep) - HEP_UPKEEP_COST;
		setCount(hep, power);
		
		return true;
	}
	
	
	public static boolean checkSpyStealthUpkeep(obj_id player) throws InterruptedException
	{
		float camouflage = getEnhancedSkillStatisticModifierUncapped(player, "camouflage");
		camouflage += getEnhancedSkillStatisticModifierUncapped(player, "stealth");
		
		float drainRatio = 10.0f;
		if (camouflage < 100.0f)
		{
			drainRatio = 7.91f - (.1f / (.1f + ((camouflage) / 100.0f)));
		}
		int actionCost = (int)(getMaxAction(player) / drainRatio);
		int action = testDrainAttribute(player, ACTION, actionCost);
		if (action < 0 || !drainAttributes(player, actionCost, 0))
		{
			checkForAndMakeVisible(player);
			return false;
		}
		
		string_id strSpam = new string_id("combat_effects", "damage_fly");
		prose_package pp = new prose_package();
		pp = prose.setStringId(pp, strSpam);
		pp = prose.setDI(pp, -actionCost);
		showFlyText(player, pp, 1f, colors.GREEN);
		
		return true;
	}
	
	
	public static boolean checkUrbanStealthUpkeep(obj_id player) throws InterruptedException
	{
		if (!isInUrban(player))
		{
			sendSystemMessage(player, new string_id("spam", "cant_urban_stealth_environment"));
			buff.removeBuff(player, "invis_urbanStealth");
			return false;
		}
		
		obj_id hep = getObjIdObjVar(player, ACTIVE_HEP);
		if (!isIdValid(hep) || !exists(hep))
		{
			
			buff.removeBuff(player, "invis_urbanStealth");
			return false;
		}
		
		if (!testAndDoHepUpkeep(hep, player, false))
		{
			buff.removeBuff(player, "invis_urbanStealth");
			return false;
		}
		
		return true;
	}
	
	
	public static boolean checkWildernessStealthUpkeep(obj_id player) throws InterruptedException
	{
		if (!isInWilderness(player))
		{
			sendSystemMessage(player, new string_id("spam", "cant_wilderness_stealth_environment"));
			buff.removeBuff(player, "invis_wildernessStealth");
			return false;
		}
		
		return true;
	}
	
	
	public static boolean checkForceCloakUpkeep(obj_id player) throws InterruptedException
	{
		float forceCost = dataTableGetFloat(jedi.JEDI_ACTIONS_FILE, "forceCloak", "extraForceCost");
		
		obj_id[] players = getPlayerCreaturesInRange(player, stealth.INVIS_BREAK_MAX_FAR_DISTANCE);
		obj_id[] npcs = getNPCsInRange(player, stealth.INVIS_BREAK_MAX_FAR_DISTANCE);
		int nearCount = players.length + npcs.length;
		
		int i = 0;
		for (i = 0; i < players.length; i++)
		{
			testAbortScript();
			if (group.inSameGroup(player, players[i]))
			{
				nearCount--;
			}
		}
		
		for (i = 0; i < npcs.length; i++)
		{
			testAbortScript();
			if (group.inSameGroup(player, npcs[i]))
			{
				nearCount--;
			}
		}
		
		if (nearCount < 0)
		{
			nearCount = 0;
		}
		
		forceCost *= nearCount;
		
		if (!jedi.drainForcePower(player, (int)forceCost))
		{
			buff.removeBuff(player, "invi_forceCloak");
			return false;
		}
		
		return true;
	}
	
	
	public static boolean checkForAndMakeVisible(obj_id player) throws InterruptedException
	{
		String ibuff = getInvisBuff(player);
		if (ibuff == null || ibuff.length() < 1)
		{
			return false;
		}
		
		buff.applyBuff(player, "sp_sneak_recourse");
		buff.removeBuff(player, ibuff);
		return true;
	}
	
	
	public static boolean checkForAndMakeVisibleNoRecourse(obj_id player) throws InterruptedException
	{
		String ibuff = getInvisBuff(player);
		if (ibuff == null || ibuff.length() < 1)
		{
			return false;
		}
		
		buff.removeBuff(player, ibuff);
		return true;
	}
	
	
	public static boolean hasInvisibleBuff(obj_id object) throws InterruptedException
	{
		return getInvisBuff(object) != null || (isMob(object) && !getCreatureCoverVisibility(object));
	}
	
	
	public static String getInvisBuff(obj_id player) throws InterruptedException
	{
		int[] buffs = buff.getAllBuffs(player);
		if (buffs == null)
		{
			return null;
		}
		
		String curBuff = "";
		for (int i = 0; i < buffs.length; i++)
		{
			testAbortScript();
			curBuff = buff.getBuffNameFromCrc(buffs[i]);
			if (curBuff != null && curBuff.startsWith("invis_"))
			{
				return curBuff;
			}
		}
		
		return null;
	}
	
	
	public static boolean hasServerCoverState(obj_id player) throws InterruptedException
	{
		return getState(player, STATE_COVER) > 0;
	}
	
	
	public static obj_id getHEPObj(obj_id player, obj_id possibleHep) throws InterruptedException
	{
		int power = 0;
		if (hasObjVar(possibleHep, "isHEP" ))
		{
			power = getCount(possibleHep);
			if (power >= HEP_UPKEEP_COST)
			{
				
				return possibleHep;
			}
		}
		
		obj_id pInv = utils.getInventoryContainer(player);
		if (!isIdValid(pInv))
		{
			return null;
		}
		
		obj_id[] contents = getContents(pInv);
		obj_id hep = null;
		for (int i = 0; i < contents.length; i++)
		{
			testAbortScript();
			if (hasObjVar(contents[i], "isHEP" ))
			{
				power = getCount(contents[i]);
				if (power >= HEP_UPKEEP_COST)
				{
					
					hep = contents[i];
					break;
				}
			}
		}
		
		return hep;
	}
	
	
	public static obj_id getCamoKitForCurrentPlanet(obj_id player) throws InterruptedException
	{
		
		String planetName = getCurrentSceneName();
		if (planetName.length() > 7)
		{
			String sceneSubString = planetName.substring(0,8);
			
			if (sceneSubString.equals("kashyyyk"))
			{
				planetName = "kashyyyk";
			}
		}
		
		obj_id pInv = utils.getInventoryContainer(player);
		if (!isIdValid(pInv))
		{
			return null;
		}
		
		obj_id[] contents = getContents(pInv);
		obj_id kit = null;
		for (int i=0; i<contents.length; i++)
		{
			testAbortScript();
			if (hasObjVar( contents[i], "camokit" ))
			{
				
				String kitPlanet = getStringObjVar(contents[i], "camokit");
				if (kitPlanet.equals(planetName))
				{
					kit = contents[i];
					break;
				}
			}
		}
		
		return kit;
	}
	
	
	public static void consumeCamoKit(obj_id kit) throws InterruptedException
	{
		int count = getCount( kit );
		if (count <= 1)
		{
			destroyObject( kit );
		}
		else
		{
			count--;
			setCount( kit, count );
		}
	}
	
	
	public static boolean isInWilderness(obj_id player) throws InterruptedException
	{
		
		obj_id container = getTopMostContainer(player);
		if (isIdValid(container) && isGameObjectTypeOf(container, GOT_building))
		{
			return true;
		}
		
		location loc = getLocation(player);
		
		if (locations.isInCity(loc) || city.isInCity(loc))
		{
			return false;
		}
		
		obj_id[] stuff = getNonCreaturesInRange(loc, 50f);
		int numStructures = 0;
		for (int i = 0; i < stuff.length; i++)
		{
			testAbortScript();
			if (isGameObjectTypeOf(stuff[i], GOT_building ) || isGameObjectTypeOf(stuff[i], GOT_installation))
			{
				numStructures++;
			}
		}
		
		if (numStructures > MAX_BUILDINGS_FOR_WILDNERNESS)
		{
			return false;
		}
		
		return true;
	}
	
	
	public static boolean isInUrban(obj_id player) throws InterruptedException
	{
		return (!isInWilderness(player));
	}
	
	
	public static boolean isFlagSet(int mask, int flag) throws InterruptedException
	{
		if ((mask & flag) == flag)
		{
			return true;
		}
		
		return false;
	}
	
	
	public static boolean isPvPTrap(obj_id trap) throws InterruptedException
	{
		int triggerTypeInt = getIntObjVar(trap, TRIGGER_TYPE);
		if (triggerTypeInt == TRIGGER_TYPE_PROXIMITY)
		{
			return true;
		}
		
		return false;
	}
	
	
	public static obj_id createDecoy(obj_id spy) throws InterruptedException
	{
		location myLoc = getLocation(spy);
		int species = getSpecies(spy);
		int gender = getGender(spy);
		
		String template = "object/mobile/hologram/";
		
		String speciesString = "human";
		String genderString = "male";
		
		switch(species)
		{
			case SPECIES_RODIAN: speciesString = "rodian";
			break;
			case SPECIES_TRANDOSHAN: speciesString = "trandoshan";
			break;
			case SPECIES_MON_CALAMARI: speciesString = "moncal";
			break;
			case SPECIES_WOOKIEE: speciesString = "wookiee";
			break;
			case SPECIES_BOTHAN: speciesString = "bothan";
			break;
			case SPECIES_TWILEK: speciesString = "twilek";
			break;
			case SPECIES_ZABRAK: speciesString = "zabrak";
			break;
			case SPECIES_ITHORIAN: speciesString = "ithorian";
			break;
			case SPECIES_SULLUSTAN: speciesString = "sullustan";
			break;
		}
		
		if (gender == GENDER_FEMALE)
		{
			genderString = "female";
		}
		
		template += speciesString + "_"+ genderString + ".iff";
		
		obj_id hologram = createObject(template, myLoc);
		
		if (isIdValid(hologram))
		{
			setMaster(hologram, spy);
			setScale(hologram, getScale(spy));
			setYaw(hologram, getYaw(spy));
			setName(hologram, getName(spy));
			setDescriptionStringId(hologram, new string_id("mob/creature_names", "decoy_bio"));
			setInvulnerable(hologram, false);
			factions.setFaction(hologram, factions.getFaction(spy));
			setDecoyOrigin(hologram, spy);
			
			custom_var[] var_list = getAllCustomVars(spy);
			
			if (var_list != null && var_list.length > 0)
			{
				for (int i = 0; i < var_list.length; i++)
				{
					testAbortScript();
					ranged_int_custom_var ricv = (ranged_int_custom_var)var_list[i];
					
					String var = ricv.getVarName();
					int value = ricv.getValue();
					
					if (value != 0)
					{
						setRangedIntCustomVarValue(hologram, var, value);
					}
					
				}
			}
			
			obj_id[] contents = getAllWornItems(spy, false);
			obj_id headSlotItem = getObjectInSlot(spy, "hat");
			
			utils.setObjVar(hologram, "hologram_performer", 1);
			if (contents != null && contents.length > 0)
			{
				for (int i = 0; i < contents.length; i++)
				{
					testAbortScript();
					if (isIdValid(contents[i]))
					{
						if (getContainerType(contents[i]) == 0)
						{
							if (!getTemplateName(contents[i]).endsWith("player.iff"))
							{
								
								if (headSlotItem == contents[i] && isPlayerHelmetHidden(spy))
								{
									continue;
								}
								
								if (getGameObjectType(contents[i]) == GOT_misc_appearance_only_invisible)
								{
									continue;
								}
								
								custom_var[] object_var = getAllCustomVars(contents[i]);
								
								obj_id newObject = createObject(getTemplateName(contents[i]), hologram, "");
								
								if (object_var != null && isIdValid(newObject))
								{
									for (int q=0; q<object_var.length; q++)
									{
										testAbortScript();
										ranged_int_custom_var ricv = (ranged_int_custom_var)object_var[q];
										
										String var = ricv.getVarName();
										int value = ricv.getValue();
										
										if (value != 0)
										{
											setRangedIntCustomVarValue(newObject, var, value);
										}
									}
								}
							}
						}
					}
				}
			}
			
			obj_id backpack = getObjectInSlot(spy, "back");
			if (isIdValid(backpack) && exists(backpack) && !isPlayerBackpackHidden(spy))
			{
				custom_var[] object_var = getAllCustomVars(backpack);
				
				obj_id newObject = createObject(getTemplateName(backpack), hologram, "back");
				
				if (object_var != null && isIdValid(newObject))
				{
					for (int q=0; q<object_var.length; q++)
					{
						testAbortScript();
						ranged_int_custom_var ricv = (ranged_int_custom_var)object_var[q];
						
						String var = ricv.getVarName();
						int value = ricv.getValue();
						
						if (value != 0)
						{
							setRangedIntCustomVarValue(newObject, var, value);
						}
					}
				}
			}
			
			obj_id spyWeapon = getCurrentWeapon(spy);
			weapon_data weaponData = weapons.getNewWeaponData(spyWeapon);
			obj_id holoWeapon = null;
			
			if (isDefaultWeapon(spyWeapon))
			{
				holoWeapon = getCurrentWeapon(hologram);
			}
			else
			{
				holoWeapon = createObject(getTemplateName(spyWeapon), hologram, "");
			}
			
			setObjVar(hologram, "intCombatDifficulty", getLevel(spy));
			setLevel(hologram, getLevel(spy));
			setObjVar(hologram, "difficultyClass", 0);
			
			if (isIdValid(holoWeapon))
			{
				setWeaponAttackSpeed(holoWeapon, 1.0f);
				setWeaponMaxDamage(holoWeapon, 2);
				setWeaponMinDamage(holoWeapon, 1);
				weapons.setWeaponData(holoWeapon);
				
				utils.setScriptVar(holoWeapon, "isCreatureWeapon", 1);
				
			}
			
			setCurrentWeapon(hologram, holoWeapon);
			
			setMaxAttrib(hologram, HEALTH, getMaxAttrib(spy, HEALTH) );
			setAttrib( hologram, HEALTH, getMaxAttrib(spy, HEALTH) );
			
			utils.setScriptVar(spy, combat.DAMAGE_REDIRECT, hologram);
			
			attachScript(hologram, "ai.ai");
			attachScript(hologram, "ai.creature_combat");
			attachScript(hologram, "systems.combat.combat_actions");
			attachScript(hologram, "ai.spy_decoy");
			
			int armorRating = utils.getIntScriptVar(spy, "armor.cache.generalProtection");
			utils.setScriptVar(hologram, "armor.cache.generalProtection", armorRating);
			
			int[] buffCrc = buff.getAllBuffs(spy);
			
			if (buffCrc != null && buffCrc.length > 0)
			{
				for (int i=0; i<buffCrc.length; i++)
				{
					testAbortScript();
					String curBuff = buff.getBuffNameFromCrc(buffCrc[i]);
					if (curBuff != null && curBuff.startsWith("invis_"))
					{
						continue;
					}
					
					buff.applyBuff(hologram, buffCrc[i], 360.0f);
				}
			}
			
			float dodgeChance = combat.getDefenderDodgeChance(spy);
			float parryChance = combat.getDefenderParryChance(spy);
			float blockChance = combat.getDefenderBlockChance(spy);
			float glanceChance = combat.getDefenderGlancingBlowChance(spy);
			float evadeChance = combat.getDefenderEvasionChance(spy);
			
			addSkillModModifier(hologram, "expertise_dodge", "expertise_dodge", (int)dodgeChance, 20.0f, false, true);
			addSkillModModifier(hologram, "expertise_parry", "expertise_parry", (int)parryChance, 20.0f, false, true);
			addSkillModModifier(hologram, "expertise_block_chance", "expertise_block_chance", (int)blockChance, 20.0f, false, true);
			addSkillModModifier(hologram, "expertise_glancing_blow_all", "expertise_glancing_blow_all", (int)glanceChance, 20.0f, false, true);
			addSkillModModifier(hologram, "expertise_evasion_chance", "expertise_evasion_chance", (int)evadeChance, 20.0f, false, true);
			
			if (hasCommand(spy, "sp_avoid_damage"))
			{
				messageTo(hologram, "queue_ad", null, 1.0f, false);
			}
			
			int diversionLevel = getEnhancedSkillStatisticModifierUncapped(spy, "expertise_improved_decoy");
			
			if (diversionLevel > 0)
			{
				utils.setScriptVar(hologram, "flank", true);
			}
			else
			{
				utils.setScriptVar(hologram, "flank", false);
			}
			
			return hologram;
		}
		
		return null;
	}
	
	
	public static boolean isDecoy(obj_id object) throws InterruptedException
	{
		return utils.hasScript(object, "ai.spy_decoy");
	}
}
