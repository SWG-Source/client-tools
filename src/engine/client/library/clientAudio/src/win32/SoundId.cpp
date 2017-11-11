// ============================================================================
//
// SoundId.cpp
// Copyright Sony Online Entertainment
//
// ============================================================================

#include "clientAudio/FirstClientAudio.h"
#include "clientAudio/SoundId.h"

#include "clientAudio/Audio.h"

// ============================================================================
//
// SoundId
//
// ============================================================================

namespace SoundIdNamespace
{
	SoundId * cms_invalid = NULL;
	bool ms_installed = false;
}

using namespace SoundIdNamespace;

//----------------------------------------------------------------------

void SoundId::install()
{
	cms_invalid = new SoundId();
	ms_installed = true;
}

//-----------------------------------------------------------------------------

void SoundId::remove()
{
	ms_installed = false;
	delete cms_invalid;
	cms_invalid = NULL;
}

//-----------------------------------------------------------------------------

SoundId const & SoundId::getInvalid()
{
	FATAL(!ms_installed, ("Not installed"));
	return *cms_invalid;
}

//-----------------------------------------------------------------------------

SoundId::SoundId()
 : m_id(0)
 , m_path()
{
}

//-----------------------------------------------------------------------------
SoundId::SoundId(SoundId const &rhs)
 : m_id(rhs.m_id)
 , m_path()
{
//	const char * const str = rhs.m_path.getString();
//	uint32 const crc = rhs.m_path.getCrc();
//	m_path.set(str, crc);
	m_path.set(rhs.m_path.getString(), rhs.m_path.getCrc());
}

//-----------------------------------------------------------------------------
SoundId::SoundId(int const id)
 : m_id(id)
 , m_path()
{
}

//-----------------------------------------------------------------------------
SoundId::SoundId(int const id, char const *path)
 : m_id(id)
 , m_path(path, true)
{
}

//-----------------------------------------------------------------------------
SoundId &SoundId::operator =(SoundId const &rhs)
{
	if (this != &rhs)
	{
		m_id = rhs.m_id;
		m_path.set(rhs.m_path.getString(), rhs.m_path.getCrc());
	}

	return *this;
}

//-----------------------------------------------------------------------------
int SoundId::getId() const
{
	return m_id;
}

//-----------------------------------------------------------------------------
void SoundId::invalidate()
{
	m_id = 0;
}

//-----------------------------------------------------------------------------
bool SoundId::isValid() const
{
	return Audio::isSoundValid(*this);
}

//-----------------------------------------------------------------------------
TemporaryCrcString const &SoundId::getPath() const
{
	return m_path;
}

// ============================================================================
