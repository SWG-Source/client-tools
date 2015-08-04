// ======================================================================
//
// SoundTemplateList.cpp
// Copyright Sony Online Entertainment
//
// ======================================================================

#include "clientAudio/FirstClientAudio.h"
#include "clientAudio/SoundTemplateList.h"

#include "clientAudio/SoundTemplate.h"
#include "clientAudio/Sound2dTemplate.h"
#include "clientAudio/Sound3dTemplate.h"
#include "sharedDebug/DataLint.h"
#include "sharedFoundation/CrcString.h"
#include "sharedFoundation/LessPointerComparator.h"
#include "sharedFoundation/TemporaryCrcString.h"
#include "sharedFile/Iff.h"
#include "sharedFile/TreeFile.h"

#include <cstdio>
#include <map>
#include <set>
#include <string>
#include <vector>

// ======================================================================

//std::string SoundTemplateList::ms_defaultSoundTemplateName("Sound\\defaultSound.msh");

// ======================================================================

namespace SoundTemplateListNamespace
{
	typedef std::vector<SoundTemplate const *>  PreloadedTemplates;

	//-- the private implementation of the list of Tag->CreateFunction bindings

	struct BindImpl
	{
		typedef std::map<Tag, SoundTemplateList::CreateFunction> Map;
		Map * m_map;
	};

	//-- the private implementation of the named list of Sound templates

	struct NamedListImpl
	{
		typedef std::map<const CrcString *, SoundTemplate *, LessPointerComparator> Map;
		Map * m_map;
	};

	typedef std::set<SoundTemplate*> AnonymousContainer;

	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

	bool                s_installed;
	NamedListImpl       s_namedList;
	BindImpl            s_bindImpl;
	AnonymousContainer *s_anonymousList;
	PreloadedTemplates  s_preloadedTemplates;

#ifdef _DEBUG
	bool                s_debugReportFlag;
	typedef std::set<std::string> StringSet;
	StringSet           s_warningSet;
#endif // _DEBUG
}

using namespace SoundTemplateListNamespace;

// ======================================================================
// Install the SoundTemplateList
//
// Remarks:
//
//   If the default bindings are added, the routine will add all the engine
//   supported SoundTemplate types to the Iff.

void SoundTemplateList::install()
{
	DEBUG_FATAL(s_installed, ("SoundTemplateList::install() - The SoundTemplateList has already been installed."));

	s_installed = true;

	s_namedList.m_map = new NamedListImpl::Map;
	s_bindImpl.m_map = new BindImpl::Map;
	s_anonymousList = new AnonymousContainer();

	Sound2dTemplate::install();
	Sound3dTemplate::install();
}

// ----------------------------------------------------------------------
/**
 * Remove a SoundTemplateList.
 */

void SoundTemplateList::remove(void)
{
	DEBUG_FATAL(!s_installed, ("SoundTemplateList::remove() - The SoundTemplateList has not been installed."));

	Sound2dTemplate::remove();
	Sound3dTemplate::remove();
	releasePreloadedAssets();

	{
		//-- report if we leaked anonymous Sound templates

		NOT_NULL(s_anonymousList);
		const size_t entryCount = s_anonymousList->size();
		DEBUG_WARNING(entryCount, ("leaked %u anonymous Sound templates", entryCount));
		UNREF(entryCount);

		const AnonymousContainer::const_iterator itEnd = s_anonymousList->end();
		for (AnonymousContainer::iterator it = s_anonymousList->begin(); it != itEnd; ++it)
		{
			//-- Report the leak.
			SoundTemplate *const at = *it;
			DEBUG_REPORT_LOG(true, ("--leaked [<anonymous>], refcount=[%d]\n", at->getReferenceCount()));
			UNREF(at);

			//-- Don't delete these, just leak them.  We can't guarantee that we can delete them without FATALs or exceptions at this point in shutdown code.
			// delete at;
		}

		delete s_anonymousList;
		s_anonymousList = 0;
	}

	{
		//-- report if we leaked named Sound templates
#ifdef _DEBUG
		const size_t entryCount = s_namedList.m_map->size();
		DEBUG_WARNING(entryCount, ("leaked %u named Sound templates", entryCount));
#endif

		const NamedListImpl::Map::const_iterator itEnd = s_namedList.m_map->end();
		for (NamedListImpl::Map::iterator it = s_namedList.m_map->begin(); it != itEnd; ++it)
		{
			//-- Report the leak.
			SoundTemplate *const at = NON_NULL((*it).second);
			DEBUG_REPORT_LOG(true, ("--leaked [%s], refcount=[%d]\n", at->getCrcName().getString(), at->getReferenceCount()));
			UNREF(at);

			//-- Don't delete these, just report and leak them.  We can't guarantee that we can delete them without FATALs or exceptions at this point in shutdown code.
			// delete at;
		}

		delete s_namedList.m_map;
		s_namedList.m_map = 0;
	}

	delete s_bindImpl.m_map;
	s_bindImpl.m_map = 0;

	s_installed = false;
}

// ----------------------------------------------------------------------

void SoundTemplateList::debugReport()
{
	DEBUG_REPORT_PRINT(true, ("SoundTemplates loaded:\n"));

	DEBUG_OUTPUT_STATIC_VIEW_BEGINFRAME("Engine\\SoundTemplateList");
	DEBUG_OUTPUT_STATIC_VIEW("Engine\\SoundTemplateList", ("SoundTemplates loaded:\n"));

	for (NamedListImpl::Map::const_iterator it = s_namedList.m_map->begin (); it != s_namedList.m_map->end (); ++it) //lint !e1702 //operator is both
	{
		const SoundTemplate * const t = (*it).second; //lint !e1702 //operator is both
		DEBUG_REPORT_PRINT(true, ("  %s %d\n", t->m_crcName.getString(), t->m_referenceCount));
		DEBUG_OUTPUT_STATIC_VIEW("Engine\\SoundTemplateList", ("  %s %d\n", t->m_crcName.getString(), t->m_referenceCount));
		UNREF(t);
	}

	DEBUG_OUTPUT_STATIC_VIEW_ENDFRAME("Engine\\SoundTemplateList");
}

// ----------------------------------------------------------------------
/**
 * Assign a binding between a Tag and a CreateFunction.
 *
 * If a previous binding for the specified tag exists, it will be replaced
 * by the new binding.
 *
 * @param tag  Tag for the binding
 * @param createFunction  Create function to call for data files with the tag
 */

void SoundTemplateList::assignBinding(Tag tag, CreateFunction createFunction)
{
	DEBUG_FATAL(!s_installed, ("not installed"));
	DEBUG_FATAL(!createFunction, ("createFunction may not be NULL"));

	(*s_bindImpl.m_map) [tag] = createFunction;
}

// ----------------------------------------------------------------------
/**
 * Remove a binding from the SoundTemplateList.
 *
 * If the specified Tag is not bound, the routine will return without
 * error.
 *
 * @param tag  Tag for the binding to remove
 */

void SoundTemplateList::removeBinding(Tag tag)
{
	IGNORE_RETURN (s_bindImpl.m_map->erase (tag));
}

// ----------------------------------------------------------------------

void SoundTemplateList::preload(const char *fileName)
{
	if (TreeFile::exists(fileName))
	{
		SoundTemplate const * soundTemplate = fetch(fileName);

		if (soundTemplate != NULL)
		{
			s_preloadedTemplates.push_back(soundTemplate);
		}
		else
		{
			DEBUG_WARNING(true, ("Unable to preload asset: %s", fileName));
		}
	}
}

// ----------------------------------------------------------------------

void SoundTemplateList::releasePreloadedAssets()
{
	while (!s_preloadedTemplates.empty())
	{
		release(s_preloadedTemplates.back());
		s_preloadedTemplates.pop_back();
	}
}

// ----------------------------------------------------------------------
/**
 * Fetch an SoundTemplate by name.
 *
 * This routine will increment the reference count of the SoundTemplate
 * by one.
 *
 * If the createFromFileFunction is NULL (as per default), the SoundTemplate
 * file will be opened up into an iff and will call the create function bound to
 * the iff data's first block name.  If the createFromFileFunction is non-null,
 * that function will be called with the filename should the SoundTemplate
 * need to be loaded.
 *
 * @param fileName  Name of the SoundTemplate to load,
 * @param createFromFileFunction  function used to load the file directly from filename
 * @param context  user context pointer for createFromFile function
 */

const SoundTemplate *SoundTemplateList::fetch(const char *fileName, CreateFromFileFunction createFromFileFunction, void *context)
{
	if (!Audio::isEnabled())
	{
		return NULL;
	}

	DEBUG_FATAL(!s_installed, ("SoundTemplateList::fetch() - not installed"));

	TemporaryCrcString const crcString(fileName, true);

	// search for the Sound
	SoundTemplate *soundTemplate = 0;

	{
		const NamedListImpl::Map::const_iterator it = s_namedList.m_map->find (&crcString);

		if (it != s_namedList.m_map->end ()) //lint !e1702 //operator is both
		{
			soundTemplate = (*it).second; //lint !e1702 //operator is both

			// up the reference count
			return fetch(soundTemplate);
		}
	}

	// see if we need to load the Sound template

	if (!createFromFileFunction)
	{
		// load from iff

		Iff iff;

		if (!iff.open(fileName, true))
		{
#ifdef _DEBUG
			char text[1024];
			sprintf(text, "SoundTemplateList::fetch() - File not found: %s", fileName);

			if (s_warningSet.find(text) == s_warningSet.end())
			{
				IGNORE_RETURN(s_warningSet.insert(text));
				WARNING(true, (text));
			}
#endif // _DEBUG

			return NULL;
		}

		const Tag tag = iff.getCurrentName();

#ifdef _DEBUG
		char tagString[5];
		ConvertTagToString(tag, tagString);
#endif // DEBUG

		const BindImpl::Map::const_iterator it = s_bindImpl.m_map->find (tag);

		if (it != s_bindImpl.m_map->end ()) //lint !e1702 //operator is both
		{
#ifdef _DEBUG
			DataLint::pushAsset(fileName);
#endif // _DEBUG

			soundTemplate = (*it).second (fileName, iff); //lint !e1702 //operator is both

#ifdef _DEBUG
			DataLint::popAsset();
#endif // _DEBUG
		}
		else
		{
#ifdef _DEBUG
			DEBUG_FATAL(true, ("SoundTemplateList::fetch() - Binding not found: %s", tagString));
#endif // DEBUG
			return NULL; //lint !e527 // unreachable
		}
	}
	else
	{
#ifdef _DEBUG
		DataLint::pushAsset(fileName);
#endif // _DEBUG

		// load from create function
		soundTemplate = createFromFileFunction(fileName, context);

#ifdef _DEBUG
		DataLint::popAsset();
#endif // _DEBUG
	}

	addNamedSoundTemplate(soundTemplate);

	// up the reference count
	return fetch(soundTemplate);
}

// ----------------------------------------------------------------------
/**
 * Add a reference to the specified Sound.
 *
 * This routine will do nothing if passed in NULL.  Otherwise, it will
 * increase the reference count of the specified SoundTemplate
 * by one.
 *
 * @param SoundTemplate  SoundTemplate to remove the reference count from
 * @return The SoundTemplate that was passed into the routine.
 * @see SoundTemplateList::release()
 */

const SoundTemplate *SoundTemplateList::fetch(const SoundTemplate *soundTemplate)
{
	if (soundTemplate)
		++soundTemplate->m_referenceCount;

	return soundTemplate;
}

// ----------------------------------------------------------------------
/**
 * Fetch an SoundTemplate from an Iff.
 *
 * This routine will increment the reference count of the SoundTemplate
 * by one.
 *
 * @param iff  File from which to load the SoundTemplate
 */

const SoundTemplate *SoundTemplateList::fetch(Iff *iff)
{
	DEBUG_FATAL(!iff, ("null iff arg"));

	const Tag tag = iff->getCurrentName();

	const BindImpl::Map::const_iterator it = s_bindImpl.m_map->find (tag);

	if (it == s_bindImpl.m_map->end ()) //lint !e1702 //operator is both
	{
		DEBUG_FATAL(true, ("binding not found"));
		return NULL; //lint !e527 // unreachable
	}

#ifdef _DEBUG
	DataLint::pushAsset(iff->getFileName());
#endif // _DEBUG

	SoundTemplate *soundTemplate = (*it).second (NULL, *iff); //lint !e1702 //operator is both
	NOT_NULL (soundTemplate);

#ifdef _DEBUG
	DataLint::popAsset();
#endif // _DEBUG

	addAnonymousSoundTemplate(soundTemplate);

	// up the reference count
	return fetch(soundTemplate);
}

// ----------------------------------------------------------------------
/**
 * Fetch an SoundTemplate from an existing object and add the object
 * to the SoundTemplateList.
 *
 * The SoundTemplate object is assumed to be created outside the
 * SoundTemplateList fetch-from-file or fetch-from-iff mechanism.
 * This may happen within an editor that creates an SoundTemplate
 * object from scratch.  In this case, the object is created outside
 * the SoundTemplateList and must be added to the list.  Use this
 * function to do so.  Failure to add the object to this
 * list likely will cause failures when an Sound making use of the
 * SoundTemplate object gets released.
 *
 * @param SoundTemplate  the SoundTemplate object to add to the SoundTemplateList
 *
 * @return                    the SoundTemplate object, returned just like all the other fetch routines
 */

const SoundTemplate *SoundTemplateList::fetchNew(SoundTemplate *soundTemplate)
{
	NOT_NULL(soundTemplate);

	//-- up the reference count
	IGNORE_RETURN(fetch(soundTemplate));

	const PersistentCrcString &crcName = soundTemplate->getCrcName();
	const char *crcString = crcName.getString();

	if (crcString && *crcString)
	{
		// we've got a named Sound template list
		addNamedSoundTemplate(soundTemplate);
	}
	else
	{
		// we've got an anonymous Sound template list
		addAnonymousSoundTemplate(soundTemplate);
	}

	return soundTemplate;
}

// ----------------------------------------------------------------------
/**
 * Remove a reference to the specified SoundTemplate.
 *
 * This routine will do nothing if passed in NULL.
 *
 * If the reference count drops to 0, the SoundTemplate will be deleted.
 *
 * @param SoundTemplate  SoundTemplate to remove the reference count from
 * @see SoundTemplateList::release()
 */

void SoundTemplateList::release(const SoundTemplate *soundTemplate)
{
	if (soundTemplate && --soundTemplate->m_referenceCount <= 0)
	{
		DEBUG_FATAL(soundTemplate->m_referenceCount < 0, ("referenceCount is negative"));

		// remove from list
		removeSoundTemplate(const_cast<SoundTemplate *>(soundTemplate));

		// free its resources
		delete const_cast<SoundTemplate *>(soundTemplate);
	}
}

// ----------------------------------------------------------------------
/**
 * Add an anonymous Sound template to the SoundTemplateList.
 *
 * @param SoundTemplate  SoundTemplate to add
 */

void SoundTemplateList::addAnonymousSoundTemplate(SoundTemplate *soundTemplate)
{
	DEBUG_FATAL(!s_installed, ("not installed"));
	DEBUG_FATAL(!soundTemplate, ("SoundTemplate may not be NULL"));

	//-- add to anonymous list
	NOT_NULL(s_anonymousList);
	std::pair<AnonymousContainer::iterator, bool> result = s_anonymousList->insert(soundTemplate);

	// make sure it's not already there
	DEBUG_FATAL(!result.second, ("tried to add an existing anonymous SoundTemplate"));
	UNREF(result);
}

// ----------------------------------------------------------------------
/**
 * Add a named Sound template to the SoundTemplateList.
 *
 * @param SoundTemplate  SoundTemplate to add
 */

void SoundTemplateList::addNamedSoundTemplate(SoundTemplate *soundTemplate)
{
	DEBUG_FATAL(!s_installed, ("not installed"));
	DEBUG_FATAL(!soundTemplate, ("SoundTemplate may not be NULL"));

	//-- add to anonymous list
	NamedListImpl::Map::value_type value(&(soundTemplate->getCrcName()), soundTemplate);
	std::pair<NamedListImpl::Map::iterator, bool> result = s_namedList.m_map->insert(value);

	// make sure it's not already there
	DEBUG_FATAL(!result.second, ("tried to add an existing named SoundTemplate"));
	UNREF(result);
}

// ----------------------------------------------------------------------
/**
 * Remove an Sound template from the SoundTemplateList.
 *
 * @param SoundTemplate  SoundTemplate to remove
 */

void SoundTemplateList::removeSoundTemplate(SoundTemplate *soundTemplate)
{
	DEBUG_FATAL(!soundTemplate, ("SoundTemplate may not be NULL"));

	// first check named list
	const bool inNamedList = soundTemplate->getCrcName ().getString () && s_namedList.m_map->erase (&(soundTemplate->getCrcName ())) == 1;
	if (!inNamedList)
	{
		// check anonymous list
		NOT_NULL(s_anonymousList);

		const AnonymousContainer::iterator anonymousIt = s_anonymousList->find(soundTemplate);
		DEBUG_FATAL(anonymousIt == s_anonymousList->end(), ("failed to find Sound template in named or anonymous list [0x%08x]", soundTemplate));

		s_anonymousList->erase(anonymousIt);
	}
}

// ----------------------------------------------------------------------
/**
 * Create an Sound for the specified SoundTemplate.
 *
 * @param fileName  Name of the SoundTemplate
 * @return An Sound for the specified SoundTemplate
 */

Sound2 *SoundTemplateList::createSound(const char *fileName)
{
	// get the Sound template
	const SoundTemplate *soundTemplate = fetch(fileName);
	Sound2 *sound = NULL;

	if (soundTemplate != NULL)
	{
		// creating the Sound will increment the reference count
		sound = soundTemplate->createSound();

		// release the Sound template, since the Sound now holds a ref count for it
		release(soundTemplate);
	}

	return sound;
}

// ======================================================================
