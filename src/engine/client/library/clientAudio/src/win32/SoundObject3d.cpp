// ============================================================================
//
// SoundObject3d.cpp
// Copyright Sony Online Entertainment
//
// ============================================================================

#include "clientAudio/FirstClientAudio.h"
#include "clientAudio/SoundObject3d.h"

// ============================================================================
//
// SoundObject3d
//
// ============================================================================

//-----------------------------------------------------------------------------
SoundObject3d::SoundObject3d()
 : m_object(NULL)
 , m_positionCurrent(Vector::zero)
 , m_positionPrevious(Vector::zero)
 , m_vectorForward(Vector::unitZ)
 , m_vectorUp(Vector::unitY)
{
}

//-----------------------------------------------------------------------------
void SoundObject3d::alter()
{
	m_positionPrevious = m_positionCurrent;

	AIL_set_listener_3D_position(m_object, m_positionCurrent.x, m_positionCurrent.y, m_positionCurrent.z);
	AIL_set_listener_3D_velocity_vector(m_object, 0.0f, 0.0f, 0.0f);
	AIL_set_listener_3D_orientation(m_object, m_vectorForward.x, m_vectorForward.y, m_vectorForward.z, m_vectorUp.x, m_vectorUp.y, m_vectorUp.z);
}

// ============================================================================
