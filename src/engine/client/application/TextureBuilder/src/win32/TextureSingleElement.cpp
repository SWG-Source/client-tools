// ======================================================================
//
// <filename>
// copyright 2001 Sony Online Entertainment
// 
// ======================================================================

#include "FirstTextureBuilder.h"
#include "TextureSingleElement.h"

#include "BlueprintWriter.h"
#include "ElementTypeIndex.h"
#include "Exporter.h"
#include "sharedFile/Iff.h"
#include "clientGraphics/Texture.h"
#include "clientGraphics/TextureList.h"

#include <string>

// ======================================================================

namespace
{
	const Tag TAG_ETXS = TAG(E,T,X,S);
}

// ======================================================================

bool TextureSingleElement::isPersistedNext(Iff &iff)
{
	return (iff.isCurrentForm() && (iff.getCurrentName() == TAG_ETXS));
}

// ======================================================================

TextureSingleElement::TextureSingleElement()
:	TextureElement(),
	m_pathname(new std::string()),
	m_texture(0)
{
}

// ----------------------------------------------------------------------

TextureSingleElement::TextureSingleElement(Iff &iff)
:	TextureElement(),
	m_pathname(new std::string()),
	m_texture(0)
{
	iff.enterForm(TAG_ETXS);

		switch (iff.getCurrentName())
		{
			case TAG_0000:
				load_0000(iff);
				break;
			default:
				{
					char version[5];
					ConvertTagToString(iff.getCurrentName(), version);
					FATAL(true, ("unsupported version [%s]", version));
				}
		}

	iff.exitForm(TAG_ETXS);
}

// ----------------------------------------------------------------------

TextureSingleElement::TextureSingleElement(const std::string &pathname)
:	TextureElement(),
	m_pathname(new std::string(pathname)),
	m_texture(0)
{
}

// ----------------------------------------------------------------------

TextureSingleElement::~TextureSingleElement()
{
	if (m_texture)
	{
		m_texture->release();
		m_texture = 0;
	}

	delete m_pathname;
}

// ----------------------------------------------------------------------

void TextureSingleElement::load_0000(Iff &iff)
{
	iff.enterForm(TAG_0000);
		iff.enterChunk(TAG_INFO);
		{
			char buffer[1024];
			iff.read_string(buffer, sizeof(buffer)-1);

			IGNORE_RETURN(m_pathname->assign(buffer));
		}
		iff.exitChunk(TAG_INFO);
	iff.exitForm(TAG_0000);
}

// ----------------------------------------------------------------------

bool TextureSingleElement::writeForWorkspace(Iff &iff) const
{
	iff.insertForm(TAG_ETXS);	
		iff.insertForm(TAG_0000);
			iff.insertChunk(TAG_INFO);
			{
				iff.insertChunkString(m_pathname->c_str());
			}
			iff.exitChunk(TAG_INFO);
		iff.exitForm(TAG_0000);
	iff.exitForm(TAG_ETXS);	

	return true;
}

// ----------------------------------------------------------------------

std::string TextureSingleElement::getLabel() const
{
	if (!m_pathname->empty())
		return *m_pathname;
	else
		return "<unassigned texture pathname>";
}

// ----------------------------------------------------------------------

unsigned int TextureSingleElement::getTypeIndex() const
{
	return ETI_textureSingle;
}

// ----------------------------------------------------------------------

std::string TextureSingleElement::getShortLabel() const
{
	return *m_pathname;
}

// ----------------------------------------------------------------------

bool TextureSingleElement::userCanDelete() const
{
	return true;
}

// ----------------------------------------------------------------------

int TextureSingleElement::getArraySubscriptCount() const
{
	return 0;
}

// ----------------------------------------------------------------------

int TextureSingleElement::getArraySubscriptUpperBound(int) const
{
	FATAL(true, ("TextureSingleElement has no subscripts")); 
	return 0; //lint !e527 // warning -- unreachable // for MSVC
}

// ----------------------------------------------------------------------

const Texture &TextureSingleElement::getTexture() const
{
	if (!m_texture)
	{
		//-- load the texture

		// get relative pathname
		const std::string textureRelativePathname = getReferencePathname(*m_pathname);
		m_texture = TextureList::fetch(textureRelativePathname.c_str());
	}

	NOT_NULL(m_texture);
	return *m_texture;
}

// ----------------------------------------------------------------------

void TextureSingleElement::exportTextureRenderer(Exporter &exporter) const
{
	BlueprintWriter &writer = exporter.getWriter();

	const int textureIndex = writer.addTexture(getReferencePathname(*m_pathname));
	exporter.setTextureIndex(*this, textureIndex);
}

// ======================================================================
