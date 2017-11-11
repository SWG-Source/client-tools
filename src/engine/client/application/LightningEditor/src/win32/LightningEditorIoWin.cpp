// ============================================================================
//
// LightningEditorIoWin.cpp
// Copyright Sony Online Entertainment
//
// ============================================================================

#include "FirstLightningEditor.h"
#include "LightningEditorIoWin.h"

#include "clientGame/ClientWorld.h"
#include "clientGame/FreeCamera.h"
#include "clientGame/Game.h"
#include "clientGame/GroundScene.h"
#include "clientGraphics/Graphics.h"
#include "clientGraphics/RenderWorld.h"
#include "clientGraphics/ShaderTemplateList.h"
#include "clientObject/GameCamera.h"
#include "clientParticle/LightningAppearance.h"
#include "clientParticle/LightningAppearanceTemplate.h"
#include "clientParticle/ParticleEffectAppearance.h"
#include "MainWindow.h"
#include "sharedDebug/PerformanceTimer.h"
#include "sharedObject/AppearanceTemplate.h"
#include "sharedObject/AppearanceTemplateList.h"
#include "sharedObject/Object.h"
#include "sharedObject/World.h"
#include "sharedTerrain/TerrainObject.h"

//=============================================================================
//
// LightningEditorIoWin
//
//=============================================================================

//-----------------------------------------------------------------------------
LightningEditorIoWin::LightningEditorIoWin(MainWindow *particleEditor)
 : IoWin ("LightningEditorIoWin")
 , m_object(NULL)
 , m_objectTransform(Transform::identity)
 , m_timeScale(1.0f)
 , m_mainWindow(particleEditor)
 , m_timeOfDayCycle(false)
 , m_boltCount(1)
 , m_endPoints(EP_fixedPosition)
{
	m_object = new Object();

	RenderWorld::addObjectNotifications(*m_object);
	m_object->addNotification(ClientWorld::getIntangibleNotification());
	m_object->addToWorld();
}

//-----------------------------------------------------------------------------
LightningEditorIoWin::~LightningEditorIoWin()
{
	delete m_object;
	m_object = NULL;
}

//-----------------------------------------------------------------------------
void LightningEditorIoWin::setAppearanceTemplate(AppearanceTemplate const * const appearanceTemplate, int const boltCount)
{
	m_boltCount = boltCount;

	// Recreate the object each time a appearance is assigned

	if ((m_object != NULL) &&
	    (appearanceTemplate != NULL))
	{
		Appearance *appearance = appearanceTemplate->createAppearance();
		LightningAppearance * const lightningAppearance = LightningAppearance::asLightningAppearance(appearance);
	
		// Do not allow the appearance to delete the object
	
		lightningAppearance->setUnBounded(true);
		
		m_object->removeFromWorld();
		m_object->setAppearance(lightningAppearance);

		for (int index = 0; index < boltCount; ++index)
		{
			lightningAppearance->setPosition_w(index, Vector(-10.0f, 3.0f, 0.0f), Vector(+10.0f, 3.0f, 0.0f));
		}

		m_object->addToWorld();
	}
}

//-----------------------------------------------------------------------------
void LightningEditorIoWin::draw() const
{
	IoWin::draw();

	// Hold the lighting of the terrain constant

	TerrainObject * const terrainObject = TerrainObject::getInstance();
	
	static float timeOfDay = 0.25f;

	if (terrainObject != NULL)
	{
		terrainObject->setTime(timeOfDay, true);
	}
}

//-----------------------------------------------------------------------------
void LightningEditorIoWin::setObjectTransform(Transform const &transform)
{
	m_objectTransform = transform;
}

//-----------------------------------------------------------------------------
void LightningEditorIoWin::setTimeScale(float const timeScale)
{
	if (m_object != NULL)
	{
		LightningAppearance * const lightningAppearance = LightningAppearance::asLightningAppearance(m_object->getAppearance());

		m_timeScale = timeScale;

		lightningAppearance->setTimeScale(timeScale);
	}
}

//-----------------------------------------------------------------------------
void LightningEditorIoWin::setTimeOfDayCycle(bool const timeOfDayCycle)
{
	m_timeOfDayCycle = timeOfDayCycle;
}

//-----------------------------------------------------------------------------
void LightningEditorIoWin::setEndPoints(EndPoints const endPoints)
{
	m_endPoints = endPoints;
}

//-----------------------------------------------------------------------------
void LightningEditorIoWin::alter(float const deltaTime)
{
	LightningAppearance * const lightningAppearance = LightningAppearance::asLightningAppearance(m_object->getAppearance());

	if (lightningAppearance == NULL)
	{
		return;
	}

	Vector const start(-10.0f, 3.0f, 0.0f);
	Vector const end(+10.0f, 3.0f, 0.0f);

	if (m_endPoints == EP_fixedPosition)
	{
		for (int index = 0; index < m_boltCount; ++index)
		{
			lightningAppearance->setPosition_w(index, start, end);
		}
	}

	if (m_endPoints == EP_contractExpand)
	{
		static float timer = 0.0f;
		timer += deltaTime * 2.0f;
		float const offset = sin(timer) * 4.0f;

		for (int index = 0; index < m_boltCount; ++index)
		{
			lightningAppearance->setPosition_w(index, Vector(-6.0f - offset, 3.0f, 0.0f), Vector(+6.0f + offset, 3.0f, 0.0f));
		}
	}

	if (m_endPoints == EP_spreadOut)
	{
		if (m_boltCount >= 1)
		{
			lightningAppearance->setPosition_w(0, start + Vector(0.0f, -0.5f, +0.0f), end + Vector(0.0f, -0.5f, +0.0f));
		}

		if (m_boltCount >= 2)
		{
			lightningAppearance->setPosition_w(1, start + Vector(0.0f, -0.5f, -0.5f), end + Vector(0.0f, -0.5f, -0.5f));
		}

		if (m_boltCount >= 3)
		{
			lightningAppearance->setPosition_w(2, start + Vector(0.0f, -0.5f, +0.5f), end + Vector(0.0f, -0.5f, +0.5f));
		}

		if (m_boltCount >= 4)
		{
			lightningAppearance->setPosition_w(3, start + Vector(0.0f, +0.5f, -0.5f), end + Vector(0.0f, +0.5f, -0.5f));
		}

		if (m_boltCount >= 5)
		{
			lightningAppearance->setPosition_w(4, start + Vector(0.0f, +0.5f, +0.5f), end + Vector(0.0f, +0.5f, +0.5f));
		}
	}

	if (m_endPoints == EP_rotate)
	{
		static float timer = 0.0f;
		timer += deltaTime * 2.0f;
		float const offset = sin(timer) * 8.0f;

		for (int index = 0; index < m_boltCount; ++index)
		{
			lightningAppearance->setPosition_w(index, Vector(-10.0f, 3.0f, -offset), Vector(10.0f, 3.0f, +offset));
		}
	}
}

//-----------------------------------------------------------------------------
float LightningEditorIoWin::getTimeScale() const
{
	return m_timeScale;
}

//=============================================================================
