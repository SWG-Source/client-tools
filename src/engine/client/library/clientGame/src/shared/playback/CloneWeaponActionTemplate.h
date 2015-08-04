// ======================================================================
//
// CloneWeaponActionTemplate.h
// Copyright 2002 Sony Online Entertainment, Inc.
// All Rights Reserved.
//
// ======================================================================

#ifndef INCLUDED_CloneWeaponActionTemplate_H
#define INCLUDED_CloneWeaponActionTemplate_H

// ======================================================================

#include "clientAnimation/PlaybackActionTemplate.h"
#include "../../../../../../engine/shared/library/sharedFoundation/include/public/sharedFoundation/Tag.h"

class CrcLowerString;
class Iff;
class ObjectTemplate;
class SkeletalAppearance2;

// ======================================================================
/**
 * An action that will take the attacker weapon Object, clone it,
 * and stick the cloned version on the specified hardpoint of the
 * attacker.
 */

class CloneWeaponActionTemplate: public PlaybackActionTemplate
{
public:

	static void install();

public:

	virtual PlaybackAction *createPlaybackAction() const;
	virtual bool            update(float deltaTime, PlaybackScript &script) const;

private:

	static void                    remove();
	static PlaybackActionTemplate *create(Iff &iff);

private:

	CloneWeaponActionTemplate(Iff &iff);

	void                  load_0000(Iff &iff);

	int                   getActorIndex() const;
	Tag                   getFallbackObjectTemplateNameVariable() const;
	Tag                   getHardpointNameVariable() const;

	const ObjectTemplate *fetchWeaponObjectTemplate(const PlaybackScript &script) const;
	SkeletalAppearance2  *getActorAppearance(PlaybackScript &script) const;
	bool                  getHardpointName(const PlaybackScript &script, CrcLowerString &name) const;

	// Disabled.
	CloneWeaponActionTemplate();
	CloneWeaponActionTemplate(const CloneWeaponActionTemplate&);
	CloneWeaponActionTemplate &operator =(const CloneWeaponActionTemplate&);

private:

	static bool  ms_installed;

private:

	/// Actor index for the object that will hold the cloned object.
	int  m_actorIndex;

	/// Variable containing the name of the object template to use as a fallback should the weapon object be NULL.
	Tag  m_fallbackObjectTemplateNameVariable;

	/// Variable containing the name of the hardpoint where the cloned weapon object will be attached on the specified actor.
	Tag  m_hardpointNameVariable;
};

// ======================================================================

inline int CloneWeaponActionTemplate::getActorIndex() const
{
	return m_actorIndex;
}

// ----------------------------------------------------------------------

inline Tag CloneWeaponActionTemplate::getFallbackObjectTemplateNameVariable() const
{
	return m_fallbackObjectTemplateNameVariable;
}

// ----------------------------------------------------------------------

inline Tag CloneWeaponActionTemplate::getHardpointNameVariable() const
{
	return m_hardpointNameVariable;
}

// ======================================================================

#endif
