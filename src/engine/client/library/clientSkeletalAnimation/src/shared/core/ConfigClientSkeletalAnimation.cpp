// ======================================================================
//
// ConfigClientSkeletalAnimation.cpp
// copyright 2002, sony online entertainment
//
// ======================================================================

#include "clientSkeletalAnimation/FirstClientSkeletalAnimation.h"
#include "clientSkeletalAnimation/ConfigClientSkeletalAnimation.h"

#include "clientSkeletalAnimation/AnimationCompressor.h"
#include "sharedDebug/DebugFlags.h"
#include "sharedFoundation/ConfigFile.h"

#include <cstdlib>

// ======================================================================

namespace
{
	int   s_freeShaderPrimitiveFrameCount;

	float s_noRenderScreenFraction;
	float s_noSkinningScreenFraction;
	float s_batchRenderScreenFraction;
	float s_hardSkinningScreenFraction;

	float s_lod3ScreenFraction;
	float s_lod2ScreenFraction;
	float s_lod1ScreenFraction;

	float s_targetPitchClampDegrees;
	float s_lookAtYawClampDegrees;
	float s_maxHeadTurnSpeed;

	int   s_missingLodWarningThreshold;
	bool s_warningTooManyLods;

	bool  s_allowSameTrackTrumping;
	bool  s_logSameTrackTrumping;

	bool  s_logStateTraversal;
	bool  s_logActionSelection;
	bool  s_logAnimationLoading;
	bool  s_logAnimationLookup;
	bool  s_logSktCreateDestroy;
	bool  s_logStringSelectorAnimation;

	bool  s_renderPlaybackScriptFeedback;

	bool  s_lodManagerEnable;
	float s_lodManagerFirstLodCount;
	int   s_lodManagerEveryOtherFrameSkinningCharacterCount;
	int   s_lodManagerHardSkinningCharacterCount;

	int   s_skipActionGenerationFrameCount;

	int   s_animationMaxValidReferenceCount;
	int   s_controllerMaxChannelCount;

	bool  s_skeletonSegmentSanityCheckerEnabled;
	
	bool  s_optimizeSkinnedIndexBuffers;

	float s_blendTime;
}

// ======================================================================

#define KEY_BOOL(a,b)    (s_ ## a = ConfigFile::getKeyBool("ClientSkeletalAnimation", #a, b))
#define KEY_FLOAT(a,b)   (s_ ## a = ConfigFile::getKeyFloat("ClientSkeletalAnimation", #a, b))
#define KEY_INT(a,b)     (s_ ## a = ConfigFile::getKeyInt("ClientSkeletalAnimation", #a, b))
//#define KEY_STRING(a,b)  (ms_ ## a = ConfigFile::getKeyString("ClientSkeletalAnimation", #a, b))

#define REGISTER_FLAG(a) (DebugFlags::registerFlag(s_ ## a, "ClientSkeletalAnimation", #a))

// ======================================================================

void ConfigClientSkeletalAnimation::install()
{
	int defaultFreeShaderPrimitiveFrameCount = 30 * 15;   // <= 256 MB = 30 seconds @ 15 fps
	if (MemoryManager::getLimit() > 300)
		defaultFreeShaderPrimitiveFrameCount = 60 * 15;   // <= 512 MB = 60 seconds @ 15 fps
	if (MemoryManager::getLimit() > 600)
		defaultFreeShaderPrimitiveFrameCount = 600 * 15;  //  > 512 MB = ~unlimited
	KEY_INT(freeShaderPrimitiveFrameCount, defaultFreeShaderPrimitiveFrameCount);

	KEY_FLOAT (noRenderScreenFraction,     1.0f / 100.0f);
	KEY_FLOAT (noSkinningScreenFraction,   1.0f / 200.0f);
	KEY_FLOAT (batchRenderScreenFraction,  1.0f /  16.0f);
	KEY_FLOAT (hardSkinningScreenFraction, 1.0f /   4.0f);

	KEY_FLOAT (lod3ScreenFraction,         1.0f / 20.0f);
	KEY_FLOAT (lod2ScreenFraction,         2.0f / 20.0f);
	KEY_FLOAT (lod1ScreenFraction,         4.0f / 20.0f);

	KEY_FLOAT (targetPitchClampDegrees,    60.0f);
	KEY_FLOAT (lookAtYawClampDegrees,      70.0f);
	KEY_FLOAT (maxHeadTurnSpeed,          600.0f);

	KEY_INT   (missingLodWarningThreshold, 4);
	KEY_BOOL(warningTooManyLods, false);

	KEY_BOOL      (logStateTraversal, false);
	REGISTER_FLAG (logStateTraversal);

	KEY_BOOL      (logActionSelection, false);
	REGISTER_FLAG (logActionSelection);

	KEY_BOOL      (logAnimationLoading, false);
	REGISTER_FLAG (logAnimationLoading);

	KEY_BOOL      (logAnimationLookup, false);
	REGISTER_FLAG (logAnimationLookup);

	KEY_BOOL      (logSktCreateDestroy, false);
	REGISTER_FLAG (logSktCreateDestroy);

	KEY_BOOL      (logStringSelectorAnimation, false);
	REGISTER_FLAG (logStringSelectorAnimation);

	KEY_BOOL      (renderPlaybackScriptFeedback, false);
	REGISTER_FLAG (renderPlaybackScriptFeedback);

	KEY_BOOL      (allowSameTrackTrumping, true);
	REGISTER_FLAG (allowSameTrackTrumping);

	KEY_BOOL      (logSameTrackTrumping, false);
	REGISTER_FLAG (logSameTrackTrumping);

	KEY_BOOL      (lodManagerEnable, true);
	KEY_FLOAT     (lodManagerFirstLodCount, 2.5f);
	KEY_INT       (lodManagerEveryOtherFrameSkinningCharacterCount, 10);
	KEY_INT       (lodManagerHardSkinningCharacterCount, 5);

	KEY_INT       (skipActionGenerationFrameCount, 30 * 1);

	KEY_INT       (animationMaxValidReferenceCount, 15);
	KEY_INT       (controllerMaxChannelCount, 15);

	KEY_BOOL      (skeletonSegmentSanityCheckerEnabled, false);
	
	KEY_BOOL      (optimizeSkinnedIndexBuffers, false);
	KEY_FLOAT     (blendTime, 0.25f);
#ifdef _DEBUG
	char const *const compressionResponseFilename = ConfigFile::getKeyString("ClientSkeletalAnimation", "compressionResponseFilename", "");
	if (strlen(compressionResponseFilename) > 0)
	{
		AnimationCompressor::compressAnimations(compressionResponseFilename);
		exit(0);
	}
#endif
}

// ----------------------------------------------------------------------

int ConfigClientSkeletalAnimation::getFreeShaderPrimitiveFrameCount()
{
	return s_freeShaderPrimitiveFrameCount;
}

// ----------------------------------------------------------------------

float ConfigClientSkeletalAnimation::getNoRenderScreenFraction()
{
	return s_noRenderScreenFraction;
}

// ----------------------------------------------------------------------

float ConfigClientSkeletalAnimation::getNoSkinningScreenFraction()
{
	return s_noSkinningScreenFraction;
}

// ----------------------------------------------------------------------

float ConfigClientSkeletalAnimation::getBatchRenderScreenFraction()
{
	return s_batchRenderScreenFraction;
}

// ----------------------------------------------------------------------

float ConfigClientSkeletalAnimation::getHardSkinningScreenFraction()
{
	return s_hardSkinningScreenFraction;
}

// ----------------------------------------------------------------------

float ConfigClientSkeletalAnimation::getLod3ScreenFraction()
{
	return s_lod3ScreenFraction;
}

// ----------------------------------------------------------------------

float ConfigClientSkeletalAnimation::getLod2ScreenFraction()
{
	return s_lod2ScreenFraction;
}

// ----------------------------------------------------------------------

float ConfigClientSkeletalAnimation::getLod1ScreenFraction()
{
	return s_lod1ScreenFraction;
}

// ----------------------------------------------------------------------

float ConfigClientSkeletalAnimation::getTargetPitchClampDegrees()
{
	return s_targetPitchClampDegrees;
}

// ----------------------------------------------------------------------

float ConfigClientSkeletalAnimation::getLookAtYawClampDegrees()
{
	return s_lookAtYawClampDegrees;
}

// ----------------------------------------------------------------------

float ConfigClientSkeletalAnimation::getMaxHeadTurnSpeed()
{
	return s_maxHeadTurnSpeed;
}

// ----------------------------------------------------------------------

int ConfigClientSkeletalAnimation::getMissingLodWarningThreshold()
{
	return s_missingLodWarningThreshold;
}

// ----------------------------------------------------------------------

bool ConfigClientSkeletalAnimation::getLogStateTraversal()
{
	return s_logStateTraversal;
}

// ----------------------------------------------------------------------

bool ConfigClientSkeletalAnimation::getLogActionSelection()
{
	return s_logActionSelection;
}

// ----------------------------------------------------------------------

bool ConfigClientSkeletalAnimation::getLogAnimationLoading()
{
	return s_logAnimationLoading;
}

// ----------------------------------------------------------------------

bool ConfigClientSkeletalAnimation::getLogAnimationLookup()
{
	return s_logAnimationLookup;
}

// ----------------------------------------------------------------------

bool ConfigClientSkeletalAnimation::getLogSktCreateDestroy()
{
	return s_logSktCreateDestroy;
}

// ----------------------------------------------------------------------

bool ConfigClientSkeletalAnimation::getLogStringSelectorAnimation()
{
	return s_logStringSelectorAnimation;
}

// ----------------------------------------------------------------------

bool ConfigClientSkeletalAnimation::getRenderPlaybackScriptFeedback()
{
	return s_renderPlaybackScriptFeedback;
}

// ----------------------------------------------------------------------

bool ConfigClientSkeletalAnimation::getAllowSameTrackTrumping()
{
	return s_allowSameTrackTrumping;
}

// ----------------------------------------------------------------------

bool ConfigClientSkeletalAnimation::getLogSameTrackTrumping()
{
	return s_logSameTrackTrumping;
}

// ----------------------------------------------------------------------

bool ConfigClientSkeletalAnimation::getLodManagerEnable()
{
	return s_lodManagerEnable;
}

// ----------------------------------------------------------------------

float ConfigClientSkeletalAnimation::getLodManagerFirstLodCount()
{
	return s_lodManagerFirstLodCount;
}

// ----------------------------------------------------------------------

int ConfigClientSkeletalAnimation::getLodManagerEveryOtherFrameSkinningCharacterCount()
{
	return s_lodManagerEveryOtherFrameSkinningCharacterCount;
}

// ----------------------------------------------------------------------

int ConfigClientSkeletalAnimation::getLodManagerHardSkinningCharacterCount()
{
	return s_lodManagerHardSkinningCharacterCount;
}

// ----------------------------------------------------------------------

int ConfigClientSkeletalAnimation::getSkipActionGenerationFrameCount()
{
	//-- Indicates # frames since last time the appearance was rendered after which we no longer generate animation actions.
	return s_skipActionGenerationFrameCount;
}

// ----------------------------------------------------------------------

int ConfigClientSkeletalAnimation::getAnimationMaxValidReferenceCount()
{
	return s_animationMaxValidReferenceCount;
}

// ----------------------------------------------------------------------

int ConfigClientSkeletalAnimation::getControllerMaxChannelCount()
{
	return s_controllerMaxChannelCount;
}

// ----------------------------------------------------------------------

bool ConfigClientSkeletalAnimation::getSkeletonSegmentSanityCheckerEnabled()
{
	return s_skeletonSegmentSanityCheckerEnabled;
}

// ----------------------------------------------------------------------

bool ConfigClientSkeletalAnimation::getOptimizeSkinnedIndexBuffers()
{
	return s_optimizeSkinnedIndexBuffers;
}

//----------------------------------------------------------------------

bool ConfigClientSkeletalAnimation::getWarningTooManyLods()
{
	return s_warningTooManyLods;
}

//----------------------------------------------------------------------

float ConfigClientSkeletalAnimation::getBlendTime()
{
	return s_blendTime;
}


// ======================================================================
