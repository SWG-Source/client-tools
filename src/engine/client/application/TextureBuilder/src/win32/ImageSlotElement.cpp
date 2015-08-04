// ======================================================================
//
// ImageSlotElement.cpp
// copyright 2001 Sony Online Entertainment
// 
// ======================================================================

#include "FirstTextureBuilder.h"
#include "ImageSlotElement.h"

#include "BoolElement.h"
#include "BlueprintWriter.h"
#include "ElementTypeIndex.h"
#include "sharedFoundation/ExitChain.h"
#include "Exporter.h"
#include "sharedFile/Iff.h"
#include "BasicRangedIntVariableElement.h"
#include "clientGraphics/Texture.h"
#include "TextureArray1dElement.h"
#include "TextureElement.h"
#include "clientGraphics/TextureList.h"
#include "TextureSingleElement.h"
#include "VariableDefinitionElement.h"

#include <string>
#include <vector>

// ======================================================================

namespace
{
	const std::string ms_uVariableName = "Flip U";
	const std::string ms_vVariableName = "Flip V";
	const std::string ms_uvOnName      = "yes";
	const std::string ms_uvOffName     = "no";

	const Tag         TAG_EISL         = TAG(E,I,S,L);

	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

	const Texture *ms_defaultTexture;

	bool  ms_installed;
}

// ======================================================================

namespace
{
	/**
	 * Find the source texture in the texture list with a given short name and index type.
	 */
	TextureElement *findSourceTexture(Element::ElementVector &sourceTextures, const std::string &textureName, unsigned int indexType)
	{
		const Element::ElementVector::iterator itEnd = sourceTextures.end();
		for (Element::ElementVector::iterator it = sourceTextures.begin(); it != itEnd; ++it)
		{
			TextureElement *const textureElement = dynamic_cast<TextureElement*>(NON_NULL(*it));
			if (textureElement && (textureElement->getTypeIndex() == indexType) && (textureElement->getShortLabel() == textureName))
				return textureElement;
		}

		//-- didn't find it
		return NULL;
	}
}

// ======================================================================

void ImageSlotElement::install()
{
	DEBUG_FATAL(ms_installed, ("ImageSlotElement already installed"));

	ExitChain::add(remove, "ImageSlotElement");
	ms_defaultTexture = TextureList::fetchDefaultTexture();

	ms_installed = true;
}

// ----------------------------------------------------------------------

void ImageSlotElement::remove()
{
	DEBUG_FATAL(!ms_installed, ("ImageSlotElement not installed"));

	ms_defaultTexture->release();
	ms_defaultTexture = 0;

	ms_installed = false;
}

// ----------------------------------------------------------------------

bool ImageSlotElement::isPersistedNext(Iff &iff)
{
	return (iff.isCurrentForm() && (iff.getCurrentName() == TAG_EISL));
}

// ======================================================================

ImageSlotElement::ImageSlotElement()
:	VariableContainerElement(),
	m_attachedTexture(0),
	m_flipU(false),
	m_flipV(false)
{
	DEBUG_FATAL(!ms_installed, ("ImageSlotElement not installed"));

	ElementVector &children = getChildren();
	children.push_back(new BoolElement(m_flipU, ms_uVariableName, ms_uvOnName, ms_uvOffName));
	children.push_back(new BoolElement(m_flipV, ms_vVariableName, ms_uvOnName, ms_uvOffName));
}

// ----------------------------------------------------------------------

ImageSlotElement::ImageSlotElement(Iff &iff, ElementVector &sourceTextures)
:	VariableContainerElement(),
	m_attachedTexture(0),
	m_flipU(false),
	m_flipV(false)
{
	DEBUG_FATAL(!ms_installed, ("ImageSlotElement not installed"));

	ElementVector &children = getChildren();
	children.push_back(new BoolElement(m_flipU, ms_uVariableName, ms_uvOnName, ms_uvOffName));
	children.push_back(new BoolElement(m_flipV, ms_vVariableName, ms_uvOnName, ms_uvOffName));

	iff.enterForm(TAG_EISL);

		switch (iff.getCurrentName())
		{
			case TAG_0000:
				load_0000(iff, sourceTextures);
				break;
			default:
				{
					char version[5];
					ConvertTagToString(iff.getCurrentName(), version);
					FATAL(true, ("unsupported version [%s]", version));
				}
		}

	iff.exitForm(TAG_EISL);
}

// ----------------------------------------------------------------------

ImageSlotElement::~ImageSlotElement()
{
	m_attachedTexture = 0;
}

// ----------------------------------------------------------------------

void ImageSlotElement::load_0000(Iff &iff, ElementVector &sourceTextures)
{
	iff.enterForm(TAG_0000);
		iff.enterChunk(TAG_INFO);
		{
			m_flipU                      = (iff.read_uint8() != 0) ? true : false;
			m_flipV                      = (iff.read_uint8() != 0) ? true : false;
			const unsigned int indexType = static_cast<unsigned int>(iff.read_uint32());

			char buffer[1024];
			iff.read_string(buffer, sizeof(buffer)-1);
			
			m_attachedTexture = findSourceTexture(sourceTextures, buffer, indexType);
		}
		iff.exitChunk(TAG_INFO);

		VariableDefinitionElement::loadVariableDefinitions(iff, getChildren());

	iff.exitForm(TAG_0000);
}

// ----------------------------------------------------------------------

bool ImageSlotElement::writeForWorkspace(Iff &iff) const
{
	iff.insertForm(TAG_EISL);	
		iff.insertForm(TAG_0000);
			iff.insertChunk(TAG_INFO);
			{
				iff.insertChunkData(static_cast<uint8>(m_flipU));
				iff.insertChunkData(static_cast<uint8>(m_flipV));

				if (m_attachedTexture)
				{
					iff.insertChunkData(static_cast<unsigned int>(m_attachedTexture->getTypeIndex()));
					iff.insertChunkString(m_attachedTexture->getShortLabel().c_str());
				}
				else
				{
					iff.insertChunkData(static_cast<unsigned int>(0));
					iff.insertChunkString("");
				}
			}
			iff.exitChunk(TAG_INFO);

			if (!VariableDefinitionElement::writeVariableDefinitions(iff, getChildren()))
				return false;

		iff.exitForm(TAG_0000);
	iff.exitForm(TAG_EISL);	

	return true;
}

// ----------------------------------------------------------------------

std::string ImageSlotElement::getLabel() const
{
	std::string label("Texture slot: ");
	if (m_attachedTexture)
	{
		label += m_attachedTexture->getShortLabel();
		//m_attachedTexture->appendTextureName(label);
		//-- if an array texture, list variables that parameterize it
	}
	else
	{
		//-- no image assigned
		label += "<not assigned>";
	}

	return label;
}

// ----------------------------------------------------------------------

unsigned int ImageSlotElement::getTypeIndex() const
{
	return ETI_imageSlot;
}

// ----------------------------------------------------------------------

bool ImageSlotElement::isDragTarget(const Element &source) const
{
	//-- we only accept TextureElement-derived classes
	if (dynamic_cast<const TextureElement*>(&source))
		return true;
	else
		return false;
}

// ----------------------------------------------------------------------

void ImageSlotElement::dropElementHere(Element &source)
{
	m_attachedTexture = dynamic_cast<TextureElement*>(&source);
	FATAL(!m_attachedTexture, ("dropElementHere(): source element is not a texture type, type index %u", source.getTypeIndex()));

	//-- add variable definition elements as needed
	removeAllVariableDefinitionElements();

	ElementVector &children = getChildren();

	const int arraySubscriptCount = m_attachedTexture->getArraySubscriptCount();
	for (int i = 0; i < arraySubscriptCount; ++i)
	{
		char nameBuffer[64];

		//-- note we create the variable definition element with an id such that
		//   id 0 is the fastest moving subscript (i.e. the c++ rightmost subscript).
		sprintf(nameBuffer, "<array subscript %d>", arraySubscriptCount-i);
		children.push_back(new VariableDefinitionElement(std::string(nameBuffer), i));
	}
}

// ----------------------------------------------------------------------

const Texture &ImageSlotElement::getTexture(CustomizationData const &customizationData) const
{
	DEBUG_FATAL(!isSlotFilled(), ("tried to getTexture() when image slot is not filled"));

	// variables are bound here, not within texture
	
	const TextureSingleElement *const textureSingleElement = dynamic_cast<const TextureSingleElement*>(m_attachedTexture);
	if (textureSingleElement)
	{
		// dealing with a texture single element
		return textureSingleElement->getTexture();
	}

	const TextureArray1dElement *const array1dElement = dynamic_cast<TextureArray1dElement*>(m_attachedTexture);
	if (array1dElement)
	{
		// dealing with an array1d element
		//-- fetch value of first index
		const int index = getIntVariableDefinitionValue(0, 0, customizationData, 0);

		const bool useArrayTexture = (array1dElement->getTextureCount() > 1) && WithinRangeInclusiveInclusive(0, index, array1dElement->getTextureCount()-1);
		if (useArrayTexture)
			return *NON_NULL(array1dElement->getTexture(index));
		else
			return *ms_defaultTexture;
	} 

	DEBUG_FATAL(true, ("shouldn't get here, must be new texture type"));
	return *reinterpret_cast<const Texture*>(0); //lint !e413 !e527 // unreachable and use of null pointer // yup, for MSVC
}

// ----------------------------------------------------------------------

void ImageSlotElement::createVariableElements(VariableElementVector &variables) const
{
	const ElementVector                 &children = getChildren();
	const ElementVector::const_iterator  itEnd    = children.end();
	for (ElementVector::const_iterator it = children.begin(); it != itEnd; ++it)
	{
		const VariableDefinitionElement *const vdElement = dynamic_cast<const VariableDefinitionElement*>(*it);
		if (vdElement && vdElement->getVariableName().length())
		{
			switch (vdElement->getTypeId())
			{
				// always the fastest-moving array index as elements are walked in memory-linear order (i.e. rightmost)
				case 0:
					{
						NOT_NULL(m_attachedTexture);
						const int upperBoundExclusive = m_attachedTexture->getArraySubscriptUpperBound(0);
						if (upperBoundExclusive > 0)
							variables.push_back(new BasicRangedIntVariableElement(vdElement->getFullyQualifiedVariableName("/shared_owner").c_str(), 0, 0, upperBoundExclusive - 1));
					}
					break;

				default:
					FATAL(true, ("unknown variable type id [%d]", vdElement->getTypeId()));
			}
		}
	}
}

// ----------------------------------------------------------------------

void ImageSlotElement::exportPrepareOperations(Exporter &exporter, int shaderIndex, Tag textureTag) const
{
	//-- make sure the texture is attached
	FATAL(!m_attachedTexture, ("cannot export, image slot not attached to image source"));

	//-- get the texture index for the texture element (texture elements already exported)
	const int textureIndex = exporter.getTextureIndex(*m_attachedTexture);
	FATAL(textureIndex < 0, ("bad textureIndex, was this texture exported properly? [%s]", m_attachedTexture->getLabel().c_str()));

	BlueprintWriter &writer = exporter.getWriter();

	// the prepare operation we add depends on the type of texture element

	//-- handle single texture element
	const TextureSingleElement *const textureSingleElement = dynamic_cast<const TextureSingleElement*>(m_attachedTexture);
	if (textureSingleElement)
	{
		// dealing with a texture single element
		writer.addSetShaderConstantTextureOperation(shaderIndex, textureTag, textureIndex);
		return;
	}

	//-- handle 1d texture array element
	const TextureArray1dElement *const array1dElement = dynamic_cast<TextureArray1dElement*>(m_attachedTexture);
	if (array1dElement)
	{
		// dealing with an array1d element
		//-- get variable descriptor for array element
		const VariableDefinitionElement *vdefElement = getVariableDefinitionElement(0, 0);
		NOT_NULL(vdefElement);

		std::string const shortVariableName = Element::makeShortVariableName(vdefElement->getVariableName());
		writer.addSetShaderTexture1dOperation(shaderIndex, textureTag, textureIndex, shortVariableName, array1dElement->getTextureCount());
	}

	//-- handle 2d texture array element
	// -TRF- to do
}

// ======================================================================
