// ============================================================================
//
// ParticleEditorIoWin.h
// Copyright Sony Online Entertainment
//
// ============================================================================

#ifndef INCLUDED_ParticleEditorIoWin_H
#define INCLUDED_ParticleEditorIoWin_H

#include "ParticleEffectTransformEdit.h"
#include "sharedIoWin/IoWin.h"
#include "sharedMath/Transform.h"

class AppearanceTemplate;
class Object;

//-----------------------------------------------------------------------------
class ParticleEditorIoWin : public IoWin
{
public:

	ParticleEditorIoWin(MainWindow *particleEditor);
	virtual ~ParticleEditorIoWin();

	void draw() const;
	void alter(float const deltaTime);
	void setAppearanceTemplate(AppearanceTemplate const * const appearanceTemplate);
	void setObjectTransform(Transform const &transform);
	void setEffectScale(float const effectScale);
	void setPlayBackRate(float const playBackRate);
	void setObjectMovement(ParticleEffectTransformEdit::ObjectMovement const objectMovement);
	Object const * const getObject() const;
	void setTimeOfDayCycle(bool const timeOfDayCycle);
	void setSize(float size);
	void setSpeed(float speed);

private:

	Object *                                    m_object;
	Object *                                    m_lightningObject;
	Transform                                   m_objectTransform;
	float                                       m_playBackRate;
	float                                       m_effectScale;
	ParticleEffectTransformEdit::ObjectMovement m_objectMovement;
	MainWindow *                                m_particleEditor;
	mutable float                               m_deltaX;
	mutable float                               m_radian;
	float                                       m_size;
	float                                       m_speed;

	void setObjectMovement(float const deltaTime) const;

private:

	// Disable these functions

	ParticleEditorIoWin(ParticleEditorIoWin const &rhs);
	ParticleEditorIoWin & operator=(ParticleEditorIoWin const &rhs);
};

//=============================================================================

#endif // INCLUDED_ParticleEditorIoWin_H
