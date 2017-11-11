// ======================================================================
//
// DrawTextureCommandElement.h
// copyright 2001 Sony Online Entertainment
// 
// ======================================================================

#ifndef DRAW_TEXTURE_COMMAND_ELEMENT_H
#define DRAW_TEXTURE_COMMAND_ELEMENT_H

// ======================================================================

#include "CommandElement.h"

class DynamicVertexBuffer;
class HueElement;
class Iff;
class ImageSlotElement;
class PackedArgb;
class RegionElement;
class Shader;
class StaticShader;
class Texture;
class VariableDefinitionElement;
class VertexBufferWriteIterator;

// ======================================================================

class DrawTextureCommandElement: public CommandElement
{
public:

	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

	enum HueMode
	{
		HM_none,
		HM_oneColor
	};

	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

	enum SourceTextureMode
	{
		STM_texture1,
		STM_textureBlend2
	};

	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

	enum WriteTextureMode
	{
		WTM_overwrite,
		WTM_alphaBlend
	};

	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

public:

	static void install();

	static bool isPersistedNext(const Iff &iff);

public:

	DrawTextureCommandElement();
	DrawTextureCommandElement(Iff &iff, ElementVector &sourceTextures);
	virtual ~DrawTextureCommandElement();

	virtual std::string     getLabel() const;
	virtual unsigned int    getTypeIndex() const;

	virtual bool						userCanDelete() const;

	virtual void            createVariableElements(VariableElementVector &variables) const;
	virtual void            applyCommand(Texture &bakeTexture, const ElementVector &sourceTextures, CustomizationData const &variables) const;

	virtual bool            hasLeftDoubleClickAction() const;
	virtual bool            doLeftDoubleClickAction(TextureBuilderDoc &document);

	virtual bool            writeForWorkspace(Iff &iff) const;

	virtual void            exportTextureRenderer(Exporter &exporter) const;

	const HueElement       *getHueElement(int index) const;
	HueElement             *getHueElement(int index);

	const ImageSlotElement *getImageSlotElement(int index) const;
	ImageSlotElement       *getImageSlotElement(int index);

	HueMode                 getHueMode() const;
	void                    setHueMode(HueMode hueMode);

	SourceTextureMode       getSourceTextureMode() const;
	void                    setSourceTextureMode(SourceTextureMode sourceTextureMode);

	WriteTextureMode        getWriteTextureMode() const;
	void                    setWriteTextureMode(WriteTextureMode writeTextureMode);

	void                    getFirstFilledImageSlotDimensions(int &imageWidth, int &imageHeight) const;

private:

	static void remove();

private:

	void                 load_0000(Iff &iff, ElementVector &sourceTextures);
	void                 load_0001(Iff &iff, ElementVector &sourceTextures);

	const Texture       &getImageSlotTexture(int index, CustomizationData const &customizationData) const;
	PackedArgb           getHueColor(int index, CustomizationData const &variables) const;

	uint32               getTextureFactorAlpha(int variableTypeId, CustomizationData const &customizationData) const;

	const RegionElement &getRegionElement() const;

	void                 fillVertexBuffer(VertexBufferWriteIterator vertexIterator) const;
	const StaticShader  &prepareShaderForBaking(CustomizationData const &variables) const;

	void                 createShader();

private:

	HueMode            m_hueMode;
	int                m_hueElementCount;

	SourceTextureMode  m_sourceTextureMode;
	WriteTextureMode   m_writeTextureMode;
	int                m_imageSlotCount;

	std::string       *m_commandDescription;

	bool               m_disableRender;
	Shader mutable    *m_shader;

private:
	// disabled
	DrawTextureCommandElement(const DrawTextureCommandElement&);
	DrawTextureCommandElement &operator =(const DrawTextureCommandElement&);
};

// ======================================================================

inline DrawTextureCommandElement::HueMode DrawTextureCommandElement::getHueMode() const
{
	return m_hueMode;
}

// ----------------------------------------------------------------------

inline DrawTextureCommandElement::SourceTextureMode DrawTextureCommandElement::getSourceTextureMode() const
{
	return m_sourceTextureMode;
}

// ----------------------------------------------------------------------

inline DrawTextureCommandElement::WriteTextureMode DrawTextureCommandElement::getWriteTextureMode() const
{
	return m_writeTextureMode;
}

// ----------------------------------------------------------------------

inline void DrawTextureCommandElement::setWriteTextureMode(DrawTextureCommandElement::WriteTextureMode writeTextureMode)
{
	m_writeTextureMode = writeTextureMode;
}

// ======================================================================

#endif
