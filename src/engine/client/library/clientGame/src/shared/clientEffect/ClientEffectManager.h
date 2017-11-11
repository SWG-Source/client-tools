// ======================================================================
//
// ClientEffectManager.h
// Copyright 2002 Sony Online Entertainment Inc.
// All Rights Reserved.
// 
// ======================================================================

#ifndef INCLUDED_ClientEffectManager_H
#define INCLUDED_ClientEffectManager_H

// ======================================================================

class CellProperty;
class ClientEffect;
class CrcLowerString;
class Light;
class Object;
class ObjectClientEffect;
class Plane;
class PositionClientEffect;
class Shader;
class Transform;
class Vector;

// ======================================================================

/** This class is a statically-implemented singlton manager class used to provide a system-inteface to playing
 *  client effects. Given the name of a client effect template, and a position in space or an object, the system
 *  will play the given effect on it.  The results will be simple command such as spawning a particle system for a given time,
 *  applying a decal, etc.
 */
class ClientEffectManager
{
/** Allow the effects to hand-off created objects to this system to manage.  The effect's lifetime might not extend long enough
 *  to handle cleanup.
 */
friend ClientEffect;
friend ObjectClientEffect;
friend PositionClientEffect;

private:

	struct ManagedParticleSystem;
	friend ClientEffectManager::ManagedParticleSystem;
	struct ManagedLight;
	friend ClientEffectManager::ManagedLight;

public:
	static void install();

	static void sendHeartbeat(float timeElapsed);
	static bool playClientEffect(const CrcLowerString& clientEffectName, Object* object, const CrcLowerString& hardpoint);
	static bool playClientEffect(const CrcLowerString& clientEffectName, Object* object, Transform const & transform);
	static bool playClientEffect(const CrcLowerString& clientEffectName, const CellProperty* cell, const Vector& location, const Vector& up);

	static bool playLabeledClientEffect(const CrcLowerString& clientEffectName, Object* object, const CrcLowerString& hardpoint, CrcLowerString const & label);
	static bool playLabeledClientEffect(const CrcLowerString& clientEffectName, Object* object, Transform const & transform, CrcLowerString const & label);
	static bool playLabeledClientEffect(const CrcLowerString& clientEffectName, const CellProperty* cell, const Vector& location, const Vector& up, CrcLowerString const & label);

	static void removeAllClientEffectsForObject(Object const * object);
	static void removeAllClientEffectsForObjectByLabel(Object const * const object, CrcLowerString const & label, bool softTerminate);

	static void transferOwnerShipOfAllClientEffects(Object & previousOwner, Object & newOwner, Plane const * partition, bool reparentObjects);
	static void removeNonStealthClientEffects(Object const * object);

	static void createClientNonTrackingProjectile(std::string const & weaponObjectTemplateName, CellProperty const * const cellProperty, Vector const & startLoc, Vector const & endLoc, float const speed, float const expiration, bool const hasTrail, uint32 const trailArgb);
	static void createClientTrackingProjectileObjectToObject(std::string const & weaponObjectTemplateName, CellProperty const * const cellProperty, Object const & source, std::string const & sourceHardpoint, Object const & target, std::string const & targetHardpoint, float const speed, float const expiration, bool hasTrail, uint32 const trailArgb);
	static void createClientTrackingProjectileLocationToObject(std::string const & weaponObjectTemplateName, CellProperty const * const cellProperty, Vector const & startLoc, Object const & target, std::string const & targetHardpoint, float const speed, float const expiration, bool const hasTrail, uint32 const trailArgb);
	static void createClientTrackingProjectileObjectToLocation(std::string const & weaponObjectTemplateName, CellProperty const * const cellProperty, Object const & source, std::string const & sourceHardpoint, Vector const & endLoc, float const speed, float const expiration, bool const hasTrail, uint32 const trailArgb);
private:
	static void remove();

	static void addManagedLight(Light* light, Object* owner, const CrcLowerString& hardpoint, Transform const & transform, float time, CrcLowerString &label);
	static void addManagedParticleSystem(Object* particleSystem, Object* owner, const CrcLowerString& hardpoint, Transform const & transform, float time, bool softParticleTerminate, bool ignoreDuration, CrcLowerString &label);
	static void addManagerShader(Shader* shader, float time);
	static bool isStealthEffect(const char *particleName);
private:
	static bool ms_installed;

	typedef stdvector<ManagedLight*>::fwd LightList;
	static LightList    m_lights;

	typedef stdvector<ManagedParticleSystem*>::fwd ParticleList;
	static ParticleList m_particleSystems;

	typedef stdvector<std::pair<float, Shader*> >::fwd ShaderList;
	static ShaderList   m_shaders;
};

// ======================================================================

#endif
