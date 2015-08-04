// ======================================================================
//
// ClientDataTemplateList.cpp
// asommers
//
// copyright 2003, sony online entertainment
//
// ======================================================================

#include "clientGame/FirstClientGame.h"
#include "clientGame/ClientDataTemplateList.h"

#include "clientGame/ClientDataFile.h"
#include "sharedDebug/DebugFlags.h"
#include "sharedDebug/InstallTimer.h"
#include "sharedFoundation/DataResourceList.h"

// ======================================================================

namespace ClientDataTemplateListNamespace
{
	bool ms_logFetch;
}

using namespace ClientDataTemplateListNamespace;

// ======================================================================

typedef ClientDataFile ClientDataTemplate;
typedef DataResourceList<ClientDataTemplate> ClientDataTemplateListDataResourceList;

ClientDataTemplateListDataResourceList::CreateDataResourceMap *ClientDataTemplateListDataResourceList::ms_bindings = NULL;
ClientDataTemplateListDataResourceList::LoadedDataResourceMap *ClientDataTemplateListDataResourceList::ms_loaded = NULL;

// ======================================================================

void ClientDataTemplateList::install()
{
	InstallTimer const installTimer("ClientDataTemplateList::install");

	ClientDataTemplateListDataResourceList::install ();

	DebugFlags::registerFlag(ms_logFetch, "ClientGame/ClientDataTemplateList", "logFetch");
}

// ----------------------------------------------------------------------

void ClientDataTemplateList::remove ()
{
	ClientDataTemplateListDataResourceList::remove ();

	DebugFlags::unregisterFlag(ms_logFetch);
}

// ----------------------------------------------------------------------

void ClientDataTemplateList::assignBinding (const Tag tag, CreateDataResourceFunction createFunction)
{
	ClientDataTemplateListDataResourceList::registerTemplate (tag, createFunction);
}

// ----------------------------------------------------------------------

void ClientDataTemplateList::removeBinding (const Tag tag)
{
	IGNORE_RETURN (ClientDataTemplateListDataResourceList::removeBinding (tag));
}

// ----------------------------------------------------------------------

const ClientDataTemplate* ClientDataTemplateList::fetch (const std::string& filename)
{
	DEBUG_REPORT_LOG(ms_logFetch, ("[fetch] ClientDataTemplateList::fetch: %s\n", filename.c_str()));
	return ClientDataTemplateListDataResourceList::fetch(filename);
}

// ----------------------------------------------------------------------

const ClientDataTemplate* ClientDataTemplateList::fetch (const char* filename)
{
	DEBUG_REPORT_LOG(ms_logFetch, ("[fetch] ClientDataTemplateList::fetch: %s\n", filename));
	return ClientDataTemplateListDataResourceList::fetch (filename);
}

// ----------------------------------------------------------------------

const ClientDataTemplate* ClientDataTemplateList::fetch (const CrcString& filename)
{
	DEBUG_REPORT_LOG(ms_logFetch, ("[fetch] ClientDataTemplateList::fetch: %s\n", filename.getString()));
	return ClientDataTemplateListDataResourceList::fetch (filename);
}

// ----------------------------------------------------------------------

void ClientDataTemplateList::release (const ClientDataTemplate& dataResource)
{
	ClientDataTemplateListDataResourceList::release (dataResource);
}

// ----------------------------------------------------------------------

void ClientDataTemplateList::garbageCollect ()
{
	ClientDataTemplateListDataResourceList::garbageCollect ();
}

//----------------------------------------------------------------------

