// ============================================================================
//
// ParticleTexture.cpp
// Copyright Sony Online Entertainment
//
// ============================================================================

#include "clientParticle/FirstClientParticle.h"
#include "clientParticle/ParticleTexture.h"

#include "clientGraphics/ShaderTemplate.h"
#include "clientGraphics/ShaderTemplateList.h"
#include "sharedFile/Iff.h"
#include "sharedFoundation/TemporaryCrcString.h"
#include <string>

// ============================================================================
//
// ParticleTexture
//
// ============================================================================

Tag const TAG_PTEX = TAG(P,T,E,X);

//-----------------------------------------------------------------------------
ParticleTexture::ParticleTexture()
 : m_cachedShaderTemplate(NULL)
 , m_shaderPath()
 , m_frameCount(1)
 , m_frameStart(0)
 , m_frameEnd(0)
 , m_frameUVSize(1.0f)
 , m_framesPerColumn(1)
 , m_framesPerSecond(-1.0f)
 , m_textureVisible(true)
{
}

//-----------------------------------------------------------------------------
ParticleTexture::ParticleTexture(char const *shaderPath, int const frameCount, int const startFrame, int const endFrame, float const framesPerSecond, bool textureVisible)
 : m_cachedShaderTemplate(NULL)
 , m_shaderPath()
 , m_frameCount(1)
 , m_frameStart(startFrame)
 , m_frameEnd(endFrame)
 , m_frameUVSize(1.0f)
 , m_framesPerColumn(1)
 , m_framesPerSecond(framesPerSecond)
 , m_textureVisible(textureVisible)
{
	DEBUG_WARNING(m_framesPerColumn < 0, ("ParticleTexture::ParticleTexture() - m_framesPerColumn(%d) < 0", m_framesPerColumn));
	DEBUG_FATAL(m_framesPerSecond < -1.0, ("ParticleTexture::ParticleTexture() - m_framesPerSecond(%f) < -1", m_framesPerSecond));

	setFrameCount(frameCount);
	setShaderPath(shaderPath);
}

//-----------------------------------------------------------------------------
ParticleTexture::ParticleTexture(ParticleTexture const &rhs)
 : m_cachedShaderTemplate(NULL)
 , m_shaderPath()
 , m_frameCount(rhs.m_frameCount)
 , m_frameStart(rhs.m_frameStart)
 , m_frameEnd(rhs.m_frameEnd)
 , m_frameUVSize(rhs.m_frameUVSize)
 , m_framesPerColumn(rhs.m_framesPerColumn)
 , m_framesPerSecond(rhs.m_framesPerSecond)
 , m_textureVisible(rhs.m_textureVisible)
{
	setShaderPath(rhs.m_shaderPath.getString());
}

//-----------------------------------------------------------------------------
ParticleTexture::~ParticleTexture()
{
	if (m_cachedShaderTemplate)
	{
		m_cachedShaderTemplate->release();
		m_cachedShaderTemplate = NULL;
	}
}

//-----------------------------------------------------------------------------
ParticleTexture &ParticleTexture::operator =(const ParticleTexture &rhs)
{
	if (this != &rhs)
	{
		m_frameCount = rhs.m_frameCount;
		m_frameStart = rhs.m_frameStart;
		m_frameEnd = rhs.m_frameEnd;
		m_frameUVSize = rhs.m_frameUVSize;
		m_framesPerColumn = rhs.m_framesPerColumn;
		m_framesPerSecond = rhs.m_framesPerSecond;
		m_textureVisible = rhs.m_textureVisible;
		setShaderPath(rhs.m_shaderPath.getString());
	}

	return *this;
}

//-----------------------------------------------------------------------------
void ParticleTexture::getFrameUVs(int const frame, float &au, float &av, float &bu, float &bv, float &cu, float &cv, float &du, float &dv) const
{
	float const xIndex = static_cast<float>(frame % m_framesPerColumn);
	float const yIndex = static_cast<float>(frame / m_framesPerColumn);

	// Calculate the quad positions

	// b - c
	// |   |
	// a - d

	// [0,0] - [1,0]
	// |           |
	// [0,1] - [1,1]

	cu = xIndex * m_frameUVSize;
	cv = yIndex * m_frameUVSize;
	au = cu + m_frameUVSize;
	av = cv + m_frameUVSize;
	bu = au;
	bv = cv;
	du = cu;
	dv = av;
}

//-----------------------------------------------------------------------------
void ParticleTexture::getUVs(float const particleAgePercent, float const age, float &au, float &av, float &bu, float &bv, float &cu, float &cv, float &du, float &dv) const
{
	int currentFrame = m_frameStart;
	
	int const usedFrames = (m_frameEnd + 1) - m_frameStart;

	if (m_framesPerSecond == -1)
	{
		if (m_frameStart != m_frameEnd)
		{
			// Set the frame rate based on the particle lifetime

			currentFrame = m_frameStart + static_cast<int>(usedFrames * particleAgePercent);
		}
	}
	else
	{
		if ((age > 0.0f) && (m_framesPerSecond > 0.0f))
		{
			int frameNumber = static_cast<int>(age * m_framesPerSecond);
			currentFrame += frameNumber % usedFrames;
		}
	}

	float const xIndex = static_cast<float>(currentFrame % m_framesPerColumn);
	float const yIndex = static_cast<float>(currentFrame / m_framesPerColumn);

	// Calculate the quad positions

	// b - c
	// |   |
	// a - d

	// [0,0] - [0,1]
	// |           |
	// [1,0] - [1,1]

	cu = xIndex * m_frameUVSize;
	cv = yIndex * m_frameUVSize;
	au = cu + m_frameUVSize;
	av = cv + m_frameUVSize;
	bu = au;
	bv = cv;
	du = cu;
	dv = av;
}

//-----------------------------------------------------------------------------
int ParticleTexture::getFrameCount() const
{
	return m_frameCount;
}

//-----------------------------------------------------------------------------
int ParticleTexture::getFrameStart() const
{
	return m_frameStart;
}

//-----------------------------------------------------------------------------
int ParticleTexture::getFrameEnd() const
{
	return m_frameEnd;
}

//-----------------------------------------------------------------------------
void ParticleTexture::load(Iff &iff)
{
	iff.enterForm(TAG_PTEX);
	
	switch (iff.getCurrentName())
	{
		case TAG_0000:
			{
				load_0000(iff);
				break;
			}
		default:
			{
				char currentTagName[256];
				ConvertTagToString(iff.getCurrentName(), currentTagName);
	
				DEBUG_WARNING(true, ("ParticleTexture::load() - Unsupported data version: %s", currentTagName));

				// Need to load the default state.
			}
	}
	
	iff.exitForm(TAG_PTEX);

	DEBUG_WARNING(m_frameCount < 0, ("ParticleTexture::load_0000() - m_frameCount(%d) < 0", m_frameCount));
	DEBUG_WARNING(m_frameStart > m_frameEnd, ("ParticleTexture::load_0000() - m_frameStart(%d) > m_frameEnd(%d)", m_frameStart, m_frameEnd));
	DEBUG_WARNING(m_frameStart < 0, ("ParticleTexture::load_0000() - m_frameStart(%d) < 0", m_frameStart));
	DEBUG_WARNING(m_frameEnd < 0, ("ParticleTexture::load_0000() - m_frameEnd(%d) < 0", m_frameEnd));
	DEBUG_WARNING(m_frameStart > m_frameCount, ("ParticleTexture::load_0000() - m_frameStart(%d) > m_frameCount(%d)", m_frameStart, m_frameCount));
	DEBUG_WARNING(m_frameEnd > m_frameCount, ("ParticleTexture::load_0000() - m_frameEnd(%d) > m_frameCount(%d)", m_frameEnd, m_frameCount));
	DEBUG_WARNING(m_framesPerColumn <= 0, ("ParticleTexture::load_0000() - m_framesPerColumn(%f) <= 0", m_framesPerColumn));
	DEBUG_WARNING((m_framesPerSecond < 0.0f) && (m_framesPerSecond != -1.0f), ("ParticleTexture::load_0000() - (m_framesPerSecond(%f) < 0) && (m_framesPerSecond != -1)", m_framesPerSecond));
}

//-----------------------------------------------------------------------------
void ParticleTexture::setShaderPath(char const *path)
{
	TemporaryCrcString temporaryCrcString(path, false);

	if (m_shaderPath != temporaryCrcString)
	{
		if (m_cachedShaderTemplate != NULL)
		{
			m_cachedShaderTemplate->release();
			m_cachedShaderTemplate = NULL;
		}

		m_shaderPath.set(path, false);

		if (!m_shaderPath.isEmpty())
		{
			m_cachedShaderTemplate = ShaderTemplateList::fetch(path);
		}
	}
}

//-----------------------------------------------------------------------------
void ParticleTexture::load_0000(Iff &iff)
{
	iff.enterChunk(TAG_0000);
	{
		std::string shaderPath;
		iff.read_string(shaderPath);
		setShaderPath(shaderPath.c_str());
		m_frameCount = iff.read_int32();
		m_frameStart = iff.read_int32();
		m_frameEnd = iff.read_int32();
		m_frameUVSize = iff.read_float();
		m_framesPerColumn = iff.read_int32();
		m_framesPerSecond = iff.read_float();
		m_textureVisible = (iff.read_uint8() != 0);
	}
	iff.exitChunk();
}

//-----------------------------------------------------------------------------
void ParticleTexture::write(Iff &iff) const
{
	iff.insertForm(TAG_PTEX);
	{
		iff.insertChunk(TAG_0000);
		{
			iff.insertChunkString(m_shaderPath.getString());
			iff.insertChunkData(m_frameCount);
			iff.insertChunkData(m_frameStart);
			iff.insertChunkData(m_frameEnd);
			iff.insertChunkData(m_frameUVSize);
			iff.insertChunkData(m_framesPerColumn);
			iff.insertChunkData(m_framesPerSecond);
			iff.insertChunkData(static_cast<uint8>(m_textureVisible));
		}
		iff.exitChunk();
	}
	iff.exitForm(TAG_PTEX);
}

//-----------------------------------------------------------------------------
float ParticleTexture::getFramesPerSecond() const
{
	return m_framesPerSecond;
}

//-----------------------------------------------------------------------------
bool ParticleTexture::isTextureVisible() const
{
	return m_textureVisible;
}

//-----------------------------------------------------------------------------
PersistentCrcString const &ParticleTexture::getShaderPath() const
{
	return m_shaderPath;
}

//-----------------------------------------------------------------------------
void ParticleTexture::setStartFrame(int const startFrame)
{
	m_frameStart = startFrame;
}

//-----------------------------------------------------------------------------
void ParticleTexture::setEndFrame(int const endFrame)
{
	m_frameEnd = endFrame;
}

//-----------------------------------------------------------------------------
void ParticleTexture::setFrameCount(int const frameCount)
{
	m_frameCount = frameCount;

	if (frameCount == 4)
	{
		m_framesPerColumn = 2;
	}
	else if (frameCount == 16)
	{
		m_framesPerColumn = 4;
	}
	else if (frameCount != 1)
	{
		m_framesPerColumn = 1;
		DEBUG_WARNING(true, ("Invalid m_frameCount(%d). Must be a power of 2 or it may be too many frames.", m_frameCount));
	}

	if (m_framesPerColumn > 1)
	{
		m_frameUVSize = (1.0f / static_cast<float>(m_framesPerColumn));
	}
}

//-----------------------------------------------------------------------------
void ParticleTexture::setFramesPerSecond(float const framesPerSecond)
{
	m_framesPerSecond = framesPerSecond;
}

// ============================================================================
