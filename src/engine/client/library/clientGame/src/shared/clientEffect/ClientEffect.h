// ======================================================================
//
// ClientEffect.h
// Copyright 2002 Sony Online Entertainment Inc.
// All Rights Reserved.
// 
// ======================================================================

#ifndef INCLUDED_ClientEffect_H
#define INCLUDED_ClientEffect_H

// ======================================================================

#include "../../../../../../engine/shared/library/sharedFoundation/include/public/sharedFoundation/MemoryBlockManagerMacros.h"
#include "sharedMath/Transform.h"
#include "sharedMath/Vector.h"

// ======================================================================

class ClientEffectTemplate;
class CellProperty;
class CrcLowerString;
class ForceFeedbackEffectTemplate;
class Object;

// ======================================================================

/** Abstract class used to play client effects
 */
class ClientEffect
{
public:

	virtual ~ClientEffect();

	virtual void execute() = 0;

	void setUniformScale(float scale);
	void setUseRenderEnableFlags(bool use);
	void setLabel(CrcLowerString const & label);

protected:

	explicit ClientEffect(const ClientEffectTemplate* clientEffectTemplate);

	void shakeCamera(const Vector& objectPosition_w, float falloff, float time, float magnitude, float frequency);
	void playForceFeedbackEffect(Vector const & effectPosition, Vector const & playerPosition, ForceFeedbackEffectTemplate const * forceFeedbackTemplate, int iterations, float range) const;

private:

	ClientEffect();
	ClientEffect(const ClientEffect&);
	ClientEffect& operator=(const ClientEffect&);

protected:
	const ClientEffectTemplate* const m_clientEffectTemplate;

protected:
	
	float           m_scaleOverride;
	bool            m_useRenderEnableFlags;
	CrcLowerString  * m_label;

};

// ======================================================================

/** This class represents a effect to play on the game client.  This is comprised of a list of simple commands
 *  (i.e. "put a decal here", "play this sound", "shake the camera", etc.) and a specific point in space to 
 *  play the effect.
 */
class PositionClientEffect : public ClientEffect
{
	MEMORY_BLOCK_MANAGER_INTERFACE_WITH_INSTALL;

	friend ClientEffectTemplate;

public:

	virtual ~PositionClientEffect();

	virtual void execute();

protected:

	PositionClientEffect(const ClientEffectTemplate* cet, const CellProperty* cell, const Vector& position, const Vector& up);

private:

	PositionClientEffect();
	PositionClientEffect(const PositionClientEffect&);
	PositionClientEffect& operator=(const PositionClientEffect&);

private:

	const CellProperty* const m_cell;
	const Vector              m_position;
	const Vector              m_up;
};

// ======================================================================

/** This class represents a effect to play on the game client.  This is comprised of a list of simple commands
 *  (i.e. "put a decal here", "play this sound", "shake the camera", etc.) and a specific object on which to 
 *  play the effect.
 */
class ObjectClientEffect : public ClientEffect
{
	MEMORY_BLOCK_MANAGER_INTERFACE_WITH_INSTALL;

	friend ClientEffectTemplate;

public:

	virtual ~ObjectClientEffect();
	virtual void execute();

protected:

	ObjectClientEffect(const ClientEffectTemplate* cet, Object* object, const CrcLowerString& hardpoint);
	ObjectClientEffect(const ClientEffectTemplate* cet, Object* object, Transform const & transform);

private:

	ObjectClientEffect();
	ObjectClientEffect(const ObjectClientEffect&);
	ObjectClientEffect& operator=(const ObjectClientEffect&);

private:

	Object* const         m_object;
	const CrcLowerString& m_hardPoint;
	Transform m_transform;
};

// ======================================================================

#endif
