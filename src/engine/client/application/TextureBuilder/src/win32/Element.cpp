// ======================================================================
//
// Element.cpp
// Copyright 2001-2003 Sony Online Entertainment, Inc.
// All rights reserved.
// 
// ======================================================================

#include "FirstTextureBuilder.h"
#include "Element.h"

#include "ElementTypeIndex.h"
#include "sharedFoundation/PointerDeleter.h"

#include <algorithm>
#include <string>
#include <vector>

// ======================================================================

std::string Element::makeFullyQualifiedVariableName(char const *defaultDirectoryName, std::string const &variableName)
{
	if (variableName.empty())
		return "";

	std::string  fullVariableName;

	if (variableName[0] == '/')
		fullVariableName = variableName;
	else
	{
		fullVariableName = defaultDirectoryName;
		if (fullVariableName.empty() || (fullVariableName[fullVariableName.length() - 1] != '/'))
			fullVariableName += '/';

		fullVariableName += variableName;
	}

	return fullVariableName;
}

// ----------------------------------------------------------------------

std::string Element::makeShortVariableName(std::string const &variableName)
{
	//-- Check if name appears to already be short.
	std::string::size_type lastSlashPosition = variableName.rfind('/');
	if (static_cast<int>(lastSlashPosition) == static_cast<int>(std::string::npos))
	{
		// This name already is a short variable name.
		return variableName;
	}
	else
	{
		return std::string(variableName, lastSlashPosition + 1);
	}
}

// ======================================================================

Element::Element()
:	m_children(new ElementVector())
{
}

// ----------------------------------------------------------------------

Element::~Element()
{
	std::for_each(m_children->begin(), m_children->end(), PointerDeleter());
	delete m_children;
}

// ----------------------------------------------------------------------

bool Element::isDragSource() const
{
	//-- default is that selected object is not a drag source
	return false;
}

// ----------------------------------------------------------------------

bool Element::isDragTarget(const Element &source) const
{
	//-- default is that selected object is not a drag target
	UNREF(source);
	return false;
}

// ----------------------------------------------------------------------

void Element::dropElementHere(Element &source)
{
	UNREF(source);

	// if we got here, it's because a derived class overloaded isDragTarget(),
	// returned true within it, then did not override dropElementHere().
	FATAL(true, ("implement virtual Element::dropElementHere() for element with label [%s]", source.getLabel().c_str()));
}

// ----------------------------------------------------------------------

unsigned int Element::getTypeIndex() const
{
	return ETI_unknown;
}

// ----------------------------------------------------------------------

bool Element::hasLeftDoubleClickAction() const
{
	return false;
}

// ----------------------------------------------------------------------

bool Element::doLeftDoubleClickAction(TextureBuilderDoc&)
{
	// if we got here, it's because a derived class overloaded hasLeftDoubleClickAction(),
	// returned true within it, then did not override doLeftDoubleClickAction().
	FATAL(true, ("implement virtual Element::doLeftDoubleClickAction() for element with label [%s]", getLabel().c_str()));
	return false; //lint !e527 // unreachable // for MSVC
}

// ----------------------------------------------------------------------

bool Element::userCanDelete() const
{
	// By default, an element cannot be deleted.
	return false;
}

// ----------------------------------------------------------------------

void Element::doPreDeleteTasks()
{
	// No default pre-delete tasks.
}

// ----------------------------------------------------------------------

namespace
{
	bool HasDescendantHelper(const Element &checkElement, const Element &targetElement)
	{
		//-- check the check element
		if (&checkElement == &targetElement)
			return true;
		else
		{
			//-- check children
			const Element::ElementVector children = checkElement.getChildren();

			const Element::ElementVector::const_iterator itEnd = children.end();
			for (Element::ElementVector::const_iterator it = children.begin(); it != itEnd; ++it)
			{
				if (HasDescendantHelper(*(*it), targetElement))
					return true;
			}
		}

		//-- didn't find it
		return false;
	}
}

// ----------------------------------------------------------------------

bool Element::hasDescendant(const Element &element) const
{
	return HasDescendantHelper(*this, element);
}

// ----------------------------------------------------------------------

namespace
{
	void CreateHierarchyVariableElementsHelper(const Element &element, Element::VariableElementVector &variables)
	{
		//-- create this element's variables
		element.createVariableElements(variables);

		//-- create children variables
		const Element::ElementVector children = element.getChildren();

		const Element::ElementVector::const_iterator itEnd = children.end();
		for (Element::ElementVector::const_iterator it = children.begin(); it != itEnd; ++it)
			CreateHierarchyVariableElementsHelper(*(*it), variables);
	}
}

// ----------------------------------------------------------------------

void Element::createHierarchyVariableElements(VariableElementVector &variables) const
{
	CreateHierarchyVariableElementsHelper(*this, variables);
}

// ----------------------------------------------------------------------

void Element::createVariableElements(VariableElementVector&) const
{
	//-- default: do nothing because element has no variable elements
}

// ----------------------------------------------------------------------

bool Element::writeForWorkspace(Iff&) const
{
	FATAL(true, ("shouldn't get here, caller asked class to write itself but no writeForWorkspace() provided"));
	return false; //lint !e527 // unreachable // for MSVC
}

// ======================================================================
