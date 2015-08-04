package script.theme_park.meatlump;

import script.*;
import script.base_class.*;
import script.combat_engine.*;
import java.util.Arrays;
import java.util.Hashtable;
import java.util.Vector;
import script.base_script;

import script.library.buff;
import script.library.collection;
import script.library.consumable;
import script.library.loot;
import script.library.sui;
import script.library.stealth;
import script.library.utils;
import java.util.Random;


public class code_break_minigame extends script.base_script
{
	public code_break_minigame()
	{
	}
	public static final String VAR_PREFIX = "meatlump_code";
	public static final String PID_NAME = VAR_PREFIX + ".pid";
	public static final String NUMBER_OF_COMBOS = VAR_PREFIX + ".numberOfCombinations";
	public static final String GUESS_POSITION = VAR_PREFIX + ".guessPostion";
	public static final String TOTAL_GUESSES = VAR_PREFIX + ".total_guesses";
	
	public static final String CURRENTLY_SLICING = VAR_PREFIX + ".slicing";
	public static final String OBJVAR_SLOT_NAME = "collection.slotName";
	public static final String SAFE_CAPTION = "LOCK BREAKER";
	public static final String DEVICE_TEMPLATE = "object/tangible/meatlump/event/slicing_device_meatlump_container.iff";
	public static final String DEVICE_OBJVAR = "puzzle";
	public static final String CONTAINER_PUZZLE_BUFF = "locked_container_puzzle_buff";
	
	public static final String THRESHOLD_TABLE = "datatables/theme_park/meatlump_threshold.iff";
	
	public static final string_id SID_OPEN_SAFE = new string_id("meatlump/meatlump", "meatlump_container_open");
	public static final string_id SAFE_INTRO_TEXT = new string_id("meatlump/meatlump", "slicing_minigame_text");
	public static final string_id SID_YOU_GUESSED_CORRECT = new string_id("meatlump/meatlump", "guessed_correct");
	public static final string_id SID_YOU_GUESSED_CORRECT_GOD = new string_id("meatlump/meatlump", "guessed_correct_godmode");
	public static final string_id SID_YOU_GUESSED_INCORRECT_GOD = new string_id("meatlump/meatlump", "guessed_incorrect_godmode");
	public static final string_id SID_ALL_THRESHOLDS_MET = new string_id("meatlump/meatlump", "all_thresholds_met");
	public static final string_id YOU_CANCELED_EARLY = new string_id("meatlump/meatlump", "you_canceled_early");
	public static final string_id YOU_HAVE_DEBUFF = new string_id("meatlump/meatlump", "you_have_debuff");
	public static final string_id YOU_NEED_DEVICE = new string_id("meatlump/meatlump", "you_need_device");
	
	public static final int TOTAL_GUESS_DEFAULT = 0;
	public static final int MAX_INT_COMBO_1 = 10000;
	public static final int MAX_INT_COMBO_2 = 5000;
	public static final int MAX_INT_COMBO_3 = 2000;
	public static final int MAX_INT_COMBO_4 = 1000;
	public static final int MAX_INT_COMBO_5 = 500;
	public static final int MAX_INT_COMBO_6 = 100;
	
	
	public int OnObjectMenuRequest(obj_id self, obj_id player, menu_info mi) throws InterruptedException
	{
		
		mi.addRootMenu(menu_info_types.ITEM_USE, SID_OPEN_SAFE);
		return SCRIPT_CONTINUE;
	}
	
	
	public int OnObjectMenuSelect(obj_id self, obj_id player, int item) throws InterruptedException
	{
		if (!isValidId(player))
		{
			return SCRIPT_CONTINUE;
		}
		
		obj_id collectionItem = self;
		
		if (item != menu_info_types.ITEM_USE)
		{
			return SCRIPT_CONTINUE;
		}
		else if (!collection.canCollectCollectible(player, self))
		{
			return SCRIPT_CONTINUE;
		}
		else if (buff.hasBuff(player, "locked_container_puzzle_downer"))
		{
			sendSystemMessage(player, YOU_HAVE_DEBUFF);
			return SCRIPT_CONTINUE;
		}
		else if (!consumable.decrementObjectInventoryOrEquipped(player, DEVICE_TEMPLATE, DEVICE_OBJVAR))
		{
			sendSystemMessage(player, YOU_NEED_DEVICE);
			return SCRIPT_CONTINUE;
		}
		
		closeOldWindow(player);
		blog("OnObjectMenuSelect");
		createSui(collectionItem, player);
		
		return SCRIPT_CONTINUE;
	}
	
	
	public boolean createSui(obj_id collectionItem, obj_id player) throws InterruptedException
	{
		if (!isValidId(collectionItem) || !isValidId(player))
		{
			return false;
		}
		
		blog("createSui");
		
		if (!utils.hasScriptVar(player, CURRENTLY_SLICING))
		{
			blog("setting up password for slicing session");
			boolean varsSet = getRandomNumberCombinations(collectionItem, player);
			if (!varsSet)
			{
				return false;
			}
			utils.setScriptVar(player, CURRENTLY_SLICING, true);
		}
		else
		{
			closeOldWindow(player);
		}
		
		int numberOfCombos = utils.getIntScriptVar(player, NUMBER_OF_COMBOS);
		if (numberOfCombos <= 0)
		{
			blog("numberOfCombos failed: "+numberOfCombos);
			return false;
		}
		
		blog("numberOfCombos: "+numberOfCombos);
		
		blog("createSui - displaying SUI data");
		String questionnaireText = localize(SAFE_INTRO_TEXT);
		
		int guessPostion = 1;
		
		if (utils.hasScriptVar(player, GUESS_POSITION))
		{
			guessPostion = utils.getIntScriptVar(player, GUESS_POSITION);
		}
		else
		{
			utils.setScriptVar(player, GUESS_POSITION, guessPostion);
		}
		
		blog("createSui - guessPostion: "+guessPostion);
		
		questionnaireText += sui.newLine(2);
		for (int i = 1; i < numberOfCombos+1; i++)
		{
			testAbortScript();
			questionnaireText += "Number combination "+ i + ": ";
			if (guessPostion <= i)
			{
				blog("createSui - guessPostion <= "+i);
				
				questionnaireText += sui.colorRed() + utils.getStringScriptVar(player, VAR_PREFIX + ".number_combo_asterisk_"+ i);
				if (utils.hasScriptVar(player, VAR_PREFIX + ".number_combo_guess_"+i))
				{
					blog("createSui - FOUND number_combo_guess_"+i);
					questionnaireText += sui.newLine() + sui.colorBlue() + utils.getStringScriptVar(player, VAR_PREFIX + ".number_combo_guess_"+ i);
					questionnaireText += sui.colorWhite();
				}
				else
				{
					questionnaireText += sui.newLine();
				}
				
			}
			else
			{
				questionnaireText += sui.colorGreen() + utils.getIntScriptVar(player, VAR_PREFIX + ".number_combo_"+ i) + sui.newLine();
			}
			
			questionnaireText += sui.colorWhite() + sui.newLine();
		}
		
		dictionary params = new dictionary();
		int pid = createSUIPage("/Script.sliceTerminal", collectionItem, player);
		setSUIAssociatedLocation(pid, collectionItem);
		setSUIMaxRangeToObject(pid, 8);
		params.put("callingPid", pid);
		sui.setPid(player, pid, PID_NAME);
		
		setSUIProperty(pid, "sliceInstructions.lblPrompt", "LocalText", questionnaireText);
		setSUIProperty(pid, "bg.caption.lblTitle", "Text", SAFE_CAPTION);
		setSUIProperty(pid, "sliceInstructions.lblPrompt", "Editable", "false");
		setSUIProperty(pid, "sliceInstructions.lblPrompt", "GetsInput", "false");
		setSUIProperty(pid, "result.numberBox", "Editable", "true");
		setSUIProperty(pid, "result.numberBox", "GetsInput", "true");
		
		setSUIProperty(pid, "btnOk", "Visible", "true");
		setSUIProperty(pid, "btnCancel", "Visible", "true");
		
		subscribeToSUIPropertyForEvent(pid, sui_event_type.SET_onClosedOk, "%button2%", "result.numberBox", "LocalText");
		
		subscribeToSUIEvent(pid, sui_event_type.SET_onClosedOk, "%button2%", "handleDialogInput");
		subscribeToSUIEvent(pid, sui_event_type.SET_onClosedCancel, "%button0%", "closeSui");
		
		showSUIPage(pid);
		flushSUIPage(pid);
		
		return true;
	}
	
	
	public int handleDialogInput(obj_id self, dictionary params) throws InterruptedException
	{
		blog("handleDialogInput");
		
		obj_id player = sui.getPlayerId(params);
		if (!isValidId(player))
		{
			return SCRIPT_CONTINUE;
		}
		
		blog("handleDialogInput - player validated");
		blog("handleDialogInput - params: "+params);
		
		if (params.getString("result.numberBox.LocalText") == null)
		{
			return SCRIPT_CONTINUE;
		}
		
		blog("handleDialogInput - validation completed: ");
		
		String playerGuess = params.getString("result.numberBox.LocalText");
		blog("handleDialogInput - playerGuess: "+playerGuess);
		
		int playerGuessInt = utils.stringToInt(playerGuess);
		if (playerGuessInt < 0)
		{
			blog("handleDialogInput - playerGuessInt faled: "+playerGuessInt);
			return SCRIPT_CONTINUE;
		}
		blog("handleDialogInput - playerGuessInt: "+playerGuessInt);
		
		if (!utils.hasScriptVar(player, GUESS_POSITION))
		{
			blog("handleDialogInput - no GUESS_POSITION");
			return SCRIPT_CONTINUE;
		}
		blog("handleDialogInput - GUESS_POSITION attained");
		
		int guessPostion = utils.getIntScriptVar(player, GUESS_POSITION);
		if (guessPostion <= 0)
		{
			blog("handleDialogInput - guessPostion: "+guessPostion);
			return SCRIPT_CONTINUE;
		}
		blog("handleDialogInput - guessPostion: "+guessPostion);
		
		int secretNumber = utils.getIntScriptVar(player, VAR_PREFIX + ".number_combo_"+ guessPostion);
		if (secretNumber < 0)
		{
			return SCRIPT_CONTINUE;
		}
		
		blog("handleDialogInput - secretNumber: "+secretNumber+" and the player guess was: "+playerGuessInt);
		
		int totalGuesses = TOTAL_GUESS_DEFAULT;
		if (utils.hasScriptVar(player, TOTAL_GUESSES))
		{
			totalGuesses = utils.getIntScriptVar(player, TOTAL_GUESSES);
		}
		
		blog("handleDialogInput - totalGuesses: "+totalGuesses);
		
		if (playerGuessInt == secretNumber)
		{
			blog("handleDialogInput - Password Solved!! Rewardign Player");
			completeNumberGuess(self, player, guessPostion);
			return SCRIPT_CONTINUE;
		}
		else if (playerGuessInt > secretNumber)
		{
			
			blog("handleDialogInput Too High");
			utils.setScriptVar(player, VAR_PREFIX + ".number_combo_guess_"+ guessPostion, " ("+ playerGuessInt + " too high)");
			utils.setScriptVar(player, TOTAL_GUESSES, totalGuesses++);
		}
		else
		{
			
			blog("handleDialogInput Too Low");
			utils.setScriptVar(player, VAR_PREFIX + ".number_combo_guess_"+ guessPostion, " ("+ playerGuessInt + " too low)");
			utils.setScriptVar(player, TOTAL_GUESSES, totalGuesses++);
		}
		
		createSui(self, player);
		return SCRIPT_CONTINUE;
	}
	
	
	public boolean getRandomNumberCombinations(obj_id collectionItem, obj_id player) throws InterruptedException
	{
		if (!isValidId(collectionItem) || !isValidId(player))
		{
			return false;
		}
		
		boolean hasBuff = false;
		if (buff.hasBuff(player, CONTAINER_PUZZLE_BUFF))
		{
			hasBuff = true;
		}
		
		blog("HASBUFF: "+hasBuff);
		
		int amountOfNumbers = rand(4,6);
		if (hasBuff)
		{
			amountOfNumbers = 3;
		}
		
		blog("getRandomNumberCombinations amountOfNumbers = "+amountOfNumbers);
		
		for (int i = 1; i < amountOfNumbers+1; i++)
		{
			testAbortScript();
			
			int randMax = MAX_INT_COMBO_1;
			if (i == 2)
			{
				randMax = MAX_INT_COMBO_2;
			}
			else if (i == 3)
			{
				randMax = MAX_INT_COMBO_3;
			}
			else if (i == 4)
			{
				randMax = MAX_INT_COMBO_4;
			}
			else if (i == 5)
			{
				randMax = MAX_INT_COMBO_5;
			}
			else if (i == 6)
			{
				randMax = MAX_INT_COMBO_6;
			}
			
			int tempRandVariable = rand(0, randMax);
			
			utils.setScriptVar(player, VAR_PREFIX + ".number_combo_"+ i, tempRandVariable);
			
			int lengthOfVar = (""+ tempRandVariable).length();
			String asterisks = "";
			for (int j = 0; j < lengthOfVar; j++)
			{
				testAbortScript();
				asterisks += "*";
			}
			utils.setScriptVar(player, VAR_PREFIX + ".number_combo_asterisk_"+ i, asterisks);
		}
		
		utils.setScriptVar(player, NUMBER_OF_COMBOS, amountOfNumbers);
		utils.setScriptVar(player, TOTAL_GUESSES, TOTAL_GUESS_DEFAULT);
		
		int thresholdRow = dataTableSearchColumnForInt(amountOfNumbers, "combinations", THRESHOLD_TABLE);
		if (thresholdRow < 0)
		{
			return false;
		}
		
		dictionary threshDict = dataTableGetRow(THRESHOLD_TABLE, thresholdRow);
		
		for (int t = 1; t < amountOfNumbers + 1; t++)
		{
			testAbortScript();
			int threshold = threshDict.getInt("threshold"+t);
			utils.setScriptVar(player, VAR_PREFIX + ".number_combo_threshold_"+ t, threshold);
			blog("getRandomNumberCombinations - .number_combo_threshold_"+ t);
		}
		
		return true;
	}
	
	
	public boolean calculateThreshold(obj_id collectionItem, obj_id player, int guessPosition) throws InterruptedException
	{
		if (!isValidId(collectionItem) || !isValidId(player))
		{
			return false;
		}
		
		int threshold = utils.getIntScriptVar(player, VAR_PREFIX + ".number_combo_threshold_"+ guessPosition);
		int totalGuesses = utils.getIntScriptVar(player, TOTAL_GUESSES);
		
		if (threshold < 0 || totalGuesses < 0)
		{
			return false;
		}
		else if (totalGuesses < threshold)
		{
			
			blog("setting threshold to TRUE");
			utils.setScriptVar(player, VAR_PREFIX + ".under_threshold_"+ guessPosition, true);
		}
		else
		{
			
			blog("setting threshold to FALSE");
			utils.setScriptVar(player, VAR_PREFIX + ".under_threshold_"+ guessPosition, false);
		}
		
		utils.setScriptVar(player, TOTAL_GUESSES, TOTAL_GUESS_DEFAULT);
		return true;
	}
	
	
	public boolean completeNumberGuess(obj_id collectionItem, obj_id player, int guessPosition) throws InterruptedException
	{
		if (!isValidId(collectionItem) || !isValidId(player))
		{
			return false;
		}
		
		blog("completeNumberGuess - validation completed");
		
		int totalNumberOfCombos = utils.getIntScriptVar(player, NUMBER_OF_COMBOS);
		if (totalNumberOfCombos <= 0)
		{
			return false;
		}
		
		blog("completeNumberGuess - totalNumberOfCombos: "+totalNumberOfCombos);
		blog("completeNumberGuess - guessPosition: "+guessPosition);
		
		calculateThreshold(collectionItem, player, guessPosition);
		
		if (guessPosition < totalNumberOfCombos)
		{
			blog("completeNumberGuess - player still has more combinations to win.");
			utils.setScriptVar(player, GUESS_POSITION, guessPosition + 1);
			createSui(collectionItem, player);
			return true;
		}
		
		blog("completeNumberGuess - player completed all guesses.");
		
		boolean completedAllThresholds = true;
		for (int i = 1; i < totalNumberOfCombos + 1; i++)
		{
			testAbortScript();
			if (!utils.hasScriptVar(player, VAR_PREFIX + ".under_threshold_" + i))
			{
				blog("completeNumberGuess - player did not have the proper thresholds saved.");
				return false;
			}
			
			boolean guessThreshold = utils.getBooleanScriptVar(player, VAR_PREFIX + ".under_threshold_"+ i);
			if (!guessThreshold)
			{
				completedAllThresholds = false;
			}
		}
		
		if (!collection.rewardPlayerCollectionSlot(player, collectionItem))
		{
			blog("rewardPlayer - rewardPlayerCollectionSlot = FAIL!!!");
			closeOldWindow(player);
			removePlayerVars(player);
			return false;
		}
		blog("rewardPlayer - rewardPlayerCollectionSlot = GIVE LOOT");
		
		loot.giveMeatlumpPuzzleLoot(player, completedAllThresholds, false);
		
		blog("rewardPlayer - rewardPlayerCollectionSlot = SUCCESS");
		closeOldWindow(player);
		removePlayerVars(player);
		
		return true;
	}
	
	
	public int closeSui(obj_id self, dictionary params) throws InterruptedException
	{
		obj_id player = sui.getPlayerId(params);
		if (!isValidId(player))
		{
			return SCRIPT_CONTINUE;
		}
		
		if (buff.applyBuff(player, "locked_container_puzzle_downer"))
		{;
		}
		{
			sendSystemMessage(player, YOU_CANCELED_EARLY);
		}
		
		blog("closeSui");
		closeOldWindow(player);
		
		return SCRIPT_CONTINUE;
	}
	
	
	public void closeOldWindow(obj_id player) throws InterruptedException
	{
		int pid = sui.getPid(player, PID_NAME);
		if (pid > -1)
		{
			forceCloseSUIPage(pid);
			sui.removePid(player, PID_NAME);
		}
	}
	
	
	public void removePlayerVars(obj_id player) throws InterruptedException
	{
		blog("Removing Player Scriptvars and Objvars from player: "+player);
		utils.removeScriptVarTree(player, VAR_PREFIX);
		utils.removeObjVar(player, VAR_PREFIX);
	}
	
	
	public boolean blog(String msg) throws InterruptedException
	{
		LOG("minigame",msg);
		return true;
	}
}
