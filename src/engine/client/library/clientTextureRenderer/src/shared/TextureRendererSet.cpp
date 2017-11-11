// ======================================================================
//
// TextureRendererSet.cpp
// copyright 2001 Sony Online Entertainment
// 
// ======================================================================

#include "clientTextureRenderer/FirstClientTextureRenderer.h"
#include "clientTextureRenderer/TextureRendererSet.h"

#include "clientTextureRenderer/TextureRenderer.h"
#include "clientTextureRenderer/TextureRendererTemplate.h"

#include <algorithm>
#include <utility>
#include <vector>

// ======================================================================

namespace
{
	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

	class TextureRendererIsFromTemplate
	{
	public:

		explicit TextureRendererIsFromTemplate(const TextureRendererTemplate *textureRendererTemplate)
			: m_textureRendererTemplate(textureRendererTemplate)
			{
			}

		bool operator ()(const TextureRendererSet::TextureRendererData &textureRendererData) const
			{
				return (&(textureRendererData.m_textureRenderer->getTextureRendererTemplate()) == m_textureRendererTemplate);
			}

	private:

		const TextureRendererTemplate *m_textureRendererTemplate;

	private:
		// disabled
		TextureRendererIsFromTemplate();
	};

	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

	class IsTextureRenderer
	{
	public:

		explicit IsTextureRenderer(TextureRenderer *textureRenderer)
			: m_textureRenderer(textureRenderer)
			{
			}

		bool operator ()(const TextureRendererSet::TextureRendererData &textureRendererData) const
			{
				return textureRendererData.m_textureRenderer == m_textureRenderer;
			}

	private:

		TextureRenderer *m_textureRenderer;

	private:
		// disabled
		IsTextureRenderer();
	};

	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

	inline void ReleaseTextureRendererData(TextureRendererSet::TextureRendererData &textureRendererData)
	{
		DEBUG_WARNING(textureRendererData.m_referenceCount, ("TextureRendererSet::TextureRendererData deleted with [%d] references", textureRendererData.m_referenceCount));

		// release our reference
		textureRendererData.m_textureRenderer->release();
		textureRendererData.m_textureRenderer = 0;
	}

	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

}

// ======================================================================
// struct TextureRendererSet::TextureRendererData
// ======================================================================

TextureRendererSet::TextureRendererData::TextureRendererData(const TextureRendererTemplate *textureRendererTemplate)
:	m_textureRenderer(0),
	m_referenceCount(0)
{
	//-- create the texture renderer
	m_textureRenderer = textureRendererTemplate->createTextureRenderer();
}

// ======================================================================
// class TextureRendererSet
// ======================================================================

TextureRendererSet::TextureRendererSet()
:	m_referenceCount(0),
	m_textureRenderers(new TextureRendererDataContainer())
{
}

// ----------------------------------------------------------------------

TextureRendererSet::~TextureRendererSet()
{
	IGNORE_RETURN( std::for_each(m_textureRenderers->begin(), m_textureRenderers->end(), ReleaseTextureRendererData) );
	delete m_textureRenderers;
}

// ----------------------------------------------------------------------

void TextureRendererSet::release()
{
	--m_referenceCount;
	if (m_referenceCount < 1)
	{
		//-- delete this
		DEBUG_FATAL(m_referenceCount != 0, ("expecting zero reference count on TextureRendererSet [%d]", m_referenceCount));
		delete this;
	}
}

// ----------------------------------------------------------------------

TextureRenderer *TextureRendererSet::fetchTextureRenderer(const TextureRendererTemplate *textureRendererTemplate, bool *isNew)
{
	TextureRendererDataContainer::iterator it = std::find_if(m_textureRenderers->begin(), m_textureRenderers->end(), TextureRendererIsFromTemplate(textureRendererTemplate));
	if (it != m_textureRenderers->end())
	{
		//-- we've already created the textureRenderer for this textureRenderer template, return it
		TextureRendererData &textureRendererData = *it;
		++textureRendererData.m_referenceCount;

		textureRendererData.m_textureRenderer->fetch();

		if (isNew)
			*isNew = false;
		return textureRendererData.m_textureRenderer;
	}

	//-- create the textureRenderer data
	TextureRendererData newTextureRendererData(textureRendererTemplate);
	++newTextureRendererData.m_referenceCount;

	//-- keep track of it (the copy is okay here)
	m_textureRenderers->push_back(newTextureRendererData);

	//-- fetch and return new textureRenderer
	newTextureRendererData.m_textureRenderer->fetch();

		if (isNew)
			*isNew = true;
	return newTextureRendererData.m_textureRenderer;
}

// ----------------------------------------------------------------------

void TextureRendererSet::releaseTextureRenderer(TextureRenderer *textureRenderer)
{
	TextureRendererDataContainer::iterator it = std::find_if(m_textureRenderers->begin(), m_textureRenderers->end(), IsTextureRenderer(textureRenderer));
	DEBUG_FATAL(it == m_textureRenderers->end(), ("textureRenderer [0x%08x] not managed by this TextureRendererSet", textureRenderer));
	if (it != m_textureRenderers->end())
	{
		NOT_NULL(textureRenderer);

		//-- release caller's reference (matches fetch in fetchTextureRenderer())
		textureRenderer->release();
		textureRenderer = 0;

		TextureRendererData &textureRendererData = *it;
		--textureRendererData.m_referenceCount;
		if (textureRendererData.m_referenceCount < 1)
		{
			//-- time for this textureRenderer data to retire
			ReleaseTextureRendererData(textureRendererData);
			IGNORE_RETURN(m_textureRenderers->erase(it));
		}
	}
}

// ======================================================================
