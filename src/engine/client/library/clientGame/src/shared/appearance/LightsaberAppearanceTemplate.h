// ======================================================================
//
// LightsaberAppearanceTemplate.h
// Copyright 2003 Sony Online Entertainment, Inc.
// All Rights Reserved.
//
// ======================================================================

#ifndef INCLUDED_LightsaberAppearanceTemplate_H
#define INCLUDED_LightsaberAppearanceTemplate_H

// ======================================================================

class BeamAppearance;
class MemoryBlockManager;
class Shader;

#include "../../../../../../engine/shared/library/sharedFoundation/include/public/sharedFoundation/MemoryBlockManagerMacros.h"
#include "../../../../../../engine/shared/library/sharedFoundation/include/public/sharedFoundation/PersistentCrcString.h"
#include "sharedMath/PackedArgb.h"
#include "sharedObject/AppearanceTemplate.h"

#include <vector>

// ======================================================================

class LightsaberAppearanceTemplate: public AppearanceTemplate
{
	MEMORY_BLOCK_MANAGER_INTERFACE_WITHOUT_INSTALL;

public:

	// Exposed for inner classes.
	class BladeInfo;
	typedef stdvector<BladeInfo*>::fwd  BladeInfoVector;

public:

	static void         install();

public:

	virtual Appearance *createAppearance() const;

	virtual void        preloadAssets () const;
	virtual void        garbageCollect () const;

	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
	// Class-specific functionality
	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

	Appearance         *createBaseAppearance() const;
	
	CrcString const    &getAmbientSoundTemplateName() const;

	bool                useLightFlicker() const;
	PackedArgb const   &getFlickerSecondaryColor() const;
	void                getFlickerRange(float &min, float &max) const;
	void                getFlickerTime(float &min, float &max) const;
	bool                getFlickerIsDayNightAware() const;

	int                 getBladeCount() const;

	BeamAppearance     *createBladeAppearance(int bladeNumber) const;
	Shader             *createBladeShader(int bladeNumber) const;
	float               getBladeLength(int bladeNumber) const;
	float               getBladeWidth(int bladeNumber) const;
	float               getBladeOpenRate(int bladeNumber) const;
	float               getBladeCloseRate(int bladeNumber) const;

private:

	class PreloadData;

private:

	static void                remove();
	static AppearanceTemplate *create(const char *name, Iff *iff);

private:

	LightsaberAppearanceTemplate(const char *name, Iff &iff);
	virtual ~LightsaberAppearanceTemplate();

	void                       load_0000(Iff &iff);
	BladeInfo const           &getBladeInfo(int bladeIndex) const;

	// Disabled.
	LightsaberAppearanceTemplate();
	LightsaberAppearanceTemplate(LightsaberAppearanceTemplate const&);
	LightsaberAppearanceTemplate &operator =(LightsaberAppearanceTemplate const&);

private:

	PersistentCrcString  m_hiltAppearanceTemplateName;
	PersistentCrcString  m_ambientSoundTemplateName;

	bool                 m_useLightFlicker;
	PackedArgb           m_flickerSecondaryColor;
	float                m_flickerMinRange;
	float                m_flickerMaxRange;
	float                m_flickerMinTime;
	float                m_flickerMaxTime;
	bool                 m_flickerIsDayNightAware;

	BladeInfoVector      m_bladeInfoVector;
	PreloadData mutable *m_preloadData;

};

// ======================================================================

#endif
