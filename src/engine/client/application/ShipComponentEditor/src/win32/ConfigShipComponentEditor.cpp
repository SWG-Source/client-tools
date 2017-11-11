//======================================================================
//
// ConfigShipComponentEditor.cpp
// copyright (c) 2004 Sony Online Entertainment
//
//======================================================================

#include "FirstShipComponentEditor.h"
#include "ConfigShipComponentEditor.h"

#include "sharedFoundation/ConfigFile.h"
#include "UnicodeUtils.h"

//======================================================================

namespace ConfigShipComponentEditorNamespace
{
	std::string s_serverPathDsrc;
	std::string s_serverPathData;
	std::string s_sharedPathDsrc;
	std::string s_sharedPathData;
	std::string s_serverTemplateCrcStringTable;
	std::string s_clientDataFilePathDsrc;

	std::string s_textEditorPath;

	std::string s_toolPath;
	std::string s_branchName;

	void testFolder(std::string const & folderName, std::string const & path, std::string & result)
	{
		std::string const & trimmed = Unicode::getTrim(path);
		if (!trimmed.empty() && trimmed[trimmed.size() - 1] == '/')
		{
			result += std::string(folderName + " (ends in /)=[") + path + "\n"; 
			return;
		}

		QDir const qdir(QString(path.c_str()));
		if (!qdir.exists())
		{
			result += std::string(folderName + "=[") + path + "]\n";
		}
	}

	void testFile(std::string const & folderName, std::string const & path, std::string & result)
	{
		QFile const qfile(QString(path.c_str()));
		if (!qfile.exists())
		{
			result += std::string(folderName + "=[") + path + "]\n";
		}
	}

}

using namespace ConfigShipComponentEditorNamespace;

//----------------------------------------------------------------------

//#define KEY_INT(a,b)     (s_ ## a = ConfigFile::getKeyInt("ShipComponentEditor", #a,  b))
//#define KEY_BOOL(a,b)    (s_ ## a = ConfigFile::getKeyBool("ShipComponentEditor", #a, b))
//#define KEY_FLOAT(a,b)   (s_ ## a = ConfigFile::getKeyFloat("ShipComponentEditor", #a, b))
#define KEY_STRING(a,b)  (s_ ## a = ConfigFile::getKeyString("ShipComponentEditor", #a, b))

//----------------------------------------------------------------------

void ConfigShipComponentEditor::install()
{
	KEY_STRING(serverPathDsrc, "c:/projects/swg/x1/dsrc/sku.0/sys.server/compiled/game");
	KEY_STRING(serverPathData, "c:/projects/swg/x1/data/sku.0/sys.server/compiled/game");
	KEY_STRING(sharedPathDsrc, "c:/projects/swg/x1/dsrc/sku.0/sys.shared/compiled/game");
	KEY_STRING(sharedPathData, "c:/projects/swg/x1/data/sku.0/sys.shared/compiled/game");
	KEY_STRING(serverTemplateCrcStringTable, "c:/projects/swg/x1/data/sku.0/sys.server/built/game/misc/object_template_crc_string_table.iff");
	KEY_STRING(clientDataFilePathDsrc, "c:/projects/swg/x1/dsrc/sku.0/sys.client/compiled/game");
	KEY_STRING(textEditorPath, "TextPad");
	KEY_STRING(toolPath, "c:/projects/swg/x1/tools/");
	KEY_STRING(branchName, "x1");

}

//----------------------------------------------------------------------

bool ConfigShipComponentEditor::testFolders(std::string & result)
{
	testFolder("Server Path (dsrc)", s_serverPathDsrc, result);
	testFolder("Server Path (data)", s_serverPathData, result);
	testFolder("Shared Path (dsrc)", s_sharedPathDsrc, result);
	testFolder("Shared Path (data)", s_sharedPathData, result);
	testFolder("ClientDataFile Path (dsrc)", s_clientDataFilePathDsrc, result);

	
	testFile("Server Template Crc StringTable", s_serverTemplateCrcStringTable, result);

	return result.empty();
}

//----------------------------------------------------------------------

std::string const & ConfigShipComponentEditor::getServerPathDsrc()
{
	return s_serverPathDsrc;
}

//----------------------------------------------------------------------

std::string const & ConfigShipComponentEditor::getServerPathData()
{
	return s_serverPathData;
}

//----------------------------------------------------------------------

std::string const & ConfigShipComponentEditor::getSharedPathDsrc()
{
	return s_sharedPathDsrc;
}

//----------------------------------------------------------------------

std::string const & ConfigShipComponentEditor::getSharedPathData()
{
	return s_sharedPathData;
}

//----------------------------------------------------------------------

std::string const & ConfigShipComponentEditor::getServerTemplateCrcStringTable()
{
	return s_serverTemplateCrcStringTable;
}

//----------------------------------------------------------------------

std::string const & ConfigShipComponentEditor::getClientDataFilePathDsrc()
{
	return s_clientDataFilePathDsrc;
}

//----------------------------------------------------------------------

std::string const & ConfigShipComponentEditor::getTextEditorPath()
{
	return s_textEditorPath;
}

//----------------------------------------------------------------------

std::string const & ConfigShipComponentEditor::getToolPath()
{
	return s_toolPath;
}

//----------------------------------------------------------------------

std::string const & ConfigShipComponentEditor::getBranchName()
{
	return s_branchName;
}

//======================================================================
