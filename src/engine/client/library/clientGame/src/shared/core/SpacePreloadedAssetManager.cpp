// ======================================================================
//
// SpacePreloadedAssetManager.cpp
// asommers
//
// copyright 2004, sony online entertainment
//
// ======================================================================

#include "clientGame/FirstClientGame.h"
#include "clientGame/SpacePreloadedAssetManager.h"

#include "clientAudio/SoundTemplate.h"
#include "clientAudio/SoundTemplateList.h"
#include "clientGame/ClientEffectTemplate.h"
#include "clientGame/ClientEffectTemplateList.h"
#include "clientGame/ConfigClientGame.h"
#include "clientGame/Game.h"
#include "sharedDebug/DebugFlags.h"
#include "sharedDebug/InstallTimer.h"
#include "sharedDebug/PerformanceTimer.h"
#include "sharedFile/Iff.h"
#include "sharedFile/TreeFile.h"
#include "sharedFoundation/ExitChain.h"
#include "sharedFoundation/Os.h"
#include "sharedObject/AppearanceTemplate.h"
#include "sharedObject/AppearanceTemplateList.h"
#include "sharedObject/PortalPropertyTemplate.h"
#include "sharedObject/PortalPropertyTemplateList.h"

#include <vector>

// ======================================================================

namespace SpacePreloadedAssetManagerNamespace
{
	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

	class Asset
	{
	public:

		enum Type
		{
			T_appearanceTemplate,
			T_clientEffectTemplate,
			T_portalPropertyTemplate,
			T_soundTemplate
		};

	public:

		Asset(Type type, char const * name);
		~Asset();

		void fetch();
		void release();

	private:

		Asset();
		Asset(Asset const &);
		Asset & operator=(Asset const &);

	private:

		Type const m_type;
		PersistentCrcString const m_name;
		void const * m_template;
	};

	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

	float const cms_callbackTime = 1.f;

	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

	typedef std::vector<Asset *> AssetList;
	AssetList ms_assetList;

	bool ms_logSpacePreloadedAssetManager;
	bool ms_loaded;
	int ms_numberOfAssetsLoaded;

	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

	void remove();
	void loadType(Iff & iff, Tag tag, Asset::Type type);

	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
}

using namespace SpacePreloadedAssetManagerNamespace;

// ======================================================================

SpacePreloadedAssetManagerNamespace::Asset::Asset(Type const type, char const * const name) :
	m_type(type),
	m_name(name, true),
	m_template(0)
{
}

// ----------------------------------------------------------------------

SpacePreloadedAssetManagerNamespace::Asset::~Asset()
{
	IS_NULL(m_template);
}

// ----------------------------------------------------------------------

void SpacePreloadedAssetManagerNamespace::Asset::fetch()
{
	if (m_template)
		return;

	switch (m_type)
	{
	case T_appearanceTemplate:
		{
			AppearanceTemplate const * const appearanceTemplate = AppearanceTemplateList::fetch(m_name.getString());
			if (appearanceTemplate)
				appearanceTemplate->preloadAssets();

			m_template = appearanceTemplate;
		}
		break;

	case T_clientEffectTemplate:
		{
			CrcLowerString const name(m_name.getString());
			m_template = ClientEffectTemplateList::fetch(name);
		}
		break;

	case T_portalPropertyTemplate:
		{
			PortalPropertyTemplate const * const portalPropertyTemplate = PortalPropertyTemplateList::fetch(m_name);
			if (portalPropertyTemplate)
				portalPropertyTemplate->preloadAssets();

			m_template = portalPropertyTemplate;
		}
		break;

	case T_soundTemplate:
		m_template = SoundTemplateList::fetch(m_name.getString());
		break;

	default:
		DEBUG_FATAL(true, ("SpacePreloadedAssetManager: Unknown type of asset %i [%s]", static_cast<int>(m_type), m_name.getString()));
		break;
	}
}

// ----------------------------------------------------------------------

void SpacePreloadedAssetManagerNamespace::Asset::release()
{
	if (!m_template)
		return;

	switch (m_type)
	{
	case T_appearanceTemplate:
		{
			AppearanceTemplate const * const appearanceTemplate = reinterpret_cast<AppearanceTemplate const *>(m_template);
			AppearanceTemplateList::release(appearanceTemplate);
		}
		break;

	case T_clientEffectTemplate:
		{
			ClientEffectTemplate const * const clientEffectTemplate = reinterpret_cast<ClientEffectTemplate const *>(m_template);
			clientEffectTemplate->release();
		}
		break;

	case T_portalPropertyTemplate:
		{
			PortalPropertyTemplate const * const portalPropertyTemplate = reinterpret_cast<PortalPropertyTemplate const *>(m_template);
			portalPropertyTemplate->release();
		}
		break;

	case T_soundTemplate:
		{
			SoundTemplate const * const soundTemplate = reinterpret_cast<SoundTemplate const *>(m_template);
			SoundTemplateList::release(soundTemplate);
		}
		break;

	default:
		DEBUG_FATAL(true, ("SpacePreloadedAssetManager: Unknown type of asset %i [%s]", static_cast<int>(m_type), m_name.getString()));
		break;
	}

	m_template = 0;
}

// ======================================================================

void SpacePreloadedAssetManager::install()
{
	InstallTimer const installTimer("SpacePreloadedAssetManager::install");

	if (!ConfigClientGame::getPreloadWorldSnapshot())
		return;

	DebugFlags::registerFlag(ms_logSpacePreloadedAssetManager, "ClientGame/SpacePreloadedAssetManager", "debugReport");

	Iff iff;
	if (iff.open("misc/space_preload.iff", true))
	{
		iff.enterForm(TAG(S,P,A,M));
			iff.enterForm(TAG_0000);

				loadType(iff, TAG(A,P,P,E), Asset::T_appearanceTemplate);
				loadType(iff, TAG(C,L,I,E), Asset::T_clientEffectTemplate);
				loadType(iff, TAG(P,O,R,T), Asset::T_portalPropertyTemplate);
				loadType(iff, TAG(S,O,U,N), Asset::T_soundTemplate);

			iff.exitForm();
		iff.exitForm();
	}

	ExitChain::add(remove, "SpacePreloadedAssetManagerNamespace::remove");
}

// ----------------------------------------------------------------------

void SpacePreloadedAssetManagerNamespace::loadType(Iff & iff, Tag const tag, Asset::Type const type)
{
	iff.enterForm(tag);
		iff.enterChunk(TAG_0000);

			char fileName[Os::MAX_PATH_LENGTH];
			while (iff.getChunkLengthLeft())
			{
				iff.read_string(fileName, Os::MAX_PATH_LENGTH);
				if (TreeFile::exists(fileName))
					ms_assetList.push_back(new Asset(type, fileName));
			}

		iff.exitChunk();
	iff.exitForm();
}

// ----------------------------------------------------------------------

void SpacePreloadedAssetManagerNamespace::remove()
{
	ms_loaded = false;

	for (size_t i = 0; i < ms_assetList.size(); ++i)
	{
		ms_assetList[i]->release();
		delete ms_assetList[i];
	}

	ms_assetList.clear();

	DebugFlags::unregisterFlag(ms_logSpacePreloadedAssetManager);
}

// ----------------------------------------------------------------------

void SpacePreloadedAssetManager::load(char const * const fileName)
{
	bool const isSpace = strstr(fileName, "space_") != 0;

	if (isSpace)
	{
		if (!ms_loaded)
		{
			ms_loaded = true;
			ms_numberOfAssetsLoaded = 0;
			preloadSomeAssets();
		}
	}
	else
	{
		for (size_t i = 0; i < ms_assetList.size(); ++i)
		{
			ms_assetList[i]->release();
		}

		ms_numberOfAssetsLoaded = ms_assetList.size();
		ms_loaded = false;
	}
}

// ----------------------------------------------------------------------

void SpacePreloadedAssetManager::preloadSomeAssets()
{
	PerformanceTimer preloadTimer;
	preloadTimer.start();

	while (ms_numberOfAssetsLoaded < static_cast<int>(ms_assetList.size()) && preloadTimer.getSplitTime() < cms_callbackTime)
	{
		ms_assetList[ms_numberOfAssetsLoaded]->fetch();
		++ms_numberOfAssetsLoaded;
	}
}

// ----------------------------------------------------------------------

int SpacePreloadedAssetManager::getLoadingPercent()
{
	if (ms_assetList.empty())
		return 100;

	return (ms_numberOfAssetsLoaded * 100) / static_cast<int>(ms_assetList.size());
}

// ----------------------------------------------------------------------

bool SpacePreloadedAssetManager::donePreloading()
{
	return ms_numberOfAssetsLoaded >= static_cast<int>(ms_assetList.size());
}

// ======================================================================
