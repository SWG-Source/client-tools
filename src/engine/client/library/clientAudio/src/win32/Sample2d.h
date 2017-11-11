// ============================================================================
//
// Sample2d.h
// Copyright Sony Online Entertainment
//
// ============================================================================

#ifndef INCLUDED_Sample2d_H
#define INCLUDED_Sample2d_H

#include "clientAudio/Audio.h"
#include "../../../../../../engine/shared/library/sharedFoundation/include/public/sharedFoundation/CrcString.h"

class Sound2d;

//-----------------------------------------------------------------------------
class Sample2d
{
public:

	Sample2d();
	Sample2d(Sample2d const &sample2d);
	Sample2d & operator =(Sample2d const &rhs);
	~Sample2d();

	void                    setPath(char const *path);
	CrcString const * const getPath() const;

	HSAMPLE               m_sample;
	Sound2 *              m_sound;
	Audio::PlayBackStatus m_status;

private:

	CrcString const *     m_path;
};

// ============================================================================

#endif // INCLUDED_Sample2d_H
