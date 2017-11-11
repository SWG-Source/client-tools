// ======================================================================
//
// TextureWriteModeElement.h
// copyright 2001 Sony Online Entertainment
// 
// ======================================================================

#ifndef INCLUDED_TextureWriteModeElement_h
#define INCLUDED_TextureWriteModeElement_h

// ======================================================================

#include "Element.h"

class DrawTextureCommandElement;

// ======================================================================

class TextureWriteModeElement: public Element
{
public:

	explicit TextureWriteModeElement(DrawTextureCommandElement &drawTextureCommandElement);

	virtual std::string        getLabel() const;
	virtual unsigned int       getTypeIndex() const;

	DrawTextureCommandElement &getDrawTextureCommandElement();

private:

	DrawTextureCommandElement &m_drawTextureCommandElement;

private:
	// disabled
	TextureWriteModeElement();
	TextureWriteModeElement(const TextureWriteModeElement&);
	TextureWriteModeElement &operator =(const TextureWriteModeElement&);
};

// ======================================================================

inline DrawTextureCommandElement &TextureWriteModeElement::getDrawTextureCommandElement()
{
	return m_drawTextureCommandElement; //lint !e1536 // exposing low access member // yes
}

// ======================================================================

#endif
