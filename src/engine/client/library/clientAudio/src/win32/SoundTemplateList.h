// ============================================================================
//
// SoundTemplateList.h
// Copyright Sony Online Entertainment
//
// ============================================================================

#ifndef SOUNDTEMPLATELIST_H
#define SOUNDTEMPLATELIST_H

// ======================================================================

class Sound2;
class SoundTemplate;
class Iff;

#include "../../../../../../engine/shared/library/sharedFoundation/include/public/sharedFoundation/Tag.h"

// ======================================================================

// Maintains a list of SoundTemplates.

class SoundTemplateList
{
public:

	typedef SoundTemplate *(*CreateFunction)(const char *name, Iff &iff);
	typedef SoundTemplate *(*CreateFromFileFunction)(const char *filename, void *context);

public:

	static void                 install();
	static void                 remove(void);

	static void                 debugReport();

	static void                 assignBinding(Tag tag, CreateFunction createFunction);
	static void                 removeBinding(Tag tag);

	static void                 preload(const char *fileName);
	static void                 releasePreloadedAssets();

	static const SoundTemplate *fetch(const char *fileName, CreateFromFileFunction createFromFileFunction = 0, void *context = 0);
	static const SoundTemplate *fetch(Iff *iff);
	static const SoundTemplate *fetch(const SoundTemplate *soundTemplate);
	static const SoundTemplate *fetchNew(SoundTemplate *soundTemplate);
	static void                 release(const SoundTemplate *soundTemplate);

	static Sound2 *             createSound(const char *fileName);

private:

	static void                 addAnonymousSoundTemplate(SoundTemplate *soundTemplate);
	static void                 addNamedSoundTemplate(SoundTemplate *soundTemplate);
	static void                 removeSoundTemplate(SoundTemplate *soundTemplate);

private:
	// disabled
	SoundTemplateList(void);
	SoundTemplateList(const SoundTemplateList &);
	SoundTemplateList &operator =(const SoundTemplateList &);
};


// ======================================================================

#endif // SOUNDTEMPLATELIST_H
