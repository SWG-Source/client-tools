//======================================================================
//
// ClientDataFile_VehicleThrusterSoundData.cpp
// copyright (c) 2003 Sony Online Entertainment
//
//======================================================================

#include "clientGame/FirstClientGame.h"
#include "clientGame/ClientDataFile_VehicleThrusterSoundData.h"

#include "sharedFile/Iff.h"

//======================================================================

void VehicleThrusterSoundData::load (Iff & iff)
{
	iff.read_string (m_hardpointName);
	iff.read_string (m_idle);
	iff.read_string (m_accel);
	iff.read_string (m_decel);
	iff.read_string (m_run);
	iff.read_string (m_damageIdle);
	iff.read_string (m_damageAccel);
	iff.read_string (m_damageDecel);
	iff.read_string (m_damageRun);
}

//======================================================================
