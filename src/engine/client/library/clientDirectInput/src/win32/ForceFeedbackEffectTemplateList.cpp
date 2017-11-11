// ======================================================================
//
// ForceFeedbackEffectTemplateList.cpp
// Copyright Sony Online Entertainment
//
// ======================================================================

#include "clientDirectInput/FirstClientDirectInput.h"
#include "clientDirectInput/ForceFeedbackEffectTemplateList.h"

#include "clientDirectInput/ConfigClientDirectInput.h"
#include "clientDirectInput/ForceFeedbackEffectTemplate.h"
#include "sharedFile/TreeFile.h"
#include "sharedFoundation/Crc.h"
#include "sharedFoundation/ExitChain.h"

#include <map>
#include <string>

// ======================================================================

namespace ForceFeedbackEffectTemplateListNamespace
{
	struct NamedListImpl
	{
		typedef std::map<uint32, ForceFeedbackEffectTemplate *> Map;
		Map * m_map;
	};

	int cms_bufferSize = 512;
	char cms_buffer[512];

	bool ms_installed;
	NamedListImpl ms_forceFeedbackEffectsList;
}

using namespace ForceFeedbackEffectTemplateListNamespace;

// ======================================================================

void ForceFeedbackEffectTemplateList::install()
{
	DEBUG_FATAL(ms_installed, ("ForceFeedbackEffectTemplateList::install() - The ForceFeedbackEffectTemplateList has already been installed."));
	ms_installed = true;
	ms_forceFeedbackEffectsList.m_map = new NamedListImpl::Map;
	ExitChain::add(remove, "ForceFeedbackEffectTemplateList");
}

// ----------------------------------------------------------------------

void ForceFeedbackEffectTemplateList::remove()
{
	DEBUG_FATAL(!ms_installed, ("ForceFeedbackEffectTemplateList::remove() - The ForceFeedbackEffectTemplateList has not been installed."));

	ForceFeedbackEffectTemplate::deleteAllQueuedDeletes();

	//report if we leaked ForceFeedback templates
#ifdef _DEBUG
	const size_t entryCount = ms_forceFeedbackEffectsList.m_map->size();
	DEBUG_WARNING(entryCount, ("leaked %u ForceFeedbackEffectTemplates", entryCount));
	for(NamedListImpl::Map::const_iterator i = ms_forceFeedbackEffectsList.m_map->begin(); i != ms_forceFeedbackEffectsList.m_map->end(); ++i)
	{
		DEBUG_WARNING(entryCount, ("leaked ForceFeedbackEffectTemplate with crc[%d]", i->first));
	}
#endif

	delete ms_forceFeedbackEffectsList.m_map;
	ms_forceFeedbackEffectsList.m_map = 0;

	ms_installed = false;
}

// ----------------------------------------------------------------------

ForceFeedbackEffectTemplate const * ForceFeedbackEffectTemplateList::fetch(std::string const & fileName, bool fireAndForget)
{
	DEBUG_FATAL(!ms_installed, ("ForceFeedbackEffectTemplateList::fetch() - not installed"));
	if(!ConfigClientDirectInput::getEnableForceFeedback())
		return NULL;

	bool const pathResult = TreeFile::getPathName(fileName.c_str(), cms_buffer, cms_bufferSize);
	if(pathResult)
	{
		uint32 const crc = Crc::calculate(cms_buffer);

		//search for the ForceFeedback template
		ForceFeedbackEffectTemplate * forceFeedbackEffectTemplate = NULL;
		NamedListImpl::Map::const_iterator const it = ms_forceFeedbackEffectsList.m_map->find (crc);
		if (it != ms_forceFeedbackEffectsList.m_map->end ()) //lint !e1702 //operator is both
		{
			forceFeedbackEffectTemplate = (*it).second; //lint !e1702 //operator is both
		}
		else
		{
			//load the file
			forceFeedbackEffectTemplate = new ForceFeedbackEffectTemplate(cms_buffer, fireAndForget);
			addNamedForceFeedbackEffectTemplate(forceFeedbackEffectTemplate);
		}
		//up the reference count
		return fetch(forceFeedbackEffectTemplate);
	}
	else
		return NULL;
}

// ----------------------------------------------------------------------

ForceFeedbackEffectTemplate const * ForceFeedbackEffectTemplateList::fetch(ForceFeedbackEffectTemplate const * const forceFeedbackEffectTemplate)
{
	DEBUG_FATAL(!ms_installed, ("ForceFeedbackEffectTemplateList::fetch() - ForceFeedbackEffectTemplateList not installed."));
	if(!ConfigClientDirectInput::getEnableForceFeedback())
		return NULL;

	if (forceFeedbackEffectTemplate)
		forceFeedbackEffectTemplate->fetch();
	return forceFeedbackEffectTemplate;
}

// ----------------------------------------------------------------------

void ForceFeedbackEffectTemplateList::release(ForceFeedbackEffectTemplate const * const forceFeedbackEffectTemplate)
{
	DEBUG_FATAL(!ms_installed, ("ForceFeedbackEffectTemplateList::release() - ForceFeedbackEffectTemplateList not installed."));

	if (forceFeedbackEffectTemplate)
	{
		forceFeedbackEffectTemplate->release();
	}
}

// ----------------------------------------------------------------------

void ForceFeedbackEffectTemplateList::addNamedForceFeedbackEffectTemplate(ForceFeedbackEffectTemplate * const forceFeedbackEffectTemplate)
{
	DEBUG_FATAL(!ms_installed, ("ForceFeedbackEffectTemplateList::addNamedForceFeedbackEffectTemplate() - ForceFeedbackEffectTemplateList not installed."));
	DEBUG_FATAL(!forceFeedbackEffectTemplate, ("ForceFeedbackEffectTemplate may not be NULL"));

	NamedListImpl::Map::value_type value(forceFeedbackEffectTemplate->getCrc(), forceFeedbackEffectTemplate);
	std::pair<NamedListImpl::Map::iterator, bool> result = ms_forceFeedbackEffectsList.m_map->insert(value);

	//make sure it's not already there
	DEBUG_FATAL(!result.second, ("tried to add an existing named ForceFeedbackEffectTemplate"));
	UNREF(result);
}

// ----------------------------------------------------------------------

bool ForceFeedbackEffectTemplateList::removeForceFeedbackEffectTemplate(ForceFeedbackEffectTemplate * const forceFeedbackEffectTemplate)
{
	DEBUG_FATAL(!ms_installed, ("ForceFeedbackEffectTemplateList::removeForceFeedbackEffectTemplate() - ForceFeedbackEffectTemplateList not installed."));
	DEBUG_FATAL(!forceFeedbackEffectTemplate, ("ForceFeedbackEffectTemplate may not be NULL"));

	bool const inNamedList = ms_forceFeedbackEffectsList.m_map->erase (forceFeedbackEffectTemplate->getCrc ()) == 1;
	return inNamedList;
}

// ======================================================================
