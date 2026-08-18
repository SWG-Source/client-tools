//===================================================================
//
// ClientProceduralTerrainAppearanceTemplate.cpp
// asommers
//
// copyright 2000, verant interactive
//
//===================================================================

#include "clientTerrain/FirstClientTerrain.h"
#include "clientTerrain/ClientProceduralTerrainAppearanceTemplate.h"

#include "clientGraphics/ShaderTemplate.h"
#include "clientGraphics/ShaderTemplateList.h"
#include "clientTerrain/ConfigClientTerrain.h"
#include "clientTerrain/ClientProceduralTerrainAppearance.h"
#include "sharedDebug/PerformanceTimer.h"
#include "sharedFile/Iff.h"
#include "sharedFoundation/ExitChain.h"
#include "sharedObject/AppearanceTemplateList.h"
#include "sharedTerrain/ConfigSharedTerrain.h"
#include "sharedUtility/FileName.h"

#include <string>
#include <vector>

//===================================================================
// ClientProceduralTerrainAppearanceTemplate::PreloadManager
//===================================================================

// CONSULT-59: the constructor used to synchronously fetch every terrain shader,
// flora appearance, and radial-flora shader for the whole planet -- a 3-4.5s
// main-loop freeze inside the GroundScene constructor (watchdog-convicted
// 2026-07-04). The constructor now only CAPTURES the name lists; step() performs
// the fetches under a per-frame time budget. The destructor is unchanged: it
// releases exactly what was fetched so far, so early teardown mid-preload stays
// refcount-balanced.
class ClientProceduralTerrainAppearanceTemplate::PreloadManager
{
public:

	explicit PreloadManager (const ClientProceduralTerrainAppearanceTemplate* clientProceduralTerrainAppearanceTemplate);
	~PreloadManager ();

	bool step (float budgetMs);
	bool isComplete () const;

private:

	PreloadManager ();
	PreloadManager (const PreloadManager&);
	PreloadManager& operator= (const PreloadManager&);

private:

	typedef std::vector<const AppearanceTemplate*> PreloadAppearanceTemplateList;
	PreloadAppearanceTemplateList m_preloadAppearanceTemplateList;

	typedef std::vector<const ShaderTemplate*> PreloadShaderTemplateList;
	PreloadShaderTemplateList m_preloadShaderTemplateList;

	typedef std::vector<std::string> PendingNameList;
	PendingNameList m_pendingShaderNames;
	PendingNameList m_pendingFloraNames;
	bool m_complete;
};

//-------------------------------------------------------------------

ClientProceduralTerrainAppearanceTemplate::PreloadManager::PreloadManager (const ClientProceduralTerrainAppearanceTemplate* const clientProceduralTerrainAppearanceTemplate) :
	m_preloadAppearanceTemplateList (),
	m_preloadShaderTemplateList (),
	m_pendingShaderNames (),
	m_pendingFloraNames (),
	m_complete (false)
{
	const TerrainGenerator* const generator = clientProceduralTerrainAppearanceTemplate->getTerrainGenerator ();

	//-- capture terrain shader names (fetched incrementally in step())
	{
		const ShaderGroup& shaderGroup = generator->getShaderGroup ();

		int i;
		for (i = 0; i < shaderGroup.getNumberOfFamilies (); ++i)
		{
			int j;
			for (j = 0; j < shaderGroup.getNumberOfChildren (i); ++j)
			{
				const ShaderGroup::FamilyChildData fcd = shaderGroup.getChild (i, j);
				m_pendingShaderNames.push_back (std::string (FileName (FileName::P_shader, fcd.shaderTemplateName).getString ()));
			}
		}
	}

	//-- capture flora appearance names
	{
		const FloraGroup& floraGroup = generator->getFloraGroup ();

		int i;
		for (i = 0; i < floraGroup.getNumberOfFamilies (); ++i)
		{
			int j;
			for (j = 0; j < floraGroup.getNumberOfChildren (i); ++j)
			{
				const FloraGroup::FamilyChildData fcd = floraGroup.getChild (i, j);
				m_pendingFloraNames.push_back (std::string (FileName (FileName::P_appearance, fcd.appearanceTemplateName).getString ()));
			}
		}
	}

	//-- capture flora shader names
	{
		const RadialGroup& radialGroup = generator->getRadialGroup ();

		int i;
		for (i = 0; i < radialGroup.getNumberOfFamilies (); ++i)
		{
			int j;
			for (j = 0; j < radialGroup.getNumberOfChildren (i); ++j)
			{
				const RadialGroup::FamilyChildData fcd = radialGroup.getChild (i, j);
				m_pendingShaderNames.push_back (std::string (FileName (FileName::P_shader, fcd.shaderTemplateName).getString ()));
			}
		}
	}

	//-- budget <= 0 restores the old single-shot synchronous preload
	if (ConfigClientTerrain::getTerrainPreloadBudgetMs () <= 0)
		IGNORE_RETURN (step (0.f));
}

//-------------------------------------------------------------------

bool ClientProceduralTerrainAppearanceTemplate::PreloadManager::step (float const budgetMs)
{
	if (m_complete)
		return true;

	PerformanceTimer timer;
	timer.start ();

	for (;;)
	{
		if (!m_pendingShaderNames.empty ())
		{
			m_preloadShaderTemplateList.push_back (ShaderTemplateList::fetch (m_pendingShaderNames.back ().c_str ()));
			m_pendingShaderNames.pop_back ();
		}
		else if (!m_pendingFloraNames.empty ())
		{
			const AppearanceTemplate* const appearanceTemplate = AppearanceTemplateList::fetch (m_pendingFloraNames.back ().c_str ());
			m_pendingFloraNames.pop_back ();
			appearanceTemplate->preloadAssets ();
			m_preloadAppearanceTemplateList.push_back (appearanceTemplate);
		}
		else
		{
			m_complete = true;
			break;
		}

		//-- budgetMs <= 0 means run to completion (old synchronous behavior)
		if (budgetMs > 0.f && timer.getSplitTime () * 1000.f >= budgetMs)
			break;
	}

	return m_complete;
}

//-------------------------------------------------------------------

bool ClientProceduralTerrainAppearanceTemplate::PreloadManager::isComplete () const
{
	return m_complete;
}

//-------------------------------------------------------------------

ClientProceduralTerrainAppearanceTemplate::PreloadManager::~PreloadManager ()
{
	{
		uint i;
		for (i = 0; i < m_preloadAppearanceTemplateList.size (); ++i)
			AppearanceTemplateList::release (m_preloadAppearanceTemplateList [i]);

		m_preloadAppearanceTemplateList.clear ();
	}

	{
		uint i;
		for (i = 0; i < m_preloadShaderTemplateList.size (); ++i)
			m_preloadShaderTemplateList [i]->release ();

		m_preloadShaderTemplateList.clear ();
	}
}

//===================================================================

void ClientProceduralTerrainAppearanceTemplate::install ()
{
	DEBUG_REPORT_LOG_PRINT (ConfigSharedTerrain::getDebugReportInstall (), ("ClientProceduralTerrainAppearanceTemplate::install\n"));

	AppearanceTemplateList::assignBinding (TAG (M,P,T,A), ClientProceduralTerrainAppearanceTemplate::create);
	AppearanceTemplateList::assignBinding (TAG (P,T,A,T), ClientProceduralTerrainAppearanceTemplate::create);

	ExitChain::add (remove, "ClientProceduralTerrainAppearanceTemplate::remove");
}

//-------------------------------------------------------------------

void ClientProceduralTerrainAppearanceTemplate::remove ()
{
	AppearanceTemplateList::removeBinding (TAG (M,P,T,A));
	AppearanceTemplateList::removeBinding (TAG (P,T,A,T));
}

//-------------------------------------------------------------------

AppearanceTemplate* ClientProceduralTerrainAppearanceTemplate::create (const char* filename, Iff* iff)
{
	AppearanceTemplate* const appearanceTemplate = new ClientProceduralTerrainAppearanceTemplate (filename, iff);
	appearanceTemplate->preloadAssets ();

	return appearanceTemplate;
}

//===================================================================

ClientProceduralTerrainAppearanceTemplate::ClientProceduralTerrainAppearanceTemplate (const char* filename, Iff* iff) :
	ProceduralTerrainAppearanceTemplate (filename, iff, false, 2, 2),
	m_preloadManager (0)
{
}

//-------------------------------------------------------------------

ClientProceduralTerrainAppearanceTemplate::~ClientProceduralTerrainAppearanceTemplate ()
{
	if (m_preloadManager)
	{
		delete m_preloadManager;
		m_preloadManager = 0;
	}
}

//-------------------------------------------------------------------

Appearance* ClientProceduralTerrainAppearanceTemplate::createAppearance () const
{
	return new ClientProceduralTerrainAppearance (this);
}

//-------------------------------------------------------------------

void ClientProceduralTerrainAppearanceTemplate::preloadAssets () const
{
	AppearanceTemplate::preloadAssets ();

	if (!m_preloadManager)
		m_preloadManager = new PreloadManager (this);
}

//-------------------------------------------------------------------

bool ClientProceduralTerrainAppearanceTemplate::preloadAssetsStep () const
{
	if (!m_preloadManager)
		preloadAssets ();

	NOT_NULL (m_preloadManager);
	return m_preloadManager->step (static_cast<float> (ConfigClientTerrain::getTerrainPreloadBudgetMs ()));
}

//-------------------------------------------------------------------

bool ClientProceduralTerrainAppearanceTemplate::isPreloadComplete () const
{
	return m_preloadManager && m_preloadManager->isComplete ();
}

//-------------------------------------------------------------------

void ClientProceduralTerrainAppearanceTemplate::garbageCollect () const
{
	AppearanceTemplate::garbageCollect ();

	// CONSULT-59 Guard A: Game::garbageCollect walks every named template each
	// time real GC work fires, and the incremental preload spans many frames.
	// Deleting a mid-preload manager would drop the partial pins and silently
	// restart the preload from zero on the next step. Only collect once complete.
	if (m_preloadManager && m_preloadManager->isComplete ())
	{
		delete m_preloadManager;
		m_preloadManager = 0;
	}
}

//===================================================================
