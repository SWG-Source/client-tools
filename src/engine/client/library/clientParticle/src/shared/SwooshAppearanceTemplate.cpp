// ============================================================================
//
// SwooshAppearanceTemplate.cpp
// Copyright Sony Online Entertainment
//
// ============================================================================

#include "clientParticle/FirstClientParticle.h"
#include "clientParticle/SwooshAppearanceTemplate.h"

#include "clientParticle/SwooshAppearance.h"
#include "sharedFile/Iff.h"
#include "sharedFoundation/ExitChain.h"
#include "sharedFoundation/PersistentCrcString.h"
#include "sharedObject/AppearanceTemplateList.h"
#include "sharedObject/ObjectTemplate.h"
#include "sharedUtility/DataTable.h"

#include <map>
#include <vector>

// ============================================================================
//
// SwooshAppearanceTemplateNamespace
//
// ============================================================================

//-----------------------------------------------------------------------------
namespace SwooshAppearanceTemplateNamespace
{
	bool s_installed = false;
	Tag s_tag = TAG(S,W,S,H);
	SwooshAppearanceTemplate *s_defaultSwooshAppearanceTemplate = NULL;

	float s_fullVisibilitySpeed = 0.0f;
	float s_noVisibilitySpeed = 0.0f;
#ifdef _DEBUG
	bool s_useFinalGameSwooshes = true;
#endif // _DEBUG
}

using namespace SwooshAppearanceTemplateNamespace;

// ============================================================================
//
// SwooshAppearanceTemplateNamespace
//
// ============================================================================




// ============================================================================
//
// SwooshAppearanceTemplate
//
// ============================================================================

//-----------------------------------------------------------------------------
SwooshAppearanceTemplate::SwooshAppearanceTemplate()
 : AppearanceTemplate("")
 , m_particleTexture()
 , m_alpha(1.0f)
 , m_color(VectorArgb::solidRed)
 , m_width(0.06f)
 , m_shaderStretchDistance(0.5f)
 , m_shaderScrollSpeed(0.0f)
 , m_fadeAlpha(FA_4)
 , m_taperGeometry(TG_none)
 , m_multiplyColorByAlpha(false)
 , m_startAppearancePath()
 , m_endAppearancePath()
 , m_soundPath()
 , m_startAppearancePosition(AP_middle)
 , m_endAppearancePosition(AP_middle)
 , m_samplesPerSecond(30.0f)
 , m_samplePositionCount(10)
 , m_splineSubQuads(5)
{
}

//-----------------------------------------------------------------------------
SwooshAppearanceTemplate::SwooshAppearanceTemplate(const char *name, Iff *iff)
 : AppearanceTemplate(name)
 , m_particleTexture()
 , m_alpha(1.0f)
 , m_color(VectorArgb::solidWhite)
 , m_width(0.1f)
 , m_shaderStretchDistance(0.5f)
 , m_shaderScrollSpeed(0.0f)
 , m_fadeAlpha(FA_4)
 , m_taperGeometry(TG_none)
 , m_multiplyColorByAlpha(false)
 , m_startAppearancePath()
 , m_endAppearancePath()
 , m_soundPath()
 , m_startAppearancePosition(AP_middle)
 , m_endAppearancePosition(AP_middle)
 , m_samplesPerSecond(30.0f)
 , m_samplePositionCount(10)
 , m_splineSubQuads(5)
{
	if (iff != NULL)
	{
		load(*iff);
	}
}

//-----------------------------------------------------------------------------
SwooshAppearanceTemplate::~SwooshAppearanceTemplate()
{
}

//-----------------------------------------------------------------------------
void SwooshAppearanceTemplate::setDefaultTemplate(SwooshAppearanceTemplate const &swooshAppearanceTemplate)
{
	if (s_defaultSwooshAppearanceTemplate != NULL)
	{
		s_defaultSwooshAppearanceTemplate->m_particleTexture = swooshAppearanceTemplate.m_particleTexture;
		s_defaultSwooshAppearanceTemplate->m_alpha = swooshAppearanceTemplate.m_alpha;
		s_defaultSwooshAppearanceTemplate->m_color = swooshAppearanceTemplate.m_color;
		s_defaultSwooshAppearanceTemplate->m_width = swooshAppearanceTemplate.m_width;
		s_defaultSwooshAppearanceTemplate->m_startAppearancePath = swooshAppearanceTemplate.m_startAppearancePath;
		s_defaultSwooshAppearanceTemplate->m_endAppearancePath = swooshAppearanceTemplate.m_endAppearancePath;
		s_defaultSwooshAppearanceTemplate->m_shaderStretchDistance = swooshAppearanceTemplate.m_shaderStretchDistance;
		s_defaultSwooshAppearanceTemplate->m_shaderScrollSpeed = swooshAppearanceTemplate.m_shaderScrollSpeed;
		s_defaultSwooshAppearanceTemplate->m_soundPath = swooshAppearanceTemplate.m_soundPath;
		s_defaultSwooshAppearanceTemplate->m_fadeAlpha = swooshAppearanceTemplate.m_fadeAlpha;
		s_defaultSwooshAppearanceTemplate->m_taperGeometry = swooshAppearanceTemplate.m_taperGeometry;
		s_defaultSwooshAppearanceTemplate->m_multiplyColorByAlpha = swooshAppearanceTemplate.m_multiplyColorByAlpha;
		s_defaultSwooshAppearanceTemplate->m_startAppearancePosition = swooshAppearanceTemplate.m_startAppearancePosition;
		s_defaultSwooshAppearanceTemplate->m_endAppearancePosition = swooshAppearanceTemplate.m_endAppearancePosition;
		s_defaultSwooshAppearanceTemplate->m_samplesPerSecond = swooshAppearanceTemplate.m_samplesPerSecond;
		s_defaultSwooshAppearanceTemplate->m_samplePositionCount = swooshAppearanceTemplate.m_samplePositionCount;
		s_defaultSwooshAppearanceTemplate->m_splineSubQuads = swooshAppearanceTemplate.m_splineSubQuads;
	}
}

//-----------------------------------------------------------------------------
AppearanceTemplate *SwooshAppearanceTemplate::create(const char* name, Iff* iff)
{
	return new SwooshAppearanceTemplate(name, iff);
}

//-----------------------------------------------------------------------------
void SwooshAppearanceTemplate::install()
{
	DEBUG_FATAL(s_installed, ("Already installed"));

	s_defaultSwooshAppearanceTemplate = new SwooshAppearanceTemplate;

	AppearanceTemplateList::assignBinding(s_tag, create);
	ExitChain::add(&remove, "SwooshAppearanceTemplate::remove()");

	s_installed = true;
}

//-----------------------------------------------------------------------------
void SwooshAppearanceTemplate::remove()
{
	DEBUG_FATAL(!s_installed, ("Not installed"));

	delete s_defaultSwooshAppearanceTemplate;
	s_defaultSwooshAppearanceTemplate = NULL;

	AppearanceTemplateList::removeBinding(s_tag);

	s_installed = false;
}

//-----------------------------------------------------------------------------
Appearance *SwooshAppearanceTemplate::createAppearance() const
{
	return new SwooshAppearance(this);
}

//-----------------------------------------------------------------------------
Tag SwooshAppearanceTemplate::getTag()
{
	return s_tag;
}

//-----------------------------------------------------------------------------
void SwooshAppearanceTemplate::load(Iff &iff)
{
	if (iff.enterForm(s_tag, true))
	{
		switch (iff.getCurrentName())
		{
			case TAG_0000: { load_0000(iff); } break;
			case TAG_0001: { load_0001(iff); } break;
			default:
				{
					// Gracefully handle the unknown file and just initialize to default values

					char currentTagName[256];
					ConvertTagToString(iff.getCurrentName(), currentTagName);
					DEBUG_WARNING(true, ("Unable to load due to unsupported data version: %s", currentTagName));
				}
		}
		iff.exitForm(s_tag);
	}
	else
	{
		// Issue a warning

		char expectedTagName[256];
		ConvertTagToString(s_tag, expectedTagName);
		char currentTagName[256];
		ConvertTagToString(iff.getCurrentName(), currentTagName);
		DEBUG_WARNING(true, ("Unable to load swoosh effect. Expecting tag(%s) but encountered(%s)", expectedTagName, currentTagName));
	}

	iff.allowNonlinearFunctions();
	iff.goToTopOfForm();
}

//-----------------------------------------------------------------------------
void SwooshAppearanceTemplate::load_0000(Iff &iff)
{
	iff.enterForm(TAG_0000);
	{
		m_particleTexture.load(iff);

		iff.enterChunk(TAG_0000);
		{
			float const red = iff.read_float();
			float const green = iff.read_float();
			float const blue = iff.read_float();
			m_color = VectorArgb(1.0f, red, green, blue);
			m_alpha = iff.read_float();
			m_width = iff.read_float();
			m_startAppearancePath = iff.read_stdstring();
			m_endAppearancePath = iff.read_stdstring();
			m_soundPath = iff.read_stdstring();
			m_shaderStretchDistance = iff.read_float();
			m_shaderScrollSpeed = iff.read_float();
			m_fadeAlpha = static_cast<FadeAlpha>(iff.read_int32());
			m_taperGeometry = static_cast<TaperGeometry>(iff.read_int32());
			m_multiplyColorByAlpha = iff.read_bool8();
			m_startAppearancePosition = static_cast<AppearancePosition>(iff.read_int32());
			m_endAppearancePosition = static_cast<AppearancePosition>(iff.read_int32());
		}
		iff.exitChunk();
	}
	iff.exitForm(TAG_0000);
}

//-----------------------------------------------------------------------------
void SwooshAppearanceTemplate::load_0001(Iff &iff)
{
	iff.enterForm(TAG_0001);
	{
		m_particleTexture.load(iff);

		iff.enterChunk(TAG_0000);
		{
			float const red = iff.read_float();
			float const green = iff.read_float();
			float const blue = iff.read_float();
			m_color = VectorArgb(1.0f, red, green, blue);
			m_alpha = iff.read_float();
			m_width = iff.read_float();
			m_startAppearancePath = iff.read_stdstring();
			m_endAppearancePath = iff.read_stdstring();
			m_soundPath = iff.read_stdstring();
			m_shaderStretchDistance = iff.read_float();
			m_shaderScrollSpeed = iff.read_float();
			m_fadeAlpha = static_cast<FadeAlpha>(iff.read_int32());
			m_taperGeometry = static_cast<TaperGeometry>(iff.read_int32());
			m_multiplyColorByAlpha = iff.read_bool8();
			m_startAppearancePosition = static_cast<AppearancePosition>(iff.read_int32());
			m_endAppearancePosition = static_cast<AppearancePosition>(iff.read_int32());
			m_samplesPerSecond = iff.read_float();
			m_samplePositionCount = iff.read_uint32();
			m_splineSubQuads = iff.read_int32();
		}
		iff.exitChunk();
	}
	iff.exitForm(TAG_0001);
}

//-----------------------------------------------------------------------------
void SwooshAppearanceTemplate::write(Iff &iff) const
{
	iff.insertForm(s_tag);
	{
		iff.insertForm(TAG_0001);
		{
			m_particleTexture.write(iff);

			iff.insertChunk(TAG_0000);
			{
				iff.insertChunkData(m_color.r);
				iff.insertChunkData(m_color.g);
				iff.insertChunkData(m_color.b);
				iff.insertChunkData(m_alpha);
				iff.insertChunkData(m_width);
				iff.insertChunkString(m_startAppearancePath.c_str());
				iff.insertChunkString(m_endAppearancePath.c_str());
				iff.insertChunkString(m_soundPath.c_str());
				iff.insertChunkData(m_shaderStretchDistance);
				iff.insertChunkData(m_shaderScrollSpeed);
				iff.insertChunkData(static_cast<int32>(m_fadeAlpha));
				iff.insertChunkData(static_cast<int32>(m_taperGeometry));
				iff.insertChunkData(static_cast<uint8>(m_multiplyColorByAlpha));
				iff.insertChunkData(static_cast<int32>(m_startAppearancePosition));
				iff.insertChunkData(static_cast<int32>(m_endAppearancePosition));
				iff.insertChunkData(m_samplesPerSecond);
				iff.insertChunkData(m_samplePositionCount);
				iff.insertChunkData(m_splineSubQuads);
			}
			iff.exitChunk();
		}
		iff.exitForm(TAG_0001);
	}
	iff.exitForm(s_tag);

	iff.allowNonlinearFunctions();
	iff.goToTopOfForm();
}

//-----------------------------------------------------------------------------
ParticleTexture const &SwooshAppearanceTemplate::getParticleTexture() const
{
	return m_particleTexture;
}

//-----------------------------------------------------------------------------
void SwooshAppearanceTemplate::setAlpha(float const alpha)
{
	m_alpha = alpha;
}

//-----------------------------------------------------------------------------
float SwooshAppearanceTemplate::getAlpha() const
{
	return m_alpha;
}

//-----------------------------------------------------------------------------
void SwooshAppearanceTemplate::setWidth(float const width)
{
	m_width = width;
}

//-----------------------------------------------------------------------------
float SwooshAppearanceTemplate::getWidth() const
{
	return m_width;
}

//-----------------------------------------------------------------------------
void SwooshAppearanceTemplate::setShaderStretchDistance(float const distance)
{
	m_shaderStretchDistance = distance;
}

//-----------------------------------------------------------------------------
float SwooshAppearanceTemplate::getShaderStretchDistance() const
{
	return m_shaderStretchDistance;
}

//-----------------------------------------------------------------------------
void SwooshAppearanceTemplate::setShaderScrollSpeed(float const scrollSpeed)
{
	m_shaderScrollSpeed = scrollSpeed;
}

//-----------------------------------------------------------------------------
float SwooshAppearanceTemplate::getShaderScrollSpeed() const
{
	return m_shaderScrollSpeed;
}

//-----------------------------------------------------------------------------
void SwooshAppearanceTemplate::setColor(float const red, float const green, float const blue)
{
	m_color.r = red;
	m_color.g = green;
	m_color.b = blue;
}

//-----------------------------------------------------------------------------
VectorArgb const &SwooshAppearanceTemplate::getColor() const
{
	return m_color;
}

//-----------------------------------------------------------------------------
void SwooshAppearanceTemplate::setShader(char const *path, int const frameCount, float const framesPerSecond, int const startFrame, int const endFrame)
{
	m_particleTexture.setShaderPath(path);
	m_particleTexture.setFrameCount(frameCount);
	m_particleTexture.setFramesPerSecond(framesPerSecond);
	m_particleTexture.setStartFrame(startFrame);
	m_particleTexture.setEndFrame(endFrame);
}

//-----------------------------------------------------------------------------
void SwooshAppearanceTemplate::setStartAppearancePath(std::string const &path)
{
	m_startAppearancePath = path;
}

//-----------------------------------------------------------------------------
std::string const &SwooshAppearanceTemplate::getStartAppearancePath() const
{
	return m_startAppearancePath;
}

//-----------------------------------------------------------------------------
void SwooshAppearanceTemplate::setEndAppearancePath(std::string const &path)
{
	m_endAppearancePath = path;
}

//-----------------------------------------------------------------------------
std::string const &SwooshAppearanceTemplate::getEndAppearancePath() const
{
	return m_endAppearancePath;
}

//-----------------------------------------------------------------------------
void SwooshAppearanceTemplate::setSoundPath(std::string const &path)
{
	m_soundPath = path;
}

//-----------------------------------------------------------------------------
std::string const &SwooshAppearanceTemplate::getSoundPath() const
{
	return m_soundPath;
}

//-----------------------------------------------------------------------------
void SwooshAppearanceTemplate::setFadeAlpha(FadeAlpha const fadeAlpha)
{
	m_fadeAlpha = fadeAlpha;
}

//-----------------------------------------------------------------------------
SwooshAppearanceTemplate::FadeAlpha SwooshAppearanceTemplate::getFadeAlpha() const
{
	return m_fadeAlpha;
}

//-----------------------------------------------------------------------------
void SwooshAppearanceTemplate::setTaperGeometry(TaperGeometry const taperGeometry)
{
	m_taperGeometry = taperGeometry;
}

//-----------------------------------------------------------------------------
SwooshAppearanceTemplate::TaperGeometry SwooshAppearanceTemplate::getTaperGeometry() const
{
	return m_taperGeometry;
}

//-----------------------------------------------------------------------------
void SwooshAppearanceTemplate::setMultiplyColorByAlpha(bool const enabled)
{
	m_multiplyColorByAlpha = enabled;
}

//-----------------------------------------------------------------------------
bool SwooshAppearanceTemplate::isMultiplyColorByAlpha() const
{
	return m_multiplyColorByAlpha;
}

//-----------------------------------------------------------------------------
SwooshAppearanceTemplate const *SwooshAppearanceTemplate::getDefaultTemplate()
{
	return s_defaultSwooshAppearanceTemplate;
}

//-----------------------------------------------------------------------------
void SwooshAppearanceTemplate::setStartAppearancePosition(AppearancePosition const appearancePosition)
{
	m_startAppearancePosition = appearancePosition;
}

//-----------------------------------------------------------------------------
SwooshAppearanceTemplate::AppearancePosition SwooshAppearanceTemplate::getStartAppearancePosition() const
{
	return m_startAppearancePosition;
}

//-----------------------------------------------------------------------------
void SwooshAppearanceTemplate::setEndAppearancePosition(AppearancePosition const appearancePosition)
{
	m_endAppearancePosition = appearancePosition;
}

//-----------------------------------------------------------------------------
SwooshAppearanceTemplate::AppearancePosition SwooshAppearanceTemplate::getEndAppearancePosition() const
{
	return m_endAppearancePosition;
}

//-----------------------------------------------------------------------------
void SwooshAppearanceTemplate::setFullVisibilitySpeed(float const speed)
{
	s_fullVisibilitySpeed = speed;
}

//-----------------------------------------------------------------------------
float SwooshAppearanceTemplate::getFullVisibilitySpeed()
{
	return s_fullVisibilitySpeed;
}

//-----------------------------------------------------------------------------
void SwooshAppearanceTemplate::setNoVisibilitySpeed(float const speed)
{
	s_noVisibilitySpeed = speed;
}

//-----------------------------------------------------------------------------
float SwooshAppearanceTemplate::getNoVisibilitySpeed()
{
	return s_noVisibilitySpeed;
}

//-----------------------------------------------------------------------------
void SwooshAppearanceTemplate::setSamplesPerSecond(float const samplesPerSecond)
{
	m_samplesPerSecond = clamp(0.01f, samplesPerSecond, 256.0f);
}

//-----------------------------------------------------------------------------
float SwooshAppearanceTemplate::getSamplesPerSecond() const
{
	return m_samplesPerSecond;
}

//-----------------------------------------------------------------------------
float SwooshAppearanceTemplate::getMinSamplesPerSecond()
{
	return 1.0f;
}

//-----------------------------------------------------------------------------
float SwooshAppearanceTemplate::getMaxSamplesPerSecond()
{
	return 256.0f;
}
																				
//-----------------------------------------------------------------------------
void SwooshAppearanceTemplate::setSamplePositionCount(unsigned int const samplePositionCount)
{
	m_samplePositionCount = clamp(2u, samplePositionCount, 4096u);
}

//-----------------------------------------------------------------------------
unsigned int SwooshAppearanceTemplate::getSamplePositionCount() const
{
	return m_samplePositionCount;
}

//-----------------------------------------------------------------------------
void SwooshAppearanceTemplate::setSplineSubQuads(int const splineSubQuads)
{
	m_splineSubQuads = clamp(0, splineSubQuads, 16);
}

//-----------------------------------------------------------------------------
int SwooshAppearanceTemplate::getSplineSubQuads() const
{
	return m_splineSubQuads;
}

#ifdef _DEBUG
//-----------------------------------------------------------------------------
bool SwooshAppearanceTemplate::useFinalGameSwooshes()
{
	return s_useFinalGameSwooshes;
}

//-----------------------------------------------------------------------------
void SwooshAppearanceTemplate::setUseFinalGameSwooshes(bool const enabled)
{
	s_useFinalGameSwooshes = enabled;
}
#endif // _DEBUG

// ============================================================================
