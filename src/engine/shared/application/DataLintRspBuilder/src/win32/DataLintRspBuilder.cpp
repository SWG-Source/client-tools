//===================================================================
//
// DataLintRspBuilder.cpp
//
// Copyright Sony Online Entertainment
//
//===================================================================

#include "stdafx.h"

#pragma warning(disable: 4702)

#include <algorithm>
#include <cstdio>
#include <direct.h>
#include <map>
#include <vector>
#include <string>

//-----------------------------------------------------------------------------
namespace
{
	typedef std::multimap<CString, CString> StringMap;
	typedef std::map<CString, CString>      DataMap;
	typedef std::vector<CString>            StringList;
	typedef std::map<CString, StringList>   DuplicateMap;

	DuplicateMap ms_duplicateMap;
	DataMap      ms_rspMap;
	StringMap    ms_stringMap;

	bool         ms_server = false;
}

//-----------------------------------------------------------------------------
static void fixUpSlashes(char *path)
{
	char *walkPtr = &path[0];

	while (*walkPtr != '\0')
	{
		if (*walkPtr == '\\')
		{
			*walkPtr = '/';
		}

		++walkPtr;
	}
}

//-----------------------------------------------------------------------------
static bool parseConfigFile(char const *path)
{
	bool badFile = false;

	if (path == NULL || strlen(path) <= 0)
	{
		badFile = true;
	}
	else
	{
		FILE *fp = fopen(path, "rt");

		if (fp)
		{
			fclose(fp);
		}
		else
		{
			badFile = true;
		}
	}

	if (badFile)
	{
		printf("ABORTING: Invalid configuration file specified: %s\n", path);
		return false;
	}

	//-- open the config file

	CStdioFile infile(path, CFile::modeRead | CFile::typeText);

	//-- read each line...

	CString line;

	while (infile.ReadString(line))
	{
		//-- see if the line is empty

		line.TrimLeft();
		line.TrimRight();

		if (line.GetLength() == 0)
		{
			continue;
		}

		//-- see if the first character is a comment

		int index = line.Find("#");

		if (index == 0)
		{
			continue;
		}

		//-- see if the first character is a comment

		index = line.Find(";");

		if (index == 0)
		{
			continue;
		}

		//-- find the =

		index = line.Find("=");

		if (index == -1)
		{
			continue;
		}

		//-- left half goes in key, right half goes in value

		const int     length = line.GetLength();
		const CString left   = line.Left(index);
		const CString right  = line.Right(length - index - 1);

		//-- add to list

		index = left.Find("searchPath");

		if (index == -1)
		{
			continue;
		}

		StringMap::iterator i = ms_stringMap.begin();
		for ( ; i != ms_stringMap.end(); ++i)
		{
			if (i->second == right)
			{
				break;
			}
		}

		if (i == ms_stringMap.end())
		{
			ms_stringMap.insert(std::make_pair(left, right));
		}
	}

	return true;
}

//-----------------------------------------------------------------------------
static bool isIff(char const *path)
{
	bool result = false;
	FILE *fp = fopen(path, "r");

	if (fp)
	{
		if ((getc(fp) == 'F') &&
			(getc(fp) == 'O') &&
			(getc(fp) == 'R') &&
			(getc(fp) == 'M'))
		{
			result = true;
		}

		fclose(fp);
	}

	return result;
}

//-----------------------------------------------------------------------------
static void generateFile(const CString& explicitDirectory, const CString& entryDirectory)
{
	const int explicitLength = explicitDirectory.GetLength();

	if (explicitLength <= 0)
	{
		return;
	}

	const int entryLength = entryDirectory.GetLength();

	CString searchMask;
	searchMask.Format("%s%s*.*", explicitDirectory, (explicitLength > 1 && explicitDirectory[explicitLength-1] != '/') ? "/" : "");

	CFileFind finder;

	BOOL working = finder.FindFile(searchMask);

	while (working)
	{
		working = finder.FindNextFile();

		if (!finder.IsDots())
		{
			CString explicitName;

			explicitName.Format("%s%s%s", explicitDirectory, (explicitLength > 1 && explicitDirectory[explicitLength-1] != '/') ? "/" : "", finder.GetFileName());

			CString entryName;

			entryName.Format("%s%s%s", entryDirectory, (entryLength > 1 && entryDirectory[entryLength-1] != '/' ? "/" : ""), finder.GetFileName());

			if (finder.IsDirectory())
			{
				char const *text = static_cast<char const *>(entryName);
				bool recurse = false;

				if (strstr(text, "appearance") != NULL)
				{
					if ((strchr(text, '/') == NULL) && (strchr(text, '\\') == NULL))
					{
						recurse = true;
					}
				}
				else if ((strstr(text, "object") != NULL) && (strstr(text, "structure_footprint") == NULL))
				{
					recurse = true;
				}
				else
				{
					recurse = true;
				}

				if (recurse)
				{
					generateFile(explicitName, entryName);
				}
			}
			else
			{
				char entryNameFixed[4096];
				char explicitNameFixed[4096];
				strncpy(entryNameFixed, static_cast<char const *>(entryName), sizeof(entryNameFixed));
				strncpy(explicitNameFixed, static_cast<char const *>(explicitName), sizeof(explicitNameFixed));
				fixUpSlashes(entryNameFixed);
				fixUpSlashes(explicitNameFixed);

				DataMap::iterator dataSetIter = ms_rspMap.find(static_cast<CString>(entryNameFixed));

				if (dataSetIter == ms_rspMap.end())
				{
					ms_rspMap[static_cast<CString>(entryNameFixed)] = static_cast<CString>(explicitNameFixed);
				}
				else
				{
					// Find where this entry exists in the duplicate map

					DuplicateMap::iterator duplicateMapIter = ms_duplicateMap.find(static_cast<CString>(entryNameFixed));

					if (duplicateMapIter == ms_duplicateMap.end())
					{
						// Since the entry does not exist in the duplicate map, add it

						ms_duplicateMap.insert(std::make_pair(static_cast<CString>(entryNameFixed), StringList()));

						// Find the item in the duplicate map now that we have just added it

						duplicateMapIter = ms_duplicateMap.find(static_cast<CString>(entryNameFixed));

						StringList &stringList = duplicateMapIter->second;
						stringList.push_back(dataSetIter->second);
					}

					// Now that the item is in the duplicate map, make sure the explicitName is assigned to it

					StringList &stringList = duplicateMapIter->second;

					StringList::iterator stringListIter = std::find(stringList.begin(), stringList.end(), static_cast<CString>(explicitNameFixed));
					
					if (stringListIter == stringList.end())
					{
						// Since the explicit name is not assigned to this duplicate, add it to the list

						stringList.push_back(static_cast<CString>(explicitNameFixed));
					}
				}
			}
		}
	}
}

//-----------------------------------------------------------------------------
static void writeRsp(std::string const &destPath)
{
	//-- open the config file

	mkdir(destPath.c_str());

	// Set the output filenames depending if this is client or server output

	std::string dataLintRsp(ms_server ? "DataLintServer.rsp" : "DataLint.rsp");
	std::string dataLintRemovedFromRsp(ms_server ? "DataLintServer_RemovedFromRsp.txt" : "DataLint_RemovedFromRsp.txt");
	std::string dataLintDuplicateAssets(ms_server ? "DataLintServer_DuplicateAssets.txt" : "DataLint_DuplicateAssets.txt");
	std::string dataLintTestAssets(ms_server ? "DataLintServer_TestAssets.txt" : "DataLint_TestAssets.txt");

	// Set the output paths

	std::string supportedAssetsRspPath(destPath + (!destPath.empty() ? "/" : "") + dataLintRsp);
	std::string unSupportedAssetsRspPath(destPath + (!destPath.empty() ? "/" : "") + dataLintRemovedFromRsp);
	std::string duplicateAssetsRspPath(destPath + (!destPath.empty() ? "/" : "") + dataLintDuplicateAssets);
	std::string testAssetsRspPath(destPath + (!destPath.empty() ? "/" : "") + dataLintTestAssets);

	//-- write each line...

	int const size = static_cast<int>(ms_rspMap.size());
	int current = 0;
	int accumulated = 0;
	int displayInterval = size / 100;
	StringList testAssetsStringList;
	testAssetsStringList.reserve(512);

	StringList supportedAssetsStringList;
	supportedAssetsStringList.reserve(16384);

	StringList unSupportedAssetsStringList;
	unSupportedAssetsStringList.reserve(4096);

	printf("Writing assets: (%5d/%5d)", 0, size);

	for (DataMap::iterator dataSetIter = ms_rspMap.begin(); dataSetIter != ms_rspMap.end(); ++dataSetIter)
	{
		char const *first = static_cast<char const *>(dataSetIter->first);
		char const *second = static_cast<char const *>(dataSetIter->second);

		bool const isFileIff = isIff(second);
		bool const isFileDDS = (strstr(first, ".dds") != NULL);
		bool const isFileSTF = (strstr(first, ".stf") != NULL);
		bool const isATestAsset = (strstr(first, "test") != NULL);
		bool const isFileLAY = (strstr(first, ".lay") != NULL);

		char text[4096];
		sprintf(text, "%s @ %s\n", first, second);

		if ((isFileIff || isFileDDS || isFileSTF) &&
			!isFileLAY)
		{
			supportedAssetsStringList.push_back(text);
		}
		else
		{
			CString line;
			line.Format("%4d %s", unSupportedAssetsStringList.size() + 1, text);
			unSupportedAssetsStringList.push_back(line);
		}

		if (isATestAsset)
		{
			testAssetsStringList.push_back(text);
		}

		++current;

		if (current > displayInterval)
		{
			accumulated += current;
			current = 0;
			printf("\b\b\b\b\b\b\b\b\b\b\b\b\b(%5d/%5d)", accumulated, size);
		}
	}

	CStdioFile supportedAssetsOutFile(supportedAssetsRspPath.c_str(), CFile::modeWrite | CFile::modeCreate | CFile::typeText);
	CStdioFile unSupportedAssetsOutFile(unSupportedAssetsRspPath.c_str(), CFile::modeWrite | CFile::modeCreate | CFile::typeText);
	CStdioFile duplicateAssetsOutFile(duplicateAssetsRspPath.c_str(), CFile::modeWrite | CFile::modeCreate | CFile::typeText);
	CStdioFile testAssetsOutFile(testAssetsRspPath.c_str(), CFile::modeWrite | CFile::modeCreate | CFile::typeText);

	unSupportedAssetsOutFile.WriteString("This file contains data that there is no current method known how to lint. Linting data\n"
	                                     "means that it is attempted to be created and destroyed. Any errors that occur in this process \n"
	                                     "are logged. If you know what some of the assets are listed in this file and would like them to be \n"
	                                     "linted, then we need to add support for it in DataLintRspBuilder.cpp and the DataLint itself \n"
	                                     "inside the client or server game build.\n"
	                                     "\n"
	                                     "\n");

	// Supported assets

	CString validRspString;
	validRspString.Format("Valid DataLint Rsp (%d) files\n", supportedAssetsStringList.size());
	supportedAssetsOutFile.WriteString(validRspString);

	StringList::iterator supportedAssetsStringListIter = supportedAssetsStringList.begin();

	for (; supportedAssetsStringListIter != supportedAssetsStringList.end(); ++supportedAssetsStringListIter)
	{
		supportedAssetsOutFile.WriteString(*supportedAssetsStringListIter);
	}

	// UnSupported Assets

	StringList::iterator unSupportedAssetsStringListIter = unSupportedAssetsStringList.begin();

	CString line;
	line.Format("UnSupported Asset Count (%d)\n\n", unSupportedAssetsStringList.size());
	unSupportedAssetsOutFile.WriteString(line);

	for (; unSupportedAssetsStringListIter != unSupportedAssetsStringList.end(); ++unSupportedAssetsStringListIter)
	{
		unSupportedAssetsOutFile.WriteString(*unSupportedAssetsStringListIter);
	}

	// Test Assets

	line.Format("Test Assets (%d) that should not be part of the game, but we DataLint them anyways if possible.\n", testAssetsStringList.size());
	testAssetsOutFile.WriteString(line);
	testAssetsOutFile.WriteString("\n");

	StringList::iterator testAssetsStringListIter = testAssetsStringList.begin();
	current = 0;

	for (; testAssetsStringListIter != testAssetsStringList.end(); ++testAssetsStringListIter)
	{
		CString line;
		line.Format("%4d %s", current, *testAssetsStringListIter);

		testAssetsOutFile.WriteString(line);
		++current;
	}

	printf("\b\b\b\b\b\b\b\b\b\b\b\b\bDONE               \n");

	// Write the duplicate file

	int duplicateCount = 1;

	DuplicateMap::const_iterator duplicateMapIter = ms_duplicateMap.begin();

	line.Format("Duplicate Asset Count (%d)\n\n", ms_duplicateMap.size());
	duplicateAssetsOutFile.WriteString(line);

	for (; duplicateMapIter != ms_duplicateMap.end(); ++duplicateMapIter)
	{
		CString const &first = duplicateMapIter->first;
		StringList const &second = duplicateMapIter->second;

		CString line;
		line.Format("%3d Duplicate Asset: %s\n", duplicateCount, first);
		duplicateAssetsOutFile.WriteString(line);

		StringList::const_iterator stringListIter = second.begin();
		int instanceDuplicateCount = 1;

		for (; stringListIter != second.end(); ++stringListIter)
		{
			CString line;
			line.Format("    %2d Referenced: %s\n", instanceDuplicateCount, (*stringListIter));
			duplicateAssetsOutFile.WriteString(line);
			++instanceDuplicateCount;
		}

		++duplicateCount;
		duplicateAssetsOutFile.WriteString("\n");
	}

	printf("Number of files written to %s: %d\n", static_cast<char const *>(supportedAssetsOutFile.GetFileName()), supportedAssetsStringList.size());
	printf("Number of files written to %s: %d\n", static_cast<char const *>(unSupportedAssetsOutFile.GetFileName()), unSupportedAssetsStringList.size());
	printf("Number of files written to %s: %d\n", static_cast<char const *>(duplicateAssetsOutFile.GetFileName()), duplicateCount);
	printf("Number of files written to %s: %d\n", static_cast<char const *>(testAssetsOutFile.GetFileName()), testAssetsStringList.size());
}

//-----------------------------------------------------------------------------
void main(int argc, char * argv[])
{
	printf("Build %s %s\n", __DATE__, __TIME__);

	std::string configFile;
	std::string destPath;
	bool invalidArgument = false;

	if (argc > 1)
	{
		for (int i = 1; i < argc; ++i)
		{
			char const *walkPtr = argv[i];

			if ((walkPtr != NULL) && (*walkPtr == '-'))
			{
				++walkPtr;

				if (*walkPtr == 's')
				{
					ms_server = true;
				}
				else if (*walkPtr == 'o')
				{
					++walkPtr;
					destPath = walkPtr;
				}
				else if (*walkPtr == 'c')
				{
					++walkPtr;
					configFile = walkPtr;
				}
			}
			else
			{
				printf("Invalid argument: %s\n", argv[i]);
				invalidArgument = true;
			}
		}
	}

	if (!invalidArgument)
	{
		if (configFile.empty())
		{
			if (ms_server)
			{
				configFile = "../../exe/shared/servercommon.cfg";
			}
			else
			{
				configFile = "../../exe/win32/common.cfg";
			}
		}

		if (parseConfigFile(configFile.c_str()))
		{
			// Traverse all the search paths and gather files

			for (StringMap::reverse_iterator i = ms_stringMap.rbegin(); i != ms_stringMap.rend(); ++i)
			{
				printf("%s -> %s\n", i->first, i->second);
				generateFile(i->second, "");
			}

			// Show the config file used and the output directory
			if (argc < 2)
			{
				printf("Configuration file not specified, attempting to use \"%s\".\n", configFile.c_str());
			}
			else
			{
				printf("Configuration file specified: %s\n", configFile.c_str());
			}

			if (!destPath.empty())
			{
				printf("Output directory specified:   %s\n", destPath.c_str());
			}

			// Write the output files

			writeRsp(destPath);
		}
	}
}

//===================================================================

enum MemoryManagerNotALeak
{
	MM_notALeak
};

void *operator new(size_t size, MemoryManagerNotALeak)
{
	return operator new(size);
}
