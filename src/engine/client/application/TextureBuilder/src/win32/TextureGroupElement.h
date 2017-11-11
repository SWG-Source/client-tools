// ======================================================================
//
// TextureGroupElement.h
// copyright 2001 Sony Online Entertainment
// 
// ======================================================================

#ifndef TEXTURE_GROUP_ELEMENT_H
#define TEXTURE_GROUP_ELEMENT_H

// ======================================================================

#include "Element.h"

// ======================================================================

class TextureGroupElement: public Element
{
public:

	TextureGroupElement();
	virtual ~TextureGroupElement();

	virtual std::string   getLabel() const;
	virtual unsigned int  getTypeIndex() const;

private:
	// disabled
	TextureGroupElement(const TextureGroupElement&);
	TextureGroupElement &operator =(const TextureGroupElement&);
};

// ======================================================================

#endif
