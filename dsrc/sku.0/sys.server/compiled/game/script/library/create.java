package script.library;

import script.*;
import script.base_class.*;
import script.combat_engine.*;
import java.util.Arrays;
import java.util.Hashtable;
import java.util.Vector;
import script.base_script;

import script.library.utils;
import script.library.hue;
import script.library.ai_lib;
import script.ai.ai_combat;
import script.library.factions;
import script.library.dressup;
import script.library.attrib;
import script.library.weapons;
import script.library.stealth;


public class create extends script.base_script
{
	public create()
	{
	}
	public static final String CREATURE_TABLE = "datatables/mob/creatures.iff";
	public static final String STAT_BALANCE_TABLE = "datatables/mob/stat_balance.iff";
	public static final String VEHICLE_TABLE = "datatables/vehicle/vehicle_template.iff";
	public static final String TEMPLATE_PREFIX = "object/mobile/";
	public static final String CREATURE_NAME_FILE = "mob/creature_names";
	public static final String NPC_CUSTOMIZATION_PREFIX = "datatables/npc_customization/";
	public static final String DRESSED_NPC_TABLE = "datatables/npc_customization/dressed_species.iff";
	public static final String DRESSED_NPC_TABLE_ROOT = "datatables/npc_customization/dressed_species_";
	public static final String BIOLINK_SCRIPT = "item.armor.biolink_item_non_faction";
	
	public static final float MELEE_SPEED_MOD = 1.25f;
	public static final float MELEE_HP_MOD = 1.15f;
	
	public static final String INITIALIZE_CREATURE_DO_NOT_SCALE_OBJVAR = "create_doNotRescale";
	
	public static final String[] MINATTRIBNAMES =
	{
		"minHealth",
		"minConstitution",
		"minAction",
		"minStamina",
		"minMind",
		"minWillpower"
	};
	
	public static final String[] MAXATTRIBNAMES =
	{
		"maxHealth",
		"maxConstitution",
		"maxAction",
		"maxStamina",
		"maxMind",
		"maxWillpower"
	};
	
	
	public static obj_id staticObject(String objectName, location spawnLocation) throws InterruptedException
	{
		
		if (objectName == null)
		{
			LOG("create", "create.staticObject called with NULL objectName!");
			return null;
		}
		
		obj_id creature = create.object( objectName, spawnLocation, false);
		if (creature != null)
		{
			setCreatureStatic(creature, true);
			setWantSawAttackTriggers( creature, false );
			setInvulnerable( creature, true );
		}
		return creature;
	}
	
	
	public static obj_id object(String objectName, location spawnLocation) throws InterruptedException
	{
		return object( objectName, spawnLocation, -1 );
	}
	
	
	public static obj_id object(String objectName, location spawnLocation, int level) throws InterruptedException
	{
		if (objectName == null)
		{
			LOG("create", "create.object called with NULL objectName!");
			return null;
		}
		
		return create.object( objectName, spawnLocation, level, true);
	}
	
	
	public static obj_id object(String objectName, location spawnLocation, boolean withAi) throws InterruptedException
	{
		return object( objectName, spawnLocation, -1, withAi );
	}
	
	
	public static obj_id object(String objectName, location spawnLocation, int level, boolean withAi) throws InterruptedException
	{
		return create.object( objectName, spawnLocation, level, withAi, false );
	}
	
	
	public static obj_id object(String objectName, location spawnLocation, boolean withAi, boolean isPet) throws InterruptedException
	{
		return object( objectName, spawnLocation, -1, withAi, isPet );
	}
	
	
	public static obj_id object(String objectName, location spawnLocation, int level, boolean withAi, boolean isPet) throws InterruptedException
	{
		if (objectName == null)
		{
			LOG( "create", "create.object called with NULL objectName!");
			return null;
		}
		if (objectName.endsWith(".iff"))
		{
			LOG( "create", getName( getSelf() ) + " creating: "+ objectName + " at "+ spawnLocation );
			obj_id creature = createObject( objectName, spawnLocation );
			if (isIdValid(creature))
			{
				utils.setScriptVar( creature, "spawnedBy", getSelf() );
			}
			
			return creature;
		}
		else
		{
			obj_id creature = createCreature( objectName, spawnLocation, level, withAi, isPet );
			if (isIdValid(creature))
			{
				utils.setScriptVar( creature, "spawnedBy", getSelf() );
			}
			
			return creature;
		}
	}
	
	
	public static obj_id object(String objectName, obj_id objContainer, boolean withAi, boolean isPet) throws InterruptedException
	{
		obj_id creature = object( objectName, objContainer, -1, withAi, isPet );
		if (isIdValid(creature))
		{
			utils.setScriptVar( creature, "spawnedBy", getSelf() );
		}
		
		return creature;
	}
	
	
	public static obj_id object(String objectName, obj_id objContainer, int level, boolean withAi, boolean isPet) throws InterruptedException
	{
		if (objectName == null)
		{
			LOG( "create", "create.object called with NULL objectName!");
			return null;
		}
		if (objectName.endsWith(".iff"))
		{
			LOG( "create", getName( getSelf() ) + " creating: "+ objectName + " at "+ objContainer);
			obj_id creature = createObject( objectName, objContainer, "");
			if (isIdValid(creature))
			{
				utils.setScriptVar( creature, "spawnedBy", getSelf() );
			}
			
			return creature;
		}
		else
		{
			obj_id creature = createCreature( objectName, objContainer, level, withAi, isPet );
			if (isIdValid(creature))
			{
				utils.setScriptVar( creature, "spawnedBy", getSelf() );
			}
			
			return creature;
		}
	}
	
	
	public static obj_id object(String objectName, float xOffset, float zOffset) throws InterruptedException
	{
		return object( objectName, xOffset, zOffset, -1 );
	}
	
	
	public static obj_id object(String objectName, float xOffset, float zOffset, int level) throws InterruptedException
	{
		if (objectName == null)
		{
			LOG("create", "create.object called with NULL objectName!");
			return null;
		}
		
		location spawnLoc = new location( getLocation( getSelf() ) );
		spawnLoc.x += xOffset;
		spawnLoc.z += zOffset;
		return create.object( objectName, spawnLoc, level );
	}
	
	
	public static obj_id themeParkObject(String objectName, float xOffset, float zOffset, String destroyHandlerName, float destroyMessageDelay) throws InterruptedException
	{
		if (objectName == null)
		{
			LOG("create", "create.themeParkObject called with NULL objectName!");
			return null;
		}
		
		obj_id newObject = create.themeParkObject( objectName, xOffset, zOffset );
		addDestroyMessage( newObject, destroyHandlerName, destroyMessageDelay, getSelf() );
		return newObject;
	}
	
	
	public static obj_id themeParkObject(String objectName, float xOffset, float zOffset) throws InterruptedException
	{
		if (objectName == null)
		{
			LOG("create", "create.themeParkObject called with NULL objectName!");
			return null;
		}
		
		location spawnLoc = new location( getLocation( getSelf() ) );
		spawnLoc.x += xOffset;
		spawnLoc.z += zOffset;
		obj_id newObject = create.object( objectName, spawnLoc );
		setInvulnerable( newObject, true );
		ai_lib.setDefaultCalmBehavior( newObject, ai_lib.BEHAVIOR_SENTINEL );
		return newObject;
	}
	
	
	public static obj_id createNpc(String creatureName, String templateName, location spawnLocation) throws InterruptedException
	{
		return createNpc( creatureName, templateName, spawnLocation, -1 );
	}
	
	
	public static obj_id createNpc(String creatureName, String templateName, location spawnLocation, int level) throws InterruptedException
	{
		if (creatureName == null)
		{
			LOG("create", "create.createNpc called with NULL creatureName!");
			return null;
		}
		
		if (templateName == null)
		{
			LOG("create", "create.createNpc called with NULL templateName!");
			return null;
		}
		
		dictionary creatureDict = utils.dataTableGetRow( CREATURE_TABLE, creatureName );
		if (creatureDict == null)
		{
			return null;
		}
		
		if (!templateName.endsWith(".iff" ))
		{
			
			String tableName = DRESSED_NPC_TABLE_ROOT + templateName.charAt(0) + ".iff";
			if (dataTableHasColumn( tableName, templateName ))
			{
				String[] templateList = dataTableGetStringColumnNoDefaults( tableName, templateName );
				if (templateList.length < 1 || templateList == null)
				{
					LOG( "create", templateName + " column in dressed_species is empty!");
					return null;
				}
				templateName = templateList[rand(0, templateList.length-1 ) ];
			}
			else
			{
				
				String[] templateList = dataTableGetStringColumnNoDefaults( NPC_CUSTOMIZATION_PREFIX + templateName + ".iff", rand(0,1));
				if (templateList.length < 1 || templateList == null)
				{
					LOG( "create", templateName + " datatable does not exist!");
					return null;
				}
				templateName = templateList[rand(0, templateList.length-1 ) ];
			}
		}
		return createCreature( creatureName, templateName, spawnLocation, creatureDict, level, true );
	}
	
	
	public static obj_id createNpc(String creatureName, String templateName, float xOffset, float zOffset) throws InterruptedException
	{
		return createNpc( creatureName, templateName, xOffset, zOffset, -1 );
	}
	
	
	public static obj_id createNpc(String creatureName, String templateName, float xOffset, float zOffset, int level) throws InterruptedException
	{
		
		if (creatureName == null)
		{
			LOG("create", "create.createNpc called with NULL creatureName!");
			return null;
		}
		
		if (templateName == null)
		{
			LOG("create", "create.createNpc called with NULL templateName!");
			return null;
		}
		
		location spawnLoc = new location( getLocation( getSelf() ) );
		spawnLoc.x += xOffset;
		spawnLoc.z += zOffset;
		return create.createNpc( creatureName, templateName, spawnLoc, level );
	}
	
	
	public static obj_id createCreature(String creatureName, location spawnLocation, boolean withAi) throws InterruptedException
	{
		return createCreature( creatureName, spawnLocation, -1, withAi );
	}
	
	
	public static obj_id createCreature(String creatureName, location spawnLocation, int level, boolean withAi) throws InterruptedException
	{
		return create.createCreature( creatureName, spawnLocation, level, withAi, false );
	}
	
	
	public static obj_id createCreature(String creatureName, location spawnLocation, boolean withAi, boolean isPet) throws InterruptedException
	{
		return createCreature( creatureName, spawnLocation, -1, withAi, isPet );
	}
	
	
	public static obj_id createCreature(String creatureName, location spawnLocation, int level, boolean withAi, boolean isPet) throws InterruptedException
	{
		if (creatureName == null)
		{
			LOG("create", "create.createCreature called with NULL creatureName!");
			return null;
		}
		
		dictionary creatureDict = utils.dataTableGetRow( CREATURE_TABLE, creatureName );
		if (creatureDict == null)
		{
			debugServerConsoleMsg( null, "WARNING: Unable to spawn invalid creatureType: "+ creatureName );
			return null;
		}
		
		String templateName = creatureDict.getString("template");
		if (templateName.equals("") || templateName == null)
		{
			debugServerConsoleMsg( null, "WARNING: Unable to spawn invalid creatureType: "+ creatureName + " - bad templateName?");
			return null;
		}
		
		if (!templateName.endsWith(".iff" ))
		{
			
			String tableName = DRESSED_NPC_TABLE_ROOT + templateName.charAt(0) + ".iff";
			if (dataTableHasColumn( tableName, templateName ))
			{
				String[] templateList = dataTableGetStringColumnNoDefaults( tableName, templateName );
				if (templateList.length < 1 || templateList == null)
				{
					LOG( "create", templateName + " column in dressed_species is empty!");
					return null;
				}
				templateName = templateList[rand(0, templateList.length-1 ) ];
			}
			else
			{
				String[] templateList = dataTableGetStringColumnNoDefaults( NPC_CUSTOMIZATION_PREFIX + templateName + ".iff", rand(0,1));
				if (templateList == null)
				{
					debugServerConsoleMsg( null, "WARNING: Unable to spawn "+ creatureName + " - bad templateName? "+ templateName );
					LOG( "create", templateName + " does not exist!");
					return null;
				}
				if (templateList.length == 0)
				{
					debugServerConsoleMsg( null, "WARNING: Unable to spawn "+ creatureName + " - bad templateName? "+ templateName );
					LOG( "create", templateName + " File was Empty!");
					return null;
				}
				templateName = templateList[rand(0, templateList.length-1 ) ] + ".iff";
			}
		}
		return createCreature( creatureName, templateName, spawnLocation, creatureDict, level, withAi, isPet );
	}
	
	
	public static obj_id createCreature(String creatureName, obj_id objContainer, boolean withAi, boolean isPet) throws InterruptedException
	{
		return createCreature( creatureName, objContainer, -1, withAi, isPet );
	}
	
	
	public static obj_id createCreature(String creatureName, obj_id objContainer, int level, boolean withAi, boolean isPet) throws InterruptedException
	{
		if (creatureName == null)
		{
			LOG("create", "create.createCreature called with NULL creatureName!");
			return null;
		}
		
		dictionary creatureDict = utils.dataTableGetRow( CREATURE_TABLE, creatureName );
		if (creatureDict == null)
		{
			debugServerConsoleMsg( null, "WARNING: Unable to spawn invalid creatureType: "+ creatureName );
			return null;
		}
		
		String templateName = creatureDict.getString("template");
		if (templateName.equals("") || templateName == null)
		{
			debugServerConsoleMsg( null, "WARNING: Unable to spawn invalid creatureType: "+ creatureName + " - bad templateName?");
			return null;
		}
		
		if (!templateName.endsWith(".iff" ))
		{
			
			String tableName = DRESSED_NPC_TABLE_ROOT + templateName.charAt(0) + ".iff";
			if (dataTableHasColumn( tableName, templateName ))
			{
				String[] templateList = dataTableGetStringColumnNoDefaults( tableName, templateName );
				if (templateList.length < 1 || templateList == null)
				{
					LOG( "create", templateName + " column in dressed_species is empty!");
					return null;
				}
				templateName = templateList[rand(0, templateList.length-1 ) ];
			}
			else
			{
				String[] templateList = dataTableGetStringColumnNoDefaults( NPC_CUSTOMIZATION_PREFIX + templateName + ".iff", rand(0,1));
				if (templateList == null)
				{
					debugServerConsoleMsg( null, "WARNING: Unable to spawn "+ creatureName + " - bad templateName? "+ templateName );
					LOG( "create", templateName + " does not exist!");
					return null;
				}
				if (templateList.length == 0)
				{
					debugServerConsoleMsg( null, "WARNING: Unable to spawn "+ creatureName + " - bad templateName? "+ templateName );
					LOG( "create", templateName + " File was Empty!");
					return null;
				}
				templateName = templateList[rand(0, templateList.length-1 ) ] + ".iff";
			}
		}
		return createCreature( creatureName, templateName, objContainer, creatureDict, level, withAi, isPet );
	}
	
	
	public static obj_id createCreature(String creatureName, String templateName, location spawnLocation, dictionary creatureDict, boolean withAi) throws InterruptedException
	{
		return createCreature( creatureName, templateName, spawnLocation, creatureDict, -1, withAi );
	}
	
	
	public static obj_id createCreature(String creatureName, String templateName, location spawnLocation, dictionary creatureDict, int level, boolean withAi) throws InterruptedException
	{
		return createCreature( creatureName, templateName, spawnLocation, creatureDict, level, withAi, false );
	}
	
	
	public static obj_id createCreature(String creatureName, String templateName, location spawnLocation, dictionary creatureDict, boolean withAi, boolean isPet) throws InterruptedException
	{
		return createCreature( creatureName, templateName, spawnLocation, creatureDict, -1, withAi, isPet );
	}
	
	
	public static obj_id createCreature(String creatureName, String templateName, location spawnLocation, dictionary creatureDict, int level, boolean withAi, boolean isPet) throws InterruptedException
	{
		if (templateName.equals("") || templateName == null)
		{
			LOG( "create", creatureName + " could not be made because templateName is null");
			return null;
		}
		if (creatureName.equals("") || creatureName == null)
		{
			LOG( "create", "creatureName is null - can't spawn whatever this was supposed to be");
			return null;
		}
		
		templateName = TEMPLATE_PREFIX + templateName;
		
		LOG( "create", getName( getSelf() ) + " creating: "+ creatureName + " at "+ spawnLocation );
		obj_id creature = createObject( templateName, spawnLocation );
		if (!isIdValid(creature))
		{
			return null;
		}
		else
		{
			utils.setScriptVar( creature, "spawnedBy", getSelf() );
		}
		
		if (isPet)
		{
			utils.setScriptVar( creature, "petBeingInitialized", true );
		}
		
		String finalCreatureName = getStringObjVar( creature, "ai.creatureBaseName");
		if (finalCreatureName == null || finalCreatureName.equals(""))
		{
			finalCreatureName = creatureName;
		}
		
		randomlyNameCreature( creature, finalCreatureName );
		
		initializeCreature ( creature, creatureName, creatureDict, level );
		
		attachCreatureScripts( creature, creatureDict.getString( "scripts"), withAi );
		
		return creature;
	}
	
	
	public static obj_id createCreature(String creatureName, String templateName, obj_id objContainer, dictionary creatureDict, boolean withAi, boolean isPet) throws InterruptedException
	{
		return createCreature( creatureName, templateName, objContainer, creatureDict, -1, withAi, isPet );
	}
	
	
	public static obj_id createCreature(String creatureName, String templateName, obj_id objContainer, dictionary creatureDict, int level, boolean withAi, boolean isPet) throws InterruptedException
	{
		if (templateName.equals("") || templateName == null)
		{
			LOG( "create", creatureName + " could not be made because templateName is null");
			return null;
		}
		if (creatureName.equals("") || creatureName == null)
		{
			LOG( "create", "creatureName is null - can't spawn whatever this was supposed to be");
			return null;
		}
		
		templateName = TEMPLATE_PREFIX + templateName;
		
		LOG( "create", getName( getSelf() ) + " creating: "+ creatureName + " at "+ objContainer );
		obj_id creature = createObject( templateName, objContainer, "");
		if (!isIdValid(creature))
		{
			return null;
		}
		else
		{
			utils.setScriptVar( creature, "spawnedBy", getSelf() );
		}
		
		if (isPet)
		{
			utils.setScriptVar( creature, "petBeingInitialized", true );
		}
		
		String finalCreatureName = getStringObjVar( creature, "ai.creatureBaseName");
		if (finalCreatureName == null || finalCreatureName.equals(""))
		{
			finalCreatureName = creatureName;
		}
		
		randomlyNameCreature( creature, finalCreatureName );
		
		initializeCreature ( creature, creatureName, creatureDict, level );
		
		attachCreatureScripts( creature, creatureDict.getString( "scripts"), withAi );
		
		return creature;
	}
	
	
	public static void initializeCreature(obj_id creature, String creatureName, dictionary creatureDict) throws InterruptedException
	{
		initializeCreature( creature, creatureName, creatureDict, -1);
	}
	
	
	public static void initializeCreature(obj_id creature, String creatureName, dictionary creatureDict, int level) throws InterruptedException
	{
		LOGC(aiLoggingEnabled(creature), "debug_ai", ("########## create::initializeCreature() BEGIN ai("+ creature + ") creatureName("+ creatureName + ") ##########"));
		
		setCreatureName(creature, creatureName);
		
		float minScale = creatureDict.getFloat( "minScale");
		float maxScale = creatureDict.getFloat( "maxScale");
		float baseScale = getScale( creature );
		if (baseScale != 1.0f)
		{
			utils.setScriptVar( creature, "ai.baseScale", baseScale );
		}
		
		if (!hasObjVar(creature, "storytellerid") && !hasObjVar(creature, INITIALIZE_CREATURE_DO_NOT_SCALE_OBJVAR))
		{
			float newScale = baseScale * rand( minScale, maxScale );
			setScale( creature, newScale );
			
			setYaw( creature, rand(0.0f, 360.0f ) );
		}
		
		setObjVar(creature, "creature_type", creatureName);
		setObjVar(creature, "socialGroup", creatureDict.getString("socialGroup"));
		
		int huevar = creatureDict.getInt( "hue");
		if (huevar != 0)
		{
			int highhuevar = (huevar*8)-1;
			int lowhuevar = highhuevar-7;
			huevar = rand(lowhuevar, highhuevar);
			ranged_int_custom_var[] c = hue.getPalcolorVars(creature);
			if (c != null)
			{
				for (int i = 0; i < c.length; i++)
				{
					testAbortScript();
					c[i].setValue(huevar);
				}
			}
		}
		
		int baseLevel = creatureDict.getInt("BaseLevel");
		int dmgLevel = baseLevel + creatureDict.getInt("Damagelevelmodifier");
		int statLevel = baseLevel + creatureDict.getInt("StatLevelModifier");
		int toHitLevel = baseLevel + creatureDict.getInt("ToHitLevelModifier");
		int armorLevel = baseLevel + creatureDict.getInt("ArmorLevelModifier");
		
		if (dmgLevel <= 0)
		{
			dmgLevel = 1;
		}
		if (statLevel <= 0)
		{
			statLevel = 1;
		}
		if (toHitLevel <= 0)
		{
			toHitLevel = 1;
		}
		if (armorLevel <= 0)
		{
			armorLevel = 1;
		}
		
		if (level > 0)
		{
			dmgLevel = level;
			statLevel = level;
			toHitLevel = level;
			armorLevel = level;
		}
		else
		{
			
			level = calcCreatureLevel(statLevel, dmgLevel, toHitLevel, armorLevel);
		}
		
		setObjVar(creature, "intCombatDifficulty", level);
		setLevel(creature, level);
		
		int stealType = creatureDict.getInt("stealingFlags");
		utils.setScriptVar(creature, stealth.STEAL_TYPE, stealType);
		
		int difficultyClass = creatureDict.getInt("difficultyClass");
		if (difficultyClass < 0)
		{
			difficultyClass = 0;
		}
		
		setObjVar(creature, "difficultyClass", difficultyClass);
		
		String diffClassName = "";
		if (difficultyClass == 1)
		{
			diffClassName = "Elite_";
		}
		if (difficultyClass == 2)
		{
			diffClassName = "Boss_";
		}
		
		float damagePerSecond = dataTableGetFloat(STAT_BALANCE_TABLE, dmgLevel - 1, diffClassName+"damagePerSecond");
		
		int toHitChance = dataTableGetInt(STAT_BALANCE_TABLE, toHitLevel - 1, diffClassName + "ToHit");
		int defenseValue = dataTableGetInt(STAT_BALANCE_TABLE, toHitLevel - 1, diffClassName + "Def");
		boolean hasRanged = false;
		obj_id creatureWeapon = getCurrentWeapon( creature );
		float primarySpeed = creatureDict.getFloat("primary_weapon_speed");
		float secondarySpeed = creatureDict.getFloat("secondary_weapon_speed");
		
		int priMinDamage = Math.round((damagePerSecond * primarySpeed) * 0.5f);
		int priMaxDamage = Math.round((damagePerSecond * primarySpeed) * 1.5f);
		
		int secMinDamage = Math.round((damagePerSecond * secondarySpeed) * 0.5f);
		int secMaxDamage = Math.round((damagePerSecond * secondarySpeed) * 1.5f);
		
		if (isIdValid(creatureWeapon))
		{
			setWeaponAttackSpeed(creatureWeapon, primarySpeed);
			setWeaponMaxDamage(creatureWeapon, priMaxDamage);
			setWeaponMinDamage(creatureWeapon, priMinDamage);
			weapons.setWeaponData(creatureWeapon);
			
			utils.setScriptVar(creatureWeapon, "isCreatureWeapon", 1);
			setObjVar(creatureWeapon, "isCreatureWeapon", 1);
		}
		
		obj_id defaultWeapon = getDefaultWeapon( creature );
		if (isIdValid(defaultWeapon))
		{
			setWeaponAttackSpeed(defaultWeapon, primarySpeed);
			setWeaponMaxDamage(defaultWeapon, secMaxDamage);
			setWeaponMinDamage(defaultWeapon, secMinDamage);
			weapons.setWeaponData(defaultWeapon);
			
			utils.setScriptVar(defaultWeapon, "isCreatureWeapon", 1);
			setObjVar(defaultWeapon, "isCreatureWeapon", 1);
		}
		
		{
			if (aiHasPrimaryWeapon(creature))
			{
				final obj_id primaryWeapon = aiGetPrimaryWeapon(creature);
				dictionary primDat = weapons.getWeaponDat(primaryWeapon);
				
				if (primDat != null)
				{
					
					weapons.setWeaponAttributes(primaryWeapon, primDat, 1f);
				}
				
				setWeaponAttackSpeed(primaryWeapon, primarySpeed);
				setWeaponMaxDamage(primaryWeapon, priMaxDamage);
				setWeaponMinDamage(primaryWeapon, priMinDamage);
				weapons.setWeaponData(primaryWeapon);
				hasRanged = hasRanged || combat.isRangedWeapon(primaryWeapon);
				if (hasScript(primaryWeapon, BIOLINK_SCRIPT))
				{
					cleanOffBioLink(primaryWeapon);
				}
				
				utils.setScriptVar(primaryWeapon, "isCreatureWeapon", 1);
				setObjVar(primaryWeapon, "isCreatureWeapon", 1);
			}
		}
		
		{
			if (aiHasSecondaryWeapon(creature))
			{
				final obj_id secondaryWeapon = aiGetSecondaryWeapon(creature);
				dictionary secDat = weapons.getWeaponDat(secondaryWeapon);
				
				if (secDat != null)
				{
					weapons.setWeaponAttributes(secondaryWeapon, secDat, 1f);
				}
				
				setWeaponAttackSpeed(secondaryWeapon, secondarySpeed);
				setWeaponMaxDamage(secondaryWeapon, secMaxDamage);
				setWeaponMinDamage(secondaryWeapon, secMinDamage);
				weapons.setWeaponData(secondaryWeapon);
				hasRanged = hasRanged || combat.isRangedWeapon(secondaryWeapon);
				if (hasScript(secondaryWeapon, BIOLINK_SCRIPT))
				{
					cleanOffBioLink(secondaryWeapon);
				}
				
				utils.setScriptVar(secondaryWeapon, "isCreatureWeapon", 1);
				setObjVar(secondaryWeapon, "isCreatureWeapon", 1);
			}
		}
		
		float speedMod = 1;
		float hpMod = 1;
		if (hasRanged)
		{
			speedMod = MELEE_SPEED_MOD;
			hpMod = MELEE_HP_MOD;
		}
		
		int avgAttribHealth = dataTableGetInt(STAT_BALANCE_TABLE, statLevel - 1, diffClassName + "HP");
		int minAttribHealth = minAttribHealth = (int)(avgAttribHealth * 0.9f);
		int maxAttribHealth = maxAttribHealth = (int)(avgAttribHealth * 1.1f);
		float newAttribValueHealth = rand( minAttribHealth, maxAttribHealth );
		newAttribValueHealth *= hpMod;
		
		setMaxAttrib(creature, HEALTH, (int)newAttribValueHealth );
		setAttrib( creature, HEALTH, (int)newAttribValueHealth );
		
		int avgAttribAction = dataTableGetInt(STAT_BALANCE_TABLE, statLevel - 1, diffClassName + "Action");
		int minAttribAction = minAttribAction = (int)(avgAttribAction * 0.9f);
		int maxAttribAction = maxAttribAction = (int)(avgAttribAction * 1.1f);
		int newAttribValueAction = rand( minAttribAction, maxAttribAction);
		setMaxAttrib(creature, ACTION, newAttribValueAction );
		setAttrib( creature, ACTION, newAttribValueAction );
		
		setMaxAttrib(creature, MIND, 1000 );
		setAttrib( creature, MIND, 1000 );
		
		int healthRegen = dataTableGetInt(STAT_BALANCE_TABLE, statLevel - 1, "HealthRegen");
		int actionRegen = dataTableGetInt(STAT_BALANCE_TABLE, statLevel - 1, "ActionRegen");
		int mindRegen = dataTableGetInt(STAT_BALANCE_TABLE, statLevel - 1, "MindRegen");
		
		setRegenRate(creature, CONSTITUTION, healthRegen);
		
		setRegenRate(creature, STAMINA, healthRegen);
		
		setRegenRate(creature, WILLPOWER, healthRegen);
		
		float normalRegen = dataTableGetFloat(STAT_BALANCE_TABLE, statLevel - 1, diffClassName + "Regen");
		float combatRegen = dataTableGetFloat(STAT_BALANCE_TABLE, statLevel - 1, diffClassName + "CombatRegen");
		
		int xpValue = xp.getLevelBasedXP(level);
		setObjVar( creature, "combat.intCombatXP", xpValue );
		
		{
			float runSpeed = ai_lib.AI_MAX_MOVEMENT_SPEED * aiGetMovementSpeedPercent(creature);
			runSpeed *= speedMod;
			setBaseRunSpeed(creature, runSpeed);
			
			{
				if (getBaseWalkSpeed(creature) > runSpeed)
				{
					setBaseWalkSpeed(creature, runSpeed);
				}
			}
			
			LOGC(aiLoggingEnabled(creature), "debug_ai", ("create::initializeCreature() ai("+ creature + ") runSpeed("+ runSpeed + ")"));
		}
		
		applySkillStatisticModifiers( creature, toHitChance, defenseValue );
		
		applyCreatureImmunities(creature, creatureDict);
		
		setCreatureObjVars( creature, creatureDict.getString( "objvars") );
		
		int pvpOnly = creatureDict.getInt("isSpecialForces");
		
		String faction = creatureDict.getString( "pvpFaction");
		if ((faction != null) && (!faction.equals("")))
		{
			factions.setFaction( creature, faction, pvpOnly);
		}
		
		int invulnerable = creatureDict.getInt( "invulnerable");
		if (invulnerable != 0)
		{
			setInvulnerable( creature, true );
		}
		
		String diction = creatureDict.getString( "diction");
		if (!diction.equals("none"))
		{
			setObjVar( creature, "ai.diction", diction );
		}
		
		int niche = creatureDict.getInt( "niche");
		
		if (getTopMostContainer(creature) != creature)
		{
			ai_lib.setDefaultCalmBehavior(creature, ai_lib.BEHAVIOR_SENTINEL);
		}
		
		randomlyNameCreature( creature, creatureName );
		
		setAttributeAttained( creature, attrib.ALL);
		switch ( niche )
		{
			case NICHE_HERBIVORE :
			setAttributeAttained( creature, attrib.HERBIVORE );
			setAttributeInterested( creature, attrib.CARNIVORE );
			break;
			case NICHE_CARNIVORE :
			case NICHE_PREDATOR :
			setAttributeAttained( creature, attrib.CARNIVORE );
			setAttributeInterested( creature, attrib.HERBIVORE );
			setAttributeInterested( creature, attrib.NPC );
			break;
			case NICHE_NPC :
			setAttributeAttained( creature, attrib.NPC );
			setAttributeInterested( creature, attrib.CARNIVORE );
			setUpFactionalEnemyAttribs( creature, faction );
			break;
			case NICHE_ANDROID :
			setAttributeAttained( creature, attrib.NPC );
			setAttributeInterested( creature, attrib.CARNIVORE );
			setUpFactionalEnemyAttribs( creature, faction );
			break;
		}
		
		String strLootTable = creatureDict.getString("lootTable");
		int intRolls = creatureDict.getInt("intLootRolls");
		int intPercentage = creatureDict.getInt("intRollPercent");
		if (!strLootTable.equals(""))
		{
			LOG("npe", "table is "+strLootTable);
			int intItems = 0;
			for (int intI =0; intI < intRolls; intI++)
			{
				testAbortScript();
				int intRand = rand(1, 99);
				LOG("npe", "roll is "+intRand+" and +% is "+intPercentage);
				if (intRand < intPercentage)
				{
					
					intItems++;
				}
			}
			LOG("npe", "items is "+intItems);
			
			setObjVar(creature, "loot.lootTable", strLootTable);
			setObjVar(creature, "loot.numItems", intItems);
		}
		
		if (!initializeArmor(creature, creatureDict, dataTableGetInt(STAT_BALANCE_TABLE, armorLevel - 1, diffClassName + "Armor")))
		{
			LOG( "create", "Unable to initialize armor on "+ creatureName + "("+ creature + ")");
		}
		
		int stringCheck = creatureName.indexOf("recruiter");
		if (stringCheck > -1)
		{
			String pvpFaction = creatureDict.getString( "pvpFaction");
			setFactionRecruiter(creature, pvpFaction);
		}
		
		if (hasObjVar(creature, "detect_hidden"))
		{
			int detectInt = getIntObjVar(creature, "detect_hidden");
			if (detectInt > 0)
			{
				applySkillStatisticModifier(creature, "detect_hidden", detectInt);
			}
		}
		
		LOGC(aiLoggingEnabled(creature), "debug_ai", ("########## create::initializeCreature() END ai("+ creature + ") creatureName("+ creatureName + ") ##########"));
	}
	
	
	public static int calcCreatureLevel(String creatureName) throws InterruptedException
	{
		dictionary creatureDict = utils.dataTableGetRow( CREATURE_TABLE, creatureName );
		if (creatureDict == null)
		{
			debugServerConsoleMsg( null, "WARNING: Unable to find invalid creatureType: "+ creatureName );
			return -1;
		}
		
		return calcCreatureLevel(creatureDict);
	}
	
	
	public static int calcCreatureLevel(dictionary creatureDict) throws InterruptedException
	{
		
		int baseLevel = creatureDict.getInt("BaseLevel");
		int dmgLevel = baseLevel + creatureDict.getInt("Damagelevelmodifier");
		int statLevel = baseLevel + creatureDict.getInt("StatLevelModifier");
		int toHitLevel = baseLevel + creatureDict.getInt("ToHitLevelModifier");
		int armorLevel = baseLevel + creatureDict.getInt("ArmorLevelModifier");
		
		if (statLevel <= 0)
		{
			statLevel = 1;
		}
		if (dmgLevel <= 0)
		{
			dmgLevel = 1;
		}
		if (toHitLevel <= 0)
		{
			toHitLevel = 1;
		}
		if (armorLevel <= 0)
		{
			armorLevel = 1;
		}
		
		return calcCreatureLevel(statLevel, dmgLevel, toHitLevel, armorLevel);
	}
	
	
	public static int calcCreatureLevel(int statLevel, int dmgLevel, int toHitLevel, int armorLevel) throws InterruptedException
	{
		int level = 0;
		
		{
			
			level = (int)(((statLevel*6) + (dmgLevel*6) + (toHitLevel*2) + (armorLevel*4))/18f);
		}
		
		return level;
	}
	
	
	public static void attachCreatureScripts(obj_id creature, String scriptList, boolean withAi) throws InterruptedException
	{
		if (getConfigSetting("GameServer", "disableAI")!=null)
		{
			return;
		}
		
		if (withAi)
		{
			
			attachScript( creature, "ai.ai");
			attachCombatScripts( creature );
			attachScript(creature, "systems.skills.stealth.player_stealth");
			
		}
		
		if (scriptList == null || scriptList.equals(""))
		{
			return;
		}
		
		String[] scriptArray = split(scriptList, ',');
		for (int i = 0; i < scriptArray.length; i++)
		{
			testAbortScript();
			attachScript( creature, scriptArray[i] );
		}
	}
	
	
	public static boolean initializeArmor(obj_id creature, dictionary creatureDict) throws InterruptedException
	{
		int armorLevel = creatureDict.getInt("BaseLevel") + creatureDict.getInt("ArmorLevelModifier");
		
		int difficultyClass = creatureDict.getInt("difficultyClass");
		if (difficultyClass < 0)
		{
			difficultyClass = 0;
		}
		
		String diffClassName = "";
		if (difficultyClass == 1)
		{
			diffClassName = "Elite_";
		}
		if (difficultyClass == 2)
		{
			diffClassName = "Boss_";
		}
		
		int armorRating = dataTableGetInt(STAT_BALANCE_TABLE, armorLevel - 1, diffClassName + "Armor");
		
		return initializeArmor(creature, creatureDict, armorRating);
	}
	
	
	public static boolean initializeArmor(obj_id creature, dictionary creatureDict, int armorRating) throws InterruptedException
	{
		int[] armorData = new int[10];
		
		armorData[0] = 0;
		armorData[1] = armorRating;
		armorData[2] = creatureDict.getInt("armorKinetic");
		armorData[3] = creatureDict.getInt("armorEnergy");
		armorData[4] = creatureDict.getInt("armorBlast");
		armorData[5] = creatureDict.getInt("armorHeat");
		armorData[6] = creatureDict.getInt("armorCold");
		armorData[7] = creatureDict.getInt("armorElectric");
		armorData[8] = creatureDict.getInt("armorAcid");
		armorData[9] = creatureDict.getInt("armorStun");
		
		return initializeArmor(creature, armorData);
	}
	
	
	public static boolean initializeArmor(obj_id creature, int[] armorData) throws InterruptedException
	{
		int armorRating = armorData[0];
		int armorEffectiveness = armorData[1];
		int armorKinetic = armorData[2];
		int armorEnergy = armorData[3];
		int armorBlast = armorData[4];
		int armorHeat = armorData[5];
		int armorCold = armorData[6];
		int armorElectric = armorData[7];
		int armorAcid = armorData[8];
		int armorStun = armorData[9];
		
		boolean success = true;
		int vulnerability = DAMAGE_RESTRAINT;
		
		armor.removeAllArmorData(creature);
		
		String armorCategoryObjVar = armor.OBJVAR_ARMOR_BASE + "."+ armor.OBJVAR_ARMOR_CATEGORY;
		setObjVar(creature, armorCategoryObjVar, AC_battle);
		
		String armorLevelObjVar = armor.OBJVAR_ARMOR_BASE + "."+ armor.OBJVAR_ARMOR_LEVEL;
		setObjVar(creature, armorLevelObjVar, AL_standard);
		
		String genProtectionObjVar = armor.OBJVAR_ARMOR_BASE + "."+ armor.OBJVAR_GENERAL_PROTECTION;
		setObjVar(creature, genProtectionObjVar, (float)armorEffectiveness);
		utils.setScriptVar(creature, armor.SCRIPTVAR_CACHED_GENERAL_PROTECTION, armorEffectiveness);
		
		return success;
	}
	
	
	public static void setCreatureObjVars(obj_id creature, String objVarList) throws InterruptedException
	{
		
		utils.setObjVarsList(creature, objVarList);
	}
	
	
	public static String getLairDifficulty(int intMinDifficulty, int intMaxDifficulty, int intPlayerDifficulty) throws InterruptedException
	{
		
		final String[] LAIR_DIFFICULTIES =
		{
			"veryEasy", "easy", "medium", "hard", "veryHard"
		};
		
		int intDifference = intMaxDifficulty - intMinDifficulty;
		intPlayerDifficulty = intPlayerDifficulty - intMinDifficulty;
		
		if (intDifference == 0)
		{
			intDifference = 1;
		}
		float fltPercent = (float)(intPlayerDifficulty / intDifference);
		fltPercent = fltPercent * 100;
		int intIndex = (int)(fltPercent / 20);
		
		intIndex = intIndex - 1;
		if (intIndex < 0)
		{
			intIndex = 0;
		}
		if (intIndex >= LAIR_DIFFICULTIES.length)
		{
			intIndex = LAIR_DIFFICULTIES.length - 1;
		}
		
		return LAIR_DIFFICULTIES[intIndex];
	}
	
	
	public static void grantAllAbilities(obj_id npc) throws InterruptedException
	{
		return;
		
	}
	
	
	public static void attachCombatScripts(obj_id npc) throws InterruptedException
	{
		if (!hasScript( npc, "ai.creature_combat"))
		{
			attachScript( npc, "ai.creature_combat");
		}
		
		if (!hasScript(npc, "systems.combat.combat_actions"))
		{
			attachScript(npc, "systems.combat.combat_actions");
		}
		
		obj_id primaryWeapon = aiGetPrimaryWeapon(npc);
		obj_id secondaryWeapon = aiGetSecondaryWeapon(npc);
		boolean hasJediWeapon = false;
		
		if (isIdValid(primaryWeapon))
		{
			if (jedi.isLightsaber(primaryWeapon))
			{
				hasJediWeapon = true;
			}
		}
		
		if (isIdValid(secondaryWeapon))
		{
			if (jedi.isLightsaber(secondaryWeapon))
			{
				hasJediWeapon = true;
			}
		}
		
	}
	
	
	public static void applySkillStatisticModifiers(obj_id creature, int toHitChance, int defenseValue) throws InterruptedException
	{
		applySkillStatisticModifier(creature, "rifle_accuracy", toHitChance);
		applySkillStatisticModifier(creature, "toHitChance", toHitChance);
		applySkillStatisticModifier(creature, "carbine_accuracy", toHitChance);
		applySkillStatisticModifier(creature, "pistol_accuracy", toHitChance);
		applySkillStatisticModifier(creature, "heavyweapon_accuracy", toHitChance);
		applySkillStatisticModifier(creature, "onehandmelee_accuracy", toHitChance);
		applySkillStatisticModifier(creature, "twohandmelee_accuracy", toHitChance);
		applySkillStatisticModifier(creature, "unarmed_accuracy", toHitChance);
		applySkillStatisticModifier(creature, "polearm_accuracy", toHitChance);
		applySkillStatisticModifier(creature, "thrown_accuracy", toHitChance);
		applySkillStatisticModifier(creature, "onehandlightsaber_accuracy", toHitChance);
		applySkillStatisticModifier(creature, "twohandlightsaber_accuracy", toHitChance);
		applySkillStatisticModifier(creature, "polearmlightsaber_accuracy", toHitChance);
		applySkillStatisticModifier(creature, "force_accuracy", toHitChance);
		applySkillStatisticModifier(creature, "ranged_defense", defenseValue);
		applySkillStatisticModifier(creature, "melee_defense", defenseValue);
	}
	
	
	public static void applyCreatureImmunities(obj_id creature, dictionary creatureDict) throws InterruptedException
	{
		int rootImmune = creatureDict.getInt("rootImmune");
		if (rootImmune > 0)
		{
			applySkillStatisticModifier(creature, "movement_resist_root", rootImmune);
		}
		
		int snareImmune = creatureDict.getInt("snareImmune");
		if (snareImmune > 0)
		{
			applySkillStatisticModifier(creature, "movement_resist_snare", snareImmune);
		}
		
		int stunImmune = creatureDict.getInt("stunImmune");
		if (stunImmune > 0)
		{
			applySkillStatisticModifier(creature, "movement_resist_stun", stunImmune);
		}
		
		int mezImmune = creatureDict.getInt("mezImmune");
		if (mezImmune > 0)
		{
			applySkillStatisticModifier(creature, "movement_resist_mez", mezImmune);
		}
		
		int armorBreakImmune = creatureDict.getInt("canNotPunish");
		if (armorBreakImmune > 0)
		{
			utils.setScriptVar(creature, "combat.immune.no_punish", 1);
		}
		int tauntImmune = creatureDict.getInt("tauntImmune");
		if (tauntImmune > 0)
		{
			utils.setScriptVar(creature, "combat.immune.taunt", 1);
		}
		
		int ignorePlayer = creatureDict.getInt("ignorePlayer");
		if (ignorePlayer > 0)
		{
			utils.setScriptVar(creature, "combat.immune.taunt", 1);
			factions.setIgnorePlayer(creature);
		}
	}
	
	
	public static void addDestroyMessage(obj_id creature, String handlerName, float delay, obj_id recipient) throws InterruptedException
	{
		if (!isIdValid(creature))
		{
			return;
		}
		
		if (!hasScript( creature, "object.destroy_message" ))
		{
			attachScript( creature, "object.destroy_message");
		}
		
		Vector destroyMessageNames = new Vector();
		destroyMessageNames.setSize(0);
		Vector destroyMessageDelays = new Vector();
		destroyMessageDelays.setSize(0);
		Vector destroyMessageRecipients = new Vector();
		destroyMessageRecipients.setSize(0);
		
		if (hasObjVar( creature, "destroyMessageList" ))
		{
			destroyMessageNames = getResizeableStringArrayObjVar( creature, "destroyMessageList.handlerNames");
			destroyMessageDelays = getResizeableFloatArrayObjVar( creature, "destroyMessageList.delays");
			destroyMessageRecipients = getResizeableObjIdArrayObjVar( creature, "destroyMessageList.recipients");
		}
		
		destroyMessageNames = utils.addElement( destroyMessageNames, handlerName );
		destroyMessageDelays = utils.addElement( destroyMessageDelays, delay );
		destroyMessageRecipients = utils.addElement( destroyMessageRecipients, recipient );
		
		if ((((destroyMessageNames != null) && (destroyMessageNames.size() > 0)) && ((destroyMessageDelays != null) && (destroyMessageDelays.size() > 0)) && ((destroyMessageRecipients != null) && (destroyMessageRecipients.size() > 0))))
		{
			
			setObjVar(creature, "destroyMessageList.handlerNames", destroyMessageNames );
			setObjVar(creature, "destroyMessageList.delays", destroyMessageDelays );
			setObjVar(creature, "destroyMessageList.recipients", destroyMessageRecipients );
		}
	}
	
	
	public static void randomlyNameCreature(obj_id npc, String creatureName) throws InterruptedException
	{
		
		String templateName = getTemplateName( npc );
		if (templateName.indexOf("stormtrooper") != -1)
		{
			nameStormTrooper( npc );
			
		}
		
		if (templateName.indexOf("swamp_trooper") != -1)
		{
			nameSwampTrooper( npc );
			
		}
		
		if (templateName.indexOf("scout_trooper") != -1)
		{
			nameScoutTrooper( npc );
			
		}
		if (templateName.indexOf("tie_fighter") != -1)
		{
			nameImperialPilot( npc );
			
		}
		if (templateName.indexOf("dark_trooper") != -1)
		{
			nameDarkTrooper( npc );
			
		}
		
		String oldName = getAssignedName( npc );
		string_id nameId = new string_id( CREATURE_NAME_FILE, creatureName );
		
		String actualName = getString(nameId);
		if (actualName == null || actualName.equals(""))
		{
			setTitle( npc, creatureName );
			return;
		}
		
		setName( npc, "");
		setName( npc, nameId );
	}
	
	
	public static void setTitle(obj_id npc, String creatureName) throws InterruptedException
	{
		string_id nameId = new string_id( CREATURE_NAME_FILE, creatureName );
		if (nameId == null)
		{
			return;
		}
		
		String newName = getString( nameId );
		if (newName == null || newName.equals(""))
		{
			return;
		}
		
		String oldName = getAssignedName( npc );
		String oldNameId = getString( getNameStringId( npc ));
		if (oldName.equals(oldNameId))
		{
			setName( npc, "");
			setName( npc, nameId );
		}
		else if (oldName == null || oldName.equals(""))
		{
			setName( npc, "");
			setName( npc, nameId );
		}
		else
		{
			setName( npc, oldName + " ("+ getString(nameId) + ")");
		}
	}
	
	
	public static void nameStormTrooper(obj_id npc) throws InterruptedException
	{
		int designation = rand (1,5);
		String StName = "TK-";
		switch (designation)
		{
			case 1:
			StName = "TK-";
			break;
			case 2:
			StName = "GK-";
			break;
			case 3:
			StName = "RK-";
			break;
			case 4:
			StName = "LK-";
			break;
			case 5:
			StName = "VK-";
			break;
		}
		setName (npc, StName + rand(1,820));
	}
	
	
	public static void nameImperialPilot(obj_id npc) throws InterruptedException
	{
		setName (npc, "DS-"+ rand(1,820));
	}
	
	
	public static void nameScoutTrooper(obj_id npc) throws InterruptedException
	{
		int designation = rand (1,5);
		String StName = "TK-";
		switch (designation)
		{
			case 1:
			StName = "SX-";
			break;
			case 2:
			StName = "GX-";
			break;
			case 3:
			StName = "VX-";
			break;
			case 4:
			StName = "CX-";
			break;
			case 5:
			StName = "NX-";
			break;
		}
		setName (npc, StName + rand(1,820));
	}
	
	
	public static void nameDarkTrooper(obj_id npc) throws InterruptedException
	{
		int designation = rand (1,5);
		String StName = "DLX-";
		switch (designation)
		{
			case 1:
			StName = "JLB-";
			break;
			case 2:
			StName = "RAR-";
			break;
			case 3:
			StName = "KNP-";
			break;
			case 4:
			StName = "BCP-";
			break;
			case 5:
			StName = "RTZ-";
			break;
		}
		setName (npc, StName + rand(10,99));
	}
	
	
	public static void nameSwampTrooper(obj_id npc) throws InterruptedException
	{
		int designation = rand (1,5);
		String StName = "TK-";
		switch (designation)
		{
			case 1:
			StName = "GL-";
			break;
			case 2:
			StName = "TL-";
			break;
			case 3:
			StName = "RL-";
			break;
			case 4:
			StName = "NL-";
			break;
			case 5:
			StName = "CL-";
			break;
		}
		setName (npc, StName + rand(1,820));
	}
	
	
	public static boolean setFactionRecruiter(obj_id npc, String faction) throws InterruptedException
	{
		
		if (npc == null || npc == obj_id.NULL_ID)
		{
			return false;
		}
		
		if (faction == null)
		{
			return false;
		}
		
		setObjVar(npc, "faction_recruiter.faction", faction);
		attachScript(npc, "npc.faction_recruiter.faction_recruiter");
		return true;
		
	}
	
	
	public static void setUpFactionalEnemyAttribs(obj_id npc, String pvpFaction) throws InterruptedException
	{
		if (pvpFaction == null)
		{
			return;
		}
		
		if (pvpFaction.equals(""))
		{
			return;
		}
		
		setAttributeAttained( npc, attrib.NPC );
		setAttributeInterested( npc, attrib.NPC );
	}
	
	
	public static void cleanOffBioLink(obj_id weapon) throws InterruptedException
	{
		detachScript(weapon, BIOLINK_SCRIPT);
		removeObjVar(weapon, "biolink");
		return;
	}
}
