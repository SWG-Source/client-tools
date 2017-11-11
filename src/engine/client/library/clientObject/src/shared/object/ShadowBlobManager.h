// ======================================================================
//
// ShadowBlobManager.h
// asommers
// 
// copyright 2003, sony online entertainment
//
// ======================================================================

#ifndef INCLUDED_ShadowBlobManager_H
#define INCLUDED_ShadowBlobManager_H

// ======================================================================

class Appearance;
class ShadowBlobObject;
class Vector;

// ======================================================================

class ShadowBlobManager
{
public:

	static void install ();

	static bool getEnabled ();
	static void setEnabled (bool enabled);

	static void enableShadowBlobObject (const ShadowBlobObject& shadowBlobObject);
	static void disableShadowBlobObject (const ShadowBlobObject& shadowBlobObject);

	static void renderShadowBlobs (const Appearance& appearance);
	static void addVertex (const Vector& position, const float u, const float v);

	static void update (float elapsedTime);

private:

	ShadowBlobManager ();
	~ShadowBlobManager ();
	ShadowBlobManager (const ShadowBlobManager&);
	ShadowBlobManager& operator= (const ShadowBlobManager&);
};

// ======================================================================

#endif
