//======================================================================
//
// SwgCuiChatWindow_Tab.cpp
// copyright (c) 2002 Sony Online Entertainment
//
//======================================================================

#include "swgClientUserInterface/FirstSwgClientUserInterface.h"
#include "swgClientUserInterface/SwgCuiChatWindow_Tab.h"

#include "clientGame/ChatLogManager.h"
#include "clientGame/ConfigClientGame.h"
#include "UnicodeUtils.h"
#include "clientUserInterface/CuiChatHistory.h"
#include "clientUserInterface/CuiChatManager.h"
#include "clientUserInterface/CuiChatRoomManager.h"
#include "clientUserInterface/CuiSettings.h"
#include <cstdio>
#include <set>
#include <vector>

//#include "DejaLib.h"

//======================================================================

namespace SwgCuiChatWindow_Tab
{
	int ms_lastTabId = 0;
}

using namespace SwgCuiChatWindow_Tab;

//----------------------------------------------------------------------

SwgCuiChatWindow::Tab::Tab () :
m_defaultChannel   (),
m_channels         (new ChannelSet),
m_modifiedChannels (new ChannelSet),
m_text             (),
m_name             (),
m_userAssignedName (),
m_modified         (false),
m_tabId            (++ms_lastTabId),
m_charactersCut    (0)
{

}

//----------------------------------------------------------------------

SwgCuiChatWindow::Tab::Tab   (const SwgCuiChatWindow::Tab & rhs) :
m_defaultChannel   (rhs.m_defaultChannel),
m_channels         (new ChannelSet (*rhs.m_channels)),
m_modifiedChannels (new ChannelSet),
m_text             (rhs.m_text),
m_name             (rhs.m_name),
m_userAssignedName (rhs.m_userAssignedName),
m_modified         (false),
m_tabId            (++ms_lastTabId),
m_charactersCut    (0)
{
}

//----------------------------------------------------------------------

SwgCuiChatWindow::Tab::~Tab ()
{
	delete m_channels;
	m_channels = 0;
	delete m_modifiedChannels;
	m_modifiedChannels = 0;
}

//----------------------------------------------------------------------

SwgCuiChatWindow::Tab & SwgCuiChatWindow::Tab::operator= (const SwgCuiChatWindow::Tab & rhs)
{
	//DEJA_CONTEXT("SwgCuiChatWindow::Tab::operator");

	return copy (rhs, true);
}

//----------------------------------------------------------------------

SwgCuiChatWindow::Tab & SwgCuiChatWindow::Tab::copy              (const SwgCuiChatWindow::Tab & rhs, bool copyText)
{
	//DEJA_CONTEXT("SwgCuiChatWindow::Tab::copy");

	m_defaultChannel  = rhs.m_defaultChannel;
	*m_channels       = *rhs.m_channels;
	m_modifiedChannels->clear ();

	if (copyText)
		m_text            = rhs.m_text;

	m_name             = rhs.m_name;
	m_userAssignedName = rhs.m_userAssignedName;
	m_modified         = false;

	SendNotification (UINotification::ObjectChanged, 0);

	return *this;
}

//----------------------------------------------------------------------

bool SwgCuiChatWindow::Tab::hasChannel (const ChannelId & id) const
{
	//DEJA_CONTEXT("SwgCuiChatWindow::Tab::hasChannel");

	return m_channels->find (id) != m_channels->end ();
}

//----------------------------------------------------------------------

void SwgCuiChatWindow::Tab::parse             (const Unicode::String & str)
{
	//DEJA_CONTEXT("SwgCuiChatWindow::Tab::parse");

	switch (m_defaultChannel.type)
	{
	case CT_none:
	case CT_spatial:
	case CT_combat:
	case CT_chatRoom:
		break;
	}

	UNREF (str);
}

//----------------------------------------------------------------------

void SwgCuiChatWindow::Tab::addChannel        (const ChannelId & id)
{
	//DEJA_CONTEXT("SwgCuiChatWindow::Tab::addChannel");

	if (id.type == CT_none || hasChannel (id))
		return;

	const bool empty = m_channels->empty ();
	m_channels->insert (id);

	if (empty)
		setDefaultChannel (id);
}

//----------------------------------------------------------------------

void SwgCuiChatWindow::Tab::removeChannel     (const ChannelId & id)
{
	//DEJA_CONTEXT("SwgCuiChatWindow::Tab::removeChannel");

	m_channels->erase (id);

	if (id == m_defaultChannel)
	{
		if (!m_channels->empty ())
			setDefaultChannel (*m_channels->begin ());
		else
			setDefaultChannel (ChannelId ());
	}
}

//----------------------------------------------------------------------

void SwgCuiChatWindow::Tab::setDefaultChannel (const ChannelId & id)
{
	//DEJA_CONTEXT("SwgCuiChatWindow::Tab::setDefaultChannel");

	addChannel (id);
	m_defaultChannel = id;
	updateName (true);
}

//----------------------------------------------------------------------

void SwgCuiChatWindow::Tab::getChannels       (ChannelVector & cv) const
{
	//DEJA_CONTEXT("SwgCuiChatWindow::Tab::getChannels");

	cv.reserve (m_channels->size ());
	cv.assign (m_channels->begin (), m_channels->end ());
}

//----------------------------------------------------------------------

/**
* append the text, removing any unneccessary linebreaks
* @param result takes a copy because it must be modified internally
*/

void SwgCuiChatWindow::Tab::appendText (const ChannelId & id, const Unicode::String & str)
{	
	//DEJA_CONTEXT(SwgCuiChatWindow::Tab::appendText);

	if (id.type != CT_none && !hasChannel (id))
		return;

	if (str.empty ())
	{
		WARNING (true, ("Attempt to SwgCuiChatWindow::Tab::appendText empty string"));
		return;
	}
	Unicode::String fixupStr;
	
	if (CuiChatManager::getChatBoxTimestamp ())
	{
		const time_t t = time (0);
		const tm * localTime = localtime (&t);

		char buf [64];

		const int hour = localTime->tm_hour;
		const int min  = localTime->tm_min;
		const int sec  = localTime->tm_sec;

		_snprintf (buf, sizeof (buf), "%02d:%02d:%02d ", hour, min, sec);

		//-- rip off any leading \n's

		const size_t skipped_pos = Unicode::skipWhitespace (str, 0);

		if (skipped_pos != Unicode::String::npos)
			fixupStr = Unicode::narrowToWide (buf) + str.substr (skipped_pos);
	}
	else
		fixupStr = str;


	Unicode::String		strHold = fixupStr;

	getAppendableText (m_text, fixupStr);
	//DEJA_TRACE("SwgCuiChatWindow::Tab::appendText",L"getAppendableText 1 returns >> %s <<",fixupStr);

	ChatLogManager::appendLine(Unicode::narrowToWide("[") + getName() + Unicode::narrowToWide("] ") + fixupStr);
	
	getAppendableText(m_appendText,strHold);
	//DEJA_TRACE("SwgCuiChatWindow::Tab::appendText",L"getAppendableText 2 returns >> %s <<",strHold);

	m_appendText += strHold;
	m_text += fixupStr;

	//-----------------------------------------------------------------
	//-- clamp buffer len

	const int outputTextLines  = getLineCount ();
	
	if (outputTextLines > ConfigClientGame::getChatTabMaxTextLines())
	{
		m_charactersCut += removeLeadingLines (outputTextLines - ConfigClientGame::getChatTabMaxTextLines());
	}

	setModified (id, true);
}

//----------------------------------------------------------------------

/**
* Strips newlines from the end of @str, adds a newline to the beginning of @str
* if no newline exists at the end of @current.
*/
void SwgCuiChatWindow::Tab::getAppendableText (const Unicode::String & current, Unicode::String & str, bool appendMode) const
{
	//DEJA_CONTEXT("SwgCuiChatWindow::Tab::getAppendableText");

	if (str.empty ())
		return;

	if (str [str.length () - 1] == '\n')
		str.erase (str.length () - 1);
	
	if (!current.empty ())
	{
		if (current [current.length () - 1] != '\n' && str [0] != '\n')
			str.insert (Unicode::String::size_type(0), Unicode::String::size_type(1), '\n');
	}
	else if(current.empty() && appendMode)
	{
		// In append mode we want to add a leading newline
		str.insert (Unicode::String::size_type(0), Unicode::String::size_type(1), '\n');
	}
}

//----------------------------------------------------------------------

int SwgCuiChatWindow::Tab::getLineCount () const
{
	//DEJA_CONTEXT("SwgCuiChatWindow::Tab::getLineCount");

	int count = 0;

	for (Unicode::String::const_iterator it = m_text.begin (); it != m_text.end (); ++it)
	{
		if ((*it) == '\n')
			++count;
	}

	return count;
}

//----------------------------------------------------------------------

/**

*/
int SwgCuiChatWindow::Tab::removeLeadingLines (int num)
{
	//DEJA_CONTEXT("SwgCuiChatWindow::Tab::removeLeadingLines");

	int count = 0;
	size_t pos = 0;

	for ( ; pos != Unicode::String::npos && count < num; ++count)
	{
		pos = m_text.find ('\n', pos);

		if (pos == Unicode::String::npos)
			break;

		++pos;
	}

	int numCharsCut = 0;

	if (pos != Unicode::String::npos)
	{
		numCharsCut = pos;
		m_text.erase (0, pos);
	}
	else
	{
		numCharsCut = static_cast<int>(m_text.size ());
		m_text.clear ();
	}

	return numCharsCut;
}

//----------------------------------------------------------------------

void SwgCuiChatWindow::Tab::updateName(bool const notify)
{
	//DEJA_CONTEXT("SwgCuiChatWindow::Tab::updateName");

	if (!m_userAssignedName.empty())
	{
		if (m_name != m_userAssignedName) 
		{
			m_name = m_userAssignedName;

			if (notify)
				SendNotification(UINotification::ObjectChanged, 0);
		}
	}
	else
	{
		Unicode::String const & str = m_defaultChannel.getDisplayName();

		if (str != m_name)
		{
			m_name = str;

			if (notify)
				SendNotification(UINotification::ObjectChanged, 0);
		}
	}
}

//----------------------------------------------------------------------

void SwgCuiChatWindow::Tab::mergeTab (const SwgCuiChatWindow::Tab & rhs)
{
	//DEJA_CONTEXT("SwgCuiChatWindow::Tab::mergeTab");

	m_channels->insert (rhs.m_channels->begin (), rhs.m_channels->end ());
	
	m_defaultChannel.type = CT_none;
	m_defaultChannel = rhs.m_defaultChannel;
}

//----------------------------------------------------------------------

void SwgCuiChatWindow::Tab::setModified (const ChannelId & id, bool b, bool notify)
{
	//DEJA_CONTEXT("SwgCuiChatWindow::Tab::setModified");

	if (id.type != CT_none && !hasChannel (id))
		return;
	
	//-- the channel is already flagged modified
	if (isModified (id))
	{
		if (!b)
		{
			m_modifiedChannels->erase (id);
			
			if (notify)
				SendNotification (UINotification::ObjectChanged, 0);
		}
	}
	else if (b)
	{
		if (id.type != CT_none)
			m_modifiedChannels->insert (id);
		
		if (notify)
			SendNotification (UINotification::ObjectChanged, 0);
	}	
}

//----------------------------------------------------------------------

void SwgCuiChatWindow::Tab::setUnmodified     (bool notify)
{
	//DEJA_CONTEXT("SwgCuiChatWindow::Tab::setUnmodified");

	if (!m_modifiedChannels->empty ())
	{
		m_modifiedChannels->clear ();
		m_charactersCut = 0;

		if (notify)
			SendNotification (UINotification::ObjectChanged, 0);		
	}
}

//----------------------------------------------------------------------

inline bool SwgCuiChatWindow::Tab::isModified  (const ChannelId & id) const
{
	//DEJA_CONTEXT("SwgCuiChatWindow::Tab::isModified");

	const ChannelSet::iterator it = m_modifiedChannels->find (id);
	return it != m_modifiedChannels->end ();
}

//----------------------------------------------------------------------

bool SwgCuiChatWindow::Tab::channelsEmpty () const
{
	return m_channels->empty ();
}

//----------------------------------------------------------------------

bool SwgCuiChatWindow::Tab::updateModifiedDifference (const Tab & rhs, bool notify)
{
	//DEJA_CONTEXT("SwgCuiChatWindow::Tab::updateModifiedDifference");

	bool found = false;
	bool changed = false;

	for (ChannelSet::const_iterator it = m_modifiedChannels->begin (); it != m_modifiedChannels->end ();)
	{
		const ChannelId & id = *it;
		if (!rhs.hasChannel (id))
		{
			found = true;
			++it;
		}
		else 
		{
			m_modifiedChannels->erase (it++);
			changed = true;
		}
	}

	if (changed && notify)
	{
		SendNotification (UINotification::ObjectChanged, 0);
	}

	return found;
}

//----------------------------------------------------------------------

void SwgCuiChatWindow::Tab::setLastTabId      (int id)
{
	ms_lastTabId = id;
}

//----------------------------------------------------------------------

void SwgCuiChatWindow::Tab::setTabId          (int id)
{
	m_tabId = id;
	ms_lastTabId = std::max (id, ms_lastTabId);
}

//----------------------------------------------------------------------

void SwgCuiChatWindow::Tab::clearCharactersCut ()
{
	m_charactersCut = 0;
}

//----------------------------------------------------------------------

void SwgCuiChatWindow::Tab::setUserAssignedName(const Unicode::String & str)
{
	m_userAssignedName = str;
	updateName(true);
}


namespace 
{
	namespace Settings
	{
		const std::string numChannelsPrefix          = "numChannels";
		const std::string tabIdPrefix                = "tabId";
		const std::string channelIdPrefix            = "channelId";
		const std::string channelNamePrefix          = "channelName";
		const std::string defaultChannelTypePrefix   = "defaultChannelType";
		const std::string defaultChannelNamePrefix   = "defaultChannelName";
		const std::string channelIsPublic            = "channelIsPublic";
		const std::string userAssignedNamePrefix     = "userAssignedName";
		const std::string saveUserAssignedNamePrefix = "saveUserAssignedName";
		const std::string textPrefix                 = "text";
	}
}

//----------------------------------------------------------------------

void SwgCuiChatWindow::Tab::saveTabSettings(const std::string & mediatorName, int index) const
{
	//DEJA_CONTEXT("SwgCuiChatWindow::Tab::saveTabSettings");

	static char buf [128];
	static const size_t buf_size = sizeof (buf);

	snprintf (buf, buf_size, "%s_%d", Settings::numChannelsPrefix.c_str (), index);
	CuiSettings::saveInteger (mediatorName, buf, static_cast<int>(m_channels->size ()));

	snprintf (buf, buf_size,  "%s_%d", Settings::tabIdPrefix.c_str (), index);
	CuiSettings::saveInteger (mediatorName, buf, m_tabId);

	int channel_index = 0;

	for (ChannelSet::const_iterator it = m_channels->begin (); it != m_channels->end (); ++it, ++channel_index)
	{
		const ChannelId & cid = *it;

		snprintf (buf, buf_size, "%s_%d_%d", Settings::channelIdPrefix.c_str (), index, channel_index);
		CuiSettings::saveInteger (mediatorName, buf, static_cast<int>(cid.type));

		snprintf (buf, buf_size, "%s_%d_%d", Settings::channelNamePrefix.c_str (), index, channel_index);
		CuiSettings::saveData (mediatorName, buf, cid.getName ());

		snprintf (buf, buf_size, "%s_%d_%d", Settings::channelIsPublic.c_str (), index, channel_index);
		CuiSettings::saveBoolean (mediatorName, buf, cid.getIsPublic ());
	}

	snprintf (buf, buf_size, "%s_%d", Settings::defaultChannelTypePrefix.c_str (), index);
	CuiSettings::saveInteger (mediatorName, buf, static_cast<int>(m_defaultChannel.type));

	snprintf (buf, buf_size, "%s_%d", Settings::defaultChannelNamePrefix.c_str (), index);
	CuiSettings::saveData (mediatorName, buf, m_defaultChannel.getName ());

	snprintf (buf, buf_size, "%s_%d", Settings::userAssignedNamePrefix.c_str (), index);
	CuiSettings::saveString (mediatorName, buf, m_userAssignedName);
}

//----------------------------------------------------------------------

void SwgCuiChatWindow::Tab::saveChatHistory(const std::string & mediatorName, int index) const
{
	//DEJA_CONTEXT("SwgCuiChatWindow::Tab::saveChatHistory");

	static char buf [128];
	static const size_t buf_size = sizeof (buf);

	int const bufferSize = ConfigClientGame::getChatTabMaxOutputTextLines() * ConfigClientGame::getChatTabOutputAverageCharactersPerLine();
	int const end = static_cast<int>(m_text.size());
	int const begin = std::max((end - bufferSize), 0);

	Unicode::String const outputString = m_text.substr(begin, end);
	snprintf (buf, buf_size, "%s_%d", Settings::textPrefix.c_str (), index);
	CuiChatHistory::saveString (mediatorName, buf, outputString);
}

//----------------------------------------------------------------------

void SwgCuiChatWindow::Tab::loadTabSettings     (const std::string & mediatorName, int index)
{
	//DEJA_CONTEXT("SwgCuiChatWindow::Tab::loadTabSettings");

	static char buf [128];
	static const size_t buf_size = sizeof (buf);

	snprintf (buf, buf_size,  "%s_%d", Settings::tabIdPrefix.c_str (), index);
	int tabId = 0;
	if (CuiSettings::loadInteger (mediatorName, buf, tabId))
		setTabId (tabId);

	int numChannels = 0;
	snprintf (buf, buf_size, "%s_%d", Settings::numChannelsPrefix.c_str (), index);
	if (CuiSettings::loadInteger (mediatorName, buf, numChannels))
	{
		for (int channel_index = 0; channel_index < numChannels; ++channel_index)
		{
			int type = 0;
			std::string name;

			snprintf (buf, buf_size, "%s_%d_%d", Settings::channelIdPrefix.c_str (), index, channel_index);
			if (!CuiSettings::loadInteger (mediatorName, buf, type))
				WARNING (true, (""));
			else
			{
				snprintf (buf, buf_size, "%s_%d_%d", Settings::channelNamePrefix.c_str (), index, channel_index);
				if (!CuiSettings::loadData (mediatorName, buf, name))
					WARNING (true, (""));
				else
				{
					bool isPublic = true;
					snprintf (buf, buf_size, "%s_%d_%d", Settings::channelIsPublic.c_str (), index, channel_index);
					IGNORE_RETURN(CuiSettings::loadBoolean(mediatorName, buf, isPublic));
					if(type != CT_named && ! name.empty())
					{
						const ChannelId cid (static_cast<ChannelType>(type), name, isPublic);
						addChannel (cid);
						if(type == CT_chatRoom)
						{
							if(isPublic)
							{
								Unicode::String none;
								CuiChatRoomManager::enterRoom(name, none);
							}
						}
					}
				}
			}
		}
	}

	int defaultChannelType = 0;
	snprintf (buf, buf_size, "%s_%d", Settings::defaultChannelTypePrefix.c_str (), index);
	if (!CuiSettings::loadInteger (mediatorName, buf, defaultChannelType))
		WARNING (true, (""));
	else
	{
		std::string defaultChannelName;
		snprintf (buf, buf_size, "%s_%d", Settings::defaultChannelNamePrefix.c_str (), index);
		if (!CuiSettings::loadData (mediatorName, buf, defaultChannelName))
			WARNING (true, (""));
		else
		{
			const ChannelId cid (static_cast<ChannelType>(defaultChannelType), defaultChannelName);
			setDefaultChannel (cid);
		}
	}

	Unicode::String userAssignedName;
	snprintf (buf, buf_size, "%s_%d", Settings::userAssignedNamePrefix.c_str (), index);
	IGNORE_RETURN(CuiSettings::loadString (mediatorName, buf, userAssignedName));
	setUserAssignedName(userAssignedName);

	Unicode::String text;
	snprintf (buf, buf_size, "%s_%d", Settings::textPrefix.c_str (), index);
	if (CuiSettings::loadString (mediatorName, buf, text))
	{
		m_text = text;
		// remove the chat history from the CuiSettings, it's going to be saved in CuiChatHistory now
		CuiSettings::saveString (mediatorName, buf, Unicode::String());
	}
}

//----------------------------------------------------------------------

void SwgCuiChatWindow::Tab::loadChatHistory     (const std::string & mediatorName, int index)
{
	//DEJA_CONTEXT("SwgCuiChatWindow::Tab::loadChatHistory");

	static char buf [128];
	static const size_t buf_size = sizeof (buf);

	Unicode::String text;
	snprintf (buf, buf_size, "%s_%d", Settings::textPrefix.c_str (), index);
	if (CuiChatHistory::loadString (mediatorName, buf, text))
		m_text = text;
}


//======================================================================
void SwgCuiChatWindow::Tab::resetAppendText()
{
	m_appendText.clear();
}

//======================================================================
