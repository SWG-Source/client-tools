// ======================================================================
//
// SetupClientTextureRenderer.cpp
// Copyright 2002 Sony Online Entertainment Inc.
// All Rights Reserved.
//
// ======================================================================

#include "clientTextureRenderer/FirstClientTextureRenderer.h"
#include "clientTextureRenderer/SetupClientTextureRenderer.h"

#include "clientTextureRenderer/BlueprintTextureRendererTemplate.h"
#include "clientTextureRenderer/ConfigClientTextureRenderer.h"
#include "clientTextureRenderer/TextureRendererList.h"
#include "clientTextureRenderer/TextureRendererManager.h"
#include "clientTextureRenderer/TextureRendererShaderPrimitive.h"
#include "sharedDebug/InstallTimer.h"

// ======================================================================

void SetupClientTextureRenderer::install()
{
	InstallTimer const installTimer("SetupClientTextureRenderer::install");

	ConfigClientTextureRenderer::install();

	TextureRendererList::install();
	BlueprintTextureRendererTemplate::install();
	TextureRendererShaderPrimitive::install();
	TextureRendererManager::install();
}

// ======================================================================
