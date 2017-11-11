#ifndef SwgCuiconsole_H
#define SwgCuiconsole_H

#include "clientUserInterface/CuiMediator.h"
#include "sharedMessageDispatch/Receiver.h"

//----------------------------------------------------------------------

class CommandParser;
class CommandParserHistory;
class CuiConsoleHelper;
class CuiConsoleHelperParserStrategy;
class Scene;
class UIPage;
class UIText;
class UITextbox;
class CuiChatRoomManagerStatusMessage;

namespace MessageDispatch
{
	class Callback;
}

//----------------------------------------------------------------------

class SwgCuiConsole : 
public CuiMediator, 
public MessageDispatch::Receiver
{
public:
	explicit                       SwgCuiConsole      (UIPage & page);

	void                           processChatMessage (const Unicode::String & str);

	virtual void                   receiveMessage     (const MessageDispatch::Emitter & source, const MessageDispatch::MessageBase & message);

	void                           onDebugPrintUi     (const Unicode::String & str);

	void                           onChatRoomStatusMessage (const CuiChatRoomManagerStatusMessage & msg);

protected:
	virtual void                   performActivate   ();
	virtual void                   performDeactivate ();

private:
	virtual                       ~SwgCuiConsole ();
	                               SwgCuiConsole ();
	                               SwgCuiConsole (const SwgCuiConsole &);
	SwgCuiConsole &                operator=   (const SwgCuiConsole &);
	void                           scrollToBottom ();

private:

	UIText *                           m_outputText;
	UITextbox *                        m_inputTextbox;

	CommandParserHistory *             m_history;
	CuiConsoleHelperParserStrategy *   m_parserStrategy;
	CuiConsoleHelper *                 m_consoleHelper;
	CuiConsoleHelper *                 m_prevConsoleHelper;

	MessageDispatch::Callback *        m_callback;
};

//-----------------------------------------------------------------

#endif
