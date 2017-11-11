// ======================================================================
//
// SetupClientTerrain.cpp
// asommers 9-10-2000
//
// copyright 2000, verant interactive
//
// ======================================================================

#include "clientTerrain/FirstClientTerrain.h"
#include "clientTerrain/SetupClientTerrain.h"

#include "clientGraphics/RenderWorld.h"
#include "clientGraphics/ShaderPrimitiveSorter.h"
#include "clientTerrain/CelestialObject.h"
#include "clientTerrain/ClientDynamicRadialFloraManager.h"
#include "clientTerrain/ClientGlobalWaterManager2.h"
#include "clientTerrain/ClientProceduralTerrainAppearance.h"
#include "clientTerrain/ClientProceduralTerrainAppearanceTemplate.h"
#include "clientTerrain/ClientRadialFloraManager.h"
#include "clientTerrain/ClientServerProceduralTerrainAppearance.h"
#include "clientTerrain/ClientServerProceduralTerrainAppearanceTemplate.h"
#include "clientTerrain/ClientSpaceTerrainAppearanceTemplate.h"
#include "clientTerrain/ClientStaticRadialFloraManager.h"
#include "clientTerrain/CloudLayerAppearance.h"
#include "clientTerrain/ConfigClientTerrain.h"
#include "clientTerrain/GradientSkyAppearance.h"
#include "clientTerrain/PlanetAppearance.h"
#include "clientTerrain/PlanetAppearanceTemplate.h"
#include "clientTerrain/SkyBoxAppearance.h"
#include "clientTerrain/StarAppearance.h"
#include "clientTerrain/WaterManager.h"
#include "sharedDebug/InstallTimer.h"
#include "sharedFoundation/ExitChain.h"
#include "sharedTerrain/ConfigSharedTerrain.h"
#include "sharedTerrain/SetupSharedTerrain.h"
#include "sharedTerrain/TerrainObject.h"
#include "sharedTerrain/WaterTypeManager.h"

// ======================================================================

namespace SetupClientTerrainNamespace
{
	bool ms_installed;

	bool belowTransparentWaterFunction(const Vector& position_w)
	{
		TerrainGeneratorWaterType waterType;
		return TerrainObject::getInstance() && TerrainObject::getInstance()->isBelowWater(position_w, waterType, true);
	}

	const Tag TAG_ALPH = TAG(A,L,P,H);
	const Tag TAG_WABV = TAG(W,A,B,V);
	const Tag TAG_WBLW = TAG(W,B,L,W);
}

using namespace SetupClientTerrainNamespace;

// ======================================================================

void SetupClientTerrain::install()
{
	InstallTimer const installTimer("SetupClientTerrain::install");

	DEBUG_FATAL(ms_installed, ("SetupClientTerrain::install already installed"));
	ms_installed = true;

	DEBUG_FATAL(!SetupSharedTerrain::isInstalled(), ("SetupSharedTerrain is not installed"));
	DEBUG_REPORT_LOG_PRINT(ConfigSharedTerrain::getDebugReportInstall(), ("SetupClientTerrain::install\n"));

	//-- setup config file
	ConfigClientTerrain::install();

	//-- install appropriate terrain system
#ifdef _DEBUG
	if(ConfigClientTerrain::getUseClientServerProceduralTerrainAppearanceTemplate())
	{
		ClientServerProceduralTerrainAppearanceTemplate::install();
		ClientServerProceduralTerrainAppearance::install();
	}
	else
#endif
	{
		ProceduralTerrainAppearance::setMaximumNumberOfChunksAllowed(ConfigClientTerrain::getMaximumNumberOfChunksAllowed());
		ClientProceduralTerrainAppearanceTemplate::install();
		ClientProceduralTerrainAppearance::install();
		ClientRadialFloraManager::install();
		ClientStaticRadialFloraManager::install();
		ClientDynamicRadialFloraManager::install();
		ClientGlobalWaterManager2::install();
	}

	ClientSpaceTerrainAppearanceTemplate::install();

	CelestialObject::install();
	CloudLayerAppearance::install();
	GradientSkyAppearance::install();
	PlanetAppearanceTemplate::install();
	StarAppearance::install();
	SkyBoxAppearance::install();

	//-- setup below water hook
	ShaderPrimitiveSorter::setBelowTransparentWaterFunctionAndPhases(belowTransparentWaterFunction, ShaderPrimitiveSorter::getPhase(TAG_ALPH), ShaderPrimitiveSorter::getPhase(TAG_WBLW), ShaderPrimitiveSorter::getPhase(TAG_WABV));

	ExitChain::add(SetupClientTerrain::remove, "SetupClientTerrain");
}

// ----------------------------------------------------------------------

void SetupClientTerrain::remove()
{
	DEBUG_FATAL(!ms_installed, ("SetupClientTerrain::remove not installed"));
	ms_installed = false;

	ShaderPrimitiveSorter::setBelowTransparentWaterFunctionAndPhases(0, 0, 0, 0);
}

// ----------------------------------------------------------------------

bool SetupClientTerrain::isInstalled()
{
	return ms_installed;
}

// ======================================================================
