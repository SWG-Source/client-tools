// ======================================================================
//
// KeyframeSkeletalAnimationTemplateDef.h
// copyright 2001 Sony Online Entertainment
// 
// ======================================================================

#ifndef KEYFRAME_SKELETAL_ANIMATION_TEMPLATE_DEF_H
#define KEYFRAME_SKELETAL_ANIMATION_TEMPLATE_DEF_H

// ======================================================================

enum // channel component flags
{
	SATCCF_xRotation       = BINARY2(0000,0001),  // if set, this rotation animates; otherwise, it is static throughout the course of this skeletal animation
	SATCCF_yRotation       = BINARY2(0000,0010),
	SATCCF_zRotation       = BINARY2(0000,0100),
	SATCCF_rotationMask    = BINARY2(0000,0111),

	SATCCF_xTranslation    = BINARY2(0000,1000),  // if set, this translation animates; otherwise, it is static
	SATCCF_yTranslation    = BINARY2(0001,0000),
	SATCCF_zTranslation    = BINARY2(0010,0000),
	SATCCF_translationMask = BINARY2(0011,1000)

};

// ======================================================================

#endif
