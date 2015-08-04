// SWGSchematicXmlParser.cpp : Defines the entry point for the console application.
//

#include "FirstSwgSchematicXmlParser.h"

#include "SchematicXmlParser.h"

int main(int argc, char* argv[])
{
	/*
		Syntax: SWGSchematicXmlParser [XML File] [Path Base]
	*/

	if(argc != 3)
	{
		printf(" !! Failed !! No XML file specified\n");
		printf("Syntax: SWGSchematicXmlParser [XML File] [Path Base]\n");
		return 1;
	}

	// Parse the XML File
	SchematicXmlParser	parser;

	if(!parser.loadXML(argv[1]))
	{

		printf(" !! Failed !! loadXML() returned false\n");
		return 1;
	}

	if(!parser.generateOutput(argv[2]))
	{
		printf(" !! Failed !! generateOutput() returned false\n");
	}

	printf("Success: File %s Successfully parsed\n",argv[1]);

	return 0;
}

