// ======================================================================
//
// TextureSourceModeElement.h
// copyright 2001 Sony Online Entertainment
// 
// ======================================================================

#ifndef INCLUDED_TextureSourceModeElement_h
#define INCLUDED_TextureSourceModeElement_h

// ======================================================================

#include "Element.h"

class DrawTextureCommandElement;

// ======================================================================

class TextureSourceModeElement: public Element
{
public:

	explicit TextureSourceModeElement(DrawTextureCommandElement &drawTextureCommandElement);

	virtual std::string        getLabel() const;
	virtual unsigned int       getTypeIndex() const;

	DrawTextureCommandElement &getDrawTextureCommandElement();

private:

	DrawTextureCommandElement &m_drawTextureCommandElement;

private:
	// disabled
	TextureSourceModeElement();
	TextureSourceModeElement(const TextureSourceModeElement&);
	TextureSourceModeElement &operator =(const TextureSourceModeElement&);
};

// ======================================================================

inline DrawTextureCommandElement &TextureSourceModeElement::getDrawTextureCommandElement()
{
	return m_drawTextureCommandElement; //lint !e1536 // exposing low access member // yes
}

// ======================================================================

#endif
