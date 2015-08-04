// ======================================================================
//
// SetupClientObject.cpp
// copyright (c) 2001 Sony Online Entertainment
//
// ======================================================================

#include "clientObject/FirstClientObject.h"
#include "clientObject/SetupClientObject.h"

#include "clientObject/BeamAppearance.h"
#include "clientObject/BeamAppearanceTemplate.h"
#include "clientObject/ConfigClientObject.h"
#include "clientObject/ComponentAppearanceTemplate.h"
#include "clientObject/DetailAppearance.h"
#include "clientObject/DetailAppearanceTemplate.h"
#include "clientObject/GameCamera.h"
#include "clientObject/HardpointObject.h"
#include "clientObject/InteriorEnvironmentBlockManager.h"
#include "clientObject/MarkerAppearance.h"
#include "clientObject/MarkerAppearanceTemplate.h"
#include "clientObject/MeshAppearance.h"
#include "clientObject/MeshAppearanceTemplate.h"
#include "clientObject/MouseCursor.h"
#include "clientObject/RibbonAppearance.h"
#include "clientObject/RibbonTrailAppearance.h"
#include "clientObject/ShadowBlobManager.h"
#include "clientObject/ShadowManager.h"
#include "clientObject/ShadowVolume.h"
#include "clientObject/SpriteAppearance.h"
#include "clientObject/SpriteAppearanceTemplate.h"
#include "clientObject/ReticleManager.h"
#include "clientObject/TimerObject.h"
#include "clientObject/TrailAppearance.h"
#include "sharedDebug/InstallTimer.h"
#include "sharedFoundation/ExitChain.h"

// ======================================================================
// SetupClientObjectNamespace
// ======================================================================

namespace SetupClientObjectNamespace
{
	void remove ();
}

using namespace SetupClientObjectNamespace;

// ======================================================================
// SetupClientObject::Data
// ======================================================================

SetupClientObject::Data::Data () :
	m_viewer (false)
{
}

// ======================================================================
// STATIC PUBLIC SetupClientObject
// ======================================================================

void SetupClientObject::install (const Data& data)
{
	InstallTimer const installTimer("SetupClientObject::install");

	ConfigClientObject::install ();

#ifdef PLATFORM_WIN32
	// setup the default mouse acceleration
	DWORD mouseData[3];
	memset(mouseData, 0, sizeof(mouseData));
	const BOOL result = SystemParametersInfo(SPI_GETMOUSE, 0, mouseData, 0);
	DEBUG_FATAL(!result, ("SystemParametersInfo SPI_GETMOUSE failed"));
	if (result)
		MouseCursor::setAcceleration(static_cast<int>(mouseData[2]), static_cast<int>(mouseData[0]), static_cast<int>(mouseData[1]));
#endif

	TimerObject::install ();
	HardpointObject::install ();
	SpriteAppearance::install ();
	SpriteAppearanceTemplate::install ();
	DetailAppearance::install ();
	DetailAppearanceTemplate::install (data.m_viewer);
	ComponentAppearanceTemplate::install ();
	BeamAppearance::install ();
	BeamAppearanceTemplate::install();
	MarkerAppearance::install ();
	MarkerAppearanceTemplate::install ();
	MeshAppearanceTemplate::install();
	ShadowManager::install ();
	ShadowVolume::install ();
	GameCamera::install ();
	TrailAppearance::install ();
	RibbonAppearance::install ();
	RibbonTrailAppearance::install ();
	InteriorEnvironmentBlockManager::install ();
	ShadowBlobManager::install ();
	ReticleManager::install ();
	MeshAppearance::install ();

	ExitChain::add (SetupClientObjectNamespace::remove, "SetupClientObject");
}

// ----------------------------------------------------------------------

void SetupClientObject::setupGameData (Data& data)
{
	data.m_viewer = false;
}

// ----------------------------------------------------------------------

void SetupClientObject::setupToolData (Data& data)
{
	data.m_viewer = true;
}

// ======================================================================
// SetupClientObjectNamespace
// ======================================================================

void SetupClientObjectNamespace::remove ()
{
}

// ======================================================================
