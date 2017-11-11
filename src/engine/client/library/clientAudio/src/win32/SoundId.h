// ============================================================================
//
// SoundId.h
// Copyright Sony Online Entertainment
//
// ============================================================================

#ifndef INCLUDED_SoundId_H
#define INCLUDED_SoundId_H

#include "../../../../../../engine/shared/library/sharedFoundation/include/public/sharedFoundation/TemporaryCrcString.h"

// ============================================================================
//
// SoundId
//
// ============================================================================

//-----------------------------------------------------------------------------
class SoundId
{
public:

	static void install();
	static void remove();
	static SoundId const & getInvalid();

	SoundId();
	SoundId(SoundId const &rhs);
	explicit SoundId(int const id);
	SoundId(int const id, char const *path);

	SoundId &operator =(SoundId const &rhs);

	int                       getId() const;
	TemporaryCrcString const &getPath() const;
	void                      invalidate();
	bool                      isValid() const;

	// The less than (<) operator must be overloaded to be used in a map

	bool operator <(SoundId const &rhs) const;

private:

	int                m_id;
	TemporaryCrcString m_path;
};

//-----------------------------------------------------------------------------
inline bool SoundId::operator <(SoundId const &rhs) const
{
	return (m_id < rhs.m_id);
}

// ============================================================================

#endif // INCLUDED_SoundId_H
