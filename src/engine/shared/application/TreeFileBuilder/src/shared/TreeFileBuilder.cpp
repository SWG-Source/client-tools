// ======================================================================
//
// TreeFileBuilder.cpp
// ala diaz
//
// Portions copyright 1999 Bootprint Entertainment
// Portions copyright 2001-2002 Sony Online Entertainment Inc
//
// ======================================================================

#include "FirstTreeFileBuilder.h"
#include "TreeFileBuilder.h"

#include "sharedCompression/SetupSharedCompression.h"
#include "sharedCompression/Compressor.h"
#include "sharedCompression/Lz77.h"
#include "sharedFile/TreeFile_SearchNode.h"
#include "sharedFoundation/CommandLine.h"
#include "sharedFoundation/Crc.h"
#include "sharedFoundation/Os.h"
#include "sharedFoundation/SetupSharedFoundation.h"
#include "sharedThread/SetupSharedThread.h"
#include "sharedDebug/SetupSharedDebug.h"
#include "sharedIoWin/SetupSharedIoWin.h"
#include "sharedFile/SetupSharedFile.h"

#include <cstdio>
#include <algorithm>

// ======================================================================

const Tag TAG_TREE = TAG(T,R,E,E);

// ======================================================================

extern int main(int argc, char **argv);
static void run(void);
static void usage(void);

// ======================================================================
// command line stuff

static const char * const LNAME_HELP				 = "help";
static const char * const LNAME_RSP_FILE			 = "responseFile";
static const char * const LNAME_NO_TOC_COMPRESSION	 = "noTOCCompression";
static const char * const LNAME_NO_FILE_COMPRESSION  = "noFileCompression";
static const char * const LNAME_NO_CREATE		     = "noCreate";
static const char * const LNAME_QUIET		     = "quiet";
static const char         SNAME_HELP				= 'h';
static const char         SNAME_RSP_FILE			= 'r';
static const char         SNAME_NO_TOC_COMPRESSION	= 't';
static const char         SNAME_NO_FILE_COMPRESSION = 'f';
static const char         SNAME_NO_CREATE			= 'c';
static const char         SNAME_QUIET			= 'q';

static CommandLine::OptionSpec optionSpecArray[] =
{
	OP_BEGIN_SWITCH(OP_NODE_REQUIRED),

		// help
		OP_SINGLE_SWITCH_NODE(SNAME_HELP, LNAME_HELP, OP_ARG_NONE, OP_MULTIPLE_DENIED),

		// real options
		OP_BEGIN_SWITCH_NODE(OP_MULTIPLE_DENIED),
			OP_BEGIN_LIST(),

				// rsp file required
				OP_SINGLE_LIST_NODE(SNAME_RSP_FILE, LNAME_RSP_FILE, OP_ARG_REQUIRED, OP_MULTIPLE_DENIED,  OP_NODE_REQUIRED),

				// if specified, don't use compression on file entries
				OP_SINGLE_LIST_NODE(SNAME_NO_TOC_COMPRESSION, LNAME_NO_TOC_COMPRESSION, OP_ARG_NONE, OP_MULTIPLE_DENIED, OP_NODE_OPTIONAL),

				// if specified, don't use compression on file entries and TOC
				OP_SINGLE_LIST_NODE(SNAME_NO_FILE_COMPRESSION, LNAME_NO_FILE_COMPRESSION, OP_ARG_NONE, OP_MULTIPLE_DENIED, OP_NODE_OPTIONAL),

				// if specified, don't create the treefile
				OP_SINGLE_LIST_NODE(SNAME_NO_CREATE, LNAME_NO_CREATE, OP_ARG_NONE, OP_MULTIPLE_DENIED, OP_NODE_OPTIONAL),

				OP_SINGLE_LIST_NODE(SNAME_QUIET, LNAME_QUIET, OP_ARG_NONE, OP_MULTIPLE_ALLOWED, OP_NODE_OPTIONAL),

				// get the output tree file name
				OP_SINGLE_LIST_NODE(OP_SNAME_UNTAGGED, OP_LNAME_UNTAGGED, OP_ARG_REQUIRED, OP_MULTIPLE_DENIED,  OP_NODE_REQUIRED),

			OP_END_LIST(),
		OP_END_SWITCH_NODE(),

	OP_END_SWITCH()
};

static const int optionSpecCount = sizeof(optionSpecArray) / sizeof(optionSpecArray[0]);

static int       errors;
static bool      disableTOCCompression;
static bool      disableFileCompression;
static bool      disableCreation;
static int       quiet;

static std::vector<std::string> warnings;

// ======================================================================

int main(int argc, char **argv)
{
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

	SetupSharedFoundation::callbackWithExceptionHandling(run);
	SetupSharedFoundation::remove();
	SetupSharedThread::remove();

	return -errors;
}

// ----------------------------------------------------------------------

class TreeFileBuilderHelper
{
public:

	static const Tag getToken()
	{
		return TAG(T,R,E,E);
	}

	static const Tag getVersion()
	{
		return TAG_0005;
	}
};

// ----------------------------------------------------------------------

static void run(void)
{
	// handle options
	const CommandLine::MatchCode mc = CommandLine::parseOptions(optionSpecArray, optionSpecCount);

	if (mc != CommandLine::MC_MATCH)
	{
		printf("Invalid command line specified.  Printing usage...\n");
		usage();
		++errors;
		return;
	}

	if (CommandLine::getOccurrenceCount(SNAME_HELP))
	{
		usage();
		return;
	}

	if (CommandLine::getOccurrenceCount(SNAME_NO_TOC_COMPRESSION))
		disableTOCCompression = true;

	if (CommandLine::getOccurrenceCount(SNAME_NO_FILE_COMPRESSION))
		disableFileCompression = true;

	if (CommandLine::getOccurrenceCount(SNAME_NO_CREATE))
		disableCreation = true;

	quiet = CommandLine::getOccurrenceCount(SNAME_QUIET);

	// a valid set of command line options has been specified
	TreeFileBuilder t(CommandLine::getUntaggedString(0));

	if (errors)
		return;

	t.addResponseFile(CommandLine::getOptionString(SNAME_RSP_FILE));

	if (errors)
		return;

	if (disableCreation)
	{
		printf("Scan complete. No errors.\n");
		return;
	}

	t.createFile();

	if (errors)
		return;

	t.write();

	if (warnings.size ())
	{
		printf ("Warnings:\n");

		uint i;
		for (i = 0; i < warnings.size (); ++i)
			printf ("  %s", warnings [i].c_str ());
	}
}

// ----------------------------------------------------------------------

static void usage(void)
{
	#include "TreeFileBuilder.dox"
}

// ======================================================================

TreeFileBuilder::FileEntry::FileEntry(const char *diskFileName, const char *treeFileName)
: diskFileEntry(DuplicateString(diskFileName)),
	treeFileEntry(treeFileName),
	offset(0),
	length(0),
	compressor(0),
	compressedLength(0),
	deleted(false),
	uncompressed(false)
{
}

// ----------------------------------------------------------------------

TreeFileBuilder::FileEntry::~FileEntry(void)
{
}

// ======================================================================

TreeFileBuilder::TreeFileBuilder(const char *newTreeFileName)
: treeFileName(DuplicateString(newTreeFileName)),
	treeFileHandle(INVALID_HANDLE_VALUE),
	numberOfFiles(0),
	totalFileSize(0),
	totalSmallestSize(0),
	sizeOfTOC(0),
	tocCompressorID(0),
	duplicateCount(0),
	uncompSizeOfNameBlock(0)
{
	DEBUG_FATAL(!treeFileName, ("treeFileName may not be NULL"));
}

// ----------------------------------------------------------------------

TreeFileBuilder::~TreeFileBuilder(void)
{
	if (treeFileHandle != INVALID_HANDLE_VALUE)
	{
		const BOOL result = CloseHandle(treeFileHandle);
		FATAL(!result, ("error %d closing treefile '%s'", GetLastError(), treeFileName));
		treeFileHandle = INVALID_HANDLE_VALUE;
	}

	if (errors)
	{
		if(unlink(treeFileName))
			printf("Could not delete '%s'.\n", treeFileName);
	}

	delete [] treeFileName;

	std::vector<FileEntry*>::iterator iter = tocOrder.begin();

	// takes care of referenced memory in both responseFileOrder and tocOrder
	for (; iter != tocOrder.end(); ++iter)
	{
		delete [] (*iter)->diskFileEntry;
		delete *iter;
	}
}

// ----------------------------------------------------------------------

void TreeFileBuilder::createFile(void)
{
	treeFileHandle = CreateFile(treeFileName, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);

	if (treeFileHandle == INVALID_HANDLE_VALUE)
	{
		fprintf(stderr, "Error opening output TreeFile %s\n", treeFileName);
		++errors;
	}
}

// ----------------------------------------------------------------------

bool TreeFileBuilder::LessFileEntryCrcNameCompare(const FileEntry* a, const FileEntry* b)
{
	// comparing new file entry with iterator to determine insert index
	return (a->treeFileEntry < b->treeFileEntry);
}

// ----------------------------------------------------------------------

static bool isjunk(int ch)
{
	return isspace(ch) || ch == '\n' || ch == '\r';
}

// ----------------------------------------------------------------------

void TreeFileBuilder::addResponseFile(const char *responseFileName)
{
	char currentBuffer [10 * 1024];
	char nameBuffer[10 * 1024];

	printf("Processing response file %s\n", responseFileName);
	FILE *file = fopen(responseFileName, "rt");

	if (!file)
	{
		fprintf(stderr, "Unable to open response file %s\n", responseFileName);
		++errors;
		return;
	}

	for (;;)
	{
		nameBuffer[0] = '\0';
		currentBuffer[0] = '\0';

		// new name that may be found within current
		char *newName = nameBuffer;

		// get the whole line
		if (fgets(currentBuffer, sizeof(currentBuffer), file) == NULL)
			break;

		// ignore line if it doesn't have an @
		if (strstr(currentBuffer, "@") == 0)
			continue;

		// strip out "TF::open(x) " spit out in output window with the DFP_treeFileLog flag
		char *current = currentBuffer;
		char *start = strstr(current, "TF::open");
		bool validTFopen = false;
		if (start)
		{
			current = start + 12;

			// find ,
			char *end = strstr(current, ",");
			if (end)
			{
				validTFopen = true;
				*end = '\0';
			}
		}

		// lop off leading whitespace
		if (!validTFopen)
		{
			while (isjunk(*current))
				++current;

			//lop off trailing whitespace
			while (strlen(current) && isjunk(current[strlen(current)-1]))
				 current[strlen(current)-1] = '\0';
		}

		// will indicate if '@' name change switch is in the current file name
		bool switchEncountered = false;

		// will indicate if 'u' followed name change switch - designating the file should not be compressed
		bool uncompressedFile = false;

		// will update to the index of the first char in disk file name after new name is removed from the front
		int index=0;

		// examine every character in the file name
		for (int i=0; current[i] != '\0'; ++i)
		{
			if(current[i] == '@')
			{
				//insert all chars up to the space before the '@' symbol
				for(int k=0; k < i-1; ++k)
					newName[k] = current[k];

				//check to see if file should not be compressed
				if(current[i+1] == 'u')
					uncompressedFile = true;

				switchEncountered = true;

				//terminate string that is the new name (the name stored in the tree file)
				newName[i-1] = '\0';

				//update the index to the first char of the disk file name
				i += uncompressedFile ? 3 : 2;
			}

			if (switchEncountered)
				//update current without the new name on the front starting at index = 0
				current[index++] = current[i];
		}

		if (switchEncountered)
			//null terminate the reset file name with the new file name removed from front
			current[index] = '\0';
		else
			newName = current;

		addFile(current, newName, switchEncountered, uncompressedFile);

		if (errors)
		{
			fclose(file);
			return;
		}

	}//end for

	printf("Added %d files with %d duplicate file(s)\n", numberOfFiles, duplicateCount);
	fclose(file);
}

// ----------------------------------------------------------------------

void TreeFileBuilder::addFile(const char *diskFileNameEntry, const char *treeFileNameEntry, bool changingFileName, bool uncompressedFile)
{
	const char *src;
	char       *dest;
	char	     lowerTreeFileName[Os::MAX_PATH_LENGTH];
	const int  len = strlen(treeFileNameEntry);

	src = lowerTreeFileName;
	FATAL(len >= Os::MAX_PATH_LENGTH, ("File name too long: '%s'", treeFileNameEntry));

	// copy and lowercase the tree file name
	for (src = treeFileNameEntry, dest = lowerTreeFileName; *src; ++src, ++dest)
	{
		char c = *src;
		if (c == '\\')
			*dest = '/';
		else
			*dest = static_cast<char> (tolower (c));
	}

	*dest = '\0';

	bool deleted = true;
	if (strcmp (diskFileNameEntry, "deleted") != 0)
	{
		deleted = false;

		// try to open the file
 		HANDLE handle = CreateFile(diskFileNameEntry, GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);

		if (handle == INVALID_HANDLE_VALUE)
		{
			char warning [1024];
			sprintf (warning, "Unable to open data file %s\n", diskFileNameEntry);
			warnings.push_back (warning);
			return;
		}

		// detect 0 byte files, warn, and handle them as deleted files
		if (GetFileSize(handle, NULL) == 0)
		{
			char warning [1024];
			sprintf (warning, "0-byte file detected %s\n", diskFileNameEntry);
			warnings.push_back (warning);
			deleted = true;
		}

		// close the file
		BOOL result = CloseHandle(handle);
		handle = INVALID_HANDLE_VALUE;
		FATAL(!result, ("error closing %s.  Error %d", diskFileNameEntry, GetLastError()));
	}

	FileEntry* const newFileEntry = new FileEntry(diskFileNameEntry, lowerTreeFileName);
	newFileEntry->deleted = deleted;
	newFileEntry->uncompressed = uncompressedFile;

	std::vector<FileEntry*>::iterator iter;

	// set iterator to the first element whose Crc value is not less than newFileEntry
	iter = std::lower_bound(tocOrder.begin(), tocOrder.end(), newFileEntry, LessFileEntryCrcNameCompare);

	bool duplicateFileEntry = false;

	if (iter != tocOrder.end())
	{
		// an error results if adding two different diskFiles as the same treeFile
		if (newFileEntry->treeFileEntry == (*iter)->treeFileEntry && *(newFileEntry->diskFileEntry) != *((*iter)->diskFileEntry))
		{
			fprintf(stderr, "Unable to rename two different files with the same name: %s\n", (newFileEntry->treeFileEntry).getString());
			++errors;

			delete [] newFileEntry->diskFileEntry;
			delete newFileEntry;
			return;
		}
		else
			// check to see if the new file entry is a duplicate
			duplicateFileEntry = (newFileEntry->treeFileEntry == (*iter)->treeFileEntry);
	}

	if (duplicateFileEntry)
	{
		if (changingFileName)
			printf("*Duplicate file not added: %s[%s]\n", newFileEntry->treeFileEntry.getString(), newFileEntry->diskFileEntry);
		else
			printf("*Duplicate file not added: %s\n", newFileEntry->treeFileEntry.getString());

		++duplicateCount;

		delete [] newFileEntry->diskFileEntry;
		delete newFileEntry;
	}
	else
	{
		if (quiet < 1)
		{
			if (changingFileName)
				printf("Adding file %s[%s]\n", newFileEntry->treeFileEntry.getString(), newFileEntry->diskFileEntry);
			else
				printf("Adding file %s\n", newFileEntry->treeFileEntry.getString());
		}

		tocOrder.insert(iter, newFileEntry);
		responseFileOrder.insert(responseFileOrder.end(), newFileEntry);

		// allows a known creation size of array right before compression of name block
		uncompSizeOfNameBlock += (strlen(newFileEntry->treeFileEntry.getString()) + 1);
		++numberOfFiles;
	}
}

// ----------------------------------------------------------------------

void TreeFileBuilder::write(const void *data, int length)
{
	DWORD wrote = 0;
	DWORD result = WriteFile(treeFileHandle, data, length, &wrote, NULL);
	UNREF(result);
	DEBUG_FATAL(result == 0 || static_cast<DWORD>(length) != wrote, ("write failed"));
}

// ----------------------------------------------------------------------

void TreeFileBuilder::writeFile(FileEntry *fileEntry)
{
	if (fileEntry->deleted)
	{
		if (quiet < 2)
			printf("  storing deleted file %s\n", fileEntry->treeFileEntry.getString());

		return;
	}

	const char *fileName = fileEntry->diskFileEntry;

	HANDLE handle = CreateFile(fileName, GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);

	// get the file length
	const int fileLength = static_cast<int>(GetFileSize(handle, NULL));

	// read the file into a buffer
	byte *uncompressed = new byte[fileLength];

	DWORD red;
	BOOL b = ReadFile(handle, uncompressed, fileLength, &red, NULL);

	FATAL(!b || red != static_cast<DWORD>(fileLength), ("could not read file %s\n", fileName));

	// close the file
	b = CloseHandle(handle);

	FATAL(!b, ("could not close file %s\n", fileName));

	// store the info about this file
	fileEntry->length     = fileLength;
	fileEntry->offset     = SetFilePointer(treeFileHandle, 0, NULL, FILE_CURRENT);

	// variable determining whether file compression is disabled or not
	bool disableCompression = (disableFileCompression || fileEntry->uncompressed);

	compressAndWrite(uncompressed, fileEntry->compressedLength, fileLength, fileEntry->compressor, true, disableCompression, &fileEntry->md5);
	delete [] uncompressed;
}

// ----------------------------------------------------------------------

void TreeFileBuilder::writeTableOfContents()
{
	TreeFile::SearchTree::TableOfContentsEntry  entry;

	int uncompSizeOfTOC = sizeof(entry) * numberOfFiles;
	int currentOffset   = 0;

	std::vector<FileEntry*>::iterator iter = tocOrder.begin();

	byte *uncompressed = new byte[uncompSizeOfTOC];
	byte *uncompIter   = uncompressed;

	// compress and write all of the file TOC info
	for (; iter != tocOrder.end(); ++iter, uncompIter += sizeof(entry))
	{
		// prepare entry by zeroing out the total size of data to be stored
		memset(&entry, 0, sizeof(entry));

		entry.crc                = (*iter)->treeFileEntry.getCrc();
		entry.length             = (*iter)->length;
		entry.offset             = (*iter)->offset;
		entry.compressor         = (*iter)->compressor;
		entry.compressedLength   = (*iter)->compressedLength;
		entry.fileNameOffset     = currentOffset;
		currentOffset           += strlen((*iter)->treeFileEntry.getString()) + 1;

		// copy the entry
		memcpy(uncompIter, &entry, sizeof(entry));
		fileNameBlock.push_back((*iter)->treeFileEntry.getString());
	}

	compressAndWrite(uncompressed, sizeOfTOC, uncompSizeOfTOC, tocCompressorID, false, disableTOCCompression, NULL);
	delete [] uncompressed;
}

// ----------------------------------------------------------------------

void TreeFileBuilder::writeFileNameBlock()
{
	byte *uncompressed = new byte[uncompSizeOfNameBlock];
	byte *uncompIter   = uncompressed;

	std::vector<const char *>::iterator iter = fileNameBlock.begin();

	for (int nameLength=0; iter != fileNameBlock.end(); ++iter)
	{
		nameLength = strlen(*iter) + 1;
		memcpy(uncompIter, *iter, nameLength);
		uncompIter += nameLength;
	}

	compressAndWrite(uncompressed, sizeOfNameBlock, uncompSizeOfNameBlock, blockCompressorID, false, disableTOCCompression, NULL);
	delete [] uncompressed;
}

// ----------------------------------------------------------------------

void TreeFileBuilder::writeMd5Block()
{
	int const sizeOfMd5Block = numberOfFiles * Md5::Value::cms_dataSize;
	byte *md5Block = new byte[sizeOfMd5Block];
	byte *destination = md5Block;

	// copy all of the md5 sums into a single block
	for (std::vector<FileEntry*>::iterator iter = tocOrder.begin(); iter != tocOrder.end(); ++iter, destination += Md5::Value::cms_dataSize)
		memcpy(destination, (*iter)->md5.getData(), Md5::Value::cms_dataSize);

	int compressedSize = 0;
	int compressor = 0;
	compressAndWrite(md5Block, compressedSize, sizeOfMd5Block, compressor, false, true, NULL);
	delete [] md5Block;
}

// ----------------------------------------------------------------------

void TreeFileBuilder::compressAndWrite(const byte *uncompressed, int &sizeOfData, const int uncompressedSize, int &compressor, const bool isFileData, const bool disableCompression, Md5::Value *md5)
{
	int smallest = TreeFile::SearchTree::CT_none;
	int smallestSize = uncompressedSize;
	byte * smallestBuffer = NULL;

	if (!disableCompression && uncompressedSize > 1024)
	{
		// try every compressor on the buffer
		TreeFile::SearchTree::CompressorType compressors[] =
		{
			TreeFile::SearchTree::CT_zlib
		};
		int const numberOfCompressors = sizeof(compressors) / sizeof(compressors[0]);

		for (int i = 0; i < numberOfCompressors; i++)
		{
			TreeFile::SearchTree::CompressorType compressorType = compressors[i];

			// make a temporary buffer
			int newBufferLength = uncompressedSize * 2;
			byte *newBuffer = new byte[newBufferLength];

			// try the compressor on this data
			Compressor *compressor = TreeFile::SearchTree::borrowCompressor(compressorType);
			int size = compressor->compress(uncompressed, uncompressedSize, newBuffer, newBufferLength);
			TreeFile::SearchTree::returnCompressor(compressorType, compressor);
			compressor = NULL;

			if (size < smallestSize)
			{
				// this compressor beat our old compressor and it compressed the original data by at least 512 bytes
				smallestSize = size;
				smallest = compressorType;
				delete [] smallestBuffer;
				smallestBuffer = newBuffer;
			}
			else
			{
				// this compressor lost, too bad
				delete [] newBuffer;
			}
		}
	}

	// if file data, print out compression summary for each file
	if (isFileData)
	{
		if (TreeFile::SearchTree::isCompressed(smallest))
		{
			if (quiet < 2)
				printf("  storing compressed(%d)   - OrigSize: %6d  CmpSize: %6d  Ratio: %3d%%\n", smallest, uncompressedSize, smallestSize, 100 - ((smallestSize * 100) / uncompressedSize));
			sizeOfData = smallestSize;
			write(smallestBuffer, smallestSize);
		}
		else
		{
			sizeOfData = 0;
			if (quiet < 2)
				printf("  storing uncompressed    - size: %6d\n", uncompressedSize);
			write(uncompressed, uncompressedSize);
		}

		totalFileSize += uncompressedSize;
	}
	else
	{
		if (TreeFile::SearchTree::isCompressed(smallest))
		{
			sizeOfData = smallestSize;
			write(smallestBuffer, smallestSize);
		}
		else
		{
			sizeOfData = uncompressedSize;
			write(uncompressed, uncompressedSize);
		}

		totalFileSize += smallestSize;
	}

	if (md5)
	{
		if (TreeFile::SearchTree::isCompressed(smallest))
			*md5 = Md5::calculate(smallestBuffer, smallestSize);
		else
			*md5 = Md5::calculate(uncompressed, uncompressedSize);
	}

	totalSmallestSize += smallestSize;
	compressor		   = smallest;

	delete [] smallestBuffer;
}

// ----------------------------------------------------------------------

void TreeFileBuilder::write(void)
{
	// write the header
	TreeFile::SearchTree::Header header;
	header.token         = TAG_TREE;
	header.version       = TAG_0005;
	header.numberOfFiles = numberOfFiles;
	write(&header, sizeof(header));

	std::vector<FileEntry*>::iterator iter = responseFileOrder.begin();

	printf("Generating tree file body\n");

	// write all of the files
	const uint total = responseFileOrder.size ();
	for(int current = 1; iter != responseFileOrder.end(); ++iter, ++current)
	{
		if (quiet < 2)
			printf("[%4i/%4i] ", current, total);
		writeFile(*iter);
	}

	// calculate the tocOffset before writing the TOC
	int tocOffset = totalSmallestSize + sizeof(header);

	printf("Writing table of contents\n");
	writeTableOfContents();
	writeFileNameBlock();

	printf("Writing md5sum block\n");
	writeMd5Block();

	// now go back and update the header info with the correct data
	SetFilePointer(treeFileHandle, FILE_BEGIN, NULL, FILE_BEGIN);
	header.tocOffset		     = tocOffset;
	header.tocCompressor	     = tocCompressorID;
	header.sizeOfTOC	         = sizeOfTOC;
	header.blockCompressor       = blockCompressorID;
	header.sizeOfNameBlock		 = sizeOfNameBlock;
	header.uncompSizeOfNameBlock = uncompSizeOfNameBlock;
	write(&header, sizeof(header));

	if(totalFileSize == totalSmallestSize)
		printf("Final compression: OrigSize: %6d  CmpSize: %6d  Ratio: %3d%%\n", totalFileSize,
			    totalSmallestSize, 0);
	else
	{
		if (totalFileSize > 1024 * 1024)
		{
			const int totalFileSizeMegaBytes = totalFileSize /  (1024 * 1024);
			const int totalSmallestSizeMegaBytes = totalSmallestSize / (1024 * 1024);
			printf("Final compression: OrigSize: %6dMB CmpSize: %6dMB  Ratio: %3d%%\n", totalFileSizeMegaBytes,
					totalSmallestSizeMegaBytes, 100 - ((totalSmallestSizeMegaBytes * 100) / totalFileSizeMegaBytes));
		}
		else
		{
			printf("Final compression: OrigSize: %6d  CmpSize: %6d  Ratio: %3d%%\n", totalFileSize,
					totalSmallestSize, 100 - ((totalSmallestSize * 100) / totalFileSize));
		}
	}
}

// ======================================================================
