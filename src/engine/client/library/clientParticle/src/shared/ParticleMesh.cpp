// ============================================================================
//
// ParticleMesh.cpp
// Copyright Sony Online Entertainment
//
// ============================================================================

#include "clientParticle/FirstClientParticle.h"
#include "clientParticle/ParticleMesh.h"

#include "clientParticle/ConfigClientParticle.h"
#include "sharedFoundation/ExitChain.h"
#include "sharedFoundation/MemoryBlockManager.h"
#include "sharedObject/Object.h"
#include "sharedObject/MemoryBlockManagedObject.h"

// ============================================================================
//
// ParticleMesh
//
// ============================================================================

MemoryBlockManager *ParticleMesh::m_memoryBlockManager;

//-----------------------------------------------------------------------------
ParticleMesh::ParticleMesh()
 : Particle()
 , m_iterScale()
 , m_iterRotationX()
 , m_iterRotationY()
 , m_iterRotationZ()
 , m_rotationInitial(Vector::zero)
 , m_object(NULL)
{
}

//-----------------------------------------------------------------------------
ParticleMesh::~ParticleMesh()
{
	delete m_object;
}

//-----------------------------------------------------------------------------
void ParticleMesh::install()
{
	DEBUG_FATAL(m_memoryBlockManager, ("ParticleMesh::install() - Already installed"));

	int const elementsPerBlock = 512;
	m_memoryBlockManager = new MemoryBlockManager("ParticleMesh::install::m_memoryBlockManager", false, sizeof(ParticleMesh), elementsPerBlock, 1, ConfigClientParticle::getMaxMeshParticles() / elementsPerBlock);

	ExitChain::add(&remove, "ParticleMesh::remove()");
}

//-----------------------------------------------------------------------------
void ParticleMesh::remove()
{
	DEBUG_FATAL(!m_memoryBlockManager, ("ParticleMesh::remove() - ParticleMesh is not installed"));

	delete m_memoryBlockManager;
	m_memoryBlockManager = NULL;
}

//-----------------------------------------------------------------------------
void *ParticleMesh::operator new(size_t size)
{
	UNREF(size);
	NOT_NULL(m_memoryBlockManager);
	DEBUG_FATAL(size != sizeof (ParticleMesh), ("bad size"));
	DEBUG_FATAL(size != static_cast<size_t> (m_memoryBlockManager->getElementSize()), ("installed with bad size"));

	return m_memoryBlockManager->allocate();
}

//-----------------------------------------------------------------------------
void ParticleMesh::operator delete(void *pointer)
{
	NOT_NULL(m_memoryBlockManager);

	m_memoryBlockManager->free(pointer);
}

//-----------------------------------------------------------------------------
bool ParticleMesh::isParticlePoolFull()
{
	return m_memoryBlockManager->isFull();
}

//-----------------------------------------------------------------------------
int ParticleMesh::getGlobalCount()
{
	return m_memoryBlockManager->getElementCount();
}

// ============================================================================
