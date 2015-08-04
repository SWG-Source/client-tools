// ======================================================================
//
// SequenceObject.cpp
// Copyright 2001 Sony Online Entertainment Inc.
// All Rights Reserved.
// 
// ======================================================================

#include "clientGame/FirstClientGame.h"
#include "clientGame/SequenceObject.h"

#include "clientGame/ClientWorld.h"
#include "sharedFoundation/PointerDeleter.h"
#include "sharedFoundation/Watcher.h"
#include "sharedObject/AlterResult.h"
#include "sharedObject/World.h"

#include <algorithm>
#include <vector>

// ======================================================================

namespace
{
	bool  ms_logSequenceObject;
}

// ======================================================================

class SequenceObject::ObjectData
{
public:

	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

	struct LessPhaseComparator
	{
		bool operator ()(const ObjectData *lhs, const ObjectData *rhs) const;
		bool operator ()(const ObjectData *lhs, int rhs) const;
		bool operator ()(int lhs, const ObjectData *rhs) const;
	};

	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

public:

	ObjectData(Object *object, int phase);

	Object *getObject() const;
	int     getPhase() const;

private:

	// disabled
	ObjectData();
	ObjectData(const ObjectData&);              //lint -esym(754, ObjectData::ObjectData) // defensive hiding
	ObjectData& operator= (const ObjectData&);  //lint -esym(754, ObjectData::operator=)  // defensive hiding

private:

	Watcher<Object>  m_object;
	int              m_phase;

};

// ======================================================================
// struct SequenceObject::ObjectData
// ======================================================================

SequenceObject::ObjectData::ObjectData(Object *object, int phase)
:	m_object(object),
	m_phase(phase)
{
}

// ----------------------------------------------------------------------

inline Object *SequenceObject::ObjectData::getObject() const
{
	return m_object.getPointer();
}

// ----------------------------------------------------------------------

inline int SequenceObject::ObjectData::getPhase() const
{
	return m_phase;
}

// ======================================================================
// struct SequenceObject::ObjectData::LessPhaseComparator
// ======================================================================

inline bool SequenceObject::ObjectData::LessPhaseComparator::operator ()(const ObjectData *lhs, const ObjectData *rhs) const
{
	NOT_NULL(lhs);
	NOT_NULL(rhs);

	return lhs->getPhase() < rhs->getPhase();
}

// ----------------------------------------------------------------------

inline bool SequenceObject::ObjectData::LessPhaseComparator::operator ()(const ObjectData *lhs, int rhs) const
{
	NOT_NULL(lhs);

	return lhs->getPhase() < rhs;
}

// ----------------------------------------------------------------------

inline bool SequenceObject::ObjectData::LessPhaseComparator::operator ()(int lhs, const ObjectData *rhs) const
{
	NOT_NULL(rhs);

	return lhs < rhs->getPhase();
}

// ======================================================================

SequenceObject::SequenceObject(const ObjectVector &objects, const IntVector &phases)
:	Object(),
	m_objectDataVector(new ObjectDataVector),
	m_phaseBeginIndex(0),
	m_phaseEndIndex(0)
{
	DEBUG_FATAL(objects.size() != phases.size(), ("# objects provided (%u) not equal to # phases provided (%u)\n", objects.size(), phases.size()));
	DEBUG_FATAL(objects.empty(), ("no objects to sequence --- this is an expensive waste.\n"));

	//-- create the object data (phase/watcher info)
	const size_t objectCount = objects.size();

	m_objectDataVector->reserve(objectCount);
	for (size_t i = 0; i < objectCount; ++i)
		m_objectDataVector->push_back(new ObjectData(objects[i], phases[i]));

	//-- sort object data by phase in ascending order
	std::sort(m_objectDataVector->begin(), m_objectDataVector->end(), ObjectData::LessPhaseComparator());

	//-- add the first phase's objects to world
	findPhaseRange(m_phaseBeginIndex, m_phaseEndIndex);
	queuePhaseObjectsInWorld();

	//-- Ensure sequence object goes on the intangible list.
	addNotification(ClientWorld::getIntangibleNotification());

#ifdef _DEBUG
	if (m_phaseEndIndex < static_cast<int>(m_objectDataVector->size()))
	{
		DEBUG_REPORT_LOG(ms_logSequenceObject, ("SEQOBJ: launching phase [%d].\n", (*m_objectDataVector)[static_cast<ObjectDataVector::size_type>(m_phaseBeginIndex)]->getPhase()));
	}
#endif
}

// ----------------------------------------------------------------------

SequenceObject::~SequenceObject()
{
	// all objects in the current phase are in the world and will be destroyed;
	// however, we should destroy all the rest of the objects

	const int objectDataCount = static_cast<int>(m_objectDataVector->size());
	if (m_phaseEndIndex < objectDataCount)
	{
		// delete all objects in the range [m_phaseEndIndex .. objectDataCount).
		for (int i = m_phaseEndIndex; i < objectDataCount; ++i)
		{
			delete (*m_objectDataVector)[static_cast<size_t>(i)]->getObject();
		}
	}

	std::for_each(m_objectDataVector->begin(), m_objectDataVector->end(), PointerDeleter());
	delete m_objectDataVector;
}

// ----------------------------------------------------------------------

float SequenceObject::alter(float time)
{
	const float result = Object::alter (time);

	if (!allPhaseObjectsDead())
		return result;

	//-- time to move on to new range, all objects in previous phase are now dead.
	if (m_phaseEndIndex >= static_cast<int>(m_objectDataVector->size()))
	{
		//-- the sequence object is dead
		kill();
		DEBUG_REPORT_LOG(ms_logSequenceObject, ("SEQOBJ: dying, all children dead.\n"));

		return AlterResult::cms_kill;
	}

	//-- reset the phase range
	m_phaseBeginIndex = m_phaseEndIndex;
	findPhaseRange(m_phaseBeginIndex, m_phaseEndIndex);

	//-- queue all objects in range for addition to world
	queuePhaseObjectsInWorld();

#ifdef _DEBUG
	if (m_phaseEndIndex < static_cast<int>(m_objectDataVector->size()))
	{
		DEBUG_REPORT_LOG(ms_logSequenceObject, ("SEQOBJ: launching phase [%d].\n", (*m_objectDataVector)[static_cast<ObjectDataVector::size_type>(m_phaseBeginIndex)]->getPhase()));
	}
#endif

	return result;
}

// ======================================================================

void SequenceObject::findPhaseRange(int phaseBeginIndex, int &phaseEndIndex)
{
	//-- ensure we haven't run through all objects yet
	const int objectDataCount = static_cast<int>(m_objectDataVector->size());
	DEBUG_FATAL(phaseBeginIndex >= objectDataCount, ("findPhaseRange() called with bad phaseBeginIndex (%d/%d)", phaseBeginIndex, objectDataCount));

	//-- get current phase
	const int currentPhase = (*m_objectDataVector)[static_cast<size_t>(phaseBeginIndex)]->getPhase();

	//-- find all object in current phase
	int index = phaseBeginIndex;
	for ( ; (index < objectDataCount) && ((*m_objectDataVector)[static_cast<size_t>(index)]->getPhase() == currentPhase); ++index)
	{
	}

	//-- return end of range (exclusive)
	phaseEndIndex = index;
}

// ----------------------------------------------------------------------

void SequenceObject::queuePhaseObjectsInWorld()
{
	VALIDATE_RANGE_INCLUSIVE_EXCLUSIVE(0, m_phaseBeginIndex, static_cast<int>(m_objectDataVector->size()));
	VALIDATE_RANGE_INCLUSIVE_INCLUSIVE(0, m_phaseEndIndex, static_cast<int>(m_objectDataVector->size()));

	//-- ensure we haven't run through all objects yet
	for (int index = m_phaseBeginIndex; index < m_phaseEndIndex; ++index)
	{
		Object *object = (*m_objectDataVector)[static_cast<size_t>(index)]->getObject();
		if (object)
		{
			ClientWorld::queueObject(object); //lint !e1060 // Function is protected // No, itis exposed through World::queueObject using a public using clause.
		}
	}
}

// ----------------------------------------------------------------------

bool SequenceObject::allPhaseObjectsDead()
{
	int liveCount = 0;

	//-- count the number of objects in the phase that are alive
	for (int index = m_phaseBeginIndex; index < m_phaseEndIndex; ++index)
	{
		if ((*m_objectDataVector)[static_cast<size_t>(index)]->getObject())
			++liveCount;
	}

	//-- all phase objects are dead if none are alive
	return (liveCount == 0);
}

// ======================================================================
