// ======================================================================
//
// ConfigClientSkeletalAnimation.h
// Copyright 2002 Sony Online Entertainment, Inc.
// All Rights Reserved.
//
// ======================================================================

#ifndef INCLUDED_ConfigClientSkeletalAnimation_H
#define INCLUDED_ConfigClientSkeletalAnimation_H

// ======================================================================

class ConfigClientSkeletalAnimation
{
public:

	static void  install();

	static int   getFreeShaderPrimitiveFrameCount();

	static float getNoRenderScreenFraction();
	static float getNoSkinningScreenFraction();
	static float getBatchRenderScreenFraction();
	static float getHardSkinningScreenFraction();

	static float getLod3ScreenFraction();
	static float getLod2ScreenFraction();
	static float getLod1ScreenFraction();

	static float getTargetPitchClampDegrees();
	static float getLookAtYawClampDegrees();
	static float getMaxHeadTurnSpeed();

	static int   getMissingLodWarningThreshold();

	static bool  getAllowSameTrackTrumping();
	static bool  getLogSameTrackTrumping();

	static bool  getLogStateTraversal();
	static bool  getLogActionSelection();
	static bool  getLogAnimationLoading();
	static bool  getLogAnimationLookup();
	static bool  getLogSktCreateDestroy();
	static bool  getLogStringSelectorAnimation();

	static bool  getRenderPlaybackScriptFeedback();

	static bool  getLodManagerEnable();
	static float getLodManagerFirstLodCount();
	static int   getLodManagerEveryOtherFrameSkinningCharacterCount();
	static int   getLodManagerHardSkinningCharacterCount();

	static int   getSkipActionGenerationFrameCount();

	static int   getAnimationMaxValidReferenceCount();
	static int   getControllerMaxChannelCount();

	static bool  getSkeletonSegmentSanityCheckerEnabled();

	static bool  getOptimizeSkinnedIndexBuffers();

	static bool getWarningTooManyLods();

	static float getBlendTime();

private:

	// Disabled.
	ConfigClientSkeletalAnimation();
	ConfigClientSkeletalAnimation(const ConfigClientSkeletalAnimation &);
	ConfigClientSkeletalAnimation &operator =(const ConfigClientSkeletalAnimation &);

};

// ======================================================================

#endif
