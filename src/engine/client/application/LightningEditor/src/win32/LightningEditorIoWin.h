// ============================================================================
//
// LightningEditorIoWin.h
// Copyright Sony Online Entertainment
//
// ============================================================================

#ifndef INCLUDED_LightningEditorIoWin_H
#define INCLUDED_LightningEditorIoWin_H

#include "sharedIoWin/IoWin.h"
#include "sharedMath/Transform.h"

class AppearanceTemplate;
class MainWindow;
class Object;

//-----------------------------------------------------------------------------
class LightningEditorIoWin : public IoWin
{
public:

	enum EndPoints
	{
		EP_fixedPosition,
		EP_contractExpand,
		EP_spreadOut,
		EP_rotate
	};

public:

	LightningEditorIoWin(MainWindow *mainWindow);
	virtual ~LightningEditorIoWin();

	void draw() const;
	void alter(float const deltaTime);
	void setAppearanceTemplate(AppearanceTemplate const * const appearanceTemplate, int const boltCount);
	void setObjectTransform(Transform const &transform);
	void setTimeScale(float const timeScale);
	void setTimeOfDayCycle(bool const timeOfDayCycle);
	void setEndPoints(EndPoints const endPoints);

	float getTimeScale() const;

private:

	Object *     m_object;
	Transform    m_objectTransform;
	float        m_timeScale;
	MainWindow * m_mainWindow;
	bool         m_timeOfDayCycle;
	int          m_boltCount;
	EndPoints    m_endPoints;

private:

	// Disable these functions

	LightningEditorIoWin(LightningEditorIoWin const &rhs);
	LightningEditorIoWin & operator=(LightningEditorIoWin const &rhs);
};

//=============================================================================

#endif // INCLUDED_LightningEditorIoWin_H
