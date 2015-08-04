// ============================================================================
//
// ParticleEmitterGroup.cpp
// Copyright Sony Online Entertainment
//
// ============================================================================

#include "clientParticle/FirstClientParticle.h"
#include "clientParticle/ParticleEmitterGroup.h"

#include "clientGraphics/Graphics.h"
#include "clientGraphics/ShaderTemplateList.h"
#include "clientParticle/ParticleEffectAppearance.h"
#include "clientParticle/ParticleEmitter.h"
#include "clientParticle/ParticleEmitterDescription.h"
#include "clientParticle/ParticleEmitterGroupDescription.h"
#include "sharedDebug/Profiler.h"
#include "sharedFoundation/ExitChain.h"
#include "sharedFoundation/MemoryBlockManager.h"
#include "sharedObject/Object.h"

#include <vector>

// ============================================================================
//
// ParticleEmitterGroupNamespace
//
// ============================================================================

namespace ParticleEmitterGroupNamespace
{
	typedef stdvector<ParticleEmitter *>::fwd ParticleEmitterList;
	typedef stdvector<ParticleEmitterList *>::fwd ParticleEmitterListList;

	ParticleEmitterListList ms_particleEmitterListList;
	ParticleEmitterList * newParticleEmitterList();
	void deleteParticleEmitterList(ParticleEmitterList * particleEmitterList);
}

using namespace ParticleEmitterGroupNamespace;

//-----------------------------------------------------------------------------
ParticleEmitterList * ParticleEmitterGroupNamespace::newParticleEmitterList()
{
	if (ms_particleEmitterListList.empty())
	{
		for (int i = 0; i < 256; ++i)
			ms_particleEmitterListList.push_back(new ParticleEmitterList);
	}

	ParticleEmitterList * const result = ms_particleEmitterListList.back();
	ms_particleEmitterListList.pop_back();

	return result;
}

//-----------------------------------------------------------------------------
void ParticleEmitterGroupNamespace::deleteParticleEmitterList(ParticleEmitterList * particleEmitterList)
{
	if (ms_particleEmitterListList.size() < 256)
	{
		DEBUG_FATAL(!particleEmitterList->empty(), ("particleEmitterList is not empty"));
		particleEmitterList->clear();
		ms_particleEmitterListList.push_back(particleEmitterList);
	}
	else
		delete particleEmitterList;
}

// ============================================================================
//
// ParticleEmitterGroup
//
// ============================================================================

//-----------------------------------------------------------------------------
MEMORY_BLOCK_MANAGER_IMPLEMENTATION_WITHOUT_INSTALL(ParticleEmitterGroup, true, 0, 0, 0);

//-----------------------------------------------------------------------------
void ParticleEmitterGroup::install()
{
	installMemoryBlockManager();

	ExitChain::add(remove, "ParticleEmitterGroup::remove");
}

//-----------------------------------------------------------------------------
void ParticleEmitterGroup::remove()
{
	while (!ms_particleEmitterListList.empty())
	{
		delete ms_particleEmitterListList.back();
		ms_particleEmitterListList.pop_back();
	}

	removeMemoryBlockManager();
}

//-----------------------------------------------------------------------------
int ParticleEmitterGroup::getGlobalCount()
{
	return ms_memoryBlockManager->getElementCount();
}

//-----------------------------------------------------------------------------
ParticleEmitterGroup::ParticleEmitterGroup(ParticleEmitterGroupDescription const &particleEmitterGroupDescription, ParticleEffectAppearance const * const particleEffectAppearance)
 : ParticleGenerator(particleEffectAppearance)
 , m_particleEmitterList(NULL)
 , m_particleEmitterGroupDescription(particleEmitterGroupDescription)
 , m_currentTime(0.0f)
 , m_startDelay(0.0f)
 , m_loopCount(m_particleEmitterGroupDescription.getParticleTiming().getRandomLoopCount())
 , m_currentLoop(0)
 , m_object(NULL)
{
	m_object = new MemoryBlockManagedObject();
	m_particleEmitterList = newParticleEmitterList();

	// Create the list of emitters from the particleEmitter description list

	ParticleEmitterGroupDescription::ParticleEmitterDescriptions::const_iterator iterParticleEmitterDescriptions = m_particleEmitterGroupDescription.getParticleEmitterDescriptions().begin();

	m_particleEmitterList->reserve(m_particleEmitterGroupDescription.getParticleEmitterDescriptions().size());

	for (; iterParticleEmitterDescriptions != m_particleEmitterGroupDescription.getParticleEmitterDescriptions().end(); ++iterParticleEmitterDescriptions)
	{
		ParticleEmitter *particleEmitter = new ParticleEmitter(*iterParticleEmitterDescriptions, particleEffectAppearance);
		particleEmitter->restart();
		particleEmitter->setOwner(m_object);

		m_particleEmitterList->push_back(particleEmitter);
	}
}

//-----------------------------------------------------------------------------
ParticleEmitterGroup::~ParticleEmitterGroup()
{
	// Delete all the ParticleEmitter objects

	ParticleEmitterList::iterator iterParticleEmitter = m_particleEmitterList->begin();

	for (; iterParticleEmitter != m_particleEmitterList->end(); ++iterParticleEmitter)
	{
		ParticleEmitter * const particleEmitter = NON_NULL(*iterParticleEmitter);
		delete particleEmitter;
	}
	m_particleEmitterList->clear();

	deleteParticleEmitterList(m_particleEmitterList);
	m_particleEmitterList = NULL;

	delete m_object.getPointer();
	m_object = NULL;
}

//-----------------------------------------------------------------------------
void ParticleEmitterGroup::addToCameraScene(Camera const *camera, Object const *object) const
{
	NP_PROFILER_AUTO_BLOCK_DEFINE("ParticleEmitterGroup::addToCameraScene");

	ParticleEmitterList::const_iterator iterParticleEmitterList = m_particleEmitterList->begin();

	for (; iterParticleEmitterList != m_particleEmitterList->end(); ++iterParticleEmitterList)
	{
		ParticleEmitter * const particleEmitter = NON_NULL(*iterParticleEmitterList);

		particleEmitter->addToCameraScene(camera, object);
	}
}

//-----------------------------------------------------------------------------
void ParticleEmitterGroup::alter(float const deltaTime)
{
	m_currentTime += deltaTime;

	BoxExtent newBoxExtent;
	bool addedSomeParticles = false;

	if (m_currentTime > m_startDelay)
	{
		bool readyToLoop = true;

		ParticleEmitterList::iterator iterParticleEmitterList = m_particleEmitterList->begin();

		for (; iterParticleEmitterList != m_particleEmitterList->end(); ++iterParticleEmitterList)
		{
			ParticleEmitter *particleEmitter = (*iterParticleEmitterList);
			NOT_NULL(particleEmitter);

			particleEmitter->alter(deltaTime);
			
			if (particleEmitter->hasAliveParticles())
			{
				newBoxExtent.grow(particleEmitter->getExtent());
				addedSomeParticles = true;
			}

			if (!particleEmitter->isDeletable())
			{
				readyToLoop = false;
			}
		}

		// See if we need to restart all the emitters

		if (readyToLoop)
		{
			++m_currentLoop;

			if ((isInfiniteLooping()) ||
			    (m_currentLoop < m_loopCount))
			{
				loop();
			}
		}
	}

	if (!addedSomeParticles)
	{
		newBoxExtent.setMin(m_object->getPosition_p());
		newBoxExtent.setMax(m_object->getPosition_p());
	}

	m_extent_w.setMin(newBoxExtent.getMin());
	m_extent_w.setMax(newBoxExtent.getMax());

	DEBUG_FATAL(m_extent_w.getBox().isEmpty(), ("Invalid extent"));
	//DEBUG_REPORT_LOG(true, ("ParticleEmitterGroup: m_extent_w.getRadius(%f) min(%f, %f, %f) max(%f, %f, %f)\n", m_extent_w.getRadius(), m_extent_w.getMin().x, m_extent_w.getMin().y, m_extent_w.getMin().z, m_extent_w.getMax().x, m_extent_w.getMax().y, m_extent_w.getMax().z));
}

//-----------------------------------------------------------------------------
void ParticleEmitterGroup::restart()
{
	m_currentLoop = 0;
	m_loopCount = m_particleEmitterGroupDescription.getParticleTiming().getRandomLoopCount();

	loop();
}

//-----------------------------------------------------------------------------
void ParticleEmitterGroup::loop()
{
	m_currentTime = 0.0f;

	if (m_currentLoop <= 0)
	{
		m_startDelay = m_particleEmitterGroupDescription.getParticleTiming().getRandomStartDelay();
	}
	else
	{
		m_startDelay = m_particleEmitterGroupDescription.getParticleTiming().getRandomLoopDelay();
	}

	// Restart all the emitters

	ParticleEmitterList::iterator iterParticleEmitterList = m_particleEmitterList->begin();

	for (; iterParticleEmitterList != m_particleEmitterList->end(); ++iterParticleEmitterList)
	{
		ParticleEmitter *particleEmitter = (*iterParticleEmitterList);
		NOT_NULL(particleEmitter);

		particleEmitter->restart();
	}
}

//-----------------------------------------------------------------------------
bool ParticleEmitterGroup::isDeletable() const
{
	bool result = !(isInfiniteLooping() && getParentParticleEffectAppearance().isEnabled());

	if (result)
	{
		ParticleEmitterList::const_iterator iterParticleEmitterList = m_particleEmitterList->begin();

		for (; iterParticleEmitterList != m_particleEmitterList->end(); ++iterParticleEmitterList)
		{
			ParticleEmitter * const particleEmitter = NON_NULL(*iterParticleEmitterList);

			bool const notDeletable = !particleEmitter->isDeletable();

			if (notDeletable)
			{
				result = false;
				break;
			}
		}
	}

	return result;
}

//-----------------------------------------------------------------------------
int ParticleEmitterGroup::getParticleCount() const
{
	int result = 0;

	ParticleEmitterList::const_iterator iterParticleEmitterList = m_particleEmitterList->begin();

	for (; iterParticleEmitterList != m_particleEmitterList->end(); ++iterParticleEmitterList)
	{
		ParticleEmitter * const particleEmitter = NON_NULL(*iterParticleEmitterList);

		result += particleEmitter->getParticleCountIncludingAttachments();
	}

	return result;
}

//-----------------------------------------------------------------------------
ParticleEmitter const * const ParticleEmitterGroup::getEmitter(int const index) const
{
	// Make sure they are requesting a valid particle emitter

	DEBUG_FATAL((index < 0) || (index >= static_cast<int>(m_particleEmitterList->size())), ("Invalid emitter index (%d)", index));

	ParticleEmitter const * const particleEmitter = (*m_particleEmitterList)[static_cast<unsigned int>(index)];

	return particleEmitter;
}

//-----------------------------------------------------------------------------
int ParticleEmitterGroup::getEmitterCount() const
{
	return static_cast<int>(m_particleEmitterList->size());
}

//-----------------------------------------------------------------------------
BoxExtent const &ParticleEmitterGroup::getExtent() const
{
	return m_extent_w;
}

//-----------------------------------------------------------------------------
void ParticleEmitterGroup::setOwner(Object *newOwner)
{
	if (newOwner != NULL)
	{
		if (m_object->getParent() != NULL)
		{
			m_object->getParent()->removeChildObject(m_object, Object::DF_none);
		}

		newOwner->addChildObject_o(m_object);
	}
}

//--------------------------------------------------------------------------
void ParticleEmitterGroup::drawDebugExtents() const
{
	if (m_object != NULL)
	{
		Graphics::setStaticShader(ShaderTemplateList::get3dVertexColorZStaticShader());
		Graphics::setObjectToWorldTransformAndScale(Transform::identity, Vector::xyz111);
		Graphics::drawExtent(&m_extent_w, VectorArgb::solidYellow);
	}
}

//--------------------------------------------------------------------------
void ParticleEmitterGroup::addToWorld()
{
	ParticleEmitterList::iterator iterParticleEmitterList = m_particleEmitterList->begin();

	for (; iterParticleEmitterList != m_particleEmitterList->end(); ++iterParticleEmitterList)
	{
		ParticleEmitter * const particleEmitter = NON_NULL(*iterParticleEmitterList);

		particleEmitter->addToWorld();
	}
}

//--------------------------------------------------------------------------
void ParticleEmitterGroup::removeFromWorld()
{
	ParticleEmitterList::iterator iterParticleEmitterList = m_particleEmitterList->begin();

	for (; iterParticleEmitterList != m_particleEmitterList->end(); ++iterParticleEmitterList)
	{
		ParticleEmitter * const particleEmitter = NON_NULL(*iterParticleEmitterList);

		particleEmitter->removeFromWorld();
	}
}

//--------------------------------------------------------------------------
bool ParticleEmitterGroup::hasChildrenLoopedOnce() const
{
	bool result = true;

	ParticleEmitterList::iterator iterParticleEmitterList = m_particleEmitterList->begin();

	for (; iterParticleEmitterList != m_particleEmitterList->end(); ++iterParticleEmitterList)
	{
		ParticleEmitter * const particleEmitter = NON_NULL(*iterParticleEmitterList);

		result = (particleEmitter->getCurrentLoopCount() > 0);

		if (!result)
		{
			break;
		}
	}

	return result;
}

//--------------------------------------------------------------------------
bool ParticleEmitterGroup::isInfiniteLooping() const
{
	return (m_loopCount == -1);
}

//--------------------------------------------------------------------------
bool ParticleEmitterGroup::hasAliveParticles() const
{
	bool result = false;
	ParticleEmitterList::const_iterator iterParticleEmitterList = m_particleEmitterList->begin();

	for (; iterParticleEmitterList != m_particleEmitterList->end(); ++iterParticleEmitterList)
	{
		ParticleEmitter const & particleEmitter = *NON_NULL(*iterParticleEmitterList);

		if (particleEmitter.hasAliveParticles())
		{
			result = true;
			break;
		}
	}

	return result;
}
 
// ============================================================================
