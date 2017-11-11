// ======================================================================
//
// HueElement.h
// Copyright 2002 Sony Online Entertainment, Inc.
// All Rights Reserved.
//
// ======================================================================

#ifndef INCLUDED_HueElement_H
#define INCLUDED_HueElement_H

// ======================================================================

#include "Element.h"

// class IntSliderElement;
class Iff;
class PathElement;
class VariableDefinitionElement;

// ======================================================================

class HueElement: public Element
{
public:

	explicit HueElement(Iff &iff);
	explicit HueElement(const std::string &description);
	virtual ~HueElement();

	virtual std::string   getLabel() const;
	virtual unsigned int  getTypeIndex() const;

	const std::string    &getVariableName() const;
	std::string           getFullyQualifiedVariableName(char const *defaultDirectory) const;
	bool                  isVariablePrivate() const;

	const std::string    &getPalettePathName() const;
	int                   getDefaultPaletteIndex() const;

	virtual bool          writeForWorkspace(Iff &iff) const;

private:

	void load_0000(Iff &iff);

	// Disabled.
	HueElement();
	HueElement(const HueElement&);
	HueElement &operator =(const HueElement&);

private:

	static const std::string  cms_variableDescription;

private:

	std::string               *m_description;
	PathElement               *m_palettePathElement;
	VariableDefinitionElement *m_variableElement;
	bool                       m_isVariablePrivate;
	int                        m_defaultPaletteIndex;

};

// ======================================================================

inline bool HueElement::isVariablePrivate() const
{
	return m_isVariablePrivate;
}

// ======================================================================

#endif
