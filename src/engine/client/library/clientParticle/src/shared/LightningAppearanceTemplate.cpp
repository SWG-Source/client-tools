// ============================================================================
//
// LightningAppearanceTemplate.cpp
// Copyright Sony Online Entertainment
//
// ============================================================================

#include "clientParticle/FirstClientParticle.h"
#include "clientParticle/LightningAppearanceTemplate.h"

#include "clientParticle/LightningAppearance.h"
#include "sharedFile/Iff.h"
#include "sharedFoundation/ExitChain.h"
#include "sharedObject/AppearanceTemplateList.h"

// ============================================================================
//
// LightningAppearanceTemplateNamespace
//
// ============================================================================

namespace LightningAppearanceTemplateNamespace
{
	bool s_installed = false;
	Tag s_tag = TAG(L,E,F,X);
};

using namespace LightningAppearanceTemplateNamespace;

// ============================================================================
//
// LightningAppearanceTemplate
//
// ============================================================================

//-----------------------------------------------------------------------------
LightningAppearanceTemplate::LightningAppearanceTemplate()
 : AppearanceTemplate("")
 , m_particleTexture()
 , m_largeBoltPercent(0.5)
 , m_largeBoltThickness(0.25f)
 , m_largeBoltAlpha(1.0f)
 , m_largeBoltColor(VectorArgb::solidWhite)
 , m_largeBoltChaos(1.0f)
 , m_largeBoltAmplitude()
 , m_largeBoltArc(0.0f)
 , m_largeBoltShaderScrollSpeed(0.0f)
 , m_largeBoltShaderStretchDistance(0.5f)
 , m_smallBoltThickness(0.1f)
 , m_smallBoltAlpha(0.5f)
 , m_smallBoltColor(VectorArgb::solidWhite)
 , m_smallBoltChaos(1.0f)
 , m_smallBoltAmplitude()
 , m_smallBoltArc(0.0f)
 , m_smallBoltShaderScrollSpeed(0.0f)
 , m_smallBoltShaderStretchDistance(0.5f)
 , m_startAppearancePath()
 , m_endAppearancePath()
 , m_soundPath()
 , m_startAppearanceCount(AC_single)
 , m_endAppearanceCount(AC_single)
{
}

//-----------------------------------------------------------------------------
LightningAppearanceTemplate::LightningAppearanceTemplate(const char *name, Iff *iff)
 : AppearanceTemplate(name)
 , m_particleTexture()
 , m_largeBoltPercent(0.5)
 , m_largeBoltThickness(0.25f)
 , m_largeBoltAlpha(1.0f)
 , m_largeBoltColor(VectorArgb::solidWhite)
 , m_largeBoltChaos(1.0f)
 , m_largeBoltAmplitude()
 , m_largeBoltArc(0.0f)
 , m_largeBoltShaderScrollSpeed(0.0f)
 , m_largeBoltShaderStretchDistance(0.5f)
 , m_smallBoltThickness(0.1f)
 , m_smallBoltAlpha(0.5f)
 , m_smallBoltColor(VectorArgb::solidWhite)
 , m_smallBoltChaos(1.0f)
 , m_smallBoltAmplitude()
 , m_smallBoltArc(0.0f)
 , m_smallBoltShaderScrollSpeed(0.0f)
 , m_smallBoltShaderStretchDistance(0.5f)
 , m_startAppearancePath()
 , m_endAppearancePath()
 , m_soundPath()
 , m_startAppearanceCount(AC_single)
 , m_endAppearanceCount(AC_single)
{
	if (iff != NULL)
	{
		load(*iff);
	}
}

//-----------------------------------------------------------------------------
LightningAppearanceTemplate::~LightningAppearanceTemplate()
{
}

//-----------------------------------------------------------------------------
AppearanceTemplate *LightningAppearanceTemplate::create(const char* name, Iff* iff)
{
	return new LightningAppearanceTemplate(name, iff);
}

//-----------------------------------------------------------------------------
void LightningAppearanceTemplate::install()
{
	DEBUG_FATAL(s_installed, ("Already installed"));
	s_installed = true;

	AppearanceTemplateList::assignBinding(s_tag, create);

	ExitChain::add(&remove, "LightningAppearanceTemplate::remove()");
}

//-----------------------------------------------------------------------------
void LightningAppearanceTemplate::remove()
{
	DEBUG_FATAL(!s_installed, ("Not installed"));

	AppearanceTemplateList::removeBinding(s_tag);

	s_installed = false;
}

//-----------------------------------------------------------------------------
Appearance *LightningAppearanceTemplate::createAppearance() const
{
	return new LightningAppearance(this);
}

//-----------------------------------------------------------------------------
Tag LightningAppearanceTemplate::getTag()
{
	return s_tag;
}

//--------------------------------------------------------------------------
void LightningAppearanceTemplate::load(Iff &iff)
{
	if (iff.enterForm(s_tag, true))
	{
		switch (iff.getCurrentName())
		{
			case TAG_0000: { load_0000(iff); } break;
			case TAG_0001: { load_0001(iff); } break;
			case TAG_0002: { load_0002(iff); } break;
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
		DEBUG_WARNING(true, ("Unable to load lightning effect. Expecting tag(%s) but encountered(%s)", expectedTagName, currentTagName));
	}

	iff.allowNonlinearFunctions();
	iff.goToTopOfForm();
}

//--------------------------------------------------------------------------
void LightningAppearanceTemplate::load_0000(Iff &iff)
{
	iff.enterForm(TAG_0000);
	{
		m_particleTexture.load(iff);

		iff.enterChunk(TAG_0000);
		{
			m_largeBoltPercent = iff.read_float();
			m_startAppearancePath = iff.read_stdstring();
			m_endAppearancePath = iff.read_stdstring();
			m_soundPath = iff.read_stdstring();

			// Large Bolt

			m_largeBoltThickness = iff.read_float();
			m_largeBoltAlpha = iff.read_float();
			{
				float const red = iff.read_float();
				float const green = iff.read_float();
				float const blue = iff.read_float();
				
				m_largeBoltColor = VectorArgb(1.0f, red, green, blue);
			}
			m_largeBoltChaos = iff.read_float();
			float const largeBoltAmplitude = iff.read_float();

			m_largeBoltAmplitude.clear();
			m_largeBoltAmplitude.insert(WaveFormControlPoint(0.0f, 0.0f));
			m_largeBoltAmplitude.insert(WaveFormControlPoint(0.5f, largeBoltAmplitude));
			m_largeBoltAmplitude.insert(WaveFormControlPoint(0.95f, largeBoltAmplitude));
			m_largeBoltAmplitude.insert(WaveFormControlPoint(1.0f, 0.0f));

			// Small Bolt

			m_smallBoltThickness = iff.read_float();
			m_smallBoltAlpha = iff.read_float();
			{
				float const red = iff.read_float();
				float const green = iff.read_float();
				float const blue = iff.read_float();
				
				m_smallBoltColor = VectorArgb(1.0f, red, green, blue);
			}
			m_smallBoltChaos = iff.read_float();
			float const smallBoltAmplitude = iff.read_float();

			m_smallBoltAmplitude.clear();
			m_smallBoltAmplitude.insert(WaveFormControlPoint(0.0f, 0.0f));
			m_smallBoltAmplitude.insert(WaveFormControlPoint(0.5f, smallBoltAmplitude));
			m_smallBoltAmplitude.insert(WaveFormControlPoint(0.95f, smallBoltAmplitude));
			m_smallBoltAmplitude.insert(WaveFormControlPoint(1.0f, 0.0f));

			m_largeBoltShaderStretchDistance = m_smallBoltShaderStretchDistance = iff.read_float();
		}
		iff.exitChunk();
	}
	iff.exitForm(TAG_0000);

	m_startAppearanceCount = AC_onePerBolt;
	m_endAppearanceCount = AC_onePerBolt;
}

//--------------------------------------------------------------------------
void LightningAppearanceTemplate::load_0001(Iff &iff)
{
	iff.enterForm(TAG_0001);
	{
		m_particleTexture.load(iff);
		m_largeBoltAmplitude.load(iff);
		m_smallBoltAmplitude.load(iff);

		iff.enterChunk(TAG_0000);
		{
			m_largeBoltPercent = iff.read_float();
			m_startAppearancePath = iff.read_stdstring();
			m_endAppearancePath = iff.read_stdstring();
			m_soundPath = iff.read_stdstring();
			m_largeBoltShaderStretchDistance = m_smallBoltShaderStretchDistance = iff.read_float();
			m_startAppearanceCount = static_cast<AppearanceCount>(iff.read_uint32());
			m_endAppearanceCount = static_cast<AppearanceCount>(iff.read_uint32());
			iff.read_bool8();

			// Large Bolt

			m_largeBoltThickness = iff.read_float();
			m_largeBoltAlpha = iff.read_float();
			{
				float const red = iff.read_float();
				float const green = iff.read_float();
				float const blue = iff.read_float();
				
				m_largeBoltColor = VectorArgb(1.0f, red, green, blue);
			}
			m_largeBoltChaos = iff.read_float();
			m_largeBoltArc = iff.read_float();

			// Small Bolt

			m_smallBoltThickness = iff.read_float();
			m_smallBoltAlpha = iff.read_float();
			{
				float const red = iff.read_float();
				float const green = iff.read_float();
				float const blue = iff.read_float();
				
				m_smallBoltColor = VectorArgb(1.0f, red, green, blue);
			}
			m_smallBoltChaos = iff.read_float();
			m_smallBoltArc = iff.read_float();
		}
		iff.exitChunk();
	}
	iff.exitForm(TAG_0001);
}

//--------------------------------------------------------------------------
void LightningAppearanceTemplate::load_0002(Iff &iff)
{
	iff.enterForm(TAG_0002);
	{
		m_particleTexture.load(iff);
		m_largeBoltAmplitude.load(iff);
		m_smallBoltAmplitude.load(iff);

		iff.enterChunk(TAG_0000);
		{
			m_largeBoltPercent = iff.read_float();
			m_startAppearancePath = iff.read_stdstring();
			m_endAppearancePath = iff.read_stdstring();
			m_soundPath = iff.read_stdstring();
			m_startAppearanceCount = static_cast<AppearanceCount>(iff.read_uint32());
			m_endAppearanceCount = static_cast<AppearanceCount>(iff.read_uint32());

			// Large Bolt

			m_largeBoltThickness = iff.read_float();
			m_largeBoltAlpha = iff.read_float();
			{
				float const red = iff.read_float();
				float const green = iff.read_float();
				float const blue = iff.read_float();
				
				m_largeBoltColor = VectorArgb(1.0f, red, green, blue);
			}
			m_largeBoltChaos = iff.read_float();
			m_largeBoltArc = iff.read_float();
			m_largeBoltShaderStretchDistance = iff.read_float();
			m_largeBoltShaderScrollSpeed = iff.read_float();

			// Small Bolt

			m_smallBoltThickness = iff.read_float();
			m_smallBoltAlpha = iff.read_float();
			{
				float const red = iff.read_float();
				float const green = iff.read_float();
				float const blue = iff.read_float();
				
				m_smallBoltColor = VectorArgb(1.0f, red, green, blue);
			}
			m_smallBoltChaos = iff.read_float();
			m_smallBoltArc = iff.read_float();
			m_smallBoltShaderStretchDistance = iff.read_float();
			m_smallBoltShaderScrollSpeed = iff.read_float();
		}
		iff.exitChunk();
	}
	iff.exitForm(TAG_0002);
}

//--------------------------------------------------------------------------
void LightningAppearanceTemplate::write(Iff &iff) const
{
	iff.insertForm(s_tag);
	{
		iff.insertForm(TAG_0002);
		{
			m_particleTexture.write(iff);
			m_largeBoltAmplitude.write(iff);
			m_smallBoltAmplitude.write(iff);

			iff.insertChunk(TAG_0000);
			{
				iff.insertChunkData(m_largeBoltPercent);
				iff.insertChunkString(m_startAppearancePath.c_str());
				iff.insertChunkString(m_endAppearancePath.c_str());
				iff.insertChunkString(m_soundPath.c_str());
				iff.insertChunkData(static_cast<uint32>(m_startAppearanceCount));
				iff.insertChunkData(static_cast<uint32>(m_endAppearanceCount));

				// Large Bolt

				iff.insertChunkData(m_largeBoltThickness);
				iff.insertChunkData(m_largeBoltAlpha);
				iff.insertChunkData(m_largeBoltColor.r);
				iff.insertChunkData(m_largeBoltColor.g);
				iff.insertChunkData(m_largeBoltColor.b);
				iff.insertChunkData(m_largeBoltChaos);
				iff.insertChunkData(m_largeBoltArc);
				iff.insertChunkData(m_largeBoltShaderStretchDistance);
				iff.insertChunkData(m_largeBoltShaderScrollSpeed);

				// Small Bolt

				iff.insertChunkData(m_smallBoltThickness);
				iff.insertChunkData(m_smallBoltAlpha);
				iff.insertChunkData(m_smallBoltColor.r);
				iff.insertChunkData(m_smallBoltColor.g);
				iff.insertChunkData(m_smallBoltColor.b);
				iff.insertChunkData(m_smallBoltChaos);
				iff.insertChunkData(m_smallBoltArc);
				iff.insertChunkData(m_smallBoltShaderStretchDistance);
				iff.insertChunkData(m_smallBoltShaderScrollSpeed);
			}
			iff.exitChunk();
		}
		iff.exitForm(TAG_0002);
	}
	iff.exitForm(s_tag);

	iff.allowNonlinearFunctions();
	iff.goToTopOfForm();
}

//-----------------------------------------------------------------------------
ParticleTexture const &LightningAppearanceTemplate::getParticleTexture() const
{
	return m_particleTexture;
}

//-----------------------------------------------------------------------------
void LightningAppearanceTemplate::setLargeBoltThickness(float const thickness)
{
	m_largeBoltThickness = thickness;
}

//-----------------------------------------------------------------------------
float LightningAppearanceTemplate::getLargeBoltThickness() const
{
	return m_largeBoltThickness;
}

//-----------------------------------------------------------------------------
void LightningAppearanceTemplate::setSmallBoltThickness(float const thickness)
{
	m_smallBoltThickness = thickness;
}

//-----------------------------------------------------------------------------
float LightningAppearanceTemplate::getSmallBoltThickness() const
{
	return m_smallBoltThickness;
}

//-----------------------------------------------------------------------------
void LightningAppearanceTemplate::setLargeBoltAlpha(float const alpha)
{
	m_largeBoltAlpha = alpha;
}

//-----------------------------------------------------------------------------
float LightningAppearanceTemplate::getLargeBoltAlpha() const
{
	return m_largeBoltAlpha;
}

//-----------------------------------------------------------------------------
void LightningAppearanceTemplate::setSmallBoltAlpha(float const alpha)
{
	m_smallBoltAlpha = alpha;
}

//-----------------------------------------------------------------------------
float LightningAppearanceTemplate::getSmallBoltAlpha() const
{
	return m_smallBoltAlpha;
}

//-----------------------------------------------------------------------------
void LightningAppearanceTemplate::setLargeBoltChaos(float const chaos)
{
	m_largeBoltChaos = chaos;
}

//-----------------------------------------------------------------------------
float LightningAppearanceTemplate::getLargeBoltChaos() const
{
	return m_largeBoltChaos;
}

//-----------------------------------------------------------------------------
void LightningAppearanceTemplate::setSmallBoltChaos(float const chaos)
{
	m_smallBoltChaos = chaos;
}

//-----------------------------------------------------------------------------
float LightningAppearanceTemplate::getSmallBoltChaos() const
{
	return m_smallBoltChaos;
}

//-----------------------------------------------------------------------------
void LightningAppearanceTemplate::setLargeBoltAmplitude(WaveForm const &amplitude)
{
	m_largeBoltAmplitude = amplitude;
}

//-----------------------------------------------------------------------------
WaveForm const &LightningAppearanceTemplate::getLargeBoltAmplitude() const
{
	return m_largeBoltAmplitude;
}

//-----------------------------------------------------------------------------
float LightningAppearanceTemplate::getLargeBoltBaseAmplitude() const
{
	WaveFormControlPointIter iterAmplitude;
	iterAmplitude.reset(m_largeBoltAmplitude.getIteratorBegin());

	++(iterAmplitude.m_iter);

	return iterAmplitude.m_iter->getValue();
}

//-----------------------------------------------------------------------------
float LightningAppearanceTemplate::getLargeBoltStartAmplitude() const
{
	WaveFormControlPointIter iterAmplitude;
	iterAmplitude.reset(m_largeBoltAmplitude.getIteratorBegin());

	return iterAmplitude.m_iter->getValue();
}

//-----------------------------------------------------------------------------
float LightningAppearanceTemplate::getLargeBoltStartUntil() const
{
	WaveFormControlPointIter iterAmplitude;
	iterAmplitude.reset(m_largeBoltAmplitude.getIteratorBegin());

	++(iterAmplitude.m_iter);

	return iterAmplitude.m_iter->getPercent();
}

//-----------------------------------------------------------------------------
float LightningAppearanceTemplate::getLargeBoltEndFrom() const
{
	WaveFormControlPointIter iterAmplitude;
	iterAmplitude.reset(m_largeBoltAmplitude.getIteratorBegin());

	++(iterAmplitude.m_iter);
	++(iterAmplitude.m_iter);

	return iterAmplitude.m_iter->getPercent();
}

//-----------------------------------------------------------------------------
float LightningAppearanceTemplate::getLargeBoltEndAmplitude() const
{
	WaveFormControlPointIter iterAmplitude;
	iterAmplitude.reset(m_largeBoltAmplitude.getIteratorBegin());

	++(iterAmplitude.m_iter);
	++(iterAmplitude.m_iter);
	++(iterAmplitude.m_iter);

	return iterAmplitude.m_iter->getValue();
}

//-----------------------------------------------------------------------------
void LightningAppearanceTemplate::setLargeBoltArc(float const arc)
{
	m_largeBoltArc = arc;
}

//-----------------------------------------------------------------------------
float LightningAppearanceTemplate::getLargeBoltArc() const
{
	return m_largeBoltArc;
}

//-----------------------------------------------------------------------------
void LightningAppearanceTemplate::setSmallBoltAmplitude(WaveForm const &amplitude)
{
	m_smallBoltAmplitude = amplitude;
}

//-----------------------------------------------------------------------------
WaveForm const &LightningAppearanceTemplate::getSmallBoltAmplitude() const
{
	return m_smallBoltAmplitude;
}

//-----------------------------------------------------------------------------
float LightningAppearanceTemplate::getSmallBoltBaseAmplitude() const
{
	WaveFormControlPointIter iterAmplitude;
	iterAmplitude.reset(m_smallBoltAmplitude.getIteratorBegin());

	++(iterAmplitude.m_iter);

	return iterAmplitude.m_iter->getValue();
}

//-----------------------------------------------------------------------------
float LightningAppearanceTemplate::getSmallBoltStartAmplitude() const
{
	WaveFormControlPointIter iterAmplitude;
	iterAmplitude.reset(m_smallBoltAmplitude.getIteratorBegin());

	return iterAmplitude.m_iter->getValue();
}

//-----------------------------------------------------------------------------
float LightningAppearanceTemplate::getSmallBoltStartUntil() const
{
	WaveFormControlPointIter iterAmplitude;
	iterAmplitude.reset(m_smallBoltAmplitude.getIteratorBegin());

	++(iterAmplitude.m_iter);

	return iterAmplitude.m_iter->getPercent();
}

//-----------------------------------------------------------------------------
float LightningAppearanceTemplate::getSmallBoltEndFrom() const
{
	WaveFormControlPointIter iterAmplitude;
	iterAmplitude.reset(m_smallBoltAmplitude.getIteratorBegin());

	++(iterAmplitude.m_iter);
	++(iterAmplitude.m_iter);

	return iterAmplitude.m_iter->getPercent();
}

//-----------------------------------------------------------------------------
float LightningAppearanceTemplate::getSmallBoltEndAmplitude() const
{
	WaveFormControlPointIter iterAmplitude;
	iterAmplitude.reset(m_smallBoltAmplitude.getIteratorBegin());

	++(iterAmplitude.m_iter);
	++(iterAmplitude.m_iter);
	++(iterAmplitude.m_iter);

	return iterAmplitude.m_iter->getValue();
}

//-----------------------------------------------------------------------------
void LightningAppearanceTemplate::setSmallBoltArc(float const arc)
{
	m_smallBoltArc = arc;
}

//-----------------------------------------------------------------------------
float LightningAppearanceTemplate::getSmallBoltArc() const
{
	return m_smallBoltArc;
}

//-----------------------------------------------------------------------------
void LightningAppearanceTemplate::setLargeBoltPercent(float const percent)
{
	m_largeBoltPercent = clamp(0.0f, percent, 1.0f);
}

//-----------------------------------------------------------------------------
float LightningAppearanceTemplate::getLargeBoltPercent() const
{
	return m_largeBoltPercent;
}

//-----------------------------------------------------------------------------
void LightningAppearanceTemplate::setLargeBoltColor(float const red, float const green, float const blue)
{
	m_largeBoltColor.r = red;
	m_largeBoltColor.g = green;
	m_largeBoltColor.b = blue;
}

//-----------------------------------------------------------------------------
VectorArgb const &LightningAppearanceTemplate::getLargeBoltColor() const
{
	return m_largeBoltColor;
}

//-----------------------------------------------------------------------------
void LightningAppearanceTemplate::setSmallBoltColor(float const red, float const green, float const blue)
{
	m_smallBoltColor.r = red;
	m_smallBoltColor.g = green;
	m_smallBoltColor.b = blue;
}

//-----------------------------------------------------------------------------
VectorArgb const &LightningAppearanceTemplate::getSmallBoltColor() const
{
	return m_smallBoltColor;
}

//-----------------------------------------------------------------------------
void LightningAppearanceTemplate::setShader(char const *path, int const frameCount, float const framesPerSecond, int const startFrame, int const endFrame)
{
	m_particleTexture.setShaderPath(path);
	m_particleTexture.setFrameCount(frameCount);
	m_particleTexture.setFramesPerSecond(framesPerSecond);
	m_particleTexture.setStartFrame(startFrame);
	m_particleTexture.setEndFrame(endFrame);
}

//-----------------------------------------------------------------------------
void LightningAppearanceTemplate::setStartAppearancePath(std::string const &startAppearancePath)
{
	m_startAppearancePath = startAppearancePath;
}

//-----------------------------------------------------------------------------
std::string const &LightningAppearanceTemplate::getStartAppearancePath() const
{
	return m_startAppearancePath;
}

//-----------------------------------------------------------------------------
void LightningAppearanceTemplate::setStartAppearanceCount(AppearanceCount const appearanceCount)
{
	m_startAppearanceCount = appearanceCount;
}

//-----------------------------------------------------------------------------
LightningAppearanceTemplate::AppearanceCount LightningAppearanceTemplate::getStartAppearanceCount() const
{
	return m_startAppearanceCount;
}

//-----------------------------------------------------------------------------
void LightningAppearanceTemplate::setEndAppearancePath(std::string const &endAppearancePath)
{
	m_endAppearancePath = endAppearancePath;
}

//-----------------------------------------------------------------------------
std::string const &LightningAppearanceTemplate::getEndAppearancePath() const
{
	return m_endAppearancePath;
}

//-----------------------------------------------------------------------------
void LightningAppearanceTemplate::setEndAppearanceCount(AppearanceCount const appearanceCount)
{
	m_endAppearanceCount = appearanceCount;
}

//-----------------------------------------------------------------------------
LightningAppearanceTemplate::AppearanceCount LightningAppearanceTemplate::getEndAppearanceCount() const
{
	return m_endAppearanceCount;
}

//-----------------------------------------------------------------------------
void LightningAppearanceTemplate::setSoundPath(std::string const &soundPath)
{
	m_soundPath = soundPath;
}

//-----------------------------------------------------------------------------
std::string const &LightningAppearanceTemplate::getSoundPath() const
{
	return m_soundPath;
}

//-----------------------------------------------------------------------------
void LightningAppearanceTemplate::setLargeBoltShaderScrollSpeed(float const scrollSpeed)
{
	m_largeBoltShaderScrollSpeed = scrollSpeed;
}

//-----------------------------------------------------------------------------
float LightningAppearanceTemplate::getLargeBoltShaderScrollSpeed() const
{
	return m_largeBoltShaderScrollSpeed;
}

//-----------------------------------------------------------------------------
void LightningAppearanceTemplate::setSmallBoltShaderScrollSpeed(float const scrollSpeed)
{
	m_smallBoltShaderScrollSpeed = scrollSpeed;
}

//-----------------------------------------------------------------------------
float LightningAppearanceTemplate::getSmallBoltShaderScrollSpeed() const
{
	return m_smallBoltShaderScrollSpeed;
}

//-----------------------------------------------------------------------------
void LightningAppearanceTemplate::setLargeBoltShaderStretchDistance(float const distance)
{
	m_largeBoltShaderStretchDistance = distance;
}

//-----------------------------------------------------------------------------
float LightningAppearanceTemplate::getLargeBoltShaderStretchDistance() const
{
	return m_largeBoltShaderStretchDistance;
}

//-----------------------------------------------------------------------------
void LightningAppearanceTemplate::setSmallBoltShaderStretchDistance(float const distance)
{
	m_smallBoltShaderStretchDistance = distance;
}

//-----------------------------------------------------------------------------
float LightningAppearanceTemplate::getSmallBoltShaderStretchDistance() const
{
	return m_smallBoltShaderStretchDistance;
}

// ============================================================================
