// ======================================================================
//
// TextureRendererTemplate.h
// copyright 2001 Sony Online Entertainment
// 
// ======================================================================

#ifndef TEXTURE_RENDERER_TEMPLATE
#define TEXTURE_RENDERER_TEMPLATE

// ======================================================================

class AvailableVariables;
class CrcLowerString;
class CustomizationData;
class CustomizationVariable;
class Texture;
class TextureRenderer;

#include "clientGraphics/Texture.def"

// ======================================================================

class TextureRendererTemplate
{
public:

	virtual ~TextureRendererTemplate();

	virtual TextureRenderer *createTextureRenderer() const = 0;

	/**
	 * Add all CustomizationData variables that influence this TextureRendererTemplate instance
	 * to the given CustomizationData instance.
	 *
	 * This is primarily useful as a mechanism for tools.  The game should already
	 * know which customization variables it has enabled for Object instances via the
	 * ObjectTemplate system.
	 *
	 * Derived classes do not need to chain down to this function.
	 *
	 * @param customizationData  the CustomizationData instance to which new
	 *                           variables will be added.
	 */
	virtual void             addCustomizationVariables(CustomizationData &customizationData) const = 0;

	virtual int                    getCustomizationVariableCount() const = 0;
	virtual const std::string     &getCustomizationVariableName(int index) const = 0;
	virtual bool                   isCustomizationVariablePrivate(int index) const = 0;

	virtual CustomizationVariable  *createCustomizationVariable(int index) const = 0;

	const CrcLowerString    &getCrcName() const;
	int                      getReferenceCount() const;

	int                      getDestinationPreferredWidth() const;
	int                      getDestinationPreferredHeight() const;
	int                      getDestinationRuntimeFormatCount() const;
	TextureFormat            getDestinationRuntimeFormat(int index) const;

	void                     fetch() const;
	void                     release() const;

	Texture                 *fetchCompatibleTexture() const;

protected:

	explicit TextureRendererTemplate(const char *name);

	void     setDestinationPreferredWidth(int width);
	void     setDestinationPreferredHeight(int height);
	void     setDestinationRuntimeFormats(const TextureFormat *runtimeFormats, int runtimeFormatCount);

private:

	typedef stdvector<TextureFormat>::fwd  TextureFormatContainer;

private:

	mutable int              m_referenceCount;
	CrcLowerString          *m_crcName;

	int                      m_destinationPreferredWidth;
	int                      m_destinationPreferredHeight;
	TextureFormatContainer  *m_runtimeFormats;

private:
	// disabled
	TextureRendererTemplate();
	TextureRendererTemplate(const TextureRendererTemplate&);
	TextureRendererTemplate &operator =(const TextureRendererTemplate&);
};

// ======================================================================

inline void TextureRendererTemplate::fetch() const
{
	++m_referenceCount;
}

// ----------------------------------------------------------------------

inline int TextureRendererTemplate::getReferenceCount() const
{
	return m_referenceCount;
}

// ----------------------------------------------------------------------

inline const CrcLowerString &TextureRendererTemplate::getCrcName() const
{
	NOT_NULL(m_crcName);
	return *m_crcName;
}

// ----------------------------------------------------------------------

inline int TextureRendererTemplate::getDestinationPreferredWidth() const
{
	return m_destinationPreferredWidth;
}

// ----------------------------------------------------------------------

inline int TextureRendererTemplate::getDestinationPreferredHeight() const
{
	return m_destinationPreferredHeight;
}

// ----------------------------------------------------------------------

inline void TextureRendererTemplate::setDestinationPreferredWidth(int width)
{
	m_destinationPreferredWidth = width;
}

// ----------------------------------------------------------------------

inline void TextureRendererTemplate::setDestinationPreferredHeight(int height)
{
	m_destinationPreferredHeight = height;
}

// ======================================================================

#endif
