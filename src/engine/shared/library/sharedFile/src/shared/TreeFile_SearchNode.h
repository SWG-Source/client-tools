// ======================================================================
//
// TreeFile_SearchNode.h
// Portions copyright 1998 Bootprint Entertainment
// Portions copyright 2001-2002 Sony Online Entertainment
// All Rights Reserved.
//
// ======================================================================

#ifndef INCLUDED_TreeFile_SearchNode_H
#define INCLUDED_TreeFile_SearchNode_H

// ======================================================================

class CrcString;
class MemoryBlockManager;

#include "sharedFile/TreeFile.h"
#include "sharedFile/FileStreamer.h"
#include "sharedSynchronization/Mutex.h"

#include <string>
#include <unordered_set>
#include "../../../../../../engine/shared/library/sharedFoundation/include/public/sharedFoundation/LessPointerComparator.h"
#include "../../../../../../engine/shared/library/sharedFoundation/include/public/sharedFoundation/Os.h"
#include "../../../../../../engine/shared/library/sharedFoundation/include/public/sharedFoundation/Tag.h"

// ======================================================================

/* This module defines the inner classes of Treefile, to help keep TreeFile.h clean and easy to read.
   Since all of these classes are private, only TreeFile.h/cpp needs to even know these files exist.
*/

class TreeFile::SearchNode
{
public:

	explicit SearchNode(int priority);
	virtual ~SearchNode();

	int                   getPriority() const;

	virtual void          debugPrint() = 0;
	virtual bool          exists(const char *fileName, bool &deleted) const = 0;
	virtual int           getFileSize(const char *fileName, bool &deleted) const = 0;
	virtual void          getPathName(const char *fileName, char *pathName, int pathNameLength) const = 0;
	virtual AbstractFile *open(const char *fileName, AbstractFile::PriorityType priority, bool &deleted) = 0;

private:

	SearchNode();
	SearchNode(const SearchNode &);
	SearchNode &operator =(const SearchNode &);

private:

	const int m_priority;
};

// ======================================================================

inline int TreeFile::SearchNode::getPriority() const
{
	return m_priority;
}

// ======================================================================

class TreeFile::SearchPath : public TreeFile::SearchNode
{
public:

	SearchPath(int priority, const char *path);
	virtual ~SearchPath();

	virtual void          debugPrint();
	virtual bool          exists(const char *fileName, bool &deleted) const;
	virtual int           getFileSize(const char *fileName, bool &deleted) const;
	virtual void          getPathName(const char *fileName, char *pathName, int pathNameLength) const;
	virtual AbstractFile *open(const char *fileName, AbstractFile::PriorityType priority, bool &deleted);

	const char           *getPathName() const; //lint !e1411  // Warning -- member with different signature hides virtual member (bug in PC-Lint, incorrect warning)

	// Clear the negative-cache entry (and insert into a built manifest) for one
	// fixed-up engine-relative name -- the hook TreeFile::forgetMissingFile
	// broadcasts after something WRITES a loose file mid-session.
	void forgetMissing(const char *fileName) const;

	// 2026-08-15: Release-visible A/B probe (compare-the-numbers telemetry).
	// One REPORT_LOG line per loose node at exit: how many REAL filesystem
	// probes ran vs how many the manifest / negative cache answered from
	// memory. A/B recipe = same binary, one session with
	// [SharedFile] searchPathFileManifest=false (pre-fix behavior) vs default
	// on, same route -- realProbes is the number the fix exists to shrink.
	void reportProbeCounters() const;

private:

	SearchPath();
	SearchPath(const SearchPath &);
	SearchPath &operator =(const SearchPath &);

	void makeAbsolutePath(const char *fileName, char *buffer) const;
	bool cachedMissing(const char *fileName) const;
	void noteMissing(const char *fileName) const;
	bool mayContain(const char *fileName) const;

private:

	char  *m_pathName;
	int    m_pathNameLength;

	// CONSULT-59 deferred item (2026-07-06): loose-searchPath stat-storm fix. Nearly every
	// TreeFile::open resolves inside a TRE/TOC, but loose SearchPath nodes sit ABOVE the TOCs
	// (stage/override must keep winning), so each open first paid one CreateFileA kernel
	// round-trip MISS per loose path -- the watchdog-sampled dominant cost of the world-entry
	// load path. Cache the misses per node: a fixed-up name that missed once is answered from
	// this set without touching the disk. Misses ONLY -- an existing file never enters the set,
	// so override-wins semantics are unchanged. Consequence: a loose file dropped into the
	// directory mid-session stays invisible for names already probed (restart the client, or
	// set [SharedFile] searchPathNegativeCache=false). Leaf mutex -- nodes run OUTSIDE
	// TreeFile::ms_criticalSection via the snapshot walk, concurrently from the main and
	// asynchronous-loader threads.
	mutable std::unordered_set<std::string> m_missingFiles;
	mutable Mutex m_missingFilesMutex;
	mutable int   m_missingFilesHits;

	// 2026-08-15 (cold-singles perf arc): loose-directory FILE MANIFEST -- the
	// first-touch completion of the negative cache above. The miss cache only
	// helps names probed BEFORE; a zone preload or a novel in-world asset still
	// paid one real CreateFileA/GetFileAttributes MISS per loose path per NEW
	// name (watchdog-sampled 643ms space-preload exists() storm + the loose-probe
	// prefix of every cold-single texture open, 2026-08-15). On first probe the
	// node enumerates its directory tree ONCE (lowercase/forward-slash relative
	// names -- the fixUpFileName convention) and every later probe answers
	// absent-from-manifest with ZERO syscalls, first-touch included. Misses that
	// slip past a stale manifest still land in m_missingFiles (self-healing for
	// mid-session deletions); forgetMissing() INSERTS into a built manifest so a
	// freshly written loose file becomes visible without a restart. A loose
	// file added mid-session by anything else stays invisible for this node
	// (same restart-or-disable caveat as the miss cache; kill switch
	// [SharedFile] searchPathFileManifest=false). Guarded by
	// m_missingFilesMutex (one leaf lock per node; the build runs under it once,
	// blocking a concurrent prober for the walk's few ms at first touch only).
	mutable std::unordered_set<std::string> m_manifest;
	mutable bool m_manifestBuilt;
	mutable int  m_manifestSkips;
	mutable int  m_realProbes;   // real syscall-backed probes (exists/getFileSize/open reaching the disk)
};

// ======================================================================

inline const char *TreeFile::SearchPath::getPathName() const
{
	return m_pathName;
}

// ======================================================================

class TreeFile::SearchAbsolute : public TreeFile::SearchNode
{
public:

	explicit SearchAbsolute(int priority);
	virtual ~SearchAbsolute();

	virtual void          debugPrint();
	virtual bool          exists(const char *fileName, bool &deleted) const;
	virtual int           getFileSize(const char *fileName, bool &deleted) const;
	virtual void          getPathName(const char *fileName, char *pathName, int pathNameLength) const;
	virtual AbstractFile *open(const char *fileName, AbstractFile::PriorityType priority, bool &deleted);

private:

	SearchAbsolute();
	SearchAbsolute(const SearchAbsolute &);
	SearchAbsolute &operator =(const SearchAbsolute &);
};

// ======================================================================

class TreeFile::SearchTree : public TreeFile::SearchNode
{
	// these friend classes are from the treefile tool
	friend class TOC;
	friend class FileEntry;
	friend class TreeFileBuilder;
	friend class TreeFileExtractor;

public:

	static bool        validate(const char *fileName);
	static bool        isCompressed(int compressorIndex);

public:

	SearchTree(int priority, const char *fileName);
	virtual ~SearchTree();

	virtual void          debugPrint();
	virtual bool          exists(const char *fileName, bool &deleted) const;
	virtual int           getFileSize(const char *fileName, bool &deleted) const;
	virtual void          getPathName(const char *fileName, char *pathName, int pathNameLength) const;
	virtual AbstractFile *open(const char *fileName, AbstractFile::PriorityType priority, bool &deleted);

private:

	// disabled
	SearchTree();

	// disabled
	SearchTree(const SearchTree &);

	// disabled
	SearchTree &operator =(const SearchTree &);

private:

	bool localExists(const char *fileName, int *index, bool &deleted) const;

private:

	enum CompressorType
	{
		CT_none,
		CT_deprecated,
		CT_zlib,
		CT_max
	};

	struct Header
	{
		Tag    token;
		Tag    version;
		uint32 numberOfFiles;
		uint32 tocOffset;
		uint32 tocCompressor;
		uint32 sizeOfTOC;
		uint32 blockCompressor;
		uint32 sizeOfNameBlock;
		uint32 uncompSizeOfNameBlock;
	};

public:

	struct TableOfContentsEntry
	{
		uint32 crc;
		int    length;
		int    offset;
		int    compressor;
		int    compressedLength;
		int    fileNameOffset;
	};

private:

	char                   *m_treeFileName;
	FileStreamer::File     *m_treeFile;
	uint32                  m_version;
	int                     m_numberOfFiles;
	char                   *m_fileNames;
	TableOfContentsEntry   *m_tableOfContents;
};

// ======================================================================

inline bool TreeFile::SearchTree::isCompressed(int compressorIndex)
{
	DEBUG_FATAL(compressorIndex == CT_deprecated, ("No longer supported compressor"));
	return compressorIndex != static_cast<int>(CT_none);
}

// ======================================================================

class TreeFile::SearchTOC : public TreeFile::SearchNode
{
public:

	static bool        validate(const char *fileName);
	static bool        isCompressed(int compressorIndex);

public:

	SearchTOC(int priority, const char *fileName);
	virtual ~SearchTOC();

	virtual void          debugPrint();
	virtual bool          exists(const char *fileName, bool &deleted) const;
	virtual int           getFileSize(const char *fileName, bool &deleted) const;
	virtual void          getPathName(const char *fileName, char *pathName, int pathNameLength) const;
	virtual AbstractFile *open(const char *fileName, AbstractFile::PriorityType priority, bool &deleted);

private:

	// disabled
	SearchTOC();

	// disabled
	SearchTOC(const SearchTOC &);

	// disabled
	SearchTOC &operator =(const SearchTOC &);

private:

	bool localExists(const char *fileName, int *index) const;

private:

	enum CompressorType
	{
		CT_none,
		CT_deprecated,
		CT_zlib,
		CT_max
	};

	struct Header
	{
		Tag    token;
		Tag    version;
		uint8  tocCompressor;
		uint8  fileNameBlockCompressor;
		uint8  unusedOne;
		uint8  unusedTwo;
		uint32 numberOfFiles;
		uint32 sizeOfTOC;
		uint32 sizeOfNameBlock;
		uint32 uncompSizeOfNameBlock;
		uint32 numberOfTreeFiles;
		uint32 sizeOfTreeFileNameBlock;
	};

public:

	// unused entry is there to verify 32 bit word alignment
	struct TableOfContentsEntry
	{
		uint8  compressor;
		uint8  unused;
		uint16 treeFileIndex;
		uint32 crc;
		uint32 fileNameOffset;
		uint32 offset;
		uint32 length;
		uint32 compressedLength;
	};

private:

	char                   *m_TOCFileName;
	FileStreamer::File     *m_TOCFile;
	FileStreamer::File     **m_treeFiles;
	uint32                 m_numberOfTreeFiles;
	uint32                 m_numberOfFiles;
	char                   *m_treeFileNames;
	char                   **m_treeFileNamePointers;
	TableOfContentsEntry   *m_tableOfContents;
	char                   *m_fileNames;
};

// ======================================================================

inline bool TreeFile::SearchTOC::isCompressed(int compressorIndex)
{
	DEBUG_FATAL(compressorIndex == CT_deprecated, ("No longer supported compressor"));
	return compressorIndex != static_cast<int>(CT_none);
}

// ======================================================================

class TreeFile::SearchCache : public TreeFile::SearchNode
{
public:

	explicit SearchCache(int priority);
	virtual ~SearchCache();

	int                   addCachedFile(char const * fileName);

	virtual void          debugPrint();
	virtual bool          exists(char const * fileName, bool & deleted) const;
	virtual int           getFileSize(char const * fileName, bool & deleted) const;
	virtual void          getPathName(char const * fileName, char * pathName, int pathNameLength) const;
	virtual AbstractFile *open(char const * fileName, AbstractFile::PriorityType priority, bool & deleted);

private:

	SearchCache();
	SearchCache(const SearchCache &);
	SearchCache &operator =(const SearchCache &);

private:

	class CachedFile;
	typedef stdmap<CrcString const *, CachedFile *, LessPointerComparator>::fwd CachedFileMap;
	CachedFileMap * const m_cachedFileMap;

	// CONSULT-55 (2026-07-01): m_cachedFileMap is populated by the MAIN thread across many zone-in
	// frames (CachedFileManager::preloadSomeAssets) while the AsynchronousLoader BACKGROUND thread
	// reads it via TreeFile::open->SearchCache::open. Concurrent std::map find/insert is UB (torn
	// red-black rebalance) -> a fetched file lands on the wrong/torn node -> wrong bytes -> the
	// intermittent "Unknown shader template tag" crash (worse after a rebuild: cold caches stretch the
	// insert stream across more frames, widening the window). This leaf mutex serializes ONLY the map
	// find/insert; disk I/O + decompress + createAbstractFile stay OUTSIDE it (never held while another
	// lock is taken -> strict leaf, no deadlock with TreeFile::ms_criticalSection / async ms_mutex).
	mutable Mutex m_cachedFileMapMutex;
};

// ======================================================================

#endif
