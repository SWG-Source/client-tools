// ======================================================================
//
// AnimationTrackId.h
// Copyright 2001 Sony Online Entertainment Inc.
// All Rights Reserved.
// 
// ======================================================================

#ifndef INCLUDED_AnimationTrackId_H
#define INCLUDED_AnimationTrackId_H

// ======================================================================

class AnimationTrackId
{
friend class TrackAnimationController;

public:

	static const AnimationTrackId cms_invalid;

public:

	AnimationTrackId();

	//const CrcLowerString &getTrackName() const;

	bool operator ==(const AnimationTrackId &rhs) const;
	bool operator !=(const AnimationTrackId &rhs) const;

	int  getTrackIdValue() const;

private:

	explicit AnimationTrackId(int trackIdValue);

private:

	int m_trackIdValue;

};

// ======================================================================
/**
 * Construct a track id value that indicates no track whatsoever.
 *
 * The user of this initializer should be expecting to assign a
 * value to this instance prior to performing any activities with
 * the TrackId.  Most operations that accept TrackId instances should
 * be checking for invalid track ids, so it should be the case
 * that usage of such a constructed instance is caught before doing
 * any harm.
 */

inline AnimationTrackId::AnimationTrackId() :
	m_trackIdValue(cms_invalid.getTrackIdValue())
{
}

// ----------------------------------------------------------------------

inline int AnimationTrackId::getTrackIdValue() const
{
	return m_trackIdValue;
}

// ----------------------------------------------------------------------

inline AnimationTrackId::AnimationTrackId(int trackIdValue)
:	m_trackIdValue(trackIdValue)
{
}

// ----------------------------------------------------------------------

inline bool AnimationTrackId::operator ==(const AnimationTrackId &rhs) const
{
	return m_trackIdValue == rhs.m_trackIdValue;
}

// ----------------------------------------------------------------------

inline bool AnimationTrackId::operator !=(const AnimationTrackId &rhs) const
{
	return !(*this == rhs);
}

// ======================================================================

#endif
