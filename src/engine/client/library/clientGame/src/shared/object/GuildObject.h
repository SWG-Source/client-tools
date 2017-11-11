// ======================================================================
//
// GuildObject.h
//
// Copyright 2002 Sony Online Entertainment
//
// ======================================================================

#ifndef	_INCLUDED_GuildObject_H
#define	_INCLUDED_GuildObject_H

// ======================================================================

#include "Archive/AutoDeltaMap.h"
#include "Archive/AutoDeltaSet.h"
#include "clientGame/UniverseObject.h"

// ======================================================================

class SharedGuildObjectTemplate;

// ======================================================================

class GuildObject: public UniverseObject
{
public:
	struct Messages
	{
		struct GCWScoreUpdatedThisGalaxy 
		{ 
			typedef std::pair<std::string, std::pair<bool, std::pair<int, int> > > Payload; 
		};
		struct GCWScoreUpdatedOtherGalaxies
		{
			typedef std::pair<std::pair<std::string, std::string>, int> Payload;
		};
		struct GCWGroupScoreUpdatedThisGalaxy
		{
			typedef std::pair<std::string, int> Payload;
		};
		struct GCWGroupScoreUpdatedOtherGalaxies
		{
			typedef std::pair<std::pair<std::string, std::string>, int> Payload;
		};
	};

	explicit GuildObject(SharedGuildObjectTemplate const *sharedGuildObjectTemplate);
	~GuildObject();

	static GuildObject const *     getGuildObject        ();
	static char const *            getGuildAbbrev        (int guildId);
	static const Unicode::String & getGuildAbbrevUnicode (int guildId);

	std::map<std::string, int> const & getGcwImperialScorePercentileThisGalaxy() const;
	std::map<std::string, int> const & getGcwGroupImperialScorePercentileThisGalaxy() const;

	std::map<std::pair<std::string, int>, int> const & getGcwImperialScorePercentileHistoryThisGalaxy() const;
	std::map<std::pair<std::string, int>, int> const & getGcwGroupImperialScorePercentileHistoryThisGalaxy() const;

	std::map<std::pair<std::string, std::string>, int> const & getGcwImperialScorePercentileOtherGalaxies() const;
	std::map<std::pair<std::string, std::string>, int> const & getGcwGroupImperialScorePercentileOtherGalaxies() const;

	virtual void getObjectInfo(stdmap<std::string, stdmap<std::string, Unicode::String>::fwd >::fwd & propertyMap) const;

private:
	void guildOnChanged();

	// Functions to hit our transceivers.
	void gcwScoreThisGalaxyOnErase(std::string const & keyValue, int const & value);
	void gcwScoreThisGalaxyOnInsert(std::string const & keyValue, int const & value);
	void gcwScoreThisGalaxyOnSet(std::string const & keyValue, int const & oldValue, int const & newValue);

	void gcwGroupScoresThisGalaxyOnErase(std::string const & keyValue, int const & value);
	void gcwGroupScoresThisGalaxyOnInsert(std::string const & keyValue, int const & value);
	void gcwGroupScoresThisGalaxyOnSet(std::string const & keyValue, int const & oldValue, int const & newValue);

	void gcwScoreOtherGalaxiesOnErase(std::pair<std::string, std::string> const & keyValue, int const & value);
	void gcwScoreOtherGalaxiesOnInsert(std::pair<std::string, std::string> const & keyValue, int const & value);
	void gcwScoreOtherGalaxiesOnSet(std::pair<std::string, std::string> const & keyValue, int const & oldValue, int const & newValue);

	void gcwGroupScoresOtherGalaxiesOnErase(std::pair<std::string, std::string> const & keyValue, int const & value);
	void gcwGroupScoresOtherGalaxiesOnInsert(std::pair<std::string, std::string> const & keyValue, int const & value);
	void gcwGroupScoresOtherGalaxiesOnSet(std::pair<std::string, std::string> const & keyValue, int const & oldValue, int const & newValue);

private:
	GuildObject();
	GuildObject &operator= (GuildObject const &rhs);
	GuildObject(GuildObject const &source);

private:

	Archive::AutoDeltaSet<std::string, GuildObject> m_abbrevs;

	// GCW score tracking

	// std::map<gcwScoreCategory, percentile>
	Archive::AutoDeltaMap<std::string, int, GuildObject> m_gcwImperialScorePercentileThisGalaxy;

	// std::map<gcwScoreGroup, percentile>
	Archive::AutoDeltaMap<std::string, int, GuildObject> m_gcwGroupImperialScorePercentileThisGalaxy;

	// std::map<std::pair<gcwScoreCategory, time when percentile was calculated>, percentile>
	Archive::AutoDeltaMap<std::pair<std::string, int>, int> m_gcwImperialScorePercentileHistoryThisGalaxy;

	// std::map<std::pair<gcwScoreGroup, time when percentile was calculated>, percentile>
	Archive::AutoDeltaMap<std::pair<std::string, int>, int> m_gcwGroupImperialScorePercentileHistoryThisGalaxy;

	// std::map<std::pair<galaxyName, gcwScoreCategory>, percentile> (galaxyName of SWG is the score for the entire game)
	Archive::AutoDeltaMap<std::pair<std::string, std::string>, int, GuildObject> m_gcwImperialScorePercentileOtherGalaxies;

	// std::map<std::pair<galaxyName, gcwScoreGroup>, percentile> (galaxyName of SWG is the score for the entire game)
	Archive::AutoDeltaMap<std::pair<std::string, std::string>, int, GuildObject> m_gcwGroupImperialScorePercentileOtherGalaxies;
};

// ----------------------------------------------------------------------

inline std::map<std::string, int> const & GuildObject::getGcwImperialScorePercentileThisGalaxy() const
{
	return m_gcwImperialScorePercentileThisGalaxy.getMap();
}

// ----------------------------------------------------------------------

inline std::map<std::string, int> const & GuildObject::getGcwGroupImperialScorePercentileThisGalaxy() const
{
	return m_gcwGroupImperialScorePercentileThisGalaxy.getMap();
}

// ----------------------------------------------------------------------

inline std::map<std::pair<std::string, int>, int> const & GuildObject::getGcwImperialScorePercentileHistoryThisGalaxy() const
{
	return m_gcwImperialScorePercentileHistoryThisGalaxy.getMap();
}

// ----------------------------------------------------------------------

inline std::map<std::pair<std::string, int>, int> const & GuildObject::getGcwGroupImperialScorePercentileHistoryThisGalaxy() const
{
	return m_gcwGroupImperialScorePercentileHistoryThisGalaxy.getMap();
}

// ----------------------------------------------------------------------

inline std::map<std::pair<std::string, std::string>, int> const & GuildObject::getGcwImperialScorePercentileOtherGalaxies() const
{
	return m_gcwImperialScorePercentileOtherGalaxies.getMap();
}

// ----------------------------------------------------------------------

inline std::map<std::pair<std::string, std::string>, int> const & GuildObject::getGcwGroupImperialScorePercentileOtherGalaxies() const
{
	return m_gcwGroupImperialScorePercentileOtherGalaxies.getMap();
}

// ======================================================================

#endif	// _INCLUDED_GuildObject_H
