// ======================================================================
//
// WordCountTool.cpp
// asommers
//
// copyright 2003, sony online entertainment
//
// ======================================================================

#include "FirstWordCountTool.h"


#include "StringTable.h"

// ======================================================================

namespace WordCountToolNamespace
{
	typedef std::vector<CString> StringList;
	StringList ms_fileNameList;
	StringList ms_newFileNameList;

	void printUsage ();
	void createFileNameList (char const * fileName, StringList & fileNameList);
	void countFileNameList ();
	void dumpFileNameList ();
	void findFileNameList (char const * searchString, bool useID = false);
	void evaluateFileNameList (char const * const baseLanguageCode, char const * const compareLanguageCode);
	bool convertLanguageCode(const CString & source, CString & dest, char const * const sourceCode, char const * const destCode);
	void countNumberOfTranslations (char const * const newFileName);
	int  getOldFileNameIndex (char const * const newFileName);
}

using namespace WordCountToolNamespace;

// ======================================================================

void WordCountToolNamespace::printUsage ()
{
	printf ("WordCountTool is used to perform basic reading of localization files (*.stf)\n");
	printf ("usage:\n");
	printf ("    WordCountTool -c(ount) filenamelist.rsp\n");
	printf ("    WordCountTool -f(ind) filenamelist.rsp string\n");
	printf ("    WordCountTool -i(d find) filenamelist.rsp id\n");
	printf ("    WordCountTool -d(ump) filenamelist.rsp\n");
	printf ("    WordCountTool -e(valuate) filenamelist.rsp base_language_code compare_language_code\n");
	printf ("    WordCountTool -t(ranslations needed from x to y) oldfilenamelist.rsp newfilenamelist.rsp\n");
}

// ----------------------------------------------------------------------

void WordCountToolNamespace::createFileNameList (char const * const fileName, StringList & fileNameList)
{
	CStdioFile infile;
	if (!infile.Open (fileName, CFile::modeRead | CFile::typeText))
		return;

	//-- read each line...
	CString line;
	while (infile.ReadString (line))
	{
		//-- see if the line is empty
		line.TrimLeft ();
		line.TrimRight ();
		if (line.GetLength () == 0)
			continue;

		//-- skip comments
		if (line [0] == '#')
			continue;

		//-- skip any lines that don't end in .stf
		if (line.Find (".stf") == -1)
			continue;

		//-- force lower case
		line.MakeLower ();

		//-- make sure the file exists
		CStdioFile infile2;
		if (infile2.Open (line, CFile::modeRead | CFile::typeText))
		{
			fileNameList.push_back (line);
			infile2.Close ();
		}
	}
}

// ----------------------------------------------------------------------

void WordCountToolNamespace::countFileNameList ()
{
	int totalNumberOfStrings = 0;
	int totalNumberOfLines = 0;
	int totalNumberOfWords = 0;
	int totalNumberOfFiles = 0;

	printf ("Strings\tLines\tWords\tFileName\n");
	for (size_t i = 0; i < ms_fileNameList.size (); ++i)
	{
		StringTable stringTable;
		stringTable.load (ms_fileNameList [i]);

		int const numberOfStrings = stringTable.getNumberOfStrings ();
		int const numberOfLines = stringTable.getNumberOfLines ();
		int const numberOfWords = stringTable.getNumberOfWords ();

		printf ("%i\t%i\t%i\t%s\n", numberOfStrings, numberOfLines, numberOfWords, ms_fileNameList [i]);

		totalNumberOfStrings += numberOfStrings;
		totalNumberOfLines += numberOfLines;
		totalNumberOfWords += numberOfWords;
		++totalNumberOfFiles;
	}

	printf ("%i\t%i\t%i\t%i files\n", totalNumberOfStrings, totalNumberOfLines, totalNumberOfWords, totalNumberOfFiles);
}

// ----------------------------------------------------------------------

void WordCountToolNamespace::countNumberOfTranslations (char const * const newFileName)
{
	createFileNameList (newFileName, ms_newFileNameList);
	
	int totalNumberOfNewTranslations = 0;
	int totalNumberOfChangedTranslations = 0;

	printf ("New\tChanged\tFileName\n");
	for (size_t i = 0; i < ms_newFileNameList.size (); ++i)
	{
		StringTable oldStringTable;
		StringTable newStringTable;
		int oldFileIndex = -1;
		newStringTable.load (ms_newFileNameList [i]);
		oldFileIndex = getOldFileNameIndex(ms_newFileNameList [i]);

		int numberOfNewTranslations = 0;
		int numberOfChangedTranslations = 0;

		if (oldFileIndex != -1)
		{
			oldStringTable.load(ms_fileNameList [oldFileIndex]);
			int const numberOfStrings = newStringTable.getNumberOfStrings ();

			for (int j = 0; j < numberOfStrings; j++)
			{
				std::string newStringId = newStringTable.getStringId(j);
				int oldStringIndex = oldStringTable.getIndexForStringId(newStringId);
				if (oldStringIndex == -1)
					numberOfNewTranslations += newStringTable.getNumberOfWords(j);
				else
				{
					std::string newValue = newStringTable.getString(j);
					std::string oldValue = oldStringTable.getString(oldStringIndex);
					if (newValue.compare(oldValue) != 0)
					{
						numberOfChangedTranslations += newStringTable.getNumberOfWords(j);
					}
				}
			}
		}
		else
		{
			numberOfNewTranslations = newStringTable.getNumberOfWords();
		}

		printf ("%i\t%i\t%s\n", numberOfNewTranslations, numberOfChangedTranslations, ms_newFileNameList [i]);

		totalNumberOfNewTranslations += numberOfNewTranslations;
		totalNumberOfChangedTranslations += numberOfChangedTranslations;
	}


	printf ("%i New\t%i Changed\t%i Total\n", totalNumberOfNewTranslations, totalNumberOfChangedTranslations, totalNumberOfNewTranslations + totalNumberOfChangedTranslations);
}

// ----------------------------------------------------------------------

int WordCountToolNamespace::getOldFileNameIndex (char const * const newFileName)
{
	int index = -1;
	std::string newFileString = newFileName;
	std::string newSearchName = newFileString.substr(newFileString.rfind("/en/"));
	for (size_t i = 0; i < ms_fileNameList.size (); ++i)
	{
		std::string oldFileString = ms_fileNameList [i];
		std::string oldSearchName = oldFileString.substr(oldFileString.rfind("/en/"));
		if (oldSearchName.compare(newSearchName) == 0)
		{
			index = i;
			break;
		}
	} 
	return index;
}

// ----------------------------------------------------------------------

void WordCountToolNamespace::dumpFileNameList ()
{
	for (size_t i = 0; i < ms_fileNameList.size (); ++i)
	{
		StringTable stringTable;
		stringTable.load (ms_fileNameList [i]);

		printf ("%s\n", ms_fileNameList [i]);

		for (int j = 0; j < stringTable.getNumberOfStrings (); ++j)
		{
			//-- Remove all newlines before printing
			CString string = stringTable.getString (j).c_str ();
			string.Remove('\n');

			printf ("\t%s\t%s\n", stringTable.getStringId (j).c_str (), string);
		}
	}
}

// ----------------------------------------------------------------------

void WordCountToolNamespace::findFileNameList (char const * const searchString, bool useID)
{
	for (size_t i = 0; i < ms_fileNameList.size (); ++i)
	{
		StringTable stringTable;
		stringTable.load (ms_fileNameList [i]);

		bool printedHeader = false;
		for (int j = 0; j < stringTable.getNumberOfStrings (); ++j)
		{
			CString string;
			if (useID)
				string = stringTable.getStringId (j).c_str ();
			else
				string = stringTable.getString (j).c_str ();
			string.Remove('\n');

			CString lowerString = string;
			lowerString.MakeLower();

			CString lowerSearchString = searchString;
			lowerSearchString.MakeLower();

			if (lowerString.Find (lowerSearchString) != -1)
			{
				if (!printedHeader)
				{
					printf ("%s\n", ms_fileNameList [i]);
					printedHeader = true;
				}

				std::string stringId;
				if (useID)
				{
					stringId = stringTable.getString(j);
					printf ("\t%s\t%s\n", string, stringId.c_str());
				}
				else
				{
					stringId = stringTable.getStringId(j);
					printf ("\t%s\t%s\n", stringId.c_str(), string);
				}
			}
		}
	}
}

// ----------------------------------------------------------------------

bool WordCountToolNamespace::convertLanguageCode(const CString & source, CString & dest, char const * const sourceCode, char const * const destCode)
{
	int previousSlash = 0;
	int nextSlash = source.Find('\\', previousSlash);
	char word[4096];
	bool found = false;
	int i;
	while(nextSlash > 0)
	{
		int wordSize = (nextSlash - previousSlash + 1);
		for(i = previousSlash; i < nextSlash; i++)
			word[i - previousSlash] = source[i];
		word[wordSize - 1] = '\0';
		//printf("found word '%s'\n", word);		
		if(strcmp(sourceCode, word) == 0)		
		{
			found = true;
			dest += destCode;		
		}
		else		
			dest += word;		
		dest += '\\';

		previousSlash = nextSlash + 1;
		nextSlash = source.Find('\\', previousSlash);
	}
	dest += source.Right(strlen(source) - previousSlash);
	return found;
}

// ----------------------------------------------------------------------

void WordCountToolNamespace::evaluateFileNameList (char const * const baseLanguageCode, char const * const compareLanguageCode)
{
	int missingTableCount = 0;
	int missingCount = 0;
	int missingCountPerTable = 0;
	int olderCount = 0;
	int olderCountPerTable = 0;
	int count = 0;
	printf("Strings\tMissing\tOlder\tTableMissing\tFileName\n");
	for (size_t i = 0; i < ms_fileNameList.size (); ++i)
	{
		missingCountPerTable = 0;
		olderCountPerTable = 0;
		CString compareLanguageFileName;
		if (!convertLanguageCode(ms_fileNameList [i], compareLanguageFileName, baseLanguageCode, compareLanguageCode))
		{
			//printf("Couldn't convert filename '%s'\n", ms_fileNameList[i]);
			continue;
		}
		//printf("Converted filename '%s' to '%s'\n", ms_fileNameList[i], compareLanguageFileName);
		StringTable stringTable;
		stringTable.load (ms_fileNameList [i]);

		count += stringTable.getNumberOfStrings();
		CFileStatus fileStatus;
		if(!CFile::GetStatus(compareLanguageFileName, fileStatus))
		{
			missingTableCount++;
			missingCount += stringTable.getNumberOfStrings();			
			printf("%d\t%d\t%d\tMISSING\t\t%s\n", stringTable.getNumberOfStrings(), stringTable.getNumberOfStrings(), 0, ms_fileNameList[i]);
			continue;
		}
		
		StringTable compareStringTable;
		compareStringTable.load(compareLanguageFileName);

		for (int j = 0; j < stringTable.getNumberOfStrings (); ++j)
		{
			const std::string & stringTableStringId = stringTable.getStringId (j);
			const int compareStringTableIndex = compareStringTable.getIndexForStringId(stringTableStringId);
			if(compareStringTableIndex == -1)
			{
				missingCount++;
				missingCountPerTable++;
				continue;
			}

			LocalizedString::crc_type const & crcBase = stringTable.getCRC(j);
			LocalizedString::crc_type const & crcBaseSource = stringTable.getSourceCRC(j);
			
			LocalizedString::crc_type const & crcCompare = compareStringTable.getCRC(compareStringTableIndex);
			LocalizedString::crc_type const & crcCompareSource = compareStringTable.getSourceCRC(compareStringTableIndex);

			if ((crcCompareSource != LocalizedString::nullCrc && crcBase != crcCompareSource) || 
				(crcBaseSource != LocalizedString::nullCrc && crcCompare != crcBaseSource))
			{
				olderCount++;
				olderCountPerTable++;
			}
		}
		printf("%d\t%d\t%d\t \t\t%s\n", stringTable.getNumberOfStrings(), missingCountPerTable, olderCountPerTable, ms_fileNameList[i]);
	}
	printf("Totals: Count %d Number Missing Tables: %d Number Missing Strings: %d Number Older Strings: %d\n", count, missingTableCount, missingCount, olderCount);
}

// ======================================================================

void main (int const argc, char const * const argv [])
{
	if (argc < 3)
	{
		printUsage ();
		return;
	}

	createFileNameList (argv [2], ms_fileNameList);

	if (ms_fileNameList.empty ())
	{
		printUsage ();
		printf ("error: %s does not contain any valid string filenames\n");
		return;
	}

	char const * const option = argv [1];
	if (option [1] == 'c')
		countFileNameList ();
	else if (option [1] == 't')
	{
		if (argc < 4)
		{
			printUsage ();
			printf ("error: specified translation count with no old files\n");
			return;
		}

		countNumberOfTranslations (argv [3]);
	}
	else if (option [1] == 'd')
		dumpFileNameList ();
	else if (option [1] == 'f' ||
			 option [1] == 'i')
	{
		if (argc < 4)
		{
			printUsage ();
			printf ("error: specified find with no search string\n");
			return;
		}
		
		char const * const findString = argv [3];
		findFileNameList (findString, option [1] == 'i');
	}
	else if (option[1] == 'e')
	{
		if (argc < 5)
		{
			printUsage ();
			printf ("error: evaluate specified without enough parameters\n");
			return;
		}
		char const * const baseLanguageCode = argv[3];
		char const * const compareLanguageCode = argv[4];
		evaluateFileNameList (baseLanguageCode, compareLanguageCode);
	}
	else
	{				
		printUsage ();
		printf ("error: invalid option\n");
		return;
	}
}

// ======================================================================
