// ============================================================================
//
// ParticleEmitterGroup.cpp
// Copyright Sony Online Entertainment
//
// ============================================================================

#include "clientParticle/FirstClientParticle.h"
#include "clientParticle/ParticleEmitterGroupDescription.h"

#include "clientParticle/ParticleEmitterDescription.h"
#include "sharedFile/Iff.h"
#include <vector>

#include "clientParticle/ParticleTexture.h"

// ============================================================================
//
// ParticleEmitterGroupDescription
//
// ============================================================================

int ParticleEmitterGroupDescription::m_globalCount = 0;

#define TAG_EMGP TAG(E,M,G,P)

//-----------------------------------------------------------------------------
ParticleEmitterGroupDescription::ParticleEmitterGroupDescription()
 : m_particleEmitterDescriptions(NULL)
 , m_timing()
{
	++m_globalCount;

	m_particleEmitterDescriptions = new ParticleEmitterDescriptions;
	NOT_NULL(m_particleEmitterDescriptions);
}

//-----------------------------------------------------------------------------
ParticleEmitterGroupDescription::ParticleEmitterGroupDescription(ParticleEmitterGroupDescription const &rhs)
 : m_particleEmitterDescriptions(NULL)
 , m_timing(rhs.m_timing)
{
	m_particleEmitterDescriptions = new ParticleEmitterDescriptions;
	NOT_NULL(m_particleEmitterDescriptions);

	*m_particleEmitterDescriptions = *rhs.m_particleEmitterDescriptions;

	validate();
}

//-----------------------------------------------------------------------------
ParticleEmitterGroupDescription::~ParticleEmitterGroupDescription()
{
	--m_globalCount;

	// Delete the list

	delete m_particleEmitterDescriptions;
}

//--------------------------------------------------------------------------
ParticleEmitterGroupDescription &ParticleEmitterGroupDescription::operator =(ParticleEmitterGroupDescription const &rhs)
{
	if (this != &rhs)
	{
		NOT_NULL(m_particleEmitterDescriptions);

		*m_particleEmitterDescriptions = *rhs.m_particleEmitterDescriptions;
		m_timing = rhs.m_timing;

		validate();
	}

	return *this;
}

//--------------------------------------------------------------------------
void ParticleEmitterGroupDescription::initializeDefault()
{
	m_timing.reset();

	// Clear the list

	m_particleEmitterDescriptions->clear();

	// Put a single description in the list

	ParticleEmitterDescription particleEmitterDescription;
	particleEmitterDescription.initializeDefault();

	m_particleEmitterDescriptions->push_back(particleEmitterDescription);
}

//--------------------------------------------------------------------------
int ParticleEmitterGroupDescription::getGlobalCount()
{
	return m_globalCount;
}

//--------------------------------------------------------------------------
void ParticleEmitterGroupDescription::setParticleEmitterDescriptions(ParticleEmitterDescriptions &particleEmitterDescriptions)
{
	NOT_NULL(m_particleEmitterDescriptions);
	m_particleEmitterDescriptions->clear();
	m_particleEmitterDescriptions->swap(particleEmitterDescriptions);
}

//--------------------------------------------------------------------------
void ParticleEmitterGroupDescription::setParticleTiming(ParticleTiming const &particleTiming)
{
	m_timing = particleTiming;
}

//--------------------------------------------------------------------------
ParticleEmitterGroupDescription::ParticleEmitterDescriptions const &ParticleEmitterGroupDescription::getParticleEmitterDescriptions() const
{
	NOT_NULL(m_particleEmitterDescriptions);

	return *m_particleEmitterDescriptions;
}

//--------------------------------------------------------------------------
ParticleTiming const &ParticleEmitterGroupDescription::getParticleTiming() const
{
	return m_timing;
}

//--------------------------------------------------------------------------
void ParticleEmitterGroupDescription::clearParticleEmitterDescriptions()
{
	NOT_NULL(m_particleEmitterDescriptions);

	m_particleEmitterDescriptions->clear();
}

//--------------------------------------------------------------------------
bool ParticleEmitterGroupDescription::isInfiniteLooping() const
{
	bool result = (m_timing.getLoopCountMin() == -1);

	if (!result &&
	    (m_particleEmitterDescriptions != NULL))
	{
		ParticleEmitterDescriptions::const_iterator iterParticleEmitterDescriptions = m_particleEmitterDescriptions->begin();

		for (; iterParticleEmitterDescriptions != m_particleEmitterDescriptions->end(); ++iterParticleEmitterDescriptions)
		{
			if (iterParticleEmitterDescriptions->isInfiniteLooping())
			{
				result = true;
				break;
			}
		}
	}

	return result;
}

//--------------------------------------------------------------------------
void ParticleEmitterGroupDescription::load(Iff &iff)
{
	iff.enterForm(TAG_EMGP);
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
			default:
				{
					char currentTagName[256];
					ConvertTagToString(iff.getCurrentName(), currentTagName);
		
					FATAL(true, ("ParticleEmitterGroupDescription::load() - Unsupported data version: %s", currentTagName));
				}
		}
	}
	iff.exitForm(TAG_EMGP);

	// Make sure there is at least 1 item in the description list

	DEBUG_FATAL(m_particleEmitterDescriptions->empty(), ("ParticleEmitterGroupDescription::load() - There must be at least one item in the emitter group description list."));

	validate();
}

//--------------------------------------------------------------------------
void ParticleEmitterGroupDescription::load_0000(Iff &iff)
{
	iff.enterForm(TAG_0000);
	{
		int emitterDescriptionCount = 0;

		iff.enterChunk(TAG_0000);
		{
			// Get the particle emitter description count

			emitterDescriptionCount = iff.read_int32();
		}
		iff.exitChunk(TAG_0000);

		// Create and load the particle emitter descriptions

		for (int i = 0; i < emitterDescriptionCount; ++i)
		{
			ParticleEmitterDescription particleEmitterDescription;
			particleEmitterDescription.load(iff);

			m_particleEmitterDescriptions->push_back(particleEmitterDescription);
		}
	}
	iff.exitForm(TAG_0000);
}

//--------------------------------------------------------------------------
void ParticleEmitterGroupDescription::load_0001(Iff &iff)
{
	iff.enterForm(TAG_0001);
	{
		// Get the timing information

		m_timing.load(iff);

		// Get the particle emitter description count

		int emitterDescriptionCount = 0;

		iff.enterChunk(TAG_0000);
		{
			emitterDescriptionCount = iff.read_int32();
		}
		iff.exitChunk(TAG_0000);

		// Create and load the particle emitter descriptions

		for (int i = 0; i < emitterDescriptionCount; ++i)
		{
			ParticleEmitterDescription particleEmitterDescription;
			particleEmitterDescription.load(iff);

			m_particleEmitterDescriptions->push_back(particleEmitterDescription);
		}
	}
	iff.exitForm(TAG_0001);
}

//--------------------------------------------------------------------------
void ParticleEmitterGroupDescription::write(Iff &iff) const
{
	iff.insertForm(TAG_EMGP);
	{
		iff.insertForm(TAG_0001);
		{
			// Save the timing information

			m_timing.write(iff);

			// Save the number of particle emitter descriptions

			iff.insertChunk(TAG_0000);
			{
				int const emitterDescriptionCount = static_cast<int>(m_particleEmitterDescriptions->size());
				iff.insertChunkData(static_cast<int32>(emitterDescriptionCount));
			}
			iff.exitChunk(TAG_0000);

			// Save all the particleEmitter descriptions

			ParticleEmitterDescriptions::const_iterator current = m_particleEmitterDescriptions->begin();

			for (; current != m_particleEmitterDescriptions->end(); ++current)
			{
				current->write(iff);
			}
		}
		iff.exitForm(TAG_0001);
	}
	iff.exitForm(TAG_EMGP);
}

#ifdef _DEBUG
//--------------------------------------------------------------------------
void ParticleEmitterGroupDescription::validate()
{
	m_timing.validate();
}
#endif // _DEBUG

// ============================================================================
