// ======================================================================
//
// TreeFileExtractor.cpp
// Copyright 2002 Sony Online Entertainment Inc
//
// ======================================================================

#include "FirstTreeFileExtractor.h"
#include "TreeFileExtractor.h"

#include "fileInterface/AbstractFile.h"
#include "sharedFoundation/Os.h"
#include "sharedFoundation/SetupSharedFoundation.h"
#include "sharedFoundation/Tag.h"
#include "sharedThread/SetupSharedThread.h"
#include "sharedCompression/SetupSharedCompression.h"
#include "sharedDebug/SetupSharedDebug.h"
#include "sharedIoWin/SetupSharedIoWin.h"
#include "sharedFile/FileStreamer.h"
#include "sharedFile/SetupSharedFile.h"
#include "sharedFile/TreeFile.h"
#include "sharedFile/TreeFile_SearchNode.h"

#include <cstdio>
#include <string>

// ======================================================================

namespace TreeFileExtractorNamespace
{
	//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

	static void usage();
	static std::string getFileName(const std::string& filePath)
	{
		std::string::size_type directoryEndPosition = filePath.find_last_of("\\/");
		if (static_cast<int>(directoryEndPosition) == static_cast<int>(std::string::npos))
			return filePath;

		return std::string(filePath, directoryEndPosition + 1);
	}

	static std::string getDirectoryName(const std::string& filePath)
	{
		std::string::size_type directoryEndPosition = filePath.find_last_of("\\/");
		if (static_cast<int>(directoryEndPosition) == static_cast<int>(std::string::npos))
			return std::string("");

		return filePath.substr(0, directoryEndPosition);
	}

	//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

	static char** ms_argv   = 0;
	static int    ms_argc   = 0;
	static int    ms_errors = 0;

	//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
};

using namespace TreeFileExtractorNamespace;

// ======================================================================

int main(int argc, char **argv)
{
	ms_argc = argc;
	ms_argv = argv;

	//-- thread
	SetupSharedThread::install();

	//-- debug
	SetupSharedDebug::install(4096);

	{
		SetupSharedFoundation::Data data(SetupSharedFoundation::Data::D_console);
		data.argc  = argc;
		data.argv  = argv;
		SetupSharedFoundation::install(data);
	}

	SetupSharedCompression::install();

	//-- file
	SetupSharedFile::install(false);

	//-- iowin
	SetupSharedIoWin::install();

	SetupSharedFoundation::callbackWithExceptionHandling(TreeFileExtractor::run);
	SetupSharedFoundation::remove();
	SetupSharedThread::remove();

	return -ms_errors;
}

// ----------------------------------------------------------------------

void TreeFileExtractor::run()
{
	//-- verify arguments
	if (ms_argc < 3)
	{
		usage();
		++ms_errors;
		return;
	}

	bool listContents = false;
	bool extract = false;
	if (strcmp(ms_argv[1], "-l") == 0)
		listContents = true;
	else
		if (strcmp(ms_argv[1], "-e") == 0)
			extract = true;

	if (!listContents && !extract)
	{
		usage();
		++ms_errors;
		return;
	}

	//-- make sure the file exists
	const char* const treeFileName = ms_argv[2];
	if (!FileStreamer::exists(treeFileName))
	{
		printf("error: %s does not exist\n", treeFileName);
		++ms_errors;
		return;
	}

	//-- make sure we can create the directory
	char const * const extractDirectoryName = ms_argv[3];
	if (extract)
	{
		if (!Os::createDirectories(extractDirectoryName))
		{
			printf("error: could not create directory %s\n", extractDirectoryName);
			++ms_errors;
			return;
		}
	}

	//-- add the tree file
	TreeFile::removeAllSearches();
	TreeFile::addSearchTree(treeFileName, 0);

	//-- make sure the tree file is valid
	if (TreeFile::ms_searchNodes.empty())
	{
		printf("error: %s is not a valid tree file\n", treeFileName);
		++ms_errors;
		return;
	}

	//-- grab the tree
	const TreeFile::SearchTree* const searchTree = dynamic_cast<const TreeFile::SearchTree*>(TreeFile::ms_searchNodes[0]);

	if (!searchTree)
	{
		printf("error: %s is not a valid tree file\n", treeFileName);
		++ms_errors;
		return;
	}

	if (extract)
	{
		//-- iterate through number of files
		const char* relativeFileName = searchTree->m_fileNames;

		const int numberOfFiles = searchTree->m_numberOfFiles;
		int i;
		for(i = 0; i < numberOfFiles; ++i)
		{
			printf("[%4i/%4i] file: %s", i, numberOfFiles, relativeFileName);

			std::string fileName  = getFileName(relativeFileName);
			std::string directory = getDirectoryName(relativeFileName);
			std::string diskDirectoryName = std::string(extractDirectoryName) + std::string("\\") + directory;
			std::string extractedFileName = diskDirectoryName + std::string("\\") + fileName;

			AbstractFile* const abstractFile = TreeFile::open(relativeFileName, AbstractFile::PriorityData, true);
			if (!abstractFile)
			{
				if (unlink(extractedFileName.c_str()) != 0)
				{
					printf("\nerror: could not delete file %s\n", extractedFileName.c_str());
					++ms_errors;
					// return;
				}
				else
					printf(" deleted\n");
			}
			else
			{
				if (!Os::createDirectories(diskDirectoryName.c_str()))
				{
					printf("\nerror: directory %s could not be created\n", diskDirectoryName.c_str());
					++ms_errors;
					return;
				}

				//-- create decompress file
				const int fileLength = abstractFile->length();
				byte* const buffer = abstractFile->readEntireFileAndClose();
				FILE* const outfile = fopen(extractedFileName.c_str(), "wb");

				if (!outfile)
				{
					printf("\nerror: file %s could not be created\n", extractedFileName.c_str());
					++ms_errors;
					return;
				}

				fwrite(buffer, fileLength, 1, outfile);
				fclose(outfile);

				printf(" created\n");

				delete [] buffer;
				delete abstractFile;
			}

			//-- skip to next filename
			while(*relativeFileName++)
				;
		}
	}
	else
		if (listContents)
		{
			int const numberOfFiles = searchTree->m_numberOfFiles;
			for(int i = 0; i < numberOfFiles; ++i)
			{
				TreeFile::SearchTree::TableOfContentsEntry const & entry = searchTree->m_tableOfContents[i];
				printf("%s\t%i\n", searchTree->m_fileNames + entry.fileNameOffset, entry.offset);
			}
		}
		else
		{
			printf("error: unknown error\n");
			++ms_errors;
			return;
		}
}

// ----------------------------------------------------------------------

void TreeFileExtractorNamespace::usage()
{
	printf("TreeFileExtractor " __DATE__ " " __TIME__ "\n");
	printf("TreeFileExtractor <-option> <treefile> [extract directory]\n");
	printf("  -e(xtract) - Extracts contents to extract directory\n");
	printf("  -l(ist) - List contents of treefile\n");
}

// ======================================================================
