// ======================================================================
//
// UseLightningActionTemplate.h
// Copyright 2003 Sony Online Entertainment, Inc.
// All Rights Reserved.
//
// ======================================================================

#ifndef INCLUDED_UseLightningActionTemplate_H
#define INCLUDED_UseLightningActionTemplate_H

// ======================================================================

#include "clientAnimation/PlaybackActionTemplate.h"
#include "../../../../../../engine/shared/library/sharedFoundation/include/public/sharedFoundation/MemoryBlockManagerMacros.h"
#include "../../../../../../engine/shared/library/sharedFoundation/include/public/sharedFoundation/Tag.h"

class Iff;

// ======================================================================

class UseLightningActionTemplate: public PlaybackActionTemplate
{
	MEMORY_BLOCK_MANAGER_INTERFACE_WITHOUT_INSTALL;

public:

	enum
	{
		cms_maxHardpointCount = 2
	};

public:

	static void install();

public:

	virtual PlaybackAction *createPlaybackAction() const;

	int  getAttackerActorIndex() const;
	int  getDefenderActorIndex() const;

	Tag  getHardpointVariableTag(int index) const;
	Tag  getLevelVariableTag() const;
	Tag  getTotalTimeVariableTag() const;
	Tag  getGrowTimeFractionVariableTag() const;
	Tag  getDeathTimeFractionVariableTag() const;

	Tag  getBeginSoundVariableTag() const;
	Tag  getLoopSoundVariableTag() const;
	Tag  getEndSoundVariableTag() const;

	Tag  getAppearanceTemplateVariableTag() const;

	Tag  getBeamRotationRadiusVariableTag() const;   // radius around target endpoint
	Tag  getBeamDpsRotationRateVariableTag() const;  // in degrees per second (DPS)

private:

	static void                    remove();
	static PlaybackActionTemplate *create(Iff &iff);

private:

	explicit UseLightningActionTemplate(Iff &iff);
	void     load_0000(Iff &iff);
	void     load_0001(Iff &iff);
	void     load_0002(Iff &iff);

	// Disabled.
	UseLightningActionTemplate();
	UseLightningActionTemplate(UseLightningActionTemplate const&);
	UseLightningActionTemplate &operator =(UseLightningActionTemplate const&);

private:

	int  m_attackerActorIndex;
	int  m_defenderActorIndex;

	Tag  m_appearanceTemplateVariableTag;
	Tag  m_hardpointVariableTag[cms_maxHardpointCount];
	Tag  m_levelVariableTag;
	Tag  m_totalTimeVariableTag;
	Tag  m_growTimeFractionVariableTag;
	Tag  m_deathTimeFractionVariableTag;
	Tag  m_beginSoundVariableTag;
	Tag  m_loopSoundVariableTag;
	Tag  m_endSoundVariableTag;
	Tag  m_beamRotationRadiusVariableTag;
	Tag  m_beamRotationRateVariableTag;

};

// ======================================================================

#endif
