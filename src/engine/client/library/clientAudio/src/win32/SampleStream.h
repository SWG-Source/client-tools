// ============================================================================
//
// SampleStream.h
// Copyright Sony Online Entertainment
//
// ============================================================================

#ifndef INCLUDED_SampleStream_H
#define INCLUDED_SampleStream_H

#include "clientAudio/Audio.h"
#include "../../../../../../engine/shared/library/sharedFoundation/include/public/sharedFoundation/CrcString.h"

//-----------------------------------------------------------------------------
class SampleStream
{
public:

	SampleStream();
	SampleStream(SampleStream const &sampleStream);
	SampleStream & operator =(SampleStream const &rhs);
	~SampleStream();

	void                    setPath(char const *path);
	CrcString const * const getPath() const;

	HSTREAM               m_stream;
	Sound2 *              m_sound;
	Audio::PlayBackStatus m_status;

private:

	CrcString const *     m_path;
};

// ============================================================================

#endif // INCLUDED_SampleStream_H
