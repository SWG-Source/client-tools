// ======================================================================
//
// LightsaberAppearance.h
// Copyright 2003 Sony Online Entertainment, Inc.
// All Rights Reserved.
//
// ======================================================================

#ifndef INCLUDED_LightsaberAppearance_H
#define INCLUDED_LightsaberAppearance_H

// ======================================================================

class BeamAppearance;
class MemoryBlockManager;
class Object;
class Segment3d;

#include "clientAudio/SoundId.h"
#include "sharedCollision/BoxExtent.h"
#include "../../../../../../engine/shared/library/sharedFoundation/include/public/sharedFoundation/MemoryBlockManagerMacros.h"
#include "../../../../../../engine/shared/library/sharedFoundation/include/public/sharedFoundation/Watcher.h"
#include "sharedObject/Appearance.h"
#include "sharedObject/Object.h"
#include <vector>

// ======================================================================

class LightsaberAppearance: public Appearance
{
	friend class LightsaberAppearanceTemplate;
	MEMORY_BLOCK_MANAGER_INTERFACE_WITH_INSTALL;

public:

	enum BladeState
	{
		BS_off,
		BS_on,
		BS_transitioningOff,
		BS_transitioningOn
	};

public:

	static CrcString const &getBaseHardpointName(int bladeIndex);
	static CrcString const &getMidpointHardpointName(int bladeIndex);
	static CrcString const &getTipHardpointName(int bladeIndex);

public:

	virtual ~LightsaberAppearance();

	virtual bool collide(Vector const & start_o, Vector const & end_o, CollideParameters const & collideParameters, CollisionInfo& result) const;
	virtual const Extent *getExtent() const;

	virtual float         alter(float elapsedTime);

	virtual void          setOwner(Object *newOwner);;

	virtual void          setCustomizationData(CustomizationData *customizationData);
	virtual void          addCustomizationVariables(CustomizationData &customizationData) const;

	virtual bool          findHardpoint(CrcString const &hardpointName, Transform &hardpointTransform) const;

	virtual void          onEvent(LabelHash::Id eventId);

	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

	void                  setSoundAndEventTarget(Object const *target);

	int                   getBladeCount() const;
	BladeState            getBladeState(int bladeNumber) const;
	void                  turnBladeOn(int bladeNumber, bool turnOn, bool skipTransitionAndSound);
	Object const         *getWielderObject() const;
	void                  getBladeSegment(int bladeNumber, Segment3d &segment) const;

private:

	typedef stdvector<BeamAppearance*>::fwd  BeamAppearanceVector;	
	typedef stdvector<BladeState>::fwd       BladeStateVector;
	typedef stdvector<float>::fwd            FloatVector;
	typedef stdvector<Object*>::fwd          ObjectVector;

private:

	explicit LightsaberAppearance(LightsaberAppearanceTemplate const &appearanceTemplate);

	LightsaberAppearanceTemplate const &getLightsaberAppearanceTemplate() const;

	float                               getBladeLength(int bladeNumber, float elapsedTime, BladeState &endState) const;

	virtual DPVS::Object               *getDpvsObject() const;

	void                                handleCustomizationModification(const CustomizationData &customizationData);
	static void                         handleCustomizationModificationCallback(const CustomizationData &customizationData, const void *context);

	void                                turnBladeOffInternal();

	// Disabled.
	LightsaberAppearance();
	LightsaberAppearance(LightsaberAppearance const&);
	LightsaberAppearance &operator =(LightsaberAppearance const&);

private:

	Object               *m_hiltObject;
	Appearance           *m_hiltAppearance;

	SoundId               m_ambientSoundId;
	bool                  m_ambientSoundIsPlaying;

	ObjectVector          m_bladeObjects;
	BeamAppearanceVector  m_bladeAppearances;
	FloatVector           m_bladeLengths;
	BladeStateVector      m_bladeStates;

	BoxExtent mutable     m_boxExtent;
	bool mutable          m_extentUpdateRequired;	
	bool mutable          m_doObjectHardpointLookup;

	DPVS::Object         *m_dpvsObject;

	ConstWatcher<Object>  m_soundAndEventTarget;
	Watcher<Object>       m_lightWatcher;

	CustomizationData    *m_customizationData;
	int                   m_alternateShader;

	float                 m_debounceBladeTimer;
};

// ======================================================================

#endif
