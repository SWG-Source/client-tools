// ======================================================================
//
// CuiConsoleHelper.h
// copyright (c) 2001 Sony Online Entertainment
//
// ======================================================================

#ifndef INCLUDED_CuiConsoleHelper_H
#define INCLUDED_CuiConsoleHelper_H

// ======================================================================

#include "UIEventCallback.h"

class UIText;
class UITextbox;
class UIPage;
class CommandParserHistory;
class CuiConsoleHelperOutputGenerated;
class CuiAliasHandler;

//-----------------------------------------------------------------

/**
* An interface to a pre-processing command parser.  The Strategy might perform analysis and
* interpretation of the command string that is irrelevant to the actual CommandParsers.
* For instance, a ConsoleHelperParserStrategy might choose between one of several CommandParsers
* based on the contents of the command string.
*
* A CuiConsoleHelper must have a CuiConsoleHelperParserStrategy to function.
*/

class CuiConsoleHelperParserStrategy
{
public:
	virtual bool parse            (const Unicode::String & str, Unicode::String & result) const = 0;
	virtual bool tabCompleteToken (const Unicode::String & str, size_t tokenPos, stdvector<Unicode::String>::fwd & results, size_t & token_start, size_t & token_end) const = 0;
	virtual ~CuiConsoleHelperParserStrategy () = 0;
};

//-----------------------------------------------------------------

/**
* CuiConsoleHelper provides the bulk of the advanced console functionality.
* It provides command substitution, history storage and manipulation, tab completion,
* scrollback buffer management, and others.
*
* Any UIText-UITextbox pair can become a fully functional console by using a CuiConsoleHelper,
* in conjunction with a CuiConsoleHelperParserStrategy.
*/

class CuiConsoleHelper : 
public UIEventCallback
{
public:

	struct Messages
	{
		struct OutputGenerated
		{
			typedef CuiConsoleHelperOutputGenerated Payload;
		};
	};

	//----------------------------------------------------------------------

	
	                         CuiConsoleHelper      (UIPage & containerPage, UIText & text, UITextbox & textbox, const CuiConsoleHelperParserStrategy * ps, CommandParserHistory * history);

	                        ~CuiConsoleHelper      ();
	void                     setMaxOutputTextLines (int newLines);
	int                      getMaxOutputTextLines () const;

	static void              update                (float deltaTimeSecs);

	bool                     processInput          (const Unicode::String & istr, stdset<Unicode::String>::fwd & recursionCheckStack, bool addToHistory = true);
	bool                     processCurrentInput   (bool addToHistory = true);
	bool                     processEscape         ();
	int                      appendOutputText      (Unicode::String result, bool notify = true);
	static void              getAppendableText     (const Unicode::String & current, Unicode::String & str, bool appendMode = false);
	void                     setOutputText         (const Unicode::String & str);


	virtual bool             OnMessage             (UIWidget *context, const UIMessage & msg);
	virtual void             OnTextboxChanged      (UIWidget *context);

	bool                     getEcho               () const;
	void                     setEcho               (bool b);

	static Unicode::unicode_char_t    getStopChar       ();
	static Unicode::unicode_char_t    getRepeatChar     ();
	static Unicode::unicode_char_t    getSubstituteChar ();

	void                     setAlwaysProcessEscape     (bool b);

	int                      getScrolledToCharacter     (bool & isAtEnd) const;
	void                     scrollToCharacter          (int c);
	void                     scrollToBottom             ();

	CuiAliasHandler &        getAliasHandler            ();

	void                     setEscapeErasesInput       (bool b);
	void                     setEatEnterMessageOnInput  (bool b);

	void                     setInputText               (const Unicode::String& input);
	const Unicode::String &  getInputText               () const;

	UIText&					 getOutputTextObject		();
	
	void                     cursorMove                 (int distance, bool words);
	void                     cursorGotoEnd              (int direction);
	void                     historyTraverse            (int distance);
	void                     commandComplete            ();
	void                     performBackspaceKey        ();
	void                     performDeleteKey           ();

	static int               dumpPausedCommands         ();

	static void              setActiveConsoleHelper     (CuiConsoleHelper * consoleHelper);
	static CuiConsoleHelper *getActiveConsoleHelper     (void);

public:
	static stdset<Unicode::String>::fwd & getRecurseStackForCommandBeingParsed();

private:
	                         CuiConsoleHelper ();
	                         CuiConsoleHelper (const CuiConsoleHelper & rhs);
	CuiConsoleHelper &       operator=        (const CuiConsoleHelper & rhs);
	
private:
	
	bool                     processInputLine (const Unicode::String & line);

	UIPage &                 m_containerPage;
	UIText &                 m_outputText;
	UITextbox &              m_inputTextbox;
	
	UIString                 m_curInputString;
	bool                     m_inHistory;
	
	int                      m_maxOutputTextLines;

	const CuiConsoleHelperParserStrategy *   m_parserStrategy;

	CommandParserHistory *   m_history;

	static Unicode::unicode_char_t    ms_stopChar;
	static Unicode::unicode_char_t    ms_repeatChar;
	static Unicode::unicode_char_t    ms_substituteChar;


	bool                              m_echo;
	bool                              m_alwaysProcessEscape;

	class AliasHandler;
	friend class AliasHandler;
	AliasHandler *                    m_aliasHandler;

	bool                              m_escapeErasesInput;
	bool                              m_eatEnterMessageOnInput;

	static CuiConsoleHelper *         s_cuiConsoleHelper;

private:
	static stdset<Unicode::String>::fwd ms_currentCommandBeingParsed;
};

//-----------------------------------------------------------------

inline int CuiConsoleHelper::getMaxOutputTextLines (void) const
{
	return m_maxOutputTextLines;
}

//-----------------------------------------------------------------

inline bool CuiConsoleHelper::getEcho () const
{
	return m_echo;
}

//-----------------------------------------------------------------

inline void CuiConsoleHelper::setActiveConsoleHelper(CuiConsoleHelper * consoleHelper)
{
	s_cuiConsoleHelper = consoleHelper;
}

//-----------------------------------------------------------------

inline CuiConsoleHelper *CuiConsoleHelper::getActiveConsoleHelper(void)
{
	return s_cuiConsoleHelper;
}

// ======================================================================

#endif
