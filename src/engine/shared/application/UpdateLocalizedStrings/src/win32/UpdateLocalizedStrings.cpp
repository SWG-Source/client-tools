// ======================================================================
//
// UpdateLocalizedStrings.cpp
// rsitton
//
// copyright 2004, sony online entertainment
//
// ======================================================================

#include "FirstUpdateLocalizedStrings.h"

#include "fileInterface/StdioFile.h"
#include "sharedCompression/SetupSharedCompression.h"
#include "sharedDebug/SetupSharedDebug.h"
#include "sharedFile/SetupSharedFile.h"
#include "sharedFile/TreeFile.h"
#include "sharedFoundation/SetupSharedFoundation.h"
#include "sharedThread/SetupSharedThread.h"

#include "LocalizedString.h"
#include "LocalizedStringTableReaderWriter.h"
#include "UnicodeUtils.h"

// ======================================================================
// Example batch processing
//
// Display version:
// for /r \swg\current\data\sku.0\sys.shared\built\game\string\ %%f in (*.stf) do UpdateLocalizedStrings.exe version %%f
//
// Update version:
// for /r %1\data\sku.0\sys.shared\built\game\string\ %%f in (*.stf) do UpdateLocalizedStrings.exe update %%f

// ======================================================================

namespace UpdateLocalizedStringsNamespace
{
	void printUsage(int argc, char const * const argv[]);
	bool updateCrcs(char const * const baseFileEnglish, char const * const currentFileEnglish, char const * const currentFileTranslated);
	bool displayAndUpdateStringFileVersion(char const * const baseFileEnglish, bool const update);

	int const s_latestVersion = 1;
}

using namespace UpdateLocalizedStringsNamespace;

// ======================================================================

void UpdateLocalizedStringsNamespace::printUsage(int const argc, char const * const argv[])
{
	printf("UpdateLocalizedStrings is used to update timestamp string files (version 0) to CRC string files (version 1).\n");
	printf ("\t\t[Build Info " __DATE__ " " __TIME__ "]\n");

	printf("\nDisplay the target filename's version.\n");
	printf("\tUpdateLocalizedStrings version filename.stf\n");

	printf("\nDisplay the target filename's version (and update).\n");
	printf("\tUpdateLocalizedStrings update filename.stf\n");

	printf("\nUpdate timestamp string files (version 0) to CRC string files (version 1) from a know version.\n");
	printf("\tUpdateLocalizedStrings baseline current_english.stf current_translated.stf baseline_english.stf\n");

	printf("\n\nCommand line: ");
	for (int i = 0; i < argc; ++i)
		printf("%s ", argv[i]);
	printf("\n");
}

// ----------------------------------------------------------------------

bool UpdateLocalizedStringsNamespace::updateCrcs(char const * const baseFileEnglish, char const * const currentFileEnglish, char const * const currentFileTranslated)
{
	//////////////////////////////////////////////////////////////////////
	
	DEBUG_REPORT_LOG(true, ("Processing %s\n", currentFileEnglish));
	StdioFileFactory fileFactoryCurrent;
	LocalizedStringTableRW * const currentStringFile = LocalizedStringTableRW::loadRW(fileFactoryCurrent, std::string(currentFileEnglish));
	if (!currentStringFile) 
	{
		DEBUG_WARNING(true, ("Unable to open the current string file %s", currentFileEnglish));
		return false;
	}
	
	//-- Since the source file may change when loaded, go ahead and save it out anyway.  Let Perforce iron out the real changes
	if (currentStringFile->getVersion() < s_latestVersion)
	{
		StdioFileFactory fileFactoryCurrent;
		if (!currentStringFile->writeRW(fileFactoryCurrent, std::string(currentFileEnglish)))
			DEBUG_WARNING(true, ("Could not write %s", currentFileEnglish));
	}


	//////////////////////////////////////////////////////////////////////
	
	DEBUG_REPORT_LOG(true, ("Processing %s\n", currentFileTranslated));
	StdioFileFactory fileFactoryTranslated;
	LocalizedStringTableRW * const translatedStringFile = LocalizedStringTableRW::loadRW(fileFactoryTranslated, std::string(currentFileTranslated));
	if (!translatedStringFile) 
	{
		DEBUG_WARNING(true, ("Unable to open the translated string file %s", currentFileTranslated));
		return false;
	}

	//////////////////////////////////////////////////////////////////////

	DEBUG_REPORT_LOG(true, ("Processing %s\n", baseFileEnglish));
	StdioFileFactory fileFactoryBase;
	LocalizedStringTableRW * const baseStringFile = LocalizedStringTableRW::loadRW(fileFactoryBase, std::string(baseFileEnglish));
	if (baseStringFile) 
	{
		LocalizedStringTable::Map_t const & baseFileMap = baseStringFile->getMap();
		LocalizedStringTable::Map_t const & currentFileMap = currentStringFile->getMap();
		LocalizedStringTable::Map_t & translatedFileMap = translatedStringFile->getMap();
		
		LocalizedStringTable::Map_t::const_iterator end = baseFileMap.end();
		for(LocalizedStringTable::Map_t::const_iterator itBase = baseFileMap.begin(); itBase != end; ++itBase)
		{	
			// Look to see if the current map and translated map contains the base map id.
			LocalizedStringTable::Map_t::const_iterator itCurrentHasStringPair = currentFileMap.find(itBase->first);
			LocalizedStringTable::Map_t::const_iterator itTranslatedHasStringPair = translatedFileMap.find(itBase->first);
			
			// if both containers contain the base string id, continue.
			if (itCurrentHasStringPair != currentFileMap.end() && itTranslatedHasStringPair != translatedFileMap.end())
			{
				LocalizedString const * const baseString = itBase->second;
				LocalizedString const * const currentString = itCurrentHasStringPair->second;
				
				// If the Crcs are the same, update the translated string.
				if (baseString->getCrc() == currentString->getCrc()) 
				{
					// Updated translated string.
					LocalizedString * const translatedString = itTranslatedHasStringPair->second;
					translatedString->setSourceCrc(baseString->getCrc());
				}
			}
		}
		
	}
	else
	{
		DEBUG_WARNING(true, ("Unable to open the base string file %s", baseFileEnglish));
	}

	//-- Since the destination file may change, go ahead and save it out anyway.  Let Perforce iron out the real changes
	if (translatedStringFile->getVersion() < s_latestVersion)
	{
		StdioFileFactory fileFactoryTranslatedUpdate;
		if (!translatedStringFile->writeRW(fileFactoryTranslatedUpdate, std::string(currentFileTranslated)))
			DEBUG_WARNING(true, ("Could not write %s", currentFileTranslated));
	}


	delete translatedStringFile;
	delete baseStringFile;
	delete currentStringFile;
	
	return true;
}

// ----------------------------------------------------------------------

bool UpdateLocalizedStringsNamespace::displayAndUpdateStringFileVersion(char const * const targetFile, bool const update)
{
	DEBUG_REPORT_LOG(true, ("Processing %s\n", targetFile));
	
	StdioFileFactory fileFactoryBase;
	LocalizedStringTableRW * const localizedStringFile = LocalizedStringTableRW::loadRW(fileFactoryBase, std::string(targetFile));
	if (localizedStringFile)
	{
		printf("[version %2d]", localizedStringFile->getVersion());

		//-- Update string file.
		if (update && (localizedStringFile->getVersion() < s_latestVersion))
		{
			StdioFileFactory fileFactorStringFile;
			if (!localizedStringFile->writeRW(fileFactorStringFile, std::string(targetFile)))
				DEBUG_WARNING(true, ("Could not write %s", targetFile));
			
			printf(" updated to version %d.", s_latestVersion);
		}
		
		printf("\n");
	}
	else
	{
		printf("[version ??] %s\n", targetFile);
	}
	
	delete localizedStringFile;
	
	return true;
}

// ======================================================================

void main(int const argc, char const * const argv [])
{
	if (argc < 3)
	{
		printUsage(argc, argv);
		return;
	}
	
	//-- thread
	SetupSharedThread::install();
	
	//-- debug
	SetupSharedDebug::install(4096);
	
	//-- foundation
	SetupSharedFoundation::Data setupFoundationData(SetupSharedFoundation::Data::D_mfc);
	setupFoundationData.useWindowHandle  = false;
	SetupSharedFoundation::install(setupFoundationData);
	
	//-- compression
	SetupSharedCompression::install();
	
	//-- file
	SetupSharedFile::install(false);
	
	//-- 
	TreeFile::addSearchAbsolute(0);
	
	Unicode::String updateMode(Unicode::narrowToWide(argv[1]));
	bool success = false;
	
	if (Unicode::caseInsensitiveCompare(updateMode, Unicode::narrowToWide("version")))
	{
		if (argc == 3)
		{
			char const * const fileName = argv[2];
			displayAndUpdateStringFileVersion(fileName, false);
			success = true;
		}
	}
	else if (Unicode::caseInsensitiveCompare(updateMode, Unicode::narrowToWide("update")))
	{
		if (argc == 3)
		{
			char const * const fileName = argv[2];
			displayAndUpdateStringFileVersion(fileName, true);
			success = true;
		}
	}
	else if (Unicode::caseInsensitiveCompare(updateMode, Unicode::narrowToWide("baseline")))
	{
		if (argc == 5) 
		{
			char const * const baseFileEnglish = argv[2];
			char const * const currentFileEnglish = argv[3];
			char const * const translatedFile = argv[4];
			
			updateCrcs(baseFileEnglish, currentFileEnglish, translatedFile);
			success = true;
		}
	}
	
	if (!success) 
	{
		printUsage(argc, argv);
	}
	
	//-- remove engine
	SetupSharedFoundation::remove();
	SetupSharedThread::remove();
}

// ======================================================================
