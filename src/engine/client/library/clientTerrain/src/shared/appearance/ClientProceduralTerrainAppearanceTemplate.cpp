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
#include "sharedFile/Iff.h"
#include "sharedFoundation/ExitChain.h"
#include "sharedObject/AppearanceTemplateList.h"
#include "sharedTerrain/ConfigSharedTerrain.h"
#include "sharedUtility/FileName.h"

#include <vector>

//===================================================================
// ClientProceduralTerrainAppearanceTemplate::PreloadManager
//===================================================================

class ClientProceduralTerrainAppearanceTemplate::PreloadManager
{
public:

	explicit PreloadManager (const ClientProceduralTerrainAppearanceTemplate* clientProceduralTerrainAppearanceTemplate);
	~PreloadManager ();

private:
	
	PreloadManager ();
	PreloadManager (const PreloadManager&);
	PreloadManager& operator= (const PreloadManager&);

private:

	typedef std::vector<const AppearanceTemplate*> PreloadAppearanceTemplateList;
	PreloadAppearanceTemplateList m_preloadAppearanceTemplateList;

	typedef std::vector<const ShaderTemplate*> PreloadShaderTemplateList;
	PreloadShaderTemplateList m_preloadShaderTemplateList;
};

//-------------------------------------------------------------------

ClientProceduralTerrainAppearanceTemplate::PreloadManager::PreloadManager (const ClientProceduralTerrainAppearanceTemplate* const clientProceduralTerrainAppearanceTemplate) :
	m_preloadAppearanceTemplateList (),
	m_preloadShaderTemplateList ()
{
	const TerrainGenerator* const generator = clientProceduralTerrainAppearanceTemplate->getTerrainGenerator ();

	//-- handle terrain shaders
	{
		const ShaderGroup& shaderGroup = generator->getShaderGroup ();

		int i;
		for (i = 0; i < shaderGroup.getNumberOfFamilies (); ++i)
		{
			int j;
			for (j = 0; j < shaderGroup.getNumberOfChildren (i); ++j)
			{
				const ShaderGroup::FamilyChildData fcd = shaderGroup.getChild (i, j);
				m_preloadShaderTemplateList.push_back (ShaderTemplateList::fetch (FileName (FileName::P_shader, fcd.shaderTemplateName)));
			}
		}
	}

	//-- handle appearances
	{
		const FloraGroup& floraGroup = generator->getFloraGroup ();

		int i;
		for (i = 0; i < floraGroup.getNumberOfFamilies (); ++i)
		{
			int j;
			for (j = 0; j < floraGroup.getNumberOfChildren (i); ++j)
			{
				const FloraGroup::FamilyChildData fcd = floraGroup.getChild (i, j);
				const AppearanceTemplate* const appearanceTemplate = AppearanceTemplateList::fetch (FileName (FileName::P_appearance, fcd.appearanceTemplateName));
				appearanceTemplate->preloadAssets ();
				m_preloadAppearanceTemplateList.push_back (appearanceTemplate);
			}
		}
	}

	//-- handle flora shaders
	{
		const RadialGroup& radialGroup = generator->getRadialGroup ();

		int i;
		for (i = 0; i < radialGroup.getNumberOfFamilies (); ++i)
		{
			int j;
			for (j = 0; j < radialGroup.getNumberOfChildren (i); ++j)
			{
				const RadialGroup::FamilyChildData fcd = radialGroup.getChild (i, j);
				m_preloadShaderTemplateList.push_back (ShaderTemplateList::fetch (FileName (FileName::P_shader, fcd.shaderTemplateName)));
			}
		}
	}
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

void ClientProceduralTerrainAppearanceTemplate::garbageCollect () const
{
	AppearanceTemplate::garbageCollect ();

	if (m_preloadManager)
	{
		delete m_preloadManager;
		m_preloadManager = 0;
	}
}

//===================================================================
