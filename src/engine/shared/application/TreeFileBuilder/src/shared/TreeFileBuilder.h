// ======================================================================
//
// TreeFileBuilder.h
// ala diaz
//
// copyright 1999 Bootprint Entertainment
//
// ======================================================================

#ifndef INCLUDED_TreeFileBuilder_H
#define INCLUDED_TreeFileBuilder_H

// ======================================================================

#include "sharedFile/TreeFile.h"
#include "sharedFoundation/CrcLowerString.h"
#include "sharedFoundation/Md5.h"
#include <vector>
#include <string>

// ======================================================================

class Compressor;

// ======================================================================

class TreeFileBuilder
{
private:

	TreeFileBuilder(void);
	TreeFileBuilder(const TreeFileBuilder&);
	TreeFileBuilder &operator =(const TreeFileBuilder&);

private:

	struct FileEntry
	{
	private:

		FileEntry(void);
		FileEntry(const FileEntry &);
		FileEntry &operator =(const FileEntry &);

	public:

		char           *diskFileEntry;
		CrcLowerString treeFileEntry;
		int            offset;
		int            length;
		int            compressor;
		int            compressedLength;
		Md5::Value     md5;
		bool           deleted;
		bool           uncompressed;
		FileEntry(const char *newName, const char *newlyChangedName);
		~FileEntry(void);
	};

private:

	char   *treeFileName;
	HANDLE treeFileHandle;
	int    numberOfFiles;
	int    totalFileSize;
	int    totalSmallestSize;
	int    sizeOfTOC;
	int    tocCompressorID;
	int    blockCompressorID;
	int    duplicateCount;
	int    sizeOfNameBlock;
	int    uncompSizeOfNameBlock;
	std::vector<FileEntry*> responseFileOrder;
	std::vector<FileEntry*> tocOrder;
	std::vector<const char *> fileNameBlock;

private:

	void write(const void *data, int length);
	void writeTableOfContents();
	void writeFileNameBlock();
	void writeMd5Block();
	void writeFile(FileEntry *fileEntry);
	void compressAndWrite(const byte * uncompressed, int &sizeOfData, const int uncompSize, int &compressor, const bool isFileData, const bool disableCompression, Md5::Value *md5sum);

public:

	TreeFileBuilder(const char *fileName);
	~TreeFileBuilder(void);
	void createFile(void);
	void addFile(const char *diskFileNameEntry, const char *treeFileNameEntry, bool changedFileName, bool uncompressedFile);
	void addResponseFile(const char *responseFileEntry);
	void write(void);
	static bool LessFileEntryCrcNameCompare(const FileEntry* a, const FileEntry* b);
};

// ======================================================================

#endif
