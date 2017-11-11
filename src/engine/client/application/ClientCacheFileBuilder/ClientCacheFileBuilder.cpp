// ======================================================================
//
// ClientCacheFileBuilder.cpp
// asommers
//
// copyright 2003, sony online entertainment
//
// ======================================================================

#include "StdAfx.h"

#include <algorithm>
#include <cstdio>
#include <map>
#include <set>
#include <vector>

// ======================================================================

namespace ClientCacheFileBuilderNamespace
{
	typedef std::map<CString, int> StringMap;
	StringMap ms_stringMap;

	typedef std::vector<CString> StringList;

	typedef std::set<CString> StringSet;
	StringSet ms_smallSet;
	StringSet ms_mediumSet;
	StringSet ms_largeSet;
	StringSet ms_ignoreSet;
	StringSet ms_processSet;

	StringSet ms_preloadAppearanceTemplateSet;
	StringSet ms_preloadClientEffectTemplateSet;
	StringSet ms_preloadPortalPropertyTemplateSet;
	StringSet ms_preloadSoundTemplateSet;

	bool s_verbose = false;

	struct Node
	{
		CString m_fileName;
		int m_offset;
		bool m_cached;
	};

	bool compareNodes(Node const * const a, Node const * const b)
	{
		return a->m_offset < b->m_offset;
	}

	typedef std::vector<Node const *> NodeList;
	typedef std::map<CString, Node> SearchTree;
	typedef std::map<CString, SearchTree *> SearchTreeMap;
	SearchTreeMap ms_searchTreeMap;
}

using namespace ClientCacheFileBuilderNamespace;

// ======================================================================

inline void rejectLine(CString const & line)
{
	if (s_verbose)
	{
		OutputDebugString("rejected: ");
		OutputDebugString(line);
		OutputDebugString("\n");
	}
}

// ======================================================================

inline bool addToCache(CString const & line, CString const & extension, bool const built, bool const character)
{
	size_t const size = ms_ignoreSet.size();

	//-- ignore the following extensions
	if (extension.Compare("cfg") == 0)
	{
		ms_ignoreSet.insert(line);
		return true;
	}

	if (extension.Compare("dds") == 0)
	{
		if (built && character)
		{
			ms_smallSet.insert(line);
			ms_mediumSet.insert(line);
			ms_largeSet.insert(line);
		}
		else
			ms_ignoreSet.insert(line);

		return true;
	}

	if (extension.Compare("flr") == 0)
	{
		ms_ignoreSet.insert(line);
		return true;
	}

	if (extension.Compare("mgn") == 0)
	{
		ms_ignoreSet.insert(line);
		return true;
	}

	if (extension.Compare("mp3") == 0)
	{
		ms_ignoreSet.insert(line);
		return true;
	}

	if (extension.Compare("msh") == 0)
	{
		ms_ignoreSet.insert(line);
		return true;
	}

	if (extension.Compare("wav") == 0)
	{
		ms_ignoreSet.insert(line);
		return true;
	}

	if (extension.Compare("ws") == 0)
	{
		ms_ignoreSet.insert(line);
		return true;
	}

	//-- add the following extensions to all cache files
	if (extension.Compare("cdf") == 0)
	{
		ms_smallSet.insert(line);
		ms_mediumSet.insert(line);
		ms_largeSet.insert(line);

		return true;
	}

	if (extension.Compare("cef") == 0)
	{
		ms_smallSet.insert(line);
		ms_mediumSet.insert(line);
		ms_largeSet.insert(line);

		return true;
	}

	if (extension.Compare("ilf") == 0)
	{
		ms_smallSet.insert(line);
		ms_mediumSet.insert(line);
		ms_largeSet.insert(line);

		return true;
	}

	if (extension.Compare("pal") == 0)
	{
		ms_smallSet.insert(line);
		ms_mediumSet.insert(line);
		ms_largeSet.insert(line);

		return true;
	}

	if (extension.Compare("pob") == 0)
	{
		ms_smallSet.insert(line);
		ms_mediumSet.insert(line);
		ms_largeSet.insert(line);

		return true;
	}

	if (extension.Compare("pst") == 0)
	{
		ms_smallSet.insert(line);
		ms_mediumSet.insert(line);
		ms_largeSet.insert(line);

		return true;
	}

	if (extension.Compare("sfp") == 0)
	{
		ms_smallSet.insert(line);
		ms_mediumSet.insert(line);
		ms_largeSet.insert(line);

		return true;
	}

	if (extension.Compare("snd") == 0)
	{
		ms_smallSet.insert(line);
		ms_mediumSet.insert(line);
		ms_largeSet.insert(line);

		return true;
	}

	if (extension.Compare("stf") == 0)
	{
		ms_smallSet.insert(line);
		ms_mediumSet.insert(line);
		ms_largeSet.insert(line);

		return true;
	}

	if (extension.Compare("trt") == 0)
	{
		ms_smallSet.insert(line);
		ms_mediumSet.insert(line);
		ms_largeSet.insert(line);

		return true;
	}

	if (extension.Compare("ffe") == 0)
	{
		ms_smallSet.insert(line);
		ms_mediumSet.insert(line);
		ms_largeSet.insert(line);

		return true;
	}

	//-- animations are only for medium/large
	if (extension.Compare("ans") == 0)
	{
		ms_mediumSet.insert(line);
		ms_largeSet.insert(line);

		return true;
	}

	//-- only add .iff files if they're in the object directory
	if (extension.Compare("iff") == 0)
	{
		if (line.Find("object/") == 0 || line.Find("abstract/") == 0)
		{
			ms_smallSet.insert(line);
			ms_mediumSet.insert(line);
			ms_largeSet.insert(line);
		}
		else
			ms_ignoreSet.insert(line);

		return true;
	}

	//-- shaders are only for medium/large
	if (line.Find("shader/") == 0)
	{
		ms_mediumSet.insert(line);
		ms_largeSet.insert(line);

		return true;
	}

	//-- ignore any files in the following directories
	if (line.Find("terrain/") == 0)
	{
		ms_ignoreSet.insert(line);
		return true;
	}

	if (line.Find("vertex_program/") == 0)
	{
		ms_ignoreSet.insert(line);
		return true;
	}

	if (line.Find("pixel_program/") == 0)
	{
		ms_ignoreSet.insert(line);
		return true;
	}

	if (line.Find("shared_program/") == 0)
	{
		ms_ignoreSet.insert(line);
		return true;
	}

	if (line.Find("effect/") == 0)
	{
		ms_ignoreSet.insert(line);
		return true;
	}

	if (line.Find("ui/") == 0)
	{
		ms_ignoreSet.insert(line);
		return true;
	}

	//-- all remaining appearances should be valid
	if (line.Find("appearance/") != -1)
	{
		ms_smallSet.insert(line);
		ms_mediumSet.insert(line);
		ms_largeSet.insert(line);

		return true;
	}

	return false;
}

// ======================================================================

static void addToSpace(CString const & line, CString const & extension)
{
	//-- Only add root level assets
	if (line.Find('/', line.Find('/', 0) + 1) != -1)
		return;

	if (extension.Compare("apt") == 0)
		ms_preloadAppearanceTemplateSet.insert(line);

	if (extension.Compare("prt") == 0)
		ms_preloadAppearanceTemplateSet.insert(line);

	if (extension.Compare("cef") == 0)
		ms_preloadClientEffectTemplateSet.insert(line);

	if (extension.Compare("pob") == 0)
		ms_preloadPortalPropertyTemplateSet.insert(line);

	if (extension.Compare("snd") == 0)
		ms_preloadSoundTemplateSet.insert(line);
}

// ======================================================================

static bool loadSearchTree(CString const & searchTreeName)
{
	if (ms_searchTreeMap.find(searchTreeName) != ms_searchTreeMap.end())
	{
		printf("Found duplicate search tree %s\n", searchTreeName);
		return false;
	}

	SearchTree * const searchTree = new SearchTree;
	ms_searchTreeMap.insert(std::make_pair(searchTreeName, searchTree));

	//-- open the config file
	CStdioFile infile;
	if (!infile.Open(searchTreeName, CFile::modeRead | CFile::typeText))
	{
		printf("Could not open search tree %s\n", searchTreeName);
		return false;
	}

	//-- read each line...
	int lineIndex = 0;

	CString line;
	while (infile.ReadString(line))
	{
		++lineIndex;

		//-- see if the line is empty
		line.TrimLeft();
		line.TrimRight();
		if (line.GetLength() == 0)
			continue;

		//-- find the \t
		int index = line.Find("\t");
		if (index == -1)
			continue;

		CString const fileName = line.Left(index);
		int const offset = atoi(line.Right(line.GetLength() - index - 1));
		if (fileName.IsEmpty())
		{
			printf("Found bad file entry in searchTree %s on line %i: %s\n", searchTreeName, lineIndex, line);
			return false;
		}

		Node node;
		node.m_fileName = fileName;
		node.m_offset = offset;
		node.m_cached = false;
		searchTree->insert(std::make_pair(fileName, node));
	}

	printf ("Loaded search tree %s w/ %i files\n", searchTreeName, searchTree->size());

	return true;
}

// ======================================================================

static bool parseConfigFile(CString const & name, CString const & publishDirectory)
{
	printf ("Parsing config file %s\n", name);

	//-- open the config file
	CStdioFile infile(name, CFile::modeRead | CFile::typeText);

	//-- read each line...
	CString line;
	while (infile.ReadString(line))
	{
		//-- see if the line is empty
		line.TrimLeft();
		line.TrimRight();
		if (line.GetLength() == 0)
			continue;

		//-- see if the file is a server file
		int index = line.Find("searchTree");
		if (index == -1)
			continue;

		//-- see if the file is a server file
		index = line.Find("=");
		if (index == -1)
			continue;

		CString searchTree = line.Right(line.GetLength() - index - 1);
		searchTree.Replace(".tre", ".txt");

		CString const prefix = publishDirectory.IsEmpty() ? publishDirectory : (publishDirectory + (publishDirectory[publishDirectory.GetLength() - 1] == '\\' ? "" : "\\"));
		loadSearchTree(prefix + searchTree);
	}

	return true;
}

// ======================================================================

static bool parseFileList(CString const & name)
{
	printf ("Parsing file list %s\n", name);

	//-- open the config file
	CStdioFile infile(name, CFile::modeRead | CFile::typeText);

	//-- read each line...
	CString line;
	while (infile.ReadString(line))
	{
		//-- see if the line is empty
		line.TrimLeft();
		line.TrimRight();
		if (line.GetLength() == 0)
			continue;

		//-- see if the file is a server file
		int index = line.Find("sys.server");
		if (index != -1)
		{
			rejectLine(line);

			continue;
		}

		//-- see if the file is for sku.
		index = line.Find("sku.");
		if (index == -1)
		{
			rejectLine(line);

			continue;
		}

		//-- see if the file is for sku.1
		bool const forSpace = line.Find("sku.1") != -1;

		//-- see if the file is for sys.
		index = line.Find("sys.");
		if (index == -1)
		{
			rejectLine(line);

			continue;
		}

		//-- convert \ to /
		line.Replace('\\', '/');

		bool const built = line.Find("built/") != -1;
		bool const character = line.Find("character/") != -1;

		//-- convert to treefile path name(stripping two directories beyond sys.x)
		index = line.Find('/', index);
		index = line.Find('/', index + 1);
		index = line.Find('/', index + 1);
		line = line.Right(line.GetLength() - index - 1);

		//-- find extension
		index = line.ReverseFind('.');
		if (index == -1)
		{
			rejectLine(line);

			continue;
		}

		line.MakeLower();

		CString const extension(line.Right(line.GetLength() - index - 1));

		//-- Add the line to the space preload list if possible
		if (forSpace)
			addToSpace(line, extension);

		//-- Add the line to one or more cache files if possible
		if (addToCache(line, extension, built, character))
			continue;
		else
			ms_processSet.insert(line);
	}

	return true;
}

// ======================================================================

static bool markCached(CString const & fileName)
{
	for (SearchTreeMap::iterator iter = ms_searchTreeMap.begin(); iter != ms_searchTreeMap.end(); ++iter)
	{
		SearchTree * const searchTree = iter->second;
		SearchTree::iterator jter = searchTree->find(fileName);
		if (jter != searchTree->end())
		{
			jter->second.m_cached = true;

			return true;
		}
	}

	return false;
}

// ======================================================================

static void writeCacheMif (StringSet const & stringSet, CString const & fileName)
{
	printf ("Sorting cache mif %s\n", fileName);

	StringList stringList;
	{
		//-- clear out the "used" flag
		for (SearchTreeMap::iterator iter = ms_searchTreeMap.begin(); iter != ms_searchTreeMap.end(); ++iter)
		{
			SearchTree * const searchTree = iter->second;
			for (SearchTree::iterator jter = searchTree->begin(); jter != searchTree->end(); ++jter)
				jter->second.m_cached = false;
		}

		StringSet::const_iterator end = stringSet.end();
		for(StringSet::const_iterator iterator = stringSet.begin(); iterator != end; ++iterator)
			if (!markCached(*iterator))
				stringList.push_back(*iterator);
	}

	printf ("Writing cache mif %s\n", fileName);

	//-- open the config file
	CStdioFile outfile(fileName, CFile::modeWrite | CFile::modeCreate | CFile::typeText);

	//-- header
	CString header;
	header += "// ======================================================================\n";
	header += "//\n";
	header += "// " + fileName + " GENERATED WITH ClientCacheFileBuilder\n";
	header += "// copyright 2003, sony online entertainment\n";
	header += "//\n";
	header += "// ======================================================================\n";
	header += "\n";
	header += "FORM \"CACH\"\n";
	header += "{\n";
	header += "\tCHUNK \"0000\"\n";
	header += "\t{\n";
	outfile.WriteString(header);

	//-- write each line...
	{
		NodeList nodeList;

		//-- write the search trees
		for (SearchTreeMap::iterator iter = ms_searchTreeMap.begin(); iter != ms_searchTreeMap.end(); ++iter)
		{
			nodeList.clear();

			SearchTree * const searchTree = iter->second;
			if (!searchTree->empty())
			{
				outfile.WriteString("\n\t\t//-- " + iter->first + "\n");

				{
					for (SearchTree::iterator jter = searchTree->begin(); jter != searchTree->end(); ++jter)
						if (jter->second.m_cached)
							nodeList.push_back(&jter->second);

					std::stable_sort(nodeList.begin(), nodeList.end(), compareNodes);
				}

				char buffer[64];
				for (NodeList::iterator jter = nodeList.begin(); jter != nodeList.end(); ++jter)
				{
					Node const * const node = (*jter);
					outfile.WriteString(CString("\t\tcstring \"") + (node->m_fileName + "\"  // " + _itoa(node->m_offset, buffer, 10) + "\n"));
				}
			}
		}

		outfile.WriteString("\n\t\t//-- misc\n");

		//-- write the misc list
		StringList::const_iterator end = stringList.end();
		for(StringList::const_iterator iterator = stringList.begin(); iterator != end; ++iterator)
			outfile.WriteString("\t\tcstring \"" + *iterator + "\"\n");
	}

	CString footer;
	footer += "\t}\n";
	footer += "}\n";
	outfile.WriteString(footer);
}

// ======================================================================

static void writeCacheTxt(StringSet const & stringSet, CString const & fileName)
{
	CStdioFile outfile(fileName, CFile::modeWrite | CFile::modeCreate | CFile::typeText);

	StringSet::const_iterator end = stringSet.end();
	for(StringSet::const_iterator iterator = stringSet.begin(); iterator != end; ++iterator)
		outfile.WriteString(*iterator + "\n");
}

// ======================================================================

static void writePreloadMifSection(CStdioFile & outfile, CString const & section, StringSet const & stringSet)
{
	//-- write each line...
	outfile.WriteString("\t\tFORM \"" + section + "\"\n\t\t{\n\t\t\tCHUNK \"0000\"\n\t\t\t{\n");

	StringSet::const_iterator end = stringSet.end();
	for(StringSet::const_iterator iterator = stringSet.begin(); iterator != end; ++iterator)
		outfile.WriteString("\t\t\t\tcstring \"" + *iterator + "\"\n");

	outfile.WriteString("\t\t\t}\n\t\t}\n\n");
}

// ======================================================================

static void writePreloadMif (CString const & fileName)
{
	printf ("Writing preload mif %s\n", fileName);

	//-- open the config file
	CStdioFile outfile(fileName, CFile::modeWrite | CFile::modeCreate | CFile::typeText);

	//-- header
	CString header;
	header += "// ======================================================================\n";
	header += "//\n";
	header += "// " + fileName + " GENERATED WITH ClientCacheFileBuilder\n";
	header += "// copyright 2003, sony online entertainment\n";
	header += "//\n";
	header += "// ======================================================================\n";
	header += "\n";
	header += "FORM \"SPAM\"\n";
	header += "{\n";
	header += "\tFORM \"0000\"\n";
	header += "\t{\n\n";
	outfile.WriteString(header);

	writePreloadMifSection(outfile, "APPE", ms_preloadAppearanceTemplateSet);
	writePreloadMifSection(outfile, "CLIE", ms_preloadClientEffectTemplateSet);
	writePreloadMifSection(outfile, "PORT", ms_preloadPortalPropertyTemplateSet);
	writePreloadMifSection(outfile, "SOUN", ms_preloadSoundTemplateSet);

	CString footer;
	footer += "\t}\n";
	footer += "}\n";
	outfile.WriteString(footer);
}

// ======================================================================

void main(int argc, char* argv [])
{
	if (argc < 4)
	{
		printf("ClientCacheFileBuilder <config> <publish directory> <rsp> \n");
		printf("  config = config file name with search paths (ie. client.cfg)\n");
		printf("  publish = path to where the treefile .txt files are (d:\\work\\swg\\test\\publish\n");
		printf("  rsp = list of files to consider");

		return;
	}

	if (parseConfigFile(argv[1], argv[2]) && parseFileList(argv[3]))
	{
		writeCacheMif(ms_smallSet, "cache_small.mif");
		writeCacheMif(ms_mediumSet, "cache_medium.mif");
		writeCacheMif(ms_largeSet, "cache_large.mif");
		writeCacheTxt(ms_ignoreSet, "ignore.txt");
		writeCacheTxt(ms_processSet, "process.txt");
		writePreloadMif("space_preload.mif");
	}

	for (SearchTreeMap::iterator iter = ms_searchTreeMap.begin(); iter != ms_searchTreeMap.end(); ++iter)
		delete iter->second;
}

// ======================================================================
