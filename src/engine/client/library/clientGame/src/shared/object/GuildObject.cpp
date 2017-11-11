// ======================================================================
//
// GuildObject.cpp
//
// Copyright 2002 Sony Online Entertainment
//
// ======================================================================

#include "clientGame/FirstClientGame.h"
#include "clientGame/GuildObject.h"

#include "clientGame/GameNetwork.h"
#include "sharedFoundation/CalendarTime.h"
#include "sharedFoundation/DebugInfoManager.h"
#include "sharedFoundation/Production.h"
#include "sharedGame/SharedGuildObjectTemplate.h"
#include "sharedMessageDispatch/Transceiver.h"
#include <map>

// ======================================================================

namespace GuildObjectNamespace
{
	static std::map<int, char const *> s_guildAbbrevMap;
	static char const *s_nullStr = "";

	typedef stdmap<int, Unicode::String>::fwd GuildAbbrevUnicodeMap;
	GuildAbbrevUnicodeMap s_guildAbbrevUnicodeMap;

	GuildObject const * s_theGuildObject = NULL;

	std::string const ms_debugInfoSectionName("GuildObject");

	namespace Transceivers
	{
		MessageDispatch::Transceiver<const GuildObject::Messages::GCWScoreUpdatedThisGalaxy::Payload &, GuildObject::Messages::GCWScoreUpdatedThisGalaxy>
			gcwScoreUpdatedThisGalaxy;
		
		MessageDispatch::Transceiver<const GuildObject::Messages::GCWScoreUpdatedOtherGalaxies::Payload &, GuildObject::Messages::GCWScoreUpdatedOtherGalaxies>
			gcwScoreUpdatedOtherGalaxies;

		MessageDispatch::Transceiver<const GuildObject::Messages::GCWGroupScoreUpdatedThisGalaxy::Payload &, GuildObject::Messages::GCWGroupScoreUpdatedThisGalaxy>
			gcwGroupScoreUpdatedThisGalaxy;

		MessageDispatch::Transceiver<const GuildObject::Messages::GCWGroupScoreUpdatedOtherGalaxies::Payload &, GuildObject::Messages::GCWGroupScoreUpdatedOtherGalaxies>
			gcwGroupScoreUpdatedOtherGalaxies;

	}
}

using namespace GuildObjectNamespace;

// ======================================================================

GuildObject::GuildObject(SharedGuildObjectTemplate const *t) :
	UniverseObject(t),
	m_abbrevs(),
	m_gcwImperialScorePercentileThisGalaxy(),
	m_gcwGroupImperialScorePercentileThisGalaxy(),
	m_gcwImperialScorePercentileHistoryThisGalaxy(),
	m_gcwGroupImperialScorePercentileHistoryThisGalaxy(),
	m_gcwImperialScorePercentileOtherGalaxies(),
	m_gcwGroupImperialScorePercentileOtherGalaxies()
{
	addSharedVariable(m_abbrevs);
	addSharedVariable_np(m_gcwImperialScorePercentileThisGalaxy);
	addSharedVariable_np(m_gcwGroupImperialScorePercentileThisGalaxy);
	addSharedVariable_np(m_gcwImperialScorePercentileHistoryThisGalaxy);
	addSharedVariable_np(m_gcwGroupImperialScorePercentileHistoryThisGalaxy);
	addSharedVariable_np(m_gcwImperialScorePercentileOtherGalaxies);
	addSharedVariable_np(m_gcwGroupImperialScorePercentileOtherGalaxies);

	m_abbrevs.setOnChanged(this, &GuildObject::guildOnChanged);

	m_gcwImperialScorePercentileThisGalaxy.setOnErase(this, &GuildObject::gcwScoreThisGalaxyOnErase);
	m_gcwImperialScorePercentileThisGalaxy.setOnInsert(this, &GuildObject::gcwScoreThisGalaxyOnInsert);
	m_gcwImperialScorePercentileThisGalaxy.setOnSet(this, &GuildObject::gcwScoreThisGalaxyOnSet);

	m_gcwImperialScorePercentileOtherGalaxies.setOnErase(this, &GuildObject::gcwScoreOtherGalaxiesOnErase);
	m_gcwImperialScorePercentileOtherGalaxies.setOnInsert(this, &GuildObject::gcwScoreOtherGalaxiesOnInsert);
	m_gcwImperialScorePercentileOtherGalaxies.setOnSet(this, &GuildObject::gcwScoreOtherGalaxiesOnSet);

	m_gcwGroupImperialScorePercentileThisGalaxy.setOnErase(this, &GuildObject::gcwGroupScoresThisGalaxyOnErase);
	m_gcwGroupImperialScorePercentileThisGalaxy.setOnInsert(this, &GuildObject::gcwGroupScoresThisGalaxyOnInsert);
	m_gcwGroupImperialScorePercentileThisGalaxy.setOnSet(this, &GuildObject::gcwGroupScoresThisGalaxyOnSet);

	m_gcwGroupImperialScorePercentileOtherGalaxies.setOnErase(this, &GuildObject::gcwGroupScoresOtherGalaxiesOnErase);
	m_gcwGroupImperialScorePercentileOtherGalaxies.setOnInsert(this, &GuildObject::gcwGroupScoresOtherGalaxiesOnInsert);
	m_gcwGroupImperialScorePercentileOtherGalaxies.setOnSet(this, &GuildObject::gcwGroupScoresOtherGalaxiesOnSet);

	s_theGuildObject = this;
}

// ----------------------------------------------------------------------

GuildObject::~GuildObject()
{
	s_guildAbbrevMap.clear        ();
	s_guildAbbrevUnicodeMap.clear ();

	if (s_theGuildObject == this)
	{
		s_theGuildObject = NULL;
	}
}

// ----------------------------------------------------------------------

void GuildObject::guildOnChanged()
{
	s_guildAbbrevMap.clear        ();
	s_guildAbbrevUnicodeMap.clear ();

	std::set<std::string> const &abbrevs = m_abbrevs.get();
	for (std::set<std::string>::const_iterator i = abbrevs.begin(); i != abbrevs.end(); ++i)
	{
		std::string const &source = *i;
		int pos = 0;
		while (source[pos] && source[pos] != ':')
			++pos;
		if (source[pos] == ':')
			++pos;

		const char * const abbrev = source.c_str()+pos;
		const int id = atoi(source.c_str());

		s_guildAbbrevMap.insert        (std::make_pair (id, abbrev));
		s_guildAbbrevUnicodeMap.insert (std::make_pair (id, Unicode::narrowToWide (abbrev)));
	}
}

//----------------------------------------------------------------------

GuildObject const * GuildObject::getGuildObject()
{
	return s_theGuildObject;
}

// ----------------------------------------------------------------------

char const *GuildObject::getGuildAbbrev(int guildId) // static
{
	const std::map<int, char const *>::const_iterator i = s_guildAbbrevMap.find(guildId);
	if (i != s_guildAbbrevMap.end())
		return (*i).second;
	return s_nullStr;
}

//----------------------------------------------------------------------

const Unicode::String & GuildObject::getGuildAbbrevUnicode (int guildId) // static
{
	const GuildAbbrevUnicodeMap::const_iterator i = s_guildAbbrevUnicodeMap.find(guildId);
	if (i != s_guildAbbrevUnicodeMap.end())
		return (*i).second;
	return Unicode::emptyString;
}

//-----------------------------------------------------------------------

void GuildObject::getObjectInfo(std::map<std::string, std::map<std::string, Unicode::String> > & propertyMap) const
{
#if PRODUCTION == 0
	/**
	When adding a variable to this class, please add it here.  Variable that aren't easily displayable are still listed, for tracking purposes.
	*/

	/**
	Don't compile in production build because this maps human-readable values to data members and makes hacking easier
	*/

	Archive::AutoDeltaMap<std::string, int>::const_iterator iterThisGalaxy = m_gcwImperialScorePercentileThisGalaxy.begin();
	Archive::AutoDeltaMap<std::string, int>::const_iterator iterThisGalaxyEnd = m_gcwImperialScorePercentileThisGalaxy.end();
	std::string gcwScoreCategory;
	for (; iterThisGalaxy != iterThisGalaxyEnd; ++iterThisGalaxy)
	{
		gcwScoreCategory = "GcwScoreCategory.";
		gcwScoreCategory += GameNetwork::getCentralServerName();
		gcwScoreCategory += ".";
		gcwScoreCategory += iterThisGalaxy->first;
		gcwScoreCategory += ".imperial";

		DebugInfoManager::addProperty(propertyMap, ms_debugInfoSectionName + std::string(".GcwScore.") + GameNetwork::getCentralServerName(), gcwScoreCategory, iterThisGalaxy->second);
	}

	iterThisGalaxy = m_gcwGroupImperialScorePercentileThisGalaxy.begin();
	iterThisGalaxyEnd = m_gcwGroupImperialScorePercentileThisGalaxy.end();
	for (; iterThisGalaxy != iterThisGalaxyEnd; ++iterThisGalaxy)
	{
		gcwScoreCategory = "GcwScoreGroup.";
		gcwScoreCategory += GameNetwork::getCentralServerName();
		gcwScoreCategory += ".";
		gcwScoreCategory += iterThisGalaxy->first;
		gcwScoreCategory += ".imperial";

		DebugInfoManager::addProperty(propertyMap, ms_debugInfoSectionName + std::string(".GcwScore.") + GameNetwork::getCentralServerName(), gcwScoreCategory, iterThisGalaxy->second);
	}

	Archive::AutoDeltaMap<std::pair<std::string, std::string>, int>::const_iterator iterOtherGalaxy = m_gcwImperialScorePercentileOtherGalaxies.begin();
	Archive::AutoDeltaMap<std::pair<std::string, std::string>, int>::const_iterator iterOtherGalaxyEnd = m_gcwImperialScorePercentileOtherGalaxies.end();
	for (; iterOtherGalaxy != iterOtherGalaxyEnd; ++iterOtherGalaxy)
	{
		gcwScoreCategory = "GcwScoreCategory.";
		gcwScoreCategory += iterOtherGalaxy->first.first;
		gcwScoreCategory += ".";
		gcwScoreCategory += iterOtherGalaxy->first.second;
		gcwScoreCategory += ".imperial";

		DebugInfoManager::addProperty(propertyMap, ms_debugInfoSectionName + std::string(".GcwScore.") + iterOtherGalaxy->first.first, gcwScoreCategory, iterOtherGalaxy->second);
	}

	iterOtherGalaxy = m_gcwGroupImperialScorePercentileOtherGalaxies.begin();
	iterOtherGalaxyEnd = m_gcwGroupImperialScorePercentileOtherGalaxies.end();
	for (; iterOtherGalaxy != iterOtherGalaxyEnd; ++iterOtherGalaxy)
	{
		gcwScoreCategory = "GcwScoreGroup.";
		gcwScoreCategory += iterOtherGalaxy->first.first;
		gcwScoreCategory += ".";
		gcwScoreCategory += iterOtherGalaxy->first.second;
		gcwScoreCategory += ".imperial";

		DebugInfoManager::addProperty(propertyMap, ms_debugInfoSectionName + std::string(".GcwScore.") + iterOtherGalaxy->first.first, gcwScoreCategory, iterOtherGalaxy->second);
	}

	Archive::AutoDeltaMap<std::pair<std::string, int>, int>::const_iterator iterHistoryThisGalaxy = m_gcwImperialScorePercentileHistoryThisGalaxy.begin();
	Archive::AutoDeltaMap<std::pair<std::string, int>, int>::const_iterator iterHistoryThisGalaxyEnd = m_gcwImperialScorePercentileHistoryThisGalaxy.end();
	for (; iterHistoryThisGalaxy != iterHistoryThisGalaxyEnd; ++iterHistoryThisGalaxy)
	{
		gcwScoreCategory = "GcwScoreCategoryHistory.";
		gcwScoreCategory += GameNetwork::getCentralServerName();
		gcwScoreCategory += ".";
		gcwScoreCategory += iterHistoryThisGalaxy->first.first;
		gcwScoreCategory += ".imperial.";
		gcwScoreCategory += CalendarTime::convertEpochToTimeStringLocal_YYYYMMDDHHMMSS(static_cast<time_t>(iterHistoryThisGalaxy->first.second));

		DebugInfoManager::addProperty(propertyMap, ms_debugInfoSectionName + std::string(".GcwScoreHistory.") + GameNetwork::getCentralServerName(), gcwScoreCategory, iterHistoryThisGalaxy->second);
	}

	iterHistoryThisGalaxy = m_gcwGroupImperialScorePercentileHistoryThisGalaxy.begin();
	iterHistoryThisGalaxyEnd = m_gcwGroupImperialScorePercentileHistoryThisGalaxy.end();
	for (; iterHistoryThisGalaxy != iterHistoryThisGalaxyEnd; ++iterHistoryThisGalaxy)
	{
		gcwScoreCategory = "GcwScoreGroupHistory.";
		gcwScoreCategory += GameNetwork::getCentralServerName();
		gcwScoreCategory += ".";
		gcwScoreCategory += iterHistoryThisGalaxy->first.first;
		gcwScoreCategory += ".imperial.";
		gcwScoreCategory += CalendarTime::convertEpochToTimeStringLocal_YYYYMMDDHHMMSS(static_cast<time_t>(iterHistoryThisGalaxy->first.second));

		DebugInfoManager::addProperty(propertyMap, ms_debugInfoSectionName + std::string(".GcwScoreHistory.") + GameNetwork::getCentralServerName(), gcwScoreCategory, iterHistoryThisGalaxy->second);
	}

	UniverseObject::getObjectInfo(propertyMap);

#else
	UNREF(propertyMap);
#endif
}

//-----------------------------------------------------------------------

void GuildObject::gcwScoreThisGalaxyOnErase(std::string const & keyValue, int const & value)
{
	UNREF(keyValue);
	UNREF(value);

	// the false means that this is from an erase/insert operation vs a set/change operation
	Transceivers::gcwScoreUpdatedThisGalaxy.emitMessage(GuildObject::Messages::GCWScoreUpdatedThisGalaxy::Payload(keyValue, std::make_pair(false, std::make_pair(value, 0))));
}

//-----------------------------------------------------------------------

void GuildObject::gcwScoreThisGalaxyOnInsert(std::string const & keyValue, int const & value)
{
	UNREF(keyValue);
	UNREF(value);

	// the false means that this is from an erase/insert operation vs a set/change operation
	Transceivers::gcwScoreUpdatedThisGalaxy.emitMessage(GuildObject::Messages::GCWScoreUpdatedThisGalaxy::Payload(keyValue, std::make_pair(false, std::make_pair(0, value))));
}

//-----------------------------------------------------------------------

void GuildObject::gcwScoreThisGalaxyOnSet(std::string const & keyValue, int const & oldValue, int const & newValue)
{
	UNREF(keyValue);
	UNREF(oldValue);
	UNREF(newValue);

	// the true means that this is from a set/change operation vs an erase/insert operation
	Transceivers::gcwScoreUpdatedThisGalaxy.emitMessage(GuildObject::Messages::GCWScoreUpdatedThisGalaxy::Payload(keyValue, std::make_pair(true, std::make_pair(oldValue, newValue))));
}

//-----------------------------------------------------------------------

void GuildObject::gcwGroupScoresThisGalaxyOnErase(std::string const & keyValue, int const & value)
{
	UNREF(keyValue);
	UNREF(value);
	Transceivers::gcwGroupScoreUpdatedThisGalaxy.emitMessage(GuildObject::Messages::GCWGroupScoreUpdatedThisGalaxy::Payload(keyValue, value));
}

//-----------------------------------------------------------------------

void GuildObject::gcwGroupScoresThisGalaxyOnInsert(std::string const & keyValue, int const & value)
{
	UNREF(keyValue);
	UNREF(value);
	Transceivers::gcwGroupScoreUpdatedThisGalaxy.emitMessage(GuildObject::Messages::GCWGroupScoreUpdatedThisGalaxy::Payload(keyValue, value));
}

//-----------------------------------------------------------------------

void GuildObject::gcwGroupScoresThisGalaxyOnSet(std::string const & keyValue, int const & oldValue, int const & newValue)
{
	UNREF(keyValue);
	UNREF(oldValue);
	UNREF(newValue);
	Transceivers::gcwGroupScoreUpdatedThisGalaxy.emitMessage(GuildObject::Messages::GCWGroupScoreUpdatedThisGalaxy::Payload(keyValue, newValue));
}

//-----------------------------------------------------------------------

void GuildObject::gcwScoreOtherGalaxiesOnErase(std::pair<std::string, std::string> const & keyValue, int const & value)
{
	UNREF(keyValue);
	UNREF(value);
	Transceivers::gcwScoreUpdatedOtherGalaxies.emitMessage(GuildObject::Messages::GCWScoreUpdatedOtherGalaxies::Payload(keyValue, value));
}

//-----------------------------------------------------------------------

void GuildObject::gcwScoreOtherGalaxiesOnInsert(std::pair<std::string, std::string> const & keyValue, int const & value)
{
	UNREF(keyValue);
	UNREF(value);
	Transceivers::gcwScoreUpdatedOtherGalaxies.emitMessage(GuildObject::Messages::GCWScoreUpdatedOtherGalaxies::Payload(keyValue, value));
}

//-----------------------------------------------------------------------

void GuildObject::gcwScoreOtherGalaxiesOnSet(std::pair<std::string, std::string> const & keyValue, int const & oldValue, int const & newValue)
{
	UNREF(keyValue);
	UNREF(oldValue);
	UNREF(newValue);
	Transceivers::gcwScoreUpdatedOtherGalaxies.emitMessage(GuildObject::Messages::GCWScoreUpdatedOtherGalaxies::Payload(keyValue, newValue));
}

//-----------------------------------------------------------------------

void GuildObject::gcwGroupScoresOtherGalaxiesOnErase(std::pair<std::string, std::string> const & keyValue, int const & value)
{
	UNREF(keyValue);
	UNREF(value);
	Transceivers::gcwGroupScoreUpdatedOtherGalaxies.emitMessage(GuildObject::Messages::GCWGroupScoreUpdatedOtherGalaxies::Payload(keyValue, value));
}

//-----------------------------------------------------------------------

void GuildObject::gcwGroupScoresOtherGalaxiesOnInsert(std::pair<std::string, std::string> const & keyValue, int const & value)
{
	UNREF(keyValue);
	UNREF(value);
	Transceivers::gcwGroupScoreUpdatedOtherGalaxies.emitMessage(GuildObject::Messages::GCWGroupScoreUpdatedOtherGalaxies::Payload(keyValue, value));
}

//-----------------------------------------------------------------------

void GuildObject::gcwGroupScoresOtherGalaxiesOnSet(std::pair<std::string, std::string> const & keyValue, int const & oldValue, int const & newValue)
{
	UNREF(keyValue);
	UNREF(oldValue);
	UNREF(newValue);
	Transceivers::gcwGroupScoreUpdatedOtherGalaxies.emitMessage(GuildObject::Messages::GCWGroupScoreUpdatedOtherGalaxies::Payload(keyValue, newValue));
}

// ======================================================================
