// ======================================================================
//
// SystemInstaller.cpp
// Copyright 2002 Sony Online Entertainment, Inc.
// All Rights Reserved.
//
// ======================================================================

#include "AnimationEditor/FirstAnimationEditor.h"
#include "AnimationEditor/SystemInstaller.h"

#include "AnimationEditor/QtActionGeneratorSkeletalAnimationTemplateUiFactory.h"
#include "AnimationEditor/QtDirectionSkeletalAnimationTemplateUiFactory.h"
#include "AnimationEditor/QtPriorityBlendAnimationTemplateUiFactory.h"
#include "AnimationEditor/QtProxySkeletalAnimationTemplateUiFactory.h"
#include "AnimationEditor/QtSpeedSkeletalAnimationTemplateUiFactory.h"
#include "AnimationEditor/QtStringSelectorSkeletalAnimationTemplateUiFactory.h"
#include "AnimationEditor/QtTimeScaleSkeletalAnimationTemplateUiFactory.h"
#include "AnimationEditor/QtYawSkeletalAnimationTemplateUiFactory.h"
#include "AnimationEditor/SkeletalAnimationTemplateFactory.h"
#include "AnimationEditor/TemplatedAnimationTemplateData.h"
#include "clientSkeletalAnimation/ActionGeneratorSkeletalAnimationTemplate.h"
#include "clientSkeletalAnimation/DirectionSkeletalAnimationTemplate.h"
#include "clientSkeletalAnimation/PriorityBlendAnimationTemplate.h"
#include "clientSkeletalAnimation/ProxySkeletalAnimationTemplate.h"
#include "clientSkeletalAnimation/SpeedSkeletalAnimationTemplate.h"
#include "clientSkeletalAnimation/StringSelectorSkeletalAnimationTemplate.h"
#include "clientSkeletalAnimation/TimeScaleSkeletalAnimationTemplate.h"
#include "clientSkeletalAnimation/YawSkeletalAnimationTemplate.h"

// ======================================================================

void SystemInstaller::preMainWindowInstall(HINSTANCE appInstanceHandle, const char *appCommandLine)
{
	UNREF(appInstanceHandle);
	UNREF(appCommandLine);
}

// ----------------------------------------------------------------------

void SystemInstaller::postMainWindowInstall()
{
	SkeletalAnimationTemplateFactory::install();

	//-- Setup supported SkeletalAnimationTemplate classes.  This involves
	//   setting a UiFactory for the SkeletalAnimationTemplate-derived class,
	//   and registering the SkeletalAnimationTemplate-derived class with
	//   the AnimationEditor's SkeletalAnimationTemplateFactory.  The latter
	//   step associated a name with a type of SkeletalAnimationTemplate,
	//   and allows the code to create any type of registered SkeletalAnimationTemplate
	//   from within the UI.

	// Setup ProxySkeletalAnimationTemplate.
	ProxySkeletalAnimationTemplate::setUiFactory(new QtProxySkeletalAnimationTemplateUiFactory());
	SkeletalAnimationTemplateFactory::registerAnimationTemplateData(new TemplatedAnimationTemplateData<ProxySkeletalAnimationTemplate>("ANS File Animation (a)"), static_cast<char>(Qt::Key_A));

	// Setup ActionGeneratorSkeletalAnimationTemplate.
	ActionGeneratorSkeletalAnimationTemplate::setUiFactory(new QtActionGeneratorSkeletalAnimationTemplateUiFactory());
	SkeletalAnimationTemplateFactory::registerAnimationTemplateData(new TemplatedAnimationTemplateData<ActionGeneratorSkeletalAnimationTemplate>("Action Generator Animation (x)"), static_cast<char>(Qt::Key_X));

	// Setup DirectionSkeletalAnimationTemplate.
	DirectionSkeletalAnimationTemplate::setUiFactory(new QtDirectionSkeletalAnimationTemplateUiFactory());
	SkeletalAnimationTemplateFactory::registerAnimationTemplateData(new TemplatedAnimationTemplateData<DirectionSkeletalAnimationTemplate>("Direction Chooser Animation (d)"), static_cast<char>(Qt::Key_D));

	// Setup PriorityBlendAnimationTemplate.
	PriorityBlendAnimationTemplate::setUiFactory(new QtPriorityBlendAnimationTemplateUiFactory());
	SkeletalAnimationTemplateFactory::registerAnimationTemplateData(new TemplatedAnimationTemplateData<PriorityBlendAnimationTemplate>("Priority Blend Animation (p)"), static_cast<char>(Qt::Key_P));

	// Setup SpeedSkeletalAnimationTemplate.
	SpeedSkeletalAnimationTemplate::setUiFactory(new QtSpeedSkeletalAnimationTemplateUiFactory());
	SkeletalAnimationTemplateFactory::registerAnimationTemplateData(new TemplatedAnimationTemplateData<SpeedSkeletalAnimationTemplate>("Speed Chooser Animation (v)"), static_cast<char>(Qt::Key_V));

	// Setup StringSelectorSkeletalAnimationTemplate.
	StringSelectorSkeletalAnimationTemplate::setUiFactory(new QtStringSelectorSkeletalAnimationTemplateUiFactory());
	SkeletalAnimationTemplateFactory::registerAnimationTemplateData(new TemplatedAnimationTemplateData<StringSelectorSkeletalAnimationTemplate>("String Selector Animation (s)"), static_cast<char>(Qt::Key_S));

	// Setup TimeScaleSkeletalAnimationTemplate.
	TimeScaleSkeletalAnimationTemplate::setUiFactory(new QtTimeScaleSkeletalAnimationTemplateUiFactory());
	SkeletalAnimationTemplateFactory::registerAnimationTemplateData(new TemplatedAnimationTemplateData<TimeScaleSkeletalAnimationTemplate>("Time Scaler Animation (t)"), static_cast<char>(Qt::Key_T));

	// Setup YawSkeletalAnimationTemplate.
	YawSkeletalAnimationTemplate::setUiFactory(new QtYawSkeletalAnimationTemplateUiFactory());
	SkeletalAnimationTemplateFactory::registerAnimationTemplateData(new TemplatedAnimationTemplateData<YawSkeletalAnimationTemplate>("Yaw Chooser Animation (y)"), static_cast<char>(Qt::Key_Y));
}

// ----------------------------------------------------------------------

void SystemInstaller::remove()
{
}

// ======================================================================
