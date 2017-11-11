// ======================================================================
//
// TextureElement.h
// copyright 2001 Sony Online Entertainment
// 
// ======================================================================

#ifndef TEXTURE_ELEMENT_H
#define TEXTURE_ELEMENT_H

// ======================================================================

#include "Element.h"

class Exporter;

// ======================================================================
/**
 * Virtual base class for all texture-related element types.
 * These include the single texture, the 1d array texture and
 * the 2d array texture.
 */

class TextureElement: public Element
{
public:

	static std::string    getReferencePathname(const std::string &projectRelativePathname);

public:

	TextureElement();
	virtual ~TextureElement();

	virtual bool          isDragSource() const;
	virtual std::string   getShortLabel() const = 0;

	virtual int           getArraySubscriptCount() const = 0;
	virtual int           getArraySubscriptUpperBound(int subscriptIndex) const = 0;

	virtual void          exportTextureRenderer(Exporter &exporter) const = 0;

private:
	// disabled
	TextureElement(const TextureElement&);
	TextureElement &operator =(const TextureElement&);
};

// ======================================================================

#endif
