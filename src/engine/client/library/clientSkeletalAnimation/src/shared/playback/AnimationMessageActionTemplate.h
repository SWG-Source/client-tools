// ======================================================================
//
// AnimationMessageActionTemplate.h
// Copyright 2001 Sony Online Entertainment Inc.
// All Rights Reserved.
// 
// ======================================================================

#ifndef INCLUDED_AnimationMessageActionTemplate_H
#define INCLUDED_AnimationMessageActionTemplate_H

// ======================================================================

#include "clientAnimation/PlaybackActionTemplate.h"
#include "../../../../../../engine/shared/library/sharedFoundation/include/public/sharedFoundation/Tag.h"

class Iff;

// ======================================================================

class AnimationMessageActionTemplate: public PlaybackActionTemplate
{
public:

	static void install();

public:

	virtual PlaybackAction  *createPlaybackAction() const;

	const Tag               &getMessageNameVariable() const;
	int                      getActorIndex() const;

private:

	static void                     remove();
	static PlaybackActionTemplate *create(Iff &iff);

private:

	explicit AnimationMessageActionTemplate(Iff &iff);

	void  load_0000(Iff &iff);

	// disabled
	AnimationMessageActionTemplate();
	AnimationMessageActionTemplate(const AnimationMessageActionTemplate&);
	AnimationMessageActionTemplate &operator =(const AnimationMessageActionTemplate&);

private:

	static bool  ms_installed;

private:

	Tag  m_messageNameVariable;
	int  m_actorIndex;

};

// ======================================================================

#endif
