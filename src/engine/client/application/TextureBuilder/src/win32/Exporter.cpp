// ======================================================================
//
// Exporter.cpp
// copyright 2001 Sony Online Entertainment
// 
// ======================================================================

#include "FirstTextureBuilder.h"
#include "Exporter.h"

#include "BlueprintWriter.h"
#include "sharedFile/Iff.h"
#include "TextureElement.h"

#include <utility>
#include <map>
#include <string>

// ======================================================================

namespace
{
	const int ms_iffSize = 128 * 1024;
}

// ======================================================================

Exporter::Exporter()
:	m_blueprintWriter(new BlueprintWriter),
	m_textureIndexMap(new TextureIndexMap)
{
}

// ----------------------------------------------------------------------

Exporter::~Exporter()
{
}

// ----------------------------------------------------------------------
/**
 * Return the BlueprintWriter's texture index associated with the
 * specified textureElement.
 *
 * @return  index of TextureElement's (first) element (>= 0); -1 if not found.
 */

int Exporter::getTextureIndex(const TextureElement &textureElement)
{
	TextureIndexMap::const_iterator it = m_textureIndexMap->find(&textureElement);
	if (it != m_textureIndexMap->end())
		return it->second;
	else
		return -1;
}

// ----------------------------------------------------------------------

void Exporter::setTextureIndex(const TextureElement &textureElement, int index)
{
	std::pair<TextureIndexMap::iterator, bool> result = m_textureIndexMap->insert(TextureIndexMap::value_type(&textureElement, index));
	DEBUG_FATAL(!result.second, ("failed to add textureElement [%s] to list, probably already there", textureElement.getLabel().c_str()));
}

// ----------------------------------------------------------------------

bool Exporter::writeToFile(const std::string &fullPathname)
{
	//-- construct the iff image
	Iff iff(ms_iffSize);

	m_blueprintWriter->write(iff);

	//-- save to disk
	return iff.write(fullPathname.c_str(), true);
}

// ======================================================================
