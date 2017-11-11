// ======================================================================
//
// PlaySkeletalAnimationActionTemplate.h
// Copyright 2001 Sony Online Entertainment Inc.
// All Rights Reserved.
// 
// ======================================================================

#ifndef INCLUDE_PlaySkeletalAnimationActionTemplate_H
#define INCLUDE_PlaySkeletalAnimationActionTemplate_H

// ======================================================================

#include "clientAnimation/PlaybackActionTemplate.h"
#include "../../../../../../engine/shared/library/sharedFoundation/include/public/sharedFoundation/Tag.h"

class CrcLowerString;
class Iff;
class Object;

// ======================================================================

class PlaySkeletalAnimationActionTemplate: public PlaybackActionTemplate
{
public:

	static void install();

public:
	
	PlaySkeletalAnimationActionTemplate(int actorIndex, Tag animationNameVariable, bool enablePostureChange, Tag newServerPostureVariable, Tag specialAttackEffectVariable);

	virtual PlaybackAction *createPlaybackAction() const;

	int                     getActorIndex() const;
	Tag                     getActionNameVariable() const;

	bool                    doPostureChange() const;
	Tag                     getPostureVariable() const;

	Tag                     getSpecialAttackEffectVariable() const;

	bool                    doFaceTracking() const;
	int                     getFaceTrackTargetActorIndex() const;

	bool                    getStopAnimationOnAbort() const;

private:

	static void                    remove();
	static PlaybackActionTemplate *create(Iff &iff);

private:

	explicit PlaySkeletalAnimationActionTemplate(Iff &iff);
	virtual ~PlaySkeletalAnimationActionTemplate();

	void     load_0000(Iff &iff);
	void     load_0001(Iff &iff);
	void     load_0002(Iff &iff);
	void     load_0003(Iff &iff);

	// Disabled.
	PlaySkeletalAnimationActionTemplate();
	PlaySkeletalAnimationActionTemplate(const PlaySkeletalAnimationActionTemplate&);
	PlaySkeletalAnimationActionTemplate &operator =(const PlaySkeletalAnimationActionTemplate&);

private:

	static bool        ms_installed;

private:

	int   m_actorIndex;
	Tag   m_animationNameVariable;

	bool  m_postureChangeEnabled;
	Tag   m_newServerPostureVariable;

	Tag   m_specialAttackEffectVariable;

	bool  m_faceTrackingEnabled;
	int   m_faceTrackTargetActorIndex;

	bool  m_stopAnimationOnAbort;

};

// ======================================================================

inline int PlaySkeletalAnimationActionTemplate::getActorIndex() const
{
	return m_actorIndex;
}

// ----------------------------------------------------------------------

inline Tag PlaySkeletalAnimationActionTemplate::getActionNameVariable() const
{
	return m_animationNameVariable;
}

// ----------------------------------------------------------------------

inline bool PlaySkeletalAnimationActionTemplate::doPostureChange() const
{
	return m_postureChangeEnabled;
}

// ----------------------------------------------------------------------

inline Tag PlaySkeletalAnimationActionTemplate::getPostureVariable() const
{
	return m_newServerPostureVariable;
}

// ----------------------------------------------------------------------

inline Tag PlaySkeletalAnimationActionTemplate::getSpecialAttackEffectVariable() const
{
	return m_specialAttackEffectVariable;
}

// ----------------------------------------------------------------------

inline bool PlaySkeletalAnimationActionTemplate::doFaceTracking() const
{
	return m_faceTrackingEnabled;
}

// ----------------------------------------------------------------------

inline int PlaySkeletalAnimationActionTemplate::getFaceTrackTargetActorIndex() const
{
	return m_faceTrackTargetActorIndex;
}

// ----------------------------------------------------------------------

inline bool PlaySkeletalAnimationActionTemplate::getStopAnimationOnAbort() const
{
	return m_stopAnimationOnAbort;
}

// ======================================================================

#endif
