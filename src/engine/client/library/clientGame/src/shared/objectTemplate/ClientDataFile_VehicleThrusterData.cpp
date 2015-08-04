//======================================================================
//
// ClientDataFile_VehicleThrusterData.cpp
// copyright (c) 2003 Sony Online Entertainment
//
//======================================================================

#include "clientGame/FirstClientGame.h"
#include "clientGame/ClientDataFile_VehicleThrusterData.h"

#include "clientGame/ClientDataFile_HardpointChildObject.h"
#include "sharedFile/Iff.h"
#include "sharedFoundation/PointerDeleter.h"
#include <algorithm>
#include <vector>

//======================================================================

namespace
{
	const Tag TAG_HOBJ = TAG (H,O,B,J);
	const Tag TAG_VSND = TAG (V,S,N,D);
}

//----------------------------------------------------------------------

const ClientDataFile::VehicleThrusterData ClientDataFile::VehicleThrusterData::cms_invalid;

//----------------------------------------------------------------------

void ClientDataFile::VehicleThrusterData::load (Iff & iff)
{
	iff.enterChunk (TAG_INFO);
	m_damageLevelMin = iff.read_float ();
	iff.exitChunk (TAG_INFO);
	
	while (iff.enterChunk (TAG_HOBJ, true))
	{
		HardpointChildObject* const hardpointChildObject = new HardpointChildObject;
		hardpointChildObject->load (iff);
		m_hardpointEffects->push_back (hardpointChildObject);
		
		iff.exitChunk (TAG_HOBJ);
	}
	
	if (iff.enterChunk (TAG_VSND, true))
	{
		m_soundData.load (iff);
		iff.exitChunk (TAG_VSND);
	}
}

//----------------------------------------------------------------------

ClientDataFile::VehicleThrusterData::VehicleThrusterData () :
m_hardpointEffects (new HardpointChildObjectList),
m_damageLevelMin   (1.0f),
m_soundData        ()
{
}

//----------------------------------------------------------------------

ClientDataFile::VehicleThrusterData::~VehicleThrusterData ()
{
	std::for_each (m_hardpointEffects->begin (), m_hardpointEffects->end (), PointerDeleter ());
	delete m_hardpointEffects;
	m_hardpointEffects = 0;
}

//======================================================================
