// ======================================================================
//
// DestinationTextureElement.cpp
// copyright 2001 Sony Online Entertainment
// 
// ======================================================================

#include "FirstTextureBuilder.h"
#include "DestinationTextureElement.h"

#include "DestinationTextureDimensionElement.h"
#include "ElementTypeIndex.h"
#include "sharedFile/Iff.h"

#include <string>
#include <vector>

// ======================================================================

namespace
{
	const Tag TAG_EDTX = TAG(E,D,T,X);
}

// ======================================================================

bool DestinationTextureElement::isPersistedNext(Iff &iff)
{
	return ((iff.getCurrentName() == TAG_EDTX) && iff.isCurrentForm());
}

// ======================================================================

DestinationTextureElement::DestinationTextureElement(Iff &iff)
:	Element(),
	m_width(0),
	m_height(0)
{
	//-- create component elements
	ElementVector &children = getChildren();

	children.push_back(new DestinationTextureDimensionElement(*this, DestinationTextureDimensionElement::DT_width));
	children.push_back(new DestinationTextureDimensionElement(*this, DestinationTextureDimensionElement::DT_height));

	//-- load from IFF
	iff.enterForm(TAG_EDTX);

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

	iff.exitForm(TAG_EDTX);
}

// ----------------------------------------------------------------------

DestinationTextureElement::DestinationTextureElement(int width, int height)
:	Element(),
	m_width(width),
	m_height(height)
{
	ElementVector &children = getChildren();

	children.push_back(new DestinationTextureDimensionElement(*this, DestinationTextureDimensionElement::DT_width));
	children.push_back(new DestinationTextureDimensionElement(*this, DestinationTextureDimensionElement::DT_height));
}

// ----------------------------------------------------------------------

DestinationTextureElement::~DestinationTextureElement()
{
}

// ----------------------------------------------------------------------

void DestinationTextureElement::load_0000(Iff &iff)
{
	iff.enterForm(TAG_0000);
		iff.enterChunk(TAG_INFO);
		{
			m_width  = static_cast<int>(iff.read_int32());
			m_height = static_cast<int>(iff.read_int32());
		}
		iff.exitChunk(TAG_INFO);
	iff.exitForm(TAG_0000);
}

// ----------------------------------------------------------------------

bool DestinationTextureElement::writeForWorkspace(Iff &iff) const
{
	iff.insertForm(TAG_EDTX);
		iff.insertForm(TAG_0000);
			iff.insertChunk(TAG_INFO);
			{
				iff.insertChunkData(static_cast<int32>(m_width));
				iff.insertChunkData(static_cast<int32>(m_height));
			}
			iff.exitChunk(TAG_INFO);
		iff.exitForm(TAG_0000);
	iff.exitForm(TAG_EDTX);

	return true;
}

// ----------------------------------------------------------------------

std::string DestinationTextureElement::getLabel() const
{
	return "Destination Texture";
}

// ----------------------------------------------------------------------

unsigned int DestinationTextureElement::getTypeIndex() const
{
	return ETI_destinationTexture;
}

// ======================================================================
