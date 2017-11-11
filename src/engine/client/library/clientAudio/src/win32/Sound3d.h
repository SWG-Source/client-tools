// ============================================================================
//
// Sound3d.h
// Copyright Sony Online Entertainment
//
// ============================================================================

#ifndef INCLUDED_Sound3d_H
#define INCLUDED_Sound3d_H

#include "clientAudio/Sound2d.h"

class MemoryBlockManager;
class Sound3dTemplate;

//-----------------------------------------------------------------------------
class Sound3d : public Sound2d
{
	friend class Audio;

public:

	Sound3d(Sound3dTemplate const *sound3dTemplate, SoundId const &soundId);

	static void *operator new (size_t size);
	static void  operator delete (void *pointer);
	static void install();
	static void remove();
	static int  getCount();
	static int  getMaxCount();

	virtual void           alter(float const deltaTime);
	virtual bool           is2d() const;
	virtual bool           is3d() const;

	Sound3dTemplate const *getTemplate() const;

private:

	static MemoryBlockManager *m_memoryBlockManager;

private:

	// Disabled

	Sound3d();
	Sound3d(Sound3d const &);
	Sound3d &operator =(Sound3d const &);
};

// ============================================================================

#endif // INCLUDED_Sound3d_H
