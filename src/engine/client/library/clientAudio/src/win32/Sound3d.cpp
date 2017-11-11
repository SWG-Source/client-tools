
// ============================================================================
//
// Sound3d.cpp
// Copyright Sony Online Entertainment
//
// ============================================================================

#include "clientAudio/FirstClientAudio.h"
#include "clientAudio/Sound3d.h"

#include "clientAudio/Audio.h"
#include "clientAudio/Sound3dTemplate.h"
#include "sharedFoundation/ExitChain.h"
#include "sharedFoundation/MemoryBlockManager.h"

// ============================================================================
//
// Sound3dNamespace
//
// ============================================================================

namespace Sound3dNamespace
{
	int s_maxAllocatedSoundCount = 0;
}

using namespace Sound3dNamespace;

// ============================================================================
//
// Sound3d
//
// ============================================================================

MemoryBlockManager *Sound3d::m_memoryBlockManager = NULL;

//-----------------------------------------------------------------------------
Sound3d::Sound3d(Sound3dTemplate const *sound3dTemplate, SoundId const &soundId)
 : Sound2d(sound3dTemplate, soundId)
{
}

//-----------------------------------------------------------------------------
void Sound3d::install()
{
	DEBUG_FATAL(m_memoryBlockManager, ("Sound3d::install() - Already installed"));

	char const * const name = "Sound3d::install::m_memoryBlockManager";
	bool const shared = false;
	int const elementSize = sizeof(Sound3d);
	int const elementsPerBlock = 128;
	int const minimumNumberOfBlocks = 1;
	int const maximumNumberOfBlocks = 0;

	m_memoryBlockManager = new MemoryBlockManager(name, shared, elementSize, elementsPerBlock, minimumNumberOfBlocks, maximumNumberOfBlocks);

	ExitChain::add(&remove, "Sound3d::remove()");
}

//-----------------------------------------------------------------------------
void Sound3d::remove()
{
	DEBUG_FATAL(!m_memoryBlockManager, ("Sound3d::remove() - Sound3d is not installed"));

	delete m_memoryBlockManager;
	m_memoryBlockManager = NULL;
}

//-----------------------------------------------------------------------------
void *Sound3d::operator new(size_t size)
{
	UNREF(size);
	NOT_NULL(m_memoryBlockManager);
	DEBUG_FATAL(size != sizeof(Sound3d), ("bad size: %d sizeof: %d", size, sizeof(Sound3d)));
	DEBUG_FATAL(size != static_cast<size_t> (m_memoryBlockManager->getElementSize()), ("installed with bad size"));

	//DEBUG_REPORT_LOG(true, ("Sound3d: sound newed (%d)\n", m_memoryBlockManager->getElementCount() + 1));

	if (m_memoryBlockManager->getElementCount() + 1 > s_maxAllocatedSoundCount)
	{
		s_maxAllocatedSoundCount = m_memoryBlockManager->getElementCount() + 1;
	}

	return m_memoryBlockManager->allocate();
}

//-----------------------------------------------------------------------------
void Sound3d::operator delete(void *pointer)
{
	NOT_NULL(m_memoryBlockManager);

	m_memoryBlockManager->free(pointer);

	//DEBUG_REPORT_LOG(true, ("Sound3d: sound deleted (%d)\n", m_memoryBlockManager->getElementCount()));
}

//-----------------------------------------------------------------------------
int Sound3d::getCount()
{
	return m_memoryBlockManager->getElementCount();
}

//-----------------------------------------------------------------------------
int Sound3d::getMaxCount()
{
	return s_maxAllocatedSoundCount;
}

//-----------------------------------------------------------------------------
Sound3dTemplate const *Sound3d::getTemplate() const
{
	Sound3dTemplate const *sound3dTemplate = static_cast<Sound3dTemplate const *>(m_template);
	NOT_NULL(sound3dTemplate);

	return sound3dTemplate;
}

//-----------------------------------------------------------------------------
void Sound3d::alter(float const deltaTime)
{
	Audio::setSamplePosition_w(m_sampleId, getPosition_w());

	Sound2d::alter(deltaTime);
}

//-----------------------------------------------------------------------------
bool Sound3d::is2d() const
{
	return false;
}

//-----------------------------------------------------------------------------
bool Sound3d::is3d() const
{
	return true;
}

// ============================================================================
