// ======================================================================
//
// CelestialObject.h
// asommers
// 
// copyright 2003, sony online entertainment
//
// ======================================================================

#ifndef INCLUDED_CelestialObject_H
#define INCLUDED_CelestialObject_H

// ======================================================================

#include "sharedObject/Object.h"

class Camera;
class CelestialAppearance;
class CellProperty;
class CollideParameters;
class CollisionInfo;
class VectorArgb;

// ======================================================================

class CelestialObject : public Object
{
public:
	typedef bool (*CollideFunctionExcludeObjectFunction) (Object const *);
	typedef bool (*CollideFunction) (const CellProperty* startCell, const Vector& startPosition_w, const Vector& endPosition_w, CollideParameters const & collideParameters, CollisionInfo& result, uint16 flags, const Object* excludeObject, CollideFunctionExcludeObjectFunction cfeof);
	typedef void (*NotificationFunction) (Object& object);

public:

	static void install ();
	static void setCollideFunction (CollideFunction collideFunction, uint16 collideFlags);
	static void setNotificationFunction (NotificationFunction notificationFunction);

public:

	CelestialObject (const char* shaderTemplateName, float size, const char* glowShaderTemplateName, float glowSize, bool space = false);
	virtual ~CelestialObject ();

	virtual void setActive (bool active);

	void  setAlpha (float alpha);
	void  setColor (const VectorArgb& color);
	void  update (float elapsedTime, const Camera& camera);
	float getGlowAlpha () const;
	void setSpace();

	virtual void getObjectInfo(stdmap<std::string, stdmap<std::string, Unicode::String>::fwd >::fwd & propertyMap) const;

private:

	CelestialObject (const CelestialObject&);
	CelestialObject& operator= (const CelestialObject&);

private:

	static CollideFunction      ms_collideFunction;
	static uint16               ms_collideFlags;
	static NotificationFunction ms_notificationFunction;

private:

	bool const m_space;

	CelestialAppearance*        m_celestialAppearance;
	float                       m_alpha;

	Object*                     m_glowObject;
	CelestialAppearance*        m_glowCelestialAppearance;
	float                       m_glowAlpha;
};

// ======================================================================

#endif

