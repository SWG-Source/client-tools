//////////////////////////////////////////////////////////////////////
//  SchematicXmlParser.cpp
//
//  Copyright 2006, Sony Online Entertainment, LLC 
//  All rights reserved by Sony Online Entertainment, LLC
//
//////////////////////////////////////////////////////////////////////
//! \file


#include "FirstSwgSchematicXmlParser.h" 

#include <fstream>

#include "SchematicXmlParser.h" 

void StringOut(std::ofstream&	stream,
			   TiXmlElement*	element,
			   const char*		key)
{
	TiXmlElement*	elemX = element->FirstChildElement(key);

	if(elemX)
	{
		stream << key << " = \"";
		
		if(elemX->GetText() == NULL)
		{
			stream << "\"";
		}
		else
		{
			stream << elemX->GetText() << "\"";
		}
	}
}

void StringIDOut(std::ofstream&		stream,
				 TiXmlElement*		element,
				 const char*		key)
{
	TiXmlElement*	elemX = element->FirstChildElement(key);

	if(elemX)
	{
		TiXmlElement*	elemName = elemX->FirstChildElement("tableName");
		TiXmlElement*	elemIndex = elemX->FirstChildElement("lookupIndex");

		if(elemName && elemIndex)
		{
			stream << key << " = ";
			if(elemName->GetText() == NULL)
			{
				stream << "\"\"";
			}
			else
			{
				stream << "\"" << elemName->GetText() << "\"";
			}
			
			stream << " ";
			if(elemIndex->GetText() == NULL)
			{
				stream << "\"\"";
			}
			else
			{
				stream << "\"" << elemIndex->GetText() << "\"" ;
			}
		}
	}
}

void ValueOut(std::ofstream&	stream,
			  TiXmlElement*		element,
			  const char*		key)
{
	TiXmlElement*	elemX = element->FirstChildElement(key);
	if(elemX && elemX->GetText() != NULL)
	{
		stream << key << " = " << elemX->GetText();
	}
}

void LineOutString(std::ofstream&	stream,
				   TiXmlElement*	element,
				   const char*		key)
{
	TiXmlElement*	elemX = element->FirstChildElement(key);

	if(elemX)
	{
		stream << key << " = \"" << (elemX->GetText() == NULL? "":elemX->GetText()) << "\"\n";
	}
}

void LineOutValue(std::ofstream&	stream,
				   TiXmlElement*	element,
				   const char*		key)
{
	TiXmlElement*	elemX = element->FirstChildElement(key);
	if(elemX && elemX->GetText() != NULL)
	{
		stream << key << " = " << elemX->GetText() << "\n";
	}
}

void LineOutStringID(std::ofstream&		stream,
					 TiXmlElement*		element,
					 const char*		key)
{
	TiXmlElement*	elemX = element->FirstChildElement(key);

	if(elemX)
	{
		TiXmlElement*	elemName = elemX->FirstChildElement("tableName");
		TiXmlElement*	elemIndex = elemX->FirstChildElement("lookupIndex");

		if(elemName && elemIndex)
		{
			stream << key << " = \"" << elemName->GetText() << "\" \"" << elemIndex->GetText() << "\"\n" ;
		}
	}
}

void SimpleListOutString(std::ofstream&		stream,
						 TiXmlElement*		element,
						 const char*		listKey,
						 const char*		childKey,
						 bool				append = false)
{
	TiXmlElement*	elemParent = element->FirstChildElement(listKey);

	if(elemParent)
	{
		if(append)
		{
			stream << listKey << " = +[";
		}
		else
		{
			stream << listKey << " = [";
		}
		
		TiXmlElement*	elem = elemParent->FirstChildElement(childKey);
		
		while(elem)
		{
			stream << "\"" << elem->GetText() << "\"";
			
			elem = elem->NextSiblingElement();
			if(elem)
			{
				stream << ", ";
			}
		}

		stream <<"]\n";
	}
}

void SimpleListOut(std::ofstream&	stream,
				   TiXmlElement*	element,
				   const char*		listKey,
				   const char*		childKey,
				   bool				append = false)
{
	TiXmlElement*	elemParent = element->FirstChildElement(listKey);

	if(elemParent)
	{
		if(append)
		{
			stream << listKey << " = +[";
		}
		else
		{
			stream << listKey << " = [";
		}

		TiXmlElement*	elem = elemParent->FirstChildElement(childKey);

		while(elem)
		{
			if(elem->GetText() != NULL)
			{
				stream << elem->GetText();
			}

			elem = elem->NextSiblingElement();
			if(elem)
			{
				stream << ", ";
			}
		}

		stream <<"]\n";
	}
}

SchematicXmlParser::SchematicXmlParser()
	: m_itemElement(NULL)
{

}

SchematicXmlParser::~SchematicXmlParser()
{

}

bool				
SchematicXmlParser::loadXML(const char*		documentPath)
{
	if(!m_tinyDocument.LoadFile(documentPath))
	{
		printf(" !! Failed !! LoadFile(0 returned false\n");

		return false;
	}

	// Get the root item key
	TiXmlHandle docHandle(&m_tinyDocument);

	m_itemElement = docHandle.FirstChild("Items").FirstChild("item").ToElement();

	if(!m_itemElement)
	{
		printf(" !! Failed !! item element not found\n");

		return false;
	}


	return true;
}

bool				
SchematicXmlParser::generateOutput(const char*	basePath)
{
	if(!m_itemElement)
	{
		return false;
	}

	m_fileBasePath = basePath;

	if(!outputServerTangible())
	{
		printf(" !! Failed !! outputServerTangible() returned false\n");
	}

	if(!outputSharedTangible())
	{
		printf(" !! Failed !! outputSharedTangible() returned false\n");
	}

	if(!outputServerSchematic())
	{
		printf(" !! Failed !! outputServerSchematic() returned false\n");
	}

	if(!outputSharedSchematic())
	{
		printf(" !! Failed !! outputSharedSchematic() returned false\n");
	}

	return true;
}


bool				
SchematicXmlParser::outputServerTangible()
{
	// Get the server element
	TiXmlHandle		item(m_itemElement);

	TiXmlElement*	server = item.FirstChild("tangibleObject").FirstChild("server").ToElement();

	if(!server)
	{
		return false;
	}
	
	// Get the output file
	std::string	filename = getOutputFileName("tangibleObject","server");

	if(filename.size() == 0)
	{
		return false;
	}

	std::ofstream	stream(filename.c_str());

	if(stream.bad())
	{
		return false;
	}
	
	// Start writing it out
	TiXmlElement*	baseIFF = server->FirstChildElement("baseIFFFilename");

	if(!baseIFF)
	{
		return false;
	}

	stream << "@base " << baseIFF->GetText() << "\n\n";

	// First the tangible object
	TiXmlElement*	elementTangible = server->FirstChildElement("tangibleObject");
	TiXmlElement*	elementBase = server->FirstChildElement("baseObject");

	if(!elementTangible || !elementBase)
	{
		return false;
	}

	parseServerTangible(stream,elementTangible);
	parseServerBase(stream,elementBase);

	stream.close();

	return true;
}

bool				
SchematicXmlParser::outputSharedTangible()
{
	// Get the server element
	TiXmlHandle		item(m_itemElement);

	TiXmlElement*	server = item.FirstChild("tangibleObject").FirstChild("shared").ToElement();

	if(!server)
	{
		return false;
	}

	// Get the file name
	std::string	filename = getOutputFileName("tangibleObject","shared");

	if(filename.size() == 0)
	{
		return false;
	}

	std::ofstream	stream(filename.c_str());

	if(stream.bad())
	{
		return false;
	}

	// Start writing it out
	TiXmlElement*	baseIFF = server->FirstChildElement("baseIFFFilename");

	if(!baseIFF)
	{
		return false;
	}

	stream << "@base " << baseIFF->GetText() << "\n\n";

	// First the tangible object
	TiXmlElement*	elementTangible = server->FirstChildElement("tangibleObject");
	TiXmlElement*	elementBase = server->FirstChildElement("baseObject");

	if(!elementTangible || !elementBase)
	{
		return false;
	}

	parseSharedTangible(stream,elementTangible);
	parseSharedBase(stream,elementBase);

	stream.close();

	return true;
}

bool				
SchematicXmlParser::outputServerSchematic()
{
	// Get the server element
	TiXmlHandle		item(m_itemElement);

	TiXmlElement*	server = item.FirstChild("draftSchematicObject").FirstChild("server").ToElement();

	if(!server)
	{
		return false;
	}

	// Get the file name
	std::string	filename = getOutputFileName("draftSchematicObject","server");

	if(filename.size() == 0)
	{
		return false;
	}

	std::ofstream	stream(filename.c_str());

	if(stream.bad())
	{
		return false;
	}

	// Start writing it out
	TiXmlElement*	baseIFF = server->FirstChildElement("baseIFFFilename");

	if(!baseIFF)
	{
		return false;
	}

	stream << "@base " << baseIFF->GetText() << "\n\n";

	// Get the component objects
	TiXmlElement*	elementSchematic = server->FirstChildElement("draftSchematicObject");
	TiXmlElement*	elementIntangible = server->FirstChildElement("intangibleObject");
	TiXmlElement*	elementBase = server->FirstChildElement("baseObject");

	if(!elementIntangible || !elementBase || !elementSchematic)
	{
		return false;
	}

	parseServerSchematic(stream,elementSchematic);
	parseServerIntangible(stream,elementIntangible);
	parseServerBase(stream,elementBase);

	stream.close();

	return true;
}

bool				
SchematicXmlParser::outputSharedSchematic()
{
	// Get the server element
	TiXmlHandle		item(m_itemElement);

	TiXmlElement*	server = item.FirstChild("draftSchematicObject").FirstChild("shared").ToElement();

	if(!server)
	{
		return false;
	}

	// Get the file name
	std::string	filename = getOutputFileName("draftSchematicObject","shared");

	if(filename.size() == 0)
	{
		return false;
	}

	std::ofstream	stream(filename.c_str());

	if(stream.bad())
	{
		return false;
	}

	// Start writing it out
	TiXmlElement*	baseIFF = server->FirstChildElement("baseIFFFilename");

	if(!baseIFF)
	{
		return false;
	}

	stream << "@base " << baseIFF->GetText() << "\n\n";

	// Get the component objects
	TiXmlElement*	elementSchematic = server->FirstChildElement("draftSchematicObject");
	TiXmlElement*	elementIntangible = server->FirstChildElement("intangibleObject");
	TiXmlElement*	elementBase = server->FirstChildElement("baseObject");

	if(!elementIntangible || !elementBase || !elementSchematic)
	{
		return false;
	}

	parseSharedSchematic(stream,elementSchematic);
	parseSharedIntangible(stream,elementIntangible);
	parseSharedBase(stream,elementBase);

	stream.close();

	return true;
}

std::string				
SchematicXmlParser::getOutputFileName(const char*		keyObj,
									  const char*		keyContext)
{
	// Get the server element
	TiXmlHandle		item(m_itemElement);

	TiXmlElement*	element = item.FirstChild(keyObj).FirstChild(keyContext).ToElement();

	if(!element)
	{
		return false;
	}

	// Get the file name
	TiXmlElement*	templateFile = element->FirstChildElement("templateFilename");

	if(!templateFile)
	{
		return false;
	}

	// Create the output file
	std::string		filename = m_fileBasePath;
	filename = filename + "/" + templateFile->GetText();

	return filename;
}

void				
SchematicXmlParser::parseServerTangible(std::ofstream&	stream,
										TiXmlElement*	elementTangible)
{
	stream << "@class tangible_object_template " << SERVER_TANGIBLE_VERSION << "\n\n";

	// Trigger volume
	SimpleListOutString(stream,elementTangible,"triggerVolumes","entry");

	// Combat skeleton
	LineOutValue(stream,elementTangible,"combatSkeleton");

	// Max Hit Points
	LineOutValue(stream,elementTangible,"maxHitPoints");

	// armor
	LineOutString(stream,elementTangible,"armor");

	// Interest Radius
	LineOutValue(stream,elementTangible,"interestRadius");

	// Count
	LineOutValue(stream,elementTangible,"count");

	// Condition
	LineOutValue(stream,elementTangible,"condition");

	// Want Saw Attack Triggers
	LineOutValue(stream,elementTangible,"wantSawAttackTriggers");
}

void				
SchematicXmlParser::parseServerBase(std::ofstream&	stream,
									TiXmlElement*	elementBase)
{
	stream << "@class object_template " << SERVER_BASEOBJECT_VERSION << "\n\n";

	// Shared Template
	LineOutString(stream,elementBase,"sharedTemplate");

	// Scripts
	SimpleListOutString(stream,elementBase,"scripts","script",true);

	// ObjVars
	SimpleListOut(stream,elementBase,"objvar","entry");

	// Volume
	LineOutValue(stream,elementBase,"volume");

	// Visible Flags
	SimpleListOut(stream,elementBase,"visibleFlags","entry");

	// Delete Flags
	SimpleListOut(stream,elementBase,"deleteFlags","entry");

	// Delete Flags
	SimpleListOut(stream,elementBase,"moveFlags","entry");

	// Invulnerable
	LineOutValue(stream,elementBase,"invulnerable");

	// Complexity
	LineOutValue(stream,elementBase,"complexity");

	// Tint Index
	LineOutValue(stream,elementBase,"tintIndex");

	// Update Ranges
	TiXmlElement*	elemUpdate = elementBase->FirstChildElement("updateRanges");

	if(elemUpdate)
	{
		TiXmlElement*	elem;

		elem = elemUpdate->FirstChildElement("UR_Near");

		if(elem)
		{
			stream << "updateRanges[UR_near] = " << elem->GetText() << "\n";
		}

		elem = elemUpdate->FirstChildElement("UR_normal");

		if(elem)
		{
			stream << "updateRanges[UR_normal] = " << elem->GetText() << "\n";
		}

		elem = elemUpdate->FirstChildElement("UR_far");

		if(elem)
		{
			stream << "updateRanges[UR_far] = " << elem->GetText() << "\n";
		}
	}

	// Contents
	TiXmlElement*	elemCtnts = elementBase->FirstChildElement("contents");

	if(elemCtnts)
	{
		stream << "contents = [";

		TiXmlElement*	elem = elemCtnts->FirstChildElement("entry");

		while(elem)
		{
			stream << "[";
			TiXmlElement*	elemKey = elem->FirstChildElement("slotName");
			if(elemKey)
			{
				stream << "slotName = \"" << elemKey->GetText() << "\n, ";
			}

			elemKey = elem->FirstChildElement("level");
			if(elemKey)
			{
				stream << "equipObject = " << elemKey->GetText() << ", ";
			}

			elemKey = elem->FirstChildElement("value");
			if(elemKey)
			{
				stream << "content = \"" << elemKey->GetText() << "\""; 
			}

			stream << "]";

			elem = elem->NextSiblingElement("entry");

			if(elem)
			{
				stream << ",\n";
			}
		}

		stream << "]\n";
	}

	// XP Points
	TiXmlElement*	elemXP = elementBase->FirstChildElement("xpPoints");

	if(elemXP)
	{
		stream << "xpPoints = [";

		TiXmlElement*	elem = elemXP->FirstChildElement("entry");

		while(elem)
		{
			stream << "[";
			TiXmlElement*	elemKey = elem->FirstChildElement("type");
			if(elemKey)
			{
				stream << "type = " << elemKey->GetText() << ", ";
			}

			elemKey = elem->FirstChildElement("level");
			if(elemKey)
			{
				stream << "level = " << elemKey->GetText() << ", ";
			}

			elemKey = elem->FirstChildElement("value");
			if(elemKey)
			{
				stream << "value = " << elemKey->GetText(); 
			}

			stream << "]";

			elem = elem->NextSiblingElement("entry");

			if(elem)
			{
				stream << ",\n";
			}
		}

		stream << "]\n";
	}

	// Persist by default
	LineOutValue(stream,elementBase,"persistByDefault");

	// Persist Contents
	LineOutValue(stream,elementBase,"persistContents");
}

void				
SchematicXmlParser::parseSharedIntangible(std::ofstream&	stream,
										  TiXmlElement*		elementIntangible)
{
	stream << "@class intangible_object_template " << SHARED_INTANGIBLE_VERSION<< "\n\n";
}

void				
SchematicXmlParser::parseServerIntangible(std::ofstream&	stream,
										  TiXmlElement*		elementIntangible)
{
	stream << "@class intangible_object_template " << SERVER_INTANGIBLE_VERSION<< "\n\n";
}

void				
SchematicXmlParser::parseSharedTangible(std::ofstream&	stream,
										TiXmlElement*	elementTangible)
{
	stream << "@class tangible_object_template " << SHARED_TANGIBLE_VERSION << "\n\n";


	// paletteColorCustomizationVariables
	TiXmlElement*	elem = elementTangible->FirstChildElement("paletteColorCustomizationVariables");
	if(elem)
	{
		stream << "paletteColorCustomizationVariables = [\n";

		TiXmlElement*	elemEntry = elem->FirstChildElement("entry");

		while(elemEntry)
		{
			stream << "    [";
			StringOut(stream,elemEntry,"variableName");
			stream << ", ";

			StringOut(stream,elemEntry,"palettePathName");
			stream << ", ";

			ValueOut(stream,elemEntry,"palettePathName");
			stream << " ]";

			elemEntry = elemEntry->NextSiblingElement("entry");

			if(elemEntry)
			{
				stream << ",\n";
			}
		}
		
		stream << "]\n";
	}

	// rangedIntCustomizationVariables
	elem = elementTangible->FirstChildElement("rangedIntCustomizationVariables");
	if(elem)
	{
		stream << "paletteColorCustomizationVariables = [\n";

		TiXmlElement*	elemEntry = elem->FirstChildElement("entry");

		while(elemEntry)
		{
			stream << "    [";
			StringOut(stream,elemEntry,"variableName");
			stream << ", ";

			ValueOut(stream,elemEntry,"minValueInclusive");
			stream << ", ";

			ValueOut(stream,elemEntry,"defaultValue");
			stream << ", ";

			ValueOut(stream,elemEntry,"maxValueExclusive");
			stream << " ]";

			elemEntry = elemEntry->NextSiblingElement("entry");

			if(elemEntry)
			{
				stream << ",\n";
			}
		}

		stream << "]\n";
	}

	// constStringCustomizationVariables
	elem = elementTangible->FirstChildElement("constStringCustomizationVariables");
	if(elem)
	{
		stream << "constStringCustomizationVariables = [\n";

		TiXmlElement*	elemEntry = elem->FirstChildElement("entry");

		while(elemEntry)
		{
			stream << "    [";
			StringOut(stream,elemEntry,"variableName");
			stream << ", ";

			StringOut(stream,elemEntry,"constValue");
			stream << " ]";

			elemEntry = elemEntry->NextSiblingElement("entry");

			if(elemEntry)
			{
				stream << ",\n";
			}
		}

		stream << "]\n";
	}

	// socketDestinations
	LineOutValue(stream,elementTangible,"socketDestinations");

	// structureFootprintFileName
	LineOutString(stream,elementTangible,"structureFootprintFileName");

	// useStructureFootprintOutline
	LineOutValue(stream,elementTangible,"useStructureFootprintOutline");

	// targetable
	LineOutValue(stream,elementTangible,"targetable");

	// certificationsRequired
	SimpleListOut(stream,elementTangible,"certificationsRequired","entry");
	
	// customizationVariableMapping
	elem = elementTangible->FirstChildElement("customizationVariableMapping");
	if(elem)
	{
		stream << "customizationVariableMapping = [\n";

		TiXmlElement*	elemEntry = elem->FirstChildElement("entry");

		while(elemEntry)
		{
			stream << "    [";
			ValueOut(stream,elemEntry,"sourceVariable");
			stream << ", ";

			ValueOut(stream,elemEntry,"dependentVariable");
			stream << " ]";

			elemEntry = elemEntry->NextSiblingElement("entry");

			if(elemEntry)
			{
				stream << ",\n";
			}
		}

		stream << "]\n";
	}

	// clientVisabilityFlag
	LineOutValue(stream,elementTangible,"targetable");
}

void				
SchematicXmlParser::parseSharedBase(std::ofstream&	stream,
									TiXmlElement*	elementBase)
{
	stream << "@class object_template " << SHARED_BASEOBJECT_VERSION << "\n\n";

	// objectName
	LineOutStringID(stream,elementBase,"objectName");

	// objectName
	LineOutStringID(stream,elementBase,"detailedDescription");

	// lookAtText
	LineOutStringID(stream,elementBase,"lookAtText");

	// snapToTerrain
	LineOutValue(stream,elementBase,"snapToTerrain");

	// containerType
	LineOutValue(stream,elementBase,"containerType");

	// containerVolumeLimit
	LineOutValue(stream,elementBase,"containerVolumeLimit");

	// tintPalette
	LineOutString(stream,elementBase,"tintPalette");

	// slotDescriptorFilename
	LineOutString(stream,elementBase,"slotDescriptorFilename");

	// arrangementDescriptorFilename
	LineOutString(stream,elementBase,"arrangementDescriptorFilename");

	// appearanceFilename
	LineOutString(stream,elementBase,"appearanceFilename");

	// portalLayoutFilename
	LineOutString(stream,elementBase,"portalLayoutFilename");

	// clientDataFile
	LineOutString(stream,elementBase,"clientDataFile");

	// scale
	LineOutValue(stream,elementBase,"scale");
	
	// gameObjectType
	LineOutValue(stream,elementBase,"gameObjectType");

	// sendToClient
	LineOutValue(stream,elementBase,"sendToClient");

	// scaleThresholdBeforeExtentTest
	LineOutValue(stream,elementBase,"scaleThresholdBeforeExtentTest");

	// clearFloraRadius
	LineOutValue(stream,elementBase,"clearFloraRadius");
	
	// surfaceType
	LineOutValue(stream,elementBase,"surfaceType");

	// noBuildRadius
	LineOutValue(stream,elementBase,"noBuildRadius");

	// onlyVisibleInTools
	LineOutValue(stream,elementBase,"onlyVisibleInTools");

	// locationReservationRadius
	LineOutValue(stream,elementBase,"locationReservationRadius");

	// forceNoCollision
	LineOutValue(stream,elementBase,"forceNoCollision");

}

void				
SchematicXmlParser::parseSharedSchematic(std::ofstream&	stream,
										 TiXmlElement*	elementSchematic)
{
	stream << "@class draft_schematic_object_template " << SHARED_SCHEMATIC_VERSION<< "\n\n";

	// slots
	TiXmlElement*	elem = elementSchematic->FirstChildElement("slots");
	if(elem)
	{
		stream << "slots = [\n";

		TiXmlElement*	elemEntry = elem->FirstChildElement("entry");

		while(elemEntry)
		{
			stream << "    [";
			
			StringIDOut(stream,elemEntry,"name");
			stream << ", ";

			StringOut(stream,elemEntry,"hardpoint");
			stream << " ]";

			elemEntry = elemEntry->NextSiblingElement("entry");

			if(elemEntry)
			{
				stream << ",\n";
			}
		}

		stream << "]\n";
	}

	// attributes
	elem = elementSchematic->FirstChildElement("attributes");
	if(elem)
	{
		stream << "attributes = [\n";

		TiXmlElement*	elemEntry = elem->FirstChildElement("entry");

		while(elemEntry)
		{
			stream << "    [";
			
			StringIDOut(stream,elemEntry,"name");
			stream << ", ";

			StringIDOut(stream,elemEntry,"experiment");
			stream << ", ";

			stream << "value = ";
			
			TiXmlElement*	elemMin = elemEntry->FirstChildElement("valueMin");
			TiXmlElement*	elemMax = elemEntry->FirstChildElement("valueMax");

			if(elemMin && elemMax)
			{
				stream << elemMin->GetText() << ".." << elemMax->GetText();
			}

			stream << " ]";

			elemEntry = elemEntry->NextSiblingElement("entry");

			if(elemEntry)
			{
				stream << ",\n";
			}
		}

		stream << "]\n\n";
	}

	// craftedSharedTemplate
	LineOutString(stream,elementSchematic,"craftedSharedTemplate");

	stream << "\n";
}

void				
SchematicXmlParser::parseServerSchematic(std::ofstream&	stream,
										 TiXmlElement*	elementSchematic)
{
	stream << "@class draft_schematic_object_template " << SERVER_SCHEMATIC_VERSION << "\n\n";

	// category
	LineOutValue(stream,elementSchematic,"category");

	// craftedObjectTemplate
	LineOutString(stream,elementSchematic,"craftedObjectTemplate");

	// crateObjectTemplate
	LineOutString(stream,elementSchematic,"crateObjectTemplate");

	// slots
	TiXmlElement*	elem = elementSchematic->FirstChildElement("slots");
	if(elem)
	{
		stream << "slots = [\n";

		TiXmlElement*	elemEntry = elem->FirstChildElement("entry");

		while(elemEntry)
		{
			//        "slots = [\n"
			stream << "    [\n      ";

			// optional
			ValueOut(stream,elemEntry,"optional");
			stream << ", ";

			// name
			StringIDOut(stream,elemEntry,"name");
			stream << ", ";

			// options
			TiXmlElement*	elemOpts = elemEntry->FirstChildElement("options");
			if(elemOpts)
			{
				stream << "\n      options = [\n";

				TiXmlElement*	elemNext = elemOpts->FirstChildElement("entry");

				while(elemNext)
				{
					stream << "        [\n          ";

					// ingredientType
					ValueOut(stream,elemNext,"ingredientType");
					stream << ", ";

					// ingredients
					// Right now I am only supporting one engredient per slot
					// that could be changed pretty easiliy
					TiXmlElement*	elemAgain = elemNext->FirstChildElement("ingredients");
					if(elemAgain)
					{
						stream << "\n          ingredients = [[";

						TiXmlElement*	elemIngredients = elemAgain->FirstChildElement("entry");

						if(elemIngredients)
						{
							// experiment
							StringIDOut(stream,elemIngredients,"name");
							stream << ", ";

							// ingredient
							StringOut(stream,elemIngredients,"ingredient");
							stream << ", ";

							// count
							ValueOut(stream,elemIngredients,"count");
							stream << "]],\n";
						}
					}

					stream << "          ";

					// complexity
					ValueOut(stream,elemNext,"complexity");
					stream << ", ";

					// skillCommand
					StringOut(stream,elemNext,"skillCommand");
					stream << "\n        ]\n";

					elemNext = elemNext->NextSiblingElement("entry");

					if(elemNext)
					{
						stream << ",\n      ";
					}
					else
					{
						stream << "      ";
					}
				}

				stream << "]\n      ";
			}

			// optionalSkillCommand
			StringOut(stream,elemEntry,"optionalSkillCommand");
			stream << ", ";
			
			// complexity
			ValueOut(stream,elemEntry,"complexity");
			stream << ", ";

			// appearance
			StringOut(stream,elemEntry,"appearance");
			stream << "\n    ]";

			elemEntry = elemEntry->NextSiblingElement("entry");

			if(elemEntry)
			{
				stream << ",\n";
			}
		}

		stream << "\n]\n";
	}

	// skillCommands
	SimpleListOutString(stream,elementSchematic,"skillCommands","entry");

	// destroyIngredients
	LineOutValue(stream,elementSchematic,"destroyIngredients");

	// manufactureScripts
	SimpleListOutString(stream,elementSchematic,"manufactureScripts","entry");

	// itemsPerContainer
	LineOutValue(stream,elementSchematic,"itemsPerContainer");

	// manufactureTime
	LineOutValue(stream,elementSchematic,"manufactureTime");

	// prototypeTime
	LineOutValue(stream,elementSchematic,"prototypeTime");
}

