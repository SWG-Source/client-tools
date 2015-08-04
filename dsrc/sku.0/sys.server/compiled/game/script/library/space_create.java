package script.library;

import script.*;
import script.base_class.*;
import script.combat_engine.*;
import java.util.Arrays;
import java.util.Hashtable;
import java.util.Vector;
import script.base_script;

import script.vector;
import script.transform;
import script.library.hue;
import script.library.ship_ai;
import script.library.sui;


public class space_create extends script.base_script
{
	public space_create()
	{
	}
	public static final int MAX_SQUAD_SIZE = 10;
	
	public static final String SHIP_DATATABLE = "datatables/space_mobile/space_mobile.iff";
	public static final String SHIP_DIRECTORY = "object/ship/";
	
	
	public static obj_id createShip(String strShipType, transform locTransform) throws InterruptedException
	{
		return createShip( strShipType, locTransform, null );
	}
	
	
	public static obj_id createShip(String strShipType, transform locTransform, obj_id objCell) throws InterruptedException
	{
		return _createShip( strShipType, locTransform, objCell, false );
	}
	
	
	public static obj_id createShipHyperspace(String strShipType, transform locTransform) throws InterruptedException
	{
		return _createShip( strShipType, locTransform, null, true );
	}
	
	
	public static obj_id createShipHyperspace(String strShipType, transform locTransform, obj_id objCell) throws InterruptedException
	{
		return _createShip( strShipType, locTransform, objCell, true );
	}
	
	
	public static obj_id _createShip(String strShipType, transform locTransform, obj_id objCell, boolean hyperspace) throws InterruptedException
	{
		dictionary dctShipInfo = dataTableGetRow( SHIP_DATATABLE, strShipType );
		if (dctShipInfo == null)
		{
			
			return null;
		}
		String strShip = "object/ship/"+dctShipInfo.getString("strShip")+".iff";
		
		obj_id objShip = null;
		if (hyperspace)
		{
			objShip = createObjectHyperspace( strShip, locTransform, objCell );
		}
		else
		{
			objShip = createObject( strShip, locTransform, objCell );
		}
		if (!isIdValid(objShip))
		{
			
			return null;
		}
		
		setObjVar( objShip, "ship.shipName", strShipType );
		String strPilot = dctShipInfo.getString( "strPilot");
		setObjVar( objShip, "ship.pilotType", strPilot );
		ship_ai.unitSetPilotType( objShip, strPilot );
		setupAIFlightModeling(objShip, strPilot);
		int intXP = dctShipInfo.getInt( "intXP");
		setObjVar( objShip, "xp.intXP", intXP );
		
		String strShipt = dctShipInfo.getString("strShip");
		setShipTypeName( objShip, strShipt );
		
		if (strShipt.indexOf( "tier" ) > -1)
		{
			int index = strShipt.lastIndexOf( "_");
			if (index > -1)
			{
				String tier = strShipt.substring( index+1, strShipt.length() );
				setShipDifficulty( objShip, tier );
			}
		}
		
		String strFaction = dctShipInfo.getString("space_faction");
		setShipFaction(objShip, strFaction);
		
		int intMinCredits = dctShipInfo.getInt( "intMinCredits");
		int intMaxCredits = dctShipInfo.getInt( "intMaxCredits");
		setObjVar( objShip, "loot.intCredits", rand(intMinCredits, intMaxCredits) );
		
		int intRolls = dctShipInfo.getInt( "intRolls");
		float fltItemChance = dctShipInfo.getFloat( "fltItemChance");
		
		int intNumItems = 0;
		fltItemChance = fltItemChance * 100;
		
		for (int intI = 0; intI<intRolls; intI++)
		{
			testAbortScript();
			int intRoll = rand(1, 100);
			
			if (intRoll < fltItemChance)
			{
				
				intNumItems = intNumItems+1;
			}
		}
		setObjVar( objShip, "loot.intNumItems", intNumItems );
		String strLootLookup = dctShipInfo.getString( "strLootLookup");
		setObjVar( objShip, "loot.strLootTable", strLootLookup );
		
		String collectionLoot = dctShipInfo.getString("collectionLoot");
		if (!collectionLoot.equals("no_loot") && collectionLoot != null)
		{
			LOG( "space_collections", "ADDING COLLECTION DATA TO SHIP");
			int rollBaseChance = dctShipInfo.getInt("collectionRoll");
			setObjVar(objShip, "collection.collectionLoot", collectionLoot);
			setObjVar(objShip, "collection.rollBaseChance", rollBaseChance);
		}
		
		attachScript( objShip, "space.ai.space_ai");
		
		string_id strSpam = new string_id("space/ship_names", strShipType);
		setName( objShip, strSpam );
		
		int intColor0 = dctShipInfo.getInt( "index_color_1");
		int intColor1 = dctShipInfo.getInt( "index_color_2");
		int intTextureIndex = dctShipInfo.getInt("index_texture_1");
		
		if (intColor0 > -1)
		{
			hue.setColor( objShip, "/shared_owner/index_color_1", intColor0);
		}
		
		if (intColor1 > -1)
		{
			hue.setColor( objShip, "/shared_owner/index_color_2", intColor1);
		}
		
		if (intTextureIndex > -1)
		{
			hue.setRangedIntCustomVar( objShip, "/shared_owner/index_texture_1", intTextureIndex);
		}
		
		String cargo_type = dctShipInfo.getString( "cargo");
		if ((cargo_type != null) && !cargo_type.equals( "none" ) && !cargo_type.equals( "" ))
		{
			String[] cargo_types = dataTableGetStringColumn( "datatables/spacequest/inspect_cargo/"+cargo_type+".iff", "name");
			
			if (cargo_types != null && cargo_types.length > 0)
			{
				String cargo = null;
				
				if (cargo_types.length == 1)
				{
					cargo = cargo_types[0];
				}
				else
				{
					cargo = cargo_types[rand(0,cargo_types.length-1)];
				}
				
				setObjVar( objShip, "cargo", cargo );
				setObjVar( objShip, "cargo_type", cargo_type );
			}
		}
		String strCapitalShipType = dctShipInfo.getString("strCapitalShipType");
		
		if (!strCapitalShipType.equals(""))
		{
			setObjVar( objShip, "strCapitalShipType", strCapitalShipType );
			attachScript( objShip, "space.combat.combat_ship_capital");
		}
		String strInteriorChassisType = dctShipInfo.getString( "strInteriorChassisType");
		if (!strInteriorChassisType.equals(""))
		{
			
			setObjVar( objShip, "strInteriorChassisType", strInteriorChassisType );
			attachScript( objShip, "space.ship.ship_interior");
			dictionary dctParams = new dictionary();
			space_utils.notifyObject( objShip, "doInteriorBuildout", dctParams );
		}
		
		String strBoardingType = dctShipInfo.getString( "strBoardingType");
		if (!strBoardingType.equals(""))
		{
			
			setObjVar( objShip, "strBoardingType", strBoardingType );
			attachScript( objShip, "space.combat.combat_ship_boardable");
			
		}
		String strAppearanceType = dctShipInfo.getString("strAppearance");
		String[] strAppearances = dataTableGetStringColumnNoDefaults("datatables/space_mobile/space_appearances.iff", strAppearanceType);
		if ((strAppearances != null)&&(strAppearances.length > 0))
		{
			if (strAppearances.length == 1)
			{
				setObjVar(objShip, "convo.appearance", strAppearances[0]);
			}
			else
			{
				setObjVar(objShip, "convo.appearance", strAppearances[rand(0, strAppearances.length-1)]);
			}
		}
		String strDiction = dctShipInfo.getString("strTauntDiction");
		if (!strDiction.equals(""))
		{
			utils.setLocalVar(objShip, "strTauntDiction", strDiction);
		}
		String strConversation = dctShipInfo.getString("strConversation");
		if ((strConversation != null)&&(!strConversation.equals("")))
		{
			attachScript(objShip, strConversation);
		}
		
		float fltAttackTauntChance = dctShipInfo.getFloat("fltAttackTauntChance");
		utils.setLocalVar(objShip, "fltAttackTauntChance", fltAttackTauntChance);
		
		float fltDefendTauntChance = dctShipInfo.getFloat("fltDefendTauntChance");
		utils.setLocalVar(objShip, "fltDefendTauntChance", fltDefendTauntChance);
		
		float fltDieTauntChance = dctShipInfo.getFloat("fltDieTauntChance");
		utils.setLocalVar(objShip, "fltDieTauntChance", fltDieTauntChance);
		
		utils.setLocalVar(objShip, "intImperialFactionPoints", dctShipInfo.getInt("intImperialFactionPoints"));
		utils.setLocalVar(objShip, "intRebelFactionPoints", dctShipInfo.getInt("intRebelFactionPoints"));
		
		return objShip;
	}
	
	
	public static obj_id makeCreditChip(obj_id objContainer, int intCredits) throws InterruptedException
	{
		
		String TEMPLATE_NAME = "object/tangible/item/loot_credit_chip.iff";
		if (intCredits > 0)
		{
			
			obj_id objCreditChip = getCreditChip(objContainer);
			if (isIdValid(objCreditChip))
			{
				int intAmount = getIntObjVar(objCreditChip, "loot.intCredits");
				intAmount = intAmount + intCredits;
				setCount(objCreditChip, intAmount);
				setObjVar(objCreditChip, "loot.intCredits", intAmount);
				return objCreditChip;
			}
			else
			{
				
				if (isIdValid(objContainer))
				{
					
					obj_id objChip = space_utils.createObjectTrackItemCount(TEMPLATE_NAME, objContainer);
					if (isIdValid(objChip))
					{
						setCount(objChip, intCredits);
						setObjVar(objChip, "loot.intCredits", intCredits);
						
					}
					return objChip;
				}
			}
		}
		return null;
	}
	
	
	public static obj_id getCreditChip(obj_id objContainer) throws InterruptedException
	{
		
		if (!isIdValid(objContainer))
		{
			
			return null;
		}
		obj_id[] objContents = getContents(objContainer);
		if (objContents == null)
		{
			
			return null;
		}
		for (int intI= 0; intI< objContents.length; intI++)
		{
			testAbortScript();
			
			if (hasObjVar(objContents[intI], "loot.intCredits"))
			{
				return objContents[intI];
			}
		}
		return null;
	}
	
	
	public static Vector createSquad(obj_id objParent, String strSquad, transform trSpawnLocation, float fltDistance, obj_id objCell) throws InterruptedException
	{
		return _createSquad( objParent, strSquad, trSpawnLocation, fltDistance, objCell, false);
	}
	
	
	public static Vector createSquadHyperspace(obj_id objParent, String strSquad, transform trSpawnLocation, float fltDistance, obj_id objCell) throws InterruptedException
	{
		return _createSquad( objParent, strSquad, trSpawnLocation, fltDistance, objCell, true);
	}
	
	
	public static Vector _createSquad(obj_id objParent, String strSquad, transform trSpawnLocation, float fltDistance, obj_id objCell, boolean hyperspace) throws InterruptedException
	{
		dictionary dctSquadInfo = dataTableGetRow( "datatables/space_content/spawners/squads.iff", strSquad );
		if (strSquad == null)
		{
			
			return null;
		}
		Vector objMembers = new Vector();
		objMembers.setSize(0);
		
		for (int intI = 1; intI< MAX_SQUAD_SIZE; intI++)
		{
			testAbortScript();
			String strShipType = dctSquadInfo.getString("strShip"+intI);
			if (!strShipType.equals(""))
			{
				obj_id objShip = null;
				if (hasObjVar(objParent, "intLaunchFromDockingPoint"))
				{
					objShip = space_create.createShip(strShipType, getTransform_o2p(objParent), null);
					transform trTest = ship_ai.unitGetDockTransform(objParent, objShip);
					transform trFoo = getTransform_o2p(objShip).rotateTranslate_l2p(trTest);
					
					location locTest = space_utils.getLocationFromTransform(trFoo);
					
					setTransform_o2p(objShip, trFoo);
				}
				else
				{
					transform trTest = space_utils.getRandomPositionInSphere(trSpawnLocation, 0f, fltDistance, true);
					objShip = space_create._createShip( strShipType, trTest, objCell, hyperspace);
				}
				
				if (!isIdValid(objShip))
				{
					
				}
				else
				{
					objMembers = utils.addElement(objMembers, objShip);
					String strMemberScript = dctSquadInfo.getString("strMemberScript");
					if (!strMemberScript.equals(""))
					{
						attachScript(objShip, strMemberScript);
					}
					attachScript(objShip, "space.content_tools.squad_member");
				}
			}
			else
			{
				intI = MAX_SQUAD_SIZE +1;
			}
			
		}
		if (objMembers.size() > 0)
		{
			float fltDestroyPercentage = dctSquadInfo.getFloat("fltDestroyPercentage");
			
			int intSquadId = ship_ai.unitGetSquadId(((obj_id)(objMembers.get(0))));
			utils.setScriptVar(((obj_id)(objMembers.get(0))), "fltDestroyPercentage", fltDestroyPercentage);
			utils.setScriptVar(((obj_id)(objMembers.get(0))), "intStartPopulation", objMembers.size());
			for (int intI = 1; intI < objMembers.size(); intI++)
			{
				testAbortScript();
				
				utils.setScriptVar(((obj_id)(objMembers.get(intI))), "fltDestroyPercentage", fltDestroyPercentage);
				utils.setScriptVar(((obj_id)(objMembers.get(intI))), "intStartPopulation", objMembers.size());
				setObjVar(((obj_id)(objMembers.get(intI))), "intNoDump", 1);
				ship_ai.unitSetSquadId(((obj_id)(objMembers.get(intI))), intSquadId);
				if (!isIdValid(objParent))
				{
					setObjVar( ((obj_id)(objMembers.get(intI))), "objParent", objParent );
				}
			}
			
			ship_ai.squadSetFormation( intSquadId, dctSquadInfo.getInt("intFormationShape") );
			ship_ai.squadSetLeader( intSquadId, ((obj_id)(objMembers.get(0))) );
		}
		else
		{
			
			return null;
		}
		for (int intI = 0; intI < objMembers.size(); intI++)
		{
			testAbortScript();
			
		}
		
		return objMembers;
	}
	
	
	public static obj_id setupShipFromObjVars(obj_id objShip) throws InterruptedException
	{
		
		String strShipType = getStringObjVar(objShip, "ship.shipName");
		dictionary dctShipInfo = dataTableGetRow( space_create.SHIP_DATATABLE, strShipType );
		
		if (dctShipInfo == null)
		{
			
			destroyObject(objShip);
			return null;
		}
		
		setObjVar( objShip, "ship.shipName", strShipType );
		String strPilot = dctShipInfo.getString( "strPilot");
		if (hasObjVar(objShip, "ship.pilotType"))
		{
			strPilot = getStringObjVar(objShip, "ship.pilotType");
			if ((strPilot == null)||(strPilot.equals("")))
			{
				strPilot = "default";
			}
			
		}
		else
		{
			if ((strPilot == null)||(strPilot.equals("")))
			{
				strPilot = "default";
			}
			setObjVar( objShip, "ship.pilotType", strPilot );
			
		}
		
		if (strPilot.indexOf( "tier" ) > -1)
		{
			int index = strPilot.lastIndexOf( "_");
			if (index > -1)
			{
				String tier = strPilot.substring( index+1, strPilot.length() );
				setShipDifficulty( objShip, tier );
			}
		}
		setShipTypeName( objShip, dctShipInfo.getString("strShip") );
		String strFaction = "";
		if (!hasObjVar(objShip, "ship.space_faction"))
		{
			
			strFaction = dctShipInfo.getString("space_faction");
		}
		else
		{
			
			strFaction = getStringObjVar(objShip, "ship.space_faction");
		}
		
		setShipFaction(objShip, strFaction);
		
		string_id strSpam = new string_id("space/ship_names", strShipType);
		setName( objShip, strSpam );
		ship_ai.unitSetPilotType(objShip, strPilot );
		
		return objShip;
		
	}
	
	public static void setupAIFlightModeling(obj_id objShip, String strPilotType) throws InterruptedException
	{
		if ((strPilotType != null)&&(!strPilotType.equals("")))
		{
			dictionary dctShipInfo = dataTableGetRow("datatables/ship/ship_debug.iff",strPilotType);
			if (dctShipInfo == null)
			{
				
				return;
			}
			
			setShipEngineAccelerationRate(objShip, dctShipInfo.getFloat("engine_accel"));
			setShipEngineDecelerationRate(objShip, dctShipInfo.getFloat("engine_decel"));
			setShipEnginePitchAccelerationRateDegrees(objShip, dctShipInfo.getFloat("engine_pitch_accel"));
			setShipEngineYawAccelerationRateDegrees(objShip, dctShipInfo.getFloat("engine_yaw_accel"));
			setShipEngineRollAccelerationRateDegrees(objShip, dctShipInfo.getFloat("engine_roll_accel"));
			setShipEnginePitchRateMaximumDegrees(objShip, dctShipInfo.getFloat("engine_pitch"));
			setShipEngineYawRateMaximumDegrees(objShip, dctShipInfo.getFloat("engine_yaw"));
			setShipEngineRollRateMaximumDegrees(objShip, dctShipInfo.getFloat("engine_roll"));
			setShipEngineSpeedMaximum(objShip, dctShipInfo.getFloat("engine_speed"));
			setShipEngineSpeedRotationFactorMaximum(objShip, dctShipInfo.getFloat("speed_rotation_factor_max"));
			setShipEngineSpeedRotationFactorMinimum(objShip, dctShipInfo.getFloat("speed_rotation_factor_min"));
			setShipEngineSpeedRotationFactorOptimal(objShip, dctShipInfo.getFloat("speed_rotation_factor_optimal"));
			setShipSlideDampener(objShip, dctShipInfo.getFloat("slideDamp"));
			int intHoldFire = dctShipInfo.getInt("intHoldFire");
			if (intHoldFire > 0)
			{
				ship_ai.unitSetAttackOrders(objShip, ship_ai.ATTACK_ORDERS_HOLD_FIRE);
			}
			if (isShipSlotInstalled(objShip, space_crafting.BOOSTER))
			{
				
				setShipBoosterEnergyCurrent(objShip, dctShipInfo.getFloat("booster_energy"));
				setShipBoosterEnergyMaximum(objShip, dctShipInfo.getFloat("booster_energy"));
				setShipBoosterEnergyRechargeRate(objShip, dctShipInfo.getFloat("booster_recharge"));
				setShipBoosterEnergyConsumptionRate(objShip, dctShipInfo.getFloat("booster_consumption"));
				setShipBoosterAcceleration(objShip, dctShipInfo.getFloat("booster_accel"));
				setShipBoosterSpeedMaximum(objShip, dctShipInfo.getFloat("booster_speed"));
			}
		}
	}
}
