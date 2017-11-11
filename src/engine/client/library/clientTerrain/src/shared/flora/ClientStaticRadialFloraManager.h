//===================================================================
//
// ClientStaticRadialFloraManager.h
// asommers
//
// copyright 2001, sony online entertainment
//
//===================================================================

#ifndef INCLUDED_ClientStaticRadialFloraManager_H
#define INCLUDED_ClientStaticRadialFloraManager_H

//===================================================================

#include "clientTerrain/ClientRadialFloraManager.h"
#include "clientTerrain/ClientProceduralTerrainAppearance.h"

#include <vector>

//===================================================================

class ClientStaticRadialFloraManager : public ClientRadialFloraManager
{
	typedef bool (ClientProceduralTerrainAppearance:: *FindFloraFunction)(float positionX, float positionZ, ClientProceduralTerrainAppearance::StaticFloraData& data, bool& floraAllowed) const;

public:

	static void install ();
	static void remove ();

	typedef void (*RenderDetailLevelFunction) (const Object* object);
	static void setRenderDetailLevelFunction (RenderDetailLevelFunction renderDetailLevelFunction);

public:

	ClientStaticRadialFloraManager (const ClientProceduralTerrainAppearance& terrainAppearance, const bool& enabled, float minimumDistance, float const & maximumDistance, FindFloraFunction findFloraFunction);
	virtual ~ClientStaticRadialFloraManager (void);

	virtual void alter (float elapsedTime);
	virtual void draw () const;
	virtual bool collide (const Vector& start_w, const Vector& end_w, CollisionInfo& result) const;
	virtual bool approximateCollide (const Vector& start_w, const Vector& end_w, CollisionInfo& result) const;

private:

	class StaticFloraNode;
	class StaticRadialNode;

private:

	typedef std::vector<StaticFloraNode*> FloraNodeList;

private:

	virtual RadialNode* createRadialNode (const Vector& position) const;
	virtual bool        createFlora (float positionX, float positionZ, RadialNode* radialNode, bool& floraAllowed) const;
	virtual void        maximumDistanceChanged ();

private:

	ClientStaticRadialFloraManager (void);
	ClientStaticRadialFloraManager (const ClientStaticRadialFloraManager&);
	ClientStaticRadialFloraManager& operator= (const ClientStaticRadialFloraManager&);

private:

	mutable FloraNodeList  m_floraNodeList;
	FindFloraFunction      m_findFloraFunction;

#ifdef _DEBUG
	mutable int            m_nextObjectId;
#endif

private:

	friend class StaticFloraNode;
	friend class StaticRadialNode;
};

//===================================================================

#endif
