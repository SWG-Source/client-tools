// ======================================================================
//
// ClientEffectTemplateList.cpp
// copyright 2002 Sony Online Entertainment
//
// ======================================================================

#include "clientGame/FirstClientGame.h"
#include "clientGame/ClientEffectTemplateList.h"

#include "sharedDebug/DebugFlags.h"
#include "sharedDebug/InstallTimer.h"
#include "sharedFile/Iff.h"
#include "sharedFile/Treefile.h"
#include "sharedFoundation/CrcLowerString.h"
#include "sharedFoundation/ExitChain.h"
#include "sharedFoundation/LessPointerComparator.h"

#include "clientGame/ClientEffectTemplate.h"

#include <map>
#include <vector>

// ======================================================================

bool ClientEffectTemplateList::ms_installed;
ClientEffectTemplateList::TemplateMap *ClientEffectTemplateList::ms_templates;

// ----------------------------------------------------------------------

/** Install a ClientEffectTemplateList and prepare it for use.
 *  @post all data structures associated with this object are allocated
 *  @remove() has been added to the exitchain, remove() need not be called directly
 */
void ClientEffectTemplateList::install()
{
	InstallTimer const installTimer("ClientEffectTemplateList::install");

	DEBUG_FATAL(ms_installed, ("ClientEffectTemplateList already installed"));

	ms_installed = true;

	ms_templates = new TemplateMap();

	ExitChain::add(remove, "ClientEffectTemplateList::remove");
}

// ----------------------------------------------------------------------

/** Remove a ClientEffectTemplateList.
 *  @post all data structures associated with this object have been deallocated
 */
void ClientEffectTemplateList::remove(void)
{
	DEBUG_FATAL(!ms_installed, ("ClientEffectTemplateList not installed"));

	//-- handle leaks
	if (!ms_templates->empty())
	{
		DEBUG_WARNING(true, ("ClientEffectTemplateList: [%u] ClientEffectTemplate leaks.", ms_templates->size()));

		const TemplateMap::iterator endIt = ms_templates->end();
		for (TemplateMap::iterator it     = ms_templates->begin(); it != endIt; ++it)
		{
			ClientEffectTemplate* const clientEffectTemplate = it->second;
			DEBUG_REPORT_LOG(clientEffectTemplate, ("--leaked [%s], refcount=[%d]\n", clientEffectTemplate->getCrcName().getString(), clientEffectTemplate->getReferenceCount()));
			UNREF(clientEffectTemplate);
			// Leak this stuff anyway --- we really need to know where the leaks are coming from.  This auto-cleanup hides it.
			// delete clientEffectTemplate;
		}
	}

	delete ms_templates;
	ms_templates = 0;

	ms_installed = false;
}

// ----------------------------------------------------------------------

/** Fetch a ClientEffectTemplate by name.  We track the template until it release()'s itself
 *  @return pointer to a cached template if already loaded, pointer to a newly allocated template if found but not loaded, or NULL if the file can not be found
 */
const ClientEffectTemplate* ClientEffectTemplateList::fetch(const CrcLowerString& effectTemplateName)
{
	DEBUG_FATAL(!ms_installed, ("ClientEffectTemplateList not installed"));

	if (effectTemplateName == CrcLowerString::empty)
	{
		WARNING (true, ("ClientEffectTemplateList::fetch called with empty filename"));
		return 0;
	}

	ClientEffectTemplate* clientEffectTemplate = NULL;

	//check if we've got this one loaded
	TemplateMap::iterator lowerBoundResult = ms_templates->lower_bound(&effectTemplateName);
	if ((lowerBoundResult != ms_templates->end()) && !ms_templates->key_comp()(&effectTemplateName, lowerBoundResult->first))
	{
		//it's loaded, return it
		clientEffectTemplate = lowerBoundResult->second;
	}
	else
	{
		//it's not available

		//ensure sure file exists
		if (!TreeFile::exists(effectTemplateName.getString()))
		{
			WARNING(true, ("specified ClientEffectTemplate file [%s] does not exist", effectTemplateName.getString()));
			return NULL;
		}

		//load it
		Iff iff(effectTemplateName.getString());
		clientEffectTemplate = new ClientEffectTemplate(iff, effectTemplateName);

		//track it
		IGNORE_RETURN(ms_templates->insert(lowerBoundResult, TemplateMap::value_type(&clientEffectTemplate->getCrcName(), clientEffectTemplate)));
	}

	//we should have a valid ClientEffectTemplate here
	NOT_NULL(clientEffectTemplate);

	//bump up reference count
	clientEffectTemplate->fetch();

	return clientEffectTemplate;
}

// ----------------------------------------------------------------------

/** Short-circuit command to load a ClientEffectTemplate, and create a ClientEffect from it.
 *  This call creates a client effect based on a world position.
 *  @return pointer to a newly allocated client effect, or NULL if the template name can not be found
 */
ClientEffect* ClientEffectTemplateList::createClientEffect(const CrcLowerString& effectTemplateName, const CellProperty* cell, const Vector& position, const Vector& up)
{
	DEBUG_FATAL(!ms_installed, ("ClientEffectTemplateList not installed"));

	//fetch the ClientEffectTemplate
	const ClientEffectTemplate* const clientEffectTemplate = fetch(effectTemplateName);
	if(!clientEffectTemplate)
		return NULL;

	//create the ClientEffect
	ClientEffect* clientEffect = clientEffectTemplate->createClientEffect(cell, position, up);

	//-- Release local references.
	clientEffectTemplate->release();

	return clientEffect;
}

// ----------------------------------------------------------------------

/** Short-circuit command to load a ClientEffectTemplate, and create a ClientEffect from it.
 *  This call creates a client effect attached to an object.
 *  @return pointer to a newly allocated client effect, or NULL if the template name can not be found
 */
ClientEffect* ClientEffectTemplateList::createClientEffect(const CrcLowerString& effectTemplateName, Object* object, const CrcLowerString& hardPointName)
{
	DEBUG_FATAL(!ms_installed, ("ClientEffectTemplateList not installed"));

	//fetch the ClientEffectTemplate
	const ClientEffectTemplate* const clientEffectTemplate = fetch(effectTemplateName);
	if(!clientEffectTemplate)
		return NULL;

	//create the ClientEffect
	ClientEffect* clientEffect = clientEffectTemplate->createClientEffect(object, hardPointName);

	//-- Release local references.
	clientEffectTemplate->release();

	return clientEffect;
}

//----------------------------------------------------------------------

ClientEffect * ClientEffectTemplateList::createClientEffect(const CrcLowerString& filename, Object* object, Transform const & transform)
{
	DEBUG_FATAL(!ms_installed, ("ClientEffectTemplateList not installed"));

	//fetch the ClientEffectTemplate
	const ClientEffectTemplate * const clientEffectTemplate = fetch(filename);
	if(!clientEffectTemplate)
		return NULL;

	//create the ClientEffect
	ClientEffect * const clientEffect = clientEffectTemplate->createClientEffect(object, transform);

	//-- Release local references.
	clientEffectTemplate->release();

	return clientEffect;
}

// ----------------------------------------------------------------------

/** Stop tracking the given ClientEffectTemplate.  This should only be called from within ClientEffectTemplate::release()
 *  @post the template is not longer tracked by the templatelist
 */
void ClientEffectTemplateList::stopTracking(const ClientEffectTemplate *clientEffectTemplate)
{
	DEBUG_FATAL(!ms_installed, ("ClientEffectTemplateList not installed"));

	NOT_NULL(clientEffectTemplate);
	if(!clientEffectTemplate)
		return;

	const CrcLowerString &filename = clientEffectTemplate->getCrcName();

	TemplateMap::iterator lowerBoundResult = ms_templates->lower_bound(&filename);
	if ((lowerBoundResult == ms_templates->end()) || ms_templates->key_comp()(&filename, lowerBoundResult->first))
	{
		//we don't have this ClientEffectTemplate in our list
		return;
	}

	//remove the ClientEffectTemplate from the container
	ms_templates->erase(lowerBoundResult);
}

// ======================================================================
