// ======================================================================
//
// SetupClientParticle.h
// Copyright Sony Online Entertainment
//
// ======================================================================

#ifndef INCLUDED_SetupClientParticle_H
#define INCLUDED_SetupClientParticle_H

// ======================================================================

class Object;

// ======================================================================

class SetupClientParticle
{
public:

	typedef void (*AddNotificationFunction) (Object& object);

public:

	static void install ();
	static void setAddNotificationFunction (AddNotificationFunction addNotificationFunction);
	static void addNotifications (Object& object);

private:

	static AddNotificationFunction ms_addNotificationFunction;

private:

	SetupClientParticle (void);
	SetupClientParticle (const SetupClientParticle&);
	SetupClientParticle& operator= (const SetupClientParticle&);
};

// ======================================================================

#endif 
