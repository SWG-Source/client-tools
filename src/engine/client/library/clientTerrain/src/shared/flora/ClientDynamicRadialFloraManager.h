//===================================================================
//
// ClientDynamicRadialFloraManager.h
// asommers
//
// copyright 2001, sony online entertainment
//
//===================================================================

#ifndef INCLUDED_ClientDynamicRadialFloraManager_H
#define INCLUDED_ClientDynamicRadialFloraManager_H

//===================================================================

#include "clientTerrain/ClientProceduralTerrainAppearance.h"
#include "clientTerrain/ClientRadialFloraManager.h"
#include "sharedMath/PackedRgb.h"

//===================================================================

class ClientDynamicRadialFloraManager : public ClientRadialFloraManager
{
	typedef bool (ClientProceduralTerrainAppearance:: *FindFloraFunction)(float positionX, float positionZ, ClientProceduralTerrainAppearance::DynamicFloraData& data, bool& floraAllowed) const;

public:

	class FloraRenderBucket;

public:

	static void install ();
	static void remove ();

public:

	ClientDynamicRadialFloraManager (const ClientProceduralTerrainAppearance& terrainAppearance, const bool& enabled, float minimumRadius, float const & maximumRadius, FindFloraFunction findFloraFunction, bool applyColor);
	virtual ~ClientDynamicRadialFloraManager (void);

	virtual void alter(float elapsedTime);
	virtual void draw () const;

private:

	class DynamicRadialNode;

private:

	typedef std::vector<FloraRenderBucket*>      FloraRenderBucketList;

	class FloraBucket
	{
	public:

		FloraBucket(float i_ringDistance) : m_ringDistance(i_ringDistance) {}

		float                             m_ringDistance;
		mutable FloraRenderBucketList     m_belowWaterFloraRenderBucketList;
		mutable FloraRenderBucketList     m_aboveWaterFloraRenderBucketList;
	};

	typedef std::vector<FloraBucket> FloraBucketList;

private:

	virtual void        maximumDistanceChanged ();

	void                freeBuckets();
	static void         freeFloraRenderBuckets(FloraRenderBucketList &i_list);

	void                clearBuckets() const;
	static void         clearFloraRenderBuckets(FloraRenderBucketList &i_list);

	void                drawBuckets() const;
	static void         drawFloraRenderBuckets(FloraRenderBucketList &i_list);

	void                internalInitialize ();
	virtual RadialNode* createRadialNode (const Vector& position) const;
	virtual bool        createFlora (float positionX, float positionZ, RadialNode* radialNode, bool& floraAllowed) const;
	void                addToBucket (const Vector& position, float depth, const DynamicRadialNode* const dynamicRadialNode);

private:

	ClientDynamicRadialFloraManager (void);
	ClientDynamicRadialFloraManager (const ClientDynamicRadialFloraManager&);
	ClientDynamicRadialFloraManager& operator= (const ClientDynamicRadialFloraManager&);

private:

#ifdef _DEBUG
	static bool ms_noDraw;
	static bool ms_reportDynamicFlora;
#endif

private:

	double                     m_floraSwayAngle;
	FloraBucketList            m_floraBucketList;
	FindFloraFunction          m_findFloraFunction;
	const bool                 m_applyColor;

private:

	friend class FloraRenderBucket;
	friend class DynamicRadialNode;
};

//===================================================================

#endif
