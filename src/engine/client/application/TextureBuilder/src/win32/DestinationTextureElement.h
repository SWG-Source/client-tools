// ======================================================================
//
// DestinationTextureElement.h
// copyright 2001 Sony Online Entertainment
// 
// ======================================================================

#ifndef DESTINATION_TEXTURE_ELEMENT_H
#define DESTINATION_TEXTURE_ELEMENT_H

// ======================================================================

#include "Element.h"

class Iff;

// ======================================================================

class DestinationTextureElement: public Element
{
public:

	static bool           isPersistedNext(Iff &iff);

public:

	explicit DestinationTextureElement(Iff &iff);
	DestinationTextureElement(int width, int height);
	virtual ~DestinationTextureElement();

	virtual std::string   getLabel() const;
	virtual unsigned int  getTypeIndex() const;

	virtual bool          writeForWorkspace(Iff &iff) const;

	int                   getWidth() const;
	int                   getHeight() const;

	void                  setWidth(int width);
	void                  setHeight(int height);

private:

	void                  load_0000(Iff &iff);

private:

	int                   m_width;
	int                   m_height;

private:
	// disabled
	DestinationTextureElement();
	DestinationTextureElement(const DestinationTextureElement&);
	DestinationTextureElement &operator =(const DestinationTextureElement&);
};

// ======================================================================

inline int DestinationTextureElement::getWidth() const
{
	return m_width;
}

// ----------------------------------------------------------------------

inline int DestinationTextureElement::getHeight() const
{
	return m_height;
}

// ----------------------------------------------------------------------

inline void DestinationTextureElement::setWidth(int width)
{
	m_width = width;
}

// ----------------------------------------------------------------------

inline void DestinationTextureElement::setHeight(int height)
{
	m_height = height;
}

// ======================================================================

#endif
