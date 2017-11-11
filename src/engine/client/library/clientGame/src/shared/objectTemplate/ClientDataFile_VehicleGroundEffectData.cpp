//======================================================================
//
// ClientDataFile_VehicleGroundEffectData.cpp
// copyright (c) 2003 Sony Online Entertainment
//
//======================================================================

#include "clientGame/FirstClientGame.h"
#include "clientGame/ClientDataFile_VehicleGroundEffectData.h"

#include "clientGame/ClientDataFile_HardpointChildObject.h"
#include "sharedFile/Iff.h"
#include "sharedFoundation/PointerDeleter.h"
#include "sharedObject/AppearanceTemplate.h"
#include "sharedObject/AppearanceTemplateList.h"
#include <algorithm>
#include <vector>

//======================================================================

namespace
{
	const Tag TAG_HOBJ = TAG (H,O,B,J);
	const Tag TAG_VSND = TAG (V,S,N,D);
}

//----------------------------------------------------------------------

const VehicleGroundEffectData VehicleGroundEffectData::cms_invalid;

//----------------------------------------------------------------------

void VehicleGroundEffectData::load (Iff & iff)
{
	iff.enterChunk (TAG_INFO);
	{
		m_isWaterEffect = iff.read_uint8 () != 0;
		iff.read_string (m_soundTemplateName);
		iff.read_string (m_hardpointName);
		iff.read_string (m_appearanceTemplateName);

		if (!m_appearanceTemplateName.empty ())
			m_appearanceTemplate = AppearanceTemplateList::fetch (m_appearanceTemplateName.c_str ());
	}
	iff.exitChunk (TAG_INFO);
}

//----------------------------------------------------------------------

VehicleGroundEffectData::VehicleGroundEffectData () :
m_soundTemplateName      (),
m_isWaterEffect          (false),
m_appearanceTemplateName (),
m_appearanceTemplate     (0)
{
}

//----------------------------------------------------------------------

VehicleGroundEffectData::~VehicleGroundEffectData ()
{
	if (m_appearanceTemplate)
	{
		AppearanceTemplateList::release (m_appearanceTemplate);
		m_appearanceTemplate = 0;
	}
}

//======================================================================
