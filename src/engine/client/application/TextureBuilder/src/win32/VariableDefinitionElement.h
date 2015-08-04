// ======================================================================
//
// VariableDefinitionElement.h
// copyright 2001 Sony Online Entertainment
// 
// ======================================================================

#ifndef INCLUDED_VariableDefinitionElement_h
#define INCLUDED_VariableDefinitionElement_h

// ======================================================================

#include "Element.h"

class Iff;

// ======================================================================

class VariableDefinitionElement: public Element
{
public:

	static bool isPersistedNext(Iff &iff);

	static void loadVariableDefinitions(Iff &iff, ElementVector &elements);
	static bool writeVariableDefinitions(Iff &iff, const ElementVector &elements);

public:

	explicit VariableDefinitionElement(Iff &iff);
	VariableDefinitionElement(const std::string &variableDescription, const int typeId);
	virtual ~VariableDefinitionElement();

	virtual std::string   getLabel() const;
	virtual unsigned int  getTypeIndex() const;

	virtual bool          hasLeftDoubleClickAction() const;
	virtual bool          doLeftDoubleClickAction(TextureBuilderDoc &document);

	virtual bool          writeForWorkspace(Iff &iff) const;

	const std::string    &getVariableDescription() const;
	const std::string    &getVariableName() const;
	std::string           getFullyQualifiedVariableName(char const *defaultDirectoryName) const;

	const int             getTypeId() const;

private:

	void                  load_0000(Iff &iff);

private:

	std::string *m_variableDescription;
	std::string *m_variableName;
	int          m_typeId;

private:
	// disabled
	VariableDefinitionElement();
	VariableDefinitionElement(const VariableDefinitionElement&);
	VariableDefinitionElement &operator =(const VariableDefinitionElement&);
};

// ======================================================================

inline const std::string &VariableDefinitionElement::getVariableDescription() const
{
	return *m_variableDescription;
}

// ----------------------------------------------------------------------

inline const std::string &VariableDefinitionElement::getVariableName() const
{
	return *m_variableName;
}

// ----------------------------------------------------------------------

inline const int VariableDefinitionElement::getTypeId() const
{
	return m_typeId;
}

// ======================================================================

#endif
