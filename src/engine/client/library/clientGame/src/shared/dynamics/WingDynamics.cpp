// ======================================================================
//
// WingDynamics.cpp
// asommers
//
// copyright 2003, sony online entertainment
//
// ======================================================================

#include "clientGame/FirstClientGame.h"
#include "clientGame/WingDynamics.h"

#include "clientAudio/SoundTemplate.h"
#include "clientAudio/SoundTemplateList.h"
#include "clientGame/TangibleObject.h"
#include "sharedObject/Object.h"

// ======================================================================

WingDynamics::WingDynamics(Object * const owner, float const rotation, float const rotationTime, SoundTemplate const * const soundTemplate) :
	Dynamics(owner),
	m_currentState(false),
	m_desiredState(false),
	m_initialTransform_p(owner->getTransform_o2p()),
	m_rotation(rotation),
	m_timer(rotationTime),
	m_transitioning(false),
	m_soundTemplate(soundTemplate ? SoundTemplateList::fetch(soundTemplate) : 0)
{
}

// ----------------------------------------------------------------------

WingDynamics::~WingDynamics()
{
	if (m_soundTemplate)
		SoundTemplateList::release(m_soundTemplate);
}

// ----------------------------------------------------------------------

float WingDynamics::alter(float const elapsedTime)
{
	//-- Chain up to base
	float const result = Dynamics::alter(elapsedTime);

	//-- Do we have an owner?
	Object * const owner = getOwner();
	if (!owner)
		return result;

	//-- Are we still transitioning from our previous wing state?
	if (m_transitioning)
	{
		if (m_timer.updateZero(elapsedTime))
		{
			m_transitioning = false;
			m_currentState = m_desiredState;
		}

		float const currentRotation = m_currentState ? m_rotation : 0.f;
		float const desiredRotation = m_desiredState ? m_rotation : 0.f;
		float const rotation = linearInterpolate(currentRotation, desiredRotation, m_timer.getElapsedRatio());

		owner->setTransform_o2p(m_initialTransform_p);
		owner->roll_o (rotation);
	}
	else
	{
		//-- Grab our parent object
		TangibleObject const * const tangibleObject = safe_cast<TangibleObject const *>(owner->getAttachedTo());
		if (!tangibleObject)
			return result;

		//-- Our parent object should have our wing state
		m_desiredState = tangibleObject->hasCondition(TangibleObject::C_wingsOpened);

		//-- Set us up to for opening/closing the wings
		if (m_currentState != m_desiredState)
		{
			m_transitioning = true;
			m_timer.reset();

			if (m_soundTemplate)
				Audio::playSound(m_soundTemplate->getName(), owner->getPosition_w(), owner->getParentCell());
		}
	}

	return result;
}

// ======================================================================
