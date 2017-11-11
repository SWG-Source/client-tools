// ============================================================================
//
// ParticleAttachmentDescription.cpp
// Copyright Sony Online Entertainment
//
// ============================================================================

#include "clientParticle/FirstClientParticle.h"
#include "clientParticle/ParticleAttachmentDescription.h"

#include "clientParticle/ParticleEffectAppearance.h"
#include "sharedFile/Iff.h"
#include "sharedObject/AppearanceTemplate.h"
#include "sharedObject/AppearanceTemplateList.h"

// ============================================================================
//
// ParticleAttachmentDescription
//
// ============================================================================

Tag const ParticleAttachmentDescription::m_tag = TAG(P,A,T,T);

//-----------------------------------------------------------------------------
ParticleAttachmentDescription::ParticleAttachmentDescription()
 : m_startPercentMin(0.0f)
 , m_startPercentMax(0.0f)
 , m_killAttachmentWhenParticleDies(true)
 , m_attachmentPath()
 , m_cachedAppearanceTemplate(NULL)
 , m_spawn(S_atPercent)
{
}

//-----------------------------------------------------------------------------
ParticleAttachmentDescription::~ParticleAttachmentDescription()
{
	if (m_cachedAppearanceTemplate != NULL)
	{
		AppearanceTemplateList::release(m_cachedAppearanceTemplate);
		m_cachedAppearanceTemplate = NULL;
	}
}

//-----------------------------------------------------------------------------
ParticleAttachmentDescription::ParticleAttachmentDescription(ParticleAttachmentDescription const &rhs)
 : m_cachedAppearanceTemplate(NULL)
{
	setAttachmentPath(rhs.getAttachmentPath());
	setStartPercent(rhs.getStartPercentMin(), rhs.getStartPercentMax());
	setKillAttachmentWhenParticleDies(rhs.isKillAttachmentWhenParticleDies());
	setSpawn(rhs.getSpawn());
}

//-----------------------------------------------------------------------------
ParticleAttachmentDescription &ParticleAttachmentDescription::operator =(ParticleAttachmentDescription const &rhs)
{
	if (this != &rhs)
	{
		setAttachmentPath(rhs.getAttachmentPath());
		setStartPercent(rhs.getStartPercentMin(), rhs.getStartPercentMax());
		setKillAttachmentWhenParticleDies(rhs.isKillAttachmentWhenParticleDies());
		setSpawn(rhs.getSpawn());
	}

	return *this;
}

//-----------------------------------------------------------------------------
Tag ParticleAttachmentDescription::getTag()
{
	return m_tag;
}

//-----------------------------------------------------------------------------
void ParticleAttachmentDescription::initializeDefault()
{
	setAttachmentPath("");
	setStartPercent(0.0f, 0.0f);
	setKillAttachmentWhenParticleDies(true);
}

//-----------------------------------------------------------------------------
void ParticleAttachmentDescription::setAttachmentPath(std::string const &path)
{
	if (m_attachmentPath != path)
	{
		if (m_cachedAppearanceTemplate != NULL)
		{
			AppearanceTemplateList::release(m_cachedAppearanceTemplate);
			m_cachedAppearanceTemplate = NULL;
		}

		m_attachmentPath = path;

		if (!path.empty())
		{
			m_cachedAppearanceTemplate = AppearanceTemplateList::fetch(path.c_str());
		}
	}
}

//-----------------------------------------------------------------------------
void ParticleAttachmentDescription::setStartPercent(float const min, float const max)
{
	m_startPercentMin = min;
	m_startPercentMax = max;

	if (min > max)
	{
		std::swap(m_startPercentMin, m_startPercentMax);
	}
}

//-----------------------------------------------------------------------------
void ParticleAttachmentDescription::setKillAttachmentWhenParticleDies(bool const killAttachmentWhenParticleDies)
{
	m_killAttachmentWhenParticleDies = killAttachmentWhenParticleDies;
}

//-----------------------------------------------------------------------------
std::string const &ParticleAttachmentDescription::getAttachmentPath() const
{
	return m_attachmentPath;
}

//-----------------------------------------------------------------------------
float ParticleAttachmentDescription::getStartPercentMin() const
{
	return m_startPercentMin;
}

//-----------------------------------------------------------------------------
float ParticleAttachmentDescription::getStartPercentMax() const
{
	return m_startPercentMax;
}

//-----------------------------------------------------------------------------
bool ParticleAttachmentDescription::isKillAttachmentWhenParticleDies() const
{
	return m_killAttachmentWhenParticleDies;
}

//-----------------------------------------------------------------------------
void ParticleAttachmentDescription::setSpawn(Spawn const spawn)
{
	m_spawn = spawn;
}

//-----------------------------------------------------------------------------
ParticleAttachmentDescription::Spawn ParticleAttachmentDescription::getSpawn() const
{
	return m_spawn;
}

//-----------------------------------------------------------------------------
bool ParticleAttachmentDescription::isInfiniteLooping() const
{
	bool result = false;

	ParticleEffectAppearance const *particleEffectAppearance = dynamic_cast<ParticleEffectAppearance const *>(m_cachedAppearanceTemplate);

	if (particleEffectAppearance != NULL)
	{
		result = particleEffectAppearance->isInfiniteLooping();
	}

	return result;
}

//-----------------------------------------------------------------------------
bool ParticleAttachmentDescription::load(Iff &iff)
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
			default:
				{
					result = false;

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

		char expectedTagName[256];
		ConvertTagToString(m_tag, expectedTagName);
		char currentTagName[256];
		ConvertTagToString(iff.getCurrentName(), currentTagName);
		DEBUG_WARNING(true, ("Unable to load particle description. Expecting tag(%s) but encountered(%s)", expectedTagName, currentTagName));
	}

	if (!result)
	{
		initializeDefault();
	}

	return result;
}

//-----------------------------------------------------------------------------
void ParticleAttachmentDescription::load_0000(Iff &iff)
{
	iff.enterForm(TAG_0000);
	{
		iff.enterChunk(TAG_0000);
		{
			std::string attachmentPath;
			iff.read_string(attachmentPath);
			setAttachmentPath(attachmentPath);
			setStartPercent(iff.read_float(), iff.read_float());
			iff.read_int32();
			iff.read_int32();
			iff.read_int32();
			iff.read_int32();
			iff.read_int32();
			setKillAttachmentWhenParticleDies((iff.read_bool8() != 0));
			setSpawn(S_atPercent);
		}
		iff.exitChunk();
	}
	iff.exitForm(TAG_0000);
}

//-----------------------------------------------------------------------------
void ParticleAttachmentDescription::load_0001(Iff &iff)
{
	iff.enterForm(TAG_0001);
	{
		iff.enterChunk(TAG_0000);
		{
			std::string attachmentPath;
			iff.read_string(attachmentPath);
			setAttachmentPath(attachmentPath);
			setStartPercent(iff.read_float(), iff.read_float());
			setKillAttachmentWhenParticleDies((iff.read_bool8() != 0));
			setSpawn(static_cast<Spawn>(iff.read_uint32()));
		}
		iff.exitChunk();
	}
	iff.exitForm(TAG_0001);
}

//-----------------------------------------------------------------------------
void ParticleAttachmentDescription::write(Iff &iff) const
{
	iff.insertForm(m_tag);
	{
		iff.insertForm(TAG_0001);
		{
			iff.insertChunk(TAG_0000);
			{
				iff.insertChunkString(getAttachmentPath().c_str());
				iff.insertChunkData(m_startPercentMin);
				iff.insertChunkData(m_startPercentMax);
				iff.insertChunkData(static_cast<uint8>(m_killAttachmentWhenParticleDies));
				iff.insertChunkData(static_cast<uint32>(m_spawn));
			}
			iff.exitChunk();
		}
		iff.exitForm(TAG_0001);
	}
	iff.exitForm(m_tag);
}

// ============================================================================
