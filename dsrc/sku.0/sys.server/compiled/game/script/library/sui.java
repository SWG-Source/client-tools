package script.library;

import script.*;
import script.base_class.*;
import script.combat_engine.*;
import java.util.Arrays;
import java.util.Hashtable;
import java.util.Vector;
import script.base_script;


import script.library.colors_hex;


public class sui extends script.base_script
{
	public sui()
	{
	}
	public static final int MAX_INPUT_LENGTH = 127;
	
	public static final String DEFAULT_TITLE = "@base_player:swg";
	
	public static final String OWNER = "owner";
	public static final String THIS = "this";
	public static final String ASSOCIATE = "associate";
	public static final String RANGE = "range";
	
	public static final String TITLE = "title";
	
	public static final String BUTTONS = "buttons";
	
	public static final String HANDLER = "handler";
	
	public static final String HANDLER_SUI = "handleSUI";
	
	public static final String CTRL_TITLE = "bg.caption.lblTitle";
	
	public static final String BTN_OK = "btnOk";
	public static final String BTN_REVERT = "btnRevert";
	public static final String BTN_CANCEL = "btnCancel";
	
	public static final String PROP_NAME = "Name";
	
	public static final String PROP_TEXT = "Text";
	public static final String PROP_LOCALTEXT = "LocalText";
	
	public static final String PROP_SELECTEDROW = "SelectedRow";
	public static final String PROP_SELECTEDTEXT = "SelectedText";
	public static final String PROP_SELECTEDINDEX = "SelectedIndex";
	
	public static final String PROP_TARGETID = "TargetNetworkId";
	public static final String PROP_TARGETVAR = "TargetVariable";
	public static final String PROP_TARGETRANGEMIN = "TargetRangeMin";
	public static final String PROP_TARGETRANGEMAX = "TargetRangeMax";
	
	public static final String PROP_BUTTONPRESSED = "buttonPressed";
	
	public static final String PROP_VISIBLE = "visible";
	
	public static final String PROP_AUTOSAVE = "autosave";
	public static final String PROP_LOCATION = "Location";
	public static final String PROP_SIZE = "Size";
	public static final String PROP_SOUND = "sound";
	
	public static final String SUI_COLORPICKER = "Script.ColorPicker";
	
	public static final String COLORPICKER_PAGE_CAPTION = "bg.caption";
	
	public static final String COLORPICKER_COLORPICKER = "ColorPicker";
	public static final String COLORPICKER_TITLE = COLORPICKER_PAGE_CAPTION + ".lblTitle";
	
	public static final String SUI_MSGBOX = "Script.messageBox";
	
	public static final String MSGBOX_PAGE_PROMPT = "Prompt";
	public static final String MSGBOX_PAGE_CAPTION = "bg.caption";
	
	public static final String MSGBOX_BTN_REVERT = "btnRevert";
	public static final String MSGBOX_BTN_CANCEL = "btnCancel";
	public static final String MSGBOX_BTN_OK = "btnOk";
	public static final String MSGBOX_PROMPT = MSGBOX_PAGE_PROMPT + ".lblPrompt";
	public static final String MSGBOX_TITLE = MSGBOX_PAGE_CAPTION + ".lblTitle";
	
	public static final String SUI_COUNTDOWNTIMER = "Script.CountdownTimerBar";
	public static final String COUNTDOWNTIMER_PROMPT = "comp.text";
	public static final String COUNTDOWNTIMER_TIMEVALUE = "countdownTimerTimeValue";
	
	public static final String COUNTDOWNTIMER_VAR = "countdown_sui";
	public static final String COUNTDOWNTIMER_OWNER_VAR = COUNTDOWNTIMER_VAR + ".owner";
	public static final String COUNTDOWNTIMER_CALLBACK_VAR = COUNTDOWNTIMER_VAR + ".callback";
	public static final String COUNTDOWNTIMER_FLAGS_VAR = COUNTDOWNTIMER_VAR + ".flags";
	public static final String COUNTDOWNTIMER_SUI_VAR = COUNTDOWNTIMER_VAR + ".sui_pid";
	
	public static final String COUNTDOWNTIMER_PLAYER_SCRIPT = "player.player_countdown";
	
	public static final int COUNTDOWNTIMER_MAXTIMEVALUE = 86400;
	
	public static final int CD_EVENT_NONE = 0x00000000;
	public static final int CD_EVENT_COMBAT = 0x00000001;
	public static final int CD_EVENT_LOCOMOTION = 0x00000002;
	public static final int CD_EVENT_POSTURE = 0x00000004;
	public static final int CD_EVENT_INCAPACITATE = 0x00000008;
	public static final int CD_EVENT_DAMAGED = 0x00000010;
	
	public static final String SUI_INPUTBOX = "Script.inputBox";
	
	public static final String INPUTBOX_PAGE_PROMPT = "Prompt";
	public static final String INPUTBOX_PAGE_CAPTION = "bg.caption";
	
	public static final String INPUTBOX_BTN_CANCEL = "btnCancel";
	public static final String INPUTBOX_BTN_OK = "btnOk";
	public static final String INPUTBOX_PROMPT = INPUTBOX_PAGE_PROMPT + ".lblPrompt";
	public static final String INPUTBOX_TITLE = INPUTBOX_PAGE_CAPTION + ".lblTitle";
	public static final String INPUTBOX_INPUT = "txtInput";
	public static final String INPUTBOX_COMBO = "cmbInput";
	public static final String INPUTBOX_DATASOURCE = "dataInput";
	
	public static final String INPUTBOX_INPUT_NAME = INPUTBOX_INPUT + "."+ PROP_NAME;
	public static final String INPUTBOX_INPUT_LOCALTEXT = INPUTBOX_INPUT + "."+ PROP_LOCALTEXT;
	
	public static final String INPUTBOX_COMBO_SELECTEDTEXT = INPUTBOX_INPUT + "."+ PROP_SELECTEDTEXT;
	public static final String INPUTBOX_COMBO_SELECTEDINDEX = INPUTBOX_INPUT + "."+ PROP_SELECTEDINDEX;
	
	public static final String SUI_LISTBOX = "Script.listBox";
	
	public static final String LISTBOX_PAGE_PROMPT = "Prompt";
	public static final String LISTBOX_PAGE_CAPTION = "bg.caption";
	public static final String LISTBOX_PAGE_LIST = "List";
	
	public static final String LISTBOX_BTN_REVERT = "btnRevert";
	public static final String LISTBOX_BTN_CANCEL = "btnCancel";
	public static final String LISTBOX_BTN_OK = "btnOk";
	public static final String LISTBOX_BTN_OTHER = "btnOther";
	public static final String LISTBOX_TITLE = LISTBOX_PAGE_CAPTION + ".lblTitle";
	public static final String LISTBOX_PROMPT = LISTBOX_PAGE_PROMPT + ".lblPrompt";
	public static final String LISTBOX_LIST = LISTBOX_PAGE_LIST + ".lstList";
	public static final String LISTBOX_DATASOURCE = LISTBOX_PAGE_LIST + ".dataList";
	public static final String LISTBOX_SELECTEDROW = LISTBOX_LIST + "."+ PROP_SELECTEDROW;
	
	public static final String LISTBOX_PROP_OTHER_PRESSED = "otherPressed";
	
	public static final String SUI_TRANSFER = "Script.transfer";
	
	public static final String TRANSFER_PAGE_PROMPT = "Prompt";
	public static final String TRANSFER_PAGE_CAPTION = "bg.caption";
	public static final String TRANSFER_PAGE_TRANSACTION = "transaction";
	
	public static final String TRANSFER_BTN_OK = "btnOk";
	public static final String TRANSFER_BTN_CANCEL = "btnCancel";
	public static final String TRANSFER_BTN_REVERT = "btnRevert";
	
	public static final String TRANSFER_TITLE = TRANSFER_PAGE_CAPTION + ".lblTitle";
	public static final String TRANSFER_PROMPT = TRANSFER_PAGE_PROMPT + ".lblPrompt";
	public static final String TRANSFER_INPUT_FROM = TRANSFER_PAGE_TRANSACTION + ".txtInputFrom";
	public static final String TRANSFER_INPUT_TO = TRANSFER_PAGE_TRANSACTION + ".txtInputTo";
	public static final String TRANSFER_FROM = TRANSFER_PAGE_TRANSACTION + ".lblStartingFrom";
	public static final String TRANSFER_TO = TRANSFER_PAGE_TRANSACTION + ".lblStartingTo";
	public static final String TRANSFER_FROM_TEXT = TRANSFER_PAGE_TRANSACTION + ".lblFrom";
	public static final String TRANSFER_TO_TEXT = TRANSFER_PAGE_TRANSACTION + ".lblTo";
	public static final String TRANSFER_SLIDER = TRANSFER_PAGE_TRANSACTION + ".slider";
	
	public static final String TRANSFER_INPUT_FROM_TEXT = TRANSFER_INPUT_FROM + "."+ PROP_TEXT;
	public static final String TRANSFER_INPUT_TO_TEXT = TRANSFER_INPUT_TO + "."+ PROP_TEXT;
	
	public static final String PROP_CONVERSION_RATIO_FROM = "ConversionRatioFrom";
	public static final String PROP_CONVERSION_RATIO_TO = "ConversionRatioTo";
	
	public static final String MSG_BANK_TITLE = "@base_player:bank_title";
	public static final String MSG_BANK_PROMPT = "@base_player:bank_prompt";
	
	public static final String HANDLER_DEPOSIT_WITHDRAW = "handleDepositWithdraw";
	
	public static final int MAX_ARRAY_SIZE = 50;
	
	public static final int OK_ONLY = 0;
	public static final int OK_CANCEL = 1;
	public static final int YES_NO = 2;
	public static final int YES_NO_CANCEL = 3;
	public static final int YES_NO_MAYBE = 4;
	public static final int YES_NO_ABSTAIN = 5;
	public static final int RETRY_CANCEL = 6;
	public static final int ABORT_RETRY_CANCEL = 7;
	public static final int OK_REFRESH = 8;
	public static final int OK_CANCEL_REFRESH = 9;
	public static final int REFRESH_CANCEL = 10;
	public static final int REFRESH_ONLY = 11;
	public static final int OK_CANCEL_ALL = 12;
	public static final int MOVEUP_MOVEDOWN_DONE = 13;
	public static final int BET_MAX_BET_ONE_QUIT = 14;
	public static final int BET_MAX_BET_ONE_SPIN = 15;
	public static final int REFRESH_LEAVE_GAME = 16;
	public static final int REMOVE_CANCEL = 17;
	
	public static final int MSG_NORMAL = 0;
	public static final int MSG_CRITICAL = 1;
	public static final int MSG_EXCLAMATION = 2;
	public static final int MSG_QUESTION = 3;
	public static final int MSG_INFORMATION = 4;
	
	public static final int INPUT_NORMAL = 0;
	public static final int INPUT_COMBO = 1;
	
	public static final String OK = "Ok";
	public static final String CANCEL = "Cancel";
	public static final String REVERT = "Revert";
	
	public static final int BP_OK = 0;
	public static final int BP_CANCEL = 1;
	public static final int BP_REVERT = 2;
	
	public static final string_id PROSE_OVER_MAX_ENTRIES = new string_id("error_message","prose_over_max_entries");
	public static final string_id OUT_OF_RANGE = new string_id("error_message","out_of_range");
	
	public static final String SCRIPT_FILTER_OWNER = "sui.filter.owner";
	
	public static final String VAR_FILTER_BASE = "sui.filter";
	public static final String VAR_FILTER_COUNT = VAR_FILTER_BASE + ".count";
	
	public static final String HANDLER_FILTER_INPUT = "handleFilterInput";
	
	
	public static int msgbox(obj_id target, string_id sid_prompt, dictionary params) throws InterruptedException
	{
		if (!isIdValid(target))
		{
			return -1;
		}
		
		if (sid_prompt == null)
		{
			return -1;
		}
		
		int pid = createSUIPage(SUI_MSGBOX, target, target, HANDLER_SUI);
		
		String prompt = utils.packStringId(sid_prompt);
		
		setSUIProperty(pid, MSGBOX_PROMPT, PROP_TEXT, prompt);
		setSUIProperty(pid, MSGBOX_TITLE, PROP_TEXT, DEFAULT_TITLE);
		
		setPageProperties(pid, params);
		
		trackSUI(target, pid, params);
		
		showSUIPage(pid);
		
		return pid;
	}
	
	
	public static int msgbox(obj_id target, string_id sid_prompt) throws InterruptedException
	{
		return msgbox(target, sid_prompt, null);
	}
	
	
	public static int countdownTimerSUI(obj_id owner, obj_id target, String type, string_id sid_prompt, int current_time, int total_time, String suiHandler) throws InterruptedException
	{
		if (!isIdValid(owner) || !isIdValid(target))
		{
			return -1;
		}
		
		if (sid_prompt == null)
		{
			sid_prompt = new string_id("sui", "countdowntimer_default_prompt");
		}
		
		String prompt = utils.packStringId(sid_prompt);
		if (prompt == null || prompt.equals(""))
		{
			prompt = "@sui:countdowntimer_default_prompt";
		}
		
		if (type == null || type.equals(""))
		{
			return -1;
		}
		
		if (current_time < 0)
		{
			return -1;
		}
		
		if (total_time <= 0)
		{
			return -1;
		}
		
		if (total_time > COUNTDOWNTIMER_MAXTIMEVALUE)
		{
			return -1;
		}
		
		if (current_time >= total_time)
		{
			return -1;
		}
		
		if (suiHandler == null || suiHandler.equals(""))
		{
			suiHandler = "noHandler";
		}
		
		int pid = createSUIPage(SUI_COUNTDOWNTIMER, owner, target, suiHandler);
		
		if (pid == -1)
		{
			return -1;
		}
		
		if (!setSUIProperty(pid, COUNTDOWNTIMER_PROMPT, PROP_TEXT, prompt))
		{
			return -1;
		}
		
		if (!setSUIProperty(pid, CTRL_TITLE, PROP_TEXT, type))
		{
			return -1;
		}
		
		if (!setSUIProperty(pid, THIS, COUNTDOWNTIMER_TIMEVALUE, current_time + "," + total_time))
		{
			return -1;
		}
		
		if (!showSUIPage(pid))
		{
			return -1;
		}
		
		if (owner != target)
		{
			setAssociateRange(target, pid, owner);
		}
		
		return pid;
	}
	
	
	public static int smartCountdownTimerSUI(obj_id owner, obj_id target, String type, string_id sid_prompt, int current_time, int total_time, String suiHandler, float range, int flags) throws InterruptedException
	{
		if (!isIdValid(owner) || !isIdValid(target))
		{
			return -1;
		}
		
		if (sid_prompt == null)
		{
			sid_prompt = new string_id("sui", "countdowntimer_default_prompt");
		}
		
		String prompt = utils.packStringId(sid_prompt);
		if (prompt == null || prompt.equals(""))
		{
			prompt = "@sui:countdowntimer_default_prompt";
		}
		
		if (type == null || type.equals(""))
		{
			return -1;
		}
		
		if (current_time < 0)
		{
			return -1;
		}
		
		if (total_time <= 0)
		{
			return -1;
		}
		
		if (total_time > COUNTDOWNTIMER_MAXTIMEVALUE)
		{
			return -1;
		}
		
		if (current_time >= total_time)
		{
			return -1;
		}
		
		if (suiHandler == null || suiHandler.equals(""))
		{
			suiHandler = "noHandler";
		}
		
		if (hasScript(target, COUNTDOWNTIMER_PLAYER_SCRIPT))
		{
			sendSystemMessage(target, new string_id("sui", "countdowntimer_too_busy"));
			return -1;
		}
		
		int pid = createSUIPage(SUI_COUNTDOWNTIMER, owner, target, suiHandler);
		
		if (pid == -1)
		{
			return -1;
		}
		
		if (!setSUIProperty(pid, COUNTDOWNTIMER_PROMPT, PROP_TEXT, prompt))
		{
			return -1;
		}
		
		if (!setSUIProperty(pid, CTRL_TITLE, PROP_TEXT, type))
		{
			return -1;
		}
		
		if (!setSUIProperty(pid, THIS, COUNTDOWNTIMER_TIMEVALUE, current_time + "," + total_time))
		{
			return -1;
		}
		
		if (!showSUIPage(pid))
		{
			return -1;
		}
		
		if (owner != target)
		{
			setSUIAssociatedObject(pid, owner);
			setSUIMaxRangeToObject(pid, range);
		}
		else
		{
			setSUIAssociatedLocation(pid,owner);
			setSUIMaxRangeToObject(pid, range);
		}
		
		flushSUIPage(pid);
		
		if (pid > -1)
		{
			setObjVar(target, COUNTDOWNTIMER_SUI_VAR, pid);
			
			utils.setScriptVar(target, COUNTDOWNTIMER_OWNER_VAR, owner);
			utils.setScriptVar(target, COUNTDOWNTIMER_CALLBACK_VAR, suiHandler);
			utils.setScriptVar(target, COUNTDOWNTIMER_FLAGS_VAR, flags);
			
			attachScript(target, COUNTDOWNTIMER_PLAYER_SCRIPT);
			
			dictionary d = new dictionary();
			d.put("buttonPressed", OK);
			d.put("player", target);
			d.put("id", pid);
			d.put("event", CD_EVENT_NONE);
			
			int timeRemaining = total_time - current_time;
			
			messageTo(owner, suiHandler, d, timeRemaining, false);
			messageTo(target, "handleCountdownTimerCleanup", d, (timeRemaining + 2), false);
		}
		
		return pid;
	}
	
	
	public static boolean updateCountdownTimerSUI(int pid, string_id sid_prompt, int current_time, int total_time) throws InterruptedException
	{
		if (sid_prompt == null)
		{
			return false;
		}
		
		String prompt = utils.packStringId(sid_prompt);
		if (prompt == null || prompt.equals(""))
		{
			return false;
		}
		
		if (current_time < 0)
		{
			return false;
		}
		
		if (total_time <= 0)
		{
			return false;
		}
		
		if (total_time > COUNTDOWNTIMER_MAXTIMEVALUE)
		{
			return false;
		}
		
		if (current_time >= total_time)
		{
			return false;
		}
		
		if (!setSUIProperty(pid, COUNTDOWNTIMER_PROMPT, PROP_TEXT, prompt))
		{
			return false;
		}
		
		if (!setSUIProperty(pid, THIS, COUNTDOWNTIMER_TIMEVALUE, current_time + "," + total_time))
		{
			return false;
		}
		
		if (!flushSUIPage(pid))
		{
			return false;
		}
		
		return true;
	}
	
	
	public static boolean setPageProperties(int pid, dictionary params) throws InterruptedException
	{
		if ((params == null) || (params.isEmpty()))
		{
			setButtonStyle(pid, OK_ONLY);
			return true;
		}
		
		setPageTitle(pid, params);
		
		setPageButtons(pid, params);
		
		java.util.Enumeration keys = params.keys();
		while (keys.hasMoreElements())
		{
			testAbortScript();
			String key = (String)(keys.nextElement());
			if (key.indexOf('-') > -1)
			{
				String[] s = split(key, '-');
				if ((s != null) && (s.length == 2))
				{
					Object val = params.get(key);
					String sVal = null;
					
					if (val != null)
					{
						if (val instanceof string_id)
						{
							sVal = utils.packStringId((string_id)(val));
						}
						else if (val instanceof String)
						{
							sVal = (String)(val);
						}
						else
						{
							
						}
						
						if (sVal != null)
						{
							setSUIProperty(pid, s[0], s[1], sVal);
						}
					}
				}
			}
		}
		
		return true;
	}
	
	
	public static boolean setPageTitle(int pid, dictionary params) throws InterruptedException
	{
		if ((params == null) || (params.isEmpty()))
		{
			return false;
		}
		
		Object tmp = params.get(TITLE);
		String title = null;
		if (tmp != null)
		{
			if (tmp instanceof string_id)
			{
				title = utils.packStringId((string_id)(tmp));
			}
			else if (tmp instanceof String)
			{
				title = (String)(tmp);
			}
			
			if (title != null)
			{
				return setSUIProperty(pid, CTRL_TITLE, PROP_TEXT, DEFAULT_TITLE);
			}
			
			params.remove(TITLE);
		}
		
		return false;
	}
	
	
	public static boolean setPageButtons(int pid, dictionary params) throws InterruptedException
	{
		if ((params == null) || (params.isEmpty()))
		{
			return false;
		}
		
		int btns = params.getInt(BUTTONS);
		setButtonStyle(pid, btns);
		return true;
	}
	
	
	public static void setButtonStyle(int pid, int btns) throws InterruptedException
	{
		switch ( btns )
		{
			case OK_ONLY:
			hideBtnCancel(pid);
			hideBtnRevert(pid);
			break;
			
			case OK_CANCEL:
			hideBtnRevert(pid);
			break;
			
			case YES_NO:
			hideBtnRevert(pid);
			setSUIProperty(pid, BTN_CANCEL, PROP_TEXT,"@no");
			setSUIProperty(pid, BTN_OK, PROP_TEXT,"@yes");
			break;
			
			case YES_NO_CANCEL:
			setSUIProperty(pid, BTN_REVERT, PROP_TEXT,"@no");
			setSUIProperty(pid, BTN_CANCEL, PROP_TEXT,"@cancel");
			setSUIProperty(pid, BTN_OK, PROP_TEXT,"@yes");
			break;
			
			case YES_NO_MAYBE:
			setSUIProperty(pid, BTN_REVERT, PROP_TEXT,"@maybe");
			setSUIProperty(pid, BTN_CANCEL, PROP_TEXT,"@no");
			setSUIProperty(pid, BTN_OK, PROP_TEXT,"@yes");
			break;
			
			case YES_NO_ABSTAIN:
			setSUIProperty(pid, BTN_REVERT, PROP_TEXT,"@abstain");
			setSUIProperty(pid, BTN_CANCEL, PROP_TEXT,"@no");
			setSUIProperty(pid, BTN_OK, PROP_TEXT,"@yes");
			break;
			
			case RETRY_CANCEL:
			hideBtnRevert(pid);
			setSUIProperty(pid, BTN_CANCEL, PROP_TEXT,"@cancel");
			setSUIProperty(pid, BTN_OK, PROP_TEXT,"@retry");
			break;
			
			case ABORT_RETRY_CANCEL:
			setSUIProperty(pid, BTN_REVERT, PROP_TEXT,"@retry");
			setSUIProperty(pid, BTN_CANCEL, PROP_TEXT,"@cancel");
			setSUIProperty(pid, BTN_OK, PROP_TEXT,"@abort");
			break;
			
			case OK_REFRESH:
			hideBtnRevert(pid);
			setSUIProperty(pid, BTN_CANCEL, PROP_TEXT,"@refresh");
			break;
			
			case OK_CANCEL_REFRESH:
			setSUIProperty(pid, BTN_REVERT, PROP_TEXT,"@refresh");
			break;
			
			default:
			break;
		}
	}
	
	
	public static void hideBtnCancel(int pid) throws InterruptedException
	{
		setSUIProperty(pid, BTN_CANCEL, "Enabled","False");
		setSUIProperty(pid, BTN_CANCEL, "Visible","False");
	}
	
	
	public static void hideBtnRevert(int pid) throws InterruptedException
	{
		setSUIProperty(pid, BTN_REVERT, "Enabled","False");
		setSUIProperty(pid, BTN_REVERT, "Visible","False");
	}
	
	
	public static boolean trackSUI(obj_id target, int pid, dictionary params) throws InterruptedException
	{
		if (!isIdValid(target))
		{
			return false;
		}
		
		if ((params == null) || (params.isEmpty()))
		{
			trackSUI(target, pid, target, "noHandler");
			return false;
		}
		
		obj_id owner = params.getObjId(OWNER);
		if (!isIdValid(owner))
		{
			owner = target;
		}
		
		String handler = params.getString(HANDLER);
		if ((handler == null) || (handler.equals("")))
		{
			handler = "noHandler";
		}
		
		return trackSUI(target, pid, owner, handler);
	}
	
	
	public static boolean trackSUI(obj_id target, int pid, obj_id owner, String handler) throws InterruptedException
	{
		if (!isIdValid(target) || !isIdValid(owner) || (handler == null) || (handler.equals("")))
		{
			return false;
		}
		
		String path = "sui."+ pid;
		utils.setScriptVar(target, path + ".owner", owner);
		utils.setScriptVar(target, path + ".handler", handler);
		
		return true;
	}
	
	
	public static int msgbox(obj_id owner, obj_id target, String prompt, int buttons, String box_title, int style, String suiHandler) throws InterruptedException
	{
		if (!isIdValid(owner) || !isIdValid(target))
		{
			return -1;
		}
		
		if (prompt == null || prompt.equals(""))
		{
			return -1;
		}
		
		if (suiHandler == null || suiHandler.equals(""))
		{
			suiHandler = "noHandler";
		}
		
		if (box_title == null || box_title.equals(""))
		{
			box_title = DEFAULT_TITLE;
		}
		
		int pid = createSUIPage(SUI_MSGBOX, owner, target, suiHandler);
		
		setSUIProperty(pid, MSGBOX_PROMPT, PROP_TEXT, prompt);
		setSUIProperty(pid, MSGBOX_TITLE, PROP_TEXT, box_title);
		
		msgboxButtonSetup(pid, buttons);
		
		showSUIPage(pid);
		
		if (owner != target)
		{
			setAssociateRange(target, pid, owner);
		}
		
		return pid;
	}
	
	
	public static void msgboxButtonSetup(int pid, int buttons) throws InterruptedException
	{
		switch(buttons)
		{
			case OK_CANCEL:
			msgboxHideBtnLeft(pid);
			setSUIProperty(pid, MSGBOX_BTN_CANCEL, PROP_TEXT,"@cancel");
			setSUIProperty(pid, MSGBOX_BTN_OK, PROP_TEXT,"@ok");
			break;
			case YES_NO:
			msgboxHideBtnLeft(pid);
			setSUIProperty(pid, MSGBOX_BTN_CANCEL, PROP_TEXT,"@no");
			setSUIProperty(pid, MSGBOX_BTN_OK, PROP_TEXT,"@yes");
			break;
			case YES_NO_CANCEL:
			setSUIProperty(pid, MSGBOX_BTN_REVERT, PROP_TEXT,"@no");
			setSUIProperty(pid, MSGBOX_BTN_CANCEL, PROP_TEXT,"@cancel");
			setSUIProperty(pid, MSGBOX_BTN_OK, PROP_TEXT,"@yes");
			break;
			case YES_NO_MAYBE:
			setSUIProperty(pid, MSGBOX_BTN_REVERT, PROP_TEXT,"@maybe");
			setSUIProperty(pid, MSGBOX_BTN_CANCEL, PROP_TEXT,"@no");
			setSUIProperty(pid, MSGBOX_BTN_OK, PROP_TEXT,"@yes");
			break;
			case YES_NO_ABSTAIN:
			setSUIProperty(pid, MSGBOX_BTN_REVERT, PROP_TEXT,"@abstain");
			setSUIProperty(pid, MSGBOX_BTN_CANCEL, PROP_TEXT,"@no");
			setSUIProperty(pid, MSGBOX_BTN_OK, PROP_TEXT,"@yes");
			break;
			case RETRY_CANCEL:
			msgboxHideBtnLeft(pid);
			setSUIProperty(pid, MSGBOX_BTN_CANCEL, PROP_TEXT,"@cancel");
			setSUIProperty(pid, MSGBOX_BTN_OK, PROP_TEXT,"@retry");
			break;
			case ABORT_RETRY_CANCEL:
			setSUIProperty(pid, MSGBOX_BTN_REVERT, PROP_TEXT,"@retry");
			setSUIProperty(pid, MSGBOX_BTN_CANCEL, PROP_TEXT,"@cancel");
			setSUIProperty(pid, MSGBOX_BTN_OK, PROP_TEXT,"@abort");
			break;
			case OK_ONLY: 
			default:
			msgboxHideBtnLeft(pid);
			msgboxHideBtnMiddle(pid);
			setSUIProperty(pid, MSGBOX_BTN_OK, PROP_TEXT,"@ok");
			break;
		}
	}
	
	
	public static void msgboxHideBtnLeft(int pid) throws InterruptedException
	{
		setSUIProperty(pid, MSGBOX_BTN_REVERT, "Enabled","False");
		setSUIProperty(pid, MSGBOX_BTN_REVERT, "Visible","False");
	}
	
	
	public static void msgboxHideBtnMiddle(int pid) throws InterruptedException
	{
		setSUIProperty(pid, MSGBOX_BTN_CANCEL, "Enabled","False");
		setSUIProperty(pid, MSGBOX_BTN_CANCEL, "Visible","False");
	}
	
	
	public static int msgbox(obj_id owner, obj_id target, String prompt, String suiHandler) throws InterruptedException
	{
		return msgbox(owner, target, prompt, OK_ONLY, DEFAULT_TITLE, MSG_NORMAL, suiHandler);
	}
	
	
	public static int msgbox(obj_id owner, String prompt, String suiHandler) throws InterruptedException
	{
		return msgbox(owner, owner, prompt, OK_ONLY, DEFAULT_TITLE, MSG_NORMAL, suiHandler);
	}
	
	
	public static int msgbox(obj_id owner, obj_id target, String prompt) throws InterruptedException
	{
		return msgbox(owner, target, prompt, OK_ONLY, DEFAULT_TITLE, MSG_NORMAL, "noHandler");
	}
	
	
	public static int msgbox(obj_id owner, String prompt) throws InterruptedException
	{
		return msgbox(owner, owner, prompt, OK_ONLY, DEFAULT_TITLE, MSG_NORMAL, "noHandler");
	}
	
	
	public static int msgbox(obj_id owner, obj_id target, String prompt, int buttons, String suiHandler) throws InterruptedException
	{
		return msgbox(owner, target, prompt, buttons, DEFAULT_TITLE, MSG_NORMAL, suiHandler);
	}
	
	
	public static int msgbox(obj_id owner, obj_id target, String prompt, int buttons, String box_title, String suiHandler) throws InterruptedException
	{
		return msgbox(owner, target, prompt, buttons, box_title, MSG_NORMAL, suiHandler);
	}
	
	
	public static int msgbox(obj_id owner, obj_id target, prose_package prompt, int buttons, String box_title, int style, String suiHandler) throws InterruptedException
	{
		String oob = packOutOfBandProsePackage(null, prompt);
		return msgbox(owner, target, "\0" + oob, buttons, box_title, style, suiHandler);
	}
	
	
	public static int msgbox(obj_id owner, obj_id target, prose_package prompt, String suiHandler) throws InterruptedException
	{
		return msgbox(owner, target, prompt, OK_ONLY, DEFAULT_TITLE, MSG_NORMAL, suiHandler);
	}
	
	
	public static int msgbox(obj_id owner, prose_package prompt, String suiHandler) throws InterruptedException
	{
		return msgbox(owner, owner, prompt, OK_ONLY, DEFAULT_TITLE, MSG_NORMAL, suiHandler);
	}
	
	
	public static int msgbox(obj_id owner, obj_id target, prose_package prompt) throws InterruptedException
	{
		return msgbox(owner, target, prompt, OK_ONLY, DEFAULT_TITLE, MSG_NORMAL, "noHandler");
	}
	
	
	public static int msgbox(obj_id owner, prose_package prompt) throws InterruptedException
	{
		return msgbox(owner, owner, prompt, OK_ONLY, DEFAULT_TITLE, MSG_NORMAL, "noHandler");
	}
	
	
	public static int msgbox(obj_id owner, obj_id target, prose_package prompt, int buttons, String suiHandler) throws InterruptedException
	{
		return msgbox(owner, target, prompt, buttons, DEFAULT_TITLE, MSG_NORMAL, suiHandler);
	}
	
	
	public static int msgbox(obj_id owner, obj_id target, prose_package prompt, int buttons, String box_title, String suiHandler) throws InterruptedException
	{
		return msgbox(owner, target, prompt, buttons, box_title, MSG_NORMAL, suiHandler);
	}
	
	
	public static int inputbox(obj_id owner, obj_id target, String prompt, int buttons, String box_title, int style, String[] dsrc, String suiHandler, dictionary params) throws InterruptedException
	{
		if (dsrc == null)
		{
			LOG("sui", "inputbox: dsrc = null & style = "+ style);
		}
		
		if (!isIdValid(owner) || !isIdValid(target))
		{
			return -1;
		}
		
		if (prompt == null || prompt.equals(""))
		{
			return -1;
		}
		
		if (suiHandler == null || suiHandler.equals(""))
		{
			suiHandler = "noHandler";
		}
		
		if (box_title.equals(""))
		{
			box_title = DEFAULT_TITLE;
		}
		
		int pid = createSUIPage(SUI_INPUTBOX, owner, target, suiHandler);
		
		setSUIProperty(pid, INPUTBOX_PROMPT, PROP_TEXT, prompt);
		setSUIProperty(pid, INPUTBOX_TITLE, PROP_TEXT, box_title);
		
		inputboxButtonSetup(pid, buttons);
		inputboxStyleSetup(pid, style);
		
		if ((style > INPUT_NORMAL) && (dsrc != null))
		{
			LOG("sui", "sui.inputbox: entering data source modification");
			int maxArraySize = utils.getIntConfigSetting("GameServer", "suiListLimit");
			if (maxArraySize == 0)
			{
				CustomerServiceLog("sui", "WARNING: could not find config setting [GameServer] suiListLimit, "+
				"using default value of "+ MAX_ARRAY_SIZE);
				maxArraySize = MAX_ARRAY_SIZE;
			}
			else if (maxArraySize > MAX_ARRAY_SIZE)
			{
				CustomerServiceLog("sui", "WARNING: config setting [GameServer] suiListLimit = "+ maxArraySize +
				" greater than default value of "+ MAX_ARRAY_SIZE + ", please verify that the sui system can handle the value");
			}
			if (dsrc.length > maxArraySize)
			{
				LOG("sui", "inputbox: passed dsrc array too large!");
				
				prose_package pp = prose.getPackage(PROSE_OVER_MAX_ENTRIES, "", maxArraySize);
				sendSystemMessageProse(target, pp);
				
				String[] newSrc = new String[maxArraySize];
				for (int i = 0; i < maxArraySize; i++)
				{
					testAbortScript();
					newSrc[i] = dsrc[i];
				}
				
				dsrc = newSrc;
			}
			
			LOG("sui", "inputbox: clearing datasource dataInput");
			clearSUIDataSource(pid, INPUTBOX_DATASOURCE);
			
			int ubound = dsrc.length;
			for (int i = 0; i < ubound; i++)
			{
				testAbortScript();
				addSUIDataItem(pid, INPUTBOX_DATASOURCE, ""+ i);
				setSUIProperty(pid, INPUTBOX_DATASOURCE + "."+ i, PROP_TEXT, dsrc[i]);
				
			}
		}
		
		setSUIProperty(pid, INPUTBOX_INPUT, "MaxLength", String.valueOf(MAX_INPUT_LENGTH));
		setSUIProperty(pid, INPUTBOX_COMBO, "MaxLength", String.valueOf(MAX_INPUT_LENGTH));
		
		if ((params != null) && (!params.isEmpty()))
		{
			java.util.Enumeration keys = params.keys();
			while (keys.hasMoreElements())
			{
				testAbortScript();
				String key = (String)(keys.nextElement());
				String val = params.getString(key);
				
				LOG("sui","inputbox set params: key = "+ key + " val = "+ val);
				java.util.StringTokenizer st = new java.util.StringTokenizer(key,"-");
				if (st.countTokens() == 2)
				{
					String control = st.nextToken();
					String prop = st.nextToken();
					
					LOG("sui","control = "+ control + " prop = "+ prop + " val = "+ val);
					setSUIProperty(pid, control, prop, val);
				}
			}
		}
		
		subscribeToSUIProperty(pid, INPUTBOX_INPUT, PROP_LOCALTEXT);
		subscribeToSUIProperty(pid, INPUTBOX_COMBO, PROP_SELECTEDTEXT);
		
		showSUIPage(pid);
		
		if (owner != target)
		{
			setAssociateRange(target, pid, owner);
		}
		
		return pid;
	}
	
	
	public static int inputbox(obj_id owner, obj_id target, String prompt, String box_title, String suiHandler, int maxLength, boolean filtered, String defaultText) throws InterruptedException
	{
		if (!isIdValid(owner) || !isIdValid(target))
		{
			return -1;
		}
		
		if (prompt == null || prompt.equals(""))
		{
			return -1;
		}
		
		if (suiHandler == null || suiHandler.equals(""))
		{
			return -1;
		}
		
		String scriptvar_pid = VAR_FILTER_BASE + "."+ target + ".pid";
		String scriptvar_handler = VAR_FILTER_BASE + "."+ target + ".handler";
		
		if (utils.hasScriptVar(owner, scriptvar_pid))
		{
			int oldpid = utils.getIntScriptVar(owner, scriptvar_pid);
			if (oldpid > -1)
			{
				closeSUI(target, oldpid);
			}
			
			cleanupFilteredInputbox(owner, target);
		}
		
		dictionary params = null;
		
		if (maxLength < 1)
		{
			maxLength = MAX_INPUT_LENGTH;
		}
		else
		{
			params = new dictionary();
			params.put(INPUTBOX_INPUT + "-MaxLength", maxLength);
		}
		
		if (!filtered)
		{
			return inputbox(owner, target, prompt, OK_CANCEL, box_title, INPUT_NORMAL, null, suiHandler, params);
		}
		
		attachScript(owner, SCRIPT_FILTER_OWNER);
		
		int pid = createSUIPage(SUI_INPUTBOX, owner, target, HANDLER_FILTER_INPUT);
		if (pid > -1)
		{
			trackFilteredInputbox(owner, target, pid, suiHandler, defaultText);
			
			if (box_title.equals(""))
			{
				box_title = DEFAULT_TITLE;
			}
			
			setSUIProperty(pid, INPUTBOX_PROMPT, PROP_TEXT, prompt);
			setSUIProperty(pid, INPUTBOX_TITLE, PROP_TEXT, box_title);
			
			inputboxButtonSetup(pid, OK_CANCEL);
			inputboxStyleSetup(pid, INPUT_NORMAL);
			
			setSUIProperty(pid, INPUTBOX_INPUT, "MaxLength", String.valueOf(MAX_INPUT_LENGTH));
			
			if (defaultText != null && !defaultText.equals(""))
			{
				setSUIProperty(pid, INPUTBOX_INPUT, PROP_TEXT, defaultText);
			}
			
			subscribeToSUIProperty(pid, INPUTBOX_PROMPT, PROP_TEXT);
			subscribeToSUIProperty(pid, INPUTBOX_TITLE, PROP_TEXT);
			subscribeToSUIProperty(pid, INPUTBOX_INPUT, "MaxLength");
			
			subscribeToSUIProperty(pid, INPUTBOX_INPUT, PROP_LOCALTEXT);
			
			showSUIPage(pid);
			return pid;
		}
		
		return -1;
	}
	
	
	public static int inputbox(obj_id owner, obj_id target, String prompt, String box_title, String suiHandler, String defaultText) throws InterruptedException
	{
		return inputbox(owner, target, prompt, box_title, suiHandler, 32, true, defaultText);
	}
	
	
	public static int filteredInputbox(obj_id owner, obj_id target, String prompt, String box_title, String suiHandler, String defaultText) throws InterruptedException
	{
		return inputbox(owner, target, prompt, box_title, suiHandler, defaultText);
	}
	
	
	public static void trackFilteredInputbox(obj_id owner, obj_id player, int pid, String suiHandler, String defaultText) throws InterruptedException
	{
		if (!isIdValid(owner) || !isIdValid(player))
		{
			return;
		}
		
		String scriptvar_pid = VAR_FILTER_BASE + "."+ player + ".pid";
		String scriptvar_handler = VAR_FILTER_BASE + "."+ player + ".handler";
		String scriptvar_default = VAR_FILTER_BASE + "."+ player + ".defaultText";
		
		utils.setScriptVar(owner, scriptvar_pid, pid);
		utils.setScriptVar(owner, scriptvar_handler, suiHandler);
		
		if (defaultText != null && !defaultText.equals(""))
		{
			utils.setScriptVar(owner, scriptvar_default, defaultText);
		}
		
		if (!hasScript(owner, SCRIPT_FILTER_OWNER))
		{
			attachScript(owner, SCRIPT_FILTER_OWNER);
		}
		
		int cnt = utils.getIntScriptVar(owner, VAR_FILTER_COUNT);
		cnt++;
		
		utils.setScriptVar(owner, VAR_FILTER_COUNT, cnt);
	}
	
	
	public static void cleanupFilteredInputbox(obj_id owner, obj_id player) throws InterruptedException
	{
		if (!isIdValid(owner) || !isIdValid(player))
		{
			return;
		}
		
		String scriptvar_pid = VAR_FILTER_BASE + "."+ player + ".pid";
		String scriptvar_handler = VAR_FILTER_BASE + "."+ player + ".handler";
		String scriptvar_default = VAR_FILTER_BASE + "."+ player + ".defaultText";
		
		utils.removeScriptVar(owner, scriptvar_pid);
		utils.removeScriptVar(owner, scriptvar_handler);
		utils.removeScriptVar(owner, scriptvar_default);
		
		int cnt = utils.getIntScriptVar(owner, VAR_FILTER_COUNT);
		cnt--;
		
		if (cnt < 1)
		{
			utils.removeScriptVar(owner, VAR_FILTER_COUNT);
			detachScript(owner, SCRIPT_FILTER_OWNER);
		}
		else
		{
			utils.setScriptVar(owner, VAR_FILTER_COUNT, cnt);
		}
	}
	
	
	public static String getInputBoxText(dictionary params) throws InterruptedException
	{
		return params.getString(INPUTBOX_INPUT + "." + PROP_LOCALTEXT);
	}
	
	
	public static String getInputBoxTitle(dictionary params) throws InterruptedException
	{
		return params.getString(INPUTBOX_TITLE + "." + PROP_TEXT);
	}
	
	
	public static String getInputBoxPrompt(dictionary params) throws InterruptedException
	{
		return params.getString(INPUTBOX_PROMPT + "." + PROP_TEXT);
	}
	
	
	public static int getInputBoxMaxLength(dictionary params) throws InterruptedException
	{
		return utils.stringToInt(params.getString(INPUTBOX_INPUT + ".MaxLength"));
	}
	
	
	public static String getComboBoxText(dictionary params) throws InterruptedException
	{
		return params.getString(INPUTBOX_COMBO + "." + PROP_SELECTEDTEXT);
	}
	
	
	public static void inputboxButtonSetup(int pid, int buttons) throws InterruptedException
	{
		switch(buttons)
		{
			case OK_CANCEL:
			setSUIProperty(pid, INPUTBOX_BTN_CANCEL, PROP_TEXT,"@cancel");
			setSUIProperty(pid, INPUTBOX_BTN_OK, PROP_TEXT,"@ok");
			break;
			case OK_ONLY: 
			default:
			msgboxHideBtnMiddle(pid);
			setSUIProperty(pid, INPUTBOX_BTN_OK, PROP_TEXT,"@ok");
			break;
		}
	}
	
	
	public static void inputboxHideBtnMiddle(int pid) throws InterruptedException
	{
		setSUIProperty(pid, INPUTBOX_BTN_CANCEL, "Enabled","False");
		setSUIProperty(pid, INPUTBOX_BTN_CANCEL, "Visible","False");
	}
	
	
	public static void inputboxStyleSetup(int pid, int style) throws InterruptedException
	{
		switch(style)
		{
			case INPUT_COMBO:
			setSUIProperty(pid, INPUTBOX_INPUT, "Enabled","false");
			setSUIProperty(pid, INPUTBOX_INPUT, "Visible","false");
			
			setSUIProperty(pid, INPUTBOX_COMBO, "Enabled","true");
			setSUIProperty(pid, INPUTBOX_COMBO, "Visible","true");
			break;
			case INPUT_NORMAL: 
			default:
			setSUIProperty(pid, INPUTBOX_INPUT, "Enabled","true");
			setSUIProperty(pid, INPUTBOX_INPUT, "Visible","true");
			
			setSUIProperty(pid, INPUTBOX_COMBO, "Enabled","false");
			setSUIProperty(pid, INPUTBOX_COMBO, "Visible","false");
			break;
		}
	}
	
	
	public static int inputbox(obj_id owner, obj_id target, String prompt, int buttons, String box_title, int style, String[] dsrc, String suiHandler) throws InterruptedException
	{
		return inputbox(owner, target, prompt, buttons, box_title, style, dsrc, suiHandler, null);
	}
	
	
	public static int inputbox(obj_id owner, obj_id target, String prompt, String suiHandler) throws InterruptedException
	{
		return inputbox(owner, target, prompt, OK_ONLY, DEFAULT_TITLE, INPUT_NORMAL, null, suiHandler);
	}
	
	
	public static int inputbox(obj_id owner, String prompt, String suiHandler) throws InterruptedException
	{
		return inputbox(owner, owner, prompt, OK_ONLY, DEFAULT_TITLE, INPUT_NORMAL, null, suiHandler);
	}
	
	
	public static int inputbox(obj_id owner, obj_id target, String prompt) throws InterruptedException
	{
		return inputbox(owner, target, prompt, OK_ONLY, DEFAULT_TITLE, INPUT_NORMAL, null, "noHandler");
	}
	
	
	public static int inputbox(obj_id owner, String prompt) throws InterruptedException
	{
		return inputbox(owner, owner, prompt, OK_ONLY, DEFAULT_TITLE, INPUT_NORMAL, null, "noHandler");
	}
	
	
	public static int inputbox(obj_id owner, obj_id target, String prompt, String[] dsrc, String suiHandler) throws InterruptedException
	{
		return inputbox(owner, target, prompt, OK_ONLY, DEFAULT_TITLE, INPUT_COMBO, dsrc, suiHandler);
	}
	
	
	public static int combobox(obj_id owner, obj_id target, String prompt, String[] dsrc, String suiHandler) throws InterruptedException
	{
		return inputbox(owner, target, prompt, OK_ONLY, DEFAULT_TITLE, INPUT_COMBO, dsrc, suiHandler);
	}
	
	
	public static int combobox(obj_id owner, String prompt, String[] dsrc, String suiHandler) throws InterruptedException
	{
		return inputbox(owner, owner, prompt, OK_ONLY, DEFAULT_TITLE, INPUT_COMBO, dsrc, suiHandler);
	}
	
	
	public static int listbox(obj_id owner, obj_id target, String prompt, int buttons, String box_title, prose_package[] dsrc, String suiHandler, boolean show, boolean truncate) throws InterruptedException
	{
		if (!isIdValid(owner) || !isIdValid(target) || suiHandler == null || suiHandler.equals("") || dsrc == null || dsrc.length == 0)
		{
			if (!isIdValid(owner))
			{
				LOG("sui", "sui.listbox: invalid owner");
			}
			if (!isIdValid(target))
			{
				LOG("sui", "sui.listbox: invalid target");
			}
			if (suiHandler == null || suiHandler.equals(""))
			{
				LOG("sui", "sui.listbox: invalid suiHandler");
			}
			if (dsrc == null || dsrc.length == 0)
			{
				LOG("sui", "sui.listbox: invalid dsrc");
			}
			return -1;
		}
		
		if (box_title.equals(""))
		{
			box_title = DEFAULT_TITLE;
		}
		
		int maxArraySize = utils.getIntConfigSetting("GameServer", "suiListLimit");
		if (maxArraySize == 0)
		{
			CustomerServiceLog("sui", "WARNING: could not find config setting [GameServer] suiListLimit, "+
			"using default value of "+ MAX_ARRAY_SIZE);
			maxArraySize = MAX_ARRAY_SIZE;
		}
		else if (maxArraySize > MAX_ARRAY_SIZE)
		{
			CustomerServiceLog("sui", "WARNING: config setting [GameServer] suiListLimit = "+ maxArraySize +
			" greater than default value of "+ MAX_ARRAY_SIZE + ", please verify that the sui system can handle the value");
		}
		if (dsrc.length > maxArraySize && truncate)
		{
			LOG("sui", "sui.listbox: entering data source modification");
			if (dsrc.length > maxArraySize)
			{
				LOG("sui", "listbox: passed dsrc array too large!");
				
				prose_package pp = prose.getPackage(PROSE_OVER_MAX_ENTRIES, "", maxArraySize);
				sendSystemMessageProse(target, pp);
				
				prose_package[] newSrc = new prose_package[maxArraySize];
				for (int i = 0; i < maxArraySize; i++)
				{
					testAbortScript();
					newSrc[i] = dsrc[i];
				}
				
				dsrc = newSrc;
			}
		}
		
		int pid = createSUIPage(SUI_LISTBOX, owner, target, suiHandler);
		if (pid > -1)
		{
			setSUIProperty(pid, LISTBOX_TITLE, PROP_TEXT, box_title);
			setSUIProperty(pid, LISTBOX_PROMPT, PROP_TEXT, prompt);
			
			listboxButtonSetup(pid, buttons);
			
			if (dsrc != null)
			{
				clearSUIDataSource(pid, LISTBOX_DATASOURCE);
				
				int ubound = dsrc.length;
				for (int i = 0; i < ubound; i++)
				{
					testAbortScript();
					addSUIDataItem(pid, LISTBOX_DATASOURCE, ""+ i);
					String oob = "";
					oob = packOutOfBandProsePackage(oob, -1, dsrc[i]);
					setSUIPropertyOOB(pid, LISTBOX_DATASOURCE + "."+ i, PROP_TEXT, oob);
				}
			}
			
			subscribeToSUIProperty(pid, LISTBOX_LIST, PROP_SELECTEDROW);
			subscribeToSUIProperty(pid, LISTBOX_TITLE, PROP_TEXT);
			
			if (show)
			{
				showSUIPage(pid);
				
				if (owner != target)
				{
					setAssociateRange(target, pid, owner);
				}
			}
		}
		
		return pid;
	}
	
	
	public static int listbox(obj_id owner, obj_id target, String prompt, int buttons, String box_title, String[] dsrc, String suiHandler, boolean show, boolean truncate) throws InterruptedException
	{
		if (!isIdValid(owner) || !isIdValid(target) || suiHandler == null || suiHandler.equals("") || dsrc == null || dsrc.length == 0)
		{
			if (!isIdValid(owner))
			{
				LOG("sui", "sui.listbox: invalid owner");
			}
			if (!isIdValid(target))
			{
				LOG("sui", "sui.listbox: invalid target");
			}
			if (suiHandler == null || suiHandler.equals(""))
			{
				LOG("sui", "sui.listbox: invalid suiHandler");
			}
			if (dsrc == null || dsrc.length == 0)
			{
				LOG("sui", "sui.listbox: invalid dsrc");
			}
			return -1;
		}
		
		if (box_title.equals(""))
		{
			box_title = DEFAULT_TITLE;
		}
		
		int maxArraySize = utils.getIntConfigSetting("GameServer", "suiListLimit");
		if (maxArraySize == 0)
		{
			CustomerServiceLog("sui", "WARNING: could not find config setting [GameServer] suiListLimit, "+
			"using default value of "+ MAX_ARRAY_SIZE);
			maxArraySize = MAX_ARRAY_SIZE;
		}
		else if (maxArraySize > MAX_ARRAY_SIZE)
		{
			CustomerServiceLog("sui", "WARNING: config setting [GameServer] suiListLimit = "+ maxArraySize +
			" greater than default value of "+ MAX_ARRAY_SIZE + ", please verify that the sui system can handle the value");
		}
		if (dsrc.length > maxArraySize && truncate)
		{
			LOG("sui", "sui.listbox: entering data source modification");
			if (dsrc.length > maxArraySize)
			{
				LOG("sui", "listbox: passed dsrc array too large!");
				
				prose_package pp = prose.getPackage(PROSE_OVER_MAX_ENTRIES, "", maxArraySize);
				sendSystemMessageProse(target, pp);
				
				String[] newSrc = new String[maxArraySize];
				for (int i = 0; i < maxArraySize; i++)
				{
					testAbortScript();
					newSrc[i] = dsrc[i];
				}
				
				dsrc = newSrc;
			}
		}
		
		int pid = createSUIPage(SUI_LISTBOX, owner, target, suiHandler);
		if (pid > -1)
		{
			setSUIProperty(pid, LISTBOX_TITLE, PROP_TEXT, box_title);
			setSUIProperty(pid, LISTBOX_PROMPT, PROP_TEXT, prompt);
			
			listboxButtonSetup(pid, buttons);
			
			if (dsrc != null)
			{
				clearSUIDataSource(pid, LISTBOX_DATASOURCE);
				
				int ubound = dsrc.length;
				for (int i = 0; i < ubound; i++)
				{
					testAbortScript();
					addSUIDataItem(pid, LISTBOX_DATASOURCE, ""+ i);
					setSUIProperty(pid, LISTBOX_DATASOURCE + "."+ i, PROP_TEXT, dsrc[i]);
				}
			}
			
			subscribeToSUIProperty(pid, LISTBOX_LIST, PROP_SELECTEDROW);
			subscribeToSUIProperty(pid, LISTBOX_TITLE, PROP_TEXT);
			
			if (show)
			{
				showSUIPage(pid);
				
				if (owner != target)
				{
					setAssociateRange(target, pid, owner);
				}
			}
		}
		
		return pid;
	}
	
	
	public static int emptylistbox(obj_id owner, obj_id target, String prompt, int buttons, String box_title, String suiHandler, boolean show) throws InterruptedException
	{
		if (!isIdValid(owner) || !isIdValid(target) || suiHandler == null || suiHandler.equals(""))
		{
			if (!isIdValid(owner))
			{
				LOG("sui", "sui.emptylistbox: invalid owner");
			}
			if (!isIdValid(target))
			{
				LOG("sui", "sui.emptylistbox: invalid target");
			}
			if (suiHandler == null || suiHandler.equals(""))
			{
				LOG("sui", "sui.emptylistbox: invalid suiHandler");
			}
			return -1;
		}
		
		if (box_title.equals(""))
		{
			box_title = DEFAULT_TITLE;
		}
		
		int pid = createSUIPage(SUI_LISTBOX, owner, target, suiHandler);
		if (pid > -1)
		{
			setSUIProperty(pid, LISTBOX_TITLE, PROP_TEXT, box_title);
			setSUIProperty(pid, LISTBOX_PROMPT, PROP_TEXT, prompt);
			
			listboxButtonSetup(pid, buttons);
			
			clearSUIDataSource(pid, LISTBOX_DATASOURCE);
			
			subscribeToSUIProperty(pid, LISTBOX_LIST, PROP_SELECTEDROW);
			subscribeToSUIProperty(pid, LISTBOX_TITLE, PROP_TEXT);
			
			if (show)
			{
				showSUIPage(pid);
			}
		}
		
		return pid;
	}
	
	
	public static int getListboxSelectedRow(dictionary params) throws InterruptedException
	{
		return utils.stringToInt(params.getString(LISTBOX_LIST + "." + PROP_SELECTEDROW));
	}
	
	
	public static String getListboxTitle(dictionary params) throws InterruptedException
	{
		return params.getString(LISTBOX_TITLE + "." + PROP_TEXT);
	}
	
	
	public static void listboxButtonSetup(int pid, int buttons) throws InterruptedException
	{
		switch(buttons)
		{
			case OK_CANCEL:
			setSUIProperty(pid, LISTBOX_BTN_CANCEL, PROP_TEXT,"@cancel");
			setSUIProperty(pid, LISTBOX_BTN_OK, PROP_TEXT,"@ok");
			break;
			case YES_NO:
			setSUIProperty(pid, LISTBOX_BTN_CANCEL, PROP_TEXT,"@no");
			setSUIProperty(pid, LISTBOX_BTN_OK, PROP_TEXT,"@yes");
			break;
			case OK_REFRESH:
			setSUIProperty(pid, LISTBOX_BTN_CANCEL, PROP_TEXT,"@refresh");
			setSUIProperty(pid, LISTBOX_BTN_OK, PROP_TEXT,"@ok");
			break;
			case OK_CANCEL_REFRESH:
			listboxUseOtherButton(pid, "@refresh");
			setSUIProperty(pid, LISTBOX_BTN_CANCEL, PROP_TEXT,"@cancel");
			setSUIProperty(pid, LISTBOX_BTN_OK, PROP_TEXT,"@ok");
			break;
			case REFRESH_CANCEL:
			setSUIProperty(pid, LISTBOX_BTN_OK, PROP_TEXT,"@refresh");
			setSUIProperty(pid, LISTBOX_BTN_CANCEL, PROP_TEXT,"@cancel");
			break;
			case REFRESH_ONLY:
			listboxHideBtnMiddle(pid);
			setSUIProperty(pid, LISTBOX_BTN_OK, PROP_TEXT,"@refresh");
			break;
			case OK_CANCEL_ALL:
			listboxUseOtherButton(pid, "@all");
			setSUIProperty(pid, LISTBOX_BTN_CANCEL, PROP_TEXT,"@cancel");
			setSUIProperty(pid, LISTBOX_BTN_OK, PROP_TEXT,"@ok");
			break;
			case MOVEUP_MOVEDOWN_DONE:
			listboxUseOtherButton(pid, "@movedown");
			setSUIProperty(pid, LISTBOX_BTN_CANCEL, PROP_TEXT,"@done");
			setSUIProperty(pid, LISTBOX_BTN_OK, PROP_TEXT,"@moveup");
			break;
			case BET_MAX_BET_ONE_QUIT:
			listboxUseOtherButton(pid, "@bet_one");
			setSUIProperty(pid, LISTBOX_BTN_CANCEL, PROP_TEXT,"@quit");
			setSUIProperty(pid, LISTBOX_BTN_OK, PROP_TEXT,"@bet_max");
			break;
			case BET_MAX_BET_ONE_SPIN:
			listboxUseOtherButton(pid, "@bet_one");
			setSUIProperty(pid, LISTBOX_BTN_CANCEL, PROP_TEXT,"@spin");
			setSUIProperty(pid, LISTBOX_BTN_OK, PROP_TEXT,"@bet_max");
			break;
			case REFRESH_LEAVE_GAME:
			setSUIProperty(pid, LISTBOX_BTN_CANCEL, PROP_TEXT,"@leave_game");
			setSUIProperty(pid, LISTBOX_BTN_OK, PROP_TEXT,"@refresh");
			break;
			case REMOVE_CANCEL:
			setSUIProperty(pid, LISTBOX_BTN_CANCEL, PROP_TEXT,"@cancel");
			setSUIProperty(pid, LISTBOX_BTN_OK, PROP_TEXT,"@remove");
			break;
			case OK_ONLY: 
			default:
			msgboxHideBtnMiddle(pid);
			setSUIProperty(pid, LISTBOX_BTN_OK, PROP_TEXT,"@ok");
			break;
		}
	}
	
	
	public static void listboxHideBtnMiddle(int pid) throws InterruptedException
	{
		setSUIProperty(pid, LISTBOX_BTN_CANCEL, "Enabled","False");
		setSUIProperty(pid, LISTBOX_BTN_CANCEL, "Visible","False");
	}
	
	
	public static void listboxUseOtherButton(int pid, String caption) throws InterruptedException
	{
		setSUIProperty (pid, LISTBOX_BTN_OTHER, PROP_VISIBLE, "true");
		setSUIProperty (pid, LISTBOX_BTN_OTHER, PROP_TEXT, caption);
		subscribeToSUIProperty (pid, "this", LISTBOX_PROP_OTHER_PRESSED);
	}
	
	
	public static boolean getListboxOtherButtonPressed(dictionary params) throws InterruptedException
	{
		return Boolean.valueOf (params.getString("this." + LISTBOX_PROP_OTHER_PRESSED)).booleanValue ();
	}
	
	
	public static int listbox(obj_id owner, obj_id target, String prompt, int buttons, String box_title, String[] dsrc, String suiHandler, boolean show) throws InterruptedException
	{
		return listbox(owner, target, prompt, buttons, box_title, dsrc, suiHandler, show, true);
	}
	
	
	public static int listbox(obj_id owner, obj_id target, String prompt, int buttons, String box_title, Vector dsrc, String suiHandler, boolean show) throws InterruptedException
	{
		if (dsrc == null)
		{
			return -1;
		}
		
		String[] dsrcArray = new String[dsrc.size()];
		dsrc.toArray(dsrcArray);
		return listbox(owner, target, prompt, buttons, box_title, dsrcArray, suiHandler, show, true);
	}
	
	
	public static int listbox(obj_id owner, obj_id target, String prompt, int buttons, String box_title, Vector dsrc, String suiHandler, boolean show, boolean truncate) throws InterruptedException
	{
		if (dsrc == null)
		{
			return -1;
		}
		
		String[] dsrcArray = new String[dsrc.size()];
		dsrc.toArray(dsrcArray);
		return listbox(owner, target, prompt, buttons, box_title, dsrcArray, suiHandler, show, truncate);
	}
	
	
	public static int listbox(obj_id owner, obj_id target, String prompt, int buttons, String box_title, String[] dsrc, String suiHandler) throws InterruptedException
	{
		return listbox (owner, target, prompt, buttons, box_title, dsrc, suiHandler, true, true);
	}
	
	
	public static int listbox(obj_id owner, obj_id target, String prompt, int buttons, String box_title, Vector dsrc, String suiHandler) throws InterruptedException
	{
		if (dsrc == null)
		{
			return -1;
		}
		
		String[] dsrcArray = new String[dsrc.size()];
		dsrc.toArray(dsrcArray);
		return listbox (owner, target, prompt, buttons, box_title, dsrcArray, suiHandler, true, true);
	}
	
	
	public static int listbox(obj_id owner, obj_id target, String prompt, String[] dsrc, String suiHandler) throws InterruptedException
	{
		return listbox(owner, target, prompt, OK_ONLY, "", dsrc, suiHandler);
	}
	
	
	public static int listbox(obj_id owner, obj_id target, String prompt, Vector dsrc, String suiHandler) throws InterruptedException
	{
		if (dsrc == null)
		{
			return -1;
		}
		
		String[] dsrcArray = new String[dsrc.size()];
		dsrc.toArray(dsrcArray);
		return listbox(owner, target, prompt, OK_ONLY, "", dsrcArray, suiHandler);
	}
	
	
	public static int listbox(obj_id owner, String prompt, String[] dsrc, String suiHandler) throws InterruptedException
	{
		return listbox(owner, owner, prompt, OK_ONLY, "", dsrc, suiHandler);
	}
	
	
	public static int listbox(obj_id owner, String prompt, Vector dsrc, String suiHandler) throws InterruptedException
	{
		if (dsrc == null)
		{
			return -1;
		}
		
		String[] dsrcArray = new String[dsrc.size()];
		dsrc.toArray(dsrcArray);
		return listbox(owner, owner, prompt, OK_ONLY, "", dsrcArray, suiHandler);
	}
	
	
	public static int listbox(obj_id owner, obj_id target, String prompt, String[] dsrc) throws InterruptedException
	{
		return listbox(owner, target, prompt, OK_ONLY, "", dsrc, "noHandler");
	}
	
	
	public static int listbox(obj_id owner, obj_id target, String prompt, Vector dsrc) throws InterruptedException
	{
		if (dsrc == null)
		{
			return -1;
		}
		
		String[] dsrcArray = new String[dsrc.size()];
		dsrc.toArray(dsrcArray);
		return listbox(owner, target, prompt, OK_ONLY, "", dsrc, "noHandler");
	}
	
	
	public static int listbox(obj_id owner, String prompt, String[] dsrc) throws InterruptedException
	{
		return listbox(owner, owner, prompt, OK_ONLY, "", dsrc, "noHandler");
	}
	
	
	public static int listbox(obj_id owner, String prompt, Vector dsrc) throws InterruptedException
	{
		if (dsrc == null)
		{
			return -1;
		}
		
		String[] dsrcArray = new String[dsrc.size()];
		dsrc.toArray(dsrcArray);
		return listbox(owner, owner, prompt, OK_ONLY, "", dsrcArray, "noHandler");
	}
	
	
	public static int listbox(obj_id owner, obj_id target, String prompt, String title, String[] dsrc, String suiHandler) throws InterruptedException
	{
		return listbox(owner, target, prompt, OK_ONLY, title, dsrc, suiHandler);
	}
	
	
	public static int listbox(obj_id owner, obj_id target, String prompt, String title, Vector dsrc, String suiHandler) throws InterruptedException
	{
		if (dsrc == null)
		{
			return -1;
		}
		
		String[] dsrcArray = new String[dsrc.size()];
		dsrc.toArray(dsrcArray);
		return listbox(owner, target, prompt, OK_ONLY, title, dsrcArray, suiHandler);
	}
	
	
	public static int listbox(obj_id owner, String prompt, String title, String[] dsrc, String suiHandler) throws InterruptedException
	{
		return listbox(owner, owner, prompt, OK_ONLY, title, dsrc, suiHandler);
	}
	
	
	public static int listbox(obj_id owner, String prompt, String title, Vector dsrc, String suiHandler) throws InterruptedException
	{
		if (dsrc == null)
		{
			return -1;
		}
		
		String[] dsrcArray = new String[dsrc.size()];
		dsrc.toArray(dsrcArray);
		return listbox(owner, owner, prompt, OK_ONLY, title, dsrcArray, suiHandler);
	}
	
	
	public static int listbox(obj_id owner, obj_id target, String prompt, String title, String[] dsrc) throws InterruptedException
	{
		return listbox(owner, target, prompt, OK_ONLY, title, dsrc, "noHandler");
	}
	
	
	public static int listbox(obj_id owner, obj_id target, String prompt, String title, Vector dsrc) throws InterruptedException
	{
		if (dsrc == null)
		{
			return -1;
		}
		
		String[] dsrcArray = new String[dsrc.size()];
		dsrc.toArray(dsrcArray);
		return listbox(owner, target, prompt, OK_ONLY, title, dsrcArray, "noHandler");
	}
	
	
	public static int listbox(obj_id owner, String prompt, String title, String[] dsrc) throws InterruptedException
	{
		return listbox(owner, owner, prompt, OK_ONLY, title, dsrc, "noHandler");
	}
	
	
	public static int listbox(obj_id owner, String prompt, String title, Vector dsrc) throws InterruptedException
	{
		if (dsrc == null)
		{
			return -1;
		}
		
		String[] dsrcArray = new String[dsrc.size()];
		dsrc.toArray(dsrcArray);
		return listbox(owner, owner, prompt, OK_ONLY, title, dsrcArray, "noHandler");
	}
	
	
	public static int listbox(obj_id owner, String prompt, String title, int btn, String[] dsrc) throws InterruptedException
	{
		return listbox(owner, owner, prompt, btn, title, dsrc, "noHandler");
	}
	
	
	public static int listbox(obj_id owner, String prompt, String title, int btn, Vector dsrc) throws InterruptedException
	{
		if (dsrc == null)
		{
			return -1;
		}
		
		String[] dsrcArray = new String[dsrc.size()];
		dsrc.toArray(dsrcArray);
		return listbox(owner, owner, prompt, btn, title, dsrcArray, "noHandler");
	}
	
	
	public static int transfer(obj_id owner, obj_id target, String prompt, String box_title, String lblFrom, int inputFrom, String lblTo, int inputTo, String suiHandler, int conversionFrom, int conversionTo) throws InterruptedException
	{
		if ((owner == null) || (target == null) || (suiHandler.equals("")) || conversionFrom <= 0 || conversionTo <= 0)
		{
			return -1;
		}
		
		if (box_title.equals(""))
		{
			box_title = DEFAULT_TITLE;
		}
		
		int pid = createSUIPage(SUI_TRANSFER, owner, target, suiHandler);
		
		setSUIProperty(pid, TRANSFER_TITLE, PROP_TEXT, box_title);
		setSUIProperty(pid, TRANSFER_PROMPT, PROP_TEXT, prompt);
		
		setSUIProperty(pid, TRANSFER_FROM_TEXT, PROP_TEXT, lblFrom);
		setSUIProperty(pid, TRANSFER_TO_TEXT, PROP_TEXT, lblTo);
		
		setSUIProperty(pid, TRANSFER_FROM, PROP_TEXT, Integer.toString(inputFrom));
		setSUIProperty(pid, TRANSFER_TO, PROP_TEXT, Integer.toString(inputTo));
		
		setSUIProperty(pid, TRANSFER_INPUT_FROM, PROP_TEXT, Integer.toString(inputFrom));
		setSUIProperty(pid, TRANSFER_INPUT_TO, PROP_TEXT, Integer.toString(inputTo));
		
		setSUIProperty(pid, TRANSFER_PAGE_TRANSACTION, PROP_CONVERSION_RATIO_FROM, Integer.toString(conversionFrom));
		setSUIProperty(pid, TRANSFER_PAGE_TRANSACTION, PROP_CONVERSION_RATIO_TO, Integer.toString(conversionTo));
		
		subscribeToSUIProperty(pid, TRANSFER_INPUT_FROM, PROP_TEXT);
		subscribeToSUIProperty(pid, TRANSFER_INPUT_TO, PROP_TEXT);
		
		showSUIPage(pid);
		
		if (owner != target)
		{
			setAssociateRange(target, pid, owner);
		}
		
		return pid;
	}
	
	
	public static int transfer(obj_id owner, obj_id target, String prompt, String box_title, String lblFrom, int inputFrom, String lblTo, int inputTo, String suiHandler) throws InterruptedException
	{
		return transfer(owner, target, prompt, box_title, lblFrom, inputFrom, lblTo, inputTo, suiHandler, 1, 1);
	}
	
	
	public static int getTransferInputTo(dictionary params) throws InterruptedException
	{
		return utils.stringToInt(params.getString(TRANSFER_INPUT_TO + "." + PROP_TEXT));
	}
	
	
	public static int getTransferInputFrom(dictionary params) throws InterruptedException
	{
		return utils.stringToInt(params.getString(TRANSFER_INPUT_FROM + "." + PROP_TEXT));
	}
	
	
	public static int bank(obj_id player) throws InterruptedException
	{
		if (player == null)
		{
			return -1;
		}
		
		int cash = getCashBalance(player);
		int bank = getBankBalance(player);
		return transfer(player, player, MSG_BANK_PROMPT, MSG_BANK_TITLE, "Cash", cash, "Bank", bank, HANDLER_DEPOSIT_WITHDRAW);
	}
	
	
	public static int colorize(obj_id owner, obj_id player, obj_id target, String customizationVar, String handler) throws InterruptedException
	{
		if ((player == null) || (target == null) || (customizationVar.equals("")))
		{
			return -1;
		}
		
		customizationVar = customizationVar.trim();
		if (customizationVar.startsWith("/"))
		{
			customizationVar = customizationVar.substring(1);
		}
		
		LOG("sui", "colorize: customizationVar = "+ customizationVar + "<>");
		
		int pid = createSUIPage (SUI_COLORPICKER, owner, player, handler);
		if (pid > -1)
		{
			setSUIProperty (pid, COLORPICKER_COLORPICKER, PROP_TARGETID, target.toString());
			setSUIProperty (pid, COLORPICKER_COLORPICKER, PROP_TARGETVAR, customizationVar);
			setSUIProperty (pid, COLORPICKER_COLORPICKER, PROP_TARGETRANGEMAX, "500");
			
			setSUIProperty (pid, COLORPICKER_TITLE, PROP_TEXT, DEFAULT_TITLE);
			
			subscribeToSUIProperty (pid, COLORPICKER_COLORPICKER, PROP_SELECTEDINDEX);
			showSUIPage (pid);
		}
		
		return pid;
	}
	
	
	public static int getColorPickerIndex(dictionary params) throws InterruptedException
	{
		if (params == null || params.isEmpty())
		{
			return -1;
		}
		
		return utils.stringToInt(params.getString(COLORPICKER_COLORPICKER + "." + PROP_SELECTEDINDEX));
	}
	
	
	public static String getButtonPressed(dictionary params) throws InterruptedException
	{
		return params.getString(PROP_BUTTONPRESSED);
	}
	
	
	public static int getIntButtonPressed(dictionary params) throws InterruptedException
	{
		String bp = params.getString(PROP_BUTTONPRESSED);
		if ((bp == null) || (bp.equals("")))
		{
			
		}
		else if (bp.equals(OK))
		{
			if (params.containsKey("this.otherPressed"))
			{
				if (params.getString("this.otherPressed").equals("true"))
				{
					return BP_REVERT;
				}
			}
			
			return BP_OK;
		}
		else if (bp.equals(REVERT))
		{
			return BP_REVERT;
		}
		
		return BP_CANCEL;
	}
	
	
	public static obj_id getPlayerId(dictionary params) throws InterruptedException
	{
		return params.getObjId("player");
	}
	
	
	public static boolean setSUIObjVars(obj_id target, int pid, dictionary params) throws InterruptedException
	{
		java.util.Enumeration keys = params.keys();
		
		while (keys.hasMoreElements())
		{
			testAbortScript();
			Object key = keys.nextElement();
			Object value = params.get(key);
			
			if (value instanceof Integer)
			{
				int v = ((Integer)value).intValue();
				setObjVar(target, "sui."+ pid + "."+ key, v);
			}
			else if (value instanceof int[])
			{
				int[] v = (int[])value;
				if (v.length > 0)
				{
					setObjVar(target, "sui."+ pid + "."+ key, v);
				}
			}
			else if (value instanceof Float)
			{
				float v = ((Float)value).floatValue();
				setObjVar(target, "sui."+ pid + "."+ key, v);
			}
			else if (value instanceof float[])
			{
				float[] v = (float[])value;
				if (v.length > 0)
				{
					setObjVar(target, "sui."+ pid + "."+ key, v);
				}
			}
			else if (value instanceof String)
			{
				String v = (String)value;
				setObjVar(target, "sui."+ pid + "."+ key, v);
			}
			else if (value instanceof String[])
			{
				String[] v = (String[])value;
				if (v.length > 0)
				{
					setObjVar(target, "sui."+ pid + "."+ key, v);
				}
			}
			else if (value instanceof obj_id)
			{
				obj_id v = (obj_id)value;
				setObjVar(target, "sui."+ pid + "."+ key, v);
			}
			else if (value instanceof obj_id[])
			{
				obj_id[] v = (obj_id[])value;
				if (v.length > 0)
				{
					setObjVar(target, "sui."+ pid + "."+ key, v);
				}
			}
			else if (value instanceof location)
			{
				location v = (location)value;
				setObjVar(target, "sui."+ pid + "."+ key, v);
			}
			else if (value instanceof location[])
			{
				location[] v = (location[])value;
				if (v.length > 0)
				{
					setObjVar(target, "sui."+ pid + "."+ key, v);
				}
			}
			else if (value instanceof string_id)
			{
				string_id v = (string_id)value;
				setObjVar(target, "sui."+ pid + "."+ key, v);
			}
			else if (value instanceof string_id[])
			{
				string_id[] v = (string_id[])value;
				if (v.length > 0)
				{
					setObjVar(target, "sui."+ pid + "."+ key, v);
				}
			}
			
		}
		return true;
	}
	
	
	public static void clearSUIObjVar(obj_id target, int pid) throws InterruptedException
	{
		removeObjVar(target, "sui."+ pid);
	}
	
	
	public static void closeSUI(obj_id target, int pid) throws InterruptedException
	{
		if (!isIdValid(target))
		{
			return;
		}
		
		dictionary params = new dictionary();
		params.put("pid", pid);
		messageTo(target, "handleCloseSui", params, 0, false);
	}
	
	
	public static void setAssociateRange(obj_id player, int pid, obj_id target, float range) throws InterruptedException
	{
		if (!isIdValid(player))
		{
			LOG("sui", "sui.setAssociateRange: invalid player");
			return;
		}
		
		if (!isIdValid(target))
		{
			LOG("sui", "sui.setAssociateRange: invalid target");
			return;
		}
		
		if (pid < 0)
		{
			LOG("sui", "sui.setAssociateRange: pid < 0");
			return;
		}
		
		if (range <= 0f)
		{
			LOG("sui", "sui.setAssociateRange: invalid range <= 0f");
			return;
		}
		
		setSUIAssociatedObject(pid, target);
		setSUIMaxRangeToObject(pid, range);
	}
	
	
	public static void setAssociateRange(obj_id player, int pid, obj_id target) throws InterruptedException
	{
		setAssociateRange(player, pid, target, 16f);
	}
	
	
	public static void setAssociateRange(obj_id player, int pid, obj_id target, String key) throws InterruptedException
	{
		float range = 16f;
		
		String tbl = "datatables/sui/associate_range.iff";
		float dta = dataTableGetFloat(tbl, key, "RANGE");
		if (dta > 0f)
		{
			range = dta;
		}
		
		setAssociateRange(player, pid, target, range);
	}
	
	
	public static void setAutosaveProperty(int pid, boolean autosave) throws InterruptedException
	{
		setSUIProperty(pid, THIS, PROP_AUTOSAVE, autosave ? "true": "false");
	}
	
	
	public static void setSizeProperty(int pid, int width, int height) throws InterruptedException
	{
		if (width < 0)
		{
			LOG("sui", "setSizeProperty: Invalid size. width = "+ width);
			return;
		}
		
		if (height < 0)
		{
			LOG("sui", "setSizeProperty: Invalid size. height = "+ height);
			return;
		}
		
		setSUIProperty(pid, THIS, PROP_SIZE, width + ","+ height);
	}
	
	
	public static void setLocationProperty(int pid, int x, int y) throws InterruptedException
	{
		setSUIProperty(pid, THIS, PROP_LOCATION, x + ","+ y);
	}
	
	
	public static void setSoundProperty(int pid, String sound) throws InterruptedException
	{
		setSUIProperty(pid, THIS, PROP_SOUND, sound == null ? "": sound);
	}
	
	
	public static boolean hasPid(obj_id player, String pidName) throws InterruptedException
	{
		if (!isIdValid(player) || !exists(player))
		{
			return false;
		}
		
		if (pidName == null || pidName.equals(""))
		{
			return false;
		}
		
		return utils.hasScriptVar(player, "pid." + pidName);
	}
	
	
	public static void setPid(obj_id player, int pid, String pidName) throws InterruptedException
	{
		if (!isIdValid(player) || !exists(player))
		{
			return;
		}
		
		if (pidName == null || pidName.equals(""))
		{
			return;
		}
		
		if (pid == 0)
		{
			return;
		}
		
		utils.setScriptVar(player, "pid."+ pidName, pid);
	}
	
	
	public static void removePid(obj_id player, String pidName) throws InterruptedException
	{
		if (!isIdValid(player) || !exists(player))
		{
			return;
		}
		
		if (pidName == null || pidName.equals(""))
		{
			return;
		}
		
		utils.removeScriptVar(player, "pid."+ pidName);
	}
	
	
	public static int getPid(obj_id player, String pidName) throws InterruptedException
	{
		if (!isIdValid(player) || !exists(player))
		{
			return 0;
		}
		
		if (pidName == null || pidName.equals(""))
		{
			return 0;
		}
		
		return utils.getIntScriptVar(player, "pid." + pidName);
	}
	
	
	public static boolean outOfRange(obj_id object, obj_id player, float range, boolean message) throws InterruptedException
	{
		if (!isIdValid(object))
		{
			return false;
		}
		
		if (!isIdValid(player))
		{
			return false;
		}
		
		location a = getLocation(object);
		location b = getLocation(player);
		if (a.cell == b.cell && a.distance(b) < range)
		{
			return false;
		}
		
		if (message)
		{
			sendSystemMessage(player, OUT_OF_RANGE);
		}
		
		return true;
	}
	
	
	public static String colorGreen() throws InterruptedException
	{
		return "\\#99FF33";
	}
	
	
	public static String colorRed() throws InterruptedException
	{
		return "\\#FF3300";
	}
	
	
	public static String colorOrange() throws InterruptedException
	{
		return "\\#FFCC66";
	}
	
	
	public static String colorWhite() throws InterruptedException
	{
		return "\\#FFFFFF";
	}
	
	
	public static String colorBlue() throws InterruptedException
	{
		return "\\#0099FF";
	}
	
	
	public static String newLine() throws InterruptedException
	{
		return newLine(1);
	}
	
	
	public static String newLine(int n) throws InterruptedException
	{
		String returnString = "";
		for (int i = 0; i < n; i++)
		{
			testAbortScript();
			returnString += "\n";
		}
		
		return returnString;
	}
}
