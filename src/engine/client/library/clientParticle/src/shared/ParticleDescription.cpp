// ============================================================================
//
// ParticleDescription.cpp
// Copyright Sony Online Entertainment
//
// ============================================================================

#include "clientParticle/FirstClientParticle.h"
#include "clientParticle/ParticleDescription.h"

#include "clientParticle/ParticleAttachmentDescription.h"
#include "clientParticle/ParticleDescriptionMesh.h"
#include "clientParticle/ParticleDescriptionQuad.h"
#include "sharedFile/Iff.h"
#include <vector>

// ============================================================================
//
// ParticleDescription
//
// ============================================================================

std::string const ParticleDescription::m_defaultName("Default");
bool const ParticleDescription::m_defaultRandomRotationDirection = false;
Tag const ParticleDescription::m_tag = TAG(P,T,C,L);

//-----------------------------------------------------------------------------
ParticleDescription::ParticleDescription()
 : m_name(m_defaultName)
 , m_randomRotationDirection(m_defaultRandomRotationDirection)
 , m_color()
 , m_alpha()
 , m_speedScale()
 , m_particleRelativeRotationX()
 , m_particleRelativeRotationY()
 , m_particleRelativeRotationZ()
 , m_particleAttachmentDescriptions(NULL)
 , m_particleUsesRelativeRotation(false)
{
	m_particleAttachmentDescriptions = new ParticleAttachmentDescriptions;
	NOT_NULL(m_particleAttachmentDescriptions);
}

//-----------------------------------------------------------------------------
ParticleDescription::ParticleDescription(ParticleDescription const &particleDescription)
 : m_name(particleDescription.m_name)
 , m_randomRotationDirection(particleDescription.m_randomRotationDirection)
 , m_color(particleDescription.m_color)
 , m_alpha(particleDescription.m_alpha)
 , m_speedScale(particleDescription.m_speedScale)
 , m_particleRelativeRotationX(particleDescription.m_particleRelativeRotationX)
 , m_particleRelativeRotationY(particleDescription.m_particleRelativeRotationY)
 , m_particleRelativeRotationZ(particleDescription.m_particleRelativeRotationZ)
 , m_particleAttachmentDescriptions(NULL)
 , m_particleUsesRelativeRotation(particleDescription.m_particleUsesRelativeRotation)
{
	m_particleAttachmentDescriptions = new ParticleAttachmentDescriptions;
	NOT_NULL(m_particleAttachmentDescriptions);

	*m_particleAttachmentDescriptions = *particleDescription.m_particleAttachmentDescriptions;

#ifdef _DEBUG
	int const size = m_particleAttachmentDescriptions->size();
	UNREF(size);
#endif // _DEBUG
}

//-----------------------------------------------------------------------------
ParticleDescription::~ParticleDescription()
{
	delete m_particleAttachmentDescriptions;
	m_particleAttachmentDescriptions = NULL;
}

//-----------------------------------------------------------------------------
ParticleDescription *ParticleDescription::createDescription(ParticleType const particleType)
{
	ParticleDescription *particleDescription = NULL;

	switch (particleType)
	{
		case PT_quad:
			{
				particleDescription = new ParticleDescriptionQuad();
				NOT_NULL(particleDescription);
			}
			break;
		case PT_mesh:
			{
				particleDescription = new ParticleDescriptionMesh();
				NOT_NULL(particleDescription);
			}
			break;
		default:
			{
				DEBUG_FATAL(true, ("Invalid particle type specified."));
			}
			break;
	}

	return particleDescription;
}

//--------------------------------------------------------------------------
Tag ParticleDescription::getTag()
{
	return m_tag;
}

//--------------------------------------------------------------------------
void ParticleDescription::initializeDefault()
{
	m_name = m_defaultName;
	m_randomRotationDirection = m_defaultRandomRotationDirection;

	setDefaultColor(m_color);
	setDefaultAlpha(m_alpha);
	setDefaultSpeedScale(m_speedScale);
	setDefaultParticleRelativeRotationX(m_particleRelativeRotationX);
	setDefaultParticleRelativeRotationY(m_particleRelativeRotationY);
	setDefaultParticleRelativeRotationZ(m_particleRelativeRotationZ);


	delete m_particleAttachmentDescriptions;
	m_particleAttachmentDescriptions = new ParticleAttachmentDescriptions;
	NOT_NULL(m_particleAttachmentDescriptions);
}

//--------------------------------------------------------------------------
void ParticleDescription::setDefaultColor(ColorRamp &colorRamp)
{
#ifdef _DEBUG
	colorRamp.setName("Particle - Color (RGB)");
#endif // _DEBUG
	colorRamp.clear();
	colorRamp.insert(ColorRampControlPoint(0.0f, 1.0f, 0.0f, 0.0f));
	colorRamp.insert(ColorRampControlPoint(1.0f, 0.0f, 1.0f, 0.0f));
}

//--------------------------------------------------------------------------
void ParticleDescription::setDefaultAlpha(WaveForm &waveForm)
{
#ifdef _DEBUG
	waveForm.setName("Particle - Alpha");
#endif // _DEBUG
	waveForm.clear();
	waveForm.setValueMax(1.0f);
	waveForm.setValueMin(0.0f);
	waveForm.insert(WaveFormControlPoint(0.0f, 0.0f));
	waveForm.insert(WaveFormControlPoint(0.2f, 1.0f));
	waveForm.insert(WaveFormControlPoint(0.8f, 1.0f));
	waveForm.insert(WaveFormControlPoint(1.0f, 0.0f));
}

//--------------------------------------------------------------------------
void ParticleDescription::setDefaultSpeedScale(WaveForm &waveForm)
{
#ifdef _DEBUG
	waveForm.setName("Particle - Speed Scale");
#endif // _DEBUG
	waveForm.clear();
	waveForm.setValueMax(4096.0f);
	waveForm.setValueMin(-4096.0f);
	waveForm.insert(WaveFormControlPoint(0.0f, 1.0f));
	waveForm.insert(WaveFormControlPoint(1.0f, 1.0f));
}


//--------------------------------------------------------------------------
void ParticleDescription::setDefaultParticleRelativeRotationX(WaveForm &waveForm)
{
#ifdef _DEBUG
	waveForm.setName("Particle - Local Rotation X");
#endif // _DEBUG
	waveForm.clear();
	waveForm.setValueMax(4096.0f);
	waveForm.setValueMin(-4096.0f);
	waveForm.insert(WaveFormControlPoint(0.0f, 0.0f));
	waveForm.insert(WaveFormControlPoint(1.0f, 0.0f));
}

//--------------------------------------------------------------------------
void ParticleDescription::setDefaultParticleRelativeRotationY(WaveForm &waveForm)
{
#ifdef _DEBUG
	waveForm.setName("Particle - Local Rotation Y");
#endif // _DEBUG
	waveForm.clear();
	waveForm.setValueMax(4096.0f);
	waveForm.setValueMin(-4096.0f);
	waveForm.insert(WaveFormControlPoint(0.0f, 0.0f));
	waveForm.insert(WaveFormControlPoint(1.0f, 0.0f));
}

//--------------------------------------------------------------------------
void ParticleDescription::setDefaultParticleRelativeRotationZ(WaveForm &waveForm)
{
#ifdef _DEBUG
	waveForm.setName("Particle - Local Rotation Z");
#endif // _DEBUG
	waveForm.clear();
	waveForm.setValueMax(4096.0f);
	waveForm.setValueMin(-4096.0f);
	waveForm.insert(WaveFormControlPoint(0.0f, 0.0f));
	waveForm.insert(WaveFormControlPoint(1.0f, 0.0f));
}

//--------------------------------------------------------------------------
void ParticleDescription::setName(char const *name)
{
	if (name != NULL)
	{
		m_name = name;
	}
	else
	{
		m_name.clear();
	}
}

//--------------------------------------------------------------------------
void ParticleDescription::setRandomRotationDirection(bool const randomRotationDirection)
{
	m_randomRotationDirection = randomRotationDirection;
}

//--------------------------------------------------------------------------
void ParticleDescription::setColor(ColorRamp const &color)
{
	m_color = color;
}

//--------------------------------------------------------------------------
void ParticleDescription::setAlpha(WaveForm const &alpha)
{
	m_alpha = alpha;
}

//--------------------------------------------------------------------------
void ParticleDescription::setSpeedScale(WaveForm const &speedScale)
{
	m_speedScale = speedScale;
}

//--------------------------------------------------------------------------
void ParticleDescription::setParticleRelativeRotationX(WaveForm const &particleRelativeRotationX)
{
	m_particleRelativeRotationX = particleRelativeRotationX;
}

//--------------------------------------------------------------------------
void ParticleDescription::setParticleRelativeRotationY(WaveForm const &particleRelativeRotationY)
{
	m_particleRelativeRotationY = particleRelativeRotationY;
}

//--------------------------------------------------------------------------
void ParticleDescription::setParticleRelativeRotationZ(WaveForm const &particleRelativeRotationZ)
{
	m_particleRelativeRotationZ = particleRelativeRotationZ;
}

//--------------------------------------------------------------------------
void ParticleDescription::setParticleAttachmentDescriptions(ParticleAttachmentDescriptions &particleAttachmentDescriptions)
{
	NOT_NULL(m_particleAttachmentDescriptions);
	m_particleAttachmentDescriptions->clear();
	m_particleAttachmentDescriptions->swap(particleAttachmentDescriptions);
}

//--------------------------------------------------------------------------
std::string const &ParticleDescription::getName() const
{
	return m_name;
}

//--------------------------------------------------------------------------
bool ParticleDescription::isRandomRotationDirection() const
{
	return m_randomRotationDirection;
}

//--------------------------------------------------------------------------
ColorRamp const &ParticleDescription::getColor() const
{
	return m_color;
}

//--------------------------------------------------------------------------
WaveForm const &ParticleDescription::getAlpha() const
{
	return m_alpha;
}

//--------------------------------------------------------------------------
WaveForm const &ParticleDescription::getSpeedScale() const
{
	return m_speedScale;
}

//--------------------------------------------------------------------------
WaveForm const &ParticleDescription::getParticleRelativeRotationX() const
{
	return m_particleRelativeRotationX;
}

//--------------------------------------------------------------------------
WaveForm const &ParticleDescription::getParticleRelativeRotationY() const
{
	return m_particleRelativeRotationY;
}

//--------------------------------------------------------------------------
WaveForm const &ParticleDescription::getParticleRelativeRotationZ() const
{
	return m_particleRelativeRotationZ;
}

//--------------------------------------------------------------------------
ParticleDescription::ParticleAttachmentDescriptions const &ParticleDescription::getParticleAttachmentDescriptions() const
{
	NOT_NULL(m_particleAttachmentDescriptions);

	return *m_particleAttachmentDescriptions;
}

//--------------------------------------------------------------------------

bool const ParticleDescription::getUsesParticleRelativeRotation() const
{
	return m_particleUsesRelativeRotation;
}

//--------------------------------------------------------------------------
void ParticleDescription::clearParticleAttachmentDescriptions()
{
	NOT_NULL(m_particleAttachmentDescriptions);

	m_particleAttachmentDescriptions->clear();
}

//--------------------------------------------------------------------------
bool ParticleDescription::isInfiniteLooping() const
{
	bool result = false;

	if (m_particleAttachmentDescriptions != NULL)
	{
		ParticleAttachmentDescriptions::const_iterator iterParticleAttachmentDescriptions = m_particleAttachmentDescriptions->begin();

		for (; iterParticleAttachmentDescriptions != m_particleAttachmentDescriptions->end(); ++iterParticleAttachmentDescriptions)
		{
			if (iterParticleAttachmentDescriptions->isInfiniteLooping())
			{
				result = true;
				break;
			}
		}
	}

	return result;
}

//--------------------------------------------------------------------------
bool ParticleDescription::load(Iff &iff)
{
	bool result = true;

	if (iff.enterForm(m_tag, true))
	{
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
			case TAG_0002:
				{
					load_0002(iff);
					break;
				}				
			case TAG_0003:
				{
					load_0003(iff);
					break;
				}
			default:
				{
					result = false;

					// Gracefully  handle the unknown file and just initialize to default values

					char currentTagName[256];
					ConvertTagToString(iff.getCurrentName(), currentTagName);
					DEBUG_WARNING(true, ("Unable to load particle description due to unsupported data version: %s", currentTagName));
				}
		}
		iff.exitForm(m_tag);
	}
	else
	{
		result = false;

		// Issue a warning

		char expectedTagName[256];
		ConvertTagToString(m_tag, expectedTagName);
		char currentTagName[256];
		ConvertTagToString(iff.getCurrentName(), currentTagName);
		DEBUG_WARNING(true, ("Unable to load particle description. Expecting tag(%s) but encountered(%s)", expectedTagName, currentTagName));
	}

	if (!result)
	{
		// Initialize to a default state

		initializeDefault();
	}

	// This needs to be removed and data needs to be validated

	m_alpha.clampAll(0.0f, 1.0f);

	return result;
}

//--------------------------------------------------------------------------
void ParticleDescription::load_0000(Iff &iff)
{
	delete m_particleAttachmentDescriptions;
	m_particleAttachmentDescriptions = new ParticleAttachmentDescriptions;
	NOT_NULL(m_particleAttachmentDescriptions);

	iff.enterForm(TAG_0000);
	{
		iff.enterChunk(TAG_0000);
		{
			std::string name;
			iff.read_string(name);
			setName(name.c_str());
			setRandomRotationDirection(iff.read_uint8() != 0);
		}
		iff.exitChunk();

		m_color.load(iff);
		m_alpha.load(iff);
		m_speedScale.load(iff);
	}
	iff.exitForm(TAG_0000);
	
	setDefaultParticleRelativeRotationX(m_particleRelativeRotationX);	
	setDefaultParticleRelativeRotationY(m_particleRelativeRotationY);	
	setDefaultParticleRelativeRotationZ(m_particleRelativeRotationZ);
	m_particleUsesRelativeRotation = false;
}

//--------------------------------------------------------------------------
void ParticleDescription::load_0001(Iff &iff)
{
	delete m_particleAttachmentDescriptions;
	m_particleAttachmentDescriptions = new ParticleAttachmentDescriptions;
	NOT_NULL(m_particleAttachmentDescriptions);

	iff.enterForm(TAG_0001);
	{
		iff.enterChunk(TAG_0000);
		{
			std::string name;
			iff.read_string(name);
			setName(name.c_str());
			setRandomRotationDirection(iff.read_uint8() != 0);

			std::string dummyString;
			iff.read_string(dummyString);
		}
		iff.exitChunk();

		m_color.load(iff);
		m_alpha.load(iff);
		m_speedScale.load(iff);
	}
	iff.exitForm(TAG_0001);
	
	setDefaultParticleRelativeRotationX(m_particleRelativeRotationX);	
	setDefaultParticleRelativeRotationY(m_particleRelativeRotationY);	
	setDefaultParticleRelativeRotationZ(m_particleRelativeRotationZ);
	m_particleUsesRelativeRotation = false;
}

//--------------------------------------------------------------------------
void ParticleDescription::load_0002(Iff &iff)
{
	delete m_particleAttachmentDescriptions;
	m_particleAttachmentDescriptions = new ParticleAttachmentDescriptions;
	NOT_NULL(m_particleAttachmentDescriptions);

	iff.enterForm(TAG_0002);
	{
		int attachmentCount = 0;

		iff.enterChunk(TAG_0000);
		{
			std::string name;
			iff.read_string(name);
			setName(name.c_str());
			setRandomRotationDirection(iff.read_uint8() != 0);

			attachmentCount = iff.read_int32();
		}
		iff.exitChunk();

		m_color.load(iff);
		m_alpha.load(iff);
		m_speedScale.load(iff);

		for (int i = 0; i < attachmentCount; ++i)
		{
			ParticleAttachmentDescription particleAttachmentDescription;
			particleAttachmentDescription.load(iff);

			m_particleAttachmentDescriptions->push_back(particleAttachmentDescription);
		}
	}
	iff.exitForm(TAG_0002);

	setDefaultParticleRelativeRotationX(m_particleRelativeRotationX);	
	setDefaultParticleRelativeRotationY(m_particleRelativeRotationY);	
	setDefaultParticleRelativeRotationZ(m_particleRelativeRotationZ);
	m_particleUsesRelativeRotation = false;
}

//--------------------------------------------------------------------------
void ParticleDescription::load_0003(Iff &iff)
{
	delete m_particleAttachmentDescriptions;
	m_particleAttachmentDescriptions = new ParticleAttachmentDescriptions;
	NOT_NULL(m_particleAttachmentDescriptions);

	iff.enterForm(TAG_0003);
	{
		int attachmentCount = 0;

		iff.enterChunk(TAG_0000);
		{
			std::string name;
			iff.read_string(name);
			setName(name.c_str());
			setRandomRotationDirection(iff.read_uint8() != 0);

			attachmentCount = iff.read_int32();
		}
		iff.exitChunk();

		m_color.load(iff);
		m_alpha.load(iff);
		m_speedScale.load(iff);
		m_particleRelativeRotationX.load(iff);
		m_particleRelativeRotationY.load(iff);
		m_particleRelativeRotationZ.load(iff);

		for (int i = 0; i < attachmentCount; ++i)
		{
			ParticleAttachmentDescription particleAttachmentDescription;
			particleAttachmentDescription.load(iff);

			m_particleAttachmentDescriptions->push_back(particleAttachmentDescription);
		}
	}
	iff.exitForm(TAG_0003);
	
	m_particleUsesRelativeRotation = true;
	if(m_particleRelativeRotationX.isConstantValue(0.0f) && m_particleRelativeRotationY.isConstantValue(0.0f) && m_particleRelativeRotationZ.isConstantValue(0.0f))
		m_particleUsesRelativeRotation = false;
}

//--------------------------------------------------------------------------
void ParticleDescription::write(Iff &iff) const
{
	iff.insertForm(m_tag);
	{
		iff.insertForm(TAG_0003);
		{
			iff.insertChunk(TAG_0000);
			{
				iff.insertChunkString(m_name.c_str());
				iff.insertChunkData(static_cast<uint8>(m_randomRotationDirection));
				iff.insertChunkData(static_cast<int32>(m_particleAttachmentDescriptions->size()));
			}
			iff.exitChunk();

			m_color.write(iff);
			m_alpha.write(iff);
			m_speedScale.write(iff);
			m_particleRelativeRotationX.write(iff);
			m_particleRelativeRotationY.write(iff);
			m_particleRelativeRotationZ.write(iff);

			ParticleAttachmentDescriptions::const_iterator iterParticleAttachmentDescriptions = m_particleAttachmentDescriptions->begin();

			for (; iterParticleAttachmentDescriptions != m_particleAttachmentDescriptions->end(); ++iterParticleAttachmentDescriptions)
			{
				iterParticleAttachmentDescriptions->write(iff);
			}
		}
		iff.exitForm(TAG_0003);
	}
	iff.exitForm(m_tag);
}

// ============================================================================
