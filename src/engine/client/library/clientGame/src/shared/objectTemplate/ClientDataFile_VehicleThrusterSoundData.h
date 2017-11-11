//======================================================================
//
// ClientDataFile_VehicleThrusterSoundData.h
// copyright (c) 2003 Sony Online Entertainment
//
//======================================================================

#ifndef INCLUDED_ClientDataFile_VehicleThrusterSoundData_H
#define INCLUDED_ClientDataFile_VehicleThrusterSoundData_H

class Iff;

//======================================================================

class VehicleThrusterSoundData
{
public:
	std::string m_hardpointName;
	std::string m_idle;
	std::string m_accel;
	std::string m_decel;
	std::string m_run;
	std::string m_damageIdle;
	std::string m_damageAccel;
	std::string m_damageDecel;
	std::string m_damageRun;

	void load (Iff & iff);
};

//======================================================================

#endif
