//======================================================================
//
// ClientDataFile_HardpointChildObject.cpp
// copyright (c) 2003 Sony Online Entertainment
//
//======================================================================

#include "clientGame/FirstClientGame.h"
#include "clientGame/ClientDataFile_HardpointChildObject.h"

#include "sharedFile/Iff.h"
#include "sharedObject/AppearanceTemplate.h"
#include "sharedObject/AppearanceTemplateList.h"


//======================================================================

ClientDataFile::HardpointChildObject::HardpointChildObject () :
m_appearanceTemplateName (),
m_hardpointName (),
m_appearanceTemplate (0)
{
}

//----------------------------------------------------------------------

void ClientDataFile::HardpointChildObject::load (Iff & iff)
{
	iff.read_string (m_appearanceTemplateName);
	
	if (!m_appearanceTemplateName.empty ())
		m_appearanceTemplate = AppearanceTemplateList::fetch (m_appearanceTemplateName.c_str ());
	
	iff.read_string (m_hardpointName);
}

//----------------------------------------------------------------------

ClientDataFile::HardpointChildObject::~HardpointChildObject ()
{
	if (m_appearanceTemplate)
	{
		AppearanceTemplateList::release (m_appearanceTemplate);
		m_appearanceTemplate = 0;
	}
}

//======================================================================
