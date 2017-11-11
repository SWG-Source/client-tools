// ======================================================================
//
// BeamAppearanceTemplate.cpp
// asommers
//
// copyright 2004, sony online entertainment
//
// ======================================================================

#include "clientObject/FirstClientObject.h"
#include "clientObject/BeamAppearanceTemplate.h"

#include "clientGraphics/ShaderTemplate.h"
#include "clientGraphics/ShaderTemplateList.h"
#include "clientObject/BeamAppearance.h"
#include "sharedFile/Iff.h"
#include "sharedFoundation/ExitChain.h"
#include "sharedObject/AppearanceTemplate.h"
#include "sharedObject/AppearanceTemplateList.h"

#include <string>

// ======================================================================

namespace BeamAppearanceTemplateNamespace
{
	Tag const TAG_BEAM = TAG(B,E,A,M);

	void remove();
}

using namespace BeamAppearanceTemplateNamespace;

// ======================================================================

void BeamAppearanceTemplate::install()
{
	AppearanceTemplateList::assignBinding(TAG_BEAM, create);

	ExitChain::add(remove, "BeamAppearanceTemplate::remove");
}

// ----------------------------------------------------------------------

void BeamAppearanceTemplateNamespace::remove()
{
	AppearanceTemplateList::removeBinding(TAG_BEAM);
}

// ----------------------------------------------------------------------

AppearanceTemplate * BeamAppearanceTemplate::create(char const * const fileName, Iff * const iff)
{
	return new BeamAppearanceTemplate(fileName, iff);
}

// ======================================================================

BeamAppearanceTemplate::BeamAppearanceTemplate(char const * const fileName, Iff * const iff) :
	AppearanceTemplate(fileName),
	m_length(0.f),
	m_width(0.f),
	m_color(VectorArgb::solidWhite),
	m_shaderTemplate(0)
{
	load(iff);
}

// ----------------------------------------------------------------------

BeamAppearanceTemplate::~BeamAppearanceTemplate()
{
	if (m_shaderTemplate)
	{
		m_shaderTemplate->release();
		m_shaderTemplate = 0;
	}
}

// ----------------------------------------------------------------------

Appearance * BeamAppearanceTemplate::createAppearance() const
{
	NOT_NULL(m_shaderTemplate);
	return new BeamAppearance(m_length, m_width, m_shaderTemplate->fetchModifiableShader(), m_color);
}

// ----------------------------------------------------------------------

void BeamAppearanceTemplate::load(Iff * const iff)
{
	NOT_NULL (iff);

	iff->enterForm(TAG_BEAM);

		switch (iff->getCurrentName())
		{
		case TAG_0000:
			load_0000 (iff);
			break;

		default:
			{
				char tagBuffer[5];
				ConvertTagToString(iff->getCurrentName(), tagBuffer);

				char buffer[128];
				iff->formatLocation(buffer, sizeof(buffer));
				FATAL(true, ("BeamAppearanceTemplate::load - unsupported version %s/%s", buffer, tagBuffer));
			}
			break;
		}

	iff->exitForm ();
}

// ----------------------------------------------------------------------

void BeamAppearanceTemplate::load_0000(Iff * const iff)
{
	NOT_NULL(iff);

	iff->enterChunk(TAG_0000);

		m_length = iff->read_float();
		m_width = iff->read_float();
		m_color  = iff->read_floatVectorArgb();

		std::string fileName;
		iff->read_string(fileName);
		m_shaderTemplate = ShaderTemplateList::fetch(fileName.c_str());

	iff->exitChunk(TAG_0000);
}

// ======================================================================
