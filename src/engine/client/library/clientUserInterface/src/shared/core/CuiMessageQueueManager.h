//======================================================================
//
// CuiMessageQueueManager.h
// copyright (c) 2002 Sony Online Entertainment
//
//======================================================================

#ifndef INCLUDED_CuiMessageQueueManager_H
#define INCLUDED_CuiMessageQueueManager_H

//======================================================================

class InputMap;
class MessageQueue;
class UIBaseObject;

//----------------------------------------------------------------------

class CuiMessageQueueManager
{
public:
	struct Messages
	{
		struct CommandParserRequest
		{
			typedef std::pair<const std::string, bool> Payload;
		};
	};

	static void              install                 ();
	static void              remove                  ();

	static void              scanMessageQueue        ();
	static void              scanMessageQueue        (MessageQueue & messageQueue);
	static bool              executeCommandByName    (const std::string & str);
	static bool              executeCommandByString  (const std::string & str, bool transformCompositeCommand);

	static bool              executeCommandForObject (const UIBaseObject & obj);
	static bool              composeTooltipForObject (const UIBaseObject & obj, Unicode::String & str);

	static bool              findCommandString (const std::string & cmd, std::string & str, bool warn);
};

//======================================================================

#endif
