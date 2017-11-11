// ======================================================================
//
// TextureRenderer.h
// copyright 2001 Sony Online Entertainment
// 
// ======================================================================

#ifndef TEXTURE_RENDERER_H
#define TEXTURE_RENDERER_H

// ======================================================================

class CustomizationData;
class Texture;
class TextureRendererTemplate;

// ======================================================================

class TextureRenderer
{
friend class TextureRendererList;

public:

	/**
	 * Render the TextureRenderer data to the associated texture.
	 *
	 * This function call causes the TextureRenderer to draw any data necessary into
	 * the associated Texture.
	 */
	virtual bool render() = 0;

	/**
	 * Set the CustomizationData instance to be associated with this
	 * TextureRenderer instance.
	 *
	 * @param customizationData  the new CustomizationData instance to be
	 *                           associated with this TextureRenderer instance.
	 *                           This value may be NULL.
	 */
	virtual void setCustomizationData(CustomizationData *customizationData) = 0;

	/**
	 * Add all CustomizationData variables influencing this TextureRenderer instance
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
	virtual void addCustomizationVariables(CustomizationData &customizationData) const = 0;

	/**
	 * Retrieve the const Texture instance associated with this TextureRenderer and
	 * increment its reference count.
	 *
	 * @return  Texture instance containing the texture data output by this
	 *          TextureRenderer.
	 */
	virtual const Texture         *fetchTexture() const = 0;

	/**
	 * Find out if the Texture instance associated with this TextureRenderer is all prepared
	 * and ready for retrieval and use.
	 *
	 * This function may kick off the process that prepares the texture for use.
	 *
	 * This function is used by the TextureRendererShaderPrimitive to indicate if it is ready
	 * for use.
	 *
	 * @return  true if a call to fetchTexture() will return an immediately-usable texture; false
	 *          otherwise.
	 */
	virtual bool                   isTextureReady() const = 0;

	void                           fetch() const;
	void                           release() const;
	int                            getReferenceCount() const;

	const TextureRendererTemplate &getTextureRendererTemplate() const;

protected:

	TextureRenderer(const TextureRendererTemplate &textureRendererTemplate);
	virtual ~TextureRenderer();

	void  signalModified();

private:

	// disabled
	TextureRenderer();
	TextureRenderer(const TextureRenderer&);
	TextureRenderer &operator =(const TextureRenderer&);

private:

	const TextureRendererTemplate &m_textureRendererTemplate;
	mutable int                    m_referenceCount;

};

// ======================================================================

inline void TextureRenderer::fetch() const
{
	++m_referenceCount;
}

// ----------------------------------------------------------------------

inline int TextureRenderer::getReferenceCount() const
{
	return m_referenceCount;
}

// ----------------------------------------------------------------------

inline const TextureRendererTemplate &TextureRenderer::getTextureRendererTemplate() const
{
	return m_textureRendererTemplate;
}

// ======================================================================

#endif
