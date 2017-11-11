// ======================================================================
//
// SetupClientSkeletalAnimation.cpp
// Copyright 2001, 2002 Sony Online Entertainment, Inc.
// All Rights Reserved.
//
// ======================================================================

#include "clientSkeletalAnimation/FirstClientSkeletalAnimation.h"
#include "clientSkeletalAnimation/SetupClientSkeletalAnimation.h"

#include "clientSkeletalAnimation/ActionGeneratorSkeletalAnimationTemplate.h"
#include "clientSkeletalAnimation/AnimationEnvironment.h"
#include "clientSkeletalAnimation/AnimationHeldItemMapper.h"
#include "clientSkeletalAnimation/AnimationMessageActionTemplate.h"
#include "clientSkeletalAnimation/AnimationNotification.h"
#include "clientSkeletalAnimation/AnimationPostureMapper.h"
#include "clientSkeletalAnimation/AnimationPriorityMap.h"
#include "clientSkeletalAnimation/AnimationStateNameIdManager.h"
#include "clientSkeletalAnimation/AnimationStateHierarchyTemplateList.h"
#include "clientSkeletalAnimation/BasicSkeletonTemplate.h"
#include "clientSkeletalAnimation/CallbackAnimationNotification.h"
#include "clientSkeletalAnimation/CharacterLodManager.h"
#include "clientSkeletalAnimation/CompositeMesh.h"
#include "clientSkeletalAnimation/CompressedKeyframeAnimation.h"
#include "clientSkeletalAnimation/CompressedKeyframeAnimationTemplate.h"
#include "clientSkeletalAnimation/ConfigClientSkeletalAnimation.h"
#include "clientSkeletalAnimation/DirectionSkeletalAnimation.h"
#include "clientSkeletalAnimation/DirectionSkeletalAnimationTemplate.h"
#include "clientSkeletalAnimation/EditableAnimationState.h"
#include "clientSkeletalAnimation/EditableAnimationStateHierarchyTemplate.h"
#include "clientSkeletalAnimation/EditableBasicAnimationAction.h"
#include "clientSkeletalAnimation/EditableMovementAnimationAction.h"
#include "clientSkeletalAnimation/FullGeometrySkeletalAppearanceBatchRenderer.h"
#include "clientSkeletalAnimation/KeyframeSkeletalAnimation.h"
#include "clientSkeletalAnimation/KeyframeSkeletalAnimationTemplate.h"
#include "clientSkeletalAnimation/LodMeshGeneratorTemplate.h"
#include "clientSkeletalAnimation/LodSkeletonTemplate.h"
#include "clientSkeletalAnimation/LogicalAnimationTableTemplate.h"
#include "clientSkeletalAnimation/LogicalAnimationTableTemplateList.h"
#include "clientSkeletalAnimation/LookAtTransformModifier.h"
#include "clientSkeletalAnimation/MeshGeneratorTemplateList.h"
#include "clientSkeletalAnimation/OcclusionZoneSet.h"
#include "clientSkeletalAnimation/OwnerProxyShader.h"
#include "clientSkeletalAnimation/OwnerProxyShaderTemplate.h"
#include "clientSkeletalAnimation/PriorityBlendAnimation.h"
#include "clientSkeletalAnimation/PriorityBlendAnimationTemplate.h"
#include "clientSkeletalAnimation/ProxySkeletalAnimationTemplate.h"
#include "clientSkeletalAnimation/ShowAttachedObjectAction.h"
#include "clientSkeletalAnimation/ShowAttachedObjectActionTemplate.h"
#include "clientSkeletalAnimation/SinglePrioritySkeletalAnimation.h"
#include "clientSkeletalAnimation/SkeletalAnimation.h"
#include "clientSkeletalAnimation/SkeletalAnimationDebugging.h"
#include "clientSkeletalAnimation/SkeletalAnimationTemplate.h"
#include "clientSkeletalAnimation/SkeletalAnimationTemplateList.h"
#include "clientSkeletalAnimation/SkeletalAppearance2.h"
#include "clientSkeletalAnimation/SkeletalAppearanceTemplate.h"
#include "clientSkeletalAnimation/SkeletalMeshGenerator.h"
#include "clientSkeletalAnimation/SkeletalMeshGeneratorTemplate.h"
#include "clientSkeletalAnimation/Skeleton.h"
#include "clientSkeletalAnimation/SkeletonTemplateList.h"
#include "clientSkeletalAnimation/SpeedSkeletalAnimation.h"
#include "clientSkeletalAnimation/StateHierarchyAnimationController.h"
#include "clientSkeletalAnimation/StringSelectorSkeletalAnimationTemplate.h"
#include "clientSkeletalAnimation/SpeedSkeletalAnimationTemplate.h"
#include "clientSkeletalAnimation/SoftwareBlendSkeletalShaderPrimitive.h"
#include "clientSkeletalAnimation/TargetPitchTransformModifier.h"
#include "clientSkeletalAnimation/TimeScaleSkeletalAnimation.h"
#include "clientSkeletalAnimation/TimeScaleSkeletalAnimationTemplate.h"
#include "clientSkeletalAnimation/TrackAnimationController.h"
#include "clientSkeletalAnimation/TransformMaskList.h"
#include "clientSkeletalAnimation/YawSkeletalAnimationTemplate.h"
#include "sharedDebug/InstallTimer.h"

// ======================================================================

const char *const SetupClientSkeletalAnimation::cms_defaultPriorityMapFileName = "animation/priority_map.iff";

// ======================================================================

SetupClientSkeletalAnimation::Data::Data() :
	allowLod0Skipping(true),
	stitchedSkinInheritsFromSelf(false)
{
}

// ======================================================================

void SetupClientSkeletalAnimation::setupGameData(Data &data)
{
	data.allowLod0Skipping = true;
}

// ----------------------------------------------------------------------

void SetupClientSkeletalAnimation::setupToolData(Data &data)
{
	data.allowLod0Skipping = false;
}

// ----------------------------------------------------------------------

void SetupClientSkeletalAnimation::setupViewerData(Data &data)
{
	data.allowLod0Skipping = false;
	data.stitchedSkinInheritsFromSelf = true;
}

// ----------------------------------------------------------------------

void SetupClientSkeletalAnimation::install(Data const &data)
{
	InstallTimer const installTimer("SetupClientSkeletalAnimation::install");

	ConfigClientSkeletalAnimation::install();

	SkeletalAnimationDebugging::install();

	AnimationPriorityMap::install(cms_defaultPriorityMapFileName);
	TrackAnimationController::install();
	StateHierarchyAnimationController::install();
	AnimationStateHierarchyTemplateList::install();
	LogicalAnimationTableTemplate::install();
	LogicalAnimationTableTemplateList::install();
	AnimationEnvironment::install();

	Skeleton::install();
	SkeletonTemplateList::install();
	BasicSkeletonTemplate::install();
	LodSkeletonTemplate::install(data.allowLod0Skipping);

	TransformMaskList::install();

	AnimationNotification::install();
	CallbackAnimationNotification::install();
	SkeletalAnimationTemplateList::install();
	SkeletalAnimationTemplate::install();
	SkeletalAnimation::install();
	BasePriorityBlendAnimation::install();
	SpeedSkeletalAnimation::install();
	CompressedKeyframeAnimationTemplate::install();
	CompressedKeyframeAnimation::install();
	KeyframeSkeletalAnimationTemplate::install();
	KeyframeSkeletalAnimation::install();
	ProxySkeletalAnimationTemplate::install();
	DirectionSkeletalAnimationTemplate::install();
	DirectionSkeletalAnimation::install();
	PriorityBlendAnimationTemplate::install();
	PriorityBlendAnimation::install();
	SinglePrioritySkeletalAnimation::install();
	SpeedSkeletalAnimationTemplate::install();
	StringSelectorSkeletalAnimationTemplate::install();
	TimeScaleSkeletalAnimationTemplate::install();
	TimeScaleSkeletalAnimation::install();
	ActionGeneratorSkeletalAnimationTemplate::install();
	YawSkeletalAnimationTemplate::install();

	OcclusionZoneSet::install();
	CompositeMesh::install();
	SkeletalAppearanceTemplate::install();
	SkeletalAppearance2::install();
	SoftwareBlendSkeletalShaderPrimitive::install();

	MeshGeneratorTemplateList::install();
	SkeletalMeshGenerator::install();
	SkeletalMeshGeneratorTemplate::install();
	LodMeshGeneratorTemplate::install(data.allowLod0Skipping);

	AnimationMessageActionTemplate::install();
	ShowAttachedObjectAction::install();
	ShowAttachedObjectActionTemplate::install();

	AnimationStateNameIdManager::install();

	// @todo -TRF- Someday make non-editable, more efficient versions and only use these editable ones in the AnimationEditor.
	EditableBasicAnimationAction::install();
	EditableMovementAnimationAction::install();
	EditableAnimationState::install();
	EditableAnimationStateHierarchyTemplate::install();

	AnimationHeldItemMapper::install("animation/held_item_map.iff");
	AnimationPostureMapper::install("animation/posture_map.iff");

	OwnerProxyShaderTemplate::install();
	OwnerProxyShader::install(data.stitchedSkinInheritsFromSelf);

	FullGeometrySkeletalAppearanceBatchRenderer::install();

	LookAtTransformModifier::install();
	TargetPitchTransformModifier::install();

	CharacterLodManager::install();
}

// ======================================================================
