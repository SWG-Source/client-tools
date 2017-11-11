// ======================================================================
//
// TextureArray1dElement.h
// copyright 2001 Sony Online Entertainment
// 
// ======================================================================

#ifndef INCLUDED_TextureArray1dElement_h
#define INCLUDED_TextureArray1dElement_h

// ======================================================================

#include "TextureElement.h"

class Iff;
class Texture;

// ======================================================================

class TextureArray1dElement: public TextureElement
{
public:

	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

	class ArrayElement: public Element
	{
	public:

		ArrayElement(TextureArray1dElement &array, int index);
		virtual ~ArrayElement();

		virtual std::string   getLabel() const;
		virtual unsigned int  getTypeIndex() const;

		virtual bool          userCanDelete() const;
		virtual void          doPreDeleteTasks();

		virtual bool          hasLeftDoubleClickAction() const;
		virtual bool          doLeftDoubleClickAction(TextureBuilderDoc &document);

		void                  setIndex(int index);
		int                   getIndex() const;

	private:

		TextureArray1dElement &m_array;
		int                    m_index;

	private:
		// disabled
		ArrayElement();
		ArrayElement(const ArrayElement&);
		ArrayElement &operator =(const ArrayElement&);
	};

	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

public:

	static bool           isPersistedNext(const Iff &iff);

public:

	TextureArray1dElement();
	explicit TextureArray1dElement(Iff &iff);
	explicit TextureArray1dElement(const std::string &arrayName);

	virtual ~TextureArray1dElement();

	virtual std::string   getLabel() const;
	virtual unsigned int  getTypeIndex() const;

	virtual bool          hasLeftDoubleClickAction() const;
	virtual bool          doLeftDoubleClickAction(TextureBuilderDoc &document);

	virtual bool          userCanDelete() const;

	virtual std::string   getShortLabel() const;

	virtual int           getArraySubscriptCount() const;
	virtual int           getArraySubscriptUpperBound(int subscriptIndex) const;

	virtual bool          writeForWorkspace(Iff &iff) const;

	virtual void          exportTextureRenderer(Exporter &exporter) const;

	const int             getTextureCount() const;
	const std::string    &getPathname(int index) const;
	const Texture        *getTexture(int index) const;

	bool                  hasPathname(const std::string &pathname);
	void                  addTexture(const std::string &projectRelativePathname);
	void                  setPathname(int index, const std::string &projectRelativePathname);

	void                  moveElementTowardFront(int elementIndex);
	void                  moveElementTowardBack(int elementIndex);

	void                  removeTextureByIndex(int elementIndex);

private:

	class TextureInfo;

	typedef stdvector<TextureInfo*>::fwd  TextureInfoVector;

private:

	void                  load_0000(Iff &iff);
	void                  renumberTextureElements();

private:

	std::string          *m_arrayName;
	TextureInfoVector    *m_textureInfoVector;

private:
	// disabled
	TextureArray1dElement(const TextureArray1dElement&);
	TextureArray1dElement &operator =(const TextureArray1dElement&);
};

// ======================================================================

inline void TextureArray1dElement::ArrayElement::setIndex(int index)
{
	VALIDATE_RANGE_INCLUSIVE_EXCLUSIVE(0, index, m_array.getTextureCount());
	m_index = index;
}

// ----------------------------------------------------------------------

inline int TextureArray1dElement::ArrayElement::getIndex() const
{
	return m_index;
}

// ======================================================================

#endif
