// ======================================================================
//
// HueElement.cpp
// Copyright 2002 Sony Online Entertainment, Inc.
// All Rights Reserved.
//
// ======================================================================

#include "FirstTextureBuilder.h"
#include "HueElement.h"

#include "BoolElement.h"
#include "ElementTypeIndex.h"
#include "PathElement.h"
#include "sharedFile/Iff.h"
#include "VariableDefinitionElement.h"

#include <string>
#include <vector>

// ======================================================================

const std::string HueElement::cms_variableDescription("variable name");
namespace
{
	const std::string stringPrivate = "private";
	const std::string stringYes = "yes";
	const std::string stringNo = "no";
	const std::string stringPalette = "Palette";
	const std::string stringPal     = "pal";
	const std::string stringPalFile = "Palette File (*.pal)|*.pal||";
}

const Tag TAG_HUE = TAG3(H,U,E);

// ======================================================================

HueElement::HueElement(Iff &iff) :
	Element(),
	m_description(new std::string),
	m_palettePathElement(0),
	m_variableElement(0),
	m_isVariablePrivate(true),
	m_defaultPaletteIndex(0)
{
	iff.enterForm(TAG_HUE);

		const Tag version = iff.getCurrentName();
		switch (version)
		{
			case TAG_0000:
				load_0000(iff);
				break;

			default:
				{
					char buffer[5];

					ConvertTagToString(version, buffer);
					DEBUG_FATAL(true, ("unsupported HueElement version [%s].", buffer));
				}
		}

	iff.exitForm(TAG_HUE);
}

// ----------------------------------------------------------------------

HueElement::HueElement(const std::string &description) :
	Element(),
	m_description(new std::string(description)),
	m_palettePathElement(0),
	m_variableElement(0),
	m_isVariablePrivate(true),
	m_defaultPaletteIndex(0)
{
	//-- Add children.
	ElementVector &children = getChildren();

	m_variableElement = new VariableDefinitionElement(cms_variableDescription, 0);
	children.push_back(m_variableElement);
	children.push_back(new BoolElement(m_isVariablePrivate, stringPrivate, stringYes, stringNo));

	m_palettePathElement = new PathElement(stringPalette, stringPal, stringPalFile, "renameMe.pal");
	children.push_back(m_palettePathElement);
}

// ----------------------------------------------------------------------

HueElement::~HueElement()
{
	m_palettePathElement = 0;
	m_variableElement    = 0;

	delete m_description;
}

// ----------------------------------------------------------------------

std::string HueElement::getLabel() const
{
	return "hue: " + *m_description;
}

// ----------------------------------------------------------------------

unsigned int HueElement::getTypeIndex() const
{
	return ETI_hue;
}

// ----------------------------------------------------------------------

const std::string &HueElement::getVariableName() const
{
	NOT_NULL(m_variableElement);
	return m_variableElement->getVariableName();
}

// ----------------------------------------------------------------------

std::string HueElement::getFullyQualifiedVariableName(char const * /* defaultDirectory */) const
{
	return m_variableElement->getFullyQualifiedVariableName(m_isVariablePrivate ? "/private/" : "/shared_owner/");
}

// ----------------------------------------------------------------------

const std::string &HueElement::getPalettePathName() const
{
	NOT_NULL(m_palettePathElement);
	return m_palettePathElement->getPathName();
}

// ----------------------------------------------------------------------

int HueElement::getDefaultPaletteIndex() const
{
	return m_defaultPaletteIndex;
}

// ----------------------------------------------------------------------

bool HueElement::writeForWorkspace(Iff &iff) const
{
	iff.insertForm(TAG_HUE);
		iff.insertForm(TAG_0000);

			iff.insertChunk(TAG_INFO);

				//-- Write description.
				iff.insertChunkString(m_description->c_str());

				//-- Write palette path name.
				iff.insertChunkString(getPalettePathName().c_str());

				//-- Write default palette index.
				// @todo replace with real default.
				iff.insertChunkData(static_cast<int16>(0));

				//-- Write variable private/shared status.
				iff.insertChunkData(static_cast<int8>(m_isVariablePrivate ? 1 : 0));

			iff.exitChunk(TAG_INFO);

			//-- Write VariableDefinitionElement.
			NOT_NULL(m_variableElement);
			const bool success = m_variableElement->writeForWorkspace(iff);
			if (!success)
				return false;

		iff.exitForm(TAG_0000);
	iff.exitForm(TAG_HUE);

	//-- Success.
	return true;
}

// ======================================================================

void HueElement::load_0000(Iff &iff)
{
	std::string    palettePathName;
	int            defaultPaletteIndex;
	ElementVector &children = getChildren();

	iff.enterForm(TAG_0000);
		
		iff.enterChunk(TAG_INFO);

			//-- Load the description.
			iff.read_string(*m_description);

			//-- Load the palette path name.
			iff.read_string(palettePathName);

			//-- Load default palette index.
			defaultPaletteIndex = static_cast<int>(iff.read_int16());
			UNREF(defaultPaletteIndex);

			//-- Read variable private/shared status.
			m_isVariablePrivate = (iff.read_int8() != 0);

		iff.exitChunk(TAG_INFO);

		//-- Load palette color VariableDefinitionElement.
		m_variableElement = new VariableDefinitionElement(iff);
		children.push_back(m_variableElement);

	iff.exitForm(TAG_0000);

	children.push_back(new BoolElement(m_isVariablePrivate, stringPrivate, stringYes, stringNo));

	m_palettePathElement = new PathElement(stringPalette, stringPal, stringPalFile, palettePathName);
	children.push_back(m_palettePathElement);
}

// ======================================================================
