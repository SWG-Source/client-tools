package script.library;

import script.*;
import script.base_class.*;
import script.combat_engine.*;
import java.util.Arrays;
import java.util.Hashtable;
import java.util.Vector;
import script.base_script;

import script.library.beast_lib;
import script.library.incubator;
import script.library.spawning;
import script.library.static_item;
import script.library.target_dummy;
import script.library.utils;


public class tcg extends script.base_script
{
	public tcg()
	{
	}
	public static final String RANCHHAND_CELL_SCRIPT = "systems.tcg.barn_cell";
	
	public static final String RANCHHAND_PREFIX = "ranchhand";
	public static final String RANCHHAND_CELLCHECK = RANCHHAND_PREFIX + ".ranchhandcheck";
	public static final String RANCHHAND_CELL = RANCHHAND_PREFIX + ".ranchhandcell";
	public static final String RANCHHAND_STRUCTURE = RANCHHAND_PREFIX + ".structure";
	public static final String RANCHHAND_OBJECT = RANCHHAND_PREFIX + ".ranchhand_object";
	
	public static final String RANCH_HAND_TEMPLATE = "object/tangible/tcg/series1/structure_barn_ranchhand.iff";
	public static final String CELL_NAME = "entry";
	
	public static final float RANCH_HAND_X = 4.2f;
	public static final float RANCH_HAND_Y = 0.0f;
	public static final float RANCH_HAND_Z = 2.7f;
	public static final float RANCH_HAND_HEADING = 172.0f;
	
	public static final String BARN_STRUCTURE_TEMPLATE = "object/building/player/city/barn_no_planet_restriction.iff";
	public static final String BARN_RANCHHAND_SCRIPT = "systems.tcg.barn_ranchhand";
	public static final String BARN_BEAST_SCRIPT = "systems.tcg.barn_beast";
	
	public static final int MAX_NUM_BARN_PETS = 40;
	
	public static final String[] BEAST_OBJVARS =
	{
		"beast",
		"beastmood",
		"abilities"
	};
	
	
	public static boolean isBarnRanchhand(obj_id target) throws InterruptedException
	{
		if (hasScript(target, BARN_RANCHHAND_SCRIPT))
		{
			return true;
		}
		
		return false;
	}
	
	
	public static boolean isBarnStructure(obj_id building) throws InterruptedException
	{
		String template = getTemplateName(building);
		if (template.equals(BARN_STRUCTURE_TEMPLATE))
		{
			return true;
		}
		return false;
	}
	
	
	public static void barnStoreBeastPrompt(obj_id player, obj_id ranchhand) throws InterruptedException
	{
		obj_id[] initialBcdsList = beast_lib.getDatapadBCDs(player);
		if (initialBcdsList == null || initialBcdsList.length < 1)
		{
			sendSystemMessage(player, new string_id("tcg", "barn_no_beast_control_devices_found"));
		}
		else
		{
			Vector vectorBcdsList = new Vector();
			vectorBcdsList.setSize(0);
			Vector vectorBeastNames = new Vector();
			vectorBeastNames.setSize(0);
			for (int i = 0; i < initialBcdsList.length; i++)
			{
				testAbortScript();
				obj_id bcd = initialBcdsList[i];
				String beastName = beast_lib.getBCDBeastName(bcd);
				if (beastName != null && beastName.length() > 0)
				{
					utils.addElement(vectorBeastNames, toUpper(beastName, 0));
					utils.addElement(vectorBcdsList, bcd);
				}
			}
			
			if (vectorBeastNames != null && vectorBcdsList != null && vectorBeastNames.size() > 0 && vectorBcdsList.size() > 0)
			{
				obj_id[] finalBcdsList = utils.toStaticObjIdArray(vectorBcdsList);
				String[] finalBeastNames = utils.toStaticStringArray(vectorBeastNames);
				
				String promptExtra = "";
				obj_id barn = getTopMostContainer(ranchhand);
				if (isIdValid(barn))
				{
					promptExtra = target_dummy.addLineBreaks(2);
					promptExtra += "Number of Beasts in the Barn: "+getTotalBarnStoredBeasts(barn)+" of "+MAX_NUM_BARN_PETS;
				}
				
				barnSelectPlayerBeastSui(player, ranchhand, finalBcdsList, finalBeastNames, "barn_beast_selection_storage_prompt", promptExtra, "handleBarnBeastStorage");
			}
		}
		return;
	}
	
	
	public static void barnSelectPlayerBeastSui(obj_id player, obj_id ranchhand, obj_id[] beastControlDevices, String[] beastNames, String promptRef, String promptExtra, String handler) throws InterruptedException
	{
		
		if (utils.hasScriptVar(player, "barnRanchHand.pid"))
		{
			int oldPid = utils.getIntScriptVar(player, "barnRanchHand.pid");
			forceCloseSUIPage(oldPid);
			utils.removeScriptVar(player, "barnRanchHand.pid");
		}
		
		if (utils.hasScriptVar(player, "barnRanchHand."+handler))
		{
			utils.removeScriptVar(player, "barnRanchHand."+handler);
		}
		
		String[] alphabetizedBeastNames = new String[beastNames.length];
		obj_id[] sortedBeastControlDevices = new obj_id[beastControlDevices.length];
		
		utils.copyArray(beastNames, alphabetizedBeastNames);
		Arrays.sort(alphabetizedBeastNames);
		for (int j = 0; j < alphabetizedBeastNames.length; j++)
		{
			testAbortScript();
			String alphabetizedName = alphabetizedBeastNames[j];
			int index = utils.getElementPositionInArray(beastNames, alphabetizedName);
			obj_id sortedBeast = beastControlDevices[index];
			
			sortedBeastControlDevices[j] = sortedBeast;
			beastNames[index] = "_";
		}
		
		String title = utils.packStringId(new string_id("tcg", "barn_beast_selection_title"));
		String prompt = utils.packStringId(new string_id("tcg", promptRef));
		if (promptExtra != null && promptExtra.length() > 0)
		{
			prompt += promptExtra;
		}
		
		int pid = sui.listbox(ranchhand, player, prompt, sui.OK_CANCEL, title, alphabetizedBeastNames, handler);
		
		if (pid > -1)
		{
			utils.setScriptVar(player, "barnRanchHand.pid", pid);
			utils.setScriptVar(player, "barnRanchHand."+handler, sortedBeastControlDevices);
		}
		return;
	}
	
	
	public static void barnReclaimBeastPrompt(obj_id player, obj_id barn, obj_id ranchhand) throws InterruptedException
	{
		String[] initialStorageList = getBarnBeastStorageSlots(barn);
		if (initialStorageList == null || initialStorageList.length < 1)
		{
			sendSystemMessage(player, new string_id("tcg", "barn_no_beast_control_devices_found"));
		}
		else
		{
			Vector vectorBcdsList = new Vector();
			vectorBcdsList.setSize(0);
			Vector vectorBeastNames = new Vector();
			vectorBeastNames.setSize(0);
			for (int i = 0; i < initialStorageList.length; i++)
			{
				testAbortScript();
				String bcdSlot = initialStorageList[i];
				String beastName = "";
				if (hasObjVar(barn, "barnStorage."+bcdSlot+".beast.beastName"))
				{
					beastName =getStringObjVar(barn, "barnStorage."+bcdSlot+".beast.beastName");
				}
				else if (hasObjVar(barn, "barnStorage."+bcdSlot+".tempName"))
				{
					beastName = getStringObjVar(barn, "barnStorage."+bcdSlot+".tempName");
				}
				if (beastName != null && beastName.length() > 0)
				{
					utils.addElement(vectorBeastNames, toUpper(beastName, 0));
					utils.addElement(vectorBcdsList, bcdSlot);
				}
			}
			
			if (vectorBeastNames != null && vectorBcdsList != null && vectorBeastNames.size() > 0 && vectorBcdsList.size() > 0)
			{
				String[] finalBcdsList = utils.toStaticStringArray(vectorBcdsList);
				String[] finalBeastNames = utils.toStaticStringArray(vectorBeastNames);
				
				String promptExtra = target_dummy.addLineBreaks(2);
				promptExtra += "Number of Beasts in the Barn: "+getTotalBarnStoredBeasts(barn)+" of "+MAX_NUM_BARN_PETS;
				
				barnSelectStoredBeastSui(player, ranchhand, finalBcdsList, finalBeastNames, "barn_beast_selection_reclaim_prompt", promptExtra, "handleBarnBeastReclaim");
			}
		}
		return;
	}
	
	
	public static void barnDisplayBeastPrompt(obj_id player, obj_id barn, obj_id ranchhand) throws InterruptedException
	{
		String[] initialStorageList = getBarnBeastStorageSlots(barn);
		if (initialStorageList == null || initialStorageList.length < 1)
		{
			sendSystemMessage(player, new string_id("tcg", "barn_no_beast_control_devices_found"));
		}
		else
		{
			Vector vectorBcdsList = new Vector();
			vectorBcdsList.setSize(0);
			Vector vectorBeastNames = new Vector();
			vectorBeastNames.setSize(0);
			for (int i = 0; i < initialStorageList.length; i++)
			{
				testAbortScript();
				String bcdSlot = initialStorageList[i];
				String beastName = "";
				if (hasObjVar(barn, "barnStorage."+bcdSlot+".beast.beastName"))
				{
					beastName =getStringObjVar(barn, "barnStorage."+bcdSlot+".beast.beastName");
				}
				else if (hasObjVar(barn, "barnStorage."+bcdSlot+".tempName"))
				{
					beastName = getStringObjVar(barn, "barnStorage."+bcdSlot+".tempName");
				}
				
				if (utils.hasScriptVar(barn, bcdSlot))
				{
					beastName = beastName + " [Displayed]";
				}
				else
				{
					beastName = beastName + " [Stored]";
				}
				
				if (beastName != null && beastName.length() > 0)
				{
					utils.addElement(vectorBeastNames, toUpper(beastName, 0));
					utils.addElement(vectorBcdsList, bcdSlot);
				}
			}
			
			if (vectorBeastNames != null && vectorBcdsList != null && vectorBeastNames.size() > 0 && vectorBcdsList.size() > 0)
			{
				String[] finalBcdsList = utils.toStaticStringArray(vectorBcdsList);
				String[] finalBeastNames = utils.toStaticStringArray(vectorBeastNames);
				barnSelectStoredBeastSui(player, ranchhand, finalBcdsList, finalBeastNames, "barn_beast_selection_display_prompt", "", "handleBarnBeastDisplay");
			}
		}
		return;
	}
	
	
	public static void barnSelectStoredBeastSui(obj_id player, obj_id ranchhand, String[] beastControlSlots, String[] beastNames, String promptRef, String promptExtra, String handler) throws InterruptedException
	{
		
		if (utils.hasScriptVar(player, "barnRanchHand.pid"))
		{
			int oldPid = utils.getIntScriptVar(player, "barnRanchHand.pid");
			forceCloseSUIPage(oldPid);
			utils.removeScriptVar(player, "barnRanchHand.pid");
		}
		
		if (utils.hasScriptVar(player, "barnRanchHand."+handler))
		{
			utils.removeScriptVar(player, "barnRanchHand."+handler);
		}
		
		String[] alphabetizedBeastNames = new String[beastNames.length];
		String[] sortedBeastControlSlots = new String[beastControlSlots.length];
		
		utils.copyArray(beastNames, alphabetizedBeastNames);
		Arrays.sort(alphabetizedBeastNames);
		for (int j = 0; j < alphabetizedBeastNames.length; j++)
		{
			testAbortScript();
			String alphabetizedName = alphabetizedBeastNames[j];
			int index = utils.getElementPositionInArray(beastNames, alphabetizedName);
			String sortedBeast = beastControlSlots[index];
			
			sortedBeastControlSlots[j] = sortedBeast;
			beastNames[index] = "_";
		}
		
		String title = utils.packStringId(new string_id("tcg", "barn_beast_selection_title"));
		String prompt = utils.packStringId(new string_id("tcg", promptRef));
		if (promptExtra != null && promptExtra.length() > 0)
		{
			prompt += promptExtra;
		}
		
		int pid = sui.listbox(ranchhand, player, prompt, sui.OK_CANCEL, title, alphabetizedBeastNames, handler);
		
		if (pid > -1)
		{
			utils.setScriptVar(player, "barnRanchHand.pid", pid);
			utils.setScriptVar(player, "barnRanchHand."+handler, sortedBeastControlSlots);
		}
		return;
	}
	
	
	public static boolean transferBeastControlDeviceToBarn(obj_id selectedBeastControlDevice, obj_id player, obj_id barn) throws InterruptedException
	{
		String nextAvailableStorageSlot = getNextAvailableBarnStorageSlot(barn, player);
		if (nextAvailableStorageSlot != null && nextAvailableStorageSlot.length() > 0)
		{
			String bcdTemplate = getTemplateName(selectedBeastControlDevice);
			if (bcdTemplate != null && bcdTemplate.length() > 0)
			{
				setObjVar(barn, nextAvailableStorageSlot+".bcdTemplate", bcdTemplate);
				if (!hasObjVar(selectedBeastControlDevice, "beast.beastName"))
				{
					setObjVar(barn, nextAvailableStorageSlot+".tempName", beast_lib.getBeastLocalizedName(selectedBeastControlDevice));
				}
				
				for (int i = 0; i < BEAST_OBJVARS.length; i++)
				{
					testAbortScript();
					String beastObjvar = BEAST_OBJVARS[i];
					if (hasObjVar(selectedBeastControlDevice, beastObjvar))
					{
						obj_var_list beastData = getObjVarList(selectedBeastControlDevice, beastObjvar);
						if (beastData != null)
						{
							setBCDDataObjVars(player, barn, nextAvailableStorageSlot+"."+ beastObjvar +".", beastData);
						}
						else
						{
							if (hasObjVar(barn, nextAvailableStorageSlot))
							{
								removeObjVar(barn, nextAvailableStorageSlot);
							}
							return false;
						}
					}
				}
			}
			else
			{
				if (hasObjVar(barn, nextAvailableStorageSlot))
				{
					removeObjVar(barn, nextAvailableStorageSlot);
				}
				return false;
			}
		}
		else
		{
			if (hasObjVar(barn, nextAvailableStorageSlot))
			{
				removeObjVar(barn, nextAvailableStorageSlot);
			}
			return false;
		}
		
		beast_lib.storeBeast(selectedBeastControlDevice);
		destroyObject(selectedBeastControlDevice);
		return true;
	}
	
	
	public static obj_id transferBeastControlDeviceFromBarn(String selectedStorageSlot, obj_id player, obj_id barn) throws InterruptedException
	{
		obj_id bcd = obj_id.NULL_ID;
		
		if (callable.hasMaxStoredCombatPets(player))
		{
			sendSystemMessage(player, new string_id("tcg", "barn_beast_reclaim_player_datapad_full"));
			return obj_id.NULL_ID;
		}
		
		obj_id datapad = utils.getPlayerDatapad(player);
		if (isIdValid(datapad))
		{
			String bcdTemplate = getStringObjVar(barn, "barnStorage."+selectedStorageSlot+".bcdTemplate");
			if (bcdTemplate != null && bcdTemplate.length() > 0)
			{
				bcd = createObject(bcdTemplate, datapad, "");
				if (isIdValid(bcd))
				{
					for (int i = 0; i < BEAST_OBJVARS.length; i++)
					{
						testAbortScript();
						String beastObjvar = BEAST_OBJVARS[i];
						if (hasObjVar(barn, "barnStorage."+selectedStorageSlot+"."+beastObjvar))
						{
							obj_var_list beastData = getObjVarList(barn, "barnStorage."+selectedStorageSlot+"."+beastObjvar);
							if (beastData != null)
							{
								setBCDDataObjVars(player, bcd, beastObjvar+".", beastData);
							}
							else
							{
								
								destroyObject(bcd);
								return obj_id.NULL_ID;
							}
						}
					}
					
					attachScript(bcd, "ai.beast_control_device");
					setName(bcd, beast_lib.getBCDBeastName(bcd));
					
					removeObjVar(barn, "barnStorage."+selectedStorageSlot);
					
					if (utils.hasScriptVar(barn, selectedStorageSlot))
					{
						obj_id displayedBeast = utils.getObjIdScriptVar(barn, selectedStorageSlot);
						if (isIdValid(displayedBeast) && exists(displayedBeast))
						{
							destroyObject(displayedBeast);
							utils.removeScriptVar(barn, selectedStorageSlot);
						}
					}
				}
				else
				{
					
				}
			}
			else
			{
				
			}
		}
		else
		{
			
		}
		
		return bcd;
	}
	
	
	public static void setBCDDataObjVars(obj_id player, obj_id target, String objvarNamePrefix, obj_var_list data) throws InterruptedException
	{
		int numItem = data.getNumItems();
		for (int i = 0; i < numItem; i++)
		{
			testAbortScript();
			obj_var dataObjvar = data.getObjVar(i);
			String dataObjvarName = dataObjvar.getName();
			
			if (data.isIntObjVar(dataObjvarName))
			{
				int intData = data.getIntObjVar(dataObjvarName);
				setObjVar(target, objvarNamePrefix + dataObjvarName, intData);
			}
			else if (data.isIntArrayObjVar(dataObjvarName))
			{
				int[] intArrayData = data.getIntArrayObjVar(dataObjvarName);
				setObjVar(target, objvarNamePrefix + dataObjvarName, intArrayData);
			}
			else if (data.isFloatObjVar(dataObjvarName))
			{
				float floatData = data.getFloatObjVar(dataObjvarName);
				setObjVar(target, objvarNamePrefix + dataObjvarName, floatData);
			}
			else if (data.isFloatArrayObjVar(dataObjvarName))
			{
				float[] floatArrayData = data.getFloatArrayObjVar(dataObjvarName);
				setObjVar(target, objvarNamePrefix + dataObjvarName, floatArrayData);
			}
			else if (data.isStringObjVar(dataObjvarName))
			{
				String stringData = data.getStringObjVar(dataObjvarName);
				setObjVar(target, objvarNamePrefix + dataObjvarName, stringData);
			}
			else if (data.isStringArrayObjVar(dataObjvarName))
			{
				String[] stringArrayData = data.getStringArrayObjVar(dataObjvarName);
				setObjVar(target, objvarNamePrefix + dataObjvarName, stringArrayData);
			}
			else if (data.isObjIdObjVar(dataObjvarName))
			{
				obj_id objIdData = data.getObjIdObjVar(dataObjvarName);
				setObjVar(target, objvarNamePrefix + dataObjvarName, objIdData);
			}
			else if (data.isObjIdArrayObjVar(dataObjvarName))
			{
				obj_id[] objIdArrayData = data.getObjIdArrayObjVar(dataObjvarName);
				setObjVar(target, objvarNamePrefix + dataObjvarName, objIdArrayData);
			}
			else if (data.isBooleanObjVar(dataObjvarName))
			{
				boolean booleanData = data.getBooleanObjVar(dataObjvarName);
				setObjVar(target, objvarNamePrefix + dataObjvarName, booleanData);
			}
			else
			{
				if (isGod(player))
				{
					sendSystemMessage(player, "God Mode Message::Unknown data type for "+dataObjvarName+".", "");
				}
			}
		}
		return;
	}
	
	
	public static String getNextAvailableBarnStorageSlot(obj_id barn, obj_id player) throws InterruptedException
	{
		String storageSlot = "";
		
		if (hasObjVar(barn, "barnStorage"))
		{
			obj_var_list barnStorageData = getObjVarList(barn, "barnStorage");
			if (barnStorageData != null)
			{
				int numItem = barnStorageData.getNumItems();
				if (numItem < MAX_NUM_BARN_PETS)
				{
					for (int i = 0; i < MAX_NUM_BARN_PETS; i++)
					{
						testAbortScript();
						String testName = "slot_0"+i;
						if (i > 9)
						{
							testName = "slot_"+i;
						}
						if (!barnStorageData.hasObjVar(testName))
						{
							storageSlot = "barnStorage."+testName;
							break;
						}
					}
				}
				else
				{
					sendSystemMessage(player, new string_id ("tcg","barn_is_full"));
				}
			}
		}
		else
		{
			storageSlot = "barnStorage.slot_00";
		}
		
		return storageSlot;
	}
	
	
	public static String[] getBarnBeastStorageSlots(obj_id barn) throws InterruptedException
	{
		obj_var_list beastData = getObjVarList(barn, "barnStorage");
		if (beastData != null)
		{
			int numItems = beastData.getNumItems();
			String[] slots = new String[numItems];
			for (int i = 0; i < numItems; i++)
			{
				testAbortScript();
				obj_var beastDataObjvar = beastData.getObjVar(i);
				String beastDataObjvarName = beastDataObjvar.getName();
				slots[i] = beastDataObjvarName;
			}
			
			return slots;
		}
		
		return null;
	}
	
	
	public static int getTotalBarnStoredBeastsFromRanchhand(obj_id ranchhand) throws InterruptedException
	{
		int numStoredBeasts = 0;
		
		if (isIdValid(ranchhand) && exists(ranchhand))
		{
			obj_id barn = getTopMostContainer(ranchhand);
			if (isIdValid(barn))
			{
				numStoredBeasts = getTotalBarnStoredBeasts(barn);
			}
		}
		
		return numStoredBeasts;
	}
	
	
	public static int getTotalBarnStoredBeasts(obj_id barn) throws InterruptedException
	{
		int numStoredBeasts = 0;
		
		if (isIdValid(barn) && exists(barn))
		{
			obj_var_list beastStorageData = getObjVarList(barn, "barnStorage");
			if (beastStorageData != null)
			{
				numStoredBeasts = beastStorageData.getNumItems();
			}
		}
		
		return numStoredBeasts;
	}
	
	
	public static obj_id barnDisplayBeast(obj_id ranchhand, String storageSlot, obj_id barn) throws InterruptedException
	{
		location here = getLocation(ranchhand);
		location spawnCenter = new location(-7.0f, 0.0f, -2.4f, here.area, here.cell);
		location spawnLocation = spawning.getRandomLocationInCircle(spawnCenter, 5.0f);
		
		String beastType = "";
		if (hasObjVar(barn, "barnStorage."+storageSlot+"."+beast_lib.OBJVAR_BEAST_TYPE))
		{
			int hashBeastType = getIntObjVar(barn, "barnStorage."+storageSlot+"."+beast_lib.OBJVAR_BEAST_TYPE);
			beastType = incubator.convertHashTypeToString(hashBeastType);
		}
		if (beastType == null || beastType.length() < 1)
		{
			return obj_id.NULL_ID;
		}
		
		dictionary beastDict = utils.dataTableGetRow(beast_lib.BEASTS_TABLE, beastType);
		if (beastDict == null)
		{
			return obj_id.NULL_ID;
		}
		
		String mobileTemplateName = beastDict.getString("template");
		if (mobileTemplateName.equals("") || mobileTemplateName == null)
		{
			return obj_id.NULL_ID;
		}
		
		obj_id beast = createObject(beast_lib.MOBILE_TEMPLATE_PREFIX + mobileTemplateName, spawnLocation);
		if (!isIdValid(beast))
		{
			return obj_id.NULL_ID;
		}
		
		LOG("barnStorage","getting hues for beast: "+ beast);
		
		int primaryHue = -1;
		if (hasObjVar(barn, "barnStorage."+storageSlot+"."+beast_lib.OBJVAR_BEAST_HUE))
		{
			primaryHue = getIntObjVar(barn, "barnStorage."+storageSlot+"."+beast_lib.OBJVAR_BEAST_HUE);
			LOG("barnStorage","primaryHue: "+ primaryHue);
		}
		int secondaryHue = -1;
		if (hasObjVar(barn, "barnStorage."+storageSlot+"."+beast_lib.OBJVAR_BEAST_HUE2))
		{
			secondaryHue = getIntObjVar(barn, "barnStorage."+storageSlot+"."+beast_lib.OBJVAR_BEAST_HUE2);
			LOG("barnStorage","secondaryHue: "+ secondaryHue);
			
		}
		int thirdHue = -1;
		if (hasObjVar(barn, "barnStorage."+storageSlot+"."+beast_lib.OBJVAR_BEAST_HUE3))
		{
			thirdHue = getIntObjVar(barn, "barnStorage."+storageSlot+"."+beast_lib.OBJVAR_BEAST_HUE3);
			LOG("barnStorage","thirdHue: "+ secondaryHue);
		}
		
		String primaryPalette = "";
		if (hasObjVar(barn, "barnStorage."+storageSlot+"."+beast_lib.OBJVAR_BEAST_PALETTE))
		{
			primaryPalette = getStringObjVar(barn, "barnStorage."+storageSlot+"."+beast_lib.OBJVAR_BEAST_PALETTE);
			LOG("barnStorage","primaryPalette: "+ primaryPalette);
			
		}
		String secondaryPalette = "";
		if (hasObjVar(barn, "barnStorage."+storageSlot+"."+beast_lib.OBJVAR_BEAST_PALETTE2))
		{
			secondaryPalette = getStringObjVar(barn, "barnStorage."+storageSlot+"."+beast_lib.OBJVAR_BEAST_PALETTE2);
			LOG("barnStorage","secondaryPalette: "+ secondaryPalette);
		}
		String thirdPalette = "";
		if (hasObjVar(barn, "barnStorage."+storageSlot+"."+beast_lib.OBJVAR_BEAST_PALETTE3))
		{
			thirdPalette = getStringObjVar(barn, "barnStorage."+storageSlot+"."+beast_lib.OBJVAR_BEAST_PALETTE3);
			LOG("barnStorage","thirdPalette: "+ thirdPalette);
		}
		
		if (primaryHue >= 0 && (primaryPalette == null || primaryPalette.length() <= 0))
		{
			LOG("barnStorage","setting primary hue only!! "+ primaryHue);
			setRoamingBeastHue(beast, "/private/index_color_1", primaryHue);
		}
		else
		{
			LOG("barnStorage","setting all available hues!");
			
			if (primaryPalette != null || primaryPalette.length() > 0)
			{
				LOG("barnStorage","setting "+ primaryPalette + " to "+primaryHue);
				setRoamingBeastHue(beast, primaryPalette, primaryHue);
			}
			if (secondaryPalette != null || secondaryPalette.length() > 0)
			{
				LOG("barnStorage","setting "+ secondaryPalette + " to "+secondaryHue);
				setRoamingBeastHue(beast, secondaryPalette, secondaryHue);
			}
			if (thirdPalette != null || thirdPalette.length() > 0)
			{
				LOG("barnStorage","setting "+ thirdPalette + " to "+thirdHue);
				setRoamingBeastHue(beast, thirdPalette, thirdHue);
			}
		}
		
		utils.setScriptVar(barn, storageSlot, beast);
		
		utils.setScriptVar(beast, "barnSlot", storageSlot);
		utils.setScriptVar(beast, "barnId", barn);
		
		beast_lib.setBeastBCD(beast, barn);
		
		utils.setScriptVar(beast, "beast.creationInProgress", true);
		
		initializeBeastStatsFromBarn(barn, beast, storageSlot, beastType);
		
		setYaw(beast, rand(0.0f, 360.0f));
		
		utils.removeScriptVar(beast, "beast.creationInProgress");
		
		setAttributeAttained(beast, attrib.BEAST);
		
		attachScript(beast, BARN_BEAST_SCRIPT);
		attachScript(beast, "ai.ai");
		
		setInvulnerable(beast, true);
		
		return beast;
	}
	
	
	public static void initializeBeastStatsFromBarn(obj_id barn, obj_id beast, String storageSlot, String beastType) throws InterruptedException
	{
		int primaryHue = -1;
		if (hasObjVar(barn, "barnStorage."+storageSlot+"."+beast_lib.OBJVAR_BEAST_HUE))
		{
			primaryHue = getIntObjVar(barn, "barnStorage."+storageSlot+"."+beast_lib.OBJVAR_BEAST_HUE);
		}
		int secondaryHue = -1;
		if (hasObjVar(barn, "barnStorage."+storageSlot+"."+beast_lib.OBJVAR_BEAST_HUE2))
		{
			secondaryHue = getIntObjVar(barn, "barnStorage."+storageSlot+"."+beast_lib.OBJVAR_BEAST_HUE2);
		}
		int thirdHue = -1;
		if (hasObjVar(barn, "barnStorage."+storageSlot+"."+beast_lib.OBJVAR_BEAST_HUE3))
		{
			thirdHue = getIntObjVar(barn, "barnStorage."+storageSlot+"."+beast_lib.OBJVAR_BEAST_HUE3);
		}
		
		String primaryPalette = "";
		if (hasObjVar(barn, "barnStorage."+storageSlot+"."+beast_lib.OBJVAR_BEAST_PALETTE))
		{
			primaryPalette = getStringObjVar(barn, "barnStorage."+storageSlot+"."+beast_lib.OBJVAR_BEAST_PALETTE);
		}
		String secondaryPalette = "";
		if (hasObjVar(barn, "barnStorage."+storageSlot+"."+beast_lib.OBJVAR_BEAST_PALETTE2))
		{
			secondaryPalette = getStringObjVar(barn, "barnStorage."+storageSlot+"."+beast_lib.OBJVAR_BEAST_PALETTE2);
		}
		String thirdPalette = "";
		if (hasObjVar(barn, "barnStorage."+storageSlot+"."+beast_lib.OBJVAR_BEAST_PALETTE3))
		{
			thirdPalette = getStringObjVar(barn, "barnStorage."+storageSlot+"."+beast_lib.OBJVAR_BEAST_PALETTE3);
		}
		
		if (primaryHue >= 0 && (primaryPalette == null || primaryPalette.length() <= 0))
		{
			beast_lib.setBeastHuePrimary(beast, "/private/index_color_1", primaryHue);
		}
		else
		{
			beast_lib.setBeastHuePrimary(beast, primaryPalette, primaryHue);
			beast_lib.setBeastHueSecondary(beast, secondaryPalette, secondaryHue);
			beast_lib.setBeastHueThird(beast, thirdPalette, thirdHue);
		}
		
		int level = 1;
		
		if (hasObjVar(barn, "barnStorage."+storageSlot+".beast.level"))
		{
			level = getIntObjVar(barn, "barnStorage."+storageSlot+".beast.level");
		}
		setLevel(beast, level);
		
		String beastName = beast_lib.getBeastLocalizedName(beast);
		if (hasObjVar(barn, "barnStorage."+storageSlot+".beast.beastName"))
		{
			beastName = getStringObjVar(barn, "barnStorage."+storageSlot+".beast.beastName");
		}
		else if (hasObjVar(barn, "barnStorage."+storageSlot+".tempName"))
		{
			beastName = getStringObjVar(barn, "barnStorage."+storageSlot+".tempName");
		}
		setName(beast, beastName);
		
		dictionary beastDict = utils.dataTableGetRow(beast_lib.BEASTS_TABLE, beastType);
		if (beastDict == null || beastDict.size() <= 0)
		{
			return;
		}
		
		dictionary beastStatsDict = utils.dataTableGetRow(beast_lib.BEASTS_STATS, level - 1);
		if (beastStatsDict == null || beastStatsDict.size() <= 0)
		{
			return;
		}
		
		int[] incubationBonuses = new int[beast_lib.ARRAY_BEAST_INCUBATION_STATS.length];
		
		if (hasObjVar(barn, "barnStorage."+storageSlot+"."+beast_lib.OBJVAR_BEAST_INCUBATION_BONUSES))
		{
			incubationBonuses = getIntArrayObjVar(barn, "barnStorage."+storageSlot+"."+beast_lib.OBJVAR_BEAST_INCUBATION_BONUSES);
		}
		else
		{
			for (int i = 0; i < beast_lib.ARRAY_BEAST_INCUBATION_STATS.length; ++i)
			{
				testAbortScript();
				incubationBonuses[i] = 0;
			}
		}
		
		for (int i = 0; i < beast_lib.ARRAY_BEAST_INCUBATION_STATS.length; ++i)
		{
			testAbortScript();
			utils.setScriptVar(beast, beast_lib.ARRAY_BEAST_INCUBATION_STATS[i], (float)incubationBonuses[i]);
		}
		
		beast_lib.addInnateBeastBonuses(beast, beastType);
		
		beast_lib.setBeastSkillBonuses(beast);
		
		setScale(beast, beast_lib.getBeastScaleByLevel(beast));
		
		addSkillModModifier(beast, "slope_move", "slope_move", 50, -1, false, false);
		
		int expertiseRegen = getEnhancedSkillStatisticModifierUncapped(getMaster(beast), "expertise_bm_pet_regen");
		int expertiseHealth = getEnhancedSkillStatisticModifierUncapped(getMaster(beast), "expertise_bm_pet_health");
		int expertiseAttackSpeed = getEnhancedSkillStatisticModifierUncapped(getMaster(beast), "expertise_bm_pet_attack_speed");
		int expertiseArmor = getEnhancedSkillStatisticModifierUncapped(getMaster(beast), "expertise_bm_pet_armor");
		int expertiseDamage = getEnhancedSkillStatisticModifierUncapped(getMaster(beast), "expertise_bm_pet_damage");
		
		float incubationArmorBonus = utils.getFloatScriptVar(beast, beast_lib.OBJVAR_INCREASE_ARMOR);
		float incubationDamageBonus = utils.getFloatScriptVar(beast, beast_lib.OBJVAR_INCREASE_DPS);
		float incubationHealthBonus = utils.getFloatScriptVar(beast, beast_lib.OBJVAR_INCREASE_HEALTH);
		
		int intMinDamage = (int)beastStatsDict.getInt("MinDmg");
		int intMaxDamage = (int)beastStatsDict.getInt("MaxDmg");
		intMinDamage = beast_lib.getExpertiseStat(intMinDamage, expertiseDamage, .5f);
		intMaxDamage = beast_lib.getExpertiseStat(intMaxDamage, expertiseDamage, .5f);
		
		float floatMinDamage = (float)intMinDamage * (1.0f + incubationDamageBonus / 100.0f);
		float floatMaxDamage = (float)intMaxDamage * (1.0f + incubationDamageBonus / 100.0f);
		intMinDamage = (int)floatMinDamage;
		intMaxDamage = (int)floatMaxDamage;
		
		int specialDamagePercent = beast_lib.getExpertiseStat(100, expertiseDamage, .5f) - 100;
		
		if (!hasSkillModModifier (beast, "expertise_damage_line_beast_only"))
		{
			addSkillModModifier(beast, "expertise_damage_line_beast_only", "expertise_damage_line_beast_only", specialDamagePercent, -1, false, false);
		}
		
		float primarySpeed = beast_lib.getExpertiseSpeed(beast_lib.BEAST_WEAPON_SPEED, expertiseAttackSpeed);
		
		obj_id beastWeapon = getCurrentWeapon(beast);
		
		if (isIdValid(beastWeapon))
		{
			setWeaponAttackSpeed(beastWeapon, primarySpeed);
			setWeaponMaxDamage(beastWeapon, intMaxDamage);
			setWeaponMinDamage(beastWeapon, intMinDamage);
			weapons.setWeaponData(beastWeapon);
			
			utils.setScriptVar(beastWeapon, "isCreatureWeapon", 1);
		}
		
		obj_id defaultWeapon = getDefaultWeapon(beast);
		
		if (isIdValid(defaultWeapon))
		{
			setWeaponAttackSpeed(defaultWeapon, primarySpeed);
			setWeaponMaxDamage(defaultWeapon, intMaxDamage);
			setWeaponMinDamage(defaultWeapon, intMinDamage);
			weapons.setWeaponData(defaultWeapon);
			
			utils.setScriptVar(defaultWeapon, "isCreatureWeapon", 1);
		}
		
		int beastHealth = (int)(beast_lib.getExpertiseStat(beastStatsDict.getInt("HP"), expertiseHealth, .5f) * (1.0f + (incubationHealthBonus * 0.2f) / 100.0f));
		
		int constitutionBonus = getEnhancedSkillStatisticModifierUncapped(beast, "constitution_modified");
		int staminaBonus = getEnhancedSkillStatisticModifierUncapped(beast, "stamina_modified");
		
		beastHealth += (constitutionBonus * 8) + (staminaBonus * 2);
		
		setMaxAttrib(beast, HEALTH, beastHealth);
		
		beastHealth = 0;
		if (hasObjVar(barn, "barnStorage."+storageSlot+".beast.health"))
		{
			beastHealth = getIntObjVar(barn, "barnStorage."+storageSlot+".beast.health");
		}
		
		if (beastHealth <= 0)
		{
			setAttrib(beast, HEALTH, getMaxAttrib(beast, HEALTH));
		}
		else
		{
			if (utils.hasScriptVar(beast, "beast.creationInProgress"))
			{
				setAttrib(beast, HEALTH, beastHealth);
				setAttrib(beast, ACTION, 100);
			}
			
			boolean beastIsDead = false;
			if (hasObjVar(barn, "barnStorage."+storageSlot+".beast.isDead"))
			{
				beastIsDead = getBooleanObjVar(barn, "barnStorage."+storageSlot+".beast.isDead");
			}
			
			if (beastIsDead)
			{
				kill(beast);
			}
		}
		
		setMaxAttrib(beast, ACTION, 100);
		
		if (!combat.isInCombat(beast))
		{
			int healthRegen = beast_lib.getExpertiseStat(beastStatsDict.getInt("HealthRegen"), expertiseRegen, .5f);
			int actionRegen = beast_lib.getExpertiseStat(beastStatsDict.getInt("ActionRegen"), expertiseRegen, .5f);
			
			setRegenRate(beast, HEALTH, healthRegen);
			setRegenRate(beast, ACTION, actionRegen);
		}
		
		float runSpeed = 12;
		setBaseRunSpeed(beast, runSpeed);
		if (getBaseWalkSpeed(beast) > runSpeed)
		{
			setBaseWalkSpeed(beast, runSpeed);
		}
		
		armor.removeAllArmorData(beast);
		
		int intArmor = (int)(beast_lib.getExpertiseStat(beastStatsDict.getInt("Armor"), expertiseArmor, .5f) * (1.0f + incubationArmorBonus / 100.0f));
		
		intArmor += (int)getSkillStatisticModifier(beast, "expertise_innate_protection_all");
		
		utils.setScriptVar(beast, "beast.display.armor", intArmor);
		
		if (intArmor >= 0)
		{
			
			String armorCategoryObjVar = armor.OBJVAR_ARMOR_BASE + "."+ armor.OBJVAR_ARMOR_CATEGORY;
			setObjVar(beast, armorCategoryObjVar, AC_battle);
			
			String armorLevelObjVar = armor.OBJVAR_ARMOR_BASE + "."+ armor.OBJVAR_ARMOR_LEVEL;
			setObjVar(beast, armorLevelObjVar, AL_standard);
			
			String genProtectionObjVar = armor.OBJVAR_ARMOR_BASE + "."+ armor.OBJVAR_GENERAL_PROTECTION;
			setObjVar(beast, genProtectionObjVar, (float)intArmor);
			utils.setScriptVar(beast, armor.SCRIPTVAR_CACHED_GENERAL_PROTECTION, intArmor);
		}
		
		int exp = 0;
		if (hasObjVar(barn, "barnStorage."+storageSlot+".beast.experience"))
		{
			exp = getIntObjVar(barn, "barnStorage."+storageSlot+".beast.experience");
		}
		beast_lib.setBeastExperience(beast, exp);
		
		boolean canLevel = false;
		if (hasObjVar(barn, "barnStorage."+storageSlot+".beast.canLevel"))
		{
			canLevel = getBooleanObjVar(barn, "barnStorage."+storageSlot+".beast.canLevel");
		}
		beast_lib.setBeastCanLevel(beast, canLevel);
		
		int attentionPenaltyReduction = getEnhancedSkillStatisticModifierUncapped(getMaster(beast), "expertise_bm_self_debuff_reduction");
		int attentionPenalty = beast_lib.BASE_ATTENTION_PENALTY + attentionPenaltyReduction;
		int hackeyWorkAroundValue = (attentionPenalty / 5) * -1;
		
		if (hackeyWorkAroundValue > 5)
		{
			hackeyWorkAroundValue = 5;
		}
		
		if (attentionPenalty < 0 || hackeyWorkAroundValue > 0)
		{
			buff.applyBuff(getMaster(beast), beast_lib.ATTENTION_PENALTY_DEBUFF + hackeyWorkAroundValue);
		}
		else
		{
			beast_lib.removeAttentionPenaltyDebuff(getMaster(beast));
		}
		
		return;
	}
	
	
	public static boolean grantPackageContents(obj_id objectPacked, obj_id player) throws InterruptedException
	{
		if (!isValidId(objectPacked) || !exists(objectPacked))
		{
			return false;
		}
		if (!isValidId(player) || !exists(player))
		{
			return false;
		}
		if (!hasObjVar(objectPacked, "tcg.object"))
		{
			return false;
		}
		
		String objList = getStringObjVar(objectPacked, "tcg.object");
		if (objList == null || objList.equals(""))
		{
			return false;
		}
		
		String[] staticObjects = split(objList, ':');
		if (staticObjects == null)
		{
			return false;
		}
		
		for (int i = 0; i < staticObjects.length; i++)
		{
			testAbortScript();
			obj_id unpackedObject = static_item.createNewItemFunction(staticObjects[i], player);
			CustomerServiceLog("tcg", "Packed item: "+ objectPacked + " "+ getName(objectPacked) + " unpacked: "+unpackedObject+" "+getName(unpackedObject)+" for player: "+ player + " "+ getName(player)+".");
		}
		
		return true;
	}
	
	
	public static boolean setRoamingBeastHue(obj_id beast, String palette, int hueIndex) throws InterruptedException
	{
		if (!isValidId(beast) || !exists(beast))
		{
			return false;
		}
		if (palette == null || palette.length() <= 0)
		{
			return false;
		}
		if (hueIndex < 0)
		{
			return false;
		}
		
		LOG("barnStorage", "setRoamingBeastHue palette: "+ palette + " hueIndex: "+ hueIndex);
		
		hue.setColor(beast, palette, hueIndex);
		return true;
	}
	
}
