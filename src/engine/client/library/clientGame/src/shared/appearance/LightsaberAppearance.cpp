// ======================================================================
//
// LightsaberAppearance.cpp
// Copyright 2003 Sony Online Entertainment, Inc.
// All Rights Reserved.
//
// ======================================================================

#include "clientGame/FirstClientGame.h"
#include "clientGame/LightsaberAppearance.h"

#include "clientAudio/Audio.h"
#include "clientGame/ClientEventManager.h"
#include "clientGame/ClientGameAppearanceEvents.h"
#include "clientGame/ConfigClientGame.h"
#include "clientGame/ContainerInterface.h"
#include "clientGame/GameLight.h"
#include "clientGame/LightsaberAppearanceTemplate.h"
#include "clientGame/LightsaberCollisionManager.h"
#include "clientGraphics/RenderWorld.h"
#include "clientGraphics/ShaderTemplate.h"
#include "clientGraphics/ShaderTemplateList.h"
#include "clientObject/BeamAppearance.h"
#include "clientObject/HardpointObject.h"
#include "clientSkeletalAnimation/SkeletalAppearance2.h"
#include "sharedFoundation/ConstCharCrcLowerString.h"
#include "sharedFoundation/ExitChain.h"
#include "sharedFoundation/MemoryBlockManager.h"
#include "sharedFoundation/PointerDeleter.h"
#include "sharedMath/Segment3d.h"
#include "sharedMath/Transform.h"
#include "sharedMath/Vector.h"
#include "sharedObject/AlterResult.h"
#include "sharedObject/CustomizationData.h"
#include "sharedObject/CustomizationDataProperty.h"
#include "sharedObject/MemoryBlockManagedObject.h"
#include "sharedObject/Object.h"
#include "sharedObject/PaletteColorCustomizationVariable.h"

#include "dpvsModel.hpp"
#include "dpvsObject.hpp"

#include <algorithm>
#include <string>

// ======================================================================

namespace LightsaberAppearanceNamespace
{
	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

	ConstCharCrcLowerString const cs_weaponHardpointName("hold_r");

	ConstCharCrcLowerString const cs_bladeBaseHardpointNames[] =
		{
			ConstCharCrcLowerString("tr1s"),
			ConstCharCrcLowerString("tr2s"),
			ConstCharCrcLowerString("tr3s")
		};
	int const cs_bladeBaseHardpointNameCount = isizeof(cs_bladeBaseHardpointNames) / isizeof(cs_bladeBaseHardpointNames[0]);

	ConstCharCrcLowerString const cs_bladeMidpointHardpointNames[] =
		{
			ConstCharCrcLowerString("tr1m"),
			ConstCharCrcLowerString("tr2m"),
			ConstCharCrcLowerString("tr3m")
		};
	int const cs_bladeMidpointHardpointNameCount = isizeof(cs_bladeMidpointHardpointNames) / isizeof(cs_bladeMidpointHardpointNames[0]);

	ConstCharCrcLowerString const cs_bladeTipHardpointNames[] =
		{
			ConstCharCrcLowerString("tr1e"),
			ConstCharCrcLowerString("tr2e"),
			ConstCharCrcLowerString("tr3e")
		};
	int const cs_bladeTipHardpointNameCount = isizeof(cs_bladeTipHardpointNames) / isizeof(cs_bladeTipHardpointNames[0]);

	ConstCharCrcLowerString const cs_bladeOnEventName("bladeon");
	ConstCharCrcLowerString const cs_bladeOffEventName("bladeoff");

	std::string const             cs_bladeColorVariableName("/private/index_color_blade");
	std::string const             cs_bladeShaderVariableName("/private/alternate_shader_blade");

	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

	Transform  s_transform;

	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

}

using namespace LightsaberAppearanceNamespace;

// ======================================================================

MEMORY_BLOCK_MANAGER_IMPLEMENTATION_WITH_INSTALL(LightsaberAppearance, true, 0, 0, 0);

// ======================================================================
// class LightsaberAppearance: PUBLIC STATIC
// ======================================================================

CrcString const &LightsaberAppearance::getBaseHardpointName(int bladeIndex)
{
	VALIDATE_RANGE_INCLUSIVE_EXCLUSIVE(0, bladeIndex, cs_bladeBaseHardpointNameCount);
	return cs_bladeBaseHardpointNames[bladeIndex];
}

// ----------------------------------------------------------------------

CrcString const &LightsaberAppearance::getMidpointHardpointName(int bladeIndex)
{
	VALIDATE_RANGE_INCLUSIVE_EXCLUSIVE(0, bladeIndex, cs_bladeMidpointHardpointNameCount);
	return cs_bladeMidpointHardpointNames[bladeIndex];
}

// ----------------------------------------------------------------------

CrcString const &LightsaberAppearance::getTipHardpointName(int bladeIndex)
{
	VALIDATE_RANGE_INCLUSIVE_EXCLUSIVE(0, bladeIndex, cs_bladeTipHardpointNameCount);
	return cs_bladeTipHardpointNames[bladeIndex];
}

// ----------------------------------------------------------------------

void LightsaberAppearance::handleCustomizationModificationCallback(const CustomizationData &customizationData, const void *context)
{
	NOT_NULL(context);

	//-- convert context to LightsaberAppearance instance
	LightsaberAppearance *const app = const_cast<LightsaberAppearance*>(static_cast<const LightsaberAppearance*>(context));

	//-- call the instance handler
	app->handleCustomizationModification(customizationData);
}


// ======================================================================
// class LightsaberAppearance: PUBLIC
// ======================================================================

LightsaberAppearance::~LightsaberAppearance()
{
	//-- Ensure lightsaber blade collision manager is no longer tracking this object.
	int const bladeCount = getBladeCount();
	for (int bladeIndex = 0; bladeIndex < bladeCount; ++bladeIndex)
		LightsaberCollisionManager::removeLightsaberBlade(*this, bladeIndex);

	delete m_hiltObject;
	m_hiltAppearance = 0;

	// Since appearances and objects are children, these get cleaned up for us
	// when our owner is destroyed.
	m_bladeObjects.clear();
	m_bladeAppearances.clear();
	
	IGNORE_RETURN(m_dpvsObject->release());
	m_dpvsObject = 0;

	if (m_customizationData)
	{
		m_customizationData->deregisterModificationListener(handleCustomizationModificationCallback, this);

		m_customizationData->release();
		m_customizationData = 0;
	}
}

// ----------------------------------------------------------------------

bool LightsaberAppearance::collide(const Vector &start_o, const Vector &end_o, CollideParameters const & collideParameters, CollisionInfo& result) const
{
	// @todo determine if we want to support collision with the blades.  If so, implement --- BeamAppearance
	//       does not support collision detection.

	//-- Check for collision with lightsaber base.
	NOT_NULL(m_hiltAppearance);
	return m_hiltAppearance->collide(start_o, end_o, collideParameters, result);
}

// ----------------------------------------------------------------------

const Extent *LightsaberAppearance::getExtent() const
{
	// @todo revisit, may no longer need to calculate blade extent.
#if 0
	NOT_NULL(m_hiltAppearance);
	return m_hiltAppearance->getExtent();
#else
	//-- Fill up extent with current info.  Only needs to be updated if growing or shrinking.
	if (m_extentUpdateRequired)
	{
		// Set box extent to base appearance's extents.
		NOT_NULL(m_hiltAppearance);
		Extent const *const hiltExtent = m_hiltAppearance->getExtent();
		if (hiltExtent)
			m_boxExtent.copy(hiltExtent);
		else
		{
			// Initialize extent to small box.
			m_boxExtent.setMin(-0.10f * Vector::xyz111);
			m_boxExtent.setMax(0.10f * Vector::xyz111);
		}

		// Grow box extent for blades, apply transforms to extent.
		LightsaberAppearanceTemplate const &appearanceTemplate = getLightsaberAppearanceTemplate();
		int const                           bladeCount         = appearanceTemplate.getBladeCount();
		VALIDATE_RANGE_INCLUSIVE_INCLUSIVE(0, bladeCount, static_cast<int>(m_bladeLengths.size()));
		VALIDATE_RANGE_INCLUSIVE_INCLUSIVE(0, bladeCount, static_cast<int>(m_bladeObjects.size()));

		for (int i = 0; i < bladeCount; ++i)
		{
			//-- Get blade length.
			float const bladeLength = m_bladeLengths[static_cast<FloatVector::size_type>(i)];
			if (bladeLength > 0.0f)
			{
				//-- Get transform for blade.
				Transform const &bladeTransform = NON_NULL(m_bladeObjects[static_cast<ObjectVector::size_type>(i)])->getTransform_o2p();

				//-- Get blade width.
				float const bladeWidth = appearanceTemplate.getBladeWidth(i);

				//-- Update extent info for this blade.
				m_boxExtent.updateMinAndMax(bladeTransform.rotateTranslate_l2p(Vector(-bladeWidth, -bladeWidth, 0.0f)));
				m_boxExtent.updateMinAndMax(bladeTransform.rotateTranslate_l2p(Vector( bladeWidth,  bladeWidth, 0.0f)));

				m_boxExtent.updateMinAndMax(bladeTransform.rotateTranslate_l2p(Vector(-bladeWidth, -bladeWidth, bladeLength)));
				m_boxExtent.updateMinAndMax(bladeTransform.rotateTranslate_l2p(Vector( bladeWidth,  bladeWidth, bladeLength)));
			}
		}

		//-- Update center and radius if box extent is modified.
		m_boxExtent.calculateCenterAndRadius();

		//-- Extent update no longer required until a new event causes it to be needed.
		//   Exception: if there's no hit extent, it is asynchronously loading.  We'll
		//   continue to calculate extent until hilt extent is fixed up.
		if (hiltExtent)
			m_extentUpdateRequired = false;

		// Tell game that the extent just changed.
		Object *owner = const_cast<Object*>(getOwner());
		if (owner)
			owner->extentChanged();

		// Fix up DPVS test object.
		{
			DPVS::Model *const testModel = RenderWorld::fetchBoxModel(m_boxExtent.getBox());
			NOT_NULL(testModel);
			m_dpvsObject->setTestModel(testModel);
			IGNORE_RETURN(testModel->release());
		}
	}

	return &m_boxExtent;
#endif
}

// ----------------------------------------------------------------------

float LightsaberAppearance::alter(float elapsedTime)
{
	//-- Alter the base class.
	float finalAlterResult = Appearance::alter(elapsedTime);

	//-- This class wants to alter instances every frame.
	AlterResult::incorporateAlterResult(finalAlterResult, AlterResult::cms_alterNextFrame);

	//-- Alter the base appearance.
	NOT_NULL(m_hiltAppearance);
	// gets altered by child relationship.
#if 0
	float alterResult = m_hiltAppearance->alter(elapsedTime);
	AlterResult::incorporateAlterResult(finalAlterResult, alterResult);
#endif

	//-- Update blade debounce timer
	if(m_debounceBladeTimer > 0.0f)
	{
		m_debounceBladeTimer -= elapsedTime;
		if(m_debounceBladeTimer <= 0.0f)
			turnBladeOffInternal();
	}

	//-- Update blade lengths.
	LightsaberAppearanceTemplate const &appearanceTemplate = getLightsaberAppearanceTemplate();
	int   failedHardpointLookupCount = 0;

	int const bladeCount = appearanceTemplate.getBladeCount();
	for (int i = 0; i < bladeCount; ++i)
	{
		//-- Handle first-time hardpoint lookup.
		if (m_doObjectHardpointLookup)
		{
			Transform  blade_o2p(Transform::IF_none);
			bool const found = m_hiltAppearance->findHardpoint(cs_bladeBaseHardpointNames[i], blade_o2p);
			if (!found)
				++failedHardpointLookupCount;
			else
			{
				// Get the blade object.
				VALIDATE_RANGE_INCLUSIVE_EXCLUSIVE(0, i, static_cast<int>(m_bladeObjects.size()));
				Object *const bladeObject = m_bladeObjects[static_cast<ObjectVector::size_type>(i)];
				NOT_NULL(bladeObject);

				// Set its object to parent transform.
				bladeObject->setTransform_o2p(blade_o2p);
			}
		}

		BladeState  newState = BS_off;

		// Get the blade's new length and state.
		float const newLength = getBladeLength(i, elapsedTime, newState);

		// Set the blade's new length.
		VALIDATE_RANGE_INCLUSIVE_EXCLUSIVE(0, i, static_cast<int>(m_bladeLengths.size()));
		m_bladeLengths[static_cast<FloatVector::size_type>(i)] = newLength;

		// Set the blade's new state.
		VALIDATE_RANGE_INCLUSIVE_EXCLUSIVE(0, i, static_cast<int>(m_bladeStates.size()));

		BladeState const oldState = m_bladeStates[static_cast<BladeStateVector::size_type>(i)];
		m_bladeStates[static_cast<BladeStateVector::size_type>(i)] = newState;

		// Extents must be updated if old state and new state for a given blade have changed or if we're transitioning.
		if ((oldState != newState) || (newState == BS_transitioningOn) || (newState == BS_transitioningOff))
		{
			//-- We need an extent update any time blade status changes.
			m_extentUpdateRequired = true;
		}

		// Get the blade appearance.
		VALIDATE_RANGE_INCLUSIVE_EXCLUSIVE(0, i, static_cast<int>(m_bladeAppearances.size()));
		BeamAppearance *const appearance = m_bladeAppearances[static_cast<BeamAppearanceVector::size_type>(i)];
		NOT_NULL(appearance);

		// Set the blade appearance's length.
		appearance->setLength(newLength);

		// gets altered by child relationship.
#if 0
		// Alter the blade appearance.
		alterResult = appearance->alter(elapsedTime);
		AlterResult::incorporateAlterResult(finalAlterResult, alterResult);
#endif
	}

	//-- Track whether we need to keep trying to do blade object hardpoint lookups within base appearance.
	m_doObjectHardpointLookup = (failedHardpointLookupCount > 0);
		
	return finalAlterResult;
}

// ----------------------------------------------------------------------

void LightsaberAppearance::setOwner(Object * const newOwner)
{
	if (newOwner)
	{
		//-- Handle lightsaber base.
		NOT_NULL(m_hiltObject);
		m_hiltObject->attachToObject_p(newOwner, true);

		//-- Handle blades.
		ObjectVector::iterator const endIt = m_bladeObjects.end();
		for (ObjectVector::iterator it = m_bladeObjects.begin(); it != endIt; ++it)
		{
			Object * const bladeObject = *it;
			NOT_NULL(bladeObject);
			bladeObject->attachToObject_p(newOwner, true); // attach as child so blades get alter calls for things like animating shaders.
		}
	}
	else
	{
		m_hiltObject->detachFromObject(Object::DF_none);

		//-- Handle blades.
		ObjectVector::iterator const endIt = m_bladeObjects.end();
		for (ObjectVector::iterator it = m_bladeObjects.begin(); it != endIt; ++it)
		{
			Object * const bladeObject = *it;
			NOT_NULL(bladeObject);
			bladeObject->detachFromObject(Object::DF_none);
		}
	}

	//-- Handle base class.
	Appearance::setOwner(newOwner);
}

// ----------------------------------------------------------------------

void LightsaberAppearance::setCustomizationData(CustomizationData *customizationData)
{
	//-- Pass customization data on to all child appearances.
	// Handle base.
	NOT_NULL(m_hiltAppearance);
	m_hiltAppearance->setCustomizationData(customizationData);

	// Handle blades.
	BeamAppearanceVector::iterator const endIt = m_bladeAppearances.end();
	for (BeamAppearanceVector::iterator it = m_bladeAppearances.begin(); it != endIt; ++it)
	{
		BeamAppearance *const appearance = *it;
		NOT_NULL(appearance);
		appearance->setCustomizationData(customizationData);
	}


	//-- check for assignment of same CustomizationData instance.
	if (customizationData != m_customizationData)
	{
		//-- release modification callback set on old CustomizationData
		if (m_customizationData)
		{
			m_customizationData->deregisterModificationListener(handleCustomizationModificationCallback, this);

			//-- release local reference
			m_customizationData->release();
		}

		//-- assign new customization data
		m_customizationData = customizationData;

		//-- attach modification callback on new CustomizationData
		if (m_customizationData)
		{
			m_customizationData->registerModificationListener(handleCustomizationModificationCallback, this);

			//-- handle any customizations as any variable in the new CustomizationData may contain a value different than what we have.
			handleCustomizationModification(*m_customizationData);

			// fetch local reference
			m_customizationData->fetch();
		}
	}
}

// ----------------------------------------------------------------------

void LightsaberAppearance::addCustomizationVariables(CustomizationData &customizationData) const
{
	//-- Get customization data from all child appearances.
	// Handle base.
	NOT_NULL(m_hiltAppearance);
	m_hiltAppearance->addCustomizationVariables(customizationData);

	// Handle blades.
	BeamAppearanceVector::const_iterator const endIt = m_bladeAppearances.end();
	for (BeamAppearanceVector::const_iterator it = m_bladeAppearances.begin(); it != endIt; ++it)
	{
		BeamAppearance const *const appearance = *it;
		NOT_NULL(appearance);
		appearance->addCustomizationVariables(customizationData);
	}
}

// ----------------------------------------------------------------------

void LightsaberAppearance::handleCustomizationModification(const CustomizationData &customizationData)
{
	//-- sanity check: the given customizationData should be the same as our internal m_customizationData.
	DEBUG_FATAL(&customizationData != m_customizationData, ("LightsaberAppearance instance is notified of a CustomizationData change not associated with this LightsaberAppearance instance"));
	UNREF(customizationData);
	NOT_NULL(m_customizationData);

	RangedIntCustomizationVariable const *const shaderVariable = dynamic_cast<RangedIntCustomizationVariable const *>(customizationData.findConstVariable(cs_bladeShaderVariableName));

	if (shaderVariable)
	{
		int alternateShader = shaderVariable->getValue();
		
		if (alternateShader != m_alternateShader)
		{

			BeamAppearanceVector::iterator const endIt = m_bladeAppearances.end();
			int bladeNum = 0;
			for (BeamAppearanceVector::iterator it = m_bladeAppearances.begin(); it != endIt; ++it, ++bladeNum)
			{
				BeamAppearance *const appearance = *it;
				NOT_NULL(appearance);

				if(alternateShader == 0)
					appearance->useBaseShader();
				else
					appearance->useAlternateShader(alternateShader);
			}

			m_alternateShader = alternateShader;

		}
	}
}

// ----------------------------------------------------------------------

bool LightsaberAppearance::findHardpoint(CrcString const &hardpointName, Transform &hardpointTransform) const
{
	//-- Check if hardpoint name is any of our blade tip hardpoint names.
	LightsaberAppearanceTemplate const &appearanceTemplate = getLightsaberAppearanceTemplate();
	int const bladeCount = appearanceTemplate.getBladeCount();

	VALIDATE_RANGE_INCLUSIVE_INCLUSIVE(0, bladeCount, cs_bladeBaseHardpointNameCount);
	VALIDATE_RANGE_INCLUSIVE_INCLUSIVE(0, bladeCount, cs_bladeTipHardpointNameCount);
	VALIDATE_RANGE_INCLUSIVE_INCLUSIVE(0, bladeCount, static_cast<int>(m_bladeAppearances.size()));

	//-- Check for hardpoint tips.
	{
		for (int i = 0; i < bladeCount; ++i)
		{
			bool  wantThisTip      = false;
			bool  wantThisMidpoint = false;

			if (hardpointName == cs_bladeTipHardpointNames[i])
				wantThisTip = true;
			else if (hardpointName == cs_bladeMidpointHardpointNames[i])
				wantThisMidpoint = true;

			//-- Check if hardpoint is the tip or midpoint of blade 1.
			if (wantThisTip || wantThisMidpoint)
			{
				VALIDATE_RANGE_INCLUSIVE_EXCLUSIVE(0, i, static_cast<int>(m_bladeObjects.size()));
				Transform const &baseTransform = NON_NULL(m_bladeObjects[static_cast<ObjectVector::size_type>(i)])->getTransform_o2p();

				//-- Move end blade transform along +z axis of base hardpoint for blade's length.
				VALIDATE_RANGE_INCLUSIVE_EXCLUSIVE(0, i, static_cast<int>(m_bladeLengths.size()));
				Transform lengthTransform;

				float const length = m_bladeLengths[static_cast<FloatVector::size_type>(i)] * (wantThisMidpoint ? 0.5f : 1.0f);
				lengthTransform.setPosition_p(Vector(0.0f, 0.0f, length));

				//-- Return end hardpoint.
				hardpointTransform.multiply(baseTransform, lengthTransform);
				return true;
			}
		}
	}

	//-- Check if hardpoint is part of hilt appearance.
	NOT_NULL(m_hiltAppearance);
	if (m_hiltAppearance->findHardpoint(hardpointName, hardpointTransform))
		return true;

	//-- Pass along to blade appearances.
	{
		for (int i = 0; i < bladeCount; ++i)
		{
			//-- Check if blade i's appearamce knows about this hardpoint.
			Appearance const *const bladeAppearance = m_bladeAppearances[static_cast<BeamAppearanceVector::size_type>(i)];
			NOT_NULL(bladeAppearance);

			if (bladeAppearance->findHardpoint(hardpointName, hardpointTransform))
				return true;
		}
	}

	//-- Didn't find it.
	return false;
}

// ----------------------------------------------------------------------

void LightsaberAppearance::onEvent(LabelHash::Id eventId)
{
	if (eventId == ClientGameAppearanceEvents::getOnEquippedEnteredCombatEventId())
	{
		// Ensure the sound source for the object is the container Object of the owner Object.  This should be the player Object.
		Object *const lightsaberObject = getOwner();
		Object *const wielderObject    = lightsaberObject ? ContainerInterface::getContainedByObject(*lightsaberObject) : NULL;
		setSoundAndEventTarget(wielderObject);	
		
		// Ensure all blades are on. Turn on any blades that are in the off or transitioning off state.
		int const bladeCount = getBladeCount();
		for (int i = 0; i < bladeCount; ++i)
			turnBladeOn(i, true, false);

		m_debounceBladeTimer = 0.0f;
	}
	else if (eventId == ClientGameAppearanceEvents::getOnEquippedExitedCombatEventId())
	{
		// If we are debouncing the blade, ensure the debouncing timer gets set
		m_debounceBladeTimer = ConfigClientGame::getDebounceLightsaberBlade();
		if(m_debounceBladeTimer <= 0.0f)
			turnBladeOffInternal();
	}
	else if (eventId == ClientGameAppearanceEvents::getOnUnequippedEventId())
	{
		m_debounceBladeTimer = 0.0f;
		turnBladeOffInternal();
	}
	else if (eventId == ClientGameAppearanceEvents::getOnInitializeCopyEquippedInCombatStateEventId())
	{
		// Ensure all blades are on. Skip transitions and sounds, just make the state on.
		int const bladeCount = getBladeCount();
		for (int i = 0; i < bladeCount; ++i)
			turnBladeOn(i, true, true);
		
		m_debounceBladeTimer = 0.0f;
	}
}

// ----------------------------------------------------------------------

void LightsaberAppearance::turnBladeOffInternal()
{
	// Ensure all blades are off.  Turn off any blades that are on or are transitioning on.
	int const bladeCount = getBladeCount();
	for (int i = 0; i < bladeCount; ++i)
		turnBladeOn(i, false, false);

	// Clear the sound and event target.
	setSoundAndEventTarget(NULL);
}

// ----------------------------------------------------------------------

void LightsaberAppearance::setSoundAndEventTarget(Object const *target)
{
	m_soundAndEventTarget = target;
}

// ----------------------------------------------------------------------

int LightsaberAppearance::getBladeCount() const
{
	return getLightsaberAppearanceTemplate().getBladeCount();
}

// ----------------------------------------------------------------------

LightsaberAppearance::BladeState LightsaberAppearance::getBladeState(int bladeNumber) const
{
	VALIDATE_RANGE_INCLUSIVE_EXCLUSIVE(0, bladeNumber, static_cast<int>(m_bladeStates.size()));
	return m_bladeStates[static_cast<BladeStateVector::size_type>(bladeNumber)];
}

// ----------------------------------------------------------------------

void LightsaberAppearance::turnBladeOn(int bladeNumber, bool turnOn, bool skipTransitionAndSound)
{
	VALIDATE_RANGE_INCLUSIVE_EXCLUSIVE(0, bladeNumber, static_cast<int>(m_bladeStates.size()));
	VALIDATE_RANGE_INCLUSIVE_EXCLUSIVE(0, bladeNumber, cs_bladeBaseHardpointNameCount);

	LightsaberAppearanceTemplate const &appearanceTemplate = getLightsaberAppearanceTemplate();

	BladeState const currentState = getBladeState(bladeNumber);

	if (turnOn && (currentState != BS_on) && (currentState != BS_transitioningOn))
	{
		//-- Ensure lightsaber blade exists in the lightsaber collision manager.
		LightsaberCollisionManager::addLightsaberBlade(*this, bladeNumber);

		//-- Set the state.
		m_bladeStates[static_cast<BladeStateVector::size_type>(bladeNumber)] = skipTransitionAndSound ? BS_on : BS_transitioningOn;

		if (bladeNumber == 0)
		{
		//-- Trigger the event.
			if (!skipTransitionAndSound)
			{
#if 1
				if (getOwner())
				{
					bool const success = ClientEventManager::playEvent(cs_bladeOnEventName, getOwner(), cs_bladeBaseHardpointNames[0]);
					UNREF(success);
				}
#else
				if ((bladeNumber == 0) && m_soundAndEventTarget)
				{
					bool const success = ClientEventManager::playEvent(cs_bladeOnEventName, const_cast<Object*>(m_soundAndEventTarget.getPointer()), cs_bladeBaseHardpointNames[0]);
					UNREF(success);
				}
#endif

				//-- Attach ambient sound.
				if (Audio::isEnabled())
				{
					DEBUG_FATAL(m_ambientSoundIsPlaying, ("Logic error: tried to play sound when already playing."));

					if (m_soundAndEventTarget)
					{
						m_ambientSoundId = Audio::attachSound(appearanceTemplate.getAmbientSoundTemplateName().getString(), m_soundAndEventTarget, cs_bladeTipHardpointNames[0].getString());
						m_ambientSoundIsPlaying = true;
					}
				}
			}

			//-- Attach light.
			VectorArgb  bladeColor(VectorArgb::solidRed);

			// Remove light if not NULL.
			if (m_lightWatcher.getPointer() != 0)
			{
				m_lightWatcher.getPointer()->kill();
				m_lightWatcher = 0;
			}

			// Lookup blade color & alternate shader.
			Object *const ownerObject = getOwner();
			if (ownerObject)
			{
				CustomizationDataProperty *const property = dynamic_cast<CustomizationDataProperty*>(ownerObject->getProperty(CustomizationDataProperty::getClassPropertyId()));
				if (property)
				{
					CustomizationData *const customizationData = property->fetchCustomizationData();
					NOT_NULL(customizationData);

					// get blade color customization variable.
					PaletteColorCustomizationVariable const *const variable = dynamic_cast<PaletteColorCustomizationVariable const*>(customizationData->findConstVariable(cs_bladeColorVariableName));
					if (variable)
						bladeColor = VectorArgb(variable->getValueAsColor());
					else
					{
						DEBUG_WARNING(true, ("lightsaber: object template [%s] does not define customization variable [%s] needed to retrieve blade color, dumping customization data:", ownerObject->getObjectTemplateName(), cs_bladeColorVariableName.c_str()));
	#ifdef _DEBUG
						customizationData->debugDump();
	#endif
					}

					// release local reference.
					customizationData->release();
				}
				else
					DEBUG_WARNING(true, ("lightsaber: object template [%s] does not have a CustomizationDataProperty.", ownerObject->getObjectTemplateName()));
			}
			else
				DEBUG_WARNING(true, ("lightsaber: has a null owner object, can't find lightsaber color."));

			// Get creature target object.
			Object *const creatureTarget = const_cast<Object*>(m_soundAndEventTarget.getPointer());
			if (creatureTarget)
			{

				// If we are cloaked, don't display any lighting effects. We don't want to give our position away.
				SkeletalAppearance2* const wielderAppearance = creatureTarget->getAppearance()->asSkeletalAppearance2();
				if(wielderAppearance)
				{
					if(wielderAppearance->getFadeState() == SkeletalAppearance2::FS_hold || wielderAppearance->getFadeState() == SkeletalAppearance2::FS_removed)
						return;
				}

				// Create light, attach as child of creature target.
				GameLight *const light = new GameLight;

				light->setDiffuseColor(bladeColor);
				light->Light::setRange(2.0f);

				// Cause light intensity to falloff as 1/D (real light falls off at 1/D^2)
				light->setConstantAttenuation(0.0f);
				light->setLinearAttenuation(1.0f);
				light->setQuadraticAttenuation(0.0f);

				bool const useFlicker = appearanceTemplate.useLightFlicker();
				light->setFlicker(useFlicker);

				if (useFlicker)
				{
					light->setColor(appearanceTemplate.getFlickerSecondaryColor(), bladeColor);

					float min;
					float max;
					appearanceTemplate.getFlickerRange(min, max);
					light->setRange(min, max);

					appearanceTemplate.getFlickerTime(min, max);
					light->setTime(min, max);

					light->setDayNightAware(appearanceTemplate.getFlickerIsDayNightAware());
				}

				// Attach light to a hardpoint object.  If single-bladed, attach to lightsaber's base of first blade.
				// If multi-bladed, attach to center of lightsaber.
				HardpointObject *const hardpointObject = new HardpointObject((appearanceTemplate.getBladeCount() == 1) ? cs_bladeBaseHardpointNames[0] : cs_weaponHardpointName);
				hardpointObject->addChildObject_o(light);
				creatureTarget->addChildObject_o(hardpointObject);

				m_lightWatcher = hardpointObject;
			} //lint !e429 // light & hardpoint object not freed or returned // owned by light watcher
			else
				WARNING(true, ("lightsaber: object template [%s]: no creature target set.", ownerObject ? ownerObject->getObjectTemplateName() : "<null owner>"));
		}
	}
	else if (!turnOn && (currentState != BS_off) && (currentState != BS_transitioningOff))
	{
		//-- Ensure lightsaber blade gets removed from the lightsaber collision manager.
		LightsaberCollisionManager::removeLightsaberBlade(*this, bladeNumber);

		//-- Set the state.
		m_bladeStates[static_cast<BladeStateVector::size_type>(bladeNumber)] = skipTransitionAndSound ? BS_off : BS_transitioningOff;

		if (bladeNumber == 0)
		{
			//-- Kill the light.
			if (m_lightWatcher)
			{
				m_lightWatcher.getPointer()->kill();
				m_lightWatcher = 0;
			}

			//-- Detach ambient sound.
			if (m_ambientSoundIsPlaying)
			{
				Audio::detachSound(m_ambientSoundId);
				m_ambientSoundIsPlaying = false;
			}

			//-- Trigger the event.
			if (!skipTransitionAndSound)
			{
#if 1
				if (getOwner())
				{
					bool const success = ClientEventManager::playEvent(cs_bladeOffEventName, getOwner(), cs_bladeBaseHardpointNames[0]);
					UNREF(success);
				}
#else
				if (m_soundAndEventTarget)
				{
					bool const success = ClientEventManager::playEvent(cs_bladeOffEventName, const_cast<Object*>(m_soundAndEventTarget.getPointer()), cs_bladeBaseHardpointNames[0]);
					UNREF(success);
				}
#endif
			}
		}
	}
}

// ----------------------------------------------------------------------

Object const *LightsaberAppearance::getWielderObject() const
{
	// The weilder is the object that contains the object associated with the lightsaber.
	//-- Get lightsaber owner.
	Object const *const bladeObject = getOwner();
	if (!bladeObject)
		return NULL;

	//-- Get object containing owner.
	Object const *const bladeWielder = ContainerInterface::getContainedByObject(*bladeObject);
	if (bladeWielder)
	{
		//-- Lightsaber is contained by another object, assume it is the wielder.
		return bladeWielder;
	}
	else
	{
		//-- There is no wielder, return the lightsaber itself.  Most likely it's in the world
		//   during a flying attack.
		return bladeObject;
	}
}

// ----------------------------------------------------------------------
/**
 * Retrieve blade start and end points of specified blade in world space coordinates.
 */

void LightsaberAppearance::getBladeSegment(int bladeNumber, Segment3d &segment) const
{
	//-- Get the wielder, presumably an object in the world.  We need this so we can report
	//   world-relative coordinates.
	Object const     *wielderObject     = getWielderObject();
	Appearance const *wielderAppearance = (wielderObject ? wielderObject->getAppearance() : NULL);
	if (!wielderObject || !wielderAppearance)
	{
		DEBUG_WARNING(true, ("failed to get a wielder object (or its appearance) for lightsaber blade at [0x%p].", this));
		segment.getBegin() = Vector::zero;
		segment.getEnd()   = Vector::zero;
		return;
	}

	//-- Get wielder object-to-world transform.
	Transform const &wielder_o2w = wielderObject->getTransform_o2w();

	//-- Get wielder-relative position for the base of the blade, convert to world space.
	bool const gotBase = wielderAppearance->findHardpoint(getBaseHardpointName(bladeNumber), s_transform);
	segment.getBegin() = (gotBase ? wielder_o2w.rotateTranslate_l2p(s_transform.getPosition_p()) : Vector::zero);

	//-- Get wielder-relative position for the tip of the blade, convert to world space.
	bool const gotTip = wielderAppearance->findHardpoint(getTipHardpointName(bladeNumber), s_transform);
	segment.getEnd()  = (gotTip ? wielder_o2w.rotateTranslate_l2p(s_transform.getPosition_p()) : Vector::zero);
}

// ======================================================================
// class LightsaberAppearance: PRIVATE
// ======================================================================

LightsaberAppearance::LightsaberAppearance(LightsaberAppearanceTemplate const &appearanceTemplate) :
	Appearance(&appearanceTemplate),
	m_hiltObject(new MemoryBlockManagedObject()),
	m_hiltAppearance(NON_NULL(appearanceTemplate.createBaseAppearance())),
	m_ambientSoundId(),
	m_ambientSoundIsPlaying(false),
	m_bladeObjects(static_cast<ObjectVector::size_type>(appearanceTemplate.getBladeCount())),
	m_bladeAppearances(static_cast<BeamAppearanceVector::size_type>(appearanceTemplate.getBladeCount())),
	m_bladeLengths(static_cast<FloatVector::size_type>(appearanceTemplate.getBladeCount())),
	m_bladeStates(static_cast<BladeStateVector::size_type>(appearanceTemplate.getBladeCount()), BS_off),
	m_boxExtent(),
	m_extentUpdateRequired(true),
	m_doObjectHardpointLookup(true),
	m_dpvsObject(0),
	m_soundAndEventTarget(0),
	m_lightWatcher(0),
	m_customizationData(0),
	m_alternateShader(0),
	m_debounceBladeTimer(0.0f)
{
	//-- Create the DPVS object.
	{
		DPVS::Model *const testModel = RenderWorld::fetchDefaultModel();
		m_dpvsObject = RenderWorld::createObject(this, testModel);
		NOT_NULL(m_dpvsObject);
		IGNORE_RETURN(testModel->release());
	}

	//-- Setup hilt object.
	m_hiltObject->setAppearance(m_hiltAppearance);
	{
		DPVS::Object *dpvsObject = m_hiltAppearance->getDpvsObject();
		dpvsObject->setVisibilityParent(m_dpvsObject);
	}
	RenderWorld::addObjectNotifications(*m_hiltObject);

	//-- Handle per-blade setup.
	int const bladeCount = appearanceTemplate.getBladeCount();
	for (int i = 0; i < bladeCount; ++i)
	{
		//-- Create blade appearances.
		Object         *const bladeObject     = new MemoryBlockManagedObject();
		BeamAppearance *const bladeAppearance = NON_NULL(appearanceTemplate.createBladeAppearance(i));

		bladeObject->setAppearance(bladeAppearance);
		RenderWorld::addObjectNotifications(*bladeObject);

		// set the visibility parent and disable the DPVS object
		DPVS::Object *dpvsObject = static_cast<Appearance*>(bladeAppearance)->getDpvsObject();
		dpvsObject->setVisibilityParent(m_dpvsObject);
		// dpvsObject->set(DPVS::Object::ENABLED, false);

		m_bladeObjects[static_cast<ObjectVector::size_type>(i)]             = bladeObject;
		m_bladeAppearances[static_cast<BeamAppearanceVector::size_type>(i)] = bladeAppearance;
	}

	//-- Force an extent update.
	IGNORE_RETURN(LightsaberAppearance::getExtent());
}

// ----------------------------------------------------------------------

LightsaberAppearanceTemplate const &LightsaberAppearance::getLightsaberAppearanceTemplate() const
{
	return *NON_NULL(safe_cast<LightsaberAppearanceTemplate const*>(getAppearanceTemplate()));
}

// ----------------------------------------------------------------------

float LightsaberAppearance::getBladeLength(int bladeNumber, float elapsedTime, BladeState &endState) const
{
	//-- Get the current state for the specified blade.
	LightsaberAppearanceTemplate const &appearanceTemplate = getLightsaberAppearanceTemplate();

	VALIDATE_RANGE_INCLUSIVE_EXCLUSIVE(0, bladeNumber, static_cast<int>(m_bladeStates.size()));
	BladeState const bladeState = m_bladeStates[static_cast<BladeStateVector::size_type>(bladeNumber)];

	//-- Calculate new length and end state based on current state and elapsed time.
	float bladeLength;

	switch (bladeState)
	{
		case BS_off:
			bladeLength = 0.0f;
			endState    = BS_off;
			break;

		case BS_on:
			bladeLength = appearanceTemplate.getBladeLength(bladeNumber);
			endState    = BS_on;
			break;

		case BS_transitioningOff:
			VALIDATE_RANGE_INCLUSIVE_EXCLUSIVE(0, bladeNumber, static_cast<int>(m_bladeLengths.size()));
			bladeLength = std::max(0.0f, m_bladeLengths[static_cast<FloatVector::size_type>(bladeNumber)] - appearanceTemplate.getBladeCloseRate(bladeNumber) * elapsedTime);

			if (bladeLength > 0.0f)
				endState = BS_transitioningOff;
			else
				endState = BS_off;

			break;

		case BS_transitioningOn:
			{
				float const maxBladeLength = appearanceTemplate.getBladeLength(bladeNumber);

				VALIDATE_RANGE_INCLUSIVE_EXCLUSIVE(0, bladeNumber, static_cast<int>(m_bladeLengths.size()));
				bladeLength = std::min(maxBladeLength, m_bladeLengths[static_cast<FloatVector::size_type>(bladeNumber)] + appearanceTemplate.getBladeOpenRate(bladeNumber) * elapsedTime);

				if (bladeLength < maxBladeLength)
					endState = BS_transitioningOn;
				else
					endState = BS_on;
			}
			break;
	
		default:
			DEBUG_FATAL(true, ("unsupported BladeState [%d] set.\n", bladeState));
			endState    = BS_off; //lint !e527 // unreachable // reachable in release.
			bladeLength = 0.0f;
			break;
	}

	return bladeLength;
}

// ----------------------------------------------------------------------

DPVS::Object *LightsaberAppearance::getDpvsObject() const
{
	return m_dpvsObject;
} //lint !e1763 // function indirectly modifies class // required by contract.

// ======================================================================
