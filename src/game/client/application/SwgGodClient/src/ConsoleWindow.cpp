// ======================================================================
//
// ConsoleWindow.cpp
// copyright(c) 2001 Sony Online Entertainment
//
// ======================================================================

#include "SwgGodClient/FirstSwgGodClient.h"
#include "ConsoleWindow.h"
#include "ConsoleWindow.moc"

#include "sharedNetworkMessages/ConsoleChannelMessages.h"

#include "clientGame/GameNetwork.h"

#include "GodClientPerforce.h"
#include "ServerCommander.h"

#include <qtextview.h>

// ======================================================================

ConsoleWindow::ConsoleWindow(QWidget*parent, const char*name)
: BaseConsoleWindow(parent, name),
  MessageDispatch::Receiver()
{
	connectToMessage(GodClientPerforce::Messages::COMMAND_MESSAGE);
	connectToMessage("ConGenericMessage");
	IGNORE_RETURN(connect(&ServerCommander::getInstance(), SIGNAL(consoleCommandIssued(const char*)), this, SLOT(onConsoleCommandIssued(const char*))));
}

//-----------------------------------------------------------------

ConsoleWindow::~ConsoleWindow()
{
}

//-----------------------------------------------------------------

void ConsoleWindow::receiveMessage(const MessageDispatch::Emitter & source, const MessageDispatch::MessageBase & message)
{
	UNREF(source);
	if(message.isType("ConGenericMessage"))
	{
		Archive::ReadIterator ri = dynamic_cast<const GameNetworkMessage &>(message).getByteStream().begin();
		ConGenericMessage cg(ri);
		m_textView->append(cg.getMsg().c_str());		
	}
	else if(message.isType(GodClientPerforce::Messages::COMMAND_MESSAGE))
	{
		const GodClientPerforce::CommandMessage* const cm = NON_NULL(dynamic_cast<const GodClientPerforce::CommandMessage*>(&message));
		const std::string txt = std::string("P4: ") + cm->getMessage();
		m_textView->append(txt.c_str());
	}
}

//-----------------------------------------------------------------

void ConsoleWindow::onConsoleCommandIssued(const char* cmd)
{
	m_textView->append(cmd);
}


// ======================================================================
