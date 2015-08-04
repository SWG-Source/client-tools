// ======================================================================
//
// AlignToHardpointActionTemplate.h
// Copyright 2002 Sony Online Entertainment, Inc.
// All Rights Reserved.
//
// ======================================================================

#ifndef INCLUDED_AlignToHardpointActionTemplate_H
#define INCLUDED_AlignToHardpointActionTemplate_H

// ======================================================================

#include "clientAnimation/PlaybackActionTemplate.h"
#include "../../../../../../engine/shared/library/sharedFoundation/include/public/sharedFoundation/Tag.h"

class Iff;

// ======================================================================

class AlignToHardpointActionTemplate: public PlaybackActionTemplate
{
public:

	static void install();

public:

	virtual PlaybackAction *createPlaybackAction() const;

	int  getMoveActorIndex() const;
	int  getAlignmentActorIndex() const;
	Tag  getAlignmentHardpointNameVariable() const;

private:

	static void                    remove();
	static PlaybackActionTemplate *create(Iff &iff);

private:

	AlignToHardpointActionTemplate(Iff &iff);

	void load_0000(Iff &iff);

	// Disabled.
	AlignToHardpointActionTemplate();
	AlignToHardpointActionTemplate(const AlignToHardpointActionTemplate&);
	AlignToHardpointActionTemplate &operator =(const AlignToHardpointActionTemplate&);

private:

	static bool  ms_installed;

private:

	/// Actor index for the object to be yawed.
	int  m_moveActorIndex;

	/// Actor index for the target.
	int  m_alignmentActorIndex;

	Tag  m_alignmentHardpointNameVariable;
};

// ======================================================================

inline int AlignToHardpointActionTemplate::getMoveActorIndex() const
{
	return m_moveActorIndex;
}

// ----------------------------------------------------------------------

inline int AlignToHardpointActionTemplate::getAlignmentActorIndex() const
{
	return m_alignmentActorIndex;
}

// ----------------------------------------------------------------------

inline Tag AlignToHardpointActionTemplate::getAlignmentHardpointNameVariable() const
{
	return m_alignmentHardpointNameVariable;
}

// ======================================================================

#endif
