// ======================================================================
//
// TextureRendererManager.h
// Copyright 2002 Sony Online Entertainment Inc.
// All Rights Reserved.
// 
// ======================================================================

#ifndef INCLUDED_TextureRendererManager_H
#define INCLUDED_TextureRendererManager_H

// ======================================================================

class TextureRenderer;

// ======================================================================
/**
 * Ensures that all TextureRenderer instances submitted are rendered
 * in a timely fashion.
 *
 * Currently this render manager for TextureRenderer instances operates
 * in a single-threaded fashion.  This is because the underlying render
 * mechanism used by at least one of the TextureRenderer classes
 * makes use of the graphics pipeline.  Access to the graphics pipeline
 * must occur at a known point.  All TextureRenderer instances will
 * be (potentially) rendered at one point during the game loop.
 *
 * If TextureRenderer rendering is done via some other mechanism (CPU, 
 * for example), this could be turned into a multi-threaded process
 * that might allow both single and (especially) multi-CPU machines higher
 * throughput on TextureRenderer rendering.
 */

class TextureRendererManager
{
public:

	static void install();

	static void submitTextureRenderer(TextureRenderer *textureRenderer);

	static void alter(float deltaTime);

private:

	typedef stdvector<TextureRenderer*>::fwd  TextureRendererVector;

private:

	static void remove();

private:

	// disabled
	TextureRendererManager();
	TextureRendererManager(const TextureRendererManager&);
	TextureRendererManager &operator =(const TextureRendererManager&);

private:

	static bool                   ms_installed;
	static TextureRendererVector *ms_textureRenderers;

};

// ======================================================================

#endif
