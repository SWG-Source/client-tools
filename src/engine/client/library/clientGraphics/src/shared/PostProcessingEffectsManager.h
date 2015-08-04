// ======================================================================
//
// PostProcessingEffectsManager.h
// Copyright 2004, Sony Online Entertainment Inc.
// All Rights Reserved.
//
// ======================================================================

#ifndef INCLUDED_PostProcessingEffectsManager_H
#define INCLUDED_PostProcessingEffectsManager_H

// ======================================================================

class Texture;
class StaticShader;

//----------------------------------------------------------------------

class PostProcessingEffectsManager
{
public:

	static void install();

	static bool isSupported();
	static bool isEnabled();
	static void setEnabled(bool enabled);
		
	static void preSceneRender();
	static void postSceneRender();

	static Texture * getPrimaryBuffer();
	static Texture * getSecondaryBuffer();
	static Texture * getTertiaryBuffer();

	static StaticShader * getHeatCompositingShader();

	static void swapBuffers();

	static void setAntialiasEnabled(bool enabled);
	static bool getAntialiasEnabled();

private:

	static void remove();

	static void enable();
	static void disable();

	PostProcessingEffectsManager();
	PostProcessingEffectsManager(PostProcessingEffectsManager const &);
	~PostProcessingEffectsManager();
};

// ======================================================================

#endif
