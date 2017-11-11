// ============================================================================
//
// SampleCacheEntry.h
// Copyright Sony Online Entertainment
//
// ============================================================================

#ifndef INCLUDED_SampleCacheEntry_H
#define INCLUDED_SampleCacheEntry_H

#include <string>

//-----------------------------------------------------------------------------
class SampleCacheEntry
{
public:

	SampleCacheEntry();

	unsigned char * m_sampleRawData;
	int             m_fileSize;
	float           m_time;
	int             m_referenceCount;

	void        setExtension(char const *path);
	char const *getExtension() const;

private:

	char m_extension[4 + 1];          // Needs to be 4 characters, ex. ".wav"
};

// ============================================================================

#endif // INCLUDED_SampleCacheEntry_H
