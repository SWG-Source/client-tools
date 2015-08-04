// ======================================================================
//
// PathElement.h
// Copyright 2002 Sony Online Entertainment, Inc.
// All Rights Reserved.
//
// ======================================================================

#include "FirstTextureBuilder.h"
#include "PathElement.h"

#include "ElementTypeIndex.h"
#include "sharedFile/TreeFile.h"

#include <string>

// ======================================================================

PathElement::PathElement(const std::string &fileType, const std::string &fileExtension, const std::string &fileFilter, const std::string &defaultPathName) :
	Element(),
	m_fileType(new std::string(fileType)),
	m_fileExtension(new std::string(fileExtension)),
	m_fileFilter(new std::string(fileFilter)),
	m_pathName(new std::string(defaultPathName))
{
}

// ----------------------------------------------------------------------

PathElement::~PathElement()
{
	delete m_pathName;
	delete m_fileFilter;
	delete m_fileExtension;
	delete m_fileType;
}

// ----------------------------------------------------------------------

std::string PathElement::getLabel() const
{
	return std::string("file: ") + *m_pathName;
}

// ----------------------------------------------------------------------

unsigned int PathElement::getTypeIndex() const
{
	return ETI_path;
}

// ----------------------------------------------------------------------

bool PathElement::hasLeftDoubleClickAction() const
{
	return true;
}

// ----------------------------------------------------------------------

bool PathElement::doLeftDoubleClickAction(TextureBuilderDoc & /* document */)
{
	//-- Convert current TreeFile-relative pathname to a file-system pathname.
	char filesystemPathName[4*MAX_PATH];

	const bool treeToFsSuccess = TreeFile::getPathName(m_pathName->c_str(), filesystemPathName, sizeof(filesystemPathName) - 1);
	if (!treeToFsSuccess)
		strcpy(filesystemPathName, m_pathName->c_str());

	//-- Convert / to \.  TreeFile::getPathName changed since I wrote this.
	size_t const length = strlen(filesystemPathName);
	for (size_t i = 0; i < length; ++i)
	{
		if (filesystemPathName[i] == '/')
			filesystemPathName[i] = '\\';
	}

	//-- Display file selection dialog.
	CFileDialog dlg(TRUE, m_fileExtension->c_str(), filesystemPathName, 0, m_fileFilter->c_str(), NULL);
	bool        isModified = false;

	if (dlg.DoModal() == IDOK)
	{
		//-- Convert dlg pathname into a TreeFile-relative pathname.
		std::string  treeFilePathName;

		const bool fsToTreeSuccess = TreeFile::stripTreeFileSearchPathFromFile(std::string(dlg.GetPathName()), treeFilePathName);
		if (fsToTreeSuccess)
		{
			*m_pathName = treeFilePathName;

			//-- Indicate the element was modified.
			isModified = true;
		}
		else
		{
			//-- Indicate the specified filename was not in the TreeFile path.
			char buffer[1024];

			sprintf(buffer, "The specified file [%s] is not within the TreeFile search path, ignoring.", static_cast<const char*>(dlg.GetPathName()));
			MessageBox(NULL, buffer, "File Not in TreeFile Path", MB_OK | MB_ICONSTOP);
		}
	}

	//-- Return modified status.
	return isModified;
}

// ----------------------------------------------------------------------

const std::string &PathElement::getPathName() const
{
	return *m_pathName;
}

// ======================================================================
