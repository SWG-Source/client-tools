// ======================================================================
//
// PlanetAppearanceTemplate.cpp
// asommers
//
// copyright 2004, sony online entertainment
//
// ======================================================================

#include "clientTerrain/FirstClientTerrain.h"
#include "clientTerrain/PlanetAppearanceTemplate.h"

#include "clientGraphics/Shader.h"
#include "clientGraphics/ShaderTemplateList.h"
#include "clientTerrain/PlanetAppearance.h"
#include "sharedFile/Iff.h"
#include "sharedFoundation/ExitChain.h"
#include "sharedObject/AppearanceTemplateList.h"

#include <string>

// ======================================================================

namespace PlanetAppearanceTemplateNamespace
{
	Tag const TAG_INIT = TAG(I,N,I,T);
	Tag const TAG_SURF = TAG(S,U,R,F);
	Tag const TAG_CLOD = TAG(C,L,O,D);
	Tag const TAG_HALO = TAG(H,A,L,O);
}

using namespace PlanetAppearanceTemplateNamespace;

// ======================================================================
// STATIC PUBLIC PlanetAppearanceTemplate
// ======================================================================

void PlanetAppearanceTemplate::install()
{
	AppearanceTemplateList::assignBinding(cms_planetAppearanceTemplateTag, PlanetAppearanceTemplate::create);

	ExitChain::add(PlanetAppearanceTemplate::remove, "PlanetAppearanceTemplate::remove");
}

// ----------------------------------------------------------------------

AppearanceTemplate * PlanetAppearanceTemplate::create(char const * const filename, Iff * const iff)
{
	return new PlanetAppearanceTemplate(filename, iff);
}

// ======================================================================
// PUBLIC PlanetAppearanceTemplate
// ======================================================================

PlanetAppearanceTemplate::PlanetAppearanceTemplate(char const * const filename, Iff * const iff) :
	AppearanceTemplate(filename),
	m_numberOfLatitudeLines(0),
	m_numberOfLongitudeLines(0),
	m_surfaceRotationRate(0.f),
	m_surfaceShader(0),
	m_surfaceRadius(0.f),
	m_surfaceUScaleMAIN(1.f),
	m_surfaceVScaleMAIN(1.f),
	m_surfaceUScaleDETA(1.f),
	m_surfaceVScaleDETA(1.f),
	m_cloudRotationRate(0.f),
	m_cloudShader(0),
	m_cloudRadius(0.f),
	m_cloudUScaleMAIN(1.f),
	m_cloudVScaleMAIN(1.f),
	m_haloShader(0),
	m_haloScale(0.f)
{
	NOT_NULL(iff);
	load(*iff);
}

// ----------------------------------------------------------------------

PlanetAppearanceTemplate::~PlanetAppearanceTemplate()
{
	if (m_surfaceShader)
	{
		m_surfaceShader->release();
		m_surfaceShader = 0;
	}

	if (m_cloudShader)
	{
		m_cloudShader->release();
		m_cloudShader = 0;
	}
	
	if (m_haloShader)
	{
		m_haloShader->release();
		m_haloShader = 0;
	}
}

// ----------------------------------------------------------------------

Appearance * PlanetAppearanceTemplate::createAppearance() const
{
	return new PlanetAppearance(this);
}

// ----------------------------------------------------------------------

int PlanetAppearanceTemplate::getNumberOfLatitudeLines() const
{
	return m_numberOfLatitudeLines;
}

// ----------------------------------------------------------------------

int PlanetAppearanceTemplate::getNumberOfLongitudeLines() const
{
	return m_numberOfLongitudeLines;
}

// ----------------------------------------------------------------------

float PlanetAppearanceTemplate::getSurfaceRadius() const
{
	return m_surfaceRadius;
}

// ----------------------------------------------------------------------

float PlanetAppearanceTemplate::getSurfaceRotationRate() const
{
	return m_surfaceRotationRate;
}

// ----------------------------------------------------------------------

Shader const * PlanetAppearanceTemplate::fetchSurfaceShader() const
{
	NOT_NULL(m_surfaceShader);
	m_surfaceShader->fetch();

	return m_surfaceShader;
}

// ----------------------------------------------------------------------

float PlanetAppearanceTemplate::getSurfaceUScaleMAIN() const
{
	return m_surfaceUScaleMAIN;
}

// ----------------------------------------------------------------------

float PlanetAppearanceTemplate::getSurfaceVScaleMAIN() const
{
	return m_surfaceVScaleMAIN;
}

// ----------------------------------------------------------------------

float PlanetAppearanceTemplate::getSurfaceUScaleDETA() const
{
	return m_surfaceUScaleDETA;
}

// ----------------------------------------------------------------------

float PlanetAppearanceTemplate::getSurfaceVScaleDETA() const
{
	return m_surfaceVScaleDETA;
}

// ----------------------------------------------------------------------

float PlanetAppearanceTemplate::getCloudRadius() const
{
	return m_cloudRadius;
}

// ----------------------------------------------------------------------

float PlanetAppearanceTemplate::getCloudRotationRate() const
{
	return m_cloudRotationRate;
}

// ----------------------------------------------------------------------

Shader const * PlanetAppearanceTemplate::fetchCloudShader() const
{
	if (m_cloudShader)
		m_cloudShader->fetch();

	return m_cloudShader;
}

// ----------------------------------------------------------------------

float PlanetAppearanceTemplate::getCloudUScaleMAIN() const
{
	return m_cloudUScaleMAIN;
}

// ----------------------------------------------------------------------

float PlanetAppearanceTemplate::getCloudVScaleMAIN() const
{
	return m_cloudVScaleMAIN;
}

// ----------------------------------------------------------------------

float PlanetAppearanceTemplate::getHaloScale() const
{
	return m_haloScale;
}

// ----------------------------------------------------------------------

Shader const * PlanetAppearanceTemplate::fetchHaloShader() const
{
	if (m_haloShader)
		m_haloShader->fetch();
	
	return m_haloShader;
}

// ----------------------------------------------------------------------

Sphere const PlanetAppearanceTemplate::getSphere() const
{
	return Sphere(Vector::zero, std::max(m_surfaceRadius, m_cloudRadius));
}

// ======================================================================
// STATIC PRIVATE PlanetAppearanceTemplate
// ======================================================================

Tag const PlanetAppearanceTemplate::cms_planetAppearanceTemplateTag = TAG(P,L,N,T);

// ----------------------------------------------------------------------

void PlanetAppearanceTemplate::remove()
{
	AppearanceTemplateList::removeBinding(cms_planetAppearanceTemplateTag);
}

// ======================================================================
// PRIVATE PlanetAppearanceTemplate
// ======================================================================

void PlanetAppearanceTemplate::load(Iff & iff)
{
	iff.enterForm(cms_planetAppearanceTemplateTag);

		switch(iff.getCurrentName())
		{
		case TAG_0000:
			load_0000(iff);
			break;

		default:
			{
				char tagBuffer[5];
				ConvertTagToString(iff.getCurrentName(), tagBuffer);

				char buffer[128];
				iff.formatLocation(buffer, sizeof(buffer));
				
				FATAL(true, ("PlanetAppearanceTemplate::load: unsupported version tag %s/%s", buffer, tagBuffer));
			}
			break;
		}

	iff.exitForm(cms_planetAppearanceTemplateTag);

	//-- Validate parameters
	DEBUG_FATAL(!m_surfaceShader, (""));
}

// ----------------------------------------------------------------------

void PlanetAppearanceTemplate::load_0000(Iff & iff)
{
	iff.enterForm(TAG_0000);

		while (iff.getNumberOfBlocksLeft())
		{
			switch (iff.getCurrentName())
			{
			case TAG_INIT:
				{
					iff.enterChunk(TAG_INIT);

						m_numberOfLatitudeLines = iff.read_int32();

						if (m_numberOfLatitudeLines < 2)
						{
							DEBUG_WARNING(true, ("PlanetAppearanceTemplate::load(%s): numberOfLatitudeLines < 2", iff.getFileName(), m_numberOfLatitudeLines));
							m_numberOfLatitudeLines = 2;
						}

						if (m_numberOfLatitudeLines % 2 != 0)
						{
							DEBUG_WARNING(true, ("PlanetAppearanceTemplate::load(%s): numberOfLatitudeLines is not divisible by 2 (%i)", iff.getFileName(), m_numberOfLatitudeLines));
							++m_numberOfLatitudeLines;
						}

						m_numberOfLongitudeLines = iff.read_int32();

						if (m_numberOfLongitudeLines < 2)
						{
							DEBUG_WARNING(true, ("PlanetAppearanceTemplate::load(%s): numberOfLongitudeLines < 2", iff.getFileName(), m_numberOfLongitudeLines));
							m_numberOfLongitudeLines = 2;
						}

						if (m_numberOfLongitudeLines % 2 != 0)
						{
							DEBUG_WARNING(true, ("PlanetAppearanceTemplate::load(%s): numberOfLongitudeLines is not divisible by 2 (%i)", iff.getFileName(), m_numberOfLongitudeLines));
							++m_numberOfLongitudeLines;
						}

					iff.exitChunk(TAG_INIT);
				}
				break;

			case TAG_SURF:
				{
					iff.enterChunk(TAG_SURF);

						m_surfaceRotationRate = convertDegreesToRadians(iff.read_float());

						std::string shaderTemplateName;
						iff.read_string(shaderTemplateName);
						m_surfaceShader = ShaderTemplateList::fetchShader(shaderTemplateName.c_str());
						m_surfaceRadius = iff.read_float();
						m_surfaceUScaleMAIN = iff.read_float();
						m_surfaceVScaleMAIN = iff.read_float();
						m_surfaceUScaleDETA = iff.read_float();
						m_surfaceVScaleDETA = iff.read_float();

					iff.exitChunk(TAG_SURF);
				}
				break;

			case TAG_CLOD:
				{
					iff.enterChunk(TAG_CLOD);

						m_cloudRotationRate = convertDegreesToRadians(iff.read_float());

						std::string shaderTemplateName;
						iff.read_string(shaderTemplateName);
						m_cloudShader = ShaderTemplateList::fetchShader(shaderTemplateName.c_str());
						m_cloudRadius = iff.read_float();
						m_cloudUScaleMAIN = iff.read_float();
						m_cloudVScaleMAIN = iff.read_float();

					iff.exitChunk(TAG_CLOD);
				}
				break;

			case TAG_HALO:
				{
					iff.enterChunk(TAG_HALO);

						std::string shaderTemplateName;
						iff.read_string(shaderTemplateName);
						m_haloShader = ShaderTemplateList::fetchShader(shaderTemplateName.c_str());
						m_haloScale = iff.read_float();
			
		 			iff.exitChunk(TAG_HALO);
				}
				break;

			default:
				{
					char tagBuffer[5];
					ConvertTagToString(iff.getCurrentName(), tagBuffer);

					char buffer[128];
					iff.formatLocation(buffer, sizeof(buffer));
					
					DEBUG_WARNING(true, ("PlanetAppearanceTemplate::load: skipping unknown chunk type %s/%s", buffer, tagBuffer));

					IGNORE_RETURN(iff.goForward());
				}
				break;
			}
		}

	iff.exitForm(TAG_0000);
}

// ======================================================================
