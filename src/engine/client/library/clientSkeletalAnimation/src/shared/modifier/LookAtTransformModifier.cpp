// ======================================================================
//
// LookAtTransformModifier.cpp
// Copyright 2003 Sony Online Entertainment, Inc.
// All Rights Reserved.
//
// ======================================================================

#include "clientSkeletalAnimation/FirstClientSkeletalAnimation.h"
#include "clientSkeletalAnimation/LookAtTransformModifier.h"

#include "clientGraphics/Camera.h"
#include "clientGraphics/DebugPrimitive.h"
#include "clientGraphics/ShaderPrimitiveSorter.h"
#include "clientSkeletalAnimation/ConfigClientSkeletalAnimation.h"
#include "clientSkeletalAnimation/SkeletalAppearance2.h"
#include "clientSkeletalAnimation/Skeleton.h"
#include "sharedDebug/DebugFlags.h"
#include "sharedFoundation/ExitChain.h"
#include "sharedFoundation/MemoryBlockManager.h"
#include "sharedFoundation/Os.h"
#include "sharedFoundation/Production.h"
#include "sharedMath/Transform.h"
#include "sharedObject/Object.h"

// ======================================================================

namespace LookAtTransformModifierNamespace
{
	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

	Camera const *getCamera();

#ifdef _DEBUG
	void          renderFrame(Transform const &lhsTransform, Transform const &rhsTransform, float scale);
	void          renderSphere(Transform const &objectToWorld, Vector const &objectPosition, float radius);
#endif

	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

	float const PI_UNDER_180      = 180.0f / PI;

	float const cs_masterFraction = 0.40f;
	float const cs_slaveFraction  = 0.60f;
	
	float const cs_wrapAroundHalfAngleInDegrees = 20.0f;
	float const cs_wrapAroundHalfAngle          = cs_wrapAroundHalfAngleInDegrees * PI_OVER_180;
	float const cs_wrapRightMaxAngle            = PI + cs_wrapAroundHalfAngle;
	float const cs_wrapLeftMinAngle             = -PI - cs_wrapAroundHalfAngle;

	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

	bool s_installed;
	bool s_disableClamp;
	bool s_disableModifier;
	bool s_print;
	bool s_renderPreFrame;
	bool s_renderPostFrame;
	bool s_renderTarget;

	float s_yawClampMin;
	float s_yawClampMax;

	float s_maxHeadTurnSpeed;

	LookAtTransformModifier::GetCameraFunction          s_getCameraFunction;
	LookAtTransformModifier::AllowLookAtTargetFunction  s_allowLookAtTargetFunction;
}

using namespace LookAtTransformModifierNamespace;

// ======================================================================

Camera const *LookAtTransformModifierNamespace::getCamera()
{
	if (s_getCameraFunction)
		return (*s_getCameraFunction)();
	else
		return 0;
}

// ----------------------------------------------------------------------

#ifdef _DEBUG

void LookAtTransformModifierNamespace::renderFrame(Transform const &lhsTransform, Transform const &rhsTransform, float scale)
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

void LookAtTransformModifierNamespace::renderSphere(Transform const &objectToWorld, Vector const &objectPosition, float radius)
{
	//-- Get the camera.
	Camera const *const camera = getCamera();
	if (!camera)
		return;

	camera->addDebugPrimitive(new SphereDebugPrimitive(UtilityDebugPrimitive::S_none, objectToWorld, objectPosition, radius, 6, 6));
}

#endif

// ======================================================================

MEMORY_BLOCK_MANAGER_IMPLEMENTATION_WITHOUT_INSTALL(LookAtTransformModifier, true, 0, 0, 0);

// ======================================================================

void LookAtTransformModifier::install()
{
	DEBUG_FATAL(s_installed, ("LookAtTransformModifier already installed."));

	//-- Get yaw clamp values from config file, convert to radians.
	s_yawClampMax = ConfigClientSkeletalAnimation::getLookAtYawClampDegrees() * PI_OVER_180;
	s_yawClampMin = -s_yawClampMax;

	if (s_yawClampMax < s_yawClampMin)
		std::swap(s_yawClampMax, s_yawClampMin);

	//-- Get max head turn speed from config file (in degrees per second)
	s_maxHeadTurnSpeed = ConfigClientSkeletalAnimation::getMaxHeadTurnSpeed() * PI_OVER_180;

	//-- Setup debug flags.
	char const *const sectionName = "ClientSkeletalAnimation/LookAtModifier";
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
	ExitChain::add(remove, "LookAtTransformModifier");
}

// ----------------------------------------------------------------------

void LookAtTransformModifier::setGetCameraFunction(GetCameraFunction function)
{
	s_getCameraFunction = function;
}

// ----------------------------------------------------------------------

void LookAtTransformModifier::setAllowLookAtTargetFunction(AllowLookAtTargetFunction function)
{
	s_allowLookAtTargetFunction = function;
}

// ======================================================================

LookAtTransformModifier::LookAtTransformModifier() :
	TransformModifier(),
	m_master(0),
	m_evaluateFrameNumber(-1),
	m_totalYawCorrection(0.0f)
{
}

// ----------------------------------------------------------------------

bool LookAtTransformModifier::modifyTransform(float elapsedTime, Skeleton const &skeleton, CrcString const & /* transformName */, Transform const &transform_p2o, Transform const &transform_l2p, Transform &transform_l2o)
{
	UNREF(transform_p2o);
	UNREF(transform_l2p);
	UNREF(transform_l2o);

	//-- Do nothing if globally disabled.
	if (s_disableModifier)
		return false;

	//-- Get angle to yaw in joint local space.
	float yawAngle;

	LookAtTransformModifier const *const master = getMaster();
	if (master)
	{
		//-- This is the slave.  Get angle from master.
		yawAngle = master->getSlaveYawRotation();
	}
	else
	{
		//-- This is the master.  If we've already calculated it this frame, skip it.
		int const currentFrameNumber = Os::getNumberOfUpdates();
		if (currentFrameNumber == m_evaluateFrameNumber)
		{
			//-- Use already calculated master yaw value.
			yawAngle = m_totalYawCorrection * cs_masterFraction;
		}
		else
		{
			//-- Calculate master yaw rotation.
			float const oldYawCorrection = m_totalYawCorrection;
			float const newYawCorrection = calculateTotalYawCorrection(elapsedTime, skeleton, transform_l2o);

			//-- Clamp the angle.
			float const maxAngle = s_maxHeadTurnSpeed * elapsedTime;
			float const curAngle = newYawCorrection - oldYawCorrection;
			m_totalYawCorrection = oldYawCorrection + clamp(-maxAngle, curAngle, maxAngle);

			//-- Calculate the current yaw.
			yawAngle              = m_totalYawCorrection * cs_masterFraction;
			m_evaluateFrameNumber = currentFrameNumber;
		}
	}

	// Definitions:
	// modify object = the character that will do the yawing.
	// target object = the game object that is targeted (i.e. the object we're aiming at)

#if (PRODUCTION == 0) && defined(_DEBUG)
	SkeletalAppearance2 const &modifyAppearance = skeleton.getSkeletalAppearance();
	Object const *const modifyObject = modifyAppearance.getOwner();
	NOT_NULL(modifyObject);

	if (s_renderPreFrame)
		renderFrame(modifyObject->getTransform_o2w(), transform_l2o, 0.3f);
#endif

	//-- Save joint position in parent space, we only want to change its rotation.
	Vector const oldJointPosition_o = transform_l2o.getPosition_p();

	//-- Perform the rotation.
	transform_l2o.yaw_l(yawAngle);

	//-- Place it back in the same position within parent space.
	transform_l2o.setPosition_p(oldJointPosition_o);

#if (PRODUCTION == 0) && defined(_DEBUG)
	if (s_renderPostFrame)
		renderFrame(modifyObject->getTransform_o2w(), transform_l2o, 0.15f);
#endif

	return true;
}

// ======================================================================

void LookAtTransformModifier::remove()
{
	DEBUG_FATAL(!s_installed, ("LookAtTransformModifier not installed."));
	s_installed = false;

	removeMemoryBlockManager();
}

// ======================================================================

float LookAtTransformModifier::getSlaveYawRotation() const
{
	//-- Assumed to be called by the slave, which is assumed to be called after the master.
	//   We do not check for evaluation here.
	return m_totalYawCorrection * cs_slaveFraction;
}

// ----------------------------------------------------------------------

float LookAtTransformModifier::calculateTotalYawCorrection(float elapsedTime, Skeleton const &skeleton, Transform const &transform_l2o) const
{
	UNREF(transform_l2o);
	
	//-- Get center of target Object relative to modified Object (in modified object's space).
	// First get modify object.
	SkeletalAppearance2 const & modifyAppearance = skeleton.getSkeletalAppearance();

	//-- Check if app will allow this skeletal appearance to look at its look-at target.
	//   The app can disallow looking at the look at target based on game rules such as
	//   disallowing based on posture.
	if (s_allowLookAtTargetFunction)
	{
		bool const allow = (*s_allowLookAtTargetFunction)(modifyAppearance);
		if (!allow)
		{
			//-- Don't allow looking at the target, so total yaw correction is 0.0f.
			return 0.0f;
		}
	}

	Object const *const modifyObject = modifyAppearance.getOwner();
	NOT_NULL(modifyObject);

	//-- Construct direction to target from target position.
	Vector const & targetDirection_j = modifyAppearance.getDirectionToTarget();

#if (PRODUCTION == 0) && defined(_DEBUG)
	// Get target object from modify object.
	Vector const & targetPosition_o = modifyAppearance.getTargetPosition_o();
	Object const *const targetObject = modifyAppearance.getTargetObject();
	if (s_renderTarget && targetObject)
	{
		renderSphere(Transform::identity, modifyAppearance.getTargetPosition_w(), 0.20f);
		renderSphere(modifyObject->getTransform_o2w(), targetPosition_o, 0.30f);
		Camera const *const camera = getCamera();
		if (camera)
			camera->addDebugPrimitive(new Line3dDebugPrimitive(UtilityDebugPrimitive::S_none, modifyObject->getTransform_o2w(), transform_l2o.getPosition_p(), targetPosition_o, PackedArgb::solidBlue));
	}
#endif

	DEBUG_REPORT_PRINT(s_print, ("LATM:direction: (%.2f,%.2f,%.2f)\n", targetDirection_j.x, targetDirection_j.y, targetDirection_j.z));

	float targetYaw = targetDirection_j.theta();

	//-- Adjust yaw for head flipping.
	if (targetYaw < 0.0f)
	{
		// Check if we went right last frame.
		if ((m_totalYawCorrection >= 0.0f) && ((targetYaw + PI_TIMES_2) <= cs_wrapRightMaxAngle))
		{
			// wrap around right.
			targetYaw += PI_TIMES_2;
		}
	}
	else
	{
		// Check if we went left last frame.
		if ((m_totalYawCorrection < 0.0f) && ((targetYaw - PI_TIMES_2) >= cs_wrapLeftMinAngle))
		{
			// wrap around left.
			targetYaw -= PI_TIMES_2;
		}
	}

	//-- Clamp target yaw.
#if PRODUCTION == 0
	float const clampedTargetYaw = (!s_disableClamp ? clamp(s_yawClampMin, targetYaw, s_yawClampMax) : targetYaw);
#else
	float const clampedTargetYaw = clamp(s_yawClampMin, targetYaw, s_yawClampMax);
#endif

	//-- Clamp for rotation rate.
	float const maxAngle             = s_maxHeadTurnSpeed * elapsedTime;
	float const deltaYaw             = clampedTargetYaw - m_totalYawCorrection;

	float const timeClampedTargetYaw = clamp(-maxAngle, deltaYaw, maxAngle);

	DEBUG_REPORT_PRINT(s_print, ("LATM:yaw: %.2f; delta: %.2f\n", (m_totalYawCorrection + deltaYaw) * PI_UNDER_180, deltaYaw * PI_UNDER_180));

	return m_totalYawCorrection + timeClampedTargetYaw;
}

// ======================================================================
