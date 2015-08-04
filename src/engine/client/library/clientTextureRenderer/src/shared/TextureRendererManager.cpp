// ======================================================================
//
// TextureRendererManager.cpp
// Copyright 2002 Sony Online Entertainment Inc.
// All Rights Reserved.
// 
// ======================================================================

#include "clientTextureRenderer/FirstClientTextureRenderer.h"
#include "clientTextureRenderer/TextureRendererManager.h"

#include "clientGraphics/Graphics.h"
#include "clientTextureRenderer/TextureRenderer.h"
#include "sharedDebug/DebugFlags.h"
#include "sharedDebug/Profiler.h"
#include "sharedDebug/ProfilerTimer.h"
#include "sharedFoundation/ExitChain.h"
#include "sharedFoundation/Production.h"
#include "sharedFoundation/VoidMemberFunction.h"

#include <algorithm>
#include <vector>

// ======================================================================

namespace TextureRendererManagerNamespace
{
	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

#if PRODUCTION == 0
	
	void  reportStatistics();

#endif

	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

#if PRODUCTION == 0
	bool  s_reportStatistics;

	int                  s_bakedTextureCount;
	ProfilerTimer::Type  s_callTime;
#endif
}

using namespace TextureRendererManagerNamespace;

// ======================================================================

bool                                           TextureRendererManager::ms_installed;
TextureRendererManager::TextureRendererVector *TextureRendererManager::ms_textureRenderers;

// ======================================================================
// namespace TextureRendererManagerNamespace
// ======================================================================

#if PRODUCTION == 0

void TextureRendererManagerNamespace::reportStatistics()
{
	//-- Get profiler frequency.
	ProfilerTimer::Type  timeUnused;
	ProfilerTimer::Type  ticksPerSecond;

	ProfilerTimer::getCalibratedTime(timeUnused, ticksPerSecond);
	float const callTimeInSeconds = (ticksPerSecond > 0) ? static_cast<float>(s_callTime)/static_cast<float>(ticksPerSecond) : 0.0f;

	REPORT_PRINT(true, ("texture baking: [%d] textures in [%g] seconds, graphics frame [%d]\n", s_bakedTextureCount, callTimeInSeconds, Graphics::getFrameNumber()));

	if (s_bakedTextureCount > 0)
		REPORT_LOG(true, ("texture baking: [%d] textures in [%g] seconds, graphics frame [%d]\n", s_bakedTextureCount, callTimeInSeconds, Graphics::getFrameNumber()));

	//-- Reset statistics data for next frame.
	s_bakedTextureCount = 0;
	s_callTime          = 0;
}

#endif

// ======================================================================
/**
 * Prepare the TextureRendererManager static class for use.
 *
 * This function must be called prior to calling any other function
 * in the TextureRendererManager interface.  SetupClientTextureRenderer
 * calls this function.
 *
 * This function will place a remove() function on the ExitChain.
 */

void TextureRendererManager::install()
{
	DEBUG_FATAL(ms_installed, ("TextureRendererManager already installed"));

	ms_textureRenderers = new TextureRendererVector();

#if PRODUCTION == 0
	DebugFlags::registerFlag(s_reportStatistics, "ClientTextureRenderer", "reportStatistics", reportStatistics);

	s_bakedTextureCount = 0;
	s_callTime          = 0;
#endif

	ms_installed = true;
	ExitChain::add(remove, "TextureRendererManager");
}

// ----------------------------------------------------------------------
/**
 * Submit a TextureRenderer for rendering as soon as possible.
 *
 * This function submits a TextureRenderer for renderering as soon as
 * possible.  It is not a problem to submit the same TextureRenderer
 * if it already has been submitted but not yet processed.
 *
 * @param textureRenderer  the TextureRenderer instance to be rendered
 *                         as soon as possible.
 */

void TextureRendererManager::submitTextureRenderer(TextureRenderer *textureRenderer)
{
	DEBUG_FATAL(!ms_installed, ("TextureRendererManager not installed"));
	NOT_NULL(textureRenderer);

	const TextureRendererVector::iterator findIt = std::find(ms_textureRenderers->begin(), ms_textureRenderers->end(), textureRenderer);
	if (findIt != ms_textureRenderers->end())
	{
		// ignore request: the given texture renderer has been submitted already
		return;
	}

	//-- fetch local reference
	textureRenderer->fetch();

	//-- add to list
	ms_textureRenderers->push_back(textureRenderer);
}

// ----------------------------------------------------------------------
/**
 * Perform any per-frame processing/bookkeeping to handle TextureRenderer
 * rendering.
 *
 * This function should be called once per main loop, and should not occur
 * within a Graphics::beginFrame()/Graphics::endFrame() pair.
 *
 * In the current implementation, all pending TextureRenderer rendering 
 * (baking) takes place during this call.
 */

void TextureRendererManager::alter(float deltaTime)
{
	DEBUG_FATAL(!ms_installed, ("TextureRendererManager not installed"));
	UNREF(deltaTime);

	NP_PROFILER_AUTO_BLOCK_DEFINE("TextureRendererManager::alter");

#if PRODUCTION == 0
	//-- Record processing start time.
	ProfilerTimer::Type  startTime;
	ProfilerTimer::getTime (startTime);
#endif

	//-- render the texture renderers
	for (TextureRendererVector::iterator it = ms_textureRenderers->begin(); it != ms_textureRenderers->end();)
	{
		TextureRenderer *const tr = (*it);
		NOT_NULL(tr);

#if PRODUCTION == 0
		//-- Track statistics.
		++s_bakedTextureCount;
#endif

		//-- render it
		const bool renderSuccess = tr->render();

		if (renderSuccess)
		{
			// Release local reference.
			tr->release();

			// Remove from list and increment loop.
			it = ms_textureRenderers->erase(it);
		}
		else
		{
			// Try to render this next frame, increment loop.
			++it;
		}
	}

#if PRODUCTION == 0
	//-- Add processing delta time to call time variable.
	ProfilerTimer::Type  endTime;
	ProfilerTimer::getTime(endTime);

	s_callTime += (endTime - startTime);
#endif

	DEBUG_REPORT_LOG(!ms_textureRenderers->empty(), ("TextureRendererManager: failed to bake %d textures, trying again next frame.", static_cast<int>(ms_textureRenderers->size())));
}

// ======================================================================
/**
 * Release resources held by the TextureRendererManager static class.
 *
 * This function should not be called directly.  install() installs this
 * function on the ExitChain.
 */

void TextureRendererManager::remove()
{
	DEBUG_FATAL(!ms_installed, ("TextureRendererManager not installed"));

#if PRODUCTION == 0
	//-- Remove debug falgs.
	DebugFlags::unregisterFlag(s_reportStatistics);
#endif

	//-- release outstanding TextureRenderer references
	DEBUG_WARNING(!ms_textureRenderers->empty(), ("TextureRendererManager: %u texture renderers still queued for render\n", ms_textureRenderers->size()));
	std::for_each(ms_textureRenderers->begin(), ms_textureRenderers->end(), VoidMemberFunction(&TextureRenderer::release));

	//-- delete TextureRenderer container
	delete ms_textureRenderers;
	ms_textureRenderers = 0;

	ms_installed = false;
}

// ======================================================================
