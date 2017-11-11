//
// ConfigGodClient.cpp
//
// copyright 2000, verant interactive
//

//-------------------------------------------------------------------

#include "SwgGodClient/FirstSwgGodClient.h"
#include "ConfigGodClient.h"

#include "sharedFoundation/ConfigFile.h"
#include "sharedNetwork/SetupSharedNetwork.h"

//-------------------------------------------------------------------

ConfigGodClient::Data* ConfigGodClient::data;

//-------------------------------------------------------------------

static ConfigGodClient::Data staticData;

//-------------------------------------------------------------------

namespace ConfigGodClientNamespace
{
	const char* ms_serverObjectTemplateTheaterDirectory = 0;
	const char* ms_serverDataTableTheaterDirectory = 0;
	const char* ms_sharedObjectTemplateTheaterDirectory = 0;
	const char* ms_clientDataFileTheaterDirectory = 0;
	const char* ms_serverObjectTemplateTheaterPath = 0;
	const char* ms_serverDataTableTheaterPath = 0;
	const char* ms_sharedObjectTemplateTheaterPath = 0;
	const char* ms_clientDataFileTheaterPath = 0;
	const char* ms_sharedTerrainDirectory = 0;

	bool  ms_connectToPerforce = true;
	bool  ms_loadServerObjects = true;
}

using namespace ConfigGodClientNamespace;

//-------------------------------------------------------------------

#define KEY_STRING(a,b)(ms_ ## a = ConfigFile::getKeyString("GodClient", #a, b))
#define KEY_BOOL(a,b)(ms_ ## a = ConfigFile::getKeyBool("GodClient", #a, b))
#define KEY_INT2(a,b)(data->a = ConfigFile::getKeyInt("GodClient", #a, b))
#define KEY_STRING2(a,b)(data->a = ConfigFile::getKeyString("GodClient", #a, b))

//-------------------------------------------------------------------

void ConfigGodClient::install()
{
	SetupSharedNetwork::SetupData  networkSetupData;
	SetupSharedNetwork::getDefaultClientSetupData(networkSetupData);
	SetupSharedNetwork::install(networkSetupData);

	data = &staticData;

	KEY_STRING2(scriptClassPath, "//depot/swg/current/data/sku.0/sys.server/compiled/game/script");
	KEY_STRING2(scriptSourcePath, "//depot/swg/current/dsrc/sku.0/sys.server/compiled/game/script");
	KEY_STRING2(scriptEditor, "Textpad.exe");

	KEY_STRING2(templateClientIffPath, "//depot/swg/current/data/sku.0/sys.shared/compiled/game/object");
	KEY_STRING2(templateClientSourcePath, "//depot/swg/current/dsrc/sku.0/sys.shared/compiled/game/object");
	KEY_STRING2(templateServerIffPath, "//depot/swg/current/data/sku.0/sys.server/compiled/game/object");
	KEY_STRING2(templateServerSourcePath, "//depot/swg/current/dsrc/sku.0/sys.server/compiled/game/object");
	KEY_STRING2(templateEditor, "Textpad.exe");

	KEY_STRING2(localClientSrcPath, "c:/work/swg/current/dsrc/sku.0/sys.shared/compiled/game");
	KEY_STRING2(localClientDataPath, "c:/work/swg/current/data/sku.0/sys.shared/compiled/game");
	KEY_STRING2(localServerSrcPath, "c:/work/swg/current/dsrc/sku.0/sys.server/compiled/game");
	KEY_STRING2(localServerDataPath, "c:/work/swg/current/data/sku.0/sys.server/compiled/game");
	KEY_STRING2(localServerCrcStringTable, "c:/work/swg/current/data/sku.0/sys.server/built/game/misc/object_template_crc_string_table.iff");

	KEY_INT2(frameRateLimit, 0);

	KEY_STRING(serverObjectTemplateTheaterDirectory, "../../dsrc/sku.0/sys.server/compiled/game");
	KEY_STRING(serverDataTableTheaterDirectory, "../../dsrc/sku.0/sys.server/compiled/game");
	KEY_STRING(sharedObjectTemplateTheaterDirectory, "../../dsrc/sku.0/sys.shared/compiled/game");
	KEY_STRING(clientDataFileTheaterDirectory, "../../dsrc/sku.0/sys.client/compiled/game");
	KEY_STRING(serverObjectTemplateTheaterPath, "object/building/poi");
	KEY_STRING(serverDataTableTheaterPath, "datatables/poi");
	KEY_STRING(sharedObjectTemplateTheaterPath, "object/building/poi");
	KEY_STRING(clientDataFileTheaterPath, "clientdata/poi");
	KEY_STRING(sharedTerrainDirectory, "../../data/sku.0/sys.shared/built/game/terrain");
	KEY_BOOL(connectToPerforce, true);
	KEY_BOOL(loadServerObjects, true);
}

//-------------------------------------------------------------------

void ConfigGodClient::remove()
{
}

//-------------------------------------------------------------------

const char* ConfigGodClient::getServerObjectTemplateTheaterDirectory()
{
	return ms_serverObjectTemplateTheaterDirectory;
}

//-------------------------------------------------------------------

const char* ConfigGodClient::getServerDataTableTheaterDirectory()
{
	return ms_serverDataTableTheaterDirectory;
}

//-------------------------------------------------------------------

const char* ConfigGodClient::getSharedObjectTemplateTheaterDirectory()
{
	return ms_sharedObjectTemplateTheaterDirectory;
}

//-------------------------------------------------------------------

const char* ConfigGodClient::getClientDataFileTheaterDirectory()
{
	return ms_clientDataFileTheaterDirectory;
}

//-------------------------------------------------------------------

const char* ConfigGodClient::getServerObjectTemplateTheaterPath()
{
	return ms_serverObjectTemplateTheaterPath;
}

//-------------------------------------------------------------------

const char* ConfigGodClient::getServerDataTableTheaterPath()
{
	return ms_serverDataTableTheaterPath;
}

//-------------------------------------------------------------------

const char* ConfigGodClient::getSharedObjectTemplateTheaterPath()
{
	return ms_sharedObjectTemplateTheaterPath;
}

//-------------------------------------------------------------------

const char* ConfigGodClient::getClientDataFileTheaterPath()
{
	return ms_clientDataFileTheaterPath;
}

//-------------------------------------------------------------------

const char* ConfigGodClient::getSharedTerrainDirectory ()
{
	return ms_sharedTerrainDirectory;
}

//-------------------------------------------------------------------

bool ConfigGodClient::getConnectToPerforce ()
{
	return ms_connectToPerforce;
}

//-------------------------------------------------------------------

bool ConfigGodClient::getLoadServerObjects ()
{
	return ms_loadServerObjects;
}

//-------------------------------------------------------------------


