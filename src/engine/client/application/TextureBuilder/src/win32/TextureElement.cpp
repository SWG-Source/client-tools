// ======================================================================
//
// TextureElement.cpp
// copyright 2001 Sony Online Entertainment
// 
// ======================================================================

#include "FirstTextureBuilder.h"
#include "TextureElement.h"

#include <string>

// ======================================================================

namespace
{
	const char *const ms_directorySeparators       = "\\/";
	const char *const ms_textureReferenceDirectory = "texture\\";
}

// ======================================================================

TextureElement::TextureElement()
:	Element()
{
}

// ----------------------------------------------------------------------

TextureElement::~TextureElement()
{
}

// ----------------------------------------------------------------------

bool TextureElement::isDragSource() const
{
	//-- all textures can be drag sources
	return true;
}

// ----------------------------------------------------------------------

std::string TextureElement::getReferencePathname(const std::string &projectRelativePathname)
{
	std::string referencePathname(ms_textureReferenceDirectory);

	//-- scan backward for end of directory
	const std::string::size_type endOfDirectoryIndex = projectRelativePathname.find_last_of(ms_directorySeparators);
	if (static_cast<int>(endOfDirectoryIndex) != projectRelativePathname.npos)
	{
		// append texture filename without path
		IGNORE_RETURN(referencePathname.append(projectRelativePathname, endOfDirectoryIndex + 1, projectRelativePathname.length() - endOfDirectoryIndex - 1));
	}
	else
	{
		// no path in projectRelativePathname
		referencePathname += projectRelativePathname;
	}

	return referencePathname;
}

// ======================================================================
