//======================================================================
//
// ClientDataFile_ContrailData.cpp
// copyright (c) 2004 Sony Online Entertainment
//
//======================================================================

#include "clientGame/FirstClientGame.h"
#include "clientGame/ClientDataFile_ContrailData.h"

#include "sharedFile/Iff.h"
#include "sharedObject/AppearanceTemplateList.h"
#include "clientParticle/SwooshAppearanceTemplate.h"
#include "clientParticle/SwooshAppearance.h"
#include "sharedFoundation/ConstCharCrcString.h"

//======================================================================

ContrailData::ContrailData() :
m_name(),
m_hardpointName0(),
m_hardpointName1(),
m_swooshAppearanceName(),
m_overrideName(),
m_width(0.0f),
m_swooshAppearanceTemplate(NULL)
{
}

//----------------------------------------------------------------------

ContrailData::~ContrailData()
{
	if (m_swooshAppearanceTemplate)
	{
		AppearanceTemplateList::release (m_swooshAppearanceTemplate);
		m_swooshAppearanceTemplate = 0;
	}	
}

//----------------------------------------------------------------------

void ContrailData::load(Iff & iff)
{
	switch (iff.getCurrentName())
	{
	case TAG_0000:
		load_0000(iff);
		break;
	default:
		{
			char tagString [5];
			ConvertTagToString (iff.getCurrentName (), tagString);
			
			WARNING(true, ("ClientDataFile_ContrailData invalid version [%s]", tagString));
		}
		break;
	}
}

//----------------------------------------------------------------------

void ContrailData::load_0000(Iff & iff)
{
	iff.enterForm(TAG_0000);
	{
		iff.enterChunk (TAG_INFO);
		{
			iff.read_string(m_name);
			iff.read_string(m_hardpointName0);
			iff.read_string(m_hardpointName1);
			iff.read_string(m_swooshAppearanceName);
			iff.read_string(m_overrideName);
			m_width = iff.read_float();
			
			if (!m_swooshAppearanceName.empty ())
			{
				m_swooshAppearanceTemplate = safe_cast<SwooshAppearanceTemplate const *>(AppearanceTemplateList::fetch (m_swooshAppearanceName.c_str ()));
			}
			
		}
		iff.exitChunk (TAG_INFO);
	}
	iff.exitForm(TAG_0000);
}

//----------------------------------------------------------------------

SwooshAppearance * ContrailData::createAppearance(Appearance const & appearance) const
{
	if (m_swooshAppearanceTemplate != NULL)
	{
		if (!m_hardpointName1.empty())
			return new SwooshAppearance(m_swooshAppearanceTemplate, &appearance, ConstCharCrcString(m_hardpointName0.c_str()), ConstCharCrcString(m_hardpointName1.c_str()));
		else
			return new SwooshAppearance(m_swooshAppearanceTemplate, &appearance, ConstCharCrcString(m_hardpointName0.c_str()));
	}
	return NULL;
}

//======================================================================
