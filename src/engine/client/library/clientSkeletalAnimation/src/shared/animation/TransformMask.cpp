// ======================================================================
//
// TransformMask.cpp
// Copyright 2002 Sony Online Entertainment, Inc.
// All Rights Reserved.
//
// ======================================================================

#include "clientSkeletalAnimation/FirstClientSkeletalAnimation.h"
#include "clientSkeletalAnimation/TransformMask.h"

#include "clientSkeletalAnimation/TransformMaskList.h"
#include "clientSkeletalAnimation/TransformNameMap.h"
#include "sharedFile/Iff.h"
#include "sharedFoundation/CrcLowerString.h"
#include "sharedFoundation/PersistentCrcString.h"
#include "sharedFoundation/TemporaryCrcString.h"

#include <algorithm>
#include <set>
#include <vector>

// ======================================================================

const Tag TAG_INGR = TAG(I,N,G,R);
const Tag TAG_XFMS = TAG(X,F,M,S);

// ======================================================================
// class TransformMask: public member functions
// ======================================================================

void TransformMask::fetch() const
{
	DEBUG_FATAL(m_referenceCount < 0, ("TransformMask::fetch() called on deleted instance."));
	++m_referenceCount;
}

// ----------------------------------------------------------------------

void TransformMask::release() const
{
	//-- Decrement reference count.
	--m_referenceCount;

	if (m_referenceCount < 1)
	{
		DEBUG_WARNING(m_referenceCount < 0, ("TransformMask::release() called on deleted instance [ref count = %d].", m_referenceCount));

		//-- Delete this instance, no more references.
		TransformMaskList::stopTracking(*this);
		delete const_cast<TransformMask*>(this);
	}
}

// ----------------------------------------------------------------------

int TransformMask::getReferenceCount() const
{
	return m_referenceCount;
}

// ----------------------------------------------------------------------

const CrcLowerString &TransformMask::getName() const
{
	return *m_name;
}

// ----------------------------------------------------------------------

int TransformMask::getInGroupTransformCount() const
{
	return static_cast<int>(m_inGroupTransformNames->size());
}

// ----------------------------------------------------------------------

CrcString const &TransformMask::getInGroupTransformName(int index) const
{
	VALIDATE_RANGE_INCLUSIVE_EXCLUSIVE(0, index, getInGroupTransformCount());

	// @todo convert set to sorted vector.
	PersistentCrcStringSet::const_iterator it = m_inGroupTransformNames->begin();
	std::advance(it, static_cast<PersistentCrcStringSet::size_type>(index));

	return *it;
}

// ----------------------------------------------------------------------

bool TransformMask::isTransformWithinInGroup(CrcString const &transformName) const
{
	const PersistentCrcStringSet::const_iterator findIt = m_inGroupTransformNames->find(PersistentCrcString(transformName));
	return (findIt != m_inGroupTransformNames->end());
}

// ----------------------------------------------------------------------

void TransformMask::fillTransformPriorityVector(const TransformNameMap &transformNameMap, int inGroupPriority, int outGroupPriority, IntVector &priorityLookupVector) const
{
	//-- Size the return vector to the # entries in the transformNameMap.
	const int transformCount = transformNameMap.getTransformCount();
	priorityLookupVector.resize(static_cast<IntVector::size_type>(transformCount));

	for (int i = 0; i < transformCount; ++i)
	{
		//-- Lookup whether transform is in the "in" group.
		const bool withinInGroup = isTransformWithinInGroup(transformNameMap.getTransformName(i));

		//-- Set transform priority based on "in" group membership.
		priorityLookupVector[static_cast<IntVector::size_type>(i)] = (withinInGroup ? inGroupPriority : outGroupPriority);
	}
}

// ----------------------------------------------------------------------

TransformMask::TransformMask() :
	m_referenceCount(0),
	m_name(new CrcLowerString(CrcLowerString::empty)),
	m_inGroupTransformNames(new PersistentCrcStringSet())
{
}

// ----------------------------------------------------------------------

void TransformMask::addInGroupTransformName(const CrcLowerString &transformName)
{
	IGNORE_RETURN(m_inGroupTransformNames->insert(transformName));
}

// ----------------------------------------------------------------------

void TransformMask::write(Iff &iff) const
{
	iff.insertForm(TAG_XFMS);
		iff.insertForm(TAG_0000);

			iff.insertChunk(TAG_INGR);
				
				//-- Write # transforms in "in" group.
				iff.insertChunkData(static_cast<int16>(m_inGroupTransformNames->size()));

				//-- Write each transform.
				const PersistentCrcStringSet::const_iterator endIt = m_inGroupTransformNames->end();
				for (PersistentCrcStringSet::const_iterator it = m_inGroupTransformNames->begin(); it != endIt; ++it)
					iff.insertChunkString(it->getString());

			iff.exitChunk(TAG_INGR);

		iff.exitForm(TAG_0000);
	iff.exitForm(TAG_XFMS);
}

// ======================================================================
// class TransformMask: private member functions
// ======================================================================

TransformMask::TransformMask(const char *name, Iff &iff) :
	m_referenceCount(0),
	m_name(new CrcLowerString(name)),
	m_inGroupTransformNames(new PersistentCrcStringSet())
{
	iff.enterForm(TAG_XFMS);
	
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
					DEBUG_FATAL(true, ("TransformMask version [%s] unsupported.", buffer));
				}
		}

	iff.exitForm(TAG_XFMS);
}

// ----------------------------------------------------------------------

TransformMask::~TransformMask()
{
	delete m_name;
	delete m_inGroupTransformNames;
}

// ----------------------------------------------------------------------

void TransformMask::load_0000(Iff &iff)
{
	iff.enterForm(TAG_0000);
		iff.enterChunk(TAG_INGR);

			char      buffer[1024];
			const int transformCount = static_cast<int>(iff.read_int16());

			for (int i = 0; i < transformCount; ++i)
			{
				iff.read_string(buffer, sizeof(buffer) - 1);
				IGNORE_RETURN(m_inGroupTransformNames->insert(PersistentCrcString(buffer, true)));
			}

		iff.exitChunk(TAG_INGR);
	iff.exitForm(TAG_0000);
}

// ======================================================================
