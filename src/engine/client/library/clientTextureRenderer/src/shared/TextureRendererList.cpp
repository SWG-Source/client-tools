// ======================================================================
//
// TextureRendererList.cpp
// copyright 2001 Sony Online Entertainment
// 
// ======================================================================

#include "clientTextureRenderer/FirstClientTextureRenderer.h"
#include "clientTextureRenderer/TextureRendererList.h"

#include "clientTextureRenderer/TextureRendererTemplate.h"
#include "sharedDebug/DataLint.h"
#include "sharedFile/Iff.h"
#include "sharedFile/TreeFile.h"
#include "sharedFoundation/CrcLowerString.h"
#include "sharedFoundation/ExitChain.h"
#include "sharedFoundation/LessPointerComparator.h"
#include "sharedFoundation/PointerDeleter.h"

#include <algorithm>
#include <map>

// ======================================================================

namespace
{
	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

	typedef std::map<const CrcLowerString*, TextureRendererTemplate*, LessPointerComparator>  NamedContainer;
	typedef NamedContainer::iterator                                                          NCIterator;

	typedef std::map<Tag, TextureRendererList::TemplateLoader>  LoaderContainer;
	typedef LoaderContainer::iterator                           LCIterator;
	
	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

#ifdef _DEBUG
	class LeakReporter
	{
	public:

		LeakReporter();

		void operator ()(const NamedContainer::value_type &namedContainerEntry);
		int  getLeakCount() const;

	private:

		int m_leakCount;
	};
#endif

	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

	bool             ms_installed;

	NamedContainer  *ms_textureRendererTemplates;
	LoaderContainer *ms_templateLoaders;
}

// ======================================================================
// class LeakReporter
// ======================================================================

#ifdef _DEBUG

LeakReporter::LeakReporter()
:	m_leakCount(0)
{
}

// ----------------------------------------------------------------------

void LeakReporter::operator ()(const NamedContainer::value_type &namedContainerEntry)
{
	const TextureRendererTemplate *const textureRendererTemplate = namedContainerEntry.second;

	DEBUG_REPORT_LOG(true, ("--| leaked [%s] [%d references outstanding]\n", 
		textureRendererTemplate->getCrcName().getString(), 
		textureRendererTemplate->getReferenceCount()));

	++m_leakCount;
}

// ----------------------------------------------------------------------

inline int LeakReporter::getLeakCount() const
{
	return m_leakCount;
}

#endif

// ======================================================================
// class TextureRendererList
// ======================================================================

void TextureRendererList::install()
{
	DEBUG_FATAL(ms_installed, ("TextureRendererList already installed"));

	ExitChain::add(remove, "TextureRendererList");

	ms_textureRendererTemplates = new NamedContainer();
	ms_templateLoaders          = new LoaderContainer();

	ms_installed = true;
}

// ----------------------------------------------------------------------

void TextureRendererList::remove()
{
	DEBUG_FATAL(!ms_installed, ("TextureRendererList not installed"));
	
	NOT_NULL(ms_templateLoaders);
	delete ms_templateLoaders;
	ms_templateLoaders = 0;

	//-- delete renderer templates
	NOT_NULL(ms_textureRendererTemplates);

	// report leaks
#ifdef _DEBUG
	LeakReporter leakReporter = std::for_each(ms_textureRendererTemplates->begin(), ms_textureRendererTemplates->end(), LeakReporter());
	DEBUG_WARNING(leakReporter.getLeakCount(), ("TextureRendererList: %d TextureRenderTemplate leaks, see above\n", leakReporter.getLeakCount()));
#endif

	// @todo track down texture renderer template leaks
#if 0
	// delete texture renderer templates
	IGNORE_RETURN( std::for_each(ms_textureRendererTemplates->begin(), ms_textureRendererTemplates->end(), PointerDeleterPairSecond()) );
#endif

	delete ms_textureRendererTemplates;
	ms_textureRendererTemplates = 0;
}

// ----------------------------------------------------------------------

void TextureRendererList::registerTemplateLoader(Tag templateIdTag, TemplateLoader templateLoader)
{
	NOT_NULL(ms_templateLoaders);

	std::pair<LCIterator, bool> result = ms_templateLoaders->insert(LoaderContainer::value_type(templateIdTag, templateLoader));
	UNREF(result);

#ifdef _DEBUG
	if (!result.second)
	{
		// loader already existed
		char name[5];
		ConvertTagToString(templateIdTag, name);
		DEBUG_FATAL(true, ("tried to registerTemplateLoader for [%s] twice", name));
	}
#endif
}

// ----------------------------------------------------------------------

void TextureRendererList::deregisterTemplateLoader(Tag templateIdTag)
{
	NOT_NULL(ms_templateLoaders);

	LCIterator loaderIt = ms_templateLoaders->find(templateIdTag);
	if (loaderIt != ms_templateLoaders->end())
		ms_templateLoaders->erase(loaderIt);
#ifdef _DEBUG
	else
	{
		char name[5];
		ConvertTagToString(templateIdTag, name);
		DEBUG_FATAL(true, ("TextureRendererTemplate loader [%s] not registered", name));
	}
#endif
}

// ----------------------------------------------------------------------
/**
 * Retrieve a TextureRendererTemplate given its filename.
 *
 * This function returns a reference-counted TextureRendererTemplate.  The
 * object may be cached and cause no disk I/O.
 *
 * NOTE: this function will return NULL if the TextureRendererTemplate does
 * not exist on disk.
 *
 * @return  the texture renderer object if it exists; NULL otherwise.
 */

const TextureRendererTemplate *TextureRendererList::fetch(const CrcLowerString &crcString)
{
	DEBUG_FATAL(!ms_installed, ("TextureRendererList not installed"));
	DEBUG_FATAL(!crcString.getString() || !*crcString.getString(), ("bad filename arg"));

	//-- check if we've already opened this one
	NOT_NULL(ms_textureRendererTemplates);

	NCIterator nameIt = ms_textureRendererTemplates->find(&crcString);
	if (nameIt == ms_textureRendererTemplates->end())
	{
		//-- if the file doesn't exist, return NULL
		const bool exists = TreeFile::exists(crcString.getString());
		if (!exists)
		{
			WARNING(true, ("failed to load TextureRendererTemplate [%s]", crcString.getString()));
			return 0;
		}

		//-- doesn't exist, create it
		Iff iff(crcString.getString());

		// find loader
		NOT_NULL(ms_templateLoaders);

		LCIterator loaderIt = ms_templateLoaders->find(iff.getCurrentName());
		DEBUG_FATAL(loaderIt == ms_templateLoaders->end(), ("unknown texture renderer template loader for [%s]", crcString.getString()));

#ifdef _DEBUG
		DataLint::pushAsset(crcString.getString());
#endif // _DEBUG

		TextureRendererTemplate *const newTextureRendererTemplate = (*loaderIt->second)(&iff, crcString.getString());

#ifdef _DEBUG
		DataLint::popAsset();
#endif // _DEBUG

		const CrcLowerString &newCrcName = newTextureRendererTemplate->getCrcName();
		NamedContainer::value_type value(&newCrcName, newTextureRendererTemplate);
		std::pair<NCIterator, bool> result = ms_textureRendererTemplates->insert(value);
		
		// keep track of the iterator
		nameIt = result.first;
		DEBUG_FATAL(!result.second, ("both find and insert failed for TextureRendererTemplate in [%s]", crcString.getString()));
	}

	TextureRendererTemplate *const textureRendererTemplate = nameIt->second;
	NOT_NULL(textureRendererTemplate);

	textureRendererTemplate->fetch();

	return textureRendererTemplate;
}

// ----------------------------------------------------------------------

const TextureRendererTemplate *TextureRendererList::fetch(const char *filename)
{
	const CrcLowerString crcString(filename);
	return fetch(crcString);
}

// ----------------------------------------------------------------------

const TextureRendererTemplate *TextureRendererList::fetch(Iff *iff)
{
	DEBUG_FATAL(!ms_installed, ("TextureRendererList not installed"));

	// find loader
	NOT_NULL(ms_templateLoaders);

	LCIterator loaderIt = ms_templateLoaders->find(iff->getCurrentName());
	DEBUG_FATAL(loaderIt == ms_templateLoaders->end(), ("unknown texture renderer template"));

	TextureRendererTemplate *const textureRendererTemplate = (*loaderIt->second)(iff, "");
	textureRendererTemplate->fetch();

	return textureRendererTemplate;
}

// ----------------------------------------------------------------------

void TextureRendererList::removeFromList(const TextureRendererTemplate *textureRendererTemplate)
{
	DEBUG_FATAL(!ms_installed, ("TextureRendererList not installed"));

	// check if we have this in our named list
	NOT_NULL(ms_textureRendererTemplates);

	const CrcLowerString &crcName = textureRendererTemplate->getCrcName();
	NamedContainer::iterator it = ms_textureRendererTemplates->find(&crcName);
	if (it != ms_textureRendererTemplates->end())
	{
		// we've got it in our named container, release it
		ms_textureRendererTemplates->erase(it);
	}
}

// ======================================================================
