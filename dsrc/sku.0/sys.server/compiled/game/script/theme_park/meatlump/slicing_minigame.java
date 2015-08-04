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


public class slicing_minigame extends script.base_script
{
	public slicing_minigame()
	{
	}
	public static final boolean LOGGING_ON = false;
	
	public static final String VAR_PREFIX = "meatlump_safe";
	public static final String NONIMPERATIVE_VAR_PREFIX = "meatlump_safe_nonimperative";
	public static final String PID_NAME = NONIMPERATIVE_VAR_PREFIX + ".pid";
	public static final String PASSWORD = VAR_PREFIX + ".password";
	public static final String PASSWORD_SCRAMBLED = VAR_PREFIX + ".password_scrambled";
	public static final String PASSWORD_ROW = VAR_PREFIX + ".password_row";
	public static final String PASSWORD_POINTS_NEEDED = VAR_PREFIX + ".password_points_needed";
	public static final String PASSWORD_POINTS_CURRENT = NONIMPERATIVE_VAR_PREFIX + ".password_points_current";
	public static final String CURRENTLY_SLICING = NONIMPERATIVE_VAR_PREFIX + ".slicing";
	public static final String ANAGRAM_GUESS_LIST = NONIMPERATIVE_VAR_PREFIX + ".anagram_guess_list";
	public static final String WRONG_ANSWER_COUNT = NONIMPERATIVE_VAR_PREFIX + ".wrong_answer_count";
	public static final String WRONG_ANSWER_THRESHOLD = NONIMPERATIVE_VAR_PREFIX + ".wrong_answer_threshold";
	
	public static final String OBJVAR_SLOT_NAME = "collection.slotName";
	public static final String SAFE_CAPTION = "OLD SAFE";
	public static final String DEVICE_TEMPLATE = "object/tangible/meatlump/event/slicing_device_meatlump_safe.iff";
	public static final String DEVICE_OBJVAR = "puzzle";
	public static final String SAFE_PUZZLE_BUFF = "safe_puzzle_buff";
	
	public static final String PASSWORD_TABLE = "datatables/theme_park/meatlump_safe_passwords.iff";
	
	public static final string_id SID_OPEN_SAFE = new string_id("meatlump/meatlump", "meatlump_safe_open");
	public static final string_id SAFE_INTRO_TEXT = new string_id("meatlump/meatlump", "safe_minigame_text");
	public static final string_id SID_ALREADY_GUESSED_PREVIOUS = new string_id("meatlump/meatlump", "safe_guessed_previous");
	public static final string_id SID_YOU_FAILED_TO_SOLVE = new string_id("meatlump/meatlump", "you_failed_to_solve_slicing_puzzle");
	public static final string_id YOU_FAILED = new string_id("meatlump/meatlump", "you_failed");
	public static final string_id YOU_CANCELED_EARLY = new string_id("meatlump/meatlump", "you_canceled_early");
	public static final string_id YOU_HAVE_DEBUFF = new string_id("meatlump/meatlump", "you_have_debuff");
	public static final string_id YOU_NEED_DEVICE = new string_id("meatlump/meatlump", "you_need_device");
	
	public static final int SKIPPED_COLUMNS = 2;
	public static final int DEFAULT_ANAGRAM_POINT = 1;
	
	
	public int OnObjectMenuRequest(obj_id self, obj_id player, menu_info mi) throws InterruptedException
	{
		
		mi.addRootMenu(menu_info_types.ITEM_USE, SID_OPEN_SAFE);
		return SCRIPT_CONTINUE;
	}
	
	
	public int OnObjectMenuSelect(obj_id self, obj_id player, int item) throws InterruptedException
	{
		if (!isValidId(player) || !exists(player))
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
		else if (buff.hasBuff(player, "safe_puzzle_downer"))
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
		createSafeSui(collectionItem, player);
		
		return SCRIPT_CONTINUE;
	}
	
	
	public boolean createSafeSui(obj_id collectionItem, obj_id player) throws InterruptedException
	{
		if (!isValidId(collectionItem) || !isValidId(player))
		{
			return false;
		}
		
		blog("createSafeSui");
		
		if (!utils.hasScriptVar(player, CURRENTLY_SLICING) && !utils.hasScriptVar(player, PASSWORD+"_"+collectionItem))
		{
			blog("setting up password for slicing session");
			boolean varsSet = getRandomPassword(collectionItem, player);
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
		
		String jumbledPassword = utils.getStringScriptVar(player, PASSWORD_SCRAMBLED+"_"+collectionItem);
		int datatableRow = utils.getIntScriptVar(player, PASSWORD_ROW+"_"+collectionItem);
		int passwordPointsNeeded = utils.getIntScriptVar(player, PASSWORD_POINTS_NEEDED);
		int passwordPointsCurrent = utils.getIntScriptVar(player, PASSWORD_POINTS_CURRENT);
		int wrongAnswerCount = utils.getIntScriptVar(player, WRONG_ANSWER_COUNT);
		int wrongAnswerThreshold = utils.getIntScriptVar(player, WRONG_ANSWER_THRESHOLD);
		String questionnaireText = localize(SAFE_INTRO_TEXT);
		String guessList = utils.getStringScriptVar(player, ANAGRAM_GUESS_LIST);
		
		blog("createSafeSui - displaying SUI data");
		
		questionnaireText += sui.newLine(2);
		questionnaireText += "Scrambled Password: ";
		questionnaireText += sui.colorRed() + jumbledPassword + sui.colorWhite() + sui.newLine();
		questionnaireText += "Total Fail Attempts: "+ sui.colorRed() + wrongAnswerCount + sui.colorWhite() +sui.newLine();
		questionnaireText += "Maximum Fail Attempts Allowed: "+ sui.colorRed() + (wrongAnswerThreshold + 1) + sui.colorWhite() +sui.newLine();
		questionnaireText += "Points Needed: ";
		questionnaireText += sui.colorRed() + passwordPointsNeeded + sui.colorWhite() + sui.newLine();
		questionnaireText += "Current Points: ";
		questionnaireText += sui.colorRed() + passwordPointsCurrent + sui.colorWhite() + sui.newLine(2);
		
		if (guessList != null && !guessList.equals(""))
		{
			questionnaireText += sui.colorGreen() + guessList + sui.colorWhite() + sui.newLine();
		}
		
		dictionary params = new dictionary();
		int pid = createSUIPage("/Script.questionnaire", collectionItem, player);
		setSUIAssociatedLocation(pid, collectionItem);
		setSUIMaxRangeToObject(pid, 8);
		params.put("callingPid", pid);
		sui.setPid(player, pid, PID_NAME);
		
		setSUIProperty(pid, "Prompt.lblPrompt", "LocalText", questionnaireText);
		setSUIProperty(pid, "bg.caption.lblTitle", "Text", SAFE_CAPTION);
		setSUIProperty(pid, "Prompt.lblPrompt", "Editable", "false");
		setSUIProperty(pid, "Prompt.lblPrompt", "GetsInput", "false");
		setSUIProperty(pid, "txtInput", "Editable", "true");
		setSUIProperty(pid, "txtInput", "GetsInput", "true");
		
		setSUIProperty(pid, "btnOk", "Visible", "true");
		setSUIProperty(pid, "btnCancel", "Visible", "true");
		
		subscribeToSUIPropertyForEvent(pid, sui_event_type.SET_onClosedOk, "%button2%", "txtInput", "LocalText");
		
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
		
		if (params.getString("txtInput.LocalText") == null)
		{
			return SCRIPT_CONTINUE;
		}
		
		blog("handleDialogInput - validation completed: ");
		
		String playerGuess = params.getString("txtInput.LocalText");
		blog("handleDialogInput - playerGuess: "+playerGuess);
		
		String correctPassword = utils.getStringScriptVar(player, PASSWORD+"_"+self);
		if (playerGuess.equals(correctPassword))
		{
			blog("handleDialogInput - Password Solved!! Rewardign Player");
			rewardPlayer(self, player);
			return SCRIPT_CONTINUE;
		}
		
		else if (isNameReserved(playerGuess))
		{
			blog("handleDialogInput - reserved or foul language");
			if (!incrementWrongAnswer(self, player))
			{
				applyPuzzleDebuff(self, player);
				return SCRIPT_CONTINUE;
			}
			
			createSafeSui(self, player);
		}
		
		else if (utils.hasScriptVar(player, ANAGRAM_GUESS_LIST) && !checkGuessList(self, player, playerGuess))
		{
			blog("handleDialogInput - player attempted to reuse a previously used guess: "+ playerGuess);
			sendSystemMessage(player, SID_ALREADY_GUESSED_PREVIOUS);
			if (!incrementWrongAnswer(self, player))
			{
				applyPuzzleDebuff(self, player);
				return SCRIPT_CONTINUE;
			}
			
			createSafeSui(self, player);
			return SCRIPT_CONTINUE;
		}
		
		blog("handleDialogInput - Guess != password");
		
		int datatableRow = utils.getIntScriptVar(player, PASSWORD_ROW+"_"+self);
		if (datatableRow < 0)
		{
			return SCRIPT_CONTINUE;
		}
		
		int totalCols = (dataTableGetNumColumns(PASSWORD_TABLE) - SKIPPED_COLUMNS);
		if (totalCols < 0)
		{
			return SCRIPT_CONTINUE;
		}
		
		dictionary dictRow = dataTableGetRow(PASSWORD_TABLE, datatableRow);
		if (dictRow == null)
		{
			return SCRIPT_CONTINUE;
		}
		
		blog("handleDialogInput - Datatable Validation Complete");
		
		boolean recreateSui = true;
		boolean incrementWrong = true;
		
		for (int i = 1; i < totalCols; i++)
		{
			testAbortScript();
			String anagram = dictRow.getString("anagram"+i);
			if (anagram == null || anagram.equals(""))
			{
				continue;
			}
			else if (!playerGuess.equals(anagram))
			{
				continue;
			}
			else
			{
				
				blog("handleDialogInput - match found for: "+ playerGuess);
				recreateSui = givePoints(self, player, anagram);
				incrementWrong = false;
				break;
			}
		}
		
		blog("handleDialogInput - No Match Found");
		
		if (incrementWrong)
		{
			if (!incrementWrongAnswer(self, player))
			{
				applyPuzzleDebuff(self, player);
				return SCRIPT_CONTINUE;
			}
			else if (recreateSui)
			{
				createSafeSui(self, player);
			}
		}
		return SCRIPT_CONTINUE;
	}
	
	
	public boolean applyPuzzleDebuff(obj_id collectionItem, obj_id player) throws InterruptedException
	{
		if (!isValidId(collectionItem) || !isValidId(player))
		{
			return false;
		}
		
		closeOldWindow(player);
		removePlayerNonImperativeVars(player);
		return true;
	}
	
	
	public boolean incrementWrongAnswer(obj_id collectionItem, obj_id player) throws InterruptedException
	{
		blog("incrementWrongAnswer - INIT");
		
		if (!isValidId(collectionItem) || !isValidId(player))
		{
			return false;
		}
		
		if (!utils.hasScriptVar(player, WRONG_ANSWER_COUNT))
		{
			return false;
		}
		else if (!utils.hasScriptVar(player, WRONG_ANSWER_THRESHOLD))
		{
			return false;
		}
		
		int currentCount = utils.getIntScriptVar(player, WRONG_ANSWER_COUNT);
		int threshold = utils.getIntScriptVar(player, WRONG_ANSWER_THRESHOLD);
		if (currentCount < 0 || threshold < 0)
		{
			return false;
		}
		
		blog("incrementWrongAnswer - currentCount: "+ currentCount);
		blog("incrementWrongAnswer - threshold: "+ threshold);
		
		if (currentCount >= threshold)
		{
			if (buff.applyBuff(player, "safe_puzzle_downer"))
			{;
			}
			{
				sendSystemMessage(player, YOU_FAILED);
			}
			return false;
		}
		
		blog("incrementWrongAnswer - INCREMENTING "+ currentCount);
		utils.setScriptVar(player, WRONG_ANSWER_COUNT, currentCount+1);
		blog("incrementWrongAnswer - INCREMENTED "+ utils.getIntScriptVar(player, WRONG_ANSWER_COUNT));
		return true;
	}
	
	
	public boolean getRandomPassword(obj_id collectionItem, obj_id player) throws InterruptedException
	{
		if (!isValidId(collectionItem) || !isValidId(player))
		{
			return false;
		}
		
		int datatableLength = dataTableGetNumRows(PASSWORD_TABLE);
		int randomNum = rand(0, datatableLength-1);
		if (randomNum < 0)
		{
			return false;
		}
		
		String password = dataTableGetString(PASSWORD_TABLE, randomNum, "password");
		if (password == null || password.equals(""))
		{
			return false;
		}
		
		int pointsNeeded = dataTableGetInt(PASSWORD_TABLE, randomNum, "points_needed");
		if (pointsNeeded < 0)
		{
			return false;
		}
		int threshold = dataTableGetInt(PASSWORD_TABLE, randomNum, "threshold");
		if (threshold < 0)
		{
			return false;
		}
		
		String scrambled = scramblePassword(password);
		if (scrambled == null || scrambled.equals(""))
		{
			return false;
		}
		
		boolean hasBuff = false;
		if (buff.hasBuff(player, SAFE_PUZZLE_BUFF))
		{
			if (pointsNeeded > 1)
			{
				pointsNeeded -= 1;
			}
			
			threshold += 3;
		}
		
		utils.setScriptVar(player, PASSWORD+"_"+collectionItem, password);
		utils.setScriptVar(player, PASSWORD_SCRAMBLED+"_"+collectionItem, scrambled);
		utils.setScriptVar(player, PASSWORD_ROW+"_"+collectionItem, randomNum);
		utils.setScriptVar(player, PASSWORD_POINTS_NEEDED, pointsNeeded);
		utils.setScriptVar(player, PASSWORD_POINTS_CURRENT, 0);
		utils.setScriptVar(player, WRONG_ANSWER_THRESHOLD, threshold);
		utils.setScriptVar(player, WRONG_ANSWER_COUNT, 0);
		
		return true;
	}
	
	
	public String scramblePassword(String password) throws InterruptedException
	{
		if (password == null || password.equals(""))
		{
			return null;
		}
		
		String reversedPassword = "";
		String scrambled = "";
		java.util.Random r;
		r = new java.util.Random();
		
		for (int i = 0; i < password.length(); i++)
		{
			testAbortScript();
			if (r.nextBoolean())
			{
				scrambled = scrambled + password.charAt(i);
			}
			else
			{
				scrambled = password.charAt(i) + scrambled;
			}
		}
		
		for (int d = (password.length() - 1); d >= 0; d--)
		{
			testAbortScript();
			reversedPassword += scrambled.charAt(d);
		}
		if (reversedPassword == null || reversedPassword.equals("") || reversedPassword.length() < 0)
		{
			return null;
		}
		else if (reversedPassword.length() != password.length())
		{
			return null;
		}
		
		return scrambled;
	}
	
	
	public boolean checkGuessList(obj_id collectionItem, obj_id player, String anagram) throws InterruptedException
	{
		blog("checkGuessList");
		
		if (!isValidId(collectionItem) || !isValidId(player))
		{
			return false;
		}
		else if (anagram == null || anagram.equals(""))
		{
			return false;
		}
		
		blog("checkGuessList validation completed. anagram = "+anagram);
		
		if (!utils.hasScriptVar(player, ANAGRAM_GUESS_LIST))
		{
			return true;
		}
		
		blog("checkGuessList anagram list found");
		
		String guessList = utils.getStringScriptVar(player, ANAGRAM_GUESS_LIST);
		if (guessList == null || guessList.equals(""))
		{
			return false;
		}
		
		blog("checkGuessList guessList populated with previous list: "+ guessList);
		
		String[] splitAnagramList = split(guessList, ',');
		if (splitAnagramList == null || splitAnagramList.length < 0)
		{
			return false;
		}
		
		int listLength = splitAnagramList.length;
		blog("checkGuessList listLength: "+listLength);
		for (int i = 0; i < listLength; i++)
		{
			testAbortScript();
			blog("checkGuessList splitAnagramList:"+ splitAnagramList[i].trim() + "* anagram:"+anagram);
			
			if (splitAnagramList[i].trim().equals(anagram))
			{
				return false;
			}
		}
		
		blog("checkGuessList went through the entire list and found no match.");
		return true;
	}
	
	
	public boolean givePoints(obj_id collectionItem, obj_id player, String anagram) throws InterruptedException
	{
		if (!isValidId(collectionItem) || !isValidId(player))
		{
			return false;
		}
		else if (anagram == null || anagram.equals(""))
		{
			return false;
		}
		int passwordPointsNeeded = utils.getIntScriptVar(player, PASSWORD_POINTS_NEEDED);
		int passwordPointsCurrent = utils.getIntScriptVar(player, PASSWORD_POINTS_CURRENT);
		if (passwordPointsNeeded < 0 || passwordPointsCurrent < 0)
		{
			return false;
		}
		else if (passwordPointsNeeded == passwordPointsCurrent)
		{
			rewardPlayer(collectionItem, player);
			
			return false;
		}
		else if (passwordPointsCurrent >= passwordPointsNeeded)
		{
			rewardPlayer(collectionItem, player);
			
			return false;
		}
		
		String guessList = utils.getStringScriptVar(player, ANAGRAM_GUESS_LIST);
		if (guessList != null && !guessList.equals(""))
		{
			utils.setScriptVar(player, ANAGRAM_GUESS_LIST, guessList + ", "+ anagram);
		}
		else
		{
			utils.setScriptVar(player, ANAGRAM_GUESS_LIST, anagram);
		}
		
		int point = DEFAULT_ANAGRAM_POINT;
		
		if ((passwordPointsCurrent + point) >= passwordPointsNeeded)
		{
			rewardPlayer(collectionItem, player);
			
			return false;
		}
		
		utils.setScriptVar(player, PASSWORD_POINTS_CURRENT, passwordPointsCurrent+point);
		createSafeSui(collectionItem, player);
		
		return true;
	}
	
	
	public boolean rewardPlayer(obj_id collectionItem, obj_id player) throws InterruptedException
	{
		blog("rewardPlayer - init");
		if (!isValidId(collectionItem) || !isValidId(player))
		{
			return false;
		}
		else if (!exists(collectionItem) || !exists(player))
		{
			return false;
		}
		blog("rewardPlayer - validation complete");
		
		if (!collection.rewardPlayerCollectionSlot(player, collectionItem))
		{
			blog("rewardPlayer - rewardPlayerCollectionSlot = FAIL!!!");
			closeOldWindow(player);
			removePlayerNonImperativeVars(player);
			return false;
		}
		blog("rewardPlayer - rewardPlayerCollectionSlot = GIVE LOOT");
		
		loot.giveMeatlumpPuzzleLoot(player, true, false);
		
		blog("rewardPlayer - rewardPlayerCollectionSlot = SUCCESS");
		closeOldWindow(player);
		removePlayerNonImperativeVars(player);
		
		removePlayerImperativeVars(player);
		
		return true;
	}
	
	
	public int closeSui(obj_id self, dictionary params) throws InterruptedException
	{
		obj_id player = sui.getPlayerId(params);
		if (!isValidId(player))
		{
			return SCRIPT_CONTINUE;
		}
		if (buff.applyBuff(player, "safe_puzzle_downer"))
		{;
		}
		{
			sendSystemMessage(player, YOU_CANCELED_EARLY);
		}
		
		blog("closeSui");
		closeOldWindow(player);
		
		removePlayerNonImperativeVars(player);
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
	
	
	public void removePlayerNonImperativeVars(obj_id player) throws InterruptedException
	{
		utils.removeScriptVarTree(player, NONIMPERATIVE_VAR_PREFIX);
		utils.removeObjVar(player, NONIMPERATIVE_VAR_PREFIX);
	}
	
	
	public void removePlayerImperativeVars(obj_id player) throws InterruptedException
	{
		utils.removeScriptVarTree(player, VAR_PREFIX);
		utils.removeObjVar(player, VAR_PREFIX);
	}
	
	
	public boolean blog(String msg) throws InterruptedException
	{
		if (LOGGING_ON)
		{
			LOG("minigame",msg);
		}
		return true;
	}
}
