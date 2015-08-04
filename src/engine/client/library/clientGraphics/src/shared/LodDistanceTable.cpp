// ======================================================================
//
// LodDistanceTable.cpp
// Copyright 2002 Sony Online Entertainment, Inc.
// All Rights Reserved.
//
// ======================================================================

#include "clientGraphics/FirstClientGraphics.h"
#include "clientGraphics/LodDistanceTable.h"

#include "sharedFile/Iff.h"

#include <vector>

// ======================================================================

const Tag TAG_LDTB = TAG(L,D,T,B);

// ======================================================================

struct LodDistanceTable::Level
{
public:

	float  m_minDistanceSquared; // Min distance squared.
	float  m_maxDistanceSquared; // Max distance squared.

};

// ======================================================================
// class LodDistanceTable: public member functions
// ======================================================================

LodDistanceTable::LodDistanceTable(Iff &iff) :
	m_levels(0),
	m_levelCount(0)
{
	iff.enterForm(TAG_LDTB);
		
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
					DEBUG_FATAL(true, ("unsupported LodDistanceTable version [%s].", buffer));
				}
		}

	iff.exitForm(TAG_LDTB);
}

// ----------------------------------------------------------------------

LodDistanceTable::~LodDistanceTable()
{
	delete m_levels;
}

// ----------------------------------------------------------------------

int LodDistanceTable::getDetailLevel(float distanceFromCameraSquared, int currentDetailLevel, float lodBias) const
{
	NOT_NULL(m_levels);

	int          index      = std::max(0, std::min(currentDetailLevel, m_levelCount - 1));
	const Level &startLevel = (*m_levels)[static_cast<LevelVector::size_type>(index)];

	if (distanceFromCameraSquared < startLevel.m_minDistanceSquared * lodBias)
	{
		//-- Find the minimum index LOD (highest detail) appropriate for the given camera distance.
		for (; index >= 0; --index)
		{
			const Level &level = (*m_levels)[static_cast<LevelVector::size_type>(index)];	
			if (distanceFromCameraSquared > level.m_minDistanceSquared * lodBias)
			{
				// Found it.
				return index;
			}
		}

		//-- Camera is closer than the distance of the minimum level index, so return it.
		return 0;
	}
	else if (distanceFromCameraSquared > startLevel.m_maxDistanceSquared * lodBias)
	{
		//-- Find the maximum index LOD (lowest detail) appropriate for the given camera distance.
		for (; index < m_levelCount; ++index)
		{
			const Level &level = (*m_levels)[static_cast<LevelVector::size_type>(index)];	
			if (distanceFromCameraSquared < level.m_maxDistanceSquared * lodBias)
			{
				// Found it.
				return index;
			}
		}

		//-- Camera is further out than the distance of the minimum level index, so return it.
		return m_levelCount - 1;
	}

	return index;
}

// ----------------------------------------------------------------------

void LodDistanceTable::write(Iff &iff)
{
	NOT_NULL(m_levels);

	iff.insertForm(TAG_LDTB);
		iff.insertForm(TAG_0000);
			
			iff.insertChunk(TAG_INFO);

				//-- Write # detail levels.
				iff.insertChunkData(static_cast<int16>(m_levelCount));

				//-- Write each detail level's data.
				for (int i = 0; i < m_levelCount; ++i)
				{
					Level &level = (*m_levels)[static_cast<LevelVector::size_type>(i)];

					iff.insertChunkData(sqrt(level.m_minDistanceSquared));
					iff.insertChunkData(sqrt(level.m_maxDistanceSquared));
				}

			iff.exitChunk(TAG_INFO);

		iff.exitForm(TAG_0000);
	iff.exitForm(TAG_LDTB);
}

// ======================================================================
// class LodDistanceTable: private member functions
// ======================================================================

void LodDistanceTable::load_0000(Iff &iff)
{
	iff.enterForm(TAG_0000);

		iff.enterChunk(TAG_INFO);

			//-- Retrieve # detail levels.
			m_levelCount = static_cast<int>(iff.read_int16());
			DEBUG_WARNING(m_levelCount < 1, ("data issue: LodDistanceTable: less than 1 detail level [%d].", m_levelCount));
			if (m_levelCount < 0)
				m_levelCount = 0;

			//-- Create Level data storage.
			m_levels = new LevelVector(static_cast<LevelVector::size_type>(m_levelCount));

			for (int i = 0; i < m_levelCount; ++i)
			{
				Level &level = (*m_levels)[static_cast<LevelVector::size_type>(i)];

				const float minDistance = iff.read_float();
				level.m_minDistanceSquared = minDistance * minDistance;

				const float maxDistance = iff.read_float();
				level.m_maxDistanceSquared = maxDistance * maxDistance;

				DEBUG_FATAL(level.m_minDistanceSquared > level.m_maxDistanceSquared, ("LodDistanceTable: bad data, detail level [%d], min distance [%g] > max distance [%g].", i, level.m_minDistanceSquared, level.m_maxDistanceSquared));
			}

		iff.exitChunk(TAG_INFO);

	iff.exitForm(TAG_0000);
}

// ======================================================================
