//////////////////////////////////////////////////////////////////////
//  SchematicXmlParser.h
//
//  Copyright 2006, Sony Online Entertainment, LLC 
//  All rights reserved by Sony Online Entertainment, LLC
//
//  NOTES:
//      
//
//////////////////////////////////////////////////////////////////////
//! \file


#ifndef INCLUDED_SchematicXmlParser_H
#define INCLUDED_SchematicXmlParser_H

#include <string>

#pragma warning(disable:4100)
#include <tinyxml.h>

#define		SERVER_TANGIBLE_VERSION		"4"
#define		SERVER_BASEOBJECT_VERSION	"11"
#define		SERVER_SCHEMATIC_VERSION	"7"
#define		SERVER_INTANGIBLE_VERSION	"1"

#define		SHARED_TANGIBLE_VERSION		"10"
#define		SHARED_BASEOBJECT_VERSION	"10"
#define		SHARED_SCHEMATIC_VERSION	"3"
#define		SHARED_INTANGIBLE_VERSION	"0"

class SchematicXmlParser
{
	// Members
public:

protected:

	TiXmlDocument		m_tinyDocument;
	TiXmlElement*		m_itemElement;

	std::string			m_fileBasePath;

private:

	// Functions

public:
						SchematicXmlParser();
						~SchematicXmlParser();

	bool				loadXML(const char*		documentPath);

	bool				generateOutput(const char*	basePath);

protected:

	std::string			getOutputFileName(const char*		keyObj,
										  const char*		keyContext);

	bool				outputServerTangible();
	bool				outputSharedTangible();
	bool				outputServerSchematic();
	bool				outputSharedSchematic();



	void				parseSharedBase(std::ofstream&	stream,
										TiXmlElement*	elementBase);

	void				parseServerBase(std::ofstream&	stream,
										TiXmlElement*	elementBase);

	void				parseSharedTangible(std::ofstream&	stream,
											TiXmlElement*	elementTangible);

	void				parseServerTangible(std::ofstream&	stream,
											TiXmlElement*	elementTangible);


	void				parseSharedIntangible(std::ofstream&	stream,
											  TiXmlElement*		elementIntangible);

	void				parseServerIntangible(std::ofstream&	stream,
											  TiXmlElement*		elementIntangible);

	void				parseSharedSchematic(std::ofstream&	stream,
											 TiXmlElement*	elementSchematic);

	void				parseServerSchematic(std::ofstream&	stream,
											 TiXmlElement*	elementSchematic);
private:

};

#endif // INCLUDED_SchematicXmlParser_H
