// ============================================================================
//
// DroidProgramSizeManager.cpp
// Copyright Sony Online Entertainment, Inc.
//
// ============================================================================

#include "clientGame/FirstClientGame.h"
#include "clientGame/DroidProgramSizeManager.h"

#include "clientGame/ClientObject.h"
#include "sharedDebug/InstallTimer.h"
#include "sharedFoundation/ExitChain.h"
#include "sharedUtility/DataTable.h"
#include "sharedUtility/DataTableManager.h"
#include <map>
#include <set>

//-----------------------------------------------------------------------------

namespace DroidProgramSizeManagerNamespace
{
	bool ms_installed;
	std::map<std::string, int> ms_droidProgramSizes;
	std::set<std::string> ms_programmableDroids;

	std::string const ms_droidCommandProgramSizeFile("datatables/space_command/droid_program_size.iff");
	std::string const ms_programmableDroidsFile("datatables/space_command/programmable_droids.iff");
}

using namespace DroidProgramSizeManagerNamespace;

// ============================================================================

void DroidProgramSizeManager::install()
{
	InstallTimer const installTimer("DroidProgramSizeManager::install");

	DEBUG_FATAL(ms_installed, ("DroidProgramSizeManager::install() - Already installed."));

	ms_installed = true;

	ExitChain::add(DroidProgramSizeManager::remove, "DroidProgramSizeManager::remove", 0, false);

	ms_droidProgramSizes.clear();
	DataTable * table = DataTableManager::getTable(ms_droidCommandProgramSizeFile, true);
	if(table)
	{
		int const numRows = table->getNumRows ();

		for (int i = 0; i < numRows; ++i)
		{
			std::string const & programName = table->getStringValue("program_name", i);
			int size = table->getIntValue("program_size", i);
			ms_droidProgramSizes[programName] = size;
		}
		DataTableManager::close(ms_droidCommandProgramSizeFile);
		table = NULL;
	}

	ms_programmableDroids.clear();
	table = DataTableManager::getTable(ms_programmableDroidsFile, true);
	if(table)
	{
		int const numRows = table->getNumRows ();

		for (int i = 0; i < numRows; ++i)
		{
			std::string const & templateName = table->getStringValue("object_template", i);
			ms_programmableDroids.insert(templateName);
		}
		DataTableManager::close(ms_programmableDroidsFile);
		table = NULL;
	}
}

//-----------------------------------------------------------------------------

void DroidProgramSizeManager::remove()
{
	ms_droidProgramSizes.clear();
	ms_programmableDroids.clear();
	ms_installed = false;
}

//-----------------------------------------------------------------------------

int DroidProgramSizeManager::getDroidProgramSize(std::string const & droidProgramCommandName)
{
	if(ms_droidProgramSizes.find(droidProgramCommandName) != ms_droidProgramSizes.end())
		return ms_droidProgramSizes[droidProgramCommandName];
	return 0;
}

//-----------------------------------------------------------------------------

bool DroidProgramSizeManager::isDroidProgrammable(ClientObject const & droidControlDevice)
{
	std::string const objectTemplateName = droidControlDevice.getTemplateName();
	return ms_programmableDroids.find(objectTemplateName) != ms_programmableDroids.end();
}

// ============================================================================
