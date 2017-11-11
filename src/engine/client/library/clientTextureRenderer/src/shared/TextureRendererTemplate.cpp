// ======================================================================
//
// TextureRendererTemplate.cpp
// copyright 2001 Sony Online Entertainment
// 
// ======================================================================

#include "clientTextureRenderer/FirstClientTextureRenderer.h"
#include "clientTextureRenderer/TextureRendererTemplate.h"

#include "clientGraphics/TextureList.h"
#include "clientTextureRenderer/TextureRendererList.h"
#include "sharedFoundation/CrcLowerString.h"

#include <vector>

// ======================================================================

TextureRendererTemplate::TextureRendererTemplate(const char *name) :
	m_referenceCount(0),
	m_crcName(new CrcLowerString(name)),
	m_destinationPreferredWidth(0),
	m_destinationPreferredHeight(0),
	m_runtimeFormats(new TextureFormatContainer())
{
}

// ----------------------------------------------------------------------

TextureRendererTemplate::~TextureRendererTemplate()
{
	delete m_runtimeFormats;
	delete m_crcName;
}

// ----------------------------------------------------------------------

void TextureRendererTemplate::release() const
{
	--m_referenceCount;
	DEBUG_WARNING(m_referenceCount < 0, ("bad reference handling %d\n", m_referenceCount));
	if (m_referenceCount == 0)
	{
		TextureRendererList::removeFromList(this);
		delete const_cast<TextureRendererTemplate*>(this);
	}
}

// ----------------------------------------------------------------------

int TextureRendererTemplate::getDestinationRuntimeFormatCount() const
{
	return static_cast<int>(m_runtimeFormats->size());
}

// ----------------------------------------------------------------------

TextureFormat TextureRendererTemplate::getDestinationRuntimeFormat(int index) const
{
	VALIDATE_RANGE_INCLUSIVE_EXCLUSIVE(0, index, static_cast<int>(m_runtimeFormats->size()));
	return (*m_runtimeFormats)[static_cast<size_t>(index)];
}

// ----------------------------------------------------------------------

void TextureRendererTemplate::setDestinationRuntimeFormats(const TextureFormat *runtimeFormats, int runtimeFormatCount)
{
#if 0
	m_runtimeFormats->insert(m_runtimeFormats->end(), runtimeFormats, runtimeFormats + runtimeFormatCount);
#else
	UNREF(runtimeFormats);
	UNREF(runtimeFormatCount);

	const TextureFormat singleTextureFormat = TF_ARGB_8888;
	m_runtimeFormats->insert(m_runtimeFormats->end(), &singleTextureFormat, &singleTextureFormat + 1);
#endif
}

// ----------------------------------------------------------------------

Texture *TextureRendererTemplate::fetchCompatibleTexture() const
{
	//-- create the texture of the requested size and format
	const int textureWidth  = getDestinationPreferredWidth();
	const int textureHeight = getDestinationPreferredHeight();

#if 1
	const int mipmapCount = 1;
#else
	const int mipmapCount        = 1 + GetFirstBitSet(static_cast<uint>(std::min(textureWidth, textureHeight)));
#endif
	const int runtimeFormatCount = getDestinationRuntimeFormatCount();
	
	return TextureList::fetch(0, textureWidth, textureHeight, mipmapCount, &(*m_runtimeFormats)[0], runtimeFormatCount);
}

// ======================================================================
