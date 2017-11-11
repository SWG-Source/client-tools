// ======================================================================
//
// SwgCuiConsole.cpp
// copyright (c) 2001 Sony Online Entertainment
//
// ======================================================================

#include "swgClientUserInterface/FirstSwgClientUserInterface.h"
#include "swgClientUserInterface/SwgCuiConsole.h"

#include "UIPage.h"
#include "UIText.h"
#include "UITextbox.h"
#include "clientGame/Game.h"
#include "clientUserInterface/CuiChatRoomManager.h"
#include "clientUserInterface/CuiChatRoomManagerStatusMessage.h"
#include "clientUserInterface/CuiCommandTableParser.h"
#include "clientUserInterface/CuiConsoleHelper.h"
#include "sharedCommandParser/CommandParserHistory.h"
#include "sharedFoundation/NetworkId.h"
#include "sharedMessageDispatch/Transceiver.h"
#include "sharedNetworkMessages/ConsoleChannelMessages.h"
#include "sharedNetworkMessages/GalaxyLoopTimesResponse.h"
#include "sharedUtility/CurrentUserOptionManager.h"
#include "sharedUtility/LocalMachineOptionManager.h"
#include "swgClientUserInterface/SwgCuiCommandParserAudio.h"
#include "swgClientUserInterface/SwgCuiCommandParserCombat.h"
#include "swgClientUserInterface/SwgCuiCommandParserCustomization.h"
#include "swgClientUserInterface/SwgCuiCommandParserDefault.h"
#include "swgClientUserInterface/SwgCuiCommandParserLocale.h"
#include "swgClientUserInterface/SwgCuiCommandParserMount.h"
#include "swgClientUserInterface/SwgCuiCommandParserNet.h"
#include "swgClientUserInterface/SwgCuiCommandParserParticle.h"
#include "swgClientUserInterface/SwgCuiCommandParserRemote.h"
#include "swgClientUserInterface/SwgCuiCommandParserScene.h"
#include "swgClientUserInterface/SwgCuiCommandParserUI.h"
#include "swgClientUserInterface/SwgCuiCommandParserVideo.h"
#include "unicodeArchive/UnicodeArchive.h"

//-----------------------------------------------------------------

struct DefaultParserStrategy : public CuiConsoleHelperParserStrategy
{
	explicit                DefaultParserStrategy (CommandParserHistory * history);
	                       ~DefaultParserStrategy ();

	bool                    parse            (const Unicode::String & str, Unicode::String & result) const;
	bool                    tabCompleteToken (const Unicode::String & str, size_t tokenPos, std::vector<Unicode::String> & results, size_t & token_start, size_t & token_end) const;
	CommandParserHistory *  getHistory () const;
private:

	                        DefaultParserStrategy ();
	                        DefaultParserStrategy (const DefaultParserStrategy &);
	DefaultParserStrategy & operator= (const DefaultParserStrategy &);

	CommandParser *         m_parser;
	CommandParser *         m_commandTableParser;

};

//-----------------------------------------------------------------

DefaultParserStrategy::DefaultParserStrategy (CommandParserHistory * history) :
CuiConsoleHelperParserStrategy (),
m_parser (new SwgCuiCommandParserDefault (history)),
m_commandTableParser (new CuiCommandTableParser())
{
	m_parser->addSubCommand (new SwgCuiCommandParserCombat ());
	m_parser->addSubCommand (new SwgCuiCommandParserUI ());
	m_parser->addSubCommand (new SwgCuiCommandParserNet ());
	m_parser->addSubCommand (new SwgCuiCommandParserScene ());
	m_parser->addSubCommand (new SwgCuiCommandParserLocale ());
	m_parser->addSubCommand (new SwgCuiCommandParserRemote ());
	m_parser->addSubCommand (new SwgCuiCommandParserParticle ());
	m_parser->addSubCommand (new SwgCuiCommandParserAudio ());
	m_parser->addSubCommand (new SwgCuiCommandParserCustomization ());
	m_parser->addSubCommand (new SwgCuiCommandParserMount ());
	m_parser->addSubCommand (new SwgCuiCommandParserVideo ());
}

//-----------------------------------------------------------------

DefaultParserStrategy::~DefaultParserStrategy ()
{
	delete m_parser;
	m_parser = 0;
	delete m_commandTableParser;
	m_commandTableParser = 0;
}

//-----------------------------------------------------------------

bool DefaultParserStrategy::parse (const Unicode::String & str, Unicode::String & result) const
{
	CommandParser::ErrorType ret;

	ret = m_parser->parse (NetworkId::cms_invalid, str, result);
	if (ret == CommandParser::ERR_CMD_NOT_FOUND)
	{
		result.clear();
		ret = m_commandTableParser->parse (NetworkId::cms_invalid, str, result);
	}
	if (ret == CommandParser::ERR_NO_HANDLER)
	{
		// TODO: make this message lookup from string table
		result += Unicode::narrowToWide (" :: Console Parser Failed!\n");
		return false;
	}
	return true;
}

//-----------------------------------------------------------------

bool DefaultParserStrategy::tabCompleteToken (const Unicode::String & str, size_t tokenPos, std::vector<Unicode::String> & results, size_t & token_start, size_t & token_end) const
{
	IGNORE_RETURN (m_parser->tabCompleteToken (str, tokenPos, results, token_start, token_end));
	IGNORE_RETURN (m_commandTableParser->tabCompleteToken (str, tokenPos, results, token_start, token_end));
	return true;
}

//-----------------------------------------------------------------

SwgCuiConsole::SwgCuiConsole (UIPage & page) :
CuiMediator      ("SwgCuiConsole", page),
MessageDispatch::Receiver (),
m_outputText     (NON_NULL (GET_UI_OBJ (getPage (), UIText,    "OutputText"))),
m_inputTextbox   (NON_NULL (GET_UI_OBJ (getPage (), UITextbox, "InputTextbox"))),
m_history        (new CommandParserHistory),
m_parserStrategy (new DefaultParserStrategy (m_history)),
m_consoleHelper  (new CuiConsoleHelper (getPage (), *m_outputText, *m_inputTextbox, m_parserStrategy, m_history)),
m_callback       (new MessageDispatch::Callback),
m_prevConsoleHelper(NULL)
{
	m_consoleHelper->setAlwaysProcessEscape (true);
	m_consoleHelper->setMaxOutputTextLines (6000);

	UINarrowString str ("\\#ffffff");

	m_outputText->AppendLocalText ( UIString (str.begin (), str.end ()));
	connectToMessage (ConGenericMessage::MessageType);
	connectToMessage (ExecuteConsoleCommand::MessageType);
	connectToMessage ("GalaxyLoopTimesResponse");

	m_callback->connect (*this, &SwgCuiConsole::onDebugPrintUi,           static_cast<Game::Messages::DebugPrintUi*>   (0));
	m_callback->connect (*this, &SwgCuiConsole::onChatRoomStatusMessage , static_cast<CuiChatRoomManager::Messages::StatusMessage *>   (0));

}

//-----------------------------------------------------------------

SwgCuiConsole::~SwgCuiConsole ()
{
	m_callback->disconnect (*this, &SwgCuiConsole::onDebugPrintUi,       static_cast<Game::Messages::DebugPrintUi*>   (0));
	m_callback->disconnect (*this, &SwgCuiConsole::onChatRoomStatusMessage , static_cast<CuiChatRoomManager::Messages::StatusMessage *>   (0));

	delete m_callback;
	m_callback = 0;

	m_inputTextbox = 0;
	m_outputText   = 0;

	delete m_consoleHelper;
	m_consoleHelper = 0;

	delete m_parserStrategy;
	m_parserStrategy = 0;

	delete m_history;
	m_history = 0;
}

//-----------------------------------------------------------------

void SwgCuiConsole::performActivate ()
{
	setPointerInputActive  (true);
	setKeyboardInputActive (true);
	setInputToggleActive   (false);

	m_inputTextbox->SetFocus ();

	m_prevConsoleHelper = CuiConsoleHelper::getActiveConsoleHelper();
	CuiConsoleHelper::setActiveConsoleHelper(m_consoleHelper);
}

//-----------------------------------------------------------------

void SwgCuiConsole::performDeactivate ()
{
	UIPage * const parent = dynamic_cast<UIPage *>(getPage ().GetParentWidget ());
	if (parent)
	{
		parent->MoveKeyboardFocus (true);
	}

	CurrentUserOptionManager::save ();
	LocalMachineOptionManager::save ();

	if(m_prevConsoleHelper)
	{
		CuiConsoleHelper::setActiveConsoleHelper(m_prevConsoleHelper);

		m_prevConsoleHelper = NULL;
	}
}

//-----------------------------------------------------------------

void SwgCuiConsole::processChatMessage (const Unicode::String & str)
{
	m_consoleHelper->appendOutputText (str);
}

//-----------------------------------------------------------------

void SwgCuiConsole::onDebugPrintUi     (const Unicode::String & str)
{
	m_consoleHelper->appendOutputText (str);
}

//----------------------------------------------------------------------

void SwgCuiConsole::onChatRoomStatusMessage (const CuiChatRoomManagerStatusMessage & msg)
{
	m_consoleHelper->appendOutputText (msg.str);
}

//----------------------------------------------------------------------

void SwgCuiConsole::receiveMessage(const MessageDispatch::Emitter &, const MessageDispatch::MessageBase & message)
{
	if (message.isType(ConGenericMessage::MessageType))
	{
		Archive::ReadIterator ri = static_cast<const GameNetworkMessage &>(message).getByteStream().begin();
		const ConGenericMessage cg(ri);
		m_consoleHelper->appendOutputText (Unicode::narrowToWide(cg.getMsg()) );
	}
}

//-----------------------------------------------------------------------
