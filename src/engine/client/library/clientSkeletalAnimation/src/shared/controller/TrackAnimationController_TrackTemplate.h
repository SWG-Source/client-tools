// ======================================================================
//
// TrackAnimationController_TrackTemplate.h
// Copyright 2002 Sony Online Entertainment, Inc.
// All Rights Reserved.
//
// ======================================================================

#ifndef INCLUDED_TrackAnimationController_TrackTemplate_H
#define INCLUDED_TrackAnimationController_TrackTemplate_H

// ======================================================================

#include "clientSkeletalAnimation/TrackAnimationController.h"

// I'm including these only because this is a private inner class only referenced by a couple other files 
// and is broken out soley for source size management.
#include "clientSkeletalAnimation/AnimationTrackId.h"
#include "../../../../../../engine/shared/library/sharedFoundation/include/public/sharedFoundation/CrcLowerString.h"

// ======================================================================

class TrackAnimationController::TrackTemplate
{
public:

	virtual ~TrackTemplate();

	const CrcLowerString    &getTrackName() const;
	const AnimationTrackId  &getTrackId() const;

	virtual bool             hasTrackIndexResolver() const = 0;
	virtual int              getTrackIndexResolverIndex() const = 0;

	virtual int              getPhysicalTrackIndex() const = 0;

protected:

	TrackTemplate(const CrcLowerString &trackName, const AnimationTrackId &trackId);

private:

	// disabled
	TrackTemplate();
	TrackTemplate(const TrackTemplate&);
	TrackTemplate &operator =(const TrackTemplate&); 

private:

	CrcLowerString    m_trackName;
	AnimationTrackId  m_trackId;

};

// ======================================================================

inline const CrcLowerString &TrackAnimationController::TrackTemplate::getTrackName() const
{
	return m_trackName;
}

// ----------------------------------------------------------------------

inline const AnimationTrackId &TrackAnimationController::TrackTemplate::getTrackId() const
{
	return m_trackId;
}

// ======================================================================

#endif
