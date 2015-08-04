// ======================================================================
//
// TextureRendererSet.h
// copyright 2001 Sony Online Entertainment
// 
// ======================================================================

#ifndef	TEXTURE_RENDERER_SET_H
#define TEXTURE_RENDERER_SET_H

// ======================================================================

class TextureRenderer;
class TextureRendererTemplate;

// ======================================================================
/**
 * Provides the vehicle for sharing a group of TextureRenderer objects among different
 * objects that have no knowlege of one another but need to use the same
 * TextureRenderer objects.
 * 
 * This class is used by the character system's SkeletalAppearance class.
 * For each SkeletalAppearance object's TextureRendererTemplate, only one
 * TextureRenderer object should exist.  Across LODs, the same TextureRenderer should be
 * used.  This object is reference counted and will destroy itself when
 * the reference count reaches zero.
 */

class TextureRendererSet
{
public:

	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

	struct TextureRendererData
	{
	public:

		explicit TextureRendererData(const TextureRendererTemplate *textureRendererTemplate);

	public:

		TextureRenderer *m_textureRenderer;
		int              m_referenceCount;

	private:
		// disabled
		TextureRendererData();
	};

	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

public:

	TextureRendererSet();

	void  fetch();
	void  release();

	TextureRenderer *fetchTextureRenderer(const TextureRendererTemplate *textureRendererTemplate, bool *isNew = 0);
	void             releaseTextureRenderer(TextureRenderer *textureRenderer);

private:

	~TextureRendererSet();

private:

	typedef stdvector<TextureRendererData>::fwd TextureRendererDataContainer;

private:

	int                           m_referenceCount;
	TextureRendererDataContainer *m_textureRenderers;

private:
	// disabled
	TextureRendererSet(const TextureRendererSet&);
	TextureRendererSet &operator =(const TextureRendererSet&);
};

// ======================================================================

inline void TextureRendererSet::fetch()
{
	++m_referenceCount;
}

// ======================================================================

#endif
