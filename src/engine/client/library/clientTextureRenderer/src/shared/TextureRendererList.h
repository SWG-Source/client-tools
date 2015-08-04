// ======================================================================
//
// TextureRendererList.h
// copyright 2001 Sony Online Entertainment
// 
// ======================================================================

#ifndef TEXTURE_RENDERER_LIST_H
#define TEXTURE_RENDERER_LIST_H

// ======================================================================

#include "../../../../../../engine/shared/library/sharedFoundation/include/public/sharedFoundation/Tag.h"

class CrcLowerString;
class TextureRendererTemplate;
class Iff;

// ======================================================================

class TextureRendererList
{
friend class TextureRendererTemplate;

public:

	typedef TextureRendererTemplate *(*TemplateLoader)(Iff *iff, const char *name);

public:

	static void install();
	static void remove();

	static void registerTemplateLoader(Tag templateIdTag, TemplateLoader templateLoader);
	static void deregisterTemplateLoader(Tag templateIdTag);

	static const TextureRendererTemplate *fetch(const CrcLowerString &filename);
	static const TextureRendererTemplate *fetch(const char *filename);
	static const TextureRendererTemplate *fetch(Iff *iff);

private:

	static void removeFromList(const TextureRendererTemplate *textureRendererTemplate);

};

// ======================================================================

#endif
