// ======================================================================
//
// VariableDefinitionElement.cpp
// copyright 2001 Sony Online Entertainment
// 
// ======================================================================

#include "FirstTextureBuilder.h"
#include "VariableDefinitionElement.h"

#include "DialogEnterName.h"
#include "ElementTypeIndex.h"
#include "sharedFile/Iff.h"

#include <string>
#include <vector>

// ======================================================================

namespace
{
	const Tag TAG_EVDE = TAG(E,V,D,E);
	const Tag TAG_EVDS = TAG(E,V,D,S);
}

// ======================================================================

bool VariableDefinitionElement::isPersistedNext(Iff &iff)
{
	return (iff.isCurrentForm() && (iff.getCurrentName() == TAG_EVDE));
}

// ----------------------------------------------------------------------

void VariableDefinitionElement::loadVariableDefinitions(Iff &iff, ElementVector &elements)
{
	FATAL(!(iff.isCurrentForm() && (iff.getCurrentName() == TAG_EVDS)), ("no variable definition IFF form where expected"));

	int count = 0;

	iff.enterForm(TAG_EVDS);
		iff.enterChunk(TAG_INFO);
			count = static_cast<int>(iff.read_int32());
		iff.exitChunk(TAG_INFO);

		for (int i = 0; i < count; ++i)
			elements.push_back(new VariableDefinitionElement(iff));

	iff.exitForm(TAG_EVDS);
}

// ----------------------------------------------------------------------

bool VariableDefinitionElement::writeVariableDefinitions(Iff &iff, const ElementVector &elements)
{
	//-- count # of elements
	int count = 0;

	const ElementVector::const_iterator itEnd = elements.end();
	{
		for (ElementVector::const_iterator it = elements.begin(); it != itEnd; ++it)
			if (dynamic_cast<VariableDefinitionElement*>(NON_NULL(*it)))
				++count;
	}

	//-- write variable definitions
	iff.insertForm(TAG_EVDS);

		iff.insertChunk(TAG_INFO);
			iff.insertChunkData(static_cast<int32>(count));
		iff.exitChunk(TAG_INFO);

		{
			for (ElementVector::const_iterator it = elements.begin(); it != itEnd; ++it)
			{
				VariableDefinitionElement *element = (dynamic_cast<VariableDefinitionElement*>(*it));
				if (element)
				{
					//-- write it
					if (!element->writeForWorkspace(iff))
						return false;
				}
			}
		}

	iff.exitForm(TAG_EVDS);

	return true;
}

// ======================================================================

VariableDefinitionElement::VariableDefinitionElement(Iff &iff)
:	Element(),
	m_variableDescription(new std::string()),
	m_variableName(new std::string()),
	m_typeId(-1)
{
	iff.enterForm(TAG_EVDE);

		switch (iff.getCurrentName())
		{
			case TAG_0000:
				load_0000(iff);
				break;
			default:
				{
					char version[5];
					ConvertTagToString(iff.getCurrentName(), version);
					FATAL(true, ("unsupported version [%s]", version));
				}
		}

	iff.exitForm(TAG_EVDE);
}

// ----------------------------------------------------------------------

VariableDefinitionElement::VariableDefinitionElement(const std::string &variableDescription, const int typeId)
:	Element(),
	m_variableDescription(new std::string(variableDescription)),
	m_variableName(new std::string()),
	m_typeId(typeId)
{
}

// ----------------------------------------------------------------------

VariableDefinitionElement::~VariableDefinitionElement()
{
	delete m_variableName;
	delete m_variableDescription;
}

// ----------------------------------------------------------------------

void VariableDefinitionElement::load_0000(Iff &iff)
{
	iff.enterForm(TAG_0000);
		iff.enterChunk(TAG_INFO);
		{
			char buffer[1024];

			iff.read_string(buffer, sizeof(buffer)-1);
			IGNORE_RETURN(m_variableDescription->assign(buffer));

			iff.read_string(buffer, sizeof(buffer)-1);
			IGNORE_RETURN(m_variableName->assign(buffer));

			m_typeId = static_cast<int>(iff.read_int32());
		}
		iff.exitChunk(TAG_INFO);
	iff.exitForm(TAG_0000);
}

// ----------------------------------------------------------------------

bool VariableDefinitionElement::writeForWorkspace(Iff &iff) const
{
	iff.insertForm(TAG_EVDE);	
		iff.insertForm(TAG_0000);
			iff.insertChunk(TAG_INFO);
			{
				iff.insertChunkString(m_variableDescription->c_str());
				iff.insertChunkString(m_variableName->c_str());
				iff.insertChunkData(static_cast<int32>(m_typeId));
			}
			iff.exitChunk(TAG_INFO);
		iff.exitForm(TAG_0000);
	iff.exitForm(TAG_EVDE);	

	return true;
}

// ----------------------------------------------------------------------

std::string VariableDefinitionElement::getFullyQualifiedVariableName(char const *defaultDirectoryName) const
{
	std::string fullVariableName = makeFullyQualifiedVariableName(defaultDirectoryName, getVariableName());

	// Upgrade variable name to fully qualified name if not already set.
	if (fullVariableName != getVariableName())
		*m_variableName = fullVariableName;

	return fullVariableName;
}

// ----------------------------------------------------------------------

std::string VariableDefinitionElement::getLabel() const
{
	std::string result(*m_variableDescription);
	result += ": ";
	result += *m_variableName;
	
	return result;
}

// ----------------------------------------------------------------------

unsigned int VariableDefinitionElement::getTypeIndex() const
{
	return ETI_variableDefinition;
}

// ----------------------------------------------------------------------

bool VariableDefinitionElement::hasLeftDoubleClickAction() const
{
	return true;
}

// ----------------------------------------------------------------------

bool VariableDefinitionElement::doLeftDoubleClickAction(TextureBuilderDoc&)
{
	DialogEnterName dlg(*m_variableName);

	if (dlg.DoModal() == IDOK)
	{
		//-- name was entered
		dlg.m_name.MakeLower();
		IGNORE_RETURN(m_variableName->assign(dlg.m_name));

		//-- indicate we're modified
		return true;
	}
	else
	{
		// indicate we did not modify this element
		return false;
	}
}

// ======================================================================
