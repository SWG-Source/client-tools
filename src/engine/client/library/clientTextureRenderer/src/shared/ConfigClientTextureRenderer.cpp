// ======================================================================
//
// ConfigClientTextureRenderer.cpp
// Copyright 2002 Sony Online Entertainment, Inc.
// All Rights Reserved.
//
// ======================================================================

#include "clientTextureRenderer/FirstClientTextureRenderer.h"
#include "clientTextureRenderer/ConfigClientTextureRenderer.h"

#include "sharedFoundation/ConfigFile.h"

//===================================================================

namespace
{
	bool  s_disableTextureBaking;
}

//===================================================================

#define KEY_BOOL(a,b)     (s_ ## a = ConfigFile::getKeyBool("ClientTextureRenderer", #a, b))
// #define REGISTER_BOOL(a)   DebugFlags::registerFlag(s_ ## a, "ClientAnimation", #a)
// #define KEY_INT(a,b)    (s_ ## a = ConfigFile::getKeyInt("ClientAnimation", #a,  b))
// #define KEY_FLOAT(a,b)  (s_ ## a = ConfigFile::getKeyFloat("ClientAnimation", #a, b))
// #define KEY_STRING(a,b) (s_ ## a = ConfigFile::getKeyString("ClientAnimation", #a, b))

//===================================================================

void ConfigClientTextureRenderer::install(void)
{
	KEY_BOOL(disableTextureBaking, false);
}

// ----------------------------------------------------------------------

bool ConfigClientTextureRenderer::getDisableTextureBaking()
{
	return s_disableTextureBaking;
}

//===================================================================
