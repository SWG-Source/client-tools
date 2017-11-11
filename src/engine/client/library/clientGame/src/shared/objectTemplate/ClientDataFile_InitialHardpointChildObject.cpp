// ======================================================================
//
// ClientDataFile_InitialHardpointChildObject.cpp
//
// Copyright 2004 Sony Online Entertainment
//
// ======================================================================

#include "clientGame/FirstClientGame.h"
#include "clientGame/ClientDataFile_InitialHardpointChildObject.h"

#include "sharedFile/Iff.h"
#include "sharedObject/AppearanceTemplate.h"
#include "sharedObject/AppearanceTemplateList.h"


// ======================================================================

ClientDataFile::InitialHardpointChildObject::InitialHardpointChildObject() :
	m_appearanceTemplateName(),
	m_hardpointName(),
	m_recoilDistance(0.f),
	m_recoilRate(0.f),
	m_recoilReturnRate(0.f),
	m_appearanceTemplate(0)
{
}

// ----------------------------------------------------------------------

void ClientDataFile::InitialHardpointChildObject::load(Iff &iff)
{
	iff.read_string(m_appearanceTemplateName);
	
	if (!m_appearanceTemplateName.empty ())
		m_appearanceTemplate = AppearanceTemplateList::fetch(m_appearanceTemplateName.c_str ());
	
	iff.read_string(m_hardpointName);
	m_recoilDistance = iff.read_float();
	m_recoilRate = iff.read_float();
	m_recoilReturnRate = iff.read_float();
}

// ----------------------------------------------------------------------

ClientDataFile::InitialHardpointChildObject::~InitialHardpointChildObject()
{
	if (m_appearanceTemplate)
	{
		AppearanceTemplateList::release(m_appearanceTemplate);
		m_appearanceTemplate = 0;
	}
}

// ======================================================================
