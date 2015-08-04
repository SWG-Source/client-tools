// ============================================================================
//
// ParticleDescriptionQuad.cpp
// Copyright Sony Online Entertainment
//
// ============================================================================

#include "clientParticle/FirstClientParticle.h"
#include "clientParticle/ParticleDescriptionQuad.h"

#include "sharedFile/Iff.h"

// ============================================================================
//
// ParticleDescriptionQuad
//
// ============================================================================

Tag const TAG_PTQD = TAG(P, T, Q, D);

//-----------------------------------------------------------------------------
ParticleDescriptionQuad::ParticleDescriptionQuad()
 : m_rotation()
 , m_length()
 , m_width()
 , m_lengthAndWidthLinked(false)
 , m_particleTexture()
{
}

//-----------------------------------------------------------------------------
ParticleDescriptionQuad::ParticleDescriptionQuad(ParticleDescriptionQuad const &particleDescriptionQuad)
 : ParticleDescription(particleDescriptionQuad)
 , m_rotation(particleDescriptionQuad.m_rotation)
 , m_length(particleDescriptionQuad.m_length)
 , m_width(particleDescriptionQuad.m_width)
 , m_lengthAndWidthLinked(particleDescriptionQuad.m_lengthAndWidthLinked)
 , m_particleTexture(particleDescriptionQuad.m_particleTexture)
{
}

//--------------------------------------------------------------------------
ParticleDescription *ParticleDescriptionQuad::clone() const
{
	return new ParticleDescriptionQuad(*this);
}

//--------------------------------------------------------------------------
ParticleDescription::ParticleType ParticleDescriptionQuad::getParticleType() const
{
	return ParticleDescription::PT_quad;
}

//--------------------------------------------------------------------------
void ParticleDescriptionQuad::initializeDefault()
{
	ParticleDescription::initializeDefault();

	setDefaultRotation(m_rotation);
	setDefaultLength(m_length);
	setDefaultWidth(m_width);
	m_lengthAndWidthLinked = false;
	m_particleTexture = ParticleTexture();
}

//--------------------------------------------------------------------------
void ParticleDescriptionQuad::setDefaultRotation(WaveForm &waveForm)
{
#ifdef _DEBUG
	waveForm.setName("Particle - Rotation");
#endif // _DEBUG
	waveForm.clear();
	waveForm.setValueMax(4096.0f);
	waveForm.setValueMin(-4096.0f);
	waveForm.insert(WaveFormControlPoint(0.0f, 0.0f));
	waveForm.insert(WaveFormControlPoint(1.0f, 0.0f));
}

//--------------------------------------------------------------------------
void ParticleDescriptionQuad::setDefaultLength(WaveForm &waveForm)
{
#ifdef _DEBUG
	waveForm.setName("Particle - Length");
#endif // _DEBUG
	waveForm.clear();
	waveForm.setValueMax(4096.0f);
	waveForm.setValueMin(-4096.0f);
	waveForm.insert(WaveFormControlPoint(0.0f, 0.25f));
	waveForm.insert(WaveFormControlPoint(1.0f, 0.25f));
}

//--------------------------------------------------------------------------
void ParticleDescriptionQuad::setDefaultWidth(WaveForm &waveForm)
{
#ifdef _DEBUG
	waveForm.setName("Particle - Width");
#endif // _DEBUG
	waveForm.clear();
	waveForm.setValueMax(4096.0f);
	waveForm.setValueMin(-4096.0f);
	waveForm.insert(WaveFormControlPoint(0.0f, 0.25f));
	waveForm.insert(WaveFormControlPoint(1.0f, 0.25f));
}

//--------------------------------------------------------------------------
void ParticleDescriptionQuad::setRotation(WaveForm const &rotation)
{
	m_rotation = rotation;
}

//--------------------------------------------------------------------------
void ParticleDescriptionQuad::setLength(WaveForm const &length)
{
	m_length = length;
}

//--------------------------------------------------------------------------
void ParticleDescriptionQuad::setWidth(WaveForm const &width)
{
	m_width = width;
}

//--------------------------------------------------------------------------
void ParticleDescriptionQuad::setLengthAndWidthLinked(bool const lengthAndWidthLinked)
{
	m_lengthAndWidthLinked = lengthAndWidthLinked;
}

//--------------------------------------------------------------------------
void ParticleDescriptionQuad::setParticleTexture(ParticleTexture const &particleTexture)
{
	m_particleTexture = particleTexture;
}

//--------------------------------------------------------------------------
WaveForm const &ParticleDescriptionQuad::getRotation() const
{
	return m_rotation;
}

//--------------------------------------------------------------------------
WaveForm const &ParticleDescriptionQuad::getLength() const
{
	return m_length;
}

//--------------------------------------------------------------------------
WaveForm const &ParticleDescriptionQuad::getWidth() const
{
	return m_width;
}

//--------------------------------------------------------------------------
bool ParticleDescriptionQuad::isLengthAndWidthLinked() const
{
	return m_lengthAndWidthLinked;
}

//--------------------------------------------------------------------------
ParticleTexture const &ParticleDescriptionQuad::getParticleTexture() const
{
	return m_particleTexture;
}

//--------------------------------------------------------------------------
bool ParticleDescriptionQuad::load(Iff &iff)
{
	bool result = true;

	if (iff.enterForm(TAG_PTQD, true))
	{
		ParticleDescription::load(iff);

		switch (iff.getCurrentName())
		{
			case TAG_0000:
				{
					load_0000(iff);
					break;
				}
			case TAG_0001:
				{
					load_0001(iff);
					break;
				}
			default:
				{
					result = false;

					// Gracefully  handle the unknown file and just initialize to default values

					char currentTagName[256];
					ConvertTagToString(iff.getCurrentName(), currentTagName);
					DEBUG_WARNING(true, ("ParticleDescriptionQuad::load() - Unable to load particle description due to unsupported data version: %s", currentTagName));
				}
			}
		iff.exitForm(TAG_PTQD);
	}
	else if (iff.enterForm(ParticleDescription::getTag(), true))
	{
		switch (iff.getCurrentName())
		{
			case TAG_0000:
				{
					load_old_0000(iff);
					break;
				}
			case TAG_0001:
				{
					load_old_0001(iff);
					break;
				}
			default:
				{
					result = false;

					// Gracefully  handle the unknown file and just initialize to default values

					char currentTagName[256];
					ConvertTagToString(iff.getCurrentName(), currentTagName);
					DEBUG_WARNING(true, ("ParticleDescriptionQuad::load() - Unable to load particle description due to unsupported data version: %s", currentTagName));
				}
		}
		iff.exitForm(ParticleDescription::getTag());
	}
	else
	{
		result = false;

		// Issue a warning

		char expectedTagName[256];
		ConvertTagToString(TAG_PTQD, expectedTagName);
		char currentTagName[256];
		ConvertTagToString(iff.getCurrentName(), currentTagName);
		DEBUG_WARNING(true, ("ParticleDescriptionQuad::load() - Unable to load particle description. Expecting tag(%s) but encountered(%s)", expectedTagName, currentTagName));
	}

	if (!result)
	{
		// Initialize to a default state

		initializeDefault();
	}

	return result;
}

//--------------------------------------------------------------------------
void ParticleDescriptionQuad::load_old_0000(Iff &iff)
{
	iff.enterForm(TAG_0000);
	{
		std::string shaderFilePath;

		iff.enterChunk(TAG_0000);
		{
			std::string name;
			iff.read_string(name);
			setName(name.c_str());
			std::string shaderPath;
			iff.read_string(shaderPath);
			setName(shaderPath.c_str());
			setRandomRotationDirection(iff.read_uint8() != 0);
		}
		iff.exitChunk();

		m_rotation.load(iff);
		m_length.load(iff);
		m_width.load(iff);
		m_color.load(iff);
		m_alpha.load(iff);
		m_speedScale.load(iff);
		m_particleTexture = ParticleTexture(shaderFilePath.c_str());
	}
	iff.exitForm(TAG_0000);
}

//--------------------------------------------------------------------------
void ParticleDescriptionQuad::load_old_0001(Iff &iff)
{
	iff.enterForm(TAG_0001);
	{
		iff.enterChunk(TAG_0000);
		{
			std::string name;
			iff.read_string(name);
			setName(name.c_str());
			setRandomRotationDirection(iff.read_uint8() != 0);
		}
		iff.exitChunk();

		m_rotation.load(iff);
		m_length.load(iff);
		m_width.load(iff);
		m_color.load(iff);
		m_alpha.load(iff);
		m_speedScale.load(iff);
		m_particleTexture.load(iff);
	}
	iff.exitForm(TAG_0001);
}

//--------------------------------------------------------------------------
void ParticleDescriptionQuad::load_0000(Iff &iff)
{
	iff.enterForm(TAG_0000);
	{
		m_rotation.load(iff);
		m_length.load(iff);
		m_width.load(iff);
		m_particleTexture.load(iff);
	}
	iff.exitForm(TAG_0000);
}

//--------------------------------------------------------------------------
void ParticleDescriptionQuad::load_0001(Iff &iff)
{
	iff.enterForm(TAG_0001);
	{
		m_rotation.load(iff);
		m_length.load(iff);
		m_width.load(iff);
		m_particleTexture.load(iff);

		iff.enterChunk(TAG_0000);
		{
			setLengthAndWidthLinked(iff.read_uint8() != 0);
		}
		iff.exitChunk();
	}
	iff.exitForm(TAG_0001);
}

//--------------------------------------------------------------------------
void ParticleDescriptionQuad::write(Iff &iff) const
{
	iff.insertForm(TAG_PTQD);
	{
		// Write the base class

		ParticleDescription::write(iff);

		// Write the class specific data

		iff.insertForm(TAG_0001);
		{
			m_rotation.write(iff);
			m_length.write(iff);
			m_width.write(iff);
			m_particleTexture.write(iff);

			iff.insertChunk(TAG_0000);
			{
				iff.insertChunkData(static_cast<uint8>(m_lengthAndWidthLinked));
			}
			iff.exitChunk();
		}
		iff.exitForm(TAG_0001);
	}
	iff.exitForm(TAG_PTQD);
}

// ============================================================================
