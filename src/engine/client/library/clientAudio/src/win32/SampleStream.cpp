// ============================================================================
//
// SampleStream.cpp
// Copyright Sony Online Entertainment
//
// ============================================================================

#include "clientAudio/FirstClientAudio.h"
#include "clientAudio/SampleStream.h"

// ============================================================================
//
// SampleStream
//
// ============================================================================

//-----------------------------------------------------------------------------
SampleStream::SampleStream()
 : m_path(NULL)
 , m_stream(NULL)
 , m_sound(NULL)
 , m_status(Audio::PS_notStarted)
{
}

//-----------------------------------------------------------------------------
SampleStream::SampleStream(SampleStream const &sampleStream)
 : m_path(NULL)
 , m_stream(sampleStream.m_stream)
 , m_sound(sampleStream.m_sound)
 , m_status(sampleStream.m_status)
{
	setPath(sampleStream.getPath()->getString());
}

//-----------------------------------------------------------------------------
SampleStream & SampleStream::operator =(SampleStream const &rhs)
{
	if (this != &rhs)
	{
		setPath(rhs.getPath()->getString());

		m_stream = rhs.m_stream;
		m_sound = rhs.m_sound;
		m_status = rhs.m_status;
	}

	return *this;
}

//-----------------------------------------------------------------------------
SampleStream::~SampleStream()
{
	if (m_path != NULL)
	{
		Audio::decreaseReferenceCount(*m_path);
	}

	m_path = NULL;
	m_stream = NULL;
	m_sound = NULL;
	m_status = Audio::PS_notStarted;
}

//-----------------------------------------------------------------------------
void SampleStream::setPath(char const *path)
{
	if (m_path != NULL)
	{
		Audio::decreaseReferenceCount(*m_path);
	}

	bool const cacheSample = false;

	m_path = Audio::increaseReferenceCount(path, cacheSample);
}

//-----------------------------------------------------------------------------
CrcString const * const SampleStream::getPath() const
{
	NOT_NULL(m_path);

	return m_path;
}

// ============================================================================
