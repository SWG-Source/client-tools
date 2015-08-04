// ======================================================================
//
// ClientEventManager.h
// Copyright 2002 Sony Online Entertainment Inc.
// All Rights Reserved.
// 
// ======================================================================

#ifndef INCLUDED_ClientEventManager_H
#define INCLUDED_ClientEventManager_H

// ======================================================================

class CellProperty;
class CrcLowerString;
class Iff;
class Object;
class Transform;
class Vector;

// ======================================================================

class ClientEventManager
{
public:
	static void install();

	static bool playEvent(const CrcLowerString& eventType, Object* object, const CrcLowerString& hardpoint);
	static bool playEvent(const CrcLowerString& eventType, Object* object, Transform const & transform);
	static bool playEvent(const CrcLowerString& eventType, Object* objectToPlayEffectOn, const Object* objectWithEventBinding, const CrcLowerString& hardpoint);
	static bool playEvent(const CrcLowerString& eventType, Object* objectToPlayEffectOn, const Object* objectWithEventBinding, const CrcLowerString& hardpoint, Transform const & transform);
	static bool playEvent(const CrcLowerString& sourceType, const CrcLowerString& destType, const CellProperty* cell, const Vector& position, const Vector& up);

	static void getEffectName(const CrcLowerString& eventType, const Object& object, CrcLowerString& effectName);

private:
	static void remove();

	static void load();
	static void load_0000(Iff& iff);

private:
	static bool ms_installed;
	static stdmap<CrcLowerString, stdmap<CrcLowerString, CrcLowerString>::fwd >::fwd *m_eventSourceDestMap;
};

// ======================================================================

#endif