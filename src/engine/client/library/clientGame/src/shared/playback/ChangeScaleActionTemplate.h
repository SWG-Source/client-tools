// ======================================================================
//
// ChangeScaleActionTemplate.h
// Copyright 2002 Sony Online Entertainment, Inc.
// All Rights Reserved.
//
// ======================================================================

#ifndef INCLUDED_ChangeScaleActionTemplate_H
#define INCLUDED_ChangeScaleActionTemplate_H

// ======================================================================

#include "clientAnimation/PlaybackActionTemplate.h"
#include "../../../../../../engine/shared/library/sharedFoundation/include/public/sharedFoundation/Tag.h"

class Iff;

// ======================================================================

class ChangeScaleActionTemplate: public PlaybackActionTemplate
{
public:

	enum BlendMode
	{
		BM_objectToTarget,  // blend from the object's authoratative scale to the target scale.
		BM_targetToObject   // blend from the target scale to the object's authoratative scale.
	};

public:

	static void install();

public:

	virtual PlaybackAction *createPlaybackAction() const;

	int        getActorIndex() const;
	BlendMode  getBlendMode() const;
	Tag        getTargetScaleVariable() const;
	Tag        getTimeDurationVariable() const;

private:

	static void                    remove();
	static PlaybackActionTemplate *create(Iff &iff);

private:

	ChangeScaleActionTemplate(Iff &iff);

	void load_0000(Iff &iff);

	// Disabled.
	ChangeScaleActionTemplate();
	ChangeScaleActionTemplate(const ChangeScaleActionTemplate&);
	ChangeScaleActionTemplate &operator =(const ChangeScaleActionTemplate&);

private:

	static bool  ms_installed;

private:

	/// Actor index for the object to be yawed.
	int        m_actorIndex;
	BlendMode  m_blendMode;
	Tag        m_targetScaleVariable;
	Tag        m_timeDurationVariable;
};

// ======================================================================

inline int ChangeScaleActionTemplate::getActorIndex() const
{
	return m_actorIndex;
}

// ----------------------------------------------------------------------

inline ChangeScaleActionTemplate::BlendMode ChangeScaleActionTemplate::getBlendMode() const
{
	return m_blendMode;
}

// ----------------------------------------------------------------------

inline Tag ChangeScaleActionTemplate::getTargetScaleVariable() const
{
	return m_targetScaleVariable;
}

// ----------------------------------------------------------------------

inline Tag ChangeScaleActionTemplate::getTimeDurationVariable() const
{
	return m_timeDurationVariable;
}

// ======================================================================

#endif
