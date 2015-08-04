// ============================================================================
//
// SampleCacheEntry.cpp
// Copyright Sony Online Entertainment
//
// ============================================================================

#include "clientAudio/FirstClientAudio.h"
#include "clientAudio/SampleCacheEntry.h"

#include "sharedFile/FileNameUtils.h"

// ============================================================================
//
// SampleCacheEntry
//
// ============================================================================

//-----------------------------------------------------------------------------
SampleCacheEntry::SampleCacheEntry()
 : m_sampleRawData(NULL)
 , m_fileSize(0)
 , m_time(0.0f)
 , m_referenceCount(0)
{
	m_extension[0] = '\0';
}

//-----------------------------------------------------------------------------
void SampleCacheEntry::setExtension(char const *path)
{
	m_extension[0] = '\0';

	if (path != NULL)
	{
		const char *dotPosition = strstr(path, ".");

		if ((dotPosition != NULL) &&
			strlen(dotPosition) == 4)
		{
			m_extension[0] = '.';
			m_extension[1] = *(dotPosition + 1);
			m_extension[2] = *(dotPosition + 2);
			m_extension[3] = *(dotPosition + 3);
			m_extension[4] = '\0';
		}
	}
}

//-----------------------------------------------------------------------------
char const *SampleCacheEntry::getExtension() const
{
	return m_extension;
}

// ============================================================================
