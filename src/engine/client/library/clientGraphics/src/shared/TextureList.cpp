// ======================================================================
//
// TextureList.cpp
// Portions copyright 1998 Bootprint Entertainment
// Portions copyright 2000-2002 Sony Online Entertainment
// All Rights Reserved.
//
// ======================================================================

#include "clientGraphics/FirstClientGraphics.h"
#include "clientGraphics/TextureList.h"

#include "clientGraphics/Graphics.h"
#include "clientGraphics/Texture.h"
#include "clientGraphics/TextureFormatInfo.h"
#include "sharedDebug/DataLint.h"
#include "sharedFile/AsynchronousLoader.h"
#include "sharedFile/Iff.h"
#include "sharedFile/TreeFile.h"
#include "sharedFoundation/ConstCharCrcString.h"
#include "sharedFoundation/ExitChain.h"
#include "sharedFoundation/LessPointerComparator.h"
#include "sharedFoundation/TemporaryCrcString.h"
#include "sharedFoundation/PointerDeleter.h"
#include "sharedSynchronization/Mutex.h"

#include <algorithm>
#include <map>
#include <vector>

// ======================================================================

namespace TextureListNamespace
{
	typedef std::map<CrcString const *,Texture *, LessPointerComparator> NamedContainer;

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

	bool               ms_installed;
	bool               ms_buildingAsynchronousLoaderData;
	Mutex              ms_criticalSection;
	NamedContainer    *ms_namedTextures;

	typedef std::vector<TextureList::CleanupCallbackFunction> CleanupCallbackFunctionList;
	CleanupCallbackFunctionList ms_cleanupCallbackFunctionList;
}
using namespace TextureListNamespace;

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
	DEBUG_REPORT_LOG(true, ("--| leaked [%s] [%d references outstanding]\n", 
		namedContainerEntry.second->getCrcString().getString(), 
		namedContainerEntry.second->getReferenceCount()));

	++m_leakCount;
}

// ----------------------------------------------------------------------

inline int LeakReporter::getLeakCount() const
{
	return m_leakCount;
}

#endif

// ======================================================================
// class TextureList
// ======================================================================

void TextureList::install()
{
	DEBUG_FATAL(ms_installed, ("attempted to install TextureList when already ms_installed"));
	ExitChain::add(TextureList::remove, ("TextureList::remove"));

	if (TreeFile::isLoggingFiles())
	{
		delete TreeFile::open(getDefaultTextureName(), AbstractFile::PriorityData, true);
	}

	ms_namedTextures   = new NamedContainer();
	ms_installed = true;
}

// ----------------------------------------------------------------------

void TextureList::remove()
{
	DEBUG_FATAL(!ms_installed, ("attempted to remove TextureList when not ms_installed"));

	//-- Call cleanup callback functions
	{
		for (CleanupCallbackFunctionList::iterator iter = ms_cleanupCallbackFunctionList.begin(); iter != ms_cleanupCallbackFunctionList.end(); ++iter)
			(*iter)();
	}

	Graphics::releaseAllGlobalTextures();

	//-- report leaks
#ifdef _DEBUG
	LeakReporter leakReporter = std::for_each(ms_namedTextures->begin(), ms_namedTextures->end(), LeakReporter());
	DEBUG_WARNING(leakReporter.getLeakCount(), ("TextureList: %d named Texture leaks, see above", leakReporter.getLeakCount()));
#endif

	//-- delete leaked named textures
	// note: we don't use for_each/PointerDeleterPairSecond here because that would force us to make
	//       Texture::~Texture public.  That is dangerous, as Texture clients should use Texture::release()
	//       to release a texture.
	NamedContainer::iterator itEnd = ms_namedTextures->end();
	for (NamedContainer::iterator it = ms_namedTextures->begin(); it != itEnd; ++it)
		delete it->second;

	delete ms_namedTextures;
	ms_namedTextures = NULL;

	ms_installed = false;
}

// ----------------------------------------------------------------------

void TextureList::assignAsynchronousLoaderFunctions()
{
	if (AsynchronousLoader::isInstalled())
		AsynchronousLoader::bindFetchReleaseFunctions("dds", &asynchronousLoaderFetchNoCreate, &asynchronousLoaderRelease);
}

// ----------------------------------------------------------------------

void TextureList::reloadTextures()
{
	NamedContainer::iterator const iEnd = ms_namedTextures->end();
	for (NamedContainer::iterator i = ms_namedTextures->begin(); i != iEnd; ++i)
		i->second->load(i->first->getString());
}

// ----------------------------------------------------------------------

void TextureList::setBuildingAsynchronousLoaderData(bool const buildingAsynchronousLoaderData)
{
	ms_buildingAsynchronousLoaderData = buildingAsynchronousLoaderData;
}

// ----------------------------------------------------------------------

const void *TextureList::asynchronousLoaderFetchNoCreate(const char *fileName)
{
	TemporaryCrcString cfn(fileName, true);
	return TextureList::fetch(cfn, false);
}

// ----------------------------------------------------------------------

void TextureList::asynchronousLoaderRelease(const void *texture)
{
	static_cast<const Texture *>(texture)->release();
}

// ----------------------------------------------------------------------

const char *TextureList::getDefaultTextureName()
{
	return "texture/defaulttexture.dds";
}

// ----------------------------------------------------------------------

Texture const * TextureList::fetchDefaultNormalTexture()
{
	return fetch(getDefaultNormalTextureName());
}

// ----------------------------------------------------------------------

char const * TextureList::getDefaultNormalTextureName()
{
	return "texture/defaultnormal.dds";
}

// ----------------------------------------------------------------------

const Texture *TextureList::fetchDefaultTexture()
{
	return fetch(getDefaultTextureName());
}

// ----------------------------------------------------------------------

const char *TextureList::getDefaultEnvironmentTextureName()
{
	return "texture/defaultcubemap.dds";
}

// ----------------------------------------------------------------------

const Texture *TextureList::fetchDefaultEnvironmentTexture()
{
	return fetch(getDefaultEnvironmentTextureName());
}

// ----------------------------------------------------------------------

const Texture * TextureList::fetch(CrcString const & fileName)
{
	return fetch(fileName, true);
}

//----------------------------------------------------------------------

const Texture * TextureList::fetchDefaultWhiteTexture()
{
	return fetch("texture/white.dds");
}


const Texture * TextureList::fetchDefaultBlackTexture()
{
	return fetch("texture/black.dds");
}

// ----------------------------------------------------------------------
/**
 * Retrieve a texture from an IFF or DDS file
 *
 * The format for the data is defined in $/new/doc/AppearFormats.txt
 * under the section {texture_map}.
 *
 * This function will first check all loaded named textures to see if
 * the texture is already in memory.  If found, it will bump up the
 * reference count and return the already-loaded texture; otherwise,
 * it will load the texture from the specified file.
 */
Texture const * TextureList::fetch(CrcString const & filename, bool const createTexture)
{
	DEBUG_FATAL(!ms_installed, ("TextureList not installed"));

	if (ms_buildingAsynchronousLoaderData)
	{
		//-- fake a TF::open and return the default texture for speed
		DEBUG_REPORT_LOG(true, ("TF::open(F) %s @ [faked]\n", filename.getString()));
		return create(ConstCharCrcString(getDefaultTextureName()), true);
	}

	if (filename.isEmpty())
	{
		DEBUG_WARNING(true, ("zero-length filename passed to Texture::fetch"));
		return create(ConstCharCrcString(getDefaultTextureName()), true);
	}

	return create(filename, createTexture);
}

// ----------------------------------------------------------------------

Texture const * TextureList::create(CrcString const & filename, bool const createTexture)
{
	ms_criticalSection.enter();

#ifdef _DEBUG
		DataLint::pushAsset(filename.getString());
#endif // _DEBUG

		NamedContainer::iterator it = ms_namedTextures->find(&filename);
		if (it == ms_namedTextures->end())
		{
			if (!createTexture)
			{
				ms_criticalSection.leave();
				return NULL;
			}

			// need to create
			Texture *const newTexture = new Texture(filename);

			// add to list
			NamedContainer::value_type newValue(&(newTexture->getCrcString()), newTexture);
			std::pair<NamedContainer::iterator, bool> insertResult = ms_namedTextures->insert(newValue);
			DEBUG_FATAL(!insertResult.second, ("both find and insert failed for [%s]", filename.getString()));

			// set iterator
			it = insertResult.first;
		}
		
		Texture *const texture = it->second;
		texture->fetch();

#ifdef _DEBUG
		DataLint::popAsset();
#endif // _DEBUG

	ms_criticalSection.leave();

	return texture;
}

// ----------------------------------------------------------------------

const Texture *TextureList::fetch(const char *filename)
{
	TemporaryCrcString cfn(filename, true);
	return fetch(cfn, true);
}

// ----------------------------------------------------------------------
/** retrieve a texture from an IFF
 *
 * The format for the data is defined in $/new/doc/AppearFormats.txt 
 *   under the section {texture_map}.
 */
const Texture *TextureList::fetch(Iff *iff)
{
	NOT_NULL(iff);

	// fetch the named texture
	if (iff->getCurrentName() == TAG_NAME)
	{
		char  fileName[256];

		iff->enterChunk(TAG_NAME);
			iff->read_string(fileName, sizeof(fileName));
		iff->exitChunk(TAG_NAME);

		return fetch(fileName);
	}

	DEBUG_FATAL(true, ("anonymous TXMs are no longer supported"));
	return NULL;
}

// ----------------------------------------------------------------------
/**
 * Create an anonymous 1x1 texture of a given color
 */

const Texture *TextureList::fetch(int alpha, int red, int green, int blue)
{
	Texture *texture = new Texture(alpha, red, green, blue);
	texture->fetch();

	return texture;
}

// ----------------------------------------------------------------------
/**
 * Create a texture from pixel data.
 *
 * The pixel data is assumed to have no mip map levels, and to be formatted row-major.
 * 
 * @param pixelData               The actual pixel data.
 * @param format                  The source format of the pixel data.
 * @param width                   The width of the pixel data.
 * @param height                  The height of the pixel data.
 * @param runtimeFormats          Array of the allowable runtime formats for the texture.  May not be null.
 * @param numberOfRuntimeFormats  Number of valid entries in the runtimeFormats array.
 */

const Texture *TextureList::fetch(const void *pixelData, TextureFormat format, int width, int height, const TextureFormat *runtimeFormats, int numberOfRuntimeFormats)
{
	NOT_NULL(runtimeFormats);

	Texture *texture = new Texture(pixelData, format, width, height, 1, runtimeFormats, numberOfRuntimeFormats);
	texture->fetch();

	return texture;
}

// ----------------------------------------------------------------------
/**
 * Create a modifiable texture.
 * 
 * The initial contents of the texture are undefined.
 *
 * @param width                  width of the texture.
 * @param height                 height of the texture.
 * @param numberOfMipMapLevels   number of mip map levels in the texture.
 * @param runtimeTextureFormats  list of pixel formats appropriate for this texture ordered by preference.
 * @param textureFormatCount     number of texture formats in runtimeTextureFormats.  must be set to a value greater than or equal to one.
 */
Texture *TextureList::fetch(int textureCreationFlags, int width, int height, int numberOfMipMapLevels, const TextureFormat *runtimeTextureFormats, int textureFormatCount)
{
	Texture *texture = new Texture(textureCreationFlags, width, height, numberOfMipMapLevels, runtimeTextureFormats, textureFormatCount);
	texture->fetch();

	return texture;
}

// ----------------------------------------------------------------------

void TextureList::removeFromList(const Texture *texture)
{
	DEBUG_FATAL(!ms_installed, ("TextureList not installed"));
	NOT_NULL(texture);

	//-- see if this is something we would store in a named container
	const CrcString &crcString = texture->getCrcString();
	if (crcString.isEmpty())
	{
		// not named, so won't be in named texture list
		return;
	}

	ms_criticalSection.enter();

		NamedContainer::iterator it = ms_namedTextures->find(&crcString);
		if (it != ms_namedTextures->end())
			ms_namedTextures->erase(it);
		else
		{
			DEBUG_FATAL(it != ms_namedTextures->end(), ("named texture [%s] not in named texture map", crcString.getString()));
		}

	ms_criticalSection.leave();
}

// ----------------------------------------------------------------------

void TextureList::addCleanupCallbackFunction(CleanupCallbackFunction cleanupCallbackFunction)
{
	ms_cleanupCallbackFunctionList.push_back(cleanupCallbackFunction);
}

// ======================================================================
