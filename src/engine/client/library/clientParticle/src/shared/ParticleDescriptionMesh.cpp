// ============================================================================
//
// ParticleDescriptionMesh.cpp
// Copyright Sony Online Entertainment
//
// ============================================================================

#include "clientParticle/FirstClientParticle.h"
#include "clientParticle/ParticleDescriptionMesh.h"

#include "sharedFile/Iff.h"
#include "sharedFoundation/TemporaryCrcString.h"
#include "sharedObject/AppearanceTemplateList.h"

// ============================================================================
//
// ParticleDescriptionMesh
//
// ============================================================================

Tag const TAG_PTMH = TAG(P, T, M, H);

//-----------------------------------------------------------------------------
ParticleDescriptionMesh::ParticleDescriptionMesh()
 : m_scale()
 , m_rotationX()
 , m_rotationY()
 , m_rotationZ()
 , m_cachedMeshAppearanceTemplate(NULL)
 , m_meshPath()
{
	ParticleDescription::setDefaultParticleRelativeRotationX(m_particleRelativeRotationX);
	ParticleDescription::setDefaultParticleRelativeRotationY(m_particleRelativeRotationY);
	ParticleDescription::setDefaultParticleRelativeRotationZ(m_particleRelativeRotationZ);
}

//-----------------------------------------------------------------------------
ParticleDescriptionMesh::ParticleDescriptionMesh(ParticleDescriptionMesh const &particleDescriptionMesh)
 : ParticleDescription(particleDescriptionMesh)
 , m_scale(particleDescriptionMesh.m_scale)
 , m_rotationX(particleDescriptionMesh.m_rotationX)
 , m_rotationY(particleDescriptionMesh.m_rotationY)
 , m_rotationZ(particleDescriptionMesh.m_rotationZ)
 , m_cachedMeshAppearanceTemplate(NULL)
 , m_meshPath()
{
	ParticleDescription::setDefaultParticleRelativeRotationX(m_particleRelativeRotationX);
	ParticleDescription::setDefaultParticleRelativeRotationY(m_particleRelativeRotationY);
	ParticleDescription::setDefaultParticleRelativeRotationZ(m_particleRelativeRotationZ);

	setMeshPath(particleDescriptionMesh.getMeshPath().getString());
}

//--------------------------------------------------------------------------
ParticleDescriptionMesh::~ParticleDescriptionMesh()
{
	if (m_cachedMeshAppearanceTemplate != NULL)
	{
		AppearanceTemplateList::release(m_cachedMeshAppearanceTemplate);
		m_cachedMeshAppearanceTemplate = NULL;
	}
}

//--------------------------------------------------------------------------
ParticleDescription *ParticleDescriptionMesh::clone() const
{
	return new ParticleDescriptionMesh(*this);
}

//--------------------------------------------------------------------------
ParticleDescription::ParticleType ParticleDescriptionMesh::getParticleType() const
{
	return ParticleDescription::PT_mesh;
}

//--------------------------------------------------------------------------
void ParticleDescriptionMesh::initializeDefault()
{
	ParticleDescription::initializeDefault();

	setDefaultScale(m_scale);
	setDefaultRotationX(m_rotationX);
	setDefaultRotationY(m_rotationY);
	setDefaultRotationZ(m_rotationZ);
	setMeshPath("");
}

//--------------------------------------------------------------------------
void ParticleDescriptionMesh::setDefaultScale(WaveForm &waveForm)
{
#ifdef _DEBUG
	waveForm.setName("Particle - Scale");
#endif // _DEBUG
	waveForm.clear();
	waveForm.setValueMax(4096.0f);
	waveForm.setValueMin(-4096.0f);
	waveForm.insert(WaveFormControlPoint(0.0f, 1.0f));
	waveForm.insert(WaveFormControlPoint(1.0f, 1.0f));
}

//--------------------------------------------------------------------------
void ParticleDescriptionMesh::setDefaultRotationX(WaveForm &waveForm)
{
#ifdef _DEBUG
	waveForm.setName("Particle - RotationX");
#endif // _DEBUG
	waveForm.clear();
	waveForm.setValueMax(4096.0f);
	waveForm.setValueMin(-4096.0f);
	waveForm.insert(WaveFormControlPoint(0.0f, 0.0f));
	waveForm.insert(WaveFormControlPoint(1.0f, 0.0f));
}

//--------------------------------------------------------------------------
void ParticleDescriptionMesh::setDefaultRotationY(WaveForm &waveForm)
{
#ifdef _DEBUG
	waveForm.setName("Particle - RotationY");
#endif // _DEBUG
	waveForm.clear();
	waveForm.setValueMax(4096.0f);
	waveForm.setValueMin(-4096.0f);
	waveForm.insert(WaveFormControlPoint(0.0f, 0.0f));
	waveForm.insert(WaveFormControlPoint(1.0f, 0.0f));
}

//--------------------------------------------------------------------------
void ParticleDescriptionMesh::setDefaultRotationZ(WaveForm &waveForm)
{
#ifdef _DEBUG
	waveForm.setName("Particle - RotationZ");
#endif // _DEBUG
	waveForm.clear();
	waveForm.setValueMax(4096.0f);
	waveForm.setValueMin(-4096.0f);
	waveForm.insert(WaveFormControlPoint(0.0f, 0.0f));
	waveForm.insert(WaveFormControlPoint(1.0f, 0.0f));
}

//--------------------------------------------------------------------------
void ParticleDescriptionMesh::setMeshPath(char const *path)
{
	TemporaryCrcString temporaryCrcString(path, false);

	if (m_meshPath != temporaryCrcString)
	{
		if (m_cachedMeshAppearanceTemplate != NULL)
		{
			AppearanceTemplateList::release(m_cachedMeshAppearanceTemplate);
			m_cachedMeshAppearanceTemplate = NULL;
		}

		m_meshPath.set(path, false);

		if (!m_meshPath.isEmpty())
		{
			m_cachedMeshAppearanceTemplate = AppearanceTemplateList::fetch(path);
		}
	}
}

//--------------------------------------------------------------------------
PersistentCrcString const &ParticleDescriptionMesh::getMeshPath() const
{
	return m_meshPath;
}

//--------------------------------------------------------------------------
bool ParticleDescriptionMesh::load(Iff &iff)
{
	bool result = true;

	if (iff.enterForm(TAG_PTMH, true))
	{
		ParticleDescription::load(iff);

		switch (iff.getCurrentName())
		{
			case TAG_0000:
				{
					load_0000(iff);
					break;
				}
			default:
				{
					result = false;

					// Gracefully  handle the unknown file and just initialize to default values

					char currentTagName[256];
					ConvertTagToString(iff.getCurrentName(), currentTagName);
					DEBUG_WARNING(true, ("ParticleDescriptionMesh::load() - Unable to load particle description due to unsupported data version: %s", currentTagName));
				}
		}
		iff.exitForm(TAG_PTMH);
	}
	else
	{
		result = false;

		// Issue a warning

		char expectedTagName[256];
		ConvertTagToString(TAG_PTMH, expectedTagName);
		char currentTagName[256];
		ConvertTagToString(iff.getCurrentName(), currentTagName);
		DEBUG_WARNING(true, ("ParticleDescriptionMesh::load() - Unable to load particle description. Expecting tag(%s) but encountered(%s)", expectedTagName, currentTagName));
	}

	if (!result)
	{
		// Initialize to a default state

		initializeDefault();
	}

	return result;
}

//--------------------------------------------------------------------------
void ParticleDescriptionMesh::load_0000(Iff &iff)
{
	iff.enterChunk(TAG_0000);
	{
		std::string meshPath;
		iff.read_string(meshPath);
		setMeshPath(meshPath.c_str());
	}
	iff.exitChunk(TAG_0000);

	m_scale.load(iff);
	m_rotationX.load(iff);
	m_rotationY.load(iff);
	m_rotationZ.load(iff);
}

//--------------------------------------------------------------------------
void ParticleDescriptionMesh::write(Iff &iff) const
{
	iff.insertForm(TAG_PTMH);
	{
		// Write the base class

		ParticleDescription::write(iff);

		// Write the class specific data

		iff.insertChunk(TAG_0000);
		{
			iff.insertChunkString(m_meshPath.getString());
		}
		iff.exitChunk(TAG_0000);

		m_scale.write(iff);
		m_rotationX.write(iff);
		m_rotationY.write(iff);
		m_rotationZ.write(iff);
	}
	iff.exitForm(TAG_PTMH);
}

// ============================================================================
