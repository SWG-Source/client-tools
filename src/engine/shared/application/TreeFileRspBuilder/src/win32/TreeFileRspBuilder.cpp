//===================================================================
//
// TreeFileRspBuilder.cpp
// asommers
//
// copyright 2001, sony online entertainment
//
//===================================================================

#include "FirstTreeFileRspBuilder.h"

//===================================================================

namespace
{
	typedef std::multimap<CString, CString> StringMap;
	StringMap ms_stringMap;

	typedef std::map<CString, CString> DataMap;
	DataMap ms_uncompressedMusicMap;
	DataMap ms_uncompressedSampleMap;
	DataMap ms_compressedTextureMap;
	DataMap ms_compressedAnimationMap;
	DataMap ms_compressedMeshSkeletalMap;
	DataMap ms_compressedMeshStaticMap;
	DataMap ms_compressedOtherMap;

	struct Bucket
	{
	public:

		bool     m_ext;
		CString  m_key;
		int      m_keySize;
		DataMap* m_dataMap;

	public:

		Bucket (const bool ext, const char* const key, DataMap* const dataMap) :
			m_ext (ext),
			m_key (key),
			m_keySize (m_key.GetLength ()),
			m_dataMap (dataMap)
		{
		}
	};

	typedef std::vector<Bucket> TreeFileList;
	TreeFileList ms_treeFileList;
}

//===================================================================

static void parseCommonCfg (const CString& name)
{
	//-- open the config file
	CStdioFile infile (name, CFile::modeRead | CFile::typeText);

	//-- read each line...
	CString line;
	while (infile.ReadString (line))
	{
		//-- see if the line is empty
		line.TrimLeft ();
		line.TrimRight ();
		if (line.GetLength () == 0)
			continue;

		//-- see if the first character is a comment
		int index = line.Find ("#");
		if (index == 0)
			continue;

		//-- see if the first character is a comment
		index = line.Find (";");
		if (index == 0)
			continue;

		//-- find the =
		index = line.Find ("=");
		if (index == -1)
			continue;

		//-- left half goes in key, right half goes in value
		const int     length = line.GetLength ();
		const CString left   = line.Left (index);
		const CString right  = line.Right (length - index - 1);

		//-- add to list
		index = left.Find ("searchPath");
		if (index == -1)
			continue;

		StringMap::iterator i = ms_stringMap.begin ();
		for ( ; i != ms_stringMap.end (); ++i)
			if (i->second == right)
				break;

		if (i == ms_stringMap.end ())
			ms_stringMap.insert (StringMap::value_type (left, right));
	}
}

//===================================================================

static void generateFiles (const CString& explicitDirectory, const CString& entryDirectory)
{
	const int explicitLength = explicitDirectory.GetLength ();
	if (explicitLength == 0)
		return;

	const int entryLength = entryDirectory.GetLength ();

	CString searchMask;
	searchMask.Format ("%s%s*.*", explicitDirectory, (explicitLength > 1 && explicitDirectory [explicitLength-1] != '/') ? "/" : "");

	CFileFind finder;
	BOOL working = finder.FindFile (searchMask);
	while (working)
	{
		working = finder.FindNextFile();

		if (!finder.IsDots ())
		{
			CString explicitName;
			explicitName.Format ("%s%s%s", explicitDirectory, (explicitLength > 1 && explicitDirectory [explicitLength-1] != '/') ? "/" : "", finder.GetFileName ());

			CString entryName;
			entryName.Format ("%s%s%s", entryDirectory, (entryLength > 1 && entryDirectory [entryLength-1] != '/' ? "/" : ""), finder.GetFileName ());

			if (finder.IsDirectory ())
			{
				generateFiles (explicitName, entryName);
			}
			else
			{
				size_t i;
				for (i = 0; i < ms_treeFileList.size (); ++i)
				{
					bool found = i == ms_treeFileList.size () - 1;
					if (!found)
					{
						if (ms_treeFileList [i].m_ext)
							found = entryName.Find (ms_treeFileList [i].m_key) != -1;
						else
							found = _strnicmp (entryName, ms_treeFileList [i].m_key, ms_treeFileList [i].m_keySize) == 0;
					}

					if (found)
					{
						DataMap& dataMap = *ms_treeFileList [i].m_dataMap;
						DataMap::iterator iterator = dataMap.find (entryName);

						if (iterator == dataMap.end ())
							dataMap [entryName] = explicitName;
						else
							printf ("Duplicate found: %s\n", entryName);

						break;
					}
				}
			}
		}
	}
}

//===================================================================

static void writeRsp (const DataMap& dataMap, const CString& name)
{
	//-- open the config file
	CStdioFile outfile (name, CFile::modeWrite | CFile::modeCreate | CFile::typeText);

	//-- write each line...
	DataMap::const_iterator end = dataMap.end ();
	for (DataMap::const_iterator iterator = dataMap.begin (); iterator != end; ++iterator)
	{
		CString line;
		line.Format ("%s @ %s\n", iterator->first, iterator->second);

		outfile.WriteString (line);
	}
}

//===================================================================

void main (int argc, char* argv [])
{
	if (argc < 2)
	{
		printf ("TreeFileRspBuilder configName\n");

		return;
	}

	parseCommonCfg (argv [1]);

	ms_treeFileList.push_back (Bucket (true, ".mp3", &ms_uncompressedMusicMap));
	ms_treeFileList.push_back (Bucket (true, ".wav", &ms_uncompressedSampleMap));
	ms_treeFileList.push_back (Bucket (true, ".dds", &ms_compressedTextureMap));
	ms_treeFileList.push_back (Bucket (true, ".ans", &ms_compressedAnimationMap));
	ms_treeFileList.push_back (Bucket (true, ".mgn", &ms_compressedMeshSkeletalMap));
	ms_treeFileList.push_back (Bucket (true, ".msh", &ms_compressedMeshStaticMap));
	ms_treeFileList.push_back (Bucket (true, "", &ms_compressedOtherMap));
	
	{
		for (StringMap::reverse_iterator i = ms_stringMap.rbegin (); i != ms_stringMap.rend (); ++i)
		{
			printf ("%s -> %s\n", i->first, i->second);
			generateFiles (i->second, "");
		}
	}

	writeRsp (ms_uncompressedMusicMap, "data_uncompressed_music.rsp");
	writeRsp (ms_uncompressedSampleMap, "data_uncompressed_sample.rsp");
	writeRsp (ms_compressedTextureMap, "data_compressed_texture.rsp");
	writeRsp (ms_compressedAnimationMap, "data_compressed_animation.rsp");
	writeRsp (ms_compressedMeshStaticMap, "data_compressed_mesh_static.rsp");
	writeRsp (ms_compressedMeshSkeletalMap, "data_compressed_mesh_skeletal.rsp");
	writeRsp (ms_compressedOtherMap, "data_compressed_other.rsp");
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
