//======================================================================
//
// CuiMessageQueueManager.cpp
// copyright (c) 2002 Sony Online Entertainment
//
//======================================================================

#include "clientUserInterface/FirstClientUserInterface.h"
#include "clientUserInterface/CuiMessageQueueManager.h"

#include "UIActionListener.h"
#include "UIManager.h"
#include "UIBaseObject.h"
#include "UIWidget.h"
#include "clientGame/Game.h"
#include "clientUserInterface/CuiDragInfo.h"
#include "clientUserInterface/CuiInputNames.h"
#include "sharedFoundation/MessageQueue.h"
#include "sharedFoundation/MessageQueueDataTemplate.h"
#include "sharedInputMap/InputMap_Command.h"
#include "sharedMessageDispatch/Transceiver.h"

//@todo remove this and have the SwgCuiManager inform the CuiMessageQueueManager what its command parser message is to be
#include "sharedFoundation/GameControllerMessage.h"

//----------------------------------------------------------------------

namespace
{
	namespace Transceivers
	{
		MessageDispatch::Transceiver<CuiMessageQueueManager::Messages::CommandParserRequest::Payload &, CuiMessageQueueManager::Messages::CommandParserRequest > 
			commandParserRequest;
	}

	class ActionListener : public UIActionListener
	{
	public:
		~ActionListener () {}
		bool performActionForObject    (const UIBaseObject & obj)
		{
			return CuiMessageQueueManager::executeCommandForObject (obj);
		}

		bool constructTooltipForObject (const UIBaseObject & obj, Unicode::String & str)
		{
			return CuiMessageQueueManager::composeTooltipForObject (obj, str);
		}
	};

	ActionListener actionListener;
}

//----------------------------------------------------------------------

void CuiMessageQueueManager::install                 ()
{
	UIManager::gUIManager ().SetActionListener (&actionListener);
}

//----------------------------------------------------------------------

void CuiMessageQueueManager::remove                  ()
{
	UIManager::gUIManager ().SetActionListener (0);
}

//----------------------------------------------------------------------

void CuiMessageQueueManager::scanMessageQueue ()
{
	MessageQueue * const queue = Game::getGameMessageQueue ();
	if (queue)
		CuiMessageQueueManager::scanMessageQueue (*queue);
}

//----------------------------------------------------------------------

bool CuiMessageQueueManager::executeCommandByName   (const std::string & str)
{
	InputMap * const imap = Game::getGameInputMap ();
	if (imap)
	{
		if (imap->executeCommandByName (str.c_str (), 0, 0, 0))
			return true;
		
		WARNING (true, ("CuiMessageQueueManager failed to execute command by name: '%s'", str.c_str ()));
	}

	return false;
}

//----------------------------------------------------------------------

bool CuiMessageQueueManager::executeCommandByString (const std::string & str, bool transformCompositeCommand)
{
	static std::string transformed;
	transformed = str;

	if (transformCompositeCommand)
	{
		size_t pos = 0;
		for (;;)
		{
			pos = transformed.find ('+', 0);
			if (pos != std::string::npos)
				transformed [pos] = ' ';
			else
				break;
		}
	}

	Messages::CommandParserRequest::Payload payload (transformed, false);
	Transceivers::commandParserRequest.emitMessage (payload);
	return true;
}

//----------------------------------------------------------------------

bool CuiMessageQueueManager::executeCommandForObject (const UIBaseObject & obj)
{
	std::string str;

	if (obj.GetPropertyNarrow (CuiDragInfo::Properties::CommandName, str))
		return executeCommandByName (str);
	if (obj.GetPropertyNarrow (CuiDragInfo::Properties::CommandString, str))
		return executeCommandByString (str, true);

	return false;
}

//----------------------------------------------------------------------

bool CuiMessageQueueManager::composeTooltipForObject (const UIBaseObject & obj, Unicode::String & str)
{
	Unicode::String cmdStr;
	if (obj.HasProperty (UIWidget::PropertyName::DragType))
	{
		CuiDragInfo cdi (obj);
		if (cdi.type == CuiDragInfoTypes::CDIT_command || cdi.type == CuiDragInfoTypes::CDIT_macro)
		{
			if (!cdi.cmd.empty ())
			{
				if (cdi.type == CuiDragInfoTypes::CDIT_command)
				{
					str += CuiInputNames::getLocalizedCommandName (cdi.cmd);
				}
				else if (cdi.type == CuiDragInfoTypes::CDIT_macro)
				{
					static const Unicode::String macroPrefix = Unicode::narrowToWide ("Macro: ");
					str += macroPrefix + cdi.name;
				}
				
				InputMap * const imap = Game::getGameInputMap ();
				if (imap)
				{
					Unicode::String bindStr;
					if (!CuiInputNames::getInputValueString (*imap, cdi.cmd, bindStr))
						WARNING (true, ("No such input map command: '%s'", cdi.cmd.c_str ()));
					else
					{
						if (!bindStr.empty ())
						{
							str.append (1, ' ');
							str.append (1, '(');
							str.append (bindStr);
							str.append (1, ')');
						}
					}
				}
				
				return true;
			}
			
			else if (!cdi.str.empty ())
			{
				static std::string transformed;
				transformed = cdi.str;
				
				size_t pos = 0;
				for (;;)
				{
					pos = transformed.find ('+', 0);
					if (pos != std::string::npos)
						transformed [pos] = ' ';
					else
						break;
				}
				
				str.append (Unicode::narrowToWide (transformed));
				return true;
			}
		}
	}
	
	return false;
}

//----------------------------------------------------------------------

bool CuiMessageQueueManager::findCommandString (const std::string & cmd, std::string & str, bool warn)
{
	const InputMap * const imap = Game::getGameInputMap ();
	if (imap)
	{
		const InputMap::Command * const imapCommand = imap->findCommandByName     (cmd.c_str (), true);
		if (imapCommand)
		{
			if (!imapCommand->pressEvent.str.empty ())
			{
				str = imapCommand->pressEvent.str;
				return true;
			}
			else 
				WARNING (warn, ("SwgCuiCommandBrowser imap command [%s] has no string", cmd.c_str ()));				
		}
	}
	
	return false;
}

//----------------------------------------------------------------------

/**
* Iterate through the messages in a messagequeue, attempting to perform an action for each.
*/

void CuiMessageQueueManager::scanMessageQueue (MessageQueue & queue)
{
	for (int i = 0; i < queue.getNumberOfMessages (); i++)
	{
		int   message;
		float value;	
		MessageQueue::Data * data = 0;
		queue.getMessage (i, &message, &value, &data);

		typedef MessageQueueDataTemplate<std::string> MessageQueueDataString;

		static const int clientCommandParser = CM_clientCommandParser;

		if (message == clientCommandParser)
		{
			if (!data)
				WARNING (true, ("Bad string"));
			else
			{
				const MessageQueueDataString * const mqds = safe_cast<MessageQueueDataString *>(data);
				NOT_NULL (mqds);

				const std::string & str = mqds->getData ();
				if (!executeCommandByString (str, true))
					WARNING (true, ("CM_clientCommandParser failed for command '%s'", str.c_str ()));

				if (data)
				{
					queue.clearMessageData (i);
					queue.clearMessage     (i);
						
					delete data;
					data = 0;
				}
			}
		}
	}
}

//======================================================================
