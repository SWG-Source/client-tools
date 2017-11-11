//======================================================================
//
// ShipObjectEffects.h
// copyright (c) 2004 Sony Online Entertainment
//
//======================================================================

#ifndef INCLUDED_ShipObjectEffects_H
#define INCLUDED_ShipObjectEffects_H

//======================================================================

#include "../../../../../../engine/shared/library/sharedFoundation/include/public/sharedFoundation/Watcher.h"
#include "../../../../../../engine/shared/library/sharedFoundation/include/public/sharedFoundation/Timer.h"
#include "sharedObject/CachedNetworkId.h"

class ShipObject;
class Object;
class ClientObject;
class TangibleObject;
class VehicleThrusterSoundRuntimeData;
class InterpolatedSoundRuntime;
class SoundTemplate;
class ClientEffect;
template <typename T> class Watcher;

//----------------------------------------------------------------------

class ShipObjectEffects
{
public:

	class EngineEffects;

	~ShipObjectEffects();

	typedef Watcher<Object> ObjectWatcher;
	typedef stdvector<ObjectWatcher>::fwd   WatcherVector;
	typedef stdmap<CachedNetworkId, EngineEffects>::fwd EngineEffectsMap;
	typedef stdset<std::string>::fwd StringSet;			
	
	explicit ShipObjectEffects(ShipObject & ship);
	
	void updateComponentState(TangibleObject * tangibleObject, int const chassisSlot, float oldDamageLevel, float currentDamageLevel);
	void updateEngineEffects(float elapsedTime);
	void resetEngineEffects(ClientObject * child, int chassisSlot);
	void removeThrusterEffects();
	void removeBoosterEffect();
	
	void clearContrails();
	void resetContrails();
	void addContrailsFor(Object & object, StringSet & overrides, WatcherVector & oldContrails);
	
	void initialize();
	void updateFlybySounds(float const elapsedTime);
	
	void removeAllTargetingEffects();
	void setTargetAcquiredActive(bool const active);
	void setTargetAcquiringActive(bool const active, float const acquisitionTime);
	void updateTargetAppearance(float const elapsedTime);
	bool getTargetAppearanceActive() const;
	void setTargetAppearanceActive(bool active);
	void updateTargetAcquisition(float elapsedTime);

	void updateGlowLists();

	void releaseEffectsForSlot(int chassisSlot);

	static void setHideAllTargetEffects(bool b);
	
private:
	
	ShipObjectEffects(ShipObjectEffects const & rhs);
	ShipObjectEffects & operator=(ShipObjectEffects const & rhs);

	void updateGlowListsInternal(Object & object);
	void updateEngineInterpolatedSoundState(EngineEffects & effects, float currentDamageLevel);

private:

	ShipObject * m_ship;
	EngineEffectsMap * m_engineEffects;
	WatcherVector * m_contrails;
	InterpolatedSoundRuntime * m_boosterInterpolatedSoundRuntime;
	SoundTemplate const * m_flyByTemplate;

    bool m_targetAppearanceObjectFlag;
    int m_targetComponentAppearanceObjectSlot;
    bool m_targetAppearanceUseParentOrientation;
	float m_targetAppearanceSilhouetteDistance;
	ObjectWatcher m_targetAcquiredAppearanceObject;
	ClientEffect * m_targetAcquiringEffect;
	WatcherVector * m_glowsEngine;
	WatcherVector * m_glowsBooster;
	Timer m_targetAcquiringEffectTimer;

#ifdef _DEBUG
	bool m_missingHardpoint0Warning;
	bool m_missingHardpoint1Warning;
#endif
};

//======================================================================

#endif
