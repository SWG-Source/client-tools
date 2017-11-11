// ============================================================================
//
// SoundTemplate.cpp
// Copyright Sony Online Entertainment
//
// ============================================================================

#include "clientAudio/FirstClientAudio.h"
#include "clientAudio/SoundTemplate.h"

#include <vector>

// ============================================================================
//
// SoundTemplate
//
// ============================================================================

//-----------------------------------------------------------------------------
SoundTemplate::SoundTemplate()
 : m_samplePathList(NULL)
 , m_referenceCount(0)
 , m_crcName()
{
	m_samplePathList = new StringList();
}

//-----------------------------------------------------------------------------
SoundTemplate::SoundTemplate(const char *name)
 : m_samplePathList(NULL)
 , m_referenceCount(0)
 , m_crcName(name, true)
{
	m_samplePathList = new StringList();
}

//-----------------------------------------------------------------------------
SoundTemplate::~SoundTemplate()
{
	clearSampleList();

	delete m_samplePathList;
	m_samplePathList = NULL;
}

//-----------------------------------------------------------------------------
bool SoundTemplate::operator ==(SoundTemplate const &rhs) const
{
	bool result = false;

	if (this == &rhs)
	{
		result = true;
	}
	else
	{
		if ((m_crcName == rhs.m_crcName) &&
		    (*m_samplePathList == *rhs.m_samplePathList))
		{
			result = true;
		}
	}

	return result;
}

//-----------------------------------------------------------------------------
void SoundTemplate::clearSampleList()
{
	// Uncache all the samples

	StringList::const_iterator iterSampleList = m_samplePathList->begin();

	for (; iterSampleList != m_samplePathList->end(); ++iterSampleList)
	{
		CrcString const *path = (*iterSampleList);

		if (path != NULL)
		{
			Audio::decreaseReferenceCount(*path);
		}
	}

	m_samplePathList->clear();
}

// ----------------------------------------------------------------------
int SoundTemplate::getReferenceCount() const
{
	return m_referenceCount;
}

//-----------------------------------------------------------------------------
PersistentCrcString const &SoundTemplate::getCrcName() const
{
	return m_crcName;
}

//-----------------------------------------------------------------------------
char const *SoundTemplate::getName() const
{
	return m_crcName.getString();
}

//-----------------------------------------------------------------------------
bool SoundTemplate::is2d() const
{
	return false;
}

//-----------------------------------------------------------------------------
bool SoundTemplate::is3d() const
{
	return false;
}

//-----------------------------------------------------------------------------
SoundTemplate::StringList const &SoundTemplate::getSampleList() const
{
	return *m_samplePathList;
}

//--------------------------------------------------------------------------
void SoundTemplate::addSample(char const *path, bool const forceCacheSample)
{
	if ((MemoryManager::getLimit() >= 375) ||
	    ((MemoryManager::getLimit() < 260) && (m_samplePathList->size() < 2)) ||
	    ((MemoryManager::getLimit() < 375) && (m_samplePathList->size() < 3)) ||
	    Audio::isToolApplication())
	{
		bool const cacheSample = forceCacheSample || (Audio::getSampleSize(path) <= Audio::getMaxCached2dSampleSize());
		CrcString const *crcString = Audio::increaseReferenceCount(path, cacheSample);

		if (crcString != NULL)
		{
			m_samplePathList->push_back(crcString);
		}
	}
}

//-----------------------------------------------------------------------------
int SoundTemplate::getSampleCount() const
{
	return static_cast<int>(m_samplePathList->size());
}

// ============================================================================
