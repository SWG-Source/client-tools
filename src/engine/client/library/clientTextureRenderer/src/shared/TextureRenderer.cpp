// ======================================================================
//
// TextureRenderer.cpp
// copyright 2001 Sony Online Entertainment
// 
// ======================================================================

#include "clientTextureRenderer/FirstClientTextureRenderer.h"
#include "clientTextureRenderer/TextureRenderer.h"

#include "clientTextureRenderer/TextureRendererList.h"
#include "clientTextureRenderer/TextureRendererManager.h"
#include "clientTextureRenderer/TextureRendererTemplate.h"

// ======================================================================
/**
 * Release a reference count to the TextureRenderer.
 *
 * When the reference count reaches zero, the TextureRenderer may be
 * deleted at any time.  All references to theTextureRenderer become
 * invalid when the reference count reaches zero.
 */

void TextureRenderer::release() const
{
	--m_referenceCount;
	DEBUG_WARNING(m_referenceCount < 0, ("bad reference handling %d\n", m_referenceCount));

	if (m_referenceCount == 0)
		delete const_cast<TextureRenderer*>(this);
}

// ======================================================================
/**
 * Construct a new TextureRenderer instance.
 *
 * @param textureRendererTemplate  the template instance from which this
 *                                 TextureRenderer is instantiated.
 * @param texture                  the texture to which the TextureRenderer
 *                                 should render its texture data.
 */

TextureRenderer::TextureRenderer(const TextureRendererTemplate &textureRendererTemplate) :
	m_textureRendererTemplate(textureRendererTemplate),
	m_referenceCount(0)
{
	m_textureRendererTemplate.fetch();
}

// ----------------------------------------------------------------------
/**
 * Release any resources associated with the TextureRenderer instance.
 */

TextureRenderer::~TextureRenderer()
{
	m_textureRendererTemplate.release();
}

// ----------------------------------------------------------------------
/**
 * Indicate that the TextureRenderer has been modified and needs to be
 * rendered.
 *
 * This function will submit the TextureRenderer instance to the
 * TextureRendererManager for rendering at its earliest convenience.
 */

void TextureRenderer::signalModified()
{
	TextureRendererManager::submitTextureRenderer(this);
}


// ======================================================================
