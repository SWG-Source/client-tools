// ======================================================================
//
// FadingTextAppearance.cpp
// Copyright 2001, Sony Online Entertainment Inc.
// All Rights Reserved.
//
// ======================================================================

#include "clientGame/FirstClientGame.h"
#include "clientGame/FadingTextAppearance.h"

//----------------------------------------------------------------------

#include "UITypes.h"
#include "clientGame/Game.h"
#include "clientGraphics/Camera.h"
#include "clientGraphics/RenderWorld.h"
#include "clientGraphics/ShaderPrimitiveSorter.h"
#include "clientUserInterface/CuiChatBubbleManager.h"
#include "clientUserInterface/CuiObjectTextManager.h"
#include "clientUserInterface/CuiTextManager.h"
#include "clientUserInterface/CuiUtils.h"
#include "sharedCollision/Extent.h"
#include "sharedObject/AlterResult.h"

#include "dpvsObject.hpp"
#include "dpvsModel.hpp"

// ======================================================================

bool FadingTextAppearance::ms_enabled = true;

//----------------------------------------------------------------------

FadingTextAppearance::FadingTextAppearance(const Unicode::String & str, const VectorArgb & color, float startingOpacity, float fadeOutTime, float sizeModifier, int textType, bool unbounded)
: TextAppearance            (),
	m_string                (str),
	m_opacityFactor         (1.f - startingOpacity),
	m_falloffPerTime        (fadeOutTime == 0.0f ? 0.0f : (startingOpacity / fadeOutTime)),
	m_color                 (new UIColor (CuiUtils::convertColor (color))),
	m_sizeModifier          (sizeModifier),
	m_sphere                (),
	m_dpvsObject            (0),
	m_fadeEnabled           (true),
	m_textType              (textType),
	m_renderWithChatBubbles (true),
	m_screenSpaceLocation   (),
	m_screenSpaceVelocity   (),
	m_speedModifier(1.0f),
	m_useScreenSpace        (false)
{
	{
		DPVS::Model * const testModel = RenderWorld::fetchDefaultModel();
		m_dpvsObject = RenderWorld::createObject(this, testModel);
		IGNORE_RETURN(testModel->release());
	}

	// set unbounded to true so that it will render if not attached to an object
	if (unbounded)
	{
		setUnBounded(true);
	}
}

FadingTextAppearance::FadingTextAppearance(const Unicode::String & str, const VectorArgb & color, float startingOpacity, float fadeOutTime, float sizeModifier, int textType, Vector screenSpaceLocation, Vector screenSpaceVelocity, float speedModifier)
: TextAppearance            (),
	m_string                (str),
	m_opacityFactor         (1.f - startingOpacity),
	m_falloffPerTime        (fadeOutTime == 0.0f ? 0.0f : (startingOpacity / fadeOutTime)),
	m_color                 (new UIColor (CuiUtils::convertColor (color))),
	m_sizeModifier          (sizeModifier),
	m_sphere                (),
	m_dpvsObject            (0),
	m_fadeEnabled           (true),
	m_textType              (textType),
	m_renderWithChatBubbles (true),
	m_screenSpaceLocation   (screenSpaceLocation),
	m_screenSpaceVelocity   (screenSpaceVelocity),
	m_speedModifier(speedModifier),
	m_useScreenSpace        (true)
{
	{
		DPVS::Model * const testModel = RenderWorld::fetchDefaultModel();
		m_dpvsObject = RenderWorld::createObject(this, testModel);
		IGNORE_RETURN(testModel->release());
	}
	setUnBounded(true);
}

// ----------------------------------------------------------------------

FadingTextAppearance::~FadingTextAppearance ()
{
	delete m_color;
	m_color = 0;

	IGNORE_RETURN(m_dpvsObject->release());
	m_dpvsObject = 0;
}

// ----------------------------------------------------------------------

DPVS::Object *FadingTextAppearance::getDpvsObject() const
{
	return m_dpvsObject;
} //lint !e1763 // Const member indirectly modifies class // Logical const-ness is maintained.

// ----------------------------------------------------------------------

void  FadingTextAppearance::render() const
{
	if (!ms_enabled)
		return;

	CuiTextManager::TextEnqueueInfo info;
	info.textWeight = static_cast<CuiTextManager::TextEnqueueInfo::TextWeight>(m_textType);
	info.textSize   = m_sizeModifier;
	info.worldDistance = 1.0f;

	if (m_useScreenSpace)
	{
		info.screenVect.x = m_screenSpaceLocation.x;
		info.screenVect.y = m_screenSpaceLocation.y;
		info.screenVect.z = m_screenSpaceLocation.z;
	}
	else
	{
		const Camera * const camera = &ShaderPrimitiveSorter::getCurrentCamera();
		if (!camera)
			return;

		const Object * const object = getOwner();
		if (!object)
			return;

		const Object * const parent = object->getParent ();

		info.id = parent ? parent->getNetworkId () : object->getNetworkId();

		if ((!m_renderWithChatBubbles) && (CuiChatBubbleManager::objectHasChatBubblesVisible (info.id)))
			return;

		const Vector & position = object->getPosition_w();

		// @todo build a sphere and test the sphere, don't just ignore side culling
		if (camera->projectInWorldSpace(position, &info.screenVect.x, &info.screenVect.y, &info.screenVect.z, false))
		{
			if (m_fadeEnabled)
			{
				info.worldDistance = camera->getPosition_w().magnitudeBetween(position);
			}

			if (parent)
			{
				int lastFrameOffset  = 0;
				CuiObjectTextManager::getObjectHeadPointOffset (info.id, &lastFrameOffset);

				UIPoint parentHeadPt;
				if (CuiObjectTextManager::getObjectHeadPoint     (*parent, *camera, 0.0f, parentHeadPt))
				{
					info.screenVect.y  = std::min (static_cast<float>(parentHeadPt.y + lastFrameOffset), info.screenVect.y);
				}
			}
		}
	}

	const float approximateTextSize = 10.f * info.textSize;

	// keep the text on the screen vertically
	info.screenVect.y = info.screenVect.y < approximateTextSize ? approximateTextSize : info.screenVect.y;

	info.opacity           = 1.f - (m_opacityFactor * m_opacityFactor);
	info.backgroundOpacity = 0.0f;
	info.textColor         = *m_color;
	info.updateOffset      = false;

	CuiTextManager::enqueueText(m_string, info);
}

// ----------------------------------------------------------------------

float FadingTextAppearance::alter(float deltaTime)
{
	if (m_fadeEnabled)
	{
		m_opacityFactor += m_falloffPerTime * deltaTime;
	}

	if (m_useScreenSpace)
	{
		//m_speedModifier *= m_speedModifier;
		m_screenSpaceVelocity *= m_speedModifier;
		m_screenSpaceLocation += m_screenSpaceVelocity * deltaTime;
	}

	return AlterResult::cms_alterNextFrame;
}

// ----------------------------------------------------------------------

const Sphere & FadingTextAppearance::getSphere() const
{
	return m_sphere;
}

// ----------------------------------------------------------------------

void FadingTextAppearance::setUnBounded(const bool unBounded)
{
	m_dpvsObject->set(DPVS::Object::UNBOUNDED, unBounded);
}

// ----------------------------------------------------------------------

void FadingTextAppearance::setText(const char* const text)
{
	setText (Unicode::narrowToWide (text));
}

// ----------------------------------------------------------------------

void FadingTextAppearance::setText(const Unicode::String & str)
{
	m_string = str;
}

// ----------------------------------------------------------------------

void FadingTextAppearance::setFadeEnabled(const bool fadeEnabled)
{
	m_fadeEnabled = fadeEnabled;
}

// ----------------------------------------------------------------------

void FadingTextAppearance::setRenderWithChatBubbles (bool b)
{
	m_renderWithChatBubbles = b;
}

// ----------------------------------------------------------------------

void FadingTextAppearance::setColor(const VectorArgb & color)
{
	m_color->Set(CuiUtils::convertColor(color));
}

//----------------------------------------------------------------------

void FadingTextAppearance::setEnabled   (bool b)
{
	ms_enabled = b;
}

// ======================================================================
