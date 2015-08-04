// ============================================================================
//
// Sample2d.cpp
// Copyright Sony Online Entertainment
//
// ============================================================================

#include "clientAudio/FirstClientAudio.h"
#include "clientAudio/Sample2d.h"

// ============================================================================
//
// Sample2d
//
// ============================================================================

//-----------------------------------------------------------------------------
Sample2d::Sample2d()
 : m_path(NULL)
 , m_sample(NULL)
 , m_sound(NULL)
 , m_status(Audio::PS_notStarted)
{
}

//-----------------------------------------------------------------------------
Sample2d::Sample2d(Sample2d const &sample2d)
 : m_path(NULL)
 , m_sample(sample2d.m_sample)
 , m_sound(sample2d.m_sound)
 , m_status(sample2d.m_status)
{
	setPath(sample2d.getPath()->getString());
}

//-----------------------------------------------------------------------------
Sample2d & Sample2d::operator =(Sample2d const &rhs)
{
	if (this != &rhs)
	{
		setPath(rhs.getPath()->getString());

		m_sample = rhs.m_sample;
		m_sound = rhs.m_sound;
		m_status = rhs.m_status;
	}

	return *this;
}

//-----------------------------------------------------------------------------
Sample2d::~Sample2d()
{
	if (m_path != NULL)
	{
		Audio::decreaseReferenceCount(*m_path);
	}

	m_path = NULL;
	m_sample = NULL;
	m_sound = NULL;
	m_status = Audio::PS_notStarted;
}

//-----------------------------------------------------------------------------
void Sample2d::setPath(char const *path)
{
	if (m_path != NULL)
	{
		Audio::decreaseReferenceCount(*m_path);
	}

	bool const cacheSample = false;

	m_path = Audio::increaseReferenceCount(path, cacheSample);
}

//-----------------------------------------------------------------------------
CrcString const * const Sample2d::getPath() const
{
	NOT_NULL(m_path);

	return m_path;
}

// ============================================================================
