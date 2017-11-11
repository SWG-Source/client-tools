// ======================================================================
//
// CuiConsoleHelper.cpp
// copyright (c) 2001 Sony Online Entertainment
//
// ======================================================================

#include "clientUserInterface/FirstClientUserInterface.h"
#include "clientUserInterface/CuiConsoleHelper.h"

#include "UIMessage.h"
#include "UIPage.h"
#include "UIText.h"
#include "UITextbox.h"
#include "UnicodeUtils.h"
#include "clientAudio/Audio.h"
#include "clientUserInterface/CuiAliasHandler.h"
#include "clientUserInterface/CuiConsoleHelperOutputGenerated.h"
#include "sharedCommandParser/CommandParser.h"
#include "sharedCommandParser/CommandParserHistory.h"
#include "sharedMessageDispatch/Transceiver.h"
#include "clientUserInterface/CuiSoundManager.h"
#include "clientUserInterface/CuiSounds.h"
#include "clientUserInterface/CuiSystemMessageManager.h"

#include <algorithm>
#include <list>

//#include "DejaLib.h"

// ======================================================================
/**
* pure virtual impl
*/

CuiConsoleHelperParserStrategy::~CuiConsoleHelperParserStrategy () {}

//-----------------------------------------------------------------

/**
* Predicate for searching for a command by fullname in the guard set
*/

struct StrEqualsNoCaseSet
{
	const CommandParser::String_t & str;

	explicit                StrEqualsNoCaseSet (const CommandParser::String_t & theStr) : str (theStr) {}

	bool                    operator() (const std::set<Unicode::String>::value_type & t) const
	{
		return !_wcsicmp (str.c_str (), t.c_str ());
	}

	                        StrEqualsNoCaseSet (const StrEqualsNoCaseSet & rhs) : str (rhs.str) {}

private:
	StrEqualsNoCaseSet &       operator= (const StrEqualsNoCaseSet & rhs); //lint !e754
	                        StrEqualsNoCaseSet ();
};

//-----------------------------------------------------------------

namespace CuiConsoleHelperNamespace
{
	namespace Transceivers
	{
		MessageDispatch::Transceiver<const CuiConsoleHelper::Messages::OutputGenerated::Payload &, CuiConsoleHelper::Messages::OutputGenerated> 
			outputGenerated;
	}

	typedef stdset<Unicode::String>::fwd        StringSet;
	typedef std::pair <float, Unicode::String>  PausedString;
	typedef std::pair <PausedString, StringSet> PausedStringState;
	typedef std::list<PausedStringState>        PausedCommands;

	PausedCommands s_pausedCommands;

	const Unicode::String s_pauseString = Unicode::narrowToWide("/pause ");
}

using namespace CuiConsoleHelperNamespace;

//----------------------------------------------------------------------

std::set<Unicode::String> CuiConsoleHelper::ms_currentCommandBeingParsed;
CuiConsoleHelper *CuiConsoleHelper::s_cuiConsoleHelper = 0;

//----------------------------------------------------------------------

class CuiConsoleHelper::AliasHandler :
public CuiAliasHandler
{
public:
	AliasHandler (CuiConsoleHelper & helper) : CuiAliasHandler (), m_helper (helper) {}

	int handleAlias (const Unicode::String & str, Unicode::String & )
	{
		if (!m_helper.processInput (str, CuiConsoleHelper::getRecurseStackForCommandBeingParsed()))
			return CommandParser::ERR_NO_HANDLER;

		return CommandParser::ERR_SUCCESS;
	}

private:
	AliasHandler (const AliasHandler& rhs);
	AliasHandler & operator= (const AliasHandler& rhs);

	CuiConsoleHelper & m_helper;
};

//----------------------------------------------------------------------

Unicode::unicode_char_t CuiConsoleHelper::ms_stopChar       = '.';
Unicode::unicode_char_t CuiConsoleHelper::ms_repeatChar     = '!';
Unicode::unicode_char_t CuiConsoleHelper::ms_substituteChar = '^';

//-----------------------------------------------------------------

CuiConsoleHelper::CuiConsoleHelper (UIPage & containerPage, UIText & text, UITextbox & textbox, const CuiConsoleHelperParserStrategy * strategy, CommandParserHistory * history) :
UIEventCallback          (), //lint !e1769
m_containerPage          (containerPage),
m_outputText             (text),
m_inputTextbox           (textbox),
m_curInputString         (),
m_inHistory              (false),
//m_doScroll				 (false),
m_maxOutputTextLines     (1500),
m_parserStrategy         (strategy),
m_history                (history),
m_echo                   (true),
m_alwaysProcessEscape    (false),
m_aliasHandler           (0),
m_escapeErasesInput      (true),
m_eatEnterMessageOnInput (true)
{
	m_containerPage.AddCallback  (this);
	m_outputText.SetPreLocalized (true);
	textbox.SetLocalText         (Unicode::emptyString);
	m_aliasHandler = new AliasHandler (*this);
	m_outputText.SetMaxLines (3000);
	m_outputText.SetMaximumCharacters (32768);
}

//-----------------------------------------------------------------

CuiConsoleHelper::~CuiConsoleHelper ()
{
	m_containerPage.RemoveCallback (this);
	m_parserStrategy = 0;
	m_history        = 0;

	delete m_aliasHandler;
	m_aliasHandler = 0;

	if(s_cuiConsoleHelper == this)
		s_cuiConsoleHelper = 0;
}

//-----------------------------------------------------------------

void CuiConsoleHelper::setMaxOutputTextLines (int newLines)
{
	m_maxOutputTextLines = newLines;
	m_outputText.SetMaxLines (newLines * 2);
}

//-----------------------------------------------------------------

void CuiConsoleHelper::OnTextboxChanged( UIWidget *)
{
	if (!m_inHistory)
		m_inputTextbox.GetLocalText (m_curInputString);
}


//-----------------------------------------------------------------

/**
* append the text, removing any unneccessary linebreaks
* @param result takes a copy because it must be modified internally
*/

int CuiConsoleHelper::appendOutputText (Unicode::String result, bool notify)
{
	//DEJA_CONTEXT("CuiConsoleHelper::appendOutputText");
	
	getAppendableText (m_outputText.GetLocalText(), result, true);
	
	if (notify)
	{
		const Messages::OutputGenerated::Payload payload (*this, result);
		Transceivers::outputGenerated.emitMessage (payload);
	}
	
	UIPoint oldScrollLocation = m_outputText.GetScrollLocation ();
	UISize  oldScrollExtent;
	m_outputText.GetScrollExtent (oldScrollExtent);

	const long oldScrollLocationToBottom = oldScrollLocation.y + m_outputText.GetHeight ();
	const bool wasAtBottom = (oldScrollLocationToBottom >= oldScrollExtent.y);

	m_outputText.AppendLocalText (result);

	//-----------------------------------------------------------------
	//-- clamp buffer len

	int			trimLength = 0;
	const int	outputTextLines  = m_outputText.GetLineCount ();

	if (outputTextLines > m_maxOutputTextLines)
	{
		UIPoint scrollExtentBeforeTrim;
		m_outputText.GetScrollExtent (scrollExtentBeforeTrim);

		m_outputText.RemoveLeadingLines (outputTextLines - m_maxOutputTextLines);

		UIPoint scrollExtentAfterTrim;
		m_outputText.GetScrollExtent (scrollExtentAfterTrim);

		trimLength = scrollExtentAfterTrim.y - scrollExtentBeforeTrim.y;
	}


	if (wasAtBottom)
	{
		scrollToBottom ();
	}
	else
	{
		oldScrollLocation.y += trimLength;
		m_outputText.ScrollToPoint (oldScrollLocation);
	}

	//setDoAppend(true);

	return trimLength;
}

//-----------------------------------------------------------------

/**
* Strips newlines from the end of @str, adds a newline to the beginning of @str
* if no newline exists at the end of @current.
*/
void CuiConsoleHelper::getAppendableText (const Unicode::String & current, Unicode::String & str, bool appendMode)
{
	if (str [str.length () - 1] == '\n' && !appendMode)
		str.erase (str.length () - 1);
	
	if (!current.empty())
	{
		if(current [current.length () - 1] != '\n')
		{
			str.insert (Unicode::String::size_type(0), Unicode::String::size_type(1), '\n');
		}
	}
	else if(current.empty() && appendMode)
	{
		// In append mode we want to add a leading newline
		str.insert (Unicode::String::size_type(0), Unicode::String::size_type(1), '\n');
	}
}

//-----------------------------------------------------------------

void CuiConsoleHelper::setOutputText (const Unicode::String & str)
{
	UIPoint oldScrollLocation = m_outputText.GetScrollLocation ();
	UISize  oldScrollExtent;
	m_outputText.GetScrollExtent (oldScrollExtent);

	const long oldScrollLocationToBottom = oldScrollLocation.y + m_outputText.GetHeight ();
	const bool wasAtBottom = (oldScrollLocationToBottom >= oldScrollExtent.y);

	m_outputText.SetLocalText (Unicode::String ());
	const int trimLength = appendOutputText (str, false);
	
	if (!wasAtBottom)
	{
		oldScrollLocation.y += trimLength;
		m_outputText.ScrollToPoint (oldScrollLocation);
	}
}

//----------------------------------------------------------------------

bool CuiConsoleHelper::processInput (const Unicode::String & istr, std::set<Unicode::String> & recursionCheckStack, bool addToHistory)
{
	bool retval = false;
	size_t pos = 0;
	size_t endpos = 0;
	UIString line;

	if (istr.empty ())
		return false;

	if (std::find_if (recursionCheckStack.begin (), recursionCheckStack.end (), StrEqualsNoCaseSet (istr)) != recursionCheckStack.end ())
	{
		CuiSystemMessageManager::sendFakeSystemMessage (Unicode::narrowToWide ("Alias recursion guard hit.  Stopping alias execution."));
		return false;
	}

	ms_currentCommandBeingParsed = recursionCheckStack;

	//add command as entered into the history
	if (m_history && addToHistory)
		m_history->push (istr);

	Unicode::String str = istr;

	static const Unicode::unicode_char_t separators [2] = { '\n', 0 };

	CommandParser::StringVector_t sv;

	bool pauseFound = false;
	Unicode::String pauseLine;
	float pauseTime = 0.0f;

	while (Unicode::getFirstToken (str, pos, endpos, line, separators))
	{
		sv.clear ();

		CommandParser::chopInputStrings (line, sv);

		for (CommandParser::StringVector_t::const_iterator it = sv.begin (); it != sv.end (); ++it)
		{
			const Unicode::String & cmd = *it;

			if (!pauseFound && Unicode::caseInsensitiveCompare (s_pauseString, cmd, 0, s_pauseString.size ()))
			{
				pauseFound = true;
				const Unicode::String & timeStr = cmd.substr(s_pauseString.size ());
				pauseTime = static_cast<float>(atof(Unicode::wideToNarrow(timeStr).c_str()));
				retval = true;
				continue;
			}

			if (pauseFound)
			{
				if (!pauseLine.empty () && pauseLine [pauseLine.size () - 1] != '\n')
					pauseLine.push_back  (';');
				pauseLine += cmd;
			}
			
			else if(!cmd.empty() && cmd != Unicode::narrowToWide(" "))
				retval = processInputLine (cmd) || retval;
		}

		if (pauseFound)
		{
			if (!pauseLine.empty ())
				pauseLine.push_back ('\n');
		}

		if (endpos == str.npos)
			break;

		pos = endpos + 1;
	}

	if (pauseFound)
	{
		if (!pauseLine.empty ())
		{

			const PausedString p1 (pauseTime, pauseLine);
			
			recursionCheckStack.insert(istr);

			const PausedStringState state (p1, recursionCheckStack);

			s_pausedCommands.push_back(state);
		}
	}

	return retval;
}

//-----------------------------------------------------------------

void CuiConsoleHelper::setInputText(const Unicode::String& input)
{
	m_inputTextbox.SetLocalText (input);
}

//----------------------------------------------------------------------

const Unicode::String &  CuiConsoleHelper::getInputText               () const
{
	return m_inputTextbox.GetLocalText ();
}

UIText&  CuiConsoleHelper::getOutputTextObject               () 
{
	return m_outputText;
}


//-----------------------------------------------------------------

/**
* process the current input line, usually as a result of an ENTER keypress
* 
* @return true if any input was processsed, false if the line was empty
*
*/
bool CuiConsoleHelper::processCurrentInput (bool addToHistory)
{
	UIString istr;
	m_inputTextbox.GetLocalText (istr);
	std::set<Unicode::String> s;
	const bool retval = processInput (istr, s, addToHistory);

	//-----------------------------------------------------------------
	//-- prepare the input box for the text input

	m_curInputString.erase ();
	m_inputTextbox.SetLocalText (m_curInputString);

	scrollToBottom ();

	return retval && m_eatEnterMessageOnInput;
}

//----------------------------------------------------------------------

/**
*
*/

bool CuiConsoleHelper::processInputLine (const Unicode::String & line)
{
	//DEJA_CONTEXT("CuiConsoleHelper::processInputLine");

	Unicode::String istr (Unicode::getTrim (line));

	m_inHistory = false;
	
	if (istr.empty ())
	{
		if (m_history)
			m_history->resetIndex ();
		return false;
	}

	Unicode::String result;

	//-----------------------------------------------------------------
	//-- echo raw command to output window

	if (m_echo)
	{
		result += Unicode::narrowToWide ("\\#ffffff > \\#888888") + istr + Unicode::narrowToWide ("\\#ffffff\n");
	}
		
	bool ok = true;

	if (m_parserStrategy)
	{
		//-----------------------------------------------------------------
		//-- handle command repeats ala ! symbol

		if (istr [0] == ms_repeatChar && m_history)
		{
			size_t argsPos = 1;
			Unicode::String token;

			//-- there is a regexp to repeat
			if (istr.length () > 1 && !Unicode::isWhitespace (istr [1]))
			{
				if (Unicode::getFirstToken (istr, 1, argsPos, token))
				{
					//-- match token in history
					if (!m_history->getHistoryCommandByAbbrev (token, token))
					{
						result += Unicode::narrowToWide (" :: Could not find history match for: '") + token;
						IGNORE_RETURN (result.append (1, '\''));
						IGNORE_RETURN (result.append (1, '\n'));
						ok = false;
					}
				}
				else
				{
					result += Unicode::narrowToWide (" :: Unable to parse repeat regexp.\n");
					ok = false;
				}
			}

			//-- just pick the last issued command
			else if (!m_history->front (token))
			{
				result += Unicode::narrowToWide (" :: Nothing in history.\n");
				ok = false;
			}

			//-- paste current arguments onto the command 
			if (ok)
			{
				if (argsPos < istr.length ())
					istr = istr.substr (argsPos);
				else
					istr.erase ();

				istr = token + istr;
			}
		}

		//-----------------------------------------------------------------
		//-- handle command substitution via ^ symbol

		else if (istr [0] == ms_substituteChar && m_history)
		{
			size_t argsPos = 1;
			Unicode::String token;
			Unicode::String lastCmd;

			//-----------------------------------------------------------------
			//-- retrieve the last issued command, if applicable
			if (!m_history->front (lastCmd))
			{
				result += Unicode::narrowToWide (" :: Nothing in history.\n");
				ok = false;
			}

			//-----------------------------------------------------------------
			//-- last command exists, try to substitute
			else
			{	
				//-- there is a regexp to substitute
				if (istr.length () > 1 && !Unicode::isWhitespace (istr [1]))
				{
					//-----------------------------------------------------------------
					//-- 

					if (Unicode::getFirstToken (istr, 1, argsPos, token))
					{
						Unicode::String secondArg;
						
						const size_t secondPos = token.find (ms_substituteChar);

						//-----------------------------------------------------------------
						//-- secondArg may be empty, which just erases instances of token in the string

						if (secondPos < token.length ())
						{
							if (secondPos < token.length () - 1)
								secondArg = token.substr (secondPos + 1);
							
							token = token.substr (0, secondPos);
						}
									
						//-----------------------------------------------------------------
						//-- perform the substitution
						
						const size_t tokenLen = token.length ();
						size_t findPos = 0;
						bool found = false;
						if (!token.empty ())
						{
							while (findPos < lastCmd.length () && (findPos = lastCmd.find (token, findPos)) < lastCmd.length ())
							{
								IGNORE_RETURN (lastCmd.replace (findPos, tokenLen, secondArg));
								findPos += secondArg.length ();
								found = true;
							}
						}
						
						//-----------------------------------------------------------------
						//-- if no instances of substitution token are found, this probably indicates user error
						
						if (!found)
						{
							result += Unicode::narrowToWide (" :: No instances of substitution token '") + token;
							result += Unicode::narrowToWide ("' found in last command '") + lastCmd;
							IGNORE_RETURN (result.append (1, '\''));
							IGNORE_RETURN (result.append (1, '\n'));
							ok = false;
						}

						//-----------------------------------------------------------------
						//-- process

						else
						{
							//-- remove the leading substitution code from istr
							
							if (argsPos < istr.length ())
								istr = istr.substr (argsPos);
							else
								istr.erase ();
							
							istr = lastCmd + istr;
						}
					}
					//-----------------------------------------------------------------
					//-- no regextp
					else
					{
						result += Unicode::narrowToWide (" :: Expected token(s) after substitution character.\n");
						ok = false;
					}
				}

				//-----------------------------------------------------------------
				//-- no regexp
				else
				{
					result += Unicode::narrowToWide (" :: Expected token(s) after substitution character.\n");
					ok = false;
				}
			}
		}

		//-----------------------------------------------------------------
		//-- if all the processing (if any) succeeded, attempt to parse the modified command

		if (ok)
		{
			if (!m_parserStrategy->parse (istr, result))
			{
				result += Unicode::narrowToWide ("CuiConsoleHelper::ParserStrategy parsing failed.\n");
			}
		}
	}
	else
	{
		result += Unicode::narrowToWide ("CuiConsoleHelper::ParserStrategy is null, no parsing available.\n");
	}
	
	//-----------------------------------------------------------------

	if (!result.empty ())
	{
		appendOutputText (result);
	}
	
	return true;
}

//-----------------------------------------------------------------

/**
* @return true if we delete the current input line, false if the line is already empty
*/

bool CuiConsoleHelper::processEscape ()
{
	//DEJA_CONTEXT("CuiConsoleHelper::processEscape");

	UIString istr;
	m_inputTextbox.GetLocalText (istr);
	
	bool retval = false;

	if (istr.empty ())
	{
		//- todo we need a callback in case the owner of the ConsoleHelper would like to deactivate
		//			deactivate ();
	}
	else
	{
		if (m_escapeErasesInput)
		{
			m_curInputString.erase ();
			m_inputTextbox.SetLocalText (m_curInputString);
			retval = true;
		}
	}
	
	if (m_history)
		m_history->resetIndex ();

	m_inHistory = false;
	
	return retval || m_alwaysProcessEscape;
}

//-----------------------------------------------------------------

bool CuiConsoleHelper::OnMessage( UIWidget *, const UIMessage & msg )
{
	//DEJA_CONTEXT("CuiConsoleHelper::OnMessage");

	if (msg.Type == UIMessage::Character)
		m_inputTextbox.SetFocus ();

	if (msg.IsUndoCommand ())
		m_inputTextbox.SetFocus ();

	if (msg.IsPasteCommand ())
	{
		m_outputText.ClearSelection ();
		m_inputTextbox.SetFocus ();
	}

	if (msg.Type != UIMessage::KeyDown )
		return true;
	
	//-----------------------------------------------------------------
	
	if (msg.Keystroke == UIMessage::UpArrow)
	{
		m_inputTextbox.SetFocus ();
		historyTraverse (-1);
	}
	
	//-----------------------------------------------------------------
	
	else if (msg.Keystroke == UIMessage::DownArrow)
	{
		m_inputTextbox.SetFocus ();
		historyTraverse (1);
		return false;
	}
	
	//-----------------------------------------------------------------
	
	else if (msg.Keystroke == UIMessage::Escape)
	{
		if (m_outputText.IsSelected ())
		{
			if (m_outputText.ClearSelection ())
			{
				m_inputTextbox.SetFocus ();
				return false;
			}
		}

		return !processEscape ();
	}
	
	//-----------------------------------------------------------------
	
	else if (msg.Keystroke == UIMessage::Enter)
	{
		m_inputTextbox.SetFocus ();

		return !processCurrentInput ();
	}
	
	//-----------------------------------------------------------------
	
	else if (msg.Keystroke == UIMessage::Tab)
	{
		m_inputTextbox.SetFocus ();

		if (msg.Modifiers.isAltDown ())
			return true;
		
		commandComplete ();

		return false;
		
	}
	
	//-----------------------------------------------------------------
	
	else if (msg.Keystroke == UIMessage::PageDown)
	{
		m_inputTextbox.SetFocus ();

		UISize   ScrollPageSize;
		UISize   ScrollLineSize;
		UIPoint  scrollLocation (m_outputText.GetScrollLocation ());
		m_outputText.GetScrollSizes( ScrollPageSize, ScrollLineSize );
		
		UISize   scrollExtent;
		m_outputText.GetScrollExtent (scrollExtent);
		
		long min_y =  scrollExtent.y - ScrollPageSize.y;
		
		if (scrollLocation.y == min_y)
		{
			CuiSoundManager::play (CuiSounds::negative);
		}
		else
		{
			scrollLocation.y += ScrollPageSize.y;
			scrollLocation.y = std::min (min_y, scrollLocation.y);
					
			m_outputText.ScrollToPoint ( scrollLocation );
		}
		return false;
	}
	
	//-----------------------------------------------------------------
	
	else if (msg.Keystroke == UIMessage::PageUp)
	{
		m_inputTextbox.SetFocus ();

		UISize   ScrollPageSize;
		UISize   ScrollLineSize;
		UIPoint  scrollLocation (m_outputText.GetScrollLocation ());
		
		if (scrollLocation.y == 0)
		{
			CuiSoundManager::play (CuiSounds::negative);
		}
		else
		{
			m_outputText.GetScrollSizes( ScrollPageSize, ScrollLineSize );		
			scrollLocation.y -= ScrollPageSize.y;
			scrollLocation.y = std::max ( static_cast<long>(0), scrollLocation.y);
			m_outputText.SetScrollLocation( scrollLocation );
		}
		
		return false;
	}
	
	return true;
	
}

//----------------------------------------------------------------------

void CuiConsoleHelper::setAlwaysProcessEscape (bool b)
{
	m_alwaysProcessEscape = b;
}

//----------------------------------------------------------------------

int CuiConsoleHelper::getScrolledToCharacter    (bool & isAtEnd) const
{
	//DEJA_CONTEXT("CuiConsoleHelper::getScrolledToCharacter");

	const UIPoint & scrollLocation = m_outputText.GetScrollLocation ();
	const long scrollToVertical = scrollLocation.y + m_outputText.GetHeight ();

	UISize scrollExtent;
	m_outputText.GetScrollExtent (scrollExtent);

	if (scrollToVertical >= scrollExtent.y)
		isAtEnd = true;

	const int c =    m_outputText.GetCharacterFromLocation (scrollLocation);
	return c;
}

//----------------------------------------------------------------------

void CuiConsoleHelper::scrollToCharacter (int c)
{
	//DEJA_CONTEXT("CuiConsoleHelper::scrollToCharacter");

	const UIPoint & pt = m_outputText.FindCaratPos (c);
	m_outputText.ScrollToPoint (pt);
}

//----------------------------------------------------------------------

void CuiConsoleHelper::scrollToBottom ()
{
	//DEJA_CONTEXT("CuiConsoleHelper::scrollToBottom");

	m_outputText.SetTextFlag (UIText::TF_dirty, true);
	m_outputText.ScrollToBottom ();
}

//----------------------------------------------------------------------

void CuiConsoleHelper::setEcho (bool b)
{
	m_echo = b;
}

//----------------------------------------------------------------------

CuiAliasHandler & CuiConsoleHelper::getAliasHandler ()
{
	return *m_aliasHandler;
}

//----------------------------------------------------------------------

void CuiConsoleHelper::setEscapeErasesInput (bool b)
{
	m_escapeErasesInput = b;
}

//----------------------------------------------------------------------

void CuiConsoleHelper::setEatEnterMessageOnInput  (bool b)
{
	m_eatEnterMessageOnInput = b;
}

//----------------------------------------------------------------------

void CuiConsoleHelper::cursorMove                 (int distance, bool words)
{
	if (distance > 0)
	{
		while (distance--)
		{
			if (words)
				m_inputTextbox.MoveCaratRightOneWord ();
			else
				m_inputTextbox.MoveCaratRight ();
		}
	}
	else if (distance < 0)
	{
		while (distance++)
		{
			if (words)
				m_inputTextbox.MoveCaratLeftOneWord ();
			else
				m_inputTextbox.MoveCaratLeft ();
		}
	}
}

//----------------------------------------------------------------------

void CuiConsoleHelper::cursorGotoEnd              (int direction)
{
	if (direction > 0)
		m_inputTextbox.MoveCaratToEndOfLine ();
	else if (direction < 0)
		m_inputTextbox.MoveCaratToStartOfLine ();
}

//----------------------------------------------------------------------

void CuiConsoleHelper::historyTraverse            (int distance)
{
	UIString s;
	
	bool ok = false;

	if (m_history == 0)
		return;
	
	if (distance > 0)
	{
		while (distance--)
		{
			if (m_history->forward (s))
			{
				m_inHistory = true;
				ok = true;
			}
			else if (m_inHistory)
			{
				m_inHistory = false;
				s = m_curInputString;
				ok = true;
				break;
			}
			else
			{
				CuiSoundManager::play (CuiSounds::negative);
				break;
			}
		}
	}
	else if (distance < 0)
	{		
		while (distance++)
		{
			if (m_history->back (s))
			{
				m_inHistory = true;
				ok = true;
			}
			else
			{
				CuiSoundManager::play (CuiSounds::negative);
				break;
			}
		}
	}

	if (ok)
	{
		m_inputTextbox.SetLocalText (s);
		m_inputTextbox.MoveCaratToEndOfLine();
		CuiSoundManager::play (CuiSounds::increment_big);
	}
}

//----------------------------------------------------------------------

void CuiConsoleHelper::commandComplete            ()
{
	if (!m_parserStrategy)
	{
		// @TODO: make this message lookup from string table
		appendOutputText ( Unicode::narrowToWide (" :: No Console Parser available!\n"));
		return;
	}
	
	UIString istr;
	m_inputTextbox.GetLocalText (istr);
	
	CommandParser::StringVector_t resultList;
	size_t token_start;
	size_t token_end;
	
	const size_t caratPos = static_cast<size_t>(m_inputTextbox.GetCaratIndex ());
	
	if (!m_parserStrategy->tabCompleteToken (istr, caratPos, resultList, token_start, token_end))
		return;
	
	if (resultList.size () == 1)
	{
		UIString token = resultList.front ();
		
		IGNORE_RETURN (istr.replace (token_start, token_end-token_start, token));
		
		// put a space at the end if necessary
		if (istr.length () <= (token_start + token.length ()))
		{
			IGNORE_RETURN (istr.append (1, ' '));
		}
		
		m_inputTextbox.SetLocalText (istr);
		
		// move the carat to one-space-past the token
		size_t diff = token_start + token.length () + 1 - caratPos;
		
		while (diff--)
			m_inputTextbox.MoveCaratRight();
	}
	else if (resultList.size () == 0)
	{
		// @TODO: make this message lookup from string table
		const Unicode::String outputStr (Unicode::narrowToWide (" :: No matches.\n"));
		appendOutputText (outputStr);
		
		CuiSoundManager::play (CuiSounds::negative);
	}
	else
	{
		CuiSoundManager::play (CuiSounds::increment_small);
		
		// @TODO: make this message lookup from string table
		Unicode::String output (Unicode::narrowToWide (" :: Possible completions:"));				
		Unicode::String commonStr;  // the substring that is common among all the choices
		const Unicode::String textColor   (Unicode::narrowToWide ("\\#aaffff"));
		
		bool found = false;
		
		for (CommandParser::StringVector_t::const_iterator iter = resultList.begin (); iter != resultList.end (); ++iter)
		{
			const Unicode::String & str = *iter;
			
			IGNORE_RETURN (output.append (1, '\n'));
			IGNORE_RETURN (output.append (8, ' '));
			output += textColor + str;
			
			//-- see how much of this string is in common with the others
			if (!found)
			{
				commonStr = str;
				found = true;
			}
			else
			{
				const size_t size = std::min (commonStr.size (), str.size ());
				for (size_t i = 0;i < size; ++i)
				{
					if (tolower (commonStr [i]) != tolower (str [i]))
					{
						commonStr = commonStr.substr (0, i);
						break;
					}
				}
			}
		}
		
		if (!commonStr.empty ())
		{
			IGNORE_RETURN (istr.replace (token_start, token_end-token_start, commonStr));
			m_inputTextbox.SetLocalText (istr);
			
			// move the carat to one-space-past the token
			size_t diff = token_start + commonStr.length () + 1 - caratPos;
			
			while (diff--)
				m_inputTextbox.MoveCaratRight();
		}
		
		output += Unicode::narrowToWide ("\\#ffffff");
		
		appendOutputText (output);
	}
}

//----------------------------------------------------------------------

void CuiConsoleHelper::performBackspaceKey ()
{
	m_inputTextbox.performBackspaceKey (true, false);
}

//----------------------------------------------------------------------

void CuiConsoleHelper::performDeleteKey ()
{
	m_inputTextbox.performDeleteKey (true, false);
}

//----------------------------------------------------------------------

void CuiConsoleHelper::update (float deltaTimeSecs)
{
	//DEJA_CONTEXT("CuiConsoleHelper::update");
	
	if(s_cuiConsoleHelper)
	{
		for (PausedCommands::iterator it = s_pausedCommands.begin(); it != s_pausedCommands.end(); ++it)
		{
			PausedStringState & state = (*it);
			PausedString      & pausedString = state.first;
			StringSet         & recursionSet = state.second;

			pausedString.first -= deltaTimeSecs;
			if (pausedString.first <= 0)
			{
				s_cuiConsoleHelper->processInput (pausedString.second, recursionSet, false);
				it = s_pausedCommands.erase (it);
			}
		}
	}
}

//----------------------------------------------------------------------

std::set<Unicode::String> & CuiConsoleHelper::getRecurseStackForCommandBeingParsed()
{
	return ms_currentCommandBeingParsed;
}

//----------------------------------------------------------------------

int CuiConsoleHelper::dumpPausedCommands ()
{
	const int size = s_pausedCommands.size ();
	s_pausedCommands.clear ();
	return size;
}

// ======================================================================
