// ======================================================================
//
// SoundObject.h
// Copyright 2001 Sony Online Entertainment Inc.
// All Rights Reserved.
// 
// ======================================================================

#ifndef INCLUDED_SoundObject_H
#define INCLUDED_SoundObject_H

// ======================================================================

#include "sharedObject/Object.h"

class CrcLowerString;

// ======================================================================

class SoundObject: public Object
{
public:

	SoundObject(const char *soundPathName);
	virtual ~SoundObject();

	virtual float alter(float deltaTime);

private:

	// disabled
	SoundObject();
	SoundObject(const SoundObject&);
	SoundObject &operator =(const SoundObject&);

private:

	CrcLowerString      *m_soundPathName;

};

// ======================================================================

#endif
