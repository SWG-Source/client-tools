// ======================================================================
//
// ProjectileManager.h
// asommers
//
// copyright 2004, sony online entertainment
//
// ======================================================================

#ifndef INCLUDED_ProjectileManager_H
#define INCLUDED_ProjectileManager_H

// ======================================================================

#include "sharedMath/Transform.h"

class Object;

// ======================================================================

class ProjectileManager
{
public:

	static void install();

	//-- Create a projectile at the object's current position and send message to server
	static void createClientProjectile(Object * object, int weaponIndex, int projectileIndex, float speed, float expireTime);

	//-- Receive a request from the server to create a projectile on the client
	static void createServerProjectile(Object * object, int weaponIndex, int projectileIndex, Transform const & transform_p, float speed, float expireTime);

	static bool stopBeam(Object & object, int weaponIndex, bool notifyServer);

private:

	ProjectileManager();
	~ProjectileManager();
	ProjectileManager(ProjectileManager const &);
	ProjectileManager & operator= (ProjectileManager const &);
};

// ======================================================================

#endif
