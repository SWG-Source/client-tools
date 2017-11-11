// ======================================================================
//
// DetailAppearance.cpp
// Portions Copyright 1999 Bootprint Entertainment
// Portions Copyright 2001 Sony Online Entertainment Inc.
// All Rights Reserved.
//
// ======================================================================

#include "clientObject/FirstClientObject.h"
#include "clientObject/DetailAppearance.h"

#include "clientGraphics/Camera.h"
#include "clientGraphics/Graphics.h"
#include "clientGraphics/RenderWorld.h"
#include "clientGraphics/ShaderCapability.h"
#include "clientGraphics/ShaderPrimitiveSorter.h"
#include "clientGraphics/Texture.h"
#include "clientObject/ConfigClientObject.h"
#include "clientObject/DetailAppearanceTemplate.h"
#include "sharedCollision/BoxExtent.h"
#include "sharedCollision/CollideParameters.h"
#include "sharedCollision/Extent.h"
#include "sharedCollision/ExtentList.h"
#include "sharedDebug/DataLint.h"
#include "sharedDebug/DebugFlags.h"
#include "sharedFoundation/CrcLowerString.h"
#include "sharedFoundation/PointerDeleter.h"
#include "sharedFoundation/VoidBindSecond.h"
#include "sharedMath/DebugShapeRenderer.h"
#include "sharedMath/IndexedTriangleList.h"
#include "sharedMath/Sphere.h"
#include "sharedObject/AlterResult.h"
#include "sharedObject/AlterScheduler.h"
#include "sharedObject/NoRenderAppearance.h"
#include "sharedObject/Object.h"
#include "sharedObject/CellProperty.h"
#include "sharedUtility/LocalMachineOptionManager.h"

#include "dpvsObject.hpp"
#include "dpvsModel.hpp"

#include <algorithm>
#include <vector>
#include <stdio.h>

// ======================================================================

namespace DetailAppearanceNamespace
{
	bool  ms_drawRadarShape        = false;
	bool  ms_drawTestShape         = false;
	bool  ms_drawWriteShape        = false;
	bool  ms_forceLowDetailLevels  = false;
	bool  ms_forceHighDetailLevels = false;
	bool  ms_enableDetailLevelStretch = true;
	bool  ms_fadeInEnabled         = true;
	bool  ms_crossFadeEnabled      = false;
	bool  ms_lockLod               = false;
	float ms_detailLevelBias       = 1.f;
}
using namespace DetailAppearanceNamespace;

// ======================================================================

void DetailAppearance::install()
{
	const char * const section = "ClientObject";

#ifdef _DEBUG
	DebugFlags::registerFlag(ms_drawRadarShape,           section, "drawRadarShapes");
	DebugFlags::registerFlag(ms_drawTestShape,            section, "drawTestShapes");
	DebugFlags::registerFlag(ms_drawWriteShape,           section, "drawWriteShapes");
	DebugFlags::registerFlag(ms_lockLod,                  section, "lockDetailAppearanceLod");
	DebugFlags::registerFlag(ms_enableDetailLevelStretch, section, "enableDetailLevelStretch");
#endif

	ms_detailLevelBias = ConfigClientObject::getDetailLevelBias ();

	//-- set any options that persist
	LocalMachineOptionManager::registerOption (ms_forceLowDetailLevels,  section, "forceLowDetailLevels");
	LocalMachineOptionManager::registerOption (ms_forceHighDetailLevels, section, "forceHighDetailLevels");
	LocalMachineOptionManager::registerOption (ms_detailLevelBias,       section, "detailLevelBias");
	LocalMachineOptionManager::registerOption (ms_fadeInEnabled,         section, "fadeInEnabled");
	LocalMachineOptionManager::registerOption (ms_crossFadeEnabled,      section, "crossFadeEnabled");

	if (Graphics::getShaderCapability() < ShaderCapability(1, 1))
	{
		ms_fadeInEnabled = false;
		ms_crossFadeEnabled = false;
	}
}

// ----------------------------------------------------------------------

void DetailAppearance::setForceLowDetailLevels (bool forceLowDetailLevels)
{
	ms_forceLowDetailLevels = forceLowDetailLevels;
}

//-------------------------------------------------------------------

bool DetailAppearance::getForceLowDetailLevels ()
{
	return ms_forceLowDetailLevels;
}

//-------------------------------------------------------------------

void DetailAppearance::setForceHighDetailLevels (bool forceHighDetailLevels)
{
	ms_forceHighDetailLevels = forceHighDetailLevels;
}

//-------------------------------------------------------------------

bool DetailAppearance::getForceHighDetailLevels ()
{
	return ms_forceHighDetailLevels;
}

//-------------------------------------------------------------------

void DetailAppearance::setDetailLevelBias (float detailLevelBias)
{
	ms_detailLevelBias = detailLevelBias;
}

//-------------------------------------------------------------------

float DetailAppearance::getDetailLevelBias ()
{
	return ms_detailLevelBias;
}

// ----------------------------------------------------------------------

void DetailAppearance::setDrawRadarShape (bool drawRadarShape)
{
	ms_drawRadarShape = drawRadarShape;
}

//-------------------------------------------------------------------

bool DetailAppearance::getDrawRadarShape ()
{
	return ms_drawRadarShape;
}

//-------------------------------------------------------------------

void DetailAppearance::setDrawTestShape (bool drawTestShape)
{
	ms_drawTestShape = drawTestShape;
}

//-------------------------------------------------------------------

bool DetailAppearance::getDrawTestShape ()
{
	return ms_drawTestShape;
}

//-------------------------------------------------------------------

void DetailAppearance::setDrawWriteShape (bool drawWriteShape)
{
	ms_drawWriteShape = drawWriteShape;
}

//-------------------------------------------------------------------

bool DetailAppearance::getDrawWriteShape ()
{
	return ms_drawWriteShape;
}

//-------------------------------------------------------------------

bool DetailAppearance::getFadeInEnabled()
{
	return ms_fadeInEnabled;
}

//-------------------------------------------------------------------

void DetailAppearance::setFadeInEnabled(bool enabled)
{
	ms_fadeInEnabled = enabled;

	if (Graphics::getShaderCapability() < ShaderCapability(1, 1))
		ms_fadeInEnabled = false;
}

//-------------------------------------------------------------------

bool DetailAppearance::getGlobalCrossFadeEnabled()
{
	return ms_crossFadeEnabled;
}

//-------------------------------------------------------------------

void DetailAppearance::setGlobalCrossFadeEnabled(bool enabled)
{
	ms_crossFadeEnabled = enabled;
	if (Graphics::getShaderCapability() < ShaderCapability(1, 1))
	{
		ms_crossFadeEnabled = false;
	}
}

// ======================================================================

DetailAppearance::DetailAppearance(const DetailAppearanceTemplate *detailAppearanceTemplate)
: Appearance(detailAppearanceTemplate),
	m_appearanceListSize(0),
	m_setTextureTag(TAG_0000),
	m_setTextureTexture(0),
	m_setShadowBlobAllowed(false),
	m_crossFadeEnabled(!detailAppearanceTemplate->getDisableLodCrossFade()),
	m_dpvsObject (NULL),
	m_userControlled(false),
	m_alters(0),
	m_currentDetailLevel(-1),
	m_nextDetailLevel(-1),
	m_customizationData(0),
	m_timer(0.0f)
{
	// create the detail level appearances
	int count = detailAppearanceTemplate->getDetailLevelCount();
	if (count > AppearanceListCapacity)
	{
		DEBUG_WARNING(true, ("DetailAppearanceTemplate [%s]: AB found %i detail levels. We only allow %i -- ignoring the rest.", detailAppearanceTemplate->getName(), count, AppearanceListCapacity));
		count = AppearanceListCapacity;
	}

	m_appearanceListSize = count;
	{
		int i;
		for (i = 0; i < count; ++i)
		{
			m_appearanceList [i] = 0;
			m_setTextureList [i] = false;
			m_setShadowBlobAllowedList[i] = false;
		}
	}

	IGNORE_RETURN (getAppearance (0));
	if (DataLint::isEnabled () || ConfigClientObject::getPreloadDetailLevels ())
	{
		for (int i = 1; i < count; ++i)
			IGNORE_RETURN (getAppearance (i));
	}

	// we need the DPVS object created here so it can be a visibility parent to the child appearance's dpvs objects
	// due to old art assets, we may not have the proper test shape at this point so just use the default one now
	{
		DPVS::Model *const testModel = RenderWorld::fetchDefaultModel();
		m_dpvsObject = RenderWorld::createObject (this, testModel);
		IGNORE_RETURN(testModel->release());
	}

	// now that we've loaded appearance 0, we will have an appropriate test shape, even for old art.
	{
		DPVS::Model *const testModel = fetchDpvsTestShape (detailAppearanceTemplate);
		m_dpvsObject->setTestModel(testModel);
		IGNORE_RETURN(testModel->release());
	}

	// we need the preRender callback to do LOD selection
	m_dpvsObject->set(DPVS::Object::REPORT_IMMEDIATELY, true);

#ifndef _DEBUG
	// in debug builds, DetailAppearance::render() will draw the debug shapes.  In release builds, it does nothing, and we don't even need the render callback.
	m_dpvsObject->set(DPVS::Object::INFORM_VISIBLE, false);
#endif

#ifdef _DEBUG
	if (DataLint::isEnabled ())
		dataLint();
#endif
}

// ----------------------------------------------------------------------

DetailAppearance::~DetailAppearance()
{
	int i;
	for (i = 0; i < m_appearanceListSize; ++i)
	{
		delete m_appearanceList [i];
		m_appearanceList [i] = 0;
	}

	if (m_setTextureTexture)
	{
		m_setTextureTexture->release();
		m_setTextureTexture = 0;
	}

	IGNORE_RETURN(m_dpvsObject->release());
}

// ----------------------------------------------------------------------

inline
bool DetailAppearance::_canCrossFade() const
{
	return m_crossFadeEnabled && ms_crossFadeEnabled;
}

// ----------------------------------------------------------------------

inline const DetailAppearanceTemplate* DetailAppearance::getDetailAppearanceTemplate() const
{
	return safe_cast<const DetailAppearanceTemplate *>(getAppearanceTemplate());
}

// ----------------------------------------------------------------------

bool DetailAppearance::isLoaded() const
{
	return getAppearance(0)->isLoaded();
}

// ----------------------------------------------------------------------

Appearance* DetailAppearance::_getAppearance(int detailLevel)
{
	VALIDATE_RANGE_INCLUSIVE_EXCLUSIVE (0, detailLevel, m_appearanceListSize);

	Appearance *&appearance = m_appearanceList [detailLevel];
	if (!appearance)
	{
		DetailAppearanceTemplate const *const appearanceTemplate = getDetailAppearanceTemplate ();
		NOT_NULL(appearanceTemplate);

		appearance = getDetailAppearanceTemplate()->createAppearance(detailLevel);
		NOT_NULL(appearance);

		Object *const owner = getOwner();
		appearance->setOwner (owner);
		if (owner && owner->isInWorld())
		{
			appearance->addToWorld();
		}

		// set the visibility parent and disable the DPVS object
		DPVS::Object * const dpvsObject = appearance->getDpvsObject();
		if (dpvsObject)
		{
			dpvsObject->setVisibilityParent(m_dpvsObject);
			dpvsObject->set(DPVS::Object::ENABLED, false);
		}

		//-- Set customization data for newly created detail level appearance.
		appearance->setCustomizationData (m_customizationData);

		//-- We only schedule the owner for alter if the owner is already in the AlterScheduler
		if (owner && AlterScheduler::findObject(owner) != 0)
		{
			owner->scheduleForAlter();
		}
		appearance->setScale(getScale());
	}

	return appearance;
}

// ----------------------------------------------------------------------

const Appearance* DetailAppearance::getAppearance (int detailLevel) const
{
	return const_cast<DetailAppearance*>(this)->_getAppearance(detailLevel);
}

// ----------------------------------------------------------------------

int DetailAppearance::_chooseDetailLevel() const
{
	// ignore request when the LOD is user controlled or are there no choices.
	if (m_appearanceListSize == 0)
		return -1;

	// If we're in user-controlled mode, stick with the user's choice.
	if (m_userControlled)
	{
		return m_currentDetailLevel;
	}

	// handle forced low lod
	if (ms_forceLowDetailLevels)
	{
		const Appearance *appearance = getAppearance(0);
		if (appearance->isLoaded())
		{
			return 0;
		}
		return -1;
	}

	const DetailAppearanceTemplate *at = getDetailAppearanceTemplate();
	const int numberOfDetailLevels = at->getDetailLevelCount();

	// handle forced high lod
	if (ms_forceHighDetailLevels)
	{
		const Appearance *appearance = getAppearance(numberOfDetailLevels  - 1);
		if (appearance->isLoaded())
		{
			return numberOfDetailLevels - 1;
		}
		return -1;
	}

	// get the distance from the camera to the object
	const Camera *camera = &ShaderPrimitiveSorter::getCurrentCamera();
	const float distance = camera->getPosition_w().magnitudeBetween(at->getUsePivotPoint() ? getTransform_w().getPosition_p() : getTransform_w ().rotateTranslate_l2p (getSphere().getCenter()));

	// search from where we currently are
	int detailLevel = m_currentDetailLevel;
	if (detailLevel < 0)
	{
		detailLevel = 0;
	}

	int direction = 0;
	for (;;)
	{
		// get the near and far distances for this detail level
		float nearDist = at->getNearDistance(detailLevel);
		float farDist = at->getFarDistance(detailLevel);
		if (ms_enableDetailLevelStretch)
		{
			farDist += ConfigClientObject::getDetailLevelStretch ();
		}
		nearDist *= ms_detailLevelBias;
		farDist *= ms_detailLevelBias;

		if (distance > farDist)
		{
			// Object is too far from the camera for this level, go down a level
			if (direction == 1)
			{
				// Goldilocks problem - the last detail level was too close, this one's too far away
				DEBUG_WARNING(true, ("DetailAppearance::chooseDetailLevel3 - No suitable detail level for appearance [%s] at distance %f", at->getName(),distance));
				break;
			}

			// If we've gone off the end of our detail levels, use the most- or least-detailed one
			if (--detailLevel < 0)
			{
				detailLevel = 0;
				break;
			}
			direction = -1;
		}
		else if (distance < nearDist)
		{
			// Object is too close to the camera for this level, go up a level
			if (direction == -1)
			{
				// Goldilocks problem - the last detail level was too far away, this one's too close
				DEBUG_WARNING(true, ("DetailAppearance::chooseDetailLevel3 - No suitable detail level for appearance [%s] at distance %f", at->getName(),distance));
				break;
			}

			if (++detailLevel >= numberOfDetailLevels)
			{
				detailLevel = numberOfDetailLevels - 1;
				break;
			}
			direction = 1;
		}
		else
		{
			// Object can use this detail level
			break;
		}
	}

	// get the appearance.  if not loaded, this will request it to be loaded
	const Appearance *appearance = getAppearance(detailLevel);
	if (!appearance->isLoaded())
	{
		appearance = NULL;

		// it wasn't loaded, so now try to find one that is.  search from the current detail level, both higher and lower, looking for something loaded
		const int numberOfDetailLevels = getDetailAppearanceTemplate()->getDetailLevelCount();
		for (int offset = 1; offset < numberOfDetailLevels; ++offset)
		{
			if (detailLevel - offset >= 0)
			{
				appearance = m_appearanceList [detailLevel - offset];
				if (appearance && appearance->isLoaded())
				{
					detailLevel -= offset;
					break;
				}
				appearance = NULL;
			}

			if (detailLevel + offset < numberOfDetailLevels)
			{
				appearance = m_appearanceList [detailLevel + offset];
				if (appearance && appearance->isLoaded())
				{
					detailLevel += offset;
					break;
				}
				appearance = NULL;
			}
		}
	}

	// If we were unable to find an appearance, return -1 to
	// indicate failure.
	if (appearance == NULL)
	{
		detailLevel = -1;
	}

	return detailLevel;
}

// ----------------------------------------------------------------------

DPVS::Object *DetailAppearance::getDpvsObject() const
{
	return m_dpvsObject;
} //lint !e1763 // (Info -- Member function 'DetailAppearance::getDpvsObject(void) const' marked as const indirectly modifies class) // Intentional.

// ----------------------------------------------------------------------

void DetailAppearance::addToWorld()
{
	Appearance::addToWorld();

	// re-fade in appearances when they have changed owners.  this is common when we cache old appearances to be used later.
	m_timer = 0.0f;
	m_currentDetailLevel = -1;
	m_dpvsObject->setWriteModel(NULL);

	for (int i = 0; i < m_appearanceListSize; ++i)
	{
		Appearance *appearance = m_appearanceList [i];
		if (appearance)
		{
			appearance->addToWorld();
			DPVS::Object *dpvsObject = appearance->getDpvsObject();
			if (dpvsObject)
				dpvsObject->set(DPVS::Object::ENABLED, false);
		}
	}
}

// ----------------------------------------------------------------------

void DetailAppearance::removeFromWorld()
{
	Appearance::removeFromWorld();

	for (int i = 0; i < m_appearanceListSize; ++i)
	{
		Appearance *appearance = m_appearanceList [i];
		if (appearance)
		{
			DPVS::Object * const dpvsObject = appearance->getDpvsObject();
			if (dpvsObject)
				dpvsObject->set(DPVS::Object::ENABLED, false);
			appearance->removeFromWorld();
		}
	}

	m_timer = 0.0f;
	m_currentDetailLevel = -1;
}

// ----------------------------------------------------------------------

void DetailAppearance::preRender() const
{
	if (!m_userControlled && !ms_lockLod && m_nextDetailLevel == -1)
	{
		m_nextDetailLevel = _chooseDetailLevel();

		if (m_nextDetailLevel == m_currentDetailLevel)
		{
			m_nextDetailLevel = -1;
		}
		else
		{
			// Begin transition to next detail level.
			m_timer = 0.0f;
			const Object *owner = getOwner();
			if (owner)
			{
				const_cast<Object*>(owner)->scheduleForAlter();
			}
		}
	}

	//--------------------------------------------------
	// transfer our world transform to our active LOD(s)
	const Appearance *lod;
	if (m_currentDetailLevel>=0)
	{
		lod = getAppearance(m_currentDetailLevel);
		if (lod)
		{
			lod->setTransform_w(getTransform_w());
		}
	}

	if (m_nextDetailLevel>=0)
	{
		lod = getAppearance(m_nextDetailLevel);

		if (lod)
		{
			lod->setTransform_w(getTransform_w());
		}
	}
	//--------------------------------------------------
}

// ----------------------------------------------------------------------

void DetailAppearance::objectListCameraRender() const
{
	int const detailLevel = _chooseDetailLevel();
	if (detailLevel >= 0)
	{
		if (m_setTextureTexture && !m_setTextureList[detailLevel])
		{
			m_setTextureList[detailLevel] = true;
			m_appearanceList[detailLevel]->setTexture(m_setTextureTag, *m_setTextureTexture);
		}

		if (m_setShadowBlobAllowed && !m_setShadowBlobAllowedList[detailLevel])
		{
			m_setShadowBlobAllowedList[detailLevel] = true;
			m_appearanceList[detailLevel]->setShadowBlobAllowed();
		}

		const Appearance* appearance = getAppearance(detailLevel);
		appearance->setTransform_w(getTransform_w());

		// @todo @hack this works around a problem where an object rendered by the RenderWorld system might not be visible
		// when also rendered by an object list camera.  if the objects happen to be at the same detail level, all works
		// fine, but if they're at different detail levels, the alpha value may be something other than 1.0, which will cause
		// the object to be faded or completely invisible.  so, here we explicitly set it opaque to make sure it is visible.
		// however, there is a problem with this solution.  the alpha values get updated by alter(), so if the RenderWorld
		// system renders the object after an object list camera renders it, then the alpha will be wrong for that render.
		// however, swg doesn't do this.  if you want to fix this, you'll need to reset this alpha back to the previous value.
		// unfortunately, there's no Appearace::getAlpha() routine, so you'd need to either cache the alpha values,
		// or recalculating the same values again.
		const_cast<Appearance *>(appearance)->setAlpha(false, 0.0f, false, 0.0f);
		appearance->objectListCameraRender();
	}
}

// ----------------------------------------------------------------------

void DetailAppearance::drawDebugShapes (DebugShapeRenderer *const renderer) const
{
	if (!renderer)
		return;

#ifdef _DEBUG

	Appearance::drawDebugShapes(renderer);

	if (ms_drawRadarShape)
	{
		const IndexedTriangleList* radarShape = getDetailAppearanceTemplate ()->getRadarShape ();
		if (radarShape)
		{
			renderer->setColor (VectorArgb (1.f, 0.f, 0.75f, 0.f));
			renderer->drawMesh (radarShape);
		}
	}

	if (ms_drawTestShape)
	{
		const IndexedTriangleList *testShape = getDetailAppearanceTemplate()->getTestShape();
		if (testShape)
		{
			renderer->setColor(VectorArgb::solidBlue);
			renderer->drawMesh(testShape);
		}
	}

	if (ms_drawWriteShape)
	{
		const IndexedTriangleList *writeShape = getDetailAppearanceTemplate()->getWriteShape();
		if (writeShape)
		{
			renderer->setColor(VectorArgb::solidRed);
			renderer->drawMesh(writeShape);
		}
	}

#endif
}

// ----------------------------------------------------------------------

void DetailAppearance::setTexture(const Tag tag, const Texture &texture)
{
	if (m_setTextureTexture)
	{
		m_setTextureTexture->release ();
		m_setTextureTexture = 0;
	}

	m_setTextureTag = tag;
	m_setTextureTexture = &texture;
	m_setTextureTexture->fetch ();

	int i;
	for (i = 0; i < m_appearanceListSize; ++i)
		m_setTextureList [i] = false;

	if (m_currentDetailLevel >= 0)
	{
		m_setTextureList[m_currentDetailLevel] = true;
		m_appearanceList[m_currentDetailLevel]->setTexture(m_setTextureTag, *m_setTextureTexture);
	}
}

// ----------------------------------------------------------------------

float DetailAppearance::alter(float deltaTime)
{
	float alterResult = Appearance::alter(deltaTime);

	// ----------------------------------------------------------
	// alter all the child appearances
	for (int i = 0; i < m_appearanceListSize; ++i)
	{
		Appearance * const appearance = m_appearanceList [i];
		if (appearance)
		{
			float const detailResult = appearance->alter(deltaTime);
			AlterResult::incorporateAlterResult(alterResult, detailResult);
		}
	}
	// ----------------------------------------------------------

	if (m_nextDetailLevel >= 0) // if transitioning to another detail level.
	{
		// we need to keep altering until the fade is complete
		alterResult = AlterResult::cms_alterNextFrame;

		// if we haven't altered in a while, we may be given a large time here. we don't want that, so lower deltaTime to something small.
		if (m_timer == 0.0f)
		{
			deltaTime = 0.00001f;
		}

		_updateLodSettings(m_nextDetailLevel);

		// if the fade is just starting, enable the DPVS object
		// of the next appearance.
		if (m_timer == 0.0f)
		{
			_alterBeginFade();
		}

		if (m_currentDetailLevel < 0) // Fade-in case.
		{
			_alterFadeIn(deltaTime);
		}
		else // Cross-fade case.
		{
			_alterCrossFade(deltaTime);
		}
	}
	else // next detail level is invalid - not transitioning.
	{
		// objects out of view shouldn't fade in
		if (m_currentDetailLevel == -1)
		{
			alterResult = AlterResult::cms_alterNextFrame;

			_alterShowLodZero();
		}
	}

#ifdef _DEBUG
	_checkState();
#endif

	return alterResult;
}

// ----------------------------------------------------------------------

void DetailAppearance::_alterFadeIn(float deltaTime)
{
	Appearance *const nextAppearance = m_appearanceList[m_nextDetailLevel];

	if (ms_fadeInEnabled && m_timer < 1.0f)
	{
		nextAppearance->setAlpha(true, m_timer, true, m_timer);
		m_timer += deltaTime;
	}
	else // no fade-in.  Just switch to next appearance instantly.
	{
		_endAppearanceFade(nextAppearance, true);
		_alterFinishFade();

		// If there is a write-shape for this, start using it.
		_setDpvsWriteShape();
	}
}

// ----------------------------------------------------------------------

void DetailAppearance::_alterCrossFade(float deltaTime)
{
	Appearance *const nextAppearance = m_appearanceList[m_nextDetailLevel];
	Appearance *const currentAppearance = m_appearanceList[m_currentDetailLevel];

	const bool canCrossFade = _canCrossFade();

	if (canCrossFade && m_timer < 0.5f) // cross-fade: first half.
	{
		currentAppearance->setAlpha(false, 1.0f, true, 1.0f - m_timer);
		nextAppearance->setAlpha(true, m_timer * 2.0f, true, m_timer);

		m_timer += deltaTime;
	}
	else if (canCrossFade && m_timer < 1.0f) // cross-fade: second half.
	{
		currentAppearance->setAlpha(true, 1.0f - ((m_timer - 0.5f) * 2.0f), true, 1.0f - m_timer);
		nextAppearance->setAlpha(false, 1.0f, true, m_timer);

		m_timer += deltaTime;
	}
	else // no cross-fading or cross-fade is finished.
	{
		_endAppearanceFade(currentAppearance, false);
		_endAppearanceFade(nextAppearance, true);
		_alterFinishFade();
	}
}

// ----------------------------------------------------------------------

void DetailAppearance::_alterShowLodZero()
{
	if (++m_alters > 2)
	{
		Appearance * a = _getAppearance(0);
		if (a && a->isLoaded())
		{
			m_currentDetailLevel = 0;

			_endAppearanceFade(a, true);
			_updateLodSettings(m_currentDetailLevel);

			_setDpvsWriteShape();
		}
	}
}

// ----------------------------------------------------------------------

void DetailAppearance::_alterBeginFade()
{
	const Appearance *const nextAppearance = m_appearanceList[m_nextDetailLevel];

	DPVS::Object *const dpvsObject = nextAppearance->getDpvsObject();
	if (dpvsObject)
	{
		dpvsObject->set(DPVS::Object::ENABLED, true);
	}
}

// ----------------------------------------------------------------------

void DetailAppearance::_alterFinishFade()
{
	m_currentDetailLevel = m_nextDetailLevel;
	m_nextDetailLevel = -1;
	m_timer = 0.0f;
}

// ----------------------------------------------------------------------

void DetailAppearance::_endAppearanceFade(Appearance *const appearance, bool enabled)
{
	DPVS::Object *const dpvsObject = appearance->getDpvsObject();
	if (dpvsObject)
	{
		dpvsObject->set(DPVS::Object::ENABLED, enabled);
	}
	appearance->setAlpha(false, 0.0f, false, 0.0f);
}

// ----------------------------------------------------------------------

void DetailAppearance::_updateLodSettings(int lodIndex)
{
	if (m_setTextureTexture && !m_setTextureList[lodIndex])
	{
		m_setTextureList[lodIndex] = true;
		m_appearanceList[lodIndex]->setTexture(m_setTextureTag, *m_setTextureTexture);
	}

	if (m_setShadowBlobAllowed && !m_setShadowBlobAllowedList[lodIndex])
	{
		m_setShadowBlobAllowedList[lodIndex] = true;
		m_appearanceList[lodIndex]->setShadowBlobAllowed();
	}
}

// ----------------------------------------------------------------------

void DetailAppearance::_setDpvsWriteShape()
{
	DPVS::Model* dpvsWriteShape = getDetailAppearanceTemplate()->getDpvsWriteShape();
	if (dpvsWriteShape)
		m_dpvsObject->setWriteModel(dpvsWriteShape);
}

// ----------------------------------------------------------------------

#ifdef _DEBUG
void DetailAppearance::_checkState()
{
	for (int i = 0; i < m_appearanceListSize; ++i)
	{
		Appearance *appearance = m_appearanceList[i];
		if (appearance)
		{
			DPVS::Object * const dpvsObject = appearance->getDpvsObject();
			if (dpvsObject)
			{
				bool enabled = dpvsObject->test(DPVS::Object::ENABLED);
				DEBUG_FATAL(enabled && i != m_currentDetailLevel && i != m_nextDetailLevel, ("Incorrect detail level enabled %d != %d | %d", i, m_currentDetailLevel, m_nextDetailLevel));
				DEBUG_FATAL(!enabled && (i == m_currentDetailLevel || i == m_nextDetailLevel), ("Proper detail level not enabled %d = %d | %d", i, m_currentDetailLevel, m_nextDetailLevel));
				DEBUG_FATAL(i == m_currentDetailLevel && !appearance->isLoaded(), ("current detail level isn't loaded"));
				DEBUG_FATAL(i == m_nextDetailLevel && !appearance->isLoaded(), ("next detail level isn't loaded"));
			}
		}
		else
		{
			DEBUG_FATAL(i == m_currentDetailLevel, ("current detail level doesn't exist"));
			DEBUG_FATAL(i == m_nextDetailLevel, ("next detail level doesn't exist"));
		}
	}
}
#endif

// ----------------------------------------------------------------------

void DetailAppearance::setCustomizationData (CustomizationData *customizationData)
{
	//-- Keep track of the customization data.  We'll need it later if we haven't
	//   already created all the detail level appearances.
	m_customizationData = customizationData;

	//-- Set customization data for all constructed detail levels.
	for (int i = 0; i < m_appearanceListSize; ++i)
	{
		Appearance *appearance = m_appearanceList [i];
		if (appearance)
			appearance->setCustomizationData (customizationData);
	}
}

// ----------------------------------------------------------------------

void DetailAppearance::addCustomizationVariables (CustomizationData &customizationData) const
{
	//-- Add customization variables from the most detailed LOD appearance.  Note this
	//   function is typically called by tools, not during the game, so do not worry
	//   about efficiency here.
	const Appearance *const appearance = getAppearance (0);
	if (appearance)
		appearance->addCustomizationVariables (customizationData);
	else
		DEBUG_WARNING(true, ("getDetailLevel(0) returned NULL."));
}

// ----------------------------------------------------------------------

void DetailAppearance::setOwner(Object *newOwner)
{
	Appearance::setOwner(newOwner);
	for (int i = 0; i < m_appearanceListSize; ++i)
	{
		Appearance *const appearance = m_appearanceList [i];
		if (appearance)
			appearance->setOwner(newOwner);
	}
}

// ----------------------------------------------------------------------

int DetailAppearance::getDetailLevelCount() const
{
	return getDetailAppearanceTemplate()->getDetailLevelCount();
}

// ----------------------------------------------------------------------

void DetailAppearance::incrementDetailLevel()
{
	if (m_userControlled && m_nextDetailLevel == -1 && m_currentDetailLevel+1 < m_appearanceListSize)
		m_nextDetailLevel = m_currentDetailLevel + 1;
}

// ----------------------------------------------------------------------

void DetailAppearance::decrementDetailLevel()
{
	if (m_userControlled && m_nextDetailLevel == -1 && m_currentDetailLevel > 0)
		m_nextDetailLevel = m_currentDetailLevel - 1;
}

// ----------------------------------------------------------------------

bool DetailAppearance::collide(Vector const & start_o, Vector const & end_o, CollideParameters const & collideParameters, CollisionInfo & result) const
{
	//-- Collide w/ extent only
	if (collideParameters.getQuality() == CollideParameters::Q_low)
		return getAppearance(0)->collide(start_o, end_o, collideParameters, result);

	//-- Collide w/ low detail level only
	if (collideParameters.getQuality() == CollideParameters::Q_medium)
		return getAppearance(0)->collide(start_o, end_o, collideParameters, result);

	//-- Collide w/ currently rendered detail level
	if (m_currentDetailLevel == -1)
		return false;

	if (collideParameters.getQuality() == CollideParameters::Q_high)
		return getAppearance(m_currentDetailLevel)->collide(start_o, end_o, collideParameters, result);

	return false;
}

// ----------------------------------------------------------------------

bool DetailAppearance::implementsCollide() const
{
	return true;
}

// ----------------------------------------------------------------------

const Extent *DetailAppearance::getExtent() const
{
	// All DetailAppearance instances share the same extent information stored within the AppearanceTemplate.
	AppearanceTemplate const *const appearanceTemplate = getAppearanceTemplate ();
	NOT_NULL (appearanceTemplate);

	return appearanceTemplate->getExtent ();
}

// ----------------------------------------------------------------------

AxialBox const DetailAppearance::getTangibleExtent() const
{
	if (m_currentDetailLevel != -1)
	{
		Appearance const * const appearance = getAppearance(m_currentDetailLevel);
		if (appearance)
			return appearance->getTangibleExtent();
	}

	return AxialBox();
}

// ----------------------------------------------------------------------

void DetailAppearance::onScaleModified(Vector const &oldScale, Vector const &newScale)
{
	//-- Chain down.
	Appearance::onScaleModified(oldScale, newScale);

	// set the scale for each lod
	for (int i = 0; i < m_appearanceListSize; ++i)
	{
		Appearance *appearance = m_appearanceList [i];
		if (appearance)
		{
			appearance->setScale(newScale);
		}
	}
}

// ======================================================================

#ifdef _DEBUG

int DetailAppearance::getPolygonCount () const
{
	int total = 0;

	for (int i = 0; i < m_appearanceListSize; ++i)
		if (m_appearanceList [i])
			total += m_appearanceList [i]->getPolygonCount ();

	return total;
}

static void indent (std::string &result, const int indentLevel)
{
	int i;
	for (i = 0; i < indentLevel; ++i)
		result += "  ";
}

void DetailAppearance::debugDump (std::string &result, const int indentLevel) const
{
	char buffer [1024];

	sprintf (buffer, "DetailAppearance %s\r\n", getAppearanceTemplate ()->getName ());
	indent (result, indentLevel);
	result += buffer;

	sprintf (buffer, "%i lods\r\n", m_appearanceListSize);
	indent (result, indentLevel);
	result += buffer;

	const DetailAppearanceTemplate* const detailAppearanceTemplate = getDetailAppearanceTemplate ();

	int i;
	for (i = 0; i < m_appearanceListSize; ++i)
	{
		sprintf (buffer, "lod %i [programmers=%i], distance=%1.2f\r\n", (m_appearanceListSize - 1) - i, i, detailAppearanceTemplate->getFarDistance (static_cast<int>(i)));
		indent (result, indentLevel + 1);
		result += buffer;

		const IndexedTriangleList *radarShape = getDetailAppearanceTemplate()->getRadarShape();
		if (radarShape)
		{
			sprintf (buffer, "radar shape: %i vertices, %i triangles\r\n", radarShape->getVertices ().size (), radarShape->getIndices ().size () / 3);
			indent (result, indentLevel + 2);
			result += buffer;
		}
		else
		{
			sprintf (buffer, "no radar shape\r\n");
			indent (result, indentLevel + 2);
			result += buffer;
		}

		const IndexedTriangleList *testShape = getDetailAppearanceTemplate()->getTestShape();
		if (testShape)
		{
			sprintf (buffer, "test shape: %i vertices, %i triangles\r\n", testShape->getVertices ().size (), testShape->getIndices ().size () / 3);
			indent (result, indentLevel + 2);
			result += buffer;
		}
		else
		{
			sprintf (buffer, "no test shape\r\n");
			indent (result, indentLevel + 2);
			result += buffer;
		}

		const IndexedTriangleList *writeShape = getDetailAppearanceTemplate()->getWriteShape();
		if (writeShape)
		{
			sprintf (buffer, "write shape: %i vertices, %i triangles\r\n", writeShape->getVertices ().size (), writeShape->getIndices ().size () / 3);
			indent (result, indentLevel + 2);
			result += buffer;
		}
		else
		{
			sprintf (buffer, "no write shape\r\n");
			indent (result, indentLevel + 2);
			result += buffer;
		}

		const Appearance* const appearance = m_appearanceList [i];
		if (appearance)
			appearance->debugDump (result, indentLevel + 3);
	}
}

#endif

// ----------------------------------------------------------------------

DPVS::Model* DetailAppearance::fetchDpvsTestShape (DetailAppearanceTemplate const *appearanceTemplate)
{
	// Nothing to do without an appearance template.
	if (!appearanceTemplate)
		return 0;

	//-- Get the appearanceTemplate's model.
	DPVS::Model* testShape = appearanceTemplate->fetchDpvsTestShape ();
	if (testShape)
	{
		return testShape;
	}

	// No DetailAppearanceTemplate test shape exists

	//-- Get the L0 extent.  We are guaranteed that L0 is already loaded at this time.
	//   In the event that testShape returns NULL, we force L0 to load synchronously
	//   for getAppearance (0).
	Appearance *const appearance = _getAppearance(0);
	NOT_NULL (appearance);

	//-- Ensure the extent is a box extent.
	BoxExtent const * boxExtent = safe_cast<BoxExtent const *> (appearance->getExtent ());
	if (!boxExtent)
	{
		DEBUG_WARNING (true, ("DetailAppearance::fetchDpvsTestShape: [%s] boxExtent is NULL, using default box", appearanceTemplate->getName ()));
		boxExtent = new BoxExtent (AxialBox (-Vector::xyz111, Vector::xyz111));
		setExtent (ExtentList::fetch (boxExtent));
	}

	//-- Set DetailAppearanceTemplate's extent to this extent, fetching a new reference to it.
	const_cast<DetailAppearanceTemplate*> (appearanceTemplate)->setExtent (ExtentList::fetch (boxExtent));

	//-- Create the test shape model.
	DPVS::Model *const newDpvsModel = RenderWorld::fetchBoxModel(boxExtent->getBox());

	//-- Tell the appearance template about the model so we don't have to do this again.
	const_cast<DetailAppearanceTemplate*>(appearanceTemplate)->setDpvsTestShape(newDpvsModel);

	return newDpvsModel;
}

// ----------------------------------------------------------------------

#ifdef _DEBUG

// these are down here to prevent accidental dependencies on these headers
#include "clientObject/MeshAppearance.h"
#include "clientGraphics/ShaderPrimitiveSet.h"

const int cms_maximumNumberOfDetailLevels = 5;

void DetailAppearance::dataLint() const
{
	const char* const appearanceTemplateName = getAppearanceTemplate ()->getCrcName ().getString ();

	int const numberOfDetailLevels = getDetailAppearanceTemplate()->getDetailLevelCount();
	DEBUG_WARNING(numberOfDetailLevels == 1, ("DetailAppearanceTemplate [%s]: AB only one detail level found. Is this intentional?", appearanceTemplateName));
	DEBUG_WARNING(numberOfDetailLevels > cms_maximumNumberOfDetailLevels, ("DetailAppearanceTemplate [%s]: AB too many detail levels found (%d) > %d", appearanceTemplateName, numberOfDetailLevels, cms_maximumNumberOfDetailLevels));
	DEBUG_WARNING(numberOfDetailLevels > 1 && dynamic_cast<MeshAppearance const *>(getAppearance(0)) == NULL && dynamic_cast<NoRenderAppearance const *>(getAppearance(0)) == NULL, ("DetailAppearanceTemplate [%s]: AB multiple levels of detail found, but last detail level is not a mesh or norender", appearanceTemplateName));

	{
		for (int i = 0; i < numberOfDetailLevels-2; ++i)
		{
			MeshAppearance const * m0 = dynamic_cast<MeshAppearance const *>(getAppearance(i));
			MeshAppearance const * m1 = dynamic_cast<MeshAppearance const *>(getAppearance(i+2));
			if (m0 && m1)
			{
				ShaderPrimitiveSet const * s0 = m0->getShaderPrimitiveSet();
				ShaderPrimitiveSet const * s1 = m1->getShaderPrimitiveSet();

				int const n0 = s0->getNumberOfShaders();
				int const n1 = s1->getNumberOfShaders();

				int v0 = 0;
				int t0 = 0;
				float c0 = 0.0f;

				s0->getCostEstimate(v0, t0, c0);

				int v1 = 0;
				int t1 = 0;
				float c1 = 0.0f;
				s1->getCostEstimate(v1, t1, c1);

				// show them in artist ways
				const int a0 = numberOfDetailLevels-(1+i);
				const int a1 = numberOfDetailLevels-(1+i+2);
				DEBUG_WARNING(n1 > 1 && n0 > ((n1 * 2) / 3), ("DetailAppearanceTemplate [%s]: AB Insufficient shader reduction between level %d(%d) and level %d(%d)", appearanceTemplateName, a1, n1, a0, n0));
				DEBUG_WARNING(t1 > 1 && t0 > ((t1 * 2) / 3), ("DetailAppearanceTemplate [%s]: AB Insufficient triangle reduction between level %d(%d) and level %d(%d)", appearanceTemplateName, a1, t1, a0, t0));
			}
		}
	}

	{
		for (int i = 0; i < numberOfDetailLevels - 1; ++i)
		{
			Appearance const * const appearance0 = getAppearance (i);
			Appearance const * const appearance1 = getAppearance (i + 1);
			if (appearance0 && appearance1)
			{
				int const polygonCount0 = appearance0->getPolygonCount ();
				int const polygonCount1 = appearance1->getPolygonCount ();

				const int detailLevel0 = numberOfDetailLevels - (1 + i);
				const int detailLevel1 = numberOfDetailLevels - (1 + i + 1);
				DEBUG_WARNING (polygonCount0 >= polygonCount1, ("DetailAppearanceTemplate [%s]: AB Detail level %d(%d) has more polygons than level %d(%d)", appearanceTemplateName, detailLevel0, polygonCount0, detailLevel1, polygonCount1));
			}
		}
	}

	//-- verify write shape doesn't have two dimensions smaller than one meter
	AxialBox writeBox;
	bool writeBoxValid = false;
	if (getDetailAppearanceTemplate ()->getWriteShape ())
	{
		const IndexedTriangleList* const indexedTriangleList = getDetailAppearanceTemplate ()->getWriteShape ();
		if (indexedTriangleList)
		{
			const std::vector<Vector> vertices = indexedTriangleList->getVertices ();

			writeBox.clear ();

			for (uint k = 0; k < vertices.size (); ++k)
				writeBox.add (vertices [k]);

			if ((writeBox.getWidth () <= 1.f && writeBox.getHeight () <= 1.f) ||
				(writeBox.getHeight () <= 1.f && writeBox.getDepth () <= 1.f) ||
				(writeBox.getDepth () <= 1.f && writeBox.getWidth () <= 1.f))
			{
				DEBUG_WARNING (true, ("DetailAppearanceTemplate [%s]: AB Write shape has at least 2 dimensions smaller than one meter.  Does this appearance really need a write shape?", appearanceTemplateName));
			}

			writeBoxValid = true;
		}
	}

	//-- verify test shape is not smaller than the extents by a large factor
	AxialBox testBox;
	bool testBoxValid = false;
	if (getDetailAppearanceTemplate ()->getTestShape ())
	{
		const IndexedTriangleList* const indexedTriangleList = getDetailAppearanceTemplate ()->getTestShape ();
		if (indexedTriangleList)
		{
			const std::vector<Vector> vertices = indexedTriangleList->getVertices ();

			testBox.clear ();

			for (uint k = 0; k < vertices.size (); ++k)
				testBox.add (vertices [k]);

			const BoxExtent* const boxExtent = dynamic_cast<const BoxExtent*> (getExtent ());
			if (boxExtent)
				DEBUG_WARNING (!testBox.contains (boxExtent->getBox ()), ("DetailAppearanceTemplate [%s]: AB Appearance extents do not fit inside the test shape.  Is the test shape too small?", appearanceTemplateName));
			else
				DEBUG_WARNING (true, ("DetailAppearanceTemplate [%s]: PB Appearance does not have a box extent", appearanceTemplateName));

			if ((testBox.getWidth () <= 1.f && testBox.getHeight () <= 1.f) ||
				(testBox.getHeight () <= 1.f && testBox.getDepth () <= 1.f) ||
				(testBox.getDepth () <= 1.f && testBox.getWidth () <= 1.f))
			{
				DEBUG_WARNING (true, ("DetailAppearanceTemplate [%s]: AB Test shape has at least 2 dimensions smaller than one meter.  Does this appearance really need a test shape?", appearanceTemplateName));
			}

			testBoxValid = true;
		}
	}
	else
	{
		const BoxExtent* const boxExtent = dynamic_cast<const BoxExtent*> (getExtent ());
		if (boxExtent)
		{
			testBox = boxExtent->getBox ();

			testBoxValid = true;
		}
		else
			DEBUG_WARNING (true, ("DetailAppearanceTemplate [%s]: PB Appearance does not have a box extent", appearanceTemplateName));
	}

	//-- verify write shape fits inside test shape
	if (writeBoxValid && testBoxValid)
		DEBUG_WARNING (!testBox.contains (writeBox), ("DetailAppearanceTemplate [%s]: AB Write shape does not fit within the test shape", appearanceTemplateName));

	// verify all child appearance extents fit within this detail appearances' extent
	{
		BoxExtent const *const myExtent = safe_cast<BoxExtent const *> (getExtent ());
		if (myExtent)
		{
			AxialBox const myBox = myExtent->getBox();
			for (int i = 0; i < numberOfDetailLevels; ++i)
			{
				BoxExtent const *childExtent = dynamic_cast<BoxExtent const *>(getAppearance (i)->getExtent());

				if (childExtent)
				{
					AxialBox const childBox = childExtent->getBox();
					if (!myBox.contains(childBox))
						DEBUG_WARNING(true, ("DetailAppearanceTemplate [%s]: AB Detail level %i has extents that don't fit within its parent's extents.  This asset needs to be re-exported.", appearanceTemplateName, numberOfDetailLevels - 1 - i));
				}
			}
		}
	}
}

#endif

// ----------------------------------------------------------------------

const IndexedTriangleList* DetailAppearance::getRadarShape () const
{
	return getDetailAppearanceTemplate ()->getRadarShape ();
}

// ----------------------------------------------------------------------

DetailAppearance * DetailAppearance::asDetailAppearance()
{
	return this;
}

// ----------------------------------------------------------------------

DetailAppearance const * DetailAppearance::asDetailAppearance() const
{
	return this;
}

// ----------------------------------------------------------------------

void DetailAppearance::setShadowBlobAllowed()
{
	Appearance::setShadowBlobAllowed();

	m_setShadowBlobAllowed = true;
}

// ======================================================================

