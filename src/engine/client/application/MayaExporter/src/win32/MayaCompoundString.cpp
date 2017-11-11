// ======================================================================
//
// MayaCompoundString.cpp
// copyright 2001 Sony Online Entertainment
// 
// ======================================================================

#include "FirstMayaExporter.h"
#include "MayaCompoundString.h"

// ======================================================================

MayaCompoundString::MayaCompoundString()
:
	m_compoundString(),
	m_componentStrings(),
	m_separator('_')
{
}

// ----------------------------------------------------------------------

MayaCompoundString::MayaCompoundString(const MString &compoundString, char separator)
:
	m_compoundString(compoundString),
	m_componentStrings(),
	m_separator(separator)
{
	buildComponentStrings();
}

// ----------------------------------------------------------------------

MayaCompoundString::~MayaCompoundString()
{
	// nothing to do
}

// ----------------------------------------------------------------------

void MayaCompoundString::setCompoundString(const MString &compoundString)
{
	m_compoundString = compoundString;
	buildComponentStrings();
}

// ----------------------------------------------------------------------

void MayaCompoundString::buildComponentStrings()
{
	//-- clear existing contents of array
	MStatus status;

	status = m_componentStrings.clear();
	FATAL(!status, ("failed to clear component strings"));

	//-- build the components
	const size_t maxComponentLength = 2 * MAX_PATH;

	const unsigned    sourceLength  = m_compoundString.length();
	const char *const sourceCstring = m_compoundString.asChar();
	NOT_NULL(sourceCstring);

	char  componentBuffer[maxComponentLength];
	char *currentComponent = componentBuffer;

	for (unsigned i = 0; i < sourceLength; ++i)
	{
		const char sourceChar = sourceCstring[i];

		//-- check if we've finished the current component
		if (sourceChar == m_separator)
		{
			if (sourceCstring[i+1] == m_separator)
			{
				// found the end of a component
				*currentComponent = 0;
				status = m_componentStrings.append(MString(componentBuffer));
				FATAL(!status, ("failed to add component"));

				currentComponent = componentBuffer;
				++i;
				continue;
			}
		}

		//-- add the character to the current component
		if (currentComponent + 1 < (componentBuffer + maxComponentLength))
		{
			*currentComponent = sourceChar;
			++currentComponent;
		}
	}

	//-- add final component
	if (currentComponent != componentBuffer)
	{
		*currentComponent = 0;
		status = m_componentStrings.append(MString(componentBuffer));
		FATAL(!status, ("failed to add component"));
	}
}

// ======================================================================

