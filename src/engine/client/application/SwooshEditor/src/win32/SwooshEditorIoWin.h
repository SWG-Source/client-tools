// ============================================================================
//
// SwooshEditorIoWin.h
// Copyright Sony Online Entertainment
//
// ============================================================================

#ifndef INCLUDED_SwooshEditorIoWin_H
#define INCLUDED_SwooshEditorIoWin_H

#include "sharedIoWin/IoWin.h"
#include "sharedMath/Transform.h"
#include "../../../../../../engine/shared/library/sharedFoundation/include/public/sharedFoundation/Watcher.h"

class AppearanceTemplate;
class MainWindow;
class Object;

//-----------------------------------------------------------------------------
class SwooshEditorIoWin : public IoWin
{
public:

	enum ReferenceSwoosh
	{
		  RS_none
		, RS_circling
		, RS_spiraling
	};

	SwooshEditorIoWin(MainWindow *mainWindow);
	virtual ~SwooshEditorIoWin();

	void alter(float const deltaTime);
	void draw() const;
	void setAppearanceTemplate(AppearanceTemplate const * const appearanceTemplate);
	void setObjectTransform(Transform const &transform);
	void setTimeOfDayCycle(bool const timeOfDayCycle);
	void setObjectRotationSpeed(float const objectRotationSpeed);
	void setAnimation(std::string const &animationName, std::string const &weaponName, int const trailFlags, float const defenderDistance);
	void setReferenceSwoosh(ReferenceSwoosh const referenceSwoosh);
	void setPauseAfterEachAnimation(bool const enabled);
	bool isPauseAfterEachAnimation() const;

private:

	Object *        m_object1;
	Object *        m_object2;
	Watcher<Object> m_defenderObject;
	Watcher<Object> m_weaponObject;
	Transform       m_objectTransform;
	MainWindow *    m_mainWindow;
	bool            m_timeOfDayCycle;
	float           m_objectRotationSpeed;
	int             m_gameUpdate;
	std::string     m_animationName;
	int             m_trailFlags;
	float           m_defenderDistance;
	std::string     m_weaponName;
	bool            m_showReferenceSwooshes;
	bool            m_pauseAfterEachAnimation;
	float           m_pauseAfterEachAnimationTimer;
	AppearanceTemplate const * m_nextAppearanceTemplate;
	bool m_appearanceChanged;
	ReferenceSwoosh m_referenceSwoosh;
	float m_spiralRadius;

	void unequipWeapon();
	void equipWeapon(std::string const &weaponName);

private:

	// Disable these functions

	SwooshEditorIoWin(SwooshEditorIoWin const &rhs);
	SwooshEditorIoWin & operator=(SwooshEditorIoWin const &rhs);
};

//=============================================================================

#endif // INCLUDED_SwooshEditorIoWin_H
