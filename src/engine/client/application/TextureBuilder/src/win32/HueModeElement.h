// ======================================================================
//
// HueModeElement.h
// Copyright 2002 Sony Online Entertainment, Inc.
// All Rights Reserved.
//
// ======================================================================

#ifndef INCLUDED_HueModeElement_h
#define INCLUDED_HueModeElement_h

// ======================================================================

#include "Element.h"

class DrawTextureCommandElement;

// ======================================================================

class HueModeElement: public Element
{
public:

	explicit HueModeElement(DrawTextureCommandElement &drawTextureCommandElement);

	virtual std::string        getLabel() const;
	virtual unsigned int       getTypeIndex() const;

	DrawTextureCommandElement &getDrawTextureCommandElement();

private:

	DrawTextureCommandElement &m_drawTextureCommandElement;

private:

	// Disabled.
	HueModeElement();
	HueModeElement(const HueModeElement&);
	HueModeElement &operator =(const HueModeElement&);

};

// ======================================================================

inline DrawTextureCommandElement &HueModeElement::getDrawTextureCommandElement()
{
	return m_drawTextureCommandElement; //lint !e1536 // exposing low access member // yes
}

// ======================================================================

#endif
