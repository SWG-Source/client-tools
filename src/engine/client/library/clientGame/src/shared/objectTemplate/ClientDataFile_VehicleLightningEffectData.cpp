//======================================================================
//
// ClientDataFile_VehicleLightningEffectData.cpp
// copyright (c) 2008 Sony Online Entertainment
//
//======================================================================

#include "clientGame/FirstClientGame.h"
#include "clientGame/ClientDataFile_VehicleLightningEffectData.h"

#include "sharedFile/Iff.h"


//======================================================================

const VehicleLightningEffectData VehicleLightningEffectData::cms_invalid;

//----------------------------------------------------------------------

void VehicleLightningEffectData::load (Iff & iff)
{
	iff.enterChunk (TAG_INFO);
	{
		iff.read_string (m_startHardpointName);
		iff.read_string (m_endHardpointName);
		iff.read_string (m_appearanceTemplateName);
	}
	iff.exitChunk (TAG_INFO);
}

//----------------------------------------------------------------------

VehicleLightningEffectData::VehicleLightningEffectData () :
  m_startHardpointName(),
  m_endHardpointName(),
  m_appearanceTemplateName()
{
}

//----------------------------------------------------------------------

VehicleLightningEffectData::~VehicleLightningEffectData ()
{

}

//======================================================================
