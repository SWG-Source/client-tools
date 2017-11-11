//======================================================================
//
// MoodManagerClient.cpp
// copyright (c) 2002 Sony Online Entertainment
//
//======================================================================

#include "clientGame/FirstClientGame.h"
#include "clientGame/MoodManagerClient.h"

#include "LocalizationManager.h"
#include "LocalizedStringTable.h"
#include "UnicodeUtils.h"
#include "sharedDebug/InstallTimer.h"
#include "sharedMessageDispatch/Message.h"
#include "sharedMessageDispatch/Receiver.h"
#include "sharedMessageDispatch/Transceiver.h"
#include "sharedObject/World.h"
#include "sharedRandom/Random.h"
#include "sharedGame/MoodManager.h"

//======================================================================

namespace
{
	//----------------------------------------------------------------------

	namespace MoodTypes
	{
		namespace Lines
		{
			const size_t NAME         = 0;
			const size_t EMOTE        = 1;
			const size_t ADVERB_START = 2;
		}

		namespace Names
		{
			const std::string idle   = "neutral";
		}
	}

	//----------------------------------------------------------------------

	const std::string MOOD_TABLE_NAME = "mood_types";

	//----------------------------------------------------------------------

	//-- @todo: localize these

	const Unicode::String s_moodCommand     = Unicode::narrowToWide ("mood");
	const Unicode::String s_moodListCommand = Unicode::narrowToWide ("moods");

	//----------------------------------------------------------------------

	typedef std::map<uint32, Unicode::String> MoodIdNameMap;
	typedef std::map<Unicode::String, uint32> MoodNameIdMap;
	typedef std::map<uint32, std::string>     MoodIdAnimationMap;
	typedef std::vector<Unicode::String>      StringVector;
	typedef std::map<uint32, StringVector>    MoodIdAdverbMap;

	MoodIdNameMap              s_moodIdNameMap;
	MoodNameIdMap              s_moodNameIdMap;

	MoodIdAnimationMap         s_moodIdAnimationMap;
	MoodIdAdverbMap            s_moodIdAdverbMap;

	bool                       s_installed;

	LocalizedStringTable *     s_moodsTable = 0;

	struct AbbrevFinder
	{
		const Unicode::String & m_str;

		AbbrevFinder (const Unicode::String & str) : m_str (str) {}

		bool operator () (const std::pair<Unicode::String, uint32> & t) const
		{
			return t.first.compare (0, m_str.length (), m_str) == 0;
		}

		AbbrevFinder (const AbbrevFinder & rhs) : m_str (rhs.m_str) {}

	private:
		AbbrevFinder & operator=(const AbbrevFinder &);
	};

	//-----------------------------------------------------------------

	inline  Unicode::String chooseRandomString (const LocalizedString & locstr, size_t start)
	{
		if (locstr.getNumLines () > 1)
			return locstr.getStringLine (static_cast<size_t>(Random::random (start, static_cast<int32>(locstr.getNumLines ()) - 1)));

		return locstr.getString ();
	}
}

//-----------------------------------------------------------------

void MoodManagerClient::install ()
{
	InstallTimer const installTimer("MoodManagerClient::install");

	DEBUG_FATAL (s_installed, ("already installed.\n"));

	s_moodIdNameMap.clear ();
	s_moodNameIdMap.clear ();
	s_moodIdAnimationMap.clear ();
	s_moodIdAdverbMap.clear ();

	s_moodsTable     = LocalizationManager::getManager ().fetchStringTable (MOOD_TABLE_NAME);

	if (s_moodsTable)
	{
		const LocalizedStringTable::NameMap_t & nameMap = s_moodsTable->getNameMap ();

		for (LocalizedStringTable::NameMap_t::const_iterator it = nameMap.begin (); it != nameMap.end (); ++it)
		{
			const std::string & name = (*it).first;

			if (name != "none")
			{
				const uint32 stringId    = (*it).second;

				const uint32 moodType = MoodManager::getMoodByCanonicalName (name);

				if (moodType)
				{
					const LocalizedString * const locStr = s_moodsTable->getLocalizedString (stringId);
					NOT_NULL (locStr);

					if (locStr)
					{
						const Unicode::String command = Unicode::narrowToWide(name);
						const Unicode::String dummyName = locStr->getStringLine (MoodTypes::Lines::NAME);
						UNREF(dummyName);
						const Unicode::String animation = locStr->getStringLine (MoodTypes::Lines::EMOTE);

						const Unicode::String lowerCommand (Unicode::toLower (command));

						IGNORE_RETURN (s_moodIdNameMap.insert      (std::make_pair (moodType, command)));
						IGNORE_RETURN (s_moodNameIdMap.insert      (std::make_pair (lowerCommand, moodType)));
						IGNORE_RETURN (s_moodIdAnimationMap.insert (std::make_pair (moodType, Unicode::wideToNarrow (animation))));

						StringVector sv;

						const size_t numLines = locStr->getNumLines ();

						for (size_t i = MoodTypes::Lines::ADVERB_START; i < numLines; ++i)
							sv.push_back (locStr->getStringLine (i));

						IGNORE_RETURN (s_moodIdAdverbMap.insert (std::make_pair (moodType, sv)));
					}
				}
				else
					WARNING_STRICT_FATAL (true, ("MoodManagerClient '%s' specifies invalid mood type '%s'", MOOD_TABLE_NAME.c_str (), name.c_str ()));
			}
		}

	}
	else
		WARNING_STRICT_FATAL (true, ("Unable to locate moods table: %s", MOOD_TABLE_NAME.c_str ()));

	s_installed = true;
}

//-----------------------------------------------------------------

void MoodManagerClient::remove ()
{
	DEBUG_FATAL (!s_installed, ("not installed.\n"));

	s_moodIdNameMap.clear ();
	s_moodNameIdMap.clear ();
	s_moodIdAnimationMap.clear ();
	s_moodIdAdverbMap.clear ();

	if (s_moodsTable)
		LocalizationManager::getManager ().releaseStringTable (s_moodsTable);

	s_installed = false;
}

//-----------------------------------------------------------------

uint32 MoodManagerClient::getMoodByName (const Unicode::String & name)
{
	DEBUG_FATAL (!s_installed, ("not installed.\n"));

	const Unicode::String & lowerName = Unicode::toLower (name);

	const MoodNameIdMap::const_iterator it = s_moodNameIdMap.find (lowerName);

	if (it != s_moodNameIdMap.end ())
	{
		return (*it).second;
	}

	return 0;
}

//----------------------------------------------------------------------

uint32 MoodManagerClient::getMoodByAbbrevNoCase (const Unicode::String & abbrev)
{
	DEBUG_FATAL (!s_installed, ("not installed.\n"));

	const Unicode::String lowerAbbrev (Unicode::toLower (abbrev));

	const MoodNameIdMap::const_iterator it = std::find_if (s_moodNameIdMap.begin (), s_moodNameIdMap.end (), AbbrevFinder (lowerAbbrev));
	if (it != s_moodNameIdMap.end ())
	{
		return (*it).second;
	}

	return 0;
}

//----------------------------------------------------------------------

bool MoodManagerClient::getMoodName (uint32 mood, Unicode::String & name)
{
	DEBUG_FATAL (!s_installed, ("not installed.\n"));

	const MoodIdNameMap::const_iterator it = s_moodIdNameMap.find (mood);

	if (it != s_moodIdNameMap.end ())
	{
		name = (*it).second;
		return true;
	}

	return false;
}

//----------------------------------------------------------------------

bool MoodManagerClient::getMoodEmoteAnimation (uint32 mood, std::string & emoteName)
{
	DEBUG_FATAL (!s_installed, ("not installed.\n"));

	if (mood == 0)
	{
		emoteName = MoodTypes::Names::idle;
		return true;
	}

	const MoodIdAnimationMap::const_iterator it = s_moodIdAnimationMap.find (mood);

	if (it != s_moodIdAnimationMap.end ())
	{
		emoteName = (*it).second;

		return (!emoteName.empty () && emoteName [0] != '~');
	}

	WARNING (true, ("mood not found: %d", mood));
	return false;
}

//-----------------------------------------------------------------

/*
* @param random choose a adverb phrase randomly, or simply select the first one.  The first adverbial phrase in the string table entry should be the simple form.
*/

Unicode::String MoodManagerClient::getMoodAdverbPhrase (uint32 moodType, bool random)
{
	DEBUG_FATAL (!s_installed, ("not installed.\n"));
	DEBUG_FATAL (moodType == 0, ("Don't call getMoodAdverbPhrase with a zero moodType.\n"));

	const MoodIdAdverbMap::const_iterator it = s_moodIdAdverbMap.find (moodType);

	if (it != s_moodIdAdverbMap.end ())
	{

		const StringVector & sv = (*it).second;

		if (sv.empty ())
			return Unicode::narrowToWide ("ERROR ADVERB PHRASE IS EMPTY");
		else
		{
			size_t index = 0;
			if (random)
				index = Random::random (0, sv.size () - 1);

			return sv [index];
		}
	}

	return Unicode::narrowToWide ("ERROR ADVERB PHRASE NOT FOUND");
}

//----------------------------------------------------------------------

const LocalizedStringTable *  MoodManagerClient::getMoodsTable ()
{
	DEBUG_FATAL (!s_installed, ("not installed.\n"));
	return s_moodsTable;
}

//----------------------------------------------------------------------

const Unicode::String & MoodManagerClient::getMoodCommand     ()
{
	return s_moodCommand;
}

//----------------------------------------------------------------------

const Unicode::String & MoodManagerClient::getMoodListCommand ()
{
	return s_moodListCommand;
}

//======================================================================
