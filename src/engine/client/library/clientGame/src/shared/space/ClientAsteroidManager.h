// ============================================================================
// 
// ClientAsteroidManager.h
// Copyright Sony Online Entertainment, Inc.
//
// ============================================================================

#ifndef INCLUDED_ClientAsteroidManager_H
#define INCLUDED_ClientAsteroidManager_H

// ============================================================================

#include "sharedGame/AsteroidGenerationManager.h"

// ============================================================================

class ConstCharCrcString;
class Sphere;
class SphereDebugPrimitive;
class Vector;

// ============================================================================

class ClientAsteroidManager
{
public:
	static void install();
	static void setupStaticFields(std::string const & sceneName);
	static void update(float elapsedTime);
	static void setServerAsteroidData(stdvector<Sphere>::fwd const & spheres);
	
private:
	static void remove();

private:
	//disabled
	ClientAsteroidManager();
	~ClientAsteroidManager();
	ClientAsteroidManager(ClientAsteroidManager const &);
	ClientAsteroidManager &operator =(ClientAsteroidManager const &);
};

// ============================================================================

#endif // INCLUDED_ClientAsteroidManager_H
