// ======================================================================
//
// CombatSpecialMoveVisualData.cpp
// Copyright 2002 Sony Online Entertainment, Inc.
// All Rights Reserved.
//
// ======================================================================

#include "clientGame/FirstClientGame.h"
#include "clientGame/CombatSpecialMoveVisualData.h"

#include "sharedDebug/InstallTimer.h"
#include "sharedFile/Iff.h"
#include "sharedFoundation/ExitChain.h"
#include "sharedFoundation/PersistentCrcString.h"
#include "sharedFoundation/PointerDeleter.h"

#include <algorithm>
#include <vector>

// ======================================================================

namespace CombatSpecialMoveVisualDataNamespace
{

	const Tag TAG_CSMV = TAG(C,S,M,V);
	const Tag TAG_MOVE = TAG(M,O,V,E);

	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

	class MoveData
	{
	public:

		MoveData(int effectId, const char *appearanceName, float timeToLive);

		int              getEffectId() const;
		const CrcString &getAppearanceName() const;
		float            getTimeToLive() const;

	private:

		// Disabled.
		MoveData();
		MoveData(const MoveData &moveData);    //lint -esym(754, MoveData::MoveData) // local structure member not referenced. // Defensive hiding.
		MoveData &operator =(const MoveData&);

	private:

		int                  m_effectId;
		PersistentCrcString  m_appearanceName;
		float                m_timeToLive;

	};

	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

	struct MoveDataLessIdComparator
	{
		bool operator ()(const MoveData *lhs, const MoveData *rhs) const;
		bool operator ()(const int       lhs, const MoveData *rhs) const;
		bool operator ()(const MoveData *lhs, int             rhs) const;
	};

	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

	typedef std::vector<MoveData*>  MoveDataVector;

	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

	void remove();
	void load_0000(Iff &iff);

	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

	bool            s_installed;
	MoveDataVector  s_moveDataVector;

}

using namespace CombatSpecialMoveVisualDataNamespace;

// ======================================================================
// class CombatSpecialMoveVisualDataNamespace::MoveData
// ======================================================================

CombatSpecialMoveVisualDataNamespace::MoveData::MoveData(int effectId, const char *appearanceName, float timeToLive) :
	m_effectId(effectId),
	m_appearanceName(appearanceName, true),
	m_timeToLive(timeToLive)
{
}

// ----------------------------------------------------------------------

inline int CombatSpecialMoveVisualDataNamespace::MoveData::getEffectId() const
{
	return m_effectId;
}

// ----------------------------------------------------------------------

inline const CrcString &CombatSpecialMoveVisualDataNamespace::MoveData::getAppearanceName() const
{
	return m_appearanceName;
}

// ----------------------------------------------------------------------

inline float CombatSpecialMoveVisualDataNamespace::MoveData::getTimeToLive() const
{
	return m_timeToLive;
}

// ======================================================================
// class CombatSpecialMoveVisualDataNamespace::MoveDataLessIdComparator
// ======================================================================

inline bool CombatSpecialMoveVisualDataNamespace::MoveDataLessIdComparator::operator ()(const MoveData *lhs, const MoveData *rhs) const
{
	NOT_NULL(lhs);
	NOT_NULL(rhs);

	return lhs->getEffectId() < rhs->getEffectId();
}

// ----------------------------------------------------------------------

inline bool CombatSpecialMoveVisualDataNamespace::MoveDataLessIdComparator::operator ()(int lhs, const MoveData *rhs) const
{
	NOT_NULL(rhs);

	return lhs < rhs->getEffectId();
}

// ----------------------------------------------------------------------

inline bool CombatSpecialMoveVisualDataNamespace::MoveDataLessIdComparator::operator ()(const MoveData *lhs, int rhs) const
{
	NOT_NULL(lhs);

	return lhs->getEffectId() < rhs;
}

// ======================================================================
// namespace CombatSpecialMoveVisualDataNamespace functions
// ======================================================================

void CombatSpecialMoveVisualDataNamespace::remove()
{
	DEBUG_FATAL(!s_installed, ("CombatSpecialMoveVisualData not installed."));

	//-- Cleanup the container of MoveData entries.
	std::for_each(s_moveDataVector.begin(), s_moveDataVector.end(), PointerDeleter());

	//-- Use Scott Meyer's trick to shrink wrap the size of s_moveDataVector.
	s_moveDataVector.clear();
	MoveDataVector().swap(s_moveDataVector);

	s_installed = false;
}

// ----------------------------------------------------------------------

void CombatSpecialMoveVisualDataNamespace::load_0000(Iff &iff)
{
	iff.enterForm(TAG_0000);

		//-- Add all entries to the list.
		while (!iff.atEndOfForm())
		{
			// Read data.
			char buffer[2 * MAX_PATH];

			iff.enterChunk(TAG_MOVE);

				const int clientEffectId = static_cast<int>(iff.read_uint8());
				iff.read_string(buffer, sizeof(buffer) - 1);
				const float timeToLive = iff.read_float();

				// Add new MoveData entry.
				s_moveDataVector.push_back(new MoveData(clientEffectId, buffer, timeToLive));

			iff.exitChunk(TAG_MOVE);
		}

	iff.exitForm(TAG_0000);

	//-- Sort entries by client effect id.
	std::sort(s_moveDataVector.begin(), s_moveDataVector.end(), MoveDataLessIdComparator());

#ifdef _DEBUG
	//-- Check for duplicate entries.
	int lastEffectId = -1;

	const MoveDataVector::iterator endIt = s_moveDataVector.end();
	for (MoveDataVector::iterator it = s_moveDataVector.begin(); it != endIt; ++it)
	{
		NOT_NULL(*it);

		const int newEffectId = (*it)->getEffectId();
		DEBUG_WARNING(newEffectId == lastEffectId, ("CombatSpecialMoveVisualDataNamespace: multiple combat special move effect entries for effect id [%d], only one will be used.", newEffectId));

		lastEffectId = newEffectId;
	}
#endif
}

// ======================================================================
// class CombatSpecialMoveVisualData public static member functions
// ======================================================================

void CombatSpecialMoveVisualData::install(const char *filename)
{
	InstallTimer const installTimer("CombatSpecialMoveVisualData::install");

	DEBUG_FATAL(s_installed, ("CombatSpecialMoveVisualData already installed."));

	//-- Load the data.
	Iff   iff;
	bool  exitingCleanly = true;

	const bool openSuccess = iff.open(filename, true);
	if (!openSuccess)
		DEBUG_WARNING(true, ("CombatSpecialMoveVisualData: failed to load [%s], no combat special move effects will display.", filename));
	else
	{
		iff.enterForm(TAG_CSMV);

			const Tag version = iff.getCurrentName();
			switch (version)
			{
				case TAG_0000:
					load_0000(iff);
					break;

				default:
					{
						char buffer[5];

						ConvertTagToString(version, buffer);
						DEBUG_WARNING(true, ("CombatSpecialMoveVisualData: unsupported data version [%s], no combat special move effects will display.", buffer));

						iff.exitForm(TAG_CSMV, true);
						exitingCleanly = false;
					}
			}

		iff.exitForm(TAG_CSMV, !exitingCleanly);
	}

	s_installed = true;
	ExitChain::add(remove, "CombatSpecialMoveVisualData");
}

// ----------------------------------------------------------------------

bool CombatSpecialMoveVisualData::lookupVisualData(int effectId, const CrcString *&appearanceFilename, float &timeToLive)
{
	DEBUG_FATAL(!s_installed, ("CombatSpecialMoveVisualData not installed."));

	//-- Find the entry.
	std::pair<MoveDataVector::iterator, MoveDataVector::iterator> result = std::equal_range(s_moveDataVector.begin(), s_moveDataVector.end(), effectId, MoveDataLessIdComparator());
	if (result.first == result.second)
	{
		// Entry for specified client id is not found.
		appearanceFilename = 0;
		return false;
	}

	//-- Return the specified data.
	const MoveData *const moveData = *(result.first);
	NOT_NULL(moveData);

	appearanceFilename = &(moveData->getAppearanceName());
	timeToLive         =   moveData->getTimeToLive();
	return true;
}

// ======================================================================
