// ======================================================================
//
// ImageSlotElement.h
// copyright 2001 Sony Online Entertainment
// 
// ======================================================================

#ifndef IMAGE_SLOT_ELEMENT_H
#define IMAGE_SLOT_ELEMENT_H

// ======================================================================

#include "../../../../../../engine/shared/library/sharedFoundation/include/public/sharedFoundation/Tag.h"
#include "VariableContainerElement.h"

class Exporter;
class Iff;
class Texture;
class TextureElement;

// ======================================================================

class ImageSlotElement: public VariableContainerElement
{
public:

	static void install();

	static bool isPersistedNext(Iff &iff);

public:

	ImageSlotElement();
	ImageSlotElement(Iff &iff, ElementVector &sourceTextures);
	virtual ~ImageSlotElement();

	virtual std::string   getLabel() const;
	virtual unsigned int  getTypeIndex() const;

	virtual bool          isDragTarget(const Element &source) const;
	virtual void          dropElementHere(Element &source);

	virtual bool          writeForWorkspace(Iff &iff) const;

	virtual void          createVariableElements(VariableElementVector &variables) const;

	bool                  isSlotFilled() const;
	const Texture        &getTexture(CustomizationData const &customizationData) const;

	bool                  getFlipU() const;
	bool                  getFlipV() const;

	void                  exportPrepareOperations(Exporter &exporter, int shaderIndex, Tag textureTag) const;

private:

	static void remove();

private:

	void                  load_0000(Iff &iff, ElementVector &sourceTextures);

private:

	TextureElement *m_attachedTexture;
	bool            m_flipU;
	bool            m_flipV;

private:
	// disabled
	ImageSlotElement(const ImageSlotElement&);
	ImageSlotElement &operator =(const ImageSlotElement&);
};

// ======================================================================

inline bool ImageSlotElement::isSlotFilled() const
{
	return m_attachedTexture != 0;
}

// ----------------------------------------------------------------------

inline bool ImageSlotElement::getFlipU() const
{
	return m_flipU;
}

// ----------------------------------------------------------------------

inline bool ImageSlotElement::getFlipV() const
{
	return m_flipV;
}

// ======================================================================

#endif
