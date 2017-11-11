//===================================================================
//
// ClientRadialFloraManager.h
// asommers
//
// copyright 2001, sony online entertainment
//
//===================================================================

#ifndef INCLUDED_ClientRadialFloraManager_H
#define INCLUDED_ClientRadialFloraManager_H

//===================================================================

#include "sharedMath/Rectangle2d.h"
#include "sharedMath/Vector.h"
#include "sharedTerrain/FloraManager.h"

#include <vector>

class BoxExtent;
class Camera;
class ClientProceduralTerrainAppearance;
class CollisionInfo;
class Object;
class Transform;

//===================================================================

class ClientRadialFloraManager : public FloraManager
{
public:

	class ClearFloraEntry;

public:

	static void install ();

	typedef stdvector<std::pair<Vector, float> >::fwd ClearFloraEntryList;
	static void addClearFloraObject (const Object* object, const ClearFloraEntryList& clearFloraEntryList);
	static void removeClearFloraObject (const Object* object);

public:

	ClientRadialFloraManager (const ClientProceduralTerrainAppearance& terrainAppearance, const bool& enabled, float minimumDistance, float const & maximumDistance);
	virtual ~ClientRadialFloraManager (void)=0;

	void setDebugName (const std::string& debugName);
	void initialize (float floraTileSize, float floraTileBorderIgnoreDistance, uint32 seed);

	void markExtentDirty (const BoxExtent* boxExtent);
	void markRegionDirty (const Rectangle2d& rectangle);

	bool isEnabled () const;
	const ClientProceduralTerrainAppearance &getTerrainAppearance() const { return m_terrainAppearance; }

	virtual void alter (float time);
	virtual void preRender (const Camera* camera);
	virtual void draw () const=0;
	virtual bool collide (const Vector& start_w, const Vector& end_w, CollisionInfo& result) const;
	virtual bool approximateCollide (const Vector& start_w, const Vector& end_w, CollisionInfo& result) const;

protected:

	//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

	class RadialNode
	{
	public:

		RadialNode ();
		virtual ~RadialNode ()=0;

		virtual void             enabledChanged ();
		virtual const Vector     getPosition () const=0;
		virtual void             setPosition (const Vector& position)=0;
		virtual bool             shouldFloat () const=0;
		virtual bool             shouldAlignToTerrain () const=0;
		virtual const Transform& getTransform () const=0;
		virtual void             setTransform (const Transform& transform)=0;
		virtual void             setHasFlora (bool hasFlora);

		void setAlpha (float alpha);

		void                     setManager (ClientRadialFloraManager* manager);

		bool                     getHasFlora ()     const { return m_hasFlora; }
		bool                     getFloraAllowed () const { return m_floraAllowed; }
		const Vector &           getNormal ()       const { return m_normal; }
		real                     getAlpha ()        const { return static_cast<float>(m_alpha) / 255.0f; }

	protected:

		ClientRadialFloraManager* m_manager;

	private:

		void setNormal (const Vector& normal);
		void setFloraAllowed (bool floraAllowed);

	private:

		RadialNode (const RadialNode&);
		RadialNode& operator= (const RadialNode&);

	private:

		bool   m_hasFlora;
		bool   m_floraAllowed;
		uint8  m_alpha;
		Vector m_normal;
		
	private:

		friend class ClientRadialFloraManager;
	};

protected:

	static bool shouldClearFlora(float positionX, float positionZ, float mapHalfWidthInMeters);

protected:

	virtual RadialNode* createRadialNode (const Vector& position) const=0;
	virtual bool        createFlora (float positionX, float positionZ, RadialNode* radialNode, bool& floraAllowed) const=0;
	virtual void        maximumDistanceChanged ();

protected:

	void clearRegionList () const;
	bool isWithinDirtyRegion (float x, float y) const;

	void update (const Vector& origin) const;

protected:

	struct RadialNodeReference
	{
		RadialNode *nodePointer;
		float position_x;
		float position_z;
		uint8 floraAllowed : 1;
		uint8 hasFlora     : 1;
	};

	typedef std::vector<Rectangle2d>          RegionList;
	typedef std::vector<RadialNodeReference>  RadialNodeList;

	void _constructRadialNodeReference(RadialNodeReference &o_reference, RadialNode *node);
	void _addRadialNode(RadialNode *newRadialNode);
#ifdef _DEBUG
	void _verifyRadialNodeReference(const RadialNodeReference &reference) const;
#endif

protected:

	std::string*                             m_debugName;
	const bool&                              m_enabled;
	bool                                     m_lastEnabled;
	
	const ClientProceduralTerrainAppearance& m_terrainAppearance;
	const float                              m_mapHalfWidthInMeters;
	const float                              m_minimumDistance;
	float const &                            m_maximumDistance;
	float                                    m_lastMaximumDistance;
	float                                    m_floraTileSize;
	float                                    m_floraTileBorderIgnoreDistance;
	uint32                                   m_seed;

	mutable RegionList                       m_regionList;
	mutable RadialNodeList                   m_radialNodeList;
	
	mutable Vector                           m_oldOrigin;

private:

	ClientRadialFloraManager (void);
	ClientRadialFloraManager (const ClientRadialFloraManager&);
	ClientRadialFloraManager& operator= (const ClientRadialFloraManager&);
};

//===================================================================

#endif

