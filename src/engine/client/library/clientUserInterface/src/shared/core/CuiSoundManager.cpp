//======================================================================
//
// CuiSoundManager.cpp
// copyright (c) 2002 Sony Online Entertainment
//
//======================================================================

#include "clientUserInterface/FirstClientUserInterface.h"
#include "clientUserInterface/CuiSoundManager.h"

#include "clientAudio/Audio.h"
#include "sharedUtility/DataTable.h"
#include "sharedUtility/DataTableManager.h"
#include <map>

//======================================================================

namespace CuiSoundManagerNamespace
{
	const std::string emptyString;

	bool s_installed = false;

	typedef stdmap<std::string, std::string>::fwd PathMap;
	PathMap s_pathMap;

	void install ()
	{
		DEBUG_FATAL (s_installed, (""));

		s_installed = true;

		static const std::string table_name ("datatables/player/sounds.iff");
		const DataTable * const table = DataTableManager::getTable (table_name, true);
		if (table)
		{
			static const std::string planet_default ("default");
			const int numRows = table->getNumRows ();

			for (int i = 0; i < numRows; ++i)
			{
				const std::string & name   = table->getStringValue (0, i);
				const std::string & path   = table->getStringValue (1, i);

				s_pathMap.insert (std::make_pair (name, path));
			}
			DataTableManager::close (table_name);
		}
		else
			WARNING (true, ("could not load data table [%s]", table_name.c_str ()));
	}
}

using namespace CuiSoundManagerNamespace;

//----------------------------------------------------------------------

void CuiSoundManager::play (const std::string & name)
{
	if (!s_installed)
		install ();

	const std::string & path = findSoundPath (name);

	if (!path.empty ())
	{
		Audio::playSound (path.c_str (), NULL);
	}
	else
		WARNING (true, ("CuiSoundManager: No path exists for sound name [%s]", name.c_str ()));
}

//----------------------------------------------------------------------

void CuiSoundManager::restart(const std::string & name)
{
	if (!s_installed)
		install ();

	const std::string & path = findSoundPath (name);

	if (!path.empty ())
	{
		Audio::restartSound (path.c_str (), NULL);
	}
	else
		WARNING (true, ("CuiSoundManager: No path exists for sound name [%s]", name.c_str ()));
}

//----------------------------------------------------------------------

const std::string & CuiSoundManager::findSoundPath (const std::string & name)
{
	if (!s_installed)
		install ();

	if (!name.empty ())
	{
		const PathMap::const_iterator it = s_pathMap.find (name);

		if (it != s_pathMap.end ())
			return (*it).second;
	}

	return emptyString;
}

//======================================================================
