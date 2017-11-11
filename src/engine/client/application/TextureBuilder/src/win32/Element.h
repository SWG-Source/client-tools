// ======================================================================
//
// Element.h
// copyright 2001 Sony Online Entertainment
// 
// ======================================================================

#ifndef INCLUDED_Element_h
#define INCLUDED_Element_h

// ======================================================================

class Iff;
class TextureBuilderDoc;
class VariableElement;

// ======================================================================

class Element
{
public:

	typedef stdvector<Element*>::fwd          ElementVector;
	typedef stdvector<VariableElement*>::fwd  VariableElementVector;

public:

	static std::string    makeFullyQualifiedVariableName(char const *defaultDirectoryName, std::string const &variableName);
	static std::string    makeShortVariableName(std::string const &variableName);

public:

	virtual ~Element();

	ElementVector        &getChildren();
	const ElementVector  &getChildren() const;

	bool                  hasDescendant(const Element &element) const;

	void                  createHierarchyVariableElements(VariableElementVector &variables) const;

	virtual std::string   getLabel() const = 0;
	virtual unsigned int  getTypeIndex() const;

	virtual bool          isDragSource() const;
	virtual bool          isDragTarget(const Element &source) const;
	virtual void          dropElementHere(Element &source);

	virtual bool          hasLeftDoubleClickAction() const;
	virtual bool          doLeftDoubleClickAction(TextureBuilderDoc &document);

	virtual bool          userCanDelete() const;
	virtual void          doPreDeleteTasks();

	virtual void          createVariableElements(VariableElementVector &variables) const;

	virtual bool          writeForWorkspace(Iff &iff) const;

protected:

	Element();

private:

	ElementVector *m_children;

private:
	// disabled
	Element(const Element&);
	Element &operator =(const Element&);
};

// ======================================================================

inline Element::ElementVector &Element::getChildren()
{
	return *m_children;
}

// ----------------------------------------------------------------------

inline const Element::ElementVector &Element::getChildren() const
{
	return *m_children;
}

// ======================================================================

#endif
