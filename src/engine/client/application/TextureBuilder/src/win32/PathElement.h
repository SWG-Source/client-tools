// ======================================================================
//
// PathElement.h
// Copyright 2002 Sony Online Entertainment, Inc.
// All Rights Reserved.
//
// ======================================================================

#ifndef INCLUDED_PathElement_H
#define INCLUDED_PathElement_H

// ======================================================================

#include "Element.h"

// ======================================================================
/**
 * A generic Element for retrieving a TreeFile-relative pathname.
 */

class PathElement: public Element
{
public:

	PathElement(const std::string &fileType, const std::string &fileExtension, const std::string &fileFilter, const std::string &defaultPathName);
	virtual ~PathElement();

	virtual std::string   getLabel() const;
	virtual unsigned int  getTypeIndex() const;

	virtual bool          hasLeftDoubleClickAction() const;
	virtual bool          doLeftDoubleClickAction(TextureBuilderDoc &document);

	const std::string    &getPathName() const;

private:

	// Disabled.
	PathElement();
	PathElement(const PathElement&);
	PathElement &operator =(const PathElement&);

private:

	std::string *m_fileType;
	std::string *m_fileExtension;
	std::string *m_fileFilter;
	std::string *m_pathName;

};

// ======================================================================

#endif
