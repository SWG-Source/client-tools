// ============================================================================
//
// Sound2.cpp
// Copyright Sony Online Entertainment
//
// ============================================================================

#include "clientAudio/FirstClientAudio.h"
#include "clientAudio/Sound2.h"

#include "clientAudio/SoundTemplate.h"
#include "clientAudio/SoundTemplateList.h"
#include "sharedDebug/DebugFlags.h"
#include "sharedFoundation/CrcString.h"
#include "sharedFoundation/ExitChain.h"
#include "sharedMath/VectorArgb.h"
#include "sharedObject/Appearance.h"
#include "sharedObject/CellProperty.h"
#include "sharedObject/PortalProperty.h"
#include "sharedObject/TextAppearance.h"

#include <vector>

// ============================================================================
//
// SoundNamespace
//
// ============================================================================

namespace SoundNamespace
{
	Sound2::CreateTextAppearanceObjectFunction ms_createTextAppearanceObjectFunction = NULL;
	Sound2::DrawCircleFunction ms_drawCircleFunction = NULL;

	void remove();

	bool s_installed;
	bool s_warnOnMissingHardpoints;
}

using namespace SoundNamespace;

// ======================================================================

void SoundNamespace::remove()
{
	DEBUG_FATAL(!s_installed, ("Sound2 not installed."));
	s_installed = false;

	DebugFlags::unregisterFlag(s_warnOnMissingHardpoints);
}

// ============================================================================
//
// Sound2
//
// ============================================================================

void Sound2::install()
{
	DEBUG_FATAL(s_installed, ("Sound2 already installed."));

	DebugFlags::registerFlag(s_warnOnMissingHardpoints, "ClientAudio", "warnOnMissingHardpoints");

	s_installed = true;
	ExitChain::add(SoundNamespace::remove, "Sound2");
}

//-----------------------------------------------------------------------------
Sound2::Sound2(SoundTemplate const *soundTemplate, SoundId const &soundId)
 : m_template(SoundTemplateList::fetch(soundTemplate))
 , m_soundId(soundId)
 , m_autoDelete(true)
 , m_callBack(NULL)
 , m_sampleIndex(-1)
 , m_loopCount(0)
 , m_position_w(Vector::zero)
 , m_wasParented(false)
 , m_object(NULL)
 , m_hardPointName()
 , m_samplePreviousTime(0.0f)
 , m_positionSpecified(false)
 , m_parentCell(NULL)
 , m_userVolume(1.0f)
 , m_userPitchDelta(0.0f)
 , m_obstructionPercent(0.0f)
 , m_occlusionPercent(0.0f)
#ifdef _DEBUG
 , m_locationTextObject(NULL)
 , m_appearance(NULL)
#endif // _DEBUG
{
}

//-----------------------------------------------------------------------------
Sound2::~Sound2()
{
	if (m_template)
	{
		SoundTemplateList::release(m_template);
		m_template = NULL;
	}

	m_callBack = NULL;
	m_parentCell = NULL;

#ifdef _DEBUG
	delete m_locationTextObject;
	m_locationTextObject = NULL;
#endif // _DEBUG
}

//-----------------------------------------------------------------------------
void Sound2::setCreateTextAppearanceObjectFunction(CreateTextAppearanceObjectFunction createTextAppearanceObjectFunction)
{
	ms_createTextAppearanceObjectFunction = createTextAppearanceObjectFunction;
}

//-----------------------------------------------------------------------------
void Sound2::setDrawCircleFunction(DrawCircleFunction drawCircleFunction)
{
	ms_drawCircleFunction = drawCircleFunction;
}

//-----------------------------------------------------------------------------
SoundTemplate const *Sound2::getTemplate() const
{
	return m_template;
}

//-----------------------------------------------------------------------------
void Sound2::setAutoDelete(bool const autoDelete)
{
	m_autoDelete = autoDelete;
}

//-----------------------------------------------------------------------------
void Sound2::setEndOfSampleCallBack(EndOfSampleCallBack callBack)
{
	m_callBack = callBack;
}

//-----------------------------------------------------------------------------
SoundId const &Sound2::getSoundId() const
{
	return m_soundId;
}

//-----------------------------------------------------------------------------
void Sound2::setSampleId(SampleId const &sampleId)
{
	m_sampleId = sampleId;
}

//-----------------------------------------------------------------------------
CrcString const *Sound2::getSamplePath() const
{
	DEBUG_WARNING(m_sampleIndex < 0, ("Invalid sample index: %d", m_sampleIndex));
	DEBUG_WARNING(m_sampleIndex >= static_cast<int>(getTemplate()->getSampleList().size()), ("Invalid sample index: %d", m_sampleIndex));

	CrcString const * result = NULL;

	if ((m_sampleIndex >= 0) &&
	    (m_sampleIndex < static_cast<int>(getTemplate()->getSampleList().size())))
	{
		result = getTemplate()->getSampleList()[static_cast<unsigned int>(m_sampleIndex)];
	}

	return result;
}

//-----------------------------------------------------------------------------
SampleId const &Sound2::getSampleId() const
{
	return m_sampleId;
}

//-----------------------------------------------------------------------------
void Sound2::setPosition_w(Vector const &position_w)
{
	m_positionSpecified = true;
	m_position_w = position_w;
}

//-----------------------------------------------------------------------------
void Sound2::setObject(Object const *object)
{
	DEBUG_WARNING((object == NULL), ("Setting a NULL object on a sound."));

	m_positionSpecified = true;
	m_object = object;
	m_wasParented = (object != NULL);

	DEBUG_WARNING((object != NULL) && (object->getParentCell() == NULL), ("Attaching a sound to an object with a NULL getParentCell(). object: %s", object->getObjectTemplateName()));

	if (object == NULL)
	{
		m_hardPointName.clear();
	}
}

//-----------------------------------------------------------------------------
void Sound2::setHardPointName(char const * const hardPointName)
{
	if (hardPointName != NULL)
	{
		m_hardPointName.set(hardPointName, true);
	}
	else
	{
		m_hardPointName.clear();
	}
}

//-----------------------------------------------------------------------------
Object const * const Sound2::getObject() const
{
	return m_object.getPointer();
}

//-----------------------------------------------------------------------------
int Sound2::getLoopCount() const
{
	return m_loopCount;
}

//-----------------------------------------------------------------------------
bool Sound2::isInfiniteLooping() const
{
	return (m_loopCount == -1);
}

//-----------------------------------------------------------------------------
bool Sound2::isPositionSpecified() const
{
	return m_positionSpecified;
}

//-----------------------------------------------------------------------------
Vector Sound2::getPosition_w() const
{
	if (!isPositionSpecified())
	{
		// The sound is global so the position is the same as the listener

		m_position_w = Audio::getListenerPosition();
	}
	else if (m_object != NULL)
	{
		//-- Don't look up a hardpoint position if we have a zero-length hardpoint name
		//   OR if the appearance is currently asynchronously loading.  Asynchronously loaded
		//   appearances don't have hardpoint info available until finished loading.
		if (m_hardPointName.isEmpty() || ((m_object->getAppearance() == NULL) || !m_object->getAppearance()->isLoaded()))
		{
			m_position_w = m_object->getPosition_w();
		}
		else
		{
			Transform hardpointTransform;

			if (m_object->getAppearance()->findHardpoint(m_hardPointName, hardpointTransform))
			{
				m_position_w = m_object->rotateTranslate_o2w(hardpointTransform.getPosition_p());
			}
			else
			{
				m_position_w = m_object->getPosition_w();

				DEBUG_WARNING(s_warnOnMissingHardpoints, ("Unable to find hardpoint [%s] on object id=[%s],template=[%s]", m_hardPointName.getString(), m_object->getNetworkId().getValueString().c_str(), m_object->getObjectTemplateName()));
			}
		}
	}

	return m_position_w;
}

//-----------------------------------------------------------------------------
void Sound2::kill(float const fadeOutTime)
{
	stop(fadeOutTime);
}

//-----------------------------------------------------------------------------
void Sound2::setParentCell(CellProperty const *parentCell)
{
	DEBUG_WARNING((m_object != NULL), ("Setting the cell property on a sound that already has an object defined: %s", getTemplate()->getName()));

	m_parentCell = parentCell;
}

//-----------------------------------------------------------------------------
CellProperty const * const Sound2::getParentCell() const
{
	if (m_object != NULL)
	{
		return m_object->getParentCell();
	}

	return m_parentCell;
}

//-----------------------------------------------------------------------------
bool Sound2::isObstructed() const
{
	bool result = false;

	if ((getTemplate()->getAttenuationMethod() != Audio::AM_none) &&
	    (getParentCell() != NULL) &&
	    (Audio::getListener() != NULL) &&
	    (Audio::getListener()->getParentCell() != NULL))
	{
		if (getParentCell() != Audio::getListener()->getParentCell())
		{
			if (getParentCell()->getPortalProperty() == Audio::getListener()->getParentCell()->getPortalProperty())
			{
				// Obstructed

				result = true;
			}
		}
	}

	return result;
}

//-----------------------------------------------------------------------------
bool Sound2::isOccluded() const
{
	bool result = false;

	if ((getTemplate()->getAttenuationMethod() != Audio::AM_none) &&
	    (getParentCell() != NULL) &&
	    (Audio::getListener() != NULL) &&
	    (Audio::getListener()->getParentCell() != NULL))
	{
		if (getParentCell() != Audio::getListener()->getParentCell())
		{
			if (getParentCell()->getPortalProperty() != Audio::getListener()->getParentCell()->getPortalProperty())
			{
				// Occluded

				result = true;
			}
		}
	}

	return result;
}

//-----------------------------------------------------------------------------
float Sound2::getOcclusion() const
{
	return m_occlusionPercent * Audio::getOcclusion();
}

//-----------------------------------------------------------------------------
float Sound2::getObstruction() const
{
	return m_obstructionPercent * Audio::getObstruction();
}

//-----------------------------------------------------------------------------
float Sound2::getOcclusionPercent() const
{
	return m_occlusionPercent;
}

//-----------------------------------------------------------------------------
float Sound2::getObstructionPercent() const
{
	return m_obstructionPercent;
}

//-----------------------------------------------------------------------------
void Sound2::setUserVolume(float const volume)
{
	m_userVolume = volume;
}

//-----------------------------------------------------------------------------
float Sound2::getUserVolume() const
{
	return m_userVolume;
}

//-----------------------------------------------------------------------------
void Sound2::setUserPitchDelta(float const pitchDelta)
{
	m_userPitchDelta = pitchDelta;
}

//-----------------------------------------------------------------------------
float Sound2::getUserPitchDelta() const
{
	return m_userPitchDelta;
}

#ifdef _DEBUG
//-----------------------------------------------------------------------------
void Sound2::setText(char const * const string, VectorArgb const &color)
{
	if (Audio::isDebugEnabled())
	{
		if (m_positionSpecified &&
			(m_locationTextObject == NULL))
		{
			addDebugText(m_object, getPosition_w());
		}

		if (m_locationTextObject != NULL)
		{
			float const distanceFromListener = getDistanceFromListener();
			float const innerRadius = getTemplate()->getDistanceAtMaxVolume();
			float const outerRadius = getDistanceAtVolumeCutOff();
			int const segments = 64;
			Vector position_w(getPosition_w().x, getPosition_w().y + 0.75f, getPosition_w().z);
			VectorArgb const &circleColor = (distanceFromListener <= innerRadius) ? VectorArgb::solidWhite : color;

			if (ms_drawCircleFunction != NULL)
			{
				ms_drawCircleFunction(position_w, Vector::unitY, innerRadius * 0.99f, segments, circleColor);
				ms_drawCircleFunction(position_w, Vector::unitY, innerRadius, segments, circleColor);
				ms_drawCircleFunction(position_w, Vector::unitY, outerRadius, segments, circleColor);
			}

			m_appearance->setText(string);
			m_appearance->setColor(circleColor);
		}
	}
}

//-----------------------------------------------------------------------------
void Sound2::addDebugText(Object const *object, Vector const &position)
{
	if (Audio::isDebugEnabled() &&
	    !Audio::isToolApplication())
	{
		if (ms_createTextAppearanceObjectFunction)
		{
			VectorArgb const color(VectorArgb::solidRed);
			float const sizeModifier = 0.6f;
			m_locationTextObject = ms_createTextAppearanceObjectFunction("", color, sizeModifier);
			m_appearance = safe_cast<TextAppearance*> (m_locationTextObject->getAppearance());

			if (object != NULL)
			{
				(const_cast<Object *>(object))->addChildObject_o(m_locationTextObject);
				m_locationTextObject->move_o(Vector::unitY * 0.15f);
			}
			else
			{
				m_locationTextObject->setPosition_w(position);
			}
		}
		else
		{
			// Trying to create sound object appearances but the callback function is not hooked up

			DEBUG_REPORT_LOG(true, ("Trying to create sound object appearances but the callback function is not hooked up.\n"));
		}
	}
}
#endif // _DEBUG

// ============================================================================
