// ======================================================================
//
// main.cpp
// copyright (c) 2005 Sony Online Entertainment
//
// ======================================================================

#include "FirstLocalizationToolCon.h"
#include <cstdio>
#include <string>
#include "fileInterface/StdioFile.h"
#include "LocalizedStringTableReaderWriter.h"
#include "UnicodeUtils.h"

//----------------------------------------------------------------------
//-- this is a SWG hack to work with our modified STLPORT

enum MemoryManagerNotALeak
{
	MM_notALeak
};

#ifdef WIN32
#define CDECL __cdecl
#else
#define CDECL
#endif//WIN32

void * CDECL operator new(size_t size, MemoryManagerNotALeak)
{
	return operator new(size);
}
//-- end SWG hack to work with our modified STLPORT
//----------------------------------------------------------------------

int runBatchMode(int argc, char ** argv)
{
	
	LocalizedStringTableRW * table = 0;
	bool dirty = false;

	for (int i = 1; i < argc; ++i)
	{
		const char* arg = argv[i];

		if (arg[0] == '-')
		{
			if (table == 0)
			{
				printf("No file specified.\n");
				return -3;
			}
			std::string argName(arg);
			if (argName == "-list")
			{
				const LocalizedStringTable::Map_t& map = table->getMap();
				const LocalizedStringTable::NameMap_t& nameMap = table->getNameMap();

				LocalizedStringTable::NameMap_t::const_iterator i;
				LocalizedStringTable::Map_t::const_iterator j;

				for (i = nameMap.begin(); i != nameMap.end(); ++i)
				{
					j = map.find(i->second);

					if (j != map.end())
					{
						LocalizedString* str = j->second;
						Unicode::UTF8String s = Unicode::wideToUTF8(str->getString());
						printf("%s -> %s\n", i->first.c_str(), s.c_str());
					}
				}
				break;
			}

			else if (argName == "-set")
			{
				if (i + 2 >= argc)
				{
					printf("Insufficient arguments supplied to 'set'\n");
					return -4;
				}

				const char* tag = argv[++i];
				const char* val = argv[++i];

				Unicode::String value;
				value = Unicode::narrowToWide(val);

				Unicode::NarrowString name(tag);

				table->removeStringByName(name);

				Unicode::NarrowString defname;

				table->addString(value, defname);
				table->rename(defname, name);

				dirty = true;
				break;
			}

			else if (argName == "-find")
			{
				if (i + 1 >= argc)
				{
					printf("Insufficient arguments supplied to 'find'\n");
					return -5;
				}

				const char* tag = argv[++i];

				Unicode::NarrowString name(tag);

				LocalizedString * str = table->getLocalizedStringByName(name);

				if (str)
				{
					Unicode::UTF8String s = Unicode::wideToUTF8(str->getString());
					printf("%s -> %s\n", name.c_str(), s.c_str());
				}
				else
				{
					return -2;
				}
				break;
			}

			else if (argName == "-delete")
			{
				if (i + 1 >= argc)
				{
					printf("Insufficient arguments supplied to 'delete'\n");
					return -6;
				}

				const char* tag = argv[++i];

				Unicode::NarrowString name(tag);

				table->removeStringByName(name);

				dirty = true;
				break;
			}
		}
		else
		{
			StdioFileFactory fileFactory;

			if (table)
			{
				if (dirty)
				{
					bool ret = table->writeRW(fileFactory, table->getFileName());

					if (!ret)
					{
						printf("Could not write file %s\n", table->getFileName().c_str());
						delete table;
						return -1;
					}
				}
				delete table;
			}

			std::string fixedFilename (arg);

			const int len = static_cast<int>(fixedFilename.size ());
			for (int i = 0; i <  len; ++i)
			{
				if (fixedFilename [i] == '\\')
					fixedFilename [i] = '/';
			}


			bool fileExists = false;
			{
				StdioFile theFile(fixedFilename.c_str(),"rb");
				if(theFile.isOpen())
				{
					fileExists = true;
				}
			}

			/*
			// create it if it doesn't exist
			if(!fileExists) 
			{
				LocalizedStringTableRW newTable(fixedFilename);
				bool ret = newTable.writeRW(fileFactory, newTable.getFileName());

				if (!ret)
				{
					printf("Could not write file %s\n", table->getFileName().c_str());
					delete table;
					return -1;
				}
			}
			*/


			if(fileExists)
			{
				table = LocalizedStringTableRW::loadRW (fileFactory, fixedFilename);
				dirty = false;

				if (table == 0)
				{
					printf("could not load file %s\n", fixedFilename.c_str());
					return -1;
				}
			}
		}
	}

	if (table)
	{
		if (dirty)
		{
			StdioFileFactory fileFactory;
			bool ret = table->writeRW(fileFactory, table->getFileName());

			if (!ret)
			{
				printf("Could not write file %s\n", table->getFileName().c_str());
				delete table;
				return -1;
			}
		}

		delete table;
	}


	return 0;
}
	


void displayUsage()
{
	printf("\n\n");
	printf("LocalizationToolCon [filename] [-command] <commandArgs>\n");
	printf("where [-command] is:\n");
	printf("-list (list contents of file)\n");
	printf("-set [tag] [value] (sets contents of tag to value)\n");
	printf("-find [tag] (finds and prints out tag and it's associated value)\n");
	printf("-delete [tag] (deletes tag and it's associated value)\n");
	printf("\n\n");
	printf("returned error conditions:\n");
	printf(" 0 == success\n");
	printf("-1 == file i/0 error\n");
	printf("-2 == no entry found on -find\n");
	printf("-3 == first argument was not a file\n");
	printf("-4 == Insufficient arguments supplied to 'set\n");

}

int main (int argc, char ** argv)
{
	if(argc == 1)
	{
		displayUsage();
		return 0;
	}

	for (int i = 1; i < argc; ++i)
	{
		if (argv[i][0] == '-')
			return runBatchMode(argc,argv);
	}

}

//-----------------------------------------------------------------

void *operator new[](size_t size)
{
	return operator new(size);
}

//-----------------------------------------------------------------

void operator delete[](void *pointer)
{
	operator delete(pointer);
}

//-----------------------------------------------------------------
