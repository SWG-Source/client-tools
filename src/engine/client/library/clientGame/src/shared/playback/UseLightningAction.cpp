// ======================================================================
//
// UseLightningAction.cpp
// Copyright 2003 Sony Online Entertainment, Inc.
// All Rights Reserved.
//
// ======================================================================

#include "clientGame/FirstClientGame.h"
#include "clientGame/UseLightningAction.h"

#include "clientAnimation/PlaybackScript.h"
#include "clientAudio/Audio.h"
#include "clientGame/ClientWorld.h"
#include "clientGame/UseLightningActionTemplate.h"
#include "clientGraphics/RenderWorld.h"
#include "clientParticle/LightningAppearance.h"
#include "clientParticle/LightningAppearanceTemplate.h"
#include "clientSkeletalAnimation/SkeletalAppearance2.h"
#include "sharedDebug/InstallTimer.h"
#include "sharedFile/TreeFile.h"
#include "sharedFoundation/ExitChain.h"
#include "sharedFoundation/MemoryBlockManager.h"
#include "sharedMath/Vector.h"
#include "sharedObject/AppearanceTemplateList.h"
#include "sharedObject/MemoryBlockManagedObject.h"
#include "sharedRandom/Random.h"

// ======================================================================

namespace UseLightningActionNamespace
{
	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

	bool                               s_installed;

	std::string                        s_hardpointName;
	Transform                          s_hardpointTransform;

	std::string                        s_nameString;
}

using namespace UseLightningActionNamespace;

// ======================================================================

MEMORY_BLOCK_MANAGER_IMPLEMENTATION_WITHOUT_INSTALL(UseLightningAction, true, 0, 0, 0);

// ======================================================================
// class UseLightningAction: PUBLIC STATIC
// ======================================================================

void UseLightningAction::install()
{
	InstallTimer const installTimer("UseLightningAction::install");

	DEBUG_FATAL(s_installed, ("UseLightningAction already installed."));

	installMemoryBlockManager();

	s_installed = true;
	ExitChain::add(remove, "UseLightningAction");
}

// ======================================================================
// class UseLightningAction: PUBLIC
// ======================================================================

UseLightningAction::~UseLightningAction()
{
	realCleanup();
} //lint !e1740 // m_appearanceTemplate not freed or zeroed by destructor // Wrong, look at realCleanup()

// ----------------------------------------------------------------------

void UseLightningAction::cleanup(PlaybackScript & /* script */)
{
	realCleanup();
}

// ----------------------------------------------------------------------

bool UseLightningAction::update(float deltaTime, PlaybackScript &script)
{
	//-- Handle first time initialization.
	if (!m_started)
	{
		// Do not consume deltaTime.  Act like this is just the starting trigger.
		m_started = true;
		return initialize(script);
	}

	//-- Update base rotation angle.
	m_boltBaseRotationInRadians += deltaTime * m_rotationRateInRadiansPerSecond;

	//-- Increment current time.
	m_currentTime = std::min(m_totalTime, m_currentTime + deltaTime);
	
	//-- Figure out start and end fraction for the lightning bolts based on timers.
	float boltStartFraction;

	if (m_currentTime < m_deathStartTime)
		boltStartFraction = 0.0f;
	else
	{
		if (m_deathStartTime < m_totalTime)
			boltStartFraction = (m_currentTime - m_deathStartTime) / (m_totalTime - m_deathStartTime);
		else
			boltStartFraction = 1.0f;
	}

	float boltEndFraction;

	if (m_currentTime > m_growEndTime)
		boltEndFraction = 1.0f;
	else
	{
		if (m_growEndTime > 0.0f)
			boltEndFraction = m_currentTime / m_growEndTime;
		else
			boltEndFraction = 1.0f;
	}

	//-- Update the bolts.
	for (int boltIndex = 0; boltIndex < m_boltCount; ++boltIndex)
	{
		bool const result = updateBolt(boltIndex, boltStartFraction, boltEndFraction);
		if (!result)
		{
			DEBUG_REPORT_LOG(true, ("UseLightningAction: killing because updateBolt(%d) failed.\n", boltIndex));
			return false;
		}
	}

	//-- Handle end of bolts.
	if (m_currentTime >= m_totalTime)
	{
		//-- Delete bolts, stop looping sound, free appearance template.
		realCleanup();

		//-- Play end sound.
		if (m_defenderWatcher.getPointer())
		{
			bool const gotEndSound = script.getStringVariable(getOurActionTemplate().getEndSoundVariableTag(), s_nameString);
			if (gotEndSound && !s_nameString.empty())
				IGNORE_RETURN(Audio::attachSound(s_nameString.c_str(), m_defenderWatcher.getPointer(), ""));
		}
	}

	return (m_currentTime < m_totalTime);
}

// ======================================================================
// class UseLightningAction: PRIVATE STATIC
// ======================================================================

void UseLightningAction::remove()
{
	DEBUG_FATAL(!s_installed, ("UseLightningAction not instlled."));
	s_installed = false;

	removeMemoryBlockManager();
}

// ======================================================================
// class UseLightningAction: PRIVATE
// ======================================================================

UseLightningAction::UseLightningAction(UseLightningActionTemplate const &actionTemplate):
	PlaybackAction(&actionTemplate),
	m_started(false),
	m_currentTime(0.0f),
	m_totalTime(1.0f),
	m_growEndTime(0.25f),
	m_deathStartTime(0.75f),
	m_level(1),
	m_attackerWatcher(),
	m_defenderWatcher(),
	m_hardpointCount(0),
	m_boltCount(0),
	m_boltObject(),
	m_boltRotationRadius(0.0f),
	m_boltRadianAngleOffsetArray(),
	m_boltBaseRotationInRadians(0.0f),
	m_rotationRateInRadiansPerSecond(PI / 1.0f),
	m_loopSoundId(),
	m_appearanceTemplate(0)
{ //lint !e1926 // m_emitterHardpointNameArray's default constructor implicitly called. // Show me how to explicitly call it without making Lint bitch and I will.
	m_loopSoundId.invalidate();
}

// ----------------------------------------------------------------------

UseLightningActionTemplate const &UseLightningAction::getOurActionTemplate() const
{
	return *NON_NULL(safe_cast<UseLightningActionTemplate const*>(getPlaybackActionTemplate()));
}

// ----------------------------------------------------------------------

void UseLightningAction::realCleanup()
{
	//-- Stop loop sound
	if (m_loopSoundId.isValid())
	{
		Audio::stopSound(m_loopSoundId);
		m_loopSoundId.invalidate();
	}

	//-- Release appearance template.
	if (m_appearanceTemplate != NULL)
	{
		AppearanceTemplateList::release(m_appearanceTemplate);
		m_appearanceTemplate = NULL;
	}

	//-- Ensure bolts are killed off.
	if (m_boltObject != NULL)
	{
		m_boltObject->kill();
	}
}

// ----------------------------------------------------------------------

bool UseLightningAction::initialize(PlaybackScript &script)
{
	UseLightningActionTemplate const &actionTemplate = getOurActionTemplate();

	//-- Fetch the appearance template for the effect.
	bool const gotAppearanceTemplateName = script.getStringVariable(actionTemplate.getAppearanceTemplateVariableTag(), s_nameString);
	if (!gotAppearanceTemplateName)
	{
		DEBUG_REPORT_LOG(true, ("UseLightningAction: aborting action because playback script didn't specify the appearance template name.\n"));
		return false;
	}

	//-- Fetch the appearance template.
	DEBUG_WARNING(!TreeFile::exists(s_nameString.c_str()), ("UseLightningAction: specified appearance template [%s] does not exist, default appearance will be used.", s_nameString.c_str()));
	m_appearanceTemplate = AppearanceTemplateList::fetch(s_nameString.c_str());

	if (m_appearanceTemplate == NULL)
	{
		DEBUG_REPORT_LOG(true, ("UseLightningAction: aborting action because specified AppearanceTemplate name [%s] doesn't exist.\n", s_nameString.c_str()));
		return false;
	}

	//-- Setup object watchers.
	m_attackerWatcher = script.getActor(actionTemplate.getAttackerActorIndex());
	Object const *const attackerObject = m_attackerWatcher.getPointer();

	if (attackerObject == NULL)
	{
		DEBUG_REPORT_LOG(true, ("UseLightningAction: aborting action because attacker actor is NULL.\n"));
		return false;
	}

	Appearance const *const attackerBaseAppearance      = attackerObject->getAppearance();
	SkeletalAppearance2 const *const attackerAppearance = attackerBaseAppearance ? attackerBaseAppearance->asSkeletalAppearance2() : NULL;
	Object const *const animationTarget                 = (attackerAppearance ? attackerAppearance->getTargetObject() : NULL);
	m_defenderWatcher = (animationTarget ? animationTarget : script.getActor(actionTemplate.getDefenderActorIndex()));

	if (m_defenderWatcher.getPointer() == NULL)
	{
		DEBUG_REPORT_LOG(true, ("UseLightningAction: aborting action because defender actor is NULL.\n"));
		return false;
	}

	//-- Get attacker hardpoint names.  We support one or two hardpoints for one or two lightning streams.
	//   If a hardpoint name is the empty string, we don't play anything on that channel.
	bool const gotHardpointName1 = script.getStringVariable(actionTemplate.getHardpointVariableTag(0), s_hardpointName);
	if (gotHardpointName1 && (s_hardpointName.length() > 0))
	{
		m_emitterHardpointNameArray[0].set(s_hardpointName.c_str(), true);
		++m_hardpointCount;
	}

	bool const gotHardpointName2 = script.getStringVariable(actionTemplate.getHardpointVariableTag(1), s_hardpointName);
	if (gotHardpointName2 && (s_hardpointName.length() > 0))
	{
		m_emitterHardpointNameArray[1].set(s_hardpointName.c_str(), true);
		++m_hardpointCount;
	}

	if (m_hardpointCount < 1)
	{
		DEBUG_REPORT_LOG(true, ("UseLightningAction: aborting action because combat manager didn't specify any emitter hardpoints.\n"));
		return false;
	}

	//-- Get effect level.
	bool const gotLevel = script.getIntVariable(actionTemplate.getLevelVariableTag(), m_level);

	if (!gotLevel)
	{
		DEBUG_REPORT_LOG(true, ("UseLightningAction: aborting action because playback script didn't specify a level for the action.\n"));
		return false;
	}

	if ((m_level < 1) || (m_level > LightningAppearance::getMaxLightningBoltCount()))
	{
		DEBUG_REPORT_LOG(true, ("UseLightningAction: aborting action because playback script specified an effect level [%d] out of valid range [1 .. %d].\n", m_level, LightningAppearance::getMaxLightningBoltCount()));
		return false;
	}

	// We'll fire a count of level bolts from each hardpoint.
	m_boltCount = m_level;

	//-- Setup timers.

	// Get total time.
	bool const gotTotalTime = script.getFloatVariable(actionTemplate.getTotalTimeVariableTag(), m_totalTime);
	if (!gotTotalTime)
	{
		DEBUG_REPORT_LOG(true, ("UseLightningAction: aborting action because combat manager didn't specify a total time for the action.\n"));
		return false;
	}

	if (m_totalTime <= 0.0f)
	{
		DEBUG_REPORT_LOG(true, ("UseLightningAction: aborting action because combat manager specified an invalid total time [%g] for the action.\n", m_totalTime));
		return false;
	}

	// Get end of grow time.
	float growTimeFraction = 0.0f;
	bool const gotGrowTimeFraction = script.getFloatVariable(actionTemplate.getGrowTimeFractionVariableTag(), growTimeFraction);
	if (!gotGrowTimeFraction)
	{
		DEBUG_REPORT_LOG(true, ("UseLightningAction: aborting action because combat manager didn't specify a grow time fraction for the action.\n"));
		return false;
	}

	// Get start of death time.
	if ((growTimeFraction < 0.0f) || (growTimeFraction > 1.0f))
	{
		DEBUG_REPORT_LOG(true, ("UseLightningAction: aborting action because grow time fraction [%g] was out of valid range [0.0f .. 1.0f].\n", growTimeFraction));
		return false;
	}

	m_growEndTime = growTimeFraction * m_totalTime;

	// Get end of grow time.
	float deathTimeFraction = 0.0f;
	bool const gotDeathTimeFraction = script.getFloatVariable(actionTemplate.getDeathTimeFractionVariableTag(), deathTimeFraction);
	if (!gotDeathTimeFraction)
	{
		DEBUG_REPORT_LOG(true, ("UseLightningAction: aborting action because combat manager didn't specify a death time fraction for the action.\n"));
		return false;
	}

	// Get start of death time.
	if ((deathTimeFraction < 0.0f) || (deathTimeFraction > 1.0f))
	{
		DEBUG_REPORT_LOG(true, ("UseLightningAction: aborting action because death time fraction [%g] was out of valid range [0.0f .. 1.0f].\n", deathTimeFraction));
		return false;
	}

	m_deathStartTime = m_totalTime * (1.0f - deathTimeFraction);

	//-- Play sounds.
	// ... startup sound.
	bool const gotBeginSound = script.getStringVariable(actionTemplate.getBeginSoundVariableTag(), s_nameString);
	if (gotBeginSound && !s_nameString.empty())
		IGNORE_RETURN(Audio::attachSound(s_nameString.c_str(), m_attackerWatcher.getPointer(), m_emitterHardpointNameArray[0].getString()));

	// ... loop sound.
	bool const gotLoopSound = script.getStringVariable(actionTemplate.getLoopSoundVariableTag(), s_nameString);
	if (gotLoopSound && !s_nameString.empty())
	{
		//-- Play loop sound.
		m_loopSoundId = Audio::attachSound(s_nameString.c_str(), m_attackerWatcher.getPointer(), m_emitterHardpointNameArray[0].getString());
	}

	//-- Initialize base rotation angle to something random.
	m_boltBaseRotationInRadians = Random::randomReal(2.0f * PI);

	//-- Retrieve beam rotation radius.
	bool const gotRadius = script.getFloatVariable(actionTemplate.getBeamRotationRadiusVariableTag(), m_boltRotationRadius);
	if (!gotRadius)
	{
		DEBUG_REPORT_LOG(true, ("UseLightningAction: aborting action because PST didn't specify a rotation radius.\n"));
		return false;
	}

	//-- Retrieve beam rotation rate, convert from degrees per second to radians per second.
	float rateInDegreesPerSecond = 0.0f;
	bool const gotDpsRate = script.getFloatVariable(actionTemplate.getBeamDpsRotationRateVariableTag(), rateInDegreesPerSecond);
	if (!gotDpsRate)
	{
		DEBUG_REPORT_LOG(true, ("UseLightningAction: aborting action because PST didn't specify a rotation rate.\n"));
		return false;
	}

	m_rotationRateInRadiansPerSecond = rateInDegreesPerSecond * PI_OVER_180;

	//-- Initialize 
	m_boltRadianAngleOffsetArray.reserve(static_cast<FloatVector::size_type>(m_level));
	float const factor = 2.0f * PI / static_cast<float>(m_level);

	for (int i = 0; i < m_level; ++i)
		m_boltRadianAngleOffsetArray.push_back(static_cast<float>(i) * factor);

	//-- Success.
	return true;
}

// ----------------------------------------------------------------------

bool UseLightningAction::updateBolt(int boltIndex, float startFraction, float endFraction)
{
	DEBUG_FATAL(m_boltCount < 1, ("unexpected: m_boltCount < 1 [%d]", m_boltCount));
	VALIDATE_RANGE_INCLUSIVE_EXCLUSIVE(0, boltIndex, m_boltCount);

	//-- Create the bolt object and appearance if it doesn't yet exist.

	if (m_boltObject == NULL)
	{
		// Create the object.
		m_boltObject = new MemoryBlockManagedObject();

		// Set position to something other than 0,0,0.
		m_boltObject->setPosition_p(Vector::xyz111);

		// Create appearance.
		NOT_NULL(m_appearanceTemplate);

		LightningAppearance *const newAppearance = safe_cast<LightningAppearance*>(m_appearanceTemplate->createAppearance());
		NOT_NULL(newAppearance);

		// Feed the new appearance some random time so they don't all move lock-step.
		// newAppearance->alter(Random::randomReal(m_totalTime));

		// Set appearance for object.
		m_boltObject->setAppearance(newAppearance);

		// Set appearance attributes.
		//newAppearance->setAutoDelete(true);
		newAppearance->setEnabled(true);
		newAppearance->setPaused(false);

		// Add render world notifictions.
		m_boltObject->addNotification(ClientWorld::getIntangibleNotification());
		RenderWorld::addObjectNotifications(*m_boltObject);

		// Add to world.
		m_boltObject->addToWorld();
	}

	//-- Get the appearance.
	NOT_NULL(m_boltObject);
	LightningAppearance *const boltAppearance = safe_cast<LightningAppearance*>(m_boltObject->getAppearance());
	NOT_NULL(boltAppearance);

	//-- Get the start and end positions.
	// ... get attacker object and appearance.
	Object const *const attackerObject = m_attackerWatcher.getPointer();
	if (!attackerObject)
	{
		DEBUG_REPORT_LOG(true, ("UseLightningAction: aborted because attacker object died.\n"));
		return false;
	}

	Appearance const *const attackerAppearance = attackerObject->getAppearance();
	if (!attackerAppearance)
	{
		DEBUG_REPORT_LOG(true, ("UseLightningAction: aborted because attacker appearance was NULL.\n"));
		return false;
	}

	// ... find attacker hardpoint.
	int const hardpointIndex = boltIndex / m_level;
	bool const gotHardpoint = attackerAppearance->findHardpoint(m_emitterHardpointNameArray[hardpointIndex], s_hardpointTransform);
	if (!gotHardpoint)
	{
		DEBUG_REPORT_LOG(true, ("UseLightningAction: aborted because attacker hardpoint [%s] was not found.\n", m_emitterHardpointNameArray[hardpointIndex].getString()));
		return false;
	}

	Vector const startPosition_w = attackerObject->rotateTranslate_o2w(s_hardpointTransform.getPosition_p());

	// ... from defender.
	Object const *const defenderObject = m_defenderWatcher.getPointer();
	if (!defenderObject)
	{
		DEBUG_REPORT_LOG(true, ("UseLightningAction: aborted because defender object died.\n"));
		return false;
	}

	// ... get attacker's x and y axis in world space.
	Vector const circleAxisX_w = attackerObject->rotate_o2w(Vector::unitX);
	Vector const circleAxisY_w = attackerObject->rotate_o2w(Vector::unitY);

	// ... compute this beam's x and y offset due to beam rotation around longitudinal axis.
	FloatVector::size_type rotationOffsetIndex = static_cast<FloatVector::size_type>(boltIndex % m_level);
	VALIDATE_RANGE_INCLUSIVE_EXCLUSIVE(0, static_cast<int>(rotationOffsetIndex), static_cast<int>(m_boltRadianAngleOffsetArray.size()));

	float const angleInRadians = m_boltBaseRotationInRadians + m_boltRadianAngleOffsetArray[rotationOffsetIndex];
	float const circleX        = m_boltRotationRadius * cos(angleInRadians);
	float const circleY        = m_boltRotationRadius * sin(angleInRadians);

	Vector const rotationOffsetX_w = circleAxisX_w * circleX;
	Vector const rotationOffsetY_w = circleAxisY_w * circleY;

	// ... get defender hardpoint.  We're going to go 3/4 diameter up defender's local y axis, centered on appearance center in defender local x-z.
	Vector const endPosition_w = defenderObject->getAppearanceSphereCenter_w() 
			+ (0.2f * defenderObject->getAppearanceSphereRadius()) * defenderObject->getTransform_o2w().rotate_l2p(Vector::unitY)
			+ rotationOffsetX_w + rotationOffsetY_w;
 
	//-- Ensure bolt starts in same cell as attacker.
	m_boltObject->setParentCell(attackerObject->getParentCell());

	//-- Figure out bolt start and end based on start and end fraction.
	Vector const deltaPosition_w = endPosition_w - startPosition_w;
	Vector const boltStart_w     = startPosition_w + startFraction * deltaPosition_w;
	Vector const boltEnd_w       = startPosition_w + endFraction   * deltaPosition_w;

	//-- Set bolt start and end positions.
	boltAppearance->setPosition_w(boltIndex, boltStart_w, boltEnd_w);

	//-- Success.
	return true;
}

// ======================================================================
