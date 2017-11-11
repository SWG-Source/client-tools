// ======================================================================
//
// TargetPitchTransformModifier.cpp
// Copyright 2003 Sony Online Entertainment, Inc.
// All Rights Reserved.
//
// ======================================================================

#include "clientSkeletalAnimation/FirstClientSkeletalAnimation.h"
#include "clientSkeletalAnimation/TargetPitchTransformModifier.h"

#include "clientGraphics/Camera.h"
#include "clientGraphics/DebugPrimitive.h"
#include "clientGraphics/ShaderPrimitiveSorter.h"

#include "clientSkeletalAnimation/AnimationStateNameId.h"
#include "clientSkeletalAnimation/AnimationStateNameIdManager.h"
#include "clientSkeletalAnimation/AnimationStatePath.h"
#include "clientSkeletalAnimation/ConfigClientSkeletalAnimation.h"
#include "clientSkeletalAnimation/SkeletalAppearance2.h"
#include "clientSkeletalAnimation/Skeleton.h"
#include "clientSkeletalAnimation/StateHierarchyAnimationController.h"
#include "sharedDebug/DebugFlags.h"
#include "sharedFoundation/ConstCharCrcLowerString.h"
#include "sharedFoundation/ExitChain.h"
#include "sharedFoundation/MemoryBlockManager.h"
#include "sharedFoundation/Production.h"
#include "sharedMath/Quaternion.h"
#include "sharedMath/Transform.h"
#include "sharedObject/Object.h"

// ======================================================================

namespace TargetPitchTransformModifierNamespace
{
	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

	Camera const *getCamera();

#ifdef _DEBUG
	void          renderFrame(Transform const &lhsTransform, Transform const &rhsTransform, float scale);
	void          renderSphere(Transform const &objectToWorld, Vector const &objectPosition, float radius);
#endif

	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

	ConstCharCrcLowerString const  cs_combatStateName("combat");

	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

	bool s_installed;
	bool s_disableClamp;
	bool s_disableModifier;
	bool s_print;
	bool s_renderPreFrame;
	bool s_renderPostFrame;
	bool s_renderTarget;

	float s_pitchClampMin;
	float s_pitchClampMax;

	TargetPitchTransformModifier::GetCameraFunction  s_getCameraFunction;
	AnimationStateNameId                             s_combatStateId;

}

using namespace TargetPitchTransformModifierNamespace;

// ======================================================================

Camera const *TargetPitchTransformModifierNamespace::getCamera()
{
	if (s_getCameraFunction)
		return (*s_getCameraFunction)();
	else
		return 0;
}

// ----------------------------------------------------------------------

#ifdef _DEBUG

void TargetPitchTransformModifierNamespace::renderFrame(Transform const &lhsTransform, Transform const &rhsTransform, float scale)
{
	//-- Get the camera.
	Camera const *const camera = getCamera();
	if (!camera)
		return;

	//-- Combine the two transforms.
	Transform  newTransform(Transform::IF_none);
	newTransform.multiply(lhsTransform, rhsTransform);

	camera->addDebugPrimitive(new FrameDebugPrimitive(UtilityDebugPrimitive::S_none, newTransform, scale));
}

#endif

// ----------------------------------------------------------------------

#ifdef _DEBUG

void TargetPitchTransformModifierNamespace::renderSphere(Transform const &objectToWorld, Vector const &objectPosition, float radius)
{
	//-- Get the camera.
	Camera const *const camera = getCamera();
	if (!camera)
		return;

	camera->addDebugPrimitive(new SphereDebugPrimitive(UtilityDebugPrimitive::S_none, objectToWorld, objectPosition, radius, 6, 6));
}

#endif

// ======================================================================

MEMORY_BLOCK_MANAGER_IMPLEMENTATION_WITHOUT_INSTALL(TargetPitchTransformModifier, true, 0, 0, 0);

// ======================================================================

void TargetPitchTransformModifier::install()
{
	DEBUG_FATAL(s_installed, ("TargetPitchTransformModifier already installed."));

	//-- Get pitch clamp values from config file, convert to radians.
	s_pitchClampMax = ConfigClientSkeletalAnimation::getTargetPitchClampDegrees() * PI_OVER_180;
	s_pitchClampMin = -s_pitchClampMax;

	if (s_pitchClampMax < s_pitchClampMin)
		std::swap(s_pitchClampMax, s_pitchClampMin);

	//-- Setup combat state name id.
	s_combatStateId = AnimationStateNameIdManager::createId(cs_combatStateName);

	//-- Setup debug flags.
	char const *const sectionName = "ClientSkeletalAnimation/TargetModifier";
	DebugFlags::registerFlag(s_disableModifier, sectionName, "disableModifier");

#if PRODUCTION == 0
	DebugFlags::registerFlag(s_disableClamp, sectionName, "disableClamp");
	DebugFlags::registerFlag(s_print, sectionName, "printTargetModifier");
	DebugFlags::registerFlag(s_renderPreFrame, sectionName, "renderPreFrame");
	DebugFlags::registerFlag(s_renderPostFrame, sectionName, "renderPostFrame");
	DebugFlags::registerFlag(s_renderTarget, sectionName, "renderTarget");
#endif

	installMemoryBlockManager();

	s_installed = true;
	ExitChain::add(remove, "TargetPitchTransformModifier");
}

// ----------------------------------------------------------------------

void TargetPitchTransformModifier::setGetCameraFunction(GetCameraFunction function)
{
	s_getCameraFunction = function;
}

// ======================================================================

TargetPitchTransformModifier::TargetPitchTransformModifier() :
	TransformModifier()
{
}

// ----------------------------------------------------------------------

bool TargetPitchTransformModifier::modifyTransform(float elapsedTime, Skeleton const &skeleton, CrcString const &transformName, Transform const &transform_p2o, Transform const &transform_l2p, Transform &transform_l2o)
{
	UNREF(elapsedTime);
	UNREF(transform_l2p);
	UNREF(transform_p2o);
	UNREF(transformName);

	//-- Do nothing if globally disabled.
	if (s_disableModifier)
		return false;

	//-- Skip this modifier if the appearance is not in combat.  Search for combat in the current animation state path.
	// @todo move this logic higher up --- we may want to use this for non-combat, non-spine purposes sometime in the future.
	SkeletalAppearance2 const & modifyAppearance = skeleton.getSkeletalAppearance();

	// Definitions:
	// modify object = the character that will do the pitching.
	// target object = the game object that is targeted (i.e. the object we're aiming at)

	//-- Get center of target Object relative to modified Object (in modified object's space).
	// First get modify object.
	Object const *const modifyObject = modifyAppearance.getOwner();
	NOT_NULL(modifyObject);

	// Get target object from modify object.
	Object const *const targetObject = modifyAppearance.getTargetObject();
	if (targetObject == modifyObject)
	{
		// Nothing to do.
		return false;
	}

#if (PRODUCTION == 0) && defined(_DEBUG)

	if (s_renderPreFrame)
		renderFrame(modifyObject->getTransform_o2w(), transform_l2o, 0.3f);
#endif


	//-- Find amount of pitch to apply.
	// Compare bend joint and target position in modify object space.
	Vector const & targetDirection_o = modifyAppearance.getDirectionToTarget();

#if (PRODUCTION == 0) && defined(_DEBUG)
	Vector const & targetPosition_o = modifyAppearance.getTargetPosition_o();
	if (s_renderTarget)
	{
		renderSphere(modifyObject->getTransform_o2w(), targetPosition_o, 0.20f);
		Camera const *const camera = getCamera();
		if (camera)
			camera->addDebugPrimitive(new Line3dDebugPrimitive(UtilityDebugPrimitive::S_none, modifyObject->getTransform_o2w(), transform_l2o.getPosition_p(), targetPosition_o, PackedArgb::solidRed));
	}

#endif

	DEBUG_REPORT_PRINT(s_print, ("TPTM:direction: (%.2f,%.2f,%.2f)\n", targetDirection_o.x, targetDirection_o.y, targetDirection_o.z));

	float const rawPitch = -targetDirection_o.phi();

#if PRODUCTION == 0
	float const pitchDesired = (!s_disableClamp ? clamp(s_pitchClampMin, rawPitch, s_pitchClampMax) : rawPitch);
#else
	float const pitchDesired = clamp(s_pitchClampMin, rawPitch, s_pitchClampMax);
#endif
	
	DEBUG_REPORT_PRINT(s_print, ("TPTM:pitch: %.2f\n", pitchDesired * 180.0f/PI));

	//-- Find vector along which to apply.
	// Find vector in modify space.
	Vector const pitchAxis_o = targetDirection_o.cross(Vector::unitY);

	// Bring into pre-modified local joint space.
	Vector const pitchAxis_j = transform_l2o.rotate_p2l(pitchAxis_o);

	//-- Create transform to apply desired pitch in parent joint space.
	Transform pitchTransform(Transform::IF_none);

	Quaternion const pitchQuaternion = Quaternion(pitchDesired, pitchAxis_j);
	pitchQuaternion.getTransform(&pitchTransform);

	//-- Build new joint transform.
	// Save this, we need it later.
	Vector const oldJointPosition_o = transform_l2o.getPosition_p();
	transform_l2o.setPosition_p(Vector::zero);
	
	// Build new transform.
	Transform newTransform(Transform::IF_none);

	newTransform.multiply(transform_l2o, pitchTransform);
	transform_l2o = newTransform;

	//-- Note we only wanted to change the rotation of the joint's frame, not it's position.  We must now
	//   fixup it's position.
	transform_l2o.setPosition_p(oldJointPosition_o);

#if (PRODUCTION == 0) && defined(_DEBUG)
	if (s_renderPostFrame)
		renderFrame(modifyObject->getTransform_o2w(), transform_l2o, 0.15f);
#endif

	return true;
}

// ======================================================================

void TargetPitchTransformModifier::remove()
{
	DEBUG_FATAL(!s_installed, ("TargetPitchTransformModifier not installed."));
	s_installed = false;

	removeMemoryBlockManager();
}

// ======================================================================
