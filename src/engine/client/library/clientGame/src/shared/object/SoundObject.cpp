// ======================================================================
//
// SoundObject.cpp
// Copyright 2001 Sony Online Entertainment Inc.
// All Rights Reserved.
// 
// ======================================================================

#include "clientGame/FirstClientGame.h"
#include "clientGame/SoundObject.h"

#include "clientAudio/Audio.h"
#include "clientGame/ClientWorld.h"
#include "sharedFoundation/CrcLowerString.h"
#include "sharedObject/AlterResult.h"

// ======================================================================

SoundObject::SoundObject(const char *soundPathName)
:	Object(),
	m_soundPathName(new CrcLowerString(soundPathName))
{
	addNotification(ClientWorld::getIntangibleNotification());
}

// ----------------------------------------------------------------------

SoundObject::~SoundObject()
{
	delete m_soundPathName;
}

// ----------------------------------------------------------------------

float SoundObject::alter(float deltaTime)
{
	//-- play the sound
	Audio::playSound(m_soundPathName->getString(), getPosition_w(), getParentCell());

	//-- alter ancestor
	IGNORE_RETURN(Object::alter(deltaTime));

	//-- instances of this class always die after first alter call
	return AlterResult::cms_kill;
}

// ======================================================================
