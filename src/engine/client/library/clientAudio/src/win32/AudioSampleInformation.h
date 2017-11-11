// ============================================================================
//
// AudioSampleInformation.h
// Copyright Sony Online Entertainment
//
// ============================================================================

#ifndef INCLUDED_AudioSampleInformation_H
#define INCLUDED_AudioSampleInformation_H

#include <string>

//-----------------------------------------------------------------------------
class AudioSampleInformation
{
public:

	AudioSampleInformation();

	int         m_bits;                   // Contains the bit depth of the sound data (usually 4, 8, or 16).
	int         m_blockSize;              // Contains the IMA ADPCM block size.
	int         m_channels;               // Contains the number of channels in the wave file (1 is mono, 2 is stereo).
	int         m_dataLength;             // Contains the length of the sound data in the wave file.
	int         m_format;
	float       m_time;                   // Length of the sample in seconds
	int         m_rate;                   // Contains the playback rate of the wave file.
	int         m_samples;                // Contains the total number of samples in the wave file (used with ADPCM).
	std::string m_type;                   // Text description of the file type.
};

// ============================================================================

#endif // INCLUDED_AudioSampleInformation_H
