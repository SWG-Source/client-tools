//======================================================================
//
// ClientDataFile_VehicleThrusterData.h
// copyright (c) 2003 Sony Online Entertainment
//
//======================================================================

#ifndef INCLUDED_ClientDataFile_VehicleThrusterData_H
#define INCLUDED_ClientDataFile_VehicleThrusterData_H

//======================================================================

#include "clientGame/ClientDataFile.h"
#include "clientGame/ClientDataFile_VehicleThrusterSoundData.h"

//----------------------------------------------------------------------

class ClientDataFile::VehicleThrusterData
{
public:
	
	static const VehicleThrusterData cms_invalid;
	
	//----------------------------------------------------------------------

	void load (Iff & iff);
	VehicleThrusterData ();
	~VehicleThrusterData ();
	
public:
	
	HardpointChildObjectList * m_hardpointEffects;
	float                    m_damageLevelMin;
	VehicleThrusterSoundData m_soundData;

private:
	VehicleThrusterData (const VehicleThrusterData & rhs);
	VehicleThrusterData& operator = (const VehicleThrusterData & rhs);
};

//======================================================================

#endif
