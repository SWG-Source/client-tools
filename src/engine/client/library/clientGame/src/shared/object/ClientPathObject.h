// ======================================================================
//
// ClientPathObject.h
// asommers
//
// copyright 2003, sony online entertainment
//
// ======================================================================

#ifndef INCLUDED_ClientPathObject_H
#define INCLUDED_ClientPathObject_H

// ======================================================================

#include "sharedObject/Object.h"

class TerrainObject;

// ======================================================================

class ClientPathObject : public Object
{
public:
	static void install ();
	static void setAppearance(std::string const & appearanceName);
public:

	ClientPathObject (stdvector<Vector>::fwd const & pointList);
	virtual ~ClientPathObject ();

	void resetBoundary ();
	virtual float alter(float time);

private:

	void create (stdvector<Vector>::fwd const & pointList);

	void updatePath();
	void optimizePath();
	void getTerrainHeight(TerrainObject const * const terrain, Vector & point);
	void addPathNode(float distance = 0.0f);
	void updateNodes();
	void updateSpeedAndHeight();

private:

	ClientPathObject ();
	ClientPathObject (const ClientPathObject&);
	ClientPathObject& operator= (const ClientPathObject&);
	
private:
	class Implementation;
	Implementation * m_pimpl;
};

// ======================================================================

#endif

