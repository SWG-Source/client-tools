// ======================================================================
//
// BoolElement.h
// copyright 2001 Sony Online Entertainment
// 
// ======================================================================

#ifndef INCLUDED_BoolElement_h
#define INCLUDED_BoolElement_h

// ======================================================================

#include "Element.h"

// ======================================================================

class BoolElement: public Element
{
public:

	BoolElement(bool &boolVariable, const std::string &variableName, const std::string &onString, const std::string &offString);
	virtual ~BoolElement();

	virtual std::string   getLabel() const;
	virtual unsigned int  getTypeIndex() const;

	virtual bool          hasLeftDoubleClickAction() const;
	virtual bool          doLeftDoubleClickAction(TextureBuilderDoc &document);

private:

	bool        &m_variable;
	std::string *m_variableName;
	std::string *m_onString;
	std::string *m_offString;

private:
	// disabled
	BoolElement();
	BoolElement(const BoolElement&);
	BoolElement &operator =(const BoolElement&);
};

// ======================================================================

#endif
