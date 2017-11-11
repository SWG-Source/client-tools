// ============================================================================
//
// AudioSampleInformation.cpp
// Copyright Sony Online Entertainment
//
// ============================================================================

#include "clientAudio/FirstClientAudio.h"
#include "clientAudio/AudioSampleInformation.h"

//-----------------------------------------------------------------------------
AudioSampleInformation::AudioSampleInformation()
 : m_bits(0)
 , m_blockSize(0)
 , m_channels(0)
 , m_dataLength(0)
 , m_format(0)
 , m_time(0)
 , m_rate(0)
 , m_samples(0)
 , m_type("")
{
}

// ============================================================================
