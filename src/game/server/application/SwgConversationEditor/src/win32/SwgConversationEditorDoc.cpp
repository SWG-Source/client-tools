// ======================================================================
//
// SwgConversationEditorDoc.cpp
// asommers
//
// copyright 2003, sony online entertainment
//
// ======================================================================

#include "FirstSwgConversationEditor.h"
#include "SwgConversationEditorDoc.h"

#include "Configuration.h"
#include "Conversation.h"
#include "DialogFindText.h"
#include "ScriptShellFrame.h"
#include "RecentDirectory.h"
#include "ScriptGroup.h"
#include "sharedFoundation/ConfigFile.h"
#include "sharedUtility/FileName.h"
#include "StringIdTracker.h"
#include "StringTable.h"
#include "SwgConversationEditor.h"
#include "WarningFrame.h"

#include <string>

// ======================================================================

namespace SwgConversationEditorDocNamespace
{
	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

	CString const cs_conditionDefaultName = "_defaultCondition";
	CString const cs_actionDefaultName = "_defaultAction";
	CString const cs_tokenDefaultName = "_none";
	CString const cs_labelDefaultName = "_none";
	CString const cs_labelRootName = "_root";

	CString const cs_scriptPreNameDeclarationConditionString = "boolean ";
	CString const cs_scriptPostNameDeclarationConditionString = " (obj_id player, obj_id npc)\r\n";
	CString const cs_scriptBodyConditionString = "{\r\n\treturn true;\r\n}\r\n";
	CString const cs_scriptPreNameDeclarationActionString = "void ";
	CString const cs_scriptPostNameDeclarationActionString = " (obj_id player, obj_id npc)\r\n";
	CString const cs_scriptBodyActionString = "{\r\n}\r\n";
	CString const cs_scriptPreNameDeclarationTokenTOString = "string ";
	CString const cs_scriptPostNameDeclarationTokenTOString = " (obj_id player, obj_id npc)\r\n";
	CString const cs_scriptBodyTokenTOString = "{\r\n\treturn new string();\r\n}\r\n";
	CString const cs_scriptPreNameDeclarationTokenDIString = "int ";
	CString const cs_scriptPostNameDeclarationTokenDIString = " (obj_id player, obj_id npc)\r\n";
	CString const cs_scriptBodyTokenDIString = "{\r\n\treturn 0;\r\n}\r\n";
	CString const cs_scriptPreNameDeclarationTokenDFString = "float ";
	CString const cs_scriptPostNameDeclarationTokenDFString = " (obj_id player, obj_id npc)\r\n";
	CString const cs_scriptBodyTokenDFString = "{\r\n\treturn 0.f;\r\n}\r\n";

	CString const cs_scriptHeader1 = "// ======================================================================\n//\n// ";
	CString const cs_scriptHeader2 = "\n// Copyright 2004, Sony Online Entertainment\n// All Rights Reserved.\n//\n// Created with " + SwgConversationEditorApp::getVersion() + " - DO NOT EDIT THIS AUTO-GENERATED FILE!\n//\n// ======================================================================\n\n";

	CString const cs_scriptLibraryDelimiter = "// ======================================================================\n// Library Includes\n// ======================================================================\n\n";
	CString const cs_scriptConstantsDelimiter = "// ======================================================================\n// Script Constants\n// ======================================================================\n\n";
	CString const cs_scriptConditionDelimiter = "// ======================================================================\n// Script Conditions\n// ======================================================================\n\n";
	CString const cs_scriptActionDelimiter = "// ======================================================================\n// Script Actions\n// ======================================================================\n\n";
	CString const cs_scriptTokenTODelimiter = "// ======================================================================\n// Script %TO Tokens\n// ======================================================================\n\n";
	CString const cs_scriptTokenDIDelimiter = "// ======================================================================\n// Script %DI Tokens\n// ======================================================================\n\n";
	CString const cs_scriptTokenDFDelimiter = "// ======================================================================\n// Script %DF Tokens\n// ======================================================================\n\n";
	CString const cs_scriptTriggerDelimiter = "// ======================================================================\n// Script Triggers\n// ======================================================================\n\n";
	CString const cs_scriptUserTriggerDelimiter = "// ======================================================================\n// User Script Triggers\n// ======================================================================\n\n";
	CString const cs_scriptHandleBranchDelimiter = "// ======================================================================\n// handleBranch<n> Functions \n// ======================================================================\n\n";
	CString const cs_scriptSingleDelimiter = "// ----------------------------------------------------------------------\n\n";
	CString const cs_scriptDoubleDelimiter = "// ======================================================================\n\n";

	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

	int countNumberOfLeadingTabs (char const * current)
	{
		int numberOfTabs = 0;
		while (*current++ == '\t')
			++numberOfTabs;

		return numberOfTabs;
	}

	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

	CString getFirstLine (CString const & text)
	{
		int const index = text.Find ('\n');
		if (index != -1)
			return text.Left (index - 1);

		return text;
	}

	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

	bool continuesVariableName (int const ch)
	{
		return isalnum (ch) || ch == '_';
	}

	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

	int findWholeString (CString const & input, CString const & source)
	{
		int index = -1;
		for (;;)
		{
			//-- search for the first occurrance
			index = input.Find (source, index + 1);
			if (index == -1)
				return index;

			//-- determine whether or not to check the start, end, or both
			bool checkStart = false;
			bool checkEnd = false;

			if (index == 0)
				checkEnd = true;
			else
				if (index == input.GetLength () - 1)
					checkStart = true;
				else
				{
					checkStart = true;
					checkEnd = true;
				}

			//-- if we have to check the start, and the previous character continues a variable, we're invalid
			if (checkStart && continuesVariableName (input [index - 1]))
				continue;

			//-- if we have to check the end, and the next character continues a variable, we're invalid
			if (checkEnd && continuesVariableName (input [index + source.GetLength ()]))
				continue;

			//-- make sure we're not in a literal string
			int start = index;
			int count = 0;
			while (start > 0)
			{
				if (input [start] == '\n')
					break;

				if (input [start] == '"')
					++count;

				--start;
			}

			if (count % 2 == 1)
				continue;

			//-- we've found a match
			return index;
		}

		return index;
	}

	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

	void replaceWholeString (CString & text, CString const & source, CString const & destination)
	{
		int index = -1;
		while ((index = findWholeString (text, source)) != -1)
		{
			text.Delete (index, source.GetLength ());
			text.Insert (index, destination);
		}
	}

	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

	CString getFullScriptFileName (CString const & shortFileName)
	{
		CString fullScriptFileName(ConfigFile::getKeyString("SwgConversationEditor", "scriptPath", 0) + CString ("/") + shortFileName + ".script");
		fullScriptFileName.MakeLower();
		return fullScriptFileName;
	}

	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

	CString getFullStringTableFileName (CString const & shortFileName)
	{
		CString fullStringTableFileName(ConfigFile::getKeyString("SwgConversationEditor", "stringPath", 0) + CString ("/") + shortFileName + ".stf");
		fullStringTableFileName.MakeLower();
		return fullStringTableFileName;
	}

	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

	void extractTokens (char const * const string, std::vector<CString> & stringList) 
	{
		int const len = strlen (string);

		char * const buffer = new char [len + 1];
		strcpy (buffer, string);

		char const seps [] = " ,\t\n/\\=-;!*&.(){}:?[]\"";

		char * current = strtok (buffer, seps);
		while (current)
		{
			stringList.push_back (current);
			current = strtok (0, seps);
		}

		delete [] buffer;
	}

	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

	CString clipText (CString const & text)
	{
		if (text.GetLength () < 25)
			return text;

		return text.Left (25) + "...";
	}

	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

	CString getLetters(int number)
	{
		int const base = 26;
		CString result = static_cast<char>('a' + (number % base));

		if (number < base)
			return result;

		return getLetters(number / base - 1) + result;
	}

	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

	CString getIndentation(int number, char const * indentString = "  ")
	{
		CString result = "";

		for (; number > 0; --number)
		{
			result += indentString;
		}

		return result;
	}

	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
}

using namespace SwgConversationEditorDocNamespace;

// ======================================================================

SwgConversationEditorDoc::ItemData::ItemData () :
	m_type (T_unknown),
	m_conversationItem (0)
{
}

// ----------------------------------------------------------------------

SwgConversationEditorDoc::ItemData::~ItemData ()
{
	m_type = T_unknown;
	m_conversationItem = 0;
}

// ----------------------------------------------------------------------

CString const SwgConversationEditorDoc::ItemData::getText () const
{
	CString text = m_conversationItem->getText ().c_str ();
	if (text.IsEmpty ())
	{
		switch (m_type)
		{
		case T_branch:
			return "Npc Branch";

		case T_response:
			return "Player Response";

		default:
			break;
		}

		return "Unknown";
	}
	else
	{
		CString result;
		
		switch (m_type)
		{
		case T_branch:
			result += "[NPC] ";
			break;

		case T_response:
			result += "[PLAYER] ";
			break;

		default:
			result += "[UNKNOWN] ";
			break;
		}

		bool const addDots = text.GetLength () > 100;
		text = text.Left (100);
		result += text;

		if (addDots)
			result += "...";

		return result;
	}
}

// ======================================================================

IMPLEMENT_DYNCREATE(SwgConversationEditorDoc, CDocument)

BEGIN_MESSAGE_MAP(SwgConversationEditorDoc, CDocument)
	//{{AFX_MSG_MAP(SwgConversationEditorDoc)
	ON_COMMAND(ID_FILE_IMPORT, OnFileImport)
	ON_UPDATE_COMMAND_UI(ID_BUTTON_COMPILE, OnUpdateButtonCompile)
	ON_COMMAND(ID_BUTTON_COMPILEDEBUG, OnButtonCompiledebug)
	ON_COMMAND(ID_BUTTON_COMPILERELEASE, OnButtonCompilerelease)
	ON_COMMAND(ID_BUTTON_P4EDIT, OnButtonP4edit)
	ON_COMMAND(ID_BUTTON_SCAN, OnButtonScan)
	ON_COMMAND(ID_BUTTON_SPELLCHECK, OnButtonSpellcheck)
	ON_COMMAND(ID_BUTTON_FIND, OnButtonFind)
	ON_COMMAND(ID_FILE_EXPORT, OnFileExport)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

// ======================================================================

CString const & SwgConversationEditorDoc::getScriptPreNameDeclarationConditionString ()
{
	return cs_scriptPreNameDeclarationConditionString;
}

// ----------------------------------------------------------------------

CString const & SwgConversationEditorDoc::getScriptPostNameDeclarationConditionString ()
{
	return cs_scriptPostNameDeclarationConditionString;
}

// ----------------------------------------------------------------------

CString const & SwgConversationEditorDoc::getScriptBodyConditionString ()
{
	return cs_scriptBodyConditionString;
}

// ----------------------------------------------------------------------

CString const SwgConversationEditorDoc::createEmptyConditionScript (CString const & scriptName)
{
	return 
		getScriptPreNameDeclarationConditionString () + 
		scriptName +
		getScriptPostNameDeclarationConditionString () +
		getScriptBodyConditionString ();
}

// ----------------------------------------------------------------------

CString const & SwgConversationEditorDoc::getScriptPreNameDeclarationActionString ()
{
	return cs_scriptPreNameDeclarationActionString;
}

// ----------------------------------------------------------------------

CString const & SwgConversationEditorDoc::getScriptPostNameDeclarationActionString ()
{
	return cs_scriptPostNameDeclarationActionString;
}

// ----------------------------------------------------------------------

CString const & SwgConversationEditorDoc::getScriptBodyActionString ()
{
	return cs_scriptBodyActionString;
}

// ----------------------------------------------------------------------

CString const SwgConversationEditorDoc::createEmptyActionScript (CString const & scriptName)
{
	return 
		getScriptPreNameDeclarationActionString () + 
		scriptName +
		getScriptPostNameDeclarationActionString () +
		getScriptBodyActionString ();
}

// ----------------------------------------------------------------------

CString const & SwgConversationEditorDoc::getScriptPreNameDeclarationTokenTOString ()
{
	return cs_scriptPreNameDeclarationTokenTOString;
}

// ----------------------------------------------------------------------

CString const & SwgConversationEditorDoc::getScriptPostNameDeclarationTokenTOString ()
{
	return cs_scriptPostNameDeclarationTokenTOString;
}

// ----------------------------------------------------------------------

CString const & SwgConversationEditorDoc::getScriptBodyTokenTOString ()
{
	return cs_scriptBodyTokenTOString;
}

// ----------------------------------------------------------------------

CString const SwgConversationEditorDoc::createEmptyTokenTOScript (CString const & scriptName)
{
	return 
		getScriptPreNameDeclarationTokenTOString () + 
		scriptName +
		getScriptPostNameDeclarationTokenTOString () +
		getScriptBodyTokenTOString ();
}

// ----------------------------------------------------------------------

CString const & SwgConversationEditorDoc::getScriptPreNameDeclarationTokenDIString ()
{
	return cs_scriptPreNameDeclarationTokenDIString;
}

// ----------------------------------------------------------------------

CString const & SwgConversationEditorDoc::getScriptPostNameDeclarationTokenDIString ()
{
	return cs_scriptPostNameDeclarationTokenDIString;
}

// ----------------------------------------------------------------------

CString const & SwgConversationEditorDoc::getScriptBodyTokenDIString ()
{
	return cs_scriptBodyTokenDIString;
}

// ----------------------------------------------------------------------

CString const SwgConversationEditorDoc::createEmptyTokenDIScript (CString const & scriptName)
{
	return 
		getScriptPreNameDeclarationTokenDIString () + 
		scriptName +
		getScriptPostNameDeclarationTokenDIString () +
		getScriptBodyTokenDIString ();
}

// ----------------------------------------------------------------------

CString const & SwgConversationEditorDoc::getScriptPreNameDeclarationTokenDFString ()
{
	return cs_scriptPreNameDeclarationTokenDFString;
}

// ----------------------------------------------------------------------

CString const & SwgConversationEditorDoc::getScriptPostNameDeclarationTokenDFString ()
{
	return cs_scriptPostNameDeclarationTokenDFString;
}

// ----------------------------------------------------------------------

CString const & SwgConversationEditorDoc::getScriptBodyTokenDFString ()
{
	return cs_scriptBodyTokenDFString;
}

// ----------------------------------------------------------------------

CString const SwgConversationEditorDoc::createEmptyTokenDFScript (CString const & scriptName)
{
	return 
		getScriptPreNameDeclarationTokenDFString () + 
		scriptName +
		getScriptPostNameDeclarationTokenDFString () +
		getScriptBodyTokenDFString ();
}

// ----------------------------------------------------------------------

CString const SwgConversationEditorDoc::getDefaultTriggerText ()
{
	CString buffer;

	//-- write OnInitialize
	{
		buffer += "trigger OnInitialize ()\r\n" \
			"{\r\n" \
			"\tif ((!isTangible (self)) || (isPlayer (self)))\r\n" \
			"\t\tdetachScript(self, \"conversation.filename\");\r\n\r\n" \
			"\tsetCondition (self, CONDITION_CONVERSABLE);\r\n\r\n" \
			"\treturn SCRIPT_CONTINUE;\r\n" \
			"}\r\n\r\n";
	}

	//-- write OnAttach
	{
		buffer += "trigger OnAttach ()\r\n" \
			"{\r\n" \
			"\tsetCondition (self, CONDITION_CONVERSABLE);\r\n\r\n" \
			"\treturn SCRIPT_CONTINUE;\r\n" \
			"}\r\n\r\n";
	}

	//-- write OnObjectMenuRequest
	{
		buffer +=
			"trigger OnObjectMenuRequest (obj_id player, menu_info menuInfo)\r\n" \
			"{\r\n" \
			"\tint menu = menuInfo.addRootMenu (menu_info_types.CONVERSE_START, null);\r\n" \
			"\tmenu_info_data menuInfoData = menuInfo.getMenuItemById (menu);\r\n" \
			"\tmenuInfoData.setServerNotify (false);\r\n" \
			"\tsetCondition (self, CONDITION_CONVERSABLE);\r\n\r\n" \
			"\treturn SCRIPT_CONTINUE;\r\n" \
			"}\r\n\r\n";
	}

	//-- write OnIncapacitated
	{
		buffer += "trigger OnIncapacitated (obj_id killer)\r\n" \
			"{\r\n" \
			"\tclearCondition (self, CONDITION_CONVERSABLE);\r\n" \
			"\tdetachScript (self, \"conversation.filename\");\r\n\r\n" \
			"\treturn SCRIPT_CONTINUE;\r\n" \
			"}\r\n\r\n";
	}

	return buffer;
}

// ----------------------------------------------------------------------

void SwgConversationEditorDoc::fixupTriggerText (CString const & fileName)
{
	CString const oldText (m_conversation->getTriggerText ().c_str ());
	CString text (oldText);
	text.Replace("npc.conversation.", "conversation.");

	int index = 0;
	while ((index = text.Find ("conversation.", index)) != -1)
	{
		CString left = text.Left (index);
		int rightIndex = text.Find ('\"', index);
		CString right = text.Right (text.GetLength () - rightIndex);

		text = left + "conversation." + fileName + right;
		index = left.GetLength () + 1;
	}

	if (text != oldText)
	{
		m_conversation->setTriggerText (std::string (text));
		UpdateAllViews (0, H_triggerChanged);
		SetModifiedFlag (true);
	}
}

// ----------------------------------------------------------------------

CString const SwgConversationEditorDoc::fixConditionFunctionName (CString const & text, CString const & prefix, ScriptGroup const * const scriptGroup, int const familyId)
{
	int const index = text.Find ('{');
	if (index != -1)
	{
		CString const right = text.Right (text.GetLength () - index);
		return
			SwgConversationEditorDoc::getScriptPreNameDeclarationConditionString () +
			prefix + scriptGroup->getFamilyName (familyId) +
			SwgConversationEditorDoc::getScriptPostNameDeclarationConditionString () +
			right;
	}

	return createEmptyConditionScript (scriptGroup->getFamilyName (familyId));
}

// ----------------------------------------------------------------------

CString const SwgConversationEditorDoc::fixActionFunctionName (CString const & text, CString const & prefix, ScriptGroup const * const scriptGroup, int const familyId)
{
	int const index = text.Find ('{');
	if (index != -1)
	{
		CString const right = text.Right (text.GetLength () - index);
		return 
			SwgConversationEditorDoc::getScriptPreNameDeclarationActionString () +
			prefix + scriptGroup->getFamilyName (familyId) +
			SwgConversationEditorDoc::getScriptPostNameDeclarationActionString () +
			right;
	}

	return createEmptyActionScript (scriptGroup->getFamilyName (familyId));
}

// ----------------------------------------------------------------------

CString const SwgConversationEditorDoc::fixTokenTOFunctionName (CString const & text, CString const & prefix, ScriptGroup const * const scriptGroup, int const familyId)
{
	int const index = text.Find ('{');
	if (index != -1)
	{
		CString const right = text.Right (text.GetLength () - index);
		return 
			SwgConversationEditorDoc::getScriptPreNameDeclarationTokenTOString () +
			prefix + scriptGroup->getFamilyName (familyId) +
			SwgConversationEditorDoc::getScriptPostNameDeclarationTokenTOString () +
			right;
	}

	return createEmptyTokenTOScript (scriptGroup->getFamilyName (familyId));
}

// ----------------------------------------------------------------------

CString const SwgConversationEditorDoc::fixTokenDIFunctionName (CString const & text, CString const & prefix, ScriptGroup const * const scriptGroup, int const familyId)
{
	int const index = text.Find ('{');
	if (index != -1)
	{
		CString const right = text.Right (text.GetLength () - index);
		return 
			SwgConversationEditorDoc::getScriptPreNameDeclarationTokenDIString () +
			prefix + scriptGroup->getFamilyName (familyId) +
			SwgConversationEditorDoc::getScriptPostNameDeclarationTokenDIString () +
			right;
	}

	return createEmptyTokenDIScript (scriptGroup->getFamilyName (familyId));
}

// ----------------------------------------------------------------------

CString const SwgConversationEditorDoc::fixTokenDFFunctionName (CString const & text, CString const & prefix, ScriptGroup const * const scriptGroup, int const familyId)
{
	int const index = text.Find ('{');
	if (index != -1)
	{
		CString const right = text.Right (text.GetLength () - index);
		return 
			SwgConversationEditorDoc::getScriptPreNameDeclarationTokenDFString () +
			prefix + scriptGroup->getFamilyName (familyId) +
			SwgConversationEditorDoc::getScriptPostNameDeclarationTokenDFString () +
			right;
	}

	return createEmptyTokenDFScript (scriptGroup->getFamilyName (familyId));
}

// ======================================================================

SwgConversationEditorDoc::SwgConversationEditorDoc() :
	m_conversation (new Conversation ()),
	m_conversationFrame (0),
	m_scriptFrame (0),
	m_warningFrame (0),
	m_scriptShellFrame (0),
	m_numberOfWarnings (0),
	m_numberOfErrors (0),
	m_numberOfComments (0)
{
}

// ----------------------------------------------------------------------

SwgConversationEditorDoc::~SwgConversationEditorDoc()
{
	delete m_conversation;

	m_conversationFrame = 0;
	m_scriptFrame = 0;
	m_warningFrame = 0;
	m_scriptShellFrame = 0;
}

// ----------------------------------------------------------------------

void SwgConversationEditorDoc::Serialize(CArchive & /*ar*/)
{
}

// ----------------------------------------------------------------------

#ifdef _DEBUG
void SwgConversationEditorDoc::AssertValid() const
{
	CDocument::AssertValid();
}

void SwgConversationEditorDoc::Dump(CDumpContext& dc) const
{
	CDocument::Dump(dc);
}
#endif //_DEBUG

// ----------------------------------------------------------------------

Conversation * SwgConversationEditorDoc::getConversation ()
{
	return m_conversation;
}

// ----------------------------------------------------------------------

Conversation const * SwgConversationEditorDoc::getConversation () const
{
	return m_conversation;
}

// ----------------------------------------------------------------------

ScriptGroup * SwgConversationEditorDoc::getConditionGroup ()
{
	return m_conversation->getConditionGroup ();
}

// ----------------------------------------------------------------------

ScriptGroup const * SwgConversationEditorDoc::getConditionGroup () const
{
	return m_conversation->getConditionGroup ();
}

// ----------------------------------------------------------------------

ScriptGroup * SwgConversationEditorDoc::getActionGroup ()
{
	return m_conversation->getActionGroup ();
}

// ----------------------------------------------------------------------

ScriptGroup const * SwgConversationEditorDoc::getActionGroup () const
{
	return m_conversation->getActionGroup ();
}

// ----------------------------------------------------------------------

ScriptGroup * SwgConversationEditorDoc::getLabelGroup ()
{
	return m_conversation->getLabelGroup ();
}

// ----------------------------------------------------------------------

ScriptGroup const * SwgConversationEditorDoc::getLabelGroup () const
{
	return m_conversation->getLabelGroup ();
}

// ----------------------------------------------------------------------

ScriptGroup * SwgConversationEditorDoc::getTokenTOGroup ()
{
	return m_conversation->getTokenTOGroup ();
}

// ----------------------------------------------------------------------

ScriptGroup const * SwgConversationEditorDoc::getTokenTOGroup () const
{
	return m_conversation->getTokenTOGroup ();
}

// ----------------------------------------------------------------------

ScriptGroup * SwgConversationEditorDoc::getTokenDIGroup ()
{
	return m_conversation->getTokenDIGroup ();
}

// ----------------------------------------------------------------------

ScriptGroup const * SwgConversationEditorDoc::getTokenDIGroup () const
{
	return m_conversation->getTokenDIGroup ();
}

// ----------------------------------------------------------------------

ScriptGroup * SwgConversationEditorDoc::getTokenDFGroup ()
{
	return m_conversation->getTokenDFGroup ();
}

// ----------------------------------------------------------------------

ScriptGroup const * SwgConversationEditorDoc::getTokenDFGroup () const
{
	return m_conversation->getTokenDFGroup ();
}

// ----------------------------------------------------------------------

Conversation::LibrarySet & SwgConversationEditorDoc::getLibrarySet ()
{
	return m_conversation->getLibrarySet ();
}

// ----------------------------------------------------------------------

Conversation::LibrarySet const & SwgConversationEditorDoc::getLibrarySet () const
{
	return m_conversation->getLibrarySet ();
}

// ----------------------------------------------------------------------

ConversationFrame * SwgConversationEditorDoc::getConversationFrame ()
{
	return m_conversationFrame;
}

// ----------------------------------------------------------------------

void SwgConversationEditorDoc::setConversationFrame (ConversationFrame * const conversationFrame)
{
	m_conversationFrame = conversationFrame;
}

// ----------------------------------------------------------------------

ScriptFrame * SwgConversationEditorDoc::getScriptFrame ()
{
	return m_scriptFrame;
}

// ----------------------------------------------------------------------

void SwgConversationEditorDoc::setScriptFrame (ScriptFrame * const scriptFrame)
{
	m_scriptFrame = scriptFrame;
}

// ----------------------------------------------------------------------

WarningFrame * SwgConversationEditorDoc::getWarningFrame ()
{
	return m_warningFrame;
}

// ----------------------------------------------------------------------

void SwgConversationEditorDoc::setWarningFrame (WarningFrame * const warningFrame)
{
	m_warningFrame = warningFrame;
}

// ----------------------------------------------------------------------

ScriptShellFrame * SwgConversationEditorDoc::getScriptShellFrame ()
{
	return m_scriptShellFrame;
}

// ----------------------------------------------------------------------

void SwgConversationEditorDoc::setScriptShellFrame (ScriptShellFrame * const scriptShellFrame)
{
	m_scriptShellFrame = scriptShellFrame;
}

// ----------------------------------------------------------------------

StringIdTracker * SwgConversationEditorDoc::getStringIdTracker ()
{
	return m_conversation->getStringIdTracker();
}

// ----------------------------------------------------------------------

void SwgConversationEditorDoc::buildScriptGroupComboBox (CComboBox & comboBox, ScriptGroup const * const scriptGroup, int const selectedFamilyId, bool const skipRoot) const
{
	comboBox.Clear ();
	CDC * const dc = comboBox.GetDC ();

	int dropWidth = 0;

	for (int i = 0; i < scriptGroup->getNumberOfFamilies (); ++i)
	{
		int const familyId = scriptGroup->getFamilyId (i);
		if (skipRoot && familyId == 1)
			continue;

		CString const string = scriptGroup->getFamilyName (familyId);
		int const position = comboBox.AddString (string);
		IGNORE_RETURN (comboBox.SetItemData (position, static_cast<DWORD> (familyId)));

		if (familyId == selectedFamilyId)
			IGNORE_RETURN (comboBox.SetCurSel (position));

		CSize const size = dc->GetTextExtent (string);

		if (size.cx > dropWidth)
			dropWidth = size.cx;
	}

	comboBox.ReleaseDC (dc);
	comboBox.SetDroppedWidth (dropWidth);
}

// ----------------------------------------------------------------------

void SwgConversationEditorDoc::buildConditionGroupComboBox (CComboBox & comboBox, int const selectedFamilyId) const
{
	buildScriptGroupComboBox (comboBox, getConditionGroup (), selectedFamilyId, false);
}

// ----------------------------------------------------------------------

void SwgConversationEditorDoc::buildActionGroupComboBox (CComboBox & comboBox, int const selectedFamilyId) const
{
	buildScriptGroupComboBox (comboBox, getActionGroup (), selectedFamilyId, false);
}

// ----------------------------------------------------------------------

void SwgConversationEditorDoc::buildLabelGroupComboBox (CComboBox & comboBox, int const selectedFamilyId, bool const skipRoot) const
{
	buildScriptGroupComboBox (comboBox, getLabelGroup (), selectedFamilyId, skipRoot);
}

// ----------------------------------------------------------------------

void SwgConversationEditorDoc::buildTokenTOGroupComboBox (CComboBox & comboBox, int const selectedFamilyId) const
{
	buildScriptGroupComboBox (comboBox, getTokenTOGroup (), selectedFamilyId, false);
}

// ----------------------------------------------------------------------

void SwgConversationEditorDoc::buildTokenDIGroupComboBox (CComboBox & comboBox, int const selectedFamilyId) const
{
	buildScriptGroupComboBox (comboBox, getTokenDIGroup (), selectedFamilyId, false);
}

// ----------------------------------------------------------------------

void SwgConversationEditorDoc::buildTokenDFGroupComboBox (CComboBox & comboBox, int const selectedFamilyId) const
{
	buildScriptGroupComboBox (comboBox, getTokenDFGroup (), selectedFamilyId, false);
}

// ----------------------------------------------------------------------

BOOL SwgConversationEditorDoc::OnNewDocument()
{
	if (!CDocument::OnNewDocument())
		return FALSE;

	//-- reset the conversation
	m_conversation->reset ();
	m_conversation->addBranch (new ConversationBranch (getStringIdTracker()));
	m_conversation->setLabelFamilyId (1);

	ScriptGroup * const conditionGroup = getConditionGroup ();
	conditionGroup->addFamily (0, cs_conditionDefaultName);
	conditionGroup->setFamilyText (0, createEmptyConditionScript (cs_conditionDefaultName));

	ScriptGroup * const actionGroup = getActionGroup ();
	actionGroup->addFamily (0, cs_actionDefaultName);
	actionGroup->setFamilyText (0, createEmptyActionScript (cs_actionDefaultName));

	ScriptGroup * const tokenTOGroup = getTokenTOGroup ();
	tokenTOGroup->addFamily (0, cs_tokenDefaultName);
	tokenTOGroup->setFamilyText (0, createEmptyTokenTOScript (cs_tokenDefaultName));

	ScriptGroup * const tokenDIGroup = getTokenDIGroup ();
	tokenDIGroup->addFamily (0, cs_tokenDefaultName);
	tokenDIGroup->setFamilyText (0, createEmptyTokenDIScript (cs_tokenDefaultName));

	ScriptGroup * const tokenDFGroup = getTokenDFGroup ();
	tokenDFGroup->addFamily (0, cs_tokenDefaultName);
	tokenDFGroup->setFamilyText (0, createEmptyTokenDFScript (cs_tokenDefaultName));

	ScriptGroup * const labelGroup = getLabelGroup ();
	labelGroup->addFamily (0, cs_labelDefaultName);
	labelGroup->addFamily (1, cs_labelRootName);

	getLibrarySet ().insert (std::string ("ai_lib"));
	getLibrarySet ().insert (std::string ("chat"));
	getLibrarySet ().insert (std::string ("conversation"));
	getLibrarySet ().insert (std::string ("utils"));

	m_conversation->setTriggerText (std::string (getDefaultTriggerText ()));

	//-- open default views
	safe_cast<SwgConversationEditorApp*> (AfxGetApp ())->onOpenDefaultViews (this);

	return TRUE;
}

// ----------------------------------------------------------------------

BOOL SwgConversationEditorDoc::OnOpenDocument(LPCTSTR lpszPathName) 
{
	if (!CDocument::OnOpenDocument(lpszPathName))
		return FALSE;
	
	// TODO: Add your specialized creation code here
	FatalSetThrowExceptions (false);

		try
		{
			if (!m_conversation->load (lpszPathName))
				return FALSE;

			if (m_conversation->getTriggerText ().empty ())
			{
				m_conversation->setTriggerText (std::string (getDefaultTriggerText ()));
				SetModifiedFlag (true);
			}

			if (getTokenTOGroup ()->getNumberOfFamilies () == 0)
			{
				getTokenTOGroup ()->addFamily (0, cs_tokenDefaultName);
				SetModifiedFlag (true);
			}

			if (getTokenDIGroup ()->getNumberOfFamilies () == 0)
			{
				getTokenDIGroup ()->addFamily (0, cs_tokenDefaultName);
				SetModifiedFlag (true);
			}

			if (getTokenDFGroup ()->getNumberOfFamilies () == 0)
			{
				getTokenDFGroup ()->addFamily (0, cs_tokenDefaultName);
				SetModifiedFlag (true);
			}

			if (getLabelGroup ()->getNumberOfFamilies () == 0)
			{
				getLabelGroup ()->addFamily (0, cs_labelDefaultName);
				getLabelGroup ()->addFamily (1, cs_labelRootName);
				SetModifiedFlag (true);
			}

			{
				Conversation::LibrarySet & librarySet = getLibrarySet();
				if (librarySet.find(std::string("ai_lib")) == librarySet.end())
				{
					librarySet.insert(std::string("ai_lib"));
					SetModifiedFlag (true);
				}

				if (librarySet.find(std::string("chat")) == librarySet.end())
				{
					librarySet.insert(std::string("chat"));
					SetModifiedFlag (true);
				}

				if (librarySet.find(std::string("conversation")) == librarySet.end())
				{
					librarySet.insert(std::string("conversation"));
					SetModifiedFlag (true);
				}

				if (librarySet.find(std::string("utils")) == librarySet.end())
				{
					librarySet.insert(std::string("utils"));
					SetModifiedFlag (true);
				}
			}
		}
		catch (FatalException const & exception)
		{
			CString message ("Error loading ");
			message += lpszPathName;
			message += "\n";
			message += "Actual: ";
			message += exception.getMessage ();

			MessageBox (0, message, AfxGetApp ()->m_pszAppName, MB_ICONSTOP);
		}

	FatalSetThrowExceptions (false);

	//-- open default views
	safe_cast<SwgConversationEditorApp*> (AfxGetApp ())->onOpenDefaultViews (this);

	return TRUE;
}

// ----------------------------------------------------------------------

BOOL SwgConversationEditorDoc::OnSaveDocument(LPCTSTR lpszPathName) 
{
	UpdateAllViews (0, H_savingDocument);

	if (strstr(lpszPathName, " ") != 0)
	{
		AfxMessageBox ("Error saving conversation: Filename cannot have spaces.");
		return FALSE;
	}

	// TODO: Add your specialized code here and/or call the base class
	FatalSetThrowExceptions (false);

		try
		{
			if (!m_conversation->save (lpszPathName))
				return FALSE;
		}
		catch (FatalException const & exception)
		{
			CString message ("Error saving ");
			message += lpszPathName;
			message += "\n";
			message += "Actual: ";
			message += exception.getMessage ();

			MessageBox (0, message, AfxGetApp ()->m_pszAppName, MB_ICONSTOP);
		}

	FatalSetThrowExceptions (false);
	
	SetModifiedFlag (false);

	return TRUE;
}

// ----------------------------------------------------------------------

void SwgConversationEditorDoc::OnFileImport() 
{
	CFileDialog dlg (true, "*.txt", 0, (OFN_FILEMUSTEXIST | OFN_NOCHANGEDIR), "Text Files *.txt|*.txt||");
	dlg.m_ofn.lpstrInitialDir = RecentDirectory::find ("Import");
	dlg.m_ofn.lpstrDefExt = "txt";
	dlg.m_ofn.lpstrTitle= "Importing Text File";
	if (dlg.DoModal () == IDOK)
	{
		CStdioFile infile;
		if (!infile.Open (dlg.GetPathName (), CFile::modeRead | CFile::typeText))
			return;

		IGNORE_RETURN (RecentDirectory::update ("Import", dlg.GetPathName ()));

		std::vector<ConversationItem *> conversationStack;
		conversationStack.push_back (m_conversation);

		int lineNumber = 0;
		CString line;
		while (infile.ReadString (line))
		{
			++lineNumber;
			int const tabs = countNumberOfLeadingTabs (line);
			if (tabs == 0)
			{
				CString message;
				message.Format ("Import failed error 1 (line %i) -- line with no leading tabs", lineNumber);
				MessageBox (0, message, AfxGetApp ()->m_pszAppName, MB_OK);
				return;
			}

			CString text = line;
			text.TrimLeft ();
			text.TrimRight ();
			if (text.IsEmpty ())
				continue;

			if (conversationStack.empty ())
			{
				CString message;
				message.Format ("Import failed error 2 (line %i) -- mismatched leading tabs", lineNumber);
				MessageBox (0, message, AfxGetApp ()->m_pszAppName, MB_OK);
				return;
			}

			while (static_cast<int> (conversationStack.size ()) > tabs)
				conversationStack.pop_back ();

			switch (tabs % 2)
			{
			case 0:
				{
					ConversationResponse * const response = new ConversationResponse(getStringIdTracker());
					response->setText (std::string (text));
					safe_cast<ConversationBranch *> (conversationStack.back ())->addResponse (response);
					conversationStack.push_back (response);
				}
				break;

			case 1:
				{
					ConversationBranch * const branch = new ConversationBranch(getStringIdTracker());
					branch->setText (std::string (text));
					safe_cast<ConversationResponse *> (conversationStack.back ())->addBranch (branch);
					conversationStack.push_back (branch);
				}
				break;
			}

//			CString temp;
//			temp.Format ("%i %i %s\n", tabs, conversationStack.size (), line);
//			OutputDebugString (temp);
		}
	}

	UpdateAllViews (0, SwgConversationEditorDoc::H_fileImported, 0);
}

// ----------------------------------------------------------------------

void SwgConversationEditorDoc::OnUpdateButtonCompile(CCmdUI* pCmdUI) 
{
	pCmdUI->Enable (!GetPathName ().IsEmpty ());
}

// ----------------------------------------------------------------------

std::pair <bool, bool> SwgConversationEditorDoc::compile (bool const spellCheck, bool const scan, bool const compile, bool const debug, bool const test) 
{
	CString pathName = GetPathName ();
	pathName.MakeLower ();

	FileName fileName (pathName);
	fileName.stripPathAndExt ();

	if (strlen (fileName) == 0)
	{
		CString const message = "Please save the conversation before continuing.";
		IGNORE_RETURN (MessageBox (0, message, AfxGetApp ()->m_pszAppName, MB_OK));
		return std::make_pair (false, true);
	}

	int const index = pathName.ReverseFind ('.');
	if (index != -1)
		pathName = pathName.Left (index);

	safe_cast<SwgConversationEditorApp *> (AfxGetApp ())->onViewWarning (this);
	m_warningFrame->clear ();

	if (m_scriptShellFrame)
		m_scriptShellFrame->DestroyWindow ();

	m_numberOfWarnings = 0;
	m_numberOfErrors = 0;

	ConversationItemList conversationItemList;
	int branchId = 0;
	if (compile || scan)
	{
		prepareCompileResponse (m_conversation, conversationItemList, branchId);
		verifyLabels ();
		verifyLinks ();
	}

	if (spellCheck)
	{
		spellCheckResponse (m_conversation);

		if (m_numberOfWarnings == 0)
		{
			m_warningFrame->DestroyWindow ();

			MessageBox (0, "Spell check complete.", AfxGetApp ()->m_pszAppName, MB_OK);
		}

		return std::make_pair (m_numberOfErrors == 0, false);
	}

	if (scan && !compile)
	{
		if (m_numberOfErrors == 0 && m_numberOfWarnings == 0)
		{
			m_warningFrame->DestroyWindow ();

			if (!test)
				MessageBox (0, "Scan complete.", AfxGetApp ()->m_pszAppName, MB_OK);
		}

		return std::make_pair (m_numberOfErrors == 0, false);
	}

	if (compile)
	{
		if (m_numberOfErrors > 0)
		{
			CString buffer;
			buffer.Format ("%i errors found, fix before continuing.  See Output window for details.", m_numberOfErrors);

			IGNORE_RETURN (MessageBox (0, buffer, AfxGetApp ()->m_pszAppName, MB_OK));
		}
		else
		{
			if (m_numberOfWarnings == 0)
				m_warningFrame->DestroyWindow ();

			//-- pre-compile save
			OnSaveDocument (GetPathName ());

			CString const shortFileName (fileName);
			CString const scriptName = getFullScriptFileName (shortFileName);
			fixupTriggerText (shortFileName);
			if (writeScript (scriptName, shortFileName, conversationItemList, debug))
			{
				CString const stringTableName = getFullStringTableFileName (shortFileName);
				if (writeStringTable (stringTableName, shortFileName, conversationItemList))
				{
					safe_cast<SwgConversationEditorApp *> (AfxGetApp ())->onViewShell (this);

					ShellData * const shellData = new ShellData;
					shellData->m_debug = debug;
					shellData->m_shortFileName = shortFileName;
					shellData->m_fullScriptFileName = scriptName;
					UpdateAllViews (0, H_shellCompile, shellData);

					//-- post-compile save
					OnSaveDocument (GetPathName ());
				}
				else
					MessageBox (0, stringTableName + " could not be saved.  Does the destination directory exist?  Is the file read-only?", AfxGetApp ()->m_pszAppName, MB_OK);
			}
			else
				MessageBox (0, scriptName + " could not be saved.  Does the destination directory exist?  Is the file read-only?", AfxGetApp ()->m_pszAppName, MB_OK);
		}

		return std::make_pair (m_numberOfErrors == 0, false);
	}

	return std::make_pair (true, false);
}

// ----------------------------------------------------------------------

void SwgConversationEditorDoc::prepareCompileBranch (ConversationBranch const * const branch, std::vector<ConversationItem const *> & conversationItemList, int & branchId)
{
	conversationItemList.push_back (branch);

	branch->setBranchId (++branchId);

	if (branch->getText ().empty ())
		addError ("Branch missing text entry.", branch);
	else
	{
		CString const text(branch->getText ().c_str());

		bool const hasTU = text.Find("%TU") != -1;
		bool const hasTO = text.Find("%TO") != -1;
		bool const hasTT = text.Find("%TT") != -1;
		bool const hasNU = text.Find("%NU") != -1;
		bool const hasNO = text.Find("%NO") != -1;
		bool const hasNT = text.Find("%NT") != -1;
		bool const hasDI = text.Find("%DI") != -1;
		bool const hasDF = text.Find("%DF") != -1;

		if (branch->getUseProsePackage ())
		{
			if (!hasTU && !hasTO && !hasTT && !hasNU && !hasNO && !hasNT && !hasDI && !hasDF)
				addError ("Branch text is using prose package, but no tokens (%TU, %NU, %TT, %NT, %TO, %NO, %DI or %DF) were found.", branch);

			if (branch->getTokenTOFamilyId() != 0 && !hasTO)
				addError ("Branch specifies token function %TO, but %TO is not present in the branch text.", branch);

			if (branch->getTokenDIFamilyId() != 0 && !hasDI)
				addError ("Branch specifies token function %DI, but %DI is not present in the branch text.", branch);

			if (branch->getTokenDFFamilyId() != 0 && !hasDF)
				addError ("Branch specifies token function %DF, but %DF is not present in the branch text.", branch);

			if (branch->getTokenTOFamilyId() == 0 && hasTO)
				addError ("Branch text specifies token %TO, but token function %TO is set to _none.", branch);

			if (branch->getTokenDIFamilyId() == 0 && hasDI)
				addError ("Branch text specifies token %DI, but token function %DI is set to _none.", branch);

			if (branch->getTokenDFFamilyId() == 0 && hasDF)
				addError ("Branch text specifies token %DF, but token function %DF is set to _none.", branch);
		}
		else
		{
			if (hasTU || hasTO || hasTT || hasNU || hasNO || hasNT || hasDI || hasDF)
				addError ("Branch text specifies tokens (%TU, %NU, %TT, %NT, %TO, %NO, %DI or %DF), but use prose package is not checked.", branch);
		}

		if (text.Find("\"") != -1)
			addError ("You cannot use quotes (\") in branch text.", branch);
		else
		{
			for (int i = 0; i < text.GetLength(); ++i)
				if (text[i] < 0)
					addError (CString("Invalid character [") + text[i] + "] found in branch text.", branch);
		}
	}

	if (branch->getLabelFamilyId () != 0 && branch->getLabelFamilyId () == branch->getLinkFamilyId ())
		addError ("Branch is linked to itself.", branch);
		
	if (branch->getLinkFamilyId () != 0 && branch->hasChildren ())
		addError ("Branch with children is also linked.  You can have one or the other.", branch);

	for (int i = 0; i < branch->getNumberOfResponses (); ++i)
		prepareCompileResponse (branch->getResponse (i), conversationItemList, branchId);
}

// ----------------------------------------------------------------------

void SwgConversationEditorDoc::prepareCompileResponse (ConversationResponse const * const response, std::vector<ConversationItem const *> & conversationItemList, int & branchId)
{
	conversationItemList.push_back (response);

	if (response->getText().empty())
	{
		if (response != m_conversation)
			addError ("Response missing text entry.", response);
	}
	else
	{
		CString const text(response->getText ().c_str());
		if (text.Find("\"") != -1)
			addError ("You cannot use quotes (\") in response text.", response);
		else
		{
			for (int i = 0; i < text.GetLength(); ++i)
				if (text[i] < 0)
					addError (CString("Invalid character [") + text[i] + "] found in response text.", response);
		}
	}

	if (response->getLabelFamilyId () != 0 && response->getLabelFamilyId () == response->getLinkFamilyId ())
		addError ("Response is linked to itself.", response);
		
	if (response->getLinkFamilyId () != 0 && response->hasChildren ())
		addError ("Response with children is also linked.  You can have one or the other.", response);

	if (response->getLinkFamilyId () == 0 && !response->hasChildren ())
		addError ("Responses must have at least one branch or link.  Feedback must always be given to the player.", response);

	//-- Branches must have unique response text
	{
		std::vector<std::string> stringVector;
		for (int i = 0; i < response->getNumberOfBranches(); ++i)
		{
			ConversationItem const * const conversationItem = response->getBranch(i);
			if (std::find(stringVector.begin(), stringVector.end(), conversationItem->getText()) == stringVector.end())
				stringVector.push_back(conversationItem->getText());
			else
			{
				addError(CString("Found response with multiple branches with the exact same text '") + conversationItem->getText().c_str() + "'", response);
				break;
			}
		}
	}

	//-- Branches cannot have the same conditions
	{
		std::set<int> uniqueNonDefaultBranchSet;
		for (int i = 0; i < response->getNumberOfBranches(); ++i)
		{
			ConversationBranch const * const branch = response->getBranch(i);

			int const conditionFamilyId = branch->getNegateCondition() ? -branch->getConditionFamilyId() - 1: branch->getConditionFamilyId();
			if (conditionFamilyId && uniqueNonDefaultBranchSet.find(conditionFamilyId) != uniqueNonDefaultBranchSet.end())
			{
				addError("One or more branches have within this response have the same condition.  Only the first condition(if true) will ever be displayed.", response);
				break;
			}

			uniqueNonDefaultBranchSet.insert(conditionFamilyId);

			prepareCompileBranch(branch, conversationItemList, branchId);
		}
	}
}

// ----------------------------------------------------------------------

void SwgConversationEditorDoc::addError (CString const & error, ConversationItem const * const conversationItem)
{
	if (m_warningFrame)
	{
		m_warningFrame->add (0, error, conversationItem);
		++m_numberOfErrors;
	}
}

// ----------------------------------------------------------------------

void SwgConversationEditorDoc::addWarning (CString const & warning, ConversationItem const * const conversationItem)
{
	if (m_warningFrame)
	{
		m_warningFrame->add (1, warning, conversationItem);
		++m_numberOfWarnings;
	}
}

// ----------------------------------------------------------------------

void SwgConversationEditorDoc::addComment (CString const & comment, ConversationItem const * conversationItem)
{
	if (m_warningFrame)
	{
		m_warningFrame->add (2, comment, conversationItem);
		++m_numberOfComments;
	}
}

// ----------------------------------------------------------------------

bool SwgConversationEditorDoc::findConditions (int const conditionFamilyId, ConversationItemList & conversationItemList) const
{
	findConditionResponse (m_conversation, conditionFamilyId, conversationItemList);
	return !conversationItemList.empty ();
}

// ----------------------------------------------------------------------

bool SwgConversationEditorDoc::findActions (int const actionFamilyId, ConversationItemList & conversationItemList) const
{
	findActionResponse (m_conversation, actionFamilyId, conversationItemList);
	return !conversationItemList.empty ();
}

// ----------------------------------------------------------------------

bool SwgConversationEditorDoc::findLabels (int const labelFamilyId, ConversationItemList & conversationItemList) const
{
	findLabelResponse (m_conversation, labelFamilyId, conversationItemList);
	findLinkResponse (m_conversation, labelFamilyId, conversationItemList);
	return !conversationItemList.empty ();
}

// ----------------------------------------------------------------------

void SwgConversationEditorDoc::findConditionBranch (ConversationBranch const * const branch, int const conditionFamilyId, ConversationItemList & conversationItemList) const
{
	if (branch->getConditionFamilyId () == conditionFamilyId)
		conversationItemList.push_back (branch);

	for (int i = 0; i < branch->getNumberOfResponses (); ++i)
		findConditionResponse (branch->getResponse (i), conditionFamilyId, conversationItemList);
}

// ----------------------------------------------------------------------

void SwgConversationEditorDoc::findConditionResponse (ConversationResponse const * const response, int const conditionFamilyId, ConversationItemList & conversationItemList) const
{
	if (response->getConditionFamilyId () == conditionFamilyId)
		conversationItemList.push_back (response);

	for (int i = 0; i < response->getNumberOfBranches (); ++i)
		findConditionBranch (response->getBranch (i), conditionFamilyId, conversationItemList);
}

// ----------------------------------------------------------------------

bool SwgConversationEditorDoc::conditionInUse (int conditionFamilyId) const
{
	ConversationItemList conversationItemList;
	findConditionResponse (m_conversation, conditionFamilyId, conversationItemList);
	return !conversationItemList.empty ();
}

// ----------------------------------------------------------------------

void SwgConversationEditorDoc::findActionBranch (ConversationBranch const * const branch, int const actionFamilyId, ConversationItemList & conversationItemList) const
{
	if (branch->getActionFamilyId () == actionFamilyId)
		conversationItemList.push_back (branch);

	for (int i = 0; i < branch->getNumberOfResponses (); ++i)
		findActionResponse (branch->getResponse (i), actionFamilyId, conversationItemList);
}

// ----------------------------------------------------------------------

void SwgConversationEditorDoc::findActionResponse (ConversationResponse const * const response, int const actionFamilyId, ConversationItemList & conversationItemList) const
{
	if (response->getActionFamilyId () == actionFamilyId)
		conversationItemList.push_back (response);

	for (int i = 0; i < response->getNumberOfBranches (); ++i)
		findActionBranch (response->getBranch (i), actionFamilyId, conversationItemList);
}

// ----------------------------------------------------------------------

bool SwgConversationEditorDoc::actionInUse (int const actionFamilyId) const
{
	ConversationItemList conversationItemList;
	findActionResponse (m_conversation, actionFamilyId, conversationItemList);
	return !conversationItemList.empty ();
}

// ----------------------------------------------------------------------

void SwgConversationEditorDoc::findTokenTOBranch (ConversationBranch const * const branch, int const tokenTOFamilyId, ConversationItemList & conversationItemList) const
{
	if (branch->getTokenTOFamilyId () == tokenTOFamilyId)
		conversationItemList.push_back (branch);

	for (int i = 0; i < branch->getNumberOfResponses (); ++i)
		findTokenTOResponse (branch->getResponse (i), tokenTOFamilyId, conversationItemList);
}

// ----------------------------------------------------------------------

void SwgConversationEditorDoc::findTokenTOResponse (ConversationResponse const * const response, int const tokenTOFamilyId, ConversationItemList & conversationItemList) const
{
	if (response->getTokenTOFamilyId () == tokenTOFamilyId)
		conversationItemList.push_back (response);

	for (int i = 0; i < response->getNumberOfBranches (); ++i)
		findTokenTOBranch (response->getBranch (i), tokenTOFamilyId, conversationItemList);
}

// ----------------------------------------------------------------------

bool SwgConversationEditorDoc::tokenTOInUse (int const tokenTOFamilyId) const
{
	ConversationItemList conversationItemList;
	findTokenTOResponse (m_conversation, tokenTOFamilyId, conversationItemList);
	return !conversationItemList.empty ();
}

// ----------------------------------------------------------------------

void SwgConversationEditorDoc::findTokenDIBranch (ConversationBranch const * const branch, int const tokenDIFamilyId, ConversationItemList & conversationItemList) const
{
	if (branch->getTokenDIFamilyId () == tokenDIFamilyId)
		conversationItemList.push_back (branch);

	for (int i = 0; i < branch->getNumberOfResponses (); ++i)
		findTokenDIResponse (branch->getResponse (i), tokenDIFamilyId, conversationItemList);
}

// ----------------------------------------------------------------------

void SwgConversationEditorDoc::findTokenDIResponse (ConversationResponse const * const response, int const tokenDIFamilyId, ConversationItemList & conversationItemList) const
{
	if (response->getTokenDIFamilyId () == tokenDIFamilyId)
		conversationItemList.push_back (response);

	for (int i = 0; i < response->getNumberOfBranches (); ++i)
		findTokenDIBranch (response->getBranch (i), tokenDIFamilyId, conversationItemList);
}

// ----------------------------------------------------------------------

bool SwgConversationEditorDoc::tokenDIInUse (int const tokenDIFamilyId) const
{
	ConversationItemList conversationItemList;
	findTokenDIResponse (m_conversation, tokenDIFamilyId, conversationItemList);
	return !conversationItemList.empty ();
}

// ----------------------------------------------------------------------

void SwgConversationEditorDoc::findTokenDFBranch (ConversationBranch const * const branch, int const tokenDFFamilyId, ConversationItemList & conversationItemList) const
{
	if (branch->getTokenDFFamilyId () == tokenDFFamilyId)
		conversationItemList.push_back (branch);

	for (int i = 0; i < branch->getNumberOfResponses (); ++i)
		findTokenDFResponse (branch->getResponse (i), tokenDFFamilyId, conversationItemList);
}

// ----------------------------------------------------------------------

void SwgConversationEditorDoc::findTokenDFResponse (ConversationResponse const * const response, int const tokenDFFamilyId, ConversationItemList & conversationItemList) const
{
	if (response->getTokenDFFamilyId () == tokenDFFamilyId)
		conversationItemList.push_back (response);

	for (int i = 0; i < response->getNumberOfBranches (); ++i)
		findTokenDFBranch (response->getBranch (i), tokenDFFamilyId, conversationItemList);
}

// ----------------------------------------------------------------------

bool SwgConversationEditorDoc::tokenDFInUse (int const tokenDFFamilyId) const
{
	ConversationItemList conversationItemList;
	findTokenDFResponse (m_conversation, tokenDFFamilyId, conversationItemList);
	return !conversationItemList.empty ();
}

// ----------------------------------------------------------------------

void SwgConversationEditorDoc::findLabelBranch (ConversationBranch const * const branch, int const labelFamilyId, ConversationItemList & conversationItemList) const
{
	if (branch->getLabelFamilyId () == labelFamilyId)
		conversationItemList.push_back (branch);

	for (int i = 0; i < branch->getNumberOfResponses (); ++i)
		findLabelResponse (branch->getResponse (i), labelFamilyId, conversationItemList);
}

// ----------------------------------------------------------------------

void SwgConversationEditorDoc::findLabelResponse (ConversationResponse const * const response, int const labelFamilyId, ConversationItemList & conversationItemList) const
{
	if (response->getLabelFamilyId () == labelFamilyId)
		conversationItemList.push_back (response);

	for (int i = 0; i < response->getNumberOfBranches (); ++i)
		findLabelBranch (response->getBranch (i), labelFamilyId, conversationItemList);
}

// ----------------------------------------------------------------------

bool SwgConversationEditorDoc::labelInUseAsLabel (int const labelFamilyId) const
{
	ConversationItemList conversationItemList;
	findLabelResponse (m_conversation, labelFamilyId, conversationItemList);
	return !conversationItemList.empty ();
}

// ----------------------------------------------------------------------

bool SwgConversationEditorDoc::labelInUseAsLink (int const labelFamilyId) const
{
	ConversationItemList conversationItemList;
	findLinkResponse (m_conversation, labelFamilyId, conversationItemList);
	return !conversationItemList.empty ();
}

// ----------------------------------------------------------------------

bool SwgConversationEditorDoc::labelInUseAsLabelOrLink (int const labelFamilyId) const
{
	return labelInUseAsLabel (labelFamilyId) || labelInUseAsLink (labelFamilyId);
}

// ----------------------------------------------------------------------

void SwgConversationEditorDoc::findLinkBranch (ConversationBranch const * const branch, int const linkFamilyId, ConversationItemList & conversationItemList) const
{
	if (branch->getLinkFamilyId () == linkFamilyId)
		conversationItemList.push_back (branch);

	for (int i = 0; i < branch->getNumberOfResponses (); ++i)
		findLinkResponse (branch->getResponse (i), linkFamilyId, conversationItemList);
}

// ----------------------------------------------------------------------

void SwgConversationEditorDoc::findLinkResponse (ConversationResponse const * const response, int const linkFamilyId, ConversationItemList & conversationItemList) const
{
	if (response->getLinkFamilyId () == linkFamilyId)
		conversationItemList.push_back (response);

	for (int i = 0; i < response->getNumberOfBranches (); ++i)
		findLinkBranch (response->getBranch (i), linkFamilyId, conversationItemList);
}

// ----------------------------------------------------------------------

ConversationItem const * SwgConversationEditorDoc::findLabel (int const labelFamilyId) const
{
	return findLabelResponse (m_conversation, labelFamilyId);
}

// ----------------------------------------------------------------------

ConversationItem const * SwgConversationEditorDoc::findLabelBranch (ConversationBranch const * branch, int const labelFamilyId) const
{
	if (branch->getLabelFamilyId () == labelFamilyId)
		return branch;

	ConversationItem const * result = 0;

	for (int i = 0; i < branch->getNumberOfResponses (); ++i)
	{
		result = findLabelResponse (branch->getResponse (i), labelFamilyId);
		if (result)
			return result;
	}

	return result;
}

// ----------------------------------------------------------------------

ConversationItem const * SwgConversationEditorDoc::findLabelResponse (ConversationResponse const * response, int const labelFamilyId) const
{
	if (response->getLabelFamilyId () == labelFamilyId)
		return response;

	ConversationItem const * result = 0;

	for (int i = 0; i < response->getNumberOfBranches (); ++i)
	{
		result = findLabelBranch (response->getBranch (i), labelFamilyId);
		if (result)
			return result;
	}

	return result;
}

// ----------------------------------------------------------------------

void SwgConversationEditorDoc::spellCheck (CString const & text, StringList & stringList, ConversationItem const * const conversationItem)
{
	extractTokens (text, stringList);
	if (!stringList.empty ())
	{
		uint i = 0;
		uint n = stringList.size ();
		while (i < n)
		{
			if (safe_cast<SwgConversationEditorApp *> (AfxGetApp ())->isValidWord (stringList [i]))
			{
				stringList.erase (stringList.begin () + i);
				--n;
			}
			else
				++i;
		}
	}

	if (!stringList.empty ())
	{
		CString buffer ("Spell check warning: ");

		for (uint i = 0; i < stringList.size (); ++i)
			buffer += '\"' + stringList [i] + '\"' + ((i < stringList.size () - 1) ? ", " : "");

		addWarning (buffer, conversationItem);
	}
}

// ----------------------------------------------------------------------

void SwgConversationEditorDoc::spellCheckBranch (ConversationBranch const * branch)
{
	CString text = branch->getText ().c_str ();
	text.MakeLower ();
	StringList stringList;
	spellCheck (text, stringList, branch);

	for (int i = 0; i < branch->getNumberOfResponses (); ++i)
		spellCheckResponse (branch->getResponse (i));
}

// ----------------------------------------------------------------------

void SwgConversationEditorDoc::spellCheckResponse (ConversationResponse const * response)
{
	CString text = response->getText ().c_str ();
	text.MakeLower ();
	StringList stringList;
	spellCheck (text, stringList, response);

	for (int i = 0; i < response->getNumberOfBranches (); ++i)
		spellCheckBranch (response->getBranch (i));
}

// ----------------------------------------------------------------------

void SwgConversationEditorDoc::linkCheckBranch (ConversationBranch const * const branch)
{
	for (int i = 0; i < branch->getNumberOfResponses (); ++i)
		linkCheckResponse (branch->getResponse (i));
}

// ----------------------------------------------------------------------

void SwgConversationEditorDoc::linkCheckResponse (ConversationResponse const * const response)
{
	for (int i = 0; i < response->getNumberOfBranches (); ++i)
		linkCheckBranch (response->getBranch (i));
}

// ----------------------------------------------------------------------

void SwgConversationEditorDoc::labelCheckBranch (ConversationBranch const * const branch)
{
	for (int i = 0; i < branch->getNumberOfResponses (); ++i)
		labelCheckResponse (branch->getResponse (i));
}

// ----------------------------------------------------------------------

void SwgConversationEditorDoc::labelCheckResponse (ConversationResponse const * const response)
{
	for (int i = 0; i < response->getNumberOfBranches (); ++i)
		labelCheckBranch (response->getBranch (i));
}

// ----------------------------------------------------------------------

void SwgConversationEditorDoc::verifyLabels ()
{
	ScriptGroup const * const labelGroup = getLabelGroup ();
	for (int i = 1; i < labelGroup->getNumberOfFamilies (); ++i)
	{
		int const labelFamilyId = labelGroup->getFamilyId (i);

		{
			ConversationItemList labelList;
			findLabelResponse (m_conversation, labelFamilyId, labelList);

			if (labelList.size () > 1)
			{
				CString buffer;
				buffer.Format ("Label '%s' specified more than once", labelGroup->getFamilyName (labelFamilyId));
				addError (buffer, labelList.front ());
			}
		}

		if (i > 1)
		{
			if (labelInUseAsLabel (labelFamilyId) && !labelInUseAsLink (labelFamilyId))
			{
				CString buffer;
				buffer.Format ("Label '%s' specified but not linked to.", labelGroup->getFamilyName (labelFamilyId));
				addWarning (buffer, findLabel (labelFamilyId));
			}
		}
	}
}

// ----------------------------------------------------------------------

void SwgConversationEditorDoc::verifyLinks ()
{
	ScriptGroup const * const labelGroup = getLabelGroup ();
	for (int i = 1; i < labelGroup->getNumberOfFamilies (); ++i)
	{
		int const linkFamilyId = labelGroup->getFamilyId (i);
		ConversationItemList linkList;
		findLinkResponse (m_conversation, linkFamilyId, linkList);

		for (uint j = 0; j < linkList.size (); ++j)
		{
			if (!labelInUseAsLabel (linkFamilyId))
			{
				CString buffer;
				buffer.Format ("Label not found for link '%s'", labelGroup->getFamilyName (linkFamilyId));
				addError (buffer, linkList.front ());
			}
			else
			{
				ConversationItem const * const conversationItem = findLabel (linkFamilyId);

				//-- make sure onlly like conversation items are linked
				if (dynamic_cast<ConversationBranch const *> (linkList [j]))
				{
					if (dynamic_cast<ConversationResponse const *> (conversationItem))
					{
						CString buffer;
						buffer.Format ("Branch with link '%s' is linked to a response", labelGroup->getFamilyName (linkFamilyId));
						addError (buffer, linkList [j]);
					}
					else
					{
						//-- don't allow linking to a link
						if (conversationItem->getLinkFamilyId () != 0)
						{
							CString buffer;
							buffer.Format ("Branch with link '%s' is linked to a branch with link '%s'.  You can't link to a link.", labelGroup->getFamilyName (linkFamilyId), labelGroup->getFamilyName (conversationItem->getLinkFamilyId ()));
							addError (buffer, linkList [j]);
						}
					}
				}
				else
					if (dynamic_cast<ConversationResponse const *> (linkList [j]))
					{
						if (dynamic_cast<ConversationBranch const *> (conversationItem))
						{
							CString buffer;
							buffer.Format ("Response with link '%s' is linked to a branch", labelGroup->getFamilyName (linkFamilyId));
							addError (buffer, linkList [j]);
						}
						else
						{
							//-- don't allow linking to a link
							if (conversationItem->getLinkFamilyId () != 0)
							{
								CString buffer;
								buffer.Format ("Response with link '%s' is linked to a response with link '%s'.  You can't link to a link.", labelGroup->getFamilyName (linkFamilyId), labelGroup->getFamilyName (conversationItem->getLinkFamilyId ()));
								addError (buffer, linkList [j]);
							}
						}
					}
			}
		}
	}
}

// ----------------------------------------------------------------------

void SwgConversationEditorDoc::findTextBranch (ConversationBranch const * branch, CString const & findText, bool const matchCase, bool const wholeWordOnly)
{
	CString text = branch->getText ().c_str ();
	if (!matchCase)
		text.MakeLower ();

	int const index = text.Find(findText);
	if (index != -1)
	{
		bool shouldAdd = false;

		//-- Implement later
		if (wholeWordOnly)
			shouldAdd = true;
		else
			shouldAdd = true;

		if (shouldAdd)
		{
			CString const buffer(CString("Find '") + branch->getText().c_str() + "'");
			addComment (buffer, branch);
		}
	}

	for (int i = 0; i < branch->getNumberOfResponses (); ++i)
		findTextResponse (branch->getResponse (i), findText, matchCase, wholeWordOnly);
}

// ----------------------------------------------------------------------

void SwgConversationEditorDoc::findTextResponse (ConversationResponse const * response, CString const & findText, bool const matchCase, bool const wholeWordOnly)
{
	CString text = response->getText ().c_str ();
	if (!matchCase)
		text.MakeLower ();

	int const index = text.Find(findText);
	if (index != -1)
	{
		bool shouldAdd = false;

		//-- Implement later
		if (wholeWordOnly)
			shouldAdd = true;
		else
			shouldAdd = true;

		if (shouldAdd)
		{
			CString const buffer(CString("Find '") + response->getText().c_str() + "'");
			addComment (buffer, response);
		}
	}

	for (int i = 0; i < response->getNumberOfBranches (); ++i)
		findTextBranch (response->getBranch (i), findText, matchCase, wholeWordOnly);
}

// ----------------------------------------------------------------------

bool SwgConversationEditorDoc::writeScript (CString const & scriptName, CString const & fileName, ConversationItemList const & conversationItemList, bool const debug) const
{
	CStdioFile outfile;
	if (outfile.Open (scriptName, CFile::modeCreate | CFile::modeWrite | CFile::typeText))
	{
		CString buffer;

		//-- write header
		outfile.WriteString (cs_scriptHeader1);
		outfile.WriteString (fileName);
		outfile.WriteString (".script" );
		outfile.WriteString (cs_scriptHeader2);

		//-- write libraries
		outfile.WriteString (cs_scriptLibraryDelimiter);
		writeLibraries (outfile);
		outfile.WriteString ("\n");

		//-- write constants
		outfile.WriteString (cs_scriptConstantsDelimiter);
		outfile.WriteString ("string c_stringFile = \"conversation/");
		outfile.WriteString (fileName);
		outfile.WriteString ("\";\n");
		outfile.WriteString ("\n");

		//-- generate replacements
		std::vector<std::pair<CString, CString> > replacements;
		{
			{
				int const numberOfFamilies = m_conversation->getConditionGroup ()->getNumberOfFamilies ();
				for (int i = 0; i < numberOfFamilies; ++i)
				{
					int const familyId = m_conversation->getConditionGroup ()->getFamilyId (i);
					CString const sourceFamilyName = m_conversation->getConditionGroup ()->getFamilyName (familyId);
					CString const destinationFamilyName = fileName + "_condition_" + sourceFamilyName;
					replacements.push_back (std::make_pair (sourceFamilyName, destinationFamilyName));
				}
			}

			{
				int const numberOfFamilies = m_conversation->getActionGroup ()->getNumberOfFamilies ();
				for (int i = 0; i < numberOfFamilies; ++i)
				{
					int const familyId = m_conversation->getActionGroup ()->getFamilyId (i);
					CString const sourceFamilyName = m_conversation->getActionGroup ()->getFamilyName (familyId);
					CString const destinationFamilyName = fileName + "_action_" + sourceFamilyName;
					replacements.push_back (std::make_pair (sourceFamilyName, destinationFamilyName));
				}
			}

			{
				int const numberOfFamilies = m_conversation->getTokenTOGroup ()->getNumberOfFamilies ();
				for (int i = 0; i < numberOfFamilies; ++i)
				{
					int const familyId = m_conversation->getTokenTOGroup ()->getFamilyId (i);
					CString const sourceFamilyName = m_conversation->getTokenTOGroup ()->getFamilyName (familyId);
					CString const destinationFamilyName = fileName + "_tokenTO_" + sourceFamilyName;
					replacements.push_back (std::make_pair (sourceFamilyName, destinationFamilyName));
				}
			}

			{
				int const numberOfFamilies = m_conversation->getTokenDIGroup ()->getNumberOfFamilies ();
				for (int i = 0; i < numberOfFamilies; ++i)
				{
					int const familyId = m_conversation->getTokenDIGroup ()->getFamilyId (i);
					CString const sourceFamilyName = m_conversation->getTokenDIGroup ()->getFamilyName (familyId);
					CString const destinationFamilyName = fileName + "_tokenDI_" + sourceFamilyName;
					replacements.push_back (std::make_pair (sourceFamilyName, destinationFamilyName));
				}
			}

			{
				int const numberOfFamilies = m_conversation->getTokenDFGroup ()->getNumberOfFamilies ();
				for (int i = 0; i < numberOfFamilies; ++i)
				{
					int const familyId = m_conversation->getTokenDFGroup ()->getFamilyId (i);
					CString const sourceFamilyName = m_conversation->getTokenDFGroup ()->getFamilyName (familyId);
					CString const destinationFamilyName = fileName + "_tokenDF_" + sourceFamilyName;
					replacements.push_back (std::make_pair (sourceFamilyName, destinationFamilyName));
				}
			}
		}

		//-- write conditions
		outfile.WriteString (cs_scriptConditionDelimiter);
		{
			int const numberOfFamilies = getConditionGroup ()->getNumberOfFamilies ();
			for (int i = 0; i < numberOfFamilies; ++i)
			{
				int const familyId = getConditionGroup ()->getFamilyId (i);
				CString text = getConditionGroup ()->getFamilyText (familyId);
				text.Remove ('\r');

				uint const numberOfReplacements = replacements.size ();
				for (uint j = 0; j < numberOfReplacements; ++j)
					replaceWholeString (text, replacements [j].first, replacements [j].second);

				outfile.WriteString (text);
				outfile.WriteString ("\n");

				if (i < numberOfFamilies - 1)
					outfile.WriteString (cs_scriptSingleDelimiter);
			}
		}

		//-- write actions
		outfile.WriteString (cs_scriptActionDelimiter);
		{
			int const numberOfFamilies = getActionGroup ()->getNumberOfFamilies ();
			for (int i = 1; i < numberOfFamilies; ++i)
			{
				int const familyId = getActionGroup ()->getFamilyId (i);
				CString text = getActionGroup ()->getFamilyText (familyId);
				text.Remove ('\r');

				uint const numberOfReplacements = replacements.size ();
				for (uint j = 0; j < numberOfReplacements; ++j)
					replaceWholeString (text, replacements [j].first, replacements [j].second);

				outfile.WriteString (text);
				outfile.WriteString ("\n");

				if (i < numberOfFamilies - 1)
					outfile.WriteString (cs_scriptSingleDelimiter);
			}
		}

		//-- write tokenTOs
		outfile.WriteString (cs_scriptTokenTODelimiter);
		{
			int const numberOfFamilies = getTokenTOGroup ()->getNumberOfFamilies ();
			for (int i = 1; i < numberOfFamilies; ++i)
			{
				int const familyId = getTokenTOGroup ()->getFamilyId (i);
				CString text = getTokenTOGroup ()->getFamilyText (familyId);
				text.Remove ('\r');

				uint const numberOfReplacements = replacements.size ();
				for (uint j = 0; j < numberOfReplacements; ++j)
					replaceWholeString (text, replacements [j].first, replacements [j].second);

				outfile.WriteString (text);
				outfile.WriteString ("\n");

				if (i < numberOfFamilies - 1)
					outfile.WriteString (cs_scriptSingleDelimiter);
			}
		}

		//-- write tokenDIs
		outfile.WriteString (cs_scriptTokenDIDelimiter);
		{
			int const numberOfFamilies = getTokenDIGroup ()->getNumberOfFamilies ();
			for (int i = 1; i < numberOfFamilies; ++i)
			{
				int const familyId = getTokenDIGroup ()->getFamilyId (i);
				CString text = getTokenDIGroup ()->getFamilyText (familyId);
				text.Remove ('\r');

				uint const numberOfReplacements = replacements.size ();
				for (uint j = 0; j < numberOfReplacements; ++j)
					replaceWholeString (text, replacements [j].first, replacements [j].second);

				outfile.WriteString (text);
				outfile.WriteString ("\n");

				if (i < numberOfFamilies - 1)
					outfile.WriteString (cs_scriptSingleDelimiter);
			}
		}

		//-- write tokenDFs
		outfile.WriteString (cs_scriptTokenDFDelimiter);
		{
			int const numberOfFamilies = getTokenDFGroup ()->getNumberOfFamilies ();
			for (int i = 1; i < numberOfFamilies; ++i)
			{
				int const familyId = getTokenDFGroup ()->getFamilyId (i);
				CString text = getTokenDFGroup ()->getFamilyText (familyId);
				text.Remove ('\r');

				uint const numberOfReplacements = replacements.size ();
				for (uint j = 0; j < numberOfReplacements; ++j)
					replaceWholeString (text, replacements [j].first, replacements [j].second);

				outfile.WriteString (text);
				outfile.WriteString ("\n");

				if (i < numberOfFamilies - 1)
					outfile.WriteString (cs_scriptSingleDelimiter);
			}
		}

		//-- Write branch functions
		outfile.WriteString (cs_scriptHandleBranchDelimiter);
		{
			ConversationItemList::const_iterator end = conversationItemList.end ();
			for (ConversationItemList::const_iterator iterator = conversationItemList.begin (); iterator != end; ++iterator)
			{
				ConversationItem const * const conversationItem = *iterator;
				ConversationBranch const * const sourceBranch = dynamic_cast<ConversationBranch const *> (conversationItem);
				if (sourceBranch)
				{
					ConversationBranch const * destinationBranch = sourceBranch;
					if (sourceBranch->getLinkFamilyId () != 0)
					{
						ConversationBranch const * const branch = safe_cast<ConversationBranch const *> (findLabel (sourceBranch->getLinkFamilyId ()));
						if (branch)
							destinationBranch = branch;
					}

					int const numberOfResponses = destinationBranch->getNumberOfResponses ();
					if (numberOfResponses > 0)
					{
						// int *_handleBranch<n> (obj_id player, obj_id npc, string_id response)
						buffer.Format("int %s_handleBranch%i (obj_id player, obj_id npc, string_id response)\n", fileName, sourceBranch->getBranchId());
						outfile.WriteString(buffer);

						// {
						outfile.WriteString("{\n");

						//	//-- comment BRANCH NOTE: <branchNote>
						buffer.Format ("\t//-- [BRANCH NOTE] %s\n", getFirstLine (sourceBranch->getNotes ().c_str ()));
						outfile.WriteString (buffer);

						//	//-- comment NPC: <branchText>
						buffer.Format ("\t//-- NPC: %s\n\n", getFirstLine (sourceBranch->getText ().c_str ()));
						outfile.WriteString (buffer);

						for (int i = 0; i < numberOfResponses; ++i)
						{
							ConversationResponse const * const response = destinationBranch->getResponse (i);
							if (!debug && response->getDebug ())
								continue;

							//	//-- comment RESPONSE NOTE: <responseText>
							buffer.Format ("\t//-- [RESPONSE NOTE] %s\n", getFirstLine (response->getNotes ().c_str ()));
							outfile.WriteString (buffer);

							//	//-- comment PLAYER: <responseNote>
							buffer.Format ("\t//-- PLAYER: %s\n", getFirstLine (response->getText ().c_str ()));
							outfile.WriteString (buffer);

							//	if (response == "<responseTextCrc>")
							buffer.Format("\tif (response == \"s_%s\")\n", response->getStringId().c_str());
							outfile.WriteString (buffer);
	
							//	{
							buffer.Format ("\t{\n");
							outfile.WriteString (buffer);

							//		//-- Do player animation action
							if (CString (response->getPlayerAnimation ().c_str ()) != Configuration::getEmptyAnimationAction ())
							{
								buffer.Format ("\t\tdoAnimationAction (player, \"%s\");\n\n", response->getPlayerAnimation ().c_str());
								outfile.WriteString (buffer);
							}

							//		//-- Echo player speech to group
							if (response->getGroupEcho ())
							{
								buffer.Format ("\t\tconversation.echoToGroup (player, player, npc, response);\n\n");
								outfile.WriteString (buffer);
							}

							//		<responseAction> (player, npc);
							int const actionFamilyId = response->getActionFamilyId ();
							if (actionFamilyId != 0)
							{
								buffer.Format ("\t\t%s_action_%s (player, npc);\n\n", fileName, getActionGroup ()->getFamilyName (actionFamilyId));
								outfile.WriteString (buffer);
							}

							writeResponseChildBranches (outfile, fileName, 2, response, false, debug);
				
							buffer.Format ("\t}\n\n");
							outfile.WriteString (buffer);
						}

						outfile.WriteString("\treturn SCRIPT_DEFAULT;\n");
						outfile.WriteString("}\n\n");

						outfile.WriteString (cs_scriptSingleDelimiter);
					}
				}
			}
		}

		//-- write script triggers
		outfile.WriteString (cs_scriptUserTriggerDelimiter);
		{
			CString text = m_conversation->getTriggerText ().c_str ();
			text.Replace ("\r\n", "\n");
			outfile.WriteString (text);
			outfile.WriteString ("\n");
		}

		//-- write OnStartNpcConversation
		outfile.WriteString (cs_scriptTriggerDelimiter);
		{
			outfile.WriteString ("//-- This function should move to base_class.java\n");
			outfile.WriteString ("boolean npcStartConversation(obj_id player, obj_id npc, string convoName, string_id greetingId, prose_package greetingProse, string_id[] responses)\n");
			outfile.WriteString ("{\n");
			outfile.WriteString ("\tObject[] objects = new Object[responses.length];\n");
			outfile.WriteString ("\tSystem.arraycopy(responses, 0, objects, 0, responses.length);\n");
			outfile.WriteString ("\treturn npcStartConversation(player, npc, convoName, greetingId, greetingProse, objects);\n");
			outfile.WriteString ("}\n\n");
			outfile.WriteString (cs_scriptSingleDelimiter);
		}

		{
			outfile.WriteString ("trigger OnStartNpcConversation (obj_id player)\n");
			outfile.WriteString ("{\n");

			//	obj_id npc = self;
			outfile.WriteString("\tobj_id npc = self;\n\n");

			//	if (ai_lib.isInCombat (npc) || ai_lib.isInCombat (player))
			outfile.WriteString ("\tif (ai_lib.isInCombat (npc) || ai_lib.isInCombat (player))\n");
			//		return SCRIPT_OVERRIDE;
			outfile.WriteString ("\t\treturn SCRIPT_OVERRIDE;\n\n");

			writeResponseChildBranches (outfile, fileName, 1, m_conversation, true, debug);

			//	chat.chat (npc, \"Error:  All conditions for OnStartNpcConversation were false.\");
			outfile.WriteString ("\tchat.chat (npc, \"Error:  All conditions for OnStartNpcConversation were false.\");\n\n");

			//	return SCRIPT_CONTINUE;
			outfile.WriteString ("\treturn SCRIPT_CONTINUE;\n");
			outfile.WriteString ("}\n\n");
		}

		//-- write OnNpcConversationResponse
		outfile.WriteString (cs_scriptSingleDelimiter);
		{
			outfile.WriteString ("trigger OnNpcConversationResponse (string conversationId, obj_id player, string_id response)\n{\n");

			//	if (conversationId != <conversationId>)
			buffer.Format ("\tif (conversationId != \"%s\")\n", fileName);
			outfile.WriteString (buffer);

			//		return SCRIPT_CONTINUE;
			outfile.WriteString ("\t\treturn SCRIPT_CONTINUE;\n\n");

			//	obj_id npc = self;
			outfile.WriteString("\tobj_id npc = self;\n\n");

			//	int branchId = utils.getIntScriptVar (player, "conversation.<conversationId>.branchId");
			buffer.Format ("\tint branchId = utils.getIntScriptVar (player, \"conversation.%s.branchId\");\n\n", fileName);
			outfile.WriteString (buffer);

			ConversationItemList::const_iterator end = conversationItemList.end ();
			for (ConversationItemList::const_iterator iterator = conversationItemList.begin (); iterator != end; ++iterator)
			{
				ConversationItem const * const conversationItem = *iterator;
				ConversationBranch const * const sourceBranch = dynamic_cast<ConversationBranch const *> (conversationItem);
				if (sourceBranch)
				{
					ConversationBranch const * destinationBranch = sourceBranch;
					if (sourceBranch->getLinkFamilyId () != 0)
					{
						ConversationBranch const * const branch = safe_cast<ConversationBranch const *> (findLabel (sourceBranch->getLinkFamilyId ()));
						if (branch)
							destinationBranch = branch;
					}

					int const numberOfResponses = destinationBranch->getNumberOfResponses ();
					if (numberOfResponses > 0)
					{
						buffer.Format("\tif (branchId == %i && %s_handleBranch%i (player, npc, response) == SCRIPT_CONTINUE)\n", sourceBranch->getBranchId(), fileName, sourceBranch->getBranchId());
						outfile.WriteString(buffer);

						outfile.WriteString("\t\treturn SCRIPT_CONTINUE;\n\n");
					}
				}
			}

			//	chat.chat (npc, "Error:  All conditions for OnStartNpcConversation were false.");
			buffer.Format ("\tchat.chat (npc, \"Error:  Fell through all branches and responses for OnNpcConversationResponse.\");\n\n");
			outfile.WriteString (buffer);

			//	utils.removeScriptVar (player, "conversation.<conversationId>.branchId");
			buffer.Format ("\tutils.removeScriptVar (player, \"conversation.%s.branchId\");\n\n", fileName);
			outfile.WriteString (buffer);

			outfile.WriteString ("\treturn SCRIPT_CONTINUE;\n}\n\n");
		}

		outfile.WriteString (cs_scriptDoubleDelimiter);

		return true;
	}

	return false;
}

// ----------------------------------------------------------------------

void SwgConversationEditorDoc::writeLibraries (CStdioFile & outfile) const
{
	Conversation::LibrarySet const & librarySet = getConversation ()->getLibrarySet ();
	Conversation::LibrarySet::iterator end = librarySet.end ();
	for (Conversation::LibrarySet::iterator iterator = librarySet.begin (); iterator != end; ++iterator)
	{
		outfile.WriteString ("include library.");
		outfile.WriteString (iterator->c_str ());
		outfile.WriteString (";\n");
	}
}

// ----------------------------------------------------------------------

void SwgConversationEditorDoc::writeResponseChildBranches (CStdioFile & outfile, CString const & fileName, int const indentLevel, ConversationResponse const * const sourceResponse, bool const start, bool const debug) const
{
	ConversationResponse const * destinationResponse = sourceResponse;
	if (sourceResponse->getLinkFamilyId () != 0)
	{
		ConversationResponse const * const response = safe_cast<ConversationResponse const *> (findLabel (sourceResponse->getLinkFamilyId ()));
		if (response)
			destinationResponse = response;
	}

	CString indentation;
	{
		for (int i = 0; i < indentLevel; ++i)
			indentation += "\t";
	}

	CString buffer;

	int const numberOfBranches = destinationResponse->getNumberOfBranches ();
	for (int i = 0; i < numberOfBranches; ++i)
	{
		ConversationBranch const * const sourceBranch = destinationResponse->getBranch (i);
		if (!debug && sourceBranch->getDebug ())
			continue;

		ConversationBranch const * destinationBranch = sourceBranch;
		if (sourceBranch->getLinkFamilyId () != 0)
		{
			ConversationBranch const * const branch = safe_cast<ConversationBranch const *> (findLabel (sourceBranch->getLinkFamilyId ()));
			if (branch)
				destinationBranch = branch;
		}

	//	//-- comment NOTE: <branchNotes> 
		buffer.Format ("//-- [NOTE] %s\n", getFirstLine (sourceBranch->getNotes ().c_str ()));
		outfile.WriteString (indentation + buffer);

	//	if (<branchCondition> (player, npc))
		int const conditionFamilyId = sourceBranch->getConditionFamilyId ();
		buffer.Format ("if (%s%s_condition_%s (player, npc))\n", sourceBranch->getNegateCondition () ? "!" : "", fileName, getConditionGroup ()->getFamilyName (conditionFamilyId));
		outfile.WriteString (indentation + buffer);

	//	{
		buffer.Format ("{\n");
		outfile.WriteString (indentation + buffer);


	//		//-- Do npc animation action
		if (CString (sourceBranch->getNpcAnimation ().c_str ()) != Configuration::getEmptyAnimationAction ())
		{
			buffer.Format ("\tdoAnimationAction (npc, \"%s\");\n\n", sourceBranch->getNpcAnimation ().c_str());
			outfile.WriteString (indentation + buffer);
		}

	//		//-- Do player animation action
		if (CString (sourceBranch->getPlayerAnimation ().c_str ()) != Configuration::getEmptyAnimationAction ())
		{
			buffer.Format ("\tdoAnimationAction (player, \"%s\");\n\n", sourceBranch->getPlayerAnimation ().c_str());
			outfile.WriteString (indentation + buffer);
		}

	//		<branchAction> (player, npc);
		int const actionFamilyId = sourceBranch->getActionFamilyId ();
		if (actionFamilyId != 0)
		{
			buffer.Format ("\t%s_action_%s (player, npc);\n\n", fileName, getActionGroup ()->getFamilyName (actionFamilyId));
			outfile.WriteString (indentation + buffer);
		}

	//	//-- comment NPC: <branchText> 
		buffer.Format ("\t//-- NPC: %s\n", getFirstLine (sourceBranch->getText ().c_str ()));
		outfile.WriteString (indentation + buffer);

	//		string_id message = new string_id (c_stringFile, "<branchTextCrc>");
		buffer.Format("\tstring_id message = new string_id (c_stringFile, \"s_%s\");\n", sourceBranch->getStringId().c_str());
		outfile.WriteString (indentation + buffer);
	
		int const numberOfResponses = destinationBranch->getNumberOfResponses ();
		if (numberOfResponses > 0)
		{
			{
	//		int numberOfResponses = 0;
				buffer.Format ("\tint numberOfResponses = 0;\n\n");
				outfile.WriteString (indentation + buffer);

	//		boolean hasResponse = false;
				buffer.Format ("\tboolean hasResponse = false;\n\n");
				outfile.WriteString (indentation + buffer);

				for (int j = 0; j < numberOfResponses; ++j)
				{
					ConversationResponse const * const response = destinationBranch->getResponse (j);
					if (!debug && response->getDebug ())
						continue;

	//		//-- comment PLAYER: <responseText> 
					buffer.Format ("\t//-- PLAYER: %s\n", getFirstLine (response->getText ().c_str ()));
					outfile.WriteString (indentation + buffer);

	//		boolean hasResponse<responseIndex> = false;
					buffer.Format ("\tboolean hasResponse%i = false;\n", j);
					outfile.WriteString (indentation + buffer);
	
	//		if (<responseCondition> (player, npc))
					int const conditionFamilyId = response->getConditionFamilyId ();
					buffer.Format ("\tif (%s%s_condition_%s (player, npc))\n", response->getNegateCondition () ? "!" : "", fileName, getConditionGroup ()->getFamilyName (conditionFamilyId));
					outfile.WriteString (indentation + buffer);

	//		{
					buffer.Format ("\t{\n");
					outfile.WriteString (indentation + buffer);

	//			++numberOfResponses;
					buffer.Format ("\t\t++numberOfResponses;\n");
					outfile.WriteString (indentation + buffer);

	//			hasResponse = true;
					buffer.Format ("\t\thasResponse = true;\n");
					outfile.WriteString (indentation + buffer);

	//			hasResponse<responseIndex> = true;
					buffer.Format ("\t\thasResponse%i = true;\n", j);
					outfile.WriteString (indentation + buffer);

	//		}
					buffer.Format ("\t}\n\n");
					outfile.WriteString (indentation + buffer);
				}
			}

	//		if (hasResponse)
			buffer.Format ("\tif (hasResponse)\n");
			outfile.WriteString (indentation + buffer);

	//		{
			buffer.Format ("\t{\n");
			outfile.WriteString (indentation + buffer);

			{
	//			int responseIndex = 0;
				buffer.Format ("\t\tint responseIndex = 0;\n");
				outfile.WriteString (indentation + buffer);

	//			string_id responses [] = new string_id [<numberOfResponses>];
				buffer.Format ("\t\tstring_id responses [] = new string_id [numberOfResponses];\n\n");
				outfile.WriteString (indentation + buffer);

				for (int j = 0; j < numberOfResponses; ++j)
				{
					ConversationResponse const * const response = destinationBranch->getResponse (j);
					if (!debug && response->getDebug ())
						continue;
	
	//			if (hasResponse<responseIndex>)
					buffer.Format ("\t\tif (hasResponse%i)\n", j);
					outfile.WriteString (indentation + buffer);

	//				responses [responseIndex++] = new string_id (c_stringFile, "<responseTextCrc>");
					buffer.Format("\t\t\tresponses [responseIndex++] = new string_id (c_stringFile, \"s_%s\");\n\n", response->getStringId().c_str());
					outfile.WriteString (indentation + buffer);
				}
			}

	//			utils.setScriptVar (player, "conversation.<conversationId>.branchId", <branchId>);
			buffer.Format ("\t\tutils.setScriptVar (player, \"conversation.%s.branchId\", %i);\n\n", fileName, destinationBranch->getBranchId ());
			outfile.WriteString (indentation + buffer);

			if (start)
			{
				if (sourceBranch->getUseProsePackage())
				{
	//				prose_package setup
					writeProsePackage (outfile, fileName, indentLevel + 2, sourceBranch);

	//				npcStartConversation (speaker, npc, "celebConvo", null, prose_package, responses);
					buffer.Format ("\t\tnpcStartConversation (player, npc, \"%s\", null, pp, responses);\n", fileName);
					outfile.WriteString (indentation + buffer);

					if (sourceBranch->getGroupEcho())
					{
	//					conversation.echoToGroup (player, npc, player, pp);
						buffer.Format("\t\tconversation.echoToGroup (player, npc, player, pp);\n");
						outfile.WriteString (indentation + buffer);
					}
				}
				else
				{
	//				npcStartConversation (speaker, npc, "celebConvo", message, responses);
					buffer.Format ("\t\tnpcStartConversation (player, npc, \"%s\", message, responses);\n", fileName);
					outfile.WriteString (indentation + buffer);

					if (sourceBranch->getGroupEcho())
					{
	//					conversation.echoToGroup (player, npc, player, message);
						buffer.Format("\t\tconversation.echoToGroup (player, npc, player, message);\n");
						outfile.WriteString (indentation + buffer);
					}
				}
			}
			else
			{
				if (sourceBranch->getUseProsePackage())
				{
	//				prose_package setup
					writeProsePackage (outfile, fileName, indentLevel + 2, sourceBranch);

	//				npcSpeak (player, pp);
					buffer.Format ("\t\tnpcSpeak (player, pp);\n");
					outfile.WriteString (indentation + buffer);

					if (sourceBranch->getGroupEcho())
					{
	//					conversation.echoToGroup (player, npc, player, pp);
						buffer.Format("\t\tconversation.echoToGroup (player, npc, player, pp);\n");
						outfile.WriteString (indentation + buffer);
					}
				}
				else
				{
	//				npcSpeak (player, message);
					buffer.Format ("\t\tnpcSpeak (player, message);\n");
					outfile.WriteString (indentation + buffer);

					if (sourceBranch->getGroupEcho())
					{
	//					conversation.echoToGroup (player, npc, player, message);
						buffer.Format("\t\tconversation.echoToGroup (player, npc, player, message);\n");
						outfile.WriteString (indentation + buffer);
					}
				}

	//			npcSetConversationResponses (player, responses);
				buffer.Format ("\t\tnpcSetConversationResponses (player, responses);\n");
				outfile.WriteString (indentation + buffer);
			}

	//		}
			buffer.Format ("\t}\n");
			outfile.WriteString (indentation + buffer);

	//		else
			buffer.Format ("\telse\n");
			outfile.WriteString (indentation + buffer);

	//		{
			buffer.Format ("\t{\n");
			outfile.WriteString (indentation + buffer);

			if (start)
			{
				if (sourceBranch->getUseProsePackage())
				{
	//				prose_package setup
					writeProsePackage (outfile, fileName, indentLevel + 2, sourceBranch);

	//				chat.chat (npc, player, null, null, pp);
					buffer.Format ("\t\tchat.chat (npc, player, null, null, pp);\n");
					outfile.WriteString (indentation + buffer);

					if (sourceBranch->getGroupEcho())
					{
	//					conversation.echoToGroup (player, npc, player, pp);
						buffer.Format("\t\tconversation.echoToGroup (player, npc, player, pp);\n");
						outfile.WriteString (indentation + buffer);
					}
				}
				else
				{
	//				chat.chat (npc, player, message);
					buffer.Format ("\t\tchat.chat (npc, player, message);\n");
					outfile.WriteString (indentation + buffer);

					if (sourceBranch->getGroupEcho())
					{
	//					conversation.echoToGroup (player, npc, player, message);
						buffer.Format("\t\tconversation.echoToGroup (player, npc, player, message);\n");
						outfile.WriteString (indentation + buffer);
					}
				}
			}
			else
			{
	//			utils.removeScriptVar (player, "conversation.<conversationId>.branchId");
				buffer.Format ("\t\tutils.removeScriptVar (player, \"conversation.%s.branchId\");\n\n", fileName);
				outfile.WriteString (indentation + buffer);

				if (sourceBranch->getUseProsePackage())
				{
	//				prose_package setup
					writeProsePackage (outfile, fileName, indentLevel + 2, sourceBranch);

	//				npcEndConversationWithMessage (player, pp);
					buffer.Format ("\t\tnpcEndConversationWithMessage (player, pp);\n");
					outfile.WriteString (indentation + buffer);

					if (sourceBranch->getGroupEcho())
					{
	//					conversation.echoToGroup (player, npc, player, pp);
						buffer.Format("\t\tconversation.echoToGroup (player, npc, player, pp);\n");
						outfile.WriteString (indentation + buffer);
					}

				}
				else
				{
	//				npcEndConversationWithMessage (player, message);
					buffer.Format ("\t\tnpcEndConversationWithMessage (player, message);\n");
					outfile.WriteString (indentation + buffer);

					if (sourceBranch->getGroupEcho())
					{
	//					conversation.echoToGroup (player, npc, player, message);
						buffer.Format("\t\tconversation.echoToGroup (player, npc, player, message);\n");
						outfile.WriteString (indentation + buffer);
					}
				}
			}

	//		}
			buffer.Format ("\t}\n\n");
			outfile.WriteString (indentation + buffer);
		}
		else
		{
			if (start)
			{
				if (sourceBranch->getUseProsePackage())
				{
	//				prose_package setup
					writeProsePackage (outfile, fileName, indentLevel + 2, sourceBranch);

	//				chat.chat (npc, player, null, null, pp);
					buffer.Format ("\t\tchat.chat (npc, player, null, null, pp);\n");
					outfile.WriteString (indentation + buffer);

					if (sourceBranch->getGroupEcho())
					{
	//					conversation.echoToGroup (player, npc, player, pp);
						buffer.Format("\t\tconversation.echoToGroup (player, npc, player, pp);\n\n");
						outfile.WriteString (indentation + buffer);
					}
				}
				else
				{
	//				chat.chat (npc, player, message);
					buffer.Format ("\tchat.chat (npc, player, message);\n\n");
					outfile.WriteString (indentation + buffer);

					if (sourceBranch->getGroupEcho())
					{
	//					conversation.echoToGroup (player, npc, player, message);
						buffer.Format("\tconversation.echoToGroup (player, npc, player, message);\n\n");
						outfile.WriteString (indentation + buffer);
					}
				}
			}
			else
			{
	//			utils.removeScriptVar (player, "conversation.<conversationId>.branchId");
				buffer.Format ("\tutils.removeScriptVar (player, \"conversation.%s.branchId\");\n\n", fileName);
				outfile.WriteString (indentation + buffer);

				if (sourceBranch->getUseProsePackage())
				{
	//				prose_package setup
					writeProsePackage (outfile, fileName, indentLevel + 1, sourceBranch);

	//				npcEndConversationWithMessage (player, pp);
					buffer.Format ("\tnpcEndConversationWithMessage (player, pp);\n\n");
					outfile.WriteString (indentation + buffer);

					if (sourceBranch->getGroupEcho())
					{
	//					conversation.echoToGroup (player, npc, player, pp);
						buffer.Format("\tconversation.echoToGroup (player, npc, player, pp);\n\n");
						outfile.WriteString (indentation + buffer);
					}
				}
				else
				{
	//				npcEndConversationWithMessage (player, message);
					buffer.Format ("\tnpcEndConversationWithMessage (player, message);\n\n");
					outfile.WriteString (indentation + buffer);

					if (sourceBranch->getGroupEcho())
					{
	//					conversation.echoToGroup (player, npc, player, message);
						buffer.Format("\tconversation.echoToGroup (player, npc, player, message);\n\n");
						outfile.WriteString (indentation + buffer);
					}
				}
			}
		}

	//		return SCRIPT_CONTINUE;
		buffer.Format ("\treturn SCRIPT_CONTINUE;\n");
		outfile.WriteString (indentation + buffer);

	//	}
		buffer.Format ("}\n\n");
		outfile.WriteString (indentation + buffer);
	}
}

// ----------------------------------------------------------------------

void SwgConversationEditorDoc::writeProsePackage (CStdioFile & outfile, CString const & fileName, int const indentLevel, ConversationItem const * const conversationItem) const
{
	CString indentation;
	{
		for (int i = 0; i < indentLevel; ++i)
			indentation += "\t";
	}

	CString buffer;

	buffer.Format ("prose_package pp = new prose_package ();\n");
	outfile.WriteString (indentation + buffer);

	buffer.Format ("pp.stringId = message;\n");
	outfile.WriteString (indentation + buffer);
	
	buffer.Format ("pp.actor.set (player);\n");
	outfile.WriteString (indentation + buffer);
	
	buffer.Format ("pp.target.set (npc);\n");
	outfile.WriteString (indentation + buffer);
	
	int const tokenTOFamilyId = conversationItem->getTokenTOFamilyId ();
	if (tokenTOFamilyId != 0)
	{
		buffer.Format ("pp.other.set (%s_tokenTO_%s (player, npc));\n", fileName, getTokenTOGroup ()->getFamilyName (tokenTOFamilyId));
		outfile.WriteString (indentation + buffer);
	}

	int const tokenDIFamilyId = conversationItem->getTokenDIFamilyId ();
	if (tokenDIFamilyId != 0)
	{
		buffer.Format ("pp.digitInteger = %s_tokenDI_%s (player, npc);\n", fileName, getTokenDIGroup ()->getFamilyName (tokenDIFamilyId));
		outfile.WriteString (indentation + buffer);
	}

	int const tokenDFFamilyId = conversationItem->getTokenDFFamilyId ();
	if (tokenDFFamilyId != 0)
	{
		buffer.Format ("pp.digitFloat = %s_tokenDF_%s (player, npc);\n", fileName, getTokenDFGroup ()->getFamilyName (tokenDFFamilyId));
		outfile.WriteString (indentation + buffer);
	}

	outfile.WriteString ("\n");
}

// ----------------------------------------------------------------------

bool SwgConversationEditorDoc::writeStringTable (CString const & stringTableName, CString const & fileName, ConversationItemList const & conversationItemList) const
{
	StringTable stringTable;
	CString const key = "do_not_edit";
	CString const value = "The English version of this file (" + fileName + ".stf) is automatically generated by the SwgConversationEditor.";
	stringTable.setString (std::string (key), std::string (value));

	for (uint i = 0; i < static_cast<size_t> (conversationItemList.size ()); ++i)
	{
		ConversationItem const * const conversationItem = conversationItemList [i];
		CString buffer;
		buffer.Format("s_%s", conversationItem->getStringId().c_str());

		stringTable.setString(std::string(buffer), conversationItem->getText());
	}

	return stringTable.save (stringTableName);
}

// ----------------------------------------------------------------------

void SwgConversationEditorDoc::OnButtonSpellcheck() 
{
	compile (true, false, false, false);
}

// ----------------------------------------------------------------------

void SwgConversationEditorDoc::OnButtonScan() 
{
	compile (false, true, false, false);
}

// ----------------------------------------------------------------------

void SwgConversationEditorDoc::OnButtonCompiledebug() 
{
	compile (false, true, true, true);
}

// ----------------------------------------------------------------------

void SwgConversationEditorDoc::OnButtonCompilerelease() 
{
	compile (false, true, true, false);
}

// ----------------------------------------------------------------------

void SwgConversationEditorDoc::OnButtonP4edit() 
{
	CString pathName = GetPathName ();
	pathName.MakeLower ();

	FileName fileName (pathName);
	fileName.stripPathAndExt ();

	if (strlen (fileName) == 0)
	{
		CString const message = "Please save the conversation before editing/adding to Perforce.";
		IGNORE_RETURN (MessageBox (0, message, AfxGetApp ()->m_pszAppName, MB_OK));
		return;
	}

	CString const shortFileName (fileName);

	safe_cast<SwgConversationEditorApp *> (AfxGetApp ())->onViewShell (this);

	ShellData * const shellData = new ShellData;
	shellData->m_shortFileName = shortFileName;
	shellData->m_fullScriptFileName = getFullScriptFileName (shortFileName);
	shellData->m_fullStringTableFileName = getFullStringTableFileName (shortFileName);
	UpdateAllViews (0, H_shellP4edit, shellData);
}

// ----------------------------------------------------------------------

void SwgConversationEditorDoc::OnButtonFind() 
{
	DialogFindText dialog;
	if (dialog.DoModal () == IDOK)
	{
		safe_cast<SwgConversationEditorApp *> (AfxGetApp ())->onViewWarning (this);
		m_warningFrame->clear ();

		m_numberOfComments = 0;

		CString searchString = dialog.m_text;
		if (!dialog.m_matchCase)
			searchString.MakeLower();

		findTextResponse (m_conversation, searchString, dialog.m_matchCase == TRUE, dialog.m_wholeWord == TRUE);

		if (m_numberOfComments == 0)
		{
			m_warningFrame->DestroyWindow ();

			CString buffer;
			buffer.Format ("'%s' not found", dialog.m_text);
			MessageBox (0, buffer, AfxGetApp ()->m_pszAppName, MB_OK);
		}
	}
}

// ----------------------------------------------------------------------

void SwgConversationEditorDoc::OnFileExport() 
{
	CFileDialog dlg(false, "*.txt", 0, OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT, "Text Files *.txt|*.txt||");

	dlg.m_ofn.lpstrInitialDir = RecentDirectory::find("Export");
	dlg.m_ofn.lpstrDefExt = "txt";
	dlg.m_ofn.lpstrTitle= "Exporting Text File";

	if (dlg.DoModal() != IDOK)
		return;

	CStdioFile outfile;
	if (!outfile.Open(dlg.GetPathName(), CFile::modeCreate | CFile::modeWrite | CFile::typeText))
		return;

	IGNORE_RETURN (RecentDirectory::update("Export", dlg.GetPathName ()));

	ConversationItemList conversationItemList;
	int branchId = 0;

	prepareCompileResponse (m_conversation, conversationItemList, branchId);

	//-- create a vector of items to expression tags (n1_1a, etc.)
	std::vector<CString> itemTags;
	{
		std::map<int, int> depthLineCount;
		int conversationNumber = 0;

		for (uint i = 0; i < static_cast<size_t>(conversationItemList.size()); ++i)
		{
			ConversationItem const * const item = conversationItemList[i];
			int depth = item->getDepth();
			CString buffer;

			if (depth == 0)
			{
				itemTags.push_back("");
				continue;
			}

			if (depth == 1)
			{
				++conversationNumber;
				depthLineCount.clear();

				buffer.Format("N%d", conversationNumber);
			}
			else
			{
				bool player = (depth % 2) == 0;

				buffer.Format("%s%d_%d%s",
					player ? "P" : "N",
					conversationNumber,
					player ? depth / 2 : (depth - 1) / 2,
					getLetters(depthLineCount[depth]++));
			}

			itemTags.push_back(buffer);
		}
	}

	//-- write the file
	{
		for (uint i = 0; i < static_cast<size_t>(conversationItemList.size()); ++i)
		{
			ConversationItem const * const item = conversationItemList[i];
			int depth = item->getDepth();
			
			CString buffer;
			CString link = "";

			if (depth == 0)
				continue;

			if (item->getLinkFamilyId() != 0)
			{
				ConversationItemList::iterator it = conversationItemList.begin();

				int count = 0;
				for (; it != conversationItemList.end(); ++it, ++count)
					if ((*it)->getLabelFamilyId() == item->getLinkFamilyId())
						break;

				if (it == conversationItemList.end())
					link = " (linked to ***UNKNOWN***)";
				else
					link.Format(" (linked to %s)", itemTags[count]);
			}

			if (depth == 1)
			{
				buffer.Format("\n%s : [%s]%s\n\n",
					itemTags[i],
					item->getText().c_str(),
					link);
			}
			else
			{
				buffer.Format("%s%s : [%s]%s\n\n",
					getIndentation(depth - 1),
					itemTags[i],
					item->getText().c_str(),
					link);
			}

			outfile.WriteString(buffer);
		}
	}

	outfile.Close();
}

// ======================================================================
