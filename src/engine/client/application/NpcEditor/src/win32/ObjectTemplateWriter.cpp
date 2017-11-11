// ======================================================================
//
// ObjectTemplateWriter.cpp
// Copyright 2004 Sony Online Entertainment, Inc.
// All Rights Reserved.
//
// ======================================================================

#include "FirstNpcEditor.h"
#include "ObjectTemplateWriter.h"

// ----------------------------------------------------------------------------

#include <algorithm>
#include <map>
#include <stdio.h>
#include <string>

// ----------------------------------------------------------------------------

namespace ObjectTemplateWriterNamespace
{
	size_t const c_maxLineLength = 1024;
	char const * c_tokenDelimiter = "%";
}

using namespace ObjectTemplateWriterNamespace;


// ----------------------------------------------------------------------------

bool ObjectTemplateWriter::write(char const *outputFileName, char const *templateFileName, ObjectTemplateParameterMap const *parameterMap)
{
	char line[c_maxLineLength];

	FILE *templateFile = fopen(templateFileName, "r");
	if(!templateFile)
	{
		DEBUG_WARNING(true, ("Failed to open file [%s] for reading.", templateFile));
		return false;
	}

	FILE *outputFile = fopen(outputFileName, "w");
	if(!outputFile)
	{
		DEBUG_WARNING(true, ("Failed to open file [%s] for reading.", outputFile));
		fclose(templateFile);
		return false;
	}

	//-- Read the template file and replace keys from the parameter map with the values
	while(!feof(templateFile) && fgets(line, c_maxLineLength, templateFile))
	{
		bool skip = false;

		std::string outputLine(line);

		size_t start = outputLine.find(c_tokenDelimiter);

		while(start != static_cast<size_t>(std::string::npos))
		{
			size_t end = outputLine.find(c_tokenDelimiter, start + 1);

			if(end != static_cast<size_t>(std::string::npos))
			{
				std::string key = outputLine.substr(start + 1, end - start - 1); //lint !e834

				DEBUG_REPORT_LOG(false, ("ObjectTemplateWriter::write() - start = %d, end = %d, key = [%s]\n", start, end, key.c_str()));

				ObjectTemplateParameterMap::const_iterator i = parameterMap->find(key);

				if(i != parameterMap->end() && !i->second.empty())
					outputLine = outputLine.replace(start, end - start + 1, i->second); //lint !e834
				else
				{
					//-- Skip lines with unknown tokens or empty values
					skip = true;
					//-- Done processing this line
					break;
				}
			}
			else
				//-- Done processing this line
				break;

			start = outputLine.find(c_tokenDelimiter);
		}

		if(!skip)
			fputs(outputLine.c_str(), outputFile);
	}

	fclose(outputFile);
	fclose(templateFile);

	//-- Success.
	return true;
}

// ======================================================================
