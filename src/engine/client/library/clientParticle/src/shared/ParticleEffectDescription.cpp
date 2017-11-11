// ============================================================================
//
// ParticleEffectDescription.cpp
// Copyright Sony Online Entertainment
//
// ============================================================================

#include "clientParticle/FirstClientParticle.h"
#include "clientParticle/ParticleEffectAppearance.h"

#include "clientParticle/ParticleEffectAppearanceTemplate.h"
#include "sharedFile/Iff.h"
#include <vector>


// ============================================================================
//
// ParticleEffectDescription
//
// ============================================================================

//--------------------------------------------------------------------------
ParticleEffectDescription::ParticleEffectDescription()
 : m_name("")
 , m_timing()
 , m_particleEmitterGroupDescriptions(NULL)
 , m_initialPlayBackRate(1.0f)
 , m_initialPlayBackRateTime(0.0f)
 , m_playBackRate(1.0f)
 , m_scale(1.0f)
{
	m_particleEmitterGroupDescriptions = new ParticleEmitterGroupDescriptions();
	NOT_NULL(m_particleEmitterGroupDescriptions);
}

//--------------------------------------------------------------------------
ParticleEffectDescription::~ParticleEffectDescription()
{
	// Delete the list

	delete m_particleEmitterGroupDescriptions;
}

//--------------------------------------------------------------------------
ParticleEffectDescription::ParticleEffectDescription(ParticleEffectDescription const &rhs)
 : m_name(rhs.m_name)
 , m_timing(rhs.m_timing)
 , m_particleEmitterGroupDescriptions(NULL)
 , m_initialPlayBackRate(rhs.m_initialPlayBackRate)
 , m_initialPlayBackRateTime(rhs.m_initialPlayBackRateTime)
 , m_playBackRate(rhs.m_playBackRate)
 , m_scale(rhs.m_scale)
{
	m_particleEmitterGroupDescriptions = new ParticleEmitterGroupDescriptions();
	NOT_NULL(m_particleEmitterGroupDescriptions);
	*m_particleEmitterGroupDescriptions = *rhs.m_particleEmitterGroupDescriptions;

#ifdef _DEBUG
		int count = static_cast<int>(m_particleEmitterGroupDescriptions->size());
		UNREF(count);
#endif // _DEBUG
}

//--------------------------------------------------------------------------
ParticleEffectDescription &ParticleEffectDescription::operator =(ParticleEffectDescription const &rhs)
{
	if (this == &rhs)
	{
		return *this;
	}

	m_name = rhs.m_name;
	m_timing = rhs.m_timing;
	m_initialPlayBackRate = rhs.m_initialPlayBackRate;
	m_initialPlayBackRateTime = rhs.m_initialPlayBackRateTime;
	m_playBackRate = rhs.m_playBackRate;
	m_scale = rhs.m_scale;

	NOT_NULL(m_particleEmitterGroupDescriptions);

	*m_particleEmitterGroupDescriptions = *rhs.m_particleEmitterGroupDescriptions;

#ifdef _DEBUG
	int count = static_cast<int>(m_particleEmitterGroupDescriptions->size());
	UNREF(count);
#endif // _DEBUG

	return *this;
}

//--------------------------------------------------------------------------
void ParticleEffectDescription::initializeDefault()
{
	m_name = "";
	m_timing.reset();
	m_initialPlayBackRate = 1.0f;
	m_initialPlayBackRateTime = 0.0f;
	m_playBackRate = 1.0f;
	m_scale = 1.0f;

	// Clear the list

	m_particleEmitterGroupDescriptions->clear();

	// Put a single description in the list

	ParticleEmitterGroupDescription particleEmitterGroupDescription;
	particleEmitterGroupDescription.initializeDefault();

	m_particleEmitterGroupDescriptions->push_back(particleEmitterGroupDescription);
}

//--------------------------------------------------------------------------
void ParticleEffectDescription::setName(char const *name)
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
void ParticleEffectDescription::setParticleEmitterGroupDescriptions(ParticleEmitterGroupDescriptions &particleEmitterGroupDescriptions)
{
	NOT_NULL(m_particleEmitterGroupDescriptions);
	m_particleEmitterGroupDescriptions->clear();
	m_particleEmitterGroupDescriptions->swap(particleEmitterGroupDescriptions);
}

//--------------------------------------------------------------------------
void ParticleEffectDescription::setParticleTiming(ParticleTiming const &particleTiming)
{
	m_timing = particleTiming;
}

//--------------------------------------------------------------------------
std::string const &ParticleEffectDescription::getName() const
{
	return m_name;
}

//--------------------------------------------------------------------------
ParticleEffectDescription::ParticleEmitterGroupDescriptions const &ParticleEffectDescription::getParticleEmitterGroupDescriptions() const
{
	NOT_NULL(m_particleEmitterGroupDescriptions);

	return *m_particleEmitterGroupDescriptions;
}

//--------------------------------------------------------------------------
ParticleTiming const &ParticleEffectDescription::getParticleTiming() const
{
	return m_timing;
}

//--------------------------------------------------------------------------
void ParticleEffectDescription::clearParticleEmitterGroupDescriptions()
{
	NOT_NULL(m_particleEmitterGroupDescriptions);

	m_particleEmitterGroupDescriptions->clear();
}

//--------------------------------------------------------------------------
bool ParticleEffectDescription::isInfiniteLooping() const
{
	bool result = false;
	
	if (m_particleEmitterGroupDescriptions != NULL)
	{
		ParticleEmitterGroupDescriptions::const_iterator iterParticleEmitterGroupDescriptions = m_particleEmitterGroupDescriptions->begin();

		for (; iterParticleEmitterGroupDescriptions != m_particleEmitterGroupDescriptions->end(); ++iterParticleEmitterGroupDescriptions)
		{
			if (iterParticleEmitterGroupDescriptions->isInfiniteLooping())
			{
				result = true;
				break;
			}
		}
	}

	return result;
}

//--------------------------------------------------------------------------
void ParticleEffectDescription::setInitialPlayBackRate(float const initialPlayBackRate)
{
	DEBUG_FATAL((initialPlayBackRate < 1.0f), ("initialPlayBackRate(%f) is to be used for building out an effect quickly. Therefore the value should be >= 1.0f", initialPlayBackRate));

	m_initialPlayBackRate = initialPlayBackRate;
}

//--------------------------------------------------------------------------
float ParticleEffectDescription::getInitialPlayBackRate() const
{
	return m_initialPlayBackRate;
}

//--------------------------------------------------------------------------
void ParticleEffectDescription::setInitialPlayBackRateTime(float const initialPlayBackRateTime)
{
	DEBUG_FATAL((initialPlayBackRateTime > 8.0f), ("initialPlayBackRateTime(%f) should only be used for a short amount of time to build out an effect", initialPlayBackRateTime));

	m_initialPlayBackRateTime = initialPlayBackRateTime;
}

//--------------------------------------------------------------------------
float ParticleEffectDescription::getInitialPlayBackRateTime() const
{
	return m_initialPlayBackRateTime;
}

//--------------------------------------------------------------------------
void ParticleEffectDescription::setPlayBackRate(float const playBackRate)
{
	DEBUG_FATAL((playBackRate <= 0.0f), ("playBackRate(%f) needs to be > 0", playBackRate));

	m_playBackRate = playBackRate;
}

//--------------------------------------------------------------------------
float ParticleEffectDescription::getPlayBackRate() const
{
	return m_playBackRate;
}

//--------------------------------------------------------------------------
void ParticleEffectDescription::setScale(float const scale)
{
	DEBUG_FATAL((scale <= 0.0f), ("scale(%f) must be > 0", scale));

	m_scale = scale;
}

//--------------------------------------------------------------------------
float ParticleEffectDescription::getScale() const
{
	return m_scale;
}

//--------------------------------------------------------------------------
void ParticleEffectDescription::load(Iff &iff)
{
	// Save the filename

	if (iff.getFileName() != NULL)
	{
		m_name = iff.getFileName();
	}
	else
	{
		m_name = "";
	}

	iff.enterForm(ParticleEffectAppearanceTemplate::getTag());
	{
		switch (iff.getCurrentName())
		{
			case TAG_0000: { load_0000(iff); } break;
			case TAG_0001: { load_0001(iff); } break;
			case TAG_0002: { load_0002(iff); } break;
			default:
				{
					char currentTagName[256];
					ConvertTagToString(iff.getCurrentName(), currentTagName);
		
					FATAL(true, ("Unsupported data version: %s", currentTagName));
				}
		}
	}
	iff.exitForm(ParticleEffectAppearanceTemplate::getTag());

	iff.allowNonlinearFunctions();
	iff.goToTopOfForm();

	// Make sure there is at least 1 item in the description list

	DEBUG_FATAL(m_particleEmitterGroupDescriptions->empty(), ("The description list should have at least one item, it is currently empty."));
}

//--------------------------------------------------------------------------
void ParticleEffectDescription::load_0000(Iff &iff)
{
	iff.enterForm(TAG_0000);
	{
		int emitterGroupDescriptionCount = 0;

		iff.enterChunk(TAG_0000);
		{
			// Get the number of emitter group descriptions

			emitterGroupDescriptionCount = iff.read_int32();
		}
		iff.exitChunk(TAG_0000);

		// Create and initialize the emitter group descriptions

		for (int i = 0; i < emitterGroupDescriptionCount; ++i)
		{
			ParticleEmitterGroupDescription particleEmitterGroupDescription;

			particleEmitterGroupDescription.load(iff);

			m_particleEmitterGroupDescriptions->push_back(particleEmitterGroupDescription);
		}
	}
	iff.exitForm(TAG_0000);
}

//--------------------------------------------------------------------------
void ParticleEffectDescription::load_0001(Iff &iff)
{
	iff.enterForm(TAG_0001);
	{
		// Get the timing information

		m_timing.load(iff);
		
		// Get the number of emitter group descriptions

		int emitterGroupDescriptionCount = 0;

		iff.enterChunk(TAG_0000);
		{
			emitterGroupDescriptionCount = iff.read_int32();
		}
		iff.exitChunk(TAG_0000);

		// Create and initialize the emitter group descriptions

		for (int i = 0; i < emitterGroupDescriptionCount; ++i)
		{
			ParticleEmitterGroupDescription particleEmitterGroupDescription;

			particleEmitterGroupDescription.load(iff);

			m_particleEmitterGroupDescriptions->push_back(particleEmitterGroupDescription);
		}
	}
	iff.exitForm(TAG_0001);
}

//--------------------------------------------------------------------------
void ParticleEffectDescription::load_0002(Iff &iff)
{
	iff.enterForm(TAG_0002);
	{
		// Get the timing information

		m_timing.load(iff);
		
		// Get the number of emitter group descriptions

		int emitterGroupDescriptionCount = 0;

		iff.enterChunk(TAG_0000);
		{
			emitterGroupDescriptionCount = iff.read_int32();
			m_initialPlayBackRate = iff.read_float();
			m_initialPlayBackRateTime = iff.read_float();
			m_playBackRate = iff.read_float();
			m_scale = iff.read_float();
		}
		iff.exitChunk(TAG_0000);

		// Create and initialize the emitter group descriptions

		for (int i = 0; i < emitterGroupDescriptionCount; ++i)
		{
			ParticleEmitterGroupDescription particleEmitterGroupDescription;

			particleEmitterGroupDescription.load(iff);

			m_particleEmitterGroupDescriptions->push_back(particleEmitterGroupDescription);
		}
	}
	iff.exitForm(TAG_0002);
}

//--------------------------------------------------------------------------
void ParticleEffectDescription::write(Iff &iff) const
{
	iff.insertForm(ParticleEffectAppearanceTemplate::getTag());
	{
		iff.insertForm(TAG_0002);
		{
			// Save the timing information

			m_timing.write(iff);

			// Save the number of emitter group descriptions

			iff.insertChunk(TAG_0000);
			{
				int const emitterGroupDescriptionCount = static_cast<int>(m_particleEmitterGroupDescriptions->size());
				iff.insertChunkData(static_cast<uint32>(emitterGroupDescriptionCount));
				iff.insertChunkData(m_initialPlayBackRate);
				iff.insertChunkData(m_initialPlayBackRateTime);
				iff.insertChunkData(m_playBackRate);
				iff.insertChunkData(m_scale);
			}
			iff.exitChunk(TAG_0000);

			// Save all the emitter groups

			ParticleEmitterGroupDescriptions::const_iterator current = m_particleEmitterGroupDescriptions->begin();

			for (; current != m_particleEmitterGroupDescriptions->end(); ++current)
			{
				current->write(iff);
			}
		}
		iff.exitForm(TAG_0002);
	}
	iff.exitForm(ParticleEffectAppearanceTemplate::getTag());

	iff.allowNonlinearFunctions();
	iff.goToTopOfForm();
}

// ============================================================================
