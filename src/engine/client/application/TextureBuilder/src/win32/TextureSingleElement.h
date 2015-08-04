// ======================================================================
//
// TextureSingleElement.h
// copyright 2001 Sony Online Entertainment
// 
// ======================================================================

#ifndef TEXTURE_SINGLE_ELEMENT_H
#define TEXTURE_SINGLE_ELEMENT_H

// ======================================================================

#include "TextureElement.h"

class Iff;
class Texture;

// ======================================================================
/**
 * Virtual base class for all texture-related element types.
 * These include the single texture, the 1d array texture and
 * the 2d array texture.
 */

class TextureSingleElement: public TextureElement
{
public:

	static bool           isPersistedNext(Iff &iff);

public:

	TextureSingleElement();
	explicit TextureSingleElement(Iff &iff);
	explicit TextureSingleElement(const std::string &pathname);

	virtual ~TextureSingleElement();

	virtual std::string   getLabel() const;
	virtual unsigned int  getTypeIndex() const;

	virtual std::string   getShortLabel() const;
	virtual bool          userCanDelete() const;

	virtual int           getArraySubscriptCount() const;
	virtual int           getArraySubscriptUpperBound(int subscriptIndex) const;

	virtual bool          writeForWorkspace(Iff &iff) const;

	virtual void          exportTextureRenderer(Exporter &exporter) const;

	const std::string    &getPathname() const;

	const Texture        &getTexture() const;

private:

	void                  load_0000(Iff &iff);

private:

	std::string           *m_pathname;
	mutable const Texture *m_texture;

private:
	// disabled
	TextureSingleElement(const TextureSingleElement&);
	TextureSingleElement &operator =(const TextureSingleElement&);
};

// ======================================================================

#endif
