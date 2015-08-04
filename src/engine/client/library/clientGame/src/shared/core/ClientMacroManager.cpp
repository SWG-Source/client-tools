//======================================================================
//
// ClientMacroManager.cpp
// copyright (c) 2003 Sony Online Entertainment
//
//======================================================================

#include "clientGame/FirstClientGame.h"
#include "clientGame/ClientMacroManager.h"

#include "UnicodeUtils.h"
#include "clientGame/ConfigClientGame.h"
#include "clientGame/Game.h"
#include "clientUserInterface/CuiMessageBox.h"
#include "clientUserInterface/CuiMessageQueueManager.h"
#include "fileInterface/StdioFile.h"
#include "sharedFile/Iff.h"
#include "sharedFile/TreeFile.h"
#include "sharedFoundation/GameControllerMessage.h"
#include "sharedFoundation/NetworkId.h"
#include "sharedFoundation/Os.h"
#include "sharedInputMap/InputMap.h"
#include "sharedInputMap/InputMap_Command.h"
#include "sharedMessageDispatch/Transceiver.h"
#include <algorithm>
#include <vector>

//======================================================================

//======================================================================

namespace ClientMacroManagerNamespace
{
	typedef ClientMacroManager::MacroDataVector MacroDataVector;

	namespace Transceivers
	{
		MessageDispatch::Transceiver<const ClientMacroManager::Messages::Changed::Payload &,   ClientMacroManager::Messages::Changed > 
			changed;
		MessageDispatch::Transceiver<const ClientMacroManager::Messages::Reset::Payload &,     ClientMacroManager::Messages::Reset > 
			reset;
		MessageDispatch::Transceiver<const ClientMacroManager::Messages::Added::Payload &,     ClientMacroManager::Messages::Added > 
			added;
		MessageDispatch::Transceiver<const ClientMacroManager::Messages::Removed::Payload &,   ClientMacroManager::Messages::Removed > 
			removed;
		MessageDispatch::Transceiver<const ClientMacroManager::Messages::Modified::Payload &,  ClientMacroManager::Messages::Modified > 
			modified;
	}
	
	MacroDataVector s_macros;
	int             s_lastMacroNumber = 0;

	const std::string s_macroTextVersion       = "0000";
	const int         s_macroCommandLineLength = 1024;
	
	std::string getTextPath ()
	{
		std::string     loginId;
		std::string     cluster;
		Unicode::String playerName;
		NetworkId       id;
		
		if (!Game::getPlayerPath (loginId, cluster, playerName, id))
			return std::string ();
		
		return std::string ("profiles/") + loginId + "/macros.txt";
	}

	std::string getIffPath ()
	{
		std::string     loginId;
		std::string     cluster;
		Unicode::String playerName;
		NetworkId       id;
		
		if (!Game::getPlayerPath (loginId, cluster, playerName, id))
			return std::string ();
		
		return std::string ("profiles/") + loginId + "/macros.iff";
	}

	Tag tagMacro = TAG(M,C,R,O);
	Tag tagEntry = TAG(E,N,T,R);

	const unsigned int cs_maxMacroSize = 1024;
}

using namespace ClientMacroManagerNamespace;

std::string ClientMacroManager::ms_forcePauseCommand = "/pause 0.25;";
std::vector<std::string> ClientMacroManager::ms_disallowedMacroCommands;

//----------------------------------------------------------------------

const ClientMacroManager::MacroDataVector & ClientMacroManager::getMacroDataVector ()
{
	return s_macros;
}

//----------------------------------------------------------------------

ClientMacroManager::Data * ClientMacroManager::findMacroDataInternal  (const std::string & name)
{
	for (MacroDataVector::iterator it = s_macros.begin (); it != s_macros.end (); ++it)
	{
		Data & data = *it;
		if (data.name == name)
			return &data;
	}

	return 0;
}

//----------------------------------------------------------------------

ClientMacroManager::Data * ClientMacroManager::findMacroDataByUserDefinedNameInternal  (const Unicode::UTF8String & userDefinedName)
{
	for (MacroDataVector::iterator it = s_macros.begin (); it != s_macros.end (); ++it)
	{
		Data & data = *it;
		if (!_stricmp (data.userDefinedName.c_str (), userDefinedName.c_str ()))
			return &data;
	}

	return 0;
}

//----------------------------------------------------------------------

bool ClientMacroManager::executeMacroByUserDefinedName      (const Unicode::UTF8String & userDefinedName, bool insertPause)
{
	const Data * const data =  ClientMacroManager::findMacroDataByUserDefinedNameInternal  (userDefinedName);
	if (data)
	{
		if (insertPause)
		{
			static const std::string prependPauseString = "/pause 0.1;";
			return CuiMessageQueueManager::executeCommandByString (prependPauseString + data->commandString, true);
		}
		else
			return CuiMessageQueueManager::executeCommandByString (data->commandString, true);
	}

	return false;
}

//----------------------------------------------------------------------

const ClientMacroManager::Data * ClientMacroManager::findMacroData      (const std::string & name)
{
	return findMacroDataInternal (name);
}


//----------------------------------------------------------------------

ClientMacroManager::Data * ClientMacroManager::createNewMacroDataInternal (int number)
{
	if (!number)
		number = ++s_lastMacroNumber;

	char buf [64];
	IGNORE_RETURN(snprintf (buf, sizeof (buf), "custom_macro_%04d", number));

	Data data;
	data.name   = buf;
	data.number = number;
	data.color  = "#ffffff";
	s_macros.push_back (data);

	return &s_macros.back ();
}

//----------------------------------------------------------------------

const ClientMacroManager::Data * ClientMacroManager::createNewMacroData (const Unicode::UTF8String & userDefinedName, const Unicode::UTF8String & commandString, const std::string & icon, bool notify)
{
	Data * const data = createNewMacroDataInternal (0);
	NOT_NULL (data);
	modifyMacroData (data->name, userDefinedName, commandString, icon, false);

	if (notify)
	{
		Transceivers::added.emitMessage    (data->name);
		Transceivers::changed.emitMessage  (true);
	}

	return data;
}

//----------------------------------------------------------------------

const bool ClientMacroManager::modifyMacroData    (const std::string & name, const Unicode::UTF8String & userDefinedName, const Unicode::UTF8String & commandString, const std::string & icon, bool notify, bool pushToInputMap)
{
	Data * const data = findMacroDataInternal (name);
	if (!data)
	{
		WARNING (true, ("Attempt to modify macro data [%s] which does not exist", name.c_str ()));
		return false;
	}

	//prepend a forced pause if there isn't already one
	Unicode::UTF8String command = commandString;
	std::string::size_type pos = command.find(ms_forcePauseCommand);
	if(pos != 0)
	{
		command = ms_forcePauseCommand + command;
	}

	data->userDefinedName = userDefinedName;
	data->commandString   = command;
	data->icon            = icon;
	
	if (pushToInputMap)
	{
		InputMap * const inputmap = Game::getGameInputMap ();
		if (inputmap)
		{
			IGNORE_RETURN(inputmap->addCustomCommand (data->name, static_cast<int>(CM_clientCommandParser), data->commandString, true));
		}
	}

	if (notify)
	{
		Transceivers::modified.emitMessage (name);
		Transceivers::changed.emitMessage  (true);
	}

	return true;
}


//----------------------------------------------------------------------

bool ClientMacroManager::eraseMacro         (const std::string & name, bool notify)
{
	for (MacroDataVector::iterator it = s_macros.begin (); it != s_macros.end (); ++it)
	{
		Data & data = *it;
		if (data.name == name)
		{
			IGNORE_RETURN(s_macros.erase (it));
			InputMap * const inputmap = Game::getGameInputMap ();
			if (inputmap)
			{
				IGNORE_RETURN(inputmap->removeCustomCommand (name));
			}

			if (notify)
			{
				Transceivers::removed.emitMessage (name);
				Transceivers::changed.emitMessage (true);
			}

			return true;
		}
	}

	return false;
}

//----------------------------------------------------------------------

bool ClientMacroManager::saveText        ()
{
	const std::string & filename = getTextPath ();
	if (filename.empty ())
	{
		WARNING (true, ("ClientMacroManager Unable to get player path for saving"));
		return false;
	}
	
	{
		static const char path_seps [] = { '\\', '/', 0 };
		
		const size_t slashpos = filename.find_last_of (path_seps);
		
		if (slashpos != static_cast<size_t>(std::string::npos))
			IGNORE_RETURN (Os::createDirectories (filename.substr (0, slashpos).c_str ()));
	}
	
	StdioFile fl (filename.c_str(), "wb");

	if (!fl.isOpen ())
	{
		WARNING (true, ("ClientMacroManager::save Could not open [%s] for writing", filename.c_str ()));
		return false;
	}

	//save out the macro version
	char buf [2048];
	snprintf (buf, sizeof (buf), "version: %s\n", s_macroTextVersion.c_str());
	IGNORE_RETURN(fl.write (static_cast<int>(strlen (buf)), buf));

	//save out the macros
	for (MacroDataVector::const_iterator it = s_macros.begin (); it != s_macros.end (); ++it)
	{
		const Data & data = *it;
		
		//pack multi-line macros
		Unicode::UTF8String command = data.commandString;
		//cut off macro at max length
		if(command.size() > s_macroCommandLineLength)
			command = command.substr(0, s_macroCommandLineLength);

		size_t pos = command.find("\n");
		while (pos != command.npos) //lint !e737 STL lameness with npos being wrong signed-ness
		{
			//if there's a newline w/o a semicolon, replace the newline with a semicolon
			const char & prev = command.at(pos-1);
			if (prev != ';')
				command.replace(pos, 1, ";");
			//else strip out the unnecessary newlines
			else
				command.replace(pos, 1, "");
			pos = command.find("\n");
		}

		snprintf (buf, sizeof (buf), "%d %s %s %s %s\n", 
			data.number, 
			data.userDefinedName.empty () ? "." : data.userDefinedName.c_str (),
			data.icon.empty () ?            "." : data.icon.c_str (),
			data.color.empty () ?           "." : data.color.c_str (),
			command.empty () ?              "." : command.c_str ());

		IGNORE_RETURN(fl.write (static_cast<int>(strlen (buf)), buf));
	}

	return true;
}

//----------------------------------------------------------------------

bool ClientMacroManager::loadText        ()
{
	const std::string & filename = getTextPath ();
	if (filename.empty ())
	{
		WARNING (true, ("ClientMacroManager Unable to get player path for loading."));
		return false;
	}
	
	StdioFile fl (filename.c_str(), "rb");

	if (!fl.isOpen ())
	{
		return false;
	}
	
	char * cbuf = reinterpret_cast<char *>(fl.readEntireFileAndClose ());
	fl.close ();

	if (!cbuf)
	{
		WARNING (true, ("Read nothing from macro file"));
		return false;
	}

	const std::string sbuf (cbuf);
	delete [] cbuf;
	cbuf = 0;
	
	size_t endpos = 0;
	std::string lineToken;
	int linenum = 0;

	size_t mid = 0;
	std::string tokens [4];

	//get the special case  first line (should have version number)
	++linenum;
	std::string tokens1 [2];
	IGNORE_RETURN(Unicode::getFirstToken (sbuf, endpos, endpos, lineToken, "\r\n"));
	for (int i = 0; i < 2; ++i)
	{
		IGNORE_RETURN(Unicode::getFirstToken (lineToken, mid, mid, tokens1 [i]));
		++mid;
	}

	//check version, read in old version / update file if necessary
	if(tokens1[1] != s_macroTextVersion)
	{
		WARNING (true, ("Bad line in macro file [%s], line %d, token %d: [%s].\nline: [%s] Bad macro file version?", filename.c_str (), linenum, 1, tokens [1].c_str (), lineToken.c_str ()));
		return false;
	}

	//move on to reading the actual macros
	int localLastMacroNumber = 0;
	while (Unicode::getFirstToken (sbuf, endpos, endpos, lineToken, "\r\n") && endpos != Unicode::String::npos) //lint !e650 !e737 STL lameness with npos being wrong signed-ness
	{
		++linenum;
		mid = 0;
		bool success = true;
		for (int j = 0; j < 4; ++j)
		{
			if (!Unicode::getFirstToken (lineToken, mid, mid, tokens [j]) || mid == Unicode::String::npos) //lint !e650 !e737 STL lameness with npos being wrong signed-ness
			{
				WARNING (true, ("Bad line in macro file [%s], line %d, token %d: [%s].\nline: [%s]", filename.c_str (), linenum, j, tokens [j].c_str (), lineToken.c_str ()));
				success = false;
				break;
			}
			++mid;
		}
		
		//only parse data if we could read it successfully
		if(success)
		{
			int number = atoi (tokens [0].c_str ());
			localLastMacroNumber  = std::max (number, localLastMacroNumber );
			char buf [64];
			IGNORE_RETURN(snprintf (buf, sizeof (buf), "custom_macro_%04d", number));

			const std::string & name = buf;
			const Unicode::UTF8String & userDefinedName = tokens [1];
			const std::string & icon            = tokens [2];
			const std::string & color           = tokens [3];
			Unicode::UTF8String commandString           = lineToken.substr (mid);

			//cut off macro at max length
			if(commandString.size() > s_macroCommandLineLength)
				commandString = commandString.substr(0, s_macroCommandLineLength);

			//unpack command into multiple lines, based off semicolons
			size_t pos = commandString.find(";");
			while(pos != commandString.npos) //lint !e737 STL lameness with npos being wrong signed-ness
			{
				IGNORE_RETURN(commandString.replace(pos, 1, ";\n"));
				pos = commandString.find(";", pos+1);
			}

			Data * const data     = createNewMacroDataInternal (number);
			if (!data)
			{
				WARNING (true, ("Unable to set macro %d during loading: [%s] [%s]", number, name.c_str (), userDefinedName.c_str ()));
			}
			else
			{
				modifyMacroData (name, userDefinedName, commandString, icon, false, false);
				data->color = color;
			}
		}
		++endpos;
	}

	s_lastMacroNumber = localLastMacroNumber;

	Transceivers::reset.emitMessage   (true);

	return true;
}

//----------------------------------------------------------------------

bool ClientMacroManager::loadIff()
{
	const std::string & filename = getIffPath ();
	if (filename.empty ())
	{
		WARNING (true, ("ClientMacroManager Unable to get player path for saving"));
		return false;
	}

	if (!TreeFile::exists(filename.c_str()))
	{
		DEBUG_REPORT_LOG(true, ("Macro file %s does not exist\n", filename.c_str()));
		return false;
	}
	
	if (!Iff::isValid(filename.c_str()))
	{
		WARNING(true, ("Macro file %s is not valid\n", filename.c_str()));
		return false;
	}

	Iff iff;
	bool result = iff.open(filename.c_str(), true);
	if(!result)
		return false;

	if (iff.getCurrentName () == tagMacro)
		iff.enterForm ();

	result = false;
	switch (iff.getCurrentName())
	{
		case TAG_0000:
			result = loadIff_0000(iff);
			break;
		default:
			break;
	}
	iff.exitForm();
	return result;
}

//----------------------------------------------------------------------

bool ClientMacroManager::loadIff_0000(Iff& iff)
{
	iff.enterForm (TAG_0000);

	int localLastMacroNumber = 0;

	int macroNumber;
	Unicode::UTF8String  macroUserName;
	std::string macroName;
	std::string macroIcon;
	std::string macroColor;
	Unicode::UTF8String  macroCommandText;
	while(iff.enterChunk(tagEntry, true))
	{
		macroNumber      = iff.read_int32();
		macroUserName    = iff.read_stdstring();
		macroIcon        = iff.read_stdstring();
		macroColor       = iff.read_stdstring();
		macroCommandText = iff.read_stdstring();

		localLastMacroNumber  = std::max (macroNumber, localLastMacroNumber);

		char buf [64];
		snprintf (buf, sizeof (buf), "custom_macro_%04d", macroNumber);
		macroName = buf;

		//check macros for disallowed commands
		for(std::vector<std::string>::iterator i = ms_disallowedMacroCommands.begin(); i != ms_disallowedMacroCommands.end(); ++i)
		{
			const std::string & disallowedCommand       = *i;
			const std::string & lowertext               = Unicode::toLower (macroCommandText);
			const std::string & lowerDisallowedcommand  = Unicode::toLower (disallowedCommand);

			const std::string::size_type pos = lowertext.find (lowerDisallowedcommand);

			if (pos != lowertext.npos)     //lint !e737 loss of sign in promotion from broken npos type
			{
				macroCommandText.erase(pos, pos + disallowedCommand.size());
			}
		}

		//quietly ignore over-long macros
		if(macroCommandText.size() <= cs_maxMacroSize)
		{
			Data * const data = createNewMacroDataInternal (macroNumber);
			if (!data)
			{
				WARNING (true, ("Unable to set macro %d during loading: [%s] [%s]", macroNumber, macroName.c_str(), macroUserName.c_str()));
			}
			else
			{
				modifyMacroData (macroName, macroUserName, macroCommandText, macroIcon, false, false);
				data->color = macroColor;
			}
		}
		iff.exitChunk(tagEntry);
	}

	s_lastMacroNumber = localLastMacroNumber;

	iff.exitForm (TAG_0000);
	return true;
}

//----------------------------------------------------------------------

bool ClientMacroManager::saveIff()
{
	const std::string & filename = getIffPath ();
	if (filename.empty ())
	{
		WARNING (true, ("ClientMacroManager Unable to get player path for saving"));
		return false;
	}

	Iff iff(1024);
	iff.insertForm(tagMacro);
	{
		iff.insertForm (TAG_0000);
		{
			for (MacroDataVector::const_iterator it = s_macros.begin (); it != s_macros.end (); ++it)
			{
				iff.insertChunk(tagEntry, true);
				{
					const Data & data = *it;
					iff.insertChunkData(data.number);
					iff.insertChunkString(data.userDefinedName.c_str());
					iff.insertChunkString(data.icon.c_str());
					iff.insertChunkString(data.color.c_str());
					iff.insertChunkString(data.commandString.c_str());
				}
				iff.exitChunk(tagEntry);		
			}
		}
		iff.exitForm(TAG_0000);
	}
	iff.exitForm(tagMacro);
	IGNORE_RETURN(iff.write(filename.c_str()));
	return true;
}

//----------------------------------------------------------------------

bool ClientMacroManager::load()
{
	ms_disallowedMacroCommands.clear();

	s_macros.clear();
	bool result = false;
	result = loadText();
	if(!result)
	{
		result = loadIff();
	}
	return result;
}

//----------------------------------------------------------------------

bool ClientMacroManager::save()
{
	return saveText();
}

//----------------------------------------------------------------------

void ClientMacroManager::synchronizeWithInputMap   (class InputMap * inputmap)
{	
	if (!inputmap)
		inputmap = Game::getGameInputMap ();

	if (inputmap)
	{	
		//-- first remove obsolete commands.
		//-- this can happen for a variety of reasons, including
		//-- deletion or modification of the macros file

		InputMap::CommandVector cv;
		inputmap->getCommandsByCategory ("custom", cv);
		
		typedef stdvector<std::string>::fwd StringVector;
		StringVector sv;
		
		{
			for (InputMap::CommandVector::const_iterator it = cv.begin (); it != cv.end (); ++it)
			{
				const InputMap::Command * const cmd = *it;
				if (!findMacroData (cmd->name))
					sv.push_back (cmd->name);
			}
		}
		
		{
			for (StringVector::const_iterator it = sv.begin (); it != sv.end (); ++it)
			{
				IGNORE_RETURN(inputmap->removeCustomCommand (*it));
			}
		}
		
		{
			for (MacroDataVector::const_iterator it = s_macros.begin (); it != s_macros.end (); ++it)
			{
				const Data & data = *it;
				IGNORE_RETURN(inputmap->addCustomCommand (data.name, static_cast<int>(CM_clientCommandParser), data.commandString, true));		
			}
		}
	}
}

//----------------------------------------------------------------------

const std::string& ClientMacroManager::getForcePauseCommand()
{
	return ms_forcePauseCommand;
}

//----------------------------------------------------------------------

const std::vector<std::string> & ClientMacroManager::getDisallowedCommands()
{
	return ms_disallowedMacroCommands;
}

//======================================================================

