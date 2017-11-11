// ============================================================================
//
// ParticleQuad.cpp
// Copyright Sony Online Entertainment
//
// ============================================================================

#include "clientParticle/FirstClientParticle.h"
#include "clientParticle/ParticleQuad.h"

#include "clientParticle/ParticleEffectAppearance.h"
#include "clientParticle/ConfigClientParticle.h"
#include "sharedFoundation/MemoryBlockManager.h"
#include "sharedFoundation/ExitChain.h"

// ============================================================================
//
// ParticleQuad
//
// ============================================================================

MemoryBlockManager *ParticleQuad::m_memoryBlockManager = NULL;

namespace ParticleQuadNamespace
{
	float const s_reservePriorityFactor = 2.0f;
	int s_normalParticleMax = 0;
	int s_priorityParticleMax = 0;
}

//-----------------------------------------------------------------------------
ParticleQuad::ParticleQuad()
 : Particle()
 , m_upVector(Vector::unitY)
 , m_sideVector(Vector::unitX)
 , m_initialRotation(0.0f)
 , m_iterLength()
 , m_iterRotation()
 , m_iterWidth()
{
}

//-----------------------------------------------------------------------------
ParticleQuad::~ParticleQuad()
{
}

//-----------------------------------------------------------------------------
void ParticleQuad::install()
{
	DEBUG_FATAL(m_memoryBlockManager, ("ParticleQuad::install() - Already installed"));

	int const elementsPerBlock = 512;
	ParticleQuadNamespace::s_normalParticleMax = ConfigClientParticle::getMaxQuadParticles();
	ParticleQuadNamespace::s_priorityParticleMax = (int)(ParticleQuadNamespace::s_normalParticleMax * ParticleQuadNamespace::s_reservePriorityFactor);
	m_memoryBlockManager = new MemoryBlockManager("ParticleQuad::install::m_memoryBlockManager", false, sizeof(ParticleQuad), elementsPerBlock, 1,  ParticleQuadNamespace::s_priorityParticleMax / elementsPerBlock);

	ExitChain::add(&remove, "ParticleQuad::remove()");
}

//-----------------------------------------------------------------------------
void ParticleQuad::remove()
{
	DEBUG_FATAL(!m_memoryBlockManager, ("ParticleQuad::remove() - ParticleQuad is not installed"));

	delete m_memoryBlockManager;
	m_memoryBlockManager = NULL;
}

//-----------------------------------------------------------------------------
void *ParticleQuad::operator new(size_t size)
{
	UNREF(size);
	NOT_NULL(m_memoryBlockManager);
	DEBUG_FATAL(size != sizeof (ParticleQuad), ("bad size"));
	DEBUG_FATAL(size != static_cast<size_t> (m_memoryBlockManager->getElementSize()), ("installed with bad size"));

	return m_memoryBlockManager->allocate();
}

//-----------------------------------------------------------------------------
void ParticleQuad::operator delete(void *pointer)
{
	NOT_NULL(m_memoryBlockManager);

	m_memoryBlockManager->free(pointer);
}

//-----------------------------------------------------------------------------
bool ParticleQuad::isParticlePoolFull(bool priority)
{
	if(m_memoryBlockManager->isFull())
		return true;
	if(!priority)
		return (getGlobalCount() >= ParticleEffectAppearance::getGlobalUserLimit() || getGlobalCount() >= ParticleQuadNamespace::s_normalParticleMax);
	else
		return (getGlobalCount() >= ParticleQuadNamespace::s_priorityParticleMax);	
}

//-----------------------------------------------------------------------------
int ParticleQuad::getGlobalCount()
{
	return m_memoryBlockManager->getElementCount();
}

// ============================================================================
